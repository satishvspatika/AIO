#include "globals.h"

void prepare_data_and_send() {

  // v5.65 P0: Range guard for server configuration index
  if (http_no < 0 || http_no >= (int)(sizeof(httpSet) / sizeof(httpSet[0]))) {
    debugf("[HTTP] FATAL: http_no out of range (%d). Aborting send.\n", http_no);
    return;
  }

  esp_task_wdt_reset();

  // v5.83: If we are in backlog mode, the buffer was ALREADY filled by gprs_core
  // so we skip the early-return check and let the loop process it.
  // if (strlen(gprs_payload) < 10 && data_mode == eUnsentData) { ... }

  // v5.65: Guard against unconfigured UNIT string (http_no == -1).
  // Without this, httpSet[-1] or httpSet[11+] causes memory corruption.
  // http_no = -1 is set by setup() when UNIT doesn't match any known branch.
  const int httpSet_count = (int)(sizeof(httpSet) / sizeof(httpSet[0]));
  if (http_no < 0 || http_no >= httpSet_count) {
    debugln("[HTTP] FATAL: http_no out of range. UNIT/SYSTEM not configured!");
    debugln(
        "[HTTP] Check UNIT and SYSTEM defines in globals.h. Aborting send.");
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
    // v5.83 Hardening: Zero out the payload buffer BEFORE file read
    // to prevent stale RAM data from leaking if a file read fails.
    memset(gprs_payload, 0, sizeof(gprs_payload));

    if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
      File file1 = SPIFFS.open(temp_file, FILE_READ);
      if (file1) {
        debug("SPIFF FILE EXISTS ....");
        debugln(temp_file);
        s = file1.size();
        // v5.85 Hardened: Safely jump back ~3 records to generously accommodate
        // highly variable negative/positive decimal string fluctuations without
        // accidentally landing inside the final record and swallowing it.
        int seek_back = (record_length * 3);
        s = (s > (size_t)seek_back) ? (s - seek_back) : 0;
        
        file1.seek(s);

        if (s > 0) {
           // Skip partial line
           while(file1.available()) {
              char c = file1.read();
              if (c == '\n') break;
           }
        }

        // Read the actual last valid record into gprs_payload
        // v5.85: Iterate to the very last line to guarantee we don't pull
        // the previous slot if the record size was smaller than seek_back.
        char temp_buf[sizeof(gprs_payload)];
        while(file1.available()) {
           int len = read_line_to_buf(file1, temp_buf, sizeof(temp_buf));
           if (len > 10) {
              strcpy(gprs_payload, temp_buf);
           }
        }
        file1.close();
      } else {
        debugln("Failed to open temp_file for reading");
      }
      xSemaphoreGive(fsMutex);
      if (strlen(gprs_payload) < 10)
        return; 
    } else {
      debugln("[FS] Mutex Timeout: Skipping main data read.");
      return;
    }
  }

  const char *charArray = gprs_payload;

  debugln();
  debugf1("Current Data to be sent is : %s", charArray);
  debugln();

  // v5.65: Zero metadata variables before parsing to prevent stale data
  // from a previous successful send being used if this sscanf fails.
  temp_sampleNo = -1;
  temp_year = 0;
  temp_month = 0;
  temp_day = 0;
  temp_hr = 0;
  temp_min = 0;
  temp_sig = 0;
  temp_bat = 0.0;

#if SYSTEM == 0
  int res = sscanf(charArray, "%02d,%04d-%02d-%02d,%02d:%02d,%f,%f,%04d,%f",
                   &temp_sampleNo, &temp_year, &temp_month, &temp_day, &temp_hr,
                   &temp_min, &temp_instrf, &temp_crf, &temp_sig, &temp_bat);
  if (res < 10) {
    debugln("[HTTP] ERROR: sscanf parse failed (TRG). Skipping record.");
    success_count = 2;
    return;
  }
#endif
#if SYSTEM == 1
  // TWS: 12 components
  int res = sscanf(
      charArray, "%02d,%04d-%02d-%02d,%02d:%02d,%f,%f,%f,%03d,%04d,%f",
      &temp_sampleNo, &temp_year, &temp_month, &temp_day, &temp_hr, &temp_min,
      &temp_temp, &temp_hum, &temp_avg_ws, &temp_dir, &temp_sig, &temp_bat);
  if (res < 12) {
    debugln("[HTTP] ERROR: sscanf parse failed (TWS). Skipping record.");
    success_count = 2;
    return;
  }
#endif
#if SYSTEM == 2
  // TWS-RF: 13 components
  int res = sscanf(charArray,
                   "%02d,%04d-%02d-%02d,%02d:%02d,%f,%f,%f,%f,%03d,%04d,%f",
                   &temp_sampleNo, &temp_year, &temp_month, &temp_day, &temp_hr,
                   &temp_min, &temp_crf, &temp_temp, &temp_hum, &temp_avg_ws,
                   &temp_dir, &temp_sig, &temp_bat);
  if (res < 13) {
    debugln("[HTTP] ERROR: sscanf parse failed (TWS-RF). Skipping record.");
    success_count = 2;
    return;
  }
#endif

  // v5.77: Timestamp-Redundancy Fix
  // If we are sending the 'Current' met-day record, force the timestamp
  // to be exactly what the scheduler intended (record_hr/min) rather than
  // what sscanf parsed from the file. This prevents duplicate entries in the
  // backlog if the file contains a legacy record from the previous slot.
  if (data_mode == eCurrentData) {
    temp_hr = record_hr;
    temp_min = record_min;
  }

#if (SYSTEM == 0)
  snprintf(sample_bat, sizeof(sample_bat), "%04.1f", float(temp_bat));
#endif
#if (SYSTEM == 1) || (SYSTEM == 2)
  // v5.52 LOOP-3 FIX: Guard against empty station name before building FTP
  // filename
  if (strlen(ftp_station) == 0) {
    debugln("[FTP] Skip: ftp_station (Station ID) not configured.");
    return;
  }
  sample_WD[3] = 0; // AG1
  snprintf(sample_temp, sizeof(sample_temp), "%05.1f", float(temp_temp));
  snprintf(sample_hum, sizeof(sample_hum), "%05.1f", float(temp_hum));
  snprintf(sample_avgWS, sizeof(sample_avgWS), "%04.1f", float(temp_avg_ws));
  snprintf(ftpsample_avgWS, sizeof(ftpsample_avgWS), "%04.1f",
           float(temp_avg_ws)); // v1.10 from %04.1f
  snprintf(sample_WD, sizeof(sample_WD), "%03d", temp_dir);
  snprintf(sample_bat, sizeof(sample_bat), "%05.2f", float(temp_bat));
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
    temp_sig =
        signal_lvl; // v6.75: Use actual live signal level for current slot
  }

  // v5.50: BOGUS DATA GUARD — After DELETE DATA all SPIFFS files are wiped.
  // On the next boot, signature.txt is recreated as "0000-00-00,00:00"
  // which causes temp_year=0 after sscanf. Block this slot entirely and
  // show a clear LCD message so the user knows the unit is in fresh-start mode.
  if (temp_year == 0) {
    debugln(
        "[HTTP] ⚠ Bogus date detected (year=0). Skipping HTTP — fresh "
        "start or DELETE DATA reboot. Waiting for first valid 15-min slot.");
    snprintf(ui_data[FLD_SEND_STATUS].bottomRow,
             sizeof(ui_data[FLD_SEND_STATUS].bottomRow), "YES ?           ");
    snprintf(ui_data[FLD_LAST_LOGGED].bottomRow,
             sizeof(ui_data[FLD_LAST_LOGGED].bottomRow), "NO DATA YET");
    portENTER_CRITICAL(&syncMux);
    sync_mode = eHttpStop;
    portEXIT_CRITICAL(&syncMux);
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

  // v5.75 Hardened: KSNDMC Padding Rule - Pad 4-digit numeric IDs to 6-digits
  // (e.g. 1921 -> 001921) Required for TRG, TWS and TWS-RF legacy servers on
  // KSNDMC to match database primary keys.
      if (strlen(cleanStn) == 4 && isDigitStr(cleanStn)) {
        char padded[16];
        snprintf(padded, sizeof(padded), "00%s", cleanStn);
        strncpy(cleanStn, padded, sizeof(cleanStn) - 1);
        cleanStn[sizeof(cleanStn) - 1] = '\0';
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
             "\"WD\":\"%s\",\"BatVolt\":\"%s\",\"SigStr\":\"%04d\",\"ApiKey\":"
             "\"%s\"}",
             cleanStn, temp_year, temp_month, temp_day, temp_hr, temp_min,
             sample_temp, sample_hum, sample_avgWS, sample_WD, sample_bat,
             temp_sig, httpSet[http_no].Key);
#endif
#if SYSTEM == 2
    // TWS-RF JSON Format
    snprintf(http_data, sizeof(http_data),
             "{\"StnNo\":\"%s\",\"DeviceTime\":\"%04d-%02d-%02d "
             "%02d:%02d:00\",\"RainCuml\":\"%05.1f\",\"Temp\":\"%s\",\"Humid\":"
             "\"%s\","
             "\"WS\":\"%s\",\"WD\":\"%s\",\"BatVolt\":\"%s\",\"SigStr\":\"%"
             "04d\",\"ApiKey\":\"%s\"}",
             cleanStn, temp_year, temp_month, temp_day, temp_hr, temp_min,
             temp_crf, sample_temp, sample_hum, sample_avgWS, sample_WD,
             sample_bat, temp_sig, httpSet[http_no].Key);
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
      debugf("Key is %s\n", httpSet[http_no].Key);
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
      snprintf(
          http_data, sizeof(http_data),
          "stn_id=%s&rec_time=%04d-%02d-%02d,%02d:%02d&key=%s&rainfall=%05."
          "2f&temp=%s&humid=%s&w_speed=%s&w_dir=%s&signal=%04d&"
          "bat_volt=%s&bat_volt2=%s",
          cleanStn, temp_year, temp_month, temp_day, temp_hr, temp_min,
          httpSet[http_no].Key, temp_crf, sample_temp, sample_hum, sample_avgWS,
          sample_WD, temp_sig, sample_bat, sample_bat);
    } else {
      snprintf(
          http_data, sizeof(http_data),
          "stn_no=%s&rec_time=%04d-%02d-%02d,%02d:%02d&key=%s&rainfall=%05."
          "1f&temp=%s&humid=%s&w_speed=%s&w_dir=%s&signal=%04d&"
          "bat_volt=%s&bat_volt2=%s",
          cleanStn, temp_year, temp_month, temp_day, temp_hr, temp_min,
          httpSet[http_no].Key, temp_crf, sample_temp, sample_hum, sample_avgWS,
          sample_WD, temp_sig, sample_bat, sample_bat);
    }
  }
