from fastapi import APIRouter, Request, Depends
from fastapi.responses import StreamingResponse
from sqlalchemy.orm import Session
from sqlalchemy import func, desc, case
from app.database import SessionLocal
from app.models import HealthReport, FirmwareRegistry, CommandQueue, StationSettings
from app.services.health_eval import evaluate, ist_filter
import csv, io, datetime, traceback
from app.templates import templates

router = APIRouter()


def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()


def get_latest_per_station(db):
    """Returns exactly one (latest) record per unique station ID."""
    subq = db.query(
        HealthReport.stn_id,
        func.max(HealthReport.reported_at).label("m")
    ).group_by(HealthReport.stn_id).subquery()
    return db.query(HealthReport).join(
        subq,
        (HealthReport.stn_id == subq.c.stn_id) &
        (HealthReport.reported_at == subq.c.m)
    ).order_by(HealthReport.reported_at.desc()).all()


def _g(r, attr, default=None):
    """Safe getattr — returns default if column not yet in DB model."""
    return getattr(r, attr, default)


def _all_fields_row(r, now=None):
    """Returns a flat list of all health report fields for CSV export."""
    # v7.86: Offset to IST
    ist_time = r.reported_at + datetime.timedelta(hours=5, minutes=30) if r.reported_at else None
    
    ev = evaluate(r, now)
    return [
        ist_time, _g(r, "stn_id"), _g(r, "unit_type"), _g(r, "system"),
        _g(r, "health_sts"), _g(r, "sensor_sts"),
        _g(r, "bat_v"), _g(r, "sol_v"), _g(r, "signal"),
        _g(r, "net_cnt"), _g(r, "net_cnt_prev"),
        _g(r, "prev_stored"),
        _g(r, "reg_fails"), _g(r, "consec_reg_fails"),
        _g(r, "http_fails"), _g(r, "http_fail_reason"),
        _g(r, "http_suc_cnt"), _g(r, "http_suc_cnt_prev"),
        _g(r, "http_ret_cnt"), _g(r, "http_ret_cnt_prev"),
        _g(r, "http_present_fails", 0), _g(r, "http_cum_fails", 0),    # v7.70
        _g(r, "ftp_suc_cnt"), _g(r, "ftp_suc_cnt_prev"),
        _g(r, "ndm_cnt"), _g(r, "pd_cnt"), _g(r, "cdm_sts"), _g(r, "first_http"),
        _g(r, "unsent_count"),
        _g(r, "reset_reason"), _g(r, "rtc_ok"),
        _g(r, "spiffs_kb"), _g(r, "spiffs_total_kb"),
        _g(r, "sd_sts"), _g(r, "calib"), _g(r, "ver"), _g(r, "carrier"), _g(r, "iccid"), _g(r, "gps"),
        _g(r, "ota_fails"), _g(r, "ota_fail_reason"),
        _g(r, "consec_http_fails"), _g(r, "consec_sim_fails"),
        _g(r, "http_backlog_cnt", 0), _g(r, "mutex_fail", 0),    # v5.56
        " | ".join(ev.get("reasons", [])) if "reasons" in ev and ev["reasons"] else ev.get("verdict", "OK")
    ]


def _concise_fields_row(r, now=None):
    """Returns a streamlined list of essential fields only."""
    ist_time = r.reported_at + datetime.timedelta(hours=5, minutes=30) if r.reported_at else None
    ev = evaluate(r, now)
    v = ev.get("verdict", "OK")
    return [
        ist_time.strftime("%Y-%m-%d %H:%M") if ist_time else "N/A",
        v,
        f"{r.bat_v:.2f}V" if r.bat_v else "N/A",
        f"{r.sol_v:.2f}V" if r.sol_v else "N/A",
        r.signal if r.signal else "N/A",
        r.ver if r.ver else "N/A",
        r.gps if r.gps else "N/A"
    ]


