# Spatika & KSNDMC HTTP/FTP Data Transmission Specifications

**Revision:** April 2026 (v5.75 "Diamond-Elite" Hardened)
**Application:** ESP32 AIO All-In-One Firmware

---

## 🚀 The "Invisibility" Fix Summary
Data was previously "invisible" on the server despite a successful "PUT" by the modem because global firmware formatting was causing **Column Shifts**. Most older server-side aggregate scripts are strictly **Fixed-Column** or **Fixed-Byte-Length** parsers. Even a single extra decimal place or missing sign shifts all subsequent fields, causing the parser to reject the record as "Corrupted."

---

## 0. TRG SYSTEM (SYSTEM 0: HTTP Only)
*No FTP required for TRG. All data sent via HTTP REST/POST.*

### A. BIHAR_TRG (JSON Format)
*   **URL:** `http://rtdasbmsk.spatika.net:8085/Home/UpdateTRGData`
*   **Specimen Record (Absolute Truth):**  
    `{"StnNo":"001920","DeviceTime":"2026-04-01 10:00:00","RainDP":"000.0","RainCuml":"001.2","BatVolt":"04.1","SigStr":"-051","ApiKey":"bmsk1234"}`

### B. KSNDMC_TRG (URL-Encoded Format)
*   **URL:** `http://rtdas.ksndmc.net/trg_gprs/update_data_sit_v3`
*   **Specimen Record (Absolute Truth):**  
    `stn_id=001921&rec_time=2026-04-01,10:00&rainfall=000.25&signal=-051&key=pse2420&bat_volt=04.1&bat_volt1=04.1`

---

## 1. SPATIKA_GEN (SYSTEM 2: TWS-RF)
*Highly sensitive to internal column offsets.*

*   **Filename Pattern:** `TWSRF_<STN_ID>_<YYMMDD>_<HHMMSS>.swd`
*   **Required Total Length:** **63 Bytes** (including `\r\n`)
*   **Specimen Record (Absolute Truth):**  
    `WS0035;2026-04-01,10:00;000.0;028.5;054.2;01.2;180;-051;04.10`
*   **Core Logic:**
    *   **Wind Speed:** Strictly **Width 4** (`%04.1f` -> `01.2`).
    *   **Battery:** Strictly **Width 5** (`%05.2f` -> `04.10`).
*   **Column Map (Standard Truth):**
    *   **Wind Direction:** Must start exactly at **Character 48**.
    *   **Signal:** Must start exactly at **Character 52**.
    *   **Battery:** Must start exactly at **Character 57**.

> [!IMPORTANT]  
> If Wind Speed is sent as `01.25` (5 chars), the Wind Direction starts at Character 49, and the Spatika parser will discard the entire file.

---

## 2. KSNDMC_TWS (SYSTEM 1: TWS)
*Strictly enforces the 57-byte multiplier.*

*   **Filename Pattern:** `TWS_<STN_ID>_<YYMMDD>_<HHMMSS>.kwd`
*   **Required Total Length:** **57 Bytes** (including `\r\n`)
*   **Specimen Record (Absolute Truth):**  
    `003736;2025-08-21,10:00;024.6;100.0;00.0;021;-079;04.43`
*   **Hardening Rule:** Battery must use **Precision 2** (`%05.2f`). If sent as `04.4` (4 chars), the server rejects the 56-byte packet.

---

## 3. KSNDMC_ADDON (SYSTEM 2: TWS-RF)
*Strictly enforces the 63-byte multiplier.*

*   **Filename Pattern:** `TWSRF_<STN_ID>_<YYMMDD>_<HHMMSS>.kwd`
*   **Required Total Length:** **63 Bytes** (including `\r\n`)
*   **Specimen Record (Absolute Truth):**  
    `000297;2025-08-21,09:45;000.0;+26.8;070.7;00.2;260;-079;13.58`
*   **Hardening Rules:**
    *   **Temperature:** Must use **Width 5** (`%05.1f`) to allow for the sign/space column.
    *   **Wind Speed:** Strictly **Width 4** (`%04.1f`).
    *   **Battery:** Must use **Width 5** (`%05.2f`).

---

## 📦 Filename Harmonization Logic
The firmware now automatically builds the filenames based on the `UNIT` keyword and the `SYSTEM` ID to eliminate manual naming errors:

| Config | Prefix Added | Extension Added |
| :--- | :--- | :--- |
| **SYSTEM 0 (TRG)** | `TRG_` | `.kwd` (KSNDMC) or `.txt` |
| **SYSTEM 1 (TWS)** | `TWS_` | `.kwd` (KSNDMC) |
| **SYSTEM 2 (TWS-RF)** | `TWSRF_` | `.swd` (Spatika) or `.kwd` (KSNDMC) |

---

## 🛠️ Validation Testing
To verify compliance on the hardware, check the serial debug log during an `FTP_BACKLOG` or `FTP_DAILY` command:
1.  **Station ID Padding:** Confirm 4-digit IDs become 6-digits (e.g. `1080` -> `001080`).
2.  **Record Length:** Use a character count tool (or manual count) specifically on the `ftpappend_text` log output. Ensure it matches the **57/63 byte** target exactly.

---

## 🔑 Credential Registry (The Absolute Truth)

### 1. KSNDMC Servers (Karnataka)
| Configuration | FTP Server Host | Username | Password | Notes |
| :--- | :--- | :--- | :--- | :--- |
| **KSNDMC TWS (S1)** | `ftp1.ksndmc.net` | `tws_spatika_v2` | `twssp#987` | Port 21 |
| **KSNDMC ADDON (S2)**| `ftp1.ksndmc.net` | `twsrf_spatika_v2`| `sittao#10` | Port 21 |
| **Daily Reports** | `ftp1.ksndmc.net` | `trg_spatika_v2@ksndmc.net` | *(Secure)* | TRG Daily |
| **DMC General** | `89.32.144.163`| `dota_dmc` | `airdata2016` | Fallback Path |

### 2. Spatika Gen Servers
| Configuration | FTP Server Host | Username | Password | Notes |
| :--- | :--- | :--- | :--- | :--- |
| **Spatika TWS (S1)** | `ftp.spatika.net` | `tws_gen` | `spgen123` | Port 21 |
| **Spatika TWS-RF (S2)**| `ftp.spatika.net` | `twsrf_gen` | `spgen123` | Port 21 |

---
*End of Protocol Specification.*
