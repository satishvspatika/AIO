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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 23:00
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -69

************************
GETTING NETWORK 
************************
Final CCID parsed: 89914509006871527866
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] CREG(2G) but CEREG also registered. Preferring LTE.
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 38/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 57
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :56,2026-04-08,22:45,007.00,000.0,000.0,00.0,000,-097,04.10

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 22:45

NO GAPS FOUND ...


Current data inserted is 57,2026-04-08,23:00,007.00,000.0,000.0,00.0,000,-069,04.10


append_text->store_text : Used for internal status: 57,2026-04-08,23:00,007.00,000.0,000.0,00.0,000,-069,04.10


append_text written to lastrecorded_<stationname>.txt is : 57,2026-04-08,23:00,007.00,000.0,000.0,00.0,000,-069,04.10



[FILE] SPIFFS: /001951_20260409.txt | Size: 3480
   ... [Tail Content] ...
00,-069,04.10
53,2026-04-08,22:00,007.00,000.0,000.0,00.0,000,-071,04.10
54,2026-04-08,22:15,007.00,000.0,000.0,00.0,000,-111,04.10
55,2026-04-08,22:30,007.00,000.0,000.0,00.0,000,-069,04.10
56,2026-04-08,22:45,007.00,000.0,000.0,00.0,000,-097,04.10
57,2026-04-08,23:00,007.00,000.0,000.0,00.0,000,-069,04.10
-----------------------

[FILE] SD: /001951_20260409.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
51;2026-04-08,20:45;07.00;000.0;000.0;00.0;000;-071;04.10
001951;2026-04-08,21:00;07.00;000.0;000.0;00.0;000;-069;04.10
001951;2026-04-08,21:15;07.00;000.0;000.0;00.0;000;-069;04.10
001951;2026-04-08,21:30;07.00;000.0;000.0;00.0;000;-073;04.10
001951;2026-04-08,21:45;07.00;000.0;000.0;00.0;000;-069;04.10
001951;2026-04-08,22:00;07.00;000.0;000.0;00.0;000;-071;04.10
001951;2026-04-08,22:15;07.00;000.0;000.0;00.0;000;-111;04.10
001951;2026-04-08,22:30;07.00;000.0;000.0;00.0;000;-069;04.10
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11236 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 57,2026-04-08,23:00,007.00,000.0,000.0,00.0,000,-069,04.10
http_data format is stn_no=001951&rec_time=2026-04-08,23:00&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.10&bat_volt2=04.10

Payload is stn_no=001951&rec_time=2026-04-08,23:00&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.10&bat_volt2=04.10
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-08,23:00
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=34 cur_time=23:00 sched=NO cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[RTC] Time: 23:01
[FTP] Backlog Push: 34 records found. Mode: THRESHOLD_MET

FTP file name is /TWSRF_001951_260409_230000.kwd

SampleNo  is 57

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Huge Backlog! Records chunked for TX. Original queue protected until success.
Retrieved file is /TWSRF_001951_260409_230000.kwd
Initializing A7672S for FTP...
[FTP] Airtel/Jio detected. Smart Default: Passive (1).
[FTP] Configuring mode (1)...
1
[FTP] Warming up DNS...
[FTP] DNS Resolved: 27.34.245.70
[FTP] Login Result: 0
FTP Login success
[FTP] File size: 945
[FTP] FSDEL path: TWSRF_001951_260409_230000.kwd
[FTP] FSOPEN cmd: AT+FSOPEN="C:/TWSRF_001951_260409_230000.kwd",0
[FTP] Found FH: 1
[FTP] Sending FSWRITE command...
[FTP] Data stream finished.
[FTP] Dispatching PUTFILE command...
[FTP] Dispatching PUTFILE command...
Response of AT+CFTPSPUTFILE: 
OK

+CFTPSPUTFILE: 0

[GoldenData] Marked records from /TWSRF_001951_260409_230000.kwd as DELIVERED.

[GPRS] Checking SMS...

Removed READ/SENT messages (UNREAD preserved)
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 23:02
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=23:2:4
 Sleep=13:9 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 23:15
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -69

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 37/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 58
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :57,2026-04-08,23:00,007.00,000.0,000.0,00.0,000,-069,04.10

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 23:0

NO GAPS FOUND ...


Current data inserted is 58,2026-04-08,23:15,007.00,000.0,000.0,00.0,000,-069,04.10


append_text->store_text : Used for internal status: 58,2026-04-08,23:15,007.00,000.0,000.0,00.0,000,-069,04.10


append_text written to lastrecorded_<stationname>.txt is : 58,2026-04-08,23:15,007.00,000.0,000.0,00.0,000,-069,04.10



[FILE] SPIFFS: /001951_20260409.txt | Size: 3540
   ... [Tail Content] ...
00,-071,04.10
54,2026-04-08,22:15,007.00,000.0,000.0,00.0,000,-111,04.10
55,2026-04-08,22:30,007.00,000.0,000.0,00.0,000,-069,04.10
56,2026-04-08,22:45,007.00,000.0,000.0,00.0,000,-097,04.10
57,2026-04-08,23:00,007.00,000.0,000.0,00.0,000,-069,04.10
58,2026-04-08,23:15,007.00,000.0,000.0,00.0,000,-069,04.10
-----------------------

[FILE] SD: /001951_20260409.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
51;2026-04-08,20:45;07.00;000.0;000.0;00.0;000;-071;04.10
001951;2026-04-08,21:00;07.00;000.0;000.0;00.0;000;-069;04.10
001951;2026-04-08,21:15;07.00;000.0;000.0;00.0;000;-069;04.10
001951;2026-04-08,21:30;07.00;000.0;000.0;00.0;000;-073;04.10
001951;2026-04-08,21:45;07.00;000.0;000.0;00.0;000;-069;04.10
001951;2026-04-08,22:00;07.00;000.0;000.0;00.0;000;-071;04.10
001951;2026-04-08,22:15;07.00;000.0;000.0;00.0;000;-111;04.10
001951;2026-04-08,22:30;07.00;000.0;000.0;00.0;000;-069;04.10
-----------------------
--- UNSENT DATA END ---



