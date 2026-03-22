#!/bin/bash
# export_bin.sh — Build organiser + OTA deployer for AIO9 firmware
# v2.0: Fixed critical docker cp/restart race, binary validation, build detection
# Usage: ./export_bin.sh CONFIG_NAME [/path/to/binary.bin]
#
# If the binary path is omitted, the script searches common Arduino IDE 2.x output locations.

CONFIG_NAME=$1
EXPLICIT_BIN=$2   # Optional: pass the exact .bin path to avoid search ambiguity

declare -a CONFIG_NAMES=(  "SPATIKA_ADDON" "KSNDMC_ADDON" "KSNDMC_TWS"       "SPATIKA_TRG"       "KSNDMC_TRG"       "BIHAR_TRG"   )
declare -a CONFIG_VALUES=( "TWSRF9-GEN|1|FW_S1_SPATIKA_GEN.bin" "TWSRF9-DMC|2|FW_S2_KSNDMC_ADDON.bin" "TWS9-DMC|3|FW_S3_KSNDMC_TWS.bin" "TRG9-GEN|4|FW_S4_SPATIKA_GEN.bin" "TRG9-DMC|5|FW_S5_KSNDMC_TRG.bin" "TRG9-BIH|6|FW_S6_BIHAR_TRG.bin" )

CONFIG_FOUND=0
for i in "${!CONFIG_NAMES[@]}"; do
    if [ "${CONFIG_NAMES[$i]}" == "$CONFIG_NAME" ]; then
        CONFIG_FOUND=1
        CONFIG_VALUE="${CONFIG_VALUES[$i]}"
        break
    fi
done

if [ $CONFIG_FOUND -eq 0 ]; then
    echo "Usage: ./export_bin.sh CONFIG_NAME [/path/to/binary.bin]"
    echo ""
    echo "Available configurations:"
    for cfg in "${CONFIG_NAMES[@]}"; do echo "  - $cfg"; done
    echo ""
    echo "Example: ./export_bin.sh KSNDMC_TWS"
    echo "Example: ./export_bin.sh KSNDMC_TWS /tmp/arduino-build-xxx/AIO9_5.0.ino.bin"
    exit 1
fi

IFS='|' read -r SHORT_CODE CAT_ID TARGET_FILENAME <<< "$CONFIG_VALUE"
BASE_VERSION=$(grep '#define FIRMWARE_VERSION' user_config.h | sed 's/.*"\(.*\)".*/\1/')
FULL_VERSION="${SHORT_CODE}-${BASE_VERSION}"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  PRE-FLIGHT: Arduino IDE must have these settings:"
echo "  Tools -> Flash Size       -> 8MB (64Mbit)"
echo "  Tools -> Partition Scheme -> Custom (uses partitions.csv)"
echo "  Export: AIO9_5.0.ino.bin  (NOT .merged.bin)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "🚀 Building OTA package for: $CONFIG_NAME"
echo "📦 Version: $FULL_VERSION  |  DB Cat ID: $CAT_ID  |  File: $TARGET_FILENAME"
echo ""

# ─────────────────────────────────────────────────────────────────────────────
# STEP 1: Find the binary
# ─────────────────────────────────────────────────────────────────────────────
FOUND_BIN=""

if [ -n "$EXPLICIT_BIN" ]; then
    # User supplied explicit path — trust it
    if [ -f "$EXPLICIT_BIN" ]; then
        FOUND_BIN="$EXPLICIT_BIN"
        echo "✓ Using supplied binary: $EXPLICIT_BIN"
    else
        echo "❌ ERROR: Supplied binary not found: $EXPLICIT_BIN"
        exit 1
    fi
