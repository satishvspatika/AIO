
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
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.80 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.15V | Solar: 0.00V



[RTC] Time: 04:15
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260407.txt
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
Raw CICCID Resp: 
+ICCID: 89914509006871527866

OK

Final CCID parsed: 89914509006871527866
[CACHE] New SIM or No Cache. Performing full discovery...
CSPN Logic response: 
+CSPN: "airtel",0

OK

COPS Logic response: 
+COPS: 0

OK

[APN] Airtel IoT/M2M SIM detected via ICCID prefix.
IMSI: 404450687152786
Service Provider APN is airteliot.com
Carrier: Airtel
Number: 404450687152786


************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=2 (LastSucc:2 SlotsOn2G:0)
Reg Search [BSNL:0]... Status:0 Iter:#1/24
Reg Search [BSNL:0]... Status:0 Iter:#2/24
Reg Search [BSNL:0]... Status:0 Iter:#3/24
Reg Search [BSNL:0]... Status:0 Iter:#4/24
Reg Search [BSNL:0]... Status:0 Iter:#5/24
[GPRS] Tier1 @ iter5: Auto mode stalled. Fallback to GSM-only (CNMP=13)...
Reg Search [BSNL:0]... Status:0 Iter:#6/24
[RTC] Time: 04:16
Reg Search [BSNL:0]... Status:0 Iter:#7/24
Reg Search [BSNL:0]... Status:0 Iter:#8/24
Reg Search [BSNL:0]... Status:0 Iter:#9/24
Reg Search [BSNL:0]... Status:0 Iter:#10/24
[GPRS] Tier2 @ iter10: Radio-Off SIM Scrub...
Reg Search [BSNL:0]... Status:0 Iter:#11/24
Reg Search [BSNL:0]... Status:2 Iter:#12/24
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Memory Check - Stored CCID: 89914509006871527866 | Stored APN: airteliot.com
Smart APN: Match Found!
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
CGACT Resp: OK
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
SPIFFS: 28/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 78
RF Close date from RTC = 2026-04-07  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :77,2026-04-07,04:00,000.00,000.0,000.0,00.0,000,-111,04.15

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 4:0

NO GAPS FOUND ...


Current data inserted is 78,2026-04-07,04:15,000.00,000.0,000.0,00.0,000,-075,04.13


append_text->store_text : Used for internal status: 78,2026-04-07,04:15,000.00,000.0,000.0,00.0,000,-075,04.13


append_text written to lastrecorded_<stationname>.txt is : 78,2026-04-07,04:15,000.00,000.0,000.0,00.0,000,-075,04.13



[FILE] SPIFFS: /001951_20260407.txt | Size: 4740
   ... [Tail Content] ...
00,-111,04.13
74,2026-04-07,03:15,000.00,000.0,000.0,00.0,000,-111,04.13
75,2026-04-07,03:30,000.00,000.0,000.0,00.0,000,-111,04.13
76,2026-04-07,03:45,000.00,000.0,000.0,00.0,000,-111,04.13
77,2026-04-07,04:00,000.00,000.0,000.0,00.0,000,-111,04.15
78,2026-04-07,04:15,000.00,000.0,000.0,00.0,000,-075,04.13
-----------------------

[FILE] SD: /001951_20260407.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
51;2026-04-07,02:15;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,02:30;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,02:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:00;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:15;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:30;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11544 bytes free
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

SPIFF FILE EXISTS ..../001951_20260407.txt

Current Data to be sent is : 77,2026-04-07,04:00,000.00,000.0,000.0,00.0,000,-111,04.15
http_data format is stn_no=001951&rec_time=2026-04-07,04:15&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-075&bat_volt=04.15&bat_volt2=04.15

Payload is stn_no=001951&rec_time=2026-04-07,04:15&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-075&bat_volt=04.15&bat_volt2=04.15
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

**** Storing Last Logged Data as 2026-04-07,04:15
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=25 cur_time=04:17 sched=NO cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[FTP] Backlog Push: 25 records found. Mode: THRESHOLD_MET

FTP file name is /TWSRF_001951_260407_041500.kwd

SampleNo  is 78

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Huge Backlog! Records chunked for TX. Original queue protected until success.
Retrieved file is /TWSRF_001951_260407_041500.kwd
Initializing A7672S for FTP...
[FTP] Airtel/Jio detected. Smart Default: Passive (1).

ERROR


OK

+CFTPSSTART: 0

[RTC] Time: 04:18
[FTP] Configuring mode (1)...

OK

[FTP] Warming up DNS...
[FTP] DNS Resolved: 27.34.245.70

OK

+CFTPSLOGIN: 0

Result is 0
FTP Login success
File size: 945
[FTP] FSDEL path: TWSRF_001951_260407_041500.kwd
[FTP] FSOPEN cmd: AT+FSOPEN="C:/TWSRF_001951_260407_041500.kwd",0
[FTP] Found FH: 1
[FTP] Sending FSWRITE command...
[FTP] Data stream finished.
[FTP] Dispatching PUTFILE command...
[FTP] Dispatching PUTFILE command...
Response of AT+CFTPSPUTFILE: 
ERROR

[FTP] PUT failed. Re-staging and retrying in mode (0)...

ERROR


OK

+CFTPSSTART: 0

[FTP] Configuring mode (0)...

OK

[FTP] Warming up DNS...
[FTP] DNS Resolved: 89.32.144.163

OK

+CFTPSLOGIN: 9

Result is 9
FTP Login unsuccessful
[FTP] Race Guard: Logout too fast (Cleanup)? Waiting 1s drain.
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[RTC] Time: 04:19
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=4:19:11
 Sleep=11:4 (min:sec)
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
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.80 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.15V | Solar: 0.00V



[RTC] Time: 04:30
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260407.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: TWSRF_001951_260407_041500.kwd
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
Raw CICCID Resp: 
+ICCID: 89914509006871527866

OK

Final CCID parsed: 89914509006871527866
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:1)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Memory Check - Stored CCID: 89914509006871527866 | Stored APN: airteliot.com
Smart APN: Match Found!
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
CGACT Resp: OK
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
SPIFFS: 29/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 79
RF Close date from RTC = 2026-04-07  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :78,2026-04-07,04:15,000.00,000.0,000.0,00.0,000,-075,04.13

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 4:15

NO GAPS FOUND ...


Current data inserted is 79,2026-04-07,04:30,000.00,000.0,000.0,00.0,000,-069,04.13


append_text->store_text : Used for internal status: 79,2026-04-07,04:30,000.00,000.0,000.0,00.0,000,-069,04.13


append_text written to lastrecorded_<stationname>.txt is : 79,2026-04-07,04:30,000.00,000.0,000.0,00.0,000,-069,04.13



[FILE] SPIFFS: /001951_20260407.txt | Size: 4800
   ... [Tail Content] ...
00,-111,04.13
75,2026-04-07,03:30,000.00,000.0,000.0,00.0,000,-111,04.13
76,2026-04-07,03:45,000.00,000.0,000.0,00.0,000,-111,04.13
77,2026-04-07,04:00,000.00,000.0,000.0,00.0,000,-111,04.15
78,2026-04-07,04:15,000.00,000.0,000.0,00.0,000,-075,04.13
79,2026-04-07,04:30,000.00,000.0,000.0,00.0,000,-069,04.13
-----------------------

[FILE] SD: /001951_20260407.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
51;2026-04-07,02:15;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,02:30;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,02:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:00;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:15;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:30;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11544 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[RTC] Time: 04:31
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).
[GPRS] Bearer status check: Re-initializing IP stack...
[GPRS] Starting HTTP...
HTTP POST REQUEST IS AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260407.txt

Current Data to be sent is : 78,2026-04-07,04:15,000.00,000.0,000.0,00.0,000,-075,04.13
http_data format is stn_no=001951&rec_time=2026-04-07,04:30&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.13&bat_volt2=04.13

Payload is stn_no=001951&rec_time=2026-04-07,04:30&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.13&bat_volt2=04.13
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

**** Storing Last Logged Data as 2026-04-07,04:30
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=25 cur_time=04:31 sched=NO cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[FTP] Backlog Push: 25 records found. Mode: THRESHOLD_MET

FTP file name is /TWSRF_001951_260407_043000.kwd

SampleNo  is 79

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Huge Backlog! Records chunked for TX. Original queue protected until success.
Retrieved file is /TWSRF_001951_260407_043000.kwd
Initializing A7672S for FTP...
[FTP] Airtel/Jio detected. Smart Default: Passive (1).

OK


ERROR

ERROR

+CGEV: ME PDN ACT 1,4

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
[FTP] FSDEL path: TWSRF_001951_260407_043000.kwd
[FTP] FSOPEN cmd: AT+FSOPEN="C:/TWSRF_001951_260407_043000.kwd",0
[FTP] Found FH: 1
[FTP] Sending FSWRITE command...
[FTP] Data stream finished.
[FTP] Dispatching PUTFILE command...
[FTP] Dispatching PUTFILE command...
Response of AT+CFTPSPUTFILE: 
ERROR

[FTP] PUT failed. Re-staging and retrying in mode (0)...

ERROR


OK

+CFTPSSTART: 0

[FTP] Configuring mode (0)...

OK

[FTP] Warming up DNS...
[FTP] DNS Resolved: 89.32.144.163

OK

+CFTPSLOGIN: 9

Result is 9
FTP Login unsuccessful
[FTP] Race Guard: Logout too fast (Cleanup)? Waiting 1s drain.
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=4:33:56
 Sleep=11:16 (min:sec)
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
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.80 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.15V | Solar: 0.00V



[RTC] Time: 04:45
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: TWSRF_001951_260407_041500.kwd
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260407.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: TWSRF_001951_260407_043000.kwd
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
[GPRS] Pre-setting APN for CID 1: airtelgprs.com (IP)
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:2)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Memory Check - Stored CCID: 89914509006871527866 | Stored APN: airteliot.com
Smart APN: Match Found!
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
CGACT Resp: OK
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
SPIFFS: 31/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 80
RF Close date from RTC = 2026-04-07  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :79,2026-04-07,04:30,000.00,000.0,000.0,00.0,000,-069,04.13

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 4:30

NO GAPS FOUND ...


Current data inserted is 80,2026-04-07,04:45,000.00,000.0,000.0,00.0,000,-079,04.13


append_text->store_text : Used for internal status: 80,2026-04-07,04:45,000.00,000.0,000.0,00.0,000,-079,04.13


append_text written to lastrecorded_<stationname>.txt is : 80,2026-04-07,04:45,000.00,000.0,000.0,00.0,000,-079,04.13



[FILE] SPIFFS: /001951_20260407.txt | Size: 4860
   ... [Tail Content] ...
00,-111,04.13
76,2026-04-07,03:45,000.00,000.0,000.0,00.0,000,-111,04.13
77,2026-04-07,04:00,000.00,000.0,000.0,00.0,000,-111,04.15
78,2026-04-07,04:15,000.00,000.0,000.0,00.0,000,-075,04.13
79,2026-04-07,04:30,000.00,000.0,000.0,00.0,000,-069,04.13
80,2026-04-07,04:45,000.00,000.0,000.0,00.0,000,-079,04.13
-----------------------

