# Release Notes v5.33

## Overview
This release focuses on optimizing GPRS connectivity for slow networks (like BSNL) and implementing a structured quad-slot health report schedule.

## Key Changes

### 📡 GPRS & Network Optimization
- **Eliminated CNUM/USSD Discovery**: Removed unreliable and time-consuming AT commands (`CNUM`/`USSD`) which wasted ~30s on IoT/BSNL SIMs.
- **IMSI-First Identification**: Switched to direct `AT+CIMI` for SIM identification, ensuring rapid startup.
- **Strict IP Verification**: Modified `try_activate_apn` to ensure a non-zero IP is assigned before proceeding.
- **Bearer Recovery**: Implemented `verify_bearer_or_recover()` to detect and fix stale PDP contexts before every upload.
- **Accelerated Handshake**: 
  - Reduced signal strength polling timeout (for no signal) to 15s.
  - Reduced registration polling interval to 5s.
  - Added forced network search (`AT+COPS=0`) if stuck in "Not Searching" state.
- **Signal Sanitization**: Clamped "No Signal" code `85` to sentinel `-111` for consistent cloud reporting.

### 🏥 Health Reporting
- **Quad-Slot Schedule**: Implemented fixed health reports at **01:00, 07:00, 13:00, and 19:00**.
- **No-Retry Discipline**: Reports are attempted only once per scheduled hour to prevent battery drain from repeated failures.
- **RTC Persistence**: Switched to hour-based tracking (`health_last_sent_hour`) in RTC RAM for reliable scheduling across deep sleep.

### 🧹 Code Cleanup & Stability
- Removed ~100 lines of dead code (USSD recovery, obsolete bimodal reset logic).
- Updated all version strings to `v5.33`.
- Fixed multiple compilation and prototype mismatch errors.

## Release Deployment
1. Set `DEBUG 0` in `globals.h` for production.
2. Build all configurations using `python3 build_all_configs.py`.
3. Flash via `quick_flash.sh`.
