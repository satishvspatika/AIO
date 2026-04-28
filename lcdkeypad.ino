#include "globals.h"

/*
 * LCD - I2C based SCL,SDA,VCC,GND
 * KEYPAD
 */
#define ROW_NUM 2    // four rows
#define COLUMN_NUM 3 // four columns
char keys[ROW_NUM][COLUMN_NUM] = {{'1', '2', '3'}, {'4', '5', '6'}};
char *key_name[7] = {"NOKEY", "CLEAR", "UP", "SET", "LEFT", "DOWN", "RIGHT"};
byte pin_rows[ROW_NUM] = {4, 12};           // R1,R2
byte pin_column[COLUMN_NUM] = {13, 14, 15}; // C1,C2,C3
Keypad keypad =
    Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);
int calib_initial = 0;

// Keypad timing configuration for better responsiveness
void configure_keypad() {
  keypad.setDebounceTime(50); // 50ms debounce - prevents false triggers
  keypad.setHoldTime(500);    // 500ms hold time
}

/*
 * UI related
 */
extern int cur_fld_no, cur_mode;
int cur_char_no, cur_pos_no, disp_fld_no, cursor_type;

char inpCharSet[2][49] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ",
                          "0123456789./-,: "};
char key, input_buf[17];

// Returns true if the field should be shown for the current SYSTEM
bool isFieldVisible(int fld_id) {
#if ENABLE_WEBSERVER == 0
  if (fld_id == FLD_WIFI_ENABLE)
    return false;
#endif

#if ENABLE_HEALTH_REPORT == 0
  if (fld_id == FLD_SEND_HEALTH)
    return false;
#endif

#if ENABLE_PRESSURE_SENSOR == 1
  if (fld_id == FLD_PRESSURE || fld_id == FLD_ALTITUDE) {
    if (SYSTEM == 1 && strstr(UNIT, "KSNDMC_TWS-AP"))
      return true;
    return false;
  }
#else
  if (fld_id == FLD_PRESSURE || fld_id == FLD_ALTITUDE)
    return false;
#endif

#if SYSTEM == 0 // TRG ONLY
  if (fld_id == FLD_WIND_DIR || fld_id == FLD_INST_WS || fld_id == FLD_AVG_WS ||
      fld_id == FLD_TEMP || fld_id == FLD_HUMIDITY || fld_id == FLD_PRESSURE ||
      fld_id == FLD_ALTITUDE)
    return false;
#elif SYSTEM == 1 // TWS ONLY (Weather)
  if (fld_id == FLD_RF_ML || fld_id == FLD_RF_CALIB || fld_id == FLD_RF_RES)
    return false;
  // #1: Hide altitude field if BME280 is not installed
  if (fld_id == FLD_ALTITUDE && bmeType == BME_UNKNOWN)
    return false;
#elif SYSTEM == 2 // TWS-RF
  // #1: Hide altitude field if BME280 is not installed
  if (fld_id == FLD_ALTITUDE && bmeType == BME_UNKNOWN)
    return false;
#endif
  return true;
}

// Define timer variables
hw_timer_t *calib_timer = NULL;
hw_timer_t *lcd_timer = NULL;

volatile bool timerFlag = false;
unsigned long calib_start_time = 0;

// CALIB Timer ISR callback function
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux1);
  timerFlag = true; // Set the flag when the timer interrupts
  portEXIT_CRITICAL_ISR(&timerMux1);
}

volatile bool lcd_power_cut_pending = false;

// LCD Timer ISR callback function
void IRAM_ATTR lcdTimer() {
  portENTER_CRITICAL_ISR(&timerMux2);
  lcdkeypad_start = 0; // Set the flag when the timer interrupts
  // Phase 7 Fix: Defer physical 5V rail cut to the thread context to avoid corrupting I2C mid-transaction
  lcd_power_cut_pending = true;
  portEXIT_CRITICAL_ISR(&timerMux2);
}



// v5.60: Central drawing function - differential to eliminate flicker
void draw_current_page() {
  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) == pdTRUE) {
    if (cur_mode == eEditOff) {
      lcd.noBlink();
      char line0[17], line1[17];
      memset(line0, ' ', 16); line0[16] = '\0';
      memset(line1, ' ', 16); line1[16] = '\0';

      // Special handling for legacy fields with dynamic logic
      if (cur_fld_no == FLD_WIFI_ENABLE) {
        strcpy(line0, wifi_active ? "DISABLE WIFI    " : "ENABLE WIFI     ");
        strcpy(line1, wifi_active ? "ACTIVE          " : "PRESS SET       ");
      } else if (cur_fld_no == FLD_LCD_OFF) {
        strcpy(line0, "TURN OFF LCD    ");
        strcpy(line1, "PRESS SET       ");
      } else {
        snprintf(line0, 17, "%-16s", ui_data[cur_fld_no].topRow);
        snprintf(line1, 17, "%-16s", ui_data[cur_fld_no].bottomRow);

        // Blink active manual states so the user knows it hasn't crashed
        portENTER_CRITICAL(&syncMux);
        int mode_snap = sync_mode;
        portEXIT_CRITICAL(&syncMux);

        bool is_pending = (pending_manual_status && cur_fld_no == FLD_SEND_STATUS) ||
                          (pending_manual_gps && cur_fld_no == FLD_SEND_GPS) ||
                          (pending_manual_health && cur_fld_no == FLD_SEND_HEALTH) ||
                          (mode_snap == eHealthStart && cur_fld_no == FLD_SEND_HEALTH) ||
                          (mode_snap == eGPSStart && cur_fld_no == FLD_SEND_GPS) ||
                          (mode_snap == eSMSStart && cur_fld_no == FLD_SEND_STATUS) ||
                          (mode_snap == eStartupGPS && cur_fld_no == FLD_SEND_GPS);
        if (is_pending && (millis() / 500) % 2 == 0) {
          memset(line1, ' ', 16);
          line1[16] = '\0';
        }
      }

      // --- LCD Heartbeat (Restore Original Bonus) ---
      static bool heartState = false;
      static unsigned long lastHeartbeat = 0;
      if (millis() - lastHeartbeat > 1000) {
        lastHeartbeat = millis();
        heartState = !heartState;
        line0[15] = heartState ? '.' : ' '; 
      }

      // Differential Drawing: Only write if something changed
      if (strcmp(line0, present_topRow) != 0) {
        lcd.setCursor(0, 0);
        lcd.print(line0);
        strcpy(present_topRow, line0);
      }
      if (strcmp(line1, present_bottomRow) != 0) {
        lcd.setCursor(0, 1);
        lcd.print(line1);
        strcpy(present_bottomRow, line1);
      }

    } else if (cur_mode == eEditOn) {
      char line0[17], line1[17];
      lcd.setCursor(0, 0);
      snprintf(line0, sizeof(line0), "%-16s", ui_data[cur_fld_no].topRow);
      lcd.print(line0);
      lcd.setCursor(0, 1);
      snprintf(line1, sizeof(line1), "%-16s", input_buf);
      lcd.print(line1);
      lcd.setCursor(cur_pos_no, 1);
      lcd.blink();
      // Invalidate cache for transition back to EditOff
      present_topRow[0] = 0; present_bottomRow[0] = 0;
    }
    xSemaphoreGive(i2cMutex);
  }
}