[FILE] SD: /001951_20260407.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
51;2026-04-07,02:15;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,02:30;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,02:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:00;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:15;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:30;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11460 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
****RTC read error 0 (Try 1/3)
[RTC] Time: 04:46
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).
[GPRS] Bearer status check: Re-initializing IP stack...
[GPRS] Starting HTTP...
HTTP POST REQUEST IS AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260407.txt

Current Data to be sent is : 79,2026-04-07,04:30,000.00,000.0,000.0,00.0,000,-069,04.13
http_data format is stn_no=001951&rec_time=2026-04-07,04:45&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.13&bat_volt2=04.13

Payload is stn_no=001951&rec_time=2026-04-07,04:45&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.13&bat_volt2=04.13
[HTTP] Using Fast v3.0 Handshake...
Response of AT+HTTPACTION=1 is 
OK

+HTTPACTION: 1,713,0

HTTP Error (706/713/714). Clean stack requested.
[HTTP] 1st Attempt (Fast) failed. Retrying in 2s (Fast Attempt 2)...
Payload is stn_no=001951&rec_time=2026-04-07,04:45&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.13&bat_volt2=04.13
[HTTP] Using Fast v3.0 Handshake...
****RTC read error 0 (Try 1/3)
Response of AT+HTTPACTION=1 is 
ERROR

[HTTP] HTTPACTION timed out — no URC received from modem.
[HTTP] 2nd Attempt (Fast) also failed. Falling back to Robust method...
Payload is stn_no=001951&rec_time=2026-04-07,04:45&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.13&bat_volt2=04.13
[HTTP] Using Robust Handshake (Wait for DOWNLOAD)...
[RTC] Time: 04:47
[HTTP] AT+HTTPDATA failed (Missing DOWNLOAD).
[HTTP] Attempt 1 failed. Re-initialising stack for single retry...
[HTTP] TCP Zombie (706/714) detected. Executing Hard Bearer Nuke...
[RECOVERY] Bearer failed to recover. Skipping retry.
[HTTP] Monthly cum fail counter reset (New Month detected).
[RECOVERY] Consec HTTP Fails: 1 | Present: 1 | CumMth: 1
CURRENT DATA : Retries exceeded limit... Storing to backlog.

ftpappend_text is : 001951;2026-04-07,04:45;00.00;000.0;000.0;00.0;000;-079;04.13

Record written into /ftpunsent.txt is as below : 
001951;2026-04-07,04:45;00.00;000.0;000.0;00.0;000;-079;04.13

*** Current data couldn't be sent. Backlog will be handled independently.
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=26 cur_time=04:48 sched=NO cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[FTP] Backlog Push: 26 records found. Mode: THRESHOLD_MET

FTP file name is /TWSRF_001951_260407_044500.kwd

SampleNo  is 80

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Huge Backlog! Records chunked for TX. Original queue protected until success.
Retrieved file is /TWSRF_001951_260407_044500.kwd
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
[FTP] FSDEL path: TWSRF_001951_260407_044500.kwd
[FTP] FSOPEN cmd: AT+FSOPEN="C:/TWSRF_001951_260407_044500.kwd",0
[FTP] Found FH: 1
[FTP] Sending FSWRITE command...
[FTP] Data stream finished.
[FTP] Dispatching PUTFILE command...
[FTP] Dispatching PUTFILE command...
Response of AT+CFTPSPUTFILE: 
ERROR

[FTP] PUT failed. Re-staging and retrying in mode (0)...

ERROR


OK

+CFTPSSTART: 0

[FTP] Configuring mode (0)...

OK

[FTP] Warming up DNS...
[FTP] DNS Resolved: 89.32.144.163

OK

+CFTPSLOGIN: 9

Result is 9
FTP Login unsuccessful
[FTP] Race Guard: Logout too fast (Cleanup)? Waiting 1s drain.
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[RTC] Time: 04:50
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=4:50:10
 Sleep=10:5 (min:sec)
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
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.80 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.15V | Solar: 0.00V



[RTC] Time: 05:00
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: TWSRF_001951_260407_041500.kwd
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: TWSRF_001951_260407_043000.kwd
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260407.txt
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: TWSRF_001951_260407_044500.kwd
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
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:3)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Memory Check - Stored CCID: 89914509006871527866 | Stored APN: airteliot.com
Smart APN: Match Found!
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
CGACT Resp: OK
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
SPIFFS: 32/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 81
RF Close date from RTC = 2026-04-07  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :80,2026-04-07,04:45,000.00,000.0,000.0,00.0,000,-079,04.13

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 4:45

NO GAPS FOUND ...


Current data inserted is 81,2026-04-07,05:00,000.00,000.0,000.0,00.0,000,-081,04.13


append_text->store_text : Used for internal status: 81,2026-04-07,05:00,000.00,000.0,000.0,00.0,000,-081,04.13


append_text written to lastrecorded_<stationname>.txt is : 81,2026-04-07,05:00,000.00,000.0,000.0,00.0,000,-081,04.13



[FILE] SPIFFS: /001951_20260407.txt | Size: 4920
   ... [Tail Content] ...
00,-111,04.13
77,2026-04-07,04:00,000.00,000.0,000.0,00.0,000,-111,04.15
78,2026-04-07,04:15,000.00,000.0,000.0,00.0,000,-075,04.13
79,2026-04-07,04:30,000.00,000.0,000.0,00.0,000,-069,04.13
80,2026-04-07,04:45,000.00,000.0,000.0,00.0,000,-079,04.13
81,2026-04-07,05:00,000.00,000.0,000.0,00.0,000,-081,04.13
-----------------------

[FILE] SD: /001951_20260407.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
51;2026-04-07,02:30;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,02:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:00;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:15;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:30;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001951;2026-04-07,04:45;00.00;000.0;000.0;00.0;000;-079;04.13
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11544 bytes free
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
[RTC] Time: 05:02
[GPRS] HTTPINIT Failed. Trying TERM then INIT...

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260407.txt

Current Data to be sent is : 80,2026-04-07,04:45,000.00,000.0,000.0,00.0,000,-079,04.13
http_data format is stn_no=001951&rec_time=2026-04-07,05:00&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-081&bat_volt=04.13&bat_volt2=04.13

[HTTP] HTTP session not ready. Fast-fail to backlog.
[HTTP] 1st Attempt (Fast) failed. Retrying in 2s (Fast Attempt 2)...
[HTTP] HTTP session not ready. Fast-fail to backlog.
[HTTP] 2nd Attempt (Fast) also failed. Falling back to Robust method...
[HTTP] HTTP session not ready. Fast-fail to backlog.
[HTTP] Attempt 1 failed. Re-initialising stack for single retry...
[HTTP] Retry attempt...
Payload is stn_no=001951&rec_time=2026-04-07,05:00&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-081&bat_volt=04.13&bat_volt2=04.13
[HTTP] Using Robust Handshake (Wait for DOWNLOAD)...
****RTC read error 0 (Try 1/3)
****RTC read error 0 (Try 1/3)
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
[RTC] Time: 05:03
[HTTP] Retry succeeded.
[PWR] Signature persisted successfully.

**** Storing Last Logged Data as 2026-04-07,05:00
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=26 cur_time=05:03 sched=NO cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[FTP] Backlog Push: 26 records found. Mode: THRESHOLD_MET

FTP file name is /TWSRF_001951_260407_050000.kwd

SampleNo  is 81

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Huge Backlog! Records chunked for TX. Original queue protected until success.
Retrieved file is /TWSRF_001951_260407_050000.kwd
Initializing A7672S for FTP...
[FTP] Airtel/Jio detected. Smart Default: Passive (1).

ERROR


OK

+CFTPSSTART: 0

[RTC] Time: 05:04
[FTP] Configuring mode (1)...

OK

[FTP] Warming up DNS...
[FTP] DNS Failed. Switching to Insurance IP.
****RTC read error 0 (Try 1/3)

+CDNSGIP: 0,10

ERROR

OK

+CFTPSLOGIN: 9

Result is 9
FTP Login unsuccessful
[FTP] First Login attempt failed. BSNL Shield: Forcing Context Refresh...
****RTC read error 4 (Try 1/3)

ERROR

+CGEV: ME PDN ACT 1,4


OK

+CFTPSSTART: 0

[FTP] Configuring mode (1)...

OK

[FTP] Warming up DNS...
[FTP] DNS Resolved: 89.32.144.163
[RTC] Time: 05:05

OK

+CFTPSLOGIN: 9

Result is 9
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
[PWR] Sleep: CurTime=5:5:29
 Sleep=9:46 (min:sec)
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
[FS] Emergency Boot Purge: TWSRF_001951_260407_050000.kwd
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.80 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.15V | Solar: 0.00V



[RTC] Time: 05:15
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: TWSRF_001951_260407_041500.kwd
SPIFFS [INIT]: TWSRF_001951_260407_043000.kwd
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: TWSRF_001951_260407_044500.kwd
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260407.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: ftpremain.txt
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
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:4)
****RTC read error 0 (Try 1/3)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Memory Check - Stored CCID: 89914509006871527866 | Stored APN: airteliot.com
Smart APN: Match Found!
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
CGACT Resp: OK
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
SPIFFS: 33/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 82
RF Close date from RTC = 2026-04-07  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :81,2026-04-07,05:00,000.00,000.0,000.0,00.0,000,-081,04.13

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 5:0

NO GAPS FOUND ...


Current data inserted is 82,2026-04-07,05:15,000.00,000.0,000.0,00.0,000,-079,04.13


append_text->store_text : Used for internal status: 82,2026-04-07,05:15,000.00,000.0,000.0,00.0,000,-079,04.13


append_text written to lastrecorded_<stationname>.txt is : 82,2026-04-07,05:15,000.00,000.0,000.0,00.0,000,-079,04.13



[FILE] SPIFFS: /001951_20260407.txt | Size: 4980
   ... [Tail Content] ...
00,-111,04.15
78,2026-04-07,04:15,000.00,000.0,000.0,00.0,000,-075,04.13
79,2026-04-07,04:30,000.00,000.0,000.0,00.0,000,-069,04.13
80,2026-04-07,04:45,000.00,000.0,000.0,00.0,000,-079,04.13
81,2026-04-07,05:00,000.00,000.0,000.0,00.0,000,-081,04.13
82,2026-04-07,05:15,000.00,000.0,000.0,00.0,000,-079,04.13
-----------------------

[FILE] SD: /001951_20260407.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
51;2026-04-07,02:30;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,02:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:00;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:15;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:30;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001951;2026-04-07,04:45;00.00;000.0;000.0;00.0;000;-079;04.13
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11656 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[RTC] Time: 05:16
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).
[GPRS] Bearer status check: Re-initializing IP stack...
[GPRS] Starting HTTP...
HTTP POST REQUEST IS AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260407.txt

Current Data to be sent is : 81,2026-04-07,05:00,000.00,000.0,000.0,00.0,000,-081,04.13
http_data format is stn_no=001951&rec_time=2026-04-07,05:15&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.13&bat_volt2=04.13

Payload is stn_no=001951&rec_time=2026-04-07,05:15&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.13&bat_volt2=04.13
[HTTP] Using Fast v3.0 Handshake...
****RTC read error 4 (Try 1/3)
****RTC read error 0 (Try 2/3)
Response of AT+HTTPACTION=1 is 
OK

