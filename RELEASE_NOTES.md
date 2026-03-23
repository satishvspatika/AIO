# ESP32 AIO Release Notes Summary

## 🚀 Latest Version: v5.66 (March 23, 2026 - Final)
**"The Architect's Hardening & Dashboard Truth Release"**

This monumental release establishes a pristine baseline. It permanently fixes long-standing multi-core map corruption via definitive `extern` re-architecture, rebuilds the deployment Dashboard explicitly around the philosophy of ground-truth data metrics, and fortifies the GPS Map Engine against legacy missing-coordinate bugs.

### Key Highlights:
- **One Definition Rule (ODR)**: Complete `.ino` refactoring to destroy structural twin-variable memory poisoning.
- **Indestructible Web Map**: Node-healing backend that defaults to last-known-good geometric coordinates, plus robust Regex filtering.
- **Absolute Telemetry Accuracy**: Complete purge of backlog data from falsely inflating live HTTP performance metrics.
- **OTA Verification Matrix**: Stringent version conversions stopping infinite OTA loops (`>=` operators only).

[View Full v5.66 Release Notes](./RELEASE_NOTES_v5.66.md)

---

## 🕒 Previous Versions

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
