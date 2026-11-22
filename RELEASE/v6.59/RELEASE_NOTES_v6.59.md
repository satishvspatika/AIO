# AIO9 v6.59 (21 Nov 2026)

v6.58 plus one change:

- **OTA keeps the station type.** Before installing a downloaded image the unit reads the station type in its
  AIOOTA1 block and installs it only if it is the unit's own type (ota_esp.ino `ota_image_type`). Another type
  (or no block) is refused, marked failed (office page: FAILED) and not downloaded again.
  An **SD card update is unchanged** and can still change a unit's type/config.

Build: KSNDMC TRG, Nuvoton, DEBUG 0, 8 MB (same as 6.57/6.58).
`KSNDMC_TRG_NUV_8mb/AIO9_v6.59_KSNDMC_TRG_NUV.bin` — 1,300,672 bytes. Not yet run on a unit.
Backup of the 6.58 sources: `.backup_v6.58_typecheck/`.
