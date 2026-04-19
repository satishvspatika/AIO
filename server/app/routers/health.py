from fastapi import APIRouter, Request, Depends, Response
from sqlalchemy.orm import Session
from sqlalchemy import text, inspect as sa_inspect
from app.database import SessionLocal
from app.models import HealthReport, FirmwareRegistry, CommandQueue, StationSettings
from app.services.ota_service import needs_ota
import datetime, json, re, os, traceback

# Define IST timezone explicitly to prevent "name 'ist_tz' is not defined" error
ist_tz = datetime.timezone(datetime.timedelta(hours=5, minutes=30))

router = APIRouter()

TELEMETRY_TOKEN = os.getenv("TELEMETRY_TOKEN", "spatika_telemetry_2025")

BUILDS_DIR = "/app/builds"

# Whitelist: only safe column names (alphanumeric + underscore)
_SAFE_COL = re.compile(r'^[a-z][a-z0-9_]{0,63}$')

# Fields never treated as data columns in HealthReport model
_SKIP_FIELDS = {"id", "reported_at"}

# Phase 6 Fix: Strict whitelist outlaws Database Schema Poisoning
_ALLOWED_FIELDS = {
    "stn_id", "unit_type", "system", "ver", "ota_fails", "bat_v", "sol_v", 
    "rtc_ok", "reset_reason", "signal", "reg_fails", "http_fails", "net_cnt", 
    "net_cnt_prev", "prev_stored", "http_suc_cnt", "http_suc_cnt_prev", 
    "http_ret_cnt", "http_ret_cnt_prev", "ftp_suc_cnt", "ftp_suc_cnt_prev", 
    "ndm_cnt", "pd_cnt", "first_http", "spiffs_kb", "spiffs_total_kb", 
    "consec_reg_fails", "consec_http_fails", "consec_sim_fails", "unsent_count", 
    "http_present_fails", "http_cum_fails", "http_backlog_cnt", "last_cmd_id", 
    "mutex_fail", "calib", "gps", "carrier", "iccid", "last_cmd_res", "net_nuke",
    "dbg_tsk", "last_rst", "health_sts", "sensor_sts", "reg_fail_reason", 
    "cdm_sts", "ota_fail_reason"
}

# Known type hints — anything not listed defaults to TEXT
_INT_FIELDS  = {
    "system","reset_reason","rtc_ok","signal","reg_fails",
    "http_fails","net_cnt","net_cnt_prev","prev_stored",
    "http_suc_cnt","http_suc_cnt_prev","http_ret_cnt","http_ret_cnt_prev",
    "ftp_suc_cnt","ftp_suc_cnt_prev","ndm_cnt","pd_cnt","first_http",
    "spiffs_kb","spiffs_total_kb","ota_fails",
    "consec_reg_fails","consec_http_fails","consec_sim_fails",
    "unsent_count",
    "http_present_fails","http_cum_fails","http_backlog_cnt", # v7.70
    "last_cmd_id",                                      # v7.92
    "mutex_fail"                                        # v5.55
}
_FLOAT_FIELDS = {"bat_v","sol_v"}


def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()


def _infer_sql_type(key: str, val) -> str:
    """Infer SQLite column type from key name + value type."""
    if key in _INT_FIELDS:
        return "INTEGER"
    if key in _FLOAT_FIELDS or isinstance(val, float):
        return "REAL"
    if isinstance(val, int):
        return "INTEGER"
    return "TEXT"


def get_carrier_from_iccid(iccid: str) -> str:
    """Deciphers Indian carrier from ICCID prefix as a fallback."""
    if not iccid or len(iccid) < 6:
        return "Unknown"
    # Common Indian Issuer Identifiers (IIN)
    if iccid.startswith(("899116", "899110")): return "Airtel"
    if iccid.startswith("899100"): return "BSNL"
    if iccid.startswith("899184"): return "Jio"
    if iccid.startswith("899111"): return "Vi"
    return "Unknown"



