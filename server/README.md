# Spatika Health Server v3.1 (AIO v5.80)

> **For AI Sessions**: Review this file first. It contains the full current state, architecture, and deployment instructions for the Contabo server.

---

## 🖥️ Server Details
- **Host**: Contabo VPS — `75.119.148.192`
- **Port**: `80` (maps to Docker container port `8000`)
- **Stack**: FastAPI + SQLite + Jinja2 + Tailwind CSS + Leaflet.js
- **Container**: `spatika-health` (managed by `docker-compose`)
- **Server Root**: `/opt/spatika-health/` on Contabo

---

## 🚀 Recent Changes (v3.1)
- **Split FTP Commands**: `FTP_NOW` is replaced by `FTP_BACKLOG` (syncs `unsent.txt`) and `FTP_DAILY` (upload specific `/dailyftp_YYYYMMDD.txt` file using a date prompt).
- **Fleet Summary Page**: New dashboard tab with group-level health stats and OTA conversion progress across the 6 firmware groups.
- **Help & Legend**: Dedicated guide page explaining every column, command, status code, and the OTA workflow.
- **UI Refinement**: Removed Signal column from main dashboard (still in detail page). Clarified Networking column as "Data Today / Yest".
- **Unique Stations Only**: Dashboard now strictly shows one row per station (the latest check-in).

---

## 📁 Local Source (This Folder)
All server-side source files are mirrored here for version control and easy AI-assisted editing.

```
server/
├── README.md                   ← YOU ARE HERE
├── requirements.txt            ← Python dependencies
├── deploy.sh                   ← ONE-SHOT deployment script to Contabo
├── seed_db.py                  ← Seeds the 6 firmware groups in the DB
└── app/
    ├── main.py                 ← Thin router-only entry point
    ├── models.py               ← SQLAlchemy DB models
    ├── database.py             ← DB connection (not stored here — on server)
    ├── routers/
    │   ├── health.py           ← POST /health (device check-in)
    │   ├── dashboard.py        ← GET /dashboard + /station/{id} + CSVs
    │   ├── summary.py          ← GET /summary (Fleet Stats) + /help (Legend)
    │   ├── ota.py              ← GET/POST /ota + individual station OTA
    │   └── commands.py         ← GET /cmd/{stn_id}/{command} (REBOOT, FTP_BACKLOG, FTP_DAILY)
    ├── services/
    │   └── ota_service.py      ← Version comparison logic
    └── templates/
        ├── base.html           ← Shared nav + styles
        ├── dashboard.html      ← Main fleet overview + map
        ├── summary.html        ← Per-group summary cards
        ├── help.html           ← Status code legend & help guide
        ├── station.html        ← Per-station history + individual OTA
        └── ota.html            ← 6 firmware groups + progress bars
```

---

## 🗄️ Database Schema (SQLite at `/app/data/health.db`)

### `firmware_registry`
| Column | Notes |
|--------|-------|
| `category_id` | PK. 1=SPATIKA_ADDON, 2=KSNDMC_ADDON, 3=KSNDMC_TWS, 4=SPATIKA_TRG, 5=KSNDMC_TRG, 6=BIHAR_TRG |
| `name` | Internal name e.g. "SPATIKA_ADDON" |
| `display_name` | UI label e.g. "KSNDMC-TWS" |
| `unit_type` | Device filter — matches `UNIT` from `globals.h` |
| `system_mode` | Device filter — matches `SYSTEM` from `globals.h` (0=TRG, 1=TWS, 2=ADDON/RF) |
| `current_ver` | Target OTA version e.g. "5.78" |
| `total_target` | Expected total stations in this group |
| `updated_at` | Auto-updates when `current_ver` changes |

### `health_reports`
Full JSON payload from device stored here. Key fields: `stn_id`, `unit_type`, `system`, `health_sts`, `ver`, `bat_v`, `sol_v`, `signal`, `net_cnt`, `net_cnt_prev`, `reg_fails`, `reported_at`.

### `command_queue`
Remote commands waiting to be piggybacked on next device check-in. `executed_at` is NULL until device picks it up.

---

## 🔌 API Endpoints