[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11344 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 58,2026-04-08,23:15,007.00,000.0,000.0,00.0,000,-069,04.10
http_data format is stn_no=001951&rec_time=2026-04-08,23:15&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.10&bat_volt2=04.10

Payload is stn_no=001951&rec_time=2026-04-08,23:15&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.10&bat_volt2=04.10
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-08,23:15
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=19 cur_time=23:15 sched=NO cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[RTC] Time: 23:16
[FTP] Backlog Push: 19 records found. Mode: THRESHOLD_MET

FTP file name is /TWSRF_001951_260409_231500.kwd

SampleNo  is 58

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Huge Backlog! Records chunked for TX. Original queue protected until success.
Retrieved file is /TWSRF_001951_260409_231500.kwd
Initializing A7672S for FTP...
[FTP] Using SAVED Smart Mode: Passive
[FTP] Configuring mode (1)...
1
[FTP] Warming up DNS...
[FTP] DNS Resolved: 27.34.245.70
[FTP] Login Result: 0
FTP Login success
[FTP] File size: 945
[FTP] FSDEL path: TWSRF_001951_260409_231500.kwd
[FTP] FSOPEN cmd: AT+FSOPEN="C:/TWSRF_001951_260409_231500.kwd",0
[FTP] Found FH: 1
[FTP] Sending FSWRITE command...
[FTP] Data stream finished.
[FTP] Dispatching PUTFILE command...
[FTP] Dispatching PUTFILE command...
Response of AT+CFTPSPUTFILE: 
OK

+CFTPSPUTFILE: 0

[GoldenData] Marked records from /TWSRF_001951_260409_231500.kwd as DELIVERED.
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 23:17
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=23:17:7
 Sleep=13:7 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 23:30
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: ftpunsent.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -71

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 37/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 59
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :58,2026-04-08,23:15,007.00,000.0,000.0,00.0,000,-069,04.10

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 23:15

NO GAPS FOUND ...


Current data inserted is 59,2026-04-08,23:30,007.00,000.0,000.0,00.0,000,-071,04.10


append_text->store_text : Used for internal status: 59,2026-04-08,23:30,007.00,000.0,000.0,00.0,000,-071,04.10


append_text written to lastrecorded_<stationname>.txt is : 59,2026-04-08,23:30,007.00,000.0,000.0,00.0,000,-071,04.10



[FILE] SPIFFS: /001951_20260409.txt | Size: 3600
   ... [Tail Content] ...
00,-111,04.10
55,2026-04-08,22:30,007.00,000.0,000.0,00.0,000,-069,04.10
56,2026-04-08,22:45,007.00,000.0,000.0,00.0,000,-097,04.10
57,2026-04-08,23:00,007.00,000.0,000.0,00.0,000,-069,04.10
58,2026-04-08,23:15,007.00,000.0,000.0,00.0,000,-069,04.10
59,2026-04-08,23:30,007.00,000.0,000.0,00.0,000,-071,04.10
-----------------------

[FILE] SD: /001951_20260409.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
001951;2026-04-08,21:45;07.00;000.0;000.0;00.0;000;-069;04.10
001951;2026-04-08,22:00;07.00;000.0;000.0;00.0;000;-071;04.10
001951;2026-04-08,22:15;07.00;000.0;000.0;00.0;000;-111;04.10
001951;2026-04-08,22:30;07.00;000.0;000.0;00.0;000;-069;04.10
-----------------------
--- UNSENT DATA END ---



[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11236 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 59,2026-04-08,23:30,007.00,000.0,000.0,00.0,000,-071,04.10
http_data format is stn_no=001951&rec_time=2026-04-08,23:30&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.10&bat_volt2=04.10

Payload is stn_no=001951&rec_time=2026-04-08,23:30&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.10&bat_volt2=04.10
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-08,23:30
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=4 cur_time=23:30 sched=NO cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[RTC] Time: 23:31
[FTP] Backlog Push: 4 records found. Mode: THRESHOLD_MET

FTP file name is /TWSRF_001951_260409_233000.kwd

SampleNo  is 59

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Backlog fully chunked into Active Payload.
Retrieved file is /TWSRF_001951_260409_233000.kwd
Initializing A7672S for FTP...
[FTP] Using SAVED Smart Mode: Passive
[FTP] Configuring mode (1)...
1
[FTP] Warming up DNS...
[FTP] DNS Resolved: 27.34.245.70
[FTP] Login Result: 0
FTP Login success
[FTP] File size: 252
[FTP] FSDEL path: TWSRF_001951_260409_233000.kwd
[FTP] FSOPEN cmd: AT+FSOPEN="C:/TWSRF_001951_260409_233000.kwd",0
[FTP] Found FH: 1
[FTP] Sending FSWRITE command...
[FTP] Data stream finished.
[FTP] Dispatching PUTFILE command...
[FTP] Dispatching PUTFILE command...
Response of AT+CFTPSPUTFILE: 
OK

+CFTPSPUTFILE: 0

[GoldenData] Marked records from /TWSRF_001951_260409_233000.kwd as DELIVERED.
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 23:32
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=23:32:1
 Sleep=13:12 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 23:45
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Final CCID parsed: 89914509006871527866
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] CREG(2G) but CEREG also registered. Preferring LTE.
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 37/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 60
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :59,2026-04-08,23:30,007.00,000.0,000.0,00.0,000,-071,04.10

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 23:30

NO GAPS FOUND ...


Current data inserted is 60,2026-04-08,23:45,007.00,000.0,000.0,00.0,000,-071,04.09


append_text->store_text : Used for internal status: 60,2026-04-08,23:45,007.00,000.0,000.0,00.0,000,-071,04.09


append_text written to lastrecorded_<stationname>.txt is : 60,2026-04-08,23:45,007.00,000.0,000.0,00.0,000,-071,04.09



[FILE] SPIFFS: /001951_20260409.txt | Size: 3660
   ... [Tail Content] ...
00,-069,04.10
56,2026-04-08,22:45,007.00,000.0,000.0,00.0,000,-097,04.10
57,2026-04-08,23:00,007.00,000.0,000.0,00.0,000,-069,04.10
58,2026-04-08,23:15,007.00,000.0,000.0,00.0,000,-069,04.10
59,2026-04-08,23:30,007.00,000.0,000.0,00.0,000,-071,04.10
60,2026-04-08,23:45,007.00,000.0,000.0,00.0,000,-071,04.09
-----------------------

[FILE] SD: /001951_20260409.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11264 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 60,2026-04-08,23:45,007.00,000.0,000.0,00.0,000,-071,04.09
http_data format is stn_no=001951&rec_time=2026-04-08,23:45&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.09&bat_volt2=04.09

Payload is stn_no=001951&rec_time=2026-04-08,23:45&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.09&bat_volt2=04.09
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-08,23:45
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=23:45 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 23:46
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=23:46:5
 Sleep=14:9 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.12V | Solar: 0.00V



[RTC] Time: 00:00
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -71

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 37/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 61
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :60,2026-04-08,23:45,007.00,000.0,000.0,00.0,000,-071,04.09

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 23:45

NO GAPS FOUND ...


Current data inserted is 61,2026-04-09,00:00,007.00,000.0,000.0,00.0,000,-071,04.10


append_text->store_text : Used for internal status: 61,2026-04-09,00:00,007.00,000.0,000.0,00.0,000,-071,04.10


append_text written to lastrecorded_<stationname>.txt is : 61,2026-04-09,00:00,007.00,000.0,000.0,00.0,000,-071,04.10



[FILE] SPIFFS: /001951_20260409.txt | Size: 3720
   ... [Tail Content] ...
00,-097,04.10
57,2026-04-08,23:00,007.00,000.0,000.0,00.0,000,-069,04.10
58,2026-04-08,23:15,007.00,000.0,000.0,00.0,000,-069,04.10
59,2026-04-08,23:30,007.00,000.0,000.0,00.0,000,-071,04.10
60,2026-04-08,23:45,007.00,000.0,000.0,00.0,000,-071,04.09
61,2026-04-09,00:00,007.00,000.0,000.0,00.0,000,-071,04.10
-----------------------

[FILE] SD: /001951_20260409.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11236 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 61,2026-04-09,00:00,007.00,000.0,000.0,00.0,000,-071,04.10
http_data format is stn_no=001951&rec_time=2026-04-09,00:00&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.10&bat_volt2=04.10

Payload is stn_no=001951&rec_time=2026-04-09,00:00&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.10&bat_volt2=04.10
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,00:00
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=00:00 sched=YES cleanup=NO

[GPRS] Checking SMS...

Removed READ/SENT messages (UNREAD preserved)
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 00:01
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=0:1:8
 Sleep=14:4 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 00:15
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -73

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 37/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 62
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :61,2026-04-09,00:00,007.00,000.0,000.0,00.0,000,-071,04.10

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 0:0

NO GAPS FOUND ...


Current data inserted is 62,2026-04-09,00:15,007.00,000.0,000.0,00.0,000,-073,04.10


append_text->store_text : Used for internal status: 62,2026-04-09,00:15,007.00,000.0,000.0,00.0,000,-073,04.10


append_text written to lastrecorded_<stationname>.txt is : 62,2026-04-09,00:15,007.00,000.0,000.0,00.0,000,-073,04.10



[FILE] SPIFFS: /001951_20260409.txt | Size: 3780
   ... [Tail Content] ...
00,-069,04.10
58,2026-04-08,23:15,007.00,000.0,000.0,00.0,000,-069,04.10
59,2026-04-08,23:30,007.00,000.0,000.0,00.0,000,-071,04.10
60,2026-04-08,23:45,007.00,000.0,000.0,00.0,000,-071,04.09
61,2026-04-09,00:00,007.00,000.0,000.0,00.0,000,-071,04.10
62,2026-04-09,00:15,007.00,000.0,000.0,00.0,000,-073,04.10
-----------------------

[FILE] SD: /001951_20260409.txt


[SCHED] Stack HWM: 11348 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 62,2026-04-09,00:15,007.00,000.0,000.0,00.0,000,-073,04.10
http_data format is stn_no=001951&rec_time=2026-04-09,00:15&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-073&bat_volt=04.10&bat_volt2=04.10

Payload is stn_no=001951&rec_time=2026-04-09,00:15&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-073&bat_volt=04.10&bat_volt2=04.10
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,00:15
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=00:15 sched=YES cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 00:16
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=0:16:8
 Sleep=14:6 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 00:30
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -73

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 37/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 63
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :62,2026-04-09,00:15,007.00,000.0,000.0,00.0,000,-073,04.10

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 0:15

NO GAPS FOUND ...


Current data inserted is 63,2026-04-09,00:30,007.00,000.0,000.0,00.0,000,-073,04.10


append_text->store_text : Used for internal status: 63,2026-04-09,00:30,007.00,000.0,000.0,00.0,000,-073,04.10


append_text written to lastrecorded_<stationname>.txt is : 63,2026-04-09,00:30,007.00,000.0,000.0,00.0,000,-073,04.10



[FILE] SPIFFS: /001951_20260409.txt | Size: 3840
   ... [Tail Content] ...
00,-069,04.10
59,2026-04-08,23:30,007.00,000.0,000.0,00.0,000,-071,04.10
60,2026-04-08,23:45,007.00,000.0,000.0,00.0,000,-071,04.09
61,2026-04-09,00:00,007.00,000.0,000.0,00.0,000,-071,04.10
62,2026-04-09,00:15,007.00,000.0,000.0,00.0,000,-073,04.10
63,2026-04-09,00:30,007.00,000.0,000.0,00.0,000,-073,04.10
-----------------------

[FILE] SD: /001951_20260409.txt


[SCHED] Stack HWM: 11236 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 63,2026-04-09,00:30,007.00,000.0,000.0,00.0,000,-073,04.10
http_data format is stn_no=001951&rec_time=2026-04-09,00:30&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-073&bat_volt=04.10&bat_volt2=04.10

Payload is stn_no=001951&rec_time=2026-04-09,00:30&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-073&bat_volt=04.10&bat_volt2=04.10
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,00:30
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=00:30 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 00:31
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=0:31:7
 Sleep=14:8 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 00:45
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -71

************************
GETTING NETWORK 
************************
Final CCID parsed: 89914509006871527866
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] CREG(2G) but CEREG also registered. Preferring LTE.
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 37/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 64
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :63,2026-04-09,00:30,007.00,000.0,000.0,00.0,000,-073,04.10

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 0:30

