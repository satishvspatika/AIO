# AIO ESP32 Data Flow & Server Specification (v5.76+)

This document maps out the system configurations, exact packet structures, byte lengths, and server parameters for the AIO (All-In-One) ESP32 telemetry stations.

> **Note**: The exact byte footprints mapping to `RECORD_LENGTH` parameters are mathematically hardcoded to allow ultra-fast SPIFFS `seek()` traversing during backlog push cycles.

---

## 1. System Configurations & File Structures

| SYSTEM | Type | Description | Supported `UNIT_CFG` Options |
| :--- | :--- | :--- | :--- |
| **0** | **TRG** | Telemetry Rain Gauge (Rainfall/Battery only) | `KSNDMC_TRG`, `BIHAR_TRG`, `SPATIKA_GEN` |
| **1** | **TWS** | Telemetry Weather Station (Temp, Hum, Wind, RF) | `KSNDMC_TWS`, `KSNDMC_TWS-AP` |
| **2** | **TWS-RF** | TWS + Independent RF Tipping Bucket Module | `KSNDMC_ADDON`, `SPATIKA_GEN` |

### Internal Files Tracking
1.  **`/unsent.txt`**: Active HTTP staging database (managed via byte-offsets in `/unsent_pointer.txt`). Capped at 300 records manually via `pruneFile()` when deep sleep reboots happen.
2.  **`/ftpunsent.txt`**: Active Bulk FTP staging cache.
3.  **`/lastrecorded_<Stn>.txt`**: Mirror cache of the last physical reading, used strictly by the health/HTTP sync mechanism to ensure no old data is forwarded.
4.  **`/<StationName>_YYYYMMDD.txt`**: Permanent daily log of all CSV records natively collected locally by the SD Card backup or SPIFFS module.

---

## 2. HTTP Server Details & Configurations

The primary endpoint strictly utilizes `application/x-www-form-urlencoded` headers to flush 15-minute slot data instantly.

*   **Endpoint (SYSTEM 0 TRG):** `rtdas.ksndmc.net:80/trg_gprs/update_trg_data`
*   **Endpoint (SYSTEM 1/2 TWS):** `rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2`
*   **Health Diagnostics URL:** `SERVER_IP/health`

### Live Payload Footprints
Depending on the `SYSTEM`, the string appended inside `scheduler.ino` guarantees identical length:

#### SYSTEM 0 (TRG)
*   **Total Constant Length (`RECORD_LENGTH_RF`):** 46 Bytes (Including `\r\n`)
*   **String Mapping:** `%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%04d,%05.2f\r\n`
    *   2-byte Sample No. (`02`)
    *   10-byte Date (`YYYY-MM-DD`)
    *   5-byte Time (`HH:MM`)
    *   5-byte Instant Rainfall (`II.II` or `%05.2f`)
    *   5-byte Cumulative Rainfall (`CC.CC` or `%05.2f`)
    *   4-byte Cellular Signal (`SSSY` or `-067`)
    *   5-byte Battery Float (`BB.BB`)

#### SYSTEM 1 (TWS)
*   **Total Constant Length (`RECORD_LENGTH_TWS`):** 53 Bytes (Including `\r\n`)
*   **String Mapping:** `%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%s,%s,%04d,%04.1f\r\n`
    *   2-byte Sample No. (`02`)
    *   10-byte Date
    *   5-byte Time
    *   5-byte Instant Temperature (`TTT.T` format e.g., `029.5`)
    *   5-byte Humidity (`HHH.H` format e.g., `068.1`)
    *   5-byte Wind Speed (`WW.WW` format e.g., `00.00`)
    *   3-byte Wind Direction (`DDD` e.g., `187`)
    *   4-byte Cellular Signal
    *   4-byte Battery Float (`BB.B` format e.g., `04.0`)
*   **Example Output:** `02,2026-04-03,09:15,029.5,068.1,00.00,187,-067,04.0\r\n`

#### SYSTEM 2 (TWSRF)
*   **Total Constant Length (`RECORD_LENGTH_TWSRF`):** 60 Bytes
*   **String Mapping:** Has the identical TWS string structure with an extra 6-byte rainfall cumulative block appended inside the string footprint.

---

## 3. FTP Server Details & Conventions

The secondary bulk-transfer protocol bypasses byte-level formatting checks for speed, wrapping identical string fields into chunks.

*   **Server Endpoint:** Primarily `bsnl.ksndmc.net` over Port `21`
*   **Credentials:** Exported via `FTP_USER` and `FTP_PASS` constants in headers.

### File Naming (`AT+FTPPUTNAME`)
1.  **TWS Mode:** `/TWS_<stnId>_YYMMDD_HHMM00.kwd` (Example: `TWS_001941_260404_084500.kwd`)
2.  **TWS-RF Mode:** `/TWSRF_<stnId>_YYMMDD_HHMM00.kwd`
    * *Note:* Extensions automatically switch from `.kwd` to `.swd` for specific configurations labeled `SPATIKA_GEN`.
3.  **Daily Historic Rollup:** Extracted into `/dailyftp_YYYYMMDD.txt`

### FTP Payload Footprint
Instead of strictly relying on fixed byte parameters, the server utilizes a semi-colon delimited file.

*   **String Mapping:** Uses `%s;%04d-%02d-%02d,%02d:%02d;%s;%s;%s;%s;...` 
    *   The `stnId` floats at the front.
    *   Values precisely sync up to their live HTTP counterparts dynamically (for example: Battery is passed as `%04.1f` for robust precision match without breaking the line).
*   **Example Output:** `001941;2026-04-03,08:45;029.9;069.4;00.00;046;-067;04.0\r\n`