| Method | Path | Description |
|--------|------|-------------|
| `POST` | `/health` | Device check-in (JSON body). Returns `{"status","cmd","cmd_param"}` |
| `GET` | `/dashboard` | Fleet overview — unique stations only |
| `GET` | `/station/{stn_id}` | Full history page for one station |
| `GET` | `/station/{stn_id}/csv` | Download station history as CSV |
| `GET` | `/download_all` | Download all latest unique stations as CSV |
| `GET` | `/delete/{stn_id}` | Delete all records for a station |
| `GET` | `/ota` | OTA management page (6 groups) |
| `POST` | `/ota/upload/{cat_id}` | Set target version + upload .bin for a group |
| `POST` | `/station/{stn_id}/ota` | Upload custom .bin for ONE station |
| `GET` | `/cmd/{stn_id}/{command}` | Queue a command (REBOOT, FTP_NOW, OTA_CHECK) |

---

## 🚀 The 6 Firmware Groups

| ID | Name | Display | unit_type | system_mode |
|----|------|---------|-----------|-------------|
| 1 | SPATIKA_ADDON | SPATIKA-ADDON | SPATIKA_GEN | 2 |
| 2 | KSNDMC_ADDON  | KSNDMC-ADDON  | KSNDMC_ADDON | 2 |
| 3 | KSNDMC_TWS    | KSNDMC-TWS    | KSNDMC_TWS | 1 |
| 4 | SPATIKA_TRG   | SPATIKA-TRG   | SPATIKA_GEN | 0 |
| 5 | KSNDMC_TRG    | KSNDMC-TRG    | KSNDMC_TRG | 0 |
| 6 | BIHAR_TRG     | BIHAR-TRG     | BIHAR_TRG | 0 |

> **OTA Match Logic**: A device triggers OTA if its `unit_type` AND `system` (mode) match a firmware group AND its version number differs from `current_ver`.

---

## 🛠️ How to Deploy to Contabo

### Option 1: One-Shot Deploy (Best)
Run this from your **local Mac terminal** (inside the project folder):
```bash
./DEPLOY_NOW.sh
```
This script automatically syncs files directly to the VPS and restarts the Docker service.

### Option 2: Manual Mirror Sync
```bash
# From Mac:
scp -r server/* root@75.119.148.192:/opt/spatika-health/
ssh root@75.119.148.192 "cd /opt/spatika-health && docker compose restart"
```

### Option 3: Remote deploy.sh
This relies on a manual file sync to `/opt/spatika-health-src/` first.
```bash
ssh root@75.119.148.192 "bash /opt/spatika-health-src/deploy.sh"
```

---

## 📌 Known Issues & History

### v3.1 (Current — 2026-03-01)
- **Interactive Dash Filters**: Clicking **ALARMS**, **OTA**, or **LOW BAT** instantly filters the station table.
- **Improved CSV Export**: "Export Filtered CSV" generates a download for only currently visible results.
- **Map Fixes**: Stabilized Leaflet.js rendering using JSON script tag data injection.
- **Split FTP Commands**: Differentiated between `FTP_BACKLOG` (unsent.txt) and `FTP_DAILY` (specific date file).
- **New Dashboard Tabs**: Added "Fleet Summary" and "Help & Legend" pages.
- **UI Refinement**: Removed Signal from main dashboard, renamed Net C/P to Data Today/Yest.
- **Firmware (v5.80)**: ESP32 code updated to parse and execute new split FTP commands.

### v3.0 (2026-03-01)
- Full modular router architecture implemented
- Fixed JSON parsing issues (comma-separated raw strings)
- Fixed `reported_at` vs `timestamp` confusion
- Added individual station OTA upload
- Added Leaflet.js map view
- Added per-station history page with CSV download
- Added 6 firmware groups seeded at startup

### v2.0 Bugs (Fixed)
- `python-multipart` was missing → file upload always crashed with HTTP 500
- `builds/` directory didn't exist → atomic file rename failed
- Dashboard tried to access `HealthReport.timestamp` (wrong column name)
- OTA endpoint returned `{"error": "ota.html"}` because template folder was missing
- Server was parsing JSON payload as CSV (splitting on commas) causing `"unit_type":"SPATIKA_GEN"` to appear as Station ID

---

## 🔮 Planned Features (Phase 4)
- Field engineer list with area assignments
- WhatsApp dispatch for alarm stations
- Geofencing for area-based routing
- Auto-clear for executed commands older than 7 days
