/*
 * Copyright ©2025 SPATIKA INFORMATION TECHNOLOGIES PVT LTD. All rights reserved
 * Unauthorized use or distribution of this firmware is strictly prohibited and
 * may result in legal action
 */

/*
 * MAJOR ARCHITECTURAL CHANGES (v5.0 vs v3.0):
 * 1. Manual ULP Memory Map: Explicit word offsets (Word 512+) for reliable
 * ULP/CPU sharing.
 * 2. Persistent Scheduler State: State variables (last_processed_idx) in RTC
 * RAM to survive manual wakes.
 * 3. High-Precision Rainfall: All logs and payloads use 2-decimal precision
 * (0.25mm resolution) while maintaining width.
 * 4. Decoupled RF Resolution: Factory Defaults separated from User Settings to
 * prevent resolution reset on reboot.
 * 5. Watchdog & I2C Robustness: WDT integration for all tasks and I2C
 * Mutex/Timeout for sensor bus stability.
 * 6. ULP Autorun: ULP re-loaded and started on every boot (including WDT
 * resets) for continuous counting.
 * 7. Unified Snapshot Logic: Standardized interval processing across RF, TWS,
 * and TWS-RF systems.
 */

#include "globals.h"
#include <driver/adc.h>
#include <rom/rtc.h>

extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                              char *pcTaskName) {
  debug("STACK OVERFLOW in task: ");
  debugln(pcTaskName);
  debugln();
  debug("Restarting ....");
  ESP.restart();
}

int wired = 1; // Integrated is 1 or Standalone LCD module is 0
const int chipSelect = SS;
bool ws_ok = true;
bool wd_ok = true; // WD sensor wired on all KSNDMC_TWS field units

TaskHandle_t scheduler_h;
TaskHandle_t gprs_h; // http,sms,ftp,dota
volatile bool primary_data_delivered = false;
RTC_DATA_ATTR volatile bool skip_primary_http = false; // v5.50: Survive warm resets (volatile Fix)
volatile bool force_ftp = false;
volatile bool force_ftp_daily = false;
char ftp_daily_date[12] = "";
volatile bool force_reboot = false;
volatile bool force_ota = false;
volatile bool force_gps_refresh =
    false; // v7.59: Server-requested GPS re-acquire
volatile bool force_clear_ftp_queue =
    false; // v7.59: Server-requested FTP backlog clear
volatile bool ota_writing_active = false;
volatile bool ota_silent_mode = false; // Rule 43
                                       // v6.88
char ota_cmd_param[128] = "";
RTC_DATA_ATTR int last_cmd_id = 0;
RTC_DATA_ATTR char last_cmd_res[64] = "N/A";
int ota_fail_count = 0;
char ota_fail_reason[48] = "NONE";
bool health_in_progress = false;
TaskHandle_t lcdkeypad_h; // UI
TaskHandle_t tempHum_h;
TaskHandle_t bmeTask_h;
TaskHandle_t windSpeed_h;
TaskHandle_t windDirection_h;
TaskHandle_t rtcRead_h;
#if ENABLE_WEBSERVER == 1
TaskHandle_t webServer_h;
#endif

void IRAM_ATTR ext0_isr() {
  portENTER_CRITICAL_ISR(&timerMux0);
  wakeup_reason_is = ext0;
  portEXIT_CRITICAL_ISR(&timerMux0);
}

