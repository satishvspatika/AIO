# Release Notes: v5.92 (May 25, 2026)

## 🎯 Overview
Version 5.92 is a baseline version configuration release for the high-resolution ULP wind counting deployment. It maintains all architectural, hardware calibration, and stabilization improvements implemented in v5.91 under a unified `5.92` version identifier.

---

## ✨ Key Technical Specifications Retained from v5.91
- **1ms High-Resolution Wind Counting**: Wind ULP sampling period configured to `1000µs` (`WIND_SAMPLING_US`) to prevent pulse aliasing and speed dips on narrow-teeth cups at high RPM.
- **Scaled Rain Debounce**: Rain bucket debounce increased to `10` cycles to preserve the 10ms electrical noise filter window under the faster 1ms sampling rate.
- **Precision Wind Calibration**: WS calibration factor set to `0.45` to match physical anemometer physical cup rotation metrics.
- **Legacy Cleanups**: Consolidation of configuration parameters into `WIND_TEETH_COUNT` and complete removal of redundant variables.

---

## 🔧 Technical Baseline Details
- **Program Size (8MB Configuration):** 1,168,699 bytes (66% of program storage)
- **RAM Usage:** 57,940 bytes (17% of dynamic memory)
- **Target Hardware:** ESP32 TRG/TWS standard production boards (I2C/GPIO Matrix UI configuration).

---

**v5.92 is production-ready!** 🚀
