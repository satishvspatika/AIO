# 🦅 Spatika AIO 9.5.0 - Release Notes v5.68
**Release Date:** March 24, 2026
**Architecture:** ESP32-D0WD-V3 (8MB Flash) / FastAPI Server Backend
**Focus:** High-Severity Server Hardening, Firmware OTA Stability, Memory Leak Resolution, and Global Timekeeping Atomicity.

This major milestone explicitly resolves all Security, Data-Integrity, and Network Stabilization flaws flagged in the Phase 1, Phase 2, and Phase 3 audits. 

---

## 🔒 PHASE 2: SERVER & CLOUD API SECURITY (FastAPI)
*Crucial backend restructuring to prevent unauthorized Remote Wipes, Host Resource Depletion, and Session Leaks.*

* **CSRF "One-Click" Server Wipe Neutralized:** 
  Converted highly destructive endpoints (`/cmd/{stn_id}/DELETE_DATA` and `/clear_locks`) from insecure `GET` methodologies to strict `POST` validations. This permanently blocks Cross-Site Request Forgery architectures from maliciously resetting field hardware via weaponized browser links.
* **OTA Upload Path Traversal & RAM Overflow Guard:**
  Rewrote the firmware update handler in `ota.py`. The endpoint now natively strips malicious characters (`../../`) from payloads, actively enforces a `.bin` extension wrapper, and employs secure multi-part chunked streaming to prevent 2GB dummy files from crashing the Contabo VPS host memory. 
* **The "Ghost Admin" Authentication Patch:** 
  Hard-locked the administrative bypass vulnerability. The FastAPI execution engine now immediately throws an operating system exception and crashes at boot if a secure `ADMIN_PASS` is not explicitly declared in the host `.env` file, physically rejecting blank `""` dictionary injections.
* **Persistent Authentication Memory Leak Plugged:**
  Re-architected `auth.py` to establish temporal tokens. The global `SESSIONS` memory dictionary now automatically evicts stale JWT tokens after exactly 7 days. This stops the infinite RAM ballooning caused by orphaned user connections.
* **Blind SQL Exception Wrapper:**
  Implemented formal `error.html` HTTP 500 template shielding. If a database timeout occurs, the server renders a generalized UI rather than leaking internal SQLAlchemy architectures and RAW node schemas into the client browser.
* **Firmware Hardcoding Migration:** 
  All high-security Spatika, KSNDMC, and BIHAR API keys and FTP node passwords have been extracted from `gprs.ino` and compartmentalized inside the C++ preprocessor (`user_config.h`). This ensures variables are physically stripped prior to HTTP transmission, protecting intercepted raw string values.

---

## ⚙️ PHASE 3: FIRMWARE RELIABILITY & EDGE-CASE HARDENING (ESP32)
*Deep-Engine modifications to preserve payload integrity, resolve I2C Bus contention, and natively drop dead-code from the GCC Compiler.*

* **"Zero-Day" Global Time Atomicity (Ghost Stamps):**
  Deployed a heavily preemptive Thread Spinlock (`portMUX_TYPE rtcTimeMux`). All master temporal parameters (`current_hr`, `current_min`, etc.) are now assigned with absolute core atomicity inside `rtcRead.ino`. This utterly eradicates the "12:59 boundary anomaly" where context-switches injected scrambled timestamps into the SD logs. 
* **I2C / OTA Flash Contention Immunity:** 
  Engineered explicit `while (ota_silent_mode)` spin-waits inside the autonomous sensor task loops (`windDirection.ino`, `bme280.ino`). The hyper-agressive background I2C/ADC hardware interrupts now natively sleep during active OTA ROM flashes, ending mid-update driver collisions.
* **Modem Bearer Recovery Task-Starvation Fix:** 
  Created a dedicated `bearer_recovery_active` flag to isolate the 80-second modem radio nuke timeouts (`CFUN=0/1`). This replaces the global `ota_silent_mode` lock, thereby allowing high-frequency ULP pulse counters (Rain & Wind) to continue capturing organic weather data while the GPRS stack sequentially restarts itself.
* **JSON Array Headroom Expansion:** 
  The raw memory buffer serving payload telemetry (`jsonBody`) was systematically increased from deeply constrained `1280` array blocks to `1536` bytes. This prevents packet truncation and buffer-overruns when deploying heavily saturated TWS+RF node configurations.
* **Organic Wire-Bounce Filtration:**
  Dropped the `windSpeed.ino` rapid-spike hardware filter to an airtight `35.0 m/s`. This natively rejects phantom wire-capacitance triggering frequently observed on extended rural cable towers, without corrupting legitimate cyclonic event sequences.
* **Ouroboros Network Nuke Loop Arrested:**
  Injected an explicit string nullification (`diag_http_fail_reason[0] = '\0'`) at the exact wakeup moment of `send_http_data()`. Stale localized timeouts remaining in the cache from 15 minutes ago will no longer deceive the `GPRS.ino` engine into executing a false-positive bearer reboot on healthy networks.
* **OTA Chunking Ghost-Session Extirpated:**
  Surgically removed redundant `session_active = false;` assertions from `OTA_Chunk_Loop` and restored unindented control flow. Assures pristine One-Chunk-One-Session sequencing.
* **Dead-Code Elimination (DCE) on Health Infrastructure:** 
  Completely encapsulated the 500-line `send_health_report()` functionality, the `eStartupGPS` bypass sequence, and the manual `FLD_SEND_HEALTH` physical LCD keys strictly within `#if ENABLE_HEALTH_REPORT == 1`. Switching this master toggle to `0` now triggers aggressive GC Optimizer dead-code elimination, permanently recovering massive banks of FreeRTOS Dynamic RAM and ensuring absolute DB port silence.
