# ESP32 AIO Release Notes Summary

## 🚀 Latest Version: v5.69 (March 26, 2026 - Current)
**"The Concurrency & Atomic Stability Release"**

This version (v5.69) serves as a critical stability patch following the major v5.68 hardening phase. It specifically resolves deep-seated race conditions between the Modem (Core 0) and the Sensor Scheduler (Core 1) via a strict Mutex Hierarchy, while introducing hardware-level I2C bus recovery and atomic timekeeping snapshots.

### Key Highlights:
- **ABBA Deadlock Eradication**: Strict global acquisition sequence (`modemMutex -> fsMutex -> i2cMutex`) preventing task freezes.
- **Atomic Time Precision**: Port-level spinlock (`rtcTimeMux`) for all master time variables, ensuring 00:00:00 midnight record consistency.
- **I2C Self-Healing**: New bit-banging hardware recovery for stuck bus conditions caused by electrical noise.
- **Boot-Wipe Protect**: Unified SPIFFS cleanup/protection during initial startup.

[View Full v5.69 Release Notes](./RELEASE_NOTES_v5.69.md)

---

## 🕒 Previous Versions

### [v5.68 - The Full-Spectrum Hardening Release](./RELEASE_NOTES_v5.68.md) (March 24, 2026)
- **FastAPI / FastAPI Server Hardening**: Resolved CSRF vulnerabilities and RAM overflow guards.
- **Firmware Memory Leaks**: Converted GPRS/FTP buffers from stack to static/heap.
- **Energy Efficiency**: Eliminated 4mA battery leaks during deep sleep by tearing down SPI/SD buses.

### [v5.66 - The Architect's Hardening & Dashboard Truth Release](./RELEASE_NOTES_v5.66.md) (March 23, 2026)

### [v5.63 - The Airtel IoT & Power Stability Release](./RELEASE_NOTES_v5.63.md) (March 18, 2026)

### Key Highlights:
- **Hybrid HTTP Engine**: Combined v3.0 speed with v5.6x self-healing (Zombie Nuke).
- **UART Sync (Fast Flush)**: Eliminates buffer poisoning during backlog clearing.
- **Power-Safe Backlog**: Maximum 15 records per wakeup and Fail-Fast on the first error.
- **Global Trim**: Fully sanitized filenames for TWS/TRG/ADDON.

[View Full v5.63 Release Notes](./RELEASE_NOTES_v5.63.md)

---

## 🕒 Previous Versions

### [v5.60 - Mathematical Consistency Update](./RELEASE_NOTES_v5.60.md) (March 18, 2026)
- **ID Normalization**: Standardized Station ID handling across FTP and SD naming.
- **Airtel Zombie Fix**: CIPSHUT reset for 706 TCP errors.
- **I2C Isolation**: Improved stability under heavy electrical noise.

### [v5.52 - Stability & Precision Update](./RELEASE_NOTES_v5.52.md) (March 12, 2026)

### [v5.50 - Timing & Diagnostics](./RELEASE_NOTES_v5.50.md) (March 11, 2026)
- Retrospective slot recording.
- Detailed registration failure reasons (Rsn).
- Calibration status visibility.

### [v5.2 - The Resilient Engine](./RELEASE_NOTES_v5.3.md) (Feb 14, 2026)
- Bimodal power discipline (Morning/Evening health reports).
- BSNL-safe GPRS retries.
- Instant-sleep logic.

---
*Created by Antigravity AI for Spatika Information Technologies Pvt Ltd.*
