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
APP_BIN="$BUILD_PATH/AIO9_5.0.ino.bin"

if [ ! -f "$APP_BIN" ]; then
    echo "Error: Binary not found at $APP_BIN"
    echo "Run: ./compile.sh $FLASH_SIZE  first"
    exit 1
fi

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

BAUD="${BAUD:-460800}"

if [ $DO_FULL -eq 1 ]; then
    BOOTLOADER="$BUILD_PATH/AIO9_5.0.ino.bootloader.bin"
    PARTITIONS="$BUILD_PATH/AIO9_5.0.ino.partitions.bin"
    BOOT_APP="./flash_files/boot_app0.bin"
    
    if [ ! -f "$BOOT_APP" ]; then
        BOOT_APP="./flash_files/${FLASH_SIZE}/boot_app0.bin"
    fi
    
    echo "--- FULL FLASH (Bootloader, Partitions, App) | ${FLASH_SIZE} | Port: $PORT | Baud: $BAUD ---"
    
    case "$FLASH_SIZE" in
      4mb) FLASH_MB="4MB"; APP1_OFFSET="0x150000" ;;
      8mb) FLASH_MB="8MB"; APP1_OFFSET="0x1C0000" ;;
      16mb) FLASH_MB="16MB"; APP1_OFFSET="0x210000" ;;
      *) FLASH_MB="detect"; APP1_OFFSET="0x1C0000" ;;
    esac

    # Erase otadata partition so bootloader defaults to app0 at 0x10000
    esptool.py --chip esp32 --port "$PORT" erase_region 0xe000 0x2000
    
    esptool.py --chip esp32 --port "$PORT" --baud $BAUD \
        --before default_reset --after hard_reset \
        write_flash \
        --flash_mode dio \
        --flash_freq 80m \
        --flash_size "${FLASH_MB}" \
        0x1000  "$BOOTLOADER" \
        0x8000  "$PARTITIONS" \
        0xe000  "$BOOT_APP" \
        0x10000 "$APP_BIN" \
        "$APP1_OFFSET" "$APP_BIN"
else
    case "$FLASH_SIZE" in
      4mb) APP1_OFFSET="0x150000" ;;
      8mb) APP1_OFFSET="0x1C0000" ;;
      16mb) APP1_OFFSET="0x210000" ;;
      *) APP1_OFFSET="0x1C0000" ;;
    esac
    echo "--- QUICK FLASH (App0 & App1) | ${FLASH_SIZE} | Port: $PORT | Baud: $BAUD ---"
    esptool.py --chip esp32 --port "$PORT" erase_region 0xe000 0x2000
    esptool.py --chip esp32 --port "$PORT" --baud $BAUD write_flash 0x10000 "$APP_BIN" "$APP1_OFFSET" "$APP_BIN"
fi


echo "--- Flash Complete ---"

