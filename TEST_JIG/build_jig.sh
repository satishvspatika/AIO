#!/bin/bash

# Spatika JIG Build and Packaging Script
# Automates the compilation of:
#   1. QC Self-Test Firmware (4MB, 8MB, 16MB)
#   2. Production Application (4MB, 8MB, 16MB)
# And bundles them into the self-contained WEB_FLASH_FILES/ directory.

# Ensure we run from the TEST_JIG directory or workspace root
WORKSPACE_ROOT="/Users/satishkripavasan/Documents/Arduino/ESP32_NEW_DESIGN/ALL_IN_ONE/AIO9_5.0"
TEST_JIG_DIR="$WORKSPACE_ROOT/TEST_JIG"
OUT_DIR="$TEST_JIG_DIR/WEB_FLASH_FILES"
ARDUINO_CLI="/usr/local/bin/arduino-cli"

echo "=================================================="
echo "          SPATIKA TEST JIG BUILD SYSTEM           "
echo "=================================================="
echo "Workspace Root: $WORKSPACE_ROOT"
echo "Output Directory: $OUT_DIR"
echo "=================================================="

# Check arduino-cli
if [ ! -f "$ARDUINO_CLI" ]; then
    echo "❌ Error: arduino-cli not found at $ARDUINO_CLI"
    exit 1
fi

# Create target directories
mkdir -p "$OUT_DIR"

# Temporary build directory paths
QC_BUILD_BASE="/tmp/qc_jig_build"
APP_BUILD_BASE="/tmp/app_jig_build"
rm -rf "$QC_BUILD_BASE" "$APP_BUILD_BASE"

# --- 1. COMPILE QC TEST FIRMWARE (4MB, 8MB, 16MB) ---
echo "--- Compiling QC Self-Test Firmware ---"

# Compile 4MB QC Test
echo "→ Building 4MB QC Test..."
cp "$WORKSPACE_ROOT/partitions_4mb.csv" "$TEST_JIG_DIR/qc_test/partitions.csv"
$ARDUINO_CLI compile \
    --fqbn "esp32:esp32:esp32:FlashSize=4M,PartitionScheme=custom" \
    --build-property "build.partitions=custom" \
    --build-property "upload.maximum_size=1310720" \
    --build-path "$QC_BUILD_BASE/4mb" \
    "$TEST_JIG_DIR/qc_test/qc_test.ino"

if [ $? -ne 0 ]; then
    echo "❌ QC 4MB compile failed!"
    rm -f "$TEST_JIG_DIR/qc_test/partitions.csv"
    exit 1
fi

# Compile 8MB QC Test
echo "→ Building 8MB QC Test..."
cp "$WORKSPACE_ROOT/partitions.csv" "$TEST_JIG_DIR/qc_test/partitions.csv"
$ARDUINO_CLI compile \
    --fqbn "esp32:esp32:esp32:FlashSize=8M,PartitionScheme=custom" \
    --build-property "build.partitions=custom" \
    --build-property "upload.maximum_size=1769472" \
    --build-path "$QC_BUILD_BASE/8mb" \
    "$TEST_JIG_DIR/qc_test/qc_test.ino"

if [ $? -ne 0 ]; then
    echo "❌ QC 8MB compile failed!"
    rm -f "$TEST_JIG_DIR/qc_test/partitions.csv"
    exit 1
fi

# Compile 16MB QC Test
echo "→ Building 16MB QC Test..."
cp "$WORKSPACE_ROOT/partitions_16mb.csv" "$TEST_JIG_DIR/qc_test/partitions.csv"
$ARDUINO_CLI compile \
    --fqbn "esp32:esp32:esp32:FlashSize=16M,PartitionScheme=custom" \
    --build-property "build.partitions=custom" \
    --build-property "upload.maximum_size=2097152" \
    --build-path "$QC_BUILD_BASE/16mb" \
    "$TEST_JIG_DIR/qc_test/qc_test.ino"

if [ $? -ne 0 ]; then
    echo "❌ QC 16MB compile failed!"
    rm -f "$TEST_JIG_DIR/qc_test/partitions.csv"
    exit 1
fi

# Clean up QC partitions
rm -f "$TEST_JIG_DIR/qc_test/partitions.csv"


