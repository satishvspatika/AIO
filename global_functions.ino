#include "globals.h"
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
  #include "esp_adc/adc_cali_scheme.h"
#else
  #include "esp_adc_cal.h"
#endif

void start_deep_sleep() {
  // v5.70: Deep Sleep Guard (Issue 3) - Abort sleep if critical tasks are mid-flight
  // v5.75: Added schedulerBusy to guard (M-02 fix)
  if (health_in_progress || ota_writing_active || gprs_started || schedulerBusy) {
    debugln("[PWR] Critical Activity (Health/OTA/Modem/Scheduler) in progress. Deferring sleep.");
    return;
  }

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
        // Do NOT cut GPIO 32 power here! It causes SDA/SCL ESD diode glitches
        xSemaphoreGive(i2cMutex);
      }
    }
  }

  // CRITICAL: Ensure ULP counts into the main RF bucket during sleep!
  calib_mode_flag.val = 0;
  calib_flag = 0; // Reset UI state too

  WiFi.disconnect();
  // LCD power cut deferred to end of function to protect I2C bus

  // Ensure GPRS is shut down gracefully ALWAYS before cutting power
  esp_task_wdt_reset();
  graceful_modem_shutdown();
  esp_task_wdt_reset();
  delay(100);

  // SELF-HEALING: Force GPIO 26 and 32 to stay strictly LOW during sleep
  // v5.79 Fix: digitalWrite LOW must happen BEFORE gpio_hold_en()
  
  // TIER 3: SLEEP TIMER EARLY-WAKE DRIFT
  // Eliminate up to 90 seconds of RTOS polling staleness by reading the hardware directly.
  int live_min = current_min;
  int live_sec = current_sec; // fallback
  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(500)) == pdTRUE) {
      DateTime now = rtc.now();
      
      portENTER_CRITICAL(&rtcTimeMux);
      live_min = now.minute();
      current_min = live_min; // Update globals for accurate calc
      portEXIT_CRITICAL(&rtcTimeMux);
      
      live_sec = now.second();
      
      // Shut down I2C bus BEFORE cutting LCD power
      // Prevents PCF8574 ESD diodes from corrupting in-flight RTC/HDC transactions
      Wire.end();                  // Release SDA/SCL before PCF8574 loses power
      digitalWrite(32, LOW);       // Now safe to cut 5V — bus is idle
      
      // Hardware Hold: Capture the LOW state for the duration of the sleep
      gpio_hold_en(GPIO_NUM_32);
      gpio_hold_en(GPIO_NUM_26); // Modem was cut in graceful_modem_shutdown()
      gpio_hold_en(GPIO_NUM_16); // v5.70: Fix H-02 ESD diode bleed through UART pins
      gpio_hold_en(GPIO_NUM_17);
      gpio_deep_sleep_hold_en();
      
      // Phase 7 Fix: Don't intentionally leak the mutex into deep sleep.
      // We will suspend FreeRTOS instead to prevent late I2C attempts.
      xSemaphoreGive(i2cMutex);
  } else {
      digitalWrite(32, LOW);       // Fallback cut if mutex totally hung
      gpio_hold_en(GPIO_NUM_32);
      gpio_hold_en(GPIO_NUM_26);
      gpio_hold_en(GPIO_NUM_16);
      gpio_hold_en(GPIO_NUM_17);
      gpio_deep_sleep_hold_en();
  }

  // Calculate time to sleep to target the NEXT exact 15-minute boundary
  // We calculate in SECONDS for precision using the live hardware read
  int next_boundary_min = ((live_min / 15) + 1) * 15;
  int sleep_seconds;

  // Handle hour rollover (e.g., 59 minutes -> next hour at 0 minutes)
  // v7.89: Reverted to +30s offset for maximum stability.
  if (next_boundary_min >= 60) {
    sleep_seconds = (60 - live_min) * 60 - live_sec + 15; // v5.65 fix: reduced offset to 15s
  } else {
    sleep_seconds =
        (next_boundary_min * 60) - (live_min * 60 + live_sec) + 15; // v5.65 fix
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

  // v5.65 P8 Fix: Phantom UI Wakeup Prevention
  // During deep sleep, standard pinMode(INPUT_PULLUP) is deactivated.
  // This causes the push button pin (GPIO 27) to float, making it highly susceptible
  // to RF noise/wind static, which randomly triggers ext0 and turns on the LCD mid-cycle.
  // By forcing the RTC pull-up, we lock it HIGH until genuinely pressed.
  rtc_gpio_init(GPIO_NUM_27);
  rtc_gpio_set_direction(GPIO_NUM_27, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pullup_en(GPIO_NUM_27);

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_27, LOW);
  // Use the calculated seconds directly
  esp_sleep_enable_timer_wakeup((uint64_t)sleep_seconds * uS_TO_S_FACTOR);

  // CRITICAL: Restored from Version 3.0. Adjusts ULP startup delay to prevent
  // hang in deep sleep.
  REG_SET_FIELD(RTC_CNTL_TIMER2_REG, RTC_CNTL_ULPCP_TOUCH_START_WAIT, 0x20);

  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

  // Phase 7 Fix: Shut down SPI completely to eliminate 4mA SD Card Idle Bleed
  if (sd_card_ok) {
    SD.end();
    SPI.end();
  }

  Serial.flush();
  debugln("[PWR] Entering Deep Sleep");

  esp_deep_sleep_start();
  
  // Phase 9 Fix: Nuclear fallback. esp_deep_sleep_start() should never logically return.
  // If it fails silently, the disconnected RTOS will brick. Reboot immediately!
  ESP.restart();
}

