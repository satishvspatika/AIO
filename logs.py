[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Fresh Start (Reason 1). Initializing RTC variables.
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup was not caused by deep sleep: 0
[BOOT] Reset Reason: POWERON_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 1
[ULP] Hard Power-On. Wiping ULP counters.
ULP Program loaded and started.
[BOOT] ULP Anchors Synced: Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.05V | Solar: 0.00V



[RTC] Time: 22:59
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.94
ULP Counting Enabled (attachInterrupt 27 Disabled).

[SCHED] Seeded 9 ULP wind pulses from direct ULP read.
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
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -63

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] New SIM or No Cache. Performing full discovery...
CSPN Logic response: 
+CSPN: "BSNL MOBILE",1

OK

COPS Logic response: 
+COPS: 0

OK

IMSI: 404719200765848
Service Provider APN is bsnlnet
Carrier: BSNL
Number: 404719200765848

BSNL network found..

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=2 (LastSucc:2 SlotsOn2G:0)
Reg Search [BSNL:1]... Status:0 Iter:#1/24
[GPRS] CGREG registered during adaptive wait!
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 31.37 C
Humidity    : 43.93 %
Wind Speed  : 0.00 m/s
Wind Dir    : 181 deg
Wind Pulses : 9.00
----------------------------
--- Storage Status ---
SPIFFS: 528/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
Fresh boot detected. Skipping primary upload for sensor stabilization (queueing to Backlog).
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 56
RF Close date from RTC = 2026-04-08  
[SCHED] 🗓 Day Change Detected. Performing Rollover...
[SCHED] First rollover after boot/flash. Skipping destructive reset to preserve SPIFFS recovery.
[GoldenData] Starting Sent Mask Reconstruction from SPIFFS...
[GoldenData] Reconstruction Complete (Sent-Accurate).

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :56,2026-04-07,22:45,031.4,043.4,00.9,210,-111,04.05

Last sample No stored : 56
Duplicate sample detected (TWS). Data already logged for this interval.
Skipped data writing. Checking if GPRS needs to send unsent data then Sleep.
[SCHED] Between intervals. Modem will remain OFF.
[SCHED] Stack HWM: 11104 bytes free
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[RTC] Time: 23:00

[SCHED] Seeded 3 ULP wind pulses from direct ULP read.
Scheduler: RTC Sync acquired.
Scheduler: Waiting for GPRS task...
Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 31.37 C
Humidity    : 43.93 %
Wind Speed  : 0.03 m/s
Wind Dir    : 184 deg
Wind Pulses : 235.00
----------------------------
--- Storage Status ---
SPIFFS: 528/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 57
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :56,2026-04-07,22:45,031.4,043.4,00.9,210,-111,04.05

Last sample No stored : 56
Last recorded hour/min is 22:45

NO GAPS FOUND ...


Current data inserted is 57,2026-04-07,23:00,031.4,043.9,00.0,184,-063,04.01


append_text->store_text : Used for internal status: 57,2026-04-07,23:00,031.4,043.9,00.0,184,-063,04.01


append_text written to lastrecorded_<stationname>.txt is : 57,2026-04-07,23:00,031.4,043.9,00.0,184,-063,04.01



[FILE] SPIFFS: /001941_20260408.txt | Size: 3074
   ... [Tail Content] ...
0.9,196,-111,04.05
53,2026-04-07,22:00,031.4,042.5,00.8,185,-111,04.05
54,2026-04-07,22:15,031.4,042.8,00.8,185,-111,04.05
55,2026-04-07,22:30,031.4,042.6,01.0,185,-111,04.05
56,2026-04-07,22:45,031.4,043.4,00.9,210,-111,04.05
57,2026-04-07,23:00,031.4,043.9,00.0,184,-063,04.01
-----------------------

[FILE] SD: /001941_20260408.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
4-07,20:45;031.4;042.0;01.0;185;-111;04.04
001941;2026-04-07,21:00;031.4;042.0;00.0;190;-111;04.06
001941;2026-04-07,21:15;031.4;042.2;00.9;186;-111;04.05
001941;2026-04-07,21:30;031.4;042.0;01.0;203;-111;04.05
001941;2026-04-07,21:45;031.4;042.2;00.9;196;-111;04.05
001941;2026-04-07,22:00;031.4;042.5;00.8;185;-111;04.05
001941;2026-04-07,22:15;031.4;042.8;00.8;185;-111;04.05
001941;2026-04-07,22:30;031.4;042.6;01.0;185;-111;04.05
001941;2026-04-07,22:45;031.4;043.4;00.9;210;-111;04.05
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11104 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[Health] CDM window entirely missed today. Flagging FAIL.
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 56,2026-04-07,22:45,031.4,043.4,00.9,210,-111,04.05
http_data format is stn_no=001941&rec_time=2026-04-07,23:00&temp=031.4&humid=043.4&w_speed=00.9&w_dir=210&signal=-063&key=climate4p2013&bat_volt=04.05&bat_volt2=04.05

Payload is stn_no=001941&rec_time=2026-04-07,23:00&temp=031.4&humid=043.4&w_speed=00.9&w_dir=210&signal=-063&key=climate4p2013&bat_volt=04.05&bat_volt2=04.05
[HTTP] Using Fast v3.0 Handshake...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,713,0

HTTP Zombie Error (713). Count: 1/3. Clean stack requested.
[HTTP] 1st Attempt (Fast) failed. Retrying in 2s (Fast Attempt 2)...
Payload is stn_no=001941&rec_time=2026-04-07,23:00&temp=031.4&humid=043.4&w_speed=00.9&w_dir=210&signal=-063&key=climate4p2013&bat_volt=04.05&bat_volt2=04.05
[HTTP] Using Fast v3.0 Handshake...
[RTC] Time: 23:01
[HTTP] HTTPACTION timed out — no URC received from modem.
[HTTP] 2nd Attempt (Fast) also failed. Falling back to Robust method...
Payload is stn_no=001941&rec_time=2026-04-07,23:00&temp=031.4&humid=043.4&w_speed=00.9&w_dir=210&signal=-063&key=climate4p2013&bat_volt=04.05&bat_volt2=04.05
[HTTP] Using Robust Handshake (Wait for DOWNLOAD)...
[HTTP] AT+HTTPDATA failed (Missing DOWNLOAD).
[HTTP] Attempt 1 failed. Re-initialising stack for single retry...
[HTTP] Retry attempt...
Payload is stn_no=001941&rec_time=2026-04-07,23:00&temp=031.4&humid=043.4&w_speed=00.9&w_dir=210&signal=-063&key=climate4p2013&bat_volt=04.05&bat_volt2=04.05
[HTTP] Using Robust Handshake (Wait for DOWNLOAD)...
[HTTP] Response of AT+HTTPACTION=1 is: +HTTPACTION: 1,713,0

HTTP Zombie Error (713). Count: 2/3. Clean stack requested.
[DNS] Cleared fallback cache due to TCP zombie failure code.
[HTTP] Monthly cum fail counter reset (New Month detected).
[RECOVERY] Consec HTTP Fails: 1 | Present: 1 | CumMth: 1
CURRENT DATA : Retries exceeded limit... Storing to backlog.

ftpappend_text is : 001941;2026-04-07,23:00;031.4;043.4;00.9;210;-063;04.05

Record written into /ftpunsent.txt is as below : 
001941;2026-04-07,23:00;031.4;043.4;00.9;210;-063;04.05

*** Current data couldn't be sent. Backlog will be handled independently.
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=33 cur_time=23:01 sched=NO cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[FTP] Backlog Push: 33 records found. Mode: THRESHOLD_MET

FTP file name is /TWS_001941_260408_230000.kwd

SampleNo  is 57

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Huge Backlog! Records chunked for TX. Original queue protected until success.
Retrieved file is /TWS_001941_260408_230000.kwd
Initializing A7672S for FTP...
[FTP] BSNL/Other detected. Smart Default: Active (0).
[RTC] Time: 23:02
[FTP] Configuring mode (0)...
1
[FTP] Warming up DNS...
[FTP] DNS Failed. Switching to Insurance IP.
[FTP] Login Result: 13
FTP Login unsuccessful
[FTP] First Login attempt failed. BSNL Shield: Forcing Context Refresh...
[RTC] Time: 23:03
[FTP] Configuring mode (0)...
0
[FTP] Warming up DNS...
[FTP] DNS Failed. Switching to Insurance IP.
[RTC] Time: 23:04
FTP Login unsuccessful

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
[PWR] Sleep: CurTime=23:6:28
 Sleep=8:45 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_du}⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.04V | Solar: 0.00V



[RTC] Time: 23:15
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: TWS_001941_260408_230000.kwd
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.03
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
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is 85
[GPRS] Dead signal zone detected. Skipping long-poll wait.

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:1)
[RTC] Time: 23:16
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 31.33 C
Humidity    : 43.43 %
Wind Speed  : 0.18 m/s
Wind Dir    : 182 deg
Wind Pulses : 1460.00
----------------------------
--- Storage Status ---
SPIFFS: 529/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 58
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :57,2026-04-07,23:00,031.4,043.9,00.0,184,-063,04.01

Last sample No stored : 57
Last recorded hour/min is 23:0

NO GAPS FOUND ...


Current data inserted is 58,2026-04-07,23:15,031.3,043.4,00.2,182,-111,04.04


append_text->store_text : Used for internal status: 58,2026-04-07,23:15,031.3,043.4,00.2,182,-111,04.04


append_text written to lastrecorded_<stationname>.txt is : 58,2026-04-07,23:15,031.3,043.4,00.2,182,-111,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 3127
   ... [Tail Content] ...
0.8,185,-111,04.05
54,2026-04-07,22:15,031.4,042.8,00.8,185,-111,04.05
55,2026-04-07,22:30,031.4,042.6,01.0,185,-111,04.05
56,2026-04-07,22:45,031.4,043.4,00.9,210,-111,04.05
57,2026-04-07,23:00,031.4,043.9,00.0,184,-063,04.01
58,2026-04-07,23:15,031.3,043.4,00.2,182,-111,04.04
-----------------------

