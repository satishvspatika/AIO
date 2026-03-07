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
        ("http_avg",         "REAL",    "0.0"),
        ("net_cnt_prev",     "INTEGER", "0"),
        ("reg_fail_type",    "TEXT",    "'NONE'"),
        ("reg_worst",        "INTEGER", "0"),
        ("first_http",       "INTEGER", "0"),
        ("spiffs_total_kb",  "INTEGER", "4640"),
        ("calib",            "TEXT",    "'NA'"),
        ("ndm_cnt",          "INTEGER", "0"),
        ("pd_cnt",           "INTEGER", "0"),
        ("cdm_sts",          "TEXT",    "'OK'"),
        # ── Deep Diagnostics (v7.59 new firmware fields) ────────────────────
        ("stack_rtc",        "INTEGER", "0"),
        ("stack_sched",      "INTEGER", "0"),
        ("stack_gprs",       "INTEGER", "0"),
        ("stack_ui",         "INTEGER", "0"),
        ("i2c_errs",         "INTEGER", "0"),
        ("m_temp",           "INTEGER", "0"),
        ("net_type",         "TEXT",    "'NONE'"),
        ("min_rssi",         "INTEGER", "0"),
        ("max_rssi",         "INTEGER", "0"),
    ]:
        try:
            cursor.execute(f"ALTER TABLE health_reports ADD COLUMN {col} {dtype} DEFAULT {default};")
            print(f"✓ Column '{col}' added.")
        except sqlite3.OperationalError as e:
            if "duplicate column name" in str(e):
                print(f"→ Column '{col}' already exists.")
            else:
                print(f"Error adding '{col}': {e}")

    conn.commit()
    conn.close()
    print("Migration complete.")

if __name__ == "__main__":
    migrate()
