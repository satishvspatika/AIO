import sqlite3
import os

DB_PATH = "app/SpatikaHealth.db" # Standard path relative to app root

def migrate():
    if not os.path.exists(DB_PATH):
        print(f"DB not found at {DB_PATH}")
        return

    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()

    columns = [
        ("eng_name", "TEXT"),
        ("svc_type", "TEXT")
    ]

    for col_name, col_type in columns:
        try:
            print(f"Adding column {col_name} to service_reports...")
            cursor.execute(f"ALTER TABLE service_reports ADD COLUMN {col_name} {col_type}")
            print(f"✅ Added {col_name}")
        except sqlite3.OperationalError as e:
            if "duplicate column name" in str(e).lower():
                print(f"ℹ️ Column {col_name} already exists.")
            else:
                print(f"❌ Error adding {col_name}: {e}")

    conn.commit()
    conn.close()
    print("Migration finished.")

if __name__ == "__main__":
    migrate()
