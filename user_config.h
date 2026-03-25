#ifndef USER_CONFIG_H
#define USER_CONFIG_H

/* =========================================================================
 *  USER CONFIGURATION FILE
 *  Edit these values to configure the firmware before flashing or building.
 * ========================================================================= */

// --- 1. CORE SYSTEM IDENTITY ---
#define SYSTEM 2               // 0: TRG, 1: TWS, 2: TWS-RF Addon
#define UNIT_CFG "SPATIKA_GEN" // Exact station network name:
// SYSTEM 0 options: KSNDMC_TRG, BIHAR_TRG, SPATIKA_GEN
// SYSTEM 1 options: KSNDMC_TWS, KSNDMC_TWS-AP
// SYSTEM 2 options: KSNDMC_ADDON, SPATIKA_GEN

#define FIRMWARE_VERSION "6.02"

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
#define KEYPAD_TYPE                                                            \
  0 // 0: 5-Pin GPIO (Legacy), 1: PCF8574 I2C Expander, 2: Nuvoton M51 I2C
    // Processor
// --- 4. NETWORK BEHAVIOR ---
#define FILLGAP 1
#define FORCE_2G_ONLY                                                          \
  0 // v6.01 Fix: BSNL 4G support enabled. AT+CNMP=2 by default.
#define FTP_CHUNK_SIZE 15 // v5.52 ENH-2: Standardized chunk size for backlog

// --- 5. SERVER ENDPOINTS & SECURITY ---
// Spatika Health Server (Contabo VPS - plain HTTP, no SSL needed)
#define HEALTH_SERVER_IP "75.119.148.192"
#define HEALTH_SERVER_PORT "80"
#define OTA_SERVER_PORT "80"
#define HEALTH_SERVER_PATH "/health"
#define HEALTH_AUTH_KEY                                                        \
  "662172e5-42d1-4437-975c-db36c6a461e8" // Phase 2: Mandatory Device Secret

// TIER 1 API Keys (Migrated from OTA payload builders for compile-time
// stripping)
#define SEC_KS_TRG_OLD "sit1040"
#define SEC_KS_TRG_NEW "pse2420"
#define SEC_BIH_GOV "bmsk1234"
#define SEC_BIH_SPT "bmsk12345"
#define SEC_EX_SIT "sit"
#define SEC_SPT_GEN "sitgen100"
#define SEC_KS_TWS "climate4p2013"
#define SEC_KS_ADDON "rfclimate5p13"
#define SEC_SPT_TWS_RF "wsgen2014"

// FTP Passwords
#define FTP_PASS_BIH_D "rf24hrcsv2021"
#define FTP_PASS_BIH_GEN "airdata2024"
#define FTP_PASS_KS_TRG "trgsp#123"
#define FTP_PASS_KS_TWS "twssp#987"
#define FTP_PASS_KS_ADD "sittao#10"

#endif // USER_CONFIG_H
