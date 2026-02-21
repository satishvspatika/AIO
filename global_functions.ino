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
  if (rf_count.val > 10000 || rf_count.val < 0) {
    debugf1("[ULP] RF count corrupted (%d), resetting\n", rf_count.val);
    rf_count.val = 0;
    corrupted = true;
  }

  // Wind count shouldn't exceed 1 million (unrealistic in any reasonable
  // timeframe)
  if (wind_count.val > 1000000 || wind_count.val < 0) {
    debugf1("[ULP] Wind count corrupted (%d), resetting\n", wind_count.val);
    wind_count.val = 0;
    corrupted = true;
  }

  // Calib count shouldn't exceed 1000
  if (calib_count.val > 1000 || calib_count.val < 0) {
    debugf1("[ULP] Calib count corrupted (%d), resetting\n", calib_count.val);
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