// v7.93: This is a reserved block for ULP code.
// It is placed here to prevent newly recovered counts from being wiped by the first-boot
// day-change. Tracking variables like last_processed_sample_idx and
// fresh_boot_check_done are now in globals.h (RTC_DATA_ATTR) for
// persistence.
RTC_DATA_ATTR uint8_t ulp_code_reserve[512] = {0}; // Moved from globals.h (Bug#3 Fix)

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n[BOOT] HELLO! System starting... (Debug Enabled)");

  // Seed the random number generator using internal RNG for better jitter
  srand(esp_random());

  delay(1000);

  // Initialize Mutexes first
  i2cMutex = xSemaphoreCreateMutex();
  if (i2cMutex != NULL)
    xSemaphoreGive(i2cMutex);

  serialMutex = xSemaphoreCreateMutex();
  if (serialMutex != NULL)
    xSemaphoreGive(serialMutex);

  fsMutex = xSemaphoreCreateMutex();
  if (fsMutex != NULL)
    xSemaphoreGive(fsMutex);

  initialize_hw(); // sets CPU to 80MHz internally

  // Record Reset Reason and Increment Uptime
  RESET_REASON rr = rtc_get_reset_reason(0);
  diag_last_reset_reason = (int)rr;
  
  // v7.93: Only wipe counters on HARD Power-On (1). 
  // Preserve on DEEPSLEEP (5) for better testing resilience.
  // v5.50: Also treat EXT_CPU_RESET (14) and SW_CPU_RESET (12) as fresh starts.
  // Reason 12 is triggered by 'DELETE DATA' or manually via ESP.restart().
  if (rr == POWERON_RESET || rr == EXT_CPU_RESET || rr == 12) {
    debugf("[BOOT] Fresh Start (Reason %d). Initializing RTC variables.\n", (int)rr);
    if (last_valid_temp < -20.0 || last_valid_temp > 60.0)
      last_valid_temp = 26.0;
    if (last_valid_hum < 0.0 || last_valid_hum > 100.0)
      last_valid_hum = 65.0;

    total_wind_pulses_32 = 0;
    last_sched_wind_pulses_32 = 0;
    last_raw_wind_count = wind_count.val; // v5.50: Anchor to hw to prevent spike
    total_rf_pulses_32 = 0;
    last_sched_rf_pulses_32 = 0;
    last_raw_rf_count = rf_count.val;  // v5.50: Anchor to hw to prevent spike

    // v5.50 Bug#10 Fix: Reset diagnostic counters on DELETE DATA (SW_CPU_RESET)
    // so they don't carry stale values into fresh logs (explains netcount=42 issue)
    if (rr == 12) {
      strcpy(diag_cdm_status, "PENDING");
      diag_pd_count = 0;
      diag_ndm_count = 0;
      diag_net_data_count = 0;
      diag_net_data_count_prev = 0;
      diag_http_success_count = 0;
      diag_http_success_count_prev = 0;
      diag_ftp_success_count = 0;
      diag_ftp_success_count_prev = 0;
      diag_consecutive_http_fails = 0;
      diag_daily_http_fails = 0;
      diag_http_present_fails = 0;
      diag_http_cum_fails = 0;
      diag_rejected_count = 0;
      diag_sent_mask_cur[0] = 0; diag_sent_mask_cur[1] = 0; diag_sent_mask_cur[2] = 0;
      diag_sent_mask_prev[0] = 0; diag_sent_mask_prev[1] = 0; diag_sent_mask_prev[2] = 0;
      last_processed_sample_idx = -1; // Force scheduler re-entry
      fresh_boot_check_done = false;  // Allow fresh boot validation
      health_last_sent_day = -1;      // v5.52 LOOP-6 FIX: Reset health persistence
      health_last_sent_hour = -1;
      debugln("[BOOT] DELETE DATA detected. All diagnostic counters cleared.");
    }
  } else {
    debugf("[BOOT] Preserving counters for reset reason: %d\n", (int)rr);
  }

  // Pre-load essential system data: Try NVS (Preferences) first for robustness
  Preferences prefs;
  prefs.begin("sys-config", false); // Read-Write mode
  String nvs_station = prefs.getString("station", "");

  if (nvs_station.length() > 0) {
    strncpy(station_name, nvs_station.c_str(), sizeof(station_name) - 1);
    station_name[sizeof(station_name) - 1] = '\0';

    // Requirement: if it's numeric "00XXXX", treat as "XXXX"
    if (strlen(station_name) == 6 && isDigitStr(station_name) &&
        strncmp(station_name, "00", 2) == 0) {
      char temp[16];
      strcpy(temp, station_name + 2);
      strcpy(station_name, temp);
    }
    test_health_every_slot = prefs.getInt("test_health", TEST_HEALTH_DEFAULT);
    debug("Health Mode: ");
    debugln(test_health_every_slot == 1 ? "PULSE (15m)" : "DAILY (11am)");
    
    strcpy(ftp_station, station_name);
  } else {
    // If NVS empty, try SPIFFS (Legacy/First Run)
    if (SPIFFS.exists("/station.doc")) {
      File fileTemp = SPIFFS.open("/station.doc", FILE_READ);
      if (fileTemp) {
        String temp = fileTemp.readStringUntil('\n');
        temp.trim();
        strncpy(station_name, temp.c_str(), sizeof(station_name) - 1);
        station_name[sizeof(station_name) - 1] = '\0';

        if (strlen(station_name) == 6 && isDigitStr(station_name) &&
            strncmp(station_name, "00", 2) == 0) {
          char t[16];
          strcpy(t, station_name + 2);
          strcpy(station_name, t);
        }
        strcpy(ftp_station, station_name);
        fileTemp.close();

        // Migrate to NVS immediately
        prefs.putString("station", station_name);
        debug("Migrated Station ID from SPIFFS to NVS: ");
        debugln(station_name);
      }
    } else if (SPIFFS.exists("/station.txt")) {
      File fileTemp = SPIFFS.open("/station.txt", FILE_READ);
      if (fileTemp) {
        String temp = fileTemp.readStringUntil('\n');
        temp.trim();
        strncpy(station_name, temp.c_str(), sizeof(station_name) - 1);
        station_name[sizeof(station_name) - 1] = '\0';

        if (strlen(station_name) == 6 && isDigitStr(station_name) &&
            strncmp(station_name, "00", 2) == 0) {
          char t[16];
          strcpy(t, station_name + 2);
          strcpy(station_name, t);
        }
        strcpy(ftp_station, station_name);
        fileTemp.close();
        // Migrate to NVS immediately
        prefs.putString("station", station_name);
        debug("Migrated Station ID from legacy station.txt to NVS: ");
        debugln(station_name);
      }
    }
  }
  // --- GPS Location Recovery ---
  loadGPS(); // v5.51: Ensure coordinates are available even after POR

  // --- OTA Failure Tracking (Change namespace) ---
  prefs.end(); // Close sys-config
  prefs.begin("ota-track", false);
  ota_fail_count = prefs.getInt("fail_cnt", 0);
  String last_reason = prefs.getString("fail_res", "NONE");
  strncpy(ota_fail_reason, last_reason.c_str(), sizeof(ota_fail_reason) - 1);
  ota_fail_reason[sizeof(ota_fail_reason) - 1] = '\0';

  String last_ver =
      prefs.getString("last_ver", "6.55"); // Updated default to 6.55
  if (last_ver != FIRMWARE_VERSION) {
    debugln("[BOOT] Version Change! Clearing OTA Fail counters.");
    ota_fail_count = 0;
    strcpy(ota_fail_reason, "NONE");
    prefs.putInt("fail_cnt", 0);
    prefs.putString("fail_res", "NONE");
    prefs.putString("last_ver", FIRMWARE_VERSION);
  }
  prefs.end();

  // If still empty (neither NVS nor SPIFFS had valid data), check SD Card
  if (strlen(station_name) == 0 && sd_card_ok) {
    if (SD.exists("/station.doc")) {
      File fileTemp = SD.open("/station.doc", FILE_READ);
      if (fileTemp) {
        String temp = fileTemp.readStringUntil('\n');
        temp.trim();
        if (temp.length() > 0) {
          strncpy(station_name, temp.c_str(), sizeof(station_name) - 1);
          station_name[sizeof(station_name) - 1] = '\0';

          if (strlen(station_name) == 6 && isDigitStr(station_name) &&
              strncmp(station_name, "00", 2) == 0) {
            char t[16];
            strcpy(t, station_name + 2);
            strcpy(station_name, t);
          }
          strcpy(ftp_station, station_name);
          debug("Loaded Station ID from SD: ");
          debugln(station_name);

          // Save to NVS and SPIFFS for future boots
          // Re-open NVS here (prefs.end() was called above)
          Preferences prefs2;
          prefs2.begin("sys-config", false);
          prefs2.putString("station", station_name);
          prefs2.end();
          File sFile = SPIFFS.open("/station.doc", FILE_WRITE);
          if (sFile) {
            sFile.print(station_name);
            sFile.close();
          }
        }
        fileTemp.close();
      }
    }
  }

  if (strlen(station_name) > 0) {
    // Already loaded
  } else {
    // Create default station.doc if strictly needed or just use default
    // AG: Moved away from data upload dependency.
    debugln("station.doc not found, creating default...");
    File fileTemp = SPIFFS.open("/station.doc", FILE_WRITE);
    if (fileTemp) {
      fileTemp.print("SIT999");
      fileTemp.close();
      strcpy(station_name, "SIT999");
      strcpy(ftp_station, "SIT999");
    }
  }

  // --- Proactive Station Data Migration ---
  // If the station name was normalized (e.g. from 001934 to 1934),
  // proactively rename critical persistence files to maintain continuity.
  if (strlen(station_name) == 4 && isDigitStr(station_name)) {
    char old_prefix_lastrec[32], new_prefix_lastrec[32];
    snprintf(old_prefix_lastrec, sizeof(old_prefix_lastrec),
             "/lastrecorded_00%s.txt", station_name);
    snprintf(new_prefix_lastrec, sizeof(new_prefix_lastrec),
             "/lastrecorded_%s.txt", station_name);
    if (SPIFFS.exists(old_prefix_lastrec) &&
        !SPIFFS.exists(new_prefix_lastrec)) {
      debugln("[BOOT] Migrating lastrecorded file to normalized name...");
      File oldF = SPIFFS.open(old_prefix_lastrec, FILE_READ);
      File newF = SPIFFS.open(new_prefix_lastrec, FILE_WRITE);
      if (oldF && newF) {
        while (oldF.available())
          newF.write(oldF.read());
        oldF.close();
        newF.close();
        SPIFFS.remove(old_prefix_lastrec);
      }
    }
    char old_prefix_closing[32], new_prefix_closing[32];
    snprintf(old_prefix_closing, sizeof(old_prefix_closing),
             "/closingdata_00%s.txt", station_name);
    snprintf(new_prefix_closing, sizeof(new_prefix_closing),
             "/closingdata_%s.txt", station_name);
    if (SPIFFS.exists(old_prefix_closing) &&
        !SPIFFS.exists(new_prefix_closing)) {
      debugln("[BOOT] Migrating closingdata file to normalized name...");
      File oldF = SPIFFS.open(old_prefix_closing, FILE_READ);
      File newF = SPIFFS.open(new_prefix_closing, FILE_WRITE);
      if (oldF && newF) {
        while (oldF.available())
          newF.write(oldF.read());
        oldF.close();
        newF.close();
        SPIFFS.remove(old_prefix_closing);
      }
    }
  }

  if (SPIFFS.exists("/signature.txt")) {
    File fileTemp1 = SPIFFS.open("/signature.txt", FILE_READ);
    if (fileTemp1) {
      String temp1 = fileTemp1.readStringUntil('\n');
      strcpy(signature, temp1.c_str());
      sscanf(signature, "%d-%d-%d,%d:%d", &last_recorded_yy, &last_recorded_mm,
             &last_recorded_dd, &last_recorded_hr, &last_recorded_min);
      fileTemp1.close();
      signature_valid = true;

      // Populate last_logged string for UI display immediately
      snprintf(last_logged, sizeof(last_logged), "%d-%d-%d,%02d:%02d",
               last_recorded_yy, last_recorded_mm, last_recorded_dd,
               last_recorded_hr, last_recorded_min);
    }
  } else {
    File fileTemp1 = SPIFFS.open("/signature.txt", FILE_WRITE);
    if (fileTemp1) {
      snprintf(signature, sizeof(signature), "0000-00-00,00:00\r\n");
      fileTemp1.print(signature);
      fileTemp1.close();
      signature_valid = false;
    }
  }

  // Station Altitude Load (for MSLP calculation) - #1 Fix
  // Only meaningful for KSNDMC_TWS-AP (BME280 pressure sensor enabled)
