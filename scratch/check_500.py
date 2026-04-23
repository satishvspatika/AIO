from app.database import SessionLocal
from app.models import ServiceReport
from sqlalchemy import func
import datetime

db = SessionLocal()
try:
    subquery = db.query(func.max(ServiceReport.id)).group_by(ServiceReport.stn_id)
    reports = db.query(ServiceReport).filter(ServiceReport.id.in_(subquery)).order_by(ServiceReport.reported_at.desc()).limit(100).all()
    print(f"Fetched {len(reports)} reports successfully.")
    for r in reports:
        print(f"Report ID: {r.id}, Stn: {r.stn_id}, Time: {r.reported_at}")
except Exception as e:
    print(f"CRASH: {e}")
finally:
    db.close()
