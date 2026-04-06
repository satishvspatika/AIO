ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.18V | Solar: 0.00V



[RTC] Time: 21:00
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 53/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 49
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :48,2026-04-04,20:45,000.00,000.0,000.0,00.0,000,-069,04.16

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 20:45

NO GAPS FOUND ...


Current data inserted is 49,2026-04-04,21:00,000.00,000.0,000.0,00.0,000,-111,04.16


append_text->store_text : Used for internal status: 49,2026-04-04,21:00,000.00,000.0,000.0,00.0,000,-111,04.16


append_text written to lastrecorded_<stationname>.txt is : 49,2026-04-04,21:00,000.00,000.0,000.0,00.0,000,-111,04.16



[FILE] SPIFFS: /001994_20260405.txt | Size: 2880
   ... [Tail Content] ...
00,-111,04.16
45,2026-04-04,20:00,000.00,000.0,000.0,00.0,000,-111,04.16
46,2026-04-04,20:15,000.00,000.0,000.0,00.0,000,-111,04.16
47,2026-04-04,20:30,000.00,000.0,000.0,00.0,000,-111,04.16
48,2026-04-04,20:45,000.00,000.0,000.0,00.0,000,-069,04.16
49,2026-04-04,21:00,000.00,000.0,000.0,00.0,000,-111,04.16
-----------------------

[FILE] SD: /001994_20260405.txt


[SCHED] Stack HWM: 11720 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
[STORE] Monthly cum fail counter reset (New Month detected).
   ... [Tail FTP Unsent] ...
001994;2026-04-04,21:00;00.00;000.0;000.0;00.0;000;-111;04.16
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=21:0:43
 Sleep=14:30 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 21:15
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 54/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 50
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :49,2026-04-04,21:00,000.00,000.0,000.0,00.0,000,-111,04.16

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 21:0

NO GAPS FOUND ...


Current data inserted is 50,2026-04-04,21:15,000.00,000.0,000.0,00.0,000,-111,04.16


append_text->store_text : Used for internal status: 50,2026-04-04,21:15,000.00,000.0,000.0,00.0,000,-111,04.16


append_text written to lastrecorded_<stationname>.txt is : 50,2026-04-04,21:15,000.00,000.0,000.0,00.0,000,-111,04.16



[FILE] SPIFFS: /001994_20260405.txt | Size: 2940
   ... [Tail Content] ...
00,-111,04.16
46,2026-04-04,20:15,000.00,000.0,000.0,00.0,000,-111,04.16
47,2026-04-04,20:30,000.00,000.0,000.0,00.0,000,-111,04.16
48,2026-04-04,20:45,000.00,000.0,000.0,00.0,000,-069,04.16
49,2026-04-04,21:00,000.00,000.0,000.0,00.0,000,-111,04.16
50,2026-04-04,21:15,000.00,000.0,000.0,00.0,000,-111,04.16
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
001994;2026-04-04,21:00;00.00;000.0;000.0;00.0;000;-111;04.16
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
001994;2026-04-04,21:00;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,21:15;00.00;000.0;000.0;00.0;000;-111;04.16
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=21:15:42
 Sleep=14:32 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 21:30
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 54/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 51
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :50,2026-04-04,21:15,000.00,000.0,000.0,00.0,000,-111,04.16

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 21:15

NO GAPS FOUND ...


Current data inserted is 51,2026-04-04,21:30,000.00,000.0,000.0,00.0,000,-111,04.16


append_text->store_text : Used for internal status: 51,2026-04-04,21:30,000.00,000.0,000.0,00.0,000,-111,04.16


append_text written to lastrecorded_<stationname>.txt is : 51,2026-04-04,21:30,000.00,000.0,000.0,00.0,000,-111,04.16



[FILE] SPIFFS: /001994_20260405.txt | Size: 3000
   ... [Tail Content] ...
00,-111,04.16
47,2026-04-04,20:30,000.00,000.0,000.0,00.0,000,-111,04.16
48,2026-04-04,20:45,000.00,000.0,000.0,00.0,000,-069,04.16
49,2026-04-04,21:00,000.00,000.0,000.0,00.0,000,-111,04.16
50,2026-04-04,21:15,000.00,000.0,000.0,00.0,000,-111,04.16
51,2026-04-04,21:30,000.00,000.0,000.0,00.0,000,-111,04.16
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
001994;2026-04-04,21:00;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,21:15;00.00;000.0;000.0;00.0;000;-111;04.16
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
001994;2026-04-04,21:00;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,21:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,21:30;00.00;000.0;000.0;00.0;000;-111;04.16
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=21:30:43
 Sleep=14:31 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.18V | Solar: 0.00V



[RTC] Time: 21:45
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM ready in 1000 ms!
************************
GETTING SIGNAL STRENGTH 
************************

Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -71

************************
GETTING NETWORK 
************************
Raw CICCID Resp: 
+ICCID: 89914509006871527866

OK

Final CCID parsed: 89914509006871527866
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] Fast-Track: Modem already registered! Bypassing setup block.
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Memory Check - Stored CCID: 89914509006871527866 | Stored APN: airteliot.com
Smart APN: Match Found!
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 54/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 52
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :51,2026-04-04,21:30,000.00,000.0,000.0,00.0,000,-111,04.16

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 21:30

NO GAPS FOUND ...


Current data inserted is 52,2026-04-04,21:45,000.00,000.0,000.0,00.0,000,-071,04.16


append_text->store_text : Used for internal status: 52,2026-04-04,21:45,000.00,000.0,000.0,00.0,000,-071,04.16


append_text written to lastrecorded_<stationname>.txt is : 52,2026-04-04,21:45,000.00,000.0,000.0,00.0,000,-071,04.16



[FILE] SPIFFS: /001994_20260405.txt | Size: 3060
   ... [Tail Content] ...
00,-111,04.16
48,2026-04-04,20:45,000.00,000.0,000.0,00.0,000,-069,04.16
49,2026-04-04,21:00,000.00,000.0,000.0,00.0,000,-111,04.16
50,2026-04-04,21:15,000.00,000.0,000.0,00.0,000,-111,04.16
51,2026-04-04,21:30,000.00,000.0,000.0,00.0,000,-111,04.16
52,2026-04-04,21:45,000.00,000.0,000.0,00.0,000,-071,04.16
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
001994;2026-04-04,21:00;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,21:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,21:30;00.00;000.0;000.0;00.0;000;-111;04.16
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] Ghost PDP (0.0.0.0). Triggering recovery...
[GPRS] Bearer status check: Re-initializing IP stack...
[GPRS] Starting HTTP...
HTTP POST REQUEST IS AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001994_20260405.txt

Current Data to be sent is : 51,2026-04-04,21:30,000.00,000.0,000.0,00.0,000,-111,04.16
http_data format is stn_no=001994&rec_time=2026-04-04,21:45&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.16&bat_volt2=04.16

Payload is stn_no=001994&rec_time=2026-04-04,21:45&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.16&bat_volt2=04.16
[HTTP] Using Fast v3.0 Handshake...
[RTC] Time: 21:46
Response of AT+HTTPACTION=1 is 
OK

[HTTP] HTTPACTION timed out — no URC received from modem.
[HTTP] 1st Attempt (Fast) failed. Retrying in 2s (Fast Attempt 2)...
Payload is stn_no=001994&rec_time=2026-04-04,21:45&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.16&bat_volt2=04.16
[HTTP] Using Fast v3.0 Handshake...
Response of AT+HTTPACTION=1 is 
OK

+HTTPACTION: 1,200,9

Response (P) of AT+HTTPREAD=0,512 is 
OK

+HTTPREAD: 9
Success


+HTTPREAD: 0

GPRS SEND : It is a Success
[RTC-Sync] No 'tm' field in server response. Skipping.
[DNS] Proactive cache updated (Raw IP Fallback DISABLED): rtdas.ksndmc.net -> 117.216.42.181
[PWR] Signature persisted successfully.

**** Storing Last Logged Data as 2026-04-04,21:45
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=3 cur_time=21:46 sched=NO cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[FTP] Backlog Push: 3 records found. Mode: THRESHOLD_MET

FTP file name is /TWSRF_001994_260405_214500.kwd

SampleNo  is 52

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Backlog fully chunked into Active Payload.
Retrieved file is /TWSRF_001994_260405_214500.kwd
Initializing A7672S for FTP...
[FTP] Using SAVED Smart Mode: Passive

ERROR


OK

+CFTPSSTART: 0

[FTP] Configuring mode (1)...

OK

[FTP] Warming up DNS...
[FTP] DNS Resolved: 27.34.245.70
[RTC] Time: 21:47

OK

+CFTPSLOGIN: 0

Result is 0
FTP Login success
File size: 189
[FTP] FSDEL path: TWSRF_001994_260405_214500.kwd
[FTP] FSOPEN cmd: AT+FSOPEN="C:/TWSRF_001994_260405_214500.kwd",0
[FTP] Found FH: 1
[FTP] Sending FSWRITE command...
[FTP] Data stream finished.
[FTP] Dispatching PUTFILE command...
[FTP] Dispatching PUTFILE command...
Response of AT+CFTPSPUTFILE: 
OK

+CFTPSPUTFILE: 0

[GoldenData] Marked records from /TWSRF_001994_260405_214500.kwd as DELIVERED.
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=21:47:24
 Sleep=12:48 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.18V | Solar: 0.00V



[RTC] Time: 22:00
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM ready in 1000 ms!
************************
GETTING SIGNAL STRENGTH 
************************

Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -71

************************
GETTING NETWORK 
************************
Raw CICCID Resp: 
+ICCID: 89914509006871527866

OK

Final CCID parsed: 89914509006871527866
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] Fast-Track: Modem already registered! Bypassing setup block.
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Memory Check - Stored CCID: 89914509006871527866 | Stored APN: airteliot.com
Smart APN: Match Found!
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 54/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 53
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :52,2026-04-04,21:45,000.00,000.0,000.0,00.0,000,-071,04.16

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 21:45

NO GAPS FOUND ...


Current data inserted is 53,2026-04-04,22:00,000.00,000.0,000.0,00.0,000,-071,04.16


append_text->store_text : Used for internal status: 53,2026-04-04,22:00,000.00,000.0,000.0,00.0,000,-071,04.16


append_text written to lastrecorded_<stationname>.txt is : 53,2026-04-04,22:00,000.00,000.0,000.0,00.0,000,-071,04.16



[FILE] SPIFFS: /001994_20260405.txt | Size: 3120
   ... [Tail Content] ...
