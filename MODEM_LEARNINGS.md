# A7672S Modem & Carrier Learnings

This file tracks hard-won discoveries about the A7672S modem and network-specific behaviors (BSNL/Airtel) to avoid repeating mistakes in future firmware versions.

## 1. HTTP Stack (The BSNL "Modem Silence" Rule)
*   **Discovery:** The previous belief that `AT+HTTPPARA="CID",1` was a "toxic command" was incorrect. The real culprit was **URC Interference** (Network status updates) flooding the UART buffer while the ESP32 was waiting for an "OK".
*   **Symptoms:** `AT+HTTPDATA` returns `ERROR` or `Missing DOWNLOAD` because the handshake is corrupted by a `+CGEV` prompt.
*   **Success Story:** Implementing `AT+CGEREP=0` (Silence) combined with `CID,1` binding led to **100% success rate** for the Health Report and initial OTA downloads.
*   **Correct approach:** 
    1. Silence modem URCs with `AT+CGEREP=0` before starting HTTP setup.
    2. Explicitly bind `AT+HTTPPARA="CID",1`. This is necessary for BSNL/Airtel to pick the correct bearer context.
    3. Restore silence with `AT+CGEREP=2` AFTER the transmission is complete.
*   **⚠️ APPLIES TO ALL HTTP SESSIONS — no exceptions:** Data upload, Health Report, OTA HEAD request, OTA chunk download. If `CID=1` is missing from ANY HTTP session, the modem may route through a phantom PDP context on Airtel/BSNL, causing unreliable transfers or silent corruption. This was confirmed as the second root cause of OTA `invalid segment length` failures (March 4, 2026).

## 2. OTA Head Requests (Header vs. Body)
*   **Discovery:** `AT+HTTPACTION=2` (HEAD) returns the size of the **HTTP Headers** in its response field, NOT the file size.
*   **Discovery:** `AT+HTTPREAD` returns `ERROR` for HEAD requests because there is no body.
*   **Correct approach:** Use `AT+HTTPHEAD` after the `HEAD` action and parse `Content-Length:` directly from the serial stream.

## 3. DNS Latency (BSNL 2G/3G)
*   **Discovery:** BSNL DNS servers frequently take 12–18 seconds to respond.
*   **Symptoms:** `+CDNSGIP: 0,10` (Error 10 = DNS Timeout).
*   **Correct approach:** Use a minimum 20s-30s timeout for all DNS operations on BSNL.

## 4. URC Interference (The Noise Rule)
*   **Discovery:** `+CGEV` (Network status) messages from the carrier can arrive at the exact millisecond as an AT command response, corrupting the serial buffer.
*   **Symptoms:** `AT+HTTPDATA` or `AT+HTTPINIT` failing randomly with `ERROR` or handshakes timing out.
*   **Correct approach:** Always use `AT+CGEREP=0` during complex setup loops or high-speed binary transfers (OTA).

## 5. HTTPDATA Timeout Limits
*   **Discovery:** Providing a timeout > 5000ms in `AT+HTTPDATA=len,timeout` can cause the command to return `ERROR` immediately (Out of Range).
*   **Correct approach:** Always use `AT+HTTPDATA=len,5000`. Increase the ESP32's `waitForResponse` duration separately if needed to handle network latency.

## 6. Multiple Servers (Host Switching)
*   **Fact:** Primary Data and Health/OTA reside on different servers (e.g., rtdas.spatika.net vs. Contabo VPS).
*   **Correct approach:** Always call `AT+HTTPTERM` and `AT+HTTPINIT` when switching between Tasks (Primary Data -> Health) to ensure the modem resolves the correct IP and doesn't try to reuse a stale socket to the wrong host.
*   **Sanitization:** Always call `flushSerialSIT()` between these tasks to clear lingering "OK" responses from the shared UART buffer.

## 7. The "Idle Awake Trap" (Battery Drain)
*   **Discovery:** If an upload session takes more than 15 minutes, it crosses into the next time slot. After reboot, the scheduler sees a processed slot and skips its work, but stays awake.
*   **Symptoms:** Device hangs awake at 80mA for 15 minutes.
*   **Correct approach:** Implement an "Idle Timeout" in the scheduler. If 15-20 seconds pass with no activity initiated, force-enter deep sleep.

## 8. OTA Chunk Stabilization (Reading Large Files)
*   **Discovery:** After a large binary range download (`HTTPACTION=0`) finishes on BSNL, the modem's internal filesystem/buffer needs time to "settle" before a serial data dump (`HTTPREAD`) can be reliably requested.
*   **Symptoms:** `+HTTPREAD:` header missing or checksum errors during OTA.
*   **Correct approach:** Add a **500ms stabilization delay** between the HTTPACTION 200/206 response and the first `AT+HTTPREAD` command within the chunk loop.

## 9. Retry Counter Scoping
*   **Discovery:** Using `static` retry counters inside a loop (like OTA chunking) can lead to premature aborts because the count isn't reset on a "new" chunk's success.
*   **Correct approach:** Ensure local retry counters are scoped to the function or reset explicitly upon every successful transfer phase.

## 10. UART Buffer Overflow (The 1KB Limit)
*   **Fact:** The A7672S internal HTTP buffer can store up to 100KB+, but the ESP32's Hardware Serial RX buffer is often limited (e.g., 256-1024 bytes).
*   **Discovery:** Requesting a 64KB `HTTPREAD` at once without a fast, dedicated reading loop will overflow the ESP32 buffer, leading to dropped bytes and checksum failures.
*   **Correct approach:** Read data in small chunks (e.g., 4096-16384 bytes) and use `SerialSIT.readBytes()` with a generous timeout to ensure the hardware buffer is drained faster than the modem fills it.

