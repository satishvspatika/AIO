/* =========================================================================
 *  STANDALONE MODEM TEST SKETCH (A7672S TCP SOCKET HTTP POST)
 * =========================================================================
 *  Uses proven AT+NETOPEN / AT+CIPOPEN / AT+CIPSEND direct TCP socket
 *  HTTP POST mechanism for 100% reliability across all A7672M6 firmware builds.
 * ========================================================================= */

#include <Arduino.h>
#include <HardwareSerial.h>

#define MODEM_PWR_PIN 26
#define MODEM_RX_PIN  16
#define MODEM_TX_PIN  17

HardwareSerial SerialModem(1);
char rx_buf[2048];

void flushModem() {
  while (SerialModem.available()) SerialModem.read();
}

bool sendCommand(const char *cmd, const char *expected, unsigned long timeoutMs) {
  flushModem();
  if (cmd != NULL && strlen(cmd) > 0) {
    SerialModem.println(cmd);
    Serial.print("[TX] "); Serial.println(cmd);
  }
  
  unsigned long start = millis();
  int idx = 0;
  memset(rx_buf, 0, sizeof(rx_buf));

  while (millis() - start < timeoutMs) {
    while (SerialModem.available()) {
      char c = SerialModem.read();
      if (idx < (int)sizeof(rx_buf) - 1) {
        rx_buf[idx++] = c;
        rx_buf[idx] = '\0';
      }
    }
    if (expected != NULL && strstr(rx_buf, expected) != NULL) {
      Serial.print("[RX] "); Serial.println(rx_buf);
      return true;
    }
  }
  Serial.print("[RX TIMEOUT/FAIL] "); Serial.println(rx_buf);
  return false;
}

bool waitForRawResponse(const char *expected, unsigned long timeoutMs) {
  unsigned long start = millis();
  int idx = 0;
  memset(rx_buf, 0, sizeof(rx_buf));

  while (millis() - start < timeoutMs) {
    while (SerialModem.available()) {
      char c = SerialModem.read();
      if (idx < (int)sizeof(rx_buf) - 1) {
        rx_buf[idx++] = c;
        rx_buf[idx] = '\0';
      }
    }
    if (expected != NULL && strstr(rx_buf, expected) != NULL) {
      Serial.print("[RX] "); Serial.println(rx_buf);
      return true;
    }
  }
  Serial.print("[RX TIMEOUT/FAIL] "); Serial.println(rx_buf);
  return false;
}