00,-069,04.16
49,2026-04-04,21:00,000.00,000.0,000.0,00.0,000,-111,04.16
50,2026-04-04,21:15,000.00,000.0,000.0,00.0,000,-111,04.16
51,2026-04-04,21:30,000.00,000.0,000.0,00.0,000,-111,04.16
52,2026-04-04,21:45,000.00,000.0,000.0,00.0,000,-071,04.16
53,2026-04-04,22:00,000.00,000.0,000.0,00.0,000,-071,04.16
-----------------------

[FILE] SD: /001994_20260405.txt


[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).
[GPRS] Bearer status check: Re-initializing IP stack...
[GPRS] Starting HTTP...
HTTP POST REQUEST IS AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001994_20260405.txt

Current Data to be sent is : 52,2026-04-04,21:45,000.00,000.0,000.0,00.0,000,-071,04.16
http_data format is stn_no=001994&rec_time=2026-04-04,22:00&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.16&bat_volt2=04.16

Payload is stn_no=001994&rec_time=2026-04-04,22:00&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.16&bat_volt2=04.16
[HTTP] Using Fast v3.0 Handshake...
Response of AT+HTTPACTION=1 is 
OK

+HTTPACTION: 1,200,9

Response (P) of AT+HTTPREAD=0,512 is PHP/5.4.16
X-Powered-By: PHP/5.4.16
Content-Length: 9
Content-Type: text/html; charset=UTF-8
OK

OK

+HTTPREAD: 9
Success


+HTTPREAD: 0

GPRS SEND : It is a Success
[RTC-Sync] No 'tm' field in server response. Skipping.
[DNS] Proactive cache updated (Raw IP Fallback DISABLED): rtdas.ksndmc.net -> 117.216.42.181
[PWR] Signature persisted successfully.

**** Storing Last Logged Data as 2026-04-04,22:00
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=22:00 sched=NO cleanup=NO
[RTC] Time: 22:01

[GPRS] Checking SMS...

Removed READ/SENT messages (UNREAD preserved)
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=22:1:12
 Sleep=14:2 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 22:15
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 54/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 54
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :53,2026-04-04,22:00,000.00,000.0,000.0,00.0,000,-071,04.16

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 22:0

NO GAPS FOUND ...


Current data inserted is 54,2026-04-04,22:15,000.00,000.0,000.0,00.0,000,-111,04.16


append_text->store_text : Used for internal status: 54,2026-04-04,22:15,000.00,000.0,000.0,00.0,000,-111,04.16


append_text written to lastrecorded_<stationname>.txt is : 54,2026-04-04,22:15,000.00,000.0,000.0,00.0,000,-111,04.16



[FILE] SPIFFS: /001994_20260405.txt | Size: 3180
   ... [Tail Content] ...
00,-111,04.16
50,2026-04-04,21:15,000.00,000.0,000.0,00.0,000,-111,04.16
51,2026-04-04,21:30,000.00,000.0,000.0,00.0,000,-111,04.16
52,2026-04-04,21:45,000.00,000.0,000.0,00.0,000,-071,04.16
53,2026-04-04,22:00,000.00,000.0,000.0,00.0,000,-071,04.16
54,2026-04-04,22:15,000.00,000.0,000.0,00.0,000,-111,04.16
-----------------------

[FILE] SD: /001994_20260405.txt


[SCHED] Stack HWM: 11720 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
001994;2026-04-04,22:15;00.00;000.0;000.0;00.0;000;-111;04.16
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=22:15:44
 Sleep=14:29 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 22:30
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 54/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 55
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :54,2026-04-04,22:15,000.00,000.0,000.0,00.0,000,-111,04.16

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 22:15

NO GAPS FOUND ...


Current data inserted is 55,2026-04-04,22:30,000.00,000.0,000.0,00.0,000,-111,04.16


append_text->store_text : Used for internal status: 55,2026-04-04,22:30,000.00,000.0,000.0,00.0,000,-111,04.16


append_text written to lastrecorded_<stationname>.txt is : 55,2026-04-04,22:30,000.00,000.0,000.0,00.0,000,-111,04.16



[FILE] SPIFFS: /001994_20260405.txt | Size: 3240
   ... [Tail Content] ...
00,-111,04.16
51,2026-04-04,21:30,000.00,000.0,000.0,00.0,000,-111,04.16
52,2026-04-04,21:45,000.00,000.0,000.0,00.0,000,-071,04.16
53,2026-04-04,22:00,000.00,000.0,000.0,00.0,000,-071,04.16
54,2026-04-04,22:15,000.00,000.0,000.0,00.0,000,-111,04.16
55,2026-04-04,22:30,000.00,000.0,000.0,00.0,000,-111,04.16
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
001994;2026-04-04,22:15;00.00;000.0;000.0;00.0;000;-111;04.16
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
001994;2026-04-04,22:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:30;00.00;000.0;000.0;00.0;000;-111;04.16
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=22:30:42
 Sleep=14:32 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 22:45
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 54/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 56
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :55,2026-04-04,22:30,000.00,000.0,000.0,00.0,000,-111,04.16

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 22:30

NO GAPS FOUND ...


Current data inserted is 56,2026-04-04,22:45,000.00,000.0,000.0,00.0,000,-111,04.16


append_text->store_text : Used for internal status: 56,2026-04-04,22:45,000.00,000.0,000.0,00.0,000,-111,04.16


append_text written to lastrecorded_<stationname>.txt is : 56,2026-04-04,22:45,000.00,000.0,000.0,00.0,000,-111,04.16



[FILE] SPIFFS: /001994_20260405.txt | Size: 3300
   ... [Tail Content] ...
00,-111,04.16
52,2026-04-04,21:45,000.00,000.0,000.0,00.0,000,-071,04.16
53,2026-04-04,22:00,000.00,000.0,000.0,00.0,000,-071,04.16
54,2026-04-04,22:15,000.00,000.0,000.0,00.0,000,-111,04.16
55,2026-04-04,22:30,000.00,000.0,000.0,00.0,000,-111,04.16
56,2026-04-04,22:45,000.00,000.0,000.0,00.0,000,-111,04.16
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
001994;2026-04-04,22:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:30;00.00;000.0;000.0;00.0;000;-111;04.16
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
001994;2026-04-04,22:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:45;00.00;000.0;000.0;00.0;000;-111;04.16
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=22:45:41
 Sleep=14:33 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.18V | Solar: 0.00V



[RTC] Time: 23:00
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 55/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 57
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :56,2026-04-04,22:45,000.00,000.0,000.0,00.0,000,-111,04.16

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 22:45

NO GAPS FOUND ...


Current data inserted is 57,2026-04-04,23:00,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 57,2026-04-04,23:00,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 57,2026-04-04,23:00,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 3360
   ... [Tail Content] ...
00,-071,04.16
53,2026-04-04,22:00,000.00,000.0,000.0,00.0,000,-071,04.16
54,2026-04-04,22:15,000.00,000.0,000.0,00.0,000,-111,04.16
55,2026-04-04,22:30,000.00,000.0,000.0,00.0,000,-111,04.16
56,2026-04-04,22:45,000.00,000.0,000.0,00.0,000,-111,04.16
57,2026-04-04,23:00,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
001994;2026-04-04,22:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:45;00.00;000.0;000.0;00.0;000;-111;04.16
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
001994;2026-04-04,22:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:00;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=23:0:42
 Sleep=14:32 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 23:15
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 55/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 58
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :57,2026-04-04,23:00,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 23:0

NO GAPS FOUND ...


Current data inserted is 58,2026-04-04,23:15,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 58,2026-04-04,23:15,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 58,2026-04-04,23:15,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 3420
   ... [Tail Content] ...
00,-071,04.16
54,2026-04-04,22:15,000.00,000.0,000.0,00.0,000,-111,04.16
55,2026-04-04,22:30,000.00,000.0,000.0,00.0,000,-111,04.16
56,2026-04-04,22:45,000.00,000.0,000.0,00.0,000,-111,04.16
57,2026-04-04,23:00,000.00,000.0,000.0,00.0,000,-111,04.15
58,2026-04-04,23:15,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
001994;2026-04-04,22:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:00;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
001994;2026-04-04,22:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:15;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=23:15:42
 Sleep=14:33 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.18V | Solar: 0.00V



[RTC] Time: 23:30
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 55/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 59
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :58,2026-04-04,23:15,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 23:15

NO GAPS FOUND ...


Current data inserted is 59,2026-04-04,23:30,000.00,000.0,000.0,00.0,000,-111,04.16


append_text->store_text : Used for internal status: 59,2026-04-04,23:30,000.00,000.0,000.0,00.0,000,-111,04.16


append_text written to lastrecorded_<stationname>.txt is : 59,2026-04-04,23:30,000.00,000.0,000.0,00.0,000,-111,04.16



[FILE] SPIFFS: /001994_20260405.txt | Size: 3480
   ... [Tail Content] ...
00,-111,04.16
55,2026-04-04,22:30,000.00,000.0,000.0,00.0,000,-111,04.16
56,2026-04-04,22:45,000.00,000.0,000.0,00.0,000,-111,04.16
57,2026-04-04,23:00,000.00,000.0,000.0,00.0,000,-111,04.15
58,2026-04-04,23:15,000.00,000.0,000.0,00.0,000,-111,04.15
59,2026-04-04,23:30,000.00,000.0,000.0,00.0,000,-111,04.16
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
001994;2026-04-04,22:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:15;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
001994;2026-04-04,22:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:30;00.00;000.0;000.0;00.0;000;-111;04.16
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=23:30:43
 Sleep=14:31 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 23:45
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 55/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 60
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :59,2026-04-04,23:30,000.00,000.0,000.0,00.0,000,-111,04.16

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 23:30

NO GAPS FOUND ...


Current data inserted is 60,2026-04-04,23:45,000.00,000.0,000.0,00.0,000,-111,04.16


append_text->store_text : Used for internal status: 60,2026-04-04,23:45,000.00,000.0,000.0,00.0,000,-111,04.16


append_text written to lastrecorded_<stationname>.txt is : 60,2026-04-04,23:45,000.00,000.0,000.0,00.0,000,-111,04.16



[FILE] SPIFFS: /001994_20260405.txt | Size: 3540
   ... [Tail Content] ...
