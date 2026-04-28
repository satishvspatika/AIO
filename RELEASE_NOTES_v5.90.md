# Release Notes: v5.90

## Production Hardening & OTA Reliability Updates

### 1. OTA Stream Parsing & Silence Protocol (H-01 Fix)
- **Removed Unreliable `AT+HTTPHEAD`**: Replaced the legacy head request with a robust internal parser that extracts `Content-Length` natively from the `+HTTPACTION` response, eliminating silent OTA aborts.
- **"Total Silence" Implementation**: Added `AT+CEREG=0` and `AT+CGEV=0` inside the OTA download phase to suppress asynchronous URCs (LTE registration/PDP context events). This stops random modem noise from injecting into the UART stream and corrupting the binary payload.
- **Buffer Yield Optimization**: Replaced latency-inducing `vTaskDelay(1)` with non-blocking `taskYIELD()` inside the `fetchFromHttpAndUpdate` UART polling loop. This eliminates CPU-induced byte drops during high-speed 64KB firmware block streaming.

### 2. Mutex Integrity Restoration (M-01 Fix)
- **Deadlock/Race Condition Prevention**: Fixed 6 catastrophic mutex race conditions across `fetchFromHttpAndUpdate()` and `copyFromSPIFFSToFS()`. Unconditional `xSemaphoreGive(modemMutex)` calls on early-abort paths (such as Bad File Size, Update.begin failure, or malloc failure) are now properly guarded with `if (!alreadyLocked)`. This prevents the OTA logic from accidentally releasing the lock while a parent task is actively relying on it.

### 3. UI/LCD Cache Invalidation (M-NEW Fix)
- **Screen Refresh Anomaly Resolved**: Fixed a persistent UI presentation bug where the bottom row of the LCD would permanently blank out after displaying full-screen popup messages.
- **Cache Wipes**: Added explicit differential cache invalidation (`present_topRow[0]=0; present_bottomRow[0]=0;`) after all 5 menus that manually invoke `lcd.clear()` (SD Copy, Clear Backlog, RF Field Calib, File Log Searching, and WiFi Enable/Disable). The screen now reliably redraws the menu state.

### 4. Field Deployment & Serviceability
- **Multi-Unit SD Card Reusability**: Removed the destructive `SD.rename("firmware.bin", "firmware.done")` logic. `firmware.bin` now remains completely intact on the SD card after a successful flash.
- **Impact**: Technicians can now use a single SD card to sequentially update an entire fleet of stations in the field without needing a laptop to manually rename files between deployments. Infinite loop protection is safely handled by the `fw_version.txt` comparison check.