// v5.60: Background data refresh - exact original formatting
void refresh_sensor_data() {
  int d, m, y, hr, mi;
  portENTER_CRITICAL(&rtcTimeMux);
  d = current_day; m = current_month; y = current_year;
  hr = current_hour; mi = current_min;
  portEXIT_CRITICAL(&rtcTimeMux);

  // Update static fields
  strcpy(ui_data[FLD_STATION].topRow, "STATION ID");
  const char *lcd_id = station_name;
  if (strlen(station_name) == 6 && strncmp(station_name, "00", 2) == 0 && isDigitStr(station_name)) {
      lcd_id = station_name + 2;
  }
  snprintf(ui_data[FLD_STATION].bottomRow, 17, "%-16s", lcd_id);
  snprintf(ui_data[FLD_VERSION].bottomRow, 17, "%-16s", UNIT_VER);
  snprintf(ui_data[FLD_DATE].bottomRow, 17, "%02d-%02d-%04d", d, m, y);
  snprintf(ui_data[FLD_TIME].bottomRow, 17, "%02d:%02d", hr, mi);
  if (strlen(last_logged) == 0) {
    snprintf(ui_data[FLD_LAST_LOGGED].bottomRow, 17, "%-16s", "NA");
  } else {
    snprintf(ui_data[FLD_LAST_LOGGED].bottomRow, 17, "%-16s", last_logged);
  }

  // Battery & Solar
  static unsigned long last_bat = 0;
  if (millis() - last_bat > 5000) {
    last_bat = millis();
    // li_bat ADC reads handled inside get_calibrated_battery_voltage
    li_bat_val = get_calibrated_battery_voltage(); // Phase 8 Fix: eFuse-calibrated ADC
    snprintf(ui_data[FLD_BATTERY].bottomRow, 17, "%04.1f", li_bat_val);
    bat_val = li_bat_val;

     if (!wifi_active) {
        long solar_sum_ui = 0;
        int solar_samples_ui = 0;
        for (int i = 0; i < 10; i++) {
           int solar_raw_ui;
           if (adc2_get_raw(ADC2_CHANNEL_8, ADC_WIDTH_BIT_12, &solar_raw_ui) == ESP_OK) {
             solar_sum_ui += solar_raw_ui;
             solar_samples_ui++;
           }
           vTaskDelay(2 / portTICK_PERIOD_MS);
        }
        if (solar_samples_ui > 0) {
          solar_val = ((float)solar_sum_ui / solar_samples_ui / 4096.0) * 3.6 * 7.2;
          snprintf(ui_data[FLD_SOLAR].bottomRow, 17, "%04.1f", solar_val);
        }
     }
  }

  // Live fields
  // v5.76: Protected sensor read (Consistency Fix)
  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
    if (cur_fld_no == FLD_TEMP) snprintf(ui_data[FLD_TEMP].bottomRow, 17, "%0.1f", temperature);
    else if (cur_fld_no == FLD_HUMIDITY) snprintf(ui_data[FLD_HUMIDITY].bottomRow, 17, "%0.1f", humidity);
    xSemaphoreGive(i2cMutex);
  }
  
  if (cur_fld_no == FLD_INST_WS) snprintf(ui_data[FLD_INST_WS].bottomRow, 17, "%0.2f", cur_wind_speed);
  else if (cur_fld_no == FLD_AVG_WS) snprintf(ui_data[FLD_AVG_WS].bottomRow, 17, "%0.2f", cur_avg_wind_speed);
  else if (cur_fld_no == FLD_WIND_DIR) snprintf(ui_data[FLD_WIND_DIR].bottomRow, 17, "%03d", (int)windDir);
  else if (cur_fld_no == FLD_PRESSURE) snprintf(ui_data[FLD_PRESSURE].bottomRow, 17, "%0.1f", pressure);
  else if (cur_fld_no == FLD_RF_RES) {
    snprintf(ui_data[FLD_RF_RES].bottomRow, 17, "%0.2f", RF_RESOLUTION);
  }
  else if (cur_fld_no == FLD_RF_ML) {
    float live_rf = (float)rf_count.val * RF_RESOLUTION;
    snprintf(ui_data[FLD_RF_ML].bottomRow, 17, "%06.2f", live_rf);
  }
  else if (cur_fld_no == FLD_SIM_STATUS) {
    int rssi = signal_strength;
    bool sigValid = !(rssi == 0 || rssi == 99 || rssi == -114);
    if (strlen(carrier) == 0 || strcmp(carrier, "NA") == 0 || !sigValid) {
      strcpy(ui_data[FLD_SIM_STATUS].topRow, "SIGNAL          ");
      strcpy(ui_data[FLD_SIM_STATUS].bottomRow, "FETCHING...     ");
    } else {
      snprintf(ui_data[FLD_SIM_STATUS].topRow, 17, "SIM:%s", carrier);
      if (rssi > 0) strcpy(ui_data[FLD_SIM_STATUS].bottomRow, "SIGNAL: WEAK    ");
      else snprintf(ui_data[FLD_SIM_STATUS].bottomRow, 17, "%d dBm         ", rssi);
    }
  }
  else if (cur_fld_no == FLD_REGISTRATION) {
    if (strcmp(reg_status, "NA") == 0) strcpy(ui_data[FLD_REGISTRATION].bottomRow, "FETCHING...     ");
    else snprintf(ui_data[FLD_REGISTRATION].bottomRow, 17, "%-16s", reg_status);
  }
  else if (cur_fld_no == FLD_HTTP_FAILS && pcb_clear_state == 0) {
    static unsigned long last_bl = 0;
    static int bl_cur = 0;
    if (millis() - last_bl > 2000) {
      last_bl = millis();
      bl_cur = get_total_backlogs(false);
    }
    snprintf(ui_data[FLD_HTTP_FAILS].bottomRow, 17, "P:%-2d C:%-4d B:%-4d", 
             diag_http_present_fails, diag_http_cum_fails, bl_cur);
  }
  else if (cur_fld_no == FLD_RF_RES && rf_res_edit_state == 0) {
    snprintf(ui_data[FLD_RF_RES].bottomRow, 17, "%.2f", RF_RESOLUTION);
  }
  else if (cur_fld_no == FLD_LOG) {
    if (last_recorded_yy > 0) {
      snprintf(ui_data[FLD_LOG].bottomRow, 17, "%04d%02d%02d %02d:%02d", 
               last_recorded_yy, last_recorded_mm, last_recorded_dd, 
               last_recorded_hr, last_recorded_min);
    } else {
      int p_min = (current_min / 15) * 15;
      snprintf(ui_data[FLD_LOG].bottomRow, 17, "%04d%02d%02d %02d:%02d", 
               current_year, current_month, current_day, current_hour, p_min);
    }
  }
}