00,-111,04.16
56,2026-04-04,22:45,000.00,000.0,000.0,00.0,000,-111,04.16
57,2026-04-04,23:00,000.00,000.0,000.0,00.0,000,-111,04.15
58,2026-04-04,23:15,000.00,000.0,000.0,00.0,000,-111,04.15
59,2026-04-04,23:30,000.00,000.0,000.0,00.0,000,-111,04.16
60,2026-04-04,23:45,000.00,000.0,000.0,00.0,000,-111,04.16
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
001994;2026-04-04,22:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:30;00.00;000.0;000.0;00.0;000;-111;04.16
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
001994;2026-04-04,22:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:45;00.00;000.0;000.0;00.0;000;-111;04.16
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=23:45:41
 Sleep=14:33 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 00:00
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 55/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 61
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :60,2026-04-04,23:45,000.00,000.0,000.0,00.0,000,-111,04.16

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 23:45

NO GAPS FOUND ...


Current data inserted is 61,2026-04-05,00:00,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 61,2026-04-05,00:00,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 61,2026-04-05,00:00,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 3600
   ... [Tail Content] ...
00,-111,04.16
57,2026-04-04,23:00,000.00,000.0,000.0,00.0,000,-111,04.15
58,2026-04-04,23:15,000.00,000.0,000.0,00.0,000,-111,04.15
59,2026-04-04,23:30,000.00,000.0,000.0,00.0,000,-111,04.16
60,2026-04-04,23:45,000.00,000.0,000.0,00.0,000,-111,04.16
61,2026-04-05,00:00,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
001994;2026-04-04,22:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:45;00.00;000.0;000.0;00.0;000;-111;04.16
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11720 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-04,22:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,00:00;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=0:0:42
 Sleep=14:32 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 00:15
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 56/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 62
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :61,2026-04-05,00:00,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 0:0

NO GAPS FOUND ...


Current data inserted is 62,2026-04-05,00:15,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 62,2026-04-05,00:15,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 62,2026-04-05,00:15,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 3660
   ... [Tail Content] ...
00,-111,04.15
58,2026-04-04,23:15,000.00,000.0,000.0,00.0,000,-111,04.15
59,2026-04-04,23:30,000.00,000.0,000.0,00.0,000,-111,04.16
60,2026-04-04,23:45,000.00,000.0,000.0,00.0,000,-111,04.16
61,2026-04-05,00:00,000.00,000.0,000.0,00.0,000,-111,04.15
62,2026-04-05,00:15,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-04,22:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,00:00;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-04,22:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,00:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:15;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=0:15:44
 Sleep=14:30 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 00:30
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 56/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 63
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :62,2026-04-05,00:15,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 0:15

NO GAPS FOUND ...


Current data inserted is 63,2026-04-05,00:30,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 63,2026-04-05,00:30,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 63,2026-04-05,00:30,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 3720
   ... [Tail Content] ...
00,-111,04.15
59,2026-04-04,23:30,000.00,000.0,000.0,00.0,000,-111,04.16
60,2026-04-04,23:45,000.00,000.0,000.0,00.0,000,-111,04.16
61,2026-04-05,00:00,000.00,000.0,000.0,00.0,000,-111,04.15
62,2026-04-05,00:15,000.00,000.0,000.0,00.0,000,-111,04.15
63,2026-04-05,00:30,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-04,22:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,22:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,00:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:15;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11720 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-04,22:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,00:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:30;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=0:30:41
 Sleep=14:33 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.18V | Solar: 0.00V



[RTC] Time: 00:45
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 56/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 64
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :63,2026-04-05,00:30,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 0:30

NO GAPS FOUND ...


Current data inserted is 64,2026-04-05,00:45,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 64,2026-04-05,00:45,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 64,2026-04-05,00:45,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 3780
   ... [Tail Content] ...
00,-111,04.16
60,2026-04-04,23:45,000.00,000.0,000.0,00.0,000,-111,04.16
61,2026-04-05,00:00,000.00,000.0,000.0,00.0,000,-111,04.15
62,2026-04-05,00:15,000.00,000.0,000.0,00.0,000,-111,04.15
63,2026-04-05,00:30,000.00,000.0,000.0,00.0,000,-111,04.15
64,2026-04-05,00:45,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-04,22:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,00:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:30;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11464 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-04,23:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,00:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:45;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=0:45:42
 Sleep=14:32 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 01:00
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 56/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 65
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :64,2026-04-05,00:45,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 0:45

NO GAPS FOUND ...


Current data inserted is 65,2026-04-05,01:00,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 65,2026-04-05,01:00,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 65,2026-04-05,01:00,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 3840
   ... [Tail Content] ...
00,-111,04.16
61,2026-04-05,00:00,000.00,000.0,000.0,00.0,000,-111,04.15
62,2026-04-05,00:15,000.00,000.0,000.0,00.0,000,-111,04.15
63,2026-04-05,00:30,000.00,000.0,000.0,00.0,000,-111,04.15
64,2026-04-05,00:45,000.00,000.0,000.0,00.0,000,-111,04.15
65,2026-04-05,01:00,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-04,23:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,00:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:45;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-04,23:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-04,23:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,00:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:00;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=1:0:41
 Sleep=14:33 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 01:15
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
[SIM] PERSISTENT ERROR for 13 consecutive slots (~3h15m). Final resort: ESP32 SOFTWARE RESTART...
ets Jul 29 2019 12:21:46

rst:0xc (SW_CPU_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 12
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup was not caused by deep sleep: 0
[BOOT] Reset Reason: SW_CPU_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 12
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] ULP Anchors Synced: Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.14V | Solar: 0.00V



[RTC] Time: 01:15
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 1 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 56/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
Fresh boot detected. Skipping primary upload for sensor stabilization (queueing to Backlog).
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 66
RF Close date from RTC = 2026-04-05  
[SCHED] 🗓 Day Change Detected. Performing Rollover...
[SCHED] First rollover after boot/flash. Skipping destructive reset to preserve SPIFFS recovery.
[GoldenData] Starting Sent Mask Reconstruction from SPIFFS...
[GoldenData] Reconstruction Complete (Sent-Accurate).

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :65,2026-04-05,01:00,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 1:0
Gap filling for loop started... 
Appending record 01:15 (05-04-2026)
66
Primary skipped mid-day. Queuing CURRENT record to ftpunsent.txt...

append_text->store_text : Used for internal status: 66,2026-04-05,01:15,000.00,000.0,000.0,00.0,000,-111,04.17


append_text written to lastrecorded_<stationname>.txt is : 66,2026-04-05,01:15,000.00,000.0,000.0,00.0,000,-111,04.17



[FILE] SPIFFS: /001994_20260405.txt | Size: 3900
   ... [Tail Content] ...
00,-111,04.15
62,2026-04-05,00:15,000.00,000.0,000.0,00.0,000,-111,04.15
63,2026-04-05,00:30,000.00,000.0,000.0,00.0,000,-111,04.15
64,2026-04-05,00:45,000.00,000.0,000.0,00.0,000,-111,04.15
65,2026-04-05,01:00,000.00,000.0,000.0,00.0,000,-111,04.15
66,2026-04-05,01:15,000.00,000.0,000.0,00.0,000,-111,04.17
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-04,23:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,00:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:15;00.00;000.0;000.0;00.0;000;-111;04.17
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11160 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak, but data already queued cleanly by scheduler skip.
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[GPRS] State Change: sync_mode=4 gprs_mode=2
[RTC] Time: 01:16
[RTC] Time: 01:17
[RTC] Time: 01:18
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=1:18:52
 Sleep=11:20 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 01:30
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 56/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 67
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :66,2026-04-05,01:15,000.00,000.0,000.0,00.0,000,-111,04.17

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 1:15

NO GAPS FOUND ...


Current data inserted is 67,2026-04-05,01:30,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 67,2026-04-05,01:30,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 67,2026-04-05,01:30,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 3960
   ... [Tail Content] ...
00,-111,04.15
63,2026-04-05,00:30,000.00,000.0,000.0,00.0,000,-111,04.15
64,2026-04-05,00:45,000.00,000.0,000.0,00.0,000,-111,04.15
65,2026-04-05,01:00,000.00,000.0,000.0,00.0,000,-111,04.15
66,2026-04-05,01:15,000.00,000.0,000.0,00.0,000,-111,04.17
67,2026-04-05,01:30,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-04,23:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,23:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,00:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:15;00.00;000.0;000.0;00.0;000;-111;04.17
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
[STORE] Monthly cum fail counter reset (New Month detected).
   ... [Tail FTP Unsent] ...
94;2026-04-04,23:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,00:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:15;00.00;000.0;000.0;00.0;000;-111;04.17
001994;2026-04-05,01:30;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=1:30:43
 Sleep=14:31 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.16V | Solar: 0.00V



[RTC] Time: 01:45
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 57/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 68
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :67,2026-04-05,01:30,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 1:30

NO GAPS FOUND ...


Current data inserted is 68,2026-04-05,01:45,000.00,000.0,000.0,00.0,000,-111,04.16


append_text->store_text : Used for internal status: 68,2026-04-05,01:45,000.00,000.0,000.0,00.0,000,-111,04.16


append_text written to lastrecorded_<stationname>.txt is : 68,2026-04-05,01:45,000.00,000.0,000.0,00.0,000,-111,04.16



[FILE] SPIFFS: /001994_20260405.txt | Size: 4020
   ... [Tail Content] ...
00,-111,04.15
64,2026-04-05,00:45,000.00,000.0,000.0,00.0,000,-111,04.15
65,2026-04-05,01:00,000.00,000.0,000.0,00.0,000,-111,04.15
66,2026-04-05,01:15,000.00,000.0,000.0,00.0,000,-111,04.17
67,2026-04-05,01:30,000.00,000.0,000.0,00.0,000,-111,04.15
68,2026-04-05,01:45,000.00,000.0,000.0,00.0,000,-111,04.16
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-04,23:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,00:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:15;00.00;000.0;000.0;00.0;000;-111;04.17
001994;2026-04-05,01:30;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-05,00:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:15;00.00;000.0;000.0;00.0;000;-111;04.17
001994;2026-04-05,01:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:45;00.00;000.0;000.0;00.0;000;-111;04.16
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=1:45:42
 Sleep=14:32 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 02:00
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 57/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 69
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :68,2026-04-05,01:45,000.00,000.0,000.0,00.0,000,-111,04.16

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 1:45

NO GAPS FOUND ...


Current data inserted is 69,2026-04-05,02:00,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 69,2026-04-05,02:00,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 69,2026-04-05,02:00,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 4080
   ... [Tail Content] ...