#endif

  debug("http_data format is ");
  debugln(http_data);
  debugln();

  // v5.83 Elite Polish: Only audit DNS if the endpoint is a domain name
  bool is_domain = false;
  const char *srvStr = httpSet[http_no].serverName;
  for (int d = 0; srvStr[d] != '\0'; d++) {
    if (isalpha(srvStr[d])) {
      is_domain = true;
      break;
    }
  }

  if (is_domain) {
    debugln("[DNS] Auditing resolution stack...");
    SerialSIT.println("AT+CDNSCFG?");
    waitForResponse("OK", 2000);
    if (strstr(modem_response_buf, "0.0.0.0") != NULL || strstr(modem_response_buf, "127.0.0.1") != NULL) {
        debugln("[DNS] Invalid DNS detected. Forcing Refresh to 8.8.8.8...");
        SerialSIT.println("AT+CDNSCFG=\"8.8.8.8\",\"8.8.4.4\"");
        waitForResponse("OK", 1000);
        
        // Only settle when DNS was actually changed
        debugln("[GPRS] Allowing network stack 2s to stabilize after DNS refresh...");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
  }

  // v5.63: Selective Mode Logic (Airtel/Jio Backlog Optimization)
  bool isAirtelOrJio = (strstr(carrier, "Airtel") != nullptr ||
                        strstr(carrier, "Jio") != nullptr);

  if (isAirtelOrJio && data_mode == eUnsentData) {
    // M2M SIM backlog: Skip Fast, jump straight to Robust.
    // Fast always times out on M2M SIMs after a session rebuild/zombie state.
    debugln("[HTTP] Airtel/Jio backlog: using Robust direct.");
    success_count = send_at_cmd_data(http_data, true);
  } else {
    // Current data (all carriers) or Backlog for BSNL: Fast -> Fast -> Robust
    success_count = send_at_cmd_data(http_data, false);
    if (success_count == 0) {
      debugln("[HTTP] 1st Attempt (Fast) failed. Retrying in 2s (Fast Attempt "
              "2)...");
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      success_count = send_at_cmd_data(http_data, false);
      if (success_count == 0) {
        debugln("[HTTP] 2nd Attempt (Fast) also failed. Falling back to Robust "
                "method...");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        success_count = send_at_cmd_data(http_data, true);
      }
    }
  }
  // v6.76: One retry added above for current data.
  // Fails after retry will trigger FTP backlog storage.

  if (success_count == 1) {
    diag_consecutive_http_fails = 0; // v5.49 Build 5: Reset fail streaks
    diag_consecutive_reg_fails = 0;
    last_http_ok = true;          // v5.84: Success recorded
    last_http_ok_slot = sampleNo; // v5.72: Store successful slot for bearer age
                                  // check (fixed from temp_sampleNo)

    // v7.70+: PR counter resets on ANY successful HTTP/FTP (Current or Backlog)
    diag_http_present_fails = 0;
    snprintf(ui_data[FLD_HTTP_FAILS].bottomRow,
             sizeof(ui_data[FLD_HTTP_FAILS].bottomRow), "P:%d C:%d B:%d",
             diag_http_present_fails, diag_http_cum_fails,
             get_total_backlogs(false));

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
    // If that fails [INFO] store to backlog immediately and sleep.
    // The 15-min wake cycle + backlog is the recovery mechanism.
    // ─────────────────────────────────────────────────────────────────────

    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
      debugln(
          "[HTTP] Attempt 1 failed. Re-initialising stack for single retry...");
      xSemaphoreGive(serialMutex);
    }
    esp_task_wdt_reset();

    // v5.58: Hard-Kill 706/714 TCP Zombie Guard (Airtel Fix)
    bool tcp_zombie = (strcmp(diag_http_fail_reason, "706") == 0 ||
                       strcmp(diag_http_fail_reason, "713") == 0 ||
                       strcmp(diag_http_fail_reason, "714") == 0 ||
                       strcmp(diag_http_fail_reason, "TIMEOUT") == 0);

    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 3000);

    if (tcp_zombie) {
      if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
        debugln("[HTTP] TCP Zombie (706/714) detected. Executing Hard Bearer "
                "Nuke...");
        xSemaphoreGive(serialMutex);
      }

      // v5.85.1: Invalidate false freshness - Bearer is dead
      last_http_ok = false;
      last_http_ok_slot = -1;

      dns_fallback_active =
          false; // v5.72: Clear fallback cache to force fresh DNS next slot

      // Mandatory Nuke Protocol
      SerialSIT.println("AT+CIPSHUT");
      waitForResponse("SHUT OK", 3000);

      SerialSIT.println("AT+CGACT=0,1");
      waitForResponse("OK", 2000);

      vTaskDelay(5000 /
                 portTICK_PERIOD_MS); // Crucial 5-second carrier breather
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
      if (waitForResponse("OK", 5000)) {
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
        // rate on weak/noisy networks, as this is already a retry cycle.
        success_count = send_at_cmd_data(http_data, true);

        // v5.63: Tower Cooldown. Airtel towers need ~3s to clear previous
        // socket before accepting a new rapid-fire request in a backlog loop.
        vTaskDelay(3000 / portTICK_PERIOD_MS);

        if (success_count == 1) {
          diag_consecutive_reg_fails = 0;
          diag_consecutive_http_fails = 0;
          last_http_ok = true;          // v5.84: Success on retry
          last_http_ok_slot = sampleNo; // v5.72: Keep bearer age fresh (fixed
                                        // from temp_sampleNo)

          // v7.70+: Reset present fails on successful backlog retry too
          diag_http_present_fails = 0;
          snprintf(ui_data[FLD_HTTP_FAILS].bottomRow,
                   sizeof(ui_data[FLD_HTTP_FAILS].bottomRow), "P:%d C:%d B:%d",
                   diag_http_present_fails, diag_http_cum_fails,
                   get_total_backlogs(false));

          if (data_mode == eCurrentData) {
            // Success tracking centralized in send_at_cmd_data loop below
            diag_first_http_count++; // v7.86
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
      last_http_ok =
          false; // v5.84: Failure recorded - force full check next slot

      // v5.72: Force DNS re-resolution if the failure was a TCP zombie code
      if (strstr(diag_http_fail_reason, "706") != nullptr ||
          strstr(diag_http_fail_reason, "713") != nullptr ||
          strstr(diag_http_fail_reason, "714") != nullptr) {
        dns_fallback_active = false;
        debugln("[DNS] Cleared fallback cache due to TCP zombie failure code.");
      }

      diag_consecutive_http_fails++;
      diag_daily_http_fails++;
      if (data_mode == eCurrentData) {
        // v7.70: Monthly cum reset logic (Robust: any day of new month triggers
        // it)
        if (current_month > 0 && diag_cum_fail_reset_month != current_month) {
          diag_http_cum_fails = 0;
          diag_cum_fail_reset_month = current_month;
          debugln(
              "[HTTP] Monthly cum fail counter reset (New Month detected).");
        }
        diag_http_present_fails++;
        diag_http_cum_fails++;
        snprintf(ui_data[FLD_HTTP_FAILS].bottomRow,
                 sizeof(ui_data[FLD_HTTP_FAILS].bottomRow), "P:%d C:%d B:%d",
                 diag_http_present_fails, diag_http_cum_fails,
                 get_total_backlogs(false));
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
                 "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%04d,%05.2f\r\n",
                 temp_sampleNo, temp_year, temp_month, temp_day, temp_hr,
                 temp_min, inst_rf, cum_rf, temp_sig, temp_bat);
#endif
#if SYSTEM == 1 // TWS
        snprintf(current_record, sizeof(current_record),
                 "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%s,%s,%04d,%05.2f\r\n",
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
                 "%s;%04d-%02d-%02d,%02d:%02d;%s;%s;%s;%s;%04d;%05.2f\r\n",
                 stnId, temp_year, temp_month, temp_day, temp_hr, temp_min,
                 sample_temp, sample_hum, sample_avgWS, sample_WD, temp_sig,
                 temp_bat);
        debug("ftpappend_text is : ");
        debugln(ftpappend_text);
#endif
#if SYSTEM == 2 // TWS-RF
        snprintf(current_record, sizeof(current_record),
                 "%02d,%04d-%02d-%02d,%02d:%02d,%s,%s,%s,%s,%s,%04d,%05.2f\r\n",
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
                 "%s;%04d-%02d-%02d,%02d:%02d;%s;%s;%s;%s;%s;%04d;%05.2f\r\n",
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
        // v5.55 SELF-HEALING: If we are failing 4 times in a row, the bearer is
        // likely a zombie. Force a HARD MODEM RESET (GPIO 26 cycle) as per
        // Rule 19.
        if (diag_consecutive_http_fails >= 4) {
          debugln("[RECOVERY] 4 Consecutive HTTP Fail Slots (Zombie Bearer). "
                  "Hard Resetting Modem...");
          SerialSIT.println("AT+HTTPTERM");
          waitForResponse("OK", 2000);
          digitalWrite(26, LOW);                 // Pull GPRS Power LOW
          vTaskDelay(2500 / portTICK_PERIOD_MS); // ≥2s guaranteed off
          digitalWrite(26, HIGH);                // Pull GPRS Power HIGH
          vTaskDelay(8000 / portTICK_PERIOD_MS); // allow boot + SIM init
          recoverI2CBus(); // v5.82 Platinum: Mitigate spike on HTTP recovery
          diag_consecutive_http_fails = 0; // Reset counter - fresh start
        }

        char finalBuffer[100]; // AG1 [record_length + 1];

        // SYSTEM 0 BACKLOG APPEND handled above in unified TWS pattern v7.70
#if SYSTEM == 0
        if (last_unsent_sampleNo !=
            sampleNo) { // v5.72 Hardened: Dedup guard for TRG fallback write
          if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
            snprintf(unsent_file, sizeof(unsent_file), "/unsent.txt");
            File file2 = SPIFFS.open(unsent_file, FILE_APPEND);
            if (file2) {
              file2.print(
                  current_record); // Fixed: current_record is the buffer
              file2.close();
            }
            xSemaphoreGive(fsMutex);
          }
          last_unsent_sampleNo = sampleNo;
        }
#endif

#if SYSTEM == 1
        // 001881;2025-11-07,20:30;000.0;000.0;00.00;000;-083;04.2
        // v5.72: Removed hardcoded 57-byte truncation. Using full buffer for
        // reliability.
        strcpy(finalBuffer, ftpappend_text);

        debugln("Record written into /ftpunsent.txt is as below : ");
        debugln(finalBuffer);
        if (last_ftp_unsent_sampleNo !=
            temp_sampleNo) { // [FTP-03] Persistent Dedup
          if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
            snprintf(ftpunsent_file, sizeof(ftpunsent_file), "/ftpunsent.txt");
            File ftpfile2 = SPIFFS.open(ftpunsent_file, FILE_APPEND);
            if (ftpfile2) {
              ftpfile2.print(finalBuffer);
              ftpfile2.close();
              last_ftp_unsent_sampleNo = temp_sampleNo; // Mark as written
            } else {
              debugln("Failed to open ftpunsent.txt for appending (TWS)");
            }
            xSemaphoreGive(fsMutex);
          } else {
            debugln("[GPRS] fsMutex Timeout: Could not append to ftpunsent.txt "
                    "(TWS)");
          }
        } else {
          debugln("[HTTP] Record already in FTP backlog. Skipping duplicate.");
        }
#endif

#if SYSTEM == 2
        // 001881;2024-05-21,08:45;000.0;000.0;000.0;00.00;000;-111;04.2
        // v5.72: Removed hardcoded 63-byte truncation. Full buffer required for
        // 9-field TWSRF record.
        strcpy(finalBuffer, ftpappend_text);

        debugln("Record written into /ftpunsent.txt is as below : ");
        debugln(finalBuffer);
        if (last_ftp_unsent_sampleNo !=
            temp_sampleNo) { // [FTP-03] Persistent Dedup
          if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
            snprintf(ftpunsent_file, sizeof(ftpunsent_file), "/ftpunsent.txt");
            File ftpfile2 = SPIFFS.open(ftpunsent_file, FILE_APPEND);
            if (ftpfile2) {
              ftpfile2.print(finalBuffer);
              ftpfile2.close();
              last_ftp_unsent_sampleNo = temp_sampleNo; // Mark as written
            } else {
              debugln("Failed to open ftpunsent.txt for appending (TWS-RF)");
            }
            xSemaphoreGive(fsMutex);
          } else {
            debugln("[GPRS] fsMutex Timeout: Could not append to ftpunsent.txt "
                    "(TWS-RF)");
          }
        } else {
          debugln("[HTTP] Record already in FTP backlog. Skipping duplicate.");
        }
#endif
      } // closes if(data_mode == eCurrentData)
    }   // closes if(success_count == 0 - second try)
  }     // closes if(success_count == 0 - first try)
  // v5.70: fsMutex is no longer held here.
  // GPRS task now follows granular lock pattern.
} // closes prepare_data_and_send()

