#ifndef GATEWAY_GLOBALS_H
#define GATEWAY_GLOBALS_H

#define ENABLE_WEBSERVER 0 // Set to 0 to remove WebServer overhead

#ifdef ARDUINO
#include "driver/rtc_io.h"
#include "esp32/ulp.h"
#include "esp_sleep.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
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
#include <vector>
#else
// Linter fallback for non-Arduino environment (e.g. host-side analysis)
#include <stdint.h>
#include <string>
#define RTC_DATA_ATTR
#define String std::string
#endif

#include <math.h>
#include <stdio.h>
#include <time.h>

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
extern bool httpInitiated;
extern bool health_in_progress;
extern bool timeSyncRequired;
extern volatile bool
    schedulerBusy; // v5.65: Prevents sleep during 15-min slot processing
extern volatile bool
    primary_data_delivered; // v5.51: Track session success for backlog gating
extern volatile bool skip_primary_http; // v7.88: Skip live upload on duplicates
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
extern int last_cmd_id;               // v7.92: Command ID for feedback
extern char last_cmd_res[64];         // v7.92: Result message for feedback
extern volatile bool ota_silent_mode; // Rule 43: Stop all log leakage

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
void reconstructSentMasks();
void markFileAsDelivered(const char *fileName);
void convert_gmt_to_ist(struct tm *gmt_time);
void sync_rtc_from_server_tm(const char *body, bool is_ntp);
void reset_all_diagnostics();
void recoverI2CBus();

/************************************************************************************************/
#define SYSTEM 1              // SYSTEM : TRG=0 TWS=1 TWS-RF=2
char UNIT[15] = "KSNDMC_TWS"; // UNIT :
//                                0:  KSNDMC_TRG  BIHAR_TRG
//                                1:  KSNDMC_TWS KSNDMC_TWS-AP
//                                2:  KSNDMC_ADDON SPATIKA_GEN
// Optional KSNDMC_ORG BIHAR_TEST

// FIRMWARE VERSION - Change here to update all version strings
#define FIRMWARE_VERSION "5.65"

#define DEBUG 1 // Set to 1 for serial debug, 0 for production (Saves space)

#define ENABLE_PRESSURE_SENSOR 0 // Set to 0 to disable BMP/BME
#define ENABLE_HEALTH_REPORT                                                   \
  1 // Master Switch: Set to 1 to enable automated health reporting
#define TEST_HEALTH_DEFAULT                                                    \
  0 // Default: 1 (Enabled - 15 min), 0 (Disabled - Daily)
int test_health_every_slot = TEST_HEALTH_DEFAULT;
#define TEST_HEALTH_EVERY_SLOT test_health_every_slot

#define DEFAULT_RF_RESOLUTION 0.5
float RF_RESOLUTION = DEFAULT_RF_RESOLUTION;
#define ENABLE_CALIB_TEST 0 // 1 to enable CALIB TEST in UI, 0 for Field only

// 1. SYSTEM ==0 and UNIT as KSNDMC_TRG or BIHAR_TRG or SPATIKA_GEN
// 2. SYSTEM ==1 and UNIT as KSNDMC_TWS
// 3. SYSTEM ==2 and UNIT as KSNDMC_ADDON or SPATIKA_GEN

/************************************************************************************************/

#define FILLGAP 1
#define FORCE_2G_ONLY                                                          \
  1 // v7.54: BSNL Fallback. Forces AT+CNMP=13 on boot to skip 60s of failed 4G
    // auto-negotiation
#define FTP_CHUNK_SIZE 15 // v5.52 ENH-2: Standardized chunk size for backlog

// Spatika Health Server (Contabo VPS — plain HTTP, no SSL needed)
#define HEALTH_SERVER_IP "75.119.148.192"
#define HEALTH_SERVER_PATH "/health"
#define HEALTH_SERVER_PORT "80"

#define HDC_ADDR 0x40 // Default I2C address for both HDC1080 and HDC2022
#define I2C_SDA 21    // Explicit definition for I2C bus in ESP32
#define I2C_SCL 22
#define I2C_TIMEOUT_MS 2000 // I2C hardware timeout (2 seconds)
#define I2C_MUTEX_WAIT_TIME                                                    \
  2500 // Mutex wait time (slightly longer than I2C timeout to prevent premature
       // timeout)
