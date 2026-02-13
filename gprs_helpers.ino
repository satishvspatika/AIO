
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
  vTaskDelay(100);
  SerialSIT.println("AT+CICCID");
  String resp = waitForResponse("OK", 3000);
  debug("Raw CICCID Resp: ");
  debugln(resp);

  for (int i = 0; i < resp.length(); i++) {
    if (isdigit(resp[i]))
      ccid += resp[i];
  }

  // Attempt 2: Fallback to AT+CCID
  if (ccid.length() < 10) {
    ccid = ""; // Reset
    vTaskDelay(100);
    SerialSIT.println("AT+CCID");
    resp = waitForResponse("OK", 3000);
    debug("Raw CCID Resp: ");
    debugln(resp);
    for (int i = 0; i < resp.length(); i++) {
      if (isdigit(resp[i]))
        ccid += resp[i];
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

  // Check if context 1 is already active with THIS apn
  // (Optimization to avoid redundant re-activation)
  flushSerialSIT();
  SerialSIT.println("AT+CGACT?");
  String cgact = waitForResponse("OK", 3000);
  if (cgact.indexOf("+CGACT: 1,1") != -1) {
    debugln("Context already active. Testing...");
    // We'll proceed with activation just to be sure the APN matches
  }

  if (strcmp(apn, "jionet") == 0) {
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
             "AT+CGDCONT=1,\"IPV4V6\",\"%s\"", apn);
  } else {
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CGDCONT=1,\"IP\",\"%s\"",
             apn);
  }
  SerialSIT.println(gprs_xmit_buf);
  waitForResponse("OK", 3000);

  SerialSIT.println("AT+CGACT=1,1");
  String response = waitForResponse("OK", 20000); // 20s for network
  vTaskDelay(2000);

  if (response.indexOf("OK") != -1 && response.indexOf("ERROR") == -1) {
    vTaskDelay(2000); // 2-second settling delay for network routing

    // Verify assigned IP (Warms up the PDP context)
    SerialSIT.println("AT+CGPADDR=1");
    String ip_resp = waitForResponse("OK", 2000);
    debug("Assigned IP: ");
    debugln(ip_resp);

    debugln("APN Activation Success!");
    return true;
  }
  debugln("APN Activation Failed.");
  return false;
}
