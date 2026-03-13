from fastapi import APIRouter, Request, Depends, Response
from sqlalchemy.orm import Session
from sqlalchemy import text, inspect as sa_inspect
from app.database import SessionLocal
from app.models import HealthReport, FirmwareRegistry, CommandQueue, StationSettings
from app.services.ota_service import needs_ota
import datetime, json, re

router = APIRouter()

# Whitelist: only safe column names (alphanumeric + underscore)
_SAFE_COL = re.compile(r'^[a-z][a-z0-9_]{0,63}$')

# Fields never treated as data columns in HealthReport model
_SKIP_FIELDS = {"id", "reported_at"}

# Known type hints — anything not listed defaults to TEXT
_INT_FIELDS  = {
    "system","reset_reason","rtc_ok","signal","reg_fails",
    "http_fails","net_cnt","net_cnt_prev","prev_stored",
    "http_suc_cnt","http_suc_cnt_prev","http_ret_cnt","http_ret_cnt_prev",
    "ftp_suc_cnt","ftp_suc_cnt_prev","ndm_cnt","pd_cnt","first_http",
    "spiffs_kb","spiffs_total_kb","ota_fails",
    "consec_reg_fails","consec_http_fails","consec_sim_fails",
    "unsent_count",
    "http_present_fails","http_cum_fails",               # v7.70
    "last_cmd_id"                                       # v7.92
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


def _get_db_columns(db: Session, table: str) -> set:
    """Returns the set of column names currently in the table."""
    inspector = sa_inspect(db.bind)
    return {col["name"] for col in inspector.get_columns(table)}


def _auto_migrate(db: Session, data: dict, table: str = "health_reports"):
    """
    For each key in `data` that does not yet exist as a column in `table`,
    auto-add the column via ALTER TABLE.
    Returns the updated column set.
    """
    existing = _get_db_columns(db, table)
    for key, val in data.items():
        if key in _SKIP_FIELDS or key in existing:
            continue
        if not _SAFE_COL.match(key):
            print(f"[AutoMigrate] Skipping unsafe column name: {key!r}")
            continue
        col_type = _infer_sql_type(key, val)
        default  = "0" if col_type in ("INTEGER","REAL") else "''"
        try:
            db.execute(text(f"ALTER TABLE {table} ADD COLUMN {key} {col_type} DEFAULT {default}"))
            db.commit()
            existing.add(key)
            print(f"[AutoMigrate] ✅ Added column '{key}' ({col_type}) to {table}")
        except Exception as e:
            if "duplicate" not in str(e).lower():
                print(f"[AutoMigrate] ⚠️  Could not add column '{key}': {e}")
            existing.add(key)   # treat as existing to avoid retry loops
    return existing


@router.post("/health")
async def health(request: Request, db: Session = Depends(get_db)):
    try:
        body = await request.body()
        body_str = body.decode("utf-8")
        print(f"[Health] Raw Body: {body_str}")

        try:
            data = await request.json()
        except Exception:
            from urllib.parse import parse_qs
            data = {k: v[0] for k, v in parse_qs(body_str).items()}
            print(f"[Health] Fallback Parse: {data}")

        if not data:
            return Response(
                content='{"status":"err","msg":"empty body"}',
                media_type="application/json", status_code=400
            )

        stn_id    = str(data.get("stn_id", "UNKNOWN")).strip().upper()
        unit_type = str(data.get("unit_type", "UNKNOWN"))
        sys_mode  = int(data.get("system", 0))
        ver       = str(data.get("ver", "5.00")).strip()
        ota_fails = int(data.get("ota_fails", 0))

        # ── Step 1: Auto-migrate any new columns ─────────────────────────────
        existing_cols = _auto_migrate(db, data)

        # ── Step 2: Build a HealthReport from all matching fields ─────────────
        # v7.94: Store in UTC (Standard). Display filters will add +5:30 offset.
        now_ist = datetime.datetime.now(ist_tz)
        now_utc = datetime.datetime.now(datetime.timezone.utc).replace(tzinfo=None)

        report_kwargs = {"stn_id": stn_id, "reported_at": now_utc}

        for key, val in data.items():
            if key in _SKIP_FIELDS:
                continue
            if key not in existing_cols:
                continue
            if not _SAFE_COL.match(key):
                continue
            # Type-safe cast
            try:
                if key in _INT_FIELDS:
                    report_kwargs[key] = int(val) if val not in (None, "") else None
                elif key in _FLOAT_FIELDS or isinstance(val, float):
                    report_kwargs[key] = float(val) if val not in (None, "") else None
                else:
                    report_kwargs[key] = str(val) if val is not None else None
            except (ValueError, TypeError):
                report_kwargs[key] = None

        # Override stn_id to always be uppercased
        report_kwargs["stn_id"] = stn_id

        # Safe defaults for fields that firmware may not always send
        report_kwargs.setdefault("spiffs_total_kb", 4640)
        report_kwargs.setdefault("calib", "NA")

        report = HealthReport(**report_kwargs)
        db.add(report)

        # ── Step 2.5: Command Feedback Processing ────────────────────────────
        last_cmd_id = data.get("last_cmd_id")
        last_cmd_res = data.get("last_cmd_res", "N/A")
        if last_cmd_id and str(last_cmd_id).isdigit():
            cmd_id_int = int(last_cmd_id)
            if cmd_id_int > 0:
                cmd_entry = db.query(CommandQueue).filter_by(id=cmd_id_int).first()
                if cmd_entry:
                    cmd_entry.result = str(last_cmd_res)[:64]
                    cmd_entry.completed_at = now_ist
                    print(f"[CMD FEEDBACK] {stn_id} ID:{cmd_id_int} -> {cmd_entry.result}")

        # ── Step 3: Handle OTA Auto-Lock ──────────────────────────────────────
        if ota_fails >= 3:
            setting = db.query(StationSettings).filter_by(stn_id=stn_id).first()
            if not setting:
                setting = StationSettings(stn_id=stn_id, ota_exempt=1)
                db.add(setting)
                print(f"[OTA LOCK] New setting created for {stn_id}")
            elif setting.ota_exempt == 0:
                setting.ota_exempt = 1
                print(f"[OTA LOCK] {stn_id} locked after {ota_fails} failures")

        # ── Step 4: Command / OTA check ───────────────────────────────────────
        cmd, cmd_param = "", ""
        pending = db.query(CommandQueue).filter_by(stn_id=stn_id, executed_at=None).first()
        if pending:
            cmd       = pending.cmd
            cmd_param = pending.cmd_param
            cmd_id    = pending.id
            pending.executed_at = now_ist
            print(f"[CMD] {stn_id} → {cmd} (ID:{cmd_id})")
        else:
            cmd_id = 0
            # ── Auto-Command Priority Chain ──────────────────────────────────
            # Priority: Manual CMD > OTA_CHECK > CLEAR_FTP_QUEUE > GET_GPS
            setting  = db.query(StationSettings).filter_by(stn_id=stn_id).first()
            is_exempt = setting and setting.ota_exempt == 1

            if not is_exempt:
                fw = db.query(FirmwareRegistry).filter_by(
                    unit_type=unit_type, system_mode=sys_mode
                ).first()
                if fw and needs_ota(ver, fw.current_ver):
                    cmd       = "OTA_CHECK"
                    cmd_param = fw.filename
                    print(f"[OTA] {stn_id}: {ver} → {fw.current_ver}")
            else:
                print(f"[OTA] {stn_id} is EXEMPT from OTA.")

            # CLEAR_FTP_QUEUE: if backlog > 50 records, server triggers a clear
            if not cmd:
                unsent = int(data.get("unsent_count", 0) or 0)
                if unsent > 50:
                    cmd = "CLEAR_FTP_QUEUE"
                    print(f"[CMD] {stn_id}: FTP backlog={unsent} > 50 → CLEAR_FTP_QUEUE")

            # GET_GPS: if GPS is missing and no higher-priority cmd is pending
            if not cmd:
                gps_val = str(data.get("gps", "") or "").strip()
                if gps_val in ("NA", "0.000000,0.000000", "", "0,0"):
                    cmd = "GET_GPS"
                    print(f"[CMD] {stn_id}: GPS missing ({gps_val!r}) → GET_GPS")


        db.commit()

        resp_data = {
            "status": "ok",
            "stored": True,
            "tm": now_utc.strftime("%y%m%d%H%M%S"),
            "cmd": cmd,
            "p": cmd_param,
            "id": cmd_id
        }
        content = json.dumps(resp_data)
        return Response(
            content=content,
            media_type="application/json",
            headers={"Content-Length": str(len(content))}
        )

    except Exception as e:
        import traceback
        traceback.print_exc()
        content = json.dumps({"status": "err", "msg": str(e)[:80], "stored": False})
        return Response(
            content=content,
            media_type="application/json",
            headers={"Content-Length": str(len(content))}
        )
