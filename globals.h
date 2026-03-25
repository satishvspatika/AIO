#ifndef GATEWAY_GLOBALS_H
#define GATEWAY_GLOBALS_H

// =========================================================================
// PHASE 10 FIRMWARE MUTEX HIERARCHY (ABBA DEADLOCK PREVENTION):
// To prevent permanent RTOS dual-core lockups, NEVER acquire semaphores 
// in reverse order. ALWAYS follow this strict acquisition sequence:
// 
// 1. modemMutex (Highest level - Network/UART ops)
// 2. fsMutex    (Mid level - SPIFFS/VFS ops)
// 3. i2cMutex   (Lowest level - Local hardware/LCD/RTC ops)
//
// Example: If a task holds fsMutex, it CANNOT grab modemMutex. It must 
// drop fsMutex entirely, grab modemMutex, and then re-grab fsMutex.
// =========================================================================

#ifdef ARDUINO
#include "driver/rtc_io.h"
#include "esp32/ulp.h"
#include "esp_sleep.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#if KEYPAD_TYPE == 1
#include <Keypad_I2C.h>
#elif KEYPAD_TYPE == 0
#include <Keypad.h>
#endif
#if KEYPAD_TYPE != 2
#include <LiquidCrystal_I2C.h>
#endif
#include <MD5Builder.h>
#include <Preferences.h>
#include <RTClib.h>
#include <SD.h>
#include <SPIFFS.h>
#include <Update.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>
#if ENABLE_WEBSERVER == 1
#include <WebServer.h>
#endif
#include <Wire.h>
#include <esp_attr.h>
#include <esp_task_wdt.h>
#include <esp_wifi.h>
#include <hulp_arduino.h>

#include "esp_adc_cal.h"

float get_calibrated_battery_voltage();
#include <vector>
#else
// Linter fallback for non-Arduino environment (e.g. host-side analysis)
#include <stdint.h>
#include <string>
#define String std::string
#endif

#include <math.h>
#include <stdio.h>
#include <time.h>

// v5.66: Proper RTC attribute definition for ESP32.
// Do NOT define it as empty; otherwise linker will move variables out of RTC RAM.
#ifndef RTC_DATA_ATTR
#define RTC_DATA_ATTR __attribute__((section(".rtc.data")))
#endif

enum HDC_Type { // Sensor type enum
  HDC_UNKNOWN,
  HDC_1080,
  HDC_2022
};

enum BME_Type { BME_UNKNOWN, BME_280 };

extern HDC_Type hdcType;
extern BME_Type bmeType;
extern volatile bool health_in_progress;
extern volatile bool
    schedulerBusy; // v5.65: Prevents sleep during 15-min slot processing
extern volatile bool
    primary_data_delivered; // v5.51: Track session success for backlog gating
extern volatile RTC_DATA_ATTR bool skip_primary_http; // v7.88: Skip live upload on duplicates
extern volatile bool
    force_ftp; // v5.68: Command piggyback — FTP_BACKLOG (unsent backlog)
extern volatile bool force_ftp_daily; // v5.80: Command piggyback — FTP_DAILY
                                      // (specific date file)
extern char
    ftp_daily_date[12]; // v5.80: Date string for FTP_DAILY e.g. "20260228"
extern volatile bool force_reboot; // v5.68: Command piggyback
extern volatile bool force_ota;    // v5.68: Command piggyback
extern volatile bool
    force_gps_refresh; // v7.59: Server-requested GPS re-acquire
extern volatile bool
    force_clear_ftp_queue; // v7.59: Server-requested FTP backlog clear
extern volatile bool force_delete_data; // v7.94: Server-requested Factory Reset
extern volatile bool ota_writing_active; // v6.88: Prevent FS collision
extern int ota_fail_count;
extern char ota_fail_reason[48];
extern char ota_cmd_param[128];       // v75: Target binary name from dashboard
extern char ota_md5_hash[33]; // Phase 2: Binary Integrity Checksum
extern RTC_DATA_ATTR int last_cmd_id;               // v7.92: Command ID for feedback
extern RTC_DATA_ATTR char last_cmd_res[64];         // v7.92: Result message for feedback
extern volatile bool ota_silent_mode; // Rule 43: Stop all log leakage
extern volatile bool bearer_recovery_active;

// SAFETY BUFFER: Reserve 512 bytes at the start of RTC Memory to prevent
// ULP Program Code (loaded at offset 0) from overwriting C variables.
extern RTC_DATA_ATTR uint8_t ulp_code_reserve[512];

