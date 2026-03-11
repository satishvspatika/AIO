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

// v5.46: Parse "tm":"YYMMDDHHMMSS" from server JSON, convert UTC→IST, correct
// RTC. force=true: apply even on first call (rejection path) force=false: apply
// only once per day if drift > 90s (success path)
void sync_rtc_from_server_tm(const char *payload, bool force) {
  String response;
  if (!payload)
    return;

  // Reset daily sync flag at midnight each new day
  static int last_sync_day = -1;
  if (current_day != last_sync_day) {
    rtc_daily_sync_done = false;
    last_sync_day =
        current_day; // M1 FIX: was never updated — reset fired every wake cycle
  }

  // v7.87: Correct at midnight or first successful check-in of each day.
  // force=true for Rejection path (always syncs).
  // force=false for Success path (syncs only once per day).
  if (!force && rtc_daily_sync_done)
    return;

  // Find "tm":"YYMMDDHHMMSS" in payload
  const char *tm_ptr = strstr(payload, "\"tm\"");
  if (!tm_ptr)
    tm_ptr = strstr(payload, "\"tm\":");
  if (!tm_ptr) {
    debugln("[RTC-Sync] No 'tm' field in server response. Skipping.");
    return;
  }

  // Skip to the 12-digit timestamp value
  const char *colon = strchr(tm_ptr, ':');
  if (!colon)
    return;
  colon++;
  while (*colon == ' ' || *colon == '"')
    colon++;

  // Parse YYMMDDHHMMSS (12 digits)
  char ts[13];
  strncpy(ts, colon, 12);
  ts[12] = '\0';
  if (strlen(ts) < 12) {
    debugln("[RTC-Sync] Short timestamp. Skipping.");
    return;
  }

  // Decode digits
  int s_yy = (ts[0] - '0') * 10 + (ts[1] - '0');
  int s_mm = (ts[2] - '0') * 10 + (ts[3] - '0');
  int s_dd = (ts[4] - '0') * 10 + (ts[5] - '0');
  int s_hh = (ts[6] - '0') * 10 + (ts[7] - '0'); // UTC hour
  int s_mi = (ts[8] - '0') * 10 + (ts[9] - '0'); // UTC minute
  int s_ss = (ts[10] - '0') * 10 + (ts[11] - '0');

  // Sanity check raw UTC values before conversion
  if (s_mm < 1 || s_mm > 12 || s_dd < 1 || s_dd > 31 || s_hh > 23 ||
      s_mi > 59 || s_ss > 59 || s_yy < 25 || s_yy > 50) {
    debugf("[RTC-Sync] Invalid UTC timestamp: %s\n", ts);
    return;
  }

  // v5.46: Robust UTC → IST conversion handling all month/year/leap rollovers
  struct tm t_in = {0};
  t_in.tm_year = s_yy + 100; // years since 1900 (e.g., 2026 -> 126)
  t_in.tm_mon = s_mm - 1;    // months since January (0-11)
  t_in.tm_mday = s_dd;
  t_in.tm_hour = s_hh;
  t_in.tm_min = s_mi;
  t_in.tm_sec = s_ss;

  // Uses mktime() internally for perfect calendar math
  convert_gmt_to_ist(&t_in);

  // Extract the true IST values back out
  s_yy = t_in.tm_year - 100;
  s_mm = t_in.tm_mon + 1;
  s_dd = t_in.tm_mday;
  s_hh = t_in.tm_hour;
  s_mi = t_in.tm_min;
  s_ss = t_in.tm_sec;

  debugf("[RTC-Sync] Server IST = 20%02d-%02d-%02d %02d:%02d:%02d\n", s_yy,
         s_mm, s_dd, s_hh, s_mi, s_ss);

  // Calculate drift in seconds vs current RTC
  int rtc_total_sec = current_hour * 3600 + current_min * 60 + current_sec;
  int srv_total_sec = s_hh * 3600 + s_mi * 60 + s_ss;
  int drift = abs(srv_total_sec - rtc_total_sec);

  // Threshold: force path=45s, daily path=90s
  int threshold = force ? 45 : 90;
  debugf("[RTC-Sync] Drift = %ds (threshold %ds, force=%d)\n", drift, threshold,
         (int)force);

  if (drift < threshold) {
    debugln("[RTC-Sync] Drift within tolerance. No correction needed.");
    if (!force)
      rtc_daily_sync_done = true; // Mark as done even if no correction
    last_sync_day = current_day;
    return;
  }

  // Apply correction to RTC
  debugf("[RTC-Sync] Correcting RTC: %02d:%02d:%02d → %02d:%02d:%02d (drift "
         "%ds)\n",
         current_hour, current_min, current_sec, s_hh, s_mi, s_ss, drift);

  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) == pdTRUE) {
    // Use current date fields but server's IST time
    rtc.adjust(DateTime(s_yy + 2000, s_mm, s_dd, s_hh, s_mi, s_ss));
    xSemaphoreGive(i2cMutex);
  }

  // Update globals immediately so sleep calc uses correct time
  current_hour = s_hh;
  current_min = s_mi;
  current_sec = s_ss;

  if (!force) {
    rtc_daily_sync_done = true;
    last_sync_day = current_day;
  }

  debugln("[RTC-Sync] ✅ RTC corrected from server time (IST).");
}

void gprs(void *pvParameters) {
  esp_task_wdt_add(NULL);
  String response;

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
          vTaskDelay(3000 / portTICK_PERIOD_MS);
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
          vTaskDelay(2000 /
                     portTICK_PERIOD_MS); // Allow user to see coordinates

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
        vTaskDelay(5000 /
                   portTICK_PERIOD_MS); // Wait longer to let user read result

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
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        memset(ui_data[target_fld].bottomRow, 0,
               sizeof(ui_data[target_fld].bottomRow));
        strcpy(ui_data[target_fld].bottomRow, "YES ?           ");
        if (sync_mode == eSMSStart || sync_mode == eGPSStart ||
            sync_mode == eStartupGPS)
          sync_mode = eSMSStop;
      }
    }

    if (!rtcReady) {
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      continue;
    }
    if (gprs_mode == eGprsInitial) {
      vTaskDelay(100 / portTICK_PERIOD_MS);
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

          // v7.67: Health Schedule restricted to 1 Daily Window
          // Window 1 → 11:00 AM: Primary report (after FTP/backlogs clear,
          //                       all of yesterday's data accounted for).
          // Within the hour window, we retry every 15-min until success.
          // 'health_last_sent_day' is only updated on SUCCESS so retries
          // happen automatically until the window closes at the next hour.
          bool is_health_time = false;
          if (TEST_HEALTH_EVERY_SLOT == 1) {
            is_health_time = true;
          } else {
            if (current_hour == 11) {
              // Allow entry if this hour/day hasn't been successfully reported
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
            strcpy(last_cmd_res, "Success: OTA Updated");
            strcpy(diag_cdm_status, "Firmware Updated");
            // Refresh Location via Fresh CLBS
            get_gps_coordinates();
          }

          if (is_health_time || is_ota_confirm) {
            health_in_progress = true; // Block deep sleep
          }

          if (!skip_primary_http) {
            debugln(
                "[BATT] Proceeding with Spatika Upload regardless of battery "
                "voltage.");
            sync_mode = eHttpStarted;
            send_http_data();
            primary_data_delivered =
                (success_count == 1); // v5.51 Track session success
          } else {
            debugln("[SCHED] Skipping Duplicate Upload. Checking "
                    "Backlog/Health...");
            primary_data_delivered = true; // Assume 'Success' to allow backlog
          }
//          httpInitiated = false; // Moved to end of cycle to prevent race with loopTask sleep


          if (health_in_progress) {
#if ENABLE_HEALTH_REPORT == 1
            debugln("[Health] Triggering Reliable Health Report...");
            // v7.01: Consolidate reset logic. send_health_report now handles
            // termination.
            vTaskDelay(3000 / portTICK_PERIOD_MS);

            bool health_ok = send_health_report(true);

            if (health_ok) {
              // v7.67: Only flag as completed if SUCCESSFUL so it retries
              // within the same hour window on failure.
              health_last_sent_day = current_day;
              health_last_sent_hour = current_hour;
              debugln("[Health] \u2705 Report sent successfully!");
              if (strcmp(diag_cdm_status, "PENDING") == 0) {
                strcpy(diag_cdm_status,
                       "OK"); // v7.67: CDM successfully delivered
              }
            } else {
              debugln("[Health] \u274c Report failed - will retry next slot if "
                      "window open.");
              if (strcmp(diag_cdm_status, "PENDING") == 0) {
                strcpy(diag_cdm_status, "FAIL"); // CDM Missed
              }
            }

            if (!force_ota) {
              health_in_progress = false; // Finished attempt - no OTA pending
            } else {
              debugln(
                  "[Health] OTA Pending. Holding health_in_progress=TRUE to "
                  "block sleep.");
            }
#else
            debugln("[Health] Skip: Health reporting disabled.");
            health_in_progress = false; // Allow sleep
#endif
            debugln("[GPRS] Automations finished. Checking for Piggybacked Commands...");
          }

        } else if (gprs_mode == eGprsSignalForStoringOnly) {
          debugln("[GPRS] Signal too weak for HTTP. Storing data to backlog.");
          store_current_unsent_data();
        }
      }
    }

    // --- REMOTE COMMAND EXECUTION (Command Piggybacking) ---
    if (force_reboot) {
      debugln("[CMD] Remote HARD REBOOT triggered. Killing Modem Power...");
      strcpy(last_cmd_res, "Success: Rebooting...");
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

    // v7.59: GET_GPS — Re-acquire GPS coordinates on server request
    if (force_gps_refresh) {
      debugln("[CMD] GET_GPS: Re-acquiring GPS coordinates...");
      get_lat_long_date_time(universalNumber);
      if (lati != 0 || longi != 0) {
        saveGPS(); // Persist to SPIFFS for next boot
        debugf2("[CMD] GPS acquired: %.6f, %.6f\n", lati, longi);
        strcpy(last_cmd_res, "Success: GPS Refreshed");
      } else {
        debugln("[CMD] GPS re-acquire failed. Will retry next cycle.");
        strcpy(last_cmd_res, "Fail: No GPS Fix");
      }
      force_gps_refresh = false;
    }

    // v7.59: CLEAR_FTP_QUEUE — server-requested FTP backlog clear
    if (force_clear_ftp_queue) {
      debugln(
          "[CMD] CLEAR_FTP_QUEUE: Removing ftpunsent.txt by server request...");
      bool cleared = false;
      if (SPIFFS.exists("/ftpunsent.txt")) {
        SPIFFS.remove("/ftpunsent.txt");
        debugln("[CMD] /ftpunsent.txt cleared.");
        cleared = true;
      }
      if (SPIFFS.exists("/ftpremain.txt")) {
        SPIFFS.remove("/ftpremain.txt");
        debugln("[CMD] /ftpremain.txt cleared.");
        cleared = true;
      }
      strcpy(last_cmd_res, cleared ? "Success: Queue Cleared" : "Success: Already Empty");
      force_clear_ftp_queue = false;
    }

    // v7.90: Final Cycle Reset - Move here to ensure COMMANDS (OTA/FTP/GPS)
    // finish before loopTask triggers deep sleep.
    if (httpInitiated) {
      debugln("[GPRS] Cycle fully complete (including Commands). Allowing sleep.");
      sync_mode = eHttpStop;
      httpInitiated = false;
    }

    esp_task_wdt_reset();
    vTaskDelay(
        1000 /
        portTICK_PERIOD_MS); // Reduced from 2000 for faster state transitions
  }
}