ALL_FIELDS_HEADER = [
    "Timestamp_IST", "Station_ID", "Unit_Type", "System_Mode",
    "Health_Status", "Sensor_Status",
    "Battery_V", "Solar_V", "Signal_dBm",
    "Net_Count_Current", "Net_Count_PrevDay",
    "Stored_PrevDay",
    "Reg_Fails", "Consec_Reg_Fails",
    "HTTP_Fails", "HTTP_Fail_Reason",
    "HTTP_Suc_Current", "HTTP_Suc_PrevDay",
    "HTTP_Retry_Current", "HTTP_Retry_PrevDay",
    "HTTP_Present_Fails", "HTTP_Cum_Monthly_Fails",    # v7.70
    "FTP_Suc_Current", "FTP_Suc_PrevDay",
    "NDM_Count", "PD_Count", "CDM_Status", "First_HTTP_Count",
    "Unsent_Count",
    "Reset_Reason_Code", "RTC_OK",
    "SPIFFS_Used_KB", "SPIFFS_Total_KB",
    "SD_Card_Status", "Calibration_Info", "Firmware_Version", "Carrier", "ICCID", "GPS_LatLon",
    "OTA_Fails", "OTA_Fail_Reason",
    "Consec_HTTP_Fails", "Consec_SIM_Fails",
    "HTTP_Backlog_Count", "Modem_Mutex_Fails",    # v5.56
    "Calculated_Verdict"
]

CONCISE_CSV_HEADER = [
    "Date_Time_IST", "Health_Status", "Battery", "Solar", "Signal_dBm", "Firmware", "GPS_Coordinates"
]



from app.services.ota_service import needs_ota
from app.services.health_eval import evaluate

