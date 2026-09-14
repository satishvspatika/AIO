from fastapi import APIRouter, Depends, Request
from fastapi.responses import RedirectResponse
from sqlalchemy.orm import Session
from app.database import SessionLocal
from app.models import CommandQueue, StationSettings, HealthReport
from pydantic import BaseModel
from typing import List

router = APIRouter()


def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()


@router.api_route("/cmd/{stn_id}/{command}", methods=["GET", "POST"])
def queue_command(
    request: Request,
    stn_id:  str,
    command: str,
    param:   str = "",
    db:      Session = Depends(get_db)
):
    """
    Queue a remote command for a station.
    It will be piggybacked on the station's next health check-in response.
    """
    import datetime
    now_utc = datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None)
    s_raw = str(stn_id).strip()
    norm_stn = s_raw.lstrip('0') if s_raw.isdigit() else s_raw
    if not norm_stn: norm_stn = "0"
    
    # Check if reason was passed in query params
    q_reason = request.query_params.get("reason", "").strip()
    if q_reason and not param:
        param = q_reason

    db.add(CommandQueue(stn_id=norm_stn, cmd=command, cmd_param=param))

    # Instant UI state reflection in StationSettings across all ID variants
    target_ids = {s_raw, norm_stn, s_raw.zfill(6)} if s_raw.isdigit() else {s_raw}
    settings_list = db.query(StationSettings).filter(StationSettings.stn_id.in_(list(target_ids))).all()
    if not settings_list:
        s_new = StationSettings(stn_id=norm_stn)
        db.add(s_new)
        settings_list = [s_new]

    for setting in settings_list:
        if command in ("PAUSE_LIVE_POST", "PAUSE_TX", "PAUSE_KSNDMC"):
            setting.muted = 1
            setting.paused_at = now_utc
            setting.pause_reason = param if param else "Manual Pause"
        elif command in ("RESUME_LIVE_POST", "RESUME_TX", "RESUME_KSNDMC"):
            setting.muted = 0
            setting.paused_at = None
            setting.pause_reason = None

    db.commit()

    referer = request.headers.get("referer", "")
    if referer and "/summary" in referer:
        return RedirectResponse(url="/summary", status_code=303)
    return RedirectResponse(url=f"/station/{stn_id}", status_code=303)


@router.api_route("/clear-queue/{stn_id}", methods=["GET", "POST"])
def clear_queue(stn_id: str, db: Session = Depends(get_db)):
    """Deletes all pending commands (like queued OTAs) for a station."""
    s_raw = str(stn_id).strip()
    target_ids = {s_raw, s_raw.lstrip('0'), s_raw.zfill(6)} if s_raw.isdigit() else {s_raw}
    db.query(CommandQueue).filter(CommandQueue.stn_id.in_(list(target_ids)), CommandQueue.executed_at == None).delete(synchronize_session=False)
    db.commit()
    return RedirectResponse(url=f"/station/{stn_id}", status_code=303)

@router.api_route("/clear-ota-queue/{stn_id}", methods=["GET", "POST"])
def clear_ota_queue(stn_id: str, db: Session = Depends(get_db)):
    """Deletes ONLY pending OTA_CHECK commands for a station."""
    s_raw = str(stn_id).strip()
    target_ids = {s_raw, s_raw.lstrip('0'), s_raw.zfill(6)} if s_raw.isdigit() else {s_raw}
    db.query(CommandQueue).filter(CommandQueue.stn_id.in_(list(target_ids)), CommandQueue.cmd == "OTA_CHECK", CommandQueue.executed_at == None).delete(synchronize_session=False)
    db.commit()
    return RedirectResponse(url=f"/station/{stn_id}", status_code=303)


@router.api_route("/toggle-ota-lock/{stn_id}", methods=["GET", "POST"])
def toggle_ota_lock(stn_id: str, db: Session = Depends(get_db)):
    from fastapi.responses import RedirectResponse
    setting = db.query(StationSettings).filter_by(stn_id=stn_id).first()
    if not setting:
        setting = StationSettings(stn_id=stn_id, ota_exempt=1)
        db.add(setting)
    else:
        setting.ota_exempt = 1 if setting.ota_exempt == 0 else 0
    
    # If we are locking it, clear any existing pending OTA_CHECK in the queue just in case
    if setting.ota_exempt == 1:
        db.query(CommandQueue).filter_by(stn_id=stn_id, cmd="OTA_CHECK", executed_at=None).delete()
        
    db.commit()
    return RedirectResponse(url=f"/station/{stn_id}", status_code=303)


