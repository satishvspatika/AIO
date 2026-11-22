# AIO9 v6.61 (22 Nov 2026): replaces 6.60 (never released)

> **Superseded by v6.62** (adds the SD-card hardware check). Use v6.62.

**Type:** production (DEBUG 0), 8 MB flash. **Not yet run on a unit.**

## Builds

| Config | For units | LCD version (`fw_version.txt`) | OTA type | Size (bytes) |
|---|---|---|---|---|
| BIHAR_TRG_NUV | Bihar TRG, Nuvoton LCD/keypad | `TRG9-BIH-6.61-N` | 12 | 1,301,088 |
| BIHAR_TRG_MAT | Bihar TRG, matrix keypad / I2C LCD | `TRG9-BIH-6.61-M` | 13 | 1,300,816 |
| KSNDMC_TRG_NUV | KSNDMC TRG, Nuvoton | `TRG9-DMC-6.61-N` | 10 | 1,301,088 |
| KSNDMC_TRG_MAT | KSNDMC TRG, matrix | `TRG9-DMC-6.61-M` | 11 | 1,300,816 |
| KSNDMC_TWS_NUV | KSNDMC TWS, Nuvoton | `TWS9-DMC-6.61-N` | 14 | 1,314,080 |
| KSNDMC_TWS_MAT | KSNDMC TWS, matrix | `TWS9-DMC-6.61-M` | 15 | 1,313,744 |

App slot on the current partition table: 1,769,472 bytes.

## Folders

| Folder | Used for |
|---|---|
| `OTA/` | Upload to the office page. The office page shows the station type it reads from the file. Set each station to the file of its own type: a unit refuses a file of another type (e=T) and keeps running. |
| `SD_CARD/SD_v6.61_<config>/` | Copy **both** files (`firmware.bin`, `fw_version.txt`) to the root of the card, power-cycle. See `SD_CARD/README.txt`. |
| `FACTORY_USB/<config>/` | A blank board over USB (the four files below). |

## What is new (6.60 + 6.61)

- **The unit tells the office why a download waits** (`w=`: battery below 3.7 V, records still to send), instead of
  the office page showing "Starting download".
- **Why an update was given up** is sent with the check (`e=`): C crashed on trial, N no record for 2 h, B did not
  start, T other station type, K failed the image check, D arrived damaged 3 times. The office page says it in words.
- **Damaged download no longer repeats forever**: a whole-file CRC failure is fetched again at most 3 times (was:
  every slot, 1.3 MB each time, on a flash fault).
- **A unit stopped by SMS (VARSHA STOP) still checks in**: the office sees "STOPPED" in its health instead of a
  station gone silent; its stopped slots no longer count towards the 24 h recovery restart or an update trial.
- **No new download while new firmware is on trial** (the trial and its way back were lost if the office changed the
  target during it).
- **Health report retried**: a missed 13:00 check is asked again hourly until the day's report has gone.
- **Health figures kept over a firmware update** (record layout, not image, decides), **power-ons counted** (`pon`),
  and a clock that went back no longer stops the reports for good.
- OTA reply wait feeds the watchdog every 2 s (was: only when millis() hit a multiple of 2000).

## Factory flashing (blank board)

From the config's `FACTORY_USB/<config>/` folder:

```
esptool.py --chip esp32 --port <PORT> --baud 921600 write_flash --flash_mode keep --flash_freq keep --flash_size 8MB \
  0x1000 bootloader.bin  0x8000 partitions.bin  0xe000 boot_app0.bin  0x10000 firmware.bin
```

`bootloader.bin` and `partitions.bin` come from this build (current `partitions.csv`: app0/app1 1,769,472 bytes each,
SPIFFS at 0x370000). **Do not use `flash_files/8mb/` or the `flash_fresh_board*.sh` scripts for now:** their
partition table dates from Feb 2026 (app slots 1,441,792 bytes, SPIFFS at 0x2D0000) and does not match `partitions.csv`.

## What to test

| # | Test | Expected |
|---|---|---|
| 1 | OTA from 6.59 to 6.61, one unit of each config | Office page: "Updated to 6.61: running and confirmed" |
| 2 | Offer with battery below 3.7 V | Office page: "Waiting: battery low" |
| 3 | Office sets a Matrix build for a Nuvoton unit | Unit refuses it (e=T) and keeps running; office page says "for another station type" |
| 4 | Remove the antenna for over 2 h right after an OTA install | Unit goes back to the previous firmware; office page says why |
| 5 | VARSHA STOP, then wait for 13:00 | Unit still checks in; health shows STOPPED |
| 6 | SD card update from each `SD_CARD` folder | `SD FW SUCCESS! REBOOTING...`, LCD shows the `fw_version.txt` version |
| 7 | Day after install | Health report with power-ons on the office page |

Backup of the 6.59 sources: `.backup_v6.59_otawait/`.
