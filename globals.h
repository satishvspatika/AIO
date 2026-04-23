#ifndef GATEWAY_GLOBALS_H
#define GATEWAY_GLOBALS_H

#include "user_config.h"

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
// Do NOT define it as empty; otherwise linker will move variables out of RTC
// RAM.
#ifndef RTC_DATA_ATTR
#define RTC_DATA_ATTR __attribute__((section(".rtc.data")))
#endif

enum HDC_Type { // Sensor type enum
  HDC_UNKNOWN,
  HDC_1080,
  HDC_2022
};

enum BME_Type { BME_UNKNOWN, BME_280 };

extern volatile bool ota_silent_mode;
// v5.81: Multi-Core Control Flags (volatile to prevent compiler optimization across tasks)
extern volatile bool bearer_recovery_active;
extern RTC_DATA_ATTR volatile bool low_bat_mode_active;
extern volatile bool gprs_active;
extern volatile bool timeSyncRequired;
extern volatile bool schedulerBusy;
extern volatile bool ota_writing_active;
extern volatile uint32_t last_activity_time; // v5.85: Safety Heartbeat Timer

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
int send_at_cmd_data(char *payload, bool robust);
void get_signal_strength();
void analyzeFileHealth(uint32_t *mask, int *outNetCount, bool *hasUnresolvedPD,
                       bool *hasUnresolvedNDM);
void reconstructSentMasks(bool alreadyLocked = false);
void markFileAsDelivered(const char *fileName, bool alreadyLocked = false);
void convert_gmt_to_ist(struct tm *gmt_time);
void sync_rtc_from_server_tm(const char *body, bool is_ntp);
void reset_all_diagnostics();
void recoverI2CBus(bool alreadyLocked = false);
void pruneFile(const char *path, size_t limit, bool alreadyLocked = false);
void cleanup_service_report(bool fatal = false);
bool send_service_report();

/************************************************************************************************/
extern char
    UNIT[15]; // (Initialized secretly in .ino via UNIT_CFG to avoid ODR issues)
extern int test_health_every_slot;
extern float RF_RESOLUTION;

/************************************************************************************************/

#define HDC_ADDR 0x40 // Default I2C address for both HDC1080 and HDC2022
#define I2C_SDA 21    // Explicit definition for I2C bus in ESP32
#define I2C_SCL 22
#define I2C_TIMEOUT_MS 2000 // I2C hardware timeout (2 seconds)
#define I2C_MUTEX_WAIT_TIME                                                    \
  800 // v5.84: Reduced from 2500 to 800 to prevent UI stalls during I2C hangs
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

// Record length constants for different systems (Audit-Hardened v5.75)
#define RECORD_LENGTH_RF 46    // SYSTEM == 0 (NN,YYYY-MM-DD,HH:MM,II.II,CC.CC,SSSY,BB.BB\r\n)
#define RECORD_LENGTH_TWS 57   // SYSTEM == 1 (NN,YYYY-MM-DD,HH:MM,TT.T,HHH.H,WW.W,DDD,SSSY,BB.BB\r\n) => 55 + 2 = 57
#define RECORD_LENGTH_TWSRF 63 // SYSTEM == 2 (NN,YYYY-MM-DD,HH:MM,RR.RR,TT.T,HHH.H,WW.W,DDD,SSSY,BB.BB\r\n) => 61 + 2 = 63

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
#define TEMP_SAME_COUNT_THRESHOLD 15
#define HUM_SAME_COUNT_THRESHOLD 15
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
extern LiquidCrystal_I2C lcd;
extern SemaphoreHandle_t i2cMutex;
extern SemaphoreHandle_t serialMutex;
extern SemaphoreHandle_t modemMutex;
extern SemaphoreHandle_t fsMutex;
extern volatile bool gprs_pdp_ready;
extern RTC_DS1307 rtc;
extern char modem_response_buf[2048]; // v6.0 Zero-Heap GPRS Buffer
extern char gprs_payload[2048];       // v6.0 Transmission Payload Buffer (Standardized v5.82 (Golden Master))
extern int gprs_payload_len;

