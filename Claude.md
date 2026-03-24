# 🦅 SPATIKA AIO 9.5.0 - Ultimate Master Roadmap (v5.68+)

This file consolidates all structural, security, and hardware stability flaws across the entire AIO9_5.0 project, reorganized by our execution phases (Firmware First, Server Second).

---

## 🚨 PHASE 1: FIRMWARE DATA LOSS & STABILITY (Execute First!)
*These vulnerabilities cause total field node data loss or force hardware into infinite reboot loops, entirely independent of the Contabo server.*

1. **The Healer Reboot Death Loop**: `healer_reboot_in_progress` is wiped on line 1 of boot, rendering it totally useless. Coupled with the aggressive 2-hour offline limit (8 fails), rural BSNL units trapped in low-signal areas will endlessly hard-reboot themselves. 
   *(Fix: Remove flag reset block from top of setup; increase reboot threshold to 24 consecutive hours of offline failures `diag_consecutive_http_fails >= 96`).* **[✅ FIXED]**
2. **SPIFFS Silent Data Auto-Drop**: The SPIFFS 512-byte limit guard at the top of the GPRS task logs "Pruning..." but actually just aborts the write sequence completely, skipping the 15-minute slot permanently. 
   *(Fix: Implement true circular logging using `SPIFFS.remove(oldest file)` and increase guard to 4096 bytes).*
3. **SPIFFS Gap-Fill Silent Black-Hole**: During the `FILLGAP` data recovery loop, the `unsent.txt` file handler is opened but never checked for `NULL`. If the file fails to open, thousands of recovery records are blindly printed to nowhere and permanently lost.
   *(Fix: Add an explicit `if (!unsent)` abort check).*
4. **ULP Wind Speed Extreme Inflation**: The ULP Anemometer pin completely lacks software debounce logic. A 3ms physical magnetic-switch vibration registers as 3 wind rotations due to the 1ms execution loop. 
   *(Fix: Duplicate the `U_DEBOUNCE_CNT` assembly block from the Rainfall gauge into the Wind Speed block).*
5. **UART Heap Fragmentation on Response**: `waitForResponse()` builds immense JSON strings byte-by-byte using `response += c`. This forces the ESP32 to rapidly reallocate dynamic memory up to 500 times per command, heavily fragmenting the heap until `malloc` fails and the SIM module suddenly returns phantom "ERRORs". 
   *(Fix: Use a fixed static `char buf[2048]`).*
6. **UART Multi-Threading Race Condition**: The `modemMutex` briefly unlocks during gap-fill transmissions. If the LCD is used to successfully trigger a manual SMS/GPS send in that microsecond-window, it crushes the active HTTP UART stream. 
   *(Fix: Implement an `eHttpBusy` state variable so the LCD queues commands until the modem is safely released).*
7. **Battery Brownout GPRS Guard**: GPRS modules draw 2+ Amp transmission bursts. Activating the SIM7600 when the LiPo is deep-discharged under 3.5V instantly causes a hardware power collapse (Reset 15). 
   *(Fix: Bypass GPRS completely and write purely to SPIFFS if `bat_val < 3.5V`).*

---

## 🚨 PHASE 2: SERVER API SECURITY (Python / FastAPI)
*These vulnerabilities allow remote server wipe-outs and full command injection against the Contabo dashboard.*

8. **CSRF "1-Click" Server Wipe**: `/cmd/{stn_id}/DELETE_DATA` and `/delete/{stn_id}` are unauthenticated `GET` routes. A malicious URL loaded dynamically on any site you visit while authenticated to the dashboard will instantly factory-reset a field node. 
   *(Fix: Convert all destructive endpoints to POST).*
9. **The "Empty Password" Admin Bypass**: `os.getenv("ADMIN_PASS", "")` in `auth.py` defaults to `""`. If the `.env` file is missing, typing `admin` with a blank password grants total root server access to anyone on the internet.
   *(Fix: Crash FastAPI on boot if ADMIN_PASS is not strictly set in the environment).*
10. **Firmware Path Traversal & OOM Crash**: The individual OTA upload route blindly trusts the `{stn_id}` parameter without regex sanitization, allowing `../../etc/passwd` overwriting. Furthermore, it buffers infinite file sizes into RAM, easily crashing the Contabo VPS host if someone uploads a 2GB file. 
    *(Fix: Regex sanitize `[^a-zA-Z0-9]`, loop-stream uploads, enforce 2.5MB size limit).*
11. **Hardcoded API Keys / FTP Credentials**: All Spatika and Government (`KSNDMC`, `BIHAR_TRG`) passwords and keys are written in plaintext inside the `.ino` payload builders. Anyone intercepting a plain-HTTP OTA binary (over unencrypted 2G) gets all the keys. 
    *(Fix: Move to `user_config.h` for compile-time stripping).*
12. **Raw SQL Exception Leaks**: Uncaught database crashes dump raw SQLAlchemy code structures and table names directly into the browser. 
    *(Fix: Implement generic HTTP 500 template wrappers).*

---

## 🟡 TIER 3: RELIABILITY & CLEANUP (v5.69 Target)

13. **JSON Array Truncation Recovery Trap**: The 1280-byte `jsonBody` on the ESP32 doesn't have enough headroom. Expand buffer securely to `1536` bytes.
14. **Python In-Memory Session Devourer**: User login session tokens are stored in `SESSIONS = {}` with zero expiry logic. 
15. **Atomic RTC Time Synchronization**: `current_hr` and `current_min` are read from the RTC and written independently across threads. Needs a Mutex to prevent ghost 12:59 boundary timestamps.
16. **Wind Spike Filter**: Lower the instantaneous logic filter from `100 m/s` (Category 5 Hurricane) to `35 m/s` to organically filter out long wire cable bounce on the TWS configurations.
17. **Stale Zombie TCP Disconnects**: Clear `diag_http_fail_reason` securely at the start of the `send_http_data` sequence so a stale error string left over from 15 minutes ago doesn't unnecessarily trigger the highly aggressive AT nuke sequence.
