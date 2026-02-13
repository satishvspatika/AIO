#!/bin/bash
# Helper script to organize Arduino IDE compiled binaries
# Usage: ./organize_build.sh CONFIG_NAME

CONFIG_NAME=$1

if [ -z "$CONFIG_NAME" ]; then
    echo "Usage: ./organize_build.sh CONFIG_NAME"
    echo ""
    echo "Available configurations:"
    echo "  TRG_KSNDMC      (SYSTEM=0, UNIT=KSNDMC_TRG)"
    echo "  TRG_BIHAR       (SYSTEM=0, UNIT=BIHAR_TRG)"
    echo "  TRG_SPATIKA     (SYSTEM=0, UNIT=SPATIKA_GEN)"
    echo "  TWS_KSNDMC      (SYSTEM=1, UNIT=KSNDMC_TWS)"
    echo "  TWSRF_KSNDMC    (SYSTEM=2, UNIT=KSNDMC_ADDON)"
    echo "  TWSRF_SPATIKA   (SYSTEM=2, UNIT=SPATIKA_GEN)"
    echo ""
    echo "Example: ./organize_build.sh TWSRF_SPATIKA"
    exit 1
fi

# Get firmware version from globals.h
FIRMWARE_VERSION=$(grep '#define FIRMWARE_VERSION' globals.h | sed 's/.*"\(.*\)".*/\1/')

echo "Organizing build for: $CONFIG_NAME"
echo "Firmware version: $FIRMWARE_VERSION"
echo ""

# Create output directory
mkdir -p builds/$CONFIG_NAME

# Find and move binary (try different possible names)
BINARY_FOUND=0

if [ -f "AIO9_5.0.ino.esp32.bin" ]; then
    mv AIO9_5.0.ino.esp32.bin builds/$CONFIG_NAME/firmware.bin
    BINARY_FOUND=1
    echo "✓ Found and moved: AIO9_5.0.ino.esp32.bin"
elif [ -f "AIO9_5.0.ino.bin" ]; then
    mv AIO9_5.0.ino.bin builds/$CONFIG_NAME/firmware.bin
    BINARY_FOUND=1
    echo "✓ Found and moved: AIO9_5.0.ino.bin"
else
    echo "✗ Binary not found!"
    echo "  Expected: AIO9_5.0.ino.esp32.bin or AIO9_5.0.ino.bin"
    echo "  Make sure you ran: Sketch → Export Compiled Binary"
    exit 1
fi

# Create fw_version.txt
echo "$FIRMWARE_VERSION" > builds/$CONFIG_NAME/fw_version.txt
echo "✓ Created: fw_version.txt ($FIRMWARE_VERSION)"

# Create build info
cat > builds/$CONFIG_NAME/build_info.txt << EOF
Build Configuration
===================
Configuration: $CONFIG_NAME
Firmware Version: $FIRMWARE_VERSION
Build Date: $(date)
Build Method: Manual (Arduino IDE)
EOF
echo "✓ Created: build_info.txt"

# Show results
echo ""
echo "========================================="
echo "  Build Organized Successfully!"
echo "========================================="
echo ""
echo "Output directory: builds/$CONFIG_NAME/"
echo ""
ls -lh builds/$CONFIG_NAME/
echo ""
echo "Files ready for deployment:"
echo "  - firmware.bin (for FTP or SD card)"
echo "  - fw_version.txt (for SD card)"
echo ""
