#include "globals.h"

void start_deep_sleep() {

  if (wired == 1) {
    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
        pdTRUE) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("SYSTEM SLEEPING");
      lcd.setCursor(0, 1);
      lcd.print("  GOODBYE...   ");
      xSemaphoreGive(i2cMutex);

      vTaskDelay(1000 / portTICK_PERIOD_MS); // Show for 1s before power cut

      if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
          pdTRUE) {
        lcd.noBacklight();
        digitalWrite(32, LOW); // Turn OFF power to LCD (5V)
        xSemaphoreGive(i2cMutex);
      }
    }
  }

  // CRITICAL: Ensure ULP counts into the main RF bucket during sleep!
  calib_mode_flag.val = 0;
  calib_flag = 0; // Reset UI state too

  WiFi.disconnect();
  digitalWrite(32, LOW); // Turn off LCD (5V)

  // Ensure GPRS is shut down gracefully ALWAYS before cutting power
  esp_task_wdt_reset();
  graceful_modem_shutdown();
  esp_task_wdt_reset();
  delay(100);

  // SELF-HEALING: Force GPIO 26 and 32 to stay strictly LOW during sleep
  // This prevents the modem from staying in a "zombie" state.
  gpio_hold_en(GPIO_NUM_26);
  gpio_hold_en(GPIO_NUM_32);
  gpio_deep_sleep_hold_en();

  // Calculate time to sleep to target the NEXT exact 15-minute boundary
  // We calculate in SECONDS for precision using current_sec
  int next_boundary_min = ((current_min / 15) + 1) * 15;
  int sleep_seconds;

  // Handle hour rollover (e.g., 59 minutes -> next hour at 0 minutes)
  // v7.89: Reverted to +30s offset for maximum stability.
  if (next_boundary_min >= 60) {
    next_boundary_min = 0;
    sleep_seconds = (60 - current_min) * 60 - current_sec + 30;
  } else {
    sleep_seconds =
        (next_boundary_min * 60) - (current_min * 60 + current_sec) + 30;
  }

  // Safety bounds: 1 minute minimum, 20 minutes maximum
  if (sleep_seconds < 60) {
    debugln("[PWR] Sleep time < 1min, setting to 1min");
    sleep_seconds = 60;
  }
  if (sleep_seconds > 1200) {
    debugln("[PWR] Sleep time > 20min, setting to 15min");
    sleep_seconds = 900; // Default to 15 mins
  }

  debug("[PWR] Sleep: CurTime=");
  debug(current_hour);
  debug(":");
  debug(current_min);
  debug(":");
  debugln(current_sec);
  debug(" Sleep=");
  debug(sleep_seconds / 60);
  debug(":");
  debug(sleep_seconds % 60);
  debugln(" (min:sec)");

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_27, LOW);
  // Use the calculated seconds directly
  esp_sleep_enable_timer_wakeup((uint64_t)sleep_seconds * uS_TO_S_FACTOR);

  // CRITICAL: Restored from Version 3.0. Adjusts ULP startup delay to prevent
  // hang in deep sleep.
  REG_SET_FIELD(RTC_CNTL_TIMER2_REG, RTC_CNTL_ULPCP_TOUCH_START_WAIT, 0x20);

  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

  Serial.flush();
  debugln("[PWR] Entering Deep Sleep");
  esp_deep_sleep_start();
}

// Validate ULP counters to prevent memory corruption
void validate_ulp_counters() {
  bool corrupted = false;

  // RF count shouldn't exceed 10000 (2500mm at 0.25mm resolution = unrealistic)
  // Note: ulp_var_t.val is unsigned - check for wrap-around (very large values)
  // which is how corruption manifests (val wraps from 0 to ~65535)
  if (rf_count.val > 10000) {
    debugf1("[ULP] RF count corrupted (%u), resetting\n", rf_count.val);
    rf_count.val = 0;
    corrupted = true;
  }

  // Wind count shouldn't exceed 65000 pulses per 15 min (unrealistic)
  // 65000 pulses / 4 teeth / 900 sec * 0.4398 = ~7.9 m/s continuous — cap at 2x
  if (wind_count.val > 130000) {
    debugf1("[ULP] Wind count corrupted (%u), resetting\n", wind_count.val);
    wind_count.val = 0;
    corrupted = true;
  }

  // Calib count shouldn't exceed 1000
  if (calib_count.val > 1000) {
    debugf1("[ULP] Calib count corrupted (%u), resetting\n", calib_count.val);
    calib_count.val = 0;
    corrupted = true;
  }

  if (corrupted) {
    debugln("[ULP] Memory corruption detected and fixed");
  }
}

