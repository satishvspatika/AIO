# Release Notes - v5.75 (Platinum)

**Release Date:** 2026-03-31  
**Status:** Production Stable (Race-Hardened)  
**Target:** TWS/TRG Systems (Power Integrity & Logic Hardening)

---

## 🚀 Overview
Version 5.75 provides critical logical hardening to eliminate a rare race condition between the background loop and the data scheduler that caused automated reboots at the 15-minute boundaries. It also fixes the field issue where the LCD would unexpectedly turn on mid-cycle after a race-recovery reset.

---

## 🛰️ Logic Hardening: Sleep Gate Shield
- **[Fix] 60s Boundary Mask**: Implemented a mandatory 60-second "Deep Sleep Block" at every 15-minute boundary (`XX:00`, `XX:15`, etc.). This ensures the scheduler has a guaranteed window to wake up and lock the system before the main loop logic attempts to enter deep sleep.
- **[Fix] Critical Race Resolution**: Resolved the issue where `sync_mode` (left at `eHttpStop` from a previous cycle) would trick the system into sleeping exactly when a new slot started.

---

## 🖥️ UI & Power Optimization
- **[Fix] LCD Race-Shield**: Added `race_recovery_reboot` tracking in RTC RAM. If the system reboots to recover from a task race, it now **suppresses the LCD 5V Relay**, keeping the unit stealthy and saving battery in the field.
- **[Optimization] Binary Data Integrity**: Hardened the `A7672S` binary stream interface by removing redundant newline terminators in `AT+FSWRITE` routines, preventing command-buffer corruption on Bihar stations.

---

## 📊 Summary of Improvements
1. **0% Unwanted LCD On-time** during automated race recoveries.
2. **Eliminated "Emergency Abort" restarts** at 15-minute boundaries via the 60s grace window.
3. **Rock-solid binary transfers** for larger backlog files on rural networks.

---
**Build Notes:**  
- Compiled with: `ESP32 Dev Module`, `8MB Flash`.
- Verified on: `A7672S` (Fixed 22:30 race condition observed in manual logs).
