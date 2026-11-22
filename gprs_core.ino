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
      if (pending_manual_status || pending_manual_gps) {
        portENTER_CRITICAL(&syncMux);
        if (pending_manual_status) {
          send_status = 1;
          sync_mode = eSMSStart;
        }
        else if (pending_manual_gps) sync_mode = eGPSStart;
        mode_snap = sync_mode; // Update local snap to trigger the handler below
        portEXIT_CRITICAL(&syncMux);
      }
    }

    if (mode_snap == eSMSStart || mode_snap == eGPSStart || mode_snap == eStartupGPS) {
      target_fld = (mode_snap == eSMSStart) ? FLD_SEND_STATUS : FLD_SEND_GPS;

      // [A-09] v5.89: Added 100ms breather to prevent contention timeout 
      // if an automated task just finished releasing the mutex.
      vTaskDelay(100 / portTICK_PERIOD_MS); 

      if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(15000)) == pdTRUE) {
        // Clear the queued pending flag once lock is successfully acquired
        if (mode_snap == eSMSStart) pending_manual_status = false;
        else if (mode_snap == eGPSStart) pending_manual_gps = false;
        
        if (gprs_mode == eGprsInitial) {
          debugln("[GPRS] Manual Trigger: Initiating Power On...");
          strcpy(ui_data[target_fld].bottomRow, "GPRS POWER ON...");
          show_now = 1;
          start_gprs();
        }
        // v6.56 C-1: This check used to run only right after a power-on above. If the
        // slot had already left the modem in eGprsSignalForStoringOnly (weak signal,
        // SIM error, no registration) the code skipped both this and the SignalOk block,
        // and the modemMutex taken above was never given back: the unit could then
        // never sleep. Now any state other than SignalOk releases it and aborts.
        {
          if (gprs_mode != eGprsSignalOk) {
            debugln("[GPRS] Network not ready. Aborting manual trigger.");
          if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            strcpy(ui_data[target_fld].bottomRow, "NETWORK ERROR   ");
            show_now = 1;
            xSemaphoreGive(i2cMutex);
          }
          xSemaphoreGive(modemMutex); 
          vTaskDelay(3000 / portTICK_PERIOD_MS);
          if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            strcpy(ui_data[target_fld].bottomRow, "YES ?           ");
            show_now = 1;
            xSemaphoreGive(i2cMutex);
          }
            portENTER_CRITICAL(&syncMux);
            sync_mode = eSMSStop;
            portEXIT_CRITICAL(&syncMux);
            continue; 
          }
        }

      // v5.82: The mutex taken above at Line 53 is STILL HELD here! 

      if (gprs_mode == eGprsSignalOk) {
        strcpy(ui_data[target_fld].bottomRow, "CONNECTING...");
        show_now = 1;
        if (mode_snap == eSMSStart) {
          debugln("[GPRS] Keypad Triggered SMS Status");

          if (signal_strength <= -110) {
            get_signal_strength();
            get_network();
            get_registration();
          }

          strcpy(ui_data[target_fld].bottomRow, "SENDING STATUS..");
          show_now = 1;
          vTaskDelay(500 / portTICK_PERIOD_MS);
          
          prepare_and_send_status(universalNumber, true);

          if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            if (msg_sent) {
              strcpy(ui_data[target_fld].bottomRow, "SMS SUCCESS     ");
            } else {
              strcpy(ui_data[target_fld].bottomRow, "SMS FAILED      ");
            }
            show_now = 1;
            xSemaphoreGive(i2cMutex);
          }
          // NOTE: modemMutex released by the single outer xSemaphoreGive below

          vTaskDelay(3000 / portTICK_PERIOD_MS);
          if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            strcpy(ui_data[target_fld].bottomRow, "YES ?           ");
            show_now = 1;
            xSemaphoreGive(i2cMutex);
          }

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
        } else if (mode_snap == eStartupGPS) {
          debugln("[GPRS] Startup/Station Change Triggered GPS fix");
          if (signal_strength == 0) {
            get_signal_strength();
            get_network();
            get_registration();
            get_a7672s();
          }
          strcpy(ui_data[target_fld].bottomRow, "GETTING GPS...  ");
          show_now = 1;
          get_gps_coordinates(true);

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

        } // Close targets (status, GPS)
        
        xSemaphoreGive(modemMutex); // Release lock after transmission attempt
      } // Close gprs_mode == eGprsSignalOk
      } else {
        debugln("[GPRS] Error: Modem Mutex Timeout during manual trigger!");
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
          strcpy(ui_data[target_fld].bottomRow, "MODEM BUSY      ");
          show_now = 1;
          xSemaphoreGive(i2cMutex);
        }
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
          strcpy(ui_data[target_fld].bottomRow, "YES ?           ");
          show_now = 1;
          xSemaphoreGive(i2cMutex);
        }
        
        portENTER_CRITICAL(&syncMux);
        sync_mode = eSMSStop; // Reset state to unlock UI
        health_in_progress = false; // [H-03] Reset health_in_progress on mutex timeout to allow deep sleep
        portEXIT_CRITICAL(&syncMux);
      } // Close xSemaphoreTake(modemMutex)

      // [HR-C01] Unified Result Display (Ensures LCD updates after modem/mutex release)
      if (msg_sent == 1) {
        if (target_fld == FLD_SEND_GPS) {
          if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            snprintf(ui_data[target_fld].bottomRow, 17, "%0.3f,%0.3f", lati, longi);
            show_now = 1;
            xSemaphoreGive(i2cMutex);
          }
          vTaskDelay(4000 / portTICK_PERIOD_MS);
          if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            strcpy(ui_data[target_fld].bottomRow, "YES ?           ");
            show_now = 1;
            xSemaphoreGive(i2cMutex);
          }
        } else if (target_fld != FLD_SEND_STATUS) {
          if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            strcpy(ui_data[target_fld].bottomRow, "SENT SUCCESS    ");
            show_now = 1;
            xSemaphoreGive(i2cMutex);
          }
          vTaskDelay(3000 / portTICK_PERIOD_MS);
          if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            strcpy(ui_data[target_fld].bottomRow, "YES ?           ");
            show_now = 1;
            xSemaphoreGive(i2cMutex);
          }
        }
      } else {
        // If msg_sent is 0, check if we should show failure (excluding auto-tasks)
        if (target_fld != FLD_SEND_STATUS) {
            // Note: If gprs_mode was never SignalOk, this provides feedback
            if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
              if (strlen(ui_data[target_fld].bottomRow) == 0 || strstr(ui_data[target_fld].bottomRow, "CONNECTED")) {
                   strcpy(ui_data[target_fld].bottomRow, "SEND FAILED     ");
              }
              show_now = 1;
              xSemaphoreGive(i2cMutex);
            }
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
              strcpy(ui_data[target_fld].bottomRow, "YES ?           ");
              show_now = 1;
              xSemaphoreGive(i2cMutex);
            }
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
            if (primary_data_delivered) {
              debugln("[HTTP] Primary Server Upload: SUCCESS (200 OK)");
            } else {
              debugln("[HTTP] Primary Server Upload: FAILED (Data stored to Backlog)");
            }
          } else {
            debugln("[SCHED] Skipping Duplicate/Fresh Upload. Checking Backlog...");
            primary_data_delivered = true; // Assume 'Success' to allow backlog
            portENTER_CRITICAL(&syncMux);
            sync_mode = eHttpStop; // v5.51 FIX: Advance state to prevent infinite loop!
            gprs_started = false;
            portEXIT_CRITICAL(&syncMux);
            send_sms(); // v6.57: VARSHA commands are read every slot, sent or not
          }


          // v6.56: ESP32 OTA check/download, after the slot's record and backlog
          // (v6.57: the day check also carries the daily health report, health_day.ino)
          ota_slot_step(sampleNo);
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

    // VARSHA ERASE_DATA <id> (sms_remote.ino): records and backlog wiped, then a restart
    if (force_delete_data) {
      debugln("[CMD] ERASE_DATA: wiping records and backlog by SMS...");

      // v6.56 M-8: the wipe holds fsMutex (the scheduler or the web page could be writing
      // a file while it was walked and deleted). Held through the restart.
      if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(15000)) != pdTRUE) {
        debugln("[CMD] DELETE_DATA: fsMutex busy for 15 s: wiping anyway before restart.");
      }

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

    // v7.90: Final Cycle Reset: after the slot's commands, before loopTask lets the unit sleep
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
      } else {
        debugln("[GPRS] No queued commands. Allowing sleep.");
        portENTER_CRITICAL(&syncMux);
        sync_mode = eHttpStop; // Set success flag for sleep audit
        gprs_started = false;
        portEXIT_CRITICAL(&syncMux);
        __atomic_store_n(&httpInitiated, false, __ATOMIC_RELEASE);
        set_sys_status("IDLE");
        
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