@router.get("/dashboard")
async def dashboard(request: Request, db: Session = Depends(get_db)):
    try:
        reports = get_latest_per_station(db)
        
        # Fetch & Sort Firmware Groups: KSNDMC first, Bihar second, Spatika third
        fws = db.query(FirmwareRegistry).all()
        def get_priority(fw):
            ut = (fw.unit_type or "").upper()
            sys = fw.system_mode
            if "DMC" in ut and sys == 0: return 1  # KSNDMC-TRG
            if "DMC" in ut and sys == 1: return 2  # KSNDMC-TWS
            if "DMC" in ut and sys == 2: return 3  # KSNDMC-ADDON
            if "BIH" in ut: return 4               # BIHAR-TRG
            if "GEN" in ut and sys == 0: return 5  # SPATIKA-TRG
            if "GEN" in ut and sys == 2: return 6  # SPATIKA-ADDON
            return 99 + fw.category_id

        fws.sort(key=get_priority)
        
        # Build lookup map
        fw_map = {
            (str(fw.unit_type or "") + str(fw.system_mode or 0)): fw
            for fw in fws
        }
        
        # Bulk fetch all settings (GPS cache, OTA exempt, Mute state) with normalized keys
        settings_raw = db.query(StationSettings).all()
        settings_map = {}
        for s in settings_raw:
            if s.stn_id:
                settings_map[s.stn_id] = s
                s_norm = s.stn_id.lstrip('0') if s.stn_id.isdigit() else s.stn_id
                if s_norm: settings_map[s_norm] = s
                if s.stn_id.isdigit(): settings_map[s.stn_id.zfill(6)] = s
        
        # Bulk fetch all pending commands
        pending_map = {
            c.stn_id: c for c in db.query(CommandQueue).filter(CommandQueue.executed_at == None).all()
        }
        
        # Bulk fetch latest PAUSE / RESUME commands to guarantee bulletproof mute state
        latest_pause_cmds = db.query(CommandQueue).filter(
            CommandQueue.cmd.in_(["PAUSE_LIVE_POST", "PAUSE_TX", "PAUSE_KSNDMC", "RESUME_LIVE_POST", "RESUME_TX", "RESUME_KSNDMC"])
        ).order_by(CommandQueue.id.asc()).all()

        cmd_muted_stns = {}
        for c in latest_pause_cmds:
            norm = c.stn_id.lstrip('0') if c.stn_id and c.stn_id.isdigit() else c.stn_id
            if c.cmd in ("PAUSE_LIVE_POST", "PAUSE_TX", "PAUSE_KSNDMC"):
                cmd_muted_stns[norm] = c.cmd_param if c.cmd_param else None
                cmd_muted_stns[c.stn_id] = c.cmd_param if c.cmd_param else None
            else:
                cmd_muted_stns.pop(norm, None)
                cmd_muted_stns.pop(c.stn_id, None)

        now = datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None)

        # ── MET Day Boundaries (IST-based) ──────────────────────────────────────
        # MET day closes at 08:30 IST, new MET day starts at 08:45 IST
        IST_OFFSET  = datetime.timedelta(hours=5, minutes=30)
        now_ist     = now + IST_OFFSET
        today_ist   = now_ist.date()

        # MET-today window: 08:45 IST today → now (if current IST time is before 08:45 IST, query from yesterday 08:45 IST)
        met_today_start_ist = datetime.datetime.combine(today_ist, datetime.time(8, 45))
        if now_ist < met_today_start_ist:
            met_today_start_ist = met_today_start_ist - datetime.timedelta(days=1)
        met_today_start_utc = met_today_start_ist - IST_OFFSET   # 03:15 UTC

        # MET-yesterday window: 08:45 IST yesterday → 08:30 IST today
        met_ydy_start_ist   = met_today_start_ist - datetime.timedelta(days=1)
        met_ydy_end_ist     = datetime.datetime.combine(today_ist, datetime.time(8, 30))
        met_ydy_start_utc   = met_ydy_start_ist - IST_OFFSET
        met_ydy_end_utc     = met_ydy_end_ist   - IST_OFFSET     # 03:00 UTC

        # Always keep met_today_active True so figures are never hidden with '—'
        met_today_active = True

        # Bulk DB count & Direct/Backlog stats for MET Today
        met_today_map = {}
        if met_today_active:
            _today_rows = db.query(
                HealthReport.stn_id,
                func.count(HealthReport.id).label("tot"),
                func.sum(
                    case(
                        (
                            (func.coalesce(HealthReport.http_fails, 0) == 0) &
                            (func.coalesce(HealthReport.http_ret_cnt, 0) == 0) &
                            (func.coalesce(HealthReport.http_backlog_cnt, 0) == 0),
                            1
                        ),
                        else_=0
                    )
                ).label("dir")
            ).filter(HealthReport.reported_at >= met_today_start_utc
            ).group_by(HealthReport.stn_id).all()
            
            for row in _today_rows:
                tot = row.tot or 0
                dir_cnt = row.dir or 0
                backlog_cnt = max(0, tot - dir_cnt)
                met_today_map[row.stn_id] = {
                    "total": tot,
                    "direct": dir_cnt,
                    "backlog": backlog_cnt,
                    "display": f"{dir_cnt} / {backlog_cnt}"
                }

        # Bulk DB count & Direct/Backlog stats for MET Yesterday
        _ydy_rows = db.query(
            HealthReport.stn_id,
            func.count(HealthReport.id).label("tot"),
            func.sum(
                case(
                    (
                        (func.coalesce(HealthReport.http_fails, 0) == 0) &
                        (func.coalesce(HealthReport.http_ret_cnt, 0) == 0) &
                        (func.coalesce(HealthReport.http_backlog_cnt, 0) == 0),
                        1
                    ),
                    else_=0
                )
            ).label("dir")
        ).filter(
            HealthReport.reported_at >= met_ydy_start_utc,
            HealthReport.reported_at <  met_ydy_end_utc
        ).group_by(HealthReport.stn_id).all()

        met_ydy_map = {}
        for row in _ydy_rows:
            tot = row.tot or 0
            dir_cnt = row.dir or 0
            backlog_cnt = max(0, tot - dir_cnt)
            met_ydy_map[row.stn_id] = {
                "total": tot,
                "direct": dir_cnt,
                "backlog": backlog_cnt,
                "display": f"{dir_cnt} / {backlog_cnt}"
            }

        import re
        deduped = {}
        for r in reports:
            s_clean = re.sub(r'[^A-Z0-9]', '', str(r.stn_id or "").upper())
            if not s_clean: continue
            
            norm_id = s_clean.lstrip('0')
            if not norm_id: norm_id = "0"
            
            if norm_id not in deduped:
                deduped[norm_id] = r
            else:
                existing = deduped[norm_id]
                if r.reported_at and existing.reported_at:
                    if r.reported_at > existing.reported_at:
                        deduped[norm_id] = r
                elif r.reported_at:
                    deduped[norm_id] = r
        
        reports = list(deduped.values())

        # Summary card counts
        total       = len(reports)
        alarms      = 0
        low_bat     = 0
        ota_pending = 0

        for r in reports:
            key        = (r.unit_type or "") + str(r.system or 0)
            r.fw_group = fw_map.get(key)
            r.group_code = r.fw_group.display_name if r.fw_group else (r.unit_type or 'UNKNOWN')
            
            # Use cached settings & GPS fallback
            s_raw = str(r.stn_id or "").strip()
            s_norm = s_raw.lstrip('0') if s_raw.isdigit() else s_raw
            s_cache = settings_map.get(r.stn_id) or settings_map.get(s_norm)
            r.is_exempt = (s_cache.ota_exempt == 1) if s_cache else False
            if s_cache and s_cache.last_gps and str(s_cache.last_gps).strip() not in ("NA", "0,0", "0.000000,0.000000", "None", ""):
                if not r.gps or str(r.gps).strip() in ("NA", "0,0", "0.000000,0.000000", "None", ""):
                    r.gps = s_cache.last_gps

            # Pause & Mute Metadata
            r.is_muted = False
            r.pause_reason = None
            r.paused_at_ist = None
            r.paused_duration = None
            is_cmd_muted = s_norm in cmd_muted_stns or r.stn_id in cmd_muted_stns
            if (s_cache and s_cache.muted == 1) or r.muted == 1 or 'MUTED' in (r.health_sts or '') or is_cmd_muted:
                r.is_muted = True
                r.pause_reason = (s_cache.pause_reason if (s_cache and s_cache.pause_reason) else cmd_muted_stns.get(s_norm) or cmd_muted_stns.get(r.stn_id))
                if s_cache and s_cache.paused_at:
                    p_ist = s_cache.paused_at + datetime.timedelta(hours=5, minutes=30)
                    r.paused_at_ist = p_ist.strftime("%d-%m %H:%M IST")
                    delta = now - s_cache.paused_at
                    p_mins = max(0, int(delta.total_seconds() / 60))
                    r.paused_duration = f"{p_mins}m" if p_mins < 60 else f"{p_mins // 60}h {p_mins % 60}m"

            # Deep DB historical GPS scan if still missing
            if not r.gps or str(r.gps).strip() in ("NA", "0,0", "0.000000,0.000000", "None", ""):
                target_stn_ids = [r.stn_id]
                s_raw = str(r.stn_id).strip()
                if s_raw.isdigit():
                    target_stn_ids.extend([s_raw.lstrip('0'), s_raw.zfill(6)])
                hist_gps_rec = db.query(HealthReport.gps).filter(
                    HealthReport.stn_id.in_(target_stn_ids),
                    HealthReport.gps.isnot(None),
                    HealthReport.gps != "",
                    HealthReport.gps != "NA",
                    HealthReport.gps != "0,0",
                    HealthReport.gps != "0.000000,0.000000",
                    HealthReport.gps != "None"
                ).order_by(HealthReport.reported_at.desc()).first()
                if hist_gps_rec and hist_gps_rec[0]:
                    r.gps = hist_gps_rec[0]
                    if s_cache:
                        s_cache.last_gps = r.gps

            # OTA badge
            r.ota_needed = False
            if not r.is_exempt and r.fw_group and r.ver:
                if needs_ota(r.ver, r.fw_group.current_ver):
                    r.ota_needed = True
                    ota_pending += 1

            # MET-day counts: prefer firmware net_cnt (ground truth), fall back to DB health-report row count.
            # DB row count can exceed 96 in 15-min pulse mode (one health report per slot).
            # net_cnt / net_cnt_prev are already the verified delivered data-record counts.
            SLOTS_CAP = 96

            t_info = met_today_map.get(r.stn_id)
            fw_today = r.net_cnt or 0
            dir_c = r.http_suc_cnt if (r.http_suc_cnt is not None and r.http_suc_cnt >= 0) else 0
            ret_c = (r.http_ret_cnt or 0) + (r.ftp_suc_cnt or 0)
            deliv_today = dir_c + ret_c

            r.muted_today_slots = max(0, fw_today - deliv_today) if (r.is_muted or (dir_c > 0 and dir_c < fw_today)) else 0

            if r.is_muted or (r.http_suc_cnt is not None and deliv_today < fw_today):
                r.met_today = min(deliv_today, SLOTS_CAP)
            elif fw_today > 0:
                r.met_today = min(fw_today, SLOTS_CAP)
            elif t_info and t_info["total"] > 0:
                r.met_today = min(t_info["total"], SLOTS_CAP)
            else:
                r.met_today = min(deliv_today, SLOTS_CAP)

            r.met_today_http = f"{min(dir_c, SLOTS_CAP)} / {min(ret_c, SLOTS_CAP)}"

            y_info = met_ydy_map.get(r.stn_id)
            fw_ydy = r.net_cnt_prev or 0
            dir_p = r.http_suc_cnt_prev if (r.http_suc_cnt_prev is not None and r.http_suc_cnt_prev >= 0) else 0
            ret_p = (r.http_ret_cnt_prev or 0) + (r.ftp_suc_cnt_prev or 0)
            deliv_ydy = dir_p + ret_p

            r.muted_ydy_slots = max(0, fw_ydy - deliv_ydy) if (r.is_muted or (dir_p > 0 and dir_p < fw_ydy)) else 0

            if r.is_muted or (r.http_suc_cnt_prev is not None and deliv_ydy < fw_ydy):
                r.met_ydy = min(deliv_ydy, SLOTS_CAP)
            elif fw_ydy > 0:
                r.met_ydy = min(fw_ydy, SLOTS_CAP)
            elif y_info and y_info["total"] > 0:
                r.met_ydy = min(y_info["total"], SLOTS_CAP)
            else:
                r.met_ydy = min(deliv_ydy, SLOTS_CAP)

            r.met_ydy_http = f"{min(dir_p, SLOTS_CAP)} / {min(ret_p, SLOTS_CAP)}"

            # Objective Health Evaluation (Using computed MET day totals)
            r.eval = evaluate(r, now)
            if r.eval["verdict"] in ("CRITICAL", "WARN", "OFFLINE"):
                alarms += 1
            if any("BATT" in str(reason) for reason in r.eval["reasons"]):
                low_bat += 1

            # Sort Priority Arrangement
            ut = (r.unit_type or r.ver or "").upper()
            sys = r.system
            r.sort_priority = 99
            
            if "DMC" in ut and sys == 0: r.sort_priority = 1
            elif "DMC" in ut and sys == 1: r.sort_priority = 2
            elif "DMC" in ut and sys == 2: r.sort_priority = 3
            elif "BIH" in ut: r.sort_priority = 4
            elif "GEN" in ut and sys == 0: r.sort_priority = 5
            elif "GEN" in ut and sys == 2: r.sort_priority = 6

            # Relative time
            if r.reported_at:
                delta     = now - r.reported_at
                mins      = int(delta.total_seconds() / 60)
                r.time_ago = f"{mins}m ago" if mins < 60 else f"{mins // 60}h {mins % 60}m ago"
            else:
                r.time_ago = "?"

            # Pending command badge
            r.pending = pending_map.get(r.stn_id)

        # Helper function for version comparison
        def get_numeric_ver(v_str):
            if not v_str: return 0.0
            m = re.search(r'(\d+\.\d+)', str(v_str))
            return float(m.group(1)) if m else 0.0

        # Build Group Health & Fleet Stats Map (Pre-populated for all firmware groups)
        group_stats = {
            "all": {
                "total_seen": 0, "converted": 0, "latest_ver": "N/A", "latest_num": 0.0,
                "target_ver": "N/A",
                "ok_count": 0, "fail_count": 0, "ota_pending": 0, "low_bat": 0, "muted_count": 0,
                "healthy_gprs_bat": 0, "marginal_gprs_bat": 0, "critical_gprs_bat": 0,
                "healthy_mcu_bat": 0, "marginal_mcu_bat": 0, "critical_mcu_bat": 0,
                "solar_active": 0, "solar_optimal": 0, "weak_signal": 0, "carriers": {}
            }
        }
        for fw in fws:
            gname = fw.display_name or fw.name
            group_stats[gname] = {
                "total_seen": 0, "converted": 0, "latest_ver": "N/A", "latest_num": 0.0,
                "target_ver": fw.current_ver or "N/A",
                "ok_count": 0, "fail_count": 0, "ota_pending": 0, "low_bat": 0, "muted_count": 0,
                "healthy_gprs_bat": 0, "marginal_gprs_bat": 0, "critical_gprs_bat": 0,
                "healthy_mcu_bat": 0, "marginal_mcu_bat": 0, "critical_mcu_bat": 0,
                "solar_active": 0, "solar_optimal": 0, "weak_signal": 0, "carriers": {}
            }

        for r in reports:
            grp = r.group_code or 'UNKNOWN'
            for key in (grp, 'all'):
                if key not in group_stats:
                    group_stats[key] = {
                        "total_seen": 0, "converted": 0, "latest_ver": "N/A", "latest_num": 0.0,
                        "target_ver": "N/A",
                        "ok_count": 0, "fail_count": 0, "ota_pending": 0, "low_bat": 0, "muted_count": 0,
                        "healthy_gprs_bat": 0, "marginal_gprs_bat": 0, "critical_gprs_bat": 0,
                        "healthy_mcu_bat": 0, "marginal_mcu_bat": 0, "critical_mcu_bat": 0,
                        "solar_active": 0, "solar_optimal": 0, "weak_signal": 0, "carriers": {}
                    }
                st = group_stats[key]
                st["total_seen"] += 1
                if getattr(r, 'is_muted', False):
                    st["muted_count"] += 1
                
                # Dynamic Latest Firmware Version Actually Seen in Field
                if r.ver and str(r.ver).strip() not in ("", "N/A", "None"):
                    v_num = get_numeric_ver(r.ver)
                    if v_num >= st["latest_num"]:
                        st["latest_ver"] = str(r.ver).strip()
                        st["latest_num"] = v_num

                # Target Ver & Conversion
                if r.fw_group and r.fw_group.current_ver:
                    st["target_ver"] = r.fw_group.current_ver
                    if r.ver and get_numeric_ver(r.ver) >= get_numeric_ver(r.fw_group.current_ver):
                        st["converted"] += 1
                
                # Health
                if r.eval["verdict"] in ("OK", "INFO"):
                    st["ok_count"] += 1
                else:
                    st["fail_count"] += 1

                # OTA & Low Bat per group
                if r.ota_needed:
                    st["ota_pending"] += 1
                if any("BATT" in str(reason) for reason in r.eval["reasons"]):
                    st["low_bat"] += 1
                
                # 1. GPRS Main Battery Evaluation (Li-Ion / SLA 3.7V - 4.2V / 12V)
                if r.bat_v and r.bat_v > 0:
                    if r.bat_v >= 3.8: st["healthy_gprs_bat"] += 1
                    elif 3.6 <= r.bat_v < 3.8: st["marginal_gprs_bat"] += 1
                    else: st["critical_gprs_bat"] += 1

                # 2. MCU Logic Battery Evaluation (3.3V Logic Rail)
                if r.mcu_bat and r.mcu_bat > 0:
                    if r.mcu_bat >= 3.2: st["healthy_mcu_bat"] += 1
                    elif 3.0 <= r.mcu_bat < 3.2: st["marginal_mcu_bat"] += 1
                    else: st["critical_mcu_bat"] += 1

                # 3. Solar Panel Active & Optimal Charging (>= 10.0V is Daylight Optimal, >= 1.2V is Active Charging)
                if r.sol_v:
                    if r.sol_v >= 1.2:
                        st["solar_active"] += 1
                    if r.sol_v >= 10.0:
                        st["solar_optimal"] += 1
                
                # Network
                if r.signal and r.signal < -100:
                    st["weak_signal"] += 1
                
                c = (r.carrier or "UNKNOWN").upper()
                if "AIRTEL" in c: c = "AIRTEL"
                elif "BSNL" in c: c = "BSNL"
                elif "JIO" in c: c = "JIO"
                elif "VI" in c or "VODA" in c: c = "VI"
                st["carriers"][c] = st["carriers"].get(c, 0) + 1

        # Calculate percentages
        for key, st in group_stats.items():
            st["pct"] = int((st["converted"] / st["total_seen"] * 100)) if st["total_seen"] > 0 else 0

        # Final Sort
        reports.sort(key=lambda x: (x.sort_priority, x.stn_id))

        return templates.TemplateResponse(request=request, name="dashboard.html", context={
            "request": request, "reports": reports, "fws": fws,
            "total": total, "alarms": alarms,
            "ota_pending": ota_pending, "low_bat": low_bat,
            "group_stats": group_stats,
            "met_today_active": met_today_active,
        })
    except Exception as e:
        print(f"CRITICAL 500 DASHBOARD ERROR: {e}")
        import traceback
        traceback.print_exc()
        return templates.TemplateResponse(request=request, name="error.html", context={"error_msg": str(e)}, status_code=500)


