from fastapi import APIRouter, Request, Form, File, UploadFile, Depends
from fastapi.responses import RedirectResponse
from fastapi.templating import Jinja2Templates
from sqlalchemy.orm import Session
from sqlalchemy import func
from app.database import SessionLocal
from app.models import FirmwareRegistry, HealthReport, CommandQueue
from app.services.health_eval import ist_filter
from app.services.ota_service import get_numeric_ver
import os, shutil

router    = APIRouter()
templates = Jinja2Templates(directory="app/templates")
templates.env.filters["ist"] = ist_filter
BUILDS_DIR = "/app/builds"


def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()


@router.get("/ota")
async def ota_page(request: Request, db: Session = Depends(get_db)):
    try:
        fws = db.query(FirmwareRegistry).order_by(FirmwareRegistry.category_id).all()
        for fw in fws:
            # Count stations in this group and how many are on the target version
            stations = (
                db.query(HealthReport.stn_id, func.max(HealthReport.reported_at), HealthReport.ver)
                .filter_by(unit_type=fw.unit_type, system=fw.system_mode)
                .group_by(HealthReport.stn_id)
                .all()
            )
            fw.total_stations = len(stations)
            fw.converted = sum(
                1 for s in stations
                if s[2] and get_numeric_ver(s[2]) >= get_numeric_ver(fw.current_ver)
            )
            # Check if file exists in builds folder
            dest = os.path.join(BUILDS_DIR, f"FW_S{fw.category_id}_{fw.unit_type}.bin")
            fw.file_exists = os.path.exists(dest)
        return templates.TemplateResponse("ota.html", {"request": request, "fws": fws})
    except Exception as e:
        return {"OTA Error": str(e)}


@router.post("/ota/upload/{cat_id}")
async def ota_upload(
    cat_id: int,
    ver:    str         = Form(...),
    file:   UploadFile  = File(None),
    db:     Session     = Depends(get_db)
):
    """Set a new target version for a firmware group and optionally upload the .bin."""
    fw = db.query(FirmwareRegistry).filter_by(category_id=cat_id).first()
    if fw:
        fw.current_ver = ver
        # Always set the canonical filename, even without an upload
        fw_filename = f"FW_S{cat_id}_{fw.unit_type}.bin"

        if file and file.filename:
            os.makedirs(BUILDS_DIR, exist_ok=True)
            dest = os.path.join(BUILDS_DIR, fw_filename)
            tmp  = dest + ".tmp"
            with open(tmp, "wb") as b:
                b.write(await file.read())
            shutil.move(tmp, dest)  # Atomic rename — safe even if upload fails mid-way
        
        fw.filename = fw_filename
        db.commit()
    return RedirectResponse(url="/ota", status_code=303)


@router.post("/ota/delete/{cat_id}")
async def ota_delete(
    cat_id: int,
    request: Request,
    db: Session = Depends(get_db)
):
    """Delete the target version and associated firmware bin file for a specific group."""
    if not hasattr(request.state, "user") or not request.state.user or request.state.user.get("role") != "supervisor":
        return RedirectResponse(url="/ota", status_code=303)
        
    fw = db.query(FirmwareRegistry).filter_by(category_id=cat_id).first()
    if fw:
        fw.current_ver = ""
        dest = os.path.join(BUILDS_DIR, f"FW_S{cat_id}_{fw.unit_type}.bin")
        if os.path.exists(dest):
            os.remove(dest)
        db.commit()
    return RedirectResponse(url="/ota", status_code=303)


@router.post("/station/{stn_id}/ota")
async def station_individual_ota(
    stn_id: str,
    ver:    str        = Form(...),
    file:   UploadFile = File(...),
    db:     Session    = Depends(get_db)
):
    """Upload a custom .bin targeted at ONE specific station only."""
    os.makedirs(BUILDS_DIR, exist_ok=True)
    filename = f"FW_CUSTOM_{stn_id}.bin"
    dest     = os.path.join(BUILDS_DIR, filename)
    tmp      = dest + ".tmp"
    with open(tmp, "wb") as b:
        b.write(await file.read())
    shutil.move(tmp, dest)
    # Queue an OTA command specifically for this station
    db.add(CommandQueue(stn_id=stn_id, cmd="OTA_CHECK", cmd_param=filename))
    db.commit()
    return RedirectResponse(url=f"/station/{stn_id}", status_code=303)