#define WDT_TIMEOUT 180
#define uS_TO_S_FACTOR                                                         \
  1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP                                                          \
  15 /* Time ESP32 will go to sleep (in seconds) 5mts = 300 */

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
float station_altitude_m = 0.0; // Runtime configurable (meters)
#define SEALEVEL_CALC_FACTOR 44330.769

// Fill signal strength constants
#define FILL_SIG_MIN 115
#define FILL_SIG_MAX 120

extern HardwareSerial SerialSIT;
extern LiquidCrystal_I2C lcd;
extern SemaphoreHandle_t i2cMutex;
extern SemaphoreHandle_t serialMutex;
extern SemaphoreHandle_t modemMutex;
extern SemaphoreHandle_t fsMutex;
extern RTC_DS1307 rtc;

extern portMUX_TYPE timerMux0;
extern portMUX_TYPE timerMux1;
extern portMUX_TYPE timerMux2;
extern portMUX_TYPE windMux;

// Keypad timing
unsigned long last_key_time = 0;

int record_length;
int cur_file_found = 0;
#if DEBUG == 1
#define debugln(...)                                                           \
  do {                                                                         \
    if (!ota_silent_mode)                                                      \
      Serial.println(__VA_ARGS__);                                             \
  } while (0)
#define debug(...)                                                             \
  do {                                                                         \
    if (!ota_silent_mode)                                                      \
      Serial.print(__VA_ARGS__);                                               \
  } while (0)
#define debugf1(a, x)                                                          \
  do {                                                                         \
    if (!ota_silent_mode)                                                      \
      Serial.printf(a, x);                                                     \
  } while (0)
#define debugf2(a, x, y)                                                       \
  do {                                                                         \
    if (!ota_silent_mode)                                                      \
      Serial.printf(a, x, y);                                                  \
  } while (0)
#define debugf3(a, x, y, z)                                                    \
  do {                                                                         \
    if (!ota_silent_mode)                                                      \
      Serial.printf(a, x, y, z);                                               \
  } while (0)
#define debugf4(a, x, y, z, p)                                                 \
  do {                                                                         \
    if (!ota_silent_mode)                                                      \
      Serial.printf(a, x, y, z, p);                                            \
  } while (0)
#define debugf5(a, x, y, z, p, q)                                              \
  do {                                                                         \
    if (!ota_silent_mode)                                                      \
      Serial.printf(a, x, y, z, p, q);                                         \
  } while (0)
#define debugf(a, ...)                                                         \
  do {                                                                         \
    if (!ota_silent_mode)                                                      \
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
  eStartupGPS
}; // sync_mode
enum {
  eGprsInitial,
  eGprsSignalOk,
  eGprsSignalForStoringOnly,
  eGprsSleepMode
};                                    // gprs_mode
volatile bool gprs_pdp_ready = false; // Add flag for PDP context readiness
enum { eCurrentData, eClosingData, eUnsentData }; // for http

enum { eEditOff, eEditOn };                   // lcdkeypad
enum { eCursorOff, eCursorUl, eCursorBlink }; // lcdkeypad

/*
 *  Variabe declaration
 */
bool webServerStarted = false;
bool wifi_active = false;
unsigned long last_wifi_activity_time = 0;
float temp_crf, temp_instrf, temp_bat, temp_temp, temp_hum, temp_avg_ws;
int temp_sampleNo, temp_day, temp_month, temp_year, temp_hr, temp_min, temp_sig,
    temp_dir;
int unsent_pointer_count;
int data_writing_initiated = 0;
int time_to_deepsleep = 13;
// Common
char UNIT_VER[20], STATION_TYPE[10], NETWORK[10]; // TRG8-3.0.5
char universalNumber[20];
bool timeSyncRequired = true;
bool httpInitiated = false;
char *startptr;
int send_status;
bool valid_dt, valid_time;
// SCHEDULER (GLOBAL)
volatile int sync_mode, gprs_mode, data_mode;
int sampleNo;
int rf_cls_dd, rf_cls_mm,
    rf_cls_yy; // this is for storing the rf close date from RTC date
