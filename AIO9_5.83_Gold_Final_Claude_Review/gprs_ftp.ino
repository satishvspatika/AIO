#include "globals.h"

void send_ftp_file(char *fileName, bool isDailyFTP, bool alreadyLocked) {
  // v7.70+: ABBA DEADLOCK FIX — Strictly enforce hierarchy: modemMutex → fsMutex
  char put_buf[64] = {0}; // v5.70: Moved to function scope for Cleanup Race Guard
  if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(20000)) != pdTRUE) {
      debugln("[FTP] Mutex Timeout: modemMutex busy.");
      return;
  }

  // v7.70: Release fsMutex BEFORE entering modem setup (Deadlock Prevent)
  // alreadyLocked is ignored now; helper functions will take what they need.
  bool weLockedFS = false; 
  // Function logic proceeds lock-free; file accesses will take fsMutex locally.

  String response;
  const char *response_char;
  const char *csqstr;
  char gprs_xmit_buf[300];
  int handle_no, fileSize = 0;
  String response1;

  flushSerialSIT(); 
  debugln("Initializing A7672S for FTP...");
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  bool fileExists = false;
  bool ftp_started = false; // v5.70 Hardened: [N-8] Guard cleanup drain on skipped files
  if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
      fileExists = SPIFFS.exists(fileName);
      xSemaphoreGive(fsMutex);
  }

  if (fileExists) {
    send_daily = 2;
    int initial_ftp_mode = 0; 
    if (preferred_ftp_mode != -1) {
       initial_ftp_mode = preferred_ftp_mode;
       debugln("[FTP] Using SAVED Smart Mode: " + String(initial_ftp_mode == 1 ? "Passive" : "Active"));
    } else if (strstr(carrier, "AIRTEL") || strstr(carrier, "Airtel") || strstr(carrier, "Jio")) {
      initial_ftp_mode = 1; 
      debugln("[FTP] Airtel/Jio detected. Smart Default: Passive (1).");
    } else {
      debugln("[FTP] BSNL/Other detected. Smart Default: Active (0).");
    }
    
    ftp_login_flag = setup_ftp(initial_ftp_mode);
    if (ftp_login_flag == 1) {
      ftp_started = true;
      SerialSIT.println("ATE0");
      waitForResponse("OK", 3000);
      SerialSIT.println("AT+FSCD=C:/");
      waitForResponse("OK", 10000);

      char *modulePath = (fileName[0] == '/') ? &fileName[1] : fileName;
      snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSOPEN=\"C:/%s\",0\r\n", modulePath);
      SerialSIT.println(gprs_xmit_buf);
      response = waitForResponse("+FSOPEN", 5000);
      csqstr = strstr(response.c_str(), "+FSOPEN:");
      if (csqstr == NULL) {
        debugln("Error: +FSOPEN not found");
        goto ftp_cleanup;
      }
      sscanf(csqstr, "+FSOPEN: %d,", &handle_no);

            File file1;
            if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
                file1 = SPIFFS.open(fileName, FILE_READ);
                if (file1) {
                    fileSize = file1.size();
                    debugln("File size: " + String(fileSize));
                    if (fileSize > 0) {
                        snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSWRITE=%d,%d,30\r\n", handle_no, fileSize);
                        debugln("[FTP] Sending FSWRITE command...");
                        vTaskDelay(200 / portTICK_PERIOD_MS);
                        SerialSIT.println(gprs_xmit_buf);
                        if (waitForResponse("CONNECT", 5000).indexOf("CONNECT") != -1) {
                            char file_buf[256];
                            int bytes_streamed = 0;
                            while (file1.available()) {
                                int bytesRead = file1.read((uint8_t*)file_buf, 255);
                                SerialSIT.write((uint8_t*)file_buf, bytesRead);
                                bytes_streamed += bytesRead;
                                if (bytes_streamed % (16 * 1024) == 0) esp_task_wdt_reset();
                            }
                            SerialSIT.println();
                            waitForResponse("+FSWRITE", 5000);
                        }
                    }
                    file1.close();
                } else {
                   debugln("Failed to open local file for FTP.");
                   xSemaphoreGive(fsMutex);
                   goto ftp_cleanup;
                }
                xSemaphoreGive(fsMutex);
            } else {
                debugln("[FTP] Mutex Timeout: Skipping SPIFFS->Modem stream phase.");
                goto ftp_cleanup;
            }

        snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSCLOSE=%d", handle_no);
        SerialSIT.println(gprs_xmit_buf);
        waitForResponse("OK", 5000);

        int ftp_put_retries = 0;
        bool upload_success = false;
        snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CFTPSPUTFILE=\"%s\",1", modulePath);

        while (ftp_put_retries <= 1) {
          esp_task_wdt_reset();
          if (!verify_bearer_or_recover()) break;

          debugln("[FTP] Dispatching PUTFILE command...");
          SerialSIT.println(gprs_xmit_buf);
          memset(put_buf, 0, sizeof(put_buf)); // Clear for each attempt
          int put_idx = 0;
          unsigned long put_start = millis();
          while ((millis() - put_start) < 60000) {
            esp_task_wdt_reset();
            vTaskDelay(200 / portTICK_PERIOD_MS); // v5.75: Increased poll frequency
            while (SerialSIT.available() && put_idx < 63) {
                put_buf[put_idx++] = (char)SerialSIT.read();
                put_buf[put_idx] = '\0';
            }
            if (strstr(put_buf, "+CFTPSPUTFILE:")) break;
            if (strstr(put_buf, "+CGEV: ME PDN DEACT 1")) break;
          }
          debug("Response of AT+CFTPSPUTFILE: ");
          debugln(put_buf);

          if (strstr(put_buf, "+CFTPSPUTFILE: 0") != NULL) {
            upload_success = true;
            preferred_ftp_mode = (ftp_put_retries == 0) ? initial_ftp_mode : (1 - initial_ftp_mode);
            if (isDailyFTP) strcpy(last_cmd_res, "Success: Daily FTP OK");
            else strcpy(last_cmd_res, "Success: Backlog FTP OK");
            break;
          } else {
            if (ftp_put_retries < 1) {
              int next_mode = (ftp_put_retries == 0) ? (1 - initial_ftp_mode) : initial_ftp_mode;
              
              // v5.70: Fix C-03 FTP Put Race (Retry Path)
              if (strstr(put_buf, "+CFTPSPUTFILE:") == NULL) {
                  debugln("[FTP] Race Guard: Logout too fast? Waiting 1s drain.");
                  vTaskDelay(1000 / portTICK_PERIOD_MS);
              }

              SerialSIT.println("AT+CFTPSLOGOUT");
              waitForResponse("+CFTPSLOGOUT: 0", 5000);
              vTaskDelay(2000 / portTICK_PERIOD_MS);
              if (setup_ftp(next_mode) != 1) break;
              SerialSIT.println("AT+FSCD=C:/");
              waitForResponse("OK", 5000);
            }
            ftp_put_retries++;
          }
        }

        if (upload_success) {
          diag_consecutive_reg_fails = 0;
          diag_http_present_fails = 0;
          markFileAsDelivered(fileName, true);
          if (isDailyFTP) SPIFFS.remove(fileName);
          else {
            SPIFFS.remove("/ftpunsent.txt");
            if (SPIFFS.exists("/ftpremain.txt")) SPIFFS.rename("/ftpremain.txt", "/ftpunsent.txt");
          }

          // Cleanup .swd/.kwd files
          const char *pattern = strstr(UNIT, "SPATIKA") ? ".swd" : ".kwd";
          File rootDir = SPIFFS.open("/");
          if (rootDir) {
             File f = rootDir.openNextFile();
             while(f) {
                String n = f.name();
                if (n.endsWith(pattern)) { f.close(); SPIFFS.remove("/" + n); }
                else f.close();
                f = rootDir.openNextFile();
             }
             rootDir.close();
          }

          snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSDEL=\"C:/%s\"", modulePath);
          SerialSIT.println(gprs_xmit_buf);
          waitForResponse("OK", 5000);
        } else {
          strcpy(last_cmd_res, "Fail: FTP PUT Error");
          if (SPIFFS.exists("/ftpremain.txt")) SPIFFS.remove("/ftpremain.txt");
        }
      } 
    }

