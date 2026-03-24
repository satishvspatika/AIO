# 🦅 SPATIKA AIO 9.5.0 - Master Roadmap (Phases 4-6)

Based on the highly refined security & architecture audit (v5.68), the following roadmap extracts only the genuinely critical "Silent Death", Data Corruption, and Security flaws while stripping away hallucinated or non-essential cosmetic warnings.

---

## ⚡ PHASE 4: FIRMWARE VFS CORRUPTION & MUTEX DEADLOCKS (ESP32)
*These logic flows cause the ESP32 filesystems to implode or stall global thread execution entirely.*

1. **`signature.txt` Non-Atomic Write Destruction:**
   Currently, the HTTP-Success block opens `/signature.txt` for `FILE_WRITE`. If power is lost or a brownout hits mid-write, the signature truncates to 0 bytes. Upon reboot, the system misidentifies the time anchor, generating cascading data gaps.
   **[Goal]:** Mirror the `resync_time` sequence. Write to `/signature.tmp`, flush, and then `SPIFFS.rename()` atomically.
2. **Concurrent `fsMutex` Collision on APN Config:**
   `save_apn_config()` inside `gprs_helpers.ino` blindly executes `SPIFFS.open()` without claiming `fsMutex`. If the Core 1 Scheduler attempts to write `unsent.txt` in that exact millisecond, the physical VFS index corrupts.
   **[Goal]:** Wrap `save_apn_config()` strictly within an `xSemaphoreTake(fsMutex)` block.
3. **Global System-Starvation via `i2cMutex` (readHDC):**
   The Temp/Hum task holds `i2cMutex` exclusively through a hardware `vTaskDelay(20ms)` simply to wait for the ADC IC to settle. This totally locks the Core 0 `rtcRead` task and breaks instantaneous timekeeping boundaries for the rest of the ESP32.
   **[Goal]:** Release the `i2cMutex` during the 20ms hardware conversion sleep, then re-acquire it merely to pull the final datagram.
4. **The "6.55" Permanent Amnesia Bug:**
   `last_ver = prefs.getString("last_ver", "6.55")` forces every brand-new or cleanly flashed module to falsely interpret a "Version Change" against firmware `5.68`. This erroneously wipes the node's internal state mechanics and resets critical anchor variables.
   **[Goal]:** Match default NVS query states to dynamically align with `UNIT_VER` directly.

---

## 💾 PHASE 5: FIRMWARE HEAP EXHAUSTION & STACK OVERFLOWS (ESP32)
*Correcting runaway RAM usage inside the Core 0 HTTP structures.*

5. **2KB Stack Overflow Risk (`waitForResponse`):**
   The GPRS subsystem relies on `char buf[2048]` allocated *on the stack* inside `waitForResponse`. During extremely deep OTA failure paths, multiple functions push 2KB frames onto the limited 16KB FreeRTOS envelope, culminating in catastrophic stack-smash reboots.
   **[Goal]:** Convert `buf` to a `static` array safely; the overarching `modemMutex` already serializes this function globally.
6. **FTP Ram-Sledgehammer (`readString`):**
    `content = file1.readString()` attempts to copy an entire contiguous 100KB FTP log directly into the dynamic String Heap. Fragmented systems cannot find a continuous 100KB block, resulting in silent `malloc()` failure and blank payload FTP deliveries.
    **[Goal]:** Chunk the file read `file.read(buf, 256)` actively into the `AT+FSWRITE` socket rather than pre-buffering.
7. **`content` String Core 0 / Core 1 Cross-Contamination:**
    A globally scoped `extern String content` is accessed by both the Scheduler Core and the Modem Core without `fsMutex` guarding. At scale, Core-0 reading the `.c_str()` while Core-1 appends JSON will mathematically induce a hard CPU panic.
    **[Goal]:** Eliminate the shared String structure and segregate execution scopes via controlled Mutex buffers.
8. **FreeRTOS Dead-Lock via Arduino `delay(1000)`:**
    In the FTP pipeline, an explicit Arduino `delay(1000)` is issued. This forcefully halts the FreeRTOS RTOS Task Context completely without surrendering processing ticks to the Watchdog, frequently prompting `Task Watchdog Triggers`.
    **[Goal]:** Universal upgrade to `vTaskDelay(pdMS_TO_TICKS(1000))`.

---

## 🔒 PHASE 6: CONTABO SERVER VULNERABILITIES & EVENT-LOOP STARVATION (Cloud API)
*These fixes protect the Fast API host from remote compromise, database poisoning, and asynchronous connection blockages.*

9. **Arbitrary Schema Poisoning via `/health`:** 
   The `_auto_migrate` function dynamically adds SQLite columns for *any* unknown JSON key. A malicious or malformed packet could instantly flood `SpatikaFleet.db` with thousands of garbage columns, irreparably corrupting the fleet database.
   **[Goal]:** Implement a strict Python `_ALLOWED_FIELDS` whitelist array; reject unknown payload keys.
10. **Path Traversal on `/builds/` Endpoint:** 
   The OS file-joiner `os.path.join("/app/builds", filename)` does not protect against directory escaping if a user injects `../../`.
   **[Goal]:** Enforce `os.path.realpath` boundary anchoring.
11. **Synchronous OTA Event-Loop Starvation:**
   `StreamingResponse` for OTA firmware chunks relies on a synchronous python `open(filepath, "rb")`. Reading chunk sequences directly blocks the Uvicorn global event loop. A 2-minute sluggish OTA download will literally queue and freeze health reports from the entire fleet.
   **[Goal]:** Transpose the generator to use `aiofiles.open()` for non-blocking asynchronous disk streaming.
12. **_DB_COLUMNS_CACHE Multi-Worker Desync:**
   The SQLite schema cache is a process-local object. In a multi-worker ASGI setup, Worker B won't see Worker A's auto-migrated schema changes, causing SQLite `OperationalError: duplicate column name` loops.
   **[Goal]:** Convert the cache to a forced live-query or shared state machine across workers.
