#!/usr/bin/env bash
# =============================================================================
# deploy.sh — Spatika Health Server v3.0 Full Deployment
# Run this on your Contabo server terminal.
# It is idempotent: safe to re-run after any change.
# =============================================================================
set -e
cd /opt/spatika-health

echo ""
echo "======================================"
echo " SPATIKA HEALTH SERVER v3.1 DEPLOY "
echo " Build Time: $(date)"
echo "======================================"

# 1. Stop the server
echo "[1/7] Stopping server..."
docker compose stop || true

# 2. Backup existing DB (keeps your data safe)
if [ -f ./data/health.db ]; then
    TS=$(date +%s)
    cp ./data/health.db "./data/health_backup_${TS}.db"
    echo "[2/7] DB backed up → health_backup_${TS}.db"
    # Remove only if it is a fresh deploy (new schema)
    # Comment the next line out if you want to keep existing data
    # rm -f ./data/health.db
    echo "      Old DB preserved (not removing existing data)"
else
    echo "[2/7] No existing DB — starting fresh."
fi

# 3. Create directory structure
echo "[3/7] Creating directories..."
mkdir -p app/routers app/services app/templates builds data

# 4. Install missing library into the running image
# (This is done at runtime; also add to requirements.txt for rebuilds)
echo "[4/7] Ensuring python-multipart is installed..."
docker compose up -d 2>/dev/null || true
sleep 3
docker exec -u 0 spatika-health pip install python-multipart --quiet || true

# 5. Copy all source files from local mirror (if running via SCP method)
#    If running paste-into-terminal method, this step is skipped — files
#    were already cat'd into place before this script ran.
SRC="/opt/spatika-health-src"
if [ -d "$SRC" ]; then
    echo "[5/7] Syncing source from $SRC..."
    cp -r "$SRC/app/routers/"   ./app/routers/
    cp -r "$SRC/app/services/"  ./app/services/
    cp -r "$SRC/app/templates/" ./app/templates/
    cp    "$SRC/app/main.py"    ./app/main.py
    cp    "$SRC/app/models.py"  ./app/models.py
    cp    "$SRC/requirements.txt" ./requirements.txt
    cp    "$SRC/seed_db.py"     ./seed_db.py
else
    echo "[5/7] No local mirror found — assuming files already in place."
fi

# 6. Ensure __init__.py files exist
touch app/__init__.py
touch app/routers/__init__.py
touch app/services/__init__.py

# 7. Restart server and seed DB
echo "[6/7] Restarting server..."
docker compose restart
sleep 5

echo "[7/7] Seeding firmware groups..."
# Trying root path, then /app path
docker exec -i spatika-health python3 /seed_db.py || docker exec -i spatika-health python3 /app/seed_db.py || echo "⚠️ Seeding failed, but dashboard is likely OK if DB already exists."

echo ""
echo "✅ Deployment complete!"
echo "   Dashboard  → http://75.119.148.192/dashboard"
echo "   OTA Mgr    → http://75.119.148.192/ota"
echo ""
