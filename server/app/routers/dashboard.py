from fastapi import APIRouter, Request, Depends
from fastapi.responses import StreamingResponse
from fastapi.templating import Jinja2Templates
from sqlalchemy.orm import Session
from sqlalchemy import func, desc
from app.database import SessionLocal
from app.models import HealthReport, FirmwareRegistry, CommandQueue, StationSettings
from app.services.health_eval import evaluate, ist_filter
import csv, io, datetime

router = APIRouter()
templates = Jinja2Templates(directory="app/templates")
templates.env.filters["ist"] = ist_filter


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



from app.services.ota_service import needs_ota
from app.services.health_eval import evaluate

@router.get("/dashboard")
async def dashboard(request: Request, db: Session = Depends(get_db)):
    try:
        reports = get_latest_per_station(db)
        fw_map  = {
            (fw.unit_type + str(fw.system_mode)): fw
            for fw in db.query(FirmwareRegistry).all()
        }
        
        now = datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None)

        # Summary card counts
        total       = len(reports)
        alarms      = 0
        low_bat     = 0
        ota_pending = 0

        settings_rows = db.query(StationSettings).all()
        exempt_map    = {s.stn_id: s.ota_exempt for s in settings_rows}

        for r in reports:
            key        = (r.unit_type or "") + str(r.system or 0)
            r.fw_group = fw_map.get(key)
            r.is_exempt = (exempt_map.get(r.stn_id, 0) == 1)

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
            # Order: BIHAR-TRG, KSNDMC-TRG, KSNDMC-TWS, KSNDMC-ADDON, SPATIKA-TRG, SPATIKA-ADDON
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


            # Pending command badge
            r.pending = db.query(CommandQueue).filter_by(
                stn_id=r.stn_id, executed_at=None
            ).first()

            # GPS Fallback: If latest report has no GPS (e.g. older firmware sends NA during non-GPS wakeups),
            # search history for the last known good coordinate.
            if not r.gps or str(r.gps).strip() in ("NA", "0.000000,0.000000", "", "0,0", "None"):
                last_good = db.query(HealthReport).filter(
                    HealthReport.stn_id == r.stn_id,
                    HealthReport.gps.isnot(None),
                    HealthReport.gps.notin_(("NA", "0.000000,0.000000", "", "0,0"))
                ).order_by(HealthReport.reported_at.desc()).first()
                if last_good:
                    r.gps = last_good.gps

        # Final Sort: priority, then station ID
        reports.sort(key=lambda x: (x.sort_priority, x.stn_id))

        return templates.TemplateResponse(request=request, name="dashboard.html", context= {

            "request": request, "reports": reports,
            "total": total, "alarms": alarms,
            "ota_pending": ota_pending, "low_bat": low_bat,
        })
    except Exception as e:
        print(f"CRITICAL 500 DASHBOARD ERROR: {e}")
        return templates.TemplateResponse(request=request, name="error.html", context= {"request": request}, status_code=500)


@router.get("/station/{stn_id}")
async def station_detail(stn_id: str, request: Request, db: Session = Depends(get_db)):
    """Full history page with de-cluttered daily trends."""
    try:
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
        
        # Trim to a reasonable view (e.g., last 40 entries/days)
        history = history[:40]

        commands = (
            db.query(CommandQueue)
            .filter_by(stn_id=stn_id)
            .order_by(CommandQueue.created_at.desc())
            .limit(10)
            .all()
        )
        
        setting = db.query(StationSettings).filter_by(stn_id=stn_id).first()
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

        return templates.TemplateResponse(
            request=request, name="station.html", context={
                "request": request,
                "stn_id": stn_id,
                "history": history,
                "commands": commands,
                "is_exempt": is_exempt,
                "category_id": category_id
            }
        )
    except Exception as e:
        print(f"CRITICAL 500 STATION ERROR: {e}")
        return templates.TemplateResponse(request=request, name="error.html", context= {"request": request}, status_code=500)


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
