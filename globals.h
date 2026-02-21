#ifndef GATEWAY_GLOBALS_H
#define GATEWAY_GLOBALS_H

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
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>
#include <esp_attr.h>
#ifndef RTC_DATA_ATTR
#define RTC_DATA_ATTR __attribute__((section(".rtc.data")))
#endif

enum HDC_Type { // Sensor type enum
  HDC_UNKNOWN,
  HDC_1080,
  HDC_2022
};
extern HDC_Type hdcType;
extern bool ws_ok;
extern bool wd_ok;

// SAFETY BUFFER: Reserve 512 bytes at the start of RTC Memory to prevent
// ULP Program Code (loaded at offset 0) from overwriting C variables.
RTC_DATA_ATTR uint8_t ulp_code_reserve[512] = {0};

#include <esp_now.h>
#include <esp_task_wdt.h>
#include <esp_wifi.h>
#include <hulp_arduino.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
//#include <semphr.h>

#include "driver/rtc_io.h"
#include "esp32/ulp.h"
#include "esp_sleep.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/sens_reg.h"

// CHANGE THESE FOR DIFFERENT SYSTEMS
// Function Prototypes for Cross-File Visibility
void graceful_modem_shutdown();
void start_deep_sleep();
void flushSerialSIT();
bool verify_bearer_or_recover();
int send_at_cmd_data(char *payload, String charArray);
void get_signal_strength();

/************************************************************************************************/
#define SYSTEM 1              // SYSTEM : TRG=0 TWS=1 TWS-RF=2
char UNIT[15] = "KSNDMC_TWS"; // UNIT :  KSNDMC_TRG  BIHAR_TRG  KSNDMC_TWS
                              // KSNDMC_ADDON SPATIKA_GEN
// Optional KSNDMC_ORG BIHAR_TEST

// FIRMWARE VERSION - Change here to update all version strings
#define FIRMWARE_VERSION "v5.37"

#define DEBUG 1 // Set to 1 for serial debug, 0 for production (Saves space)
#define ENABLE_ESPNOW 0 // Set to 0 to remove ESP-NOW footprint (SAVES SPACE)
#define ENABLE_WEBSERVER                                                       \
  0 // Set to 0 to remove WebServer footprint (SAVES SPACE)

#define DEFAULT_RF_RESOLUTION 0.5
float RF_RESOLUTION = DEFAULT_RF_RESOLUTION;
#define ENABLE_CALIB_TEST 0 // 1 to enable CALIB TEST in UI, 0 for Field only

// 1. SYSTEM ==0 and UNIT as KSNDMC_TRG or BIHAR_TRG or SPATIKA_GEN
// 2. SYSTEM ==1 and UNIT as KSNDMC_TWS
// 3. SYSTEM ==2 and UNIT as KSNDMC_ADDON or SPATIKA_GEN

/************************************************************************************************/

#define FILLGAP 1

// Google Sheets Health Monitor (Standard URL)
#define GOOGLE_HEALTH_URL                                                      \
  "https://script.google.com/macros/s/"                                        \
  "AKfycbx1HFoaTQVSUygYgu0WVj_P8jMImfRnnLxJQWT3GE2M3Ub8jNIilvDxp0V1J7_KTOOBWQ" \
  "/exec"

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
#define RECORD_LENGTH_TWS 53   // SYSTEM == 1
#define RECORD_LENGTH_TWSRF 60 // SYSTEM == 2

// Sample number constants
#define MAX_SAMPLE_NO 95
#define MIDNIGHT_SAMPLE_NO 60
#define SAMPLES_PER_HOUR 4
#define MINUTES_PER_SAMPLE 15

// Signal strength constants
#define SIGNAL_STRENGTH_NO_DATA -111
#define SIGNAL_STRENGTH_GAP_FILLED -112
#define SIGNAL_STRENGTH_PREV_DAY_GAP -110
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

// Fill signal strength constants
#define FILL_SIG_MIN 115
#define FILL_SIG_MAX 120

HardwareSerial SerialSIT(2);

LiquidCrystal_I2C
    lcd(0x27, 16,
        2); // set the LCD address to 0x27 for a 16 chars and 2 line display
SemaphoreHandle_t i2cMutex;
// SemaphoreHandle_t dataMutex;

RTC_DS1307 rtc; //  RTC

// Timer
portMUX_TYPE timerMux0 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timerMux1 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timerMux2 = portMUX_INITIALIZER_UNLOCKED;

// Keypad timing
unsigned long last_key_time = 0;

