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
// v5.78 BUILD SIGNATURE: 0xDEADBEEF (Hardening Revision A)
#include <driver/adc.h>
#include <rom/rtc.h>
#include <esp_flash.h>


int wired = 1; // Integrated is 1 or Standalone LCD module is 0
const int chipSelect = SS;
bool ws_ok = true;
volatile bool wd_ok = true; // WD sensor wired on all KSNDMC_TWS field units

TaskHandle_t scheduler_h;
TaskHandle_t gprs_h; // http,sms,ftp,dota
volatile bool primary_data_delivered = false;
RTC_DATA_ATTR volatile bool skip_primary_http = false; // v5.50: Survive warm resets (volatile Fix)
volatile bool force_ftp = false;
volatile bool sleep_sequence_active = false; // v5.77: Sleep Gate signal
volatile bool force_ftp_daily = false;
RTC_DATA_ATTR int rtc_daily_sync_count = 0; // v5.77: Daily sync retry cap
char ftp_daily_date[12] = "";
volatile bool force_reboot = false;
volatile bool force_ota = false;
volatile bool force_gps_refresh =
    false; // v7.59: Server-requested GPS re-acquire
volatile bool force_clear_ftp_queue =
    false; // v7.59: Server-requested FTP backlog clear
volatile bool force_delete_data = false; // v7.94: Server-requested Factory Reset
volatile bool ota_writing_active = false;
volatile bool ota_silent_mode = false; // Rule 43
volatile bool bearer_recovery_active = false;
                                       // v6.88
char ota_cmd_param[128] = "";
RTC_DATA_ATTR int last_cmd_id = 0;
RTC_DATA_ATTR char last_cmd_res[64] = "N/A";
int ota_fail_count = 0;
char ota_fail_reason[48] = "NONE";
volatile bool health_in_progress = false;
TaskHandle_t lcdkeypad_h; // UI
TaskHandle_t tempHum_h;
volatile uint32_t last_activity_time = 0; // v5.85: Safety Heartbeat Timer

// v5.66: Core System State (Moved from globals.h)
volatile int sync_mode = eSyncModeInitial;
volatile int gprs_mode = eGprsInitial;
volatile int data_mode = eCurrentData;
volatile int sampleNo = 0;
int rf_cls_dd = 0, rf_cls_mm = 0, rf_cls_yy = 0;
int rf_cls_ram_dd = 0, rf_cls_ram_mm = 0, rf_cls_ram_yy = 0;
int time_to_sleep = 0;

// v5.66: Global String Buffers (Moved from globals.h)
char cur_file[32] = "";
char unsent_file[32] = "";
char new_file[32] = "";
char temp_file[50] = "";
char station_name[16] = "";
char chip_id[13] = "";
char calib_state[5] = "";
char calib_text[40] = "";
char calib_content[16] = "";
char ftpunsent_file[50] = "";
char ftpdaily_file[50] = "";

// v5.66: GPRS Counters (Moved from globals.h)
int http_no = 0, msg_sent = 0;
int rssiIndex = 0, rssiEndIndex = 0, registration = 0;
volatile int retries = 0;               // cross-task: written by GPRS, read by scheduler
volatile int unsent_count = 0, success_count = 0; // cross-task shared counters
volatile int s = 0, fileSize = 0;       // cross-task file-size helpers
int delay_val = 10000; // Default delay before starting GPRS(Moved from globals.h)

// v5.66: System & Power Variables (Moved from globals.h)
char UNIT_VER[20] = ""; 
char NETWORK[15] = ""; 
char STATION_TYPE[10] = "";
char universalNumber[20] = "";
char battery[10] = "0.0";
char solar_sense[10] = "0.0";
float solar_val = 0.0, solar = 0.0;
float li_bat = 0.0, li_bat_val = 0.0;
char httpPostRequest[256] = "";
char httpContent[12] = "";
char append_text[160] = "";
char store_text[160] = "";
char ftpappend_text[160] = "";
char ftp_station[16] = "";
SensorData latestSensorData; // Global sensor data anchor

// v5.66: Remaining Hoisted Definitions (System State Restoration)
char *startptr = NULL;
int send_status = 0;
bool valid_dt = false, valid_time = false;
int ftp_login_flag = 0;
size_t len = 0;
char last_logged[16] = "";
char http_data[350] = ""; 
char sample_cum_rf[10], sample_inst_rf[10], sample_temp[10], sample_hum[10],
    sample_avgWS[10], sample_WD[10], sample_bat[10], ftpsample_avgWS[10],
    ftpsample_cum_rf[10];
char ht_data[80] = ""; 
 // v5.75: apn_str and carrier relocated to block below with 32-char expansion
 
 char reg_status[16] = "";
char modem_response_buf[2048] = {0}; // v6.0 Fixed Modem Response Buffer
char gprs_payload[1280] = {0};       // v6.0 Transmission Payload Buffer
int gprs_payload_len = 0;
char *reg_status_ptr = NULL;
char reg_val[3] = "";
char rssi_resp[10] = "";
uint8_t rssi_val = 0;
char date_now[11] = "";
char time_now[6] = "";
int ULP_WAKEUP_TC = 2000;
float rf_value = 0, current_rf_value = 0, calib_rf_float = 0;
int rf_res_edit_state = 0;
char cum_rf[10], inst_rf[10], inst_temp[10], avg_cum_rf[10];
char ftpcum_rf[10];
float avg_cumRF = 0, new_current_cumRF = 0, new_current_instRF = 0;
RTC_DATA_ATTR volatile float temperature = 0, humidity = 0, pressure = 0;
volatile float windSpCount = 0, cur_wind_speed = 0;
RTC_DATA_ATTR float cur_avg_wind_speed = 0.0;
RTC_DATA_ATTR volatile int windDir = 0;               // written by windDirection task, read by scheduler
float sea_level_pressure = 0;
char inst_hum[10], avg_wind_speed[10], inst_wd[10];
int pcb_clear_state = 0;
char signature[20] = "";
int temp_count_rf = 0, calib_count_rf = 0, calib_flag = 0;
char rf_str[10], calib_rf[10], temp_str[16], hum_str[16], windSpeedInst_str[16],
    prevWindSpeedAvg_str[7], windDir_str[16];

// v5.66: Final ODR Hoist Block
char UNIT[15] = UNIT_CFG; 
float station_altitude_m = 0.0;
int record_length = 0;
volatile bool gprs_pdp_ready = false;
float temp_crf, temp_instrf, temp_bat, temp_temp, temp_hum, temp_avg_ws;
int temp_dir = 0;
int temp_sampleNo = 0, temp_day = 0, temp_month = 0, temp_year = 0, temp_hr = 0, temp_min = 0, temp_sig = 0;
int unsent_pointer_count = 0;
char present_topRow[17] = "";
char present_bottomRow[17] = "";

// Definitions of global objects (v5.65 Extern Pass)
HardwareSerial SerialSIT(2);
LiquidCrystal_I2C lcd(0x27, 16, 2);
SemaphoreHandle_t i2cMutex = NULL;
SemaphoreHandle_t serialMutex = NULL;
SemaphoreHandle_t modemMutex = NULL;
SemaphoreHandle_t fsMutex = NULL;
RTC_DS1307 rtc;
portMUX_TYPE timerMux0 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timerMux1 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timerMux2 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE windMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE rtcTimeMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE syncMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE sensorDataMux = portMUX_INITIALIZER_UNLOCKED;

volatile char show_now = 0;
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

// --- Global Variable Definitions (v5.65 ODR Fix) ---
bool webServerStarted = false;
volatile bool wifi_active = false;
unsigned long last_wifi_activity_time = 0;
volatile bool timeSyncRequired = true;
volatile bool httpInitiated = false;
volatile bool schedulerBusy = false;
// --- End Global Definitions ---

