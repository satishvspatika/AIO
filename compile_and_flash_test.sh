#!/bin/bash
PORT=${1:-/dev/cu.usbserial-A5069RR4}
echo "--- Compiling Standalone Test Sketch: STANDALONE_MODEM_TEST.ino ---"

BUILD_PATH="/tmp/standalone_test_build"

/usr/local/bin/arduino-cli compile \
    --clean \
    --fqbn "esp32:esp32:esp32:FlashSize=8M,FlashMode=dio" \
    --build-path "$BUILD_PATH" \
    --export-binaries \
    ./STANDALONE_MODEM_TEST

if [ $? -eq 0 ]; then
    echo "--- Compilation Successful ---"
    echo "--- Flashing Standalone Test Sketch to $PORT ---"
    python3 -m esptool --chip esp32 --port "$PORT" --baud 460800 write_flash -z 0x10000 "$BUILD_PATH/STANDALONE_MODEM_TEST.ino.bin"
    echo "--- Flash Complete! ---"
else
    echo "--- Compilation Failed ---"
    exit 1
fi
