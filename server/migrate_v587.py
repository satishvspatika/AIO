import sqlite3
import os

DB_PATH = "app/SpatikaHealth.db"

def migrate():
    if not os.path.exists(DB_PATH):
        print(f"❌ DB not found at {DB_PATH}")
        return

    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()

    print(f"🚀 Starting Migration v5.88 (Whitespace & Leading Zero Hardening)...")

    # 1. Normalize stn_id across all tables (Strip leading zeros AND whitespace)
    tables_to_normalize = [
        "health_reports",
        "service_reports",
        "command_queue",
        "station_settings"
    ]

    for table in tables_to_normalize:
        print(f"  → Hardening stn_id in table: {table}")
        
        # Select all IDs to check for any that need trimming or leading zero removal
        cursor.execute(f"SELECT DISTINCT stn_id FROM {table};")
        all_ids = cursor.fetchall()
        
        for (raw_id,) in all_ids:
            if not raw_id: continue
            
            # 1. Strip whitespace
            clean_id = raw_id.strip()
            
            # 2. Convert to int and back to string to strip leading zeros if it's numeric
            digit_id = "".join(filter(str.isdigit, clean_id))
            if digit_id and digit_id.isdigit():
                norm_id = str(int(digit_id))
                
                if norm_id != raw_id:
                    # Check if norm_id already exists in station_settings to avoid unique constraint 500 error
                    if table == "station_settings":
                        cursor.execute("SELECT 1 FROM station_settings WHERE stn_id = ?", (norm_id,))
                        if cursor.fetchone():
                            print(f"    - Merging {raw_id} into existing {norm_id}")
                            # For settings, we might want to delete the dirty one
                            cursor.execute("DELETE FROM station_settings WHERE stn_id = ?", (raw_id,))
                            continue

                    cursor.execute(f"UPDATE {table} SET stn_id = ? WHERE stn_id = ?;", (norm_id, raw_id))
                    print(f"    - Migrated [{raw_id}] → [{norm_id}]")

    conn.commit()
    conn.close()
    print("✅ Hardening Complete!")

if __name__ == "__main__":
    migrate()