void start_gprs() {
  String response;
  // Power ON GPRS
  digitalWrite(26, HIGH);               // Power on GPRS module
  vTaskDelay(500 / portTICK_PERIOD_MS); // v5.64: Reduced from 1s for fast-boot
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
      vTaskDelay(1000 / portTICK_PERIOD_MS);
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
  const char *charArray;

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
        return; // Bug #10: add early return if file1 cannot be opened.
      }         // #TRUEFIX
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

  // v5.52: KSNDMC Padding Rule - Pad 4-digit numeric IDs to 6-digits (e.g. 1881
  // -> 001881)
  // This is required for TWS and TWS-RF legacy servers on KSNDMC.
  if ((SYSTEM == 1 || SYSTEM == 2) && tLen == 4 && isDigitStr(cleanStn)) {
    char padded[16];
    snprintf(padded, sizeof(padded), "00%s", cleanStn);
    strcpy(cleanStn, padded);
  }

  if (!strcmp(httpSet[http_no].Format,
              "json")) { // if json then this loop otherwise goto urlencoded one
// BIHAR TRG
#if SYSTEM == 0
    snprintf(http_data, sizeof(http_data),
             "{\"StnNo\":\"%s\",\"DeviceTime\":\"%04d-%02d-%02d "
             "%02d:%02d:00\",\"RainDP\":\"%05.1f\",\"RainCuml\":\"%05.1f\","
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
  snprintf(http_data, sizeof(http_data),
           "stn_no=%s&rec_time=%04d-%02d-%02d,%02d:%02d&temp=%s&humid=%s&w_"
           "speed=%s&w_dir=%s&signal=%04d&key=%s&bat_volt=%s&bat_volt2=%s",
           cleanStn, temp_year, temp_month, temp_day, temp_hr, temp_min,
           sample_temp, sample_hum, sample_avgWS, sample_WD, temp_sig,
           httpSet[http_no].Key, sample_bat, sample_bat);
#endif

// TWS-RF (ADDON / SPATIKA)
#if SYSTEM == 2
  // v7.53: DMC Legacy requires stn_no and %05.1f. Spatika General requires
  // stn_id.
  if (strstr(UNIT, "SPATIKA_GEN")) {
    snprintf(http_data, sizeof(http_data),
             "stn_id=%s&rec_time=%04d-%02d-%02d,%02d:%02d&key=%s&rainfall=%05."
             "2f&temp=%s&humid=%s&w_speed=%s&w_dir=%s&signal=%04d&"
             "bat_volt=%s&bat_volt2=%s",
             cleanStn, temp_year, temp_month, temp_day, temp_hr, temp_min,
             httpSet[http_no].Key, temp_crf, sample_temp, sample_hum,
             sample_avgWS, sample_WD, temp_sig, sample_bat, sample_bat);
  } else {
    snprintf(http_data, sizeof(http_data),
             "stn_no=%s&rec_time=%04d-%02d-%02d,%02d:%02d&key=%s&rainfall=%05."
             "1f&temp=%s&humid=%s&w_speed=%s&w_dir=%s&signal=%04d&"
             "bat_volt=%s&bat_volt2=%s",
             cleanStn, temp_year, temp_month, temp_day, temp_hr, temp_min,
             httpSet[http_no].Key, temp_crf, sample_temp, sample_hum,
             sample_avgWS, sample_WD, temp_sig, sample_bat, sample_bat);
  }
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
    diag_consecutive_http_fails = 0; // v5.49 Build 5: Reset fail streaks
    diag_consecutive_reg_fails = 0;
    if (data_mode == eCurrentData) {
      diag_first_http_count++;
      diag_http_success_count++; // v7.86 Track total live HTTP successes
      // v7.70: Present fails resets on any current-slot success
      diag_http_present_fails = 0;
      snprintf(ui_data[FLD_HTTP_FAILS].bottomRow,
               sizeof(ui_data[FLD_HTTP_FAILS].bottomRow), "PR:%d CUM:%d",
               diag_http_present_fails, diag_http_cum_fails);
    } else if (data_mode == eUnsentData) {
      diag_http_retry_count++; // v7.86 Track total backlog HTTP successes
    }
  }
  vTaskDelay(200 / portTICK_PERIOD_MS);

  unsent_counter = 0; // Initialize each time ...

  //      if(success_count == 0) { // Failure to send the current data in 1st
  //      attempt
  //            for(unsent_counter=1; unsent_counter <=5 && http_code!=200;
  //            unsent_counter++)  {
  //              esp_task_wdt_reset();
  //                debug("Retrying ");debug(unsent_counter);debugln(" times");
  //                success_count = send_at_cmd_data(http_data,charArray);
  //                vTaskDelay(200 / portTICK_PERIOD_MS);
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
        if (data_mode == eCurrentData) {
          // v7.70: Retry succeeded — reset present fails
          diag_http_present_fails = 0;
          diag_http_success_count++; // v7.86
          diag_first_http_count++;   // v7.86
          snprintf(ui_data[FLD_HTTP_FAILS].bottomRow,
                   sizeof(ui_data[FLD_HTTP_FAILS].bottomRow), "PR:%d CUM:%d",
                   diag_http_present_fails, diag_http_cum_fails);
        } else if (data_mode == eUnsentData) {
          diag_http_retry_count++; // v7.86
        }
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
      if (data_mode == eCurrentData) {
        // v7.70: Monthly cum reset on 1st of month
        if (current_day == 1 && diag_cum_fail_reset_month != current_month) {
          diag_http_cum_fails = 0;
          diag_cum_fail_reset_month = current_month;
          debugln("[HTTP] Monthly cum fail counter reset (1st of month).");
        }
        diag_http_present_fails++;
        diag_http_cum_fails++;
        snprintf(ui_data[FLD_HTTP_FAILS].bottomRow,
                 sizeof(ui_data[FLD_HTTP_FAILS].bottomRow), "PR:%d CUM:%d",
                 diag_http_present_fails, diag_http_cum_fails);
      }
      debugf1("[RECOVERY] Consec HTTP Fails: %d", diag_consecutive_http_fails);
      debugf1(" | Present: %d", diag_http_present_fails);
      debugf1(" | CumMth: %d\n", diag_http_cum_fails);

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
        vTaskDelay(100 / portTICK_PERIOD_MS);

        // SMART RECOVERY: If data fails for 4 consecutive slots (~1 hour),
        // trigger a Full Modem Reset. Use diag_consecutive_http_fails here
        // (NOT diag_consecutive_reg_fails — that counter is only for actual
        // registration failures and gates graceful_modem_shutdown).
        if (diag_consecutive_http_fails >= 4) {
          debugln("[RECOVERY] 4 Consecutive HTTP Fail Slots. Triggering Full "
                  "Modem Reset...");
          SerialSIT.println("AT+CFUN=1,1");
          vTaskDelay(5000 / portTICK_PERIOD_MS);
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

        debugln("!!! Persistent Server Rejection/Failure !!!");
        debugln("The failed record was: ");
        debugln(current_record);
        debugln("Backlog delivery will be attempted in future cycles.");

      } // closes if(data_mode == eCurrentData)
    }   // closes if(success_count == 0)
  }     // closes if(SPIFFS.exists(temp_file))
} // closes prepare_data_and_send()

void send_http_data() {
  String response;
  const char *charArray;
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
        debugln("[DNS Cache] Force Deactivating CIDs 1 & 8 (Flapping)...");
        xSemaphoreGive(serialMutex);
      }
      SerialSIT.println("AT+CGACT=0,1"); // v7.11: Kill CID 1 too if DNS is dead
      waitForResponse("OK", 2000);
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
  if (response.indexOf("+CGACT: " + String(active_cid) + ",1") != -1 &&
      diag_consecutive_http_fails == 0) {
    debugln("[GPRS] Bearer already live. Skipping CIPSHUT to save time.");
  } else {
    debugln("[GPRS] Bearer status check: Re-initializing IP stack...");
    debugln("[GPRS] Starting HTTP...");
    debug("HTTP POST REQUEST IS ");
    debugln(httpPostRequest);
    vTaskDelay(500 / portTICK_PERIOD_MS);

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
  vTaskDelay(500 / portTICK_PERIOD_MS);

  // v5.50: Silence URCs during critical HTTP setup
  SerialSIT.println("AT+CGEREP=0");
  waitForResponse("OK", 1000);

  flushSerialSIT();   // v5.42: Clear stale UART bytes before HTTPINIT
  http_ready = false; // v5.42: Reset session flag

  SerialSIT.println("AT+HTTPINIT");
  response = waitForResponse("OK", 5000);
  if (response.indexOf("OK") == -1) {
    debugln("[GPRS] HTTPINIT Failed. Trying TERM then INIT...");
    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 3000);
    vTaskDelay(500 / portTICK_PERIOD_MS);
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
            return; // C4 FIX: Do not dereference invalid File object
          }
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
          vTaskDelay(100 / portTICK_PERIOD_MS); // iter10

          // FAST PIPELINING RESET: Pre-emptively tear down and rebuild the HTTP
          // session. The remote server usually replies with "Connection: close"
          // which breaks the A7672S state machine if we pipeline directly.
          // This 1.5s overhead PREVENTS the massive ~30-second error timeouts
          // of the '706' fault.
          SerialSIT.println("AT+HTTPTERM");
          waitForResponse("OK", 1000);
          vTaskDelay(100 / portTICK_PERIOD_MS);

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
    debugln("*** Current data couldn't be sent. Backlog will be handled "
            "independently.");

    // v5.49 Build 5: NDM tracking is now handled EXCLUSIVELY by the scheduler
    // on successful record write to SPIFFS. This prevents double-counting
    // and ensures NDM represents "Processed Night Data" volume accurately.
    // diag_daily_http_fails is already incremented inside send_at_cmd_data().
  }

#if (SYSTEM == 1 || SYSTEM == 2)
  // v5.49 Build 5: INDEPENDENT FTP TRIGGER
  // Decoupled from HTTP Success. FTP serves as the robust rescue layer.
  if (gprs_mode == eGprsSignalOk && (signal_lvl > -96)) {
    send_unsent_data();
  }
#endif

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
    sync_mode = eHttpStop;
  }

  // v5.50: Restore URCs for the rest of the cycle
  SerialSIT.println("AT+CGEREP=2");
  waitForResponse("OK", 1000);

} // end of send_http_data

void send_daily_file(
    const char *dateStr) { // v5.80: Send /dailyftp_YYYYMMDD.txt
  char dailyFile[32];
  snprintf(dailyFile, sizeof(dailyFile), "/dailyftp_%s.txt", dateStr);

  if (SPIFFS.exists(dailyFile)) {
    debugf1("[FTP] Found daily file: %s. Initiating upload...\n", dailyFile);
    // Signal guard: skip 24-retry loop if signal already known to be dead
    if (signal_lvl <= -98) {
      debugln("[FTP] Skip Daily: Signal too weak (" + String(signal_lvl) +
              " dBm).");
      return;
    }
    get_registration();
    if (gprs_mode == eGprsSignalOk) {
      get_a7672s();
      vTaskDelay(3000 / portTICK_PERIOD_MS);
      esp_task_wdt_reset();
      send_ftp_file(dailyFile, true);
    } else {
      debugln("[FTP] Skip Daily: No Network.");
    }
  } else {
    debugf1("[FTP] Daily file NOT FOUND: %s\n", dailyFile);
  }
}