NO GAPS FOUND ...


Current data inserted is 64,2026-04-09,00:45,007.00,000.0,000.0,00.0,000,-071,04.09


append_text->store_text : Used for internal status: 64,2026-04-09,00:45,007.00,000.0,000.0,00.0,000,-071,04.09


append_text written to lastrecorded_<stationname>.txt is : 64,2026-04-09,00:45,007.00,000.0,000.0,00.0,000,-071,04.09



[FILE] SPIFFS: /001951_20260409.txt | Size: 3900
   ... [Tail Content] ...
00,-071,04.10
60,2026-04-08,23:45,007.00,000.0,000.0,00.0,000,-071,04.09
61,2026-04-09,00:00,007.00,000.0,000.0,00.0,000,-071,04.10
62,2026-04-09,00:15,007.00,000.0,000.0,00.0,000,-073,04.10
63,2026-04-09,00:30,007.00,000.0,000.0,00.0,000,-073,04.10
64,2026-04-09,00:45,007.00,000.0,000.0,00.0,000,-071,04.09
-----------------------

[FILE] SD: /001951_20260409.txt


[SCHED] Stack HWM: 11348 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 64,2026-04-09,00:45,007.00,000.0,000.0,00.0,000,-071,04.09
http_data format is stn_no=001951&rec_time=2026-04-09,00:45&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.09&bat_volt2=04.09

Payload is stn_no=001951&rec_time=2026-04-09,00:45&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.09&bat_volt2=04.09
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,00:45
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=00:45 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 00:46
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=0:46:8
 Sleep=14:6 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 01:00
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -75

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 37/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 65
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :64,2026-04-09,00:45,007.00,000.0,000.0,00.0,000,-071,04.09

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 0:45

NO GAPS FOUND ...


Current data inserted is 65,2026-04-09,01:00,007.00,000.0,000.0,00.0,000,-075,04.10


append_text->store_text : Used for internal status: 65,2026-04-09,01:00,007.00,000.0,000.0,00.0,000,-075,04.10


append_text written to lastrecorded_<stationname>.txt is : 65,2026-04-09,01:00,007.00,000.0,000.0,00.0,000,-075,04.10



[FILE] SPIFFS: /001951_20260409.txt | Size: 3960
   ... [Tail Content] ...
00,-071,04.09
61,2026-04-09,00:00,007.00,000.0,000.0,00.0,000,-071,04.10
62,2026-04-09,00:15,007.00,000.0,000.0,00.0,000,-073,04.10
63,2026-04-09,00:30,007.00,000.0,000.0,00.0,000,-073,04.10
64,2026-04-09,00:45,007.00,000.0,000.0,00.0,000,-071,04.09
65,2026-04-09,01:00,007.00,000.0,000.0,00.0,000,-075,04.10
-----------------------

[FILE] SD: /001951_20260409.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11396 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 65,2026-04-09,01:00,007.00,000.0,000.0,00.0,000,-075,04.10
http_data format is stn_no=001951&rec_time=2026-04-09,01:00&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-075&bat_volt=04.10&bat_volt2=04.10

Payload is stn_no=001951&rec_time=2026-04-09,01:00&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-075&bat_volt=04.10&bat_volt2=04.10
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,01:00
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=01:00 sched=NO cleanup=NO

[GPRS] Checking SMS...

[RTC] Time: 01:01
Removed READ/SENT messages (UNREAD preserved)
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=1:1:9
 Sleep=14:5 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 01:15
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -69

************************
GETTING NETWORK 
************************
Final CCID parsed: 89914509006871527866
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] CREG(2G) but CEREG also registered. Preferring LTE.
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 37/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 66
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :65,2026-04-09,01:00,007.00,000.0,000.0,00.0,000,-075,04.10

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 1:0

NO GAPS FOUND ...


Current data inserted is 66,2026-04-09,01:15,007.00,000.0,000.0,00.0,000,-069,04.09


append_text->store_text : Used for internal status: 66,2026-04-09,01:15,007.00,000.0,000.0,00.0,000,-069,04.09


append_text written to lastrecorded_<stationname>.txt is : 66,2026-04-09,01:15,007.00,000.0,000.0,00.0,000,-069,04.09



[FILE] SPIFFS: /001951_20260409.txt | Size: 4020
   ... [Tail Content] ...
00,-071,04.10
62,2026-04-09,00:15,007.00,000.0,000.0,00.0,000,-073,04.10
63,2026-04-09,00:30,007.00,000.0,000.0,00.0,000,-073,04.10
64,2026-04-09,00:45,007.00,000.0,000.0,00.0,000,-071,04.09
65,2026-04-09,01:00,007.00,000.0,000.0,00.0,000,-075,04.10
66,2026-04-09,01:15,007.00,000.0,000.0,00.0,000,-069,04.09
-----------------------

[FILE] SD: /001951_20260409.txt


[SCHED] Stack HWM: 11236 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 66,2026-04-09,01:15,007.00,000.0,000.0,00.0,000,-069,04.09
http_data format is stn_no=001951&rec_time=2026-04-09,01:15&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.09&bat_volt2=04.09

Payload is stn_no=001951&rec_time=2026-04-09,01:15&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.09&bat_volt2=04.09
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,01:15
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=01:15 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[RTC] Time: 01:16
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=1:16:6
 Sleep=14:6 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 01:30
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -75

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 37/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 67
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :66,2026-04-09,01:15,007.00,000.0,000.0,00.0,000,-069,04.09

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 1:15

NO GAPS FOUND ...


Current data inserted is 67,2026-04-09,01:30,007.00,000.0,000.0,00.0,000,-075,04.10


append_text->store_text : Used for internal status: 67,2026-04-09,01:30,007.00,000.0,000.0,00.0,000,-075,04.10


append_text written to lastrecorded_<stationname>.txt is : 67,2026-04-09,01:30,007.00,000.0,000.0,00.0,000,-075,04.10



[FILE] SPIFFS: /001951_20260409.txt | Size: 4080
   ... [Tail Content] ...
00,-073,04.10
63,2026-04-09,00:30,007.00,000.0,000.0,00.0,000,-073,04.10
64,2026-04-09,00:45,007.00,000.0,000.0,00.0,000,-071,04.09
65,2026-04-09,01:00,007.00,000.0,000.0,00.0,000,-075,04.10
66,2026-04-09,01:15,007.00,000.0,000.0,00.0,000,-069,04.09
67,2026-04-09,01:30,007.00,000.0,000.0,00.0,000,-075,04.10
-----------------------

[FILE] SD: /001951_20260409.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11236 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 67,2026-04-09,01:30,007.00,000.0,000.0,00.0,000,-075,04.10
http_data format is stn_no=001951&rec_time=2026-04-09,01:30&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-075&bat_volt=04.10&bat_volt2=04.10

Payload is stn_no=001951&rec_time=2026-04-09,01:30&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-075&bat_volt=04.10&bat_volt2=04.10
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,01:30
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=01:30 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 01:31
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=1:31:8
 Sleep=14:6 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 01:45
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -77

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 37/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 68
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :67,2026-04-09,01:30,007.00,000.0,000.0,00.0,000,-075,04.10

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 1:30

NO GAPS FOUND ...


Current data inserted is 68,2026-04-09,01:45,007.00,000.0,000.0,00.0,000,-077,04.09


append_text->store_text : Used for internal status: 68,2026-04-09,01:45,007.00,000.0,000.0,00.0,000,-077,04.09


append_text written to lastrecorded_<stationname>.txt is : 68,2026-04-09,01:45,007.00,000.0,000.0,00.0,000,-077,04.09



[FILE] SPIFFS: /001951_20260409.txt | Size: 4140
   ... [Tail Content] ...
00,-073,04.10
64,2026-04-09,00:45,007.00,000.0,000.0,00.0,000,-071,04.09
65,2026-04-09,01:00,007.00,000.0,000.0,00.0,000,-075,04.10
66,2026-04-09,01:15,007.00,000.0,000.0,00.0,000,-069,04.09
67,2026-04-09,01:30,007.00,000.0,000.0,00.0,000,-075,04.10
68,2026-04-09,01:45,007.00,000.0,000.0,00.0,000,-077,04.09
-----------------------

[FILE] SD: /001951_20260409.txt


[SCHED] Stack HWM: 11408 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 68,2026-04-09,01:45,007.00,000.0,000.0,00.0,000,-077,04.09
http_data format is stn_no=001951&rec_time=2026-04-09,01:45&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-077&bat_volt=04.09&bat_volt2=04.09