[FILE] SD: /001941_20260408.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
4-07,21:00;031.4;042.0;00.0;190;-111;04.06
001941;2026-04-07,21:15;031.4;042.2;00.9;186;-111;04.05
001941;2026-04-07,21:30;031.4;042.0;01.0;203;-111;04.05
001941;2026-04-07,21:45;031.4;042.2;00.9;196;-111;04.05
001941;2026-04-07,22:00;031.4;042.5;00.8;185;-111;04.05
001941;2026-04-07,22:15;031.4;042.8;00.8;185;-111;04.05
001941;2026-04-07,22:30;031.4;042.6;01.0;185;-111;04.05
001941;2026-04-07,22:45;031.4;043.4;00.9;210;-111;04.05
001941;2026-04-07,23:00;031.4;043.4;00.9;210;-063;04.05
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11424 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 57,2026-04-07,23:00,031.4,043.9,00.0,184,-063,04.01
http_data format is stn_no=001941&rec_time=2026-04-07,23:15&temp=031.4&humid=043.9&w_speed=00.0&w_dir=184&signal=-111&key=climate4p2013&bat_volt=04.01&bat_volt2=04.01

Payload is stn_no=001941&rec_time=2026-04-07,23:15&temp=031.4&humid=043.9&w_speed=00.0&w_dir=184&signal=-111&key=climate4p2013&bat_volt=04.01&bat_volt2=04.01
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
[FS] Saved APN Config: bsnlnet
[PWR] Signature persisted successfully.

**** Storing Last Logged Data as 2026-04-07,23:15
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=23:16:59
 Sleep=13:16 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮_flash: No core dump partition found!
E (48) esp_core_dump_flash: No core dump partition found!


[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.05V | Solar: 0.00V



[RTC] Time: 23:30
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: TWS_001941_260408_230000.kwd
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.18
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
Signal strength IN gprs task is -63

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] Pre-setting APN for CID 1: bsnlnet (IP)
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:2)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 31.29 C
Humidity    : 43.93 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 529/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 59
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :58,2026-04-07,23:15,031.3,043.4,00.2,182,-111,04.04

Last sample No stored : 58
Last recorded hour/min is 23:15

NO GAPS FOUND ...


Current data inserted is 59,2026-04-07,23:30,031.3,043.9,00.0,182,-063,04.05


append_text->store_text : Used for internal status: 59,2026-04-07,23:30,031.3,043.9,00.0,182,-063,04.05


append_text written to lastrecorded_<stationname>.txt is : 59,2026-04-07,23:30,031.3,043.9,00.0,182,-063,04.05



[FILE] SPIFFS: /001941_20260408.txt | Size: 3180
   ... [Tail Content] ...
0.8,185,-111,04.05
55,2026-04-07,22:30,031.4,042.6,01.0,185,-111,04.05
56,2026-04-07,22:45,031.4,043.4,00.9,210,-111,04.05
57,2026-04-07,23:00,031.4,043.9,00.0,184,-063,04.01
58,2026-04-07,23:15,031.3,043.4,00.2,182,-111,04.04
59,2026-04-07,23:30,031.3,043.9,00.0,182,-063,04.05
-----------------------

[FILE] SD: /001941_20260408.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
4-07,21:00;031.4;042.0;00.0;190;-111;04.06
001941;2026-04-07,21:15;031.4;042.2;00.9;186;-111;04.05
001941;2026-04-07,21:30;031.4;042.0;01.0;203;-111;04.05
001941;2026-04-07,21:45;031.4;042.2;00.9;196;-111;04.05
001941;2026-04-07,22:00;031.4;042.5;00.8;185;-111;04.05
001941;2026-04-07,22:15;031.4;042.8;00.8;185;-111;04.05
001941;2026-04-07,22:30;031.4;042.6;01.0;185;-111;04.05
001941;2026-04-07,22:45;031.4;043.4;00.9;210;-111;04.05
001941;2026-04-07,23:00;031.4;043.4;00.9;210;-063;04.05
-----------------------
--- UNSENT DATA END ---



[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11252 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).
[RTC] Time: 23:31

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 58,2026-04-07,23:15,031.3,043.4,00.2,182,-111,04.04
http_data format is stn_no=001941&rec_time=2026-04-07,23:30&temp=031.3&humid=043.4&w_speed=00.2&w_dir=182&signal=-063&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-07,23:30&temp=031.3&humid=043.4&w_speed=00.2&w_dir=182&signal=-063&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
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

**** Storing Last Logged Data as 2026-04-07,23:30
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=33 cur_time=23:31 sched=NO cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[FTP] Backlog Push: 33 records found. Mode: THRESHOLD_MET

FTP file name is /TWS_001941_260408_233000.kwd

SampleNo  is 59

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Huge Backlog! Records chunked for TX. Original queue protected until success.
Retrieved file is /TWS_001941_260408_233000.kwd
Initializing A7672S for FTP...
[FTP] Purged orphaned staging file: /TWS_001941_260408_230000.kwd
[FTP] BSNL/Other detected. Smart Default: Active (0).
[FTP] Configuring mode (0)...
1
[FTP] Warming up DNS...
[FTP] DNS Resolved: 27.34.245.70
[FTP] Login Result: 0
FTP Login success
[FTP] File size: 855
[FTP] FSDEL path: TWS_001941_260408_233000.kwd
[FTP] FSOPEN cmd: AT+FSOPEN="C:/TWS_001941_260408_233000.kwd",0
[FTP] Found FH: 1
[FTP] Sending FSWRITE command...
[FTP] Data stream finished.
[FTP] Dispatching PUTFILE command...
[RTC] Time: 23:32
[FTP] Dispatching PUTFILE command...
Response of AT+CFTPSPUTFILE: 
OK

+CFTPSPUTFILE: 0

[GoldenData] Marked records from /TWS_001941_260408_233000.kwd as DELIVERED.
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=23:32:17
 Sleep=12:55 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.05V | Solar: 0.00V



[RTC] Time: 23:45
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
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
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -61

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] Pre-setting APN for CID 1: bsnlnet (IP)
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:3)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 31.24 C
Humidity    : 44.22 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 527/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 60
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :59,2026-04-07,23:30,031.3,043.9,00.0,182,-063,04.05

Last sample No stored : 59
Last recorded hour/min is 23:30

NO GAPS FOUND ...


Current data inserted is 60,2026-04-07,23:45,031.2,044.2,00.0,182,-061,04.04


append_text->store_text : Used for internal status: 60,2026-04-07,23:45,031.2,044.2,00.0,182,-061,04.04


append_text written to lastrecorded_<stationname>.txt is : 60,2026-04-07,23:45,031.2,044.2,00.0,182,-061,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 3233
   ... [Tail Content] ...
1.0,185,-111,04.05
56,2026-04-07,22:45,031.4,043.4,00.9,210,-111,04.05
57,2026-04-07,23:00,031.4,043.9,00.0,184,-063,04.01
58,2026-04-07,23:15,031.3,043.4,00.2,182,-111,04.04
59,2026-04-07,23:30,031.3,043.9,00.0,182,-063,04.05
60,2026-04-07,23:45,031.2,044.2,00.0,182,-061,04.04
-----------------------

[FILE] SD: /001941_20260408.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
4-07,21:00;031.4;042.0;00.0;190;-111;04.06
001941;2026-04-07,21:15;031.4;042.2;00.9;186;-111;04.05
001941;2026-04-07,21:30;031.4;042.0;01.0;203;-111;04.05
001941;2026-04-07,21:45;031.4;042.2;00.9;196;-111;04.05
001941;2026-04-07,22:00;031.4;042.5;00.8;185;-111;04.05
001941;2026-04-07,22:15;031.4;042.8;00.8;185;-111;04.05
001941;2026-04-07,22:30;031.4;042.6;01.0;185;-111;04.05
001941;2026-04-07,22:45;031.4;043.4;00.9;210;-111;04.05
001941;2026-04-07,23:00;031.4;043.4;00.9;210;-063;04.05
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11412 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 59,2026-04-07,23:30,031.3,043.9,00.0,182,-063,04.05
http_data format is stn_no=001941&rec_time=2026-04-07,23:45&temp=031.3&humid=043.9&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.05&bat_volt2=04.05

Payload is stn_no=001941&rec_time=2026-04-07,23:45&temp=031.3&humid=043.9&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.05&bat_volt2=04.05
[HTTP] Using Fast v3.0 Handshake...
[RTC] Time: 23:46
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

**** Storing Last Logged Data as 2026-04-07,23:45
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=18 cur_time=23:46 sched=NO cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[FTP] Backlog Push: 18 records found. Mode: THRESHOLD_MET

FTP file name is /TWS_001941_260408_234500.kwd

SampleNo  is 60

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Huge Backlog! Records chunked for TX. Original queue protected until success.
Retrieved file is /TWS_001941_260408_234500.kwd
Initializing A7672S for FTP...
[FTP] Using SAVED Smart Mode: Active
[FTP] Configuring mode (0)...
1
[FTP] Warming up DNS...
[FTP] DNS Resolved: 27.34.245.70
[FTP] Login Result: 0
FTP Login success
[FTP] File size: 855
[FTP] FSDEL path: TWS_001941_260408_234500.kwd
[FTP] FSOPEN cmd: AT+FSOPEN="C:/TWS_001941_260408_234500.kwd",0
[FTP] Found FH: 1
[FTP] Sending FSWRITE command...
[FTP] Data stream finished.
[FTP] Dispatching PUTFILE command...
[RTC] Time: 23:47
[FTP] Dispatching PUTFILE command...
Response of AT+CFTPSPUTFILE: 
OK

+CFTPSPUTFILE: 0

[GoldenData] Marked records from /TWS_001941_260408_234500.kwd as DELIVERED.
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=23:47:16
 Sleep=12:57 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_du=⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.05V | Solar: 0.00V



[RTC] Time: 00:00
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
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
Signal strength IN gprs task is -61

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] Pre-setting APN for CID 1: bsnlnet (IP)
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:4)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 31.21 C
Humidity    : 44.42 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 527/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 61
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :60,2026-04-07,23:45,031.2,044.2,00.0,182,-061,04.04

Last sample No stored : 60
Last recorded hour/min is 23:45

NO GAPS FOUND ...


Current data inserted is 61,2026-04-08,00:00,031.2,044.4,00.0,182,-061,04.05


