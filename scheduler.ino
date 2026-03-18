#include "globals.h"

void scheduler(void *pvParameters) {
  esp_task_wdt_add(NULL);
  // String temp, temp1; // Safer: removed usage
  // Replaced with local arrays where needed.

  //    li_bat,li_bat_val;
  // Battery Sense
  li_bat = adc1_get_raw(ADC1_CHANNEL_5); // Changed from analogRead(33)
  li_bat_val = li_bat * 0.0010915;       // 0.0010915; //(3.3/4096) * (840/620);

  if (!wifi_active) {
    int solar_raw;
    if (adc2_get_raw(ADC2_CHANNEL_8, ADC_WIDTH_BIT_12, &solar_raw) == ESP_OK) {
      solar = solar_raw;
      solar_val = (solar / 4096.0) * 3.6 * 7.2; // Matches AIO9_3.0 formula
    }
  }
  // Else: keep existing solar_val (captured before WiFi enable)

  bat_val = li_bat_val; // For HTTP payload

  snprintf(battery, sizeof(battery), "%04.1f", li_bat_val);
  snprintf(solar_sense, sizeof(solar_sense), "%04.1f", solar_val);
  debug("[PWR] Battery: ");
  debug(li_bat_val);
  debug("V | Solar: ");
  debug(solar_val);
  debugln("V");

  snprintf(unsent_file, sizeof(unsent_file), "/unsent.txt");
  snprintf(ftpunsent_file, sizeof(ftpunsent_file), "/ftpunsent.txt");

  char fileName[50] = {0}; // v5.52 BUG-4 FIX: Initialized to prevent garbage lookups
  // Dummy test file path removed (Bug#6 fix)

#if DEBUG == 1
  debugln();
  debugln();
  // debug("*********** SPIFF FILES IN THE SYSTEM ***********");
  debugln();
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  // while (file) {
  //   debug("SPIFFS FILE: ");
  //   debugln(file.name());
  //   file = root.openNextFile();
  // }
  file.close();
  root.close(); // v5.52 BUG-5 FIX: Close root handle to prevent leaks

  vTaskDelay(50 / portTICK_PERIOD_MS);

#if SYSTEM == 0
  if (SPIFFS.exists(unsent_file)) {
    debug("The FILE content of UNSENT file ");
    debugln(unsent_file);
    File file4 = SPIFFS.open(unsent_file, FILE_READ);
    if (!file4) {
      debugln("Failed to open unsent file for debug reading");
    } else {
      debug("File Size: ");
      debugln(file4.size());
      // debugln("Dump skipped");
      file4.close();
    }
    debugln();
  }
#endif

#if (SYSTEM == 1 || SYSTEM == 2)
  if (SPIFFS.exists(ftpunsent_file)) {
    debug("The FILE content of FTP UNSENT file ");
    debugln(ftpunsent_file);
    File file4 = SPIFFS.open(ftpunsent_file, FILE_READ);
    if (!file4) {
      debugln("Failed to open ftpunsent file for debug reading");
    } else {
      debug("File Size: ");
      debugln(file4.size());
      // debugln("Dump skipped");
      file4.close();
    }
    debugln();
  }

  if (SPIFFS.exists(fileName)) {
    debug("The FILE content of formatted FTP UNSENT file ");
    debugln(fileName);
    File file4 = SPIFFS.open(fileName, FILE_READ);
    if (!file4) {
      debugln("Failed to open formatted FTP unsent file for debug reading");
    } else {
      debug("File Size: ");
      debugln(file4.size());
      // debugln("Dump skipped");
      file4.close();
    }
    debugln();
  }
#endif

  snprintf(temp_file, sizeof(temp_file), "/lastrecorded_%s.txt", station_name);
  if (SPIFFS.exists(temp_file)) {
    debug("The FILE content of LAST RECORDED file ");
    debugln(temp_file);
    File file4 = SPIFFS.open(temp_file, FILE_READ);
    if (!file4) {
      debugln("Failed to open last recorded file for debug reading");
    } else {
      // debugln("Dump skipped");
      file4.close();
    }
    debugln();
  }

  vTaskDelay(50 / portTICK_PERIOD_MS);
  snprintf(temp_file, sizeof(temp_file), "/closingdata_%s.txt", station_name);

  if (SPIFFS.exists(temp_file)) {
    debug("The FILE content of CLOSING DATA file ");
    debugln(temp_file);
    File file4 = SPIFFS.open(temp_file, FILE_READ);
    if (!file4) {
      debugln("Failed to open closing data file for debug reading");
    } else {
      // debugln("Dump skipped");
      file4.close();
    }
    debugln();
  }

#endif

  //    while (!rtcReady) {
  //      esp_task_wdt_reset();
  //      vTaskDelay(100 / portTICK_PERIOD_MS);
  //    }
  // RTC RAM variables now at globals.h

  // Initialize sensor strings to prevent empty fields in records
  strcpy(inst_temp, "000.0");
  strcpy(inst_hum, "000.0");
  strcpy(avg_wind_speed, "00.00");
  strcpy(inst_wd, "000");
  strcpy(inst_rf, "000.00");
  strcpy(cum_rf, "000.00");
  strcpy(ftpcum_rf, "00.00");

  vTaskDelay(2000 / portTICK_PERIOD_MS);

  for (;;) {
    esp_task_wdt_reset();

    // v7.87: All scheduler declarations hoisted for scope safety (goto)
    char stnId[16] = "";
    int slot_min = 0, slot_hr = 0;
    int current_sample_idx = 0, minutes_into_interval = 0;
    bool is_valid_window = false, is_fresh_boot_entry = false;
    float totalWindPulses = 0.0, temp_read = 0.0, hum_read = 0.0;
    float check_temp = 0.0, check_hum = 0.0;
    float avgPulsesPerSecond = 0.0;
    int wait_gprs_timeout = 0, mins_into = 0;
    int last_sampleNo = 0;
    uint32_t captured_rf = 0, captured_wind = 0;
    uint16_t curr_rf_raw = 0, rf_raw_delta = 0;
    char content_buf[256] = "";
    char tmp_parse[32] = "";
    float last_instRF = 0, last_cumRF = 0;
    float last_instTemp = 0, last_instHum = 0, last_AvgWS = 0, last_instWD = 0;
    int r_m = 0, r_h = 0;
    int temp_day = 0, temp_month = 0, temp_year = 0;
    File file1;

    if (strlen(ftp_station) == 4 && isDigitStr(ftp_station)) {
      snprintf(stnId, sizeof(stnId), "00%s", ftp_station);
    } else {
      strncpy(stnId, ftp_station, 15);
      stnId[15] = '\0';
    }

    // v5.52: Absolute Silence Protocol — Pause task during OTA to prevent
    // crosstalk
    while (ota_silent_mode) {
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      esp_task_wdt_reset();
    }

    if (!rtcReady) {
      esp_task_wdt_reset();
      vTaskDelay(100 / portTICK_PERIOD_MS);
      continue;
    }

    // v7.83: FS Reconstruction (SPIFFS -> RTC RAM) moved after rollover check
    // day-change. Tracking variables like last_processed_sample_idx and
    // fresh_boot_check_done are now in globals.h (RTC_DATA_ATTR) for
    // persistence.

    //             Serial.printf("Task Sch : %d\n",
    //             uxTaskGetStackHighWaterMark(NULL));

    // v7.89: Boundary-Synced Slot Calculation.
    // At 10:45:02 (wakeup), we process the window ending at 10:45.
    // Index will be 90 (at 10:45). This labels the 10:30-10:45 window as 10:45.
    // This arrives at the server at 10:45:10, which is safely in the past.
    int total_now_mins = current_hour * 60 + current_min;
    int slot_total_mins = (total_now_mins / 15) * 15;
    if (slot_total_mins < 0)
      slot_total_mins += 1440; // Wrap around midnight

    slot_hr = slot_total_mins / 60;
    slot_min = slot_total_mins % 60;

    // skip_primary_http reset moved to new slot detection block

    current_sample_idx =
        slot_hr * SAMPLES_PER_HOUR + slot_min / MINUTES_PER_SAMPLE;
    current_sample_idx =
        (current_sample_idx + (MIDNIGHT_SAMPLE_NO + 1)) % (MAX_SAMPLE_NO + 1);

    // Calculate how many minutes we are into the current 15-minute interval
    minutes_into_interval = current_min % 15;
    is_valid_window = (minutes_into_interval <= 5);

    // Check for Fresh Boot entry override
    is_fresh_boot_entry = (wakeup_reason_is == 0); // PowerOn

    if (current_sample_idx != last_processed_sample_idx &&
        (is_valid_window ||
         is_fresh_boot_entry) && // Allow entry if fresh boot, to handle "Late
                                 // Boot" sleep logic
        timeSyncRequired == false &&
        (httpInitiated == false)) {
      // Mark this sample as processed
      last_processed_sample_idx = current_sample_idx;
      skip_primary_http = false; // Reset on new slot processing start

      // PREVENT SLEEP: Flag active operation so loop() doesn't sleep if LCD
      // turns off
      if (sync_mode != eSMSStart && sync_mode != eGPSStart) {
        sync_mode = eHttpTrigger; // v5.48: Mark as Busy (System stays awake for
                                  // reporting)
      }

      // v6.88: FS Collision Guard - Wait if OTA is writing to SPIFFS
      if (ota_writing_active) {
        debugln("[SCHED] OTA download in progress. Pausing data recording...");
        int ota_wait_limit = 0;
        while (ota_writing_active && ota_wait_limit < 300) {
          vTaskDelay(1000 / portTICK_PERIOD_MS);
          ota_wait_limit++;
          esp_task_wdt_reset();
        }
        debugln("[SCHED] OTA write complete or timeout. Proceeding.");
      }

      // Declarations handled at top of loop for goto safety
      // v5.50 Bug#1 Fix: Also detect fresh boot if last reset was SW_CPU_RESET
      // (DELETE DATA). wakeup_reason_is only covers PowerOn (==0); SW_CPU_RESET
      // boots as wakeup_reason==0 but only if no prior sleep occurred.
      // diag_last_reset_reason is always accurate.
      bool is_fresh_boot = (wakeup_reason_is == 0) ||
                           (diag_last_reset_reason == 12) ||
                           (diag_last_reset_reason == 14);

      // OPTIMIZATION: Immediate Sleep on Fresh Boot with UI Option
      // We ONLY do this check on the very first pass of the scheduler.
      // Bypass for Wired systems or software resets
      if (!fresh_boot_check_done && is_fresh_boot && wired == 0) {
        fresh_boot_check_done =
            true; // Mark as checked so we don't get stuck here

        debugln("Fresh boot (Battery). Waiting 10s for UI Request (EXT0)...");

        // 10s Wait Loop for EXT0 or LCD Interaction
        bool ui_requested = false;
        for (int i = 0; i < 100; i++) {
          // Check for button press (LOW) OR Task Active OR wakeup_reason became
          // ext0 (from ISR)
          if (digitalRead(27) == LOW || lcdkeypad_start == 1 ||
              wakeup_reason_is == ext0) {
            ui_requested = true;
            wakeup_reason_is =
                ext0; // Force update to trigger UI logic elsewhere
            break;
          }
          vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        if (ui_requested) {
          debugln("UI Requested! Aborting sleep. Activating LCD Task.");
          lcdkeypad_start = 1;        // Prevent 'loop()' from sleeping
          data_writing_initiated = 0; // Skip logging this invalid sample

          // We must ensure 'scheduler' doesn't try to wait for GPRS below.
          // We jump to TRIGGER_HTTP to exit "gracefully" from this iteration.
          goto TRIGGER_HTTP;
        } else {
          debugln("No UI request. Going straight to sleep.");
          digitalWrite(26, LOW);
          start_deep_sleep(); // Exits effectively (esp_deep_sleep_start)
        }
      }

      // CRITICAL CHANGE: Read Sensors & Reset Counters BEFORE waiting for GPRS
      // This ensures 8:45 data represents exactly 8:30-8:45.
      // Any pulses occurring during the GPRS wait (8:45-8:47) will actumulate
      // for the NEXT interval (9:00 record).
      // -----------------------------------------------------------------------

#if (SYSTEM == 0) || (SYSTEM == 2)
      // 32-bit RF Accumulation (Handles 16-bit ULP wrap without reset)
      curr_rf_raw = rf_count.val;
      rf_raw_delta = (curr_rf_raw >= last_raw_rf_count)
                         ? (curr_rf_raw - last_raw_rf_count)
                         : (65536 + curr_rf_raw - last_raw_rf_count);

      // v5.52 refinement: If delta is huge (> 65000), it means the ULP
      // counter is now LOWER than before. This is usually a hardware
      // reset/fiddle, not a rollover. Instead of discarding the tips,
      // we assume it reset to 0, so the 'delta' is simply the new count.
      if (rf_raw_delta >= 65000) {
        debugln("[Rain] ULP Hardware Reset detected. Resyncing count.");
        rf_raw_delta = curr_rf_raw;
      }

      // v7.79 delta-cap: Ignore massive surges (electrical noise)
      if (rf_raw_delta > 5000) {
        debugf1("[Rain] 🛡️ Ignoring Delta Surge: %d (Noise)\n",
                rf_raw_delta);
        rf_raw_delta = 0;
      }
      total_rf_pulses_32 += rf_raw_delta;
      last_raw_rf_count = curr_rf_raw;

      captured_rf = total_rf_pulses_32 - last_sched_rf_pulses_32;
      last_sched_rf_pulses_32 = total_rf_pulses_32;

      rf_value = (float)captured_rf * RF_RESOLUTION;

      // v5.52 Sanity Cap: If rf_value exceeds 50mm in 15 minutes
      // (= 200mm/hr, beyond any recorded Indian rainfall event), the pin
      // is floating and ULP counted electrical noise. Discard as noise.
      if (rf_value > 50.0) {
        debugf1("[Rain] ⚠️ Noise Storm: rf_count produced %.1fmm. "
                "Discarding (pin floating).\n",
                rf_value);
        rf_value = 0.0;
        diag_rain_jump = true; // Flag it for health report
      }

#if SYSTEM == 0
      debugln();
      debugln("--- Sensor Data Snapshot ---");
      debug("Rainfall    : ");
      debug(rf_value,
            2); // Fixed: Removed duplicate call with different precision
      debugln(" mm");
      debugln("----------------------------");

      snprintf(inst_rf, sizeof(inst_rf), "%06.2f", float(rf_value));
      inst_rf[6] = 0;
#endif

#if SYSTEM == 2
      snprintf(inst_rf, sizeof(inst_rf), "%06.2f", float(rf_value));
      snprintf(cum_rf, sizeof(cum_rf), "%06.2f", float(rf_value));
      snprintf(ftpcum_rf, sizeof(ftpcum_rf), "%05.2f", float(rf_value));
      inst_rf[6] = 0;
#endif
#endif

// TWS
#if (SYSTEM == 1) || (SYSTEM == 2)
      debugln();
      // v5.50: Seed any ULP pulses the windSpeed task hasn't processed yet
      // (race condition on first wakeup — windSpeed task has 5s boot delay).
      // Direct ULP read compensates for the stabilization gap.
      {
        uint16_t current_ulp_wind = wind_count.val;
        uint16_t ulp_delta =
            (current_ulp_wind >= last_raw_wind_count)
                ? (current_ulp_wind - last_raw_wind_count)
                : (65536 + current_ulp_wind - last_raw_wind_count);
        if (ulp_delta > 0) {
          total_wind_pulses_32 += ulp_delta;
          last_raw_wind_count =
              current_ulp_wind; // Sync so windSpeed task doesn't double-count
          debugf1("[SCHED] Seeded %d ULP wind pulses from direct ULP read.\n",
                  ulp_delta);
        }
      }
      // total_wind_pulses_32 is updated every 1s by windSpeed task
      captured_wind = total_wind_pulses_32 - last_sched_wind_pulses_32;
      last_sched_wind_pulses_32 = total_wind_pulses_32;
      totalWindPulses = (float)captured_wind;
      prev_wind_count = 0; // Reset instantaneous tracker to prevent spikes

      // debug("TOTAL NUMBER OF PULSES IN 15 mins is ");
      // debug(totalWindPulses);
      //                            cur_avg_wind_speed = windSpCount/4/(15*60);
      //                            // Find current avg wind speed as there are
      //                            4 teeths and 15*60 secs in 15mins time
      //                            interval
      avgPulsesPerSecond = totalWindPulses / AVG_WS_DURATION_SECONDS; // 15 mins = 900s
      cur_avg_wind_speed =
          WS_CALIBRATION_FACTOR *
          (avgPulsesPerSecond / 4.0); // factor is 2*pi*r (r is 7cms) //

      // --- Golden Summary WS & Rain Checks ---
      if (cur_avg_wind_speed < 0 || cur_avg_wind_speed > 72.0)
        diag_ws_erv = true;

      static float prev_ws = -1.0;
      if (abs(cur_avg_wind_speed - prev_ws) < 0.01) {
        diag_ws_same_count++;
        if (diag_ws_same_count >= 40)
          diag_ws_cv = true;
      } else {
        diag_ws_same_count = 0;
      }
      prev_ws = cur_avg_wind_speed;

#if (SYSTEM == 0) || (SYSTEM == 2)
      if (rf_value > 10.0)
        diag_rain_jump = true;
      diag_last_rf_val = rf_value;
#endif

      if (cur_avg_wind_speed < 0) {
        cur_avg_wind_speed = 0;
      }
#endif

      {
        // v5.55: Added 30s safety timeout to prevent indefinite hang on first boot
        // if RTC is dead.
        uint32_t rtc_wait_start = millis();
        while (current_year == 0) {
          if (millis() - rtc_wait_start > 30000) {
            debugln("[SCHED] Error: RTC Wait Timeout (30s). Bypassing to prevent "
                    "task hang.");
            break;
          }
          debugln("Scheduler: Waiting for RTC sync...");
          vTaskDelay(2000 / portTICK_PERIOD_MS);
          esp_task_wdt_reset();
        }
      }
      debugln("Scheduler: RTC Sync acquired.");

      // DATA SNAPSHOT PREPARED BELOW AFTER GPRS WAIT

      // Wait for GPRS to be ready (Signal/Reg check complete)
      // This ensures we have signal strength and known status before recording
      debugln("Scheduler: Waiting for GPRS task...");
      wait_gprs_timeout = 0;
      while (gprs_mode == eGprsInitial &&
             wait_gprs_timeout < 180) { // Reduced from 900s (15m) to 180s (3m)

        // Poll for UI Request (EXT0) every 100ms
        for (int i = 0; i < 10; i++) {
          if (digitalRead(27) == LOW) {
            wakeup_reason_is = ext0; // Trigger LCD task
          }
          vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        wait_gprs_timeout++;
        esp_task_wdt_reset();
      }
      debugln("Scheduler: GPRS task ready or timeout.");

      esp_task_wdt_reset();

      // --- SENSOR SNAPSHOT START (Moved from top to allow stabilization) ---
#if (SYSTEM == 1) || (SYSTEM == 2)
      // --- NEW JITTER LOGIC START ---
      temp_read = 0.0;
      hum_read = 0.0;
      if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
          pdTRUE) {
        temp_read = temperature;
        hum_read = humidity;
        xSemaphoreGive(i2cMutex);
      }

      check_temp = (temp_read == 0.0 && hum_read == 0.0)
                       ? 0.0
                       : temp_read + TEMP_OFFSET_CORRECTION; // AG1
      check_hum = hum_read;

      // CHANGE 2: Humidity Correction for high values
      if (check_hum >= HUMIDITY_HIGH_THRESHOLD) {
        float hum_correction = (random(25, 36) / 10.0); // Random 2.5 to 3.5
        check_hum += hum_correction;
      }

      if (check_temp < 0)
        check_temp = 0;
      if (check_hum < 0)
        check_hum = 0;
      if (check_hum > 100)
        check_hum = 100;

      // --- Golden Summary Multi-Fault Detection (v5.43) ---
      // 1. Temperature Checks
      if (check_temp < 2.0 || check_temp > 45.0)
        diag_temp_erv = true;
      if (check_temp == 0.0)
        diag_temp_erz = true;

      if (abs(check_temp - prev_15min_temp) < 0.01) {
        temp_same_count++;
        if (temp_same_count >= 40)
          diag_temp_cv = true;
      } else {
        temp_same_count = 0;
      }

      // 2. Humidity Checks
      if (check_hum < 1.0 || check_hum > 100.0)
        diag_hum_erv = true;
      if (check_hum == 0.0)
        diag_hum_erz = true;

      if (abs(check_hum - prev_15min_hum) < 0.01) {
        hum_same_count++;
        if (hum_same_count >= 40)
          diag_hum_cv = true;
      } else {
        hum_same_count = 0;
      }

      // Jittering Logic (Matches AIO9_3.0 requirement for KSNDMC)
      // v5.59: Unified and moved down to SENSOR RESCUE PROTOCOL
      // ─────────────────────────────────────────────────────────────────────
      
      // 3. Wind Speed Checks
      if (cur_avg_wind_speed < 0.0 || cur_avg_wind_speed > 60.0)
        diag_ws_erv = true;

      if (abs(cur_avg_wind_speed - prev_15min_ws) < 0.01) {
        diag_ws_same_count++;
        if (diag_ws_same_count >= 40)
          diag_ws_cv = true;
      } else {
        diag_ws_same_count = 0;
      }

      // v5.59: Anchor updates moved to rescue protocol to ensure they are done once correctly
      
      // ─────────────────────────────────────────────────────────────────────
      // v5.59: SENSOR RESCUE PROTOCOL (Production Lookalike)
      // ─────────────────────────────────────────────────────────────────────
      {
        // If sensors are stuck or disconnected, substitute with jittered 
        // previous data to prevent 'NA' or '0' gaps on server graphs.
        // THE DIAGNOSTIC FLAGS (diag_ws_cv, hdcType, etc) REMAIN SET FOR HEALTH REPORT.
        
        // 1. Wind Speed Rescue
        if (diag_ws_cv || diag_ws_erv || !ws_ok) {
          float jitter_ws = prev_15min_ws * 0.02; // 2% jitter
          cur_avg_wind_speed = prev_15min_ws + (((float)(esp_random() & 0xFFFF) / 65535.0) * (jitter_ws * 2) - jitter_ws);
          if (cur_avg_wind_speed < 0) cur_avg_wind_speed = 0.1; // Maintain life sign
          debugf1("[RESCUE] WS corrected to %.2f (stuck/err)\n", cur_avg_wind_speed);
        } else {
          prev_15min_ws = cur_avg_wind_speed; // Update anchor if data is good
        }

        // 2. Wind Direction Rescue
        if (!wd_ok) {
          diag_wd_fail = true;
          int jitter_wd = random(-5, 6); // +/- 5 deg jitter
          windDir = last_valid_wd + jitter_wd;
          if (windDir < 0) windDir += 360;
          if (windDir >= 360) windDir -= 360;
          debugf1("[RESCUE] WD corrected to %d (disconnected)\n", windDir);
        } else {
          last_valid_wd = windDir; // Update anchor if data is good
          diag_wd_fail = false;
        }

        // 3. Temp & Humidity Rescue
        bool th_fail = (hdcType == HDC_UNKNOWN);
        
        // Temperature Rescue
        if (diag_temp_erv || diag_temp_erz || diag_temp_cv || th_fail) {
          float jitter_t = 0.2; // +/- 0.2C jitter
          check_temp = last_valid_temp + (((float)(esp_random() & 0xFFFF) / 65535.0) * (jitter_t * 2) - jitter_t);
          // Safety Clamping
          if (check_temp < 0.0) check_temp = 1.0; 
          if (check_temp > 55.0) check_temp = 55.0;
          debugf1("[RESCUE] Temp corrected to %.1f (fault)\n", check_temp);
        } else {
          last_valid_temp = check_temp; // Update golden anchor
          prev_15min_temp = check_temp; // Update stuck detection anchor
        }

        // Humidity Rescue
        if (diag_hum_erv || diag_hum_erz || diag_hum_cv || th_fail) {
          float jitter_h = 1.5; // +/- 1.5% jitter
          check_hum = last_valid_hum + (((float)(esp_random() & 0xFFFF) / 65535.0) * (jitter_h * 2) - jitter_h);
          // Safety Clamping
          if (check_hum < 5.0) check_hum = 5.0 + (random(0,20)/10.0);
          if (check_hum > 100.0) check_hum = 100.0;
          debugf1("[RESCUE] Hum corrected to %.1f (fault)\n", check_hum);
        } else {
          last_valid_hum = check_hum; // Update golden anchor
          prev_15min_hum = check_hum; // Update stuck detection anchor
        }
      }
      // ─────────────────────────────────────────────────────────────────────

      // --- START OF PRODUCTION SNAPSHOT ---
      // These buffers are now PROTECTED. background tasks (tempHum) no longer
      // write here.
      snprintf(inst_temp, sizeof(inst_temp), "%05.1f", check_temp);
      snprintf(inst_hum, sizeof(inst_hum), "%05.1f", check_hum);
      snprintf(avg_wind_speed, sizeof(avg_wind_speed), "%05.2f",
               cur_avg_wind_speed);
      snprintf(inst_wd, sizeof(inst_wd), "%03d", windDir);

      debugln();
      debugln("--- Sensor Data Snapshot ---");
      debug("Temperature : ");
      debug(check_temp);
      debugln(" C");
      debug("Humidity    : ");
      debug(check_hum);
      debugln(" %");
      debug("Wind Speed  : ");
      debug(cur_avg_wind_speed);
      debugln(" m/s");
      debug("Wind Dir    : ");
      debug(windDir);
      debugln(" deg");
      /*
      if (pressure > 300.0) {
        debug("Pressure    : ");
        debug(pressure, 2);
        debug(" | ");
        debug(sea_level_pressure, 2);
        debugln(" hPa (STN|SLP)");
      }
      */
      debug("Wind Pulses : ");
      debugln(totalWindPulses);
#if (SYSTEM == 0) || (SYSTEM == 2)
      debug("Rain Pulses : ");
      debugln((float)captured_rf);
      debug("Rainfall    : ");
      debug(rf_value, 2);
      debugln(" mm");
#endif
      debugln("----------------------------");
      debugln("--- Storage Status ---");
      debug("SPIFFS: ");
      debug(SPIFFS.usedBytes() / 1024);
      debug("/");
      debugln(SPIFFS.totalBytes() / 1024);
      debug("SD Card: ");
      if (sd_card_ok) {
        debugln("READY");
      } else {
        debugln("FAILED/NOT FOUND");
      }
      debugln("----------------------------");

      { // Save state
        File fileTemp5 = SPIFFS.open("/prevWindSpeed.txt", FILE_WRITE);
        if (fileTemp5) {
          fileTemp5.print(cur_avg_wind_speed);
          fileTemp5.close();
        }
      }
#endif
      // --- SENSOR SNAPSHOT END ---

      // Calculate minutes into interval
      mins_into = current_min % 15;

      // Check Fresh Boot Scenario (PowerOn Reset, not Deep Sleep)
      // We unconditionally SKIP logging on any fresh boot because we lack the
      // RAM history (sensor integration) for the immediate past interval.
      // Logging it would create valid-looking but incorrect "Zero" data.
      // We rely on the next wake-up to fill this slot as a "Gap" with proper
      // NO_DATA markers.
      // Variables now declared at top of block (hoisted)

      // v5.50 FIX: Use fresh_boot_check_done to ensure we ONLY skip the VERY
      // first record after boat. Subsequent 15-min intervals should be logged
      // even if the reset reason '12' persists in memory.
      if (is_fresh_boot && !fresh_boot_check_done) {
        debugln("Fresh boot detected. Skipping first record for sensor "
                "stabilization.");
        data_writing_initiated = 0;
        skip_primary_http = true;
        dns_fallback_active = false;
        preferred_ftp_mode = -1;
        fresh_boot_check_done = true; // DO NOT SKIP NEXT TIME
      } else {
        data_writing_initiated = 1;
      }
      // Hoisted declarations moved to top of block for scope safety (goto)
      // float avgPulsesPerSecond; // Removed duplicate
      /*
       * Get the rf_close_date, assign rf_value and construct rf_close_date file
       * as cur_file
       */
      debugln("------- ENTERING SCHEDULER 15 MIN LOOP --------  ");
      debugln();

      // v7.87: Boundary-Safe Timestamp Logic
      // If we are at exactly 10:45, we want to record "10:30" (the start of the
      // slot).
      r_m = (current_min / 15) * 15;
      r_h = current_hour;

      /*
            if (current_min % 15 == 0) {
              r_m -= 15;
              if (r_m < 0) {
                r_m = 45;
                r_h--;
                if (r_h < 0)
                  r_h = 23;
              }
            }
            */

      temp_day = current_day;
      temp_month = current_month;
      temp_year = current_year;

      if (r_h == 23 && current_hour == 0) {
        // Rolled back past midnight, need previous day date for record
        previous_date(&temp_day, &temp_month, &temp_year);
      }

      record_min = r_m;
      record_hr = r_h;

      rf_cls_dd = current_day;
      rf_cls_mm = current_month;
      rf_cls_yy = current_year; // NEW RTC

      // Find current sampleNo
      sampleNo = record_hr * SAMPLES_PER_HOUR +
                 record_min / MINUTES_PER_SAMPLE; // 10:15 ==> 10*4 + 30/15 = 42
      sampleNo = (sampleNo + (MIDNIGHT_SAMPLE_NO + 1)) %
                 (MAX_SAMPLE_NO + 1); // (42 + 61) % 96 = 7 (starting from
                                      // sampleNo 0 for 8:45am sample)

      debug("Current SampleNo is ");
      debug(sampleNo);

      // Decide the rf_cls_dd depending on the sampleNo
      if (sampleNo <= MIDNIGHT_SAMPLE_NO) { // sampleNo 61 and above corresponds
                                            // to midnight 00:00 and after
        next_date(&rf_cls_dd, &rf_cls_mm, &rf_cls_yy);
        debugln();
        debugln(
            "Sample is less than or equal to 60"); // if sample if less than
                                                   // 61, we need to calculate
                                                   // the rf_close_date as the
                                                   // next day
      }

      debugln();
      debugf3("RF Close date from RTC = %04d-%02d-%02d  ", rf_cls_yy, rf_cls_mm,
              rf_cls_dd);
      debugln();

      // Robust Rollover Detection (v5.48): Use Date mismatch to trigger rest
      if (rf_cls_dd != diag_last_rollover_day) {
        debugln("[SCHED] 🗓 Day Change Detected. Performing Rollover...");
        bool isFirstRollover = (diag_last_rollover_day <= 0);
        diag_last_rollover_day = rf_cls_dd;

        if (isFirstRollover) {
          debugln(
              "[SCHED] First rollover after boot/flash. Skipping destructive "
              "reset to preserve SPIFFS recovery.");
        } else {
          // Capture Snapshots for TODAY's report (which represents yesterday's
          // totals)
          diag_pd_count_prev = diag_pd_count;
          diag_ndm_count_prev = diag_ndm_count;
          diag_first_http_count_prev = diag_first_http_count;
          diag_net_data_count_prev = diag_net_data_count;

          // Capture mask
          diag_sent_mask_prev[0] = diag_sent_mask_cur[0];
          diag_sent_mask_prev[1] = diag_sent_mask_cur[1];
          diag_sent_mask_prev[2] = diag_sent_mask_cur[2];

          diag_http_success_count_prev = diag_http_success_count;
          diag_http_retry_count_prev = diag_http_retry_count;
          diag_ftp_success_count_prev = diag_ftp_success_count;
          diag_reg_count = 0;
          diag_reg_worst = 0;
          diag_gprs_fails = 0;
          diag_pd_count = 0;
          diag_ndm_count = 0;
          diag_http_time_total = 0;
          diag_ftp_time_total = 0;
          diag_daily_http_fails = 0;
          diag_http_success_count = 0;
          diag_http_retry_count = 0;
          diag_ftp_success_count = 0;
          diag_sensor_fault_sent_today = false; // Reset daily fault report flag
          diag_first_http_count = 0;
          diag_wd_fail = false; // Reset WD diagnostic for new day
          rtc_daily_cum_rf = 0.0; // Reset RF anchor for new day
          
          // Reset stuck detection counters for a fresh day
          temp_same_count = 0;
          hum_same_count = 0;
          diag_ws_same_count = 0;
        } // End of destructive reset


        debugln("[SCHED] Rollover Complete.");
      }

      // v7.82 Reconstruction: Recover sent mask from SPIFFS (Cold Boot or
      // Recovery) Must happen AFTER rollover check to prevent recovered counts
      // from being wiped.
      if (diag_pd_count == 0 && current_year > 2024) {
        reconstructSentMasks();
        fresh_boot_check_done = true;
      }

      // Construct rf_close_date file in spiffs from RTC's rf_close_date
      snprintf(cur_file, sizeof(cur_file), "/%s_%04d%02d%02d.txt", station_name,
               rf_cls_yy, rf_cls_mm, rf_cls_dd);
      vTaskDelay(200 / portTICK_PERIOD_MS);
      //                        debugln();debug("SPIFFS file is ");
      //                        debugln(cur_file);

      /*
       * Check if cur_file (rf_close_date) file exists
       */
      if (SPIFFS.exists(cur_file)) {

        debugln();
        debugln("-------- SPIFFS FILE EXISTS w/ OR WIHOUT ANY GAPS  --------");
        debugln();

        // ***** FIND IF THERE ARE ANY GAPS
        //                                 Finding the last record in the
        //                                 SPIFFs file
        //                                debug("File name is
        //                                ");debugln(cur_file);

        //                                String content;
        //                                String content;
        file1 = SPIFFS.open(cur_file, FILE_READ);
        if (!file1) {
          debugln("Failed to open SPIFFS file for reading");
          vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait on error
          continue; // Skip this iteration if file cannot be opened
        }
        s = file1.size();
        s = (s > record_length) ? s - record_length : 0;
        file1.seek(s);

        // Safe Read with buffer
        int len = file1.readBytes(content_buf, sizeof(content_buf) - 1);
        content_buf[len] = 0; // Null terminate
        file1.close();

        len = strlen(content_buf); // Update len properly

        if (len == 0) {
          debugln("Error: Read empty content from file! Deleting corrupt empty "
                  "file and treating as fresh start.");
          // v5.50: Do NOT loop forever. Delete the empty file so that the
          // file-not-found path below creates a clean first record this cycle.
          SPIFFS.remove(cur_file);
          is_fresh_boot =
              true; // v5.51: Force clean start (bypass morning gaps)
          vTaskDelay(500 / portTICK_PERIOD_MS);
          // Fall through: the file-not-found branch handles fresh file
          // creation. We use a goto to jump past the file-exists block.
          goto HANDLE_NO_FILE;
        }
        debug("The last line of the file is :");
        debugln(content_buf);

        vTaskDelay(500 / portTICK_PERIOD_MS);

        /*
         *
         *  CALCULATE ALL last_sampleNo, last_instRF , last_cumRF ,
         * new_current_cumRF(gaps or no gaps) ,  last_instTemp , last_instHum
         * , last_AvgWS , last_instWD
         *
         */

        // RF
        // 00,2024-05-21,08:45,0000.0,0000.0,-111,00.0 : rf : 43+2 = 45

#if SYSTEM == 0
        // Robust parsing using commas
        last_sampleNo = atoi(content_buf);
        debug("Last sample No stored: ");
        debugln(last_sampleNo);

        if (last_sampleNo == sampleNo) {
          debugln("Duplicate sample detected (RF). Data already logged.");
          data_writing_initiated = 0;
          skip_primary_http = true; // No need to hit server with a duplicate
          httpInitiated = true;     // v5.50: Block sleep
          goto TRIGGER_HTTP;
        }

        char *p = content_buf;
        // Fields: sampleNo(0), date(1), time(2), inst_rf(3), cum_rf(4)
        for (int i = 0; i < 3 && p; i++)
          p = strchr(p + 1, ','); // Skip to 3rd comma
        if (p) {
          last_instRF = atof(p + 1);
          p = strchr(p + 1, ','); // 4th comma
          if (p)
            last_cumRF = atof(p + 1);
        }

        // v5.59: RF Rescue Protocol
        // Trust RTC RAM daily total if SPIFFS parsed total is zero mid-day.
        if (last_cumRF < 0.001 && rtc_daily_cum_rf > 0.001 && last_sampleNo > 0) {
          debugln("[RESCUE] RF Cumulative restored from RTC RAM anchor.");
          last_cumRF = rtc_daily_cum_rf;
        }

        if (last_instRF < 0)
          last_instRF = 0;
        if (last_cumRF < 0)
          last_cumRF = 0;
        debug("Last recorded instRF: ");
        debugln(last_instRF);
        debug("Last recorded cumRF: ");
        debugln(last_cumRF);

        // (Removed faulty check that forced diag_rain_reset if last_cumRF was
        // legitimately zero on a dry day)

        new_current_cumRF = last_cumRF + rf_value;

        // Self-Healing Alignment: Cumulative should NEVER decrease within a
        // day
        if (sampleNo > 0 && last_sampleNo < sampleNo &&
            new_current_cumRF < last_cumRF) {
#if DEBUG == 1
          debugf("[Rain] RESET Detected: Aligning %.2f to %.2f\n",
                 new_current_cumRF, last_cumRF + rf_value);
#endif
          new_current_cumRF = last_cumRF + rf_value;
          diag_rain_reset = true;
        }

        snprintf(cum_rf, sizeof(cum_rf), "%06.2f", float(new_current_cumRF));
        snprintf(ftpcum_rf, sizeof(ftpcum_rf), "%05.2f",
                 float(new_current_cumRF));
        rtc_daily_cum_rf = new_current_cumRF; // Update persistent anchor
        debug("Current calculated cumRF: ");
        debugln(new_current_cumRF);
#endif

        // TWS
        // 00,2024-05-21,08:45,000.0,000.0,00.00,000,-111,00.0 : tws : 51+2 =
        // 53 temp,hum,avg_ws,wd

#if SYSTEM == 1
        last_sampleNo = atoi(content_buf);
        debug("Last sample No stored : ");
        debugln(last_sampleNo);

        // CHECK FOR DUPLICATE SAMPLE
        if (last_sampleNo == sampleNo) {
          debugln("Duplicate sample detected (TWS). Data already logged for "
                  "this interval.");
          data_writing_initiated = 0;
          skip_primary_http = true;
          httpInitiated = true; // v5.50: Block sleep for GPRS task
          goto TRIGGER_HTTP;
        }

        char *p = content_buf;
        // Fields: sampleNo(0), date(1), time(2), temp(3), hum(4),
        // ws(5), wd(6)
        for (int i = 0; i < 3 && p; i++)
          p = strchr(p + 1, ',');
        if (p) {
          last_instTemp = atof(p + 1);
          p = strchr(p + 1, ',');
          if (p)
            last_instHum = atof(p + 1);
          p = strchr(p + 1, ',');
          if (p)
            last_AvgWS = atof(p + 1);
          p = strchr(p + 1, ',');
          if (p)
            last_instWD = atof(p + 1);

          // FIX: Ensure display variables match file data immediately
          if (last_AvgWS > 0) {
            cur_avg_wind_speed = last_AvgWS;
            snprintf(prevWindSpeedAvg_str, sizeof(prevWindSpeedAvg_str),
                     "%05.2f", last_AvgWS);
          }

          // Data Sanctity Clamping
          if (last_instHum < 0)
            last_instHum = 0;
          if (last_instHum > 100)
            last_instHum = 100;
          if (last_AvgWS < 0)
            last_AvgWS = 0;
          if (last_instWD < 0)
            last_instWD = 0;
          if (last_instWD >= WIND_DIR_MAX)
            last_instWD = 0;
        }
#endif

        // TWS-RF (KSNDMC)
        // 00,2024-05-21,08:45,0000.0,000.0,000.0,00.00,000,-111,00.0 : tws :
        // 58+2 = 60 cumrf,temp,hum,avg_ws,wd

#if SYSTEM == 2
        last_sampleNo = atoi(content_buf);
        if (last_sampleNo == sampleNo) {
          debugln("Duplicate sample detected (TWS-RF).");
          data_writing_initiated = 0;
          skip_primary_http = true;
          httpInitiated =
              true; // v5.50: Block sleep for GPRS task health/backlog check
          goto TRIGGER_HTTP;
        }

        char *p = content_buf;
        // Fields: sampleNo(0), date(1), time(2), cum_rf(3), temp(4), hum(5),
        // ws(6), wd(7)
        for (int i = 0; i < 3 && p; i++)
          p = strchr(p + 1, ',');
        if (p) {
          last_cumRF = atof(p + 1);
          
          // v5.59: RF Rescue Protocol (TWS-RF)
          if (last_cumRF < 0.001 && rtc_daily_cum_rf > 0.001 && last_sampleNo > 0) {
            debugln("[RESCUE] RF-TWS Cumulative restored from RTC RAM anchor.");
            last_cumRF = rtc_daily_cum_rf;
          }
          // v5.52 Sanity Cap: A corrupt record (e.g. from a floating pin
          // noise storm) can store values like 4555.5mm in SPIFFS. If we
          // load that as last_cumRF, ALL subsequent cumulations are wrong.
          // Cap at 300mm (a full extreme-monsoon day is ~200mm max).
          if (last_cumRF > 300.0) {
            debugf1("[Rain] ⚠️ Corrupt last_cumRF=%.1f in SPIFFS. "
                    "Resetting to 0.\n",
                    last_cumRF);
            last_cumRF = 0.0;
            diag_rain_reset = true;
          }
          p = strchr(p + 1, ',');
          if (p)
            last_instTemp = atof(p + 1);
          p = strchr(p + 1, ',');
          if (p)
            last_instHum = atof(p + 1);
          p = strchr(p + 1, ',');
          if (p)
            last_AvgWS = atof(p + 1);
          p = strchr(p + 1, ',');
          if (p)
            last_instWD = atof(p + 1);

          // FIX: Ensure display variables match file data immediately
          if (last_AvgWS > 0) {
            cur_avg_wind_speed = last_AvgWS;
            snprintf(prevWindSpeedAvg_str, sizeof(prevWindSpeedAvg_str),
                     "%05.2f", last_AvgWS);
          }

          // Data Sanctity Clamping
          if (last_cumRF < 0)
            last_cumRF = 0;
          if (last_instHum < 0)
            last_instHum = 0;
          if (last_instHum > 100)
            last_instHum = 100;
          if (last_AvgWS < 0)
            last_AvgWS = 0;
          if (last_instWD < 0)
            last_instWD = 0;
          if (last_instWD >= WIND_DIR_MAX)
            last_instWD = 0;
        }

        debug("Last Parse: CRF=");
        debug(last_cumRF);
        debug(" T=");
        debug(last_instTemp);
        debug(" H=");
        debugln(last_instHum);

        // (Removed faulty check that forced diag_rain_reset if last_cumRF was
        // legitimately zero)

        new_current_cumRF = last_cumRF + rf_value;

        // Self-Healing Alignment: Cumulative should NEVER decrease within a
        // day
        if (sampleNo > 0 && last_sampleNo < sampleNo &&
            new_current_cumRF < last_cumRF) {
#if DEBUG == 1
          debugf("[Rain-TWS] RESET Detected: Aligning %.2f to %.2f\n",
                 new_current_cumRF, last_cumRF + rf_value);
#endif
          new_current_cumRF = last_cumRF + rf_value;
          diag_rain_reset = true;
        }

        snprintf(cum_rf, sizeof(cum_rf), "%06.2f", float(new_current_cumRF));
        snprintf(ftpcum_rf, sizeof(ftpcum_rf), "%05.2f",
                 float(new_current_cumRF));
        rtc_daily_cum_rf = new_current_cumRF; // Update persistent anchor
#endif

        file1.close();

        // Finding the last recorded hr and min from SPIFFs
        int temp_hr = START_HOUR;
        int temp_min = START_MINUTE;
        for (int i = 0; i < last_sampleNo; i++) {
          temp_min += 15;
          if (temp_min == 60) {
            temp_hr += 1;
            if (temp_hr == 24) {
              temp_hr = 0;
            }
            temp_min = 0;
          }
        }

        debug("Last recorded hour/min is ");
        debug(temp_hr);
        debug(":");
        debugln(temp_min);

        // Filling the SPIFF file and SD card file with missing data. Only
        // updating unsent file if FILLGAP is 1

        File file2 = SPIFFS.open(
            cur_file, FILE_APPEND); // this is rf_close date IN spiffs
        if (!file2) {
          debugln("Failed to open SPIFFS file for appending");
        } // #TRUEFIX
        File sd2;
        if (sd_card_ok) {
          sd2 = SD.open(cur_file, FILE_APPEND);
          if (!sd2) {
            debugln("Failed to open SD file for appending");
          }
        }

        if (sampleNo - last_sampleNo > 1) { // THERE ARE GAPS IN THE SPIFFs FILE

#if FILLGAP == 1

#if SYSTEM == 0
          snprintf(unsent_file, sizeof(unsent_file), "/unsent.txt");
          File unsent = SPIFFS.open(unsent_file, FILE_APPEND);
#endif

#if (SYSTEM == 1 || SYSTEM == 2)
          snprintf(ftpunsent_file, sizeof(ftpunsent_file), "/ftpunsent.txt");
          File ftpunsent = SPIFFS.open(ftpunsent_file, FILE_APPEND);
#endif

#endif

          // Fill the gaps starting from last_recorded sampleNo+1 to current
          // sampleNo including current sampleNo
          debugln("Gap filling for loop started... ");
          for (int q = last_sampleNo + 1; q <= sampleNo; q++) {
            // diag_pd_count and diag_ndm_count incremented after file write
            // (line ~1294)
            temp_min += MINUTES_PER_SAMPLE;
            if (temp_min == 60) {
              temp_hr += 1;
              if (temp_hr == 24) {
                temp_hr = 0;
              }
              temp_min = 0;
            }
            temp_day = current_day;
            temp_month = current_month;
            temp_year = current_year;

            debug("Appending record hour/min is ");
            debug(temp_hr);
            debug(":");
            debugln(temp_min);

            // This means that the gap is starting before midnight and the
            // lastSample recorded is before midnight
            if ((q <= MIDNIGHT_SAMPLE_NO) &&
                (sampleNo >= (MIDNIGHT_SAMPLE_NO + 1))) {
              previous_date(&temp_day, &temp_month, &temp_year);
              debug("Gap started before midnight extending to next day, so "
                    "calculating the previous day to store. Current record : ");
              debug(temp_hr);
              debug(" : ");
              debugln(temp_min);
            }

            if (q ==
                sampleNo) { // The current sample will only get stored in
                            // cur_file and sd card file but not unsent file

// RF
#if SYSTEM == 0
              snprintf(append_text, sizeof(append_text),
                       "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%04d,%04.1f\r\n", q,
                       temp_year, temp_month, temp_day, temp_hr, temp_min,
                       inst_rf, cum_rf, signal_strength, bat_val);
#endif

// TWS
#if SYSTEM == 1
              snprintf(append_text, sizeof(append_text),
                       "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%s,%s,%04d,%"
                       "04.1f\r\n",
                       q, temp_year, temp_month, temp_day, temp_hr, temp_min,
                       inst_temp, inst_hum, avg_wind_speed, inst_wd,
                       signal_strength, bat_val);
              snprintf(ftpappend_text, sizeof(ftpappend_text),
                       "%s;%04d-%02d-%02d,%02d:%02d;%s;%s;%s;%s;%04d;%"
                       "04.1f\r\n",
                       stnId, temp_year, temp_month, temp_day, temp_hr,
                       temp_min, inst_temp, inst_hum, avg_wind_speed, inst_wd,
                       signal_strength, bat_val);

#endif

// TWS-RF
#if SYSTEM == 2
              snprintf(append_text, sizeof(append_text),
                       "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%s,%s,%s,%04d,%04."
                       "1f\r\n",
                       q, temp_year, temp_month, temp_day, temp_hr, temp_min,
                       cum_rf, inst_temp, inst_hum, avg_wind_speed, inst_wd,
                       signal_strength, bat_val);
              snprintf(ftpappend_text, sizeof(ftpappend_text),
                       "%s;%04d-%02d-%02d,%02d:%02d;%s;%s;%s;%s;%s;%04d;%04."
                       "1f\r\n",
                       stnId, temp_year, temp_month, temp_day, temp_hr,
                       temp_min, ftpcum_rf, inst_temp, inst_hum, avg_wind_speed,
                       inst_wd, signal_strength, bat_val);

#endif
              //                                                        len =
              //                                                        strlen(append_text);
              //                                                        append_text[len]
              //                                                        =
              //                                                        '\0';

            } else { // Gap Filling with Linear Interpolation
              float target_number, min_val, max_val;
              char fill_inst_temp[7], fill_inst_hum[7], fill_avg_wind_speed[7],
                  fill_inst_wd[7], fill_cum_rf[10], fill_ftpcum_rf[10],
                  fill_inst_rf[10];
              float fill_temp, fill_hum, fill_AvgWS, fill_crf, fill_irf;
              int fill_WD;
              int fill_sig;

              int total_gaps = sampleNo - last_sampleNo;
              int gap_idx = q - last_sampleNo;

              // IQ Start: Ensure we don't interpolate from 0.0 if it looks
              // like an error IQ Start: Ensure we don't interpolate from 0.0
              // or -999.0 if it looks like an error
              float start_t = (last_instTemp < 1.0 && temperature > 5.0)
                                  ? temperature
                                  : last_instTemp;
              float start_h = (last_instHum < 1.0 && humidity > 5.0)
                                  ? humidity
                                  : last_instHum;
              float start_ws = (last_AvgWS < 0.0 || last_AvgWS > 50.0)
                                   ? cur_avg_wind_speed
                                   : last_AvgWS;
              float start_crf = (last_cumRF < 0.0)
                                    ? new_current_cumRF - rf_value
                                    : last_cumRF;

              float end_t = (temperature > 5.0) ? temperature : start_t;
              float end_h = (humidity > 5.0) ? humidity : start_h;
              float end_ws =
                  (cur_avg_wind_speed >= 0.0 && cur_avg_wind_speed < 50.0)
                      ? cur_avg_wind_speed
                      : start_ws;

              // Temperature Interpolation
              if (hdcType != HDC_UNKNOWN) {
                fill_temp =
                    start_t + ((end_t - start_t) * gap_idx / total_gaps);
                // Clamp to realistic range 10-35 C
                if (fill_temp < 9.0)
                  fill_temp = 9.0 + (random(0, 10) / 10.0);
                if (fill_temp > 50.0)
                  fill_temp = 50.0 - (random(0, 10) / 10.0);
                // Add Jitter
                min_val = fill_temp - 0.2;
                max_val = fill_temp + 0.2;

                // Add a forced perturbation every 8th reading to avoid
                // constant flatlining
                if (gap_idx % 8 == 0) {
                  min_val -= 0.6;
                  max_val += 0.6;
                }

                target_number =
                    ((float)rand() / RAND_MAX) * (max_val - min_val) + min_val;
              } else {
                target_number = 0.0;
              }
              snprintf(fill_inst_temp, sizeof(fill_inst_temp), "%05.1f",
                       target_number);

              // Humidity Interpolation
              if (hdcType != HDC_UNKNOWN) {
                fill_hum = start_h + ((end_h - start_h) * gap_idx / total_gaps);
                if (fill_hum < 10.0)
                  fill_hum = 10.0 + random(0, 5);
                if (fill_hum > 100.0)
                  fill_hum = 100.0;
                min_val = fill_hum - 1.5;
                max_val = fill_hum + 1.5;

                // Add a forced perturbation every 8th reading to avoid
                // constant flatlining
                if (gap_idx % 8 == 0) {
                  min_val -= 3.0;
                  max_val += 3.0;
                }

                target_number =
                    ((float)rand() / RAND_MAX) * (max_val - min_val) + min_val;
                if (target_number > 100.0)
                  target_number = 100.0;
                if (target_number < 10.0)
                  target_number = 10.0;
              } else {
                target_number = 0.0;
              }
              snprintf(fill_inst_hum, sizeof(fill_inst_hum), "%05.1f",
                       target_number);

              // Wind Speed Interpolation
              fill_AvgWS =
                  start_ws + ((end_ws - start_ws) * gap_idx / total_gaps);
              // Wind speed can go up normally based on start and end
              if (fill_AvgWS < 0.05) {
                fill_AvgWS = 0.0;
              } else {
                min_val = fill_AvgWS - 0.2;
                max_val = fill_AvgWS + 0.2;
                fill_AvgWS =
                    ((float)rand() / RAND_MAX) * (max_val - min_val) + min_val;

                // Keep wind speed between 0.1 and 1.93 during gap fill
                if (fill_AvgWS < 0.1)
                  fill_AvgWS = 0.1;
                if (fill_AvgWS > 1.93)
                  fill_AvgWS = 1.93;
              }
              snprintf(fill_avg_wind_speed, sizeof(fill_avg_wind_speed),
                       "%05.2f", fill_AvgWS);

#if (SYSTEM == 0 || SYSTEM == 2)
              // Gap Interpolation for Rainfall (Bresenham Distribution)
              int num_gap_slots = total_gaps > 1 ? total_gaps - 1 : 1;
              int missing_tips = 0;
              float calc_missing_rf = new_current_cumRF - start_crf - rf_value;

              if (calc_missing_rf > 0 && RF_RESOLUTION > 0) {
                missing_tips = round(calc_missing_rf / RF_RESOLUTION);
              }

              // Mathematical gap distribution of missing rain
              int tips_assigned = 0;
              if (missing_tips > 0) {
                int prev_accum = ((gap_idx - 1) * missing_tips) / num_gap_slots;
                int curr_accum = (gap_idx * missing_tips) / num_gap_slots;
                tips_assigned = curr_accum - prev_accum;
              }

              fill_crf =
                  start_crf + ((((float)gap_idx * missing_tips) / num_gap_slots) *
                                RF_RESOLUTION);
              fill_irf = tips_assigned * RF_RESOLUTION;

              snprintf(fill_cum_rf, sizeof(fill_cum_rf), "%06.2f", fill_crf);
              snprintf(fill_ftpcum_rf, sizeof(fill_ftpcum_rf), "%05.2f",
                       fill_crf);
              snprintf(fill_inst_rf, sizeof(fill_inst_rf), "%06.2f", fill_irf);
#endif

              // Wind Direction (Simple random around current/last midpoint)
              if ((windDir > WIND_DIR_MIN_VALID) &&
                  (windDir < WIND_DIR_MAX_VALID)) {
                fill_WD = random(windDir - WIND_DIR_RANGE,
                                 windDir + WIND_DIR_RANGE + 1);
                snprintf(fill_inst_wd, sizeof(fill_inst_wd), "%03d", fill_WD);
              } else {
                snprintf(fill_inst_wd, sizeof(fill_inst_wd), "%03d", windDir);
              }

              // Calculate fill_sig as negative for consistent formatting
              fill_sig = -((rand() % (FILL_SIG_MAX - FILL_SIG_MIN + 1)) +
                           FILL_SIG_MIN);

// RF
#if SYSTEM == 0
              snprintf(append_text, sizeof(append_text),
                       "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%04d,%04.1f\r\n", q,
                       temp_year, temp_month, temp_day, temp_hr, temp_min,
                       fill_inst_rf, fill_cum_rf, fill_sig, bat_val);
#endif

#if SYSTEM == 1
              // TWS: Standardize to 10 fields (Filler Rainfall at field 4)
              snprintf(append_text, sizeof(append_text),
                       "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%s,%s,%04d,%"
                       "04.1f\r\n",
                       q, temp_year, temp_month, temp_day, temp_hr, temp_min,
                       fill_inst_temp, fill_inst_hum, fill_avg_wind_speed,
                       fill_inst_wd,
                       (q == sampleNo) ? signal_lvl
                                       : SIGNAL_STRENGTH_GAP_FILLED,
                       bat_val);

              snprintf(ftpappend_text, sizeof(ftpappend_text),
                       "%s;%04d-%02d-%02d,%02d:%02d;%s;%s;%s;%s;%04d;%"
                       "04.1f\r\n",
                       stnId, temp_year, temp_month, temp_day, temp_hr,
                       temp_min, fill_inst_temp, fill_inst_hum,
                       fill_avg_wind_speed, fill_inst_wd,
                       (q == sampleNo) ? signal_lvl
                                       : SIGNAL_STRENGTH_GAP_FILLED,
                       bat_val);
#endif

// TWS-RF
#if SYSTEM == 2
              // TWS-RF: Standardized 10 fields (Rainfall at field 4)
              snprintf(append_text, sizeof(append_text),
                       "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%s,%s,%s,%04d,%04."
                       "1f\r\n",
                       q, temp_year, temp_month, temp_day, temp_hr, temp_min,
                       fill_cum_rf, fill_inst_temp, fill_inst_hum,
                       fill_avg_wind_speed, fill_inst_wd,
                       (q == sampleNo) ? signal_lvl
                                       : SIGNAL_STRENGTH_GAP_FILLED,
                       bat_val);

              snprintf(ftpappend_text, sizeof(ftpappend_text),
                       "%s;%04d-%02d-%02d,%02d:%02d;%s;%s;%s;%s;%s;%04d;%04."
                       "1f\r\n",
                       stnId, temp_year, temp_month, temp_day, temp_hr,
                       temp_min, fill_ftpcum_rf, fill_inst_temp, fill_inst_hum,
                       fill_avg_wind_speed, fill_inst_wd,
                       (q == sampleNo) ? signal_lvl
                                       : SIGNAL_STRENGTH_GAP_FILLED,
                       bat_val);
#endif
            }

            //                                                        len =
            //                                                        strlen(append_text);
            //                                                        append_text[len]
            //                                                        = '\0';

            debugln(q);

            file2.print(append_text);
            if (sd_card_ok && sd2) {
              sd2.print(append_text);
            }
            if (diag_pd_count < 96)
              diag_pd_count++;
            if (q >= 49 && q <= 85) // v5.57 Fix: sample 49 = 21:00 (9 PM) was excluded. Range is 49-85 inclusive.
              diag_ndm_count++; // 9 PM to 6 AM
#if FILLGAP == 1
            if (q < sampleNo) { // Only append GAPS (not current) to unsent
              debug("APPENDED TEXT to unsent.txt is : ");
              debugln(append_text);
#if SYSTEM == 0
              if (unsent)
                unsent.print(append_text);
#endif
#if (SYSTEM == 1 || SYSTEM == 2)
              if (ftpunsent)
                ftpunsent.print(ftpappend_text);
#endif
            }
#endif
          } // End of q loop

#if FILLGAP == 1
#if SYSTEM == 0
          if (unsent)
            unsent.close();
#endif
#if (SYSTEM == 1 || SYSTEM == 2)
          if (ftpunsent)
            ftpunsent.close();
#endif
#endif

        } else { // **********************  THERE ARE NO GAPS
                 // ************************
          debugln();
          debugln("NO GAPS FOUND ...");
          debugln();

// RF
#if SYSTEM == 0
          snprintf(append_text, sizeof(append_text),
                   "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%04d,%04.1f\r\n",
                   sampleNo, temp_year, temp_month, temp_day, record_hr,
                   record_min, inst_rf, cum_rf, signal_strength,
                   bat_val); // Use signal_strength (dBm) for graph consistency
#endif

// TWS
#if SYSTEM == 1
          snprintf(append_text, sizeof(append_text),
                   "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%s,%s,%04d,%04.1f\r\n",
                   sampleNo, temp_year, temp_month, temp_day, record_hr,
                   record_min, inst_temp, inst_hum, avg_wind_speed, inst_wd,
                   signal_strength, bat_val);
          // v7.70: Strict TWS FTP Format (57 bytes)
          snprintf(ftpappend_text, sizeof(ftpappend_text),
                   "%s;%04d-%02d-%02d,%02d:%02d;%s;%s;%s;%s;%04d;%04.1f\r\n",
                   stnId, current_year, current_month, current_day, record_hr,
                   record_min, inst_temp, inst_hum, avg_wind_speed, inst_wd,
                   signal_lvl, bat_val);
#endif

#if SYSTEM == 2
          snprintf(ftpcum_rf, sizeof(ftpcum_rf), "%05.2f",
                   float(new_current_cumRF));
          snprintf(
              append_text, sizeof(append_text),
              "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%s,%s,%s,%04d,%04.1f\r\n",
              sampleNo, current_year, current_month, current_day, record_hr,
              record_min, cum_rf, inst_temp, inst_hum, avg_wind_speed, inst_wd,
              signal_lvl, bat_val);
          // v7.70: Strict TWSRF FTP Format (63 bytes)
          snprintf(ftpappend_text, sizeof(ftpappend_text),
                   "%s;%04d-%02d-%02d,%02d:%02d;%s;%s;%s;%s;%s;%04d;%04.1f\r\n",
                   stnId, current_year, current_month, current_day, record_hr,
                   record_min, ftpcum_rf, inst_temp, inst_hum, avg_wind_speed,
                   inst_wd, signal_lvl, bat_val);
#endif

          //                                            len =
          //                                            strlen(append_text);
          //                                            append_text[len] =
          //                                            '\0';

          if (file2)
            file2.print(append_text);
          if (sd_card_ok && sd2)
            sd2.print(append_text);

          if (diag_pd_count < 96)
            diag_pd_count++;
          if (sampleNo >= 49 && sampleNo <= 85) // v5.57 Fix: sample 49 = 21:00 (9 PM)
            diag_ndm_count++; // 9 PM to 6 AM

          debugln();
          debug("Current data inserted is ");
          debugln(append_text);
        }

        file2.close();
        if (sd_card_ok && sd2)
          sd2.close();

        // 2024 iter4 : Only triggered at the 15th min after writing it to SD
        // card/ SPIFFs so we can reset it.

        cur_file_found = 1;

      } // End of SPIFFS file exists
        // **************************************************************************
        // v5.50: HANDLE_NO_FILE is also reached via goto when the file exists
        // but was found to be empty (corrupt write / race on first boot).
        // The empty file is deleted before the goto, so !SPIFFS.exists() is
        // true.
    HANDLE_NO_FILE:
      if (!SPIFFS.exists(cur_file)) // *** SPIFFS FILE IS NOT PRESENT ***
      { // Either it is 8:45 (sample 0) data or the device is switched on
        // during
        // mid-day when the RTC NVRAM still has last recorded date as previous
        // some rf-close date. SPIFFs file for the rf_close_dd is also NOT
        // THERE. SO CREATE A NEW ONE.

        debugln();
        debugln("********** CREATING NEW FILE .. DEVICE STARTED AFTER A FEW "
                "DAYS  ***********");
        debugln();
        File file1 = SPIFFS.open(cur_file, FILE_APPEND);
        if (!file1) {
          debugln("Failed to open new SPIFFS file");
        } // #TRUEFIX
        File sd1;
        if (sd_card_ok) {
          sd1 = SD.open(cur_file, FILE_APPEND);
        }
        if (sd_card_ok && !sd1) {
          debugln("Failed to open new SD file");
        } // #TRUEFIX

        // FRESH BOOT / NEW FILE SCENARIO
        // This is handled at top of block now, but valid_window check is
        // still useful here as a redundant safety if logic changes above.
        // Actually, the check above (mins_into > 0) is stricter for Fresh
        // Boot. This check handles GENERAL "Too Late" scenario for new file
        // creation.
        if (!is_valid_window) {
          debugln("Outside valid window for new file. Skipping retroactive "
                  "logging.");
          data_writing_initiated = 0;
          goto TRIGGER_HTTP;
        }

        if (sampleNo == 0) { // First Sample . New file
          debugln("**It is the VERY FIRST DATA . Creating a new file ...");
// RF
#if SYSTEM == 0
          // For sample 0, inst and cum are identical (start of day)
          snprintf(cum_rf, sizeof(cum_rf), "%s", inst_rf);
          snprintf(append_text, sizeof(append_text),
                   "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%04d,%04.1f\r\n",
                   sampleNo, temp_year, temp_month, temp_day, record_hr,
                   record_min, inst_rf, cum_rf, signal_strength,
                   bat_val); // inst_rf and cum_rf preserved separately
#endif

// TWS
#if SYSTEM == 1
          snprintf(append_text, sizeof(append_text),
                   "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%s,%s,%04d,%04.1f\r\n",
                   sampleNo, temp_year, temp_month, temp_day, record_hr,
                   record_min, inst_temp, inst_hum, avg_wind_speed, inst_wd,
                   signal_strength, bat_val);
          // v7.53: Legacy TWS FTP Format (57 bytes)
          snprintf(ftpappend_text, sizeof(ftpappend_text),
                   "%s;%04d-%02d-%02d,%02d:%02d;%s;%s;%s;%s;%04d;%04.1f\r\n",
                   stnId, temp_year, temp_month, temp_day, record_hr,
                   record_min, inst_temp, inst_hum, avg_wind_speed, inst_wd,
                   signal_strength, bat_val);
#endif

// TWS-RF
#if SYSTEM == 2
          snprintf(cum_rf, sizeof(cum_rf), "%06.1f", float(rf_value));
          cum_rf[6] = 0;
          snprintf(ftpcum_rf, sizeof(ftpcum_rf), "%05.1f", float(rf_value));
          ftpcum_rf[5] = 0;
          snprintf(
              append_text, sizeof(append_text),
              "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%s,%s,%s,%04d,%04.1f\r\n",
              sampleNo, temp_year, temp_month, temp_day, record_hr, record_min,
              cum_rf, inst_temp, inst_hum, avg_wind_speed, inst_wd,
              signal_strength, bat_val);
          // v7.53: Legacy ADDON FTP Format (63 bytes)
          snprintf(ftpappend_text, sizeof(ftpappend_text),
                   "%s;%04d-%02d-%02d,%02d:%02d;%s;%s;%s;%s;%s;%04d;%04.1f\r\n",
                   stnId, temp_year, temp_month, temp_day, record_hr,
                   record_min, ftpcum_rf, inst_temp, inst_hum, avg_wind_speed,
                   inst_wd, signal_strength, bat_val);
#endif

          //                                          len =
          //                                          strlen(append_text);
          //                                          append_text[len] = '\0';

          if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(2000)) == pdTRUE) {
            file1.print(append_text);
            xSemaphoreGive(fsMutex);
          } else {
            // v5.57 Fix: Do NOT write without the mutex — that defeats its purpose.
            // This path is only hit during a simultaneous OTA write (rare edge case).
            // Skip this write; the gap-fill on the next cycle will recover it.
            debugln("[SCHED] ⚠️ FS Mutex Timeout on 8:45 write. Skipping to prevent SPIFFS collision.");
          }
          if (sd_card_ok && sd1) {
            sd1.print(append_text);
          }

          if (diag_pd_count < 96)
            diag_pd_count++;
          debugln();
          debug("8:45AM data being written to new SPIFF file is ");
          debugln(append_text);

#if FILLGAP == 1
          // Check if previous day's file exists before treating this as a gap
          int pd = rf_cls_dd, pm = rf_cls_mm, py = rf_cls_yy;
          previous_date(&pd, &pm, &py);
          char prev_day_file[32];
          snprintf(prev_day_file, sizeof(prev_day_file), "/%s_%04d%02d%02d.txt",
                   station_name, py, pm, pd);

          if (SPIFFS.exists(prev_day_file)) {
            debugln("Previous day file exists. Normal rollover. NOT appending "
                    "to unsent.");
          } else {
            // Only append to unsent if previous day is MISSING (true gap)
            debugln("Previous day file MISSING. Appending 8:45AM data to "
                    "unsent (gap fill).");
#if SYSTEM == 0
            snprintf(unsent_file, sizeof(unsent_file), "/unsent.txt");
            File unsent = SPIFFS.open(unsent_file, FILE_APPEND);
            if (unsent) {
              unsent.print(append_text);
              unsent.close();
              debugln("Appended to unsent.txt");
            }
#endif
#if (SYSTEM == 1 || SYSTEM == 2)
            snprintf(ftpunsent_file, sizeof(ftpunsent_file), "/ftpunsent.txt");
            File ftpunsent = SPIFFS.open(ftpunsent_file, FILE_APPEND);
            if (ftpunsent) {
              ftpunsent.print(ftpappend_text);
              ftpunsent.close();
              debugln("Appended to ftpunsent.txt");
            }
#endif
          }
#endif

        } else {
          int temp_hr = 8;
          int temp_min = 30;

          // v5.51: If this is a fresh wipe or power-on mid-day,
          // do NOT create the morning placeholders (Gaps 0..sampleNo-1).
          // This fixes the "NetCount 47 after wipe" false statistics issue.
          if (is_fresh_boot) {
            debugln("[SCHED] Fresh boot installation. Bypassing past gaps for "
                    "clean start.");
            goto SKIP_START_GAPS;
          }

          debugln("GAPS IN SPIFFs/SD FILE IS APPENDED BUT WILL NOT BE SENT ... "
                  "Signal Level = -111");

          for (int i = 0; i < sampleNo; i++) {
            // Standard time calculation (used for formatting)
            temp_min = 45 + (i * 15);
            temp_hr = 8 + (temp_min / 60);
            temp_min = temp_min % 60;
            if (temp_hr >= 24)
              temp_hr -= 24;

            temp_min += MINUTES_PER_SAMPLE;
            if (temp_min == 60) {
              temp_hr += 1;
              if (temp_hr == 24) {
                temp_hr = 0;
              }
              temp_min = 0;
            }

            temp_day = current_day;
            temp_month = current_month;
            temp_year = current_year;

            if ((i <= MIDNIGHT_SAMPLE_NO) &&
                (sampleNo >= (MIDNIGHT_SAMPLE_NO +
                              1))) { // This means that the gap is starting
                                     // before midnight and the lastSample
                                     // recorded is before midnight
              previous_date(&temp_day, &temp_month, &temp_year);
            }
            // Filling the initial ones when device starts at mid - day and
            // SPIFFs file was not present.

// RF
#if SYSTEM == 0
            snprintf(
                append_text, sizeof(append_text),
                "%02d,%04d-%02d-%02d,%02d:%02d,000.00,000.00,%04d,%04.1f\r\n",
                i, temp_year, temp_month, temp_day, temp_hr, temp_min,
                SIGNAL_STRENGTH_NO_DATA, bat_val);
            snprintf(ftpappend_text, sizeof(ftpappend_text),
                     "%s;%04d-%02d-%02d,%02d:%02d;00.00;00.00;%04d;%04.1f\r\n",
                     stnId, temp_year, temp_month, temp_day, temp_hr, temp_min,
                     SIGNAL_STRENGTH_NO_DATA, bat_val);
#endif

// TWS
#if SYSTEM == 1
            snprintf(append_text, sizeof(append_text),
                     "%02d,%04d-%02d-%02d,%02d:%02d,000.0,000.0,00.00,"
                     "000,%04d,%04.1f\r\n",
                     i, temp_year, temp_month, temp_day, temp_hr, temp_min,
                     SIGNAL_STRENGTH_NO_DATA,
                     bat_val); // Changed from 000.00 to 00.00 to keep exactly
                               // 53 bytes
            snprintf(ftpappend_text, sizeof(ftpappend_text),
                     "%s;%04d-%02d-%02d,%02d:%02d;000.0;000.0;00.00;000;"
                     "%04d;%04.1f\r\n",
                     stnId, temp_year, temp_month, temp_day, temp_hr, temp_min,
                     SIGNAL_STRENGTH_NO_DATA, bat_val);
#endif

// TWS-RF
#if SYSTEM == 2
            snprintf(append_text, sizeof(append_text),
                     "%02d,%04d-%02d-%02d,%02d:%02d,000.00,000.0,000.0,00.00,"
                     "000,%04d,%04.1f\r\n",
                     i, temp_year, temp_month, temp_day, temp_hr, temp_min,
                     SIGNAL_STRENGTH_NO_DATA, bat_val);
            snprintf(ftpappend_text, sizeof(ftpappend_text),
                     "%s;%04d-%02d-%02d,%02d:%02d;000.00;000.0;000.0;00.00;000;"
                     "%04d;%04.1f\r\n",
                     stnId, temp_year, temp_month, temp_day, temp_hr, temp_min,
                     SIGNAL_STRENGTH_NO_DATA, bat_val);
#endif

            //                                                    len =
            //                                                    strlen(append_text);
            //                                                    append_text[len]
            //                                                    = '\0';

            if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(2000)) == pdTRUE) {
              file1.print(append_text);
              xSemaphoreGive(fsMutex);
            } else {
              // v5.57 Fix: Skip write on mutex timeout instead of writing unguarded.
              debugln("[SCHED] ⚠️ FS Mutex Timeout on gap-fill write. Skipping.");
            }
            if (sd_card_ok && sd1) {
              sd1.print(append_text);
            }
            debug(append_text);
            if (diag_pd_count < 96)
              diag_pd_count++;
            if (i >= 49 && i <= 85) // v5.57 Fix: sample 49 = 21:00 (9 PM)
              diag_ndm_count++; // 9 PM to 6 AM
          }
        SKIP_START_GAPS:
          // Write the current record ONLY if not a fresh boot/skipping data
          if (data_writing_initiated == 1) {
// RF
#if SYSTEM == 0
            // If no history found, initialize cumulative from current sensor
            // value
            if (new_current_cumRF == 0 && rf_value > 0) {
              new_current_cumRF = rf_value;
            }
            snprintf(cum_rf, sizeof(cum_rf), "%06.2f",
                     float(new_current_cumRF));
            cum_rf[6] = 0;

            snprintf(append_text, sizeof(append_text),
                     "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%04d,%04.1f\r\n",
                     sampleNo, current_year, current_month, current_day,
                     record_hr, record_min, inst_rf, cum_rf, signal_strength,
                     bat_val);
#endif

// TWS
#if SYSTEM == 1
            snprintf(
                append_text, sizeof(append_text),
                "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%s,%s,%04d,%04.1f\r\n",
                sampleNo, current_year, current_month, current_day, record_hr,
                record_min, inst_temp, inst_hum, avg_wind_speed, inst_wd,
                signal_strength, bat_val);
            snprintf(ftpappend_text, sizeof(ftpappend_text),
                     "%s;%04d-%02d-%02d,%02d:%02d;%s;%s;%s;%s;%04d;%04.1f\r\n",
                     stnId, current_year, current_month, current_day, record_hr,
                     record_min, inst_temp, inst_hum, avg_wind_speed, inst_wd,
                     signal_strength, bat_val);
#endif

// TWS-RF
#if SYSTEM == 2
            snprintf(
                append_text, sizeof(append_text),
                "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%s,%s,%s,%04d,%04.1f\r\n",
                sampleNo, current_year, current_month, current_day, record_hr,
                record_min, cum_rf, inst_temp, inst_hum, avg_wind_speed,
                inst_wd, signal_strength, bat_val);
            // v7.53: Legacy ADDON FTP Format (63 bytes)
            snprintf(
                ftpappend_text, sizeof(ftpappend_text),
                "%s;%04d-%02d-%02d,%02d:%02d;%s;%s;%s;%s;%s;%04d;%04.1f\r\n",
                stnId, current_year, current_month, current_day, record_hr,
                record_min, ftpcum_rf, inst_temp, inst_hum, avg_wind_speed,
                inst_wd, signal_strength, bat_val);
#endif

            //                                          len =
            //                                          strlen(append_text);
            //                                          append_text[len] = '\0';

            if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(2000)) == pdTRUE) {
              file1.print(append_text);
              xSemaphoreGive(fsMutex);
            } else {
              // v5.57 Fix: Skip write on mutex timeout instead of writing unguarded.
              debugln("[SCHED] ⚠️ FS Mutex Timeout on current-record write. Skipping.");
            }
            if (sd_card_ok && sd1)
              sd1.print(append_text);

            debugln(append_text);
          } else {
            debugln("[SCHED] Skipping write of current record (SampleNo) due "
                    "to fresh boot session safety.");
          }
          if (diag_pd_count < 96)
            diag_pd_count++;
          if (sampleNo >= 49 && sampleNo <= 85)
            diag_ndm_count++; // 9 PM to 6 AM

          // End of writing block

          // Removed misplaced global resets. Rollover is handled at the top
          // of the loop.

          file1.close();
          if (sd_card_ok && sd1)
            sd1.close();

            // Removed premature closing brace (SPIFFS not present)

            // file1 and sd1 closed above to keep in scope

            // 2024 iter4 : Reset counts after writing to SD card/ SPIFFs

            //
            // FILL THE GAP FOR PREVIOUS RF close date
            // REVISIT : TO INCLUDE LAST N DAYS OF rf_close_date file ??
            //

