# Release Notes - v6.49

**Release Date:** September 21, 2026  
**Firmware Version:** `6.49`  
**Target Hardware:** ESP32-WROOM-32 / ESP32-S3 (8MB Flash)

---

## 🎯 Summary of Changes in v6.49

### 1. 📟 Nuvoton UI SD Firmware Update Collision Fix (`USE_NUVOTON_UI 1`)
- **Root Cause Identified:** During boot with an SD card inserted, `lcdkeypadTask` is created early in `setup()` to display the initial splash screen. `sd_ota_lcd_active` was declared as `static` in `AIO9_5.0.ino`, making it invisible to `lcdkeypad.ino`. As a result, `lcdkeypadTask` on Core 0 remained unaware of SD firmware update activity and continuously blasted `INITIALIZING... PLEASE WAIT...` over UART (`Serial1` at 9600 baud) every ~100ms. This collided directly with the `SD FW UPDATING..` progress messages sent by `initialize_hw()`, causing Nuvoton LCD displays to freeze or get stuck on `INITIALIZING...`.
- **Global Variable Scope Fix:** Removed the `static` modifier from `volatile bool sd_ota_lcd_active` in `AIO9_5.0.ino` to link cleanly with `extern volatile bool sd_ota_lcd_active` in `globals.h`.
- **Task Yield & Display Pause:** Added explicit yield blocks in `lcdkeypad.ino` (`draw_current_page()` and `lcdkeypad()` main loop). When `sd_ota_lcd_active` is `true`, `lcdkeypadTask` halts all UART transmissions and yields CPU execution, giving the SD firmware updater complete control over `Serial1`.

### 2. 💾 SD Card Re-update Protection & Multi-Unit Deployment
- **Automatic Renaming:** Upon successful completion of SD firmware flashing (or when version strings match), `firmware.bin` on the SD card is automatically renamed to `firmware.bin.installed`.
- **SPIFFS MD5 & Version Preservation:** Preserved `/sd_fw_md5.txt` and `/sd_fw_ver.txt` across SPIFFS cross-flash wipes.
- **Field Reusability:** If an SD card is left inserted in the unit's slot after an update or during routine field operations, subsequent reboots fast-skip flashing and will not trigger infinite update loops.

---

## 📦 Released Build Binaries

| Configuration | UI Type | Flash Size | Binary Filename |
| :--- | :--- | :--- | :--- |
| **KSNDMC_TRG_NUV_8mb** | Nuvoton UART LCD | 8MB | `TRG9-DMC-6.49-N.bin` |
| **KSNDMC_TRG_MAT_8mb** | Matrix I2C LCD | 8MB | `TRG9-DMC-6.49-M.bin` |

---
