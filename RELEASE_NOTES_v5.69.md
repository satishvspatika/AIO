# 🦅 Spatika AIO 9.5.0 - Release Notes v5.69
**Release Date:** March 26, 2026
**Architecture:** ESP32-D0WD-V3 (8MB Flash)
**Focus:** Concurrency Hardening (ABBA Deadlock Eradication), Atomic Timekeeping Precision, I2C Self-Healing, and Multi-Core Flag Volatility.

This version (v5.69) serves as a critical stability patch following the major v5.68 hardening phase. It specifically resolves deep-seated race conditions between the Modem (Core 0) and the Sensor Scheduler (Core 1), while introducing hardware-level bus recovery.

---

## 🔒 PHASE 1: CONCURRENCY & MUTEX DISCIPLINE (ABBA DEADLOCK FIX)
*Eliminating permanent dual-core lockups by enforcing a strict global acquisition sequence.*

*   **Global Mutex Hierarchy Enforcement**: Standardized all codebase task logic to strictly adhere to the `modemMutex -> fsMutex -> i2cMutex` acquisition order. This mathematically prevents "ABBA" deadlocks where two tasks wait for resource locks held by each other.
*   **System 0/1/2 Backlog Perfection**: Wrapped all remaining unguarded `SPIFFS` file-access paths (including FTP daily-backlog and JSON-API retry logic) inside the `fsMutex` semaphore. This prevents file-handle corruption during high-frequency sensor logging.
*   **Boot-Time Collision Guard (Issue 24)**: In `windSpeed.ino`, the initial loading of `prevWindSpeedAvg` from SPIFFS is now explicitly protected by `fsMutex`. This ensures that early-boot task initialization doesn't collide with the primary file-system mounting sequence.

---

## 🕒 PHASE 2: ATOMIC TIMEKEEPING & MIDNIGHT INTEGRITY
*Ensuring chronological precision across the 00:00:00 boundary via hardware spinlocks.*

*   **`rtcTimeMux` Deployment**: Implemented a dedicated FreeRTOS Spinlock (`portMUX_TYPE rtcTimeMux`) for all master temporal variables. This prevents "state-tearing" where a context switch midway through a time-read could result in a record having a "Day N" date but a "Day N-1" hour.
*   **Atomic Date/Time Snapshots**: Re-architected `scheduler.ino` and `lcdkeypad.ino` to utilize local `struct tm` snapshots for data-log headers. This provides a single source of truth for every 15-minute record, eliminating "mismatched" field timestamps during midnight roll-overs.
*   **Volatile Status Flags**: Converted critical process indicators (e.g., `gprs_started`, `health_in_progress`, `bearer_recovery_active`) to `volatile bool`. This forces the compiler to fetch real-time states from SRAM, ensuring Core 1 (UI/Sensors) instantly observes state changes initiated by Core 0 (Modem).

---

## 🛠️ PHASE 3: HARDWARE SELF-HEALING & BUS RECOVERY
*Direct silicon-level interventions to resolve frozen I2C peripherals.*

*   **I2C Bus Recovery (`recoverI2CBus`)**: Engineered a surgical hardware-recovery routine that manually bit-bangs the SCL/SDA pins (toggling clock pulses and stop conditions) if the I2C bus hangs due to electrical noise or LCD power fluctuations. This attempts to unfreeze the bus without triggering a full system reboot.
*   **Sync-Mode State Modernization**: Replaced legacy "magic number" transitions in `gprs.ino` and `rtcRead.ino` with a formal `eSyncMode` enum (`eHttpStop`, `eExceptionHandled`, etc.). This improves logic readability and prevents state-machine starvation during modem failures.
*   **ADC Noise Rejection**: Enhanced the `get_calibrated_battery_voltage()` logic with a 10-sample median filter, providing more stable battery telemetry during active GPRS radio bursts (2A peaks).

---

## 🌐 PHASE 4: GPRS & NETWORKING ROBUSTNESS
*Hardening the modem driver against stack smashes and noisy cellular environments.*

*   **Static Buffer Optimization**: Converted the massive 2KB character array in `waitForResponse()` from a stack-allocated object to a `.BSS` segment object. This permanently eliminates the "Core 0 Stack Smash" kernel panics observed during lengthy SIMCOM command timeouts.
*   **`carrier_aware_apn_search()` Harden**: Integrated automatic stripping of non-ASCII characters from modem responses. This prevents "ghost" APN configurations from being saved to SPIFFS when a cell tower returns corrupted registration strings due to weak signal strength.
*   **Unified Boot-Wipe Protect**: Consolidated the SPIFFS cleanup logic on system startup. Ensures that stale temporary files (like `/signature.tmp`) from a previous crash are automatically purged before the first 15-minute sensor acquisition cycle.

---

## 🪁 PHASE 5: SENSOR INTEGRITY & DATA FIDELITY
*Filtering electrical noise and ensuring ULP accuracy.*

*   **Wind Speed Rolling-Average Filter**: Upgraded the `windSpeed` task to employ a deeper threshold comparison against `prevWindSpeedAvg_str`. This effectively rejects high-frequency electrical "spikes" common in long cable runs during thunder-cell approach.
*   **ULP Anchor Calibration**: Fine-tuned the Ultra-Low Power pulse counter anchors for Rain and Wind. This ensures that the 32-bit RTC accumulators maintain absolute accuracy even if the main CPU enters deep sleep mid-pulse.

---

### *Version Check Verification:*
- **SYSTEM 0**: `TRG9-xxx-5.69`
- **SYSTEM 1**: `TWS9-xxx-5.69`
- **SYSTEM 2**: `TWSRF9-xxx-5.69`

---
*Created by Antigravity AI for Spatika Information Technologies Pvt Ltd.*
