# Release Notes - v5.74: Post-v5.73 Audit Fixes

**Branch:** v5.70-stable  
**Date:** 2026-03-31  
**Type:** Critical Bug Fix

---

## Bugs Found and Fixed

### BUG-1 (CRITICAL): `ota_silent_mode` extern removed from `globals.h`
**File:** `globals.h`  
**Severity:** 🔴 COMPILE ERROR — would prevent build on any target with DEBUG=1

The v5.73 commit accidentally removed `extern volatile bool ota_silent_mode` from `globals.h`.  
This variable is referenced directly by the `debugln()`, `debug()`, and `debugf()` macros defined  
in `globals.h` at lines 211, 216, 226. Every `.ino` that includes `globals.h` with `DEBUG=1`  
would fail to compile.

**Fix:** Restored `extern volatile bool ota_silent_mode;` at line 74 of `globals.h`.

---

### BUG-2: Triple-duplicate `extern volatile bool gprs_started` in `globals.h`
**File:** `globals.h`  
**Severity:** 🟡 MINOR (harmless in C++, but clutters header and can confuse linters)

The last commit left `gprs_started` declared three times: at lines 194, 454, and 568.  
This was caused by code reorganization without cleanup.

**Fix:** Removed the two redundant declarations at original lines 194 and 454.  
Single canonical declaration retained at line 568 (in the Task Handles section).

---

### BUG-3 (LOGIC): `last_unsent_sampleNo` dedup guard never reset on day rollover
**File:** `scheduler.ino`  
**Severity:** 🔴 DATA LOSS — could silently skip legitimate gap-fill backlog writes on new day

`last_unsent_sampleNo` is declared `RTC_DATA_ATTR` (persists across deep sleep).  
The gap-fill loop in `scheduler.ino` guards writes with `q != last_unsent_sampleNo`.  

**Problem scenario:**
- Yesterday ends with `last_unsent_sampleNo = 45` (sample 45 was written to ftpunsent.txt)
- Device deep sleeps overnight, today has a communication gap at sample 45
- Gap-fill loop skips sample 45 (`q == 45 == last_unsent_sampleNo`) → DATA LOST

**Fix:** Added `last_unsent_sampleNo = -1;` to the daily rollover reset block in `scheduler.ino`  
(alongside the existing `diag_sent_mask_cur` resets). This ensures the dedup guard only  
prevents same-boot duplicates, not cross-day false matches.

---

## No-Action Items (Investigated, Not Bugs)

- **`verify_bearer_or_recover()` inside FTP retry loop**: Correct — `modemMutex` is already
  held by `send_ftp_file()`. `verify_bearer_or_recover()` does not re-acquire `modemMutex`.
  No deadlock risk.

- **`store_current_unsent_data()` mutex pulse pattern**: Correct — mutex is taken once to
  read `lastrecorded_*.txt`, released, string processing done without lock, then re-taken
  for the write. The dedup guard prevents redundant writes if called multiple times per slot.

- **`send_at_cmd_data()` duplicate prototype removed**: Pre-existing cleanup in globals.h
  working tree (removed from line 633, canonical at line 90).

- **Safety Valve forceful reset of `gprs_started/schedulerBusy` in `loop()`**: By design.
  The 4.5-minute absolute cap is a battery emergency measure. `http_ready = false` ensures
  the modem stack is properly reinitiated on next wakeup.

---

## Post-Audit Hardening (Concurrency & Architecture Review)

### FIX C-1: Scheduler `total_wind_pulses_32` torn read
**File:** `scheduler.ino`
**Severity:** 🟡 MEDIUM — Multi-instruction 32-bit load on 8-bit registers could occasionally read a torn value if `windSpeedTask` updated it mid-read.
**Fix:** Wrapped the `total_wind_pulses_32` read in `schedulerTask` with `portENTER_CRITICAL(&windMux)`.

### FIX C-4: `httpInitiated` Cross-Core Atomic Visibility
**Files:** `AIO9_5.0.ino`, `global_functions.ino`
**Severity:** 🔴 CRITICAL — Plain reads of `httpInitiated` on Core 1 could be heavily cached or reordered, leading to early deep-sleep initiation while Core 0 was actively executing an HTTP request.
**Fix:** Upgraded all bare reads in the sleep gate and `start_deep_sleep()` guard to `__atomic_load_n(&httpInitiated, __ATOMIC_ACQUIRE)`.

### FIX Memory Visibility & Shadow Variables
**Files:** `globals.h`, `AIO9_5.0.ino`
**Severity:** 🔴 CRITICAL — Cross-core signaling variables (`wd_ok`, `sampleNo`, `temperature`, `humidity`) were declared without `volatile` qualifiers, allowing compiler register-caching to silently hide updates from other tasks. Additionally, a local variable shadowed `sampleNo` in the warm boot recovery block.
**Fix:** Enforced `volatile` globally on all shared primitives and eliminated the local shadow variable `int sampleNo = ...`.

### FIX R-2: `pruneFile` Mutex Race Condition
**File:** `global_functions.ino`
**Severity:** 🔴 CRITICAL — The `pruneFile` retry loop performed an unprotected `xSemaphoreGive(fsMutex)` immediately after a failed rename, even though it no longer owned the mutex lock if the initial `take` failed or timed out. This abandoned the binary semaphore queue and caused catastrophic file system races.
**Fix:** Removed the unsupported `give` and `vTaskDelay` logic. The sequence now loops safely while retaining the continuous SPIFFS lock.

### FIX R-3: `gprs_http` Heap Fragmentation (readString)
**File:** `gprs_http.ino`
**Severity:** 🔴 CRITICAL — Unbounded `file1.readString()` calls caused severe heap fragmentation via the Arduino Core `Stream.cpp` dynamic re-allocation mechanics on dense backlog files.
**Fix:** Replaced dynamic string sizing with a rapid, stable `file1.readBytes(...)` block-read directly into a strictly bounded 256-byte stack array.

### FIX R-4: `latestSensorData` Struct Spinlocks
**Files:** `windSpeed.ino`, `tempHum.ino`, `globals.h`
**Severity:** 🟡 LOW — Dead code (unused reads), but theoretically vulnerable to Xtensa word-read struct tearing.
**Fix:** Defined `sensorDataMux` globally and wrapped all structural member writes to `latestSensorData.windSpeed`, `temperature`, and `humidity`. 

### FIX R-5: Scheduler Stack Diagnostic Tracing
**File:** `scheduler.ino`
**Severity:** 🟢 ENHANCEMENT
**Fix:** Placed `uxTaskGetStackHighWaterMark` directly inside the 15-minute gap-fill execution block (post processing) to track absolute maximum recursion stack depth without spamming the serial output during idle 1Hz sweeps.
