from fastapi import APIRouter, Request, Depends
from fastapi.templating import Jinja2Templates
from sqlalchemy.orm import Session
from sqlalchemy import func
from app.database import SessionLocal
from app.models import HealthReport, FirmwareRegistry
from app.services.ota_service import get_numeric_ver

router = APIRouter()
templates = Jinja2Templates(directory="app/templates")


def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()


@router.get("/summary")
async def fleet_summary(request: Request, db: Session = Depends(get_db)):
    """
    Per-group fleet summary page.
    Shows each firmware group with:
      - Station count
      - Firmware conversion progress
      - Health breakdown (OK / FAIL counts)
      - Last active station in each group
    """
    try:
        fws     = db.query(FirmwareRegistry).order_by(FirmwareRegistry.category_id).all()
        groups  = []

        for fw in fws:
            # Get latest report per station within this group
            subq = (
                db.query(HealthReport.stn_id, func.max(HealthReport.reported_at).label("m"))
                .filter(HealthReport.unit_type == fw.unit_type,
                        HealthReport.system == fw.system_mode)
                .group_by(HealthReport.stn_id)
                .subquery()
            )
            latest_reports = (
                db.query(HealthReport)
                .join(subq, (HealthReport.stn_id == subq.c.stn_id) &
                      (HealthReport.reported_at == subq.c.m))
                .order_by(HealthReport.reported_at.desc())
                .all()
            )

            total_seen = len(latest_reports)
            converted  = sum(
                1 for r in latest_reports
                if r.ver and get_numeric_ver(r.ver) == get_numeric_ver(fw.current_ver)
            )
            ok_count   = sum(1 for r in latest_reports if r.health_sts == "OK")
            fail_count = total_seen - ok_count
            low_bat    = sum(1 for r in latest_reports if r.bat_v and r.bat_v < 3.6)

            groups.append({
                "fw":            fw,
                "stations":      latest_reports,
                "total_seen":    total_seen,
                "converted":     converted,
                "ok_count":      ok_count,
                "fail_count":    fail_count,
                "low_bat":       low_bat,
                "pct":           int((converted / total_seen * 100)) if total_seen > 0 else 0,
            })

        return templates.TemplateResponse("summary.html", {
            "request": request,
            "groups":  groups,
        })
    except Exception as e:
        return {"Summary Error": str(e)}


@router.get("/help")
async def help_page(request: Request):
    """Static help and legend page."""
    return templates.TemplateResponse("help.html", {"request": request})