#if (SYSTEM == 1) || (SYSTEM == 2)
  bool is_ap_unit = (strstr(UNIT, "-AP") != NULL);
  if (SPIFFS.exists("/station_alt.txt")) {
    File altF = SPIFFS.open("/station_alt.txt", FILE_READ);
    if (altF) {
      float loaded_alt = altF.readString().toFloat();
      altF.close();
      if (loaded_alt >= 0.0 && loaded_alt <= 5000.0) {
        station_altitude_m = loaded_alt;
      }
    }
  } else if (is_ap_unit) {
    // First boot of a TWS-AP unit: create file with default 900m
    // (user must update via LCD menu for actual site altitude)
    station_altitude_m = 900.0;
    File altF = SPIFFS.open("/station_alt.txt", FILE_WRITE);
    if (altF) {
      altF.print(station_altitude_m);
      altF.close();
    }
  }
  // Update LCD field display with loaded value
  snprintf(ui_data[FLD_ALTITUDE].bottomRow,
           sizeof(ui_data[FLD_ALTITUDE].bottomRow), "%.0f", station_altitude_m);
  debug("[BOOT] Station Altitude: ");
  debugln(station_altitude_m);
#endif

  // RF Resolution Parameterization Logic
  bool rf_res_changed = false;
  float active_res = 0.5; // Final resolved resolution
  float last_fw_res = 0;  // Last known code hardcode
  float user_res = 0;     // Last saved UI/active resolution

  // 1. Initial Load from Persistent Files
  if (SPIFFS.exists("/rf_fw.txt")) {
    File f = SPIFFS.open("/rf_fw.txt", FILE_READ);
    if (f) { // EX2 FIX: Null handle returns "", toFloat()=0 → false data wipe!
      last_fw_res = f.readString().toFloat();
      f.close();
    } else {
      last_fw_res = DEFAULT_RF_RESOLUTION; // Safe fallback prevents false
                                           // change detection
    }
  } else {
    last_fw_res = DEFAULT_RF_RESOLUTION;
    File f = SPIFFS.open("/rf_fw.txt", FILE_WRITE);
    if (f) {
      f.print(DEFAULT_RF_RESOLUTION);
      f.close();
    } // EX4 FIX
  }

  if (SPIFFS.exists("/rf_res.txt")) {
    File f = SPIFFS.open("/rf_res.txt", FILE_READ);
    if (f) { // EX2 FIX: same pattern as rf_fw.txt
      user_res = f.readString().toFloat();
      f.close();
    } else {
      user_res = DEFAULT_RF_RESOLUTION;
    }
  } else {
    user_res = DEFAULT_RF_RESOLUTION;
    File f = SPIFFS.open("/rf_res.txt", FILE_WRITE);
    if (f) {
      f.print(DEFAULT_RF_RESOLUTION);
      f.close();
    } // EX4 FIX
  }

  // 2. Scenario Resolution
  if (fabs(DEFAULT_RF_RESOLUTION - last_fw_res) > 0.01) {
    debugln("!!! FIRMWARE HARDCODE UPDATE DETECTED !!!");
    active_res = DEFAULT_RF_RESOLUTION;
    rf_res_changed = true;
    File f1 = SPIFFS.open("/rf_fw.txt", FILE_WRITE);
    if (f1) {
      f1.print(DEFAULT_RF_RESOLUTION);
      f1.close();
    } // EX4 FIX
    File f2 = SPIFFS.open("/rf_res.txt", FILE_WRITE);
    if (f2) {
      f2.print(DEFAULT_RF_RESOLUTION);
      f2.close();
    } // EX4 FIX
  } else {
    active_res = user_res;
    rf_res_changed = false;
  }

  RF_RESOLUTION = active_res; // Apply to system
  debug("Active RF Resolution: ");
  debugln(RF_RESOLUTION);

  // UNIT VERSION

  if ((strstr(UNIT, "KSNDMC_OLD") && (SYSTEM == 0))) {
    strcpy(universalNumber, "9980945474");
    snprintf(UNIT_VER, sizeof(UNIT_VER), "TRG8-DMC-%s",
             FIRMWARE_VERSION); // #16: Single source of truth
    strcpy(NETWORK, "KSNDMC");
    strcpy(STATION_TYPE, "TRG");
    debug("[BOOT] Unit: ");
    debug(UNIT_VER);
    debug(" | Network: ");
    debug(NETWORK);
    debug(" | Type: ");
    debugln(STATION_TYPE);
    http_no = 0;
  } else if ((strstr(UNIT, "KSNDMC_TRG") && (SYSTEM == 0))) {
    strcpy(universalNumber, "9980945474");
    snprintf(UNIT_VER, sizeof(UNIT_VER), "TRG9-DMC-%s", FIRMWARE_VERSION);
    strcpy(NETWORK, "KSNDMC");
    strcpy(STATION_TYPE, "TRG");
    debug("[BOOT] Unit: ");
    debug(UNIT_VER);
    debug(" | Network: ");
    debug(NETWORK);
    debug(" | Type: ");
    debugln(STATION_TYPE);
    http_no = 1;
  } else if ((strstr(UNIT, "BIHAR_TRG") && (SYSTEM == 0))) {
    strcpy(universalNumber, "9741391102");
    snprintf(UNIT_VER, sizeof(UNIT_VER), "TRG9-BIH-%s", FIRMWARE_VERSION);
    strcpy(NETWORK, "DES-BIH");
    strcpy(STATION_TYPE, "TRG");
    debug("[BOOT] Unit: ");
    debug(UNIT_VER);
    debug(" | Network: ");
    debug(NETWORK);
    debug(" | Type: ");
    debugln(STATION_TYPE);
    http_no = 2;
  } else if ((strstr(UNIT, "BIHAR_TEST") && (SYSTEM == 0))) {
    strcpy(universalNumber, "9741391102");
    snprintf(UNIT_VER, sizeof(UNIT_VER), "TRG9-BIH-%s", FIRMWARE_VERSION);
    strcpy(NETWORK, "DES-BIH");
    strcpy(STATION_TYPE, "TRG");
    debug("[BOOT] Unit: ");
    debug(UNIT_VER);
    debug(" | Network: ");
    debug(NETWORK);
    debug(" | Type: ");
    debugln(STATION_TYPE);
    http_no = 3;
  } else if ((strstr(UNIT, "KSNDMC_TWS") && (SYSTEM == 1))) {
    strcpy(universalNumber, "9980945474");
    if (strstr(UNIT, "-AP")) {
      snprintf(UNIT_VER, sizeof(UNIT_VER), "TWS9-AP-DMC-%s", FIRMWARE_VERSION);
    } else {
      snprintf(UNIT_VER, sizeof(UNIT_VER), "TWS9-DMC-%s", FIRMWARE_VERSION);
    }
    strcpy(NETWORK, "KSNDMC");
    strcpy(STATION_TYPE, "TWS");
    debug("[BOOT] Unit: ");
    debug(UNIT_VER);
    debug(" | Network: ");
    debug(NETWORK);
    debug(" | Type: ");
    debugln(STATION_TYPE);
    http_no = 6; // 6(KSNDMC SERVER)
  } else if ((strstr(UNIT, "KSNDMC_ADDON") && (SYSTEM == 2))) {
    strcpy(universalNumber, "9980945474");
    snprintf(UNIT_VER, sizeof(UNIT_VER), "TWSRF9-DMC-%s", FIRMWARE_VERSION);
    strcpy(NETWORK, "KSNDMC");
    strcpy(STATION_TYPE, "TWS-RF");
    debug("[BOOT] Unit: ");
    debug(UNIT_VER);
    debug(" | Network: ");
    debug(NETWORK);
    debug(" | Type: ");
    debugln(STATION_TYPE);
    http_no = 7; // 7(KSNDMC SERVER)
  } else if ((strstr(UNIT, "SPATIKA_GEN") && (SYSTEM == 0))) {
    strcpy(universalNumber, "9980945474"); //"9980945474"); // Universal number
    snprintf(UNIT_VER, sizeof(UNIT_VER), "TRG9-GEN-%s", FIRMWARE_VERSION);
    strcpy(NETWORK, "SPATIKA");
    strcpy(STATION_TYPE, "TRG");
    debug("[BOOT] Unit: ");
    debug(UNIT_VER);
    debug(" | Network: ");
    debug(NETWORK);
    debug(" | Type: ");
    debugln(STATION_TYPE);
    http_no = 6; // 8(SPATIKA SERVER)
  } else if ((strstr(UNIT, "SPATIKA_GEN") && (SYSTEM == 1))) {
    strcpy(universalNumber, "9980945474");
    snprintf(UNIT_VER, sizeof(UNIT_VER), "TWS9-GEN-%s", FIRMWARE_VERSION);
    strcpy(NETWORK, "SPATIKA");
    strcpy(STATION_TYPE, "TWS");
    debug("[BOOT] Unit: ");
    debug(UNIT_VER);
    debug(" | Network: ");
    debug(NETWORK);
    debug(" | Type: ");
    debugln(STATION_TYPE);
    http_no = 8; // Spatika Server for TWS
  } else if ((strstr(UNIT, "SPATIKA_GEN") && (SYSTEM == 2))) { // EMPRII
    strcpy(universalNumber, "9980945474"); //"9980945474"); // Universal number
    snprintf(UNIT_VER, sizeof(UNIT_VER), "TWSRF9-GEN-%s", FIRMWARE_VERSION);
    strcpy(NETWORK, "SPATIKA");
    strcpy(STATION_TYPE, "TWS-RF");
    debug("[BOOT] Unit: ");
    debug(UNIT_VER);
    debug(" | Network: ");
    debug(NETWORK);
    debug(" | Type: ");
    debugln(STATION_TYPE);
    http_no = 10; // 10 (SPATIKA SERVER)
  } else {
    debugln("********** NO SYSTEM FOUND **********");
  }

  debugln();

