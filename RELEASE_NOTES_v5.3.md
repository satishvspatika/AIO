# Release Notes - ESP32 AIO Firmware v5.3
**Release Date:** February 15, 2026
**Target Hardware:** ESP32 (AIO Build)
**Status:** Stable / Production Ready

## [v5.3] - Major Diagnostic & Reliability Update

### 🚀 Highlights
This version introduces a comprehensive **Remote Diagnostic System** and a **Live Dashboard** integration with Google Sheets, while significantly optimizing memory footprint.

### 🛠 Added Features
*   **Remote Health Monitoring:** 
    *   Automated Health Reports sent to Google Sheets (Target: 11:15 AM and 11:00 PM).
    *   Health metrics include: RTC Battery health, Uptime (Hrs), SPIFFS usage, Signal Registration performance, and Reset Reasons.
    *   **Plain English Status Reporting:** Remote status strings like `SYSTEM_OK`, `BROWNOUT_RECOVERY`, `RTC_BATTERY_FAULT`, etc.
*   **Live Dashboard Mode:** Integration with Google Apps Script to update a single specific row per station (no scrolling history).
*   **Persistent Diagnostics:** Added tracking for average/worst-case network registration times and failure counts across reboots.

### 🔧 Improvements & Reliability
*   **HTTPS/SSL Robustness:** 
    *   Optimized SSL handshake with mandatory stack-clear delay (200ms).
    *   Increased HTTPS timeouts to 35s for stable operations in weak signal areas.
    *   Automated SNI configuration for `script.google.com`.
*   **Smart APN Management:** Updated to prioritize stored successful APN configurations based on SIM ICCID.
*   **Network Recovery:** Improved FTP backlog retrieval logic; if a 15-min report fails, it is cached and automatically sent on the next successful connection.

### 📉 Optimization
*   **Memory Footprint Reduction:** Webserver functionality disabled by default, saving **~87KB** of program storage (now ~89% total usage).
*   **UI Cleanup:** "ENABLE WIFI" menu item is now dynamically hidden when the WebServer is disabled to prevent user confusion.

### 🐞 Bug Fixes
*   **SSL Handshake Failure (715):** Resolved by utilizing correct active network CID instead of forcing CID 1.
*   **SD Card Handling:** SD card detection refined to be an optional diagnostic field rather than a blocking critical error.
*   **Daily Reset Logic:** Network performance metrics are now freshly reset at the 8:45 AM daily transition.

---
**Verified on Unit:** WS1923
**Carrier Verified:** Airtel (IoT / airteliot.com)
