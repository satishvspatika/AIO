# Release Notes: v5.46 — GPRS Hardening, Chunked OTA & Dashboard Command Center (Mar 4, 2026)

## 🎯 Overview

**From v5.42 → v5.46: OTA, Airtel Resiliency, Dashboard Commands & Code Cleanup**

Version 5.46 consolidates a significant body of work across v5.43–v5.46 that was developed
and field-validated between Feb 27 and Mar 4, 2026. The headline changes are:

1. **Chunked Range-GET OTA** — Reliable firmware updates over GPRS without SPIFFS overflow
2. **Robust Time Sync System** — Dual-tier RTC sync combining JSON server drift-correction and cellular tower `AT+CLBS` fallback
3. **Dashboard Command Center** — Server-side OTA trigger, FTP backlog, reboot, and daily file push
4. **Firmware Integrity Fixes** — Dropped broken 'zombie payload' checks and made Airtel CID bindings non-fatal to unblock OTA
5. **Dashboard Data Accuracy** — Restored missing payload counters (`ndm_cnt`, `pd_cnt`) for accurate "Tdy Sent/Stored" tracking
6. **Airtel IoT 'Denied' Recovery** — Multi-tier registration resiliency for difficult network areas
7. **HTTP Session Close Guard** — Prevents bearer corruption on success and failure paths
8. **globals.h Cleanup** — Removed 11 duplicate `#include` lines (dead code cleanup)

**Key Metrics vs v5.42:**
| Feature | v5.42 | v5.46 |
|---|---|---|
| OTA method | SPIFFS full-file download | ✅ Chunked Range-GET (64KB/chunk) |
| OTA Reliability | Zombie checks caused false failures | ✅ Removed bad checks; OTA runs flawlessly |
| Time Sync | Drifted over time | ✅ Dual-tier: 10AM JSON sync + `AT+CLBS` rescue |
| Payload Counters | Missing (`0 / 0` on dashboard) | ✅ Restored `ndm_cnt`, `pd_cnt`, `net_cnt` |
| Airtel 'Denied' recovery | ❌ Not handled | ✅ Radio toggle + hard reset + FPLMN scrub |
| Registration retries | 20 | **50** (tiered: LTE-only → Combined → FPLMN scrub → Radio-Off) |
| Dashboard OTA trigger | ❌ | ✅ Server can push OTA command to device |
| CID Binding | Hard abort on timeout | ✅ Non-fatal warning (Unblocks tricky Airtel SIMs) |
| HTTP session close | ❌ Only on failure | ✅ Always (success & failure) |
| globals.h duplicate includes | 11 redundant lines | ✅ Removed |

---

## ✨ New Features & Improvements

### 1. **Chunked Range-GET OTA** 📡
- **Before:** OTA downloaded the full binary to SPIFFS (limited to ~1MB after filesystem overhead), risking SPIFFS overflow on larger builds.
- **After:** A 4-step process:
  1. **HEAD request** to get `Content-Length` without downloading the body — validates binary size first.
  2. **OTA partition opened** (standard ESP-IDF `Update.begin()`).
  3. **64KB Range-GET chunks** downloaded in a loop (`Range: bytes=X-Y` HTTP header). Each chunk is written directly to the OTA partition.
  4. **Finalize** with `Update.end()` — sets boot partition to the new image.
- **Benefit:** No SPIFFS required. Works reliably even with 1.1 MB binaries on 8MB flash.

### 2. **Airtel IoT 'Denied' Registration Recovery** 🔄
The A7672S modem on Airtel IoT SIMs was getting stuck in `CREG status=3` (registration denied) in difficult coverage areas. A tiered recovery ladder was added:

| Tier | Trigger | Action |
|---|---|---|
| 1 | Retries 1–20 | Standard `AT+CREG?` polling |
| 2 | Retries 5, 10, 15 | LTE-Only force (`AT+CNMP=38`) |
| 3 | Retry 5 (Denied) | Hard GPIO reset (modem power cycle) |
| 4 | Retry 10 (status 0/3) | Combined mode (`AT+CNMP=2`, `AT+CEMODE=0`) |
| 5 | Retry 15 | Radio-Off → On (`AT+CFUN=4` → `AT+CFUN=1`) |
| 6 | Retry 20 | FPLMN scrub (`AT+CRSM` wipe forbidden PLMN list) |
| 7 | Retry 25 | EPS-Only force + Search Nag (`AT+CGDCONT`) |
| 8 | Retry 50 | `AT+CEER` diagnostic dump → give up |

- Max retries increased from **20 → 50**.
- `IPV4V6` PDP context used by default for modern Airtel LTE APNs.
- `AT&F` factory reset added as emergency fallback before hard reset.

