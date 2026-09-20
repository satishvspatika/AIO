#include "esp_ota_ops.h"
#include "globals.h"

int last_ftp_login_result = -1; // Added for HTTP Fallback tracking

void send_ftp_file(char *fileName, bool isDailyFTP, bool alreadyLocked) {
  set_sys_status("FTP UPLOAD");
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
                   if (modem_ftp_handle > 0) {
                       if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
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
                   }
                   snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+FSCLOSE=%d", modem_ftp_handle);
                   SerialSIT.println(gprs_xmit_buf); waitForResponse("OK", 3000);
                   modem_ftp_handle = 0; // Clear after success
                 } else { debugln("[FTP] CID9 restart: FSOPEN failed. Giving up."); goto ftp_cleanup; }
              } else { debugln("[FTP] CID9 restart: FSOPEN NO-PROMPT. Giving up."); goto ftp_cleanup; }
              snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CFTPSPUTFILE=\"%s\",1", modulePath);
            } else { debugln("[FTP] CID9 restart: Re-login failed. Giving up."); goto ftp_cleanup; }
          }
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
          // v6.09: Recount backlog after successful upload to prevent monotonic growth desync
          get_total_backlogs(true);

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
#if SYSTEM == 3
    ftpServer = "ftp.spatika.net"; ftpUser = "twsrp"; ftpPassword = FTP_PASS_TWSRP; portName = 21;
#endif
  }

  send_daily = 0; // Ensure flag is reset after use
  result = -1;

  // v13.10: Restore-to-Golden (Simplified Setup)
  SerialSIT.println("AT+CFTPSSTOP");
  waitForResponse("OK", 5000);

  // v6.29 FIX: CFTPSCFG configuration commands MUST be sent BEFORE CFTPSSTART on A7672S.
  // Sending them after CFTPSSTART causes the modem to return ERROR and fail to apply transmode=1 (Passive Mode).
  SerialSIT.println("AT+CFTPSCFG=\"security\",0");
  waitForResponse("OK", 2000);
  SerialSIT.println("AT+CFTPSCFG=\"bindcid\",1");
  waitForResponse("OK", 2000);
  debugf("[FTP] Configuring mode (%d)...\n", transMode);
  snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CFTPSCFG=\"transmode\",%d", transMode);
  SerialSIT.println(gprs_xmit_buf);
  waitForResponse("OK", 3000);
  SerialSIT.println("AT+CFTPSCFG=\"type\",I");
  waitForResponse("OK", 3000);

  SerialSIT.println("AT+CFTPSSTART");
  bool ftp_stack_ok = waitForResponse("+CFTPSSTART: 0", 20000); // v7.70: Increased to 20s for BSNL stability (H-03)

  // v6.29-FIX: CFTPSSTART error guard — if FTP stack fails to start (common after HTTP
  // context teardown), recycle the PDP bearer and retry ONCE before giving up.
  // Without this, all subsequent config commands + CFTPSLOGIN silently fail with error 9.
  if (!ftp_stack_ok) {
    debugln("[FTP] CFTPSSTART failed. Recycling bearer and retrying...");
    SerialSIT.println("AT+CGACT=0,1");
    waitForResponse("OK", 3000);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    SerialSIT.println("AT+CGACT=1,1");
    waitForResponse("OK", 10000);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    flushSerialSIT();
    // Re-apply CFTPSCFG parameters before retrying CFTPSSTART
    SerialSIT.println("AT+CFTPSCFG=\"security\",0");
    waitForResponse("OK", 2000);
    SerialSIT.println("AT+CFTPSCFG=\"bindcid\",1");
    waitForResponse("OK", 2000);
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CFTPSCFG=\"transmode\",%d", transMode);
    SerialSIT.println(gprs_xmit_buf);
    waitForResponse("OK", 3000);
    SerialSIT.println("AT+CFTPSSTART");
    ftp_stack_ok = waitForResponse("+CFTPSSTART: 0", 20000);
    if (!ftp_stack_ok) {
      debugln("[FTP] CFTPSSTART failed after bearer recycle. Aborting FTP setup.");
      return 0;
    }
    debugln("[FTP] CFTPSSTART OK after bearer recycle.");
  } else {
    debugln("[FTP] CFTPSSTART OK.");
  }

  SerialSIT.println("AT+CFTPSSINGLEIP=1"); // Data channel binding — confirmed working in v13.9
  SerialSIT.println("AT+CMEE=1");
  debugln(waitForResponse("OK", 2000));


  // v5.76.6: DNS Resilience - Fresh retry per call. 
  char targetAddress[64];
  strncpy(targetAddress, ftpServer, sizeof(targetAddress)-1);
  targetAddress[sizeof(targetAddress)-1] = '\0';

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
      // v6.29-FIX: Corrected insurance IP for ftp.spatika.net.
      // 89.32.144.163 is the Dota/intovps server (WRONG). Real IP is 144.91.104.105.
      if (strstr(ftpServer, "ftp.spatika.net")) {
        strncpy(targetAddress, "144.91.104.105", sizeof(targetAddress)-1);
      } else if (strstr(ftpServer, "spatika.net")) {
        strncpy(targetAddress, "89.32.144.163", sizeof(targetAddress)-1); // Dota/Bihar server
      } else if (strstr(ftpServer, "ksndmc.net")) {
        strncpy(targetAddress, "27.34.245.70", sizeof(targetAddress)-1);
      }
      targetAddress[sizeof(targetAddress)-1] = '\0';
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
        last_ftp_login_result = result; // Capture for HTTP Fallback
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