// --- RTC Persistent Definitions (v5.65 ODR Fix) ---
RTC_DATA_ATTR double lati = 0.0, longi = 0.0;
RTC_DATA_ATTR double gps_latitude = 0.0, gps_longitude = 0.0;
RTC_DATA_ATTR int gps_fix_dd = 0, gps_fix_mm = 0, gps_fix_yy = 0;
RTC_DATA_ATTR char apn_str[32] = ""; // v5.75: Expanded from 20 to 32 (M-03 fix)
RTC_DATA_ATTR char carrier[32] = ""; // v5.75: Expanded from 20 to 32 (M-03 fix)
RTC_DATA_ATTR char sim_number[20] = "NA";
RTC_DATA_ATTR char cached_iccid[25] = "";
RTC_DATA_ATTR bool isLTE = false;
RTC_DATA_ATTR float prev_15min_temp = INITIAL_PREV_TEMP;
RTC_DATA_ATTR float prev_15min_hum = INITIAL_PREV_HUM;
RTC_DATA_ATTR float prev_15min_ws = 0.0;
RTC_DATA_ATTR int temp_same_count = 0;
RTC_DATA_ATTR int hum_same_count = 0;
RTC_DATA_ATTR int diag_reg_time_total = 0;
RTC_DATA_ATTR int diag_backlog_total = 0;
RTC_DATA_ATTR int diag_backlog_total_prev = 0;
RTC_DATA_ATTR int diag_reg_count = 0;
RTC_DATA_ATTR int diag_reg_worst = 0;
RTC_DATA_ATTR int diag_gprs_fails = 0;
RTC_DATA_ATTR int diag_modem_mutex_fails = 0;
RTC_DATA_ATTR int diag_last_reset_reason = 0;
RTC_DATA_ATTR bool diag_rtc_battery_ok = true;
RTC_DATA_ATTR int diag_consecutive_reg_fails = 0;
RTC_DATA_ATTR int diag_stored_apn_fails = 0;
RTC_DATA_ATTR int diag_consecutive_sim_fails = 0;

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
RTC_DATA_ATTR bool diag_wd_fail = false;
RTC_DATA_ATTR int diag_consecutive_http_fails = 0;
RTC_DATA_ATTR int last_ftp_unsent_sampleNo = -1; // v5.77: Persistent FTP Dedup
RTC_DATA_ATTR int diag_daily_http_fails = 0;
RTC_DATA_ATTR int diag_http_present_fails = 0;
RTC_DATA_ATTR int diag_http_cum_fails = 0;
RTC_DATA_ATTR int diag_cum_fail_reset_month = -1;
RTC_DATA_ATTR int last_http_ok_slot = -1; // v5.72: Track slot of last successful HTTP
RTC_DATA_ATTR int last_unsent_sampleNo = -1; // v5.72: Dedup guard for unsent.txt
RTC_DATA_ATTR int diag_rejected_count = 0;
RTC_DATA_ATTR bool diag_sensor_fault_sent_today = false;
RTC_DATA_ATTR char diag_crash_task[16] = "NONE";

RTC_DATA_ATTR uint32_t total_wind_pulses_32 = 0;
RTC_DATA_ATTR uint32_t last_sched_wind_pulses_32 = 0;
RTC_DATA_ATTR uint16_t last_raw_wind_count = 0;
RTC_DATA_ATTR uint32_t total_rf_pulses_32 = 0;
RTC_DATA_ATTR uint32_t last_sched_rf_pulses_32 = 0;
RTC_DATA_ATTR uint16_t last_raw_rf_count = 0;

RTC_DATA_ATTR int diag_ndm_count = 0;
RTC_DATA_ATTR int diag_ndm_count_prev = 0;
RTC_DATA_ATTR char diag_cdm_status[20] = "PENDING";
RTC_DATA_ATTR int diag_pd_count = 0;
RTC_DATA_ATTR int diag_pd_count_prev = 0;
RTC_DATA_ATTR int diag_first_http_count = 0;
RTC_DATA_ATTR int diag_first_http_count_prev = 0;
RTC_DATA_ATTR int diag_net_data_count = 0;
RTC_DATA_ATTR int diag_net_data_count_prev = 0;
RTC_DATA_ATTR int diag_last_rollover_day = -1;

RTC_DATA_ATTR uint32_t diag_http_time_total = 0;
RTC_DATA_ATTR uint32_t diag_ftp_time_total = 0;
RTC_DATA_ATTR uint32_t diag_sent_mask_cur[3] = {0, 0, 0};
RTC_DATA_ATTR uint32_t diag_sent_mask_prev[3] = {0, 0, 0};
RTC_DATA_ATTR int diag_http_success_count = 0;
RTC_DATA_ATTR int diag_http_success_count_prev = 0;
RTC_DATA_ATTR int diag_http_retry_count = 0;
RTC_DATA_ATTR int diag_http_retry_count_prev = 0;
RTC_DATA_ATTR int diag_ftp_success_count = 0;
RTC_DATA_ATTR int diag_ftp_success_count_prev = 0;
RTC_DATA_ATTR char diag_reg_fail_type[16] = "NONE";
RTC_DATA_ATTR char diag_http_fail_reason[16] = "NONE";

RTC_DATA_ATTR char cached_server_ip[32] = "";
RTC_DATA_ATTR char cached_server_domain[64] = "";
RTC_DATA_ATTR bool dns_fallback_active = false;
RTC_DATA_ATTR int preferred_ftp_mode = -1;
RTC_DATA_ATTR bool healer_reboot_in_progress = false;
RTC_DATA_ATTR char diag_crash_task_rtc[16] = ""; // v5.74: Task name that triggered last stack overflow
RTC_DATA_ATTR int diag_crash_count = 0;           // v5.74: Consecutive crashes of the same task

RTC_DATA_ATTR int health_last_sent_hour = -1;
RTC_DATA_ATTR int health_last_sent_day = -1;
RTC_DATA_ATTR bool diag_fw_just_updated = false;
RTC_DATA_ATTR bool rtc_daily_sync_done = false;

RTC_DATA_ATTR int current_year = 0, current_month = 0, current_day = 0, current_hour = 0,
    current_min = 0, current_sec = 0, previous_min = -1, record_hr = -1, record_min = -1,
    previous_rfclose_year = 0, previous_rfclose_month = 0, previous_rfclose_day = 0,
    calib_year = 0, calib_month = 0, calib_day = 0, calib_sts = 0;
RTC_DATA_ATTR int firmwareUpdated = 0;
RTC_DATA_ATTR int last_processed_sample_idx = -1;
RTC_DATA_ATTR bool fresh_boot_check_done = false;
RTC_DATA_ATTR bool apn_saved_this_sim = false;
RTC_DATA_ATTR float last_valid_temp = 0.0;
RTC_DATA_ATTR float last_valid_hum = 0.0;
RTC_DATA_ATTR int last_valid_wd = 0;
RTC_DATA_ATTR float rtc_daily_cum_rf = 0.0;

RTC_DATA_ATTR int last_recorded_dd = 0;
RTC_DATA_ATTR int last_recorded_mm = 0;
RTC_DATA_ATTR int last_recorded_yy = 0;
RTC_DATA_ATTR int last_recorded_hr = 0;
RTC_DATA_ATTR int last_recorded_min = 0;

RTC_DATA_ATTR bool signature_valid = false;
char hw_tag = ' '; // v5.75: Passive hardware/VFS health tag
RTC_DATA_ATTR bool pending_manual_status = false;
RTC_DATA_ATTR bool pending_manual_gps = false;
RTC_DATA_ATTR bool pending_manual_health = false;
RTC_DATA_ATTR int last_successful_cnmp = 2; // v5.84: 2=Auto, 13=GSM, 38=LTE
RTC_DATA_ATTR bool last_http_ok = false;    // v5.84: Skip IP check if last slot worked
RTC_DATA_ATTR int gprs_2g_slots_count = 0; // v5.84: Self-recovering 'Peeking' for LTE
RTC_DATA_ATTR int low_bat_skip_count = 0;   // v5.85: P6 - counts skipped slots
RTC_DATA_ATTR bool low_bat_mode_active = false; // v5.85: Tracking flag
// --- End RTC Definitions ---

// --- UI & Server Configurations (v5.65 ODR Fix) ---
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
    {15, "SEND LAT/LONG", "YES ?", eDisplayOnly},    // 9
    {28, "SEND HEALTH", "YES ?", eDisplayOnly},      // 10
    {7, "RF CALIBRATION", "Yes?", eLive},            // 11
    {25, "RF RESOLUTION", "0.50", eNumeric},         // 12
    {11, "DELETE DATA?", "YES?", eNumeric},          // 13
    {12, "COPY TO SDCARD?", "YES?", eDisplayOnly},   // 14
    {13, "BATTERY VOLTAGE", "0", eLive},             // 15
    {14, "SOLAR VOLTAGE", "0", eLive},               // 16
    {23, "ENABLE WIFI", "PRESS SET", eDisplayOnly},  // 17
    {21, "LOG (DT TM)", "20260205 08:30", eNumeric}, // 18
    {16, "WIND DIRECTION", "NA", eLive},             // 19
    {17, "INST WIND SPEED", "NA", eLive},            // 20
    {18, "AVG WIND SPEED", "NA", eLive},             // 21
    {19, "TEMPERATURE", "NA", eLive},                // 22
    {20, "HUMIDITY", "NA", eLive},                   // 23
    {24, "PRESSURE", "NA", eLive},                   // 24
    {26, "STATION ALT", "900", eNumeric},            // 25 BME only
    {27, "HTTP FAIL STATS", "                ",
     eLive},                                         // 26 v7.70: HTTP fail counters
    {22, "TURN OFF LCD", "YES?", eDisplayOnly}       // 27
};