@router.get("/station/{stn_id}")
async def station_detail(stn_id: str, request: Request, db: Session = Depends(get_db)):
    """Full history page with de-cluttered daily trends."""
    try:
        s_raw = str(stn_id).strip()
        target_ids = {s_raw}
        if s_raw.isdigit():
            target_ids.add(s_raw.lstrip('0'))
            target_ids.add(s_raw.zfill(6))
        target_list = list(target_ids)

        raw_history = (
            db.query(HealthReport)
            .filter(HealthReport.stn_id.in_(target_list))
            .order_by(HealthReport.reported_at.desc())
            .limit(400)
            .all()
        )
        
        # Grouping Logic: Keep all from "Today", but only LATEST per day for history
        today = datetime.date.today()
        seen_dates = set()
        history = []
        
        for r in raw_history:
            if not r.reported_at: continue
            dt = r.reported_at.date()
            if dt == today:
                history.append(r)
            else:
                if dt not in seen_dates:
                    history.append(r)
                    seen_dates.add(dt)
        
        # Trim to last 10 check-ins
        history = history[:10]

        commands = (
            db.query(CommandQueue)
            .filter(CommandQueue.stn_id.in_(target_list))
            .order_by(CommandQueue.created_at.desc())
            .limit(10)
            .all()
        )
        
        setting = db.query(StationSettings).filter(StationSettings.stn_id.in_(target_list)).first()
        is_exempt = (setting.ota_exempt == 1) if setting else False

        
        now = datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None)
        today = datetime.date.today()
        for r in history:
            if r.reported_at and r.reported_at.date() < today:
                # Historical record: evaluate relative to report time + 1h
                # This preserves the health verdict as it was when filed,
                # instead of always showing OFFLINE for old records.
                eval_now = r.reported_at + datetime.timedelta(hours=1)
            else:
                eval_now = now   # Today's records: use real wall clock
            r.eval = evaluate(r, eval_now)

        # v7.93: Determine category for 'Back to Fleet' deep-linking
        category_id = None
        latest_report = raw_history[0] if raw_history else None
        if latest_report:
            fw = db.query(FirmwareRegistry).filter_by(
                unit_type=latest_report.unit_type,
                system_mode=latest_report.system
            ).first()
            if fw:
                category_id = fw.category_id

        # Phase 2: Fetch last known GPS/Settings
        settings = db.query(StationSettings).filter(StationSettings.stn_id.in_(target_list)).first()

        # v5.86 FIX: UI Resilience. If the primary GPS cache is empty/invalid, 
        # scan historical records to find the last known "Good" coordinate.
        # This prevents the badge from showing "Searching" when historical data exists.
        if settings and (not settings.last_gps or settings.last_gps in ("NA", "0,0", "0.000000,0.000000", "None")):
            for r in raw_history:
                if r.gps and str(r.gps).strip() not in ("NA", "0,0", "0.000000,0.000000", "None", ""):
                    settings.last_gps = r.gps
                    break

        # v5.90: Fetch last SET_WIFI_PASS command for supervisor display
        last_wifi_cmd = (
            db.query(CommandQueue)
            .filter(CommandQueue.stn_id.in_(target_list), CommandQueue.cmd=="SET_WIFI_PASS")
            .order_by(CommandQueue.created_at.desc())
            .first()
        )
        last_wifi_pass = last_wifi_cmd.cmd_param if last_wifi_cmd else None

        # Pause Info Calculation with CommandQueue fallback
        latest_pause_resume = db.query(CommandQueue).filter(
            CommandQueue.stn_id.in_(target_list),
            CommandQueue.cmd.in_(["PAUSE_LIVE_POST", "PAUSE_TX", "PAUSE_KSNDMC", "RESUME_LIVE_POST", "RESUME_TX", "RESUME_KSNDMC"])
        ).order_by(CommandQueue.id.desc()).first()

        is_stn_muted = False
        pause_reason = None
        if latest_pause_resume and latest_pause_resume.cmd in ("PAUSE_LIVE_POST", "PAUSE_TX", "PAUSE_KSNDMC"):
            is_stn_muted = True
            pause_reason = latest_pause_resume.cmd_param if latest_pause_resume.cmd_param else None
        elif (settings and settings.muted == 1) or (history and (history[0].muted == 1 or 'MUTED' in (history[0].health_sts or ''))):
            is_stn_muted = True
            pause_reason = settings.pause_reason if settings else None

        paused_info = None
        if is_stn_muted:
            p_at_str = "N/A"
            p_dur_str = ""
            if settings and settings.paused_at:
                p_ist = settings.paused_at + datetime.timedelta(hours=5, minutes=30)
                p_at_str = p_ist.strftime("%Y-%m-%d %H:%M IST")
                delta = now - settings.paused_at
                p_mins = max(0, int(delta.total_seconds() / 60))
                p_dur_str = f"{p_mins}m ago" if p_mins < 60 else f"{p_mins // 60}h {p_mins % 60}m ago"
            paused_info = {
                "muted": True,
                "reason": pause_reason,
                "paused_at": p_at_str,
                "duration": p_dur_str
            }

        return templates.TemplateResponse(
            request=request, name="station.html", context={
                "request": request,
                "stn_id": stn_id,
                "history": history,
                "commands": commands,
                "is_exempt": is_exempt,
                "category_id": category_id,
                "settings": settings,
                "last_wifi_pass": last_wifi_pass,
                "paused_info": paused_info,
            }
        )
    except Exception as e:
        print(f"CRITICAL 500 STATION ERROR: {e}")
        return templates.TemplateResponse(request=request, name="error.html", context={"error_msg": str(e)}, status_code=500)


