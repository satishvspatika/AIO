#include "globals.h"

int active_cid = 1;      // Default to 1
bool http_ready = false; // v5.42: Track if HTTPINIT succeeded for this cycle
static String content = ""; // Phase 5 Fix: Scoped locally to GPRS core to prevent Core 1 cross-contamination

// Helper to clear UART buffer to prevent stale data contamination between tasks
void flushSerialSIT() {
  // P2 fix v5.65: Added 500ms hard deadline.
  // Without it, a modem URC storm (continuous bearer-bounce events) could cause
  // this loop to spin indefinitely: data arrives faster than the 1ms drain,
  // so SerialSIT.available() never returns false, hanging the GPRS task.
  unsigned long deadline = millis() + 500;
  while (SerialSIT.available() && millis() < deadline) {
    SerialSIT.read();
    esp_task_wdt_reset();
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
  int raw_drift = abs(srv_total_sec - rtc_total_sec);
  
  // v5.66: 🚨 CRITICAL FIX - Handle Midnight Rollover!
  // If Server=23:59:50 and RTC=00:00:10, drift is technically 20s, not 86380s.
  int drift = (raw_drift > 43200) ? (86400 - raw_drift) : raw_drift;

  // Threshold: force path=45s, daily path=90s
  int threshold = force ? 45 : 90;
  debugf("[RTC-Sync] Drift = %ds (threshold %ds, force=%d)\n", drift, threshold,
         (int)force);

  if (drift < threshold && raw_drift < 43200) { 
    // Extra safety: only skip if it's literally just a few seconds off.
    // If we wrapped days, we probably want to sync anyway just to lock dates.
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
    // Write perfectly translated server IST time and date to physical RTC
    rtc.adjust(DateTime(s_yy + 2000, s_mm, s_dd, s_hh, s_mi, s_ss));
    xSemaphoreGive(i2cMutex);
  }

  // v5.66: 🚨 CRITICAL FIX - Update ALL globals immediately!
  // If the physical RTC battery died and it woke up in 1970, we MUST legally
  // update the current_day/year variables or else the rest of this waking cycle 
  // will create data filed under 1970 despite the physical RTC being fixed!
  current_day = s_dd;
  current_month = s_mm;
  current_year = s_yy + 2000;
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
  static int target_fld = FLD_SEND_STATUS; // v5.50: Moved to top for sequential queue scope
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
        sync_mode == eStartupGPS || sync_mode == eHealthStart) {
      target_fld = (sync_mode == eSMSStart) ? FLD_SEND_STATUS :
                   (sync_mode == eGPSStart) ? FLD_SEND_GPS :
                   (sync_mode == eHealthStart) ? FLD_SEND_HEALTH : FLD_SEND_GPS;

      // Clear the queued pending flag immediately to prevent double-execution
      if (sync_mode == eSMSStart) pending_manual_status = false;
      else if (sync_mode == eGPSStart) pending_manual_gps = false;
      else if (sync_mode == eHealthStart) pending_manual_health = false;

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

          // Re-verify network & APN for manual triggers (Ensures connection is live)
          get_signal_strength();
          get_network();
          get_registration();
          get_a7672s();

          // v5.68 User Request: SEPARATE 'GET GPS' and 'HEALTH' from 'SEND STATUS'
          // We no longer block the LCD interface for 3 minutes trying to send HTTP health 
          // or waiting for physical GPS satellite locks just to send an SMS text!
          strcpy(ui_data[target_fld].bottomRow, "SENDING SMS...  ");
          vTaskDelay(2000 / portTICK_PERIOD_MS);
          
          prepare_and_send_status(universalNumber);

          if (msg_sent) {
            strcpy(ui_data[target_fld].bottomRow, "SMS SUCCESS     ");
          } else {
            strcpy(ui_data[target_fld].bottomRow, "SMS FAILED      ");
          }
          show_now = 1;
          vTaskDelay(3000 / portTICK_PERIOD_MS);
          strcpy(ui_data[target_fld].bottomRow, "YES ?           ");
          show_now = 1;

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
        } else if (sync_mode == eStartupGPS || sync_mode == eHealthStart) {
          debugln(
              "[GPRS] Startup/Station Change Triggered GPS & Health Report");
          if (signal_strength == 0) {
            get_signal_strength();
            get_network();
            get_registration();
            get_a7672s();
          }
          strcpy(ui_data[target_fld].bottomRow, "GETTING GPS...  ");
          show_now = 1;
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
            strcpy(ui_data[target_fld].bottomRow, "GPS FAILED      ");
          }
          vTaskDelay(2000 /
                     portTICK_PERIOD_MS); // Allow user to see coordinates

#if ENABLE_HEALTH_REPORT == 1
          if (target_fld == FLD_SEND_HEALTH) {
             strcpy(ui_data[target_fld].bottomRow, "SENDING HEALTH..");
             show_now = 1;
          }
          bool health_ok = send_health_report(false); // Manual: No Jitter
          if (target_fld == FLD_SEND_HEALTH) {
             if (health_ok) strcpy(ui_data[target_fld].bottomRow, "HEALTH SUCCESS! ");
             else strcpy(ui_data[target_fld].bottomRow, "HEALTH FAILED   ");
             show_now = 1;
             vTaskDelay(3000 / portTICK_PERIOD_MS);
             strcpy(ui_data[target_fld].bottomRow, "YES ?           ");
             show_now = 1;
          }
#else
          debugln("[Health] Startup send skipped (disabled).");
#endif
          sync_mode = eSMSStop;
          msg_sent = 1; // Mark block as handled so it doesn't print "SEND FAILED"
        }

        // Unified result display for Manual Triggers
        if (msg_sent == 1) {
          if (target_fld == FLD_SEND_GPS) {
            snprintf(ui_data[target_fld].bottomRow, 17, "%0.3f,%0.3f", lati, longi);
            show_now = 1;
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            strcpy(ui_data[target_fld].bottomRow, "YES ?           ");
            show_now = 1;
          } else if (target_fld != FLD_SEND_HEALTH && target_fld != FLD_SEND_STATUS) {
            strcpy(ui_data[target_fld].bottomRow, "SENT SUCCESS    ");
            show_now = 1;
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            strcpy(ui_data[target_fld].bottomRow, "YES ?           ");
            show_now = 1;
          }
        } else {
          if (target_fld != FLD_SEND_HEALTH && target_fld != FLD_SEND_STATUS) {
            strcpy(ui_data[target_fld].bottomRow, "SEND FAILED     ");
            show_now = 1;
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            strcpy(ui_data[target_fld].bottomRow, "YES ?           ");
            show_now = 1;
          }
        }
        
        show_now = 1; // Trigger UI refresh
        vTaskDelay(5000 / portTICK_PERIOD_MS); // Wait for user to read

        // After showing result, revert to idle state
        if (target_fld == FLD_SEND_GPS || target_fld == FLD_SEND_STATUS) {
          strcpy(ui_data[target_fld].bottomRow, "YES ?           ");
        }
        
        msg_sent = 0; // Reset for next trigger
        show_now = 1;

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
            sync_mode == eStartupGPS || sync_mode == eHealthStart)
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
        if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(10000)) == pdTRUE) {
          debugln("[GPRS] Powering On...");
          signal_strength = -111;
          signal_lvl = -111; 
          strcpy(reg_status, "NA");
          gprs_pdp_ready = false; // Reset PDP state
          start_gprs();
          xSemaphoreGive(modemMutex);
        }
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
          httpInitiated = true; // v5.65 P4: Mark cycle as started for responsive cleanup block

          // v5.48 Daily Health Triggering (11:00 AM Primary)
          bool is_health_time = false;
#if ENABLE_HEALTH_REPORT == 1
          if (test_health_every_slot == 1) {
            is_health_time = true; // Every 15 mins
          } else if (test_health_every_slot == 0) {
            if (current_hour == 11 && health_last_sent_day != current_day) {
              is_health_time = true; // Daily 11 AM
            } else if (current_hour == 12 && current_min < 20 && health_last_sent_day != current_day) {
              is_health_time = true; // v5.66: Graceful fallback for 11AM BSNL network congestion
            }
          }
          // If test_health_every_slot == 2, it remains false (Disabled)
#endif

          // v5.66: CDM True-Failure Fallback Check
          // If the Morning closing data (08:30) and Health windows (11:00-12:20) both pass
          // without success, the closing window is permanently missed for this day.
          if (strcmp(diag_cdm_status, "PENDING") == 0 && current_hour >= 13 && health_last_sent_day != current_day) {
            strcpy(diag_cdm_status, "FAIL");
            debugln("[Health] CDM window entirely missed today. Flagging FAIL.");
          }

#if ENABLE_HEALTH_REPORT == 1
          // v5.76 Post-OTA Confirmation Health Report Check
          bool is_ota_confirm = false;
          if (diag_fw_just_updated && gprs_mode == eGprsSignalOk) {
            is_ota_confirm = true;
            diag_fw_just_updated = false; // Reset persistent flag
            strcpy(diag_cdm_status, "Firmware Updated");
            // Refresh Location via Fresh CLBS
            get_gps_coordinates();
          }

          // v5.55: One-time sensor fault trigger (Non-11:00 AM)
          bool is_sensor_fault_trigger = false;
          bool has_sensor_issue = diag_temp_cv || diag_hum_cv || diag_ws_cv || 
                                 diag_temp_erv || diag_hum_erv || diag_ws_erv || 
                                 diag_temp_erz || diag_hum_erz || diag_rain_jump ||
                                 diag_rain_reset || diag_rain_calc_invalid;

          if (has_sensor_issue && !diag_sensor_fault_sent_today && current_hour != 11) {
              is_sensor_fault_trigger = true;
              debugln("[Health] 🚨 Sensor issue detected! Triggering one-time fault report.");
          }

          if (is_health_time || is_ota_confirm || is_sensor_fault_trigger) {
            health_in_progress = true; // Block deep sleep
          }