#if SYSTEM == 0
struct http_params httpSet[7] = {
    {"rtdas.ksndmc.net", "117.216.42.181", "/trg_gprs/update_data_sit_v2", "80", SEC_KS_TRG_OLD, "x-www-form-urlencoded"}, 
    {"rtdas.ksndmc.net", "117.216.42.181", "/trg_gprs/update_data_sit_v3", "80", SEC_KS_TRG_NEW, "x-www-form-urlencoded"}, 
    {"rtdasbmsk.spatika.net", "164.100.130.199", "/Home/UpdateTRGData", "8085", SEC_BIH_GOV, "json"}, 
    {"rtdasbih.spatika.net", "185.250.105.225", "/trg_gprs/upload_bih_trg_data_new", "80", SEC_BIH_SPT, "json"}, 
    {"104.211.5.142", "104.211.5.142", "/esprain", "3002", SEC_EX_SIT, "json"},
    {"104.211.5.142", "104.211.5.142", "/dmc_trg_data", "3003", SEC_EX_SIT, "x-www-form-urlencoded"},
    {"rtdas1.spatika.net", "89.32.144.163", "/hmrtdas/trg_gen", "80", SEC_SPT_GEN, "x-www-form-urlencoded"}, 
};
#endif

#if (SYSTEM == 1) || (SYSTEM == 2)
struct http_params httpSet[11] = {
    {"rtdas.ksndmc.net", "117.216.42.181", "/trg_gprs/update_data_sit_v2", "80", SEC_KS_TRG_OLD, "x-www-form-urlencoded"}, 
    {"rtdas.ksndmc.net", "117.216.42.181", "/trg_gprs/update_data_sit_v3", "80", SEC_KS_TRG_NEW, "x-www-form-urlencoded"}, 
    {"rtdasbmsk.spatika.net", "164.100.130.199", "/Home/UpdateTRGData", "8085", SEC_BIH_GOV, "json"}, 
    {"rtdasbih.spatika.net", "185.250.105.225", "/trg_gprs/upload_bih_trg_data_new", "80", SEC_BIH_SPT, "json"}, 
    {"104.211.5.142", "104.211.5.142", "/esprain", "3002", SEC_EX_SIT, "json"},
    {"104.211.5.142", "104.211.5.142", "/dmc_trg_data", "3003", SEC_EX_SIT, "x-www-form-urlencoded"},
    {"rtdas.ksndmc.net", "117.216.42.181", "/tws_gprs/update_tws_data_v2", "80", SEC_KS_TWS, "x-www-form-urlencoded"}, 
    {"rtdas.ksndmc.net", "117.216.42.181", "/tws_gprs/update_twsrf_data_v2", "80", SEC_KS_ADDON, "x-www-form-urlencoded"}, 
    {"rtdas.spatika.net", "144.91.104.105", "/tws_gprs/update_tws_data_v2", "80", SEC_KS_TWS, "x-www-form-urlencoded"}, 
    {"rtdas.spatika.net", "144.91.104.105", "/tws_gprs/update_twsrf_data_v2", "80", SEC_KS_ADDON, "x-www-form-urlencoded"}, 
    {"rtdas.spatika.net", "89.32.144.163", "/tws_gprs/twsrf_gen", "80", SEC_SPT_TWS_RF, "x-www-form-urlencoded"}, 
};
#endif
// --- End Configuration Tables ---

// --- Volatile Handlers (v5.65 ODR Fix) ---
volatile bool rtcReady = false;
volatile bool rtcTimeChanged = false;
volatile int wakeup_reason_is = 0;
volatile int lcdkeypad_start = 0;
// --- End Volatile Definitions ---

// --- System Configuration & Counters (v5.65 ODR Fix) ---
int test_health_every_slot = TEST_HEALTH_DEFAULT;
float RF_RESOLUTION = DEFAULT_RF_RESOLUTION;
unsigned long last_key_time = 0;
int cur_file_found = 0;
volatile int data_writing_initiated = 0;
int time_to_deepsleep = 13;
float bat_val = 0.0;
int solar_conn = 0;
int calib_header_drawn = 0;
int unsent_counter = 0;
int http_code = -1;
volatile int signal_strength = SIGNAL_STRENGTH_NO_DATA; // cross-task: GPRS writes, scheduler reads
volatile int signal_lvl = SIGNAL_STRENGTH_NO_DATA;      // cross-task: GPRS writes, scheduler reads
int sd_card_ok = 0;
int send_daily = 0;
int cur_mode = 0;
int cur_fld_no = 0;
int last_lcd_state = 0; // v5.70 Final: Global UI state tracker
volatile bool gprs_started = false;
int badReads = 0;
int prev_wind_count = 0;
char pres_str[20] = "NA";
// --- End System Definitions ---

void setup() {
  // v5.77 Hardened [UI-WAKE]: Instant LCD Activation
  // If we woke up by EXT0 (User Button), flip the 5V rail ON immediately
  // before the 5s safety delay to prevent the 'Double-Press' requirement.
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
      pinMode(32, OUTPUT);
      digitalWrite(32, HIGH);
      gpio_hold_dis(GPIO_NUM_32);
      wakeup_reason_is = ext0; // v5.77: Pre-anchor reason for task inheritance
  }
  
  // v5.85 P10 CRITICAL: Clear power-saving holds from Deep Sleep
  gpio_hold_dis(GPIO_NUM_26);
  gpio_hold_dis(GPIO_NUM_32);
  gpio_hold_dis(GPIO_NUM_16); 
  gpio_hold_dis(GPIO_NUM_17);
  rtc_gpio_deinit(GPIO_NUM_27); // Re-map keypad to digital IO
  pinMode(27, INPUT_PULLUP);    // v5.85: Force pull-up early for set_wakeup_reason()

  // v5.85: P8 - Serial production guard (saved ~1.5mA)
#if DEBUG == 1
  Serial.begin(115200);