int rf_cls_ram_dd, rf_cls_ram_mm,
    rf_cls_ram_yy; // this is for storing the rf close date from last recorded
                   // data from NVRAM
int time_to_sleep;

float bat_val = 0.0;

char battery[10], solar_sense[10];
int solar_conn = 0;

volatile bool schedulerBusy =
    false; // v5.65: Prevents sleep during 15-min slot processing
char cur_file[32], unsent_file[32], new_file[32],
    temp_file[50]; // Dddmmyyyy.txt
char station_name[16];
char chip_id[13]; // Unique ESP32 Chip ID
char calib_state[5], calib_text[40], calib_content[16];
String content;
char ftpunsent_file[50];
char ftpdaily_file[50];
int ftp_login_flag;
int calib_header_drawn = 0;
// GPRS
int http_no, msg_sent;
int rssiIndex, rssiEndIndex, retries, registration;
int unsent_count, success_count;
int s, fileSize;
int unsent_counter = 0;
int http_code = -1;
int delay_val = 10000; // Delay before starting GPRS (Reduced from 15s)
int signal_strength = SIGNAL_STRENGTH_NO_DATA;
int signal_lvl = SIGNAL_STRENGTH_NO_DATA;
int sd_card_ok = 0;
int send_daily = 0;
float solar_val, solar;
float li_bat, li_bat_val;

RTC_DATA_ATTR double lati,
    longi; // v5.65: Changed to double for coordinate precision
RTC_DATA_ATTR double gps_latitude, gps_longitude;
RTC_DATA_ATTR int gps_fix_dd = 0, gps_fix_mm = 0,
                  gps_fix_yy = 0; // GPS age tracking

char httpPostRequest[256], httpContent[12];
char append_text[160],
    store_text[160];      // Increased to 160 for better safety
char ftpappend_text[160]; // Increased to 160 for better safety
int cur_mode = 0;
int cur_fld_no = 0;
char ftp_station[16]; // AG2 from int so that alphanumeric can be stored
size_t len;
char last_logged[16];
char http_data[350]; // AG1
char sample_cum_rf[10], sample_inst_rf[10], sample_temp[10], sample_hum[10],
    sample_avgWS[10], sample_WD[10], sample_bat[10], ftpsample_avgWS[10],
    ftpsample_cum_rf[10];
char ht_data[80]; // AG1
char apn_str[20];
char reg_status[16];
RTC_DATA_ATTR char carrier[20] = "";
RTC_DATA_ATTR char sim_number[20] = "NA";
RTC_DATA_ATTR char cached_iccid[25] = ""; // To detect SIM change
RTC_DATA_ATTR bool isLTE = false;         // Track if on LTE vs GSM fallback

// Persistent Sensor Diagnostics (survive deep sleep) // RTC RAM variables now
// at globals.h
RTC_DATA_ATTR float prev_15min_temp = INITIAL_PREV_TEMP;
RTC_DATA_ATTR float prev_15min_hum = INITIAL_PREV_HUM;
RTC_DATA_ATTR float prev_15min_ws = 0.0;
RTC_DATA_ATTR int temp_same_count = 0;
RTC_DATA_ATTR int hum_same_count = 0;

// Field Diagnostic Insights (v5.2) - Tracked across resets
RTC_DATA_ATTR int diag_reg_time_total = 0; // Cumulative seconds
RTC_DATA_ATTR int diag_backlog_total = 0;  // v7.70
RTC_DATA_ATTR int diag_reg_count = 0;      // Number of cycles
RTC_DATA_ATTR int diag_reg_worst = 0;      // Max seconds for single reg
RTC_DATA_ATTR int diag_gprs_fails = 0;     // Total registration timeouts
RTC_DATA_ATTR int diag_modem_mutex_fails =
    0; // v5.55: Resource contention tracker
RTC_DATA_ATTR int diag_last_reset_reason = 0;
RTC_DATA_ATTR bool diag_rtc_battery_ok = true;
RTC_DATA_ATTR int diag_consecutive_reg_fails = 0;
RTC_DATA_ATTR int diag_stored_apn_fails = 0;
RTC_DATA_ATTR int diag_consecutive_sim_fails = 0;

