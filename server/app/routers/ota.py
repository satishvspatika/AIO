from fastapi import APIRouter, Request, Form, File, UploadFile, Depends
from fastapi.responses import RedirectResponse
from sqlalchemy.orm import Session
from sqlalchemy import func
from app.database import SessionLocal
from app.models import FirmwareRegistry, HealthReport, CommandQueue
from app.services.health_eval import ist_filter
from app.services.ota_service import get_numeric_ver
import os, shutil, re
from fastapi import HTTPException
from app.templates import templates
router = APIRouter()
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
        return templates.TemplateResponse(request=request, name="ota.html", context={"request": request, "fws": fws})
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
                size = 0
                while True:
                    chunk = await file.read(64 * 1024)
                    if not chunk:
                        break
                    size += len(chunk)
                    if size > 2.5 * 1024 * 1024:
                        b.close()
                        if os.path.exists(tmp): os.remove(tmp)
                        raise HTTPException(status_code=413, detail="Payload too large. Max 2.5MB strict ceiling.")
                    b.write(chunk)
            shutil.move(tmp, dest)  # Atomic rename — safe even if upload fails mid-way
            # Legacy Fallback: Copy to firmware.bin so legacy v6.21 DLs requesting default firmware.bin find it
            try:
                shutil.copy(dest, os.path.join(BUILDS_DIR, "firmware.bin"))
            except Exception:
                pass
        
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
    ver:    str               = Form(None),
    file:   UploadFile        = File(None),
    existing_filename: str    = Form(None),
    db:     Session           = Depends(get_db)
):
    """Deploy an existing server build OR upload a custom .bin for ONE station only."""
    if not re.match(r"^[a-zA-Z0-9_\-]+$", stn_id):
        raise HTTPException(status_code=400, detail="Path Traversal Blocked: Invalid characters in station ID")

    target_filename = None

    if file and file.filename:
        os.makedirs(BUILDS_DIR, exist_ok=True)
        filename = f"FW_CUSTOM_{stn_id}.bin"
        dest     = os.path.join(BUILDS_DIR, filename)
        tmp      = dest + ".tmp"
        
        with open(tmp, "wb") as b:
            size = 0
            while True:
                chunk = await file.read(64 * 1024)
                if not chunk:
                    break
                size += len(chunk)
                if size > 2.5 * 1024 * 1024:
                    b.close()
                    if os.path.exists(tmp): os.remove(tmp)
                    raise HTTPException(status_code=413, detail="Payload too large. Max 2.5MB strict ceiling.")
                b.write(chunk)
                
        shutil.move(tmp, dest)
        target_filename = filename
    elif existing_filename:
        clean_name = os.path.basename(existing_filename.strip())
        if clean_name.endswith(".bin") and os.path.exists(os.path.join(BUILDS_DIR, clean_name)):
            target_filename = clean_name
        else:
            raise HTTPException(status_code=400, detail="Selected firmware build file does not exist on server.")

    if target_filename:
        # Clear any old pending OTA commands for this station to avoid duplicate queueing
        db.query(CommandQueue).filter(
            CommandQueue.stn_id == stn_id,
            CommandQueue.cmd == "OTA_CHECK",
            CommandQueue.executed_at == None
        ).delete()
        db.add(CommandQueue(stn_id=stn_id, cmd="OTA_CHECK", cmd_param=target_filename))
        db.commit()

    return RedirectResponse(url=f"/station/{stn_id}", status_code=303)


@router.post("/ota/deploy_group/{cat_id}")
async def ota_deploy_group(
    cat_id: int,
    request: Request,
    db: Session = Depends(get_db)
):
    """Queue OTA_CHECK command explicitly for all stations in this group (Batch OTA)."""
    if not hasattr(request.state, "user") or not request.state.user:
        return RedirectResponse(url="/ota", status_code=303)

    fw = db.query(FirmwareRegistry).filter_by(category_id=cat_id).first()
    if fw and fw.filename:
        dest = os.path.join(BUILDS_DIR, fw.filename)
        if os.path.exists(dest):
            # Find all unique station IDs in this group from health reports
            stations = (
                db.query(HealthReport.stn_id)
                .filter_by(unit_type=fw.unit_type, system=fw.system_mode)
                .group_by(HealthReport.stn_id)
                .all()
            )
            for s in stations:
                stn_id = s[0]
                if stn_id:
                    db.query(CommandQueue).filter(
                        CommandQueue.stn_id == stn_id,
                        CommandQueue.cmd == "OTA_CHECK",
                        CommandQueue.executed_at == None
                    ).delete()
                    db.add(CommandQueue(stn_id=stn_id, cmd="OTA_CHECK", cmd_param=fw.filename))
            db.commit()
    return RedirectResponse(url="/ota", status_code=303)

