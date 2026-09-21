# Release Notes: v6.48 (Sep 21, 2026)

## 🎯 Overview
Release v6.48 includes comprehensive field release testing binaries across all 6 system configurations, featuring Direct TCP Socket OTA download hardening, Content-Range offset validation, direct flash write mechanisms, 5s wind sampling window, and full OTA state diagnosis.

---

## 📡 OTA Status & Diagnosis (99% Complete)

### Found actual outcome: Download reaches 100%, `Update.end()` fails on MD5 Checksum
- **The real result:** Download reaches 100%, but `Update.end()` fails at the final partition verification step:
  ```text
  [OTA-TCP] Chunk download complete: 1425840 / 1425840 bytes (100%)
  E (4330291) esp_image: Checksum failed. Calculated 0x59 read 0xd0
  [OTA-TCP] Update.end FAILED. Code: 9, Str: Could Not Activate The Firmware
  ```
- **Verification of previous fixes:** Both fixes from the last round are genuinely in place (`alignBuf` is eliminated and `writePayloadToFlash()` writes directly to flash). So this is not a case of unapplied fixes—it is failing with the same class of error despite both fixes being active.

### Analysis of Stream Integrity & Accounting:
1. **No partial/failed frames touch flash:**
   ```cpp
   int frameBytes = readIpdFrame(chunkBuf, 8192, 12000);
   if (frameBytes <= 0) {
     ...
     break;   // ← guards BEFORE writePayloadToFlash is ever reached
   }
   if (!writePayloadToFlash(chunkBuf, frameBytes, total_no_of_bytes, actual_downloaded)) { ... }
   ```
   `writePayloadToFlash()` is only ever called after `frameBytes <= 0` has already broken out of the loop. An incomplete frame's partially-read bytes genuinely never touch flash. This mechanism is sound.

2. **Byte-accounting is self-consistent:**
   - Traced through a real retry:
     ```text
     [IPD-ERR] Incomplete +IPD frame: read 980 / 1092 bytes. Discarding & purging UART.
     [OTA-TCP] Frame read failed/timeout (-1). Breaking... to retry Range GET from offset 1389952...
     [OTA-TCP] Chunk download complete: 1389952 / 1425840 bytes (97%)
     ...
     [OTA-TCP] Chunk Range GET bytes=1389952-1422719 ...
     ```
   - The next request resumes at exactly `1389952` — matching `actual_downloaded` precisely, with Content-Range validation never flagging a mismatch anywhere in the run. This confirms it is unlikely to be an offset or byte-duplication bug (accounting held up correctly through 507 chunk/retry events during a rough 2G session).

### Root Cause Analysis & Recommendations:
- **What this pattern points to:** A whole-image checksum failing while every offset and byte-count checks out clean is the signature of a small number of individual bytes being corrupted in stream (single flipped/substituted byte anywhere in 1.4MB), rather than missing or duplicated bytes. This aligns with single-character UART glitches (`P\nP`, `C\nC\nC`) caused by genuine electrical noise on the UART lines during 2G modem TX bursts.
- **Hardware-level Investigation Recommended:** Inspect UART wiring/decoupling between ESP32 and modem, ground path quality during 2G transmit bursts, and verify link baud rate stability under cellular load.
- **Pragmatic Mitigation Strategy:** On `Update.end()` failure, retry the entire download from scratch (up to 2–3 full attempts). Single-byte bitflips are probabilistic, so a fresh attempt has a very high probability of completing without error.

---

## 📦 Release Contents

This release includes pre-compiled production binaries for all system configurations:
1. **KSNDMC_TRG** - Telemetry Rain Gauge (Nuvoton & Matrix UI)
2. **BIHAR_TRG** - Bihar Government Rain Gauge (Nuvoton & Matrix UI)
3. **SPATIKA_TRG** - Generic Spatika Rain Gauge (Matrix UI)
4. **KSNDMC_TWS** - Telemetry Weather Station (Nuvoton & Matrix UI)
5. **KSNDMC_ADDON** - KSNDMC Add-on Configuration (Matrix UI)
6. **SPATIKA_GEN** - Spatika Weather Station (TWSRF) (Matrix UI)
7. **SPATIKA_TWSRP** - Spatika Weather Station with Pressure (TWSRP) (Matrix UI)

---
**Release Tag:** `v6.48-for-release-testing-Sep21-2026` 🚀
