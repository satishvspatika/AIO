
#include "globals.h"

// Needed for SerialSIT usage if not included
extern HardwareSerial SerialSIT;
// debugln/debug are macros in globals.h
extern String waitForResponse(const char *expected, unsigned long timeout);

void save_apn_config(String apn, String ccid) {
  if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(2000)) == pdTRUE) {
    File f = SPIFFS.open("/apn_config.txt", FILE_WRITE);
    if (f) {
      f.println(ccid);
      f.println(apn);
      f.close();
      debugln("Saved APN Config: " + apn);
    }
    xSemaphoreGive(fsMutex);
  } else {
    debugln("APN Save Failed: fsMutex Locked");
  }
}

bool load_apn_config(String current_ccid, char *target_apn, size_t max_len) {
  if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(15000)) != pdTRUE) { // v5.72: Increased for heavy daily logs
    debugln("APN Load Failed: fsMutex Locked");
    return false;
  }

  if (!SPIFFS.exists("/apn_config.txt")) {
    xSemaphoreGive(fsMutex);
    return false;
  }
  File f = SPIFFS.open("/apn_config.txt", FILE_READ);
  if (!f) {
    xSemaphoreGive(fsMutex);
    return false;
  }

  // Use fixed stack buffers to avoid Heap fragmentation/OOM on bad file
  char ccid_buf[50] = {0};
  char apn_buf[50] = {0};

  // Safe Read Line 1 (CCID)
  size_t i = 0;
  while (f.available() && i < sizeof(ccid_buf) - 1) {
    char c = f.read();
    if (c == '\n' || c == '\r')
      break;
    ccid_buf[i++] = c;
  }
  ccid_buf[i] = '\0';

  // Consume remaining newline chars if any
  while (f.available()) {
    char c = f.peek();
    if (c == '\n' || c == '\r')
      f.read();
    else
      break;
  }

  // Safe Read Line 2 (APN)
  i = 0;
  while (f.available() && i < sizeof(apn_buf) - 1) {
    char c = f.read();
    if (c == '\n' || c == '\r')
      break;
    apn_buf[i++] = c;
  }
  apn_buf[i] = '\0';

  f.close();

  debug("Memory Check - Stored CCID: ");
  debug(ccid_buf);
  debug(" | Stored APN: ");
  debugln(apn_buf);

  // Convert to String only for comparison (safe short strings)
  String stored_ccid_str = String(ccid_buf);
  stored_ccid_str.trim();
  String stored_apn_str = String(apn_buf);
  stored_apn_str.trim();

  // Validate CCID match
  if (stored_ccid_str.length() > 5 &&
      current_ccid.indexOf(stored_ccid_str) != -1 &&
      stored_apn_str.length() > 0) {

    // v5.65 CRITICAL FIX: Cross-check stored APN against current carrier.
    // A stale cache (e.g. from a test with Airtel SIM) can store airteliot.com
    // for a BSNL SIM's ICCID. If the carrier (set by get_network() via CSPN/COPS)
    // disagrees with the stored APN, REJECT the cache and force fresh discovery.
    bool apn_carrier_mismatch = false;
    if (strstr(carrier, "BSNL") && strstr(stored_apn_str.c_str(), "airtel")) {
      apn_carrier_mismatch = true;
    } else if (strstr(carrier, "Jio") && !strstr(stored_apn_str.c_str(), "jio")) {
      apn_carrier_mismatch = true;
    } else if ((strstr(carrier, "Airtel")) && strstr(stored_apn_str.c_str(), "bsnl")) {
      apn_carrier_mismatch = true;
    }

    if (apn_carrier_mismatch) {
      debugln("Smart APN: ⚠ MISMATCH — Stored APN conflicts with detected carrier. Ignoring cache.");
      // Delete the stale file so it gets re-saved with the correct APN this cycle
      SPIFFS.remove("/apn_config.txt");
      xSemaphoreGive(fsMutex);
      return false;
    }

    strncpy(target_apn, stored_apn_str.c_str(), max_len);
    target_apn[max_len - 1] = '\0';
    debugln("Smart APN: Match Found!");
    xSemaphoreGive(fsMutex);
    return true;
  }
  debugln("Smart APN: No Match.");
  xSemaphoreGive(fsMutex);
  return false;
}