append_text->store_text : Used for internal status: 61,2026-04-08,00:00,031.2,044.4,00.0,182,-061,04.05


append_text written to lastrecorded_<stationname>.txt is : 61,2026-04-08,00:00,031.2,044.4,00.0,182,-061,04.05



[FILE] SPIFFS: /001941_20260408.txt | Size: 3286
   ... [Tail Content] ...
0.9,210,-111,04.05
57,2026-04-07,23:00,031.4,043.9,00.0,184,-063,04.01
58,2026-04-07,23:15,031.3,043.4,00.2,182,-111,04.04
59,2026-04-07,23:30,031.3,043.9,00.0,182,-063,04.05
60,2026-04-07,23:45,031.2,044.2,00.0,182,-061,04.04
61,2026-04-08,00:00,031.2,044.4,00.0,182,-061,04.05
-----------------------

[FILE] SD: /001941_20260408.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
001941;2026-04-07,22:30;031.4;042.6;01.0;185;-111;04.05
001941;2026-04-07,22:45;031.4;043.4;00.9;210;-111;04.05
001941;2026-04-07,23:00;031.4;043.4;00.9;210;-063;04.05
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11424 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).
[RTC] Time: 00:01

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 60,2026-04-07,23:45,031.2,044.2,00.0,182,-061,04.04
http_data format is stn_no=001941&rec_time=2026-04-07,00:00&temp=031.2&humid=044.2&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-07,00:00&temp=031.2&humid=044.2&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
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

**** Storing Last Logged Data as 2026-04-08,00:00
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=3 cur_time=00:01 sched=YES cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[FTP] Backlog Push: 3 records found. Mode: SCHEDULED_SLOT

FTP file name is /TWS_001941_260408_000000.kwd

SampleNo  is 61

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Backlog fully chunked into Active Payload.
Retrieved file is /TWS_001941_260408_000000.kwd
Initializing A7672S for FTP...
[FTP] Using SAVED Smart Mode: Active
[FTP] Configuring mode (0)...
1
[FTP] Warming up DNS...
[FTP] DNS Resolved: 27.34.245.70
[FTP] Login Result: 0
FTP Login success
[FTP] File size: 171
[FTP] FSDEL path: TWS_001941_260408_000000.kwd
[FTP] FSOPEN cmd: AT+FSOPEN="C:/TWS_001941_260408_000000.kwd",0
[FTP] Found FH: 1
[FTP] Sending FSWRITE command...
[FTP] Data stream finished.
[FTP] Dispatching PUTFILE command...
[RTC] Time: 00:02
[FTP] Dispatching PUTFILE command...
Response of AT+CFTPSPUTFILE: 
OK

+CFTPSPUTFILE: 0

[GoldenData] Marked records from /TWS_001941_260408_000000.kwd as DELIVERED.

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
[PWR] Sleep: CurTime=0:2:17
 Sleep=12:55 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.05V | Solar: 0.00V



[RTC] Time: 00:15
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
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
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -59

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] Pre-setting APN for CID 1: bsnlnet (IP)
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:5)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 31.18 C
Humidity    : 44.81 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 527/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 62
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :61,2026-04-08,00:00,031.2,044.4,00.0,182,-061,04.05

Last sample No stored : 61
Last recorded hour/min is 0:0

NO GAPS FOUND ...


Current data inserted is 62,2026-04-08,00:15,031.2,044.8,00.0,182,-059,04.05


append_text->store_text : Used for internal status: 62,2026-04-08,00:15,031.2,044.8,00.0,182,-059,04.05


append_text written to lastrecorded_<stationname>.txt is : 62,2026-04-08,00:15,031.2,044.8,00.0,182,-059,04.05



[FILE] SPIFFS: /001941_20260408.txt | Size: 3339
   ... [Tail Content] ...
0.0,184,-063,04.01
58,2026-04-07,23:15,031.3,043.4,00.2,182,-111,04.04
59,2026-04-07,23:30,031.3,043.9,00.0,182,-063,04.05
60,2026-04-07,23:45,031.2,044.2,00.0,182,-061,04.04
61,2026-04-08,00:00,031.2,044.4,00.0,182,-061,04.05
62,2026-04-08,00:15,031.2,044.8,00.0,182,-059,04.05
-----------------------

[FILE] SD: /001941_20260408.txt


[SCHED] Stack HWM: 11424 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 61,2026-04-08,00:00,031.2,044.4,00.0,182,-061,04.05
http_data format is stn_no=001941&rec_time=2026-04-08,00:15&temp=031.2&humid=044.4&w_speed=00.0&w_dir=182&signal=-059&key=climate4p2013&bat_volt=04.05&bat_volt2=04.05

Payload is stn_no=001941&rec_time=2026-04-08,00:15&temp=031.2&humid=044.4&w_speed=00.0&w_dir=182&signal=-059&key=climate4p2013&bat_volt=04.05&bat_volt2=04.05
[HTTP] Using Fast v3.0 Handshake...
[RTC] Time: 00:16
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

**** Storing Last Logged Data as 2026-04-08,00:15
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=00:16 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=0:16:20
 Sleep=13:53 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.05V | Solar: 0.00V



[RTC] Time: 00:30
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
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
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -61

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] Pre-setting APN for CID 1: bsnlnet (IP)
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:6)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 31.17 C
Humidity    : 45.50 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 527/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 63
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :62,2026-04-08,00:15,031.2,044.8,00.0,182,-059,04.05

Last sample No stored : 62
Last recorded hour/min is 0:15

NO GAPS FOUND ...


Current data inserted is 63,2026-04-08,00:30,031.2,045.5,00.0,182,-061,04.04


append_text->store_text : Used for internal status: 63,2026-04-08,00:30,031.2,045.5,00.0,182,-061,04.04


append_text written to lastrecorded_<stationname>.txt is : 63,2026-04-08,00:30,031.2,045.5,00.0,182,-061,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 3392
   ... [Tail Content] ...
0.2,182,-111,04.04
59,2026-04-07,23:30,031.3,043.9,00.0,182,-063,04.05
60,2026-04-07,23:45,031.2,044.2,00.0,182,-061,04.04
61,2026-04-08,00:00,031.2,044.4,00.0,182,-061,04.05
62,2026-04-08,00:15,031.2,044.8,00.0,182,-059,04.05
63,2026-04-08,00:30,031.2,045.5,00.0,182,-061,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[SCHED] Stack HWM: 11424 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 62,2026-04-08,00:15,031.2,044.8,00.0,182,-059,04.05
http_data format is stn_no=001941&rec_time=2026-04-08,00:30&temp=031.2&humid=044.8&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.05&bat_volt2=04.05

Payload is stn_no=001941&rec_time=2026-04-08,00:30&temp=031.2&humid=044.8&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.05&bat_volt2=04.05
[HTTP] Using Fast v3.0 Handshake...
[RTC] Time: 00:31
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

**** Storing Last Logged Data as 2026-04-08,00:30
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=00:31 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=0:31:20
 Sleep=13:54 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_du}⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.05V | Solar: 0.00V



[RTC] Time: 00:45
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
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
Signal strength IN gprs task is -65

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] Pre-setting APN for CID 1: bsnlnet (IP)
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:7)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 31.15 C
Humidity    : 46.21 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 527/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 64
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :63,2026-04-08,00:30,031.2,045.5,00.0,182,-061,04.04

Last sample No stored : 63
Last recorded hour/min is 0:30

NO GAPS FOUND ...


Current data inserted is 64,2026-04-08,00:45,031.1,046.2,00.0,182,-065,04.05


append_text->store_text : Used for internal status: 64,2026-04-08,00:45,031.1,046.2,00.0,182,-065,04.05


append_text written to lastrecorded_<stationname>.txt is : 64,2026-04-08,00:45,031.1,046.2,00.0,182,-065,04.05



[FILE] SPIFFS: /001941_20260408.txt | Size: 3445
   ... [Tail Content] ...
0.0,182,-063,04.05
60,2026-04-07,23:45,031.2,044.2,00.0,182,-061,04.04
61,2026-04-08,00:00,031.2,044.4,00.0,182,-061,04.05
62,2026-04-08,00:15,031.2,044.8,00.0,182,-059,04.05
63,2026-04-08,00:30,031.2,045.5,00.0,182,-061,04.04
64,2026-04-08,00:45,031.1,046.2,00.0,182,-065,04.05
-----------------------

[FILE] SD: /001941_20260408.txt


[SCHED] Stack HWM: 11252 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).
[RTC] Time: 00:46

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 63,2026-04-08,00:30,031.2,045.5,00.0,182,-061,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,00:45&temp=031.2&humid=045.5&w_speed=00.0&w_dir=182&signal=-065&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,00:45&temp=031.2&humid=045.5&w_speed=00.0&w_dir=182&signal=-065&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
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

**** Storing Last Logged Data as 2026-04-08,00:45
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=00:46 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=0:46:18
 Sleep=13:55 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.05V | Solar: 0.00V



[RTC] Time: 01:00
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
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
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -55

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] Pre-setting APN for CID 1: bsnlnet (IP)
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:8)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 31.13 C
Humidity    : 46.80 %
Wind Speed  : 0.00 m/s
Wind Dir    : 183 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 527/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 65
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :64,2026-04-08,00:45,031.1,046.2,00.0,182,-065,04.05

Last sample No stored : 64
Last recorded hour/min is 0:45

NO GAPS FOUND ...


Current data inserted is 65,2026-04-08,01:00,031.1,046.8,00.0,183,-055,04.05


append_text->store_text : Used for internal status: 65,2026-04-08,01:00,031.1,046.8,00.0,183,-055,04.05


append_text written to lastrecorded_<stationname>.txt is : 65,2026-04-08,01:00,031.1,046.8,00.0,183,-055,04.05



[FILE] SPIFFS: /001941_20260408.txt | Size: 3498
   ... [Tail Content] ...
0.0,182,-061,04.04
61,2026-04-08,00:00,031.2,044.4,00.0,182,-061,04.05
62,2026-04-08,00:15,031.2,044.8,00.0,182,-059,04.05
63,2026-04-08,00:30,031.2,045.5,00.0,182,-061,04.04
64,2026-04-08,00:45,031.1,046.2,00.0,182,-065,04.05
65,2026-04-08,01:00,031.1,046.8,00.0,183,-055,04.05
-----------------------