## 11. The "Lean Handshake" Rule (Health Success v5.45.12)
*   **Discovery:** The Health Report succeeded perfectly when the command sequence was "Lean"—meaning NO extra settlement delays (`vTaskDelay`) or UART flushes were used between `HTTPPARA` and `HTTPDATA`.
*   **Success Hook:** Version 5.45.12 achieved 100% Health Report reliability by setting `CGEREP=0` right before `HTTPINIT` and pushing parameters immediately.
*   **Correct approach:** Keep the Health Report sequence fast and continuous. Use `CGEREP=0` to ensure no URCs drift in, relying on the clean buffer instead of manual flushes.

## 12. Carrier Congestion (The 3s Breather Rule)
*   **Discovery:** Switching hosts rapidly (e.g., from Spatika Main to Contabo Health) can trigger a BSNL network reset.
*   **Symptoms:** `+CREG: 0` followed by `+HTTPACTION: 1,714` (Socket error).
*   **Correct approach:** Add a **3000ms delay** after the initial `AT+HTTPTERM` when switching hosts. This gives the carrier time to release the old PDP session.
*   **Purity Guard:** Silence registration URCs with `AT+CREG=0` and `AT+CEREG=0` during the transmission window to prevent them from hitting the serial buffer during sensitive handshakes.
## 13. Chunk-Atomic OTA Rule (No More Shifts)
*   **Discovery:** Firmware corruption (`invalid segment length`) often occurs because if an `HTTPREAD` fails *partially* (e.g., 3 out of 16 segments succeed), the progress counter was already incremented. The next retry would then request the *wrong* range, shifting the binary and corrupting the entire image.
*   **Correct approach:** Implement **Chunk-Atomic Writes**. Buffer the entire Range segment (e.g., 16KB) in RAM first. Only call `Update.write()` and increment `actual_downloaded` AFTER the entire chunk is verified as successfully read.
*   **Success Hook:** Failure to read now leads to an *identical* Range request on the next attempt, preserving byte-alignment.

## 14. Zero-Gap Payload Prep (The BSNL "Flash" Rule)
*   **Discovery:** Even with URCs silenced, BSNL 2G sessions can timeout if there is a >50ms gap between `AT+HTTPPARA="CONTENT"` and `AT+HTTPDATA`.
*   **Symptoms:** `Missing DOWNLOAD` error. This happens because the ESP32 is busy calculating JSON strings, formatting sensor data, or analyzing file health *during* the AT command sequence.
*   **Correct approach:** **Pre-Calculate EVERYTHING**.
    1.  Perform all `snprintf`, `checkRainfallIntegrity`, and `analyzeFileHealth` calls at the VERY START of the function.
    2.  Store the final `jsonBody` and `msgLen` in RAM.
    3.  Only then begin the `AT+HTTPINIT` sequence.
    4.  Send the parameters (`CID`, `URL`, `CONTENT`) and the `DATA` command in a single, un-interrupted burst.
*   **Success Hook:** This "Payload-First" architecture eliminates all timing gaps, ensuring the modem's listener is hit within microseconds of the parameter setup.
## 15. The 714 "Socket Zombie" Rule (Host Switching)
*   **Discovery:** Error 714 (+HTTPACTION: 1,714) often occurs on BSNL if a new HTTP session is started too quickly after a high-bandwidth task like FTP. The modem reports the bearer as "active," but the internal socket stack is in a "zombie" state, refusing new connections.
*   **Success Hook:** Adding a **5000ms "Carrier Congestion Breather"** at the start of the next task allows the cell tower to fully release the previous session before the new one initiates.
*   **Correct approach:** Always provide a 5-second silence gap when transitioning from FTP (Main/Backlog) to Health/OTA.

## 16. In-Loop Bearer Residency
*   **Fact:** A bearer verified at the start of a function may drop *during* the sequence of AT commands if host resolution or context switching occurs.
*   **Correct approach:** Move `verify_bearer_or_recover()` **inside the attempt loop**, right before `AT+HTTPINIT`. This ensures that even if Attempt 1 fails due to a context drop, Attempt 2 will proactively rebuild the connection before trying again.

## 17. The "Body-Not-Header" Rule (Command Parsing)
*   **Discovery:** The `+HTTPACTION:` URC (e.g., `+HTTPACTION: 1,200,104`) only provides the metadata of the transaction. Searching for server commands like `"OTA_CHECK"` or `"REBOOT"` inside the URC string will always fail.
*   **Fact:** The actual JSON payload resides in the modem's internal memory and must be explicitly fetched.
*   **Correct approach:** Always use `AT+HTTPREAD=0,512` (or a similar range) after a successful `200` response to pull the response body into the ESP32's serial buffer before attempting to parse for commands or parameters.

