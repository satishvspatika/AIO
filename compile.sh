#!/bin/bash

# AIO ESP32 Compilation Script
# Usage: ./compile.sh

echo "--- Starting Compilation ---"

# We use the custom partition scheme defined in your partitions.csv
arduino-cli compile --fqbn esp32:esp32:esp32:FlashSize=8M,PartitionScheme=custom \
    --build-property build.partitions=custom \
    --build-property build.custom_partitions=partitions.csv \
    --export-binaries .

if [ $? -eq 0 ]; then
    echo "--- Compilation Successful ---"
    echo "You can now run ./quick_flash.sh to upload."
else
    echo "--- Compilation FAILED ---"
    exit 1
fi