else
    # Auto-search. Arduino IDE 2.x puts builds in a temp directory.
    # We search from newest to oldest to avoid picking up stale builds.
    # IMPORTANT: only accept .ino.bin — NEVER .merged.bin or .bootloader.bin
    echo "🔍 Searching for compiled binary (newest first)..."

    # Search locations
    # NOTE: Removed /var/folders because find traverses it incredibly slowly on macOS.
    SEARCH_DIRS=(
        "/tmp"                   # Linux/macOS temp (Arduino IDE 2.x / CLI)
        "."                      # Sketch directory (Arduino IDE 1.x / export)
        ".pio/build/esp32dev"    # PlatformIO
    )

    # Use find to locate all matching .ino.bin files, sorted by modification time (newest first)
    # Explicitly exclude .merged.bin and .bootloader.bin and .partitions.bin
    CANDIDATES=$(find "${SEARCH_DIRS[@]}" -name "AIO9_5.0.ino.bin" \
        ! -name "*.merged.bin" \
        ! -name "*.bootloader.bin" \
        ! -name "*.partitions.bin" \
        -newer user_config.h \
        -type f 2>/dev/null | xargs ls -t 2>/dev/null | head -5)

    # Also check platformio firmware.bin
    if [ -z "$CANDIDATES" ]; then
        CANDIDATES=$(find .pio -name "firmware.bin" ! -name "firmware_merged.bin" -type f 2>/dev/null | head -3)
    fi

    if [ -z "$CANDIDATES" ]; then
        echo "❌ ERROR: No binary found newer than user_config.h."
        echo ""
        echo "  Either:"
        echo "  1. Compile the project first (Sketch → Verify/Compile)"
        echo "  2. Pass the binary path explicitly:"
        echo "     ./export_bin.sh $CONFIG_NAME /path/to/AIO9_5.0.ino.bin"
        echo ""
        echo "  DO NOT use .merged.bin — that is the full flash image and will break OTA."
        exit 1
    fi

    FOUND_BIN=$(echo "$CANDIDATES" | head -1)
    echo "✓ Found binary: $FOUND_BIN"
fi

# ─────────────────────────────────────────────────────────────────────────────
# STEP 2: Validate the binary is a real ESP32 app partition image
# ─────────────────────────────────────────────────────────────────────────────
MAGIC=$(xxd -l 1 "$FOUND_BIN" 2>/dev/null | awk '{print $2}' | tr '[:lower:]' '[:upper:]')
if [ "$MAGIC" != "E9" ]; then
    echo "❌ ERROR: Binary does not start with ESP32 magic byte 0xE9 (got: 0x$MAGIC)"
    echo "   This is not a valid ESP32 firmware binary."
    exit 1
fi

# Reject if file name contains "merged" (double-safety)
if [[ "$FOUND_BIN" == *"merged"* ]]; then
    echo "❌ ERROR: Binary path contains 'merged' — this is a full flash image, NOT OTA-compatible."
    echo "   Use the plain .ino.bin file, not .merged.bin"
    exit 1
fi

BIN_SIZE=$(wc -c < "$FOUND_BIN" | awk '{print $1}')

MAX_APP_SIZE=1769472   # Exact app slot size for AIO9 8MB partition (0x1B0000 bytes)
if [ "$BIN_SIZE" -gt "$MAX_APP_SIZE" ]; then
    echo "⚠️  WARNING: Binary ($BIN_SIZE bytes) is dangerously larger than the maximum App partition size ($MAX_APP_SIZE bytes)!"
    echo "   Ensure your partitions.csv actively supports this binary size before deploying. Update.end() WILL fail otherwise."
    echo ""
fi
echo "✓ Binary validated: magic=0xE9, size=${BIN_SIZE} bytes ($(echo "scale=1; $BIN_SIZE/1024/1024" | bc)MB)"

if [ "$BIN_SIZE" -lt 500000 ]; then
    echo "⚠️  WARNING: Binary is suspiciously small (${BIN_SIZE} bytes < 500KB)."
    echo "   Are you sure this is a complete build?"
    read -p "   Continue anyway? (y/n) " -n 1 -r; echo
    [[ $REPLY =~ ^[Nn]$ ]] && exit 1
fi

# ─────────────────────────────────────────────────────────────────────────────
# STEP 3: Stage locally
# ─────────────────────────────────────────────────────────────────────────────
TARGET_DIR="builds/$CONFIG_NAME"
rm -rf "$TARGET_DIR"
mkdir -p "$TARGET_DIR"
cp "$FOUND_BIN" "$TARGET_DIR/firmware.bin"
echo "$FULL_VERSION" > "$TARGET_DIR/fw_version.txt"
cat > "$TARGET_DIR/build_info.txt" << BINFO
Build Configuration
===================
Configuration:   $CONFIG_NAME
Short Code:      $SHORT_CODE
Version:         $FULL_VERSION
Base Version:    $BASE_VERSION
Target Cat ID:   $CAT_ID
Target File:     $TARGET_FILENAME
Build Date:      $(date)
Binary Size:     ${BIN_SIZE} bytes
Source Binary:   $FOUND_BIN
BINFO

echo "✓ Staged to: $TARGET_DIR/"

# Copy to local release archive
EXTERNAL_RELEASE_PATH="/Users/satishkripavasan/Documents/Arduino/ESP32_NEW_DESIGN/RELEASE/AIO9_5/AIO9_5.0/$CONFIG_NAME"
mkdir -p "$EXTERNAL_RELEASE_PATH"
cp "$TARGET_DIR/firmware.bin" "$EXTERNAL_RELEASE_PATH/firmware.bin"
cp "$TARGET_DIR/fw_version.txt" "$EXTERNAL_RELEASE_PATH/fw_version.txt"
cp PROJECT_STATUS.md "/Users/satishkripavasan/Documents/Arduino/ESP32_NEW_DESIGN/RELEASE/AIO9_5/AIO9_5.0/RELEASE_NOTES.md" 2>/dev/null
echo "✓ Archived locally: $EXTERNAL_RELEASE_PATH"

