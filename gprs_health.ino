#include "globals.h"

void get_signal_strength() {
  char rssiStr[4];
  const char *resp;

  debugln("************************");
  debugln("GETTING SIGNAL STRENGTH ");
  debugln("************************");
  // [HR-C02] Mutex Assertion: Ensure modem is locked to this task
  if (uxSemaphoreGetCount(modemMutex) != 0 && !bearer_recovery_active) {
      debugln("[CRIT] modemMutex NOT held during get_signal_strength!");
  }

  signal_lvl = SIGNAL_STRENGTH_MISSING_DATA; // v5.52 FIX: Default to SIGNAL_STRENGTH_MISSING_DATA not 0
  retries = 0;

  // [v5.86] Pre-settle delay: modem returns 85 (not ready) on first 1-3 polls
  // immediately after boot. Reduced to 200ms for faster startup.
  vTaskDelay(200 / portTICK_PERIOD_MS);

  int invalid_signal_count = 0;
  // rssi 0 = -113dBm. Continuous -113 is essentially no signal.
  while ((signal_lvl == SIGNAL_STRENGTH_MISSING_DATA) &&
         (retries < 30)) { // [C-03] v5.88: Capped at 30 (54s) to protect 180s scheduler budget
    esp_task_wdt_reset();
    last_activity_time =
        millis(); // v5.85: Pet the safety heartbeat during long signal search
    SerialSIT.println("AT+CSQ");
    if (waitForResponse("+CSQ:", 1000)) {
      const char* resp_ptr = strstr(modem_response_buf, "+CSQ: ");
      if (resp_ptr != NULL) {
        int raw_rssi = atoi(resp_ptr + 6);
        signal_strength = (-113 + 2 * raw_rssi);
        debug("Signal strength IN gprs task is ");
        debugln(signal_strength);
        if (signal_strength != 85 && signal_strength != -113) {
          signal_lvl = signal_strength;
          break;
        }
      }
    }
    
    if (signal_lvl == SIGNAL_STRENGTH_MISSING_DATA) {
      invalid_signal_count++;
      // [H-01] v5.88: Reduced to 10 for legitimate fast-fail
      if (invalid_signal_count >= 10) {
        debugln("[GPRS] Dead signal zone detected. Skipping long-poll wait.");
        signal_lvl = SIGNAL_STRENGTH_MISSING_DATA; 
        break;
      }
    }
    debug("Signal Level is ");
    debugln(signal_lvl);
    retries++;
    vTaskDelay(800 / portTICK_PERIOD_MS);
  }

  // CLAMP: Modem returns 85 for "No Signal". Convert to -111 sentinel.
  if (signal_strength == 85 || signal_strength > 31) {
    signal_strength = SIGNAL_STRENGTH_MISSING_DATA;
    signal_lvl = SIGNAL_STRENGTH_MISSING_DATA;
  }
}

/**
 * Attempts to fetch SIM number via USSD as a secondary fallback
 */
// USSD Discovery Removed - Inefficient for release