00,-111,04.15
65,2026-04-05,01:00,000.00,000.0,000.0,00.0,000,-111,04.15
66,2026-04-05,01:15,000.00,000.0,000.0,00.0,000,-111,04.17
67,2026-04-05,01:30,000.00,000.0,000.0,00.0,000,-111,04.15
68,2026-04-05,01:45,000.00,000.0,000.0,00.0,000,-111,04.16
69,2026-04-05,02:00,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,00:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:15;00.00;000.0;000.0;00.0;000;-111;04.17
001994;2026-04-05,01:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:45;00.00;000.0;000.0;00.0;000;-111;04.16
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-05,00:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:15;00.00;000.0;000.0;00.0;000;-111;04.17
001994;2026-04-05,01:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:00;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=2:0:42
 Sleep=14:32 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 02:15
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 57/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 70
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :69,2026-04-05,02:00,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 2:0

NO GAPS FOUND ...


Current data inserted is 70,2026-04-05,02:15,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 70,2026-04-05,02:15,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 70,2026-04-05,02:15,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 4140
   ... [Tail Content] ...
00,-111,04.15
66,2026-04-05,01:15,000.00,000.0,000.0,00.0,000,-111,04.17
67,2026-04-05,01:30,000.00,000.0,000.0,00.0,000,-111,04.15
68,2026-04-05,01:45,000.00,000.0,000.0,00.0,000,-111,04.16
69,2026-04-05,02:00,000.00,000.0,000.0,00.0,000,-111,04.15
70,2026-04-05,02:15,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,00:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:15;00.00;000.0;000.0;00.0;000;-111;04.17
001994;2026-04-05,01:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:00;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-05,00:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:15;00.00;000.0;000.0;00.0;000;-111;04.17
001994;2026-04-05,01:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:15;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=2:15:43
 Sleep=14:31 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 02:30
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 57/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 71
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :70,2026-04-05,02:15,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 2:15

NO GAPS FOUND ...


Current data inserted is 71,2026-04-05,02:30,000.00,000.0,000.0,00.0,000,-111,04.16


append_text->store_text : Used for internal status: 71,2026-04-05,02:30,000.00,000.0,000.0,00.0,000,-111,04.16


append_text written to lastrecorded_<stationname>.txt is : 71,2026-04-05,02:30,000.00,000.0,000.0,00.0,000,-111,04.16



[FILE] SPIFFS: /001994_20260405.txt | Size: 4200
   ... [Tail Content] ...
00,-111,04.17
67,2026-04-05,01:30,000.00,000.0,000.0,00.0,000,-111,04.15
68,2026-04-05,01:45,000.00,000.0,000.0,00.0,000,-111,04.16
69,2026-04-05,02:00,000.00,000.0,000.0,00.0,000,-111,04.15
70,2026-04-05,02:15,000.00,000.0,000.0,00.0,000,-111,04.15
71,2026-04-05,02:30,000.00,000.0,000.0,00.0,000,-111,04.16
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,00:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,00:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:15;00.00;000.0;000.0;00.0;000;-111;04.17
001994;2026-04-05,01:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:15;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-05,00:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:15;00.00;000.0;000.0;00.0;000;-111;04.17
001994;2026-04-05,01:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:30;00.00;000.0;000.0;00.0;000;-111;04.16
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=2:30:43
 Sleep=14:31 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 02:45
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 57/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 72
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :71,2026-04-05,02:30,000.00,000.0,000.0,00.0,000,-111,04.16

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 2:30

NO GAPS FOUND ...


Current data inserted is 72,2026-04-05,02:45,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 72,2026-04-05,02:45,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 72,2026-04-05,02:45,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 4260
   ... [Tail Content] ...
00,-111,04.15
68,2026-04-05,01:45,000.00,000.0,000.0,00.0,000,-111,04.16
69,2026-04-05,02:00,000.00,000.0,000.0,00.0,000,-111,04.15
70,2026-04-05,02:15,000.00,000.0,000.0,00.0,000,-111,04.15
71,2026-04-05,02:30,000.00,000.0,000.0,00.0,000,-111,04.16
72,2026-04-05,02:45,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,00:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:15;00.00;000.0;000.0;00.0;000;-111;04.17
001994;2026-04-05,01:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:30;00.00;000.0;000.0;00.0;000;-111;04.16
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11656 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-05,01:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:15;00.00;000.0;000.0;00.0;000;-111;04.17
001994;2026-04-05,01:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:45;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=2:45:42
 Sleep=14:32 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 03:00
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 57/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 73
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :72,2026-04-05,02:45,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 2:45

NO GAPS FOUND ...


Current data inserted is 73,2026-04-05,03:00,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 73,2026-04-05,03:00,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 73,2026-04-05,03:00,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 4320
   ... [Tail Content] ...
00,-111,04.16
69,2026-04-05,02:00,000.00,000.0,000.0,00.0,000,-111,04.15
70,2026-04-05,02:15,000.00,000.0,000.0,00.0,000,-111,04.15
71,2026-04-05,02:30,000.00,000.0,000.0,00.0,000,-111,04.16
72,2026-04-05,02:45,000.00,000.0,000.0,00.0,000,-111,04.15
73,2026-04-05,03:00,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,01:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:15;00.00;000.0;000.0;00.0;000;-111;04.17
001994;2026-04-05,01:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:45;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11720 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-05,01:15;00.00;000.0;000.0;00.0;000;-111;04.17
001994;2026-04-05,01:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:00;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=3:0:43
 Sleep=14:31 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 03:15
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 58/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 74
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :73,2026-04-05,03:00,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 3:0

NO GAPS FOUND ...


Current data inserted is 74,2026-04-05,03:15,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 74,2026-04-05,03:15,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 74,2026-04-05,03:15,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 4380
   ... [Tail Content] ...
00,-111,04.15
70,2026-04-05,02:15,000.00,000.0,000.0,00.0,000,-111,04.15
71,2026-04-05,02:30,000.00,000.0,000.0,00.0,000,-111,04.16
72,2026-04-05,02:45,000.00,000.0,000.0,00.0,000,-111,04.15
73,2026-04-05,03:00,000.00,000.0,000.0,00.0,000,-111,04.15
74,2026-04-05,03:15,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,01:15;00.00;000.0;000.0;00.0;000;-111;04.17
001994;2026-04-05,01:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:00;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11656 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-05,01:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:15;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=3:15:41
 Sleep=14:33 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.16V | Solar: 0.00V



[RTC] Time: 03:30
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 58/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 75
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :74,2026-04-05,03:15,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 3:15

NO GAPS FOUND ...


Current data inserted is 75,2026-04-05,03:30,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 75,2026-04-05,03:30,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 75,2026-04-05,03:30,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 4440
   ... [Tail Content] ...
00,-111,04.15
71,2026-04-05,02:30,000.00,000.0,000.0,00.0,000,-111,04.16
72,2026-04-05,02:45,000.00,000.0,000.0,00.0,000,-111,04.15
73,2026-04-05,03:00,000.00,000.0,000.0,00.0,000,-111,04.15
74,2026-04-05,03:15,000.00,000.0,000.0,00.0,000,-111,04.15
75,2026-04-05,03:30,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,01:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,01:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:15;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-05,01:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:30;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=3:30:42
 Sleep=14:32 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 03:45
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 58/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 76
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :75,2026-04-05,03:30,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 3:30

NO GAPS FOUND ...


Current data inserted is 76,2026-04-05,03:45,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 76,2026-04-05,03:45,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 76,2026-04-05,03:45,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 4500
   ... [Tail Content] ...
00,-111,04.16
72,2026-04-05,02:45,000.00,000.0,000.0,00.0,000,-111,04.15
73,2026-04-05,03:00,000.00,000.0,000.0,00.0,000,-111,04.15
74,2026-04-05,03:15,000.00,000.0,000.0,00.0,000,-111,04.15
75,2026-04-05,03:30,000.00,000.0,000.0,00.0,000,-111,04.15
76,2026-04-05,03:45,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,01:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:30;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-05,02:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:45;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=3:45:42
 Sleep=14:32 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 04:00
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 58/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 77
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :76,2026-04-05,03:45,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 3:45

NO GAPS FOUND ...


Current data inserted is 77,2026-04-05,04:00,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 77,2026-04-05,04:00,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 77,2026-04-05,04:00,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 4560
   ... [Tail Content] ...
00,-111,04.15
73,2026-04-05,03:00,000.00,000.0,000.0,00.0,000,-111,04.15
74,2026-04-05,03:15,000.00,000.0,000.0,00.0,000,-111,04.15
75,2026-04-05,03:30,000.00,000.0,000.0,00.0,000,-111,04.15
76,2026-04-05,03:45,000.00,000.0,000.0,00.0,000,-111,04.15
77,2026-04-05,04:00,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,02:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:45;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-05,02:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,02:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=4:0:43
 Sleep=14:31 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 04:15
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
[SIM] PERSISTENT ERROR for 13 consecutive slots (~3h15m). Final resort: ESP32 SOFTWARE RESTART...
ets Jul 29 2019 12:21:46

rst:0xc (SW_CPU_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 12
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup was not caused by deep sleep: 0
[BOOT] Reset Reason: SW_CPU_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 12
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] ULP Anchors Synced: Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.16V | Solar: 0.00V



[RTC] Time: 04:15
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 1 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 59/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
Fresh boot detected. Skipping primary upload for sensor stabilization (queueing to Backlog).
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 78
RF Close date from RTC = 2026-04-05  
[SCHED] 🗓 Day Change Detected. Performing Rollover...
[SCHED] First rollover after boot/flash. Skipping destructive reset to preserve SPIFFS recovery.
[GoldenData] Starting Sent Mask Reconstruction from SPIFFS...
[GoldenData] Reconstruction Complete (Sent-Accurate).

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :77,2026-04-05,04:00,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 4:0
Gap filling for loop started... 
Appending record 04:15 (05-04-2026)
78
Primary skipped mid-day. Queuing CURRENT record to ftpunsent.txt...

append_text->store_text : Used for internal status: 78,2026-04-05,04:15,000.00,000.0,000.0,00.0,000,-111,04.16


append_text written to lastrecorded_<stationname>.txt is : 78,2026-04-05,04:15,000.00,000.0,000.0,00.0,000,-111,04.16



[FILE] SPIFFS: /001994_20260405.txt | Size: 4620
   ... [Tail Content] ...
00,-111,04.15
74,2026-04-05,03:15,000.00,000.0,000.0,00.0,000,-111,04.15
75,2026-04-05,03:30,000.00,000.0,000.0,00.0,000,-111,04.15
76,2026-04-05,03:45,000.00,000.0,000.0,00.0,000,-111,04.15
77,2026-04-05,04:00,000.00,000.0,000.0,00.0,000,-111,04.15
78,2026-04-05,04:15,000.00,000.0,000.0,00.0,000,-111,04.16
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,02:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:15;00.00;000.0;000.0;00.0;000;-111;04.16
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11148 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak, but data already queued cleanly by scheduler skip.
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[GPRS] State Change: sync_mode=4 gprs_mode=2
[RTC] Time: 04:16
[RTC] Time: 04:17
[RTC] Time: 04:18
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=4:18:50
 Sleep=11:21 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 04:30
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 59/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 79
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :78,2026-04-05,04:15,000.00,000.0,000.0,00.0,000,-111,04.16

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 4:15