// Bounded memory search helper that does not stop at embedded null bytes (0x00)
static const char *memfind(const char *buf, int bufLen, const char *target) {
  if (!buf || !target || bufLen <= 0) return NULL;
  int targetLen = strlen(target);
  if (targetLen == 0 || targetLen > bufLen) return NULL;
  for (int i = 0; i <= bufLen - targetLen; i++) {
    if (memcmp(buf + i, target, targetLen) == 0) {
      return buf + i;
    }
  }
  return NULL;
}

static void drainUartUntilQuiet(uint32_t quiet_ms) {
  flushSerialSIT();
  unsigned long start = millis();
  while (millis() - start < quiet_ms) {
    while (SerialSIT.available()) {
      SerialSIT.read();
      start = millis();
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

static int readIpdFrame(uint8_t *dest, int maxDest, uint32_t timeout_ms) {
  unsigned long start = millis();
  int state = 0; // 0=seeking '+', 1='I', 2='P', 3='D'
  int lenAcc = 0;
  bool lenValid = false;

  while (millis() - start < timeout_ms) {
    esp_task_wdt_reset();
    if (!SerialSIT.available()) {
      vTaskDelay(1);
      continue;
    }
    uint8_t c = SerialSIT.read();

    if (state == 0) {
      if (c == '+') state = 1;
    } else if (state == 1) {
      if (c == 'I') state = 2;
      else state = (c == '+') ? 1 : 0;
    } else if (state == 2) {
      if (c == 'P') state = 3;
      else state = (c == '+') ? 1 : 0;
    } else if (state == 3) {
      if (c == 'D') {
        state = 4;
        lenAcc = 0;
        lenValid = false;
      } else if (c == 'C') {
        state = 10;
      } else state = (c == '+') ? 1 : 0;
    } else if (state == 10) {
      if (c == 'L') state = 11;
      else state = (c == '+') ? 1 : 0;
    } else if (state == 11) {
      if (c == 'O') state = 12;
      else state = (c == '+') ? 1 : 0;
    } else if (state == 12) {
      if (c == 'S') state = 13;
      else state = (c == '+') ? 1 : 0;
    } else if (state == 13) {
      if (c == 'E') state = 14;
      else state = (c == '+') ? 1 : 0;
    } else if (state == 14) {
      if (c == ':' || c == ' ' || c == '\n') {
        state = 0; // Ignore stale +IPCLOSE URC from previous socket
      } else {
        state = (c == '+') ? 1 : 0;
      }
    } else if (state == 4) {
      if (c >= '0' && c <= '9') {
        lenAcc = lenAcc * 10 + (c - '0');
        lenValid = true;
      } else if (c == ',') {
        // Comma separates socket ID from payload length (e.g. +IPD,1,1460:)
        lenAcc = 0;
        lenValid = false;
      } else if (lenValid && lenAcc > 0 && (c == ':' || c == '\r' || c == '\n')) {
        // Colon ':', '\r', or '\n' terminates length integer — header parsing COMPLETE!
        int payloadLen = lenAcc;
        int readTotal = 0;

        // Consume trailing '\n' if delimiter was '\r'
        if (c == '\r') {
          unsigned long tw = millis();
          while (!SerialSIT.available() && (millis() - tw < 100)) vTaskDelay(1);
          if (SerialSIT.peek() == '\n') SerialSIT.read();
        }

        unsigned long readStart = millis();
        while (readTotal < payloadLen && (millis() - readStart < 10000)) {
          esp_task_wdt_reset();
          if (SerialSIT.available()) {
            int avail = SerialSIT.available();
            int take = min(avail, payloadLen - readTotal);
            if (readTotal + take > maxDest) {
              take = maxDest - readTotal;
            }
            if (take <= 0) break;
            int n = SerialSIT.readBytes((char *)(dest + readTotal), take);
            if (n > 0) {
              readTotal += n;
            }
            readStart = millis();
          } else {
            vTaskDelay(1);
          }
        }

        state = 0;
        lenAcc = 0;
        lenValid = false;
        if (readTotal == payloadLen) {
          return readTotal;
        } else {
          Serial.printf("[IPD-ERR] Incomplete +IPD frame: read %d / %d bytes. Discarding & purging UART.\n", readTotal, payloadLen);
          drainUartUntilQuiet(300);
          return -1;
        }
      } else {
        state = 0;
        lenAcc = 0;
        lenValid = false;
      }
    }
  }
  return 0;
}


// Mirrors the AT+NETOPEN?/AT+NETOPEN check-then-open pattern used in
// gprs_health.ino and gprs_http.ino. Must be called after any AT+NETCLOSE
// to rebuild the IP application layer before AT+CIPOPEN can succeed.
// AT+CGACT (PDP bearer) and AT+NETOPEN (IP stack) are separate layers —
// verify_bearer_or_recover() only covers CGACT, not NETOPEN.
// Rebuilds the A7672S IP application layer after AT+NETCLOSE.
// KEY: AT+NETOPEN sends "OK" immediately (command queued), then sends
// "+NETOPEN: 0" asynchronously when the stack is actually ready.
// We use waitForResponseNoFlush() to catch the URC WITHOUT discarding it
// first (waitForResponse always calls flushSerialSIT() at entry, which
// would wipe the pending URC before we could read it).
static void ensureNetOpen() {
  flushSerialSIT();
  SerialSIT.println("AT+NETOPEN?");
  waitForResponse("OK", 2000);
  if (strstr(modem_response_buf, "+NETOPEN: 1") != NULL) {
    Serial.println("[OTA-TCP] AT+NETOPEN: IP stack already open.");
    return; // Already up, nothing to do
  }
  Serial.println("[OTA-TCP] IP stack closed — calling AT+NETOPEN...");
  flushSerialSIT();
  SerialSIT.println("AT+NETOPEN");
  // Step 1: Wait for the immediate OK acknowledgment (command queued)
  waitForResponseNoFlush("OK", 3000);
  // Step 2: Wait for +NETOPEN: async URC (stack actually ready)
  // Do NOT use waitForResponse here — its flushSerialSIT() would discard
  // the +NETOPEN: 0 completion URC before we can read it.
  bool netopen_ok = waitForResponseNoFlush("+NETOPEN:", 8000);
  if (!netopen_ok) {
    Serial.println("[OTA-TCP] NETOPEN URC timeout — stack may not be ready.");
  } else {
    Serial.printf("[OTA-TCP] NETOPEN result: %s\n",
                  strstr(modem_response_buf, "+NETOPEN: 0") ? "OK" : "ERROR/already-open");
  }
  vTaskDelay(1000 / portTICK_PERIOD_MS); // Final settle
}

static bool writePayloadToFlash(uint8_t *payload, int len, int total_no_of_bytes, int &actual_downloaded) {
  if (!payload || len <= 0) return true;

  int bytesToWrite = min(len, total_no_of_bytes - actual_downloaded);
  if (bytesToWrite <= 0) return true;

  size_t w = Update.write(payload, bytesToWrite);
  if (w != (size_t)bytesToWrite) {
    Serial.printf("[OTA-TCP] Flash Write Error: wrote %u / %d bytes. Error: %s\n", (unsigned int)w, bytesToWrite, Update.errorString());
    return false;
  }
  actual_downloaded += w;
  return true;
}

void fetchFromTcpAndUpdate(char *fileName, bool alreadyLocked) {
  if (!alreadyLocked) {
    if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(30000)) != pdTRUE) {
      Serial.println("[OTA-TCP] Error: Modem Mutex Timeout - deferring download");
      diag_modem_mutex_fails++;
      return;
    }
  }

  ota_silent_mode = true; // Silence background tasks/logs before binary streaming
  char gprs_xmit_buf[350];
  int total_no_of_bytes = 0;
  int actual_downloaded = 0;

  Serial.printf("[OTA-TCP] Starting Direct TCP Socket OTA for: %s\n", fileName);

  // 1. URC Protocol Configuration
  SerialSIT.setRxBufferSize(16384); // Expand UART RX FIFO buffer to 16KB to prevent hardware RX overflow during flash writes
  flushSerialSIT();
  SerialSIT.println("ATE0"); waitForResponse("OK", 1000); // Disable echo to prevent request echo matching

  // Ensure PDP Bearer is active
  if (!verify_bearer_or_recover()) {
    Serial.println("[OTA-TCP] Bearer Recovery Failed. Aborting OTA.");
    if (!alreadyLocked) xSemaphoreGive(modemMutex);
    ota_silent_mode = false;
    Preferences p;
    p.begin("ota-track", false);
    p.putInt("fail_cnt", p.getInt("fail_cnt", 0) + 1);
    p.putString("fail_res", "Fail: Bearer Down");
    p.end();
    return;
  }
  // Ensure the IP application layer (NETOPEN) is up — separate from PDP bearer.
  // OTA must not rely on health check having left NETOPEN open.
  ensureNetOpen();

  uint8_t *chunkBuf = (uint8_t *)malloc(8192);
  if (!chunkBuf) {
    Serial.println("[OTA-TCP] malloc failed!");
    if (!alreadyLocked) xSemaphoreGive(modemMutex);
    ota_silent_mode = false;
    return;
  }

  bool magicChecked = false;
  bool updateInitialized = false;
  IpdFilterCtx ipdCtx; // Stateful IPD filter — persists across readBytes() calls within an attempt
  int maxRetries = 200; // High retry limit to guarantee 1.4MB file completion across GPRS drops (44 chunks * retries)
  int cipopen_consecutive_fails = 0; // Track consecutive CIPOPEN failures for escalation
  // Direct IP connections avoid cellular DNS resolution timeouts on airteliot SIMs
  const char *ota_hosts[2] = { HEALTH_SERVER_IP, HEALTH_SERVER_IP };
  int hostIdx = 0; // Start with IP; rotate to domain on first CIPOPEN fail
  const char *connHost = ota_hosts[hostIdx];

  // Enforce clean IP stack reset before OTA to force fresh local ephemeral port
  vTaskDelay(3000 / portTICK_PERIOD_MS);
  SerialSIT.println("AT+CIPCLOSE=1,1"); waitForResponse("OK", 1000);
  SerialSIT.println("AT+CIPCLOSE=1"); waitForResponse("OK", 1000);
  SerialSIT.println("AT+NETCLOSE"); waitForResponse("OK", 3000);
  vTaskDelay(3000 / portTICK_PERIOD_MS);
  ensureNetOpen();
  vTaskDelay(3000 / portTICK_PERIOD_MS);
  const int CHUNK_SIZE = 32768; // 32 KB per chunk halves socket connection overhead and RF battery drain

  for (int attempt = 0; attempt < maxRetries && (total_no_of_bytes == 0 || actual_downloaded < total_no_of_bytes); attempt++) {
    esp_task_wdt_reset();
    ipdCtx.reset();

    if (actual_downloaded == 0 && updateInitialized) {
      Update.abort();
      updateInitialized = false;
      Serial.println("[OTA-TCP] Aborted stale flash update session for clean retry.");
    }

    int startByte = actual_downloaded;
    int endByte = (total_no_of_bytes > 0) ? min(startByte + CHUNK_SIZE - 1, total_no_of_bytes - 1) : (startByte + CHUNK_SIZE - 1);
    int expectedChunkLen = endByte - startByte + 1;

    // Close previous socket cleanly before opening fresh socket for next chunk
    SerialSIT.println("AT+CIPCLOSE=1,1"); waitForResponse("OK", 300);
    SerialSIT.println("AT+CIPCLOSE=1"); waitForResponse("OK", 300);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    // 2. Open Direct TCP Socket 1 for 32KB Chunk Request
    flushSerialSIT();
    SerialSIT.println("AT+CSOCKSETPN=1,1");
    waitForResponse("OK", 1000);
    Serial.printf("[OTA-TCP] Opening TCP socket 1 to %s:%s...\n", connHost, OTA_SERVER_PORT);
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CIPOPEN=1,\"TCP\",\"%s\",%s", connHost, OTA_SERVER_PORT);
    SerialSIT.println(gprs_xmit_buf);

    if (!waitForResponse("+CIPOPEN: 1,0", 15000)) {
      Serial.printf("[OTA-TCP] CIPOPEN Failed (attempt %d, host: %s). Resp: '%s'\n",
                    attempt + 1, connHost, modem_response_buf);
      SerialSIT.println("AT+CIPCLOSE=1,1"); waitForResponse("OK", 1000);
      SerialSIT.println("AT+CIPCLOSE=1"); waitForResponse("OK", 1000);
      cipopen_consecutive_fails++;
      if (cipopen_consecutive_fails >= 2) {
        Serial.println("[OTA-TCP] Consecutive CIPOPEN failures — NETCLOSE + PDP cycle + NETOPEN.");
        SerialSIT.println("AT+NETCLOSE"); waitForResponse("OK", 3000);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        SerialSIT.println("AT+CGACT=0,1"); waitForResponse("OK", 5000);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        SerialSIT.println("AT+CGACT=1,1"); waitForResponse("OK", 10000);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        ensureNetOpen();
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        cipopen_consecutive_fails = 0;
        hostIdx = (hostIdx + 1) % 2;
        connHost = ota_hosts[hostIdx];
      }
      continue;
    }
    cipopen_consecutive_fails = 0;
    SerialSIT.println("ATE0"); waitForResponse("OK", 1000);

    // 3. Formulate 32KB Range GET Request
    char httpRequest[384];
    snprintf(httpRequest, sizeof(httpRequest),
             "GET /builds/%s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Accept: */*\r\n"
             "Range: bytes=%d-%d\r\n"
             "Connection: close\r\n\r\n", fileName, HEALTH_SERVER_DOMAIN, startByte, endByte);

    int reqLen = strlen(httpRequest);
    Serial.printf("[OTA-TCP] Chunk Range GET bytes=%d-%d (%d bytes expected, total downloaded: %d / %d):\n%s",
                  startByte, endByte, expectedChunkLen, actual_downloaded, total_no_of_bytes, httpRequest);
    drainUartUntilQuiet(200);
    modem_response_buf[0] = '\0';
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+CIPSEND=1,%d", reqLen);
    SerialSIT.println(gprs_xmit_buf);

    if (!waitForResponse(">", 5000)) {
      Serial.printf("[OTA-TCP] CIPSEND prompt (>) failed (attempt %d). Resp: '%s'\n", attempt + 1, modem_response_buf);
      SerialSIT.println("AT+CIPCLOSE=1"); waitForResponse("OK", 1000);
      continue;
    }

    // 4. Send Range GET request payload
    vTaskDelay(20 / portTICK_PERIOD_MS);
    modem_response_buf[0] = '\0';
    SerialSIT.write((const uint8_t *)httpRequest, reqLen);
    SerialSIT.flush();

    // 5. Read HTTP Response Header & First Frame using readIpdFrame
    int headerFrameBytes = readIpdFrame(chunkBuf, 8192, 15000);
    if (headerFrameBytes <= 0) {
      Serial.printf("[OTA-TCP] Timeout waiting for HTTP response frame (attempt %d)\n", attempt + 1);
      SerialSIT.println("AT+CIPCLOSE=1"); waitForResponse("OK", 1000);
      continue;
    }
    const char *hPtr = strstr((const char *)chunkBuf, "HTTP/1.");
    if (!hPtr || (strncmp(hPtr + 8, " 2", 2) != 0 && strncmp(hPtr + 8, " 3", 2) != 0)) {
      char tmpHeader[201];
      int cpyLen = min(headerFrameBytes, 200);
      memcpy(tmpHeader, chunkBuf, cpyLen);
      tmpHeader[cpyLen] = '\0';
      Serial.printf("[OTA-TCP] Range GET non-2xx/3xx HTTP Response (attempt %d). Resp: '%s'\n", attempt + 1, tmpHeader);
      SerialSIT.println("AT+CIPCLOSE=1"); waitForResponse("OK", 1000);
      drainUartUntilQuiet(500);
      continue;
    }

    const char *headerEnd = memfind(hPtr, headerFrameBytes - (hPtr - (const char *)chunkBuf), "\r\n\r\n");
    if (!headerEnd) {
      Serial.printf("[OTA-TCP] Missing \\r\\n\\r\\n header boundary in response (attempt %d)\n", attempt + 1);
      SerialSIT.println("AT+CIPCLOSE=1"); waitForResponse("OK", 1000);
      drainUartUntilQuiet(500);
      continue;
    }

    // Extract range start & total file size from "Content-Range: bytes START-END/TOTAL"
    int rangeStart = -1;
    const char *crPtr = strcasestr(hPtr, "content-range:");
    if (crPtr) {
      const char *bytesPtr = strcasestr(crPtr, "bytes ");
      if (bytesPtr) {
        rangeStart = atoi(bytesPtr + 6);
      }
      const char *slashPtr = strchr(crPtr, '/');
      if (slashPtr && total_no_of_bytes == 0) {
        total_no_of_bytes = atoi(slashPtr + 1);
      }
    }
    if (total_no_of_bytes == 0) {
      const char *clPtr = strcasestr(hPtr, "content-length:");
      if (clPtr) total_no_of_bytes = atoi(clPtr + 15);
    }

    // Verify offset matching: rangeStart MUST match actual_downloaded
    if (rangeStart != -1 && rangeStart != actual_downloaded) {
      Serial.printf("[OTA-TCP] Content-Range mismatch: server returned start=%d, expected actual_downloaded=%d. Retrying Range GET!\n", rangeStart, actual_downloaded);
      SerialSIT.println("AT+CIPCLOSE=1"); waitForResponse("OK", 1000);
      drainUartUntilQuiet(500);
      continue;
    }

    Serial.printf("[OTA-TCP] Verified Content-Range start=%d, total file size=%d bytes\n", rangeStart, total_no_of_bytes);

    if (total_no_of_bytes <= 100000) {
      Serial.printf("[OTA-TCP] Invalid File Size (%d bytes). Rotating host...\n", total_no_of_bytes);
      SerialSIT.println("AT+CIPCLOSE=1"); waitForResponse("OK", 1000);
      drainUartUntilQuiet(500);
      hostIdx = (hostIdx + 1) % 2;
      connHost = ota_hosts[hostIdx];
      continue;
    }

    if (!updateInitialized && total_no_of_bytes > 0) {
      Update.abort(); // Clear any stale error flags
      const esp_partition_t *rPart = esp_ota_get_running_partition();
      const esp_partition_t *tPart = esp_ota_get_next_update_partition(rPart);
      Serial.printf("[OTA-TCP] Running: %s (0x%06X), Target: %s (0x%06X)\n",
                    rPart ? rPart->label : "NULL", rPart ? rPart->address : 0,
                    tPart ? tPart->label : "NULL", tPart ? tPart->address : 0);

      if (!Update.begin(total_no_of_bytes, U_FLASH)) {
        Serial.printf("[OTA-TCP] Update.begin failed: %s\n", Update.errorString());
        SerialSIT.println("AT+CIPCLOSE=1"); waitForResponse("OK", 1000);
        break;
      }
      updateInitialized = true;
      ota_writing_active = true;
      Serial.println("[OTA-TCP] Partition update initialized cleanly.");
    }

    // Process binary payload after \r\n\r\n in first frame
    headerEnd += 4; // Skip \r\n\r\n
    uint8_t *leftoverData = (uint8_t *)headerEnd;
    int leftoverBytes = headerFrameBytes - (leftoverData - chunkBuf);
    int chunkBytesReadThisReq = 0;

    if (leftoverBytes > 0) {
      if (!magicChecked && leftoverData[0] == 0xE9) {
        magicChecked = true;
        Serial.println("[OTA-TCP] Verified 0xE9 Magic Byte at exact body start!");
      }
      if (!writePayloadToFlash(leftoverData, leftoverBytes, total_no_of_bytes, actual_downloaded)) {
        actual_downloaded = 0;
        break;
      }
      chunkBytesReadThisReq += leftoverBytes;
    }

    // 6. Direct TCP Binary Streaming Loop for this 32KB chunk
    unsigned long chunkStart = millis();
    int readFails = 0;
    while (chunkBytesReadThisReq < expectedChunkLen && actual_downloaded < total_no_of_bytes && (millis() - chunkStart < 45000)) {
      esp_task_wdt_reset();
      int frameBytes = readIpdFrame(chunkBuf, 8192, 12000);
      if (frameBytes <= 0) {
        Serial.printf("[OTA-TCP] Frame read failed/timeout (%d). Breaking socket stream loop to retry Range GET from offset %d...\n", frameBytes, actual_downloaded);
        break; // Break IMMEDIATELY to avoid skipping binary bytes in flash stream!
      }
      readFails = 0;
      if (!magicChecked && chunkBuf[0] == 0xE9) magicChecked = true;

      if (!writePayloadToFlash(chunkBuf, frameBytes, total_no_of_bytes, actual_downloaded)) {
        actual_downloaded = 0;
        break;
      }
      chunkBytesReadThisReq += frameBytes;
    }

    int currentPct = (int)((float)actual_downloaded * 100.0 / total_no_of_bytes);
    Serial.printf("[OTA-TCP] Chunk download complete: %d / %d bytes (%d%%)\n", actual_downloaded, total_no_of_bytes, currentPct);

    SerialSIT.println("AT+CIPCLOSE=1"); waitForResponse("OK", 500);
    drainUartUntilQuiet(300); // Purge any trailing bytes from modem before next Range GET request
    vTaskDelay(2000 / portTICK_PERIOD_MS); // Recharge GPRS battery rail capacitor between 2G bursts
  }

  free(chunkBuf);
  SerialSIT.println("AT+CIPCLOSE=1");
  waitForResponse("OK", 1000);

  // Restore ATE1 command echo for normal AT command usage
  SerialSIT.println("ATE1"); waitForResponse("OK", 1000);

  // 6. Finalize OTA & Save Confirmation into NVS
  if (updateInitialized && actual_downloaded == total_no_of_bytes) {
    if (Update.end(true)) {
      Serial.println("[OTA-TCP] Flash COMPLETE and VERIFIED SUCCESSFUL!");
      const esp_partition_t *otaPart = esp_ota_get_next_update_partition(NULL);
      if (otaPart) {
        uint8_t hdr[16];
        if (esp_partition_read(otaPart, 0, hdr, sizeof(hdr)) == ESP_OK) {
          Serial.printf("[OTA-TCP] Verified Flash offset 0 header: %02X %02X %02X %02X %02X %02X %02X %02X\n",
                        hdr[0], hdr[1], hdr[2], hdr[3], hdr[4], hdr[5], hdr[6], hdr[7]);
        }
      }

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
      char succMsg[64];
      snprintf(succMsg, sizeof(succMsg), "Success: OTA %s Installed", UNIT_VER);
      p.putString("last_cmd_res", succMsg);
      p.putInt("last_cmd_id", last_cmd_id);
      p.end();

      Serial.println("[OTA-TCP] Rebooting in 3s into updated firmware...");
      vTaskDelay(3000 / portTICK_PERIOD_MS);
      ESP.restart();
    } else {
      int errCode = Update.getError();
      Serial.printf("[OTA-TCP] Update.end FAILED. Code: %d, Str: %s\n", errCode, Update.errorString());
      Preferences p;
      p.begin("ota-track", false);
      p.putInt("fail_cnt", p.getInt("fail_cnt", 0) + 1);
      char resBuf[32];
      snprintf(resBuf, sizeof(resBuf), "Fail: End Code %d", errCode);
      p.putString("fail_res", resBuf);
      p.end();
    }
  } else {
    Serial.printf("[OTA-TCP] Incomplete OTA: %d / %d bytes downloaded. Aborting.\n", actual_downloaded, total_no_of_bytes);
    if (updateInitialized) Update.abort();
    Preferences p;
    p.begin("ota-track", false);
    p.putInt("fail_cnt", p.getInt("fail_cnt", 0) + 1);
    p.putString("fail_res", "Fail: Stream Incomplete");
    p.end();
  }

  ota_writing_active = false;
  ota_silent_mode = false;
  if (!alreadyLocked) xSemaphoreGive(modemMutex);

  // Restore Modem URCs
  SerialSIT.println("AT+CREG=1"); waitForResponse("OK", 1000);
  SerialSIT.println("AT+CEREG=1"); waitForResponse("OK", 1000);
  SerialSIT.println("AT+CGEREP=2"); waitForResponse("OK", 1000);
  SerialSIT.println("AT+CNMI=2,1,0,0,0"); waitForResponse("OK", 1000);

  ota_writing_active = false;
  ota_silent_mode = false;

  if (!alreadyLocked) xSemaphoreGive(modemMutex);
  Serial.println("[OTA-TCP] Direct TCP OTA process finished.");
}