Payload is stn_no=001951&rec_time=2026-04-09,01:45&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-077&bat_volt=04.09&bat_volt2=04.09
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,01:45
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=01:45 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 01:46
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=1:46:8
 Sleep=14:7 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 02:00
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Final CCID parsed: 89914509006871527866
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] CREG(2G) but CEREG also registered. Preferring LTE.
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 37/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 69
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :68,2026-04-09,01:45,007.00,000.0,000.0,00.0,000,-077,04.09

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 1:45

NO GAPS FOUND ...


Current data inserted is 69,2026-04-09,02:00,007.00,000.0,000.0,00.0,000,-071,04.09


append_text->store_text : Used for internal status: 69,2026-04-09,02:00,007.00,000.0,000.0,00.0,000,-071,04.09


append_text written to lastrecorded_<stationname>.txt is : 69,2026-04-09,02:00,007.00,000.0,000.0,00.0,000,-071,04.09



[FILE] SPIFFS: /001951_20260409.txt | Size: 4200
   ... [Tail Content] ...
00,-071,04.09
65,2026-04-09,01:00,007.00,000.0,000.0,00.0,000,-075,04.10
66,2026-04-09,01:15,007.00,000.0,000.0,00.0,000,-069,04.09
67,2026-04-09,01:30,007.00,000.0,000.0,00.0,000,-075,04.10
68,2026-04-09,01:45,007.00,000.0,000.0,00.0,000,-077,04.09
69,2026-04-09,02:00,007.00,000.0,000.0,00.0,000,-071,04.09
-----------------------

[FILE] SD: /001951_20260409.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11408 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 69,2026-04-09,02:00,007.00,000.0,000.0,00.0,000,-071,04.09
http_data format is stn_no=001951&rec_time=2026-04-09,02:00&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.09&bat_volt2=04.09

Payload is stn_no=001951&rec_time=2026-04-09,02:00&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.09&bat_volt2=04.09
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,02:00
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=02:00 sched=NO cleanup=NO

[GPRS] Checking SMS...

Removed READ/SENT messages (UNREAD preserved)
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[RTC] Time: 02:01
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=2:1:9
 Sleep=14:6 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 02:15
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Final CCID parsed: 89914509006871527866
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] CREG(2G) but CEREG also registered. Preferring LTE.
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 37/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 70
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :69,2026-04-09,02:00,007.00,000.0,000.0,00.0,000,-071,04.09

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 2:0

NO GAPS FOUND ...


Current data inserted is 70,2026-04-09,02:15,007.00,000.0,000.0,00.0,000,-071,04.09


append_text->store_text : Used for internal status: 70,2026-04-09,02:15,007.00,000.0,000.0,00.0,000,-071,04.09


append_text written to lastrecorded_<stationname>.txt is : 70,2026-04-09,02:15,007.00,000.0,000.0,00.0,000,-071,04.09



[FILE] SPIFFS: /001951_20260409.txt | Size: 4260
   ... [Tail Content] ...
00,-075,04.10
66,2026-04-09,01:15,007.00,000.0,000.0,00.0,000,-069,04.09
67,2026-04-09,01:30,007.00,000.0,000.0,00.0,000,-075,04.10
68,2026-04-09,01:45,007.00,000.0,000.0,00.0,000,-077,04.09
69,2026-04-09,02:00,007.00,000.0,000.0,00.0,000,-071,04.09
70,2026-04-09,02:15,007.00,000.0,000.0,00.0,000,-071,04.09
-----------------------

[FILE] SD: /001951_20260409.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11348 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 70,2026-04-09,02:15,007.00,000.0,000.0,00.0,000,-071,04.09
http_data format is stn_no=001951&rec_time=2026-04-09,02:15&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.09&bat_volt2=04.09

Payload is stn_no=001951&rec_time=2026-04-09,02:15&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.09&bat_volt2=04.09
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,02:15
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=02:15 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 02:16
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=2:16:5
 Sleep=14:8 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 02:30
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -73

************************
GETTING NETWORK 
************************
Final CCID parsed: 89914509006871527866
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] CREG(2G) but CEREG also registered. Preferring LTE.
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 37/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 71
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :70,2026-04-09,02:15,007.00,000.0,000.0,00.0,000,-071,04.09

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 2:15

NO GAPS FOUND ...


Current data inserted is 71,2026-04-09,02:30,007.00,000.0,000.0,00.0,000,-073,04.10


append_text->store_text : Used for internal status: 71,2026-04-09,02:30,007.00,000.0,000.0,00.0,000,-073,04.10


append_text written to lastrecorded_<stationname>.txt is : 71,2026-04-09,02:30,007.00,000.0,000.0,00.0,000,-073,04.10



[FILE] SPIFFS: /001951_20260409.txt | Size: 4320
   ... [Tail Content] ...
00,-069,04.09
67,2026-04-09,01:30,007.00,000.0,000.0,00.0,000,-075,04.10
68,2026-04-09,01:45,007.00,000.0,000.0,00.0,000,-077,04.09
69,2026-04-09,02:00,007.00,000.0,000.0,00.0,000,-071,04.09
70,2026-04-09,02:15,007.00,000.0,000.0,00.0,000,-071,04.09
71,2026-04-09,02:30,007.00,000.0,000.0,00.0,000,-073,04.10
-----------------------

[FILE] SD: /001951_20260409.txt


[SCHED] Stack HWM: 11236 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 71,2026-04-09,02:30,007.00,000.0,000.0,00.0,000,-073,04.10
http_data format is stn_no=001951&rec_time=2026-04-09,02:30&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-073&bat_volt=04.10&bat_volt2=04.10

Payload is stn_no=001951&rec_time=2026-04-09,02:30&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-073&bat_volt=04.10&bat_volt2=04.10
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,02:30
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=02:30 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 02:31
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=2:31:7
 Sleep=14:6 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 02:45
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -77

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 38/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 72
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :71,2026-04-09,02:30,007.00,000.0,000.0,00.0,000,-073,04.10

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 2:30

NO GAPS FOUND ...


Current data inserted is 72,2026-04-09,02:45,007.00,000.0,000.0,00.0,000,-077,04.09


append_text->store_text : Used for internal status: 72,2026-04-09,02:45,007.00,000.0,000.0,00.0,000,-077,04.09


append_text written to lastrecorded_<stationname>.txt is : 72,2026-04-09,02:45,007.00,000.0,000.0,00.0,000,-077,04.09



[FILE] SPIFFS: /001951_20260409.txt | Size: 4380
   ... [Tail Content] ...
00,-075,04.10
68,2026-04-09,01:45,007.00,000.0,000.0,00.0,000,-077,04.09
69,2026-04-09,02:00,007.00,000.0,000.0,00.0,000,-071,04.09
70,2026-04-09,02:15,007.00,000.0,000.0,00.0,000,-071,04.09
71,2026-04-09,02:30,007.00,000.0,000.0,00.0,000,-073,04.10
72,2026-04-09,02:45,007.00,000.0,000.0,00.0,000,-077,04.09
-----------------------

[FILE] SD: /001951_20260409.txt


[SCHED] Stack HWM: 11236 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 72,2026-04-09,02:45,007.00,000.0,000.0,00.0,000,-077,04.09
http_data format is stn_no=001951&rec_time=2026-04-09,02:45&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-077&bat_volt=04.09&bat_volt2=04.09

Payload is stn_no=001951&rec_time=2026-04-09,02:45&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-077&bat_volt=04.09&bat_volt2=04.09
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,02:45
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=02:45 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[RTC] Time: 02:46
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=2:46:9
 Sleep=14:6 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 03:00
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -75

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 38/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 73
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :72,2026-04-09,02:45,007.00,000.0,000.0,00.0,000,-077,04.09

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 2:45

NO GAPS FOUND ...


Current data inserted is 73,2026-04-09,03:00,007.00,000.0,000.0,00.0,000,-075,04.10


append_text->store_text : Used for internal status: 73,2026-04-09,03:00,007.00,000.0,000.0,00.0,000,-075,04.10


append_text written to lastrecorded_<stationname>.txt is : 73,2026-04-09,03:00,007.00,000.0,000.0,00.0,000,-075,04.10



[FILE] SPIFFS: /001951_20260409.txt | Size: 4440
   ... [Tail Content] ...
00,-077,04.09
69,2026-04-09,02:00,007.00,000.0,000.0,00.0,000,-071,04.09
70,2026-04-09,02:15,007.00,000.0,000.0,00.0,000,-071,04.09
71,2026-04-09,02:30,007.00,000.0,000.0,00.0,000,-073,04.10
72,2026-04-09,02:45,007.00,000.0,000.0,00.0,000,-077,04.09
73,2026-04-09,03:00,007.00,000.0,000.0,00.0,000,-075,04.10
-----------------------

[FILE] SD: /001951_20260409.txt


[SCHED] Stack HWM: 11152 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 73,2026-04-09,03:00,007.00,000.0,000.0,00.0,000,-075,04.10
http_data format is stn_no=001951&rec_time=2026-04-09,03:00&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-075&bat_volt=04.10&bat_volt2=04.10

