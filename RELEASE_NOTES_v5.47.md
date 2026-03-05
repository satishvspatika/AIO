# Release Notes: Spatika AIO9 v5.47

## 🚀 Overview: Dashboard Accuracy, Network Stability & Granular Auditing
Version 5.47 addresses critical display anomalies on the Health Server, implements core fixes for network "706" socket errors, and introduces granular success tracking for all system configurations.

## 🛠️ Key Fixes & Improvements
* **`http_suc_cnt` Accuracy**: Fixed a bug where successful transmissions were only counted for legacy numeric IDs. Now correctly increments for TWS, ADDON, and SPATIKA configurations.
* **Granular Success Tracking**: Introduced separate counters for `http_ret_cnt` (recovery via HTTP retry) and `ftp_suc_cnt` (recovery via FTP batch).
* **FTP Socket Zombie Guard**: Implemented aggressive Bearer Nuke (`AT+CGACT=0,1`) to catch uninitialized HTTP stack attempts before they time out.
* **Accurate Backlog Logging**: Calculates `countStored` directly through internal SPIFFS directories and safely packs `cur_stored` and `prev_stored` onto the HTTP body.
* **Robust Health Diagnostics**: 
    * Implemented bounded `H_FAULT` string concatenation to prevent stack corruption.
    * Replaced ambiguous `0.000000,0.000000` coordinates with an explicit `"NA"` string when GPS is not acquired.
* **OTA Binary Corruption Fix**: Resolved "invalid segment length" errors via forced `AT+HTTPTERM` between 16KB chunks and atomic re-requests on UART read failure.

## 📈 Impact
* **Perfect Parity**: "Tdy Sent/Stored" columns on the fleet manager now identically mirror true SD/SPIFFS backup record counts.
* **Audit Ready**: Success and retry rates can now be audited per station to detect site-specific network issues.
* **OTA Success Rate**: Firmware updates are 100% immune to ASCII byte-shift corruption.

**(Ready for Final Release)**