void fetchFromHttpAndUpdate(char *fileName, bool alreadyLocked) {
  if (!alreadyLocked) {
    if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(30000)) != pdTRUE) {
      Serial.println("[OTA] Error: Modem Mutex Timeout - deferring download");
      diag_modem_mutex_fails++;
      return;
    }
  }
  ota_silent_mode = true; // Rule 50: Silence sensor tasks before modem reconfiguration
  int handle_no;
  char gprs_xmit_buf[300];
  int total_no_of_bytes = 0;
  int actual_downloaded = 0;
  (void)handle_no; // suppress unused warning

  Serial.printf("[OTA] Starting Range-GET download for: %s\n", fileName);

  // v7.16: Rule 27 (Total Silence Protocol)
  // Completely silence the modem BEFORE any binary transfers occur.
  SerialSIT.println("AT+CREG=0");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CEREG=0");  // suppress LTE/EPS registration URCs
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CGEREP=0");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CGEV=0");   // suppress PDP context events
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CNMI=0,0,0,0,0");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CIURC=0");
  waitForResponse("OK", 1000);

  // === STEP 1: Size Probe GET request ===
  SerialSIT.println("AT+HTTPTERM");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+NETCLOSE");
  waitForResponse("OK", 3000);
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  if (!verify_bearer_or_recover()) {
    Serial.println("[OTA] Bearer verification failed. Aborting.");
    if (!alreadyLocked) xSemaphoreGive(modemMutex);
    return;
  }

  SerialSIT.println("AT+NETOPEN");
  waitForResponse("OK", 5000);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  flushSerialSIT();
  modem_response_buf[0] = '\0'; // Clear buffer of any error leftover

  Serial.println("[OTA] Executing AT+HTTPINIT...");
  SerialSIT.println("AT+HTTPINIT");
  waitForResponse("OK", 5000);
  Serial.printf("[OTA] HTTPINIT response: %s\n", modem_response_buf);
  modem_response_buf[0] = '\0';

  SerialSIT.println("AT+HTTPPARA=\"CID\",1");
  waitForResponse("OK", 2000);


  
  // Try raw IP first, fallback to domain (Omit :80 port if OTA_SERVER_PORT is "80")
  if (strcmp(OTA_SERVER_PORT, "80") == 0) {
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
             "AT+HTTPPARA=\"URL\",\"http://%s/builds/%s\"", HEALTH_SERVER_IP, fileName);
  } else {
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
             "AT+HTTPPARA=\"URL\",\"http://%s:%s/builds/%s\"", HEALTH_SERVER_IP, OTA_SERVER_PORT, fileName);
  }
  Serial.printf("[OTA] Setting URL: %s\n", gprs_xmit_buf);
  SerialSIT.println(gprs_xmit_buf);
  waitForResponse("OK", 2000);
  Serial.printf("[OTA] URL response: %s\n", modem_response_buf);
  modem_response_buf[0] = '\0';

  // Execute standard GET (without Range header) to retrieve Content-Length in +HTTPACTION URC
  flushSerialSIT();
  modem_response_buf[0] = '\0';
  Serial.println("[OTA] Triggering AT+HTTPACTION=0 (Size Probe)...");
  SerialSIT.println("AT+HTTPACTION=0");
  if (!waitForResponse("+HTTPACTION:", 25000)) {
    Serial.printf("[OTA] HTTPACTION Timeout/Error. Buffer: %s\n", modem_response_buf);
    SerialSIT.println("AT+HTTPTERM");
    waitForResponse("OK", 2000);
    if (!alreadyLocked) xSemaphoreGive(modemMutex);
    return;
  }
  Serial.printf("[OTA] HTTPACTION URC received: %s\n", modem_response_buf);

  // +HTTPACTION: 0,200,<total_bytes>
  int size_probe_status = 0;
  const char *first_c = strchr(modem_response_buf, ',');
  const char *last_c = strrchr(modem_response_buf, ',');
  if (first_c != NULL && last_c != NULL && last_c > first_c) {
    size_probe_status = atoi(first_c + 1);
    total_no_of_bytes = atoi(last_c + 1);
  }
  
  Serial.printf("[OTA] Size Probe result: Status=%d, Total Bytes=%d\n", size_probe_status, total_no_of_bytes);

  SerialSIT.println("AT+HTTPTERM");
  waitForResponse("OK", 2000);

  if (size_probe_status != 200 || total_no_of_bytes <= 100000) {
    Serial.printf("[OTA] Size Probe failed (Status: %d, Size: %d). Aborting.\n", size_probe_status, total_no_of_bytes);
    if (!alreadyLocked) xSemaphoreGive(modemMutex);
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
    Serial.printf("[OTA] Update.begin failed: %s\n", Update.errorString());
    if (!alreadyLocked) xSemaphoreGive(modemMutex); // v5.70: Fix Mutex Hierarchy - release before NVS
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

  Update.onProgress(progressCallBack);
  ota_writing_active = true;
  Serial.printf("[OTA] OTA partition ready (%d bytes). Downloading in 32KB Range chunks...\n", total_no_of_bytes);

  const int RANGE_SIZE = 32768; // Phase 8 Fix: Shrink to 32KB to avoid fragmentation failures
  const int READ_SIZE = 32768;
  uint8_t *buf = (uint8_t *)malloc(READ_SIZE);
  if (!buf) {
    Serial.println("[OTA] malloc failed!");
    Update.abort();
    ota_writing_active = false;
    ota_silent_mode = false;  // Fixed: restore silent mode correctly
    if (!alreadyLocked) xSemaphoreGive(modemMutex); // v5.66: Fix missing mutex release
    return;
  }

  int chunk_retries = 0;
  int consecutive_fails = 0; // Rule 42b: Hardware Reset Trigger

  // === STEP 3: Download 32KB chunks via Range GET ===
  while (actual_downloaded < total_no_of_bytes) {
    Serial.printf("[OTA] Range %d-%d (Progress: %.1f%%)\n", actual_downloaded,
                  min(actual_downloaded + RANGE_SIZE - 1, total_no_of_bytes - 1),
                  (float)actual_downloaded * 100.0 / total_no_of_bytes);

    if (consecutive_fails >= 3) {
      Serial.println("[OTA] [CRIT] 3 Consecutive chunk failures! Hard Hardware Resetting Modem...");
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

    modem_response_buf[0] = '\0'; // Clear buffer before HTTPINIT
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


    if (strcmp(OTA_SERVER_PORT, "80") == 0) {
      snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
               "AT+HTTPPARA=\"URL\",\"http://%s/builds/%s\"", HEALTH_SERVER_IP, fileName);
    } else {
      snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
               "AT+HTTPPARA=\"URL\",\"http://%s:%s/builds/%s\"", HEALTH_SERVER_IP, OTA_SERVER_PORT, fileName);
    }
    SerialSIT.println(gprs_xmit_buf);
    if (!waitForResponse("OK", 2000)) {
      // Fallback to domain
      if (strcmp(OTA_SERVER_PORT, "80") == 0) {
        snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
                 "AT+HTTPPARA=\"URL\",\"http://%s/builds/%s\"", HEALTH_SERVER_DOMAIN, fileName);
      } else {
        snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
                 "AT+HTTPPARA=\"URL\",\"http://%s:%s/builds/%s\"", HEALTH_SERVER_DOMAIN, OTA_SERVER_PORT, fileName);
      }
      SerialSIT.println(gprs_xmit_buf);
      waitForResponse("OK", 2000);
    }

    // Set Range
    int r_start = actual_downloaded;
    int r_end = min(actual_downloaded + RANGE_SIZE - 1, total_no_of_bytes - 1);
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf),
             "AT+HTTPPARA=\"USERDATA\",\"Range: bytes=%d-%d\"", r_start, r_end);
    SerialSIT.println(gprs_xmit_buf);
    waitForResponse("OK", 2000);

    // Execute GET
    Serial.printf("[OTA] Triggering GET Action (Range %d)... ", r_start);
    modem_response_buf[0] = '\0'; // Clear buffer before HTTPACTION
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
      Serial.printf("[OTA] [ERR] HTTP Error: %d. Resetting session.\n", http_status);
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      chunk_retries++;
      consecutive_fails++;
      esp_task_wdt_reset();
      continue;
    }

    // READ Data
    vTaskDelay(500 / portTICK_PERIOD_MS);
    flushSerialSIT();
    snprintf(gprs_xmit_buf, sizeof(gprs_xmit_buf), "AT+HTTPREAD=0,%d", chunk_total);
    SerialSIT.println(gprs_xmit_buf);

    char read_hdr[64] = {0};
    int rh_idx = 0;
    unsigned long t_start = millis();
    bool hdr_found = false;
    while (millis() - t_start < 12000) {
      esp_task_wdt_reset();
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
      taskYIELD();
    }

    if (!hdr_found) {
      Serial.println("[OTA] [ERR] +HTTPREAD header not found");
      chunk_retries++;
      consecutive_fails++;
      esp_task_wdt_reset();
      continue;
    }

    int safe_total = min(chunk_total, READ_SIZE);
    int got = 0;
    unsigned long startRead = millis();
    while (got < safe_total && (millis() - startRead < 120000)) {
      if (SerialSIT.available()) {
        buf[got++] = SerialSIT.read();
      } else {
        taskYIELD();
      }
      if (got % 1024 == 0)
        esp_task_wdt_reset();
    }

    if (got != safe_total) {
      Serial.printf("[OTA] [ERR] Read Timeout/Incomplete: got %d of %d\n", got, safe_total);
      flushSerialSIT();
      chunk_retries++;
      consecutive_fails++;
      esp_task_wdt_reset();
      continue;
    }

    // Consume the Tail
    unsigned long tail_start = millis();
    while (millis() - tail_start < 1000) {
      if (SerialSIT.find("OK"))
        break;
      taskYIELD();
    }

    if (!isBufferSanityOK(buf, got, actual_downloaded)) {
      Serial.println("[OTA] [ERR] Buffer Sanity check failed!");
      if (buf[0] == 0x75 && buf[1] == 0x68 && buf[2] == 0x40 && buf[3] == 0x08) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP.restart();
      }
      flushSerialSIT();
      chunk_retries++;
      consecutive_fails++;
      esp_task_wdt_reset();
      continue;
    }

    int bytes_to_write = min(got, total_no_of_bytes - actual_downloaded);
    esp_task_wdt_reset();
    if (Update.write(buf, bytes_to_write) == (size_t)bytes_to_write) {
      actual_downloaded += bytes_to_write;
      chunk_retries = 0;
      consecutive_fails = 0;

      Serial.printf("[OTA] Progress: %d / %d (HEAD: %02X %02X %02X %02X)\n",
                    actual_downloaded, total_no_of_bytes, buf[0], buf[1],
                    buf[2], buf[3]);
    } else {
      Serial.printf("[OTA] Flash Write ERROR: %d - %s\n", Update.getError(),
                     Update.errorString());
      Update.abort();
      break;
    }

    flushSerialSIT();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();
  }

  free(buf);
  ota_writing_active = false;
  ota_silent_mode = false;

  SerialSIT.println("ATE1");
  waitForResponse("OK", 500);
  SerialSIT.println("AT+CREG=1");
  waitForResponse("OK", 500);
  SerialSIT.println("AT+CGEREP=2");
  waitForResponse("OK", 500);

  // === STEP 4: Finalize OTA ===
  if (actual_downloaded == total_no_of_bytes && total_no_of_bytes > 0) {
    if (Update.end()) {
      Serial.println("[OTA] Flash COMPLETE and VERIFIED!");
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
      Serial.println("[OTA] Rebooting in 3s...");
      vTaskDelay(3000 / portTICK_PERIOD_MS);
      ESP.restart();
    } else {
      int errCode = Update.getError();
      Serial.printf("[OTA] Update.end FAILED. Code: %d, Str: %s\n", errCode, Update.errorString());
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
    Serial.printf("[OTA] Incomplete: %d/%d. Aborting.\n", actual_downloaded, total_no_of_bytes);
    Update.abort();
    Preferences p;
    p.begin("ota-track", false);
    p.putInt("fail_cnt", p.getInt("fail_cnt", 0) + 1);
    p.putString("fail_res", "Incomplete download");
    p.end();
  }

  SerialSIT.println("AT+CREG=1");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CGEREP=2");
  waitForResponse("OK", 1000);
  SerialSIT.println("AT+CNMI=2,1,0,0,0");
  waitForResponse("OK", 1000);

  SerialSIT.println("AT+HTTPTERM");
  waitForResponse("OK", 2000);
  SerialSIT.println("AT+HTTPINIT");
  if (!waitForResponse("OK", 2000)) {
    SerialSIT.println("AT+CREG=1");
    waitForResponse("OK", 1000);
    SerialSIT.println("AT+CEREG=1");
    waitForResponse("OK", 1000);
  }

  Serial.println("[OTA] Done.");
  if (!alreadyLocked)
    xSemaphoreGive(modemMutex);
}

