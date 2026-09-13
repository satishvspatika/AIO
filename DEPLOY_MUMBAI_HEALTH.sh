#!/usr/bin/env bash
# ==============================================================================
# 📦 DEPLOY_MUMBAI_HEALTH.sh — DEPLOY SPATIKA HEALTH DASHBOARD TO HOSTINGER MUMBAI
# Target Host : 187.127.175.183
# Target Path : /opt/spatika-health
# Dashboard UI: http://187.127.175.183:8000/dashboard
# Health API  : http://187.127.175.183:8000/health
# ==============================================================================
set -e

HOST="187.127.175.183"
USER="root"
REMOTE_PATH="/opt/spatika-health"
BUNDLE_NAME="spatika_health_deploy.tar.gz"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "=========================================================="
echo " 📦 SPATIKA HEALTH DASHBOARD DEPLOYMENT TO HOSTINGER MUMBAI "
echo " Target Host : $HOST"
echo " Target Path : $REMOTE_PATH"
echo " Time        : $(date)"
echo "=========================================================="

echo "▶ [1/4] Creating application bundle ($BUNDLE_NAME)..."
cd "$SCRIPT_DIR/server"
cp migrate.py app/migrate_internal.py
cp seed_db.py app/seed_db_internal.py
find app -name "*.pyc" -delete || true
find app -name "__pycache__" -exec rm -rf {} + || true

tar -czf "/tmp/$BUNDLE_NAME" app requirements.txt

echo "▶ [2/4] Uploading bundle and active database to Hostinger Mumbai VPS..."
scp -o StrictHostKeyChecking=no -o ConnectTimeout=15 "/tmp/$BUNDLE_NAME" "$USER@$HOST:/tmp/$BUNDLE_NAME"
scp -o StrictHostKeyChecking=no -o ConnectTimeout=15 /tmp/SpatikaHealth.db* "$USER@$HOST:/tmp/"

echo "▶ [3/4] Extracting & configuring service on Hostinger VPS..."
ssh -o StrictHostKeyChecking=no -o ConnectTimeout=15 "$USER@$HOST" 'bash -s' << 'EOF_REMOTE'
set -e
systemctl stop spatika-health.service || true
fuser -k -9 8000/tcp || true

REMOTE_PATH="/opt/spatika-health"
BUNDLE_NAME="spatika_health_deploy.tar.gz"

mkdir -p $REMOTE_PATH/app $REMOTE_PATH/logs $REMOTE_PATH/data
tar --overwrite -xzf /tmp/$BUNDLE_NAME -C $REMOTE_PATH
rm -f /tmp/$BUNDLE_NAME

# Restore database files
mv /tmp/SpatikaHealth.db* $REMOTE_PATH/app/ 2>/dev/null || true

# Setup Python Virtual Environment
if [ ! -d "$REMOTE_PATH/venv" ]; then
    python3 -m venv $REMOTE_PATH/venv
fi

source $REMOTE_PATH/venv/bin/activate
pip install --upgrade pip setuptools -q
pip install --prefer-binary -r $REMOTE_PATH/requirements.txt -q

# Run internal DB migration & seeding
cd $REMOTE_PATH
PYTHONPATH=$REMOTE_PATH $REMOTE_PATH/venv/bin/python3 app/migrate_internal.py || true
PYTHONPATH=$REMOTE_PATH $REMOTE_PATH/venv/bin/python3 app/seed_db_internal.py || true

# Register Systemd Service (Port 8000)
cat << 'EOF_SERVICE' > /etc/systemd/system/spatika-health.service
[Unit]
Description=Spatika Health Monitor Dashboard (Hostinger Mumbai)
After=network.target

[Service]
Type=simple
User=root
WorkingDirectory=/opt/spatika-health
ExecStart=/opt/spatika-health/venv/bin/python3 -m uvicorn app.main:app --host 0.0.0.0 --port 8000
Restart=always
RestartSec=5
Environment=PYTHONPATH=/opt/spatika-health

[Install]
WantedBy=multi-user.target
EOF_SERVICE

systemctl daemon-reload
systemctl enable spatika-health.service
fuser -k -9 8000/tcp || true
sleep 1
systemctl restart spatika-health.service
EOF_REMOTE

echo "=========================================================="
echo " ✅ SPATIKA HEALTH DASHBOARD DEPLOYMENT COMPLETE!"
echo " Service Directory        : $REMOTE_PATH"
echo " 🌐 Web Dashboard         : http://$HOST:8000/dashboard"
echo " 📡 Health Telemetry POST : http://$HOST:8000/health"
echo "=========================================================="