extern portMUX_TYPE timerMux0;
extern portMUX_TYPE timerMux1;
extern portMUX_TYPE timerMux2;
extern portMUX_TYPE windMux;
extern portMUX_TYPE rtcTimeMux;
extern portMUX_TYPE syncMux;
extern portMUX_TYPE sensorDataMux;

// Keypad timing
extern unsigned long last_key_time;

// record_length moved below
extern int cur_file_found;
#if DEBUG == 1
  #define debug(...)         do { if(!ota_silent_mode && !bearer_recovery_active) Serial.print(__VA_ARGS__); } while (0)
  #define debugln(...)       do { if(!ota_silent_mode && !bearer_recovery_active) Serial.println(__VA_ARGS__); } while (0)
  #define debugf(fmt, ...)   do { if(!ota_silent_mode && !bearer_recovery_active) Serial.printf(fmt, ##__VA_ARGS__); } while (0)
  #define debugf1(fmt, ...)  debugf(fmt, ##__VA_ARGS__)
  #define debugf2(fmt, ...)  debugf(fmt, ##__VA_ARGS__)
  #define debugf3(fmt, ...)  debugf(fmt, ##__VA_ARGS__)
  #define debugf4(fmt, ...)  debugf(fmt, ##__VA_ARGS__)
  #define debugf5(fmt, ...)  debugf(fmt, ##__VA_ARGS__)
#else
  #define debug(...)
  #define debugln(...)
  #define debugf(fmt, ...)
  #define debugf1(fmt, ...)
  #define debugf2(fmt, ...)
  #define debugf3(fmt, ...)
  #define debugf4(fmt, ...)
  #define debugf5(fmt, ...)
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
};                                                // gprs_mode
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
extern volatile bool
    wd_ok; // v5.74: WD sensor health flag (written by Core 1, read by Core 0)
extern volatile int
    sampleNo; // v5.72: Global slot counter for bearer age checks
extern int temp_sampleNo, temp_day, temp_month, temp_year, temp_hr, temp_min,
    temp_sig;
// v5.70: Use __atomic_store_n/__atomic_load_n for data_writing_initiated
extern volatile int data_writing_initiated;
extern int time_to_deepsleep;
// Common
extern char UNIT_VER[32], STATION_TYPE[10], NETWORK[15];
extern char universalNumber[20];
// --- Process Control Flags (v5.66 ODR Pass) ---
extern HDC_Type hdcType;
extern BME_Type bmeType;
extern volatile bool health_in_progress;
extern volatile bool
    schedulerBusy; // Prevents sleep during 15-min slot processing
extern volatile bool primary_data_delivered;
extern volatile bool local_svc_upload_active; // v5.93: Priority flag for onsite engineers
extern volatile RTC_DATA_ATTR bool skip_primary_http;
extern volatile bool sleep_sequence_active;    // v5.77: Sleep Gate signal
extern volatile bool force_ftp;               // v5.77: RESTORED
extern volatile bool force_ftp_daily;
extern RTC_DATA_ATTR int rtc_daily_sync_count; // v5.77: Daily sync retry cap
extern char ftp_daily_date[12];
extern volatile bool force_reboot;
extern volatile bool force_ota;
extern volatile bool force_gps_refresh;
extern volatile bool force_clear_ftp_queue;
extern volatile bool force_delete_data;
// v5.87: ota_writing_active deduplicated (defined at line 84)
extern char hw_tag; // v5.75: Passive hardware/VFS health tag
extern bool
    http_ready; // v5.42: Tracks HTTPINIT success; defined in gprs_core.ino
extern int ota_fail_count;
extern char ota_fail_reason[48];
extern char ota_cmd_param[128];
extern RTC_DATA_ATTR int last_cmd_id;
extern RTC_DATA_ATTR char last_cmd_res[64];
extern char cur_file[32], unsent_file[32], new_file[32], temp_file[50];
extern char station_name[16];
extern char last_fw_ver[32]; // v5.87: Standardized to [32]
extern char ftp_station[16];  // v5.86: RESTORED
extern char last_logged[16];  // v5.86: RESTORED
extern size_t len;            // v5.86: RESTORED

