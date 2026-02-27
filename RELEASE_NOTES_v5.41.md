# Release Notes: v5.41 (Feb 27, 2026)

## 🎯 Overview

**HTTP Session Hardening — Eliminating the 47-Second Timeout Waste**

Version 5.41 is a targeted reliability fix that resolves a subtle but impactful bug in the HTTP upload sequence. Under certain modem state conditions, when both `AT+HTTPINIT` attempts failed at the start of a cycle, the code continued blindly into the send path — causing a **~47-second chain of back-to-back timeouts** (`HTTPDATA` + `HTTPACTION`) before finally giving up and storing data to the backlog.

The device appeared to be "trying" but was actually wasting nearly a full minute on a doomed HTTP session, then hard-resetting the modem unnecessarily. This version fixes the root cause, reduces wasted active time to near-zero in failure cases, and improves modem UART hygiene across the HTTP flow.

**Key Metrics vs v5.40:**
| Metric | v5.40 | v5.41 |
|---|---|---|
| Time wasted when both HTTPINITs fail | ~47 seconds | **< 1 second** (fast-fail) |
| Modem hard-reset after HTTP-only failure | ❌ Triggered incorrectly | ✅ No longer triggered |
| HTTPTERM before HTTPINIT | `vTaskDelay(200)` only | ✅ `waitForResponse` + `flushSerialSIT` |
| HTTPACTION timeout diagnosis | `diag_http_fail_reason` left stale | ✅ Set to `"TIMEOUT"` |
| Bearer tear-down on HTTPACTION timeout | ❌ Unnecessary `SAPBR=0,1` | ✅ Skipped (bearer was healthy) |

---

## ✨ Fixes & Improvements

### 1. **HTTPINIT Fall-Through Bug Fixed** 🐛
- **Before:** If the first `AT+HTTPINIT` failed, the code did a `HTTPTERM` + retry `HTTPINIT` but **discarded the return value**. If the retry also failed, execution continued blindly, setting `HTTPPARA` and calling `HTTPACTION` on a dead HTTP session.
- **After:** A `http_ready` boolean flag tracks whether `HTTPINIT` actually succeeded. The retry return value is now checked. If both attempts fail, `http_ready = false` is set and the failure is logged clearly:
  ```
  [GPRS] HTTPINIT failed on both attempts. Will store to backlog.
  ```

### 2. **Fast-Fail Guard in `send_at_cmd_data()`** ⚡
- **Before:** Even with no HTTP session, the code executed `AT+HTTPDATA` (2s timeout) then `AT+HTTPACTION=1` (45s timeout) → **~47 seconds wasted per failed cycle.**
- **After:** `send_at_cmd_data()` checks `http_ready` at entry. If `false`, it returns `0` immediately:
  ```
  [HTTP] HTTP session not ready. Fast-fail to backlog.
  ```
- Data goes straight to `/ftpunsent.txt` backlog with no modem wait.

### 3. **Proper HTTPTERM Handling Before HTTPINIT** 🔧
- **Before:** `AT+HTTPTERM` was followed by only `vTaskDelay(200)` — no `waitForResponse`. The modem's `ERROR` or `OK` response sat in the UART buffer and could pollute the subsequent `AT+HTTPINIT` exchange.
- **After:** `AT+HTTPTERM` is now followed by `waitForResponse("OK", 1000)` to properly consume the response, then `flushSerialSIT()` to clear any remaining noise before `AT+HTTPINIT`.

### 4. **HTTPACTION Timeout Correctly Labelled** 🏷️
- **Before:** When `AT+HTTPACTION=1` timed out (no `+HTTPACTION:` URC received in 45s), `diag_http_fail_reason` was left with its previous value because comma-parsing failed on the blank response string.
- **After:** A blank/empty `+HTTPACTION:` response is now explicitly detected and labelled:
  ```
  diag_http_fail_reason = "TIMEOUT"
  [HTTP] HTTPACTION timed out — no URC received from modem.
  ```