// ESP-NOW permanently disabled (v5.40+) - includes removed
// #include <esp_now.h>

// CHANGE THESE FOR DIFFERENT SYSTEMS
// Function Prototypes for Cross-File Visibility
void graceful_modem_shutdown();
void start_deep_sleep();
void flushSerialSIT();
bool verify_bearer_or_recover();
int send_at_cmd_data(char *payload, String response_arg);
void get_signal_strength();
void analyzeFileHealth(uint32_t *mask, int *outNetCount, bool *hasUnresolvedPD,
                       bool *hasUnresolvedNDM);
void reconstructSentMasks(bool alreadyLocked = false);
void markFileAsDelivered(const char *fileName, bool alreadyLocked = false);
void convert_gmt_to_ist(struct tm *gmt_time);
void sync_rtc_from_server_tm(const char *body, bool is_ntp);
void reset_all_diagnostics();
void recoverI2CBus(bool alreadyLocked = false);

/************************************************************************************************/
#include "user_config.h"

extern char UNIT[15];        // (Initialized secretly in .ino via UNIT_CFG to avoid ODR issues)
extern int test_health_every_slot;
extern float RF_RESOLUTION;

/************************************************************************************************/

#define HDC_ADDR 0x40 // Default I2C address for both HDC1080 and HDC2022
#define I2C_SDA 21    // Explicit definition for I2C bus in ESP32
#define I2C_SCL 22
#define I2C_TIMEOUT_MS 2000 // I2C hardware timeout (2 seconds)
#define I2C_MUTEX_WAIT_TIME                                                    \
  2500 // Mutex wait time (slightly longer than I2C timeout to prevent premature
       // timeout)
#define uS_TO_S_FACTOR                                                         \
  1000000 /* Conversion factor for micro seconds to seconds */

// WS
//#define RADIUS_METERS 0.07       // e.g., 7 cm
//#define PULSES_PER_REV 4
#define AVG_WS_DURATION_SECONDS 900 // 15 minutes
#define NUM_OF_TEETH 4
#define TWO_PI 6.28318530718
// 2*pi*r (pulses per revolution = 4 and r = 0.07m)
#define WS_CALIBRATION_FACTOR 0.4398229715026

// Record length constants for different systems
#define RECORD_LENGTH_RF 45    // SYSTEM == 0
#define RECORD_LENGTH_TWS 53   // SYSTEM == 1 (Standardized without rainfall)
#define RECORD_LENGTH_TWSRF 60 // SYSTEM == 2

// Sample number constants
#define MAX_SAMPLE_NO 95
#define MIDNIGHT_SAMPLE_NO 60
#define SAMPLES_PER_HOUR 4
#define MINUTES_PER_SAMPLE 15

// Signal strength constants
#define SIGNAL_STRENGTH_NO_DATA                                                \
  -112 // v5.51: Was -87 (shifted to avoid real signal overlap)
#define SIGNAL_STRENGTH_GAP_FILLED -113   // v5.51: Was -88
#define SIGNAL_STRENGTH_MISSING_DATA -111 // Official "No Data" marker
#define SIGNAL_STRENGTH_PREV_DAY_GAP -114 // v5.51: Was -89
#define SIGNAL_STRENGTH_MIN_RANGE -130
#define SIGNAL_STRENGTH_MAX_RANGE -110

// Temperature/Humidity constants
#define TEMP_OFFSET_CORRECTION 2.1
#define HUMIDITY_HIGH_THRESHOLD 96.0
#define HUMIDITY_SATURATION_THRESHOLD 99.0
#define TEMP_SAME_COUNT_THRESHOLD 7
#define HUM_SAME_COUNT_THRESHOLD 7
#define TEMP_JITTER_MIN 0.1
#define TEMP_JITTER_MAX 0.3
#define HUM_JITTER_MIN 0.7
#define HUM_JITTER_MAX 1.6
#define INVALID_PREV_VALUE_THRESHOLD -900.0
#define INITIAL_PREV_TEMP -999.0
#define INITIAL_PREV_HUM -999.0

// Time constants
#define HOURS_PER_DAY 24
#define MINUTES_PER_HOUR 60
#define START_HOUR 8
#define START_MINUTE 45
#define START_MINUTE_ALT 30

// Wind direction constants
#define WIND_DIR_MIN_VALID 10
#define WIND_DIR_MAX_VALID 350
#define WIND_DIR_RANGE 5
#define WIND_DIR_MAX 360