Payload is stn_no=001951&rec_time=2026-04-09,03:00&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-075&bat_volt=04.10&bat_volt2=04.10
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,03:00
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=03:00 sched=YES cleanup=NO

[GPRS] Checking SMS...

[RTC] Time: 03:01
Removed READ/SENT messages (UNREAD preserved)
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=3:1:9
 Sleep=14:4 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 03:15
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -77

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 38/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 74
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :73,2026-04-09,03:00,007.00,000.0,000.0,00.0,000,-075,04.10

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 3:0

NO GAPS FOUND ...


Current data inserted is 74,2026-04-09,03:15,007.00,000.0,000.0,00.0,000,-077,04.09


append_text->store_text : Used for internal status: 74,2026-04-09,03:15,007.00,000.0,000.0,00.0,000,-077,04.09


append_text written to lastrecorded_<stationname>.txt is : 74,2026-04-09,03:15,007.00,000.0,000.0,00.0,000,-077,04.09



[FILE] SPIFFS: /001951_20260409.txt | Size: 4500
   ... [Tail Content] ...
00,-071,04.09
70,2026-04-09,02:15,007.00,000.0,000.0,00.0,000,-071,04.09
71,2026-04-09,02:30,007.00,000.0,000.0,00.0,000,-073,04.10
72,2026-04-09,02:45,007.00,000.0,000.0,00.0,000,-077,04.09
73,2026-04-09,03:00,007.00,000.0,000.0,00.0,000,-075,04.10
74,2026-04-09,03:15,007.00,000.0,000.0,00.0,000,-077,04.09
-----------------------

[FILE] SD: /001951_20260409.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11236 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 74,2026-04-09,03:15,007.00,000.0,000.0,00.0,000,-077,04.09
http_data format is stn_no=001951&rec_time=2026-04-09,03:15&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-077&bat_volt=04.09&bat_volt2=04.09

Payload is stn_no=001951&rec_time=2026-04-09,03:15&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-077&bat_volt=04.09&bat_volt2=04.09
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: 
OK

+HTTPREAD: 9
Success


+HTTPREAD: 0

GPRS SEND : It is a Success
[RTC-Sync] No 'tm' field in server response. Skipping.
[DNS] Proactive cache updated (Raw IP Fallback DISABLED): rtdas.ksndmc.net -> 117.216.42.181
[PWR] Signature persisted successfully.

**** Storing Last Logged Data as 2026-04-09,03:15
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=03:15 sched=YES cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[RTC] Time: 03:16
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=3:16:6
 Sleep=14:6 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 03:30
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -75

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 38/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 75
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :74,2026-04-09,03:15,007.00,000.0,000.0,00.0,000,-077,04.09

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 3:15

NO GAPS FOUND ...


Current data inserted is 75,2026-04-09,03:30,007.00,000.0,000.0,00.0,000,-075,04.09


append_text->store_text : Used for internal status: 75,2026-04-09,03:30,007.00,000.0,000.0,00.0,000,-075,04.09


append_text written to lastrecorded_<stationname>.txt is : 75,2026-04-09,03:30,007.00,000.0,000.0,00.0,000,-075,04.09



[FILE] SPIFFS: /001951_20260409.txt | Size: 4560
   ... [Tail Content] ...
00,-071,04.09
71,2026-04-09,02:30,007.00,000.0,000.0,00.0,000,-073,04.10
72,2026-04-09,02:45,007.00,000.0,000.0,00.0,000,-077,04.09
73,2026-04-09,03:00,007.00,000.0,000.0,00.0,000,-075,04.10
74,2026-04-09,03:15,007.00,000.0,000.0,00.0,000,-077,04.09
75,2026-04-09,03:30,007.00,000.0,000.0,00.0,000,-075,04.09
-----------------------

[FILE] SD: /001951_20260409.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11236 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 75,2026-04-09,03:30,007.00,000.0,000.0,00.0,000,-075,04.09
http_data format is stn_no=001951&rec_time=2026-04-09,03:30&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-075&bat_volt=04.09&bat_volt2=04.09

Payload is stn_no=001951&rec_time=2026-04-09,03:30&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-075&bat_volt=04.09&bat_volt2=04.09
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,03:30
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=03:30 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 03:31
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=3:31:7
 Sleep=14:8 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 03:45
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -73

************************
GETTING NETWORK 
************************
Final CCID parsed: 89914509006871527866
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] CREG(2G) but CEREG also registered. Preferring LTE.
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 38/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 76
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :75,2026-04-09,03:30,007.00,000.0,000.0,00.0,000,-075,04.09

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 3:30

NO GAPS FOUND ...


Current data inserted is 76,2026-04-09,03:45,007.00,000.0,000.0,00.0,000,-073,04.09


append_text->store_text : Used for internal status: 76,2026-04-09,03:45,007.00,000.0,000.0,00.0,000,-073,04.09


append_text written to lastrecorded_<stationname>.txt is : 76,2026-04-09,03:45,007.00,000.0,000.0,00.0,000,-073,04.09



[FILE] SPIFFS: /001951_20260409.txt | Size: 4620
   ... [Tail Content] ...
00,-073,04.10
72,2026-04-09,02:45,007.00,000.0,000.0,00.0,000,-077,04.09
73,2026-04-09,03:00,007.00,000.0,000.0,00.0,000,-075,04.10
74,2026-04-09,03:15,007.00,000.0,000.0,00.0,000,-077,04.09
75,2026-04-09,03:30,007.00,000.0,000.0,00.0,000,-075,04.09
76,2026-04-09,03:45,007.00,000.0,000.0,00.0,000,-073,04.09
-----------------------

[FILE] SD: /001951_20260409.txt


[SCHED] Stack HWM: 11236 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 76,2026-04-09,03:45,007.00,000.0,000.0,00.0,000,-073,04.09
http_data format is stn_no=001951&rec_time=2026-04-09,03:45&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-073&bat_volt=04.09&bat_volt2=04.09

Payload is stn_no=001951&rec_time=2026-04-09,03:45&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-073&bat_volt=04.09&bat_volt2=04.09
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,03:45
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=03:45 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 03:46
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=3:46:5
 Sleep=14:8 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 04:00
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -77

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 38/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 77
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :76,2026-04-09,03:45,007.00,000.0,000.0,00.0,000,-073,04.09

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 3:45

NO GAPS FOUND ...


Current data inserted is 77,2026-04-09,04:00,007.00,000.0,000.0,00.0,000,-077,04.09


append_text->store_text : Used for internal status: 77,2026-04-09,04:00,007.00,000.0,000.0,00.0,000,-077,04.09


append_text written to lastrecorded_<stationname>.txt is : 77,2026-04-09,04:00,007.00,000.0,000.0,00.0,000,-077,04.09



[FILE] SPIFFS: /001951_20260409.txt | Size: 4680
   ... [Tail Content] ...
00,-077,04.09
73,2026-04-09,03:00,007.00,000.0,000.0,00.0,000,-075,04.10
74,2026-04-09,03:15,007.00,000.0,000.0,00.0,000,-077,04.09
75,2026-04-09,03:30,007.00,000.0,000.0,00.0,000,-075,04.09
76,2026-04-09,03:45,007.00,000.0,000.0,00.0,000,-073,04.09
77,2026-04-09,04:00,007.00,000.0,000.0,00.0,000,-077,04.09
-----------------------

[FILE] SD: /001951_20260409.txt


[SCHED] Stack HWM: 11348 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 77,2026-04-09,04:00,007.00,000.0,000.0,00.0,000,-077,04.09
http_data format is stn_no=001951&rec_time=2026-04-09,04:00&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-077&bat_volt=04.09&bat_volt2=04.09

Payload is stn_no=001951&rec_time=2026-04-09,04:00&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-077&bat_volt=04.09&bat_volt2=04.09
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,04:00
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=04:00 sched=NO cleanup=NO

[GPRS] Checking SMS...

Removed READ/SENT messages (UNREAD preserved)
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 04:01
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=4:1:8
 Sleep=14:5 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 04:15
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -77

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 38/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 78
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :77,2026-04-09,04:00,007.00,000.0,000.0,00.0,000,-077,04.09

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 4:0

NO GAPS FOUND ...


Current data inserted is 78,2026-04-09,04:15,007.00,000.0,000.0,00.0,000,-077,04.09


append_text->store_text : Used for internal status: 78,2026-04-09,04:15,007.00,000.0,000.0,00.0,000,-077,04.09


append_text written to lastrecorded_<stationname>.txt is : 78,2026-04-09,04:15,007.00,000.0,000.0,00.0,000,-077,04.09



[FILE] SPIFFS: /001951_20260409.txt | Size: 4740
   ... [Tail Content] ...
