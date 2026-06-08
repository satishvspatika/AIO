# Release Notes: v6.07 (Jun 08, 2026)

## 🎯 Overview
Release v6.07 brings significant enhancements to system diagnostics, network registration reliability, UI stability, and power management compared to the v5.90 baseline. Key upgrades include full Nuvoton MG51 UART LCD UI integration, a detailed Field Diagnostics Dashboard in the Web UI, rotation of active system alarms, and core scheduler hardening to prevent file descriptor leaks and sleep timing issues.

---

## ✨ Enhancements & New Features (Compared to v5.90)

### 1. **Nuvoton UART LCD & Keypad Support** 📺
* **Compile-Time Master Switch:** Added `#define USE_NUVOTON_UI` (default `1`) to dynamically choose between the legacy I2C UI and the new UART-based Nuvoton MG51 UI.
* **Hardware Bug Bypasses:**
  * **Sacrificial Write:** Transmits cursor address twice with 10ms delay to wake up the idle MG51 UART interface without dropping data.
  * **0x80 Cursor Bug Bypass:** Bypasses MG51's firmware bug where it ignores standard row 0 cursor reset commands. The screen is now cleared using `0x01` and redrawn dynamically.
  * **Blink/Heartbeat Suppression:** Disabled the legacy 1-second blinking dot heartbeat to decrease UART traffic and noise-induced screen wrapping.
* **Flicker-Free Edit Mode:** Implemented differential caching in Edit Mode (`eEditOn`) so that only modifying values are redrawn instead of full screen refreshes.
* **Full Cursor Control:** Added support for cursor blinking and visibility control commands (`blink()`, `noBlink()`, `cursor()`, `noCursor()`).

### 2. **System Stage Tracking & Status Updates** 🔄
* Added global `set_sys_status()` to monitor active hardware tasks.
* Real-time status now transitions dynamically through: `IDLE`, `WIFI ACTIVE`, `FTP UPLOAD`, `SENDING SMS`, `GETTING GPS`, `SENDING HEALTH`, `CHECKING SMS`, `SENDING HTTP`, `HTTP BKLOG(...)`, and `RESYNC TIME`.

### 3. **Field Diagnostics Dashboard (Web Server UI)** 🌐
* Restored and enabled local Web Server by default (`ENABLE_WEBSERVER 1`).
* **Interactive Diagnostics Card:** Added detailed diagnostics panel visible over local AP, featuring:
  * Active alarms list (e.g. Low Battery, Solar Low, RTC Fault, SD Error, HTTP Fails, Sensor Range Errors).
  * Network registration RSSI and Carrier/APN details.
  * SIM Card ICCID readout.
  * Real-time voltage diagnostics for both GPRS and MCU 3.3V power rails.
  * Reset reason readouts (POWERON, SOFTWARE, WATCHDOG, DEEPSLEEP).
  * Counts of unsent HTTP and FTP backlog files.
* LCD Screen `FLD_SYS_STATUS` now automatically rotates multiple active system alarms every 3 seconds statically on the top row, avoiding marquee scrolling that causes Nuvoton LCD flickering.

### 4. **Power & Wakeup Policy Tuning** ⚡
* **Deep Sleep Keep-Alive:** Modified sleep sequence so that when `USE_NUVOTON_UI` is active, the Nuvoton module remains powered (`GPIO32` held `HIGH`) during deep sleep. This allows it to monitor keypresses and wake the ESP32 via GPIO27 (`ext0` interrupt).
* **LCD Blanking:** Replaced physical power-cut logic with standard LCD backlight control (`noBacklight()`) when screen timeout occurs.

---

## 🔧 Bug Fixes & Hardening

* **File Descriptor Leak Fix (Scheduler):** Resolved a critical path in `scheduler.ino` where entering file creation logic outside the valid window jumped to `TRIGGER_HTTP` without closing `file1`/`sd1` or clearing the `fsMutex` semaphore, preventing eventual SPIFFS corruption and system locks.
* **Unprocessed-Slot Sleep Guard:** Added check in `loop()` to delay deep sleep if the RTC advances to a new unsaved slot (sample index) during boot or processing, ensuring no data slots are missed.
* **Rain Delta Spike Prevention:** Reset both `rf_count.val` and `last_raw_rf_count` to `0` during the 15-minute rain loop reset, eliminating negative delta calculations and ignored tips.
* **Modem Initialization Settle Delay:** Increased startup delay to 3 seconds inside `start_gprs()` to allow the SIM module reader to settle and decrease verbose `+CME` errors on boot.
* **Unsent Log File Detail:** Upgraded debug console logs to read and print the total number of records within unsent HTTP/FTP backlog queues.

---

## 📋 Technical Details

### Modified Files
* **`AIO9_5.0.ino`** - Sleep guard logic, UI fields definition, and rain resets.
* **`user_config.h`** - Set version to `6.07` and default web server to `1`.
* **`globals.h`** - UI structures, backlight commands declarations.
* **`lcdkeypad.ino`** - Nuvoton LCD methods, dynamic error rotations, and wifi status updates.
* **`scheduler.ino`** - File leak resolution, unsent log counts, and rain resets.
* **`webServer.ino`** - System diagnostics Web UI components and JSON responses.
* **`global_functions.ino`** - Deep sleep GPIO32 hold parameters for Nuvoton UI.
* **`gprs_core.ino`**, **`gprs_ftp.ino`**, **`gprs_http.ino`**, **`gprs_health.ino`**, **`gprs_helpers.ino`** - GPRS/modem tracking states.
* **`rtcRead.ino`** - Resync time logging status tracking.

---

## 📦 Release Configuration & Verification

* Target Board: ESP32 NodeMCU / Custom Spatika AIO Board
* Partition Scheme: 8MB Flash (with FAT/SPIFFS support)
* Recommended Verification:
  1. Boot device and verify "SYS STATUS" LCD carousel handles alerts.
  2. Start WiFi AP, connect, and verify Diagnostics Dashboard displays active fields.
  3. Validate ext0 deep-sleep wakeup via Nuvoton keypad.
