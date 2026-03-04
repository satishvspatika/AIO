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

## 18. Session Preservation Rule (OTA Chunking)
*   **Discovery:** High-speed, multi-chunk downloads (like OTA) fail on BSNL if each 16KB chunk performs a full `AT+HTTPTERM` / `AT+HTTPINIT` cycle. This rapid context switching triggers the "Socket Zombie" state or tower-side throttling.
*   **Fact:** The A7672S can maintain a high-speed HTTP session indefinitely as long as the registration URCs (`CGEREP=0`) are silenced and the bearer is verified.
*   **Correct approach:** Keep the HTTP session active (`session_active = true`) throughout the entire chunk loop. Only perform a full `HTTPTERM` reset if a chunk actually fails. This reduces UART noise and stabilizes the carrier-side connection for files > 1MB.

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

## 30. The Session Nuke Rule (706/714 Hard Kill)
*   **Discovery:** A simple context reset (Rule 19) is sometimes insufficient to clear a glitched session on BSNL 2G. The modem stack can get "stuck" in a state where it replays stale buffers.
*   **Symptoms:** Repeating socket errors or identical chunks being delivered for different requests.
*   **Correct approach:** On a `706/714` error, execute a full "Nuke Protocol": `AT+HTTPTERM`, `AT+CGACT=0,1`, and a 5-second delay to force the carrier side to assign a fresh IP and clear its internal memory.

## 31. The Extended Zombie Guard (Multi-Signature Replay)
*   **Discovery:** In severe socket glitches (Airtel/BSNL), the A7672S sometimes re-delivers the buffer of ANY of the previous 3-4 chunks, or trickles one extra byte into the stream that shifts the rest of the file.
*   **Symptoms:** Final `Update.end()` failure with `invalid segment length` after 100% "progress".
*   **Correct approach:** Maintain a signature buffer of the headers of the **last 4 successful chunks**. 
*   **Atomic Guard:** Check the first 16 bytes of every new chunk against all 4 previous signatures BEFORE writing to flash. If a match is found, the modem is replaying stale data—nuke the session (`AT+CGACT=0,1`) and retry.

## 32. The Magic Byte Guard (Offset 0 Safeguard)
*   **Discovery:** If the very first chunk (Offset 0) is corrupted or shifted by even one byte, the entire OTA is doomed to fail at the 100% mark.
*   **Correct approach:** Explicitly verify that `buf[0] == 0xE9` (ESP32 Magic Byte) for the first chunk of every application download. If it's missing, the stream is corrupted—reset the modem immediately.

## 33. The Post-Read UART Purge (Mandatory)
*   **Discovery:** Residual characters (like `\r\n` or the modem's `OK` response) can linger in the UART buffer after an `AT+HTTPREAD` binary dump. If these characters are not cleared, they will be read as the START of the next `+HTTPREAD:` header, causing a shift-bug.
*   **Symptoms:** `No HTTPREAD hdr` errors or "Shortfall" errors in the middle of a download.
*   **Correct approach:** Execute a mandatory `flushSerialSIT()` AFTER every binary read (success or failure) to ensure a clean slate for the next AT command.

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

