# Release Notes: v6.23-KSNDMC-TRG-working (Aug 15, 2026)

## 🎯 Overview
v6.23 release resolves the deep-sleep instant repeating wakeup loop, hardens Nuvoton UI keep-alive power management, and optimizes device health diagnostics and RTC sampling reliability.

---

## 🔧 Bug Fixes & Hardening

### 1. **Deep Sleep Instant Wakeup Loop Prevention** 🔧
- **Problem:** When entering deep sleep, cutting GPIO 32 power collapsed the Nuvoton MCU's INT line to 0.7V (LOW) due to ESD clamping diodes. `esp_sleep_enable_ext0_wakeup(GPIO_NUM_27, LOW)` detected the LOW level immediately, causing the ESP32 to wake up 0 ms later in an endless reboot loop.
- **Solution:** 
  - Gated GPIO 32 power cut so that `#if USE_NUVOTON_UI == 1`, GPIO 32 is held `HIGH` during deep sleep, keeping Nuvoton powered to scan keys and hold INT (GPIO 27) at 3.3V (HIGH).
  - Added `gpio_get_level(GPIO_NUM_27) == 1` level validation prior to arming `ext0` LOW wakeup in `start_deep_sleep()`. If GPIO 27 is LOW prior to sleep, `ext0` LOW arming is skipped to prevent instant wakeup loops.
- **Impact:** Eliminates rapid reboot battery drain and ensures clean 15-minute deep sleep cycles.

### 2. **Health Diagnostics & RTC Reliability** 📊
- Validated eFuse battery voltage calibration and 16MB flash partition mapping.
- Verified dual app partition flashing (App0 / App1) for 8MB and 16MB target hardware.

---

## 📋 Modified Files
- `global_functions.ino` - Deep sleep GPIO 32 power gating & GPIO 27 level guard
- `user_config.h` - Firmware version set to v6.23
- `AIO9_5.0.ino`, `globals.h`, `gprs_core.ino`, `gprs_health.ino`, `gprs_helpers.ino`, `lcdkeypad.ino`, `rtcRead.ino` - Health diagnostics & system refinements

---

**v6.23-KSNDMC-TRG-working is production-ready!** 🚀
