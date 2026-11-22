# AIO9 v6.60 (22 Nov 2026)

v6.59 plus one change:

- **The unit tells the office why a download is waiting.** Every OTA check now carries `w=`:
  `0` ready to download, `b<volts>` battery below 3.7 V, `q<n>` records still to send. The same value decides
  whether the unit downloads an offered file, so the office page and the unit agree. The office page shows
  "Waiting: battery low" / "Waiting: records to send" with the next check time instead of "Starting download".
  Needs the office server change of 22 Nov 2026 (older servers ignore the field).

Build: KSNDMC TRG, Nuvoton, DEBUG 0, 8 MB (same as 6.57-6.59).
`KSNDMC_TRG_NUV_8mb/AIO9_v6.60_KSNDMC_TRG_NUV.bin` — 1,300,816 bytes. Not yet run on a unit.
Backup of the 6.59 sources: `.backup_v6.59_otawait/`.

**Superseded by 6.61 before release.**
