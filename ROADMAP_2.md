# 🗺️ ROADMAP 2: v5.66 Post-Analysis Cleanup

Based on the secondary analysis provided by Claude, here is the curated list of the *required* fixes. Many of the "Medium" issues raised are legitimate C++ linkage and visibility concerns that only work currently due to forgiving aspects of the Arduino single-translation-unit build system. Fixing them makes the codebase truly bulletproof for future ESP-IDF or PlatformIO migration.

---

## 🔴 Critical Linkage (Blockers for TWS)
*Status: COMPLETED* ✅

- [x] **Missing Externs for Data Parsers** 📦
    - **Issue:** `String content` and the `float temp_crf`, `temp_instrf` group are globally defined in the `.ino` but heavily used in `gprs.ino` without `extern` header definitions in `globals.h`.
    - **Action:** Added explicit `extern` declarations to `globals.h` (line 334) to enforce strict C++ compliance.

## 🟡 Functional Regressions & Bloat
*Status: COMPLETED* ✅

- [x] **Boot Delay Bloat** ⏱️
    - **Issue:** `delay_val` was subtly bumped from `10000` to `15000`.
    - **Action:** Reverted to `int delay_val = 10000;` in `AIO9_5.0.ino`.
- [x] **Dead Code Elimination** 🧹
    - **Issue:** `apn_user` and `apn_pass` are defined but completely orphaned.
    - **Action:** Deleted from `AIO9_5.0.ino` to clean up the global scope.

---

## ✅ Previously Addressed / Audited (No Action Required)

- [x] **`send_ftp_file` Mismatch**: Claude correctly identified that `alreadyLocked` lacked a default value in the header. (Note: I already caught and silently fixed this in the previous step by adding `= false` to the `globals.h` declaration!)
- [x] **`send_health_report` Early Mutex Check**: Audited. Clause raised a concern about SPIFFS writes happening after the early `fsMutex` release. I analyzed lines `5260` - `5500`. The only data written afterwards is `Preferences prefs.putInt...` which writes to the ESP32's internal Non-Volatile Storage (NVS) using flash sectors entirely separate from the SPIFFS partition. It is 100% thread-safe and mechanically correct.

---
Let me know if this roadmap reflects exactly what you want tackled, and I will execute the `PENDING` items immediately.