String get_ccid() {
  flushSerialSIT();
  String ccid = "";

  // v7.11: Rule 23 - Verify SIM is ready before CCID fetch
  for (int i = 0; i < 5; i++) {
    SerialSIT.println("AT+CPIN?");
    if (waitForResponse("READY", 2000).indexOf("READY") != -1)
      break;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  // Attempt 1: Try AT+CICCID (Primary for A7672)
  flushSerialSIT();
  vTaskDelay(200 / portTICK_PERIOD_MS);
  SerialSIT.println("AT+CICCID");
  String response = waitForResponse("OK", 3000);
  String resp = response;
  debug("Raw CICCID Resp: ");
  debugln(resp);

  // Flexible Header Check: Look for either +CICCID: or +ICCID:
  int startIdx = resp.indexOf("+CICCID:");
  int headerLen = 8;
  if (startIdx == -1) {
    startIdx = resp.indexOf("+ICCID:");
    headerLen = 7;
  }

  if (startIdx != -1) {
    for (int i = startIdx + headerLen; i < resp.length(); i++) {
      if (isdigit(resp[i]))
        ccid += resp[i];
      else if (ccid.length() > 0)
        break;
    }
  }

  // Attempt 2: Fallback to AT+CCID
  if (ccid.length() < 10) {
    ccid = ""; // Reset
    flushSerialSIT();
    vTaskDelay(300 / portTICK_PERIOD_MS); // Extra settle time on stressed modem
    SerialSIT.println("AT+CCID");
    // v5.53 fix: modem may respond with +ICCID: (not +CCID:) when stressed.
    // Wait for "OK" to capture the full response regardless of URC prefix.
    String response = waitForResponse("OK", 3000);
    String resp = response;
    debug("Raw CCID Resp: ");
    debugln(resp);
    // Check BOTH +CCID: and +ICCID: in the response
    startIdx = resp.indexOf("+CCID:");
    int hdLen2 = 6;
    if (startIdx == -1) {
      startIdx = resp.indexOf("+ICCID:");
      hdLen2 = 7;
    }
    if (startIdx != -1) {
      for (int i = startIdx + hdLen2; i < resp.length(); i++) {
        if (isdigit(resp[i]))
          ccid += resp[i];
        else if (ccid.length() > 0)
          break;
      }
    }
  }

  debug("Final CCID parsed: ");
  debugln(ccid);
  return ccid;
}

bool try_activate_apn(const char *apn) {
  if (apn == NULL || strlen(apn) == 0)
    return false;

  char gprs_xmit_buf[300];
  debug("Trying APN: ");
  debugln(apn);

  // v5.70: Fix C-01 - Prevent redundant APN writes to modem flash to preserve endurance
  // We read the current definition for CID 1 and only update if it differs.
  SerialSIT.println("AT+CGDCONT?");
  String current_apn_resp = waitForResponse("OK", 2000);
  String target_apn_quoted = "\"" + String(apn) + "\"";
  
  if (current_apn_resp.indexOf("+CGDCONT: 1,") != -1 && 
     (current_apn_resp.indexOf(target_apn_quoted) != -1 || current_apn_resp.indexOf(apn) != -1)) {
    debugln("[APN] Flash match found. Skipping redundant write.");
  } else {
    debugln("[APN] Flash mismatch or missing. Updating modem profile...");
    if (strcmp(apn, "jionet") == 0) {
      snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
               "AT+CGDCONT=1,\"IPV4V6\",\"%s\"", apn);
    } else {
      snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CGDCONT=1,\"IP\",\"%s\"",
               apn);
    }
    SerialSIT.println(gprs_xmit_buf);
    waitForResponse("OK", 3000);
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }

  flushSerialSIT(); // v5.79: Clear any "PB DONE" boot noise before command
  SerialSIT.println("AT+CGACT=1,1");
  
  // v5.79 Robust Response Wait: Filter async URCs (PB DONE, SMS DONE)
  String response = "";
  unsigned long start = millis();
  while (millis() - start < 25000) {
    if (SerialSIT.available()) {
      String line = SerialSIT.readStringUntil('\n');
      line.trim();
      if (line.equalsIgnoreCase("OK")) { response = "OK"; break; }
      if (line.indexOf("ERROR") != -1) { response = line; break; }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();
  }
  
  debugln("CGACT Resp: " + response); 

  if (response.indexOf("OK") != -1) {
    // v5.70 Hardened [H-4 Fix]: Verify actual IP assigned — BSNL returns OK but 0.0.0.0
    vTaskDelay(500 / portTICK_PERIOD_MS);
    SerialSIT.println("AT+CGPADDR=1");
    String ip_resp = waitForResponse("OK", 3000);
    if (ip_resp.indexOf("0.0.0.0") != -1 || ip_resp.indexOf("+CGPADDR:") == -1) {
        debugln("[APN] CGACT OK but no valid IP (0.0.0.0). Treating as failure.");
        return false;
    }
    return true;
  }
  return false;
}

