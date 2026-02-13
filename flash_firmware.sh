#!/bin/bash

# AIO ESP32 Flashing Script
# Usage: ./flash_firmware.sh <PORT>
# Example: ./flash_firmware.sh /dev/cu.usbserial-A50285BI

PORT=$1

if [ -z "$PORT" ]; then
    echo "Usage: ./flash_firmware.sh <PORT>"
    echo "Available ports:"
    ls /dev/cu.usb*
    exit 1
fi

B_DIR="./build/esp32.esp32.esp32"

echo "--- Starting Flash on $PORT ---"

esptool.py --chip esp32 --port "$PORT" --baud 921600 write_flash \
    0x1000  "$B_DIR/AIO9_5.0.ino.bootloader.bin" \
    0x8000  "$B_DIR/AIO9_5.0.ino.partitions.bin" \
    0xe000  "$B_DIR/boot_app0.bin" \
    0x10000 "$B_DIR/AIO9_5.0.ino.bin"

echo "--- Flash Complete ---"
