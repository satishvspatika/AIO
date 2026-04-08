# Release Notes - AIO9 v5.81 Hardened
**Date:** April 8, 2026
**Target Identifiers:** `SYSTEM 0` (TRG), `SYSTEM 1` (TWS), `SYSTEM 2` (TWS-RF)
**Status:** FULL PRODUCTION RELEASE (Audited & Verified)

## Overview
Version 5.81 represents a massive culmination of structural audits, memory optimization, and edge-case hardening. This release specifically targets "silent killer" race conditions, long-term memory resilience, and deep-tier cellular recovery, graduating the architecture from stable to mathematically robust.

---

## 🟢 Critical Architecture Upgrades

### 1. Robust Record Extraction (Zero-Heap)
*   **Issue:** The legacy extraction logic seeked exactly 2 records backward to find the final line. When sensor strings unpredictably grew in width (e.g., negative floats adding a `-` symbol), the seek would land incorrectly inside the previous record, causing stale data transmission (e.g., 09:15 data sent at 09:30).
*   **Resolution:** Completely refactored `send_http_data()` to iterate safely using a new zero-heap C-string function `read_line_to_buf()`. The function now seeks backward deeply and loops safely to `EOF`, mathematically guaranteeing the absolute latest reading is fetched.

### 2. Orphan Mutex Security [V4-H01]
*   **Issue:** If the network abruptly dropped mid-FTP handshake (CID 9 bounce), the stack re-opened the payload file without legally establishing the FreeRTOS `fsMutex` lock. When the block ended, an "orphan" `xSemaphoreGive` executed, destroying the concurrency counter and inevitably causing a multithreading crash later in the day.
*   **Resolution:** Explicitly wrapped the CID 9 recovery `SPIFFS.open/read` cycle inside `if (xSemaphoreTake(fsMutex...))` within `gprs_ftp.ino`. 

### 3. Hardware Watchdog Shielding [HOL-H01]
*   **Issue:** If the cellular network went completely dark, the system plunged into deep Tier-4 recovery mapping (running AT+CNMP, AT+COPS=0, AT+CGATT). These commands could consume ~11 continuous seconds. At `10s`, the ESP32 hardware watchdog timer (WDT) would panic and ungracefully reboot the unit before the modem could fix itself.
*   **Resolution:** Injected `esp_task_wdt_reset()` calls directly into the deep polling layers of `gprs_health.ino`, permitting the system indefinite time to securely rescue dead connections.

---

## 🟡 Reliability & Precision Enhancements

### 4. Backlog Pointer Isolation [HOL-M02]
*   **Issue:** `SYSTEM 0` used a globally shared scratchpad (`modem_response_buf`) to parse the `unsent_pointer.txt` integer. If the background continuous RTC checker snagged the modem buffer at the exact same microsecond, it could corrupt the upload pointer.
*   **Resolution:** Swapped the global string for a tightly localized `char ptr_buf[16]` inside `gprs_http.ino`. The pointer math is now perfectly isolated in a private memory bubble.

### 5. FTP Backlog Batch Management
*   **Issue:** Sending massive `.txt` files containing 50+ offline records would invoke heavy HTTP constraints and crash the GPRS stack on Airtel/BSNL towers. 
*   **Resolution:** Verified the 15-record auto-slicer (`FTP_CHUNK_SIZE`). It flawlessly extracts the top 15 records to an active `.kwd` file, shunts the rest to `/ftpremain.txt`, and uses an instant pointer-flip to cleanly slice massive queues down without bandwidth penalties or physical data loss on sudden power cuts.

### 6. Atomic Timestamp Roll-over Protection 
*   **Issue:** Midnight transitions during asynchronous FTP file generation could create disjointed file names if the hour flipped from 23 to 0 in the middle of naming execution.
*   **Resolution:** Established an atomic snapshot mechanism (`portENTER_CRITICAL`) at the very top of `prepare_data_and_send()` to freeze time exactly at entry, securing immutable file naming.

### 7. Instant Deep-Sleep on Manual Triggers 
*   **Issue:** Manual diagnostics triggers previously possessed undefined sleep routes if the scheduler was asleep.
*   **Resolution:** Firing a manual trigger now securely routes `sync_mode = eHttpStop` from within the interrupt lock, commanding instantaneous board sleep after data dispatch rather than waiting 15 minutes.

### 8. Daily Backlog Drain (Orphan Recovery)
*   **Issue:** Low-volume stations could have a single "orphan" record sit in the backlog indefinitely if it never reached the >2 record threshold for a 3-hourly push.
*   **Resolution:** Implemented a "Daily Drain" at **08:45 AM**. During this morning window, the threshold is bypassed, guaranteeing that even a single backlog record is cleared every 24 hours without an SMS command.

---

## 🛡️ Long-Term Stability Validation
*   **True Zero-Heap Integrity**: Swept entire architecture for raw `String` objects — ZERO FOUND. 0% heap fragmentation possible over time. 
*   **Buffer Capping**: Verified 0 unsafe `sprintf` and unbounded `strcpy` commands. Bounds checked dynamically using `snprintf`.
*   **SD Card Endurance**: Verified "Leave-and-Forget" mode: updates via `.bin` will correctly write MD5 hashes and Version identifiers dynamically to deep `SPIFFS`, ensuring firmware never loops/reflashes recursively when the SD card remains docked for decades of CSV backups. 
*   **I2C Boot Stability**: Deep Sleep reset protocols functionally verified to boot securely via ESP32 `setup()`, guaranteeing pure state initialization across multiplexers and BME/HDC chips.
