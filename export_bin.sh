#!/bin/bash
# Helper script to organize Arduino IDE compiled binaries
# Usage: ./export_bin.sh CONFIG_NAME

CONFIG_NAME=$1

# Map config names to short codes, Category IDs, and Target Filenames
declare -a CONFIG_NAMES=( "SPATIKA_ADDON" "KSNDMC_ADDON" "KSNDMC_TWS" "SPATIKA_TRG" "KSNDMC_TRG" "BIHAR_TRG" )
declare -a CONFIG_VALUES=( "TWSRF9-GEN|1|FW_S1_SPATIKA_GEN.bin" "TWSRF9-DMC|2|FW_S2_KSNDMC_ADDON.bin" "TWS9-DMC|3|FW_S3_KSNDMC_TWS.bin" "TRG9-GEN|4|FW_S4_SPATIKA_GEN.bin" "TRG9-DMC|5|FW_S5_KSNDMC_TRG.bin" "TRG9-BIH|6|FW_S6_BIHAR_TRG.bin" )

CONFIG_FOUND=0
CONFIG_VALUE=""

for i in "${!CONFIG_NAMES[@]}"; do
    if [ "${CONFIG_NAMES[$i]}" == "$CONFIG_NAME" ]; then
        CONFIG_FOUND=1
        CONFIG_VALUE="${CONFIG_VALUES[$i]}"
        break
    fi
done

if [ $CONFIG_FOUND -eq 0 ]; then
    echo "Usage: ./export_bin.sh CONFIG_NAME"
    echo ""
    echo "Available configurations:"
    for cfg in "${CONFIG_NAMES[@]}"; do echo "  - $cfg"; done
    echo ""
    echo "Example: ./export_bin.sh SPATIKA_ADDON"
    exit 1
fi

IFS='|' read -r SHORT_CODE CAT_ID TARGET_FILENAME <<< "$CONFIG_VALUE"

# Get base firmware version from user_config.h
BASE_VERSION=$(grep '#define FIRMWARE_VERSION' user_config.h | sed 's/.*"\(.*\)".*/\1/')
FULL_VERSION="${SHORT_CODE}-${BASE_VERSION}"

echo "🚀 Organizing build for: $CONFIG_NAME"
echo "📦 Base Version: $BASE_VERSION"
echo "🏷️  Full Version: $FULL_VERSION"
echo ""

# Create output directory and CLEAN IT
TARGET_DIR="builds/$CONFIG_NAME"
if [ -d "$TARGET_DIR" ]; then
    echo "🧹 Cleaning existing builds in $TARGET_DIR..."
    rm -rf "$TARGET_DIR"
fi
mkdir -p "$TARGET_DIR"

# Find and move binary
BINARY_FOUND=0
POSSIBLE_BINS=(
    "AIO9_5.0.ino.esp32.bin"
    "AIO9_5.0.ino.bin"
    "build/esp32.esp32.esp32/AIO9_5.0.ino.bin"
)

for bin_path in "${POSSIBLE_BINS[@]}"; do
    if [ -f "$bin_path" ]; then
        cp "$bin_path" "$TARGET_DIR/firmware.bin"
        BINARY_FOUND=1
        echo "✓ Found and copied binary: $bin_path"
        break
    fi
done

if [ $BINARY_FOUND -eq 0 ]; then
    echo "❌ ERROR: No binary found! Compile the project first."
    exit 1
fi

echo "$FULL_VERSION" > "$TARGET_DIR/fw_version.txt"
cat > "$TARGET_DIR/build_info.txt" << EOF
Build Configuration
===================
Configuration: $CONFIG_NAME
Short Code: $SHORT_CODE
Firmware Version: $FULL_VERSION
Base Version: $BASE_VERSION
Target DB Cat ID: $CAT_ID
Target File: $TARGET_FILENAME
Build Date: $(date)
EOF

# Show results
echo ""
echo "========================================="
echo "  Build Organized Successfully!"
echo "========================================="
echo "Output directory: $TARGET_DIR/"
ls -lh "$TARGET_DIR/"
echo ""

# Copy to external release directory
EXTERNAL_RELEASE_PATH="/Users/satishkripavasan/Documents/Arduino/ESP32_NEW_DESIGN/RELEASE/AIO9_5/AIO9_5.0/$CONFIG_NAME"
mkdir -p "$EXTERNAL_RELEASE_PATH"
cp "$TARGET_DIR/firmware.bin" "$EXTERNAL_RELEASE_PATH/firmware.bin"
cp "$TARGET_DIR/fw_version.txt" "$EXTERNAL_RELEASE_PATH/fw_version.txt"
cp PROJECT_STATUS.md "/Users/satishkripavasan/Documents/Arduino/ESP32_NEW_DESIGN/RELEASE/AIO9_5/AIO9_5.0/RELEASE_NOTES.md" 2>/dev/null
echo "✓ Copied to external release: $EXTERNAL_RELEASE_PATH"
echo ""

# --- AUTO DEPLOY OTA ---
echo "Would you like to auto-deploy this firmware to the remote server OTA?"
read -p "Push as target for $CONFIG_NAME now? (y/n) " -n 1 -r
echo ""
if [[ $REPLY =~ ^[Yy]$ ]]; then
    HOST="75.119.148.192"
    echo "🔑 Connecting to Server... (You will be prompted for your SSH password up to 3 times)"
    
    # 1. SSH mkdir to ensure builds folder exists
    ssh root@$HOST "mkdir -p /opt/spatika-health/app/builds"
    
    # 2. SCP the file
    echo "📡 Uploading $TARGET_FILENAME to Server..."
    scp "$TARGET_DIR/firmware.bin" "root@$HOST:/opt/spatika-health/app/builds/$TARGET_FILENAME"
    
    # 3. Handle Docker Transfer and DB Update in a single SSH session
    echo "💾 Injecting into Docker Container & Updating Database..."
    ssh root@$HOST "docker cp /opt/spatika-health/app/builds/$TARGET_FILENAME spatika-health:/app/builds/$TARGET_FILENAME && \
docker restart spatika-health && \
docker exec spatika-health python3 -c \"
import sqlite3
try:
    conn = sqlite3.connect('/app/data/health.db')
    c = conn.cursor()
    c.execute('UPDATE firmware_registry SET current_ver=\\'$FULL_VERSION\\', filename=\\'$TARGET_FILENAME\\' WHERE category_id=$CAT_ID')
    conn.commit()
    conn.close()
    print('✅ Database updated successfully.')
except Exception as e:
    print('❌ Database Error:', e)
\""
    echo "🚀 OTA DEPLOYMENT COMPLETE! Stations will now pull $FULL_VERSION."
fi
echo ""
