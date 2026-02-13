#!/bin/bash

# AIO ESP32 Build and Quick Flash Script
# Automates: Compilation -> Finding Port -> App-only Flash

# 1. Compile the code
./compile.sh
if [ $? -ne 0 ]; then
    echo "❌ Compilation failed. Aborting flash."
    exit 1
fi

# 2. Determine Port
PORT=$1
if [ -z "$PORT" ]; then
    # Try your preferred port first
    MY_PORT="/dev/cu.usbserial-A50285BI"
    if [ -e "$MY_PORT" ]; then
        PORT="$MY_PORT"
    else
        # Intelligent Auto-detection
        PORT=$(ls /dev/cu.usbserial-* 2>/dev/null | head -1) # Try any usbserial
        if [ -z "$PORT" ]; then
            PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1) # Try any usbmodem
        fi
    fi
fi

if [ -z "$PORT" ]; then
    echo "❌ Error: Could not find your ESP32. Please connect it or specify port manually."
    echo "Usage: ./build_and_flash.sh [PORT]"
    exit 1
fi

# New: Auto-release the port if it's busy (kills existing serial monitors)
echo "🔍 Checking if $PORT is busy..."
# On macOS, lsof is more reliable for finding serial port usage
PID=$(lsof -t "$PORT")
if [ -n "$PID" ]; then
    echo "⚠️  Port $PORT is being used by PID: $PID. Releasing..."
    kill -9 $PID
    sleep 1 # Give it a moment to release
else
    # Fallback for other environments or if lsof didn't catch it
    fuser -k "$PORT" 2>/dev/null || true
fi

# Specifically check for any arduino-cli monitor processes that might be lingering
ARDUINO_PID=$(ps aux | grep "arduino-cli monitor" | grep "$PORT" | grep -v grep | awk '{print $2}')
if [ -n "$ARDUINO_PID" ]; then
    echo "⚠️  Found active arduino-cli monitor for $PORT. Killing PID: $ARDUINO_PID"
    kill -9 $ARDUINO_PID
    sleep 1
fi

# 3. Flash the app
echo "🚀 Target Port Detected: $PORT"
./quick_flash.sh "$PORT"