## 18. Session Preservation Rule (OTA Chunking) ⚠️ SUPERSEDED by Rule 41
*   **Original Discovery:** High-speed, multi-chunk downloads fail on BSNL if each chunk performs a full `AT+HTTPTERM` / `AT+HTTPINIT` cycle. Rapid context switching triggers "Socket Zombie" or tower throttling.
*   **⚠️ CORRECTION (v5.47 — Rule 41):** This rule was **WRONG for Range GET downloads**. The A7672S `AT+HTTPREAD=0,N` always reads from position 0 of an **accumulating** buffer. `AT+HTTPACTION=0` **appends** new response data to the existing buffer; it does NOT replace it. Keeping the session alive means HTTPREAD=0,16384 always returns the same first 16384 bytes (chunk 0), regardless of the Range header — the flash is filled with chunk 0 repeated.
*   **Correct approach (v5.47):** Force `session_active = false` after **every** successful chunk. `AT+HTTPTERM` + `AT+HTTPINIT` clears the buffer before each new Range GET, ensuring each chunk's actual bytes are written to flash. The inner HTTPREAD retry loop (Rule 40) avoids redundant HTTPTERM on transient HTTPREAD failures, keeping network overhead reasonable.

## 19. The 714 "Socket Zombie" Recovery Logic
*   **Discovery:** When Error 714 (+HTTPACTION: 1,714,0) occurs, simply retrying the HTTP command with `HTTPTERM/HTTPINIT` is not enough. The modem's socket stack remains contaminated by the previous failed task (often an FTP DNS timeout).
*   **Fact:** The A7672S "bearer active" flag (`CGACT: 1,1`) can be a lie if the tower has closed the session due to a timeout but the modem's internal IP stack hasn't cleared it.
*   **Correct approach:** Upon detecting a `714` error, proactively deactivate the PDP context with `AT+CGACT=0,1` followed by a 2-3 second delay. This forces a clean re-initialization of the bearer on the next attempt, which is the only reliable way to clear the "Zombie" session.

## 20. The Range Boundary Rule (Final Chunk Sync)
*   **Discovery:** Some web servers or cloud proxies ignore the high-bound of a Range request (e.g., `bytes=1000-1050`) and return a full power-of-2 block (e.g., 16KB) if the file data exists.
*   **Symptoms:** `[OTA] Incomplete: 1228800/1226656. Aborting.` The download overshoots the expected file size, causing the `Update` library to fail due to a size mismatch.
*   **Correct approach:** Always calculate `bytes_to_write = min(received, total - actual)`. Proactively crop the final chunk to match the exact expected file size before writing to the flash partition. This ensures byte-parity even with greedy servers.

## 21. The Multi-Context Sync Rule (CID 1/6/8)
*   **Discovery:** BSNL SIMs often open "Phantom" PDP contexts on CIDs other than 1 (specifically 6 and 8) during high-bandwidth tasks like FTP or registration.
*   **Fact:** If `CGACT: 8,1` is active while we try to re-activate CID 1, the carrier may reject the new session with `ERROR` or Error 714 because it thinks the device already has an active IP segment.
*   **Correct approach:** When performing a recovery deactivation, don't just kill CID 1. Explicitly deactivate **CID 1, 6, and 8** to ensure the entire IP stack is released before trying to start a new Health or OTA session.

## 22. The Radio Refresh Rule (CFUN 0/1 Fallback)
*   **Discovery:** If `AT+CGACT=1,1` fails repeatedly even after registration is OK and contexts are cleared, the modem's internal radio state machine is likely desynced from the tower's.
*   **Symptoms:** `APN Activation Failed` loop despite `CREG: 1`.
*   **Correct approach:** After two failed PDP activation attempts, perform a full **Radio Refresh** (`AT+CFUN=0` followed by `AT+CFUN=1`). This forces the modem to re-attach to the network, which clears the tower-side socket "contamination" and provides a fresh chance for PDP attachment. **v7.13 Upgrade:** Allow up to **60 seconds** for the modem to re-find the tower post-refresh on 2G networks.

## 26. The Post-Registration Settle Rule (SIM Lag)
*   **Discovery:** On BSNL 2G, regaining registration (`CREG: 1`) does not mean the SIM's data profile is fully loaded into the modem. Attempting `CGACT` immediately often leads to `+CME ERROR: unknown error`.
*   **Symptoms:** Registration is OK, but data attachment fails with "unknown error" for the first 3-5 seconds.
*   **Correct approach:** After registration is confirmed (especially after a refresh or signal loss), enforce a mandatory **5-second settle delay** before attempting to activate the APN. This ensures the SIM controller is ready for data-plane commands.

## 23. The SIM Readiness Rule (CPIN vs CCID)
*   **Discovery:** `AT+CCID` or `AT+CGACT` returning `+CME ERROR: unknown error` indicates that the modem's internal SIM controller is reset or busy. This often happens for up to 5-10 seconds after a `CFUN=1` or power-on.
*   **Symptoms:** `Final CCID parsed: [EMPTY]` followed by PDP activation failure.
*   **Correct approach:** Always verify `AT+CPIN?` returns `READY` before attempting to read the CCID or activate a PDP context. If it's not ready, waiting is better than flooding with more commands.

## 24. The 2G Settle Rule (Congestion Guard)
*   **Discovery:** On BSNL 2G, the cell tower may take up to 8 seconds to "notice" that a PDP context has been deactivated. Attempting to re-activate within this window often results in an `ERROR` (Context already active) or a "ghost" attachment with no IP.
*   **Symptoms:** Flashing LED goes from Fast (Searching) to Slow (Registered), but `CGACT=1,1` fails immediately.
*   **Correct approach:** Increase the "Breather" delay between `CGACT=0` and `CGACT=1` to at least **5000ms** on 2G/stressed networks.

