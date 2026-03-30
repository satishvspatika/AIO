#include "globals.h"
#define IST_OFFSET_HOURS 5
#define IST_OFFSET_MINUTES 30
DateTime now;
#include <Wire.h>
#define RTC_ADDRESS 0x68 // DS1307 I2C address
int sec, mi, hr, dy, mo, yr;

int bcdToDec(byte val) { return ((val >> 4) * 10) + (val & 0x0F); }



bool rtc_ok;
byte data[7];
void rtcRead(void *pvParameters) {

  esp_task_wdt_add(NULL);
  String response;
  memset(signature, 0, sizeof(signature)); // Phase 14: Replaced hardcoded [16]=0 to dynamically protect string lengths

  rtcReady = false;
  timeSyncRequired = true;
  int rtcRetries = 0;
  int fail_count = 0;

  debugln("[RTC] Task Started");

  for (;;) {
    esp_task_wdt_reset();

    // v5.52: Absolute Silence Protocol — Pause task during OTA to prevent
    // crosstalk
    while (ota_silent_mode) {
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      esp_task_wdt_reset();
    }

    memset(data, 0, sizeof(data)); // Initialize array to zero
    rtc_ok = false;

    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
        pdTRUE) { // #TRUEFIX

      int read_retries = 0;
      while (read_retries < 3) {
        Wire.beginTransmission(RTC_ADDRESS);
        Wire.write(0x00);
        byte err = Wire.endTransmission();
        if (err == 0) {
          int bytes = Wire.requestFrom(RTC_ADDRESS, 7);
          if (bytes == 7) {
            for (int i = 0; i < 7; i++) {
              data[i] = Wire.read();
            }
            rtc_ok = true;
            rtcReady = true;
            break; // Success!
          }
        }

        read_retries++;
        debugf2("****RTC read error %d (Try %d/3)\n", err, read_retries);
        
        // v5.66: Only trigger recovery if err is 4 (Bus Busy/Hung) or 1 (Buffer Overflow)
        // If err is 2/3 (NACK), the device is just not responding (likely powered off).
        if (err == 4 || err == 1) {
            if (read_retries == 3) recoverI2CBus(true);
        }
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }

      xSemaphoreGive(i2cMutex);
    } else {
        // v5.66: Reduced debug noise if mutex is busy (expected during UI/Sensor reads)
    }

    if (!rtc_ok) {
      fail_count++;
      if (fail_count == 30) {
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
            pdTRUE) {
          recoverI2CBus(true); // Use unified recovery logic
          xSemaphoreGive(i2cMutex);
        }
      }
      if (fail_count > 60) {
        debugln("[RTC] Persistent Hardware Failure! Restarting...");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP.restart();
      }
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      continue;
    }
    fail_count = 0;

    sec = bcdToDec(data[0] & 0x7F);
    mi = bcdToDec(data[1]);
    hr = bcdToDec(data[2] & 0x3F);
    dy = bcdToDec(data[4]);
    mo = bcdToDec(data[5]);
    yr = 2000 + bcdToDec(data[6]);

    // Very simple sanity check
    valid_dt = (yr >= 2025 && yr <= 2060 && mo >= 1 && mo <= 12 && dy >= 1 && // v5.57 Fix: Extended from 2040 to 2060
                dy <= 31);
    valid_time =
        (hr >= 0 && hr <= 23 && mi >= 0 && mi <= 59); // Allow hour 0 (midnight)

    if (!valid_dt || !valid_time) {
      badReads++;
      rtcReady = false;
      timeSyncRequired = true;
      debug("rtcRead: Invalid RTC data with iter");
      debugln(badReads);
    } else {
      badReads = 0;
      rtcReady = true;
      timeSyncRequired = false;

      // Handle user manual RTC adjustment
      if (rtcTimeChanged) {
        debugln("rtcRead: Manual RTC time change detected");
        rtcTimeChanged = false;
      }

      if (mi != current_min) {
        if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
          debugf("[RTC] Time: %02d:%02d\n", hr, mi);
          xSemaphoreGive(serialMutex);
        }
      }
      // Update global time
      portENTER_CRITICAL(&rtcTimeMux);
      current_year = yr;
      current_month = mo;
      current_day = dy;
      current_hour = hr;
      current_min = mi;
      current_sec = sec;
      portEXIT_CRITICAL(&rtcTimeMux);

      // Compute nearest 15-min slot
      //        record_hr  = hr;
      //        record_min = (mi / 15) * 15;
      //        sampleNo   = (record_hr * 60 + record_min) / 15;
    }

    // Handle too many bad reads
    if (badReads >= 24) {
      portENTER_CRITICAL(&syncMux);
      int mode_snap = sync_mode;
      portEXIT_CRITICAL(&syncMux);
      bool gprs_idle = ((mode_snap == eHttpStop || mode_snap == eSMSStop ||
                         mode_snap == eExceptionHandled) &&
                        !health_in_progress && !wifi_active);
      if (gprs_idle) {
        badReads = -1;
        debugln("rtcRead: Too many bad reads — resyncing RTC");
        resync_time();
      } else {
        debugln("[RTC] Deferring resync, GPRS task is busy.");
      }
    }

    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(5000)); // wait 5 seconds
  }
}

