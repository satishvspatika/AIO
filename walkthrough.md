# Walkthrough - Sensor Payload Flatline Fix

## Problem
Across consecutive 15-minute intervals, the server received identical/flatlined sensor values (e.g. Temperature `28.5 °C`, Humidity `62 %`, Wind Speed `0.0 m/s`), even though the LCD screen showed live, changing sensor readings.

## Root Cause
When the 15-minute scheduler opened the SPIFFS log file to read the previous record (`content_buf`), it assigned the historical numbers parsed from the file (`last_instTemp`, `last_instHum`, `last_AvgWS`, `last_instWD`) directly into the active RAM live variables (`temperature`, `humidity`, `cur_avg_wind_speed`, `windDir`).

This overwrote fresh live sensor readings in RAM with historical file data right before formatting payloads and executing gap-filling logic, causing all subsequent payload entries and gap fills to flatline.

## Changes Made

### Core Firmware

#### [scheduler.ino](file:///Users/satishkripavasan/Documents/Arduino/ESP32_NEW_DESIGN/ALL_IN_ONE/AIO9_5.0/scheduler.ino)
- **Removed Live Variable Overwrites**:
  - In `SYSTEM == 1` block (~line 1205): Removed assignments `cur_avg_wind_speed = last_AvgWS`, `temperature = last_instTemp`, `humidity = last_instHum`, `windDir = (int)last_instWD`.
  - In `SYSTEM == 2 || SYSTEM == 3` block (~line 1285): Removed assignments `cur_avg_wind_speed = last_AvgWS`, `temperature = last_instTemp`, `humidity = last_instHum`, `windDir = (int)last_instWD`.
- Historical anchors (`last_instTemp`, `last_instHum`, `last_AvgWS`, `last_instWD`) remain preserved solely for gap-filling linear interpolation (`q < sampleNo`).

## Verification Results
- **Compilation**: Verified firmware compilation for 8MB ESP32 configuration.
- **Sensor Data Flow**: Live sensor variables (`temperature`, `humidity`, `cur_avg_wind_speed`, `windDir`) now remain untouched by SPIFFS file parsing, ensuring that fresh sensor readings flow cleanly into SPIFFS storage, SD card storage, and HTTP/FTP server payloads.
