# Release Notes: v6.57 (Nov 19, 2026), bench build, not yet tested on a unit

v6.57 builds on v6.56c (`RELEASE_NOTES_v6.56b.md`, `RELEASE_NOTES_v6.56c.md`). The ESP32 units now
work the same way as the LPC units in two ways: VARSHA SMS commands, and a daily health report that
goes to the OTA server and office page. The separate Spatika health server (Contabo) is no longer
used, and its code and other dead code are removed.

## VARSHA SMS commands (`sms_remote.ino`, `sms_cmd.c/.h`)
The LPC unit's command set and behaviour (AIO_LPC design 7.4b). The text handling is the LPC's own
`sms_cmd.c`, copied unchanged (its host test passes here: `bench/tests/sms_cmd_test.c`).

`VARSHA <CMD> [value]` (any case, `=` optional) or the older `#CMD[=value]`. Replies go to the sender
and start `STN <id>`.

| Command | Who | Does |
|---|---|---|
| `GET_STATUS` | anyone | the status SMS, to the sender |
| `GET_GPS` | anyone | position (fresh network fix), to the sender |
| `GET_CONFIG` | anyone | unit type, firmware, server mode, server 2, number of office numbers, OTA server, STOPPED |
| `SET_TIME YYYYMMDDHHMMSS` | office | sets the clock (rain-day total kept); refused if the SMS is over 15 min old by the network's clock |
| `SET_STN_ID <id>` | office | 1-6 digits (Bihar 1-8); clears the old station's records and backlog, as the keypad does |
| `SET_SERVER_2 host=..;port=..;path=/..;key=..` / `OFF` | office | server 2 |
| `SET_SERVER_MODE 0/1/2` | office | 0 server 1 only, 1 server 2 only, 2 both |
| `SET_OTA_SERVER <host> [port]` / `OFF` | office | where the unit asks for firmware (NVS); OFF = built-in |
| `ADD_NUM` / `DEL_NUM <number>`, `GET_NUM` | office | office number list (up to 5; the last one can't be removed) |
| `STOP` / `RESUME` | office | STOP: nothing sent or queued, SMS still read; RESUME from the next slot |
| `ERASE_DATA <this unit's ID>` | office | wipes records and backlog, then restarts |

- Office numbers: `SMS_OFFICE_NUMBERS` in `user_config.h` (the LPC list) until changed by
  ADD_NUM / DEL_NUM, then kept in NVS (`sms-cfg`).
- The SIM is read **every slot** after the send (was once an hour, at :00), also while sending is
  STOPPED and in slots that skip the send. Up to 4 whole messages a slot, each deleted
  (`AT+CMGD=<i>`) before it runs; more, or one cut off, wait for the next slot.
- Dropped unrun: commands older than 48 h, a message the network delivers twice (same sender,
  time stamp and text), a GET_ repeated by the same sender within 10 min, senders that aren't phone
  numbers. UCS2 messages (phones with a Kannada/Hindi keyboard) are decoded.
- Removed: `SEND_FTP_DAILY_DATA` (FTP is gone) and `SET_FW_FNAME` (the old TCP firmware download;
  updates come from the OTA server).

## Daily health report (`health_day.ino`)
Once a day the OTA check carries the last whole MET day (08:45 to 08:30, by its closing date), as
the LPC units do (v6.9.62): `hs day sent bk bd bn bmin ne err dur bv sv sig net op q sd rst wdt clk
gps`, and `sens` on TWS units. Sent with the 13:00 check (by station number up to 45 min later);
if that fails, with the 01:00 check. The office page shows it as on the LPC stations: a Health card,
health history and "Needs attention" (ESP32 battery limits: below 3.5 V now, 3.7 V night average).
Kept in RTC no-init memory, which survives fault and watchdog restarts (ordinary RTC variables are set back to their starting values by the bootloader on any reset except a deep-sleep wake); a power cut or a firmware update starts it afresh. The first report after an update covers a part day.

## Removed (dead code)
- The Spatika health server report and its remote-command parser (`send_health_report`,
  `parse_health_response`, `set_server_config`) and everything only they used: the
  `ENABLE_HEALTH_REPORT` / `TEST_HEALTH_DEFAULT` settings, the hidden SEND HEALTH menu item,
  `force_ota`/`force_ftp`/`force_ftp_daily`/`force_get_num`/`force_gps_refresh`/
  `force_clear_ftp_queue`/`force_reboot`/`force_health_upload`, `last_cmd_res`/`last_cmd_id`, the
  health schedule and CDM-failure check, `TELEMETRY_TOKEN`, `HEALTH_SERVER_*`.
- `ota_legacy.ino` (the old TCP firmware download) and `ota_silent_mode` with the task pauses that
  waited on it. Log lines it guarded used plain `Serial.printf` and printed even in production
  builds (one printed the whole payload with its key): they are debug-only now.
- `process_sms()` and the old hourly SMS reader, `read_line()`, the unused FTP passwords in
  `secrets.h`.
- Server 2's default was the Contabo health server: now none until set by SMS.

`ERASE_DATA` keeps the existing wipe (`force_delete_data`). `health_in_progress` stays: the clock
resync uses it.

## Build
`FIRMWARE_VERSION "6.57"`. Bench and matrix scripts now also copy `*.c` (`sms_cmd.c`). Compile-checked: BIHAR_TRG Nuvoton, KSNDMC_TRG Nuvoton, KSNDMC_TWS, KSNDMC_ADDON, SPATIKA_GEN S3 (8 MB, no errors). Released for BIHAR_TRG Nuvoton 8 MB: `RELEASE/v6.57/BIHAR_TRG_NUV_8mb/AIO9_v6.57_BIHAR_TRG_NUV.bin`, 1,300,064 bytes, CRC32 CF56041A, image 44D40D56.