int record_length;
int cur_file_found = 0;
#if DEBUG == 1
#define debug(...) Serial.print(__VA_ARGS__)
#define debugln(...) Serial.println(__VA_ARGS__)
#define debugf1(a, x) Serial.printf(a, x)
#define debugf2(a, x, y) Serial.printf(a, x, y)
#define debugf3(a, x, y, z) Serial.printf(a, x, y, z)
#else
#define debug(...)
#define debugln(...)
#define debugf1(a, x)
#define debugf2(a, x, y)
#define debugf3(a, x, y, z)
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
};                                                // gprs_mode
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
int sync_mode, gprs_mode, data_mode, sampleNo;
int rf_cls_dd, rf_cls_mm,
    rf_cls_yy; // this is for storing the rf close date from RTC date
int rf_cls_ram_dd, rf_cls_ram_mm,
    rf_cls_ram_yy; // this is for storing the rf close date from last recorded
                   // data from NVRAM
int time_to_sleep;

float bat_val = 0.0;

char battery[6], solar_sense[6];
int solar_conn = 0;

char cur_file[32], unsent_file[32], new_file[32],
    temp_file[50]; // Dddmmyyyy.txt
char station_name[16];
char chip_id[13]; // Unique ESP32 Chip ID
char calib_state[5], calib_text[16], calib_content[16];
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
int delay_val =
    10000; // Delay before starting GPRS (Reduced from 15s for A7672)
int signal_strength = 0;
int signal_lvl = 0;
int sd_card_ok = 0;
int send_daily = 0;

float solar_val, solar;
float li_bat, li_bat_val;

float lati, longi;
float gps_latitude, gps_longitude;

char httpPostRequest[125], httpContent[12];
char append_text[100],
    store_text[100];      // Increased from 65 to 100 for safety #TRUEFIX
char ftpappend_text[100]; // Increased from 65 to 100 for safety #TRUEFIX
int cur_mode = 0;
int cur_fld_no = 0;
char ftp_station[16]; // AG2 from int so that alphanumeric can be stored
size_t len;
char last_logged[16];
char http_data[300]; // AG1
char sample_cum_rf[7], sample_inst_rf[7], sample_temp[7], sample_hum[7],
    sample_avgWS[7], sample_WD[4], sample_bat[5], ftpsample_avgWS[6],
    ftpsample_cum_rf[6];
char ht_data[40]; // AG1
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
RTC_DATA_ATTR int temp_same_count = 0;
RTC_DATA_ATTR int hum_same_count = 0;

// Field Diagnostic Insights (v5.2) - Tracked across resets
RTC_DATA_ATTR int diag_reg_time_total = 0; // Cumulative seconds
RTC_DATA_ATTR int diag_reg_count = 0;      // Number of cycles
RTC_DATA_ATTR int diag_reg_worst = 0;      // Max seconds for single reg
RTC_DATA_ATTR int diag_gprs_fails = 0;     // Total registration timeouts
RTC_DATA_ATTR int diag_last_reset_reason = 0;
RTC_DATA_ATTR int diag_reg_count_total_cycles = 0;
RTC_DATA_ATTR uint32_t diag_total_uptime_hrs = 0;
RTC_DATA_ATTR unsigned long diag_last_health_millis = 0;
RTC_DATA_ATTR bool diag_rtc_battery_ok = true;
RTC_DATA_ATTR int diag_consecutive_reg_fails =
    0; // Tracks slots with NO registration
RTC_DATA_ATTR int diag_stored_apn_fails =
    0; // Tracks consecutive failures of known good APN
RTC_DATA_ATTR int diag_consecutive_sim_fails = 0; // Tracks CPIN failures
RTC_DATA_ATTR int diag_consecutive_http_fails =
    0; // Tracks persistent 713/714 errors

// DNS IP Caching to prevent DNS lookup every 15 mins (Saves ~1.5 seconds
// modem-on time)
RTC_DATA_ATTR char cached_server_ip[32] = "";
RTC_DATA_ATTR char cached_server_domain[64] = "";

// Health Report Retry Logic (persists across deep sleep)
RTC_DATA_ATTR int health_last_sent_hour =
    -1; // Track last successful or attempted hour

String response;
String rssiStr;
String spiffs_stn_name;

const char *resp;
const char *charArray;
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
RTC_DATA_ATTR float last_valid_temp = 26.0;
RTC_DATA_ATTR float last_valid_hum = 65.0;