void resync_time() {
  String response;
  int response_no;
  int tmp, tmp3;
  char tmp2[16];
  int day1, month1, year1, hour1, minute1, seconds1;
  char status_response[256];
  char gprs_xmit_buf[300];

  const char *csqstr;

  const char *response_char;

  debugln("[RTC] Resync Requested. Powering on GPRS...");
  debugln();
  health_in_progress = true; // Guard against sleep
  portENTER_CRITICAL(&syncMux);
  sync_mode = eHttpTrigger;  // Mark busy
  portEXIT_CRITICAL(&syncMux);

  signal_strength = 0;
  signal_lvl = 0;
  strcpy(reg_status, "NA");
  
  // H-NEW-1: Wrapped in critical section to prevent sleep gate race
  portENTER_CRITICAL(&syncMux);
  gprs_started = true;
  portEXIT_CRITICAL(&syncMux);

  // M-NEW-3: Pet WDT before and after the 15s blocking take
  esp_task_wdt_reset();
  if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(15000)) == pdTRUE) {
    esp_task_wdt_reset();
    start_gprs();
    esp_task_wdt_reset();
    SerialSIT.println("ATE0");
    response = waitForResponse("OK", 3000);
    debug("HTTP response of ATE0: ");
    debugln(response);

    vTaskDelay(5000 / portTICK_PERIOD_MS); 
    SerialSIT.println("AT+CLBS=4");
    response = waitForResponse("+CLBS:", 10000);
    xSemaphoreGive(modemMutex);
  } else {
    debugln("[RTC] Error: Modem Mutex Timeout - deferring resync");
    diag_modem_mutex_fails++;
    
    // Phase 11 Fix: Restore all pre-flight state so loop() can sleep normally.
    // Without this, health_in_progress=true and sync_mode=eHttpTrigger
    // are leaked permanently, blocking the sleep gate until reboot!
    health_in_progress = false;
    portENTER_CRITICAL(&syncMux);
    gprs_started = false;
    sync_mode = eExceptionHandled;
    portEXIT_CRITICAL(&syncMux);
    
    return;
  }
  
  vTaskDelay(200 / portTICK_PERIOD_MS);
  debug("Response of AT+CLBS=4 is ");
  debugln(response);
  vTaskDelay(200 / portTICK_PERIOD_MS);
  response_char = response.c_str();
  vTaskDelay(200 / portTICK_PERIOD_MS);
  csqstr = strstr(response_char, "+CLBS");

  if (csqstr != NULL) {
    // Robust Manual Parsing for +CLBS:
    // <err>,<lat>,<long>,<alt>,<year/mm/dd,hh:mm:ss> Handles SIMCOM 7672
    // response format variations (with or without quotes)
    const char *p = strchr(csqstr, ':');
    if (p) {

      p++; // Skip ':'
      tmp = atoi(p);
      p = strchr(p, ',');
      if (p) {
        lati = atof(++p);
        p = strchr(p, ',');
        if (p) {
          longi = atof(++p);
          p = strchr(p, ',');
          if (p) {
            tmp3 = atoi(++p); // skip altitude
            p = strchr(p, ',');
            if (p) {
              p++; // Skip comma
              // Handle optional quotes around date string
              if (*p == '"')
                p++;

              // v5.65 Fix: Robust manual parsing. Handle comma OR space as separator
              // between date and time (standard modems vary). Also use double %lf.
              if (sscanf(p, "%d/%d/%d%*[ ,]%d:%d:%d", &year1, &month1, &day1, &hour1,
                         &minute1, &seconds1) == 6) {
                debugln("[RTC] CLBS data parsed successfully (Manual)");
                parse_and_convert_clbs_response(response_char, year1, month1,
                                                day1, hour1, minute1, seconds1);
                badReads = 0;
              } else {
                debugln("[RTC] Error: Date/Time parsing failed");
              }
            }
          }
        }
      }
    }
  } else {
    debugln("Error: +CLBS not found in response - will retry later");
    // Removed ESP.restart() to prevent boot loops on poor signal
  }

  //  sscanf(csqstr, "+CLBS: %d,%f,%f,%d,%d/%d/%d,%d:%d:%d", &tmp,
  //  &lati,&longi,&tmp3,&year1,&month1,&day1,&hour1,&minute1,&seconds1);

  // Parse and convert the time from the response
}

