# Release Notes - v5.73 (Platinum Edition)

**Release Date:** 2026-03-30  
**Status:** Production Stable (Golden Revert)  
**Target:** TWS/TRG Systems with BSNL 2G or Hybrid GPRS

---

## 🚀 Overview
Version 5.73 represents the **"Golden Baseline Revert"** for FTP connectivity while maintaining the advanced Platinum hardening features. This update specifically resolves the persistent `ERROR` responses seen on BSNL 2G towers by restoring proven transmission modes.

---

## 🛰️ GPRS & FTP Hardening (The "Golden Fix")
- **[Restore] BSNL Active Mode**: Reverted default BSNL FTP mode to **Active (0)** matching the proven v5.70 behavior.
- **[Fix] Initialization Cleanup**: Simplified the `setup_ftp` handshake. Removed redundant `CFTPSCFG` security/cid commands that were returning `ERROR` on current modem firmware.
- **[Enhancement] DNS Resilience**: Increased DNS warmup timeout to **25 seconds** and Login timeout to **60 seconds** to survive peak-hour 2G congestion.
- **[Fix] Binary Mode Baseline**: Enforced Binary mode (`1`) for all `PUTFILE` operations to ensure data integrity.
- **[Optimization] Ghost PDP Recovery**: Implemented detection for "Ghost IPs" (`0.0.0.0`). The system now proactively nukes and rebuilds the bearer if the modem fails to acquire a valid IP.

---

## 💾 Data Integrity & Backlog Logic
- **[Atomic] Dedup Guard**: Implemented `last_unsent_sampleNo` tracking. This prevents the system from accidentally double-writing records to the backlog during fresh boots or interval rollovers.
- **[Flash] Wear Reduction**: Added "Smart Save" for `lastrecorded` files. The system now reads the flash first and **skips the write** if the data is identical, significantly extending SPIFFS lifespan.
- **[Mutex] Deadlock Shield**: Strictly enforced the `modemMutex -> fsMutex` hierarchy across all tasks to eliminate 15-minute "modem hangs" caused by core-to-core contention.

---

## 🛠️ System Stability
- **[WDT] Watchdog Hardening**: Added explicit `esp_task_wdt_reset()` calls in long-running loops (Gap-fills, HTTP header searches, and large backlog pushes).
- **[Recovery] TCP Zombie Nuke**: Implemented detection for HTTP/TCP "Zombie" states (Error 706/714). The system now executes a hard context nuke to recover the stack without a full system reboot.
- **[Optimization] Handshake Caching**: Successfully implemented carrier and DNS caching, reducing successful cycle times from **120s down to ~75s** in steady-state operation.

---

## 📋 Critical Verification Log (13:15 Slot)
```text
[FTP] Using Active Mode (transmode=0)
+CDNSGIP: 1,"ftp1.ksndmc.net","27.34.245.70"
+CFTPSLOGIN: 0
+CFTPSPUTFILE: 0 (SUCCESS ✅)
```

---
**Build Notes:**  
- Compiled with: `ESP32 Dev Module`, `8MB Flash`, `Core 0/1 Scheduler`.
- Verified on: `A7672S` Modem (BSNL 2G and Airtel 4G carriers).
