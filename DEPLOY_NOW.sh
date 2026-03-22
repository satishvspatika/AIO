#!/usr/bin/env bash
# =============================================================================
# DEPLOY_NOW.sh - Final Fixed Version
# =============================================================================
HOST="75.119.148.192"

echo "🚀 Syncing FILES directly to Live Folder..."
scp -r server/app server/deploy.sh server/requirements.txt \
    server/seed_db.py server/migrate.py \
    root@$HOST:/opt/spatika-health/

echo "▶ Restarting Spatika Service..."
ssh root@$HOST "cd /opt/spatika-health && docker compose restart"
sleep 2

echo "▶ Running DB Migrations..."
cat server/migrate.py | ssh root@$HOST "docker exec -i spatika-health python3 -"

echo "✅ DEPLOYMENT SUCCESSFUL!"
echo "Visit: http://$HOST/dashboard"
echo "TIP: If changes don't appear, press Cmd+Shift+R or Ctrl+F5 in your browser."