[FILE] SD: /001941_20260408.txt


[SCHED] Stack HWM: 11252 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 64,2026-04-08,00:45,031.1,046.2,00.0,182,-065,04.05
http_data format is stn_no=001941&rec_time=2026-04-08,01:00&temp=031.1&humid=046.2&w_speed=00.0&w_dir=182&signal=-055&key=climate4p2013&bat_volt=04.05&bat_volt2=04.05

Payload is stn_no=001941&rec_time=2026-04-08,01:00&temp=031.1&humid=046.2&w_speed=00.0&w_dir=182&signal=-055&key=climate4p2013&bat_volt=04.05&bat_volt2=04.05
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
[RTC] Time: 01:01
[PWR] Signature persisted successfully.

**** Storing Last Logged Data as 2026-04-08,01:00
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=01:01 sched=NO cleanup=NO

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
[PWR] Sleep: CurTime=1:1:17
 Sleep=13:55 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮ }⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.05V | Solar: 0.00V



[RTC] Time: 01:15
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
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
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -61

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:9)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 31.13 C
Humidity    : 47.29 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 527/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 66
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :65,2026-04-08,01:00,031.1,046.8,00.0,183,-055,04.05

Last sample No stored : 65
Last recorded hour/min is 1:0

NO GAPS FOUND ...


Current data inserted is 66,2026-04-08,01:15,031.1,047.3,00.0,182,-061,04.04


append_text->store_text : Used for internal status: 66,2026-04-08,01:15,031.1,047.3,00.0,182,-061,04.04


append_text written to lastrecorded_<stationname>.txt is : 66,2026-04-08,01:15,031.1,047.3,00.0,182,-061,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 3551
   ... [Tail Content] ...
0.0,182,-061,04.05
62,2026-04-08,00:15,031.2,044.8,00.0,182,-059,04.05
63,2026-04-08,00:30,031.2,045.5,00.0,182,-061,04.04
64,2026-04-08,00:45,031.1,046.2,00.0,182,-065,04.05
65,2026-04-08,01:00,031.1,046.8,00.0,183,-055,04.05
66,2026-04-08,01:15,031.1,047.3,00.0,182,-061,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11252 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 65,2026-04-08,01:00,031.1,046.8,00.0,183,-055,04.05
http_data format is stn_no=001941&rec_time=2026-04-08,01:15&temp=031.1&humid=046.8&w_speed=00.0&w_dir=183&signal=-061&key=climate4p2013&bat_volt=04.05&bat_volt2=04.05

Payload is stn_no=001941&rec_time=2026-04-08,01:15&temp=031.1&humid=046.8&w_speed=00.0&w_dir=183&signal=-061&key=climate4p2013&bat_volt=04.05&bat_volt2=04.05
[HTTP] Using Fast v3.0 Handshake...
[RTC] Time: 01:16
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

**** Storing Last Logged Data as 2026-04-08,01:15
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=01:16 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=1:16:20
 Sleep=13:54 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_du=⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.05V | Solar: 0.00V



[RTC] Time: 01:30
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
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
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -61

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:10)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 31.09 C
Humidity    : 47.69 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 527/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 67
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :66,2026-04-08,01:15,031.1,047.3,00.0,182,-061,04.04

Last sample No stored : 66
Last recorded hour/min is 1:15

NO GAPS FOUND ...


Current data inserted is 67,2026-04-08,01:30,031.1,047.7,00.0,182,-061,04.04


append_text->store_text : Used for internal status: 67,2026-04-08,01:30,031.1,047.7,00.0,182,-061,04.04


append_text written to lastrecorded_<stationname>.txt is : 67,2026-04-08,01:30,031.1,047.7,00.0,182,-061,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 3604
   ... [Tail Content] ...
0.0,182,-059,04.05
63,2026-04-08,00:30,031.2,045.5,00.0,182,-061,04.04
64,2026-04-08,00:45,031.1,046.2,00.0,182,-065,04.05
65,2026-04-08,01:00,031.1,046.8,00.0,183,-055,04.05
66,2026-04-08,01:15,031.1,047.3,00.0,182,-061,04.04
67,2026-04-08,01:30,031.1,047.7,00.0,182,-061,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11252 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 66,2026-04-08,01:15,031.1,047.3,00.0,182,-061,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,01:30&temp=031.1&humid=047.3&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,01:30&temp=031.1&humid=047.3&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
[HTTP] Using Fast v3.0 Handshake...
[RTC] Time: 01:31
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

**** Storing Last Logged Data as 2026-04-08,01:30
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=01:31 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=1:31:20
 Sleep=13:55 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.04V | Solar: 0.00V



[RTC] Time: 01:45
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
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
Signal strength IN gprs task is -61

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:11)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 31.06 C
Humidity    : 48.18 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 527/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 68
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :67,2026-04-08,01:30,031.1,047.7,00.0,182,-061,04.04

Last sample No stored : 67
Last recorded hour/min is 1:30

NO GAPS FOUND ...


Current data inserted is 68,2026-04-08,01:45,031.1,048.2,00.0,182,-061,04.04


append_text->store_text : Used for internal status: 68,2026-04-08,01:45,031.1,048.2,00.0,182,-061,04.04


append_text written to lastrecorded_<stationname>.txt is : 68,2026-04-08,01:45,031.1,048.2,00.0,182,-061,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 3657
   ... [Tail Content] ...
0.0,182,-061,04.04
64,2026-04-08,00:45,031.1,046.2,00.0,182,-065,04.05
65,2026-04-08,01:00,031.1,046.8,00.0,183,-055,04.05
66,2026-04-08,01:15,031.1,047.3,00.0,182,-061,04.04
67,2026-04-08,01:30,031.1,047.7,00.0,182,-061,04.04
68,2026-04-08,01:45,031.1,048.2,00.0,182,-061,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[SCHED] Stack HWM: 11424 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).
[RTC] Time: 01:46

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 67,2026-04-08,01:30,031.1,047.7,00.0,182,-061,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,01:45&temp=031.1&humid=047.7&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,01:45&temp=031.1&humid=047.7&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
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

**** Storing Last Logged Data as 2026-04-08,01:45
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=01:46 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=1:46:21
 Sleep=13:54 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_du=⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.05V | Solar: 0.00V



[RTC] Time: 02:00
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
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
Signal strength IN gprs task is -59

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] Pre-setting APN for CID 1: bsnlnet (IP)
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:12)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 31.03 C
Humidity    : 48.77 %
Wind Speed  : 0.00 m/s
Wind Dir    : 183 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 527/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 69
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :68,2026-04-08,01:45,031.1,048.2,00.0,182,-061,04.04

Last sample No stored : 68
Last recorded hour/min is 1:45

NO GAPS FOUND ...


Current data inserted is 69,2026-04-08,02:00,031.0,048.8,00.0,183,-059,04.04


append_text->store_text : Used for internal status: 69,2026-04-08,02:00,031.0,048.8,00.0,183,-059,04.04


append_text written to lastrecorded_<stationname>.txt is : 69,2026-04-08,02:00,031.0,048.8,00.0,183,-059,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 3710
   ... [Tail Content] ...
0.0,182,-065,04.05
65,2026-04-08,01:00,031.1,046.8,00.0,183,-055,04.05
66,2026-04-08,01:15,031.1,047.3,00.0,182,-061,04.04
67,2026-04-08,01:30,031.1,047.7,00.0,182,-061,04.04
68,2026-04-08,01:45,031.1,048.2,00.0,182,-061,04.04
69,2026-04-08,02:00,031.0,048.8,00.0,183,-059,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[SCHED] Stack HWM: 11252 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).
[RTC] Time: 02:01

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 68,2026-04-08,01:45,031.1,048.2,00.0,182,-061,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,02:00&temp=031.1&humid=048.2&w_speed=00.0&w_dir=182&signal=-059&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,02:00&temp=031.1&humid=048.2&w_speed=00.0&w_dir=182&signal=-059&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
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

**** Storing Last Logged Data as 2026-04-08,02:00
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=02:01 sched=NO cleanup=NO

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
[PWR] Sleep: CurTime=2:1:21
 Sleep=13:53 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮_flash: No core dump partition found!
E (48) esp_core_dump_flash: No core dump partition found!


[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.04V | Solar: 0.00V



[RTC] Time: 02:15
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
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
Signal strength IN gprs task is -61

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:13)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 30.99 C
Humidity    : 49.35 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 527/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 70
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :69,2026-04-08,02:00,031.0,048.8,00.0,183,-059,04.04

Last sample No stored : 69
Last recorded hour/min is 2:0

NO GAPS FOUND ...


Current data inserted is 70,2026-04-08,02:15,031.0,049.4,00.0,182,-061,04.05


append_text->store_text : Used for internal status: 70,2026-04-08,02:15,031.0,049.4,00.0,182,-061,04.05


append_text written to lastrecorded_<stationname>.txt is : 70,2026-04-08,02:15,031.0,049.4,00.0,182,-061,04.05



[FILE] SPIFFS: /001941_20260408.txt | Size: 3763
   ... [Tail Content] ...
0.0,183,-055,04.05
66,2026-04-08,01:15,031.1,047.3,00.0,182,-061,04.04
67,2026-04-08,01:30,031.1,047.7,00.0,182,-061,04.04
68,2026-04-08,01:45,031.1,048.2,00.0,182,-061,04.04
69,2026-04-08,02:00,031.0,048.8,00.0,183,-059,04.04
70,2026-04-08,02:15,031.0,049.4,00.0,182,-061,04.05
-----------------------

[FILE] SD: /001941_20260408.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11424 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).
[RTC] Time: 02:16

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 69,2026-04-08,02:00,031.0,048.8,00.0,183,-059,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,02:15&temp=031.0&humid=048.8&w_speed=00.0&w_dir=183&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,02:15&temp=031.0&humid=048.8&w_speed=00.0&w_dir=183&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
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

**** Storing Last Logged Data as 2026-04-08,02:15
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=02:16 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=2:16:18
 Sleep=13:55 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮ }⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.05V | Solar: 0.00V



