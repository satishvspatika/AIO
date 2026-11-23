# AIO9 v6.64 (23 Nov 2026): SEND STATUS also reaches the office page

**Type:** production (DEBUG 0), 8 MB flash. **Not yet run on a unit.** Everything in 6.63 is kept.

## Builds

| Config | For units | LCD version (`fw_version.txt`) | OTA type | Size (bytes) | CRC32 |
|---|---|---|---|---|---|
| BIHAR_TRG_NUV | Bihar TRG, Nuvoton LCD/keypad | `TRG9-BIH-6.64-N` | 12 | 1,304,240 | 49FF532B |
| BIHAR_TRG_MAT | Bihar TRG, matrix keypad / I2C LCD | `TRG9-BIH-6.64-M` | 13 | 1,304,160 | 93C67A1E |
| KSNDMC_TRG_NUV | KSNDMC TRG, Nuvoton | `TRG9-DMC-6.64-N` | 10 | 1,304,240 | 039BD10F |
| KSNDMC_TRG_MAT | KSNDMC TRG, matrix | `TRG9-DMC-6.64-M` | 11 | 1,304,160 | 6D083075 |
| KSNDMC_TWS_NUV | KSNDMC TWS, Nuvoton | `TWS9-DMC-6.64-N` | 14 | 1,317,360 | 19830936 |
| KSNDMC_TWS_MAT | KSNDMC TWS, matrix | `TWS9-DMC-6.64-M` | 15 | 1,317,088 | B2F096D0 |

All images are below 1,441,792 bytes, so units on the Feb 2026 partition table can take them too.
`FACTORY_USB/` bootloader, partitions and boot_app0 are the same files as v6.63.

## Folders

| Folder | Used for |
|---|---|
| `OTA/` | Upload to the office page. Set each station to the file of its own type (a unit refuses another type, e=T). |
| `SD_CARD/SD_v6.64_<config>/` | Copy **both** files (`firmware.bin`, `fw_version.txt`) to the root of the card, power-cycle. See `SD_CARD/README.txt`. |
| `FACTORY_USB/<config>/` | A blank board over USB. |

## What is new in 6.64

- **SEND STATUS on the keypad also tells the office page**, right after the status SMS and with the modem still on:
  firmware version, battery, signal, network (2G/4G), operator, records waiting and SD card. The station page shows
  it as "Last SEND STATUS on the unit", with an event in the station's history.
- **The screen shows both results** for 5 s: `SMS+OFFICE OK`, `SMS OK,OFFICE NO`, `SMS NO,OFFICE OK` or
  `SMS+OFFICE FAIL`. A technician on site knows at once whether the unit reaches the office server.
- It is kept apart from the daily health report and never starts a firmware download (the office answers
  `OTA NONE`); downloads still happen only at the unit's own checks.
- Needs the office page update (AIO_LPC `ota_server/lpc_ota.py`, `./deploy_now.sh`) to show the report.

## What to test
1. SEND STATUS with a working SIM: SMS arrives, screen shows `SMS+OFFICE OK`, station page shows the report.
2. SEND STATUS with the antenna off: the failure is shown and the unit carries on and sleeps normally.
3. OTA from 6.63 to 6.64 on one unit of each display type.
