# Release Notes: v6.13 (Jul 20, 2026)

## 🎯 Overview
Release v6.13: Precise calibration multipliers for 3.7V Battery, 3.2V/3.3V MCU Rail, and Solar Panel voltage measurement based on physical multimeter measurements.

---

## 🔧 Voltage Calibration Updates

### 1. **3.7V Li-Ion Battery Calibration**
- **Multimeter Target:** 4.12 V
- **Previous Display:** 3.78 V
- **New Multiplier:** `1.613` (Adjusted from 1.48 $\times \frac{4.12}{3.78}$)
- **Result:** Displays exact 4.12 V matching multimeter.

### 2. **3.2V / 3.3V System Rail Calibration**
- **Multimeter Target:** 3.49 V
- **Previous Display:** 3.35 V
- **New Multiplier:** `1.542` (Adjusted from 1.48 $\times \frac{3.49}{3.35}$)
- **Result:** Displays exact 3.49 V matching multimeter.

### 3. **Solar Panel Voltage Calibration**
- **Multimeter Target:** 14.80 V
- **Previous Display:** 13.96 V
- **New Multiplier:** `8.27` (Adjusted from 7.80 $\times \frac{14.80}{13.96}$)
- **Result:** Displays exact 14.80 V matching multimeter.

---

## 📋 Modified Files
- `user_config.h` - FIRMWARE_VERSION set to `"6.13"`
- `global_functions.ino` - Calibrated multipliers applied to `read_and_calibrate_voltages()` and non-UI battery read
- `AIO9_5.0.ino` - Updated legacy solar startup calculation with factor `8.27`
- `gprs_health.ino` - Updated solar calculation with factor `8.27`
- `scheduler.ino` - Updated solar calculations with factor `8.27`
- `TEST_JIG/qc_test/qc_test.ino` - Calibrated multipliers `1.613`, `1.542`, `8.27` applied to QC test ADC reads

---

**v6.13 is ready for deployment!** 🚀