[RTC] Time: 02:30
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
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
Signal strength IN gprs task is -61

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] Pre-setting APN for CID 1: bsnlnet (IP)
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:14)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 30.97 C
Humidity    : 49.46 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 527/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 71
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :70,2026-04-08,02:15,031.0,049.4,00.0,182,-061,04.05

Last sample No stored : 70
Last recorded hour/min is 2:15

NO GAPS FOUND ...


Current data inserted is 71,2026-04-08,02:30,031.0,049.5,00.0,182,-061,04.04


append_text->store_text : Used for internal status: 71,2026-04-08,02:30,031.0,049.5,00.0,182,-061,04.04


append_text written to lastrecorded_<stationname>.txt is : 71,2026-04-08,02:30,031.0,049.5,00.0,182,-061,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 3816
   ... [Tail Content] ...
0.0,182,-061,04.04
67,2026-04-08,01:30,031.1,047.7,00.0,182,-061,04.04
68,2026-04-08,01:45,031.1,048.2,00.0,182,-061,04.04
69,2026-04-08,02:00,031.0,048.8,00.0,183,-059,04.04
70,2026-04-08,02:15,031.0,049.4,00.0,182,-061,04.05
71,2026-04-08,02:30,031.0,049.5,00.0,182,-061,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11252 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).
[RTC] Time: 02:31

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 70,2026-04-08,02:15,031.0,049.4,00.0,182,-061,04.05
http_data format is stn_no=001941&rec_time=2026-04-08,02:30&temp=031.0&humid=049.4&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.05&bat_volt2=04.05

Payload is stn_no=001941&rec_time=2026-04-08,02:30&temp=031.0&humid=049.4&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.05&bat_volt2=04.05
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

**** Storing Last Logged Data as 2026-04-08,02:30
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=02:31 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=2:31:21
 Sleep=13:54 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.04V | Solar: 0.00V



[RTC] Time: 02:45
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
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
Signal strength IN gprs task is -63

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] Pre-setting APN for CID 1: bsnlnet (IP)
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:15)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 30.95 C
Humidity    : 49.06 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 527/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 72
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :71,2026-04-08,02:30,031.0,049.5,00.0,182,-061,04.04

Last sample No stored : 71
Last recorded hour/min is 2:30

NO GAPS FOUND ...


Current data inserted is 72,2026-04-08,02:45,031.0,049.1,00.0,182,-063,04.04


append_text->store_text : Used for internal status: 72,2026-04-08,02:45,031.0,049.1,00.0,182,-063,04.04


append_text written to lastrecorded_<stationname>.txt is : 72,2026-04-08,02:45,031.0,049.1,00.0,182,-063,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 3869
   ... [Tail Content] ...
0.0,182,-061,04.04
68,2026-04-08,01:45,031.1,048.2,00.0,182,-061,04.04
69,2026-04-08,02:00,031.0,048.8,00.0,183,-059,04.04
70,2026-04-08,02:15,031.0,049.4,00.0,182,-061,04.05
71,2026-04-08,02:30,031.0,049.5,00.0,182,-061,04.04
72,2026-04-08,02:45,031.0,049.1,00.0,182,-063,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[SCHED] Stack HWM: 11424 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).
[RTC] Time: 02:46

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 71,2026-04-08,02:30,031.0,049.5,00.0,182,-061,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,02:45&temp=031.0&humid=049.5&w_speed=00.0&w_dir=182&signal=-063&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,02:45&temp=031.0&humid=049.5&w_speed=00.0&w_dir=182&signal=-063&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
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

**** Storing Last Logged Data as 2026-04-08,02:45
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=02:46 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=2:46:21
 Sleep=13:53 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮_flash: No core dump partition found!
E (48) esp_core_dump_flash: No core dump partition found!


[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.04V | Solar: 0.00V



[RTC] Time: 03:00
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
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
Signal strength IN gprs task is -59

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:16)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 30.91 C
Humidity    : 49.46 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 527/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 73
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :72,2026-04-08,02:45,031.0,049.1,00.0,182,-063,04.04

Last sample No stored : 72
Last recorded hour/min is 2:45

NO GAPS FOUND ...


Current data inserted is 73,2026-04-08,03:00,030.9,049.5,00.0,182,-059,04.04


append_text->store_text : Used for internal status: 73,2026-04-08,03:00,030.9,049.5,00.0,182,-059,04.04


append_text written to lastrecorded_<stationname>.txt is : 73,2026-04-08,03:00,030.9,049.5,00.0,182,-059,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 3922
   ... [Tail Content] ...
0.0,182,-061,04.04
69,2026-04-08,02:00,031.0,048.8,00.0,183,-059,04.04
70,2026-04-08,02:15,031.0,049.4,00.0,182,-061,04.05
71,2026-04-08,02:30,031.0,049.5,00.0,182,-061,04.04
72,2026-04-08,02:45,031.0,049.1,00.0,182,-063,04.04
73,2026-04-08,03:00,030.9,049.5,00.0,182,-059,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11140 bytes free
[RTC] Time: 03:01
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 72,2026-04-08,02:45,031.0,049.1,00.0,182,-063,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,03:00&temp=031.0&humid=049.1&w_speed=00.0&w_dir=182&signal=-059&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,03:00&temp=031.0&humid=049.1&w_speed=00.0&w_dir=182&signal=-059&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
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

**** Storing Last Logged Data as 2026-04-08,03:00
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=03:01 sched=YES cleanup=NO

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
[PWR] Sleep: CurTime=3:1:24
 Sleep=13:49 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.04V | Solar: 0.00V



[RTC] Time: 03:15
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
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
Signal strength IN gprs task is -61

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] Pre-setting APN for CID 1: bsnlnet (IP)
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:17)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 30.89 C
Humidity    : 50.26 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 527/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 74
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :73,2026-04-08,03:00,030.9,049.5,00.0,182,-059,04.04

Last sample No stored : 73
Last recorded hour/min is 3:0

NO GAPS FOUND ...


Current data inserted is 74,2026-04-08,03:15,030.9,050.3,00.0,182,-061,04.04


append_text->store_text : Used for internal status: 74,2026-04-08,03:15,030.9,050.3,00.0,182,-061,04.04


append_text written to lastrecorded_<stationname>.txt is : 74,2026-04-08,03:15,030.9,050.3,00.0,182,-061,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 3975
   ... [Tail Content] ...
0.0,183,-059,04.04
70,2026-04-08,02:15,031.0,049.4,00.0,182,-061,04.05
71,2026-04-08,02:30,031.0,049.5,00.0,182,-061,04.04
72,2026-04-08,02:45,031.0,049.1,00.0,182,-063,04.04
73,2026-04-08,03:00,030.9,049.5,00.0,182,-059,04.04
74,2026-04-08,03:15,030.9,050.3,00.0,182,-061,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[SCHED] Stack HWM: 11124 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).
[RTC] Time: 03:16

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 73,2026-04-08,03:00,030.9,049.5,00.0,182,-059,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,03:15&temp=030.9&humid=049.5&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,03:15&temp=030.9&humid=049.5&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
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

**** Storing Last Logged Data as 2026-04-08,03:15
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=03:16 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=3:16:21
 Sleep=13:53 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮_flash: No core dump partition found!
E (48) esp_core_dump_flash: No core dump partition found!


[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.05V | Solar: 0.00V



[RTC] Time: 03:30
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
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
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -61

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] Pre-setting APN for CID 1: bsnlnet (IP)
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:18)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 30.87 C
Humidity    : 51.05 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 528/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 75
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :74,2026-04-08,03:15,030.9,050.3,00.0,182,-061,04.04

Last sample No stored : 74
Last recorded hour/min is 3:15

NO GAPS FOUND ...


Current data inserted is 75,2026-04-08,03:30,030.9,051.0,00.0,182,-061,04.04


append_text->store_text : Used for internal status: 75,2026-04-08,03:30,030.9,051.0,00.0,182,-061,04.04


append_text written to lastrecorded_<stationname>.txt is : 75,2026-04-08,03:30,030.9,051.0,00.0,182,-061,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 4028
   ... [Tail Content] ...
0.0,182,-061,04.05
71,2026-04-08,02:30,031.0,049.5,00.0,182,-061,04.04
72,2026-04-08,02:45,031.0,049.1,00.0,182,-063,04.04
73,2026-04-08,03:00,030.9,049.5,00.0,182,-059,04.04
74,2026-04-08,03:15,030.9,050.3,00.0,182,-061,04.04
75,2026-04-08,03:30,030.9,051.0,00.0,182,-061,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11124 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).
[RTC] Time: 03:31

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 74,2026-04-08,03:15,030.9,050.3,00.0,182,-061,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,03:30&temp=030.9&humid=050.3&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,03:30&temp=030.9&humid=050.3&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
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

**** Storing Last Logged Data as 2026-04-08,03:30
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=03:31 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=3:31:21
 Sleep=13:52 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮_flash: No core dump partition found!
E (48) esp_core_dump_flash: No core dump partition found!


[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.04V | Solar: 0.00V



[RTC] Time: 03:45
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
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
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -59

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] Pre-setting APN for CID 1: bsnlnet (IP)
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:19)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 30.86 C
Humidity    : 51.54 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 528/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 76
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :75,2026-04-08,03:30,030.9,051.0,00.0,182,-061,04.04

Last sample No stored : 75
Last recorded hour/min is 3:30

NO GAPS FOUND ...


Current data inserted is 76,2026-04-08,03:45,030.9,051.5,00.0,182,-059,04.04


append_text->store_text : Used for internal status: 76,2026-04-08,03:45,030.9,051.5,00.0,182,-059,04.04


append_text written to lastrecorded_<stationname>.txt is : 76,2026-04-08,03:45,030.9,051.5,00.0,182,-059,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 4081
   ... [Tail Content] ...
0.0,182,-061,04.04
72,2026-04-08,02:45,031.0,049.1,00.0,182,-063,04.04
73,2026-04-08,03:00,030.9,049.5,00.0,182,-059,04.04
74,2026-04-08,03:15,030.9,050.3,00.0,182,-061,04.04
75,2026-04-08,03:30,030.9,051.0,00.0,182,-061,04.04
76,2026-04-08,03:45,030.9,051.5,00.0,182,-059,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[SCHED] Stack HWM: 11188 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).
[RTC] Time: 03:46

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 75,2026-04-08,03:30,030.9,051.0,00.0,182,-061,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,03:45&temp=030.9&humid=051.0&w_speed=00.0&w_dir=182&signal=-059&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,03:45&temp=030.9&humid=051.0&w_speed=00.0&w_dir=182&signal=-059&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
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

