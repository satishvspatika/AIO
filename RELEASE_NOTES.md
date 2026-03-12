# ESP32 AIO Release Notes Summary

## 🚀 Latest Version: v5.52 (March 12, 2026)
**"Stability & Precision Update"**

This release focuses on industrial-grade reliability, introducing task-silencing during OTA to prevent crashes, fixing long-term resource leaks, and ensuring multi-century clock accuracy.

### Key Highlights:
- **Absolute Silence Protocol**: Tasks are paused during OTA to prevent SPIFFS collisions.
- **Resource Integrity**: Fixed BUG-5 memory leak in file system handling.
- **Precision Calendar**: Full Gregorian leap year rule implementation.
- **Standardized FTP**: Fixed chunk sizes (15) for robust backlog reporting.
- **Sanity Guards**: Protective caps for rainfall and backlog counters.

[View Full v5.52 Release Notes](./RELEASE_NOTES_v5.52.md)

---

## 🕒 Previous Versions

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
