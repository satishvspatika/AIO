#!/usr/bin/env bash
# =============================================================================
# DEPLOY_NOW.sh - Server Code Sync ONLY
#
# This script deploys server application code (Python, templates, DB migrations).
# It does NOT touch:
#   - /app/builds/      (OTA firmware binaries)
#   - FirmwareRegistry  (firmware target versions in DB)
#
# To compile and push a new firmware version to the server, use:
#   python3 build_all_configs.py --configs KSNDMC_TRG --flash 16mb --upload http://75.119.148.192
# =============================================================================
set -e
HOST="75.119.148.192"

echo "📦 Bundling server files..."
cp server/migrate.py server/app/migrate_internal.py
cp server/seed_db.py server/app/seed_db_internal.py
find server/app -name "*.pyc" -delete || true
find server/app -name "__pycache__" -exec rm -rf {} + || true
tar -cvz -C server app requirements.txt docker-compose.yml > deploy.tar.gz

echo "🚀 Sending and Deploying (Single Connection)..."
if ! cat deploy.tar.gz | ssh root@$HOST "
    set -e
    mkdir -p /opt/spatika-health && \
    cd /opt/spatika-health && \
    cp -a app/SpatikaHealth.db* /tmp/ 2>/dev/null || true && \
    cp -rp app/builds /tmp/spatika_builds_bak 2>/dev/null || true && \
    rm -rf app requirements.txt && \
    tar -xz && \
    find app -name '*.pyc' -delete || true && \
    find app -name '__pycache__' -exec rm -rf {} + || true && \
    mv /tmp/SpatikaHealth.db* app/ 2>/dev/null || true && \
    mkdir -p app/builds builds && \
    cp -rp /tmp/spatika_builds_bak/. app/builds/ 2>/dev/null || true && \
    echo '▶ Restarting Spatika Service...' && \
    docker compose restart && \
    sleep 3 && \
    echo '▶ Running DB Migrations...' && \
    docker exec -i -w /app -e PYTHONPATH=/app spatika-health python3 app/migrate_internal.py && \
    echo '▶ Seeding Fleet Categories (preserving existing firmware targets)...' && \
    docker exec -i -w /app -e PYTHONPATH=/app spatika-health python3 app/seed_db_internal.py
"; then
    echo ""
    echo "❌ DEPLOYMENT FAILED!"
    echo "Reason: SSH connection failed or permission denied. Check your password."
    rm -f deploy.tar.gz
    exit 1
fi

rm -f deploy.tar.gz

echo ""
echo "✅ DEPLOYMENT SUCCESSFUL!"
echo "   Visit: https://devhlt.spatika.net/dashboard"
echo ""
echo "📌 To push a new firmware version, run separately:"
echo "   python3 build_all_configs.py --configs KSNDMC_TRG --flash 16mb --upload https://devhlt.spatika.net"
echo ""
echo "TIP: If changes don't appear, press Cmd+Shift+R or Ctrl+F5 in your browser."
