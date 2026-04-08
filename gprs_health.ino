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

  signal_lvl = -111; // v5.52 FIX: Default to -111 not 0
  retries = 0;

  // [v5.61] Pre-settle delay: modem returns 85 (not ready) on first 1-3 polls
  // immediately after boot. A single 400ms wait eliminates redundant retries.
  vTaskDelay(400 / portTICK_PERIOD_MS);

  int invalid_signal_count = 0;
  // rssi 0 = -113dBm. Continuous -113 is essentially no signal.
  while ((signal_lvl == -111) &&
         (retries < 60)) { // v5.85: Restored v5.67 patience (H-05 Restore)
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
    
    if (signal_lvl == -111) {
      invalid_signal_count++;
      if (invalid_signal_count >= 60) {
        debugln("[GPRS] Dead signal zone detected. Skipping long-poll wait.");
        signal_lvl = signal_strength; 
        break;
      }
    }
    debug("Signal Level is ");
    debugln(signal_lvl);
    retries++;
    vTaskDelay(500 / portTICK_PERIOD_MS);
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
    if (strstr(carrier, "Airtel")) {
      strcpy(apn_str, "airtelgprs.com"); 
      if (strlen(current_iccid) >= 6) {
        if (strncmp(current_iccid, "899116", 6) == 0 || strncmp(current_iccid, "899110", 6) == 0) {
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
      strcpy(apn_str, "airtelgprs.com"); 
    }
    return;
  }

full_discovery:
  debugln("[CACHE] New SIM or No Cache. Performing full discovery...");
  strncpy(cached_iccid, current_iccid, sizeof(cached_iccid) - 1);
  cached_iccid[sizeof(cached_iccid) - 1] = '\0';
  strcpy(sim_number, "NA");
  strcpy(carrier, "NA");
  apn_saved_this_sim = false; 

  dns_fallback_active = false;
  preferred_ftp_mode = -1;
  cached_server_ip[0] = '\0';
  cached_server_domain[0] = '\0';

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
    strcpy(carrier, "SIM OK");
    strcpy(apn_str, "airtelgprs.com");
  }

  // Get SIM identifier via IMSI (fast, always works on IoT/BSNL SIMs)
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
      debug("IMSI: ");
      debugln(sim_number);
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

    // v5.84: Adaptive Network Mode — Start with whatever mode succeeded last
    // slot. Default is Auto (2). RESCAN RULE: If on 2G for >96 slots (once
    // daily), try Auto again.
    if (last_successful_cnmp == 13)
      gprs_2g_slots_count++;
    else
      gprs_2g_slots_count = 0;

    initial_cnmp = last_successful_cnmp;
    if (gprs_2g_slots_count > 96 ||
        (initial_cnmp != 13 && initial_cnmp != 38)) {
      initial_cnmp = 2; // Force rescan or safe default
      if (gprs_2g_slots_count > 96)
        gprs_2g_slots_count = 0;
    }

    char cnmp_cmd[20];
    snprintf(cnmp_cmd, sizeof(cnmp_cmd), "AT+CNMP=%d", initial_cnmp);
    SerialSIT.println(cnmp_cmd);
    debugf("[GPRS] Adaptive Mode: %s (LastSucc:%d SlotsOn2G:%d)\n", cnmp_cmd,
           last_successful_cnmp, gprs_2g_slots_count);

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
  SerialSIT.println("AT+CNETLIGHT=0"); // Reset LED driver
  waitForResponse("OK", 500);
  SerialSIT.println("AT+CNETLIGHT=1"); // Ensure LED blinks
  waitForResponse("OK", 500);


  while (!is_registered && (retries < no_of_retries)) {
    esp_task_wdt_reset();
    last_activity_time = millis(); // v5.85: Pet the safety heartbeat during
                                   // long registration cycles

    // UI Feedback: Show progress on LCD (v5.81 Surgical: Guarded 16-char buffer)
    snprintf(reg_status, 16, "TRY #%d...", retries + 1);

    // v5.45.6: 4G-AWARE REGISTRATION POLL
    // Strategy:
    //   BSNL  [INFO] Uses CREG (2G) + CGREG (GPRS/3G). No LTE on BSNL.
    //   Airtel/Jio [INFO] Uses CEREG (4G/LTE) first, then CREG fallback.
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
      if (waitForResponse("+CEREG:", 2000)) {
        const char* resp4 = modem_response_buf;
        const char* c4_ptr = strchr(resp4, ',');
        if (c4_ptr != NULL) {
          r4 = atoi(c4_ptr + 1);

          if (r4 == 3) {
            const char* c4b_ptr = strchr(c4_ptr + 1, ','); // comma after stat field
            if (c4b_ptr != NULL) {
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
      }

      // Also check CREG as fallback for 2G/3G registration
      SerialSIT.println("AT+CREG?");
      if (waitForResponse("+CREG:", 2000)) {
        const char* resp2 = modem_response_buf;
        const char* c2_ptr = strchr(resp2, ',');
        if (c2_ptr != NULL)
          r2 = atoi(c2_ptr + 1);
      }
    } else {
      // --- BSNL Path: 2G/3G only. Check CREG + CGREG (adaptive wait below)
      // ---
      SerialSIT.println("AT+CREG?");
      if (waitForResponse("+CREG:", 2000)) {
        const char* resp2 = modem_response_buf;
        const char* c2_ptr = strchr(resp2, ',');
        if (c2_ptr != NULL)
          r2 = atoi(c2_ptr + 1);
      }
    }

    // Determine overall registration status (4G preferred)
    // r4 has already been normalized (Airtel ghost CEREG=3 [INFO] set to 0)
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
      if (!isBSNL) {
        SerialSIT.println("AT+CEREG?");
        if (waitForResponse("+CEREG:", 1000)) {
          const char* resp4x = modem_response_buf;
          const char* c4x_ptr = strchr(resp4x, ',');
          if (c4x_ptr != NULL) {
            int r4x = atoi(c4x_ptr + 1);
            if (r4x == 1 || r4x == 5) {
              is_registered = true;
              isLTE = true;
              last_successful_cnmp = 2; // 4G available, keep Auto
              strcpy(reg_status, "LTE:Home:OK");
              debugln(
                  "[GPRS] CREG(2G) but CEREG also registered. Preferring LTE.");
              break;
            }
          }
        }
      }
      is_registered = true;
      isLTE = false;
      last_successful_cnmp = 13; // v5.84: Mark GSM success
      strcpy(reg_status, (r2 == 1) ? "GSM:Home:OK" : "GSM:Roam:OK");
      debugf("[GPRS] Registered via CREG! (2G:%d)\n", r2);
      break;
    }
    if (r4 == 1 || r4 == 5) {
      is_registered = true;
      isLTE = true;
      last_successful_cnmp = 2; // v5.84: Mark Auto/LTE success (CNMP=2 is safer
                                // for future 2G fallback)
      strcpy(reg_status, (r4 == 1) ? "LTE:Home:OK" : "LTE:Roam:OK");
      debugf("[GPRS] Registered via CEREG! (4G:%d)\n", r4);
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
        if (isBSNL) {
          // v5.85 BSNL Strategy: Lock to GSM-only (CNMP=13) and Re-scan.
          // BSNL 4G (Mode 38) is virtually non-existent; hunting for it wastes
          // battery/time.
          if (initial_cnmp != 13) {
            debugln(
                "[GPRS] BSNL Tier1 @ iter5: Locking to GSM-only (CNMP=13)...");
            SerialSIT.println("AT+CNMP=13");
          } else {
            debugln("[GPRS] BSNL Tier1 @ iter5: Already in GSM-only. Forcing "
                    "fresh scan (COPS=0)...");
            SerialSIT.println("AT+COPS=0");
          }
        } else {
          // v5.84 Airtel/Jio Strategy: Toggle Mode if stalled.
          if (initial_cnmp != 13) {
            debugln("[GPRS] Tier1 @ iter5: Auto mode stalled. Fallback to "
                    "GSM-only (CNMP=13)...");
            SerialSIT.println("AT+CNMP=13");
          } else {
            debugln("[GPRS] Tier1 @ iter5: GSM-only stalled. Peeking Auto "
                    "(CNMP=2)...");
            SerialSIT.println("AT+CNMP=2");
          }
        }
        waitForResponse("OK", 1000);
        if (isBSNL)
          SerialSIT.println(
              "AT+COPS=0"); // Ensure BSNL probes for a fresh 2G cell
        else
          SerialSIT.println("AT+COPS=0");
        esp_task_wdt_reset();
        waitForResponse("OK", 3000);
        SerialSIT.println("AT+CGATT=1");
        waitForResponse("OK", 3000);
      } else if (retries == 10) {
        // v5.84 Tier 2 @ iter 10: Full Radio Reset & SIM Scrub
        debugln("[GPRS] Tier2 @ iter10: Radio-Off SIM Scrub...");
        SerialSIT.println("AT+CFUN=0");
        esp_task_wdt_reset();
        waitForResponse("OK", 5000);
        SerialSIT.println(
            "AT+CRSM=214,28539,0,0,12,\"FFFFFFFFFFFFFFFFFFFFFFFF\"");
        waitForResponse("OK", 2000);
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
                 23) { // [C-01] Tier 4 aligned for final 24th attempt
        // Tier 4 @ iter 23 (Final Attempt): Restore Auto-Mode + COPS auto
        debugln("[GPRS] Tier4 @ iter23: Restoring Auto-Mode (CNMP=2, COPS=0) "
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
      SerialSIT.println("AT+COPS=0");
      esp_task_wdt_reset();
      waitForResponse("OK", 5000);
    }

    debugf("Reg Search [BSNL:%d]... Status:%d Iter:#%d/%d\n", isBSNL,
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
    debugln("Registration Successful.");
  } else {
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

  active_cid = 0;
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
      strcpy(apn_str, fallback_apns[i]);
      save_apn_config(fallback_apns[i], cached_iccid);
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

void process_sms(char msg_no) {
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
  waitForResponse("OK", 10000);

  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CMGR=%d", msg_no);
  SerialSIT.println(gprs_xmit_buf);
  if (waitForResponse("OK", 6000)) {
    const char *response_char = modem_response_buf;

    if (strstr(response_char, "REC UNREAD") && strstr(response_char, "VARSHA")) {
      debug("SMS: Valid command (VARSHA) received: ");
      debugln(response_char);
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
      prepare_and_send_status(msg_rcvd_number, true);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    } else if (strstr(response_char, "GET_GPS")) {
      get_lat_long_date_time(msg_rcvd_number,
                             true); // modemMutex held by send_sms() loop
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
          copyFromSPIFFSToFS(requestedDate, true); // temp1 is date
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
          fetchFromHttpAndUpdate(temp1, true);
        }
      }

    } else {

      // debug("No message recd ");
      // debugln(response);
      }
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
  }
}

void prepare_and_send_status(char *gsm_no, bool alreadyLocked) {
  if (!alreadyLocked) {
    if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(10000)) != pdTRUE) {
      debugln("[SMS] FAILED: Modem Busy. Deferring Status request.");
      // Find the active LCD field to show busy status
      for (int i = 0; i < FLD_COUNT; i++) {
        if (i == FLD_SEND_STATUS || i == FLD_SEND_HEALTH) {
          strcpy(ui_data[i].bottomRow, "MODEM BUSY      ");
          show_now = 1;
        }
      }
      return;
    }
  }

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

  // li_bat ADC reads handled inside get_calibrated_battery_voltage
  li_bat_val =
      get_calibrated_battery_voltage(); // Phase 8 Fix: eFuse-calibrated ADC
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
  if (waitForResponse(">", 15000)) {
    debugln(" Received!");
    flushSerialSIT(); // v5.81: Ensure clean UART pipe for Ctrl+Z termination
    SerialSIT.print(status_response); 
    debug("Waiting for +CMGS confirmation...");
    if (waitForResponse("+CMGS:", 35000)) {
       debugln(" Done.");
       debug("Response of AT+CMGS is ");
       debugln(modem_response_buf);
       msg_sent = 1;
       debugln("SUCCESS IN SENDING MSG");
    } else {
       debugln(" Done.");
       debug("Response of AT+CMGS is ");
       debugln(modem_response_buf);
       debugln("FAILED TO SEND MSG - No +CMGS in response");
    }
  } else {
    debugln(" TIMEOUT! No '>' prompt received.");
  }

  if (!alreadyLocked)
    xSemaphoreGive(modemMutex);
}

void get_gps_coordinates() {
  if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(10000)) != pdTRUE) {
    debugln("[GPS] Error: Modem Mutex Timeout - skipping GPS request");
    return;
  }

  int tmp;
  double lat, lon;
  const char *response_ptr;
  char *csqstr;

  for (int retry = 0; retry < 2; retry++) {
    debugf1("[GPS] Requesting Coordinates (AT+CLBS=1), Attempt %d...\n",
            retry + 1);
    SerialSIT.println("ATE0");
    waitForResponse("OK", 2000);

    SerialSIT.println("AT+CLBS=1");
    if (waitForResponse("+CLBS:", 15000)) {
      const char* response_ptr = modem_response_buf;
      const char* csqstr = strstr(response_ptr, "+CLBS");
      if (csqstr != NULL) {
        if (sscanf(csqstr, "+CLBS: %d,%lf,%lf,", &tmp, &lat, &lon) >= 3) {
          if (fabs(lat) > 0.00001 && fabs(lon) > 0.00001) {
            lati = lat;
            longi = lon;
            saveGPS(); // Persist immediately
            xSemaphoreGive(modemMutex);
            return; // SUCCESS
          }
        }
      }
    }
    debugln("[GPS] Attempt failed. Retrying...");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
  debugln("[GPS] All attempts to get fresh coordinates failed.");
  xSemaphoreGive(modemMutex);
}

