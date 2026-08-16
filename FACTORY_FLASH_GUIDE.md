# AIO9 ESP32 Factory Flashing & New Board Provisioning Guide

This guide provides complete instructions for initial factory flashing and provisioning of brand-new, unprogrammed ESP32 hardware boards for both **8MB (Production Standard)** and **16MB (High Capacity)** flash hardware variants.

---

## 📊 8MB vs 16MB Hardware Comparison & Details

| Parameter | 8MB Flash (Current Production) | 16MB Flash (High Capacity) |
| :--- | :--- | :--- |
| **Arduino CLI FQBN** | `esp32:esp32:esp32:FlashSize=8M,FlashMode=dio` | `esp32:esp32:esp32:FlashSize=16M,FlashMode=dio,FlashFreq=80` |
| **Partition Table Source** | [`partitions.csv`](file:///Users/satishkripavasan/Documents/Arduino/ESP32_NEW_DESIGN/ALL_IN_ONE/AIO9_5.0/partitions.csv) | [`partitions_16mb.csv`](file:///Users/satishkripavasan/Documents/Arduino/ESP32_NEW_DESIGN/ALL_IN_ONE/AIO9_5.0/partitions_16mb.csv) |
| **Bootloader Path** | `flash_files/8mb/bootloader.bin` | `flash_files/16mb/bootloader.bin` |
| **Partition Bin Path** | `flash_files/8mb/partitions.bin` | `flash_files/16mb/partitions.bin` |
| **Active App (`app0`) Offset** | `0x10000` | `0x10000` |
| **App Slot Max Size** | **1,769,472 bytes (~1.728 MB)** | **2,097,152 bytes (2.00 MB)** |
| **OTA Staging (`app1`) Offset** | `0x1C0000` | `0x210000` |
| **SPIFFS Offset & Size** | Offset `0x370000`, Size **~4.6 MB** | Offset `0x410000`, Size **~11.9 MB** |
| **Runtime Hardware Tag** | `'H'` (Healthy 8MB Production) | `'X'` (16MB High Capacity) |

---

## 📐 Memory Map & Offset Table

When flashing a new, blank ESP32 board from scratch via USB/Serial, all 4 binaries must be written to their exact memory offsets:

| Offset | Binary Name | Description | Source Path |
| :--- | :--- | :--- | :--- |
| **`0x1000`** | `bootloader.bin` | ESP-IDF 2nd-stage bootloader | `flash_files/<size>/bootloader.bin` |
| **`0x8000`** | `partitions.bin` | Custom partition table mapping | `flash_files/<size>/partitions.bin` |
| **`0xe000`** | `boot_app0.bin` | OTA data partition default state | `flash_files/boot_app0.bin` |
| **`0x10000`** | `firmware.bin` | AIO9 Application firmware binary | `<CONFIG_FOLDER>/firmware.bin` |

---

## ⚡ 1. Flashing via Shell Script (Automated)

### 8MB Full Factory Provisioning (Erase + Complete Flash)
```bash
./flash_firmware.sh /dev/cu.usbserial-110 8mb
```

### 16MB Full Factory Provisioning (Erase + Complete Flash)
```bash
./flash_firmware.sh /dev/cu.usbserial-110 16mb
```

---

## 🔧 2. Manual Flashing via `esptool.py`

### Step 1: Full Chip Erase
```bash
esptool.py --chip esp32 --port <PORT> erase_flash
```

### Step 2: 8MB Hardware Initial Flash (Offset 0x1000, 0x8000, 0xe000, 0x10000)
```bash
esptool.py --chip esp32 --port <PORT> --baud 460800 \
  --before default_reset --after hard_reset \
  write_flash \
  --flash_mode dio \
  --flash_freq 80m \
  --flash_size 8MB \
  0x1000  flash_files/8mb/bootloader.bin \
  0x8000  flash_files/8mb/partitions.bin \
  0xe000  flash_files/boot_app0.bin \
  0x10000 <CONFIG_NAME>/firmware.bin
```

### Step 3: 16MB Hardware Initial Flash (Offset 0x1000, 0x8000, 0xe000, 0x10000)
```bash
esptool.py --chip esp32 --port <PORT> --baud 460800 \
  --before default_reset --after hard_reset \
  write_flash \
  --flash_mode dio \
  --flash_freq 80m \
  --flash_size 16MB \
  0x1000  flash_files/16mb/bootloader.bin \
  0x8000  flash_files/16mb/partitions.bin \
  0xe000  flash_files/boot_app0.bin \
  0x10000 <CONFIG_NAME>/firmware.bin
```

---

## 💾 3. SD Card Field Updating (Existing Board Upgrade)

For field units that already have bootloader and partition tables flashed:
1. Copy `firmware.bin` and `fw_version.txt` from the specific configuration folder (e.g. `KSNDMC_TRG_NUV_8mb`) to the root of a FAT32 MicroSD card.
2. Insert MicroSD card into the unit and reboot.
3. Firmware will auto-detect version mismatch, perform internal OTA update to `app0`, save the version marker to SPIFFS, and reboot into the new firmware.