void send_unsent_data() { // ONLY FOR TWS AND TWS-ADDON\n  const char
                          // *charArray;
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
  // v5.51: Power-Saving FTP Gating logic
  snprintf(ftpunsent_file, sizeof(ftpunsent_file), "/ftpunsent.txt");
  int unsent_cnt = countStored(ftpunsent_file);

  // v7.62: Power-Saving FTP Gating logic (User Request)
  // 1. Threshold Met: At least 6 records (1.5 hours of data, v5.49 Build 5)
  // 2. Scheduled Sync Slots: 00:00, 06:00, 12:00, 18:00 (Synoptic hours)
  // 3. Final Day Cleanup: 08:45 AM (IST) to ensure all data for the
  // meteorological day is cleared
  bool scheduled_slot =
      (record_min == 0) &&
      (record_hr % 3 == 0);
  bool morning_cleanup = (record_hr == 8 && record_min == 45);
  bool should_push = (unsent_cnt >= 6) ||
                     (unsent_cnt > 0 && (scheduled_slot || morning_cleanup));

  if (signal_lvl > -95 && (should_push || force_ftp) &&
      SPIFFS.exists(ftpunsent_file)) {
    // v7.54: BSNL Carrier Congestion Breather
    // Switching from HTTP POST to FTP immediately causes Socket Zombies and
    // drops the modem into an '85' signal panic loop. Force a 5s tower release
    // delay.
    debugln("[FTP] HTTP finished. Waiting 5s for Tower to release context "
            "(Breather)...");
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();

    debugf2("[FTP] Backlog Push: %d records found. Mode: %s\n", unsent_cnt,
            (scheduled_slot
                 ? "SCHEDULED_SLOT"
                 : (morning_cleanup ? "MORNING_CLEANUP" : "THRESHOLD_MET")));
    debugln();
    debug("FTP file name is ");
    debugln(fileName);
    debugln();
    debug("SampleNo  is ");
    debugln(sampleNo);
    debugln();
    debugln("Entering time bound FTP loop");

    if (SPIFFS.exists(ftpunsent_file)) {
      debugln("[FTP] Checking Registration before backlog upload...");
      // Signal guard: skip 24-retry loop if signal already known to be dead
      if (signal_lvl <= -98) {
        debugln("[FTP] Skip Backlog: Signal too weak (" + String(signal_lvl) +
                " dBm).");
      } else {
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

          // v7.55 Queue Draining & Payload Chunking (Massive Bug Fix)
          // Avoids 5.7 KB Timeout on BSNL by securely extracting 15 records
          // (~900 bytes)
          SPIFFS.remove(fileName); // Ensure dest is clean

          File src = SPIFFS.open(ftpunsent_file, FILE_READ);
          if (src) {
            File chunk = SPIFFS.open(fileName, FILE_WRITE);
            File remainder = SPIFFS.open("/ftpremain.txt", FILE_WRITE);
            int linesRead = 0;

            while (src.available()) {
              esp_task_wdt_reset(); // Feed WDT for huge backlog files
              String line = src.readStringUntil('\n');
              line.trim();              // Kill wandering \r
              if (line.length() > 15) { // Valid data line length check
                line += "\r\n";
                if (linesRead < 15) {
                  if (chunk)
                    chunk.print(line);
                } else {
                  if (remainder)
                    remainder.print(line);
                }
                linesRead++;
              }
            }
            if (chunk)
              chunk.close();
            if (remainder)
              remainder.close();
            src.close();

            // DO NOT DELETE ftpunsent_file here!
            // Wait for upload success in send_ftp_file() before committing the
            // remainder.
            if (linesRead > 15) {
              debugln("[FTP] Huge Backlog! 15 records chunked for TX. "
                      "Original queue protected until success.");
            } else {
              SPIFFS.remove("/ftpremain.txt");
              debugln("[FTP] Backlog fully chunked into Active Payload.");
            }
          } else {
            debugln("Failed to open ftpunsent.txt for chunking.");
          }
          debug("Retrieved file is ");
          debugln(fileName);
          esp_task_wdt_reset();
          send_ftp_file(fileName, false);
        } else {
          debugln("[FTP] Skip: Registration lost. Retrying next hour.");
        }
      } // end else (signal OK)
    } else {
      debugln("No ftpunsent.txt found. Skipping FTP.");
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
      snprintf(fileName, sizeof(fileName),
               "/TWS_%s_%02d%02d%02d_%02d%02d00.kwd", ftp_station, ftp_year,
               rf_cls_mm, rf_cls_dd, record_hr, record_min);
#endif
#if SYSTEM == 2
      // v7.53: SPATIKA_GEN uses .swd, KSNDMC_ADDON uses .kwd
      if (strstr(UNIT, "SPATIKA_GEN"))
        snprintf(fileName, sizeof(fileName),
                 "/TWSRF_%s_%02d%02d%02d_%02d%02d00.swd", ftp_station, ftp_year,
                 rf_cls_mm, rf_cls_dd, record_hr, record_min);
      else
        snprintf(fileName, sizeof(fileName),
                 "/TWSRF_%s_%02d%02d%02d_%02d%02d00.kwd", ftp_station, ftp_year,
                 rf_cls_mm, rf_cls_dd, record_hr, record_min);
#endif

      if (SPIFFS.exists(temp_file)) {
        copyFile(temp_file, fileName); // copyFile(source,destination);
        debug("Retrieved file is ");
        debugln(fileName);
        esp_task_wdt_reset();
        send_ftp_file(fileName, true);
      } else {
        debugln("Daily FTP: Temp file not found. Skipping.");
      }
    }
  }
} // end send_unsent_data

void send_ftp_file(char *fileName, bool isDailyFTP) {
  String response;
  const char *response_char;
  const char *charArray; // Added as per instruction
  const char *resp;      // Added as per instruction
  // Use a local module-safe path (removes leading slash for cellular module
  // commands)
  char *modulePath = (fileName[0] == '/') ? &fileName[1] : fileName;

  char gprs_xmit_buf[300];
  String rssiStr;
  int rssiIndex, rssiEndIndex, retries, registration;
  int handle_no;
  const char *ftpCharArray;
  char *csqstr;
  int success;
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
      vTaskDelay(200 / portTICK_PERIOD_MS);
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
        debugln("Failed to open FTP file for reading — aborting transfer.");
        SerialSIT.println("AT+CFTPSLOGOUT");
        waitForResponse("+CFTPSLOGOUT: 0", 9000);
        SerialSIT.println("AT+CFTPSSTOP");
        waitForResponse("OK", 3000);
        return; // v7.67: Guard against null file dereference
      }
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
        vTaskDelay(200 / portTICK_PERIOD_MS);
        SerialSIT.println("AT+CFTPSSTOP");
        waitForResponse("OK", 3000);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        SerialSIT.println("AT+FSLS");
        response = waitForResponse("OK", 10000);

      } else {
        snprintf(
            gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSWRITE=%d,%d,10\r\n",
            handle_no,
            fileSize); // 0 : if the file does not exist, it will be created
        vTaskDelay(200 / portTICK_PERIOD_MS);
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
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        int ftp_put_retries = 0;
        const int MAX_FTP_PUT_RETRIES = 1;
        bool upload_success = false;

        snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
                 "AT+CFTPSPUTFILE=\"C:/%s\",0", modulePath);

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
          // v5.45: Smart PUT monitor — poll every 500ms for success OR PS
          // bearer loss (+CGEV: ME PDN DEACT 1). On 2G BSNL, +CREG:0/1 (CS
          // domain events) are harmless for data. But +CGEV: ME PDN DEACT 1
          // means the PS bearer (CID 1) itself died — bail immediately rather
          // than waiting out the full 60s timeout.
          {
            response1 = "";
            unsigned long put_start = millis();
            bool ps_bearer_lost = false;
            while ((millis() - put_start) < 60000) {
              esp_task_wdt_reset();
              vTaskDelay(500 / portTICK_PERIOD_MS);
          while (SerialSIT.available()) {
                char cc = SerialSIT.read();
                if (response1.length() < 2048)
                  response1 += cc;
              }
              if (response1.indexOf("+CFTPSPUTFILE:") != -1)
                break; // Done
              if (response1.indexOf("+CGEV: ME PDN DEACT 1") != -1) {
                debugln("[FTP] PS Bearer (CID1) lost mid-PUT. Bailing early.");
                ps_bearer_lost = true;
                break;
              }
            }
            if (ps_bearer_lost)
              response1 = ""; // Force retry path
          }
          debug("Response of AT+CFTPSPUTFILE ");
          debugln(response1);

          if (response1.indexOf("+CFTPSPUTFILE: 0") != -1) {
            upload_success = true;
            if (isDailyFTP) strcpy(last_cmd_res, "Success: Daily FTP OK");
            else strcpy(last_cmd_res, "Success: Backlog FTP OK");
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
          // last_cmd_res already set in the loop
          diag_consecutive_reg_fails =
              0; // RESET counter on any successful data upload
          markFileAsDelivered(fileName); // v5.48 Track recovered records

          if (isDailyFTP) { // If Daily FTP is successful,
                            // remove the dailyftp file.
            SPIFFS.remove(temp_file);
            debug("Removed Daily FTP file: ");
            debugln(temp_file);
          } else {
            // COMMIT Queue Remainder on Success
            SPIFFS.remove("/ftpunsent.txt");
            if (SPIFFS.exists("/ftpremain.txt")) {
              SPIFFS.rename("/ftpremain.txt", "/ftpunsent.txt");
              debugln("[FTP] Chunk successful! ftpunsent.txt overwritten with "
                      "remainder queue.");
            } else {
              debugln("Cleared ftpunsent.txt permanently after full upload.");
            }
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
              vTaskDelay(100 / portTICK_PERIOD_MS);
              if (SPIFFS.remove(prefixedFileName)) {
                debugln("File removed successfully");
              } else {
                debugln("File removal failed");
              }
            }
            file.close(); // v5.49 Build 5: FIX LEAK - Close file handle before
                          // next iteration
            file = root.openNextFile();
          }
          root.close();

          // Success Cleanup: Remove the temporary file from cellular module
          // storage
          snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSDEL=\"C:/%s\"",
                   modulePath);
          SerialSIT.println(gprs_xmit_buf);
          waitForResponse("OK", 5000);
        } else {
          debugln("Did not succeed in FTP ..");
          strcpy(last_cmd_res, "Fail: FTP PUT Error");
          // Failed: Clean up ONLY the temporary .swd/.kwd copies.
          // DO NOT delete 'temp_file' or 'ftpunsent_file' (Source Data) so we
          // can retry later.
          if (SPIFFS.exists("/ftpremain.txt")) {
            SPIFFS.remove(
                "/ftpremain.txt"); // Discard remainder, preserve original queue
            debugln("[FTP] Upload failed. Discarded remainder to protect "
                    "original queue.");
          }

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
                // v7.91: Targeted cleanup. Only remove if it matches the active
                // filename.
                if (prefixedFileName == String(fileName)) {
                  debugf("Removing file (failure cleanup): %s\n",
                         prefixedFileName.c_str());
                  vTaskDelay(100 / portTICK_PERIOD_MS);
                  SPIFFS.remove(prefixedFileName);
                }
              }
              file.close(); // v5.49 Build 5: FIX LEAK
              file = root.openNextFile();
            }
            root.close();
          }
        }

        esp_task_wdt_reset();
        SerialSIT.println("AT+CFTPSLOGOUT");
        response = waitForResponse("+CFTPSLOGOUT: 0", 9000);
        debug("Response of AT+CFTPSLOGOUT is ");
        debugln(response);
        vTaskDelay(200 / portTICK_PERIOD_MS);

        // Clean up FTP service internally so it frees the IP stack for HTTP
        SerialSIT.println("AT+CFTPSSTOP");
        response = waitForResponse("OK", 3000);
        debug("Response of AT+CFTPSSTOP is ");
        debugln(response);
        vTaskDelay(200 / portTICK_PERIOD_MS);

        // NOTE: Modem FS file already deleted in the success cleanup path
        // above. A second AT+FSDEL here is redundant and always returns
        // ERROR. Suppressed to eliminate confusing noise in the log.

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
  String response;
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
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
  } else {
    debugln("[GPRS] ⚠️ Session was unstable (reg fails). Hard Hardware "
            "Reset.");
  }

  debugln("[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).");
  digitalWrite(26, LOW);
  gprs_started = false;
  gprs_mode = eGprsSleepMode; // Prevent Ghost Restart during sleep entry
}