#endif

          if (!skip_primary_http) {
            debugln(
                "[BATT] Proceeding with Spatika Upload regardless of battery "
                "voltage.");
            sync_mode = eHttpStarted;
            send_http_data();
            primary_data_delivered =
                (success_count == 1); // v5.51 Track session success
          } else {
            debugln("[SCHED] Skipping Duplicate/Fresh Upload. Checking Backlog/Health...");
            primary_data_delivered = true; // Assume 'Success' to allow backlog
#if (SYSTEM == 1 || SYSTEM == 2)
            // v7.65: Handle backlog push sequentially in GPRS task
            if (gprs_mode == eGprsSignalOk && (signal_lvl > -96)) {
              send_unsent_data();
            }
#endif
            sync_mode = eHttpStop; // v5.51 FIX: Advance state to prevent infinite loop!
          }


          // v5.65 P4: HIGH RESPONSIVENESS - If a manual command is pending, prioritize it
          // over the lengthy/unreliable automated health report.
          if (pending_manual_status || pending_manual_gps || pending_manual_health) {
            debugln("[GPRS] 💡 Priority Interrupt: Manual command detected during automated cycle.");
            // We let the end-of-cycle block (line ~600) handle the actual transition
            // but we skip the optional automated health report below to speed things up.
            health_in_progress = false; 
          }

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
              
              // v5.55: One-time fault successful - mark it so we don't spam
              if (is_sensor_fault_trigger) {
                diag_sensor_fault_sent_today = true;
                debugln("[Health] One-time sensor fault report marked as SENT.");
              }
              
              debugln("[Health] \u2705 Report sent successfully!");
              if (strcmp(diag_cdm_status, "PENDING") == 0) {
                strcpy(diag_cdm_status,
                       "OK"); // v7.67: CDM successfully delivered
              }
            } else {
              debugln("[Health] \u274c Report failed - will retry next slot if "
                      "window open.");
              // v5.66: Removed premature diag_cdm_status="FAIL" assignment to allow fallback slot retries
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
          if (skip_primary_http) {
            debugln("[GPRS] Signal too weak, but data already queued cleanly by scheduler skip.");
          } else {
            debugln("[GPRS] Signal too weak for HTTP/FTP. Storing data to backlog.");
            store_current_unsent_data();
          }
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
      if (strlen(ftp_daily_date) >= 8) { // Validate date string to prevent root directory iteration
        send_ftp_file(ftp_daily_date, true);
      } else {
        debugln("[CMD] Error: Invalid FTP date format. Skipping FTP_DAILY.");
      }
#endif
      force_ftp_daily = false;
    }

    if (force_ota) {
      snprintf(ui_data[FLD_SEND_STATUS].bottomRow, sizeof(ui_data[FLD_SEND_STATUS].bottomRow), "OTA UPDATING...");
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
      snprintf(ui_data[FLD_SEND_STATUS].bottomRow, sizeof(ui_data[FLD_SEND_STATUS].bottomRow), "OTA FINISHED");
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

    // v7.94: DELETE_DATA — Server-requested Factory Reset
    if (force_delete_data) {
      debugln("[CMD] DELETE_DATA: Factory Reset requested by server...");
      strcpy(last_cmd_res, "Success: Deleting & Reboot");

      // Perform deletion (Matches LCD Factory Reset logic)
      SPIFFS.remove("/unsent.txt");
      SPIFFS.remove("/ftpunsent.txt");
      SPIFFS.remove("/unsent_pointer.txt");

      File root = SPIFFS.open("/");
      File file = root.openNextFile();
      while (file) {
        String fileName = file.name();
        String fullPath = fileName.startsWith("/") ? fileName : "/" + fileName;

        // List of files to PRESERVE (Only basic station info)
        if (fullPath == "/station.doc" || fullPath == "/station.txt" ||
            fullPath == "/rf_fw.txt") {
          debug("Preserving: ");
          debugln(fullPath);
        } else {
          debug("Deleting: ");
          debugln(fullPath);
          SPIFFS.remove(fullPath);
        }
        file.close();
        file = root.openNextFile();
      }
      root.close();

      reset_all_diagnostics(); // Reset all RTC RAM diagnostic counters

      debugln("[CMD] Factory Reset Complete. Restarting...");
      vTaskDelay(3000 / portTICK_PERIOD_MS);
      ESP.restart();
    }

    // v7.90: Final Cycle Reset - Move here to ensure COMMANDS (OTA/FTP/GPS)
    // finish before loopTask triggers deep sleep.
    // v7.90: Final Cycle Reset - Move here to ensure COMMANDS (OTA/FTP/GPS)
    // finish before loopTask triggers deep sleep.
    if (httpInitiated) {
      debugln("[GPRS] Cycle fully complete (including Commands).");
      
      // v5.50: Sequential Handling — Check if a manual LCD command was queued
      if (pending_manual_status) {
        debugln("[GPRS] 💡 Found queued manual Status/SMS request. Handling now...");
        pending_manual_status = false;
        target_fld = FLD_SEND_STATUS; // v5.50 must set target_fld for the eSMSStart runner
        sync_mode = eSMSStart;       // Transition to SMS flow before stopping
      } else if (pending_manual_gps) {
        debugln("[GPRS] 💡 Found queued manual GPS request. Handling now...");
        pending_manual_gps = false;
        target_fld = FLD_SEND_GPS;
        sync_mode = eGPSStart;
      } else if (pending_manual_health) {
        debugln("[GPRS] 💡 Found queued manual Health request. Handling now...");
        pending_manual_health = false;
        target_fld = FLD_SEND_HEALTH;
        sync_mode = eHealthStart;
      } else {
        debugln("[GPRS] No queued commands. Allowing sleep.");
        sync_mode = eHttpStop;
        httpInitiated = false;
      }
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
    signal_strength = -111; // v5.52 Fix: Use MISSING_DATA (-111) not PREV_DAY (-114)
    debugln("[SIM] ERROR DETECTED (Timeout or CME).");
    strcpy(reg_status, "SIM ERROR");
    strcpy(diag_reg_fail_type, "SIM_ERR");

    // v5.50: Threshold raised from 6 (1.5h) to 13 (3h15m).
    // We now wait until at least one 3-hourly FTP scheduled window has
    // definitively passed without recovery before forcing a hard reset.
    // ESP.restart() is a FULL software reset: re-runs setup(), toggles modem
    // power, reloads all tasks. RTC_DATA_ATTR variables (backlog counts, etc.)
    // ARE preserved across this reset since it is not a hard power cycle.
    if (diag_consecutive_sim_fails >= 13) {
      debugf1("[SIM] PERSISTENT ERROR for %d consecutive slots (~3h15m). "
              "Final resort: ESP32 SOFTWARE RESTART...\n",
              diag_consecutive_sim_fails);
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
  // v5.65 P0: Range guard for server configuration index
  if (http_no < 0 || http_no >= (int)(sizeof(httpSet) / sizeof(httpSet[0]))) {
      debugln("[HTTP] FATAL: http_no out of range (" + String(http_no) + "). Aborting send.");
      success_count = 0;
      return;
  }

  esp_task_wdt_reset();
  char stnId[16];
  const char *charArray;

  // v5.56: Global Sanity Check for record content
  if (content.length() < 10 && data_mode == eUnsentData) {
      debugln("[HTTP] Skip: Record too short/Empty — advancing pointer without counting as sent.");
      // P2 fix v5.65: Use sentinel value 2 (not 1) to signal 'skipped'.
      // success_count==1 increments diag_http_retry_count in the caller,
      // which would wrongly inflate the "backlog sent" dashboard counter.
      // 2 = skipped (corrupt/empty line): advances pointer, no counter hit.
      success_count = 2;
      return;
  }

  // v5.65: Guard against unconfigured UNIT string (http_no == -1).
  // Without this, httpSet[-1] or httpSet[11+] causes memory corruption.
  // http_no = -1 is set by setup() when UNIT doesn't match any known branch.
  const int httpSet_count = (int)(sizeof(httpSet) / sizeof(httpSet[0]));
  if (http_no < 0 || http_no >= httpSet_count) {
    debugln("[HTTP] FATAL: http_no out of range. UNIT/SYSTEM not configured!");
    debugln("[HTTP] Check UNIT and SYSTEM defines in globals.h. Aborting send.");
    success_count = 0;
    return;
  }

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
        return; 
      }         
      s = file1.size();
      s = (s > record_length) ? s - record_length : 0;
      file1.seek(s);
      content = file1.readString(); // Read the rest of the file
      file1.close();
    }
  }

  // v5.56: Ensure pointer follows current 'content' regardless of source
  charArray = content.c_str(); 

  debugln();
  debugf1("Current Data to be sent is : %s", charArray);
  debugln();

  // v5.65: Zero metadata variables before parsing to prevent stale data
  // from a previous successful send being used if this sscanf fails.
  temp_sampleNo = -1; temp_year = 0; temp_month = 0; temp_day = 0;
  temp_hr = 0; temp_min = 0; temp_sig = 0; temp_bat = 0.0;

#if SYSTEM == 0
  int res = sscanf(charArray, "%02d,%04d-%02d-%02d,%02d:%02d,%f,%f,%04d,%f",
         &temp_sampleNo, &temp_year, &temp_month, &temp_day, &temp_hr,
         &temp_min, &temp_instrf, &temp_crf, &temp_sig, &temp_bat);
  if (res < 10) { debugln("[HTTP] ERROR: sscanf parse failed (TRG). Skipping record."); success_count = 2; return; }
#endif
#if SYSTEM == 1
  // TWS: 12 components
  int res = sscanf(charArray, "%02d,%04d-%02d-%02d,%02d:%02d,%f,%f,%f,%03d,%04d,%f",
         &temp_sampleNo, &temp_year, &temp_month, &temp_day, &temp_hr,
         &temp_min, &temp_temp, &temp_hum, &temp_avg_ws, &temp_dir, &temp_sig,
         &temp_bat);
  if (res < 12) { debugln("[HTTP] ERROR: sscanf parse failed (TWS). Skipping record."); success_count = 2; return; }
#endif
#if SYSTEM == 2
  // TWS-RF: 13 components
  int res = sscanf(charArray, "%02d,%04d-%02d-%02d,%02d:%02d,%f,%f,%f,%f,%03d,%04d,%f",
         &temp_sampleNo, &temp_year, &temp_month, &temp_day, &temp_hr,
         &temp_min, &temp_crf, &temp_temp, &temp_hum, &temp_avg_ws, &temp_dir,
         &temp_sig, &temp_bat);
  if (res < 13) { debugln("[HTTP] ERROR: sscanf parse failed (TWS-RF). Skipping record."); success_count = 2; return; }
#endif
#if (SYSTEM == 0)
  snprintf(sample_bat, sizeof(sample_bat), "%04.1f", float(temp_bat));
#endif
#if (SYSTEM == 1) || (SYSTEM == 2)
  // v5.52 LOOP-3 FIX: Guard against empty station name before building FTP filename
  if (strlen(ftp_station) == 0) {
    debugln("[FTP] Skip: ftp_station (Station ID) not configured.");
    return;
  }
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

  // P0-B fix v5.65: Only apply live signal to CURRENT data.
  // For eUnsentData (backlog), temp_sig was already set by sscanf() from the
  // SPIFFS record and reflects the actual signal at the time of recording.
  // Overwriting with the current signal_lvl would corrupt historical signal
  // values in every backlog record sent to the server database.
  if (data_mode == eCurrentData) {
    temp_sig = signal_lvl; // v6.75: Use actual live signal level for current slot
  }

  // v5.50: BOGUS DATA GUARD — After DELETE DATA all SPIFFS files are wiped.
  // On the next boot, signature.txt is recreated as "0000-00-00,00:00"
  // which causes temp_year=0 after sscanf. Block this slot entirely and
  // show a clear LCD message so the user knows the unit is in fresh-start mode.
  if (temp_year == 0) {
    debugln("[HTTP] ⚠ Bogus date detected (year=0). Skipping HTTP — fresh "
            "start or DELETE DATA reboot. Waiting for first valid 15-min slot.");
    snprintf(ui_data[FLD_SEND_STATUS].bottomRow,
             sizeof(ui_data[FLD_SEND_STATUS].bottomRow), "YES ?           ");
    snprintf(ui_data[FLD_LAST_LOGGED].bottomRow,
             sizeof(ui_data[FLD_LAST_LOGGED].bottomRow), "NO DATA YET");
    sync_mode = eHttpStop;
    success_count = 0;
    return;
  }

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
#if SYSTEM == 1
    // TWS JSON Format
    snprintf(http_data, sizeof(http_data),
             "{\"StnNo\":\"%s\",\"DeviceTime\":\"%04d-%02d-%02d "
             "%02d:%02d:00\",\"Temp\":\"%s\",\"Humid\":\"%s\",\"WS\":\"%s\","
             "\"WD\":\"%s\",\"BatVolt\":\"%s\",\"SigStr\":\"%04d\",\"ApiKey\":\"%s\"}",
             cleanStn, temp_year, temp_month, temp_day, temp_hr, temp_min,
             sample_temp, sample_hum, sample_avgWS, sample_WD, sample_bat, temp_sig, httpSet[http_no].Key);
#endif
#if SYSTEM == 2
    // TWS-RF JSON Format
    snprintf(http_data, sizeof(http_data),
             "{\"StnNo\":\"%s\",\"DeviceTime\":\"%04d-%02d-%02d "
             "%02d:%02d:00\",\"RainCuml\":\"%05.1f\",\"Temp\":\"%s\",\"Humid\":\"%s\","
             "\"WS\":\"%s\",\"WD\":\"%s\",\"BatVolt\":\"%s\",\"SigStr\":\"%04d\",\"ApiKey\":\"%s\"}",
             cleanStn, temp_year, temp_month, temp_day, temp_hr, temp_min,
             temp_crf, sample_temp, sample_hum, sample_avgWS, sample_WD, sample_bat, temp_sig, httpSet[http_no].Key);
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
  if (strcmp(httpSet[http_no].Format, "json")) { // Only if NOT json
      snprintf(http_data, sizeof(http_data),
               "stn_no=%s&rec_time=%04d-%02d-%02d,%02d:%02d&temp=%s&humid=%s&w_"
               "speed=%s&w_dir=%s&signal=%04d&key=%s&bat_volt=%s&bat_volt2=%s",
               cleanStn, temp_year, temp_month, temp_day, temp_hr, temp_min,
               sample_temp, sample_hum, sample_avgWS, sample_WD, temp_sig,
               httpSet[http_no].Key, sample_bat, sample_bat);
  }
#endif