# ── CSV Downloads ─────────────────────────────────────────────────────────────

@router.get("/csv/summary")
def csv_summary(db: Session = Depends(get_db)):
    """
    Summary CSV: One row per station (latest report only), all fields.
    Good for sharing fleet status snapshots.
    """
    reports = get_latest_per_station(db)
    now = datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None)
    output  = io.StringIO()
    writer  = csv.writer(output)
    writer.writerow(ALL_FIELDS_HEADER)
    for r in reports:
        writer.writerow(_all_fields_row(r, now))
    output.seek(0)
    return StreamingResponse(
        output, media_type="text/csv",
        headers={"Content-Disposition": "attachment; filename=spatika_fleet_summary.csv"}
    )


@router.get("/csv/full_history")
def csv_full_history(db: Session = Depends(get_db)):
    """
    Full History CSV: Every health report for every station.
    Use for audits or deep analysis.
    """
    records = (
        db.query(HealthReport)
        .order_by(HealthReport.stn_id, HealthReport.reported_at.desc())
        .all()
    )
    now = datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None)
    today = datetime.date.today()
    output = io.StringIO()
    writer = csv.writer(output)
    writer.writerow(ALL_FIELDS_HEADER)
    for r in records:
        if r.reported_at and r.reported_at.date() < today:
            eval_now = r.reported_at + datetime.timedelta(hours=1)
        else:
            eval_now = now
        writer.writerow(_all_fields_row(r, eval_now))
    output.seek(0)
    return StreamingResponse(
        output, media_type="text/csv",
        headers={"Content-Disposition": "attachment; filename=spatika_full_history.csv"}
    )


