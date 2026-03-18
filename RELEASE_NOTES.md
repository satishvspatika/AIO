# ESP32 AIO Release Notes Summary

## 🚀 Latest Version: v5.60 (March 18, 2026)
**"The Mathematical Consistency Update"**

This release completes the standardization of Station ID handling to match the `v5.56` industrial design, ensuring perfect compatibility between internal storage and legacy external servers.

### Key Highlights:
- **Intelligent ID Normalization**: Strips leading `00` from 6-digit numeric IDs internally (`001931` -> `1931`).
- **External 6-Char Compliance**: Automatically expands 4-digit numeric IDs to 6 characters for all FTP filenames and data records.
- **Log Search Fallback**: LCD searches now automatically fallback to `00XXXX` if the normalized name is not found, ensuring support for historical logs.
- **Alphanumeric Immunity**: Preserves unique IDs like `WS0094` without modification.

[View Full v5.60 Release Notes](./RELEASE_NOTES_v5.60.md)

---

## 🕒 Previous Versions

### [v5.59 - Airtel & I2C Stabilization](./85b6389) (March 17, 2026)
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