void get_lat_long_date_time(char *gsm_no, bool alreadyLocked) {
  if (!alreadyLocked) {
    if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(10000)) != pdTRUE) {
      debugln("[GPS] FAILED: Modem Busy. Deferring GPS request.");
      strcpy(ui_data[FLD_SEND_GPS].bottomRow, "MODEM BUSY      ");
      show_now = 1;
      return;
    }
  }

  int response_no;
  int tmp, tmp3;
  char tmp2[16];
  int day, month, year, hour, minute, seconds;
  char status_response[256];
  char gprs_xmit_buf[300];
  msg_sent = 0;

  char *csqstr;

  SerialSIT.println("ATE0");
  if (waitForResponse("OK", 3000)) {
    debugln("HTTP response of ATE0: OK");
  }

  // To find Latitude and Longitude. Only with A7672S
  vTaskDelay(5000 / portTICK_PERIOD_MS);
  SerialSIT.println("AT+CLBS=1");
  if (waitForResponse("+CLBS:", 15000)) {
    const char* csqstr = strstr(modem_response_buf, "+CLBS");
    if (csqstr != NULL) {
      sscanf(csqstr, "+CLBS: %d,%lf,%lf,", &tmp, &lati, &longi);
      debugf("Latitude is : %.6f\n", lati);
      debugf("Longitude is : %.6f\n", longi);
      if (lati != 0 && longi != 0) {
        saveGPS();
      } 
    }
  } 

  snprintf(status_response, sizeof(status_response),
           "%s,%s,STAT_AD-C,%s,%04d-%02d-%02dT%02d:%02d,SIM_1,%04d,%.6f,%.6f,0."
           "0\r\n\032",
           NETWORK, STATION_TYPE, station_name, current_year, current_month,
           current_day, current_hour, current_min, signal_strength, lati,
           longi);

  debug("Status response for GET_GPS is ");
  debugln(status_response);

  SerialSIT.println("AT+CMGF=1");
  waitForResponse("OK", 5000);

  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CMGS=\"%s\"\r", gsm_no);
  SerialSIT.println(gprs_xmit_buf);
  debug("Waiting for '>' prompt...");
  if (waitForResponse(">", 15000)) {
    debugln(" Received!");
    flushSerialSIT();
    SerialSIT.print(status_response);
    debug("Waiting for +CMGS confirmation...");
    if (waitForResponse("+CMGS:", 35000)) {
      debugln(" Done.");
      debug("Response of AT+CMGS is ");
      debugln(modem_response_buf);
      msg_sent = 1;
      debugln("SUCCESS IN SENDING GPS");
    } else {
      debugln(" Done.");
      debugln("FAILED TO SEND GPS - No +CMGS in response");
    }
  } else {
    debugln(" TIMEOUT! No '>' prompt received.");
  }

  vTaskDelay(1000 / portTICK_PERIOD_MS);
  SerialSIT.println("ATE1");
  waitForResponse("OK", 3000);

  if (!alreadyLocked)
    xSemaphoreGive(modemMutex); 
}