void send_http_data() {
  if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(15000)) != pdTRUE) {
    debugln("[GPRS] Error: Modem Mutex Timeout - deferring HTTP send");
    diag_modem_mutex_fails++;
    return;
  }

  // Clear any stale TCP errors from previous runs to prevent false-positive
  // network nuke loops
  diag_http_fail_reason[0] = '\0';

  const char *charArray;
  /*
   * PREPARE HTTP PARAMS
   */

  const char *domain = httpSet[http_no].serverName;
  // char target_ip[64] = {0}; // Removed, target_ip is now handled by
  // httpPostRequest directly

  bool is_ip_format = true;
  for (int i = 0; domain[i] != '\0'; i++) {
    if (isalpha(domain[i])) {
      is_ip_format = false;
      break;
    }
  }

  // v5.55: PREPARE URL (Zero-Gap Prep)
  // Logic: Use static IP from globals.h if DNS is problematic or if first
  // attempt fails.
  snprintf(httpPostRequest, sizeof(httpPostRequest),
           "AT+HTTPPARA=\"URL\",\"http://%s:%s%s\"", domain,
           httpSet[http_no].Port, httpSet[http_no].Link);

  char fallbackUrl[150] = {0};
  snprintf(fallbackUrl, sizeof(fallbackUrl),
           "AT+HTTPPARA=\"URL\",\"http://%s:%s%s\"", httpSet[http_no].IP,
           httpSet[http_no].Port, httpSet[http_no].Link);

  debugf("[GPRS] Prepared URL: %s\n", httpPostRequest);

  // v7.05: Bulletproof Context-Ready Anchor
  // Before starting HTTP, we MUST have a non-zero IP address.
  // Raw CGACT=1 is not enough for the A7672S HTTP internal state.
  bool ip_ready = false;
  for (int ip_retry = 0; ip_retry < 5; ip_retry++) {
    esp_task_wdt_reset(); // Pet the watchdog during each IP check iteration
    SerialSIT.print("AT+CGPADDR=");
    SerialSIT.println(active_cid); 
    if (waitForResponse("+CGPADDR:", 3000)) {
       if (strstr(modem_response_buf, "0.0.0.0") == NULL && strstr(modem_response_buf, ".") != NULL) {
          ip_ready = true;
          break;
       }
    }
    debugln("[GPRS] Waiting for IP address allocation...");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    if (ip_retry == 2) {
       debugln("[GPRS] IP Delay: Nuking context for re-activation...");
       SerialSIT.print("AT+CGACT=0,"); SerialSIT.println(active_cid); waitForResponse("OK", 3000);
       SerialSIT.print("AT+CGACT=1,"); SerialSIT.println(active_cid); waitForResponse("OK", 15000);
    }
  }
  
  if (!ip_ready) {
    debugln("[GPRS] FATAL: Context active but no IP assigned. Deferring.");
    return;
  }

  // v5.55: SMART DNS FALLBACK (Fast-Track)
  // v5.55 SELF-HEALING: Every 10 successful sends, or if we just recovered from
  // a fail streak, try DNS again to see if the network has healed.
  bool forceDnsRetry =
      (diag_http_success_count % 10 == 0) || (diag_consecutive_http_fails == 1);

  if (dns_fallback_active && strcmp(cached_server_domain, domain) == 0 &&
      !forceDnsRetry) {
    debugf("[GPRS] Smart Fallback ACTIVE. Bypassing DNS trials for %s\n", domain);
    strncpy(httpPostRequest, fallbackUrl, sizeof(httpPostRequest)-1);
    httpPostRequest[sizeof(httpPostRequest)-1] = '\0';
  } else if (!is_ip_format) {
    // v5.63: Reverting to v3.0 logic. Use Domain Name directly in URL.
    // The modem's internal stack (AT+HTTPACTION) is more reliable at
    // resolving DNS on private Airtel APNs than manual AT+CDNSGIP.
    snprintf(httpPostRequest, sizeof(httpPostRequest),
             "AT+HTTPPARA=\"URL\",\"http://%s:%s%s\"", domain,
             httpSet[http_no].Port, httpSet[http_no].Link);

    // v5.63: Manual DNS remains as a secondary "Diagnostic/Resolver" only if
    // domain fails But for the primary request, we use the Domain.
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

  // especially useful during combined Health + Main data slots.
  SerialSIT.println("AT+CGACT?");
  if (waitForResponse("OK", 3000)) {
    char target[20];
    snprintf(target, sizeof(target), "+CGACT: %d,1", active_cid);
    if (strstr(modem_response_buf, target) != NULL && diag_consecutive_http_fails == 0) {
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
      // v5.78 Hardening: Slot-Boundary Guard
      // If we are within 10s of a 15-min boundary, defer reset to avoid scheduler race
      int mins_into; int secs_into;
      portENTER_CRITICAL(&rtcTimeMux);
      mins_into = current_min % 15;
      secs_into = current_sec;
      portEXIT_CRITICAL(&rtcTimeMux);

      if (mins_into == 14 && secs_into >= 50) {
          debugln("[GPRS] Slot boundary imminent (T-10s). Deferring Bearer Nuke.");
          vTaskDelay(12000 / portTICK_PERIOD_MS); // Wait for boundary to pass (10s + 2s buffer)
          esp_task_wdt_reset();
      }

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
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CGACT=1,%d",
             clean_target);
    SerialSIT.println(gprs_xmit_buf);
    waitForResponse("OK", 12000);
    // diag_consecutive_http_fails = 0; // Don't reset -- only reset on success
  }
  // snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), ...); // Prepared in
  // prepare_data_and_send()

  // v5.63: Removed proactive AT+HTTPTERM here (prior to HTTPINIT) because it
  // creates a race condition on Airtel stacks if done while the session is
  // internally cleaning up. v3.0 logic: Proactive HTTPINIT directly!
  // (Deliberate Roadmap Exception N-8: TERM is handled in retry block at line
  // 814)
  vTaskDelay(50 / portTICK_PERIOD_MS);
  flushSerialSIT();

  SerialSIT.println("AT+HTTPINIT");
  if (!waitForResponse("OK", 5000)) {
    debugln("[GPRS] HTTPINIT Failed. Trying TERM then INIT...");
    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 3000);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    flushSerialSIT();
    SerialSIT.println("AT+HTTPINIT");
    if (waitForResponse("OK", 5000)) {
      http_ready = true;
    }
  } else {
    http_ready = true;
  }

  // v5.99: Carrier Settle Delay — Give BSNL/2G time to bind the IP stack
  if (strstr(carrier, "BSNL")) vTaskDelay(3000 / portTICK_PERIOD_MS);

  // Restore all parameters with individual error checking
  bool http_param_fail = false;
  SerialSIT.println("AT+HTTPPARA=\"CID\",1");
  if (!waitForResponse("OK", 5000)) http_param_fail = true;
  vTaskDelay(500 / portTICK_PERIOD_MS); // v7.01 Breather

  if (!http_param_fail) {
    SerialSIT.println(httpPostRequest);
    if (!waitForResponse("OK", 5000)) http_param_fail = true;
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }

  if (!http_param_fail) {
    SerialSIT.println("AT+HTTPPARA=\"ACCEPT\",\"*/*\"");
    if (!waitForResponse("OK", 5000)) http_param_fail = true;
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }

  if (!http_param_fail) {
    if (!strcmp(httpSet[http_no].Format, "json")) {
      SerialSIT.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
    } else {
      SerialSIT.println("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"");
    }
    if (!waitForResponse("OK", 5000)) http_param_fail = true;
  }

  if (http_param_fail) {
    debugln("[GPRS] HTTPPARA failed in main flow. Retrying TERM/INIT...");
    SerialSIT.println("AT+HTTPTERM"); waitForResponse("OK", 2000);
    SerialSIT.println("AT+HTTPINIT");
    if (!waitForResponse("OK", 5000)) {
        debugln("[GPRS] Secondary HTTPINIT failed. Deferring to next slot.");
        http_ready = false;
        diag_http_present_fails++;
        return;
    }
    // Re-attempt parameters ONCE
    SerialSIT.println("AT+HTTPPARA=\"CID\",1"); waitForResponse("OK", 2000);
    SerialSIT.println(httpPostRequest); waitForResponse("OK", 2000);
    if (!strcmp(httpSet[http_no].Format, "json")) {
       SerialSIT.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
    } else {
       SerialSIT.println("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"");
    }
    waitForResponse("OK", 2000);
  }

  /*
   * SENDING CURRENT DATA
   */

  debugln();
  debugln("*********  STARTING TO SEND HTTP ... ***********");
  debugln();
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

        if (SPIFFS.exists("/signature.txt"))
          SPIFFS.remove("/signature.txt");
        if (SPIFFS.rename("/signature.tmp", "/signature.txt")) {
          debugln("[PWR] Signature persisted successfully.");
        }
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
    // v7.70+: Broad fsMutex removed. Helper pulses used below.
    bool exists = false;
    size_t fsize = 0;
    if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
      exists = SPIFFS.exists(unsent_file);
      if (exists) {
        File f = SPIFFS.open(unsent_file, FILE_READ);
        if (f) {
          fsize = f.size();
          f.close();
        }
      }
      if (exists && fsize == 0) {
        SPIFFS.remove(unsent_file);
        SPIFFS.remove("/unsent_pointer.txt");
      }
      xSemaphoreGive(fsMutex);
    }

    if (exists && fsize > 0) {
      // Backlog exists, proceed with granular locking below
      debugln();
      debugln("*********  Sending UNSENT data to main server... ***********");
      debugln();

      // [CRIT] CRITICAL FIX: If skip_primary_http was true, send_http_data() was
      // skipped and httpPostRequest is empty. Rebuild it locally so backlog has
      // a target!
      const char *domain = httpSet[http_no].serverName;
      snprintf(httpPostRequest, sizeof(httpPostRequest),
               "AT+HTTPPARA=\"URL\",\"http://%s:%s%s\"", domain,
               httpSet[http_no].Port, httpSet[http_no].Link);
      unsent_pointer_count = 0; // resetting to 1st record of unsent data ..
      unsent_counter = 0;
      int total_unsent = 0;
      if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
        total_unsent = countStored(unsent_file);
        xSemaphoreGive(fsMutex);
      }
      debugf("[Backlog] Total records in queue: %d\n", total_unsent);

      if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
        bool ptr_loaded = false;
        if (SPIFFS.exists("/unsent_pointer.txt")) {
          File read_unsent_count =
              SPIFFS.open("/unsent_pointer.txt", FILE_READ);
          if (read_unsent_count) {
            char ptr_buf[16] = {0};
            int r = read_line_to_buf(read_unsent_count, ptr_buf, sizeof(ptr_buf));
            unsent_pointer_count = (r > 0) ? atoi(ptr_buf) : 0;
            read_unsent_count.close();
            ptr_loaded = true;
          }
        }

        // LTS-03 recovery: Check for tmp neighbor before defaulting to zero
        if (!ptr_loaded && SPIFFS.exists("/unsent_ptr.tmp")) {
          debugln("[Backlog] Recovering pointer from /unsent_ptr.tmp...");
          SPIFFS.remove("/unsent_pointer.txt");
          vTaskDelay(20 / portTICK_PERIOD_MS);
          if (SPIFFS.rename("/unsent_ptr.tmp", "/unsent_pointer.txt")) {
            File read_unsent_count =
                SPIFFS.open("/unsent_pointer.txt", FILE_READ);
            if (read_unsent_count) {
              char ptr_buf[16] = {0};
              int r = read_line_to_buf(read_unsent_count, ptr_buf, sizeof(ptr_buf));
              unsent_pointer_count = (r > 0) ? atoi(ptr_buf) : 0;
              read_unsent_count.close();
              ptr_loaded = true;
            }
          }
        }

        if (!ptr_loaded) {
          debugln("[Backlog] No pointer found. Starting from 0.");
          unsent_pointer_count = 0;
          File fTmp = SPIFFS.open("/unsent_ptr.tmp", FILE_WRITE);
          if (fTmp) {
            fTmp.print(unsent_pointer_count);
            fTmp.close();
            SPIFFS.remove("/unsent_pointer.txt");
            vTaskDelay(20 / portTICK_PERIOD_MS);
            SPIFFS.rename("/unsent_ptr.tmp", "/unsent_pointer.txt");
          }
        }
        xSemaphoreGive(fsMutex);
      }

      // filesize logic decoupled
      debugln("Backlog session ready.");

      if (strstr(carrier, "Airtel") != nullptr ||
          strstr(carrier, "Jio") != nullptr) {
        debugln(
            "[Backlog] Airtel/Jio: proactive bearer refresh before backlog.");
        SerialSIT.println("AT+HTTPTERM");
        waitForResponse("OK", 2000);
        SerialSIT.println("AT+CIPSHUT");
        waitForResponse("SHUT OK", 3000);
        SerialSIT.println("AT+CGACT=0,1");
        waitForResponse("OK", 2000);
        vTaskDelay((isLTE ? 800 : 5000) /
                   portTICK_PERIOD_MS); // v5.85: Carrier breather
        verify_bearer_or_recover();     // rebuilds APN + IP
        SerialSIT.println("AT+CGEREP=0");
        waitForResponse("OK", 1000);
        flushSerialSIT();
        SerialSIT.println("AT+HTTPINIT");
        if (waitForResponse("OK", 5000)) {
           http_ready = true;
        }
        // Set all HTTPPARA fields — required before AT+HTTPACTION can fire
        SerialSIT.println("AT+HTTPPARA=\"CID\",1");
        waitForResponse("OK", 1000);
        SerialSIT.println(
            httpPostRequest); // URL already built by
                              // send_http_data()/send_unsent_data()
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
        debugln("[Backlog] Session rebuilt and configured. Starting backlog "
                "sends.");
      }

      int backlog_processed_count = 0;
      if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(10000)) == pdTRUE) {
        File f_size = SPIFFS.open(unsent_file, FILE_READ);
        if (f_size) {
          fileSize = f_size.size();
          f_size.close();
        }
        xSemaphoreGive(fsMutex);
      }

      size_t backlogFileSize = (size_t)fileSize;
      while (unsent_pointer_count < backlogFileSize) {
        // v7.70+: Granular Lock Pulse per record fetch
        if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
          debugln("[Backlog] Mutex Timeout: Skipping record fetch.");
          // v5.70: Modem is still initialized, TERM it before breaking or let
          // it fall through
          break;
        }

        File file_backlog = SPIFFS.open(unsent_file, FILE_READ);
        if (file_backlog) {
          file_backlog.seek(unsent_pointer_count);
          
          // Read record into gprs_payload without readStringUntil
          int i = 0;
          while(file_backlog.available() && i < (int)sizeof(gprs_payload) - 1) {
             char c = file_backlog.read();
             if (c == '\r') continue;
             if (c == '\n') break;
             gprs_payload[i++] = c;
          }
          gprs_payload[i] = '\0';
          
          unsent_pointer_count =
              file_backlog.position(); // Capture next pointer location
          file_backlog.close();
        } else {
          xSemaphoreGive(fsMutex);
          debugln("[Backlog] Failed to reopen file. Stopping loop.");
          break;
        }
        xSemaphoreGive(fsMutex); // Unlock BEFORE long HTTP handshake

        debugf("[Backlog] Processing Record #%d of %d (Pointer: %d)\n",
               backlog_processed_count + 1, total_unsent,
               (int)unsent_pointer_count);

        vTaskDelay(100 / portTICK_PERIOD_MS); // iter10

        if (strlen(gprs_payload) < 10) {
          debugln("Skipping blank/invalid line in unsent backlog.");
          // v5.82 Platinum: Advance pointer even for skipped corrupt/blank
          // lines
          if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
            File fTmp = SPIFFS.open("/unsent_ptr.tmp", FILE_WRITE);
            if (fTmp) {
              fTmp.print(unsent_pointer_count);
              fTmp.close();
              SPIFFS.remove("/unsent_pointer.txt");
              vTaskDelay(20 / portTICK_PERIOD_MS); // metadata breath
              SPIFFS.rename("/unsent_ptr.tmp", "/unsent_pointer.txt");
            }
            xSemaphoreGive(fsMutex);
          }
          continue; // Does NOT increment backlog_processed_count (v5.74 Fix
                    // #23)
        }

        backlog_processed_count++; // v5.74 Fix #23: Only count real HTTP
                                   // attempts toward the 15-cap

        // Set the data mode
        data_mode = eUnsentData;
        prepare_data_and_send();

        // v5.82 LTS: Atomic update MUST happen after the send attempt.
        // If power cuts during handshake, next boot retries. If succeeds, flash
        // advances.
        if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
          File fTmp = SPIFFS.open("/unsent_ptr.tmp", FILE_WRITE);
          if (fTmp) {
            fTmp.print(unsent_pointer_count);
            fTmp.close();
            SPIFFS.remove("/unsent_pointer.txt");
            vTaskDelay(20 / portTICK_PERIOD_MS); // metadata breath
            SPIFFS.rename("/unsent_ptr.tmp", "/unsent_pointer.txt");
          }
          xSemaphoreGive(fsMutex);
        }
        // v5.65 FINAL PRODUCTION RULES:
        // 1. Fail-Fast: If a record fails (success_count == 0), STOP
        // immediately to save battery.
        // 2. Power-Cap: Limit to 15 records per 15-min wakeup to prevent
        // overheating/drain.
        // 3. Tower-Breather: Mandatory 3s delay between lines to let Airtel
        // clear the session.

        if (success_count == 0 || (backlog_processed_count >= 15) ||
            (pending_manual_status || pending_manual_gps ||
             pending_manual_health)) {
          if (success_count == 0)
            debugln("[Power] Backlog line FAILED. Stopping.");
          else if (backlog_processed_count >= 15)
            debugln("[Power] Backlog limit (15) reached.");
          else
            debugln("[GPRS] Manual interrupt during backlog.");
          break;
        }

        // NOTE: The old unsent_counter==6 safety valve has been removed (BUG-C3
        // fix v5.65). unsent_counter was never incremented in this loop, making
        // that check dead code. Active guards: (1) Fail-Fast break on
        // success_count==0 above,
        //                (2) 15-record power cap on backlog_processed_count
        //                above.

        // Tower breather: only needed after a real HTTP attempt, not for
        // skipped records
        if (success_count != 2) { // 2 = skip sentinel (P2 fix v5.65)
          vTaskDelay((isLTE ? 300 : 3000) /
                     portTICK_PERIOD_MS); // v5.85: P11 Carrier-aware breather
        }

      } // while loop

      // v5.75 FIX: [H-04] Stale FileSize Bug
      // Dynamically re-read the true physical size of the file. The
      // schedulerTask may have appended a new meteorological record to this
      // file while the 4-minute drain loop was actively running!
      if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
        File f_sz = SPIFFS.open(unsent_file, FILE_READ);
        if (f_sz) {
          backlogFileSize = f_sz.size();
          f_sz.close();
        }

        if (unsent_pointer_count >= backlogFileSize) {
          debugln("Unsent data sent .. Going to remove the unsent file and "
                  "pointer ...");
          SPIFFS.remove(unsent_file);
          SPIFFS.remove("/unsent_pointer.txt");
          
          // v5.83 Ghost Backlog Fix:
          // 1. Reset pointer in memory immediately to prevent re-writing stale index
          unsent_pointer_count = 0;
          // 2. Allow SPIFFS index 1s to finalize changes before Deep Sleep cuts power
          debugln("[FS] Waiting 1s for Storage Commit...");
          vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        xSemaphoreGive(fsMutex);
      }
    } else {
      debugln("No unsent file found ...");
    }
