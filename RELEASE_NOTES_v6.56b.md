# Release Notes: v6.56b (Nov 19, 2026)

## Overview
v6.56b builds on v6.56 (see `RELEASE_NOTES_v6.56.md`). It adds firmware updates over the air
(OTA) for the ESP32 units, changes the backlog send, and fixes Bihar data, Bihar SMS and the
blank screen at sleep. Released for BIHAR_TRG Nuvoton 8 MB only (ESP32 core 3.3.6, no errors or
warnings): `RELEASE/v6.56b/BIHAR_TRG_NUV_8mb/AIO9_v6.56b_BIHAR_TRG_NUV.bin`, 1,296,592 bytes,
CRC32 75652575, image ID 85639066. Other configurations not built for this release.

Bench status: OTA tested end to end on one 16 MB Nuvoton unit (KSNDMC_TRG 6.56, BSNL SIM,
19 Nov). As a BIHAR_TRG Nuvoton unit on an Airtel Bihar SIM: data to `rtdasbmsk` delivered
(HTTP 200) and the OTA check was answered. The 6.56b changes below (SMS, 8-digit ID, sleep
message) are not yet tested on a unit.

---

## Since v6.56

### ESP32 OTA (`ota_esp.ino`)
- The unit asks the OTA server (`devctl.spatika.net`, `/lpc/check`, same protocol as the LPC
  units) twice a day, at 13:00 and 01:00 (`OTA_DAY_SLOT`, `OTA_NIGHT_SLOT`, offset by station
  so units do not all ask at once). Office page: ESP32 family tab, types 10-27.
- Download in 4/2/1 KB pieces (adapts to the link), resumes after a power cut, image checked
  (size, CRCs, `.bin` header) before it is set to boot.
- Trial and rollback in the firmware (the units' bootloader has none): a new image is
  "On trial" until it delivers a record; it is confirmed in that same slot ("Updated").
  If it does not deliver within 8 slots, the unit goes back to the old image and the
  office shows FAILED.
- A version change of the same unit type (for example 6.56 to 6.56b, TRG to TRG) keeps the
  rain/wind counters. A change of type still clears the data, as before.

### Backlog send
- The backlog runs on a time budget inside the slot and does not refresh the data bearer
  when it does not need to.

## New in v6.56b

### Bihar (BIHAR_TRG)
- **Server**: `BIHAR_SERVER_SELECT` now defaults to 0, `rtdasbmsk.spatika.net:8085`
  (`/Home/UpdateTRGData`). From the Airtel Bihar SIM, `bmsknew.spatika.net` (1) failed and
  `rtdasbmsk` delivered current and backlog records (19 Nov).
- **Station ID is 8 digits** in `StnNo`, as the LPC Bihar unit sends it (AIO_LPC v6.9.32):
  stored `009983` goes out as `00009983` (`gprs_http.ino`). Other networks unchanged.
  Units already in the field change format when they update: check the Bihar portal.
- **SMS number** `9980945474` (was `9741391102`), the Bihar production number, the same as
  the LPC unit. `BIHAR_TEST` keeps `9741391102`.

### SMS (`gprs_health.ino`, all unit types)
- Status SMS and GPS SMS use one send routine, `sms_send_text()`, matching the LPC unit:
  - `AT+CMGS="<number>"` ends on one CR (`println` added CR LF, which could reach the text
    after the `>` prompt);
  - no CR/LF at the end of the text; 100 ms before Ctrl+Z;
  - waits up to 60 s for `+CMGS` (was 20 s / 35 s);
  - logs the SMS centre and, on failure, the modem's own answer (`+CMS ERROR: <code>`).
- Found on the bench: GPS SMS to 9741391102 from the Airtel IoT SIM was refused by the
  network 3 s after Ctrl+Z.

### Sleep from the keypad (Nuvoton UI)
- MENU > TURN OFF LCD + SET used to blank the screen with the backlight still on, while the
  unit waited (about 30 s) for the modem to finish. Now the screen shows
  `GOING TO SLEEP` / `MODEM OFF..WAIT` (or `PLEASE WAIT...`) until the unit sleeps; if it is
  still awake after 60 s the screen is blanked as before (`lcdkeypad.ino`).

## Settings (user_config.h)
`FIRMWARE_VERSION "6.56b"`, `DEBUG 0`, `OTA_CHECK_EVERY_SLOT 0` (1 only for bench builds),
`ENABLE_ESP_OTA 1`, `OTA_SERVER_HOST "devctl.spatika.net"`, `BIHAR_SERVER_SELECT 0`.

## Field notes
- Airtel M2M KSNDMC SIMs block `187.127.175.183` (ports 80 and 5002): they need it
  whitelisted before OTA can reach them. Airtel Bihar and BSNL SIMs reach it.
- `rtdasbmsk` replies carry no time field, so Bihar units do not set their clock from it.