[HTTP] HTTPACTION timed out — no URC received from modem.
[HTTP] 1st Attempt (Fast) failed. Retrying in 2s (Fast Attempt 2)...
Payload is stn_no=001951&rec_time=2026-04-07,05:15&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.13&bat_volt2=04.13
[HTTP] Using Fast v3.0 Handshake...
Response of AT+HTTPACTION=1 is 
OK

+HTTPACTION: 1,714,0

HTTP Error (706/713/714). Clean stack requested.
[HTTP] 2nd Attempt (Fast) also failed. Falling back to Robust method...
Payload is stn_no=001951&rec_time=2026-04-07,05:15&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.13&bat_volt2=04.13
[HTTP] Using Robust Handshake (Wait for DOWNLOAD)...
[RTC] Time: 05:17
[HTTP] AT+HTTPDATA failed (Missing DOWNLOAD).
[HTTP] Attempt 1 failed. Re-initialising stack for single retry...
[HTTP] TCP Zombie (706/714) detected. Executing Hard Bearer Nuke...
[HTTP] Retry attempt...
Payload is stn_no=001951&rec_time=2026-04-07,05:15&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.13&bat_volt2=04.13
[HTTP] Using Robust Handshake (Wait for DOWNLOAD)...
****RTC read error 0 (Try 1/3)
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
[HTTP] Retry succeeded.
[PWR] Signature persisted successfully.

**** Storing Last Logged Data as 2026-04-07,05:15
[RTC] Time: 05:18
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=26 cur_time=05:18 sched=NO cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[FTP] Backlog Push: 26 records found. Mode: THRESHOLD_MET

FTP file name is /TWSRF_001951_260407_051500.kwd

SampleNo  is 82

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Huge Backlog! Records chunked for TX. Original queue protected until success.
Retrieved file is /TWSRF_001951_260407_051500.kwd
Initializing A7672S for FTP...
[FTP] Airtel/Jio detected. Smart Default: Passive (1).

ERROR


OK

+CFTPSSTART: 0

[FTP] Configuring mode (1)...

OK

[FTP] Warming up DNS...
[FTP] DNS Resolved: 27.34.245.70
[RTC] Time: 05:19

OK

+CFTPSLOGIN: 9

Result is 9
FTP Login unsuccessful
[FTP] First Login attempt failed. BSNL Shield: Forcing Context Refresh...

OK


ERROR

+CGEV: ME PDN ACT 1,4

OK

+CFTPSSTART: 0

[FTP] Configuring mode (1)...

OK

[FTP] Warming up DNS...
[FTP] DNS Resolved: 89.32.144.163

OK

+CFTPSLOGIN: 9

Result is 9
FTP Login unsuccessful
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 05:20
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=5:20:2
 Sleep=10:13 (min:sec)
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
[FS] Emergency Boot Purge: TWSRF_001951_260407_051500.kwd
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.80 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.14V | Solar: 0.00V



[RTC] Time: 05:30
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: TWSRF_001951_260407_041500.kwd
SPIFFS [INIT]: TWSRF_001951_260407_043000.kwd
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: TWSRF_001951_260407_044500.kwd
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260407.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: ftpremain.txt
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
Signal strength IN gprs task is 85
Signal Level is -111
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
[CACHE] Using cached carrier/number to save power.

************************
GETTING REGISTRATION 
************************
[GPRS] APN already set in CGDCONT. Bypassing flash write.
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:5)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Memory Check - Stored CCID: 89914509006871527866 | Stored APN: airteliot.com
Smart APN: Match Found!
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
CGACT Resp: OK
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
SPIFFS: 33/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 83
RF Close date from RTC = 2026-04-07  
[RTC] Time: 05:31

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :82,2026-04-07,05:15,000.00,000.0,000.0,00.0,000,-079,04.13

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 5:15

NO GAPS FOUND ...


Current data inserted is 83,2026-04-07,05:30,000.00,000.0,000.0,00.0,000,-081,04.13


append_text->store_text : Used for internal status: 83,2026-04-07,05:30,000.00,000.0,000.0,00.0,000,-081,04.13


append_text written to lastrecorded_<stationname>.txt is : 83,2026-04-07,05:30,000.00,000.0,000.0,00.0,000,-081,04.13



[FILE] SPIFFS: /001951_20260407.txt | Size: 5040
   ... [Tail Content] ...
00,-075,04.13
79,2026-04-07,04:30,000.00,000.0,000.0,00.0,000,-069,04.13
80,2026-04-07,04:45,000.00,000.0,000.0,00.0,000,-079,04.13
81,2026-04-07,05:00,000.00,000.0,000.0,00.0,000,-081,04.13
82,2026-04-07,05:15,000.00,000.0,000.0,00.0,000,-079,04.13
83,2026-04-07,05:30,000.00,000.0,000.0,00.0,000,-081,04.13
-----------------------

[FILE] SD: /001951_20260407.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
51;2026-04-07,02:30;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,02:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:00;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:15;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:30;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001951;2026-04-07,04:45;00.00;000.0;000.0;00.0;000;-079;04.13
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11656 bytes free
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

SPIFF FILE EXISTS ..../001951_20260407.txt

Current Data to be sent is : 82,2026-04-07,05:15,000.00,000.0,000.0,00.0,000,-079,04.13
http_data format is stn_no=001951&rec_time=2026-04-07,05:30&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-081&bat_volt=04.13&bat_volt2=04.13

Payload is stn_no=001951&rec_time=2026-04-07,05:30&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-081&bat_volt=04.13&bat_volt2=04.13
[HTTP] Using Fast v3.0 Handshake...
Response of AT+HTTPACTION=1 is 
OK

+HTTPACTION: 1,713,0

HTTP Error (706/713/714). Clean stack requested.
[HTTP] 1st Attempt (Fast) failed. Retrying in 2s (Fast Attempt 2)...
Payload is stn_no=001951&rec_time=2026-04-07,05:30&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-081&bat_volt=04.13&bat_volt2=04.13
[HTTP] Using Fast v3.0 Handshake...
[RTC] Time: 05:32
Response of AT+HTTPACTION=1 is 
ERROR

[HTTP] HTTPACTION timed out — no URC received from modem.
[HTTP] 2nd Attempt (Fast) also failed. Falling back to Robust method...
Payload is stn_no=001951&rec_time=2026-04-07,05:30&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-081&bat_volt=04.13&bat_volt2=04.13
[HTTP] Using Robust Handshake (Wait for DOWNLOAD)...
[HTTP] AT+HTTPDATA failed (Missing DOWNLOAD).
[HTTP] Attempt 1 failed. Re-initialising stack for single retry...
[HTTP] TCP Zombie (706/714) detected. Executing Hard Bearer Nuke...
****RTC read error 4 (Try 1/3)
****RTC read error 0 (Try 2/3)
[HTTP] Retry attempt...
Payload is stn_no=001951&rec_time=2026-04-07,05:30&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-081&bat_volt=04.13&bat_volt2=04.13
[HTTP] Using Robust Handshake (Wait for DOWNLOAD)...
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
[RTC] Time: 05:33
[HTTP] Retry succeeded.
[PWR] Signature persisted successfully.

**** Storing Last Logged Data as 2026-04-07,05:30
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=26 cur_time=05:33 sched=NO cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[FTP] Backlog Push: 26 records found. Mode: THRESHOLD_MET

FTP file name is /TWSRF_001951_260407_053000.kwd

SampleNo  is 83

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Huge Backlog! Records chunked for TX. Original queue protected until success.
Retrieved file is /TWSRF_001951_260407_053000.kwd
Initializing A7672S for FTP...
[FTP] Airtel/Jio detected. Smart Default: Passive (1).
****RTC read error 4 (Try 1/3)

ERROR


OK

+CFTPSSTART: 0

[FTP] Configuring mode (1)...

OK

[FTP] Warming up DNS...
[RTC] Time: 05:34
[FTP] DNS Resolved: 27.34.245.70
****RTC read error 4 (Try 1/3)

OK

+CFTPSLOGIN: 0

Result is 0
FTP Login success
File size: 945
[FTP] FSDEL path: TWSRF_001951_260407_053000.kwd
[FTP] FSOPEN cmd: AT+FSOPEN="C:/TWSRF_001951_260407_053000.kwd",0
[FTP] Found FH: 1
[FTP] Sending FSWRITE command...
[FTP] Data stream finished.
[FTP] Dispatching PUTFILE command...
[FTP] Dispatching PUTFILE command...
Response of AT+CFTPSPUTFILE: 
ERROR

[FTP] PUT failed. Re-staging and retrying in mode (0)...

ERROR


OK

+CFTPSSTART: 0

[FTP] Configuring mode (0)...

OK

[FTP] Warming up DNS...
[FTP] DNS Resolved: 89.32.144.163

OK

+CFTPSLOGIN: 9

Result is 9
FTP Login unsuccessful
[FTP] Race Guard: Logout too fast (Cleanup)? Waiting 1s drain.
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=5:35:53
 Sleep=9:20 (min:sec)
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
[FS] Emergency Boot Purge: TWSRF_001951_260407_053000.kwd
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.80 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.14V | Solar: 0.00V



[RTC] Time: 05:45
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: TWSRF_001951_260407_041500.kwd
SPIFFS [INIT]: TWSRF_001951_260407_043000.kwd
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: TWSRF_001951_260407_044500.kwd
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260407.txt
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
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -69

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
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:6)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Memory Check - Stored CCID: 89914509006871527866 | Stored APN: airteliot.com
Smart APN: Match Found!
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
CGACT Resp: OK
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
SPIFFS: 33/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 84
RF Close date from RTC = 2026-04-07  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :83,2026-04-07,05:30,000.00,000.0,000.0,00.0,000,-081,04.13

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 5:30

NO GAPS FOUND ...


Current data inserted is 84,2026-04-07,05:45,000.00,000.0,000.0,00.0,000,-069,04.12


append_text->store_text : Used for internal status: 84,2026-04-07,05:45,000.00,000.0,000.0,00.0,000,-069,04.12


append_text written to lastrecorded_<stationname>.txt is : 84,2026-04-07,05:45,000.00,000.0,000.0,00.0,000,-069,04.12



[FILE] SPIFFS: /001951_20260407.txt | Size: 5100
   ... [Tail Content] ...
00,-069,04.13
80,2026-04-07,04:45,000.00,000.0,000.0,00.0,000,-079,04.13
81,2026-04-07,05:00,000.00,000.0,000.0,00.0,000,-081,04.13
82,2026-04-07,05:15,000.00,000.0,000.0,00.0,000,-079,04.13
83,2026-04-07,05:30,000.00,000.0,000.0,00.0,000,-081,04.13
84,2026-04-07,05:45,000.00,000.0,000.0,00.0,000,-069,04.12
-----------------------

[FILE] SD: /001951_20260407.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
51;2026-04-07,02:30;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,02:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:00;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:15;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:30;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001951;2026-04-07,04:45;00.00;000.0;000.0;00.0;000;-079;04.13
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11544 bytes free
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
[RTC] Time: 05:46

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260407.txt

Current Data to be sent is : 83,2026-04-07,05:30,000.00,000.0,000.0,00.0,000,-081,04.13
http_data format is stn_no=001951&rec_time=2026-04-07,05:45&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.13&bat_volt2=04.13

Payload is stn_no=001951&rec_time=2026-04-07,05:45&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.13&bat_volt2=04.13
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

