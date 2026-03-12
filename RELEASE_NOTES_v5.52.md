# Release Notes: v5.52 (March 12, 2026)

## 🎯 Overview
Release v5.52: The "Stability & Precision" Update. This version introduces the **Absolute Silence Protocol** for OTA safety, fixes critical resource leaks, and implements high-precision leap year logic to ensure lifetime clock accuracy for field-deployed units.

---

## ✨ New Features & Enhancements

### 1. **Absolute Silence Protocol (OTA Safety)** 🏅
- **Problem:** Tasks like sensor reading or scheduling could attempt to write to Flash/SPIFFS while an OTA update was writing the new binary, causing file system corruption or system crashes.
- **Solution:** Implemented a total suspension of all non-essential tasks (Scheduler, Temperature/Humidity, Wind Speed/Direction) the moment an OTA update is detected.
- **Impact:** Guarantees 100% stable OTA updates even in electrically noisy environments.

### 2. **High-Precision Leap Year Logic** 🏅
- **Enhancement:** Implemented the full Gregorian leap year rule: `(Year % 4 == 0 && Year % 100 != 0) || (Year % 400 == 0)`.
- **Impact:** Ensures the internal scheduler and date-increment logic remain perfectly accurate for the next several centuries.

### 3. **Standardized FTP Backlog (ENH-2)** 🏅
- **Change:** Fixed the `FTP_CHUNK_SIZE` to 15 records per upload.
- **Impact:** Optimizes transmission reliability over slow 2G/BSNL networks by keeping package sizes small and manageable.

---

## 🔧 Bug Fixes & Stability

### 1. **Root Handle Memory Leak (BUG-5)** 🔧
- **Fix:** Added explicit `root.close()` calls after performing directory checks in the scheduler.
- **Impact:** Prevents "Out of File Descriptors" errors and system hangs after long periods of uptime.

### 2. **Data Sanity Caps** 🔧
- **RF Cap:** Rainfall is now capped at 50mm per 15-minute slot to filter out instrumentation spikes.
- **Backlog Cap:** Backlog counters are capped to prevent overflow errors in environments with massive historical data.

### 3. **ULP Frequency Refinement** 🔧
- **Fix:** Improved handling of high-frequency pulses from wind/rain sensors to prevent delta-calculation overflows (>65,000 pulses).

### 4. **Wind Direction Log Silence** 🔧
- **Optimization:** Silenced repetitive "Disconnected" serial messages for wind sensors. Logs now only trigger when the connection state actually changes.

### 5. **Reliable GPS Recovery (v5.51 Inheritance)** 🔧
- **Enhancement:** Ensures GPS coordinates are proactively reloaded from SPIFFS on hard power-on (POR), providing immediate location context even before the first satellite fix.

---

## 📋 Technical Details

### Modified Files 
- `globals.h` - Defined FTP chunk sizes and signal strength mapping.
- `AIO9_5.0.ino` - Implemented OTA silence hooks and health-persistence resets.
- `scheduler.ino` - Fixed BUG-5 root leak, implemented Leap Year rule and Sanity caps.
- `tempHum.ino` / `windSpeed.ino` - Integrated with OTA Silence Protocol.
- `windDirection.ino` - Refined serial logging logic.

---

## 📦 Upgrade Path

- **From v5.51/v5.50:** Direct binary update via OTA. No configuration wipe required.
- **From older versions:** Direct update supported; system will automatically migrate `station.doc` to the new NVS-based persistence layer.

---

**v5.52: Decoupling Task Interference for Uninterrupted Field Reliability.** 🚀
