# Release Notes: v5.32 (Feb 16, 2026)

## 🎯 Overview
Critical stability update addressing BSNL 2G connectivity issues, station ID formatting, and build system reliability.

---

## ✨ New Features

### 1. **BSNL 2G Stability Package** 📶
- **Extended SSL Handshake Timeout:** Increased to **120 seconds** (from 60s) for Google Sheets health reports. This ensures successful handshakes even on low-bandwidth 2G networks.
- **Extended HTTP Action Timeout:** Increased to **45 seconds** for standard data uploads to account for network congestion.
- **Improved SMS Reliability:**
    - Increased `+CMGS` confirmation timeout to **35 seconds**.
    - Added `AT+CSCA?` (Service Center Query) before sending to refresh SMS settings on BSNL SIMs.

### 2. **Automatic Station ID Trimming** ✂️
- **The "Space-Free" SMS:** Station IDs are now automatically trimmed of trailing spaces before being included in the SMS status string.
- **Clean URLs:** HTTP URLs for all system types (TRG, TWS, Add-On) now use trimmed station IDs, preventing backend database mismatches caused by trailing spaces.

---

## 🔧 Bug Fixes

### 3. **Build Binary Isolation Fix** 🛡️
- **Problem:** Previous releases occasionally contained the wrong system binary (e.g., TWS folder containing TRG firmware) due to a shared build cache in the automation script.
- **Solution:** Restructured `build_all_configs.py` to use **isolated build paths** for every configuration (`build/config_KSNDMC_TWS/`, etc.).
- **Impact:** Guaranteed binary accuracy for all 6 configurations.

---

## 📋 Technical Details

### Modified Files
- `gprs.ino` - Implemented timeout extensions, station ID trimming logic, and SMS reliability commands.
- `build_all_configs.py` - Implemented build sandboxing and clean-sweep logic.

### Code Size Impact
- **Program:** ~1,173,000 bytes
- **RAM:** No significant change

---

## 🧪 Testing Recommendations

1. ✅ **BSNL Stress Test:** Verify SMS and Health Report success on a 2G-only signal.
2. ✅ **Formatting Check:** Send a status SMS for a station with a short ID (e.g., "1923") and verify no trailing spaces appear.
3. ✅ **Configuration Check:** Verify TWS firmware correctly reports as TWS in its `build_info.txt`.

---

**v5.32 is a mandatory upgrade for stations using BSNL SIM cards or short Station IDs.** 🚀