# --- 2. COMPILE PRODUCTION APP (4MB, 8MB, 16MB) ---
echo "--- Compiling Production Application ---"

# Save original user_config.h and partitions.csv
cp "$WORKSPACE_ROOT/user_config.h" /tmp/user_config_backup.h
cp "$WORKSPACE_ROOT/partitions.csv" /tmp/partitions_backup.csv

# Build 4MB Production App (Patching WebServer OFF to fit slot)
echo "→ Building 4MB Production Application..."
  python3 -c 'import re; p="'"$WORKSPACE_ROOT"'/user_config.h"; c=open(p).read(); c=re.sub(r"(#define ENABLE_WEBSERVER\s*\\?\s*\n?\s*)1", r"\g<1>0", c); open(p,"w").write(c)'
cp "$WORKSPACE_ROOT/partitions_4mb.csv" "$WORKSPACE_ROOT/partitions.csv"

$ARDUINO_CLI compile \
    --fqbn "esp32:esp32:esp32:FlashSize=4M,PartitionScheme=custom" \
    --build-property "build.partitions=custom" \
    --build-property "upload.maximum_size=1310720" \
    --build-path "$APP_BUILD_BASE/4mb" \
    "$WORKSPACE_ROOT"

if [ $? -ne 0 ]; then
    echo "❌ App 4MB compile failed!"
    cp /tmp/user_config_backup.h "$WORKSPACE_ROOT/user_config.h"
    cp /tmp/partitions_backup.csv "$WORKSPACE_ROOT/partitions.csv"
    exit 1
fi

# Restore original files
cp /tmp/user_config_backup.h "$WORKSPACE_ROOT/user_config.h"
cp /tmp/partitions_backup.csv "$WORKSPACE_ROOT/partitions.csv"

# Build 8MB Production App
echo "→ Building 8MB Production Application..."
$ARDUINO_CLI compile \
    --fqbn "esp32:esp32:esp32:FlashSize=8M,PartitionScheme=custom" \
    --build-property "build.partitions=custom" \
    --build-property "upload.maximum_size=1769472" \
    --build-path "$APP_BUILD_BASE/8mb" \
    "$WORKSPACE_ROOT"

if [ $? -ne 0 ]; then
    echo "❌ App 8MB compile failed!"
    exit 1;
fi

# Build 16MB Production App
echo "→ Building 16MB Production Application..."
cp "$WORKSPACE_ROOT/partitions_16mb.csv" "$WORKSPACE_ROOT/partitions.csv"

$ARDUINO_CLI compile \
    --fqbn "esp32:esp32:esp32:FlashSize=16M,PartitionScheme=custom" \
    --build-property "build.partitions=custom" \
    --build-property "upload.maximum_size=2097152" \
    --build-path "$APP_BUILD_BASE/16mb" \
    "$WORKSPACE_ROOT"

if [ $? -ne 0 ]; then
    echo "❌ App 16MB compile failed!"
    cp /tmp/partitions_backup.csv "$WORKSPACE_ROOT/partitions.csv"
    exit 1
fi

# Restore original root partitions
cp /tmp/partitions_backup.csv "$WORKSPACE_ROOT/partitions.csv"


# --- 3. GATHER AND BUNDLE TARGET ARTIFACTS ---
echo "--- Gathering and Bundling Artifacts ---"

# Copy common boot sector files from the 8MB compile path
cp "$QC_BUILD_BASE/8mb/qc_test.ino.bootloader.bin" "$OUT_DIR/bootloader.bin"
cp "$WORKSPACE_ROOT/flash_files/boot_app0.bin" "$OUT_DIR/boot_app0.bin"

# Copy compiled partition binaries
cp "$QC_BUILD_BASE/4mb/qc_test.ino.partitions.bin" "$OUT_DIR/partitions_4mb.bin"
cp "$QC_BUILD_BASE/8mb/qc_test.ino.partitions.bin" "$OUT_DIR/partitions_8mb.bin"
cp "$QC_BUILD_BASE/16mb/qc_test.ino.partitions.bin" "$OUT_DIR/partitions_16mb.bin"

