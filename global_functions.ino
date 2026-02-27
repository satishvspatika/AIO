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
  if (next_boundary_min >= 60) {
    next_boundary_min = 0; // Next hour, 0 minutes
    sleep_seconds = (60 - current_min) * 60 - current_sec;
  } else {
    sleep_seconds = (next_boundary_min * 60) - (current_min * 60 + current_sec);
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

  // Switch SDA/SCL to GPIO output mode
  pinMode(I2C_SDA, OUTPUT);
  pinMode(I2C_SCL, OUTPUT);

  // Digital write both high
  digitalWrite(I2C_SDA, HIGH);
  digitalWrite(I2C_SCL, HIGH);

  // If SDA is stuck low, pulse SCL up to 9 times to "clock out" the hanging
  // data bit
  for (int i = 0; i < 10; i++) {
    if (digitalRead(I2C_SDA) == HIGH) {
      debugf1("[I2C] Bus released after %d pulses\n", i);
      break;
    }
    digitalWrite(I2C_SCL, LOW);
    delayMicroseconds(5);
    digitalWrite(I2C_SCL, HIGH);
    delayMicroseconds(5);
  }

  // Final check
  if (digitalRead(I2C_SDA) == LOW) {
    debugln("[I2C] ❌ Recovery failed: SDA still stuck low!");
  } else {
    debugln("[I2C] ✅ Recovery successful.");
  }

  // Re-initialize Wire
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000); // Standard 100kHz for robustness
}

/**
 * Persists current coordinates to SPIFFS
 */
void saveGPS() {
  File file = SPIFFS.open("/gps_coords.txt", FILE_WRITE);
  if (file) {
    file.printf("%.6f,%.6f", lati, longi);
    file.close();
    debugln("[GPS] Location persisted to SPIFFS");
  }
}

/**
 * Loads last known coordinates from SPIFFS
 */
void loadGPS() {
  if (SPIFFS.exists("/gps_coords.txt")) {
    File file = SPIFFS.open("/gps_coords.txt", FILE_READ);
    if (file) {
      String line = file.readString();
      int commaIndex = line.indexOf(',');
      if (commaIndex != -1) {
        lati = line.substring(0, commaIndex).toFloat();
        longi = line.substring(commaIndex + 1).toFloat();
        debugf2("[GPS] Loaded from SPIFFS: %.6f, %.6f\n", lati, longi);
      }
      file.close();
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
        Serial.printf("[RainCheck] RESET detected @ Sample %d (%.2f < %.2f)\n",
                      sample_no, cum, prev_cum);
#endif
      }

      // Calculation detection: cum = prev_cum + inst (TRG Only)
      if (SYSTEM == 0) {
        if (abs(cum - (prev_cum + inst)) > 0.05) {
          diag_rain_calc_invalid = true;
#if DEBUG == 1
          Serial.printf(
              "[RainCheck] CALC error @ Sample %d: %.2f != %.2f + %.2f\n",
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
        *hasUnresolvedPD = true;
        // Night Data Tracking: 9 PM (Sample 49) to 6 AM (Sample 85)
        if (i >= 49 && i <= 85) {
          *hasUnresolvedNDM = true;
        }
      }
    }
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

#if DEBUG == 1
  Serial.printf("[HealthScan] NetCount: %d | CDM: %s | PD: %s | NDM: %s\n",
                *outNetCount, diag_cdm_status,
                (*hasUnresolvedPD ? "FAIL" : "OK"),
                (*hasUnresolvedNDM ? "FAIL" : "OK"));
#endif
}

// One-time reconstruction of sent masks from SPIFFS files (Session Recovery)
bool backfill_done = false;

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
      int sNum = line.substring(0, commaIdx).toInt();
      if (sNum >= 0 && sNum <= 95) {
        mask[sNum / 32] |= (1UL << (sNum % 32));
      }
    }
  }
  f.close();
}

