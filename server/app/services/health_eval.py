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

def ist_filter(dt):
    """Jinja2 filter to convert UTC datetime to IST string."""
    if not dt: return "-"
    try:
        # If it's already a string, just return it or try simple ISO parse
        if isinstance(dt, str):
            if " " in dt: dt = dt.split(".")[0] # Clean up miliseconds
            return dt
            
        # IST = UTC + 5:30
        ist_time = dt + datetime.timedelta(hours=5, minutes=30)
        return ist_time.strftime("%m-%d %H:%M:%S")
    except Exception:
        return str(dt)

# ── Thresholds ────────────────────────────────────────────────────────────────
OFFLINE_MINS         = 1470   # Level: Relaxed to 24.5 hours (Prev 7h) — Flag offline only if full day of reports is missing.
CRITICAL_BAT         = 3.40   # V — immediate risk of shutdown
LOW_BAT              = 3.60   # V — needs attention
WEAK_SIGNAL          = -95    # dBm — calls likely to fail at this level
MAX_REG_FAILS_DAY    = 25     # Increase threshold for 2G networks
MAX_HTTP_FAILS_DAY   = 30     # Increase threshold for 2G networks
MIN_PREV_STORED_PCT  = 0.90   # yesterday should have at least 90% (86/96)
MIN_TODAY_SENT_PCT   = 0.85   # today sent ≥ 85% so far = OK

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

    # ── 1. Basic check ───────────────────────────────────────────────────────
    if r.reported_at is None:
        return {"verdict": "OFFLINE", "reasons": ["No report received"], "score": 0}

    delta_mins = (now - r.reported_at).total_seconds() / 60
    if delta_mins > OFFLINE_MINS:
        return {"verdict": "OFFLINE",
                "reasons": [f"No report for {int(delta_mins/60)}h"],
                "score": 0}

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

    # ── 4. Previous day completeness & Labels ────────────────────────────────
    prev_sent   = r.net_cnt_prev or 0
    prev_stored = r.prev_stored  or 0

    # ── PD: Partial Data (Previous Day records missing) ──
    # net_cnt_prev = confirmed records (Live + Backlog)
    # http_ret_cnt_prev = records recovered from yesterday AFTER the closing window
    total_gap = SLOTS_PER_DAY - prev_sent
    is_pd     = total_gap >= 10              # ≥10 missing = PD

    # v5.65 P4 Improvement: If net_cnt_prev is low, but we see significant 
    # recovery (http_ret_cnt_prev), then the station is 'Catching Up'.
    recov_p   = int(getattr(r, "http_ret_cnt_prev", 0) or 0)
    is_catching_up = (is_pd and recov_p > 5)

    # GPRS-Er: Data was stored locally but never reached the server
    is_gprs_er = (prev_sent == 0 and prev_stored > 0)

    # ── NDM: Night Data Missing ──
    # Fires when >50% of the 36 night slots (9 PM → 6 AM) were NOT sent
    # during their LIVE window. Backlog retries do NOT clear this — it captures
    # whether the device had battery power during the night.
    # Night window = 21:00 to 06:00 = 9 hours = 36 slots. 50% threshold = 18.
    # Firmware tracks this as `ndm_cnt` (diag_ndm_count).
    # NDM is INDEPENDENT of PD — a data-complete station can still have NDM.
    ndm_cnt = int(getattr(r, "ndm_cnt", 0) or 0)
    is_ndm  = ndm_cnt > 18    # >50% of 36 night slots missed live

    # ── CDM: Closing Data Missing ──
    # The 8:30 AM closing record for yesterday not received on server,
    # neither via that slot's HTTP nor any subsequent backlog retry.
    # Sourced directly from firmware's cdm_sts field.
    cdm_sts_val = str(getattr(r, "cdm_sts", "") or "").strip().upper()
    is_cdm      = cdm_sts_val in ("FAIL", "PENDING")

    historical_tags = []

    # ── Build verdict tags (order matters — most severe first) ──
    if is_pd:
        if is_ndm and is_cdm:
            reasons.append(f"PD({prev_sent}/96) NDM({ndm_cnt}/36) CDM")
            historical_tags.append("PD_NDM_CDM")
            demerits += 45
        elif is_cdm:
            reasons.append(f"PD({prev_sent}/96) CDM")
            historical_tags.append("PD_CDM")
            demerits += 35
        elif is_ndm:
            reasons.append(f"PD({prev_sent}/96) NDM({ndm_cnt}/36)")
            historical_tags.append("PD_NDM")
            demerits += 30
        elif is_gprs_er:
            reasons.append("GPRS-Er")
            historical_tags.append("GPRS-Er")
            demerits += 30
        else:
            if is_catching_up:
                reasons.append(f"PD({prev_sent}/96) + RECOV({recov_p})")
                historical_tags.append("PD (RECOVERING)")
                demerits += 5 # Minimal demerit for recovering stations
            else:
                reasons.append(f"PD({prev_sent}/96)")
                historical_tags.append("PD")
                demerits += 15
        # CDM can co-exist with PD independently (already handled above in PD+CDM)
    else:
        # Data count OK (≥86 records on server)
        if is_cdm:
            reasons.append("CDM")
            historical_tags.append("CDM")
            demerits += 15
        if is_ndm:
            # Station is data-OK but had night transmission failures → battery concern
            reasons.append(f"NDM({ndm_cnt}/36)")
            historical_tags.append("NDM")
            demerits += 5   # Reduced demerit
    # Else: <10 gap + CDM OK + NDM OK → fully green, no tags


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
    # Map internal codes to short friendly names
    # v7.75: Use exact matching to avoid "Rain_J" matching "Rain_JUMP" incorrectly
    FLAG_MAP = {
        "Temp_E": "TH-Er", "Temp_CV": "TH-Er", "Temp_ERV": "TH-Er", "TEMP_STUCK": "TH-Er", "TEMP_UNREAL": "TH-Er",
        "Humi_E": "TH-Er", "Humi_CV": "TH-Er", "Humi_ERV": "TH-Er", "HUM_STUCK": "TH-Er", "HUM_UNREAL": "TH-Er",
        "Rain_R": "RF-Er", "Rain_J": "RF-Er", "RAIN_SPIKE": "RF-Er", "RAIN_RESET": "RF-Er", "RAIN_CALC": "RF-Er",
        "WD_E": "WD-Er", "WD_FAIL": "WD-Er",
        "WS_E": "WS-Er", "WS_CV": "WS-Er", "WS_ERV": "WS-Er", "WS_STUCK": "WS-Er", "WS_UNREAL": "WS-Er",
        "RTC_F": "RTC-Er", "SPIFF": "SYS-Er", "FAIL": "SYS-Er",
        "WDOG": "SYS-Er", "BROWNOUT": "SYS-Er"
    }

    # v5.57: System-Aware Sensor Checks
    sys_type = int(getattr(r, "system", -1))
    sens_sts = (r.sensor_sts or "").upper()

    # TH, WS, WD error checks (Applicable to Systems 1, 2, or unknown)
    if sys_type in (1, 2, -1):
        if "TH-FAIL" in sens_sts or "TH-ERR" in sens_sts: reasons.append("TH-Er"); demerits += 10
        if "WS-FAIL" in sens_sts or "WS-ERR" in sens_sts: reasons.append("WS-Er"); demerits += 10
        if "WD-FAIL" in sens_sts or "WD-ERR" in sens_sts: reasons.append("WD-Er"); demerits += 10

    # RF error checks (Applicable to Systems 0, 2, or unknown)
    if sys_type in (0, 2, -1):
        if "RF-FAIL" in sens_sts or "RF-ERR" in sens_sts: reasons.append("RF-Er"); demerits += 10
    
    if "RTC-FAIL" in sens_sts: reasons.append("RTC-Er"); demerits += 10

    # HTTP Live Performance (Present Fails)
    # v7.70: Tracked LIVE current-slot failures.
    h_pres = int(getattr(r, "http_present_fails", 0) or 0)
    h_back = int(getattr(r, "http_backlog_cnt", 0) or 0)
    
    if h_pres > 3:
        # Live transmission is failing, forcing backlog.
        reasons.append(f"LV-Er({h_pres})")
        demerits += 5
        
    if h_back > 10:
        # Significant backlog building up
        reasons.append(f"BL-Er({h_back})")
        demerits += 5

    # Resource Contention (v5.55)
    m_fail = int(getattr(r, "mutex_fail", 0) or 0)
    if m_fail > 0:
        reasons.append(f"MUTEX-Er({m_fail})")
        demerits += 5

    # If the station stored a perfect 96 records yesterday, we are much more
    # forgiving of sensor flags
    flag_penalty = 2 if prev_stored >= 96 else 10
    
    sensor_tags = []
    for code, friendly_name in FLAG_MAP.items():
        # Use underscore boundary or start/end to avoid partial matches
        # e.g. "Rain_R" should not match "Rain_RESET"
        if f"_{code}" in f"_{fw_sts}" or f"{code}_" in f"{fw_sts}_":
            if friendly_name not in reasons:
                reasons.append(friendly_name)
                demerits += flag_penalty
                if friendly_name not in sensor_tags:
                    sensor_tags.append(friendly_name)
    
    # Deduplicate reasons (especially since we check two sources now)
    reasons = list(dict.fromkeys(reasons))

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

    # Build final verdict string from tags + sensor flags
    combined_tags = historical_tags + sensor_tags
    if combined_tags:
        # Special case: NDM alone (data count OK, no PD, no CDM) → "OK (NDM)"
        # Shows that the station is data-complete but has a night battery concern.
        if combined_tags == ["NDM"]:
            verdict = "OK (NDM)"
        else:
            verdict = ", ".join(combined_tags)

    else:
        verdict = base_verdict

    return {
        "verdict": verdict,
        "reasons": reasons if reasons else [],
        "score":   score,
        "fw_sts":  fw_sts,
    }