// RF
float rf_value, current_rf_value, calib_rf_float;
int rf_res_edit_state = 0; // 0:Idle, 1:Password, 2:Editing
char cum_rf[7], inst_rf[7], inst_temp[7], avg_cum_rf[7];
char ftpcum_rf[6];
float avg_cumRF, new_current_cumRF, new_current_instRF;

// T/H , WS and WD
int prev_wind_count = 0;
float temperature, humidity, windSpCount, cur_wind_speed;
float cur_avg_wind_speed;
int windDir;
char inst_hum[7], avg_wind_speed[7], inst_wd[7];

// RTC
volatile bool rtcReady = false;

RTC_DATA_ATTR int last_recorded_dd = 0;
RTC_DATA_ATTR int last_recorded_mm = 0;
RTC_DATA_ATTR int last_recorded_yy = 0;
RTC_DATA_ATTR int last_recorded_hr = 0;
RTC_DATA_ATTR int last_recorded_min = 0;
char signature[20]; // 2023-02-23,11:15
volatile bool rtcTimeChanged = false;
bool signature_valid = false;

//  ESPNOW
uint8_t destinationAddress[] = {
    0xEC, 0x94, 0xCB,
    0x6F, 0x11, 0x11}; // REPLACE WITH YOUR RECEIVER MAC Address
uint8_t newMACAddress[] = {0xEC, 0x94, 0xCB,
                           0x6F, 0x99, 0x99}; // NewMac addressof this device
volatile int espnow_start, lcdkeypad_start = 0;
volatile int wakeup_reason_is;
int temp_count_rf, calib_count_rf, calib_flag;
int send_espnow = 0;
int peer_added = 0;

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
void espnow(void *pvParameters);
void tempHum(void *pvParameters);
void windSpeed(void *pvParameters);
void windDirection(void *pvParameters);
void rtcRead(void *pvParameters);

// Function Prototypes
void initialize_hw();
void set_rtc_time();
void set_wakeup_reason();
void initiate_espnow();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
void send_at_cmd(char *cmd, char *check, char *spl);
void resync_time();
char *parse_http_head(char *response, char *check);
void next_date(int *Nd, int *Nm, int *Ny);
void previous_date(int *Cd, int *Cm, int *Cy);
int send_at_cmd_data(char *payload, String charArray);
void send_http_data();
bool send_health_report(bool useJitter = true);
void send_unsent_data();
void send_ftp_file(char *fileName);
void start_gprs();
void send_sms();
void process_sms(char msg_no);
int setup_ftp();
void fetchFromFtpAndUpdate(char *fileName);
void copyFromSPIFFSToFS(char *dateFile);
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
int read_line(char *src, char *dest, char delim_chr);
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
  int windDirection;
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
  FLD_LCD_OFF,
  FLD_COUNT // Total count
};

typedef struct {
  int idx;
  char topRow[17];
  char bottomRow[17];
  char fieldType;
} ui_data_t;

esp_now_peer_info_t peerInfo;

//        ui_data_t ui_data[22] = {
//          {1,"STATION","SIT099",eAlphaNum}, //
//          {2,"DATE(dd-mm-yyyy)","08-03-2023",eNumeric},//
//          {3,"TIME 24hr:mm","00:00",eNumeric},//
//          {4,"RF","0.0",eLive},
//          {5,"VERSION","TRG23 0.1",eDisplayOnly},
//          {6,"SEND STATUS","YES ?",eDisplayOnly},
//          {7,"RF CALIBRATION","Yes?",eLive},
//          {8,"SIGNAL LVL(dBm)","0",eLive},
//          {9,"REGISTRATION","NA",eLive},
//          {10,"LAST LOGGED AT","0",eNumeric},//
//          {11,"DELETE DATA?","YES?",eNumeric},//
//          {12,"COPY TO SDCARD?","YES?",eDisplayOnly},
//          {13,"BATTERY VOLTAGE","0",eLive},
//          {14,"SOLAR VOLTAGE","0",eLive},
//          {15,"SEND LAT/LONG","YES ?",eDisplayOnly}, //13->14
//          {16,"Wind Direction","NA",eLive},//14->15
//          {17,"INST WND SP(m/s)","NA",eLive},//15->16
//          {18,"AVG WND SP(m/s)","NA",eLive},//16->17
//          {19,"Temperature (C)","NA",eLive},//17->18
//          {20,"Humidity (%)","NA",eLive},//18->19
//          {21,"TURN OFF LCD","YES?",eDisplayOnly}
//         };