ftp_cleanup:
  if (ftp_started) {
    // v5.70: Fix C-03 FTP Put Race (Cleanup Path)
    if (strstr(put_buf, "+CFTPSPUTFILE:") == NULL) {
        debugln("[FTP] Race Guard: Logout too fast (Cleanup)? Waiting 1s drain.");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    SerialSIT.println("AT+CFTPSLOGOUT");
    waitForResponse("+CFTPSLOGOUT: 0", 5000);
    SerialSIT.println("AT+CFTPSSTOP");
    waitForResponse("OK", 3000);
  }
  
  if (weLockedFS) xSemaphoreGive(fsMutex);
  xSemaphoreGive(modemMutex);
}

void copyFile(const char *sourcePath, const char *destPath, bool alreadyLocked) {
  debugf2("Copying file %s to %s\n", sourcePath, destPath);

  if (!alreadyLocked) {
    if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
      debugln("[FS] Mutex timeout inside copyFile");
      return;
    }
  }

  File sourceFile = SPIFFS.open(sourcePath, "r");
  if (!sourceFile) {
    debugln("Failed to open source file for reading");
    if (!alreadyLocked) xSemaphoreGive(fsMutex); // Fix CRITICAL: Double-Give
    return;
  }

  File destFile = SPIFFS.open(destPath, "w");
  if (!destFile) {
    debugln("Failed to open destination file for writing");
    sourceFile.close();
    if (!alreadyLocked) xSemaphoreGive(fsMutex); // Fix CRITICAL: Double-Give
    return;
  }

  byte buffer[512]; 
  size_t bytesRead;

  while ((bytesRead = sourceFile.read(buffer, sizeof(buffer))) > 0) {
    destFile.write(buffer, bytesRead);
  }

  sourceFile.close();
  destFile.close();

  if (!alreadyLocked) xSemaphoreGive(fsMutex); // Fix CRITICAL: Double-Give
  debugln("File copied successfully!");
}

bool isBufferSanityOK(uint8_t *buf, int len, int offset) {
  // 1. Check for UART Cross-talk Signature (uh@\b)
  if (len >= 4) {
    if (buf[0] == 0x75 && buf[1] == 0x68 && buf[2] == 0x40 && buf[3] == 0x08) {
      Serial.println("[OTA] ❌ UART Cross-talk (0x75684008) DETECTED! Poisoned "
                     "chunk. Restarting...");
      return false;
    }
  }

  // 2. Check for "Modem-speak" strings at the BEGINNING of a chunk.
  // BUG-1 Fix: Removed "SampleNo", "Gaps", and "HTTP/" — these are common
  // byte sequences inside valid ESP32 firmware binaries and cause false
  // positives. Only uniquely modem-specific strings remain.
  // BUG-8 Fix: Loop auto-sized via sizeof to prevent count mismatch bugs.
  const char *traps[] = {"+HTTPREAD:", "ERROR\r\n"};
  int scanLimit = min(len, 32); // Only scan first 32 bytes for modem leakage
  for (int t = 0; t < (int)(sizeof(traps) / sizeof(traps[0])); t++) {
    int trapLen = strlen(traps[t]);
    for (int i = 0; i <= scanLimit - trapLen; i++) {
      if (memcmp(&buf[i], traps[t], trapLen) == 0) {
        Serial.printf("[OTA] \u274c Modem-speak '%s' detected at offset %d!\n",
                      traps[t], i);
        return false;
      }
    }
  }

  // 3. Header specific checks for first chunk
  if (offset == 0) {
    if (buf[0] != 0xE9) {
      Serial.printf("[OTA] ❌ Invalid Magic Byte: 0x%02X (Expected 0xE9)\n",
                    buf[0]);
      return false;
    }
  }

  // 4. Large block of repeating bytes (often a sign of UART/Flash sync
  // issues)
  int repeatCount = 0;
  for (int i = 1; i < len; i++) {
    if (buf[i] == buf[0])
      repeatCount++;
  }
  if (repeatCount > (len - 16)) { // 99.9% identical
    Serial.printf("[OTA] ❌ Suspicious data (99%% repeating 0x%02X). Aborting "
                  "chunk.\n",
                  buf[0]);
    return false;
  }

  return true;
}