## 25. The Chunk Resilience Rule (706/714 Recovery)
*   **Discovery:** A long-running OTA download effectively "stresses" the carrier's temporary socket. BSNL may drop a socket (`706`) after 15-20 successful chunks even if the registration is perfect.
*   **Symptoms:** `+HTTPACTION: 0,706,0` in the middle of a download.
*   **Correct approach:** Do NOT abort the entire OTA on a single chunk failure. Instead, invalidate the session (`session_active = false`), proactively kill the bearer context (`AT+CGACT=0,1`) to clear the tower's "Zombie" state, and retry the *same* range request. Only abort if a single chunk fails > 5 times.

## 27. The Total Silence Rule (URC Contamination)
*   **Discovery:** ASCII strings from unsolicited notifications (like `+CMTI` for SMS or `+CREG` for registration) can interleave into the binary UART stream during a download. Even a 10-byte string shifts the binary offset, causing `invalid segment length`.
*   **Symptoms:** OTA reaches 100% but fails verification with a garbage hex error like `0x3a53474d`.
*   **Correct approach:** Explicitly disable ALL notifications (`AT+CNMI=0,0,0,0,0` and `AT+CREG=0`) at the start of the OTA task.

## 28. The Pre-Read Flush Rule
*   **Discovery:** If a previous command failed or a URC arrived just before `HTTPREAD`, the UART buffer contains "Ghost Bytes" that shift the start of the binary chunk.
*   **Symptoms:** MD5 mismatch on the very first chunk or shifted headers.
*   **Correct approach:** Always execute a full UART flush (reading all available bytes to trash) immediately before sending the `AT+HTTPREAD` command.

## 29. The Dangling Byte Guard (Range Mismatch)
*   **Discovery:** If the server/modem sends more bytes than requested in the Range header (e.g., at EOF), the extra "Dangling Bytes" remain in the UART buffer and contaminate the *next* AT command.
*   **Symptoms:** Consecutive AT commands returning `ERROR` or garbage because they are reading the tail of the previous binary chunk.
*   **Correct approach:** After writing the requested range to flash, check if `chunk_got` was larger than `bytes_written`. If so, explicitly read and discard the excess bytes from the UART before continuing the loop.

## 45. The "Pulsing" Gap-Fill Loop (Handle Leak)
*   **Discovery:** During data recovery for multiple missing days, the system would only recover exactly ONE record per day per boot, despite dozens being missing.
*   **Root Cause:** A logic error where file handles (`SPIFFS.open`) were being **closed inside the first iteration** of the `for` loop. The first gap was filled, the file was closed, and subsequent loop iterations failed silently because the handle was dead.
*   **Symptoms:** Old data (e.g., March 1st) "creeping" back into reports one record at a time for days.
*   **Correct approach:** Always separate Handle Management from Loop Logic. Open handles *before* the loop, iterate through all data, and close handles only *after* the loop completes.

## 46. The Binary False-Positive Trap (OTA Check)
*   **Discovery:** A 1.2MB ESP32 binary is statistically guaranteed to contain the sequence `AT+` (0x41 0x54 0x2B) somewhere in its machine code.
*   **Symptoms:** OTA chunk 3 or 4 failing with "Modem-speak 'AT+' detected at offset 3543" followed by a hardware reset.
*   **Correct approach:** Limit "Sanity Checks" for modem URCs (`HTTP/`, `ERROR`, `+HTTPREAD:`) to the first **128 bytes** of a chunk. Real modem leaks always occur at the start of a read. Scanning the entire payload for 3-byte strings leads to 100% failure rate on larger binaries.

## 47. The "Entry Point" Header Trap
*   **Discovery:** ESP32 binary entry point offsets (8-11) vary depending on the build environment, flash mode (DIO/QIO), and bootloader version. Hardcoding a check for `0x40` or `0x3F` at a specific offset often fails on perfectly valid builds.
*   **Correct approach:** Rely on the **Magic Byte (0xE9)** at `buf[0]` and a valid `Content-Length`. Do not attempt deeper structural validation of the binary header inside the GPRS driver, as it is too brittle across different firmware versions.
## 48. Health URL Consistency (Port 80 Rule)
*   **Discovery:** The main data upload succeeded on BSNL using an explicit `:80` port in the URL. Meanwhile, the Health Report (running just seconds later) was using a "clean" URL without the port and failing with Error 706.
*   **Correct approach:** Match the working pattern exactly. If one server/task works with `:80`, use it for all HTTP tasks on that modem. A7672S firmware can be sensitive to URL string formatting inconsistencies between tasks.

## 49. Soft-Retry for Health Reports (706/714 Logic)
*   **Discovery:** Nuking the bearer (`AT+CGACT=0,1`) on the very first socket error (706/714) in `send_health_report` causes a 60-90 second re-attachment delay. 
*   **Success Hook:** The main data upload logic (v5.50) uses a "soft reset" (TERM only) for the first retry. 
*   **Correct approach:** Apply the same "Soft-Retry" to Health Reports. Only nuke the bearer as a last resort on the 3rd attempt. This allows the system to recover from transient carrier glitches in ~5 seconds instead of ~2 minutes.

---
### 🛠️ Session Summary (March 5, 2026 - Late Morning): "Health Resilience & URL Alignment"
Refined `send_health_report` to use the same "Soft-Retry" strategy as the main data upload and aligned the URL formatting to include an explicit port 80. These changes eliminate the excessive bearer-nuke cycles observed on BSNL 2G when transient socket errors occur.

