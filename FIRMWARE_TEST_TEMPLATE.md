# 🧪 AIO 9_5.0 Firmware Verification & Validation Test Plan
**Generic Quality Assurance (QA) & Field Engineering Test Template**

---

## 📌 1. Test Session Metadata

| Parameter | Value / Record | Notes |
| :--- | :--- | :--- |
| **Test Date & Time** | `YYYY-MM-DD HH:MM IST` | |
| **Test Engineer Name** | | |
| **Firmware Version Under Test**| e.g. `v6.48` | Read from `user_config.h` or UI boot screen |
| **Station ID / Name** | e.g. `WS0099` / `TRG012` | Configured in SPIFFS or NVS |
| **System Target (`SYSTEM`)** | `0` (TRG) / `1` (TWS) / `2` (TWS-RF) / `3` (TWSRP) | Check system configuration |
| **Unit Configuration (`UNIT_CFG`)**| `KSNDMC_TRG` / `BIHAR_TRG` / `KSNDMC_TWS` / `SPATIKA_GEN` / `KSNDMC_ADDON` | |
| **UI Variant** | `0` (Matrix LCD I2C) / `1` (Nuvoton UART) | `_MAT` or `_NUV` binary |
| **Cellular Network / SIM** | Airtel / BSNL / Jio / Vi | Band & Signal strength (CSQ) |
| **Flash Size Target** | 8MB / 16MB / 4MB | Partition scheme |

---

## 📋 2. Verification Checklist

### Phase 1: Firmware Flashing & Serial Boot Audit
- [ ] **Flash Execution**: Binary flashed using `esptool.js` or `./quick_flash.sh` without bootloader errors.
- [ ] **Serial Console Output**: Baud rate set to `115200`. Serial log starts cleanly without crash loops.
- [ ] **SPIFFS File System**: SPIFFS partitions mounted successfully (`/signature.txt`, `/unsent.txt`, `/lastrecorded_*.txt`).
- [ ] **Version String Verification**: Serial output logs correct version tag (e.g., `TRG9-DMC-6.48-M` or `TWSRP9-GEN-6.48-M`).

---

### Phase 2: User Interface & LCD Screen Audit
- [ ] **Display Initialization**: LCD initializes cleanly at startup (I2C address `0x27` or `0x3F` for Matrix; UART for Nuvoton).
- [ ] **Menu Navigation**: UI keypresses respond promptly without lag or missed interrupts.
- [ ] **Dashboard Field Displays**:
  - `FLD_LAST_LOGGED`: Displays correct date/time of last logged slot (`YYYY-MM-DD,HH:MM`).
  - `FLD_HTTP_FAILS`: Formatted as `P:x C:y B:z` (Present Fails, Cumulative Monthly Fails, Backlog Queue Count).
  - Battery Voltage (`bat_volt`) and Signal Strength (`CSQ`) update correctly.

---

### Phase 3: Sensor Measurement & Signal Processing Validation

#### 🔹 SYSTEM 0: Telemetry Rain Gauge (TRG)
- [ ] **Rain Pulse Count**: Tipping bucket pulse (0.5mm / 0.25mm resolution) increments instantaneous and daily cumulative rain.
- [ ] **Rain Counter Reset**: Cumulative rainfall resets at meteorological rollover time (08:30 AM IST).

#### 🔹 SYSTEM 1: Telemetry Weather Station (TWS)
- [ ] **Anemometer Sampling**: Wind speed pulse counting registers pulse frequency smoothly (ULP 1ms period).
- [ ] **Wind Direction Vane**: ADC reading (0–4095) accurately converts voltage to 0–360° wind angle.
- [ ] **Temp & Humidity**: SHT/DHT sensor reads plausible ambient temperature (°C) and relative humidity (%).

#### 🔹 SYSTEM 2: TWS-RF / ADDON Configuration
- [ ] **Sensor Fusion**: Rain bucket + Wind speed + Wind direction + Temp/Humidity readings all captured in single record buffer.

#### 🔹 SYSTEM 3: TWSRP (Weather Station with Pressure)
- [ ] **Barometric Pressure Sensor**: BMP/BME sensor returns valid atmospheric pressure (700.0–1150.0 hPa).
- [ ] **Glitch Sanitizer**: Out-of-bounds pressure spikes are auto-sanitized to default reference altitude pressure (~911.0 hPa).

---

### Phase 4: Cellular Modem & Network Attachment
- [ ] **Modem Power-On**: SIMCom A7672S modem powers up cleanly via PWRKEY / GPIO 26.
- [ ] **Signal Registration**: `AT+CSQ` returns valid RSSI (Signal > 10, CSQ >= -95 dBm).
- [ ] **Network Registration**: `AT+CREG?` / `AT+CEREG?` returns registered home/roaming (`1` or `5`).
- [ ] **PDP Context & IP Assignment**: `AT+CGACT=1,1` activates bearer and `AT+CGPADDR=1` returns a valid, non-zero IP address (not `0.0.0.0`).

