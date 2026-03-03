#include "globals.h"

int active_cid = 1;      // Default to 1
bool http_ready = false; // v5.42: Track if HTTPINIT succeeded for this cycle

// Helper to clear UART buffer to prevent stale data contamination between tasks
void flushSerialSIT() {
  while (SerialSIT.available()) {
    SerialSIT.read();
    esp_task_wdt_reset();
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void gprs(void *pvParameters) {
  esp_task_wdt_add(NULL);

  for (;;) {
    esp_task_wdt_reset();

    // Debug: Check sync_mode at loop start (Only on change)
    static int last_debug_sync_mode = -1;
    if (sync_mode != eSyncModeInitial && sync_mode != last_debug_sync_mode) {
      if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
        debugf2("[GPRS] State Change: sync_mode=%d gprs_mode=%d\n", sync_mode,
                gprs_mode);
        xSemaphoreGive(serialMutex);
      }
      last_debug_sync_mode = sync_mode;
    } else if (sync_mode == eSyncModeInitial) {
      last_debug_sync_mode = eSyncModeInitial;
    }

    // --- MANUAL TRIGGERS (Keypad) ---
    if (sync_mode == eSMSStart || sync_mode == eGPSStart ||
        sync_mode == eStartupGPS) {
      int target_fld =
          (sync_mode == eSMSStart) ? FLD_SEND_STATUS : FLD_SEND_GPS;

      if (gprs_mode == eGprsInitial) {
        debugln("[GPRS] Manual Trigger: Initiating Power On...");
        strcpy(ui_data[target_fld].bottomRow, "GPRS POWER ON...");
        start_gprs();
        debugf2("Start_GPRS done. gprs_mode=%d sync_mode=%d\n", gprs_mode,
                sync_mode);

        // If start_gprs failed, don't proceed
        if (gprs_mode != eGprsSignalOk) {
          debugln("[GPRS] Initialization failed. Aborting trigger.");
          strcpy(ui_data[target_fld].bottomRow, "NETWORK ERROR");
          vTaskDelay(3000);
          sync_mode = eSMSStop;
          continue; // Skip to next loop iteration
        }
      }

      if (gprs_mode == eGprsSignalOk) {
        strcpy(ui_data[target_fld].bottomRow, "CONNECTING...");
        if (sync_mode == eSMSStart) {
          debugln("[GPRS] Keypad Triggered SMS Status");

          // Re-verify network & APN for manual triggers (Ensures connection is
          // live)
          get_signal_strength();
          get_network();
          get_registration();
          get_a7672s();

          strcpy(ui_data[target_fld].bottomRow, "SENDING...");
#if ENABLE_HEALTH_REPORT == 1
          send_health_report(false); // Manual trigger: NO JITTER
#else
          debugln("[Health] Manual send skipped (disabled).");
#endif
          prepare_and_send_status(universalNumber);
          if (sync_mode == eSMSStart)
            sync_mode = eSMSStop;
        } else if (sync_mode == eGPSStart) {
          debugln("[GPRS] Keypad Triggered GPS Send");

          get_signal_strength();
          get_network();
          get_registration();
          get_a7672s();

          strcpy(ui_data[target_fld].bottomRow, "SENDING...");
          get_lat_long_date_time(universalNumber);
          if (sync_mode == eGPSStart)
            sync_mode = eSMSStop;
        } else if (sync_mode == eStartupGPS) {
          debugln(
              "[GPRS] Startup/Station Change Triggered GPS & Health Report");
          if (signal_strength == 0) {
            get_signal_strength();
            get_network();
            get_registration();
            get_a7672s();
          }
          strcpy(ui_data[target_fld].bottomRow, "GETTING GPS...");
          get_gps_coordinates();

          // If fresh fix failed, try loading from SPIFFS before showing FAILED
          if (lati == 0 || longi == 0)
            loadGPS();

          // Visual confirmation of fetched coordinates
          if (lati != 0 || longi != 0) {
            snprintf(ui_data[target_fld].bottomRow,
                     sizeof(ui_data[target_fld].bottomRow), "%.3f,%.3f", lati,
                     longi);
          } else {
            strcpy(ui_data[target_fld].bottomRow, "GPS FAILED");
          }
          vTaskDelay(2000); // Allow user to see coordinates

          // Keep coordinates visible during send (don't overwrite)
#if ENABLE_HEALTH_REPORT == 1
          send_health_report(false);
#else
          debugln("[Health] Startup send skipped (disabled).");
#endif

          sync_mode = eSMSStop;
          msg_sent = 1; // Mark as success for UI feedback
        }

        if (msg_sent == 1) {
          if (target_fld == FLD_SEND_GPS) {
            memset(ui_data[target_fld].bottomRow, 0,
                   sizeof(ui_data[target_fld].bottomRow));
            snprintf(ui_data[target_fld].bottomRow,
                     sizeof(ui_data[target_fld].bottomRow), "%0.3f,%0.3f", lati,
                     longi);
          } else {
            memset(ui_data[target_fld].bottomRow, 0,
                   sizeof(ui_data[target_fld].bottomRow));
            strcpy(ui_data[target_fld].bottomRow, "SENT SUCCESS    ");
          }
        } else {
          memset(ui_data[target_fld].bottomRow, 0,
                 sizeof(ui_data[target_fld].bottomRow));
          strcpy(ui_data[target_fld].bottomRow, "SEND FAILED     ");
        }
        vTaskDelay(5000); // Wait longer to let user read result

        // After showing result, revert to idle state
        if (target_fld != FLD_SEND_GPS) {
          memset(ui_data[target_fld].bottomRow, 0,
                 sizeof(ui_data[target_fld].bottomRow));
          strcpy(ui_data[target_fld].bottomRow, "YES ?           ");
        }

        msg_sent = 0; // Reset for next trigger
      } else {
        debugln("[GPRS] Cannot send SMS/GPS: SIM Error or No Network");
        memset(ui_data[target_fld].bottomRow, 0,
               sizeof(ui_data[target_fld].bottomRow));
        strcpy(ui_data[target_fld].bottomRow, "NETWORK ERROR   ");
        vTaskDelay(3000);
        memset(ui_data[target_fld].bottomRow, 0,
               sizeof(ui_data[target_fld].bottomRow));
        strcpy(ui_data[target_fld].bottomRow, "YES ?           ");
        if (sync_mode == eSMSStart || sync_mode == eGPSStart ||
            sync_mode == eStartupGPS)
          sync_mode = eSMSStop;
      }
    }

    if (!rtcReady) {
      vTaskDelay(5000);
      continue;
    }
    if (gprs_mode == eGprsInitial) {
      vTaskDelay(100);
      if (gprs_started == false) {
        debugln("[GPRS] Powering On...");
        signal_strength = 0;
        signal_lvl = 0;
        strcpy(reg_status, "NA");
        start_gprs();
      }
    }

    // --- REGULAR AUTOMATED DATA REPORTING ---
    if (timeSyncRequired == false) {
      if (sync_mode == eHttpBegin) {
        if (gprs_mode == eGprsSignalOk) {
          if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
            debugln("\n****************\nStarting Automated Data "
                    "Flow\n****************");
            xSemaphoreGive(serialMutex);
          }

          // v5.48 Daily Health Triggering (09:45 AM Primary, 10:45 AM Retry)
          bool is_health_time = false;
          if (TEST_HEALTH_EVERY_SLOT == 1) {
            is_health_time = true;
          } else {
            if ((current_hour == 10 || current_hour == 16 ||
                 current_hour == 22)) {
              if (health_last_sent_day != current_day ||
                  health_last_sent_hour != current_hour) {
                is_health_time = true;
              }
            }
          }

          // v5.76 Post-OTA Confirmation Health Report Check
          bool is_ota_confirm = false;
          if (diag_fw_just_updated && gprs_mode == eGprsSignalOk) {
            is_ota_confirm = true;
            diag_fw_just_updated = false; // Reset persistent flag
            strcpy(diag_cdm_status, "Firmware Updated");
            // Refresh Location via Fresh CLBS
            get_gps_coordinates();
          }

          if (is_health_time || is_ota_confirm) {
            health_in_progress = true; // Block deep sleep
          }

          debugln("[BATT] Proceeding with Spatika Upload regardless of battery "
                  "voltage.");
          sync_mode = eHttpStarted;
          send_http_data();
          primary_data_delivered =
              (success_count == 1); // v5.51 Track session success
          httpInitiated = false;

          if (health_in_progress) {
#if ENABLE_HEALTH_REPORT == 1
            debugln("[Health] Triggering Reliable Health Report...");
            // v7.01: Consolidate reset logic. send_health_report now handles
            // termination.
            vTaskDelay(3000);

            bool success = send_health_report(true);

            // v5.77: Flag day/hour as attempted even on failure to prevent
            // 15-min retry loops
            health_last_sent_day = current_day;
            health_last_sent_hour = current_hour;

            if (!force_ota) {
              health_in_progress = false; // Finished attempt - no OTA pending
            } else {
              debugln(
                  "[Health] OTA Pending. Holding health_in_progress=TRUE to "
                  "block sleep.");
            }

            if (success) {
              debugln("[Health] ✅ Report sent successfully!");
              if (strcmp(diag_cdm_status, "PENDING") == 0) {
                strcpy(diag_cdm_status, "OK"); // CDM Delivered
              }
            } else {
              debugln("[Health] ❌ Report failed.");
              if (strcmp(diag_cdm_status, "PENDING") == 0) {
                strcpy(diag_cdm_status, "FAIL"); // CDM Missed
              }
            }
#else
            debugln("[Health] Skip: Health reporting disabled.");
            health_in_progress = false; // Allow sleep
#endif
          }
        } else if (gprs_mode == eGprsSignalForStoringOnly) {
          store_current_unsent_data();
          sync_mode = eHttpStop;
          httpInitiated = false;
        }
      }
    }

    // --- REMOTE COMMAND EXECUTION (Command Piggybacking) ---
    if (force_reboot) {
      debugln("[CMD] Remote HARD REBOOT triggered. Killing Modem Power...");
      digitalWrite(26, LOW); // Kill VCC to A7672S
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      debugln("[CMD] Restarting ESP32...");
      ESP.restart();
    }

    if (force_ftp) { // v5.80: FTP_BACKLOG
      debugln("[CMD] Remote FTP_BACKLOG triggered. Syncing unsent backlog...");
#if (SYSTEM == 1 || SYSTEM == 2)
      send_unsent_data();
#endif
      force_ftp = false;
    }

    if (force_ftp_daily) { // v5.80: FTP_DAILY
      debugf1("[CMD] Remote FTP_DAILY triggered for date: %s\n",
              ftp_daily_date);
#if (SYSTEM == 1 || SYSTEM == 2)
      send_daily_file(ftp_daily_date);
#endif
      force_ftp_daily = false;
    }

    if (force_ota) {
      health_in_progress = true; // Ensure sleep is blocked
      debugln("[CMD] Remote OTA_CHECK triggered. Checking for updates...");
      // v7.03: Ensure HTTP is closed from health report before OTA begins
      SerialSIT.println("AT+HTTPTERM");
      waitForResponse("OK", 2000);
      http_ready = false;
      vTaskDelay(2000 / portTICK_PERIOD_MS); // Stabilize modem stack
      if (strlen(ota_cmd_param) > 0) {
        fetchFromHttpAndUpdate(ota_cmd_param);
      } else {
        char defaultUpdate[] = "firmware.bin";
        fetchFromHttpAndUpdate(defaultUpdate);
      }
      force_ota = false;
      if (!force_reboot) {
        health_in_progress =
            false; // Allow sleep now (only if download failed/skipped)
      } else {
        debugln("[GPRS] Reboot pending. Holding health_in_progress=TRUE.");
      }
    }

    // v5.76 Post-OTA Logic moved to Unified Health section above to prevent
    // conflicts

    esp_task_wdt_reset();
    vTaskDelay(1000); // Reduced from 2000 for faster state transitions
  }
}

