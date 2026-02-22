
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

  // Attempt 1: Try AT+CICCID (Primary for A7672)
  flushSerialSIT();
  vTaskDelay(200);
  SerialSIT.println("AT+CICCID");
  String resp = waitForResponse("OK", 3000);
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
    vTaskDelay(200);
    SerialSIT.println("AT+CCID");
    resp = waitForResponse("+CCID:", 3000);
    debug("Raw CCID Resp: ");
    debugln(resp);
    startIdx = resp.indexOf("+CCID:");
    if (startIdx != -1) {
      for (int i = startIdx + 6; i < resp.length(); i++) {
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

  // 1. Force Deactivation for a clean state
  SerialSIT.println("AT+CGACT=0,1");
  waitForResponse("OK", 3000);
  vTaskDelay(500 / portTICK_PERIOD_MS);

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

  if (response.indexOf("OK") != -1 && response.indexOf("ERROR") == -1) {
    vTaskDelay(2000); // Settling delay

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
  flushSerialSIT();

  // 1. First explicitly check if the context is Active via CGACT
  SerialSIT.println("AT+CGACT?");
  String cgact_resp = waitForResponse("OK", 3000);

  bool context_active = false;
  if (cgact_resp.indexOf("+CGACT: 1,1") != -1) {
    context_active = true;
  }

  // 2. If context is active, verify assigned IP is valid
  if (context_active) {
    SerialSIT.println("AT+CGPADDR=1");
    String ip_resp = waitForResponse("OK", 3000);

    if (ip_resp.indexOf("+CGPADDR: 1,") != -1 &&
        ip_resp.indexOf("0.0.0.0") == -1 && ip_resp.indexOf(".") != -1) {
      debugln("Bearer Check: OK (Context Active & Valid IP)");
      return true;
    } else {
      debugln("Bearer Check: FAILED (Context Active but Invalid IP). "
              "Recovering...");
    }
  } else {
    debugln("Bearer Check: FAILED (Context Not Active). Recovering...");
  }

  // Clear serial buffers explicitly before fetching CCID to drop dirty URCs
  vTaskDelay(500 / portTICK_PERIOD_MS);
  flushSerialSIT();

  // 3. Bearer is dead or invalid. Attempt recovery using stored APN.
  String ccid = get_ccid();
  char stored_apn[20] = {0};
  if (load_apn_config(ccid, stored_apn, sizeof(stored_apn))) {
    if (try_activate_apn(stored_apn))
      return true;
  }

  // Fallback to current runtime apn_str
  if (strlen(apn_str) > 0) {
    if (try_activate_apn(apn_str))
      return true;
  }

  return false;
}
