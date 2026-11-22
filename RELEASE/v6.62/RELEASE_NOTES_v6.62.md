# AIO9 v6.62 (22 Nov 2026): replaces 6.60 and 6.61 (never released)

**Type:** production (DEBUG 0), 8 MB flash. **Not yet run on a unit.**

## Builds

| Config | For units | LCD version (`fw_version.txt`) | OTA type | Size (bytes) |
|---|---|---|---|---|
| BIHAR_TRG_NUV | Bihar TRG, Nuvoton LCD/keypad | `TRG9-BIH-6.62-N` | 12 | 1,302,080 |
| BIHAR_TRG_MAT | Bihar TRG, matrix keypad / I2C LCD | `TRG9-BIH-6.62-M` | 13 | 1,301,872 |
| KSNDMC_TRG_NUV | KSNDMC TRG, Nuvoton | `TRG9-DMC-6.62-N` | 10 | 1,302,080 |
| KSNDMC_TRG_MAT | KSNDMC TRG, matrix | `TRG9-DMC-6.62-M` | 11 | 1,301,872 |
| KSNDMC_TWS_NUV | KSNDMC TWS, Nuvoton | `TWS9-DMC-6.62-N` | 14 | 1,315,120 |
| KSNDMC_TWS_MAT | KSNDMC TWS, matrix | `TWS9-DMC-6.62-M` | 15 | 1,314,768 |

App slot on the current partition table: 1,769,472 bytes.

## Folders

| Folder | Used for |
|---|---|
| `OTA/` | Upload to the office page. The office page shows the station type it reads from the file. Set each station to the file of its own type: a unit refuses a file of another type (e=T) and keeps running. |
| `SD_CARD/SD_v6.62_<config>/` | Copy **both** files (`firmware.bin`, `fw_version.txt`) to the root of the card, power-cycle. See `SD_CARD/README.txt`. |
| `FACTORY_USB/<config>/` | A blank board over USB (the four files below). |

## What is new (6.60 + 6.61 + 6.62)

- **6.62: an SD card update can change the station type, never the display hardware.** Before installing, the unit reads
  the display the card's firmware is built for (from its type block, or the `-N`/`-M` ending of `fw_version.txt` for
  firmware without one). KSNDMC TWS Nuvoton on a KSNDMC TRG or BIHAR TRG Nuvoton unit installs; Nuvoton firmware on a
  Matrix unit (or the reverse), DWLR firmware, or a file whose display cannot be told is refused: LCD
  `SD FW REFUSED: / OTHER HARDWARE`. The refused file is remembered (by MD5), so a wrong card left in the unit is not
  read again at every wake; the LCD message still shows at power-on or a key wake. A board whose display was changed
  must be flashed over USB (`FACTORY_USB/`). The check is in the firmware the unit is running, so it protects units
  from 6.62 on: a unit still on 6.59 or older installs any SD card as before.
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

## Compatibility

| Route | Works? | Why |
|---|---|---|
| OTA 6.59 / 6.61 → 6.62 | Yes | Same type numbers and AIOOTA1 block; the older unit's own checks accept it. |
| OTA 6.62 → later builds | Yes | As long as later builds keep the AIOOTA1 block and the type numbers (they are made by the same `ota_esp.ino`). |
| 6.62 goes back to the previous firmware | Yes | The OTA state is stored as separate NVS keys: the older firmware reads its own keys, ignores `why`, and does not take the refused image again. |
| SD card on a 6.62 unit, later builds | Yes | The display is read from the type block (10-27: even Nuvoton, odd Matrix). A type added later, or no block, falls back to the `-N`/`-M` ending of `fw_version.txt`: keep that ending in every release. |
| SD card on a 6.62 unit, older builds (6.57, 6.59, 6.61) | Yes | They carry the type block (checked on the 6.57 and 6.61 SD packages). |
| Health figures over the update | Reset once if the record layout differs, otherwise kept | Layout check, as in 6.61. |
| Units on the Feb 2026 partition table (1,441,792-byte app slot) | Yes while firmware stays below 1,441,792 bytes (6.62: 1.32 MB) | A bigger image is refused by OTA (size check) and fails cleanly by SD (`SD FW FAILED! CHECK BINARY`), never half-written. |

Rules for future releases: keep `ota_esp.ino`'s AIOOTA1 block and `ota_type_of()` numbering (new AIO9 types as
base + 1 for Matrix, outside 28-31 which are DWLR), and keep the `-N`/`-M` ending in `fw_version.txt`.

## Factory flashing (blank board)

From the config's `FACTORY_USB/<config>/` folder:

```
esptool.py --chip esp32 --port <PORT> --baud 921600 write_flash --flash_mode keep --flash_freq keep --flash_size 8MB \
  0x1000 bootloader.bin  0x8000 partitions.bin  0xe000 boot_app0.bin  0x10000 firmware.bin
```

`bootloader.bin` and `partitions.bin` come from this build (current `partitions.csv`: app0/app1 1,769,472 bytes each,
SPIFFS at 0x370000), the same as `flash_files/8mb/` (refreshed 22 Nov 2026: it held a Feb 2026 table with 1,441,792-byte
app slots; `flash_files/16mb/` held the 8 MB table). The old files are in `flash_files_backup_20261122/`.

## What to test

| # | Test | Expected |
|---|---|---|
| 1 | OTA from 6.59 to 6.62, one unit of each config | Office page: "Updated to 6.62: running and confirmed" |
| 2 | Offer with battery below 3.7 V | Office page: "Waiting: battery low" |
| 3 | Office sets a Matrix build for a Nuvoton unit | Unit refuses it (e=T) and keeps running; office page says "for another station type" |
| 4 | Remove the antenna for over 2 h right after an OTA install | Unit goes back to the previous firmware; office page says why |
| 5 | VARSHA STOP, then wait for 13:00 | Unit still checks in; health shows STOPPED |
| 6 | SD card update from each `SD_CARD` folder | `SD FW SUCCESS! REBOOTING...`, LCD shows the `fw_version.txt` version |
| 6a | SD: KSNDMC TWS Nuvoton card in a KSNDMC TRG Nuvoton unit | Installs; unit is now TWS9-DMC-6.62-N |
| 6b | SD: any Matrix card in a Nuvoton unit (and the reverse) | `SD FW REFUSED: OTHER HARDWARE`, unit keeps running its firmware; no message on later timer wakes |
| 7 | Day after install | Health report with power-ons on the office page |

Backups: 6.59 sources `.backup_v6.59_otawait/`, 6.61 sources `.backup_v6.61_sdcheck/`.
