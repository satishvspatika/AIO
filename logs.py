ets Jul 29 2019 12:21:46

rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
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
[BOOT] Fresh Start (Reason 1). Initializing RTC variables.
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001994
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.77 | Network: KSNDMC | Type: TWS-RF

Wakeup was not caused by deep sleep: 0
[BOOT] Reset Reason: POWERON_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 1
[ULP] Hard Power-On. Wiping ULP counters.
ULP Program loaded and started.
[BOOT] ULP Anchors Synced: Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.18V | Solar: 0.00V



[RTC] Time: 15:28
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
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
SPIFFS: 50/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
Fresh boot detected. Skipping primary upload for sensor stabilization (queueing to Backlog).
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 26
RF Close date from RTC = 2026-04-05  
[SCHED] 🗓 Day Change Detected. Performing Rollover...
[SCHED] First rollover after boot/flash. Skipping destructive reset to preserve SPIFFS recovery.
[GoldenData] Starting Sent Mask Reconstruction from SPIFFS...
[GoldenData] Reconstruction Complete (Sent-Accurate).

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :26,2026-04-04,15:15,000.00,000.0,000.0,00.0,000,-083,04.17

Duplicate sample detected (TWS-RF).
Skipped data writing. Checking if GPRS needs to send unsent data then Sleep.
[SCHED] Between intervals. Modem will remain OFF.
[SCHED] Stack HWM: 11396 bytes free
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[RTC] Time: 15:29
[RTC] Time: 15:30

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
SPIFFS: 50/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 27
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :26,2026-04-04,15:15,000.00,000.0,000.0,00.0,000,-083,04.17

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 15:15

NO GAPS FOUND ...


Current data inserted is 27,2026-04-04,15:30,000.00,000.0,000.0,00.0,000,-111,04.18


append_text->store_text : Used for internal status: 27,2026-04-04,15:30,000.00,000.0,000.0,00.0,000,-111,04.18


append_text written to lastrecorded_<stationname>.txt is : 27,2026-04-04,15:30,000.00,000.0,000.0,00.0,000,-111,04.18



[FILE] SPIFFS: /001994_20260405.txt | Size: 1560
   ... [Tail Content] ...
00,-079,04.16
23,2026-04-04,14:30,000.00,000.0,000.0,00.0,000,-081,04.16
24,2026-04-04,14:45,000.00,000.0,000.0,00.0,000,-081,04.16
25,2026-04-04,15:00,000.00,000.0,000.0,00.0,000,-081,04.17
26,2026-04-04,15:15,000.00,000.0,000.0,00.0,000,-083,04.17
27,2026-04-04,15:30,000.00,000.0,000.0,00.0,000,-111,04.18
-----------------------

[FILE] SD: /001994_20260405.txt