NO GAPS FOUND ...


Current data inserted is 79,2026-04-05,04:30,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 79,2026-04-05,04:30,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 79,2026-04-05,04:30,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 4680
   ... [Tail Content] ...
00,-111,04.15
75,2026-04-05,03:30,000.00,000.0,000.0,00.0,000,-111,04.15
76,2026-04-05,03:45,000.00,000.0,000.0,00.0,000,-111,04.15
77,2026-04-05,04:00,000.00,000.0,000.0,00.0,000,-111,04.15
78,2026-04-05,04:15,000.00,000.0,000.0,00.0,000,-111,04.16
79,2026-04-05,04:30,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,02:30;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,02:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:15;00.00;000.0;000.0;00.0;000;-111;04.16
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11720 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
[STORE] Monthly cum fail counter reset (New Month detected).
   ... [Tail FTP Unsent] ...
94;2026-04-05,02:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,04:30;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=4:30:42
 Sleep=14:32 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 04:45
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 59/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 80
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :79,2026-04-05,04:30,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 4:30

NO GAPS FOUND ...


Current data inserted is 80,2026-04-05,04:45,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 80,2026-04-05,04:45,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 80,2026-04-05,04:45,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 4740
   ... [Tail Content] ...
00,-111,04.15
76,2026-04-05,03:45,000.00,000.0,000.0,00.0,000,-111,04.15
77,2026-04-05,04:00,000.00,000.0,000.0,00.0,000,-111,04.15
78,2026-04-05,04:15,000.00,000.0,000.0,00.0,000,-111,04.16
79,2026-04-05,04:30,000.00,000.0,000.0,00.0,000,-111,04.15
80,2026-04-05,04:45,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,02:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,04:30;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-05,03:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,04:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:45;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=4:45:43
 Sleep=14:31 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 05:00
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 59/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 81
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :80,2026-04-05,04:45,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 4:45

NO GAPS FOUND ...


Current data inserted is 81,2026-04-05,05:00,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 81,2026-04-05,05:00,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 81,2026-04-05,05:00,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 4800
   ... [Tail Content] ...
00,-111,04.15
77,2026-04-05,04:00,000.00,000.0,000.0,00.0,000,-111,04.15
78,2026-04-05,04:15,000.00,000.0,000.0,00.0,000,-111,04.16
79,2026-04-05,04:30,000.00,000.0,000.0,00.0,000,-111,04.15
80,2026-04-05,04:45,000.00,000.0,000.0,00.0,000,-111,04.15
81,2026-04-05,05:00,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,03:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,04:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:45;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11720 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-05,03:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,04:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:00;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=5:0:42
 Sleep=14:32 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.16V | Solar: 0.00V



[RTC] Time: 05:15
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 59/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 82
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :81,2026-04-05,05:00,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 5:0

NO GAPS FOUND ...


Current data inserted is 82,2026-04-05,05:15,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 82,2026-04-05,05:15,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 82,2026-04-05,05:15,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 4860
   ... [Tail Content] ...
00,-111,04.15
78,2026-04-05,04:15,000.00,000.0,000.0,00.0,000,-111,04.16
79,2026-04-05,04:30,000.00,000.0,000.0,00.0,000,-111,04.15
80,2026-04-05,04:45,000.00,000.0,000.0,00.0,000,-111,04.15
81,2026-04-05,05:00,000.00,000.0,000.0,00.0,000,-111,04.15
82,2026-04-05,05:15,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,03:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,04:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:00;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-05,03:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,04:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:15;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=5:15:42
 Sleep=14:32 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 05:30
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 59/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 83
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :82,2026-04-05,05:15,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 5:15

NO GAPS FOUND ...


Current data inserted is 83,2026-04-05,05:30,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 83,2026-04-05,05:30,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 83,2026-04-05,05:30,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 4920
   ... [Tail Content] ...
00,-111,04.16
79,2026-04-05,04:30,000.00,000.0,000.0,00.0,000,-111,04.15
80,2026-04-05,04:45,000.00,000.0,000.0,00.0,000,-111,04.15
81,2026-04-05,05:00,000.00,000.0,000.0,00.0,000,-111,04.15
82,2026-04-05,05:15,000.00,000.0,000.0,00.0,000,-111,04.15
83,2026-04-05,05:30,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,03:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,03:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,04:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:15;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11464 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-05,03:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,04:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:30;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=5:30:42
 Sleep=14:32 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 05:45
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 60/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 84
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :83,2026-04-05,05:30,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 5:30

NO GAPS FOUND ...


Current data inserted is 84,2026-04-05,05:45,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 84,2026-04-05,05:45,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 84,2026-04-05,05:45,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 4980
   ... [Tail Content] ...
00,-111,04.15
80,2026-04-05,04:45,000.00,000.0,000.0,00.0,000,-111,04.15
81,2026-04-05,05:00,000.00,000.0,000.0,00.0,000,-111,04.15
82,2026-04-05,05:15,000.00,000.0,000.0,00.0,000,-111,04.15
83,2026-04-05,05:30,000.00,000.0,000.0,00.0,000,-111,04.15
84,2026-04-05,05:45,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,03:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,04:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:30;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-05,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,04:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:45;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=5:45:41
 Sleep=14:33 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 06:00
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 60/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 85
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :84,2026-04-05,05:45,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 5:45

NO GAPS FOUND ...


Current data inserted is 85,2026-04-05,06:00,000.00,000.0,000.0,00.0,000,-111,04.16


append_text->store_text : Used for internal status: 85,2026-04-05,06:00,000.00,000.0,000.0,00.0,000,-111,04.16


append_text written to lastrecorded_<stationname>.txt is : 85,2026-04-05,06:00,000.00,000.0,000.0,00.0,000,-111,04.16



[FILE] SPIFFS: /001994_20260405.txt | Size: 5040
   ... [Tail Content] ...
00,-111,04.15
81,2026-04-05,05:00,000.00,000.0,000.0,00.0,000,-111,04.15
82,2026-04-05,05:15,000.00,000.0,000.0,00.0,000,-111,04.15
83,2026-04-05,05:30,000.00,000.0,000.0,00.0,000,-111,04.15
84,2026-04-05,05:45,000.00,000.0,000.0,00.0,000,-111,04.15
85,2026-04-05,06:00,000.00,000.0,000.0,00.0,000,-111,04.16
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,04:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:45;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-05,04:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,04:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:00;00.00;000.0;000.0;00.0;000;-111;04.16
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=6:0:42
 Sleep=14:32 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 06:15
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 60/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 86
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :85,2026-04-05,06:00,000.00,000.0,000.0,00.0,000,-111,04.16

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 6:0

NO GAPS FOUND ...


Current data inserted is 86,2026-04-05,06:15,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 86,2026-04-05,06:15,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 86,2026-04-05,06:15,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 5100
   ... [Tail Content] ...
00,-111,04.15
82,2026-04-05,05:15,000.00,000.0,000.0,00.0,000,-111,04.15
83,2026-04-05,05:30,000.00,000.0,000.0,00.0,000,-111,04.15
84,2026-04-05,05:45,000.00,000.0,000.0,00.0,000,-111,04.15
85,2026-04-05,06:00,000.00,000.0,000.0,00.0,000,-111,04.16
86,2026-04-05,06:15,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,04:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,04:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:00;00.00;000.0;000.0;00.0;000;-111;04.16
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-05,04:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:00;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,06:15;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=6:15:41
 Sleep=14:33 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.16V | Solar: 0.00V



[RTC] Time: 06:30
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 60/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 87
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :86,2026-04-05,06:15,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 6:15

NO GAPS FOUND ...


Current data inserted is 87,2026-04-05,06:30,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 87,2026-04-05,06:30,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 87,2026-04-05,06:30,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 5160
   ... [Tail Content] ...
00,-111,04.15
83,2026-04-05,05:30,000.00,000.0,000.0,00.0,000,-111,04.15
84,2026-04-05,05:45,000.00,000.0,000.0,00.0,000,-111,04.15
85,2026-04-05,06:00,000.00,000.0,000.0,00.0,000,-111,04.16
86,2026-04-05,06:15,000.00,000.0,000.0,00.0,000,-111,04.15
87,2026-04-05,06:30,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,04:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,04:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:00;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,06:15;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11720 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-05,04:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:00;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,06:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:30;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=6:30:42
 Sleep=14:32 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.16V | Solar: 0.00V



[RTC] Time: 06:45
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 60/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 88
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :87,2026-04-05,06:30,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 6:30

NO GAPS FOUND ...


Current data inserted is 88,2026-04-05,06:45,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 88,2026-04-05,06:45,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 88,2026-04-05,06:45,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 5220
   ... [Tail Content] ...
00,-111,04.15
84,2026-04-05,05:45,000.00,000.0,000.0,00.0,000,-111,04.15
85,2026-04-05,06:00,000.00,000.0,000.0,00.0,000,-111,04.16
86,2026-04-05,06:15,000.00,000.0,000.0,00.0,000,-111,04.15
87,2026-04-05,06:30,000.00,000.0,000.0,00.0,000,-111,04.15
88,2026-04-05,06:45,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,04:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:00;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,06:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:30;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-05,05:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:00;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,06:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:45;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=6:45:42
 Sleep=14:32 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 07:00
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 60/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 89
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :88,2026-04-05,06:45,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 6:45

NO GAPS FOUND ...


Current data inserted is 89,2026-04-05,07:00,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 89,2026-04-05,07:00,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 89,2026-04-05,07:00,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 5280
   ... [Tail Content] ...
00,-111,04.15
85,2026-04-05,06:00,000.00,000.0,000.0,00.0,000,-111,04.16
86,2026-04-05,06:15,000.00,000.0,000.0,00.0,000,-111,04.15
87,2026-04-05,06:30,000.00,000.0,000.0,00.0,000,-111,04.15
88,2026-04-05,06:45,000.00,000.0,000.0,00.0,000,-111,04.15
89,2026-04-05,07:00,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,05:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:00;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,06:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:45;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-05,05:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:00;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,06:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,07:00;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=7:0:42
 Sleep=14:32 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 07:15
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
[SIM] PERSISTENT ERROR for 13 consecutive slots (~3h15m). Final resort: ESP32 SOFTWARE RESTART...
ets Jul 29 2019 12:21:46

