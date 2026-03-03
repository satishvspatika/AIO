from fastapi import APIRouter, Request, Depends
from fastapi.responses import StreamingResponse
from fastapi.templating import Jinja2Templates
from sqlalchemy.orm import Session
from sqlalchemy import func, desc
from app.database import SessionLocal
from app.models import HealthReport, FirmwareRegistry, CommandQueue, StationSettings
import csv, io, datetime

router = APIRouter()
templates = Jinja2Templates(directory="app/templates")


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


def _all_fields_row(r):
    """Returns a flat list of all health report fields for CSV export."""
    return [
        r.reported_at, r.stn_id, r.unit_type, r.system,
        r.health_sts, r.sensor_sts,
        r.bat_v, r.sol_v, r.signal,
        r.net_cnt, r.net_cnt_prev,
        r.reg_fails, r.reg_avg, r.reg_worst, r.reg_fail_type,
        r.http_fails, r.http_fail_reason, r.http_avg,
        r.ndm_cnt, r.pd_cnt, r.cdm_sts, r.first_http,
        r.uptime_hrs, r.reset_reason, r.rtc_ok,
        r.spiffs_kb, r.spiffs_total_kb, r.sd_sts,
        r.calib, r.ver, r.carrier, r.iccid, r.gps,
        r.ota_fails, r.ota_fail_reason
    ]


ALL_FIELDS_HEADER = [
    "Timestamp_IST", "Station_ID", "Unit_Type", "System_Mode",
    "Health_Status", "Sensor_Status",
    "Battery_V", "Solar_V", "Signal_dBm",
    "Net_Count_Current", "Net_Count_PrevDay",
    "Reg_Fails", "Reg_Avg_ms", "Reg_Worst_ms", "Reg_Fail_Type",
    "HTTP_Fails", "HTTP_Fail_Reason", "HTTP_Avg_ms",
    "NDM_Count", "PD_Count", "CDM_Status", "First_HTTP_Count",
    "Uptime_Hours", "Reset_Reason_Code", "RTC_OK",
    "SPIFFS_Used_KB", "SPIFFS_Total_KB", "SD_Card_Status",
    "Calibration", "Firmware_Version", "Carrier", "ICCID", "GPS_LatLon",
    "OTA_Fails", "OTA_Fail_Reason"
]


from app.services.ota_service import needs_ota

@router.get("/dashboard")
async def dashboard(request: Request, db: Session = Depends(get_db)):
    try:
        reports = get_latest_per_station(db)
        fw_map  = {
            (fw.unit_type + str(fw.system_mode)): fw
            for fw in db.query(FirmwareRegistry).all()
        }
        now = datetime.datetime.now()

        # Summary card counts
        total       = len(reports)
        alarms      = sum(1 for r in reports if r.health_sts and r.health_sts != "OK")
        low_bat     = sum(1 for r in reports if r.bat_v and r.bat_v < 3.6)
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

        return templates.TemplateResponse("dashboard.html", {
            "request": request, "reports": reports,
            "total": total, "alarms": alarms,
            "ota_pending": ota_pending, "low_bat": low_bat,
        })
    except Exception as e:
        return {"Dashboard Error": str(e)}


@router.get("/station/{stn_id}")
async def station_detail(stn_id: str, request: Request, db: Session = Depends(get_db)):
    """Full history page for one station."""
    try:
        history = (
            db.query(HealthReport)
            .filter_by(stn_id=stn_id)
            .order_by(HealthReport.reported_at.desc())
            .limit(100)
            .all()
        )
        commands = (
            db.query(CommandQueue)
            .filter_by(stn_id=stn_id)
            .order_by(CommandQueue.created_at.desc())
            .limit(10)
            .all()
        )
        
        setting = db.query(StationSettings).filter_by(stn_id=stn_id).first()
        is_exempt = (setting.ota_exempt == 1) if setting else False

        return templates.TemplateResponse(
            "station.html", {
                "request": request,
                "stn_id": stn_id,
                "history": history,
                "commands": commands,
                "is_exempt": is_exempt
            }
        )
    except Exception as e:
        return {"Error": str(e)}


# ── CSV Downloads ─────────────────────────────────────────────────────────────

@router.get("/csv/summary")
def csv_summary(db: Session = Depends(get_db)):
    """
    Summary CSV: One row per station (latest report only), all fields.
    Good for sharing fleet status snapshots.
    """
    reports = get_latest_per_station(db)
    output  = io.StringIO()
    writer  = csv.writer(output)
    writer.writerow(ALL_FIELDS_HEADER)
    for r in reports:
        writer.writerow(_all_fields_row(r))
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
    output = io.StringIO()
    writer = csv.writer(output)
    writer.writerow(ALL_FIELDS_HEADER)
    for r in records:
        writer.writerow(_all_fields_row(r))
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
    output = io.StringIO()
    writer = csv.writer(output)
    writer.writerow(ALL_FIELDS_HEADER)
    for r in history:
        writer.writerow(_all_fields_row(r))
    output.seek(0)
    return StreamingResponse(
        output, media_type="text/csv",
        headers={"Content-Disposition": f"attachment; filename={stn_id}_history.csv"}
    )
