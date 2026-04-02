# 🏆 AIO Firmware v5.75 "Diamond-Elite" - Release Notes
**Date:** April 2, 2026  
**Status:** **GOLDEN PRODUCTION MASTER** (v5.75_v4 Hardened)

---

## 💎 Release Overview: "Diamond-Elite"
Firmware v5.75 "Diamond-Elite" is the final, surgical hardening of the Golden v5.75 milestone. It achieves perfect data parity across all transmission paths (HTTP/FTP) and resolves critical registration timing issues for congested BSNL towers. This version is the master-stable baseline for all AIO9 hardware (TRG/TWS/TWS-RF).

---

## 🛠️ Diamond-Elite Hardening (Final v4 Pass)

### 1. [C-01] BSNL "Slot Window" Hardening
*   **20-Retry Registration Cap:** Optimized the registration loop to a 20-retry hard cap (~120s total). This ensures the modem successfully registers and transmits within its 10-minute slot window even on the slowest 2G towers.
*   **Tier 4 Recovery at 19:** Shifted the Tier 4 recovery (modem auto-mode reset) to the 19th iteration to provide a final "Sure-Shot" probe before the transmission window closes.

### 2. [FTP-03] SYSTEM 1 & 2 Record Unification (TWS/TWSRF)
*   **Strict Record Lengths:** Unified all CSV formatting to match current-slot specifications:
    *   **SYSTEM 1 (TWS):** Locked to **53 characters** per record.
    *   **SYSTEM 2 (TWS-RF):** Locked to **60 characters** per record.
*   **Server-Side Parity:** FTP backlog records now use semicolon (`;`) delimiters and unified column ordering to ensure 100% processing success on Spatika/KSNDMC dashboards.
*   **Battery Precision:** Enforced **%05.2f** (e.g., `03.99`) battery formatting across all 10+ transmission call sites to prevent seek-address drift.

### 3. [M-04] Smart Diagnostic Gating
*   **Stuck Sensor Guard:** Upgraded the 15-minute diagnostic logic. If a sensor is absent (reading strictly `0.0`), the system now ignores the "Stuck Value" increment. This prevents false "Temperature Stuck" fault reports for Rain Gauges or standalone units.

### 4. [H-04] Unconditional Daily Time Resync
*   **Rollover Fix:** Moved the `rtc_daily_sync_done = false` reset to the unconditional day-rollover block. This ensures the device always attempts a fresh server-clock sync every 24 hours, even if it reboots mid-day or is powered on for the first time in a week.

### 5. [M-03] APN Memory Safety & Stability
*   **Buffer Expansion:** Expanded `apn_str` and `carrier` buffers to **32 characters** to prevent memory corruption when using long enterprise M2M APN strings.
*   **[C-03] Server Index Guard:** Implemented boundary safety checks for the `httpSet` array to prevent out-of-bounds access during station initialization.

---

## 🛡️ Technical Improvements (v5.75 Baseline)

*   **[FTP-01] BSNL Context Breather:** Added a mandatory 3-second delay between HTTP completion and FTP start.
*   **[FTP-02] Smart Mode Switching:** Automatically defaults to **Active Mode (0)** for BSNL and **Passive Mode (1)** for Airtel/Jio 4G.
*   **[H-04] "Ghost Sensor" Fix:** Strictly enforces **0.0** temperature/humidity if the sensor (HDC/BME) is missing (NA on UI).
*   **[FS] fsMutex Hardening:** Increased `fsMutex` timeouts to 15s for large file operations to prevent WDT triggers during SPIFFS purges.

---

## 🚀 Deployment Verification
1.  **LCD Check:** Verify Unit ID and Version `5.75` on boot. Ensure no `!` error tag is present in healthy conditions.
2.  **Backlog Test:** Monitor logs for `[FTP] Backlog Push: X records found`. Verify that TWS records match the 53ndndndndndndndnd-byte alignment.
3.  **Sensor Test:** (Optional) Verify that uninstalled sensors do NOT trigger a "Stuck Value" health fault after 4ndndndndndndnd hours of operation.

**AIO v5.75_v4 is the finalized "Diamond-Elite" binary for mass production.**