#endif

    // v5.51: send_unsent_data() is called ONCE via the Independent FTP Trigger
    // below (line 1580+). Do NOT call it here again inside the HTTP-success
    // block to avoid double-upload and wasted power.

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

  xSemaphoreGive(
      modemMutex); // v5.55: Release modem early to allow sub-calls to take it

#if (SYSTEM == 1 || SYSTEM == 2)
  // v5.49 Build 5: INDEPENDENT FTP TRIGGER
  // Decoupled from HTTP Success. FTP serves as the robust rescue layer.
  if (gprs_mode == eGprsSignalOk && (signal_lvl > -96)) {
    // v5.68 FIX: UART Race Condition Prevented. If the user queued a manual
    // LCD command during the HTTP HTTP-Fill, skip the massive FTP job
    // so the Modem Mutex is immediately available for the UI.
    if (!pending_manual_status && !pending_manual_gps &&
        !pending_manual_health) {
      send_unsent_data();
    } else {
      debugln("[GPRS] Yielding FTP transfer to instantly service manual LCD/UI "
              "request.");
    }
  }
#endif

  // v5.65 Selective SMS Check: Guaranteed once an hour (at Minute 0 slot)
  if (record_min == 0) {
    portENTER_CRITICAL(&syncMux);
    sync_mode = eSMSStart;
    portEXIT_CRITICAL(&syncMux);
    send_sms(); // This function will internally take modemMutex
  } else {
    debugln("[GPRS] Skipping SMS check (hourly task).");
    portENTER_CRITICAL(&syncMux);
    sync_mode = eHttpStop;
    portEXIT_CRITICAL(&syncMux);
  }
} // end of send_http_data

