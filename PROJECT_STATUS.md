# ESP32 AIO v5.2 Project Status - Feb 14, 2026

## 🚀 Key Improvements & Fixes (v5.2 - The Resilient Engine)

### 1. Bimodal Power Discipline
- **Status**: ✅ Implemented. Health reports restricted to 10:30 AM and 11:00 PM.
- **Benefit**: Reduces daily airtime by ~40-60 minutes, preserving solar battery during monsoon/cloudy periods.

### 2. GPRS Resilience (v7.4.1 Hardening)
- **High-Freq Polling**: 500ms intervals for faster Airtel latching.
- **BSNL Ready**: 60s signal timeout and 60 registration retries for slow towers.
- **Stabilization**: 1s delay after CREG ensures reliable APN activation.

### 3. Protocol & Sleep Fixes (v7.5)
- **Zero Airtime Leakage**: Fixed logic hang that kept device awake for 14 minutes. Device now sleeps immediately after data task.
- **Server Acceptance**: Added `AT+CIPSHUT` to kill half-open sessions, solving the "Rejected" status issue.

### 4. Lean Core Architecture
- **Command Removal**: Stripped OTA, REBOOT, and PING logic. 
- **Status**: Project is now a dedicated, foolproof reporting engine with reduced code complexity.

---

## 🚀 Version 5.1 Legacy Improvements (Feb 13, 2026)

### 1. Hardware Stability (ADC Fix)
- **Solution**: Standardized all ADC reads to `adc_get_raw`. Verified stable on Core 3.x.

### 2. UI & Keypad Responsiveness
- **Polish**: Flicker-free display refresh and Alphanumeric Station ID support.

### 3. Persistence
- **NVS Integration**: Station ID and settings stored in persistent preferences, independent of SPIFFS.

---

## 🛠 Active Technical Specs
- **Unit**: `SPATIKA_GEN`
- **System**: `2` (TWS-RF)
- **CPU Freq**: `80MHz`
- **Baud Rate**: `115200`
- **Resolution**: `0.5mm` (Active RF)

---
*Last Updated: 2026-02-14 | v5.2 Release Build*
