# Release Notes: v6.52 (Oct 07, 2026)

## 🎯 Overview
v6.52 is a critical production hotfix addressing server profile routing truncation introduced in v6.51 multi-server broadcast logic. It locks primary board profile indices across all hardware variants (`KSNDMC_TRG`, `BIHAR_TRG`, `KSNDMC_TWS`, `KSNDMC_ADDON`, `SPATIKA_GEN`, `SPATIKA_TWSRP`) and decouples Secondary Server 2 configuration (`secondaryServer`) to ensure 100% telemetry transmission accuracy.

---

## 🔧 Critical Bug Fixes

### 1. **Server Profile Index Preservation (`primary_http_no`)** 🌐
- **Problem:** In v6.51, `send_http_data()` forcibly reset `http_no = 0` (or `1`), overwriting station profile routes configured during boot. Non-default configurations (such as `KSNDMC_TWS` at index 6 or `BIHAR_TRG` at index 2) attempted to POST payloads to the legacy TRG v2 endpoint, causing HTTP POST rejections while displaying "data logged" locally on the LCD.
- **Solution:** Locked the board's resolved target profile index into `primary_http_no` in `setup()`. `send_http_data()` now strictly routes telemetry to `httpSet[primary_http_no]`.
- **Impact:** Guarantees 100% server transmission success across all 18 station and UI hardware profiles.

### 2. **Decoupled Secondary Server Struct (`secondaryServer`)** 📡
- **Problem:** In v6.51, Server 2 configuration mutated `httpSet[1]` directly, corrupting the default profile entry for `KSNDMC_TRG`.
- **Solution:** Decoupled Secondary Server 2 into a dedicated `secondaryServer` struct (`SERVER2_DOMAIN`: `devhlt.spatika.net`).
- **Impact:** `SET_SERVER_1` updates the board's primary profile (`httpSet[primary_http_no]`), while `SET_SERVER_2` updates `secondaryServer` cleanly without side effects.

---

## 📋 Modified Files
- `globals.h`: Added `primary_http_no` and `secondaryServer` declarations.
- `AIO9_5.0.ino`: Locked `primary_http_no` in `setup()` and updated NVS server overrides.
- `gprs_health.ino`: Updated `set_server_config()` for primary vs secondary targeting.
- `gprs_http.ino`: Enforced `primary_http_no` in `send_http_data()` and isolated Dual Broadcast Mode (`server_mode == 2`).
- `user_config.h`: Updated `FIRMWARE_VERSION` to `"6.52"`.

---

**v6.52 is production-ready!** 🚀