#if SYSTEM == 0
  record_length = RECORD_LENGTH_RF;
#endif

#if SYSTEM == 1
  record_length = RECORD_LENGTH_TWS;
#endif

#if SYSTEM == 2
  record_length = RECORD_LENGTH_TWSRF;
#endif

  set_wakeup_reason();

#if DEBUG == 1
  print_reset_reason(rtc_get_reset_reason(0));
  print_reset_reason(rtc_get_reset_reason(1));
#endif

  // Firmware Update from SD Card
  if (sd_card_ok && SD.exists("/firmware.bin")) {
    bool needUpdate = true;

    // 1. Check fw_version.txt if it exists
    if (SD.exists("/fw_version.txt")) {
      File vFile = SD.open("/fw_version.txt", FILE_READ);
      if (vFile) {
        String sd_ver = vFile.readStringUntil('\n');
        sd_ver.trim();
        vFile.close();
        if (sd_ver.equalsIgnoreCase(FIRMWARE_VERSION)) {
          debugln("SD Version matches current version. Skipping.");
          needUpdate = false;
        }
      }
    }

    // 2. MD5 check
    if (needUpdate) {
      debugln("Found firmware.bin in SD Card... Verifying MD5...");
      File firmware = SD.open("/firmware.bin", FILE_READ);
      if (firmware) {
        MD5Builder md5;
        md5.begin();
        md5.addStream(firmware, firmware.size());
        md5.calculate();
        String sd_md5 = md5.toString();
        firmware.close();

        if (SPIFFS.exists("/sd_fw_md5.txt")) {
          File md5File = SPIFFS.open("/sd_fw_md5.txt", FILE_READ);
          if (md5File) {
            String stored_md5 = md5File.readStringUntil('\n');
            stored_md5.trim();
            md5File.close();
            if (sd_md5.equalsIgnoreCase(stored_md5)) {
              debugln("Firmware identical to last installed (MD5 match). "
                      "Skipping.");
              needUpdate = false;
            }
          }
        }

        if (needUpdate) {
          // Perform Update
          firmware = SD.open("/firmware.bin", FILE_READ);
          if (firmware) {
            debugln("Starting Firmware Update...");
            Update.onProgress(progressCallBack);
            if (Update.begin(firmware.size(), U_FLASH)) {
              Update.writeStream(firmware);
              if (Update.end()) {
                debugln("Update finished! Storing MD5 and restarting...");
                diag_fw_just_updated = true; // v5.76
                File md5Write = SPIFFS.open("/sd_fw_md5.txt", FILE_WRITE);
                if (md5Write) {
                  md5Write.print(sd_md5);
                  md5Write.close();
                }
                delay(1000);
                ESP.restart();
              } else {
                debugln("Update failed!");
                debugln(Update.getError());
              }
            }
            firmware.close();
          }
        }
      }
    }
  }

  if (SPIFFS.exists("/firmware.doc")) {
    String temp;

    File verTemp = SPIFFS.open("/firmware.doc", FILE_READ);
    if (!verTemp) {
      debugln("Failed to open firmware.doc for reading");
    } else { // v7.67: Guard against null file dereference
      temp = verTemp.readStringUntil('\n');
      verTemp.close();
      temp.trim();
    }
    if (strcmp(UNIT_VER, temp.c_str())) {
      debug("Firmware in SPIFFs file is ");
      debugln(temp.c_str());
      debug("Firmware in current firmware is ");
      debugln(UNIT_VER);
      debugln("Updating firmware.doc to current version ...");
      debugln();
      File verTemp1 = SPIFFS.open("/firmware.doc", FILE_WRITE);
      if (!verTemp1) {
        debugln("Failed to open firmware.doc for writing");
      } else { // v7.67: Guard null deref
        verTemp1.print(UNIT_VER);
        verTemp1.close();
      }
      debug("Firmware ver stored in SPIFFS is ");
      debugln(UNIT_VER);
      debugln("Deleting unsent data and resetting rf...");
      debugln();
      SPIFFS.remove("/unsent.txt");
      SPIFFS.remove("/unsent_pointer.txt"); // 2024 iter6
      SPIFFS.remove("/ftpunsent.txt");

      // RF
#if (SYSTEM == 0) || (SYSTEM == 2)
      rf_count.val = 0;
      rf_value = 0;
#endif

// TWS
#if (SYSTEM == 1) || (SYSTEM == 2)
      wind_count.val = 0; // Making the count as 0
#endif
    }
  } else {
    // Create firmware.doc if missing (Fresh install)
    debugln("firmware.doc not found, creating with current version...");
    File verTemp1 = SPIFFS.open("/firmware.doc", FILE_WRITE);
    if (verTemp1) {
      verTemp1.print(UNIT_VER);
      verTemp1.close();
    }
  }

  // v6.77: Atomic DOTA - Only update if binary AND ready flag exist
  if (SPIFFS.exists("/firmware.bin")) {
    if (!SPIFFS.exists("/firmware.ready")) {
      debugln(
          "[OTA] ❌ Found partial firmware.bin without ready flag. Deleting.");
      SPIFFS.remove("/firmware.bin");
    } else {
      File firmware = SPIFFS.open("/firmware.bin");
      if (firmware) {
        // v6.81: Verify Magic Byte (0xE9) before starting
        int firstByte = firmware.read();
        if (firstByte != 0xE9) {
          debugf1("[OTA] ❌ CORRUPT! Magic byte is 0x%02X (Expected 0xE9). "
                  "Aborting.\n",
                  firstByte);
          firmware.close();
          SPIFFS.remove("/firmware.bin");
          SPIFFS.remove("/firmware.ready");
          return; // Skip update
        }
        firmware.seek(0); // Reset to start for writeStream

        debugln("Found!");
        debugln("Try to update!");
        esp_task_wdt_reset(); // WDT

        Update.onProgress(progressCallBack);

        Update.begin(firmware.size(), U_FLASH);
        Update.writeStream(firmware);
        if (Update.end()) {
          debugln("Update finished!");
          File firm = SPIFFS.open("/firmware.doc", FILE_WRITE);
          if (firm) {
            firm.print(UNIT_VER);
            firm.close();
          }
          diag_fw_just_updated = true;
          delay(2000);
        } else {
          String err = Update.errorString();
          debugf1("[OTA] Update FAILED in setup: %s\n", err.c_str());

          // Dump first 32 bytes and last 32 bytes for debugging
          firmware.seek(0);
          debug("[OTA DEBUG] First 32 bytes: ");
          for (int i = 0; i < 32; i++)
            debugf1("%02X ", firmware.read());
          debugln();

          int fsize = firmware.size();
          firmware.seek(fsize > 32 ? fsize - 32 : 0);
          debug("[OTA DEBUG] Last 32 bytes: ");
          for (int i = 0; i < 32; i++)
            debugf1("%02X ", firmware.read());
          debugln();

          Preferences p;
          p.begin("ota-track", false);
          int cnt = p.getInt("fail_cnt", 0) + 1;
          p.putInt("fail_cnt", cnt);
          p.putString("fail_res", err);
          p.end();
        }

        firmware.close();

        // ONLY DELETE IF SUCCESSFUL OR CORRUPT MAGIC BYTE (handled earlier)
        if (diag_fw_just_updated) {
          if (SPIFFS.exists("/firmware.bin")) {
            if (SPIFFS.remove("/firmware.bin")) {
              debugln("Firmware removed succesfully!");
            }
          }
          if (SPIFFS.exists("/firmware.ready")) {
            SPIFFS.remove("/firmware.ready");
          }
        } else {
          debugln(
              "Firmware update failed, keeping /firmware.bin for analysis.");
          if (SPIFFS.exists("/firmware.ready")) {
            SPIFFS.remove("/firmware.ready");
          }
        }

        delay(500); // TRG8-3.0.5g reduced from 2secs to 500ms
        ESP.restart();
      }
    }
  }

  gprs_mode = eGprsInitial;
  sync_mode = eSyncModeInitial;

  // Task creation and spawning
  xTaskCreatePinnedToCore(rtcRead, "rtcReadTask", 8192, NULL, 2, &rtcRead_h,
                          1); // Core 1
  xTaskCreatePinnedToCore(scheduler, "schedulerTask", 14336, NULL, 3,
                          &scheduler_h, 1); // Core 1
  xTaskCreatePinnedToCore(gprs, "gprsTask", 16384, NULL, 2, &gprs_h,
                          0); // Core 0

  //    #if DEBUG == 1
  //        xTaskCreatePinnedToCore(stackMonitor, "StackMonitor", 2048, NULL,
  //        1, NULL, 1);
  //    #endif

