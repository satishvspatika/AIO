# Release Notes: v5.42 — Contabo Health Server Integration (Feb 27, 2026)

## 🎯 Overview

**Spatika Health Monitor — Self-Hosted Server on Contabo VPS**

Version 5.42 replaces the Google Sheets health reporting backend with a self-hosted
FastAPI service running on a Contabo Ubuntu VPS (`75.119.148.192`). This eliminates
the dependency on Google Apps Script (HTTPS/TLS overhead, redirect chains, DNS
resolution complexity) and replaces it with a direct plain-HTTP POST to a dedicated
monitoring server — significantly more reliable on the A7672S modem.

A companion Docker-hosted web dashboard is now live at:
**`http://75.119.148.192/dashboard`**

**Key Metrics vs v5.41:**
| Metric | v5.41 | v5.42 |
|---|---|---|
| Health report target | Google Sheets (HTTPS) | Contabo VPS (plain HTTP) |
| SSL/TLS setup commands | 7 AT commands | **0** (removed entirely) |
| HTTPACTION timeout | 60 seconds | 30 seconds |
| DNS lookup per report | Yes (script.google.com) | **No** (direct IP) |
| Redirect handling | Required (`REDIR=1`, 302) | Not needed (200 direct) |
| Dashboard | Google Sheets | ✅ Web UI at `/dashboard` |
| ENABLE_HEALTH_REPORT | 0 (off) | **1 (on)** |

---

## ✨ New Features & Improvements

### 1. **Self-Hosted Health Server (Docker)** 🐳
A new FastAPI service runs on the Contabo VPS in a Docker container:
- **POST `/health`** — ESP32 devices post JSON health reports here
- **GET `/dashboard`** — Live overview of all stations (green=OK, red=fault)
- **GET `/dashboard/{stn_id}`** — Per-station history (last 96 reports)
- **GET `/api/stations`** — JSON API for latest report per station
- **GET `/api/station/{stn_id}`** — JSON API for station history
- **SQLite** database (`/opt/spatika-health/data/health.db`) — persistent across restarts
- Auto-restarts on server reboot (`restart: unless-stopped`)

### 2. **Plain HTTP Health Reporting** ⚡
- **Before:** Health reports required HTTPS with full SSL/TLS setup:
  `AT+HTTPPARA="HTTPS",1` → `AT+CSSLCFG="sslversion"` → `AT+CSSLCFG="authmode"`
  → `AT+CSSLCFG="ignorecerc"` → `AT+CSSLCFG="sni"` → DNS warmup → 60s timeout
- **After:** Plain HTTP POST in 3 AT commands:
  `AT+HTTPINIT` → `AT+HTTPPARA="URL","http://75.119.148.192:80/health"` → `AT+HTTPACTION=1`
- **Saving:** ~8–12 seconds per health report cycle (SSL handshake eliminated)

### 3. **New globals.h Defines**
```cpp
#define HEALTH_SERVER_IP   "75.119.148.192"
#define HEALTH_SERVER_PATH "/health"
#define HEALTH_SERVER_PORT "80"
```
Easy to update if the server IP changes — no code changes needed in `gprs.ino`.

### 4. **Health Reporting Enabled**
```cpp
#define ENABLE_HEALTH_REPORT 1   // was 0
```
With `TEST_HEALTH_EVERY_SLOT 1`, a health report is sent every 15-minute cycle
(useful for commissioning and field diagnostics).

---

## 🖥️ Server Infrastructure

**Host:** Contabo VPS — `75.119.148.192`
**OS:** Ubuntu 24.04.4 LTS (`spatika-health`)
**Stack:** Docker → Python 3.11-slim → FastAPI + SQLite
**Location:** `/opt/spatika-health/`

```
/opt/spatika-health/
├── Dockerfile
├── docker-compose.yml
├── requirements.txt
├── data/
│   └── health.db          ← SQLite database (persisted via volume)
└── app/
    ├── __init__.py
    ├── database.py
    ├── models.py
    └── main.py
```

### JSON Payload (ESP32 → Server)
```json
{
  "stn_id": "WS0034",
  "unit_type": "SPATIKA_GEN",
  "health_sts": "OK",
  "sensor_sts": "H:OK,B:FAIL,W:OK,D:OK",
  "bat_v": 4.10,
  "sol_v": 12.90,
  "net_cnt": 5,
  "http_fails": 0,
  "http_fail_reason": "NONE",
  "ver": "TWSRF9-GEN-5.42",
  "carrier": "Airtel",
  "iccid": "89914509...",
  "gps": "12.971599,77.594566"
}
```

---

## 🔧 Modified Files

- `globals.h` — Version `5.42`, `ENABLE_HEALTH_REPORT 1`, replaced `GOOGLE_HEALTH_URL`
  with `HEALTH_SERVER_IP/PATH/PORT`
- `gprs.ino` — `send_health_report()` rewritten to use plain HTTP; all SSL/TLS
  setup removed; HTTPACTION timeout reduced from 60s → 30s
- `fw_version.txt` — Updated to `v5.42`

---

## 🧪 Server Validation

```bash
# Health endpoint accepts POST:
curl -X POST http://75.119.148.192/health \
  -H "Content-Type: application/json" \
  -d '{"stn_id":"WS0034","health_sts":"OK","bat_v":4.1}' 
# → {"status":"ok","stn_id":"WS0034"}

# Dashboard renders both stations:
# WS0034 → green (OK)
# WS0033 → red  (BATT_LOW_HTTP_BACKLOG)
```

---

## 🏷️ Tags

- `v5.42` — Source version tag
- `v5.42-contabo` — Marks this as the Contabo health server integration release

---

## 🔄 Upgrade Path

### From v5.41:
- **Direct flash** — no NVS or SPIFFS changes
- **Server must be running** before flashing (health reports will fail silently
  and be skipped if server is unreachable — no device impact)

---

## ⚠️ Notes

1. `ENABLE_HEALTH_REPORT 1` + `TEST_HEALTH_EVERY_SLOT 1` sends a report every
   15 minutes. Set `TEST_HEALTH_EVERY_SLOT 0` for daily-only reporting in production.
2. The server IP `75.119.148.192` is hardcoded via `HEALTH_SERVER_IP`. Update
   this define if the VPS IP changes.
3. Plain HTTP (port 80) — no TLS. Acceptable for telemetry data (no credentials
   or sensitive user data in the payload).

---

**v5.42 is the Contabo health server integration release.** 🚀
