import sqlite3
import os
import sys

# Try to find the correct database path
try:
    # Look in app.db as default if running on server
    db_path = "app/app.db"
    if not os.path.exists(db_path):
        db_path = "server/app/app.db"
    if not os.path.exists(db_path):
        db_path = "app.db"
except Exception:
    db_path = "app.db" 

def migrate():
    print(f"Connecting to {db_path}...")
    if not os.path.exists(db_path):
        print(f"Error: Database file {db_path} not found.")
        # Try finding it in the current directory if relative paths fail
        db_path_alt = "app.db"
        if os.path.exists(db_path_alt):
            print(f"Found {db_path_alt}, using it.")
        else:
            return

    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()

    # List of columns to drop
    cols_to_drop = [
        "cur_stored",
        "spiffs_free_kb",
        "slot_no",
        "reg_avg",
        "reg_worst",
        "reg_fail_type",
        "http_avg",
        "ws_same_cnt"
    ]

    print(f"Attempting to drop {len(cols_to_drop)} columns...")

    for col in cols_to_drop:
        try:
            # Check if column exists first
            cursor.execute("PRAGMA table_info(health_reports)")
            cols = [c[1] for c in cursor.fetchall()]
            
            if col in cols:
                print(f"Dropping column '{col}'...")
                cursor.execute(f"ALTER TABLE health_reports DROP COLUMN {col};")
                print(f"✓ Column '{col}' dropped.")
            else:
                print(f"→ Column '{col}' does not exist, skipping.")
        except sqlite3.OperationalError as e:
            print(f"⚠️ Could not drop '{col}': {e}")
            if "no such column" in str(e):
                continue
            
            # Fallback for older SQLite versions that don't support DROP COLUMN
            print(f"Note: Your SQLite version might be too old to support DROP COLUMN (3.35+ required).")
            break

    conn.commit()
    conn.close()
    print("Migration complete.")

if __name__ == "__main__":
    migrate()
