from fastapi import APIRouter, Request, Depends
from fastapi.templating import Jinja2Templates
from fastapi.responses import StreamingResponse
from sqlalchemy.orm import Session
from sqlalchemy import func
from app.database import SessionLocal
from app.models import HealthReport
import datetime, csv, io

router = APIRouter()
templates = Jinja2Templates(directory="app/templates")


def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()


def _get_latest_per_station(db: Session):
    """Returns one full HealthReport ORM object per station (the latest)."""
    subq = db.query(
        HealthReport.stn_id,
        func.max(HealthReport.reported_at).label("m")
    ).group_by(HealthReport.stn_id).subquery()

    return db.query(HealthReport).join(
        subq,
        (HealthReport.stn_id == subq.c.stn_id) &
        (HealthReport.reported_at == subq.c.m)
    ).order_by(HealthReport.stn_id).all()


def _safe_int(val, default=0):
    try:
        return int(val) if val is not None else default
    except (TypeError, ValueError):
        return default


def _safe_str(val, default="—"):
    return str(val).strip() if val else default


def _time_ago(reported_at):
    if not reported_at:
        return "?"
    mins = int((datetime.datetime.now() - reported_at).total_seconds() / 60)
    if mins < 60:
        return f"{mins}m ago"
    hours = mins // 60
    rem = mins % 60
    return f"{hours}h {rem}m ago"


# ── Page Route ────────────────────────────────────────────────────────────────

@router.get("/diagnostics")
async def diagnostics_page(request: Request, db: Session = Depends(get_db)):
    """Hardware deep-dive diagnostics — one row per station, latest report."""
    try:
        reports = _get_latest_per_station(db)
        rows = []
        for r in reports:
            rows.append({
                "stn_id":       r.stn_id,
                "time_ago":     _time_ago(r.reported_at),
                "ver":          _safe_str(r.ver),
                "carrier":      _safe_str(r.carrier),
                "net_type":     _safe_str(getattr(r, "net_type", None), "N/A"),
                "m_temp":       _safe_int(getattr(r, "m_temp", None)),
                "min_rssi":     _safe_int(getattr(r, "min_rssi", None)),
                "max_rssi":     _safe_int(getattr(r, "max_rssi", None)),
                "i2c_errs":     _safe_int(getattr(r, "i2c_errs", None)),
                "stack_rtc":    _safe_int(getattr(r, "stack_rtc", None)),
                "stack_sched":  _safe_int(getattr(r, "stack_sched", None)),
                "stack_gprs":   _safe_int(getattr(r, "stack_gprs", None)),
                "stack_ui":     _safe_int(getattr(r, "stack_ui", None)),
                "free_heap_kb": _safe_int(r.free_heap_kb),
                "uptime_hrs":   _safe_int(r.uptime_hrs),
            })

        return templates.TemplateResponse("diagnostics.html", {
            "request": request,
            "reports": rows,
            "active":  "diagnostics",
        })
    except Exception as e:
        import traceback; traceback.print_exc()
        return {"Diagnostics Error": str(e)}


# ── CSV Download Route ────────────────────────────────────────────────────────

DIAG_CSV_HEADER = [
    "Station_ID", "Reported_At", "Firmware_Ver", "Carrier", "Network_Type",
    "Modem_Temp_C", "Min_RSSI_dBm", "Max_RSSI_dBm",
    "I2C_Errors",
    "Stack_Min_RTC_bytes", "Stack_Min_Sched_bytes",
    "Stack_Min_GPRS_bytes", "Stack_Min_UI_bytes",
    "Free_Heap_KB", "Uptime_Hrs",
]


def _diag_csv_row(r):
    return [
        r.stn_id,
        r.reported_at.strftime("%Y-%m-%d %H:%M:%S") if r.reported_at else "",
        _safe_str(r.ver),
        _safe_str(r.carrier),
        _safe_str(getattr(r, "net_type", None), "N/A"),
        _safe_int(getattr(r, "m_temp", None)),
        _safe_int(getattr(r, "min_rssi", None)),
        _safe_int(getattr(r, "max_rssi", None)),
        _safe_int(getattr(r, "i2c_errs", None)),
        _safe_int(getattr(r, "stack_rtc", None)),
        _safe_int(getattr(r, "stack_sched", None)),
        _safe_int(getattr(r, "stack_gprs", None)),
        _safe_int(getattr(r, "stack_ui", None)),
        _safe_int(r.free_heap_kb),
        _safe_int(r.uptime_hrs),
    ]


@router.get("/csv/diagnostics")
def csv_diagnostics_latest(db: Session = Depends(get_db)):
    """
    Diagnostics Summary CSV — one row per station (latest), diagnostic fields only.
    Clean export for reliability analysis without operational clutter.
    """
    reports = _get_latest_per_station(db)
    output = io.StringIO()
    writer = csv.writer(output)
    writer.writerow(DIAG_CSV_HEADER)
    for r in reports:
        writer.writerow(_diag_csv_row(r))
    output.seek(0)
    return StreamingResponse(
        output, media_type="text/csv",
        headers={"Content-Disposition": "attachment; filename=spatika_diagnostics_snapshot.csv"}
    )


@router.get("/csv/diagnostics/full")
def csv_diagnostics_full(db: Session = Depends(get_db)):
    """
    Diagnostics Full History CSV — every check-in for all stations, diagnostic fields only.
    Use for long-term trend analysis (stack drift, RSSI degradation over time).
    """
    records = (
        db.query(HealthReport)
        .order_by(HealthReport.stn_id, HealthReport.reported_at.desc())
        .all()
    )
    output = io.StringIO()
    writer = csv.writer(output)
    writer.writerow(DIAG_CSV_HEADER)
    for r in records:
        writer.writerow(_diag_csv_row(r))
    output.seek(0)
    return StreamingResponse(
        output, media_type="text/csv",
        headers={"Content-Disposition": "attachment; filename=spatika_diagnostics_full_history.csv"}
    )