void get_network() {
  const char *resp;
  debugln();
  debugln("************************");
  debugln("GETTING NETWORK ");
  debugln("************************");

  char current_iccid[25] = {0};
  get_ccid(current_iccid, sizeof(current_iccid));

  // SMART CACHE LOGIC: Skip querying SIM info if we already have it in RTC
  // AND it matches the physical SIM in the slot.
  if (current_iccid[0] != '\0' && strcmp(cached_iccid, current_iccid) == 0 &&
      strcmp(sim_number, "NA") != 0) {
    debugln("[CACHE] Using cached carrier/number to save power.");
    if (strcmp(sim_msisdn, "NA") == 0 || sim_msisdn[0] == '\0') {
      retrieveOwnNumber(sim_msisdn, sizeof(sim_msisdn), true);
      if (strlen(sim_msisdn) > 3 && strcmp(sim_msisdn, "Not Found") != 0) {
        debugf("[SIM] MSISDN retrieved on cache hit: %s\n", sim_msisdn);
      } else {
        strcpy(sim_msisdn, "NONE");
      }
    }
    if (strstr(carrier, "Airtel")) {
      strcpy(apn_str, "airtelgprs.com"); 
      if (strlen(current_iccid) >= 6) {
        if (strncmp(current_iccid, "899116", 6) == 0 || 
            strncmp(current_iccid, "899110", 6) == 0 || 
            strncmp(current_iccid, "899145", 6) == 0) {
          strcpy(apn_str, "airteliot.com");
        }
      }
    } else if (strstr(carrier, "Jio")) {
      strcpy(apn_str, "jionet");
    } else if (strstr(carrier, "BSNL")) {
      strcpy(apn_str, "bsnlnet");
    } else if (strstr(carrier, "Vi")) {
      strcpy(apn_str, "www");
    } else {
      // [BSNL-FIX] ICCID prefix fallback for poisoned "SIM OK" cache entries
      if (strlen(current_iccid) >= 5 && 
          (strncmp(current_iccid, "89917", 5) == 0 ||
           strncmp(current_iccid, "89913", 5) == 0 ||
           strncmp(current_iccid, "89915", 5) == 0)) {
        strcpy(carrier, "BSNL");
        strcpy(apn_str, "bsnlnet");
        debugln("[CACHE] BSNL SIM detected via ICCID prefix on cache hit.");
      } else {
        if (strlen(current_iccid) >= 6 && 
            (strncmp(current_iccid, "899116", 6) == 0 || 
             strncmp(current_iccid, "899110", 6) == 0 || 
             strncmp(current_iccid, "899145", 6) == 0)) {
          strcpy(carrier, "Airtel");
          strcpy(apn_str, "airteliot.com");
          debugln("[CACHE] Airtel M2M SIM detected via ICCID prefix on cache hit.");
        } else {
          strcpy(apn_str, "airtelgprs.com"); 
        }
      }
    }
    return;
  }

full_discovery:
  debugln("[CACHE] New SIM or No Cache. Performing full discovery...");
  strncpy(cached_iccid, current_iccid, sizeof(cached_iccid) - 1);
  cached_iccid[sizeof(cached_iccid) - 1] = '\0';
  strcpy(sim_number, "NA");
  strcpy(sim_msisdn, "NA");
  strcpy(carrier, "NA");
  apn_saved_this_sim = false; 

  dns_fallback_active = false;
  preferred_ftp_mode = -1;
  cached_server_ip[0] = '\0';
  // Query Modem Hardware Revision (ATI) & Firmware Build (CGMR)
  SerialSIT.println("AT+ATI");
  if (waitForResponse("OK", 2000)) {
    debugf("[MODEM-INFO] Model (ATI): %s\n", modem_response_buf);
  }
  SerialSIT.println("AT+CGMR");
  if (waitForResponse("OK", 2000)) {
    debugf("[MODEM-INFO] Firmware Revision (CGMR): %s\n", modem_response_buf);
  }

  // [BSNL-FIX] Step 0: CIMI first — works before registration, SIM-resident
  SerialSIT.println("AT+CIMI");
  if (waitForResponse("OK", 2000)) {
    const char* resp = modem_response_buf;
    int outIdx = 0;
    bool foundStart = false;
    for (int i = 0; resp[i] != '\0' && outIdx < 15; i++) {
      if (isdigit(resp[i])) {
        sim_number[outIdx++] = resp[i];
        foundStart = true;
      } else if (foundStart) {
        break;
      }
    }
    sim_number[outIdx] = '\0';
    if (outIdx >= 10) {
      debug("IMSI parsed: ");
      debugln(sim_number);
    } else {
      strcpy(sim_number, "NA");
    }
  } else {
    strcpy(sim_number, "NA");
  }

  // Query MSISDN (Phone Number) if programmed on the SIM card
  retrieveOwnNumber(sim_msisdn, sizeof(sim_msisdn), true);
  if (strlen(sim_msisdn) > 3 && strcmp(sim_msisdn, "Not Found") != 0) {
    debugf("[SIM] MSISDN parsed: %s\n", sim_msisdn);
  } else {
    strcpy(sim_msisdn, "NA");
  }

  // [BSNL-FIX] Step 1: ICCID prefix detection (fast, no network needed)
  bool detected = false;
  if (!detected && strlen(current_iccid) >= 5 && 
      (strncmp(current_iccid, "89917", 5) == 0 ||
       strncmp(current_iccid, "89913", 5) == 0 ||
       strncmp(current_iccid, "89915", 5) == 0)) {
      strcpy(carrier, "BSNL"); 
      strcpy(apn_str, "bsnlnet");
      debugln("[APN] BSNL SIM detected via ICCID prefix.");
      detected = true;
  }
  // [BSNL-FIX] Step 2: IMSI MCC/MNC fallback for newer BSNL SIMs
  if (!detected && strlen(sim_number) >= 5) {
      char plmn[6] = {0};
      strncpy(plmn, sim_number, 5); // First 5 digits = MCC(3) + MNC(2)
      // BSNL major circles: 40434, 40438, 40454, 40455, 40471, 40522, 40523, 40524
      const char* bsnl_plmns[] = {"40434","40438","40454","40455","40471","40522","40523","40524", NULL};
      for (int i = 0; bsnl_plmns[i]; i++) {
          if (strcmp(plmn, bsnl_plmns[i]) == 0) {
              strcpy(carrier, "BSNL"); 
              strcpy(apn_str, "bsnlnet");
              debugln("[APN] BSNL SIM detected via IMSI prefix.");
              detected = true; 
              break;
          }
      }
  }
  
  if (detected) goto skip_cops_detection;

  {
    // 1. Try CSPN (Provider Name)
    SerialSIT.println("AT+CSPN?");
    waitForResponse("OK", 2000);
    char cspnResp[100];
    strncpy(cspnResp, modem_response_buf, sizeof(cspnResp)-1);
    cspnResp[sizeof(cspnResp)-1] = '\0';
    debug("CSPN Logic response: ");
    debugln(cspnResp);

    // 2. Try COPS (Operator) as fallback
    SerialSIT.println("AT+COPS?");
    waitForResponse("OK", 2000);
    char copsResp[100];
    strncpy(copsResp, modem_response_buf, sizeof(copsResp)-1);
    copsResp[sizeof(copsResp)-1] = '\0';
    debug("COPS Logic response: ");
    debugln(copsResp);

    for (int i = 0; cspnResp[i]; i++) cspnResp[i] = tolower(cspnResp[i]);
    for (int i = 0; copsResp[i]; i++) copsResp[i] = tolower(copsResp[i]);
    const char *r1 = cspnResp;
    const char *r2 = copsResp;

    // Determine Carrier and APN
    if (strstr(r1, "airtel") || strstr(r2, "airtel")) {
      strcpy(carrier, "Airtel");
      // v5.78 Hardening: Standard Airtel 10-digit SIMs use airtelgprs.com 
      // IoT/M2M SIMs (13-digit) use airteliot.com. We refine this via ICCID.
      strcpy(apn_str, "airtelgprs.com"); 

      if (strlen(current_iccid) >= 6) {
        if (strncmp(current_iccid, "899116", 6) == 0 || 
            strncmp(current_iccid, "899110", 6) == 0 || 
            strncmp(current_iccid, "899145", 6) == 0) {
          strcpy(apn_str, "airteliot.com");
          debugln("[APN] Airtel IoT/M2M SIM detected via ICCID prefix.");
        } else {
          strcpy(apn_str, "airtelgprs.com");
          debugln("[APN] Airtel Commercial SIM detected via ICCID prefix.");
        }
      }
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
      if (strlen(current_iccid) >= 6 && 
          (strncmp(current_iccid, "899116", 6) == 0 || 
           strncmp(current_iccid, "899110", 6) == 0 || 
           strncmp(current_iccid, "899145", 6) == 0)) {
        strcpy(carrier, "Airtel");
        strcpy(apn_str, "airteliot.com");
        debugln("[APN] Airtel IoT/M2M SIM detected via ICCID prefix fallback.");
      } else {
        strcpy(carrier, "SIM OK");
        strcpy(apn_str, "airtelgprs.com");
      }
    }
  }

skip_cops_detection:
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
  set_sys_status("GPRS REGISTER");
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
  int no_of_retries = 24; // v5.75: BSNL-Hardened cap (C-01 adjusted to 24) to
                          // prioritize data success
  int initial_cnmp = last_successful_cnmp; // v5.85: Elevated to function scope
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
  if (!isBSNL && waitForResponse("+CGREG:", 1000)) {
    const char* cgregResp = modem_response_buf;
    if (strstr(cgregResp, ",1") != NULL || strstr(cgregResp, ",5") != NULL) {
      debugln(
          "[GPRS] Fast-Track: Modem already registered! Bypassing setup block.");
      is_registered = true;
      strcpy(reg_status,
             (strstr(cgregResp, ",1") != NULL) ? "GSM:Home:OK" : "GSM:Roam:OK");
    }
  } else {
    // RUN FULL SETUP BLOCK ONLY IF NOT REGISTERED
    // v5.60 SURE-SHOT REGISTRATION:
    SerialSIT.println("AT+CFUN=1"); // Full functionality
    waitForResponse("OK", 1000);
    SerialSIT.println("AT+CGDCONT=8,\"IP\",\"\""); // Kill CID 8 side-channel
    waitForResponse("OK", 1000);
    SerialSIT.println("AT+CGDCONT=9,\"IP\",\"\""); // Kill CID 9 side-channel
                                                   // (Claude Recommendation)
    waitForResponse("OK", 1000);

    // v5.70 Hardened: [N-12] Read-Before-Write guard to prevent Modem Flash
    // Wear
    if (strlen(apn_str) > 3) {
      SerialSIT.println("AT+CGDCONT?");
      waitForResponse("OK", 2000);
      const char* cgdcont_resp = modem_response_buf;
      const char* expected_proto =
          (strcmp(apn_str, "jionet") == 0) ? "IPV4V6" : "IP";

      if (strstr(cgdcont_resp, apn_str) == NULL ||
          strstr(cgdcont_resp, expected_proto) == NULL) {
        debugf("[GPRS] Pre-setting APN for CID 1: %s (%s)\n", apn_str, expected_proto);
        SerialSIT.print("AT+CGDCONT=1,\"");
        SerialSIT.print(expected_proto);
        SerialSIT.print("\",\"");
        SerialSIT.print(apn_str);
        SerialSIT.println("\"");
        waitForResponse("OK", 1000);
      } else {
        debugln("[GPRS] APN already set in CGDCONT. Bypassing flash write.");
      }
    }

    // v6.40: 4G-First Strategy for all SIM cards (Airtel, Jio, BSNL 4G) with 2G fallback
    if (last_successful_cnmp == 13) {
      initial_cnmp = 13; // Preserve GSM mode from previous session for fast 2G boot
    } else {
      initial_cnmp = 38; // Default to 4G LTE (CNMP=38) for Airtel / Jio / BSNL 4G
    }

    char cnmp_cmd[20];
    snprintf(cnmp_cmd, sizeof(cnmp_cmd), "AT+CNMP=%d", initial_cnmp);
    SerialSIT.println(cnmp_cmd);
    debugf("[GPRS] Network Mode: %s (isBSNL:%d)\n", cnmp_cmd, isBSNL);

    waitForResponse("OK", 1000);
    vTaskDelay(500 / portTICK_PERIOD_MS); // v5.72: Radio settle delay after
                                          // mode change (Issue 5)
    SerialSIT.println("AT+CMNB=3");       // LTE then GSM
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
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Let it settle
  }

  // v5.79: Restore v5.75 Netlight logic (Ensures pulse even if Fast-Tracked)
  // (Netlight already enabled in start_gprs for faster visual feedback)


  while (!is_registered && (retries < no_of_retries)) {
    esp_task_wdt_reset();
    last_activity_time = millis(); // v5.85: Pet the safety heartbeat during
                                   // long registration cycles

    // UI Feedback: Show progress on LCD (v5.81 Surgical: Guarded 16-char buffer)
    snprintf(reg_status, 16, "TRY #%d...", retries + 1);

    int r2 = -1, r4 = -1;

    // --- 1. Check CEREG (4G LTE) for ALL SIM Cards (Airtel, Jio, BSNL 4G) ---
    SerialSIT.println("AT+CEREG?");
    if (waitForResponse("+CEREG:", 2000)) {
      const char* resp4 = modem_response_buf;
      const char* c4_ptr = strchr(resp4, ',');
      if (c4_ptr != NULL) {
        r4 = atoi(c4_ptr + 1);

        if (r4 == 3) {
          const char* c4b_ptr = strchr(c4_ptr + 1, ','); // comma after stat field
          if (c4b_ptr != NULL) {
            debugln(
                "[GPRS] CEREG=3 but LTE cell visible."
                " Pushing CGATT and waiting...");
            SerialSIT.println("AT+CGATT=1");
            waitForResponse("OK", 3000);
            r4 = 0; // Reset to 'searching' — do NOT count as denied
          } else {
            debugln("[GPRS] CEREG=3, no cell info. Truly no LTE signal.");
          }
        }
      }
    }

    // --- 2. Check CREG (2G/3G GSM) for ALL SIM Cards ---
    SerialSIT.println("AT+CREG?");
    if (waitForResponse("+CREG:", 2000)) {
      const char* resp2 = modem_response_buf;
      const char* c2_ptr = strchr(resp2, ',');
      if (c2_ptr != NULL)
        r2 = atoi(c2_ptr + 1);
    }

    // Determine overall registration status (4G preferred)
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
    // 1. Prioritize 4G LTE (CEREG) for all SIMs
    if (r4 == 1 || r4 == 5) {
      is_registered = true;
      isLTE = true;
      last_successful_cnmp = 38; // Keep 4G LTE Preferred
      strcpy(reg_status, (r4 == 1) ? "LTE:Home:OK" : "LTE:Roam:OK");
      debugf("[GPRS] Registered via CEREG! (4G:%d)\n", r4);
      break;
    }

    // 2. 2G Fallback (CREG): Accept 2G after retry #5 or if already in GSM mode
    if ((r2 == 1 || r2 == 5) && (initial_cnmp == 13 || retries >= 5)) {
      is_registered = true;
      isLTE = false;
      last_successful_cnmp = 13; // Save 2G mode for fast next boot
      strcpy(reg_status, (r2 == 1) ? "GSM:Home:OK" : "GSM:Roam:OK");
      debugf("[GPRS] Registered via CREG! (2G:%d)\n", r2);
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
        if (initial_cnmp != 13) {
          debugln("[GPRS] Tier1 @ iter5: 4G LTE attach pending. Switching to GSM-only fallback (CNMP=13)...");
          SerialSIT.println("AT+CNMP=13");
          waitForResponse("OK", 1000);
        } else {
          debugln("[GPRS] Tier1 @ iter5: Already in GSM mode. Forcing cell rescan (COPS=0)...");
        }
        SerialSIT.println("AT+COPS=0");
        waitForResponse("OK", 3000);
        SerialSIT.println("AT+CGATT=1");
        waitForResponse("OK", 3000);
      } else if (retries == 10) {
        if (!isBSNL) {
          // v6.40 Tier 2 @ iter 10 for Airtel/Jio: 4G did not register in 10s. Fallback to 2G GSM
          debugln("[GPRS] Airtel/Jio Tier2 @ iter10: 4G LTE timed out (10s). Fallback to GSM-only (CNMP=13)...");
          SerialSIT.println("AT+CNMP=13");
          waitForResponse("OK", 1000);
          SerialSIT.println("AT+COPS=0");
        } else {
          // v5.84 Tier 2 @ iter 10: Full Radio Reset & SIM Scrub for BSNL
          debugln("[GPRS] Tier2 @ iter10: Radio-Off SIM Scrub...");
          SerialSIT.println("AT+CFUN=0");
          esp_task_wdt_reset();
          waitForResponse("OK", 5000);
          SerialSIT.println(
              "AT+CRSM=214,28539,0,0,12,\"FFFFFFFFFFFFFFFFFFFFFFFF\"");
          waitForResponse("OK", 2000);
        }
        SerialSIT.println("AT+CFUN=1");
        esp_task_wdt_reset();
        waitForResponse("OK", 5000);
        SerialSIT.println("AT+COPS=0");
        esp_task_wdt_reset();
        waitForResponse("OK", 5000);
        SerialSIT.println("AT+CGATT=1");
        waitForResponse("OK", 3000);
      } else if (retries == 15) {
        if (isBSNL) {
          // v5.85 BSNL-Aware Tier 3: Instead of wasting time on LTE-Only
          // (CNMP=38), we perform another fresh Operator probe while staying in
          // GSM-only.
          debugln("[GPRS] BSNL Tier3 @ iter15: Skipping LTE probe. Performing "
                  "fresh COPS scan (GSM-only)...");
          SerialSIT.println("AT+COPS=0");
          esp_task_wdt_reset();
          waitForResponse("OK", 5000);
          SerialSIT.println("AT+CGATT=1");
          waitForResponse("OK", 3000);
        } else {
          // v5.84 Airtel/Jio Tier 3: Force LTE-Only mode
          debugln(
              "[GPRS] Tier3 @ iter15: Testing LTE-Only Mode (AT+CNMP=38)...");
          SerialSIT.println("AT+CNMP=38");
          waitForResponse("OK", 2000);
          SerialSIT.println("AT+COPS=0");
          esp_task_wdt_reset();
          waitForResponse("OK", 5000);
          SerialSIT.println("AT+CGATT=1");
          waitForResponse("OK", 3000);
        }
      } else if (retries ==
                 20) { // [C-01] Tier 4 aligned for final retries
        // Tier 4 @ iter 20 (Final Attempt): Restore Auto-Mode + COPS auto
        debugln("[GPRS] Tier4 @ iter20: Restoring Auto-Mode (CNMP=2, COPS=0) "
                "for final retry...");
        SerialSIT.println("AT+CNMP=2"); // Auto (LTE+GSM)
        waitForResponse("OK", 1000);
        SerialSIT.println("AT+COPS=0"); // Auto operator
        esp_task_wdt_reset();
        waitForResponse("OK", 5000);    // Reduced from 10s to stay in window
        SerialSIT.println("AT+CGATT=1");
        waitForResponse("OK", 3000);
      }
    }

    // BSNL early kick: if stuck in Idle at iter 3, force re-scan
    if (isBSNL && registration == 0 && retries == 3) {
      debugln(
          "[GPRS] BSNL Stubborn Idle. Forcing Frequency Re-scan (COPS=0)...");
      SerialSIT.println("AT+CGACT=0,1");
      waitForResponse("OK", 2000);
      diag_http_present_fails++;
      SerialSIT.println("AT+COPS=0");
      esp_task_wdt_reset();
      waitForResponse("OK", 5000);
    }

    debugf("Reg Search [Carrier:%s | BSNL_Flag:%d]... Status:%d Iter:#%d/%d\n", carrier, isBSNL,
           registration, retries + 1, no_of_retries);

    // v5.84: ADAPTIVE WAIT: Poll CGREG every 1s for up to 2s.
    // M-NEW-4: Capped at 2s max for all carriers to save window time.
    int poll_count = 2;
    for (int w = 0; w < poll_count && !is_registered; w++) {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      esp_task_wdt_reset();
      flushSerialSIT();
      SerialSIT.println("AT+CGREG?");
      if (waitForResponse("OK", 2000)) {
        const char* qr = modem_response_buf;
        const char* qi_ptr = strstr(qr, "+CGREG:");
        if (qi_ptr != NULL) {
          const char* qc_ptr = strchr(qi_ptr, ',');
          if (qc_ptr != NULL) {
            int qreg = atoi(qc_ptr + 1);
            if (qreg == 1 || qreg == 5) {
              debugln("[GPRS] CGREG registered during adaptive wait!");
              isLTE = !isBSNL;
              last_successful_cnmp = isLTE ? 2 : 13; // v5.84
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
    set_sys_status("GPRS REG OK");
    debugln("Registration Successful.");
    // v5.86: PDP Breather — give the modem 800ms to settle its IP stack after registration
    vTaskDelay(800 / portTICK_PERIOD_MS);
  } else {
    set_sys_status("GPRS REG FAIL");
    diag_gprs_fails++;
    diag_consecutive_reg_fails++;
    gprs_mode = eGprsSignalForStoringOnly;
    debugf1("Registration failed. Consecutive fails: %d/10\n",
            diag_consecutive_reg_fails);

    // v5.56: Aggressive Modem Reset before full system reboot
    if (diag_consecutive_reg_fails == 4 || diag_consecutive_reg_fails == 8) {
      debugln("[GPRS] Persistent failure. Triggering MODEM HARD RESET (GPIO "
              "26)...");
      digitalWrite(26, LOW);
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      digitalWrite(26, HIGH);
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      recoverI2CBus(); // v5.82: Mitigation for power-spike induced I2C hangs
    }
    if (diag_consecutive_reg_fails >= 10) {
      debugln("[GPRS] PERSISTENT REG FAIL. Resetting system...");
      diag_consecutive_reg_fails =
          0; // Prevent infinite persistent restart loop
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      ESP.restart();
    }
  }
}

void get_a7672s() {
  if (strlen(cached_iccid) < 10) {
    get_ccid(cached_iccid, 25);
  }
  char stored_apn[50];

  debugln("--- GPRS SETTING PDP ---");
  debugln();
  debugln("************************");
  debugln("Setting PDP context ");
  debugln("************************");
  debugln();

  active_cid = 1; // Default to CID 1 for all carriers / systems
  // v5.63: Native v3.0 style activation.
  // No status queries, fire and move on.
  if (load_apn_config(cached_iccid, stored_apn, sizeof(stored_apn))) {
    strncpy(apn_str, stored_apn, sizeof(apn_str) - 1);
    if (try_activate_apn(apn_str)) {
      vTaskDelay((isLTE ? 500 : 3000) /
                 portTICK_PERIOD_MS); // v5.85: Carrier-aware breather
      gprs_pdp_ready = true;

      // CID State Anchor
      SerialSIT.println("AT+CGDCONT?");
      if (waitForResponse("OK", 3000)) {
        const char* final_cont = modem_response_buf;
        char target_apn_quoted[64];
        snprintf(target_apn_quoted, sizeof(target_apn_quoted), "\"%s\"", apn_str);
        if (strstr(final_cont, target_apn_quoted) == NULL &&
            strstr(final_cont, apn_str) == NULL) {
          char cg_buf[100];
          snprintf(cg_buf, sizeof(cg_buf), "AT+CGDCONT=1,\"IP\",\"%s\"", apn_str);
          SerialSIT.println(cg_buf);
          waitForResponse("OK", 3000);
          SerialSIT.println("AT+CGACT=1,1");
          waitForResponse("OK", 5000);
        }
      }
      return;
    }
  }

  // If stored APN failed or doesn't exist, start Smart APN cycle.
  // v5.65 FIX: Carrier-aware search — use carrier detected by get_network() to
  // try the correct APN first. Prevents BSNL units from connecting with
  // airteliot.com.
  debugln("APN: Starting Carrier-Aware APN Search...");

  // 5. Build priority list: correct carrier APN first, then fallbacks
  const char *primary_apn =
      apn_str; 
  const char *fallback_apns[] = {"airteliot.com", "airtelgprs.com", "bsnlnet",
                                 "jionet", "bsnlm2m"};

  if (strlen(primary_apn) > 0) {
    debugf("APN: Trying carrier-matched APN first -> %s\n", primary_apn);
    if (try_activate_apn(primary_apn)) {
      save_apn_config(primary_apn, cached_iccid);
      vTaskDelay((isLTE ? 500 : 3000) / portTICK_PERIOD_MS);
      gprs_pdp_ready = true;

      SerialSIT.println("AT+CGDCONT?");
      if (waitForResponse("OK", 3000)) {
        const char* final_cont = modem_response_buf;
        char target_apn_quoted[50];
        snprintf(target_apn_quoted, sizeof(target_apn_quoted), "\"%s\"", primary_apn);
        if (strstr(final_cont, target_apn_quoted) == NULL &&
            strstr(final_cont, primary_apn) == NULL) {
          char cg_buf[100];
          snprintf(cg_buf, sizeof(cg_buf), "AT+CGDCONT=1,\"IP\",\"%s\"", primary_apn);
          SerialSIT.println(cg_buf);
          waitForResponse("OK", 3000);
          SerialSIT.println("AT+CGACT=1,1");
          waitForResponse("OK", 5000);
        }
      }
      return;
    }
  }

  // Fallback: try remaining APNs (skipping the one we already tried)
  for (int i = 0; i < 5; i++) {
    if (strcmp(fallback_apns[i], primary_apn) == 0)
      continue; 
    if (try_activate_apn(fallback_apns[i])) {
      if (!((strstr(cached_iccid, "899145") != NULL || strstr(cached_iccid, "899116") != NULL || strstr(cached_iccid, "899110") != NULL) &&
            strcmp(fallback_apns[i], "airtelgprs.com") == 0)) {
        strcpy(apn_str, fallback_apns[i]);
        save_apn_config(fallback_apns[i], cached_iccid);
      } else {
        debugln("[APN] Guard: Connected on temporary fallback 'airtelgprs.com', but preserving global apn_str as 'airteliot.com'.");
      }
      vTaskDelay((isLTE ? 500 : 3000) / portTICK_PERIOD_MS);
      gprs_pdp_ready = true;

      SerialSIT.println("AT+CGDCONT?");
      if (waitForResponse("OK", 3000)) {
        const char* final_cont = modem_response_buf;
        char target_apn_quoted[50];
        snprintf(target_apn_quoted, sizeof(target_apn_quoted), "\"%s\"", fallback_apns[i]);
        if (strstr(final_cont, target_apn_quoted) == NULL &&
            strstr(final_cont, fallback_apns[i]) == NULL) {
          char cg_buf[100];
          snprintf(cg_buf, sizeof(cg_buf), "AT+CGDCONT=1,\"IP\",\"%s\"", fallback_apns[i]);
          SerialSIT.println(cg_buf);
          waitForResponse("OK", 3000);
          SerialSIT.println("AT+CGACT=1,1");
          waitForResponse("OK", 5000);
        }
      }
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
    save_apn_config(apn_str, cached_iccid);
    vTaskDelay((isLTE ? 500 : 3000) / portTICK_PERIOD_MS);
    gprs_pdp_ready = true;

    SerialSIT.println("AT+CGDCONT?");
    if (waitForResponse("OK", 3000)) {
        const char* final_cont = modem_response_buf;
        char target_apn_quoted[64];
        snprintf(target_apn_quoted, sizeof(target_apn_quoted), "\"%s\"", apn_str);
        if (strstr(final_cont, target_apn_quoted) == NULL &&
            strstr(final_cont, apn_str) == NULL) {
          debugln("[APN] CID Contamination detected. Forcing State Anchor...");
          char cg_buf[100];
          snprintf(cg_buf, sizeof(cg_buf), "AT+CGDCONT=1,\"IP\",\"%s\"", apn_str);
          SerialSIT.println(cg_buf);
          waitForResponse("OK", 3000);
          SerialSIT.println("AT+CGACT=1,1");
          waitForResponse("OK", 5000);
        }
    }
    return;
  }

  debugln("APN: FAILED. Going to store only mode.");
  gprs_mode = eGprsSignalForStoringOnly;
  gprs_pdp_ready = false;
}

// v6.56b: one SMS send, as the LPC unit does it (AIO_LPC sim7600_driver.c send_sms_to_number):
// the command ends on a single CR (println added CR LF, which reached the text after the '>'),
// the text has no trailing CR/LF (a CR opens a new input line), 100 ms before Ctrl+Z, a longer
// wait for the network, and the modem's own answer is logged when it is refused.
// REQUIRES: modemMutex held by caller.
bool sms_send_text(const char *number, const char *text, unsigned long confirm_ms) {
  SerialSIT.print("AT+CMGF=1\r");
  waitForResponse("OK", 2000);
  SerialSIT.print("AT+CSCA?\r");
  if (waitForResponse("OK", 1000)) {
    debug("[SMS] SMS centre: ");
    debugln(modem_response_buf);
  }
  debug("[SMS] To: ");
  debugln(number);
  char cmd[48];
  snprintf(cmd, sizeof(cmd), "AT+CMGS=\"%s\"\r", number);
  SerialSIT.print(cmd);
  if (!waitForResponse(">", 15000)) {
    debug("[SMS] No '>' prompt. Modem said: ");
    debugln(modem_response_buf);
    SerialSIT.write(0x1B);            // leave text input mode
    waitForResponse("OK", 1000);
    return false;
  }
  size_t len = strlen(text);
  while (len > 0 && (text[len - 1] == '\r' || text[len - 1] == '\n')) len--;
  vTaskDelay(50 / portTICK_PERIOD_MS);
  SerialSIT.write((const uint8_t *)text, len);
  vTaskDelay(100 / portTICK_PERIOD_MS); // breather before the transmit surge
  SerialSIT.write(0x1A);                // Ctrl+Z: send
  if (waitForResponse("+CMGS:", confirm_ms)) {
    debug("[SMS] Sent: ");
    debugln(modem_response_buf);
    return true;
  }
  debug("[SMS] FAILED. Modem said: ");
  debugln(modem_response_buf[0] ? modem_response_buf : "(nothing, timed out)");
  return false;
}

void prepare_and_send_status(char *gsm_no, bool alreadyLocked) {
  set_sys_status("SENDING SMS");
  if (!alreadyLocked) {
    if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(10000)) != pdTRUE) {
      debugln("[SMS] FAILED: Modem Busy. Deferring Status request.");
      // Find the active LCD field to show busy status
      for (int i = 0; i < FLD_COUNT; i++) {
        if (i == FLD_SEND_STATUS) {
          strcpy(ui_data[i].bottomRow, "MODEM BUSY      ");
          show_now = 1;
        }
      }
      return;
    }
  }

  // Validate destination phone number
  if (!gsm_no || strlen(gsm_no) < 8) {
    debugln("[SMS] ERROR: Invalid or missing target phone number.");
    strcpy(ui_data[FLD_SEND_STATUS].bottomRow, "NO PHONE NUMBER ");
    show_now = 1;
    if (!alreadyLocked) xSemaphoreGive(modemMutex);
    return;
  }

  int response_no;
  char msg_type[9];
  static char status_response[256];
  static char gprs_xmit_buf[300];
  msg_sent = 0;
#if USE_NUVOTON_UI == 1
  read_and_calibrate_voltages();
#endif
  solar_val = (solar / (float)WIND_DIR_ADC_MAX) * 3.3 * 6.119;  // R_top=620K R_bot=100K → 720/100, adj for ADC_MAX=3480

  if (solar_val > 4)
    solar_conn = 1;
  else
    solar_conn = 0;

  // li_bat ADC reads handled inside get_calibrated_battery_voltage
  li_bat_val =
      get_calibrated_battery_voltage(); // Phase 8 Fix: eFuse-calibrated ADC
  snprintf(battery, sizeof(battery), "%04.1f", li_bat_val);
  bat_val = li_bat_val; // bat_val is given for storage in spiffs

  // Brownout Safeguard: If battery is critically low (< 3.35V), block 2A RF transmit surge to prevent brownout reset
  if (li_bat_val > 0.1f && li_bat_val < 3.35f) {
    debugf("[SMS] CRITICAL: Battery too low for SMS RF transmit (%.2fV < 3.35V). Blocking transmit to prevent Brownout Reset.\n", li_bat_val);
    strcpy(ui_data[FLD_SEND_STATUS].bottomRow, "LOW BATT (<3.4V)");
    show_now = 1;
    if (!alreadyLocked) xSemaphoreGive(modemMutex);
    return;
  }

  if (STATION_TYPE[0] == '\0') {
#if SYSTEM == 0
    strcpy(STATION_TYPE, "TRG");
#elif SYSTEM == 1
    strcpy(STATION_TYPE, "TWS");
#elif SYSTEM == 2
    strcpy(STATION_TYPE, "TWS-RF");
#elif SYSTEM == 3
    strcpy(STATION_TYPE, "TWSRP");
#endif
  }

  if (firmwareUpdated == 1) {
    strcpy(msg_type, "FW-UPD");
    firmwareUpdated = 0;
  } else if (send_status == 1 || sync_mode == eSMSStart) { // SMS trigger from LCD SEND_STATUS
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

  // Bat1 = MCU Power (3.3V system rail), Bat2 = GPRS Power (3.7V Li-ion battery)
  float primary_bat = (bat_3v3_val > 0.5f) ? bat_3v3_val : 3.3f;
  float secondary_bat = (li_bat_val > 0.5f) ? li_bat_val : 3.7f;

  // RF & TWS
  snprintf(
      status_response, sizeof(status_response),
      "%s,%s,%s,%s,%04d-%02d-%02dT%02d:%02d,%s,15,%04d,%04.1f,%04.1f,%s,%s,%s,"
      "%04d-%02d-%02d,%04d-%02d-%02dT%02d:%02d,0\r\n",
      NETWORK, STATION_TYPE, msg_type, cleanStn, current_year, current_month,
      current_day, current_hour, current_min, UNIT_VER, signal_strength,
      primary_bat, secondary_bat, (solar_conn ? "Y" : "N"), (sd_card_ok ? "SDC-OK" : "SDC-FAIL"),
      (calib_sts == 1 ? "CLB-OK" : "CLB-FAIL"),
      calib_year,  // calib
      calib_month, // calib
      calib_day,   // calib
      last_recorded_yy, last_recorded_mm, last_recorded_dd, last_recorded_hr,
      last_recorded_min);

  debug("Status response prepared is ");
  debugln(status_response);
  if (sms_send_text(gsm_no, status_response, 60000)) {
    msg_sent = 1;
    debugln("SUCCESS IN SENDING MSG");
  } else {
    debugln("FAILED TO SEND MSG");
  }

  if (!alreadyLocked)
    xSemaphoreGive(modemMutex);
}

void get_gps_coordinates(bool alreadyLocked) {
  set_sys_status("GETTING GPS");
  bool locked_by_us = false;

  if (alreadyLocked || (xSemaphoreGetMutexHolder(modemMutex) == xTaskGetCurrentTaskHandle())) {
    locked_by_us = false;
  } else if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(10000)) == pdTRUE) {
    locked_by_us = true;
  } else {
    debugln("[GPS] Error: Modem Mutex Timeout - skipping GPS request");
    return;
  }

  int tmp;
  double lat = 0.0, lon = 0.0;
  const char *response_ptr;
  char *csqstr;

  verify_bearer_or_recover();

  for (int retry = 0; retry < 4; retry++) {
    debugf1("[GPS] Requesting Coordinates (AT+CLBS=1), Attempt %d...\n",
            retry + 1);
    SerialSIT.println("ATE0");
    waitForResponse("OK", 2000);

    flushSerialSIT();
    if (retry % 2 == 0) {
      SerialSIT.println("AT+CLBS=1");
    } else {
      SerialSIT.println("AT+CLBS=1,1");
    }
    bool got_clbs = waitForResponse("+CLBS:", 15000);
    debugf("[GPS] Modem +CLBS Response: %s\n", modem_response_buf);
    if (got_clbs) {
      char date_buf[16] = {0}, time_buf[16] = {0};
      if (parse_clbs_response(modem_response_buf, lat, lon, date_buf, time_buf)) {
        lati = lat;
        longi = lon;
        gps_latitude = lat;
        gps_longitude = lon;
        saveGPS(); // Persist immediately

        // SYNC TIME: Extract YY/MM/DD and HH:MM:SS from network
        if (strlen(date_buf) >= 8 && strlen(time_buf) >= 8) {
          int yy, mm, dd, hr, mi, ss;
          if (sscanf(date_buf, "%d/%d/%d", &yy, &mm, &dd) == 3 &&
              sscanf(time_buf, "%d:%d:%d", &hr, &mi, &ss) == 3) {
            int full_year = 2000 + yy;
            if (full_year >= 2025 && mm >= 1 && mm <= 12 && dd >= 1 && dd <= 31) {
              debugf("[GPS] Network Time Sync (UTC): %04d-%02d-%02d %02d:%02d:%02d\n", full_year, mm, dd, hr, mi, ss);
              // v6.56 M-6: +CLBS time is UTC: set the RTC to IST (it was written raw,
              // 5 h 30 min behind, by this path only)
              set_rtc_from_utc(full_year, mm, dd, hr, mi, ss);
            }
          }
        }
        if (locked_by_us) xSemaphoreGive(modemMutex);
        return; // SUCCESS
      }
    }
    debugln("[GPS] Attempt failed (+CLBS error or timeout). Retrying in 2.5s...");
    vTaskDelay(2500 / portTICK_PERIOD_MS);
    if (retry == 1) {
      debugln("[GPS] Re-verifying bearer context before final attempts...");
      verify_bearer_or_recover();
    }
  }
  debugln("[GPS] Fresh coordinate acquisition failed. Checking SPIFFS backup...");
  if (lati == 0.0 || longi == 0.0) {
    loadGPS();
  }
  verify_bearer_or_recover(); // Ensure network bearer is active before health telemetry
  if (locked_by_us) xSemaphoreGive(modemMutex);
}

void get_lat_long_date_time(char *gsm_no, bool alreadyLocked) {
  set_sys_status("GETTING GPS");
  if (!alreadyLocked) {
    if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(10000)) != pdTRUE) {
      debugln("[GPS] FAILED: Modem Busy. Deferring GPS request.");
      strcpy(ui_data[FLD_SEND_GPS].bottomRow, "MODEM BUSY      ");
      show_now = 1;
      return;
    }
  }

  char status_response[256];
  char gprs_xmit_buf[300];
  msg_sent = 0;

  // Ensure modem bearer context (CGACT 1,1) is active before requesting location via CLBS
  verify_bearer_or_recover();

  SerialSIT.println("ATE0");
  if (waitForResponse("OK", 3000)) {
    debugln("HTTP response of ATE0: OK");
  }

  // To find Latitude and Longitude. Only with A7672S
  bool fresh_fix_ok = false;
  double fresh_lat = 0.0, fresh_lon = 0.0;
  for (int retry = 0; retry < 4; retry++) {
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    flushSerialSIT();
    if (retry % 2 == 0) {
      SerialSIT.println("AT+CLBS=1");
    } else {
      SerialSIT.println("AT+CLBS=1,1");
    }
    bool got_clbs = waitForResponse("+CLBS:", 15000);
    debugf("[GPS] Modem +CLBS Response: %s\n", modem_response_buf);
    if (got_clbs) {
      char date_buf[16] = {0}, time_buf[16] = {0};
      if (parse_clbs_response(modem_response_buf, fresh_lat, fresh_lon, date_buf, time_buf)) {
        lati = fresh_lat;
        longi = fresh_lon;
        gps_latitude = fresh_lat;
        gps_longitude = fresh_lon;
        saveGPS();
        fresh_fix_ok = true;

        // SYNC TIME: Extract YY/MM/DD and HH:MM:SS from network
        if (strlen(date_buf) >= 8 && strlen(time_buf) >= 8) {
          int yy, mm, dd, hr, mi, ss;
          if (sscanf(date_buf, "%d/%d/%d", &yy, &mm, &dd) == 3 &&
              sscanf(time_buf, "%d:%d:%d", &hr, &mi, &ss) == 3) {
            int full_year = 2000 + yy;
            if (full_year >= 2025 && mm >= 1 && mm <= 12 && dd >= 1 && dd <= 31) {
              debugf("[GPS] Network Time Sync (Manual, UTC): %04d-%02d-%02d %02d:%02d:%02d\n", full_year, mm, dd, hr, mi, ss);
              // v6.56 M-6: +CLBS time is UTC: set the RTC to IST (it was written raw,
              // 5 h 30 min behind, by this path only)
              set_rtc_from_utc(full_year, mm, dd, hr, mi, ss);
            }
          }
        }
        break; // Fresh fix acquired
      }
    }
    debugln("[GPS] Attempt failed (+CLBS error or timeout). Retrying in 2.5s...");
    vTaskDelay(2500 / portTICK_PERIOD_MS);
    if (retry == 1) {
      debugln("[GPS] Re-verifying bearer context before final attempts...");
      verify_bearer_or_recover();
    }
  }

  // Fallback to persisted SPIFFS location if fresh fix failed or lati/longi are zero
  if (!fresh_fix_ok || lati == 0.0 || longi == 0.0) {
    debugln("[GPS] Fresh fix unavailable in get_lat_long_date_time. Loading persisted location...");
    loadGPS();
  } 

  if (STATION_TYPE[0] == '\0') {
#if SYSTEM == 0
    strcpy(STATION_TYPE, "TRG");
#elif SYSTEM == 1
    strcpy(STATION_TYPE, "TWS");
#elif SYSTEM == 2
    strcpy(STATION_TYPE, "TWS-RF");
#elif SYSTEM == 3
    strcpy(STATION_TYPE, "TWSRP");
#endif
  }

  char gps_msg_type[12];
  if (alreadyLocked && !pending_manual_gps && sync_mode != eGPSStart) {
    strcpy(gps_msg_type, "STAT_AD-C");
  } else {
    strcpy(gps_msg_type, "STAT_AD-F");
  }

  snprintf(status_response, sizeof(status_response),
           "%s,%s,%s,%s,%04d-%02d-%02dT%02d:%02d,SIM_1,%04d,%.6f,%.6f,0."
           "0\r\n",
           NETWORK, STATION_TYPE, gps_msg_type, station_name, current_year, current_month,
           current_day, current_hour, current_min, signal_strength, lati,
           longi);

  debug("Status response for GET_GPS is ");
  debugln(status_response);

  if (sms_send_text(gsm_no, status_response, 60000)) {
    msg_sent = 1;
    debugln("SUCCESS IN SENDING GPS");
  } else {
    debugln("FAILED TO SEND GPS");
  }

  vTaskDelay(1000 / portTICK_PERIOD_MS);
  SerialSIT.println("ATE1");
  waitForResponse("OK", 3000);

  if (!alreadyLocked)
    xSemaphoreGive(modemMutex); 
}

