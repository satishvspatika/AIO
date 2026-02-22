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
char show_now;

// Returns true if the field should be shown for the current SYSTEM
bool isFieldVisible(int fld_id) {
#if ENABLE_WEBSERVER == 0
  if (fld_id == FLD_WIFI_ENABLE)
    return false;
#endif

#if SYSTEM == 0 // TRG ONLY
  if (fld_id == FLD_WIND_DIR || fld_id == FLD_INST_WS || fld_id == FLD_AVG_WS ||
      fld_id == FLD_TEMP || fld_id == FLD_HUMIDITY)
    return false;
#elif SYSTEM == 1 // TWS ONLY (Weather)
  if (fld_id == FLD_RF_ML || fld_id == FLD_RF_CALIB || fld_id == FLD_RF_RES)
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

// LCD Timer ISR callback function
void IRAM_ATTR lcdTimer() {
  portENTER_CRITICAL_ISR(&timerMux2);
  lcdkeypad_start = 0;   // Set the flag when the timer interrupts
  digitalWrite(32, LOW); // Turn OFF power to LCD (5V)
  portEXIT_CRITICAL_ISR(&timerMux2);
}

void lcdkeypad(void *pvParameters) {
  esp_task_wdt_add(NULL);
  static int calib_mode = 0;  // 0=Field, 1=Test
  static char calib_text[40]; // Result buffer

  // Configure keypad for better responsiveness
  configure_keypad();

  lcdkeypad_start = 0;
  calib_flag = 0;
  delay(1000);
  disp_fld_no = -1;
  cur_fld_no = 0;
  input_buf[0] = 0;
  input_buf[16] =
      0; // ensure input_buf is null-terminated (valid indices are 0-16)
  show_now = 1;
  if (wired == 0) {
    digitalWrite(32, LOW); // Turn OFF power to LCD (5V) for battery units
  } else {
    // For wired units, only force LCD ON if it's NOT a background timer wakeup
    if (wakeup_reason_is != timer) {
      digitalWrite(32, HIGH); // Keep ON for wired/manual units
      lcdkeypad_start = 1;
    } else {
      digitalWrite(32, LOW);
      lcdkeypad_start = 0;
    }
  }
  vTaskDelay(100);

  debug("[UI] Data Size: ");
  debugln(sizeof(ui_data));

  // Configure Station ID input type - Default to AlphaNum for flexibility
  ui_data[FLD_STATION].fieldType = eAlphaNum;

#if (SYSTEM == 0) || (SYSTEM == 2)

  if (SPIFFS.exists("/calib.txt")) {
    String content;
    File file8 = SPIFFS.open("/calib.txt", FILE_READ);
    if (!file8) {
      debugln("Failed to open calib.txt for reading");
    }                                      // #TRUEFIX
    content = file8.readStringUntil('\n'); // Read till EOL
    debug("Calibration done on  ");
    debugln(content);
    // Parse YYYY-MM-DD format
    if (content.length() >= 10) {
      calib_year = content.substring(0, 4).toInt();
      calib_month = content.substring(5, 7).toInt();
      calib_day = content.substring(8, 10).toInt();
      String stateStr = content.substring(11);
      stateStr.trim();

      if (stateStr.indexOf("PASS") != -1) {
        calib_sts = 1; // PASS
        debugln("LOADED: CALIB PASS");
      } else if (stateStr.indexOf("FAIL") != -1) {
        calib_sts = 0; // FAIL
        debugln("LOADED: CALIB FAIL");
      } else if (stateStr.indexOf("T:") != -1) {
        calib_sts = 4; // TEST MODE
        debugln("LOADED: CALIB TEST");
      } else {
        calib_sts = 3; // Unknown
      }
    }
    file8.close();
    strcpy(ui_data[FLD_RF_CALIB].bottomRow, content.c_str());
    snprintf(calib_text, sizeof(calib_text), "%s", content.c_str());

  } else {
    calib_sts = 3;
    calib_year = 0;
    calib_month = 0;
    calib_day = 0;
    strcpy(ui_data[FLD_RF_CALIB].bottomRow, "Yes ?");
    debugln();
    debugln("NO CALIB FILE ...");
    debugln();
  }

#endif

  for (;;) {

    esp_task_wdt_reset();

    // Keep LCD awake symmetrically as long as WiFi is active
    if (lcdkeypad_start == 1 && wifi_active) {
      timerWrite(lcd_timer, 0);
    }

    // Fallback: If LCD timed out but device is awake (e.g. GPRS active),

    // poll keypad manually to detect user wake-up attempt.
    if (lcdkeypad_start == 0) {
      char wakeupKey = keypad.getKey();
      // Detect User Interaction: Any Keypad Key OR Button 27 (EXT0)
      if (wakeupKey != NO_KEY || digitalRead(27) == LOW) {
        wakeup_reason_is = ext0;
        debugln("[UI] User interaction detected (Key/Pin27)!");
        delay(200); // Debounce
      }
    }

    bool should_activate =
        (wakeup_reason_is == ext0) ||
        (wired == 1 && wakeup_reason_is == 0 && lcd_timer == NULL);

    static int last_lcd_state = 0;

    // Auto-Sleep logic removed from here as it conflicts with GPRS/Scheduler
    // System sleep is now exclusively managed by loop() and scheduler()

    // Capture the transition for logging only
    if (last_lcd_state == 1 && lcdkeypad_start == 0) {
      debugln("LCD Timeout detected.");
    }
    last_lcd_state = lcdkeypad_start;

    if (should_activate) {
      // Debounce to prevent log spam
      static unsigned long last_ext0 = 0;
      if (wakeup_reason_is == ext0) {
        if (millis() - last_ext0 < 500) {
          wakeup_reason_is = timer;
          continue;
        }
        last_ext0 = millis();
      }

      // If LCD is not started OR the timer hasn't been initialized yet
      if (lcdkeypad_start == 0 || lcd_timer == NULL) {
        debugln("Turning LCD_CTRL to HIGH (Activation)...");
        digitalWrite(32, HIGH); // Turn ON power to LCD (5V)
        vTaskDelay(300);        // Wait for power stabilization

        if (lcd_timer == NULL) {
          lcd_timer = timerBegin(1000000); // 1MHz timer frequency
          timerAttachInterrupt(lcd_timer, &lcdTimer);
        }
        // Force Keypad Pin Configuration to override JTAG/Defaults
        pinMode(4, INPUT_PULLUP);
        pinMode(12, INPUT_PULLUP);
        pinMode(13, OUTPUT);
        pinMode(14, OUTPUT);
        pinMode(15, OUTPUT);

        timerAlarm(lcd_timer, 120000000, false, 0); // 2 mins (120 seconds)
        timerWrite(lcd_timer, 0); // Reset timer count to avoid immediate firing
        lcdkeypad_start = 1;
        vTaskDelay(300);

        if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
            pdTRUE) {
          lcd.init();
          lcd.clear();
          lcd.noCursor();
          lcd.noBlink();
          lcd.backlight();

          // Pre-populate Station ID immediately so first display is complete
          cur_fld_no = 0;
          strcpy(ui_data[FLD_STATION].topRow, "STATION ID");
          strcpy(ui_data[FLD_STATION].bottomRow, station_name);

          // Force refresh of cached comparison strings
          present_topRow[0] = 0;
          present_bottomRow[0] = 0;

          show_now = 1; // Trigger immediate full-screen draw

          xSemaphoreGive(i2cMutex);
        }
      } else {
        debugln("EXT0 pressed while LCD already ON. Resetting timer.");
        // If already started, just refresh/restart the timer to extend on-time
        // Use timerWrite(timer, 0) for better compatibility with different core
        // versions
        timerWrite(lcd_timer, 0);
        timerAlarm(lcd_timer, 120000000, false, 0); // Re-enable 2 min timeout
      }
      wakeup_reason_is = timer; // Consume the reason
    }

    if (lcdkeypad_start == 1) { // Only when ext0 is triggered
      static unsigned long debug_loop_timer = 0;
      if (millis() - debug_loop_timer > 5000) {
        debug_loop_timer = millis();
        // debug("[UI] Loop Running. Field: ");
        // debug(cur_fld_no);
        // debug(" Type: ");
        // debugln(ui_data[cur_fld_no].fieldType);
        if (ui_data[cur_fld_no].fieldType < 0 ||
            ui_data[cur_fld_no].fieldType > 3) {
          debugln("[UI] Invalid Type! Resetting.");
          cur_fld_no = 0;
          show_now = 1;
        }
      }

      // Data Preparation & Calibration Logic
      // Throttled to 5Hz (200ms) - balanced for display quality
      static unsigned long ui_update_timer = 0;
      if (millis() - ui_update_timer > 200) {
        ui_update_timer = millis();

        // RF
        float temp_rf = (float)(rf_count.val) * RF_RESOLUTION;
        snprintf(rf_str, sizeof(rf_str), "%06.2f", temp_rf);

        // Prepare the data for sending through ESPNOW
        if (cur_mode == eEditOff) {
          // Generate actual strings from variables
          snprintf(date_now, sizeof(date_now), "%02d-%02d-%04d", current_day,
                   current_month, current_year);
          snprintf(time_now, sizeof(time_now), "%02d:%02d", current_hour,
                   current_min);

          strcpy(ui_data[FLD_STATION].bottomRow, station_name);
          strcpy(ui_data[FLD_DATE].bottomRow, date_now);
          strcpy(ui_data[FLD_TIME].bottomRow, time_now);
        }

        // Ensure station_name display is padded/cleaned if needed
        int len_stn = strlen(ui_data[FLD_STATION].bottomRow);
        while (len_stn > 0 &&
               ui_data[FLD_STATION].bottomRow[len_stn - 1] == ' ') {
          ui_data[FLD_STATION].bottomRow[len_stn - 1] = 0;
          len_stn--;
        }

        // LIVE RF UPDATE: Calculate current millimetres live from raw ULP
        // pulses
#if SYSTEM == 1
        strcpy(ui_data[FLD_RF_ML].bottomRow, "NA");
#else
        float live_rf = (float)rf_count.val * RF_RESOLUTION;
        snprintf(ui_data[FLD_RF_ML].bottomRow,
                 sizeof(ui_data[FLD_RF_ML].bottomRow), "%06.2f", live_rf);
#endif

        strcpy(ui_data[FLD_VERSION].bottomRow, UNIT_VER);
        strcpy(ui_data[FLD_REGISTRATION].bottomRow, reg_status);
        strcpy(ui_data[FLD_LAST_LOGGED].bottomRow, last_logged);
        strcpy(ui_data[FLD_BATTERY].bottomRow, battery);
        strcpy(ui_data[FLD_SOLAR].bottomRow, solar_sense);

        // --- Item 10/21: LOG (RFCL_DT TM / LAST LOGGED AT) ---
        // Determine the most likely active file date (similar to webServer
        // logic) Optimized: Check SPIFFS only every 2 seconds AND only if we
        // are on the LOG page preventing UI lag on other screens.
        static unsigned long spiffs_check_timer = 0;
        static unsigned long log_page_entry_time = 0;
        static int last_fld = -1;

        // Track field changes to debounce heavy operations
        if (cur_fld_no != last_fld) {
          log_page_entry_time = millis();
          last_fld = cur_fld_no;
        }

        bool isLogPage = (cur_fld_no == FLD_LOG);

        // Checking SPIFFS is blocking. Only do it if:
        // 1. We are on the LOG page
        // 2. We have been on this page for > 500ms (prevent lag when scrolling
        // past)
        // 3. The refresh interval (2s) has passed
        if (isLogPage && cur_mode == eEditOff &&
            (millis() - log_page_entry_time > 500) &&
            (millis() - spiffs_check_timer > 2000)) {
          spiffs_check_timer = millis();

          int lcdDay = current_day, lcdMonth = current_month,
              lcdYear = current_year;
          int lcdSN = (current_hour * 4 + current_min / 15 + 61) % 96;
          if (lcdSN <= 60)
            next_date(
                &lcdDay, &lcdMonth,
                &lcdYear); // Logic is inverted in original code? next_date is
                           // called for <=60? Original code: if (lcdSN <= 60)
                           // next_date(...). Preservation of logic is key.

          char lcdActiveDate[10];
          snprintf(lcdActiveDate, sizeof(lcdActiveDate), "%04d%02d%02d",
                   lcdYear, lcdMonth, lcdDay);
          char lcdCheckPath[32];
          snprintf(lcdCheckPath, sizeof(lcdCheckPath), "/%s_%s.txt",
                   station_name, lcdActiveDate);

          // If active today's file doesn't exist, check yesterday's
          if (!SPIFFS.exists(lcdCheckPath)) {
            previous_date(&lcdDay, &lcdMonth, &lcdYear);
            snprintf(lcdActiveDate, sizeof(lcdActiveDate), "%04d%02d%02d",
                     lcdYear, lcdMonth, lcdDay);
          }

          char log_item_display[20];
          // Default to current calendar date and normalized last record time
          // for intuitive search
          int show_hr = record_hr;
          int show_min = record_min;
          // Use last_recorded if available (it comes from file read at boot)
          if (last_recorded_hr != 0 || last_recorded_min != 0 ||
              last_recorded_dd != 0) {
            show_hr = last_recorded_hr;
            show_min = last_recorded_min;
          }

          snprintf(log_item_display, sizeof(log_item_display),
                   "%04d%02d%02d %02d:%02d", current_year, current_month,
                   current_day, show_hr, show_min);

          strcpy(ui_data[FLD_LOG].bottomRow, log_item_display); // LOG Page
        }

        strcpy(ui_data[FLD_WIND_DIR].bottomRow, windDir_str);
        strcpy(ui_data[FLD_INST_WS].bottomRow, windSpeedInst_str);
        strcpy(ui_data[FLD_AVG_WS].bottomRow, prevWindSpeedAvg_str);
        strcpy(ui_data[FLD_TEMP].bottomRow, temp_str);
        strcpy(ui_data[FLD_HUMIDITY].bottomRow, hum_str);

        // Update WiFi status menu item based on actual state
        strcpy(ui_data[FLD_WIFI_ENABLE].bottomRow,
               wifi_active ? "ACTIVE" : "PRESS SET");

        // RF Resolution Display
        if (rf_res_edit_state == 0) {
          snprintf(ui_data[FLD_RF_RES].bottomRow,
                   sizeof(ui_data[FLD_RF_RES].bottomRow), "%.2f",
                   RF_RESOLUTION);
        }

        // Calibration Running
        if (calib_flag == 1) {
          // Timeout check for Field Calib (3 mins = 180000 ms)
          if (calib_mode == 0 && (millis() - calib_start_time > 180000)) {
            calib_flag = 2; // Auto-stop
          }

#if (SYSTEM == 0) || (SYSTEM == 2)
          calib_count_rf = calib_count.val;
          calib_rf_float = (float)calib_count_rf * RF_RESOLUTION;
          snprintf(calib_rf, sizeof(calib_rf), "%06.2f", calib_rf_float);
          strcpy(ui_data[FLD_RF_CALIB].bottomRow, calib_rf);
#endif
          // Update Display with Tips/MM
          if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
              pdTRUE) {
            if (calib_header_drawn == 0) {
              lcd.setCursor(0, 0);
              lcd.print("Count     RF(mm)");
              calib_header_drawn = 1;
            }
            lcd.setCursor(0, 1);
            char bot[17];
            snprintf(bot, sizeof(bot), "%-5d     %-6.2f", (int)calib_count_rf,
                     calib_rf_float);
            lcd.print(bot);
            xSemaphoreGive(i2cMutex);
          }
        }
        // Calibration Process Result
        else if (calib_flag == 2) {
          calib_mode_flag.val = 0; // PHYSICAL SEPARATION: Stop calib counting
          // Process based on mode
          calib_year = current_year;
          calib_month = current_month;
          calib_day = current_day;

          if (calib_mode == 0) {
            // Field Calib: Criteria is EXACTLY 10 tips (2.5mm)
            // User requested: "Not more not less"
            calib_sts = (calib_count_rf == 10) ? 1 : 0;
            snprintf(calib_text, sizeof(calib_text), "%04d-%02d-%02d %s",
                     calib_year, calib_month, calib_day,
                     (calib_sts ? "PASS" : "FAIL"));
            debugf1("Field Calib Result: %s\n", calib_text);

            // PERSISTENT: Update official calib file for SMS
            File f_calib = SPIFFS.open("/calib.txt", FILE_WRITE);
            if (f_calib) {
              f_calib.print(calib_text);
              f_calib.close();
            }
          } else {
            // CALIB TEST (Just show count, no pass/fail storage)
            snprintf(calib_text, sizeof(calib_text), "Total Tips: %d",
                     (int)calib_count_rf);
            debugf1("Calib Test: %s\n", calib_text);
            // Do NOT save to /calib.txt
          }

#if (SYSTEM == 0) || (SYSTEM == 2)
          // File writing moved into mode logic above
#endif

          // Display Result
          if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
              pdTRUE) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(calib_mode == 0 ? "FIELD CALIB:" : "CALIB TEST:");
            lcd.setCursor(0, 1);
            lcd.print(calib_text);
            xSemaphoreGive(i2cMutex);
            vTaskDelay(3000); // Wait for user to see

            // Cleanup: Revert to standard timeout after showing result
            // User said: "Once the test completes after 5 mins or manually
            // SET... After this only the 2 min IDLE timer should start"
            timerAlarm(lcd_timer, 120000000, false, 0); // 2 mins standard idle
            timerWrite(lcd_timer, 0);
            timerRestart(lcd_timer);

            // Cleanup
            strcpy(ui_data[FLD_RF_CALIB].bottomRow, calib_text);
            calib_flag = 0;
            show_now = 1;

          } else {
            // If mutex fails, DO NOT reset calib_flag.
            // Let the loop retry display in the next iteration.
            debugln("[UI] Calibration Result Display Delayed (Mutex failed to "
                    "take)");
          }
        }

        //                  else if(calib_flag == 0) {
        //                           if(calib_sts != 3) { // This means a calib
        //                           file is available
        //                                if(cur_fld_no == 6) {
        //                                        if(strcmp(calib_text,present_bottomRow))
        //                                        {
        //                                              strcpy(ui_data[6].bottomRow,calib_text);
        //                                              lcd.setCursor(0,1);
        //                                              lcd.print(ui_data[6].bottomRow);
        //                                              strcpy(present_bottomRow,ui_data[6].bottomRow);
        //                                        }
        //                                 }
        //                           }
        //                  }

        if (calib_flag == 0 && show_now) {
          if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
              pdTRUE) { // iter7
            // REMOVED: lcd.clear(); // AG: This was causing the 200ms flicker
            if (cur_mode == eEditOff) {
              lcd.noBlink();
              if (cur_fld_no == FLD_WIFI_ENABLE) {
                lcd.setCursor(0, 0);
                lcd.print(wifi_active ? "DISABLE WIFI    "
                                      : "ENABLE WIFI     ");
                lcd.setCursor(0, 1);
                lcd.print(wifi_active ? "ACTIVE          "
                                      : "PRESS SET       ");
              } else if (cur_fld_no == FLD_LCD_OFF) {
                lcd.setCursor(0, 0);
                lcd.print("TURN OFF LCD    ");
                lcd.setCursor(0, 1);
                lcd.print("PRESS SET       ");
              } else {
                char line0[17], line1[17];
                lcd.setCursor(0, 0);
                snprintf(line0, sizeof(line0), "%-16s",
                         ui_data[cur_fld_no].topRow);
                lcd.print(line0);
                lcd.setCursor(0, 1);
                snprintf(line1, sizeof(line1), "%-16s",
                         ui_data[cur_fld_no].bottomRow);
                lcd.print(line1);
              }
              disp_fld_no = cur_fld_no;
            } else if (cur_mode == eEditOn) {
              char line0[17], line1[17];
              lcd.setCursor(0, 0);
              snprintf(line0, sizeof(line0), "%-16s",
                       ui_data[cur_fld_no].topRow);
              lcd.print(line0);
              lcd.setCursor(0, 1);
              snprintf(line1, sizeof(line1), "%-16s", input_buf);
              lcd.print(line1);
              // Enable cursor blinking at current position
              lcd.setCursor(cur_pos_no, 1);
              lcd.blink();
            }
            show_now = 0;
            xSemaphoreGive(i2cMutex);
          }
        }
      } // End of ui_update_timer 200ms block

      // --- UNIFIED DISPLAY UPDATE LOGIC ---
      // Refresh display text from ui_data defaults (e.g. sensor readings)
      // Throttling to 3.3Hz (300ms) - balanced for smoothness and
      // responsiveness
      static unsigned long display_timer = 0;
      if (calib_flag == 0 && (millis() - display_timer > 300)) {
        display_timer = millis();
        // Only refresh background if NOT interacting (EditOff)
        // Checks valid for ALL field types now (Improvement!)
        if (cur_mode == eEditOff &&
            xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
                pdTRUE) {

          // Check if it's the WiFi field (Index 22)
          bool isWifiField = (cur_fld_no == FLD_WIFI_ENABLE);

          if (isWifiField) {
            const char *wantedTop =
                wifi_active ? "DISABLE WIFI    " : "ENABLE WIFI     ";
            if (strcmp(wantedTop, present_topRow)) {
              lcd.setCursor(0, 0);
              lcd.print(wantedTop);
              strcpy(present_topRow, wantedTop);
            }
          } else {
            if (strcmp(ui_data[cur_fld_no].topRow, present_topRow)) {
              lcd.setCursor(0, 0);
              char tBuf[17];
              snprintf(tBuf, sizeof(tBuf), "%-15s", ui_data[cur_fld_no].topRow);
              lcd.print(tBuf);
              strcpy(present_topRow, ui_data[cur_fld_no].topRow);
            }
          }

          // --- LCD Heartbeat (Bonus) ---
          static bool heartState = false;
          static unsigned long lastHeartbeat = 0;
          if (millis() - lastHeartbeat > 1000) {
            lastHeartbeat = millis();
            heartState = !heartState;
            lcd.setCursor(15, 0);
            lcd.print(heartState ? "." : " ");
          }

          if (cur_fld_no == FLD_SIM_STATUS) { // Signal Strength & SIM Info
            int rssi = signal_strength;
            bool sigValid = !(rssi == 0 || rssi == 99 || rssi == -114);
            bool carValid =
                !(strlen(carrier) == 0 || strcmp(carrier, "NA") == 0 ||
                  strcmp(carrier, "0") == 0);

            char topBuffer[17];
            char sigStr[17];

            if (!carValid || !sigValid) {
              strcpy(topBuffer, "SIGNAL          ");
              strcpy(sigStr, "FETCHING...     ");
            } else {
              snprintf(topBuffer, 17, "SIM:%s", carrier);
              if (rssi > 0) {
                strcpy(sigStr, "SIGNAL: WEAK    ");
              } else {
                snprintf(sigStr, 17, "%d dBm         ", rssi);
              }
            }

            if (strcmp(topBuffer, present_topRow) != 0) {
              lcd.setCursor(0, 0);
              lcd.print(topBuffer);
              for (int k = strlen(topBuffer); k < 16; k++)
                lcd.print(" ");
              strcpy(ui_data[cur_fld_no].topRow, topBuffer);
              strcpy(present_topRow, topBuffer);
            }

            if (strcmp(sigStr, present_bottomRow) != 0) {
              lcd.setCursor(0, 1);
              lcd.print(sigStr);
              strcpy(present_bottomRow, sigStr);
              strcpy(ui_data[cur_fld_no].bottomRow, sigStr);
            }
          } else if (cur_fld_no == FLD_REGISTRATION) {
            char disp_reg[17];
            if (strcmp(reg_status, "NA") == 0) {
              strcpy(disp_reg, "FETCHING...     ");
            } else {
              strncpy(disp_reg, reg_status, 16);
              disp_reg[16] = 0;
            }

            if (strcmp(disp_reg, present_bottomRow) != 0) {
              lcd.setCursor(0, 1);
              lcd.print(disp_reg);
              for (int k = strlen(disp_reg); k < 16; k++)
                lcd.print(" ");
              strcpy(present_bottomRow, disp_reg);
              strcpy(ui_data[cur_fld_no].bottomRow, disp_reg);
            }
          } else if ((cur_fld_no == FLD_RF_CALIB) &&
                     (calib_flag != 1)) { // RF Calib
            if (strcmp(calib_text, present_bottomRow)) {
              lcd.setCursor(0, 1);
              lcd.print(calib_text);
              strcpy(present_bottomRow, calib_text);
            }
          } else if (cur_fld_no == FLD_BATTERY) { // Battery
            static unsigned long bat_timer = 0;
            if (millis() - bat_timer > 2000) {
              bat_timer = millis();
              li_bat =
                  adc1_get_raw(ADC1_CHANNEL_5); // Changed from analogRead(33)
              li_bat_val = li_bat * 0.0010915;
              snprintf(battery, sizeof(battery), "%04.1f", li_bat_val);
              bat_val = li_bat_val;
            }

            if (strcmp(battery, present_bottomRow) != 0) {
              lcd.setCursor(0, 1);
              lcd.print(battery);
              strcpy(present_bottomRow, battery);
            }
          } else if (cur_fld_no == FLD_SOLAR) { // Solar
            static unsigned long solar_timer = 0;
            if (millis() - solar_timer > 2000) {
              solar_timer = millis();
              if (!wifi_active) {
                // Using ADC2 (shared with WiFi)
                int solar_raw;
                if (adc2_get_raw(ADC2_CHANNEL_8, ADC_WIDTH_BIT_12,
                                 &solar_raw) == ESP_OK) {
                  solar = solar_raw;
                  solar_val = (solar / 4096.0) * 3.6 * 7.2;
                  snprintf(solar_sense, sizeof(solar_sense), "%04.1f",
                           solar_val);
                }
              }
            }

            if (strcmp(solar_sense, present_bottomRow) != 0) {
              lcd.setCursor(0, 1);
              lcd.print(solar_sense);
              strcpy(present_bottomRow, solar_sense);
            }
          }

          // Handle "PRESS SET" for Turn Off LCD and Wifi
          else if (cur_fld_no == FLD_LCD_OFF || cur_fld_no == FLD_WIFI_ENABLE) {
            if (cur_fld_no == FLD_WIFI_ENABLE && wifi_active) {
              if (strcmp("ACTIVE", present_bottomRow) != 0) {
                lcd.setCursor(0, 1);
                lcd.print("ACTIVE          ");
                strcpy(present_bottomRow, "ACTIVE");
              }
            } else {
              if (strcmp("PRESS SET       ", present_bottomRow) != 0) {
                lcd.setCursor(0, 1);
                lcd.print("PRESS SET       ");
                strcpy(present_bottomRow, "PRESS SET       ");
              }
            }
          }

          else {
            if (strcmp(ui_data[cur_fld_no].bottomRow, present_bottomRow)) {
              lcd.setCursor(0, 1);
              char bBuf[17];
              snprintf(bBuf, sizeof(bBuf), "%-16s",
                       ui_data[cur_fld_no].bottomRow);
              lcd.print(bBuf);
              strcpy(present_bottomRow, ui_data[cur_fld_no].bottomRow);
            }
          }

          xSemaphoreGive(i2cMutex);
        }
      }

      // --- UNIFIED KEYPAD HANDLING ---
      if ((key = keypad.getKey()) != NULL) {
        lcdkeypad_start = 1; // KEEP UI ACTIVE ON ANY KEY PRESS

        // Track key activity for adaptive polling (power optimization)
        extern unsigned long last_key_time;
        last_key_time = millis();

        // TIMEOUT LOGIC:
        // Field Calib (calib_mode == 0): Do NOT refresh. Timer runs down from
        // start (5 mins). Test Mode (calib_mode != 0): Refresh to Infinite (30
        // mins). Normal Mode: Refresh to 2 mins.

        if (calib_flag > 0) {
          if (calib_mode == 0) {
            // FIELD CALIB: DO NOT RESET TIMER!
            // Just let the key be processed (CLEAR/SET logic below will handle
            // exit)
          } else {
            // TEST MODE: Refresh to 30 mins
            timerAlarm(lcd_timer, 1800000000, false,
                       0); // 30 mins for calibration
            timerWrite(lcd_timer, 0);
          }
        } else {
          // NORMAL MODE: Refresh to 2 mins
          timerAlarm(lcd_timer, 120000000, false, 0); // 2 mins standard
          timerWrite(lcd_timer, 0);
        }

        // This block was moved from gprs.ino to here to avoid duplicate
        // HTTPACTION and to ensure it's triggered by a key press in normal
        // mode. It's placed after the timer reset to ensure the display stays
        // on for the duration of the HTTP action.
        // on for the duration of the HTTP action.

        int i = (int)key - 48; // Converting from ASCII to INT
        debugln();
        debug("Key: ");
        debug(key_name[i]);
        debug(", i: ");
        debug(i);
        debug(", Fld: ");
        debugln(cur_fld_no);

        // entry_in_progress vars
        int len = 0;
        if (ui_data[cur_fld_no].fieldType == eNumeric ||
            ui_data[cur_fld_no].fieldType == eAlphaNum) {
          len = strlen(inpCharSet[ui_data[cur_fld_no].fieldType]);
        }

        switch (i) {
        // case 1: CLEAR / BACK
        case 1:
          if (cur_mode == eEditOn) {
            // Special case for FLD_LOG to reset default time
            if (cur_fld_no == FLD_LOG) {
              char defaultTimeStr[17];
              snprintf(defaultTimeStr, sizeof(defaultTimeStr),
                       "%04d%02d%02d %02d:%02d", current_year, current_month,
                       current_day, record_hr, record_min);
              strcpy(ui_data[FLD_LOG].bottomRow, defaultTimeStr);
            } else if (cur_fld_no == FLD_RF_RES) {
              rf_res_edit_state = 0;
              strcpy(ui_data[FLD_RF_RES].topRow, "RF RESOLUTION");
            } else if (cur_fld_no == FLD_DELETE_DATA) {
              strcpy(ui_data[FLD_DELETE_DATA].topRow, "DELETE DATA?");
            }
            cur_mode = eEditOff;
            show_now = 1;
          } else {
            // Cancel from Caliabration Selection (Mode Selection) or Active
            // Calibration
            if (calib_flag == 10 || calib_flag == 1) {
              calib_flag = 0;
              calib_mode_flag.val = 0; // Stop ULP counting if it was running
              show_now = 1;
              debugln("Calibration Canceled via CLEAR key.");

              // Explicitly reset timer to 2 mins standard idle
              timerAlarm(lcd_timer, 120000000, false, 0); // Reset to 2 mins
              timerWrite(lcd_timer, 0);
              timerRestart(lcd_timer);
            }
          }
          break;

        case 2:
          if (cur_mode == eEditOn) {
            cur_char_no = (cur_char_no + 1 + len) % len;
            if (cur_pos_no < 16) {
              input_buf[cur_pos_no] =
                  inpCharSet[ui_data[cur_fld_no].fieldType][cur_char_no];
            }
          } else {
            // Loop until we find a visible field
            int start_fld = cur_fld_no;
            do {
              cur_fld_no = (cur_fld_no + 1) % FLD_COUNT;
            } while (!isFieldVisible(cur_fld_no) && cur_fld_no != start_fld);
            show_now = 1;

            if (cur_fld_no == FLD_LOG) {
              char defStr[17];
              snprintf(defStr, sizeof(defStr), "%04d%02d%02d %02d:%02d",
                       current_year, current_month, current_day, record_hr,
                       record_min);
              strcpy(ui_data[FLD_LOG].bottomRow, defStr);
            }
          }
          show_now = 1;
          break;

        // case 3: SET / ACTION
        case 3:
          if (cur_fld_no == FLD_LCD_OFF) {
            debugln("Turning off the display ...");
            lcdkeypad_start = 0;
            digitalWrite(32, LOW);

            // Link Wi-Fi shutdown to manual LCD shutdown
            if (wifi_active) {
              WiFi.softAPdisconnect(true);
              wifi_active = false;
              webServerStarted = false;
              debugln("LCD Manually OFF -> Wi-Fi Disabled");
              strcpy(ui_data[FLD_WIFI_ENABLE].bottomRow, "ENABLE WIFI     ");
            }

            // Check if we can go to deep sleep
            bool task_running =
                (sync_mode == eSMSStart || sync_mode == eGPSStart ||
                 sync_mode == eHttpTrigger || sync_mode == eHttpBegin ||
                 sync_mode == eHttpStarted);

            int m = current_min;
            bool within_window =
                (m == 58 || m == 59 || m == 0 || m == 14 || m == 15 ||
                 m == 29 || m == 30 || m == 44 || m == 45);

            if (!task_running && !within_window) {
              debugln("Manual LCD OFF -> Force Deep Sleep");
              start_deep_sleep();
            } else {
              debugln(
                  "Manual LCD OFF -> Keeping Awake (Task Active or In Window)");
            }
            vTaskDelay(100);
          } else if (cur_fld_no == FLD_SEND_STATUS) {
            if ((sync_mode == eSyncModeInitial) || (sync_mode == eSMSStop) ||
                (sync_mode == eHttpStop) || (sync_mode == eExceptionHandled)) {
              debugln("Button: Triggering eSMSStart");
              send_status = 1;
              sync_mode = eSMSStart; // Trigger GPRS SMS/Status task
              strcpy(ui_data[FLD_SEND_STATUS].bottomRow, "SENDING...");
              show_now = 1;
            } else {
              // Provide feedback when busy
              debugln("Button: SEND STATUS ignored - HTTP in progress");
              strcpy(ui_data[FLD_SEND_STATUS].bottomRow, "BUSY-TRY LATER");
              show_now = 1;
              vTaskDelay(2000);
              strcpy(ui_data[FLD_SEND_STATUS].bottomRow, "YES ?           ");
              show_now = 1;
            }
          } else if (cur_fld_no == FLD_SEND_GPS) {
            if ((sync_mode == eSyncModeInitial) || (sync_mode == eSMSStop) ||
                (sync_mode == eHttpStop) || (sync_mode == eExceptionHandled)) {
              debugln("Button: Triggering eGPSStart");
              sync_mode = eGPSStart; // Trigger GPRS GPS task
              strcpy(ui_data[FLD_SEND_GPS].bottomRow, "SENDING...");
              show_now = 1;
            } else {
              debugln("Button: SEND GPS ignored - HTTP in progress");
              strcpy(ui_data[FLD_SEND_GPS].bottomRow, "BUSY-TRY LATER");
              show_now = 1;
              vTaskDelay(2000);
              strcpy(ui_data[FLD_SEND_GPS].bottomRow, "YES ?           ");
              show_now = 1;
            }
          } else if (cur_fld_no == FLD_RF_CALIB) {
            if (calib_flag == 0) {
              // Mode is now strictly defined by the global flag
              calib_mode = 0; // ALWAYS Field Calibration (User Request)

              calib_start_time = millis();
              calib_count.val = 0; // PHYSICAL SEPARATION: Reset calib bucket
              calib_mode_flag.val =
                  1; // PHYSICAL SEPARATION: Start calib counting
              calib_count_rf = 0;
              calib_flag = 1; // Start
              calib_header_drawn = 0;

              // Set initial timeout based on mode
              // Field Calib: 5 mins allowed for test
              // Field Calib: 5 mins allowed (User Correction)
              timerAlarm(lcd_timer, 300000000, false, 0); // 5 mins
              timerWrite(lcd_timer, 0);                   // Reset count
              timerRestart(lcd_timer);

              lcd.clear();
              vTaskDelay(300);
            } else if (calib_flag == 1) {
              calib_flag = 2; // Stop
              vTaskDelay(300);
            }
          } else if (cur_fld_no == FLD_SD_COPY) {
            copyFilesFromSPIFFSToSD("/");
          } else if (cur_fld_no == FLD_WIFI_ENABLE) {
            if (wifi_active) {
              WiFi.mode(WIFI_OFF);
              wifi_active = false;
              webServerStarted = false;
            } else {
              wifi_active = true;
              last_wifi_activity_time = millis();
#if ENABLE_WEBSERVER == 1
              if (!webServerStarted) {
                xTaskCreatePinnedToCore(webServer, "webServerTask", 8192, NULL,
                                        3, &webServer_h, 0);
                webServerStarted = true;
              }
#endif
            }
          } else {
            if (cur_mode == eEditOff) {
              // Only allow entering Edit Mode for specific configuration fields
              if (cur_fld_no == FLD_STATION || cur_fld_no == FLD_DATE ||
                  cur_fld_no == FLD_TIME || cur_fld_no == FLD_RF_RES ||
                  cur_fld_no == FLD_DELETE_DATA || cur_fld_no == FLD_LOG) {

                if ((cur_fld_no == FLD_RF_RES && rf_res_edit_state == 0) ||
                    (cur_fld_no == FLD_DELETE_DATA)) {
                  if (cur_fld_no == FLD_RF_RES) {
                    strcpy(ui_data[FLD_RF_RES].topRow, "ENTER PWD");
                    rf_res_edit_state = 1;
                    strcpy(input_buf, "0000");
                  } else {
                    strcpy(ui_data[FLD_DELETE_DATA].topRow, "CONFIRM?");
                    strcpy(input_buf, "Yes ");
                  }
                } else {
                  // Special handling for Station ID: Start with cleared field
                  if (cur_fld_no == FLD_STATION) {
                    strcpy(input_buf, "                "); // 16 spaces
                  } else {
                    strcpy(input_buf, ui_data[cur_fld_no].bottomRow);
                  }
                }
                cur_mode = eEditOn;
                cur_pos_no = 0;
                cur_char_no = 0;

                char c[2] = {input_buf[0], 0};
                char *ptr =
                    strstr(inpCharSet[ui_data[cur_fld_no].fieldType], c);
                if (ptr)
                  cur_char_no = ptr - inpCharSet[ui_data[cur_fld_no].fieldType];
              } else {
                debugln("Interaction ignored: Field is Read-Only.");
              }
            } else {
              if (cur_fld_no == FLD_RF_RES) {
                if (rf_res_edit_state == 1) {
                  if (strcmp(input_buf, "2000") == 0) {
                    rf_res_edit_state = 2;
                    strcpy(ui_data[FLD_RF_RES].topRow, "SET RESOLUTION");
                    snprintf(input_buf, sizeof(input_buf), "%0.2f",
                             RF_RESOLUTION);
                    cur_pos_no = 0;
                    cur_char_no = 0;
                    // Ensure we stay in Edit Mode to edit the value
                    cur_mode = eEditOn;
                    // Wait, logic below says cur_mode = eEditOff normally?
                    // original logic said:
                    // } else { ... cur_mode = eEditOff; }
                    // If state=2, we want to EDIT.
                    // So we should NOT set eEditOff here.
                    // But we fall through to 'cur_mode = eEditOff' below? No.
                    // The if/else blocks handle it.
                    // In state 2, user enters value.
                    // Pressing Set again triggers state 2 logic (save).
                  } else {
                    rf_res_edit_state = 0;
                    strcpy(ui_data[FLD_RF_RES].topRow, "RF RESOLUTION");
                    cur_mode = eEditOff;
                  }
                } else if (rf_res_edit_state == 2) {
                  // Robust parsing: replace , with . just in case user uses it
                  for (int x = 0; x < 16; x++) {
                    if (input_buf[x] == ',')
                      input_buf[x] = '.';
                  }
                  float new_res = atof(input_buf);
                  debug("SET RESOLUTION: ");
                  debugln(input_buf);

                  if (fabs(new_res - 0.25) < 0.05 ||
                      fabs(new_res - 0.50) < 0.05) {
                    RF_RESOLUTION = (fabs(new_res - 0.25) < 0.05) ? 0.25 : 0.50;
                    File resFile = SPIFFS.open("/rf_res.txt", FILE_WRITE);
                    if (resFile) {
                      resFile.print(RF_RESOLUTION);
                      resFile.close();
                    }
                    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) ==
                        pdTRUE) {
                      lcd.clear();
                      lcd.setCursor(0, 0);
                      lcd.print("SET TO ");
                      lcd.print(RF_RESOLUTION, 2);
                      lcd.setCursor(0, 1);
                      lcd.print("RESTARTING...");
                      xSemaphoreGive(i2cMutex);
                    }
                    debugln("Resolution updated. Restarting...");
                    vTaskDelay(3000);
                    ESP.restart();
                  } else {
                    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) ==
                        pdTRUE) {
                      lcd.clear();
                      lcd.setCursor(0, 0);
                      lcd.print("VAL: 0.25 / 0.50");
                      xSemaphoreGive(i2cMutex);
                    }
                    vTaskDelay(2000);
                  }
                  rf_res_edit_state = 0;
                  strcpy(ui_data[FLD_RF_RES].topRow, "RF RESOLUTION");
                  cur_mode = eEditOff;
                }
              } else {
                strcpy(ui_data[cur_fld_no].bottomRow, input_buf);
                cur_mode = eEditOff;

                if (cur_fld_no == FLD_STATION) {
                  // Store old station name before changing
                  char old_station[16];
                  strcpy(old_station, station_name);

                  // Trim leading and trailing spaces from input_buf
                  String trimmed = String(input_buf);
                  trimmed.trim();

                  // Requirement: if it's numeric "00XXXX", treat as "XXXX"
                  if (trimmed.length() == 6 && isDigitStr(trimmed.c_str()) &&
                      trimmed.startsWith("00")) {
                    trimmed = trimmed.substring(2);
                  }

                  // Update to new station name
                  strncpy(station_name, trimmed.c_str(),
                          sizeof(station_name) - 1);
                  station_name[sizeof(station_name) - 1] = '\0';
                  strcpy(ftp_station, station_name);

                  // Save to SPIFFS
                  File file = SPIFFS.open("/station.doc", FILE_WRITE);
                  if (file) {
                    file.print(station_name);
                    file.close();
                  }

                  // Save to NVS
                  Preferences prefs;
                  prefs.begin("sys-config", false);
                  prefs.putString("station", station_name);
                  prefs.end();

                  // Cleanup old station files if station ID actually changed
                  if (strcmp(old_station, station_name) != 0 &&
                      strlen(old_station) > 0) {
                    debugln(
                        "[UI] Station ID changed. Cleaning up old files...");

                    File root = SPIFFS.open("/");
                    File file = root.openNextFile();
                    int deleted_count = 0;

                    while (file) {
                      String fileName = file.name();

                      // Delete files matching old station pattern
                      // (OLDSTATION_YYYYMMDD.txt)
                      if (fileName.startsWith("/" + String(old_station) +
                                              "_")) {
                        String fullPath = fileName.startsWith("/")
                                              ? fileName
                                              : "/" + fileName;
                        debug("Deleting old station file: ");
                        debugln(fullPath);
                        SPIFFS.remove(fullPath);
                        deleted_count++;
                      }

                      file = root.openNextFile();
                    }

                    debugf1("[UI] Deleted %d old station files\n",
                            deleted_count);

                    // Show confirmation on LCD
                    if (xSemaphoreTake(i2cMutex,
                                       pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
                        pdTRUE) {
                      lcd.clear();
                      lcd.setCursor(0, 0);
                      lcd.print("Station Changed");
                      lcd.setCursor(0, 1);
                      if (deleted_count > 0) {
                        char msg[17];
                        snprintf(msg, sizeof(msg), "Deleted %d files",
                                 deleted_count);
                        lcd.print(msg);
                      } else {
                        lcd.print("No old data");
                      }
                      xSemaphoreGive(i2cMutex);
                    }
                    vTaskDelay(2000);

                    // NEW: Cleanup for new station
                    debugln("[UI] Station Changed: Deleting Calib & Unsent "
                            "files...");
                    SPIFFS.remove("/calib.txt");
                    SPIFFS.remove("/unsent.txt");
                    SPIFFS.remove("/ftpunsent.txt");
                    SPIFFS.remove("/unsent_pointer.txt");

                    // CRITICAL: Delete GPS cache to force fresh fix for new
                    // location
                    SPIFFS.remove("/gps_coords.txt");
                    debugln("[UI] Station Changed: Deleted GPS cache (will get "
                            "fresh fix)");
                    lati = 0; // Clear in-memory coordinates
                    longi = 0;

                    // Reset internal calibration state fully
                    calib_sts = 0;
                    calib_day = 0;
                    calib_month = 0;
                    calib_year = 0;
                    strcpy(ui_data[FLD_RF_CALIB].bottomRow, "NOT CALIBRATED");

                    // NEW: Automatic Health Report with GPS after Station ID
                    // Change
                    debugln("[UI] Station Changed: Scheduling Automatic GPS & "
                            "Health Report");
                    sync_mode = eStartupGPS;
                    cur_fld_no = FLD_SEND_GPS; // Switch view to GPS Status
                    strcpy(ui_data[FLD_SEND_GPS].bottomRow, "INITIALIZING...");
                  }
                } else if (cur_fld_no == FLD_DATE) {
                  sscanf(ui_data[FLD_DATE].bottomRow, "%02d-%02d-%04d",
                         &current_day, &current_month, &current_year);
                  if (xSemaphoreTake(i2cMutex,
                                     pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
                      pdTRUE) {
                    rtc.adjust(DateTime(current_year, current_month,
                                        current_day, current_hour, current_min,
                                        0));
                    record_hr = current_hour;
                    record_min = current_min;
                    // Rounding for signature (Slot START time)
                    if (record_min < 15)
                      record_min = 0;
                    else if (record_min < 30)
                      record_min = 15;
                    else if (record_min < 45)
                      record_min = 30;
                    else
                      record_min = 45;

                    File fileTemp3 = SPIFFS.open("/signature.txt", FILE_WRITE);
                    if (fileTemp3) {
                      snprintf(signature, sizeof(signature),
                               "%04d-%02d-%02d,%02d:%02d", current_year,
                               current_month, current_day, record_hr,
                               record_min);
                      fileTemp3.print(signature);
                      fileTemp3.close();
                    }
                    rtcTimeChanged = true;
                    xSemaphoreGive(i2cMutex);
                  }
                } else if (cur_fld_no == FLD_TIME) {
                  sscanf(ui_data[FLD_TIME].bottomRow, "%02d:%02d",
                         &current_hour, &current_min);
                  if (xSemaphoreTake(i2cMutex,
                                     pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
                      pdTRUE) {
                    rtc.adjust(DateTime(current_year, current_month,
                                        current_day, current_hour, current_min,
                                        0));
                    record_hr = current_hour;
                    record_min = current_min;
                    // Rounding for signature
                    if (record_min < 15)
                      record_min = 0;
                    else if (record_min < 30)
                      record_min = 15;
                    else if (record_min < 45)
                      record_min = 30;
                    else
                      record_min = 45;

                    File fileTemp4 = SPIFFS.open("/signature.txt", FILE_WRITE);
                    if (fileTemp4) {
                      snprintf(signature, sizeof(signature),
                               "%04d-%02d-%02d,%02d:%02d", current_year,
                               current_month, current_day, record_hr,
                               record_min);
                      fileTemp4.print(signature);
                      fileTemp4.close();
                    }
                    rtcTimeChanged = true;
                    xSemaphoreGive(i2cMutex);
                  }
                } else if (cur_fld_no == FLD_DELETE_DATA) {
                  if (strstr(input_buf, "Yes") != NULL) {
                    if (xSemaphoreTake(i2cMutex,
                                       pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) ==
                        pdTRUE) {
                      lcd.clear();
                      lcd.print("Deleting Data");
                      debugln("[UI] Deleting SPIFFS data files...");

                      // 1. Explicitly remove global unsent buffers
                      SPIFFS.remove("/unsent.txt");
                      SPIFFS.remove("/ftpunsent.txt");
                      SPIFFS.remove("/unsent_pointer.txt");

                      // 2. Enumerate and remove ONLY data files, preserving
                      // config
                      File root = SPIFFS.open("/");
                      File file = root.openNextFile();
                      while (file) {
                        String fileName = file.name();
                        String fullPath = fileName.startsWith("/")
                                              ? fileName
                                              : "/" + fileName;

                        // List of files to PRESERVE (Configuration & State)
                        if (fullPath == "/apn_config.txt" ||
                            fullPath == "/rf_fw.txt" ||
                            fullPath == "/rf_res.txt" ||
                            fullPath == "/signature.txt" ||
                            fullPath == "/prevWindSpeed.txt" ||
                            fullPath == "/calib.txt" ||
                            fullPath == "/calib_test.txt" ||
                            fullPath == "/sd_fw_md5.txt") {
                          debug("Preserving Config: ");
                          debugln(fullPath);
                        }
                        // Delete logs and records (.txt, .swd, .kwd, .krd)
                        else if (fullPath.endsWith(".txt") ||
                                 fullPath.endsWith(".swd") ||
                                 fullPath.endsWith(".kwd") ||
                                 fullPath.endsWith(".krd")) {
                          debug("Deleting Data: ");
                          debugln(fullPath);
                          SPIFFS.remove(fullPath);
                        }
                        file = root.openNextFile();
                      }
                      lcd.clear();
                      lcd.print("COMPLETED!");
                      vTaskDelay(1000);
                      strcpy(ui_data[FLD_DELETE_DATA].topRow, "DELETE DATA?");
                      strcpy(ui_data[FLD_DELETE_DATA].bottomRow, "Yes ?");
                      xSemaphoreGive(i2cMutex);
                    }
                  } else {
                    strcpy(ui_data[FLD_DELETE_DATA].topRow, "DELETE DATA?");
                    strcpy(ui_data[FLD_DELETE_DATA].bottomRow, "Yes ?");
                  }
                } else if (cur_fld_no == FLD_LOG) {
                  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("SEARCHING...");
                    xSemaphoreGive(i2cMutex);
                  }

                  // dateTimeStr is "YYYYMMDD HH:MM"
                  char dateTimeStr[17];
                  strcpy(dateTimeStr, ui_data[FLD_LOG].bottomRow);

                  char year_s[5], month_s[3], day_s[3], hour_s[3], min_s[3];
                  strncpy(year_s, dateTimeStr, 4);
                  year_s[4] = 0;
                  strncpy(month_s, dateTimeStr + 4, 2);
                  month_s[2] = 0;
                  strncpy(day_s, dateTimeStr + 6, 2);
                  day_s[2] = 0;
                  strncpy(hour_s, dateTimeStr + 9, 2);
                  hour_s[2] = 0;
                  strncpy(min_s, dateTimeStr + 12, 2);
                  min_s[2] = 0;

                  int hr = atoi(hour_s);
                  int mn = atoi(min_s);
                  int dy = atoi(day_s);
                  int mo = atoi(month_s);
                  int yr = atoi(year_s);

                  int sample = (hr * 4 + mn / 15 + 61) % 96;

                  // RF Day Logic: Samples 0-60 (8:45 AM to Midnight) belong
                  // to the NEXT day's file
                  if (sample <= 60) {
                    int no_of_days[13] = {0,  31, 28, 31, 30, 31, 30,
                                          31, 31, 30, 31, 30, 31};
                    dy++;
                    int days_in_mo = no_of_days[mo];
                    if (mo == 2 && yr % 4 == 0)
                      days_in_mo = 29;
                    if (dy > days_in_mo) {
                      dy = 1;
                      mo++;
                      if (mo > 12) {
                        mo = 1;
                        yr++;
                      }
                    }
                  }

                  char log_filename[50];
                  snprintf(log_filename, sizeof(log_filename),
                           "/%s_%04d%02d%02d.txt", station_name, yr, mo, dy);

                  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
                    lcd.clear();
                    if (!SPIFFS.exists(log_filename)) {
                      // Fallback check for legacy "00" prefix or normalized
                      // station ID
                      char fallback_filename[50];
                      if (strncmp(station_name, "00", 2) == 0) {
                        snprintf(fallback_filename, sizeof(fallback_filename),
                                 "/%s_%04d%02d%02d.txt", station_name + 2, yr,
                                 mo, dy);
                      } else {
                        snprintf(fallback_filename, sizeof(fallback_filename),
                                 "/00%s_%04d%02d%02d.txt", station_name, yr, mo,
                                 dy);
                      }

                      if (SPIFFS.exists(fallback_filename)) {
                        strcpy(log_filename, fallback_filename);
                      } else {
                        lcd.print("NO LOG FILE");
                        debug("Search Failed. Searched for: ");
                        debug(log_filename);
                        debug(" and fallback: ");
                        debugln(fallback_filename);
                        // DO NOT return; - this would kill the keypad task!
                      }
                    }

                    // Only attempt to open if the file was found or exists
                    if (SPIFFS.exists(log_filename)) {
                      File f = SPIFFS.open(log_filename, FILE_READ);
                      if (!f) {
                        lcd.print("OPEN FAILED");
                      } else {
                        char line_buf[100];
                        bool found = false;
                        while (f.available()) {
                          String l = f.readStringUntil('\n');
                          int sNo = l.substring(0, 2).toInt();
                          if (sNo == sample) {
                            strcpy(line_buf, l.c_str());
                            found = true;
                            break;
                          }
                        }
                        f.close();

                        if (!found) {
                          lcd.print("DATA NOT FOUND");
                        } else {
                          // SYSTEM 0:
                          // 01,2026-02-18,11:30,000.00,000.00,-111,04.2
                          // SYSTEM 1:
                          // 11,2026-02-18,11:30,25.32,38.42,00.38,105,-055,04.2
                          // SYSTEM 2:
                          // 11,2026-02-18,11:30,001.00,25.32,38.42,00.10,105,-055,04.2

                          float rf = 0, temp = 0, hum = 0, aws = 0;
                          int wd = 0;
                          char dbuf1[16], dbuf2[16];

                          if (SYSTEM == 0) {
                            float instRF, cumRF;
                            sscanf(line_buf, "%*d,%*[^,],%*[^,],%f,%f", &instRF,
                                   &cumRF);
                            lcd.setCursor(0, 0);
                            lcd.print("CUM_RF:");
                            lcd.setCursor(0, 1);
                            lcd.print(cumRF, 2);
                          } else if (SYSTEM == 1) {
                            sscanf(line_buf, "%*d,%*[^,],%*[^,],%f,%f,%f,%d",
                                   &temp, &hum, &aws, &wd);
                            lcd.setCursor(0, 0);
                            snprintf(dbuf1, sizeof(dbuf1), "T:%-4.1f H:%-4.1f",
                                     temp, hum);
                            lcd.print(dbuf1);
                            lcd.setCursor(0, 1);
                            snprintf(dbuf2, sizeof(dbuf2), "AWS:%-4.1f WD:%-3d",
                                     aws, wd);
                            lcd.print(dbuf2);
                          } else if (SYSTEM == 2) {
                            sscanf(line_buf, "%*d,%*[^,],%*[^,],%f,%f,%f,%f,%d",
                                   &rf, &temp, &hum, &aws, &wd);
                            lcd.setCursor(0, 0);
                            snprintf(dbuf1, sizeof(dbuf1),
                                     "RF:%-3.1fT:%-4.1fH:%-2.0f", rf, temp,
                                     hum);
                            lcd.print(dbuf1);
                            lcd.setCursor(0, 1);
                            snprintf(dbuf2, sizeof(dbuf2), "AWS:%-4.1f WD:%-3d",
                                     aws, wd);
                            lcd.print(dbuf2);
                          }
                        }
                      }
                    }
                    xSemaphoreGive(i2cMutex);
                    vTaskDelay(5000); // Allow user to read
                  }
                }
              }
            }
          }
          show_now = 1; // Trigger display update
          break;

        // case 4: LEFT
        case 4:
          if (cur_mode == eEditOn) {
            if (cur_pos_no > 0)
              cur_pos_no--;
            char c1[2] = {input_buf[cur_pos_no], 0};
            char *ptr = strstr(inpCharSet[ui_data[cur_fld_no].fieldType], c1);
            if (ptr)
              cur_char_no = ptr - inpCharSet[ui_data[cur_fld_no].fieldType];
          }
          show_now = 1;
          break;

        // case 5: DOWN
        case 5:
          if (cur_mode == eEditOn) {
            cur_char_no = (cur_char_no - 1 + len) % len;
            if (cur_pos_no < 16) {
              input_buf[cur_pos_no] =
                  inpCharSet[ui_data[cur_fld_no].fieldType][cur_char_no];
            }
          } else {
            if (cur_fld_no == FLD_RF_CALIB && calib_flag == 10) {
              // Toggle removed: User only wants Field Calibration
              show_now = 1;
            } else {
              // Loop until we find a visible field
              int start_fld = cur_fld_no;
              do {
                cur_fld_no = (cur_fld_no + FLD_COUNT - 1) % FLD_COUNT;
              } while (!isFieldVisible(cur_fld_no) && cur_fld_no != start_fld);
              if (cur_fld_no == FLD_LOG) {
                int p_min = (current_min / 15) * 15;
                int p_hr = current_hour;
                int d_day = rf_cls_dd;
                int d_month = rf_cls_mm;
                int d_year = rf_cls_yy;

                if (d_day <= 0 || (d_day == current_day &&
                                   (current_hour > 8 || (current_hour == 8 &&
                                                         current_min >= 30)))) {
                  if (d_day <= 0) {
                    d_day = current_day;
                    d_month = current_month;
                    d_year = current_year;
                  }
                  if ((current_hour > 8) ||
                      (current_hour == 8 && current_min >= 30)) {
                    d_day++;
                    int daysInMonth[] = {0,  31, 28, 31, 30, 31, 30,
                                         31, 31, 30, 31, 30, 31};
                    if ((d_year % 4 == 0 && d_year % 100 != 0) ||
                        (d_year % 400 == 0))
                      daysInMonth[2] = 29;
                    if (d_day > daysInMonth[d_month]) {
                      d_day = 1;
                      d_month++;
                      if (d_month > 12) {
                        d_month = 1;
                        d_year++;
                      }
                    }
                  }
                }
                char defStr[17];
                snprintf(defStr, sizeof(defStr), "%04d%02d%02d %02d:%02d",
                         d_year, d_month, d_day, p_hr, p_min);
                strcpy(ui_data[FLD_LOG].bottomRow, defStr);
              }
            }
          }
          show_now = 1;
          break;

        // case 6: RIGHT
        case 6:
          if (cur_mode == eEditOn) {
            if (cur_pos_no < 15)
              cur_pos_no++;
            char c1[2] = {input_buf[cur_pos_no], 0};
            char *ptr = strstr(inpCharSet[ui_data[cur_fld_no].fieldType], c1);
            if (ptr)
              cur_char_no = ptr - inpCharSet[ui_data[cur_fld_no].fieldType];
          }
          show_now = 1;
          break;
        }

      } // End of if key != NULL
    }   // End of checking lcdkeypad_start == 1

    esp_task_wdt_reset();

    // Adaptive polling: Fast when active, slower when idle (power
    // optimization)
    static bool recent_activity = false;

    if (lcdkeypad_start) {
      // Check if there was recent key activity (within last 2 seconds)
      if (millis() - last_key_time < 2000) {
        vTaskDelay(5); // Fast polling when user is actively pressing keys
        recent_activity = true;
      } else {
        vTaskDelay(20); // Slower polling when idle - saves power
        if (recent_activity) {
          recent_activity = false;
          // debugln("[UI] Switching to power-save polling");
        }
      }
    } else {
      vTaskDelay(50); // Faster polling (50ms) to ensure EXT0 manual wakeups are
                      // cleanly caught
    }

  } // End of forever loop

} // End of lcdkeypad task