00,-075,04.10
74,2026-04-09,03:15,007.00,000.0,000.0,00.0,000,-077,04.09
75,2026-04-09,03:30,007.00,000.0,000.0,00.0,000,-075,04.09
76,2026-04-09,03:45,007.00,000.0,000.0,00.0,000,-073,04.09
77,2026-04-09,04:00,007.00,000.0,000.0,00.0,000,-077,04.09
78,2026-04-09,04:15,007.00,000.0,000.0,00.0,000,-077,04.09
-----------------------

[FILE] SD: /001951_20260409.txt


[SCHED] Stack HWM: 11236 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 78,2026-04-09,04:15,007.00,000.0,000.0,00.0,000,-077,04.09
http_data format is stn_no=001951&rec_time=2026-04-09,04:15&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-077&bat_volt=04.09&bat_volt2=04.09

Payload is stn_no=001951&rec_time=2026-04-09,04:15&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-077&bat_volt=04.09&bat_volt2=04.09
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,04:15
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=04:15 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 04:16
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=4:16:8
 Sleep=14:6 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 04:30
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -73

************************
GETTING NETWORK 
************************
Final CCID parsed: 89914509006871527866
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
Reg Search [BSNL:0]... Status:0 Iter:#1/24
[GPRS] CGREG registered during adaptive wait!
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 38/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 79
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :78,2026-04-09,04:15,007.00,000.0,000.0,00.0,000,-077,04.09

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 4:15

NO GAPS FOUND ...


Current data inserted is 79,2026-04-09,04:30,007.00,000.0,000.0,00.0,000,-073,04.09


append_text->store_text : Used for internal status: 79,2026-04-09,04:30,007.00,000.0,000.0,00.0,000,-073,04.09


append_text written to lastrecorded_<stationname>.txt is : 79,2026-04-09,04:30,007.00,000.0,000.0,00.0,000,-073,04.09



[FILE] SPIFFS: /001951_20260409.txt | Size: 4800
   ... [Tail Content] ...
00,-077,04.09
75,2026-04-09,03:30,007.00,000.0,000.0,00.0,000,-075,04.09
76,2026-04-09,03:45,007.00,000.0,000.0,00.0,000,-073,04.09
77,2026-04-09,04:00,007.00,000.0,000.0,00.0,000,-077,04.09
78,2026-04-09,04:15,007.00,000.0,000.0,00.0,000,-077,04.09
79,2026-04-09,04:30,007.00,000.0,000.0,00.0,000,-073,04.09
-----------------------

[FILE] SD: /001951_20260409.txt


[SCHED] Stack HWM: 11152 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 79,2026-04-09,04:30,007.00,000.0,000.0,00.0,000,-073,04.09
http_data format is stn_no=001951&rec_time=2026-04-09,04:30&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-073&bat_volt=04.09&bat_volt2=04.09

Payload is stn_no=001951&rec_time=2026-04-09,04:30&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-073&bat_volt=04.09&bat_volt2=04.09
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,04:30
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=04:30 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 04:31
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=4:31:8
 Sleep=14:7 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 04:45
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -77

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 39/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 80
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :79,2026-04-09,04:30,007.00,000.0,000.0,00.0,000,-073,04.09

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 4:30

NO GAPS FOUND ...


Current data inserted is 80,2026-04-09,04:45,007.00,000.0,000.0,00.0,000,-077,04.10


append_text->store_text : Used for internal status: 80,2026-04-09,04:45,007.00,000.0,000.0,00.0,000,-077,04.10


append_text written to lastrecorded_<stationname>.txt is : 80,2026-04-09,04:45,007.00,000.0,000.0,00.0,000,-077,04.10



[FILE] SPIFFS: /001951_20260409.txt | Size: 4860
   ... [Tail Content] ...
00,-075,04.09
76,2026-04-09,03:45,007.00,000.0,000.0,00.0,000,-073,04.09
77,2026-04-09,04:00,007.00,000.0,000.0,00.0,000,-077,04.09
78,2026-04-09,04:15,007.00,000.0,000.0,00.0,000,-077,04.09
79,2026-04-09,04:30,007.00,000.0,000.0,00.0,000,-073,04.09
80,2026-04-09,04:45,007.00,000.0,000.0,00.0,000,-077,04.10
-----------------------

[FILE] SD: /001951_20260409.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11408 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 80,2026-04-09,04:45,007.00,000.0,000.0,00.0,000,-077,04.10
http_data format is stn_no=001951&rec_time=2026-04-09,04:45&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-077&bat_volt=04.10&bat_volt2=04.10

Payload is stn_no=001951&rec_time=2026-04-09,04:45&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-077&bat_volt=04.10&bat_volt2=04.10
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,04:45
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=04:45 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[RTC] Time: 04:46
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=4:46:9
 Sleep=14:6 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 05:00
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -77

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 39/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 81
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :80,2026-04-09,04:45,007.00,000.0,000.0,00.0,000,-077,04.10

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 4:45

NO GAPS FOUND ...


Current data inserted is 81,2026-04-09,05:00,007.00,000.0,000.0,00.0,000,-077,04.09


append_text->store_text : Used for internal status: 81,2026-04-09,05:00,007.00,000.0,000.0,00.0,000,-077,04.09


append_text written to lastrecorded_<stationname>.txt is : 81,2026-04-09,05:00,007.00,000.0,000.0,00.0,000,-077,04.09



[FILE] SPIFFS: /001951_20260409.txt | Size: 4920
   ... [Tail Content] ...
00,-073,04.09
77,2026-04-09,04:00,007.00,000.0,000.0,00.0,000,-077,04.09
78,2026-04-09,04:15,007.00,000.0,000.0,00.0,000,-077,04.09
79,2026-04-09,04:30,007.00,000.0,000.0,00.0,000,-073,04.09
80,2026-04-09,04:45,007.00,000.0,000.0,00.0,000,-077,04.10
81,2026-04-09,05:00,007.00,000.0,000.0,00.0,000,-077,04.09
-----------------------

[FILE] SD: /001951_20260409.txt


[SCHED] Stack HWM: 11396 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 81,2026-04-09,05:00,007.00,000.0,000.0,00.0,000,-077,04.09
http_data format is stn_no=001951&rec_time=2026-04-09,05:00&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-077&bat_volt=04.09&bat_volt2=04.09

Payload is stn_no=001951&rec_time=2026-04-09,05:00&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-077&bat_volt=04.09&bat_volt2=04.09
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,05:00
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=05:00 sched=NO cleanup=NO

[GPRS] Checking SMS...

Removed READ/SENT messages (UNREAD preserved)
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[RTC] Time: 05:01
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=5:1:11
 Sleep=14:4 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 05:15
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -75

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 39/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 82
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :81,2026-04-09,05:00,007.00,000.0,000.0,00.0,000,-077,04.09

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 5:0

NO GAPS FOUND ...


Current data inserted is 82,2026-04-09,05:15,007.00,000.0,000.0,00.0,000,-075,04.09


append_text->store_text : Used for internal status: 82,2026-04-09,05:15,007.00,000.0,000.0,00.0,000,-075,04.09


append_text written to lastrecorded_<stationname>.txt is : 82,2026-04-09,05:15,007.00,000.0,000.0,00.0,000,-075,04.09



[FILE] SPIFFS: /001951_20260409.txt | Size: 4980
   ... [Tail Content] ...
00,-077,04.09
78,2026-04-09,04:15,007.00,000.0,000.0,00.0,000,-077,04.09
79,2026-04-09,04:30,007.00,000.0,000.0,00.0,000,-073,04.09
80,2026-04-09,04:45,007.00,000.0,000.0,00.0,000,-077,04.10
81,2026-04-09,05:00,007.00,000.0,000.0,00.0,000,-077,04.09
82,2026-04-09,05:15,007.00,000.0,000.0,00.0,000,-075,04.09
-----------------------

[FILE] SD: /001951_20260409.txt


[SCHED] Stack HWM: 11236 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 82,2026-04-09,05:15,007.00,000.0,000.0,00.0,000,-075,04.09
http_data format is stn_no=001951&rec_time=2026-04-09,05:15&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-075&bat_volt=04.09&bat_volt2=04.09

Payload is stn_no=001951&rec_time=2026-04-09,05:15&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-075&bat_volt=04.09&bat_volt2=04.09
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,05:15
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[RTC] Time: 05:16
[FTP-Gate] unsent=0 cur_time=05:15 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=5:16:9
 Sleep=14:6 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 05:30
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -75

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 39/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 83
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :82,2026-04-09,05:15,007.00,000.0,000.0,00.0,000,-075,04.09

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 5:15

NO GAPS FOUND ...


Current data inserted is 83,2026-04-09,05:30,007.00,000.0,000.0,00.0,000,-075,04.10


append_text->store_text : Used for internal status: 83,2026-04-09,05:30,007.00,000.0,000.0,00.0,000,-075,04.10


append_text written to lastrecorded_<stationname>.txt is : 83,2026-04-09,05:30,007.00,000.0,000.0,00.0,000,-075,04.10



[FILE] SPIFFS: /001951_20260409.txt | Size: 5040
   ... [Tail Content] ...