#if (SYSTEM == 1) || (SYSTEM == 2)
  // #7: Smart Task Creation - Only spawn tasks if sensors were found in
  // initialize_hw
  if (hdcType != HDC_UNKNOWN || bmeType != BME_UNKNOWN) {
    xTaskCreatePinnedToCore(tempHum, "tempHumTask", 4096, NULL, 2, &tempHum_h,
                            1); // Core 1
  } else {
    tempHum_h = NULL;
    debugln("[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.");
  }

  // v5.50: Only spawn bmeTask if Pressure is enabled AND this is a TWS-AP
  // unit
  bool pressure_supported =
      (SYSTEM == 1 && strstr(UNIT, "KSNDMC_TWS-AP") != NULL);
  if (bmeType == BME_280 && ENABLE_PRESSURE_SENSOR == 1 && pressure_supported) {
    xTaskCreatePinnedToCore(bmeTask, "bmeTask", 4096, NULL, 2, &bmeTask_h,
                            1); // Core 1
  } else {
    bmeTask_h = NULL;
    if (bmeType == BME_280) {
      debugln("[BOOT] BME280 found but Pressure Task DISABLED per "
              "configuration/unit type.");
    } else {
      debugln("[BOOT] BME280 not found. bmeTask NOT created.");
    }
  }

  xTaskCreatePinnedToCore(windSpeed, "windSpeedTask", 4096, NULL, 2,
                          &windSpeed_h,
                          1); // Core 1
  xTaskCreatePinnedToCore(windDirection, "windDirectionTask", 4096, NULL, 2,
                          &windDirection_h, 1); // Core 1