class BulkDeleteRecords(BaseModel):
    ids: List[int]

class BulkDeleteStations(BaseModel):
    stn_ids: List[str]

@router.api_route("/delete/bulk-records", methods=["GET", "POST"])
def delete_bulk_records(payload: BulkDeleteRecords, db: Session = Depends(get_db)):
    try:
        if not payload.ids:
            return {"status": "ok", "deleted": 0}
        from app.models import StationSettings, HealthReport
        records = db.query(HealthReport).filter(HealthReport.id.in_(payload.ids)).all()
        stn_ids = {r.stn_id for r in records if r.stn_id}
        db.query(HealthReport).filter(HealthReport.id.in_(payload.ids)).delete(synchronize_session=False)
        db.commit()

        # Clean up orphan StationSettings for any station that now has 0 health reports
        for stn in stn_ids:
            s_raw = str(stn).strip()
            target_ids = {s_raw}
            if s_raw.isdigit():
                target_ids.add(s_raw.lstrip('0'))
                target_ids.add(s_raw.zfill(6))
            remaining = db.query(HealthReport).filter(HealthReport.stn_id.in_(list(target_ids))).count()
            if remaining == 0:
                db.query(StationSettings).filter(StationSettings.stn_id.in_(list(target_ids))).delete(synchronize_session=False)
        db.commit()
        return {"status": "ok", "deleted": len(payload.ids)}
    except Exception as e:
        db.rollback()
        print(f"BULK RECORD DELETE ERROR: {e}")
        from fastapi import HTTPException
        raise HTTPException(status_code=500, detail=f"Database error during record deletion: {str(e)}")

@router.api_route("/delete/bulk-stations", methods=["GET", "POST"])
def delete_bulk_stations(payload: BulkDeleteStations, db: Session = Depends(get_db)):
    try:
        from app.models import HealthReport, CommandQueue, StationSettings
        from sqlalchemy import text, bindparam, or_
        import re
        
        expanded_targets = set()
        include_null_or_empty = False

        for stn in payload.stn_ids:
            if stn is None:
                include_null_or_empty = True
                continue
            s_raw = str(stn).strip()
            if not s_raw or s_raw.upper() in ("UNKNOWN", "NONE", "NULL", "0", "N/A"):
                include_null_or_empty = True
                expanded_targets.update(["UNKNOWN", "unknown", "Unknown", "0", "N/A", "NA", "none", "NULL", "None", ""])
                continue
            
            s_clean = re.sub(r'[^A-Z0-9]', '', s_raw.upper())
            if not s_clean:
                include_null_or_empty = True
                continue
            
            expanded_targets.add(s_clean)
            expanded_targets.add(s_raw)
            expanded_targets.add(s_raw.lower())
            expanded_targets.add(s_raw.upper())
            norm = s_clean.lstrip('0')
            if not norm: norm = "0"
            expanded_targets.add(norm)
            expanded_targets.add(norm.zfill(6))

        if not expanded_targets and not include_null_or_empty:
            return {"status": "ok", "deleted": 0}

        target_list = list(expanded_targets)

        if include_null_or_empty:
            db.query(CommandQueue).filter(or_(CommandQueue.stn_id.in_(target_list), CommandQueue.stn_id == None, CommandQueue.stn_id == '', CommandQueue.stn_id == 'None')).delete(synchronize_session=False)
            db.query(HealthReport).filter(or_(HealthReport.stn_id.in_(target_list), HealthReport.stn_id == None, HealthReport.stn_id == '', HealthReport.stn_id == 'None')).delete(synchronize_session=False)
            db.query(StationSettings).filter(or_(StationSettings.stn_id.in_(target_list), StationSettings.stn_id == None, StationSettings.stn_id == '', StationSettings.stn_id == 'None')).delete(synchronize_session=False)
        else:
            db.query(CommandQueue).filter(CommandQueue.stn_id.in_(target_list)).delete(synchronize_session=False)
            db.query(HealthReport).filter(HealthReport.stn_id.in_(target_list)).delete(synchronize_session=False)
            db.query(StationSettings).filter(StationSettings.stn_id.in_(target_list)).delete(synchronize_session=False)

        db.commit()
        return {"status": "ok", "deleted": len(payload.stn_ids)}
    except Exception as e:
        db.rollback()
        print(f"BULK STATION DELETE ERROR: {e}")
        from fastapi import HTTPException
        raise HTTPException(status_code=500, detail=f"Database error during station wipe: {str(e)}")


