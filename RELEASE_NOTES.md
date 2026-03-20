# ESP32 AIO Release Notes Summary

## 🚀 Latest Version: v5.65 (March 20, 2026 - Final)
**"The Health Reliability & UART Protocol Release"**

This release resolves "Health Report Failed" issues by implementing a multi-attempt retry loop and fixing trailing byte poisoning in the modem's HTTP stack.

[View Full v5.65 Release Notes](./RELEASE_NOTES_v5.65.md)

---

## 🕒 Previous Versions

### [v5.64 - The Data Integrity & Mask Synchronization Release](./RELEASE_NOTES_v5.64.md) (March 20, 2026)
This release resolves dashboard counter inconsistencies by ensuring perfect bitmask synchronization during meteorological rollovers and system reconstructions.

### [v5.63 - The Airtel IoT & Power Stability Release](./RELEASE_NOTES_v5.63.md) (March 18, 2026)
This release completes the GPRS/HTTP stabilization for Airtel IoT and BSNL. It integrates the v3.0 fast-push engine with v5.60's robust error recovery and adds advanced battery protection for backlogs.

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
