# Release Notes: AIO9 Firmware v5.70-Hardened (Gold Final)

*Compiled & Verified: March 2026*
*Target Hardware: ESP32 (8MB Flash), A7672S Modem*

## Overview
Version **5.70-Hardened (Gold Final)** represents the culmination of 7 rigorous code audit and validation cycles. This release fundamentally focuses on **Hardware Survival, Data Integrity, and Subsystem Reliability**. It surgically resolves **23 critical and high-severity bugs** spanning filesystem locks, modem flash wear, race conditions, edge-case UI data loss, and power bleed. 

The core objective of v5.70 is achieving absolute resilience during prolonged power cuts, poor cellular-coverage conditions, and field sensor fault recovery.

---

## I. Critical Stability & Deadlock Fixes

*   **[C-4 & R-2] Unbounded Backlog Exhaustion Resolved:** 
    *   **Prior State:** Local SPIFFS backlog files (`unsent.txt` / `ftpunsent.txt`) grew without boundary during sustained network failure, eventually crashing the entire filesystem. The implemented trimming logic copied the entire file instead of trimming it.
    *   **Fix:** `pruneFile()` was completely rewritten with a dynamic size limiter (`limit / 2`) mapped to a 300-record threshold (~13.5KB). A **15-second self-deadlock** triggered during HTTP transmission failures was resolved by passing explicit lock-inheritance (`alreadyLocked = true`) to the pruning function.
    
*   **[R-1] Power-Loss SPIFFS Recovery Mechanism:** 
    *   Filesystem operations are now fully atomic. If a unit loses external and battery power in the exact 10ms window of a file rotation, the system automatically detects missing files on next boot and reconstructs the data from a secure `/trim.tmp` shadow file.
    
*   **[N-4] Atomic Rename Verification:** 
    *   Sensor data buffers (e.g., `prevWindSpeed.txt`, `lastrecorded.txt`) now verify atomic rename operations with integrated retry logic instead of silently dropping data on mutex blocks.

## II. Modem & Telemetry Hardening

*   **[C-1 & N-12] Prevention of Permanent Cellular NVRAM Damage:** 
    *   **Prior State:** Code was blindly executing `AT+CGDCONT=1` context activations on every connection attempt, causing thousands of permanent writes per year to the modem’s non-volatile memory (NVRAM), slowly destroying the hardware.
    *   **Fix:** Read-Before-Write logic is now universal. The system queries `AT+CGDCONT?` and successfully bypasses the write if the active APN and protocol match. A ghost write to CID 6 was also deleted.

*   **[N-12 Update] Jio LTE-Only Network Hardening (`IPV4V6`):** 
    *   Added a compound protection guard. If a Jio SIM card is detected, the context automatically upgrades to `"IPV4V6"` on the very first boot to guarantee LTE-only network attachment in remote/modern setups.
    
*   **[H-4] Zero-IP Connectivity Trap Destroyed:** 
    *   A network configuration could theoretically return `OK` without issuing an IP address. The bearer check now mandates a valid packet data address using `AT+CGPADDR` before allowing data transmission loops to start.
    
*   **[N-6] Stale APN Transmission Overhead:** 
    *   Bearer recovery loops now locally cross-sync the global `apn_str` definition upon a successful `stored_apn` connection. This removes an 8-second time waste per cycle on unreliable networks.

*   **[C-5] HTTPTERM Zombie Sessions:** 
    *   Eliminated mid-action session freezes by re-sequencing the `HTTPTERM` loop logic and enforcing rigorous boundary flushes via `flushSerialSIT()`.

## III. FTP Optimizations (SYSTEM 1 / 2)

*   **[C-3] FTP URC Race Guard:** 
    *   Closed a severe timing race. The FTP system now successfully waits up to 60 seconds specifically parsing the `+CFTPSPUTFILE:` URC, ensuring the modem actually transmits the data payload instead of cutting off mid-stream.
    
*   **[N-8] 9-Second Phantom Battery Drain Fixed:** 
    *   When the FTP backlog queue was empty, the cleanup logic was still firing a useless 9-second `CFTPSLOGOUT` / `STOP` sequence. This has been gated behind an `ftp_started` state flag.

## IV. Sensor Processing & Hardware Safety

*   **[C-2] Temperature Anchor Drift Fixed:** 
    *   The fault-recovery `last_valid_temp` anchor is now mathematically isolated. Random noise bursts (-39.9 or spikes) are securely discarded by the limits check and **never** contaminate the historical fault anchor.
    
*   **[H-1] ADC2 / WiFi Interference:** 
    *   Bypassed solar/battery data corruption caused by the ESP32's internal conflicts between the WiFi driver and the secondary Analogue-to-Digital Converter.
    
*   **[H-2] GPIO ESD Bleed:** 
    *   Asserted `gpio_hold_en()` and `_dis()` on UART pins 16/17 to protect them from Electrostatic Discharge (ESD) and floating states during Deep Sleep.

## V. Edge-Case Logic & UI Reliability

*   **[N-14] RF Resolution LCD Modification Discard:** 
    *   Fixed a bug where Physical UI edits to the Rainfall Calibration (e.g., 0.50mm to 0.25mm) were blindly overwritten during non-volatile saves. User modification input is now heavily validated using `atof()` and saved flawlessly.
    
*   **[N-13] The "Day 1" Data Hole Fixed:** 
    *   Resolved a logic gap during pristine, factory-reset installations where the absolute first 15-minute slot sent local data but entirely skipped the backlog transmission queue. 
    
*   **[Medium] Server Remote Wipe Integrity:** 
    *   The server's remote `DELETE_DATA` footprint failed to preserve user-assigned `rf_res.txt` values. The server command is now mapped 1:1 with the local LCD factory-wipe rules.

## VI. Deferred / Acknowledged Items (Roadmap v6.x)

*   **SPIFFS Endurance:** Current daily logs and constant pruning will hit maximum Flash write cycles in ~3.5 years. A full migration to physical SD-Card persistence is planned for v6.x architecture.
*   **`data_writing_initiated` Atomics:** Deferred explicit FreeRTOS `<atomic>` macro wrapping due to native 32-bit hardware thread safety on the ESP32 platform.

---
**Status:** VALIDATED
**Build Recommendation:** CLEAR FOR ALL DEPLOYMENTS (KSNDMC / SPI / Bihar / TRG)
