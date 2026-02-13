#include "globals.h"
#define IST_OFFSET_HOURS 5
#define IST_OFFSET_MINUTES 30
DateTime now;
#include <Wire.h>
#define RTC_ADDRESS 0x68 // DS1307 I2C address
int sec, mi, hr, dy, mo, yr;

int bcdToDec(byte val) { return ((val >> 4) * 10) + (val & 0x0F); }

int timeToMinutes(int years, int months, int dd, int hr, int mins) {
  return (((yr - 2000) * 365 + (months - 1) * 30 + (dd - 1)) * 24 * 60) +
         (hr * 60 + mins);
}

bool rtc_ok;
byte data[7];
void rtcRead(void *pvParameters) {

  esp_task_wdt_add(NULL);
  signature[16] = 0;

  rtcReady = false;
  timeSyncRequired = true;
  int rtcRetries = 0;
  int fail_count = 0;

  debugln("[RTC] Task Started");

  for (;;) {

    memset(data, 0, sizeof(data)); // Initialize array to zero
    rtc_ok = false;

    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
        pdTRUE) { // #TRUEFIX

      Wire.beginTransmission(RTC_ADDRESS);
      Wire.write(0x00);
      Wire.endTransmission();

      int bytes = Wire.requestFrom(RTC_ADDRESS, 7);
      if (bytes == 7) {
        for (int i = 0; i < 7; i++) {
          data[i] = Wire.read();
        }
        rtc_ok = true;
      } else {
        debugln("****RTC read incomplete");
      }

      xSemaphoreGive(i2cMutex);
    }

    if (!rtc_ok) {
      fail_count++;
      if (fail_count == 30) {
        debugln("[RTC] I2C Failure. Attempting Bus Reset...");
        Wire.end();
        delay(100);
        Wire.begin(21, 22, 25000);
        Wire.setTimeOut(I2C_TIMEOUT_MS); // Restore timeout after reset
        delay(100);
      }
      if (fail_count > 60) {
        debugln("[RTC] Persistent Hardware Failure! Restarting...");
        vTaskDelay(1000);
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
    valid_dt = (yr >= 2025 && yr <= 2040 && mo >= 1 && mo <= 12 && dy >= 1 &&
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
        // debugln();
        debug("[RTC] Time: ");
        debug(hr);
        debug(":");
        debugln(mi);
      }
      // Update global time
      current_year = yr;
      current_month = mo;
      current_day = dy;
      current_hour = hr;
      current_min = mi;
      current_sec = sec;

      // Compute nearest 15-min slot
      //        record_hr  = hr;
      //        record_min = (mi / 15) * 15;
      //        sampleNo   = (record_hr * 60 + record_min) / 15;
    }

    // Handle too many bad reads
    if (badReads >= 24) {
      badReads = -1;
      debugln("rtcRead: Too many bad reads — resyncing RTC");
      resync_time();
    }

    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(5000)); // wait 5 seconds
  }
}

void resync_time() {

  int response_no;
  int tmp, tmp3;
  char tmp2[16];
  int day1, month1, year1, hour1, minute1, seconds1;
  char status_response[256];
  char gprs_xmit_buf[300];

  char *csqstr;
  float lati, longi;

  debugln("[RTC] Resync Requested. Powering on GPRS...");
  debugln();
  signal_strength = 0;
  signal_lvl = 0;
  strcpy(reg_status, "NA");
  gprs_started = true;
  start_gprs();
  esp_task_wdt_reset();
  SerialSIT.println("ATE0");
  response = waitForResponse("OK", 3000);
  debug("HTTP response of ATE0: ");
  debugln(response);

  const char *response_char;

  vTaskDelay(5000);
  SerialSIT.println("AT+CLBS=4");
  response = waitForResponse("+CLBS:", 10000);
  vTaskDelay(200);
  debug("Response of AT+CLBS=4 is ");
  debugln(response);
  vTaskDelay(200);
  response_char = response.c_str();
  vTaskDelay(200);
  csqstr = strstr(response_char, "+CLBS");

  if (csqstr != NULL) {
    // Safe to parse
    int parsed = sscanf(csqstr, "+CLBS: %d,%f,%f,%d,%d/%d/%d,%d:%d:%d", &tmp,
                        &lati, &longi, &tmp3, &year1, &month1, &day1, &hour1,
                        &minute1, &seconds1);

    if (parsed == 10) {
      debugln("CLBS data parsed successfully");
      parse_and_convert_clbs_response(response_char, year1, month1, day1, hour1,
                                      minute1, seconds1);
      badReads = 0;
    } else {
      debug("Warning: sscanf parsed only ");
      debugln(parsed);
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
  printf("Original GMT Time: %s", asctime(&timeinfo));

  // Convert to IST
  convert_gmt_to_ist(&timeinfo);

  // Print the converted IST time
  printf("Converted IST Time: %s", asctime(&timeinfo));

  if ((timeinfo.tm_mon + 1) <= 12 && (timeinfo.tm_mday <= 31) &&
      (timeinfo.tm_year + 1900 <= 2099) && (timeinfo.tm_hour <= 24) &&
      (timeinfo.tm_min <= 60)) { // Basic check
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
    debug(last_recorded_hr);
    debug(':');
    debugln(last_recorded_min);
    debugln();
    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
        pdTRUE) {
      rtc.adjust(DateTime(last_recorded_yy, last_recorded_mm, last_recorded_dd,
                          last_recorded_hr, last_recorded_min, 0));
      xSemaphoreGive(i2cMutex);
    }
    vTaskDelay(2000);

    // Assign for making the correct decision on time to sleep
    current_year = last_recorded_yy;
    current_month = last_recorded_mm;
    current_day = last_recorded_dd;
    current_hour = last_recorded_hr;
    current_min = last_recorded_min;

    //                    current_hour = now.hour(); current_min = now.minute();
    //                    // This is for getting into the loop of %15 in
    //                    scheduler current_hour = now.hour(); current_min =
    //                    now.minute(); // This is for getting into the loop of
    //                    %15 in scheduler
    File fileTemp2 = SPIFFS.open("/signature.txt", FILE_WRITE);
    if (!fileTemp2) {
      debugln("Failed to open signature.txt for writing");
    } // #TRUEFIX
    snprintf(signature, sizeof(signature), "%04d-%02d-%02d,%02d:%02d",
             last_recorded_yy, last_recorded_mm, last_recorded_dd,
             last_recorded_hr, last_recorded_min);
    fileTemp2.print(signature);
    fileTemp2.close();
    vTaskDelay(500);
    debug("[RTC] Time: ");
    debug(current_hour);
    debug(":");
    debugln(current_min);
    vTaskDelay(1000);
    sync_mode = eExceptionHandled;

  } else {
    debugln("Failed to get the correct time");
    sync_mode = eExceptionHandled;
  }
}
