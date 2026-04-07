#include "globals.h"

void send_ftp_file(char *fileName, bool isDailyFTP, bool alreadyLocked) {
  int modem_ftp_handle = 0; // Turner-Fix: Persistent handle scope to prevent leak (C-04)
  int saved_send_daily = send_daily; // Backup to prevent credential-loss on retry
  bool cid9_bounced = false; // Turner-Fix
  // v7.70+: ABBA DEADLOCK FIX — Strictly enforce hierarchy: modemMutex [INFO] fsMutex
  char put_buf[256] = {0}; 
  bool success = true; // v5.76.5 Protocol Armor Flag
  char *modulePath = (fileName[0] == '/') ? &fileName[1] : fileName;
  if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(20000)) != pdTRUE) {
      debugln("[FTP] Mutex Timeout: modemMutex busy.");
      return;
  }

  // v7.70: Release fsMutex BEFORE entering modem setup (Deadlock Prevent)
  // alreadyLocked is ignored now; helper functions will take what they need.
  bool weLockedFS = false; 
  // Function logic proceeds lock-free; file accesses will take fsMutex locally.

  const char *response_char;
  const char *csqstr;
  char gprs_xmit_buf[300];
  int handle_no = 0, fileSize = 0; // v5.76: Explicitly initialized

  flushSerialSIT(); 
  debugln("Initializing A7672S for FTP...");
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  bool fileExists = false;
  bool ftp_started = false; // v5.70 Hardened: [N-8] Guard cleanup drain on skipped files

  // v5.85.1: Proactive SPIFFS Orphan Cleanup - Before FTP starts, sweep old staging files
  const char *pattern = strstr(UNIT, "SPATIKA") ? ".swd" : ".kwd";
  char kwdFiles[12][64]; 
  int kwdCount = 0;
  
  if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
      File rootDir = SPIFFS.open("/");
      if (rootDir) {
          File f = rootDir.openNextFile();
          while (f && kwdCount < 12) {
              const char* n = f.name();
              int nLen = strlen(n);
              int pLen = strlen(pattern);
              
              bool match = false;
              if (nLen >= pLen && strcmp(n + nLen - pLen, pattern) == 0) {
                 // Check if it's not the current file
                 if (n[0] == '/') {
                    if (strcmp(n, fileName) != 0) match = true;
                 } else {
                    char fullPath[64];
                    snprintf(fullPath, sizeof(fullPath), "/%s", n);
                    if (strcmp(fullPath, fileName) != 0) match = true;
                 }
              }

              if (match) {
                 if (n[0] == '/') strncpy(kwdFiles[kwdCount++], n, 63);
                 else snprintf(kwdFiles[kwdCount++], 64, "/%s", n);
              }
              f.close();
              f = rootDir.openNextFile();
          }
          rootDir.close();
      }
      
      // Now check if our actual target file exists
      fileExists = SPIFFS.exists(fileName);
      xSemaphoreGive(fsMutex);
  }

  // Delete outside the main mutex hold, taking it individually for each file
  // This prevents blocking out the scheduler task for 500ms+ during cleanup
  for (int i = 0; i < kwdCount; i++) {
      if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(2000)) == pdTRUE) {
          SPIFFS.remove(kwdFiles[i]);
          debugf("[FTP] Purged orphaned staging file: %s\n", kwdFiles[i]);
          xSemaphoreGive(fsMutex);
      }
  }

  if (fileExists) {
    send_daily = 2;
    int initial_ftp_mode = 0; 
    if (preferred_ftp_mode != -1) {
       initial_ftp_mode = preferred_ftp_mode;
       debugf("[FTP] Using SAVED Smart Mode: %s\n", (initial_ftp_mode == 1 ? "Passive" : "Active"));
    } else if (strstr(carrier, "AIRTEL") || strstr(carrier, "Airtel") || strstr(carrier, "Jio")) {
      initial_ftp_mode = 1; 
      debugln("[FTP] Airtel/Jio detected. Smart Default: Passive (1).");
    } else {
      initial_ftp_mode = 0; // v13.10: Reverted to Golden v5.70 default (Active) for BSNL
      debugln("[FTP] BSNL/Other detected. Smart Default: Active (0).");
    }
    
    // v5.85: [BSNL Shield] - Verify modem's internet session is still alive after HTTP context drop
    verify_bearer_or_recover();
    
    // v5.85: Proactive Modem FS Cleanup to prevent C:/ Disk Full (FSOPEN ERROR)
    // We try to delete the specific file if it exists from a prior failed run
    char *targetFile = (fileName[0] == '/') ? &fileName[1] : fileName;
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSDEL=\"C:/%s\"", targetFile);
    SerialSIT.println(gprs_xmit_buf);
    waitForResponse("OK", 2000);
    
    ftp_login_flag = setup_ftp(initial_ftp_mode);
    if (ftp_login_flag == 0) {
        debugln("[FTP] First Login attempt failed. BSNL Shield: Forcing Context Refresh...");
        
        // [FTP-05]: BSNL NAT Port Clear - Tightened Delays in v5.76.3
        SerialSIT.println("AT+CGACT=0,1");
        waitForResponse("OK", 2000);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        SerialSIT.println("AT+CGACT=1,1");
        waitForResponse("OK", 8000); // Reduced from 15s
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
        // Retry with same mode (Active for BSNL)
        ftp_login_flag = setup_ftp(initial_ftp_mode);
    }
    
    if (ftp_login_flag == 1) {
      ftp_started = true;
      // v13.8: Removed ATE0 — sending AT commands after CFTPSLOGIN may disrupt A7672S FTP state
      SerialSIT.println("AT+FSCD=C:/");
      waitForResponse("OK", 10000);

      char *modulePath = (fileName[0] == '/') ? &fileName[1] : fileName;

            File file1;
            if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
                file1 = SPIFFS.open(fileName, FILE_READ);
                if (file1) {
                    fileSize = file1.size();
                    debugf("[FTP] File size: %d\n", fileSize);
                    if (fileSize == 0) {
                        debugln("[FTP] ABORT: Source file is 0 bytes. Skipping transfer.");
                        file1.close();
                        xSemaphoreGive(fsMutex);
                        goto ftp_cleanup;
                    }
                    if (fileSize > 0) {
                        debugf("[FTP] FSDEL path: %s\n", modulePath);
                        snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSDEL=\"C:/%s\"", modulePath);
                         SerialSIT.println(gprs_xmit_buf); 
                         waitForResponse("OK", 3000); // Clean up old staging file
 
                         snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSOPEN=\"C:/%s\",0", modulePath);
                         debugf("[FTP] FSOPEN cmd: %s\n", gprs_xmit_buf);
                        SerialSIT.println(gprs_xmit_buf);
                        handle_no = 0;
                        if (waitForResponse("OK", 10000)) {
                            const char* respPtr = modem_response_buf;
                            const char* handleIdx = strstr(respPtr, "+FSOPEN: ");
                            if (handleIdx != NULL) {
                                handle_no = atoi(handleIdx + 9);
                                debugf("[FTP] Found FH: %d\n", handle_no);
                            }
                        }

                        if (handle_no > 0) {
                            snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSWRITE=%d,%d,30", handle_no, fileSize);
                            debugln("[FTP] Sending FSWRITE command...");
                            vTaskDelay(200 / portTICK_PERIOD_MS);
                            SerialSIT.println(gprs_xmit_buf);
                            bool write_ready = waitForResponse("CONNECT", 7000);
                            
                            if (write_ready) {
                                flushSerialSIT(); // [FTP-02] Fix: Drain echo residue before streaming
                                // Stream data directly
                                File rf = SPIFFS.open(fileName, FILE_READ);
                                if (rf) {
                                    uint8_t buf[256];
                                    while (rf.available()) {
                                        size_t len = rf.read(buf, sizeof(buf));
                                        SerialSIT.write(buf, len);
                                        vTaskDelay(1); // Allow UART buffer to breathe
                                    }
                                    rf.close();
                                    debugln("[FTP] Data stream finished.");
                                    waitForResponse("+FSWRITE", 10000);
                                    
                                    // MANDATORY: Close handle so FTP stack can read the file
                                    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSCLOSE=%d", handle_no);
                                    SerialSIT.println(gprs_xmit_buf);
                                    waitForResponse("OK", 3000);
                                    handle_no = 0; // Mark as released
                                }
                            } else {
                                debugln("[FTP] [ERR] FSWRITE failed to prompt (CONNECT). Aborting current attempt.");
                                if (handle_no > 0) {
                                    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSCLOSE=%d", handle_no);
                                    SerialSIT.println(gprs_xmit_buf);
                                    waitForResponse("OK", 2000);
                                }
                                success = false; // Force retry
                            }
                        } else {
                             // v5.76.7: Treat any other response (timeout/garbage) as failure
                             debugln("[FTP] [ERR] FSOPEN timeout or unrecognized error. Retrying.");
                             success = false;
                        }
                    }

                    file1.close(); // v5.76.7 Fix: Close SPIFFS handle regardless of staging success
                    if (success) {
                        debugln("[FTP] Dispatching PUTFILE command...");
                        snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CFTPSPUTFILE=\"%s\",1", modulePath);
                    }
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

        // v5.76: Buffer isolated below in the retry loop.

        // v13.6: Detect CID 9 bounce — modem FTP stack corrupted. Restart before PUTFILE.
        // Drain UART for any pending URCs from the setup/login phase
        {
          char probe[64] = {0}; int pi = 0;
          unsigned long drain_start = millis();
          while ((millis() - drain_start) < 1000) {
            while (SerialSIT.available() && pi < 63) { probe[pi++] = (char)SerialSIT.read(); probe[pi] = '\0'; }
            vTaskDelay(50 / portTICK_PERIOD_MS);
          }
          // v13.7: Broaden CID probe — CID 8 or DEACT also disrupts FTP stack
          if (strstr(probe, "+CGEV: ME PDN DEACT") || strstr(probe, "+CGEV: ME PDN ACT 8") || strstr(probe, "+CGEV: NW PDN DEACT")) {
            debugln("[FTP] PDN Event (DEACT/ACT8) detected after login. Restarting FTP stack...");
            cid9_bounced = true;
            if (setup_ftp(initial_ftp_mode) == 1) {
              // Re-stage file after stack restart
              SerialSIT.println("AT+FSCD=C:/");
              waitForResponse("OK", 3000);
              snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSDEL=\"C:/%s\"", modulePath);
              SerialSIT.println(gprs_xmit_buf); waitForResponse("OK", 2000);
              snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSOPEN=\"C:/%s\",0\r\n", modulePath);
              SerialSIT.println(gprs_xmit_buf);
              modem_ftp_handle = 0;
              if (waitForResponse("+FSOPEN", 5000)) {
                 const char *rp = strstr(modem_response_buf, "+FSOPEN:");
                 if (rp) {
                   sscanf(rp, "+FSOPEN: %d,", &modem_ftp_handle);
                 }
              }
              if (modem_ftp_handle > 0) {
                  File rrf = SPIFFS.open(fileName, FILE_READ);
                  if (rrf && rrf.size() > 0) {
                    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSWRITE=%d,%d,30\r\n", modem_ftp_handle, (int)rrf.size());
                    SerialSIT.println(gprs_xmit_buf);
                    if (waitForResponse("CONNECT", 5000)) {
                      char rb[256]; while (rrf.available()) { int n = rrf.read((uint8_t*)rb,255); SerialSIT.write((uint8_t*)rb,n); }
                      waitForResponse("+FSWRITE", 5000);
                    }
                  }
                  if (rrf) rrf.close();
                  xSemaphoreGive(fsMutex);
                }
                snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSCLOSE=%d", modem_ftp_handle);
                SerialSIT.println(gprs_xmit_buf); waitForResponse("OK", 3000);
                modem_ftp_handle = 0; // Clear after success
              } else { debugln("[FTP] CID9 restart: FSOPEN failed. Giving up."); goto ftp_cleanup; }
              snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CFTPSPUTFILE=\"%s\",1", modulePath);
            } else { debugln("[FTP] CID9 restart: Re-login failed. Giving up."); goto ftp_cleanup; }
          }
        }

        int ftp_put_retries = 0;
        bool upload_success = false;
        // v5.76: cid9_bounced moved to function top
        
        // v5.76.3: Refined recovery and breather
        if (cid9_bounced) vTaskDelay(3000 / portTICK_PERIOD_MS); 
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        
        while (ftp_put_retries <= 1) {
          esp_task_wdt_reset();
          if (!verify_bearer_or_recover()) break;

          debugln("[FTP] Dispatching PUTFILE command...");
          // v5.76.7 Elite: Naming compatibility with v3.0 scripts (now .txt by construction)
          snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CFTPSPUTFILE=\"%s\",1", modulePath);
          SerialSIT.println(gprs_xmit_buf);
          memset(put_buf, 0, sizeof(put_buf));
          int put_idx = 0;
          unsigned long put_start = millis();
          while ((millis() - put_start) < 60000) {
            esp_task_wdt_reset();
            vTaskDelay(200 / portTICK_PERIOD_MS);
            while (SerialSIT.available() && put_idx < 255) {
                put_buf[put_idx++] = (char)SerialSIT.read();
                put_buf[put_idx] = '\0';
            }
            if (strstr(put_buf, "+CFTPSPUTFILE:")) break;     // success or error code
            if (strstr(put_buf, "\r\nERROR\r\n")) break;       // v13.6: bare rejection — don't wait 60s
            if (strstr(put_buf, "+CGEV: ME PDN DEACT")) break; // bearer truly dropped mid-transfer
          }
          debugf("Response of AT+CFTPSPUTFILE: %s\n", put_buf);

          if (strstr(put_buf, "+CFTPSPUTFILE: 0") != NULL) {
            upload_success = true;
            preferred_ftp_mode = initial_ftp_mode; // Keep current mode — it worked
            if (isDailyFTP) strcpy(last_cmd_res, "Success: Daily FTP OK");
            else strcpy(last_cmd_res, "Success: Backlog FTP OK");
            break;
          } else {
            // v13.6: Only retry if CID 9 didn't already bounce (we already did one restart above)
            // v5.75: HYBRID SMART FTP RETRY
            // If the chosen mode crashes or rejects, flip the mode dynamically (Active -> Passive, or Passive -> Active).
            if (ftp_put_retries < 1 && !cid9_bounced) {
              // v5.75: BSNL NAT Shield — Force PASSIVE mode on retry if BSNL detected (CGNAT Safe)
              int retry_mode = (strstr(carrier, "BSNL")) ? 1 : (1 - initial_ftp_mode);
              debugf1("[FTP] PUT failed. Re-staging and retrying in mode (%d)...\n", retry_mode);
              verify_bearer_or_recover();
              SerialSIT.println("AT+CFTPSLOGOUT"); // v5.75+: Standard Logout before stack reset
              waitForResponse("+CFTPSLOGOUT: 0", 3000);
              send_daily = saved_send_daily; // Restore credentials for retry
              if (setup_ftp(retry_mode) != 1) { break; }
              
              // Give the modem filesystem an extra breath to mount if it just rebooted
              vTaskDelay(2000 / portTICK_PERIOD_MS); 
              
              SerialSIT.println("AT+FSCD=C:/");
              waitForResponse("OK", 3000);
              snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSDEL=\"C:/%s\"", modulePath);
              debugln(gprs_xmit_buf); // Log it for visibility
              SerialSIT.println(gprs_xmit_buf); waitForResponse("OK", 2000);
              
              snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSOPEN=\"C:/%s\",0\r\n", modulePath);
              debugln(gprs_xmit_buf); // Log it for visibility
              SerialSIT.println(gprs_xmit_buf);
              modem_ftp_handle = 0;
              if (waitForResponse("+FSOPEN", 5000)) {
                  const char *fso = strstr(modem_response_buf, "+FSOPEN:");
                  if (fso != NULL) {
                      sscanf(fso, "+FSOPEN: %d,", &modem_ftp_handle);
                  }
              }
              if (modem_ftp_handle == 0) { debugln("[FTP] Re-stage: FSOPEN failed. Giving up."); break; }
              if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
                File rf = SPIFFS.open(fileName, FILE_READ);
                if (rf && rf.size() > 0) {
                  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSWRITE=%d,%d,30", modem_ftp_handle, (int)rf.size());
                  debugf("[FTP] Retry FSWRITE cmd: %s\n", gprs_xmit_buf);
                  SerialSIT.println(gprs_xmit_buf);
                  bool retry_write_ready = waitForResponse("CONNECT", 7000);
                  if (retry_write_ready) {
                    char rb[256]; 
                    while (rf.available()) { 
                      size_t l = rf.read((uint8_t*)rb, sizeof(rb)); 
                      SerialSIT.write((uint8_t*)rb, l); 
                      vTaskDelay(1);
                    } 
                    rf.close(); 
                    debugln("[FTP] Retry stream complete.");
                    // v5.76.7: Wait for modem to commit flash before closing handle
                    waitForResponse("+FSWRITE", 10000); 
                    
                    // MANDATORY: Close handle before PUTFILE
                    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSCLOSE=%d", modem_ftp_handle);
                    SerialSIT.println(gprs_xmit_buf);
                    waitForResponse("OK", 3000);
                    modem_ftp_handle = 0;

                    // Final ATOMIC PUTFILE (Only if write succeeded)
                    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CFTPSPUTFILE=\"%s\",1", modulePath);
                    SerialSIT.println(gprs_xmit_buf);
                    if (waitForResponse("+CFTPSPUTFILE:", 60000)) {
                        debugf("[FTP] Retry CFTPSPUTFILE response: %s\n", modem_response_buf);
                        if (strstr(modem_response_buf, "+CFTPSPUTFILE: 0") != NULL) upload_success = true;
                    }
                    // v5.76.6: Final retry attempt concluded. Terminate loop regardless of result.
                    ftp_put_retries = 2; 
                  } else {
                    debugln("[FTP] [ERR] Retry FSWRITE failed. Aborting.");
                    rf.close();
                  }
                }
                xSemaphoreGive(fsMutex);
              }
              if (modem_ftp_handle > 0) {
                 snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSCLOSE=%d", modem_ftp_handle);
                 SerialSIT.println(gprs_xmit_buf); waitForResponse("OK", 3000);
                 modem_ftp_handle = 0;
              }
               // v5.75 Retry: Standard numeric ID 1 for C:/
               snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CFTPSPUTFILE=\"%s\",1", modulePath);
            } else {
              debugln("[FTP] PUT failed after all attempts. Giving up to save battery.");
              break; // Don't waste more time — sleep and retry next cycle
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

          // v13.4: After successful daily FTP, purge old data/daily txt files
          // Uses two-pass: collect first, delete after — safe SPIFFS iterator pattern (Bug 5 fix)
          if (isDailyFTP) {
            char toDelete[16][64];
            int delCount = 0;
            char today_str[16], yest_str[16];
            int y_day = 0, y_mon = 0, y_yr = 0;
            diag_rejected_count = 0; // Reset on success

            // v5.75: FTP Breather — Allow modem stack to clear HTTP context before FTP starts
            // Prevents the 'ERROR' response during rapid data-flow transitions.
            // Pass 1: collect candidates (Wrapped in Mutex to prevent rtcRead race)
            if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(10000)) == pdTRUE) {
            // Pass 1: collect candidates
            // Bug B fix: use ftp_station dynamically instead of hardcoded "1941_2"
            char stn_prefix[24];
            snprintf(stn_prefix, sizeof(stn_prefix), "%s_", ftp_station);
            File scanDir = SPIFFS.open("/");
            if (scanDir) {
              File sf = scanDir.openNextFile();
              while (sf && delCount < 14) {
                const char* sn = sf.name();
                int snLen = strlen(sn);
                
                bool match = false;
                if ((strstr(sn, stn_prefix) == sn || strstr(sn, "dailyftp_") == sn) && 
                    snLen >= 4 && strcmp(sn + snLen - 4, ".txt") == 0) {
                  match = true;
                }
                
                if (match) {
                   if (sn[0] == '/') strncpy(toDelete[delCount++], sn, 63);
                   else snprintf(toDelete[delCount++], 64, "/%s", sn);
                }
                sf.close();
                sf = scanDir.openNextFile();
              }
              scanDir.close();
            }
            // Bug A fix: proper days-per-month table — no more hardcoded y_day=31
            snprintf(today_str, sizeof(today_str), "%04d%02d%02d", rf_cls_yy, rf_cls_mm, rf_cls_dd);
            y_day = rf_cls_dd - 1; y_mon = rf_cls_mm; y_yr = rf_cls_yy;
            if (y_day < 1) {
              y_mon--;
              if (y_mon < 1) { y_mon = 12; y_yr--; }
              const uint8_t dim[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
              bool leap = (y_yr % 4 == 0 && (y_yr % 100 != 0 || y_yr % 400 == 0));
              y_day = (y_mon == 2 && leap) ? 29 : dim[y_mon];
            }
            snprintf(yest_str, sizeof(yest_str), "%04d%02d%02d", y_yr, y_mon, y_day);
            
            // Pass 2: delete files not matching today or yesterday
            for (int i = 0; i < delCount; i++) {
              if (strstr(toDelete[i], today_str) == NULL && strstr(toDelete[i], yest_str) == NULL) {
                SPIFFS.remove(toDelete[i]);
                debugf("[SPIFFS] Old file purged: %s\n", toDelete[i]);
              }
            }
            xSemaphoreGive(fsMutex);
    }
  }

          // Cleanup .swd/.kwd staging files (two-pass, iterator-safe)
          const char *pattern = strstr(UNIT, "SPATIKA") ? ".swd" : ".kwd";
          char kwdFiles[8][64]; int kwdCount = 0;
          if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
            File rootDir = SPIFFS.open("/");
            if (rootDir) {
              File f = rootDir.openNextFile();
              while (f && kwdCount < 8) {
                const char* n = f.name();
                int nLen = strlen(n);
                int pLen = strlen(pattern);
                if (nLen >= pLen && strcmp(n + nLen - pLen, pattern) == 0) {
                   if (n[0] == '/') strncpy(kwdFiles[kwdCount++], n, 63);
                   else snprintf(kwdFiles[kwdCount++], 64, "/%s", n);
                }
                f.close();
                f = rootDir.openNextFile();
              }
              rootDir.close();
            }
            for (int i = 0; i < kwdCount; i++) SPIFFS.remove(kwdFiles[i]);
            xSemaphoreGive(fsMutex);
          }

          snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSDEL=\"C:/%s\"", modulePath);
          SerialSIT.println(gprs_xmit_buf);
          waitForResponse("OK", 5000);
        } else {
          strcpy(last_cmd_res, "Fail: FTP PUT Error");
          if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
            if (SPIFFS.exists("/ftpremain.txt")) SPIFFS.remove("/ftpremain.txt");
            xSemaphoreGive(fsMutex);
          }
        }
    }
  }

