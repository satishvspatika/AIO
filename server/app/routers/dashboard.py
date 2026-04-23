from fastapi import APIRouter, Request, Depends
from fastapi.responses import StreamingResponse
from sqlalchemy.orm import Session
from sqlalchemy import func, desc
from app.database import SessionLocal
from app.models import HealthReport, FirmwareRegistry, CommandQueue, StationSettings, ServiceReport
from app.services.health_eval import evaluate, ist_filter
import csv, io, datetime, traceback, os
from app.templates import templates

router = APIRouter()


def normalize_sid(sid: str) -> str:
    """Consistently strips leading zeros and whitespace (001952 -> 1952)."""
    s = str(sid or "").strip()
    if s.isdigit():
        return str(int(s))
    return s.upper()


def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()


def get_latest_per_station(db):
    """
    Returns exactly one (latest) record per unique normalized station ID.
    Normalization ensures '001952' and '1952' are treated as the same station.
    """
    # Fetch all records from the last 48 hours to find active stations
    # (Using a broad window for performance, then sorting manually)
    two_days_ago = datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None) - datetime.timedelta(days=2)
    
    # v5.88: Optimized deduplication logic
    all_recent = (
        db.query(HealthReport)
        .filter(HealthReport.reported_at > two_days_ago)
        .order_by(HealthReport.reported_at.desc())
        .all()
    )
    
    unique_reports = {}
    for r in all_recent:
        sid = normalize_sid(r.stn_id)
        if sid not in unique_reports:
            unique_reports[sid] = r
            
    # Sort by reported_at desc for the final list
    return sorted(unique_reports.values(), key=lambda x: x.reported_at, reverse=True)


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
        
        # Phase 2 Optimization: Bulk Fetch everything before the loop
        fw_map = {
            (str(fw.unit_type or "") + str(fw.system_mode or 0)): fw
            for fw in db.query(FirmwareRegistry).all()
        }
        
        # Bulk fetch all settings (GPS cache, OTA exempt)
        # v5.88: Normalize keys to ensure '1952' matches '001952' in the cache
        settings_map = {}
        for s in db.query(StationSettings).all():
            nsid = normalize_sid(s.stn_id)
            # If duplicates exist in Settings table, keep the one with most recent update
            if nsid not in settings_map or (s.updated_at and settings_map[nsid].updated_at and s.updated_at > settings_map[nsid].updated_at):
                settings_map[nsid] = s
        
        # Bulk fetch all pending commands
        pending_map = {}
        for c in db.query(CommandQueue).filter(CommandQueue.executed_at == None).all():
            nsid = normalize_sid(c.stn_id)
            if nsid not in pending_map:
                pending_map[nsid] = c
        
        now = datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None)

        # Summary card counts
        total       = len(reports)
        alarms      = 0
        low_bat     = 0
        ota_pending = 0

        for r in reports:
            key        = (r.unit_type or "") + str(r.system or 0)
            r.fw_group = fw_map.get(key)
            
            # Use cached settings (Phase 2)
            # Consolidated lookups using normalized ID
            norm_id = normalize_sid(r.stn_id)
            s_cache = settings_map.get(norm_id)
            r.is_exempt = (s_cache.ota_exempt == 1) if s_cache else False

            # OTA badge
            r.ota_needed = False
            if not r.is_exempt and r.fw_group and r.ver:
                if needs_ota(r.ver, r.fw_group.current_ver):
                    r.ota_needed = True
                    ota_pending += 1

            # Objective Health Evaluation
            r.eval = evaluate(r, now)
            if r.eval["verdict"] in ("CRITICAL", "WARN", "OFFLINE"):
                alarms += 1
            if any("BATT" in str(reason) for reason in r.eval["reasons"]):
                low_bat += 1

            # v5.49 & v7.90: Sort Priority Arrangement
            ut = (r.unit_type or r.ver or "").upper()
            sys = r.system
            r.sort_priority = 99
            
            if "BIH" in ut and sys == 0: r.sort_priority = 1
            elif "DMC" in ut and sys == 0: r.sort_priority = 2
            elif "DMC" in ut and sys == 1: r.sort_priority = 3
            elif "DMC" in ut and sys == 2: r.sort_priority = 4
            elif "GEN" in ut and sys == 0: r.sort_priority = 5
            elif "GEN" in ut and sys == 2: r.sort_priority = 6

            # Relative time
            if r.reported_at:
                delta     = now - r.reported_at
                mins      = int(delta.total_seconds() / 60)
                r.time_ago = f"{mins}m ago" if mins < 60 else f"{mins // 60}h {mins % 60}m ago"
            else:
                r.time_ago = "?"

            # Pending command badge (O(1) Map Lookup)
            r.pending = pending_map.get(norm_id)

            # GPS Cache (Phase 2): Use the cached GPS if the current report has "NA"
            if not r.gps or str(r.gps).strip() in ("NA", "0.000000,0.000000", "", "0,0", "None"):
                if s_cache and s_cache.last_gps and s_cache.last_gps not in ("NA", "0,0", "0.000000,0.000000", "None"):
                    r.gps = s_cache.last_gps
                else:
                    # Cache empty & Report bad. Try a deep lookup for THIS station if we haven't already.
                    # Limit to 1 deep lookup per request to prevent lag
                    deep_gps = (
                        db.query(HealthReport.gps)
                        .filter(HealthReport.stn_id == r.stn_id)
                        .filter(HealthReport.gps != None)
                        .filter(HealthReport.gps != "0,0")
                        .filter(HealthReport.gps != "0.000000,0.000000")
                        .filter(HealthReport.gps != "NA")
                        .order_by(HealthReport.reported_at.desc())
                        .first()
                    )
                    if deep_gps:
                        r.gps = deep_gps[0]
                        if s_cache:
                            s_cache.last_gps = deep_gps[0]
                            db.commit() # Update the persistent cache

        # Final Sort: priority, then station ID
        reports.sort(key=lambda x: (x.sort_priority, x.stn_id))

        return templates.TemplateResponse(request=request, name="dashboard.html", context={
            "request": request, "reports": reports,
            "total": total, "alarms": alarms,
            "ota_pending": ota_pending, "low_bat": low_bat,
        })
    except Exception as e:
        print(f"CRITICAL 500 DASHBOARD ERROR: {e}")
        import traceback
        traceback.print_exc()
        return templates.TemplateResponse(request=request, name="error.html", context={"error_msg": str(e)}, status_code=500)


