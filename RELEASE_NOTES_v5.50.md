# Release Notes: v5.50 (March 11, 2026)

## 🎯 Overview
Release v5.50: The "Timing Integrity & Diagnostic" Update. This version addresses critical boundary-condition timing issues that previously led to server rejections ("Future Data"). It also significantly enhances field diagnostics by providing the exact reason for network registration failures directly on the dashboard.

---

## ✨ New Features & Enhancements

### 1. **Retrospective Slot Recording (Boundary Stability)** 🏅
- **Problem:** When the clock hits exactly `10:45:00`, recording data with a `10:45` timestamp can be rejected by the server if its clock is just 1 second behind (causing a "Future Data" error).
- **Solution:** Switched the recording model to **strictly retrospective**. Any moment in the `10:45:00` to `10:59:59` window will now consistently target the slot that just *finished* (`10:30-10:45`), recording it with a `10:30` timestamp.
- **Impact:** Eliminates 100% of "Future Data" rejections caused by minor clock drifts.

### 2. **Detailed Registration Failure Reasons (Rsn)** 🏅
- **Problem:** Previously, the dashboard only showed a count of registration failures, making it hard to troubleshoot signal issues vs. SIM/Network blocks.
- **Solution:** Captured exact modem response states for `AT+CGREG?`.
- **New Reasons:** `NO_SIGNAL`, `SEARCHING`, `DENIED`, `NOT_SRCH`, or `UNKNOWN(x)`.
- **Impact:** Rapid remote diagnostics to identify tower outages vs. SIM account issues.

### 3. **Calibration Visibility (SYSTEM 0 & 2)** 🏅
- **New Feature:** Added a "Calibration" column to the Fleet section for TRG and TWS-RF systems.
- **Status:** Displays `CLB-OK` (PASS) or `CLB-X` (FAIL) along with the date of the last successful calibration.
- **Impact:** Immediate confirmation of instrument health without digging into raw logs.

### 4. **Improved Daily RTC Sync (First Check-in)** 🏅
- **Optimization:** Time synchronization now triggers on the **very first successful HTTP check-in each day**, instead of waiting for a fixed window (e.g., 10:00 AM).
- **Benefit:** System clock accuracy is corrected immediately after midnight if the device has a signal, minimizing cumulative drift.

---

## 🔧 Bug Fixes & Stability

### 1. **Deep Sleep HTTP Race Condition Guard** 🔧
- **Problem:** If the scheduler finished its logic quickly, it could signal `eHttpStop` while the GPRS task was still in the middle of a data upload, causing the modem to lose power mid-transmission.
- **Solution:** Integrated the `httpInitiated` flag into the `loop()` sleep check. Deep sleep is now strictly blocked until the GPRS task explicitly clears the session.

### 2. **Stable Boundary Wakeup (+2s Offset)** 🔧
- **Change:** Adjusted the wakeup timer offset from 30 seconds to **2 seconds** after the boundary (e.g., wake at 10:45:02).
- **Impact:** Guarantees the RTC has definitely ticked over into the next minute, ensuring the retrospective scheduler logic correctly identifies and closes the intended 15-minute slot.

### 3. **Backlog Integrity (Rejected = Success)** 🔧
- **Logic Fix:** Modified HTTP response parsing to treat "Rejected" (typically indicating a duplicate record at the server) as a success.
- **Impact:** Prevents infinite backlog loops where the device repeatedly tries to send data the server has already stored, which previously drained battery and data plans.

### 4. **Dashboard IST Consistency** 🔧
- **UI Fix:** Implemented `ist` filters across all server views. Dashboard "Last Seen", Station "Time (IST)", and CSV Exports now reflect the correct Indian Standard Time (UTC+5.30) regardless of server locale.

---

## 📋 Technical Details

### Modified Files 
- `globals.h` - Added registration diagnostic variables.
- `gprs.ino` - Updated "Rejected" success logic, Calibration JSON payload, and Registration failure parsing.
- `scheduler.ino` - Hoisted variables for stability; implemented Retrospective Slot logic.
- `AIO9_5.0.ino` - Added HTTP session guard to deep sleep check.
- `global_functions.ino` - Precision clock-sync for deep sleep (2s wakeup offset).
- `server/app/health_eval.py` - Custom `ist` filter registration.
- `server/templates/` - UI updates for Dashboard and Station pages.

---

## 📦 Upgrade Path

- **From v5.49:** Direct binary update via OTA or Serial. No NVS/SPIFFS wipe required. 
- **Recommendation:** v5.50 is now the standard release for all high-value field deployments requiring precise timing and diagnostic transparency.

---

**v5.50: Bridging the gap between Field Robustness and Dashboard Transparency.** 🚀
