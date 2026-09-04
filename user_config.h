#ifndef USER_CONFIG_H
#define USER_CONFIG_H

/* =========================================================================
 *  USER CONFIGURATION FILE
 *  Edit these values to configure the firmware before flashing or building.
 * ========================================================================= */

// --- 1. CORE SYSTEM IDENTITY ---
#define SYSTEM 2
#define UNIT_CFG "SPATIKA_ADDON_AP"
// SYSTEM 0 options: KSNDMC_TRG, BIHAR_TRG, SPATIKA_TRG
// SYSTEM 1 options: KSNDMC_TWS, KSNDMC_TWS-AP, SPATIKA_TWS
// SYSTEM 2 options: KSNDMC_ADDON, SPATIKA_ADDON, SPATIKA_ADDON_AP

#define FIRMWARE_VERSION "6.27"

#define ENABLE_HEALTH_REPORT                                                   \
  0 // Master Switch: Enable automated device health reporting
#define TEST_HEALTH_DEFAULT                                                    \
  0 // Default frequency: 1 (Every 15 mins), 0 (Daily at 11am), 2 (Disabled)

// --- 2. COMPILE OPTIONS ---
#define USE_NUVOTON_UI                                                         \
  0 // 1: Use Nuvoton UART LCD/Keypad, 0: Use I2C/GPIO Matrix
#ifndef LCD_I2C_ADDR
#define LCD_I2C_ADDR 0x27 // 0x27 (Default PCF8574T) or 0x3F (PCF8574AT)
#endif
#define DEBUG 1 // 1: Enable Serial Logs (Dev), 0: Production (Saves ROM)
#define ENABLE_WEBSERVER                                                       \
  0 // 0: Disable, 1: Enable (requires 8MB flash partition)

// --- 3. HARDWARE CAPABILITIES ---
#define ENABLE_PRESSURE_SENSOR 1 // 0: Disable BMP/BME routines, 1: Enable
#define DEFAULT_RF_RESOLUTION                                                  \
  0.5 // Factory default rain resolution (0.50mm / 0.25mm)
#define WIND_TEETH_COUNT 2.0 // H-02: Customizable anemometer teeth divisor
#define WIND_SAMPLING_US                                                       \
  1000 // ULP wakeup period in microseconds (1000 = 1ms for high-res pulse
       // sampling)
#define WIND_DEBOUNCE_CYCLES 1 // Pulse stability cycles (1 = 1 loop @ 1ms)
#define WIND_DIR_ADC_MAX                                                       \
  3480 // Calibrated max raw ADC value at VCC (3.3V) right before dead zone
#define ENABLE_CALIB_TEST 0 // 1: IMD CALIB TEST in UI, 0: Field deployment mode

// --- 4. NETWORK BEHAVIOR ---
#define FILLGAP 1
#define FTP_CHUNK_SIZE 15 // v5.52 ENH-2: Standardized chunk size for backlog

#include "secrets.h"

// --- 5. SERVER ENDPOINTS & SECURITY ---
// Spatika Health Server (Contabo VPS - Plain HTTP API)
#define HEALTH_SERVER_DOMAIN "devhlt.spatika.net"
#define HEALTH_SERVER_IP "75.119.148.192"
#define HEALTH_SERVER_PORT "80"
#define OTA_SERVER_PORT "80"
#define HEALTH_SERVER_PATH "/health"

// Note: Secure keys (SEC_*) and FTP passwords (FTP_PASS_*) have been moved
// to secrets.h for security. Do not add plain-text secrets back here.

#endif // USER_CONFIG_H