void start_gprs() {
  // Power ON GPRS
  digitalWrite(26, HIGH); // Power on GPRS module
  vTaskDelay(500);        // v5.64: Reduced from 1s for fast-boot
  debugln("[GPRS] Waiting for active UART...");

  // Active fast-polling for Modem Boot instead of a blind 10-second wait
  bool modem_ready = false;
  for (int i = 0; i < 25; i++) {
    SerialSIT.println("AT");
    if (waitForResponse("OK", 400).indexOf("OK") != -1) {
      modem_ready = true;
      debugf1("[GPRS] Modem ready in %d iterations!\n", i + 1);
      break;
    }
    vTaskDelay(400 / portTICK_PERIOD_MS);
  }

  if (!modem_ready) {
    debugln("[GPRS] AT Timeout, trying CPIN anyway...");
  }
  // PROACTIVE SIM POLLING: Instead of a blind 2-second wait, poll for SIM
  // readiness. This allows us to proceed as soon as the SIM is ready, saving
  // significant active power.
  bool sim_ready = false;
  debugln("[GPRS] Polling for SIM (CPIN)...");
  for (int i = 0; i < 10; i++) {
    SerialSIT.println("AT+CPIN?");
    String cpin_resp = waitForResponse("+CPIN: READY", 1000);
    if (cpin_resp.indexOf("+CPIN: READY") != -1) {
      sim_ready = true;
      debugf1("[GPRS] SIM ready in %d ms!\n", (i + 1) * 1000);
      break;
    }
    if (cpin_resp.indexOf("+CME ERROR") != -1) {
      debugln("[GPRS] SIM Error detected during polling.");
      break;
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }

  SerialSIT.println("ATE0"); // Echo OFF to prevent parsing races
  waitForResponse("OK", 500);

  if (!sim_ready) {
    diag_consecutive_sim_fails++;
    signal_strength = -114;
    debugln("[SIM] ERROR DETECTED (Timeout or CME).");
    strcpy(reg_status, "SIM ERROR");
    strcpy(diag_reg_fail_type, "SIM_ERR");

    if (diag_consecutive_sim_fails >= 6) {
      debugln("[SIM] PERSISTENT ERROR. Final resort: ESP32 RESTART...");
      vTaskDelay(1000);
      ESP.restart();
    }
    gprs_mode = eGprsSignalForStoringOnly;
  } else {
    diag_consecutive_sim_fails = 0;
    get_signal_strength();
    // [v5.63] Low-Signal Fail-Fast:
    // If signal is weaker than -98 dBm, don't bother with the long registration
    // dance. Marginal signals often fail during HTTP POST, wasting ~45s of
    // active battery.
    if (signal_lvl <= -98) {
      debugln("[GPRS] Signal too weak (" + String(signal_lvl) +
              " dBm). Failing fast to save power.");
      gprs_mode = eGprsSignalForStoringOnly;
    }

    if (gprs_mode != eGprsSignalForStoringOnly) {
      get_network();
      get_registration();
      if (gprs_mode != eGprsSignalForStoringOnly) {
        get_a7672s();
      }
    }
  }
}

int parseHttpDate(const char *dateStr, struct tm *tm) {
  // Parse date string in format "Sun, 06 Nov 1994 08:49:37 GMT"
  // Use strptime to parse the date string to tm struct
  //            strptime(date_str, "%a, %d %b %Y %H:%M:%S %Z", &timeinfo);

  if (strptime(dateStr, "%a, %d %b %Y %H:%M:%S GMT", tm) == NULL) {
    return -1; // Failed to parse date
  }
  // Adjust for tm structure values (e.g., year since 1900)
  tm->tm_isdst = -1; // Not considering daylight saving time
  return 0;
}
/*
 *          HTTP
 *    - prepare_data_and_send()
 *    - send_http_data()
 *    - send_at_cmd_data()
 *    - store_current_unsent_data()
 */

void prepare_data_and_send() {
  esp_task_wdt_reset();

  if (data_mode == eCurrentData) {

    debugln();
    debugln("*********  Sending Current data to main server... ***********");
    debugln();
    snprintf(temp_file, sizeof(temp_file), "%s", cur_file);

  } else if (data_mode == eClosingData) {

    debugln();
    debugln("*********  Sending PREVIOUS day's last_data/8:30am data to main "
            "server... ***********");
    debugln();
    previous_rfclose_day = rf_cls_dd;
    previous_rfclose_month = rf_cls_mm;
    previous_rfclose_year = rf_cls_yy;
    previous_date(
        &previous_rfclose_day, &previous_rfclose_month,
        &previous_rfclose_year); // to ge the previous rf_close_date file
    snprintf(temp_file, sizeof(temp_file), "/%s_%04d%02d%02d.txt", station_name,
             previous_rfclose_year, previous_rfclose_month,
             previous_rfclose_day);
  }

  if (data_mode == eCurrentData || data_mode == eClosingData) {
    if (SPIFFS.exists(temp_file)) {
      debug("SPIFF FILE EXISTS ....");
      debugln(temp_file);
      File file1 = SPIFFS.open(temp_file, FILE_READ);
      if (!file1) {
        debugln("Failed to open temp_file for reading");
      } // #TRUEFIX
      s = file1.size();
      s = (s > record_length) ? s - record_length : 0;
      file1.seek(s);
      content = file1.readString(); // Read the rest of the file
      file1.close();
    }
    charArray = content.c_str();
  }

  debugln();
  debugf1("Current Data to be sent is : %s", charArray);
  debugln();

#if SYSTEM == 0
  sscanf(charArray, "%02d,%04d-%02d-%02d,%02d:%02d,%f,%f,%04d,%f",
         &temp_sampleNo, &temp_year, &temp_month, &temp_day, &temp_hr,
         &temp_min, &temp_instrf, &temp_crf, &temp_sig, &temp_bat);
#endif
#if SYSTEM == 1
  // TWS: 12 components — No Rainfall field (TWS is wind+temp+hum only)
  sscanf(charArray, "%02d,%04d-%02d-%02d,%02d:%02d,%f,%f,%f,%03d,%04d,%f",
         &temp_sampleNo, &temp_year, &temp_month, &temp_day, &temp_hr,
         &temp_min, &temp_temp, &temp_hum, &temp_avg_ws, &temp_dir, &temp_sig,
         &temp_bat);
#endif
#if SYSTEM == 2
  // TWS-RF: 13 components
  sscanf(charArray, "%02d,%04d-%02d-%02d,%02d:%02d,%f,%f,%f,%f,%03d,%04d,%f",
         &temp_sampleNo, &temp_year, &temp_month, &temp_day, &temp_hr,
         &temp_min, &temp_crf, &temp_temp, &temp_hum, &temp_avg_ws, &temp_dir,
         &temp_sig, &temp_bat);
#endif
#if (SYSTEM == 0)
  snprintf(sample_bat, sizeof(sample_bat), "%04.1f", float(temp_bat));
#endif
#if (SYSTEM == 1) || (SYSTEM == 2)
  sample_WD[3] = 0; // AG1
  snprintf(sample_temp, sizeof(sample_temp), "%05.1f", float(temp_temp));
  snprintf(sample_hum, sizeof(sample_hum), "%05.1f", float(temp_hum));
  snprintf(sample_avgWS, sizeof(sample_avgWS), "%05.2f", float(temp_avg_ws));
  snprintf(ftpsample_avgWS, sizeof(ftpsample_avgWS), "%05.2f",
           float(temp_avg_ws)); // v1.10 from %04.1f
  snprintf(sample_WD, sizeof(sample_WD), "%03d", temp_dir);
  snprintf(sample_bat, sizeof(sample_bat), "%04.1f", float(temp_bat));
#endif
#if (SYSTEM == 0) || (SYSTEM == 2)
  snprintf(sample_cum_rf, sizeof(sample_cum_rf), "%06.2f", float(temp_crf));
  snprintf(sample_inst_rf, sizeof(sample_inst_rf), "%06.2f",
           float(temp_instrf));
  snprintf(ftpsample_cum_rf, sizeof(ftpsample_cum_rf), "%05.2f",
           float(temp_crf));
#endif

  // v6.75: Use actual signal level (reverted hard-lock to -68)
  temp_sig = signal_lvl;

  if (data_mode == eClosingData) {
    temp_hr = 8;
    temp_min = 30;
  }

  // Create trimmed station name for URL
  char rawStn[16];
  strncpy(rawStn, station_name, 15);
  rawStn[15] = '\0';

  // Trim leading
  char *st = rawStn;
  while (*st == ' ')
    st++;

  char cleanStn[16];
  strncpy(cleanStn, st, 15);
  cleanStn[15] = '\0';

  // Trim trailing
  int tLen = strlen(cleanStn);
  while (tLen > 0 && cleanStn[tLen - 1] == ' ') {
    cleanStn[tLen - 1] = '\0';
    tLen--;
  }

  if (!strcmp(httpSet[http_no].Format,
              "json")) { // if json then this loop otherwise goto urlencoded one
// BIHAR TRG
#if SYSTEM == 0
    snprintf(http_data, sizeof(http_data),
             "{\"StnNo\":\"%s\",\"DeviceTime\":\"%04d-%02d-%02d "
             "%02d:%02d:00\",\"RainDP\":\"%05.2f\",\"RainCuml\":\"%05.2f\","
             "\"BatVolt\":\"%s\",\"SigStr\":\"%04d\",\"ApiKey\":\"%s\"}",
             cleanStn, temp_year, temp_month, temp_day, temp_hr, temp_min,
             temp_instrf, temp_crf, sample_bat, temp_sig, httpSet[http_no].Key);
#endif

  } else {
    // KSNDMC TRG
    if (!strcmp(NETWORK, "KSNDMC")) {
#if (SYSTEM == 0)
      snprintf(http_data, sizeof(http_data),
               "stn_id=%s&rec_time=%04d-%02d-%02d,%02d:%02d&rainfall=%05.2f&"
               "signal=%04d&key=%s&bat_volt=%s&bat_volt1=%s",
               cleanStn, temp_year, temp_month, temp_day, temp_hr, temp_min,
               temp_crf, temp_sig, httpSet[http_no].Key, sample_bat,
               sample_bat);
#endif
    } else if (!strcmp(NETWORK, "SPATIKA")) {
      debugln();
      debug("Key is ");
      debugln(httpSet[http_no].Key);
#if (SYSTEM == 0)
      snprintf(http_data, sizeof(http_data),
               "stn_id=%s&rec_time=%04d-%02d-%02d,%02d:%02d&rainfall=%05.2f&"
               "signal=%04d&bat_volt=%s&key=%s",
               cleanStn, temp_year, temp_month, temp_day, temp_hr, temp_min,
               temp_crf, temp_sig, sample_bat, httpSet[http_no].Key);
      debugln();
      debugln("SPATIKA TRG data ..");
#endif
    }
  } // Ensure this cleanly closes the TRG checks

  // TWS
#if SYSTEM == 1
  if (strstr(UNIT, "SPATIKA_GEN"))
    snprintf(http_data, sizeof(http_data),
             "stn_id=%s&rec_time=%04d-%02d-%02d,%02d:%02d&temp=%s&humid=%s&w_"
             "speed=%s&w_dir=%s&signal=%04d&key=%s&bat_volt=%s&bat_volt2=%s",
             cleanStn, temp_year, temp_month, temp_day, temp_hr, temp_min,
             sample_temp, sample_hum, sample_avgWS, sample_WD, temp_sig,
             httpSet[http_no].Key, sample_bat, sample_bat);
  else
    snprintf(http_data, sizeof(http_data),
             "stn_id=%s&rec_time=%04d-%02d-%02d,%02d:%02d&temp=%s&humid=%s&w_"
             "speed=%s&w_dir=%s&signal=%04d&key=%s&bat_volt=%s&bat_volt2=%s",
             cleanStn, temp_year, temp_month, temp_day, temp_hr, temp_min,
             sample_temp, sample_hum, sample_avgWS, sample_WD, temp_sig,
             httpSet[http_no].Key, sample_bat, sample_bat);
#endif

    // TWS-RF
#if SYSTEM == 2
  if (strstr(UNIT, "SPATIKA_GEN")) {
    // v6.75: Reverted to stn_id as per user confirmation
    // v6.81: Reverted to stn_id and comma as per user confirmation from v4.29
    snprintf(http_data, sizeof(http_data),
             "stn_id=%s&rec_time=%04d-%02d-%02d,%02d:%02d&key=%s&rainfall=%05."
             "2f&temp=%s&humid=%s&w_speed=%s&w_dir=%s&signal=%04d&"
             "bat_volt=%s&bat_volt2=%s",
             cleanStn, temp_year, temp_month, temp_day, temp_hr, temp_min,
             httpSet[http_no].Key, temp_crf, sample_temp, sample_hum,
             sample_avgWS, sample_WD, temp_sig, sample_bat, sample_bat);
  } else
    // v6.81: Reverted to stn_id and comma as per user confirmation from v4.29
    snprintf(http_data, sizeof(http_data),
             "stn_id=%s&rec_time=%04d-%02d-%02d,%02d:%02d&key=%s&rainfall=%05."
             "2f&temp=%s&humid=%s&w_speed=%s&w_dir=%s&signal=%04d&bat_volt=%s&"
             "bat_volt2=%s",
             cleanStn, temp_year, temp_month, temp_day, temp_hr, temp_min,
             httpSet[http_no].Key, temp_crf, sample_temp, sample_hum,
             sample_avgWS, sample_WD, temp_sig, sample_bat, sample_bat);
#endif

  debug("http_data format is ");
  debugln(http_data);
  debugln();
  success_count = send_at_cmd_data(http_data, charArray);
  if (success_count == 0 && data_mode == eCurrentData) {
    debugln("[HTTP] 1st Attempt failed. Retrying in 2s...");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    success_count = send_at_cmd_data(http_data, charArray);
  }
  // v6.76: One retry added above for current data.
  // Fails after retry will trigger FTP backlog storage.

  if (success_count == 1) {
    if (data_mode == eCurrentData) {
      diag_first_http_count++;
    }
  }
  vTaskDelay(200);

  unsent_counter = 0; // Initialize each time ...

  //      if(success_count == 0) { // Failure to send the current data in 1st
  //      attempt
  //            for(unsent_counter=1; unsent_counter <=5 && http_code!=200;
  //            unsent_counter++)  {
  //              esp_task_wdt_reset();
  //                debug("Retrying ");debug(unsent_counter);debugln(" times");
  //                success_count = send_at_cmd_data(http_data,charArray);
  //                vTaskDelay(200);
  //
  //                if(success_count == 1){
  //                  debug("Success in sending current data .. Attempt :
  //                  ");debugln(unsent_counter);
  //
  //
  //                  break;
  //                }
  //             }
  //       }

  if (success_count == 0) { // First attempt failed
    // v5.54: SIMPLIFIED RETRY STRATEGY
    // ─────────────────────────────────────────────────────────────────────
    // Errors 706/713/714 are TCP/server-path failures, NOT bearer failures.
    // Bearer resets, CGACT resets, and CREG polling don't fix a server that
    // won't respond — they just waste 1-3 minutes of battery per cycle.
    //
    // New strategy: ONE clean HTTP stack re-init, ONE retry.
    // If that fails → store to backlog immediately and sleep.
    // The 15-min wake cycle + backlog is the recovery mechanism.
    // ─────────────────────────────────────────────────────────────────────

    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
      debugln(
          "[HTTP] Attempt 1 failed. Re-initialising stack for single retry...");
      xSemaphoreGive(serialMutex);
    }
    esp_task_wdt_reset();

    // ─────────────────────────────────────────────────────────────────────
    // v5.64 Fast-Retry: Skip bearer reset for transient 706/713 errors
    // v5.42: TIMEOUT = HTTPACTION URC never arrived (network drop/server slow).
    // Treat it as transient — don't tear down the bearer, just reinit HTTP.
    bool transientErr = (String(diag_http_fail_reason) == "706" ||
                         String(diag_http_fail_reason) == "713" ||
                         String(diag_http_fail_reason) == "714" ||
                         String(diag_http_fail_reason) == "TIMEOUT");

    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 3000);

    if (!transientErr) {
      SerialSIT.println("AT+SAPBR=0,1");
      waitForResponse("OK", 1000);
      vTaskDelay(500 / portTICK_PERIOD_MS);
    } else {
      debugln(
          "[HTTP] Transient error (706/713). Skipping bearer reset for speed.");
      vTaskDelay(200 / portTICK_PERIOD_MS);
    }

    flushSerialSIT();

    // Ensure primary bearer is actually active before retrying
    SerialSIT.println("AT+CGACT?");
    String cgact_resp = waitForResponse("OK", 3000);
    if (cgact_resp.indexOf("+CGACT: " + String(active_cid) + ",1") == -1) {
      if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
        debugln("[RECOVERY] Bearer lost. Hard Re-activating CID 1...");
        xSemaphoreGive(serialMutex);
      }
      SerialSIT.print("AT+CGACT=1,");
      SerialSIT.println(active_cid);
      waitForResponse("OK", 10000);
    }

    SerialSIT.println("AT+HTTPINIT");
    if (waitForResponse("OK", 5000).indexOf("OK") != -1) {
      http_ready = true; // v5.42: Session live for retry attempt
      // Restore all parameters
      SerialSIT.println("AT+HTTPPARA=\"CID\",1"); // v5.58: Hard-lock
      waitForResponse("OK", 1000);

      SerialSIT.println(httpPostRequest);
      waitForResponse("OK", 1000);

      if (!strcmp(httpSet[http_no].Format, "json")) {
        SerialSIT.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
      } else {
        SerialSIT.println(
            "AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"");
      }
      waitForResponse("OK", 1000);

      SerialSIT.println("AT+HTTPPARA=\"ACCEPT\",\"*/*\"");
      waitForResponse("OK", 1000);

      if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
        debugln("[HTTP] Retry attempt...");
        xSemaphoreGive(serialMutex);
      }
      success_count = send_at_cmd_data(http_data, charArray);

      if (success_count == 1) {
        diag_consecutive_reg_fails = 0;
        diag_consecutive_http_fails = 0;
        if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
          debugln("[HTTP] Retry succeeded.");
          xSemaphoreGive(serialMutex);
        }
      }
    } else {
      if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
        debugln("[HTTP] HTTPINIT failed on retry. Skipping to backlog.");
        xSemaphoreGive(serialMutex);
      }
    }

    if (success_count == 0) { // Complete failure
      diag_consecutive_http_fails++;
      diag_daily_http_fails++;
      debugf1("[RECOVERY] Consecutive HTTP Fails: %d\n",
              diag_consecutive_http_fails);

      if (data_mode == eCurrentData) {
        char current_record[record_length + 1];
        debugln("CURRENT DATA : Retries exceeded limit... Storing to backlog.");

        debugln();
#if SYSTEM == 0 // RF
        snprintf(current_record, sizeof(current_record),
                 "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%04d,%04.1f\r\n",
                 temp_sampleNo, temp_year, temp_month, temp_day, temp_hr,
                 temp_min, sample_inst_rf, sample_cum_rf, temp_sig, temp_bat);
#endif
#if SYSTEM == 1 // TWS
        snprintf(current_record, sizeof(current_record),
                 "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%s,%s,%04d,%04.1f\r\n",
                 temp_sampleNo, temp_year, temp_month, temp_day, temp_hr,
                 temp_min, sample_temp, sample_hum, sample_avgWS, sample_WD,
                 temp_sig, temp_bat);
        char stnId[16];
        if (strlen(ftp_station) == 4 && isDigitStr(ftp_station)) {
          snprintf(stnId, sizeof(stnId), "00%s", ftp_station);
        } else {
          strcpy(stnId, ftp_station);
        }

        snprintf(ftpappend_text, sizeof(ftpappend_text),
                 "%s;%04d-%02d-%02d,%02d:%02d;%s;%s;%s;%s;%04d;%04.1f\r\n",
                 stnId, temp_year, temp_month, temp_day, temp_hr, temp_min,
                 sample_temp, sample_hum, sample_avgWS, sample_WD, temp_sig,
                 temp_bat);
        debug("ftpappend_text is : ");
        debugln(ftpappend_text);
#endif
#if SYSTEM == 2 // TWS-RF
        snprintf(current_record, sizeof(current_record),
                 "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%s,%s,%s,%04d,%04.1f\r\n",
                 temp_sampleNo, temp_year, temp_month, temp_day, temp_hr,
                 temp_min, sample_cum_rf, sample_temp, sample_hum, sample_avgWS,
                 sample_WD, temp_sig, temp_bat);
        char stnId[16];
        if (strlen(ftp_station) == 4 && isDigitStr(ftp_station)) {
          snprintf(stnId, sizeof(stnId), "00%s", ftp_station);
        } else {
          strcpy(stnId, ftp_station);
        }

        snprintf(ftpappend_text, sizeof(ftpappend_text),
                 "%s;%04d-%02d-%02d,%02d:%02d;%s;%s;%s;%s;%s;%04d;%04.1f\r\n",
                 stnId, temp_year, temp_month, temp_day, temp_hr, temp_min,
                 ftpsample_cum_rf, sample_temp, sample_hum, sample_avgWS,
                 sample_WD, temp_sig, temp_bat);
        debug("ftpappend_text is : ");
        debugln(ftpappend_text);
#endif
        vTaskDelay(100);

        // SMART RECOVERY: If data fails for 4 consecutive slots (~1 hour),
        // trigger a Full Modem Reset. Use diag_consecutive_http_fails here
        // (NOT diag_consecutive_reg_fails — that counter is only for actual
        // registration failures and gates graceful_modem_shutdown).
        if (diag_consecutive_http_fails >= 4) {
          debugln("[RECOVERY] 4 Consecutive HTTP Fail Slots. Triggering Full "
                  "Modem Reset...");
          SerialSIT.println("AT+CFUN=1,1");
          vTaskDelay(5000);
          diag_consecutive_http_fails =
              0; // Reset counter - next slot is a fresh start
        }

        char finalBuffer[100]; // AG1 [record_length + 1];

#if SYSTEM == 0
        size_t textLength =
            strlen(current_record); // Get the length of the C-style string
        if (textLength > record_length) {
          debugln("WARNING: Text length exceeds maximum allowed characters. "
                  "Truncating...");
          strncpy(finalBuffer, current_record, record_length);
          finalBuffer[record_length] = '\0'; // Manually null-terminate
        } else {
          strcpy(finalBuffer,
                 current_record); // strcpy handles null termination
        }
        snprintf(unsent_file, sizeof(unsent_file), "/unsent.txt");
        File file2 = SPIFFS.open(unsent_file, FILE_APPEND);
        if (file2) {
          file2.print(finalBuffer);
          file2.close();
        } else {
          debugln("Failed to open unsent.txt for appending");
        }
#endif

#if SYSTEM == 1
        // 001881;2025-11-07,20:30;000.0;000.0;00.00;000;-083;04.2
        // 55+2 = 57 is the record length
        size_t textLength =
            strlen(ftpappend_text); // Get the length of the C-style string
        debugln();
        debug("Text length is ");
        debugln(textLength);
        if (textLength > 57) {
          debugln("WARNING: Text length exceeds maximum allowed characters. "
                  "Truncating...");
          strncpy(finalBuffer, ftpappend_text, 57);
          finalBuffer[57] = '\0'; // Manually null-terminate
        } else {
          strcpy(finalBuffer,
                 ftpappend_text); // strcpy handles null termination
        }
        debugln("Record written into /ftpunsent.txt is as below : ");
        debugln(finalBuffer);
        snprintf(ftpunsent_file, sizeof(ftpunsent_file), "/ftpunsent.txt");
        File ftpfile2 = SPIFFS.open(ftpunsent_file, FILE_APPEND);
        if (ftpfile2) {
          ftpfile2.print(finalBuffer);
          ftpfile2.close();
        } else {
          debugln("Failed to open ftpunsent.txt for appending (TWS)");
        }
#endif

#if SYSTEM == 2
        // 001881;2024-05-21,08:45;000.0;000.0;000.0;00.00;000;-111;04.2
        // 61+2 is the record_length
        size_t textLength =
            strlen(ftpappend_text); // Get the length of the C-style string
        debugln();
        debug("Text length is ");
        debugln(textLength);

        if (textLength > 63) {
          debugln("WARNING: Text length exceeds maximum allowed characters. "
                  "Truncating...");
          strncpy(finalBuffer, ftpappend_text, 63);
          finalBuffer[63] = '\0'; // Manually null-terminate
        } else {
          strcpy(finalBuffer,
                 ftpappend_text); // strcpy handles null termination
        }
        debugln("Record written into /ftpunsent.txt is as below : ");
        debugln(finalBuffer);
        snprintf(ftpunsent_file, sizeof(ftpunsent_file), "/ftpunsent.txt");
        File ftpfile2 = SPIFFS.open(ftpunsent_file, FILE_APPEND);
        if (ftpfile2) {
          ftpfile2.print(finalBuffer);
          ftpfile2.close();
        } else {
          debugln("Failed to open ftpunsent.txt for appending (TWS-RF)");
        }
#endif

        debugln("Current/Closing data not sent to unsent.txt is ");
        debugln(current_record);
      } // closes if(data_mode == eCurrentData)
    }   // closes if(success_count == 0)
  }     // closes if(SPIFFS.exists(temp_file))
} // closes prepare_data_and_send()