// Station altitude for Sea Level Pressure correction
// Only relevant for KSNDMC_TWS-AP (BME280 pressure sensor enabled)
// Loaded from /station_alt.txt on SPIFFS. Default 0.0 = no correction applied
extern float station_altitude_m; 
#define SEALEVEL_CALC_FACTOR 44330.769

// Fill signal strength constants
#define FILL_SIG_MIN 115
#define FILL_SIG_MAX 120

extern HardwareSerial SerialSIT;
#if KEYPAD_TYPE == 2
struct Nuvoton_Smart_LCD : public Print {
    void init() {}
    void begin() {}
    void display() {}
    void backlight() {}
    void noBacklight() {}
    void clear() { Wire.beginTransmission(0x20); Wire.write(0x01); Wire.endTransmission(); vTaskDelay(2 / portTICK_PERIOD_MS); }
    void noCursor() {}
    void noBlink() {}
    void setCursor(uint8_t col, uint8_t row) { 
        uint8_t cmd = (row == 0 ? 0x80 : 0xC0) + col;
        Wire.beginTransmission(0x20); Wire.write(cmd); Wire.endTransmission();
    }
    size_t write(uint8_t c) override {
        Wire.beginTransmission(0x20); Wire.write(c); Wire.endTransmission(); return 1;
    }
    size_t write(const uint8_t *buffer, size_t size) override {
        Wire.beginTransmission(0x20);
        for(size_t i=0; i<size; i++) Wire.write(buffer[i]);
        Wire.endTransmission();
        return size;
    }
};
extern Nuvoton_Smart_LCD lcd;

struct Nuvoton_Smart_Keypad {
    void setDebounceTime(int t) {}
    void setHoldTime(int t) {}
    void begin() {}
    char getKey() {
        char k = NO_KEY;
        Wire.requestFrom((uint8_t)0x20, (uint8_t)1);
        if(Wire.available()) k = Wire.read();
        return k;
    }
};
extern Nuvoton_Smart_Keypad keypad;
#else
extern LiquidCrystal_I2C lcd;
#endif
extern SemaphoreHandle_t i2cMutex;
extern SemaphoreHandle_t serialMutex;
extern SemaphoreHandle_t modemMutex;
extern SemaphoreHandle_t fsMutex;
extern RTC_DS1307 rtc;

extern portMUX_TYPE timerMux0;
extern portMUX_TYPE timerMux1;
extern portMUX_TYPE timerMux2;
extern portMUX_TYPE windMux;
extern portMUX_TYPE rtcTimeMux;

// Keypad timing
extern unsigned long last_key_time;

// record_length moved below
extern int cur_file_found;
#if DEBUG == 1
#define debugln(...)                                                           \
  do {                                                                         \
    if (!ota_silent_mode && !bearer_recovery_active)                           \
      Serial.println(__VA_ARGS__);                                             \
  } while (0)
#define debug(...)                                                             \
  do {                                                                         \
    if (!ota_silent_mode && !bearer_recovery_active)                           \
      Serial.print(__VA_ARGS__);                                               \
  } while (0)
#define debugf1(a, x)                                                          \
  do {                                                                         \
    if (!ota_silent_mode && !bearer_recovery_active)                           \
      Serial.printf(a, x);                                                     \
  } while (0)
#define debugf2(a, x, y)                                                       \
  do {                                                                         \
    if (!ota_silent_mode && !bearer_recovery_active)                           \
      Serial.printf(a, x, y);                                                  \
  } while (0)
#define debugf3(a, x, y, z)                                                    \
  do {                                                                         \
    if (!ota_silent_mode && !bearer_recovery_active)                           \
      Serial.printf(a, x, y, z);                                               \
  } while (0)
#define debugf4(a, x, y, z, p)                                                 \
  do {                                                                         \
    if (!ota_silent_mode && !bearer_recovery_active)                           \
      Serial.printf(a, x, y, z, p);                                            \
  } while (0)
#define debugf5(a, x, y, z, p, q)                                              \
  do {                                                                         \
    if (!ota_silent_mode && !bearer_recovery_active)                           \
      Serial.printf(a, x, y, z, p, q);                                         \
  } while (0)