// Golden Summary Diagnostic Flags (v5.43)
RTC_DATA_ATTR int diag_ws_same_count = 0;
RTC_DATA_ATTR bool diag_temp_cv = false;
RTC_DATA_ATTR bool diag_hum_cv = false;
RTC_DATA_ATTR bool diag_ws_cv = false;
RTC_DATA_ATTR bool diag_temp_erv = false;
RTC_DATA_ATTR bool diag_hum_erv = false;
RTC_DATA_ATTR bool diag_ws_erv = false;
RTC_DATA_ATTR bool diag_temp_erz = false;
RTC_DATA_ATTR bool diag_hum_erz = false;
RTC_DATA_ATTR bool diag_rain_jump = false;
RTC_DATA_ATTR float diag_last_rf_val = 0;
RTC_DATA_ATTR bool diag_rain_calc_invalid = false;
RTC_DATA_ATTR bool diag_rain_reset = false;
RTC_DATA_ATTR bool diag_wd_fail = false; // v5.59: WD Disconnection tracker
RTC_DATA_ATTR int diag_consecutive_http_fails = 0;
RTC_DATA_ATTR int diag_daily_http_fails = 0; // Total failures today
RTC_DATA_ATTR int diag_http_present_fails =
    0; // v7.70: Consecutive current-slot fails — resets on HTTP success
RTC_DATA_ATTR int diag_http_cum_fails =
    0; // v7.70: Cumulative monthly HTTP fails — resets on 1st of month
RTC_DATA_ATTR int diag_cum_fail_reset_month =
    -1; // v7.70: Last month cum counter was reset
RTC_DATA_ATTR int diag_rejected_count =
    0; // Track consecutive "Rejected" (Time) errors
RTC_DATA_ATTR bool diag_sensor_fault_sent_today =
    false; // v5.55: One-time fault trigger
RTC_DATA_ATTR char diag_crash_task[16] = "NONE"; // v5.59: Survive restart

// Accuracy Counters (v5.49): Use 32-bit accumulators to prevent 16-bit ULP
// wraps and spikes
RTC_DATA_ATTR uint32_t total_wind_pulses_32 = 0;
RTC_DATA_ATTR uint32_t last_sched_wind_pulses_32 =
    0; // P3 fix v5.65: Was a duplicate total_wind_pulses_32=0 — missing reset
RTC_DATA_ATTR uint16_t last_raw_wind_count = 0; // Raw 16-bit ULP snapshot
RTC_DATA_ATTR uint32_t total_rf_pulses_32 = 0;
RTC_DATA_ATTR uint32_t last_sched_rf_pulses_32 = 0;
RTC_DATA_ATTR uint16_t last_raw_rf_count = 0; // Raw 16-bit ULP snapshot

// v5.49 Splitted Diagnostic Trackers for Golden Data Reporting
RTC_DATA_ATTR int diag_ndm_count = 0;      // Today
RTC_DATA_ATTR int diag_ndm_count_prev = 0; // Reported at 09:45
RTC_DATA_ATTR char diag_cdm_status[20] =
    "PENDING"; // P8 FIX: Default to PENDING - set to OK only after confirmed
               // delivery
RTC_DATA_ATTR int diag_pd_count = 0;      // Today
RTC_DATA_ATTR int diag_pd_count_prev = 0; // Reported at 09:45
RTC_DATA_ATTR int diag_first_http_count = 0;
RTC_DATA_ATTR int diag_first_http_count_prev = 0;
RTC_DATA_ATTR int diag_net_data_count = 0;
RTC_DATA_ATTR int diag_net_data_count_prev = 0;
RTC_DATA_ATTR int diag_last_rollover_day =
    -1; // v5.49: Robust rollover tracking