**** Storing Last Logged Data as 2026-04-07,05:45
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=26 cur_time=05:46 sched=NO cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[FTP] Backlog Push: 26 records found. Mode: THRESHOLD_MET

FTP file name is /TWSRF_001951_260407_054500.kwd

SampleNo  is 84

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Huge Backlog! Records chunked for TX. Original queue protected until success.
Retrieved file is /TWSRF_001951_260407_054500.kwd
Initializing A7672S for FTP...
[FTP] Airtel/Jio detected. Smart Default: Passive (1).

ERROR


OK

+CFTPSSTART: 0

[FTP] Configuring mode (1)...

OK

[FTP] Warming up DNS...
****RTC read error 4 (Try 1/3)
****RTC read error 4 (Try 2/3)
[RTC] Time: 05:47
[FTP] DNS Resolved: 27.34.245.70

OK

+CFTPSLOGIN: 0

Result is 0
FTP Login success
File size: 945
[FTP] FSDEL path: TWSRF_001951_260407_054500.kwd
[FTP] FSOPEN cmd: AT+FSOPEN="C:/TWSRF_001951_260407_054500.kwd",0
[FTP] Found FH: 1
[FTP] Sending FSWRITE command...
[FTP] Data stream finished.
[FTP] Dispatching PUTFILE command...
[FTP] Dispatching PUTFILE command...
Response of AT+CFTPSPUTFILE: 
ERROR

[FTP] PUT failed. Re-staging and retrying in mode (0)...

ERROR


OK

+CFTPSSTART: 0

[FTP] Configuring mode (0)...

OK

[FTP] Warming up DNS...
[FTP] DNS Resolved: 89.32.144.163

OK

+CFTPSLOGIN: 9

Result is 9
FTP Login unsuccessful
[FTP] Race Guard: Logout too fast (Cleanup)? Waiting 1s drain.
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[RTC] Time: 05:48
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=5:48:6
 Sleep=12:6 (min:sec)
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
[FS] Emergency Boot Purge: TWSRF_001951_260407_054500.kwd
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.80 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.14V | Solar: 0.00V



[RTC] Time: 06:00
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: TWSRF_001951_260407_041500.kwd
SPIFFS [INIT]: TWSRF_001951_260407_043000.kwd
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: TWSRF_001951_260407_044500.kwd
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260407.txt
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
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:7)
****RTC read error 4 (Try 1/3)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Memory Check - Stored CCID: 89914509006871527866 | Stored APN: airteliot.com
Smart APN: Match Found!
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
CGACT Resp: OK
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
SPIFFS: 33/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 85
RF Close date from RTC = 2026-04-07  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :84,2026-04-07,05:45,000.00,000.0,000.0,00.0,000,-069,04.12

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 5:45

NO GAPS FOUND ...


Current data inserted is 85,2026-04-07,06:00,000.00,000.0,000.0,00.0,000,-079,04.13


append_text->store_text : Used for internal status: 85,2026-04-07,06:00,000.00,000.0,000.0,00.0,000,-079,04.13


append_text written to lastrecorded_<stationname>.txt is : 85,2026-04-07,06:00,000.00,000.0,000.0,00.0,000,-079,04.13



[FILE] SPIFFS: /001951_20260407.txt | Size: 5160
   ... [Tail Content] ...
00,-079,04.13
81,2026-04-07,05:00,000.00,000.0,000.0,00.0,000,-081,04.13
82,2026-04-07,05:15,000.00,000.0,000.0,00.0,000,-079,04.13
83,2026-04-07,05:30,000.00,000.0,000.0,00.0,000,-081,04.13
84,2026-04-07,05:45,000.00,000.0,000.0,00.0,000,-069,04.12
85,2026-04-07,06:00,000.00,000.0,000.0,00.0,000,-079,04.13
-----------------------

[FILE] SD: /001951_20260407.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
51;2026-04-07,02:30;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,02:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:00;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:15;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:30;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001951;2026-04-07,04:45;00.00;000.0;000.0;00.0;000;-079;04.13
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11656 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[RTC] Time: 06:01
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).
[GPRS] Bearer status check: Re-initializing IP stack...
[GPRS] Starting HTTP...
HTTP POST REQUEST IS AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260407.txt

Current Data to be sent is : 84,2026-04-07,05:45,000.00,000.0,000.0,00.0,000,-069,04.12
http_data format is stn_no=001951&rec_time=2026-04-07,06:00&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.12&bat_volt2=04.12

Payload is stn_no=001951&rec_time=2026-04-07,06:00&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.12&bat_volt2=04.12
[HTTP] Using Fast v3.0 Handshake...
****RTC read error 0 (Try 1/3)
Response of AT+HTTPACTION=1 is 
OK

+HTTPACTION: 1,713,0

HTTP Error (706/713/714). Clean stack requested.
[HTTP] 1st Attempt (Fast) failed. Retrying in 2s (Fast Attempt 2)...
Payload is stn_no=001951&rec_time=2026-04-07,06:00&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.12&bat_volt2=04.12
[HTTP] Using Fast v3.0 Handshake...
****RTC read error 0 (Try 1/3)
Response of AT+HTTPACTION=1 is 
ERROR

[HTTP] HTTPACTION timed out — no URC received from modem.
[HTTP] 2nd Attempt (Fast) also failed. Falling back to Robust method...
Payload is stn_no=001951&rec_time=2026-04-07,06:00&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.12&bat_volt2=04.12
[HTTP] Using Robust Handshake (Wait for DOWNLOAD)...
[RTC] Time: 06:02
[HTTP] AT+HTTPDATA failed (Missing DOWNLOAD).
[HTTP] Attempt 1 failed. Re-initialising stack for single retry...
[HTTP] TCP Zombie (706/714) detected. Executing Hard Bearer Nuke...
[HTTP] Retry attempt...
Payload is stn_no=001951&rec_time=2026-04-07,06:00&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.12&bat_volt2=04.12
[HTTP] Using Robust Handshake (Wait for DOWNLOAD)...
Response of AT+HTTPACTION=1 is 
OK

+HTTPACTION: 1,713,0

HTTP Error (706/713/714). Clean stack requested.
[DNS] Cleared fallback cache due to TCP zombie failure code.
[RECOVERY] Consec HTTP Fails: 1 | Present: 1 | CumMth: 2
CURRENT DATA : Retries exceeded limit... Storing to backlog.

ftpappend_text is : 001951;2026-04-07,06:00;00.00;000.0;000.0;00.0;000;-079;04.12

Record written into /ftpunsent.txt is as below : 
001951;2026-04-07,06:00;00.00;000.0;000.0;00.0;000;-079;04.12

*** Current data couldn't be sent. Backlog will be handled independently.
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=27 cur_time=06:02 sched=YES cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[FTP] Backlog Push: 27 records found. Mode: SCHEDULED_SLOT

FTP file name is /TWSRF_001951_260407_060000.kwd

SampleNo  is 85

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Huge Backlog! Records chunked for TX. Original queue protected until success.
Retrieved file is /TWSRF_001951_260407_060000.kwd
Initializing A7672S for FTP...
[FTP] Airtel/Jio detected. Smart Default: Passive (1).

ERROR


OK

+CFTPSSTART: 0

[FTP] Configuring mode (1)...

OK

[FTP] Warming up DNS...
[FTP] DNS Failed. Switching to Insurance IP.
****RTC read error 4 (Try 1/3)
****RTC read error 0 (Try 2/3)
[RTC] Time: 06:04
****RTC read error 4 (Try 1/3)

+CDNSGIP: 0,10

ERROR

OK

+CFTPSLOGIN: 9

Result is 9
FTP Login unsuccessful
[FTP] First Login attempt failed. BSNL Shield: Forcing Context Refresh...

****RTC read error 4 (Try 1/3)
****RTC read error 0 (Try 2/3)

OK

ERROR

+CGEV: ME PDN ACT 1,4

OK

+CFTPSSTART: 0

[FTP] Configuring mode (1)...

OK

[FTP] Warming up DNS...
[FTP] DNS Resolved: 89.32.144.163
[RTC] Time: 06:05
****RTC read error 0 (Try 1/3)
****RTC read error 0 (Try 1/3)

OK

+CFTPSLOGIN: 9

Result is 9
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
[PWR] Sleep: CurTime=6:5:43
 Sleep=9:30 (min:sec)
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
[FS] Emergency Boot Purge: TWSRF_001951_260407_060000.kwd
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.80 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.14V | Solar: 0.00V



[RTC] Time: 06:15
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: TWSRF_001951_260407_041500.kwd
SPIFFS [INIT]: TWSRF_001951_260407_043000.kwd
SPIFFS [INIT]: TWSRF_001951_260407_044500.kwd
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260407.txt
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: ftpremain.txt
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
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:8)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Memory Check - Stored CCID: 89914509006871527866 | Stored APN: airteliot.com
Smart APN: Match Found!
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
CGACT Resp: OK
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
SPIFFS: 34/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 86
RF Close date from RTC = 2026-04-07  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :85,2026-04-07,06:00,000.00,000.0,000.0,00.0,000,-079,04.13

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 6:0

NO GAPS FOUND ...


Current data inserted is 86,2026-04-07,06:15,000.00,000.0,000.0,00.0,000,-079,04.12


append_text->store_text : Used for internal status: 86,2026-04-07,06:15,000.00,000.0,000.0,00.0,000,-079,04.12


append_text written to lastrecorded_<stationname>.txt is : 86,2026-04-07,06:15,000.00,000.0,000.0,00.0,000,-079,04.12



[FILE] SPIFFS: /001951_20260407.txt | Size: 5220
   ... [Tail Content] ...
00,-081,04.13
82,2026-04-07,05:15,000.00,000.0,000.0,00.0,000,-079,04.13
83,2026-04-07,05:30,000.00,000.0,000.0,00.0,000,-081,04.13
84,2026-04-07,05:45,000.00,000.0,000.0,00.0,000,-069,04.12
85,2026-04-07,06:00,000.00,000.0,000.0,00.0,000,-079,04.13
86,2026-04-07,06:15,000.00,000.0,000.0,00.0,000,-079,04.12
-----------------------

[FILE] SD: /001951_20260407.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
51;2026-04-07,02:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:00;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:15;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:30;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001951;2026-04-07,04:45;00.00;000.0;000.0;00.0;000;-079;04.13
001951;2026-04-07,06:00;00.00;000.0;000.0;00.0;000;-079;04.12
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11544 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[RTC] Time: 06:16
[GPRS] Ghost PDP (0.0.0.0). Triggering recovery...
[GPRS] Bearer status check: Re-initializing IP stack...
[GPRS] Starting HTTP...
HTTP POST REQUEST IS AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260407.txt

Current Data to be sent is : 85,2026-04-07,06:00,000.00,000.0,000.0,00.0,000,-079,04.13
http_data format is stn_no=001951&rec_time=2026-04-07,06:15&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.13&bat_volt2=04.13

Payload is stn_no=001951&rec_time=2026-04-07,06:15&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.13&bat_volt2=04.13
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

**** Storing Last Logged Data as 2026-04-07,06:15
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=27 cur_time=06:16 sched=NO cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[FTP] Backlog Push: 27 records found. Mode: THRESHOLD_MET

FTP file name is /TWSRF_001951_260407_061500.kwd