void lcdkeypad(void *pvParameters) {
  esp_task_wdt_add(NULL);
  static int calib_mode = 0; // 0=Field, 1=Test
  static char savedWakeupKey = NO_KEY;
  static int delete_confirm_state = 0; // v5.60: For two-step factory reset

  configure_keypad();

  lcdkeypad_start = 0;
  calib_flag = 0;
  delay(1000);
  disp_fld_no = -1;
  cur_fld_no = 0;
  input_buf[0] = 0;
  input_buf[16] = 0; 
  show_now = 1;

  if (wired == 0) {
    digitalWrite(32, LOW); 
  } else {
    if (wakeup_reason_is != timer) {
      digitalWrite(32, HIGH); 
      portENTER_CRITICAL(&syncMux);
      lcdkeypad_start = 1;
      portEXIT_CRITICAL(&syncMux);
    } else {
      digitalWrite(32, LOW);
      portENTER_CRITICAL(&syncMux);
      lcdkeypad_start = 0;
      portEXIT_CRITICAL(&syncMux);
    }
  }
  vTaskDelay(100 / portTICK_PERIOD_MS);

  // Load Calibration string for UI display
#if (SYSTEM == 0) || (SYSTEM == 2)
  // v5.70: Protect calibration load with fsMutex
  if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
    if (SPIFFS.exists("/calib.txt")) {
      File f8 = SPIFFS.open("/calib.txt", FILE_READ);
      if (f8) {
        String c = f8.readStringUntil('\n');
        f8.close();
        strncpy(ui_data[FLD_RF_CALIB].bottomRow, c.c_str(), 16);
        ui_data[FLD_RF_CALIB].bottomRow[16] = '\0';
        strcpy(calib_text, c.c_str());

        // v5.61: Parse historical calibration data into RTC variables for SMS/Status reports
        int yr, mo, dy;
        char st[8];
        if (sscanf(calib_text, "%04d-%02d-%02d %s", &yr, &mo, &dy, st) >= 3) {
          calib_year = yr;
          calib_month = mo;
          calib_day = dy;
          if (strstr(st, "PASS"))
            calib_sts = 1;
          else
            calib_sts = 0;
          debugf("[CALIB] Loaded: %04d-%02d-%02d Status: %d\n", calib_year,
                 calib_month, calib_day, calib_sts);
        }
      }
    }
    xSemaphoreGive(fsMutex);
  } else {
    debugln("[UI] Mutex Timeout: Skipping calib.txt load.");
  }