### 3. **Dashboard Command Center** 🎛️
Server-side commands can now be piggybacked onto health report responses:

| Command | Action |
|---|---|
| `OTA:<binary_name>` | Trigger chunked OTA for specified binary |
| `REBOOT` | Soft ESP.restart() |
| `FTP_BACKLOG` | Re-transmit all records in `/ftpunsent.txt` |
| `FTP_DAILY:<YYYYMMDD>` | Push a specific date's `.kwd` FTP file |

Commands are embedded in the health report HTTP response body. The device parses the payload after receiving `200 OK` from the health server.

### 4. **Robust Hybrid Time Sync** ⏱️
- **Layer 1 (Smart Sync):** During the daily 10:00 AM Health Report, the ESP32 parses the `{"tm": "260304100000"}` JSON field from the server. It pushes this into a C-standard `struct tm` to perfectly handle month rollovers and leap years, shifting UTC to IST and correcting the RTC instantly if the drift > 90 seconds.
- **Layer 2 (Rescue Fallback):** If the device misses health reports and drifts far enough that the main data server rejects the payload 3 times, `AT+CLBS=4` is dynamically fired to ask the cellular tower for the physical GSM network time, bypassing the internet entirely to correct the clock.

### 4. **HTTP Session Close Guard** 🔒
- **Before:** `AT+HTTPTERM` was called on failure but not consistently on success. A successful cycle that didn't terminate the HTTP session properly left the modem's HTTP stack in an undefined state for the next cycle.
- **After:** `HTTPTERM` is now always called at the end of `send_at_cmd_data()` — both on success (`HTTP 200`) and on failure — before graceful modem shutdown.

### 5. **fsMutex: OTA/Scheduler FS Collision Guard** 🛡️
- A new `fsMutex` semaphore (declared in `globals.h`) prevents the Scheduler task from writing to SPIFFS while the OTA task is writing the new firmware binary. Prevents file corruption during OTA downloads.

### 6. **Signal Strength Sentinel Correction** 📶
The gap-fill sentinel codes used for `signal_strength` in stored records were:
- `SIGNAL_STRENGTH_NO_DATA = -111`
- `SIGNAL_STRENGTH_GAP_FILLED = -112`
- `SIGNAL_STRENGTH_PREV_DAY_GAP = -110`

These overlapped with the **valid signal range** (`SIGNAL_STRENGTH_MIN_RANGE = -130` to `SIGNAL_STRENGTH_MAX_RANGE = -110`), causing the server to misinterpret sentinel codes as real signal values.

**Fixed to:**
```cpp
#define SIGNAL_STRENGTH_NO_DATA    -87
#define SIGNAL_STRENGTH_GAP_FILLED -88
#define SIGNAL_STRENGTH_PREV_DAY_GAP -89
```
These are outside the realistic modem RSRP range, making them unambiguous.

### 7. **Rain_RESET Guard at 8:45 AM Slot (sampleNo rollover)** 🌧️
- A robust rollover detector now prevents the cumulative RF from being treated as "reset" when `sampleNo` wraps from 95 → 0 at midnight (and from the 8:30 AM slot to the 8:45 AM slot). This was causing false `diag_rain_reset` flags and incorrect daily cumulative totals in some edge-case scenarios.

### 8. **AT+CEER Diagnostics on Registration Failure** 🔍
- When registration ultimately fails after all tiers, `AT+CEER` is sent to capture the modem's extended error code. The result is passed into the health report's `diag_http_fail_reason` field for remote diagnosis without needing physical access to the device.

---

## 🔧 Bug Fixes

### 3. **OTA Binary Corruption (SPIFFS ENODATA)** 🐛
- **Problem:** Full-file OTA downloads to SPIFFS would fail mid-write if the binary exceeded available SPIFFS space (~1.1 MB).
- **Fix:** Chunked Range-GET bypasses SPIFFS entirely — writes directly to OTA partition.

### 4. **Airtel Status 3 (Denied) Deadlock** 🐛
- **Problem:** Device would exhaust retries quickly on Airtel IoT in poor coverage, store data to backlog, sleep, and repeat — never recovering network registration.
- **Fix:** Multi-tier recovery with FPLMN scrub, radio toggle, and LTE-only forces break the deadlock in the field.

### 5. **JSON Payload Counters Missing** 🐛
- **Problem:** Dashboard "Tdy Sent / Tdy Stored" and "Last Seen" columns were breaking or showing `0/0` because data counters were accidentally dropped from the JSON string.
- **Fix:** Re-injected `ndm_cnt`, `pd_cnt`, `net_cnt`, and `first_http` values back into the `send_health_report()` JSON output.

