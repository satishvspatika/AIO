# Release Notes: v5.36 (Feb 20, 2026)

## 🎯 Overview
Critical stability release focusing on GPRS registration reliability (BSNL fix), watchdog hardening, and sensor fault display accuracy.

---

## ✨ New Features

### 1. **BSNL "Zombie Session" Recovery** 📡
- **Iterative Electronic Reset**: If GPRS registration fails for 10 consecutive attempts, the system now triggers an electronic modem reset (`AT+CFUN=1,1`).
- **Impact**: Dramatically improves reliability for BSNL SIM cards that occasionally get stuck in a "searching" state but refuse to connect without a fresh reset.

### 2. **Enhanced Sensor "NA" Logic** 🔍
- **Noise Suppression**: Implemented a software noise floor (25 raw counts) for Wind Direction and Wind Speed to prevent "ghost" readings (like `1 deg` or `0.02 m/s`) when sensors are disconnected.
- **Snappier Detection**: Reduced the fault confirmation timeout for analog/pulse sensors from 10 seconds to 3 seconds.
- **UI Consistency**: Standardized `NA` display across Temperature, Humidity, and Wind Direction when hardware is missing.

---

## 🔧 Bug Fixes

### 1. **Task Watchdog Hardening (TWDT)** 🛡️
- **Loop Task Registration**: The main system task is now registered with the Task Watchdog.
- **Selective Petting**: Added `esp_task_wdt_reset()` to the main loop and deep-sleep entry to prevent accidental reboots during long graceful shutdown sequences.
- **Noise Cleanup**: Eliminated the `task not found` watchdog errors in the serial logs.

### 2. **Sleep Conflict Resolution** 😴
- **Aggressive Sleep Removal**: Removed the auto-sleep logic from the LCD task that was prematurely shutting down the system while GPRS was still active.
- **Single Source of Truth**: Consolidated all shutdown and sleep logic into `start_deep_sleep()`, preventing race conditions between the UI and the GPRS background task.

---

## 📋 Technical Details

### Modified Files
- `AIO9_5.0.ino` - Watchdog registration and main loop hardening.
- `gprs.ino` - Implementation of electronic reset loop and consolidated shutdown removal.
- `global_functions.ino` - Integrated WDT resets into the deep sleep entry sequence.
- `windDirection.ino` - Noise suppression and faster fault detection.
- `windSpeed.ino` - Software support for `NA` strings on hardware failure.
- `lcdkeypad.ino` - Removal of aggressive power management overrides.

### Versioning
- **Current Version:** v5.36
- **Build ID:** Feb 20 Release

---

## 📦 Upgrade Path

### From v5.35:
- **Direct upgrade** - Flash v5.36 firmware.
- **Key benefit**: Resolves intermittent "searching" hangs and watchdog crashes seen in the field.

---

**v5.36 is the most stable release for BSNL network environments to date.** 🚀
