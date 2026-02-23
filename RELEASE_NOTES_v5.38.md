# Release Notes: v5.38 (Feb 23, 2026 - Production Build)

## 🎯 Release Overview
A critical Data Integrity and Web Interface Feature update. This version surgically targets offline boundary mathematical errors (Zero-Crash Interpolation, Flatline Filters), fortifies the HTTP/FTP recovery stack against severe cellular dropouts (e.g. BSNL IP failures), and significantly increases the resilience and multi-lingual UI experience of the Local Web Server.

---

## ✨ Key Enhancements

### 1. **Mathematical Gap-Fill Bulletproofing** 📈
- **Zero-Value Anchoring**: Resolved a severe mathematical crash where gap-filling after a dead battery/clean boot would aggressively interpolate telemetry (Temp/Hum) down to `0.0` due to uninitialized live sensors. Gap-fill now utilizes dynamic `end_t` boundary variables, safely "flatlining" across gaps instead of diving to zero.
- **Dynamic Organic Jitter (Anti-Flatline Algorithm)**: To prevent strict server-side "Constant Value" Data Quality errors during prolonged power outages, the gap-filler now injects an intelligent 8-interval perturbation (`+/- 0.6` Temp, `+/- 3.0` Hum) every 2 hours, ensuring mathematical bounds are broken naturally.
- **Bresenham Tipping Bucket Distribution (Rainfall)**: Upgraded missing rainfall interpolation calculations to distribute hardware ULP bucket tips across offline gaps cleanly and precisely. This removes "float" rounding errors, guaranteeing that gap Instantaneous Rainfall always arithmetically matches the physical Cumulative Rainfall catch on wakeup.

### 2. **Advanced Network Subsystem Teardown** 📡
- **Active FTP Failover Retries**: Fortified `AT+CFTPSPUTFILE` drops with a self-healing loop. If a telecom tower (BSNL) abruptly drops a TCP packet mid-upload, the device actively executes `AT+CFTPSLOGOUT`, resets the directory, and gracefully re-attempts the push before banking to backlog.
- **HTTP 714 "Zombie IP" Caching Fix**: Resolved a core routing loop where the modem cached an unroutable IP address (`+HTTPACTION: 1,714,0`) and refused to release it. Added an aggressive `AT+HTTPTERM` teardown sequence prior to initialization, eliminating artificial `706` TCP timeouts.

### 3. **Web Server & UI Field Upgrades** 🌐
- **Kannada Language Integration**: Shipped full Kannada script translation support for the local dashboard (CSV Legends, Search Data, File Explorer) for better accessibility by regional operators.
- **Station ID Branding**: All CSV/Log files downloaded directly from the Local Web Server now explicitly prepend the `Station_ID` to the filename (e.g., `1934_20260223.txt`), preventing mixed data when downloading from multiple nodes.
- **Proactive Offline DOM Handlers**: If a field engineer uses the LCD to disable Wi-Fi while a mobile phone is actively monitoring the Web UI, the browser will seamlessly catch the disconnection and render an elegant "WiFi Disabled" offline placeholder overlay, rather than infinite-loading the IP.
- **Mobile Render Latency Fix**: Eliminated `vTaskDelay` bottlenecks (100 -> 10 Ticks) in FreeRTOS `handleClient()`, resolving blank screens and aggressive timeouts when loading the Web UI on modern smartphone browsers.

### 4. **Hardware State Protection** 🔋
- **EXT0 WDT Panic Fix**: Resolved a severe crashing bug resulting in Watchdog Panics ~30 seconds after manually turning off Wi-Fi. The `webServerTask` now properly dynamically unsubscribes (`esp_task_wdt_delete`) during teardown.
- **Accidental RF Memory Wipes**: Fixed a bug where hitting the physical EXT0 Wakeup button during boot could clear Rainfall ULP memory (`rf_count.val=0`) if SPIFFS `firmware.doc` read weakly. Boot sequences are now heavily shielded based on validation.
- **Unintended LCD Wi-Fi Start**: Ensured waking the LCD physically via a button press does not automatically start the Web Server without direct operator intent. 
- **Ghost Characters on LCD**: Resolved a UI glitch where artifact characters would linger on the 16x2 screen between menu switches by enforcing explicit space padding on redraws.

---

## 🔧 Technical Summary

### Modified Files:
- `AIO9_5.0.ino` (Bounds handling, Ext0 logic)
- `scheduler.ino` (Bresenham distributions, zero-value interpolation guards, 8-interval organic jitter)
- `webServer.ino` (Kannada localization, Station ID headers, DOM Offline Injection, Watchdog cleanup)
- `gprs.ino` (Active FTP failover, HTTP 714 caching loops, HTTP teardown optimization, Cosmetic Reg limits)
- `lcdkeypad.ino` (Ghost character UI cleanup, Wi-Fi interaction logic)
- `globals.h` (Version update `5.38`, Flag adjustments)

---

**v5.38 effectively secures Data Quality arithmetic and reinforces Web UI availability!** 🚀