extern char chip_id[13];
extern char calib_state[5], calib_text[40], calib_content[16];
extern char ftpunsent_file[50];
extern char ftpdaily_file[50];
extern int ftp_login_flag;
extern int calib_header_drawn;
// GPRS
extern int http_no, msg_sent;
extern int rssiIndex, rssiEndIndex, registration;
extern volatile int retries; // cross-task: written by GPRS, read by scheduler
extern volatile int unsent_count, success_count; // cross-task shared counters
extern volatile int s, fileSize;                 // cross-task file-size helpers
extern int delay_val;                            // Delay before starting GPRS
extern volatile int signal_strength; // cross-task: GPRS writes, scheduler reads
extern volatile int signal_lvl;      // cross-task: GPRS writes, scheduler reads
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
extern int
    last_lcd_state; // v5.70: Fix mysterious visibility issue
extern char signature[20]; // 2023-02-23,11:15
extern char present_topRow[17];
extern char present_bottomRow[17]; // v5.86: Standardized grouping
extern char http_data[350];
extern char sample_cum_rf[10], sample_inst_rf[10], sample_temp[10],
    sample_hum[10], sample_avgWS[10], sample_WD[10], sample_bat[10],
    ftpsample_avgWS[10], ftpsample_cum_rf[10];
extern char ht_data[80]; // AG1
extern char apn_str[32]; // v5.75: Expanded from 20 to 32 (M-03 fix)
extern char reg_status[16];
extern char *reg_status_ptr;
extern char reg_val[3];
extern char rssi_resp[10];
extern uint8_t rssi_val;
extern RTC_DATA_ATTR char carrier[32]; // v5.75: Expanded from 20 to 32 (M-03 fix)
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
extern RTC_DATA_ATTR int diag_backlog_total, diag_backlog_total_prev;
extern RTC_DATA_ATTR int diag_reg_count;
extern RTC_DATA_ATTR int diag_reg_worst;
extern RTC_DATA_ATTR int diag_gprs_fails;
extern RTC_DATA_ATTR int diag_modem_mutex_fails;
extern RTC_DATA_ATTR int diag_last_reset_reason;
extern RTC_DATA_ATTR bool diag_rtc_battery_ok;
extern RTC_DATA_ATTR int diag_consecutive_reg_fails;
extern RTC_DATA_ATTR int diag_stored_apn_fails;
extern RTC_DATA_ATTR int diag_consecutive_sim_fails;
extern RTC_DATA_ATTR int last_successful_cnmp;
extern RTC_DATA_ATTR bool last_http_ok;
extern RTC_DATA_ATTR int last_http_ok_slot; // v5.72: Stale bearer detection
extern RTC_DATA_ATTR int
    last_unsent_sampleNo; // v5.72: Dedup guard for unsent.txt
extern RTC_DATA_ATTR int gprs_2g_slots_count;
extern RTC_DATA_ATTR int low_bat_skip_count;
extern RTC_DATA_ATTR volatile int diag_http_zombie_count;
extern RTC_DATA_ATTR bool
    backfill_done; // v5.72 Hardened: Architecture fix for Health TX reporting

enum SVC_SYNC_STATUS {
  SVC_IDLE = 0,
  SVC_PENDING = 1,
  SVC_SYNC_META = 2,
  SVC_SYNC_IMG1 = 3,
  SVC_SYNC_IMG2 = 4,
  SVC_DONE = 5,
  SVC_FAIL = 6
};

extern char svc_last_error[48]; // v5.98: Detailed failure reason for UI/Serial
extern unsigned long last_svc_trigger_time; // v5.88: for timeout tracking

// Golden Summary Diagnostic Flags (v5.43)
extern RTC_DATA_ATTR int diag_ws_same_count;
extern volatile int svc_sync_status; // v5.87: 0:Idle, 1:Pending, 2:Syncing, 3:Done, 4:Failed (See SVC_SYNC_STATUS)
extern RTC_DATA_ATTR int svc_retry_count; // v5.91: Track sync attempts for fatal scrub
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
extern RTC_DATA_ATTR int last_ftp_unsent_sampleNo;
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
// v5.85: badReads relocated to main process block

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
#define wind_debounced_state                                                   \
  (*(volatile ulp_var_t *)&RTC_SLOW_MEM[U_WIND_DEBOUNCED_STATE])