// TWS-RF (ADDON / SPATIKA)
#if SYSTEM == 2
  if (strcmp(httpSet[http_no].Format, "json")) { // Only if NOT json
      // v7.53: DMC Legacy requires stn_no and %05.1f. Spatika General requires
      // stn_id.
      if (strstr(UNIT, "SPATIKA")) {
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
  }
#endif

  debug("http_data format is ");
  debugln(http_data);
  debugln();
  // v5.63: Selective Mode Logic (Airtel/Jio Backlog Optimization)
  bool isAirtelOrJio = (strstr(carrier, "Airtel") != nullptr || strstr(carrier, "Jio") != nullptr);
  
  if (isAirtelOrJio && data_mode == eUnsentData) {
    // M2M SIM backlog: Skip Fast, jump straight to Robust.
    // Fast always times out on M2M SIMs after a session rebuild/zombie state.
    debugln("[HTTP] Airtel/Jio backlog: using Robust direct.");
    success_count = send_at_cmd_data(http_data, charArray, true);
  } else {
    // Current data (all carriers) or Backlog for BSNL: Fast -> Fast -> Robust
    success_count = send_at_cmd_data(http_data, charArray, false);
    if (success_count == 0) {
      debugln("[HTTP] 1st Attempt (Fast) failed. Retrying in 2s (Fast Attempt 2)...");
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      success_count = send_at_cmd_data(http_data, charArray, false);
      if (success_count == 0) {
        debugln("[HTTP] 2nd Attempt (Fast) also failed. Falling back to Robust method...");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        success_count = send_at_cmd_data(http_data, charArray, true);
      }
    }
  }
  // v6.76: One retry added above for current data.
  // Fails after retry will trigger FTP backlog storage.

  if (success_count == 1) {
    diag_consecutive_http_fails = 0; // v5.49 Build 5: Reset fail streaks
    diag_consecutive_reg_fails = 0;
    
    // v7.70+: PR counter resets on ANY successful HTTP/FTP (Current or Backlog)
    diag_http_present_fails = 0;
    snprintf(ui_data[FLD_HTTP_FAILS].bottomRow,
             sizeof(ui_data[FLD_HTTP_FAILS].bottomRow), "P:%d C:%d B:%d",
             diag_http_present_fails, diag_http_cum_fails, get_total_backlogs());

    if (data_mode == eCurrentData) {
      diag_first_http_count++;
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

    // v5.58: Hard-Kill 706/714 TCP Zombie Guard (Airtel Fix)
    // ─────────────────────────────────────────────────────────────────────
    bool tcp_zombie = (String(diag_http_fail_reason) == "706" ||
                       String(diag_http_fail_reason) == "713" ||
                       String(diag_http_fail_reason) == "714" ||
                       String(diag_http_fail_reason) == "TIMEOUT");

    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 3000);

    if (tcp_zombie) {
      if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
        debugln("[HTTP] TCP Zombie (706/714) detected. Executing Hard Bearer Nuke...");
        xSemaphoreGive(serialMutex);
      }
      
      // Mandatory Nuke Protocol
      SerialSIT.println("AT+CIPSHUT");
      waitForResponse("SHUT OK", 3000);
      
      SerialSIT.println("AT+CGACT=0,1");
      waitForResponse("OK", 2000);
      
      vTaskDelay(5000 / portTICK_PERIOD_MS); // Crucial 5-second carrier breather
    } else {
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    flushSerialSIT();

    // After a Hard Nuke (CIPSHUT), we MUST use the full recovery routine 
    // to ensure the APN is properly re-attached and a valid IP is acquired.
    if (!verify_bearer_or_recover()) {
      if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
        debugln("[RECOVERY] Bearer failed to recover. Skipping retry.");
        xSemaphoreGive(serialMutex);
      }
      goto fail_handling;
    } else {
      // ──────────────────────────────────────────────────────────────────
      // FIX FOR AIRTEL 706 TCP ZOMBIE ERROR
      // Do NOT execute AT+CIPSHUT here! verify_bearer_or_recover() just
      // established a pristine, clean IP mapping. CIPSHUT would destroy it.
      // ──────────────────────────────────────────────────────────────────
      
      // Rule 1: Silence URCs before HTTP setup (was missing in the retry block)
      SerialSIT.println("AT+CGEREP=0");
      waitForResponse("OK", 1000);
      
      flushSerialSIT(); // Clear stale UART bytes before HTTPINIT
      
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
      // v5.65 P1: Backlogs use Robust method (handshake) for higher success rate 
      // on weak/noisy networks, as this is already a retry cycle.
      success_count = send_at_cmd_data(http_data, charArray, true);
      
      // v5.63: Tower Cooldown. Airtel towers need ~3s to clear previous socket
      // before accepting a new rapid-fire request in a backlog loop.
      vTaskDelay(3000 / portTICK_PERIOD_MS);

      if (success_count == 1) {
        diag_consecutive_reg_fails = 0;
        diag_consecutive_http_fails = 0;

        // v7.70+: Reset present fails on successful backlog retry too
        diag_http_present_fails = 0;
        snprintf(ui_data[FLD_HTTP_FAILS].bottomRow,
                 sizeof(ui_data[FLD_HTTP_FAILS].bottomRow), "P:%d C:%d B:%d",
                 diag_http_present_fails, diag_http_cum_fails, get_total_backlogs());

        if (data_mode == eCurrentData) {
          // Success tracking centralized in send_at_cmd_data loop below
          diag_first_http_count++;   // v7.86
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
    } // End of verify_bearer_or_recover else block

fail_handling:
    if (success_count == 0) { // Complete failure
      diag_consecutive_http_fails++;
      diag_daily_http_fails++;
      if (data_mode == eCurrentData) {
        // v7.70: Monthly cum reset logic (Robust: any day of new month triggers it)
        if (current_month > 0 && diag_cum_fail_reset_month != current_month) {
          diag_http_cum_fails = 0;
          diag_cum_fail_reset_month = current_month;
          debugln("[HTTP] Monthly cum fail counter reset (New Month detected).");
        }
        diag_http_present_fails++;
        diag_http_cum_fails++;
        snprintf(ui_data[FLD_HTTP_FAILS].bottomRow,
                 sizeof(ui_data[FLD_HTTP_FAILS].bottomRow), "P:%d C:%d B:%d",
                 diag_http_present_fails, diag_http_cum_fails, get_total_backlogs());
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
        // v5.55 SELF-HEALING: If we are failing 4 times in a row, the bearer is likely a zombie.
        // Force a HARD MODEM RESET (GPIO 26 cycle) as per Rule 19.
        if (diag_consecutive_http_fails >= 4) {
          debugln("[RECOVERY] 4 Consecutive HTTP Fail Slots. Triggering Full "
                  "Modem Reset...");
          SerialSIT.println("AT+CFUN=1,1");
          vTaskDelay(15000 / portTICK_PERIOD_MS); // v5.65: Increased from 5s to 15s for full reboot
          SerialSIT.println("AT"); // Handshake
          waitForResponse("OK", 1000);
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
      } // closes if(data_mode == eCurrentData)
    }   // closes if(success_count == 0 - second try)
  }     // closes if(success_count == 0 - first try)
} // closes prepare_data_and_send()


void send_http_data() {
  if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(15000)) != pdTRUE) {
    debugln("[GPRS] Error: Modem Mutex Timeout - deferring HTTP send");
    diag_modem_mutex_fails++;
    return;
  }
  
  // Clear any stale TCP errors from previous runs to prevent false-positive network nuke loops
  diag_http_fail_reason[0] = '\0';
  
  String response;
  const char *charArray;
  /*
   * PREPARE HTTP PARAMS
   */

  const char *domain = httpSet[http_no].serverName;
  // char target_ip[64] = {0}; // Removed, target_ip is now handled by httpPostRequest directly

  bool is_ip_format = true;
  for (int i = 0; domain[i] != '\0'; i++) {
    if (isalpha(domain[i])) {
      is_ip_format = false;
      break;
    }
  }

  // v5.55: PREPARE URL (Zero-Gap Prep)
  // Logic: Use static IP from globals.h if DNS is problematic or if first attempt fails.
  snprintf(httpPostRequest, sizeof(httpPostRequest),
           "AT+HTTPPARA=\"URL\",\"http://%s:%s%s\"", domain,
           httpSet[http_no].Port, httpSet[http_no].Link);

  char fallbackUrl[150] = {0};
  snprintf(fallbackUrl, sizeof(fallbackUrl),
           "AT+HTTPPARA=\"URL\",\"http://%s:%s%s\"", httpSet[http_no].IP,
           httpSet[http_no].Port, httpSet[http_no].Link);

  debugln("[GPRS] Prepared URL: " + String(httpPostRequest));

  // Ensure PDP context is active before doing DNS lookups or HTTP
  SerialSIT.println("AT+CGACT?");
  response = waitForResponse("OK", 3000);
  if (response.indexOf("+CGACT: " + String(active_cid) + ",1") == -1) {
    debugln("[GPRS] PDP context inactive. Activating for DNS/HTTP...");
    SerialSIT.print("AT+CGACT=1,");
    SerialSIT.println(active_cid);
    waitForResponse("OK", 10000);
  }

  // v5.55: SMART DNS FALLBACK (Fast-Track)
  // v5.55 SELF-HEALING: Every 10 successful sends, or if we just recovered from a fail streak,
  // try DNS again to see if the network has healed.
  bool forceDnsRetry = (diag_http_success_count % 10 == 0) || (diag_consecutive_http_fails == 1);
  
  if (dns_fallback_active && strcmp(cached_server_domain, domain) == 0 && !forceDnsRetry) {
     debugln("[GPRS] Smart Fallback ACTIVE. Bypassing DNS trials for " + String(domain));
     strcpy(httpPostRequest, fallbackUrl);
  } else if (!is_ip_format) {
  // v5.63: Reverting to v3.0 logic. Use Domain Name directly in URL.
  // The modem's internal stack (AT+HTTPACTION) is more reliable at 
  // resolving DNS on private Airtel APNs than manual AT+CDNSGIP.
  snprintf(httpPostRequest, sizeof(httpPostRequest),
           "AT+HTTPPARA=\"URL\",\"http://%s:%s%s\"", domain,
           httpSet[http_no].Port, httpSet[http_no].Link);

  // v5.63: Manual DNS remains as a secondary "Diagnostic/Resolver" only if domain fails
  // But for the primary request, we use the Domain.
  if (!is_ip_format && (diag_consecutive_http_fails >= 2)) {
      debugln("[DNS] Multiple fails. Attempting manual resolution...");
      SerialSIT.print("AT+CDNSGIP=\"");
      SerialSIT.print(domain);
      SerialSIT.println("\"");
      waitForResponse("+CDNSGIP: 1", 5000);
  }
  }

  // httpPostRequest is already prepared at the top or in the fallback block
  // Removing the redundant snprintf that used target_ip

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
      SerialSIT.print("AT+CGACT=0,");
      SerialSIT.println(active_cid);
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
  // snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), ...); // Prepared in prepare_data_and_send()
  
  // v5.63: Removed AT+HTTPTERM and AT+CGEREP=0 here (interferes with Airtel stack)
  // v3.0 logic: HTTPINIT directly!
  vTaskDelay(50 / portTICK_PERIOD_MS); 
  flushSerialSIT();
  
  SerialSIT.println("AT+HTTPINIT");
  response = waitForResponse("OK", 5000);
  if (response.indexOf("OK") == -1) {
    debugln("[GPRS] HTTPINIT Failed. Trying TERM then INIT...");
    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 3000);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    flushSerialSIT();
    SerialSIT.println("AT+HTTPINIT");
    if (waitForResponse("OK", 5000).indexOf("OK") != -1) {
      http_ready = true;
    }
  } else {
    http_ready = true;
  }
  
  // Restore all parameters
  SerialSIT.println("AT+HTTPPARA=\"CID\",1");
  waitForResponse("OK", 1000);
  
  // v5.55: Re-send URL immediately before context variables to lock session (Rule 48 alignment)
  SerialSIT.println(httpPostRequest);
  waitForResponse("OK", 1000);

  SerialSIT.println("AT+HTTPPARA=\"ACCEPT\",\"*/*\"");
  response = waitForResponse("OK", 1000);

  if (!strcmp(httpSet[http_no].Format, "json")) {
    SerialSIT.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
    response = waitForResponse("OK", 1000);
    debugln("It is json");
  } else {
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
    if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(2000)) == pdTRUE) {
      File fileTemp2 = SPIFFS.open("/signature.tmp", FILE_WRITE);
      if (fileTemp2) {
        snprintf(signature, sizeof(signature), "%04d-%02d-%02d,%02d:%02d",
                 last_recorded_yy, last_recorded_mm, last_recorded_dd,
                 last_recorded_hr, last_recorded_min);
        fileTemp2.print(signature);
        fileTemp2.close();
        
        SPIFFS.remove("/signature.txt");
        SPIFFS.rename("/signature.tmp", "/signature.txt");
      }
      xSemaphoreGive(fsMutex);
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
        
        // 🚨 CRITICAL FIX: If skip_primary_http was true, send_http_data() was skipped 
        // and httpPostRequest is empty. Rebuild it locally so backlog has a target!
        const char *domain = httpSet[http_no].serverName;
        snprintf(httpPostRequest, sizeof(httpPostRequest),
                 "AT+HTTPPARA=\"URL\",\"http://%s:%s%s\"", domain,
                 httpSet[http_no].Port, httpSet[http_no].Link);
        
        unsent_pointer_count = 0; // resetting to 1st record of unsent data ..
        unsent_counter = 0;

        if (SPIFFS.exists("/unsent_pointer.txt")) {
          File read_unsent_count =
              SPIFFS.open("/unsent_pointer.txt", FILE_READ);
          if (!read_unsent_count) {
            debugln("Failed to open unsent_pointer.txt for reading");
            xSemaphoreGive(modemMutex); // v5.64 FIX: Release mutex before returning
            return; 
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
          debugln("Failed to open unsent_file for reading - skipping backlog");
          xSemaphoreGive(modemMutex);
          return;
        } 
        fileSize = file1.size();

        if (strstr(carrier, "Airtel") != nullptr || strstr(carrier, "Jio") != nullptr) {
          debugln("[Backlog] Airtel/Jio: proactive bearer refresh before backlog.");
          SerialSIT.println("AT+HTTPTERM");
          waitForResponse("OK", 2000);
          SerialSIT.println("AT+CIPSHUT");
          waitForResponse("SHUT OK", 3000);
          SerialSIT.println("AT+CGACT=0,1");
          waitForResponse("OK", 2000);
          vTaskDelay(3000 / portTICK_PERIOD_MS);  // carrier breather
          verify_bearer_or_recover();             // rebuilds APN + IP
          SerialSIT.println("AT+CGEREP=0");
          waitForResponse("OK", 1000);
          flushSerialSIT();
          SerialSIT.println("AT+HTTPINIT");
          waitForResponse("OK", 5000);
          http_ready = true;
          // Set all HTTPPARA fields — required before AT+HTTPACTION can fire
          SerialSIT.println("AT+HTTPPARA=\"CID\",1");
          waitForResponse("OK", 1000);
          SerialSIT.println(httpPostRequest);   // URL already built by send_http_data()/send_unsent_data()
          waitForResponse("OK", 1000);
          if (!strcmp(httpSet[http_no].Format, "json")) {
              SerialSIT.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
          } else {
              SerialSIT.println("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"");
          }
          waitForResponse("OK", 1000);
          SerialSIT.println("AT+HTTPPARA=\"ACCEPT\",\"*/*\"");
          waitForResponse("OK", 1000);
          debugln("[Backlog] Session rebuilt and configured. Starting backlog sends.");
        }

        int backlog_processed_count = 0;
        while (unsent_pointer_count < fileSize) {
          vTaskDelay(100 / portTICK_PERIOD_MS); // iter10

          static int uCount = 0;
          uCount++;
          debugln();
          debug("Line Number ");
          debugln(uCount);
          file1.seek(unsent_pointer_count);
          content = file1.readStringUntil('\n'); 
          unsent_pointer_count = file1.position(); // v5.56: Use actual file position (Variable length support)
          
          content.trim(); 
          if (content.length() < 10) {
            debugln("Skipping blank/invalid line in unsent backlog.");
            continue; 
          }
          
          charArray = content.c_str();

          // Set the data mode
          data_mode = eUnsentData;
          prepare_data_and_send();
          // v5.65 FINAL PRODUCTION RULES:
          // 1. Fail-Fast: If a record fails (success_count == 0), STOP immediately to save battery.
          // 2. Power-Cap: Limit to 15 records per 15-min wakeup to prevent overheating/drain.
          // 3. Tower-Breather: Mandatory 3s delay between lines to let Airtel clear the session.
          
          if (success_count == 0) {
              debugln("[Power] Backlog line FAILED. Stopping to preserve battery.");
              // v5.65 P4 Fix: Save pointer even on failure to avoid resending successful lines
              File unsent_count_f = SPIFFS.open("/unsent_pointer.txt", FILE_WRITE);
              if (unsent_count_f) {
                  unsent_count_f.print(unsent_pointer_count);
                  unsent_count_f.close();
              }
              break;
          }
          
          // v5.65 P4: UI RESPONSIVENESS - Allow manual keypad interrupt during backlog
          if (pending_manual_status || pending_manual_gps || pending_manual_health) {
              debugln("[GPRS] 💡 Interrupt: Manual command detected. Deferring backlog...");
              File unsent_count_f = SPIFFS.open("/unsent_pointer.txt", FILE_WRITE);
              if (unsent_count_f) {
                  unsent_count_f.print(unsent_pointer_count);
                  unsent_count_f.close();
              }
              break;
          }
          
          if (++backlog_processed_count >= 15) {
              debugln("[Power] Backlog limit (15) reached. Saving remainder for next wakeup.");
              // Update pointer for next time
              File unsent_count = SPIFFS.open("/unsent_pointer.txt", FILE_WRITE);
              if (unsent_count) {
                  unsent_count.print(unsent_pointer_count);
                  unsent_count.close();
              }
              break;
          }

          // NOTE: The old unsent_counter==6 safety valve has been removed (BUG-C3 fix v5.65).
          // unsent_counter was never incremented in this loop, making that check dead code.
          // Active guards: (1) Fail-Fast break on success_count==0 above,
          //                (2) 15-record power cap on backlog_processed_count above.

          // Tower breather: only needed after a real HTTP attempt, not for skipped records
          if (success_count != 2) { // 2 = skip sentinel (P2 fix v5.65)
            vTaskDelay(3000 / portTICK_PERIOD_MS); // Tower breather (3s)
          }

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

// v5.51: send_unsent_data() is called ONCE via the Independent FTP Trigger
// below (line 1580+). Do NOT call it here again inside the HTTP-success block
// to avoid double-upload and wasted power.

  } else { // Handle failure to send current data
    debugln("*** Current data couldn't be sent. Backlog will be handled "
            "independently.");

    // v5.49 Build 5: NDM tracking is now handled EXCLUSIVELY by the scheduler
    // on successful record write to SPIFFS. This prevents double-counting
    // and ensures NDM represents "Processed Night Data" volume accurately.
    // diag_daily_http_fails is already incremented inside send_at_cmd_data().
  }

  // v5.64: Pruned buffers & Selective SMS
  SerialSIT.println("AT+HTTPTERM");
  waitForResponse("OK", 3000);
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  // v5.50: Restore URCs BEFORE releasing mutex (Modem sequence finishing)
  SerialSIT.println("AT+CGEREP=2");
  waitForResponse("OK", 1000);

  xSemaphoreGive(modemMutex); // v5.55: Release modem early to allow sub-calls to take it
  
#if (SYSTEM == 1 || SYSTEM == 2)
  // v5.49 Build 5: INDEPENDENT FTP TRIGGER
  // Decoupled from HTTP Success. FTP serves as the robust rescue layer.
  if (gprs_mode == eGprsSignalOk && (signal_lvl > -96)) {
    // v5.68 FIX: UART Race Condition Prevented. If the user queued a manual
    // LCD command during the HTTP HTTP-Fill, skip the massive FTP job
    // so the Modem Mutex is immediately available for the UI.
    if (!pending_manual_status && !pending_manual_gps && !pending_manual_health) {
       send_unsent_data(); 
    } else {
       debugln("[GPRS] Yielding FTP transfer to instantly service manual LCD/UI request.");
    }
  }
#endif

  // v5.65 Selective SMS Check: Guaranteed once an hour (at Minute 0 slot)
  if (record_min == 0) {
    sync_mode = eSMSStart;
    send_sms(); // This function will internally take modemMutex
  } else {
    debugln("[GPRS] Skipping SMS check (hourly task).");
    sync_mode = eHttpStop;
  }
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

void send_unsent_data() { // ONLY FOR TWS AND TWS-ADDON
  if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
    debugln("[SPIFFS] Mutex Timeout: Skipping send_unsent_data");
    return;
  }
  const char
                          // *charArray;
      *ptr;
  debugln("Entering FTP mode and checking if data period is correct for "
          "sending and if there is any file to be sent");
  int ftp_year = rf_cls_yy % 100;
  char fileName[50];

  char stnId[16];
  if (strlen(ftp_station) == 4 && isDigitStr(ftp_station)) {
    snprintf(stnId, sizeof(stnId), "00%s", ftp_station);
  } else {
    strcpy(stnId, ftp_station);
  }

#if SYSTEM == 1
  if (strstr(UNIT, "SPATIKA"))
    snprintf(fileName, sizeof(fileName), "/TWS_%s_%02d%02d%02d_%02d%02d00.swd",
             stnId, ftp_year, rf_cls_mm, rf_cls_dd, record_hr, record_min);
  else
    snprintf(fileName, sizeof(fileName), "/TWS_%s_%02d%02d%02d_%02d%02d00.kwd",
             stnId, ftp_year, rf_cls_mm, rf_cls_dd, record_hr, record_min);
#endif

#if SYSTEM == 2
  if (strstr(UNIT, "SPATIKA"))
    snprintf(fileName, sizeof(fileName),
             "/TWSRF_%s_%02d%02d%02d_%02d%02d00.swd", stnId, ftp_year,
             rf_cls_mm, rf_cls_dd, record_hr, record_min);
  else
    snprintf(fileName, sizeof(fileName),
             "/TWSRF_%s_%02d%02d%02d_%02d%02d00.kwd", stnId, ftp_year,
             rf_cls_mm, rf_cls_dd, record_hr, record_min);
#endif

  // v5.56: BACKLOG RECOVERY ON EVERY CYCLE
  // Removed 'primary_data_delivered' gate and hourly restriction per user
  // request. Backlog will be attempted whenever signal is usable (-95 dBm or
  // better).
  // v5.51: Power-Saving FTP Gating logic
  snprintf(ftpunsent_file, sizeof(ftpunsent_file), "/ftpunsent.txt");

  // v5.52 RELIABILITY: Recover orphaned remainders from failed previous renames
  if (!SPIFFS.exists(ftpunsent_file) && SPIFFS.exists("/ftpremain.txt")) {
    debugln("[Healer] Orphaned ftpremain.txt found. Recovering to ftpunsent.txt");
    SPIFFS.rename("/ftpremain.txt", "/ftpunsent.txt");
  }

  // v5.52 FIX: Ensure signal_lvl isn't 0 if polling failed (prevents false triggers/0000 in logs)
  if (signal_lvl == 0) signal_lvl = SIGNAL_STRENGTH_MISSING_DATA;
  int unsent_cnt = countStored(ftpunsent_file);

  // v7.63 FIX: Use current_hour/current_min (actual RTC wakeup time) instead
  // of record_hr/record_min (data record timestamp) for scheduled-slot checks.
  // The data timestamp (record_hr) is the FLOOR of the current time to the
  // nearest 15-min boundary. At a 03:01 wakeup, record_hr=3 & record_min=0
  // but at a 00:01 wakeup the record is stored under the PREVIOUS day's
  // period (rf_close logic), causing record_hr to be 23 even though the
  // actual wall-clock hour is 0. Using current_hour/current_min guarantees
  // the check fires at the correct 3-hour boundary regardless of rounding.
  //
  // Scheduled Sync Slots: every 3 hours when minute is < 15 (wakeup window)
  //   00:00-00:14, 03:00-03:14, 06:00-06:14, 09:00-09:14 ... 21:00-21:14
  // Final Day Cleanup: 08:45-08:59 to clear remaining backlog before 09:00
  // Scheduled slots fire when current_min <= 15 and hour is divisible by 3.
  // Using <= 15 ensures a 03:15 wakeup (or similar slight overshoot) is still
  // caught as part of the 03:00 3-hour boundary window.
  bool scheduled_slot =
      (current_min <= 15) &&
      (current_hour % 3 == 0);
  bool morning_cleanup = (current_hour == 8 && current_min >= 45 && current_min < 60);
  debugf5("[FTP-Gate] unsent=%d cur_time=%02d:%02d sched=%s cleanup=%s\n",
          unsent_cnt, current_hour, current_min,
          scheduled_slot ? "YES" : "NO", morning_cleanup ? "YES" : "NO");
  bool should_push = (unsent_cnt >= 2) ||  // Threshold: fire immediately if >=2 records
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

              // v5.65 Fix: Guard against file-creation failure (SPIFFS Full or busy)
              // If the chunk file fails to open, we MUST ABORT now. Proceeding
              // with an empty chunk file and then deleting the source queue
              // leads to permanent data loss. 
              if (!chunk) {
                debugln("[FTP] CRITICAL: Failed to create chunk file. Aborting FTP.");
                src.close(); if (remainder) remainder.close();
                xSemaphoreGive(fsMutex);
                return;
              }

              int linesRead = 0;
              while (src.available()) {
                esp_task_wdt_reset(); 
                String line = src.readStringUntil('\n');
                line.trim();
                if (line.length() > 15) {
                  line += "\r\n";
                  if (linesRead < FTP_CHUNK_SIZE) {
                    chunk.print(line);
                  } else {
                    if (remainder) remainder.print(line);
                  }
                  linesRead++;
                }
              }
              if (chunk) chunk.close();
              if (remainder) remainder.close();
              src.close();

              // v5.52 FIX: Constant parity
              if (linesRead > FTP_CHUNK_SIZE) {
                debugln("[FTP] Huge Backlog! Records chunked for TX. "
                        "Original queue protected until success.");
              } else {
                SPIFFS.remove("/ftpremain.txt");
                debugln("[FTP] Backlog fully chunked into Active Payload.");
              }
          } else {
            debugln("Failed to open ftpunsent.txt for chunking.");
            xSemaphoreGive(fsMutex);
            return; // Hygiene Fix: Abort on source failure to prevent wasted blank FTP sequence
          }
          debug("Retrieved file is ");
          debugln(fileName);
          esp_task_wdt_reset();
          // v5.66: Release FS mutex before long FTP upload to allow scheduler to record
          xSemaphoreGive(fsMutex); 
          send_ftp_file(fileName, false, false); // Pass alreadyLocked=false
          // Note: send_ftp_file will take its own lock for cleanup
        } else {
          debugln("[FTP] Skip: Registration lost. Retrying next hour.");
          xSemaphoreGive(fsMutex); // Ensure release on skip
        }
      } // end else (signal OK)
    } else {
      debugln("No ftpunsent.txt found. Skipping FTP.");
      xSemaphoreGive(fsMutex); // Ensure release on missing file
    }

    if (sampleNo == 3) { // v5.65 P2: Fixed cleanup condition — only run at 09:30 AM (sampleNo 3) 
                          // to avoid accidental backlog clears if sampleNo 7 is reached via reboot.
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
      // v5.52 BUG-1 FIX: Removed duplicate snprintf (was copy-paste error)
      snprintf(temp_file, sizeof(temp_file), "/dailyftp_%04d%02d%02d.txt",
               previous_rfclose_year, previous_rfclose_month,
               previous_rfclose_day);

      // Maintain original standard filename
      int ftp_year = rf_cls_yy % 100;
      char stnId[16];
      if (strlen(ftp_station) == 4 && isDigitStr(ftp_station)) {
        snprintf(stnId, sizeof(stnId), "00%s", ftp_station);
      } else {
        strcpy(stnId, ftp_station);
      }

#if SYSTEM == 1
      if (strstr(UNIT, "SPATIKA"))
        snprintf(fileName, sizeof(fileName), "/TWS_%s_%02d%02d%02d_%02d%02d00.swd",
                 stnId, ftp_year, rf_cls_mm, rf_cls_dd, record_hr, record_min);
      else
        snprintf(fileName, sizeof(fileName), "/TWS_%s_%02d%02d%02d_%02d%02d00.kwd",
                 stnId, ftp_year, rf_cls_mm, rf_cls_dd, record_hr, record_min);
#endif
#if SYSTEM == 2
      if (strstr(UNIT, "SPATIKA"))
        snprintf(fileName, sizeof(fileName),
                 "/TWSRF_%s_%02d%02d%02d_%02d%02d00.swd", stnId, ftp_year,
                 rf_cls_mm, rf_cls_dd, record_hr, record_min);
      else
        snprintf(fileName, sizeof(fileName),
                 "/TWSRF_%s_%02d%02d%02d_%02d%02d00.kwd", stnId, ftp_year,
                 rf_cls_mm, rf_cls_dd, record_hr, record_min);
#endif

      if (SPIFFS.exists(temp_file)) {
        copyFile(temp_file, fileName); // copyFile(source,destination);
        debug("Retrieved file is ");
        debugln(fileName);
        esp_task_wdt_reset();
        send_ftp_file(fileName, true, true); // v5.66: alreadyLocked=true
      } else {
        debugln("Daily FTP: Temp file not found. Skipping.");
      }
    }
  } else {
    // v5.68 BUGFIX: If should_push is False, or signal is bad, we MUST release the mutex! 
    // Otherwise, send_health_report() hangs forever waiting for this locked Mutex!
    xSemaphoreGive(fsMutex);
  }

  // v5.66: Removed block-held fsMutex from here; moved to early-release logic inside the if(should_push) block
} // end send_unsent_data

