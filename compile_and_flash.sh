#!/bin/bash
# AIO ESP32 Build and Quick Flash Script
# Automates: Compile -> Find Port -> App-only Flash
# Usage: ./compile_and_flash.sh [PORT] [4mb|8mb|16mb]
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

# 1. Compile for the correct flash target
./compile.sh "$FLASH_SIZE"
if [ $? -ne 0 ]; then
    echo "❌ Compilation failed. Aborting flash."
    exit 1
fi

# 2. Determine Port
if [ -z "$PORT" ]; then
    MY_PORT="/dev/cu.usbserial-A50285BI"
    if [ -e "$MY_PORT" ]; then
        PORT="$MY_PORT"
    else
        PORT=$(ls /dev/cu.usbserial-* 2>/dev/null | head -1)
        if [ -z "$PORT" ]; then
            PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1)
        fi
    fi
fi

if [ -z "$PORT" ]; then
    echo "❌ Error: Could not find your ESP32. Connect it or specify port manually."
    echo "Usage: ./compile_and_flash.sh [PORT] [4mb|8mb|16mb] [--full]"
    exit 1
fi

# 3. Release port if busy
echo "🔍 Checking if $PORT is busy..."
PID=$(lsof -t "$PORT")
if [ -n "$PID" ]; then
    echo "⚠️  Port $PORT is being used by PID: $PID. Releasing..."
    kill -9 $PID
    sleep 1
else
    fuser -k "$PORT" 2>/dev/null || true
fi

ARDUINO_PID=$(ps aux | grep "arduino-cli monitor" | grep "$PORT" | grep -v grep | awk '{print $2}')
if [ -n "$ARDUINO_PID" ]; then
    echo "⚠️  Found active arduino-cli monitor for $PORT. Killing PID: $ARDUINO_PID"
    kill -9 $ARDUINO_PID
    sleep 1
fi

# 4. Flash (forward --full if present)
FULL_FLAG=""
if [ $DO_FULL -eq 1 ]; then
    FULL_FLAG="--full"
fi

echo "🚀 Target Port: $PORT | Flash: $FLASH_SIZE | Full: ${FULL_FLAG:-no}"
./quick_flash.sh "$PORT" "$FLASH_SIZE" $FULL_FLAG

