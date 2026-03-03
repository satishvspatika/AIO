#!/usr/bin/env bash
HOST="75.119.148.192"

cat << 'EOF' > server/run_sql.py
import sqlite3
try:
    conn = sqlite3.connect("/data/health.db")
    c = conn.cursor()
    for col in ["cur_stored", "prev_stored", "http_suc_cnt", "http_suc_cnt_prev"]:
        try:
            c.execute(f"ALTER TABLE health_reports ADD COLUMN {col} INTEGER DEFAULT 0")
            print(f"Added column: {col}")
        except:
            pass
    conn.commit()
    print("Migration complete!")
except Exception as e:
    print("Error:", e)
EOF

echo "▶ Copying python migration script..."
scp server/run_sql.py root@$HOST:/opt/spatika-health/run_sql.py

echo "▶ Running DB Migrations via temporary Docker container..."
ssh root@$HOST "docker run --rm -v /opt/spatika-health/data:/data -v /opt/spatika-health/run_sql.py:/run_sql.py python:3.9-alpine python /run_sql.py"
echo "✅ Migration finished."
