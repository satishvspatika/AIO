from app.database import SessionLocal
from app.models import StationSettings, FirmwareRegistry
from app.routers.dashboard import get_latest_per_station

db = SessionLocal()

# 1. Clear all locks
locks = db.query(StationSettings).filter_by(ota_exempt=1).all()
for l in locks:
    print(f"Unlocking {l.stn_id}")
    l.ota_exempt = 0

# 2. Check firmware mappings
reports = get_latest_per_station(db)
fws = db.query(FirmwareRegistry).all()
fw_map = {f"{fw.unit_type}{fw.system_mode}": fw.current_ver for fw in fws}

print(f"\nFirmware Registry Map: {fw_map}")

for r in reports:
    raw_ut = str(r.unit_type or "").upper()
    if "RF" in raw_ut or "ADDON" in raw_ut: base_ut = "TWS-RF"
    elif "TWS" in raw_ut: base_ut = "TWS"
    elif "TRG" in raw_ut: base_ut = "TRG"
    elif "GEN" in raw_ut: base_ut = "GEN"
    else: base_ut = raw_ut
    
    key = f"{base_ut}{r.system or 0}"
    print(f"Station: {r.stn_id} | Raw Type: {raw_ut} -> Key: {key} | Target: {fw_map.get(key, 'None')} | Device: {r.ver}")

db.commit()