int setup_ftp(int transMode) { // 0=Active(BSNL 2G), 1=Passive(Airtel 4G)
  flushSerialSIT(); // Clear leftover data from previous task
  char gprs_xmit_buf[300];
  String response, rssiStr;
  const char *resp;
  const char *ftpServer;
  const char *ftpUser;
  const char *ftpPassword;
  int portName, rssiIndex, result;

  if (strstr(UNIT, "BIHAR")) {
    // Bihar
    if (send_daily == 1) {
      ftpServer = "ftphbih.spatika.net";
      ftpUser = "trg_desbih_csvdt";
      ftpPassword = FTP_PASS_BIH_D;
      portName = 21;
      send_daily = 0;
    } else {
      ftpServer = "89.32.144.163"; // dot.spatika.net
      ftpUser = "dota_bih";
      ftpPassword = FTP_PASS_BIH_GEN;
      portName = 21;
    }
  } else if (strstr(UNIT, "KSNDMC")) {
    // DMC
    if (send_daily == 1) {
      ftpServer = "ftp1.ksndmc.net";
      ftpUser = "trg_spatika_v2@ksndmc.net";
      ftpPassword = FTP_PASS_KS_TRG;
      portName = 21;
      send_daily = 0;
    } else if (send_daily == 2) {
#if SYSTEM == 1
      ftpServer = "ftp1.ksndmc.net";
      ftpUser = "tws_spatika_v2";
      ftpPassword = FTP_PASS_KS_TWS;
      portName = 21;
#endif
#if SYSTEM == 2
      ftpServer = "ftp1.ksndmc.net";
      ftpUser = "twsrf_spatika_v2";
      ftpPassword = FTP_PASS_KS_ADD;
      portName = 21;
#endif
      //            ftpServer = "ftp.spatika.net";
      //            ftpUser = "hmisadmin";
      //            ftpPassword = "spt2015";
      //            portName = 21;
      send_daily = 0;
    } else {
      ftpServer = "89.32.144.163"; // dota.spatika.net
      ftpUser = "dota_dmc";
      ftpPassword = "airdata2016";
      portName = 21;
    }
  } else if (strstr(UNIT, "SPATIKA")) {
#if SYSTEM == 0
    ftpServer = "ftp.spatika.net";
    ftpUser = "trg_gen";
    ftpPassword = "spgen123";
    portName = 21;
#endif
#if SYSTEM == 1
    ftpServer = "ftp.spatika.net";
    ftpUser = "tws_gen";
    ftpPassword = "spgen123";
    portName = 21;
#endif
#if SYSTEM == 2
    ftpServer = "ftp.spatika.net";
    ftpUser = "twsrf_gen";
    ftpPassword = "spgen123";
    portName = 21;
#endif
  }

  send_daily = 0; // Ensure flag is reset after use
  result = -1;

  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
           "AT+CFTPSLOGIN=\"%s\",%d,\"%s\",\"%s\",0", ftpServer, portName,
           ftpUser, ftpPassword); //\r\n removed

  // NEW DOTA
  SerialSIT.println("AT+CFTPSSTOP"); // Ensure a clean slate
  vTaskDelay(500 / portTICK_PERIOD_MS);
  SerialSIT.println("AT+CFTPSSTART");
  response = waitForResponse("+CFTPSSTART: 0", 10000);
  debugln(response);

  SerialSIT.println("AT+CFTPSSINGLEIP=1"); // FTP client context
  waitForResponse("OK", 5000);

  // Force plain FTP mode (Security level 0 = No SSL)
  SerialSIT.println("AT+CFTPSCFG=\"security\",0");
  waitForResponse("OK", 2000);

  // Explicitly bind to GPRS context 1
  SerialSIT.println("AT+CFTPSCFG=\"bindcid\",1");
  waitForResponse("OK", 2000);

  // v7.68: Mode-switchable FTP transfer mode for carrier compatibility.
  // BSNL 2G → Active Mode (transmode=0): device opens data port, server connects back to it.
  // Airtel 4G → Passive Mode (transmode=1): server opens data port, client connects to it.
  // The caller selects the mode; the PUT retry loop switches mode on Error 9.
  if (transMode == 1) {
    debugln("[FTP] Using Passive Mode (transmode=1) — Airtel/Firewall path");
    SerialSIT.println("AT+CFTPSCFG=\"transmode\",1");
  } else {
    debugln("[FTP] Using Active Mode (transmode=0) — BSNL 2G path");
    SerialSIT.println("AT+CFTPSCFG=\"transmode\",0");
  }
  waitForResponse("OK", 2000);

  // Enable numeric error codes for better diagnostics
  SerialSIT.println("AT+CMEE=1");
  waitForResponse("OK", 2000);

  // DNS Warm-up: Force resolution of FTP server before login (Fixes Error 13)
  debugln("[FTP] Warming up DNS...");
  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CDNSGIP=\"%s\"",
           ftpServer);
  SerialSIT.println(gprs_xmit_buf);
  // v5.55 Optimized: Wait up to 20s for DNS resolution (BSNL/2G Latency)
  response = waitForResponse("+CDNSGIP:", 20000);
  debugln(response);

  // v5.45: DNS retry / Fallback
  if (response.indexOf("+CDNSGIP: 1") == -1) {
    debugln("[FTP] DNS failed. Checking Insurance IP fallback...");
    const char* fallbackIP = NULL;
    // Updated with User-Verified IPs (v5.55)
    if (strstr(ftpServer, "ksndmc.net")) fallbackIP = "117.216.42.181"; // rtdas.ksndmc.net
    else if (strstr(ftpServer, "spatika.net")) {
      if (strstr(ftpServer, "ftphbih") || strstr(ftpServer, "rtdasbih")) fallbackIP = "185.250.105.225";
      else if (strstr(ftpServer, "rtdasbmsk")) fallbackIP = "164.100.130.199";
      else fallbackIP = "144.91.104.105"; // rtdas.spatika.net
    }

    if (fallbackIP != NULL) {
      // BUG-C5 fix v5.65: Use the verified IP directly in the login command
      // to bypass DNS entirely. This saves ~40s on congested BSNL 2G towers.
      debugln("[FTP] Using Insurance IP fallback: " + String(fallbackIP));
      snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
               "AT+CFTPSLOGIN=\"%s\",%d,\"%s\",\"%s\",0",
               fallbackIP, portName, ftpUser, ftpPassword);
      // gprs_xmit_buf is now re-prepared with IP for the login below.
      // Skip the redundant DNS retry.
    } else {
      // No known IP for this host — retry DNS once as last resort
      debugln("[FTP] No fallback IP known. Retrying DNS once...");
      vTaskDelay(3000 / portTICK_PERIOD_MS);
      SerialSIT.println(gprs_xmit_buf);
      response = waitForResponse("+CDNSGIP:", 20000);
      debugln(response);
    }
  }

  vTaskDelay(2000 / portTICK_PERIOD_MS); // Pre-login settling delay

  // BUG-C5 fix v5.65: Only re-prepare with domain if DNS succeeded.
  // If the fallback IP path set gprs_xmit_buf above, preserve it — overwriting
  // with ftpServer (domain) would negate the entire IP fallback.
  if (response.indexOf("+CDNSGIP: 1") != -1) {
    // DNS OK — use domain as originally planned
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
             "AT+CFTPSLOGIN=\"%s\",%d,\"%s\",\"%s\",0", ftpServer, portName,
             ftpUser, ftpPassword);
  }
  // else: gprs_xmit_buf already has either the IP-based command (fallback path)
  // or the original domain command (no-fallback retry path). Use as-is.

  SerialSIT.println(gprs_xmit_buf);
  // v5.50 Optimized: Wait 45s for login (If it hasn't succeeded by then on
  // BSNL, it likely won't)
  response = waitForResponse("+CFTPSLOGIN:", 60000); // v7.68: Restored 60s timeout (was 45s in v5.50)
  debugln(response);

  // v7.68: No login-level fallback needed — mode is controlled by caller.
  // If login fails at the selected mode, return 0 and let the PUT retry switch modes.

  rssiIndex = response.indexOf("+CFTPSLOGIN:");
  if (rssiIndex != -1) {
    rssiIndex += 13;
    rssiStr = response.substring(rssiIndex, rssiIndex + 2);
  } else {
    rssiStr = "99"; // Explicit fail code
  }
  resp = rssiStr.c_str();
  result = atoi(resp);
  debug("Result is ");
  debugln(result);
  if (result == 0) {
    debugln("FTP Login success");
    return 1;
  } else {
    debugln("FTP Login unsuccessful");
    return 0;
  }
}

