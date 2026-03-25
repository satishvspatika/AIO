import sqlite3
import os
import sys

# Dynamically find the correct database path from the server's own config
try:
    from app.database import engine
    db = engine.url.database
except ImportError:
    print("Could not import app.database. Ensure you are running this in the correct environment.")
    sys.exit(1)

def migrate():
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

    try:
        cursor.execute("ALTER TABLE health_reports ADD COLUMN http_ret_cnt INTEGER DEFAULT 0;")
        print("✓ Column 'http_ret_cnt' added.")
    except sqlite3.OperationalError as e:
        if "duplicate" not in str(e): print(e)

    try:
        cursor.execute("ALTER TABLE health_reports ADD COLUMN http_ret_cnt_prev INTEGER DEFAULT 0;")
        print("✓ Column 'http_ret_cnt_prev' added.")
    except sqlite3.OperationalError as e:
        if "duplicate" not in str(e): print(e)

    try:
        cursor.execute("ALTER TABLE health_reports ADD COLUMN ftp_suc_cnt INTEGER DEFAULT 0;")
        print("✓ Column 'ftp_suc_cnt' added.")
    except sqlite3.OperationalError as e:
        if "duplicate" not in str(e): print(e)

    try:
        cursor.execute("ALTER TABLE health_reports ADD COLUMN ftp_suc_cnt_prev INTEGER DEFAULT 0;")
        print("✓ Column 'ftp_suc_cnt_prev' added.")
    except sqlite3.OperationalError as e:
        if "duplicate" not in str(e): print(e)

    # ── v7.59: Columns present in models.py but previously missing from migrate.py ──
    for col, dtype, default in [
        ("http_fails",       "INTEGER", "0"),
        ("http_fail_reason", "TEXT",    "'NONE'"),
        ("net_cnt",          "INTEGER", "0"),
        ("net_cnt_prev",     "INTEGER", "0"),
        ("first_http",       "INTEGER", "0"),
        ("spiffs_kb",        "INTEGER", "0"),
        ("spiffs_total_kb",  "INTEGER", "4640"),
        ("calib",            "TEXT",    "'NA'"),
        ("ndm_cnt",          "INTEGER", "0"),
        ("pd_cnt",           "INTEGER", "0"),
        ("cdm_sts",          "TEXT",    "'PENDING'"),
        ("reg_fails",        "INTEGER", "0"),
        ("reg_fail_reason",  "TEXT",    "'NONE'"),
        ("reset_reason",     "INTEGER", "0"),
        ("http_present_fails", "INTEGER", "0"),
        ("http_cum_fails",   "INTEGER", "0"),
        ("consec_reg_fails", "INTEGER", "0"),
        ("consec_http_fails","INTEGER", "0"),
        ("consec_sim_fails", "INTEGER", "0"),
        ("unsent_count",     "INTEGER", "0"),
        ("http_backlog_cnt", "INTEGER", "0"),
        ("mutex_fail",       "INTEGER", "0"),
    ]:
        try:
            cursor.execute(f"ALTER TABLE health_reports ADD COLUMN {col} {dtype} DEFAULT {default};")
            print(f"✓ Column '{col}' added.")
        except sqlite3.OperationalError as e:
            if "duplicate column name" in str(e):
                print(f"→ Column '{col}' already exists.")
            else:
                print(f"Error adding '{col}': {e}")
    # ── v7.92: Command Feedback Loop ──
    for col, dtype, default in [
        ("result",       "TEXT",     "'SENT'"),
        ("completed_at", "DATETIME", "NULL"),
    ]:
        try:
            cursor.execute(f"ALTER TABLE command_queue ADD COLUMN {col} {dtype} DEFAULT {default};")
            print(f"✓ Column '{col}' added to command_queue.")
        except sqlite3.OperationalError as e:
            if "duplicate column name" in str(e):
                print(f"→ Column '{col}' already exists in command_queue.")
            else:
                print(f"Error adding '{col}' to command_queue: {e}")

    # ── Phase 2 Fix: Binary Integrity (MD5) ──
    try:
        cursor.execute("ALTER TABLE firmware_registry ADD COLUMN md5 TEXT DEFAULT '';")
        print("✓ Column 'md5' added to firmware_registry.")
    except sqlite3.OperationalError as e:
        if "duplicate column name" in str(e):
            print("→ Column 'md5' already exists in firmware_registry.")
        else:
            print(f"Error adding 'md5' to firmware_registry: {e}")

    conn.commit()
    conn.close()
    print("Migration complete.")

if __name__ == "__main__":
    migrate()
