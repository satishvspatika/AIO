# Implementation Plan - Fix Sensor Payload Flatlining and Live Variable Overwrite

## Overview
This plan fixes the issue where identical/flatlined sensor values (e.g. Temperature 28.5 °C, Humidity 62 %, Wind Speed 0 m/s) are recorded to SPIFFS/SD card and transmitted to the server across consecutive 15-minute intervals, despite the LCD displaying live, changing sensor data.

---

## Cause Analysis
1. **Live Variable Overwrite**: In `scheduler.ino` (lines ~1205-1228 and ~1285-1310), when the 15-minute scheduler opens the SPIFFS daily log file to inspect the previous record, it parses `last_instTemp`, `last_instHum`, `last_AvgWS`, and `last_instWD`.
2. **Global Variable Mutation**: The scheduler then assigns these historical values into the global active live variables:
   ```cpp
   cur_avg_wind_speed = last_AvgWS;
   temperature = last_instTemp;
   humidity = last_instHum;
   windDir = (int)last_instWD;
   ```
3. **Broken Gap Interpolation**: Because the live variables (`temperature`, `humidity`, `cur_avg_wind_speed`) get overwritten with historical file values (`last_instTemp`, `last_instHum`, `last_AvgWS`), gap-filling interpolation (`end_t - start_t`) evaluates to `0`, causing all interpolated or backlog records to flatline to identical values.
4. **Asynchronous LCD**: The LCD screen updates live every 5 seconds directly from `tempHum.ino` on Core 1, which explains why the LCD shows fresh live data while the scheduler records and sends stale data.

---

## Proposed Changes

### Core Firmware (`scheduler.ino`)

#### [MODIFY] [scheduler.ino](file:///Users/satishkripavasan/Documents/Arduino/ESP32_NEW_DESIGN/ALL_IN_ONE/AIO9_5.0/scheduler.ino)

1. **Remove Live Variable Overwrites in File Parsing Blocks**:
   - In `SYSTEM == 1` block (~lines 1205-1210 and ~lines 1224-1228):
     - Remove `cur_avg_wind_speed = last_AvgWS;`
     - Remove `temperature = last_instTemp;`
     - Remove `humidity = last_instHum;`
     - Remove `windDir = (int)last_instWD;`
   - In `SYSTEM == 2 || SYSTEM == 3` block (~lines 1285-1290 and ~lines 1305-1310):
     - Remove `cur_avg_wind_speed = last_AvgWS;`
     - Remove `temperature = last_instTemp;`
     - Remove `humidity = last_instHum;`
     - Remove `windDir = (int)last_instWD;`
   - **Rationale**: `last_instTemp`, `last_instHum`, `last_AvgWS`, `last_instWD` should only serve as historical reference anchors for gap filling (`q < sampleNo`), and must never overwrite the active live sensor readings in RAM.

2. **Preserve Sensor Rescue Protocol Counters**:
   - Ensure `temp_same_count` and `hum_same_count` are reset appropriately only when sensor data varies, preserving the 40-slot Constant Value (CV) freeze detection logic.

---

## Verification Plan

### Automated / Compilation Checks
- Run the build/compile script to verify zero compilation errors across target system configurations (`SYSTEM == 0`, `SYSTEM == 1`, `SYSTEM == 2`, `SYSTEM == 3`):
  ```bash
  ./compile.sh
  ```
- Run `build_all_configs.py` if available to ensure all release build variants compile cleanly.

### Manual / Field Verification
- Monitor serial debug output during a 15-minute interval cycle:
  - Confirm `--- Sensor Data Snapshot ---` prints live sampled values.
  - Confirm SPIFFS file parsing updates historical anchors (`last_instTemp`, etc.) without mutating `temperature` or `humidity`.
- Verify that server payloads and SPIFFS logs contain fresh, dynamic sensor readings across consecutive slots.