// Validate ULP counters to prevent memory corruption
void validate_ulp_counters() {
  bool corrupted = false;

  // [Task 2.3 Fix]: Removed arbitrary forced-limits (e.g. > 65000) on wind_count and rf_count.
  // ULP counters are increment-only 16-bit values. They naturally overflow from 65535 to 0.
  // The scheduler calculates delta via `(65536 + current - prev)`, perfectly handling one wrap.
  // Forcing them to 0 mid-count artificially destroys the delta, causing massive data drops.
  // We only check calib_count since it's a diagnostic tool, not critical data.

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
  if (!sd_card_ok) return; // Phase 7 Fix: Stop VFS File Handle Leaks
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
    debugln("Failed to open directory for deletion scan");
    return;
  }
  
  // v5.65 P1 Fix: Collect matching filenames into a buffer first. 
  // SPIFFS (and LittleFS) iterators can become invalid or skip entries 
  // if you remove files during an openNextFile() traversal.
  String toDelete[48]; 
  int found = 0;

  File file = root.openNextFile();
  while (file && found < 48) {
    esp_task_wdt_reset();
    String fName = file.name();
    if (fName.indexOf(station) != -1) {
      toDelete[found++] = fName;
    }
    file.close();
    file = root.openNextFile();
  }
  root.close();

  for (int i = 0; i < found; i++) {
    debugf1("[SPIFFS] Deleting matching file: %s\n", toDelete[i].c_str());
    SPIFFS.remove(toDelete[i]);
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
void recoverI2CBus(bool alreadyLocked) {
  if (!alreadyLocked) {
    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(2000)) != pdTRUE) {
      debugln("[I2C] Recovery skipped: Mutex held by another task");
      return;
    }
  }
  // Quick check: If SDA is high, the bus is not hung.
  // v5.66: Prevent false 1-second delays during deep sleep shutdown sequences.
  pinMode(I2C_SDA, INPUT_PULLUP);
  if (digitalRead(I2C_SDA) == HIGH) {
    Wire.begin(I2C_SDA, I2C_SCL, 100000); // Re-attach peripheral peripheral pins
    Wire.setTimeOut(I2C_TIMEOUT_MS);
    if (!alreadyLocked) xSemaphoreGive(i2cMutex);
    return;
  }

  debugln("[I2C] 🚨 Bus hang detected! Attempting recovery...");

  // End any existing Wire session
  Wire.end();

  // Switch SCL to output to drive clock pulses
  pinMode(I2C_SCL, OUTPUT);
  digitalWrite(I2C_SCL, HIGH);

  // v5.59 Power Optimization: Reduced blind delay from 5000ms to 1000ms.
  // The subsequent hardware initializations (Wire/SPI) act as surgical delays.
  vTaskDelay(1000 / portTICK_PERIOD_MS);

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
  
  if (!alreadyLocked) {
    xSemaphoreGive(i2cMutex);
  }
}


