import logging
import asyncio
import datetime
from sqlalchemy import text
from app.database import SessionLocal

logger = logging.getLogger("spatika_maintenance")
logger.setLevel(logging.INFO)

async def maintenance_loop():
    """
    Background maintenance task that runs once every 24 hours.
    - Prunes health reports older than 90 days.
    - Prunes command queue older than 30 days.
    - Optimizes the SQLite database on disk.
    """
    await asyncio.sleep(60) # Initial delay to let server settle
    
    while True:
        logger.info("[MAINTENANCE] Starting background cleanup...")
        db = SessionLocal()
        try:
            # 1. Prune Health Reports (Keep 90 days)
            res = db.execute(text("DELETE FROM health_reports WHERE reported_at < datetime('now', '-90 days')"))
            logger.info(f"[MAINTENANCE] Pruned health_reports.")
            
            # 2. Prune Command Queue (Keep 30 days)
            res = db.execute(text("DELETE FROM command_queue WHERE created_at < datetime('now', '-30 days')"))
            logger.info(f"[MAINTENANCE] Pruned command_queue.")

            # 3. Clean up expired sessions (if using DB-based sessions)
            # (Sessions are currently in-memory SESSIONS dict, no DB pruning needed yet)
            
            db.commit()
            
            # 4. Database Optimization (Vacuum)
            # Note: VACUUM cannot run within a transaction, so we use a separate connection or raw execution
            logger.info("[MAINTENANCE] Running VACUUM...")
            db.execute(text("VACUUM"))
            
            logger.info("[MAINTENANCE] Cleanup complete.")
            
        except Exception as e:
            logger.error(f"[MAINTENANCE] Error: {e}")
        finally:
            db.close()
            
        # Wait 24 hours before next run
        await asyncio.sleep(86400) 