// Station ID (SMS SET_STN_ID; the keypad has its own path). wipe: the old station's records and
// backlog are removed first, as the keypad does on an ID change (they would go out under the new ID).
void set_station_id(const char* new_id, bool wipe) {
    if (new_id == NULL || new_id[0] == '\0') return;

    char clean_id[16] = {0};
    strncpy(clean_id, new_id, sizeof(clean_id) - 1);
    trim_whitespace(clean_id);

    // Canonical 6-digit zero padding for TRG / TWS numeric IDs
#if (SYSTEM == 0 || SYSTEM == 1 || SYSTEM == 2)
    if (strlen(clean_id) == 4 && isDigitStr(clean_id)) {
        char padded[16];
        snprintf(padded, sizeof(padded), "00%s", clean_id);
        strncpy(clean_id, padded, sizeof(clean_id) - 1);
    }
#endif

    if (strlen(clean_id) == 0) return;

    if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(10000)) == pdTRUE) {
        if (wipe) {
            SPIFFS.remove("/unsent.txt");
            SPIFFS.remove("/unsent_pointer.txt");
            File root = SPIFFS.open("/");
            File file = root.openNextFile();
            while (file) {
                char n[64];
                snprintf(n, sizeof(n), "%s%s", file.name()[0] == '/' ? "" : "/", file.name());
                file.close();
                const size_t len = strlen(n);
                if (len > 4 && strcmp(n + len - 4, ".txt") == 0 && strcmp(n, "/station.txt") != 0 &&
                    strcmp(n, "/rf_fw.txt") != 0 && strcmp(n, "/rf_res.txt") != 0) {
                    SPIFFS.remove(n);
                }
                file = root.openNextFile();
            }
            root.close();
        }
        File f1 = SPIFFS.open("/station.txt", FILE_WRITE);
        if (f1) { f1.print(clean_id); f1.close(); }
        File f2 = SPIFFS.open("/station.doc", FILE_WRITE);
        if (f2) { f2.print(clean_id); f2.close(); }
        xSemaphoreGive(fsMutex);
    }

    strncpy(station_name, clean_id, sizeof(station_name) - 1);
    station_name[sizeof(station_name) - 1] = '\0';
    strncpy(ftp_station, station_name, sizeof(ftp_station) - 1);
    ftp_station[sizeof(ftp_station) - 1] = '\0';

    Preferences prefs;
    prefs.begin("sys-config", false);
    prefs.putString("station", station_name);
    prefs.end();
    debugf("[CMD] Station ID set to %s%s\n", station_name, wipe ? " (old records cleared)" : "");
}