#define debugf(a, ...)                                                         \
  do {                                                                         \
    if (!ota_silent_mode && !bearer_recovery_active)                           \
      Serial.printf(a, ##__VA_ARGS__);                                         \
  } while (0)
#else
#define debug(...)
#define debugln(...)
#define debugf1(a, x)
#define debugf2(a, x, y)
#define debugf3(a, x, y, z)
#define debugf4(a, x, y, z, p)
#define debugf5(a, x, y, z, p, q)
#define debugf(a, ...)
#endif

// ENUMS
enum { ext0 = 1, ext1, timer, touchpad, ulp };
enum { eAlphaNum, eNumeric, eDisplayOnly, eLive };
enum {
  eSyncModeInitial,
  eHttpTrigger,
  eHttpBegin,
  eHttpStarted,
  eHttpStop,
  eExceptionHandled,
  eSMSStart,
  eSMSStop,
  eGPSStart,
  eStartupGPS,
  eHealthStart
}; // sync_mode
enum {
  eGprsInitial,
  eGprsSignalOk,
  eGprsSignalForStoringOnly,
  eGprsSleepMode
};                                    // gprs_mode
enum { eCurrentData, eClosingData, eUnsentData }; // for http

enum { eEditOff, eEditOn };                   // lcdkeypad
enum { eCursorOff, eCursorUl, eCursorBlink }; // lcdkeypad

/*
 *  Variabe declaration
 */
extern bool webServerStarted;
extern volatile bool wifi_active;
extern unsigned long last_wifi_activity_time;
extern float temp_crf, temp_instrf, temp_bat, temp_temp, temp_hum, temp_avg_ws;
extern int temp_sampleNo, temp_day, temp_month, temp_year, temp_hr, temp_min, temp_sig;
extern int data_writing_initiated;
extern int time_to_deepsleep;
// Common
extern char UNIT_VER[20], STATION_TYPE[10], NETWORK[10];
extern char universalNumber[20];
extern volatile bool timeSyncRequired;
extern volatile bool httpInitiated;
extern char *startptr;
extern int send_status;
extern bool valid_dt, valid_time;
// SCHEDULER (GLOBAL) - v5.66: Moved to AIO9_5.0.ino
extern volatile int sync_mode, gprs_mode, data_mode;
extern int sampleNo;
extern int rf_cls_dd, rf_cls_mm, rf_cls_yy; 
extern int rf_cls_ram_dd, rf_cls_ram_mm, rf_cls_ram_yy;
extern int time_to_sleep;

extern float bat_val;

extern char battery[10], solar_sense[10];
extern int solar_conn;

extern volatile bool schedulerBusy;
extern char cur_file[32], unsent_file[32], new_file[32], temp_file[50];
extern char station_name[16];
extern char chip_id[13]; 
extern char calib_state[5], calib_text[40], calib_content[16];
extern char ftpunsent_file[50];
extern char ftpdaily_file[50];
extern int ftp_login_flag;
extern int calib_header_drawn;
// GPRS
extern int http_no, msg_sent;
extern int rssiIndex, rssiEndIndex, retries, registration;
extern int unsent_count, success_count;
extern int s, fileSize;
extern int delay_val; // Delay before starting GPRS
extern int signal_strength;
extern int signal_lvl;
extern int sd_card_ok;
extern int send_daily;
extern float solar_val, solar;
extern float li_bat, li_bat_val;

extern RTC_DATA_ATTR double lati, longi;
extern RTC_DATA_ATTR double gps_latitude, gps_longitude;
extern RTC_DATA_ATTR int gps_fix_dd, gps_fix_mm, gps_fix_yy;

extern char httpPostRequest[256], httpContent[12];
extern char append_text[160], store_text[160], ftpappend_text[160];
extern int cur_mode;
extern int cur_fld_no;
extern char ftp_station[16];
extern size_t len;
extern char last_logged[16];
extern char http_data[512]; 
extern char content[512], content1[512];
extern char sample_cum_rf[10], sample_inst_rf[10], sample_temp[10], sample_hum[10],
    sample_avgWS[10], sample_WD[10], sample_bat[10], ftpsample_avgWS[10],
    ftpsample_cum_rf[10];
extern char ht_data[80]; // AG1
extern char apn_str[20];
extern char reg_status[16];
extern char *reg_status_ptr;
extern char reg_val[3];
extern char rssi_resp[10];
extern uint8_t rssi_val;
extern RTC_DATA_ATTR char carrier[20];
extern RTC_DATA_ATTR char sim_number[20];
extern RTC_DATA_ATTR char cached_iccid[25];
extern RTC_DATA_ATTR bool isLTE;

// Persistent Sensor Diagnostics (survive deep sleep) // RTC RAM variables now
// at globals.h
extern RTC_DATA_ATTR float prev_15min_temp;
extern RTC_DATA_ATTR float prev_15min_hum;
extern RTC_DATA_ATTR float prev_15min_ws;
extern RTC_DATA_ATTR int temp_same_count;
extern RTC_DATA_ATTR int hum_same_count;

extern RTC_DATA_ATTR int diag_reg_time_total;
extern RTC_DATA_ATTR int diag_backlog_total;
extern RTC_DATA_ATTR int diag_reg_count;
extern RTC_DATA_ATTR int diag_reg_worst;
extern RTC_DATA_ATTR int diag_gprs_fails;
extern RTC_DATA_ATTR int diag_modem_mutex_fails;
extern RTC_DATA_ATTR int diag_last_reset_reason;
extern RTC_DATA_ATTR bool diag_rtc_battery_ok;
extern RTC_DATA_ATTR int diag_consecutive_reg_fails;
extern RTC_DATA_ATTR int diag_stored_apn_fails;
extern RTC_DATA_ATTR int diag_consecutive_sim_fails;

// Golden Summary Diagnostic Flags (v5.43)
extern RTC_DATA_ATTR int diag_ws_same_count;
extern RTC_DATA_ATTR bool diag_temp_cv;
extern RTC_DATA_ATTR bool diag_hum_cv;
extern RTC_DATA_ATTR bool diag_ws_cv;
extern RTC_DATA_ATTR bool diag_temp_erv;
extern RTC_DATA_ATTR bool diag_hum_erv;
extern RTC_DATA_ATTR bool diag_ws_erv;
extern RTC_DATA_ATTR bool diag_temp_erz;
extern RTC_DATA_ATTR bool diag_hum_erz;
extern RTC_DATA_ATTR bool diag_rain_jump;
extern RTC_DATA_ATTR float diag_last_rf_val;
extern RTC_DATA_ATTR bool diag_rain_calc_invalid;
extern RTC_DATA_ATTR bool diag_rain_reset;
extern RTC_DATA_ATTR bool diag_wd_fail; // v5.59: WD Disconnection tracker
extern RTC_DATA_ATTR int diag_consecutive_http_fails;
extern RTC_DATA_ATTR int diag_daily_http_fails; // Total failures today
extern RTC_DATA_ATTR int diag_http_present_fails;
extern RTC_DATA_ATTR int diag_http_cum_fails;
extern RTC_DATA_ATTR int diag_cum_fail_reset_month;
extern RTC_DATA_ATTR int diag_rejected_count;
extern RTC_DATA_ATTR bool diag_sensor_fault_sent_today;
extern RTC_DATA_ATTR char diag_crash_task[16]; // v5.59: Survive restart

// Accuracy Counters (v5.49): Use 32-bit accumulators to prevent 16-bit ULP
// wraps and spikes
extern RTC_DATA_ATTR uint32_t total_wind_pulses_32;
extern RTC_DATA_ATTR uint32_t last_sched_wind_pulses_32;
extern RTC_DATA_ATTR uint16_t last_raw_wind_count; // Raw 16-bit ULP snapshot
extern RTC_DATA_ATTR uint32_t total_rf_pulses_32;
extern RTC_DATA_ATTR uint32_t last_sched_rf_pulses_32;
extern RTC_DATA_ATTR uint16_t last_raw_rf_count; // Raw 16-bit ULP snapshot

// v5.49 Splitted Diagnostic Trackers for Golden Data Reporting
extern RTC_DATA_ATTR int diag_ndm_count;
extern RTC_DATA_ATTR int diag_ndm_count_prev;
extern RTC_DATA_ATTR char diag_cdm_status[20];
extern RTC_DATA_ATTR int diag_pd_count;
extern RTC_DATA_ATTR int diag_pd_count_prev;
extern RTC_DATA_ATTR int diag_first_http_count;
extern RTC_DATA_ATTR int diag_first_http_count_prev;
extern RTC_DATA_ATTR int diag_net_data_count;
extern RTC_DATA_ATTR int diag_net_data_count_prev;
extern RTC_DATA_ATTR int diag_last_rollover_day;

extern RTC_DATA_ATTR uint32_t diag_http_time_total;
extern RTC_DATA_ATTR uint32_t diag_ftp_time_total;
extern RTC_DATA_ATTR uint32_t diag_sent_mask_cur[3];
extern RTC_DATA_ATTR uint32_t diag_sent_mask_prev[3];
extern RTC_DATA_ATTR int diag_http_success_count;
extern RTC_DATA_ATTR int diag_http_success_count_prev;
// v7.53: Retry / FTP success counters (separate from primary current-slot)
extern RTC_DATA_ATTR int diag_http_retry_count;
extern RTC_DATA_ATTR int diag_http_retry_count_prev;
extern RTC_DATA_ATTR int diag_ftp_success_count;
extern RTC_DATA_ATTR int diag_ftp_success_count_prev;
extern RTC_DATA_ATTR char diag_reg_fail_type[16];
extern RTC_DATA_ATTR char diag_http_fail_reason[16];

// DNS IP Caching to prevent DNS lookup every 15 mins (Saves ~1.5 seconds
// modem-on time)
extern RTC_DATA_ATTR char cached_server_ip[32];
extern RTC_DATA_ATTR char cached_server_domain[64];
extern RTC_DATA_ATTR bool dns_fallback_active;
extern RTC_DATA_ATTR int preferred_ftp_mode;
extern RTC_DATA_ATTR bool healer_reboot_in_progress;

// Health Report Retry Logic (persists across deep sleep)
extern RTC_DATA_ATTR int health_last_sent_hour;
extern RTC_DATA_ATTR int health_last_sent_day;
extern RTC_DATA_ATTR bool diag_fw_just_updated;
extern RTC_DATA_ATTR bool rtc_daily_sync_done;

extern volatile bool gprs_started;
extern int badReads;

// ULP Memory Map (Manual offsets to ensure hardware reachability)
#define U_RF_COUNT 512
#define U_CUR_STATE 513
#define U_PREV_STATE 514
#define U_DEBOUNCED_STATE 515
#define U_DEBOUNCE_CNT 516
#define U_WIND_COUNT 517
#define U_WIND_PREV_STATE 518
#define U_CALIB_COUNT 519
#define U_CALIB_MODE 520
#define U_WIND_DEBOUNCED_STATE 521
#define U_WIND_DEBOUNCE_CNT 522

extern int ULP_WAKEUP_TC; // ulp runs every 1ms (High Resolution for Wind)

// Accessors for ULP variables (Mapped to Word Offsets in RTC_SLOW_MEM)
#define rf_count (*(volatile ulp_var_t *)&RTC_SLOW_MEM[U_RF_COUNT])
#define cur_state (*(volatile ulp_var_t *)&RTC_SLOW_MEM[U_CUR_STATE])
#define prev_state (*(volatile ulp_var_t *)&RTC_SLOW_MEM[U_PREV_STATE])
#define debounced_state                                                        \
  (*(volatile ulp_var_t *)&RTC_SLOW_MEM[U_DEBOUNCED_STATE])
#define debounce_cnt (*(volatile ulp_var_t *)&RTC_SLOW_MEM[U_DEBOUNCE_CNT])
#define wind_count (*(volatile ulp_var_t *)&RTC_SLOW_MEM[U_WIND_COUNT])
#define wind_prev_state                                                        \
  (*(volatile ulp_var_t *)&RTC_SLOW_MEM[U_WIND_PREV_STATE])
#define calib_count (*(volatile ulp_var_t *)&RTC_SLOW_MEM[U_CALIB_COUNT])
#define calib_mode_flag (*(volatile ulp_var_t *)&RTC_SLOW_MEM[U_CALIB_MODE])
#define wind_debounced_state (*(volatile ulp_var_t *)&RTC_SLOW_MEM[U_WIND_DEBOUNCED_STATE])
#define wind_debounce_cnt (*(volatile ulp_var_t *)&RTC_SLOW_MEM[U_WIND_DEBOUNCE_CNT])

extern RTC_DATA_ATTR int current_year, current_month, current_day, current_hour,
    current_min, current_sec, previous_min, record_hr, record_min,
    previous_rfclose_year, previous_rfclose_month, previous_rfclose_day,
    calib_year, calib_month, calib_day, calib_sts;
extern RTC_DATA_ATTR int firmwareUpdated;
extern RTC_DATA_ATTR int last_processed_sample_idx;
extern RTC_DATA_ATTR bool fresh_boot_check_done;
extern RTC_DATA_ATTR bool apn_saved_this_sim;
extern RTC_DATA_ATTR float last_valid_temp;
extern RTC_DATA_ATTR float last_valid_hum;
extern RTC_DATA_ATTR int last_valid_wd;        // v5.59: WD Rescue Anchor
extern RTC_DATA_ATTR float rtc_daily_cum_rf; // v5.59: RF Golden Anchor

// RF
extern float rf_value, current_rf_value, calib_rf_float;
extern int rf_res_edit_state; // 0:Idle, 1:Password, 2:Editing
extern char cum_rf[10], inst_rf[10], inst_temp[10], avg_cum_rf[10];
extern char ftpcum_rf[10];
extern float avg_cumRF, new_current_cumRF, new_current_instRF;

// T/H , WS and WD
extern int prev_wind_count;
extern float temperature, humidity, windSpCount, cur_wind_speed, pressure;
extern float cur_avg_wind_speed;
extern int windDir;
extern float sea_level_pressure;
extern char inst_hum[10], avg_wind_speed[10], inst_wd[10];
extern char pres_str[20];
extern int pcb_clear_state; // 0:Idle, 1:Confirming

// RTC
extern volatile bool rtcReady;

extern RTC_DATA_ATTR int last_recorded_dd;
extern RTC_DATA_ATTR int last_recorded_mm;
extern RTC_DATA_ATTR int last_recorded_yy;
extern RTC_DATA_ATTR int last_recorded_hr;
extern RTC_DATA_ATTR int last_recorded_min;

extern char signature[20]; // 2023-02-23,11:15
extern volatile bool rtcTimeChanged;
extern RTC_DATA_ATTR bool signature_valid;
extern RTC_DATA_ATTR bool pending_manual_status;
extern RTC_DATA_ATTR bool pending_manual_gps;
extern RTC_DATA_ATTR bool pending_manual_health;

// LCD and Navigation
extern volatile char show_now;
extern volatile int wakeup_reason_is; // ACTIVE: used by all wakeup logic
extern volatile int lcdkeypad_start;
extern int wired;
extern int temp_count_rf, calib_count_rf, calib_flag;

extern char rf_str[10], calib_rf[10], temp_str[16], hum_str[16], windSpeedInst_str[16],
    prevWindSpeedAvg_str[7], windDir_str[16];
extern char date_now[11];
extern char time_now[6];

// FTP
// const char* ftpServer = "ftp.spatika.net";
// const char* ftpUser = "hmisadmin";
// const char* ftpPassword = "spt2015";
// int port = 21;
// int calib_sts;//CLB-FAIL  - 0 or CLB-OK - 1

/*
 *  Prototypes
 */

// Task Prototypes
void lcdkeypad(void *pvParameters);
void gprs(void *pvParameters);
void tempHum(void *pvParameters);
void bmeTask(void *pvParameters);
void windSpeed(void *pvParameters);
void windDirection(void *pvParameters);
// Task Handles (for stack monitoring and state tracking)
extern TaskHandle_t scheduler_h, gprs_h, lcdkeypad_h, rtcRead_h;
extern TaskHandle_t tempHum_h, bmeTask_h, windSpeed_h, windDirection_h;
extern volatile bool ota_silent_mode;
extern volatile bool bearer_recovery_active;
extern int active_cid;

void rtcRead(void *pvParameters);

// Function Prototypes
void initialize_hw();
bool initBME();
void set_rtc_time();
void set_wakeup_reason();
// initiate_espnow(), OnDataSent(), OnDataRecv() removed - ESP-NOW disabled
// (v5.40+)
void send_at_cmd(char *cmd, char *check, char *spl);
void resync_time();
char *parse_http_head(char *response, char *check);
void next_date(int *Nd, int *Nm, int *Ny);
void previous_date(int *Cd, int *Cm, int *Cy);
int send_at_cmd_data(char *payload, String response_arg);
void send_http_data();
bool send_health_report(bool useJitter = true);
void send_unsent_data();
void send_ftp_file(char *fileName, bool isDailyFTP, bool alreadyLocked = false);
void start_gprs();
void send_sms();
void process_sms(char msg_no);
int setup_ftp(int transMode = 0);
void fetchFromHttpAndUpdate(char *fileName);
void copyFromSPIFFSToFS(char *dateFile);
void loadGPS();
// I2C Protection (v5.49)


// MODEM / GPRS
String waitForResponse(String expectedResponse, int timeout);
void disableWDT();
void saveYearToSPIFFS(int year);
void configure_sensors_for_awake();
void configure_sensors_for_sleep();
void copyFilesFromSPIFFSToSD(const char *dirname);
void copyFile(const char *sourcePath, const char *destPath);
void flushSerialSIT();
bool copyFile_legacy(String fileName);
void validate_ulp_counters();
int read_line(char *src, char *dest, int max_len, char delim_chr);
void parse_and_convert_clbs_response(const char *response, int year1,
                                     int month1, int day1, int hour1,
                                     int minute1, int seconds1);
bool initHDC();
bool readHDC(float &tempC, float &humidity);
bool isDigitStr(const char *s);

// GPRS / SMS / GPS Helper Prototypes
void get_signal_strength();
void get_network();
void get_registration();
void get_a7672s();
void prepare_and_send_status(char *number);
void get_lat_long_date_time(char *number);
void store_current_unsent_data();
void get_gps_coordinates();
void prepare_data_and_send();
void power_cut_modem_shutdown();
int send_at_cmd_data(char *payload, String response_arg, bool robust);

// GPRS Helpers (gprs_helpers.ino)
String get_ccid();
bool load_apn_config(String current_ccid, char *target_apn, size_t max_len);
bool try_activate_apn(const char *apn);
bool verify_bearer_or_recover();
void save_apn_config(String apn, String ccid);

/*
 *   Structure
 */

// Use a structure to hold sensor data. This makes it easy to pass between
// tasks.
struct SensorData {
  float temperature;
  float humidity;
  float windSpeed;
  // windDirection is intentionally excluded - always use the global `windDir`
};

extern SensorData latestSensorData; // definition in AIO9_5.0.ino

enum UI_FIELD_ID {
  FLD_STATION = 0,
  FLD_DATE,
  FLD_TIME,
  FLD_VERSION,
  FLD_RF_ML,
  FLD_SIM_STATUS,
  FLD_REGISTRATION,
  FLD_LAST_LOGGED,
  FLD_SEND_STATUS,
  FLD_SEND_GPS,
  FLD_SEND_HEALTH,
  FLD_RF_CALIB,
  FLD_RF_RES,
  FLD_DELETE_DATA,
  FLD_SD_COPY,
  FLD_BATTERY,
  FLD_SOLAR,
  FLD_WIFI_ENABLE,
  FLD_LOG,
  FLD_WIND_DIR,
  FLD_INST_WS,
  FLD_AVG_WS,
  FLD_TEMP,
  FLD_HUMIDITY,
  FLD_PRESSURE,
  FLD_ALTITUDE,   // #1: Station altitude for MSLP correction (BME only)
  FLD_HTTP_FAILS, // v7.70: Present/Cumulative HTTP failure stats
  FLD_LCD_OFF,
  FLD_COUNT // Total count
};

struct ui_data_t {
  int fld_id;
  char topRow[17];
  char bottomRow[17];
  int fieldType; // 0: AlphaNum, 1: Numeric, 2: DisplayOnly, 3: Live
};
extern ui_data_t ui_data[FLD_COUNT]; // definition moved to AIO9_5.0.ino

// esp_now_peer_info_t peerInfo; // Removed - ESP-NOW disabled (v5.40+)

extern TaskHandle_t webServer_h;
void webServer(void *pvParameters);

extern char present_topRow[17];
extern char present_bottomRow[17];

struct http_params {
  char serverName[64];
  char IP[20];   // v5.55: Added static IP for DNS fallback
  char Link[64]; // This is the URL path
  char Port[8];
  char Key[15];
  char Format[22];
};

#if SYSTEM == 0
extern struct http_params httpSet[7];
#endif

#if (SYSTEM == 1) || (SYSTEM == 2)
extern struct http_params httpSet[11];
#endif

// Function Prototypes for Tasks defined in other files
void scheduler(void *parameter);
#endif

// Unsent Data to be sent is :
// 23,2027-07-16,14:30,0002.0,24.37,77.53,01.02,341,-114,04.2 Unsent Data to be
// sent is : 23,2025-07-16,14:30,0001.5,00.00,00.00,00.00,000,-112,04.2

// http_data format is
// stn_no=99999&rec_time=2027-07-16,14:30&key=rfclimate5p13&rainfall=002.0&temp=24.37&humid=77.53&w_speed=01.02&w_dir=341&signal=-114&bat_volt=04.2&bat_volt2=04.2
// - unsent good went through http_data format is
// stn_no=99999&rec_time=2025-07-16,14:30&key=rfclimate5p13&rainfall=001.5&temp=00.00&humid=00.00&w_speed=00.00&w_dir=000&signal=-112&bat_volt=04.2&bat_volt2=04.2
// - unsent did not go through
