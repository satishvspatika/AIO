# AIO Firmware v5.38 Release Notes

## 🚀 Enhancements & Features
- **Proactive WebServer Prompts**: Added a "Tasks Complete - GPRS done. Sleep now?" modal to the local Web UI to proactively inform the user when backend GPRS tasks are finished, so they don't have to keep the web server alive unnecessarily.
- **Enhanced LCD Feedback**: Added explicit "WIFI DISABLED" and "ENTERING SLEEP.." feedback messages on the LCD screen when a user manually clicks the “Close Now” / Disconnect button via the Web UI.

## 🐛 Bug Fixes
- **Ghost Characters on LCD**: Resolved a UI glitch where ghost characters (e.g., the ".38" from "5.38" firmware strings) would occasionally linger on the 16x2 LCD screen when navigating between different display fields. This was fixed by rigorously overwriting lines with explicit space padding before drawing new text.
- **RF Accumulation Wiped on EXT0 Wakeup**: Fixed an issue where waking the device via EXT0 (Push Button) could inadvertently wipe the accumulated RF measurements (`rf_count.val = 0`) if the SPIFFS `firmware.doc` file had a slightly delayed/empty read during `setup()`. Added safeguards so memory is only wiped on a legitimately confirmed firmware string mismatch.
- **Watchdog Panic after WebServer Shutdown**: Resolved a severe crashing bug that resulted in a Task Watchdog Panic roughly 30 seconds after manually turning off the WebServer interface. The `webServerTask` had been deleting itself without properly unsubscribing from the hardware watchdog (`esp_task_wdt_delete`), leaving the OS waiting indefinitely for the dead task to "pet the dog".

## 📋 Modified Files
- `AIO9_5.0.ino` (Firmware version check bounds)
- `lcdkeypad.ino` (Ghost character UI cleanup)
- `webServer.ino` (GPRS prompt logic, LCD feedback, Watchdog panic fix)
- `globals.h` (Version bump strictly if required, handled in final script)
