#!/bin/bash
# AIO ESP32 FULL Flash Script (First-Time / Complete Erase + Flash)
# Flashes bootloader + partition table + OTA data + application
# Usage: ./flash_firmware.sh <PORT> [4mb|8mb|16mb]
#   Default flash size: 8mb

PORT=$1
FLASH_SIZE=${2:-8mb}

if [ -z "$PORT" ]; then
    echo "Usage: ./flash_firmware.sh <PORT> [4mb|8mb|16mb]"
    echo ""
    echo "Available ports:"
    ls /dev/cu.usb* 2>/dev/null
    exit 1
fi

BUILD_PATH="/tmp/aio_build_${FLASH_SIZE}"
FLASH_DIR="./flash_files/${FLASH_SIZE}"
APP_BIN="$BUILD_PATH/AIO9_5.0.ino.bin"

# Validate flash size
case "$FLASH_SIZE" in
  4mb) FLASH_MB="4MB" ;;
  8mb) FLASH_MB="8MB" ;;
  16mb) FLASH_MB="16MB" ;;
  *) echo "Unknown flash size: $FLASH_SIZE. Use 4mb, 8mb, or 16mb."; exit 1 ;;
esac

echo "======================================="
echo "  AIO9 Full Flash: $FLASH_MB ESP32"
echo "  Port: $PORT"
echo "======================================="

# Check files exist
if [ ! -f "$FLASH_DIR/bootloader.bin" ]; then
    echo "❌ Missing: $FLASH_DIR/bootloader.bin"
    exit 1
fi
if [ ! -f "$FLASH_DIR/partitions.bin" ]; then
    echo "❌ Missing: $FLASH_DIR/partitions.bin"
    exit 1
fi
if [ ! -f "$FLASH_DIR/boot_app0.bin" ]; then
    echo "❌ Missing: $FLASH_DIR/boot_app0.bin"
    exit 1
fi
if [ ! -f "$APP_BIN" ]; then
    echo "❌ App binary not found: $APP_BIN"
    echo "   Run: ./compile.sh $FLASH_SIZE  first"
    exit 1
fi

echo ""
echo "Files to flash:"
echo "  Bootloader:  $FLASH_DIR/bootloader.bin  @ 0x1000"
echo "  Partitions:  $FLASH_DIR/partitions.bin  @ 0x8000"
echo "  OTA Data:    $FLASH_DIR/boot_app0.bin   @ 0xe000"
echo "  Application: $APP_BIN @ 0x10000"
echo ""
echo "⚠️  WARNING: This will ERASE all data on the device!"
read -p "Continue? (y/N) " confirm
if [[ "$confirm" != "y" && "$confirm" != "Y" ]]; then
    echo "Aborted."
    exit 0
fi

esptool.py --chip esp32 --port "$PORT" --baud 921600 \
    --before default_reset --after hard_reset \
    write_flash \
    --flash_mode dio \
    --flash_freq 80m \
    --flash_size "${FLASH_MB}" \
    0x1000  "$FLASH_DIR/bootloader.bin" \
    0x8000  "$FLASH_DIR/partitions.bin" \
    0xe000  "$FLASH_DIR/boot_app0.bin" \
    0x10000 "$APP_BIN"

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Full Flash Complete!"
else
    echo ""
    echo "❌ Flash Failed!"
    exit 1
fi
