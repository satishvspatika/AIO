# AIO9 v6.63 (22 Nov 2026): SD card update fixes

**Type:** production (DEBUG 0), 8 MB flash. **Not yet run on a unit.** Everything in 6.62 is kept.

## Builds

| Config | For units | LCD version (`fw_version.txt`) | OTA type | Size (bytes) | CRC32 |
|---|---|---|---|---|---|
| BIHAR_TRG_NUV | Bihar TRG, Nuvoton LCD/keypad | `TRG9-BIH-6.63-N` | 12 | 1,303,504 | 760A6E9B |
| BIHAR_TRG_MAT | Bihar TRG, matrix keypad / I2C LCD | `TRG9-BIH-6.63-M` | 13 | 1,303,488 | EDE2277A |
| KSNDMC_TRG_NUV | KSNDMC TRG, Nuvoton | `TRG9-DMC-6.63-N` | 10 | 1,303,504 | 0D6907C2 |
| KSNDMC_TRG_MAT | KSNDMC TRG, matrix | `TRG9-DMC-6.63-M` | 11 | 1,303,488 | 4F1F8DF4 |
| KSNDMC_TWS_NUV | KSNDMC TWS, Nuvoton | `TWS9-DMC-6.63-N` | 14 | 1,316,608 | 9D6F1A6C |
| KSNDMC_TWS_MAT | KSNDMC TWS, matrix | `TWS9-DMC-6.63-M` | 15 | 1,316,384 | 9C1E2B69 |

App slot on the current partition table: 1,769,472 bytes. All images are below 1,441,792 bytes, so units on the
Feb 2026 table can take them too.

## Folders

| Folder | Used for |
|---|---|
| `OTA/` | Upload to the office page. Set each station to the file of its own type: a unit refuses a file of another type (e=T) and keeps running. |
| `SD_CARD/SD_v6.63_<config>/` | Copy **both** files (`firmware.bin`, `fw_version.txt`) to the root of the card, power-cycle. See `SD_CARD/README.txt`. |
| `FACTORY_USB/<config>/` | A blank board over USB (the four files below). |

## What is new in 6.63

These change how a unit decides about an SD card, so they protect units from 6.63 on. A unit on 6.62 or older still
decides the old way.

- **Whole-name version match.** A card is skipped as "same version" only when its `fw_version.txt` names exactly the
  firmware the unit runs (`TRG9-DMC-6.63-N`, `TRG9-DMC-6.63`, `6.63`, `v6.63` or `6.63-N`). Up to 6.62 the version
  was looked for anywhere in the name: a 6.56 unit skipped a `6.56b` card, a 6.7 unit would skip a `6.70` card.
  A Matrix card on a Nuvoton unit of the same version now goes to the hardware check and is refused (was: skipped).
- **Failures show on the LCD.** If the update cannot start: `SD FW FAILED! / CANNOT START` (was: stayed at
  `PROGRESS: 0%` and the unit carried on). At power-on a skipped card says why for 2 s: `SD FW SKIPPED:` with
  `SAME VERSION`, `ALREADY INSTALLD` or `TRIED TWICE`. Timer and key wakes show nothing, so a card left in a unit
  does not hold up the LCD.
- **"Already installed" is believed only while the unit still runs it.** After an SD install the unit now also stores
  the installed image ID (`/sd_fw_img.txt`, the ID the office OTA uses). If the unit has been flashed over USB since,
  it installs the card again (up to 6.62 such a unit ignored that card for good). The same file is installed again
  at most once: if it does not stay (rolled back), the unit stops and shows `TRIED TWICE`. For a file installed by
  6.62 or older, the version stored at that install decides. `/sd_fw_img.txt` is kept over a type change.

## Compatibility

| Route | Works? | Why |
|---|---|---|
| OTA 6.59 / 6.61 / 6.62 → 6.63 | Yes | Same type numbers and AIOOTA1 block. |
| SD card 6.62 or older → 6.63 | Yes | The version differs and the file is new to the unit. |
| SD card 6.63 → older builds (6.57 and up) | Yes | They carry the type block. |
| SD card 6.63 → 6.56 or older | Only if `fw_version.txt` ends in `-N`/`-M` and matches the unit's display | No type block; the ending is the only sign of the display. |
| SD card on a 6.63 unit, later builds | Yes | Keep the AIOOTA1 block, the type numbers and the `-N`/`-M` ending in `fw_version.txt`. Avoid letter releases (`6.63b`): units on 6.62 or older would skip them. |
| 6.63 goes back to older firmware | Yes | The older firmware ignores `/sd_fw_img.txt`. |
| Units on the Feb 2026 partition table (1,441,792-byte app slot) | Yes (6.63: 1.32 MB) | A bigger image is refused by OTA and fails cleanly by SD. |

A unit that a 6.62 or older firmware already refuses to update by SD (it took that file once, then was flashed over
USB) needs one USB install, or `/sd_fw_md5.txt` deleted, before it takes the card. From 6.63 on this fixes itself.

## Factory flashing (blank board)

From the config's `FACTORY_USB/<config>/` folder:

```
esptool.py --chip esp32 --port <PORT> --baud 921600 write_flash --flash_mode keep --flash_freq keep --flash_size 8MB \
  0x1000 bootloader.bin  0x8000 partitions.bin  0xe000 boot_app0.bin  0x10000 firmware.bin
```

`bootloader.bin`, `partitions.bin` and `boot_app0.bin` are the same as in 6.62 and `flash_files/8mb/`.

## What to test

| # | Test | Expected |
|---|---|---|
| 1 | OTA from 6.62 to 6.63, one unit of each config | Office page: "Updated to 6.63: running and confirmed" |
| 2 | SD card update from each `SD_CARD` folder | `SD FW SUCCESS! REBOOTING...`, LCD shows the `fw_version.txt` version |
| 3 | Same card again, power-cycle | `SD FW SKIPPED: SAME VERSION` for 2 s, unit starts normally |
| 4 | SD-install 6.63, flash 6.62 over USB, then flash 6.63 over USB with a 6.62 card in | 6.63 installs the 6.62 card (`SD FW UPDATING..`) |
| 5 | SD: KSNDMC TWS Nuvoton card in a KSNDMC TRG Nuvoton unit | Installs; unit is now TWS9-DMC-6.63-N |
| 6 | SD: any Matrix card in a Nuvoton unit (and the reverse) | `SD FW REFUSED: OTHER HARDWARE`, unit keeps its firmware |
| 7 | SD: an image too big for the slot (any file over 1,769,472 bytes, renamed `firmware.bin`) | `SD FW FAILED! CANNOT START`, unit keeps its firmware |
| 8 | Card left in, timer and key wakes | No SD message, normal LCD |

Sources: git tag/commit of this release on `main` (https://github.com/satishvspatika/AIO).