#endif

  if (wired == 1) {
    xTaskCreatePinnedToCore(lcdkeypad, "lcdkeypadTask", 4096, NULL, 2,
                            &lcdkeypad_h, 0); // Core 0
    if (wakeup_reason_is != timer) {
      digitalWrite(32, HIGH); // Power on LCD only if NOT a background wakeup
      delay(100);
    }
  }

  //    }

  // attachInterrupt(27, ext0_isr, FALLING); // DISABLE: Conflicts with ULP
  // Counting on Pin 27. ULP handles counting.
  debugln("ULP Counting Enabled (attachInterrupt 27 Disabled).");
  delay(1000);

  // Should be called only after getting the correct RF count from SPIFF. If
  // no RF is present, make rf_count.val = 0 and call this ()

  // REVERTED to AIO9_3.0 logic: Always load/restart ULP on setup to ensure
  // counting is active.
  debug("ULP Wakeup Period set to 1ms (High Resolution for Wind)");
  ULP_COUNTING(ULP_WAKEUP_TC);
  debugln("ULP Program loaded and started.");

  // RF Resolution Change Cleanup
  if (rf_res_changed) {
    debugln("Resolution changed. Cleaning up data files...");
    // 1. Get current date from RTC
    DateTime now = rtc.now();
    int currentDay = now.day();
    int currentMonth = now.month();
    int currentYear = now.year();

    // 2. Determine rf_close_date (same logic as scheduler)
    int rf_day = currentDay, rf_month = currentMonth, rf_year = currentYear;
    int sn = (now.hour() * 4 + now.minute() / 15 + 61) % 96;
    if (sn <= 60) {
      // Inline next_date logic
      int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
      if ((rf_year % 4 == 0 && rf_year % 100 != 0) || (rf_year % 400 == 0))
        daysInMonth[2] = 29;
      rf_day++;
      if (rf_day > daysInMonth[rf_month]) {
        rf_day = 1;
        rf_month++;
        if (rf_month > 12) {
          rf_month = 1;
          rf_year++;
        }
      }
    }

    // 3. Construct and Delete today's file
    char today_file[50];
    snprintf(today_file, sizeof(today_file), "/%s_%04d%02d%02d.txt",
             station_name, rf_year, rf_month, rf_day);
    if (SPIFFS.exists(today_file)) {
      debug("Deleting present day file: ");
      debugln(today_file);
      SPIFFS.remove(today_file);
    }

    // 4. Delete unsent files
    SPIFFS.remove("/unsent.txt");
    SPIFFS.remove("/ftpunsent.txt");
    SPIFFS.remove("/signature.txt");

    // 5. Reset ULP counters
    SPIFFS.remove("/calib.txt");
    rf_count.val = 0;
    wind_count.val = 0;
    debugln("Cleanup complete. Starting fresh.");
  }
}

void progressCallBack(size_t currSize, size_t totalSize) {
  // Silent during actual download to prevent console leak into Modem RX
  if (!ota_silent_mode) {
    debugf2("CALLBACK:  Update process at %d of %d bytes...\n", currSize,
            totalSize);
  }
}

void initialize_hw() {
  // v7.79: Mandatory Stability Breather. Give pins & radio time to settle
  // before attaching peripherals or loading ULP counts.
  vTaskDelay(5000 / portTICK_PERIOD_MS);

  // Setup Legacy ADC (To prevent driver_ng conflicts in Core 3.x)
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_5,
                            ADC_ATTEN_DB_11); // GPIO 33 (Battery)
  adc2_config_channel_atten(ADC2_CHANNEL_8,
                            ADC_ATTEN_DB_11); // GPIO 25 (Solar)

  Serial.begin(115200);
  // v7.06: CRITICAL! Expand UART RX buffer to 16KB so that incoming AT+HTTPREAD
  // data does not drop bytes when Update.write() blocks the CPU during flash
  // erase operations.
  SerialSIT.setRxBufferSize(16384);
  SerialSIT.begin(115200, SERIAL_8N1, 16, 17, false);

  setCpuFrequencyMhz(80); // Step down early to save power during boot init
  WiFi.mode(WIFI_OFF);    // Ensure WiFi radio is OFF
  btStop();               // Ensure Bluetooth is OFF

  Wire.begin(21, 22, 100000); // Increased to 100kHz for efficiency
  Wire.setTimeOut(I2C_TIMEOUT_MS);
  delay(100); // Reduced from 300ms as 100ms is sufficient for bus stability

#if (SYSTEM == 1 || SYSTEM == 2)
  // Early init for sensor detection status
  // v6.50: Skip BME for TWS-RF (SYSTEM 2) as requested
  if (SYSTEM != 2) {
    initBME();
  }
  initHDC();