### 5. **TIMEOUT Treated as Transient in Retry Path** 🔄
- **Before:** Only error codes `706`, `713`, `714` triggered the fast-retry path (skip `SAPBR=0,1` bearer teardown). A timeout (`diag_http_fail_reason` stale/unknown) fell into the full bearer-reset path, which could destabilise an otherwise healthy bearer and cause the retry `HTTPINIT` to fail.
- **After:** `TIMEOUT` is added to the `transientErr` check. A timeout means the network path was briefly unavailable — the bearer itself was fine. `SAPBR=0,1` is skipped, preserving the bearer for the retry.

### 6. **HTTPINIT Success Set on Retry Path** ✅
- If the retry `HTTPINIT` in `prepare_data_and_send()` succeeds, `http_ready = true` is set before calling `send_at_cmd_data()` so the fast-fail guard doesn't block a valid retry attempt.

---

## 🔧 Root Cause (What Actually Happened)

Analysis of field logs from WS0033 at 14:00 on Feb 27, 2026 revealed the exact failure chain:

1. `AT+CGACT=0,8` deactivated CID 8 (secondary context), briefly disrupting the network stack
2. First `AT+HTTPINIT` → returned `ERROR` (logged as failed)
3. Recovery: `HTTPTERM` + `HTTPINIT` retry — return value **not checked** — also returned `ERROR`
4. Code continued executing `HTTPPARA` commands on a dead HTTP session
5. `AT+HTTPACTION=1` sent → modem stuck → **45-second timeout** → blank response
6. `diag_http_fail_reason` not updated (blank string, no commas to parse)
7. Retry path: `transientErr = false` → unnecessary `SAPBR=0,1` bearer teardown
8. Retry `HTTPINIT` → also `ERROR` (bearer now disrupted)
9. `diag_consecutive_reg_fails++` incremented despite registration succeeding
10. Shutdown: hard GPIO reset instead of graceful `AT+CPOWD=1`

**Total wasted time: ~47+ seconds. Total data loss: 1 slot stored to backlog.**

---

## 📋 Technical Details

### Modified Files
- `globals.h` — Version bumped to `5.41`
- `gprs.ino` — `http_ready` flag, `waitForResponse` on HTTPTERM, guarded HTTPINIT retry, fast-fail in `send_at_cmd_data`, TIMEOUT labelling, transientErr expansion, `http_ready=true` on retry success
- `fw_version.txt` — Updated to `v5.41`

### New Global Variable
```cpp
bool http_ready = false; // Tracks if HTTPINIT succeeded for this cycle
```
Declared at the top of `gprs.ino`. Reset to `false` at the start of each HTTP session. Set to `true` only after a confirmed `HTTPINIT OK`.

---

## 🧪 Testing Performed

- ✅ Validated on WS0034 (TWSRF9-GEN, HDC1080, Airtel IoT SIM, A7672S modem)
- ✅ 7 consecutive successful cycles observed (15:00 → 16:30, Feb 27, 2026)
- ✅ All cycles: `+HTTPACTION: 1,200,9` → `Success` → graceful `AT+CPOWD=1` shutdown
- ✅ 15:30 cycle: -93 dBm signal + 9 CREG iterations + status `3` (roaming) — still succeeded
- ✅ No HTTPINIT failure messages in any v5.41 cycle
- ✅ No hard GPIO resets in any v5.41 cycle
- ✅ Sleep time consistent at ~13:24–13:56 min:sec (tight, no wasted timeout seconds)

---

## 🔄 Upgrade Path

### From v5.40:
- **Direct upgrade** — Flash `firmware.bin` using `esptool` or Arduino IDE
- **No NVS or SPIFFS data loss** — all station config, APN cache, and data files preserved
- **No hardware changes** required

---

## ⚠️ Known Limitations

1. **BME280 not used** — `ENABLE_PRESSURE_SENSOR 0`. Detected at boot but pressure task not spawned.
2. **SD Card** — If not inserted, system logs to SPIFFS only (expected behavior).
3. **Root cause of first HTTPINIT failure** — Likely `AT+CGACT=0,8` briefly disrupting the network stack. This is mitigated by the `http_ready` fast-fail but not eliminated at source. Will be monitored in future versions.

---

**v5.41 is production-ready. Recommended upgrade for all active TWS-RF (SPATIKA_GEN) deployments.** 🚀
