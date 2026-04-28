import re

new_func = r"""void fetchFromHttpAndUpdate(char *fileName, bool alreadyLocked) {
  if (!alreadyLocked) {
    if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(30000)) != pdTRUE) {
      debugln("[OTA] Error: Modem Mutex Timeout - deferring download");
      diag_modem_mutex_fails++;
      return;
    }
  }

  int handle_no;
  char gprs_xmit_buf[300];
  int actual_downloaded = 0;
  (void)handle_no; 

  debugf1("[OTA] Starting Range-GET download for: %s\n", fileName);

  const int CHUNK_SIZE = 32768;
  uint8_t *buf = (uint8_t *)malloc(CHUNK_SIZE);
  if (!buf) {
    debugln("[OTA] malloc failed!");
    if (!alreadyLocked) xSemaphoreGive(modemMutex);
    return;
  }

  // === ONE-TIME SESSION INIT ===
  SerialSIT.println("AT+HTTPTERM");
  waitForResponse("OK", 2000);

  if (!verify_bearer_or_recover()) {
    Serial.println("[OTA] [ERR] Initial Bearer Check Failed");
    free(buf);
    if (!alreadyLocked) xSemaphoreGive(modemMutex);
    return;
  }

  SerialSIT.println("ATE0");
  waitForResponse("OK", 1000);

  SerialSIT.println("AT+HTTPINIT");
  if (!waitForResponse("OK", 10000)) { 
    Serial.println("[OTA] [ERR] HTTPINIT Failed");
    free(buf);
    if (!alreadyLocked) xSemaphoreGive(modemMutex);
    return;
  }

  SerialSIT.println("AT+HTTPPARA=\"CID\",1");
  waitForResponse("OK", 2000);

  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
           "AT+HTTPPARA=\"URL\",\"http://%s:%s/builds/%s\"",
           HEALTH_SERVER_IP, OTA_SERVER_PORT, fileName);
  SerialSIT.println(gprs_xmit_buf);
  waitForResponse("OK", 2000);

  SerialSIT.println("AT+HTTPPARA=\"RECVHDR\",0");
  waitForResponse("OK", 1000);

  SerialSIT.println("AT+HTTPPARA=\"CONTENT\",0");
  waitForResponse("OK", 1000);

  SerialSIT.println("AT+CREG=0");  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CGREG=0"); waitForResponse("OK", 1000);
  SerialSIT.println("AT+CEREG=0"); waitForResponse("OK", 1000);
  SerialSIT.println("AT+CGEV=0");  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CGEREP=0");waitForResponse("OK", 1000);
  SerialSIT.println("AT+CNMI=0,0,0,0,0"); waitForResponse("OK", 1000);
  SerialSIT.println("AT+CIURC=0"); waitForResponse("OK", 1000);

  flushSerialSIT();

  if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
    Serial.printf("[OTA] Update.begin failed: %s\n", Update.errorString());
    if (!alreadyLocked) xSemaphoreGive(modemMutex);
    Preferences p;
    p.begin("ota-track", false);
    p.putInt("fail_cnt", p.getInt("fail_cnt", 0) + 1);
    char err_buf[64];
    snprintf(err_buf, sizeof(err_buf), "Begin fail: %s", Update.errorString());
    p.putString("fail_res", err_buf);
    p.end();
    free(buf);
    return;
  }

  Update.onProgress(progressCallBack);
  ota_silent_mode = true;
  ota_writing_active = true;
  Serial.printf("[OTA] OTA begun. Downloading in %d byte chunks...\n", CHUNK_SIZE);

  int chunk_retries = 0;
  int consecutive_fails = 0;

  // === CHUNK LOOP ===
  while (true) {
    debugf("[OTA] Range Request starting at: %d\n", actual_downloaded);
    last_activity_time = millis();

    if (chunk_retries > 40) {
      Serial.println("[OTA] [ABORT] Too many total retries. Giving up.");
      break;
    }

    if (consecutive_fails >= 3) {
      Serial.println("[OTA] [CRIT] 3 Consecutive failures! Resetting Session & Modem...");
      digitalWrite(26, LOW);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      esp_task_wdt_reset();
      digitalWrite(26, HIGH);
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      recoverI2CBus();
      esp_task_wdt_reset();
      consecutive_fails = 0;
      
      if (!verify_bearer_or_recover()) {
        esp_task_wdt_reset();
        chunk_retries++;
        continue;
      }

      SerialSIT.println("ATE0"); waitForResponse("OK", 1000);
      SerialSIT.println("AT+HTTPINIT"); waitForResponse("OK", 10000);
      SerialSIT.println("AT+HTTPPARA=\"CID\",1"); waitForResponse("OK", 2000);
      snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
               "AT+HTTPPARA=\"URL\",\"http://%s:%s/builds/%s\"",
               HEALTH_SERVER_IP, OTA_SERVER_PORT, fileName);
      SerialSIT.println(gprs_xmit_buf); waitForResponse("OK", 2000);
      SerialSIT.println("AT+HTTPPARA=\"RECVHDR\",0"); waitForResponse("OK", 1000);
      SerialSIT.println("AT+HTTPPARA=\"CONTENT\",0"); waitForResponse("OK", 1000);
      
      SerialSIT.println("AT+CREG=0"); waitForResponse("OK", 1000);
      SerialSIT.println("AT+CGREG=0"); waitForResponse("OK", 1000);
      SerialSIT.println("AT+CEREG=0"); waitForResponse("OK", 1000);
      SerialSIT.println("AT+CGEV=0"); waitForResponse("OK", 1000);
      SerialSIT.println("AT+CGEREP=0"); waitForResponse("OK", 1000);
      SerialSIT.println("AT+CNMI=0,0,0,0,0"); waitForResponse("OK", 1000);
      SerialSIT.println("AT+CIURC=0"); waitForResponse("OK", 1000);
    }

    int r_start = actual_downloaded;
    int r_end = actual_downloaded + CHUNK_SIZE - 1;
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
             "AT+HTTPPARA=\"USERDATA\",\"Range: bytes=%d-%d\"", r_start, r_end);
    SerialSIT.println(gprs_xmit_buf);
    waitForResponse("OK", 2000);

    Serial.printf("[OTA] Triggering GET Action (Range %d)... ", r_start);
    Serial.printf("[OTA] T1: HTTPACTION started at %lu\n", millis());
    SerialSIT.println("AT+HTTPACTION=0");
    if (!waitForResponse("+HTTPACTION:", 95000)) {
       Serial.println("[OTA] [ERR] HTTPACTION Timeout");
       chunk_retries++; consecutive_fails++;
       continue;
    }
    Serial.println(modem_response_buf);

    int http_status = 0;
    int chunk_total = 0;
    const char *first_c = strchr(modem_response_buf, ',');
    const char *last_c = strrchr(modem_response_buf, ',');
    if (first_c != NULL && last_c != NULL && last_c > first_c) {
      http_status = atoi(first_c + 1);
      chunk_total = atoi(last_c + 1);
    }

    if (http_status != 200 && http_status != 206) {
      if (http_status == 416) {
          Serial.printf("[OTA] HTTP 416: End of file reached at %d bytes. Download complete.\n",
                        actual_downloaded);
          break;
      }
      Serial.printf("[OTA] [ERR] HTTP Error: %d. Retrying.\n", http_status);
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      chunk_retries++; consecutive_fails++; esp_task_wdt_reset();
      continue;
    }

    if (chunk_total == 0) {
        Serial.printf("[OTA] Empty chunk. Download complete at %d bytes.\n", actual_downloaded);
        break;
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
    Serial.printf("[OTA] T2: Pre-HTTPREAD delay done at %lu\n", millis());
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+HTTPREAD=0,%d", chunk_total);
    SerialSIT.println(gprs_xmit_buf);

    char read_hdr[64] = {0};
    int rh_idx = 0;
    bool hdr_found = false;
    unsigned long t_start = millis();

    while (millis() - t_start < 15000) {
        esp_task_wdt_reset();
        if (!SerialSIT.available()) { vTaskDelay(2 / portTICK_PERIOD_MS); continue; }
        
        char c = SerialSIT.read();
        if (rh_idx < 63) read_hdr[rh_idx++] = c;
        
        if (c == '\n') {
            if (strstr(read_hdr, "+HTTPREAD:") != NULL) {
                hdr_found = true;
                break;
            }
            rh_idx = 0;
            memset(read_hdr, 0, sizeof(read_hdr));
        }
        if (strstr(read_hdr, "ERROR") != NULL) break;
    }

    if (!hdr_found) {
      Serial.println("[OTA] [WARN] HTTPREAD Header Timeout/Error. Cooling down...");
      vTaskDelay(3000 / portTICK_PERIOD_MS);
      chunk_retries++; consecutive_fails++; esp_task_wdt_reset();
      continue;
    }

    Serial.printf("[OTA] HDR_PARSED: hdr_buf='%s' len=%d UART_avail=%d\n", read_hdr, rh_idx, SerialSIT.available());
    Serial.printf("[OTA] T3: Header parsed at %lu, UART_now=%d\n", millis(), SerialSIT.available());

    int got = 0;
    unsigned long startRead = millis();
    while (got < chunk_total && (millis() - startRead < 120000)) {
        if (got % 512 == 0) esp_task_wdt_reset();
        if (SerialSIT.available()) {
            buf[got++] = SerialSIT.read();
        } else {
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }
    }

    if (got != chunk_total) {
        Serial.printf("[OTA] [ERR] Read Timeout/Incomplete: got %d of %d\n", got, chunk_total);
        chunk_retries++; consecutive_fails++; esp_task_wdt_reset();
        continue;
    }

    Serial.printf("[OTA] T4: Binary read complete at %lu\n", millis());

    unsigned long tail_start = millis();
    int ti = 0;
    char expected_tail[] = "\r\nOK\r\n";
    while (ti < 6 && millis() - tail_start < 3000) {
        if (SerialSIT.available()) {
            char tc = SerialSIT.read();
            if (tc == expected_tail[ti]) {
                ti++;
            } else {
                ti = 0; // Reset if mismatch
            }
        } else {
            vTaskDelay(5 / portTICK_PERIOD_MS);
        }
        esp_task_wdt_reset();
    }

    Serial.printf("[OTA] T5: Tail-Guard check at %lu, UART_avail=%d\n", millis(), SerialSIT.available());
    if (SerialSIT.available() > 10) {
        Serial.printf("[OTA] [ERR] Tail-Guard triggered! Data: %d bytes.\n", SerialSIT.available());
        flushSerialSIT();
        chunk_retries++; consecutive_fails++;
        continue;
    }

    if (!isBufferSanityOK(buf, got, actual_downloaded)) {
      if (buf[0] == 0x75 && buf[1] == 0x68 && buf[2] == 0x40 && buf[3] == 0x08) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP.restart(); 
      }
      flushSerialSIT(); 
      chunk_retries++; consecutive_fails++; esp_task_wdt_reset();
      continue;
    }

    esp_task_wdt_reset();
    if (Update.write(buf, got) == (size_t)got) {
      actual_downloaded += got;
      chunk_retries = 0;
      consecutive_fails = 0;

      uint32_t chk = 0;
      for (int i = 0; i < got; i++) chk += buf[i];
      Serial.printf("[OTA] Chunk written. Offset=%d Size=%d Checksum=%lu FirstByte=0x%02X LastByte=0x%02X\n",
                     actual_downloaded - got, got, chk, buf[0], buf[got-1]);

      Serial.printf("[OTA] Progress: %d bytes written (HEAD: %02X %02X %02X %02X)\n",
                    actual_downloaded, buf[0], buf[1], buf[2], buf[3]);
                    
      if (chunk_total < CHUNK_SIZE) {
          Serial.printf("[OTA] Last chunk received (%d < %d). Download complete at %d bytes.\n",
                        chunk_total, CHUNK_SIZE, actual_downloaded);
          break;
      }
    } else {
      Serial.printf("[OTA] Flash Write ERROR: %d - %s\n", Update.getError(), Update.errorString());
      Update.abort(); 
      break;
    }

    vTaskDelay(50 / portTICK_PERIOD_MS); 
    esp_task_wdt_reset();
  }

  free(buf);
  ota_writing_active = false;
  ota_silent_mode = false; 

  SerialSIT.println("ATE1"); waitForResponse("OK", 500);
  SerialSIT.println("AT+CREG=1"); waitForResponse("OK", 500);
  SerialSIT.println("AT+CGEREP=2"); waitForResponse("OK", 500);
  SerialSIT.println("AT+CNMI=2,1,0,0,0"); waitForResponse("OK", 1000);

  if (actual_downloaded > 100000) { 
    if (Update.hasError()) {
        debugf("[OTA] Latent Error detected before end: %s. Aborting.\n", Update.errorString());
        Update.abort();
    } else if (Update.end(true)) { 
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
      int errCode = Update.getError();
      debugf("[OTA] Update.end FAILED. Code: %d, Str: %s\n", errCode, Update.errorString());
      Update.printError(Serial); 

      Preferences p;
      p.begin("ota-track", false);
      p.putInt("fail_cnt", p.getInt("fail_cnt", 0) + 1);
      char res_buf[32];
      snprintf(res_buf, sizeof(res_buf), "End fail code: %d", errCode);
      p.putString("fail_res", res_buf);
      p.end();
    }
  } else {
    debugf("[OTA] Incomplete: downloaded %d bytes. Aborting.\n", actual_downloaded);
    Update.abort();
    Preferences p;
    p.begin("ota-track", false);
    p.putInt("fail_cnt", p.getInt("fail_cnt", 0) + 1);
    p.putString("fail_res", "Incomplete download");
    p.end();
  }

  SerialSIT.println("AT+HTTPTERM");
  waitForResponse("OK", 2000);

  debugln("[OTA] Done.");
  if (!alreadyLocked) xSemaphoreGive(modemMutex);
}"""

with open('gprs_ftp.ino', 'r') as f:
    content = f.read()

# Replace exactly from void fetchFromHttpAndUpdate... up to the first closing brace that matches the end of the old function.
# The safest way is to split the file before and after the function.
start_idx = content.find("void fetchFromHttpAndUpdate(char *fileName, bool alreadyLocked) {")
# Find the next function signature to know where it ended.
end_idx = content.find("void copyFromSPIFFSToFS(char *dateFile, bool alreadyLocked) {")

if start_idx != -1 and end_idx != -1:
    final_content = content[:start_idx] + new_func + "\n\n" + content[end_idx:]
    with open('gprs_ftp.ino', 'w') as f:
        f.write(final_content)
    print("Function replaced successfully!")
else:
    print("Could not find start or end index.")