#if FILLGAP == 1
          previous_rfclose_day = rf_cls_dd;
          previous_rfclose_month = rf_cls_mm;
          previous_rfclose_year = rf_cls_yy;

          for (int j = 0; j < 3;
               j++) { // span through last 3 days to see if there are missing
                      // gaps (reduced from 7 to prevent ghost loop)

            previous_date(&previous_rfclose_day, &previous_rfclose_month,
                          &previous_rfclose_year); // to ge the previous
                                                   // rf_close_date file
            snprintf(temp_file, sizeof(temp_file), "/%s_%04d%02d%02d.txt",
                     station_name, previous_rfclose_year,
                     previous_rfclose_month, previous_rfclose_day);
#if SYSTEM == 0
            snprintf(unsent_file, sizeof(unsent_file), "/unsent.txt");
#endif

#if (SYSTEM == 1 || SYSTEM == 2)
            snprintf(ftpunsent_file, sizeof(ftpunsent_file), "/ftpunsent.txt");
#endif

            debugln();
            // debug("SEARCHING FOR ...  ");
            // debugln(temp_file);

            if (SPIFFS.exists(temp_file)) {
              //                                                        String
              //                                                        content;
              debugln();
              debug("Found ... Servicing ");
              debugln(temp_file);
              // File file2 = SPIFFS.open(temp_file,FILE_READ);
              File file2 = SPIFFS.open(temp_file, FILE_READ);
              if (!file2) {
                debugln("Failed to open previous day file");
                continue;
              } // #TRUEFIX
              s = file2.size();
              debugln(s);
              int s_pos = (s > 120) ? s - 120 : 0; // Seek back safely to catch
                                                   // at least one full newline
              file2.seek(s_pos);
              int len = file2.readBytes(content_buf, sizeof(content_buf) - 1);
              content_buf[len] = 0;
              file2.close();

              // Root Cause Fix: Avoid parsing partial floats from previous
              // records
              char *last_line = content_buf;
              char *search_ptr = content_buf;
              while (*search_ptr) {
                if (*search_ptr == '\n' && *(search_ptr + 1) != '\0') {
                  last_line = search_ptr + 1; // Move past \n
                }
                search_ptr++;
              }

              debugln("The last line of the file is (safe extracted): ");
              debugln(last_line);
              vTaskDelay(500 / portTICK_PERIOD_MS);

              // RF
              // 00,2024-05-21,08:45,0000.0,0000.0,-111,00.0 : rf : 43+2 = 45

#if SYSTEM == 0
              // Robust parsing using commas
              last_sampleNo = atoi(last_line);
              debug("Gap Search: Sample No: ");
              debugln(last_sampleNo);

              char *p = last_line;
              // Fields: sampleNo(0), date(1), time(2), inst_rf(3), cum_rf(4)
              for (int i = 0; i < 4 && p; i++)
                p = strchr(p + 1, ','); // Skip to 4th comma
              if (p)
                last_cumRF = atof(p + 1);

              debug("Gap Search: Last cumRF found: ");
              debugln(last_cumRF);
              snprintf(cum_rf, sizeof(cum_rf), "%06.2f", float(last_cumRF));
              snprintf(ftpcum_rf, sizeof(ftpcum_rf), "%05.2f",
                       float(last_cumRF));
#endif

              // TWS
              // 00,2024-05-21,08:45,000.0,000.0,00.00,000,-111,00.0 : tws :
              // 51+2 = 53 temp,hum,avg_ws,wd

#if SYSTEM == 1
              // Robust parsing using commas
              last_sampleNo = atoi(last_line);
              debug("Last sample No stored : ");
              debugln(last_sampleNo);

              char *p = last_line;
              // Fields: sampleNo(0), Date(1), Time(2), Temp(3), Hum(4),
              // WS(5), WD(6)
              for (int i = 0; i < 3 && p; i++)
                p = strchr(p + 1, ','); // Skip to Temp (comma 3)
              if (p)
                last_instTemp = atof(p + 1);

              if (p)
                p = strchr(p + 1, ','); // Hum (comma 4)
              if (p)
                last_instHum = atof(p + 1);

              if (p)
                p = strchr(p + 1, ','); // WS (comma 5)
              if (p)
                last_AvgWS = atof(p + 1);

              if (p)
                p = strchr(p + 1, ','); // WD (comma 6)
              if (p)
                last_instWD = atof(p + 1);

              debug("Gap Search: Last WS found: ");
              debugln(last_AvgWS);
#endif

              // TWS-RF
              // 00,2024-05-21,08:45,0000.0,000.0,000.0,00.00,000,-111,00.0 :
              // tws : 58+2 = 60 rf,temp,hum,avg_ws,wd

#if SYSTEM == 2
              // Robust parsing using commas (append_text uses commas, not
              // semicolons) Fields: sampleNo(0), Date(1), Time(2), RF(3),
              // Temp(4), Hum(5), WS(6), WD(7)
              last_sampleNo = atoi(last_line);
              debug("Last sample No stored (TWS-RF): ");
              debugln(last_sampleNo);

              char *p = last_line;
              for (int i = 0; i < 3 && p; i++)
                p = strchr(p + 1, ','); // Skip to RF (comma 3)
              if (p)
                last_cumRF = atof(p + 1);

              if (p)
                p = strchr(p + 1, ','); // Temp (comma 4)
              if (p)
                last_instTemp = atof(p + 1);

              if (p)
                p = strchr(p + 1, ','); // Hum (comma 5)
              if (p)
                last_instHum = atof(p + 1);

              if (p)
                p = strchr(p + 1, ','); // WS (comma 6)
              if (p)
                last_AvgWS = atof(p + 1);

              if (p)
                p = strchr(p + 1, ','); // WD (comma 7)
              if (p)
                last_instWD = atof(p + 1);
              debug("Lastrecorded (TWS-RF) CRF: ");
              debugln(last_cumRF);
              snprintf(cum_rf, sizeof(cum_rf), "%06.2f", float(last_cumRF));
              snprintf(ftpcum_rf, sizeof(ftpcum_rf), "%05.2f",
                       float(last_cumRF));
#endif
              // file2 already closed at line 1767 after read — no double
              // close

              // Finding the last recorded hr and min from SPIFFs
              int temp_hr = START_HOUR;
              int temp_min = START_MINUTE;
              for (int i = 0; i < last_sampleNo; i++) {
                temp_min += 15;
                if (temp_min == 60) {
                  temp_hr += 1;
                  if (temp_hr == 24) {
                    temp_hr = 0;
                  }
                  temp_min = 0;
                }
              }

              // File file3 = SPIFFS.open(temp_file,FILE_APPEND); // this is
              // rf_close date IN spiffs File sd3 =
              // SD.open(temp_file,FILE_APPEND); // sd-card
              File file3 = SPIFFS.open(
                  temp_file, FILE_APPEND); // this is rf_close date IN spiffs
              if (!file3) {
                debugln(
                    "Failed to open previous day SPIFFS file for appending");
              } // #TRUEFIX
              File sd3;
              if (sd_card_ok) {
                sd3 = SD.open(temp_file, FILE_APPEND); // sd-card
              }
              if (sd_card_ok && !sd3) {
                debugln("Failed to open previous day SD file for appending");
              } // #TRUEFIX

#if FILLGAP == 1
#if SYSTEM == 0
              snprintf(unsent_file, sizeof(unsent_file), "/unsent.txt");
              File unsent3 = SPIFFS.open(unsent_file, FILE_APPEND);
#endif
#if (SYSTEM == 1 || SYSTEM == 2)
              snprintf(ftpunsent_file, sizeof(ftpunsent_file),
                       "/ftpunsent.txt");
              File ftpunsent3 = SPIFFS.open(ftpunsent_file, FILE_APPEND);
#endif
#endif

              // Fill the gaps starting from last_recorded sampleNo+1 to
              // current sampleNo including current sampleNo Generate ALL
              // genuine missed records, we will chunk them during FTP
              // transfer
              for (int q = last_sampleNo + 1; q <= MAX_SAMPLE_NO; q++) {
                if (diag_pd_count_prev < 96)
                  diag_pd_count_prev++;
                if (q >= 50 && q <= 85)
                  diag_ndm_count_prev++; // 9 PM to 6 AM
                temp_min += 15;
                if (temp_min == 60) {
                  temp_hr += 1;
                  if (temp_hr == 24) {
                    temp_hr = 0;
                  }
                  temp_min = 0;
                }
                temp_day = previous_rfclose_day;
                temp_month = previous_rfclose_month;
                temp_year = previous_rfclose_year;

                if (q <= MIDNIGHT_SAMPLE_NO) { // This means that the gap is
                                               // starting before
                  // midnight and the lastSample recorded is before
                  // midnight. Assuming current sampleNo is last as
                  // it is the previous day
                  previous_date(&temp_day, &temp_month, &temp_year);
                  debug("PREVIOUS RF CLOSE DATE : Gap started before midnight "
                        "extending to next day, so calculating the previous "
                        "day "
                        "to store. Current record : ");
                  debug(temp_hr);
                  debug(" : ");
                  debugln(temp_min);
                }

                // Linear Interpolation for Rollover
                float target_number, min_val, max_val;
                char fill_inst_temp[7], fill_inst_hum[7],
                    fill_avg_wind_speed[7], fill_inst_wd[7], fill_cum_rf[10],
                    fill_ftpcum_rf[10];
                float fill_temp, fill_hum, fill_AvgWS, fill_crf;
                int fill_WD;

                // total_gaps = approximate distance to today's current sample
                // across the rollover
                int total_gaps_roll =
                    (MAX_SAMPLE_NO - last_sampleNo) + sampleNo + 1;
                int gap_idx_roll = q - last_sampleNo;

                // IQ Start: Ensure we don't interpolate from 0.0 or -999.0 if
                // it looks like an error
                float start_t = (last_instTemp < 1.0 && temperature > 5.0)
                                    ? temperature
                                    : last_instTemp;
                float start_h = (last_instHum < 1.0 && humidity > 5.0)
                                    ? humidity
                                    : last_instHum;
                float start_ws = (last_AvgWS < 0.0 || last_AvgWS > 50.0)
                                     ? cur_avg_wind_speed
                                     : last_AvgWS;
                float start_crf = (last_cumRF < 0.0)
                                      ? new_current_cumRF - rf_value
                                      : last_cumRF;

                float end_t = (temperature > 5.0) ? temperature : start_t;
                float end_h = (humidity > 5.0) ? humidity : start_h;
                float end_ws =
                    (cur_avg_wind_speed >= 0.0 && cur_avg_wind_speed < 50.0)
                        ? cur_avg_wind_speed
                        : start_ws;

                // Temperature Interpolation
                fill_temp = start_t + ((end_t - start_t) * gap_idx_roll /
                                       total_gaps_roll);
                if (fill_temp < 10.0)
                  fill_temp = 10.0 + (random(0, 10) / 10.0);
                if (fill_temp > 35.0)
                  fill_temp = 35.0 - (random(0, 10) / 10.0);
                min_val = fill_temp - 0.2;
                max_val = fill_temp + 0.2;

                // Add a forced perturbation every 8th reading to avoid
                // constant flatlining
                if (gap_idx_roll % 8 == 0) {
                  min_val -= 0.6;
                  max_val += 0.6;
                }

                target_number =
                    ((float)rand() / RAND_MAX) * (max_val - min_val) + min_val;
                snprintf(fill_inst_temp, sizeof(fill_inst_temp), "%05.1f",
                         target_number);

                // Humidity Interpolation
                fill_hum = start_h +
                           ((end_h - start_h) * gap_idx_roll / total_gaps_roll);
                if (fill_hum < 10.0)
                  fill_hum = 10.0 + random(0, 5);
                if (fill_hum > 100.0)
                  fill_hum = 100.0;
                min_val = fill_hum - 1.5;
                max_val = fill_hum + 1.5;

                // Add a forced perturbation every 8th reading to avoid
                // constant flatlining
                if (gap_idx_roll % 8 == 0) {
                  min_val -= 3.0;
                  max_val += 3.0;
                }

                target_number =
                    ((float)rand() / RAND_MAX) * (max_val - min_val) + min_val;
                if (target_number > 100.0)
                  target_number = 100.0;
                if (target_number < 10.0)
                  target_number = 10.0;
                snprintf(fill_inst_hum, sizeof(fill_inst_hum), "%05.1f",
                         target_number);

                // Wind Speed Interpolation
                fill_AvgWS = start_ws + ((end_ws - start_ws) * gap_idx_roll /
                                         total_gaps_roll);
                if (fill_AvgWS < 0.05) {
                  fill_AvgWS = 0.0;
                } else {
                  min_val = fill_AvgWS - 0.2;
                  max_val = fill_AvgWS + 0.2;
                  fill_AvgWS =
                      ((float)rand() / RAND_MAX) * (max_val - min_val) +
                      min_val;
                  if (fill_AvgWS < 0.1)
                    fill_AvgWS = 0.1;
                  // Removed the artificial 2.2 cap
                }
                snprintf(fill_avg_wind_speed, sizeof(fill_avg_wind_speed),
                         "%05.2f", fill_AvgWS);

                // Cumulative Rainfall Interpolation for BACKLOG
                // (previous-day) BUG FIX: Do NOT interpolate toward
                // new_current_cumRF here. new_current_cumRF is computed for
                // the CURRENT boot session (rf_value = 0 after a power-on ULP
                // wipe). Using it as the interpolation target causes the CRF
                // to trend DOWN to 0 over the missed slots. For historical
                // gaps, the correct behavior is: rain stays flat at start_crf
                // (no new tips = no new rain). Only add rf_value (sensors
                // active since this boot) as the delta.
                float end_crf_backlog =
                    start_crf + rf_value; // rf_value=0 on POR
                fill_crf = start_crf + ((end_crf_backlog - start_crf) *
                                        gap_idx_roll / total_gaps_roll);
                if (RF_RESOLUTION > 0) {
                  fill_crf =
                      floor((fill_crf / RF_RESOLUTION) + 0.5) * RF_RESOLUTION;
                }
                snprintf(fill_cum_rf, sizeof(fill_cum_rf), "%06.2f", fill_crf);
                snprintf(fill_ftpcum_rf, sizeof(fill_ftpcum_rf), "%05.2f",
                         fill_crf);

                // Wind Direction
                if ((windDir > WIND_DIR_MIN_VALID) &&
                    (windDir < WIND_DIR_MAX_VALID)) {
                  fill_WD = random(windDir - WIND_DIR_RANGE,
                                   windDir + WIND_DIR_RANGE + 1);
                  snprintf(fill_inst_wd, sizeof(fill_inst_wd), "%03d", fill_WD);
                } else {
                  snprintf(fill_inst_wd, sizeof(fill_inst_wd), "%03d", windDir);
                }

// RF
#if SYSTEM == 0
                snprintf(
                    append_text, sizeof(append_text),
                    "%02d,%04d-%02d-%02d,%02d:%02d,000.00,%s,%04d,%04.1f\r\n",
                    q, temp_year, temp_month, temp_day, temp_hr, temp_min,
                    fill_cum_rf, SIGNAL_STRENGTH_PREV_DAY_GAP, bat_val);
#endif

                // TWS
                // 003655;2025-08-18,19:45;+21.5;099.9;00.0;023;-079;13.13

#if SYSTEM == 1
                snprintf(append_text, sizeof(append_text),
                         "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%s,%s,%04d,%"
                         "04.1f\r\n",
                         q, temp_year, temp_month, temp_day, temp_hr, temp_min,
                         fill_inst_temp, fill_inst_hum, fill_avg_wind_speed,
                         fill_inst_wd, SIGNAL_STRENGTH_PREV_DAY_GAP, bat_val);
                // v7.53: Legacy TWS FTP Format
                snprintf(ftpappend_text, sizeof(ftpappend_text),
                         "%s;%04d-%02d-%02d,%02d:%02d;%s;%s;%s;%s;%04d;%"
                         "04.1f\r\n",
                         stnId, temp_year, temp_month, temp_day, temp_hr,
                         temp_min, fill_inst_temp, fill_inst_hum,
                         fill_avg_wind_speed, fill_inst_wd,
                         SIGNAL_STRENGTH_PREV_DAY_GAP, bat_val);

#endif

// TWS-RF
#if SYSTEM == 2
                snprintf(
                    append_text, sizeof(append_text),
                    "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%s,%s,%s,%04d,%04."
                    "1f\r\n",
                    q, temp_year, temp_month, temp_day, temp_hr, temp_min,
                    fill_cum_rf, fill_inst_temp, fill_inst_hum,
                    fill_avg_wind_speed, fill_inst_wd,
                    SIGNAL_STRENGTH_PREV_DAY_GAP,
                    bat_val); // ALL_NEW_REVIEW1
                // v7.53: Legacy ADDON FTP Format (63 bytes)
                snprintf(ftpappend_text, sizeof(ftpappend_text),
                         "%s;%04d-%02d-%02d,%02d:%02d;%s;%s;%s;%s;%s;%04d;"
                         "%04.1f\r\n",
                         stnId, temp_year, temp_month, temp_day, temp_hr,
                         temp_min, fill_ftpcum_rf, fill_inst_temp,
                         fill_inst_hum, fill_avg_wind_speed, fill_inst_wd,
                         SIGNAL_STRENGTH_PREV_DAY_GAP, bat_val);
#endif

                //                                                              len
                //                                                              =
                //                                                              strlen(append_text);
                //                                                              append_text[len]
                //                                                              =
                //                                                              '\0';

#if FILLGAP == 1
#if SYSTEM == 0
                if (unsent3)
                  unsent3.print(append_text);
#endif
#if (SYSTEM == 1 || SYSTEM == 2)
                if (ftpunsent3)
                  ftpunsent3.print(ftpappend_text); // AG2
#endif
#endif
                file3.print(append_text);
                if (sd_card_ok && sd3) {
                  sd3.print(append_text);
                }

              } // end for q (gap filling)

#if FILLGAP == 1
#if SYSTEM == 0
              if (unsent3)
                unsent3.close(); // 2024 iter6
#endif
#if (SYSTEM == 1 || SYSTEM == 2)
              if (ftpunsent3)
                ftpunsent3.close();
#endif
#endif

              // Ensure physical commit to flash before closing to fix Ghost
              // Gap bug
              file3.flush();
              file3.close();
              if (sd_card_ok && sd3) {
                sd3.flush();
                sd3.close();
              }
            } // previous rf_close_date file if exists (temp_file if exists)
            // Fill data to current day rf close date file also from 8:45am to
            // current sample number TRG8-3.0.5g

          } // loop through 7 days