void send_http_data() {

  /*
   * PREPARE HTTP PARAMS
   */

  const char *domain = httpSet[http_no].serverName;
  char target_ip[64] = {0};

  bool is_ip_format = true;
  for (int i = 0; domain[i] != '\0'; i++) {
    if (isalpha(domain[i])) {
      is_ip_format = false;
      break;
    }
  }

  // Ensure PDP context is active before doing DNS lookups or HTTP
  SerialSIT.println("AT+CGACT?");
  response = waitForResponse("OK", 3000);
  if (response.indexOf("+CGACT: " + String(active_cid) + ",1") == -1) {
    debugln("[GPRS] PDP context inactive. Activating for DNS/HTTP...");
    SerialSIT.print("AT+CGACT=1,");
    SerialSIT.println(active_cid);
    waitForResponse("OK", 10000);
  }

  if (is_ip_format) {
    strcpy(target_ip, domain);
  } else if (strcmp(cached_server_domain, domain) == 0 &&
             strlen(cached_server_ip) > 5) {
    debugln("[DNS Cache] Using cached IP for " + String(domain) + ": " +
            String(cached_server_ip));
    strcpy(target_ip, cached_server_ip);
  } else {
    debugln("[DNS Cache] Resolving " + String(domain) + "...");
    SerialSIT.print("AT+CDNSGIP=\"");
    SerialSIT.print(domain);
    SerialSIT.println("\"");
    String resp = waitForResponse("+CDNSGIP: 1", 10000);

    int ip_start = resp.lastIndexOf(",\"");
    if (ip_start != -1) {
      int ip_end = resp.indexOf("\"", ip_start + 2);
      if (ip_end != -1) {
        String ip_str = resp.substring(ip_start + 2, ip_end);
        if (ip_str.length() > 5) {
          strncpy(cached_server_ip, ip_str.c_str(),
                  sizeof(cached_server_ip) - 1);
          strncpy(cached_server_domain, domain,
                  sizeof(cached_server_domain) - 1);
          strcpy(target_ip, cached_server_ip);
          debugln("[DNS Cache] Resolved: " + ip_str);
        }
      }
    }
    if (target_ip[0] == '\0') {
      if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
        debugln("[DNS Cache] Resolution failed. Falling back to domain name.");
        debugln("[DNS Cache] Force Deactivating CID 8 (Flapping)...");
        xSemaphoreGive(serialMutex);
      }
      SerialSIT.println("AT+CGACT=0,8"); // Cleanup noise
      waitForResponse("OK", 2000);
      SerialSIT.println("AT+CDNSCFG=\"8.8.8.8\",\"1.1.1.1\"");
      waitForResponse("OK", 1000);
      strcpy(target_ip, domain);
    }
  }

  snprintf(httpPostRequest, sizeof(httpPostRequest),
           "AT+HTTPPARA=\"URL\",\"http://%s:%s%s\"", target_ip,
           httpSet[http_no].Port, httpSet[http_no].Link);

  // SMART BEARER CHECK: Avoid redundant CIPSHUT if connection is already live
  // especially useful during combined Health + Main data slots.
  SerialSIT.println("AT+CGACT?");
  response = waitForResponse("OK", 3000);
  if (response.indexOf("+CGACT: " + String(active_cid) + ",1") != -1) {
    debugln("[GPRS] Bearer already live. Skipping CIPSHUT to save time.");
  } else {
    debugln("[GPRS] Starting HTTP...");
    debug("HTTP POST REQUEST IS ");
    debugln(httpPostRequest);
    vTaskDelay(500);

    // Hard reset IP stack to clear any half-open sessions
    // PROACTIVE: Force a hard shut if we've had consecutive HTTP failures
    // (learning)
    if (diag_consecutive_http_fails > 1) {
      debugln(
          "[PROACTIVE] Consecutive failures detected. Forcing deep CIPSHUT...");
      SerialSIT.println("AT+CIPSHUT");
      waitForResponse("SHUT OK", 5000);
      SerialSIT.println("AT+CGACT=0,1");
      waitForResponse("OK", 3000);
    } else {
      SerialSIT.println("AT+CIPSHUT");
      waitForResponse("SHUT OK", 4000);
    }
  }

  // IP Stack ready/cleared
  char gprs_xmit_buf[64];

  // v7.03: Smarter context cleanup (only if needed or if consecutive fails
  // occur)
  if (diag_consecutive_http_fails > 0) {
    int clean_target = (active_cid > 0) ? active_cid : 1;
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CGACT=0,%d",
             clean_target);
    SerialSIT.println(gprs_xmit_buf);
    waitForResponse("OK", 1000);
  }
  SerialSIT.println("AT+HTTPTERM");
  waitForResponse("OK", 3000);
  vTaskDelay(500);
  flushSerialSIT();   // v5.42: Clear stale UART bytes before HTTPINIT
  http_ready = false; // v5.42: Reset session flag

  SerialSIT.println("AT+HTTPINIT");
  response = waitForResponse("OK", 5000);
  if (response.indexOf("OK") == -1) {
    debugln("[GPRS] HTTPINIT Failed. Trying TERM then INIT...");
    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 3000);
    vTaskDelay(500);
    flushSerialSIT();
    SerialSIT.println("AT+HTTPINIT");
    // v5.42: CHECK the return value — if this also fails, mark session as not
    // ready so send_at_cmd_data() fast-fails instead of burning 47s on timeouts
    if (waitForResponse("OK", 5000).indexOf("OK") != -1) {
      http_ready = true;
      debugln("[GPRS] HTTPINIT recovered on 2nd attempt.");
    } else {
      debugln(
          "[GPRS] HTTPINIT failed on both attempts. Will store to backlog.");
    }
  } else {
    http_ready = true;
  }

  // Restore CID Setting (Quiet)
  SerialSIT.println(
      "AT+HTTPPARA=\"CID\",1"); // v5.58: Hard-lock to primary CID 1
  response = waitForResponse("OK", 1000);

  SerialSIT.println("AT+HTTPPARA=\"ACCEPT\",\"*/*\"");
  response = waitForResponse("OK", 1000);

  if (!strcmp(httpSet[http_no].Format, "json")) {
    SerialSIT.println(httpPostRequest);
    response = waitForResponse("OK", 1000);
    SerialSIT.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
    response = waitForResponse("OK", 1000);
    debugln("It is json");
  } else {
    SerialSIT.println(httpPostRequest);
    response = waitForResponse("OK", 1000);
    SerialSIT.println(
        "AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"");
    response = waitForResponse("OK", 1000);
  }

  /*
   * SENDING CURRENT DATA
   */

  debugln();
  debugln("*********  STARTING TO SEND HTTP ... ***********");
  debugln();
  charArray = content.c_str();
  data_mode = eCurrentData; // Set the data mode
  prepare_data_and_send();

  if (success_count == 1) { // Success in sending current data ... Continue
    // Update internal persistent markers for 'Last Logged'
    last_recorded_hr = record_hr;
    last_recorded_min = record_min;
    last_recorded_dd = current_day;
    last_recorded_mm = current_month;
    last_recorded_yy = current_year;

    // Storing last logged data for signature check
    File fileTemp2 = SPIFFS.open("/signature.txt", FILE_WRITE);
    if (fileTemp2) {
      snprintf(signature, sizeof(signature), "%04d-%02d-%02d,%02d:%02d",
               last_recorded_yy, last_recorded_mm, last_recorded_dd,
               last_recorded_hr, last_recorded_min);
      fileTemp2.print(signature);
      fileTemp2.close();
    }

    // Update Internal LCD Markers
    snprintf(last_logged, sizeof(last_logged), "%d-%d-%d,%02d:%02d",
             last_recorded_yy, last_recorded_mm, last_recorded_dd,
             last_recorded_hr, last_recorded_min);
    strcpy(ui_data[FLD_LAST_LOGGED].bottomRow, last_logged);

    debugln();
    debug("**** Storing Last Logged Data as ");
    debugln(signature);

#if SYSTEM == 0

    /*
     * SENDING 8:30 as well as UNSENT DATA IF FILE EXISTS ...
     */
    if (SPIFFS.exists(unsent_file)) {
      File f = SPIFFS.open(unsent_file, FILE_READ);
      size_t fsize = f.size();
      f.close();

      if (fsize == 0) {
        SPIFFS.remove(unsent_file);
        SPIFFS.remove("/unsent_pointer.txt");
      } else {
        debugln();
        debugln("*********  Sending UNSENT data to main server... ***********");
        debugln();
        unsent_pointer_count = 0; // resetting to 1st record of unsent data ..
        unsent_counter = 0;

        if (SPIFFS.exists("/unsent_pointer.txt")) {
          File read_unsent_count =
              SPIFFS.open("/unsent_pointer.txt", FILE_READ);
          if (!read_unsent_count) {
            debugln("Failed to open unsent_pointer.txt for reading");
          } // #TRUEFIX
          String retrieve_counts = read_unsent_count.readStringUntil('\n');
          const char *retrieve = retrieve_counts.c_str();
          unsent_pointer_count = atoi(retrieve);
          debug("Current unsent_pointer_count is ");
          debugln(unsent_pointer_count);
          debug("Line retrieved from unsent count SPIFFs file is ");
          debugln((unsent_pointer_count + record_length) / record_length);
          read_unsent_count.close();
        } else {
          File write_unsent_count =
              SPIFFS.open("/unsent_pointer.txt", FILE_WRITE);
          if (!write_unsent_count) {
            debugln("Failed to open unsent_pointer.txt for writing");
          } // #TRUEFIX
          write_unsent_count.print(unsent_pointer_count);
          debug("Creating unsent count SPIFFs file ");
          debugln(unsent_pointer_count);
          write_unsent_count.close();
        }

        File file1 = SPIFFS.open(unsent_file, FILE_READ);
        if (!file1) {
          debugln("Failed to open unsent_file for reading");
        } // #TRUEFIX
        fileSize = file1.size();

        while (unsent_pointer_count < fileSize) {
          vTaskDelay(100); // iter10

          // FAST PIPELINING RESET: Pre-emptively tear down and rebuild the HTTP
          // session. The remote server usually replies with "Connection: close"
          // which breaks the A7672S state machine if we pipeline directly.
          // This 1.5s overhead PREVENTS the massive ~30-second error timeouts
          // of the '706' fault.
          SerialSIT.println("AT+HTTPTERM");
          waitForResponse("OK", 1000);
          vTaskDelay(100);

          SerialSIT.println("AT+HTTPINIT");
          waitForResponse("OK", 1500);

          SerialSIT.println("AT+HTTPPARA=\"CID\",1"); // v5.58: Hard-lock
          waitForResponse("OK", 500);

          SerialSIT.println(httpPostRequest);
          waitForResponse("OK", 500);

          if (!strcmp(httpSet[http_no].Format, "json")) {
            SerialSIT.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
          } else {
            SerialSIT.println("AT+HTTPPARA=\"CONTENT\",\"application/"
                              "x-www-form-urlencoded\"");
          }
          waitForResponse("OK", 500);

          SerialSIT.println("AT+HTTPPARA=\"ACCEPT\",\"*/*\"");
          waitForResponse("OK", 500);

          debugln();
          debug("Line Number ");
          debugln((unsent_pointer_count + record_length) / record_length);
          file1.seek(unsent_pointer_count);
          content = file1.readStringUntil('\n'); // Read the rest of the file
          charArray = content.c_str();

          // Set the data mode
          data_mode = eUnsentData;
          prepare_data_and_send();

          if (unsent_counter ==
              6) { // if more than 5 retries , still the data is no sent
            debugln();
            debugln("UNSENT DATA : Retries exceeded limit ...");
            debugln();
            debug("unsent_pointer_count in unsent_pointer.txt is updated "
                  "to ");
            debugln(unsent_pointer_count);
            File unsent_count = SPIFFS.open("/unsent_pointer.txt", FILE_WRITE);
            if (!unsent_count) {
              debugln("Failed to open unsent_pointer.txt for writing");
            } // #TRUEFIX
            unsent_count.print(
                unsent_pointer_count); // s is in bytes. Overwrite
                                       // the previous one.
            unsent_count.close();
            debug("File Size is ");
            debugln(fileSize);
            debug("unsent_pointer_count is ");
            debugln(unsent_pointer_count);
            debugln("Pointer to unsent_file updated ...");
            break;
          }
          unsent_pointer_count += record_length; // Go to next record

        } // while loop

        file1.close();

        debug("unsent_pointer_count is :");
        debugln(unsent_pointer_count);
        debug("fileSize is :");
        debugln(fileSize);

        if (unsent_pointer_count == fileSize) {
          debugln("Unsent data sent .. Going to remove the unsent file and "
                  "pointer ...");
          SPIFFS.remove(unsent_file);
          SPIFFS.remove("/unsent_pointer.txt");
        }
      }
    } // if unsent file exists
    else {
      debugln("No unsent file found ...");
    }
#endif

#if (SYSTEM == 1 || SYSTEM == 2)
    // For FTP systems, always attempt to send unsent data if we have
    // signal, even if HTTP fails or is not used.
    if (gprs_mode == eGprsSignalOk) {
      send_unsent_data();
    }
#endif

  } else { // Handle failure to send current data
    debugln("*** Current data couldn't be sent. Backlog will be handled if "
            "connection is stable.");

    diag_pd_count++; // v5.49 Track real-time performance gap

    // v5.41 NDM Tracking (9 PM to 6 AM)
    if (record_hr >= 21 || record_hr < 6) {
      diag_ndm_count++;
      debugf1("[Health] NDM Increment: Fail during night hours. Count: %d\n",
              diag_ndm_count);
    }

#if (SYSTEM == 1 || SYSTEM == 2)
    // v5.58: Skip FTP rescue if HTTP fails.
    // If Primary transport failed, don't waste power on secondary.
    debugln(
        "[Rescue] HTTP Failed. Skipping FTP backlog attempt to save power.");
#endif
  }

  // v5.64: Pruned buffers & Selective SMS
  SerialSIT.println("AT+HTTPTERM");
  waitForResponse("OK", 3000);
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  // v5.65 Selective SMS Check: Guaranteed once an hour (at Minute 0 slot)
  if (record_min == 0) {
    sync_mode = eSMSStart;
    send_sms();
  } else {
    debugln("[GPRS] Skipping SMS check (hourly task).");
    sync_mode =
        eHttpStop; // Ensure we transition to stop state to allow deep sleep
  }

} // end of send_http_data

void send_daily_file(
    const char *dateStr) { // v5.80: Send /dailyftp_YYYYMMDD.txt
  char dailyFile[32];
  snprintf(dailyFile, sizeof(dailyFile), "/dailyftp_%s.txt", dateStr);

  if (SPIFFS.exists(dailyFile)) {
    debugf1("[FTP] Found daily file: %s. Initiating upload...\n", dailyFile);
    get_registration();
    if (gprs_mode == eGprsSignalOk) {
      get_a7672s();
      vTaskDelay(3000 / portTICK_PERIOD_MS);
      esp_task_wdt_reset();
      send_ftp_file(dailyFile);
    } else {
      debugln("[FTP] Skip Daily: No Network.");
    }
  } else {
    debugf1("[FTP] Daily file NOT FOUND: %s\n", dailyFile);
  }
}

void send_unsent_data() { // ONLY FOR TWS AND TWS-ADDON
  debugln("Entering FTP mode and checking if data period is correct for "
          "sending and if there is any file to be sent");
  int ftp_year = rf_cls_yy % 100;
  char fileName[50];

#if SYSTEM == 1
  snprintf(fileName, sizeof(fileName), "/TWS_%s_%02d%02d%02d_%02d%02d00.kwd",
           ftp_station, ftp_year, rf_cls_mm, rf_cls_dd, record_hr, record_min);
#endif

#if SYSTEM == 2
  if (strstr(UNIT, "SPATIKA_GEN"))
    snprintf(fileName, sizeof(fileName),
             "/TWSRF_%s_%02d%02d%02d_%02d%02d00.swd", ftp_station, ftp_year,
             rf_cls_mm, rf_cls_dd, record_hr, record_min);
  else
    snprintf(fileName, sizeof(fileName),
             "/TWSRF_%s_%02d%02d%02d_%02d%02d00.kwd", ftp_station, ftp_year,
             rf_cls_mm, rf_cls_dd, record_hr, record_min);
#endif

  // v5.56: BACKLOG RECOVERY ON EVERY CYCLE
  // Removed 'primary_data_delivered' gate and hourly restriction per user
  // request. Backlog will be attempted whenever signal is usable (-95 dBm or
  // better).
  bool is_daily_slot = (sampleNo == 3 || sampleNo == 7);
  if (!is_daily_slot && signal_lvl > -95 && SPIFFS.exists(ftpunsent_file)) {
    debug("FTP Unsent check - File present: ");
    debugln(SPIFFS.exists(ftpunsent_file));
    debugln();
    debug("FTP file name is ");
    debugln(fileName);
    debugln();
    debug("SampleNo  is ");
    debugln(sampleNo);
    debugln();
    debugln("Entering time bound FTP loop");
    snprintf(ftpunsent_file, sizeof(ftpunsent_file), "/ftpunsent.txt");
    if (SPIFFS.exists(ftpunsent_file)) {
      debugln("[FTP] Checking Registration before backlog upload...");
      // Re-verify registration status to prevent "Ghost Logins" on dropped
      // links
      get_registration();
      if (gprs_mode == eGprsSignalOk) {
        debugln("[FTP] Registration OK. Verifying IP (PDP Context)...");
        get_a7672s(); // Ensure IP is assigned and context is ready

        // v6.45: Increased stabilization delay for better login success
        debugln("[FTP] Assigned IP. Waiting 5s for tower stabilization...");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        esp_task_wdt_reset();

        // v5.51 Efficiency: Use Rename instead of Copy to save power/flash
        // cycles
        SPIFFS.remove(fileName); // Ensure dest is clean
        if (SPIFFS.rename(ftpunsent_file, fileName)) {
          debug("Renamed unsent to ");
          debugln(fileName);
        } else {
          debugln("Rename failed. Falling back to copy.");
          copyFile(ftpunsent_file, fileName);
        }
        debug("Retrieved file is ");
        debugln(fileName);
        esp_task_wdt_reset();
        send_ftp_file(fileName);
      } else {
        debugln("[FTP] Skip: Registration lost. Retrying next hour.");
      }
    } else {
      debugln("No ftpunsent.txt found. Skipping FTP.");
    }
  }

  if (sampleNo == 3 || sampleNo == 7) { // send previous day's data (96
                                        // data) if available through FTP
    if (current_hour == 9 && current_min > 30 && current_min < 45) {
      // Cleanup at start of 09:30 AM cycle
      snprintf(ftpunsent_file, sizeof(ftpunsent_file), "/ftpunsent.txt");
      SPIFFS.remove(ftpunsent_file);
      debug("Cleaned up unsent file at start of new Daily FTP (09:30): ");
      debugln(ftpunsent_file);
    }
    debugln();
    debug("***DAILY FTP file name is ");
    debugln(fileName);
    previous_rfclose_day = rf_cls_dd;
    previous_rfclose_month = rf_cls_mm;
    previous_rfclose_year = rf_cls_yy;
    previous_date(
        &previous_rfclose_day, &previous_rfclose_month,
        &previous_rfclose_year); // to ge the previous rf_close_date file
    snprintf(temp_file, sizeof(temp_file), "/dailyftp_%04d%02d%02d.txt",
             previous_rfclose_year, previous_rfclose_month,
             previous_rfclose_day);

    snprintf(temp_file, sizeof(temp_file), "/dailyftp_%04d%02d%02d.txt",
             previous_rfclose_year, previous_rfclose_month,
             previous_rfclose_day);

    // Maintain original standard filename
    int ftp_year = rf_cls_yy % 100;
#if SYSTEM == 1
    snprintf(fileName, sizeof(fileName), "/TWS_%s_%02d%02d%02d_%02d%02d00.kwd",
             ftp_station, ftp_year, rf_cls_mm, rf_cls_dd, record_hr,
             record_min);
#endif
#if SYSTEM == 2
    snprintf(fileName, sizeof(fileName),
             "/TWSRF_%s_%02d%02d%02d_%02d%02d00.kwd", ftp_station, ftp_year,
             rf_cls_mm, rf_cls_dd, record_hr, record_min);
#endif

    if (SPIFFS.exists(temp_file)) {
      copyFile(temp_file, fileName); // copyFile(source,destination);
      debug("Retrieved file is ");
      debugln(fileName);
      esp_task_wdt_reset();
      send_ftp_file(fileName);
    } else {
      debugln("Daily FTP: Temp file not found. Skipping.");
    }
  }
}