rst:0xc (SW_CPU_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 12
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup was not caused by deep sleep: 0
[BOOT] Reset Reason: SW_CPU_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 12
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] ULP Anchors Synced: Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.16V | Solar: 0.00V



[RTC] Time: 07:15
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 1 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 61/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
Fresh boot detected. Skipping primary upload for sensor stabilization (queueing to Backlog).
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 90
RF Close date from RTC = 2026-04-05  
[SCHED] 🗓 Day Change Detected. Performing Rollover...
[SCHED] First rollover after boot/flash. Skipping destructive reset to preserve SPIFFS recovery.
[GoldenData] Starting Sent Mask Reconstruction from SPIFFS...
[GoldenData] Reconstruction Complete (Sent-Accurate).

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :89,2026-04-05,07:00,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 7:0
Gap filling for loop started... 
Appending record 07:15 (05-04-2026)
90
Primary skipped mid-day. Queuing CURRENT record to ftpunsent.txt...

append_text->store_text : Used for internal status: 90,2026-04-05,07:15,000.00,000.0,000.0,00.0,000,-111,04.16


append_text written to lastrecorded_<stationname>.txt is : 90,2026-04-05,07:15,000.00,000.0,000.0,00.0,000,-111,04.16



[FILE] SPIFFS: /001994_20260405.txt | Size: 5340
   ... [Tail Content] ...
00,-111,04.16
86,2026-04-05,06:15,000.00,000.0,000.0,00.0,000,-111,04.15
87,2026-04-05,06:30,000.00,000.0,000.0,00.0,000,-111,04.15
88,2026-04-05,06:45,000.00,000.0,000.0,00.0,000,-111,04.15
89,2026-04-05,07:00,000.00,000.0,000.0,00.0,000,-111,04.15
90,2026-04-05,07:15,000.00,000.0,000.0,00.0,000,-111,04.16
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,05:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:00;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,06:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,07:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,07:15;00.00;000.0;000.0;00.0;000;-111;04.16
-----------------------
--- UNSENT DATA END ---



[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak, but data already queued cleanly by scheduler skip.
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[SCHED] Stack HWM: 11212 bytes free
[GPRS] State Change: sync_mode=4 gprs_mode=2
[RTC] Time: 07:16
[RTC] Time: 07:17
[RTC] Time: 07:18
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=7:18:51
 Sleep=11:21 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 07:30
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 61/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 91
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :90,2026-04-05,07:15,000.00,000.0,000.0,00.0,000,-111,04.16

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 7:15

NO GAPS FOUND ...


Current data inserted is 91,2026-04-05,07:30,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 91,2026-04-05,07:30,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 91,2026-04-05,07:30,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 5400
   ... [Tail Content] ...
00,-111,04.15
87,2026-04-05,06:30,000.00,000.0,000.0,00.0,000,-111,04.15
88,2026-04-05,06:45,000.00,000.0,000.0,00.0,000,-111,04.15
89,2026-04-05,07:00,000.00,000.0,000.0,00.0,000,-111,04.15
90,2026-04-05,07:15,000.00,000.0,000.0,00.0,000,-111,04.16
91,2026-04-05,07:30,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,05:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,05:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:00;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,06:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,07:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,07:15;00.00;000.0;000.0;00.0;000;-111;04.16
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
[STORE] Monthly cum fail counter reset (New Month detected).
   ... [Tail FTP Unsent] ...
94;2026-04-05,05:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:00;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,06:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,07:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,07:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,07:30;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=7:30:41
 Sleep=14:33 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 07:45
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 61/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 92
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :91,2026-04-05,07:30,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 7:30

NO GAPS FOUND ...


Current data inserted is 92,2026-04-05,07:45,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 92,2026-04-05,07:45,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 92,2026-04-05,07:45,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 5460
   ... [Tail Content] ...
00,-111,04.15
88,2026-04-05,06:45,000.00,000.0,000.0,00.0,000,-111,04.15
89,2026-04-05,07:00,000.00,000.0,000.0,00.0,000,-111,04.15
90,2026-04-05,07:15,000.00,000.0,000.0,00.0,000,-111,04.16
91,2026-04-05,07:30,000.00,000.0,000.0,00.0,000,-111,04.15
92,2026-04-05,07:45,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,05:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:00;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,06:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,07:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,07:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,07:30;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-05,06:00;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,06:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,07:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,07:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,07:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,07:45;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=7:45:43
 Sleep=14:31 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.16V | Solar: 0.00V



[RTC] Time: 08:00
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM Error detected during polling.
[SIM] ERROR DETECTED (Timeout or CME).
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 61/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 93
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :92,2026-04-05,07:45,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 7:45

NO GAPS FOUND ...


Current data inserted is 93,2026-04-05,08:00,000.00,000.0,000.0,00.0,000,-111,04.15


append_text->store_text : Used for internal status: 93,2026-04-05,08:00,000.00,000.0,000.0,00.0,000,-111,04.15


append_text written to lastrecorded_<stationname>.txt is : 93,2026-04-05,08:00,000.00,000.0,000.0,00.0,000,-111,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 5520
   ... [Tail Content] ...
00,-111,04.15
89,2026-04-05,07:00,000.00,000.0,000.0,00.0,000,-111,04.15
90,2026-04-05,07:15,000.00,000.0,000.0,00.0,000,-111,04.16
91,2026-04-05,07:30,000.00,000.0,000.0,00.0,000,-111,04.15
92,2026-04-05,07:45,000.00,000.0,000.0,00.0,000,-111,04.15
93,2026-04-05,08:00,000.00,000.0,000.0,00.0,000,-111,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,06:00;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,06:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,07:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,07:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,07:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,07:45;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11548 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
94;2026-04-05,06:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,07:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,07:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,07:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,07:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,08:00;00.00;000.0;000.0;00.0;000;-111;04.15
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=8:0:42
 Sleep=14:32 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 08:15
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM ready in 1000 ms!
************************
GETTING SIGNAL STRENGTH 
************************

Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -71

************************
GETTING NETWORK 
************************
Raw CICCID Resp: 
+ICCID: 89914509006871527866

OK

Final CCID parsed: 89914509006871527866
[CACHE] New SIM or No Cache. Performing full discovery...
CSPN Logic response: 
+CSPN: "airtel",0

OK

COPS Logic response: 
+COPS: 0,2,"40445",7

OK

IMSI: 404450687152786
Service Provider APN is airteliot.com
Carrier: Airtel
Number: 404450687152786


************************
GETTING REGISTRATION 
************************
[GPRS] Fast-Track: Modem already registered! Bypassing setup block.
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Memory Check - Stored CCID: 89914509006871527866 | Stored APN: airteliot.com
Smart APN: Match Found!
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 61/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 94
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :93,2026-04-05,08:00,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 8:0

NO GAPS FOUND ...


Current data inserted is 94,2026-04-05,08:15,000.00,000.0,000.0,00.0,000,-071,04.15


append_text->store_text : Used for internal status: 94,2026-04-05,08:15,000.00,000.0,000.0,00.0,000,-071,04.15


append_text written to lastrecorded_<stationname>.txt is : 94,2026-04-05,08:15,000.00,000.0,000.0,00.0,000,-071,04.15



[FILE] SPIFFS: /001994_20260405.txt | Size: 5580
   ... [Tail Content] ...
00,-111,04.15
90,2026-04-05,07:15,000.00,000.0,000.0,00.0,000,-111,04.16
91,2026-04-05,07:30,000.00,000.0,000.0,00.0,000,-111,04.15
92,2026-04-05,07:45,000.00,000.0,000.0,00.0,000,-111,04.15
93,2026-04-05,08:00,000.00,000.0,000.0,00.0,000,-111,04.15
94,2026-04-05,08:15,000.00,000.0,000.0,00.0,000,-071,04.15
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
94;2026-04-05,06:15;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,06:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,07:00;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,07:15;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-05,07:30;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,07:45;00.00;000.0;000.0;00.0;000;-111;04.15
001994;2026-04-05,08:00;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11548 bytes free
[GPRS] Ghost PDP (0.0.0.0). Triggering recovery...
[GPRS] Bearer status check: Re-initializing IP stack...
[GPRS] Starting HTTP...
HTTP POST REQUEST IS AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001994_20260405.txt

Current Data to be sent is : 93,2026-04-05,08:00,000.00,000.0,000.0,00.0,000,-111,04.15
http_data format is stn_no=001994&rec_time=2026-04-05,08:15&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.15&bat_volt2=04.15

Payload is stn_no=001994&rec_time=2026-04-05,08:15&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.15&bat_volt2=04.15
[HTTP] Using Fast v3.0 Handshake...
Response of AT+HTTPACTION=1 is 
OK

+HTTPACTION: 1,200,9

Response (P) of AT+HTTPREAD=0,512 is PHP/5.4.16
X-Powered-By: PHP/5.4.16
Content-Length: 9
Content-Type: text/html; charset=UTF-8
OK

OK

+HTTPREAD: 9
Success


+HTTPREAD: 0

GPRS SEND : It is a Success
[RTC-Sync] No 'tm' field in server response. Skipping.
[DNS] Proactive cache updated (Raw IP Fallback DISABLED): rtdas.ksndmc.net -> 117.216.42.181
Saved APN Config: airteliot.com
[PWR] Signature persisted successfully.

**** Storing Last Logged Data as 2026-04-05,08:15
[RTC] Time: 08:16
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=40 cur_time=08:16 sched=NO cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[FTP] Backlog Push: 40 records found. Mode: THRESHOLD_MET

FTP file name is /TWSRF_001994_260405_081500.kwd

SampleNo  is 94

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Huge Backlog! Records chunked for TX. Original queue protected until success.
Retrieved file is /TWSRF_001994_260405_081500.kwd
Initializing A7672S for FTP...
[FTP] Airtel/Jio detected. Smart Default: Passive (1).

ERROR


OK

+CFTPSSTART: 0

[FTP] Configuring mode (1)...

OK

[FTP] Warming up DNS...
[FTP] DNS Resolved: 27.34.245.70

OK

+CFTPSLOGIN: 0

Result is 0
FTP Login success
File size: 945
[FTP] FSDEL path: TWSRF_001994_260405_081500.kwd
[FTP] FSOPEN cmd: AT+FSOPEN="C:/TWSRF_001994_260405_081500.kwd",0
[FTP] Found FH: 1
[FTP] Sending FSWRITE command...
[FTP] Data stream finished.
[FTP] Dispatching PUTFILE command...
[FTP] Dispatching PUTFILE command...
Response of AT+CFTPSPUTFILE: 
OK

+CFTPSPUTFILE: 0

[GoldenData] Marked records from /TWSRF_001994_260405_081500.kwd as DELIVERED.
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=8:17:57
 Sleep=13:14 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by external signal using RTC_IO
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM ready in 1000 ms!
************************
GETTING SIGNAL STRENGTH 
************************

Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -77

************************
GETTING NETWORK 
************************
Raw CICCID Resp: 
+ICCID: 89914509006871527866

OK

Final CCID parsed: 89914509006871527866
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] Fast-Track: Modem already registered! Bypassing setup block.
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Memory Check - Stored CCID: 89914509006871527866 | Stored APN: airteliot.com
Smart APN: Match Found!
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
CGACT Resp: 
OK

