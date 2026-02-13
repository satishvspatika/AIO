#!/bin/bash

# AIO Web Flash Exporter
# Gathers the 4 files needed for online/manual flashing tools
# Usage: ./export_web_flash.sh [CONFIG_NAME]

CONFIG_NAME=$1
OUT_DIR="WEB_FLASH_FILES"

if [ -n "$CONFIG_NAME" ]; then
    # Grab from an official build folder
    # Common files (bootloader, partitions) are in the shared build folder
    B_DIR="./build/all_configs_shared"
    APP_BIN="./builds/$CONFIG_NAME/firmware.bin"
    if [ ! -f "$APP_BIN" ]; then
        echo "❌ Error: Configuration '$CONFIG_NAME' not found in builds/ folder."
        echo "Did you run 'python3 build_all_configs.py' first?"
        exit 1
    fi
else
    # Grab from the current/temporary build folder
    B_DIR="./build/esp32.esp32.esp32"
    APP_BIN="$B_DIR/AIO9_5.0.ino.bin"
    if [ ! -d "$B_DIR" ]; then
        echo "❌ Error: Default build directory not found. Please run ./compile.sh first."
        exit 1
    fi
fi

mkdir -p "$OUT_DIR"

echo "--- Gathering Web Flash Files for: ${CONFIG_NAME:-Current Build} ---"

cp "$APP_BIN" "$OUT_DIR/application.bin"
cp "$B_DIR/AIO9_5.0.ino.bootloader.bin" "$OUT_DIR/bootloader.bin"
cp "$B_DIR/AIO9_5.0.ino.partitions.bin" "$OUT_DIR/partitions.bin"
cp "$B_DIR/boot_app0.bin" "$OUT_DIR/boot_app0.bin"

echo "✅ Done! Files are ready in the '$OUT_DIR' folder:"
ls -lh "$OUT_DIR"
echo ""
echo "Send these to team members for use with Online ESP Tools."