void set_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
  case ESP_SLEEP_WAKEUP_EXT0:
    debugln("Wakeup caused by external signal using RTC_IO");
    wakeup_reason_is = ext0;
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    debugln("Wakeup caused by external signal using RTC_CNTL");
    wakeup_reason_is = ext1;
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    debugln("Wakeup caused by timer");
    wakeup_reason_is = timer;
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    debugln("Wakeup caused by touchpad");
    wakeup_reason_is = touchpad;
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    debugln("Wakeup caused by ULP program");
    wakeup_reason_is = ulp;
    break;
  default:
    debugf1("Wakeup was not caused by deep sleep: %d\n", wakeup_reason_is);
    break;
  }
}

void copyFilesFromSPIFFSToSD(const char *dirname) {
  File root = SPIFFS.open(dirname);
  if (!root) {
    debugln("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    debugln("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    esp_task_wdt_reset();
    String fileName = file.name();
    if (fileName.endsWith(".txt")) {
      String destPath = fileName;
      if (!fileName.startsWith("/")) {
        destPath = "/" + fileName;
      }

      File sourceFile = SPIFFS.open(fileName, FILE_READ);
      File destFile;
      if (sd_card_ok) {
        destFile = SD.open(destPath, FILE_WRITE);
      }

      if (sourceFile && destFile) {
        debug("Copying: ");
        debugln(fileName);
        uint8_t buffer[512];
        while (sourceFile.available()) {
          size_t bytesRead = sourceFile.read(buffer, sizeof(buffer));
          destFile.write(buffer, bytesRead);
        }
        destFile.close();
        sourceFile.close();
      }
    }
    file.close(); // v5.49 Build 5: FIX LEAK
    file = root.openNextFile();
  }
}

void removeFilesFromSPIFFS(const char *dirname) {
  File root = SPIFFS.open(dirname);
  if (!root) {
    debugln("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    debugln("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    esp_task_wdt_reset();
    String fileName = file.name();
    if (fileName.endsWith(".txt")) {
      debug("Deleting: ");
      debugln(fileName);
      SPIFFS.remove(fileName);
    }
    file.close(); // v5.49 Build 5: FIX LEAK
    file = root.openNextFile();
  }
}

void delete_multiple_files(const char *station) {
  File root = SPIFFS.open("/");
  if (!root) {
    debugln("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    debugln("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    esp_task_wdt_reset();
    String fileName = file.name();
    if (fileName.indexOf(station) != -1) {
      debug("Deleting: ");
      debugln(fileName);
      SPIFFS.remove(fileName);
    }
    file.close(); // v5.49 Build 5: FIX LEAK
    file = root.openNextFile();
  }
}

void get_chip_id() {
  uint64_t chipid = ESP.getEfuseMac();
  snprintf(chip_id, sizeof(chip_id), "%04X%08X", (uint16_t)(chipid >> 32),
           (uint32_t)chipid);
  debug("Chip ID: ");
  debugln(chip_id);
}

/**
 * Perform manual I2C Bus Recovery
 * If a slave device is hanging (holding SDA low), we pulse SCL until it
 * releases
 */
void recoverI2CBus() {
  debugln("[I2C] 🚨 Bus hang detected! Attempting recovery...");

  // End any existing Wire session
  Wire.end();

  // Switch SCL to output to drive clock pulses
  pinMode(I2C_SCL, OUTPUT);
  digitalWrite(I2C_SCL, HIGH);

  // Switch SDA to input to MONITOR if it is released
  pinMode(I2C_SDA, INPUT_PULLUP);
  vTaskDelay(1 / portTICK_PERIOD_MS);

  // If SDA is stuck low, pulse SCL up to 9 times to "clock out" the hanging
  // data bit
  bool recovered = false;
  for (int i = 0; i < 10; i++) {
    if (digitalRead(I2C_SDA) == HIGH) {
      debugf1("[I2C] Bus released after %d pulses\n", i);
      recovered = true;
      break;
    }
    digitalWrite(I2C_SCL, LOW);
    delayMicroseconds(10);
    digitalWrite(I2C_SCL, HIGH);
    delayMicroseconds(10);
  }

  if (!recovered) {
    debugln("[I2C] ❌ Recovery failed: SDA still stuck low!");
  } else {
    debugln("[I2C] ✅ Recovery successful.");
  }

  // Re-initialize Wire
  Wire.begin(I2C_SDA, I2C_SCL, 100000);
  Wire.setTimeOut(I2C_TIMEOUT_MS);
}

/**
 * Persists current coordinates and timestamp (v5.49) to SPIFFS
 */
void saveGPS() {
  File file = SPIFFS.open("/gps_fix.txt", FILE_WRITE);
  if (file) {
    file.printf("%.6f,%.6f,%d,%d,%d", lati, longi, current_day, current_month,
                current_year);
    file.close();
    gps_fix_dd = current_day;
    gps_fix_mm = current_month;
    gps_fix_yy = current_year;
    debugln("[GPS] Location and Date persisted to SPIFFS");
  }
}

/**
 * Loads last known coordinates and timestamp (v5.49) from SPIFFS
 */
void loadGPS() {
  if (SPIFFS.exists("/gps_fix.txt")) {
    File file = SPIFFS.open("/gps_fix.txt", FILE_READ);
    if (file) {
      String line = file.readString();
      file.close();
      if (sscanf(line.c_str(), "%f,%f,%d,%d,%d", &lati, &longi, &gps_fix_dd,
                 &gps_fix_mm, &gps_fix_yy) >= 2) {
        debugf5("[GPS] Loaded from SPIFFS: %.6f,%.6f (Fix: %02d/%02d/%d)\n",
                lati, longi, gps_fix_dd, gps_fix_mm, gps_fix_yy);
      }
    }
  } else {
    debugln("[GPS] No persisted location found.");
  }
}

bool isDigitStr(const char *s) {
  if (!s || !*s)
    return false;
  while (*s) {
    if (!isdigit(*s))
      return false;
    s++;
  }
  return true;
}
// --- Rainfall Integrity Check (v5.46) ---
// Verifies if cum_rf = prev_cum_rf + inst_rf for all records in the daily file.
// Also detects if cum_rf has unexpectedly reset to zero.
void checkRainfallIntegrity() {
  if (SYSTEM == 1)
    return; // No rainfall for TWS

  if (!SPIFFS.exists(cur_file)) {
    debugf1("[RainCheck] File %s not found. Skipping integrity check.\n",
            cur_file);
    return;
  }

  File file = SPIFFS.open(cur_file, FILE_READ);
  if (!file) {
    debugln("[RainCheck] Failed to open file for integrity check.");
    return;
  }

  float prev_cum = 0;
  bool first_record = true;
  char line[128];

  while (file.available()) {
    int l = file.readBytesUntil('\n', line, sizeof(line) - 1);
    line[l] = '\0';
    if (l < 10)
      continue; // Skip trash

    // Parse CSV fields
    char *p = line;
    int field = 0;
    int sample_no = -1;
    float inst = 0, cum = 0;

    char *token = strtok(p, ",");
    while (token != NULL) {
      if (field == 0)
        sample_no = atoi(token);
      else if (field == 3) {
        if (SYSTEM == 0)
          inst = atof(token);
        else if (SYSTEM == 2)
          cum = atof(token); // SYSTEM 2: Field 3 is Cum_RF
      } else if (field == 4 && SYSTEM == 0) {
        cum = atof(token); // SYSTEM 0: Field 4 is Cum_RF
      }
      token = strtok(NULL, ",");
      field++;
    }

    if (sample_no == -1)
      continue;

    // Reset detection: If cum drops below prev_cum (and it's not Sample 0)
    if (!first_record && sample_no > 0) {
      if (cum < prev_cum - 0.01) {
        diag_rain_reset = true;
#if DEBUG == 1
        debugf("[RainCheck] RESET detected @ Sample %d (%.2f < %.2f)\n",
               sample_no, cum, prev_cum);
#endif
      }

      // Calculation detection: cum = prev_cum + inst (TRG Only)
      if (SYSTEM == 0) {
        if (abs(cum - (prev_cum + inst)) > 0.05) {
          diag_rain_calc_invalid = true;
#if DEBUG == 1
          debugf("[RainCheck] CALC error @ Sample %d: %.2f != %.2f + %.2f\n",
                 sample_no, cum, prev_cum, inst);
#endif
        }
      }
    }

    prev_cum = cum;
    first_record = false;
  }

  file.close();
}

// v5.49 analyzeFileHealth: Updates Net Data Count and flags alarms only if
// STILL missing from server
void analyzeFileHealth(uint32_t *mask, int *outNetCount, bool *hasUnresolvedPD,
                       bool *hasUnresolvedNDM) {
  *outNetCount = 0;
  *hasUnresolvedPD = false;
  *hasUnresolvedNDM = false;

  // Calculate where we are in the meteorological day (0-95) for live testing
  int current_s_idx = current_hour * 4 + current_min / 15;
  current_s_idx = (current_s_idx + 61) % 96;

  for (int i = 0; i < 96; i++) {
    bool delivered = (mask[i / 32] & (1UL << (i % 32)));

    if (delivered) {
      (*outNetCount)++;
    } else {
      // Alarm Flag: Only trigger if the sample is in the PAST and NOT delivered
      // (Server Side)
      bool isPast = (mask == diag_sent_mask_prev) ? true : (i < current_s_idx);
      if (isPast) {
        // Night Data Tracking: 9 PM (Sample 49) to 6 AM (Sample 85)
        if (i >= 49 && i <= 85) {
          *hasUnresolvedNDM = true;
        }
      }
    }
  }

  // v5.68: Threshold 86 (Only flag PD if more than 10 slots are missing)
  if (mask == diag_sent_mask_prev && *outNetCount < 86) {
    *hasUnresolvedPD = true;
  }

  // CDM Calculation: Only judge the deadline if we are analyzing the CLOSED day
  // (prev)
  if (mask == diag_sent_mask_prev) {
    if (mask[2] & (1UL << (95 % 32))) {
      strcpy(diag_cdm_status, "OK");
    } else {
      strcpy(diag_cdm_status, "FAIL");
    }
  } else {
    // For the CURRENT Met Day, we don't judge CDM until 08:30 AM tomorrow.
    // We only set it to OK here; it doesn't overwrite the 'Official' Prev
    // status.
  }

  const char *dayLabel = (mask == diag_sent_mask_prev) ? "YDY" : "TDY";

#if DEBUG == 1
  debugf("[HealthScan %s] NetCount: %d | CDM: %s | PD: %s | NDM: %s\n",
         dayLabel, *outNetCount, diag_cdm_status,
         (*hasUnresolvedPD ? "FAIL" : "OK"),
         (*hasUnresolvedNDM ? "FAIL" : "OK"));
#endif
}

// One-time reconstruction of sent masks from SPIFFS files (Session Recovery)
RTC_DATA_ATTR bool backfill_done = false;

void scanFileToMask(const char *fName, uint32_t *mask) {
  if (!SPIFFS.exists(fName))
    return;
  File f = SPIFFS.open(fName, FILE_READ);
  if (!f)
    return;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    if (line.length() < 5)
      continue;
    int commaIdx = line.indexOf(',');
    if (commaIdx != -1) {
      int sNum = -1;
      int sigVal = 0;
      
      // Basic CSV parse to find SampleNo and SignalStrength
      char buf[128];
      strncpy(buf, line.c_str(), sizeof(buf)-1);
      buf[sizeof(buf)-1] = '\0';
      
      char *token;
      token = strtok(buf, ",");
      if (token) sNum = atoi(token);
      
      // Hop to field 8 (Signal Strength)
      for (int i = 0; i < 8 && token != NULL; i++) {
        token = strtok(NULL, ",");
      }
      
      if (token) sigVal = atoi(token);

      // v5.51: Skip markers that indicate placeholders, not real data delivery.
      // -111 to -114: Safe Zone for Gaps/Placeholder markers.
      if (sNum >= 0 && sNum <= 95) {
        // v5.52: Explicitly exclude our internal safe-zone markers
        if (sigVal != -111 && sigVal != -112 && sigVal != -113 && sigVal != -114) { 
          // Signal is a real GSM value (e.g. -69, -85, -101)
          mask[sNum / 32] |= (1UL << (sNum % 32));
        }
      }
    }
  }
  f.close();
}

int countStored(const char *fName) {
  if (!SPIFFS.exists(fName))
    return 0;
  File f = SPIFFS.open(fName, FILE_READ);
  if (!f)
    return 0;
  int count = 0;
  while (f.available()) {
    // v7.95: Keep WDT alive during file scan
    esp_task_wdt_reset();
    String line = f.readStringUntil('\n');
    // v7.70: Relaxed length to 10 for safety; check for comma (CSV) or semicolon (FTP format)
    // v5.52 BUG-2 FIX: FTP records use ';' not ',', so check both separators
    if (line.length() >= 10 && (line.indexOf(',') != -1 || line.indexOf(';') != -1)) {
      count++;
      if (count >= 120) // v7.95: Increased cap to 120 for safety
        break; // v7.59: Cap — ghost gap-fill cannot inflate beyond one day
    }
  }
  f.close();
  return count;
}

float restoreRainfall(const char *fName) {
  if (!SPIFFS.exists(fName))
    return 0.0;
  File f = SPIFFS.open(fName, FILE_READ);
  if (!f)
    return 0.0;

  // v7.67: Efficiently read last valid line by seeking backwards from EOF
  // avoids allocating 96 String objects for a full-file scan.
  int fSize = f.size();
  if (fSize < 20) {
    f.close();
    return 0.0;
  }

  // Seek back up to 512 bytes from EOF to capture the last 1-2 lines
  int seekFrom = max(0, fSize - 512);
  f.seek(seekFrom);
  String lastLine = "";
  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() > 20)
      lastLine = line;
  }
  f.close();

  if (lastLine.length() > 0) {
    // CSV format: sampleNo,DATE,TIME,CUM_RF,...
    // Field index 3 (0-based) is the rainfall field for SYSTEM 0 and 2.
    // For SYSTEM 1 (TWS - no rainfall), this returns 0 safely.
    int firstComma = lastLine.indexOf(',');
    if (firstComma == -1)
      return 0.0;
    int secondComma = lastLine.indexOf(',', firstComma + 1);
    if (secondComma == -1)
      return 0.0;
    int thirdComma = lastLine.indexOf(',', secondComma + 1);
    if (thirdComma == -1)
      return 0.0;
    int fourthComma = lastLine.indexOf(',', thirdComma + 1);
    // fourthComma may be -1 (last field), substring still works
    String rfStr = lastLine.substring(thirdComma + 1, fourthComma);
    rfStr.trim();
    return rfStr.toFloat();
  }
  return 0.0;
}