void send_ftp_file(char *fileName) {
  // Use a local module-safe path (removes leading slash for cellular module
  // commands)
  char *modulePath = (fileName[0] == '/') ? &fileName[1] : fileName;

  char gprs_xmit_buf[300];
  String response;
  String rssiStr;
  const char *resp;
  int rssiIndex, rssiEndIndex, retries, registration;
  int handle_no;
  const char *ftpCharArray;
  char *csqstr;
  int success;
  const char *response_char;
  int total_no_of_bytes;
  int s = 0,
      fileSize = 0; // Fixed shadowed and uninitialized local variables
  String content;
  String response1;
  int result = -1;

  flushSerialSIT(); // Ensure UART buffer is clean before starting FTP
                    // sequence
  debugln("Initializing A7672S for FTP...");
  delay(1000); // Allow module to settle

  if (SPIFFS.exists(fileName)) {
    send_daily = 2;
    ftp_login_flag = setup_ftp();

    if (ftp_login_flag == 1) {

      SerialSIT.println("ATE0");
      response = waitForResponse("OK", 3000);
      SerialSIT.println("AT+FSCD=C:/");
      response = waitForResponse("OK", 10000);
      debug("Response of AT+FSCD ");
      debugln(response);

      snprintf(
          gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSOPEN=C:/%s,0\r\n",
          modulePath); // 0 : if the file does not exist, it will be created
      SerialSIT.println(gprs_xmit_buf);
      response = waitForResponse("+FSOPEN", 5000);
      debug("Response is ");
      debugln(response);
      response_char = response.c_str();
      vTaskDelay(200);
      csqstr = strstr(response_char, "+FSOPEN:");
      if (csqstr == NULL) {
        debugln("Error: +FSOPEN not found in response");
        SerialSIT.println("AT+CFTPSLOGOUT");
        waitForResponse("+CFTPSLOGOUT: 0", 9000);
        SerialSIT.println("AT+CFTPSSTOP");
        waitForResponse("OK", 3000);
        return;
      }
      sscanf(csqstr, "+FSOPEN: %d,", &handle_no);

      File file1 = SPIFFS.open(fileName, FILE_READ);
      if (!file1) {
        debugln("Failed to open FTP file for reading");
      } // #TRUEFIX
      debug("File size of the file is ");
      debugln(file1.size());

      fileSize = file1.size();
      file1.seek(s);
      content = file1.readString(); // Read the rest of the file

      if (fileSize == 0) {
        file1.close();
        debugln("Nothing to FTP .. logging out ...");
        SerialSIT.println("AT+CFTPSLOGOUT");
        response = waitForResponse("+CFTPSLOGOUT: 0", 9000);
        debug("Response of AT+CFTPSLOGOUT is ");
        debugln(response);
        vTaskDelay(200);
        SerialSIT.println("AT+CFTPSSTOP");
        waitForResponse("OK", 3000);
        vTaskDelay(200);
        SerialSIT.println("AT+FSLS");
        response = waitForResponse("OK", 10000);

      } else {
        snprintf(
            gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSWRITE=%d,%d,10\r\n",
            handle_no,
            fileSize); // 0 : if the file does not exist, it will be created
        vTaskDelay(200);
        SerialSIT.println(gprs_xmit_buf);
        response = waitForResponse("CONNECT", 5000);
        SerialSIT.println(content);
        response = waitForResponse("+FSWRITE", 5000);
        debug("Response of FSWRITE is ");
        debugln(response);
        file1.close();

        snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSCLOSE=%d",
                 handle_no);
        SerialSIT.println(gprs_xmit_buf);
        response = waitForResponse("OK", 5000); // Fixed: FSCLOSE returns OK
        debug("Response of FSCLOSE is ");
        debugln(response);
        vTaskDelay(1000);

        int ftp_put_retries = 0;
        const int MAX_FTP_PUT_RETRIES = 1;
        bool upload_success = false;

        snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
                 "AT+CFTPSPUTFILE=\"%s\",1", modulePath);

        while (ftp_put_retries <= MAX_FTP_PUT_RETRIES) {
          esp_task_wdt_reset();

          // v5.50: Verify bearer before attempting PUT.
          if (!verify_bearer_or_recover()) {
            debugln("[FTP] Bearer lost before PUT. Recovery failed.");
            break;
          }

          // POWER SAVER: If signal is extremely poor, don't attempt expensive
          // PUT retry as it likely won't finish
          if (signal_lvl < -100 && ftp_put_retries > 0) {
            debugln("[FTP] Skip Retry: Signal too low for reliable PUT. Saving "
                    "Power.");
            break;
          }

          debug("About to send CFTPSPUTFILE ... Attempt: ");
          debugln(ftp_put_retries + 1);

          SerialSIT.println(gprs_xmit_buf); // FTP client context
          response1 = waitForResponse(
              "+CFTPSPUTFILE: 0",
              60000); // v5.50 Optimized: 60s is ample for small txt files
          debug("Response of AT+CFTPSPUTFILE ");
          debugln(response1);

          if (response1.indexOf("+CFTPSPUTFILE: 0") != -1) {
            upload_success = true;
            break;
          } else {
            debugln("FTP PUT failed. Checking recovery...");
            if (ftp_put_retries < MAX_FTP_PUT_RETRIES) {
              debugln("Attempting active recovery for FTP...");
              SerialSIT.println("AT+CFTPSLOGOUT");
              waitForResponse("+CFTPSLOGOUT: 0", 5000);
              vTaskDelay(2000 / portTICK_PERIOD_MS);

              // Re-run setup_ftp to re-establish session
              send_daily = 2;
              if (setup_ftp() == 1) {
                SerialSIT.println("AT+FSCD=C:/");
                waitForResponse("OK", 5000);
              } else {
                debugln("FTP Login Recovery failed.");
                break; // Give up if login failed to save power
              }
            }
            ftp_put_retries++;
          }
        }

        if (upload_success) {
          diag_consecutive_reg_fails =
              0; // RESET counter on any successful data upload

          if (upload_success) {
            markFileAsDelivered(fileName); // v5.48 Track recovered records
          }

          if (sampleNo == 3 || sampleNo == 7) { // If Daily FTP is successful,
                                                // remove the dailyftp file.
            SPIFFS.remove(temp_file);
            debug("Removed Daily FTP file: ");
            debugln(temp_file);
          } else {
            // If it was a generic unsent data send, clear the unsent buffer
            SPIFFS.remove("/ftpunsent.txt");
            debugln("Cleared ftpunsent.txt after successful upload");
          }

          // Remove the *.kwd files. These are the ftp files
          const char *pattern;
          if (strstr(UNIT, "SPATIKA_GEN"))
            pattern = ".swd";
          else
            pattern = ".kwd";

          const String prefix = "/"; // Define your prefix here

          File root = SPIFFS.open("/");
          if (!root) {
            debugln("Failed to open SPIFFS root");
          } // #TRUEFIX
          File file = root.openNextFile();
          while (file) {
            String fileName1 = file.name();
            if (fileName1.endsWith(pattern)) {
              String prefixedFileName = prefix + fileName1;
              debugf1("Removing file: %s\n", prefixedFileName.c_str());
              vTaskDelay(100);
              if (SPIFFS.remove(prefixedFileName)) {
                debugln("File removed successfully");
              } else {
                debugln("File removal failed");
              }
            }
            file = root.openNextFile();
          }
          file.close();

          // Success Cleanup: Remove the temporary file from cellular module
          // storage
          snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSDEL=\"C:/%s\"",
                   modulePath);
          SerialSIT.println(gprs_xmit_buf);
          waitForResponse("OK", 5000);
        } else {
          debugln("Did not succeed in FTP ..");
          // Failed: Clean up ONLY the temporary .swd/.kwd copies.
          // DO NOT delete 'temp_file' or 'ftpunsent_file' (Source Data) so
          // we can retry later.

          // Remove the *.kwd / .swd files.
          const char *pattern;
          if (strstr(UNIT, "SPATIKA_GEN"))
            pattern = ".swd";
          else
            pattern = ".kwd";

          const String prefix = "/";

          File root = SPIFFS.open("/");
          if (root) {
            File file = root.openNextFile();
            while (file) {
              String fileName1 = file.name();
              if (fileName1.endsWith(pattern)) {
                String prefixedFileName = prefix + fileName1;
                debugf1("Removing file (failure cleanup): %s\n",
                        prefixedFileName.c_str());
                vTaskDelay(100);
                SPIFFS.remove(prefixedFileName);
              }
              file = root.openNextFile();
            }
            file.close();
            root.close();
          }
        }

        esp_task_wdt_reset();
        SerialSIT.println("AT+CFTPSLOGOUT");
        response = waitForResponse("+CFTPSLOGOUT: 0", 9000);
        debug("Response of AT+CFTPSLOGOUT is ");
        debugln(response);
        vTaskDelay(200);

        // Clean up FTP service internally so it frees the IP stack for HTTP
        SerialSIT.println("AT+CFTPSSTOP");
        response = waitForResponse("OK", 3000);
        debug("Response of AT+CFTPSSTOP is ");
        debugln(response);
        vTaskDelay(200);

        // NOTE: Modem FS file already deleted in the success cleanup path
        // above. A second AT+FSDEL here is redundant and always returns ERROR.
        // Suppressed to eliminate confusing noise in the log.

        SerialSIT.println("AT+FSLS");
        response = waitForResponse("OK", 10000);
      }
    } else {
      debugln("FTP Login unsucessful");
    }

  } else {
    debugln("NO FTP UNSENT FILES TO UPLOAD ");
  }
}

// Function to copy a file from a source path to a destination path
void copyFile(const char *sourcePath, const char *destPath) {
  debugf2("Copying file %s to %s\n", sourcePath, destPath);

  File sourceFile = SPIFFS.open(sourcePath, "r");
  if (!sourceFile) {
    debugln("Failed to open source file for reading");
    return;
  }

  File destFile = SPIFFS.open(destPath, "w");
  if (!destFile) {
    debugln("Failed to open destination file for writing");
    sourceFile.close();
    return;
  }

  byte buffer[512]; // A buffer to hold data during the copy
  size_t bytesRead;

  // Read from source and write to destination
  while ((bytesRead = sourceFile.read(buffer, sizeof(buffer))) > 0) {
    destFile.write(buffer, bytesRead);
  }

  sourceFile.close();
  destFile.close();
  debugln("File copied successfully!");
}

void graceful_modem_shutdown() {
  // session_unstable = true only on ACTUAL failures (reg fails > 0).
  // eHttpStop / eSMSStop mean the session COMPLETED successfully.
  // v5.67: Removed erroneous (sync_mode == eHttpStop) from this condition.
  bool session_unstable = (diag_consecutive_reg_fails > 0);

  if (!session_unstable) {
    debugln("[GPRS] Session clean. Attempting graceful shutdown...");
    SerialSIT.println("AT");
    if (waitForResponse("OK", 500).indexOf("OK") != -1) {
      debugln("[GPRS] Modem alive. Closing network session gracefully...");
      SerialSIT.println("AT+CPOWD=1"); // Normal Power Down
      waitForResponse("NORMAL POWER OFF", 3000);
      vTaskDelay(500);
    }
  } else {
    debugln(
        "[GPRS] ⚠️ Session was unstable (reg fails). Hard Hardware Reset.");
  }

  debugln("[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).");
  digitalWrite(26, LOW);
  gprs_started = false;
  gprs_mode = eGprsSleepMode; // Prevent Ghost Restart during sleep entry
}

void send_sms() {
  vTaskDelay(500); // TRG8-3.0.5g reduced from 1min to 500ms
  int msg_no;

  debugln();
  debugln("[GPRS] Checking SMS...");
  debugln();

  for (msg_no = 1; msg_no < 5; msg_no++) {
    process_sms(msg_no);
    vTaskDelay(100);
  }
  SerialSIT.println("AT+CMGD=1,4");
  response = waitForResponse("OK", 5000);
  debugln("Removed READ messages");

  sync_mode = eSMSStop;
}

int send_at_cmd_data(char *payload, String charArray) {
  unsigned long start = millis();
  esp_task_wdt_reset();

  // v5.42: Fast-fail if HTTP session was never successfully initialized.
  // Prevents burning ~47s on back-to-back HTTPDATA + HTTPACTION timeouts
  // when the modem's HTTP stack is in a dead/stuck state.
  if (!http_ready) {
    debugln("[HTTP] HTTP session not ready. Fast-fail to backlog.");
    return 0;
  }

  int i;
  for (i = 0; payload[i] != '\0'; ++i)
    ;
  snprintf(ht_data, sizeof(ht_data), "AT+HTTPDATA=%d,10000", i);

  debugf1("Payload is %s", payload);
  debugln();
  vTaskDelay(100);
  flushSerialSIT(); // v7.02: Clear buffer before data command
  SerialSIT.println(ht_data);
  response =
      waitForResponse("DOWNLOAD", 10000); // Increased timeout significantly
  if (response.indexOf("DOWNLOAD") == -1) {
    debugln("[HTTP] AT+HTTPDATA failed (Missing DOWNLOAD).");
    debugf1("[HTTP] Raw response: %s\n", response.c_str());
    return 0;
  }
  vTaskDelay(500);          // v7.00: Increased buffer prep delay
  SerialSIT.print(payload); // v6.40: Use print to avoid trailing \r\n in body
  response = waitForResponse("OK", 5000);

  SerialSIT.println("AT+HTTPACTION=1");
  waitForResponse("OK", 2000);
  response = waitForResponse("+HTTPACTION:",
                             45000); // v5.60: Increased to 45s for BSNL
  debug("Response of AT+HTTPACTION=1 is ");
  debugln(response);

  if (response.indexOf("200") == -1) {
    // Extract Error Code if not 200
    int comma1 = response.indexOf(',');
    int comma2 = response.indexOf(',', comma1 + 1);
    if (comma1 != -1 && comma2 != -1) {
      String errCode = response.substring(comma1 + 1, comma2);
      strncpy(diag_http_fail_reason, errCode.c_str(),
              sizeof(diag_http_fail_reason) - 1);
    } else {
      // v5.42: Blank response = +HTTPACTION: URC never arrived = timeout.
      // Label it so the retry path skips the bearer teardown (SAPBR=0,1).
      strncpy(diag_http_fail_reason, "TIMEOUT",
              sizeof(diag_http_fail_reason) - 1);
      debugln("[HTTP] HTTPACTION timed out — no URC received from modem.");
    }

    if (response.indexOf("706") != -1 || response.indexOf("713") != -1 ||
        response.indexOf("714") != -1) {
      debugln("HTTP Error (706/713/714). Clean stack requested.");
      SerialSIT.println("AT+HTTPTERM");
      waitForResponse("OK", 2000);
    }
    return 0;
  }

  // If we reach here, HTTPACTION returned 200
  // v6.35: A7672S needs time after action before read
  vTaskDelay(500);

  SerialSIT.println("AT+HTTPREAD=0,290");
  response = waitForResponse("+HTTPREAD: 0", 10000);
  debug("Response (P) of AT+HTTPREAD=0,290 is ");
  debugln(response);

  // Fallback: If parameterized read fails, try a RAW read
  if (response.indexOf("ERROR") != -1 || response.length() == 0) {
    debugln("[GPRS] Param-READ failed. Trying raw read...");
    SerialSIT.println("AT+HTTPREAD");
    response = waitForResponse("+HTTPREAD: 0", 10000);
    debug("Response (R) is ");
    debugln(response);
  }

  // v6.75: Advanced Response Parsing
  // SerialSIT says: OK\r\n\r\n+HTTPREAD: <size>\r\n<PAYLOAD>\r\nOK
  // We must find the payload start after '+HTTPREAD:'
  const char *payload_ptr = strstr(response.c_str(), "+HTTPREAD:");
  if (payload_ptr) {
    // Find the end of (+HTTPREAD: <size>) line
    payload_ptr = strchr(payload_ptr, '\n');
    if (payload_ptr)
      payload_ptr++; // Move to start of actual content
  } else {
    // If +HTTPREAD marker not found (e.g. raw read), check the whole response
    // but avoid the initial 'OK' if possible.
    payload_ptr = response.c_str();
  }

  bool success = false;
  // v6.75: Server status must be in the PAYLOAD, not the modem header
  if (strstr(payload_ptr, "success") || strstr(payload_ptr, "Success") ||
      strstr(payload_ptr, "ok") || strstr(payload_ptr, "OK") ||
      strstr(payload_ptr, "stored")) {
    success = true;
  }

  if (success) {
    debugln("GPRS SEND : It is a Success");
    diag_rejected_count = 0; // Reset on success

    diag_http_success_count++;
    diag_http_time_total += (millis() - start);

    // v5.88: Ensure HTTP session is closed EVEN on success
    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 3000);

    // v5.49 Update Golden Data sent mask (Auto-Routed)
    if (temp_sampleNo >= 0 && temp_sampleNo <= 95) {
      // Calculate current timeline index (0 = 08:45 AM)
      int h = (current_hour < 24) ? current_hour : 0;
      int m = (current_min < 60) ? current_min : 0;
      int timeline_idx = h * 4 + m / 15;
      timeline_idx = (timeline_idx + 61) % 96;

      // Logic: If the sampled record's index is "Higher" than our current
      // clock, it must belong to 'Yesterday' (Before the 08:45 AM reset).
      // Exception: If we just reset (timeline_idx ~0), everything is 'Current'.
      if (temp_sampleNo <= timeline_idx || timeline_idx < 5) {
        diag_sent_mask_cur[temp_sampleNo / 32] |= (1UL << (temp_sampleNo % 32));
      } else {
        diag_sent_mask_prev[temp_sampleNo / 32] |=
            (1UL << (temp_sampleNo % 32));
      }
    }

    // v5.55: Use RTC_DATA_ATTR guard so SPIFFS write survives deep sleep.
    // 'static' local resets on every wakeup, causing a write every boot.
    if (!apn_saved_this_sim && String(cached_iccid) != "" &&
        String(apn_str) != "") {
      save_apn_config(apn_str, cached_iccid);
      apn_saved_this_sim = true;
    }

    strcpy(diag_http_fail_reason, "NONE");
    return 1;
  } else {
    debugln("GPRS SEND : It is a Failure");

    // v6.85: Trigger RTC resync if rejected >= 3 times (Time mismatch)
    if (strstr(payload_ptr, "Rejected") || strstr(payload_ptr, "rejected")) {
      diag_rejected_count++;
      debugf1("[TIME] Server Rejected Data. Count: %d\n", diag_rejected_count);

      if (diag_rejected_count > 2) {
        debugln("[TIME] Multiple rejections. Forcing RTC resync via GPRS...");
        resync_time();
        diag_rejected_count = 0; // Reset after attempt
      }
    }

    strcpy(diag_http_fail_reason, "SRV_ERR");
    // v5.88: Ensure HTTP session is closed on Failure too
    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 3000);
    return 0;
  }
}

