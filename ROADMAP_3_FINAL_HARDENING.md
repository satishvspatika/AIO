# ROADMAP 3: Ultimate Data Sanctity & Hardening
*(Distilled from the v5.66 Production Hardening Report)*

This roadmap filters the expansive analysis down to the **absolute essential** fixes required to guarantee data survival, prevent live memory races, and ensure the filesystem never destroys itself. Features that do not threaten system stability or data loss have been de-prioritized.

## 🔴 Tier 1: Filesystem & Data Survival (Critical)
These issues directly cause permanent data loss or silent filesystem corruption.

- [ ] **1. SPIFFS Auto-Format Nuke Protection**
    - **Issue:** `SPIFFS.begin(true)` will instantly wipe all historical data and backlog files if the flash fails to mount just once (e.g. from a voltage brownout during boot).
    - **Action:** Change to `SPIFFS.begin(false)` in `AIO9_5.0.ino`. If it fails, log an error before doing a targeted reformat as a last resort.
- [ ] **2. Primary Data Write Verification**
    - **Issue:** If SPIFFS is 100% full, `file.print()` fails silently. The system thinks the data is saved, but it's lost forever.
    - **Action:** Add a capacity check (`SPIFFS.usedBytes()`) before writing in `scheduler.ino`, and verify `file.print()` returns > 0.
- [ ] **3. VFS Handle Exhaustion (`signature.txt`)**
    - **Issue:** The scheduler writes `signature.txt` outside `fsMutex`. If the GPRS task is concurrently trying to send a backlog, the ESP32's 5-file handle limit is exhausted and the system crashes.
    - **Action:** Wrap the `signature.txt` file write (line ~1591 in `scheduler.ino`) inside `fsMutex`.
- [ ] **4. Brownout Filesystem Corruption**
    - **Issue:** Writing to SPIFFS while the battery is below 3.4V can cause a hardware brownout *during* the flash write, corrupting the block.
    - **Action:** Adopt a hard ceiling in `scheduler.ino` to immediately sleep the device if `li_bat_val < 3.4V`, aborting the write to protect historical data.
- [ ] **5. Orphaned File Bloat (`delete_multiple_files`)**
    - **Issue:** The cleanup array is hardcapped at 24 files. An offline device generates 8+ files a day. After 3 days, old files are silently skipped and will permanently choke SPIFFS.
    - **Action:** Increase `toDelete[24]` to `48` string capacity in `global_functions.ino`.

## 🟠 Tier 2: Live Races & Sync Failures (High)
These issues cause random mid-operation crashes or mismatched timing gaps.

- [ ] **6. `get_total_backlogs()` SPIFFS Race**
    - **Issue:** `gprs.ino` repeatedly calls this function to dynamically check backlog counts, which opens files without holding `fsMutex`. It collides with the Scheduler writing those same files.
    - **Action:** Wrap the internals of `get_total_backlogs()` in `global_functions.ino` with `fsMutex`.
- [ ] **7. Mid-Slot Deep Sleep Race**
    - **Issue:** `loop()` evaluates sleep conditions non-atomically. It can decide to sleep right as the `schedulerBusy` flag flips to true, freezing the CPU mid-calculation.
    - **Action:** Add a 50ms guard and double-check `schedulerBusy` before calling `start_deep_sleep()`.
- [ ] **8. DST Time-Shift Corruption**
    - **Issue:** `mktime()` uses uninitialized memory for `tm_isdst`, occasionally shifting Indian timestamps completely incorrectly by 1 hour backwards or forwards.
    - **Action:** Hardcode `gmt_time->tm_isdst = 0;` in `convert_gmt_to_ist()`.
- [ ] **9. OTA WDT Timeout on 2G**
    - **Issue:** 300KB OTA files take 60+ seconds on weak 2G. The hardware Watchdog Timer (WDT) triggers a panic reset mid-download.
    - **Action:** Call `esp_task_wdt_reset()` manually inside the `writeStream()` byte loop in `gprs.ino`.

## 🟡 Tier 3: Hardened Thread Safety (Medium)
These are quick one-line patches to guarantee execution safety.

- [ ] **10. `strtok` Reentrancy Crash**
    - **Issue:** `strtok` modifies memory via a global static pointer. If an interrupt fires or RTOS changes tasks, the pointer corrupts.
    - **Action:** Swap to threaded `strtok_r` inside `checkRainfallIntegrity()`.
- [ ] **11. Sleep Timer Early-Wake Drift**
    - **Issue:** The sleep calculation uses `current_sec`, which is polled every 5 seconds. The calculation can be up to ~90s stale, waking the device early so it completely skips a 15-minute slot.
    - **Action:** Force an immediate `rtc.now()` read at the very top of `start_deep_sleep()`.
- [ ] **12. NVS Station Name Garbage**
    - **Issue:** A corrupt NVS read of the unit name will cascade garbage bytes into filenames, severely corrupting SPIFFS parsing.
    - **Action:** Validate `station_name` to ensure it only has alphanumeric bounds.

---
*Roadmap generated from architectural and deterministic risk analysis.*
