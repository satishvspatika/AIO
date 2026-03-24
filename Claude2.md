# 🦅 SPATIKA AIO 9.5.0 - Master Roadmap (Phases 4-6 for v5.69+)

Now that the core OTA Deployment Pipeline, Server Cloud APIs, and Health Report Triggers have been fully stabilized in **Phase 1-3 (v5.68)**, our focus shifts to isolating the underlying operating system and file-storage vulnerabilities that can cause "Silent Failures" or mathematical errors on the sensor bus itself.

This roadmap dictates the architectural fixes for **Phase 4, 5, and 6**.

---

## 💾 PHASE 4: SPIFFS STORAGE & HEAP MEMORY RESILIENCE
*These vulnerabilities cause valid data to disappear without generating HTTP errors, or crash the dynamic ESP32 memory heap over time.*

1. **The SPIFFS Auto-Pruning Dead End:** 
   The firmware contains a 512-byte size limit for the main log file. If exceeded, it prints "Pruning..." but doesn't actually delete any lines—it just silently aborts the write instruction. This permanently skips weather logging until `FILLGAP` runs.
   **[Goal]:** Implement a true Line-By-Line Circular Buffer (FIFO) loop removing the oldest string, and expand the file headroom generously.
   
2. **Gap-Fill "Black Hole" Recovery Bug:** 
   Throughout the `FILLGAP` loop, the file handler for `unsent.txt` is opened and blindly pushed to. If the file is locked or fails to mount `if (!unsent)`, the station will infinitely print recovery data to the `NULL` console, losing 15-minute bursts entirely.
   **[Goal]:** Wrap all file `print/write` commands inside a strict file-sanity closure block before execution.

3. **UART Heap Memory Fragmentation (`response += c`):**
   The SIM module driver frequently builds enormous JSON responses inside `waitForResponse()` character-by-byte using the `String` concatenation operator (`+=`). This rapidly allocates and destroys memory blocks 500+ times per API call, heavily fragmenting FreeRTOS memory until `malloc` aborts and SIM operations fake-fail.
   **[Goal]:** Swap dynamic `String` aggregation for a fast, static compile-time Character Arrays `char stream_buf[1024]`.

---

## ⚡ PHASE 5: HARDWARE SENSOR PHYSICS & THREADING 
*Mitigating chaotic electrical noise, power limits, and race conditions.*

4. **ULP Wind Speed Extreme Data Inflation:**
   The `windSpeed.ino` ULP hardware pin absolutely lacks native software debounce logic. A 3ms vibration on the mechanical reed switch (due to physical wind-tower shaking) registers as 3 fully completed wind rotations internally heavily inflating cyclonic data. 
   **[Goal]:** Engineer the equivalent `U_DEBOUNCE_CNT` assembly-level gating block that exists within the Rain Gauge into the Wind Speed sub-processor logic.

5. **Battery Brownout "Instant Death" Guard:**
   The A7672S GPRS 4G module rips huge 2+ Amp instantaneous current bursts during payload upload. Doing this when the internal LiPo cell rests at <3.5V immediately crashes the internal `VCC` voltage rails causing Hardware Panic Shutdowns (Reset 15).
   **[Goal]:** Intercept `health_in_progress` and the HTTP upload wrapper early. If `bat_val < 3.5V`, blindly write records directly into `unsent.txt` and sleep without ever waking the modem. 

6. **The UART Multi-Threading Fatal Collision:**
   The `modemMutex` briefly and intentionally unlocks during complex gap-fill sequencing. If a field technician hits `SEND STATUS / SMS` on the LCD during that exact microsecond, both systems aggressively wrestle for the UART Tx/Rx pipeline simultaneously obliterating both.
   **[Goal]:** Implement an asynchronous `eHttpBusy` lock state flag. The LCD Keypad will queue physical button presses until the SIM modem has structurally terminated its session.

---

## 📡 PHASE 6: ADVANCED SERVER FLEET MANAGEMENT
*Server-side scaling protections and administrative oversight additions.*

7. **Fleet-Wide Configuration Database (PostgreSQL):**
   Right now, the FastAPI app stores local `.env` values and hardcoded station identifiers. Transitioning completely to a strictly relational SQL schema for station authorization logic.
   
8. **Automated Telegram / Email Exception Relaying:**
   Server internal 500 Errors currently render user-facing crash pages safely (Fixed in Phase 2), however, they require manual administrator log inspection `cat /var/log/spatika`.
   **[Goal]:** Hook `error.html` fallbacks into an immediate Notification Pipeline so administrators know instantly when a background anomaly occurs.

9. **Modem Raw-Command Auditing Engine:**
   When testing new firmware remotely via `/cmd/{stn}/AT+...`, the responses blindly print or disappear. 
   **[Goal]:** Build an integrated Server-side viewer archiving pure AT serial logs remotely streamed from isolated 1% beta stations.
