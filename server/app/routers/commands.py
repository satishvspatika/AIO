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
    stn_id:  str,
    command: str,
    param:   str = "",
    db:      Session = Depends(get_db)
):
    """
    Queue a remote command for a station.
    It will be piggybacked on the station's next health check-in response.

    Supported commands:
        REBOOT       — Powers off GPRS then calls ESP.restart()
        FTP_BACKLOG  — Forces an immediate sync of unsent data files (unsent.txt)
        FTP_DAILY    — Forces upload of a specific daily file. 'param' must be YYYYMMDD
        OTA_CHECK    — (auto-set by OTA router)
        DELETE_DATA  — Deletes all data files on SPIFFS (Factory Reset)
    """
    db.add(CommandQueue(stn_id=stn_id, cmd=command, cmd_param=param))
    db.commit()
    return RedirectResponse(url=f"/station/{stn_id}")


@router.api_route("/clear-queue/{stn_id}", methods=["GET", "POST"])
def clear_queue(stn_id: str, db: Session = Depends(get_db)):
    """Deletes all pending commands (like queued OTAs) for a station."""
    db.query(CommandQueue).filter_by(stn_id=stn_id, executed_at=None).delete()
    db.commit()
    return RedirectResponse(url=f"/station/{stn_id}")

@router.api_route("/clear-ota-queue/{stn_id}", methods=["GET", "POST"])
def clear_ota_queue(stn_id: str, db: Session = Depends(get_db)):
    """Deletes ONLY pending OTA_CHECK commands for a station."""
    db.query(CommandQueue).filter_by(stn_id=stn_id, cmd="OTA_CHECK", executed_at=None).delete()
    db.commit()
    return RedirectResponse(url=f"/station/{stn_id}")


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
    return RedirectResponse(url=f"/station/{stn_id}")


@router.api_route("/delete/{stn_id}", methods=["GET", "POST"])
def delete_station(stn_id: str, db: Session = Depends(get_db)):
    from app.models import HealthReport, StationSettings
    db.query(HealthReport).filter_by(stn_id=stn_id).delete()
    db.query(CommandQueue).filter_by(stn_id=stn_id).delete()
    db.query(StationSettings).filter_by(stn_id=stn_id).delete()
    db.commit()
    return RedirectResponse(url="/dashboard")

@router.api_route("/delete-category/{stn_id}/{unit_type}/{system}", methods=["GET", "POST"])
def delete_station_category(stn_id: str, unit_type: str, system: int, db: Session = Depends(get_db)):
    """Surgical delete: Removes a station only from a specific category (e.g. KSNDMC_TWS)."""
    from app.models import HealthReport
    db.query(HealthReport).filter_by(stn_id=stn_id, unit_type=unit_type, system=system).delete()
    db.commit()
    return RedirectResponse(url="/summary")

@router.api_route("/delete/record/{report_id}", methods=["GET", "POST"])
def delete_record(report_id: int, db: Session = Depends(get_db)):
    record = db.query(HealthReport).filter_by(id=report_id).first()
    if record:
        stn_id = record.stn_id
        db.delete(record)
        db.commit()
        return RedirectResponse(url=f"/station/{stn_id}")
    return RedirectResponse(url="/dashboard")

class BulkDeleteRecords(BaseModel):
    ids: List[int]

class BulkDeleteStations(BaseModel):
    stn_ids: List[str]

@router.post("/delete/bulk-records")
def delete_bulk_records(payload: BulkDeleteRecords, db: Session = Depends(get_db)):
    db.query(HealthReport).filter(HealthReport.id.in_(payload.ids)).delete(synchronize_session=False)
    db.commit()
    return {"status": "ok", "deleted": len(payload.ids)}

@router.post("/delete/bulk-stations")
def delete_bulk_stations(payload: BulkDeleteStations, db: Session = Depends(get_db)):
    from app.models import HealthReport, CommandQueue, StationSettings
    db.query(HealthReport).filter(HealthReport.stn_id.in_(payload.stn_ids)).delete(synchronize_session=False)
    db.query(CommandQueue).filter(CommandQueue.stn_id.in_(payload.stn_ids)).delete(synchronize_session=False)
    db.query(StationSettings).filter(StationSettings.stn_id.in_(payload.stn_ids)).delete(synchronize_session=False)
    db.commit()
    return {"status": "ok", "deleted": len(payload.stn_ids)}


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