#endif

  for (;;) {
    esp_task_wdt_reset();

    // Phase 7 Fix: Safely execute the deferred LCD power cut with I2C Mutex protection
    if (lcd_power_cut_pending) {
      if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) == pdTRUE) {
        digitalWrite(32, LOW); // Turn OFF power to LCD (5V) safely
        // Reset the I2C peripheral purely to re-float the pins and avoid diode drops
        Wire.end();
        Wire.begin(I2C_SDA, I2C_SCL, 100000); // Phase 9 Fix: Force exactly 100kHz for DS1307 stability
        Wire.setTimeOut(I2C_TIMEOUT_MS);
        xSemaphoreGive(i2cMutex);
        lcd_power_cut_pending = false; // Successfully cleared
      }
      // If we failed to get Mutex, leave lcd_power_cut_pending = true. It will retry next loop!
    }

    if (lcdkeypad_start == 1 && wifi_active) {
      timerWrite(lcd_timer, 0);
    }

    if (lcdkeypad_start == 0) {
      char wakeupKey = keypad.getKey();
      bool verified_press = (wakeupKey != NO_KEY);
      
      // v5.68 Hardware Ghosting Fix: If the physical power switch for the LCD/Keypad is OFF,
      // the data pins will float and keypad.getKey() hallucinate random keys (like '6').
      // We ping the LCD expander (0x27) to verify the bus is electrically alive.
      if (wakeupKey != NO_KEY) {
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) == pdTRUE) {
          Wire.beginTransmission(0x27);
          if (Wire.endTransmission() != 0) {
            // No ACK from the LCD. The physical power switch must be OFF.
            wakeupKey = NO_KEY; // Ignore ghost key
            // debugln("[UI] Ignored ghost keypress. LCD switch is currently OFF."); // Optional, avoid spam
          }
          xSemaphoreGive(i2cMutex);
        }
      }

        // v5.78 Hardening: Mask electrical "Ghost" presses during background GPRS cycles.
        // If the scheduler is busy and the press didn't resolve to a physical key immediately,
        // it is almost certainly a modem-induced voltage dip on the interrupt line.
        if (verified_press && schedulerBusy && wakeupKey == NO_KEY) {
           debugln("[UI] Masking electrical ghost-press during background GPRS task.");
           verified_press = false; 
        }

        if (verified_press) {
          vTaskDelay(20 / portTICK_PERIOD_MS); // v5.98: Increased to 20ms for stronger EMI rejection
          if (keypad.getState() != PRESSED) {
             // If the key didn't stay pressed, it was almost certainly a noise spike. Reject.
             debugln("[UI] Rejecting noise spike/transient keypress.");
             verified_press = false;
             wakeupKey = NO_KEY;
          } else {
             // Key is stable or went back to NO_KEY (valid short press)
             wakeup_reason_is = ext0;
             savedWakeupKey = wakeupKey;
             debugf("[UI] Verified Key: %c\n", (wakeupKey != NO_KEY ? wakeupKey : 'P'));
          }
      }
    }

    bool should_activate = (wakeup_reason_is == ext0) || (wired == 1 && wakeup_reason_is == 0 && lcd_timer == NULL);

    if (last_lcd_state == 1 && lcdkeypad_start == 0) {
      // Intentionally empty. No longer breaking the I2C peripheral connection.
    }
    last_lcd_state = lcdkeypad_start;

    if (should_activate) {
      if (lcdkeypad_start == 0 || lcd_timer == NULL) {
        lcdkeypad_start = 1; // v5.77: Claim UI priority immediately before power-up delays
        digitalWrite(32, HIGH);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        cur_fld_no = 0; // v5.60: Ensure we always start at Station ID on wakeup

        if (lcd_timer == NULL) {
          lcd_timer = timerBegin(1000000);
          if (lcd_timer) timerAttachInterrupt(lcd_timer, &lcdTimer);
        }
        
        pinMode(4, INPUT_PULLUP);
        pinMode(12, INPUT_PULLUP);
        pinMode(13, OUTPUT);
        pinMode(14, OUTPUT);
        pinMode(15, OUTPUT);

        if (lcd_timer) {
          timerAlarm(lcd_timer, 180000000, false, 0); // v5.60: 180s default 
          timerWrite(lcd_timer, 0);
        }
        // cur_fld_no = 0 already handled in wakeup block
        vTaskDelay(200 / portTICK_PERIOD_MS);

        // v5.85: Increased timeout to 3s and added retry logic to prevent LCD activation failures 
        // when RTC or Temp/Hum tasks are busy on the I2C bus.
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(3000)) == pdTRUE) {
          lcd.init();
          lcd.display();
          lcd.backlight();
          lcd.clear();
          lcd.noCursor();
          lcd.noBlink();
          show_now = 1;
          xSemaphoreGive(i2cMutex);
        } else {
          debugln("[UI] I2C Mutex Timeout on LCD Init - Attempting bus recovery...");
          recoverI2CBus(false);
          lcdkeypad_start = 0; // v5.85: Fallback to OFF if absolutely blocked
        }
      } else {
        timerWrite(lcd_timer, 0);
        timerAlarm(lcd_timer, 180000000, false, 0); // v5.60: 180s
      }
      wakeup_reason_is = timer;
    }

    if (lcdkeypad_start == 1) {
      // Background Data Preparation
      if (calib_flag == 1) {
        // Calibration Running logic
        if (millis() - calib_start_time > 300000) { // 5 min timeout
          calib_flag = 2; // Auto stop
        }
        float live_c_rf = (float)calib_count.val * RF_RESOLUTION;
        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
          if (!calib_header_drawn) {
            lcd.setCursor(0, 0);
            lcd.print("Count     RF(mm)");
            calib_header_drawn = 1;
          }
          lcd.setCursor(0, 1);
          char b[17];
          snprintf(b, 17, "%-5d     %-6.2f", (int)calib_count.val, live_c_rf);
          lcd.print(b);
          xSemaphoreGive(i2cMutex);
        }
      } else if (calib_flag == 2) {
        // Calibration Result logic
        calib_mode_flag.val = 0; 
        calib_flag = 0;
        int count = calib_count.val;
        bool pass = (count == 10);
        snprintf(calib_text, sizeof(calib_text), "%04d-%02d-%02d %s", 
                 current_year, current_month, current_day, (pass ? "PASS" : "FAIL"));
        
        // v5.61: Update global status variables immediately (Survive Sleep for SMS/Health)
        calib_sts = pass ? 1 : 0;
        calib_year = current_year;
        calib_month = current_month;
        calib_day = current_day;

        // v5.70: Protect calibration save with fsMutex
        if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
          File f = SPIFFS.open("/calib.txt", FILE_WRITE);
          if (f) { f.print(calib_text); f.close(); }
          xSemaphoreGive(fsMutex);
        }

        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("FIELD CALIB:");
          lcd.setCursor(0, 1);
          lcd.print(calib_text);
          xSemaphoreGive(i2cMutex);
          
          // Persistence: Store in UI data so it stays after navigation
          strncpy(ui_data[FLD_RF_CALIB].bottomRow, calib_text, 16);
          ui_data[FLD_RF_CALIB].bottomRow[16] = '\0';
          
          vTaskDelay(3000 / portTICK_PERIOD_MS);
          present_topRow[0] = 0; present_bottomRow[0] = 0;
          show_now = 1;
        }
      } else {
        // Periodic Refresh
        static unsigned long refresh_timer = 0;
        if (show_now || (millis() - refresh_timer > 500)) {
          refresh_timer = millis();
          refresh_sensor_data();
          draw_current_page();
          show_now = 0;
        }
      }

      // --- KEYPAD PROCESSING ---
      key = keypad.getKey();
      if (key == NO_KEY && savedWakeupKey != NO_KEY) {
        key = savedWakeupKey;
        savedWakeupKey = NO_KEY;
      }

      if (key != NO_KEY) {
        extern unsigned long last_key_time;
        last_key_time = millis();
        if (lcd_timer) {
          timerWrite(lcd_timer, 0);
          timerAlarm(lcd_timer, 180000000, false, 0);
        }

        int i = (int)key - 48;
        if (i < 0 || i > 6) i = 0;

        int len = 0;
        if (ui_data[cur_fld_no].fieldType == eNumeric || ui_data[cur_fld_no].fieldType == eAlphaNum) {
          len = strlen(inpCharSet[ui_data[cur_fld_no].fieldType]);
        }

        switch (i) {
        case 1: // CLEAR
          if (cur_mode == eEditOn) {
            if (cur_fld_no == FLD_RF_RES) {
              rf_res_edit_state = 0;
              strcpy(ui_data[FLD_RF_RES].topRow, "RF RESOLUTION");
            } else if (cur_fld_no == FLD_DELETE_DATA) {
              snprintf(ui_data[FLD_DELETE_DATA].topRow, 17, "DELETE DATA?   %c", hw_tag);
            }
            cur_mode = eEditOff;
            show_now = 1;
          } else if (pcb_clear_state == 1) {
            pcb_clear_state = 0;
            show_now = 1;
          } else if (calib_flag == 1) {
            calib_flag = 0;
            calib_mode_flag.val = 0;
            show_now = 1;
          } else if (delete_confirm_state == 1) {
            delete_confirm_state = 0;
            snprintf(ui_data[FLD_DELETE_DATA].topRow, 17, "DELETE DATA?   %c", hw_tag);
            show_now = 1;
          } else if (rf_res_edit_state > 0) {
            rf_res_edit_state = 0;
            strcpy(ui_data[FLD_RF_RES].topRow, "RF RESOLUTION");
            cur_mode = eEditOff;
            show_now = 1;
          }
          break;

        case 2: // UP
          if (cur_mode == eEditOn) {
            cur_char_no = (cur_char_no + 1 + len) % len;
            if (cur_pos_no < 16) input_buf[cur_pos_no] = inpCharSet[ui_data[cur_fld_no].fieldType][cur_char_no];
          } else {
            int start_fld = cur_fld_no;
            do {
              cur_fld_no = (cur_fld_no + 1) % FLD_COUNT;
            } while (!isFieldVisible(cur_fld_no) && cur_fld_no != start_fld);

            if (cur_fld_no == FLD_LOG && last_recorded_yy > 0) {
               snprintf(ui_data[FLD_LOG].bottomRow, 17, "%04d%02d%02d %02d:%02d", 
                        last_recorded_yy, last_recorded_mm, last_recorded_dd, 
                        last_recorded_hr, last_recorded_min);
            }
          }
          show_now = 1;
          break;

        case 3: // SET
          if (cur_fld_no == FLD_WIFI_ENABLE) {
#if ENABLE_WEBSERVER == 1
            if (!wifi_active) {
              if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                lcd.clear(); lcd.print("WIFI STARTING...");
                xSemaphoreGive(i2cMutex);
              }
              // v5.79: Hardened WiFi startup sequence
              setCpuFrequencyMhz(160); // Set frequency BEFORE task starts on Core 0
              xTaskCreatePinnedToCore(webServer, "webServerTask", 8192, NULL, 2, &webServer_h, 0); // Pin to Core 0
            } else {
              if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                lcd.clear(); lcd.print("WIFI STOPPING...");
                xSemaphoreGive(i2cMutex);
              }
              // Signal graceful Watchdog teardown; webServerTask murders itself safely
              wifi_active = false; 
            }
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            present_topRow[0] = 0; present_bottomRow[0] = 0;
            show_now = 1;
#else
            if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
              lcd.clear(); lcd.print("WIFI DISABLED");
              xSemaphoreGive(i2cMutex);
            }
            vTaskDelay(1500 / portTICK_PERIOD_MS);
            present_topRow[0] = 0; present_bottomRow[0] = 0;
            show_now = 1;
