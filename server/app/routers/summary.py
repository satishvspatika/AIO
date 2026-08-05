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
        
        # Custom Group Sorting: KSNDMC first, Bihar second, Spatika third
        def get_priority(fw):
            ut = (fw.unit_type or "").upper()
            sys = fw.system_mode
            if "DMC" in ut and sys == 0: return 1  # KSNDMC-TRG
            if "DMC" in ut and sys == 1: return 2  # KSNDMC-TWS
            if "DMC" in ut and sys == 2: return 3  # KSNDMC-ADDON
            if "BIH" in ut: return 4               # BIHAR-TRG
            if "GEN" in ut and sys == 0: return 5  # SPATIKA-TRG
            if "GEN" in ut and sys == 2: return 6  # SPATIKA-ADDON
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

            settings_raw = db.query(StationSettings).all()
            settings_map = {}
            for s in settings_raw:
                if s.stn_id:
                    settings_map[s.stn_id] = s
                    s_norm = s.stn_id.lstrip('0') if s.stn_id.isdigit() else s.stn_id
                    if s_norm: settings_map[s_norm] = s
                    if s.stn_id.isdigit(): settings_map[s.stn_id.zfill(6)] = s

            latest_pause_cmds = db.query(CommandQueue).filter(
                CommandQueue.cmd.in_(["PAUSE_LIVE_POST", "PAUSE_TX", "PAUSE_KSNDMC", "RESUME_LIVE_POST", "RESUME_TX", "RESUME_KSNDMC"])
            ).order_by(CommandQueue.id.asc()).all()

            cmd_muted_stns = {}
            for c in latest_pause_cmds:
                norm = c.stn_id.lstrip('0') if c.stn_id and c.stn_id.isdigit() else c.stn_id
                if c.cmd in ("PAUSE_LIVE_POST", "PAUSE_TX", "PAUSE_KSNDMC"):
                    cmd_muted_stns[norm] = c.cmd_param if c.cmd_param else None
                    cmd_muted_stns[c.stn_id] = c.cmd_param if c.cmd_param else None
                else:
                    cmd_muted_stns.pop(norm, None)
                    cmd_muted_stns.pop(c.stn_id, None)

            now = datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None)
            for r in latest_reports:
                r.eval = evaluate(r, now) # Populate server evaluation
                if r.reported_at:
                    delta = now - r.reported_at
                    mins = int(delta.total_seconds() / 60)
                    r.time_ago = f"{mins}m ago" if mins < 60 else f"{mins // 60}h {mins % 60}m ago"
                else:
                    r.time_ago = "?"
                
                # Mute state & pause reason
                s_raw = str(r.stn_id or "").strip()
                s_norm = s_raw.lstrip('0') if s_raw.isdigit() else s_raw
                s_cache = settings_map.get(r.stn_id) or settings_map.get(s_norm)
                is_cmd_muted = s_norm in cmd_muted_stns or r.stn_id in cmd_muted_stns
                r.is_muted = False
                r.pause_reason = None
                if (s_cache and s_cache.muted == 1) or r.muted == 1 or 'MUTED' in (r.health_sts or '') or is_cmd_muted:
                    r.is_muted = True
                    r.pause_reason = (s_cache.pause_reason if (s_cache and s_cache.pause_reason) else cmd_muted_stns.get(s_norm) or cmd_muted_stns.get(r.stn_id))

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
            healthy_bat = sum(1 for r in latest_reports if r.bat_v and r.bat_v >= 3.8)
            marginal_bat = sum(1 for r in latest_reports if r.bat_v and 3.6 <= r.bat_v < 3.8)
            critical_bat = sum(1 for r in latest_reports if r.bat_v and r.bat_v < 3.6)
            low_bat      = critical_bat
            solar_active = sum(1 for r in latest_reports if r.sol_v and r.sol_v >= 1.2)
            weak_signal  = sum(1 for r in latest_reports if r.signal and r.signal < -100)

            carriers = {}
            for r in latest_reports:
                c = (r.carrier or "UNKNOWN").upper()
                if "AIRTEL" in c: c = "AIRTEL"
                elif "BSNL" in c: c = "BSNL"
                elif "JIO" in c: c = "JIO"
                elif "VI" in c or "VODA" in c: c = "VI"
                carriers[c] = carriers.get(c, 0) + 1

            groups.append({
                "fw":            fw,
                "stations":      latest_reports,
                "total_seen":    total_seen,
                "converted":     converted,
                "ok_count":      ok_count,
                "fail_count":    fail_count,
                "low_bat":       low_bat,
                "healthy_bat":   healthy_bat,
                "marginal_bat":  marginal_bat,
                "critical_bat":  critical_bat,
                "solar_active":  solar_active,
                "weak_signal":   weak_signal,
                "carriers":      carriers,
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