void send_ftp_file(char *fileName, bool isDailyFTP, bool alreadyLocked) {
  if (!alreadyLocked) {
    if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
      debugln("[SPIFFS] Mutex Timeout: Skipping send_ftp_file");
      return;
    }
  }
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

  if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(15000)) != pdTRUE) {
    debugln("[FTP] Error: Modem Mutex Timeout - deferring upload");
    diag_modem_mutex_fails++;
    if (!alreadyLocked)
      xSemaphoreGive(fsMutex); // RELEASE FS MUTEX
    return;
  }

  flushSerialSIT(); // Ensure UART buffer is clean before starting FTP
                    // sequence
  debugln("Initializing A7672S for FTP...");
  vTaskDelay(1000 / portTICK_PERIOD_MS); // Allow module to settle

  if (SPIFFS.exists(fileName)) {
    send_daily = 2;
    
    // v5.55: Smart FTP Mode Selection
    // 1. If we have a previously successful mode, use it immediately.
    // 2. Otherwise, use carrier-based heuristic.
    int initial_ftp_mode = 0; 
    if (preferred_ftp_mode != -1) {
       initial_ftp_mode = preferred_ftp_mode;
       debugln("[FTP] Using SAVED Smart Mode: " + String(initial_ftp_mode == 1 ? "Passive" : "Active"));
    } else if (strstr(carrier, "AIRTEL") || strstr(carrier, "Airtel") || strstr(carrier, "Jio")) {
      initial_ftp_mode = 1; 
      debugln("[FTP] Airtel/Jio detected. Smart Default: Passive (1).");
    } else {
      debugln("[FTP] BSNL/Other detected. Smart Default: Active (0).");
    }
    
    ftp_login_flag = setup_ftp(initial_ftp_mode);

    if (ftp_login_flag == 1) {

      SerialSIT.println("ATE0");
      response = waitForResponse("OK", 3000);
      SerialSIT.println("AT+FSCD=C:/");
      response = waitForResponse("OK", 10000);
      debug("Response of AT+FSCD ");
      debugln(response);

      snprintf(
          gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSOPEN=\"C:/%s\",0\r\n",
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
        xSemaphoreGive(modemMutex);
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
        xSemaphoreGive(modemMutex);
        return; // v7.67: Guard against null file dereference
      }
      debug("File size of the file is ");
      debugln(file1.size());

      fileSize = file1.size();
      file1.seek(s);
      // Phase 5: Removed content = file1.readString(); to prevent massive Heap Exhaustion
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
        
        // Phase 5: Chunk stream direct from SPIFFS to UART, avoiding RAM crashes
        char file_buf[256];
        while (file1.available()) {
          int bytesRead = file1.read((uint8_t*)file_buf, 255);
          file_buf[bytesRead] = '\0';
          SerialSIT.print(file_buf);
        }
        // Send trailing newline just in case FSWRITE is expecting one
        SerialSIT.println();
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
                 "AT+CFTPSPUTFILE=\"%s\",1", modulePath); // v5.38 style: drive is implied by mode 1

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
            response1.reserve(2048); // v5.65 P4: Prevent heap fragmentation
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
            // v5.55: SAVE working mode for future iterations (Rule 48 Smart Persistence)
            preferred_ftp_mode = (ftp_put_retries == 0) ? initial_ftp_mode : ((initial_ftp_mode == 1) ? 0 : 1);
            debugln("[FTP] Smart Mode Saved: " + String(preferred_ftp_mode == 1 ? "Passive" : "Active"));
            
            if (isDailyFTP) strcpy(last_cmd_res, "Success: Daily FTP OK");
            else strcpy(last_cmd_res, "Success: Backlog FTP OK");
            break;
          } else {
            debugln("FTP PUT failed. Checking recovery...");
            if (ftp_put_retries < MAX_FTP_PUT_RETRIES) {
              // v7.68: Mode-Switch Recovery.
              // Attempt 1 used Active Mode (BSNL 2G). Error 9 = data channel timeout.
              // Attempt 2 switches to Passive Mode (Airtel 4G/firewalled networks).
              // This covers all carrier combinations without hardcoding.
              int next_mode = (ftp_put_retries == 0) ? 1 : 0; // Flip mode on first failure
              debugln("[FTP] PUT failed. Switching FTP mode and re-logging in (Attempt " +
                      String(ftp_put_retries + 2) + ")...");
              debug("[FTP] Switching to ");
              debugln(next_mode == 1 ? "Passive Mode (Airtel)" : "Active Mode (BSNL)");
              SerialSIT.println("AT+CFTPSLOGOUT");
              waitForResponse("+CFTPSLOGOUT: 0", 5000);
              vTaskDelay(2000 / portTICK_PERIOD_MS);

              // Re-login with the opposite mode
              send_daily = 2;
              if (setup_ftp(next_mode) == 1) {
                SerialSIT.println("AT+FSCD=C:/");
                waitForResponse("OK", 5000);
              } else {
                debugln("[FTP] Login Recovery failed for both modes.");
                break; // Both modes failed — give up
              }
            }
            ftp_put_retries++;
          }
        }

        if (upload_success) {
          // last_cmd_res already set in the loop
          diag_consecutive_reg_fails =
              0; // RESET counter on any successful data upload

          // v7.70+: PR counter resets on successful FTP backlog too
          diag_http_present_fails = 0;
          snprintf(ui_data[FLD_HTTP_FAILS].bottomRow,
                   sizeof(ui_data[FLD_HTTP_FAILS].bottomRow), "P:%d C:%d B:%d",
                   diag_http_present_fails, diag_http_cum_fails, get_total_backlogs());

          markFileAsDelivered(fileName, true); // v5.48 Track recovered records

          if (isDailyFTP) { // If Daily FTP is successful,
                            // remove the dailyftp file.
            // v5.52 BUG-6 FIX: Use fileName (local/captured) not global temp_file
            // which may have been overwritten by another code path mid-flight.
            SPIFFS.remove(fileName);
            debug("Removed Daily FTP file: ");
            debugln(fileName);
          } else {
            // COMMIT Queue Remainder on Success
            SPIFFS.remove("/ftpunsent.txt");
            if (SPIFFS.exists("/ftpremain.txt")) {
              if (SPIFFS.rename("/ftpremain.txt", "/ftpunsent.txt")) {
                debugln("[FTP] Chunk successful! ftpunsent.txt overwritten with "
                        "remainder queue.");
              } else {
                debugln("[FTP] CRITICAL ERROR: Failed to rename ftpremain.txt "
                        "to ftpunsent.txt. Backlog records may be orphaned.");
              }
            } else {
              debugln("Cleared ftpunsent.txt permanently after full upload.");
            }
          }

          // Remove the *.kwd files. These are the ftp files
          const char *pattern;
          if (strstr(UNIT, "SPATIKA"))
            pattern = ".swd";
          else
            pattern = ".kwd";

          const String prefix = "/";
          // v5.65 P4 Fix: Buffer filenames to prevent SPIFFS iterator corruption during deletion
          std::vector<String> filesToDelete;
          File rootDir = SPIFFS.open("/");
          if (rootDir) {
            File f = rootDir.openNextFile();
            while (f) {
              String name = f.name();
              if (name.endsWith(pattern)) {
                filesToDelete.push_back(prefix + name);
              }
              f.close();
              f = rootDir.openNextFile();
            }
            rootDir.close();
            
          // v5.66: Retake fsMutex for cleanup (Targeted operation)
          if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(10000)) == pdTRUE) {
            for (const String& fPath : filesToDelete) {
              debugf1("Removing file: %s\n", fPath.c_str());
              vTaskDelay(50 / portTICK_PERIOD_MS);
              SPIFFS.remove(fPath);
            }
            xSemaphoreGive(fsMutex);
          }
        }

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
          if (strstr(UNIT, "SPATIKA"))
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
  xSemaphoreGive(modemMutex); // v5.55: Release modem after FTP session
  if (!alreadyLocked)
    xSemaphoreGive(fsMutex);
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
  if (!gprs_started && gprs_mode == eGprsInitial) {
    debugln("[GPRS] Modem never started. Cutting power directly.");
    digitalWrite(26, LOW);
    return;
  }

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
      waitForResponse("NORMAL POWER DOWN", 8000); // Extended timeout for graceful detach
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
  } else {
    debugln("[GPRS] ⚠️ Session was unstable (reg fails). Hard Hardware "
            "Reset.");
  }

  debugln("[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).");
  digitalWrite(26, LOW);
  
  // v5.65 P4 Fix: I2C Spike Recovery
  // Modem power-down can cause spikes on the shared I2C bus (SDA/SCL).
  // Perform an immediate silent bus recovery to ensure RTC/Sensors remain accessible.
  recoverI2CBus();
  
  gprs_started = false;
  gprs_mode = eGprsSleepMode; // Prevent Ghost Restart during sleep entry
}

