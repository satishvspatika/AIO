#!/bin/bash

# AIO ESP32 QUICK Flashing Script (App Only)
# Usage: ./quick_flash.sh <PORT>

PORT=$1

if [ -z "$PORT" ]; then
    echo "Usage: ./quick_flash.sh <PORT>"
    ls /dev/cu.usb*
    exit 1
fi

# Looking for the binary in the default export location
B_DIR="./build/esp32.esp32.esp32"
APP_BIN="$B_DIR/AIO9_5.0.ino.bin"

if [ ! -f "$APP_BIN" ]; then
    echo "Error: App binary not found at $APP_BIN"
    echo "Did you run 'Sketch -> Export Compiled Binary' in the IDE?"
    exit 1
fi

echo "--- QUICK FLASH (App Only at 0x10000) on $PORT ---"

esptool.py --chip esp32 --port "$PORT" --baud 921600 write_flash 0x10000 "$APP_BIN"

echo "--- Quick Flash Complete ---"
