# 🌟 Firmware v5.66: Complete Mission Hardening & OTA Pipeline

Version 5.66 represents a massive stabilization and architectural refactor compared to the v5.63 and v5.65 lineages. It directly tackles the most insidious RTOS race conditions, network deadlocks, memory leaks, and "Silent Failures" that previously manifested under severe multi-carrier network congestion. This release fortifies the entire `AIO9_5.0` telemetry stack to guarantee survival in 100% autonomous, zero-touch environments.

## 🔴 1. Critical Deadlock Defeats (RTOS Mutex Audits)
* **The `send_health_report` Lock Inversion Fix:** Completely eradicated a catastrophic RTOS deadlock condition. v5.65 incorrectly grabbed `fsMutex` before `modemMutex`, causing a fatal freeze if the 15-minute `send_http_data` background task claimed `modemMutex` first. The lock ordering is explicitly inverted to safely grab `modemMutex` before proceeding to SPIFFS files.
* **The `copyFromSPIFFSToFS` Fix**: Eliminated the exact same lock ordering deadlock in the secondary FTP pipeline.
* **The `fetchFromHttpAndUpdate` Mutex Leaks**: Patched three fatal exit-paths during Over-The-Air (OTA) firmware installations where early-exit failures (file size misread, partition initialization error, memory crashes) failed to release `modemMutex`. This previously guaranteed that a failed OTA would eternally freeze the modem, requiring a hardware field reset.

## 🛰 2. Network Congestion & Telemetry Enhancements
* **The BSNL TCP Purgatory Teardown (`+HTTPACTION 706/714`)**: Solved the BSNL TCP cache-poisoning issue where zombie sockets would hang continuously. Added a massive 5000ms "Carrier Breather" after `AT+HTTPTERM` allowing sluggish towers to properly purge the old data stream before trying to fetch/write again.
* **Fallback Health Report Window**: Introduced a graceful BSNL network congestion fallback. If the primary `11:00 AM` daily health report slot gets slammed by server/network delays, it will intelligently retry between `12:00 PM - 12:15 PM` to guarantee the payload delivers. 
* **True-FAIL Diagnostic (`diag_cdm_status`)**: Prevented intermediate health retries from causing early panic alarms on the server (`"FAIL"`). The system now patiently waits. If the time crosses `13:00` (1:00 PM) and the status is still `"PENDING"`, it officially flags the entire day as a `FAIL` to accurately represent network abandonment.
* **Wider HTTP Processing Window**: Increased the `is_valid_window` scheduler slot tolerance from `5` minutes to `10` minutes (`minutes_into_interval <= 10`). This guarantees that nodes delayed by lengthy modem cold-starts or URC floods won't accidentally drop their entire 15-minute payload segment.

## 💾 3. Core Software / C++ Architectural Enhancements
* **The One Definition Rule (ODR) Eradication**: Handled the colossal `Multiple Definition` GCC compiler crisis. Moving from v5.65 to v5.66, over 120 global variables (including complex `RTC_DATA_ATTR` structs) were meticulously migrated out of header files and natively defined strictly in `AIO9_5.0.ino` with `extern` stubs in `globals.h`. This guarantees 100% memory isolation between the two CPU cores.
* **`wakeup_reason_is` Determinism**: Hard-initialized the critical Power-On Boot variable to zero (`volatile int wakeup_reason_is = 0;`), entirely eliminating the dangerous C++ assumption that the RTOS BSS segment will naturally zero itself out.
* **FTP Safe Validation**: Enforced a minimum array length validation for `ftp_daily_date[]`. This permanently stops a malformed JSON command from triggering a root directory query (`/`) that locks up SPIFFS iteration queues.
* **Precision Sensor Timing (Solar)**: Protected the delicate Solar ADC read on `AIO9_5.0.ino` boot from noisy modem activity. Additionally, the 15-minute `scheduler.ino` cycle actively skips polling the Solar panel natively if the `gprs_started` Engine is running, completely eliminating garbage `0.0V` measurements caused by RF voltage droops.
* **I2C Bus False Hang Prevention**: Hardened `recoverI2CBus()` to natively scan the `SDA` pin structure before arbitrarily launching lock-recoveries during deep sleep power-down transitions.

## ☁️ 4. Server-Side Infrastructure Upgrades (v5.66 Companion API)
* **OTA Python Tuple Parsing Bug**: Fixed an excruciating mathematical crash in `ota_service.py` where parsing floating version strings caused `5.10` to round down to `5.1`, forever bricking devices on older branches from fetching newer firmware.
* **SQLite Cache Throttling**: Ripped out the brute-force `PRAGMA table_info` schema calls happening 96 times per day *per station* in `health.py`. Column extraction is now lazily cached in-memory instantly reducing CPU load by over 50%.
* **FTP Clear Tolerance Limit**: Increased the maximum `queue >= 50` destructive auto-delete safety threshold to `400` in the backend. An AIO9 node can now survive 4 days totally offline without the server accidentally shredding the backed-up rainfall records once it dumps online.
* **Test Station Lock Bypass Removed**: Discarded the 1931 hard-coded test loops.
* **Admin Token Purge**: Safely rotated credentials and destroyed the inadvertently checked-in `cookies.txt` artifact from the production backend.