void send_unsent_data() { // ONLY FOR TWS AND TWS-ADDON
  // v7.70+: Decoupled lock. Function now takes locks locally for I/O only.
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

  int snap_hr, snap_mi;
  portENTER_CRITICAL(&rtcTimeMux);
  snap_hr = current_hour;
  snap_mi = current_min;
  portEXIT_CRITICAL(&rtcTimeMux);

#if SYSTEM == 1
  if (strstr(UNIT, "SPATIKA") || strstr(NETWORK, "SPATIKA"))
    snprintf(fileName, sizeof(fileName), "/TWS_%s_%02d%02d%02d_%02d%02d00.swd",
             stnId, ftp_year, rf_cls_mm, rf_cls_dd, snap_hr, snap_mi);
  else
    snprintf(fileName, sizeof(fileName), "/TWS_%s_%02d%02d%02d_%02d%02d00.kwd",
             stnId, ftp_year, rf_cls_mm, rf_cls_dd, snap_hr, snap_mi);
#endif

#if SYSTEM == 2
  if (strstr(UNIT, "SPATIKA") || strstr(NETWORK, "SPATIKA"))
    snprintf(fileName, sizeof(fileName),
             "/TWSRF_%s_%02d%02d%02d_%02d%02d00.swd", stnId, ftp_year,
             rf_cls_mm, rf_cls_dd, snap_hr, snap_mi);
  else
    snprintf(fileName, sizeof(fileName),
             "/TWSRF_%s_%02d%02d%02d_%02d%02d00.kwd", stnId, ftp_year,
             rf_cls_mm, rf_cls_dd, snap_hr, snap_mi);
#endif

  // v5.56: BACKLOG RECOVERY ON EVERY CYCLE
  // Removed 'primary_data_delivered' gate and hourly restriction per user
  // request. Backlog will be attempted whenever signal is usable (-95 dBm or
  // better).
  // v5.51: Power-Saving FTP Gating logic
  snprintf(ftpunsent_file, sizeof(ftpunsent_file), "/ftpunsent.txt");
  int unsent_cnt = 0;

  // v5.85: M-02 Hardening - Wrap orphan recovery and countStored in fsMutex
  if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(3000)) == pdTRUE) {
    // v5.52 RELIABILITY: Recover orphaned remainders from failed previous
    // renames
    if (!SPIFFS.exists(ftpunsent_file) && SPIFFS.exists("/ftpremain.txt")) {
      debugln(
          "[Healer] Orphaned ftpremain.txt found. Recovering to ftpunsent.txt");
      SPIFFS.rename("/ftpremain.txt", "/ftpunsent.txt");
    }
    unsent_cnt = countStored(ftpunsent_file);
    xSemaphoreGive(fsMutex);
  } else {
    // If mutex take fails, we must still respect the cached count to avoid
    // infinite loop
    debugln("[Healer] fsMutex timeout in orphan recovery. Skipping for this "
            "cycle.");
  }

  // v5.52 FIX: Ensure signal_lvl isn't 0 if polling failed (prevents false
  // triggers/0000 in logs)
  if (signal_lvl == 0)
    signal_lvl = SIGNAL_STRENGTH_MISSING_DATA;

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
  // v5.74: Atomic Snapshot to prevent torn reads at hour boundary (R-01 fix)
  // (Snapshot now taken at the top of the function for fileName generation)

  bool scheduled_slot = (snap_mi <= 15) && (snap_hr % 3 == 0);
  bool morning_cleanup = (snap_hr == 8 && snap_mi >= 45 && snap_mi < 60);
  debugf5("[FTP-Gate] unsent=%d cur_time=%02d:%02d sched=%s cleanup=%s\n",
          unsent_cnt, snap_hr, snap_mi, scheduled_slot ? "YES" : "NO",
          morning_cleanup ? "YES" : "NO");
  bool should_push =
      (unsent_cnt > 2) || (unsent_cnt > 0 && morning_cleanup); // v5.81: Daily Drain phase phase phase phase phase phase phase phase (Summary truncated)

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
        debugf("[FTP] Skip Backlog: Signal too weak (%d dBm).\n", signal_lvl);
      } else {
        bool fs_locked_unsent = false;
        // v13.4: Skip full re-registration if HTTP just succeeded (bearer is
        // fresh)
        bool bearer_fresh = (millis() - last_activity_time) < 90000;
        if (!bearer_fresh) {
          debugln("[FTP] Bearer stale. Re-registering...");
          get_registration();
        } else {
          debugln(
              "[FTP] Bearer fresh (HTTP<90s ago). Skipping re-registration.");
        }
        if (bearer_fresh || gprs_mode == eGprsSignalOk) {
          if (!bearer_fresh) {
            debugln("[FTP] Registration OK. Verifying IP (PDP Context)...");
            get_a7672s(); // Only needed if we re-registered
          }
          if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(10000)) == pdTRUE) {
            fs_locked_unsent = true;
          }

          if (!fs_locked_unsent) {
            debugln(
                "[FTP] Mutex Timeout: Cannot proceed with SPIFFS chunking.");
            return;
          }

          // v6.45: Brief settle before FTP to avoid socket conflicts
          debugln(
              bearer_fresh
                  ? "[FTP] Bearer fresh. Brief 5s settle before FTP..."
                  : "[FTP] Assigned IP. Waiting 5s for tower stabilization...");
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

            // v5.65 Fix: Guard against file-creation failure (SPIFFS Full or
            // busy) If the chunk file fails to open, we MUST ABORT now.
            // Proceeding with an empty chunk file and then deleting the source
            // queue leads to permanent data loss.
            if (!chunk) {
              debugln(
                  "[FTP] CRITICAL: Failed to create chunk file. Aborting FTP.");
              src.close();
              if (remainder)
                remainder.close();
              xSemaphoreGive(fsMutex);
              return;
            }

            int linesRead = 0;
            char lineBuf[128];
            while (src.available()) {
              esp_task_wdt_reset();
              size_t len =
                  src.readBytesUntil('\n', lineBuf, sizeof(lineBuf) - 1);
              if (len == 0 && !src.available())
                break;
              lineBuf[len] = '\0';

              // [FTP-M02] Defensive Hardening: Verify record length to avoid
              // corruption propagation
              int expected_len =
                  record_length - 2; // Content length excluding \r\n
              if (len > 0 && lineBuf[len - 1] == '\r') {
                lineBuf[--len] = '\0';
              }

              if (len == expected_len) {
                if (linesRead < FTP_CHUNK_SIZE) {
                  chunk.write((uint8_t *)lineBuf, len);
                  chunk.print("\r\n");
                } else {
                  if (remainder) {
                    remainder.write((uint8_t *)lineBuf, len);
                    remainder.print("\r\n");
                  }
                }
                linesRead++;
              } else if (len > 0) {
                debugf("[FTP] Dropping malformed record: len=%d, expected=%d\n",
                       len, expected_len);
              }
            }
            if (chunk)
              chunk.close();
            if (remainder)
              remainder.close();
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
            if (fs_locked_unsent) {
              xSemaphoreGive(fsMutex);
              fs_locked_unsent = false;
            }
            return; // Hygiene Fix: Abort on source failure to prevent wasted
                    // blank FTP sequence
          }
          debug("Retrieved file is ");
          debugln(fileName);
          esp_task_wdt_reset();
          // v5.66: Release FS mutex before long FTP upload to allow scheduler
          // to record
          if (fs_locked_unsent) {
            xSemaphoreGive(fsMutex);
            fs_locked_unsent = false;
          }
          send_ftp_file(fileName, false, false); // Pass alreadyLocked=false
          // Note: send_ftp_file will take its own lock for cleanup
        } else {
          debugln("[FTP] Skip: Registration lost. Retrying next hour.");
          if (fs_locked_unsent) {
            xSemaphoreGive(fsMutex);
            fs_locked_unsent = false;
          }
        }
      } // end else (signal OK)
    } else {
      debugln("No ftpunsent.txt found. Skipping FTP.");
    }

    // v5.68 BUGFIX: fsMutex was released at 2121 to allow background FTP.
    // We MUST legally retake it before performing SPIFFS file modifications for
    // Daily FTP!
    if (sampleNo == 3) { // v5.65 P2: Fixed cleanup condition — only run at
                         // 09:30 AM (sampleNo 3) to avoid accidental backlog
                         // clears if sampleNo 7 is reached via reboot.
      if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(10000)) == pdTRUE) {
        // v5.75 FIX: [M-02] 09:00 Time-Tear Race Condition
        // Snap the volatile time integers atomically to prevent a context
        // switch exactly at 08:59:59 from producing a phantom 09:59 evaluation
        // and destroying the backlog an hour early.
        int snap_hr, snap_mi;
        portENTER_CRITICAL(&rtcTimeMux);
        snap_hr = current_hour;
        snap_mi = current_min;
        portEXIT_CRITICAL(&rtcTimeMux);

        if (snap_hr == 9 && snap_mi > 30 && snap_mi < 45) {
          // v5.72: Guarded Cleanup - Only wipe if the backlog is already empty.
          // Prevents silent deletion of undrained records from overnight
          // outages.
          snprintf(ftpunsent_file, sizeof(ftpunsent_file), "/ftpunsent.txt");
          int remaining = countStored(ftpunsent_file);
          if (remaining == 0) {
            SPIFFS.remove(ftpunsent_file);
            debugln("[FTP] Daily transition: Backlog already empty, cleanup "
                    "SUCCESS.");
          } else {
            debugf("[FTP] Daily transition: %d undrained records in backlog - "
                   "PRESERVING for drain.\n",
                   remaining);
          }
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
          snprintf(fileName, sizeof(fileName),
                   "/TWS_%s_%02d%02d%02d_%02d%02d00.swd", stnId, ftp_year,
                   rf_cls_mm, rf_cls_dd, record_hr, record_min);
        else
          snprintf(fileName, sizeof(fileName),
                   "/TWS_%s_%02d%02d%02d_%02d%02d00.kwd", stnId, ftp_year,
                   rf_cls_mm, rf_cls_dd, record_hr, record_min);
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

        // v5.75: Atomic open under lock (R-02 fix)
        File f_src = SPIFFS.open(temp_file, FILE_READ);
        if (f_src) {
          f_src.close(); // Just verifying existence under lock
          copyFile(temp_file, fileName, true); // v5.75: alreadyLocked=true
          debug("Retrieved file is ");
          debugln(fileName);
          esp_task_wdt_reset();
          xSemaphoreGive(fsMutex);
          send_ftp_file(fileName, true, false);
        } else {
          debugln("Daily FTP: Temp file not found. Skipping.");
          xSemaphoreGive(fsMutex);
        }
      } else {
        debugln(
            "[FTP] Failed to retake fsMutex for Daily operation. Skipping.");
      }
    }
  }
} // end send_unsent_data

