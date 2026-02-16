# MISSION CONTROL SYSTEM - Complete Implementation Specification

**Version:** 5.4  
**Date:** 2026-02-16  
**Author:** Satish Kripa Vasan  
**Purpose:** Passive, FTP-based Command & Control system for remote ESP32 fleet management

---

## TABLE OF CONTENTS

1. [System Overview](#system-overview)
2. [Architecture](#architecture)
3. [Server-Side Setup](#server-side-setup)
4. [Firmware Implementation](#firmware-implementation)
5. [Command Reference](#command-reference)
6. [Configuration Settings](#configuration-settings)
7. [Google Sheets Integration](#google-sheets-integration)
8. [Testing & Validation](#testing--validation)
9. [Troubleshooting](#troubleshooting)

---

## SYSTEM OVERVIEW

### Problem Statement
- **Cannot rely on SMS:** Unknown mobile numbers, unreliable delivery
- **Cannot control client server:** No ability to push commands via HTTP
- **Need robust OTA:** Firmware updates for 200+ devices in the field
- **Need remote diagnostics:** Troubleshoot issues without site visits

### Solution: "Daily Mission Check"
- **Passive Pull:** Device checks FTP server once daily (12:30 PM default)
- **Simple Commands:** Text files with KEY=VALUE pairs
- **Flexible Targeting:** Mass updates (all), group updates (50 stations), or individual (1 station)
- **Feedback via Google Sheets:** Results reported in existing health dashboard

---

## ARCHITECTURE

### Communication Flow

```
┌─────────────────────────────────────────────────────────────┐
│                    FTP SERVER (dota.spatika.net)            │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  /missions/KSNDMC_TRG/                               │   │
│  │    ├── global_mission.txt  (Mass/Group commands)     │   │
│  │    ├── SIT099.txt          (Specific to SIT099)      │   │
│  │    └── firmware_v5.4.bin   (Binary file)             │   │
│  └──────────────────────────────────────────────────────┘   │
│                           ▲                                  │
│                           │ Daily Check (12:30 PM)           │
│                           │ Download mission file            │
│                           │                                  │
│  ┌────────────────────────┴──────────────────────────────┐  │
│  │                    ESP32 DEVICE                        │  │
│  │  - Parses mission file                                 │  │
│  │  - Validates TARGET_IDS                                │  │
│  │  - Executes command                                    │  │
│  │  - Stores result in RTC memory                         │  │
│  └────────────────────────────────────────────────────────┘  │
│                           │                                  │
│                           │ Next Day (11:00 PM)              │
│                           ▼                                  │
│  ┌──────────────────────────────────────────────────────┐   │
│  │            GOOGLE SHEETS (Health Report)             │   │
│  │  Columns: Station | Battery | ... | Last_Mission |   │   │
│  │           Mission_Status | Mission_Details           │   │
│  └──────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

### File Structure on FTP Server

```
/
├── missions/                          # Command Center (INPUT)
│   ├── KSNDMC_TRG/
│   │   ├── global_mission.txt         # For all/some TRG stations
│   │   ├── SIT099.txt                 # Specific to SIT099
│   │   ├── SIT100.txt                 # Specific to SIT100
│   │   └── firmware_v5.4.bin          # Firmware binary
│   │
│   ├── KSNDMC_TWS/
│   │   ├── global_mission.txt
│   │   └── firmware_v5.4.bin
│   │
│   └── BIHAR_TRG/
│       └── global_mission.txt
│
└── uploads/                           # Device Uploads (OUTPUT)
    ├── logs/                          # Requested log files
    │   ├── SIT099_20260214.txt
    │   └── SIT100_20260215.txt
    │
    └── debug_logs/                    # Debug logs (when debug_mode=1)
        ├── SIT099_debug.txt
        └── SIT100_debug.txt
```

---

## SERVER-SIDE SETUP

### FTP Server Details
- **Host:** `dota.spatika.net` (IP: `89.32.144.163`)
- **Credentials:** User: `dota_dmc`, Password: `airdata2016`
- **Protocol:** FTP (Plain, not FTPS)

### Creating Mission Files

#### Global Mission (Mass/Group Update)
**File:** `/missions/KSNDMC_TRG/global_mission.txt`

**Content:**
```ini
MISSION_ID=FW_5.4_GLOBAL
COMMAND=UPDATE_FIRMWARE
FILE=firmware_v5.4.bin
VERSION=5.4
TARGET_IDS=SIT099,SIT100,SIT105
# Omit TARGET_IDS for ALL stations
```

#### Specific Mission (Single Station)
**File:** `/missions/KSNDMC_TRG/SIT099.txt`

**Content:**
```ini
MISSION_ID=RECOVER_FEB14
COMMAND=UPLOAD_LOG
DATE=2026-02-14
```

### Mission File Format Rules
1. **Plain Text:** UTF-8 encoding, Unix line endings (`\n`)
2. **KEY=VALUE:** One per line, no spaces around `=`
3. **Comments:** Lines starting with `#` are ignored
4. **MISSION_ID:** Must be unique to prevent re-execution
5. **Case Sensitive:** Use exact key names (e.g., `COMMAND`, not `command`)

---

## FIRMWARE IMPLEMENTATION

### Files to Modify/Create

#### 1. `globals.h` - Add Variables and Prototypes

**Location 1:** After line 293 (after `RTC_DATA_ATTR bool diag_rtc_battery_ok = true;`)

```cpp
// Mission Control System
RTC_DATA_ATTR char last_global_mission_id[32] = "";
RTC_DATA_ATTR char last_specific_mission_id[32] = "";
RTC_DATA_ATTR char last_mission_status[16] = "IDLE";      // "SUCCESS", "FAIL_BATTERY", etc.
RTC_DATA_ATTR char last_mission_details[64] = "";         // "Updated to v5.4"
RTC_DATA_ATTR char last_mission_command[32] = "";         // "UPDATE_FIRMWARE"
```

**Location 2:** Before line 623 (before `void scheduler(void *parameter);`)

```cpp
// Mission Control Function Prototypes
void check_daily_mission();
void load_config_from_nvs();
void save_config_to_nvs(const char* key, const char* value);
bool validate_config_value(const char* key, const char* value);
```

**Location 3:** Add global config variables (after existing globals around line 300)

```cpp
// Runtime Configuration (loaded from NVS)
bool debug_mode_enabled = false;
int mission_check_hour = 12;              // Default 12:30 PM
int upload_interval_mins = 15;            // Default 15 minutes
bool backup_server_enabled = false;
char backup_server_url[128] = "";
char backup_server_mode[16] = "FAILOVER"; // "FAILOVER" or "DUAL"
```

---

#### 2. `mission_control.ino` - NEW FILE (Core Logic)

**Purpose:** Handles mission checking, parsing, and execution

**Key Functions:**

##### `check_daily_mission()`
Main entry point, called once per day.

```cpp
void check_daily_mission() {
  debugln("\n[MISSION] Starting Daily Mission Check...");
  
  // 1. Safety Checks
  if (bat_val < 3.7) {
    strcpy(last_mission_status, "SKIP_LOW_BAT");
    return;
  }
  
  // 2. Ensure GPRS is ready
  if (gprs_mode != eGprsSignalOk) start_gprs();
  if (gprs_mode != eGprsSignalOk) {
    strcpy(last_mission_status, "FAIL_SIGNAL");
    return;
  }
  
  // 3. Login to FTP (DOTA server)
  int prev_daily = send_daily;
  send_daily = 0; // Force DOTA server
  if (setup_ftp() != 1) {
    strcpy(last_mission_status, "FAIL_FTP_LOGIN");
    send_daily = prev_daily;
    return;
  }
  
  // 4. Check Specific Mission First (Priority)
  char specificPath[64];
  snprintf(specificPath, 64, "/missions/%s/%s.txt", UNIT, ftp_station);
  bool executed = check_and_execute_mission(specificPath, true);
  
  // 5. Check Global Mission (if no specific executed)
  if (!executed) {
    char globalPath[64];
    snprintf(globalPath, 64, "/missions/%s/global_mission.txt", UNIT);
    check_and_execute_mission(globalPath, false);
  }
  
  // 6. Upload debug log if debug_mode enabled
  if (debug_mode_enabled && SPIFFS.exists("/debug_log.txt")) {
    upload_debug_log();
  }
  
  // 7. Cleanup
  SerialSIT.println("AT+CFTPSLOGOUT");
  waitForResponse("OK", 5000);
  send_daily = prev_daily;
  
  debugln("[MISSION] Check complete");
}
```

##### `check_and_execute_mission(path, isSpecific)`
Downloads and parses a mission file.

```cpp
bool check_and_execute_mission(const char* filepath, bool isSpecific) {
  // 1. Download file to SPIFFS /mission_temp.txt
  char localFile[32] = "/mission_temp.txt";
  
  // AT+CFTPSGETFILE command to download
  char cmd[128];
  snprintf(cmd, 128, "AT+CFTPSGETFILE=\"%s\",0", filepath);
  SerialSIT.println(cmd);
  
  String response = waitForResponse("OK", 30000);
  if (response.indexOf("ERROR") >= 0) {
    debugln("[MISSION] File not found or download failed");
    return false;
  }
  
  // Wait for download completion
  vTaskDelay(2000);
  
  // 2. Check if file exists in SPIFFS
  if (!SPIFFS.exists(localFile)) {
    return false;
  }
  
  // 3. Parse mission file
  File mFile = SPIFFS.open(localFile, FILE_READ);
  if (!mFile) return false;
  
  char mission_id[32] = "";
  char command[32] = "";
  char target_ids[256] = "";
  char param1[128] = "";  // FILE, DATE, KEY, etc.
  char param2[128] = "";  // VERSION, VALUE, etc.
  
  while (mFile.available()) {
    String line = mFile.readStringUntil('\n');
    line.trim();
    
    if (line.length() == 0 || line.startsWith("#")) continue;
    
    int eqPos = line.indexOf('=');
    if (eqPos < 0) continue;
    
    String key = line.substring(0, eqPos);
    String value = line.substring(eqPos + 1);
    
    if (key == "MISSION_ID") strncpy(mission_id, value.c_str(), 31);
    else if (key == "COMMAND") strncpy(command, value.c_str(), 31);
    else if (key == "TARGET_IDS") strncpy(target_ids, value.c_str(), 255);
    else if (key == "FILE" || key == "DATE" || key == "KEY") 
      strncpy(param1, value.c_str(), 127);
    else if (key == "VERSION" || key == "VALUE") 
      strncpy(param2, value.c_str(), 127);
  }
  mFile.close();
  SPIFFS.remove(localFile);
  
  // 4. Check if already executed
  const char* lastId = isSpecific ? last_specific_mission_id : last_global_mission_id;
  if (strcmp(mission_id, lastId) == 0) {
    debugln("[MISSION] Already executed");
    return false;
  }
  
  // 5. Check TARGET_IDS (for global missions)
  if (!isSpecific && strlen(target_ids) > 0) {
    if (!is_station_in_list(ftp_station, target_ids)) {
      debugln("[MISSION] Not in target list");
      return false;
    }
  }
  
  // 6. Execute command
  execute_mission_command(command, param1, param2);
  
  // 7. Update last executed ID
  if (isSpecific) {
    strncpy(last_specific_mission_id, mission_id, 31);
  } else {
    strncpy(last_global_mission_id, mission_id, 31);
  }
  
  return true;
}
```

##### `execute_mission_command(command, param1, param2)`
Executes the parsed command.

```cpp
void execute_mission_command(const char* cmd, const char* p1, const char* p2) {
  strcpy(last_mission_command, cmd);
  
  if (strcmp(cmd, "UPDATE_FIRMWARE") == 0) {
    // p1 = FILE, p2 = VERSION
    float newVer = atof(p2);
    float curVer = atof(FIRMWARE_VERSION);
    
    if (newVer <= curVer) {
      strcpy(last_mission_status, "SKIP_SAME_VER");
      snprintf(last_mission_details, 63, "Already on v%.1f", curVer);
      return;
    }
    
    // Download and flash
    if (fetchFromFtpAndUpdate(p1) == 1) {
      strcpy(last_mission_status, "SUCCESS");
      snprintf(last_mission_details, 63, "v%.1f->v%.1f", curVer, newVer);
      // Device will restart automatically
    } else {
      strcpy(last_mission_status, "FAIL_DOWNLOAD");
      strcpy(last_mission_details, "FTP error");
    }
  }
  
  else if (strcmp(cmd, "UPLOAD_LOG") == 0) {
    // p1 = DATE (YYYY-MM-DD)
    char logFile[64];
    // Convert DATE to filename format
    char dateStr[16];
    strncpy(dateStr, p1, 15);
    // Remove hyphens: 2026-02-14 -> 20260214
    char cleanDate[16];
    int j = 0;
    for (int i = 0; dateStr[i]; i++) {
      if (dateStr[i] != '-') cleanDate[j++] = dateStr[i];
    }
    cleanDate[j] = '\0';
    
    snprintf(logFile, 63, "/%s_%s.txt", ftp_station, cleanDate);
    
    if (SPIFFS.exists(logFile)) {
      // Upload to FTP /uploads/logs/
      upload_file_to_ftp(logFile, "/uploads/logs/");
      strcpy(last_mission_status, "SUCCESS");
      snprintf(last_mission_details, 63, "Uploaded %s", logFile);
    } else {
      strcpy(last_mission_status, "FAIL_NOT_FOUND");
      snprintf(last_mission_details, 63, "File %s missing", logFile);
    }
  }
  
  else if (strcmp(cmd, "GET_DIAGNOSTICS") == 0) {
    // Generate diagnostics and include in next health report
    generate_diagnostics_report();
    strcpy(last_mission_status, "SUCCESS");
    strcpy(last_mission_details, "See health report");
  }
  
  else if (strcmp(cmd, "CONFIG_UPDATE") == 0) {
    // p1 = KEY, p2 = VALUE
    if (validate_config_value(p1, p2)) {
      save_config_to_nvs(p1, p2);
      strcpy(last_mission_status, "SUCCESS");
      snprintf(last_mission_details, 63, "%s=%s", p1, p2);
      
      // Restart if needed
      if (strcmp(p1, "station") == 0 || strcmp(p1, "rf_resolution") == 0) {
        vTaskDelay(2000);
        ESP.restart();
      }
    } else {
      strcpy(last_mission_status, "FAIL_INVALID");
      snprintf(last_mission_details, 63, "Bad value for %s", p1);
    }
  }
  
  else if (strcmp(cmd, "RESTART_SYSTEM") == 0) {
    if (bat_val > 3.8) {
      strcpy(last_mission_status, "SUCCESS");
      strcpy(last_mission_details, "Restarting...");
      vTaskDelay(2000);
      ESP.restart();
    } else {
      strcpy(last_mission_status, "FAIL_LOW_BAT");
      snprintf(last_mission_details, 63, "Bat %.1fV < 3.8V", bat_val);
    }
  }
  
  else {
    strcpy(last_mission_status, "FAIL_UNKNOWN_CMD");
    strcpy(last_mission_details, cmd);
  }
}
```

##### Helper Functions

```cpp
bool is_station_in_list(const char* station, const char* list) {
  // Check if "SIT099" is in "SIT099,SIT100,SIT105"
  String listStr = String(list);
  String stationStr = String(station);
  
  int start = 0;
  while (start < listStr.length()) {
    int commaPos = listStr.indexOf(',', start);
    if (commaPos < 0) commaPos = listStr.length();
    
    String token = listStr.substring(start, commaPos);
    token.trim();
    
    if (token == stationStr) return true;
    
    start = commaPos + 1;
  }
  return false;
}

void upload_file_to_ftp(const char* localPath, const char* remotePath) {
  // Use AT+CFTPSPUTFILE to upload
  // Implementation similar to existing FTP upload logic
}

void generate_diagnostics_report() {
  // Populate last_mission_details with diagnostics
  snprintf(last_mission_details, 63, 
    "Heap:%dKB,Up:%dh,Rst:%s", 
    ESP.getFreeHeap()/1024, 
    diag_total_uptime_hrs,
    reset_reason_str);
}
```

---

#### 3. `config_manager.ino` - NEW FILE (Configuration Handling)

**Purpose:** Load, save, and validate configuration settings

```cpp
#include "globals.h"

void load_config_from_nvs() {
  Preferences prefs;
  prefs.begin("sys-config", false);
  
  // Load all configurable settings
  debug_mode_enabled = prefs.getBool("debug_mode", false);
  mission_check_hour = prefs.getInt("mission_check_hour", 12);
  upload_interval_mins = prefs.getInt("upload_interval_mins", 15);
  RF_RESOLUTION = prefs.getFloat("rf_resolution", 0.25);
  backup_server_enabled = prefs.getBool("backup_server_enabled", false);
  
  String url = prefs.getString("backup_server_url", "");
  strncpy(backup_server_url, url.c_str(), 127);
  
  String mode = prefs.getString("backup_server_mode", "FAILOVER");
  strncpy(backup_server_mode, mode.c_str(), 15);
  
  prefs.end();
  
  debugln("[CONFIG] Loaded from NVS");
}

void save_config_to_nvs(const char* key, const char* value) {
  Preferences prefs;
  prefs.begin("sys-config", false);
  
  if (strcmp(key, "debug_mode") == 0) {
    prefs.putBool("debug_mode", atoi(value));
    debug_mode_enabled = atoi(value);
  }
  else if (strcmp(key, "mission_check_hour") == 0) {
    prefs.putInt("mission_check_hour", atoi(value));
    mission_check_hour = atoi(value);
  }
  else if (strcmp(key, "upload_interval_mins") == 0) {
    prefs.putInt("upload_interval_mins", atoi(value));
    upload_interval_mins = atoi(value);
  }
  else if (strcmp(key, "rf_resolution") == 0) {
    prefs.putFloat("rf_resolution", atof(value));
    RF_RESOLUTION = atof(value);
    // Also update SPIFFS for backward compatibility
    File f = SPIFFS.open("/rf_res.txt", FILE_WRITE);
    if (f) { f.print(value); f.close(); }
  }
  else if (strcmp(key, "station") == 0) {
    prefs.putString("station", value);
    strncpy(station_name, value, sizeof(station_name)-1);
    strncpy(ftp_station, value, sizeof(ftp_station)-1);
    // Also update SPIFFS
    File f = SPIFFS.open("/station.doc", FILE_WRITE);
    if (f) { f.print(value); f.close(); }
  }
  else if (strcmp(key, "backup_server_enabled") == 0) {
    prefs.putBool("backup_server_enabled", atoi(value));
    backup_server_enabled = atoi(value);
  }
  else if (strcmp(key, "backup_server_url") == 0) {
    prefs.putString("backup_server_url", value);
    strncpy(backup_server_url, value, 127);
  }
  else if (strcmp(key, "backup_server_mode") == 0) {
    prefs.putString("backup_server_mode", value);
    strncpy(backup_server_mode, value, 15);
  }
  
  prefs.end();
  debugln("[CONFIG] Saved to NVS");
}

bool validate_config_value(const char* key, const char* value) {
  if (strcmp(key, "debug_mode") == 0) {
    int v = atoi(value);
    return (v == 0 || v == 1);
  }
  else if (strcmp(key, "mission_check_hour") == 0) {
    int v = atoi(value);
    return (v >= 0 && v <= 23);
  }
  else if (strcmp(key, "upload_interval_mins") == 0) {
    int v = atoi(value);
    return (v == 15 || v == 30 || v == 60 || v == 120);
  }
  else if (strcmp(key, "rf_resolution") == 0) {
    float v = atof(value);
    return (fabs(v - 0.25) < 0.01 || fabs(v - 0.50) < 0.01);
  }
  else if (strcmp(key, "station") == 0) {
    return (strlen(value) > 0 && strlen(value) < 16);
  }
  else if (strcmp(key, "backup_server_enabled") == 0) {
    int v = atoi(value);
    return (v == 0 || v == 1);
  }
  else if (strcmp(key, "backup_server_url") == 0) {
    return (strlen(value) > 0 && strlen(value) < 128);
  }
  else if (strcmp(key, "backup_server_mode") == 0) {
    return (strcmp(value, "FAILOVER") == 0 || strcmp(value, "DUAL") == 0);
  }
  
  return false;
}
```

---

#### 4. `gprs.ino` - Modifications

**Location 1:** In `gprs()` task loop, around line 212-220, add mission check trigger:

```cpp
// Daily Mission Check
if (current_hour == mission_check_hour && current_min >= 30 && current_min <= 45) {
  static int last_mission_day = -1;
  if (current_day != last_mission_day) {
    debugln("[GPRS] Triggering Daily Mission Check");
    check_daily_mission();
    last_mission_day = current_day;
  }
}
```

**Location 2:** In `send_http_data()`, modify upload logic for batching:

```cpp
// Upload Interval Logic (before triggering HTTP)
static int upload_counter = 0;
int upload_skip_count = upload_interval_mins / 15; // e.g., 60/15 = 4

if (upload_counter % upload_skip_count == 0) {
  // Trigger upload
  sync_mode = eHttpTrigger;
} else {
  debugln("[GPRS] Skipping upload (batching mode)");
  // Data already saved to SPIFFS, will be sent as unsent data later
}
upload_counter++;
```

---

#### 5. `AIO9_5.0.ino` - Modifications

**In `setup()`, after loading station ID (around line 127):**

```cpp
// Load runtime configuration from NVS
load_config_from_nvs();
debug("Mission Check Hour: ");
debugln(mission_check_hour);
debug("Upload Interval: ");
debugln(upload_interval_mins);
```

---

## COMMAND REFERENCE

### 1. UPDATE_FIRMWARE

**Purpose:** Flash new firmware version

**Parameters:**
- `FILE` (required): Firmware binary filename on FTP server
- `VERSION` (required): New version number (e.g., 5.4)
- `TARGET_IDS` (optional): Comma-separated station IDs

**Example:**
```ini
MISSION_ID=FW_5.4_GLOBAL
COMMAND=UPDATE_FIRMWARE
FILE=firmware_v5.4.bin
VERSION=5.4
TARGET_IDS=SIT099,SIT100
```

**Behavior:**
- Checks if new version > current version
- Downloads firmware binary from `/missions/[CONFIG]/[FILE]`
- Flashes and restarts automatically
- Reports success/failure in next health report

**Status Codes:**
- `SUCCESS`: Updated successfully
- `SKIP_SAME_VER`: Already on this version
- `FAIL_DOWNLOAD`: FTP download failed
- `FAIL_BATTERY`: Battery too low (< 3.7V)

---

### 2. UPLOAD_LOG

**Purpose:** Upload a specific day's log file to FTP

**Parameters:**
- `DATE` (required): Date in YYYY-MM-DD format

**Example:**
```ini
MISSION_ID=RECOVER_FEB14
COMMAND=UPLOAD_LOG
DATE=2026-02-14
```

**Behavior:**
- Searches for `/[STATION]_YYYYMMDD.txt` in SPIFFS
- Uploads to FTP `/uploads/logs/[STATION]_YYYYMMDD.txt`
- Reports success/failure

**Status Codes:**
- `SUCCESS`: File uploaded
- `FAIL_NOT_FOUND`: Log file doesn't exist

---

### 3. GET_DIAGNOSTICS

**Purpose:** Generate system diagnostics report

**Parameters:** None

**Example:**
```ini
MISSION_ID=DIAG_FEB16
COMMAND=GET_DIAGNOSTICS
```

**Behavior:**
- Generates snapshot: Heap, Uptime, Reset Reasons, Signal History
- Includes in `Mission_Details` column of next health report

**Status Codes:**
- `SUCCESS`: Diagnostics generated

---

### 4. CONFIG_UPDATE

**Purpose:** Change runtime configuration setting

**Parameters:**
- `KEY` (required): Setting name
- `VALUE` (required): New value

**Example:**
```ini
MISSION_ID=ENABLE_DEBUG
COMMAND=CONFIG_UPDATE
KEY=debug_mode
VALUE=1
```

**Behavior:**
- Validates value
- Saves to NVS
- Restarts if needed (station, rf_resolution)
- Reports success/failure

**Status Codes:**
- `SUCCESS`: Config updated
- `FAIL_INVALID`: Invalid value for key

---

### 5. RESTART_SYSTEM

**Purpose:** Force system restart

**Parameters:** None

**Example:**
```ini
MISSION_ID=RESTART_FEB16
COMMAND=RESTART_SYSTEM
```

**Behavior:**
- Checks battery > 3.8V
- Restarts ESP32
- Reports success/failure in next health report

**Status Codes:**
- `SUCCESS`: Restarted
- `FAIL_LOW_BAT`: Battery too low

---

## CONFIGURATION SETTINGS

### Complete Settings Table

| # | Key | Type | Values | Default | Restart? | Description |
|---|-----|------|--------|---------|----------|-------------|
| 1 | `station` | String | Any (e.g., SIT099) | - | ✅ Yes | Station ID |
| 2 | `debug_mode` | Int | 0, 1 | 0 | ❌ No | Enable debug logging + FTP upload |
| 3 | `rf_resolution` | Float | 0.25, 0.50 | 0.25 | ✅ Yes | Rainfall bucket resolution (mm) |
| 4 | `mission_check_hour` | Int | 0-23 | 12 | ❌ No | Hour to check for missions (12 = 12:30 PM) |
| 5 | `upload_interval_mins` | Int | 15, 30, 60, 120 | 15 | ❌ No | Data upload frequency (minutes) |
| 6a | `backup_server_enabled` | Int | 0, 1 | 0 | ❌ No | Enable backup HTTP server |
| 6b | `backup_server_url` | String | URL | "" | ❌ No | Backup server URL |
| 6c | `backup_server_mode` | String | FAILOVER, DUAL | FAILOVER | ❌ No | Backup mode (failover or dual upload) |

### Upload Interval Power Savings

| Interval | Uploads/Day | Power Savings | Data Batching |
|----------|-------------|---------------|---------------|
| 15 min (default) | 96 | 0% (baseline) | 1 record/upload |
| 30 min | 48 | ~40% | 2 records/upload |
| 60 min | 24 | ~65% | 4 records/upload |
| 120 min | 12 | ~80% | 8 records/upload |

**Note:** Sensor readings still occur every 15 minutes. Only upload frequency changes.

---

## GOOGLE SHEETS INTEGRATION

### Sheet 1: "Health Status" (Current State)

**Existing Columns:**
| Timestamp | Station_ID | Battery | Signal | Version | ... |

**NEW Columns to Add:**
| ... | Last_Mission_ID | Mission_Status | Mission_Details |
|-----|-----------------|----------------|-----------------|
| ... | FW_5.4_GLOBAL | SUCCESS | v5.3→v5.4 |
| ... | RECOVER_FEB14 | SUCCESS | Uploaded log |
| ... | FW_5.4_GLOBAL | FAIL_BATTERY | Bat 3.5V < 3.7V |

### Sheet 2: "Mission History" (Audit Log - NEW)

**Columns:**
| Timestamp | Station_ID | Mission_ID | Command | Status | Details | Battery | Signal | Version_Before | Version_After |
|-----------|------------|------------|---------|--------|---------|---------|--------|----------------|---------------|
| 2026-02-16 12:35 | SIT099 | FW_5.4 | UPDATE_FIRMWARE | SUCCESS | v5.3→v5.4 | 4.1V | -73 | 5.3 | 5.4 |

### Firmware Changes for Reporting

**In `prepare_and_send_status()` function (gprs.ino):**

Add these fields to the HTTP payload:

```cpp
// Existing payload
// KSNDMC,TRG,STATUS-C,SIT099,...,CLB-OK,...

// NEW: Append mission status
snprintf(payload, sizeof(payload), 
  "%s,%s,%s,%s", 
  existing_payload,
  last_mission_command,      // "UPDATE_FIRMWARE"
  last_mission_status,       // "SUCCESS"
  last_mission_details);     // "v5.3->v5.4"
```

### Google Apps Script Modifications

**In `doPost(e)` function:**

```javascript
// Parse new fields
var missionCommand = params.mission_command || "NONE";
var missionStatus = params.mission_status || "IDLE";
var missionDetails = params.mission_details || "";

// Update Sheet 1 (Health Status)
healthSheet.getRange(row, lastCol+1).setValue(missionCommand);
healthSheet.getRange(row, lastCol+2).setValue(missionStatus);
healthSheet.getRange(row, lastCol+3).setValue(missionDetails);

// Append to Sheet 2 (Mission History) if not IDLE
if (missionStatus != "IDLE") {
  var historySheet = ss.getSheetByName("Mission History");
  historySheet.appendRow([
    new Date(),
    stationId,
    missionCommand,
    missionStatus,
    missionDetails,
    battery,
    signal,
    versionBefore,
    versionAfter
  ]);
}
```

---

## TESTING & VALIDATION

### Test Plan

#### Phase 1: Single Station Test

1. **Setup:**
   - Select test station (e.g., SIT099)
   - Ensure it has good battery (> 4.0V) and signal (> -85 dBm)

2. **Test 1: Simple Config Update**
   ```ini
   MISSION_ID=TEST_DEBUG
   COMMAND=CONFIG_UPDATE
   KEY=debug_mode
   VALUE=1
   ```
   - Upload to `/missions/KSNDMC_TRG/SIT099.txt`
   - Wait for 12:30 PM (or change `mission_check_hour` to current hour)
   - Verify in next health report: `Mission_Status=SUCCESS`

3. **Test 2: Upload Log**
   ```ini
   MISSION_ID=TEST_LOG
   COMMAND=UPLOAD_LOG
   DATE=2026-02-15
   ```
   - Verify file appears in `/uploads/logs/SIT099_20260215.txt`

4. **Test 3: Firmware Update**
   ```ini
   MISSION_ID=TEST_FW
   COMMAND=UPDATE_FIRMWARE
   FILE=firmware_v5.4.bin
   VERSION=5.4
   ```
   - Verify device restarts and reports new version

#### Phase 2: Group Test (5 Stations)

1. **Create global mission:**
   ```ini
   MISSION_ID=GROUP_TEST
   COMMAND=CONFIG_UPDATE
   KEY=upload_interval_mins
   VALUE=30
   TARGET_IDS=SIT099,SIT100,SIT101,SIT102,SIT103
   ```

2. **Verify:**
   - Only those 5 stations report `SUCCESS`
   - Other stations report `IDLE` or skip

#### Phase 3: Mass Update (All Stations)

1. **Create global mission (no TARGET_IDS):**
   ```ini
   MISSION_ID=MASS_FW_5.4
   COMMAND=UPDATE_FIRMWARE
   FILE=firmware_v5.4.bin
   VERSION=5.4
   ```

2. **Monitor Google Sheets:**
   - Track success rate
   - Identify failures (battery, signal, download errors)

### Validation Checklist

- [ ] Mission file downloads successfully
- [ ] TARGET_IDS filtering works correctly
- [ ] MISSION_ID prevents re-execution
- [ ] Config validation rejects invalid values
- [ ] Battery check prevents low-battery operations
- [ ] Results appear in Google Sheets
- [ ] Debug logs upload when debug_mode=1
- [ ] Upload interval batching works
- [ ] Firmware updates and restarts correctly
- [ ] Backup server failover works (if enabled)

---

## TROUBLESHOOTING

### Common Issues

#### Issue 1: Mission Not Executing

**Symptoms:** Device reports `IDLE`, no mission executed

**Possible Causes:**
1. Mission file not on FTP server
2. Wrong file path (check CONFIG name matches UNIT)
3. MISSION_ID already executed (check RTC memory)
4. TARGET_IDS doesn't include this station
5. Battery too low (< 3.7V)
6. GPRS connection failed

**Debug Steps:**
1. Enable `debug_mode=1` remotely
2. Check next day's debug log upload
3. Look for `[MISSION]` log entries
4. Verify FTP login success
5. Check file download errors

---

#### Issue 2: Config Update Not Applied

**Symptoms:** Setting changed but device behavior unchanged

**Possible Causes:**
1. Invalid value (validation failed)
2. Restart required but didn't happen (low battery)
3. NVS write failed

**Debug Steps:**
1. Check `Mission_Status` in Google Sheets
2. If `FAIL_INVALID`, check value format
3. If `SUCCESS` but no change, verify restart occurred
4. Check `load_config_from_nvs()` is called in setup()

---

#### Issue 3: Firmware Update Failed

**Symptoms:** `FAIL_DOWNLOAD` or device stuck in boot loop

**Possible Causes:**
1. Firmware binary corrupted
2. Wrong CONFIG (TRG binary sent to TWS device)
3. FTP download timeout
4. Insufficient SPIFFS space

**Debug Steps:**
1. Verify binary MD5 checksum
2. Check binary size < available SPIFFS
3. Test manual FTP download
4. Check `fetchFromFtpAndUpdate()` logs

---

#### Issue 4: Google Sheets Not Updating

**Symptoms:** Mission executed but not reported

**Possible Causes:**
1. HTTP upload failed
2. Google Apps Script error
3. RTC memory cleared (reset)

**Debug Steps:**
1. Check HTTP upload success in device logs
2. Check Google Apps Script execution log
3. Verify payload format matches script expectations

---

## APPENDIX

### Mission File Templates

#### Template 1: Mass Firmware Update
```ini
# Deploy v5.4 to all KSNDMC_TRG stations
MISSION_ID=FW_5.4_MASS_DEPLOY
COMMAND=UPDATE_FIRMWARE
FILE=firmware_v5.4.bin
VERSION=5.4
# No TARGET_IDS = ALL stations
```

#### Template 2: Pilot Group Update
```ini
# Test v5.5 on 10 pilot stations
MISSION_ID=FW_5.5_PILOT
COMMAND=UPDATE_FIRMWARE
FILE=firmware_v5.5_beta.bin
VERSION=5.5
TARGET_IDS=SIT001,SIT002,SIT003,SIT004,SIT005,SIT006,SIT007,SIT008,SIT009,SIT010
```

#### Template 3: Power Saving Mode
```ini
# Switch 50 low-battery stations to hourly upload
MISSION_ID=POWER_SAVE_BATCH1
COMMAND=CONFIG_UPDATE
KEY=upload_interval_mins
VALUE=60
TARGET_IDS=SIT050,SIT051,...,SIT099
```

#### Template 4: Debug Troublesome Station
```ini
# Enable debug mode for SIT099
MISSION_ID=DEBUG_SIT099
COMMAND=CONFIG_UPDATE
KEY=debug_mode
VALUE=1
```

#### Template 5: Recover Missing Data
```ini
# Get Feb 14 log from SIT099
MISSION_ID=RECOVER_FEB14_SIT099
COMMAND=UPLOAD_LOG
DATE=2026-02-14
```

---

### FTP Command Reference

**Login:**
```
AT+CFTPSLOGIN="dota.spatika.net",21,"dota_dmc","airdata2016",0
```

**Download File:**
```
AT+CFTPSGETFILE="/missions/KSNDMC_TRG/global_mission.txt",0
```

**Upload File:**
```
AT+CFTPSPUTFILE="/uploads/logs/SIT099_20260214.txt",0
```

**Logout:**
```
AT+CFTPSLOGOUT
```

---

### Version History

| Version | Date | Changes |
|---------|------|---------|
| 5.4 | 2026-02-16 | Initial Mission Control implementation |

---

**END OF SPECIFICATION**