#endif
  // v5.75: Force-Enable Logs (Clears stale RTC RAM silence flags)
  ota_silent_mode = false;
  bearer_recovery_active = false;

  delay(1000);
  debugln("\n\n[BOOT] HELLO! System starting... (Debug Enabled)");

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

  modemMutex = xSemaphoreCreateMutex();
  if (modemMutex != NULL)
    xSemaphoreGive(modemMutex);

  initialize_hw(); // sets CPU to 80MHz internally

  // Record Reset Reason and Increment Uptime
  RESET_REASON rr = rtc_get_reset_reason(0);
  diag_last_reset_reason = (int)rr;
  
  // v7.93: Only wipe counters on HARD Power-On (1). 
  // Preserve on DEEPSLEEP (5) for better testing resilience.
  // v5.50: Also treat EXT_CPU_RESET (14) and SW_CPU_RESET (12) as fresh starts.
  // Reason 12 is triggered by 'DELETE DATA' or manually via ESP.restart().
  // v5.55 SELF-HEALING: Protect metrics during maintenance reboots.
  // If healer_reboot_in_progress is set, skip wiping counters even on Reason 12.
  if ((rr == POWERON_RESET || rr == EXT_CPU_RESET) && !healer_reboot_in_progress) {
    debugf("[BOOT] Fresh Start (Reason %d). Initializing RTC variables.\n", (int)rr);
    // v5.60: Time-of-day dependent sensor anchor initialization
    int boot_hr = 10; // Default to mid-morning if RTC fails
    if (diag_rtc_battery_ok) {
      DateTime now = rtc.now();
      boot_hr = now.hour();
    }

    if (boot_hr >= 0 && boot_hr < 6) { // Night
      last_valid_temp = 18.0; last_valid_hum = 90.0;
    } else if (boot_hr >= 6 && boot_hr < 12) { // Morning
      last_valid_temp = 21.0; last_valid_hum = 78.0;
    } else if (boot_hr >= 12 && boot_hr < 18) { // Afternoon
      last_valid_temp = 24.5; last_valid_hum = 69.0;
    } else { // Evening
      last_valid_temp = 20.0; last_valid_hum = 86.0;
    }
    
    if (last_valid_wd < 0 || last_valid_wd >= 360)
      last_valid_wd = 0;
    rtc_daily_cum_rf = 0.0;

    total_wind_pulses_32 = 0;
    last_sched_wind_pulses_32 = 0;
    total_rf_pulses_32 = 0;
    last_sched_rf_pulses_32 = 0;

    // v5.50 Bug#10 Fix: Reset diagnostic counters on DELETE DATA (SW_CPU_RESET)
    // so they don't carry stale values into fresh logs (explains netcount=42 issue)
    if (false) { // v6.03: Reason 12 (SW Restart) no longer destructive to preserve continuity
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
      health_last_sent_day = -1;      // v5.52 LOOP-6 FIX: Reset health persistence
      health_last_sent_hour = -1;

      // v7.94: Clear physical ULP counters too
      rf_count.val = 0;
      wind_count.val = 0;
      calib_count.val = 0;
      total_wind_pulses_32 = 0;
      last_raw_wind_count = 0;  // P3 fix v5.65: Was a duplicate total_wind_pulses_32=0 — missing reset
      total_rf_pulses_32 = 0;
      rtc_daily_cum_rf = 0.0;
      last_valid_wd = 0;

      // v5.60: Time-aware reset for DELETE DATA
      int reset_hr = 10;
      if (diag_rtc_battery_ok) { DateTime now = rtc.now(); reset_hr = now.hour(); }
      if (reset_hr >= 0 && reset_hr < 6) { last_valid_temp = 18.0; last_valid_hum = 90.0; }
      else if (reset_hr >= 6 && reset_hr < 12) { last_valid_temp = 21.0; last_valid_hum = 78.0; }
      else if (reset_hr >= 12 && reset_hr < 18) { last_valid_temp = 24.5; last_valid_hum = 69.0; }
      else { last_valid_temp = 20.0; last_valid_hum = 86.0; }

      debugln("[BOOT] DELETE DATA detected. All diagnostic and sensor counters cleared.");
    }
  } else if (healer_reboot_in_progress) {
    debugln("[BOOT] Maintenance Reboot Recovery. Preserving all Diagnostic & Sensor counters.");
    // healer_reboot_in_progress is reset at the very end of setup
  } else {
    debugf("[BOOT] Preserving counters for reset reason: %d\n", (int)rr);
  }

  // Pre-load essential system data: Try NVS (Preferences) first for robustness
  Preferences prefs;
  prefs.begin("sys-config", false); // Read-Write mode
  char nvs_station[16] = {0};
  size_t nvs_len = prefs.getString("station", nvs_station, sizeof(nvs_station)-1);

  if (nvs_len > 0) {
    trim_whitespace(nvs_station); // v5.63: Strip LCD UI padding spaces
    strncpy(station_name, nvs_station, sizeof(station_name) - 1);
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
        char temp_buf[32];
        int r = fileTemp.readBytesUntil('\n', temp_buf, sizeof(temp_buf)-1);
        temp_buf[r] = '\0';
        trim_whitespace(temp_buf); // v5.63
        strncpy(station_name, temp_buf, sizeof(station_name) - 1);
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
         char temp_buf[32];
         int r = fileTemp.readBytesUntil('\n', temp_buf, sizeof(temp_buf)-1);
         temp_buf[r] = '\0';
         trim_whitespace(temp_buf);
         strncpy(station_name, temp_buf, sizeof(station_name) - 1);
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
  // v5.82 Platinum: Post-ID Recovery for atomic State-Anchor temp files
  if (strlen(station_name) > 0) {
      char tmpRec[64], txtRec[64];
      snprintf(tmpRec, sizeof(tmpRec), "/lastrecorded_%s.tmp", station_name);
      snprintf(txtRec, sizeof(txtRec), "/lastrecorded_%s.txt", station_name);
      if (!SPIFFS.exists(txtRec) && SPIFFS.exists(tmpRec)) {
          SPIFFS.rename(tmpRec, txtRec);
          debugln("[FS] Recovered stranded lastrecorded.tmp");
      }
  }
  if (!SPIFFS.exists("/prevWindSpeed.txt") && SPIFFS.exists("/prevWS.tmp")) {
      SPIFFS.rename("/prevWS.tmp", "/prevWindSpeed.txt");
      debugln("[FS] Recovered stranded prevWS.tmp");
  }

  loadGPS(); // v5.51: Ensure coordinates are available even after POR

  // --- OTA Failure Tracking (Change namespace) ---
  prefs.end(); // Close sys-config
  prefs.begin("ota-track", false);
  ota_fail_count = prefs.getInt("fail_cnt", 0);
  char last_reason[64] = {0};
  prefs.getString("fail_res", last_reason, sizeof(last_reason)-1);
  strncpy(ota_fail_reason, last_reason, sizeof(ota_fail_reason) - 1);
  ota_fail_reason[sizeof(ota_fail_reason) - 1] = '\0';

  char last_ver[32] = {0};
  prefs.getString("last_ver", last_ver, sizeof(last_ver)-1);
  if (last_ver[0] == '\0') strcpy(last_ver, FIRMWARE_VERSION);

  if (strcmp(last_ver, FIRMWARE_VERSION) != 0) {
    debugln("[BOOT] Version Change! Clearing OTA Fail counters.");
    ota_fail_count = 0;
    strcpy(ota_fail_reason, "NONE");
    prefs.putInt("fail_cnt", 0);
    prefs.putString("fail_res", "NONE");
    prefs.putString("last_ver", FIRMWARE_VERSION);

    // v5.60: Time-aware reset for Version Change
    int ver_hr = 10;
    if (diag_rtc_battery_ok) { DateTime now = rtc.now(); ver_hr = now.hour(); }
    if (ver_hr >= 0 && ver_hr < 6) { last_valid_temp = 18.0; last_valid_hum = 90.0; }
    else if (ver_hr >= 6 && ver_hr < 12) { last_valid_temp = 21.0; last_valid_hum = 78.0; }
    else if (ver_hr >= 12 && ver_hr < 18) { last_valid_temp = 24.5; last_valid_hum = 69.0; }
    else { last_valid_temp = 20.0; last_valid_hum = 86.0; }

    last_valid_wd = 0;
    rtc_daily_cum_rf = 0.0;
  }
  prefs.end();

  // If still empty (neither NVS nor SPIFFS had valid data), check SD Card
  if (strlen(station_name) == 0 && sd_card_ok) {
    if (SD.exists("/station.doc")) {
      File fileTemp = SD.open("/station.doc", FILE_READ);
      if (fileTemp) {
        char temp_buf[32];
        int r = fileTemp.readBytesUntil('\n', temp_buf, sizeof(temp_buf)-1);
        temp_buf[r] = '\0';
        trim_whitespace(temp_buf);
        if (strlen(temp_buf) > 0) {
          strncpy(station_name, temp_buf, sizeof(station_name) - 1);
          station_name[sizeof(station_name) - 1] = '\0';

          if (strlen(station_name) == 6 && isDigitStr(station_name) &&
              strncmp(station_name, "00", 2) == 0) {
            char t[16];
            strcpy(t, station_name + 2);
            strcpy(station_name, t);
          }
          strcpy(ftp_station, station_name);
          debugf("[BOOT] Loaded Station ID from SD: %s\n", station_name);

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

  // TIER 3: NVS Station Name Garbage Protection
  bool name_valid = (strlen(station_name) >= 2 && strlen(station_name) <= 14);
  for (int i = 0; name_valid && i < strlen(station_name); i++) {
      if (!isalnum(station_name[i]) && station_name[i] != '_') name_valid = false;
  }
  if (!name_valid && strlen(station_name) > 0) {
      debugln("[BOOT] CRITICAL: Invalid station name loaded from NVS/File. Halting corruption.");
      strcpy(station_name, ""); // Force reset below
  }

  // v5.77 Hardened [C-04/N-12]: Universal Station ID Padding
  // Ensure the station_name used for all subsequent logic is strictly 6-digits.
  char raw_unpadded[16] = {0};
  strncpy(raw_unpadded, station_name, 15);
  
  if (strlen(station_name) == 4 && isDigitStr(station_name)) {
      char tmp_pad[16];
      snprintf(tmp_pad, sizeof(tmp_pad), "00%s", station_name);
      strncpy(station_name, tmp_pad, sizeof(station_name) - 1);
      station_name[sizeof(station_name) - 1] = '\0';
      strncpy(ftp_station, station_name, sizeof(ftp_station) - 1);
      ftp_station[sizeof(ftp_station) - 1] = '\0';
      debugf("[BOOT] Canonical ID Enforced: %s\n", station_name);
  }

  // --- Proactive Station Data Migration (REVERSED for 6-Digit Hardening) ---
  // If we have legacy 4-digit files, migrate them to the 6-digit padded format.
  if (strlen(raw_unpadded) == 4 && isDigitStr(raw_unpadded)) {
    char old_prefix_lastrec[32], new_prefix_lastrec[32];
    snprintf(old_prefix_lastrec, sizeof(old_prefix_lastrec), "/lastrecorded_%s.txt", raw_unpadded);
    snprintf(new_prefix_lastrec, sizeof(new_prefix_lastrec), "/lastrecorded_00%s.txt", raw_unpadded);
    
    if (SPIFFS.exists(old_prefix_lastrec) && !SPIFFS.exists(new_prefix_lastrec)) {
      debugln("[BOOT] Migrating legacy lastrecorded to padded format...");
      SPIFFS.rename(old_prefix_lastrec, new_prefix_lastrec);
    }
    
    char old_prefix_closing[32], new_prefix_closing[32];
    snprintf(old_prefix_closing, sizeof(old_prefix_closing), "/closingdata_%s.txt", raw_unpadded);
    snprintf(new_prefix_closing, sizeof(new_prefix_closing), "/closingdata_00%s.txt", raw_unpadded);
    
    if (SPIFFS.exists(old_prefix_closing) && !SPIFFS.exists(new_prefix_closing)) {
      debugln("[BOOT] Migrating legacy closingdata to padded format...");
      SPIFFS.rename(old_prefix_closing, new_prefix_closing);
    }
  }

  if (SPIFFS.exists("/signature.txt")) {
    File fileTemp1 = SPIFFS.open("/signature.txt", FILE_READ);
    if (fileTemp1) {
      char sig_buf[48];
      int r = fileTemp1.readBytesUntil('\n', sig_buf, sizeof(sig_buf)-1);
      sig_buf[r] = '\0';
      strcpy(signature, sig_buf);
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
      char alt_buf[16];
      int r = altF.readBytes(alt_buf, sizeof(alt_buf)-1);
      alt_buf[r] = '\0';
      float loaded_alt = atof(alt_buf);
      void loadGPS();
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
char last_fw_ver[20];


  // 1. Initial Load from Persistent Files
  if (SPIFFS.exists("/rf_fw.txt")) {
    File f = SPIFFS.open("/rf_fw.txt", FILE_READ);
    if (f) { // EX2 FIX: Null handle returns "", toFloat()=0 [INFO] false data wipe!
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
    if (f) {
      char res_buf[16];
      int r = f.readBytes(res_buf, sizeof(res_buf)-1);
      res_buf[r] = '\0';
      user_res = atof(res_buf);
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
    http_no = 6; // 6 (KSNDMC TWS)
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
    http_no = 7; // 7 (KSNDMC ADDON)
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
    http_no = 6; // 6 (SPATIKA GEN TRG - index 6 confirmed)
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
    http_no = 8; // 8 (SPATIKA TWS)
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
    http_no = 10; // 10 (SPATIKA GEN TWS-RF)
  } else {
    debugln("!!! FATAL: ********** NO UNIT/SYSTEM MATCH FOUND **********");
    debugln("!!! Check UNIT and SYSTEM defines in globals.h!");
    // v5.65: Set invalid sentinel so prepare_data_and_send() can guard against it.
    // Previously http_no stayed at 0 (default int value), silently routing ALL
    // data to the first httpSet entry (KSNDMC endpoint) for any mis-typed UNIT string.
    http_no = -1;
    strncpy(UNIT_VER, "UNCONFIGURED", sizeof(UNIT_VER) - 1);
  }

  // v5.75 Hardened: [C-03] Server Index Protection
#if SYSTEM == 0
  if (http_no >= 7) { 
    http_no = -1; 
    debugln("!!! FATAL: http_no OOB for SYSTEM 0 !!!"); 
  }
#else
  if (http_no >= 11) { 
    http_no = -1; 
    debugln("!!! FATAL: http_no OOB for SYSTEM 1/2 !!!"); 
  }
#endif

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
#endif

  // v5.79 STRUCTURAL FIX: The SD OTA block was moved to Line 1700+ 
  // after hardware (SD/SPIFFS) initialization is actually verified.

  if (SPIFFS.exists("/firmware.doc")) {
    String temp;

    File verTemp = SPIFFS.open("/firmware.doc", FILE_READ);
    if (!verTemp) {
      debugln("Failed to open firmware.doc for reading");
    } else { // v7.67: Guard against null file dereference
      char temp_buf[64];
      int r = verTemp.readBytesUntil('\n', temp_buf, sizeof(temp_buf)-1);
      temp_buf[r] = '\0';
      trim_whitespace(temp_buf);
      strncpy(last_fw_ver, temp_buf, sizeof(last_fw_ver)-1);
      verTemp.close();
    }
    if (strcmp(UNIT_VER, last_fw_ver)) {
      debugf("[BOOT] Firmware in SPIFFS: %s\n", last_fw_ver);
      debugf("[BOOT] Current binary: %s\n", UNIT_VER);
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
      
      // v5.67: Differentiate between a simple OTA patch and a cross-architecture flash
      const char *p1 = strrchr(UNIT_VER, '-');
      const char *p2 = strrchr(last_fw_ver, '-');
      bool isCrossFlash = true;
      if (p1 && p2) {
          int len1 = p1 - UNIT_VER;
          int len2 = p2 - last_fw_ver;
          if (len1 == len2 && strncmp(UNIT_VER, last_fw_ver, len1) == 0) {
              isCrossFlash = false;
          }
      }

      if (isCrossFlash) {
          debugln("[OTA] CROSS-FLASH DETECTED (Different config type). Automating factory wipe...");
          SPIFFS.remove("/unsent.txt");
          SPIFFS.remove("/unsent_pointer.txt");
          SPIFFS.remove("/ftpunsent.txt");

          File root = SPIFFS.open("/"); 
          File file = root.openNextFile();
          while(file) {
              char n[64];
              strncpy(n, file.name(), sizeof(n)-1); n[sizeof(n)-1] = '\0';
              if (!(strcmp(n, "station.txt") == 0 || strcmp(n, "rf_fw.txt") == 0 || 
                    strcmp(n, "station.doc") == 0 || strcmp(n, "rf_res.txt") == 0 || 
                    strcmp(n, "firmware.doc") == 0)) {
                  debugf("[OTA] Removing incompatible structure: %s\n", n);
                  char full_path[128];
                  if (n[0] == '/') strncpy(full_path, n, sizeof(full_path)-1);
                  else snprintf(full_path, sizeof(full_path), "/%s", n);
                  full_path[sizeof(full_path)-1] = '\0';
                  SPIFFS.remove(full_path);
              }
              file.close(); file = root.openNextFile();
          }
          root.close();
          debugln("[OTA] Wipe complete.");
      } else {
          debugln("[OTA] Minor version update detected. Preserving backlog data intact.");
      }

      // RF
#if (SYSTEM == 0) || (SYSTEM == 2)
      rf_count.val = 0;
      rf_value = 0;
      total_rf_pulses_32 = 0;      // v5.78: Reset accumulator to prevent jump
      last_raw_rf_count = 0;       // v5.78: Reset anchor to 0
      last_sched_rf_pulses_32 = 0; // v5.78: Reset scheduler anchor
#endif

// TWS
#if (SYSTEM == 1) || (SYSTEM == 2)
      wind_count.val = 0; // Making the count as 0
      total_wind_pulses_32 = 0;      // v5.78: Reset accumulator to prevent jump
      last_raw_wind_count = 0;       // v5.78: Reset anchor to 0
      last_sched_wind_pulses_32 = 0; // v5.78: Reset scheduler anchor
#endif
    }
  } else {
    // Create firmware.doc if missing (Fresh install)
    debugln("firmware.doc not found, creating default...");
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
          "[OTA] [ERR] Found partial firmware.bin without ready flag. Deleting.");
      SPIFFS.remove("/firmware.bin");
    } else {
      File firmware = SPIFFS.open("/firmware.bin");
      if (firmware) {
        // v6.81: Verify Magic Byte (0xE9) before starting
        int firstByte = firmware.read();
        if (firstByte != 0xE9) {
          debugf1("[OTA] [ERR] CORRUPT! Magic byte is 0x%02X (Expected 0xE9). "
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
    // v5.57 Fix: Sync 32-bit accumulator anchors immediately after zeroing ULP
    // counters, otherwise the next cycle delta = (new_raw - old_anchor) which
    // can be a huge spurious number (treated as noise, but cleaner to prevent).
    last_raw_rf_count = 0;
    last_sched_rf_pulses_32 = total_rf_pulses_32;
    debugln("Cleanup complete. Starting fresh.");
  }
  // If the same task caused a stack overflow 3+ consecutive times (persisted in RTC RAM),
  // skip recreating it this boot to break the reboot loop.
  // The counter resets to 0 on a POWERON_RESET (fresh power cycle).
  bool skip_gprs_task      = (diag_crash_count >= 3 && strncmp(diag_crash_task_rtc, "gprsTask",      15) == 0);
  bool skip_scheduler_task = (diag_crash_count >= 3 && strncmp(diag_crash_task_rtc, "schedulerTask", 15) == 0);
  bool skip_rtc_task       = (diag_crash_count >= 3 && strncmp(diag_crash_task_rtc, "rtcReadTask",   15) == 0);
  bool skip_lcd_task       = (diag_crash_count >= 3 && strncmp(diag_crash_task_rtc, "lcdkeypadTask", 15) == 0);
  bool skip_temphum_task   = (diag_crash_count >= 3 && strncmp(diag_crash_task_rtc, "tempHumTask",   15) == 0);
  bool skip_ws_task        = (diag_crash_count >= 3 && strncmp(diag_crash_task_rtc, "windSpeedTask", 15) == 0);
  bool skip_wd_task        = (diag_crash_count >= 3 && strncmp(diag_crash_task_rtc, "windDirectio",  15) == 0); // truncated to 15 chars

  if (diag_crash_count >= 3) {
    debugf("[BOOT] [WARN] CRASH LOOP GUARD: Task '%s' crashed %d times. Skipping task.\n",
           diag_crash_task_rtc, diag_crash_count);
  }

  // v7.95: CRITICAL RACE FIX — Initialize ULP and anchors BEFORE creating tasks.
  // This prevents the scheduler from reading uninitialized ULP RAM or stale 
  // counters before setup() has finished zeroing/syncing them.
  uint16_t preserved_rf = (rr == POWERON_RESET) ? 0 : rf_count.val;
#if (SYSTEM == 1) || (SYSTEM == 2)
  uint32_t preserved_wind = (rr == POWERON_RESET) ? 0 : wind_count.val;
#endif

  debug("ULP Wakeup Period set to 1ms (High Resolution for Wind)");
  ULP_COUNTING(ULP_WAKEUP_TC);
  debugln("ULP Program loaded and started.");

  // Safely restore the physical counts back into the fresh ULP memory block
  rf_count.val = preserved_rf;
#if (SYSTEM == 1) || (SYSTEM == 2)
  wind_count.val = preserved_wind;
#endif

  // v7.94: FINAL HARDWARE ANCHOR
  // We MUST anchor the 'last_raw' counters to the CURRENT hardware state
  // AFTER any ULP code loads or version-based resets.
  // CRITICAL FIX: Only do this on FRESH BOOT, otherwise we clear all pulses
  // accumulated during Deep Sleep before the scheduler can read them!
  if ((rr == POWERON_RESET || rr == EXT_CPU_RESET || rr == SW_CPU_RESET) && !healer_reboot_in_progress) {
    last_raw_wind_count = wind_count.val;
    last_raw_rf_count = rf_count.val;
    last_sched_wind_pulses_32 = total_wind_pulses_32;
    last_sched_rf_pulses_32 = total_rf_pulses_32;
    debugf2("[BOOT] ULP Anchors Synced: Wind=%u, Rain=%u\n", last_raw_wind_count, last_raw_rf_count);
  } else {
    debugf2("[BOOT] Preserved ULP Anchors during sleep wakeup. Wind=%u, Rain=%u\n", last_raw_wind_count, last_raw_rf_count);
  }

  xTaskCreatePinnedToCore(rtcRead, "rtcReadTask", 8192, NULL, 2, &rtcRead_h,
                          1); // Core 1
  if (!skip_scheduler_task) {
    xTaskCreatePinnedToCore(scheduler, "schedulerTask", 14336, NULL, 2,
                            &scheduler_h, 1);
  } else {
    scheduler_h = NULL;
    debugln("[BOOT] schedulerTask SKIPPED (crash-loop guard).");
  }
  if (!skip_gprs_task) {
    xTaskCreatePinnedToCore(gprs, "gprsTask", 16384, NULL, 2, &gprs_h,
                            0); // Core 0
  } else {
    gprs_h = NULL;
    debugln("[BOOT] gprsTask SKIPPED (crash-loop guard). Data-store-only mode.");
    // Advance sync state so sleep logic doesn't wait for GPRS forever
    sync_mode = eHttpStop;
  }

  //    #if DEBUG == 1
  //        xTaskCreatePinnedToCore(stackMonitor, "StackMonitor", 2048, NULL,
  //        1, NULL, 1);
  //    #endif

#if (SYSTEM == 1) || (SYSTEM == 2)
  // #7: Smart Task Creation - Only spawn tasks if sensors were found in
  // initialize_hw
  if ((hdcType != HDC_UNKNOWN || bmeType != BME_UNKNOWN) && !skip_temphum_task) {
    xTaskCreatePinnedToCore(tempHum, "tempHumTask", 4096, NULL, 2, &tempHum_h,
                            1); // Core 1
  } else {
    tempHum_h = NULL;
    if (skip_temphum_task) debugln("[BOOT] tempHumTask SKIPPED (crash-loop guard).");
    else debugln("[BOOT] No T/H sensor (HDC or BME). tempHumTask NOT created.");
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

  if (!skip_ws_task) {
    xTaskCreatePinnedToCore(windSpeed, "windSpeedTask", 4096, NULL, 2,
                            &windSpeed_h, 1); // Core 1
  } else {
    windSpeed_h = NULL;
    debugln("[BOOT] windSpeedTask SKIPPED (crash-loop guard).");
  }
  if (!skip_wd_task) {
    xTaskCreatePinnedToCore(windDirection, "windDirectionTask", 4096, NULL, 2,
                            &windDirection_h, 1); // Core 1
  } else {
    windDirection_h = NULL;
    debugln("[BOOT] windDirectionTask SKIPPED (crash-loop guard).");
  }
#endif

  if (wired == 1 && !skip_lcd_task) {
    xTaskCreatePinnedToCore(lcdkeypad, "lcdkeypadTask", 6144, NULL, 2,
                            &lcdkeypad_h, 0); // Core 0
    if (wakeup_reason_is != timer) {
      digitalWrite(32, HIGH);
      delay(100);
    }
  } else if (wired == 1) {
    lcdkeypad_h = NULL;
    debugln("[BOOT] lcdkeypadTask SKIPPED (crash-loop guard).");
  }

  //    }

  // attachInterrupt(27, ext0_isr, FALLING); // DISABLE: Conflicts with ULP
  // Counting on Pin 27. ULP handles counting.
  debugln("ULP Counting Enabled (attachInterrupt 27 Disabled).");
  delay(1000);

  // Should be called only after getting the correct RF count from SPIFF. If
  // no RF is present, make rf_count.val = 0 and call this ()

  // REVERTED to AIO9_3.0 logic: Always load/restart ULP on setup to ensure counting is active.
  // v5.66 CRITICAL FIX: We MUST preserve the ULP's counted values before re-loading the code,
  // otherwise the injection permanently zeroes out any physical tips that fell during Deep Sleep
  // or right before an EXT_CPU_RESET crash!

  // Final cleanup of boot flags
  if (healer_reboot_in_progress) {
      healer_reboot_in_progress = false;
  }
}

void progressCallBack(size_t currSize, size_t totalSize) {
  // Silent during actual download to prevent console leak into Modem RX
  if (!ota_silent_mode) {
    debugf2("CALLBACK:  Update process at %d of %d bytes...\n", currSize,
            totalSize);
  }
  esp_task_wdt_reset(); // v5.79: Keep system alive during long flash writes
}

void initialize_hw() {
  RESET_REASON rr = rtc_get_reset_reason(0);
  if (rr == DEEPSLEEP_RESET) {
      vTaskDelay(500 / portTICK_PERIOD_MS); // Warm wakeup: 500ms sufficient
  } else {
      vTaskDelay(5000 / portTICK_PERIOD_MS); // Cold boot: full 5s for safety
  }

  // Setup Legacy ADC (To prevent driver_ng conflicts in Core 3.x)
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_5,
                            ADC_ATTEN_DB_11); // GPIO 33 (Battery)
  adc2_config_channel_atten(ADC2_CHANNEL_8,
                            ADC_ATTEN_DB_11); // GPIO 25 (Solar)

  // v5.66: Read Solar ADC immediately before modem power-on to prevent RF/UART contention
  int solar_raw_initial;
  if (adc2_get_raw(ADC2_CHANNEL_8, ADC_WIDTH_BIT_12, &solar_raw_initial) == ESP_OK) {
    solar = solar_raw_initial;
    solar_val = (solar / 4096.0) * 3.6 * 7.2;
  }

#if DEBUG == 1
  Serial.begin(115200);
#endif
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

  bool spiffs_mounted = false;
  uint32_t chip_size = ESP.getFlashChipSize();
  char hw_tag = (chip_size == 16*1024*1024) ? 'X' : ((chip_size == 8*1024*1024) ? 'H' : 'L');

  for (int i = 0; i < 3; i++) {
    // v5.75: Hardened Label-Based Mount (Dynamic VFS)
    // By using the label "spiffs", the ESP32 internally finds the partition segment 
    // even if it moved from 2.5MB (4MB chips) to 6MB (8MB chips).
    if (SPIFFS.begin(false, "/spiffs", 10, "spiffs")) { 
      spiffs_mounted = true;
      break;
    }
    debugln("[BOOT] SPIFFS mount failed. Retrying in 1s...");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  if (!spiffs_mounted) {
    debugln("[BOOT] CRITICAL: SPIFFS failed 3x. Safe-mode enabled (NO REFORMAT).");
    hw_tag = '!';
  } else {
    debugln("[BOOT] SPIFFS: OK");
    
    // v5.75: Secret Hardware Tag (Passive indicator)
    // Injected into the FLD_DELETE_DATA heading (Far-right column 15)
    snprintf(ui_data[FLD_DELETE_DATA].topRow, sizeof(ui_data[FLD_DELETE_DATA].topRow), "DELETE DATA?   %c", hw_tag);
    
    // v6.03: Atomic Recovery - Restore stranded .tmp files from power-fail
    if (!SPIFFS.exists("/gps_fix.txt") && SPIFFS.exists("/gps_fix.tmp")) {
        SPIFFS.rename("/gps_fix.tmp", "/gps_fix.txt");
        debugln("[FS] Recovered stranded gps_fix.tmp");
    }
    if (!SPIFFS.exists("/signature.txt") && SPIFFS.exists("/signature.tmp")) {
        SPIFFS.rename("/signature.tmp", "/signature.txt");
        debugln("[FS] Recovered stranded signature.tmp");
    }

    // v5.80: Trim Recovery (Adoption of stranded data after power-cut)
    if (!SPIFFS.exists("/unsent.txt") && SPIFFS.exists("/trim.tmp")) {
        SPIFFS.rename("/trim.tmp", "/unsent.txt");
        debugln("[FS] Recovered stranded trim.tmp -> unsent.txt");
    }
    if (!SPIFFS.exists("/ftpunsent.txt") && SPIFFS.exists("/ftptrim.tmp")) {
        SPIFFS.rename("/ftptrim.tmp", "/ftpunsent.txt");
        debugln("[FS] Recovered stranded ftptrim.tmp -> ftpunsent.txt");
    }
    
    // v5.80: Pointer Recovery
    if (!SPIFFS.exists("/unsent_pointer.txt") && SPIFFS.exists("/ptr.tmp")) {
        SPIFFS.rename("/ptr.tmp", "/unsent_pointer.txt");
        debugln("[FS] Recovered stranded ptr.tmp -> unsent_pointer.txt");
    }

    // v5.85: [Emergency Purge] - Clear orphan KWD files if they pile up at boot (Crash Recovery)
    int kwd_count = 0;
    File rootDir = SPIFFS.open("/");
    if (rootDir) {
        File f = rootDir.openNextFile();
        while(f) {
            char n[64];
            strncpy(n, f.name(), sizeof(n)-1); n[sizeof(n)-1] = '\0';
            int nLen = strlen(n);

            if (strstr(n, ".kwd") != NULL || strstr(n, ".swd") != NULL) {
                kwd_count++;
                if (kwd_count > 1) { // v5.85.1: Remove all but 1
                   f.close();
                   char full_path[128];
                   if (n[0] == '/') strncpy(full_path, n, sizeof(full_path)-1);
                   else snprintf(full_path, sizeof(full_path), "/%s", n);
                   full_path[sizeof(full_path)-1] = '\0';
                   SPIFFS.remove(full_path);
                   debugf("[FS] Emergency Boot Purge: %s\n", n);
                } else f.close();
            } else if (nLen >= 17 && (strcmp(n + nLen - 4, ".txt") == 0)) {
                // [HR-M02] Precise Migration Purge: Only target legacy 4-digit ID logs
                int offset = (n[0] == '/') ? 1 : 0;
                if (nLen == (17 + offset) && strncmp(n + offset + 4, "_20", 3) == 0) {
                    f.close();
                    char full_path[128];
                    if (n[0] == '/') strncpy(full_path, n, sizeof(full_path)-1);
                    else snprintf(full_path, sizeof(full_path), "/%s", n);
                    full_path[sizeof(full_path)-1] = '\0';
                    SPIFFS.remove(full_path);
                    debugf("[FS] Legacy Migration Purge: %s\n", n);
                } else f.close();
            } else f.close();
            f = rootDir.openNextFile();
        }
        rootDir.close();
    }
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

  // v5.79 FINAL STRUCTURAL FIX: Moved from Line 1060 to after SD Card initialization.
  // This ensures the Hardware is actually ready before we check for firmware.
  if (sd_card_ok && SD.exists("/firmware.bin")) {
    bool needUpdate = true;
    String sd_ver = ""; 

    if (SD.exists("/fw_version.txt")) {
      File vFile = SD.open("/fw_version.txt", FILE_READ);
      if (vFile) {
        sd_ver = vFile.readStringUntil('\n');
        sd_ver.trim();
        vFile.close();

        if (sd_ver.equalsIgnoreCase(UNIT_VER) || sd_ver.equalsIgnoreCase(FIRMWARE_VERSION)) {
          debugln("SD Version matches current firmware. Skipping update.");
          needUpdate = false;
        } else {
          debugf("[OTA] Version difference: [%s] vs [%s]. Proceeding...\n", sd_ver.c_str(), UNIT_VER);
        }
      }
    }

    if (needUpdate) {
      debugln("Found firmware.bin in SD... Verifying MD5...");
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
              if (sd_ver.isEmpty()) {
                debugln("Firmware identical (MD5 match, no version file). Skipping.");
                needUpdate = false;
              } else if (sd_ver.equalsIgnoreCase(UNIT_VER) || sd_ver.equalsIgnoreCase(FIRMWARE_VERSION)) {
                debugln("Firmware identical (MD5 + Version match). Skipping.");
                needUpdate = false;
              } else {
                debugln("MD5 match but Version MISMATCH. RE-FLASHING...");
                needUpdate = true;
              }
            }
          }
        }

        if (needUpdate) {
          firmware = SD.open("/firmware.bin", FILE_READ);
          if (firmware) {
            debugln("Starting Firmware Update...");
            Update.onProgress(progressCallBack);
            if (Update.begin(firmware.size(), U_FLASH)) {
              Update.writeStream(firmware);
              if (Update.end()) {
                debugln("Update finished! Storing MD5 and restarting...");
                diag_fw_just_updated = true;
                File md5Write = SPIFFS.open("/sd_fw_md5.txt", FILE_WRITE);
                if (md5Write) {
                  md5Write.print(sd_md5);
                  md5Write.close();
                }
                delay(1000);
                ESP.restart();
              } else {
                debugf("Update failed! Error: %s\n", Update.errorString());
              }
            }
            firmware.close();
          }
        }
      }
    }
  }

  last_activity_time = millis(); // v5.85: Trigger safety heartbeat starting now
  last_key_time = millis(); // v5.79: Prevent UI ghosting on first session
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
  bool safe_to_sleep_sync = false;
  
  int snap_min, snap_sec;
  portENTER_CRITICAL(&rtcTimeMux);
  snap_min = current_min;
  snap_sec = current_sec;
  portEXIT_CRITICAL(&rtcTimeMux);

  int seconds_to_next_15 = (15 - (snap_min % 15)) * 60 - snap_sec;
  bool too_close_to_slot = (seconds_to_next_15 < 45); 

  portENTER_CRITICAL(&syncMux);
  safe_to_sleep_sync = ((sync_mode == eHttpStop) || (sync_mode == eSMSStop) || (sync_mode == eExceptionHandled)) && !too_close_to_slot;
  bool snap_schedulerBusy = schedulerBusy; // v5.74 Fix #2: snapshot inside syncMux for atomicity
  portEXIT_CRITICAL(&syncMux);

  // v5.85: P7 - Dynamic Sleep Optimization (5s -> 2s for timer wakeups)
  uint32_t min_awake_ms = (wakeup_reason_is == ext0) ? 5000 : 2000;
  
  // v5.76.2: Mutex-Aware Sleep Guard (Total Race Prevention)
  // Check the actual semaphore handles to ensure no task is currently holding 
  // the modem or file system, regardless of high-level boolean flag state.
  bool modemIsBusy = (uxSemaphoreGetCount(modemMutex) == 0);
  bool fsIsBusy = (uxSemaphoreGetCount(fsMutex) == 0);

  // v5.85: Ghost-UI Safety Override
  // If we woke up by TIMER (scheduled) and all communicating tasks are DONE, 
  // we do not wait for the LCD to timeout (modem current noise can trigger it).
  // v5.79: Hardened Ghost-UI logic to check recently-pressed keys (last_key_time) 
  // AND protect active manual tasks (Calibration, Manual Syncs, OTA).
  bool manualTaskActive;
  portENTER_CRITICAL(&syncMux);
  manualTaskActive = (pending_manual_status || pending_manual_gps || pending_manual_health || force_ota || ota_writing_active || (calib_flag == 1));
  portEXIT_CRITICAL(&syncMux);

  bool uiIsGhosting = (wakeup_reason_is != ext0 && lcdkeypad_start == 1 && !manualTaskActive && (millis() - last_key_time > 60000) && (millis() > 60000));

  if ((millis() > min_awake_ms) &&
      safe_to_sleep_sync &&
      !modemIsBusy && !fsIsBusy &&
      (lcdkeypad_start == 0 || uiIsGhosting) && (wifi_active == false) &&
      (__atomic_load_n(&httpInitiated, __ATOMIC_ACQUIRE) == false) && (health_in_progress == false) &&
      (snap_schedulerBusy == false) && (gprs_started == false || sync_mode == eHttpStop) &&
      (pending_manual_status == false) && (pending_manual_gps == false) &&
      (pending_manual_health == false) &&
      (force_reboot == false) && (force_ota == false) &&
      (ota_writing_active == false)) {


    // v5.55 SAFETY VALVE: Total Communication Blackout Recovery
    // If the system has failed for 96 consecutive cycles (24 hours), assume 
    // Modem/ESP peripheral state is corrupted and force a hard system reboot.
    // v5.68 Fix: Threshold increased from 8 to 96 to prevent infinite reboot
    // loops during sustained rural cellular network outages.
    if (diag_consecutive_http_fails >= 96) {
       debugln("[HEAL] CRITICAL: 96 consecutive slots failed (24h). Forcing System Recovery Reboot...");
       diag_consecutive_http_fails = 0; // Reset so we don't boot loop if it's a real outage
       healer_reboot_in_progress = true; // v5.55: Protect counters
       vTaskDelay(2000 / portTICK_PERIOD_MS);
       ESP.restart();
    }

    // TIER 2 LIVE RACES: Mid-Slot Deep Sleep Race guard
    // Double-check the system hasn't just woken up in the last microsecond
    vTaskDelay(50 / portTICK_PERIOD_MS);
    
    // v5.78 Hardening: Consolidate Ghost-UI detection (Modem noise on GPIO 27)
    // v5.79: check for last_key_time and protect calibration to survive non-interactive sessions
    bool uiIsGhosting = (wakeup_reason_is != ext0 && lcdkeypad_start == 1 && !manualTaskActive && (millis() - last_key_time > 60000) && millis() > 60000);

    // v5.70: Catch any state changes during the 50ms yield (e.g. OTA started).
    bool sys_busy = false;
    portENTER_CRITICAL(&syncMux);
    bool race_sync_invalid = (sync_mode != eHttpStop && sync_mode != eSMSStop && sync_mode != eExceptionHandled);
    sys_busy = (snap_schedulerBusy || (gprs_started && sync_mode != eHttpStop) || health_in_progress || __atomic_load_n(&httpInitiated, __ATOMIC_ACQUIRE) || 
                force_reboot || force_ota || ota_writing_active || (lcdkeypad_start && !uiIsGhosting) || (wakeup_reason_is == ext0) || 
                wifi_active || race_sync_invalid || 
                pending_manual_status || pending_manual_gps || pending_manual_health);
    portEXIT_CRITICAL(&syncMux);

    if (sys_busy) {
        // v5.85: Final Safety Valve — Forced Duty Cycle Cap to protect battery.
        bool awake_cap_reached = (millis() > 300000); // 5 minutes absolute max
        bool idle_cap_reached = (millis() - last_activity_time > 180000); // 3 mins idle

        // v5.75: Mutex Hang Recovery
        if (awake_cap_reached && (modemIsBusy || fsIsBusy)) {
            debugln("[PWR] LOCKUP RECOVERY: Mutex held by hung task > 5min. Forcing Reboot...");
            ESP.restart();
        }

        // v5.85: Ghost-UI Safety Override
        // If we woke up by TIMER (scheduled) and all communicating tasks are DONE, 
        // we should not let a 'ghost' LCD trigger (modem noise) keep the device awake 
        // for the full 3-minute idle timeout. If it's been > 60s and tasks are stop, SHUT DOWN.
        bool scheduled_wake = (wakeup_reason_is != ext0);
        bool forced_sleep_allowed = scheduled_wake && safe_to_sleep_sync && !manualTaskActive && (millis() - last_key_time > 60000) && (millis() > 60000);

        if ((lcdkeypad_start == 0 || forced_sleep_allowed) && (awake_cap_reached || idle_cap_reached) && !too_close_to_slot) {
            debugln("[PWR] SAFETY VALVE: Duty-Cycle Cap or Forced Scheduled Sleep reached. Forcing Sleep...");
            // Force-reset all blockers
            portENTER_CRITICAL(&syncMux);
            gprs_started = false;
            sync_mode = eHttpStop;
            schedulerBusy = false;
            __atomic_store_n(&httpInitiated, false, __ATOMIC_RELEASE);
            health_in_progress = false;
            portEXIT_CRITICAL(&syncMux);
            http_ready = false; 
        } else {
            debugln("[PWR] Race Prevented: System became active during sleep delay. Aborting sleep.");
            return; // Re-enter the loop
        }
    }

    debugln("[PWR] All tasks done. Entering Deep Sleep...");
    esp_task_wdt_reset(); // Final pet before sleep
    start_deep_sleep();
  }

  esp_task_wdt_reset(); // Pet the watchdog for the loopTask
  vTaskDelay(500 / portTICK_PERIOD_MS); // v5.85: P5 - loop polling 100ms -> 500ms
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
  uint32_t chip_size = ESP.getFlashChipSize(); 
  debugf("[BOOT] Chip ID: %s | Flash: %d MB\n", chip_id, chip_size / (1024 * 1024));
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
    wind_debounced_state.val = 0;
    wind_debounce_cnt.val = 0;
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
      I_MOVI(R0, 5),                             // Debounce optimized to 5 loops (10ms total)
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
      I_GPIO_READ(GPIO_NUM_35), // R0 = current state
      I_MOVI(R3, 0), I_LD(R1, R3, U_WIND_PREV_STATE), // R1 = prev_state

      I_MOVR(R2, R0),     // R2 = current state
      I_ADDR(R0, R1, R2), // R0 = R1 + R2
      M_BGE(11, 2),        // If both are 1 (2), states haven't changed -> Jump 11
      M_BL(11, 1),         // If both are 0 (0), states haven't changed -> Jump 11

      // States are different, start debouncing
      I_MOVI(R3, 0), I_ST(R2, R3, U_WIND_PREV_STATE), // prev_state = current
      I_MOVI(R0, 3),                                  // Debounce optimized to 3 loops (3ms total)
      I_ST(R0, R3, U_WIND_DEBOUNCE_CNT),
      M_BX(5), // Skip to exit while debouncing

      M_LABEL(11), I_MOVI(R3, 0), I_LD(R0, R3, U_WIND_DEBOUNCE_CNT),
      M_BGE(12, 1), // If count > 0, go to decrement
      M_BX(5),      // If 0, no change, skip to exit

      M_LABEL(12), I_SUBI(R0, R0, 1), I_MOVI(R3, 0),
      I_ST(R0, R3, U_WIND_DEBOUNCE_CNT), M_BGE(5, 1), // If still > 0, exit

      // Debounce finished! Validate state change
      I_MOVI(R3, 0), I_LD(R1, R3, U_WIND_PREV_STATE), // R1 = new stable state
      I_LD(R2, R3, U_WIND_DEBOUNCED_STATE),           // R2 = old stable state
      I_ADDR(R0, R1, R2), M_BGE(5, 2),           // Steady High (1+1=2) -> Skip
      M_BL(5, 1),                                // Steady Low (0+0=0) -> Skip

      I_ST(R1, R3, U_WIND_DEBOUNCED_STATE), // Update stable state

      // Transition detected! Is it Rising Edge (0 -> 1)?
      I_ADDI(R0, R1, 0),
      M_BL(5, 1), // If new state is Low (0), it's a 1->0 transition, skip (Wind is Rising Edge only)

      // Rising Edge!
      // v5.68 FIX: ULP Anemometer Debounce added successfully
      I_LD(R0, R3, U_WIND_COUNT), I_ADDI(R0, R0, 1), I_ST(R0, R3, U_WIND_COUNT),

      M_LABEL(5), I_HALT()};

  rtc_gpio_init(GPIO_NUM_2);
  rtc_gpio_set_direction(GPIO_NUM_2, RTC_GPIO_MODE_OUTPUT_ONLY);

  rtc_gpio_init(GPIO_NUM_34);
  rtc_gpio_set_direction(GPIO_NUM_34, RTC_GPIO_MODE_INPUT_ONLY);

  rtc_gpio_init(GPIO_NUM_35);
  rtc_gpio_set_direction(GPIO_NUM_35, RTC_GPIO_MODE_INPUT_ONLY);

  // [Task 2.1 Fix]: GPIO 34 & 35 are input-only and physically lack internal pull-up 
  // resistors on ESP32 silicon. rtc_gpio_pullup_en() natively does nothing for them.
  // Hardware PCB *MUST* have external 10k resistors installed, otherwise these lines
  // will float violently and cause false ULP pulse accumulation!

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

// v5.59: Stack Overflow Hook to persistent RTC RAM
extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  if (pcTaskName) {
    // v5.74 Fix #16: Count consecutive crashes of the same task.
    // After 3 crashes, stop recreating it to break the reboot loop.
    if (strncmp(diag_crash_task_rtc, pcTaskName, 15) == 0) {
      diag_crash_count++;
    } else {
      diag_crash_count = 1;
      strncpy(diag_crash_task_rtc, pcTaskName, 15);
      diag_crash_task_rtc[15] = '\0';
    }
    strncpy(diag_crash_task, pcTaskName, 15);
    diag_crash_task[15] = '\0';
  }
  vTaskDelay(100 / portTICK_PERIOD_MS);
  ESP.restart();
}
