"""
health_eval.py — Server-Side Objective Health Evaluator
========================================================
This module computes a GROUND TRUTH health verdict from raw DB numbers,
independent of what the firmware's self-reported health_sts says.

Rules are ordered by severity (CRITICAL > WARN > INFO).
The result is a dict:
  {
    "verdict":  "OK" | "WARN" | "CRITICAL" | "OFFLINE",
    "reasons":  ["reason1", "reason2", ...],
    "score":    0-100   (100 = perfect, 0 = completely broken)
  }
"""

import datetime

# ── Thresholds ────────────────────────────────────────────────────────────────
OFFLINE_MINS         = 1470   # Level: Relaxed to 24.5 hours (Prev 7h) — Flag offline only if full day of reports is missing.
CRITICAL_BAT         = 3.40   # V — immediate risk of shutdown
LOW_BAT              = 3.60   # V — needs attention
WEAK_SIGNAL          = -95    # dBm — calls likely to fail at this level
MAX_REG_FAILS_DAY    = 15     # registration timeouts in one day = problem
MAX_HTTP_FAILS_DAY   = 20     # HTTP failures per day = problem
MIN_PREV_STORED_PCT  = 0.75   # yesterday should have at least 75% of 96 slots
MIN_TODAY_SENT_PCT   = 0.70   # today sent ≥ 70% of expected slots so far = OK

# Slots per day
SLOTS_PER_DAY = 96

# Hour at which the data day starts (8:45 AM → slot 0)
DATA_DAY_START_HOUR  = 8
DATA_DAY_START_MIN   = 45