int send_at_cmd_data(char *payload, bool robust) {
  uint32_t start_time = millis();
  strcpy(diag_http_fail_reason, "NONE"); // v5.81 Surgical: Clear stale context
  if (!http_ready) {
    debugln("[HTTP] HTTP session not ready. Fast-fail to backlog.");
    return 0;
  }
  int i = strlen(payload);
  char cmd_buf[80];

  debugf1("Payload is %s", payload);
  debugln();

  if (robust) {
    // Robust mode for weak-signal or strict towers (BSNL, etc.)
    // v5.86: Purge UART before data request to ensure DOWNLOAD prompt is caught correctly
    flushSerialSIT();
    vTaskDelay(500 / portTICK_PERIOD_MS);
    
    snprintf(cmd_buf, sizeof(cmd_buf), "AT+HTTPDATA=%d,15000", i);
    debugln("[HTTP] Using Robust Handshake (Wait for DOWNLOAD)...");
    SerialSIT.println(cmd_buf);
    if (!waitForResponse("DOWNLOAD", 15000)) {
      debugln("[HTTP] AT+HTTPDATA failed (Missing DOWNLOAD).");
      flushSerialSIT();
      return 0;
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
    
    // v5.88: Hardened Chunked Write (Sync with Health logic)
    int payloadLen = i;
    int sentBytes = 0;
    while (sentBytes < payloadLen) {
      int toWrite = min(32, payloadLen - sentBytes);
      SerialSIT.write(payload + sentBytes, toWrite);
      sentBytes += toWrite;
      esp_task_wdt_reset();
      vTaskDelay(50 / portTICK_PERIOD_MS); // v5.99: Increased breather
    }
    // SerialSIT.println(); // REMOVED: Do not send trailing newline for fixed-length HTTPDATA

    if (!waitForResponse("OK", 15000)) {
       debugln("[HTTP] AT+HTTPDATA confirmation timeout. Nuking PDP...");
       SerialSIT.println("AT+HTTPTERM");
       waitForResponse("OK", 2000);
       SerialSIT.println("AT+CGACT=0,1");
       waitForResponse("OK", 5000);
       return 0;
    }
  } else {
    // v5.63 Native v3.0 Fast push!
    // We send command, wait for prompt byte, then push payload.
    // This mimics v3.0's behavior while keeping the UART clean.
    // v5.67 (Claude's logic fix): Opened up latency window. If DOWNLOAD drops
    // late, we need enough of the 3000ms window remaining to clock the payload
    // JSON.
    snprintf(cmd_buf, sizeof(cmd_buf), "AT+HTTPDATA=%d,10000", i);
    debugln("[HTTP] Using Fast v3.0 Handshake...");
    SerialSIT.println(cmd_buf);
    if (waitForResponse("DOWNLOAD", 5000)) {
      vTaskDelay(200 / portTICK_PERIOD_MS);
      SerialSIT.print(payload); // v5.99: Use print() to avoid trailing \r\n desync
      waitForResponse("OK", 5000);
    } else {
      debugln("[HTTP] Fast Handshake DOWNLOAD Timeout.");
      return 0;
    }
  }

  // Fire Action
  SerialSIT.println("AT+HTTPACTION=1");
  if (!waitForResponse("+HTTPACTION:", 25000)) {
     strncpy(diag_http_fail_reason, "TIMEOUT", sizeof(diag_http_fail_reason)-1);
     debugln("[HTTP] HTTPACTION timed out — no URC received from modem.");
     return 0;
  }

  const char* response = strstr(modem_response_buf, "+HTTPACTION:");
  if (response == NULL) {
     debugln("[HTTP] HTTPACTION missing from modem buffer. Aborting.");
     return 0;
  }
  debugf("[HTTP] Response of AT+HTTPACTION=1 is: %s\n", response);

  if (strstr(response, "200") == NULL && strstr(response, "201") == NULL &&
      strstr(response, "202") == NULL) {
    // v5.45: Extract error code from +HTTPACTION: prefix ONLY.
    // Old method searched the whole buffer from comma1[INFO]comma2, which picked
    // up commas inside +CGEV: ME PDN ACT 8,0 URCs that rode in on the same
    // buffer, producing corrupt strings like "0\n\n+CGEV: ME PDN ACT 8" as
    // the code.
    const char* ha_ptr = strstr(response, "+HTTPACTION:");
    if (ha_ptr != NULL) {
      const char* c1 = strchr(ha_ptr, ',');
      const char* c2 = (c1 != NULL) ? strchr(c1 + 1, ',') : NULL;
      if (c1 != NULL && c2 != NULL) {
         int len = c2 - (c1 + 1);
         if (len > 0 && len < (int)sizeof(diag_http_fail_reason)) {
            strncpy(diag_http_fail_reason, c1 + 1, len);
            diag_http_fail_reason[len] = '\0';
         }
      }
    }

    if (strstr(response, "706") != NULL || strstr(response, "713") != NULL ||
        strstr(response, "714") != NULL || strstr(response, "601") != NULL) {
      
      diag_http_zombie_count++;
      debugf("HTTP Zombie Error (%s). Count: %d/3. Clean stack requested.\n", diag_http_fail_reason, diag_http_zombie_count);
      
      SerialSIT.println("AT+HTTPTERM");
      waitForResponse("OK", 2000);

      // v5.82 Surgical Hardening (Phase 4):
      // On legacy boards, a 706 TCP Zombie often requires an immediate stack reset
      // rather than waiting for 3 fails.
      debugln("[CRIT] TCP Zombie detected. Nuking bearer for fresh IP...");
      SerialSIT.println("AT+CIPSHUT");
      waitForResponse("SHUT OK", 3000);
      SerialSIT.println("AT+CGACT=0,1");
      waitForResponse("OK", 1000);
      http_ready = false; // Housekeeping: State follows destroyed stack
      vTaskDelay(2000 / portTICK_PERIOD_MS); 

      if (diag_http_zombie_count >= 3) {
         debugln("[CRIT] Persistent Zombie detected. Triggering Radio Refresh...");
         diag_http_zombie_count = 0;
         verify_bearer_or_recover(); // Triggers radio refresh internally if APN fails
      }
    } else {
      diag_http_zombie_count = 0; // Reset on other errors
    }
    // ISSUE-M3 fix v5.65: Do NOT increment diag_daily_http_fails here.
    // prepare_data_and_send() (the caller) increments it at line ~1155.
    // Incrementing here too caused every failure to be counted twice,
    // showing 2x the real HTTP fail count on the health report dashboard.
    return 0;
  }

  flushSerialSIT();
  SerialSIT.println("AT+HTTPHEAD");
  waitForResponse("OK", 5000);

  SerialSIT.println("AT+HTTPREAD=0,512");
  if (!waitForResponse("+HTTPREAD: 0", 10000)) {
    debugln("[GPRS] Param-READ failed. Retrying with breather...");
    vTaskDelay(200 / portTICK_PERIOD_MS); 
    SerialSIT.println("AT+HTTPREAD");
    if (!waitForResponse("+HTTPREAD: 0", 10000)) {
       debugln("[GPRS] Final RAW READ failed.");
    }
  }
  
  const char* final_resp = modem_response_buf;
  debugf("[HTTP] Final Response Body snippet: %s\n", final_resp);

  // v6.75: Advanced Response Parsing
  // SerialSIT says: OK\r\n\r\n+HTTPREAD: <size>\r\n<PAYLOAD>\r\nOK
  // We must find the payload start after '+HTTPREAD:'
  const char *payload_ptr = strstr(modem_response_buf, "+HTTPREAD:");
  if (payload_ptr) {
    // Find the end of (+HTTPREAD: <size>) line
    payload_ptr = strchr(payload_ptr, '\n');
    if (payload_ptr)
      payload_ptr++; // Move to start of actual content
  } else {
    // If +HTTPREAD marker not found (e.g. raw read), check the whole response
    // but avoid the initial 'OK' if possible.
    payload_ptr = modem_response_buf;
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

    // v5.85: P9 - Proactive DNS Caching (Only on healthy known networks)
    bool is_domain = false;
    const char *srv = httpSet[http_no].serverName;
    for (int i = 0; srv[i] != '\0'; i++) {
      if (isalpha(srv[i])) {
        is_domain = true;
        break;
      }
    }

    if (!dns_fallback_active && is_domain) {
      // Successful domain resolution! Cache the IP for next slot.
      const char *domain = httpSet[http_no].serverName;
      const char *ip = httpSet[http_no].IP;
      if (domain && ip && strlen(ip) > 5) {
        strncpy(cached_server_domain, domain, sizeof(cached_server_domain) - 1);
        strncpy(cached_server_ip, ip, sizeof(cached_server_ip) - 1);

        // v5.75 FIX: [H-01] Disable Proactive IP Routing
        // HTTP Virtual Hosts (SNI) reject raw IP requests with 706 TCP Zombie.
        // We keep the cache variables updated for emergency fallback but NEVER
        // voluntarily activate the raw IP route on a healthy system.
        // dns_fallback_active = true;

        debugf("[DNS] Proactive cache updated (Raw IP Fallback DISABLED): %s "
               "-> %s\n",
               domain, ip);
      }
    }

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
          diag_http_success_count++; // v5.65 P4: Track successful slots per day
          if (data_mode == eUnsentData)
            diag_http_retry_count++; // v5.65 P4: Track live recovery
        } else {
          diag_sent_mask_prev[temp_sampleNo / 32] |= bit;
          diag_http_success_count_prev++; // v5.65 P4: FIXED Yesterday counter
                                          // drift
          if (data_mode == eUnsentData)
            diag_http_retry_count_prev++;
        }
      }
    }

    // v5.55: Use RTC_DATA_ATTR guard so SPIFFS write survives deep sleep.
    // 'static' local resets on every wakeup, causing a write every boot.
    if (!apn_saved_this_sim && cached_iccid[0] != '\0' && apn_str[0] != '\0') {
      save_apn_config(apn_str, cached_iccid);
      apn_saved_this_sim = true;
    }

    diag_http_time_total += (millis() - start_time);
    strcpy(diag_http_fail_reason, "NONE");

    // v7.70: Ensure clean session close after success to prevent crosstalk
    // v5.66: CRITICAL BACKLOG FIX - Do NOT destroy the HTTP session
    // mid-backlog! Keeping it open allows records 2-15 to flow instantly
    // without rebuild penalties.
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

      // Attempt 1: Try payload-based sync (if server included tm field)
      sync_rtc_from_server_tm(payload_ptr, true);

      // Attempt 2: Use pre-captured headers (IST sync)
      if (diag_rejected_count >= 2) {
        sync_rtc_from_http_header(); // v5.75: IST sync using captured head

        if (diag_rejected_count > 2) {
          debugln("[TIME] Persistent rejection even after Header Sync. Falling "
                  "back to CLBS...");
          resync_time();
          diag_rejected_count = 0;
        }
      }
    } else {
      // v5.75 Hardening [H-01]: Reset rejection count on non-rejection failures
      diag_rejected_count = 0;
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
  // v5.75 FIX: [H-02] Torn sampleNo Dedup Guard
  // Calculate atomic snapshot to prevent slot-transition tearing during double
  // evaluation.
  int snap_sampleNo;
  portENTER_CRITICAL(&syncMux);
  snap_sampleNo = sampleNo;
  portEXIT_CRITICAL(&syncMux);

  char finalStringBuffer[160] = {0};
  char last_rec_file[50];
  snprintf(last_rec_file, sizeof(last_rec_file), "/lastrecorded_%s.txt",
           station_name);

  // v5.72 Hardened: Mutex Pulse Pattern (M-1)
  // Take lock briefly only to capture the source data
  if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
    File f = SPIFFS.open(last_rec_file, FILE_READ);
    if (f) {
      size_t readLen =
          f.readBytes(finalStringBuffer, sizeof(finalStringBuffer) - 1);
      finalStringBuffer[readLen] = '\0';
      f.close();
    }
    xSemaphoreGive(fsMutex);
  } else {
    debugln("[SPIFFS] Mutex Timeout on read. Aborting store.");
    return;
  }

  // Memory logic: derivation using pointers
  char cleanFtp[160] = {0};
  char cleanData[160] = {0};

#if SYSTEM == 0
  strncpy(cleanData, finalStringBuffer, sizeof(cleanData)-1);
  trim_whitespace(cleanData);
  if (strlen(cleanData) < 20) {
    strncpy(cleanData, append_text, sizeof(cleanData)-1);
    trim_whitespace(cleanData);
  }
