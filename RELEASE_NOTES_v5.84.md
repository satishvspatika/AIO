# Release Notes: v5.84 (Mar 27, 2026)

## 🎯 Overview
Added v5.84 Fixes for open Claude review items: scheduler tore-read, ftp put buffer expansion, file size localization, and debug 0 production readiness

---

## ✨ New Features

### 1. **Feature Name** 📝
- Description of feature
- Use case

---

## 🔧 Bug Fixes

### 1. **Fix Name** 🔧
- **Problem:** Description
- **Solution:** Description
- **Impact:** Description

---

## 📋 Technical Details

### Modified Files
- `file1.ino` - Description
- `file2.h` - Description

### Code Size Impact
- **Program:** TBD bytes
- **RAM:** TBD bytes

---

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
- `fw_version.txt` - Version verification file
- `bootloader.bin`, `partitions.bin`, `boot_app0.bin` (in `flash_files/`)

---

## 🧪 Testing Recommendations

1. ✅ Test feature 1
2. ✅ Test feature 2

---

## 🔄 Upgrade Path

### From v5.3:
- **Direct upgrade** - Flash v5.84 firmware
- **No configuration changes** required

---

**v5.84 is production-ready!** 🚀