/**
 * Persists current coordinates and timestamp (v5.49) to SPIFFS
 */
void saveGPS() {
  if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
    debugln("[SPIFFS] Mutex Timeout: Skipping saveGPS");
    return;
  }
  File file = SPIFFS.open("/gps_fix.tmp", FILE_WRITE);
  if (file) {
    file.printf("%.6f,%.6f,%d,%d,%d", lati, longi, current_day, current_month,
                current_year);
    file.close();
    if (SPIFFS.exists("/gps_fix.txt")) {
        SPIFFS.remove("/gps_fix.txt");
    }
    if (SPIFFS.rename("/gps_fix.tmp", "/gps_fix.txt")) {
        debugln("[PWR] GPS Fix persisted successfully.");
    }
    gps_fix_dd = current_day;
    gps_fix_mm = current_month;
    gps_fix_yy = current_year;
    debugln("[GPS] Location persisted ATOMICALLY to SPIFFS");
  }
  xSemaphoreGive(fsMutex);
}

/**
 * Loads last known coordinates and timestamp (v5.49) from SPIFFS
 */
void loadGPS() {
  if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
    debugln("[SPIFFS] Mutex Timeout: Skipping loadGPS");
    return;
  }
  if (!SPIFFS.exists("/gps_fix.txt") && SPIFFS.exists("/gps_fix.tmp")) {
    debugln("[GPS] txt missing but tmp found. Recovering...");
    SPIFFS.rename("/gps_fix.tmp", "/gps_fix.txt");
  }

  if (SPIFFS.exists("/gps_fix.txt")) {
    File file = SPIFFS.open("/gps_fix.txt", FILE_READ);
    if (file) {
      String line = file.readString();
      file.close();
      if (sscanf(line.c_str(), "%lf,%lf,%d,%d,%d", &lati, &longi, &gps_fix_dd,
                 &gps_fix_mm, &gps_fix_yy) == 5) {
        debugf5("[GPS] Loaded from SPIFFS: %.6f,%.6f (Fix: %02d/%02d/%d)\n",
                lati, longi, gps_fix_dd, gps_fix_mm, gps_fix_yy);
      }
    }
  } else {
    debugln("[GPS] No persisted location found.");
  }
  xSemaphoreGive(fsMutex);
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

    char *saveptr = NULL;
    char *token = strtok_r(p, ",", &saveptr);
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
      token = strtok_r(NULL, ",", &saveptr);
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

  // v5.79: Torn Time Read Fix
  struct tm snapshot;
  getTimeSnapshot(&snapshot);
  int current_s_idx = snapshot.tm_hour * 4 + snapshot.tm_min / 15;
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
  char buf[128];
  while (f.available()) {
    int l = f.readBytesUntil('\n', buf, sizeof(buf) - 1);
    buf[l] = '\0';
    if (l < 5)
      continue;

    char *commaPtr = strchr(buf, ',');
    if (commaPtr != NULL) {
      int sNum = -1;
      int sigVal = 0;
      
      // Basic CSV parse to find SampleNo and SignalStrength using strtok_r
      char *token;
      char *saveptr_scan = NULL;
      token = strtok_r(buf, ",", &saveptr_scan);
      if (token) sNum = atoi(token);
      
      // ISSUE-H4 fix v5.65: Signal strength field position differs per SYSTEM type.
      // SYSTEM 0 (RF):     sampleNo,date,time,instRF,cumRF,signal,bat     → field 5
      // SYSTEM 1 (TWS):    sampleNo,date,time,temp,hum,ws,wd,signal,bat   → field 7
      // SYSTEM 2 (TWS-RF): sampleNo,date,time,cumRF,temp,hum,ws,wd,signal,bat → field 8
      // Previously hardcoded field 8 for all systems, causing SYSTEM 0 and 1
      // to read bat_val (e.g. 4) instead of signal (-111), silently marking
      // gap-filled records as "sent" and inflating the dashboard count.
#if SYSTEM == 0
      const int sig_field = 5;
#elif SYSTEM == 1
      const int sig_field = 7;
#else
      const int sig_field = 8;
#endif
      for (int i = 0; i < sig_field && token != NULL; i++) {
        token = strtok_r(NULL, ",", &saveptr_scan);
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
  char line_buf[128];
  while (f.available()) {
    // v7.95: Keep WDT alive during file scan
    esp_task_wdt_reset();
    int l = f.readBytesUntil('\n', line_buf, sizeof(line_buf) - 1);
    line_buf[l] = '\0';
    
    // v5.79: Optimized char search (No heap String)
    if (l >= 10 && (strchr(line_buf, ',') || strchr(line_buf, ';'))) {
      count++;
      if (count >= 120) // v7.95: Increased cap to 120 for safety
        break; 
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
    // P0-A fix v5.65: Field layout differs per SYSTEM type.
    // SYSTEM 0 (RF):     sampleNo,date,time,inst_rf,CUM_RF,signal,bat
    //   → cum_rf is at field index 4 (between 4th and 5th comma)
    // SYSTEM 2 (TWS-RF): sampleNo,date,time,CUM_RF,temp,hum,ws,wd,signal,bat
    //   → cum_rf is at field index 3 (between 3rd and 4th comma)
    // SYSTEM 1 (TWS): no rainfall — returns 0.0 safely via empty substring.
    // Previously ALL systems used field 3. On SYSTEM 0, field 3 = inst_rf,
    // so after a power cut the anchor was restored as e.g. 0.25mm not 12.75mm.
    int firstComma = lastLine.indexOf(',');
    if (firstComma == -1)
      return 0.0;
    int secondComma = lastLine.indexOf(',', firstComma + 1);
    if (secondComma == -1)
      return 0.0;
    int thirdComma = lastLine.indexOf(',', secondComma + 1);
    if (thirdComma == -1)
      return 0.0;

#if SYSTEM == 0
    // For TRG: skip one more comma to land on cum_rf (field 4)
    int fourthComma = lastLine.indexOf(',', thirdComma + 1);
    if (fourthComma == -1)
      return 0.0;
    int fifthComma = lastLine.indexOf(',', fourthComma + 1);
    String rfStr = lastLine.substring(fourthComma + 1, fifthComma);
#else
    // For SYSTEM 2 and SYSTEM 1: cum_rf (or placeholder) is at field 3
    int fourthComma = lastLine.indexOf(',', thirdComma + 1);
    String rfStr = lastLine.substring(thirdComma + 1, fourthComma);
#endif
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
  char line_buf[128];
  while (f.available()) {
    esp_task_wdt_reset(); // Keep watchdog happy during scan
    int l = f.readBytesUntil('\n', line_buf, sizeof(line_buf) - 1);
    line_buf[l] = '\0';
    if (l >= 10) {
      char *commaPtr = strchr(line_buf, ',');
      if (commaPtr != NULL) {
        int sNum = atoi(line_buf);
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

  char line_buf[128];
  while (f.available()) {
    esp_task_wdt_reset(); // v7.80: Handle massive backlog files without timeout
    vTaskDelay(pdMS_TO_TICKS(1)); // Phase 7 Fix: Native RTOS 1ms yield
    long pos = f.position(); // Save position of the FIRST byte of the line
    // v5.80: Final heap optimization - direct buffer parsing
    int l = f.readBytesUntil('\n', line_buf, sizeof(line_buf) - 1);
    line_buf[l] = '\0';
    
#if SYSTEM == 0
    if (strstr(uFile, "unsent.txt") && !strstr(uFile, "ftpunsent.txt")) {
        if (pos < (long)unsent_pointer_count) continue;
    }
#endif

    if (l < 10) continue;

    int sNum = -1;
    int d_year = current_year, d_month = current_month, d_day = current_day;

    if (isFtpFile) {
      // FTP format: stnId;YYYY-MM-DD,HH:MM;...
      char *semi1 = strchr(line_buf, ';');
      if (semi1 == NULL) continue;
      char *semi2 = strchr(semi1 + 1, ';');
      if (semi2 == NULL) continue;
      
      *semi2 = '\0'; // Null terminate at second semicolon
      char *dateTime = semi1 + 1;
      if (strlen(dateTime) >= 10) {
          d_year = atoi(dateTime);
          d_month = atoi(dateTime + 5);
          d_day = atoi(dateTime + 8);
          
          char *comma = strchr(dateTime, ',');
          if (comma) {
              int h = atoi(comma + 1);
              char *colon = strchr(comma + 1, ':');
              int m = colon ? atoi(colon + 1) : 0;
              int raw = h * 4 + m / 15;
              sNum = (raw + 61) % 96;
          }
      }
    } else {
      // CSV format: sampleNo,YYYY-MM-DD,...
      sNum = atoi(line_buf);
      char *comma1 = strchr(line_buf, ',');
      if (comma1) {
          d_year = atoi(comma1 + 1);
          d_month = atoi(comma1 + 6);
          d_day = atoi(comma1 + 9);
      }
    }

    if (sNum >= 0 && sNum <= 95) {
      // Calculate 'NOW' meteorological close date
      int cur_dd, cur_mm, cur_yy, curr_h, curr_m;
      portENTER_CRITICAL(&rtcTimeMux);
      cur_dd = current_day;
      cur_mm = current_month;
      cur_yy = current_year;
      curr_h = (current_hour < 24) ? current_hour : 0;
      curr_m = (current_min < 60) ? current_min : 0;
      portEXIT_CRITICAL(&rtcTimeMux);

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

void reconstructSentMasks(bool alreadyLocked) {
  if (!alreadyLocked) {
    if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
      debugln("[SPIFFS] Mutex Timeout: Skipping reconstructSentMasks");
      return;
    }
  }
  if (backfill_done) {
    if (!alreadyLocked)
      xSemaphoreGive(fsMutex);
    return;
  }

  // v7.80: Allow recovery on Deep Sleep if counters are zero (RTC Wiped)
  if (rtc_get_reset_reason(0) == DEEPSLEEP_RESET && diag_pd_count > 0) {
    if (!alreadyLocked)
      xSemaphoreGive(fsMutex);
    return;
  }

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

  // v5.79: Torn Time Read Fix
  struct tm snapshot;
  getTimeSnapshot(&snapshot);
  int cur_dd = snapshot.tm_mday, cur_mm = snapshot.tm_mon + 1, cur_yy = snapshot.tm_year + 1900;
  int prev_dd = cur_dd, prev_mm = cur_mm, prev_yy = cur_yy;

  int h = snapshot.tm_hour;
  int m = snapshot.tm_min;
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
  if (!alreadyLocked)
    xSemaphoreGive(fsMutex);
}

// v5.48 reconciles delivered data after successful FTP upload
// v7.53: Fixed to handle both CSV records (sampleNo,DATE...) and FTP records
// (stnId;DATE,TIME;...) with correct sampleNo back-calculation.
void markFileAsDelivered(const char *fileName, bool alreadyLocked) {
  if (!alreadyLocked) {
    if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
      debugln("[SPIFFS] Mutex Timeout: Skipping markFileAsDelivered");
      return;
    }
  }
  if (!SPIFFS.exists(fileName)) {
    if (!alreadyLocked) xSemaphoreGive(fsMutex);
    return;
  }
  File f = SPIFFS.open(fileName, FILE_READ);
  if (!f) {
    if (!alreadyLocked) xSemaphoreGive(fsMutex);
    return;
  }
  // Detect format from extension: .kwd/.swd = FTP (semicolon), else CSV
  bool isFtpFile =
      (strstr(fileName, ".kwd") != NULL || strstr(fileName, ".swd") != NULL ||
       strstr(fileName, "ftpunsent") != NULL);

  while (f.available()) {
    esp_task_wdt_reset(); // v7.67: Prevent WDT on huge backlog files
    vTaskDelay(pdMS_TO_TICKS(1)); // Phase 7 Fix: Native RTOS yield
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() < 10)
      continue;

    int sNum = -1;
    int d_year = current_year, d_month = current_month,
        d_day = current_day; // Default if unparseable

    if (isFtpFile) {
      // v5.65 P5 Fix: Robust FTP Parser (Handles both Date,Time and Date;Time)
      // Format: stnId;YYYY-MM-DD;HH:MM;... OR stnId;YYYY-MM-DD,HH:MM;...
      int semi1 = line.indexOf(';');
      if (semi1 == -1) continue;
      
      // Date starts at semi1 + 1
      String sDate = line.substring(semi1 + 1, semi1 + 11); // "YYYY-MM-DD"
      if (sDate.length() == 10 && sDate.indexOf('-') != -1) {
        d_year = sDate.substring(0, 4).toInt();
        d_month = sDate.substring(5, 7).toInt();
        d_day = sDate.substring(8, 10).toInt();
      } else {
        continue;
      }

      // Find Time: It follows the date after 1 char (separator)
      char sep = line.charAt(semi1 + 11);
      if (sep == ',' || sep == ';') {
          String timeStr = line.substring(semi1 + 12, semi1 + 17); // "HH:MM"
          int colonIdx = timeStr.indexOf(':');
          if (colonIdx != -1) {
              int h = timeStr.substring(0, colonIdx).toInt();
              int m = timeStr.substring(colonIdx + 1).toInt();
              int raw = h * 4 + m / 15;
              sNum = (raw + 61) % 96;
          }
      }
      
      if (sNum == -1) continue; // Final safety
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
          if (diag_ftp_success_count < 96) diag_ftp_success_count++;
          if (diag_net_data_count < 96) diag_net_data_count++;   // v5.65 P5: Update Live count
        }
      } else {
        if (!(diag_sent_mask_prev[sNum / 32] & (1UL << (sNum % 32)))) {
          diag_sent_mask_prev[sNum / 32] |= (1UL << (sNum % 32));
          if (diag_ftp_success_count_prev < 96) diag_ftp_success_count_prev++;
          if (diag_net_data_count_prev < 96) diag_net_data_count_prev++;    // v5.65 P5
        }
      }
    }
  }
  f.close();
  debugf1("[GoldenData] Marked records from %s as DELIVERED.\n", fileName);
  if (!alreadyLocked) xSemaphoreGive(fsMutex);
}
void reset_all_diagnostics() {
  debugln("[SYS] Resetting all diagnostic counters...");

  backfill_done = false; // Phase 7 Fix: Prevent permanent backfill amnesia after factory wipe.
  
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

int get_total_backlogs(bool force = false) {
  static unsigned long last_backlog_check = 0;
  // If we checked in the last 10 seconds, return the cached value
  if (!force && last_backlog_check > 0 && (millis() - last_backlog_check < 10000)) {
    return diag_backlog_total;
  }

  if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) != pdTRUE) {
    return diag_backlog_total; // Return cached value on contention
  }

  int total = 0;
  const char *files[] = {"/unsent.txt", "/ftpunsent.txt"};
  for (int i = 0; i < 2; i++) {
    // v5.76: Atomic Open (R-02 fix)
    File f = SPIFFS.open(files[i], FILE_READ);
    if (f) {
      // v5.68 BUGFIX: Accurate UI Backlog Count
      if (i == 0) {
        File ptrFile = SPIFFS.open("/unsent_pointer.txt", FILE_READ);
        if (ptrFile) {
          String pStr = ptrFile.readString();
          long ptrVal = pStr.toInt();
          if (ptrVal > 0 && ptrVal < (long)f.size()) {
            f.seek(ptrVal);
          }
          ptrFile.close();
        }
      }

        // v5.67 UI FREEZE FIX: Switch from slow String allocation to raw buffer read
        char tbuf[128];
        while (f.available()) {
          int len = f.readBytesUntil('\n', tbuf, sizeof(tbuf) - 1);
          if (len > 10) {
            total++;
          }
          if (total % 100 == 0) esp_task_wdt_reset(); // Keep watchdog happy
        }
        f.close();
      }
    }
  xSemaphoreGive(fsMutex);
  diag_backlog_total = total;
  last_backlog_check = millis();
  return total;
}

float get_calibrated_battery_voltage() {
  static esp_adc_cal_characteristics_t adc_chars;
  static bool initialized = false;
  if (!initialized) {
    // Characterize ADC at 11dB attenuation for 3.3V range
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    initialized = true;
  }
  // ADC1_CHANNEL_5 corresponds to GPIO 33
  uint32_t voltage_mv = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_5), &adc_chars);
  // Apply our custom voltage divider ratio: 840K / 620K
  return ((float)voltage_mv / 1000.0) * (840.0 / 620.0);
}

void pruneFile(const char *path, size_t limit, bool alreadyLocked) {
  if (!alreadyLocked && xSemaphoreTake(fsMutex, pdMS_TO_TICKS(15000)) != pdTRUE) {
    debugln("[SPIFFS] pruneFile Mutex Timeout: skipping.");
    return;
  }

  // v5.70 Hardened: [R-1 Recovery] If original is missing but tmp exists, recover it.
  if (!SPIFFS.exists(path) && SPIFFS.exists("/trim.tmp")) {
      debugf1("[SPIFFS] pruneFile: path %s missing, recovering from /trim.tmp\n", path);
      SPIFFS.rename("/trim.tmp", path);
  }

  if (!SPIFFS.exists(path)) {
    if (!alreadyLocked) xSemaphoreGive(fsMutex);
    return;
  }

  File f = SPIFFS.open(path, FILE_READ);
  if (!f) {
    if (!alreadyLocked) xSemaphoreGive(fsMutex);
    return;
  }

  size_t current_size = f.size();
  if (current_size < limit) {
    f.close();
    if (!alreadyLocked) xSemaphoreGive(fsMutex);
    return;
  }

  debugf2("[SPIFFS] Pruning %s (%d bytes). Limit exceeded.\n", path, (int)current_size);

  // Buffer recent half of max allowed limit
  size_t keep_size = limit / 2;
  if (current_size > keep_size) {
    f.seek(current_size - keep_size);
  } else {
    f.seek(0);
  }
  f.readStringUntil('\n');

  File tmp = SPIFFS.open("/trim.tmp", FILE_WRITE);
  if (tmp) {
    uint8_t buf[512];
    while (f.available()) {
      int n = f.read(buf, sizeof(buf));
      tmp.write(buf, n);
      esp_task_wdt_reset();
    }
    tmp.close();
    f.close();
    SPIFFS.remove(path);
    if (!SPIFFS.rename("/trim.tmp", path)) {
      debugf1("[SPIFFS] ERROR: Rename failed for %s. Retrying...\n", path);
      vTaskDelay(100 / portTICK_PERIOD_MS);
      if (!SPIFFS.rename("/trim.tmp", path)) {
          debugf1("[SPIFFS] FATAL: Rename failed for %s. Data remains in /trim.tmp\n", path);
      }
    }
  } else {
    f.close();
    debugf1("[SPIFFS] Clear: %s to recover space.\n", path);
    SPIFFS.remove(path);
  }

  if (!alreadyLocked) xSemaphoreGive(fsMutex);
}