void store_current_unsent_data() {
  vTaskDelay(100);
  char finalStringBuffer[100];
  strcpy(finalStringBuffer, store_text); // strcpy handles null termination

#if SYSTEM == 0
  snprintf(unsent_file, sizeof(unsent_file), "/unsent.txt");

  // SAFETY: If the file is getting too large (>150KB), SPIFFS appends can get
  // slow or buggy. 150KB = ~2500 records. If we haven't sent by then, something
  // is very wrong.
  if (SPIFFS.exists(unsent_file)) {
    File f_check = SPIFFS.open(unsent_file, FILE_READ);
    if (f_check.size() > 150000) {
      f_check.close();
      debugln("[SPIFFS] unsent.txt exceeds 150KB. Truncating to prevent "
              "filesystem jam.");
      SPIFFS.remove(unsent_file);
    } else {
      f_check.close();
    }
  }

  File file2 = SPIFFS.open(unsent_file, FILE_APPEND);
  if (file2) {
    file2.print(finalStringBuffer);
    file2.close();
  } else {
    debugln("Failed to open unsent.txt for appending (store_current)");
  }
  debug("************************");
  debug("Storing data in unsent file due to SIM issue/REG issue/Signal "
        "issue ");
  debugln("************************");
  debug("unsent store_text is ");
  debugln(finalStringBuffer);
#endif

#if (SYSTEM == 1 || SYSTEM == 2)
  snprintf(ftpunsent_file, sizeof(ftpunsent_file), "/ftpunsent.txt");

  // SAFETY: Hard limit on backlog file size
  if (SPIFFS.exists(ftpunsent_file)) {
    File f_check = SPIFFS.open(ftpunsent_file, FILE_READ);
    if (f_check.size() > 150000) {
      f_check.close();
      debugln("[SPIFFS] ftpunsent.txt exceeds 150KB. Truncating.");
      SPIFFS.remove(ftpunsent_file);
    } else {
      f_check.close();
    }
  }

  File ftpfile2 = SPIFFS.open(ftpunsent_file, FILE_APPEND);
  if (ftpfile2) {
    ftpfile2.print(finalStringBuffer);
    ftpfile2.close();
  } else {
    debugln("Failed to open ftpunsent.txt for appending (store_current)");
  }
  debug("************************");
  debug("Storing data in FTP unsent file due to SIM issue/REG issue/Signal "
        "issue ");
  debugln("************************");
  debug("ftpunsent is ");
  debugln(finalStringBuffer);
#endif

  debugln();
  debug("Current/Truncated data written store_text->finalStringBuffer to "
        "unsent file is ");
  debugln(finalStringBuffer);

#if DEBUG == 1
#if SYSTEM == 0
  snprintf(unsent_file, sizeof(unsent_file), "/unsent.txt");
  if (SPIFFS.exists(unsent_file)) {
    debug("The FILE content of UNSENT file ");
    debugln(unsent_file);
    File file4 = SPIFFS.open(
        unsent_file,
        FILE_READ); // Open for reading and appending (writing at end of
                    // file). The file is created if it does not exist.
    content = file4.readString();
    debugln(content);
    file4.close();
    debugln();
  }
#endif

#if (SYSTEM == 1 || SYSTEM == 2)
  snprintf(ftpunsent_file, sizeof(ftpunsent_file), "/ftpunsent.txt");
  if (SPIFFS.exists(ftpunsent_file)) {
    debug("The FILE content of FTP UNSENT file ");
    debugln(ftpunsent_file);
    File ftpfile4 = SPIFFS.open(
        ftpunsent_file,
        FILE_READ); // Open for reading and appending (writing at end of
                    // file). The file is created if it does not exist.
    content = ftpfile4.readString();
    debugln(content);
    ftpfile4.close();
    debugln();
  }
#endif

#endif

  sync_mode = eHttpStop;
  vTaskDelay(300);
}

/*
 *  SIM Network Registration and Setup
 *  - get_signal_strength();//2024 iter3
 *  - get_network();
 *  - get_registration();
 *  - get_a7672s();
 */

void get_signal_strength() {

  debugln("************************");
  debugln("GETTING SIGNAL STRENGTH ");
  debugln("************************");
  debugln();

  signal_lvl = 0;
  retries = 0;

  // [v5.61] Pre-settle delay: modem returns 85 (not ready) on first 1-3 polls
  // immediately after boot. A single 400ms wait eliminates redundant retries.
  vTaskDelay(400 / portTICK_PERIOD_MS);

  int invalid_signal_count = 0;
  // rssi 0 = -113dBm. Continuous -113 is essentially no signal.
  while (((signal_lvl == 0) || (signal_lvl <= -113)) &&
         (retries < 120)) { // 120 loops @ 500ms = 60s max timeout
    esp_task_wdt_reset();
    SerialSIT.println("AT+CSQ");
    response = waitForResponse("+CSQ ", 1000);
    rssiIndex = response.indexOf("+CSQ: "); // int
    if (rssiIndex != -1) {                  // #TRUEFIX
      rssiIndex += 6;
      rssiEndIndex = response.indexOf(",", rssiIndex);         // int
      if (rssiEndIndex != -1) {                                // #TRUEFIX
        rssiStr = response.substring(rssiIndex, rssiEndIndex); // String
      } else {
        rssiStr = "0"; // Default
      }
    } else {
      rssiStr = "0"; // Default
    }
    resp = rssiStr.c_str();
    signal_strength =
        (-113 + 2 * (atoi(resp))); // convert char to int usng atoi()
    debug("Signal strength IN gprs task is ");
    debugln(signal_strength);
    if (signal_strength != 85) {
      signal_lvl = signal_strength;
      debug("Signal Level is ");
      debugln(signal_lvl);
    } else {
      invalid_signal_count++;
      // If we see "No Signal" (-113 or 85) for 30s (60 polls), move to
      // registration. USER REQUEST: Increased to 30s for better antenna
      // recovery.
      if (invalid_signal_count > 60) {
        debugln("[GPRS] Signal search timeout (30s). Moving to Registration.");
        break;
      }
    }
    retries++;
    vTaskDelay(500 / portTICK_PERIOD_MS); // High-frequency polling
  }
  // CLAMP: Modem returns 85 for "No Signal". Convert to -111 sentinel.
  if (signal_strength == 85 || signal_strength > 31) {
    signal_strength = -111;
    signal_lvl = -111;
  }
}

/**
 * Attempts to fetch SIM number via USSD as a secondary fallback
 */
// USSD Discovery Removed - Inefficient for release

void get_network() {
  debugln();
  debugln("************************");
  debugln("GETTING NETWORK ");
  debugln("************************");

  extern String get_ccid();
  String current_iccid = get_ccid();

  // SMART CACHE LOGIC: Skip querying SIM info if we already have it in RTC
  // AND it matches the physical SIM in the slot.
  if (current_iccid != "" && String(cached_iccid) == current_iccid &&
      String(sim_number) != "NA") {
    debugln("[CACHE] Using cached carrier/number to save power.");
    // APN still needs to be determined for the modem to connect
    if (strstr(carrier, "Airtel"))
      strcpy(apn_str, "airteliot.com");
    else if (strstr(carrier, "Jio"))
      strcpy(apn_str, "jionet");
    else if (strstr(carrier, "BSNL"))
      strcpy(apn_str, "bsnlnet");
    else if (strstr(carrier, "Vi"))
      strcpy(apn_str, "www");
    else
      strcpy(apn_str, "airteliot.com");

    return; // SKIP the rest of discovery
  }

full_discovery:
  // Not cached or SIM changed: Reset and perform full discovery
  debugln("[CACHE] New SIM or No Cache. Performing full discovery...");
  strncpy(cached_iccid, current_iccid.c_str(), sizeof(cached_iccid) - 1);
  cached_iccid[sizeof(cached_iccid) - 1] = '\0';
  strcpy(sim_number, "NA");
  strcpy(carrier, "NA");
  apn_saved_this_sim = false; // v5.55: New SIM — force APN re-save to SPIFFS

  // 1. Try CSPN (Provider Name)
  SerialSIT.println("AT+CSPN?");
  String cspnResp = waitForResponse("OK", 2000);
  debug("CSPN Logic response: ");
  debugln(cspnResp);

  // 2. Try COPS (Operator) as fallback
  SerialSIT.println("AT+COPS?");
  String copsResp = waitForResponse("OK", 2000);
  debug("COPS Logic response: ");
  debugln(copsResp);

  cspnResp.toLowerCase();
  copsResp.toLowerCase();
  const char *r1 = cspnResp.c_str();
  const char *r2 = copsResp.c_str();

  // Determine Carrier and APN
  if (strstr(r1, "airtel") || strstr(r2, "airtel")) {
    strcpy(carrier, "Airtel");
    strcpy(apn_str, "airteliot.com");
  } else if (strstr(r1, "jio") || strstr(r2, "jio")) {
    strcpy(carrier, "Jio");
    strcpy(apn_str, "jionet");
  } else if (strstr(r1, "bsnl") || strstr(r2, "bsnl")) {
    strcpy(carrier, "BSNL");
    strcpy(apn_str, "bsnlnet");
  } else if (strstr(r1, "idea") || strstr(r1, "vi") || strstr(r2, "idea") ||
             strstr(r2, "vi")) {
    strcpy(carrier, "Vi");
    strcpy(apn_str, "www");
  } else {
    strcpy(carrier, "SIM OK");
    strcpy(apn_str, "airteliot.com");
  }

  // Get SIM identifier via IMSI (fast, always works on IoT/BSNL SIMs)
  // CNUM and USSD are skipped - they consistently fail on IoT/BSNL SIMs
  // and waste ~30 seconds. IMSI is sufficient as a unique identifier.
  SerialSIT.println("AT+CIMI");
  String imsiResp = waitForResponse("OK", 2000);

  // Extract IMSI (15 digits)
  int imsiStart = -1;
  for (int i = 0; i < imsiResp.length(); i++) {
    if (imsiResp.charAt(i) >= '0' && imsiResp.charAt(i) <= '9') {
      imsiStart = i;
      break;
    }
  }

  if (imsiStart != -1) {
    String imsi = "";
    for (int i = imsiStart; i < imsiResp.length() && imsi.length() < 15; i++) {
      char c = imsiResp.charAt(i);
      if (c >= '0' && c <= '9') {
        imsi += c;
      } else if (imsi.length() > 0) {
        break;
      }
    }
    if (imsi.length() >= 10) {
      debug("IMSI: ");
      debugln(imsi);
      strcpy(sim_number, imsi.c_str());
    } else {
      strcpy(sim_number, "NA");
    }
  } else {
    strcpy(sim_number, "NA");
  }
  debug("Service Provider APN is ");
  debugln(apn_str);
  debug("Carrier: ");
  debugln(carrier);
  debug("Number: ");
  debugln(sim_number);
  debugln();
  if (!strcmp(apn_str, "bsnlnet")) {
    debugln("BSNL network found..");
  }
}

void get_registration() {
  esp_task_wdt_reset();
  debugln();
  debugln("************************");
  debugln("GETTING REGISTRATION ");
  debugln("************************");

  // v5.62 HYBRID ADAPTIVE REGISTRATION:
  // - Airtel/Jio: 12 retries (~60s max) — they register in 1-3 iters anyway
  // - BSNL: 24 retries (~120s max) — BSNL 2G towers need more time
  // - Adaptive wait replaces blind vTaskDelay(5000): poll CGREG every 1s
  //   and exit immediately on success. No wasted sleep.
  bool isBSNL = (strstr(carrier, "BSNL") != nullptr);
  int no_of_retries = isBSNL ? 50 : 50;
  registration = 0;
  retries = 0;
  bool is_registered = false;

  // v5.60 SURE-SHOT REGISTRATION:
  SerialSIT.println("AT+CFUN=1"); // Full functionality
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CGDCONT=8,\"IP\",\"\""); // Kill side-channel profile
  waitForResponse("OK", 1000);

  // v5.60: SET APN EARLY FOR CID 1 (Helps BSNL GPRS attachment)
  if (strlen(apn_str) > 3) {
    debugln("[GPRS] Pre-setting APN for CID 1: " + String(apn_str));
    SerialSIT.print("AT+CGDCONT=1,\"IP\",\"");
    SerialSIT.print(apn_str);
    SerialSIT.println("\"");
    waitForResponse("OK", 1000);
  }

  SerialSIT.println("AT+CNMP=2"); // Automatic Mode (LTE/GSM)
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CMNB=3"); // LTE then GSM
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CGATT=1"); // Force GPRS Attachment
  waitForResponse("OK", 5000);
  SerialSIT.println("AT+CREG=1");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CNETLIGHT=0"); // Reset LED driver
  waitForResponse("OK", 500);
  SerialSIT.println("AT+CNETLIGHT=1"); // Ensure LED blinks
  waitForResponse("OK", 1000);
  vTaskDelay(1000 / portTICK_PERIOD_MS); // Let it settle

  // [v5.61] DIAG IMEI/COPS queries removed from normal wake cycle.
  // These were firing AT+GSN and AT+COPS? each boot but always returned empty
  // strings, wasting ~1.2s of active time. Use AT+GSN once via CLI/SMS if
  // needed.

  int consecutive_unreg = 0;
  while (!is_registered && (retries < no_of_retries)) {
    esp_task_wdt_reset();

    // v5.62 BSNL STUBBORN TRIGGER:
    // If BSNL and stuck in Status 0 (Idle) for 15s, force a re-scan.
    if (isBSNL && registration == 0 && retries == 3) {
      debugln(
          "[GPRS] BSNL Stubborn Idle. Forcing Frequency Re-scan (COPS=0)...");
      SerialSIT.println("AT+COPS=0");
      waitForResponse("OK", 5000);
    }

    // UI Feedback: Show progress on LCD
    snprintf(reg_status, 16, "TRY #%d...", retries + 1);

    // 1. Check CEREG (LTE)
    flushSerialSIT();
    SerialSIT.println("AT+CEREG?");
    response = waitForResponse("OK", 5000);
    int tagIdx = response.indexOf("+CEREG:");
    if (tagIdx != -1) {
      int commaIdx = response.indexOf(',', tagIdx);
      if (commaIdx != -1) {
        registration = response.substring(commaIdx + 1).toInt();
        if (registration == 1 || registration == 5) {
          debugln("CEREG Registered");
          isLTE = true;
          strcpy(reg_status,
                 (registration == 1) ? "LTE:Home:OK" : "LTE:Roam:OK");
          is_registered = true;
          break;
        }
      }
    }

    if (!is_registered) {
      // 2. Check CGREG (GPRS)
      flushSerialSIT();
      SerialSIT.println("AT+CGREG?");
      response = waitForResponse("OK", 5000);
      tagIdx = response.indexOf("+CGREG:");
      if (tagIdx != -1) {
        int commaIdx = response.indexOf(',', tagIdx);
        if (commaIdx != -1) {
          registration = response.substring(commaIdx + 1).toInt();
          if (registration == 1 || registration == 5) {
            debugln("CGREG Registered");
            isLTE = true;
            strcpy(reg_status,
                   (registration == 1) ? "GPRS:Home:OK" : "GPRS:Roam:OK");
            is_registered = true;
            break;
          }
        }
      }
    }

    if (!is_registered) {
      // 3. Check CREG (GSM)
      flushSerialSIT();
      SerialSIT.println("AT+CREG?");
      response = waitForResponse("OK", 5000);
      debug("Raw CREG: ");
      debugln(response);
      tagIdx = response.indexOf("+CREG:");
      if (tagIdx != -1) {
        int commaIdx = response.indexOf(',', tagIdx);
        if (commaIdx != -1) {
          registration = response.substring(commaIdx + 1).toInt();
          if (registration == 0 || registration == 3 || registration == 4) {
            consecutive_unreg++;
          } else {
            consecutive_unreg = 0;
          }
        }

        if (consecutive_unreg >= 45) {
          debugln("[GPRS] Persistent Unreg status. Final fail.");
          break;
        }

        // v5.60: EARLY KICK if stuck in 'Not Searching' state (0)
        if (registration == 0 && retries == 2) {
          debugln("[GPRS] Stuck in Idle. Early search kick (COPS=0)...");
          SerialSIT.println("AT+COPS=0");
          waitForResponse("OK", 5000);
        }

        if (commaIdx != -1 &&
            (registration == 1 || registration == 5 || registration == 11)) {
          debugln("CREG Registered (GSM)");
          isLTE = false;
          strcpy(reg_status,
                 (registration == 1)
                     ? "GSM:Home:OK"
                     : (registration == 11 ? "GSM:LTE:EXT" : "GSM:Roam:OK"));
          is_registered = true;
          break;
        }
      }
    }

    if (!is_registered) {
      // Periodic Recovery Logic - Strictly Tiered to avoid interrupting search
      if (retries > 0 && retries % 5 == 0) {
        if (registration == 0 || registration == 3 || registration == 4) {
          if (retries == 5) {
            // Tier 1: TRUE HARDWARE COLD START (Prevents Back-powering)
            debugln("[GPRS] Stubborn Denied/Idle. FORCING TRUE COLD START...");

            // 1. Shudown Serial to prevent back-powering via Tx/Rx
            SerialSIT.end();
            pinMode(16, INPUT);
            pinMode(17, INPUT);

            // 2. Physical Power Cut
            digitalWrite(26, LOW);
            vTaskDelay(2500);

            // 3. Restore Power
            digitalWrite(26, HIGH);
            vTaskDelay(1000);

            // 4. Re-init Serial
            SerialSIT.begin(115200, SERIAL_8N1, 16, 17, false);
            vTaskDelay(2000);

            SerialSIT.println("AT");
            waitForResponse("OK", 2000);
            SerialSIT.println("ATE0");
            waitForResponse("OK", 1000);
            SerialSIT.println("AT+CNETLIGHT=1");
            waitForResponse("OK", 500);
            SerialSIT.println("AT+CREG=1");
            waitForResponse("OK", 1000);
            SerialSIT.println("AT+COPS=0");
            waitForResponse("OK", 5000);
          } else if (retries == 15) {
            // Tier 2: Forced LTE-Only Mode (Airtel Fix)
            debugln("[GPRS] Forcing LTE-Only Mode (AT+CNMP=38)...");
            SerialSIT.println("AT+CNMP=38"); // Force LTE
            waitForResponse("OK", 2000);
            SerialSIT.println("AT+COPS=0");
            waitForResponse("OK", 5000);
          } else if (retries == 25) {
            // Tier 3: Software Radio Kick (Nudge)
            debugln("[GPRS] Still searching. Forced Radio kick (COPS=2,0)...");
            SerialSIT.println("AT+COPS=2");
            waitForResponse("OK", 5000);
            SerialSIT.println("AT+COPS=0");
            waitForResponse("OK", 10000);
          } else if (retries == 40) {
            // Tier 4: Software Reboot (Last Software Resort)
            debugln("[GPRS] Stubborn Search. Trying Software Reboot "
                    "(CFUN=1,1)...");
            SerialSIT.println("AT+CFUN=1,1");
            waitForResponse("OK", 15000);
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            SerialSIT.println("ATE0");
            waitForResponse("OK", 1000);
            SerialSIT.println("AT+CREG=1");
            waitForResponse("OK", 1000);
            SerialSIT.println("AT+COPS=0");
            waitForResponse("OK", 5000);
          }
        }
      }

      // v5.62 CONTINUOUS PDP POKE:
      // Pokes the BSNL tower to open a data channel.
      // This often converts a CREG=1 (GSM) to a CGREG=1 (GPRS).
      SerialSIT.println("AT+CGACT=1,1");
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    debugf("Reg Search [BSNL:%d]... Status:%d Iter:#%d/%d\n", isBSNL,
           registration, retries + 1, no_of_retries);

    // v5.62 ADAPTIVE WAIT: Poll CGREG every 1s for up to 5s.
    // Exits immediately on registration — no wasted blind sleep.
    for (int w = 0; w < 5 && !is_registered; w++) {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      esp_task_wdt_reset();
      flushSerialSIT();
      SerialSIT.println("AT+CGREG?");
      String qr = waitForResponse("OK", 2000);
      int qi = qr.indexOf("+CGREG:");
      if (qi != -1) {
        int qc = qr.indexOf(',', qi);
        if (qc != -1) {
          int qreg = qr.substring(qc + 1).toInt();
          if (qreg == 1 || qreg == 5) {
            debugln("[GPRS] CGREG registered during adaptive wait!");
            isLTE = true;
            strcpy(reg_status, (qreg == 1) ? "GPRS:Home:OK" : "GPRS:Roam:OK");
            is_registered = true;
          }
        }
      }
    }
    retries++;
  }

  // Update Diagnostics
  int time_taken = retries * 5;
  if (is_registered) {
    diag_reg_time_total += time_taken;
    diag_reg_count++;
    diag_consecutive_reg_fails = 0;
    strcpy(diag_reg_fail_type, "NONE");
    if (time_taken > diag_reg_worst)
      diag_reg_worst = time_taken;
    gprs_mode = eGprsSignalOk;
    debugln("Registration Successful.");
  } else {
    diag_gprs_fails++;
    diag_consecutive_reg_fails++;
    gprs_mode = eGprsSignalForStoringOnly;
    debugf1("Registration failed. Consecutive fails: %d/10\n",
            diag_consecutive_reg_fails);

    if (diag_consecutive_reg_fails >= 10) {
      debugln("[GPRS] PERSISTENT REG FAIL. Resetting system...");
      vTaskDelay(1000);
      ESP.restart();
    }
  }
}

