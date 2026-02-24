# Release Notes: v5.40 (Feb 24, 2026 - Production Build)

## 🎯 Release Overview
A major sensor expansion and code footprint optimization update. v5.40 introduces **BME280 Barometric Pressure Sensor** support with real-time **Mean Sea Level Pressure (MSLP)** calculation, a new **LCD-configurable Station Altitude** field with auto-detection, a new **`KSNDMC_TWS-AP`** unit variant, graduated network diagnostics, and the permanent removal of the unused ESP-NOW subsystem. Data records are now extended with a pressure column for all TWS configurations.

---

## ✨ Key Enhancements

### 1. **BME280 Barometric Pressure Sensor Integration** 🌡️📊
- **New Sensor Driver** (`bme280.ino`): Full Adafruit BME280 driver integration with dual I2C address probing (`0x77` → `0x76` fallback) at boot.
- **Conditional Task Spawning**: `bmeTask` is only created if the BME280 is physically detected. Units without the sensor incur **zero RAM overhead** — RTOS task not created, saving ~4 KB of dynamic memory.
- **Station Pressure Reading**: Continuous pressure sampling in hPa with valid-range guard (`> 300 hPa`).
- **Mean Sea Level Pressure (MSLP)**: Real-time MSLP calculated using the standard hypsometric formula:
  `P_sl = P_st / (1 - h/44330.769)^5.255`
- **Web UI Pressure Card**: Live Pressure dashboard card (Station | SLP in hPa) added to Web UI. Gracefully degrades to `"BME: No Sensor"` placeholder if hardware absent.
- **Pressure in Data Records**: All logged data (real-time and gap-filled) now include a `pressure` column for TWS (`SYSTEM 1`) and TWS-RF (`SYSTEM 2`) configurations.

### 2. **Station Altitude — LCD Configurable with Auto-Detection** 🗻
- **New LCD Field** (`FLD_ALTITUDE`): New "Station Alt (m)" field in the LCD menu (hidden if BME280 not installed).
- **Auto-Detection on Press**: When the operator presses `SET` on the altitude field, the firmware reads the current BME280 pressure and computes the approximate altitude using the ISA standard atmosphere formula, displaying `"AUTO DETECTED: Xm"` for one-button confirmation.
- **Manual Override**: If BME280 is absent or pressure data is invalid, a manual numeric entry mode (0–5000 m) is offered.
- **SPIFFS Persistence** (`/station_alt.txt`): Altitude is saved to SPIFFS on every change and reloaded at boot. Default: `900 m` (Mahalakshmipuram, Bengaluru). Auto-applied when a new Station ID is set.
- **Device Auto-Restart**: Altitude changes trigger an automatic restart to re-initialize MSLP calculations with the new value.

### 3. **New Unit Variant: `KSNDMC_TWS-AP`** 📡
- Added `KSNDMC_TWS-AP` as a distinct sub-variant of `KSNDMC_TWS (SYSTEM 1)`.
- Unit version string is tagged `TWS9-AP-DMC-<version>` when the `-AP` suffix is detected in `UNIT[]`, allowing server-side differentiation of Access Point capable nodes.

### 4. **ESP-NOW Subsystem — Permanently Disabled** 🚫
- The ESP-NOW subsystem (`espnow.ino` task, `esp_now.h` include, peer MAC fields, timer setup) has been fully disabled and removed from the active build path.
- `ENABLE_ESPNOW` guard now cleanly skips all ESP-NOW includes/code, saving significant flash and RAM.
- Stub comments left in `espnow.ino` for future restoration if required.

### 5. **Graduated Network Diagnostics** 🔬
- HTTP upload failures are now **independently tracked** from RF/cellular registration failures (`diag_consecutive_http_fails`).
- Graduated status strings replace the previous binary `GPRS_FAIL` indicator:
  - `≥ 6` consecutive HTTP fails → `HTTP_UPLOAD_CRITICAL`
  - `≥ 3` consecutive HTTP fails → `HTTP_UPLOAD_ISSUE`
  - `≥ 6` registration fails → `NETWORK_CRITICAL`
  - `≥ 3` registration fails → `NETWORK_ISSUE`
  - `≥ 1` registration fail → `NETWORK_MINOR_ISSUE`
- **BME Status in Sensor Report**: `sensor_info` health string now includes `BME:OK` or `BME:FAIL` alongside `HDC`, `WS`, and `WD` sensor states.

### 6. **TRNG-Based Jitter (Anti-Flatline Improvement)** 🎲
- The organic jitter algorithm in `tempHum.ino` has been upgraded from `rand()` (dependent on seeded PRNG) to `esp_random()` (hardware True Random Number Generator backed by the ESP32's RF noise).
- This guarantees statistically unpredictable perturbations independent of boot seed, solving edge cases where power-cycling could produce identical jitter sequences.

### 7. **Firmware Version as Single Source of Truth** 🔗
- `UNIT_VER` strings for `TRG` and `TWS` variants are now built dynamically using `FIRMWARE_VERSION` macro (`snprintf` with `FIRMWARE_VERSION`), eliminating the risk of version string/define desync.

---

## 🔧 Technical Summary

### Modified / New Files:
- `bme280.ino` (**NEW**): BME280 sensor driver, `initBME()`, `bmeTask()`, MSLP computation
- `globals.h`: `BME_Type` enum, `pressure`, `sea_level_pressure`, `pres_str` globals, `station_altitude_m`, `SEALEVEL_CALC_FACTOR`, ESP-NOW cleanup, version bump to `5.40`, `KSNDMC_TWS-AP` unit string
- `AIO9_5.0.ino`: Conditional `bmeTask` creation, SPIFFS altitude load at boot, `UNIT_VER` single-source fix, TWS-AP variant handling
- `lcdkeypad.ino`: `FLD_ALTITUDE` field, auto-detect altitude flow, manual altitude entry, pressure display update, BME-conditional visibility
- `scheduler.ino`: Pressure column added to all data record formats (real-time + gap-filled) for `SYSTEM 1` and `SYSTEM 2`
- `gprs.ino`: Graduated network diagnostics, BME status in sensor health string
- `webServer.ino`: Live Pressure card (Station | MSLP hPa), BME-absent placeholder, updated Kannada CSV legend (pressure column)
- `tempHum.ino`: `esp_random()` replaces `rand()` for TRNG-backed jitter
- `espnow.ino`: Full stub — ESP-NOW task permanently disabled

### Code Size (v5.40):
- **Program Storage**: ~85% (BME driver and MSLP logic partially offset by ESP-NOW removal)
- **Dynamic RAM**: ~19% (bmeTask not created on sensor-absent units)

---

## 📦 Release Contents

Includes pre-compiled binaries for:
1. **KSNDMC_TRG**, **BIHAR_TRG**, **SPATIKA_TRG** (SYSTEM 0)
2. **KSNDMC_TWS**, **KSNDMC_TWS-AP** (SYSTEM 1)
3. **KSNDMC_ADDON**, **SPATIKA_ADDON** (SYSTEM 2)

---

**v5.40 adds real-time barometric pressure and MSLP to the data record, with zero RAM cost on units without the sensor!** 🚀
