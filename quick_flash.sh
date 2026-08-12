#!/bin/bash
# AIO ESP32 QUICK Flashing Script (App Only)
# Usage: ./quick_flash.sh <PORT> [4mb|8mb|16mb]
#   Default flash size: 8mb

PORT=""
FLASH_SIZE="8mb"
DO_FULL=0

for arg in "$@"; do
    case "$arg" in
        4mb|8mb|16mb)
            FLASH_SIZE="$arg"
            ;;
        --full)
            DO_FULL=1
            ;;
        *)
            if [ -n "$arg" ]; then
                PORT="$arg"
            fi
            ;;
    esac
done

if [ -z "$PORT" ]; then
    echo "Usage: ./quick_flash.sh <PORT> [4mb|8mb|16mb] [--full]"
    ls /dev/cu.usb* 2>/dev/null
    exit 1
fi


BUILD_PATH="/tmp/aio_build_${FLASH_SIZE}"

# Find APP_BIN
APP_BIN=""
for candidate in \
    "$BUILD_PATH/AIO9_5.0.ino.bin" \
    "./build/esp32.esp32.esp32/AIO9_5.0.ino.bin" \
    "$BUILD_PATH/sketch/AIO9_5.0.ino.bin" \
    "./builds/KSNDMC_TRG_MAT_${FLASH_SIZE}/firmware.bin" \
    "./build/config_KSNDMC_TRG_MAT_${FLASH_SIZE}/AIO9_5.0.ino.bin"
do
    if [ -f "$candidate" ]; then
        APP_BIN="$candidate"
        break
    fi
done

if [ -z "$APP_BIN" ]; then
    echo "Error: Binary not found for ${FLASH_SIZE}"
    echo "Run: ./compile.sh $FLASH_SIZE  first"
    exit 1
fi
echo "📦 Using Firmware Binary: $APP_BIN"

# Release port if busy
echo "🔍 Checking if $PORT is busy..."
PID=$(lsof -t "$PORT")
if [ -n "$PID" ]; then
    echo "⚠️  Port $PORT is being used by PID: $PID. Releasing..."
    kill -9 $PID
    sleep 1
else
    fuser -k "$PORT" 2>/dev/null || true
fi

if [ $DO_FULL -eq 1 ]; then
    BOOTLOADER=""
    for candidate in \
        "$BUILD_PATH/AIO9_5.0.ino.bootloader.bin" \
        "$BUILD_PATH/sketch/AIO9_5.0.ino.bootloader.bin" \
        "./build/config_KSNDMC_TRG_MAT_${FLASH_SIZE}/AIO9_5.0.ino.bootloader.bin" \
        "./build/config_KSNDMC_TRG_NUV_${FLASH_SIZE}/AIO9_5.0.ino.bootloader.bin" \
        "./flash_files/${FLASH_SIZE}/bootloader.bin" \
        "./flash_files/bootloader.bin"
    do
        if [ -f "$candidate" ]; then BOOTLOADER="$candidate"; break; fi
    done

    PARTITIONS=""
    for candidate in \
        "$BUILD_PATH/AIO9_5.0.ino.partitions.bin" \
        "$BUILD_PATH/sketch/AIO9_5.0.ino.partitions.bin" \
        "./build/config_KSNDMC_TRG_MAT_${FLASH_SIZE}/AIO9_5.0.ino.partitions.bin" \
        "./build/config_KSNDMC_TRG_NUV_${FLASH_SIZE}/AIO9_5.0.ino.partitions.bin" \
        "./flash_files/${FLASH_SIZE}/partitions.bin" \
        "./flash_files/partitions.bin"
    do
        if [ -f "$candidate" ]; then PARTITIONS="$candidate"; break; fi
    done
    BOOT_APP="./flash_files/boot_app0.bin"
    
    if [ ! -f "$BOOT_APP" ]; then
        BOOT_APP="./flash_files/${FLASH_SIZE}/boot_app0.bin"
    fi
    
    echo "--- FULL FLASH (Bootloader, Partitions, App) | ${FLASH_SIZE} | Port: $PORT ---"
    
    case "$FLASH_SIZE" in
      4mb) FLASH_MB="4MB" ;;
      8mb) FLASH_MB="8MB" ;;
      16mb) FLASH_MB="16MB" ;;
      *) FLASH_MB="detect" ;;
    esac
    
    esptool.py --chip esp32 --port "$PORT" --baud 921600 \
        --before default_reset --after hard_reset \
        write_flash \
        --flash_mode dio \
        --flash_freq 80m \
        --flash_size "${FLASH_MB}" \
        0x1000  "$BOOTLOADER" \
        0x8000  "$PARTITIONS" \
        0xe000  "$BOOT_APP" \
        0x10000 "$APP_BIN"
else
    echo "--- QUICK FLASH (App Only at 0x10000) | ${FLASH_SIZE} | Port: $PORT ---"
    esptool.py --chip esp32 --port "$PORT" --baud 921600 write_flash 0x10000 "$APP_BIN"
fi

echo "--- Flash Complete ---"