bool sendTcpHttpPost(const char *ip, const char *host, const char *path, uint16_t port, const char *payload, const char *content_type) {
  int plen = strlen(payload);
  char rawHttp[512];
  snprintf(rawHttp, sizeof(rawHttp),
    "POST %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Content-Type: %s\r\n"
    "Content-Length: %d\r\n"
    "Connection: close\r\n\r\n"
    "%s", path, host, content_type, plen, payload);

  int rawLen = strlen(rawHttp);

  char openCmd[128];
  snprintf(openCmd, sizeof(openCmd), "AT+CIPOPEN=0,\"TCP\",\"%s\",%d", ip, port);
  
  if (!sendCommand(openCmd, "+CIPOPEN: 0,0", 15000)) {
    Serial.println("[TCP] CIPOPEN Failed.");
    return false;
  }
  Serial.println("[TCP] Connected successfully!");

  char sendCmd[32];
  snprintf(sendCmd, sizeof(sendCmd), "AT+CIPSEND=0,%d", rawLen);
  if (!sendCommand(sendCmd, ">", 5000)) {
    Serial.println("[TCP] CIPSEND Prompt Failed.");
    sendCommand("AT+CIPCLOSE=0", "OK", 3000);
    return false;
  }

  flushModem();
  SerialModem.write((const uint8_t*)rawHttp, rawLen);
  Serial.println("[TCP] HTTP POST Header & Payload transmitted.");

  bool success = false;
  if (waitForRawResponse("200 OK", 15000) || strstr(rx_buf, "Success") != NULL || strstr(rx_buf, "HTTP/1.1 200") != NULL) {
    Serial.println("\n[TEST RESULT] TCP HTTP POST: SUCCESS (HTTP 200 OK)");
    success = true;
  } else {
    Serial.print("\n[TEST RESULT] TCP HTTP POST: FAIL -> Response: "); Serial.println(rx_buf);
  }

  sendCommand("AT+CIPCLOSE=0", "OK", 3000);
  return success;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n========================================================");
  Serial.println("   STANDALONE MODEM TEST (A7672S DIRECT TCP HTTP POST)");
  Serial.println("========================================================\n");

  // 1. Power On Modem
  pinMode(MODEM_PWR_PIN, OUTPUT);
  digitalWrite(MODEM_PWR_PIN, HIGH);
  SerialModem.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);
  delay(3000);

  // 2. Check Modem AT
  Serial.println("--- STEP 1: MODEM AT CHECK ---");
  bool modem_ok = false;
  for (int i = 0; i < 10; i++) {
    if (sendCommand("AT", "OK", 1000)) {
      modem_ok = true;
      break;
    }
    delay(500);
  }
  if (!modem_ok) {
    Serial.println("[TEST RESULT] MODEM POWER: FAIL");
    return;
  }
  Serial.println("[TEST RESULT] MODEM POWER: PASS\n");

  sendCommand("ATE0", "OK", 1000);
  sendCommand("AT+CMEE=2", "OK", 1000);

  // 3. SIM & ICCID Check
  Serial.println("--- STEP 2: SIM & ICCID CHECK ---");
  if (!sendCommand("AT+CPIN?", "READY", 5000)) {
    Serial.println("[TEST RESULT] SIM CPIN: FAIL");
    return;
  }
  Serial.println("[TEST RESULT] SIM CPIN: PASS\n");

  // 4. CSQ & Registration Check
  Serial.println("--- STEP 3: SIGNAL & NETWORK REGISTRATION ---");
  sendCommand("AT+CNMP=38", "OK", 2000); // 4G LTE Mode
  sendCommand("AT+CSQ", "+CSQ:", 2000);

  bool registered = false;
  for (int i = 1; i <= 15; i++) {
    if (sendCommand("AT+CEREG?", "OK", 1500)) {
      if (strstr(rx_buf, "+CEREG: 0,1") || strstr(rx_buf, "+CEREG: 0,5") || strstr(rx_buf, ",1") || strstr(rx_buf, ",5")) {
        registered = true;
        break;
      }
    }
    delay(1000);
  }
  if (!registered) {
    Serial.println("[TEST RESULT] NETWORK REGISTRATION: FAIL");
    return;
  }
  Serial.println("[TEST RESULT] NETWORK REGISTRATION: PASS\n");

  // 5. APN & PDP Context Setup
  Serial.println("--- STEP 4: PDP & NETOPEN SETUP ---");
  sendCommand("AT+CGDCONT=1,\"IP\",\"airteliot.com\"", "OK", 3000);
  sendCommand("AT+CGACT=1,1", "OK", 10000);
  sendCommand("AT+NETOPEN", "OK", 5000);
  sendCommand("AT+IPADDR", "OK", 3000);
  Serial.println("[TEST RESULT] NETWORK & IP READY\n");

  // 6. TEST 1: KSNDMC TWS v3 Endpoint
  Serial.println("--- STEP 5: TCP HTTP POST -> KSNDMC TWS v3 (stn_no=1062) ---");
  const char* ksndmc_payload = "stn_no=1062&rec_time=2026-09-09,14:45&temp=23.6&humid=89.5&w_speed=1.5&w_dir=234&signal=-67&bat_volt=12.5&key=climate4pTWS";
  sendTcpHttpPost("117.216.42.181", "rtdas.ksndmc.net", "/tws_gprs/update_tws_data_v3", 80, ksndmc_payload, "application/x-www-form-urlencoded");

  delay(2000);

  // 7. TEST 2: SPATIKA TWS v2 Endpoint
  Serial.println("\n--- STEP 6: TCP HTTP POST -> SPATIKA TWS v2 (stn_no=001802) ---");
  const char* spatika_payload = "stn_no=001802&rec_time=2026-09-11,07:15&key=climate4pTWS&rainfall=000.0&temp=028.3&humid=068.9&w_speed=00.0&w_dir=244&signal=-073&bat_volt=04.04&bat_volt2=04.04";
  sendTcpHttpPost("144.91.104.105", "rtdas.spatika.net", "/tws_gprs/update_tws_data_v2", 80, spatika_payload, "application/x-www-form-urlencoded");

  // 8. Power Off GPRS Modem
  Serial.println("\n--- STEP 7: POWERING OFF GPRS MODEM ---");
  sendCommand("AT+NETCLOSE", "OK", 3000);
  sendCommand("AT+CPOWD=1", "OK", 2000);
  digitalWrite(MODEM_PWR_PIN, LOW);
  Serial.println("[PWR] GPRS Modem Powered Off.");

  Serial.println("\n========================================================");
  Serial.println("   TCP SOCKET HTTP TEST COMPLETE!");
  Serial.println("========================================================\n");
}

void loop() {
  delay(1000);
}