00,-077,04.09
79,2026-04-09,04:30,007.00,000.0,000.0,00.0,000,-073,04.09
80,2026-04-09,04:45,007.00,000.0,000.0,00.0,000,-077,04.10
81,2026-04-09,05:00,007.00,000.0,000.0,00.0,000,-077,04.09
82,2026-04-09,05:15,007.00,000.0,000.0,00.0,000,-075,04.09
83,2026-04-09,05:30,007.00,000.0,000.0,00.0,000,-075,04.10
-----------------------

[FILE] SD: /001951_20260409.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11348 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 83,2026-04-09,05:30,007.00,000.0,000.0,00.0,000,-075,04.10
http_data format is stn_no=001951&rec_time=2026-04-09,05:30&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-075&bat_volt=04.10&bat_volt2=04.10

Payload is stn_no=001951&rec_time=2026-04-09,05:30&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-075&bat_volt=04.10&bat_volt2=04.10
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,05:30
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=05:30 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[RTC] Time: 05:31
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=5:31:9
 Sleep=14:6 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 05:45
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Final CCID parsed: 89914509006871527866
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] CREG(2G) but CEREG also registered. Preferring LTE.
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 39/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 84
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :83,2026-04-09,05:30,007.00,000.0,000.0,00.0,000,-075,04.10

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 5:30

NO GAPS FOUND ...


Current data inserted is 84,2026-04-09,05:45,007.00,000.0,000.0,00.0,000,-071,04.09


append_text->store_text : Used for internal status: 84,2026-04-09,05:45,007.00,000.0,000.0,00.0,000,-071,04.09


append_text written to lastrecorded_<stationname>.txt is : 84,2026-04-09,05:45,007.00,000.0,000.0,00.0,000,-071,04.09



[FILE] SPIFFS: /001951_20260409.txt | Size: 5100
   ... [Tail Content] ...
00,-073,04.09
80,2026-04-09,04:45,007.00,000.0,000.0,00.0,000,-077,04.10
81,2026-04-09,05:00,007.00,000.0,000.0,00.0,000,-077,04.09
82,2026-04-09,05:15,007.00,000.0,000.0,00.0,000,-075,04.09
83,2026-04-09,05:30,007.00,000.0,000.0,00.0,000,-075,04.10
84,2026-04-09,05:45,007.00,000.0,000.0,00.0,000,-071,04.09
-----------------------

[FILE] SD: /001951_20260409.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11408 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 84,2026-04-09,05:45,007.00,000.0,000.0,00.0,000,-071,04.09
http_data format is stn_no=001951&rec_time=2026-04-09,05:45&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.09&bat_volt2=04.09

Payload is stn_no=001951&rec_time=2026-04-09,05:45&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.09&bat_volt2=04.09
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,05:45
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=05:45 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 05:46
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=5:46:6
 Sleep=14:7 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 06:00
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -75

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 39/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 85
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :84,2026-04-09,05:45,007.00,000.0,000.0,00.0,000,-071,04.09

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 5:45

NO GAPS FOUND ...


Current data inserted is 85,2026-04-09,06:00,007.00,000.0,000.0,00.0,000,-075,04.09


append_text->store_text : Used for internal status: 85,2026-04-09,06:00,007.00,000.0,000.0,00.0,000,-075,04.09


append_text written to lastrecorded_<stationname>.txt is : 85,2026-04-09,06:00,007.00,000.0,000.0,00.0,000,-075,04.09



[FILE] SPIFFS: /001951_20260409.txt | Size: 5160
   ... [Tail Content] ...
00,-077,04.10
81,2026-04-09,05:00,007.00,000.0,000.0,00.0,000,-077,04.09
82,2026-04-09,05:15,007.00,000.0,000.0,00.0,000,-075,04.09
83,2026-04-09,05:30,007.00,000.0,000.0,00.0,000,-075,04.10
84,2026-04-09,05:45,007.00,000.0,000.0,00.0,000,-071,04.09
85,2026-04-09,06:00,007.00,000.0,000.0,00.0,000,-075,04.09
-----------------------

[FILE] SD: /001951_20260409.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11236 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 85,2026-04-09,06:00,007.00,000.0,000.0,00.0,000,-075,04.09
http_data format is stn_no=001951&rec_time=2026-04-09,06:00&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-075&bat_volt=04.09&bat_volt2=04.09

Payload is stn_no=001951&rec_time=2026-04-09,06:00&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-075&bat_volt=04.09&bat_volt2=04.09
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,06:00
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=06:00 sched=YES cleanup=NO
[RTC] Time: 06:01

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
[PWR] Sleep: CurTime=6:1:9
 Sleep=14:4 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 06:15
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -73

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 39/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 86
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :85,2026-04-09,06:00,007.00,000.0,000.0,00.0,000,-075,04.09

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 6:0

NO GAPS FOUND ...


Current data inserted is 86,2026-04-09,06:15,007.00,000.0,000.0,00.0,000,-073,04.09


append_text->store_text : Used for internal status: 86,2026-04-09,06:15,007.00,000.0,000.0,00.0,000,-073,04.09


append_text written to lastrecorded_<stationname>.txt is : 86,2026-04-09,06:15,007.00,000.0,000.0,00.0,000,-073,04.09



[FILE] SPIFFS: /001951_20260409.txt | Size: 5220
   ... [Tail Content] ...
00,-077,04.09
82,2026-04-09,05:15,007.00,000.0,000.0,00.0,000,-075,04.09
83,2026-04-09,05:30,007.00,000.0,000.0,00.0,000,-075,04.10
84,2026-04-09,05:45,007.00,000.0,000.0,00.0,000,-071,04.09
85,2026-04-09,06:00,007.00,000.0,000.0,00.0,000,-075,04.09
86,2026-04-09,06:15,007.00,000.0,000.0,00.0,000,-073,04.09
-----------------------

[FILE] SD: /001951_20260409.txt


[SCHED] Stack HWM: 11408 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 86,2026-04-09,06:15,007.00,000.0,000.0,00.0,000,-073,04.09
http_data format is stn_no=001951&rec_time=2026-04-09,06:15&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-073&bat_volt=04.09&bat_volt2=04.09

Payload is stn_no=001951&rec_time=2026-04-09,06:15&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-073&bat_volt=04.09&bat_volt2=04.09
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,06:15
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=06:15 sched=YES cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 06:16
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=6:16:8
 Sleep=14:5 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 06:30
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Final CCID parsed: 89914509006871527866
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] CREG(2G) but CEREG also registered. Preferring LTE.
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 39/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 87
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :86,2026-04-09,06:15,007.00,000.0,000.0,00.0,000,-073,04.09

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 6:15

NO GAPS FOUND ...


Current data inserted is 87,2026-04-09,06:30,007.00,000.0,000.0,00.0,000,-071,04.09


append_text->store_text : Used for internal status: 87,2026-04-09,06:30,007.00,000.0,000.0,00.0,000,-071,04.09


append_text written to lastrecorded_<stationname>.txt is : 87,2026-04-09,06:30,007.00,000.0,000.0,00.0,000,-071,04.09



[FILE] SPIFFS: /001951_20260409.txt | Size: 5280
   ... [Tail Content] ...
00,-075,04.09
83,2026-04-09,05:30,007.00,000.0,000.0,00.0,000,-075,04.10
84,2026-04-09,05:45,007.00,000.0,000.0,00.0,000,-071,04.09
85,2026-04-09,06:00,007.00,000.0,000.0,00.0,000,-075,04.09
86,2026-04-09,06:15,007.00,000.0,000.0,00.0,000,-073,04.09
87,2026-04-09,06:30,007.00,000.0,000.0,00.0,000,-071,04.09
-----------------------

[FILE] SD: /001951_20260409.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11236 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 87,2026-04-09,06:30,007.00,000.0,000.0,00.0,000,-071,04.09
http_data format is stn_no=001951&rec_time=2026-04-09,06:30&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.09&bat_volt2=04.09

Payload is stn_no=001951&rec_time=2026-04-09,06:30&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.09&bat_volt2=04.09
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,06:30
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=06:30 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 06:31
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=6:31:4
 Sleep=14:9 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 06:45
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -73

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 40/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 88
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :87,2026-04-09,06:30,007.00,000.0,000.0,00.0,000,-071,04.09

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 6:30

NO GAPS FOUND ...


Current data inserted is 88,2026-04-09,06:45,007.00,000.0,000.0,00.0,000,-073,04.09


append_text->store_text : Used for internal status: 88,2026-04-09,06:45,007.00,000.0,000.0,00.0,000,-073,04.09


append_text written to lastrecorded_<stationname>.txt is : 88,2026-04-09,06:45,007.00,000.0,000.0,00.0,000,-073,04.09



[FILE] SPIFFS: /001951_20260409.txt | Size: 5340
   ... [Tail Content] ...
00,-075,04.10
84,2026-04-09,05:45,007.00,000.0,000.0,00.0,000,-071,04.09
85,2026-04-09,06:00,007.00,000.0,000.0,00.0,000,-075,04.09
86,2026-04-09,06:15,007.00,000.0,000.0,00.0,000,-073,04.09
87,2026-04-09,06:30,007.00,000.0,000.0,00.0,000,-071,04.09
88,2026-04-09,06:45,007.00,000.0,000.0,00.0,000,-073,04.09
-----------------------

