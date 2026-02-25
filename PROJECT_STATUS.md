# ESP32 AIO v5.51 Project Status - Feb 25, 2026

## 🚀 Key Improvements & Fixes (v5.51 - Sensor & Reporting Optimization)

### 1. Pressure Sensor Gating
- **Status**: ✅ Implemented in v5.51.
- **Logic**: Atmospheric pressure and altitude fields are now **disabled completely** unless the configuration is specifically `UNIT = KSNDMC_TWS-AP` and `SYSTEM == 1`.
- **Benefit**: Removes unwanted pressure data from the LCD and records on standard `SPATIKA_GEN` (EMPRII) or basic `TWS` units.

### 2. Health Report Control
- **Status**: ✅ Implemented. Added `ENABLE_HEALTH_REPORT` global flag.
- **Benefit**: Allows complete disabling of automated health reports to save power and bandwidth if the server-side dashboard is not in use.

### 3. Accelerated FTP Backlog Retries (2-Hourly)
- **Status**: ✅ Implemented for `SYSTEM 1` (TWS) and `SYSTEM 2` (TWS-RF/ADDON).
- **Benefit**: Reduced the unsent data retry interval from 3 hours to **2 hours**. This ensures faster recovery of "Golden Data" after network outages while still avoiding session overlaps.

### 4. Optimized FTP Reliability (v5.50 Legacy)
- **Status**: ✅ Active.
- **Benefit**: Reduced timeouts (Login 45s, PUT 60s) and signal strength gating to protect battery life on weak BSNL links.

---

## 🛠 Active Technical Specs
- **Unit**: `TWS9-GEN-5.51` (Configured as `SPATIKA_GEN` / `SYSTEM 2`)
- **Status**: All non-essential telemetry (Pressure/Health) disabled to minimize footprints.

---
*Last Updated: 2026-02-25 | v5.51 Precision Build*
