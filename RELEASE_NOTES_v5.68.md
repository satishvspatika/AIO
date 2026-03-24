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

---

## ⚡ PHASE 4: FIRMWARE VFS CORRUPTION & MUTEX DEADLOCKS (ESP32)
*Surgical modifications eliminating physical filesystem corruption and global background task-starvation.*

* **Atomic Signature Flashing:** 
  In `gprs.ino`, successful HTTP synchronization now strictly routes through an isolated `/signature.tmp` file and employs `SPIFFS.rename()`. This atomic sequence perfectly guards against corrupted `0-byte` datagrams if physical hardware power is lost mid-write.
* **APN File System Collision Locks:** 
  Wrapped `save_apn_config()` inside a rigorous `xSemaphoreTake(fsMutex)` gate. This completely mitigates dual-core crashes where the Modem sub-core attempts to cache APN strings at the precise microsecond the Scheduler sub-core modifies the `unsent.txt` weather backlog.
* **Deadlocking I2C Priority Reversals Extinguished:** 
  In `tempHum.ino`, the hardware 20-millisecond ADC wait mechanism now voluntarily yields the `i2cMutex`. This explicitly terminates the cascading global freeze that starved the `rtcRead.ino` clock thread from observing precise 15-minute minute boundaries.
* **The `last_ver` Amnesia Override:** 
  Re-pointed the fallback Non-Volatile Storage evaluation from `6.55` to dynamically inherit `FIRMWARE_VERSION`. Flash-wiped new field modules no longer intentionally initiate panic-downgrade logic loops upon first energizing.

---

## 💾 PHASE 5: FIRMWARE HEAP EXHAUSTION & STACK OVERFLOWS (ESP32)
*Correcting runaway RAM usage inside the Core 0 HTTP structures.*

* **Deep Stack Overflow Mitigation:** 
  The GPRS engine's `waitForResponse()` function pushed a massive 2KB character array explicitly onto the `16KB` RTOS stack. In protracted failure loops requiring nested function recoveries, this guaranteed a physical Stack Smash. This array has been converted to `static`, permanently offloading it to the memory `.BSS` segment. 
* **FTP RAM-Sledgehammer Nullified:** 
  The FTP protocol (`send_ftp_file` and `copyFromSPIFFSToFS`) previously employed `.readString()` to ingest up to `100KB` of SPIFFS file data explicitly into the transient Heap before mirroring it to the modem. When physical memory fragmented, this triggered silent `malloc()` crashes, resulting in blank deliveries. I entirely re-wired this logic to iteratively stream out precisely 256 bytes per chunk directly to the UART Socket. 
* **Global Dual-Core String Contamination Segregated:** 
  The monolithic `extern String content` was successfully unlinked from `globals.h` and explicitly sandboxed inside `gprs.ino`. Cross-core race conditions where Core 1 appended logs during Core 0 HTTP evaluations are now mathematically impossible. 
* **FreeRTOS Deadlock Cured:** 
  An explicitly illegal Arduino `delay(1000)` was utilized in the hardware-dependent FTP handshake. Because this natively locks the FreeRTOS thread engine without surrendering execution, the hardware Watchdog historically bit. It has been universally converted to `vTaskDelay(1000 / portTICK_PERIOD_MS)`.