SampleNo  is 86

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Huge Backlog! Records chunked for TX. Original queue protected until success.
Retrieved file is /TWSRF_001951_260407_061500.kwd
Initializing A7672S for FTP...
[FTP] Airtel/Jio detected. Smart Default: Passive (1).

ERROR


+CGEV: ME PDN ACT 1,4

OK

+CFTPSSTART: 0

[FTP] Configuring mode (1)...

OK

[FTP] Warming up DNS...
[FTP] DNS Failed. Switching to Insurance IP.
[RTC] Time: 06:18

OK

+CFTPSLOGIN: 0

Result is 0
FTP Login success
File size: 945
[FTP] FSDEL path: TWSRF_001951_260407_061500.kwd
[FTP] FSOPEN cmd: AT+FSOPEN="C:/TWSRF_001951_260407_061500.kwd",0
[FTP] Found FH: 1
[FTP] Sending FSWRITE command...
[FTP] Data stream finished.
[FTP] Dispatching PUTFILE command...
[FTP] Dispatching PUTFILE command...
Response of AT+CFTPSPUTFILE: 
ERROR

[FTP] PUT failed. Re-staging and retrying in mode (0)...

ERROR


OK

+CFTPSSTART: 0

[FTP] Configuring mode (0)...

OK

[FTP] Warming up DNS...
[FTP] DNS Resolved: 89.32.144.163

OK

+CFTPSLOGIN: 9

Result is 9
FTP Login unsuccessful
[FTP] Race Guard: Logout too fast (Cleanup)? Waiting 1s drain.
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 06:19
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=6:19:7
 Sleep=11:6 (min:sec)
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
[FS] Emergency Boot Purge: TWSRF_001951_260407_061500.kwd
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.80 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.14V | Solar: 0.00V



[RTC] Time: 06:30
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: TWSRF_001951_260407_041500.kwd
SPIFFS [INIT]: TWSRF_001951_260407_043000.kwd
SPIFFS [INIT]: TWSRF_001951_260407_044500.kwd
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260407.txt
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
Signal strength IN gprs task is -69

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
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:9)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Memory Check - Stored CCID: 89914509006871527866 | Stored APN: airteliot.com
Smart APN: Match Found!
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
CGACT Resp: OK
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
SPIFFS: 33/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 87
RF Close date from RTC = 2026-04-07  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :86,2026-04-07,06:15,000.00,000.0,000.0,00.0,000,-079,04.12

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 6:15

NO GAPS FOUND ...


Current data inserted is 87,2026-04-07,06:30,000.00,000.0,000.0,00.0,000,-069,04.12


append_text->store_text : Used for internal status: 87,2026-04-07,06:30,000.00,000.0,000.0,00.0,000,-069,04.12

[RTC] Time: 06:31

append_text written to lastrecorded_<stationname>.txt is : 87,2026-04-07,06:30,000.00,000.0,000.0,00.0,000,-069,04.12



[FILE] SPIFFS: /001951_20260407.txt | Size: 5280
   ... [Tail Content] ...
00,-079,04.13
83,2026-04-07,05:30,000.00,000.0,000.0,00.0,000,-081,04.13
84,2026-04-07,05:45,000.00,000.0,000.0,00.0,000,-069,04.12
85,2026-04-07,06:00,000.00,000.0,000.0,00.0,000,-079,04.13
86,2026-04-07,06:15,000.00,000.0,000.0,00.0,000,-079,04.12
87,2026-04-07,06:30,000.00,000.0,000.0,00.0,000,-069,04.12
-----------------------

[FILE] SD: /001951_20260407.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
51;2026-04-07,02:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:00;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:15;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:30;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001951;2026-04-07,04:45;00.00;000.0;000.0;00.0;000;-079;04.13
001951;2026-04-07,06:00;00.00;000.0;000.0;00.0;000;-079;04.12
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11656 bytes free
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

SPIFF FILE EXISTS ..../001951_20260407.txt

Current Data to be sent is : 86,2026-04-07,06:15,000.00,000.0,000.0,00.0,000,-079,04.12
http_data format is stn_no=001951&rec_time=2026-04-07,06:30&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.12&bat_volt2=04.12

Payload is stn_no=001951&rec_time=2026-04-07,06:30&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.12&bat_volt2=04.12
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

**** Storing Last Logged Data as 2026-04-07,06:30
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=27 cur_time=06:31 sched=NO cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[FTP] Backlog Push: 27 records found. Mode: THRESHOLD_MET

FTP file name is /TWSRF_001951_260407_063000.kwd

SampleNo  is 87

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
****RTC read error 0 (Try 1/3)
[FTP] Huge Backlog! Records chunked for TX. Original queue protected until success.
Retrieved file is /TWSRF_001951_260407_063000.kwd
Initializing A7672S for FTP...
[FTP] Airtel/Jio detected. Smart Default: Passive (1).


****RTC read error 4 (Try 1/3)
****RTC read error 0 (Try 2/3)
[FTP] Configuring mode (1)...
[FTP] Lowercase transmode failed. Trying UPPERCASE...

[FTP] Warming up DNS...
****RTC read error 4 (Try 1/3)
[RTC] Time: 06:34
[FTP] DNS Failed. Switching to Insurance IP.
[RTC] Time: 06:35

Result is 99
FTP Login unsuccessful
[FTP] First Login attempt failed. BSNL Shield: Forcing Context Refresh...
****RTC read error 0 (Try 1/3)

+CME ERROR: AT command timeout

ERROR

ERROR


+CGEV: ME PDN ACT 1,4

OK

+CFTPSSTART: 0

[FTP] Configuring mode (1)...
[FTP] Lowercase transmode failed. Trying UPPERCASE...

[FTP] Warming up DNS...
[FTP] DNS Resolved: 27.34.245.70
****RTC read error 4 (Try 1/3)

OK

+CGEV: ME PDN DEACT 1

+CFTPSLOGIN: 13

+CFTPSNOTIFY: FTPS DISCONNECT

Result is 13
FTP Login unsuccessful
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=6:35:53
 Sleep=9:21 (min:sec)
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
[FS] Emergency Boot Purge: TWSRF_001951_260407_063000.kwd
[BOOT] SD Card: FAILED (Not Inserted/Hardware)
[BOOT] HDC Sensor: NOT FOUND
[BOOT] Rainfall Counter: OK
[BOOT] Hardware Initialized.
[BOOT] Preserving counters for reset reason: 5
Health Mode: DAILY (11am)
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.80 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.14V | Solar: 0.00V



[RTC] Time: 06:45
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: TWSRF_001951_260407_041500.kwd
SPIFFS [INIT]: TWSRF_001951_260407_043000.kwd
SPIFFS [INIT]: TWSRF_001951_260407_044500.kwd
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260407.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: ftpremain.txt
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
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:10)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Memory Check - Stored CCID: 89914509006871527866 | Stored APN: airteliot.com
Smart APN: Match Found!
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
CGACT Resp: OK
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
SPIFFS: 35/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 88
RF Close date from RTC = 2026-04-07  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :87,2026-04-07,06:30,000.00,000.0,000.0,00.0,000,-069,04.12

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 6:30

NO GAPS FOUND ...


Current data inserted is 88,2026-04-07,06:45,000.00,000.0,000.0,00.0,000,-079,04.12


append_text->store_text : Used for internal status: 88,2026-04-07,06:45,000.00,000.0,000.0,00.0,000,-079,04.12


append_text written to lastrecorded_<stationname>.txt is : 88,2026-04-07,06:45,000.00,000.0,000.0,00.0,000,-079,04.12



[FILE] SPIFFS: /001951_20260407.txt | Size: 5340
   ... [Tail Content] ...
00,-081,04.13
84,2026-04-07,05:45,000.00,000.0,000.0,00.0,000,-069,04.12
85,2026-04-07,06:00,000.00,000.0,000.0,00.0,000,-079,04.13
86,2026-04-07,06:15,000.00,000.0,000.0,00.0,000,-079,04.12
87,2026-04-07,06:30,000.00,000.0,000.0,00.0,000,-069,04.12
88,2026-04-07,06:45,000.00,000.0,000.0,00.0,000,-079,04.12
-----------------------

[FILE] SD: /001951_20260407.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
51;2026-04-07,02:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:00;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:15;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:30;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001951;2026-04-07,04:45;00.00;000.0;000.0;00.0;000;-079;04.13
001951;2026-04-07,06:00;00.00;000.0;000.0;00.0;000;-079;04.12
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11656 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[RTC] Time: 06:46
[GPRS] last_http_ok=true. Skipping CGPADDR check (age < 2 slots).
[GPRS] Bearer status check: Re-initializing IP stack...
[GPRS] Starting HTTP...
HTTP POST REQUEST IS AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260407.txt

Current Data to be sent is : 87,2026-04-07,06:30,000.00,000.0,000.0,00.0,000,-069,04.12
http_data format is stn_no=001951&rec_time=2026-04-07,06:45&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.12&bat_volt2=04.12

Payload is stn_no=001951&rec_time=2026-04-07,06:45&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.12&bat_volt2=04.12
[HTTP] Using Fast v3.0 Handshake...
Response of AT+HTTPACTION=1 is 
OK

+HTTPACTION: 1,714,0

HTTP Error (706/713/714). Clean stack requested.
[HTTP] 1st Attempt (Fast) failed. Retrying in 2s (Fast Attempt 2)...
Payload is stn_no=001951&rec_time=2026-04-07,06:45&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.12&bat_volt2=04.12
[HTTP] Using Fast v3.0 Handshake...
[RTC] Time: 06:47
Response of AT+HTTPACTION=1 is 
ERROR

[HTTP] HTTPACTION timed out — no URC received from modem.
[HTTP] 2nd Attempt (Fast) also failed. Falling back to Robust method...
Payload is stn_no=001951&rec_time=2026-04-07,06:45&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.12&bat_volt2=04.12
[HTTP] Using Robust Handshake (Wait for DOWNLOAD)...
[HTTP] AT+HTTPDATA failed (Missing DOWNLOAD).
[HTTP] Attempt 1 failed. Re-initialising stack for single retry...
[HTTP] TCP Zombie (706/714) detected. Executing Hard Bearer Nuke...
[HTTP] Retry attempt...
Payload is stn_no=001951&rec_time=2026-04-07,06:45&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.12&bat_volt2=04.12
[HTTP] Using Robust Handshake (Wait for DOWNLOAD)...
Response of AT+HTTPACTION=1 is 
OK

+HTTPACTION: 1,714,0

HTTP Error (706/713/714). Clean stack requested.
[RTC] Time: 06:48
[DNS] Cleared fallback cache due to TCP zombie failure code.
[RECOVERY] Consec HTTP Fails: 1 | Present: 1 | CumMth: 3
CURRENT DATA : Retries exceeded limit... Storing to backlog.

ftpappend_text is : 001951;2026-04-07,06:45;00.00;000.0;000.0;00.0;000;-079;04.12

Record written into /ftpunsent.txt is as below : 
001951;2026-04-07,06:45;00.00;000.0;000.0;00.0;000;-079;04.12

*** Current data couldn't be sent. Backlog will be handled independently.
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=28 cur_time=06:48 sched=NO cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[FTP] Backlog Push: 28 records found. Mode: THRESHOLD_MET

FTP file name is /TWSRF_001951_260407_064500.kwd

