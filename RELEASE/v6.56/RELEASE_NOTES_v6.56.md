# Release Notes: v6.56 (Nov 18, 2026) — bench build, not yet field-tested

## Overview
v6.56 fixes the data-loss and stuck-unit problems found in the v6.55 audit, removes FTP, and
sends every system's backlog by HTTP. It builds without errors or warnings for all 11
configurations (KSNDMC_TRG, BIHAR_TRG, SPATIKA_GEN S2/S3, KSNDMC_TWS, KSNDMC_ADDON; Matrix
and Nuvoton UI; 4/8/16 MB) on ESP32 core 3.3.6. Bench-run on one KSNDMC_TRG Nuvoton unit
(Airtel M2M SIM).

Note: the version change from 6.55 resets the rain counters at first boot, as for every
version change.

---

## Critical
- **C-1 Manual send kept the modem lock** (`gprs_core.ino`): if the slot left the modem in
  storing-only mode, an LCD send never released `modemMutex` and the unit never slept.
- **C-2 Corrupt `firmware.bin` stopped setup** (`AIO9_5.0.ino`): `setup()` returned before
  any task started. The file is now deleted and the unit restarts.

## High: data loss
- **H-1 Rain tips lost at the slot boundary** (`scheduler.ino`): the ULP rain counter is no
  longer zeroed after each slot; LCD and web show tips since the last slot
  (`rf_pulses_since_slot()`).
- **H-2 Rejected or failed posts counted as delivered** (`gprs_http.ino`): success only on
  HTTP 200-202 with no Rejected/Invalid in the body. A rejected current record goes to the
  backlog; a rejected backlog record is skipped so it cannot block the queue.
- **H-3 Clock never corrected**: the RTC is synced once a day from the server reply (body
  `tm` field or HTTP Date header), and at once after a rejection.
- **H-4 Backlog pointer wrong after a trim** (`global_functions.ino` `pruneFile`): records
  already sent are dropped first; unsent ones only if the backlog is still full. The
  pointer moves with the data, also during a send (`advance_backlog_pointer`). The file
  lock is held from start to end (a record written between delete and rename lost the
  backlog).
- **Backlog kept 3 days, now 30** (`BACKLOG_MAX_RECORDS` 2880, at most 205 KB).
- **H-5 FTP-to-HTTP migration deleted the source on a failed copy**: checked for room
  first, every write checked, source removed only after a complete copy.
- **H-6 Nothing recorded below 3.4 V**: records are written down to 3.2 V
  (`FLASH_WRITE_MIN_V`) and queued; the modem stays off below 3.4 V as before. The "every
  4th slot" modem window could never run and its record was never queued: removed.
- **H-7 Wrong record queued after a failed send**: the failed record is rebuilt from the
  send's own snapshot (it used the scheduler's `append_text`/`inst_rf`, which the next slot
  or a gap-fill may already have changed). The duplicate check uses the record's own slot.
- **TWS gap-fill trimmed the wrong file**: `/unsent.txt` is now trimmed on that path too.

## Medium
- **M-1 Self-healing never fired**: the modem power-cycle (every 4th failure in a row) no
  longer resets the count. A new per-slot counter restarts the unit once after 24 h with
  nothing delivered (no registration included). The hung-mutex restart (12 min held without
  a break) moved out of the sleep gate, where it could not be reached.
- **M-2 Late sleep abort left the hardware off**: past the point of no return the unit
  sleeps 1 s and starts clean instead of returning without modem, I2C and LCD power.
- **M-3 HTTPINIT fallback could not work**: one helper (`http_stack_prepare`) does
  HTTPTERM, HTTPINIT, URL, CONTENT; the retry paths sent the bare URL text as a command.
- **M-4 Wind pulses could be counted twice**: read, difference and update in one critical
  section in both the wind task and the scheduler.
- **M-5 Wind debounce**: left at 1 cycle. Analysis: a level must hold ~(N+1) ms, so 2
  cycles would cap wind at ~37 m/s. Raise only after scoping the anemometer's bounce.
- **M-6 GPS time set the clock 5 h 30 min behind**: +CLBS is UTC; all paths now convert to
  IST (`set_rtc_from_utc`).
- **M-7 Any phone could install firmware by SMS**: `SET_FW_FNAME` only from the control
  number or `SMS_ADMIN_NUMBERS` (user_config.h). GET_STATUS/GET_GPS still answer anyone.
- **M-8 Unlocked shared access**: remote DELETE_DATA holds `fsMutex`; the RTC clock-halt
  write holds `i2cMutex`.
- **M-9 Brownout detector**: unchanged by default; new switch `ENABLE_BROWNOUT_RESET`. Turn
  on only after checking the 3.3 V rail stays above ~2.6 V while the modem transmits.
- **LAST LOGGED cut the last digit**: buffer was 16 bytes for 16 characters; month and day
  are now zero-padded (`2026-01-05,18:15`).

## FTP removed
- `gprs_ftp.ino` removed; the old TCP firmware download and its helpers are kept in
  `ota_legacy.ino` until the chunked OTA client replaces it.
- Every system queues to `/unsent.txt` and sends by HTTP. Files left by older FTP builds
  (`ftpunsent.txt`, `ftpremain.txt`, chunk `TWS*.kwd/.swd`) are moved into the HTTP backlog
  once per power-on; `dailyftp_*.txt` staging files are deleted and no longer written.
- Server commands keep their names: FTP_BACKLOG answers that the backlog goes by HTTP each
  slot; FTP_DAILY and the SEND_FTP_DAILY_DATA SMS answer that FTP is removed.
- `FTP_CHUNK_SIZE` and `ENABLE_HTTP_BACKLOG_FALLBACK` removed from `user_config.h`.

## New settings (user_config.h / globals.h)
`SMS_ADMIN_NUMBERS`, `ENABLE_BROWNOUT_RESET`, `BACKLOG_MAX_RECORDS`, `FLASH_WRITE_MIN_V`,
`HEAL_REBOOT_SLOTS`, `LOCKUP_REBOOT_MS`.

## Not in this release
- OTA: on hold. Airtel M2M SIMs (both tested) cannot reach 187.127.175.183 on port 80 or
  5002, nor 75.119.148.192 or 144.91.104.105; KSNDMC works. Needs Airtel whitelisting.

## Backups
`.backup_v6.55/` (sources and the v6.51 board's 16 MB flash dump), `.backup_v6.56_step1/`,
`.backup_v6.56_step2/` (before FTP removal, includes `gprs_ftp.ino.removed`).
