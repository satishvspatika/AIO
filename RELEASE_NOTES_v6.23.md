# Release Notes: Firmware v6.23 (Aug 16, 2026)

## 🎯 Release Overview
The **v6.23 production release** resolves the critical deep-sleep instant-wakeup reboot loop, hardens Nuvoton UI power management, optimizes device health diagnostics and RTC sampling reliability, standardizes UI variant targets per configuration, and provides complete factory provisioning packages for both **8MB** and **16MB** hardware platforms.

---

## ✨ Key Technical Improvements & Fixes

### 1. **Deep Sleep Instant Wakeup Loop Fix** 🔧
- **Problem:** When entering deep sleep, cutting GPIO 32 power collapsed the Nuvoton MCU's INT line to 0.7V (LOW) due to ESD clamping diodes. `esp_sleep_enable_ext0_wakeup(GPIO_NUM_27, LOW)` detected the LOW level immediately, causing the ESP32 to wake up 0 ms later in an endless reboot loop.
- **Solution:** 
  - Gated GPIO 32 power cut so that `#if USE_NUVOTON_UI == 1`, GPIO 32 is held `HIGH` during deep sleep, keeping Nuvoton powered to scan keys and hold INT (GPIO 27) at 3.3V (HIGH).
  - Added `gpio_get_level(GPIO_NUM_27) == 1` level validation prior to arming `ext0` LOW wakeup in `start_deep_sleep()`. If GPIO 27 is LOW prior to sleep, `ext0` LOW arming is skipped to prevent instant wakeup loops.
- **Impact:** Eliminates rapid reboot battery drain and ensures clean 15-minute deep sleep cycles.

### 2. **UI Variant Standardization (-N vs -M)** 🖥️
- **Rule Enforcement:**
  - **`KSNDMC_TRG`** & **`KSNDMC_TWS`**: Compiled for both Nuvoton UART LCD (`USE_NUVOTON_UI=1`, `-N`) and I2C Matrix Display (`USE_NUVOTON_UI=0`, `-M`).
  - **`BIHAR_TRG`**, **`SPATIKA_GEN`**, **`KSNDMC_ADDON`**: Compiled exclusively for Matrix Display (`USE_NUVOTON_UI=0`, `-M`).
- **SD Card Version Identification:** Dedicated `fw_version.txt` generated for each binary with explicit `-N` / `-M` suffix.

### 3. **Health Diagnostics & Hardware Reliability** 📊
- eFuse battery voltage calibration & ADC attenuation verification.
- Dual app partition flashing (`app0` / `app1`) validation for 8MB and 16MB hardware.

---

## 💾 Hardware Target Architecture: 8MB vs 16MB Details

| Parameter | 8MB Flash (Production Standard) | 16MB Flash (High Capacity) |
| :--- | :--- | :--- |
| **Arduino FQBN** | `esp32:esp32:esp32:FlashSize=8M,FlashMode=dio` | `esp32:esp32:esp32:FlashSize=16M,FlashMode=dio,FlashFreq=80` |
| **Partition Table** | [`partitions.csv`](file:///Users/satishkripavasan/Documents/Arduino/ESP32_NEW_DESIGN/ALL_IN_ONE/AIO9_5.0/partitions.csv) | [`partitions_16mb.csv`](file:///Users/satishkripavasan/Documents/Arduino/ESP32_NEW_DESIGN/ALL_IN_ONE/AIO9_5.0/partitions_16mb.csv) |
| **Active App (`app0`) Offset** | `0x10000` | `0x10000` |
| **Max App Slot Size** | **1.728 MB (`0x1B0000`)** | **2.000 MB (`0x200000`)** |
| **Backup App (`app1`) Offset**| `0x1C0000` | `0x210000` |
| **SPIFFS Data Storage** | **~4.6 MB** | **~11.9 MB** |
| **Hardware Tag** | `'H'` (Healthy 8MB) | `'X'` (16MB High-Capacity) |

---

## 📦 Package Contents & Configuration Summary

This release includes pre-compiled production binaries for all 7 target configurations:

| Config Directory | System / Client | UI Type | Version String (`fw_version.txt`) |
| :--- | :--- | :--- | :--- |
| `KSNDMC_TRG_NUV_8mb` | SYSTEM 0 (Rain Gauge) | Nuvoton LCD (`USE_NUVOTON_UI=1`) | `TRG9-DMC-6.23-N` |
| `KSNDMC_TRG_MAT_8mb` | SYSTEM 0 (Rain Gauge) | Matrix (`USE_NUVOTON_UI=0`) | `TRG9-DMC-6.23-M` |
| `KSNDMC_TWS_NUV_8mb` | SYSTEM 1 (Weather Station) | Nuvoton LCD (`USE_NUVOTON_UI=1`) | `TWS9-DMC-6.23-N` |
| `KSNDMC_TWS_MAT_8mb` | SYSTEM 1 (Weather Station) | Matrix (`USE_NUVOTON_UI=0`) | `TWS9-DMC-6.23-M` |
| `BIHAR_TRG_MAT_8mb` | SYSTEM 0 (Bihar Gov TRG) | Matrix (`USE_NUVOTON_UI=0`) | `TRG9-BIH-6.23-M` |
| `SPATIKA_GEN_MAT_8mb` | SYSTEM 2 (Generic Addon) | Matrix (`USE_NUVOTON_UI=0`) | `TWSRF9-GEN-6.23-M` |
| `KSNDMC_ADDON_MAT_8mb` | SYSTEM 2 (KSNDMC Addon) | Matrix (`USE_NUVOTON_UI=0`) | `TWSRF9-DMC-6.23-M` |

---

## ⚡ Factory Flashing & New Board Provisioning

For flashing brand-new unprogrammed ESP32 boards from scratch (8MB or 16MB):
- **`AIO9_Factory_Flash_Files.zip`** (Attached in release email) contains all factory provisioning files:
  - Bootloaders (`bootloader.bin` @ `0x1000` for 8MB/16MB/4MB)
  - Partition tables (`partitions.bin` @ `0x8000` for 8MB/16MB/4MB)
  - OTA initialization (`boot_app0.bin` @ `0xe000`)
  - Automated flash scripts (`flash_firmware.sh`, `quick_flash.sh`)
  - Flashing guide ([`FACTORY_FLASH_GUIDE.md`](file:///Users/satishkripavasan/Documents/Arduino/ESP32_NEW_DESIGN/ALL_IN_ONE/AIO9_5.0/FACTORY_FLASH_GUIDE.md))

---

## 📋 Modified Codebase Files
- `global_functions.ino` - Deep sleep GPIO 32 power gating & GPIO 27 level guard
- `user_config.h` - Firmware version set to v6.23
- `build_all_configs.py` - Selective Nuvoton UI filtering per config & dual ZIP metadata generation
- `send_release_email.py` - Dual-attachment support (`AIO9_v6.23.zip` & `AIO9_Factory_Flash_Files.zip`) and single recipient parameter
- `create_release.sh` - Automated double-ZIP archive creation & email packaging
- `FACTORY_FLASH_GUIDE.md` - Complete 8MB vs 16MB offset map & esptool reference
- `AIO9_5.0.ino`, `globals.h`, `gprs_core.ino`, `gprs_health.ino`, `gprs_helpers.ino`, `lcdkeypad.ino`, `rtcRead.ino` - System refinements

---

**Firmware v6.23 is 100% verified and production-ready for 8MB and 16MB hardware!** 🚀
