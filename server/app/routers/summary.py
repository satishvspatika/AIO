from fastapi import APIRouter, Request, Depends
from sqlalchemy.orm import Session
from sqlalchemy import func
from app.database import SessionLocal
from app.models import HealthReport, FirmwareRegistry
from app.services.health_eval import ist_filter, evaluate
from app.services.ota_service import get_numeric_ver
import datetime, os
from app.templates import templates
router = APIRouter()
BUILDS_DIR = "/app/builds"


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
        fws = db.query(FirmwareRegistry).all()
        
        # v5.49 & v7.90: Custom Group Sorting
        # Order: BIHAR-TRG, KSNDMC-TRG, KSNDMC-TWS, KSNDMC-ADDON, SPATIKA-TRG, SPATIKA-ADDON
        def get_priority(fw):
            ut = (fw.unit_type or "").upper()
            sys = fw.system_mode
            if "BIH" in ut and sys == 0: return 1
            if "DMC" in ut and sys == 0: return 2
            if "DMC" in ut and sys == 1: return 3
            if "DMC" in ut and sys == 2: return 4
            if "GEN" in ut and sys == 0: return 5
            if "GEN" in ut and sys == 2: return 6
            return 99 + fw.category_id

        fws.sort(key=get_priority)
        groups  = []

        for fw in fws:
            fw.sort_priority = get_priority(fw) # Pass to template
            # Check if actual file exists on disk

            fw.file_exists = os.path.exists(os.path.join(BUILDS_DIR, f"FW_S{fw.category_id}_{fw.unit_type}.bin"))

            # Get latest report per station within this group
            subq = (
                db.query(HealthReport.stn_id, func.max(HealthReport.reported_at).label("m"))
                .filter(HealthReport.unit_type == fw.unit_type,
                        HealthReport.system == fw.system_mode)
                .group_by(HealthReport.stn_id)
                .subquery()
            )
            latest_raw = (
                db.query(HealthReport)
                .join(subq, (HealthReport.stn_id == subq.c.stn_id) &
                      (HealthReport.reported_at == subq.c.m))
                .order_by(HealthReport.reported_at.desc())
                .all()
            )

            # v5.89: NUCLEAR DEDUPLICATION & COLLAPSE
            import re
            deduped = {}
            for r in latest_raw:
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
            
            latest_reports = list(deduped.values())
            # Sort by ID
            latest_reports.sort(key=lambda x: x.stn_id)

            now = datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None)
            for r in latest_reports:
                r.eval = evaluate(r, now) # Populate server evaluation
                if r.reported_at:
                    delta = now - r.reported_at
                    mins = int(delta.total_seconds() / 60)
                    r.time_ago = f"{mins}m ago" if mins < 60 else f"{mins // 60}h {mins % 60}m ago"
                else:
                    r.time_ago = "?"
                
                # Assign ota_needed for the template
                r.ota_needed = False
                if r.ver and fw.current_ver and fw.file_exists:
                    if get_numeric_ver(r.ver) < get_numeric_ver(fw.current_ver):
                        r.ota_needed = True

            total_seen = len(latest_reports)
            converted  = sum(
                1 for r in latest_reports
                if r.ver and get_numeric_ver(r.ver) >= get_numeric_ver(fw.current_ver)
            )
            # v7.90: Base health status on SERVER evaluation - essence same
            ok_count   = sum(1 for r in latest_reports if r.eval["verdict"] == "OK")
            fail_count = total_seen - ok_count
            low_bat    = sum(1 for r in latest_reports if "BATT" in str(r.eval["reasons"]))

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

        return templates.TemplateResponse(request=request, name="summary.html", context={
            "request": request,
            "groups":  groups,
        })
    except Exception as e:
        print(f"CRITICAL 500 SUMMARY ERROR: {e}")
        return templates.TemplateResponse(request=request, name="error.html", context={"error_msg": str(e)}, status_code=500)


@router.get("/help")
async def help_page(request: Request):
    """Static help and legend page."""
    return templates.TemplateResponse(request=request, name="help.html", context={"request": request})