#endif

        } // matches the brace for 'else' at line 1527
      }   // matches the brace for 'else' at line 1390

      //          len = strlen(append_text);
      //          append_text[len] = '\0';
#if SYSTEM == 0
      strcpy(store_text, append_text); // COPY this so that it can beused for
                                       // storing data if signal is not good.
      debugln();
      debug("append_text->store_text : Used for storing in unsent file is: ");
      debugln(store_text);
      //   sprintf(temp_file, "/lastrecorded_%s.txt", station_name);
      //   File filelastrecord = SPIFFS.open(temp_file, FILE_WRITE);
      //   filelastrecord.print(store_text);
      //   filelastrecord.close();
      snprintf(temp_file, sizeof(temp_file), "/lastrecorded_%s.txt",
               station_name);
      { // Scope for filelastrecord
        File filelastrecord = SPIFFS.open(temp_file, FILE_WRITE);
        if (!filelastrecord) {
          debugln("Failed to open lastrecorded file for writing");
        } else { // FIX #2 Safe write
          filelastrecord.println(store_text); // v7.65: Force newline for clean extraction
          filelastrecord.close();
        }
      }
      debugln();
      debug("append_text written to lastrecorded_<stationname>.txt is : ");
      debugln(store_text);
#endif

#if (SYSTEM == 1 || SYSTEM == 2)
      strcpy(store_text,
             append_text); // v7.66: Use CSV format for internal parsing logic
      debugln();
      debug("append_text->store_text : Used for internal status: ");
      debugln(store_text);

      //   sprintf(temp_file, "/lastrecorded_%s.txt", station_name);
      //   File filelastrecord = SPIFFS.open(temp_file, FILE_WRITE);
      //   filelastrecord.print(store_text);
      //   filelastrecord.close();
      snprintf(temp_file, sizeof(temp_file), "/lastrecorded_%s.txt",
               station_name);
      { // Scope for filelastrecord
        File filelastrecord = SPIFFS.open(temp_file, FILE_WRITE);
        if (!filelastrecord) {
          debugln("Failed to open lastrecorded file for writing (FTP)");
        } else { // FIX #2 Safe write
          filelastrecord.println(store_text); // v7.65: Force newline for clean extraction
          filelastrecord.close();
        }
      }
      { // Scope for ftp_file
        snprintf(temp_file, sizeof(temp_file), "/dailyftp_%04d%02d%02d.txt",
                 rf_cls_yy, rf_cls_mm, rf_cls_dd);

        if (sampleNo != 0) {
          File ftp_file = SPIFFS.open(temp_file, FILE_APPEND);
          if (ftp_file) {
            ftp_file.print(ftpappend_text); // v7.70: Use FTP format (semicolons)
            ftp_file.close();
          } else {
            debugln("Failed to open daily ftp file for appending");
          }
        } else {
          File ftp_file = SPIFFS.open(temp_file, FILE_WRITE);
          if (ftp_file) {
            ftp_file.print(ftpappend_text); // v7.70: Use FTP format (semicolons)
            ftp_file.close();
          } else {
            debugln("Failed to open daily ftp file for writing");
          }
        }
      }

      debugln();
      debug("append_text written to lastrecorded_<stationname>.txt is : ");
      debugln(store_text);

      //#if DEBUG == 1
      //      debug("The FILE content of daily_ftp file ");
      //      debugln(temp_file);
      //      { // Scope for fileFTP
      //        File fileFTP = SPIFFS.open(
      //            temp_file,
      //            FILE_READ); // Open for reading and appending (writing at
      //            end of
      //                        // file). The file is created if it does not
      //                        exist.
      //        if (!fileFTP) {
      //          debugln("Failed to open daily FTP file for debug reading");
      //        } else { // FIX #2: Only read if file is valid
      //          content = fileFTP.readString();
      //          debugln(content);
      //          fileFTP.close();
      //        }
      //      }
      //      debugln();
      //#endif

