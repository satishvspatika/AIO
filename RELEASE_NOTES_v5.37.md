# Release Notes: v5.37 (Feb 21, 2026 Update)

## 🎯 Release Overview
A diagnostic refinement update specifically focused on "Intelligent Health Reporting" across different system types (TRG vs TWS), reducing false hardware alarms, and improving report clarity on the Google Sheets dashboard.

---

## ✨ Key Enhancements

### 1. **Intelligent System Diagnostics** 🧠
- **Problem:** TRG-only units (SYSTEM 0) were triggering false `HDC_SENSOR_FAULT` and `WIND_DIR_FAULT` alarms because they lack those physical sensors.
- **Solution:** 
    - The diagnostic summary now dynamically detects the `SYSTEM` type. 
    - **SYSTEM 0 (TRG)**: Status remains `SYSTEM_OK` even without Temperature, Humidity, or Wind sensors.
    - **SYSTEM 1/2 (TWS)**: Fault logic remains active only where sensors are required.
- **Impact:** Cleaner, actionable health records on the master monitoring sheet.

### 2. **Concise Sensor Status String** 📊
- **Problem:** The `Sensors:` field in the health report was cluttered with "NA" values for sensors not relevant to the unit's system type.
- **Solution:** Redesigned the status string to be system-specific:
    - **TRG (SYSTEM 0)**: Displays strictly `RF:OK`.
    - **TWS (SYSTEM 1)**: Displays `TH:OK,WS:OK,WD:OK`.
    - **TWS-RF (SYSTEM 2)**: Displays the full suite `TH:OK,WS:OK,WD:OK,RF:OK`.
- **Impact:** Simplified data analysis and reduced transmission payload size.

### 3. **Production Data Decoupling (Industrial Grade Fix)** 🛡️
- **Problem:** Background tasks (`tempHum`) were asynchronously overwriting global buffers (`inst_temp`, `inst_hum`) while the scheduler was preparing uploads. This caused inconsistent data formats (Log 2 showing 2 decimals instead of 1) and potential data corruption.
- **Solution:** 
    - Isolated **UI Strings** (`temp_str`, `hum_str`) from **Production Buffers** (`inst_temp`, `inst_hum`). 
    - The scheduler now has exclusive ownership of production buffers. 
    - Replaced `signal_strength` (0-31) with `signal_lvl` (dBm) across all SPIFFS/FTP logs for consistent high-resolution monitoring.
- **Impact:** 100% predictable data formatting and zero chance of 'snapshot contamination' by background tasks.

### 4. **LCD Field Security (UI Hardening)** 🔒
- **Problem:** Critical sensor data and system status fields could be entered into "Edit Mode" via the keypad, leading to accidental text corruption on the display.
- **Solution:** 
    - Implemented a **Read-Only Lock** on all sensor data (Temp, Hum, Wind, RF) and system diagnostic fields (Signal, Battery, Registration).
    - Keys are only responsive for configuration fields (`STATION`, `DATE`, `TIME`, `RESOLUTION`) and action triggers (GPS, STATUS, WIFI).
- **Impact:** Prevents accidental modification of displayed data and provides a more professional, "tamper-proof" UI experience for field technicians.

### 5. **Network & FTP Stability (Backported Fixes)** 📶
- **Registration Recovery:**
    - The "Radio Kick" (`AT+COPS=2` then `AT+COPS=0`) is now triggered if the modem is stuck in **Searching (Status 2)** or **Denied (Status 3)** for more than 25 seconds.
    - Total registration retries increased from **20 to 30** (approx. 50s extra search time) to ensure the modem has enough time to latch onto BSNL/weak towers after a cold boot.
- **FTP Reliability (Error 13 Mitigation):**
    - Implemented **DNS Warm-up** (`CDNSGIP`) before FTP login to ensure the modem's internal DNS cache is populated, eliminating "Handshake Error 13".
    - Increased handshake settling delay to **3000ms** to ensure cellular data context is fully stable before login.

### 6. **Self-Healing & Proactive Learning Architecture** 🤖 (NEW)
- **Problem:** Persistent GPRS "Zombie Sessions" (HTTP 713/714 errors) where the modem stays half-alive during sleep, retaining stale/corrupt network contexts that block future uploads.
- **Solution:** 
    - **Self-Healing Hardware Control**: Implemented hardware-level power isolation using `gpio_hold_en` on GPIO 26 during Deep Sleep. This guarantees a "bone-dry" modem reset every 15 minutes, clearing all internal latch-up states.
    - **Proactive Failure Learning**: The system now tracks `diag_consecutive_http_fails` in RTC memory. If failures persist across two cycles, it "learns" the session is corrupt and proactively triggers a deep IP stack cleanup (`CIPSHUT` + `CGACT=0`) before the first send attempt.
    - **Adaptive Retry Throttling**: The system "judges" its environment by signal strength. If signal is extremely poor (< -108 dBm), it proactively reduces retries to save battery, knowing transmission success is unlikely.
    - **Autonomous DNS Stabilization**: If DNS resolution fails, the system now automatically forces the modem's DNS configuration to Google (8.8.8.8), bypassing unreliable or broken local tower DNS stacks.
- **Impact:** Significant reduction in persistent "HTTP 713" hangs and improved battery longevity in weak signal areas.

---

## 🔧 Bug Fixes & Refinements

### 1. **Unified Fault Labeling**
- Renamed `HDC_SENSOR_FAULT` to **`TH_SENSOR_FAULT`** in status summaries. This provides a more universal description of Temperature/Humidity subsystem health regardless of the specific chip (HDC1080/2022) brand.

### 2. **Signal Reporting Consistency**
- Standardized dBm-level signal reporting across all internal storage formats to match the primary HTTP payload.

### 3. **Internal Diagnostics**
- Verified standard data formatting (1 decimal place for sensors) across all new recovery paths.

### 4. **Version Consistency**
- Maintained production branch at **v5.37** while incorporating these stability enhancements as a rollout update.

---

## 📂 Configuration Mapping

### Modified Files:
- `globals.h`: Added `diag_consecutive_http_fails` and hardware control prototypes.
- `gprs.ino`: Implemented proactive failure learning, adaptive retries, and smart shutdown logic.
- `gprs_helpers.ino`: Added autonomous DNS stabilization and verified bearer recovery.
- `AIO9_5.0.ino`: Added hardware-level GPIO hold release in setup.
- `global_functions.ino`: Implemented `gpio_hold_en` for modem power isolation during sleep.
- `scheduler.ino`: Implemented Production Data Snapshots and standardized logging.
- `tempHum.ino`: Decoupled background sensor polling from production buffers.
- `lcdkeypad.ino`: Implemented Field Locking security logic.

## ✅ Validation & Field Test Results (Verified 2026-02-21)

The v5.37 firmware underwent rigorous "Live Hardware Disruption" testing to verify resilience:

- **SIM Removal Resilience**: 
    - Verified that system intelligently detects SIM removal and switches to local SPIFFS queuing.
    - Successfully cleared 1+ hour of backlog data automatically upon SIM re-insertion via FTP trigger.
- **Power-Cycle Gap Filling**:
    - Verified that manual power-off during a logging slot is recovered.
    - On the subsequent boot, the system accurately detects the gap and interpolates/fills missing records to ensure data continuity.
- **Modem Recovery**:
    - Verified HTTP/714 recovery logic. The system successfully detects stale network contexts and restores communication without manual intervention.

---

**v5.37 is production-ready with verified "Zero-Data-Loss" resilience!** 🚀