RTC_DATA_ATTR uint32_t diag_http_time_total = 0; // Total ms spent in HTTP POST
RTC_DATA_ATTR uint32_t diag_ftp_time_total = 0;  // Total ms spent in FTP upload
RTC_DATA_ATTR uint32_t diag_sent_mask_cur[3] = {0, 0, 0};
RTC_DATA_ATTR uint32_t diag_sent_mask_prev[3] = {0, 0, 0};
RTC_DATA_ATTR int diag_http_success_count = 0; // Total successful HTTP attempts
RTC_DATA_ATTR int diag_http_success_count_prev = 0;
// v7.53: Retry / FTP success counters (separate from primary current-slot)
RTC_DATA_ATTR int diag_http_retry_count =
    0; // slots recovered via HTTP retry (System 0)
RTC_DATA_ATTR int diag_http_retry_count_prev = 0; // yesterday's
RTC_DATA_ATTR int diag_ftp_success_count =
    0; // slots delivered via FTP upload (System 1/2)
RTC_DATA_ATTR int diag_ftp_success_count_prev = 0; // yesterday's
RTC_DATA_ATTR char diag_reg_fail_type[16] = "NONE";
RTC_DATA_ATTR char diag_http_fail_reason[16] = "NONE";

// DNS IP Caching to prevent DNS lookup every 15 mins (Saves ~1.5 seconds
// modem-on time)
RTC_DATA_ATTR char cached_server_ip[32] = "";
RTC_DATA_ATTR char cached_server_domain[64] = "";
RTC_DATA_ATTR bool dns_fallback_active = false; // v5.55: Persist fallback state
RTC_DATA_ATTR int preferred_ftp_mode =
    -1; // v5.55: Persist working FTP mode (0=Act, 1=Pas)
RTC_DATA_ATTR bool healer_reboot_in_progress =
    false; // v5.55: Protect counters during maintenance reboots

// Health Report Retry Logic (persists across deep sleep)
RTC_DATA_ATTR int health_last_sent_hour = -1;
RTC_DATA_ATTR int health_last_sent_day = -1;
RTC_DATA_ATTR bool diag_fw_just_updated = false; // v5.76: Post-OTA Trigger
RTC_DATA_ATTR bool rtc_daily_sync_done =
    false; // v5.46: Server-time daily RTC sync

volatile bool gprs_started = false;
int badReads = 0;

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

int ULP_WAKEUP_TC = 1000; // ulp runs every 1ms (High Resolution for Wind)

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

RTC_DATA_ATTR int current_year, current_month, current_day, current_hour,
    current_min, current_sec, previous_min, record_hr, record_min,
    previous_rfclose_year, previous_rfclose_month, previous_rfclose_day,
    calib_year, calib_month, calib_day, calib_sts;
RTC_DATA_ATTR int firmwareUpdated;
RTC_DATA_ATTR int last_processed_sample_idx = -1;
RTC_DATA_ATTR bool fresh_boot_check_done = false;
RTC_DATA_ATTR bool apn_saved_this_sim =
    false; // v5.55: Guard APN SPIFFS write per SIM lifecycle
RTC_DATA_ATTR float last_valid_temp = 26.0;
RTC_DATA_ATTR float last_valid_hum = 65.0;
RTC_DATA_ATTR int last_valid_wd = 0;        // v5.59: WD Rescue Anchor
RTC_DATA_ATTR float rtc_daily_cum_rf = 0.0; // v5.59: RF Golden Anchor

// RF
float rf_value, current_rf_value, calib_rf_float;
int rf_res_edit_state = 0; // 0:Idle, 1:Password, 2:Editing
char cum_rf[10], inst_rf[10], inst_temp[10], avg_cum_rf[10];
char ftpcum_rf[10];
float avg_cumRF, new_current_cumRF, new_current_instRF;

// T/H , WS and WD
int prev_wind_count = 0;
float temperature, humidity, windSpCount, cur_wind_speed, pressure;
float cur_avg_wind_speed;
int windDir;
float sea_level_pressure;
char inst_hum[10], avg_wind_speed[10], inst_wd[10];
char pres_str[20] = "NA";
int pcb_clear_state = 0; // 0:Idle, 1:Confirming

// RTC
volatile bool rtcReady = false;

