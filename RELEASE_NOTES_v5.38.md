# AIO Firmware v5.38 Release Notes

## 🚀 Enhancements & Features
- **Proactive WebServer Prompts**: Added a "Tasks Complete - GPRS done. Sleep now?" modal to the local Web UI to proactively inform the user when backend GPRS tasks are finished, so they don't have to keep the web server alive unnecessarily.
- **Enhanced LCD Feedback**: Added explicit "WIFI DISABLED" and "ENTERING SLEEP.." feedback messages on the LCD screen when a user manually clicks the “Close Now” / Disconnect button via the Web UI.
- **Station ID File Branding**: Downloaded log text files via the internal Web Server now explicitly prepend the `Station_ID` (e.g. `1936_20260223.txt`) to the filename so downloaded data isn't easily lost or mixed up.
- **Mathematical Gap-Fill Interpolation (Bresenham Distributions)**: Upgraded missing rainfall interpolation calculations to distribute hardware ULP bucket tips across offline gaps cleanly and precisely. Removes "flatlines" or "float" rounding errors, guaranteeing that gap Instantaneous Rainfall always arithmetically sums to the recovered Cumulative Rainfall total flawlessly on wakeup.

## 🐛 Bug Fixes
- **Ghost Characters on LCD**: Resolved a UI glitch where ghost characters (e.g., the ".38" from "5.38" firmware strings) would occasionally linger on the 16x2 LCD screen when navigating between different display fields. This was fixed by rigorously overwriting lines with explicit space padding before drawing new text.
- **RF Accumulation Wiped on EXT0 Wakeup**: Fixed an issue where waking the device via EXT0 (Push Button) could inadvertently wipe the accumulated RF measurements (`rf_count.val = 0`) if the SPIFFS `firmware.doc` file had a slightly delayed/empty read during `setup()`. Added safeguards so memory is only wiped on a legitimately confirmed firmware string mismatch.
- **Watchdog Panic after WebServer Shutdown**: Resolved a severe crashing bug that resulted in a Task Watchdog Panic roughly 30 seconds after manually turning off the WebServer interface. The `webServerTask` had been deleting itself without properly unsubscribing from the hardware watchdog (`esp_task_wdt_delete`), leaving the OS waiting indefinitely for the dead task to "pet the dog".
- **Unintended Wi-Fi Start on EXT0 Wakeup**: Resolved a logic error where waking the device physically via the EXT0 button forcefully triggered the Web Server in the background before the user authorized it. The Web Server now strictly respects being launched solely from the `"ENABLE WIFI"` interaction prompt via the LCD.
- **Mobile Browser WebServer Timeouts**: Greatly reduced `vTaskDelay` blocks (from 100 Ticks to 10 Ticks) in the FreeRTOS `handleClient` queue to prevent aggressive parallel connections on mobile devices/phones from timing out and rendering blank white screens.
- **Hung Web UI on Physical Disconnect**: A localized Web UI DOM overlay is now successfully injected if a browser is actively surveying the telemetry, but the unit operator unexpectedly hits 'DISABLE WIFI' from the LCD. The screen now renders an elegant "WiFi Disabled" placeholder instead of spinning into an infinite routing redirect.

## 📋 Modified Files
- `AIO9_5.0.ino` (Firmware bounds, Ext0 wifi fix)
- `lcdkeypad.ino` (Ghost character UI cleanup, Wi-Fi interaction routing)
- `webServer.ino` (Mobile latency delay, GPRS prompt logic, LCD feedback, Watchdog panic fix, offline DOM injection, Server Download headers)
- `scheduler.ino` (Bresenham Tipping Bucket gap distribution array logic)
- `globals.h` (Version bump strictly if required, handled in final script)