SampleNo  is 88

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Huge Backlog! Records chunked for TX. Original queue protected until success.
Retrieved file is /TWSRF_001951_260407_064500.kwd
Initializing A7672S for FTP...
[FTP] Airtel/Jio detected. Smart Default: Passive (1).

ERROR


OK

+CFTPSSTART: 0

[FTP] Configuring mode (1)...

OK

[FTP] Warming up DNS...
[FTP] DNS Resolved: 27.34.245.70
[RTC] Time: 06:49
****RTC read error 0 (Try 1/3)
****RTC read error 4 (Try 1/3)
****RTC read error 4 (Try 2/3)
****RTC read error 0 (Try 3/3)

OK

+CFTPSLOGIN: 0

Result is 0
FTP Login success
File size: 945
[FTP] FSDEL path: TWSRF_001951_260407_064500.kwd
[FTP] FSOPEN cmd: AT+FSOPEN="C:/TWSRF_001951_260407_064500.kwd",0
[FTP] Found FH: 1
[FTP] Sending FSWRITE command...
[FTP] Data stream finished.
[FTP] Dispatching PUTFILE command...
[FTP] Dispatching PUTFILE command...
Response of AT+CFTPSPUTFILE: 
OK

+CFTPSPUTFILE: 0

[GoldenData] Marked records from /TWSRF_001951_260407_064500.kwd as DELIVERED.
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
****RTC read error 4 (Try 1/3)
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
****RTC read error 0 (Try 2/3)
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 06:50
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=6:50:7
 Sleep=10:7 (min:sec)
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
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.80 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.14V | Solar: 0.00V



[RTC] Time: 07:00
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260407.txt
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
[GPRS] Pre-setting APN for CID 1: airtelgprs.com (IP)
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:11)
[GPRS] Registered via CREG! (2G:1)
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Memory Check - Stored CCID: 89914509006871527866 | Stored APN: airteliot.com
Smart APN: Match Found!
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
CGACT Resp: OK
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
SPIFFS: 29/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 89
RF Close date from RTC = 2026-04-07  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :88,2026-04-07,06:45,000.00,000.0,000.0,00.0,000,-079,04.12

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 6:45

NO GAPS FOUND ...


Current data inserted is 89,2026-04-07,07:00,000.00,000.0,000.0,00.0,000,-079,04.12


append_text->store_text : Used for internal status: 89,2026-04-07,07:00,000.00,000.0,000.0,00.0,000,-079,04.12


append_text written to lastrecorded_<stationname>.txt is : 89,2026-04-07,07:00,000.00,000.0,000.0,00.0,000,-079,04.12



[FILE] SPIFFS: /001951_20260407.txt | Size: 5400
   ... [Tail Content] ...
00,-069,04.12
85,2026-04-07,06:00,000.00,000.0,000.0,00.0,000,-079,04.13
86,2026-04-07,06:15,000.00,000.0,000.0,00.0,000,-079,04.12
87,2026-04-07,06:30,000.00,000.0,000.0,00.0,000,-069,04.12
88,2026-04-07,06:45,000.00,000.0,000.0,00.0,000,-079,04.12
89,2026-04-07,07:00,000.00,000.0,000.0,00.0,000,-079,04.12
-----------------------

