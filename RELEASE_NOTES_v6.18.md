# Release Notes: v6.18 (Jul 21, 2026)

## 🎯 Overview
Release v6.18: Correction of battery, 3.3V system rail, and solar voltage ADC conversion multipliers in production firmware, restoring true hardware schematic resistor divider ratios and eFuse calibration accuracy. Full 10-configuration multi-build release.

---

## 🔧 Voltage Calculation & Hardware Divider Fixes

### 1. **3.7V Battery & 3.3V Rail Voltage Multiplier Fix**
- **Hardware Divider**: $R_{top} = 220\,\text{k}\Omega$, $R_{bot} = 620\,\text{k}\Omega$.
- **True Hardware Ratio**: $\frac{220 + 620}{620} = \frac{840}{620} = \mathbf{1.3548387}$.
- **Problem**: Code had hardcoded `1.60` (battery) and `1.48` (3.3V rail), causing +18.1% overestimation on calibrated readings (e.g. 3.55V displayed as 3.88V, and 4.13V displayed as 4.73V).
- **Fix**: Replaced hardcoded constants with true ratio `(840.0 / 620.0)` in `global_functions.ino`.

### 2. **Solar Panel Voltage Multiplier Fix**
- **Hardware Divider**: $R_{top} = 620\,\text{k}\Omega$, $R_{bot} = 100\,\text{k}\Omega$.
- **True Hardware Ratio**: $\frac{620 + 100}{100} = \frac{720}{100} = \mathbf{7.2000}$.
- **Fix**: Replaced hardcoded constant `7.80` with true ratio `(720.0 / 100.0)` in `global_functions.ino`.

---

## 🛠️ Build System & Core 3.x Compatibility
- Updated `build_all_configs.py` FQBN parameter formatting to maintain 100% compatibility with ESP32 Core 3.3.6 / 3.3.10 toolchain.
- Fully built and verified all 10 configurations (5 Nuvoton UI + 5 Matrix UI).

---

## 📋 Modified Files
- `user_config.h` — Updated `FIRMWARE_VERSION` to `"6.18"`.
- `global_functions.ino` — Restored true hardware resistor ratios `(840.0 / 620.0)` and `(720.0 / 100.0)`.
- `build_all_configs.py` — Fixed FQBN partition scheme syntax for Core 3.x builds.

---

**v6.18 is ready for official production deployment!** 🚀
