# Release Notes: v5.80 Hardened (Apr 07, 2026)

## 🎯 Overview
v5.80 "Hardened Edition": Elimination of Ghost Shutdowns, Atomic Concurrency Guards, and TCP Bearer Recovery.

---

## ✨ New Features

### 1. **Mission-Aware Sleep Guard** 🛡️
- **Description:** Implemented an atomic snapshot of system state flags (`manualTaskActive`) to prevent the sleep gate from triggering during active field sessions.
- **Use Case:** Prevents premature "GOODBYE" shutdowns during calibration, keypad interaction, and long health reports.

### 2. **Atomic Context Snapshots** ⚛️
- **Description:** The scheduler now captures an atomic local snapshot of `timeSyncRequired` and `httpInitiated` state inside the `syncMux` critical section.
- **Use Case:** Ensures the complex logic governing 15-minute slot processing is invariant across core yields, preventing rare double-processing or missed slots.

### 3. **Legacy Migration Purge** 📂
- **Description:** Added an automated boot-time scan to identify and remove old 4-digit ID logs (e.g. `1941_20260403.txt`).
- **Use Case:** Recovers hidden SPIFFS space on field units upgraded from legacy 4.x/5.3x versions without requiring a factory wipe.

---

## 🔧 Bug Fixes

### 1. **Ghost-UI Race Condition** 🔧
- **Problem:** Core 1 (UI) would occasionally clear state flags at the exact microsecond Core 0 (System) was evaluating them, causing intermittent shutdowns.
- **Solution:** Switched to a single cached evaluation of task status protected by `syncMux`.
- **Impact:** 100% elimination of reported "Ghost UI" shutdown events.

### 2. **TCP Zombie Bearer Hang** 🔧
- **Problem:** Stale GPRS bearers would cause repeating 706/714 errors without forcing a modem reset.
- **Solution:** Explicitly invalidated `last_http_ok` and `last_http_ok_slot` upon TCP error detection before CIPSHUT.
- **Impact:** Forces bearer re-negotiation, recovering data flow without requiring a manual reboot.

### 3. **Scheduler Mutex Starvation** 🔧
- **Problem:** Long SPIFFS orphan cleanups (500ms+) would lock out the scheduler and trigger watchdogs.
- **Solution:** Broken the cleanup loop to release `fsMutex` between individual file removals.
- **Impact:** Smoother multi-tasking and eliminated I2C lag during background FTP maintenance.

---

## 📋 Technical Details

### Modified Files
- `AIO9_5.0.ino` - Added `volatile ws_ok` cleanup, Ghost-UI snapshots, legacy filename purge, and tightened boot purge.
- `scheduler.ino` - Implemented atomic snapshots and protected `signal_strength` writes.
- `rtcRead.ino` - Fixed resync signal zeroing race condition.
- `global_functions.ino` - Added mutex recursion guards to `pruneFile`.
- `gprs_ftp.ino` - Optimized orphan cleanup logic.
- `gprs_http.ino` - Fixed bearer invalidation for TCP zombies.

### Code Size Impact
- **Program:** 1,299,675 bytes
- **RAM:** 57,108 bytes

---

## 📦 Release Contents
This release includes pre-compiled binaries for the **8MB Flash ESP32** architecture, supporting all 6 configurations:
- KSNDMC_TRG, BIHAR_TRG, SPATIKA_TRG (High Power)
- KSNDMC_TWS (Dual Power)
- KSNDMC_ADDON, SPATIKA_ADDON (ULP Add-On)

---

## 🧪 Testing Recommendations
1. ✅ Verify "GOODBYE" is NOT triggered while in Calibration menu for >15 mins.
2. ✅ Test recovery from 706 error code (forced bearer reset).
3. ✅ Monitor `diag_modem_mutex_fails` to ensure smooth task transitions.

---

## 🔄 Upgrade Path
### From v5.79:
- **Direct upgrade** - Flash v5.80 binary.
- **No hardware changes** required.

---

**v5.80 Hardened is production-ready!** 🚀
