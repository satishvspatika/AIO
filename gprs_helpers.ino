
#include "globals.h"

// Needed for SerialSIT usage if not included
extern HardwareSerial SerialSIT;
// debugln/debug are macros in globals.h
extern String waitForResponse(String, int);

void save_apn_config(String apn, String ccid) {
  File f = SPIFFS.open("/apn_config.txt", FILE_WRITE);
  if (f) {
    f.println(ccid);
    f.println(apn);
    f.close();
    debugln("Saved APN Config: " + apn);
  }
}

bool load_apn_config(String current_ccid, char *target_apn, size_t max_len) {
  if (!SPIFFS.exists("/apn_config.txt"))
    return false;
  File f = SPIFFS.open("/apn_config.txt", FILE_READ);
  if (!f)
    return false;

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

  // Validate
  if (stored_ccid_str.length() > 5 &&
      current_ccid.indexOf(stored_ccid_str) != -1 &&
      stored_apn_str.length() > 0) {
    strncpy(target_apn, stored_apn_str.c_str(), max_len);
    target_apn[max_len - 1] = '\0';
    debugln("Smart APN: Match Found!");
    return true;
  }
  debugln("Smart APN: No Match.");
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

  // 1. Exhaustive Context Cleanup (v5.56 Healer)
  // Query what's currently active and kill it all.
  SerialSIT.println("AT+CGACT?");
  String active_resp = waitForResponse("OK", 3000);
  int start_pos = 0;
  while ((start_pos = active_resp.indexOf("+CGACT: ", start_pos)) != -1) {
    int comma = active_resp.indexOf(',', start_pos);
    int space = active_resp.indexOf(' ', start_pos);
    if (comma != -1 && space != -1) {
      int cid = active_resp.substring(space + 1, comma).toInt();
      int status = active_resp.substring(comma + 1, comma + 2).toInt();
      if (status == 1) { // If active, kill it
        debugf("[GPRS] Cleanup: Deactivating Ghost CID:%d\n", cid);
        SerialSIT.printf("AT+CGACT=0,%d\r\n", cid);
        waitForResponse("OK", 3000);
      }
    }
    start_pos += 7;
  }
  vTaskDelay(2000 / portTICK_PERIOD_MS); // v7.11: Stabilization

  // v7.13: Resilient CPIN Handshake (Rule 23)
  // The SIM controller may be busy post-refresh. Wait and retry.
  bool sim_ready = false;
  for (int i = 0; i < 5; i++) {
    SerialSIT.println("AT+CPIN?");
    if (waitForResponse("READY", 3000).indexOf("READY") != -1) {
      sim_ready = true;
      break;
    }
    debugf("[GPRS] SIM not ready (Retry %d/5)...\n", i + 1);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();
  }

  if (!sim_ready) {
    debugln("APN Activation FAILED: SIM not ready (CPIN) after retries.");
    return false;
  }

  // 2. Set Context
  if (strcmp(apn, "jionet") == 0) {
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
             "AT+CGDCONT=1,\"IPV4V6\",\"%s\"", apn);
  } else {
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CGDCONT=1,\"IP\",\"%s\"",
             apn);
  }
  SerialSIT.println(gprs_xmit_buf);
  waitForResponse("OK", 3000);
  vTaskDelay(500 / portTICK_PERIOD_MS);

  // 3. Attempt Activation
  SerialSIT.println("AT+CGACT=1,1");
  String response = waitForResponse("OK", 25000);
  debugln("CGACT Resp: " + response); // v7.11: Log full response

  if (response.indexOf("OK") != -1 && response.indexOf("ERROR") == -1) {
    vTaskDelay(2000 / portTICK_PERIOD_MS); // Settling delay

    // 4. STRICT VERIFICATION: Assigned IP must be valid (not 0.0.0.0)
    SerialSIT.println("AT+CGPADDR=1");
    String ip_resp = waitForResponse("OK", 3000);
    debug("Assigned IP: ");
    debugln(ip_resp);

    // Look for a pattern like "+CGPADDR: 1,10.x.x.x" or similar.
    // If it contains "0.0.0.0" or no digits at all, it's a failure.
    if (ip_resp.indexOf("+CGPADDR: 1,") != -1 &&
        ip_resp.indexOf("0.0.0.0") == -1 && ip_resp.indexOf(".") != -1) {
      debugln("APN Activation Success (Valid IP)!");
      // Set explicit DNS to prevent DNS-related hangs (Error 713/715)
      SerialSIT.println("AT+CDNSCFG=\"8.8.8.8\",\"1.1.1.1\"");
      waitForResponse("OK", 500);
      return true;
    } else {
      debugln("APN Activation ERROR: No valid IP assigned (0.0.0.0).");
      // Force cleanup
      SerialSIT.println("AT+CGACT=0,1");
      waitForResponse("OK", 1000);
    }
  }

  debugln("APN Activation Failed.");
  return false;
}

bool verify_bearer_or_recover() {
  ota_silent_mode = true; // Block UI/I2C tasks from interrupting
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
    bool ip_ok =
        (ip_resp.indexOf(paddr_prefix) != -1 &&
         ip_resp.indexOf("0.0.0.0") == -1 && ip_resp.indexOf(".") != -1);

    if (ip_ok) {
      // v5.52: Extra check - is the APN actually what we expect?
      // Only check if we have a known target APN (avoids breaking new SIMs)
      if (strlen(apn_str) > 0) {
        String rdp_cmd = "AT+CGCONTRDP=" + String(check_cid);
        SerialSIT.println(rdp_cmd);
        String rdp_resp = waitForResponse("OK", 3000);
        if (rdp_resp.indexOf('"' + String(apn_str) + '"') == -1) {
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
      ota_silent_mode = false;
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
      ota_silent_mode = false;
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
      ota_silent_mode = false;
      return true;
    }
  }

  // 4. Fallback: Query CCID and load stored APN from SPIFFS.
  debugln("AG Recovery: Runtime APN failed. Querying CCID...");
  String ccid = get_ccid();
  char stored_apn[20] = {0};
  if (load_apn_config(ccid, stored_apn, sizeof(stored_apn))) {
    debugf("AG Recovery: Trying stored APN -> %s\n", stored_apn);
    if (try_activate_apn(stored_apn)) {
      ota_silent_mode = false;
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
          ota_silent_mode = false;
          return true;
        }
      }
      break;
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();
  }

  ota_silent_mode = false;
  return false;
}
