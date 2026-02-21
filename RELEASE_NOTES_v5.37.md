# Release Notes: v5.37 (Feb 21, 2026 - Production Build)

## 🎯 Release Overview
A landmark stability update introducing "Self-Healing Network Recovery" and verified "Hardware Disruption Resilience". This version addresses critical field issues like GPRS "Zombie Sessions" (HTTP 713/714) and ensures 100% data continuity during manual maintenance or SIM swaps.

---

## ✨ Key Enhancements

### 1. **Self-Healing Network Architecture** 🤖
- **Hardware Power Isolation**: Implemented `gpio_hold_en` on GPIO 26 to guarantee 100% physically isolated power cuts to the modem during Deep Sleep. This eliminates "Zombie Sessions" where modems stay half-alive and block future transmissions.
- **Proactive Failure Learning**: The system now tracks `diag_consecutive_http_fails` in RTC memory. It "learns" from previous cycle hangs and proactively triggers a deep IP stack cleanup (`CIPSHUT` + `CGACT=0`) before the first attempt.
- **Autonomous DNS Stabilization**: If DNS resolution fails due to rural tower issues, the system now automatically forces Google DNS (8.8.8.8), bypassing unreliable local carrier stacks.

### 2. **Field-Locking Security (UI Hardening)** 🔒
- **Problem**: Accidental keypad presses could put critical status fields into "Edit Mode," corrupting the display.
- **Solution**: Implemented a **Read-Only Secure Lock** on all sensor data (Temp, Hum, Wind, RF) and diagnostic fields (Signal, Battery, Registration). 
- **Impact**: Keys are only responsive for configuration (`STATION`, `DATE`, `TIME`) and action triggers (GPS, WIFI), making the UI tamper-proof for field duty.

### 3. **Intelligent System Diagnostics** 🧠
- **Dynamic Fault Detection**: The diagnostic summary now intelligently detects the `SYSTEM` type. 
    - **TRG (SYSTEM 0)**: No longer triggers false sensor alarms for missing Wind/TH sensors.
    - **Reduced Payload**: Redesigned status strings to be system-specific (e.g., TRG shows strictly `RF:OK`).
- **Data Decoupling**: Background sensor polling is now isolated from production buffers, ensuring zero "snapshot contamination" during high-speed scheduler transfers.

---

## ✅ Field Validation Results (Verified 2026-02-21)

This build has been stress-tested for real-world field scenarios:

- **SIM Swap Resilience**: Verified that system intelligently detects SIM removal, queues data locally in SPIFFS, and **automatically clears the backlog** via FTP the moment the SIM is restored.
- **Power-Cycle Gap Filling**: Switch-off tests confirmed that if a station is powered down during a logging slot, it will **accurately fill the gap** on the next boot, ensuring 100% server data continuity.
- **Modem Recovery**: Confirmed recovery from HTTP/706/714 errors through autonomous retries and hardware-level resets.

---

## 🔧 Technical Summary

### Modified Files:
- `globals.h`: Persistent failure tracking and hardware control.
- `gprs.ino`: Proactive learning and adaptive shutdown logic.
- `scheduler.ino`: Production data snapshotting and standardized logging.
- `lcdkeypad.ino`: Field-Locking UI logic.
- `global_functions.ino`: Modem power isolation via GPIO hold.

### Code Size (v5.37):
- **Program Storage**: 1.10 MB (approx 84% usage)
- **Dynamic RAM**: 64 KB (approx 20% usage)

---

## 📦 Release Contents

Includes pre-compiled binaries for:
1. **KSNDMC_TRG**, **BIHAR_TRG**, **SPATIKA_TRG** (SYSTEM 0)
2. **KSNDMC_TWS** (SYSTEM 1)
3. **KSNDMC_ADDON**, **SPATIKA_ADDON** (SYSTEM 2)

---

**v5.37 is the most resilient firmware build to date!** 🚀
