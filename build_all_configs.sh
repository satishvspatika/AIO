#!/bin/bash
# Automated Build Script for AIO9_5.0
# Builds all SYSTEM/UNIT configurations automatically
# Usage: ./build_all_configs.sh

# Configuration
ARDUINO_CLI="/Applications/Arduino.app/Contents/MacOS/Arduino"
SKETCH_PATH="/Users/satishkripavasan/Documents/Arduino/ESP32_NEW_DESIGN/ALL_IN_ONE/AIO9_5.0/AIO9_5.0.ino"
OUTPUT_BASE="/Users/satishkripavasan/Documents/Arduino/ESP32_NEW_DESIGN/ALL_IN_ONE/AIO9_5.0/builds"
GLOBALS_H="/Users/satishkripavasan/Documents/Arduino/ESP32_NEW_DESIGN/ALL_IN_ONE/AIO9_5.0/globals.h"
BACKUP_GLOBALS="/tmp/globals.h.backup"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "========================================="
echo "  AIO9_5.0 Multi-Configuration Builder"
echo "========================================="
echo ""

# Backup original globals.h
echo "Backing up globals.h..."
cp "$GLOBALS_H" "$BACKUP_GLOBALS"

# Create output directory
mkdir -p "$OUTPUT_BASE"

# Build counter
SUCCESS_COUNT=0
FAIL_COUNT=0

# Function to update globals.h
update_globals() {
    local system=$1
    local unit=$2
    
    echo -e "${YELLOW}Configuring: SYSTEM=$system, UNIT=$unit${NC}"
    
    # Restore backup first
    cp "$BACKUP_GLOBALS" "$GLOBALS_H"
    
    # Update SYSTEM
    sed -i '' "s/#define SYSTEM [0-9]/#define SYSTEM $system/" "$GLOBALS_H"
    
    # Update UNIT
    sed -i '' "s/char UNIT\[15\] = \"[^\"]*\"/char UNIT[15] = \"$unit\"/" "$GLOBALS_H"
    
    echo "  SYSTEM set to: $system"
    echo "  UNIT set to: $unit"
}

# Function to build and export
build_config() {
    local system=$1
    local unit=$2
    local output_name=$3
    
    echo ""
    echo "========================================="
    echo -e "${GREEN}Building: $output_name${NC}"
    echo "========================================="
    
    # Update configuration
    update_globals $system "$unit"
    
    # Create output directory
    local output_dir="$OUTPUT_BASE/$output_name"
    mkdir -p "$output_dir"
    
    # Build using Arduino IDE (headless)
    echo "Compiling..."
    "$ARDUINO_CLI" --verify --board esp32:esp32:esp32 --pref build.path="$output_dir/build" "$SKETCH_PATH" > "$output_dir/build.log" 2>&1
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ Compilation successful${NC}"
        
        # Find and copy the binary
        BINARY=$(find "$output_dir/build" -name "*.bin" -type f | grep -v "bootloader" | grep -v "partitions" | head -1)
        
        if [ -n "$BINARY" ]; then
            cp "$BINARY" "$output_dir/firmware.bin"
            SIZE=$(ls -lh "$output_dir/firmware.bin" | awk '{print $5}')
            echo -e "${GREEN}✓ Binary exported: firmware.bin ($SIZE)${NC}"
            
            # Get firmware version from globals.h
            FIRMWARE_VERSION=$(grep '#define FIRMWARE_VERSION' "$GLOBALS_H" | sed 's/.*"\(.*\)".*/\1/')
            
            # Create fw_version.txt for legacy SD card updates
            echo "$FIRMWARE_VERSION" > "$output_dir/fw_version.txt"
            echo -e "${GREEN}✓ Version file created: fw_version.txt ($FIRMWARE_VERSION)${NC}"
            
            SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
            
            # Create info file
            cat > "$output_dir/build_info.txt" << EOF
Build Configuration
===================
SYSTEM: $system
UNIT: $unit
Output: $output_name
Firmware Version: $FIRMWARE_VERSION
Build Date: $(date)
Binary Size: $SIZE
Binary Path: $output_dir/firmware.bin
Version File: $output_dir/fw_version.txt
EOF
        else
            echo -e "${RED}✗ Binary not found${NC}"
            FAIL_COUNT=$((FAIL_COUNT + 1))
        fi
    else
        echo -e "${RED}✗ Compilation failed${NC}"
        echo "See log: $output_dir/build.log"
        FAIL_COUNT=$((FAIL_COUNT + 1))
    fi
}

# Build all configurations
echo ""
echo "Starting builds for all configurations..."
echo ""

# SYSTEM 0 configurations
build_config 0 "KSNDMC_TRG" "TRG_KSNDMC"
build_config 0 "BIHAR_TRG" "TRG_BIHAR"
build_config 0 "SPATIKA_GEN" "TRG_SPATIKA"

# SYSTEM 1 configuration
build_config 1 "KSNDMC_TWS" "TWS_KSNDMC"

# SYSTEM 2 configurations
build_config 2 "KSNDMC_ADDON" "TWSRF_KSNDMC"
build_config 2 "SPATIKA_GEN" "TWSRF_SPATIKA"

# Restore original globals.h
echo ""
echo "Restoring original globals.h..."
cp "$BACKUP_GLOBALS" "$GLOBALS_H"

# Summary
echo ""
echo "========================================="
echo "           Build Summary"
echo "========================================="
echo -e "${GREEN}Successful: $SUCCESS_COUNT${NC}"
echo -e "${RED}Failed: $FAIL_COUNT${NC}"
echo ""
echo "Output directory: $OUTPUT_BASE"
echo ""

# List all builds
echo "Built configurations:"
ls -lh "$OUTPUT_BASE"/*/firmware.bin 2>/dev/null | awk '{print "  " $9 " (" $5 ")"}'

echo ""
echo "Done!"