int countNightStored(const char *fName) {
  if (!SPIFFS.exists(fName))
    return 0;
  File f = SPIFFS.open(fName, FILE_READ);
  if (!f)
    return 0;
  int count = 0;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    if (line.length() >= 10) {
      int commaIdx = line.indexOf(',');
      if (commaIdx != -1) {
        int sNum = line.substring(0, commaIdx).toInt();
        if (sNum >= 49 && sNum <= 85) {
          count++;
        }
      }
    }
  }
  f.close();
  return count;
}

void subtractUnsentFromMask(const char *uFile) {
  if (!SPIFFS.exists(uFile))
    return;
  File f = SPIFFS.open(uFile, FILE_READ);
  if (!f)
    return;

  bool isFtpFile =
      (strstr(uFile, ".kwd") != NULL || strstr(uFile, ".swd") != NULL ||
       strstr(uFile, "ftpunsent") != NULL);

  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() < 10)
      continue;

    int sNum = -1;
    int d_year = current_year, d_month = current_month,
        d_day = current_day; // Default if unparseable

    if (isFtpFile) {
      // FTP format: stnId;YYYY-MM-DD,HH:MM;...
      int semi1 = line.indexOf(';');
      if (semi1 == -1)
        continue;
      int semi2 = line.indexOf(';', semi1 + 1);
      if (semi2 == -1)
        continue;
      String dateTime = line.substring(semi1 + 1, semi2); // "YYYY-MM-DD,HH:MM"
      if (dateTime.length() >= 10) {
        d_year = dateTime.substring(0, 4).toInt();
        d_month = dateTime.substring(5, 7).toInt();
        d_day = dateTime.substring(8, 10).toInt();
      }
      int commaIdx = dateTime.indexOf(',');
      if (commaIdx == -1)
        continue;
      String timeStr = dateTime.substring(commaIdx + 1); // "HH:MM"
      int colonIdx = timeStr.indexOf(':');
      if (colonIdx == -1)
        continue;
      int h = timeStr.substring(0, colonIdx).toInt();
      int m = timeStr.substring(colonIdx + 1).toInt();
      int raw = h * 4 + m / 15;
      sNum = (raw + 61) % 96;
    } else {
      // CSV format: sampleNo,YYYY-MM-DD,... OR format:
      // sampleNo,YYYY-MM-DD,HH:MM,...
      int commaIdx = line.indexOf(',');
      if (commaIdx == -1)
        continue;
      sNum = line.substring(0, commaIdx).toInt();
      int comma2 = line.indexOf(',', commaIdx + 1);
      if (comma2 != -1) {
        String sDate = line.substring(commaIdx + 1, comma2);
        if (sDate.length() >= 10) {
          d_year = sDate.substring(0, 4).toInt();
          d_month = sDate.substring(5, 7).toInt();
          d_day = sDate.substring(8, 10).toInt();
        }
      }
    }

    if (sNum >= 0 && sNum <= 95) {
      // Calculate 'NOW' meteorological close date
      int cur_dd = current_day, cur_mm = current_month, cur_yy = current_year;
      int curr_h = (current_hour < 24) ? current_hour : 0;
      int curr_m = (current_min < 60) ? current_min : 0;
      int curr_sNum = (curr_h * 4 + curr_m / 15 + 61) % 96;
      if (curr_sNum <= 60) {
        next_date(&cur_dd, &cur_mm, &cur_yy);
      }

      // Calculate 'RECORD' meteorological close date
      int rec_dd = d_day, rec_mm = d_month, rec_yy = d_year;
      if (sNum <= 60) {
        next_date(&rec_dd, &rec_mm, &rec_yy);
      }

      bool isToday = (cur_yy == rec_yy && cur_mm == rec_mm && cur_dd == rec_dd);

      // Subtraction Logic: Clear the bit if found in unsent file
      if (isToday) {
        diag_sent_mask_cur[sNum / 32] &= ~(1UL << (sNum % 32));
      } else {
        diag_sent_mask_prev[sNum / 32] &= ~(1UL << (sNum % 32));
      }
    }
  }
  f.close();
}