void get_a7672s() {
  debugln("--- GPRS SETTING PDP ---");
  //      SerialSIT.println("AT+CPSI?");
  //      response = waitForResponse("+CPSI", 5000);
  //      debug("HTTP response of AT+CPSI?: ");debugln(response);

  debugln();
  debugln("************************");
  debugln("Setting PDP context ");
  debugln("************************");
  debugln();

  char gprs_xmit_buf[300];

  // Check active context FIRST
  SerialSIT.println("AT+CGACT?");
  response = waitForResponse("OK", 3000);
  debugln("CGACT Status: " + response);

  active_cid = 0;
  // Prioritize 1 (Default) if active, then 5, 8.
  if (response.indexOf("+CGACT: 1,1") != -1) {
    // v5.52: Verify that the active context's APN matches our target SIM
    SerialSIT.println("AT+CGCONTRDP=1");
    String rdp_resp = waitForResponse("OK", 3000);
    bool apn_match = false;
    if (strlen(apn_str) == 0) {
      // If we don't know the target APN yet, assume the active one is OK.
      // Also extract the active APN from the response so
      // verify_bearer_or_recover has something to compare against on subsequent
      // retries.
      apn_match = true;
      // Parse APN from +CGCONTRDP: 1,5,"<apn>","ip",...
      int q1 = rdp_resp.indexOf('"');
      if (q1 != -1) {
        int q2 = rdp_resp.indexOf('"', q1 + 1);
        if (q2 != -1) {
          String active_apn = rdp_resp.substring(q1 + 1, q2);
          if (active_apn.length() > 0 && active_apn.length() < 20) {
            strncpy(apn_str, active_apn.c_str(), sizeof(apn_str) - 1);
            debugln("[GPRS] Auto-detected active APN: " + active_apn);
          }
        }
      }
    } else if (rdp_resp.indexOf("\"" + String(apn_str) + "\"") != -1) {
      apn_match = true;
    }

    if (apn_match) {
      active_cid = 1;
      debugln("Context 1 is active and APN matches. Using CID 1.");
    } else {
      debugln("[GPRS] Context 1 APN mismatch (Airtel auto-bearer or ghost). "
              "Clearing and reactivating with stored APN...");
      SerialSIT.println("AT+CGACT=0,1");
      waitForResponse("OK", 5000);
      active_cid = 0; // Force re-activation
    }
  } else if (response.indexOf("+CGACT: 5,1") != -1) {
    active_cid = 5;
    debugln("Context 5 is active. Using CID 5.");
  } else if (response.indexOf("+CGACT: 8,1") != -1) {
    active_cid = 8;
    debugln("Context 8 is active. Using CID 8.");
  }

  if (active_cid != 0) {
    // Log the IP address of the active context
    SerialSIT.print("AT+CGCONTRDP=");
    SerialSIT.println(active_cid);
    response = waitForResponse("OK", 3000);
    debug("IP Config: ");
    debugln(response);
  } else {
    // No context active or ghost context cleared. Configure CID 1.
    active_cid = 1;

    // JUDICIOUS STABILIZATION: Allow modem stack to settle
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    extern String get_ccid();
    String ccid = String(cached_iccid);
    if (ccid == "") {
      ccid = get_ccid();
    }
    char stored_apn[20] = {0}; // Match global apn_str size

    // 1. Try Stored APN
    if (load_apn_config(ccid, stored_apn, sizeof(stored_apn))) {
      debugln("Smart APN: Trying Stored -> " + String(stored_apn));
      if (try_activate_apn(stored_apn)) {
        strncpy(apn_str, stored_apn, sizeof(apn_str) - 1);
        diag_stored_apn_fails = 0; // Reset counter on success
        return;                    // Success!
      } else {
        diag_stored_apn_fails++;
        debugf1("Smart APN: Stored APN failed. Fail count: %d\n",
                diag_stored_apn_fails);

        // If it has failed 2+ times across wake cycles, the network data layer
        // is down. Stop hunting to save power.
        if (diag_stored_apn_fails >= 2) {
          debugln("Smart APN: Tower is likely rejecting data calls. Halting "
                  "search to save power.");
          gprs_mode = eGprsSignalForStoringOnly;
          return;
        }
      }
    }

    // 2. Default Sequence (fallback or first run)
    debugln("APN Search: Starting Default Sequence...");

    // Priority 1: The SIM's primary detected APN (from get_network)
    // This covers BSNL (bsnlnet), Jio (jionet), and others.
    if (try_activate_apn(apn_str)) {
      save_apn_config(apn_str, ccid);
      return;
    }

    // Fallbacks (mostly for Airtel logic)
    // Only try these if the primary failed and it's an Airtel card or generic
    // Priority 1: airteliot.com
    if (try_activate_apn("airteliot.com")) {
      strcpy(apn_str, "airteliot.com");
      save_apn_config("airteliot.com", ccid);
      return;
    }

    // Priority 2: iot.com
    if (try_activate_apn("iot.com")) {
      strcpy(apn_str, "iot.com");
      save_apn_config("iot.com", ccid);
      return;
    }

    // Priority 3: airtelgprs.com
    if (try_activate_apn("airtelgprs.com")) {
      strcpy(apn_str, "airtelgprs.com");
      save_apn_config("airtelgprs.com", ccid);
      return;
    }

    // 3. Last Resort: GPRS Stack Reset (CGATT Reset)
    debugln("APN: Initial attempts failed. Performing Soft Reset (CGATT)...");
    SerialSIT.println("AT+CGATT=0"); // Detach
    waitForResponse("OK", 5000);
    vTaskDelay(1000);
    SerialSIT.println("AT+CGATT=1"); // Re-attach
    waitForResponse("OK", 5000);
    vTaskDelay(2000);

    // Final Retry of the SIM's primary APN
    if (try_activate_apn(apn_str)) {
      save_apn_config(apn_str, ccid);
      return;
    }

    debugln("APN: All attempts failed even after reset.");
    gprs_mode = eGprsSignalForStoringOnly;
  }
}

/*
 *   SMS & FTP
 * - process_sms()
 * - prepare_and_send_status()
 * - get_lat_long_date_time()
 * - setup_ftp()
 * - fetchFromFtpAndUpdate()
 * - copyFromSPIFFSToFS()
 *
 */

void process_sms(char msg_no) {
  char *csqstr, c, *ptr, msg_rcvd_number[100];
  int i, response_no;

  // new
  char *msg_body, msg_recd_no[20], temp[20], temp1[20], temp2[40];
  char gprs_xmit_buf[300];

  int cmd_no, cmd_no_loop;
  int offset_cnt;
  offset_cnt = 0;

  SerialSIT.println("AT+CMGF=1");
  response = waitForResponse("OK", 10000);
  // debug("Response of AT+CMGF=1");
  // debugln(response);

  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CMGR=%d", msg_no);
  SerialSIT.println(gprs_xmit_buf);
  response = waitForResponse("OK", 6000);
  // debug("Response of AT+CMGR is ");
  // debugln(response);

  const char *response_char = response.c_str();

  if (strstr(response_char, "REC UNREAD") && strstr(response_char, "VARSHA")) {
    debug("SMS: Valid command (VARSHA) received: ");
    debugln(response);
    // Extract the sender's number regardless of country code or length
    // The format is: +CMGR: "STATUS","NUMBER","","TIME"
    char *firstQuote = strchr(response_char, ',');
    if (firstQuote != NULL) {
      char *startQuote = strchr(firstQuote, '"');
      if (startQuote != NULL) {
        char *endQuote = strchr(startQuote + 1, '"');
        if (endQuote != NULL) {
          int numLen = endQuote - (startQuote + 1);
          if (numLen > 0 && numLen < 20) {
            strncpy(msg_rcvd_number, startQuote + 1, numLen);
            msg_rcvd_number[numLen] = '\0';
            debug("Extracted Number: ");
            debugln(msg_rcvd_number);
          }
        }
      }
    }

    if (strlen(msg_rcvd_number) == 0) {
      debugln("Error: Could not extract number from SMS response");
      return;
    }

    vTaskDelay(100);

    // new
    msg_body = strstr(response_char, "VARSHA");
    offset_cnt = read_line(msg_body, temp, 0x20); // 'VARSHA' is removed
                                                  // offset_cnt at end of line
    msg_body += offset_cnt;

    offset_cnt = read_line(msg_body, temp, 0x20); // command is read into temp
    msg_body += offset_cnt;

    if (strstr(response_char, "GET_STATUS")) {
      prepare_and_send_status(msg_rcvd_number);
      vTaskDelay(1000);
    } else if (strstr(response_char, "GET_GPS")) {
      get_lat_long_date_time(msg_rcvd_number); // WORKING UNCOMMENT IT
      vTaskDelay(1000);
    } else if (strstr(response_char, "SEND_FTP_DAILY_DATA")) {
      /*
       * dateStPtr = strstr(gprs_response, "DATE=");
       * sscanf(dateStPtr, "DATE=%s", &datevale);
       */
      send_daily = 1;
      char *dtPtr;
      int y1, m1, d1;
      dtPtr = strstr(response_char, "DATE");
      sscanf(dtPtr, "DATE=%04d-%02d-%02d", &y1, &m1,
             &d1); // +CCLK: \"23/08/01,09:54:35+00\"
      char requestedDate[20];
      snprintf(requestedDate, sizeof(requestedDate), "%04d%02d%02d", y1, m1,
               d1);
      debug(" Date is ");
      debugln(requestedDate);
      copyFromSPIFFSToFS(requestedDate); // temp1 is date

    } else if (strstr(response_char, "SET_FW_FNAME")) {
      // VARSHA SET_FW_FOR_LPC FILE=firmware.bin
      char *filePtr;
      filePtr = strstr(response_char, "FILE");
      sscanf(filePtr, "FILE=%19s", &temp1); // +CCLK: \"23/08/01,09:54:35+00\"
      debug("Firmware file is ");
      debugln(temp1);
      fetchFromHttpAndUpdate(temp1);
    }

  } else {

    // debug("No message recd ");
    // debugln(response);
  }
  vTaskDelay(100);
}

void prepare_and_send_status(char *gsm_no) {
  int response_no;
  char msg_type[9];
  char status_response[256];
  char gprs_xmit_buf[300];
  msg_sent = 0;
  if (!wifi_active) {
    int solar_raw;
    if (adc2_get_raw(ADC2_CHANNEL_8, ADC_WIDTH_BIT_12, &solar_raw) == ESP_OK) {
      solar = solar_raw;
    }
  }
  solar_val = (solar / 4096.0) * 3.6 * 7.2;

  if (solar_val > 4)
    solar_conn = 1;
  else
    solar_conn = 0;

  li_bat = adc1_get_raw(ADC1_CHANNEL_5); // Changed from analogRead(33)
  li_bat_val = li_bat * 0.0010915;       //(3.3/4096) * (840/620);
  snprintf(battery, sizeof(battery), "%04.1f", li_bat_val);
  bat_val = li_bat_val; // bat_val is given for storage in spiffs

  if (firmwareUpdated == 1) {
    strcpy(msg_type, "FW-UPD");
    firmwareUpdated = 0;
  } else if (send_status == 1) { // SMS trigger from LCD SEND_STATUS
    send_status = 0;
    strcpy(msg_type, "STATUS-F");
  } else {
    strcpy(msg_type, "STATUS-C");
  }

  //                  debug("NETWORK is ");debugln(NETWORK);//TRG8-3.0.5
  //        }

  // Trim Station ID for cleaner SMS
  char cleanStn[16];
  strncpy(cleanStn, station_name, 15);
  cleanStn[15] = '\0';
  int tLen = strlen(cleanStn);
  while (tLen > 0 && cleanStn[tLen - 1] == ' ') {
    cleanStn[tLen - 1] = '\0';
    tLen--;
  }

  // RF & TWS
  snprintf(
      status_response, sizeof(status_response),
      "%s,%s,%s,%s,%04d-%02d-%02dT%02d:%02d,%s,15,%04d,%04.1f,0.0,%s,%s,%s,"
      "%04d-%02d-%02d,%04d-%02d-%02dT%02d:%02d,0\r\n\032",
      NETWORK, STATION_TYPE, msg_type, cleanStn, current_year, current_month,
      current_day, current_hour, current_min, UNIT_VER, signal_strength,
      bat_val, (solar_conn ? "Y" : "N"), (sd_card_ok ? "SDC-OK" : "SDC-FAIL"),
      (calib_sts == 1 ? "CLB-OK" : "CLB-FAIL"),
      calib_year,  // calib
      calib_month, // calib
      calib_day,   // calib
      last_recorded_yy, last_recorded_mm, last_recorded_dd, last_recorded_hr,
      last_recorded_min);

  debug("Status response prepared is ");
  debugln(status_response);
  // first give send sms command

  SerialSIT.println("AT+CMGF=1");
  waitForResponse("OK", 1000);

  // Optional but helpful for BSNL: Check Service Center
  SerialSIT.println("AT+CSCA?");
  waitForResponse("OK", 500);

  debug("Mobile number to be sent to is : ");
  debugln(gsm_no);

  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CMGS=\"%s\"\r", gsm_no);
  SerialSIT.println(gprs_xmit_buf);
  debug("Waiting for '>' prompt...");
  response = waitForResponse(">", 15000);
  if (response.indexOf(">") != -1) {
    debugln(" Received!");
    SerialSIT.print(status_response); //  SerialSIT.write(26);
    debug("Waiting for +CMGS confirmation...");
    response = waitForResponse("+CMGS:", 35000); // 35s for BSNL 2G
    debugln(" Done.");
    debug("Response of AT+CMGS is ");
    debugln(response);
  } else {
    debugln(" TIMEOUT! No '>' prompt received.");
    debug("Response was: ");
    debugln(response);
  }

  const char *char_resp = response.c_str();
  if (strstr(char_resp, "+CMGS")) {
    msg_sent = 1;
    debugln("SUCCESS IN SENDING MSG");
  } else {
    debugln("FAILED TO SEND MSG - No +CMGS in response");
  }
}

void get_gps_coordinates() {
  int tmp;
  float lat, lon;
  const char *response_char;
  char *csqstr;

  for (int retry = 0; retry < 2; retry++) {
    debugf1("[GPS] Requesting Coordinates (AT+CLBS=1), Attempt %d...\n",
            retry + 1);
    SerialSIT.println("ATE0");
    waitForResponse("OK", 2000);

    SerialSIT.println("AT+CLBS=1");
    String response = waitForResponse("+CLBS:", 15000); // 15s timeout

    response_char = response.c_str();
    csqstr = strstr(response_char, "+CLBS");
    if (csqstr != NULL) {
      // Response format: +CLBS: 0,12.989436,77.537910,550
      // We use %f to capture standard floats, but print with better precision
      if (sscanf(csqstr, "+CLBS: %d,%f,%f,", &tmp, &lat, &lon) >= 3) {
        if (lat != 0 && lon != 0) {
          lati = lat;
          longi = lon;
          debug("Latitude: ");
          debugln(lati, 6);
          debug("Longitude: ");
          debugln(longi, 6);
          saveGPS(); // Persist immediately
          return;    // SUCCESS
        }
      }
    }
    debugln("[GPS] Attempt failed. Retrying...");
    vTaskDelay(2000);
  }
  debugln("[GPS] All attempts to get fresh coordinates failed.");
}

void get_lat_long_date_time(char *gsm_no) {
  int response_no;
  int tmp, tmp3;
  char tmp2[16];
  int day, month, year, hour, minute, seconds;
  char status_response[256];
  char gprs_xmit_buf[300];
  msg_sent = 0;

  char *csqstr;

  //  response_no = Send_And_Check_Gprs("AT+SIMEI?\r\n", "+SIMEI:", "ERROR",
  //  5000);// 1:lat and long 4:lat,long/date and time

  SerialSIT.println("ATE0");
  response = waitForResponse("OK", 3000);
  debug("HTTP response of ATE0: ");
  debugln(response);

  const char *response_char;

  // Make and Model
  //  SerialSIT.println("AT+SIMEI?");
  //  response = waitForResponse("+SIMEI:", 5000);
  //  debug("Response of AT+SIMEI? is ");debugln(response);
  //  response_char = response.c_str();
  //  csqstr = strstr(response_char, "+SIMEI:");
  //  sscanf(csqstr, "+SIMEI: %s", &tmp2);

  // To find Latitude and Longitude. Only with A7672S

  vTaskDelay(5000);
  SerialSIT.println("AT+CLBS=1");
  response = waitForResponse("+CLBS:", 10000);
  vTaskDelay(200);
  debug("Response of AT+CLBS=1 is ");
  debugln(response);
  vTaskDelay(200);
  response_char = response.c_str();
  vTaskDelay(200);
  csqstr = strstr(response_char, "+CLBS");
  if (csqstr == NULL) {
    debugln("Error: +CLBS not found in response");
    return; // Exit if response is invalid
  }
  //  sscanf(csqstr, "+CLBS: %d,%f,%f,%d,%d/%d/%d,%d:%d:%d", &tmp,
  //  &lati,&longi,&tmp3,&year,&month,&day,&hour,&minute,&seconds);
  // We use standard %f but print with 6 decimal precision
  sscanf(csqstr, "+CLBS: %d,%f,%f,", &tmp, &lati,
         &longi); // Response for AT+CLBS=1 +CLBS: 0,12.989436,77.537910,550

  vTaskDelay(500);
  debug("Latitude is : ");
  debugln(lati, 6);
  debug("Longitude is : ");
  debugln(longi, 6);

  // Persistence: Save to SPIFFS so we can use it in Health Reports even
  // without a fix
  if (lati != 0 && longi != 0) {
    saveGPS();
  }

  snprintf(status_response, sizeof(status_response),
           "%s,%s,STAT_AD-C,%s,%04d-%02d-%02dT%02d:%02d,SIM_1,%04d,%.6f,%.6f,0."
           "0\r\n\032",
           NETWORK, STATION_TYPE, station_name, current_year, current_month,
           current_day, current_hour, current_min, signal_strength, lati,
           longi);

  vTaskDelay(500);

  debug("Status response for GET_GPS is ");
  debugln(status_response);
  // KSNDMC,TRG,STAT_AD-C,1809,2024-10-28T08:38,SIM_1,-069,13.004021,77.549263,0.0

  // first give send sms command
  SerialSIT.println("AT+CMGF=1");
  response = waitForResponse("OK", 5000);
  debug("Response of AT+CMGF=1 is ");
  debugln(response);

  debug("Mobile number to be sent to is : ");
  debugln(gsm_no);

  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CMGS=\"%s\"\r", gsm_no);
  SerialSIT.println(gprs_xmit_buf);
  debug("Waiting for '>' prompt...");
  response = waitForResponse(">", 15000);
  if (response.indexOf(">") != -1) {
    debugln(" Received!");
    SerialSIT.print(status_response); //  SerialSIT.write(26);
    debug("Waiting for +CMGS confirmation...");
    response = waitForResponse("+CMGS:", 15000);
    debugln(" Done.");
    debug("Response of AT+CMGS is ");
    debugln(response);
  } else {
    debugln(" TIMEOUT! No '>' prompt received.");
    debug("Response was: ");
    debugln(response);
  }

  const char *char_resp = response.c_str();
  if (strstr(char_resp, "+CMGS")) {
    msg_sent = 1;
    debugln("SUCCESS IN SENDING GPS");
  } else {
    debugln("FAILED TO SEND GPS - No +CMGS in response");
  }

  vTaskDelay(1000);

  SerialSIT.println("ATE1");
  response = waitForResponse("OK", 3000);
  debug("HTTP response of ATE1: ");
  debugln(response);
}

