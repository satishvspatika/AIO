# Release Notes: v5.40 (Feb 27, 2026)

## 🎯 Overview

**GPRS Flow Optimization — Reliability, Power Efficiency & Sensor Accuracy**

Version 5.40 is a major field-reliability and power optimization release. The primary goal was to reduce per-cycle active time (i.e., the time the modem, CPU, and sensors are running), eliminate a known deep sleep hang after HTTP upload, and resolve intermittent incorrect wind direction readings. All changes are backward-compatible with existing field deployments.

**Key Metrics vs v5.39:**
| Metric | v5.39 | v5.40 |
|---|---|---|
| Modem boot time | ~10s (blind wait) | ~4-5s (fast poll) |
| SIM detection | ~2s (blind wait) | ~1s (adaptive poll) |
| SMS check frequency | Every 15 min | Once per hour (Minute 0 slot) |
| Deep sleep after upload | ❌ Hung | ✅ Immediate |
| Modem shutdown | Hard GPIO kill always | ✅ Graceful `AT+CPOWD=1` on success |
| Wind Dir near North | `NA` / `000` (wrong) | ✅ Correct degrees (e.g. 348°) |

---

## ✨ New Features & Improvements

### 1. **Fast Modem Boot (Active Polling)** ⚡
- **Before:** 10-second blind `vTaskDelay` waiting for modem to wake up.
- **After:** Active `AT` polling loop (up to 15 iterations × 400ms). Modem typically responds in 3–5 seconds. Loop exits immediately on `OK`.
- **Saving:** ~5–7 seconds per wake cycle.

### 2. **Adaptive SIM Polling** 📶
- **Before:** 2-second blind wait for SIM to be ready.
- **After:** Polls `AT+CPIN?` every 1 second (up to 10 iterations). SIM typically ready in ~1 second.
- **Saving:** ~1 second per wake cycle.

### 3. **Selective Hourly SMS Check** 💬
- **Before:** SMS check (`send_sms()`) ran every 15-minute wake cycle, consuming ~15 seconds of active modem time each time.
- **After:** SMS check runs **only once per hour** at the scheduled Minute 0 slot (e.g., 08:00, 09:00). Uses `record_min == 0` as the condition so it fires reliably even if registration takes time and the clock drifts past :00.
- **Saving:** ~15 seconds × 3 out of 4 cycles = **45 seconds per hour** saved.
- **Manual SMS trigger (keypad):** Preserved — `sync_mode = eSMSStart` from keypad still works at any time.

### 4. **Fast HTTP Retry for Transient Errors** 🔄
- **Before:** On any HTTP failure (706/713/714), the system performed a full bearer reset (`AT+SAPBR=0,1`) before retrying. This costs ~10 seconds.
- **After:** Transient errors (706/713/714) skip the bearer reset and retry immediately. A full bearer reset is only performed for persistent failures.
- **Saving:** ~10 seconds on transient network glitches.

### 5. **Reliable Deep Sleep Transition** 💤
- **Problem:** When the hourly SMS check was skipped, `sync_mode` remained stuck at `eHttpStarted`. The main `loop()` waits for `eHttpStop` or `eSMSStop` before entering deep sleep — causing the device to hang indefinitely.
- **Fix:** Added `sync_mode = eHttpStop` explicitly in the SMS-skip path. Now regardless of whether SMS is sent or skipped, the system always reaches a terminal state and enters deep sleep.

### 6. **Graceful Modem Shutdown** 🔌
- **Before:** `graceful_modem_shutdown()` had a logic error — it treated `sync_mode == eHttpStop` (a **success** state) as an "unstable session," causing it to always do a hard GPIO power-cut instead of a clean shutdown.
- **Fix:** Removed the erroneous condition. Now:
  - **Successful session:** `AT+CPOWD=1` → waits for `NORMAL POWER OFF` → then GPIO 26 LOW.
  - **Registration failure session:** Hard GPIO power-cut (correct behavior for a broken session).
- **Impact:** Kinder to the SIM808/A7672S module hardware over thousands of 15-min cycles.

### 7. **Wind Direction — Smart Disconnection Detection** 🌬️
- **Before:** The sensor was marked as "missing" if the ADC reading was near 0 or 4095 (the "rail zones"). Since North (0°/360°) physically maps to these exact ADC values, wind readings near North were falsely flagged as `NA` and forced to `000` in the payload.
- **After:** Uses **ADC spread** (max − min across 10 samples) to distinguish:
  - **Real sensor near North:** Small natural ADC noise → spread ≥ 2 → valid degrees reported.
  - **Disconnected cable at GND:** Perfectly flat at 0 → spread == 0 AND mean == 0 → flagged after 30 consecutive seconds.