## 30. The Session Nuke Rule (706/714 Hard Kill)
*   **Discovery:** A simple context reset (Rule 19) is sometimes insufficient to clear a glitched session on BSNL 2G. The modem stack can get "stuck" in a state where it replays stale buffers.
*   **Symptoms:** Repeating socket errors or identical chunks being delivered for different requests.
*   **Correct approach:** On a `706/714` error, execute a full "Nuke Protocol": `AT+HTTPTERM`, `AT+CGACT=0,1`, and a 5-second delay to force the carrier side to assign a fresh IP and clear its internal memory.

## 50. The "Total Silence & WDT Resilience" Protocol (v5.52 Final)
*   **Discovery:** Chunk failures on BSNL often lead to 90s+ timeouts. If the ESP32's Watchdog Timer (WDT) is not reset during these cycles, the chip will reboot mid-download, appearing as a "random" failure.
*   **Discovery:** Setting `ota_silent_mode = true` *inside* the loop (after the first progress print) allows a small window of vulnerability where background tasks (RTC Sync or Scheduler check) can leak ASCII into the buffer before the silence flag is hit.
*   **Correct approach (v5.52):**
    1.  **Macro-Silence:** Move the `ota_silent_mode = true` assignment BEFORE `Update.begin()` to lock the console for the entire 2-3 minute download duration.
    2.  **Explicit WDT Servicing:** Add `esp_task_wdt_reset()` to ALL retry paths, hardware reset delays, and long `waitForResponse` calls. Never assume the WDT will wait for a network timeout.
    3.  **Task Suspension:** All background FreeRTOS tasks (Scheduler, RTC, TempHum, WindSpeed) now spin-wait on `while(ota_silent_mode)` so they cannot emit ANY output or trigger I2C activity during the binary transfer.
*   **Anti-Pattern (REMOVED):** Adding strings like `"SampleNo"` and `"Gaps"` to the sanity traps causes false positives because these byte sequences naturally exist in any compiled ESP32 binary's string table. See Rule 51.

---
### 🛠️ Session Summary (March 5, 2026 - Noon): "Design Review & False-Positive Trap Fix"
A structured code review revealed 11 bugs across the OTA pipeline. The most critical was that `"SampleNo"` and `"Gaps"` added as UART cross-talk traps were **matching bytes inside the firmware binary itself** (e.g., at offset 83), causing every OTA attempt to fail. Additional fixes included:
- Moving `ota_silent_mode = true` before `Update.begin()` to close a 5-second silence gap.
- Removing duplicate AT silence commands (three commands fired twice, wasting 3 seconds).
- Adding heap overflow protection to `readBytes()` (bounded by `READ_SIZE`).
- Adding `flushSerialSIT()` to the sanity-fail path to clear stale UART bytes.
- Moving `ATE0` to AFTER `verify_bearer_or_recover()` to prevent echo confusion.
- Fixed `Update.printError(SerialSIT)` (was sending error text to the modem) to `Update.printError(Serial)`.
- Fixed health report HTTPREAD wait string from `"+HTTPREAD: 0"` to `"+HTTPREAD:"`.
## 31. The Extended Zombie Guard (Multi-Signature Replay)
*   **Discovery:** In severe socket glitches (Airtel/BSNL), the A7672S sometimes re-delivers the buffer of ANY of the previous 3-4 chunks, or trickles one extra byte into the stream that shifts the rest of the file.
*   **Symptoms:** Final `Update.end()` failure with `invalid segment length` after 100% "progress".
*   **Correct approach:** Maintain a signature buffer of the headers of the **last 4 successful chunks**. 
*   **Atomic Guard:** Check the first 16 bytes of every new chunk against all 4 previous signatures BEFORE writing to flash. If a match is found, the modem is replaying stale data—nuke the session (`AT+CGACT=0,1`) and retry.

## 32. The Magic Byte Guard (Offset 0 Safeguard)
*   **Discovery:** If the very first chunk (Offset 0) is corrupted or shifted by even one byte, the entire OTA is doomed to fail at the 100% mark.
*   **Correct approach:** Explicitly verify that `buf[0] == 0xE9` (ESP32 Magic Byte) for the first chunk of every application download. If it's missing, the stream is corrupted—reset the modem immediately.

## 51. The Binary Sanity Trap False-Positive Rule
*   **Discovery (Critical):** The `isBufferSanityOK` function was trapping on strings like `"SampleNo"`, `"Gaps"`, and `"HTTP/"` — but these are SHORT, COMMON byte sequences that can appear anywhere in a compiled ESP32 binary. For example, any string constant containing "SampleNo" in the source code gets embedded in the `.rodata` section of the binary. The sanitiy check was rejecting valid firmware data at byte offset 83 on every single attempt.
*   **Root Cause:** Short English-word traps are useless against binary data. The binary is a random soup of bytes — ANY 6-character word will appear at some offset.
*   **Correct approach (v5.52 Final):**
    1. Only trap on strings that are **definitively modem-specific and contain AT syntax**: `"+HTTPREAD:"` and `"ERROR\r\n"` (with CRLF makes it highly specific).
    2. Reduce the scan window from 128 bytes to **32 bytes**. The first 32 bytes of a valid chunk are always structured binary (segment headers, addresses). Modem text bleeds in at byte 0, not at offset 83.
    3. Auto-size the loop: `sizeof(traps)/sizeof(traps[0])`, not a magic `5`.
    4. The magic byte check (`buf[0] == 0xE9`) for the first chunk remains valid — it's binary-specific.
