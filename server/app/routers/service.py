from fastapi import APIRouter, Request, Depends, HTTPException, Response, File, Form, UploadFile
from fastapi.responses import FileResponse
from sqlalchemy.orm import Session
from app.database import SessionLocal
from app.models import ServiceReport
import os, base64, datetime, uuid, json

router = APIRouter(prefix="/api/v2/service_report")

# Storage for service photos
SERVICE_LOGS_DIR = "/app/data/service_logs"

def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()

@router.post("")
async def receive_service_report(
    request: Request, 
    db: Session = Depends(get_db),
    # Optional fields for multipart support
    eng: str = Form(None),
    type_str: str = Form(None),
    comments: str = Form(None),
    stn: str = Form(None),
    bat: float = Form(None),
    sol: float = Form(None),
    gps: str = Form(None),
    inst_rf: float = Form(None),
    cum_rf: float = Form(None),
    ts: int = Form(None),
    img1: UploadFile = File(None),
    img2: UploadFile = File(None)
):
    """Receives Geo-tagged service reports from AIO9 stations (Supports JSON and Multipart)."""
    try:
        data = {}
        content_type = request.headers.get("content-type", "")
        
        if "application/json" in content_type:
            data = await request.json()
        else:
            # Fallback to Form Data
            data = {
                "stn": stn, "eng": eng, "type_str": type_str, "com": comments,
                "bat": bat, "sol": sol, "gps": gps, "inst_rf": inst_rf, "cum_rf": cum_rf, "ts": ts
            }
            # v5.96: New logic - If the station bundled JSON into a part named 'json'
            json_part = data.get("json")
            if json_part:
                try:
                    if isinstance(json_part, (str, bytes)):
                        ext_json = json.loads(json_part)
                        data.update(ext_json)
                except: pass

        stn_id = str(data.get("stn") or data.get("stn_id") or "UNKNOWN").strip().upper()
        if stn_id.isdigit(): stn_id = str(int(stn_id))
            
        print(f"[SVC] Processing report for Stn {stn_id} (CT: {content_type})")

        # Type-safe Field Robustness
        def s_int(val, default=0):
            try: return int(float(val)) if val is not None else default
            except: return default

        comments_val = data.get("comments") or data.get("com") or data.get("comments") or "No comments."
        eng_val = data.get("eng") or data.get("eng_name") or "Unknown"
        svc_val = data.get("type_str") or data.get("svc_type") or "General"
        
        # Robust Timestamp Parsing (Handle both string and int from Form/JSON)
        ts_val = s_int(data.get("ts") or data.get("reported_at"))
        if ts_val <= 0: ts_val = int(datetime.datetime.now().timestamp())
        
        stn_dir = os.path.join(SERVICE_LOGS_DIR, stn_id)
        os.makedirs(stn_dir, exist_ok=True)
        
        paths = [None, None]
        
        # Handle Images (Binary Multipart or Base64 JSON)
        for i, img_obj in enumerate([img1, img2], 1):
            filename = f"{ts_val}_{i}.jpg"
            filepath = os.path.join(stn_dir, filename)
            
            if img_obj: # Multipart Binary
                try:
                    content = await img_obj.read()
                    if content and len(content) > 100:
                        with open(filepath, "wb") as f:
                            f.write(content)
                        paths[i-1] = f"{stn_id}/{filename}"
                except Exception as e:
                    print(f"[SVC] [WARN] Partial upload failure on img{i}: {e}")
                    if os.path.exists(filepath): os.remove(filepath)
            else: # Check for Base64 in JSON
                b64_str = data.get(f"img{i}")
                if b64_str and len(b64_str) > 100:
                    if "," in b64_str: b64_str = b64_str.split(",")[1]
                    with open(filepath, "wb") as f:
                        f.write(base64.b64decode(b64_str))
                    paths[i-1] = f"{stn_id}/{filename}"
        
        # v6.0 Deep Dive: Use Upsert logic to support Split-Atomic sync (Text first, then Images)
        # Search for an existing report from the same station within a 10-minute window
        existing_report = db.query(ServiceReport).filter(
            ServiceReport.stn_id == stn_id,
            ServiceReport.reported_at >= datetime.datetime.fromtimestamp(ts_val - 600),
            ServiceReport.reported_at <= datetime.datetime.fromtimestamp(ts_val + 600)
        ).first()

        # v6.05: Binary Sync Integrity (Multi-stage Finalization)
        m_has_i1 = data.get("has_img1") in [True, "true", 1, "1", "True"]
        m_has_i2 = data.get("has_img2") in [True, "true", 1, "1", "True"]

        if existing_report:
            print(f"[SVC] Updating existing report for Stn {stn_id} (ID: {existing_report.id})")
            report = existing_report
            if eng_val != "Unknown": report.eng_name = eng_val
            if svc_val != "General": report.svc_type = svc_val
            if comments_val != "No comments.": report.comments = comments_val
            if paths[0]: report.img1_path = paths[0]
            if paths[1]: report.img2_path = paths[1]
            if m_has_i1: report.has_img1 = True
            if m_has_i2: report.has_img2 = True
            if data.get("bat"): report.bat_v = data.get("bat")
            if data.get("sol"): report.sol_v = data.get("sol")
            if data.get("gps"): report.gps = data.get("gps")
            if data.get("inst_rf"): report.inst_rf = data.get("inst_rf")
            if data.get("cum_rf"): report.cum_rf = data.get("cum_rf")
        else:
            # Create new report
            report = ServiceReport(
                stn_id=stn_id, eng_name=eng_val, svc_type=svc_val, comments=comments_val,
                img1_path=paths[0], img2_path=paths[1],
                has_img1=m_has_i1, has_img2=m_has_i2,
                bat_v=data.get("bat"), sol_v=data.get("sol"), gps=data.get("gps"),
                inst_rf=data.get("inst_rf"), cum_rf=data.get("cum_rf"),
                reported_at=datetime.datetime.fromtimestamp(ts_val)
            )
            db.add(report)

        # Finalization Logic: All or Nothing
        c1 = (not report.has_img1 or report.img1_path)
        c2 = (not report.has_img2 or report.img2_path)
        if c1 and c2:
            report.is_finalized = True
            print(f"[SVC] Report {report.id} finalized (All parts received)")

        db.commit()
        return {"status": "ok", "msg": "Report synchronized successfully", "id": report.id}
        
    except Exception as e:
        import traceback
        traceback.print_exc()
        raise HTTPException(status_code=500, detail=str(e))

@router.get("/image/{stn_id}/{filename}")
async def get_service_image(stn_id: str, filename: str):
    """Securely serves a service report image."""
    filepath = os.path.join(SERVICE_LOGS_DIR, stn_id, filename)
    if not os.path.exists(filepath):
        raise HTTPException(status_code=404, detail="Image not found")
    
    # Simple security: check if it's actually in our logs dir and is a jpg
    if ".." in stn_id or ".." in filename or not filename.endswith(".jpg"):
        raise HTTPException(status_code=403, detail="Forbidden")
        
    return FileResponse(filepath)