void reconstructSentMasks() {
  if (backfill_done)
    return;

  // v7.80: Allow recovery on Deep Sleep if counters are zero (RTC Wiped)
  if (rtc_get_reset_reason(0) == DEEPSLEEP_RESET && diag_pd_count > 0)
    return;

  debugln("[GoldenData] Starting Sent Mask Reconstruction from SPIFFS...");

  char cleanStn[16];
  strncpy(cleanStn, station_name, 15);
  cleanStn[15] = '\0';
  int slen = strlen(cleanStn);
  while (slen > 0 && cleanStn[slen - 1] == ' ') {
    cleanStn[slen - 1] = '\0';
    slen--;
  }

  // 1. Mark everything in full log files as 'Sent' initially
  char prevFile[32];
  char curFile[32];

  int cur_dd = current_day, cur_mm = current_month, cur_yy = current_year;
  int prev_dd = current_day, prev_mm = current_month, prev_yy = current_year;

  int h = (current_hour < 24) ? current_hour : 0;
  int m = (current_min < 60) ? current_min : 0;
  int sampleNo = h * 4 + m / 15;
  sampleNo = (sampleNo + 61) % 96;

  if (sampleNo <= 60) {
    // Time is 08:45 AM or later:
    // Current close date is Tomorrow. Previous close date is Today.
    next_date(&cur_dd, &cur_mm, &cur_yy);
  } else {
    // Time is 00:00 to 08:30 AM:
    // Current close date is Today. Previous close date is Yesterday.
    previous_date(&prev_dd, &prev_mm, &prev_yy);
  }

  snprintf(prevFile, sizeof(prevFile), "/%s_%04d%02d%02d.txt", cleanStn,
           prev_yy, prev_mm, prev_dd);
  if (!SPIFFS.exists(prevFile)) {
    // Try legacy format with raw station_name (might have trailing spaces)
    snprintf(prevFile, sizeof(prevFile), "/%s_%04d%02d%02d.txt", station_name,
             prev_yy, prev_mm, prev_dd);
  }
  scanFileToMask(prevFile, diag_sent_mask_prev);
  diag_pd_count_prev = countStored(prevFile);
  diag_ndm_count_prev = countNightStored(prevFile);

  snprintf(curFile, sizeof(curFile), "/%s_%04d%02d%02d.txt", cleanStn, cur_yy,
           cur_mm, cur_dd);
  if (!SPIFFS.exists(curFile)) {
    snprintf(curFile, sizeof(curFile), "/%s_%04d%02d%02d.txt", station_name,
             cur_yy, cur_mm, cur_dd);
  }
  scanFileToMask(curFile, diag_sent_mask_cur);
  diag_pd_count = countStored(curFile);
  diag_ndm_count = countNightStored(curFile);
  new_current_cumRF = restoreRainfall(curFile);

  // 2. Subtract records that are currently in 'Unsent' files
  subtractUnsentFromMask("/unsent.txt");
  subtractUnsentFromMask("/ftpunsent.txt");

  // 3. Restore Sent Counters from the reconstructed Masks
  int cur_sent = 0;
  int prev_sent = 0;
  for (int i = 0; i < 96; i++) {
    if (diag_sent_mask_cur[i / 32] & (1UL << (i % 32)))
      cur_sent++;
    if (diag_sent_mask_prev[i / 32] & (1UL << (i % 32)))
      prev_sent++;
  }
  diag_http_success_count = cur_sent;
  diag_http_success_count_prev = prev_sent;
  diag_first_http_count =
      cur_sent; // v7.86 Reconstruct from mask so values make sense after boot
  diag_first_http_count_prev = prev_sent;

  backfill_done = true;
  debugln("[GoldenData] Reconstruction Complete (Sent-Accurate).");
}