*   **Result:** With these two targeted traps and the absolute silence protocol (Rule 50), OTA proceeds cleanly without false rejections.

## 33. The Post-Read UART Purge (Mandatory)
*   **Discovery:** Residual characters (like `\r\n` or the modem's `OK` response) can linger in the UART buffer after an `AT+HTTPREAD` binary dump. If these characters are not cleared, they will be read as the START of the next `+HTTPREAD:` header, causing a shift-bug.
*   **Symptoms:** `No HTTPREAD hdr` errors or "Shortfall" errors in the middle of a download.
*   **Correct approach:** Execute a mandatory `flushSerialSIT()` AFTER every binary read (success or failure) to ensure a clean slate for the next AT command.

## 52. The GPS Initialization Order Bug
*   **Discovery:** GPS coordinates are stored in `RTC_DATA_ATTR` (volatile RAM) and persisted to SPIFFS in `gps_coords.txt`. However, the `loadGPS()` function was only called during manual triggers or station changes. On a Power-On Reset (POR), the RTC RAM is wiped, and since coordinates weren't re-loaded from SPIFFS during `setup()`, the system reported `0.0,0.0` to the server until a fresh fix was forced.
*   **Symptoms:** Map missing station location / station appearing at Null Island (0,0).
*   **Correct approach (v5.51):** Call `loadGPS()` immediately in `setup()` after SPIFFS is ready. This ensures the last known location is ALWAYS available for the health report on every boot.

## 53. The Floating Rain Gauge Noise Storm
*   **Discovery:** If the rain gauge sensor is disconnected, the ESP32 input pin (Pin 27) can float. With the ULP program running at 1ms resolution (Rule 50), high-frequency electrical noise can be interpreted as thousands of "tips".
*   **Symptoms:** Absurd rainfall values (e.g., 4555.5mm) appearing in logs after a 15-minute period.
*   **Correct approach (v5.52):** 
    1. Implement a **50mm/15min sanity cap** on the instantaneous `rf_value` reading. Anything higher (equivalent to 200mm/hr) is physically impossible in almost any weather event and is discarded as noise.
    2. Implement a **300mm sanity cap** when loading `last_cumRF` from SPIFFS. This prevents a previous "noise storm" from corrupting the cumulative total for all future records.

## 34. Stored vs. Sent Dashboard Alignment
*   **Discovery:** The "Stored" count on the dashboard (PD) was sometimes less than the "Sent" count (HTTP).
*   **Cause:** `diag_pd_count` was only incrementing in certain paths, missing gap-filled or backfilled records.
*   **Correct approach:** Increment `diag_pd_count` on *every* successful record write to SPIFFS/SD in the scheduler. This guarantees PD $\ge$ HTTP.

## 35. The Meteorological Rollover Guard (Rain_RESET)
*   **Discovery:** The "Rain_RESET" status was being triggered daily at 08:30–08:45 AM.
*   **Cause:** The cumulative rainfall resets to 0 for the new day, which the integrity check flagged as an "error/reset."
*   **Correct approach:** Added a time-gated guard to `diag_rain_reset` that ignores a 0-drop during the 08:45 AM slot if it follows a valid 07:45–10:00 AM window.

## 36. Sent-Mask Time Alignment
*   **Discovery:** The "Today Sent Mask" on the dashboard would sometimes show gaps even after a successful multi-record send.
*   **Cause:** The code was using the *current* clock hour to mark the bitmask, rather than the `sampleNo` of the actual record being transmitted.
*   **Correct approach:** Pass the `temp_sampleNo` of the record to the success handler. Use this actual index to set the `diag_sent_mask_cur` bit, ensuring the dashboard accurately reflects which slots were delivered.

## 54. Wind Direction Logging Rate Limit
*   **Discovery:** When the wind direction (WD) sensor is disconnected or faulty (ADC:0), the system was printing a disconnection warning every single second.
*   **Symptoms:** Serial console flooded with redundant messages, making it impossible to see other system diagnostics.
*   **Correct approach (v5.51):** Implemented a state-transition check in `windDirection.ino`. The "[WD] Disconnected" message now only prints once when the sensor state changes from Connected to Disconnected.

## 55. The Backlog Interpolation Bug (RF Decrease)
*   **Discovery:** After a Power-On Reset (POR), the current `new_current_cumRF` is 0.00 until the first tip occurs. During backlog servicing for a missing previous day, the system used this 0.00 as the interpolation target for cumulative rainfall.
*   **Symptoms:** Historical rainfall data would "interpolate downward" (e.g., 5.00mm → 0.00mm) over several hours of gap-filled records, corrupting the daily total.
*   **Correct approach (v5.51):** In `scheduler.ino` backlog path, the interpolation target must be `start_crf + rf_value`. This ensures the rain remains flat (constant) during historical gaps if no new tips have been recorded in the current session.

## 56. Bulk Chunk Efficiency Rule (v5.52 Discovery)
On high-latency 2G networks (BSNL), the AT command overhead for setting up an HTTP session (`TERM -> INIT -> PARA -> ACTION`) can take up to 40-60 seconds regardless of data size. Using 4KB chunks leads to ~90% overhead time. Increasing to **16KB chunks** significantly improves throughput by amortizing the setup cost, provided the signal strength is stable (RSSI better than -85dBm).

---
### 🛠️ Session Summary (March 5, 2026 - Early Afternoon): "Stability & Integrity Overhaul"
Finalized version v5.51/5.52. Key fixes include restoring GPS persistence on boot (Rule 52), implementing high-resolution noise filtering for the rain gauge (Rule 53), rate-limiting sensor logs (Rule 54), and fixing critical interpolation logic in the backlog servicer (Rule 55). These changes ensure that the system recovers gracefully from power resets without corrupting historical data or reporting incorrect locations.


---
### 🛠️ Session Summary (March 3, 2026): "Finalizing Golden Firmware"
In this session, we resolved the terminal failure of the OTA system (Invalid Segment Length) by implementing the **Multi-Signature Zombie Guard** (Rule 31) and the **Magic Byte Verification** (Rule 32). We also synchronized the dashboard health metrics to ensure "Stored" data always exceeds "Sent" data, and eliminated the false "Rain_RESET" alarm triggered by the daily meteorological rollover. These fixes collectively form the "Golden Version" (v5.45).

---
### 🛠️ Session Summary (March 4, 2026): "OTA False Positive Fix"

## 37. The 16-Byte Zombie False Positive (Rule 31 Failure Root Cause)
*   **Discovery:** Rule 31 (Extended Zombie Guard) caused **false positives on every other chunk**, doubling download time and triggering unnecessary bearer teardowns (`AT+CGACT=0,1`) mid-download. Despite 100% progress being reached, `Update.end()` failed with `invalid segment length 0x7800040` — the OTA partition was corrupted.
*   **Root Cause:** Two compounding bugs:
    1. **Rule 31 zombie check** comparing 16-byte chunk headers of consecutive firmware blocks. Binary firmware has many repeated 16-byte patterns — this fired a false positive on ~every other chunk, triggering `AT+CGACT=0,1` bearer teardowns mid-download. The UART re-sync after each session nuke introduced byte-slip risk.
    2. **Missing `CID=1` binding in OTA sessions** (both HEAD and chunk download). Rule 1 was documented but not applied to `fetchFromHttpAndUpdate()`. Without CID binding, Airtel/BSNL may route through a phantom PDP context, causing unreliable HTTPREAD returns.
*   **Why it seemed to work in testing:** In small-firmware or lab conditions, 16-byte collisions are rare. In a live 1.2MB field binary with dense code, they're frequent enough to fire on nearly every even-numbered chunk.
*   **The Double Damage:** Each false zombie detection calls `AT+CGACT=0,1` (full bearer teardown) and a 5-second hold. Over 75 chunks, this adds ~6 minutes of wasted time AND the UART re-sync after each session nuke introduces byte-slip risk — the real source of corruption seen at `Update.end()`.
*   **Correct approach (v5.46):** **Remove the signature-based zombie check entirely.** It is solving the wrong problem. The only valid guards are:
    1.  **Magic Byte Guard (Rule 32):** Only for chunk 0 — verify `buf[0] == 0xE9`. If the very first byte is wrong, the stream is corrupted from the start. Retry with session nuke.
    2.  **`session_active` guard:** If a chunk's HTTPACTION returns a non-2xx status or HTTPREAD fails, mark `session_active = false` and let the existing retry loop handle re-init.
    3.  **Progress stall detection (if needed):** If `actual_downloaded` does not advance after N retries, abort — this catches true replay/stall without false-firing on binary data.
*   **Note:** The true "Stale Buffer Replay" failure (Rule 31 intent) is handled upstream by the 5-second `vTaskDelay` breather + `flushSerialSIT()` before each new session, which already purges modem buffers. Rule 31's signature check was a double-guard that backfired.

## 38. The Sent vs. Stored Artifact Dashboard Mismatch
*   **Discovery:** The web dashboard was showing higher sizes for "Today Sent" than "Stored", sometimes completely missing Stored packets.
*   **Cause:** Early versions of the firmware depended purely on ternary arguments and legacy loop counts (`TEST_HEALTH_EVERY_SLOT`) to dynamically send variables. If the ternary evaluated dynamically to another variable, it would visually misalign columns on the Dashboard.
*   **Correct approach (v5.47):** Pre-calculate file lines using `countStored()` directly onto variables (`cur_stored` and `prev_stored`) through SPIFFS before JSON construction. Manually push each independent variable to its respective JSON key, completely deprecating arbitrary variable reassignment and ensuring Sent and Stored metrics are perfectly represented on the Server.

## 39. The Persistent FTP-HTTP Socket Zombie
*   **Discovery:** If an FTP upload drops midway due to network weakness (Error 13), the A7672S correctly terminates the FTP service but leaves the inner IP stack flagged as active (`CGACT: 1,1`) yet silently deaf. When the device immediately transitions to `send_health_report()`, Attempt 1 fails `AT+HTTPINIT`. Prior logic simply executed `AT+HTTPTERM` and retried, meaning Attempt 2 successfully launched `HTTPINIT`, pushed the data, but suffered a 60-second hang and dropped with `+HTTPACTION: 1,706,0`.
*   **Correct approach (v5.47):** If `AT+HTTPINIT` ever fails, ALWAYS assume the worst and proactively Nuke the Bearer (`AT+CGACT=0,1`) before retrying, instead of just silently terminating HTTP. Also, `706` is identically added to the Zombie recovery list alongside `714`. Number of retries should also be realistically extended to `3`.

## 40. The Post-HTTPREAD-Failure UART Ghost (Root cause of `invalid segment length`)
*   **Discovery:** After ~20 OTA download attempts across multiple firmware versions, the OTA would consistently complete 100% progress but fail `Update.end()` with `invalid segment length 0x2f766564`. The hex `2f 76 65 64` decodes to ASCII `/ved` — a substring of `"valid IP). Recovering..."` from the firmware's own string pool.
*   **Root Cause:** When `AT+HTTPREAD` fails (modem returns `ERROR` instead of `+HTTPREAD: N`), the code printed a diagnostic and fell through to the `read_ok = false` retry path. However, the crucial `flushSerialSIT()` call only existed AFTER a **successful** read (at line ~3912), NOT in the failed-header path. This left the modem's `ERROR\r\n` response bytes sitting in the ESP32's UART hardware RX buffer.
*   **The Corruption Chain:**
    1. Chunk N: `AT+HTTPREAD` → modem returns `ERROR\r\n` (7-8 bytes in UART buffer)
    2. Code sets `session_active = false`, retries
    3. New session init'd correctly, `AT+HTTPREAD=0,16384` sent
    4. Modem responds with `+HTTPREAD: 16384\r\n` followed by 16384 binary bytes
    5. BUT: `waitForResponse("+HTTPREAD:")` reads the stale `ERROR\r\n` FIRST, then the real `+HTTPREAD:` — causing the binary payload read to START 7-8 bytes early
    6. Those 7-8 ghost bytes get written to flash, shifting every subsequent binary byte by the same offset
    7. At `Update.end()`, the ESP32 image validation reads a segment header that now points into ASCII text instead of binary code
*   **Correct approach (v5.47 Final Overhaul — Rule 42):**
    1. **Abandon Inner Retries:** The inner HTTPREAD retry loop is **DANGEROUS** for binary streams. If a read shortfall occurs (e.g. 14500/16384), the buffer is partially consumed. Retrying `HTTPREAD` pulls the NEXT bytes, causing a permanent byte-shift.
    2. **Redo Chunk Entirely:** On ANY `HTTPREAD` failure (timeout, shortfall, or `ERROR`), terminate the session and redo the entire `HTTPACTION` for that specific Range chunk.
    3. **Absolute Session Flush:** `AT+HTTPTERM` must be called at the end of EVERY chunk to ensure the modem's internal HTTP buffer is empty for the next range.
*   **Rule of Thumb:** Never assume the modem buffer is stable after a partial read. Flush everything and start over.


---
### 🛠️ Session Summary (March 4, 2026 — Night): "OTA Solved — The One-Chunk-One-Session Protocol"
The final root cause was a combination of Rule 41 (Accumulating Buffer) and Rule 42 (Fatal Shortfall). Keeping the session alive caused the modem to re-serve Chunk 0 repeatedly. Attempting to "retry" failed HTTPREADs without redoing the HTTPACTION caused byte-shifts when the UART timed out. **Final Protocol (v5.47 overhaul):** No session preservation. No inner retries. Every 16KB chunk is an atomic `TERM -> INIT -> ACTION -> READ -> TERM` transaction. This is slow (2.5 mins for 1.2MB) but 100% immune to buffer accumulation and shift-drift.



## 43. The "Log Leak" Rule (UART Cross-Talk)
*   **Discovery:** Chunk headers at 100% progress revealed ASCII strings like `"valid IP"` and `"(64.1 KB)"` inside the binary firmware buffer. These strings are identical to ESP32 `debugln` output.
*   **Root Cause:** The board hardware or shared UART wiring causes ESP32 TX (Console) output to be "seen" by the Modem UART RX. During the small timing gap between `AT+HTTPREAD` and the arrival of binary data, any foreground task (Scheduler, Health, or Update Callback) printing to `Serial` fills the Modem RX buffer with ASCII text.
*   **The Shift:** If 50 bytes of text enter the buffer, `SerialSIT.readBytes(buf, 16384)` returns a "full" 16KB block containing `50 bytes text + 16334 bytes binary`. The last 50 bytes of actual firmware are lost to UART overflow. This shifts the entire binary sequence and causes `invalid segment length` at the image tail.
*   **Correct approach (v5.47 Final):**
    1.  **Rule 43: The Log Leak (v7.50 Final Fix)**
  - *Problem*: Small ASCII segments ("vali", "6.1f", "[RTC]") were still appearing in chunks 1-10.
  - *Discovery*: Even with `ota_silent_mode = true`, tasks using direct `Serial.printf` (instead of `debugf`) bypass the logic.
  - *Solution*:
    1.  Replaced ALL `Serial.print/printf` calls in `scheduler.ino` and `rtcRead.ino` with the `debug` macro counterparts.
    2.  `ota_silent_mode` MUST remain `true` throughout the download loop.
    3.  Only the OTA progress print uses a direct `Serial.printf` bypass (safe as it is within the same task).

## 44. Command-Aware Boot (v7.50 - Quality of Life Fix)
*   **Problem:** After a fresh flash or power cycle, the device would skip GPRS/HTTP logic entirely to avoid logging a "Zero" data record. This forced developers to wait up to 15 minutes for the next slot just to test a new command (like OTA).
*   **Solution:** Modified `scheduler.ino` to allow Fresh Boots to still proceed to the `TRIGGER_HTTP` label. 
*   **Benefit:** The device now connects to the server immediately on power-up to sync time and check for pending OTA commands, even though it still skips the recorded data upload. This drastically speeds up the development/test cycle.
