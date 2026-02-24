#!/bin/bash
# AIO ESP32 Compilation Script
# Usage: ./compile.sh [4mb|8mb|16mb]
#   Default: 8mb (current production hardware)

FLASH_SIZE=${1:-8mb}

case "$FLASH_SIZE" in
  4mb)
    PARTITION_FILE="partitions_4mb.csv"
    FQBN="esp32:esp32:esp32:FlashSize=4M,PartitionScheme=custom"
    echo "--- Building for 4MB Flash ESP32 ---"
    ;;
  8mb)
    PARTITION_FILE="partitions.csv"
    FQBN="esp32:esp32:esp32:FlashSize=8M,PartitionScheme=custom"
    echo "--- Building for 8MB Flash ESP32 (Default) ---"
    ;;
  16mb)
    PARTITION_FILE="partitions_16mb.csv"
    FQBN="esp32:esp32:esp32:FlashSize=16M,PartitionScheme=custom"
    echo "--- Building for 16MB Flash ESP32 ---"
    ;;
  *)
    echo "Unknown flash size: $FLASH_SIZE"
    echo "Usage: ./compile.sh [4mb|8mb|16mb]"
    exit 1
    ;;
esac

echo "--- Using partition file: $PARTITION_FILE ---"

# Get firmware version from globals.h
FW_VER=$(grep '#define FIRMWARE_VERSION' globals.h | sed 's/.*"\(.*\)".*/\1/')
BUILD_PATH="/tmp/aio_build_${FLASH_SIZE}"

# 4MB builds: temporarily disable WebServer to fit within 1.25MB slot
PATCHED=0
if [ "$FLASH_SIZE" = "4mb" ]; then
    echo "--- 4MB build: patching ENABLE_WEBSERVER=0 ---"
    cp globals.h /tmp/globals_backup.h
    sed -i '' 's/#define ENABLE_WEBSERVER 1/#define ENABLE_WEBSERVER 0/' globals.h
    PATCHED=1
fi

arduino-cli compile \
    --fqbn "$FQBN" \
    --build-property "build.partitions=custom" \
    --build-property "build.custom_partitions=$(pwd)/$PARTITION_FILE" \
    --build-path "$BUILD_PATH" \
    --export-binaries \
    .

BUILD_RESULT=$?

# Restore globals.h if patched
if [ $PATCHED -eq 1 ]; then
    cp /tmp/globals_backup.h globals.h
    echo "--- globals.h restored ---"
fi

if [ $BUILD_RESULT -eq 0 ]; then
    echo ""
    echo "--- Compilation Successful (v${FW_VER} / ${FLASH_SIZE}) ---"
    echo "Binary: $BUILD_PATH/AIO9_5.0.ino.bin"
    echo ""
    echo "To flash: ./quick_flash.sh [PORT] $FLASH_SIZE"
else
    echo "--- Compilation FAILED ---"
    exit 1
fi