// Proposed Rule 45: The Header-Health Check
// Checks if the buffer contains "Modem-speak" or invalid ESP32 entry points
bool send_health_report(bool useJitter) {
#if ENABLE_HEALTH_REPORT == 1
  if (schedulerBusy) {
    debugln("[Health] Deferring: scheduler mid-write.");
    return false; // will retry next slot
  }

  if (strstr(carrier, "BSNL") || strstr(carrier, "bsnl"))
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  else
    vTaskDelay(100 / portTICK_PERIOD_MS);

  if (schedulerBusy) {
    debugln("[Health] Deferring: scheduler activated during delay.");
    return false;
  }

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

  if (strstr(carrier, "Airtel") && strstr(apn_str, "airteliot")) {
    debugln("[Health] Skipping: Airtel M2M SIM + foreign health server. "
            "Request IP whitelist from Airtel.");
    xSemaphoreGive(modemMutex);
    xSemaphoreGive(fsMutex);
    return false;
  }

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

  if (diag_temp_cv)
    H_FAULT("TEMP_STUCK");
  if (diag_hum_cv)
    H_FAULT("HUM_STUCK");
  if (diag_ws_cv)
    H_FAULT("WS_STUCK");
  if (diag_temp_erv || diag_temp_erz)
    H_FAULT("TEMP_UNREAL");
  if (diag_hum_erv || diag_hum_erz)
    H_FAULT("HUM_UNREAL");
  if (diag_ws_erv)
    H_FAULT("WS_UNREAL");
  if (diag_wd_fail)
    H_FAULT("WD_FAIL");
  if (diag_rain_jump)
    H_FAULT("RAIN_SPIKE");
  if (diag_rain_reset)
    H_FAULT("RAIN_RESET");
  if (diag_rain_calc_invalid)
    H_FAULT("RAIN_CALC");

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
  if (abs(lati) < 0.00001 && abs(longi) < 0.00001)
    snprintf(gps_str, sizeof(gps_str), "NA");
  else
    snprintf(gps_str, sizeof(gps_str), "%.8f,%.8f", lati, longi);

  int spiffs_used = SPIFFS.usedBytes() / 1024;
  int spiffs_total = SPIFFS.totalBytes() / 1024;

  int unsent_count = countStored("/unsent.txt") + countStored("/ftpunsent.txt");

  char calib_report[48] = "NA";
  if (calib_year > 2000) {
    snprintf(calib_report, sizeof(calib_report), "CLB-%s (%04d-%02d-%02d)",
             (calib_sts == 1 ? "OK" : "FAIL"), calib_year, calib_month,
             calib_day);
  }

  char feedback[128] = "";
  if (last_cmd_id > 0) {
    snprintf(feedback, sizeof(feedback),
             ",\"last_cmd_id\":%d,\"last_cmd_res\":\"%s\"", last_cmd_id,
             last_cmd_res);
  }

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
      "\"calib\":\"%s\"," 
      "\"ndm_cnt\":%d,\"pd_cnt\":%d,"
      "\"http_present_fails\":%d,\"http_cum_fails\":%d,"
      "\"http_backlog_cnt\":%d,\"mutex_fail\":%d,"
      "\"ota_fails\":%d,\"ota_fail_reason\":\"%s\""
      "%s}", 
      cleanStn, UNIT, SYSTEM, h_status, sensor_info,
      (diag_rtc_battery_ok ? 1 : 0), li_bat_val, solar_val, signal_lvl,
      diag_net_data_count,          
      diag_http_success_count,      
      diag_http_retry_count,        
      diag_ftp_success_count,       
      diag_net_data_count_prev,     
      diag_pd_count_prev,           
      diag_http_success_count_prev, 
      diag_http_retry_count_prev,   
      diag_ftp_success_count_prev,  
      diag_gprs_fails, diag_reg_fail_type, 
      diag_last_reset_reason,              
      spiffs_used, spiffs_total,           
      unsent_count,                        
      UNIT_VER, cached_iccid, carrier, gps_str,
      diag_cdm_status, 
      calib_report,    
      diag_ndm_count, diag_pd_count, diag_http_present_fails,
      diag_http_cum_fails, get_total_backlogs(true), diag_modem_mutex_fails,
      ota_fail_count, ota_fail_reason,
      feedback); 

  if (msgLen >= (int)sizeof(jsonBody)) {
    Serial.printf("[Health] WARNING: JSON truncated (%d > %d). Clamping.\n",
                  msgLen, (int)sizeof(jsonBody));
    msgLen = sizeof(jsonBody) - 1; 
    jsonBody[msgLen] = '\0';
    char *last_comma = strrchr(jsonBody, ',');
    if (last_comma && (jsonBody + msgLen - last_comma) < 5) {
      *last_comma = '}'; 
      *(last_comma + 1) = '\0';
      msgLen = last_comma - jsonBody + 1;
    }
  }

  xSemaphoreGive(fsMutex); 

  if (useJitter)
    vTaskDelay((esp_random() % 5000) /
               portTICK_PERIOD_MS); 
  else
    vTaskDelay(2000 / portTICK_PERIOD_MS);

  bool success = false;
  int max_attempts = useJitter ? 3 : 2; 
  for (int attempt = 1; attempt <= max_attempts; attempt++) {
    debugf2("[Health] Attempt %d/%d\n", attempt, max_attempts);
    if (!verify_bearer_or_recover())
      continue;

    SerialSIT.println("AT+CGEREP=0");
    waitForResponse("OK", 1000);

    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 5000);

    if (strstr(carrier, "BSNL") || strstr(carrier, "bsnl")) {
      vTaskDelay(8000 / portTICK_PERIOD_MS);
    } else {
      vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    flushSerialSIT();

    SerialSIT.println("AT+HTTPINIT");
    if (!waitForResponse("OK", 5000)) {
      debugln("[Health] [ERR] HTTPINIT Failed. Bearer Nuke...");
      SerialSIT.println("AT+CGACT=0,1");
      waitForResponse("OK", 5000);
      continue;
    }

    bool step_fail = false;
    char ht_url[150];
    snprintf(ht_url, sizeof(ht_url), "AT+HTTPPARA=\"URL\",\"http://%s:%s%s\"",
             HEALTH_SERVER_IP, HEALTH_SERVER_PORT, HEALTH_SERVER_PATH);
    SerialSIT.println(ht_url);
    waitForResponse("OK", 2000);

    SerialSIT.println("AT+HTTPPARA=\"CID\",1");
    waitForResponse("OK", 1000);

    SerialSIT.println("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
    waitForResponse("OK", 1000);
    SerialSIT.println("AT+HTTPPARA=\"ACCEPT\",\"*/*\"");
    waitForResponse("OK", 1000);

    if (!step_fail) {
      debugf1("[Health] Payload size: %d bytes\n", msgLen);

      flushSerialSIT();

      char ht_data_cmd[64];
      snprintf(ht_data_cmd, sizeof(ht_data_cmd), "AT+HTTPDATA=%d,15000",
               msgLen);
      SerialSIT.println(ht_data_cmd);

      if (waitForResponse("DOWNLOAD", 25000)) {
        vTaskDelay(500 / portTICK_PERIOD_MS);

        int sentBytes = 0;
        while (sentBytes < msgLen) {
          int toWrite = min(48, msgLen - sentBytes);
          SerialSIT.write(jsonBody + sentBytes, toWrite);
          sentBytes += toWrite;
          esp_task_wdt_reset(); 
          vTaskDelay(20 / portTICK_PERIOD_MS);
        }

        if (waitForResponse("OK", 20000)) {
          SerialSIT.println("AT+HTTPACTION=1");
          waitForResponse("OK", 3000);

          if (waitForResponse("+HTTPACTION:", 45000)) {
            const char* act_ptr = strstr(modem_response_buf, "+HTTPACTION:");
            debugf("[Health] Resp: %s\n", act_ptr);

            if (strstr(act_ptr, "200") != NULL) {
              success = true;
            } else if (strstr(act_ptr, "714") != NULL || strstr(act_ptr, "706") != NULL) {
              debugln("[Health] 🧟 Zombie Socket (714/706). Nuking Bearer Context...");
              SerialSIT.println("AT+HTTPTERM");
              waitForResponse("OK", 2000);
              SerialSIT.println("AT+CGACT=0,1");
              waitForResponse("OK", 5000);
              vTaskDelay(5000 / portTICK_PERIOD_MS);
              continue; 
            }
            vTaskDelay(500 / portTICK_PERIOD_MS);
            SerialSIT.println("AT+HTTPREAD=0,512");
            waitForResponse("+HTTPREAD:", 10000);
            const char* body = modem_response_buf;
            debugf("[Health] Body: %s\n", body);

            const char* idTag = strstr(body, "\"id\"");
            if (idTag != NULL) {
              const char* valStart = strchr(idTag, ':');
              if (valStart != NULL) {
                last_cmd_id = atoi(valStart + 1);
                strcpy(last_cmd_res, "PENDING"); 
              }
            }

            sync_rtc_from_server_tm(body, false);
            if (strstr(body, "\"REBOOT\"") != NULL) {
              force_reboot = true;
              strcpy(last_cmd_res, "Success: Rebooting");
            }
            if (strstr(body, "\"OTA_CHECK\"") != NULL) {
              force_ota = true;
              const char* pTag = strstr(body, "\"p\"");
              if (pTag == NULL)
                pTag = strstr(body, "\"cmd_param\"");
              if (pTag != NULL) {
                const char* valStart = strchr(pTag, ':');
                if (valStart != NULL) {
                  const char* q1 = strchr(valStart, '\"');
                  if (q1 != NULL) {
                    const char* q2 = strchr(q1 + 1, '\"');
                    if (q2 != NULL) {
                      int len = q2 - (q1 + 1);
                      if (len > 0 && len < (int)sizeof(ota_cmd_param)) {
                        strncpy(ota_cmd_param, q1 + 1, len);
                        ota_cmd_param[len] = '\0';
                      }
                    }
                  }
                }
              }
            }
            if (strstr(body, "\"FTP_BACKLOG\"") != NULL)
              force_ftp = true;
            if (strstr(body, "\"FTP_DAILY\"") != NULL) {
              force_ftp_daily = true;
              const char* pTag = strstr(body, "\"p\"");
              if (pTag != NULL) {
                const char* valStart = strchr(pTag, ':');
                if (valStart != NULL) {
                  const char* q1 = strchr(valStart, '\"');
                  if (q1 != NULL) {
                    const char* q2 = strchr(q1 + 1, '\"');
                    if (q2 != NULL) {
                      int len = q2 - (q1 + 1);
                      if (len > 0 && len < (int)sizeof(ftp_daily_date)) {
                        strncpy(ftp_daily_date, q1 + 1, len);
                        ftp_daily_date[len] = '\0';
                      }
                    }
                  }
                }
              }
            }
            if (strstr(body, "\"GET_GPS\"") != NULL)
              force_gps_refresh = true;
            if (strstr(body, "\"CLEAR_FTP_QUEUE\"") != NULL)
              force_clear_ftp_queue = true;
            if (strstr(body, "\"DELETE_DATA\"") != NULL)
              force_delete_data = true;

            if (strstr(body, "\"INTERVAL\"") != NULL) {
              const char* pTag = strstr(body, "\"p\"");
              if (pTag != NULL) {
                const char* valStart = strchr(pTag, ':');
                if (valStart != NULL) {
                  int mins = atoi(valStart + 1);
                  if (mins > 0) {
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
          }

          SerialSIT.println("AT+HTTPTERM");
          waitForResponse("OK", 1000);
          break; // Success exit
        } else {
          debugf("[Health] [ERR] HTTP Action Failed, Resp: %s\n", modem_response_buf);
        }
      } else {
        debugln("[Health] [ERR] Data Load Timeout/Error");
      }
    } else {
      debugln("[Health] [ERR] DOWNLOAD Prompt Failed");
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
#else
  debugln("[Health] Reporting Disabled (v5.72 Hardened).");
  return true;
#endif
}


/*
 *   GRACEFUL REBOOT (v5.58 Fix)
 */