bool verify_bearer_or_recover() {
  // v5.78 Hardening [P4]: Delay recovery if 15-min interval logging is active
  if (sleep_sequence_active) {
    debugln("[RECOVERY] Delaying bearer recovery for 10s to protect interval logging window...");
    for (int i = 0; i < 10; i++) {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      esp_task_wdt_reset();
    }
  }

  bearer_recovery_active = true; // Block UI/I2C tasks from interrupting
  flushSerialSIT();

  // 1. Determine which CID is active (Support CID 1, 5, or 8)
  // Use the global active_cid if set, otherwise query.
  int check_cid = (active_cid > 0) ? active_cid : 1;

  SerialSIT.println("AT+CGACT?");
  String cgact_resp = waitForResponse("OK", 3000);

  bool context_active = false;
  // Check for the specific CID that we configured
  String cid_check = "+CGACT: " + String(check_cid) + ",1";
  if (cgact_resp.indexOf(cid_check) != -1) {
    context_active = true;
  }

  // 2. If context is active, verify assigned IP is valid AND APN matches
  if (context_active) {
    String paddr_cmd = "AT+CGPADDR=" + String(check_cid);
    SerialSIT.println(paddr_cmd);
    String ip_resp = waitForResponse("OK", 3000);

    String paddr_prefix = "+CGPADDR: " + String(check_cid) + ",";
    
    // v5.70: D-2 Hardened IP verification
    String ip_str = "";
    int prefix_idx = ip_resp.indexOf(paddr_prefix);
    if (prefix_idx != -1) {
        int comma_idx = ip_resp.indexOf(',', prefix_idx);
        if (comma_idx != -1) {
            ip_str = ip_resp.substring(comma_idx + 1);
            ip_str.replace("\"", ""); // Remove quotes
            ip_str.trim();
        }
    }

    bool ip_ok = (ip_str.length() >= 7 && ip_str.indexOf("0.0.0.0") == -1 && ip_str.indexOf(".") != -1);

    if (ip_ok) {
        debugf1("Bearer Check: OK (Assigned IP: %s)\n", ip_str.c_str());
      // v5.52: Extra check - is the APN actually what we expect?
      // Only check if we have a known target APN (avoids breaking new SIMs)
      if (strlen(apn_str) > 0) {
        String rdp_cmd = "AT+CGCONTRDP=" + String(check_cid);
        SerialSIT.println(rdp_cmd);
        String rdp_resp = waitForResponse("OK", 3000);
        // P1 fix v5.65: '"' + String(apn_str) is char+String = wrong!
        // char '"' (ASCII 34) + String [INFO] appends integer 34, not a quote character.
        // Must use string literals for correct concatenation.
        String apn_check = "\"" + String(apn_str) + "\"";
        if (rdp_resp.indexOf(apn_check) == -1) {
          debugln("Bearer Check: FAILED (APN Mismatch/Ghost Session). Force "
                  "re-activating...");
          String deact_cmd = "AT+CGACT=0," + String(check_cid);
          SerialSIT.println(deact_cmd);
          waitForResponse("OK", 3000);
          // Fall through to recovery with the correct apn_str
          goto bearer_recovery;
        }
      }
      debugln("Bearer Check: OK (Context Active & Valid IP/APN)");
      bearer_recovery_active = false;
      return true;
    } else {
      debugln("Bearer Check: FAILED (Context Active but Invalid IP). "
              "Recovering...");
      // Context is active but no IP - deactivate cleanly
      String deact_cmd = "AT+CGACT=0," + String(check_cid);
      SerialSIT.println(deact_cmd);
      waitForResponse("OK", 2000);
    }
  } else {
    debugln("Bearer Check: FAILED (Context Not Active). Recovering...");
  }

bearer_recovery: // Label used for ghost-session fallthrough
  // Clear serial buffers explicitly before fetching CCID to drop dirty URCs
  vTaskDelay(500 / portTICK_PERIOD_MS);
  flushSerialSIT();

  // v5.53: CRITICAL — Check if modem is still registered BEFORE trying APN.
  // If deregistered (+CREG: 0), PDP activation will always fail.
  // v7.13: Increased wait to 60 seconds for cold 2G attach.
  {
    bool is_registered = false;
    debugln("AG Recovery: Checking network registration before APN attempt...");
    for (int r = 0; r < 30; r++) { // 30 * 2s = 60s
      SerialSIT.println("AT+CREG?");
      String creg = waitForResponse("OK", 2000);
      // Registered: ,1 (home) or ,5 (roaming)
      if (creg.indexOf(",1") != -1 || creg.indexOf(",5") != -1) {
        is_registered = true;
        break;
      }
      debugln("AG Recovery: Not registered yet, waiting...");
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      esp_task_wdt_reset();
    }
    if (!is_registered) {
      debugln("AG Recovery: Modem deregistered after 60s wait. Aborting.");
      bearer_recovery_active = false;
      return false;
    }
    // v7.13: Rule 26 - SIM-Attach Stabilization (The 5-Second Settle)
    debugln(
        "AG Recovery: Registered. Waiting 5s for SIM profile stabilization...");
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }

  // 3. Bearer is dead or invalid. Attempt recovery using current runtime APN
  // first. This avoids querying SIM CCID on a stressed modem.
  if (strlen(apn_str) > 0) {
    debug("AG Recovery: Attempting runtime APN -> ");
    debugln(apn_str);
    if (try_activate_apn(apn_str)) {
      bearer_recovery_active = false;
      return true;
    }
  }

  // 4. Fallback: Query CCID and load stored APN from SPIFFS.
  debugln("AG Recovery: Runtime APN failed. Querying CCID...");
  String ccid = get_ccid();
  char stored_apn[32] = {0}; // Turner-Fix: Expanded from 20 to 32 to support long IoT APNs
  if (load_apn_config(ccid, stored_apn, sizeof(stored_apn))) {
    debugf("AG Recovery: Trying stored APN -> %s\n", stored_apn);
    if (try_activate_apn(stored_apn)) {
      strncpy(apn_str, stored_apn, sizeof(apn_str) - 1);
      apn_str[sizeof(apn_str) - 1] = '\0';
      bearer_recovery_active = false;
      return true;
    }
  }

  // v7.10: Rule 22 - Nuclear Fallback (Radio Refresh)
  debugln(
      "AG Recovery: ALL ACTIVATION ATTEMPTS FAILED. Performing Radio Refresh "
      "(CFUN=0/1)...");
  SerialSIT.println("AT+CFUN=0");
  waitForResponse("OK", 5000);
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  SerialSIT.println("AT+CFUN=1");
  waitForResponse("OK", 5000);

  // v7.11: Wait for SIM Readiness after Radio Refresh
  debugln("AG Recovery: Waiting for SIM (CPIN) stability...");
  for (int i = 0; i < 5; i++) {
    SerialSIT.println("AT+CPIN?");
    if (waitForResponse("READY", 2000).indexOf("READY") != -1)
      break;
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  // Wait for re-registration after radio refresh
  debugln("AG Recovery: Waiting for re-registration after refresh...");
  for (int r = 0; r < 10; r++) {
    SerialSIT.println("AT+CREG?");
    String creg = waitForResponse("OK", 2000);
    if (creg.indexOf(",1") != -1 || creg.indexOf(",5") != -1) {
      debugln("AG Recovery: Re-registered after refresh. Trying one last "
              "activation...");
      // Try activation again with runtime APN
      if (strlen(apn_str) > 0) {
        if (try_activate_apn(apn_str)) {
          bearer_recovery_active = false;
          return true;
        }
      }
      break;
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();
  }

  bearer_recovery_active = false;
  return false;
}
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

/**
 * v5.75: Spatika Multi-Platform Header Sync (IST)
 * 
 * Fetches the standard HTTP 'Date' header from the server and syncs the RTC.
 * This works on any platform (IIS, Apache, Nginx, Google) as it follows RFC 7231.
 * Format: "Date: Wed, 01 Apr 2026 12:45:05 GMT"
 * 
 * This resolves the 'Rejected' issue where the server rejects future-timed data.
 */
void sync_rtc_from_http_header(const String& head) {
  if (head.length() < 20) {
    debugln("[RTC-Sync] Invalid header string passed.");
    return;
  }
  debugln("[RTC-Sync] Processing Server Header (IST Transition)...");
  
  // Standard HTTP Header Parsing
  int dateIdx = head.indexOf("Date: ");
  if (dateIdx == -1) {
    debugln("[RTC-Sync] Date header not found. Check modem logs.");
    return;
  }
  
  // Extract: "Wed, 01 Apr 2026 07:15:05 GMT"
  String dateStr = head.substring(dateIdx + 6);
  int endIdx = dateStr.indexOf('\r');
  if (endIdx != -1) dateStr = dateStr.substring(0, endIdx);
  dateStr.trim();
  
  int day, year, hour, min, sec;
  char monStr[4];
  
  // Skip weekday (e.g. "Wed, ") - move to first digit
  const char* p = dateStr.c_str();
  while (*p && !isdigit(*p)) p++; 
  
  if (sscanf(p, "%d %3s %d %d:%d:%d", &day, monStr, &year, &hour, &min, &sec) == 6) {
    int month = 1;
    // Multi-platform month array (standard abbreviated names)
    const char* months[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    for (int i=0; i<12; i++) {
       if (strcasecmp(monStr, months[i]) == 0) { month = i + 1; break; }
    }
    
    // Safety check for valid year range (2025-2050)
    if (year < 2025 || year > 2050) {
        debugf("[RTC-Sync] Invalid Year parsed: %d\n", year);
        return;
    }

    struct tm t_in = {0};
    t_in.tm_year = year - 1900;
    t_in.tm_mon = month - 1;
    t_in.tm_mday = day;
    t_in.tm_hour = hour;
    t_in.tm_min = min;
    t_in.tm_sec = sec;
    t_in.tm_isdst = 0;
    
    // Robust GMT -> IST (+5:30) logic
    convert_gmt_to_ist(&t_in);
    
    int s_yy = t_in.tm_year + 1900;
    int s_mm = t_in.tm_mon + 1;
    int s_dd = t_in.tm_mday;
    int s_hh = t_in.tm_hour;
    int s_mi = t_in.tm_min;
    int s_ss = t_in.tm_sec;

    // v5.75 Hardening [H-02]: Guard against mid-slot chaotic time jumps
    // If we are currently at min 14, and server says it is min 10 (RTC was fast),
    // we MUST NOT jump current_min back to 10 immediately or the scheduler will 
    // think it missed a minute and may not trigger the 15-min slot properly.
    // We update the physical RTC, but only update globals if drift < 120s.
    long current_ts = current_hour * 3600 + current_min * 60 + current_sec;
    long server_ts = s_hh * 3600 + s_mi * 60 + s_ss;
    bool sync_globals = (abs(server_ts - current_ts) < 120) || (current_min % 15 != 14 && s_mi % 15 != 14);

    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(2000)) == pdTRUE) {
      // 1. Update Physical RTC always (Hard correction)
      rtc.adjust(DateTime(s_yy, s_mm, s_dd, s_hh, s_mi, s_ss));
      xSemaphoreGive(i2cMutex);
      
      // 2. Update Global Variables only if safe (Soft correction)
      if (sync_globals) {
        portENTER_CRITICAL(&rtcTimeMux);
        current_day = s_dd; current_month = s_mm; current_year = s_yy;
        current_hour = s_hh; current_min = s_mi; current_sec = s_ss;
        portEXIT_CRITICAL(&rtcTimeMux);
      }
      
      portENTER_CRITICAL(&rtcTimeMux);
      rtc_daily_sync_done = true;
      portEXIT_CRITICAL(&rtcTimeMux);
      debugf("[RTC-Sync] ✅ RTC Corrected to IST: %04d-%02d-%02d %02d:%02d:%02d\n", s_yy, s_mm, s_dd, s_hh, s_mi, s_ss);
    }
  } else {
    debugln("[RTC-Sync] Header parsing failed. Non-standard date string or buffer corruption.");
  }
}
void sync_rtc_from_server_tm(const char *payload, bool force) {
  String response;
  if (!payload)
    return;

  // Reset daily sync flag at midnight each new day
  static int last_sync_day = -1;
  if (current_day != last_sync_day) {
    portENTER_CRITICAL(&rtcTimeMux);
    rtc_daily_sync_done = false;
    portEXIT_CRITICAL(&rtcTimeMux);
    last_sync_day =
        current_day; // M1 FIX: was never updated — reset fired every wake cycle
  }

  // v7.87: Correct at midnight or first successful check-in of each day.
  // force=true for Rejection path (always syncs).
  // force=false for Success path (syncs only once per day).
  bool already_synched = false;
  portENTER_CRITICAL(&rtcTimeMux);
  already_synched = rtc_daily_sync_done;
  portEXIT_CRITICAL(&rtcTimeMux);

  if (!force && already_synched)
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

  // v5.46: Robust UTC [INFO] IST conversion handling all month/year/leap rollovers
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
  // v5.74: Atomic snapshot for multi-field calculation (R-03 fix)
  int rtc_hour_s, rtc_min_s, rtc_sec_s;
  portENTER_CRITICAL(&rtcTimeMux);
  rtc_hour_s = current_hour;
  rtc_min_s = current_min;
  rtc_sec_s = current_sec;
  portEXIT_CRITICAL(&rtcTimeMux);

  int rtc_total_sec = rtc_hour_s * 3600 + rtc_min_s * 60 + rtc_sec_s;
  int srv_total_sec = s_hh * 3600 + s_mi * 60 + s_ss;
  int raw_drift = abs(srv_total_sec - rtc_total_sec);
  
  // v5.66: [CRIT] CRITICAL FIX - Handle Midnight Rollover!
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
    if (!force) {
      portENTER_CRITICAL(&rtcTimeMux);
      rtc_daily_sync_done = true; // Mark as done even if no correction
      portEXIT_CRITICAL(&rtcTimeMux);
    }
    last_sync_day = current_day;
    return;
  }

  // Apply correction to RTC
  debugf("[RTC-Sync] Correcting RTC: %02d:%02d:%02d [INFO] %02d:%02d:%02d (drift "
         "%ds)\n",
         current_hour, current_min, current_sec, s_hh, s_mi, s_ss, drift);

  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) == pdTRUE) {
    // Write perfectly translated server IST time and date to physical RTC
    rtc.adjust(DateTime(s_yy + 2000, s_mm, s_dd, s_hh, s_mi, s_ss));
    xSemaphoreGive(i2cMutex);
  }

  // v5.66: [CRIT] CRITICAL FIX - Update ALL globals immediately!
  // If the physical RTC battery died and it woke up in 1970, we MUST legally
  // update the current_day/year variables or else the rest of this waking cycle 
  // will create data filed under 1970 despite the physical RTC being fixed!
  // v5.70: Use rtcTimeMux to prevent torn reads by the scheduler or other cores.
  portENTER_CRITICAL(&rtcTimeMux);
  current_day = s_dd;
  current_month = s_mm;
  current_year = s_yy + 2000;
  current_hour = s_hh;
  current_min = s_mi;
  current_sec = s_ss;
  portEXIT_CRITICAL(&rtcTimeMux);

  // v5.75 FIX: [C-02] Correct Anchor Sequencing
  // Post-sync update: Guarded for cross-core visibility
  portENTER_CRITICAL(&rtcTimeMux);
  rtc_daily_sync_done = true;
  portEXIT_CRITICAL(&rtcTimeMux);
  last_sync_day = current_day;

  debugln("[RTC-Sync] ✅ RTC corrected from server time (IST).");
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
    if (diag_consecutive_sim_fails >= 13) {
      debugf1("[SIM] PERSISTENT ERROR for %d consecutive slots (~3h15m). "
              "Final resort: ESP32 SOFTWARE RESTART...\n",
              diag_consecutive_sim_fails);
      diag_consecutive_sim_fails = 0; // Prevent infinite persistent restart loop
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      ESP.restart();
    }
    gprs_mode = eGprsSignalForStoringOnly;
  } else {
    diag_consecutive_sim_fails = 0;
    
    // v5.85: P2 - Signal Fast-Track (Only on healthy known networks)
    if (last_http_ok && signal_lvl > -98 && strlen(carrier) > 0) {
        SerialSIT.println("AT+CSQ");
        String r = waitForResponse("+CSQ", 1000);
        int idx = r.indexOf("+CSQ: ");
        if (idx != -1) {
            int raw = r.substring(idx + 6).toInt();
            if (raw > 0 && raw < 32) {
                signal_strength = -113 + 2 * raw;
                signal_lvl = signal_strength;
                debugf("[GPRS] Fast-Track CSQ: %d dBm (carrier: %s)\n", signal_lvl, carrier);
                goto skip_full_init;
            }
        }
    }

    get_signal_strength();

    get_network();
skip_full_init:
    get_registration();
    if (gprs_mode != eGprsSignalForStoringOnly) {
      get_a7672s();
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
    if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
      SerialSIT.println("AT");
      if (waitForResponse("OK", 500).indexOf("OK") != -1) {
        debugln("[GPRS] Modem alive. Closing network session gracefully...");
        SerialSIT.println("AT+CPOWD=1"); // Normal Power Down
        waitForResponse("NORMAL POWER DOWN", 8000); // Extended timeout for graceful detach
        vTaskDelay(500 / portTICK_PERIOD_MS);
      }
      xSemaphoreGive(modemMutex);
    } else {
      debugln("[GPRS] [WARN] modemMutex locked by running task. Skipping AT+CPOWD.");
    }
  } else {
    debugln("[GPRS] [WARN] Session was unstable (reg fails). Hard Hardware "
            "Reset.");
  }

  debugln("[GPRS] Cutting physical VCC power (GPIO 26 -> LOW).");
  digitalWrite(26, LOW);
  
  // v5.74 Fix C: Clear stale HTTP session flag BEFORE cutting modem power.
  // Without this, http_ready stays true across deep sleep. On the next wakeup
  // send_at_cmd_data() skips its !http_ready fast-fail and fires AT+HTTPDATA
  // into a powered-off modem, causing a guaranteed TIMEOUT on every first attempt.
  http_ready = false;

  // v5.65 P4 Fix: I2C Spike Recovery
  // Modem power-down can cause spikes on the shared I2C bus (SDA/SCL).
  // Perform an immediate silent bus recovery to ensure RTC/Sensors remain accessible.
  recoverI2CBus();
  
  portENTER_CRITICAL(&syncMux);
  gprs_started = false;
  gprs_mode = eGprsSleepMode; // Prevent Ghost Restart during sleep entry
  portEXIT_CRITICAL(&syncMux);
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

  portENTER_CRITICAL(&syncMux);
  sync_mode = eSMSStop;
  portEXIT_CRITICAL(&syncMux);
  xSemaphoreGive(modemMutex);
}

