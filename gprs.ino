#include "globals.h"

int active_cid = 1; // Default to 1

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
      debugf2("[GPRS] State Change: sync_mode=%d gprs_mode=%d\n", sync_mode,
              gprs_mode);
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
          send_health_report(false); // Manual trigger: NO JITTER
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
          send_health_report(false);

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
          debugln("\n****************\nStarting Automated Data "
                  "Flow\n****************");

          // BIMODAL POWER DISCIPLINE: Only send Health Report to Google twice a
          // day Target: 11:00 AM - 10:59 PM (morning) and 11:00 PM - 10:59 AM
          // (evening) RETRY LOGIC: Keep trying until successful (persists
          // across deep sleep)

          // Reset flags at midnight (new day) OR first boot
          if (current_day != health_last_reset_day) {
            // Smart initialization: Set flags based on current time
            if (health_last_reset_day == -1) {
              // First boot: Mark reports as sent if we're past their windows
              if (current_hour >= 11 && current_hour < 23) {
                // Started during morning window - allow morning send
                health_morning_sent = false;
                health_evening_sent = true; // Already past evening window
              } else {
                // Started during evening window - allow evening send
                health_morning_sent = true; // Already past morning window
                health_evening_sent = false;
              }
              debugln("[Health] First boot detected. Initialized flags based "
                      "on time.");
            } else {
              // Midnight reset - clear both flags for new day
              health_morning_sent = false;
              health_evening_sent = false;
              debugln(
                  "[Health] New day detected. Reset morning/evening flags.");
            }
            health_last_reset_day = current_day;
          }

          bool morning_window =
              (current_hour >= 11 && current_hour < 23); // 11:00 AM - 10:59 PM
          bool evening_window =
              (current_hour >= 23 || current_hour < 11); // 11:00 PM - 10:59 AM

          bool should_send_morning = morning_window && !health_morning_sent;
          bool should_send_evening = evening_window && !health_evening_sent;

          if (should_send_morning) {
            debugln(
                "[Health] Morning window (11:00-22:59). Attempting send...");
            bool success = send_health_report(true);
            if (success) {
              health_morning_sent = true;
              debugln("[Health] ✅ Morning report sent successfully!");
            } else {
              debugln(
                  "[Health] ❌ Morning report failed. Will retry next cycle.");
            }
            vTaskDelay(2000 / portTICK_PERIOD_MS);
          } else if (should_send_evening) {
            debugln(
                "[Health] Evening window (23:00-10:59). Attempting send...");
            bool success = send_health_report(true);
            if (success) {
              health_evening_sent = true;
              debugln("[Health] ✅ Evening report sent successfully!");
            } else {
              debugln(
                  "[Health] ❌ Evening report failed. Will retry next cycle.");
            }
            vTaskDelay(2000 / portTICK_PERIOD_MS);
          } else {
            debugln("[Health] Reports already sent (M:" +
                    String(health_morning_sent) +
                    " E:" + String(health_evening_sent) + "). Skipping.");
          }

          if (solar_val < 3.5 && solar_val > 0.5) {
            debugln("[BATT] 🚨 LOW GPRS BATTERY! Skipping heavy data task.");
            sync_mode = eHttpStop;
          } else {
            debugln("[BATT] ✅ Battery OK. Proceeding with Spatika Upload.");
            sync_mode = eHttpStarted;
            send_http_data();
          }
          httpInitiated = false;
        } else if (gprs_mode == eGprsSignalForStoringOnly) {
          store_current_unsent_data();
          sync_mode = eHttpStop;
          httpInitiated = false;
        }
      }
    }

    // --- IDLE / STOP HANDLING ---
    if (sync_mode == eHttpStop || sync_mode == eSMSStop) {
      if (gprs_started) {
        debugln("[GPRS] Stopping module (Stop requested/Idle)...");
        digitalWrite(26, LOW); // Power OFF
        gprs_started = false;
        gprs_mode = eGprsInitial;
      }
    }

    esp_task_wdt_reset();
    vTaskDelay(1000); // Reduced from 2000 for faster state transitions
  }
}

