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
        ("last_cmd_id",      "INTEGER", "0"),    # v7.92
        ("last_cmd_res",     "TEXT",    "'N/A'"), # v7.92
        ("cum_rf",           "FLOAT",   "0.0"),    # v5.86
    ]:
        try:
            cursor.execute(f"ALTER TABLE health_reports ADD COLUMN {col} {dtype} DEFAULT {default};")
            print(f"✓ Column '{col}' added.")
        except sqlite3.OperationalError as e:
            if "duplicate column name" in str(e):
                print(f"→ Column '{col}' already exists.")
            else:
                print(f"Error adding '{col}': {e}")
    # ── Phase 2: Station State Caching ──
    for col, dtype, default in [
        ("last_gps",     "TEXT",     "NULL"),
        ("last_seen",    "DATETIME", "NULL"),
    ]:
        try:
            cursor.execute(f"ALTER TABLE station_settings ADD COLUMN {col} {dtype} DEFAULT {default};")
            print(f"✓ Column '{col}' added to station_settings.")
        except sqlite3.OperationalError as e:
            if "duplicate column name" in str(e):
                print(f"→ Column '{col}' already exists in station_settings.")
            else:
                print(f"Error adding '{col}' to station_settings: {e}")

    # ── v5.87: Service Report Hardening ──
    # Ensure table exists first (In case of fresh install)
    try:
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS service_reports (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                stn_id TEXT,
                reported_at DATETIME DEFAULT CURRENT_TIMESTAMP,
                bat_v FLOAT,
                sol_v FLOAT,
                gps TEXT,
                comments TEXT,
                img1_path TEXT,
                img2_path TEXT,
                is_sync INTEGER DEFAULT 0
            );
        """)
    except Exception as e:
        print(f"Error ensuring service_reports table: {e}")

    for col, dtype, default in [
        ("inst_rf", "FLOAT", "NULL"),
        ("cum_rf",  "FLOAT", "NULL"),
        ("eng_name", "TEXT", "'Unknown'"),
        ("svc_type", "TEXT", "'General'"),
        ("is_finalized", "BOOLEAN", "0"),
        ("has_img1", "BOOLEAN", "0"),
        ("has_img2", "BOOLEAN", "0"),
    ]:
        try:
            cursor.execute(f"ALTER TABLE service_reports ADD COLUMN {col} {dtype} DEFAULT {default};")
            print(f"✓ Column '{col}' added to service_reports.")
        except sqlite3.OperationalError as e:
            if "duplicate" in str(e).lower():
                print(f"→ Column '{col}' already exists in service_reports.")
            elif "no such table" in str(e).lower():
                print(f"→ Skipping '{col}': Table 'service_reports' not found yet.")
            else:
                print(f"Error adding '{col}' to service_reports: {e}")

    # v6.05 Backfill: Mark legacy reports as finalized so they remain visible
    try:
        # Mark all as finalized first
        cursor.execute("UPDATE service_reports SET is_finalized = 1 WHERE is_finalized = 0;")
        # THEN un-finalize those that are clearly missing expected images (Half-baked)
        cursor.execute("UPDATE service_reports SET is_finalized = 0 WHERE (has_img1 = 1 AND img1_path IS NULL) OR (has_img2 = 1 AND img2_path IS NULL);")
        print("✓ Legacy service reports backfilled and sanitized (Visibility restored).")
    except Exception as e:
        print(f"Error backfilling service reports: {e}")

    # ── v7.93: Firmware Registry Hardening ──
    for col, dtype, default in [
        ("display_name", "TEXT",    "''"),
        ("total_target", "INTEGER", "0"),
    ]:
        try:
            cursor.execute(f"ALTER TABLE firmware_registry ADD COLUMN {col} {dtype} DEFAULT {default};")
            print(f"✓ Column '{col}' added to firmware_registry.")
        except sqlite3.OperationalError as e:
            if "duplicate" not in str(e): print(f"Error adding '{col}' to firmware_registry: {e}")

    conn.commit()
    conn.close()
    print("Migration complete.")

if __name__ == "__main__":
    migrate()