RTC_DATA_ATTR int last_recorded_dd = 0;
RTC_DATA_ATTR int last_recorded_mm = 0;
RTC_DATA_ATTR int last_recorded_yy = 0;
RTC_DATA_ATTR int last_recorded_hr = 0;
RTC_DATA_ATTR int last_recorded_min = 0;
char signature[20]; // 2023-02-23,11:15
volatile bool rtcTimeChanged = false;
RTC_DATA_ATTR bool signature_valid = false;
RTC_DATA_ATTR bool pending_manual_status =
    false; // v5.50: Queue manual SMS if busy
RTC_DATA_ATTR bool pending_manual_gps =
    false; // v5.50: Queue manual GPS if busy

// LCD and Navigation
extern volatile char show_now;
volatile int wakeup_reason_is; // ACTIVE: used by all wakeup logic
volatile int lcdkeypad_start = 0;
extern int wired;
int temp_count_rf, calib_count_rf, calib_flag;

char rf_str[10], calib_rf[10], temp_str[16], hum_str[16], windSpeedInst_str[16],
    prevWindSpeedAvg_str[7], windDir_str[16];
char date_now[11];
char time_now[6];

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
void send_ftp_file(char *fileName, bool isDailyFTP);
void start_gprs();
void send_sms();
void process_sms(char msg_no);
int setup_ftp(int transMode = 0);
void fetchFromHttpAndUpdate(char *fileName);
void copyFromSPIFFSToFS(char *dateFile);
void loadGPS();
// I2C Protection (v5.49)
void protectI2CPins();
void unprotectI2CPins();

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

SensorData latestSensorData;

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
  FLD_RF_CALIB,
  FLD_RF_RES,
  FLD_DELETE_DATA,
  FLD_SD_COPY,
  FLD_BATTERY,
  FLD_SOLAR,
  FLD_SEND_GPS,
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

typedef struct {
  int idx;
  char topRow[17];
  char bottomRow[17];
  char fieldType;
} ui_data_t;

// esp_now_peer_info_t peerInfo; // Removed - ESP-NOW disabled (v5.40+)

// Unified Layout for ALL Systems
ui_data_t ui_data[FLD_COUNT] = {
    {1, "STATION", "SIT099", eAlphaNum},             // 0
    {2, "DATE(dd-mm-yyyy)", "08-03-2023", eNumeric}, // 1
    {3, "TIME 24hr:mm", "00:00", eNumeric},          // 2
    {5, "VERSION", FIRMWARE_VERSION, eDisplayOnly},  // 3
    {4, "RF (mm)", "000.0", eLive},                  // 4
    {8, "SIM STATUS", "0", eLive},                   // 5
    {9, "REGISTRATION", "NA", eLive},                // 6
    {10, "LAST LOGGED AT", "0", eNumeric},           // 7
    {6, "SEND STATUS", "YES ?", eDisplayOnly},       // 8
    {7, "RF CALIBRATION", "Yes?", eLive},            // 9
    {25, "RF RESOLUTION", "0.50", eNumeric},         // 10
    {11, "DELETE DATA?", "YES?", eNumeric},          // 11
    {12, "COPY TO SDCARD?", "YES?", eDisplayOnly},   // 12
    {13, "BATTERY VOLTAGE", "0", eLive},             // 13
    {14, "SOLAR VOLTAGE", "0", eLive},               // 14
    {15, "SEND LAT/LONG", "YES ?", eDisplayOnly},    // 15
    {23, "ENABLE WIFI", "PRESS SET", eDisplayOnly},  // 16
    {21, "LOG (DT TM)", "20260205 08:30", eNumeric}, // 17
    {16, "WIND DIRECTION", "NA", eLive},             // 18
    {17, "INST WIND SPEED", "NA", eLive},            // 19
    {18, "AVG WIND SPEED", "NA", eLive},             // 20
    {19, "TEMPERATURE", "NA", eLive},                // 21
    {20, "HUMIDITY", "NA", eLive},                   // 22
    {24, "PRESSURE", "NA", eLive},                   // 23
    {26, "STATION ALT", "900", eNumeric},            // 24 BME only
    {27, "HTTP FAIL STATS", "                ",
     eLive},                                   // 25 v7.70: HTTP fail counters
    {22, "TURN OFF LCD", "YES?", eDisplayOnly} // 26
};