void fetchFromHttpAndUpdate(char *fileName) {
  if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(30000)) != pdTRUE) {
    debugln("[OTA] Error: Modem Mutex Timeout - deferring download");
    diag_modem_mutex_fails++;
    return;
  }
  ota_silent_mode = true; // Rule 50: Silence sensor tasks before modem reconfiguration
  String response;
  int handle_no;
  char gprs_xmit_buf[300];
  int total_no_of_bytes = 0;
  int actual_downloaded = 0;
  (void)handle_no; // suppress unused warning

  debugf1("[OTA] Starting Range-GET download for: %s\n", fileName);

  // v7.16: Rule 27 (Total Silence Protocol)
  // Completely silence the modem BEFORE any binary transfers occur.
  SerialSIT.println("AT+CREG=0");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CGEREP=0");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CNMI=0,0,0,0,0");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CIURC=0");
  waitForResponse("OK", 1000);

  // === STEP 1: HEAD request ===
  SerialSIT.println("AT+HTTPTERM");
  waitForResponse("OK", 2000);
  vTaskDelay(500 / portTICK_PERIOD_MS);
  flushSerialSIT();

  SerialSIT.println("AT+HTTPINIT");
  if (waitForResponse("OK", 5000).indexOf("OK") == -1) {
    debugln("[OTA] HTTPINIT failed (HEAD). Aborting.");
    xSemaphoreGive(modemMutex);
    Preferences p;
    p.begin("ota-track", false);
    p.putInt("fail_cnt", p.getInt("fail_cnt", 0) + 1);
    p.putString("fail_res", "HTTPINIT Fail");
    p.end();
    return;
  }
  // v5.46 (Rule 1): Bind to CID 1 — MANDATORY for Airtel/BSNL reliability
  SerialSIT.println("AT+HTTPPARA=\"CID\",1");
  if (waitForResponse("OK", 2000).indexOf("OK") == -1) {
    debugln(
        "[OTA] CID binding warning: No OK received, but continuing anyway.");
  }
  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
           "AT+HTTPPARA=\"URL\",\"http://%s:%s/builds/%s\"", HEALTH_SERVER_IP,
           OTA_SERVER_PORT, fileName);
  SerialSIT.println(gprs_xmit_buf);
  if (waitForResponse("OK", 2000).indexOf("OK") == -1) {
    debugln("[OTA] URL setup failed. Aborting.");
    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 2000);
    xSemaphoreGive(modemMutex);
    return;
  }
  flushSerialSIT(); // Pre-HEAD flush (Rule 28)
  // HTTP HEAD to determine Content-Length
  SerialSIT.println("AT+HTTPACTION=2");
  String head_resp = waitForResponse("+HTTPACTION:", 30000);
  debugf1("[OTA] HEAD: %s\n", head_resp.c_str());

  // v5.45: The HTTPACTION datalen field for a HEAD request = size of HTTP
  // *response headers*, NOT the firmware file size.
  // Per A7672S manual, we must use AT+HTTPHEAD to read the buffered headers.
  // v5.45: Wait 1s for BSNL network to respond
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  SerialSIT.println("AT+HTTPHEAD");
  String hdr_body = waitForResponse("\r\n\r\n", 10000);
  debugf1("[OTA] Headers: %s\n", hdr_body.c_str());

  // Parse Content-Length: from header body (case-insensitive search)
  int cl_idx = hdr_body.indexOf("Content-Length:");
  if (cl_idx == -1)
    cl_idx = hdr_body.indexOf("content-length:");
  if (cl_idx != -1) {
    int cl_val_start = cl_idx + 15; // skip "Content-Length:"
    while (cl_val_start < (int)hdr_body.length() &&
           hdr_body[cl_val_start] == ' ')
      cl_val_start++; // skip spaces
    total_no_of_bytes = hdr_body.substring(cl_val_start).toInt();
    debugf1("[OTA] Content-Length parsed: %d bytes\n", total_no_of_bytes);
  } else {
    // Fallback: try last field of HTTPACTION (may work on some server
    // configs)
    int lc0 = head_resp.lastIndexOf(",");
    if (lc0 != -1)
      total_no_of_bytes = head_resp.substring(lc0 + 1).toInt();
    debugf1("[OTA] Content-Length fallback from HTTPACTION: %d bytes\n",
            total_no_of_bytes);
  }

  SerialSIT.println("AT+HTTPTERM");
  waitForResponse("OK", 2000);

  debugf1("[OTA] File size: %d bytes\n", total_no_of_bytes);
  if (total_no_of_bytes <= 100000) {
    debugln("[OTA] Invalid file size. Aborting.");
    xSemaphoreGive(modemMutex); // v5.70: Fix Mutex Hierarchy - release before NVS
    Preferences p;
    p.begin("ota-track", false);
    p.putInt("fail_cnt", p.getInt("fail_cnt", 0) + 1);
    p.putString("fail_res", "Bad size");
    p.end();
    return;
  }

  // Partition initialization
  flushSerialSIT(); // Hard flush the UART before starting partition write

  // === STEP 2: Begin OTA partition write ===
  if (!Update.begin(total_no_of_bytes, U_FLASH)) {
    // Direct Serial print since ota_silent_mode is true
    Serial.printf("[OTA] Update.begin failed: %s\n", Update.errorString());
    xSemaphoreGive(modemMutex); // v5.70: Fix Mutex Hierarchy - release before NVS
    Preferences p;
    p.begin("ota-track", false);
    p.putInt("fail_cnt", p.getInt("fail_cnt", 0) + 1);
    p.putString("fail_res", "Begin fail: " + String(Update.errorString()));
    p.end();
    ota_silent_mode = false; // Release silence on early-exit failure
    return;
  }
  // BUG-9 Fix: Duplicate URC silence commands removed — already silenced
  // at function entry (lines 3569-3578). No re-silencing needed here.

  Update.onProgress(progressCallBack);
  ota_writing_active = true;
  Serial.println("[OTA] OTA begun. Downloading in 64KB Range chunks...");

  const int RANGE_SIZE =
      32768; // Phase 8 Fix: Shrink to 32KB to avoid fragmentation failures
  const int READ_SIZE = 32768;
  uint8_t *buf = (uint8_t *)malloc(READ_SIZE);
  if (!buf) {
    debugln("[OTA] malloc failed!");
    Update.abort();
    ota_writing_active = false;
    ota_silent_mode = false;  // Fixed: restore silent mode correctly
    xSemaphoreGive(modemMutex); // v5.66: Fix missing mutex release
    return;
  }

  int chunk_retries = 0;
  int consecutive_fails = 0; // Rule 42b: Hardware Reset Trigger
  // Note: ota_silent_mode already set to true before Update.begin() (BUG-3
  // fix)

  // === STEP 3: Download 4KB chunks via Range GET ===
  while (actual_downloaded < total_no_of_bytes) {
    // v7.20: Explicit Offset logging to detect shift-bugs
    debugf("[OTA] Range %d-%d (Progress: %.1f%%)\n", actual_downloaded,
           actual_downloaded + RANGE_SIZE - 1,
           (float)actual_downloaded * 100.0 / total_no_of_bytes);

    if (consecutive_fails >= 3) {
      Serial.println("[OTA] 🚨 3 Consecutive chunk failures! Hard Hardware "
                     "Resetting Modem...");
      digitalWrite(26, LOW);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      esp_task_wdt_reset(); // Keep WDT alive during hardware reset
      digitalWrite(26, HIGH);
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      recoverI2CBus(); // v5.82 Platinum: Mitigate spike on OTA chunk recovery
      esp_task_wdt_reset(); // Keep WDT alive
      consecutive_fails = 0;
      // Re-initialize bearer after hard power cycle
      if (!verify_bearer_or_recover()) {
        esp_task_wdt_reset();
        chunk_retries++;
        continue;
      }
    }

    // v7.52: Reverted to One-Chunk-One-Session (Rule 41) for binary safety,
    // but using 64KB chunks to maintain high throughput.
    // One-Chunk-One-Session (Rule 41) forced re-init
    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 1000);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    if (!verify_bearer_or_recover()) {
      Serial.println("[OTA] ❌ Bearer Recovery Failed");
      chunk_retries++;
      consecutive_fails++;
      if (chunk_retries > 30)
        break;
      esp_task_wdt_reset();
      continue;
    }

    SerialSIT.println("ATE0");
    waitForResponse("OK", 1000);

    SerialSIT.println("AT+HTTPINIT");
    if (waitForResponse("OK", 5000).indexOf("OK") == -1) {
      Serial.println("[OTA] ⚠️ HTTPINIT Failed");
      chunk_retries++;
      consecutive_fails++;
      esp_task_wdt_reset();
      continue;
    }

    SerialSIT.println("AT+HTTPPARA=\"CID\",1");
    waitForResponse("OK", 2000);
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
             "AT+HTTPPARA=\"URL\",\"http://%s:%s/builds/%s\"",
             HEALTH_SERVER_IP, OTA_SERVER_PORT, fileName);
    SerialSIT.println(gprs_xmit_buf);
    waitForResponse("OK", 2000);

    // Set Range
    int r_start = actual_downloaded;
    int r_end = min(actual_downloaded + RANGE_SIZE - 1, total_no_of_bytes - 1);
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
             "AT+HTTPPARA=\"USERDATA\",\"Range: bytes=%d-%d\"", r_start, r_end);
    SerialSIT.println(gprs_xmit_buf);
    waitForResponse("OK", 2000);

    // Execute GET
    Serial.printf("[OTA] Triggering GET Action (Range %d)... ", r_start);
    SerialSIT.println("AT+HTTPACTION=0");
    String act_resp = waitForResponse("+HTTPACTION:", 95000);
    Serial.println(act_resp); // Show the response status code

    int http_status = 0;
    int chunk_total = 0;
    int first_c = act_resp.indexOf(",");
    int last_c = act_resp.lastIndexOf(",");
    if (first_c != -1 && last_c != -1 && last_c > first_c) {
      http_status = act_resp.substring(first_c + 1, last_c).toInt();
      chunk_total = act_resp.substring(last_c + 1).toInt();
    }

    if (http_status != 200 && http_status != 206) {
      Serial.printf("[OTA] ❌ HTTP Error: %d. Resetting session.\n",
                    http_status);
      // Force re-init on next loop
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      chunk_retries++;
      consecutive_fails++;
      esp_task_wdt_reset();
      continue;
    }

    // READ Data - THE SENSITIVE WINDOW
    vTaskDelay(500 / portTICK_PERIOD_MS);
    flushSerialSIT(); // Hard-Flush (Rule 42b)
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+HTTPREAD=0,%d",
             chunk_total);
    SerialSIT.println(gprs_xmit_buf);

    String read_hdr = "";
    unsigned long t_start = millis();
    bool hdr_found = false;
    while (millis() - t_start < 12000) {
      if (SerialSIT.available()) {
        char c = SerialSIT.read();
        read_hdr += c;

        // Safety valve: prevent unbounded string growth on garbage data
        if (read_hdr.length() > 64) {
          break;
        }

        if (c == '\n' && read_hdr.indexOf("+HTTPREAD:") != -1) {
          hdr_found = true;
          break;
        }
        if (read_hdr.indexOf("ERROR") != -1)
          break;
      }
      vTaskDelay(1 / portTICK_PERIOD_MS);
    }

    if (!hdr_found) {
      chunk_retries++;
      consecutive_fails++;
      esp_task_wdt_reset();
      continue;
    }

    int safe_total = min(chunk_total, READ_SIZE);

    // v5.52: Non-blocking Looped Read with WDT resets
    // readBytes is blocking and triggers WDT on slow 2G connections.
    int got = 0;
    unsigned long startRead = millis();
    while (got < safe_total &&
           (millis() - startRead < 120000)) { // 120s timeout
      if (SerialSIT.available()) {
        buf[got++] = SerialSIT.read();
      } else {
        vTaskDelay(1 / portTICK_PERIOD_MS); // Give CPU a break
      }
      if (got % 1024 == 0)
        esp_task_wdt_reset(); // Frequent resets
    }

    if (got != safe_total) {
      Serial.printf("[OTA] ❌ Read Timeout/Incomplete: got %d of %d\n", got,
                    safe_total);
      flushSerialSIT(); // Clear residual bytes from incomplete read
      chunk_retries++;
      consecutive_fails++;
      esp_task_wdt_reset(); // readBytes failure (likely 15s timeout)
      continue;
    }

    // Consume the Tail (The 6-Byte Residual \r\nOK\r\n)
    unsigned long tail_start = millis();
    while (millis() - tail_start < 1000) {
      if (SerialSIT.find("OK"))
        break;
      vTaskDelay(1 / portTICK_PERIOD_MS);
    }

    // Rule 45: The Header-Health Check & Crosstalk Trap
    if (!isBufferSanityOK(buf, got, actual_downloaded)) {
      if (buf[0] == 0x75 && buf[1] == 0x68 && buf[2] == 0x40 &&
          buf[3] == 0x08) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP.restart(); // Signature matched crosstalk, instant restart
      }
      flushSerialSIT(); // BUG-4 Fix: Clear residual UART bytes on sanity fail
      // Something is deeply wrong, reset session
      chunk_retries++;
      consecutive_fails++;
      esp_task_wdt_reset();
      continue;
    }

    // Write to Flash
    int bytes_to_write = min(got, total_no_of_bytes - actual_downloaded);
    // TIER 2 LIVE RACES: WDT timeout guard for 2G
    esp_task_wdt_reset();
    if (Update.write(buf, bytes_to_write) == (size_t)bytes_to_write) {
      actual_downloaded += bytes_to_write;
      chunk_retries = 0;
      consecutive_fails = 0;

      // Progress log (Direct print to bypass silence)
      Serial.printf("[OTA] Progress: %d / %d (HEAD: %02X %02X %02X %02X)\n",
                    actual_downloaded, total_no_of_bytes, buf[0], buf[1],
                    buf[2], buf[3]);
    } else {
      Serial.printf("[OTA] Flash Write ERROR: %d - %s\n", Update.getError(),
                    Update.errorString());
      Update.abort(); // v7.68: Terminate partition session on write failure
      break;
    }

    flushSerialSIT(); // Final tail cleanup
    vTaskDelay(100 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();
    // Force re-init for next chunk (Rule 41)
  }

  free(buf);
  ota_writing_active = false;
  ota_silent_mode = false; // Rule 43: Resume normal logging

  // Restore Modem settings after binary transfer
  SerialSIT.println("ATE1");
  waitForResponse("OK", 500);
  SerialSIT.println("AT+CREG=1");
  waitForResponse("OK", 500);
  SerialSIT.println("AT+CGEREP=2");
  waitForResponse("OK", 500);

  // === STEP 4: Finalize OTA ===
  if (actual_downloaded == total_no_of_bytes && total_no_of_bytes > 0) {
    if (Update.end()) {
      debugln("[OTA] Flash COMPLETE and VERIFIED!");
      if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
        File firm = SPIFFS.open("/firmware.doc", FILE_WRITE);
        if (firm) {
          firm.print(UNIT_VER);
          firm.close();
        }
        xSemaphoreGive(fsMutex);
      }
      Preferences p;
      p.begin("ota-track", false);
      p.putString("status", "success");
      p.end();
      debugln("[OTA] Rebooting in 3s...");
      vTaskDelay(3000 / portTICK_PERIOD_MS);
      ESP.restart();
    } else {
      String err = Update.errorString();
      int errCode = Update.getError();
      debugf("[OTA] Update.end FAILED. Code: %d, Str: %s\n", errCode,
             err.c_str());
      Update.printError(Serial); // BUG-10 Fix: Print to console, not modem UART

      Preferences p;
      p.begin("ota-track", false);
      p.putInt("fail_cnt", p.getInt("fail_cnt", 0) + 1);
      p.putString("fail_res", "End fail code: " + String(errCode));
      p.end();
    }
  } else {
    debugf("[OTA] Incomplete: %d/%d. Aborting.\n", actual_downloaded,
           total_no_of_bytes);
    Update.abort();
    Preferences p;
    p.begin("ota-track", false);
    p.putInt("fail_cnt", p.getInt("fail_cnt", 0) + 1);
    p.putString("fail_res", "Incomplete download");
    p.end();
  }

  // Restore Modem URCs (v7.16 Restoration)
  SerialSIT.println("AT+CREG=1");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CGEREP=2");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CNMI=2,1,0,0,0");
  waitForResponse("OK", 1000);

  // v7.06: Final Cleanup & Restoration
  SerialSIT.println("AT+HTTPTERM");
  waitForResponse("OK", 2000);
  SerialSIT.println("AT+CGEREP=2");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CREG=1");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CEREG=1");
  waitForResponse("OK", 1000);

  debugln("[OTA] Done.");
  xSemaphoreGive(modemMutex);
}