**** Storing Last Logged Data as 2026-04-08,03:45
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=03:46 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=3:46:21
 Sleep=13:51 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.05V | Solar: 0.00V



[RTC] Time: 04:00
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[GPRS] Powering On...
[FS] Loaded Prev Wind Speed Avg: 0.00
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
Signal strength IN gprs task is -63

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:20)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 30.84 C
Humidity    : 51.84 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 528/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 77
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :76,2026-04-08,03:45,030.9,051.5,00.0,182,-059,04.04

Last sample No stored : 76
Last recorded hour/min is 3:45

NO GAPS FOUND ...


Current data inserted is 77,2026-04-08,04:00,030.8,051.8,00.0,182,-063,04.04


append_text->store_text : Used for internal status: 77,2026-04-08,04:00,030.8,051.8,00.0,182,-063,04.04


append_text written to lastrecorded_<stationname>.txt is : 77,2026-04-08,04:00,030.8,051.8,00.0,182,-063,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 4134
   ... [Tail Content] ...
0.0,182,-063,04.04
73,2026-04-08,03:00,030.9,049.5,00.0,182,-059,04.04
74,2026-04-08,03:15,030.9,050.3,00.0,182,-061,04.04
75,2026-04-08,03:30,030.9,051.0,00.0,182,-061,04.04
76,2026-04-08,03:45,030.9,051.5,00.0,182,-059,04.04
77,2026-04-08,04:00,030.8,051.8,00.0,182,-063,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[SCHED] Stack HWM: 11252 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 76,2026-04-08,03:45,030.9,051.5,00.0,182,-059,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,04:00&temp=030.9&humid=051.5&w_speed=00.0&w_dir=182&signal=-063&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,04:00&temp=030.9&humid=051.5&w_speed=00.0&w_dir=182&signal=-063&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
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
[RTC] Time: 04:01
[PWR] Signature persisted successfully.

**** Storing Last Logged Data as 2026-04-08,04:00
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=04:01 sched=NO cleanup=NO

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
[PWR] Sleep: CurTime=4:1:20
 Sleep=13:54 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.06V | Solar: 0.00V



[RTC] Time: 04:15
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).
[GPRS] Powering On...

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
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -59

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:21)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 30.80 C
Humidity    : 52.03 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 528/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 78
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :77,2026-04-08,04:00,030.8,051.8,00.0,182,-063,04.04

Last sample No stored : 77
Last recorded hour/min is 4:0

NO GAPS FOUND ...


Current data inserted is 78,2026-04-08,04:15,030.8,052.0,00.0,182,-059,04.05


append_text->store_text : Used for internal status: 78,2026-04-08,04:15,030.8,052.0,00.0,182,-059,04.05


append_text written to lastrecorded_<stationname>.txt is : 78,2026-04-08,04:15,030.8,052.0,00.0,182,-059,04.05



[FILE] SPIFFS: /001941_20260408.txt | Size: 4187
   ... [Tail Content] ...
0.0,182,-059,04.04
74,2026-04-08,03:15,030.9,050.3,00.0,182,-061,04.04
75,2026-04-08,03:30,030.9,051.0,00.0,182,-061,04.04
76,2026-04-08,03:45,030.9,051.5,00.0,182,-059,04.04
77,2026-04-08,04:00,030.8,051.8,00.0,182,-063,04.04
78,2026-04-08,04:15,030.8,052.0,00.0,182,-059,04.05
-----------------------

[FILE] SD: /001941_20260408.txt


[SCHED] Stack HWM: 11252 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 77,2026-04-08,04:00,030.8,051.8,00.0,182,-063,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,04:15&temp=030.8&humid=051.8&w_speed=00.0&w_dir=182&signal=-059&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,04:15&temp=030.8&humid=051.8&w_speed=00.0&w_dir=182&signal=-059&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
[HTTP] Using Fast v3.0 Handshake...
[RTC] Time: 04:16
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

**** Storing Last Logged Data as 2026-04-08,04:15
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=04:16 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=4:16:20
 Sleep=13:55 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.05V | Solar: 0.00V



[RTC] Time: 04:30
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).
[GPRS] Powering On...

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
Signal strength IN gprs task is -61

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:22)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 30.79 C
Humidity    : 51.45 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 528/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 79
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :78,2026-04-08,04:15,030.8,052.0,00.0,182,-059,04.05

Last sample No stored : 78
Last recorded hour/min is 4:15

NO GAPS FOUND ...


Current data inserted is 79,2026-04-08,04:30,030.8,051.4,00.0,182,-061,04.04


append_text->store_text : Used for internal status: 79,2026-04-08,04:30,030.8,051.4,00.0,182,-061,04.04


append_text written to lastrecorded_<stationname>.txt is : 79,2026-04-08,04:30,030.8,051.4,00.0,182,-061,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 4240
   ... [Tail Content] ...
0.0,182,-061,04.04
75,2026-04-08,03:30,030.9,051.0,00.0,182,-061,04.04
76,2026-04-08,03:45,030.9,051.5,00.0,182,-059,04.04
77,2026-04-08,04:00,030.8,051.8,00.0,182,-063,04.04
78,2026-04-08,04:15,030.8,052.0,00.0,182,-059,04.05
79,2026-04-08,04:30,030.8,051.4,00.0,182,-061,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[SCHED] Stack HWM: 11252 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 78,2026-04-08,04:15,030.8,052.0,00.0,182,-059,04.05
http_data format is stn_no=001941&rec_time=2026-04-08,04:30&temp=030.8&humid=052.0&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.05&bat_volt2=04.05

Payload is stn_no=001941&rec_time=2026-04-08,04:30&temp=030.8&humid=052.0&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.05&bat_volt2=04.05
[HTTP] Using Fast v3.0 Handshake...
[RTC] Time: 04:31
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

**** Storing Last Logged Data as 2026-04-08,04:30
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=04:31 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=4:31:20
 Sleep=13:54 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮_flash: No core dump partition found!
E (48) esp_core_dump_flash: No core dump partition found!


[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.05V | Solar: 0.00V



[RTC] Time: 04:45
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).
[GPRS] Powering On...

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
Signal strength IN gprs task is -61

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:23)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 30.75 C
Humidity    : 50.07 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 528/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 80
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :79,2026-04-08,04:30,030.8,051.4,00.0,182,-061,04.04

Last sample No stored : 79
Last recorded hour/min is 4:30

NO GAPS FOUND ...


Current data inserted is 80,2026-04-08,04:45,030.8,050.1,00.0,182,-061,04.04


append_text->store_text : Used for internal status: 80,2026-04-08,04:45,030.8,050.1,00.0,182,-061,04.04


append_text written to lastrecorded_<stationname>.txt is : 80,2026-04-08,04:45,030.8,050.1,00.0,182,-061,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 4293
   ... [Tail Content] ...
0.0,182,-061,04.04
76,2026-04-08,03:45,030.9,051.5,00.0,182,-059,04.04
77,2026-04-08,04:00,030.8,051.8,00.0,182,-063,04.04
78,2026-04-08,04:15,030.8,052.0,00.0,182,-059,04.05
79,2026-04-08,04:30,030.8,051.4,00.0,182,-061,04.04
80,2026-04-08,04:45,030.8,050.1,00.0,182,-061,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[SCHED] Stack HWM: 11252 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).
[RTC] Time: 04:46

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 79,2026-04-08,04:30,030.8,051.4,00.0,182,-061,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,04:45&temp=030.8&humid=051.4&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,04:45&temp=030.8&humid=051.4&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
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

**** Storing Last Logged Data as 2026-04-08,04:45
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=04:46 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=4:46:18
 Sleep=13:55 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮_flash: No core dump partition found!
E (48) esp_core_dump_flash: No core dump partition found!


[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.04V | Solar: 0.00V



[RTC] Time: 05:00
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).
[GPRS] Powering On...

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
Signal strength IN gprs task is -61

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:24)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 30.73 C
Humidity    : 49.18 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 528/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 81
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :80,2026-04-08,04:45,030.8,050.1,00.0,182,-061,04.04

Last sample No stored : 80
Last recorded hour/min is 4:45

NO GAPS FOUND ...


Current data inserted is 81,2026-04-08,05:00,030.7,049.2,00.0,182,-061,04.04


append_text->store_text : Used for internal status: 81,2026-04-08,05:00,030.7,049.2,00.0,182,-061,04.04


append_text written to lastrecorded_<stationname>.txt is : 81,2026-04-08,05:00,030.7,049.2,00.0,182,-061,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 4346
   ... [Tail Content] ...
0.0,182,-059,04.04
77,2026-04-08,04:00,030.8,051.8,00.0,182,-063,04.04
78,2026-04-08,04:15,030.8,052.0,00.0,182,-059,04.05
79,2026-04-08,04:30,030.8,051.4,00.0,182,-061,04.04
80,2026-04-08,04:45,030.8,050.1,00.0,182,-061,04.04
81,2026-04-08,05:00,030.7,049.2,00.0,182,-061,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11424 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 80,2026-04-08,04:45,030.8,050.1,00.0,182,-061,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,05:00&temp=030.8&humid=050.1&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,05:00&temp=030.8&humid=050.1&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
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
[RTC] Time: 05:01
[PWR] Signature persisted successfully.

**** Storing Last Logged Data as 2026-04-08,05:00
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=05:01 sched=NO cleanup=NO

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
[PWR] Sleep: CurTime=5:1:17
 Sleep=13:55 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.04V | Solar: 0.00V



[RTC] Time: 05:15
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).
[GPRS] Powering On...

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
Signal strength IN gprs task is -61

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] Pre-setting APN for CID 1: bsnlnet (IP)
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:25)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 30.70 C
Humidity    : 49.08 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 528/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 82
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :81,2026-04-08,05:00,030.7,049.2,00.0,182,-061,04.04

Last sample No stored : 81
Last recorded hour/min is 5:0

NO GAPS FOUND ...