[RTC] Time: 08:18
[GPRS] State Change: sync_mode=4 gprs_mode=1
[RTC] Time: 08:19
[RTC] Time: 08:20
[RTC] Time: 08:21
[RTC] Time: 08:22
[RTC] Time: 08:23
[RTC] Time: 08:24
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=8:24:10
 Sleep=6:3 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] Loaded from SPIFFS: 13.003627,77.549286 (Fix: 04/04/2026)
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by external signal using RTC_IO
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001994_20260405.txt
SPIFFS [INIT]: lastrecorded_001994.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
[GPRS] Powering On...
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM ready in 1000 ms!
************************
GETTING SIGNAL STRENGTH 
************************

Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -79

************************
GETTING NETWORK 
************************
Raw CICCID Resp: 
+ICCID: 89914509006871527866

OK

Final CCID parsed: 89914509006871527866
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] Fast-Track: Modem already registered! Bypassing setup block.
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Memory Check - Stored CCID: 89914509006871527866 | Stored APN: airteliot.com
Smart APN: Match Found!
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
CGACT Resp: 
OK

[LCD] User selected DELETE_DATA. Awaiting confirmation...
[LCD] User confirmed Factory Reset via Keypad! Initiating...
[LCD] Factory Reset (Healthy FS). Wiping local logs...
[LCD] LOG WIPE COMPLETE. System reboot required.
ets Jul 29 2019 12:21:46

rst:0xc (SW_CPU_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 12
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup was not caused by deep sleep: 0
[BOOT] Reset Reason: SW_CPU_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
firmware.doc not found, creating default...
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 12
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] ULP Anchors Synced: Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.17V | Solar: 0.00V



[RTC] Time: 08:25
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: firmware.doc
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 1 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM ready in 1000 ms!
************************
GETTING SIGNAL STRENGTH 
************************

Signal strength IN gprs task is -75

************************
GETTING NETWORK 
************************
Raw CICCID Resp: 
+ICCID: 89914509006871527866

OK

Final CCID parsed: 89914509006871527866
[CACHE] New SIM or No Cache. Performing full discovery...
CSPN Logic response: 
+CSPN: "airtel",0

OK

COPS Logic response: 
+COPS: 0,2,"40445",7

OK

IMSI: 404450687152786
Service Provider APN is airteliot.com
Carrier: Airtel
Number: 404450687152786


************************
GETTING REGISTRATION 
************************
[GPRS] Fast-Track: Modem already registered! Bypassing setup block.
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

APN: Starting Carrier-Aware APN Search...
APN: Trying carrier-matched APN first -> airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
CGACT Resp: 
OK

Saved APN Config: airteliot.com
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 3/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
Fresh boot detected. Skipping primary upload for sensor stabilization (queueing to Backlog).
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 94
RF Close date from RTC = 2026-04-05  
[SCHED] 🗓 Day Change Detected. Performing Rollover...
[SCHED] First rollover after boot/flash. Skipping destructive reset to preserve SPIFFS recovery.
[GoldenData] Starting Sent Mask Reconstruction from SPIFFS...
[GoldenData] Reconstruction Complete (Sent-Accurate).

********** CREATING NEW FILE .. DEVICE STARTED AFTER A FEW DAYS  ***********

[SCHED] Fresh boot installation. Bypassing past gaps for clean start.
[SCHED] Queuing record to FTP backlog.
94,2026-04-05,08:15,000.00,000.0,000.0,00.0,000,-075,04.17





append_text->store_text : Used for internal status: 94,2026-04-05,08:15,000.00,000.0,000.0,00.0,000,-075,04.17


append_text written to lastrecorded_<stationname>.txt is : 94,2026-04-05,08:15,000.00,000.0,000.0,00.0,000,-075,04.17



[FILE] SPIFFS: /WS0221_20260405.txt | Size: 60
   ... [Tail Content] ...
94,2026-04-05,08:15,000.00,000.0,000.0,00.0,000,-075,04.17
-----------------------

[FILE] SD: /WS0221_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
WS0221;2026-04-05,08:15;00.00;000.0;000.0;00.0;000;-075;04.17
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11388 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[SCHED] Skipping Duplicate/Fresh Upload. Checking Backlog/Health...
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=1 cur_time=08:25 sched=NO cleanup=NO
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=8:25:28
 Sleep=4:43 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by external signal using RTC_IO
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.16V | Solar: 0.00V



[RTC] Time: 08:26
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: dailyftp_20260405.txt
[GPRS] Powering On...
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).
[GPRS] Waiting for active UART...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM ready in 1000 ms!
************************
GETTING SIGNAL STRENGTH 
************************

Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -79

************************
GETTING NETWORK 
************************
Raw CICCID Resp: 
+ICCID: 89914509006871527866

OK

Final CCID parsed: 89914509006871527866
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=2 (LastSucc:2 SlotsOn2G:0)
[RTC] Time: 08:27
[GPRS] CREG(2G) but CEREG also registered. Preferring LTE.
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Memory Check - Stored CCID: 89914509006871527866 | Stored APN: airteliot.com
Smart APN: Match Found!
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
CGACT Resp: 
OK

[GPRS] State Change: sync_mode=4 gprs_mode=1
[RTC] Time: 08:28
[RTC] Time: 08:29
[RTC] Time: 08:30
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 5/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 95
RF Close date from RTC = 2026-04-05  

********** CREATING NEW FILE .. DEVICE STARTED AFTER A FEW DAYS  ***********

