# Release Notes: v6.58 (Nov 21, 2026), built for OTA, not yet run on a unit

v6.58 = v6.57 plus one change: the daily health report's figures are also kept in NVS.

- **Why**: 001992 (KSNDMC TRG Nuvoton, 6.57 by SD card on 20 Nov) checked in at 13:02 on 21 Nov without
  a health report. 6.57 kept the day's figures only in RTC memory, which a power-on clears (battery
  change, SD card swap with the power off, power cut), so the closed day was lost.
- **Change** (`health_day.ino`): figures saved to NVS namespace "health" at every deep sleep and after a
  report; restored at a power-on if they belong to the same firmware image. New firmware still starts
  afresh. Nothing else changed.

Build: KSNDMC_TRG, USE_NUVOTON_UI 1, DEBUG 0, 8 MB layout (same as the 6.57 SD package), shared
toolchain (core 3.3.6). `KSNDMC_TRG_NUV_8mb/AIO9_v6.58_KSNDMC_TRG_NUV.bin`, 1,300,528 bytes; the office
OTA server reads it as ESP32 · KSNDMC TRG · Nuvoton, 6.58. Other types/variants: not built yet.
Sources before the change: `.backup_v6.57_health/`.