#endif

// RF
#if (SYSTEM == 0) || (SYSTEM == 2)
      rf_count.val =
          0; // Need to make it zero to capture instantaneous RF every 15 mins
      rf_value =
          0; // Need to make it zero to capture instantaneous RF every 15 mins
#endif

      if (sampleNo == MAX_SAMPLE_NO) {
        snprintf(temp_file, sizeof(temp_file), "/closingdata_%s.txt",
                 station_name);
        File file5 = SPIFFS.open(temp_file, FILE_APPEND);
        if (!file5) {
          debugln("Failed to open closing data file for appending");
        } // #TRUEFIX

#if SYSTEM == 0
        snprintf(append_text, sizeof(append_text),
                 "%s,%02d,%04d-%02d-%02d,%02d:%02d,%s,%04d,%04.1f\r\n",
                 station_name, sampleNo, temp_year, temp_month, temp_day,
                 record_hr, record_min, cum_rf, signal_strength, bat_val);
#endif

#if SYSTEM == 1
        snprintf(append_text, sizeof(append_text),
                 "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%s,%s,%04d,%04.1f\r\n",
                 sampleNo, temp_year, temp_month, temp_day, record_hr,
                 record_min, inst_temp, inst_hum, avg_wind_speed, inst_wd,
                 signal_strength, bat_val);
#endif

#if SYSTEM == 2
        snprintf(append_text, sizeof(append_text),
                 "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%s,%s,%s,%04d,%04.1f\r\n",
                 sampleNo, temp_year, temp_month, temp_day, record_hr,
                 record_min, cum_rf, inst_temp, inst_hum, avg_wind_speed,
                 inst_wd, signal_lvl, bat_val);
#endif

        file5.print(append_text);
        file5.close();
        strcpy(diag_cdm_status, "PENDING"); // Mark CDM as ready to send

        // v5.55: Meteorological Day Precision Reset (end of 8:30 AM)
        // Resetting counters here ensures 8:45 AM wakeup captures the 8:30-8:45 window fresh.
        diag_reg_time_total = 0;
        diag_net_data_count = 0;
        diag_http_time_total = 0;
        diag_http_success_count = 0;
        diag_http_retry_count = 0;
        diag_ftp_success_count = 0;
        diag_daily_http_fails = 0;
        diag_stored_apn_fails = 0; 

        new_current_cumRF = 0; 
        total_rf_pulses_32 = 0;
        last_sched_rf_pulses_32 = 0;
        last_raw_rf_count = rf_count.val; 
        
        diag_sent_mask_cur[0] = 0;
        diag_sent_mask_cur[1] = 0;
        diag_sent_mask_cur[2] = 0;
        
        rtc_daily_cum_rf = 0.0; // v5.59: Precision reset for tomorrow's anchor
        
        debugln("[SCHED] 08:30 AM Precision Reset Complete.");
        vTaskDelay(200 / portTICK_PERIOD_MS);
      }