extern TaskHandle_t webServer_h;
void webServer(void *pvParameters);

char present_topRow[17];
char present_bottomRow[17];

struct http_params {
  char serverName[64];
  char IP[20];   // v5.55: Added static IP for DNS fallback
  char Link[64]; // This is the URL path
  char Port[8];
  char Key[15];
  char Format[22];
};

#if SYSTEM == 0
struct http_params httpSet[7] = {
    {"rtdas.ksndmc.net", "117.216.42.181", "/trg_gprs/update_data_sit_v2", "80",
     "sit1040", "x-www-form-urlencoded"}, // KSNDMC Original link
    {"rtdas.ksndmc.net", "117.216.42.181", "/trg_gprs/update_data_sit_v3", "80",
     "pse2420", "x-www-form-urlencoded"}, // KSNDMC Server with link to accept
                                          // backlog from HTTP
    {"rtdasbmsk.spatika.net", "164.100.130.199", "/Home/UpdateTRGData", "8085",
     "bmsk1234", "json"}, // BIHAR Govt Server
    {"rtdasbih.spatika.net", "185.250.105.225",
     "/trg_gprs/upload_bih_trg_data_new", "80", "bmsk12345",
     "json"}, // Bihar Spatika Server
    {"104.211.5.142", "104.211.5.142", "/esprain", "3002", "sit", "json"},
    {"104.211.5.142", "104.211.5.142", "/dmc_trg_data", "3003", "sit",
     "x-www-form-urlencoded"},
    {"rtdas1.spatika.net", "89.32.144.163", "/hmrtdas/trg_gen", "80",
     "sitgen100", "x-www-form-urlencoded"}, // spatika general version
};
#endif

#if (SYSTEM == 1) || (SYSTEM == 2)
struct http_params httpSet[11] = {
    {"rtdas.ksndmc.net", "117.216.42.181", "/trg_gprs/update_data_sit_v2", "80",
     "sit1040", "x-www-form-urlencoded"}, // KSNDMC Original link KSNDMC_OLD
    {"rtdas.ksndmc.net", "117.216.42.181", "/trg_gprs/update_data_sit_v3", "80",
     "pse2420", "x-www-form-urlencoded"}, // KSNDMC Server with link to accept
                                          // backlog from HTTP : KSNDMC_TRG
    {"rtdasbmsk.spatika.net", "164.100.130.199", "/Home/UpdateTRGData", "8085",
     "bmsk1234", "json"}, // BIHAR Govt Server : BIHAR_TRG
    {"rtdasbih.spatika.net", "185.250.105.225",
     "/trg_gprs/upload_bih_trg_data_new", "80", "bmsk12345",
     "json"}, // Bihar Spatika Server
    {"104.211.5.142", "104.211.5.142", "/esprain", "3002", "sit", "json"},
    {"104.211.5.142", "104.211.5.142", "/dmc_trg_data", "3003", "sit",
     "x-www-form-urlencoded"},
    {"rtdas.ksndmc.net", "117.216.42.181", "/tws_gprs/update_tws_data_v2", "80",
     "climate4p2013",
     "x-www-form-urlencoded"}, // KSNDMC Server for TWS : KSNDMC_TWS
    {"rtdas.ksndmc.net", "117.216.42.181", "/tws_gprs/update_twsrf_data_v2",
     "80", "rfclimate5p13",
     "x-www-form-urlencoded"}, // KSNDMC Server for TWS-RF : KSNDMC_ADDON
    {"rtdas.spatika.net", "144.91.104.105", "/tws_gprs/update_tws_data_v2",
     "80", "climate4p2013", "x-www-form-urlencoded"}, // Spatika Server for TWS
    {"rtdas.spatika.net", "144.91.104.105", "/tws_gprs/update_twsrf_data_v2",
     "80", "rfclimate5p13",
     "x-www-form-urlencoded"}, // Spatika Server for TWS-RF
    {"rtdas.spatika.net", "89.32.144.163", "/tws_gprs/twsrf_gen", "80",
     "wsgen2014", "x-www-form-urlencoded"}, // Spatika Server for TWS-RF
};
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
