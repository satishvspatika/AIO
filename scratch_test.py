from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from server.app.models import Base, HealthReport, CommandQueue

engine = create_engine('sqlite:///:memory:')
Base.metadata.create_all(engine)
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
db = SessionLocal()

stn_ids = ["test1"]
try:
    db.query(HealthReport).filter(HealthReport.stn_id.in_(stn_ids)).delete(synchronize_session=False)
    db.query(CommandQueue).filter(CommandQueue.stn_id.in_(stn_ids)).delete(synchronize_session=False)
    db.commit()
    print("SUCCESS")
except Exception as e:
    print(f"FAILED: {e}")