ftp_cleanup:
  if (ftp_started) {
    // Turner-Fix: Sequence M-01 — Handle closure MUST precede logout/stop
    if (modem_ftp_handle > 0) {
        snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSCLOSE=%d", modem_ftp_handle);
        SerialSIT.println(gprs_xmit_buf); waitForResponse("OK", 2000);
        modem_ftp_handle = 0;
    }

    // v5.70: Standard Stack Teardown Sequence (Logout -> Stop)
    SerialSIT.println("AT+CFTPSLOGOUT");
    waitForResponse("+CFTPSLOGOUT: 0", 3000);
    
    // v5.70: Fix C-03 FTP Put Race (Cleanup Path)
    if (strstr(put_buf, "+CFTPSPUTFILE:") == NULL) {
        debugln("[FTP] Race Guard: Logout too fast (Cleanup)? Waiting 1s drain.");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    SerialSIT.println("AT+CFTPSSTOP");
    waitForResponse("OK", 3000);
  }
  if (weLockedFS) xSemaphoreGive(fsMutex);
  xSemaphoreGive(modemMutex);

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
      Serial.println("[OTA] [ERR] UART Cross-talk (0x75684008) DETECTED! Poisoned "
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
      Serial.printf("[OTA] [ERR] Invalid Magic Byte: 0x%02X (Expected 0xE9)\n",
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
    Serial.printf("[OTA] [ERR] Suspicious data (99%% repeating 0x%02X). Aborting "
                  "chunk.\n",
                  buf[0]);
    return false;
  }

  return true;
}

int setup_ftp(int transMode) { // 0=Active(BSNL 2G), 1=Passive(Airtel 4G)
  flushSerialSIT(); // Clear leftover data from previous task
  char gprs_xmit_buf[300];
  const char *resp;
  const char *ftpServer;
  const char *ftpUser;
  const char *ftpPassword;
  int portName, result;

  if (strstr(UNIT, "BIHAR") || strstr(NETWORK, "BIHAR")) {
    if (send_daily == 1) {
      ftpServer = "ftphbih.spatika.net"; ftpUser = "trg_desbih_csvdt"; ftpPassword = FTP_PASS_BIH_D; portName = 21; send_daily = 0;
    } else {
      ftpServer = "89.32.144.163"; ftpUser = "dota_bih"; ftpPassword = FTP_PASS_BIH_GEN; portName = 21;
    }
  } else if (strstr(UNIT, "KSNDMC") || strstr(UNIT, "DMC") || strstr(NETWORK, "KSNDMC")) {
    if (send_daily == 1) {
      ftpServer = "ftp1.ksndmc.net"; ftpUser = "trg_spatika_v2@ksndmc.net"; ftpPassword = FTP_PASS_KS_TRG; portName = 21; send_daily = 0;
    } else if (send_daily == 2) {
#if SYSTEM == 1
      ftpServer = "ftp1.ksndmc.net"; ftpUser = "tws_spatika_v2"; ftpPassword = FTP_PASS_KS_TWS; portName = 21;
#elif SYSTEM == 2
      ftpServer = "ftp1.ksndmc.net"; ftpUser = "twsrf_spatika_v2"; ftpPassword = FTP_PASS_KS_ADD; portName = 21;
#endif
      // send_daily = 0; // Removed from here to prevent credential-loss on retry
    } else {
      // v5.38 Golden Path: Standard non-daily Karnataka data always uses Dota DMC
      ftpServer = "89.32.144.163"; ftpUser = "dota_dmc"; ftpPassword = "airdata2016"; portName = 21;
    }
  } else if (strstr(UNIT, "SPATIKA") || strstr(NETWORK, "SPATIKA")) {
#if SYSTEM == 0
    ftpServer = "ftp.spatika.net"; ftpUser = "trg_gen"; ftpPassword = "spgen123"; portName = 21;
#endif
#if SYSTEM == 1
    ftpServer = "ftp.spatika.net"; ftpUser = "tws_gen"; ftpPassword = "spgen123"; portName = 21;
#endif
#if SYSTEM == 2
    ftpServer = "ftp.spatika.net"; ftpUser = "twsrf_gen"; ftpPassword = "spgen123"; portName = 21;
#endif
  }

  send_daily = 0; // Ensure flag is reset after use
  result = -1;

  // v13.10: Restore-to-Golden (Simplified Setup)
  SerialSIT.println("AT+CFTPSSTOP");
  waitForResponse("OK", 5000);

  SerialSIT.println("AT+CFTPSSTART");
  waitForResponse("+CFTPSSTART: 0", 20000); // v7.70: Increased to 20s for BSNL stability (H-03)

  // v5.38 Harmonization: Explicitly Configure FTP Client Context
  // Ensure plain FTP (No SSL/TLS) and bind strictly to GPRS Context 1
  // v5.75 FIX: These MUST be sent AFTER Start response for A7672S stack stability (M-01)
  SerialSIT.println("AT+CFTPSCFG=\"security\",0");
  waitForResponse("OK", 2000);
  SerialSIT.println("AT+CFTPSCFG=\"bindcid\",1");
  waitForResponse("OK", 2000);

  SerialSIT.println("AT+CFTPSSINGLEIP=1"); // Data channel binding — confirmed working in v13.9
  
  // v5.75 BSNL Active Mode Hardening (Moved to post-start)
  debugf("[FTP] Configuring mode (%d)...\n", transMode);
  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CFTPSCFG=\"TRANSMODE\",%d", transMode);
  SerialSIT.println(gprs_xmit_buf);
  waitForResponse("OK", 3000);
  SerialSIT.println("AT+CFTPSCFG=\"type\",I");
  waitForResponse("OK", 3000);
  SerialSIT.println("AT+CMEE=1");
  debugln(waitForResponse("OK", 2000));


  // v5.76.6: DNS Resilience - Fresh retry per call. 
  char targetAddress[64];
  strcpy(targetAddress, ftpServer); // Default to the domain string

  debugln("[FTP] Warming up DNS...");
  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CDNSGIP=\"%s\"", ftpServer);
  SerialSIT.println(gprs_xmit_buf);
  // v5.76.3: Reduced timeout to 12s for faster transition
  if (waitForResponse("+CDNSGIP:", 12000)) {
      const char* fq = strstr(modem_response_buf, "\",\"");
      if (fq != NULL) {
          const char* sq = strchr(fq + 3, '\"');
          if (sq != NULL) {
              int rLen = sq - (fq + 3);
              if (rLen > 6 && rLen < (int)sizeof(targetAddress)) {
                  strncpy(targetAddress, fq + 3, rLen);
                  targetAddress[rLen] = '\0';
                  debugf("[FTP] DNS Resolved: %s\n", targetAddress);
              }
          }
      }
  } else {
      debugln("[FTP] DNS Failed. Switching to Insurance IP.");
      if (strstr(ftpServer, "spatika.net")) strcpy(targetAddress, "89.32.144.163");
      else if (strstr(ftpServer, "ksndmc.net")) strcpy(targetAddress, "27.34.245.70");
  }

  // Pre-login settling delay
  vTaskDelay(1000 / portTICK_PERIOD_MS); 

  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
           "AT+CFTPSLOGIN=\"%s\",%d,\"%s\",\"%s\",0",
           targetAddress, portName, ftpUser, ftpPassword); // v7.70: Use resolved IP to skip BSNL DNS (H-02)

  SerialSIT.println(gprs_xmit_buf);
  if (waitForResponse("+CFTPSLOGIN:", 60000)) {
     const char* login_ptr = strstr(modem_response_buf, "+CFTPSLOGIN:");
     if (login_ptr != NULL) {
        result = atoi(login_ptr + 12);
        debugf("[FTP] Login Result: %d\n", result);
        if (result == 0) {
           debugln("FTP Login success");
           return 1;
        }
     }
  }
  debugln("FTP Login unsuccessful");
  return 0;
}