// Unified Layout for ALL Systems
ui_data_t ui_data[FLD_COUNT] = {
    {1, "STATION", "SIT099", eAlphaNum},             // 0
    {2, "DATE(dd-mm-yyyy)", "08-03-2023", eNumeric}, // 1
    {3, "TIME 24hr:mm", "00:00", eNumeric},          // 2
    {5, "VERSION", "5.1", eDisplayOnly},             // 3
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
    {16, "Wind Direction", "NA", eLive},             // 18
    {17, "INST WND SP(m/s)", "NA", eLive},           // 19
    {18, "AVG WND SP(m/s)", "NA", eLive},            // 20
    {19, "Temperature (C)", "NA", eLive},            // 21
    {20, "Humidity (%)", "NA", eLive},               // 22
    {22, "TURN OFF LCD", "YES?", eDisplayOnly}       // 23
};

extern TaskHandle_t webServer_h;
void webServer(void *pvParameters);

char present_topRow[17];
char present_bottomRow[17];

struct http_params {
  char serverName[64];
  char Link[64];
  char Port[8];
  char Key[15];
  char Format[22];
};

#if SYSTEM == 0
struct http_params httpSet[7] = {
    {"rtdas.ksndmc.net", "/trg_gprs/update_data_sit_v2", "80", "sit1040",
     "x-www-form-urlencoded"}, // KSNDMC Original link
    {"rtdas.ksndmc.net", "/trg_gprs/update_data_sit_v3", "80", "pse2420",
     "x-www-form-urlencoded"}, // KSNDMC Server with link to accept backlog from
                               // HTTP
    {"rtdasbmsk.spatika.net", "/Home/UpdateTRGData", "8085", "bmsk1234",
     "json"}, // BIHAR Govt Server
    {"rtdasbih.spatika.net", "/trg_gprs/upload_bih_trg_data_new", "80",
     "bmsk12345", "json"}, // Bihar Spatika Server
    {"104.211.5.142", "/esprain", "3002", "sit", "json"},
    {"104.211.5.142", "/dmc_trg_data", "3003", "sit", "x-www-form-urlencoded"},
    {"rtdas1.spatika.net", "/hmrtdas/trg_gen", "80", "sitgen100",
     "x-www-form-urlencoded"}, // spatika general version
};
#endif

#if (SYSTEM == 1) || (SYSTEM == 2)
struct http_params httpSet[11] = {
    {"rtdas.ksndmc.net", "/trg_gprs/update_data_sit_v2", "80", "sit1040",
     "x-www-form-urlencoded"}, // KSNDMC Original link KSNDMC_OLD
    {"rtdas.ksndmc.net", "/trg_gprs/update_data_sit_v3", "80", "pse2420",
     "x-www-form-urlencoded"}, // KSNDMC Server with link to accept backlog from
                               // HTTP : KSNDMC_TRG
    {"rtdasbmsk.spatika.net", "/Home/UpdateTRGData", "8085", "bmsk1234",
     "json"}, // BIHAR Govt Server : BIHAR_TRG
    {"rtdasbih.spatika.net", "/trg_gprs/upload_bih_trg_data_new", "80",
     "bmsk12345", "json"}, // Bihar Spatika Server
    {"104.211.5.142", "/esprain", "3002", "sit", "json"},
    {"104.211.5.142", "/dmc_trg_data", "3003", "sit", "x-www-form-urlencoded"},
    {"rtdas.ksndmc.net", "/tws_gprs/update_tws_data_v2", "80", "climate4p2013",
     "x-www-form-urlencoded"}, // KSNDMC Server for TWS : KSNDMC_TWS
    {"rtdas.ksndmc.net", "/tws_gprs/update_twsrf_data_v2", "80",
     "rfclimate5p13",
     "x-www-form-urlencoded"}, // KSNDMC Server for TWS-RF : KSNDMC_ADDON
    {"rtdas.spatika.net", "/tws_gprs/update_tws_data_v2", "80", "climate4p2013",
     "x-www-form-urlencoded"}, // Spatika Server for TWS - with link to accept
                               // backlog from HTTP
    {"rtdas.spatika.net", "/tws_gprs/update_twsrf_data_v2", "80",
     "rfclimate5p13",
     "x-www-form-urlencoded"}, // Spatika Server for TWS-RF -with link to accept
                               // backlog from HTTP
    {"rtdas.spatika.net", "/tws_gprs/twsrf_gen", "80", "wsgen2014",
     "x-www-form-urlencoded"}, // Spatika Server for TWS-RF -with link to accept
                               // backlog from HTTP AG1
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
