# Implementation Plan: Spatika Service Report & Photo Upload

## Overview
Transform the local **Spatika Web Portal** into a central maintenance dashboard. Field engineers can capture photos and detailed service logs, which are then proxied through the station's GPRS connection to the **Spatika Health Portal**.

---

## 1. Storage Strategy (Seamless SD/SPIFFS Fallback)
The system must handle the presence or absence of an SD card without engineer intervention.

- **Primary (SD Card)**: If detected, images are saved as `/service/report_ID_raw.jpg`. No strict size limit.
- **Secondary (SPIFFS Fallback)**: If SD is missing, images are saved as `/svc_min.jpg`.
- **Enforcement**: In SPIFFS mode, the local portal (Client JS) will automatically downscale images to **320x240 @ 0.5 Quality** (~10-15KB) to prevent filling the internal flash and risking a crash.
- **Safety**: A "Storage Full" check will precede every upload, with an option for the engineer to "Wipe old service logs" from the portal.

---

## 2. Connectivity Strategy: The "Post-Box" Proxy
Engineers often lack mobile data when connected to the station Wi-Fi.

1. **Local Upload (Phase 1)**: Engineer uploads photos/comments via Phone -> ESP32 (Wi-Fi).
2. **Persistence**: ESP32 saves the report to local storage (SD/SPIFFS).
3. **Background Sync (Phase 2)**: The ESP32 GPRS Task detects a pending report (`/service_pending.json`).
4. **Modem Transmission**: The A7672 modem uses `AT+HTTPDATA` to stream the binary image and JSON data directly to the Health Portal.

---

## 3. User Interface & Security
- **Role-Based Access Control (RBAC)**:
  - **Visitor Mode**: Current read-only metrics display.
  - **Engineer Mode**: Requires a **PIN/Password** (stored in `user_config.h` or `/wifi_pass.txt`). Unlocks the "Service Report" tab and "Calibration" controls.
- **Interactive Form**:
  - Image preview before upload.
  - Metrics snapshot (Battery, Solar, GPS, RF) captured at the moment of submission.
  - "Proven Presence": The report is signed with the station's current GPS lat/long and RTC timestamp.

---

## 4. Robustness & Data Integrity
- **Chunked Upload**: If GPRS signal is weak, the modem will attempt to upload the JSON metrics first, then the photos.
- **Binary Reliability**: Images will be sent as raw binary (to save 33% overhead vs Base64) using multi-part form data headers.
- **Compression**: The Spatika Web Portal will use a Canvas-based resizer in the browser to ensure no large files ever hit the ESP32 UART buffer.

---

## 5. Technical Components
### Firmware
- `service_report.ino`: New module for handling `/upload_report` POST requests and storage management.
- `gprs_http.ino`: Enhancement to support `multipart/form-data` binary uploads.

### Web Portal (Captive)
- `service.html`: A high-performance, responsive form using Vanilla JS for image resizing and validation.

### Health Portal (Server)
- `/api/service-report`: New Python/FastAPI endpoint for receiving reports.
- `app/models.py`: New `ServiceReport` table to store metadata and links to S3/Local storage for photos.

---

## 6. Verification Plan
1. **Flash Check**: Verify SPIFFS doesn't overflow when uploading 2 mini-photos without an SD card.
2. **Modem Stability**: Stress test `AT+HTTPDATA` with 50KB binary payloads.
3. **Role Verification**: Ensure "Visitor" mode cannot access the `/upload_report` endpoint.
