import sqlite3
import os

# Path on the Contabo server
DB_PATH = "/opt/spatika-health/data/health.db"

def migrate():
    if not os.path.exists(DB_PATH):
        # Local development path fallback
        DB_PATH_LOCAL = "app/SpatikaFleet.db"
        if os.path.exists(DB_PATH_LOCAL):
            db = DB_PATH_LOCAL
        else:
            print(f"Error: Database not found at {DB_PATH}")
            return
    else:
        db = DB_PATH

    print(f"Connecting to {db}...")
    conn = sqlite3.connect(db)
    cursor = conn.cursor()

    try:
        print("Adding 'ota_fails' column to 'health_reports'...")
        cursor.execute("ALTER TABLE health_reports ADD COLUMN ota_fails INTEGER DEFAULT 0;")
        print("✓ Column 'ota_fails' added.")
    except sqlite3.OperationalError as e:
        if "duplicate column name" in str(e):
            print("→ Column 'ota_fails' already exists.")
        else:
            print(f"Error adding 'ota_fails': {e}")

    try:
        print("Adding 'ota_fail_reason' column to 'health_reports'...")
        cursor.execute("ALTER TABLE health_reports ADD COLUMN ota_fail_reason TEXT DEFAULT 'NONE';")
        print("✓ Column 'ota_fail_reason' added.")
    except sqlite3.OperationalError as e:
        if "duplicate column name" in str(e):
            print("→ Column 'ota_fail_reason' already exists.")
        else:
            print(f"Error adding 'ota_fail_reason': {e}")

    try:
        cursor.execute("ALTER TABLE health_reports ADD COLUMN cur_stored INTEGER DEFAULT 0;")
        print("✓ Column 'cur_stored' added.")
    except sqlite3.OperationalError as e:
        if "duplicate" not in str(e): print(e)

    try:
        cursor.execute("ALTER TABLE health_reports ADD COLUMN prev_stored INTEGER DEFAULT 0;")
        print("✓ Column 'prev_stored' added.")
    except sqlite3.OperationalError as e:
        if "duplicate" not in str(e): print(e)

    try:
        cursor.execute("ALTER TABLE health_reports ADD COLUMN http_suc_cnt INTEGER DEFAULT 0;")
        print("✓ Column 'http_suc_cnt' added.")
    except sqlite3.OperationalError as e:
        if "duplicate" not in str(e): print(e)

    try:
        cursor.execute("ALTER TABLE health_reports ADD COLUMN http_suc_cnt_prev INTEGER DEFAULT 0;")
        print("✓ Column 'http_suc_cnt_prev' added.")
    except sqlite3.OperationalError as e:
        if "duplicate" not in str(e): print(e)

    conn.commit()
    conn.close()
    print("Migration complete.")

if __name__ == "__main__":
    migrate()