void send_sms() {
  if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(10000)) != pdTRUE) {
    debugln("[GPRS] Error: Modem Mutex Timeout - skipping SMS check");
    sync_mode = eSMSStop; // v5.65 P2 Fix: Reset sync_mode to allow GPRS task to finalize
    return;
  }
  
  String response;
  vTaskDelay(500 /
             portTICK_PERIOD_MS); // TRG8-3.0.5g reduced from 1min to 500ms
  int msg_no;

  debugln();
  debugln("[GPRS] Checking SMS...");
  debugln();

  for (msg_no = 1; msg_no < 10; msg_no++) { // v5.65 P2: Increased from 5 to 10 to scan more slots
    process_sms(msg_no);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
  // v5.65 fix: Changed flag from 4 to 3.
  // AT+CMGD=1,4 = delete message at index 1 AND ALL messages of ANY status (nuke all).
  // A command SMS arriving at the tower DURING the processing loop above (~400ms)
  // would be silently dropped — a race condition that loses operator commands.
  // AT+CMGD=1,3 = delete only READ+SENT messages. UNREAD messages are preserved
  // and will be processed on the next SMS check cycle.
  SerialSIT.println("AT+CMGD=1,3");
  response = waitForResponse("OK", 5000);
  debugln("Removed READ/SENT messages (UNREAD preserved)");

  sync_mode = eSMSStop;
  xSemaphoreGive(modemMutex);
}