#endif

  // DISABLE Watchdog during long mount/format operations
  esp_task_wdt_deinit();

  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) == pdTRUE) {
    if (!rtc.begin()) {
      debugln("[BOOT] RTC: HARDWARE FAILED");
      diag_rtc_battery_ok = false;
    } else {
      if (!rtc.isrunning()) {
        debugln("[BOOT] RTC: BATTERY FAILED / CLOCK STOPPED");
        diag_rtc_battery_ok = false;
      } else {
        debugln("[BOOT] RTC: OK");
        diag_rtc_battery_ok = true;
      }
    }
    xSemaphoreGive(i2cMutex);
  }

  pinMode(32, OUTPUT);
  pinMode(26, OUTPUT);
  pinMode(27, INPUT_PULLUP);

  // SELF-HEALING: Release GPIO holds from deep sleep
  gpio_hold_dis(GPIO_NUM_26);
  gpio_hold_dis(GPIO_NUM_32);

  if (!SPIFFS.begin(true)) {
    debugln("[BOOT] SPIFFS: FAILED");
  } else {
    debugln("[BOOT] SPIFFS: OK");
  }

  SPI.begin(18, 19, 23, 5);
  if (!SD.begin(5, SPI, 2000000)) {
    debugln("[BOOT] SD Card: FAILED (Not Inserted/Hardware)");
    sd_card_ok = 0;
    SPI.end(); // Save power by releasing SPI peripheral if SD is missing
  } else {
    debugln("[BOOT] SD Card: OK");
    sd_card_ok = 1;
  }

  // RE-INITIALIZE Watchdog with a safe 30-second timeout for first-boot tasks
  // v5.52: Increased Watchdog to 120s for slow 2G OTA stability
  esp_task_wdt_config_t wdt_config = {.timeout_ms = 120000,
                                      .idle_core_mask =
                                          (1 << portNUM_PROCESSORS) - 1,
                                      .trigger_panic = true};
  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL);
  esp_task_wdt_reset();

  if (SYSTEM != 2) {
    if (bmeType == BME_280) {
      debugln("[BOOT] BME280: OK");
    } else {
      debugln("[BOOT] BME280: NOT FOUND");
    }
  }

  if (hdcType != HDC_UNKNOWN) {
    debugln("[BOOT] HDC Sensor: OK");
  } else {
    debugln("[BOOT] HDC Sensor: NOT FOUND");
  }

#if (SYSTEM == 0 || SYSTEM == 2)
  debugln("[BOOT] Rainfall Counter: OK");
#endif

  debugln("[BOOT] Hardware Initialized.");
}

void loop() {
  /*
   * eHTTPStop : When SIM error or SIGNAL issue ot REG issue (wont service
   * SMS) but will store current record to unsent_file) eSMSStop : After HTTP
   * is over, SMS starts regardless of whether HTTP went through or not. after
   * SMS, it issues this eExceptionHandled : This is when resync_time is
   * triggered inside rtcTask
   */

  // WiFi is now manually triggered via the LCD menu.
  // We no longer aggressively auto-start the Access Point here on EXT0
  // wakeups.
  if ((millis() > 5000) &&
      ((sync_mode == eHttpStop) || (sync_mode == eSMSStop) ||
       (sync_mode == eExceptionHandled)) &&
      (lcdkeypad_start == 0) && (wifi_active == false) &&
      (httpInitiated == false) && (health_in_progress == false) &&
      (force_reboot == false) && (force_ota == false) &&
      (ota_writing_active == false)) {
    debugln("[PWR] All tasks done. Entering Deep Sleep...");
    esp_task_wdt_reset(); // Final pet before sleep
    start_deep_sleep();
  }

  esp_task_wdt_reset(); // Pet the watchdog for the loopTask
  delay(500);
}

void print_reset_reason(RESET_REASON reason) {
  debug("[BOOT] Reset Reason: ");
  switch (reason) {
  case 1:
    debugln("POWERON_RESET");
    break; /**<1, Vbat power on reset*/
  case 3:
    debugln("SW_RESET");
    break; /**<3, Software reset digital core*/
  case 4:
    debugln("OWDT_RESET");
    break; /**<4, Legacy watch dog reset digital core*/
  case 5:
    debugln("DEEPSLEEP_RESET");
    break; /**<5, Deep Sleep reset digital core*/
  case 6:
    debugln("SDIO_RESET");
    break; /**<6, Reset by SLC module, reset digital core*/
  case 7:
    debugln("TG0WDT_SYS_RESET");
    break; /**<7, Timer Group0 Watch dog reset digital core*/
  case 8:
    debugln("TG1WDT_SYS_RESET");
    break; /**<8, Timer Group1 Watch dog reset digital core*/
  case 9:
    debugln("RTCWDT_SYS_RESET");
    break; /**<9, RTC Watch dog Reset digital core*/
  case 10:
    debugln("INTRUSION_RESET");
    break; /**<10, Instrusion tested to reset CPU*/
  case 11:
    debugln("TGWDT_CPU_RESET");
    break; /**<11, Time Group reset CPU*/
  case 12:
    debugln("SW_CPU_RESET");
    break; /**<12, Software reset CPU*/
  case 13:
    debugln("RTCWDT_CPU_RESET");
    break; /**<13, RTC Watch dog Reset CPU*/
  case 14:
    debugln("EXT_CPU_RESET");
    break; /**<14, for APP CPU, reseted by PRO CPU*/
  case 15:
    debugln("RTCWDT_BROWN_OUT_RESET");
    break; /**<15, Reset when the vdd voltage is not stable*/
  case 16:
    debugln("RTCWDT_RTC_RESET");
    break; /**<16, RTC Watch dog reset digital core and rtc module*/
  default:
    debugln("NO_MEAN");
  }

  get_chip_id();
}