void copyFromSPIFFSToFS(char *dateFile, bool alreadyLocked) {
  if (!alreadyLocked) {
    if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(15000)) != pdTRUE) {
      debugln("[FS] Error: Modem Mutex Timeout - deferring copy");
      diag_modem_mutex_fails++;
      return;
    }
  }
  if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
    debugln("[FS] Error: SPIFFS Mutex Timeout - deferring copy");
    if (!alreadyLocked)
      xSemaphoreGive(modemMutex); // RELEASE MODEM MUTEX
    return;
  }
  
  char SPIFFSFile[50], fileName[100];
  char stnId[32];
  
  if (strlen(station_name) == 4 && isDigitStr(station_name)) {
    snprintf(stnId, sizeof(stnId), "00%s", station_name);
  } else {
    strncpy(stnId, station_name, sizeof(stnId)-1);
    stnId[sizeof(stnId)-1] = '\0';
  }

  const char *prefix = "UN_";
#if SYSTEM == 0
  prefix = "TRG_";
#elif SYSTEM == 1
  prefix = "TWS_";
#elif SYSTEM == 2
  prefix = "TWSRF_";
#elif SYSTEM == 3
  prefix = "TWSRP_";
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
      if (!alreadyLocked) xSemaphoreGive(modemMutex);
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
      if (!alreadyLocked) xSemaphoreGive(modemMutex);
      xSemaphoreGive(fsMutex);
      return; // Exit if response is invalid
    }

    File file1 = SPIFFS.open(SPIFFSFile, FILE_READ);
    if (!file1) {
      debugln("C5 FIX: Failed to open SPIFFS source file. Aborting "
              "copyFromSPIFFSToFS.");
      if (!alreadyLocked) xSemaphoreGive(modemMutex);
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
  if (!alreadyLocked)
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