void copyFromSPIFFSToFS(char *dateFile) {
  if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(15000)) != pdTRUE) {
    debugln("[FS] Error: Modem Mutex Timeout - deferring copy");
    diag_modem_mutex_fails++;
    return;
  }
  if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
    debugln("[FS] Error: SPIFFS Mutex Timeout - deferring copy");
    xSemaphoreGive(modemMutex); // RELEASE MODEM MUTEX
    return;
  }
  String response;
  char SPIFFSFile[50], fileName[50];
  char stnId[16];
  if (strlen(station_name) == 4 && isDigitStr(station_name)) {
    snprintf(stnId, sizeof(stnId), "00%s", station_name);
  } else {
    strcpy(stnId, station_name);
  }
  snprintf(SPIFFSFile, sizeof(SPIFFSFile), "/%s_%s.txt", station_name, dateFile);
  snprintf(fileName, sizeof(fileName), "%s_%s.txt", stnId, dateFile);
  debugln("SPIFFS Card FileName is ");
  debugln(SPIFFSFile);
  debugln("FileName is ");
  debugln(fileName);
  int fileSize, s;
  int handle_no;
  char gprs_xmit_buf[300];
  char *csqstr;
  const char *response_char;
  int len;
  // Phase 6 Final Polish: Removed dead unused `String content;` to prevent future shadowing.

  fileSize = 0;
  s = 0; // resetting to 1st record of sd card file ..

  if (SPIFFS.exists(SPIFFSFile)) {
    SerialSIT.println("ATE0");
    response = waitForResponse("OK", 3000);
    SerialSIT.println("AT+FSCD=C:/");
    response = waitForResponse("OK", 10000);
    if (response.indexOf("OK") == -1) {
      debugln("Error changing directory to C:/");
      xSemaphoreGive(modemMutex);
      xSemaphoreGive(fsMutex);
      return; // Exit if response is invalid
    }
    debug("Response of AT+FSCD ");
    debugln(response);

    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSOPEN=C:/%s,0\r\n",
             fileName); // 0 : if the file does not exist, it will be created
    SerialSIT.println(gprs_xmit_buf);
    response = waitForResponse("+FSOPEN", 5000);
    debug("Response is ");
    debugln(response);
    response_char = response.c_str();
    vTaskDelay(200 / portTICK_PERIOD_MS);
    csqstr = strstr(response_char, "+FSOPEN:");
    if (csqstr == NULL) {
      debugln("Error: +FSOPEN not found in response");
      xSemaphoreGive(modemMutex);
      xSemaphoreGive(fsMutex);
      return; // Exit if response is invalid
    }
    sscanf(csqstr, "+FSOPEN: %d,", &handle_no);

    File file1 = SPIFFS.open(SPIFFSFile, FILE_READ);
    if (!file1) {
      debugln("C5 FIX: Failed to open SPIFFS source file. Aborting "
              "copyFromSPIFFSToFS.");
      xSemaphoreGive(modemMutex);
      xSemaphoreGive(fsMutex);
      return;
    }
    debug("File size of the SPIFFS file is ");
    debugln(file1.size());
    fileSize = file1.size();
    file1.seek(s);
    // Phase 5 Fix: Removed content = file1.readString() Memory crash
    
    // filehandle,length,timeout
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSWRITE=%d,%d,30\r\n",
             handle_no,
             fileSize); // 0 : if the file does not exist, it will be created
    vTaskDelay(200 / portTICK_PERIOD_MS);
    SerialSIT.println(gprs_xmit_buf);
    response = waitForResponse("CONNECT", 5000);
    
    // Chunk flow directly to modem UART to bypass 100KB RAM fragmentation
    char file_buf[256];
    int bytes_streamed = 0;
    while (file1.available()) {
      int bytesRead = file1.read((uint8_t*)file_buf, 255);
      SerialSIT.write((uint8_t*)file_buf, bytesRead);
      bytes_streamed += bytesRead;
      if (bytes_streamed % (16 * 1024) == 0) esp_task_wdt_reset(); // Phase 6 Review: Prevent Watchdog Trips
    }
    SerialSIT.println();
    response = waitForResponse("+FSWRITE", 5000);
    debug("Response of FSWRITE is ");
    debugln(response);
    file1.close();

    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSCLOSE=%d",
             handle_no); // FILEHANDLE,offset,(0:start of file 1:cur pos of
                         // pointer 2: end of file
    SerialSIT.println(gprs_xmit_buf);
    response = waitForResponse("OK", 5000); // Fixed: FSCLOSE responds with OK, not URC
    vTaskDelay(200 / portTICK_PERIOD_MS);

    // v5.65 Fix: Pass default mode 1 (Passive) for compatibility
    ftp_login_flag = setup_ftp(1);

    if (ftp_login_flag == 1) {
      char *modulePath = (fileName[0] == '/') ? &fileName[1] : fileName;
      snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CFTPSPUTFILE=\"%s\",1",
               modulePath); 
      SerialSIT.println(gprs_xmit_buf); // FTP client context
      // v5.65 WDT-SAFE: Active poll instead of blind 150s wait (prevents WDT reset on slow BSNL)
      String put_resp = ""; // v5.70: for Race Guard
      {
        unsigned long put_start = millis();
        while ((millis() - put_start) < 150000) {
          esp_task_wdt_reset();
          vTaskDelay(500 / portTICK_PERIOD_MS);
          while (SerialSIT.available()) {
            char cc = SerialSIT.read();
            if (put_resp.length() < 2048) put_resp += cc;
          }
          if (put_resp.indexOf("+CFTPSPUTFILE: 0") != -1) break;
        }
      }
      debug("Response of AT+CFTPSPUTFILE: ");
      debugln(put_resp);
      
      // v7.70+: Only continue if success code '0' was received
      if (put_resp.indexOf("+CFTPSPUTFILE: 0") != -1) {
         // Proceed with logout/cleanup logic as before
      } else {
         debugln("[FTP] PUT failed. Skipping cleanup.");
         // v5.70: Fix C-03 FTP Put Race (OTA/copyFromSPIFFSToFS path)
         if (put_resp.indexOf("+CFTPSPUTFILE:") == -1) {
             debugln("[FTP] Race Guard (OTA): Waiting 1s drain before logout.");
             vTaskDelay(1000 / portTICK_PERIOD_MS);
         }
      }

      SerialSIT.println("AT+CFTPSLOGOUT");
      response = waitForResponse("+CFTPSLOGOUT: 0", 9000);
      debug("Response of AT+CFTPSLOGOUT is ");
      debugln(response);
      vTaskDelay(200 / portTICK_PERIOD_MS);

      SerialSIT.println("AT+CFTPSSTOP");
      response = waitForResponse("OK", 5000);
      debug("Response of AT+CFTPSSTOP is ");
      debugln(response);
      vTaskDelay(200 / portTICK_PERIOD_MS);

      snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSDEL=\"C:/%s\"",
               fileName); // 0 : if the file does not exist, it will be created
      SerialSIT.println(gprs_xmit_buf);
      response = waitForResponse("OK", 10000);
      debug("Response of AT+FSDEL is  ");
      debugln(response);
    } else {
      debugln("FTP Login unsuccessful");
    }
  } else {
    debugln("File not found");
  }
  xSemaphoreGive(fsMutex); // v5.66 BUGFIX: Was missing on the success path!
  xSemaphoreGive(modemMutex);
}