void send_sms() {
  String response;
  vTaskDelay(500 /
             portTICK_PERIOD_MS); // TRG8-3.0.5g reduced from 1min to 500ms
  int msg_no;

  debugln();
  debugln("[GPRS] Checking SMS...");
  debugln();

  for (msg_no = 1; msg_no < 5; msg_no++) {
    process_sms(msg_no);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
  SerialSIT.println("AT+CMGD=1,4");
  response = waitForResponse("OK", 5000);
  debugln("Removed READ messages");

  sync_mode = eSMSStop;
}

int send_at_cmd_data(char *payload, String response_arg) {
  unsigned long start = millis();
  esp_task_wdt_reset();
  String response;
  const char *charArray = response_arg.c_str(); // Use local pointer

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
  // v5.45: Capping timeout at 5000ms. Most A7672S firmwares reject > 5s with
  // ERROR.
  char ht_data[80];
  snprintf(ht_data, sizeof(ht_data), "AT+HTTPDATA=%d,5000", i);

  debugf1("Payload is %s", payload);
  debugln();
  SerialSIT.println(ht_data);
  // v5.45: Increased timeout to 10s for high-latency BSNL 2G handshakes
  if (waitForResponse("DOWNLOAD", 10000).indexOf("DOWNLOAD") == -1) {
    debugln("[HTTP] AT+HTTPDATA failed (Missing DOWNLOAD).");
    flushSerialSIT();
    return 0; // Original returns 0
  }
  vTaskDelay(500 / portTICK_PERIOD_MS); // v7.00: Increased buffer prep delay
  SerialSIT.print(payload); // v6.40: Use print to avoid trailing \r\n in body
  response = waitForResponse("OK", 5000);

  SerialSIT.println("AT+HTTPACTION=1");
  waitForResponse("OK", 2000);
  response = waitForResponse("+HTTPACTION:",
                             25000); // v5.45: 25s (was 45s). LTE response in
                                     // 3-8s; 45s was dead-server waste.
  debug("Response of AT+HTTPACTION=1 is ");
  debugln(response);

  if (response.indexOf("200") == -1) {
    // v5.45: Extract error code from +HTTPACTION: prefix ONLY.
    // Old method searched the whole buffer from comma1→comma2, which picked
    // up commas inside +CGEV: ME PDN ACT 8,0 URCs that rode in on the same
    // buffer, producing corrupt strings like "0\n\n+CGEV: ME PDN ACT 8" as
    // the code.
    String errCode = "";
    int ha_idx = response.indexOf("+HTTPACTION:");
    if (ha_idx != -1) {
      // Format: +HTTPACTION: <method>,<status>,<datalen>
      int c1 = response.indexOf(',', ha_idx);
      int c2 = (c1 != -1) ? response.indexOf(',', c1 + 1) : -1;
      if (c1 != -1 && c2 != -1) {
        errCode = response.substring(c1 + 1, c2);
        errCode.trim();
      }
    }
    if (errCode.length() > 0) {
      strncpy(diag_http_fail_reason, errCode.c_str(),
              sizeof(diag_http_fail_reason) - 1);
      diag_http_fail_reason[sizeof(diag_http_fail_reason) - 1] = 0;
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
    diag_daily_http_fails++; // Increment on failure
    return 0;
  }

  // If we reach here, HTTPACTION returned 200
  // v6.35: A7672S needs time after action before read
  vTaskDelay(500 / portTICK_PERIOD_MS);

  SerialSIT.println("AT+HTTPREAD=0,512"); // v5.45: 512 bytes (was 290) —
                                          // safer for OTA ACK + health JSON
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

    // v5.46: Daily silent RTC sync from server time (force=false: once/day,
    // drift>90s)
    sync_rtc_from_server_tm(payload_ptr, false);

    // v5.49 Build 5: STRICT DATE VERIFICATION for HTTP Success Tracking
    if (temp_sampleNo >= 0 && temp_sampleNo <= MAX_SAMPLE_NO) {
      // Calculate 'NOW' meteorological close date
      int cur_dd = current_day, cur_mm = current_month, cur_yy = current_year;
      int curr_h = (current_hour < 24) ? current_hour : 0;
      int curr_m = (current_min < 60) ? current_min : 0;
      int curr_sNum = (curr_h * 4 + curr_m / 15 + 61) % 96;
      if (curr_sNum <= 60) {
        next_date(&cur_dd, &cur_mm, &cur_yy);
      }

      // Calculate 'RECORD' meteorological close date
      int rec_dd = temp_day, rec_mm = temp_month, rec_yy = temp_year;
      if (temp_sampleNo <= 60) {
        next_date(&rec_dd, &rec_mm, &rec_yy);
      }

      bool isToday = (cur_yy == rec_yy && cur_mm == rec_mm && cur_dd == rec_dd);
      uint32_t bit = (1UL << (temp_sampleNo % 32));
      bool alreadySent = isToday
                             ? (diag_sent_mask_cur[temp_sampleNo / 32] & bit)
                             : (diag_sent_mask_prev[temp_sampleNo / 32] & bit);

      if (!alreadySent) {
        if (isToday) {
          diag_sent_mask_cur[temp_sampleNo / 32] |= bit;
        } else {
          diag_sent_mask_prev[temp_sampleNo / 32] |= bit;
        }
      }
    }

    // v5.55: Use RTC_DATA_ATTR guard so SPIFFS write survives deep sleep.
    // 'static' local resets on every wakeup, causing a write every boot.
    if (!apn_saved_this_sim && String(cached_iccid) != "" &&
        String(apn_str) != "") {
      save_apn_config(apn_str, cached_iccid);
      apn_saved_this_sim = true;
    }

    diag_http_time_total += (millis() - start);
    strcpy(diag_http_fail_reason, "NONE");

    return 1;
  } else {
    debugln("GPRS SEND : It is a Failure");
    // diag_daily_http_fails++; // This is now handled outside this function

    // v6.85: Trigger RTC resync on rejection
    if (strstr(payload_ptr, "Rejected") || strstr(payload_ptr, "rejected")) {
      diag_rejected_count++;
      debugf1("[TIME] Server Rejected Data. Count: %d\n", diag_rejected_count);
      sync_rtc_from_server_tm(payload_ptr, true);
      if (diag_rejected_count > 2) {
        debugln("[TIME] Persistent rejection. Falling back to CLBS resync...");
        resync_time();
        diag_rejected_count = 0;
      }
    }

    // v5.88: Ensure HTTP session is closed on failure
    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 1000);

    strncpy(diag_http_fail_reason, "SRV_ERR",
            sizeof(diag_http_fail_reason) - 1);
    diag_http_fail_reason[sizeof(diag_http_fail_reason) - 1] = '\0';
    return 0;
  }
}

void store_current_unsent_data() {
  char finalStringBuffer[160] = {0};
  char last_rec_file[50];
  snprintf(last_rec_file, sizeof(last_rec_file), "/lastrecorded_%s.txt",
           station_name);

  if (SPIFFS.exists(last_rec_file)) {
    File f = SPIFFS.open(last_rec_file, FILE_READ);
    if (f) {
      size_t readLen =
          f.readBytes(finalStringBuffer, sizeof(finalStringBuffer) - 1);
      finalStringBuffer[readLen] = '\0';
      f.close();
    }
  }

#if SYSTEM == 0
  snprintf(unsent_file, sizeof(unsent_file), "/unsent.txt");

  // SAFETY: If the file is getting too large (>150KB), SPIFFS appends can get
  // slow or buggy. 150KB = ~2500 records. If we haven't sent by then,
  // something is very wrong.
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
  vTaskDelay(300 / portTICK_PERIOD_MS);
}

/*
 *  SIM Network Registration and Setup
 *  - get_signal_strength();//2024 iter3
 *  - get_network();
 *  - get_registration();
 *  - get_a7672s();
 */