# ─────────────────────────────────────────────────────────────────────────────
# STEP 4: Deploy to server (optional)
# ─────────────────────────────────────────────────────────────────────────────
echo ""
echo "Would you like to deploy this firmware to the remote server?"
read -p "Push $TARGET_FILENAME to 75.119.148.192 now? (y/n) " -n 1 -r
echo ""
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Skipped. To deploy later:"
    echo "  scp $TARGET_DIR/firmware.bin root@75.119.148.192:/opt/spatika-health/builds/$TARGET_FILENAME"
    exit 0
fi

HOST="75.119.148.192"
HOST_BUILDS_DIR="/opt/spatika-health/builds"  # HOST path — volume-mounted into container as /app/builds

echo "🔑 Connecting to $HOST..."

# ── 4a: Ensure the builds directory exists on the HOST (not in container)
ssh root@$HOST "mkdir -p $HOST_BUILDS_DIR"

# ── 4b: SCP directly to the HOST volume path (survives container restarts)
echo "📡 Uploading $TARGET_FILENAME to host at $HOST_BUILDS_DIR/..."
scp "$TARGET_DIR/firmware.bin" "root@$HOST:$HOST_BUILDS_DIR/$TARGET_FILENAME"

if [ $? -ne 0 ]; then
    echo "❌ SCP failed. Aborting."
    exit 1
fi
echo "✓ Binary uploaded to host filesystem"

# Inject into the running container (crucial if no volume mount exists)
echo "🧬 Injecting binary into running container..."
ssh root@$HOST "docker exec spatika-health mkdir -p /app/builds && docker cp $HOST_BUILDS_DIR/$TARGET_FILENAME spatika-health:/app/builds/$TARGET_FILENAME"
if [ $? -ne 0 ]; then
    echo "⚠️  Warning: Container injection failed. Ensure container 'spatika-health' is running."
else
    echo "✓ Binary successfully injected into container overlay"
fi

# ── 4c: Update the database BEFORE restarting (no restart needed at all)
#    The running container reads /app/builds/ which is mounted from $HOST_BUILDS_DIR
#    The new file is immediately visible to the running container — NO restart needed.
echo "💾 Updating firmware registry in database..."
ssh root@$HOST "docker exec spatika-health python3 -c \"
import sqlite3, sys
DB_PATHS = ['/app/data/health.db', '/app/SpatikaFleet.db', '/data/health.db']
conn = None
for p in DB_PATHS:
    try:
        conn = sqlite3.connect(p)
        # Verify it has our table
        conn.execute('SELECT 1 FROM firmware_registry LIMIT 1')
        print(f'Connected to DB at: {p}')
        break
    except:
        if conn: conn.close()
        conn = None

if not conn:
    print('ERROR: Could not find firmware_registry table in any DB path', file=sys.stderr)
    sys.exit(1)

c = conn.cursor()
c.execute('UPDATE firmware_registry SET current_ver=?, filename=? WHERE category_id=?',
          ('$FULL_VERSION', '$TARGET_FILENAME', $CAT_ID))
if c.rowcount == 0:
    print(f'WARNING: No row updated for category_id=$CAT_ID — check seed_db.py ran correctly')
else:
    print(f'✅ firmware_registry updated: cat_id=$CAT_ID → $FULL_VERSION / $TARGET_FILENAME')
conn.commit()
conn.close()
\""

if [ $? -ne 0 ]; then
    echo "❌ DB update failed. The file is uploaded but DB still shows old version."
    echo "   Stations will NOT receive OTA_CHECK until DB is updated."
    echo "   Run manually: docker exec spatika-health python3 -c \"import sqlite3; ...\""
    exit 1
fi

# ── 4d: NO docker restart needed — the volume mount makes the file immediately available
#    Only restart if you deployed new Python code (not firmware binaries)
echo ""
echo "✅ OTA DEPLOYMENT COMPLETE"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Config:    $CONFIG_NAME"
echo "  Version:   $FULL_VERSION"
echo "  File:      $TARGET_FILENAME"
echo "  Size:      ${BIN_SIZE} bytes"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Stations matching cat_id=$CAT_ID will receive"
echo "  OTA_CHECK on their next health report."
echo ""
echo "  Monitor: http://$HOST/dashboard"
echo ""