@router.api_route("/delete/{stn_id}", methods=["GET", "POST"])
def delete_station(stn_id: str, db: Session = Depends(get_db)):
    from app.models import HealthReport, StationSettings, CommandQueue
    from sqlalchemy import text, bindparam
    import re
    s_raw = str(stn_id).strip()
    expanded_targets = set()
    include_null_or_empty = False

    if not s_raw or s_raw.upper() in ("UNKNOWN", "NONE", "NULL", "0", "N/A"):
        include_null_or_empty = True
        expanded_targets.update(["UNKNOWN", "unknown", "Unknown", "0", "N/A", "NA", "none", "NULL", "None", ""])
    else:
        s_clean = re.sub(r'[^A-Z0-9]', '', s_raw.upper())
        if s_clean: expanded_targets.add(s_clean)
        expanded_targets.add(s_raw)
        expanded_targets.add(s_raw.lower())
        norm = s_clean.lstrip('0') if s_clean else "0"
        if not norm: norm = "0"
        expanded_targets.add(norm)
        expanded_targets.add(norm.zfill(6))

    target_list = list(expanded_targets)
    from sqlalchemy import or_

    if include_null_or_empty:
        db.query(CommandQueue).filter(or_(CommandQueue.stn_id.in_(target_list), CommandQueue.stn_id == None, CommandQueue.stn_id == '', CommandQueue.stn_id == 'None')).delete(synchronize_session=False)
        db.query(HealthReport).filter(or_(HealthReport.stn_id.in_(target_list), HealthReport.stn_id == None, HealthReport.stn_id == '', HealthReport.stn_id == 'None')).delete(synchronize_session=False)
        db.query(StationSettings).filter(or_(StationSettings.stn_id.in_(target_list), StationSettings.stn_id == None, StationSettings.stn_id == '', StationSettings.stn_id == 'None')).delete(synchronize_session=False)
    else:
        db.query(CommandQueue).filter(CommandQueue.stn_id.in_(target_list)).delete(synchronize_session=False)
        db.query(HealthReport).filter(HealthReport.stn_id.in_(target_list)).delete(synchronize_session=False)
        db.query(StationSettings).filter(StationSettings.stn_id.in_(target_list)).delete(synchronize_session=False)

    db.commit()
    return RedirectResponse(url="/dashboard", status_code=303)

@router.api_route("/delete-category/{stn_id}/{unit_type}/{system}", methods=["GET", "POST"])
def delete_station_category(stn_id: str, unit_type: str, system: int, db: Session = Depends(get_db)):
    """Surgical delete: Removes a station only from a specific category (e.g. KSNDMC_TWS)."""
    from app.models import HealthReport
    db.query(HealthReport).filter_by(stn_id=stn_id, unit_type=unit_type, system=system).delete()
    db.commit()
    return RedirectResponse(url="/summary", status_code=303)