def _get_db_columns(db: Session, table: str) -> set:
    """Returns the set of column names currently in the table."""
    inspector = sa_inspect(db.bind)
    return {col["name"] for col in inspector.get_columns(table)}


def _auto_migrate(db: Session, data: dict, table: str = "health_reports"):
    """
    For each key in `data` that does not yet exist as a column in `table`,
    auto-add the column via ALTER TABLE.
    """
    # Phase 6 Fix: Avoid python-level caching which fractures across ASGI workers.
    # Querying SQLite directly is thread-safe and guarantees schema parity.
    existing_cols = _get_db_columns(db, table)
        
    for key, val in data.items():
        if key in _SKIP_FIELDS or key in existing_cols:
            continue
        if not _SAFE_COL.match(key) or key not in _ALLOWED_FIELDS:
            print(f"[AutoMigrate] Blocked unwhitelisted arbitrary column: {key!r}")
            continue
        col_type = _infer_sql_type(key, val)
        default  = "0" if col_type in ("INTEGER","REAL") else "''"
        try:
            db.execute(text(f"ALTER TABLE {table} ADD COLUMN {key} {col_type} DEFAULT {default}"))
            db.commit()
            existing_cols.add(key)
            print(f"[AutoMigrate] ✅ Added column '{key}' ({col_type}) to {table}")
        except Exception as e:
            if "duplicate" not in str(e).lower():
                print(f"[AutoMigrate] ⚠️  Could not add column '{key}': {e}")
            existing_cols.add(key)   # treat as existing to avoid retry loops
    return existing_cols

