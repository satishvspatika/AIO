#ifndef USER_CONFIG_H
#define USER_CONFIG_H

/* =========================================================================
 *  USER CONFIGURATION FILE
 *  Edit these values to configure the firmware before flashing or building.
 * ========================================================================= */

// --- 1. CORE SYSTEM IDENTITY ---
#define SYSTEM 0
#define UNIT_CFG "KSNDMC_TRG"
// SYSTEM 0 options: KSNDMC_TRG, BIHAR_TRG, SPATIKA_GEN
// SYSTEM 1 options: KSNDMC_TWS, KSNDMC_TWS-AP
// SYSTEM 2 options: KSNDMC_ADDON, SPATIKA_GEN
// SYSTEM 3 options: SPATIKA_GEN - AP

#define FIRMWARE_VERSION "6.63"

// v6.57: the daily health report goes with the 13:00 OTA check (health_day.ino), as on the LPC
// units. The separate Spatika health server (Contabo) and its remote commands are gone.

// --- 2. COMPILE OPTIONS ---
#define USE_NUVOTON_UI 1 // 1: Use Nuvoton UART LCD/Keypad, 0: Use I2C/GPIO Matrix
#ifndef LCD_I2C_ADDR
#define LCD_I2C_ADDR 0x27 // 0x27 (Default PCF8574T) or 0x3F (PCF8574AT)
#endif
#define DEBUG 0 // 1: Enable Serial Logs (Dev), 0: Production (Saves ROM)
#define ENABLE_WEBSERVER 1 // 0: Disable, 1: Enable (requires 8MB flash partition)

// --- 3. HARDWARE CAPABILITIES ---
#define ENABLE_PRESSURE_SENSOR 0 // 0: Disable BMP/BME routines, 1: Enable
#define DEFAULT_RF_RESOLUTION 0.5 // Factory default rain resolution (0.50mm / 0.25mm)
#define WIND_TEETH_COUNT 2.0 // H-02: Customizable anemometer teeth divisor
#define WIND_SAMPLING_US 1000 // ULP wakeup period in microseconds
#define WIND_DEBOUNCE_CYCLES 1 // Pulse stability cycles (1 = 1 loop @ 1ms)
// v6.56: a level must hold for about (N+1) ms to count. Pulses/s = 4.44 x m/s, so the
// highest wind still counted is about 56 m/s at 1, 37 m/s at 2, 28 m/s at 3. Raise it
// only after scoping the reed switch's bounce on the real anemometer.
#define WIND_DIR_ADC_MAX 4095 // Full 12-bit ESP32 ADC max (3.3V)
#define ENABLE_CALIB_TEST 0 // 1: IMD CALIB TEST in UI, 0: Field deployment mode
// v6.56 M-9: 0 = brownout detector off (as before: the modem's 2 A transmit bursts made
// some boards reset). 1 = on at the core default (~2.43 V). Turn on only after checking on
// a scope that the 3.3 V rail stays above ~2.6 V while the modem transmits.
#define ENABLE_BROWNOUT_RESET 0

// --- 4. NETWORK BEHAVIOR ---
#define FILLGAP 1
// v6.56: FTP removed. Every system sends its backlog by HTTP (FTP_CHUNK_SIZE and
// ENABLE_HTTP_BACKLOG_FALLBACK are gone).

// v6.57: VARSHA SMS commands (sms_remote.ino), as the LPC units. GET_STATUS, GET_GPS and
// GET_CONFIG answer anyone; SET_ commands, STOP/RESUME, ERASE_DATA and the number list only these
// office numbers (last 10 digits; up to 5). This is the list until changed by VARSHA ADD_NUM /
// DEL_NUM, which keep it in NVS. Same list as the LPC units (DMC Airtel, DMC BSNL, Spatika
// Airtel, Satish Jio, Satish Airtel).
#define SMS_OFFICE_NUMBERS "9980945474,9449020611,9741391102,9108052058,9980052058"

// v6.56: ESP32 OTA from the LPC-AIO OTA server (ota_esp.ino). Checked twice a day (about 13:00
// and 01:00, spread by station id); every slot while new firmware is on trial (v6.56c), or on a
// unit with /ota_now.txt in SPIFFS. VARSHA SET_OTA_SERVER changes the address (v6.57).
#define ENABLE_ESP_OTA 1
#define OTA_SERVER_HOST "devctl.spatika.net" // by name: some M2M filters pass only named hosts
#define OTA_SERVER_TCP_PORT 80               // nginx forwards /lpc/ to the OTA service
#define OTA_DAY_SLOT 17                      // sampleNo of 13:00 (08:45 = 0)
#define OTA_NIGHT_SLOT 65                    // sampleNo of 01:00
#define OTA_CHECK_EVERY_SLOT 0               // 1 = bench build: check every slot

#include "secrets.h"

// --- 5. SERVER ENDPOINTS & SECURITY ---

// Bihar TRG Server Selection
// 0: rtdasbmsk.spatika.net (Govt Server - 164.100.130.199:8085)
// 1: bmskinhs.spatika.net (INHS Server - Port 8085)
#define BIHAR_SERVER_SELECT 0 // 0: rtdasbmsk.spatika.net (default from v6.56b: works on the Airtel Bihar SIM), 1: bmsknew.spatika.net

#define BIHAR_SERVER1_DOMAIN "rtdasbmsk.spatika.net"
#define BIHAR_SERVER1_IP     "164.100.130.199"
#define BIHAR_SERVER1_PORT   "8085"
#define BIHAR_SERVER1_PATH   "/Home/UpdateTRGData"

#define BIHAR_SERVER2_DOMAIN "bmsknew.spatika.net"
#define BIHAR_SERVER2_IP     "bmsknew.spatika.net" // Resolved by modem DNS (or replace with direct IP once assigned)
#define BIHAR_SERVER2_PORT   "8085"
#define BIHAR_SERVER2_PATH   "/Home/UpdateTRGData"

#if (BIHAR_SERVER_SELECT == 1)
  #define BIHAR_ACTIVE_DOMAIN BIHAR_SERVER2_DOMAIN
  #define BIHAR_ACTIVE_IP     BIHAR_SERVER2_IP
  #define BIHAR_ACTIVE_PORT   BIHAR_SERVER2_PORT
  #define BIHAR_ACTIVE_PATH   BIHAR_SERVER2_PATH
#else
  #define BIHAR_ACTIVE_DOMAIN BIHAR_SERVER1_DOMAIN
  #define BIHAR_ACTIVE_IP     BIHAR_SERVER1_IP
  #define BIHAR_ACTIVE_PORT   BIHAR_SERVER1_PORT
  #define BIHAR_ACTIVE_PATH   BIHAR_SERVER1_PATH
#endif

// Server 2 (a second data server): none until set by SMS, VARSHA SET_SERVER_2
// host=..;port=..;path=/..;key=.., then VARSHA SET_SERVER_MODE 1 or 2. v6.57: was the Contabo
// health server, which is no longer used.
#define SERVER2_DOMAIN ""
#define SERVER2_IP ""
#define SERVER2_PORT "80"
#define SERVER2_PATH ""
#define SERVER2_KEY ""

// Note: Secure keys (SEC_*) have been moved
// to secrets.h for security. Do not add plain-text secrets back here.

#endif // USER_CONFIG_H