void get_signal_strength() {
  String response, rssiStr;
  const char *resp;

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
  while ((signal_lvl == 0) &&
         (retries < 120)) { // 120 loops @ 500ms = 60s max timeout
    esp_task_wdt_reset();
    SerialSIT.println("AT+CSQ");
    response = waitForResponse("+CSQ ", 1000);
    rssiIndex = response.indexOf("+CSQ: ");
    if (rssiIndex != -1) {
      rssiIndex += 6;
      rssiEndIndex = response.indexOf(",", rssiIndex);
      if (rssiEndIndex != -1) {
        rssiStr = response.substring(rssiIndex, rssiEndIndex);
      } else {
        rssiStr = "0";
      }
    } else {
      rssiStr = "0";
    }
    resp = rssiStr.c_str();
    signal_strength = (-113 + 2 * (atoi(resp)));
    debug("Signal strength IN gprs task is ");
    debugln(signal_strength);
    if (signal_strength != 85) {
      signal_lvl = signal_strength;
      // v5.50 Optimization: Exit instantly if we get a valid reading,
      // even if it is -113dBm (modem is alive but signal is zero).
      // No point looping for 60s if the modem is clearly reporting low signal.
      break;
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
    debug("Signal Level is ");
    debugln(signal_lvl);
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
  String response;
  const char *resp;
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
  String response;
  esp_task_wdt_reset();
  debugln();
  debugln("************************");
  debugln("GETTING REGISTRATION ");
  debugln("************************");

  // v5.45.6 4G-AWARE HYBRID REGISTRATION:
  // - Airtel/Jio: 4G-first. CEREG=1/5 = success. CEREG=3 w/ cell = LTE-CS
  // denied, push CGATT.
  // - BSNL: 2G/3G only. Uses CREG + CGREG (no LTE). 24 retries max.
  // - Hard cap: 24 retries total (~2.5 min max). No more 50-cycle waste.
  bool isBSNL = (strstr(carrier, "BSNL") != nullptr);
  int no_of_retries = 24; // v5.45.6: Hard cap at 24 for all carriers
  registration = 0;
  retries = 0;
  bool is_registered = false;

  // v5.45.6: Track if we've already done the first-fail DIAG to avoid
  // repeating it
  bool diag_done = false;
  int consecutive_unreg = 0;

  // v7.54 FAST-TRACK REGISTRATION CHECK:
  // If the modem was kept awake or reconnected instantly, checking CGREG first
  // saves 4-5 seconds of redundant AT commands.
  SerialSIT.println("AT+CGREG?");
  String cgregResp = waitForResponse("+CGREG:", 1000);
  if (cgregResp.indexOf(",1") != -1 || cgregResp.indexOf(",5") != -1) {
    debugln(
        "[GPRS] Fast-Track: Modem already registered! Bypassing setup block.");
    is_registered = true;
    strcpy(reg_status,
           (cgregResp.indexOf(",1") != -1) ? "GSM:Home:OK" : "GSM:Roam:OK");
  } else {
    // RUN FULL SETUP BLOCK ONLY IF NOT REGISTERED
    // v5.60 SURE-SHOT REGISTRATION:
    SerialSIT.println("AT+CFUN=1"); // Full functionality
    waitForResponse("OK", 1000);
    SerialSIT.println("AT+CGDCONT=8,\"IP\",\"\""); // Kill side-channel profile
    waitForResponse("OK", 1000);

    // v5.60: SET APN EARLY FOR CID 1 (Helps BSNL GPRS attachment)
    if (strlen(apn_str) > 3) {
      debugln("[GPRS] Pre-setting APN for CID 1 & 6: " + String(apn_str));
      // Set for both CID 1 and CID 6 as some firmware variants use CID 6 for
      // LTE
      SerialSIT.print("AT+CGDCONT=1,\"IP\",\"");
      SerialSIT.print(apn_str);
      SerialSIT.println("\"");
      waitForResponse("OK", 1000);

      SerialSIT.print("AT+CGDCONT=6,\"IP\",\"");
      SerialSIT.print(apn_str);
      SerialSIT.println("\"");
      waitForResponse("OK", 1000);
    }

#if FORCE_2G_ONLY == 1
    debugln("[GPRS] FORCE_2G_ONLY flag active. Setting CNMP=13 immediately.");
    SerialSIT.println("AT+CNMP=13"); // GSM Only
#else
    SerialSIT.println("AT+CNMP=2");     // Automatic Mode (LTE/GSM)
#endif
    waitForResponse("OK", 1000);
    SerialSIT.println("AT+CMNB=3"); // LTE then GSM
    waitForResponse("OK", 1000);
    SerialSIT.println("AT+CGATT=1"); // Force GPRS Attachment
    waitForResponse("OK", 5000);
    SerialSIT.println("AT+CREG=1");
    waitForResponse("OK", 1000);
    SerialSIT.println("AT+CEREG=2"); // Enhanced LTE reporting
    waitForResponse("OK", 1000);
    SerialSIT.println("AT+CEMODE=2"); // PS Only (Data Only)
    waitForResponse("OK", 1000);
    SerialSIT.println("AT+CPSMS=0"); // Disable Power Saving Mode
    waitForResponse("OK", 1000);
    SerialSIT.println("AT+CNETLIGHT=0"); // Reset LED driver
    waitForResponse("OK", 500);
    SerialSIT.println("AT+CNETLIGHT=1"); // Ensure LED blinks
    waitForResponse("OK", 1000);
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Let it settle
  }

  while (!is_registered && (retries < no_of_retries)) {
    esp_task_wdt_reset();

    // UI Feedback: Show progress on LCD
    snprintf(reg_status, 16, "TRY #%d...", retries + 1);

    // v5.45.6: 4G-AWARE REGISTRATION POLL
    // Strategy:
    //   BSNL  → Uses CREG (2G) + CGREG (GPRS/3G). No LTE on BSNL.
    //   Airtel/Jio → Uses CEREG (4G/LTE) first, then CREG fallback.
    //
    // CEREG=3 interpretation (THE FIX):
    //   +CEREG: 2,3         = LTE: Denied, no cell info visible. Truly
    //   denied. +CEREG: 2,3,TAC,CID = LTE: Denied at CS level BUT modem CAN
    //   see the
    //                         tower. Airtel 4G shuts down 2G/CS voice, so
    //                         CREG says Denied. Tower IS there. Force CGATT
    //                         and wait.
    int r2 = -1, r4 = -1;

    if (!isBSNL) {
      // --- 4G Path (Airtel / Jio): Check CEREG first ---
      SerialSIT.println("AT+CEREG?");
      String resp4 = waitForResponse("+CEREG:", 2000);
      int c4 = resp4.indexOf(",");
      if (c4 != -1) {
        r4 = resp4.substring(c4 + 1).toInt();

        // v5.45.6: Check for cell info after the stat field.
        // +CEREG: 2,3,TAC,CID → has a second comma after the stat.
        // +CEREG: 2,3         → no second comma. Truly no signal.
        if (r4 == 3) {
          int c4b = resp4.indexOf(',', c4 + 1); // comma after stat field
          bool has_cell_info = (c4b != -1);
          if (has_cell_info) {
            // Tower is visible but Airtel denies 2G CS registration.
            // This is the "Airtel 2G Ghost" — treat as still searching.
            debugln(
                "[GPRS] CEREG=3 but LTE cell visible (Airtel 4G-only tower)."
                " Pushing CGATT and waiting...");
            SerialSIT.println("AT+CGATT=1");
            waitForResponse("OK", 3000);
            r4 = 0; // Reset to 'searching' — do NOT count as denied
          } else {
            debugln("[GPRS] CEREG=3, no cell info. Truly no LTE signal.");
          }
        }
      }

      // Also check CREG as fallback for 2G/3G registration
      SerialSIT.println("AT+CREG?");
      String resp2 = waitForResponse("+CREG:", 2000);
      int c2 = resp2.indexOf(",");
      if (c2 != -1)
        r2 = resp2.substring(c2 + 1).toInt();
    } else {
      // --- BSNL Path: 2G/3G only. Check CREG + CGREG (adaptive wait below)
      // ---
      SerialSIT.println("AT+CREG?");
      String resp2 = waitForResponse("+CREG:", 2000);
      int c2 = resp2.indexOf(",");
      if (c2 != -1)
        r2 = resp2.substring(c2 + 1).toInt();
    }

    // Determine overall registration status (4G preferred)
    // r4 has already been normalized (Airtel ghost CEREG=3 → set to 0)
    if (r4 == 1 || r4 == 5) {
      registration = r4;
    } else if (r2 == 1 || r2 == 5) {
      registration = r2;
    } else if (r4 != -1 && r4 != 0) {
      registration = r4; // Could be 2=searching, 3=denied, 4=unknown
    } else if (r2 != -1) {
      registration = r2;
    } else {
      registration = 0;
    }

    // --- SUCCESS CHECK ---
    if (r2 == 1 || r2 == 5) {
      is_registered = true;
      isLTE = false;
      strcpy(reg_status, (r2 == 1) ? "GSM:Home:OK" : "GSM:Roam:OK");
      debugln("[GPRS] Registered via CREG! (2G:" + String(r2) + ")");
      break;
    }
    if (r4 == 1 || r4 == 5) {
      is_registered = true;
      isLTE = true;
      strcpy(reg_status, (r4 == 1) ? "LTE:Home:OK" : "LTE:Roam:OK");
      debugln("[GPRS] Registered via CEREG! (4G:" + String(r4) + ")");
      break;
    }

    // --- FAILURE TRACKING ---
    // Only count as hard-unreg if registration is truly bad (not 'searching')
    bool is_searching = (registration == 2);   // 2 = actively searching
    bool is_hard_denied = (registration == 3); // 3 = denied with no cell info
    if (!is_searching && registration != 0 && is_hard_denied) {
      consecutive_unreg++;
    } else if (registration == 0) {
      consecutive_unreg++; // No radio at all
    } else {
      consecutive_unreg = 0; // Searching (status=2) is OK, don't count
    }

    // Hard denied diag — only log ONCE, not every iteration
    if (is_hard_denied && !diag_done) {
      diag_done = true;
      debugln("[DIAG] Network Denied (first occurrence). Querying CPSI+CEER:");
      SerialSIT.println("AT+CPSI?");
      waitForResponse("OK", 2000);
      SerialSIT.println("AT+CEER");
      waitForResponse("OK", 2000);
    }

    // Hard cap: give up after too many consecutive true-denials
    if (consecutive_unreg >= 18) {
      debugln("[GPRS] 18 consecutive unreg. Giving up.");
      break;
    }

    // --- TIERED RECOVERY (fires every 5 retries) ---
    if (retries > 0 && retries % 5 == 0) {
      if (retries == 5) {
        // Tier 1 @ iter 5: RADIO-OFF SIM SCRUB + Clear Forbidden PLMNs
        debugln("[GPRS] Tier1 @ iter5: Radio-Off SIM Scrub...");
        SerialSIT.println("AT+CFUN=0");
        waitForResponse("OK", 5000);
        SerialSIT.println(
            "AT+CRSM=214,28539,0,0,12,\"FFFFFFFFFFFFFFFFFFFFFFFF\"");
        waitForResponse("OK", 2000);
        SerialSIT.println("AT+CFUN=1");
        waitForResponse("OK", 5000);
        SerialSIT.println("AT+COPS=0");
        waitForResponse("OK", 5000);
        SerialSIT.println("AT+CGATT=1");
        waitForResponse("OK", 3000);
      } else if (retries == 10) {
        // Tier 2 @ iter 10: Force GSM-only (diagnostic step)
        debugln("[GPRS] Tier2 @ iter10: Testing GSM-Only Mode (AT+CNMP=13)...");
        SerialSIT.println("AT+CNMP=13");
        waitForResponse("OK", 2000);
        SerialSIT.println("AT+COPS=0");
        waitForResponse("OK", 5000);
        SerialSIT.println("AT+CGATT=1");
        waitForResponse("OK", 3000);
      } else if (retries == 15) {
        // Tier 3 @ iter 15: Force LTE-Only mode
        debugln("[GPRS] Tier3 @ iter15: Testing LTE-Only Mode (AT+CNMP=38)...");
        SerialSIT.println("AT+CNMP=38");
        waitForResponse("OK", 2000);
        SerialSIT.println("AT+COPS=0");
        waitForResponse("OK", 5000);
        SerialSIT.println("AT+CGATT=1");
        waitForResponse("OK", 3000);
      } else if (retries == 20) {
        // Tier 4 @ iter 20: Restore Auto-Mode + COPS auto (carrier-agnostic)
        // NOTE: We do NOT hard-code 40445 (Airtel) here — BSNL would break.
        debugln(
            "[GPRS] Tier4 @ iter20: Restoring Auto-Mode (CNMP=2, COPS=0)...");
        SerialSIT.println("AT+CNMP=2"); // Auto (LTE+GSM)
        waitForResponse("OK", 1000);
        SerialSIT.println("AT+CMNB=3"); // LTE then GSM
        waitForResponse("OK", 1000);
        SerialSIT.println(
            "AT+COPS=0"); // Auto operator — works for ALL carriers
        waitForResponse("OK", 10000);
        SerialSIT.println("AT+CGATT=1");
        waitForResponse("OK", 3000);
      }
    }

    // BSNL early kick: if stuck in Idle at iter 3, force re-scan
    if (isBSNL && registration == 0 && retries == 3) {
      debugln(
          "[GPRS] BSNL Stubborn Idle. Forcing Frequency Re-scan (COPS=0)...");
      SerialSIT.println("AT+COPS=0");
      waitForResponse("OK", 5000);
    }

    debugf("Reg Search [BSNL:%d]... Status:%d Iter:#%d/%d\n", isBSNL,
           registration, retries + 1, no_of_retries);

    // v5.45.6: ADAPTIVE WAIT: Poll CGREG every 1s for up to 5s.
    // Exits immediately on GPRS/3G registration — no wasted blind sleep.
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
            isLTE =
                !isBSNL; // For BSNL it's GPRS/3G. For others it's LTE bearer.
            strcpy(reg_status, (qreg == 1) ? "GPRS:Home:OK" : "GPRS:Roam:OK");
            is_registered = true;
          } else {
            // v7.87: Track exact failure reasons
            if (signal_lvl <= -109 || signal_lvl == 0 || signal_lvl == 99) {
              strcpy(diag_reg_fail_type, "NO_SIGNAL");
            } else if (qreg == 0) {
              strcpy(diag_reg_fail_type, "NOT_SRCH");
            } else if (qreg == 2) {
              strcpy(diag_reg_fail_type, "SEARCHING");
            } else if (qreg == 3) {
              strcpy(diag_reg_fail_type, "DENIED");
            } else {
              snprintf(diag_reg_fail_type, sizeof(diag_reg_fail_type),
                       "UNKNOWN(%d)", qreg);
            }
          }
        }
      }
    }
    retries++;
  } // end while

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
    debugln("Registration Successful.");
  } else {
    diag_gprs_fails++;
    diag_consecutive_reg_fails++;
    gprs_mode = eGprsSignalForStoringOnly;
    debugf1("Registration failed. Consecutive fails: %d/10\n",
            diag_consecutive_reg_fails);

    if (diag_consecutive_reg_fails >= 10) {
      debugln("[GPRS] PERSISTENT REG FAIL. Resetting system...");
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      ESP.restart();
    }
  }
}