// ------------------------------------------------------------------------
void ULP_COUNTING(uint32_t us) {
  // Clear ULP variables on Power-On reset only (Reason 1 or 14)
  // This prevents using garbage values from uninitialized RTC RAM.
  RESET_REASON reason = rtc_get_reset_reason(0);
  debug("[ULP] Init. HW Reason: ");
  debugln((int)reason);

  // v7.93: Only wipe on HARD Power-On (1). 
  if (reason == 1) {
    debugln("[ULP] Hard Power-On. Wiping ULP counters.");
    rf_count.val = 0;
    cur_state.val = 0;
    prev_state.val = 0;
    debounced_state.val = 0;
    debounce_cnt.val = 0;
    wind_count.val = 0;
    wind_prev_state.val = 0;
    calib_count.val = 0;
    calib_mode_flag.val = 0;
  } else {
    debug("[ULP] Warm Boot. Preserving Counters. RF=");
    debugln(rf_count.val);

    // Validate counters to detect memory corruption
    validate_ulp_counters();
  }

  // RTC_SLOW_MEM[13] = 0; // REMOVED: Potential memory corruption source
  ulp_set_wakeup_period(0, us);
  const ulp_insn_t ulp_rf[] = {

      // rf_count, cur_state, prev_state, debounced_state,  debounce_cnt;
      // read the rf pin into cur_state;
      // check if this cur_state is different from the previous state
      // (prev_state)
      // if different, store this cur_state as the prev_state for next time
      // processing
      // and also start a debouncing algo. init decbounce_cnt
      // --- RAINFALL SECTION (Falling Edge 1 -> 0) ---
      I_GPIO_READ(GPIO_NUM_34), // R0 = current state

      I_MOVI(R3, 0), I_LD(R1, R3, U_PREV_STATE), // R1 = prev_state

      I_MOVR(R2, R0),     // R2 = current state
      I_ADDR(R0, R1, R2), // R0 = R1 + R2
      M_BGE(1, 2),        // If both are 1 (2), states haven't changed -> Jump 1
      M_BL(1, 1),         // If both are 0 (0), states haven't changed -> Jump 1

      // States are different, start debouncing
      I_MOVI(R3, 0), I_ST(R2, R3, U_PREV_STATE), // prev_state = current
      I_MOVI(R0, 25),                            // v7.93: Debounce increased to 25ms
      I_ST(R0, R3, U_DEBOUNCE_CNT),
      M_BX(4), // Skip to next sensor while debouncing

      M_LABEL(1), I_MOVI(R3, 0), I_LD(R0, R3, U_DEBOUNCE_CNT),
      M_BGE(2, 1), // If count > 0, go to decrement
      M_BX(4),     // If 0, no change, skip to next sensor

      M_LABEL(2), I_SUBI(R0, R0, 1), I_MOVI(R3, 0),
      I_ST(R0, R3, U_DEBOUNCE_CNT), M_BGE(4, 1), // If still > 0, exit

      // Debounce finished! Validate state change
      I_MOVI(R3, 0), I_LD(R1, R3, U_PREV_STATE), // R1 = new stable state
      I_LD(R2, R3, U_DEBOUNCED_STATE),           // R2 = old stable state
      I_ADDR(R0, R1, R2), M_BGE(4, 2),           // Steady High (1+1=2) -> Skip
      M_BL(4, 1),                                // Steady Low (0+0=0) -> Skip

      I_ST(R1, R3, U_DEBOUNCED_STATE), // Update stable state

      // Transition detected! Is it Falling Edge (1 -> 0)?
      I_ADDI(R0, R1, 0),
      M_BGE(4,
            1), // If new state is High (1), it's a 0->1 transition, skip count

      // If we are here, it's a 1 -> 0 transition (TIP)
      // Check Calibration Mode
      I_LD(R0, R3, U_CALIB_MODE), M_BGE(10, 1),

      // Normal Count
      I_LD(R0, R3, U_RF_COUNT), I_ADDI(R0, R0, 1), I_ST(R0, R3, U_RF_COUNT),
      M_BX(4),

      M_LABEL(10), // Calibration Count
      I_LD(R0, R3, U_CALIB_COUNT), I_ADDI(R0, R0, 1),
      I_ST(R0, R3, U_CALIB_COUNT),

      // --- WIND SECTION (Rising Edge 0 -> 1) ---
      M_LABEL(4),
      I_GPIO_READ(GPIO_NUM_35),                       // R0 = current state
      I_MOVI(R3, 0), I_LD(R1, R3, U_WIND_PREV_STATE), // R1 = wind_prev_state

      I_MOVR(R2, R0), I_SUBR(R2, R1, R2), // R2 = prev - curr
      M_BXZ(5),                           // If same, exit

      I_ST(R0, R3, U_WIND_PREV_STATE), // Update prev

      I_MOVI(R2, 1), I_SUBR(R2, R1, R2), // R2 = prev - 1
      M_BXZ(5), // If prev was 1, it's a 1->0, exit (Rising edge only)

      I_MOVI(R2, 0), I_SUBR(R2, R0, R2), // R2 = curr - 0
      M_BXZ(5),                          // If curr is 0, exit

      // Rising Edge!
      // v5.50 FIX: Removed faulty I_HALT() that killed the Wind counter.
      I_LD(R0, R3, U_WIND_COUNT), I_ADDI(R0, R0, 1), I_ST(R0, R3, U_WIND_COUNT),

      M_LABEL(5), I_HALT()};

  rtc_gpio_init(GPIO_NUM_2);
  rtc_gpio_set_direction(GPIO_NUM_2, RTC_GPIO_MODE_OUTPUT_ONLY);

  rtc_gpio_init(GPIO_NUM_34);
  rtc_gpio_set_direction(GPIO_NUM_34, RTC_GPIO_MODE_INPUT_ONLY);

  rtc_gpio_init(GPIO_NUM_35);
  rtc_gpio_set_direction(GPIO_NUM_35, RTC_GPIO_MODE_INPUT_ONLY);

  rtc_gpio_pullup_en(GPIO_NUM_34); // rf (Note: GPIO34 has NO internal pullup
                                   // on ESP32. External required!)
  rtc_gpio_pullup_en(GPIO_NUM_35); // wind count (Note: GPIO35 has NO internal
                                   // pullup on ESP32. External required!)

  size_t size = sizeof(ulp_rf) / sizeof(ulp_insn_t);
  ulp_process_macros_and_load(0, ulp_rf, &size);
  ulp_run(0);
}

// AIO_iter3 : Restructuring for scheduler to have all calculation for cumrf
// etc at the beginning.

/* wired = 1 : Integrated system
 * // SENSOR/PERIPHERAL
 * GPIO34 : RF - it if for rainfall monitoring even in deep sleep mode(ULP)
 * GPIO35 : WS - It is used for getting the WS count in deep sleep mode(ULP)
 * GPIO39 : WD - It is used to get analog data from Wind direction sensor
 * AS5600 magnetic encoder wind direction (earlier solar sense) I2C    :
 * Temp/Hum sensor HDC1080/HDC2022  (0x40) I2C    : LCD 16x2 (0x27) I2C    :
 * RTC (0x68)
 * // CONTROL SIGNALS
 * GPIO27 : ext0 - external wakeup for UI or lcdkeypad(for lcdkeypad, it also
 * enables GPIO32 to give 5V supply to LCD GPIO32 : When made HIGH switched on
 * the MOSFET to allow 5V supply to LCD. When set to LOW, it stops 5V supply
 * to the i2c based LCD module GPIO26 : When made HIGH , it provides 3.7V to
 * GPRS module. When set to LOW, it stops 3.7V to GPRS module
 * // SIGNAL STRENGTHS
 * -110 : This signal_strength is given while filling the previous
 * rf_close_date file -111 : This signal_strength is given while filling the
 * initial ones when device starts at mid - day and SPIFFs file was not
 * present. -112 or -115 to -120  : This signal_strength is given to all the
 * filled up values for the current day when there were some initial values in
 * the current rf_close_date file. cur_file is present here -125 to -130 : If
 * signal strength is not good including SIM ERROR, data is stored with this
 * signal range
 * // Sample data stored
 * TRG : length 45 including \r\n :
 * 00,2024-05-21,08:45,0000.0,0000.0,-111,00.0\r\n (inst_rf,cum_rf) TWS :
 * length 53 including \r\n :
 * 00,2024-05-21,08:45,00.00,00.00,00.00,000,-111,00.0\r\n
 * (temp,hum,avg_ws,wd)
 */
