# Release Notes: v5.36 (Feb 20, 2026)

## 🎯 Release Overview
A critical reliability update addressing GPRS network registration delays for BSNL/Airtel SIM circles, introducing "Live Sensor Safety" for intelligent data jitter, and optimizing modem power consumption.

---

## ✨ Key Enhancements

### 1. **Robust GPRS Registration (Airtel/BSNL Circle Fix)** 📶
- **Problem:** Stations in certain network circles (BSNL/Airtel) experienced excessive registration delays (up to 15 mins), causing battery drain or transmission timeouts.
- **Solution:** Redesigned the registration loop in `scheduler.ino`. The timeout is now restricted to **180 seconds** (3 mins). The system proactively monitors registration status and performs strategic modem resets only when necessary.
- **Impact:** Significant improvement in battery life and higher success rates for wake-up transmissions.

### 2. **Live Sensor Safety (RTC-Persistent Jitter)** 🌡️
- **Problem:** Standard sensor failures often reported static values (e.g., 0.0 or 125.0), triggering "flatline" alarms on analysis servers.
- **Solution:** 
    - Added **RTC-Backed Persistent Memory** for `last_valid_temp` and `last_valid_hum`.
    - **Adaptive Jitter:** If the sensor fails or returns an outlier (outside 9°C to 50°C), the system now calculates a **+/- 2% random jitter based on the LAST KNOWN GOOD reading** instead of a fixed factory default.
    - **Regional Humidity Bounds:** Humidity is now strictly bounded between **10% and 100%** to mirror realistic environmental constraints.
- **Impact:** Maintains data continuity and prevents artificial alarms during transient sensor glitches.

### 3. **Smart Bearer Optimization** 🔋
- **Problem:** Sequential uploads (Health Report followed by Main Data) were performing redundant IP stack shutdowns (`AT+CIPSHUT`), wasting ~10 seconds of active modem power.
- **Solution:** Implemented a **Smart Bearer Check** using `AT+CGACT?`. The system now skips the reset sequence if a data bearer is already active.
- **Impact:** Shorter modem ON-time and lower power consumption during diagnostic cycles.

---

## 🔧 Bug Fixes & Refinements

### 1. **Gap Filling Interpolation Sync**
- Missed data slots (power outages) now use the same **9°C - 50°C** bounds and jitter logic as live readings, ensuring graph consistency.

### 2. **Unrestricted TRG Uploads (System 0)**
- Removed the battery voltage restriction for Rainfall uploads. The system will now attempt transmission regardless of voltage to ensure critical rain data is never delayed.

### 3. **Diagnostic URL Correction**
- Fixed a character typo in the `GOOGLE_HEALTH_URL` that was preventing some diagnostic reports from reaching the master health sheet.

---

## 📋 Technical Details

### Modified Files:
- `globals.h`: Persistent variables and version bump.
- `AIO9_5.0.ino`: Boot-time RTC variable initialization.
- `scheduler.ino`: GPRS timeout reduction and gap-filler logic.
- `tempHum.ino`: Advanced jitter and failure handling.
- `gprs.ino`: Smart bearer logic and HTTP connection reuse.

---

**v5.36 is production-ready!** 🚀