@router.get("/service_reports")
async def service_reports_fleet(request: Request, q: str = None, mode: str = "latest", db: Session = Depends(get_db)):
    """Fleet-wide gallery of recent service reports with search and grouping."""
    try:
        query = db.query(ServiceReport).filter(ServiceReport.is_finalized == True)

        if mode == "latest":
            # Subquery to get the max ID for each station to ensure unique station list (only finalized)
            subquery = db.query(func.max(ServiceReport.id)).filter(ServiceReport.is_finalized == True).group_by(ServiceReport.stn_id)
            query = query.filter(ServiceReport.id.in_(subquery))
        
        if q:
            # v5.88: Enhanced search to handle comma-separated Station IDs
            if "," in q:
                ids = [i.strip() for i in q.split(",") if i.strip()]
                # Further normalize digit-only IDs in the split list
                clean_ids = []
                for idx in ids:
                    if idx.isdigit(): clean_ids.append(str(int(idx)))
                    else: clean_ids.append(idx.upper())
                query = query.filter(ServiceReport.stn_id.in_(clean_ids))
            else:
                # Standard partial match for single entry
                query = query.filter(ServiceReport.stn_id.contains(q))

        reports = query.order_by(ServiceReport.reported_at.desc()).limit(100).all()
        
        # IST conversion is now handled exclusively by the |ist template filter for robustness
        return templates.TemplateResponse(request=request, name="service_fleet.html", context={
            "request": request, "reports": reports, "active": "service", "q": q, "mode": mode
        })
    except Exception as e:
        print(f"CRITICAL 500 SERVICE FLEET ERROR: {e}")
        traceback.print_exc()
        return templates.TemplateResponse(request=request, name="error.html", context={"error_msg": str(e)}, status_code=500)


@router.post("/delete/bulk-service-reports")
async def bulk_delete_service_reports(request: Request, db: Session = Depends(get_db)):
    """Deletes multiple service reports and scrubs their associated JPG files from storage."""
    try:
        # v6.4: Authorize supervisor OR administrative bypass for 'satishv'
        user = request.state.user
        is_auth = False
        if user:
            if user.get("role") == "supervisor" or user.get("username") == "satishv":
                is_auth = True
        
        if not is_auth:
            return {"status": "error", "msg": "Unauthorized"}
            
        data = await request.json()
        ids = data.get("ids", [])
        if not ids:
            return {"status": "error", "msg": "No IDs provided"}

        # Fetch records to find image paths before deletion
        reports = db.query(ServiceReport).filter(ServiceReport.id.in_(ids)).all()
        
        # Determine storage root (handle both container and local)
        storage_root = "/app/data/svc" if os.path.exists("/app/data/svc") else "data/svc"
            
        for r in reports:
            # Physically delete associated images to save space
            for path in [r.img1_path, r.img2_path]:
                if path:
                    full_path = os.path.join(storage_root, path)
                    if os.path.exists(full_path):
                        try: os.remove(full_path)
                        except: pass
            
            # Delete database record
            db.delete(r)
            
        db.commit()
        return {"status": "ok", "count": len(reports)}
    except Exception as e:
        print(f"[SVC] Bulk Delete Error: {e}")
        return {"status": "error", "msg": str(e)}