// v5.48 reconciles delivered data after successful FTP upload
// v7.53: Fixed to handle both CSV records (sampleNo,DATE...) and FTP records
// (stnId;DATE,TIME;...) with correct sampleNo back-calculation.
void markFileAsDelivered(const char *fileName) {
  if (!SPIFFS.exists(fileName))
    return;
  File f = SPIFFS.open(fileName, FILE_READ);
  if (!f)
    return;
  // Detect format from extension: .kwd/.swd = FTP (semicolon), else CSV
  bool isFtpFile =
      (strstr(fileName, ".kwd") != NULL || strstr(fileName, ".swd") != NULL ||
       strstr(fileName, "ftpunsent") != NULL);

  while (f.available()) {
    esp_task_wdt_reset(); // v7.67: Prevent WDT on huge backlog files
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() < 10)
      continue;

    int sNum = -1;
    int d_year = current_year, d_month = current_month,
        d_day = current_day; // Default if unparseable

    if (isFtpFile) {
      // FTP format: stnId;YYYY-MM-DD,HH:MM;...
      // Extract sampleNo by back-calculating from the time field
      int semi1 = line.indexOf(';');
      if (semi1 == -1)
        continue;
      int semi2 = line.indexOf(';', semi1 + 1);
      if (semi2 == -1)
        continue;
      String dateTime = line.substring(semi1 + 1, semi2); // "YYYY-MM-DD,HH:MM"
      if (dateTime.length() >= 10) {
        d_year = dateTime.substring(0, 4).toInt();
        d_month = dateTime.substring(5, 7).toInt();
        d_day = dateTime.substring(8, 10).toInt();
      }
      int commaIdx = dateTime.indexOf(',');
      if (commaIdx == -1)
        continue;
      String timeStr = dateTime.substring(commaIdx + 1); // "HH:MM"
      int colonIdx = timeStr.indexOf(':');
      if (colonIdx == -1)
        continue;
      int h = timeStr.substring(0, colonIdx).toInt();
      int m = timeStr.substring(colonIdx + 1).toInt();
      // Convert time to meteorological day sample index (same formula as
      // firmware)
      int raw = h * 4 + m / 15;
      sNum = (raw + 61) % 96;
    } else {
      // CSV format: sampleNo,YYYY-MM-DD,HH:MM,...
      int commaIdx = line.indexOf(',');
      if (commaIdx == -1)
        continue;
      sNum = line.substring(0, commaIdx).toInt();
      int comma2 = line.indexOf(',', commaIdx + 1);
      if (comma2 != -1) {
        String sDate = line.substring(commaIdx + 1, comma2);
        if (sDate.length() >= 10) {
          d_year = sDate.substring(0, 4).toInt();
          d_month = sDate.substring(5, 7).toInt();
          d_day = sDate.substring(8, 10).toInt();
        }
      }
    }

    if (sNum >= 0 && sNum <= 95) {
      // v5.49 Build 5: STRICT DATE VERIFICATION
      // Calculate 'NOW' meteorological close date
      int cur_dd = current_day, cur_mm = current_month, cur_yy = current_year;
      int curr_h = (current_hour < 24) ? current_hour : 0;
      int curr_m = (current_min < 60) ? current_min : 0;
      int curr_sNum = (curr_h * 4 + curr_m / 15 + 61) % 96;
      if (curr_sNum <= 60) {
        next_date(&cur_dd, &cur_mm, &cur_yy);
      }

      // Calculate 'RECORD' meteorological close date
      int rec_dd = d_day, rec_mm = d_month, rec_yy = d_year;
      if (sNum <= 60) {
        next_date(&rec_dd, &rec_mm, &rec_yy);
      }

      bool isToday = (cur_yy == rec_yy && cur_mm == rec_mm && cur_dd == rec_dd);

      if (isToday) {
        if (!(diag_sent_mask_cur[sNum / 32] & (1UL << (sNum % 32)))) {
          diag_sent_mask_cur[sNum / 32] |= (1UL << (sNum % 32));
          if (diag_ftp_success_count < 96)
            diag_ftp_success_count++;
        }
      } else {
        if (!(diag_sent_mask_prev[sNum / 32] & (1UL << (sNum % 32)))) {
          diag_sent_mask_prev[sNum / 32] |= (1UL << (sNum % 32));
          if (diag_ftp_success_count_prev < 96)
            diag_ftp_success_count_prev++;
        }
      }
    }
  }
  f.close();
  debugf1("[GoldenData] Marked records from %s as DELIVERED.\n", fileName);
}
void reset_all_diagnostics() {
  debugln("[SYS] Resetting all diagnostic counters...");

  // Primary counters
  diag_net_data_count = 0;
  diag_net_data_count_prev = 0;
  diag_pd_count = 0;
  diag_pd_count_prev = 0;
  diag_ndm_count = 0;
  diag_ndm_count_prev = 0;
  diag_first_http_count = 0;
  diag_first_http_count_prev = 0;

  // HTTP Metrics
  diag_consecutive_http_fails = 0;
  diag_daily_http_fails = 0;
  diag_http_present_fails = 0;
  diag_http_cum_fails = 0;
  diag_http_time_total = 0;
  diag_http_success_count = 0;
  diag_http_success_count_prev = 0;
  diag_http_retry_count = 0;
  diag_http_retry_count_prev = 0;
  diag_ftp_success_count = 0;
  diag_ftp_success_count_prev = 0;

  // Registration metrics
  diag_consecutive_reg_fails = 0;
  diag_consecutive_sim_fails = 0;
  diag_reg_count = 0;
  diag_reg_time_total = 0;
  diag_reg_worst = 0;
  diag_modem_mutex_fails = 0; // v5.55
  diag_backlog_total = 0;     // v5.55
  strcpy(diag_reg_fail_type, "NONE");

  // Pulse / Accuracy counters (v5.49)
  total_wind_pulses_32 = 0;
  last_sched_wind_pulses_32 = 0;
  last_raw_wind_count = 0;
  total_rf_pulses_32 = 0;
  last_sched_rf_pulses_32 = 0;
  last_raw_rf_count = 0;
  diag_last_rf_val = 0;

  // Sensor diagnostic flags
  diag_ws_same_count = 0;
  temp_same_count = 0;
  hum_same_count = 0;
  prev_15min_temp = INITIAL_PREV_TEMP;
  prev_15min_hum = INITIAL_PREV_HUM;
  prev_15min_ws = 0.0;
  
  diag_temp_cv = false;
  diag_hum_cv = false;
  diag_ws_cv = false;
  diag_temp_erv = false;
  diag_hum_erv = false;
  diag_ws_erv = false;
  diag_temp_erz = false;
  diag_hum_erz = false;
  diag_rain_jump = false;
  diag_rain_calc_invalid = false;
  diag_rain_reset = false;

  // Status flags
  strcpy(diag_cdm_status, "PENDING");
  strcpy(diag_http_fail_reason, "NONE");
  diag_rejected_count = 0;
  diag_sensor_fault_sent_today = false;

  // Masks
  for (int i = 0; i < 3; i++) {
    diag_sent_mask_cur[i] = 0;
    diag_sent_mask_prev[i] = 0;
  }

  // Force a fresh Daily/Health cycle
  health_last_sent_day = -1;
  health_last_sent_hour = -1;
  diag_last_rollover_day = -1;
  diag_http_success_count = 0;
  diag_http_retry_count = 0;
  diag_ftp_success_count = 0;
  diag_http_present_fails = 0;
  diag_http_cum_fails = 0;
  diag_cum_fail_reset_month = -1;

  debugln("[SYS] Diagnostics cleaned.");
}

int get_total_backlogs() {
  static unsigned long last_backlog_check = 0;
  // If we checked in the last 10 seconds, return the cached value
  if (last_backlog_check > 0 && (millis() - last_backlog_check < 10000)) {
    return diag_backlog_total;
  }

  int total = 0;
  const char *files[] = {"/unsent.txt", "/ftpunsent.txt"};
  for (int i = 0; i < 2; i++) {
    if (SPIFFS.exists(files[i])) {
      File f = SPIFFS.open(files[i], FILE_READ);
      if (f) {
        while (f.available()) {
          String line = f.readStringUntil('\n');
          line.trim();
          if (line.length() > 10) {
            total++;
          }
          if (total % 50 == 0) esp_task_wdt_reset(); // Keep watchdog happy
        }
        f.close();
      }
    }
  }
  diag_backlog_total = total;
  last_backlog_check = millis();
  return total;
}
