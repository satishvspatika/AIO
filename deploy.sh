#!/bin/bash
# deploy.sh
# System for versioned and station-specific OTA deployments
# Usage: ./deploy.sh [STATION_ID] [8mb|16mb|4mb]

STN=$1
FLASH_SIZE=${2:-8mb}

if [ -z "$STN" ]; then
    echo "❌ Error: Station ID required."
    echo "Usage: ./deploy.sh [STATION_ID] [8mb|16mb|4mb]"
    exit 1
fi

# 1. Get current version and config from firmware
FW_VER=$(grep '#define FIRMWARE_VERSION' globals.h | sed 's/.*"\(.*\)".*/\1/')
SYSTEM_TYPE=$(grep '#define SYSTEM' globals.h | awk '{print $3}')
UNIT_TYPE=$(grep 'char UNIT' globals.h | sed 's/.*"\(.*\)".*/\1/')

# 2. Compile
echo "🔨 Compiling v$FW_VER for $STN ($UNIT_TYPE / System $SYSTEM_TYPE)..."
./compile.sh "$FLASH_SIZE"
if [ $? -ne 0 ]; then
    echo "❌ Compilation failed. Aborting."
    exit 1
fi

# 3. Rename Binary for Cloud Repository
# Format: v[VER]_[STN]_[SYS].bin
BIN_NAME="v${FW_VER}_${STN}_S${SYSTEM_TYPE}.bin"
cp "/tmp/aio_build_${FLASH_SIZE}/AIO9_5.0.ino.bin" "./$BIN_NAME"

# 4. Upload to Contabo Dashboard
# Update with your server's details
SERVER_IP="75.119.148.192"
REMOTE_PATH="/var/www/html/bins/" # Where your Nginx/Apache serves files

echo "🚀 Uploading $BIN_NAME to $SERVER_IP..."
scp "./$BIN_NAME" "root@$SERVER_IP:$REMOTE_PATH"

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ DEPLOYMENT COMPLETE!"
    echo "--------------------------------------------------"
    echo "Station : $STN"
    echo "Binary  : $BIN_NAME"
    echo "URL     : http://$SERVER_IP/bins/$BIN_NAME"
    echo "--------------------------------------------------"
    echo "👉 Now update your Dashboard DB to point $STN to this filename."
else
    echo "❌ Upload failed. Check your SSH/SCP connection."
fi