void get_a7672s() {
  String response;
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
      // verify_bearer_or_recover has something to compare against on
      // subsequent retries.
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

        // If it has failed 2+ times across wake cycles, the network data
        // layer is down. Stop hunting to save power.
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
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    SerialSIT.println("AT+CGATT=1"); // Re-attach
    waitForResponse("OK", 5000);
    vTaskDelay(2000 / portTICK_PERIOD_MS);

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

// SMS & FTP functions below

void process_sms(char msg_no) {
  String response;
  char *csqstr, c, *ptr, msg_rcvd_number[100] = {0};
  int i, response_no;

  // new
  char *msg_body, msg_recd_no[20] = {0}, temp[20] = {0}, temp1[20] = {0},
                  temp2[40] = {0};
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

    vTaskDelay(100 / portTICK_PERIOD_MS);

    // new
    msg_body = strstr(response_char, "VARSHA");
    offset_cnt = read_line(msg_body, temp, sizeof(temp),
                           0x20); // 'VARSHA' is removed
                                  // offset_cnt at end of line
    msg_body += offset_cnt;

    offset_cnt = read_line(msg_body, temp, sizeof(temp),
                           0x20); // command is read into temp
    msg_body += offset_cnt;

    if (strstr(response_char, "GET_STATUS")) {
      prepare_and_send_status(msg_rcvd_number);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    } else if (strstr(response_char, "GET_GPS")) {
      get_lat_long_date_time(msg_rcvd_number); // WORKING UNCOMMENT IT
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    } else if (strstr(response_char, "SEND_FTP_DAILY_DATA")) {
      /*
       * dateStPtr = strstr(gprs_response, "DATE=");
       * sscanf(dateStPtr, "DATE=%s", &datevale);
       */
      send_daily = 1;
      char *dtPtr;
      int y1, m1, d1;
      dtPtr = strstr(response_char, "DATE");
      if (dtPtr != NULL) {
        if (sscanf(dtPtr, "DATE=%04d-%02d-%02d", &y1, &m1, &d1) == 3) {
          char requestedDate[20];
          snprintf(requestedDate, sizeof(requestedDate), "%04d%02d%02d", y1, m1,
                   d1);
          debug(" Date is ");
          debugln(requestedDate);
          copyFromSPIFFSToFS(requestedDate); // temp1 is date
        }
      }
    } else if (strstr(response_char, "SET_FW_FNAME")) {
      // VARSHA SET_FW_FOR_LPC FILE=firmware.bin
      char *filePtr = strstr(response_char, "FILE");
      if (filePtr != NULL) {
        // Safe string parse to prevent stack corruption
        if (sscanf(filePtr, "FILE=%19s", temp1) ==
            1) { // +CCLK: \"23/08/01,09:54:35+00\"
          debug("Firmware file is ");
          debugln(temp1);
          fetchFromHttpAndUpdate(temp1);
        }
      }

    } else {

      // debug("No message recd ");
      // debugln(response);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void prepare_and_send_status(char *gsm_no) {
  String response;
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
  String response;
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
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
  debugln("[GPS] All attempts to get fresh coordinates failed.");
}

void get_lat_long_date_time(char *gsm_no) {
  String response;
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

  vTaskDelay(5000 / portTICK_PERIOD_MS); // M7 FIX: was bare 5000 ticks = 50s
  SerialSIT.println("AT+CLBS=1");
  response = waitForResponse("+CLBS:", 10000);
  vTaskDelay(200 / portTICK_PERIOD_MS);
  debug("Response of AT+CLBS=1 is ");
  debugln(response);
  vTaskDelay(200 / portTICK_PERIOD_MS);
  response_char = response.c_str();
  vTaskDelay(200 / portTICK_PERIOD_MS);
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

  vTaskDelay(500 / portTICK_PERIOD_MS);
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

  vTaskDelay(500 / portTICK_PERIOD_MS);

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

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  SerialSIT.println("ATE1");
  response = waitForResponse("OK", 3000);
  debug("HTTP response of ATE1: ");
  debugln(response);
}

// Proposed Rule 45: The Header-Health Check
// Checks if the buffer contains "Modem-speak" or invalid ESP32 entry points
bool isBufferSanityOK(uint8_t *buf, int len, int offset) {
  // 1. Check for UART Cross-talk Signature (uh@\b)
  if (len >= 4) {
    if (buf[0] == 0x75 && buf[1] == 0x68 && buf[2] == 0x40 && buf[3] == 0x08) {
      Serial.println("[OTA] ❌ UART Cross-talk (0x75684008) DETECTED! Poisoned "
                     "chunk. Restarting...");
      return false;
    }
  }

  // 2. Check for "Modem-speak" strings at the BEGINNING of a chunk.
  // BUG-1 Fix: Removed "SampleNo", "Gaps", and "HTTP/" — these are common
  // byte sequences inside valid ESP32 firmware binaries and cause false
  // positives. Only uniquely modem-specific strings remain.
  // BUG-8 Fix: Loop auto-sized via sizeof to prevent count mismatch bugs.
  const char *traps[] = {"+HTTPREAD:", "ERROR\r\n"};
  int scanLimit = min(len, 32); // Only scan first 32 bytes for modem leakage
  for (int t = 0; t < (int)(sizeof(traps) / sizeof(traps[0])); t++) {
    int trapLen = strlen(traps[t]);
    for (int i = 0; i <= scanLimit - trapLen; i++) {
      if (memcmp(&buf[i], traps[t], trapLen) == 0) {
        Serial.printf("[OTA] \u274c Modem-speak '%s' detected at offset %d!\n",
                      traps[t], i);
        return false;
      }
    }
  }

  // 3. Header specific checks for first chunk
  if (offset == 0) {
    if (buf[0] != 0xE9) {
      Serial.printf("[OTA] ❌ Invalid Magic Byte: 0x%02X (Expected 0xE9)\n",
                    buf[0]);
      return false;
    }
  }

  // 4. Large block of repeating bytes (often a sign of UART/Flash sync
  // issues)
  int repeatCount = 0;
  for (int i = 1; i < len; i++) {
    if (buf[i] == buf[0])
      repeatCount++;
  }
  if (repeatCount > (len - 16)) { // 99.9% identical
    Serial.printf("[OTA] ❌ Suspicious data (99%% repeating 0x%02X). Aborting "
                  "chunk.\n",
                  buf[0]);
    return false;
  }

  return true;
}

int setup_ftp() {
  flushSerialSIT(); // Clear leftover data from previous task
  char gprs_xmit_buf[300];
  String response, rssiStr;
  const char *resp;
  const char *ftpServer;
  const char *ftpUser;
  const char *ftpPassword;
  int portName, rssiIndex, result;

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

  send_daily = 0; // Ensure flag is reset after use
  result = -1;

  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
           "AT+CFTPSLOGIN=\"%s\",%d,\"%s\",\"%s\",0", ftpServer, portName,
           ftpUser, ftpPassword); //\r\n removed

  // NEW DOTA
  SerialSIT.println("AT+CFTPSSTOP"); // Ensure a clean slate
  vTaskDelay(500 / portTICK_PERIOD_MS);
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
  // v5.45 Optimized: Wait up to 20s for DNS resolution (BSNL/2G Latency)
  response = waitForResponse("+CDNSGIP:", 20000);
  debugln(response);
  // v5.45: DNS retry on BSNL — error 10 is transient DNS timeout
  if (response.indexOf("+CDNSGIP: 0") != -1 || response.length() < 10) {
    debugln("[FTP] DNS failed, retrying after 3s...");
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    SerialSIT.println(gprs_xmit_buf); // Retry same DNS query
    response = waitForResponse("+CDNSGIP:", 20000);
    debugln(response);
  }

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
  String response;
  int handle_no;
  char gprs_xmit_buf[300];
  int total_no_of_bytes = 0;
  int actual_downloaded = 0;
  (void)handle_no; // suppress unused warning

  debugf1("[OTA] Starting Range-GET download for: %s\n", fileName);

  // v7.16: Rule 27 (Total Silence Protocol)
  // Completely silence the modem BEFORE any binary transfers occur.
  SerialSIT.println("AT+CREG=0");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CGEREP=0");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CNMI=0,0,0,0,0");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CIURC=0");
  waitForResponse("OK", 1000);

  // === STEP 1: HEAD request ===
  SerialSIT.println("AT+HTTPTERM");
  waitForResponse("OK", 2000);
  vTaskDelay(500 / portTICK_PERIOD_MS);
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
  // v5.46 (Rule 1): Bind to CID 1 — MANDATORY for Airtel/BSNL reliability
  SerialSIT.println("AT+HTTPPARA=\"CID\",1");
  if (waitForResponse("OK", 2000).indexOf("OK") == -1) {
    debugln(
        "[OTA] CID binding warning: No OK received, but continuing anyway.");
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
  flushSerialSIT(); // Pre-HEAD flush (Rule 28)
  // HTTP HEAD to determine Content-Length
  SerialSIT.println("AT+HTTPACTION=2");
  String head_resp = waitForResponse("+HTTPACTION:", 30000);
  debugf1("[OTA] HEAD: %s\n", head_resp.c_str());

  // v5.45: The HTTPACTION datalen field for a HEAD request = size of HTTP
  // *response headers*, NOT the firmware file size.
  // Per A7672S manual, we must use AT+HTTPHEAD to read the buffered headers.
  // v5.45: Wait 1s for BSNL network to respond
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  SerialSIT.println("AT+HTTPHEAD");
  String hdr_body = waitForResponse("OK", 10000);
  debugf1("[OTA] Headers: %s\n", hdr_body.c_str());

  // Parse Content-Length: from header body (case-insensitive search)
  int cl_idx = hdr_body.indexOf("Content-Length:");
  if (cl_idx == -1)
    cl_idx = hdr_body.indexOf("content-length:");
  if (cl_idx != -1) {
    int cl_val_start = cl_idx + 15; // skip "Content-Length:"
    while (cl_val_start < (int)hdr_body.length() &&
           hdr_body[cl_val_start] == ' ')
      cl_val_start++; // skip spaces
    total_no_of_bytes = hdr_body.substring(cl_val_start).toInt();
    debugf1("[OTA] Content-Length parsed: %d bytes\n", total_no_of_bytes);
  } else {
    // Fallback: try last field of HTTPACTION (may work on some server
    // configs)
    int lc0 = head_resp.lastIndexOf(",");
    if (lc0 != -1)
      total_no_of_bytes = head_resp.substring(lc0 + 1).toInt();
    debugf1("[OTA] Content-Length fallback from HTTPACTION: %d bytes\n",
            total_no_of_bytes);
  }

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

  // BUG-3 Fix: Lock silence BEFORE Update.begin() to prevent any debug
  // output from leaking to the UART during partition initialization.
  ota_silent_mode = true;
  flushSerialSIT(); // Hard flush the UART before starting partition write

  // === STEP 2: Begin OTA partition write ===
  if (!Update.begin(total_no_of_bytes, U_FLASH)) {
    // Direct Serial print since ota_silent_mode is true
    Serial.printf("[OTA] Update.begin failed: %s\n", Update.errorString());
    Preferences p;
    p.begin("ota-track", false);
    p.putInt("fail_cnt", p.getInt("fail_cnt", 0) + 1);
    p.putString("fail_res", "Begin fail: " + String(Update.errorString()));
    p.end();
    ota_silent_mode = false; // Release silence on early-exit failure
    return;
  }
  // BUG-9 Fix: Duplicate URC silence commands removed — already silenced
  // at function entry (lines 3569-3578). No re-silencing needed here.

  ota_silent_mode = true; // Rule 50: Macro-Silence LOCK before begin()
  Update.onProgress(progressCallBack);
  ota_writing_active = true;
  Serial.println("[OTA] OTA begun. Downloading in 64KB Range chunks...");

  const int RANGE_SIZE =
      65536; // v7.52: 64KB (Balanced for RAM & 2G throughput)
  const int READ_SIZE = 65536;
  uint8_t *buf = (uint8_t *)malloc(READ_SIZE);
  if (!buf) {
    debugln("[OTA] malloc failed!");
    Update.abort();
    ota_writing_active = false;
    return;
  }

  int chunk_retries = 0;
  int consecutive_fails = 0; // Rule 42b: Hardware Reset Trigger
  // Note: ota_silent_mode already set to true before Update.begin() (BUG-3
  // fix)

  // === STEP 3: Download 4KB chunks via Range GET ===
  while (actual_downloaded < total_no_of_bytes) {
    // v7.20: Explicit Offset logging to detect shift-bugs
    debugf("[OTA] Range %d-%d (Progress: %.1f%%)\n", actual_downloaded,
           actual_downloaded + RANGE_SIZE - 1,
           (float)actual_downloaded * 100.0 / total_no_of_bytes);

    if (consecutive_fails >= 3) {
      Serial.println("[OTA] 🚨 3 Consecutive chunk failures! Hard Hardware "
                     "Resetting Modem...");
      digitalWrite(26, LOW);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      esp_task_wdt_reset(); // Keep WDT alive during hardware reset
      digitalWrite(26, HIGH);
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      esp_task_wdt_reset(); // Keep WDT alive
      consecutive_fails = 0;
      // Re-initialize bearer after hard power cycle
      if (!verify_bearer_or_recover()) {
        esp_task_wdt_reset();
        chunk_retries++;
        continue;
      }
    }

    // v7.52: Reverted to One-Chunk-One-Session (Rule 41) for binary safety,
    // but using 64KB chunks to maintain high throughput.
    bool session_active = false;

    if (!session_active) {
      SerialSIT.println("AT+HTTPTERM");
      waitForResponse("OK", 1000);
      vTaskDelay(1000 / portTICK_PERIOD_MS);

      if (!verify_bearer_or_recover()) {
        Serial.println("[OTA] ❌ Bearer Recovery Failed");
        chunk_retries++;
        consecutive_fails++;
        if (chunk_retries > 30)
          break;
        esp_task_wdt_reset();
        continue;
      }

      SerialSIT.println("ATE0");
      waitForResponse("OK", 1000);

      SerialSIT.println("AT+HTTPINIT");
      if (waitForResponse("OK", 5000).indexOf("OK") == -1) {
        Serial.println("[OTA] ⚠️ HTTPINIT Failed");
        chunk_retries++;
        consecutive_fails++;
        esp_task_wdt_reset();
        continue;
      }

      SerialSIT.println("AT+HTTPPARA=\"CID\",1");
      waitForResponse("OK", 2000);
      snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
               "AT+HTTPPARA=\"URL\",\"http://%s:%s/builds/%s\"",
               HEALTH_SERVER_IP, HEALTH_SERVER_PORT, fileName);
      SerialSIT.println(gprs_xmit_buf);
      waitForResponse("OK", 2000);

      session_active = true;
    }

    // Set Range
    int r_start = actual_downloaded;
    int r_end = min(actual_downloaded + RANGE_SIZE - 1, total_no_of_bytes - 1);
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
             "AT+HTTPPARA=\"USERDATA\",\"Range: bytes=%d-%d\"", r_start, r_end);
    SerialSIT.println(gprs_xmit_buf);
    waitForResponse("OK", 2000);

    // Execute GET
    Serial.printf("[OTA] Triggering GET Action (Range %d)... ", r_start);
    SerialSIT.println("AT+HTTPACTION=0");
    String act_resp = waitForResponse("+HTTPACTION:", 95000);
    Serial.println(act_resp); // Show the response status code

    int http_status = 0;
    int chunk_total = 0;
    int first_c = act_resp.indexOf(",");
    int last_c = act_resp.lastIndexOf(",");
    if (first_c != -1 && last_c != -1 && last_c > first_c) {
      http_status = act_resp.substring(first_c + 1, last_c).toInt();
      chunk_total = act_resp.substring(last_c + 1).toInt();
    }

    if (http_status != 200 && http_status != 206) {
      Serial.printf("[OTA] ❌ HTTP Error: %d. Resetting session.\n",
                    http_status);
      session_active = false; // Force re-init on next loop
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      chunk_retries++;
      consecutive_fails++;
      esp_task_wdt_reset();
      continue;
    }

    // READ Data - THE SENSITIVE WINDOW
    vTaskDelay(500 / portTICK_PERIOD_MS);
    flushSerialSIT(); // Hard-Flush (Rule 42b)
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+HTTPREAD=0,%d",
             chunk_total);
    SerialSIT.println(gprs_xmit_buf);

    String read_hdr = "";
    unsigned long t_start = millis();
    bool hdr_found = false;
    while (millis() - t_start < 12000) {
      if (SerialSIT.available()) {
        char c = SerialSIT.read();
        read_hdr += c;

        // Safety valve: prevent unbounded string growth on garbage data
        if (read_hdr.length() > 64) {
          break;
        }

        if (c == '\n' && read_hdr.indexOf("+HTTPREAD:") != -1) {
          hdr_found = true;
          break;
        }
        if (read_hdr.indexOf("ERROR") != -1)
          break;
      }
      vTaskDelay(1 / portTICK_PERIOD_MS);
    }

    if (!hdr_found) {
      chunk_retries++;
      consecutive_fails++;
      esp_task_wdt_reset();
      continue;
    }

    int safe_total = min(chunk_total, READ_SIZE);

    // v5.52: Non-blocking Looped Read with WDT resets
    // readBytes is blocking and triggers WDT on slow 2G connections.
    int got = 0;
    unsigned long startRead = millis();
    while (got < safe_total &&
           (millis() - startRead < 120000)) { // 120s timeout
      if (SerialSIT.available()) {
        buf[got++] = SerialSIT.read();
      } else {
        vTaskDelay(1 / portTICK_PERIOD_MS); // Give CPU a break
      }
      if (got % 1024 == 0)
        esp_task_wdt_reset(); // Frequent resets
    }

    if (got != safe_total) {
      Serial.printf("[OTA] ❌ Read Timeout/Incomplete: got %d of %d\n", got,
                    safe_total);
      flushSerialSIT(); // Clear residual bytes from incomplete read
      chunk_retries++;
      consecutive_fails++;
      esp_task_wdt_reset(); // readBytes failure (likely 15s timeout)
      continue;
    }

    // Consume the Tail (The 6-Byte Residual \r\nOK\r\n)
    unsigned long tail_start = millis();
    while (millis() - tail_start < 1000) {
      if (SerialSIT.find("OK"))
        break;
      vTaskDelay(1 / portTICK_PERIOD_MS);
    }

    // Rule 45: The Header-Health Check & Crosstalk Trap
    if (!isBufferSanityOK(buf, got, actual_downloaded)) {
      if (buf[0] == 0x75 && buf[1] == 0x68 && buf[2] == 0x40 &&
          buf[3] == 0x08) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP.restart(); // Signature matched crosstalk, instant restart
      }
      flushSerialSIT(); // BUG-4 Fix: Clear residual UART bytes on sanity fail
      session_active = false; // Something is deeply wrong, reset session
      chunk_retries++;
      consecutive_fails++;
      esp_task_wdt_reset();
      continue;
    }

    // Write to Flash
    int bytes_to_write = min(got, total_no_of_bytes - actual_downloaded);
    if (Update.write(buf, bytes_to_write) == (size_t)bytes_to_write) {
      actual_downloaded += bytes_to_write;
      chunk_retries = 0;
      consecutive_fails = 0;

      // Progress log (Direct print to bypass silence)
      Serial.printf("[OTA] Progress: %d / %d (HEAD: %02X %02X %02X %02X)\n",
                    actual_downloaded, total_no_of_bytes, buf[0], buf[1],
                    buf[2], buf[3]);
    } else {
      Serial.printf("[OTA] Flash Write ERROR: %d - %s\n", Update.getError(),
                    Update.errorString());
      Update.abort(); // v7.68: Terminate partition session on write failure
      break;
    }

    flushSerialSIT(); // Final tail cleanup
    vTaskDelay(100 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();
    session_active = false; // Force re-init for next chunk (Rule 41)
  }

  free(buf);
  ota_writing_active = false;
  ota_silent_mode = false; // Rule 43: Resume normal logging

  // Restore Modem settings after binary transfer
  SerialSIT.println("ATE1");
  waitForResponse("OK", 500);
  SerialSIT.println("AT+CREG=1");
  waitForResponse("OK", 500);
  SerialSIT.println("AT+CGEREP=2");
  waitForResponse("OK", 500);

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
      int errCode = Update.getError();
      debugf("[OTA] Update.end FAILED. Code: %d, Str: %s\n", errCode,
             err.c_str());
      Update.printError(Serial); // BUG-10 Fix: Print to console, not modem UART

      Preferences p;
      p.begin("ota-track", false);
      p.putInt("fail_cnt", p.getInt("fail_cnt", 0) + 1);
      p.putString("fail_res", "End fail code: " + String(errCode));
      p.end();
    }
  } else {
    debugf("[OTA] Incomplete: %d/%d. Aborting.\n", actual_downloaded,
           total_no_of_bytes);
    Update.abort();
    Preferences p;
    p.begin("ota-track", false);
    p.putInt("fail_cnt", p.getInt("fail_cnt", 0) + 1);
    p.putString("fail_res", "Incomplete download");
    p.end();
  }

  // Restore Modem URCs (v7.16 Restoration)
  SerialSIT.println("AT+CREG=1");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CGEREP=2");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CNMI=2,1,0,0,0");
  waitForResponse("OK", 1000);

  // v7.06: Final Cleanup & Restoration
  SerialSIT.println("AT+HTTPTERM");
  waitForResponse("OK", 2000);
  SerialSIT.println("AT+CGEREP=2");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CREG=1");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CEREG=1");
  waitForResponse("OK", 1000);

  debugln("[OTA] Done.");
}

