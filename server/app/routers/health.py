from fastapi import APIRouter, Request, Depends, Response
from sqlalchemy.orm import Session
from app.database import SessionLocal
from app.models import HealthReport, FirmwareRegistry, CommandQueue, StationSettings
from app.services.ota_service import needs_ota
import datetime

router = APIRouter()


def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()


@router.post("/health")
async def health(request: Request, db: Session = Depends(get_db)):
    try:
        body = await request.body()
        body_str = body.decode("utf-8")
        print(f"[Health] Raw Body: {body_str}")
        
        try:
            data = await request.json()
        except Exception:
            # Fallback for form-urlencoded or malformed JSON
            from urllib.parse import parse_qs
            data = {k: v[0] for k, v in parse_qs(body_str).items()}
            print(f"[Health] Fallback Parse: {data}")

        if not data:
            return Response(content='{"status":"err","msg":"empty body"}', media_type="application/json", status_code=400)

        stn_id    = str(data.get("stn_id", "UNKNOWN")).strip().upper()
        unit_type = data.get("unit_type", "UNKNOWN")
        sys_mode  = int(data.get("system", 0))
        ver       = str(data.get("ver", "5.00")).strip()

        ota_fails   = int(data.get("ota_fails", 0))
        ota_reason  = str(data.get("ota_fail_reason", "NONE"))

        # Store the full health report
        report = HealthReport(
            stn_id=stn_id,           unit_type=unit_type,     system=sys_mode,
            health_sts=data.get("health_sts"),                 sensor_sts=data.get("sensor_sts"),
            reset_reason=data.get("reset_reason"),             rtc_ok=data.get("rtc_ok"),
            uptime_hrs=data.get("uptime_hrs"),                 bat_v=data.get("bat_v"),
            sol_v=data.get("sol_v"),                           signal=data.get("signal"),
            reg_fails=data.get("reg_fails"),                   reg_avg=data.get("reg_avg"),
            reg_worst=data.get("reg_worst"),                   reg_fail_type=data.get("reg_fail_type"),
            http_fails=data.get("http_fails"),                 http_fail_reason=data.get("http_fail_reason"),
            http_avg=data.get("http_avg"),                     net_cnt=data.get("net_cnt"),
            net_cnt_prev=data.get("net_cnt_prev"),             cur_stored=data.get("cur_stored"),
            prev_stored=data.get("prev_stored"),               http_suc_cnt=data.get("http_suc_cnt"),
            http_suc_cnt_prev=data.get("http_suc_cnt_prev"),   ndm_cnt=data.get("ndm_cnt"),
            http_ret_cnt=data.get("http_ret_cnt"),             http_ret_cnt_prev=data.get("http_ret_cnt_prev"),
            ftp_suc_cnt=data.get("ftp_suc_cnt"),               ftp_suc_cnt_prev=data.get("ftp_suc_cnt_prev"),
            pd_cnt=data.get("pd_cnt"),                         cdm_sts=data.get("cdm_sts"),
            spiffs_kb=data.get("spiffs_kb"),                   sd_sts=data.get("sd_sts"),
            ver=ver,                                           carrier=data.get("carrier"),
            iccid=data.get("iccid"),                           gps=data.get("gps"),
            ota_fails=ota_fails,                               ota_fail_reason=ota_reason,
            reported_at=datetime.datetime.now()
        )
        db.add(report)

        # Handle Auto-Lock for repeated OTA failures
        if ota_fails >= 3:
            setting = db.query(StationSettings).filter_by(stn_id=stn_id).first()
            if not setting:
                setting = StationSettings(stn_id=stn_id, ota_exempt=1)
                db.add(setting)
                print(f"[OTA LOCK] New setting created for {stn_id} (Persistent Failures)")
            elif setting.ota_exempt == 0:
                setting.ota_exempt = 1
                print(f"[OTA LOCK] Station {stn_id} locked due to {ota_fails} failures: {ota_reason}")

        # Manual commands take priority over OTA
        cmd, cmd_param = "", ""
        pending = db.query(CommandQueue).filter_by(stn_id=stn_id, executed_at=None).first()
        if pending:
            cmd       = pending.cmd
            cmd_param = pending.cmd_param
            pending.executed_at = datetime.datetime.now()
            print(f"[CMD] {stn_id} → {cmd} ({cmd_param})")
        else:
            # First, check if station is exempt from OTA
            setting = db.query(StationSettings).filter_by(stn_id=stn_id).first()
            is_exempt = setting and setting.ota_exempt == 1

            if not is_exempt:
                # OTA version mismatch check — must match both unit_type AND system_mode
                fw = db.query(FirmwareRegistry).filter_by(
                    unit_type=unit_type, system_mode=sys_mode
                ).first()
                if fw and needs_ota(ver, fw.current_ver):
                    cmd       = "OTA_CHECK"
                    cmd_param = fw.filename
                    print(f"[OTA] {stn_id}: {ver} → {fw.current_ver}")
            else:
                print(f"[OTA] {stn_id} is EXEMPT/LOCKED from further OTA updates.")

        db.commit()
        import json
        resp_data = {
            "status": "ok", 
            "stored": True,
            "tm": datetime.datetime.now().strftime("%y%m%d%H%M%S"),
            "cmd": cmd, 
            "p": cmd_param
        }
        content = json.dumps(resp_data)
        return Response(
            content=content, 
            media_type="application/json",
            headers={"Content-Length": str(len(content))}
        )

    except Exception as e:
        import traceback
        import json
        error_msg = str(e)
        print(f"[Health Error] {error_msg}")
        traceback.print_exc()
        resp_data = {
            "status": "err", 
            "msg": error_msg[:50],
            "stored": False
        }
        content = json.dumps(resp_data)
        return Response(
            content=content, 
            media_type="application/json",
            headers={"Content-Length": str(len(content))}
        )
