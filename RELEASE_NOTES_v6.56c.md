# Release Notes: v6.56c (Nov 19, 2026)

v6.56c is v6.56b (see `RELEASE_NOTES_v6.56b.md`) with one change, so the office page shows an
update's result at once.

Released for BIHAR_TRG Nuvoton 8 MB only (ESP32 core 3.3.6, no errors or warnings):
`RELEASE/v6.56c/BIHAR_TRG_NUV_8mb/AIO9_v6.56c_BIHAR_TRG_NUV.bin`, 1,296,592 bytes,
CRC32 AF118E2C, image ID B316F861. Production settings (DEBUG 0, OTA_CHECK_EVERY_SLOT 0).
Not yet tested on a unit.

## OTA: report right after an update (`ota_esp.ino`)
- A unit normally checks the OTA server twice a day (13:00 and 01:00, up to 45 min later by
  station number). After an update it waited for that check, so the office page could show
  the old version for up to 12 h while the unit already ran the new one (seen on 009983,
  19 Nov: 6.56b running, office still "6.56a, Downloaded").
- Now, while new firmware is on trial, the unit checks every slot ("On trial" shows in the
  next slot), and once more in the slot it is confirmed ("Updated"). Then it goes back to
  twice a day.
- This takes effect for updates *from* 6.56c on: a unit running 6.56b or older still reports
  its update at its next twice-a-day check.
