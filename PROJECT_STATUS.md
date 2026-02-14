# ESP32 AIO v5.1 Project Status - Feb 13, 2026

## 🚀 Key Improvements & Fixes

### 1. Hardware Stability (ADC Fix)
- **Problem**: "ADC: CONFLICT!" crashes on ESP32 Core 3.x due to driver mismatch.
- **Solution**: Standardized all ADC reads (Battery, Solar, Wind) to the **Legacy ADC API** (`adc1_get_raw`). 
- **Status**: ✅ Verified stable; no more driver conflicts.

### 2. UI & Keypad Responsiveness
- **Flicker-Free Display**: Removed `lcd.clear()` from the update loop. Screens now refresh using smooth 16-character space padding.
- **Improved Input**: Station ID editing now supports **Alphanumeric** characters (`FLD_STATION` set to `eAlphaNum`).
- **Input Protection**: Background sensor updates are now blocked while `cur_mode == eEditOn`, preventing user edits from being overwritten.
- **Status**: ✅ UI is responsive and smooth.

### 3. Station ID Persistence
- **NVS Integration**: Station ID is now primarily stored in **Non-Volatile Storage (Preferences)**, making it independent of SPIFFS formatting.
- **Multi-Layer Fallback**:
  1. Load from **NVS**.
  2. If empty, load from **SPIFFS** (`/station.doc`).
  3. If still empty, load from **SD Card** (`/station.doc`).
  4. Fallback to `SIT999` only as a last resort.
- **Status**: ✅ Settings are robust and survive full flashes/wipes.

### 4. Data Reliability (Gap Filling)
- **Unsent Queue**: All system types (TRG, TWS, TWS-RF) now correctly use `unsent.txt` or `ftpunsent.txt` when GPRS is unavailable.
- **Intelligent Filling**: On boot, the system detects gaps in the daily SPIFFS/SD files and uses an interpolation mechanism to fill missing 15-minute slots.
- **Status**: ✅ Verified via serial logs.

### 5. Field Hardening & Longevity (v5.1)
- **I2C Bus Recovery**: Automated detection and recovery of hung SDA/SCL lines. If the bus hangs due to noise, the system now "clocks out" the hanging state automatically.
- **LCD Heartbeat**: A small blinking pixel at (15,0) provides instant visual confirmation that the firmware is running.
- **Memory Protection**: Optimized `String` handling in GPRS responses to prevent heap fragmentation during months of uptime.
- **Status**: ✅ Ready for long-term deployment.

### 6. Display Polish
- **Strict Padding**: Solved the "NA0.00" residual text bug by enforcing 16-character space-padding on every LCD update.
- **Synchronized Heartbeat**: Integrated the heartbeat into the main display loop to prevent it from being clobbered by labels.
- **Status**: ✅ Display is clean and professional.

## 🛠 Active Configuration
- **Unit**: `KSNDMC_ADDON` (TWS-RF)
- **System**: `2`
- **CPU Freq**: `80MHz` (Power Saving)
- **Baud Rate**: `115200`

---
*This document serves as the single source of truth for the v5.0 stabilization sprint.*