void subtractUnsentFromMask(const char *uFile) {
  if (!SPIFFS.exists(uFile))
    return;
  File f = SPIFFS.open(uFile, FILE_READ);
  if (!f)
    return;

  // Tomorrow's date (for Current Day file check)
  int t_dd = current_day, t_mm = current_month, t_yy = current_year;
  next_date(&t_dd, &t_mm, &t_yy);

  while (f.available()) {
    String line = f.readStringUntil('\n');
    if (line.length() < 15)
      continue;

    // Format: 04,2026-02-25,09:45,...
    int firstComma = line.indexOf(',');
    int secondComma = line.indexOf(',', firstComma + 1);
    if (firstComma == -1 || secondComma == -1)
      continue;

    int sNum = line.substring(0, firstComma).toInt();
    String sDate = line.substring(firstComma + 1, secondComma);

    char tomStr[16];
    snprintf(tomStr, sizeof(tomStr), "%04d-%02d-%02d", t_yy, t_mm, t_dd);

    if (sNum >= 0 && sNum <= 95) {
      // Subtraction Logic: Clear the bit if found in unsent file
      if (sDate == tomStr ||
          (sDate == String(current_year) + "-" +
                        (current_month < 10 ? "0" : "") +
                        String(current_month) + "-" +
                        (current_day < 10 ? "0" : "") + String(current_day) &&
           sNum <= 60)) {
        // This is part of 'Current' met day if it matches Tomorrow's date
        // OR Today's date with Sample >= 61 (08:45 AM+)
        // Wait, my logic above is slightly inverted.
        // Let's use a simpler heuristic:
        // Current Met Day records have date = Today (if Time >= 08:45) OR
        // Tomorrow (if Time <= 08:30). Yesterday Met Day records have date =
        // Today (if Time <= 08:30) OR Yesterday (if Time >= 08:45).

        // Actually, the most robust way is to check the same logic used in
        // send_at_cmd_data
        int h = (current_hour < 24) ? current_hour : 0;
        int m = (current_min < 60) ? current_min : 0;
        int timeline_idx = h * 4 + m / 15;
        timeline_idx = (timeline_idx + 61) % 96;

        if (sNum <= timeline_idx || timeline_idx < 5) {
          diag_sent_mask_cur[sNum / 32] &= ~(1UL << (sNum % 32));
        } else {
          diag_sent_mask_prev[sNum / 32] &= ~(1UL << (sNum % 32));
        }
      }
    }
  }
  f.close();
}

void reconstructSentMasks() {
  if (backfill_done || rtc_get_reset_reason(0) == DEEPSLEEP_RESET)
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
  snprintf(prevFile, sizeof(prevFile), "/%s_%04d%02d%02d.txt", cleanStn,
           current_year, current_month, current_day);
  scanFileToMask(prevFile, diag_sent_mask_prev);

  char curFile[32];
  int t_dd = current_day, t_mm = current_month, t_yy = current_year;
  next_date(&t_dd, &t_mm, &t_yy);
  snprintf(curFile, sizeof(curFile), "/%s_%04d%02d%02d.txt", cleanStn, t_yy,
           t_mm, t_dd);
  scanFileToMask(curFile, diag_sent_mask_cur);

  // 2. Subtract records that are currently in 'Unsent' files
  subtractUnsentFromMask("/unsent.txt");
  subtractUnsentFromMask("/ftpunsent.txt");

  backfill_done = true;
  debugln("[GoldenData] Reconstruction Complete (Sent-Accurate).");
}

// v5.48 reconciles delivered data after successful FTP upload
void markFileAsDelivered(const char *fileName) {
  if (!SPIFFS.exists(fileName))
    return;
  File f = SPIFFS.open(fileName, FILE_READ);
  if (!f)
    return;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    if (line.length() < 10)
      continue;
    int commaIdx = line.indexOf(',');
    if (commaIdx == -1)
      continue;
    int sNum = line.substring(0, commaIdx).toInt();
    if (sNum >= 0 && sNum <= 95) {
      int current_s_idx = current_hour * 4 + current_min / 15;
      current_s_idx = (current_s_idx + 61) % 96;

      if (sNum <= current_s_idx) {
        diag_sent_mask_cur[sNum / 32] |= (1UL << (sNum % 32));
      } else {
        diag_sent_mask_prev[sNum / 32] |= (1UL << (sNum % 32));
      }
    }
  }
  f.close();
  debugf1("[GoldenData] Marked records from %s as DELIVERED.\n", fileName);
}
