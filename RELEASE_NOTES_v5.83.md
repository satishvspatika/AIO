# AIO9 Firmware Release Notes — v5.83
**Date:** 2026-04-15
**Base:** v5.82 (tag: `v5.82`, commit `5b801f4`)
**Build config:** SYSTEM 0 / KSNDMC_TRG / 8MB Flash / DEBUG=1 / WEBSERVER=1

---

## 🔴 Critical Bug Fixes

### [FIX-01] RF Resolution Never Persisted Correctly
**File:** `lcdkeypad.ino`
**Root cause 1:** The SPIFFS save ran **unconditionally**, outside the validity check. When the user entered `0.25`, the old `RF_RESOLUTION` (0.50) was always written to `rf_res.txt` — not the new value.
**Root cause 2:** The validity check used exact float equality (`new_res == 0.25f`). Because `atof()` returns `double` which is then truncated to `float`, this comparison could silently fail even for a perfectly typed "0.25". Result: the device rebooted without updating `rf_res.txt` at all.

**Fix:**
- Replaced `==` comparison with `fabsf(new_res - 0.25f) < 0.01f` tolerance check
- Snaps to exact value (`0.25f` or `0.50f`) before saving to eliminate residual imprecision
- Save to SPIFFS moved **inside** the validity block and saves `new_res` directly
- Wipe + reboot now **conditioned on `valid_res`** — invalid input no longer triggers a destructive wipe

---

### [FIX-02] RF Resolution Lost After SPIFFS Wipe / Reboot
**File:** `AIO9_5.0.ino`, `lcdkeypad.ino`
**Root cause:** The resolution-change wipe wipes SPIFFS (even though `rf_res.txt` is protected, SPIFFS itself is vulnerable to power-cut corruption during wipe). On the next failed-SPIFFS boot, `rf_res.txt` cannot be read and the default 0.50 is used.

**Fix:**
- Resolution is now **also saved to NVS** (`sys-config` namespace, key `rf_res`) when set via LCD
- Boot now **reads NVS first** before falling back to SPIFFS `rf_res.txt`
- NVS is immune to SPIFFS wipes and SPIFFS corruption — resolution now survives any reformat

---

### [FIX-03] `hw_tag` Variable Shadowing — `!` Never Shown on LCD
**File:** `AIO9_5.0.ino`
**Root cause:** A local `char hw_tag` declaration inside `initialize_hw()` shadowed the global `char hw_tag` (line 339). When SPIFFS failed, the **local copy** was set to `'!'` but the **global** the LCD reads stayed as `' '` (displayed as `.` on the LCD).

**Fix:**
- Removed the `char` type from the local declaration — now writes directly to the global
- Moved the `snprintf` for the `DELETE DATA` label **outside** the SPIFFS `if/else` block so it always runs regardless of SPIFFS health
- Result: `H` shown on healthy 8MB devices, `!` shown when SPIFFS needs repair

---

### [FIX-04] `sync_mode` Written Outside Critical Section
**File:** `gprs_http.ino`
**Root cause:** Two write sites for `sync_mode = eHttpStop` in `prepare_data_and_send()` and `store_current_unsent_data()` were unprotected — bare writes to a shared variable accessed under `syncMux` elsewhere.

**Fix:** Both sites now wrapped in `portENTER_CRITICAL(&syncMux)` / `portEXIT_CRITICAL(&syncMux)`

---

### [FIX-05] Brace Mismatch in `get_registration()`
**File:** `gprs_health.ino`
**Root cause:** Mismatched closing brace in the CREG retry loop caused an extra `}` to be consumed, truncating the `retries++` logic outside its intended while-loop scope.

**Fix:** Brace rebalanced — `retries++` now correctly increments inside the while loop on every iteration.

---

## 🟡 Stability Hardening

### [HARD-01] I2C Mutex Timeout Reduced to Prevent UI Stalls
**File:** `globals.h`, `tempHum.ino`
- `I2C_MUTEX_WAIT_TIME` reduced from **2500ms → 800ms**
- All `tempHum.ino` mutex waits explicitly set to **1000ms** to prevent blocking the LCD keypad task during I2C hangs

### [HARD-02] `recoverI2CBus(true)` — Prevents Re-entry Deadlock
**File:** `tempHum.ino`
- `recoverI2CBus()` was called while holding `i2cMutex`, causing a potential deadlock
- Now calls `recoverI2CBus(true)` (already-locked path) to skip the internal mutex acquire

### [HARD-03] Ghost-Lock Removal in Web Server
**File:** `webServer.ino`
- `handleData()` previously took `i2cMutex` just to read `current_hour`/`current_min` — globals that are safely readable without a lock
- Removed the unnecessary mutex acquisition; eliminates a potential UI lag source
- Added `"status": "READY"` field to the JSON data response

### [HARD-04] `sync_mode` Snapshot in Scheduler Wait Loop
**File:** `scheduler.ino`
- The manual-task wait loop was reading `sync_mode` directly (bare read of a `syncMux`-protected variable)
- Fixed to snapshot `sync_mode` under `portENTER_CRITICAL` before and during the loop

### [HARD-05] SD Copy — Per-File Mutex Interleaving + Verification
**File:** `global_functions.ino`
- `copyFilesFromSPIFFSToSD()` completely rewritten:
  - `fsMutex` now acquired **per file** (not held for the entire bulk copy) — allows scheduler to write logs between files
  - Byte-count verification: warns if written bytes ≠ source size
  - Success/fail counters printed at end
  - WDT reset inside file write loop for large files
  - Added `root.close()` (was leaking the directory handle)

### [HARD-06] `pruneFile()` Mutex Re-take Timeout Increased
**File:** `global_functions.ino`
- Retry-take timeout after VFS delay increased from **5000ms → 10000ms**
- Improved error message when mutex re-take fails

---

## ⚙️ Configuration Changes (`user_config.h`)

| Setting | v5.82 | v5.83 |
|---|---|---|
| `FIRMWARE_VERSION` | `"5.82"` | `"5.83"` |
| `DEBUG` | `0` | `1` |
| `ENABLE_WEBSERVER` | `0` | `1` |

> **Note:** `DEBUG=1` and `ENABLE_WEBSERVER=1` are development/field-debug settings. Set `DEBUG=0` and `ENABLE_WEBSERVER=0` for low-power production deployments.

---

## ✅ Files Changed vs v5.82

| File | Type of Change |
|---|---|
| `AIO9_5.0.ino` | RF resolution NVS load, hw_tag shadowing fix, snprintf relocation |
| `lcdkeypad.ino` | RF resolution save: fabsf, NVS backup, valid_res guard |
| `globals.h` | I2C_MUTEX_WAIT_TIME 2500 → 800 |
| `gprs_http.ino` | sync_mode critical section guards |
| `gprs_health.ino` | Brace balance fix in get_registration() |
| `tempHum.ino` | i2cMutex timeouts to 1000ms, recoverI2CBus(true) |
| `webServer.ino` | Ghost-lock removal, "status":"READY" added |
| `scheduler.ino` | sync_mode snapshot under critical section |
| `global_functions.ino` | SD copy rewrite, pruneFile timeout |
| `user_config.h` | Version, DEBUG, WEBSERVER flags |

---

## 🔁 Upgrade Notes

- **RF resolution will load from NVS after upgrade** — no data loss even if SPIFFS is wiped
- **SPIFFS repair path (`!` tag)** now correctly shown on `DELETE DATA` field when SPIFFS fails to mount
- After an in-field SPIFFS repair (`DELETE DATA` → `!` → SET → SET), the RF resolution will be restored from NVS automatically — no need to re-enter it via LCD