void start_gprs() {
  // Power ON GPRS
  digitalWrite(26, HIGH); // Power on GPRS module
  vTaskDelay(200);
  vTaskDelay(delay_val);
  debugln();
  SerialSIT.println("AT+CPIN?");
  response = waitForResponse("+CPIN: READY", 5000);
  debug("HTTP response of AT+CPIN? ");
  debugln(response);
  const char *char_resp = response.c_str();
  SerialSIT.println("ATE0"); // Echo OFF to prevent parsing races
  waitForResponse("OK", 1000);

  if (strstr(char_resp, "+CME ERROR")) { // TRG8-3.0.5g
    signal_strength = -114;
    debugln("SIM ERROR DETECTED");
    strcpy(reg_status, "SIM ERROR");
    gprs_mode =
        eGprsSignalForStoringOnly; // It will be a trigger point for scheduler
                                   // to enter into 15min and just store
  } else {
    get_signal_strength();
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
  sscanf(charArray, "%02d,%04d-%02d-%02d,%02d:%02d,%f,%f,%f,%03d,%04d,%f",
         &temp_sampleNo, &temp_year, &temp_month, &temp_day, &temp_hr,
         &temp_min, &temp_temp, &temp_hum, &temp_avg_ws, &temp_dir, &temp_sig,
         &temp_bat);
#endif
#if SYSTEM == 2
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
           float(temp_crf)); // Adding the current rf to last recorded cum_rf
                             // will make current cum_rf
#endif

  if (data_mode == eClosingData) {
    temp_hr = 8;
    temp_min = 30;
  }

  if (!strcmp(httpSet[http_no].Format,
              "json")) { // if json then this loop otherwise goto urlencoded one
// BIHAR TRG
#if SYSTEM == 0
    snprintf(http_data, sizeof(http_data),
             "{\"StnNo\":\"%s\",\"DeviceTime\":\"%04d-%02d-%02d "
             "%02d:%02d:00\",\"RainDP\":\"%05.2f\",\"RainCuml\":\"%05.2f\","
             "\"BatVolt\":\"%s\",\"SigStr\":\"%04d\",\"ApiKey\":\"%s\"}",
             station_name, temp_year, temp_month, temp_day, temp_hr, temp_min,
             temp_instrf, temp_crf, sample_bat, temp_sig, httpSet[http_no].Key);
#endif

  } else {
    // KSNDMC TRG
    if (!strcmp(NETWORK, "KSNDMC")) {
#if (SYSTEM == 0)
      snprintf(http_data, sizeof(http_data),
               "stn_id=%s&rec_time=%04d-%02d-%02d,%02d:%02d&rainfall=%05.2f&"
               "signal=%04d&key=%s&bat_volt=%s&bat_volt1=%s",
               station_name, temp_year, temp_month, temp_day, temp_hr, temp_min,
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
               station_name, temp_year, temp_month, temp_day, temp_hr, temp_min,
               temp_crf, temp_sig, sample_bat, httpSet[http_no].Key);
      debugln();
      debugln("SPATIKA TRG data ..");
#endif
    }

    // TWS
#if SYSTEM == 1
    snprintf(http_data, sizeof(http_data),
             "stn_no=%s&rec_time=%04d-%02d-%02d,%02d:%02d&temp=%s&humid=%s&w_"
             "speed=%s&w_dir=%s&signal=%04d&key=%s&bat_volt=%s&bat_volt2=%s",
             station_name, temp_year, temp_month, temp_day, temp_hr, temp_min,
             sample_temp, sample_hum, sample_avgWS, sample_WD, temp_sig,
             httpSet[http_no].Key, sample_bat, sample_bat);
#endif

    // TWS-RF
#if SYSTEM == 2
    if (strstr(UNIT, "SPATIKA_GEN"))
      snprintf(
          http_data, sizeof(http_data),
          "stn_id=%s&rec_time=%04d-%02d-%02d,%02d:%02d&key=%s&rainfall=%05."
          "2f&temp=%s&humid=%s&w_speed=%s&w_dir=%s&signal=%04d&bat_volt=%s&"
          "bat_volt2=%s",
          station_name, temp_year, temp_month, temp_day, temp_hr, temp_min,
          httpSet[http_no].Key, temp_crf, sample_temp, sample_hum, sample_avgWS,
          sample_WD, temp_sig, sample_bat, sample_bat);
    else
      snprintf(
          http_data, sizeof(http_data),
          "stn_no=%s&rec_time=%04d-%02d-%02d,%02d:%02d&key=%s&rainfall=%05."
          "2f&temp=%s&humid=%s&w_speed=%s&w_dir=%s&signal=%04d&bat_volt=%s&"
          "bat_volt2=%s",
          station_name, temp_year, temp_month, temp_day, temp_hr, temp_min,
          httpSet[http_no].Key, temp_crf, sample_temp, sample_hum, sample_avgWS,
          sample_WD, temp_sig, sample_bat, sample_bat);
#endif
  }

  debug("http_data format is ");
  debugln(http_data);
  debugln();
  success_count = -1;
  success_count = send_at_cmd_data(
      http_data, charArray); // Current data is sent for the first time ...
                             // 0:Failure ....  1:Success
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

  if (success_count == 0) { // Failure to send the current data in 1st attempt
    for (unsent_counter = 1; unsent_counter <= 3 && http_code != 200;
         unsent_counter++) {
      esp_task_wdt_reset();
      debug("AG Retrying ");
      debug(unsent_counter);
      debugln(" times");

      // JUDICIOUS RETRY: Only flush HTTP stack, don't toggle bearer (CGACT)
      // unless fatal
      SerialSIT.println("AT+HTTPTERM");
      vTaskDelay(500);
      SerialSIT.println("AT+HTTPINIT");
      waitForResponse("OK", 2000);

      // Restore Parameters
      SerialSIT.print("AT+HTTPPARA=\"CID\",");
      SerialSIT.println(active_cid);
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

      // Flush Serial buffer before retry action //By ANTIGRAVITY
      while (SerialSIT.available())
        SerialSIT.read();

      success_count = send_at_cmd_data(http_data, charArray);
      vTaskDelay(200);

      if (success_count == 1) {
        debug("Success in sending current data .. Attempt : ");
        debugln(unsent_counter);

        break;
      }
    }
  } else if (success_count == 1) {
    debugln("Success in sending the current data in 1st attempt");
  }

  if (unsent_counter == 4) { // Complete failure in sending current data

    if (data_mode == eCurrentData) {
      char current_record[record_length + 1];

      debugln();
      debugln("CURRENT DATA : Retries exceeded limit... Going to store the "
              "current data and go to deep sleep mode ...");
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
      if (strstr(UNIT, "SPATIKA_GEN")) {
        snprintf(stnId, sizeof(stnId), "%s", ftp_station);
      } else {
        snprintf(stnId, sizeof(stnId), "%06d", atoi(ftp_station));
      }

      snprintf(ftpappend_text, sizeof(ftpappend_text),
               "%s;%04d-%02d-%02d,%02d:%02d;%s;%s;%s;%s;%04d;%04.1f\r\n", stnId,
               temp_year, temp_month, temp_day, temp_hr, temp_min, sample_temp,
               sample_hum, sample_avgWS, sample_WD, temp_sig, bat_val);
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
      if (strstr(UNIT, "SPATIKA_GEN")) {
        snprintf(stnId, sizeof(stnId), "%s", ftp_station);
      } else {
        snprintf(stnId, sizeof(stnId), "%06d", atoi(ftp_station));
      }

      snprintf(ftpappend_text, sizeof(ftpappend_text),
               "%s;%04d-%02d-%02d,%02d:%02d;%s;%s;%s;%s;%s;%04d;%04.1f\r\n",
               stnId, temp_year, temp_month, temp_day, temp_hr, temp_min,
               ftpsample_cum_rf, sample_temp, sample_hum, sample_avgWS,
               sample_WD, temp_sig, bat_val);
      debug("ftpappend_text is : ");
      debugln(ftpappend_text);
#endif
      vTaskDelay(100);

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
        strcpy(finalBuffer, current_record); // strcpy handles null termination
      }
      snprintf(unsent_file, sizeof(unsent_file), "/unsent.txt");
      File file2 = SPIFFS.open(unsent_file, FILE_APPEND);
      if (!file2) {
        debugln("Failed to open unsent.txt for appending");
      } // #TRUEFIX
      file2.print(finalBuffer);
      file2.close();
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
        strcpy(finalBuffer, ftpappend_text); // strcpy handles null termination
      }
      debugln("Record written into /ftpunsent.txt is as below : ");
      debugln(finalBuffer);
      snprintf(ftpunsent_file, sizeof(ftpunsent_file), "/ftpunsent.txt");
      File ftpfile2 = SPIFFS.open(ftpunsent_file, FILE_APPEND);
      if (!ftpfile2) {
        debugln("Failed to open ftpunsent.txt for appending (TWS)");
      } // #TRUEFIX
      ftpfile2.print(finalBuffer);
      ftpfile2.close();
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
        strcpy(finalBuffer, ftpappend_text); // strcpy handles null termination
      }
      debugln("Record written into /ftpunsent.txt is as below : ");
      debugln(finalBuffer);
      snprintf(ftpunsent_file, sizeof(ftpunsent_file), "/ftpunsent.txt");
      File ftpfile2 = SPIFFS.open(ftpunsent_file, FILE_APPEND);
      if (!ftpfile2) {
        debugln("Failed to open ftpunsent.txt for appending (TWS-RF)");
      } // #TRUEFIX
      ftpfile2.print(finalBuffer);
      ftpfile2.close();