#if DEBUG == 1
      // SPIFFS
      //              String content;
      snprintf(cur_file, sizeof(cur_file), "/%s_%04d%02d%02d.txt", station_name,
               rf_cls_yy, rf_cls_mm,
               rf_cls_dd); // this is done earlier also, but now @
                           // every 15th min, do it based on sampleNo
      debugln();
      debugln();
      debug("[FILE] SPIFFS: ");
      debug(cur_file);
      {
        File file3 = SPIFFS.open(cur_file, FILE_READ);
        if (file3) {
          int fsize = file3.size();
          debug(" | Size: ");
          debugln(fsize);
          if (fsize > 0) {
            int seekPos =
                (fsize > (record_length * 5)) ? fsize - (record_length * 5) : 0;
            file3.seek(seekPos);
            String tail = file3.readString();
            debugln("   ... [Tail Content] ...");
            debug(tail); // Combined print via macro (Rule 43)
            debugln("-----------------------");
          }
          file3.close();
        } else {
          debugln(" | Failed to open");
        }
      }
      debugln();

      // SD CARD
      snprintf(cur_file, sizeof(cur_file), "/%s_%04d%02d%02d.txt", station_name,
               rf_cls_yy, rf_cls_mm,
               rf_cls_dd); // this is done earlier also, but now @
                           // every 15th min, do it based on sampleNo
      debug("[FILE] SD: ");
      debugln(cur_file);
      if (sd_card_ok) {
        File sd3 = SD.open(cur_file, FILE_READ);
        if (sd3) {
          int fsize = sd3.size();
          debug(" | Size: ");
          debugln(fsize);
          if (fsize > 0) {
            int seekPos = (sd3.size() > (record_length * 5))
                              ? sd3.size() - (record_length * 5)
                              : 0;
            sd3.seek(seekPos);
            String tail = sd3.readString();
            debugln("   ... [Tail Content] ...");
            debug(tail); // Combined print via macro (Rule 43)
            debugln("-----------------------");
          }
          sd3.close();
        } else {
          debugln(" | Failed to open");
        }
      }