- **Disconnected reporting:** Reports `000` (numeric, not `NA`) so the server can handle it.
- **Smooth 359°→0° transition:** Mathematically guaranteed — `(4095 × 360) / 4096 = 359`, then the next rotation increments to `0`. No artificial gap.

---

## 🔧 Bug Fixes

### 1. **Deep Sleep Hang After Upload** 🐛
- **Problem:** Device uploaded data successfully but never entered deep sleep.
- **Root Cause:** Missing `sync_mode` state transition when SMS check was skipped.
- **Fix:** Explicit `sync_mode = eHttpStop` added to the skip path.

### 2. **Wind Direction Reads `NA` Near North** 🐛
- **Problem:** Vane pointing 340°–360° triggers falsely as "disconnected."
- **Root Cause:** Old fault zone was `ADC <= 25 OR ADC >= 4090`, which matches valid North-range readings.
- **Fix:** Replaced with spread-based detection; only flags after 30s of perfectly flat ADC at 0.

### 3. **Graceful Shutdown Always Hard-Killing Modem** 🐛
- **Problem:** `sync_mode == eHttpStop` was incorrectly included in the "unstable session" check, causing the modem to always receive a hard GPIO cut instead of `AT+CPOWD=1`.
- **Fix:** Removed the erroneous `sync_mode == eHttpStop` from the condition.

### 4. **Hourly SMS Skipped Due to Clock Drift** 🐛
- **Problem:** `current_min == 0` check sometimes missed the 00-minute window if registration took longer than 60 seconds (e.g., clock read `08:01` by the time check ran).
- **Fix:** Changed to `record_min == 0`, which uses the scheduled slot time (snapped to 15-min boundary) — immune to registration delays.

---

## 📋 Technical Details

### Modified Files
- `globals.h` — Version bumped to `5.40`
- `gprs.ino` — Fast boot polling, adaptive SIM check, transient HTTP retry, SMS check logic, deep sleep fix, graceful shutdown fix
- `windDirection.ino` — Full rewrite of fault detection using ADC spread

### Code Size Impact (8MB Flash)
- **Program:** 1,210,000 bytes (7% of 16MB)
- **RAM:** 54,100 bytes (16% of 328KB)

### Power Impact (Estimated Per Day)
- Each 15-min cycle saves ~25–35 seconds of active modem time
- Over 96 cycles/day: **40–56 minutes** less modem-on time per day
- Directly translates to extended battery life in off-grid deployments

---

## 📦 Release Contents

Pre-compiled binaries for all 7 system configurations (8MB Flash):

| Folder | System | Network | Type |
|---|---|---|---|
| `KSNDMC_TRG_8mb` | SYSTEM=0 | KSNDMC | Rain Gauge |
| `BIHAR_TRG_8mb` | SYSTEM=0 | Bihar Govt | Rain Gauge |
| `SPATIKA_TRG_8mb` | SYSTEM=0 | Spatika | Rain Gauge |
| `KSNDMC_TWS_8mb` | SYSTEM=1 | KSNDMC | Weather Station |
| `KSNDMC_TWS_AP_8mb` | SYSTEM=1 | KSNDMC | Weather Station + Pressure |
| `KSNDMC_ADDON_8mb` | SYSTEM=2 | KSNDMC | TWS-RF Add-on |
| `SPATIKA_ADDON_8mb` | SYSTEM=2 | Spatika | TWS-RF Add-on |

Each folder contains: `firmware.bin`, `fw_version.txt`, `build_info.txt`

---

## 🧪 Testing Performed

- ✅ Verified on WS0034 (TWSRF9-GEN, HDC1080, Airtel IoT SIM, A7672S modem)
- ✅ Full 08:00–09:00 cycle observed — boot, register, upload, sleep
- ✅ Wake time consistent at ~75–90 seconds (vs ~120s in v5.39)
- ✅ Wind direction 348° correctly reported (previously would have been `000`)
- ✅ Deep sleep entered immediately after upload — no hang
- ✅ Graceful `AT+CPOWD=1` confirmed in logs
- ✅ Hourly SMS check at `record_min == 0` — robust to registration delays

---

## 🔄 Upgrade Path

### From v5.39 or earlier:
- **Direct upgrade** — Flash `firmware.bin` using `esptool` or Arduino IDE
- **No NVS or SPIFFS data loss** — all station config, APN cache, and data files are preserved
- **No hardware changes** required

---

## ⚠️ Known Limitations

1. **BME280 not used** — This build has `ENABLE_PRESSURE_SENSOR 0`. The BME280 is detected during boot but the pressure task is not spawned.
2. **SD Card** — If not inserted, system logs to SPIFFS only (expected behavior).
3. **Wind Direction disconnection** requires 30 seconds of flat ADC before flagging — a briefly disconnected cable during a cycle may not be caught.

---

**v5.40 is production-ready for TWS-RF (SPATIKA_GEN) deployments.** 🚀
