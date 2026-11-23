AIO9 v6.64 SD-card update packages (8 MB flash, DEBUG 0)

Copy the TWO files from ONE folder to the ROOT of the SD card (not inside a folder):
    firmware.bin
    fw_version.txt
Insert the card and power-cycle the unit. The LCD shows "SD FW UPDATING.. PROGRESS xx%", then
"SD FW SUCCESS! REBOOTING...". A unit already on that version skips the update, so the same card
can be used on the next unit of the SAME type.

Folder                       fw_version.txt     For units
SD_v6.64_BIHAR_TRG_NUV        TRG9-BIH-6.64-N    Bihar TRG, Nuvoton LCD/keypad
SD_v6.64_BIHAR_TRG_MAT        TRG9-BIH-6.64-M    Bihar TRG, matrix keypad / I2C LCD
SD_v6.64_KSNDMC_TRG_NUV       TRG9-DMC-6.64-N    KSNDMC TRG, Nuvoton
SD_v6.64_KSNDMC_TRG_MAT       TRG9-DMC-6.64-M    KSNDMC TRG, matrix
SD_v6.64_KSNDMC_TWS_NUV       TWS9-DMC-6.64-N    KSNDMC TWS, Nuvoton
SD_v6.64_KSNDMC_TWS_MAT       TWS9-DMC-6.64-M    KSNDMC TWS, matrix

The unit checks the display hardware before installing: Nuvoton (-N) firmware only goes on a
Nuvoton unit and Matrix (-M) only on a Matrix unit. Otherwise the LCD shows "SD FW REFUSED: /
OTHER HARDWARE" and the unit keeps its firmware. The station type CAN be changed by SD card
(e.g. KSNDMC TWS -N firmware on a KSNDMC TRG -N or BIHAR TRG -N unit): check you have the folder
for the type you want.

From 6.64 the unit says at power-on why it skipped a card: "SD FW SKIPPED:" with SAME VERSION,
ALREADY INSTALLD or TRIED TWICE. "SD FW FAILED! CANNOT START" means the file could not be written
(too big or not a firmware image); the unit keeps its firmware.
