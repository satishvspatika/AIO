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

# Find and move binary (check root and build folder)
BINARY_FOUND=0

# Possible binary locations
POSSIBLE_BINS=(
    "AIO9_5.0.ino.esp32.bin"
    "AIO9_5.0.ino.bin"
    "build/esp32.esp32.esp32/AIO9_5.0.ino.bin"
)

for bin_path in "${POSSIBLE_BINS[@]}"; do
    if [ -f "$bin_path" ]; then
        cp "$bin_path" builds/$CONFIG_NAME/firmware.bin
        BINARY_FOUND=1
        echo "✓ Found and copied: $bin_path"
        break
    fi
done

if [ $BINARY_FOUND -eq 0 ]; then
    echo "❌ Binary not found!"
    echo "  Tried: ${POSSIBLE_BINS[*]}"
    echo "  Make sure you ran: './compile.sh' or IDE 'Export Compiled Binary'"
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
