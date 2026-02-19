# Release Notes: v5.35 (Feb 19, 2026)

## 🎯 Overview
Restoration of production baseline and stability fix for LCD log searching.

---

## ✨ New Features

### 1. **Production Baseline Sync** ⚙️
- Reverted all temporary calibration overrides from vC.4.
- `SYSTEM` and `UNIT` definitions restored to production standards.
- Deep sleep and standard task intervals fully re-enabled for power efficiency.

---

## 🔧 Bug Fixes

### 1. **LCD Keypad Task Crash Fix** 🔧
- **Problem:** The keypad task would terminate (making the UI unresponsive) if a user searched for a log file that did not exist on the SPIFFS.
- **Solution:** Replaced logical `return` statements with skip-and-continue logic.
- **Impact:** System UI remains responsive even if data is missing or files are corrupt.

### 2. **Multi-File Compilation Correction** 🐛
- **Problem:** A missing closing brace in `lcdkeypad.ino` caused legal functions in subsequent files (like `rtcRead.ino`) to be treated as nested definitions, leading to build failure.
- **Solution:** Balanced all braces and validated semantic nesting.
- **Impact:** Clean compilation for all 6 system configurations.

---

## 📋 Technical Details

### Modified Files
- `lcdkeypad.ino` - Structural fix for log search and brace balance.
- `build_all_configs.py` - Standardized `fw_version.txt` format to include system/client prefix.
- `globals.h` - Version increment to 5.35 and baseline verification.

### Code Size Impact
- **Program:** ~1.18 MB
- **RAM:** ~53 KB (Stable)

---

## 📦 Release Contents

This release includes pre-compiled binaries for all 6 system configurations:

### High-Power (SYSTEM=0) - Solar/Battery
1. **KSNDMC_TRG** - Telemetry Rain Gauge
2. **BIHAR_TRG** - Bihar Government Rain Gauge
3. **SPATIKA_TRG** - Generic Spatika Rain Gauge

### Dual-Power (SYSTEM=1) - Weather Station
4. **KSNDMC_TWS** - Telemetry Weather Station (Wind/Temp/Hum/RF)

### ULP Add-On (SYSTEM=2) - Ultra Low Power
5. **KSNDMC_ADDON** - KSNDMC Add-on Configuration
6. **SPATIKA_ADDON** - Spatika Add-on Configuration

### Each Configuration Folder Includes:
- `firmware.bin` - Main application binary
- `fw_version.txt` - Version verification file (Format: `TYPE9-CLIENT-5.35`)
- `bootloader.bin`, `partitions.bin`, `boot_app0.bin` (in `flash_files/`)

---

## 🧪 Testing Recommendations

1. ✅ Verify LCD "Search Logs" with a non-existent date (should show "NO LOG FILE" then return).
2. ✅ Verify `fw_version.txt` content for TWS and TRG configs.

---

## 🔄 Upgrade Path

### From v5.34/vC.4:
- **Direct upgrade** - Flash v5.35 firmware.
- **No hardware changes required.**

---

**v5.35 is production-ready!** 🚀