// Function to parse the +CLBS response and convert to IST
void parse_and_convert_clbs_response(const char *response, int year1,
                                     int month1, int day1, int hour1,
                                     int minute1, int seconds1) {
  struct tm timeinfo = {0};

  // Example response: +CLBS: 0,1,"2024/10/24,14:32:45"
  // Find the position of the time string in the response
  char time_str[20];

  // Fill the tm structure with parsed data
  timeinfo.tm_year = year1 - 1900; // Year since 1900
  timeinfo.tm_mon = month1 - 1;    // Month is 0-based in struct tm
  timeinfo.tm_mday = day1;
  timeinfo.tm_hour = hour1;
  timeinfo.tm_min = minute1;
  timeinfo.tm_sec = seconds1;

  // Print the original GMT time
  debugf("Original GMT Time: %s", asctime(&timeinfo));

  // Convert to IST
  convert_gmt_to_ist(&timeinfo);

  // Print the converted IST time
  debugf("Converted IST Time: %s", asctime(&timeinfo));

  if ((timeinfo.tm_mon + 1) <= 12 && (timeinfo.tm_mday <= 31) &&
      (timeinfo.tm_year + 1900 <= 2099) && (timeinfo.tm_hour < 24) &&
      (timeinfo.tm_min < 60)) { // Strict bounds: hour 0-23, minute 0-59
    // This is only when there is a resync of RTC that requires storing
    // last_recorded date to NVRAM, otherwise hr:min will be 00:00 and sampleNo
    // will be calculated wrongly
    now = rtc.now();
    //                    record_hr = now.hour(); record_min = now.minute();
    //                    current_hour = now.hour(); current_min = now.minute();
    //                    // This is for getting into the loop of %15 in
    //                    scheduler

    last_recorded_dd = timeinfo.tm_mday;
    last_recorded_mm = timeinfo.tm_mon + 1;
    last_recorded_yy = timeinfo.tm_year + 1900;
    last_recorded_hr = timeinfo.tm_hour;
    last_recorded_min = timeinfo.tm_min;

    debug("Last recorded Date : ");
    debug(last_recorded_dd);
    debug('/');
    debug(last_recorded_mm);
    debug('/');
    debug(last_recorded_yy);
    debug(" , Time : ");
    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
      debugf("%02d:%02d\n", last_recorded_hr, last_recorded_min);
      xSemaphoreGive(serialMutex);
    }
    debugln();
    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
        pdTRUE) {
      rtc.adjust(DateTime(last_recorded_yy, last_recorded_mm, last_recorded_dd,
                          last_recorded_hr, last_recorded_min, 0));
      xSemaphoreGive(i2cMutex);
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    // Assign for making the correct decision on time to sleep
    portENTER_CRITICAL(&rtcTimeMux);
    current_year = last_recorded_yy;
    current_month = last_recorded_mm;
    current_day = last_recorded_dd;
    current_hour = last_recorded_hr;
    current_min = last_recorded_min;
    portEXIT_CRITICAL(&rtcTimeMux);

    // v5.59: Atomic Save for signature.txt
    if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
      File fTmp = SPIFFS.open("/signature.tmp", FILE_WRITE);
      if (!fTmp) {
        debugln("Failed to open signature.tmp for writing");
      } else {
        snprintf(signature, sizeof(signature), "%04d-%02d-%02d,%02d:%02d",
                 last_recorded_yy, last_recorded_mm, last_recorded_dd,
                 last_recorded_hr, last_recorded_min);
        fTmp.print(signature);
        fTmp.close();
        if (SPIFFS.exists("/signature.txt"))
          SPIFFS.remove("/signature.txt");
        SPIFFS.rename("/signature.tmp", "/signature.txt");
        debugln("[RTC] Signature persisted ATOMICALLY.");
      }
      xSemaphoreGive(fsMutex);
    } else {
      debugln("[RTC] Failed to take fsMutex for signature save. File skipped.");
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);

    if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
      debugf("[RTC] Time: %02d:%02d\n", current_hour, current_min);
      xSemaphoreGive(serialMutex);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    health_in_progress = false; // Task COMPLETED
    portENTER_CRITICAL(&syncMux);
    gprs_started = false;       // Hardened: only the wrapped version remains
    sync_mode = eExceptionHandled;
    portEXIT_CRITICAL(&syncMux);

  } else {
    debugln("Failed to get the correct time");
    health_in_progress = false; // Allow error recovery
    portENTER_CRITICAL(&syncMux);
    gprs_started = false;       // Hardened: wrapped in syncMux
    sync_mode = eExceptionHandled;
    portEXIT_CRITICAL(&syncMux);
  }
}
