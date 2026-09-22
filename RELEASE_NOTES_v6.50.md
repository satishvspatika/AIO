# Release Notes - v6.50

**Release Date:** September 22, 2026  
**Firmware Version:** `6.50`  
**Git Tag:** `v6.50-for-release-test`  
**Target Hardware:** ESP32-WROOM-32 (8MB Flash)

---

## 🎯 Summary of Changes in v6.50

### 1. 📟 Nuvoton UI Deep Sleep Parasitic Back-Feed & Backlight Isolation (`USE_NUVOTON_UI 1`)
- **Parasitic Power Isolation:** Locked both GPIO 4 (TXD1) and GPIO 32 (5V VCC power rail) to `LEVEL 0` (0V) in the RTC Low-Power domain during deep sleep (`rtc_gpio_set_level(GPIO_NUM_4, 0)` and `rtc_gpio_set_level(GPIO_NUM_32, 0)`).
- **Elimination of Parasitic Back-Feed:** Prevents 3.3V current on TXD1 from back-feeding into Nuvoton's internal RX pin ESD protection diode while 5V main power collapses. Eliminates brownout resets and factory splash screen (`"Spatika Infotech / Bangalore"`) resets upon sleep entry.
- **100% Screen & Backlight Darkening:** LCD screen and backlight LED remain 100% OFF (completely dark) throughout deep sleep and background 15-minute timer wakeups.

### 2. 📊 Updated Signal Strength Sentinel Constants
- Replaced legacy `-143`, `-141`, `-142`, `-144` sentinels with clean standard values in `globals.h`:
  - `SIGNAL_STRENGTH_MISSING_DATA = -103`
  - `SIGNAL_STRENGTH_GAP_FILLED = -104`
  - `SIGNAL_STRENGTH_PREV_DAY_GAP = -105`

### 3. 💾 SD Card Firmware Downgrade Guard Reverted
- Reverted strict version ordering check (`sd_ver <= active_ver`) on SD card OTA engine to enable field engineers to install any firmware version via SD card during testing.

### 4. 🔇 Silenced ESP-IDF UART Driver Warning
- Added `!__atomic_load_n(&sleep_sequence_active, __ATOMIC_ACQUIRE)` atomic guards around `Serial1.available()` polling calls in `lcdkeypad.ino` to eliminate `E uart_get_buffered_data_len ...` driver logs during deep sleep entry.

---

## 📦 Released Build Binaries

| Configuration | UI Type | Target System | Flash Size | Version String |
| :--- | :--- | :--- | :--- | :--- |
| **KSNDMC_TRG_NUV_8mb** | Nuvoton UART LCD | SYSTEM 0 (TRG) | 8MB | `TRG9-DMC-6.50-N` |
| **KSNDMC_TRG_MAT_8mb** | Matrix I2C LCD | SYSTEM 0 (TRG) | 8MB | `TRG9-DMC-6.50-M` |
| **BIHAR_TRG_NUV_8mb** | Nuvoton UART LCD | SYSTEM 0 (TRG) | 8MB | `TRG9-BIH-6.50-N` |
| **BIHAR_TRG_MAT_8mb** | Matrix I2C LCD | SYSTEM 0 (TRG) | 8MB | `TRG9-BIH-6.50-M` |
| **SPATIKA_GEN_MAT_8mb** | Matrix I2C LCD | SYSTEM 2 (TWSRF) | 8MB | `TWSRF9-GEN-6.50-M` |
| **SPATIKA_TWSRP_MAT_8mb** | Matrix I2C LCD | SYSTEM 3 (TWSRP) | 8MB | `TWSRP9-GEN-6.50-M` |
| **KSNDMC_TWS_NUV_8mb** | Nuvoton UART LCD | SYSTEM 1 (TWS) | 8MB | `TWS9-DMC-6.50-N` |
| **KSNDMC_TWS_MAT_8mb** | Matrix I2C LCD | SYSTEM 1 (TWS) | 8MB | `TWS9-DMC-6.50-M` |
| **KSNDMC_ADDON_MAT_8mb** | Matrix I2C LCD | SYSTEM 2 (ADDON) | 8MB | `TWSRF9-DMC-6.50-M` |