def _expected_slots_today(reported_at: datetime.datetime) -> int:
    """
    Returns how many 15-min slots SHOULD have been stored by the time
    the health report was received, based on report time.
    Data day starts at 08:45 and has 96 slots.
    """
    if reported_at is None:
        return 1
    h, m = reported_at.hour, reported_at.minute
    start_minutes = DATA_DAY_START_HOUR * 60 + DATA_DAY_START_MIN
    now_minutes   = h * 60 + m
    elapsed = now_minutes - start_minutes
    if elapsed < 0:
        # Before 8:45 AM — we're still in the window of yesterday's data day
        elapsed = (24 * 60 - start_minutes) + now_minutes
    slots = max(1, elapsed // 15)
    return min(slots, SLOTS_PER_DAY)


def evaluate(r, now: datetime.datetime = None) -> dict:
    """
    r       : a HealthReport ORM row (or any object with the same attributes)
    now     : current datetime (defaults to datetime.datetime.now())
    returns : {"verdict": str, "reasons": [str], "score": int}
    """
    if now is None:
        now = datetime.datetime.now()

    reasons  = []
    demerits = 0   # each issue adds demerits; score = 100 - demerits (clamped to 0)
    verdict  = "OK"

    # ── 1. OFFLINE check ─────────────────────────────────────────────────────
    if r.reported_at is None:
        return {"verdict": "OFFLINE", "reasons": ["No report received"], "score": 0}

    age_mins = (now - r.reported_at).total_seconds() / 60
    if age_mins > OFFLINE_MINS:
        h = int(age_mins // 60)
        m = int(age_mins % 60)
        label = f"{h}h {m}m" if h > 0 else f"{m}m"
        reasons.append(f"OFFLINE ({label} since last report)")
        return {"verdict": "OFFLINE", "reasons": reasons, "score": 0}

    # ── 2. Battery ───────────────────────────────────────────────────────────
    bat = r.bat_v or 0
    if bat < CRITICAL_BAT and bat > 0:
        reasons.append(f"BATT_CRIT ({bat:.2f}V)")
        demerits += 40
    elif bat < LOW_BAT and bat > 0:
        reasons.append(f"BATT_LOW ({bat:.2f}V)")
        demerits += 15

    # ── 3. Signal ────────────────────────────────────────────────────────────
    sig = r.signal or 0
    if sig < WEAK_SIGNAL and sig != 0:
        reasons.append(f"WEAK_SIGNAL ({sig} dBm)")
        demerits += 10

    # ── 4. Previous day completeness & Labels (Calculated) ───────────────────
    prev_sent = r.net_cnt_prev or 0
    prev_stored = r.prev_stored or 0
    
    historical_tags = []
    
    total_gap = SLOTS_PER_DAY - prev_sent
    storage_gap = SLOTS_PER_DAY - prev_stored
    migration_gap = prev_stored - prev_sent # gap between stored and sent
    
    # User Rule: PD only triggers if more than 10 records are missing from server
    is_pd = total_gap > 10
    is_cdm = storage_gap > 0
    is_ndm = is_pd and migration_gap >= 24

    if is_pd:
        if is_ndm and is_cdm:
            reasons.append(f"PD_NDM_CDM ({prev_sent}/{prev_stored})")
            historical_tags.append("PD_NDM_CDM")
            demerits += 40
        elif is_cdm:
            reasons.append(f"PD_CDM ({prev_sent}/{prev_stored})")
            historical_tags.append("PD_CDM")
            demerits += 35
        elif is_ndm:
            reasons.append(f"NDM ({total_gap} gap)")
            historical_tags.append("NDM")
            demerits += 25
        elif prev_sent == 0 and prev_stored > 0:
            reasons.append(f"GPRS-Er")
            historical_tags.append("GPRS-Er")
            demerits += 30
        else:
            reasons.append(f"PD ({total_gap} gap)")
            historical_tags.append("PD")
            demerits += 15
    elif is_cdm:
        reasons.append(f"CDM ({prev_stored}/96)")
        historical_tags.append("CDM")
        demerits += 15
    # Else (<10 gap and no storage gap) -> System is OK (no tags, no demerits)

    # ── 6. Registration failures ─────────────────────────────────────────────
    reg_f = r.reg_fails or 0
    if reg_f > 0:
        reasons.append(f"REGF({reg_f})")
        if reg_f >= MAX_REG_FAILS_DAY:
            demerits += 20
        elif reg_f >= 5:
            demerits += 10

    # ── 7. Health Status specific overrides ──────────────────────────────────
    fw_sts = r.health_sts or "OK"
    
    # Map internal codes to short friendly names per USER request
    FLAG_MAP = {
        "Temp_E": "TH-Er",
        "Humi_E": "TH-Er",
        "Rain_R": "RF-Er",
        "WD_E":   "WD-Er",
        "WS_E":   "WS-Er",
        "RTC_F":  "RTC-Er",
        "FAIL":   "SYS-Er"
    }

    # If the station stored a perfect 96 records yesterday, we are much more
    # forgiving of sensor flags
    flag_penalty = 2 if prev_stored >= 96 else 10
    
    sensor_tags = []
    for code, friendly_name in FLAG_MAP.items():
        if code in fw_sts:
            if friendly_name not in reasons:
                reasons.append(friendly_name)
                demerits += flag_penalty
                if friendly_name not in sensor_tags:
                    sensor_tags.append(friendly_name)

    if "NO_GPS" in fw_sts:
        # User requested NO_GP not to be an issue in STATUS
        # We still track it internally to trigger GET_GPS in health.py
        pass

    # ── 10. Score & Verdict ──────────────────────────────────────────────────
    score = max(0, min(100, 100 - demerits))

    if demerits == 0:
        base_verdict = "OK"
    elif demerits <= 15:
        base_verdict = "INFO"     # Greenish-Grey
    elif demerits <= 40:
        base_verdict = "WARN"     # Orange
    else:
        base_verdict = "CRITICAL" # Red

    # If there are historical gaps or active hardware errors, the verdict 
    # becomes the labels (e.g. PD, CDM, TH-Er)
    combined_tags = historical_tags + sensor_tags
    if combined_tags:
        verdict = "_".join(combined_tags)
    else:
        verdict = base_verdict

    return {
        "verdict": verdict,
        "reasons": reasons if reasons else [],
        "score":   score,
        "fw_sts":  fw_sts,
    }
