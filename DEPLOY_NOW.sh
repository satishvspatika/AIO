#!/usr/bin/env bash
# =============================================================================
# DEPLOY_NOW.sh - Robust Single-Connection Sync
# =============================================================================
HOST="75.119.148.192"

echo "📦 Bundling files..."
# Move scripts into app/ temporarily to ensure they are inside the Docker mount
cp server/migrate.py server/app/migrate_internal.py
cp server/seed_db.py server/app/seed_db_internal.py
tar -cvz -C server app requirements.txt > deploy.tar.gz

echo "🚀 Sending and Deploying (Single Connection)..."
cat deploy.tar.gz | ssh root@$HOST "
    mkdir -p /opt/spatika-health && \
    cd /opt/spatika-health && \
    cp -a app/SpatikaHealth.db* /tmp/ 2>/dev/null || true && \
    rm -rf app requirements.txt && \
    tar -xz && \
    mv /tmp/SpatikaHealth.db* app/ 2>/dev/null || true && \
    echo '▶ Restarting Spatika Service...' && \
    docker compose restart && \
    sleep 3 && \
    echo '▶ Running DB Migrations...' && \
    docker exec -i -w /app -e PYTHONPATH=/app spatika-health python3 app/migrate_internal.py && \
    echo '▶ Restoring Fleet Categories (Seeding)...' && \
    docker exec -i -w /app -e PYTHONPATH=/app spatika-health python3 app/seed_db_internal.py
"

# Cleanup local tar
rm deploy.tar.gz

echo ""
echo "✅ DEPLOYMENT SUCCESSFUL!"
echo "Visit: http://$HOST/dashboard"
echo "TIP: If changes don't appear, press Cmd+Shift+R or Ctrl+F5 in your browser."