Current data inserted is 82,2026-04-08,05:15,030.7,049.1,00.0,182,-061,04.04


append_text->store_text : Used for internal status: 82,2026-04-08,05:15,030.7,049.1,00.0,182,-061,04.04


append_text written to lastrecorded_<stationname>.txt is : 82,2026-04-08,05:15,030.7,049.1,00.0,182,-061,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 4399
   ... [Tail Content] ...
0.0,182,-063,04.04
78,2026-04-08,04:15,030.8,052.0,00.0,182,-059,04.05
79,2026-04-08,04:30,030.8,051.4,00.0,182,-061,04.04
80,2026-04-08,04:45,030.8,050.1,00.0,182,-061,04.04
81,2026-04-08,05:00,030.7,049.2,00.0,182,-061,04.04
82,2026-04-08,05:15,030.7,049.1,00.0,182,-061,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[SCHED] Stack HWM: 11424 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 81,2026-04-08,05:00,030.7,049.2,00.0,182,-061,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,05:15&temp=030.7&humid=049.2&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,05:15&temp=030.7&humid=049.2&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
[HTTP] Using Fast v3.0 Handshake...
[RTC] Time: 05:16
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

**** Storing Last Logged Data as 2026-04-08,05:15
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=05:16 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=5:16:17
 Sleep=13:56 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.04V | Solar: 0.00V



[RTC] Time: 05:30
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).
[GPRS] Powering On...

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
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -59

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:26)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 30.67 C
Humidity    : 49.08 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 528/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 83
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :82,2026-04-08,05:15,030.7,049.1,00.0,182,-061,04.04

Last sample No stored : 82
Last recorded hour/min is 5:15

NO GAPS FOUND ...


Current data inserted is 83,2026-04-08,05:30,030.7,049.1,00.0,182,-059,04.04


append_text->store_text : Used for internal status: 83,2026-04-08,05:30,030.7,049.1,00.0,182,-059,04.04


append_text written to lastrecorded_<stationname>.txt is : 83,2026-04-08,05:30,030.7,049.1,00.0,182,-059,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 4452
   ... [Tail Content] ...
0.0,182,-059,04.05
79,2026-04-08,04:30,030.8,051.4,00.0,182,-061,04.04
80,2026-04-08,04:45,030.8,050.1,00.0,182,-061,04.04
81,2026-04-08,05:00,030.7,049.2,00.0,182,-061,04.04
82,2026-04-08,05:15,030.7,049.1,00.0,182,-061,04.04
83,2026-04-08,05:30,030.7,049.1,00.0,182,-059,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[SCHED] Stack HWM: 11252 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).
[RTC] Time: 05:31

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 82,2026-04-08,05:15,030.7,049.1,00.0,182,-061,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,05:30&temp=030.7&humid=049.1&w_speed=00.0&w_dir=182&signal=-059&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,05:30&temp=030.7&humid=049.1&w_speed=00.0&w_dir=182&signal=-059&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
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

**** Storing Last Logged Data as 2026-04-08,05:30
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=05:31 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=5:31:21
 Sleep=13:54 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.04V | Solar: 0.00V



[RTC] Time: 05:45
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).
[GPRS] Powering On...

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
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -57

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:27)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 30.67 C
Humidity    : 49.48 %
Wind Speed  : 0.00 m/s
Wind Dir    : 183 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 529/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 84
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :83,2026-04-08,05:30,030.7,049.1,00.0,182,-059,04.04

Last sample No stored : 83
Last recorded hour/min is 5:30

NO GAPS FOUND ...


Current data inserted is 84,2026-04-08,05:45,030.7,049.5,00.0,183,-057,04.04


append_text->store_text : Used for internal status: 84,2026-04-08,05:45,030.7,049.5,00.0,183,-057,04.04


append_text written to lastrecorded_<stationname>.txt is : 84,2026-04-08,05:45,030.7,049.5,00.0,183,-057,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 4505
   ... [Tail Content] ...
0.0,182,-061,04.04
80,2026-04-08,04:45,030.8,050.1,00.0,182,-061,04.04
81,2026-04-08,05:00,030.7,049.2,00.0,182,-061,04.04
82,2026-04-08,05:15,030.7,049.1,00.0,182,-061,04.04
83,2026-04-08,05:30,030.7,049.1,00.0,182,-059,04.04
84,2026-04-08,05:45,030.7,049.5,00.0,183,-057,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11216 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 83,2026-04-08,05:30,030.7,049.1,00.0,182,-059,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,05:45&temp=030.7&humid=049.1&w_speed=00.0&w_dir=182&signal=-057&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,05:45&temp=030.7&humid=049.1&w_speed=00.0&w_dir=182&signal=-057&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
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
[RTC] Time: 05:46
[PWR] Signature persisted successfully.

**** Storing Last Logged Data as 2026-04-08,05:45
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=05:46 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=5:46:17
 Sleep=13:57 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.04V | Solar: 0.00V



[RTC] Time: 06:00
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).
[GPRS] Powering On...

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
Signal strength IN gprs task is -61

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:28)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 30.65 C
Humidity    : 50.88 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 529/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 85
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :84,2026-04-08,05:45,030.7,049.5,00.0,183,-057,04.04

Last sample No stored : 84
Last recorded hour/min is 5:45

NO GAPS FOUND ...


Current data inserted is 85,2026-04-08,06:00,030.7,050.9,00.0,182,-061,04.04


append_text->store_text : Used for internal status: 85,2026-04-08,06:00,030.7,050.9,00.0,182,-061,04.04


append_text written to lastrecorded_<stationname>.txt is : 85,2026-04-08,06:00,030.7,050.9,00.0,182,-061,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 4558
   ... [Tail Content] ...
0.0,182,-061,04.04
81,2026-04-08,05:00,030.7,049.2,00.0,182,-061,04.04
82,2026-04-08,05:15,030.7,049.1,00.0,182,-061,04.04
83,2026-04-08,05:30,030.7,049.1,00.0,182,-059,04.04
84,2026-04-08,05:45,030.7,049.5,00.0,183,-057,04.04
85,2026-04-08,06:00,030.7,050.9,00.0,182,-061,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[SCHED] Stack HWM: 11252 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 84,2026-04-08,05:45,030.7,049.5,00.0,183,-057,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,06:00&temp=030.7&humid=049.5&w_speed=00.0&w_dir=183&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,06:00&temp=030.7&humid=049.5&w_speed=00.0&w_dir=183&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
[HTTP] Using Fast v3.0 Handshake...
[RTC] Time: 06:01
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

**** Storing Last Logged Data as 2026-04-08,06:00
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=06:01 sched=YES cleanup=NO

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
[PWR] Sleep: CurTime=6:1:20
 Sleep=13:54 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.06V | Solar: 0.00V



[RTC] Time: 06:15
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).
[GPRS] Powering On...

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
Signal strength IN gprs task is -55

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:29)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 30.62 C
Humidity    : 51.76 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 529/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 86
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :85,2026-04-08,06:00,030.7,050.9,00.0,182,-061,04.04

Last sample No stored : 85
Last recorded hour/min is 6:0

NO GAPS FOUND ...


Current data inserted is 86,2026-04-08,06:15,030.6,051.8,00.0,182,-055,04.04


append_text->store_text : Used for internal status: 86,2026-04-08,06:15,030.6,051.8,00.0,182,-055,04.04


append_text written to lastrecorded_<stationname>.txt is : 86,2026-04-08,06:15,030.6,051.8,00.0,182,-055,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 4611
   ... [Tail Content] ...
0.0,182,-061,04.04
82,2026-04-08,05:15,030.7,049.1,00.0,182,-061,04.04
83,2026-04-08,05:30,030.7,049.1,00.0,182,-059,04.04
84,2026-04-08,05:45,030.7,049.5,00.0,183,-057,04.04
85,2026-04-08,06:00,030.7,050.9,00.0,182,-061,04.04
86,2026-04-08,06:15,030.6,051.8,00.0,182,-055,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[SCHED] Stack HWM: 11424 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 85,2026-04-08,06:00,030.7,050.9,00.0,182,-061,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,06:15&temp=030.7&humid=050.9&w_speed=00.0&w_dir=182&signal=-055&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,06:15&temp=030.7&humid=050.9&w_speed=00.0&w_dir=182&signal=-055&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
[HTTP] Using Fast v3.0 Handshake...
[RTC] Time: 06:16
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

**** Storing Last Logged Data as 2026-04-08,06:15
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=06:16 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=6:16:17
 Sleep=13:55 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮ }⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.05V | Solar: 0.00V



[RTC] Time: 06:30
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).
[GPRS] Powering On...

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
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -59

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:30)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 30.58 C
Humidity    : 52.16 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 529/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 87
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :86,2026-04-08,06:15,030.6,051.8,00.0,182,-055,04.04

Last sample No stored : 86
Last recorded hour/min is 6:15

NO GAPS FOUND ...


Current data inserted is 87,2026-04-08,06:30,030.6,052.2,00.0,182,-059,04.04


append_text->store_text : Used for internal status: 87,2026-04-08,06:30,030.6,052.2,00.0,182,-059,04.04


append_text written to lastrecorded_<stationname>.txt is : 87,2026-04-08,06:30,030.6,052.2,00.0,182,-059,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 4664
   ... [Tail Content] ...
0.0,182,-061,04.04
83,2026-04-08,05:30,030.7,049.1,00.0,182,-059,04.04
84,2026-04-08,05:45,030.7,049.5,00.0,183,-057,04.04
85,2026-04-08,06:00,030.7,050.9,00.0,182,-061,04.04
86,2026-04-08,06:15,030.6,051.8,00.0,182,-055,04.04
87,2026-04-08,06:30,030.6,052.2,00.0,182,-059,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11252 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 86,2026-04-08,06:15,030.6,051.8,00.0,182,-055,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,06:30&temp=030.6&humid=051.8&w_speed=00.0&w_dir=182&signal=-059&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,06:30&temp=030.6&humid=051.8&w_speed=00.0&w_dir=182&signal=-059&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
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
[RTC] Time: 06:31
[PWR] Signature persisted successfully.

**** Storing Last Logged Data as 2026-04-08,06:30
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=06:31 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=6:31:17
 Sleep=13:57 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.05V | Solar: 0.00V