#else
  if (strlen(finalStringBuffer) > 20) {
    char csv[160];
    strncpy(csv, finalStringBuffer, sizeof(csv)-1);
    csv[sizeof(csv)-1] = '\0';
    trim_whitespace(csv);
    
    char *firstComma = strchr(csv, ',');
    if (firstComma != NULL) {
      char derived_stn[20];
      if (strlen(ftp_station) == 4 && isDigitStr(ftp_station)) {
         snprintf(derived_stn, sizeof(derived_stn), "00%s", ftp_station);
      } else {
         strncpy(derived_stn, ftp_station, sizeof(derived_stn)-1);
      }
      
      char *rest = firstComma + 1;
      char *dtComma = strchr(rest, ',');
      if (dtComma != NULL) {
         char *fieldStartPtr = strchr(dtComma + 1, ',');
         if (fieldStartPtr != NULL) {
            fieldStartPtr++; // Move past comma
            char dtPart[40] = {0};
            int dtLen = (fieldStartPtr - 1) - rest;
            if (dtLen > 0 && dtLen < (int)sizeof(dtPart)) {
               strncpy(dtPart, rest, dtLen);
               dtPart[dtLen] = '\0';
            }
            
            char *fields = fieldStartPtr;
#if SYSTEM == 2
            char *c1 = strchr(fields, ',');
            if (c1 != NULL) {
               char cumRfStr[16] = {0};
               int crfLen = c1 - fields;
               strncpy(cumRfStr, fields, (crfLen < 15) ? crfLen : 15);
               float cumRfVal = atof(cumRfStr);
               char paddedCumRf[10];
               snprintf(paddedCumRf, sizeof(paddedCumRf), "%05.2f", cumRfVal);
               
               char remaining[100];
               strncpy(remaining, c1 + 1, sizeof(remaining)-1);
               remaining[sizeof(remaining)-1] = '\0';
               // Replace commas with semicolons in remaining
               for(int j=0; remaining[j]; j++) if(remaining[j] == ',') remaining[j] = ';';
               
               snprintf(cleanFtp, sizeof(cleanFtp), "%s;%s;%s;%s", derived_stn, dtPart, paddedCumRf, remaining);
            }
#else
            char fields_buf[120];
            strncpy(fields_buf, fields, sizeof(fields_buf)-1);
            fields_buf[sizeof(fields_buf)-1] = '\0';
            for(int j=0; fields_buf[j]; j++) if(fields_buf[j] == ',') fields_buf[j] = ';';
            snprintf(cleanFtp, sizeof(cleanFtp), "%s;%s;%s", derived_stn, dtPart, fields_buf);
#endif
         }
      }
    }
  }
  
  if (strlen(cleanFtp) < 30) {
    strncpy(cleanFtp, ftpappend_text, sizeof(cleanFtp)-1);
    trim_whitespace(cleanFtp);
  }
#endif

#if SYSTEM == 0
  snprintf(unsent_file, sizeof(unsent_file), "/unsent.txt");
  if (last_unsent_sampleNo != snap_sampleNo) { // v5.75: Atomic Dedup
    // v5.93: Onside Engineer Priority Backoff
    if (local_svc_upload_active) {
       debugln("[GPRS] Onsite Engineer Active. Yielding fsMutex for unsent data...");
       vTaskDelay(3000 / portTICK_PERIOD_MS); 
    }
    
    if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
      pruneFile(unsent_file, (300 * record_length), true);
      File file2 = SPIFFS.open(unsent_file, FILE_APPEND);
      if (file2) {
        if (strlen(cleanData) > 0) {
          file2.print(cleanData); // v5.75: Explicitly print (Fixed H-06)
          file2.print("\r\n");
          if (diag_backlog_total < 999999)
            diag_backlog_total++; // v5.65 P3: Increment total backlog counter
                                  // for health report
        }
        file2.close();
        last_unsent_sampleNo = snap_sampleNo;
      }
      xSemaphoreGive(fsMutex);
    }
  }
#endif

#if (SYSTEM == 1 || SYSTEM == 2)
  snprintf(ftpunsent_file, sizeof(ftpunsent_file), "/ftpunsent.txt");
  if (last_unsent_sampleNo != snap_sampleNo) { // v5.75: Atomic Dedup
    if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
      pruneFile(ftpunsent_file, (300 * record_length), true);
      File ftpfile2 = SPIFFS.open(ftpunsent_file, FILE_APPEND);
      if (ftpfile2) {
        if (strlen(cleanFtp) > 0) {
          ftpfile2.print(cleanFtp); // v5.75: Explicitly print (Fixed H-06)
          ftpfile2.print("\r\n");
          if (diag_backlog_total < 999999)
            diag_backlog_total++;
        }
        ftpfile2.close();
        last_unsent_sampleNo = snap_sampleNo;
      }
      xSemaphoreGive(fsMutex);
    }
  }
#endif

  // v5.50: Update HTTP fail counters
  if (current_month > 0 && diag_cum_fail_reset_month != current_month) {
    diag_http_cum_fails = 0;
    diag_cum_fail_reset_month = current_month;
    debugln("[STORE] Monthly cum fail counter reset (New Month detected).");
  }
  diag_http_present_fails++;
  diag_http_cum_fails++;
  snprintf(ui_data[FLD_HTTP_FAILS].bottomRow,
           sizeof(ui_data[FLD_HTTP_FAILS].bottomRow), "P:%d C:%d B:%d",
           diag_http_present_fails, diag_http_cum_fails,
           get_total_backlogs(false));

#if DEBUG == 1
  // v5.72 Core Hardening: Protect debug reads with Mutex Pulse
  if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(2000)) == pdTRUE) {
#if SYSTEM == 0
    if (SPIFFS.exists("/unsent.txt")) {
      File file4 = SPIFFS.open("/unsent.txt", FILE_READ);
      if (file4) {
        if (file4.size() > 0) {
          int seekPos = (file4.size() > 500) ? file4.size() - 500 : 0;
          file4.seek(seekPos);
          char tail_buf[512];
          int r = file4.readBytes(tail_buf, sizeof(tail_buf)-1);
          tail_buf[r] = '\0';
          debugln("   ... [Tail Unsent] ...");
          debug(tail_buf);
        }
        file4.close();
      }
    }
#endif
#if (SYSTEM == 1 || SYSTEM == 2)
    if (SPIFFS.exists("/ftpunsent.txt")) {
      File ftpfile4 = SPIFFS.open("/ftpunsent.txt", FILE_READ);
      if (ftpfile4) {
        if (ftpfile4.size() > 0) {
          int seekPos = (ftpfile4.size() > 500) ? ftpfile4.size() - 500 : 0;
          ftpfile4.seek(seekPos);
          char tail_buf[512];
          int r = ftpfile4.readBytes(tail_buf, sizeof(tail_buf)-1);
          tail_buf[r] = '\0';
          debugln("   ... [Tail FTP Unsent] ...");
          debug(tail_buf);
        }
        ftpfile4.close();
      }
    }
#endif
    xSemaphoreGive(fsMutex);
  }
#endif

  portENTER_CRITICAL(&syncMux);
  sync_mode = eHttpStop;
  portEXIT_CRITICAL(&syncMux);
  vTaskDelay(300 / portTICK_PERIOD_MS);
}

/*
 *  SIM Network Registration and Setup
 *  - get_signal_strength();//2024 iter3
 *  - get_network();
 *  - get_registration();
 *  - get_a7672s();
 */
// v5.87: Service Report Background Sync Task
void cleanup_service_report(bool fatal) {
    if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
        if (fatal) debugln("[SVC] Scrubbing FATAL failure data from flash...");
        else debugln("[SVC] Cleaning up successful report data...");
        
        SPIFFS.remove("/svc_pending.json");
        SPIFFS.remove("/svc_img1.jpg");
        SPIFFS.remove("/svc_img2.jpg");
        
        // Legacy cleanup (v5.91 Cleanup)
        SPIFFS.remove("/svc_img1.j64");
        SPIFFS.remove("/svc_img2.j64");
        
        svc_retry_count = 0;
        xSemaphoreGive(fsMutex);
    }
}