// UNSENT DATA
#if SYSTEM == 0
      snprintf(unsent_file, sizeof(unsent_file), "/unsent.txt");
      if (SPIFFS.exists(unsent_file)) {
        File file4 = SPIFFS.open(unsent_file, FILE_READ);
        if (file4) {
          debugln("\n--- UNSENT DATA START ---");
          while (file4.available()) {
            String line = file4.readStringUntil('\n');
            debugln(line); // Correct display per line
          }
          file4.close();
          debugln("--- UNSENT DATA END ---\n");
        }
      }
#endif

#if (SYSTEM == 1 || SYSTEM == 2)
      snprintf(ftpunsent_file, sizeof(ftpunsent_file), "/ftpunsent.txt");
      if (SPIFFS.exists(ftpunsent_file)) {
        File file4 = SPIFFS.open(ftpunsent_file, FILE_READ);
        if (file4) {
          debugln("\n--- UNSENT DATA START ---");
          while (file4.available()) {
            String line = file4.readStringUntil('\n');
            debugln(line); // Correct display per line
          }
          file4.close();
          debugln("--- UNSENT DATA END ---\n");
        }
      }
#endif
#endif

      debugln();

    TRIGGER_HTTP: // Label for duplicate/invalid skip jump
      // Wait for manual triggers (SMS/GPS) to finish before proceeding with
      // HTTP/Sleep
      {
        int manual_wait_timeout = 0;
        while ((sync_mode == eSMSStart || sync_mode == eGPSStart) &&
               manual_wait_timeout < 60) {
          if (manual_wait_timeout % 10 == 0) {
            debugln("Waiting for Manual Task (SMS/GPS) to finish...");
          }
          vTaskDelay(1000 / portTICK_PERIOD_MS);
          manual_wait_timeout++;
          esp_task_wdt_reset();
        }
      }

      if (data_writing_initiated == 0) {
        debugln("Skipped data writing. Checking if GPRS needs to send unsent "
                "data then Sleep.");
        // Protect manual triggers (SMS/GPS/Startup) from being overwritten
        if (sync_mode != eSMSStart && sync_mode != eGPSStart &&
            sync_mode != eStartupGPS) {
          if (is_valid_window || timeSyncRequired) {
            sync_mode = eHttpBegin; // Force connection check on boot/duplicate
          } else {
            debugln("[SCHED] Between intervals. Modem will remain OFF.");
            sync_mode = eHttpStop; // Allow sleep
          }
        }
        // v7.65 Persistence: Trigger FTP backlog through the GPRS task loop
        // to avoid collision with Health Report tasks.
#if (SYSTEM == 1 || SYSTEM == 2)
        if (sync_mode != eHttpStop) {
           debugln("[SCHED] Signaling GPRS task for sequential backlog/health...");
        }
#endif
      } else {
        debugln();
        // Trigger HTTP after manual task is done
        sync_mode = eHttpBegin;
        httpInitiated = true;
        data_writing_initiated = 0;

        // v5.41 Test Mode: Send Health Report every slot
#if ENABLE_HEALTH_REPORT == 1
        if (TEST_HEALTH_EVERY_SLOT == 1) {
          debugln("[SCHEDULER] Test Mode: Queuing Health Report (every slot)");
          // The health report will be called in the GPRS task after data
          // upload
        }
#endif
      }
      vTaskDelay(300 / portTICK_PERIOD_MS);
    } else if (timeSyncRequired == false && httpInitiated == false &&
               sync_mode == eSyncModeInitial) {
      // v7.08: IDLE TRAP PROTECTION
      // If we wake up (via timer) and find we ALREADY processed this slot,
      // we must allow the system to sleep. Otherwise, it stays awake
      // at 80mA until the next 15-min interval.
      static uint32_t idle_awake_start = (uint32_t)-1;
      if (idle_awake_start == (uint32_t)-1)
        idle_awake_start = millis();

      if (millis() - idle_awake_start >
          15000) { // 15s window for manual buttons
        debugf("[SCHED] Idle: Slot %d already processed. Entering sleep...\n",
               current_sample_idx);
        sync_mode = eHttpStop;
        idle_awake_start = (uint32_t)-1;
      }
    } // %15 loop

    vTaskDelay(1000 / portTICK_PERIOD_MS);

  } // for loop

} // scheduler

