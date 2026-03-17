# Release Notes v5.57
**"Backlog Integrity & Surgical Cleanup"**

This version resolves critical data synchronization issues where unsent records were being reported with incorrect timestamps or corrupted due to fixed-length pointer drifts in the SPIFFS flash.

### 📜 Smart Backlog Data Handling:
- **Variable-Length Pointer Logic**: Replaced the rigid "Fixed Jump" (+45 bytes) loop with a dynamic `file.position()` tracker. The firmware now reads exactly to the end of each physical line (`\n`), preventing "pointer drift" that previously caused garbage data and year-0004 timestamp errors.
- **Backlog "Time Sticking" Fix**: Corrected a variable shadowing bug in `prepare_data_and_send` where the local `charArray` pointer was not being updated for unsent records. This ensured every backlog record now carries its original historical timestamp to the server instead of being stuck at the current time (e.g., 19:30).
- **Gap & Blank Line Resilience**: Implemented a string-trimming and minimum-length check. The system now autonomously skips blank lines and record "gaps" in `unsent.txt` without bloating the server payload with empty records.

### 🛠️ Hardware & Firmware Stability (Critical Fixes):
- **RTC 2060 Future-Proofing**: Extended the RTC year boundary check from 2040 to **2060** to prevent future time-synchronization lockups.
- **Mutex SPIFFS Protection**: Fixed three instances where `fsMutex` timeouts led to unguarded writes. The system now safely skips the write and logs a diagnostic warning, preventing flash corruption during background OTA tasks.
- **WDT Recovery Fix**: Fixed a bug where `healer_reboot_in_progress` could get stuck `true` after a Watchdog or Brownout reset. It is now unconditionally cleared in `setup()` to ensure valid counter resets.
- **ULP Accumulator Sync**: Added synchronization for 32-bit RF accumulator anchors (`last_raw_rf_count`) immediately after ULP zeroing, preventing spurious rainfall deltas during resolution changes.
- **Memory Safety**: Increased `lcdkeypad` task stack size from 4096 to **6144 bytes** to provide a safe buffer for complex UI rendering and SPIFFS operations.
- **Accurate Night Diagnostics**: Adjusted the `diag_ndm_count` range from `>= 50` to **`>= 49`**, ensuring the 9:00 PM record is correctly included in night-time data counts.
- **Hardware RNG Jitter**: Migrated health report jitter from `random()` (LCG) to **`esp_random()`** (True Hardware RNG). This ensures staggered server transmission even if 100+ units reboot simultaneously at the same 15-minute boundary.

### 📡 Smart Health & Cloud Orchestration:
- **System-Aware Diagnostics**: Implementation of System-Specific status reporting. **TRG (Rainfall-only)** units now bypass Temperature/Humid/Wind sensor checks, eliminating the "Phantom TH-Er" status. **TWS** units similarly only monitor their respective sensor arrays.
- **GPS Battery Saver**: Implemented a **24-hour cooldown** on the Server for `GET_GPS` commands. This prevents stations in low-fix or indoor locations from repeatedly attempting 90s GPS acquisition every 15 minutes, significantly extending battery life.
- **Command Feedback loop**: Enhanced the server to track command IDs and re-queue items stuck in "SENT" state for >3 hours (e.g., if a device rebooted mid-execution without sending a result).
- **Health Reporting Interval**: Changed default `TEST_HEALTH_DEFAULT` to **0 (Daily)**. This reduces cellular overhead and battery consumption by default, while still allowing manual or server-triggered 15-min health checks when needed.
- **Accurate Backlog Reporting**: The `unsent_count` logic now uses the length-validated `countStored()` engine instead of a raw newline count, ensuring the server only triggers rescue tasks for real data records.

### 📟 High-Resolution Diagnostics UI:
- **Smart Backlog Counting**: The LCD "B:" counter now only increments for lines containing valid data (length > 10), providing an accurate representation of unsent records rather than a raw newline count.
- **Snappier UI Refresh**: Reduced the LCD "linger delay" from 2.0s to 0.5s. Scrolling to the *HTTP FAIL STATS* screen now provides near-instant feedback of the backlog state without sluggishness.

### 🌐 Server-Side Precision Tools:
- **Surgical Delete Feature**: Added a new route to the Spatika Fleet Server allowing for per-category station deletion. This enables administrators to remove "phantom" entries (like station 1930) from specific firmware groups (e.g., KSNDMC_TWS) without wiping the station's global history.
- **Enhanced Fleet Actions**: Integrated a "✂ Remove from Group" shortcut into the Fleet Summary dashboard for quick data hygiene.

---
*Status: Verified / Compiled / Ready for Deployment*