#endif

      debugln("Current/Closing data not sent to unsent.txt is ");
      debugln(current_record);

    } // for unsent data it is there in send_http_data()
  }
}

void send_http_data() {

  /*
   * PREPARE HTTP PARAMS
   */

  snprintf(httpPostRequest, sizeof(httpPostRequest),
           "AT+HTTPPARA=\"URL\",\"http://%s:%s%s\"",
           httpSet[http_no].serverName, httpSet[http_no].Port,
           httpSet[http_no].Link);
  debugln("[GPRS] Starting HTTP...");
  debug("HTTP POST REQUEST IS ");
  debugln(httpPostRequest);
  vTaskDelay(50);

  // Hard reset IP stack to clear any half-open sessions
  SerialSIT.println("AT+CIPSHUT");
  waitForResponse("SHUT OK", 1000);

  // Ensure any previous NET connection is closed to avoid conflict
  SerialSIT.println("AT+NETCLOSE");
  waitForResponse("OK", 1000); // Ignore error if already closed

  SerialSIT.println("AT+HTTPTERM"); // AG1
  vTaskDelay(100);

  SerialSIT.println("AT+HTTPINIT");
  response = waitForResponse("OK", 5000);
  vTaskDelay(100);

  // Restore CID Setting (Quiet)
  SerialSIT.print("AT+HTTPPARA=\"CID\",");
  SerialSIT.println(active_cid);
  response = waitForResponse("OK", 1000);

  SerialSIT.println("AT+HTTPPARA=\"ACCEPT\",\"*/*\"");
  response = waitForResponse("OK", 1000);

  if (!strcmp(httpSet[http_no].Format, "json")) {
    SerialSIT.println(httpPostRequest);
    response = waitForResponse("OK", 100);
    SerialSIT.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
    response = waitForResponse("OK", 100);
    debugln("It is json");
  } else {
    SerialSIT.println(httpPostRequest);
    response = waitForResponse("OK", 100);
    SerialSIT.println(
        "AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"");
    response = waitForResponse("OK", 100);
    response = waitForResponse("OK", 100);
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

  if (unsent_counter != 6) { // Success in sending current data ... Continue
                             // only if the current data was sent to server
    // Storing last logged data first before proceeding to unsent data
    File fileTemp2 = SPIFFS.open("/signature.txt", FILE_WRITE);
    if (!fileTemp2) {
      debugln("Failed to open signature.txt for writing");
    } // #TRUEFIX
    snprintf(signature, sizeof(signature), "%04d-%02d-%02d,%02d:%02d",
             current_year, current_month, current_day, record_hr, record_min);
    fileTemp2.print(signature);
    fileTemp2.close();
    debugln();
    debug("**** Storing Last Logged Data as ");
    debugln(signature);

#if SYSTEM == 0

    /*
     * SENDING 8:30 as well as UNSENT DATA IF FILE EXISTS ...
     */
    snprintf(unsent_file, sizeof(unsent_file), "/unsent.txt");
    debugln(unsent_file);

    if (sampleNo == 1 || sampleNo == 3) {
      data_mode = eClosingData;
      prepare_data_and_send();
    }

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
            debug("unsent_pointer_count in unsent_pointer.txt is updated to ");
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
    send_unsent_data(); // for TWS and TWS-ADDON
#endif

  } else { // Only if the current data is sent, enter this loop above.
    debug("*** Current data couldnt be sent after 5 trials... Only if FTP data "
          "for TWS/TWSRF and it is in 1 hr cycle will it be sent ");
    debugln();
  }

  //  CHECK FOR SMS
  sync_mode = eSMSStart;
  send_sms();

} // end of send_http_data

void send_unsent_data() { // ONLY FOR TWS AND TWS-ADDON
  debugln("Entering FTP mode and checking if data period is correct for "
          "sending and if there is any file to be sent");
  int ftp_year = rf_cls_yy % 100;
  char fileName[50];

#if SYSTEM == 1
  // TWS_003655_250818_220000.kwd
  // 003655;2025-08-18,19:45;+21.5;099.9;00.0;023;-079;13.13
  // 00,2024-05-21,08:45,00.00,00.00,00.00,000,-111,00.0
  snprintf(fileName, sizeof(fileName), "/TWS_%s_%02d%02d%02d_%02d%02d00.kwd",
           ftp_station, ftp_year, rf_cls_mm, rf_cls_dd, record_hr, record_min);
#endif

#if SYSTEM == 2
  // TWSRF_000266_250818_170000.kwd
  // 000266;2025-08-18,09:30;000.0;+23.1;097.0;00.1;349;-079;13.35
  // 00,2024-05-21,08:45,0000.0,00.00,00.00,00.00,000,-111,00.0
  if (strstr(UNIT, "SPATIKA_GEN"))
    snprintf(fileName, sizeof(fileName),
             "/TWSRF_%s_%02d%02d%02d_%02d%02d00.swd", ftp_station, ftp_year,
             rf_cls_mm, rf_cls_dd, record_hr, record_min);
  else
    snprintf(fileName, sizeof(fileName),
             "/TWSRF_%s_%02d%02d%02d_%02d%02d00.kwd", ftp_station, ftp_year,
             rf_cls_mm, rf_cls_dd, record_hr, record_min);
#endif

  // 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,0,1,2,3,4,5,6,7,8 -- Unsent
  // data
  if (sampleNo == 1 || sampleNo == 5 || sampleNo == 9 || sampleNo == 13 ||
      sampleNo == 17 || sampleNo == 21 || sampleNo == 25 || sampleNo == 29 ||
      sampleNo == 33 || sampleNo == 37 || sampleNo == 41 || sampleNo == 45 ||
      sampleNo == 49 || sampleNo == 53 || sampleNo == 57 || sampleNo == 61 ||
      sampleNo == 65 || sampleNo == 69 || sampleNo == 73 || sampleNo == 77 ||
      sampleNo == 81 || sampleNo == 85 || sampleNo == 89 || sampleNo == 93 ||
      sampleNo == 95) {
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
      copyFile(ftpunsent_file, fileName); // copyFile(source,destination);
      debug("Retrieved file is ");
      debugln(fileName);
      esp_task_wdt_reset();
      send_ftp_file(fileName);
    } else {
      debugln("No ftpunsent.txt found. Skipping FTP.");
    }
  }

  if (sampleNo == 3 ||
      sampleNo ==
          7) { // send previous day's data (96 data) if available through FTP
    if (sampleNo == 0) {
      snprintf(ftpunsent_file, sizeof(ftpunsent_file), "/ftpunsent.txt");
      SPIFFS.remove(ftpunsent_file);
      debug("Removed unsent file at beginning of the day : ");
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
  int s = 0, fileSize = 0; // Fixed shadowed and uninitialized local variables
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
        waitForResponse("OK", 9000);
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
        response = waitForResponse("OK", 9000);
        debug("Response of AT+CFTPSLOGOUT is ");
        debugln(response);
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

        esp_task_wdt_reset();

        snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
                 "AT+CFTPSPUTFILE=\"%s\",1", modulePath);
        debugln("About to send CFTPSPUTFILE ...");
        SerialSIT.println(gprs_xmit_buf); // FTP client context
        response1 = waitForResponse("+CFTPSPUTFILE: 0",
                                    180000); // 180s timeout
        debug("Response of AT+CFTPSPUTFILE ");
        debugln(response1);

        if (response1.indexOf("+CFTPSPUTFILE: 0") != -1) {

          if (sampleNo == 0 || sampleNo == 3 || sampleNo == 7 ||
              sampleNo ==
                  11) { // If FTP is successful, remove the dailyftp file.
            SPIFFS.remove(temp_file);
            debug("Removed : ");
            debugln(temp_file);
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
          SPIFFS.remove(ftpunsent_file);

          // Success Cleanup: Remove the temporary file from cellular module
          // storage
          snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSDEL=\"C:/%s\"",
                   modulePath);
          SerialSIT.println(gprs_xmit_buf);
          waitForResponse("OK", 5000);
        } else {
          debugln("Did not succeed in FTP ..");
          // Failed: Clean up ONLY the temporary .swd/.kwd copies.
          // DO NOT delete 'temp_file' or 'ftpunsent_file' (Source Data) so we
          // can retry later.

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
        response = waitForResponse("OK", 9000);
        debug("Response of AT+CFTPSLOGOUT is ");
        debugln(response);
        vTaskDelay(200);

        snprintf(
            gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSDEL=\"C:/%s\"",
            fileName); // 0 : if the file does not exist, it will be created
        SerialSIT.println(gprs_xmit_buf);
        response = waitForResponse("OK", 10000);
        debug("Response of AT+FSDEL is  ");
        debugln(response);

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

  vTaskDelay(200);

  debugln("Turning off GPRS Module");
  SerialSIT.println("AT+CPOF");
  response = waitForResponse("OK", 9000);
  debug("Response of AT+CPOF is ");
  debugln(response);
  vTaskDelay(200);

  sync_mode = eSMSStop;
}

int send_at_cmd_data(char *payload, String charArray) {
  esp_task_wdt_reset();
  int i;
  for (i = 0; payload[i] != '\0'; ++i)
    ;
  snprintf(ht_data, sizeof(ht_data), "AT+HTTPDATA=%d,4000", i);

  debugf1("Payload is %s", payload);
  debugln(); //    debugln("Length is " + String(i));
  vTaskDelay(100);
  SerialSIT.println(ht_data);
  // Wait for DOWNLOAD prompt (typically > instead of OK) to speed up sending
  response = waitForResponse("DOWNLOAD", 2000);
  SerialSIT.println(payload);
  response = waitForResponse("OK", 4000);

  SerialSIT.println("AT+HTTPACTION=1");
  waitForResponse("OK", 2000); // Consume immediate OK
  response = waitForResponse("+HTTPACTION:", 25000);
  debug("Response of AT+HTTPACTION=1 is ");
  debugln(response);

  if (response.indexOf("706") != -1) {
    debugln("HTTP Stack Busy (706). Clearing...");
    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 1000);
    return 0;
  }

  SerialSIT.println("AT+HTTPREAD=0,290");
  response = waitForResponse("+HTTPREAD: 0", 10000);
  debug("Response of AT+HTTPREAD=0,290 is ");
  debugln(response);
  const char *char_resp;

  if (!strcmp(httpSet[http_no].Format,
              "json")) { // BIHAR : if json then this loop otherwise goto
                         // urlencoded one
                         //          debugln("FORMAT : json");
    char_resp = response.c_str();
    if (strstr(char_resp, "success")) {
      debugln("GPRS SEND : It is a Success");
      return (1);
    } else {
      debugln("GPRS SEND : It is a Failure");
      return (0);
      vTaskDelay(1000);
    }
  } else { // KSNDMC
    //          debugln("FORMAT : urlencoded");
    char_resp = response.c_str();
    if (strstr(char_resp, "Success")) {
      debugln("GPRS SEND : It is a Success");
      return (1);
    } else {
      debugln("GPRS SEND : It is a Failure");
      return (0);
      vTaskDelay(1000);
    }
  }
}

void store_current_unsent_data() {
  vTaskDelay(100);
  char finalStringBuffer[100];
  strcpy(finalStringBuffer, store_text); // strcpy handles null termination

#if SYSTEM == 0
  snprintf(unsent_file, sizeof(unsent_file), "/unsent.txt");
  File file2 = SPIFFS.open(unsent_file, FILE_APPEND);
  if (!file2) {
    debugln("Failed to open unsent.txt for appending (store_current)");
  } // #TRUEFIX
  file2.print(finalStringBuffer);
  file2.close();
  debug("************************");
  debug("Storing data in unsent file due to SIM issue/REG issue/Signal issue ");
  debugln("************************");
  debug("unsent store_text is ");
  debugln(finalStringBuffer);
#endif

#if (SYSTEM == 1 || SYSTEM == 2)
  snprintf(ftpunsent_file, sizeof(ftpunsent_file), "/ftpunsent.txt");
  File ftpfile2 = SPIFFS.open(ftpunsent_file, FILE_APPEND);
  if (!ftpfile2) {
    debugln("Failed to open ftpunsent.txt for appending (store_current)");
  } // #TRUEFIX
  ftpfile2.print(finalStringBuffer);
  ftpfile2.close();
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

  while (((signal_lvl == 0) || (signal_lvl == -113)) &&
         (retries < 120)) { // ULTRA-BSNL-SAFE: 120 loops @ 500ms = 60s timeout
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
    }
    retries++;
    vTaskDelay(500 / portTICK_PERIOD_MS); // High-frequency polling
  }
  if (retries >= 120) {
    debugln("Weak signal, Retries crossed the limit for getting Signal "
            "Strength ..."); // SIM is bad
    signal_strength = -(random(125, 130 + 1));
    signal_lvl = signal_strength;
    gprs_mode = eGprsSignalForStoringOnly;
    // Update UI variables so LCD doesn't stay stuck on "FETCHING..."
    strcpy(carrier, "NO SIGNAL");
    strcpy(reg_status, "WEAK SIGNAL");
  }

  debug("gprs_mode inside loop of get_signal_strength is  ");
  debugln(gprs_mode); // TRG8-3.0.5
  debug("sync_mode inside loop of get_signal_strength is  ");
  debugln(sync_mode); // TRG8-3.0.5
}

/**
 * Attempts to fetch SIM number via USSD as a secondary fallback
 */
String fetch_number_ussd() {
  String codes[3] = {"", "", ""};
  if (strstr(carrier, "Airtel")) {
    codes[0] = "*282#";
    codes[1] = "*121*9#";
    codes[2] = "*121*1#";
  } else if (strstr(carrier, "Jio")) {
    codes[0] = "*1#";
  } else if (strstr(carrier, "BSNL")) {
    codes[0] = "*222#";
    codes[1] = "*1#";
    codes[2] = "*555#";
  } else if (strstr(carrier, "Vi")) {
    codes[0] = "*199#";
    codes[1] = "*111*2#";
  }

  for (int c = 0; c < 3; c++) {
    if (codes[c] == "")
      continue;

    debug("[USSD] Trying ");
    debugln(codes[c]);
    SerialSIT.print("AT+CUSD=1,\"");
    SerialSIT.print(codes[c]);
    SerialSIT.println("\""); // Removed ,15 for better compatibility

    String ussdResp = waitForResponse("+CUSD:", 8000);
    if (ussdResp != "") {
      // Look for 10 digit number pattern (9, 8, 7, 6 prefix)
      int digitCount = 0;
      String num = "";
      for (int i = 0; i < ussdResp.length(); i++) {
        char ch = ussdResp.charAt(i);
        if (isdigit(ch)) {
          num += ch;
          digitCount++;
        } else if (digitCount > 0) {
          if (digitCount == 10) {
            char first = num.charAt(0);
            if (first >= '6' && first <= '9')
              return num; // Return 10-digit number without +91 prefix
          } else if (digitCount == 12 && num.startsWith("91")) {
            return num.substring(2); // Strip "91" prefix, return 10 digits
          }
          num = "";
          digitCount = 0;
        }
      }
    }
  }
  return "";
}

void get_network() {
  debugln();
  debugln("************************");
  debugln("GETTING NETWORK ");
  debugln("************************");

  // SMART CACHE LOGIC: Check if SIM is the same (Very fast check)
  extern String get_ccid();
  String current_iccid = get_ccid();

  if (current_iccid != "" && current_iccid == String(cached_iccid) &&
      String(sim_number) != "NA") {
    debugln("[CACHE] Same SIM detected. Using cached carrier/number to save "
            "power.");
    // APN still needs to be determined for the modem to connect
    // (apn_str is NOT in RTC because it must be set every power-on)
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

  // Get Number (Universal for all carriers)
  SerialSIT.println("AT+CNUM");
  String cnumResp = waitForResponse("+CNUM", 1000);

  int plusIndex = cnumResp.indexOf("+91");
  if (plusIndex != -1) {
    // Find the end of the number. The number usually ends with a quote,
    // comma, or newline
    int endIndex = -1;
    for (int k = plusIndex; k < cnumResp.length(); k++) {
      char c = cnumResp.charAt(k);
      if (c != '+' && (c < '0' || c > '9')) {
        endIndex = k;
        break;
      }
    }

    if (endIndex == -1)
      endIndex = cnumResp.length();

    String number = cnumResp.substring(plusIndex, endIndex);
    debug("Extracted Number: ");
    debugln(number);

    // Strip +91 prefix if present (keep only 10-digit mobile number)
    if (number.startsWith("+91") && number.length() == 13) {
      number = number.substring(3); // Remove "+91"
    } else if (number.startsWith("91") && number.length() == 12) {
      number = number.substring(2); // Remove "91"
    }

    strcpy(sim_number, number.c_str());

    // Airtel M2M Logic
    if ((strstr(r1, "airtel") || strstr(r2, "airtel")) &&
        number.length() > 13) {
      strcpy(apn_str, "airteliot.com");
      debugln("Detected 13 digit M2M card, using airteliot.com");
    }
  } else {
    // CNUM failed, try USSD first
    debugln("CNUM failed. Trying USSD discovery...");
    String ussdNum = fetch_number_ussd();
    if (ussdNum != "") {
      debug("USSD Success: ");
      debugln(ussdNum);
      strcpy(sim_number, ussdNum.c_str());
    } else {
      // USSD failed, try IMSI as final fallback
      debugln("USSD failed. Trying IMSI...");
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
        for (int i = imsiStart; i < imsiResp.length() && imsi.length() < 15;
             i++) {
          char c = imsiResp.charAt(i);
          if (c >= '0' && c <= '9') {
            imsi += c;
          } else if (imsi.length() > 0) {
            break; // Stop at first non-digit after digits started
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
}

void get_registration() {
  esp_task_wdt_reset();
  debugln();
  debugln("************************");
  debugln("GETTING REGISTRATION ");
  debugln("************************");

  int no_of_retries = 60; // 60 Retries for ultra-slow registration
  registration = 0;
  retries = 0;
  bool is_registered = false;

  // Enable registration reporting ONCE (not every retry - prevents BSNL issues)
  SerialSIT.println("AT+CREG=1");
  waitForResponse("OK", 500);

  // Diagnostic: Check IMEI (helps identify IMEI lock issues)
  SerialSIT.println("AT+GSN");
  String imei_response = waitForResponse("OK", 1000);
  if (imei_response.length() > 0) {
    debugln("[DIAG] IMEI: " + imei_response);
  }

  // Diagnostic: Check current network operator
  SerialSIT.println("AT+COPS?");
  String cops_response = waitForResponse("OK", 1000);
  debugln("[DIAG] Network: " + cops_response);

  while (!is_registered && (retries < no_of_retries)) {
    esp_task_wdt_reset();

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
            strcpy(reg_status,
                   (registration == 1) ? "GPRS:Home:OK" : "GPRS:Roam:OK");
            is_registered = true;
            break;
          }
        }
      }
    }

    if (!is_registered) {
      // 3. Check CREG (GSM) - CRITICAL FOR AIRTEL IN GSM-ONLY AREAS
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
          if (registration == 1 || registration == 5 || registration == 11) {
            debugln("CREG Registered (GSM)");
            strcpy(reg_status,
                   (registration == 1)
                       ? "GSM:Home:OK"
                       : (registration == 11 ? "GSM:LTE:EXT" : "GSM:Roam:OK"));
            is_registered = true;
            break;
          }
        }
      }
    }

    if (!is_registered) {
      // Periodic Modem Wakeup every 5 tries
      if (retries % 5 == 0 && retries > 0) {
        SerialSIT.println("AT+CGACT=1,1");
        vTaskDelay(500);
      }
      debugf2("Reg Search... Status:%d Iter:#%d\n", registration, retries + 1);
      vTaskDelay(10000); // Restored to 10s for BSNL reliability
      retries++;
    }
  }

  // Update Diagnostics
  int time_taken = retries * 10; // Approx seconds (10s delay per retry)
  if (is_registered) {
    diag_reg_time_total += time_taken;
    diag_reg_count++;
    if (time_taken > diag_reg_worst)
      diag_reg_worst = time_taken;

    gprs_mode = eGprsSignalOk;
    debugln("Registration Successful.");
  } else {
    diag_gprs_fails++;
    gprs_mode = eGprsSignalForStoringOnly;
    debugln("Registration failed after all retries.");
    strcpy(reg_status, "REG FAIL");
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
  // If 1 is active, we assume it's correct (Auto-APN).
  if (response.indexOf("+CGACT: 1,1") != -1) {
    active_cid = 1;
    debugln("Context 1 is already active. Using CID 1.");
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
    // No context active. Configure CID 1.
    active_cid = 1;

    // JUDICIOUS STABILIZATION: Allow modem stack to settle after Registration
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    String ccid = get_ccid();
    char stored_apn[20] = {0}; // Match global apn_str size

    // 1. Try Stored APN
    if (load_apn_config(ccid, stored_apn, sizeof(stored_apn))) {
      debugln("Smart APN: Trying Stored -> " + String(stored_apn));
      if (try_activate_apn(stored_apn)) {
        strncpy(apn_str, stored_apn, sizeof(apn_str) - 1);
        return; // Success!
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

    debugln("APN: All attempts failed.");
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
      ftp_login_flag = setup_ftp();
      if (ftp_login_flag == 1) {
        fetchFromFtpAndUpdate(temp1);
      } else {
        debugln("FTP Login unsuccessful");
      }
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

  //        if((strstr(UNIT, "KSNDMC_OLD")) || (strstr(UNIT, "KSNDMC_TRG")) ||
  //        (strstr(UNIT, "KSNDMC_TWS"))){
  //                  strcpy(NETWORK,"KSNDMC");
  //                  vTaskDelay(300);
  //                  debug("NETWORK is ");debugln(NETWORK);//TRG8-3.0.5
  //        } else if((strstr(UNIT, "BIHAR_ORG")) || (strstr(UNIT,
  //        "BIHAR_TEST"))){
  //                  strcpy(NETWORK,"DES-BIH");
  //                  vTaskDelay(300);
  //                  debug("NETWORK is ");debugln(NETWORK);//TRG8-3.0.5
  //        }

  strcpy(status_response, ""); // NEW POST TRG8-3.0.5

  // RF & TWS
  snprintf(
      status_response, sizeof(status_response),
      "%s,%s,%s,%s,%04d-%02d-%02dT%02d:%02d,%s,15,%04d,%04.1f,0.0,%s,%s,%s,"
      "%04d-%02d-%02d,%04d-%02d-%02dT%02d:%02d,0\r\n\032",
      NETWORK, STATION_TYPE, msg_type, station_name, current_year,
      current_month, current_day, current_hour, current_min, UNIT_VER,
      signal_strength, bat_val, (solar_conn ? "Y" : "N"),
      (sd_card_ok ? "SDC-OK" : "SDC-FAIL"),
      (calib_sts == 1 ? "CLB-OK" : "CLB-FAIL"),
      calib_year,  // calib
      calib_month, // calib
      calib_day,   // calib
      last_recorded_yy, last_recorded_mm, last_recorded_dd, last_recorded_hr,
      last_recorded_min);

  debug("Status response prepared is ");
  debugln(status_response);
  // first give send sms command

  // iter14
  //   SerialSIT.println("AT+CSMS=0");
  //   response = waitForResponse("+CSMS", 5000);
  //   debug("Response of AT+CSMS=0 is ");debugln(response);

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
  vTaskDelay(1000);
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
  sscanf(csqstr, "+CLBS: %d,%f,%f,", &tmp, &lati,
         &longi); // Response for AT+CLBS=1 +CLBS: 0,12.989436,77.537910,550

  vTaskDelay(500);
  debug("Latitude is : ");
  debugln(lati);
  debug("Longitude is : ");
  debugln(longi);

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
  SerialSIT.println("AT+CFTPSSTART");
  waitForResponse("+CFTPSSTART: 0", 10000);
  debugln(response);

  SerialSIT.println("AT+CFTPSSINGLEIP=1"); // FTP client context
  waitForResponse("OK", 5000);

  SerialSIT.println(
      "AT+CFTPSPASV=1"); // Force Passive Mode (essential for NAT/Cellular)
  waitForResponse("OK", 5000);

  SerialSIT.println(gprs_xmit_buf);
  response = waitForResponse("+CFTPSLOGIN:", 25000);
  debugln(response);

  rssiIndex = response.indexOf("+CFTPSLOGIN:"); // int
  if (rssiIndex != -1) {                        // #TRUEFIX
    rssiIndex += 13;
    rssiStr = response.substring(rssiIndex, rssiIndex + 2); // String
  } else {
    rssiStr = "0";
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

void fetchFromFtpAndUpdate(char *fileName) {

  int handle_no;
  char gprs_xmit_buf[300];
  const char *ftpCharArray;
  char *csqstr;
  int success;
  const char *response_char;
  int total_no_of_bytes;

  //      SerialSIT.println("ATE0");
  //      response = waitForResponse("OK", 3000);
  //      debug("HTTP response of ATE0: ");debugln(response);

  SerialSIT.println("AT+CFTPSLIST=\"/\"");
  response = waitForResponse("+CFTPSLIST: 0", 10000);
  //      debug("Response of AT+CFTPSLIST ");debugln(response);

  SerialSIT.println("AT+FSCD=C:/");
  response = waitForResponse("OK", 10000);
  //      debug("Response of AT+FSCD ");debugln(response);

  SerialSIT.println("AT+FSDEL=\"firmware.bin\"");
  response = waitForResponse("OK", 10000);
  //      debug("Response of AT+FSDEL of firmware.bin is ");debugln(response);

  SerialSIT.println("AT+FSLS");
  response = waitForResponse("OK", 10000);
  //      debug("Response of AT+FSLS ");debugln(response);

  debugln("Starting to download ..");
  debug("Time is : ");
  debug(current_hour);
  debug(':');
  debugln(current_min);

  esp_task_wdt_reset();

  SerialSIT.println("AT+CFTPSGETFILE=\"firmware.bin\"");  // FTP client context
  response = waitForResponse("+CFTPSGETFILE: 0", 900000); // 240000
  debug("Response of AT+CFTPSGETFILE ");
  debugln(response);

  esp_task_wdt_reset();

  debugln("Download completed ..");
  debug("Time is : ");
  debug(current_hour);
  debug(':');
  debugln(current_min);

  SerialSIT.println("AT+FSATTRI=\"firmware.bin\"");
  response = waitForResponse("+FSATTRI:", 10000);
  vTaskDelay(200);
  debug("Response of AT+FSATTRI is ");
  debugln(response);
  vTaskDelay(200);
  response_char = response.c_str();
  vTaskDelay(200);
  csqstr = strstr(response_char, "+FSATTRI");
  if (csqstr == NULL) {
    debugln("Error: +FSATTRI not found in response");
    return; // Exit if response is invalid
  }
  sscanf(csqstr, "+FSATTRI: %d", &total_no_of_bytes);
  debug("FileSize of firmware.bin is ");
  debugln(total_no_of_bytes);

  /*
    Logic to read from FS and put it in SPIFFS card
   */
  debugln("Starting to copy ..");
  debug("Time is : ");
  debug(current_hour);
  debug(':');
  debugln(current_min);
  debugln();

  int balance_to_xfer = total_no_of_bytes;
  int buf_size = 8192;
  int no_of_bytes_processed;
  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSOPEN=C:/%s,2\r\n",
           fileName); // 0 : append mode
  debug("FSOPEN command is ");
  debugln(gprs_xmit_buf);
  SerialSIT.println(gprs_xmit_buf);
  response = waitForResponse("+FSOPEN", 5000);
  response_char = response.c_str();
  delay(500);
  csqstr = strstr(response_char, "+FSOPEN:");
  if (csqstr == NULL) {
    debugln("Error: +FSOPEN not found in response");
    return; // Exit if response is invalid
  }
  sscanf(csqstr, "+FSOPEN: %d,", &handle_no);

  if (SPIFFS.exists("/firmware.bin")) {
    File sp2 = SPIFFS.open("/firmware.bin",
                           FILE_READ); // spiffs : Open new for APPEND
    int fileSize = sp2.size();
    debug("SPIFFS File size is ");
    debugln(fileSize);
    sp2.close();
    SPIFFS.remove("/firmware.bin");
  }

  File sp1 = SPIFFS.open("/firmware.bin",
                         FILE_APPEND); // SPIFFS : Open new for APPEND

  uint8_t binBytes[buf_size];
  int i;
  while (balance_to_xfer > 0) {
    if (balance_to_xfer > buf_size)
      no_of_bytes_processed = buf_size;
    else
      no_of_bytes_processed = balance_to_xfer;

    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSREAD=%d,%d\r\n",
             handle_no, no_of_bytes_processed);
    SerialSIT.println(gprs_xmit_buf);
    response = waitForResponse("OK", 2000);

    response_char = response.c_str();
    csqstr = strstr(response_char, "CONNECT"); // This pointer will point to "C"
    if (csqstr == NULL) {
      debugln("Error: CONNECT not found in response");
      break; // Exit loop if response is invalid
    }

    while (*csqstr != '\0' && *csqstr != '\n')
      csqstr++; // To point to the next line after CONNECT <NUMBER>\r\n
    if (*csqstr == '\n')
      csqstr++; // Skip the newline character

    for (i = 0; i < no_of_bytes_processed && *csqstr != '\0'; i++) {
      binBytes[i] = *csqstr++;
    }

    sp1.write(binBytes, no_of_bytes_processed);
    balance_to_xfer -= no_of_bytes_processed;
  }

  sp1.close();
  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSCLOSE=%d\r\n",
           handle_no); // fileHandle,length,timeout in secs
  SerialSIT.println(gprs_xmit_buf);
  response = waitForResponse("+FSCLOSE", 5000);

  File sp2 =
      SPIFFS.open("/firmware.bin", FILE_READ); // spiffs : Open new for APPEND
  int fileSize = sp2.size();
  debug("SPIFFS File size is ");
  debugln(fileSize);
  sp2.close();
  debugln("End of copy ..");
  debug("Time is : ");
  debug(current_hour);
  debug(':');
  debugln(current_min);
  debugln();

  SerialSIT.println("AT+CMGD=1,4");
  response = waitForResponse("OK", 5000);
  debugln("Removed READ messages");

  vTaskDelay(200);

  debugln("Turning off GPRS Module");
  SerialSIT.println("AT+CPOF");
  response = waitForResponse("OK", 9000);
  debug("Response of AT+CPOF is ");
  debugln(response);

  ESP.restart();
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
      response = waitForResponse("OK", 9000);
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

/**
 * Sends a health report to the Google Sheets "Server"
 * Parameters: stn, type, gbat, ebat, sig
 */
bool send_health_report(bool useJitter) {
  // COLLISION PREVENTION: Only used for automated 15-min wakeups
  if (useJitter) {
    int jitter = random(0, 5000);
    vTaskDelay(jitter / portTICK_PERIOD_MS);
  }

  // 1. Ensure we have the latest GPS (Load from SPIFFS if no live fix)
  if (lati == 0 || longi == 0) {
    debugln("[Health] No GPS coordinates in memory. Attempting to load from "
            "cache...");
    loadGPS();

    // Still no GPS after loading from cache?
    if (lati == 0 || longi == 0) {
      debugln("[Health] No cached GPS. Attempting fresh CLBS fix...");
      get_gps_coordinates(); // Try to get fresh GPS via CLBS

      // Final check after CLBS attempt
      if (lati == 0 || longi == 0) {
        debugln(
            "[Health] ⚠️ WARNING: All GPS attempts failed. Sending 0,0");
        debugln("[Health] → CLBS service may be unavailable or disabled");
      } else {
        debugln("[Health] ✅ Fresh GPS fix obtained via CLBS");
      }
    } else {
      debugln("[Health] ✅ GPS loaded from cache");
    }
  }

  // 2. Prepare Calibration String (Date + Status)
  char clbInfo[25];
  if (SYSTEM == 0 || SYSTEM == 2) {
    snprintf(clbInfo, sizeof(clbInfo), "%02d-%02d-%02d_%s", calib_day,
             calib_month, calib_year, (calib_sts == 1 ? "PASS" : "FAIL"));
  } else {
    strcpy(clbInfo, "NA");
  }

  // 3. Cleanup Station Name (Trim trailing spaces, then URL-safe)
  char cleanStn[16];
  strncpy(cleanStn, station_name, 15);
  cleanStn[15] = '\0';

  // Trim trailing spaces
  int len = strlen(cleanStn);
  while (len > 0 && cleanStn[len - 1] == ' ') {
    cleanStn[len - 1] = '\0';
    len--;
  }

  // Replace remaining spaces with underscores for URL
  for (int i = 0; i < strlen(cleanStn); i++)
    if (cleanStn[i] == ' ')
      cleanStn[i] = '_';

  // 4. Build Expanded URL with Proper Headings (v5.2 Diagnosis)
  float avg_reg =
      (diag_reg_count > 0) ? (float)diag_reg_time_total / diag_reg_count : 0;

  size_t spiffs_used = SPIFFS.usedBytes();
  size_t spiffs_total = SPIFFS.totalBytes();

  // Plain English Status Summary Logic (v5.2 - SD is Optional)
  const char *status_summary = "SYSTEM_OK";
  if (li_bat_val < 3.4)
    status_summary = "CRITICAL_BATTERY";
  else if (diag_last_reset_reason == 15)
    status_summary = "BROWNOUT_RECOVERY";
  else if (diag_rtc_battery_ok == false)
    status_summary = "RTC_BATTERY_FAULT";
  else if (li_bat_val < 3.6)
    status_summary = "LOW_BATTERY";
  else if (current_hour >= 10 && current_hour <= 16 &&
           solar_val < (li_bat_val + 2.0))
    status_summary = "SOLAR_CHARGING_FAULT";
  else if (diag_gprs_fails > 0)
    status_summary = "NETWORK_STABILITY_ISSUE";
  else if (diag_reg_worst > 150)
    status_summary = "POOR_SIGNAL_LOCATION";
  else if (spiffs_used > (spiffs_total * 0.9))
    status_summary = "MEMORY_NEAR_FULL";
  else if (diag_last_reset_reason >= 7 && diag_last_reset_reason <= 9)
    status_summary = "WATCHDOG_RESET";

  char healthURL[850]; // Increased for Status info
  snprintf(
      healthURL, sizeof(healthURL),
      "AT+HTTPPARA=\"URL\",\"%s?Station=%s&SystemType=%d&UnitName=%s&"
      "Version=%s&SolarGprsBat=%0.2f&"
      "Esp32Bat=%0.2f&Signal=%d&SimNo=%s&Carrier=%s&ICCID=%s&Calibration=%s&"
      "Latitude=%.6f&Longitude=%.6f&Reset=%d&RegAvg=%0.1f&RegWorst=%d&RegFails="
      "%d&Uptime=%d&SpiffsUsed=%u&SpiffsTotal=%u&SDOk=%d&RTCOk=%d&HealthStatus="
      "%s\"",
      GOOGLE_HEALTH_URL, cleanStn, SYSTEM, UNIT, UNIT_VER, solar_val,
      li_bat_val, signal_strength, sim_number, carrier, cached_iccid, clbInfo,
      lati, longi, diag_last_reset_reason, avg_reg, diag_reg_worst,
      diag_gprs_fails, (int)diag_total_uptime_hrs, (unsigned int)spiffs_used,
      (unsigned int)spiffs_total, sd_card_ok, diag_rtc_battery_ok,
      status_summary);

  debugln("[Health] Sending Detailed Report...");
  flushSerialSIT(); // Clear any stale data

  // 1. CLOCK SYNC (Crucial for SSL stability)
  char cclk_cmd[50];
  snprintf(cclk_cmd, sizeof(cclk_cmd),
           "AT+CCLK=\"%02d/%02d/%02d,%02d:%02d:%02d+22\"", current_year % 100,
           current_month, current_day, current_hour, current_min, current_sec);
  SerialSIT.println(cclk_cmd);
  waitForResponse("OK", 500);

  // 2. SSL/HTTPS Setup
  SerialSIT.println("AT+CSSLCFG=\"sslversion\",0,4"); // TLS 1.2
  waitForResponse("OK", 200);
  SerialSIT.println("AT+CSSLCFG=\"authmode\",0,0");
  waitForResponse("OK", 200);
  SerialSIT.println("AT+CSSLCFG=\"ignorertctime\",0,1");
  waitForResponse("OK", 200);
  SerialSIT.println("AT+CSSLCFG=\"sni\",0,\"script.google.com\"");
  waitForResponse("OK", 200);

  // 3. HTTP Action
  SerialSIT.println("AT+HTTPTERM");
  waitForResponse("OK", 500);
  vTaskDelay(200 / portTICK_PERIOD_MS); // Extra breath for stack cleanup
  SerialSIT.println("AT+HTTPINIT");
  waitForResponse("OK", 500);

  SerialSIT.print("AT+HTTPPARA=\"CID\",");
  SerialSIT.println(active_cid);
  waitForResponse("OK", 200);

  SerialSIT.println("AT+HTTPPARA=\"UA\",\"Mozilla/5.0 (A7672S)\"");
  waitForResponse("OK", 200);
  SerialSIT.println(
      "AT+HTTPPARA=\"REDIR\",0"); // Fastest: Don't follow jumps (Learning v7)
  waitForResponse("OK", 200);
  SerialSIT.println("AT+HTTPPARA=\"SSLCFG\",0");
  waitForResponse("OK", 200);

  SerialSIT.println(healthURL);
  waitForResponse("OK", 1000);

  debugln("Updating Google Sheet...");
  SerialSIT.println("AT+HTTPACTION=0");
  waitForResponse("OK", 500);
  String response =
      waitForResponse("+HTTPACTION", 35000); // 35s for SSL/DNS on weak signal

  // Google Sheets updates even if it returns a 302 Redir
  bool success = false;
  if (response.indexOf(",200,") != -1 || response.indexOf(",302,") != -1) {
    debugln("[Health] Update Success.");
    // Update Internal LCD Markers
    last_recorded_hr = current_hour;
    last_recorded_min = current_min;
    last_recorded_dd = current_day;
    last_recorded_mm = current_month;
    last_recorded_yy = current_year;
    snprintf(last_logged, sizeof(last_logged), "%d-%d-%d,%02d:%02d",
             last_recorded_yy, last_recorded_mm, last_recorded_dd,
             last_recorded_hr, last_recorded_min);
    strcpy(ui_data[FLD_LAST_LOGGED].bottomRow, last_logged);
    success = true;
  } else {
    debugln("[Health] Update Failed: " + response);
    success = false;
  }

  SerialSIT.println("AT+HTTPTERM");
  waitForResponse("OK", 500);
  vTaskDelay(500); // Settling delay

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