int setup_ftp() {
  flushSerialSIT(); // Clear leftover data from previous task
  char gprs_xmit_buf[300];
  String response; // Local for safety
  const char *ftpServer;
  const char *ftpUser;
  const char *ftpPassword;
  int portName;

  if ((strstr(UNIT, "BIHAR_TRG")) || (strstr(UNIT, "BIHAR_TEST"))) {
    // Bihar
    if (send_daily == 1) {
      ftpServer = "ftphbih.spatika.net";
      ftpUser = "trg_desbih_csvdt";
      ftpPassword = "rf24hrcsv2021";
      portName = 21;
      send_daily = 0;
    } else {
      ftpServer = "89.32.144.163"; // dot.spatika.net
      ftpUser = "dota_bih";
      ftpPassword = "airdata2024";
      portName = 21;
    }
  } else if ((strstr(UNIT, "KSNDMC_OLD")) || (strstr(UNIT, "KSNDMC_TRG")) ||
             (strstr(UNIT, "KSNDMC_TWS"))) {
    // DMC
    if (send_daily == 1) {
      ftpServer = "ftp1.ksndmc.net";
      ftpUser = "trg_spatika_v2@ksndmc.net";
      ftpPassword = "trgsp#123";
      portName = 21;
      send_daily = 0;
    } else if (send_daily == 2) {
#if SYSTEM == 1
      ftpServer = "ftp1.ksndmc.net";
      ftpUser = "tws_spatika_v2";
      ftpPassword = "twssp#987";
      portName = 21;
#endif
#if SYSTEM == 2
      ftpServer = "ftp1.ksndmc.net";
      ftpUser = "twsrf_spatika_v2";
      ftpPassword = "sittao#10";
      portName = 21;
#endif
      //            ftpServer = "ftp.spatika.net";
      //            ftpUser = "hmisadmin";
      //            ftpPassword = "spt2015";
      //            portName = 21;
      send_daily = 0;
    } else {
      ftpServer = "89.32.144.163"; // dota.spatika.net
      ftpUser = "dota_dmc";
      ftpPassword = "airdata2016";
      portName = 21;
    }
  } else if (strstr(UNIT, "SPATIKA_GEN")) {
#if SYSTEM == 0
    ftpServer = "ftp.spatika.net";
    ftpUser = "trg_gen";
    ftpPassword = "spgen123";
    portName = 21;
#endif
#if SYSTEM == 2
    ftpServer = "ftp.spatika.net";
    ftpUser = "twsrf_gen";
    ftpPassword = "spgen123";
    portName = 21;
#endif
  }

  int result = -1;

  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
           "AT+CFTPSLOGIN=\"%s\",%d,\"%s\",\"%s\",0", ftpServer, portName,
           ftpUser, ftpPassword); //\r\n removed

  // NEW DOTA
  SerialSIT.println("AT+CFTPSSTOP"); // Ensure a clean slate
  vTaskDelay(500);
  SerialSIT.println("AT+CFTPSSTART");
  response = waitForResponse("+CFTPSSTART: 0", 10000);
  debugln(response);

  SerialSIT.println("AT+CFTPSSINGLEIP=1"); // FTP client context
  waitForResponse("OK", 5000);

  // Force plain FTP mode (Security level 0 = No SSL)
  SerialSIT.println("AT+CFTPSCFG=\"security\",0");
  waitForResponse("OK", 2000);

  // Explicitly bind to GPRS context 1
  SerialSIT.println("AT+CFTPSCFG=\"bindcid\",1");
  waitForResponse("OK", 2000);

  SerialSIT.println("AT+CFTPSPASV=1"); // Passive Mode Mandatory for Cellular
  waitForResponse("OK", 5000);

  // DNS Warm-up: Force resolution of FTP server before login (Fixes Error 13)
  debugln("[FTP] Warming up DNS...");
  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CDNSGIP=\"%s\"",
           ftpServer);
  SerialSIT.println(gprs_xmit_buf);
  // v5.50 Optimized: Wait up to 10s for DNS resolution.
  response = waitForResponse("+CDNSGIP:", 10000);
  debugln(response);

  vTaskDelay(2000 / portTICK_PERIOD_MS); // Pre-login settling delay

  // Re-prepare login command
  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
           "AT+CFTPSLOGIN=\"%s\",%d,\"%s\",\"%s\",0", ftpServer, portName,
           ftpUser, ftpPassword);
  SerialSIT.println(gprs_xmit_buf);
  // v5.50 Optimized: Wait 45s for login (If it hasn't succeeded by then on
  // BSNL, it likely won't)
  response = waitForResponse("+CFTPSLOGIN:", 45000);
  debugln(response);

  // v5.50: Removed Active Mode retry as it usually fails during PUT on NAT
  // networks. Instead, if login fails, a full bearer reset is better.

  rssiIndex = response.indexOf("+CFTPSLOGIN:");
  if (rssiIndex != -1) {
    rssiIndex += 13;
    rssiStr = response.substring(rssiIndex, rssiIndex + 2);
  } else {
    rssiStr = "99"; // Explicit fail code
  }
  resp = rssiStr.c_str();
  result = atoi(resp);
  debug("Result is ");
  debugln(result);
  if (result == 0) {
    debugln("FTP Login success");
    return 1;
  } else {
    debugln("FTP Login unsuccessful");
    return 0;
  }
}

void fetchFromHttpAndUpdate(char *fileName) {
  int handle_no;
  char gprs_xmit_buf[300];
  int total_no_of_bytes = 0;
  int actual_downloaded = 0;
  (void)handle_no; // suppress unused warning

  debugf1("[OTA] Starting Range-GET download for: %s\n", fileName);

  // === STEP 1: HEAD request - get file size without downloading body ===
  SerialSIT.println("AT+HTTPTERM");
  waitForResponse("OK", 2000);
  vTaskDelay(500);
  flushSerialSIT();

  SerialSIT.println("AT+HTTPINIT");
  if (waitForResponse("OK", 5000).indexOf("OK") == -1) {
    debugln("[OTA] HTTPINIT failed (HEAD). Aborting.");
    Preferences p;
    p.begin("ota-track", false);
    p.putInt("fail_cnt", p.getInt("fail_cnt", 0) + 1);
    p.putString("fail_res", "HTTPINIT Fail");
    p.end();
    return;
  }
  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
           "AT+HTTPPARA=\"URL\",\"http://%s:%s/builds/%s\"", HEALTH_SERVER_IP,
           HEALTH_SERVER_PORT, fileName);
  SerialSIT.println(gprs_xmit_buf);
  if (waitForResponse("OK", 2000).indexOf("OK") == -1) {
    debugln("[OTA] URL setup failed. Aborting.");
    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 2000);
    return;
  }
  // HTTP HEAD to determine Content-Length
  SerialSIT.println("AT+HTTPACTION=2");
  String head_resp = waitForResponse("+HTTPACTION:", 30000);
  debugf1("[OTA] HEAD: %s\n", head_resp.c_str());
  int lc0 = head_resp.lastIndexOf(",");
  if (lc0 != -1)
    total_no_of_bytes = head_resp.substring(lc0 + 1).toInt();
  SerialSIT.println("AT+HTTPTERM");
  waitForResponse("OK", 2000);

  debugf1("[OTA] File size: %d bytes\n", total_no_of_bytes);
  if (total_no_of_bytes <= 100000) {
    debugln("[OTA] Invalid file size. Aborting.");
    Preferences p;
    p.begin("ota-track", false);
    p.putInt("fail_cnt", p.getInt("fail_cnt", 0) + 1);
    p.putString("fail_res", "Bad size");
    p.end();
    return;
  }

  // === STEP 2: Begin OTA partition write ===
  if (!Update.begin(total_no_of_bytes, U_FLASH)) {
    debugf1("[OTA] Update.begin failed: %s\n", Update.errorString());
    return;
  }
  Update.onProgress(progressCallBack);
  ota_writing_active = true;
  debugln("[OTA] OTA begun. Downloading in 64KB Range chunks...");

  const int RANGE_SIZE = 65536;
  const int READ_SIZE = 4096;
  uint8_t *buf = (uint8_t *)malloc(READ_SIZE);
  if (!buf) {
    debugln("[OTA] malloc failed!");
    Update.abort();
    ota_writing_active = false;
    return;
  }

  // === STEP 3: Download 64KB chunks via Range GET ===
  while (actual_downloaded < total_no_of_bytes) {
    int r_start = actual_downloaded;
    int r_end = min(actual_downloaded + RANGE_SIZE - 1, total_no_of_bytes - 1);
    debugf1("[OTA] Range bytes=%d-%d\n", r_start);
    debug(" - ");
    debugln(r_end);

    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 2000);
    vTaskDelay(300);
    flushSerialSIT();

    SerialSIT.println("AT+HTTPINIT");
    if (waitForResponse("OK", 5000).indexOf("OK") == -1) {
      debugln("[OTA] HTTPINIT fail in chunk loop.");
      break;
    }
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
             "AT+HTTPPARA=\"URL\",\"http://%s:%s/builds/%s\"", HEALTH_SERVER_IP,
             HEALTH_SERVER_PORT, fileName);
    SerialSIT.println(gprs_xmit_buf);
    waitForResponse("OK", 2000);

    // Range header
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
             "AT+HTTPPARA=\"USERDATA\",\"Range: bytes=%d-%d\"", r_start, r_end);
    SerialSIT.println(gprs_xmit_buf);
    String rng_r = waitForResponse("OK", 2000);
    debugf1("[OTA] Range set: %s\n", rng_r.indexOf("OK") != -1 ? "OK" : "FAIL");

    // Execute GET (90s timeout: 64KB on 2G ~11s + overhead)
    SerialSIT.println("AT+HTTPACTION=0");
    String act = waitForResponse("+HTTPACTION:", 90000);
    debugf1("[OTA] GET resp: %s\n", act.c_str());

    int lc2 = act.lastIndexOf(",");
    int chunk_got = (lc2 != -1) ? act.substring(lc2 + 1).toInt() : 0;
    if (chunk_got <= 0) {
      debugln("[OTA] GET failed. Aborting.");
      SerialSIT.println("AT+HTTPTERM");
      waitForResponse("OK", 2000);
      Update.abort();
      free(buf);
      ota_writing_active = false;
      Preferences p;
      p.begin("ota-track", false);
      p.putInt("fail_cnt", p.getInt("fail_cnt", 0) + 1);
      p.putString("fail_res", "Chunk GET fail");
      p.end();
      return;
    }
    debugf1("[OTA] Chunk: %d bytes\n", chunk_got);

    // Read chunk binary via HTTPREAD
    int chk_off = 0;
    bool read_ok = true;
    while (chk_off < chunk_got) {
      int to_read = min(chunk_got - chk_off, READ_SIZE);
      snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+HTTPREAD=%d,%d",
               chk_off, to_read);
      SerialSIT.println(gprs_xmit_buf);

      // Wait for +HTTPREAD: header (signals binary data follows)
      bool foundHdr = false;
      String hdr = "";
      unsigned long t0 = millis();
      while (millis() - t0 < 10000) {
        while (SerialSIT.available()) {
          char c = SerialSIT.read();
          hdr += c;
          if (c == '\n' && hdr.indexOf("+HTTPREAD:") != -1) {
            foundHdr = true;
            break;
          }
        }
        if (foundHdr)
          break;
        vTaskDelay(2);
        esp_task_wdt_reset();
      }
      if (!foundHdr) {
        debugf1("[OTA] No HTTPREAD hdr at %d. Aborting.\n", chk_off);
        read_ok = false;
        break;
      }

      // Read exact binary bytes
      SerialSIT.setTimeout(5000);
      int got = 0;
      unsigned long rs = millis();
      while (got < to_read && millis() - rs < 15000) {
        int r = SerialSIT.readBytes(buf + got, to_read - got);
        if (r > 0)
          got += r;
        else
          vTaskDelay(5);
        esp_task_wdt_reset();
      }
      if (got > 0) {
        if (Update.write(buf, got) != (size_t)got) {
          debugln("[OTA] Flash write failed!");
          read_ok = false;
          break;
        }
        chk_off += got;
        actual_downloaded += got;
        waitForResponse("OK", 1000); // consume footer
      } else {
        debugln("[OTA] HTTPREAD no bytes.");
        read_ok = false;
        break;
      }
      vTaskDelay(10);
      esp_task_wdt_reset();
    }

    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 2000);

    if (!read_ok) {
      debugln("[OTA] Chunk read error. Aborting.");
      Update.abort();
      free(buf);
      ota_writing_active = false;
      return;
    }
    debugf1("[OTA] Progress: %d / %d\n", actual_downloaded);
    vTaskDelay(500);
    esp_task_wdt_reset();
  }

  free(buf);
  ota_writing_active = false;

  // === STEP 4: Finalize OTA ===
  if (actual_downloaded == total_no_of_bytes && total_no_of_bytes > 0) {
    if (Update.end()) {
      debugln("[OTA] Flash COMPLETE and VERIFIED!");
      File firm = SPIFFS.open("/firmware.doc", FILE_WRITE);
      if (firm) {
        firm.print(UNIT_VER);
        firm.close();
      }
      Preferences p;
      p.begin("ota-track", false);
      p.putString("status", "success");
      p.end();
      debugln("[OTA] Rebooting in 3s...");
      delay(3000);
      ESP.restart();
    } else {
      String err = Update.errorString();
      debugf1("[OTA] Update.end FAILED: %s\n", err.c_str());
      Preferences p;
      p.begin("ota-track", false);
      p.putInt("fail_cnt", p.getInt("fail_cnt", 0) + 1);
      p.putString("fail_res", err.c_str());
      p.end();
    }
  } else {
    debugf1("[OTA] Incomplete: %d/%d. Aborting.\n", actual_downloaded);
    Update.abort();
  }
  debugln("[OTA] Done.");
}

void copyFromSPIFFSToFS(char *dateFile) {

  char SPIFFSFile[30], fileName[30];
  snprintf(SPIFFSFile, sizeof(SPIFFSFile), "/%s_%s.txt", station_name,
           dateFile);
  snprintf(fileName, sizeof(fileName), "%s_%s.txt", station_name, dateFile);
  debugln("SPIFFS Card FileName is ");
  debugln(SPIFFSFile);
  debugln("FileName is ");
  debugln(fileName);
  int fileSize, s;
  int handle_no;
  char gprs_xmit_buf[300];
  char *csqstr;
  const char *response_char;
  int len;
  String content; // content_append; iter17

  fileSize = 0;
  s = 0; // resetting to 1st record of sd card file ..

  if (SPIFFS.exists(SPIFFSFile)) {
    SerialSIT.println("ATE0");
    response = waitForResponse("OK", 3000);
    SerialSIT.println("AT+FSCD=C:/");
    response = waitForResponse("OK", 10000);
    debug("Response of AT+FSCD ");
    debugln(response);

    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSOPEN=C:/%s,0\r\n",
             fileName); // 0 : if the file does not exist, it will be created
    SerialSIT.println(gprs_xmit_buf);
    response = waitForResponse("+FSOPEN", 5000);
    debug("Response is ");
    debugln(response);
    response_char = response.c_str();
    vTaskDelay(200);
    csqstr = strstr(response_char, "+FSOPEN:");
    if (csqstr == NULL) {
      debugln("Error: +FSOPEN not found in response");
      return; // Exit if response is invalid
    }
    sscanf(csqstr, "+FSOPEN: %d,", &handle_no);

    File file1 = SPIFFS.open(SPIFFSFile, FILE_READ);
    debug("File size of the SPIFFS file is ");
    debugln(file1.size());
    fileSize = file1.size();
    file1.seek(s);
    content = file1.readString(); // Read the rest of the file

    // filehandle,length,timeout
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSWRITE=%d,%d,10\r\n",
             handle_no,
             fileSize); // 0 : if the file does not exist, it will be created
    vTaskDelay(200);
    SerialSIT.println(gprs_xmit_buf);
    response = waitForResponse("CONNECT", 5000);
    SerialSIT.println(content);
    response = waitForResponse("+FSWRITE", 5000);
    debug("Response of FSWRITE is ");
    debugln(response);
    file1.close();

    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSCLOSE=%d",
             handle_no); // FILEHANDLE,offset,(0:start of file 1:cur pos of
                         // pointer 2: end of file
    SerialSIT.println(gprs_xmit_buf);
    response = waitForResponse("+FSCLOSE", 5000);
    vTaskDelay(200);

    ftp_login_flag = setup_ftp();

    if (ftp_login_flag == 1) {
      snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CFTPSPUTFILE=\"%s\",1",
               fileName); // FILEHANDLE,offset,(0:start of file 1:cur pos of
                          // pointer 2: end of file
      SerialSIT.println(gprs_xmit_buf); // FTP client context
      response = waitForResponse("+CFTPSPUTFILE: 0", 150000); // 120000
      debug("Response of AT+CFTPSPUTFILE ");
      debugln(response);

      SerialSIT.println("AT+CFTPSLOGOUT");
      response = waitForResponse("+CFTPSLOGOUT: 0", 9000);
      debug("Response of AT+CFTPSLOGOUT is ");
      debugln(response);
      vTaskDelay(200);

      SerialSIT.println("AT+CFTPSSTOP");
      response = waitForResponse("OK", 5000);
      debug("Response of AT+CFTPSSTOP is ");
      debugln(response);
      vTaskDelay(200);

      snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSDEL=\"C:/%s\"",
               fileName); // 0 : if the file does not exist, it will be created
      SerialSIT.println(gprs_xmit_buf);
      response = waitForResponse("OK", 10000);
      debug("Response of AT+FSDEL is  ");
      debugln(response);
    } else {
      debugln("FTP Login unsuccessful");
    }
  } else {
    debugln("File not found");
  }
}

/*
 * COMMON FUNCTIONS
 * - parse_http_head()
 * - waitForResponse()
 * - read_line()
 * - resync_time()
 */

char *parse_http_head(char *response, char *check) {
  char *search_str = NULL;
  char *token = strtok(response, "\r\n");
  while (token != NULL) {
    if (strstr(token, check) != NULL) {
      search_str = strchr(token, ':') +
                   2; // eg. Skip the "+CSQ: " prefix ... +2 to skip \r\n
      break;
    }
    token = strtok(NULL, "\r\n");
  }
  return search_str;
}

String waitForResponse(String expectedResponse, int timeout) {
  String response = "";
  response.reserve(512); // Pre-allocate to reduce heap fragmentation
  unsigned long startTime = millis();

  while ((millis() - startTime) < timeout) {
    vTaskDelay(10);
    esp_task_wdt_reset(); // Keep watchdog happy during long AT command waits

    while (SerialSIT.available()) {
      char c = SerialSIT.read();
      if (response.length() < 2048) { // Prevent unbounded growth
        response += c;
      }
    }

    if (response.indexOf(expectedResponse) != -1) {
      return response;
    }
  }

  return response;
}

