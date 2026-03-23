Critical Issues
1. Version comparison breaks at X.10+
In ota_service.py, get_numeric_ver() converts "5.10" to float("5.10") = 5.1, which is numerically equal to 5.1. So firmware 5.10 will never OTA-push over devices on 5.1. As the version numbers grow past single-digit minor versions, this will silently freeze OTA rollouts. Fix: use tuple comparison (5, 10) from split-on-dot, not float casting.
2. Hardcoded session cookie in cookies.txt committed to source
server/cookies.txt contains a live session ID (session_id=f1021ca23692a1005b5e13ea257ef0a4) for IP 75.119.148.192. This is your production server. Anyone with this repo can immediately authenticate to the dashboard as an admin. That file must be purged from git history and the session invalidated immediately.
3. No authentication on /health endpoint — command injection risk
Any device (or attacker) can POST to /health with a spoofed stn_id and trigger DELETE_DATA, REBOOT, OTA_CHECK, or FTP_BACKLOG commands against any other station. There is no API key or device secret on the health endpoint. A rogue actor knowing a station number can factory-reset or push arbitrary firmware to any field unit.
4. Hardcoded station ID override in production code
pythonif stn_id == "1931" or stn_id == "001931":
    ota_fails = 0
This "temporary" override in health.py bypasses the OTA safety lock for station 1931 permanently and unconditionally. If left in, this station can never accumulate OTA fail-counts and will retry forever. This must be removed.
5. goto used across a mutex acquisition boundary in gprs_helpers.ino
The bearer_recovery: label is jumped to via goto bearer_recovery after a conditional APN-mismatch detection. The problem is the goto skips initialization code and jumps directly into the recovery block, but the surrounding code relies on ota_silent_mode = true being set at function entry. If the jump happens after a partial execution path where the mutex state is different, you risk a deadlock or a missed ota_silent_mode = false on failure exit. Use a structured do { } while(0) or a flag instead.