---

### Phase 5: Primary Live Telemetry Transmission (HTTP & TCP Socket)
- [ ] **Slot Boundary Hold Guard**: Unit waits until `T+1m02s` past the 15-minute slot boundary before starting HTTP POST.
- [ ] **Command Echo Control**: `ATE0` issued immediately after `+CIPOPEN: 0,0` to disable modem command echoes.
- [ ] **Direct TCP Socket POST**: `AT+CIPOPEN=0,"TCP","<HOST>",80` opens successfully.
- [ ] **Response Parsing**: System captures `200 OK` or `{"status":"Success"}` within the non-blocking response window (8s cap).
- [ ] **RTC Server Synchronization**: RTC clock resynchronizes silently from server time header on drift > 90s.
- [ ] **Present Fail Counter Reset**: `diag_http_present_fails` resets to `0` on successful send.

---

### Phase 6: Backlog Storage, Migration & Recovery Flow
- [ ] **Simulated Outage Handling**: Force network failure (disconnect antenna or disable SIM).
- [ ] **Backlog Record Creation**: Undelivered record is appended to `/unsent.txt` (or `/ftpunsent.txt`).
- [ ] **Deduplication Guard**: `last_unsent_sampleNo` prevents duplicate record creation during slot rollovers or reboot loops.
- [ ] **Pointer File Integrity**: `/unsent_pointer.txt` tracks byte read position, updated via atomic `/unsent_ptr.tmp` file swap.
- [ ] **Backlog Flush Execution**: Upon network restoration, backlog records are flushed sequentially (max 15 records per wake cycle).
- [ ] **Queue Cleanup**: File `/unsent.txt` and pointer file are removed once all backlog records are successfully delivered.

---

### Phase 7: Daily FTP & HTTP Backlog Migration (SYSTEM 1 / 2 / 3)
- [ ] **Daily FTP Payload Generation**: `/dailyftp_YYYYMMDD.txt` created at 09:30 AM IST transition.
- [ ] **FTP Transmission**: Uploads daily SWD/KWD file over FTP (`AT+CFTPSPUTFILE`).
- [ ] **Firewall Fallback (Error 9)**: If FTP port is blocked, system migrates `/ftpunsent.txt` to `/unsent.txt` for HTTP POST delivery.

---

### Phase 8: Device Health Reporting (GPRS Health)
- [ ] **Health Report Trigger**: Scheduled health report fires successfully (Daily at 11 AM or 15-min test mode).
- [ ] **Health JSON Payload**: Contains clean station name, battery voltage, solar voltage, CSQ, heap memory, awake duration, and reset reason.
- [ ] **TCP Connection Window**: Health TCP socket connects and receives response within 25s timeout cap.

---

### Phase 9: Deep Sleep & Power Management
- [ ] **Sleep Entry**: ESP32 enters Deep Sleep (`esp_deep_sleep_start()`) between 15-minute sampling cycles.
- [ ] **ULP Pulse Counting**: Rain pulse counting remains active in ULP coprocessor during deep sleep.
- [ ] **Sent Mask Reconstruction**: `reconstructSentMasks()` restores sent slot bitmasks (`diag_sent_mask_cur`) correctly upon `DEEPSLEEP_RESET`.
- [ ] **Sleep Current**: Sleep mode current draw measured at battery terminals (< 2.0 mA).

---

### Phase 10: Over-The-Air (OTA) Firmware Update Routine
- [ ] **OTA Command Trigger**: OTA update command received over HTTP/TCP command polling interface.
- [ ] **Range GET Chunking**: OTA downloader requests binary chunks using `Range: bytes=start-end` HTTP headers.
- [ ] **Offset Validation**: Content-Range response offset matches requested offset; invalid range triggers retry.
- [ ] **Partition Verification**: `Update.write()` writes data directly; `Update.end()` verifies image checksum prior to reboot.

---

## 📊 3. Test Execution Summary

| Phase | Test Category | Status (PASS / FAIL / SKIP) | Engineer Comments / Logs |
| :---: | :--- | :---: | :--- |
| **1** | Flashing & Boot Audit | | |
| **2** | LCD & UI Interface | | |
| **3** | Sensor Measurements | | |
| **4** | Cellular Registration | | |
| **5** | Live HTTP Transmission | | |
| **6** | Backlog Queue & Flush | | |
| **7** | Daily FTP / Migration | | |
| **8** | Health Diagnostics Report| | |
| **9** | Power & Deep Sleep | | |
| **10**| OTA Firmware Update | | |

---

## ✍️ 4. Test Sign-Off & Verdict

- **Final Test Verdict**: `[  ] PASSED - Ready for Deployment` &nbsp;&nbsp;&nbsp; `[  ] REJECTED - Issues Found`
- **Reviewed By (Lead Engineer)**: ___________________________
- **Date**: ___________________