void get_gps_coordinates() {
}

void fetchFromHttpAndUpdate(char *fileName) {
  if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(30000)) != pdTRUE) {
    debugln("[OTA] Error: Modem Mutex Timeout - deferring download");
    diag_modem_mutex_fails++;
    return;
  }
  ota_silent_mode = true; // Rule 50: Silence sensor tasks before modem reconfiguration
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
  if (!waitForResponse("OK", 5000)) {
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
  if (!waitForResponse("OK", 2000)) {
    debugln(
        "[OTA] CID binding warning: No OK received, but continuing anyway.");
  }
  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
           "AT+HTTPPARA=\"URL\",\"http://%s:%s/builds/%s\"", HEALTH_SERVER_IP,
           OTA_SERVER_PORT, fileName);
  SerialSIT.println(gprs_xmit_buf);
  if (!waitForResponse("OK", 2000)) {
    debugln("[OTA] URL setup failed. Aborting.");
    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 2000);
    xSemaphoreGive(modemMutex);
    return;
  }
  flushSerialSIT();  // HTTP HEAD to determine Content-Length
  SerialSIT.println("AT+HTTPACTION=2");
  if (!waitForResponse("+HTTPACTION:", 25000)) {
    debugln("[OTA] No response to HTTPACTION=2 (HEAD). Aborting.");
    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 2000);
    xSemaphoreGive(modemMutex);
    return;
  }
  
  if (strstr(modem_response_buf, "200") == NULL) {
    debugf("[OTA] HEAD failed. Status: %s\n", modem_response_buf);
    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 2000);
    xSemaphoreGive(modemMutex);
    return;
  }

  SerialSIT.println("AT+HTTPHEAD");
  if (waitForResponse("OK", 5000)) {
     const char* head_ptr = strstr(modem_response_buf, "Content-Length:");
     if (head_ptr != NULL) {
        total_no_of_bytes = atoi(head_ptr + 15);
        debugf("[OTA] Server reports file size: %d\n", total_no_of_bytes);
     }
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
    
    char err_buf[64];
    snprintf(err_buf, sizeof(err_buf), "Begin fail: %s", Update.errorString());
    p.putString("fail_res", err_buf);
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
      Serial.println("[OTA] [CRIT] 3 Consecutive chunk failures! Hard Hardware "
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
      Serial.println("[OTA] [ERR] Bearer Recovery Failed");
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
    if (!waitForResponse("OK", 5000)) {
      Serial.println("[OTA] [WARN] HTTPINIT Failed");
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
    if (!waitForResponse("+HTTPACTION:", 95000)) {
       Serial.println("[OTA] [ERR] HTTPACTION Timeout");
       chunk_retries++;
       consecutive_fails++;
       continue;
    }
    Serial.println(modem_response_buf); // Show the response status code

    int http_status = 0;
    int chunk_total = 0;
    const char *first_c = strchr(modem_response_buf, ',');
    const char *last_c = strrchr(modem_response_buf, ',');
    if (first_c != NULL && last_c != NULL && last_c > first_c) {
      http_status = atoi(first_c + 1);
      chunk_total = atoi(last_c + 1);
    }

    if (http_status != 200 && http_status != 206) {
      Serial.printf("[OTA] [ERR] HTTP Error: %d. Resetting session.\n",
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

    char read_hdr[64] = {0};
    int rh_idx = 0;
    unsigned long t_start = millis();
    bool hdr_found = false;
    while (millis() - t_start < 12000) {
      esp_task_wdt_reset(); // M-NEW-2: Pet WDT during long header search
      if (SerialSIT.available()) {
        char c = SerialSIT.read();
        if (rh_idx < (int)sizeof(read_hdr) - 1) {
           read_hdr[rh_idx++] = c;
           read_hdr[rh_idx] = '\0';
        }

        if (c == '\n' && strstr(read_hdr, "+HTTPREAD:") != NULL) {
          hdr_found = true;
          break;
        }
        if (strstr(read_hdr, "ERROR") != NULL)
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
      Serial.printf("[OTA] [ERR] Read Timeout/Incomplete: got %d of %d\n", got,
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
      int errCode = Update.getError();
      debugf("[OTA] Update.end FAILED. Code: %d, Str: %s\n", errCode, Update.errorString());
      Update.printError(Serial); // BUG-10 Fix: Print to console, not modem UART

      Preferences p;
      p.begin("ota-track", false);
      p.putInt("fail_cnt", p.getInt("fail_cnt", 0) + 1);
      char res_buf[32];
      snprintf(res_buf, sizeof(res_buf), "End fail code: %d", errCode);
      p.putString("fail_res", res_buf);
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
  SerialSIT.println("AT+HTTPINIT");
  if (!waitForResponse("OK", 2000)) {
    SerialSIT.println("AT+CREG=1");
    waitForResponse("OK", 1000);
    SerialSIT.println("AT+CEREG=1");
    waitForResponse("OK", 1000);
  }

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
  
  char SPIFFSFile[50], fileName[100];
  char stnId[32];
  
  if (strlen(station_name) == 4 && isDigitStr(station_name)) {
    snprintf(stnId, sizeof(stnId), "00%s", station_name);
  } else {
    strcpy(stnId, station_name);
  }

  const char *prefix = "UN_";
#if SYSTEM == 0
  prefix = "TRG_";
#elif SYSTEM == 1
  prefix = "TWS_";
#elif SYSTEM == 2
  prefix = "TWSRF_";
#endif

  const char *extension = ".txt";
  if (strstr(UNIT, "SPATIKA")) {
    extension = ".swd";
  } else if (strstr(UNIT, "KSNDMC") || strstr(UNIT, "DMC")) {
    extension = ".kwd";
  }

  snprintf(SPIFFSFile, sizeof(SPIFFSFile), "/%s_%s.txt", station_name,
           dateFile);
  snprintf(fileName, sizeof(fileName), "%s%s_%s%s", prefix, stnId, dateFile,
           extension);
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
    waitForResponse("OK", 3000);
    SerialSIT.println("AT+FSCD=C:/");
    if (!waitForResponse("OK", 10000)) {
      debugln("Error changing directory to C:/");
      xSemaphoreGive(modemMutex);
      xSemaphoreGive(fsMutex);
      return; // Exit if response is invalid
    }
    debug("Response of AT+FSCD ");
    debugln(modem_response_buf);

    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSOPEN=C:/%s,0\r\n",
             fileName); // 0 : if the file does not exist, it will be created
    SerialSIT.println(gprs_xmit_buf);
    handle_no = 0;
    if (waitForResponse("+FSOPEN", 5000)) {
       const char* fso = strstr(modem_response_buf, "+FSOPEN:");
       if (fso != NULL) {
          sscanf(fso, "+FSOPEN: %d,", &handle_no);
       }
    }
    
    if (handle_no == 0) {
      debugln("Error: +FSOPEN failed in copyFromSPIFFSToFS");
      xSemaphoreGive(modemMutex);
      xSemaphoreGive(fsMutex);
      return; // Exit if response is invalid
    }

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
    if (waitForResponse("CONNECT", 5000)) {
      flushSerialSIT(); // [FTP-02] Fix: Drain echo residue
    }
    
    // Chunk flow directly to modem UART to bypass 100KB RAM fragmentation
    char file_buf[256];
    int bytes_streamed = 0;
    while (file1.available()) {
      int bytesRead = file1.read((uint8_t*)file_buf, 255);
      SerialSIT.write((uint8_t*)file_buf, bytesRead);
      bytes_streamed += bytesRead;
      if (bytes_streamed % (16 * 1024) == 0) esp_task_wdt_reset(); // Phase 6 Review: Prevent Watchdog Trips
    }
    // [FTP-05] Fix: Remove SerialSIT.println() after stream
    waitForResponse("+FSWRITE", 5000);
    file1.close();

    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSCLOSE=%d",
             handle_no); 
    SerialSIT.println(gprs_xmit_buf);
    waitForResponse("OK", 5000); // Fixed: FSCLOSE responds with OK, not URC
    vTaskDelay(200 / portTICK_PERIOD_MS);

    // v5.65 Fix: Pass default mode 1 (Passive) for compatibility
    ftp_login_flag = setup_ftp(1);

    if (ftp_login_flag == 1) {
      char *modulePath = (fileName[0] == '/') ? &fileName[1] : fileName;
      snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CFTPSPUTFILE=\"%s\",1",
               modulePath); 
      SerialSIT.println(gprs_xmit_buf); // FTP client context
      // v5.65 WDT-SAFE: Active poll instead of blind 150s wait (prevents WDT reset on slow BSNL)
      char put_resp[256] = {0}; // v5.70: for Race Guard
      int pr_idx = 0;
      {
        unsigned long put_start = millis();
        while ((millis() - put_start) < 150000) {
          esp_task_wdt_reset();
          vTaskDelay(500 / portTICK_PERIOD_MS);
          while (SerialSIT.available() && pr_idx < (int)sizeof(put_resp) - 1) {
            put_resp[pr_idx++] = SerialSIT.read();
            put_resp[pr_idx] = '\0';
          }
          if (strstr(put_resp, "+CFTPSPUTFILE: 0") != NULL) break;
        }
      }
      debugf("Response of AT+CFTPSPUTFILE: %s\n", put_resp);
      
      // v7.70+: Only continue if success code '0' was received
      if (strstr(put_resp, "+CFTPSPUTFILE: 0") != NULL) {
         // Proceed with logout/cleanup logic as before
      } else {
         debugln("[FTP] PUT failed. Skipping cleanup.");
         // v5.70: Fix C-03 FTP Put Race (OTA/copyFromSPIFFSToFS path)
         if (strstr(put_resp, "+CFTPSPUTFILE:") == NULL) {
             debugln("[FTP] Race Guard (OTA): Waiting 1s drain before logout.");
             vTaskDelay(1000 / portTICK_PERIOD_MS);
         }
      }

      SerialSIT.println("AT+CFTPSLOGOUT");
      waitForResponse("+CFTPSLOGOUT: 0", 9000);
      vTaskDelay(200 / portTICK_PERIOD_MS);

      SerialSIT.println("AT+CFTPSSTOP");
      waitForResponse("OK", 5000);
      vTaskDelay(200 / portTICK_PERIOD_MS);

      snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSDEL=\"C:/%s\"",
               fileName); // 0 : if the file does not exist, it will be created
      SerialSIT.println(gprs_xmit_buf);
      waitForResponse("OK", 10000);
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
      debugf("[FTP] Skip Daily: Signal too weak (%d dBm).\n", signal_lvl);
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

