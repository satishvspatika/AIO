# Release Notes: v5.31 (Feb 16, 2026)

## 🎯 Overview
Major reliability and usability update focusing on health report resilience, GPS management, BSNL network compatibility, and Google Sheets data quality.

---

## ✨ New Features

### 1. **Enhanced Station ID Editing** 📝
- **Space character support** in station ID field
- **Auto-clear on edit** - Field starts blank for easier entry
- **Smart trim & pad** - Automatically formats to 6-character fixed width
- **Use Case:** Change `WS1923` to `1923` by entering `1923` (system pads to `"1923  "`)

### 2. **Bulletproof Health Report System** 🏥
- **Extended time windows** for reliability:
  - Morning: 11:00 AM - 10:59 PM (was: exactly 11:15)
  - Evening: 11:00 PM - 10:59 AM (was: exactly 23:00)
- **RTC-persisted retry logic** - Never miss a report:
  - Retries every 15 minutes until successful
  - Survives deep sleep and power failures
  - Prevents duplicate sends
- **Smart initialization** - Sets flags based on current time on first boot
- **Function signature change:** `send_health_report()` now returns `bool` for success/failure tracking

### 3. **Intelligent GPS Cache Management** 🗺️
- **Auto-delete GPS cache** when station ID changes
- **Forces fresh GPS fix** for new field installations
- **Prevents wrong coordinates** from being transmitted
- **Automatic CLBS fallback** - Health reports now automatically attempt GPS fix if coordinates missing:
  1. Load from cache (`/gps_coords.txt`)
  2. If cache empty → Try fresh CLBS fix (`AT+CLBS=1`)
  3. If CLBS fails → Log warning and send 0,0
- **Warning logs** if sending with 0,0 coordinates

---

## 🔧 Bug Fixes

### 4. **BSNL Registration Fix** 🔧
- **Problem:** `AT+CREG=1` was sent inside retry loop (every 10s), confusing BSNL modems
- **Symptom:** Status 3 (Registration Denied) after ~60 seconds
- **Solution:** Send `AT+CREG=1` once before loop, not every iteration
- **Impact:** BSNL SIMs now register successfully within 6 iterations

### 5. **Google Sheets Data Quality** 📊
- **Station ID trimming** - Removes trailing spaces before URL encoding
  - Before: `1934  ` → `1934__` (underscores)
  - After: `1934  ` → `1934` ✅ (clean)
- **Mobile number formatting** - Strips +91 country code prefix
  - Before: `+919449014011` → `919449014011`
  - After: `+919449014011` → `9449014011` ✅ (clean 10 digits)
  - IMSI (15 digits) preserved as-is

### 6. **Network Diagnostics** 🔍
- Added IMEI logging for troubleshooting IMEI lock issues
- Added network operator logging (`AT+COPS?`)
- Helps identify registration failures faster

---

## 📋 Technical Details

### Modified Files
- `globals.h` - Added RTC persistent variables, space to AlphaNum charset
- `gprs.ino` - Health report retry logic, GPS auto-fix, BSNL registration fix, mobile number formatting
- `lcdkeypad.ino` - Station ID editing enhancements, GPS cache deletion

### Code Size Impact
- **Program:** 1,171,271 bytes (6.98% of 16MB flash) - **+1,164 bytes from v5.3**
- **RAM:** 53,544 bytes (16.3% of 327KB SRAM) - **No change**
- **Impact:** Negligible (<0.01% increase)

### New RTC Variables (Persistent Across Deep Sleep)
```cpp
RTC_DATA_ATTR bool health_morning_sent = false;
RTC_DATA_ATTR bool health_evening_sent = false;
RTC_DATA_ATTR int health_last_reset_day = -1;
```

---

## 🧪 Testing Recommendations

1. ✅ **Station ID editing** - Try changing to 4-character ID (e.g., `1923`)
2. ✅ **Health report retry** - Disconnect network during report window, verify retry
3. ✅ **GPS cache deletion** - Change station ID, verify GPS cache deleted
4. ✅ **BSNL registration** - Test with BSNL SIM, verify registration within 60s
5. ✅ **Google Sheets** - Verify station ID shows without trailing underscores
6. ✅ **Mobile number** - Verify 10-digit format (no +91 prefix)
7. ✅ **Automatic GPS** - Delete GPS cache, wait for health report, verify CLBS auto-fix

---

## 🔄 Upgrade Path

### From v5.3:
- **Direct upgrade** - Flash v5.31 firmware
- **No configuration changes** required
- **Automatic migration** - Health report flags initialize on first boot

### Breaking Changes:
- **None** - Fully backward compatible

---

## 📦 Release Contents

This release includes pre-compiled binaries for all system configurations:

### System Configurations:
1. **KSNDMC_TRG** - Telemetry Rain Gauge (SYSTEM=0)
2. **KSNDMC_TWS** - Telemetry Weather Station (SYSTEM=1)
3. **SPATIKA_GEN** - Generic Spatika System (SYSTEM=2)
4. **KSNDMC_ADDON** - Add-on Configuration (SYSTEM=3)
5. **BIHAR_TRG** - Bihar Rain Gauge (SYSTEM=4)

### Each Configuration Includes:
- `firmware.bin` - Main application binary
- `bootloader.bin` - ESP32 bootloader
- `partitions.bin` - Partition table
- `flash_instructions.txt` - Flashing guide

---

## 🐛 Known Issues

- **CLBS GPS** may fail in areas without cell tower triangulation support
- **BSNL USSD** (*222#, *1#) may timeout in some circles
- **SD Card** detection still shows "FAILED" (hardware limitation, not a bug)

---

## 🙏 Acknowledgments

- **BSNL registration fix** - Identified through field testing with multiple SIM providers
- **GPS auto-fix** - Requested feature for new station deployments
- **Google Sheets formatting** - User feedback on data quality

---

## 📞 Support

For issues or questions:
- Check logs for diagnostic messages (e.g., `[Health]`, `[GPS]`, `[DIAG]`)
- Review Google Sheets health report for system status
- Contact development team with Station ID and timestamp

---

**v5.31 is production-ready and recommended for all deployments!** 🚀