@router.api_route("/delete/history/{stn_id}", methods=["GET", "POST"])
def clear_station_history(stn_id: str, db: Session = Depends(get_db)):
    """Wipes all historical health check-in records, station settings, and command queue for a single station."""
    from app.models import HealthReport, StationSettings, CommandQueue
    from sqlalchemy import text, bindparam
    import re
    s_raw = str(stn_id).strip()
    expanded_targets = set()
    include_null_or_empty = False

    if not s_raw or s_raw.upper() in ("UNKNOWN", "NONE", "NULL", "0", "N/A"):
        include_null_or_empty = True
        expanded_targets.update(["UNKNOWN", "unknown", "Unknown", "0", "N/A", "NA", "none", "NULL", "None", ""])
    else:
        s_clean = re.sub(r'[^A-Z0-9]', '', s_raw.upper())
        if s_clean: expanded_targets.add(s_clean)
        expanded_targets.add(s_raw)
        expanded_targets.add(s_raw.lower())
        norm = s_clean.lstrip('0') if s_clean else "0"
        if not norm: norm = "0"
        expanded_targets.add(norm)
        expanded_targets.add(norm.zfill(6))

    target_list = list(expanded_targets)

    if include_null_or_empty:
        sql1 = text("DELETE FROM health_reports WHERE stn_id IN :targets OR stn_id IS NULL OR stn_id = '' OR stn_id = 'None'").bindparams(bindparam("targets", expanding=True))
        sql2 = text("DELETE FROM station_settings WHERE stn_id IN :targets OR stn_id IS NULL OR stn_id = '' OR stn_id = 'None'").bindparams(bindparam("targets", expanding=True))
        sql3 = text("DELETE FROM command_queue WHERE stn_id IN :targets OR stn_id IS NULL OR stn_id = '' OR stn_id = 'None'").bindparams(bindparam("targets", expanding=True))
    else:
        sql1 = text("DELETE FROM health_reports WHERE stn_id IN :targets").bindparams(bindparam("targets", expanding=True))
        sql2 = text("DELETE FROM station_settings WHERE stn_id IN :targets").bindparams(bindparam("targets", expanding=True))
        sql3 = text("DELETE FROM command_queue WHERE stn_id IN :targets").bindparams(bindparam("targets", expanding=True))

    db.execute(sql1, {"targets": target_list})
    db.execute(sql2, {"targets": target_list})
    db.execute(sql3, {"targets": target_list})
    db.commit()
    return RedirectResponse(url="/dashboard", status_code=303)


@router.api_route("/delete/command-history/{stn_id}", methods=["GET", "POST"])
def clear_command_history(stn_id: str, db: Session = Depends(get_db)):
    """Wipes all command history (both pending and executed) for a single station."""
    from app.models import CommandQueue
    s_raw = str(stn_id).strip()
    target_ids = {s_raw}
    if s_raw.isdigit():
        target_ids.add(s_raw.lstrip('0'))
        target_ids.add(s_raw.zfill(6))

    db.query(CommandQueue).filter(CommandQueue.stn_id.in_(list(target_ids))).delete(synchronize_session=False)
    db.commit()
    return RedirectResponse(url=f"/station/{stn_id}", status_code=303)


@router.api_route("/delete/record/{report_id}", methods=["GET", "POST"])
def delete_record(report_id: int, db: Session = Depends(get_db)):
    record = db.query(HealthReport).filter_by(id=report_id).first()
    if record:
        stn_id = record.stn_id
        db.delete(record)
        db.commit()
        
        # Check if any health records remain for this station
        s_raw = str(stn_id).strip()
        target_ids = {s_raw}
        if s_raw.isdigit():
            target_ids.add(s_raw.lstrip('0'))
            target_ids.add(s_raw.zfill(6))
        remaining = db.query(HealthReport).filter(HealthReport.stn_id.in_(list(target_ids))).count()
        if remaining == 0:
            db.query(StationSettings).filter(StationSettings.stn_id.in_(list(target_ids))).delete(synchronize_session=False)
            db.commit()
            return RedirectResponse(url="/dashboard", status_code=303)
        return RedirectResponse(url=f"/station/{stn_id}", status_code=303)
    return RedirectResponse(url="/dashboard", status_code=303)

class WifiPassPayload(BaseModel):
    password: str
    stn_ids: List[str] = []  # Empty = broadcast to all stations


@router.post("/cmd/fleet/set-wifi-pass")
def fleet_set_wifi_pass(payload: WifiPassPayload, db: Session = Depends(get_db)):
    """
    Queue a SET_WIFI_PASS command for one or more stations.
    If stn_ids is empty, broadcasts to ALL active stations in the fleet.
    Password must be 8-63 characters (WPA2 requirement).
    """
    if len(payload.password) < 8 or len(payload.password) > 63:
        return {"status": "error", "msg": "Password must be 8-63 characters (WPA2 requirement)."}

    if payload.stn_ids:
        targets = payload.stn_ids
    else:
        # Broadcast: fetch all unique station IDs from the fleet
        rows = db.query(HealthReport.stn_id).distinct().all()
        targets = [r.stn_id for r in rows]

    if not targets:
        return {"status": "error", "msg": "No stations found."}

    for stn in targets:
        db.add(CommandQueue(stn_id=stn, cmd="SET_WIFI_PASS", cmd_param=payload.password))

    db.commit()
    return {"status": "ok", "queued": len(targets), "targets": targets}