# Copy QC self-test app binaries
cp "$QC_BUILD_BASE/4mb/qc_test.ino.bin" "$OUT_DIR/qc_test_4mb.bin"
cp "$QC_BUILD_BASE/8mb/qc_test.ino.bin" "$OUT_DIR/qc_test_8mb.bin"
cp "$QC_BUILD_BASE/16mb/qc_test.ino.bin" "$OUT_DIR/qc_test_16mb.bin"

# Copy Production app binaries
cp "$APP_BUILD_BASE/4mb/AIO9_5.0.bin" "$OUT_DIR/production_4mb.bin" 2>/dev/null || cp "$APP_BUILD_BASE/4mb/AIO9_5.0.ino.bin" "$OUT_DIR/production_4mb.bin"
cp "$APP_BUILD_BASE/8mb/AIO9_5.0.bin" "$OUT_DIR/production_8mb.bin" 2>/dev/null || cp "$APP_BUILD_BASE/8mb/AIO9_5.0.ino.bin" "$OUT_DIR/production_8mb.bin"
cp "$APP_BUILD_BASE/16mb/AIO9_5.0.bin" "$OUT_DIR/production_16mb.bin" 2>/dev/null || cp "$APP_BUILD_BASE/16mb/AIO9_5.0.ino.bin" "$OUT_DIR/production_16mb.bin"

# Copy the HTML serial portal dashboard
cp "$TEST_JIG_DIR/factory_tool.html" "$OUT_DIR/factory_tool.html"

# Extract Firmware Version from user_config.h and write to version.txt
FW_VER=$(grep '#define FIRMWARE_VERSION' "$WORKSPACE_ROOT/user_config.h" | sed 's/.*"\(.*\)".*/\1/')
echo "$FW_VER" > "$OUT_DIR/version.txt"
echo "→ Packaged version.txt: $FW_VER"

# Clean up temp
rm -rf "$QC_BUILD_BASE" "$APP_BUILD_BASE"


# --- 4. BUILD ALL NAMED CONFIGS (email-ZIP structure) ---
# Compiles every SYSTEM/UNIT combination (KSNDMC_TRG, BIHAR_TRG, KSNDMC_TWS …)
# and copies the resulting firmware.bin + fw_version.txt + metadata.json into
# WEB_FLASH_FILES/<CONFIG_NAME>/ — identical to the email ZIP layout.
# This makes WEB_FLASH_FILES fully self-contained and portable.
echo ""
echo "=================================================="
echo "  STEP 4: Building Named Release Configurations   "
echo "=================================================="

if [ -f "$WORKSPACE_ROOT/build_all_configs.py" ]; then
    echo "→ Running build_all_configs.py (8MB targets)..."
    cd "$WORKSPACE_ROOT"
    python3 build_all_configs.py --flash 8mb
    BUILD_STATUS=$?

    if [ $BUILD_STATUS -ne 0 ]; then
        echo "❌ build_all_configs.py failed. Named configs will not be included."
    else
        echo ""
        echo "--- Copying named config subdirs to WEB_FLASH_FILES ---"
        BUILDS_DIR="$WORKSPACE_ROOT/builds"

        for config_dir in "$BUILDS_DIR"/*/; do
            config_name=$(basename "$config_dir")
            src_bin="$config_dir/firmware.bin"
            src_ver="$config_dir/fw_version.txt"
            src_meta="$config_dir/metadata.json"

            if [ -f "$src_bin" ]; then
                dest="$OUT_DIR/$config_name"
                mkdir -p "$dest"
                cp "$src_bin" "$dest/firmware.bin"
                [ -f "$src_ver"  ] && cp "$src_ver"  "$dest/fw_version.txt"
                [ -f "$src_meta" ] && cp "$src_meta" "$dest/metadata.json"
                SIZE=$(du -sh "$src_bin" | cut -f1)
                echo "  ✓ $config_name ($SIZE)"
            fi
        done
        echo "→ Named configs packaged."
    fi
else
    echo "⚠  build_all_configs.py not found — skipping named configs."
fi


echo "=================================================="
echo "✅ SUCCESS: All build files packaged in:"
echo "   $OUT_DIR"
echo "=================================================="
echo ""
echo "Directory structure:"
ls -lh "$OUT_DIR"
echo ""
echo "Named config subdirectories:"
for d in "$OUT_DIR"/*/; do
    [ -d "$d" ] && echo "  📁 $(basename $d)/"
done
echo "=================================================="