// REQUIRES: modemMutex held by caller (Not thread-safe due to static buffer)
String waitForResponse(const char *expected, unsigned long timeout) {
  // v5.68 Stability Fix: Buffer UART in static char array
  // dynamic String (response += c) to prevent massive heap fragmentation
  // on long multi-kilobyte JSON HTTP reads.
  static char buf[2048]; // Phase 5 Fix: Moved from Stack to Heap/BSS to stop Stack Overflows
  memset(buf, 0, sizeof(buf));
  int buf_idx = 0;
  buf[0] = '\0'; // Initialize empty
  
  unsigned long startTime = millis();

  while ((millis() - startTime) < timeout) {
    vTaskDelay(1 / portTICK_PERIOD_MS);
    esp_task_wdt_reset(); // Keep watchdog happy during long AT command waits
    last_activity_time = millis(); // v5.85: Refresh safety heartbeat on every poll iteration

    while (SerialSIT.available()) {
      char c = SerialSIT.read();
      if (buf_idx < 2047) { // Prevent unbounded growth
        buf[buf_idx++] = c;
        buf[buf_idx] = '\0';
      }
    }

    // Use fast C-string search to avoid allocating intermediate Strings
    if (strstr(buf, expected) != NULL) {
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
void power_cut_modem_shutdown() {
  debugln("[HEAL] Graceful 2 PM Maintenance Reboot Activated.");
  sync_mode = eHttpStop; // Prevent stray UART tasks (eHttpStop equivalent)
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
#endif