[RTC] Time: 06:45
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).
[GPRS] Powering On...

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
Signal strength IN gprs task is -63

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] Pre-setting APN for CID 1: bsnlnet (IP)
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:31)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 30.58 C
Humidity    : 52.45 %
Wind Speed  : 0.00 m/s
Wind Dir    : 183 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 529/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 88
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :87,2026-04-08,06:30,030.6,052.2,00.0,182,-059,04.04

Last sample No stored : 87
Last recorded hour/min is 6:30

NO GAPS FOUND ...


Current data inserted is 88,2026-04-08,06:45,030.6,052.5,00.0,183,-063,04.04


append_text->store_text : Used for internal status: 88,2026-04-08,06:45,030.6,052.5,00.0,183,-063,04.04


append_text written to lastrecorded_<stationname>.txt is : 88,2026-04-08,06:45,030.6,052.5,00.0,183,-063,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 4717
   ... [Tail Content] ...
0.0,182,-059,04.04
84,2026-04-08,05:45,030.7,049.5,00.0,183,-057,04.04
85,2026-04-08,06:00,030.7,050.9,00.0,182,-061,04.04
86,2026-04-08,06:15,030.6,051.8,00.0,182,-055,04.04
87,2026-04-08,06:30,030.6,052.2,00.0,182,-059,04.04
88,2026-04-08,06:45,030.6,052.5,00.0,183,-063,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11424 bytes free
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 87,2026-04-08,06:30,030.6,052.2,00.0,182,-059,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,06:45&temp=030.6&humid=052.2&w_speed=00.0&w_dir=182&signal=-063&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,06:45&temp=030.6&humid=052.2&w_speed=00.0&w_dir=182&signal=-063&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
[HTTP] Using Fast v3.0 Handshake...
[RTC] Time: 06:46
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

**** Storing Last Logged Data as 2026-04-08,06:45
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=06:46 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=6:46:18
 Sleep=13:56 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_du=⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.04V | Solar: 0.00V



[RTC] Time: 07:00
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).
[GPRS] Powering On...

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
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -63

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:32)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 30.58 C
Humidity    : 52.65 %
Wind Speed  : 0.00 m/s
Wind Dir    : 182 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 529/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 89
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :88,2026-04-08,06:45,030.6,052.5,00.0,183,-063,04.04

Last sample No stored : 88
Last recorded hour/min is 6:45

NO GAPS FOUND ...


Current data inserted is 89,2026-04-08,07:00,030.6,052.6,00.0,182,-063,04.04


append_text->store_text : Used for internal status: 89,2026-04-08,07:00,030.6,052.6,00.0,182,-063,04.04


append_text written to lastrecorded_<stationname>.txt is : 89,2026-04-08,07:00,030.6,052.6,00.0,182,-063,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 4770
   ... [Tail Content] ...
0.0,183,-057,04.04
85,2026-04-08,06:00,030.7,050.9,00.0,182,-061,04.04
86,2026-04-08,06:15,030.6,051.8,00.0,182,-055,04.04
87,2026-04-08,06:30,030.6,052.2,00.0,182,-059,04.04
88,2026-04-08,06:45,030.6,052.5,00.0,183,-063,04.04
89,2026-04-08,07:00,030.6,052.6,00.0,182,-063,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[SCHED] Stack HWM: 11252 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 88,2026-04-08,06:45,030.6,052.5,00.0,183,-063,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,07:00&temp=030.6&humid=052.5&w_speed=00.0&w_dir=183&signal=-063&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,07:00&temp=030.6&humid=052.5&w_speed=00.0&w_dir=183&signal=-063&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
[HTTP] Using Fast v3.0 Handshake...
[RTC] Time: 07:01
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

**** Storing Last Logged Data as 2026-04-08,07:00
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=07:01 sched=NO cleanup=NO

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
[PWR] Sleep: CurTime=7:1:19
 Sleep=13:54 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_duy⸮ }⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.05V | Solar: 0.00V



[RTC] Time: 07:15
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).
[GPRS] Powering On...

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
Signal strength IN gprs task is -61

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:33)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 30.54 C
Humidity    : 52.95 %
Wind Speed  : 0.00 m/s
Wind Dir    : 183 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 529/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 90
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :89,2026-04-08,07:00,030.6,052.6,00.0,182,-063,04.04

Last sample No stored : 89
Last recorded hour/min is 7:0

NO GAPS FOUND ...


Current data inserted is 90,2026-04-08,07:15,030.5,052.9,00.0,183,-061,04.04


append_text->store_text : Used for internal status: 90,2026-04-08,07:15,030.5,052.9,00.0,183,-061,04.04


append_text written to lastrecorded_<stationname>.txt is : 90,2026-04-08,07:15,030.5,052.9,00.0,183,-061,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 4823
   ... [Tail Content] ...
0.0,182,-061,04.04
86,2026-04-08,06:15,030.6,051.8,00.0,182,-055,04.04
87,2026-04-08,06:30,030.6,052.2,00.0,182,-059,04.04
88,2026-04-08,06:45,030.6,052.5,00.0,183,-063,04.04
89,2026-04-08,07:00,030.6,052.6,00.0,182,-063,04.04
90,2026-04-08,07:15,030.5,052.9,00.0,183,-061,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[SCHED] Stack HWM: 11424 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).
[RTC] Time: 07:16

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 89,2026-04-08,07:00,030.6,052.6,00.0,182,-063,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,07:15&temp=030.6&humid=052.6&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,07:15&temp=030.6&humid=052.6&w_speed=00.0&w_dir=182&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
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

**** Storing Last Logged Data as 2026-04-08,07:15
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=07:16 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=7:16:21
 Sleep=13:54 (min:sec)
[PWR] Entering Deep Sleep
ets Jul 29 2019 12:21:46

rst:0x5 (DEEPSLEEP_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0030,len:4980
load:0x40078000,len:16612
load:0x40080400,len:3500
entry 0x400805b4
E (48) esp_core_du=⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jRT	⸮J*⸮⸮}⸮⸮ɕ}⸮յ⸮}⸮⸮⸮͡⸮r⸮⸮⸮ɕ⸮"յ⸮⸮⸮⸮⸮ѥѥ⸮⸮⸮2⸮չ⸮⸮jR⸮

[BOOT] HELLO! System starting... (Debug Enabled)
[BME] Testing I2C address 0x77...
[BME] 0x77 failed. Testing I2C address 0x76...
[BME] Init: SENSOR NOT FOUND!
[HDC] Checking 1080 ID (Reg 0xFF): 0x1050
[HDC] Init: SUCCESS (HDC1080)
[BOOT] RTC: OK
[BOOT] SPIFFS: OK
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] BME280: NOT FOUND
[BOOT] HDC Sensor: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001941
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWS9-DMC-5.81 | Network: KSNDMC | Type: TWS

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: A09B2029E748
[BOOT] Chip ID: A09B2029E748 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=1704, Rain=0
[RTC] Task Started
[PWR] Battery: 4.04V | Solar: 0.00V



[RTC] Time: 07:30
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001941_20260408.txt
SPIFFS [INIT]: lastrecorded_001941.txt
SPIFFS [INIT]: dailyftp_20260408.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: dailyftp_20260404.txt
SPIFFS [INIT]: 001941_20260405.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001941_20260406.txt
SPIFFS [INIT]: 001941_20260407.txt
SPIFFS [INIT]: closingdata_001941.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: rf_fw.txt
[BOOT] BME280 not found. bmeTask NOT created.
[FS] Loaded Prev Wind Speed Avg: 0.00
ULP Counting Enabled (attachInterrupt 27 Disabled).
[GPRS] Powering On...

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
Signal strength IN gprs task is -61

************************
GETTING NETWORK 
************************
Final CCID parsed: 89917190324927658487
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:34)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Smart APN: Match Found! APN: bsnlnet
Trying APN: bsnlnet
[APN] Flash match found. Skipping redundant write.
[GPRS] CGACT Resp: 
OK

Scheduler: GPRS task ready or timeout.

--- Sensor Data Snapshot ---
Temperature : 30.54 C
Humidity    : 53.14 %
Wind Speed  : 0.00 m/s
Wind Dir    : 183 deg
Wind Pulses : 0.00
----------------------------
--- Storage Status ---
SPIFFS: 529/4640
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 91
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :90,2026-04-08,07:15,030.5,052.9,00.0,183,-061,04.04

Last sample No stored : 90
Last recorded hour/min is 7:15

NO GAPS FOUND ...


Current data inserted is 91,2026-04-08,07:30,030.5,053.1,00.0,183,-061,04.04


append_text->store_text : Used for internal status: 91,2026-04-08,07:30,030.5,053.1,00.0,183,-061,04.04


append_text written to lastrecorded_<stationname>.txt is : 91,2026-04-08,07:30,030.5,053.1,00.0,183,-061,04.04



[FILE] SPIFFS: /001941_20260408.txt | Size: 4876
   ... [Tail Content] ...
0.0,182,-055,04.04
87,2026-04-08,06:30,030.6,052.2,00.0,182,-059,04.04
88,2026-04-08,06:45,030.6,052.5,00.0,183,-063,04.04
89,2026-04-08,07:00,030.6,052.6,00.0,182,-063,04.04
90,2026-04-08,07:15,030.5,052.9,00.0,183,-061,04.04
91,2026-04-08,07:30,030.5,053.1,00.0,183,-061,04.04
-----------------------

[FILE] SD: /001941_20260408.txt


[SCHED] Stack HWM: 11252 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_tws_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).
[RTC] Time: 07:31

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001941_20260408.txt

Current Data to be sent is : 90,2026-04-08,07:15,030.5,052.9,00.0,183,-061,04.04
http_data format is stn_no=001941&rec_time=2026-04-08,07:30&temp=030.5&humid=052.9&w_speed=00.0&w_dir=183&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04

Payload is stn_no=001941&rec_time=2026-04-08,07:30&temp=030.5&humid=052.9&w_speed=00.0&w_dir=183&signal=-061&key=climate4p2013&bat_volt=04.04&bat_volt2=04.04
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

**** Storing Last Logged Data as 2026-04-08,07:30
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=07:31 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=7:31:18
 Sleep=13:55 (min:sec)
[PWR] Entering Deep Sleep