void copyFromSPIFFSToFS(char *dateFile) {
  String response;
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
    vTaskDelay(200 / portTICK_PERIOD_MS);
    csqstr = strstr(response_char, "+FSOPEN:");
    if (csqstr == NULL) {
      debugln("Error: +FSOPEN not found in response");
      return; // Exit if response is invalid
    }
    sscanf(csqstr, "+FSOPEN: %d,", &handle_no);

    File file1 = SPIFFS.open(SPIFFSFile, FILE_READ);
    if (!file1) {
      debugln("C5 FIX: Failed to open SPIFFS source file. Aborting "
              "copyFromSPIFFSToFS.");
      return;
    }
    debug("File size of the SPIFFS file is ");
    debugln(file1.size());
    fileSize = file1.size();
    file1.seek(s);
    content = file1.readString(); // Read the rest of the file

    // filehandle,length,timeout
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSWRITE=%d,%d,10\r\n",
             handle_no,
             fileSize); // 0 : if the file does not exist, it will be created
    vTaskDelay(200 / portTICK_PERIOD_MS);
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
    vTaskDelay(200 / portTICK_PERIOD_MS);

    ftp_login_flag = setup_ftp();

    if (ftp_login_flag == 1) {
      char *modulePath = (fileName[0] == '/') ? &fileName[1] : fileName;
      snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CFTPSPUTFILE=\"C:/%s\",0",
               modulePath); 
      SerialSIT.println(gprs_xmit_buf); // FTP client context
      response = waitForResponse("+CFTPSPUTFILE: 0", 150000); // 120000
      debug("Response of AT+CFTPSPUTFILE ");
      debugln(response);

      SerialSIT.println("AT+CFTPSLOGOUT");
      response = waitForResponse("+CFTPSLOGOUT: 0", 9000);
      debug("Response of AT+CFTPSLOGOUT is ");
      debugln(response);
      vTaskDelay(200 / portTICK_PERIOD_MS);

      SerialSIT.println("AT+CFTPSSTOP");
      response = waitForResponse("OK", 5000);
      debug("Response of AT+CFTPSSTOP is ");
      debugln(response);
      vTaskDelay(200 / portTICK_PERIOD_MS);

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
    vTaskDelay(1 / portTICK_PERIOD_MS);
    esp_task_wdt_reset(); // Keep watchdog happy during long AT command
                          // waits

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

int read_line(char *src, char *dest, int max_len, char delim_chr) {
  int i, j;
  char c;
  if (!dest || max_len <= 0)
    return 0;
  *dest = 0;
  j = 0;
  for (i = 0; (i < (max_len - 1)); i++, j++) { // Use max_len to prevent OOB
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
      dest[i] = c;     // Standard indexing
      dest[i + 1] = 0; // Safe null termination
    }
  }
  return (j);
}

// Function to convert GMT to IST
void convert_gmt_to_ist(struct tm *gmt_time) {
  // Adding 5 hours and 30 minutes to GMT for IST conversion
  gmt_time->tm_hour += 5;
  gmt_time->tm_min += 30;

  // Normalize the time in case minutes overflow (i.e., more than 60
  // minutes)
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
  // v5.45: Carrier Congestion Breather after FTP (Special for BSNL)
  vTaskDelay(5000 / portTICK_PERIOD_MS);

  // v5.45: Purity Guard - Silence all network noise during transmission
  SerialSIT.println("AT+CGEREP=0");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CREG=0");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CEREG=0");
  waitForResponse("OK", 1000);

  if (diag_pd_count == 0 && current_year > 2024) {
    debugln("[Health] Counters zero, reconstructing...");
    reconstructSentMasks();
  }

  // --- PREPARE PAYLOAD (ZERO GAP) ---
  checkRainfallIntegrity();
  char cleanStn[16];
  strncpy(cleanStn, station_name, 15);
  cleanStn[15] = '\0';
  int slen = strlen(cleanStn);
  while (slen > 0 && cleanStn[slen - 1] == ' ') {
    cleanStn[slen - 1] = '\0';
    slen--;
  }

  bool unresolvedPD = false, unresolvedNDM = false;
  bool dummyPD = false, dummyNDM = false;
  analyzeFileHealth(diag_sent_mask_cur, &diag_net_data_count, &dummyPD,
                    &dummyNDM);
  analyzeFileHealth(diag_sent_mask_prev, &diag_net_data_count_prev,
                    &unresolvedPD, &unresolvedNDM);

  char h_status[256] = "";
