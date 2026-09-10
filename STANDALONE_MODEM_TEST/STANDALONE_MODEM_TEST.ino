/* =========================================================================
 *  STANDALONE MODEM TEST SKETCH (HTTP & FTP VERIFICATION)
 * =========================================================================
 *  Self-contained test program for ESP32 + A7672S Modem.
 *  Tests SIM, Network, HTTP POST (Spatika & KSNDMC), and FTP upload.
 * ========================================================================= */

#include <Arduino.h>
#include <HardwareSerial.h>
#include <FS.h>
#include <SPIFFS.h>

#define MODEM_PWR_PIN 26
#define MODEM_RX_PIN  16
#define MODEM_TX_PIN  17

HardwareSerial SerialModem(1);
char rx_buf[1024];

void flushModem() {
  while (SerialModem.available()) SerialModem.read();
}

bool sendCommand(const char *cmd, const char *expected, unsigned long timeoutMs) {
  flushModem();
  SerialModem.println(cmd);
  Serial.print("[TX] "); Serial.println(cmd);
  
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

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n========================================================");
  Serial.println("   STANDALONE MODEM TEST SUITE (A7672S HTTP & FTP)");
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
    Serial.println("[TEST RESULT] MODEM POWER: FAIL (No response to AT)");
    return;
  }
  Serial.println("[TEST RESULT] MODEM POWER: PASS\n");

  sendCommand("ATE0", "OK", 1000);
  sendCommand("AT+CMEE=2", "OK", 1000);

  // 3. SIM & ICCID Check
  Serial.println("--- STEP 2: SIM & ICCID CHECK ---");
  if (!sendCommand("AT+CPIN?", "READY", 5000)) {
    Serial.println("[TEST RESULT] SIM CPIN: FAIL (SIM Not Ready)");
    return;
  }
  Serial.println("[TEST RESULT] SIM CPIN: PASS");

  sendCommand("AT+CICCID", "OK", 3000);
  char iccid[32] = {0};
  const char* iccid_ptr = strstr(rx_buf, "+CICCID: ");
  if (iccid_ptr) {
    sscanf(iccid_ptr, "+CICCID: %31s", iccid);
    Serial.print("[SIM INFO] ICCID: "); Serial.println(iccid);
  }
  Serial.println();

  // 4. CSQ & Registration Check
  Serial.println("--- STEP 3: SIGNAL & NETWORK REGISTRATION ---");
  int csq = 99;
  if (sendCommand("AT+CSQ", "+CSQ:", 2000)) {
    const char* p = strstr(rx_buf, "+CSQ: ");
    if (p) csq = atoi(p + 6);
  }
  int dbm = (csq != 99 && csq > 0) ? (-113 + 2 * csq) : -141;
  Serial.print("[TEST RESULT] SIGNAL STRENGTH: "); Serial.print(dbm); Serial.println(" dBm");

  bool registered = false;
  for (int i = 1; i <= 20; i++) {
    Serial.print("Polling CGREG IterATION #"); Serial.println(i);
    if (sendCommand("AT+CGREG?", "OK", 1500)) {
      if (strstr(rx_buf, "+CGREG: 0,1") != NULL || strstr(rx_buf, "+CGREG: 0,5") != NULL) {
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

  // Refresh CSQ post-registration
  if (sendCommand("AT+CSQ", "+CSQ:", 2000)) {
    const char* p = strstr(rx_buf, "+CSQ: ");
    if (p) csq = atoi(p + 6);
  }
  dbm = (csq != 99 && csq > 0) ? (-113 + 2 * csq) : -141;
  Serial.print("[REGISTERED SIGNAL]: "); Serial.print(dbm); Serial.println(" dBm\n");

  // 5. APN & PDP Context Setup
  Serial.println("--- STEP 4: PDP CONTEXT ACTIVATION ---");
  const char* target_apn = "airteliot.com"; // Default for Airtel M2M
  if (strstr(iccid, "899145") != NULL) target_apn = "airteliot.com";
  else if (strstr(iccid, "899110") != NULL || strstr(iccid, "899116") != NULL) target_apn = "airtelgprs.com";

  char pdp_cmd[128];
  snprintf(pdp_cmd, sizeof(pdp_cmd), "AT+CGDCONT=1,\"IP\",\"%s\"", target_apn);
  sendCommand(pdp_cmd, "OK", 3000);

  sendCommand("AT+CGACT=1,1", "OK", 10000);
  if (!sendCommand("AT+CGPADDR=1", "+CGPADDR: 1,", 3000)) {
    Serial.println("[TEST RESULT] PDP ACTIVATION: FAIL (No IP assigned)");
    return;
  }
  Serial.println("[TEST RESULT] PDP ACTIVATION: PASS\n");

  // 6. TEST HTTP 1: KSNDMC TWS v3 (rtdas.ksndmc.net)
  Serial.println("--- STEP 5: HTTP POST TEST -> rtdas.ksndmc.net (KSNDMC TWS v3) ---");
  sendCommand("AT+CGEREP=0", "OK", 1000);
  sendCommand("AT+HTTPTERM", "OK", 2000);

  if (sendCommand("AT+HTTPINIT", "OK", 5000)) {
    // Note: A7672S returns ERROR for AT+HTTPPARA="CID",1. We attempt it but do not abort on ERROR.
    bool cid_resp = sendCommand("AT+HTTPPARA=\"CID\",1", "OK", 1000);
    if (!cid_resp) {
      Serial.println("[INFO] AT+HTTPPARA=\"CID\",1 returned ERROR (expected on A7672S, continuing)");
    }
    
    sendCommand("AT+HTTPPARA=\"URL\",\"http://rtdas.ksndmc.net/tws_gprs/update_tws_data_v3\"", "OK", 2000);
    sendCommand("AT+HTTPPARA=\"ACCEPT\",\"*/*\"", "OK", 1000);
    sendCommand("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"", "OK", 1000);

    const char* payload = "stn_no=001826&rec_time=2026-09-09,22:15&temp=000.0&humid=000.0&w_speed=00.0&w_dir=000&signal=-65&bat_volt=03.92&key=climate4pTWS";
    char hdata[32];
    snprintf(hdata, sizeof(hdata), "AT+HTTPDATA=%d,5000", (int)strlen(payload));

    if (sendCommand(hdata, "DOWNLOAD", 5000)) {
      flushModem();
      SerialModem.write((uint8_t*)payload, strlen(payload));
      if (sendCommand("", "OK", 3000)) {
        if (sendCommand("AT+HTTPACTION=1", "+HTTPACTION:", 20000)) {
          if (strstr(rx_buf, "+HTTPACTION: 1,200,") != NULL) {
            Serial.println("\n[TEST RESULT] HTTP KSNDMC TWS v3: SUCCESS (HTTP 200 OK)");
            sendCommand("AT+HTTPREAD=0,100", "OK", 3000);
          } else {
            Serial.print("\n[TEST RESULT] HTTP KSNDMC TWS v3: FAIL -> Response: "); Serial.println(rx_buf);
          }
        } else {
          Serial.println("\n[TEST RESULT] HTTP KSNDMC TWS v3: FAIL (HTTPACTION Timeout)");
        }
      }
    } else {
      Serial.println("\n[TEST RESULT] HTTP KSNDMC TWS v3: FAIL (DOWNLOAD Timeout)");
    }
  } else {
    Serial.println("\n[TEST RESULT] HTTP KSNDMC TWS v3: FAIL (HTTPINIT Error)");
  }

  sendCommand("AT+HTTPTERM", "OK", 2000);
  Serial.println();

  // 7. TEST HTTP 2: SPATIKA TWSRP (rtdas.spatika.net)
  Serial.println("--- STEP 6: HTTP POST TEST -> rtdas.spatika.net (SPATIKA TWSRP) ---");
  if (sendCommand("AT+HTTPINIT", "OK", 5000)) {
    sendCommand("AT+HTTPPARA=\"CID\",1", "OK", 1000); // Ignore error if return ERROR
    sendCommand("AT+HTTPPARA=\"URL\",\"http://rtdas.spatika.net/tws_gprs/update_data_twsrp\"", "OK", 2000);
    sendCommand("AT+HTTPPARA=\"ACCEPT\",\"*/*\"", "OK", 1000);
    sendCommand("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"", "OK", 1000);

    const char* trg_payload = "stn_id=WS0100&rec_time=2026-09-07,10:45&rainfall=00.00&temp=028.5&humid=062.0&w_speed=00.0&w_dir=180&atm_pressure=915.00&signal=-070&bat_volt=04.12&key=wsgen2016";
    char hdata2[32];
    snprintf(hdata2, sizeof(hdata2), "AT+HTTPDATA=%d,5000", (int)strlen(trg_payload));

    if (sendCommand(hdata2, "DOWNLOAD", 5000)) {
      flushModem();
      SerialModem.write((uint8_t*)trg_payload, strlen(trg_payload));
      if (sendCommand("", "OK", 3000)) {
        if (sendCommand("AT+HTTPACTION=1", "+HTTPACTION:", 20000)) {
          if (strstr(rx_buf, "+HTTPACTION: 1,200,") != NULL) {
            Serial.println("\n[TEST RESULT] HTTP SPATIKA TWSRP: SUCCESS (HTTP 200 OK)");
            sendCommand("AT+HTTPREAD=0,100", "OK", 3000);
          } else {
            Serial.print("\n[TEST RESULT] HTTP SPATIKA TWSRP: FAIL -> Response: "); Serial.println(rx_buf);
          }
        } else {
          Serial.println("\n[TEST RESULT] HTTP SPATIKA TWSRP: FAIL (HTTPACTION Timeout)");
        }
      }
    } else {
      Serial.println("\n[TEST RESULT] HTTP SPATIKA TWSRP: FAIL (DOWNLOAD Timeout)");
    }
  } else {
    Serial.println("\n[TEST RESULT] HTTP SPATIKA TWSRP: FAIL (HTTPINIT Error)");
  }

  sendCommand("AT+HTTPTERM", "OK", 2000);
  Serial.println();

  // 8. TEST FTP PHASE 1: Test FTP on CURRENT APN (airteliot.com)
  Serial.println("--- STEP 7A: FTP LOGIN TEST (APN: airteliot.com) -> ftp.spatika.net ---");
  sendCommand("AT+HTTPTERM", "OK", 2000);
  delay(2000);
  sendCommand("AT+CFTPSSTOP=0", "OK", 3000);
  delay(500);
  sendCommand("AT+CFTPSSTART=0", "+CFTPSSTART:", 15000);
  delay(500);
  sendCommand("AT+CFTPSCFG=\"security\",0", "OK", 2000);
  sendCommand("AT+CFTPSCFG=\"bindcid\",1", "OK", 2000);
  sendCommand("AT+CFTPSCFG=\"transmode\",1", "OK", 2000);
  sendCommand("AT+CFTPSCFG=\"type\",I", "OK", 2000);
  sendCommand("AT+CFTPSSINGLEIP=1", "OK", 1000);

  bool ftp_phase1_ok = false;
  if (sendCommand("AT+CFTPSLOGIN=\"144.91.104.105\",21,\"twsrp\",\"pressure2016\",0", "+CFTPSLOGIN:", 30000)) {
    if (strstr(rx_buf, "+CFTPSLOGIN: 0") != NULL) {
      Serial.println("\n[TEST RESULT] FTP PHASE1 (airteliot.com): SUCCESS -> Port 21 is OPEN on IoT APN!");
      ftp_phase1_ok = true;
      
      // Perform actual file upload test
      Serial.println("  Testing actual file upload (PUTFILE)...");
      const char* dummy_ftp = "WS0100;2026-09-08,12:00;0.0;28.0;60.0;0.0;180;915.0;100;4.1\r\n";
      char put_cmd[64];
      snprintf(put_cmd, sizeof(put_cmd), "AT+CFTPSPUTFILE=\"/TEST_FILE_PHASE1.swd\",1");
      if (sendCommand(put_cmd, ">", 10000)) {
        flushModem();
        SerialModem.write((uint8_t*)dummy_ftp, strlen(dummy_ftp));
        if (sendCommand("", "+CFTPSPUTFILE: 0", 20000)) {
          Serial.println("  [TEST RESULT] FTP PHASE1 FILE UPLOAD: SUCCESS!");
        } else {
          Serial.println("  [TEST RESULT] FTP PHASE1 FILE UPLOAD: FAIL.");
        }
      } else {
         Serial.println("  [TEST RESULT] FTP PHASE1 FILE UPLOAD: FAIL (No > prompt).");
      }
      
      sendCommand("AT+CFTPSLOGOUT=0", "OK", 3000);
    } else {
      Serial.print("\n[TEST RESULT] FTP PHASE1 (airteliot.com): FAIL -> "); Serial.println(rx_buf);
    }
  } else {
    Serial.println("\n[TEST RESULT] FTP PHASE1 (airteliot.com): TIMEOUT!");
  }
  sendCommand("AT+CFTPSSTOP=0", "OK", 2000);

  if (!ftp_phase1_ok) {
    // 9. TEST FTP PHASE 2: Switch APN to airtelgprs.com and retry
    Serial.println("\n--- STEP 7B: FTP APN SWITCH TEST (airtelgprs.com) ---");
    Serial.println("  Deactivating bearer...");
    sendCommand("AT+CGACT=0,1", "OK", 5000);
    delay(2000);
    Serial.println("  Switching CGDCONT to airtelgprs.com...");
    sendCommand("AT+CGDCONT=1,\"IP\",\"airtelgprs.com\"", "OK", 3000);
    delay(1000);
    Serial.println("  Reactivating bearer...");
    sendCommand("AT+CGACT=1,1", "OK", 15000);
    if (sendCommand("AT+CGPADDR=1", "+CGPADDR: 1,", 5000)) {
      Serial.print("  [APN SWITCH] Got IP: "); Serial.println(rx_buf);

      sendCommand("AT+CFTPSSTOP", "OK", 3000);
      delay(500);
      sendCommand("AT+CFTPSSTART", "+CFTPSSTART: 0", 15000);
      delay(500);
      sendCommand("AT+CFTPSCFG=\"security\",0", "OK", 2000);
      sendCommand("AT+CFTPSCFG=\"bindcid\",1", "OK", 2000);
      sendCommand("AT+CFTPSCFG=\"transmode\",1", "OK", 2000);
      sendCommand("AT+CFTPSSINGLEIP=1", "OK", 1000);

      if (sendCommand("AT+CFTPSLOGIN=\"144.91.104.105\",21,\"twsrp\",\"pressure2016\",0", "+CFTPSLOGIN:", 30000)) {
        if (strstr(rx_buf, "+CFTPSLOGIN: 0") != NULL) {
          Serial.println("\n[TEST RESULT] FTP PHASE2 (airtelgprs.com): SUCCESS!");
          Serial.println("  => FIX: Use airtelgprs.com APN for FTP on Airtel SIMs.");
          
          // Perform actual file upload test
          Serial.println("  Testing actual file upload (PUTFILE)...");
          const char* dummy_ftp = "WS0100;2026-09-08,12:00;0.0;28.0;60.0;0.0;180;915.0;100;4.1\r\n";
          char put_cmd[64];
          snprintf(put_cmd, sizeof(put_cmd), "AT+CFTPSPUTFILE=\"/TEST_FILE_PHASE2.swd\",1");
          if (sendCommand(put_cmd, ">", 10000)) {
            flushModem();
            SerialModem.write((uint8_t*)dummy_ftp, strlen(dummy_ftp));
            if (sendCommand("", "+CFTPSPUTFILE: 0", 20000)) {
              Serial.println("  [TEST RESULT] FTP PHASE2 FILE UPLOAD: SUCCESS! (Firewall bypassed!)");
            } else {
              Serial.println("  [TEST RESULT] FTP PHASE2 FILE UPLOAD: FAIL.");
            }
          } else {
             Serial.println("  [TEST RESULT] FTP PHASE2 FILE UPLOAD: FAIL (No > prompt).");
          }
          
          sendCommand("AT+CFTPSLOGOUT", "OK", 3000);
        } else {
          Serial.print("\n[TEST RESULT] FTP PHASE2 (airtelgprs.com): FAIL -> "); Serial.println(rx_buf);
          Serial.println("  => Diagnosis: Both APNs block port 21, or SIM not provisioned for airtelgprs.com.");
        }
      } else {
        Serial.println("\n[TEST RESULT] FTP PHASE2 (airtelgprs.com): TIMEOUT.");
        Serial.println("  => Diagnosis: Port 21 blocked on both APNs.");
      }
      sendCommand("AT+CFTPSSTOP", "OK", 2000);

      // Restore original APN
      Serial.println("\n  Restoring to airteliot.com APN...");
      sendCommand("AT+CGACT=0,1", "OK", 5000);
      delay(1000);
      sendCommand("AT+CGDCONT=1,\"IP\",\"airteliot.com\"", "OK", 3000);
      sendCommand("AT+CGACT=1,1", "OK", 15000);
      Serial.println("  APN restored.");
    } else {
      Serial.println("\n[TEST RESULT] FTP PHASE2: FAIL - airtelgprs.com bearer activation failed.");
      Serial.println("  => Diagnosis: IoT SIM not provisioned for airtelgprs.com APN.");
      // Restore original APN
      sendCommand("AT+CGACT=0,1", "OK", 5000);
      sendCommand("AT+CGDCONT=1,\"IP\",\"airteliot.com\"", "OK", 3000);
      sendCommand("AT+CGACT=1,1", "OK", 15000);
    }
  }

  Serial.println("\n========================================================");
  Serial.println("   ALL HARDWARE TESTS COMPLETE!");
  Serial.println("========================================================\n");
}

void loop() {
  // Idle after test completion
  delay(1000);
}