GAPS IN SPIFFs/SD FILE IS APPENDED BUT WILL NOT BE SENT ... Signal Level = -111
00,2026-04-04,09:00,000.00,000.0,000.0,00.0,000,-112,04.16
01,2026-04-04,09:15,000.00,000.0,000.0,00.0,000,-112,04.16
02,2026-04-04,09:30,000.00,000.0,000.0,00.0,000,-112,04.16
03,2026-04-04,09:45,000.00,000.0,000.0,00.0,000,-112,04.16
04,2026-04-04,10:00,000.00,000.0,000.0,00.0,000,-112,04.16
05,2026-04-04,10:15,000.00,000.0,000.0,00.0,000,-112,04.16
06,2026-04-04,10:30,000.00,000.0,000.0,00.0,000,-112,04.16
07,2026-04-04,10:45,000.00,000.0,000.0,00.0,000,-112,04.16
08,2026-04-04,11:00,000.00,000.0,000.0,00.0,000,-112,04.16
09,2026-04-04,11:15,000.00,000.0,000.0,00.0,000,-112,04.16
10,2026-04-04,11:30,000.00,000.0,000.0,00.0,000,-112,04.16
11,2026-04-04,11:45,000.00,000.0,000.0,00.0,000,-112,04.16
12,2026-04-04,12:00,000.00,000.0,000.0,00.0,000,-112,04.16
13,2026-04-04,12:15,000.00,000.0,000.0,00.0,000,-112,04.16
14,2026-04-04,12:30,000.00,000.0,000.0,00.0,000,-112,04.16
15,2026-04-04,12:45,000.00,000.0,000.0,00.0,000,-112,04.16
16,2026-04-04,13:00,000.00,000.0,000.0,00.0,000,-112,04.16
17,2026-04-04,13:15,000.00,000.0,000.0,00.0,000,-112,04.16
18,2026-04-04,13:30,000.00,000.0,000.0,00.0,000,-112,04.16
19,2026-04-04,13:45,000.00,000.0,000.0,00.0,000,-112,04.16
20,2026-04-04,14:00,000.00,000.0,000.0,00.0,000,-112,04.16
21,2026-04-04,14:15,000.00,000.0,000.0,00.0,000,-112,04.16
22,2026-04-04,14:30,000.00,000.0,000.0,00.0,000,-112,04.16
23,2026-04-04,14:45,000.00,000.0,000.0,00.0,000,-112,04.16
24,2026-04-04,15:00,000.00,000.0,000.0,00.0,000,-112,04.16
25,2026-04-04,15:15,000.00,000.0,000.0,00.0,000,-112,04.16
26,2026-04-04,15:30,000.00,000.0,000.0,00.0,000,-112,04.16
27,2026-04-04,15:45,000.00,000.0,000.0,00.0,000,-112,04.16
28,2026-04-04,16:00,000.00,000.0,000.0,00.0,000,-112,04.16
29,2026-04-04,16:15,000.00,000.0,000.0,00.0,000,-112,04.16
30,2026-04-04,16:30,000.00,000.0,000.0,00.0,000,-112,04.16
31,2026-04-04,16:45,000.00,000.0,000.0,00.0,000,-112,04.16
32,2026-04-04,17:00,000.00,000.0,000.0,00.0,000,-112,04.16
33,2026-04-04,17:15,000.00,000.0,000.0,00.0,000,-112,04.16
34,2026-04-04,17:30,000.00,000.0,000.0,00.0,000,-112,04.16
35,2026-04-04,17:45,000.00,000.0,000.0,00.0,000,-112,04.16
36,2026-04-04,18:00,000.00,000.0,000.0,00.0,000,-112,04.16
37,2026-04-04,18:15,000.00,000.0,000.0,00.0,000,-112,04.16
38,2026-04-04,18:30,000.00,000.0,000.0,00.0,000,-112,04.16
39,2026-04-04,18:45,000.00,000.0,000.0,00.0,000,-112,04.16
40,2026-04-04,19:00,000.00,000.0,000.0,00.0,000,-112,04.16
41,2026-04-04,19:15,000.00,000.0,000.0,00.0,000,-112,04.16
42,2026-04-04,19:30,000.00,000.0,000.0,00.0,000,-112,04.16
43,2026-04-04,19:45,000.00,000.0,000.0,00.0,000,-112,04.16
44,2026-04-04,20:00,000.00,000.0,000.0,00.0,000,-112,04.16
45,2026-04-04,20:15,000.00,000.0,000.0,00.0,000,-112,04.16
46,2026-04-04,20:30,000.00,000.0,000.0,00.0,000,-112,04.16
47,2026-04-04,20:45,000.00,000.0,000.0,00.0,000,-112,04.16
48,2026-04-04,21:00,000.00,000.0,000.0,00.0,000,-112,04.16
49,2026-04-04,21:15,000.00,000.0,000.0,00.0,000,-112,04.16
50,2026-04-04,21:30,000.00,000.0,000.0,00.0,000,-112,04.16
51,2026-04-04,21:45,000.00,000.0,000.0,00.0,000,-112,04.16
52,2026-04-04,22:00,000.00,000.0,000.0,00.0,000,-112,04.16
53,2026-04-04,22:15,000.00,000.0,000.0,00.0,000,-112,04.16
54,2026-04-04,22:30,000.00,000.0,000.0,00.0,000,-112,04.16
55,2026-04-04,22:45,000.00,000.0,000.0,00.0,000,-112,04.16
56,2026-04-04,23:00,000.00,000.0,000.0,00.0,000,-112,04.16
57,2026-04-04,23:15,000.00,000.0,000.0,00.0,000,-112,04.16
58,2026-04-04,23:30,000.00,000.0,000.0,00.0,000,-112,04.16
59,2026-04-04,23:45,000.00,000.0,000.0,00.0,000,-112,04.16
60,2026-04-04,00:00,000.00,000.0,000.0,00.0,000,-112,04.16
61,2026-04-05,00:15,000.00,000.0,000.0,00.0,000,-112,04.16
62,2026-04-05,00:30,000.00,000.0,000.0,00.0,000,-112,04.16
63,2026-04-05,00:45,000.00,000.0,000.0,00.0,000,-112,04.16
64,2026-04-05,01:00,000.00,000.0,000.0,00.0,000,-112,04.16
65,2026-04-05,01:15,000.00,000.0,000.0,00.0,000,-112,04.16
66,2026-04-05,01:30,000.00,000.0,000.0,00.0,000,-112,04.16
67,2026-04-05,01:45,000.00,000.0,000.0,00.0,000,-112,04.16
68,2026-04-05,02:00,000.00,000.0,000.0,00.0,000,-112,04.16
69,2026-04-05,02:15,000.00,000.0,000.0,00.0,000,-112,04.16
70,2026-04-05,02:30,000.00,000.0,000.0,00.0,000,-112,04.16
71,2026-04-05,02:45,000.00,000.0,000.0,00.0,000,-112,04.16
72,2026-04-05,03:00,000.00,000.0,000.0,00.0,000,-112,04.16
73,2026-04-05,03:15,000.00,000.0,000.0,00.0,000,-112,04.16
74,2026-04-05,03:30,000.00,000.0,000.0,00.0,000,-112,04.16
75,2026-04-05,03:45,000.00,000.0,000.0,00.0,000,-112,04.16
76,2026-04-05,04:00,000.00,000.0,000.0,00.0,000,-112,04.16
77,2026-04-05,04:15,000.00,000.0,000.0,00.0,000,-112,04.16
78,2026-04-05,04:30,000.00,000.0,000.0,00.0,000,-112,04.16
79,2026-04-05,04:45,000.00,000.0,000.0,00.0,000,-112,04.16
80,2026-04-05,05:00,000.00,000.0,000.0,00.0,000,-112,04.16
81,2026-04-05,05:15,000.00,000.0,000.0,00.0,000,-112,04.16
82,2026-04-05,05:30,000.00,000.0,000.0,00.0,000,-112,04.16
83,2026-04-05,05:45,000.00,000.0,000.0,00.0,000,-112,04.16
84,2026-04-05,06:00,000.00,000.0,000.0,00.0,000,-112,04.16
85,2026-04-05,06:15,000.00,000.0,000.0,00.0,000,-112,04.16
86,2026-04-05,06:30,000.00,000.0,000.0,00.0,000,-112,04.16
87,2026-04-05,06:45,000.00,000.0,000.0,00.0,000,-112,04.16
88,2026-04-05,07:00,000.00,000.0,000.0,00.0,000,-112,04.16
89,2026-04-05,07:15,000.00,000.0,000.0,00.0,000,-112,04.16
90,2026-04-05,07:30,000.00,000.0,000.0,00.0,000,-112,04.16
91,2026-04-05,07:45,000.00,000.0,000.0,00.0,000,-112,04.16
92,2026-04-05,08:00,000.00,000.0,000.0,00.0,000,-112,04.16
93,2026-04-05,08:15,000.00,000.0,000.0,00.0,000,-112,04.16
94,2026-04-05,08:30,000.00,000.0,000.0,00.0,000,-112,04.16
[SCHED] Queuing record to FTP backlog.
95,2026-04-05,08:30,000.00,000.0,000.0,00.0,000,-079,04.16





append_text->store_text : Used for internal status: 95,2026-04-05,08:30,000.00,000.0,000.0,00.0,000,-079,04.16


append_text written to lastrecorded_<stationname>.txt is : 95,2026-04-05,08:30,000.00,000.0,000.0,00.0,000,-079,04.16

[SCHED] 08:30 AM Cycle Complete. Rollover pending at 08:45...


[FILE] SPIFFS: /001951_20260405.txt | Size: 5760
   ... [Tail Content] ...
00,-112,04.16
91,2026-04-05,07:45,000.00,000.0,000.0,00.0,000,-112,04.16
92,2026-04-05,08:00,000.00,000.0,000.0,00.0,000,-112,04.16
93,2026-04-05,08:15,000.00,000.0,000.0,00.0,000,-112,04.16
94,2026-04-05,08:30,000.00,000.0,000.0,00.0,000,-112,04.16
95,2026-04-05,08:30,000.00,000.0,000.0,00.0,000,-079,04.16
-----------------------

[FILE] SD: /001951_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
WS0221;2026-04-05,08:15;00.00;000.0;000.0;00.0;000;-075;04.17
001951;2026-04-05,08:30;00.00;000.0;000.0;00.0;000;-079;04.16
-----------------------
--- UNSENT DATA END ---



[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[SCHED] Stack HWM: 11576 bytes free
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[PWR] Sleep: CurTime=8:30:13
 Sleep=15:1 (min:sec)
[PWR] 🚨 RACE PREVENTED: Activity detected during final shutdown. Aborting sleep to process task.
****RTC read error 4 (Try 1/3)
****RTC read error 4 (Try 2/3)
****RTC read error 4 (Try 3/3)
[GPRS] Ghost PDP (0.0.0.0). Triggering recovery...
E (349558) task_wdt: Task watchdog got triggered. The following tasks/users did not reset the watchdog in time:
E (349558) task_wdt:  - rtcReadTask (CPU 1)
E (349559) task_wdt: Tasks currently running:
E (349559) task_wdt: CPU 0: gprsTask
E (349559) task_wdt: CPU 1: IDLE1
E (349559) task_wdt: Aborting.
E (349573) task_wdt: Print CPU 1 (current core) backtrace




Backtrace: 0x4008ba43:0x3ffcc5c0 0x401b5c45:0x3ffcc5e0 0x400902ff:0x3ffcc600 0x4008f379:0x3ffcc620




ELF file SHA256: 1ce42af0f

Rebooting...
ets Jul 29 2019 12:21:46

rst:0xc (SW_CPU_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:1344
load:0x40078000,len:13964
load:0x40080400,len:3600
entry 0x400805f0


[BOOT] HELLO! System starting... (Debug Enabled)
[HDC] Checking 1080 ID (Reg 0xFF): 0xFFFF
[HDC] Checking 2022 ID (Reg 0xFE): 0xFFFF
[HDC] Init: NO SENSOR FOUND! (ID Mismatch)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 12
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup was not caused by deep sleep: 0
[BOOT] Reset Reason: SW_CPU_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 12
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] ULP Anchors Synced: Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.18V | Solar: 0.00V



[RTC] Time: 08:32
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260405.txt
SPIFFS [INIT]: closingdata_001951.txt
[GPRS] Powering On...
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).
[GPRS] Waiting for active UART...

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM ready in 1000 ms!
************************
GETTING SIGNAL STRENGTH 
************************

Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -71

************************
GETTING NETWORK 
************************
Raw CICCID Resp: 
+ICCID: 89914509006871527866

OK

Final CCID parsed: 89914509006871527866
[CACHE] New SIM or No Cache. Performing full discovery...
CSPN Logic response: 
+CSPN: "airtel",0

OK

COPS Logic response: 
+COPS: 0,2,"40445",7

OK

IMSI: 404450687152786
Service Provider APN is airteliot.com
Carrier: Airtel
Number: 404450687152786


************************
GETTING REGISTRATION 
************************
[GPRS] Fast-Track: Modem already registered! Bypassing setup block.
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Memory Check - Stored CCID: 89914509006871527866 | Stored APN: airteliot.com
Smart APN: Match Found!
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.
[RESCUE] Temp forced 0.0 (HW Absent)

--- Sensor Data Snapshot ---
Temperature : 0.00 C
Humidity    : 0.00 %
Wind Speed  : 0.00 m/s
Wind Dir    : 0 deg
Wind Pulses : 0.00
Rain Pulses : 0.00
Rainfall    : 0.00 mm
----------------------------
--- Storage Status ---
SPIFFS: 12/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
Fresh boot detected. Skipping primary upload for sensor stabilization (queueing to Backlog).
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 95
RF Close date from RTC = 2026-04-05  
[SCHED] 🗓 Day Change Detected. Performing Rollover...
[SCHED] First rollover after boot/flash. Skipping destructive reset to preserve SPIFFS recovery.
[GoldenData] Starting Sent Mask Reconstruction from SPIFFS...
[GoldenData] Reconstruction Complete (Sent-Accurate).

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :95,2026-04-05,08:30,000.00,000.0,000.0,00.0,000,-079,04.16

Duplicate sample detected (TWS-RF).
Skipped data writing. Checking if GPRS needs to send unsent data then Sleep.
[SCHED] Signaling GPRS task for sequential backlog/health...
[SCHED] Stack HWM: 11384 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[SCHED] Skipping Duplicate/Fresh Upload. Checking Backlog/Health...
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=2 cur_time=08:32 sched=NO cleanup=NO
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[GPRS] State Change: sync_mode=4 gprs_mode=1
[RTC] Time: 08:33
[RTC] Time: 08:34
[RTC] Time: 08:35
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 08:36
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=8:36:9
 Sleep=9:3 (min:sec)
[PWR] Entering Deep Sleep