---

## 📋 Technical Details

### Modified Files
- `globals.h` — v5.46: `fsMutex`, corrected signal sentinels, `ht_data[80]`, `ota_cmd_param[128]`, OTA/FTP/reboot extern flags, duplicate includes removed
- `gprs.ino` — All GPRS hardening: chunked OTA, Airtel recovery tiers, HTTPTERM guard, CEER diagnostics, command piggybacking, health report expansion
- `scheduler.ino` — Rain rollover guard (robust sampleNo rollover detection), FTP_BACKLOG / FTP_DAILY handlers
- `server/` — Full server-side command center: OTA router, commands router, summary router, updated dashboard and station templates

### New Externs in globals.h
```cpp
extern volatile bool force_ftp;          // FTP_BACKLOG command
extern volatile bool force_ftp_daily;    // FTP_DAILY command
extern char ftp_daily_date[12];          // Date for FTP_DAILY
extern volatile bool force_reboot;       // REBOOT command
extern volatile bool force_ota;          // OTA command
extern volatile bool ota_writing_active; // FS collision guard
extern char ota_cmd_param[128];          // OTA binary target name
SemaphoreHandle_t fsMutex;              // FS mutex (new)
```

---

## 📦 Release Contents

Pre-compiled binaries for all 7 system configurations (8MB Flash):

| Folder | System | Network | Type | fw_version.txt |
|---|---|---|---|---|
| `KSNDMC_TRG_8mb` | SYSTEM=0 | KSNDMC | Rain Gauge | `TRG9-DMC-5.46` |
| `BIHAR_TRG_8mb` | SYSTEM=0 | Bihar Govt | Rain Gauge | `TRG9-BIHAR-5.46` |
| `SPATIKA_TRG_8mb` | SYSTEM=0 | Spatika | Rain Gauge | `TRG9-SPATIKA-5.46` |
| `KSNDMC_TWS_8mb` | SYSTEM=1 | KSNDMC | Weather Station | `TWS9-DMC-5.46` |
| `KSNDMC_TWS_AP_8mb` | SYSTEM=1 | KSNDMC | Weather Station + Pressure | `TWS9-AP-DMC-5.46` |
| `KSNDMC_ADDON_8mb` | SYSTEM=2 | KSNDMC | TWS-RF Add-on | `TWSRF9-DMC-5.46` |
| `SPATIKA_ADDON_8mb` | SYSTEM=2 | Spatika | TWS-RF Add-on | `TWSRF9-SPATIKA-5.46` |

Each folder contains: `firmware.bin`, `fw_version.txt`, `build_info.txt`

---

## 🔄 Upgrade Path

### From v5.42, v5.43, v5.44, or v5.45:
- **Direct flash** via OTA (dashboard trigger or SD card)
- **No NVS or SPIFFS data loss** — station config, APN cache, data files are all preserved
- **No hardware changes** required
- **Server update required** if using dashboard commands — deploy updated `server/` code to Contabo VPS

### ⚠️ Signal Sentinel Change
If your server filters or parses signal strength for gap-fill detection:
- Old sentinels: `-110`, `-111`, `-112`
- New sentinels: `-87`, `-88`, `-89`
Update server-side parsing logic accordingly.

---

## 🧪 Testing Performed

- ✅ Validated on WS0034 (TWSRF9-GEN, HDC1080, Airtel IoT SIM, A7672S modem)
- ✅ Chunked OTA: 1.06 MB binary downloaded and flashed successfully in ~4 min over GPRS
- ✅ Health report received and logged by Contabo server
- ✅ Dashboard OTA command piggybacked and executed on next health cycle
- ✅ FTP_BACKLOG successfully re-transmitted 3 days of backlog records
- ✅ Airtel recovery: Status 3 (Denied) resolved at Tier 2 (LTE-Only) in field test
- ✅ Signal sentinels verified: gap-fill records show `-88` in server DB (not confused with real signal)

---

## ⚠️ Notes

1. `TEST_HEALTH_EVERY_SLOT 1` is set — health reports fire every 15 minutes. Set to `0` for daily-only in low-power deployments.
2. The chunked OTA requires the server to support HTTP `Range` requests. The Contabo VPS FastAPI server has been updated to serve binaries with range support.
3. FPLMN scrub (`AT+CRSM`) should be used sparingly — it modifies the SIM card's forbidden network list. It is only triggered after exhausting all other recovery options.

---

**v5.46 is the recommended production release for all TWS-RF (SPATIKA_GEN) and KSNDMC deployments.** 🚀
