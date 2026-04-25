# Release Notes: AIO9 v5.89 (Hardened Stable Release)

**Date**: 2026-04-25  
**Comparison Base**: v5.83 (Git)  
**Target Hardware**: AIO9 v3.0 (A7672S / ESP32)  

---

## 1. Structural Handshake Synchronization
*   **[CRITICAL] Health Handshake Alignment**: Re-engineered the Health Report sequence to match the "Proven Handshake" logic used in regular data uploads. By removing the explicit `AT+HTTPPARA="CID",1` binding, we eliminated the **"Sticky Context" rejections** that caused the modem to hang on BSNL 2G networks.
*   **Adaptive Network Breathers**: Implemented carrier-aware delay logic. On BSNL networks, the system now grants a 3000ms "IP stabilization breather" post-initialization, ensuring the internal stack is ready before the first POST request.

## 2. Memory & Buffer Hardening (Anti-Crash)
*   **[HIGH] Safe Command Parsing**: Eliminated all potential `NULL` pointer dereferences in the remote command parser. Every `strstr` and `strchr` call is now wrapped in a two-step validation (NULL check first, then offset calculation).
*   **Buffer Bounds Enforcement**: Replaced all remaining raw `strcpy` calls with `strncpy` and `snprintf` in the telemetry building blocks, ensuring no data spike can overwrite adjacent task stacks.
*   **Static Buffer Strategy**: Standardized the use of `modem_response_buf` (2048 bytes) across all GPRS tasks, protected by `modemMutex`, eliminating heap fragmentation from transient `String` allocations during HTTP responses.

## 3. Concurrency & Mutex Safety (Anti-Freeze)
*   **[CRITICAL] VFS Concurrency Shield**: Wrapped all shared SPIFFS operations—including the new `SET_WIFI_PASS` file write and `FTP_DAILY` date lookups—inside rigorous `xSemaphoreTake(fsMutex)` blocks. This prevents "VFS Guru Meditations" that occurred when the GPRS task and Scheduler task attempted to access the Flash simultaneously.
*   **Recursive Deadlock Prevention**: Standardized the `alreadyLocked` flag across helper functions (`reconstructSentMasks`, etc.), allowing them to safely execute while either holding or not holding a mutex.

## 4. Wind Speed Precision & Calculation
*   **[NEW] Parameterized Scaling**: Transitioned to a fully configurable `WIND_TEETH_COUNT` model. This ensures accurate m/s conversions for both 2-tooth and 4-tooth anemometer hardware variations.
*   **Acceleration Guard (Anti-Spike)**: Implemented a physical limit gate (25.0 m/s²). The system now automatically rejects phantom wind spikes caused by electrical noise on the pulse line, capping the instantaneous acceleration to realistic atmospheric levels.
*   **32-Bit Atomic Accumulation**: Pulse counting now uses 32-bit variables with `portENTER_CRITICAL` protection, preventing data loss during the 16-bit wrap-around of the ULP (Ultra Low Power) hardware counter.

## 5. SD Card Fail-Safe Redundancy
*   **[CRITICAL] Deadlock-Free Backups**: The `copyFilesFromSPIFFSToSD()` routine was completely rewritten with a two-phase locking strategy. This allows users to trigger manual SD backups from the keypad without risking a mutex conflict with the background logger.
*   **4mA Deep-Sleep Power Save**: Implemented an explicit SPI teardown sequence (`SD.end()` + `SPI.end()`) before deep sleep. This eliminates the persistent 4mA parasitic drain from the SD controller, significantly extending battery life.
*   **OTA Protection**: Added a "Rename-with-Fallback" safety mechanism for SD firmware updates. If the system fails to rename `/firmware.bin` to `/firmware.done`, it now executes a hard `remove()` to prevent the unit from entering an infinite re-flash loop.

## 6. Remote Management Enhancements
*   **`SET_WIFI_PASS` Hardening**: Added a security length gate (Minimum 8 characters) for WPA2 compliance. The command now performs an atomic write via a `.tmp` file to prevent corrupted credentials if power is lost during the update.
*   **`FTP_DAILY` Date Extraction**: Polished the FTP parser to extract the specific target date from the JSON `"p"` field, enabling surgical re-uploads of historical gaps rather than just the default current day.

## 7. Hardware Integrity & Recovery
*   **Bit-Banged I2C Pulse Recovery**: Implemented a low-level SCL clock-out routine (10 pulses) to clear the I2C bus if a slave device (RTC/Sensor) holds the SDA line low due to electrical noise.
*   **ADC2 Radio Interlocks**: Strictly enforced the `!wifi_active` gate for Solar ADC reads, preventing the hardware controller conflict between the ESP32 internal ADC and the WiFi radio.

---

**Status**: **STABLE / PRODUCTION READY**  
**Field Verification**: **PASS** (Successful Telemetry, Health, and SMS cycles verified on BSNL 2G).
