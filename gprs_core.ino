#include "globals.h"

int active_cid = 1;      // Default to 1
bool http_ready = false; // v5.42: Track if HTTPINIT succeeded for this cycle

// Helper to clear UART buffer to prevent stale data contamination between tasks
void gprs(void *pvParameters) {
  esp_task_wdt_add(NULL);
  static int target_fld = FLD_SEND_STATUS; // v5.50: Moved to top for sequential queue scope
  last_activity_time = millis(); // Refresh safety heartbeat

  for (;;) {
    esp_task_wdt_reset();
    last_activity_time = millis(); // Refresh safety heartbeat every loop iteration

    // v5.70: Atomic snapshot for thread-safe decision making
    portENTER_CRITICAL(&syncMux);
    int mode_snap = sync_mode;
    portEXIT_CRITICAL(&syncMux);

    // Debug: Check sync_mode at loop start (Only on change)
    static int last_debug_sync_mode = -1;
    if (mode_snap != eSyncModeInitial && mode_snap != last_debug_sync_mode) {
      if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
        debugf2("[GPRS] State Change: sync_mode=%d gprs_mode=%d\n", mode_snap,
                gprs_mode);
        xSemaphoreGive(serialMutex);
      }
      last_debug_sync_mode = mode_snap;
    } else if (mode_snap == eSyncModeInitial) {
      last_debug_sync_mode = eSyncModeInitial;
    }

    // --- MANUAL TRIGGERS (Keypad) ---
    // v5.82: Pending Request Promotion (Ensures 'PLEASE WAIT' requests are picked up when idle)
    if (mode_snap == eSyncModeInitial || mode_snap == eSMSStop || 
        mode_snap == eHttpStop || mode_snap == eExceptionHandled) {
      if (pending_manual_status || pending_manual_gps || pending_manual_health) {
        portENTER_CRITICAL(&syncMux);
        if (pending_manual_status) sync_mode = eSMSStart;
        else if (pending_manual_gps) sync_mode = eGPSStart;
        else if (pending_manual_health) sync_mode = eHealthStart;
        mode_snap = sync_mode; // Update local snap to trigger the handler below
        portEXIT_CRITICAL(&syncMux);
      }
    }

    if (mode_snap == eSMSStart || mode_snap == eGPSStart ||
        mode_snap == eStartupGPS || mode_snap == eHealthStart) {
      target_fld = (mode_snap == eSMSStart) ? FLD_SEND_STATUS :
                   (mode_snap == eGPSStart) ? FLD_SEND_GPS :
                   (mode_snap == eHealthStart) ? FLD_SEND_HEALTH : FLD_SEND_GPS;

      // Clear the queued pending flag immediately to prevent double-execution
      if (mode_snap == eSMSStart) pending_manual_status = false;
      else if (mode_snap == eGPSStart) pending_manual_gps = false;
      else if (mode_snap == eHealthStart) pending_manual_health = false;

      // [HR-C01/M01] Final Hardening: Take modemMutex ONCE for the entire manual trigger sequence.
      // This protects Power-On, Signal Fetch, and Transmission as a single atomic unit.
      if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(15000)) == pdTRUE) {
        
        if (gprs_mode == eGprsInitial) {
          debugln("[GPRS] Manual Trigger: Initiating Power On...");
          strcpy(ui_data[target_fld].bottomRow, "GPRS POWER ON...");
          start_gprs();
          
          if (gprs_mode != eGprsSignalOk) {
            debugln("[GPRS] Initialization failed. Aborting trigger.");
            strcpy(ui_data[target_fld].bottomRow, "NETWORK ERROR   ");
            show_now = 1;
            xSemaphoreGive(modemMutex); 
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            strcpy(ui_data[target_fld].bottomRow, "YES ?           ");
            show_now = 1;
            portENTER_CRITICAL(&syncMux);
            sync_mode = eSMSStop;
            portEXIT_CRITICAL(&syncMux);
            continue; 
          }
        }

      // v5.82: The mutex taken above at Line 53 is STILL HELD here! 

      if (gprs_mode == eGprsSignalOk) {
        strcpy(ui_data[target_fld].bottomRow, "CONNECTING...");
        if (mode_snap == eSMSStart) {
          debugln("[GPRS] Keypad Triggered SMS Status");

          // Re-verify network & APN for manual triggers (Ensures connection is live)
          get_signal_strength();
          get_network();
          get_registration();
          get_a7672s();

          strcpy(ui_data[target_fld].bottomRow, "SENDING SMS...  ");
          vTaskDelay(2000 / portTICK_PERIOD_MS);
          
          prepare_and_send_status(universalNumber, true);

          if (msg_sent) {
            strcpy(ui_data[target_fld].bottomRow, "SMS SUCCESS     ");
          } else {
            strcpy(ui_data[target_fld].bottomRow, "SMS FAILED      ");
          }
          // NOTE: modemMutex released by the single outer xSemaphoreGive below

          show_now = 1;
          vTaskDelay(3000 / portTICK_PERIOD_MS);
          strcpy(ui_data[target_fld].bottomRow, "YES ?           ");
          show_now = 1;

          if (mode_snap == eSMSStart) {
            portENTER_CRITICAL(&syncMux);
            sync_mode = eSMSStop;
            portEXIT_CRITICAL(&syncMux);
          }
        } else if (mode_snap == eGPSStart) {
          debugln("[GPRS] Keypad Triggered GPS Send");
          if (signal_strength <= -110) {
            strcpy(ui_data[target_fld].bottomRow, "SEEK SIGNAL...  ");
            show_now = 1;
            get_signal_strength();
            get_network();
            get_registration();
            get_a7672s();
          }

          strcpy(ui_data[target_fld].bottomRow, "GETTING GPS...  ");
          show_now = 1;
          vTaskDelay(500 / portTICK_PERIOD_MS); 
          get_lat_long_date_time(universalNumber, true);

          if (msg_sent) {
            snprintf(ui_data[target_fld].bottomRow, 17, "%0.3f,%0.3f", lati, longi);
          } else {
            strcpy(ui_data[target_fld].bottomRow, "SMS SEND FAILED ");
          }
          show_now = 1;
          vTaskDelay(4000 / portTICK_PERIOD_MS);
          strcpy(ui_data[target_fld].bottomRow, "YES ?           ");
          show_now = 1;

          if (mode_snap == eGPSStart) {
            portENTER_CRITICAL(&syncMux);
            sync_mode = eSMSStop;
            portEXIT_CRITICAL(&syncMux);
          }
        } else if (mode_snap == eStartupGPS || mode_snap == eHealthStart) {
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
          vTaskDelay(2000 / portTICK_PERIOD_MS); 

#if ENABLE_HEALTH_REPORT == 1
            if (target_fld == FLD_SEND_HEALTH) {
               strcpy(ui_data[target_fld].bottomRow, "SENDING HEALTH..");
               show_now = 1;
            }
            bool health_ok = send_health_report(false); // Manual: No Jitter

            portENTER_CRITICAL(&syncMux);
            health_in_progress = false;
            portEXIT_CRITICAL(&syncMux);

            if (target_fld == FLD_SEND_HEALTH) {
               if (health_ok) strcpy(ui_data[target_fld].bottomRow, "HEALTH SUCCESS! ");
               else strcpy(ui_data[target_fld].bottomRow, "HEALTH FAILED   ");
               show_now = 1;
               vTaskDelay(3000 / portTICK_PERIOD_MS);
             }
#else
            debugln("[Health] Startup send skipped (disabled).");
#endif
        } // Close targets (GPS, Health, etc)
        
        xSemaphoreGive(modemMutex); // Release lock after transmission attempt
      } // Close gprs_mode == eGprsSignalOk
      } else {
        debugln("[GPRS] Error: Modem Mutex Timeout during manual trigger!");
        strcpy(ui_data[target_fld].bottomRow, "MODEM BUSY      ");
        show_now = 1;
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        strcpy(ui_data[target_fld].bottomRow, "YES ?           ");
        show_now = 1;
        
        portENTER_CRITICAL(&syncMux);
        sync_mode = eSMSStop; // Reset state to unlock UI
        portEXIT_CRITICAL(&syncMux);
      } // Close xSemaphoreTake(modemMutex)

      // [HR-C01] Unified Result Display (Ensures LCD updates after modem/mutex release)
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
        // If msg_sent is 0, check if we should show failure (excluding auto-tasks)
        if (target_fld != FLD_SEND_HEALTH && target_fld != FLD_SEND_STATUS) {
            // Note: If gprs_mode was never SignalOk, this provides feedback
            if (strlen(ui_data[target_fld].bottomRow) == 0 || strstr(ui_data[target_fld].bottomRow, "CONNECTED")) {
                 strcpy(ui_data[target_fld].bottomRow, "SEND FAILED     ");
            }
            show_now = 1;
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            strcpy(ui_data[target_fld].bottomRow, "YES ?           ");
            show_now = 1;
        }
      }
      
      show_now = 1; // Trigger UI refresh

      portENTER_CRITICAL(&syncMux);
      // v5.81 Hardened: If this was a pure manual trigger (not piggybacked on scheduler HTTP), allow instant sleep!
      if (__atomic_load_n(&httpInitiated, __ATOMIC_ACQUIRE) == false) {
        sync_mode = eHttpStop;
        gprs_started = false;
      } else {
        sync_mode = eSMSStop;
      }
      portEXIT_CRITICAL(&syncMux);
    } // Close manual trigger block (Line 35)

    if (!rtcReady) {
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      continue;
    }
    if (gprs_mode == eGprsInitial) {
      vTaskDelay(100 / portTICK_PERIOD_MS);
      if (gprs_started == false) {
        // v5.85: B-3 - Survival Mode Modem Bypass
        if (low_bat_mode_active && low_bat_skip_count > 0) {
            debugln("[PWR] Survival Mode: Skipping modem power-on this slot.");
            portENTER_CRITICAL(&syncMux);
            sync_mode = eHttpStop; // Advance state to allow sleep
            gprs_started = false;
            portEXIT_CRITICAL(&syncMux);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue; // Skip start_gprs()
        }

        if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(10000)) == pdTRUE) {
          debugln("[GPRS] Powering On...");
          signal_strength = -111;
          signal_lvl = -111; 
          strcpy(reg_status, "NA");
          gprs_pdp_ready = false; // Reset PDP state
          portENTER_CRITICAL(&syncMux);
          gprs_started = true;
          portEXIT_CRITICAL(&syncMux);
          start_gprs();
          xSemaphoreGive(modemMutex);
        }
      }
    }

    // --- REGULAR AUTOMATED DATA REPORTING ---
    if (timeSyncRequired == false) {
      portENTER_CRITICAL(&syncMux);
      int http_snap = sync_mode;
      portEXIT_CRITICAL(&syncMux);
      if (http_snap == eHttpBegin) {
        if (gprs_mode == eGprsSignalOk) {
          if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
            debugln("\n****************\nStarting Automated Data "
                    "Flow\n****************");
            xSemaphoreGive(serialMutex);
          }
          __atomic_store_n(&httpInitiated, true, __ATOMIC_RELEASE); // v5.65 P4: Mark cycle as started for responsive cleanup block

          // v5.70 (N-6): Snapshot time globals under rtcTimeMux for health/cdm scheduling
          int snap_hour, snap_min, snap_day;
          portENTER_CRITICAL(&rtcTimeMux);
          snap_hour = current_hour;
          snap_min = current_min;
          snap_day = current_day;
          portEXIT_CRITICAL(&rtcTimeMux);

          // v5.48 Daily Health Triggering (11:00 AM Primary)
          bool is_health_time = false;
#if ENABLE_HEALTH_REPORT == 1
          if (test_health_every_slot == 1) {
            is_health_time = true; // Every 15 mins
          } else if (test_health_every_slot == 0) {
            if (snap_hour == 11 && health_last_sent_day != snap_day) {
              is_health_time = true; // Daily 11 AM
            } else if (snap_hour == 12 && snap_min < 20 && health_last_sent_day != snap_day) {
              is_health_time = true; // v5.66: Graceful fallback for 11AM BSNL network congestion
            }
          }
          // If test_health_every_slot == 2, it remains false (Disabled)
#endif

          // v5.66: CDM True-Failure Fallback Check
          // If the Morning closing data (08:30) and Health windows (11:00-12:20) both pass
          // without success, the closing window is permanently missed for this day.
          if (strcmp(diag_cdm_status, "PENDING") == 0 && snap_hour >= 13 && health_last_sent_day != snap_day) {
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
              debugln("[Health] [CRIT] Sensor issue detected! Triggering one-time fault report.");
          }

          if (is_health_time || is_ota_confirm || is_sensor_fault_trigger) {
            portENTER_CRITICAL(&syncMux);
            health_in_progress = true; // Block deep sleep
            portEXIT_CRITICAL(&syncMux);
          }
#endif

          if (!skip_primary_http) {
            debugln(
                "[BATT] Proceeding with Spatika Upload regardless of battery "
                "voltage.");
            portENTER_CRITICAL(&syncMux);
            sync_mode = eHttpStarted;
            portEXIT_CRITICAL(&syncMux);
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
            portENTER_CRITICAL(&syncMux);
            sync_mode = eHttpStop; // v5.51 FIX: Advance state to prevent infinite loop!
            gprs_started = false;
            portEXIT_CRITICAL(&syncMux);
          }


          // v5.65 P4: HIGH RESPONSIVENESS - If a manual command is pending, prioritize it
          // over the lengthy/unreliable automated health report.
          if (pending_manual_status || pending_manual_gps || pending_manual_health) {
            debugln("[GPRS] 💡 Priority Interrupt: Manual command detected during automated cycle.");
            // We let the end-of-cycle block (line ~600) handle the actual transition
            // but we skip the optional automated health report below to speed things up.
            portENTER_CRITICAL(&syncMux);
            health_in_progress = false; 
            portEXIT_CRITICAL(&syncMux);
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
              portENTER_CRITICAL(&syncMux);
              health_in_progress = false; // Finished attempt - no OTA pending
              portEXIT_CRITICAL(&syncMux);
            } else {
              debugln(
                  "[Health] OTA Pending. Holding health_in_progress=TRUE to "
                  "block sleep.");
            }
#else
            debugln("[Health] Skip: Health reporting disabled.");
            portENTER_CRITICAL(&syncMux);
            health_in_progress = false; // Allow sleep
            portEXIT_CRITICAL(&syncMux);
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
      if (strlen(ftp_daily_date) >= 8) { 
        char ftpPath[50];
        // Surgical Fix: Construct proper SPIFFS path from date param
        snprintf(ftpPath, sizeof(ftpPath), "/dailyftp_%s.txt", ftp_daily_date);
        send_ftp_file(ftpPath, true);
      } else {
        debugln("[CMD] Error: Invalid FTP date format. Skipping FTP_DAILY.");
      }
#endif
      force_ftp_daily = false;
    }

    if (force_ota) {
      snprintf(ui_data[FLD_SEND_STATUS].bottomRow, sizeof(ui_data[FLD_SEND_STATUS].bottomRow), "OTA UPDATING...");
      portENTER_CRITICAL(&syncMux);
      health_in_progress = true; // Ensure sleep is blocked
      portEXIT_CRITICAL(&syncMux);
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
      if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
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
          xSemaphoreGive(fsMutex);
      } else {
          debugln("[CMD] fsMutex Timeout: Could not clear FTP queue.");
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
        char fullPath[64];
        const char* n = file.name();
        if (n[0] == '/') strncpy(fullPath, n, sizeof(fullPath) - 1);
        else snprintf(fullPath, sizeof(fullPath), "/%s", n);
        fullPath[sizeof(fullPath) - 1] = '\0';

        // List of files to PRESERVE (Only basic station info)
        if (strcmp(fullPath, "/station.doc") == 0 || strcmp(fullPath, "/station.txt") == 0 ||
            strcmp(fullPath, "/rf_fw.txt") == 0 || strcmp(fullPath, "/rf_res.txt") == 0) {
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
        portENTER_CRITICAL(&syncMux);
        sync_mode = eSMSStart;       // Transition to SMS flow before stopping
        portEXIT_CRITICAL(&syncMux);
      } else if (pending_manual_gps) {
        debugln("[GPRS] 💡 Found queued manual GPS request. Handling now...");
        pending_manual_gps = false;
        target_fld = FLD_SEND_GPS;
        portENTER_CRITICAL(&syncMux);
        sync_mode = eGPSStart;
        portEXIT_CRITICAL(&syncMux);
      } else if (pending_manual_health) {
        debugln("[GPRS] 💡 Found queued manual Health request. Handling now...");
        pending_manual_health = false;
        target_fld = FLD_SEND_HEALTH;
        portENTER_CRITICAL(&syncMux);
        sync_mode = eHealthStart;
        portEXIT_CRITICAL(&syncMux);
      } else {
        debugln("[GPRS] No queued commands. Allowing sleep.");
        portENTER_CRITICAL(&syncMux);
        sync_mode = eHttpStop; // Set success flag for sleep audit
        gprs_started = false;
        portEXIT_CRITICAL(&syncMux);
        __atomic_store_n(&httpInitiated, false, __ATOMIC_RELEASE);
        
        // v5.83 Hardening: Clear success flag after 2s so it doesn't linger as 'Stale'
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        portENTER_CRITICAL(&syncMux);
        if (sync_mode == eHttpStop) sync_mode = eSyncModeInitial;
        portEXIT_CRITICAL(&syncMux);
      }
    } else if (sync_mode == eHttpStop) {
        // v5.85: GPRS Zombie Fix. 
        // If the scheduler skipped transmission (weak signal/battery) but didn't set httpInitiated,
        // the GPRS task must still release its lock to allow sleep.
        portENTER_CRITICAL(&syncMux);
        gprs_started = false;
        portEXIT_CRITICAL(&syncMux);
    }

    esp_task_wdt_reset();
    vTaskDelay(
        1000 /
        portTICK_PERIOD_MS); // Reduced from 2000 for faster state transitions
  }
}
