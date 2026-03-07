# Release Notes: v5.48 (March 07, 2026)

## 🎯 Overview
Release v5.48: Health Report Timing Optimization and Robust Deep Sleep Handlers. This version delivers extensive algorithmic optimizations focused on data-link reliability, strict power budgeting, and scheduled health transmission.

---

## ✨ New Features & Enhancements

### 1. **Optimized Single-Window Health Reports** 📝
- Health diagnostic payloads are now strictly locked to an `11:00 AM` window. 
- Prevents multiple energy-draining transmissions. Retries only happen at 11:15, 11:30, or 11:45 if the primary 11:00 attempt fails.

### 2. **Smart FTP Backlog Batching** 📝
- Backlog transfers are securely batched (threshold configured) to prevent firing multiple high-power connections for single isolated offline records.

### 3. **Cellular Power Profiling (Micro-optimizations)** 📝
- Added intelligent PDP cache state checks. The device will skip tearing down the base context (`CIPSHUT`) if the network is already warm, significantly lowering time-on-air and current draw.

---

## 🔧 Bug Fixes

### 1. **Transient HTTP Drop Recovery (706/713/714)** 🔧
- **Problem:** Tower instability could cause sudden session drops mid-post.
- **Solution:** Firmware bypasses deep sleep on a failed HTTP action and gracefully re-initializes the HTTP stack for a rapid follow-up attempt without dropping the core PDP layer.
- **Impact:** Data recovery approaches 100% even on fringe towers.

### 2. **Deep Sleep Network Termination** 🔧
- **Problem:** Zombie contexts draining slight current.
- **Solution:** `sync_mode=4` cleanly deactivates physical VCC power (GPIO 26 -> LOW).
- **Impact:** Eliminates parasitic current before entering deep sleep.

---

## 📋 Technical Details

### Modified Files 
- `gprs.ino` - Heavily reworked HTTP logic, Health scheduler, and FTP handlers.
- `AIO9_5.0.ino` & `scheduler.ino` - Data alignment and RTC checks.

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

## 🔄 Upgrade Path

### From v5.47:
- **Direct upgrade** - Flash v5.48 firmware via OTA or Serial.
- **No hardware or NVS wipe required.**

---

**v5.48 is production-ready!** 🚀
