# Release Notes: AIO9 Firmware v5.78 (Hardened)
**Date:** 2026-04-06
**Version:** v5.78 "Golden-Hardened"
**Target Hardware:** ESP32-WROOM-32 (4MB/8MB)

## Executive Summary
Version 5.78 is a critical hardening release focused on field stability, data sanctity, and connectivity resilience. It addresses edge-case race conditions and connectivity bottlenecks identified through a comprehensive system audit.

## 🔴 Critical Path Fixes
### 1. Smart APN Discovery (Airtel IoT/M2M)
*   **Issue:** Some 13-digit M2M SIMs were incorrectly identified as standard commercial Airtel SIMs, leading to PDP context failures on `airtelgprs.com`.
*   **Fix:** Implemented prefix-based ICCID analysis (`899116`, `899110`) in both the **Discovery** and **Smart-Cache** paths.
*   **Result:** All Airtel SIM types are now correctly routed to either `airteliot.com` or `airtelgprs.com` from the first slot post-upgrade.

### 2. SD Card Forward-Compatibility
*   **Issue:** The new identity-based update logic was too strict, failing against legacy `fw_version.txt` formats (simple "5.78" vs. "TWSRF9-DMC-5.78").
*   **Fix:** Updated the version comparison to accept both formats while maintaining MD5 integrity checks.
*   **Result:** Prevents infinite re-flash loops on units with legacy SD cards.

### 3. Data Wipe Atomicity (Race Condition C-01)
*   **Issue:** Changing a Station ID involved two separate filesystem transactions, allowing a race window where the scheduler could write records with stale metadata.
*   **Fix:** Merged the "Factory Reset Wipe" and "Identity Save" into a single, atomic `fsMutex` transaction.
*   **Result:** 100% data sanctity during station maintenance.

## 🟡 Reliability & Resilience Improvements
### 4. Ghost-UI Noise Filtering
*   **Issue:** Electrical noise from the modem on GPIO 27 (keypad) could trigger false LCD wakeups during scheduled reporting, holding the device awake for 3 minutes.
*   **Fix:** Implemented a universal `uiIsGhosting` exception in the deep-sleep gate.
*   **Result:** False wakeups now time out and return to sleep after 60s, preserving battery in noisy RF environments.

### 5. Watchdog-Safe Bearer Recovery
*   **Issue:** The 10-second bearer recovery stall could trigger a Task WDT timeout during heavy CPU load.
*   **Fix:** Replaced the static delay with a watchdog-safe loop that pets the WDT every 1,000ms.
*   **Result:** Eliminates accidental reboots during difficult modem initialization windows.

### 6. RTC Accumulator Resilience
*   **Issue:** 32-bit rainfall/wind anchors were not reset during firmware version bumps, causing a delta spike on the first post-OTA slot.
*   **Fix:** Explicitly clear `total_rf_pulses_32` and `last_raw_rf_count` when a version change is detected.

### 7. I2C Contention Hardening
*   **Issue:** Long I2C reads from sensors could starve the RTC task's watchdog.
*   **Fix:** Added `esp_task_wdt_reset()` calls inside the I2C mutex-wait loop in `rtcRead.ino`.

## 🟢 Technical Debt & Cosmetics
*   Reorganized `globals.h` #includes for better compilation speed.
*   Replaced non-standard emoji indicators with ASCII-safe tags (`[WARN]`, `[ERR]`, `[CRIT]`) for maximum serial terminal compatibility.
*   Verified race-free operation for both `DEBUG 0` (Production) and `DEBUG 1` (Dev) modes.

---
**Build Recommendation:** Standard 8MB Flash ESP32 Partition. Ensure `FIRMWARE_VERSION` matches "5.78" in `user_config.h`.
