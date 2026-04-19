"""
seed_db.py — Run once after a fresh DB to populate the 6 firmware groups.
Safe to run multiple times (skips existing entries).

Usage (on Contabo):
    docker exec -i spatika-health python3 /app/seed_db.py
"""
from app.database import SessionLocal, engine
from app.models import Base, FirmwareRegistry

Base.metadata.create_all(bind=engine)
db = SessionLocal()

# ── Adjust total_target when you know the actual station counts ──────────────
GROUPS = [
    dict(category_id=1, name="SPATIKA_ADDON", display_name="SPATIKA-ADDON",
         unit_type="SPATIKA_GEN",  system_mode=2, current_ver="5.79", total_target=10),
    dict(category_id=2, name="KSNDMC_ADDON",  display_name="KSNDMC-ADDON",
         unit_type="KSNDMC_ADDON", system_mode=2, current_ver="5.79", total_target=8),
    dict(category_id=3, name="KSNDMC_TWS",    display_name="KSNDMC-TWS",
         unit_type="KSNDMC_TWS",   system_mode=1, current_ver="5.79", total_target=5),
    dict(category_id=4, name="SPATIKA_TRG",   display_name="SPATIKA-TRG",
         unit_type="SPATIKA_GEN",  system_mode=0, current_ver="5.79", total_target=7),
    dict(category_id=5, name="KSNDMC_TRG",    display_name="KSNDMC-TRG",
         unit_type="KSNDMC_TRG",   system_mode=0, current_ver="5.79", total_target=3),
    dict(category_id=6, name="BIHAR_TRG",     display_name="BIHAR-TRG",
         unit_type="BIHAR_TRG",    system_mode=0, current_ver="5.79", total_target=15),
]

for g in GROUPS:
    existing = db.query(FirmwareRegistry).filter_by(category_id=g["category_id"]).first()
    if not existing:
        db.add(FirmwareRegistry(**g))
        print(f"  ✓ Seeded : {g['display_name']}")
    else:
        print(f"  → Exists : {g['display_name']} (skipped)")

db.commit()
db.close()
print("\nDone. All firmware groups are ready.")