#define H_FAULT(f)                                                             \
  do {                                                                         \
    size_t _rem = sizeof(h_status) - strlen(h_status) - 1;                     \
    if (h_status[0] != '\0' && _rem > 1)                                       \
      strncat(h_status, "_", _rem--);                                          \
    _rem = sizeof(h_status) - strlen(h_status) - 1;                            \
    if (_rem > 0)                                                              \
      strncat(h_status, f, _rem);                                              \
  } while (0)

  if (diag_last_reset_reason == 15)
    H_FAULT("BROWNOUT");
  if (diag_last_reset_reason == 7 || diag_last_reset_reason == 8 ||
      diag_last_reset_reason == 9 || diag_last_reset_reason == 13 ||
      diag_last_reset_reason == 16)
    H_FAULT("WDOG");
  if (!diag_rtc_battery_ok || current_year < 2025)
    H_FAULT("RTC_FAIL");
  if (unresolvedPD)
    H_FAULT("PD");
  if (strcmp(diag_cdm_status, "OK") != 0 && current_hour >= 9 &&
      diag_last_rollover_day > 0) // v7.83: Skip CDM on very first boot
    H_FAULT("CDM");
  if (unresolvedNDM)
    H_FAULT("NDM");
  if (lati == 0.0 && longi == 0.0)
    H_FAULT("NO_GPS");
  if (h_status[0] == '\0')
    strcpy(h_status, "OK");

  char sensor_info[48];
  snprintf(sensor_info, sizeof(sensor_info), "TH-%s,WS-%s,WD-%s",
           (hdcType == HDC_UNKNOWN ? "FAIL" : "OK"), (ws_ok ? "OK" : "FAIL"),
           (wd_ok ? "OK" : "FAIL"));

  char gps_str[32];
  if (lati == 0.0 && longi == 0.0)
    snprintf(gps_str, sizeof(gps_str), "NA");
  else
    snprintf(gps_str, sizeof(gps_str), "%.6f,%.6f", lati, longi);

  int spiffs_used = SPIFFS.usedBytes() / 1024;
  int spiffs_total = SPIFFS.totalBytes() / 1024;

  // Count records in unsent.txt and ftpunsent.txt for unsent_count
  int unsent_count = 0;
  const char *unsent_files[] = {"/unsent.txt", "/ftpunsent.txt"};
  for (int i = 0; i < 2; i++) {
    File uf = SPIFFS.open(unsent_files[i], FILE_READ);
    if (uf) {
      while (uf.available()) {
        if (uf.read() == '\n')
          unsent_count++;
      }
      uf.close();
    }
  }

  // Construction of Calibration String for System 0 & 2
  char calib_report[48] = "NA";
  if (calib_year > 2000) {
    snprintf(calib_report, sizeof(calib_report), "CLB-%s (%04d-%02d-%02d)",
             (calib_sts == 1 ? "OK" : "FAIL"), calib_year, calib_month,
             calib_day);
  }

  // v7.92: Command Feedback Payload Construction
  char feedback[128] = "";
  if (last_cmd_id > 0) {
    snprintf(feedback, sizeof(feedback),
             ",\"last_cmd_id\":%d,\"last_cmd_res\":\"%s\"", last_cmd_id,
             last_cmd_res);
  }

  // v7.83: Full payload — all columns now populated on server
  // Buffer: ~1280 bytes to accommodate all new fields
  char jsonBody[1280];
  int msgLen = snprintf(
      jsonBody, sizeof(jsonBody),
      "{\"stn_id\":\"%s\",\"unit_type\":\"%s\",\"system\":%d,"
      "\"health_sts\":\"%s\",\"sensor_sts\":\"%s\",\"rtc_ok\":%d,"
      "\"bat_v\":%.2f,\"sol_v\":%.2f,\"signal\":%d,"
      "\"net_cnt\":%d,"
      "\"http_suc_cnt\":%d,\"http_ret_cnt\":%d,\"ftp_suc_cnt\":%d,"
      "\"net_cnt_prev\":%d,\"prev_stored\":%d,"
      "\"http_suc_cnt_prev\":%d,\"http_ret_cnt_prev\":%d,\"ftp_suc_cnt_prev\":%"
      "d,"
      "\"reg_fails\":%d,\"reg_fail_reason\":\"%s\",\"reset_reason\":%d,"
      "\"spiffs_kb\":%d,\"spiffs_total_kb\":%d,"
      "\"unsent_count\":%d,"
      "\"ver\":\"%s\",\"iccid\":\"%s\",\"gps\":\"%s\","
      "\"cdm_sts\":\"%s\","
      "\"calib\":\"%s\"," // v7.86
      "\"ndm_cnt\":%d,\"pd_cnt\":%d"
      "%s}", // v7.92: Custom Feedback Block
      cleanStn, UNIT, SYSTEM, h_status, sensor_info,
      (diag_rtc_battery_ok ? 1 : 0), li_bat_val, solar_val, signal_lvl,
      diag_net_data_count,          // net_cnt        → Tdy Sent (Live)
      diag_http_success_count,      // http_suc_cnt   → Tdy HTTP
      diag_http_retry_count,        // http_ret_cnt   → Tdy Backlogs
      diag_ftp_success_count,       // ftp_suc_cnt    → Tdy Backlogs (FTP)
      diag_net_data_count_prev,     // net_cnt_prev   → Ydy Sent
      diag_pd_count_prev,           // prev_stored    → Ydy Stored
      diag_http_success_count_prev, // http_suc_cnt_prev → Ydy HTTP
      diag_http_retry_count_prev,   // http_ret_cnt_prev → Ydy Backlogs
      diag_ftp_success_count_prev,  // ftp_suc_cnt_prev  → Ydy Backlogs (FTP)
      diag_gprs_fails, diag_reg_fail_type, // reg_fails & reason
      diag_last_reset_reason,              // reset_reason
      spiffs_used, spiffs_total,           // spiffs_kb / spiffs_total_kb
      unsent_count,                        // unsent_count
      UNIT_VER, cached_iccid, gps_str,
      diag_cdm_status, // cdm_sts → server evaluates CDM from this
      calib_report,    // calibration info
      diag_ndm_count, diag_pd_count,
      feedback); // v7.92

  if (useJitter)
    vTaskDelay(random(0, 5000) / portTICK_PERIOD_MS);
  else
    vTaskDelay(2000 / portTICK_PERIOD_MS);

  bool success = false;
  for (int attempt = 1; attempt <= 3; attempt++) {
    debugf1("[Health] Attempt %d/3\n", attempt);
    if (!verify_bearer_or_recover())
      continue;

    // v7.79: Total Silence Protocol (Rule 10/27)
    SerialSIT.println("AT+CGEREP=0");
    waitForResponse("OK", 1000);

    // Rule 12/24: Extended Breather after host switch
    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 5000);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    flushSerialSIT();

    SerialSIT.println("AT+HTTPINIT");
    if (waitForResponse("OK", 5000).indexOf("OK") == -1) {
      debugln("[Health] ❌ HTTPINIT Failed. Bearer Nuke...");
      SerialSIT.println("AT+CGACT=0,1");
      waitForResponse("OK", 5000);
      continue;
    }

    bool step_fail = false;
    // Rule 59: Lean Sequence (Fast burst, no gaps)
    char ht_url[150];
    snprintf(ht_url, sizeof(ht_url),
             "AT+HTTPPARA=\"URL\",\"http://%s:%s/health\"", HEALTH_SERVER_IP,
             HEALTH_SERVER_PORT);
    SerialSIT.println(ht_url);
    waitForResponse("OK", 2000);

    SerialSIT.println("AT+HTTPPARA=\"CID\",1");
    waitForResponse("OK", 1000);

    SerialSIT.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
    waitForResponse("OK", 1000);
    SerialSIT.println("AT+HTTPPARA=\"ACCEPT\",\"*/*\"");
    waitForResponse("OK", 1000);

    // Headers & Format
    if (!step_fail) {
      debugf1("[Health] Payload size: %d bytes\n", msgLen);

      char ht_data_cmd[64];
      // v7.75: Increased prompt wait to 10s for high-latency 2G
      snprintf(ht_data_cmd, sizeof(ht_data_cmd), "AT+HTTPDATA=%d,5000", msgLen);
      SerialSIT.println(ht_data_cmd);

      String act = "";
      if (waitForResponse("DOWNLOAD", 12000).indexOf("DOWNLOAD") != -1) {
        vTaskDelay(200 / portTICK_PERIOD_MS);
        SerialSIT.write(jsonBody, msgLen);
        if (waitForResponse("OK", 6000).indexOf("OK") != -1) {

          SerialSIT.println("AT+HTTPACTION=1");
          waitForResponse("OK", 3000);

          // v7.79: Increased total wait to 45s for BSNL 2G saturation
          act = waitForResponse("+HTTPACTION:", 45000);
          debugln("[Health] Resp: " + act);

          if (act.indexOf("200") != -1) {
            success = true;
          } else if (act.indexOf("714") != -1 || act.indexOf("706") != -1) {
            // Rule 19/104: Nuke on Socket Zombie
            debugln("[Health] 🧟 Zombie Socket (714/706). Nuking Bearer "
                    "Context...");
            SerialSIT.println("AT+HTTPTERM");
            waitForResponse("OK", 2000);
            SerialSIT.println("AT+CGACT=0,1");
            waitForResponse("OK", 5000);
            vTaskDelay(5000 / portTICK_PERIOD_MS);
          }
          vTaskDelay(500 / portTICK_PERIOD_MS);
          SerialSIT.println("AT+HTTPREAD=0,512");
          String body = waitForResponse("+HTTPREAD:", 10000);
          debugln("[Health] Body: " + body);

          // v7.92: Parse Command ID for feedback loop
          int idTag = body.indexOf("\"id\"");
          if (idTag != -1) {
            int valStart = body.indexOf(":", idTag);
            if (valStart != -1) {
              String idStr = "";
              for (int k = valStart + 1; k < body.length(); k++) {
                char ch = body[k];
                if (isdigit(ch))
                  idStr += ch;
                else if (idStr.length() > 0)
                  break;
              }
              if (idStr.length() > 0) {
                last_cmd_id = idStr.toInt();
                strcpy(last_cmd_res, "PENDING"); // Reset result
              }
            }
          }

          // Command Processing
          sync_rtc_from_server_tm(body.c_str(), false);
          if (body.indexOf("\"REBOOT\"") != -1) {
            force_reboot = true;
            strcpy(last_cmd_res, "Success: Rebooting");
          }
          if (body.indexOf("\"OTA_CHECK\"") != -1) {
            force_ota = true;
            // Parse target binary if present (p or cmd_param)
            int pTag = body.indexOf("\"p\"");
            if (pTag == -1)
              pTag = body.indexOf("\"cmd_param\"");
            if (pTag != -1) {
              int valStart = body.indexOf(":", pTag);
              if (valStart != -1) {
                int q1 = body.indexOf("\"", valStart);
                if (q1 != -1) {
                  int q2 = body.indexOf("\"", q1 + 1);
                  if (q2 != -1) {
                    String param = body.substring(q1 + 1, q2);
                    strncpy(ota_cmd_param, param.c_str(),
                            sizeof(ota_cmd_param) - 1);
                    ota_cmd_param[sizeof(ota_cmd_param) - 1] = '\0';
                  }
                }
              }
            }
          }
          if (body.indexOf("\"FTP_BACKLOG\"") != -1)
            force_ftp = true;
          if (body.indexOf("\"FTP_DAILY\"") != -1) {
            force_ftp_daily = true;
            // Parse target date if present (p)
            int pTag = body.indexOf("\"p\"");
            if (pTag != -1) {
              int valStart = body.indexOf(":", pTag);
              if (valStart != -1) {
                int q1 = body.indexOf("\"", valStart);
                if (q1 != -1) {
                  int q2 = body.indexOf("\"", q1 + 1);
                  if (q2 != -1) {
                    String param = body.substring(q1 + 1, q2);
                    strncpy(ftp_daily_date, param.c_str(),
                            sizeof(ftp_daily_date) - 1);
                    ftp_daily_date[sizeof(ftp_daily_date) - 1] = '\0';
                  }
                }
              }
            }
          }
          if (body.indexOf("\"GET_GPS\"") != -1)
            force_gps_refresh = true;
          if (body.indexOf("\"CLEAR_FTP_QUEUE\"") != -1)
            force_clear_ftp_queue = true;

          if (body.indexOf("\"INTERVAL\"") != -1) {
            int pTag = body.indexOf("\"p\"");
            if (pTag != -1) {
              int valStart = body.indexOf(":", pTag);
              if (valStart != -1) {
                String valStr = "";
                for (int k = valStart + 1; k < body.length(); k++) {
                  if (isdigit(body[k])) valStr += body[k];
                  else if (valStr.length() > 0) break;
                }
                if (valStr.length() > 0) {
                  int mins = valStr.toInt();
                  Preferences prefs;
                  prefs.begin("sys-config", false);
                  if (mins <= 15) {
                    test_health_every_slot = 1;
                    strcpy(last_cmd_res, "Success: 15m Mode");
                  } else {
                    test_health_every_slot = 0;
                    strcpy(last_cmd_res, "Success: 24h Mode");
                  }
                  prefs.putInt("test_health", test_health_every_slot);
                  prefs.end();
                }
              }
            }
          }


          SerialSIT.println("AT+HTTPTERM");
          waitForResponse("OK", 1000);
          break; // Success exit
        } else {
          debugln("[Health] ❌ HTTP Action Failed, Resp: " + act);
        }
      } else {
        debugln("[Health] ❌ Data Load Timeout/Error");
      }
    } else {
      debugln("[Health] ❌ DOWNLOAD Prompt Failed");
    }

    // Final cleanup before retry
    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 2000);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }

  SerialSIT.println("AT+CGEREP=2");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CREG=1");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CEREG=1");
  waitForResponse("OK", 1000);
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
