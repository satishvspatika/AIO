AIO9 v6.61 SD-card update packages (8 MB flash, DEBUG 0)

Copy the TWO files from ONE folder to the ROOT of the SD card (not inside a folder):
    firmware.bin
    fw_version.txt
Insert the card and power-cycle the unit. The LCD shows "SD FW UPDATING.. PROGRESS xx%", then
"SD FW SUCCESS! REBOOTING...". A unit already on that version skips the update, so the same card
can be used on the next unit of the SAME type.

Folder                       fw_version.txt     For units
SD_v6.61_BIHAR_TRG_NUV        TRG9-BIH-6.61-N    Bihar TRG, Nuvoton LCD/keypad
SD_v6.61_BIHAR_TRG_MAT        TRG9-BIH-6.61-M    Bihar TRG, matrix keypad / I2C LCD
SD_v6.61_KSNDMC_TRG_NUV       TRG9-DMC-6.61-N    KSNDMC TRG, Nuvoton
SD_v6.61_KSNDMC_TRG_MAT       TRG9-DMC-6.61-M    KSNDMC TRG, matrix
SD_v6.61_KSNDMC_TWS_NUV       TWS9-DMC-6.61-N    KSNDMC TWS, Nuvoton
SD_v6.61_KSNDMC_TWS_MAT       TWS9-DMC-6.61-M    KSNDMC TWS, matrix

WARNING: an SD card update does NOT check that the firmware is for the unit's own type (it is the
way to change a unit's type). The wrong folder (TWS firmware in a TRG, or -M in a -N unit) WILL be
installed. Check the unit's LCD version (e.g. TRG9-DMC-6.59-N) before inserting the card.
Over the air (office page) the type is always checked.
