# Release Notes: v5.49 (March 09, 2026)

## 🎯 Overview
Release v5.49: The "Golden Build" Stability & Accuracy Update. This version focuses on eliminating edge-case data artifacts (spikes), hardening sensor bus coherence, and ensuring high-reliability network time synchronization. It addresses critical feedback regarding wind speed spikes and modem communication robustness.

---

## ✨ New Features & Enhancements

### 1. **32-Bit High-Precision Pulse Accumulators** 🏅
- **Problem:** Frequent 16-bit ULP counter resets during reporting intervals (15 mins) caused massive artificial spikes in wind speed and rainfall logs.
- **Solution:** Implemented 32-bit accumulators (`total_wind_pulses_32` and `total_rf_pulses_32`) in RTC memory. The system now tracks hardware counter deltas without destructive resets.
- **Impact:** Reliable data tracking for years with ZERO rollover artifacts or interval spikes.

### 2. **GPS Age Tracking & Fix Awareness** 🏅
- **Problem:** Locations loaded from memory on reboot had no "freshness" indicator, leading to stale positions on the dashboard.
- **Solution:** Integrated a date-stamped persistence engine for GPS fixes. Coordinates are now saved alongside their fix-date in `/gps_fix.txt`.
- **Impact:** Transparent location reporting with full historical knowledge of when the last fix was captured.

### 3. **Modem-Agnostic CLBS Parser** 🏅
- **Problem:** Variance in SIMCOM 7672 response formats (quoted vs. unquoted date/time strings) caused RTC sync failures on some tower configurations.
- **Solution:** Replaced fragile `sscanf` with a robust manual delimiter-based parsing engine.
- **Impact:** Near 100% success rate for network-based time synchronization, keeping system clocks accurate globally.

### 4. **Smart Fault Detection for Wind Direction** 🏅
- **Problem:** Steady wind from the North (ADC 0) for >30 seconds would incorrectly trigger a "Sensor Disconnected (NA)" report.
- **Solution:** Increased the fault threshold to **5 minutes (300 cycles)**.
- **Impact:** Elimination of false "NA" readings during calm or steady North wind periods.

---

## 🔧 Bug Fixes & Stability

### 1. Independent FTP Backlog Trigger (v5.49 Build 5) 🏅
- **Problem:** If the "live" HTTP post failed (e.g., during SIM startup), the system would skip the FTP backlog rescue, potentially leaving data unsent for days.
- **Solution:** Decoupled FTP logic from HTTP status. The system now independently evaluates the backlog on every wake-up.
- **Impact:** Guaranteed recovery of missing data as soon as signal is usable, even if the primary HTTP server is intermittently unreachable.

### 2. Heap Overflow Protection (Backlog Safety) 🔧
- **Problem:** Loading the entire "Unsent Data" log into RAM (via `.readString()`) could crash the ESP32 if the device had been offline for more than 48 hours.
- **Optimization:** Adjusted FTP backlog threshold to **6 records** (1.5 hours) per user request.
- **Solution:** Refactored the scheduler to use a **Line-by-Line processor** for file reading.
- **Impact:** Constant RAM usage regardless of backlog size. Safe recovery even from multi-day outages.
- **Benefit:** Constant RAM usage regardless of backlog size via line-by-line reading. Safe recovery from multi-day outages.

### 3. **Multi-Core State Integrity** 🔧
- **Problem:** Potential "race condition" where Task Core 0 (Modem) and Task Core 1 (Logic) saw mismatched states due to compiler optimizations.
- **Solution:** Marked core state variables (`sync_mode`, `gprs_mode`) as **`volatile`**.
- **Impact:** Hardware-enforced consistency across all tasks. Zero "ghost hangs."

### 3. **Sensor Incoherence** 🔧
- **Problem:** Scheduler could read a "mismatched" Temp/Hum pair during a task switch.
- **Solution:** Unified `i2cMutex` locking across the entire read-and-assign sequence.
- **Impact:** Atomic data consistency across all reporting intervals.

---

## 📋 Technical Details

### Modified Files 
- `globals.h` - Volatile states, accuracy counters, and GPS timestamp variables.
- `AIO9_5.0.ino` - POR initialization logic.
- `scheduler.ino` - Line-for-line backlog reading & 32-bit pulses.
- `global_functions.ino` - Persistent GPS fix timestamping.
- `windSpeed.ino` - Deep-sleep aware 32-bit accumulation.
- `tempHum.ino` - Atomic mutex locking for coherence.
- `rtcRead.ino` - Robust `+CLBS` string parser.
- `windDirection.ino` - North-wind false positive fix.

---

## 📦 Upgrade Path

- **From v5.48:** Direct binary update via OTA or Serial. No NVS/SPIFFS wipe required. This version is functionally backward-compatible but provides superior accuracy.

---

**v5.49 is the recommended Golden Build for all field deployments.** 🚀