#endif
          } else if (cur_fld_no == FLD_LCD_OFF) {
            lcdkeypad_start = 0; digitalWrite(32, LOW);
          } else if (cur_fld_no == FLD_SEND_STATUS) {
            portENTER_CRITICAL(&syncMux);
            if (sync_mode == eSyncModeInitial || sync_mode == eSMSStop || sync_mode == eHttpStop || sync_mode == eExceptionHandled) {
              send_status = 1; sync_mode = eSMSStart;
              portEXIT_CRITICAL(&syncMux);
              strcpy(ui_data[FLD_SEND_STATUS].bottomRow, "SENDING...     ");
            } else {
              portEXIT_CRITICAL(&syncMux);
            portENTER_CRITICAL(&syncMux);
            pending_manual_status = true;
            portEXIT_CRITICAL(&syncMux);
              strcpy(ui_data[FLD_SEND_STATUS].bottomRow, "PLEASE WAIT..  ");
            }
          } else if (cur_fld_no == FLD_SEND_GPS) {
            portENTER_CRITICAL(&syncMux);
            if (sync_mode == eSyncModeInitial || sync_mode == eSMSStop || sync_mode == eHttpStop || sync_mode == eExceptionHandled) {
              sync_mode = eGPSStart;
              portEXIT_CRITICAL(&syncMux);
              strcpy(ui_data[FLD_SEND_GPS].bottomRow, "SENDING...     ");
            } else {
              portEXIT_CRITICAL(&syncMux);
            portENTER_CRITICAL(&syncMux);
            pending_manual_gps = true;
            portEXIT_CRITICAL(&syncMux);
              strcpy(ui_data[FLD_SEND_GPS].bottomRow, "PLEASE WAIT..  ");
            }
          } else if (cur_fld_no == FLD_SEND_HEALTH) {
#if ENABLE_HEALTH_REPORT == 1
            portENTER_CRITICAL(&syncMux);
            pending_manual_health = true;
            if (sync_mode == eSyncModeInitial || sync_mode == eSMSStop || sync_mode == eHttpStop || sync_mode == eExceptionHandled) {
              sync_mode = eHealthStart; // Uses the GPS + HEALTH sequence explicitly
              portEXIT_CRITICAL(&syncMux);
              strcpy(ui_data[FLD_SEND_HEALTH].bottomRow, "SENDING...     ");
            } else {
              portEXIT_CRITICAL(&syncMux);
              strcpy(ui_data[FLD_SEND_HEALTH].bottomRow, "PLEASE WAIT..  ");
            }
#else
            strcpy(ui_data[FLD_SEND_HEALTH].bottomRow, "DISABLED       ");
#endif
          } else if (cur_fld_no == FLD_HTTP_FAILS) {
            if (pcb_clear_state == 0) {
              pcb_clear_state = 1;
              strcpy(ui_data[FLD_HTTP_FAILS].bottomRow, "CLEAR BACKLOG? ");
            } else {
              // WIPE BACKLOG
              if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                 lcd.clear(); lcd.print("Wiping Backlog");
                 xSemaphoreGive(i2cMutex); // Safely release immediately
                 
                 // v5.70: Protect backlog wipe with fsMutex
                 if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
                    debugln("[LCD] User confirmed CLEAR BACKLOG.");
                    SPIFFS.remove("/unsent.txt");
                    SPIFFS.remove("/ftpunsent.txt");
                    SPIFFS.remove("/unsent_pointer.txt");
                    xSemaphoreGive(fsMutex);
                 }
                 diag_http_present_fails = 0;
                 diag_http_cum_fails = 0;
                 pcb_clear_state = 0; 
                 get_total_backlogs(true); // v5.85: Force UI refresh of backlog count immediately
                 
                 vTaskDelay(1000 / portTICK_PERIOD_MS);
                 
                 if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                    lcd.clear(); lcd.print("BACKLOG CLEARED");
                    vTaskDelay(1000 / portTICK_PERIOD_MS); // allow humans to read it
                    xSemaphoreGive(i2cMutex);
                 }
              } else {
                 debugln("[LCD] ERROR: Failed to acquire i2cMutex for CLEAR BACKLOG!");
                 pcb_clear_state = 0;
                 strcpy(ui_data[FLD_HTTP_FAILS].bottomRow, "MUTEX TIMEOUT  ");
              }
              present_topRow[0] = 0; present_bottomRow[0] = 0;
            }
          } else if (cur_fld_no == FLD_RF_CALIB) {
            if (calib_flag == 0) {
              calib_start_time = millis();
              calib_count.val = 0; calib_mode_flag.val = 1; calib_flag = 1;
              calib_header_drawn = 0;
              if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                lcd.clear(); xSemaphoreGive(i2cMutex);
              }
            } else if (calib_flag == 1) {
              calib_flag = 2; // Stop
            }
          } else if (cur_fld_no == FLD_RF_RES) {
            if (rf_res_edit_state == 0) {
              rf_res_edit_state = 1; // Password entry mode
              strcpy(ui_data[FLD_RF_RES].topRow, "ENTER PASSWORD");
              strcpy(input_buf, "    ");
              cur_mode = eEditOn; cur_pos_no = 0; cur_char_no = 0;
            } else if (rf_res_edit_state == 1) {
              if (strcmp(input_buf, "2000") == 0) {
                rf_res_edit_state = 2; // Value entry mode
                strcpy(ui_data[FLD_RF_RES].topRow, "EDIT RF RES");
                snprintf(input_buf, 17, "%.2f", RF_RESOLUTION);
                cur_pos_no = 0; cur_char_no = 0;
              } else {
                rf_res_edit_state = 0;
                strcpy(ui_data[FLD_RF_RES].topRow, "RF RESOLUTION");
                cur_mode = eEditOff;
              }
            } else if (rf_res_edit_state == 2) {
              // v5.88 Fix: Use tolerance-based comparison instead of exact float equality.
              // atof() returns double; converting to float can cause a bit-mismatch with
              // 0.25f/0.50f literals even though both are exactly representable in IEEE 754.
              // fabs() check is immune to this truncation issue.
              float new_res = (float)atof(input_buf);
              bool valid_res = (fabsf(new_res - 0.25f) < 0.01f) || (fabsf(new_res - 0.50f) < 0.01f);
              if (valid_res) {
                // Snap to exact value to avoid any residual imprecision
                new_res = (fabsf(new_res - 0.25f) < 0.01f) ? 0.25f : 0.50f;
                RF_RESOLUTION = new_res;
                debugf("[RF_RES] Applying new resolution: %.2f\n", new_res);
                // Save to SPIFFS
                if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
                  File f = SPIFFS.open("/rf_res.txt", FILE_WRITE);
                  if (f) { f.print(new_res, 2); f.close(); debugln("[RF_RES] Saved to SPIFFS"); }
                  else { debugln("[RF_RES] ERROR: Failed to open rf_res.txt for writing!"); }
                  xSemaphoreGive(fsMutex);
                } else {
                  debugln("[RF_RES] ERROR: fsMutex timeout during save!");
                }
                // Also save to NVS as a bulletproof backup
                Preferences rfPrefs;
                rfPrefs.begin("sys-config", false);
                rfPrefs.putFloat("rf_res", new_res);
                rfPrefs.end();
                debugln("[RF_RES] Saved to NVS");
              } else {
                debugf("[RF_RES] Rejected invalid input: '%s' (parsed: %.4f)\n", input_buf, new_res);
              }
              // Wipe + reboot: only run if value was valid
              if (valid_res && xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(3000)) == pdTRUE) {
                 lcd.clear(); lcd.print("RES CHANGED!");
                 vTaskDelay(1000/portTICK_PERIOD_MS);
                 lcd.clear(); lcd.print("WIPING DATA...");
                 xSemaphoreGive(i2cMutex); // Release before doing massive flash wipes
                 
                 // v5.70: Protect factory wipe with fsMutex
                 if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(10000)) == pdTRUE) {
                    debugln("[LCD] Resolution changed. Factory wiping SPIFFS...");
                    SPIFFS.remove("/unsent.txt"); SPIFFS.remove("/ftpunsent.txt");
                    File root = SPIFFS.open("/"); 
                    File file = root.openNextFile();
                    while(file) {
                       String n = file.name();
                       // v5.85 Surgical Fix: Account for leading slash in SPIFFS filenames (Issue: Protection Bypass)
                       bool isCritical = (n == "station.txt" || n == "/station.txt" || 
                                         n == "rf_fw.txt" || n == "/rf_fw.txt" || 
                                         n == "station.doc" || n == "/station.doc" || 
                                         n == "rf_res.txt" || n == "/rf_res.txt");
                       
                       if (!isCritical) {
                         debug("Removing: "); debugln(n);
                         SPIFFS.remove(n.startsWith("/") ? n : "/" + n);
                       }
                       file.close(); file = root.openNextFile();
                    }
                    root.close();
                    xSemaphoreGive(fsMutex);
                 }
                 
                 if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                    lcd.clear(); lcd.print("DONE! REBOOTING");
                    xSemaphoreGive(i2cMutex);
                 }
                 debugln("[LCD] Wipe complete. Rebooting...");
                 vTaskDelay(2000 / portTICK_PERIOD_MS);
                 ESP.restart();
              } else {
                 debugln("[LCD] ERROR: Failed to acquire i2cMutex during RF_RES factory wipe!");
              }
            }
          } else if (cur_fld_no == FLD_DELETE_DATA) {
            if (delete_confirm_state == 0) {
              delete_confirm_state = 1;
              snprintf(ui_data[FLD_DELETE_DATA].topRow, 17, "ARE YOU SURE?  %c", hw_tag);
              strcpy(ui_data[FLD_DELETE_DATA].bottomRow, "PRESS SET: YES");
              debugln("[LCD] User selected DELETE_DATA. Awaiting confirmation...");
            } else {
              debugln("[LCD] User confirmed Factory Reset via Keypad! Initiating...");
              // Perform deletion
              if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(3000)) == pdTRUE) {
                 lcd.clear(); 
                 if (hw_tag == '!') {
                    lcd.print("Repairing FS...");
                    debugln("[LCD] HW Tag is '!', triggered FULL FORMAT (REPAIR)...");
                 } else {
                    lcd.print("Deleting...");
                    debugln("[LCD] Factory Reset (Healthy FS). Wiping local logs...");
                 }
                 xSemaphoreGive(i2cMutex); 
                 
                 // v5.70: Protect manual wipe with fsMutex
                 if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(15000)) == pdTRUE) {
                    if (hw_tag == '!') {
                       // v5.75 Golden: Hard repair — formats the underlying SPIFFS
                       SPIFFS.format(); // v5.75 Golden: Guaranteed re-init
                       SPIFFS.begin(true, "/spiffs", 10, "spiffs"); 
                       debugln("[LCD] SPIFFS REPAIR COMPLETE. System reboot required.");
                    } else {
                       // Standard File Wipe
                       SPIFFS.remove("/unsent.txt"); SPIFFS.remove("/ftpunsent.txt");
                       File root = SPIFFS.open("/"); 
                       File file = root.openNextFile();
                       while(file) {
                          String n = file.name();
                          if (!(n == "station.txt" || n == "rf_fw.txt" || n == "station.doc" || n == "rf_res.txt")) {
                            SPIFFS.remove(n.startsWith("/") ? n : "/" + n);
                          }
                          file.close(); file = root.openNextFile();
                       }
                       root.close();
                       debugln("[LCD] LOG WIPE COMPLETE. System reboot required.");
                    }
                    xSemaphoreGive(fsMutex);
                 }
                 
                 if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                    lcd.clear(); lcd.print("DONE! REBOOTING");
                    xSemaphoreGive(i2cMutex);
                 }
                 vTaskDelay(2000 / portTICK_PERIOD_MS);
                 ESP.restart();
              } else {
                 debugln("[LCD] ERROR: Failed to acquire i2cMutex during DELETE_DATA!");
                 delete_confirm_state = 0;
                 snprintf(ui_data[FLD_DELETE_DATA].topRow, 17, "DELETE DATA?   %c", hw_tag);
                 strcpy(ui_data[FLD_DELETE_DATA].bottomRow, "MUTEX TIMEOUT");
              }
            }
          } else if (cur_fld_no == FLD_SD_COPY) {
            if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(2000)) == pdTRUE) {
              lcd.clear(); lcd.print("COPYING TO SD...");
              xSemaphoreGive(i2cMutex); // Release lock BEFORE massive file copy sequence

              // v5.89: [FIXED DEADLOCK] copyFilesFromSPIFFSToSD handles its own internal locking in two phases.
              // Taking it here caused a nested lock timeout              
              debugln("[LCD] User initiated bulk copy from SPIFFS to SD Card...");
              bool copy_ok = copyFilesFromSPIFFSToSD("/"); 
              
              if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(2000)) == pdTRUE) {
                 if (copy_ok) {
                    lcd.clear(); lcd.print("SD COPY DONE");
                    vTaskDelay(2000 / portTICK_PERIOD_MS);
                 } else {
                    lcd.clear(); lcd.print("SD COPY FAIL");
                    vTaskDelay(3000 / portTICK_PERIOD_MS);
                 }
                 present_topRow[0] = 0; present_bottomRow[0] = 0;
                 xSemaphoreGive(i2cMutex);
              }
              show_now = 1;
            } else {
              debugln("[LCD] ERROR: Failed to acquire i2cMutex for SD Copy!");
            }
          } else if (cur_fld_no == FLD_LOG) {
             if (cur_mode == eEditOff) {
                cur_mode = eEditOn;
                strcpy(input_buf, ui_data[FLD_LOG].bottomRow);
                cur_pos_no = 0; cur_char_no = 0;
             } else {
                if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                   lcd.clear(); lcd.setCursor(0, 0); lcd.print("SEARCHING...");
                   xSemaphoreGive(i2cMutex);
                }

                char dt[17]; strcpy(dt, input_buf);
                int yr = atoi(String(dt).substring(0,4).c_str());
                int mo = atoi(String(dt).substring(4,6).c_str());
                int dy = atoi(String(dt).substring(6,8).c_str());
                int hr = atoi(String(dt).substring(9,11).c_str());
                int mn = atoi(String(dt).substring(12,14).c_str());
                int sNo = (hr * 4 + mn / 15 + 61) % 96;

                if (sNo <= 60) {
                   int daysInMonth[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
                   if ((yr%4==0 && yr%100!=0) || (yr%400==0)) daysInMonth[2]=29;
                   dy++; 
                   if (dy > daysInMonth[mo]) { dy=1; mo++; if(mo>12){mo=1; yr++;} }
                }

                char fn[50]; snprintf(fn, 50, "/%s_%04d%02d%02d.txt", station_name, yr, mo, dy);
                
                if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                   lcd.clear(); lcd.setCursor(0, 0); lcd.print("SCANNING...");
                   xSemaphoreGive(i2cMutex);
                }                

                // v5.70: DEADLOCK PREVENTION - Release i2cMutex BEFORE taking fsMutex (Issue 32 Cleanup)
                if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
                    // Fallback check for legacy/normalized prefix mismatch (v5.56 standard)
                    if (!SPIFFS.exists(fn)) {
                       char fallback[50];
                       if (strlen(station_name) == 4 && isDigitStr(station_name)) {
                          snprintf(fallback, sizeof(fallback), "/00%s_%04d%02d%02d.txt", station_name, yr, mo, dy);
                       } else if (strlen(station_name) == 6 && strncmp(station_name, "00", 2) == 0) {
                          snprintf(fallback, sizeof(fallback), "/%s_%04d%02d%02d.txt", station_name + 2, yr, mo, dy);
                       } else {
                          strcpy(fallback, "");
                       }
                       if (strlen(fallback) > 0 && SPIFFS.exists(fallback)) {
                          strcpy(fn, fallback);
                       }
                    }

                    if (SPIFFS.exists(fn)) {
                      File f = SPIFFS.open(fn, FILE_READ);
                      bool found = false;
                      char line[128];
                      while(f.available()) {
                         String l = f.readStringUntil('\n');
                         if (l.substring(0,2).toInt() == sNo) { strcpy(line, l.c_str()); found = true; break; }
                      }
                      f.close();

                      if (found) {
                         float tf=0, hf=0, af=0, rf=0; int wf=0;
                         if (SYSTEM == 0) {
                            float irf, crf; sscanf(line, "%*d,%*[^,],%*[^,],%f,%f", &irf, &crf);
                            if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                               lcd.clear();
                               lcd.setCursor(0,0); lcd.print("CUM_RF:"); lcd.setCursor(0,1); lcd.print(crf,2);
                               xSemaphoreGive(i2cMutex);
                            }
                         } else if (SYSTEM == 1) {
                            sscanf(line, "%*d,%*[^,],%*[^,],%f,%f,%f,%d", &tf, &hf, &af, &wf);
                            if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                               lcd.clear();
                               lcd.setCursor(0,0); char b1[17]; snprintf(b1,17,"T:%-4.1f H:%-4.1f",tf,hf); lcd.print(b1);
                               lcd.setCursor(0,1); char b2[17]; snprintf(b2,17,"AWS:%-4.1f WD:%-d",af,wf); lcd.print(b2);
                               xSemaphoreGive(i2cMutex);
                            }
                         } else if (SYSTEM == 2) {
                            sscanf(line, "%*d,%*[^,],%*[^,],%f,%f,%f,%f,%d", &rf, &tf, &hf, &af, &wf);
                            if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                               lcd.clear();
                               lcd.setCursor(0,0); char b1[17]; snprintf(b1,17,"R:%-3.1f T:%-4.1f",rf,tf); lcd.print(b1);
                               lcd.setCursor(0,1); char b2[17]; snprintf(b2,17,"H:%-2.0f AWS:%-4.1f",hf,af); lcd.print(b2);
                               xSemaphoreGive(i2cMutex);
                            }
                         }
                         vTaskDelay(5000 / portTICK_PERIOD_MS);
                      } else { 
                        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                            lcd.clear(); lcd.print("NOT IN FILE"); xSemaphoreGive(i2cMutex);
                        }
                        vTaskDelay(2000/portTICK_PERIOD_MS); 
                      }
                    } else { 
                       if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                          lcd.clear(); lcd.print("FILE NOT FOUND"); xSemaphoreGive(i2cMutex);
                       }
                       vTaskDelay(2000/portTICK_PERIOD_MS); 
                    }
                    xSemaphoreGive(fsMutex);
                } else {
                   debugln("[UI] Mutex Timeout: Skipping search.");
                }
                present_topRow[0] = 0; present_bottomRow[0] = 0;
                cur_mode = eEditOff;
             }
          } else {
             // Generic Edit
             if (cur_mode == eEditOff) {
                if (ui_data[cur_fld_no].fieldType != eDisplayOnly && ui_data[cur_fld_no].fieldType != eLive) {
                   cur_mode = eEditOn;
                   if (cur_fld_no == FLD_STATION) {
                      // v5.78 Hardening: Reverted to v5.74 "Blank Slate" behavior. 
                      // Field is cleared on entry to ensure intentional ID assignment.
                      memset(input_buf, ' ', 16);
                      input_buf[16] = '\0';
                   } else {
                      strcpy(input_buf, ui_data[cur_fld_no].bottomRow);
                   }
                   cur_pos_no = 0; cur_char_no = 0;
                }
             } else {
                if (cur_fld_no == FLD_STATION) {
                    String trimmed = String(input_buf);
                    trimmed.trim();

                    // Fallback: If user accidentally hit SET on a completely blank screen
                    if (trimmed.length() == 0) {
                        trimmed = String(station_name);
                    }

                    // v5.77 Hardened [UI-PAD]: Correct Padding Hierarchy
                    if (trimmed.length() == 4 && isDigitStr(trimmed.c_str())) {
                        char p_buf[16];
                        snprintf(p_buf, sizeof(p_buf), "00%s", trimmed.c_str());
                        trimmed = String(p_buf);
                    }

                    // v5.78 Hardening: Detect ID change and trigger mandatory data wipe
                    if (trimmed != String(station_name)) {
                        debugln("[UI] Station ID Change Detected. Initiating Mandatory Data Wipe...");
                        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(3000)) == pdTRUE) {
                            lcd.clear(); lcd.setCursor(0,0); lcd.print("ID CHANGED!");
                            lcd.setCursor(0,1); lcd.print("WIPING DATA...");
                            xSemaphoreGive(i2cMutex);
                        }

                        // Surgical Wipe & New ID persistence: Block the scheduler once
                        // v5.70 pattern: Protect with fsMutex
                        if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(10000)) == pdTRUE) {
                            SPIFFS.remove("/unsent.txt"); 
                            SPIFFS.remove("/ftpunsent.txt");
                            SPIFFS.remove("/unsent_pointer.txt");
                            // Also remove the old station-named log files to be 100% clean
                            File root = SPIFFS.open("/"); 
                            File file = root.openNextFile();
                            while(file) {
                                String n = file.name();
                                if (n.endsWith(".txt") && !(n == "station.txt" || n == "rf_fw.txt" || n == "rf_res.txt")) {
                                    SPIFFS.remove(n.startsWith("/") ? n : "/" + n);
                                }
                                file.close(); file = root.openNextFile();
                            }
                            root.close();

                            // v5.70: Save new identity while still holding the mutex for atomicity
                            File f1 = SPIFFS.open("/station.txt", FILE_WRITE);
                            if (f1) { f1.print(trimmed); f1.close(); }
                            File f2 = SPIFFS.open("/station.doc", FILE_WRITE);
                            if (f2) { f2.print(trimmed); f2.close(); }
                            
                            xSemaphoreGive(fsMutex);
                        }
                    }

                    strncpy(station_name, trimmed.c_str(), 15);
                    station_name[15] = '\0';
                    strcpy(ftp_station, station_name);
                   // NVS Save (Internal Flash)
                   Preferences prefs; prefs.begin("sys-config", false); 
                   prefs.putString("station", station_name); prefs.end();
                   // Requirement: Acquire GPS whenever ID changes (eStartupGPS does GPS + Health)
                   portENTER_CRITICAL(&syncMux);
                   if (sync_mode == eSyncModeInitial || sync_mode == eSMSStop || 
                       sync_mode == eHttpStop || sync_mode == eExceptionHandled) {
#if ENABLE_HEALTH_REPORT == 1
                      sync_mode = eStartupGPS;
#else
                      sync_mode = eHttpStop; // Savings: Skip GPS acquisition when health is disabled
#endif
                      portEXIT_CRITICAL(&syncMux);
                      strcpy(ui_data[FLD_SEND_GPS].bottomRow, "ACQUIRING GPS..");
                   } else {
                      portEXIT_CRITICAL(&syncMux);
                      portENTER_CRITICAL(&syncMux);
                      pending_manual_gps = true;
                      portEXIT_CRITICAL(&syncMux);
                   }
                } else if (cur_fld_no == FLD_DATE) {
                   int dd, mm, yy;
                   if (sscanf(input_buf, "%02d-%02d-%04d", &dd, &mm, &yy) == 3) {
                      portENTER_CRITICAL(&rtcTimeMux);
                       current_day = dd; current_month = mm; current_year = yy;
                       portEXIT_CRITICAL(&rtcTimeMux);
                      if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(500)) == pdTRUE) {
                         rtc.adjust(DateTime(current_year, current_month, current_day, current_hour, current_min, 0));
                         xSemaphoreGive(i2cMutex);
                      }
                   }
                } else if (cur_fld_no == FLD_TIME) {
                   int hh, mi;
                   if (sscanf(input_buf, "%02d:%02d", &hh, &mi) == 2) {
                      portENTER_CRITICAL(&rtcTimeMux);
                       current_hour = hh; current_min = mi;
                       portEXIT_CRITICAL(&rtcTimeMux);
                      if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(500)) == pdTRUE) {
                         rtc.adjust(DateTime(current_year, current_month, current_day, current_hour, current_min, 0));
                         record_hr = current_hour;
                         record_min = (current_min / 15) * 15;
                         // v5.70: Protect signature update with fsMutex
                         if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(5000)) == pdTRUE) {
                             File fileTemp4 = SPIFFS.open("/signature.txt", FILE_WRITE);
                             if (fileTemp4) {
                                snprintf(signature, 17, "%04d-%02d-%02d,%02d:%02d", current_year, current_month, current_day, record_hr, record_min);
                                fileTemp4.print(signature);
                                fileTemp4.close();
                             }
                             xSemaphoreGive(fsMutex);
                         }
                         rtcTimeChanged = true;
                         xSemaphoreGive(i2cMutex);
                      }
                   }
                }
                strcpy(ui_data[cur_fld_no].bottomRow, input_buf);
                cur_mode = eEditOff;
             }
          }
          show_now = 1;
          break;

        case 4: // LEFT
          if (cur_mode == eEditOn && cur_pos_no > 0) cur_pos_no--;
          show_now = 1;
          break;

        case 5: // DOWN
          if (cur_mode == eEditOn) {
            cur_char_no = (cur_char_no - 1 + len) % len;
            if (cur_pos_no < 16) input_buf[cur_pos_no] = inpCharSet[ui_data[cur_fld_no].fieldType][cur_char_no];
          } else {
            int start_fld = cur_fld_no;
            do {
              cur_fld_no = (cur_fld_no + FLD_COUNT - 1) % FLD_COUNT;
            } while (!isFieldVisible(cur_fld_no) && cur_fld_no != start_fld);

            if (cur_fld_no == FLD_LOG && last_recorded_yy > 0) {
               snprintf(ui_data[FLD_LOG].bottomRow, 17, "%04d%02d%02d %02d:%02d", 
                        last_recorded_yy, last_recorded_mm, last_recorded_dd, 
                        last_recorded_hr, last_recorded_min);
            } else if (cur_fld_no == FLD_LOG) {
               int p_min = (current_min / 15) * 15;
               snprintf(ui_data[FLD_LOG].bottomRow, 17, "%04d%02d%02d %02d:%02d", 
                        current_year, current_month, current_day, current_hour, p_min);
            }
          }
          show_now = 1;
          break;

        case 6: // RIGHT
          if (cur_mode == eEditOn && cur_pos_no < 15) cur_pos_no++;
          show_now = 1;
          break;
        }
      }
    }
    if (lcdkeypad_start && lcd_timer) {
      // v5.60: Stay awake during active manual triggers or startup tasks
      portENTER_CRITICAL(&syncMux);
      int mode_check = sync_mode;
      portEXIT_CRITICAL(&syncMux);

      if (mode_check == eSMSStart || mode_check == eGPSStart || 
          mode_check == eHealthStart || mode_check == eStartupGPS || cur_fld_no == FLD_LOG && cur_mode == eEditOn) {
        timerWrite(lcd_timer, 0);
      }
    }
    esp_task_wdt_reset();
    vTaskDelay((lcdkeypad_start ? 10 : 50) / portTICK_PERIOD_MS);
  }
}
