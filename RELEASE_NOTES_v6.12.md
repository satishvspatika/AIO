# Release Notes: v6.12 (Jul 17, 2026)

## 🎯 Overview
Release v6.12: Correct battery and solar voltage calculations based on WIND_DIR_ADC_MAX (3480) and loading compensation.

---

## 🔧 Bug Fixes

### 1. **Incorrect Battery & Solar Voltage Calculations** 🔧
- **Problem:** Due to the wind direction calibration introducing `WIND_DIR_ADC_MAX = 3480` to represent VCC (3.3V), ratiometric voltage calculations still using 4095/4096 produced battery and solar readings that were 8% to 15% too low. Additionally, high-impedance resistor dividers (220k/620k for battery, 620k/100k for solar) caused loading effects from the ESP32 ADC pin, causing voltages to read lower.
- **Solution:** Updated the simple division formulas to divide by `WIND_DIR_ADC_MAX` and scale based on 3.3V VCC. Applied loading-compensated divider ratios (1.48 for battery/rail instead of 1.355, and 7.8 for solar instead of 7.2) across both UI and non-UI code paths.
- **Impact:** Battery readings will correctly report up to 4.2V for 3.7V batteries, 3.6V for 3.2V batteries, 3.3V for the main rail, and accurate panel voltage for solar.

---

## 📋 Technical Details

### Modified Files
- `AIO9_5.0.ino` - Updated legacy solar startup calculation in setup
- `scheduler.ino` - Corrected solar voltage checks in the loop
- `gprs_health.ino` - Updated solar voltage calculation in status payload
- `global_functions.ino` - Updated battery, rail, and solar multipliers to use compensated ratios (1.48 and 7.8)
- `TEST_JIG/qc_test/qc_test.ino` - Defined WIND_DIR_ADC_MAX and updated battery, rail, and solar calculations

---

## 📦 Release Contents

This release includes pre-compiled binaries for the selected configuration:

### High-Power (SYSTEM=0) - Solar/Battery
1. **KSNDMC_TRG** - Telemetry Rain Gauge

### Each Configuration Folder Includes:
- `firmware.bin` - Main application binary
- `fw_version.txt` - Version verification file (e.g. TRG9-DMC-6.12-N)
- `metadata.json` - Machine-readable compile settings

---

## 🧪 Testing Recommendations

1. ✅ Verify battery voltage on LCD/Serial matches multimeter reading on the battery pin.
2. ✅ Verify solar voltage on LCD/Serial matches multimeter reading on the solar input pin.
3. ✅ Verify 3.3V rail voltage on LCD/Serial matches 3.3V.

---

## 🔄 Upgrade Path

### From v6.11:
- **Direct upgrade** - Flash v6.12 firmware
- **No configuration changes** required

---

**v6.12 is production-ready!** 🚀