// Server 2 (SMS SET_SERVER_2). host "" = none. Kept in NVS "srv-1", read at boot.
bool server2_save(const char *host, uint16_t port, const char *path, const char *key) {
    char port_s[8];
    snprintf(port_s, sizeof(port_s), "%u", (unsigned)(port ? port : 80));
    Preferences prefs;
    if (!prefs.begin("srv-1", false)) return false;
    prefs.putString("host", host);
    prefs.putString("port", port_s);
    prefs.putString("path", path ? path : "");
    prefs.putString("key", key ? key : "");
    prefs.end();
    snprintf(secondaryServer.serverName, sizeof(secondaryServer.serverName), "%s", host);
    snprintf(secondaryServer.IP, sizeof(secondaryServer.IP), "%s", host);
    snprintf(secondaryServer.Port, sizeof(secondaryServer.Port), "%s", port_s);
    snprintf(secondaryServer.Link, sizeof(secondaryServer.Link), "%s", path ? path : "");
    if (key && key[0]) snprintf(secondaryServer.Key, sizeof(secondaryServer.Key), "%s", key);
    debugf("[CMD] Server 2: %s:%s%s\n", host[0] ? host : "(none)", port_s, path ? path : "");
    return true;
}

// Server mode (SMS SET_SERVER_MODE): 0 server 1 only, 1 server 2 only, 2 both
bool server_mode_save(int mode) {
    Preferences prefs;
    if (!prefs.begin("sys-config", false)) return false;
    const bool ok = prefs.putInt("srv_mode", mode) > 0;
    prefs.end();
    if (ok) server_mode = mode;
    debugf("[CMD] Server mode %d\n", mode);
    return ok;
}