/*
 * COMMON FUNCTIONS
 * - parse_http_head()
 * - waitForResponse()
 * - read_line()
 * - resync_time()
 */

char *parse_http_head(char *response, char *check) {
  char *search_str = NULL;
  char *token = strtok(response, "\r\n");
  while (token != NULL) {
    if (strstr(token, check) != NULL) {
      search_str = strchr(token, ':') +
                   2; // eg. Skip the "+CSQ: " prefix ... +2 to skip \r\n
      break;
    }
    token = strtok(NULL, "\r\n");
  }
  return search_str;
}

void send_daily_file(
    const char *dateStr) { // v5.80: Send /dailyftp_YYYYMMDD.txt
  char dailyFile[32];
  snprintf(dailyFile, sizeof(dailyFile), "/dailyftp_%s.txt", dateStr);

  if (SPIFFS.exists(dailyFile)) {
    debugf1("[FTP] Found daily file: %s. Initiating upload...\n", dailyFile);
    // Signal guard: skip 24-retry loop if signal already known to be dead
    if (signal_lvl <= -98) {
      debugln("[FTP] Skip Daily: Signal too weak (" + String(signal_lvl) +
              " dBm).");
      return;
    }
    get_registration();
    if (gprs_mode == eGprsSignalOk) {
      get_a7672s();
      vTaskDelay(3000 / portTICK_PERIOD_MS);
      esp_task_wdt_reset();
      send_ftp_file(dailyFile, true);
    } else {
      debugln("[FTP] Skip Daily: No Network.");
    }
  } else {
    debugf1("[FTP] Daily file NOT FOUND: %s\n", dailyFile);
  }
}