Serious / Reliability Issues
6. ADC reading for battery voltage is uncalibrated
cppli_bat_val = li_bat * 0.0010915;  // (3.3/4096) * (840/620)
The ESP32 ADC is notoriously non-linear — without calling esp_adc_cal_characterize() and esp_adc_cal_raw_to_voltage(), readings at low battery (3.4–3.6V) can be off by 100–200mV. For a 3.2V LFP chemistry this matters: you may cut off at 3.4V thinking it is 3.2V, causing premature sleep, or miss a genuine brown-out condition. The adc2_get_raw for solar also shares this problem, and adc2 contends with WiFi — the if (!wifi_active) guard is correct but the raw ADC value without calibration is still unreliable.
7. RTC drift correction ignores midnight rollover
In sync_rtc_from_server_tm():
cppint drift = abs(srv_total_sec - rtc_total_sec);
If the server says 23:59:50 (86390s) and the RTC says 00:00:10 (10s), the computed drift is 86380 seconds — which triggers an incorrect correction and sets the RTC back by nearly a day. The same midnight wrap issue exists at any hour boundary near the 45s/90s threshold. The fix is modular arithmetic: drift = min(abs_diff, 86400 - abs_diff).
8. FORCE_2G_ONLY is a global compile flag but mixed BSNL/Airtel M2M SIMs are deployed together
FORCE_2G_ONLY = 1 sends AT+CNMP=13 (GSM only) at boot. For Airtel M2M SIMs on 4G-only towers (which the code itself acknowledges: "Airtel 4G-only tower"), this means the modem will never register. You cannot solve a mixed SIM fleet with a single compile-time flag — this needs to be a runtime decision based on the detected carrier after CCID/CSPN resolution. The current workaround in the 4G-aware registration poll is fighting against the boot-time CNMP=13 command.
9. Airtel M2M health server reachability silently skipped
cppif (strstr(carrier, "Airtel") && strstr(apn_str, "airteliot")) {
    debugln("[Health] Skipping: Airtel M2M SIM + foreign health server.");
This means Airtel M2M stations never send health reports, never receive server commands (REBOOT, DELETE_DATA, OTA_CHECK), and are invisible to the fleet dashboard. If the majority of your fleet is Airtel M2M (which the user config implies with airteliot.com as the default fallback), the entire health telemetry and remote command system is effectively disabled for them. The correct fix is IP whitelisting on the Airtel M2M APN for the Contabo VPS IP — this is a commercial/operational fix, not a code fix, but the code should at minimum log a structured diagnostic rather than silently skipping.
10. In-memory session store (SESSIONS = {}) is not process-safe
If the FastAPI server is restarted (deploy, crash, docker restart), all sessions are wiped and every logged-in user is silently de-authenticated. More critically, with gunicorn --workers N, each worker has its own SESSIONS dict — a session created in worker 1 is unknown to worker 2, causing intermittent 302 redirects to /login. Use Redis or at minimum signed cookies (e.g. itsdangerous) for session persistence.
11. OTA firmware served over plain HTTP with no integrity check
Firmware .bin files are served from http://75.119.148.192/builds/... (no TLS). The ESP32 fetchFromHttpAndUpdate() does not verify any hash/signature of the received binary. A man-in-the-middle on the GPRS path (possible on BSNL 2G) can substitute a malicious firmware image. At minimum, compute and embed a SHA256 hash in the health endpoint response and verify it on the device before calling Update.end().

Moderate / Design Issues
12. SPIFFS single-file APN config is a concurrency hazard
save_apn_config() opens /apn_config.txt with FILE_WRITE (truncating it), then writes CCID + APN. If the ESP32 power-cycles mid-write (very common on a 3.2V LFP cell during modem inrush), the file will be partially written. On next boot, load_apn_config() reads a truncated CCID that partially matches the real ICCID (the indexOf check), and activates a wrong APN. A write-then-rename pattern (write to /apn_config.tmp, then SPIFFS.rename()) would be atomic.
13. RTC RAM pressure — 106 RTC_DATA_ATTR variables
The ESP32 has 8KB of RTC slow memory. The 512-byte ULP reserve plus the declared RTC variables (many are 4-byte floats and ints, several are char arrays of 16–64 bytes) is perilously close to the limit. There is no compile-time assertion checking. If new variables push it over, the linker silently places them in normal RAM, losing them on deep sleep — causing ghost data, phantom sensor faults, or a stuck last_processed_sample_idx that reprocesses old slots on every boot. Add a static_assert(sizeof(rtc_slow_mem_used) < 7680, "RTC RAM overflow") or monitor via ESP.getFreePsram().
14. delete_multiple_files() uses a fixed String toDelete[48] stack array
If a station has more than 48 matching SPIFFS files (possible after a long offline period), the function silently truncates at 48 and leaves the remaining files. The stack-allocated array also consumes ~3.8KB of task stack, which is significant for the GPRS task's typical 8–16KB allocation. Use a dynamic list or iterate in passes.
15. Jitter-based temperature/humidity data is indistinguishable from real data in the server
cpp#define TEMP_JITTER_MIN 0.1
#define TEMP_JITTER_MAX 0.3
When a sensor reads the same value 7 consecutive times, artificial jitter is added. This means the server's health_eval.py sensor diagnostics (diag_temp_cv = constant value) will never fire for a genuinely frozen sensor after the jitter kicks in. The jitter masks the fault. The diagnostic flag should be sent before jitter is applied, not suppressed by it.
16. CLEAR_FTP_QUEUE triggered by server when unsent_count > 50 is destructive
The server auto-triggers CLEAR_FTP_QUEUE when backlog exceeds 50 records. This permanently deletes unsent data. In a 2G BSNL deployment where connectivity is intermittent for days, 50 records is less than 13 hours of data. A station that loses connectivity during a monsoon event — precisely when rainfall data is most critical — will have its backlog silently deleted by the server. The threshold should be configurable, much higher (e.g. 300+), or replaced with a "compress and archive" command rather than delete.
17. needs_ota() triggers on version difference, not greater-than
pythonreturn get_numeric_ver(device_ver) != get_numeric_ver(target_ver)
If a device is somehow on a newer version than the registry (e.g. manually flashed in the field), this will attempt to downgrade it. It should be get_numeric_ver(device_ver) < get_numeric_ver(target_ver).
18. Drift correction only looks at time, not date
sync_rtc_from_server_tm() corrects hours/minutes/seconds but uses the current device date — it does not update current_day, current_month, or current_year. If the RTC battery fails and the date resets to 2000-01-01, the server sync will correct the time but leave the wrong date in place, causing all data records to be filed under the wrong date indefinitely.
19. http_data[350] and append_text[160] — no overflow guard on snprintf format strings
Several snprintf calls build HTTP payloads with variable-length station names, dates, and sensor readings. While most individual fields are bounded, the station name ftp_station is read from SPIFFS without a validated max length at the source, and the universalNumber[20] can be longer than expected if the IMSI parse produces garbage. A station name of 15 chars + all other fields could theoretically overflow http_data[350] in edge cases. Enforce snprintf(..., sizeof(http_data) - 1, ...) everywhere and add an assertion.
20. Server dashboard and OTA are served on plain HTTP (port 80)
The deploy script shows http://75.119.148.192/dashboard with no mention of HTTPS. Admin credentials and session cookies traverse the network in cleartext. On a shared or corporate network, the session cookie in cookies.txt (issue #2) would be trivially sniffable. Add a self-signed cert at minimum, or better, put the service behind an nginx reverse proxy with Let's Encrypt.

Minor / Hygiene Issues
21. goto in C++ across variable initializations in gprs_helpers.ino — C++ forbids jumping over variable declarations with initializers. The current goto bearer_recovery happens to work because the skipped declarations are in an outer scope, but this is fragile and will break if any variable is added between the goto and the label.
22. FIRMWARE_VERSION "5.66" in user_config.h but version 7.88/7.94 referenced in globals — Version comments in the code reference v7.x changes (// v7.88, // v7.94) while FIRMWARE_VERSION is "5.66". This is a version tracking inconsistency that will confuse the OTA version comparison logic if not kept in sync.
23. apn_str[20] is too small for some enterprise APNs — airteliot.com is 14 chars and fits. But if Airtel M2M APN is changed to a longer enterprise APN (e.g. airtelm2m.enterprise.in), it will silently truncate and cause connection failures with no error message.
24. pres_str[20] is allocated but pressure sensor is disabled by default — ENABLE_PRESSURE_SENSOR 0 means this buffer is always allocated (20 bytes) but never populated, and the BME280 routines compile conditionally. Confirm that pressure fields are not sent in HTTP payloads when the sensor is disabled — a zero-filled pressure in a KSNDMC record could be misinterpreted as sea-level pressure.
25. solar_val formula uses a magic multiplier 7.2 without comment — (solar / 4096.0) * 3.6 * 7.2 — the 3.6 is the ADC reference voltage, but 7.2 is presumably a voltage divider ratio. This should be documented as a constant (SOLAR_DIVIDER_RATIO) and validated against the actual hardware resistor values, especially if PCBs are being manufactured in batches.

Summary for ANTIGRAVITY
Priority order for fixes:

Remove cookies.txt from the repo and rotate the session
Add device API key authentication to the /health endpoint
Fix version comparison to use tuple-based semver, not float
Remove the hardcoded station 1931 OTA override
Fix the RTC midnight-rollover drift bug
Make FORCE_2G a runtime/per-carrier decision, not a compile flag
Add firmware SHA256 verification on OTA downloads
Switch to HTTPS on the server
Fix needs_ota() to be greater-than, not not-equal
Fix sync_rtc_from_server_tm() to also update the date fields
Raise or make configurable the FTP queue auto-clear threshold (50 is far too low)
Add ESP32 ADC calibration for battery voltage readings
Replace in-memory session store with signed cookies or Redis
Validate RTC RAM budget at compile time
Move sensor diagnostic flag capture to before jitter is applied


Additional Critical Issues (Continued)
26. All server API keys are hardcoded in firmware source in plaintext
httpSet[] in AIO9_5.0.ino contains production API keys for KSNDMC, Bihar government, and Spatika servers — "sit1040", "pse2420", "bmsk1234", "bmsk12345", "rfclimate5p13", "climate4p2013", "wsgen2014" — all in clear text committed to the source repository. These keys authenticate data uploads to live government weather servers. Anyone with access to this repo (or to a decompiled firmware binary via JTAG/UART) can inject arbitrary rainfall and climate data into government systems under a valid station ID. Keys should be stored in ESP32 NVS (Preferences) and provisioned separately during manufacturing, not compiled in.
27. wakeup_reason_is is uninitialized on power-on boot
volatile int wakeup_reason_is; is declared in AIO9_5.0.ino with no initializer. On a cold power-on boot (not from deep sleep), set_wakeup_reason() hits the default: case and leaves wakeup_reason_is at whatever value was in RAM — which on ESP32 is typically 0 but is not guaranteed. The is_fresh_boot logic (wakeup_reason_is == 0) happens to work only because ESP32 zero-initializes BSS, but this is a fragile assumption. Explicitly initialize: volatile int wakeup_reason_is = 0;
28. _auto_migrate runs an ALTER TABLE on every single health report POST
The _get_db_columns() call inside _auto_migrate() uses SQLAlchemy's inspect() which queries sqlite_master on every invocation. With 100+ field devices checking in every 15 minutes, this becomes hundreds of schema introspection queries per hour hitting SQLite. More critically, _auto_migrate takes a write lock via ALTER TABLE — if two devices check in simultaneously with a new field, you get a SQLite OperationalError: database is locked. Cache the column set at application startup and invalidate it only when a migration actually runs.
29. evaluate(r) called twice in _all_fields_row() in dashboard.py
python" | ".join(evaluate(r).get("reasons", [])) or evaluate(r).get("verdict", "OK")
evaluate() is called twice per row in every CSV export. Since this runs on potentially hundreds of historical records in the /csv endpoint, it doubles the evaluation compute. Cache the result: ev = evaluate(r); result = " | ".join(ev.get("reasons", [])) or ev.get("verdict", "OK").

Additional Serious Issues
30. modemMutex take/give count mismatch — 8 takes, 19 gives
Counting the raw xSemaphoreTake(modemMutex vs xSemaphoreGive(modemMutex) across gprs.ino: there are 8 take-sites and 19 give-sites. While some gives are in branches that follow a take, the asymmetry is a strong indicator of give-without-take paths — typically in error exits where the function gives a mutex it never successfully took. A give on a non-taken mutex increments the semaphore count above 1, making a future take succeed even when no one is holding it, silently destroying mutual exclusion. Every xSemaphoreGive(modemMutex) should be preceded by an audit confirming the corresponding take succeeded in that code path.
31. send_health_report() takes fsMutex first, then modemMutex inside
Looking at the call chain: the outer scheduler/GPRS flow can hold fsMutex and then call send_health_report(), which independently attempts to take modemMutex. Meanwhile, send_ftp_file() takes modemMutex first and then takes fsMutex for log writes. This is a classic lock ordering inversion — two tasks taking the same two locks in opposite order. Under the right timing (e.g. during a backlog FTP + simultaneous health report trigger), you get a deadlock. Both are recovered by WDT reset at 120s, but the data for that slot is lost. Establish a strict lock ordering: always modemMutex before fsMutex, never the reverse.
32. GPS coordinates stored as double in RTC RAM — precision overkill with a stability risk
RTC_DATA_ATTR double lati, longi; — double is 8 bytes each. RTC RAM is precious. A float gives 7 significant decimal digits, which is ~1cm precision for GPS — more than sufficient for a weather station. But more importantly, RTC RAM alignment requirements for 8-byte double on ESP32 can cause silent padding between adjacent RTC_DATA_ATTR variables, unpredictably increasing RTC RAM usage and potentially shifting the ULP memory map if the ULP address calculation relies on sequential offsets from a known base.
33. Dashboard station_detail fetches 400 records per page load, runs evaluate() on every one
python.limit(400).all()
For an active station with frequent health reports, loading the station detail page evaluates 400 health records in a synchronous request. On the shared Contabo VPS this blocks the event loop for the full duration. Add pagination or reduce to 50 records with a "load more" control.
34. diag_rtc_battery_ok is reused as a "system health bad" proxy for SPIFFS failure
cppdiag_rtc_battery_ok = false; // Reuse as "system health bad" proxy
This comment in setup() admits that a SPIFFS format failure sets diag_rtc_battery_ok = false. The health report will then show RTC_F in the dashboard for what is actually a flash storage failure — a misleading alert that points field technicians to the wrong component.
35. OFFLINE_MINS = 1470 (24.5 hours) means a dead station stays green all day
The server health evaluator only flags a station OFFLINE after 24.5 hours of no reports. A station that dies at 9 AM is still showing as non-OFFLINE on the dashboard at 8:59 AM the next day — nearly a full 24-hour blind spot. The comment says it was "relaxed from 7h." For a monsoon-season deployment where a station going offline for a few hours can mean a dam catchment is unmonitored, 1.5 hours (6 missed slots) is a more appropriate threshold, with a separate "STALE" state for 3–6 hours.
36. http_data[350] format string uses %04d for signal which is a negative number
cpp"signal=%04d&..."
%04d with a value of -114 produces -114 (5 chars, not 4) — the 04 width specifier on a negative number doesn't pad the same way. The server receives signal=-114 which is fine for parsing, but the field width assumption breaks if the server is doing fixed-width parsing. More critically, for the fill signal sentinels (-112 through -114) these format correctly, but SIGNAL_STRENGTH_MIN_RANGE = -130 would produce -130 and might confuse server-side parsers expecting ≥-120.
37. SPIFFS.begin(true) on mount failure silently wipes all data
cppif (!SPIFFS.begin(true)) { // true = auto-format
If SPIFFS mount fails due to a transient reason (power glitch during a write, first boot after a different partition table), the firmware immediately reformats, destroying all locally stored backlog data — potentially weeks of records. The correct behavior is to retry the mount 2–3 times, log the failure, and only format as a last resort after explicit confirmation (e.g. a specific keypad sequence). At minimum, copy files to SD card before formatting if sd_card_ok == 1.
38. DEBUG 1 is the default in user_config.h — production firmware ships with full serial logging enabled
cpp#define DEBUG 1 // 1: Enable Serial Logs (Dev), 0: Production (Saves ROM)
Every debugln() call compiles in with DEBUG=1. Serial logging over UART adds latency to time-critical sections (sensor reads, UART AT command sequences). More importantly, with DEBUG=1 the serial port leaks all AT command traffic, API keys embedded in http_data, GPS coordinates, and network credentials — readable by anyone who connects a UART adapter to the exposed programming header in the field enclosure. Production firmware must ship with DEBUG 0.
39. graceful_modem_shutdown() called inside start_deep_sleep() after GPIO holds are set
cppgpio_hold_en(GPIO_NUM_26);
gpio_hold_en(GPIO_NUM_32);
gpio_deep_sleep_hold_en();
// ...
graceful_modem_shutdown(); // CALLED AFTER GPIO HOLD!
Wait — checking the actual order in global_functions.ino: graceful_modem_shutdown() is called before gpio_hold_en, which is correct. But gpio_hold_en(GPIO_NUM_26) is set, then graceful_modem_shutdown() tries to digitalWrite(26, LOW) to cut modem power — and GPIO hold blocks this write. The modem power line stays wherever it was. Verify the exact call order in the compiled output; if modem is powered on when hold is applied, it stays powered through sleep and drains the LFP cell.
40. No brown-out detection configured
The ESP32 has a hardware brown-out detector (BOD) that resets the chip when VCC drops below a threshold. With a 3.2V LFP cell directly powering the ESP32 (which has a minimum operating voltage of ~2.3V but starts behaving erratically below 2.8V), and with the A7672S modem drawing 500mA+ peaks during GPRS transmission, voltage droop can cause silent data corruption in RTC RAM, SPIFFS writes, and flash operations without triggering a clean reset. esp_brownout_disable() is not called, which means the default BOD level (~2.43V) is active — this is actually correct — but the BOD reset reason should be explicitly checked in diag_last_reset_reason and logged as a distinct fault type, not conflated with generic WDT resets.

Additional Moderate Issues
41. carrier string matching is case-sensitive and inconsistent
In gprs_helpers.ino:
cppif (strstr(carrier, "BSNL") && strstr(stored_apn_str.c_str(), "airtel"))
But in gprs.ino, carrier is set via strncpy(carrier, ...) from AT+CSPN/COPS responses, which may return "bsnl", "BSNL", "BSNL cellular", or "BSNL Ltd." depending on the tower. strstr("bsnl cellular", "BSNL") returns NULL. The APN cross-check then silently passes (no mismatch detected), and a stale Airtel APN gets used for a BSNL SIM. Normalize carrier to uppercase immediately after AT+COPS parsing.
42. ftp_daily_date[12] is initialized to "" but never validated before FTP use
force_ftp_daily = true can be set without a valid date in ftp_daily_date (e.g. if the "p" field is missing from the server command JSON). The FTP daily function then attempts to open a file named after an empty string, which on SPIFFS becomes "/" — iterating the root. Add a validation: if (strlen(ftp_daily_date) != 8) { force_ftp_daily = false; }.
43. Server cookies.txt contains a hardcoded Netscape HTTP Cookie File header suggesting it is used by a curl automation script
This means there is likely a cron job or script on the Contabo server using curl -b cookies.txt to automate dashboard interactions. That script's session never expires and permanently holds admin privileges — a persistent backdoor if the file is exposed. Find and audit that automation.
44. is_valid_window = (minutes_into_interval <= 5) means data is only recorded in the first 5 minutes of each 15-minute slot
If the ESP32 wakes up late (e.g. due to modem shutdown taking longer than expected, or a keypad interaction), minutes_into_interval can exceed 5, and the slot is skipped entirely — contributing to the PD (Partial Data) count on the dashboard. This is a known trade-off but the window is tight: a modem that takes 4 minutes to register + 1 minute for HTTP leaves zero margin. Consider widening to 7 minutes, or implement a "catch-up" mechanism where a late wakeup still records and uploads even outside the window.
45. healer_reboot_in_progress RTC flag is cleared unconditionally on every boot
cpphealer_reboot_in_progress = false; // v5.57 Fix: Unconditionally clear
The comment acknowledges this prevents the flag from getting stuck, but it also means that if the healer reboot itself causes a WDT reset (e.g. the modem won't shut down cleanly), the system re-enters normal operation instead of entering a recovery mode. The flag's purpose — preventing re-entry into the healer path — is nullified on any unclean reboot during healing.

Summary Table for ANTIGRAVITY (Prioritized Master List)
#SeverityFileIssue2🔴 Criticalcookies.txtLive admin session token committed to repo3🔴 Criticalhealth.pyNo auth on /health — command injection possible26🔴 CriticalAIO9_5.0.inoAll government server API keys hardcoded in source1🔴 Criticalota_service.pyFloat version comparison breaks at X.10+4🔴 Criticalhealth.pyHardcoded station 1931 OTA override in production5🔴 Criticalgprs_helpers.inogoto crosses mutex boundary11🔴 Criticalhealth.pyAuto-clear FTP queue at 50 records destroys monsoon data6🟠 Seriousscheduler.inoUncalibrated ADC for battery voltage7🟠 Seriousgprs.inoRTC drift midnight rollover produces wrong correction8🟠 Serioususer_config.hFORCE_2G compile flag incompatible with mixed SIM fleet9🟠 Seriousgprs.inoAirtel M2M silently skips all health reports + commands10🟠 Seriousauth.pyIn-memory session store not safe under multi-worker/restart11🟠 Seriousmain.pyOTA served over plain HTTP, no firmware integrity check17🟠 Seriousota_service.pyneeds_ota downgrades newer devices18🟠 Seriousgprs.inoRTC sync fixes time but not date30🟠 Seriousgprs.inomodemMutex give/take count mismatch31🟠 Seriousgprs.inoLock ordering inversion → deadlock risk37🟠 SeriousAIO9_5.0.inoSPIFFS.begin(true) silently destroys all data on mount fail38🟠 Serioususer_config.hDEBUG=1 ships to production, leaks keys over UART35🟠 Serioushealth_eval.pyOFFLINE threshold 24.5h means dead stations show green all day12🟡 Moderategprs_helpers.inoSPIFFS APN config has no atomic write — corruption on power loss13🟡 Moderateglobals.h106 RTC_DATA_ATTR vars — no compile-time RAM budget check15🟡 ModeratetempHum.inoJitter masks genuine stuck-sensor diagnostic20🟡 Moderatedeploy.shServer dashboard served on plain HTTP28🟡 Moderatehealth.pySchema introspection on every POST — SQLite lock contention29🟡 Moderatedashboard.pyevaluate() called twice per CSV row33🟡 Moderatedashboard.py400 records per page, each evaluated synchronously34🟡 ModerateAIO9_5.0.inoRTC_OK flag reused for SPIFFS failure — wrong alert41🟡 Moderategprs_helpers.inoCarrier string case mismatch — APN cross-check silently fails42🟡 Moderategprs.inoEmpty ftp_daily_date causes root dir iteration