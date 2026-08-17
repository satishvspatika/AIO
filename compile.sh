#!/bin/bash
# AIO ESP32 Compilation Script
# Usage: ./compile.sh [4mb|8mb|16mb]
#   Default: 8mb (current production hardware)

FLASH_SIZE=${1:-8mb}
PATCHED=0

case "$FLASH_SIZE" in
  4mb)
    PARTITION_FILE="partitions_4mb.csv"
    FQBN="esp32:esp32:esp32:FlashSize=4M,FlashMode=dio,PartitionScheme=custom"
    echo "--- Building for 4MB Flash ESP32 ---"
    ;;
  8mb)
    PARTITION_FILE="partitions.csv"
    FQBN="esp32:esp32:esp32:FlashSize=8M,FlashMode=dio,PartitionScheme=custom"
    echo "--- Building for 8MB Flash ESP32 (Default) ---"
    ;;
  16mb)
    PARTITION_FILE="partitions_16mb.csv"
    FQBN="esp32:esp32:esp32:FlashSize=16M,FlashMode=dio,FlashFreq=80,PartitionScheme=custom"
    echo "--- Building for 16MB Flash ESP32 ---"
    ;;
  *)
    echo "Unknown flash size: $FLASH_SIZE"
    echo "Usage: ./compile.sh [4mb|8mb|16mb]"
    exit 1
    ;;
esac

echo "--- Using partition file: $PARTITION_FILE ---"

USER_NAME=${USER:-satishkripavasan}
BUILD_PATH="/tmp/aio_build_${USER_NAME}_${FLASH_SIZE}"
FW_VER=$(grep '#define FIRMWARE_VERSION' user_config.h | sed 's/.*"\(.*\)".*/\1/')

# 4MB builds: temporarily disable WebServer to fit within 1.25MB slot
if [ "$FLASH_SIZE" = "4mb" ]; then
    echo "--- 4MB build: patching ENABLE_WEBSERVER=0 ---"
    cp user_config.h /tmp/user_config_backup.h
    sed 's/#define ENABLE_WEBSERVER 1/#define ENABLE_WEBSERVER 0/' user_config.h > /tmp/user_config_patched.h
    mv /tmp/user_config_patched.h user_config.h
    PATCHED=1
fi

# Clean stale build artifacts to prevent object file corruption
chflags -R nouchg "$BUILD_PATH" 2>/dev/null || true
chmod -R 777 "$BUILD_PATH" 2>/dev/null || true
rm -rf "$BUILD_PATH"

/usr/local/bin/arduino-cli compile \
    --fqbn "$FQBN" \
    --build-property "build.partitions=custom" \
    --build-property "build.custom_partitions=$(pwd)/$PARTITION_FILE" \
    --build-property "upload.maximum_size=1769472" \
    --build-path "$BUILD_PATH" \
    --clean \
    --export-binaries \
    .

BUILD_RESULT=$?

# Restore user_config.h if patched
if [ "$PATCHED" -eq 1 ]; then
    cp /tmp/user_config_backup.h user_config.h
    echo "--- user_config.h restored ---"
fi

if [ "$BUILD_RESULT" -eq 0 ]; then
    echo ""
    echo "--- Compilation Successful (v${FW_VER} / ${FLASH_SIZE}) ---"
    echo "Binary: $BUILD_PATH/AIO9_5.0.ino.bin"
    echo ""
    echo "To flash: ./quick_flash.sh [PORT] $FLASH_SIZE"
else
    echo "--- Compilation FAILED ---"
    exit 1
fi