// v5.63: Dynamic Handshake Support. Try Fast v3.0 first, then Robust as fallback.
int send_at_cmd_data(char *payload, String response_arg, bool robust) {
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
  // v5.65: Renamed from ht_data to cmd_buf to prevent shadowing the global ht_data[80].
  // The shadow caused the global to remain permanently empty/stale for debug reads.
  // Also removed the dead snprintf below (it was always overwritten before use).
  char cmd_buf[80];

  debugf1("Payload is %s", payload);
  debugln();

  if (robust) {
    // Robust mode for weak-signal or strict towers (BSNL, etc.)
    snprintf(cmd_buf, sizeof(cmd_buf), "AT+HTTPDATA=%d,5000", i);
    debugln("[HTTP] Using Robust Handshake (Wait for DOWNLOAD)...");
    SerialSIT.println(cmd_buf);
    if (waitForResponse("DOWNLOAD", 10000).indexOf("DOWNLOAD") == -1) {
      debugln("[HTTP] AT+HTTPDATA failed (Missing DOWNLOAD).");
      flushSerialSIT();
      return 0;
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
    SerialSIT.println(payload); // v3.0: use println to finalize buffer
    waitForResponse("OK", 5000);
  } else {
    // v5.63 Native v3.0 Fast push! 
    // We send command, wait for prompt byte, then push payload.
    // This mimics v3.0's behavior while keeping the UART clean.
    // v5.67 (Claude's logic fix): Opened up latency window. If DOWNLOAD drops late,
    // we need enough of the 3000ms window remaining to clock the payload JSON.
    snprintf(cmd_buf, sizeof(cmd_buf), "AT+HTTPDATA=%d,3000", i);
    debugln("[HTTP] Using Fast v3.0 Handshake...");
    SerialSIT.println(cmd_buf);
    waitForResponse("DOWNLOAD", 1500); 
    
    SerialSIT.println(payload); 
    waitForResponse("OK", 1500); 
  }

  // Fire Action
  SerialSIT.println("AT+HTTPACTION=1");
  response = waitForResponse("+HTTPACTION:", 25000);
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
    // ISSUE-M3 fix v5.65: Do NOT increment diag_daily_http_fails here.
    // prepare_data_and_send() (the caller) increments it at line ~1155.
    // Incrementing here too caused every failure to be counted twice,
    // showing 2x the real HTTP fail count on the health report dashboard.
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
          diag_http_success_count++; 
          if (data_mode == eUnsentData) diag_http_retry_count++; // v5.65 P4: Track live recovery
        } else {
          diag_sent_mask_prev[temp_sampleNo / 32] |= bit;
          diag_http_success_count_prev++; // v5.65 P4: FIXED Yesterday counter drift
          if (data_mode == eUnsentData) diag_http_retry_count_prev++;
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

    // v7.70: Ensure clean session close after success to prevent crosstalk
    // v5.66: CRITICAL BACKLOG FIX - Do NOT destroy the HTTP session mid-backlog!
    // Keeping it open allows records 2-15 to flow instantly without rebuild penalties.
    if (data_mode != eUnsentData) {
      SerialSIT.println("AT+HTTPTERM");
      waitForResponse("OK", 1000);
    }

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
  if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
    debugln("[SPIFFS] Mutex Timeout: Skipping store_current_unsent_data");
    return;
  }
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

  // v5.52 ENH-6: Warning if SPIFFS is >90% full
  if (SPIFFS.usedBytes() > SPIFFS.totalBytes() * 0.90) {
    debugln("[SPIFFS] WARNING: >90% full. unsent.txt write may fail.");
  }

  File file2 = SPIFFS.open(unsent_file, FILE_APPEND);
  if (file2) {
    String cleanData = String(finalStringBuffer);
    cleanData.trim();
    if (cleanData.length() > 0) {
      file2.println(cleanData); // v5.56: Trimmed write prevents double-newline gaps
    }
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

  // v5.52 RELIABILITY: Instead of deleting the whole file (dangerous), 
  // we trim the oldest records to make space if file > 150KB.
  if (SPIFFS.exists(ftpunsent_file)) {
    File f_check = SPIFFS.open(ftpunsent_file, FILE_READ);
    if (f_check.size() > 150000) {
      size_t target_size = f_check.size();
      f_check.close();
      debugln("[SPIFFS] ftpunsent.txt > 150KB. Trimming oldest records...");
      
      // Smart Trim: Keep the most recent 100KB
      File src = SPIFFS.open(ftpunsent_file, FILE_READ);
      File dst = SPIFFS.open("/trim.tmp", FILE_WRITE);
      if (src && dst) {
        src.seek(target_size - 100000); // Jump to last 100KB
        src.readStringUntil('\n'); // Align to next full record boundary
        while (src.available()) {
          dst.write(src.read());
        }
        src.close();
        dst.close();
        SPIFFS.remove(ftpunsent_file);
        SPIFFS.rename("/trim.tmp", ftpunsent_file);
        debugln("[SPIFFS] Trim successful. Kept most recent 100KB.");
      } else {
        if (src) src.close();
        if (dst) dst.close();
        debugln("[SPIFFS] ERROR: Trim failed. Clearing file as final fallback.");
        SPIFFS.remove(ftpunsent_file);
      }
    } else {
      f_check.close();
    }
  }

  // v5.52 ENH-6: Warning if SPIFFS is >90% full
  if (SPIFFS.usedBytes() > SPIFFS.totalBytes() * 0.90) {
    debugln("[SPIFFS] WARNING: >90% full. ftpunsent.txt write may fail.");
  }

  File ftpfile2 = SPIFFS.open(ftpunsent_file, FILE_APPEND);
  if (ftpfile2) {
    String cleanFtp = "";

    // BUG-C4 fix v5.65: Re-derive FTP string from lastrecorded CSV to prevent stale global consumption
    // EXCEPTION: For SYSTEM == 2, the CSV string possesses a 6-character rain width, which ruins 
    // the 5-character FTP layout requirement. ftpappend_text is inherently safe and correctly padded.
#if SYSTEM != 2
    if (strlen(finalStringBuffer) > 20) {
        String csv = String(finalStringBuffer);
        csv.trim();
        int firstComma = csv.indexOf(',');
        if (firstComma != -1) {
            String derived_stn = String(ftp_station);
            bool isAllDigits = true;
            for (int i = 0; i < derived_stn.length(); i++) {
                if (!isdigit(derived_stn[i])) { isAllDigits = false; break; }
            }
            if (derived_stn.length() == 4 && isAllDigits) {
                derived_stn = "00" + derived_stn;
            }
            cleanFtp = derived_stn + ";" + csv.substring(firstComma + 1);
            // The CSV has a comma between date and time which MUST remain in FTP too.
            // Replace only the commas from the second comma onwards.
            int firstCommaAfterSemi = cleanFtp.indexOf(',', cleanFtp.indexOf(';') + 1);
            if (firstCommaAfterSemi != -1) {
                int secondCommaAfterSemi = cleanFtp.indexOf(',', firstCommaAfterSemi + 1);
                if (secondCommaAfterSemi != -1) {
                    for (int i = secondCommaAfterSemi; i < (int)cleanFtp.length(); i++) {
                        if (cleanFtp[i] == ',') cleanFtp[i] = ';';
                    }
                }
            }
            debugln("[FTP-Store] Re-derived record from lastrecorded CSV.");
        }
    }
#endif

    if (cleanFtp.length() < 50) {
        cleanFtp = String(ftpappend_text);
        cleanFtp.trim();
        if (cleanFtp.length() < 50) {
            debugln("[FTP-Store] WARNING: Both lastrecorded and ftpappend_text are invalid. Skipping.");
            ftpfile2.close();
            return;
        }
    }

    if (cleanFtp.length() >= 50) {
      if (ftpfile2.println(cleanFtp)) { 
          diag_backlog_total++; // v5.65 P3: Increment total backlog counter for health report
      }
    }
    ftpfile2.close();
  } else {
    debugln("Failed to open ftpunsent.txt for appending (store_current)");
  }
  debug("************************");
  debug("Storing data in FTP unsent file due to SIM issue/REG issue/Signal "
        "issue ");
  debugln("************************");
  debug("ftpunsent is ");
  debugln(ftpappend_text);
#endif

  // v5.50: Update HTTP fail counters even when bypassing send_http_data()
  // This covers SIM errors, signal failures, and any reason HTTP was not attempted.
  // PR (present) increments every slot missed; CUM increments permanently (monthly reset).
  // PR is reset to 0 only upon a successful HTTP POST (in send_http_data()).
  if (current_month >0 && diag_cum_fail_reset_month != current_month) {
    diag_http_cum_fails = 0;
    diag_cum_fail_reset_month = current_month;
    debugln("[STORE] Monthly cum fail counter reset (New Month detected).");
  }
  diag_http_present_fails++;
  diag_http_cum_fails++;
  snprintf(ui_data[FLD_HTTP_FAILS].bottomRow,
           sizeof(ui_data[FLD_HTTP_FAILS].bottomRow), "P:%d C:%d B:%d",
           diag_http_present_fails, diag_http_cum_fails, get_total_backlogs());
  debugf1("[STORE] HTTP miss counted. Present: %d", diag_http_present_fails);
  debugf1(" | CumMth: %d\n", diag_http_cum_fails);

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
    char file_buf[256];
    while (file4.available()) {
      int bytesRead = file4.read((uint8_t*)file_buf, 255);
      file_buf[bytesRead] = '\0';
      debug(file_buf);
    }
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
    char file_buf[256];
    while (ftpfile4.available()) {
      int bytesRead = ftpfile4.read((uint8_t*)file_buf, 255);
      file_buf[bytesRead] = '\0';
      debug(file_buf);
    }
    ftpfile4.close();
    debugln();
  }
#endif

#endif

  sync_mode = eHttpStop;
  vTaskDelay(300 / portTICK_PERIOD_MS);
  xSemaphoreGive(fsMutex); // v5.66: Fix missing mutex release
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

  signal_lvl = -111; // v5.52 FIX: Default to -111 not 0
  retries = 0;

  // [v5.61] Pre-settle delay: modem returns 85 (not ready) on first 1-3 polls
  // immediately after boot. A single 400ms wait eliminates redundant retries.
  vTaskDelay(400 / portTICK_PERIOD_MS);

  int invalid_signal_count = 0;
  // rssi 0 = -113dBm. Continuous -113 is essentially no signal.
  while ((signal_lvl == -111) &&
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
    if (signal_strength != 85 && signal_strength != -113) {
      signal_lvl = signal_strength;
      // v5.50 Optimization: Exit instantly if we get a strong reading.
      break;
    } else {
      invalid_signal_count++;
      // v5.65 P1: If we see "No Signal" (-113 or 85) for 30s (60 polls), move to
      // registration. This avoids locking on a temporary -113 during warm-up.
      if (invalid_signal_count > 60) {
        debugln("[GPRS] Signal search timeout (30s). Moving to Registration.");
        signal_lvl = signal_strength;
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
  
  // v5.55 SELF-HEALING: Reset fallbacks on SIM change
  dns_fallback_active = false;
  preferred_ftp_mode = -1;
  cached_server_ip[0] = '\0';
  cached_server_domain[0] = '\0';

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
    // Final tier: ICCID prefix-based detection (on-device fallback)
    // Uses same prefix table as server-side get_carrier_from_iccid()
    if (current_iccid.length() >= 6) {
      String prefix6 = current_iccid.substring(0, 6);
      if (prefix6 == "899116" || prefix6 == "899110") {
        strcpy(carrier, "Airtel");
        strcpy(apn_str, "airteliot.com");
        debugln("[APN] ICCID Fallback: Airtel");
      } else if (prefix6 == "899100") {
        strcpy(carrier, "BSNL");
        strcpy(apn_str, "bsnlnet");
        debugln("[APN] ICCID Fallback: BSNL");
      } else if (prefix6 == "899184") {
        strcpy(carrier, "Jio");
        strcpy(apn_str, "jionet");
        debugln("[APN] ICCID Fallback: Jio");
      } else if (prefix6 == "899111") {
        strcpy(carrier, "Vi");
        strcpy(apn_str, "www");
        debugln("[APN] ICCID Fallback: Vi");
      } else {
        strcpy(carrier, "SIM OK");
        strcpy(apn_str, "airteliot.com");
        debugln("[APN] ICCID Fallback: Unknown prefix, defaulting Airtel");
      }
    } else {
      strcpy(carrier, "SIM OK");
      strcpy(apn_str, "airteliot.com");
    }
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
    if (isBSNL) {
      debugln("[GPRS] FORCE_2G_ONLY flag active & BSNL SIM. Setting CNMP=13.");
      SerialSIT.println("AT+CNMP=13"); // GSM Only
    } else {
      debugln("[GPRS] FORCE_2G_ONLY flag bypassed: Detected 4G-M2M SIM.");
      SerialSIT.println("AT+CNMP=2");  // Automatic Mode (LTE/GSM)
    }
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
              // v5.56: Immediate Recovery for Denied status
              debugln("[GPRS] Tower DENIED (3). Attempting CGATT Reset...");
              SerialSIT.println("AT+CGATT=0");
              waitForResponse("OK", 3000);
              SerialSIT.println("AT+CGATT=1");
              waitForResponse("OK", 3000);
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

    // v5.56: Aggressive Modem Reset before full system reboot
    if (diag_consecutive_reg_fails == 4 || diag_consecutive_reg_fails == 8) {
      debugln("[GPRS] Persistent failure. Triggering MODEM HARD RESET (GPIO 26)...");
      digitalWrite(26, LOW);
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      digitalWrite(26, HIGH);
      vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
    if (diag_consecutive_reg_fails >= 10) {
      debugln("[GPRS] PERSISTENT REG FAIL. Resetting system...");
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      ESP.restart();
    }
  }
}

void get_a7672s() {
  String response;
  char ccid[25];
  get_ccid().toCharArray(ccid, 25);
  char stored_apn[50];

  debugln("--- GPRS SETTING PDP ---");
  debugln();
  debugln("************************");
  debugln("Setting PDP context ");
  debugln("************************");
  debugln();

  active_cid = 0;
  // v5.63: Native v3.0 style activation.
  // No status queries, fire and move on.
  if (load_apn_config(ccid, stored_apn, sizeof(stored_apn))) {
      strncpy(apn_str, stored_apn, sizeof(apn_str) - 1);
      if (try_activate_apn(apn_str)) {
          vTaskDelay(3000 / portTICK_PERIOD_MS); // v5.63: Carrier Breather after success
          gprs_pdp_ready = true;
          return;
      }
  }

  // If stored APN failed or doesn't exist, start Smart APN cycle.
  // v5.65 FIX: Carrier-aware search — use carrier detected by get_network() to
  // try the correct APN first. Prevents BSNL units from connecting with airteliot.com.
  debugln("APN: Starting Carrier-Aware APN Search...");

  // Build priority list: correct carrier APN first, then fallbacks
  const char *primary_apn = apn_str; // apn_str was set by get_network() based on CSPN/COPS/ICCID
  const char *fallback_apns[] = {
      "airteliot.com",
      "airtelgprs.com",
      "bsnlnet",
      "jionet",
      "bsnlm2m"
  };

  // Try primary APN from carrier detection first
  if (strlen(primary_apn) > 0) {
      debugln("APN: Trying carrier-matched APN first -> " + String(primary_apn));
      if (try_activate_apn(primary_apn)) {
          save_apn_config(primary_apn, ccid);
          vTaskDelay(3000 / portTICK_PERIOD_MS);
          gprs_pdp_ready = true;
          return;
      }
  }

  // Fallback: try remaining APNs (skipping the one we already tried)
  for (int i = 0; i < 5; i++) {
      if (strcmp(fallback_apns[i], primary_apn) == 0) continue; // already tried
      if (try_activate_apn(fallback_apns[i])) {
          strcpy(apn_str, fallback_apns[i]);
          save_apn_config(fallback_apns[i], ccid);
          vTaskDelay(3000 / portTICK_PERIOD_MS);
          gprs_pdp_ready = true;
          return;
      }
  }

  // Last Resort Soft Reset
  debugln("APN: Soft Resetting Stack (CGATT)...");
  SerialSIT.println("AT+CGATT=0");
  waitForResponse("OK", 5000);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  SerialSIT.println("AT+CGATT=1");
  waitForResponse("OK", 5000);
  vTaskDelay(2000 / portTICK_PERIOD_MS);

  if (try_activate_apn(apn_str)) {
    save_apn_config(apn_str, ccid);
    vTaskDelay(3000 / portTICK_PERIOD_MS); 
    gprs_pdp_ready = true;
    return;
  }

  debugln("APN: FAILED. Going to store only mode.");
  gprs_mode = eGprsSignalForStoringOnly;
  gprs_pdp_ready = false;
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
  if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(10000)) != pdTRUE) {
    debugln("[GPS] Error: Modem Mutex Timeout - skipping GPS request");
    return;
  }
  
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
          xSemaphoreGive(modemMutex);
          return;    // SUCCESS
        }
      }
    }
    debugln("[GPS] Attempt failed. Retrying...");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
  debugln("[GPS] All attempts to get fresh coordinates failed.");
  xSemaphoreGive(modemMutex);
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
  // We use standard %lf for double and print with 6 decimal precision
  sscanf(csqstr, "+CLBS: %d,%lf,%lf,", &tmp, &lati,
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

int setup_ftp(int transMode) { // 0=Active(BSNL 2G), 1=Passive(Airtel 4G)
  flushSerialSIT(); // Clear leftover data from previous task
  char gprs_xmit_buf[300];
  String response, rssiStr;
  const char *resp;
  const char *ftpServer;
  const char *ftpUser;
  const char *ftpPassword;
  int portName, rssiIndex, result;

  if (strstr(UNIT, "BIHAR")) {
    // Bihar
    if (send_daily == 1) {
      ftpServer = "ftphbih.spatika.net";
      ftpUser = "trg_desbih_csvdt";
      ftpPassword = FTP_PASS_BIH_D;
      portName = 21;
      send_daily = 0;
    } else {
      ftpServer = "89.32.144.163"; // dot.spatika.net
      ftpUser = "dota_bih";
      ftpPassword = FTP_PASS_BIH_GEN;
      portName = 21;
    }
  } else if (strstr(UNIT, "KSNDMC")) {
    // DMC
    if (send_daily == 1) {
      ftpServer = "ftp1.ksndmc.net";
      ftpUser = "trg_spatika_v2@ksndmc.net";
      ftpPassword = FTP_PASS_KS_TRG;
      portName = 21;
      send_daily = 0;
    } else if (send_daily == 2) {
#if SYSTEM == 1
      ftpServer = "ftp1.ksndmc.net";
      ftpUser = "tws_spatika_v2";
      ftpPassword = FTP_PASS_KS_TWS;
      portName = 21;
#endif
#if SYSTEM == 2
      ftpServer = "ftp1.ksndmc.net";
      ftpUser = "twsrf_spatika_v2";
      ftpPassword = FTP_PASS_KS_ADD;
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
  } else if (strstr(UNIT, "SPATIKA")) {
#if SYSTEM == 0
    ftpServer = "ftp.spatika.net";
    ftpUser = "trg_gen";
    ftpPassword = "spgen123";
    portName = 21;
#endif
#if SYSTEM == 1
    ftpServer = "ftp.spatika.net";
    ftpUser = "tws_gen";
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

  // v7.68: Mode-switchable FTP transfer mode for carrier compatibility.
  // BSNL 2G → Active Mode (transmode=0): device opens data port, server connects back to it.
  // Airtel 4G → Passive Mode (transmode=1): server opens data port, client connects to it.
  // The caller selects the mode; the PUT retry loop switches mode on Error 9.
  if (transMode == 1) {
    debugln("[FTP] Using Passive Mode (transmode=1) — Airtel/Firewall path");
    SerialSIT.println("AT+CFTPSCFG=\"transmode\",1");
  } else {
    debugln("[FTP] Using Active Mode (transmode=0) — BSNL 2G path");
    SerialSIT.println("AT+CFTPSCFG=\"transmode\",0");
  }
  waitForResponse("OK", 2000);

  // Enable numeric error codes for better diagnostics
  SerialSIT.println("AT+CMEE=1");
  waitForResponse("OK", 2000);

  // DNS Warm-up: Force resolution of FTP server before login (Fixes Error 13)
  debugln("[FTP] Warming up DNS...");
  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CDNSGIP=\"%s\"",
           ftpServer);
  SerialSIT.println(gprs_xmit_buf);
  // v5.55 Optimized: Wait up to 20s for DNS resolution (BSNL/2G Latency)
  response = waitForResponse("+CDNSGIP:", 20000);
  debugln(response);

  // v5.45: DNS retry / Fallback
  if (response.indexOf("+CDNSGIP: 1") == -1) {
    debugln("[FTP] DNS failed. Checking Insurance IP fallback...");
    const char* fallbackIP = NULL;
    // Updated with User-Verified IPs (v5.55)
    if (strstr(ftpServer, "ksndmc.net")) fallbackIP = "117.216.42.181"; // rtdas.ksndmc.net
    else if (strstr(ftpServer, "spatika.net")) {
      if (strstr(ftpServer, "ftphbih") || strstr(ftpServer, "rtdasbih")) fallbackIP = "185.250.105.225";
      else if (strstr(ftpServer, "rtdasbmsk")) fallbackIP = "164.100.130.199";
      else fallbackIP = "144.91.104.105"; // rtdas.spatika.net
    }

    if (fallbackIP != NULL) {
      // BUG-C5 fix v5.65: Use the verified IP directly in the login command
      // to bypass DNS entirely. This saves ~40s on congested BSNL 2G towers.
      debugln("[FTP] Using Insurance IP fallback: " + String(fallbackIP));
      snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
               "AT+CFTPSLOGIN=\"%s\",%d,\"%s\",\"%s\",0",
               fallbackIP, portName, ftpUser, ftpPassword);
      // gprs_xmit_buf is now re-prepared with IP for the login below.
      // Skip the redundant DNS retry.
    } else {
      // No known IP for this host — retry DNS once as last resort
      debugln("[FTP] No fallback IP known. Retrying DNS once...");
      vTaskDelay(3000 / portTICK_PERIOD_MS);
      SerialSIT.println(gprs_xmit_buf);
      response = waitForResponse("+CDNSGIP:", 20000);
      debugln(response);
    }
  }

  vTaskDelay(2000 / portTICK_PERIOD_MS); // Pre-login settling delay

  // BUG-C5 fix v5.65: Only re-prepare with domain if DNS succeeded.
  // If the fallback IP path set gprs_xmit_buf above, preserve it — overwriting
  // with ftpServer (domain) would negate the entire IP fallback.
  if (response.indexOf("+CDNSGIP: 1") != -1) {
    // DNS OK — use domain as originally planned
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
             "AT+CFTPSLOGIN=\"%s\",%d,\"%s\",\"%s\",0", ftpServer, portName,
             ftpUser, ftpPassword);
  }
  // else: gprs_xmit_buf already has either the IP-based command (fallback path)
  // or the original domain command (no-fallback retry path). Use as-is.

  SerialSIT.println(gprs_xmit_buf);
  // v5.50 Optimized: Wait 45s for login (If it hasn't succeeded by then on
  // BSNL, it likely won't)
  response = waitForResponse("+CFTPSLOGIN:", 60000); // v7.68: Restored 60s timeout (was 45s in v5.50)
  debugln(response);

  // v7.68: No login-level fallback needed — mode is controlled by caller.
  // If login fails at the selected mode, return 0 and let the PUT retry switch modes.

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
  if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(30000)) != pdTRUE) {
    debugln("[OTA] Error: Modem Mutex Timeout - deferring download");
    diag_modem_mutex_fails++;
    return;
  }
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
    xSemaphoreGive(modemMutex);
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
           OTA_SERVER_PORT, fileName);
  SerialSIT.println(gprs_xmit_buf);
  if (waitForResponse("OK", 2000).indexOf("OK") == -1) {
    debugln("[OTA] URL setup failed. Aborting.");
    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 2000);
    xSemaphoreGive(modemMutex);
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
  String hdr_body = waitForResponse("\r\n\r\n", 10000);
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
    xSemaphoreGive(modemMutex); // v5.66: Fix missing mutex release
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
    xSemaphoreGive(modemMutex); // v5.66: Fix missing mutex release
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
    ota_silent_mode = false;  // Fixed: restore silent mode correctly
    xSemaphoreGive(modemMutex); // v5.66: Fix missing mutex release
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
    // One-Chunk-One-Session (Rule 41) forced re-init
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
             HEALTH_SERVER_IP, OTA_SERVER_PORT, fileName);
    SerialSIT.println(gprs_xmit_buf);
    waitForResponse("OK", 2000);

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
      // Force re-init on next loop
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
      // Something is deeply wrong, reset session
      chunk_retries++;
      consecutive_fails++;
      esp_task_wdt_reset();
      continue;
    }

    // Write to Flash
    int bytes_to_write = min(got, total_no_of_bytes - actual_downloaded);
    // TIER 2 LIVE RACES: WDT timeout guard for 2G
    esp_task_wdt_reset();
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
    // Force re-init for next chunk (Rule 41)
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
  xSemaphoreGive(modemMutex);
}