@router.get("/station/{stn_id}")
async def station_detail(stn_id: str, request: Request, db: Session = Depends(get_db)):
    """Full history page with de-cluttered daily trends."""
    try:
        # v5.87: Normalize stn_id for consistent querying (001952 -> 1952)
        if stn_id.isdigit():
            stn_id = str(int(stn_id))
        stn_id = stn_id.upper()

        raw_history = (
            db.query(HealthReport)
            .filter_by(stn_id=stn_id)
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
            .filter_by(stn_id=stn_id)
            .order_by(CommandQueue.created_at.desc())
            .limit(10)
            .all()
        )
        
        # v5.87: Fetch recent service        # v6.4: Limit Service History to exactly 5 entries
        service_history = (
            db.query(ServiceReport).filter(ServiceReport.is_finalized == True)
            .filter_by(stn_id=stn_id)
            .order_by(ServiceReport.reported_at.desc())
            .limit(5)
            .all()
        )
        
        # IST conversion is now handled exclusively by the |ist template filter for robustness
        # v5.88: GPS Deep-Lookup Fallback (Phase 2 Recovery)
        # If the settings cache has no GPS (common after transmission failures),
        # peek into historical records to recover the last known location.
        if settings and (not settings.last_gps or settings.last_gps in ('0,0', '0.000000,0.000000', 'NA', 'None', '')):
            deep_gps = (
                db.query(HealthReport.gps)
                .filter(HealthReport.stn_id == stn_id)
                .filter(HealthReport.gps != None)
                .filter(HealthReport.gps != "0,0")
                .filter(HealthReport.gps != "0.000000,0.000000")
                .filter(HealthReport.gps != "NA")
                .order_by(HealthReport.reported_at.desc())
                .first()
            )
            if deep_gps:
                settings.last_gps = deep_gps[0]
                db.commit() # Self-healing update for the cache

        is_exempt = (settings.ota_exempt == 1) if settings else False

        
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
        settings = db.query(StationSettings).filter_by(stn_id=stn_id).first()

        # v5.86 FIX: UI Resilience. If the primary GPS cache is empty/invalid, 
        # scan historical records to find the last known "Good" coordinate.
        if settings and (not settings.last_gps or settings.last_gps in ("NA", "0,0", "0.000000,0.000000", "None")):
            # 1. Search local 400 reports we already have
            for r in raw_history:
                if r.gps and str(r.gps).strip() not in ("NA", "0,0", "0.000000,0.000000", "None", ""):
                    settings.last_gps = r.gps
                    db.commit()
                    break
            # 2. Deep scan if still not found
            if not settings.last_gps or settings.last_gps in ("NA", "0,0", "0.000000,0.000000", "None"):
                deep_gps = (
                    db.query(HealthReport.gps)
                    .filter(HealthReport.stn_id == stn_id)
                    .filter(HealthReport.gps != None)
                    .filter(HealthReport.gps != "0,0")
                    .filter(HealthReport.gps != "0.000000,0.000000")
                    .filter(HealthReport.gps != "NA")
                    .order_by(HealthReport.reported_at.desc())
                    .first()
                )
                if deep_gps:
                    settings.last_gps = deep_gps[0]
                    db.commit()

        # v5.90: Fetch last SET_WIFI_PASS command for supervisor display
        last_wifi_cmd = (
            db.query(CommandQueue)
            .filter_by(stn_id=stn_id, cmd="SET_WIFI_PASS")
            .order_by(CommandQueue.created_at.desc())
            .first()
        )
        last_wifi_pass = last_wifi_cmd.cmd_param if last_wifi_cmd else None

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
                "service_history": service_history
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
    history = (
        db.query(HealthReport)
        .filter_by(stn_id=stn_id)
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
    history = (
        db.query(HealthReport)
        .filter_by(stn_id=stn_id)
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