[FILE] SD: /001951_20260409.txt


[SCHED] Stack HWM: 11236 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 88,2026-04-09,06:45,007.00,000.0,000.0,00.0,000,-073,04.09
http_data format is stn_no=001951&rec_time=2026-04-09,06:45&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-073&bat_volt=04.09&bat_volt2=04.09

Payload is stn_no=001951&rec_time=2026-04-09,06:45&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-073&bat_volt=04.09&bat_volt2=04.09
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,06:45
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=06:45 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 06:46
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=6:46:8
 Sleep=14:7 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 07:00
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -73

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 40/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 89
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :88,2026-04-09,06:45,007.00,000.0,000.0,00.0,000,-073,04.09

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 6:45

NO GAPS FOUND ...


Current data inserted is 89,2026-04-09,07:00,007.00,000.0,000.0,00.0,000,-073,04.10


append_text->store_text : Used for internal status: 89,2026-04-09,07:00,007.00,000.0,000.0,00.0,000,-073,04.10


append_text written to lastrecorded_<stationname>.txt is : 89,2026-04-09,07:00,007.00,000.0,000.0,00.0,000,-073,04.10



[FILE] SPIFFS: /001951_20260409.txt | Size: 5400
   ... [Tail Content] ...
00,-071,04.09
85,2026-04-09,06:00,007.00,000.0,000.0,00.0,000,-075,04.09
86,2026-04-09,06:15,007.00,000.0,000.0,00.0,000,-073,04.09
87,2026-04-09,06:30,007.00,000.0,000.0,00.0,000,-071,04.09
88,2026-04-09,06:45,007.00,000.0,000.0,00.0,000,-073,04.09
89,2026-04-09,07:00,007.00,000.0,000.0,00.0,000,-073,04.10
-----------------------

[FILE] SD: /001951_20260409.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11236 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 89,2026-04-09,07:00,007.00,000.0,000.0,00.0,000,-073,04.10
http_data format is stn_no=001951&rec_time=2026-04-09,07:00&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-073&bat_volt=04.10&bat_volt2=04.10

Payload is stn_no=001951&rec_time=2026-04-09,07:00&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-073&bat_volt=04.10&bat_volt2=04.10
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,07:00
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[RTC] Time: 07:01
[FTP-Gate] unsent=0 cur_time=07:00 sched=NO cleanup=NO

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
[PWR] Sleep: CurTime=7:1:13
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 07:15
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
[GPRS] Modem ready in 9 iterations!
[GPRS] Polling for SIM (CPIN)...
[GPRS] SIM ready in 1000 ms!
************************
GETTING SIGNAL STRENGTH 
************************
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -69

************************
GETTING NETWORK 
************************
Final CCID parsed: 89914509006871527866
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] CREG(2G) but CEREG also registered. Preferring LTE.
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 40/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 90
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :89,2026-04-09,07:00,007.00,000.0,000.0,00.0,000,-073,04.10

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 7:0

NO GAPS FOUND ...


Current data inserted is 90,2026-04-09,07:15,007.00,000.0,000.0,00.0,000,-069,04.09


append_text->store_text : Used for internal status: 90,2026-04-09,07:15,007.00,000.0,000.0,00.0,000,-069,04.09


append_text written to lastrecorded_<stationname>.txt is : 90,2026-04-09,07:15,007.00,000.0,000.0,00.0,000,-069,04.09



[FILE] SPIFFS: /001951_20260409.txt | Size: 5460
   ... [Tail Content] ...
00,-075,04.09
86,2026-04-09,06:15,007.00,000.0,000.0,00.0,000,-073,04.09
87,2026-04-09,06:30,007.00,000.0,000.0,00.0,000,-071,04.09
88,2026-04-09,06:45,007.00,000.0,000.0,00.0,000,-073,04.09
89,2026-04-09,07:00,007.00,000.0,000.0,00.0,000,-073,04.10
90,2026-04-09,07:15,007.00,000.0,000.0,00.0,000,-069,04.09
-----------------------

[FILE] SD: /001951_20260409.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11396 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 90,2026-04-09,07:15,007.00,000.0,000.0,00.0,000,-069,04.09
http_data format is stn_no=001951&rec_time=2026-04-09,07:15&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.09&bat_volt2=04.09

Payload is stn_no=001951&rec_time=2026-04-09,07:15&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.09&bat_volt2=04.09
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,07:15
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=07:15 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 07:16
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=7:16:5
 Sleep=14:7 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 07:30
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -73

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 40/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 91
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :90,2026-04-09,07:15,007.00,000.0,000.0,00.0,000,-069,04.09

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 7:15

NO GAPS FOUND ...


Current data inserted is 91,2026-04-09,07:30,007.00,000.0,000.0,00.0,000,-073,04.10


append_text->store_text : Used for internal status: 91,2026-04-09,07:30,007.00,000.0,000.0,00.0,000,-073,04.10


append_text written to lastrecorded_<stationname>.txt is : 91,2026-04-09,07:30,007.00,000.0,000.0,00.0,000,-073,04.10



[FILE] SPIFFS: /001951_20260409.txt | Size: 5520
   ... [Tail Content] ...
00,-073,04.09
87,2026-04-09,06:30,007.00,000.0,000.0,00.0,000,-071,04.09
88,2026-04-09,06:45,007.00,000.0,000.0,00.0,000,-073,04.09
89,2026-04-09,07:00,007.00,000.0,000.0,00.0,000,-073,04.10
90,2026-04-09,07:15,007.00,000.0,000.0,00.0,000,-069,04.09
91,2026-04-09,07:30,007.00,000.0,000.0,00.0,000,-073,04.10
-----------------------

[FILE] SD: /001951_20260409.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11236 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 91,2026-04-09,07:30,007.00,000.0,000.0,00.0,000,-073,04.10
http_data format is stn_no=001951&rec_time=2026-04-09,07:30&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-073&bat_volt=04.10&bat_volt2=04.10

Payload is stn_no=001951&rec_time=2026-04-09,07:30&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-073&bat_volt=04.10&bat_volt2=04.10
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,07:30
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=07:30 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 07:31
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=7:31:8
 Sleep=14:6 (min:sec)
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
[GPS] Loaded from SPIFFS: 12.989772,77.538277 (Fix: 08/04/2026)
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.81 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.11V | Solar: 0.00V



[RTC] Time: 07:45
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: calib.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: gps_fix.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260409.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260409.txt
SPIFFS [INIT]: signature.txt
[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).

Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
[GPRS] Waiting for active UART...
[CALIB] Loaded: 2026-04-07 Status: 1
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
Signal strength IN gprs task is -71

************************
GETTING NETWORK 
************************
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

Smart APN: Match Found! APN: airteliot.com
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
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
SPIFFS: 40/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 92
RF Close date from RTC = 2026-04-09  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :91,2026-04-09,07:30,007.00,000.0,000.0,00.0,000,-073,04.10

Last Parse: CRF=7.00 T=0.00 H=0.00
Last recorded hour/min is 7:30

NO GAPS FOUND ...


Current data inserted is 92,2026-04-09,07:45,007.00,000.0,000.0,00.0,000,-071,04.09


append_text->store_text : Used for internal status: 92,2026-04-09,07:45,007.00,000.0,000.0,00.0,000,-071,04.09


append_text written to lastrecorded_<stationname>.txt is : 92,2026-04-09,07:45,007.00,000.0,000.0,00.0,000,-071,04.09



[FILE] SPIFFS: /001951_20260409.txt | Size: 5580
   ... [Tail Content] ...
00,-071,04.09
88,2026-04-09,06:45,007.00,000.0,000.0,00.0,000,-073,04.09
89,2026-04-09,07:00,007.00,000.0,000.0,00.0,000,-073,04.10
90,2026-04-09,07:15,007.00,000.0,000.0,00.0,000,-069,04.09
91,2026-04-09,07:30,007.00,000.0,000.0,00.0,000,-073,04.10
92,2026-04-09,07:45,007.00,000.0,000.0,00.0,000,-071,04.09
-----------------------

[FILE] SD: /001951_20260409.txt


[SCHED] Stack HWM: 11236 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260409.txt

Current Data to be sent is : 92,2026-04-09,07:45,007.00,000.0,000.0,00.0,000,-071,04.09
http_data format is stn_no=001951&rec_time=2026-04-09,07:45&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.09&bat_volt2=04.09

Payload is stn_no=001951&rec_time=2026-04-09,07:45&key=rfclimate5p13&rainfall=007.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.09&bat_volt2=04.09
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,200,9

[HTTP] Final Response Body snippet: PHP/5.4.16
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

**** Storing Last Logged Data as 2026-04-09,07:45
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[RTC] Time: 07:46
[FTP-Gate] unsent=0 cur_time=07:45 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=7:46:9
 Sleep=14:5 (min:sec)
[PWR] Entering Deep Sleep
