#!/bin/bash

# AIO Web Flash Exporter
# Gathers the 4 files needed for online/manual flashing tools

B_DIR="./build/esp32.esp32.esp32"
OUT_DIR="WEB_FLASH_FILES"

if [ ! -d "$B_DIR" ]; then
    echo "❌ Error: Build directory not found. Please run ./compile.sh first."
    exit 1
fi

mkdir -p "$OUT_DIR"

echo "--- Gathering Web Flash Files ---"

cp "$B_DIR/AIO9_5.0.ino.bin" "$OUT_DIR/application.bin"
cp "$B_DIR/AIO9_5.0.ino.bootloader.bin" "$OUT_DIR/bootloader.bin"
cp "$B_DIR/AIO9_5.0.ino.partitions.bin" "$OUT_DIR/partitions.bin"
cp "$B_DIR/boot_app0.bin" "$OUT_DIR/boot_app0.bin"

echo "✅ Done! Files are ready in the '$OUT_DIR' folder:"
ls -lh "$OUT_DIR"
echo ""
echo "Send these to team members for use with Online ESP Tools."
