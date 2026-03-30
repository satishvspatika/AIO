#ifndef USER_CONFIG_H
#define USER_CONFIG_H

/* =========================================================================
 *  USER CONFIGURATION FILE
 *  Edit these values to configure the firmware before flashing or building.
 * ========================================================================= */

// --- 1. CORE SYSTEM IDENTITY ---
#define SYSTEM 0              // 0: TRG, 1: TWS, 2: TWS-RF Addon
#define UNIT_CFG "BIHAR_TRG" // Exact station network name:
// SYSTEM 0 options: KSNDMC_TRG, BIHAR_TRG, SPATIKA_GEN
// SYSTEM 1 options: KSNDMC_TWS, KSNDMC_TWS-AP
// SYSTEM 2 options: KSNDMC_ADDON, SPATIKA_GEN

#define FIRMWARE_VERSION "5.73"

#define ENABLE_HEALTH_REPORT                                                   \
  0 // Master Switch: Enable automated device health reporting
#define TEST_HEALTH_DEFAULT                                                    \
  0 // Default frequency: 1 (Every 15 mins), 0 (Daily at 11am), 2 (Disabled)

// --- 2. COMPILE OPTIONS ---
#define DEBUG 1 // 1: Enable Serial Logs (Dev), 0: Production (Saves ROM)
#define ENABLE_WEBSERVER                                                       \
  0 // 0: Disable, 1: Enable (requires 8MB flash partition)

// --- 3. HARDWARE CAPABILITIES ---
#define ENABLE_PRESSURE_SENSOR 0 // 0: Disable BMP/BME routines, 1: Enable
#define DEFAULT_RF_RESOLUTION                                                  \
  0.50 // Factory default rain resolution (0.50mm / 0.25mm)
#define ENABLE_CALIB_TEST                                                      \
  0 // 1: Enable CALIB TEST in UI, 0: Field deployment mode

// --- 4. NETWORK BEHAVIOR ---
#define FILLGAP 1
#define FTP_CHUNK_SIZE 15 // v5.52 ENH-2: Standardized chunk size for backlog

#include "secrets.h"

// --- 5. SERVER ENDPOINTS & SECURITY ---
// Spatika Health Server (Contabo VPS - plain HTTP, no SSL needed)
#define HEALTH_SERVER_IP "75.119.148.192"
#define HEALTH_SERVER_PORT "80"
#define OTA_SERVER_PORT "80"
#define HEALTH_SERVER_PATH "/health"

// Note: Secure keys (SEC_*) and FTP passwords (FTP_PASS_*) have been moved
// to secrets.h for security. Do not add plain-text secrets back here.

#endif // USER_CONFIG_H