void copyFromSPIFFSToFS(char *dateFile) {
  if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(15000)) != pdTRUE) {
    debugln("[FS] Error: Modem Mutex Timeout - deferring copy");
    diag_modem_mutex_fails++;
    return;
  }
  if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
    debugln("[FS] Error: SPIFFS Mutex Timeout - deferring copy");
    xSemaphoreGive(modemMutex); // RELEASE MODEM MUTEX
    return;
  }
  String response;
  char SPIFFSFile[50], fileName[50];
  char stnId[16];
  if (strlen(station_name) == 4 && isDigitStr(station_name)) {
    snprintf(stnId, sizeof(stnId), "00%s", station_name);
  } else {
    strcpy(stnId, station_name);
  }
  snprintf(SPIFFSFile, sizeof(SPIFFSFile), "/%s_%s.txt", station_name, dateFile);
  snprintf(fileName, sizeof(fileName), "%s_%s.txt", stnId, dateFile);
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
    if (response.indexOf("OK") == -1) {
      debugln("Error changing directory to C:/");
      xSemaphoreGive(modemMutex);
      xSemaphoreGive(fsMutex);
      return; // Exit if response is invalid
    }
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
      xSemaphoreGive(modemMutex);
      xSemaphoreGive(fsMutex);
      return; // Exit if response is invalid
    }
    sscanf(csqstr, "+FSOPEN: %d,", &handle_no);

    File file1 = SPIFFS.open(SPIFFSFile, FILE_READ);
    if (!file1) {
      debugln("C5 FIX: Failed to open SPIFFS source file. Aborting "
              "copyFromSPIFFSToFS.");
      xSemaphoreGive(modemMutex);
      xSemaphoreGive(fsMutex);
      return;
    }
    debug("File size of the SPIFFS file is ");
    debugln(file1.size());
    fileSize = file1.size();
    file1.seek(s);
    // Phase 5 Fix: Removed content = file1.readString() Memory crash
    
    // filehandle,length,timeout
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSWRITE=%d,%d,10\r\n",
             handle_no,
             fileSize); // 0 : if the file does not exist, it will be created
    vTaskDelay(200 / portTICK_PERIOD_MS);
    SerialSIT.println(gprs_xmit_buf);
    response = waitForResponse("CONNECT", 5000);
    
    // Chunk flow directly to modem UART to bypass 100KB RAM fragmentation
    char file_buf[256];
    while (file1.available()) {
      int bytesRead = file1.read((uint8_t*)file_buf, 255);
      file_buf[bytesRead] = '\0';
      SerialSIT.print(file_buf);
    }
    SerialSIT.println();
    response = waitForResponse("+FSWRITE", 5000);
    debug("Response of FSWRITE is ");
    debugln(response);
    file1.close();

    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSCLOSE=%d",
             handle_no); // FILEHANDLE,offset,(0:start of file 1:cur pos of
                         // pointer 2: end of file
    SerialSIT.println(gprs_xmit_buf);
    response = waitForResponse("OK", 5000); // Fixed: FSCLOSE responds with OK, not URC
    vTaskDelay(200 / portTICK_PERIOD_MS);

    // v5.65 Fix: Pass default mode 1 (Passive) for compatibility
    ftp_login_flag = setup_ftp(1);

    if (ftp_login_flag == 1) {
      char *modulePath = (fileName[0] == '/') ? &fileName[1] : fileName;
      snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CFTPSPUTFILE=\"%s\",1",
               modulePath); 
      SerialSIT.println(gprs_xmit_buf); // FTP client context
      // v5.65 WDT-SAFE: Active poll instead of blind 150s wait (prevents WDT reset on slow BSNL)
      {
        response = "";
        unsigned long put_start = millis();
        while ((millis() - put_start) < 150000) {
          esp_task_wdt_reset();
          vTaskDelay(500 / portTICK_PERIOD_MS);
          while (SerialSIT.available()) {
            char cc = SerialSIT.read();
            if (response.length() < 2048) response += cc;
          }
          if (response.indexOf("+CFTPSPUTFILE:") != -1) break;
        }
      }
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
  xSemaphoreGive(fsMutex); // v5.66 BUGFIX: Was missing on the success path!
  xSemaphoreGive(modemMutex);
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
  // v5.68 Stability Fix: Buffer UART in static char array instead of 
  // dynamic String (response += c) to prevent massive heap fragmentation
  // on long multi-kilobyte JSON HTTP reads.
  static char buf[2048]; // Phase 5 Fix: Moved from Stack to Heap/BSS to stop Stack Overflows
  int buf_idx = 0;
  buf[0] = '\0'; // Initialize empty
  
  unsigned long startTime = millis();

  while ((millis() - startTime) < timeout) {
    vTaskDelay(1 / portTICK_PERIOD_MS);
    esp_task_wdt_reset(); // Keep watchdog happy during long AT command waits

    while (SerialSIT.available()) {
      char c = SerialSIT.read();
      if (buf_idx < 2047) { // Prevent unbounded growth
        buf[buf_idx++] = c;
        buf[buf_idx] = '\0';
      }
    }

    // Use fast C-string search to avoid allocating intermediate Strings
    if (strstr(buf, expectedResponse.c_str()) != NULL) {
      return String(buf); // Only allocate the String exactly ONCE on success
    }
  }

  return String(buf); // Return whatever we caught on timeout
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
  // TIER 2 LIVE RACES: DST Time-Shift Corruption guard
  gmt_time->tm_isdst = 0; // India does not observe DST; force 0 to prevent uninitialized memory offset
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
#if ENABLE_HEALTH_REPORT == 1
bool send_health_report(bool useJitter) {
  if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(15000)) != pdTRUE) {
     debugln("[Health] Error: Modem Mutex Timeout - deferring report");
     diag_modem_mutex_fails++;
     return false;
  }
  if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
    debugln("[SPIFFS] Mutex Timeout: Skipping send_health_report");
    xSemaphoreGive(modemMutex); // RELEASE MODEM MUTEX
    return false;
  }

  // Skip health report entirely if on Airtel M2M and health server is foreign IP
  // Airtel M2M firewall blocks non-whitelisted foreign IPs — repeated attempts waste ~3 minutes
  if (strstr(carrier, "Airtel") && strstr(apn_str, "airteliot")) {
      debugln("[Health] Skipping: Airtel M2M SIM + foreign health server. Request IP whitelist from Airtel.");
      xSemaphoreGive(modemMutex);
      xSemaphoreGive(fsMutex);
      return false;
  }

  // v5.45: Carrier Congestion Breather after FTP (Special for BSNL)
  if (strstr(carrier, "BSNL") || strstr(carrier, "bsnl"))
      vTaskDelay(5000 / portTICK_PERIOD_MS);
  else
      vTaskDelay(1000 / portTICK_PERIOD_MS);

  // v5.45: Purity Guard - Silence all network noise during transmission
  SerialSIT.println("AT+CGEREP=0");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CREG=0");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CEREG=0");
  waitForResponse("OK", 1000);

  if (diag_pd_count == 0 && current_year > 2024) {
    debugln("[Health] Counters zero, reconstructing...");
    reconstructSentMasks(true);
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
  
  // v5.55: Add Sensor Diagnostics to Health Status
  if (diag_temp_cv) H_FAULT("TEMP_STUCK");
  if (diag_hum_cv) H_FAULT("HUM_STUCK");
  if (diag_ws_cv) H_FAULT("WS_STUCK");
  if (diag_temp_erv || diag_temp_erz) H_FAULT("TEMP_UNREAL");
  if (diag_hum_erv || diag_hum_erz) H_FAULT("HUM_UNREAL");
  if (diag_ws_erv) H_FAULT("WS_UNREAL");
  if (diag_wd_fail) H_FAULT("WD_FAIL");
  if (diag_rain_jump) H_FAULT("RAIN_SPIKE");
  if (diag_rain_reset) H_FAULT("RAIN_RESET");
  if (diag_rain_calc_invalid) H_FAULT("RAIN_CALC");
  
  if (strcmp(diag_crash_task, "NONE") != 0) {
    char crash_info[32];
    snprintf(crash_info, sizeof(crash_info), "CRASH-%s", diag_crash_task);
    H_FAULT(crash_info);
  }

  if (h_status[0] == '\0')
    strcpy(h_status, "OK");

  char sensor_info[48];
#if SYSTEM == 0
  snprintf(sensor_info, sizeof(sensor_info), "RF-OK"); 
#elif SYSTEM == 1
  snprintf(sensor_info, sizeof(sensor_info), "TH-%s,WS-%s,WD-%s",
           (hdcType == HDC_UNKNOWN ? "FAIL" : "OK"), (ws_ok ? "OK" : "FAIL"),
           (wd_ok ? "OK" : "FAIL"));
#elif SYSTEM == 2
  snprintf(sensor_info, sizeof(sensor_info), "RF-OK,TH-%s,WS-%s,WD-%s",
           (hdcType == HDC_UNKNOWN ? "FAIL" : "OK"), (ws_ok ? "OK" : "FAIL"),
           (wd_ok ? "OK" : "FAIL"));
#endif

  char gps_str[32];
  // v5.65 Fix: Robust epsilon check for double and higher precision reporting
  if (abs(lati) < 0.00001 && abs(longi) < 0.00001)
    snprintf(gps_str, sizeof(gps_str), "NA");
  else
    snprintf(gps_str, sizeof(gps_str), "%.8f,%.8f", lati, longi);

  int spiffs_used = SPIFFS.usedBytes() / 1024;
  int spiffs_total = SPIFFS.totalBytes() / 1024;

  // v5.57 Fix F2: Use countStored() which validates line length >= 10 chars.
  // The old newline-counting approach had an off-by-one from trailing \n,
  // causing the server to trigger CLEAR_FTP_QUEUE incorrectly (e.g. 49 real
  // records + trailing \n reported as 50, one blank line reported as 1).
  int unsent_count = countStored("/unsent.txt") + countStored("/ftpunsent.txt");

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
  char jsonBody[1536];
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
      "\"ver\":\"%s\",\"iccid\":\"%s\",\"carrier\":\"%s\",\"gps\":\"%s\","
      "\"cdm_sts\":\"%s\","
      "\"calib\":\"%s\"," // v7.86
      "\"ndm_cnt\":%d,\"pd_cnt\":%d,"
      "\"http_present_fails\":%d,\"http_cum_fails\":%d,"
      "\"http_backlog_cnt\":%d,\"mutex_fail\":%d,"
      "\"ota_fails\":%d,\"ota_fail_reason\":\"%s\""
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
      UNIT_VER, cached_iccid, carrier, gps_str,
      diag_cdm_status, // cdm_sts → server evaluates CDM from this
      calib_report,    // calibration info
      diag_ndm_count, diag_pd_count,
      diag_http_present_fails, diag_http_cum_fails,
      get_total_backlogs(),
      diag_modem_mutex_fails,
      ota_fail_count, ota_fail_reason,
      feedback); // v7.92

  // TIER 3 LIVE RACES: JSON Truncation Guard
  if (msgLen >= (int)sizeof(jsonBody)) {
      Serial.printf("[Health] WARNING: JSON truncated (%d > %d). Clamping.\n",
                    msgLen, (int)sizeof(jsonBody));
      msgLen = sizeof(jsonBody) - 1; // Use actual written bytes
      jsonBody[msgLen] = '\0';
      // Attempt to close the JSON properly if truncated mid-field:
      // Find last complete field boundary and add closing brace
      char *last_comma = strrchr(jsonBody, ',');
      if (last_comma && (jsonBody + msgLen - last_comma) < 5) {
          *last_comma = '}'; // Replace trailing comma with closing brace
          *(last_comma + 1) = '\0';
          msgLen = last_comma - jsonBody + 1;
      }
  }

  xSemaphoreGive(fsMutex); // v5.66: RELEASE FS MUTEX EARLY - Payload is built!
                           // This prevents the scheduler from being blocked for
                           // 2 mins during network delays.

  if (useJitter)
    vTaskDelay((esp_random() % 5000) / portTICK_PERIOD_MS); // v5.57 Fix F3: HW RNG — seed-independent across deep sleep reboots
  else
    vTaskDelay(2000 / portTICK_PERIOD_MS);

  bool success = false;
  int max_attempts = useJitter ? 3 : 2; // Allow 2 attempts for manual triggers to endure Bearer Nuke TCP recoveries
  for (int attempt = 1; attempt <= max_attempts; attempt++) {
    debugf2("[Health] Attempt %d/%d\n", attempt, max_attempts);
    if (!verify_bearer_or_recover())
      continue;

    // v7.79: Total Silence Protocol (Rule 10/27)
    SerialSIT.println("AT+CGEREP=0");
    waitForResponse("OK", 1000);

    // Rule 12/24: Extended Breather after host switch
    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 5000);
    
    // v5.66: BSNL 2G TCP teardown channel release requires >5s after a primary burst
    if (strstr(carrier, "BSNL") || strstr(carrier, "bsnl")) {
      vTaskDelay(8000 / portTICK_PERIOD_MS);
    } else {
      vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    
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
             "AT+HTTPPARA=\"URL\",\"http://%s:%s%s\"", HEALTH_SERVER_IP,
             HEALTH_SERVER_PORT, HEALTH_SERVER_PATH);
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

      // Pre-flush to remove any stray URCs (+CGEV etc) that block the parser
      flushSerialSIT();

      char ht_data_cmd[64];
      // v7.75: Increased prompt wait to 10s for high-latency 2G (increased to 15s in v5.56)
      snprintf(ht_data_cmd, sizeof(ht_data_cmd), "AT+HTTPDATA=%d,15000", msgLen);
      SerialSIT.println(ht_data_cmd);

      String act = "";
      // Give massive allowance (25 seconds) for 2G network to allocate HTTP socket buffer space
      if (waitForResponse("DOWNLOAD", 25000).indexOf("DOWNLOAD") != -1) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        
        // v5.65 P4 Fix: Ultra-Robust Chunked Data Write (BSNL Optimized)
        // 48 bytes + 20ms delay is the "Golden Ratio" for SIMCom 2G buffers.
        int sentBytes = 0;
        while (sentBytes < msgLen) {
            int toWrite = min(48, msgLen - sentBytes);
            SerialSIT.write(jsonBody + sentBytes, toWrite);
            sentBytes += toWrite;
            esp_task_wdt_reset(); // ultra-safe WDT reset for slow UART/modem bottlenecks
            vTaskDelay(20 / portTICK_PERIOD_MS); 
        }
        
        // Increased timeout to 20s for BSNL 2G data-day congestion
        if (waitForResponse("OK", 20000).indexOf("OK") != -1) {

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
            continue; // Skip HTTPREAD on dead session, go to next attempt
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
          if (body.indexOf("\"DELETE_DATA\"") != -1)
            force_delete_data = true;

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

  xSemaphoreGive(modemMutex); // v5.55: Release modem
  if (success) {
    strcpy(diag_crash_task, "NONE"); // v5.59: Clear after delivery
  }
  return success;
}
#endif

/*
 *   GRACEFUL REBOOT (v5.58 Fix)
 */
void power_cut_modem_shutdown() {
  debugln("[HEAL] Graceful 2 PM Maintenance Reboot Activated.");
  sync_mode = 4; // Prevent stray UART tasks (eHttpStop equivalent)
  digitalWrite(26, LOW); // Cut modem VCC instantly
  vTaskDelay(3000 / portTICK_PERIOD_MS); // allow 3-second delay to settle heavy current draw
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