int read_line(char *src, char *dest, char delim_chr) {
  int i, j;
  char c;
  *dest = 0;
  j = 0;
  for (i = 0; (i < 45); i++, j++) {
    c = *src;
    if (c == 0)
      return (j);
    if (c == delim_chr)
      return (++j);
    src++;
    if ((c == '\r') || (c == '\n')) {
      c = *src++;
      j++;
      if ((c == '\r') || (c == '\n'))
        j++;
      break;
    } else {
      *((char *)(i + dest)) = c;
      *((char *)(i + dest + 1)) = 0;
    }
  }
  return (j);
}

// Function to convert GMT to IST
void convert_gmt_to_ist(struct tm *gmt_time) {
  // Adding 5 hours and 30 minutes to GMT for IST conversion
  gmt_time->tm_hour += 5;
  gmt_time->tm_min += 30;

  // Normalize the time in case minutes overflow (i.e., more than 60 minutes)
  mktime(gmt_time);
}

// v6.67: Implementation of send_at_cmd
void send_at_cmd(char *cmd, char *check, char *spl) {
  SerialSIT.println(cmd);
  waitForResponse(check, 2000);
}

/**
 * Sends a health report to the Google Sheets "Server"
 * Parameters: stn, type, gbat, ebat, sig
 */
bool send_health_report(bool useJitter) {
  // v7.00: Ensure bearer is live before starting Health Report flow
  if (!verify_bearer_or_recover()) {
    debugln("[Health] Bearer lost. Aborting.");
    return false;
  }

  // v6.68: Graceful Reset (Term only). Do NOT use CIPSHUT as it kills the
  // bearer!
  SerialSIT.println("AT+HTTPTERM");
  waitForResponse("OK", 3000);
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  if (useJitter) {
    int jitter = random(0, 5000);
    vTaskDelay(jitter / portTICK_PERIOD_MS);
  }

  // Perform Rainfall Integrity check before reporting
  checkRainfallIntegrity();

  // v5.49 Golden Data Analysis: Strictly analyze the Closed Day (Yesterday
  // 08:45 AM -> Today 08:30 AM)
  bool unresolvedPD = false;
  bool unresolvedNDM = false;

  char cleanStn[16];
  strncpy(cleanStn, station_name, 15);
  cleanStn[15] = '\0';
  int slen = strlen(cleanStn);
  while (slen > 0 && cleanStn[slen - 1] == ' ') {
    cleanStn[slen - 1] = '\0';
    slen--;
  }

  // 1. Live Progress Look-ahead (Updates NetCount for JSON)
  int dummyNetCount;
  bool dummyPD, dummyNDM;
  analyzeFileHealth(diag_sent_mask_cur, &diag_net_data_count, &dummyPD,
                    &dummyNDM);

  // 2. Official Day Recovery (Drives the Flags/Alarms)
  // We do this LAST so it overwrites diag_cdm_status and PD/NDM flags
  // correctly.
  analyzeFileHealth(diag_sent_mask_prev, &diag_net_data_count_prev,
                    &unresolvedPD, &unresolvedNDM);

  // 2. Prepare Grouped Data
  char sensor_info[32];
  if (SYSTEM == 0) { // TRG
    snprintf(sensor_info, sizeof(sensor_info), "RF-OK");
  } else if (SYSTEM == 1) { // TWS
    snprintf(sensor_info, sizeof(sensor_info), "TH-%s,B-%s,WS-%s,WD-%s",
             (hdcType == HDC_UNKNOWN ? "FAIL" : "OK"),
             (bmeType == BME_UNKNOWN ? "NA" : "OK"), (ws_ok ? "OK" : "FAIL"),
             (wd_ok ? "OK" : "FAIL"));
  } else if (SYSTEM == 2) { // TWS-RF
    // v6.50: Platinum labels as requested
    snprintf(sensor_info, sizeof(sensor_info), "TH-%s,RF-OK,WS-%s,WD-%s",
             (hdcType == HDC_UNKNOWN ? "FAIL" : "OK"), (ws_ok ? "OK" : "FAIL"),
             (wd_ok ? "OK" : "FAIL"));
  }

  // GPS Group
  char gps_str[32];
  snprintf(gps_str, sizeof(gps_str), "%.6f,%.6f", lati, longi);

  // Time metrics
  float avg_reg =
      (diag_reg_count > 0) ? (float)diag_reg_time_total / diag_reg_count : 0;
  float avg_http =
      (diag_http_success_count > 0)
          ? (float)diag_http_time_total / diag_http_success_count / 1000.0
          : 0;

  // v5.43 Golden Summary Health Logic
  char h_status[256];
  h_status[0] = '\0';

// v5.86: Removed local version definition to avoid conflicts
// v5.88: Macro moved to globals.h to avoid redefinition errors
#define ADD_FAULT(f)                                                           \
  do {                                                                         \
    if (h_status[0] != '\0')                                                   \
      strcat(h_status, "_");                                                   \
    strcat(h_status, f);                                                       \
  } while (0)

  // 1. Critical System Faults (Power & HW)
  if (diag_last_reset_reason == 15)
    ADD_FAULT("BROWNOUT");
  if (diag_last_reset_reason == 7 || diag_last_reset_reason == 8 ||
      diag_last_reset_reason == 9 || diag_last_reset_reason == 13 ||
      diag_last_reset_reason == 16)
    ADD_FAULT("WDOG");
  if (!diag_rtc_battery_ok || current_year < 2025)
    ADD_FAULT("RTC_FAIL");

  // 2. Data Health (Golden Summary Standard)
  if (unresolvedPD)
    ADD_FAULT("PD");
  if (strcmp(diag_cdm_status, "OK") != 0 && current_hour >= 9)
    ADD_FAULT("CDM");
  if (unresolvedNDM)
    ADD_FAULT("NDM");

  // 3. Sensor Quality (Golden Summary Standard) - System Specific
  if (SYSTEM != 0) { // TWS or TWS-RF
    if (diag_temp_cv)
      ADD_FAULT("Temp_CV");
    if (diag_hum_cv)
      ADD_FAULT("Humi_CV");
    if (diag_ws_cv)
      ADD_FAULT("WS_CV");
    if (diag_temp_erv)
      ADD_FAULT("Temp_ERV");
    if (diag_hum_erv)
      ADD_FAULT("Humi_ERV");
    if (diag_ws_erv)
      ADD_FAULT("WS_ERV");
    if (diag_temp_erz)
      ADD_FAULT("Temp_ERZ");
    if (diag_hum_erz)
      ADD_FAULT("Humi_ERZ");
  }

  if (SYSTEM != 1) { // TRG or TWS-RF
    if (diag_rain_jump)
      ADD_FAULT("Rain_JUMP");
    if (diag_rain_calc_invalid)
      ADD_FAULT("Rain_CALC_ERR");
    if (diag_rain_reset)
      ADD_FAULT("Rain_RESET");
  }

  // 4. Hardware Connectivity - System Specific
  if (SYSTEM != 0) {
    if (hdcType == HDC_UNKNOWN)
      ADD_FAULT("TH_MISSING");
    if (!wd_ok)
      ADD_FAULT("WD_MISSING");
  }

  // 5. Environment & Network
  if (li_bat_val < 3.4)
    ADD_FAULT("BATT_CRIT");
  else if (li_bat_val < 3.6)
    ADD_FAULT("BATT_LOW");

  if (current_hour >= 9 && current_hour <= 16 && solar_val < (li_bat_val + 0.1))
    ADD_FAULT("SOLAR_FAIL");

  if (diag_consecutive_http_fails > 3)
    ADD_FAULT("HTTP_BACKLOG");
  if (avg_reg > 150.0)
    ADD_FAULT("POOR_SIG");

  // Default
  if (h_status[0] == '\0')
    strcpy(h_status, "OK");

  int cur_dd_h = current_day, cur_mm_h = current_month, cur_yy_h = current_year;
  int prev_dd_h = current_day, prev_mm_h = current_month,
      prev_yy_h = current_year;

  int h_idx = (current_hour < 24) ? current_hour : 0;
  int m_idx = (current_min < 60) ? current_min : 0;
  int sNo = h_idx * 4 + m_idx / 15;
  sNo = (sNo + 61) % 96;

  if (sNo <= 60) {
    next_date(&cur_dd_h, &cur_mm_h, &cur_yy_h);
  } else {
    previous_date(&prev_dd_h, &prev_mm_h, &prev_yy_h);
  }

  char prevF[32], curF[32];
  snprintf(prevF, sizeof(prevF), "/%s_%04d%02d%02d.txt", station_name,
           prev_yy_h, prev_mm_h, prev_dd_h);
  snprintf(curF, sizeof(curF), "/%s_%04d%02d%02d.txt", station_name, cur_yy_h,
           cur_mm_h, cur_dd_h);

  int cur_stored = countStored(curF);
  int prev_stored = countStored(prevF);

  // 3. Construct JSON Payload (v5.42 expanded)
  char jsonBody[1024];
  snprintf(jsonBody, sizeof(jsonBody),
           "{\"stn_id\":\"%s\",\"unit_type\":\"%s\",\"system\":%d,"
           "\"health_sts\":\"%s\",\"sensor_sts\":\"%s\","
           "\"reset_reason\":%d,\"rtc_ok\":%d,\"uptime_hrs\":%u,"
           "\"bat_v\":%.2f,\"sol_v\":%.2f,\"signal\":%d,"
           "\"reg_fails\":%d,\"reg_avg\":%.1f,\"reg_worst\":%d,"
           "\"reg_fail_type\":\"%s\","
           "\"http_fails\":%d,\"http_fail_reason\":\"%s\",\"http_avg\":%.1f,"
           "\"net_cnt\":%d,\"net_cnt_prev\":%d,"
           "\"cur_stored\":%d,\"prev_stored\":%d,"
           "\"http_suc_cnt\":%d,\"http_suc_cnt_prev\":%d,"
           "\"ndm_cnt\":%d,\"pd_cnt\":%d,\"cdm_sts\":\"%s\","
           "\"first_http\":%d,"
           "\"spiffs_kb\":%u,\"spiffs_total_kb\":%u,"
           "\"sd_sts\":\"%s\",\"calib\":\"%s\","
           "\"ver\":\"%s\",\"carrier\":\"%s\","
           "\"iccid\":\"%s\",\"gps\":\"%s\","
           "\"ota_fails\":%d,\"ota_fail_reason\":\"%s\"}",
           cleanStn, UNIT, SYSTEM, h_status, sensor_info,
           diag_last_reset_reason, (diag_rtc_battery_ok ? 1 : 0),
           diag_total_uptime_hrs, li_bat_val, solar_val, signal_lvl,
           diag_gprs_fails, avg_reg, diag_reg_worst, diag_reg_fail_type,
           diag_daily_http_fails, diag_http_fail_reason, avg_http,
           (TEST_HEALTH_EVERY_SLOT ? (int)diag_net_data_count
                                   : diag_net_data_count_prev),
           diag_net_data_count_prev, // always send prev for PD analysis
           cur_stored, prev_stored, diag_http_success_count,
           diag_http_success_count_prev,
           (TEST_HEALTH_EVERY_SLOT ? diag_ndm_count : diag_ndm_count_prev),
           (TEST_HEALTH_EVERY_SLOT ? diag_pd_count : diag_pd_count_prev),
           diag_cdm_status,
           (TEST_HEALTH_EVERY_SLOT ? diag_first_http_count
                                   : diag_first_http_count_prev),
           (unsigned int)(SPIFFS.usedBytes() / 1024),
           (unsigned int)(SPIFFS.totalBytes() / 1024),
           (sd_card_ok ? "OK" : "FAIL"), "NA", UNIT_VER, carrier, cached_iccid,
           gps_str, ota_fail_count, ota_fail_reason);

  int max_attempts = 2; // v7.01: Health is critical for OTA; allow one retry.
  bool success = false;

  // Removed AT+CIPSHUT to prevent corrupting HTTPDATA buffer allocation on
  // A7672S
  if (!verify_bearer_or_recover()) {
    debugln("[Health] Bearer recovery failed. Aborting health report.");
    return false;
  }

  for (int attempt = 1; attempt <= max_attempts; attempt++) {
    debugf2("[Health] Attempt %d of %d\n", attempt, max_attempts);

    // v7.03 Cleanup Sequence: Ensure absolute clean slate before Health/OTA
    // Instead of hard-killing CID 8, we kill ONLY if it's the target or if we
    // are stuck.
    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 2000);
    vTaskDelay(1000 / portTICK_PERIOD_MS); // v7.03: Increased stabilization
                                           // after potential FTP
    flushSerialSIT();

    http_ready = false;

    SerialSIT.println("AT+HTTPINIT");
    String init_resp = waitForResponse("OK", 5000);
    if (init_resp.indexOf("OK") == -1) {
      debugln("[Health] HTTPINIT Failed. Hard-Resetting stack...");
      SerialSIT.println("AT+HTTPTERM");
      waitForResponse("OK", 2000);
      vTaskDelay(500);
      flushSerialSIT();
      SerialSIT.println("AT+HTTPINIT");
      if (waitForResponse("OK", 5000).indexOf("OK") != -1) {
        http_ready = true;
      }
    } else {
      http_ready = true;
    }

    vTaskDelay(500); // v7.03: Stabilize after INIT before PARA commands

    snprintf(ht_data, sizeof(ht_data), "AT+HTTPPARA=\"CID\",%d",
             active_cid > 0 ? active_cid : 1);
    SerialSIT.println(ht_data);
    String cid_resp = waitForResponse("OK", 1000);
    debugf1("[Health] CID Setup: %s\n", cid_resp.c_str());

    char healthUrl[64];
    snprintf(healthUrl, sizeof(healthUrl),
             "AT+HTTPPARA=\"URL\",\"http://%s:%s%s\"", HEALTH_SERVER_IP,
             HEALTH_SERVER_PORT, HEALTH_SERVER_PATH);
    SerialSIT.println(healthUrl);
    String url_resp = waitForResponse("OK", 1000);
    debugf1("[Health] URL Setup: %s\n", url_resp.c_str());

    SerialSIT.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
    String content_resp = waitForResponse("OK", 2000);
    debugf1("[Health] CONTENT Setup: %s\n", content_resp.c_str());
    vTaskDelay(200);

    // 3. Transmit using vetted function
    debugln("[Health] Initialized. Calling send_at_cmd_data()...");
    if (send_at_cmd_data(jsonBody, "") == 1) {
      success = true;
      // Process server commands from the global 'response' string
      String body = response;

      // v6.45: Robust JSON parsing (handles optional spaces from json.dumps)
      if (body.indexOf("\"cmd\"") != -1) {
        if (body.indexOf("\"REBOOT\"") != -1) {
          debugln("[Health] CMD: REBOOT received!");
          force_reboot = true;
        }

        if (body.indexOf("\"OTA_CHECK\"") != -1) {
          debugln("[Health] CMD: OTA_CHECK received!");
          // Look for "p":" (v3.2+) or "cmd_param":" (legacy)
          int pTag = body.indexOf("\"p\"");
          if (pTag == -1)
            pTag = body.indexOf("\"cmd_param\"");

          if (pTag != -1) {
            int valStart = body.indexOf(":", pTag);
            if (valStart != -1) {
              int quoteStart = body.indexOf("\"", valStart);
              if (quoteStart != -1) {
                int quoteEnd = body.indexOf("\"", quoteStart + 1);
                if (quoteEnd != -1) {
                  String param = body.substring(quoteStart + 1, quoteEnd);
                  strncpy(ota_cmd_param, param.c_str(),
                          sizeof(ota_cmd_param) - 1);
                  ota_cmd_param[sizeof(ota_cmd_param) - 1] = '\0';
                  debugf1("[Health] Target Binary: %s\n", ota_cmd_param);
                }
              }
            }
          }
          force_ota = true;
        }

        if (body.indexOf("\"FTP_DAILY\"") != -1) {
          debugln("[Health] CMD: FTP_DAILY received!");
          int pTag = body.indexOf("\"p\"");
          if (pTag == -1)
            pTag = body.indexOf("\"cmd_param\"");

          if (pTag != -1) {
            int valStart = body.indexOf(":", pTag);
            if (valStart != -1) {
              int quoteStart = body.indexOf("\"", valStart);
              if (quoteStart != -1) {
                int quoteEnd = body.indexOf("\"", quoteStart + 1);
                if (quoteEnd != -1) {
                  String param = body.substring(quoteStart + 1, quoteEnd);
                  strncpy(ftp_daily_date, param.c_str(),
                          sizeof(ftp_daily_date) - 1);
                  ftp_daily_date[sizeof(ftp_daily_date) - 1] = '\0';
                  debugf1("[Health] Target Date: %s\n", ftp_daily_date);
                }
              }
            }
          }
          force_ftp_daily = true;
        }
      }
    } else {
      debugln("[Health] Transmission failed via send_at_cmd_data.");
    }

    if (success) {
      // v7.03: CRITICAL — Close HTTP session from health report before OTA
      // opens a new one
      SerialSIT.println("AT+HTTPTERM");
      waitForResponse("OK", 2000);
      http_ready = false;
      break;
    }
  } // End of for loop

  return success;
}
/*
 *   HTTP
 *  - send_health_report()
 *  - send_http_data()
 *  - send_at_cmd_data()
 *  - store_current_unsent_data()
 */

/*
 *  SIM Network Registration and Setup
 *  - get_signal_strength();//2024 iter3
 *  - get_network();
 *  - get_registration();
 *  - get_a7672s();
 */

/*
 *  SMS & FTP
 *  - process_sms()
 *  - prepare_and_send_status()
 *  - get_lat_long_date_time()
 *  - setup_ftp()
 *  - fetchFromFtpAndUpdate()
 *  - copyFromSPIFFSToFS()
 */

/*
 * COMMON FUNCTIONS
 *  - parse_http_head()
 *  - waitForResponse()
 *  - read_line()
 *  - resync_time()
 */

// 28,2025-07-15,15:45,0008.0,24.29,73.28,01.34,323,-073,04.2
// 26,2025-07-15,15:15,0007.0,24.29,73.67,00.89,007,-114,04.2

// http_data format is
// stn_no=99999&rec_time=2025-07-15,16:15&key=rfclimate5p13&rainfall=009.5&temp=24.22&humid=74.09&w_speed=00.33&w_dir=010&signal=-073&bat_volt=04.2&bat_volt2=04.2

// http_data format is
// stn_no=99999&rec_time=2025-07-15,15:15&key=rfclimate5p13&rainfall=007.0&temp=24.29&humid=73.67&w_speed=00.89&w_dir=007&signal=-114&bat_volt=04.2&bat_volt2=04.2

// The FILE content of UNSENT file /unsent.txt
// 82,2025-07-30,05:15,22.15,81.60,00.00,219,-081,
// 83,2025-07-30,05:30,22.15,81.60,00.00,218,-051,04.1
// 94,2025-07-30,08:15,22.19,81.50,00.00,218,-130,04.1
// 08,2025-07-30,10:45,22.63,80.23,00.00,219,-081,

// Payload is
// stn_no=99999&rec_time=2025-07-15,15:15&key=rfclimate5p13&rainfall=007.0&temp=24.29&humid=73.67&w_speed=00.89&w_dir=007&signal=-114&bat_volt=04.2&bat_volt2=04.2

// Payload is
// stn_no=99999&rec_time=2025-07-15,17:00&key=rfclimate5p13&rainfall=009.5&temp=23.87&humid=77.16&w_speed=00.12&w_dir=324&signal=-073&bat_volt=04.2&bat_volt2=04.2