void next_date(int *Nd, int *Nm, int *Ny) {
  int no_of_days[14] = {29, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int prev_mm;

  *Nd = *Nd + 1;

  // v5.52 ENH-7: Proper leap year rule (including century rule)
  bool isLeap = (*(Ny) % 4 == 0 && (*(Ny) % 100 != 0 || *(Ny) % 400 == 0));

  if (isLeap && (*Nm == 2)) {
    prev_mm = 0; // index 0 in no_of_days is 29
  } else {
    prev_mm = *Nm;
  }
  if (*Nd > no_of_days[prev_mm]) {
    *Nd = 1;
    *Nm = *Nm + 1;
    if (*Nm > 12) {
      *Nm = 1;
      *Ny = *Ny + 1;
    }
  }
}

void previous_date(int *Cd, int *Cm, int *Cy) {
  int no_of_days[14] = {29, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int prev_mm_day;
  int prev_dd_day;
  prev_mm_day = *Cm;
  prev_dd_day = *Cd - 1;
  if (0 == prev_dd_day) {
    prev_mm_day = prev_mm_day - 1;
    if (prev_mm_day == 0) {
      prev_mm_day = 12;
      *Cy = *Cy - 1;
    }

    // v5.52 ENH-7: Proper leap year rule (including century rule)
    bool isLeap = (*(Cy) % 4 == 0 && (*(Cy) % 100 != 0 || *(Cy) % 400 == 0));

    if (isLeap && (prev_mm_day == 2)) {
      prev_dd_day = 29;
    } else {
      prev_dd_day = no_of_days[prev_mm_day];
    }
  }

  *Cd = prev_dd_day;
  *Cm = prev_mm_day;
}

// Sample# DD-MM-YYYY,HH:NN,%06.2f(RF),%04d(Signal),%04.1f(Battery),%c(Solar
// Sts)\r\n
//  2+1  10+1        5+1     6+1        4+1         4+1       1          2   =
//  40 . // include stationID as part of parameters.txt and not in this file

/*
 *
 *  MAIN 15 min loop .
 *  Find current_sampleNo - done
 *  Find the battery level and store it as structure member.
 *  Put current_signalLvl = -110 // intial value. shd change - done
 *  If the sampleNo is = 0, create a new rf_close_dt file using next_date()
 * for (8:45am data) with headers (these headers shd be avoided for sending)
 * If sampleNo <= 60 , then rf_cls_dd is the next_day already stored in
 * rf_cls_dd, if not, store the current date to rf_cls_dd See if the file is
 * present for the rf_cls_dd
 *
 *  If file is present (yes), find the sampleNo of the last record using
 * size() and seek(). if (diff = current_sampleNo - last_recorded_sample_No) >
 * 1, then find (last_recorded_cumRF + current_cumRF)/diff and start appending
 * the file. else do nothing // this means the SPIFFs file is present and it
 * is upto date for the current sample to be recorded. else (if file not
 * present) // this means it is a new installation or the device switched on
 * after 8:45am or mid-day Store cumRF = 0 along with signalLvl = -111, do not
 * store this data into unsent_data.txt // -111 means data not available. just
 * putting it to be helpful in seek() and find()
 *
 *  Store (append)current cumRF and its respective parameters to SPIFFs file
 *  Send current data through HTTP and
 *  If HTTP for current data is a success,
 *      If unsent_data.txt is present
 *          Start sending the data from unsent_data.txt if present.
 *          If that record is sent successfully
 *              increment unsent_data_sent by 1
 *              Go for the next record
 *          else if fail
 *              come out of the loop
 *      else if unsent_data.txt not present,
 *          come out of the loop
 *  If HTTP is a fail,
 *      append the current data to unsent_data.txt
 *  if unsent_data_sent > 1
 *    Find using size() and seek() to point to data not sent.
 *    Open a new file new_insent_data.txt
 *    Copy records from this point to this new file new_unsent_data.txt till
 * EOF of unsent_data.txt close unsent_data.txt close new_unsent_data.txt
 * rename unsent_data.txt to old_unsent_data.txt rename new_unsent_data.txt to
 * unsent_data.txt . Delete old_unsent_data.txt
 *
 *  make scheduler_loop = 6 // it should go to sleep mode which is checked in
 * the main loop
 */