[FILE] SD: /001951_20260407.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
51;2026-04-07,03:00;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:15;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:30;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001951;2026-04-07,04:45;00.00;000.0;000.0;00.0;000;-079;04.13
001951;2026-04-07,06:00;00.00;000.0;000.0;00.0;000;-079;04.12
001951;2026-04-07,06:45;00.00;000.0;000.0;00.0;000;-079;04.12
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11544 bytes free
[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[RTC] Time: 07:01
[GPRS] Ghost PDP (0.0.0.0). Triggering recovery...
[GPRS] Bearer status check: Re-initializing IP stack...
[GPRS] Starting HTTP...
HTTP POST REQUEST IS AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[RTC] Time: 07:02
[GPRS] HTTPINIT Failed. Trying TERM then INIT...

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260407.txt

Current Data to be sent is : 88,2026-04-07,06:45,000.00,000.0,000.0,00.0,000,-079,04.12
http_data format is stn_no=001951&rec_time=2026-04-07,07:00&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.12&bat_volt2=04.12

[HTTP] HTTP session not ready. Fast-fail to backlog.
[HTTP] 1st Attempt (Fast) failed. Retrying in 2s (Fast Attempt 2)...
[HTTP] HTTP session not ready. Fast-fail to backlog.
[HTTP] 2nd Attempt (Fast) also failed. Falling back to Robust method...
[HTTP] HTTP session not ready. Fast-fail to backlog.
[HTTP] Attempt 1 failed. Re-initialising stack for single retry...
[HTTP] Retry attempt...
Payload is stn_no=001951&rec_time=2026-04-07,07:00&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-079&bat_volt=04.12&bat_volt2=04.12
[HTTP] Using Robust Handshake (Wait for DOWNLOAD)...
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
[HTTP] Retry succeeded.
[PWR] Signature persisted successfully.

**** Storing Last Logged Data as 2026-04-07,07:00
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=13 cur_time=07:02 sched=NO cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[RTC] Time: 07:03
[FTP] Backlog Push: 13 records found. Mode: THRESHOLD_MET

FTP file name is /TWSRF_001951_260407_070000.kwd

SampleNo  is 89

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Backlog fully chunked into Active Payload.
Retrieved file is /TWSRF_001951_260407_070000.kwd
Initializing A7672S for FTP...
[FTP] Using SAVED Smart Mode: Passive

ERROR


OK

+CFTPSSTART: 0

[FTP] Configuring mode (1)...

OK

[FTP] Warming up DNS...
[FTP] DNS Resolved: 27.34.245.70
****RTC read error 4 (Try 1/3)

OK

+CFTPSLOGIN: 0

Result is 0
FTP Login success
File size: 819
[FTP] FSDEL path: TWSRF_001951_260407_070000.kwd
[FTP] FSOPEN cmd: AT+FSOPEN="C:/TWSRF_001951_260407_070000.kwd",0
[FTP] Found FH: 1
[FTP] Sending FSWRITE command...
[FTP] Data stream finished.
[FTP] Dispatching PUTFILE command...
[FTP] Dispatching PUTFILE command...
Response of AT+CFTPSPUTFILE: 
ERROR

[FTP] PUT failed. Re-staging and retrying in mode (0)...

ERROR


OK

+CFTPSSTART: 0

[FTP] Configuring mode (0)...

OK

[FTP] Warming up DNS...
[FTP] DNS Resolved: 89.32.144.163
****RTC read error 4 (Try 1/3)
****RTC read error 4 (Try 2/3)
[UI] Wakeup detected. Key: P

OK

+CFTPSLOGIN: 9

Result is 9
FTP Login unsuccessful
[FTP] Race Guard: Logout too fast (Cleanup)? Waiting 1s drain.

[GPRS] Checking SMS...

Removed READ/SENT messages (UNREAD preserved)
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[RTC] Time: 07:05
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=7:5:4
 Sleep=10:9 (min:sec)
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
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.80 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.14V | Solar: 0.00V



[RTC] Time: 07:15
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260407.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: TWSRF_001951_260407_070000.kwd
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
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -69

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
[GPRS] Adaptive Mode: AT+CNMP=13 (LastSucc:13 SlotsOn2G:12)
Reg Search [BSNL:0]... Status:0 Iter:#1/24
[GPRS] CGREG registered during adaptive wait!
Registration Successful.
--- GPRS SETTING PDP ---

************************
Setting PDP context 
************************

Memory Check - Stored CCID: 89914509006871527866 | Stored APN: airteliot.com
Smart APN: Match Found!
Trying APN: airteliot.com
[APN] Flash match found. Skipping redundant write.
[RTC] Time: 07:16
CGACT Resp: OK
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
SPIFFS: 30/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 90
RF Close date from RTC = 2026-04-07  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :89,2026-04-07,07:00,000.00,000.0,000.0,00.0,000,-079,04.12

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 7:0

NO GAPS FOUND ...


Current data inserted is 90,2026-04-07,07:15,000.00,000.0,000.0,00.0,000,-069,04.12


append_text->store_text : Used for internal status: 90,2026-04-07,07:15,000.00,000.0,000.0,00.0,000,-069,04.12


append_text written to lastrecorded_<stationname>.txt is : 90,2026-04-07,07:15,000.00,000.0,000.0,00.0,000,-069,04.12



[FILE] SPIFFS: /001951_20260407.txt | Size: 5460
   ... [Tail Content] ...
00,-079,04.13
86,2026-04-07,06:15,000.00,000.0,000.0,00.0,000,-079,04.12
87,2026-04-07,06:30,000.00,000.0,000.0,00.0,000,-069,04.12
88,2026-04-07,06:45,000.00,000.0,000.0,00.0,000,-079,04.12
89,2026-04-07,07:00,000.00,000.0,000.0,00.0,000,-079,04.12
90,2026-04-07,07:15,000.00,000.0,000.0,00.0,000,-069,04.12
-----------------------

[FILE] SD: /001951_20260407.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
51;2026-04-07,03:00;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:15;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:30;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001951;2026-04-07,04:45;00.00;000.0;000.0;00.0;000;-079;04.13
001951;2026-04-07,06:00;00.00;000.0;000.0;00.0;000;-079;04.12
001951;2026-04-07,06:45;00.00;000.0;000.0;00.0;000;-079;04.12
-----------------------
--- UNSENT DATA END ---



[SCHED] Stack HWM: 11592 bytes free
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

SPIFF FILE EXISTS ..../001951_20260407.txt

Current Data to be sent is : 89,2026-04-07,07:00,000.00,000.0,000.0,00.0,000,-079,04.12
http_data format is stn_no=001951&rec_time=2026-04-07,07:15&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.12&bat_volt2=04.12

Payload is stn_no=001951&rec_time=2026-04-07,07:15&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.12&bat_volt2=04.12
[HTTP] Using Fast v3.0 Handshake...
Response of AT+HTTPACTION=1 is 
OK

+HTTPACTION: 1,713,0

HTTP Error (706/713/714). Clean stack requested.
[HTTP] 1st Attempt (Fast) failed. Retrying in 2s (Fast Attempt 2)...
Payload is stn_no=001951&rec_time=2026-04-07,07:15&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.12&bat_volt2=04.12
[HTTP] Using Fast v3.0 Handshake...
****RTC read error 0 (Try 1/3)
[RTC] Time: 07:17
Response of AT+HTTPACTION=1 is 
ERROR

[HTTP] HTTPACTION timed out — no URC received from modem.
[HTTP] 2nd Attempt (Fast) also failed. Falling back to Robust method...
Payload is stn_no=001951&rec_time=2026-04-07,07:15&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.12&bat_volt2=04.12
[HTTP] Using Robust Handshake (Wait for DOWNLOAD)...
[HTTP] AT+HTTPDATA failed (Missing DOWNLOAD).
[HTTP] Attempt 1 failed. Re-initialising stack for single retry...
[HTTP] TCP Zombie (706/714) detected. Executing Hard Bearer Nuke...
[HTTP] Retry attempt...
Payload is stn_no=001951&rec_time=2026-04-07,07:15&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.12&bat_volt2=04.12
[HTTP] Using Robust Handshake (Wait for DOWNLOAD)...
[RTC] Time: 07:18
Response of AT+HTTPACTION=1 is 
OK

+HTTPACTION: 1,706,0

HTTP Error (706/713/714). Clean stack requested.
[DNS] Cleared fallback cache due to TCP zombie failure code.
[RECOVERY] Consec HTTP Fails: 1 | Present: 1 | CumMth: 4
CURRENT DATA : Retries exceeded limit... Storing to backlog.

ftpappend_text is : 001951;2026-04-07,07:15;00.00;000.0;000.0;00.0;000;-069;04.12

Record written into /ftpunsent.txt is as below : 
001951;2026-04-07,07:15;00.00;000.0;000.0;00.0;000;-069;04.12

*** Current data couldn't be sent. Backlog will be handled independently.
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=14 cur_time=07:18 sched=NO cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
****RTC read error 4 (Try 1/3)
****RTC read error 0 (Try 2/3)
[FTP] Backlog Push: 14 records found. Mode: THRESHOLD_MET

FTP file name is /TWSRF_001951_260407_071500.kwd

SampleNo  is 90

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Backlog fully chunked into Active Payload.
Retrieved file is /TWSRF_001951_260407_071500.kwd
Initializing A7672S for FTP...
[FTP] Using SAVED Smart Mode: Passive
[RTC] Time: 07:19

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
File size: 882
[FTP] FSDEL path: TWSRF_001951_260407_071500.kwd
[FTP] FSOPEN cmd: AT+FSOPEN="C:/TWSRF_001951_260407_071500.kwd",0
[FTP] Found FH: 1
[FTP] Sending FSWRITE command...
[FTP] Data stream finished.
[FTP] Dispatching PUTFILE command...
[FTP] Dispatching PUTFILE command...
Response of AT+CFTPSPUTFILE: 
ERROR

[FTP] PUT failed. Re-staging and retrying in mode (0)...

ERROR


OK

+CFTPSSTART: 0

[FTP] Configuring mode (0)...

OK

[FTP] Warming up DNS...
[FTP] DNS Resolved: 89.32.144.163
[RTC] Time: 07:20

OK

+CFTPSLOGIN: 9

Result is 9
FTP Login unsuccessful
[FTP] Race Guard: Logout too fast (Cleanup)? Waiting 1s drain.
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=7:20:14
 Sleep=9:59 (min:sec)
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
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.80 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.13V | Solar: 0.00V



[RTC] Time: 07:30
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: signature.txt
SPIFFS [INIT]: TWSRF_001951_260407_070000.kwd
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260407.txt
SPIFFS [INIT]: ftpunsent.txt
SPIFFS [INIT]: TWSRF_001951_260407_071500.kwd
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
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -69

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
CGACT Resp: OK
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
SPIFFS: 31/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 91
RF Close date from RTC = 2026-04-07  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :90,2026-04-07,07:15,000.00,000.0,000.0,00.0,000,-069,04.12

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 7:15

NO GAPS FOUND ...


Current data inserted is 91,2026-04-07,07:30,000.00,000.0,000.0,00.0,000,-069,04.11


append_text->store_text : Used for internal status: 91,2026-04-07,07:30,000.00,000.0,000.0,00.0,000,-069,04.11


append_text written to lastrecorded_<stationname>.txt is : 91,2026-04-07,07:30,000.00,000.0,000.0,00.0,000,-069,04.11



[FILE] SPIFFS: /001951_20260407.txt | Size: 5520
   ... [Tail Content] ...
00,-079,04.12
87,2026-04-07,06:30,000.00,000.0,000.0,00.0,000,-069,04.12
88,2026-04-07,06:45,000.00,000.0,000.0,00.0,000,-079,04.12
89,2026-04-07,07:00,000.00,000.0,000.0,00.0,000,-079,04.12
90,2026-04-07,07:15,000.00,000.0,000.0,00.0,000,-069,04.12
91,2026-04-07,07:30,000.00,000.0,000.0,00.0,000,-069,04.11
-----------------------

[FILE] SD: /001951_20260407.txt

--- UNSENT DATA START ---
   ... [Tail Content] ...
51;2026-04-07,03:15;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:30;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,03:45;00.00;000.0;000.0;00.0;000;-111;04.13
001951;2026-04-07,04:00;00.00;000.0;000.0;00.0;000;-111;04.15
001951;2026-04-07,04:45;00.00;000.0;000.0;00.0;000;-079;04.13
001951;2026-04-07,06:00;00.00;000.0;000.0;00.0;000;-079;04.12
001951;2026-04-07,06:45;00.00;000.0;000.0;00.0;000;-079;04.12
001951;2026-04-07,07:15;00.00;000.0;000.0;00.0;000;-069;04.12
-----------------------
--- UNSENT DATA END ---



[GPRS] State Change: sync_mode=2 gprs_mode=1

****************
Starting Automated Data Flow
****************
[BATT] Proceeding with Spatika Upload regardless of battery voltage.
[GPRS] Prepared URL: AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"
[GPRS] PDP context inactive. Activating for DNS/HTTP...
[SCHED] Stack HWM: 11460 bytes free
[GPRS] Ghost PDP (0.0.0.0). Triggering recovery...
[GPRS] Bearer status check: Re-initializing IP stack...
[GPRS] Starting HTTP...
HTTP POST REQUEST IS AT+HTTPPARA="URL","http://rtdas.ksndmc.net:80/tws_gprs/update_twsrf_data_v2"

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260407.txt

Current Data to be sent is : 90,2026-04-07,07:15,000.00,000.0,000.0,00.0,000,-069,04.12
http_data format is stn_no=001951&rec_time=2026-04-07,07:30&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.12&bat_volt2=04.12

Payload is stn_no=001951&rec_time=2026-04-07,07:30&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.12&bat_volt2=04.12
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

**** Storing Last Logged Data as 2026-04-07,07:30
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=14 cur_time=07:31 sched=NO cleanup=NO
[FTP] HTTP finished. Waiting 5s for Tower to release context (Breather)...
[FTP] Backlog Push: 14 records found. Mode: THRESHOLD_MET

FTP file name is /TWSRF_001951_260407_073000.kwd

SampleNo  is 91

Entering time bound FTP loop
[FTP] Checking Registration before backlog upload...
[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.
[FTP] Bearer fresh. Brief 5s settle before FTP...
[FTP] Backlog fully chunked into Active Payload.
Retrieved file is /TWSRF_001951_260407_073000.kwd
Initializing A7672S for FTP...
[FTP] Using SAVED Smart Mode: Passive

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
File size: 882
[FTP] FSDEL path: TWSRF_001951_260407_073000.kwd
[RTC] Time: 07:32
[FTP] FSOPEN cmd: AT+FSOPEN="C:/TWSRF_001951_260407_073000.kwd",0
[FTP] Found FH: 1
[FTP] Sending FSWRITE command...
[FTP] Data stream finished.
[FTP] Dispatching PUTFILE command...
[FTP] Dispatching PUTFILE command...
Response of AT+CFTPSPUTFILE: 
OK

+CFTPSPUTFILE: 0

[GoldenData] Marked records from /TWSRF_001951_260407_073000.kwd as DELIVERED.
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=7:32:26
 Sleep=12:47 (min:sec)
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
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.80 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.14V | Solar: 0.00V



[RTC] Time: 07:45
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260407.txt
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
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -73

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
CGACT Resp: OK
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
SPIFFS: 28/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 92
RF Close date from RTC = 2026-04-07  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :91,2026-04-07,07:30,000.00,000.0,000.0,00.0,000,-069,04.11

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 7:30

NO GAPS FOUND ...


Current data inserted is 92,2026-04-07,07:45,000.00,000.0,000.0,00.0,000,-073,04.12


append_text->store_text : Used for internal status: 92,2026-04-07,07:45,000.00,000.0,000.0,00.0,000,-073,04.12


append_text written to lastrecorded_<stationname>.txt is : 92,2026-04-07,07:45,000.00,000.0,000.0,00.0,000,-073,04.12



[FILE] SPIFFS: /001951_20260407.txt | Size: 5580
   ... [Tail Content] ...
00,-069,04.12
88,2026-04-07,06:45,000.00,000.0,000.0,00.0,000,-079,04.12
89,2026-04-07,07:00,000.00,000.0,000.0,00.0,000,-079,04.12
90,2026-04-07,07:15,000.00,000.0,000.0,00.0,000,-069,04.12
91,2026-04-07,07:30,000.00,000.0,000.0,00.0,000,-069,04.11
92,2026-04-07,07:45,000.00,000.0,000.0,00.0,000,-073,04.12
-----------------------

[FILE] SD: /001951_20260407.txt


[SCHED] Stack HWM: 11544 bytes free
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

SPIFF FILE EXISTS ..../001951_20260407.txt

Current Data to be sent is : 91,2026-04-07,07:30,000.00,000.0,000.0,00.0,000,-069,04.11
http_data format is stn_no=001951&rec_time=2026-04-07,07:45&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-073&bat_volt=04.11&bat_volt2=04.11

Payload is stn_no=001951&rec_time=2026-04-07,07:45&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-073&bat_volt=04.11&bat_volt2=04.11
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

**** Storing Last Logged Data as 2026-04-07,07:45
[RTC] Time: 07:46
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=07:46 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=7:46:12
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
[GPS] No persisted location found.
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.80 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.13V | Solar: 0.00V



[RTC] Time: 08:00
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260407.txt
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
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -73

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
CGACT Resp: OK
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
SPIFFS: 28/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 93
RF Close date from RTC = 2026-04-07  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :92,2026-04-07,07:45,000.00,000.0,000.0,00.0,000,-073,04.12

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 7:45

NO GAPS FOUND ...


Current data inserted is 93,2026-04-07,08:00,000.00,000.0,000.0,00.0,000,-073,04.12


append_text->store_text : Used for internal status: 93,2026-04-07,08:00,000.00,000.0,000.0,00.0,000,-073,04.12


append_text written to lastrecorded_<stationname>.txt is : 93,2026-04-07,08:00,000.00,000.0,000.0,00.0,000,-073,04.12



[FILE] SPIFFS: /001951_20260407.txt | Size: 5640
   ... [Tail Content] ...
00,-079,04.12
89,2026-04-07,07:00,000.00,000.0,000.0,00.0,000,-079,04.12
90,2026-04-07,07:15,000.00,000.0,000.0,00.0,000,-069,04.12
91,2026-04-07,07:30,000.00,000.0,000.0,00.0,000,-069,04.11
92,2026-04-07,07:45,000.00,000.0,000.0,00.0,000,-073,04.12
93,2026-04-07,08:00,000.00,000.0,000.0,00.0,000,-073,04.12
-----------------------

[FILE] SD: /001951_20260407.txt


[SCHED] Stack HWM: 11544 bytes free
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

SPIFF FILE EXISTS ..../001951_20260407.txt

Current Data to be sent is : 92,2026-04-07,07:45,000.00,000.0,000.0,00.0,000,-073,04.12
http_data format is stn_no=001951&rec_time=2026-04-07,08:00&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-073&bat_volt=04.12&bat_volt2=04.12

Payload is stn_no=001951&rec_time=2026-04-07,08:00&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-073&bat_volt=04.12&bat_volt2=04.12
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

**** Storing Last Logged Data as 2026-04-07,08:00
[RTC] Time: 08:01
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=08:01 sched=NO cleanup=NO

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
[PWR] Sleep: CurTime=8:1:13
 Sleep=14:0 (min:sec)
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
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.80 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.14V | Solar: 0.00V



[RTC] Time: 08:15
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260407.txt
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
CGACT Resp: OK
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
SPIFFS: 28/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 94
RF Close date from RTC = 2026-04-07  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :93,2026-04-07,08:00,000.00,000.0,000.0,00.0,000,-073,04.12

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 8:0

NO GAPS FOUND ...


Current data inserted is 94,2026-04-07,08:15,000.00,000.0,000.0,00.0,000,-071,04.12


append_text->store_text : Used for internal status: 94,2026-04-07,08:15,000.00,000.0,000.0,00.0,000,-071,04.12


append_text written to lastrecorded_<stationname>.txt is : 94,2026-04-07,08:15,000.00,000.0,000.0,00.0,000,-071,04.12



[FILE] SPIFFS: /001951_20260407.txt | Size: 5700
   ... [Tail Content] ...
00,-079,04.12
90,2026-04-07,07:15,000.00,000.0,000.0,00.0,000,-069,04.12
91,2026-04-07,07:30,000.00,000.0,000.0,00.0,000,-069,04.11
92,2026-04-07,07:45,000.00,000.0,000.0,00.0,000,-073,04.12
93,2026-04-07,08:00,000.00,000.0,000.0,00.0,000,-073,04.12
94,2026-04-07,08:15,000.00,000.0,000.0,00.0,000,-071,04.12
-----------------------

[FILE] SD: /001951_20260407.txt


[SCHED] Stack HWM: 11544 bytes free
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

SPIFF FILE EXISTS ..../001951_20260407.txt

Current Data to be sent is : 93,2026-04-07,08:00,000.00,000.0,000.0,00.0,000,-073,04.12
http_data format is stn_no=001951&rec_time=2026-04-07,08:15&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.12&bat_volt2=04.12

Payload is stn_no=001951&rec_time=2026-04-07,08:15&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.12&bat_volt2=04.12
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

**** Storing Last Logged Data as 2026-04-07,08:15
[RTC] Time: 08:16
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=08:16 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=8:16:12
 Sleep=14:3 (min:sec)
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
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.80 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.14V | Solar: 0.00V



[RTC] Time: 08:30
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260407.txt
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
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -69

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
CGACT Resp: OK
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
SPIFFS: 28/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 95
RF Close date from RTC = 2026-04-07  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :94,2026-04-07,08:15,000.00,000.0,000.0,00.0,000,-071,04.12

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 8:15

NO GAPS FOUND ...


Current data inserted is 95,2026-04-07,08:30,000.00,000.0,000.0,00.0,000,-069,04.12


append_text->store_text : Used for internal status: 95,2026-04-07,08:30,000.00,000.0,000.0,00.0,000,-069,04.12


append_text written to lastrecorded_<stationname>.txt is : 95,2026-04-07,08:30,000.00,000.0,000.0,00.0,000,-069,04.12

[SCHED] 08:30 AM Cycle Complete. Rollover pending at 08:45...


[FILE] SPIFFS: /001951_20260407.txt | Size: 5760
   ... [Tail Content] ...
00,-069,04.12
91,2026-04-07,07:30,000.00,000.0,000.0,00.0,000,-069,04.11
92,2026-04-07,07:45,000.00,000.0,000.0,00.0,000,-073,04.12
93,2026-04-07,08:00,000.00,000.0,000.0,00.0,000,-073,04.12
94,2026-04-07,08:15,000.00,000.0,000.0,00.0,000,-071,04.12
95,2026-04-07,08:30,000.00,000.0,000.0,00.0,000,-069,04.12
-----------------------

[FILE] SD: /001951_20260407.txt


[SCHED] Stack HWM: 11544 bytes free
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

SPIFF FILE EXISTS ..../001951_20260407.txt

Current Data to be sent is : 94,2026-04-07,08:15,000.00,000.0,000.0,00.0,000,-071,04.12
http_data format is stn_no=001951&rec_time=2026-04-07,08:30&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.12&bat_volt2=04.12

Payload is stn_no=001951&rec_time=2026-04-07,08:30&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.12&bat_volt2=04.12
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

**** Storing Last Logged Data as 2026-04-07,08:30
[RTC] Time: 08:31
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=08:31 sched=NO cleanup=NO
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=8:31:13
 Sleep=14:1 (min:sec)
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
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.80 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.13V | Solar: 0.00V



[RTC] Time: 08:45
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260407.txt
SPIFFS [INIT]: closingdata_001951.txt
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
CGACT Resp: OK
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
SPIFFS: 28/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 0
RF Close date from RTC = 2026-04-08  
[SCHED] 🗓 Day Change Detected. Performing Rollover...
[SCHED] Rollover Complete.
[LTS] Yesterday's DailyFTP staging file pruned.
[GoldenData] Starting Sent Mask Reconstruction from SPIFFS...
[GoldenData] Reconstruction Complete (Sent-Accurate).

********** CREATING NEW FILE .. DEVICE STARTED AFTER A FEW DAYS  ***********

**It is the VERY FIRST DATA . Creating a new file ...

8:45AM data being written to new SPIFF file is 00,2026-04-07,08:45,0000.0,000.0,000.0,00.0,000,-071,04.12

Previous day exists. Normal rollover. Skipping redundant 8:45 backlog.

append_text->store_text : Used for internal status: 00,2026-04-07,08:45,0000.0,000.0,000.0,00.0,000,-071,04.12


append_text written to lastrecorded_<stationname>.txt is : 00,2026-04-07,08:45,0000.0,000.0,000.0,00.0,000,-071,04.12



[FILE] SPIFFS: /001951_20260408.txt | Size: 60
   ... [Tail Content] ...
00,2026-04-07,08:45,0000.0,000.0,000.0,00.0,000,-071,04.12
-----------------------

[FILE] SD: /001951_20260408.txt


[SCHED] Stack HWM: 11448 bytes free
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
[RTC] Time: 08:46

*********  STARTING TO SEND HTTP ... ***********


*********  Sending Current data to main server... ***********

SPIFF FILE EXISTS ..../001951_20260408.txt

Current Data to be sent is : 00,2026-04-07,08:45,0000.0,000.0,000.0,00.0,000,-071,04.12
http_data format is stn_no=001951&rec_time=2026-04-07,08:45&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.12&bat_volt2=04.12

Payload is stn_no=001951&rec_time=2026-04-07,08:45&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-071&bat_volt=04.12&bat_volt2=04.12
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

**** Storing Last Logged Data as 2026-04-07,08:45
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=08:46 sched=NO cleanup=YES
[GPRS] Skipping SMS check (hourly task).
[GPRS] Cycle fully complete (including Commands).
[GPRS] No queued commands. Allowing sleep.
[PWR] All tasks done. Entering Deep Sleep...
[PWR] Entering Deep Sleep Sequence...
[GPRS] State Change: sync_mode=4 gprs_mode=1
[GPRS] Session clean. Attempting graceful shutdown...
[GPRS] Modem alive. Closing network session gracefully...
[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).
[PWR] Sleep: CurTime=8:46:23
 Sleep=13:50 (min:sec)
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
[BOOT] Canonical ID Enforced: 001951
[BOOT] Station Altitude: 0.00
Active RF Resolution: 0.50
[BOOT] Unit: TWSRF9-DMC-5.80 | Network: KSNDMC | Type: TWS-RF

Wakeup caused by timer
[BOOT] Reset Reason: DEEPSLEEP_RESET
Chip ID: 509ECC641D44
[BOOT] Chip ID: 509ECC641D44 | Flash: 8 MB
ULP Wakeup Period set to 1ms (High Resolution for Wind)[ULP] Init. HW Reason: 5
[ULP] Warm Boot. Preserving Counters. RF=0
ULP Program loaded and started.
[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=0, Rain=0
[RTC] Task Started
[PWR] Battery: 4.13V | Solar: 0.00V



[RTC] Time: 09:00
SPIFFS [INIT]: rf_res.txt
SPIFFS [INIT]: rf_fw.txt
SPIFFS [INIT]: WS0221_20260405.txt
SPIFFS [INIT]: lastrecorded_WS0221.txt
SPIFFS [INIT]: station.txt
SPIFFS [INIT]: station.doc
SPIFFS [INIT]: 001951_20260405.txt
SPIFFS [INIT]: 001951_20260406.txt
SPIFFS [INIT]: firmware.doc
SPIFFS [INIT]: apn_config.txt
SPIFFS [INIT]: 001951_20260407.txt
SPIFFS [INIT]: closingdata_001951.txt
SPIFFS [INIT]: prevWindSpeed.txt
SPIFFS [INIT]: 001951_20260408.txt
SPIFFS [INIT]: lastrecorded_001951.txt
SPIFFS [INIT]: dailyftp_20260408.txt
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
Signal strength IN gprs task is 85
Signal Level is -111
Signal strength IN gprs task is -69

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
CGACT Resp: OK
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
SPIFFS: 24/4699
SD Card: FAILED/NOT FOUND
----------------------------
[GPRS] State Change: sync_mode=1 gprs_mode=1
------- ENTERING SCHEDULER 15 MIN LOOP --------  

Current SampleNo is 1
RF Close date from RTC = 2026-04-08  

-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------

The last line of the file is :00,2026-04-07,08:45,0000.0,000.0,000.0,00.0,000,-071,04.12

Last Parse: CRF=0.00 T=0.00 H=0.00
Last recorded hour/min is 8:45

NO GAPS FOUND ...


Current data inserted is 01,2026-04-07,09:00,000.00,000.0,000.0,00.0,000,-069,04.12


append_text->store_text : Used for internal status: 01,2026-04-07,09:00,000.00,000.0,000.0,00.0,000,-069,04.12


append_text written to lastrecorded_<stationname>.txt is : 01,2026-04-07,09:00,000.00,000.0,000.0,00.0,000,-069,04.12



[FILE] SPIFFS: /001951_20260408.txt | Size: 120
   ... [Tail Content] ...
00,2026-04-07,08:45,0000.0,000.0,000.0,00.0,000,-071,04.12
01,2026-04-07,09:00,000.00,000.0,000.0,00.0,000,-069,04.12
-----------------------

[FILE] SD: /001951_20260408.txt


[SCHED] Stack HWM: 11544 bytes free
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

SPIFF FILE EXISTS ..../001951_20260408.txt

Current Data to be sent is : 00,2026-04-07,08:45,0000.0,000.0,000.0,00.0,000,-071,04.12
http_data format is stn_no=001951&rec_time=2026-04-07,09:00&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.12&bat_volt2=04.12

Payload is stn_no=001951&rec_time=2026-04-07,09:00&key=rfclimate5p13&rainfall=000.0&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-069&bat_volt=04.12&bat_volt2=04.12
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

**** Storing Last Logged Data as 2026-04-07,09:00
[RTC] Time: 09:01
Entering FTP mode and checking if data period is correct for sending and if there is any file to be sent
[FTP-Gate] unsent=0 cur_time=09:01 sched=YES cleanup=NO

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
[PWR] Sleep: CurTime=9:1:13
 Sleep=14:1 (min:sec)
[PWR] Entering Deep Sleep
