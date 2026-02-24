#!/bin/bash
# AIO ESP32 QUICK Flashing Script (App Only)
# Usage: ./quick_flash.sh <PORT> [4mb|8mb|16mb]
#   Default flash size: 8mb

PORT=$1
FLASH_SIZE=${2:-8mb}

if [ -z "$PORT" ]; then
    echo "Usage: ./quick_flash.sh <PORT> [4mb|8mb|16mb]"
    ls /dev/cu.usb* 2>/dev/null
    exit 1
fi

BUILD_PATH="/tmp/aio_build_${FLASH_SIZE}"
APP_BIN="$BUILD_PATH/AIO9_5.0.ino.bin"

if [ ! -f "$APP_BIN" ]; then
    echo "Error: Binary not found at $APP_BIN"
    echo "Run: ./compile.sh $FLASH_SIZE  first"
    exit 1
fi

echo "--- QUICK FLASH (App Only at 0x10000) | ${FLASH_SIZE} | Port: $PORT ---"
esptool.py --chip esp32 --port "$PORT" --baud 921600 write_flash 0x10000 "$APP_BIN"
echo "--- Quick Flash Complete ---"