bool send_service_report() {
  if (!SPIFFS.exists("/svc_pending.json")) return false;
  
  if (local_svc_upload_active) {
     debugln("[SVC] Onsite Engineer Active. Yielding...");
     return false;
  }
  
  svc_sync_status = 2; // SYNC_META
  debugln("[SVC] Found report. Starting Hardened Split-Sync (v6.8)...");
  
  size_t img1Len = 0, img2Len = 0;
  long timestamp = 0;

  // v6.8: Extract TS without reading entire file into a String
  if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
    File fM = SPIFFS.open("/svc_pending.json", FILE_READ);
    if (fM) {
      // Look for "ts": using a sliding buffer or just reading enough
      String head = fM.readStringUntil('}'); // Usually TS is near the top
      int tsIdx = head.indexOf("\"ts\":");
      if(tsIdx > 0) {
         timestamp = head.substring(tsIdx+5).toInt();
         debugf("[SVC] Sync Anchor TS: %ld\n", timestamp);
      }
      fM.close();
    }
    if (SPIFFS.exists("/svc_img1.jpg")) {
      File fI1 = SPIFFS.open("/svc_img1.jpg", FILE_READ);
      if (fI1) { img1Len = fI1.size(); fI1.close(); }
    }
    if (SPIFFS.exists("/svc_img2.jpg")) {
      File fI2 = SPIFFS.open("/svc_img2.jpg", FILE_READ);
      if (fI2) { img2Len = fI2.size(); fI2.close(); }
    }
    xSemaphoreGive(fsMutex);
  }

  const char* bnd = "----SvcBnd";
  const char* dash = "--";
  const char* crlf = "\r\n";
  
  // Base Parts
  String stnPart = String(dash) + bnd + crlf + "Content-Disposition: form-data; name=\"stn\"" + crlf + crlf + String(station_name) + crlf;
  String metaHeader = String(dash) + bnd + crlf + "Content-Disposition: form-data; name=\"json\"" + crlf + "Content-Type: application/json" + crlf + crlf;
  String footer = String(dash) + bnd + dash + crlf;

  // --- STAGE 1: METADATA ---
  bool stage1Ok = false;
  size_t metaSize = 0;
  if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(2000)) == pdTRUE) {
      File f = SPIFFS.open("/svc_pending.json", FILE_READ);
      if (f) metaSize = f.size();
      f.close();
      xSemaphoreGive(fsMutex);
  }

  if (metaSize < 5) { debugln("[SVC] Invalid metadata. Scrubbing."); cleanup_service_report(true); return false; }

  size_t stage1Len = stnPart.length() + metaHeader.length() + metaSize + 2 + footer.length();

  int hSetupFailCount = 0; 
  for (int att = 1; att <= 2; att++) {
      active_cid = 1; // v5.89: Standardize on CID 1 for 2G stability
      if (!verify_bearer_or_recover()) { vTaskDelay(2000/portTICK_PERIOD_MS); continue; }
      
      SerialSIT.println("AT+HTTPTERM"); waitForResponse("OK", 500); 
      SerialSIT.println("AT+HTTPINIT"); if (!waitForResponse("OK", 3000)) continue;
      
      bool prep_fail = false;
      SerialSIT.println("AT+HTTPPARA=\"CID\",1"); 
      if (!waitForResponse("OK", 2000)) prep_fail = true;
      if (!prep_fail) {
          char url[128]; snprintf(url, sizeof(url), "AT+HTTPPARA=\"URL\",\"http://%s/api/v2/service_report\"", HEALTH_SERVER_IP);
          SerialSIT.println(url); if (!waitForResponse("OK", 3000)) prep_fail = true;
      }
      if (!prep_fail) {
          char ct[96]; snprintf(ct, sizeof(ct), "AT+HTTPPARA=\"CONTENT\",\"multipart/form-data; boundary=%s\"", bnd);
          SerialSIT.println(ct); if (!waitForResponse("OK", 3000)) prep_fail = true;
      }
      
      if (prep_fail) {
          debugln("[GPRS] [CRIT] HTTP Setup ERROR. Triggering NUCLEAR POWER CYCLE (Pin 26)...");
          digitalWrite(26, LOW); vTaskDelay(2000/portTICK_PERIOD_MS);
          digitalWrite(26, HIGH); vTaskDelay(5000/portTICK_PERIOD_MS);
          return false; // Exit and let the next auto-retry handle the fresh modem
      }

      char dcmd[32]; snprintf(dcmd, sizeof(dcmd), "AT+HTTPDATA=%u,30000", (uint32_t)stage1Len);
      flushSerialSIT();
      SerialSIT.println(dcmd);
      if (waitForResponse("DOWNLOAD", 15000)) {
          vTaskDelay(500 / portTICK_PERIOD_MS); // v5.96: Breather for modem internal buffer
          
          SerialSIT.print(stnPart); SerialSIT.print(metaHeader);
          if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
              File f = SPIFFS.open("/svc_pending.json", FILE_READ);
              if (f) {
                  uint8_t s_buf[128]; // v5.89: Optimized for performance
                  while(f.available()) { 
                      size_t r = f.read(s_buf, sizeof(s_buf));
                      SerialSIT.write(s_buf, r);
                      vTaskDelay(5 / portTICK_PERIOD_MS); 
                  }
                  f.close();
              }
              xSemaphoreGive(fsMutex);
          }
          SerialSIT.print(crlf); SerialSIT.print(footer); SerialSIT.flush();
          if (waitForResponse("OK", 15000)) {
              SerialSIT.println("AT+HTTPACTION=1");
              if (waitForResponse("+HTTPACTION: 1,200,", 30000)) { stage1Ok = true; break; }
          }
      } else {
          debugf("[SVC] Metadata DOWNLOAD Timeout! Resp: %s\n", modem_response_buf);
      }
      debugf("[SVC] M-Sync Retry %d...\n", att); vTaskDelay(2000/portTICK_PERIOD_MS);
  }

  if (!stage1Ok) { 
      debugln("[SVC] Stage 1 Fail (Metadata)."); 
      strcpy(svc_last_error, "META_FAIL");
      svc_sync_status = SVC_FAIL; 
      return false; 
  }
  debugf("[SVC] Stage 1 OK (Size: %d)\n", stnPart.length());

  bool s2Ok = (img1Len == 0); 
  bool s3Ok = (img2Len == 0);

  // --- STAGE 2: IMAGE 1 ---
  if (img1Len > 100) {
      svc_sync_status = SVC_SYNC_IMG1; 
      String tsPart = String(dash) + bnd + crlf + "Content-Disposition: form-data; name=\"ts\"" + crlf + crlf + String(timestamp) + crlf;
      String i1Header = String(dash) + bnd + crlf + "Content-Disposition: form-data; name=\"img1\"; filename=\"i1.jpg\"" + crlf + "Content-Type: image/jpeg" + crlf + crlf;
      size_t s2Len = stnPart.length() + tsPart.length() + i1Header.length() + img1Len + 2 + footer.length();
      
      for (int att = 1; att <= 3; att++) {
          active_cid = 1;
          if (!verify_bearer_or_recover()) { vTaskDelay(2000/portTICK_PERIOD_MS); continue; }
          SerialSIT.println("AT+HTTPTERM"); waitForResponse("OK", 500); 
          SerialSIT.println("AT+HTTPINIT"); if (!waitForResponse("OK", 2000)) continue;
          bool p2_fail = false;
          SerialSIT.println("AT+HTTPPARA=\"CID\",1"); if (!waitForResponse("OK", 2000)) p2_fail = true;
          if (!p2_fail) {
              char url[128]; snprintf(url, sizeof(url), "AT+HTTPPARA=\"URL\",\"http://%s/api/v2/service_report\"", HEALTH_SERVER_IP);
              SerialSIT.println(url); if (!waitForResponse("OK", 3000)) p2_fail = true;
          }
          if (!p2_fail) {
              char ct[96]; snprintf(ct, sizeof(ct), "AT+HTTPPARA=\"CONTENT\",\"multipart/form-data; boundary=%s\"", bnd);
              SerialSIT.println(ct); if (!waitForResponse("OK", 3000)) p2_fail = true;
          }
          if (p2_fail) {
              debugln("[GPRS] [CRIT] Image 1 HTTP Setup ERROR. Triggering NUCLEAR POWER CYCLE (Pin 26)...");
              digitalWrite(26, LOW); vTaskDelay(2000/portTICK_PERIOD_MS);
              digitalWrite(26, HIGH); vTaskDelay(5000/portTICK_PERIOD_MS);
              return false; // Exit and let auto-retry handle fresh modem
          }

          char dcmd[32]; snprintf(dcmd, sizeof(dcmd), "AT+HTTPDATA=%u,30000", (uint32_t)s2Len);
          flushSerialSIT();
          SerialSIT.println(dcmd);
          if (waitForResponse("DOWNLOAD", 15000)) {
              SerialSIT.print(stnPart); SerialSIT.print(tsPart); SerialSIT.print(i1Header);
              if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
                  File f = SPIFFS.open("/svc_img1.jpg", FILE_READ);
                  if (f) {
                      size_t yieldCounter = 0;
                      uint8_t buf[128]; // v5.89: Increased for performance
                      while (f.available()) {
                          size_t r = f.read(buf, sizeof(buf));
                          SerialSIT.write(buf, r);
                          yieldCounter += r;
                          
                          // v5.89: Yield fsMutex every 4KB
                          if (yieldCounter >= 4096) {
                              xSemaphoreGive(fsMutex);
                              vTaskDelay(20 / portTICK_PERIOD_MS); // Yield CPU
                              if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) != pdTRUE) break;
                              yieldCounter = 0;
                          }
                          vTaskDelay(5 / portTICK_PERIOD_MS); // Reduced delay
                      }
                      f.close();
                  }
                  xSemaphoreGive(fsMutex);
              }
              SerialSIT.print(crlf); SerialSIT.print(footer); SerialSIT.flush();
              if (waitForResponse("OK", 10000)) {
                  SerialSIT.println("AT+HTTPACTION=1");
                  if (waitForResponse("+HTTPACTION: 1,200,", 60000)) { s2Ok = true; break; }
              }
          }
          debugf("[SVC] Photo 1 Retry %d...\n", att); vTaskDelay(2000/portTICK_PERIOD_MS);
      }
      if (s2Ok) { debugf("[SVC] Stage 2 OK (Size: %d)\n", (int)s2Len); SPIFFS.remove("/svc_img1.jpg"); }
      else { strcpy(svc_last_error, "IMG1_FAIL"); debugln("[SVC] Stage 2 Fail."); }
  }

  // --- STAGE 3: IMAGE 2 ---
  if (img2Len > 100) {
      svc_sync_status = SVC_SYNC_IMG2;
      String tsPart = String(dash) + bnd + crlf + "Content-Disposition: form-data; name=\"ts\"" + crlf + crlf + String(timestamp) + crlf;
      String i2Header = String(dash) + bnd + crlf + "Content-Disposition: form-data; name=\"img2\"; filename=\"i2.jpg\"" + crlf + "Content-Type: image/jpeg" + crlf + crlf;
      size_t s3Len = stnPart.length() + tsPart.length() + i2Header.length() + img2Len + 2 + footer.length();

      for (int att = 1; att <= 3; att++) {
          active_cid = 1;
          if (!verify_bearer_or_recover()) { vTaskDelay(2000/portTICK_PERIOD_MS); continue; }
          SerialSIT.println("AT+HTTPTERM"); waitForResponse("OK", 500); 
          SerialSIT.println("AT+HTTPINIT"); if (!waitForResponse("OK", 2000)) continue;
          bool p3_fail = false;
          SerialSIT.println("AT+HTTPPARA=\"CID\",1"); if (!waitForResponse("OK", 2000)) p3_fail = true;
          if (!p3_fail) {
              char url[128]; snprintf(url, sizeof(url), "AT+HTTPPARA=\"URL\",\"http://%s/api/v2/service_report\"", HEALTH_SERVER_IP);
              SerialSIT.println(url); if (!waitForResponse("OK", 3000)) p3_fail = true;
          }
          if (!p3_fail) {
              char ct[96]; snprintf(ct, sizeof(ct), "AT+HTTPPARA=\"CONTENT\",\"multipart/form-data; boundary=%s\"", bnd);
              SerialSIT.println(ct); if (!waitForResponse("OK", 3000)) p3_fail = true;
          }
          if (p3_fail) {
              debugln("[GPRS] [CRIT] Image 2 HTTP Setup ERROR. Triggering NUCLEAR POWER CYCLE (Pin 26)...");
              digitalWrite(26, LOW); vTaskDelay(2000/portTICK_PERIOD_MS);
              digitalWrite(26, HIGH); vTaskDelay(5000/portTICK_PERIOD_MS);
              return false; // Exit and let auto-retry handle fresh modem
          }

          char dcmd[32]; snprintf(dcmd, sizeof(dcmd), "AT+HTTPDATA=%u,30000", (uint32_t)s3Len);
          flushSerialSIT();
          SerialSIT.println(dcmd);
          if (waitForResponse("DOWNLOAD", 15000)) {
              SerialSIT.print(stnPart); SerialSIT.print(tsPart); SerialSIT.print(i2Header);
              if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
                  File f = SPIFFS.open("/svc_img2.jpg", FILE_READ);
                  if (f) {
                      size_t yieldCounter = 0;
                      uint8_t buf[128]; // v5.89: Increased for performance
                      while (f.available()) {
                          size_t r = f.read(buf, sizeof(buf));
                          SerialSIT.write(buf, r);
                          yieldCounter += r;

                          // v5.89: Yield fsMutex every 4KB
                          if (yieldCounter >= 4096) {
                              xSemaphoreGive(fsMutex);
                              vTaskDelay(20 / portTICK_PERIOD_MS); // Yield CPU
                              if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) != pdTRUE) break;
                              yieldCounter = 0;
                          }
                          vTaskDelay(5 / portTICK_PERIOD_MS); // Reduced delay
                      }
                      f.close();
                  }
                  xSemaphoreGive(fsMutex);
              }
              SerialSIT.print(crlf); SerialSIT.print(footer); SerialSIT.flush();
              if (waitForResponse("OK", 10000)) {
                  SerialSIT.println("AT+HTTPACTION=1");
                  if (waitForResponse("+HTTPACTION: 1,200,", 60000)) { s3Ok = true; break; }
              }
          }
          debugf("[SVC] Photo 2 Retry %d...\n", att); vTaskDelay(2000/portTICK_PERIOD_MS);
      }
      if (s3Ok) { debugf("[SVC] Stage 3 OK (Size: %d)\n", (int)s3Len); SPIFFS.remove("/svc_img2.jpg"); }
      else { strcpy(svc_last_error, "IMG2_FAIL"); debugln("[SVC] Stage 3 Fail."); }
  }

  bool finalOk = (stage1Ok && s2Ok && s3Ok);
  if (finalOk) {
      svc_sync_status = SVC_DONE; 
      strcpy(svc_last_error, "NONE");
      cleanup_service_report(false);
      debugln("[SVC] Sync SUCCESS.");
  } else {
      svc_sync_status = SVC_FAIL;
      if (strlen(svc_last_error) < 4) strcpy(svc_last_error, "PARTIAL_FAIL");
      debugf("[SVC] Sync FAIL: %s. Preserving report.\n", svc_last_error);
  }

  SerialSIT.println("AT+HTTPTERM");
  waitForResponse("OK", 1000);
  return finalOk;
}