#define wind_debounce_cnt                                                      \
  (*(volatile ulp_var_t *)&RTC_SLOW_MEM[U_WIND_DEBOUNCE_CNT])

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
extern RTC_DATA_ATTR int last_valid_wd;      // v5.59: WD Rescue Anchor
extern RTC_DATA_ATTR float rtc_daily_cum_rf; // v5.59: RF Golden Anchor

// RF
extern float rf_value, current_rf_value, calib_rf_float;
extern int rf_res_edit_state; // 0:Idle, 1:Password, 2:Editing
extern char cum_rf[10], inst_rf[10], inst_temp[10], avg_cum_rf[10];
extern char ftpcum_rf[10];
extern float avg_cumRF, new_current_cumRF, new_current_instRF;

// T/H , WS and WD
extern int prev_wind_count;
extern volatile float temperature, humidity, windSpCount, cur_wind_speed,
    pressure;
extern float cur_avg_wind_speed;
extern volatile int windDir; // written by windDirection task, read by scheduler
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

// v5.87: signature deduplicated (defined at line 348)
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

extern char rf_str[10], calib_rf[10], temp_str[16], hum_str[16],
    windSpeedInst_str[16], prevWindSpeedAvg_str[7], windDir_str[16];
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
// --- Core Task & Communication Handles (v5.85 Definitive Block) ---
extern TaskHandle_t scheduler_h, gprs_h, lcdkeypad_h, rtcRead_h;
extern TaskHandle_t tempHum_h, bmeTask_h, windSpeed_h, windDirection_h, webServer_h;
extern volatile bool gprs_started; 
extern int badReads;
extern int active_cid; // v5.85 Fix: Restored cross-file visibility
extern portMUX_TYPE
    syncMux; // v5.70: Cross-core atomic protection for sync_mode

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
void get_p_file_info(char *pfn, int *pdd, int *pmm, int *pyy);
void get_c_file_info(char *cfn, int *cdd, int *cmm, int *cyy);
void getTimeSnapshot(struct tm *timeinfo); // v5.79: Hardened Time Sync
int send_at_cmd_data(char *payload, bool robust);
void send_http_data();
bool send_health_report(bool useJitter = true);
void send_unsent_data();
void send_ftp_file(char *fileName, bool isDailyFTP, bool alreadyLocked = false);
void start_gprs();
void send_sms(bool alreadyLocked = false);
void process_sms(char msg_no);
int setup_ftp(int transMode = 0);
void fetchFromHttpAndUpdate(char *fileName, bool alreadyLocked = false);
void copyFromSPIFFSToFS(char *dateFile, bool alreadyLocked = false);
void loadGPS();
void sync_rtc_from_http_header();
// I2C Protection (v5.49)

// MODEM / GPRS
bool waitForResponse(const char *expected, unsigned long timeout);
void disableWDT();
void trim_whitespace(char *str);
void saveYearToSPIFFS(int year);
void configure_sensors_for_awake();
void configure_sensors_for_sleep();
void copyFilesFromSPIFFSToSD(const char *dirname);
void copyFile(const char *sourcePath, const char *destPath,
              bool alreadyLocked = false); // v5.75
void flushSerialSIT();
bool copyFile_legacy(String fileName);
void validate_ulp_counters();
int read_line_to_buf(File &f, char *buf, size_t max_len);
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
void prepare_and_send_status(char *number, bool alreadyLocked = false);
void get_lat_long_date_time(char *number, bool alreadyLocked = false);
void store_current_unsent_data();
void get_gps_coordinates(bool alreadyLocked = false);
void check_incoming_sms(bool alreadyLocked = false);
void prepare_data_and_send();
void power_cut_modem_shutdown();
// (prototype above at line 90)

// GPRS Helpers (gprs_helpers.ino)
void get_ccid(char *out, size_t maxLen);
bool load_apn_config(const char* current_ccid, char *target_apn, size_t max_len);
void save_apn_config(const char* apn, const char* ccid);
bool verify_bearer_or_recover();
bool try_activate_apn(const char *apn);


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

void webServer(void *pvParameters);

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