[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
[SCHED] Stack HWM: 11396 bytes free
[STORE] Monthly cum fail counter reset (New Month detected).
   ... [Tail FTP Unsent] ...
001994;2026-04-04,15:30;00.00;000.0;000.0;00.0;000;-111;04.18
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[GPRS] State Change: sync_mode=4 gprs_mode=2
[RTC] Time: 15:31
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=15:31:55
 Sleep=13:17 (min:sec)
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



[RTC] Time: 15:45
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
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
SPIFFS: 51/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 28
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :27,2026-04-04,15:30,000.00,000.0,000.0,00.0,000,-111,04.18

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 15:30

NO GAPS FOUND ...


Current data inserted is 28,2026-04-04,15:45,000.00,000.0,000.0,00.0,000,-111,04.16


append_text->store_text : Used for internal status: 28,2026-04-04,15:45,000.00,000.0,000.0,00.0,000,-111,04.16


append_text written to lastrecorded_<stationname>.txt is : 28,2026-04-04,15:45,000.00,000.0,000.0,00.0,000,-111,04.16



[FILE] SPIFFS: /001994_20260405.txt | Size: 1620
   ... [Tail Content] ...
00,-081,04.16
24,2026-04-04,14:45,000.00,000.0,000.0,00.0,000,-081,04.16
25,2026-04-04,15:00,000.00,000.0,000.0,00.0,000,-081,04.17
26,2026-04-04,15:15,000.00,000.0,000.0,00.0,000,-083,04.17
27,2026-04-04,15:30,000.00,000.0,000.0,00.0,000,-111,04.18
28,2026-04-04,15:45,000.00,000.0,000.0,00.0,000,-111,04.16
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
001994;2026-04-04,15:30;00.00;000.0;000.0;00.0;000;-111;04.18
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11544 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
001994;2026-04-04,15:30;00.00;000.0;000.0;00.0;000;-111;04.18
001994;2026-04-04,15:45;00.00;000.0;000.0;00.0;000;-111;04.16
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=15:45:42
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
[GPS] No persisted location found.
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



[RTC] Time: 16:00
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
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
SPIFFS: 51/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=2
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 29
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :28,2026-04-04,15:45,000.00,000.0,000.0,00.0,000,-111,04.16

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 15:45

NO GAPS FOUND ...


Current data inserted is 29,2026-04-04,16:00,000.00,000.0,000.0,00.0,000,-111,04.16


append_text->store_text : Used for internal status: 29,2026-04-04,16:00,000.00,000.0,000.0,00.0,000,-111,04.16


append_text written to lastrecorded_<stationname>.txt is : 29,2026-04-04,16:00,000.00,000.0,000.0,00.0,000,-111,04.16



[FILE] SPIFFS: /001994_20260405.txt | Size: 1680
   ... [Tail Content] ...
00,-081,04.16
25,2026-04-04,15:00,000.00,000.0,000.0,00.0,000,-081,04.17
26,2026-04-04,15:15,000.00,000.0,000.0,00.0,000,-083,04.17
27,2026-04-04,15:30,000.00,000.0,000.0,00.0,000,-111,04.18
28,2026-04-04,15:45,000.00,000.0,000.0,00.0,000,-111,04.16
29,2026-04-04,16:00,000.00,000.0,000.0,00.0,000,-111,04.16
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
001994;2026-04-04,15:30;00.00;000.0;000.0;00.0;000;-111;04.18
001994;2026-04-04,15:45;00.00;000.0;000.0;00.0;000;-111;04.16
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11544 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=2
[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.
   ... [Tail FTP Unsent] ...
001994;2026-04-04,15:30;00.00;000.0;000.0;00.0;000;-111;04.18
001994;2026-04-04,15:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,16:00;00.00;000.0;000.0;00.0;000;-111;04.16
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=2
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=16:0:43
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
[GPS] No persisted location found.
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



[RTC] Time: 16:15
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 1994_20260404.txt
SPIFFS [INIT]: dailyftp_20260404.txt
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
Signal strength IN gprs task is -81

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
SPIFFS: 51/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 30
RF Close date from RTC = 2026-04-05  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :29,2026-04-04,16:00,000.00,000.0,000.0,00.0,000,-111,04.16

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 16:0

NO GAPS FOUND ...


Current data inserted is 30,2026-04-04,16:15,000.00,000.0,000.0,00.0,000,-081,04.16


append_text->store_text : Used for internal status: 30,2026-04-04,16:15,000.00,000.0,000.0,00.0,000,-081,04.16


append_text written to lastrecorded_<stationname>.txt is : 30,2026-04-04,16:15,000.00,000.0,000.0,00.0,000,-081,04.16



[FILE] SPIFFS: /001994_20260405.txt | Size: 1740
   ... [Tail Content] ...
00,-081,04.17
26,2026-04-04,15:15,000.00,000.0,000.0,00.0,000,-083,04.17
27,2026-04-04,15:30,000.00,000.0,000.0,00.0,000,-111,04.18
28,2026-04-04,15:45,000.00,000.0,000.0,00.0,000,-111,04.16
29,2026-04-04,16:00,000.00,000.0,000.0,00.0,000,-111,04.16
30,2026-04-04,16:15,000.00,000.0,000.0,00.0,000,-081,04.16
-----------------------

[FILE] SD: /001994_20260405.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
001994;2026-04-04,15:30;00.00;000.0;000.0;00.0;000;-111;04.18
001994;2026-04-04,15:45;00.00;000.0;000.0;00.0;000;-111;04.16
001994;2026-04-04,16:00;00.00;000.0;000.0;00.0;000;-111;04.16
-----------------------
--- UNSENT DATA END ---



[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[Health] CDM window entirely missed today. Flagging FAIL.
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11544 bytes free
[GPRS] Ghost PDP (0.0.0.0). Triggering recovery...
[GPRS] Bearer status check: Re-initializing IP stack...
[GPRS] Starting HTTP...
HTTP POST REQUEST IS AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001994_20260405.txt

Current Data to be sent is : 29,2026-04-04,16:00,000.00,000.0,000.0,00.0,000,-111,04.16
http_data format is stn_no=001994&rec_time=2026-04-04,16:15&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-081&bat_volt=04.16&bat_volt2=04.16

Payload is stn_no=001994&rec_time=2026-04-04,16:15&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-081&bat_volt=04.16&bat_volt2=04.16
[HTTP] Using Fast v3.0 Handshake...
Response of AT+HTTPACTION=1 is 
OK

+HTTPACTION: 1,200,9

Response (P) of AT+HTTPREAD=0,512 is at Enterprise Linux) PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-04,16:15
[RTC] Time: 16:16
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=3 cur_time=16:16 sched=NO cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[FTP] Backlog Push: 3 records found. Mode: THRESHOLD_MET

FTP file name is /TWSRF_001994_260405_161500.kwd

SampleNo  is 30

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Backlog fully chunked into Active Payload.
Retrieved file is /TWSRF_001994_260405_161500.kwd
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
File size: 189
[FTP] FSDEL path: TWSRF_001994_260405_161500.kwd
[FTP] FSOPEN cmd: AT+FSOPEN="C:/TWSRF_001994_260405_161500.kwd",0
[FTP] Found FH: 1
[FTP] Sending FSWRITE command...
[FTP] Data stream finished.
[FTP] Dispatching PUTFILE command...
[FTP] Dispatching PUTFILE command...
Response of AT+CFTPSPUTFILE: 
OK

+CFTPSPUTFILE: 0

[GoldenData] Marked records from /TWSRF_001994_260405_161500.kwd as DELIVERED.
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=16:17:59
 Sleep=13:14 (min:sec)
[PWR] Entering Deep Sleep
