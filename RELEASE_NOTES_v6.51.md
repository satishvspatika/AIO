# Release Notes: v6.51 (Sep 30, 2026)

## 🎯 Overview
v6.51 is a major stability and functionality update introducing SD card same-version re-flash protection, secondary server dual-broadcasting, remote control center commands, SIMCOM modem null-byte sanitization, and 16MB batch flashing automation.

---

## ✨ New Features

### 1. **Secondary Server & Dual Broadcast Mode (`server_mode`)** 🌐
- **Description:** Added multi-server configuration (`SERVER2_DOMAIN`: `devhlt.spatika.net`). Devices can route telemetry payloads to primary, secondary, or both servers simultaneously.
- **Modes:** 0: Server 1 Only, 1: Server 2 Only, 2: Dual Broadcast Mode (Posts to both endpoints).

### 2. **Remote Control Center Commands** 📡
- **Description:** Added remote command parsing in GPRS health check-in responses (`SET_STATION_ID`, `SET_SERVER_1`, `SET_SERVER_2`, `SET_SERVER_MODE`).
- **Immediate Feedback:** Automatically sets `force_health_upload = true` after applying remote config changes for instant server confirmation.

### 3. **16MB Native Batch Flashing & Automated QC Scripts** ⚡
- **Description:** Added high-speed batch flashing scripts (`flash_fresh_board.sh`, `flash_fresh_board_KSNDMC_TRG.sh`, `flash_fresh_board_bihar.sh`) with automated 10-second post-flash serial QC hardware verification.

---

## 🔧 Bug Fixes

### 1. **SD Card Same-Version Re-Flash Fix** 💾
- **Problem:** SD card update logic could misread version strings (e.g. "v6.50" vs "6.50") or mismatch hardware model prefixes (TRG vs TWS), causing unnecessary re-flashing or skipping across different units.
- **Solution:** Added version string normalization (stripping leading 'v'/'V') and full exact matching against `UNIT_VER`.
- **Impact:** Preserves SD card binary so a single SD card can update multiple field boards sequentially without deleting `.bin`.

### 2. **Modem Response Null-Byte Sanitization** 🛠️
- **Problem:** Unexpected `\0` null bytes in SIMCOM AT responses truncated C-string buffers during string search operations.
- **Solution:** Replaced incoming `\0` null bytes with spaces `' '` in `waitForResponse()`.
- **Impact:** Eliminates AT response parser truncation and modem timeout lockups.

### 3. **Port 80 HTTP URL Formatting** 🌐
- **Problem:** Explicit `:80` in SIMCOM `AT+HTTPPARA="URL"` caused HTTP errors on specific SIMCOM modem firmware builds.
- **Solution:** Omits `:80` when port is 80 (`http://domain/path`).

---

## 📋 Technical Details

### Modified Files
- `AIO9_5.0.ino` - SD card version string normalization, NVS server mode initialization.
- `gprs_health.ino` - Remote commands (`SET_STATION_ID`, `SET_SERVER_1`, `SET_SERVER_2`, `SET_SERVER_MODE`), Port 80 URL formatting, `Accept-Encoding: identity` header.
- `gprs_helpers.ino` - Null-byte sanitization in `waitForResponse()` and `waitForResponseNoFlush()`.
- `gprs_http.ino` - Dual broadcast transmission logic for `server_mode == 2`.
- `user_config.h` - Secondary server configuration macros (`SERVER2_*`).
- `flash_fresh_board*.sh` - High-speed 16MB batch flashing & post-flash QC scripts.

---

## 📦 Release Contents

Includes pre-compiled binaries for all 18 system/UI/flash configurations:
- **Rain Gauges (SYSTEM=0):** `KSNDMC_TRG`, `BIHAR_TRG` (Nuvoton UI & Matrix UI for 16MB & 8MB).
- **Weather Stations (SYSTEM=1):** `KSNDMC_TWS` (Nuvoton UI & Matrix UI for 16MB & 8MB).
- **Add-ons & Generic (SYSTEM=2 & 3):** `KSNDMC_ADDON`, `SPATIKA_GEN`, `SPATIKA_TWSRP`.

---

## 🔄 Upgrade Path

### From v6.50:
- **Direct OTA / SD Card Upgrade** - Flash v6.51 firmware binary.
- **No configuration reset required.**

---

**v6.51 is production-ready!** 🚀
