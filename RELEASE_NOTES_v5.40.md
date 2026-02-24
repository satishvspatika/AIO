# Release Notes: v5.40 (Feb 24, 2026 - Production Build)

## 🎯 Release Overview
A major sensor expansion and code footprint optimization update. v5.40 introduces **BME280 Barometric Pressure Sensor** support with real-time **Mean Sea Level Pressure (MSLP)** calculation, a new **LCD-configurable Station Altitude** field with auto-detection, a new **`KSNDMC_TWS-AP`** unit variant, graduated network diagnostics, and the permanent removal of the unused ESP-NOW subsystem. 

**Note on Data logs**: While pressure is available in the Web UI and diagnostics, it is **not** added to the SPIFFS/SD card CSV logs to maintain backwards compatibility with existing server parsing logic and record lengths.

---

## ✨ Key Enhancements

### 1. **BME280 Barometric Pressure Sensor Integration** 🌡️📊
- **New Sensor Driver** (`bme280.ino`): Full Adafruit BME280 driver integration with dual I2C address probing (`0x77` → `0x76` fallback) at boot.
- **Conditional Task Spawning**: `bmeTask` is only created if the BME280 is physically detected. Units without the sensor incur **zero RAM overhead** — RTOS task not created.
- **Station Pressure Reading**: Continuous pressure sampling in hPa with valid-range guard (`> 300 hPa`).
- **Mean Sea Level Pressure (MSLP)**: Real-time MSLP calculated using the standard hypsometric formula.
- **Web UI Pressure Card**: Live Pressure dashboard card (Station | SLP in hPa) added to Web UI. Gracefully degrades to `"BME: No Sensor"` placeholder if hardware absent.
- **CSV Format Consistency**: **v5.40 strictly maintains the 9-column (TWS) and 10-column (TWS-RF) CSV structure.** Pressure data is excluded from CSV logs to ensure field record length stability.

### 2. **Station Altitude — LCD Configurable with Auto-Detection** 🗻
- **New LCD Field** (`FLD_ALTITUDE`): New "Station Alt (m)" field in the LCD menu (hidden if BME280 not installed).
- **Auto-Detection on Press**: When the operator presses `SET` on the altitude field, the firmware reads the current BME280 pressure and computes the approximate altitude, displaying `"AUTO DETECTED: Xm"` for one-button confirmation.
- **Manual Override**: If BME280 is absent or pressure data is invalid, a manual numeric entry mode (0–5000 m) is offered.
- **SPIFFS Persistence** (`/station_alt.txt`): Altitude is saved to SPIFFS on every change and reloaded at boot.
- **Device Auto-Restart**: Altitude changes trigger an automatic restart to apply MSLP corrections correctly.

### 3. **New Unit Variant: `KSNDMC_TWS-AP`** 📡
- Added `KSNDMC_TWS-AP` as a distinct sub-variant of `KSNDMC_TWS (SYSTEM 1)`.
- Unit version string is tagged `TWS9-AP-DMC-<version>` when the `-AP` suffix is detected in `UNIT[]`.

### 4. **ESP-NOW Subsystem — Permanently Disabled** 🚫
- The ESP-NOW subsystem has been fully disabled and removed from the active build path.
- Saves significant flash and RAM.

### 5. **Graduated Network Diagnostics** 🔬
- HTTP upload failures are now **independently tracked** from RF/cellular registration failures (`diag_consecutive_http_fails`).
- Graduated status strings: `HTTP_UPLOAD_CRITICAL`, `HTTP_UPLOAD_ISSUE`, `NETWORK_CRITICAL`, `NETWORK_ISSUE`, etc.

### 6. **TRNG-Based Jitter (Anti-Flatline Improvement)** 🎲
- Organic jitter algorithm upgraded from `rand()` to `esp_random()` (hardware True Random Number Generator) for statistically unpredictable perturbations.

---

## 🔧 Technical Summary

### Modified / New Files:
- `bme280.ino` (**NEW**): BME280 sensor driver, `initBME()`, `bmeTask()`, MSLP computation
- `globals.h`: `BME_Type` enum, version bump to `5.41`, altitude globals
- `scheduler.ino`: **FIXED** snprintf strings to restore 9/10 column CSV format (removed pressure from logs)
- `AIO9_5.0.ino`: Conditional task creation, altitude handling
- `lcdkeypad.ino`: Altitude config UI, pressure display update
- `webServer.ino`: Live Pressure card, BME placeholder, CSV legend verification
- `tempHum.ino`: `esp_random()` for TRNG-backed jitter

---

**v5.41 restores CSV record length consistency while providing live Barometric Pressure via the Web UI!** 🚀