@router.get("/station/{stn_id}/csv")
def station_csv(stn_id: str, db: Session = Depends(get_db)):
    """
    Station CSV: Full history for ONE station, all fields.
    """
    s_raw = str(stn_id).strip()
    target_ids = {s_raw, s_raw.lstrip('0'), s_raw.zfill(6)} if s_raw.isdigit() else {s_raw}
    history = (
        db.query(HealthReport)
        .filter(HealthReport.stn_id.in_(list(target_ids)))
        .order_by(HealthReport.reported_at.desc())
        .all()
    )
    now = datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None)
    today = datetime.date.today()
    output = io.StringIO()
    writer = csv.writer(output)
    writer.writerow(ALL_FIELDS_HEADER)
    for r in history:
        if r.reported_at and r.reported_at.date() < today:
            eval_now = r.reported_at + datetime.timedelta(hours=1)
        else:
            eval_now = now
        writer.writerow(_all_fields_row(r, eval_now))
    output.seek(0)
    return StreamingResponse(
        output, media_type="text/csv",
        headers={"Content-Disposition": f"attachment; filename={stn_id}_history.csv"}
    )


@router.get("/station/{stn_id}/concise-csv")
def station_concise_csv(stn_id: str, db: Session = Depends(get_db)):
    """
    Concise CSV: Streamlined report for non-technical audits.
    """
    s_raw = str(stn_id).strip()
    target_ids = {s_raw, s_raw.lstrip('0'), s_raw.zfill(6)} if s_raw.isdigit() else {s_raw}
    history = (
        db.query(HealthReport)
        .filter(HealthReport.stn_id.in_(list(target_ids)))
        .order_by(HealthReport.reported_at.desc())
        .all()
    )
    now = datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None)
    output = io.StringIO()
    writer = csv.writer(output)
    writer.writerow(CONCISE_CSV_HEADER)
    for r in history:
        writer.writerow(_concise_fields_row(r, now))
    output.seek(0)
    return StreamingResponse(
        output, media_type="text/csv",
        headers={"Content-Disposition": f"attachment; filename={stn_id}_crisp_report.csv"}
    )
