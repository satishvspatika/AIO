#!/bin/bash
# High-Speed Batch Flashing & Automated QC Script for Fresh ESP32 Boards (16MB Native Unlocked)
# Configuration: KSNDMC_TRG (SYSTEM 0, Matrix I2C LCD UI - 16MB Full Capacity)

# Set high baud rate for maximum speed (50+ boards batch)
BAUD=921600

# Binaries to flash (16MB Bootloader, 16MB Partition Table & 16MB Matrix UI Firmware)
BOOTLOADER="./flash_files/16mb/bootloader.bin"
PARTITIONS="./flash_files/16mb/partitions.bin"
BOOT_APP0="./flash_files/boot_app0.bin"
VERSION=$(grep '#define FIRMWARE_VERSION' user_config.h 2>/dev/null | sed 's/.*"\(.*\)".*/\1/' | sed 's/^v//')
VERSION=${VERSION:-6.51}
FIRMWARE="/Users/satishkripavasan/Documents/Arduino/ESP32_NEW_DESIGN/RELEASE/AIO9_5/v${VERSION}/KSNDMC_TRG_MAT_16mb/firmware.bin"

# Smart Auto-Detection of Connected USB Port
detect_port() {
    local requested_port=$1
    if [ -n "$requested_port" ]; then
        echo "$requested_port"
        return 0
    fi

    # Scan for common ESP32 USB serial device patterns across macOS & Linux
    local ports=($(ls /dev/cu.usbserial* /dev/cu.usbmodem* /dev/cu.wchusbserial* /dev/cu.SLAB_USBtoUART* /dev/ttyUSB* /dev/ttyACM* 2>/dev/null))

    if [ ${#ports[@]} -eq 0 ]; then
        echo ""
        return 1
    elif [ ${#ports[@]} -eq 1 ]; then
        echo "${ports[0]}"
        return 0
    else
        echo "ℹ️  Multiple USB serial devices detected: ${ports[*]}" >&2
        echo "👉 Automatically selected primary device: ${ports[0]}" >&2
        echo "${ports[0]}"
        return 0
    fi
}

PORT=$(detect_port "$1")

if [ -z "$PORT" ]; then
    echo "❌ Error: No connected ESP32 USB serial device found!"
    echo "   Please plug in your ESP32 board via USB cable and try again."
    echo "   Usage: ./flash_fresh_board.sh [PORT]"
    exit 1
fi

if [ ! -f "$FIRMWARE" ]; then
    echo "❌ Error: Firmware binary not found at $FIRMWARE"
    echo "   Please run build_all_configs.py first."
    exit 1
fi

echo "=================================================="
echo "⚡ FAST BATCH FLASH: KSNDMC_TRG (SYSTEM 0 - 16MB Matrix UI)"
echo "🔌 Auto-Detected Port: $PORT"
echo "=================================================="

# Step 1: Full Chip Erase
echo "🧹 Erasing Chip..."
python3 -m esptool --chip esp32 --port "$PORT" erase_flash
if [ $? -ne 0 ]; then
    echo "❌ Chip Erase Failed!"
    exit 1
fi

# Step 2: High-Speed Factory Write (Native 16MB Unlocked: 2MB App + ~12MB SPIFFS)
echo "⚡ Flashing 16MB Bootloader, 16MB Partition Table & 16MB Firmware..."
python3 -m esptool --chip esp32 --port "$PORT" --baud $BAUD \
    --before default_reset --after hard_reset \
    write_flash --flash_mode dio --flash_freq 80m --flash_size 16MB \
    0x1000  "$BOOTLOADER" \
    0x8000  "$PARTITIONS" \
    0xe000  "$BOOT_APP0" \
    0x10000 "$FIRMWARE"

if [ $? -ne 0 ]; then
    echo "❌ Flashing Failed!"
    exit 1
fi

# Step 3: Automated Post-Flash Hardware QC Verification
echo ""
echo "=================================================="
echo "🔍 RUNNING AUTOMATED POST-FLASH HARDWARE QC CHECK..."
echo "=================================================="

python3 -c "
import serial, time, sys

port = '$PORT'
try:
    s = serial.Serial(port, 115200, timeout=0.2)
    s.dtr = False
    s.rts = True
    time.sleep(0.1)
    s.rts = False
    time.sleep(0.1)

    boot_pass = False
    rtc_pass = False
    spiffs_pass = False
    modem_pass = False
    modem_fail = False

    start = time.time()
    while time.time() - start < 10:
        line = s.readline()
        if line:
            text = line.decode('utf-8', errors='replace').strip()
            if 'System starting' in text or 'BOOT' in text or 'AIO' in text:
                boot_pass = True
            if 'RTC: OK' in text:
                rtc_pass = True
            if 'SPIFFS: OK' in text:
                spiffs_pass = True
            if 'Modem ready' in text or '+CSQ' in text:
                modem_pass = True
            if 'AT Timeout' in text or 'GPRS HARD RESET' in text:
                modem_fail = True

    s.close()

    print('📋 HARDWARE QC TEST RESULTS:')
    print(f'  • ESP32 CPU & Firmware Boot : {\"✅ PASS\" if boot_pass else \"❌ FAIL / NO BOOT\"}')
    print(f'  • SPIFFS Flash Storage      : {\"✅ PASS\" if spiffs_pass else \"❌ FAIL\"}')
    print(f'  • DS3231 Real-Time Clock    : {\"✅ PASS\" if rtc_pass else \"❌ FAIL / NOT CONNECTED\"}')
    if modem_pass:
        print('  • SIMCom 4G Modem UART      : ✅ PASS')
    elif modem_fail:
        print('  • SIMCom 4G Modem UART      : ❌ FAIL (AT Timeout - Check 3.8V Rail / UART Pins)')
    else:
        print('  • SIMCom 4G Modem UART      : ⚠️ WAITING / UNKNOWN')

except Exception as e:
    print('⚠️ Could not run serial QC check:', e)
"

echo ""
echo "=================================================="
echo "🎉 FLASHING COMPLETE & Native 16MB Unlocked!"
echo "➡️  Unplug this board and connect the next one!"
echo "=================================================="