async def _process_health_data(data: dict, request: Request, db: Session):
    """Internal helper to process health data for any endpoint."""
    # v5.87: Clean Stn ID (Strip leading zeros for database consistency)
    stn_id = str(data.get("stn_id", "UNKNOWN")).strip()
    if stn_id.isdigit():
        stn_id = str(int(stn_id))
    stn_id = stn_id.upper()
    
    # ── Step 0: Security Verification ────────────────────────────────────
    # v5.87: Accept token from either header (Standard) or JSON Body (IoT Friendly)
    token = request.headers.get("X-Spatika-Token") or data.get("token")
    
    if token != TELEMETRY_TOKEN:
        if token:
            print(f"[AUTH WARN] {stn_id} sent INVALID TOKEN: {token}")
        else:
            # For now, allow legacy non-token reports to avoid field outages
            pass 
    
    # ── Step 0.1: De-duplication ─────────────────────────────────────────
    # Reject rapid-fire retries within 10 seconds to save DB space
    ten_secs_ago = datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None) - datetime.timedelta(seconds=10)
    duplicate = db.query(HealthReport).filter(
        HealthReport.stn_id == stn_id,
        HealthReport.reported_at > ten_secs_ago
    ).first()
    
    if duplicate:
        print(f"[DEDUP] Dropping duplicate from {stn_id}")
        return {"status":"ok", "cmd":"", "p":"", "id":0}

    unit_type = str(data.get("unit_type", "UNKNOWN"))
    sys_mode  = int(data.get("system", 0))
    ver       = str(data.get("ver", "5.00")).strip()
    ota_fails = int(data.get("ota_fails", 0))

    # ── Step 1: Schema Migration ─────────────────────────────────────────
    existing_cols = _auto_migrate(db, data)

    # ── Step 2: Build Report ─────────────────────────────────────────────
    now_utc = datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None)
    report_kwargs = {"stn_id": stn_id, "reported_at": now_utc}

    for key, val in data.items():
        if key in _SKIP_FIELDS or key not in existing_cols or not _SAFE_COL.match(key):
            continue
        try:
            if key in _INT_FIELDS:
                report_kwargs[key] = int(val) if val not in (None, "") else None
            elif key in _FLOAT_FIELDS or isinstance(val, float):
                report_kwargs[key] = float(val) if val not in (None, "") else None
            else:
                report_kwargs[key] = str(val) if val is not None else None
        except (ValueError, TypeError):
            report_kwargs[key] = None

    # Carrier & ICCID
    carrier = str(data.get("carrier", "")).strip().upper()
    iccid = str(data.get("iccid", "")).strip()
    if carrier in ("", "NA", "SIM OK", "UNKNOWN") or "SEARCH" in carrier:
        carrier = get_carrier_from_iccid(iccid)
    report_kwargs["carrier"] = carrier
    report_kwargs.setdefault("spiffs_total_kb", 4640)
    report_kwargs.setdefault("calib", "NA")

    db.add(HealthReport(**report_kwargs))

    # ── Step 3: State Caching (Phase 2 Hardening) ─────────────────────────
    # Upsert into StationSettings to provide an O(1) cache for Dashboard
    setting = db.query(StationSettings).filter_by(stn_id=stn_id).first()
    if not setting:
        setting = StationSettings(stn_id=stn_id)
        db.add(setting)
    
    setting.last_seen = now_utc
    gps_val = str(data.get("gps", "") or "").strip()
    is_zero = False
    if "," in gps_val:
        try:
            lat, lon = [float(x.strip()) for x in gps_val.split(",")]
            if abs(lat) < 0.0001 and abs(lon) < 0.0001:
                is_zero = True
        except: pass
    
    if gps_val and gps_val not in ("NA", "None", "") and not is_zero:
        setting.last_gps = gps_val

    # OTA Auto-Lock logic
    if ota_fails >= 3:
        setting.ota_exempt = 1
        print(f"[OTA LOCK] {stn_id} locked after {ota_fails} failures")

    # ── Step 4: Command Feedback & OTA ────────────────────────────────────
    cmd, cmd_param, cmd_id = "", "", 0
    pending = db.query(CommandQueue).filter_by(stn_id=stn_id, executed_at=None).first()
    if pending:
        cmd, cmd_param, cmd_id = pending.cmd, pending.cmd_param, pending.id
        pending.executed_at = now_utc
    else:
        setting = db.query(StationSettings).filter_by(stn_id=stn_id).first()
        if not (setting and setting.ota_exempt == 1):
            fw = db.query(FirmwareRegistry).filter_by(unit_type=unit_type, system_mode=sys_mode).first()
            if fw and needs_ota(ver, fw.current_ver):
                if os.path.exists(os.path.join(BUILDS_DIR, fw.filename)):
                    cmd, cmd_param = "OTA_CHECK", fw.filename

        # Priority commands if no OTA
        if not cmd:
            three_hrs_ago = datetime.datetime.utcnow() - datetime.timedelta(hours=3)
            timed_out = db.query(CommandQueue).filter(
                CommandQueue.stn_id == stn_id, CommandQueue.result == "SENT",
                CommandQueue.executed_at < three_hrs_ago
            ).first()
            if timed_out:
                cmd, cmd_param, cmd_id = timed_out.cmd, timed_out.cmd_param, timed_out.id
                timed_out.executed_at = now_utc

    db.commit()
    return {
        "status":"ok", "stored":True, "tm": now_utc.strftime("%y%m%d%H%M%S"),
        "cmd":cmd, "p":cmd_param, "id":cmd_id
    }

@router.post("/trg_gprs")
@router.post("/tws_gprs")
async def health_checkin(request: Request, db: Session = Depends(get_db)):
    try:
        data = await request.json()
        result = await _process_health_data(data, request, db)
        return Response(content=json.dumps(result), media_type="application/json")
    except Exception as e:
        traceback.print_exc()
        return Response(content=json.dumps({"status":"err","msg":str(e)}), status_code=500)

@router.post("/health")
async def legacy_health(request: Request, db: Session = Depends(get_db)):
    try:
        body = await request.body()
        try:
            data = json.loads(body)
        except:
            from urllib.parse import parse_qs
            data = {k: v[0] for k, v in parse_qs(body.decode()).items()}
        
        result = await _process_health_data(data, request, db)
        # Legacy response mapping
        result["sts"] = "OK" if result["status"] == "ok" else "ERR"
        return Response(content=json.dumps(result), media_type="application/json")
    except Exception as e:
        return Response(content=json.dumps({"sts":"ERR","msg":str(e)}), status_code=500)
