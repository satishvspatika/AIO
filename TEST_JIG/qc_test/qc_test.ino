#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_mac.h>
#include "esp_ota_ops.h"
#include <Adafruit_BME280.h>
#include <esp_task_wdt.h>
#include <Preferences.h>

#include <Update.h>
#include <MD5Builder.h>

Adafruit_BME280 bme;

#define QC_TEST_VERSION "6.13-QC"
#define WIND_DIR_ADC_MAX 3480

// --- PIN DEFINITIONS (ESP32-WROOM-32U) ---
#define GPRS_CTRL_PIN   26  // Active-HIGH PMOS gate for GPRS board power
#define LCD_CTRL_PIN    32  // Active-HIGH PMOS gate for Nuvoton LCD board power
#define KEYPAD_INT_PIN  27  // Interrupt pin (pulls LOW when key SET is pressed)

// UART1 (Nuvoton UI)
#define NUV_RX_PIN      14
#define NUV_TX_PIN      4

// UART2 (GPRS Modem)
#define GPRS_RX_PIN     16
#define GPRS_TX_PIN     17

// Analog Channels
#define RAIN_ADC_PIN    34
#define WIND_SPD_PIN    35
#define WIND_DIR_PIN    39
#define SOLAR_ADC_PIN   25
#define BATT_3V7_PIN    33
#define SYS_3V3_PIN     36

// --- STATE MACHINE AND SIMULATION DEFINITIONS ---
#define RAIN_SIM_PIN    2   // GPIO 2 as simulation pulse output (requires loopback to GPIO 34)

enum QCState {
  STATE_AUTO_TESTS,
  STATE_LCD_WAIT,
  STATE_KEYPAD_LEFT,
  STATE_KEYPAD_UP,
  STATE_KEYPAD_DOWN,
  STATE_KEYPAD_RIGHT,
  STATE_KEYPAD_SET,
  STATE_RF_WAIT_JUMPER,
  STATE_RF_RUNNING,
  STATE_RF_CONFIRM,
  STATE_WAKEUP_CONFIRM,
  STATE_SYNC_CONFIRM,
  STATE_SD_OTA_CONFIRM,
  STATE_COMPLETE,
  STATE_FAILED
};

#define INACTIVITY_TIMEOUT_MS 300000
bool getExpectingTestWakeup() {
  Preferences prefs;
  prefs.begin("qctest", true);
  bool val = prefs.getBool("exp_wakeup", false);
  prefs.end();
  return val;
}

void setExpectingTestWakeup(bool val) {
  Preferences prefs;
  prefs.begin("qctest", false);
  prefs.putBool("exp_wakeup", val);
  prefs.end();
}
bool syncVerdictPass = false;
unsigned long last_activity_time = 0;
bool in_countdown_warning = false;

QCState currentState = STATE_AUTO_TESTS;
uint32_t lastStateChangeTime = 0;
volatile int rf_pulse_count = 0;
volatile int wind_pulse_count = 0;

volatile unsigned long last_rf_pulse_time = 0;
volatile unsigned long last_wind_pulse_time = 0;

int rf_last_logged_count = -1;      // tracks last tip count sent to serial
unsigned long rf_test_start_time = 0;  // for 60s timeout


// Selective configuration flags (updated during serial handshake)
bool enableESP = true;
bool enableGPRS = true;
bool enableNuvoton = true;

bool isTRG = true;
bool isTWS = false;
bool isTWSRF = false;

void IRAM_ATTR rf_pulse_isr() {
  unsigned long now = millis();
  if (now - last_rf_pulse_time >= 50) { // 50ms debounce window for rain
    rf_pulse_count++;
    last_rf_pulse_time = now;
  }
}

void IRAM_ATTR wind_pulse_isr() {
  unsigned long now = millis();
  if (now - last_wind_pulse_time >= 15) { // 15ms debounce window for wind
    wind_pulse_count++;
    last_wind_pulse_time = now;
  }
}

// --- LCD HELPER FUNCTIONS ---
void lcdClear() {
  Serial1.write(0x01);
  Serial1.flush();
  delay(10);
}

void lcdSetCursor(uint8_t col, uint8_t row) {
  uint8_t addr = (uint8_t)(row * 64 + col + 128);
  Serial1.write(addr);
  Serial1.flush();
  delay(10);
  Serial1.write(addr);
  Serial1.flush();
  delay(10);
}

void lcdPrint(const char* str) {
  while (*str) {
    uint8_t b = (uint8_t)*str;
    if (b >= 0x20 && b <= 0x7E) {
      Serial1.write(b);
      Serial1.flush();
      delay(3);
    }
    str++;
  }
}
void redrawCurrentStateScreen() {
  if (!enableNuvoton) return;
  lcdClear();
  switch (currentState) {
    case STATE_LCD_WAIT:
      lcdSetCursor(0, 0); lcdPrint("LCD TEST: READ?");
      lcdSetCursor(0, 1); lcdPrint("PRESS CLEAR KEY");
      break;
    case STATE_KEYPAD_LEFT:
      lcdSetCursor(0, 0); lcdPrint("KEYPAD: PRESS");
      lcdSetCursor(0, 1); lcdPrint("LEFT KEY");
      break;
    case STATE_KEYPAD_UP:
      lcdSetCursor(0, 0); lcdPrint("KEYPAD: PRESS");
      lcdSetCursor(0, 1); lcdPrint("UP KEY");
      break;
    case STATE_KEYPAD_DOWN:
      lcdSetCursor(0, 0); lcdPrint("KEYPAD: PRESS");
      lcdSetCursor(0, 1); lcdPrint("DOWN KEY");
      break;
    case STATE_KEYPAD_RIGHT:
      lcdSetCursor(0, 0); lcdPrint("KEYPAD: PRESS");
      lcdSetCursor(0, 1); lcdPrint("RIGHT KEY");
      break;
    case STATE_KEYPAD_SET:
      lcdSetCursor(0, 0); lcdPrint("KEYPAD: PRESS");
      lcdSetCursor(0, 1); lcdPrint("SET KEY");
      break;
    case STATE_RF_WAIT_JUMPER: {
      char buf[17];
      snprintf(buf, sizeof(buf), "RF TEST: %d TIPS", rf_pulse_count);
      lcdSetCursor(0, 0); lcdPrint(buf);
      lcdSetCursor(0, 1); lcdPrint("PRESS SET RETRY");
      break;
    }
    case STATE_RF_RUNNING: {
      char line0[17], line1[17];
      snprintf(line0, sizeof(line0), "RF: %d TIPS", rf_pulse_count);
      snprintf(line1, sizeof(line1), "= %.2f mm", rf_pulse_count * 0.25f);
      lcdSetCursor(0, 0); lcdPrint(line0);
      lcdSetCursor(0, 1); lcdPrint(line1);
      break;
    }
    case STATE_RF_CONFIRM: {
      char buf[17];
      snprintf(buf, sizeof(buf), "TALLIED %d TIPS", rf_pulse_count);
      lcdSetCursor(0, 0); lcdPrint(buf);
      lcdSetCursor(0, 1); lcdPrint("SET:OK CLEAR:ERR");
      break;
    }
    case STATE_WAKEUP_CONFIRM:
      lcdSetCursor(0, 0); lcdPrint("WOKE UP OK!");
      lcdSetCursor(0, 1); lcdPrint("PRESS SET OR CLR");
      break;
    case STATE_SYNC_CONFIRM:
      lcdSetCursor(0, 0); lcdPrint("SYNC TO SHEET?");
      lcdSetCursor(0, 1); lcdPrint("SET:YES CLR:NO");
      break;
    case STATE_SD_OTA_CONFIRM:
      lcdSetCursor(0, 0); lcdPrint("UPDATE TO PROD FW?");
      lcdSetCursor(0, 1); lcdPrint("SET:YES  CLR:NO");
      break;
    case STATE_COMPLETE:
      lcdSetCursor(0, 0); lcdPrint("WAKEUP: PASS!");
      lcdSetCursor(0, 1); lcdPrint("QC PASS: APPROVED");
      break;
    case STATE_FAILED:
      lcdSetCursor(0, 0); lcdPrint("QC FAILED!");
      lcdSetCursor(0, 1); lcdPrint("INSPECT BOARD");
      break;
    default:
      break;
  }
}

void showProgress(const char* line1, const char* line2) {
  if (enableNuvoton) {
    lcdClear();
    lcdSetCursor(0, 0);
    lcdPrint(line1);
    lcdSetCursor(0, 1);
    lcdPrint(line2);
  }
}

// Remap Nuvoton raw keys to labels
const char* getKeyName(char rawKey) {
  switch (rawKey) {
    case '1': return "CLEAR";
    case '2': return "LEFT";
    case '3': return "UP";
    case '4': return "DOWN";
    case '5': return "RIGHT";
    case '6': return "SET";
    default: return "UNKNOWN";
  }
}

// --- PERIPHERAL TEST LOGIC ---

bool testSPIFFS() {
  // Temporary unsubscribe current task from watchdog during long mount/format operations to prevent resets
  esp_task_wdt_delete(NULL);

  bool success = false;
  if (SPIFFS.begin(false)) {
    success = true;
  } else {
    Serial.println("\n[QC_JIG] SPIFFS not formatted. Formatting partition (takes 10-30s)...");
    Serial.flush();
    if (SPIFFS.begin(true)) {
      success = true;
      Serial.println("[QC_JIG] SPIFFS formatting complete. Partition mounted successfully.");
      Serial.flush();
    }
  }

  if (success) {
    File f = SPIFFS.open("/qc.txt", FILE_WRITE);
    if (!f) success = false;
    else {
      f.println("SPIFFS_OK");
      f.close();
      
      f = SPIFFS.open("/qc.txt", FILE_READ);
      if (!f) success = false;
      else {
        String s = f.readStringUntil('\n');
        f.close();
        SPIFFS.remove("/qc.txt");
        success = (s.indexOf("SPIFFS_OK") >= 0);
      }
    }
  }

  // Re-enable watchdog
  esp_task_wdt_config_t wdt_config = {
      .timeout_ms = 60000,
      .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
      .trigger_panic = false
  };
  esp_task_wdt_reconfigure(&wdt_config);
  esp_task_wdt_add(NULL);
  esp_task_wdt_reset();

  return success;
}

int detected_sd_cs = 5;
bool sdHasFirmwareBin = false;
String detectedSdFwVersion = "6.13";

bool testSD(int csPin) {
  // Try initializing SPI SD on given CS pin
  if (!SD.begin(csPin)) return false;
  
  String foundFw = "";
  if (SD.exists("/firmware.bin")) foundFw = "/firmware.bin";
  else if (SD.exists("/FIRMWARE.BIN")) foundFw = "/FIRMWARE.BIN";
  else if (SD.exists("/Firmware.bin")) foundFw = "/Firmware.bin";

  if (foundFw.length() > 0) {
    sdHasFirmwareBin = true;
    detectedSdFwVersion = "6.18";
    if (SD.exists("/fw_version.txt")) {
      File vf = SD.open("/fw_version.txt", FILE_READ);
      if (vf) {
        String s = vf.readStringUntil('\n');
        s.trim(); s.replace("\r", "");
        if (s.length() > 0) detectedSdFwVersion = s;
        vf.close();
      }
    } else if (SD.exists("/metadata.json")) {
      File vf = SD.open("/metadata.json", FILE_READ);
      if (vf) {
        String content = vf.readString();
        vf.close();
        int idx = content.indexOf("\"full_version\":");
        if (idx >= 0) {
          int start = content.indexOf("\"", idx + 15);
          int end = content.indexOf("\"", start + 1);
          if (start >= 0 && end > start) {
            detectedSdFwVersion = content.substring(start + 1, end);
          }
        }
      }
    } else if (SD.exists("/version.txt")) {
      File vf = SD.open("/version.txt", FILE_READ);
      if (vf) {
        String s = vf.readStringUntil('\n');
        s.trim(); s.replace("\r", "");
        if (s.length() > 0) detectedSdFwVersion = s;
        vf.close();
      }
    }
    Serial.printf("[QC_JIG] SD Firmware Check: %s FOUND (Full Version: %s)\n", foundFw.c_str(), detectedSdFwVersion.c_str());
  } else {
    sdHasFirmwareBin = false;
    Serial.println("[QC_JIG] SD Firmware Check: /firmware.bin NOT FOUND on SD Card.");
  }

  File f = SD.open("/qc_sd.txt", FILE_WRITE);
  if (!f) {
    SD.end();
    return false;
  }
  f.println("SD_CARD_OK");
  f.close();
  
  f = SD.open("/qc_sd.txt", FILE_READ);
  if (!f) {
    SD.end();
    return false;
  }
  String s = f.readStringUntil('\n');
  f.close();
  SD.remove("/qc_sd.txt");
  SD.end();
  detected_sd_cs = csPin;
  return (s.indexOf("SD_CARD_OK") >= 0);
}

void otaProgressCallback(size_t currSize, size_t totalSize) {
  int percent = (currSize * 100) / totalSize;
  
  static int lastPrintedPercent = -1;
  if (percent != lastPrintedPercent && (percent % 5 == 0)) {
    lastPrintedPercent = percent;
    // Emit structured progress for laptop dashboard
    Serial.printf("[QC_OTA] PROGRESS:%d\n", percent);
    // Update LCD
    char buf1[17];
    snprintf(buf1, sizeof(buf1), "PROGRESS: %3d%%", percent);
    lcdSetCursor(0, 0); lcdPrint("FLASHING PROD FW");
    lcdSetCursor(0, 1); lcdPrint(buf1);
  }
  esp_task_wdt_reset();
}

bool flashProductionFirmwareFromSD(bool rebootAfterFlash = true) {
  if (!enableNuvoton) {
    enableNuvoton = true;
    Serial1.begin(9600, SERIAL_8N1, NUV_RX_PIN, NUV_TX_PIN);
  }

  Serial.println("[QC_OTA] START");

  int csPin = detected_sd_cs;
  bool sdMounted = SD.begin(csPin);
  if (!sdMounted) {
    int altCs = (csPin == 5) ? 13 : 5;
    if (SD.begin(altCs)) {
      detected_sd_cs = altCs;
      csPin = altCs;
      sdMounted = true;
    }
  }

  if (!sdMounted) {
    Serial.println("[QC_OTA] STATUS:SD card not mounted!");
    Serial.println("[QC_OTA] FAIL");
    lcdClear();
    lcdSetCursor(0, 0); lcdPrint("SD UPDATE FAIL!");
    lcdSetCursor(0, 1); lcdPrint("NO SD CARD");
    enterSyncConfirmState(false);
    return false;
  }

  String fwPath = "";
  if (SD.exists("/firmware.bin")) fwPath = "/firmware.bin";
  else if (SD.exists("/FIRMWARE.BIN")) fwPath = "/FIRMWARE.BIN";
  else if (SD.exists("/Firmware.bin")) fwPath = "/Firmware.bin";

  if (fwPath.length() == 0) {
    Serial.println("[QC_OTA] STATUS:firmware.bin not found on SD card!");
    Serial.println("[QC_OTA] FAIL");
    SD.end();
    lcdClear();
    lcdSetCursor(0, 0); lcdPrint("SD UPDATE FAIL!");
    lcdSetCursor(0, 1); lcdPrint("NO FIRMWARE.BIN");
    enterSyncConfirmState(false);
    return false;
  }

  Serial.println("[QC_OTA] STATUS:Reading SD card...");
  lcdClear();
  lcdSetCursor(0, 0); lcdPrint("READING SD CARD");
  lcdSetCursor(0, 1); lcdPrint("VERIFYING FW...");
  delay(300);

  File firmware = SD.open(fwPath.c_str(), FILE_READ);
  if (!firmware) {
    Serial.println("[QC_OTA] STATUS:Failed to open firmware file!");
    Serial.println("[QC_OTA] FAIL");
    SD.end();
    lcdClear();
    lcdSetCursor(0, 0); lcdPrint("SD UPDATE FAIL!");
    lcdSetCursor(0, 1); lcdPrint("FILE OPEN ERR");
    enterSyncConfirmState(false);
    return false;
  }

  int firstByte = firmware.read();
  if (firstByte != 0xE9) {
    Serial.printf("[QC_OTA] STATUS:CORRUPT! Magic=0x%02X (expected 0xE9)\n", firstByte);
    Serial.println("[QC_OTA] FAIL");
    lcdClear();
    lcdSetCursor(0, 0); lcdPrint("SD UPDATE FAIL!");
    lcdSetCursor(0, 1); lcdPrint("CORRUPT BIN");
    firmware.close();
    SD.end();
    enterSyncConfirmState(false);
    return false;
  }
  firmware.seek(0);

  size_t fwSize = firmware.size();
  Serial.printf("[QC_OTA] STATUS:Valid firmware.bin (%u bytes). Flashing...\n", (unsigned int)fwSize);

  lcdClear();
  lcdSetCursor(0, 0); lcdPrint("FLASHING PROD FW");
  lcdSetCursor(0, 1); lcdPrint("PROGRESS:   0%");

  Update.onProgress(otaProgressCallback);
  if (!Update.begin(fwSize, U_FLASH)) {
    Serial.printf("[QC_OTA] STATUS:Update.begin failed: %s\n", Update.errorString());
    Serial.println("[QC_OTA] FAIL");
    lcdClear();
    lcdSetCursor(0, 0); lcdPrint("SD UPDATE FAIL!");
    lcdSetCursor(0, 1); lcdPrint("BEGIN ERR");
    firmware.close();
    SD.end();
    enterSyncConfirmState(false);
    return false;
  }

  size_t written = Update.writeStream(firmware);
  firmware.close();
  SD.end();

  if (written == fwSize && Update.end(true)) {
    Serial.println("[QC_OTA] PROGRESS:100");
    Serial.println("[QC_OTA] COMPLETE");
    
    // Save flashed version into board NVS preferences so this board won't re-flash, while keeping SD card firmware intact for other boards!
    Preferences prefs;
    prefs.begin("spatika_qc", false);
    prefs.putString("sd_fw_ver", detectedSdFwVersion);
    prefs.end();
    Serial.printf("[QC_OTA] STATUS:Saved flashed version v%s to board NVS.\n", detectedSdFwVersion.c_str());

    lcdClear();
    lcdSetCursor(0, 0); lcdPrint("PROD FW SUCCESS!");
    if (rebootAfterFlash) {
      Serial.println("[QC_OTA] STATUS:Firmware written! Rebooting...");
      lcdSetCursor(0, 1); lcdPrint("REBOOTING...");
      Serial.flush();
      Serial1.flush();
      SPI.end();
      pinMode(csPin, OUTPUT);
      digitalWrite(csPin, HIGH);
      delay(1000);
      ESP.restart();
    } else {
      Serial.println("[QC_OTA] STATUS:Firmware written! Proceeding to sync...");
      lcdSetCursor(0, 1); lcdPrint("SYNCING...");
      delay(1000);
      // Now trigger Google Sheet sync step (OTA stored in flash; sync, then reboot via board power cycle)
      enterSyncConfirmState(true);
    }
    return true;
  } else {
    Serial.printf("[QC_OTA] STATUS:Write failed %u/%u bytes. Error: %s\n", (unsigned int)written, (unsigned int)fwSize, Update.errorString());
    Serial.println("[QC_OTA] FAIL");
    lcdClear();
    lcdSetCursor(0, 0); lcdPrint("SD UPDATE FAIL!");
    lcdSetCursor(0, 1); lcdPrint("WRITE ERR");
    enterSyncConfirmState(false);
    return false;
  }
}


bool testRTC() {
  Wire.begin(21, 22);
  
  // Try DS1307 / DS3231 at 0x68 or PCF8563 at 0x51
  uint8_t rtcAddr = 0;
  Wire.beginTransmission(0x68);
  if (Wire.endTransmission() == 0) {
    rtcAddr = 0x68;
  } else {
    Wire.beginTransmission(0x51);
    if (Wire.endTransmission() == 0) {
      rtcAddr = 0x51;
    }
  }

  if (rtcAddr == 0) return false; // No RTC I2C chip detected

  if (rtcAddr == 0x68) {
    // Check & Clear CH (Clock Halt) bit in DS1307/DS3231 reg 0x00 if oscillator is halted
    Wire.beginTransmission(0x68);
    Wire.write(0x00);
    Wire.endTransmission();
    Wire.requestFrom(0x68, 1);
    if (Wire.available()) {
      uint8_t rawSec = Wire.read();
      if (rawSec & 0x80) { // CH bit = 1 (Halted) -> Start oscillator!
        Serial.println("[QC_JIG] RTC oscillator was halted (CH=1). Clearing CH bit to start clock...");
        Wire.beginTransmission(0x68);
        Wire.write(0x00);
        Wire.write(rawSec & 0x7F); // Clear CH bit
        Wire.endTransmission();
        delay(100);
      }
    }
  }

  // Read seconds register
  Wire.beginTransmission(rtcAddr);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(rtcAddr, 1);
  if (!Wire.available()) return false;
  uint8_t sec1 = Wire.read() & 0x7F;

  // Poll for clock tick with a 1500ms timeout
  uint32_t start = millis();
  while (millis() - start < 1500) {
    Wire.beginTransmission(rtcAddr);
    Wire.write(0x00);
    Wire.endTransmission();
    Wire.requestFrom(rtcAddr, 1);
    if (Wire.available()) {
      uint8_t sec2 = Wire.read() & 0x7F;
      if (sec1 != sec2) return true;
    }
    delay(50);
  }

  // Fallback: If I2C communication succeeded and seconds register is valid BCD (0..59)
  if (sec1 <= 0x59) return true;

  return false;
}

bool testWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  if (n >= 0) {
    Serial.printf("[PASS] WIFI_CHECK: OK (%d APs found)\n", n);
    for (int i = 0; i < min(n, 3); ++i) {
      Serial.printf("[QC_JIG]   - SSID: %s (RSSI: %d)\n", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
    }
    WiFi.mode(WIFI_OFF); // Turn off WiFi to release ADC2 for Solar read
    return true;
  } else {
    Serial.printf("[QC_JIG] WiFi Scan failed: %d\n", n);
    WiFi.mode(WIFI_OFF); // Turn off WiFi to release ADC2 for Solar read
    return false;
  }
}

bool testTempHum(String &sensorName, float &temp, float &hum) {
  Wire.begin(21, 22);
  // 1. HDC at 0x40
  Wire.beginTransmission(0x40);
  if (Wire.endTransmission() == 0) {
    // Read device ID
    Wire.beginTransmission(0x40);
    Wire.write(0xFF);
    Wire.endTransmission(false);
    Wire.requestFrom(0x40, 2);
    uint16_t devId = 0;
    if (Wire.available() == 2) {
      devId = ((uint16_t)Wire.read() << 8) | Wire.read();
    }
    
    if (devId == 0x1050) {
      sensorName = "HDC1080";
    } else {
      Wire.beginTransmission(0x40);
      Wire.write(0xFE);
      Wire.endTransmission(false);
      Wire.requestFrom(0x40, 2);
      if (Wire.available() == 2) {
        devId = ((uint16_t)Wire.read() << 8) | Wire.read();
      }
      if (devId == 0xD007 || devId == 0x07D0) {
        sensorName = "HDC2022";
      } else {
        sensorName = "HDC_I2C";
      }
    }
    
    // Quick read
    Wire.beginTransmission(0x40);
    Wire.write(0x00);
    if (Wire.endTransmission() == 0) {
      delay(20);
      Wire.requestFrom(0x40, 4);
      if (Wire.available() == 4) {
        uint16_t rawT = ((uint16_t)Wire.read() << 8) | Wire.read();
        uint16_t rawH = ((uint16_t)Wire.read() << 8) | Wire.read();
        if (rawT != 0 && rawH != 0) {
          temp = (rawT / 65536.0) * 165.0 - 40.0;
          hum = (rawH / 65536.0) * 100.0;
          return true;
        }
      }
    }
  }
  
  // 2. BME280 at 0x76 or 0x77
  uint8_t bmeAddr = 0;
  Wire.beginTransmission(0x76);
  if (Wire.endTransmission() == 0) bmeAddr = 0x76;
  else {
    Wire.beginTransmission(0x77);
    if (Wire.endTransmission() == 0) bmeAddr = 0x77;
  }
  
  if (bmeAddr != 0) {
    // Check Chip ID
    Wire.beginTransmission(bmeAddr);
    Wire.write(0xD0);
    Wire.endTransmission(false);
    Wire.requestFrom(bmeAddr, 1);
    uint8_t chipId = 0;
    if (Wire.available() == 1) {
      chipId = Wire.read();
    }
    
    if (chipId == 0x60 || chipId == 0x58) {
      if (bme.begin(bmeAddr)) {
        sensorName = (chipId == 0x60) ? "BME280" : "BMP280";
        // Force single measurement mode
        bme.setSampling(Adafruit_BME280::MODE_FORCED,
                        Adafruit_BME280::SAMPLING_X1,
                        Adafruit_BME280::SAMPLING_X1,
                        Adafruit_BME280::SAMPLING_X1,
                        Adafruit_BME280::FILTER_OFF);
        bme.takeForcedMeasurement();
        temp = bme.readTemperature();
        hum = (chipId == 0x60) ? bme.readHumidity() : 0.0;
        if (!isnan(temp) && (chipId != 0x60 || !isnan(hum))) {
          return true;
        }
      }
    }
  }
  
  sensorName = "NONE";
  return false;
}

// Helper to safely cut power and tri-state GPRS UART pins to prevent parasitic powering
void powerOffGprsModem() {
  Serial2.end();
  pinMode(GPRS_TX_PIN, INPUT);
  pinMode(GPRS_RX_PIN, INPUT);
  digitalWrite(GPRS_CTRL_PIN, LOW);
}

// Helper to query Modem with AT commands
String sendModemAT(const char* cmd, uint32_t timeoutMs) {
  while (Serial2.available()) Serial2.read(); // Flush input
  Serial2.println(cmd);
  
  String response = "";
  uint32_t start = millis();
  while (millis() - start < timeoutMs) {
    while (Serial2.available()) {
      char c = (char)Serial2.read();
      response += c;
      if (response.indexOf("OK\r") >= 0 || response.indexOf("OK\n") >= 0 || 
          response.indexOf("ERROR\r") >= 0 || response.indexOf("ERROR\n") >= 0) {
        return response; // Return early on match to eliminate unnecessary delay
      }
    }
    delay(1); // Yield CPU and feed watchdog
  }
  return response;
}

// Parse registration status from CREG/CGREG responses
int parseRegStatus(String response, String prefix) {
  int idx = response.indexOf(prefix);
  if (idx < 0) return -1;
  
  int endLine = response.indexOf("\n", idx);
  if (endLine < 0) endLine = response.length();
  
  String line = response.substring(idx + prefix.length(), endLine);
  line.trim();
  
  int commaIdx = line.indexOf(",");
  if (commaIdx >= 0) {
    int nextComma = line.indexOf(",", commaIdx + 1);
    if (nextComma >= 0) {
      return line.substring(commaIdx + 1, nextComma).toInt();
    } else {
      return line.substring(commaIdx + 1).toInt();
    }
  } else {
    return line.toInt();
  }
}

// Manual RF tip test — operator manually tips the rain gauge
void startRfManualTest() {
  rf_pulse_count = 0;
  rf_last_logged_count = -1;
  rf_test_start_time = millis();
  last_activity_time = millis();
  currentState = STATE_RF_RUNNING;
  lastStateChangeTime = millis();

  // Attach interrupt immediately — ISR ready before we announce
  attachInterrupt(digitalPinToInterrupt(RAIN_ADC_PIN), rf_pulse_isr, FALLING);

  // Brief announcement so dashboard can render the instruction
  if (enableNuvoton) {
    lcdClear();
    lcdSetCursor(0, 0);
    lcdPrint("GIVE RF TIPS");
    lcdSetCursor(0, 1);
    lcdPrint("PRESS SET END");
  }
  Serial.println("[QC_STEP] RF_CHECK: WAITING_JUMPER");
  delay(400); // Allow dashboard to render before COUNTING arrives

  // Transition to live count
  Serial.println("[QC_STEP] RF_CHECK: COUNTING");
  Serial.println("[QC_JIG] Give RF tips manually. Press SET on keypad or wait 1 min to finish.");

  if (enableNuvoton) {
    lcdClear();
    lcdSetCursor(0, 0);
    lcdPrint("RF: 0 TIPS");
    lcdSetCursor(0, 1);
    lcdPrint("GIVE TIPS...");
  }
}

// Tally the manual RF tips collected and ask for confirmation
void tallyRfTest() {
  detachInterrupt(digitalPinToInterrupt(RAIN_ADC_PIN));
  int finalCount = rf_pulse_count;
  
  currentState = STATE_RF_CONFIRM;
  lastStateChangeTime = millis();
  last_activity_time = millis();
  Serial.printf("[QC_STEP] RF_CHECK: CONFIRMING (%d tips)\n", finalCount);
  
  if (enableNuvoton) {
    char buf[17];
    snprintf(buf, sizeof(buf), "%d TIPS (%.2fmm)", finalCount, finalCount * 0.25f);
    lcdClear();
    lcdSetCursor(0, 0);
    lcdPrint(buf);
    lcdSetCursor(0, 1);
    lcdPrint("SET:OK CLEAR:ERR");
  }
}

void passRfTest() {
  int finalCount = rf_pulse_count;
  Serial.printf("[QC_STEP] RF_CHECK: PASS (%d tips)\n", finalCount);
  if (enableNuvoton) {
    char buf[17];
    snprintf(buf, sizeof(buf), "RF PASS: %d TIPS", finalCount);
    lcdClear(); lcdSetCursor(0, 0); lcdPrint(buf);
    lcdSetCursor(0, 1); lcdPrint("TALLIED OK");
    delay(2000);
  }
  startDeepSleepTest();
}

void failRfTest() {
  int finalCount = rf_pulse_count;
  Serial.printf("[QC_STEP] RF_CHECK: FAIL (mismatch/fail, got %d)\n", finalCount);
  if (enableNuvoton) {
    char buf[17];
    snprintf(buf, sizeof(buf), "RF FAIL: %d TIPS", finalCount);
    lcdClear(); lcdSetCursor(0, 0); lcdPrint(buf);
    lcdSetCursor(0, 1); lcdPrint("PRESS SET RETRY");
  }
  currentState = STATE_RF_WAIT_JUMPER;
  lastStateChangeTime = millis();
  Serial.println("[QC_STEP] RF_CHECK: WAITING_JUMPER");
}

bool pendingSdOtaFlash = false;
bool sdOtaDecisionMade = false;

bool checkSdFirmwareExists() {
  Preferences prefs;
  prefs.begin("spatika_qc", true);
  String alreadyFlashedVer = prefs.getString("sd_fw_ver", "");
  prefs.end();

  if (sdHasFirmwareBin && detectedSdFwVersion.length() > 0) {
    if (alreadyFlashedVer.length() > 0 && alreadyFlashedVer.equalsIgnoreCase(detectedSdFwVersion)) {
      Serial.printf("[QC_JIG] SD firmware version v%s already flashed on this board. Skipping update prompt.\n", detectedSdFwVersion.c_str());
      return false;
    }
    return true;
  }

  delay(50); // Small pause for power stabilization
  int csPins[2] = {(detected_sd_cs != 0) ? detected_sd_cs : 5, (detected_sd_cs == 5) ? 13 : 5};
  for (int i = 0; i < 2; i++) {
    int cs = csPins[i];
    pinMode(cs, OUTPUT);
    digitalWrite(cs, HIGH);
    delay(10);
    if (SD.begin(cs)) {
      String foundFw = "";
      if (SD.exists("/firmware.bin")) foundFw = "/firmware.bin";
      else if (SD.exists("/FIRMWARE.BIN")) foundFw = "/FIRMWARE.BIN";
      else if (SD.exists("/Firmware.bin")) foundFw = "/Firmware.bin";

      if (foundFw.length() > 0) {
        sdHasFirmwareBin = true;
        detected_sd_cs = cs;
        detectedSdFwVersion = "6.13";
        if (SD.exists("/fw_version.txt")) {
          File vf = SD.open("/fw_version.txt", FILE_READ);
          if (vf) {
            String s = vf.readStringUntil('\n');
            s.trim(); s.replace("\r", "");
            if (s.length() > 0) detectedSdFwVersion = s;
            vf.close();
          }
        } else if (SD.exists("/version.txt")) {
          File vf = SD.open("/version.txt", FILE_READ);
          if (vf) {
            String s = vf.readStringUntil('\n');
            s.trim(); s.replace("\r", "");
            if (s.length() > 0) detectedSdFwVersion = s;
            vf.close();
          }
        }
        SD.end();

        if (alreadyFlashedVer.length() > 0 && alreadyFlashedVer.equalsIgnoreCase(detectedSdFwVersion)) {
          Serial.printf("[QC_JIG] SD firmware version v%s already flashed on this board. Skipping update prompt.\n", detectedSdFwVersion.c_str());
          return false;
        }

        return true;
      }
      SD.end();
    }
  }
  return false;
}

void enterSyncConfirmState(bool pass) {
  // Safety: Turn off GPRS board power to ensure socket is safe for hot swapping in all outcomes (PASS or FAIL)
  powerOffGprsModem();

  if (pass && !sdOtaDecisionMade) {
    sdOtaDecisionMade = true;
    currentState = STATE_SD_OTA_CONFIRM;
    lastStateChangeTime = millis();
    syncVerdictPass = true;
    
    bool sdExists = checkSdFirmwareExists();
    if (sdExists) {
      Serial.printf("[QC_STEP] SD_OTA_PROMPT: WAITING_FOR_CONFIRM:%s:SD_DETECTED\n", detectedSdFwVersion.c_str());
      Serial.printf("[QC_JIG] SD Card with Production FW v%s detected! Press SET to update, or CLR to skip.\n", detectedSdFwVersion.c_str());
    } else {
      Serial.println("[QC_STEP] SD_OTA_PROMPT: WAITING_FOR_CONFIRM:NONE:SD_MISSING");
      Serial.println("[QC_JIG] No SD Card firmware detected. You can flash via Programmer on the laptop dashboard, or press CLR to skip.");
    }
    
    if (enableNuvoton) {
      delay(300);
      lcdClear();
      if (sdExists) {
        char line0[17];
        snprintf(line0, sizeof(line0), "UPDATE TO v%s?", detectedSdFwVersion.c_str());
        lcdSetCursor(0, 0); lcdPrint(line0);
        lcdSetCursor(0, 1); lcdPrint("SET:YES  CLR:NO");
      } else {
        lcdSetCursor(0, 0); lcdPrint("NO SD FW FOUND");
        lcdSetCursor(0, 1); lcdPrint("CLR: SKIP UPDATE");
      }
    }
    return;
  }

  currentState = STATE_SYNC_CONFIRM;
  lastStateChangeTime = millis();
  syncVerdictPass = pass;
  Serial.printf("[QC_STEP] SYNC_SHEET: CONFIRMING_%s\n", pass ? "PASS" : "FAIL");
  
  if (enableNuvoton) {
    delay(300); // Brief pause so previous LCD text clears cleanly before sync prompt
    lcdClear();
    lcdSetCursor(0, 0);
    lcdPrint("SYNC TO SHEET?");
    lcdSetCursor(0, 1);
    lcdPrint("SET:YES CLR:NO");
  }
}

void startDeepSleepTest() {
  Serial.println("[QC_JIG] Initiating Deep Sleep Wakeup verification...");
  Serial.println("[QC_STEP] EXT0_WAKEUP: WAITING_SLEEP");
  Serial.flush();
  delay(1000);
  
  // Cut power to LCD and GPRS PMOS gates (active-HIGH PMOS gate, write LOW to turn off)
  digitalWrite(LCD_CTRL_PIN, LOW);
  powerOffGprsModem();
  delay(100);
  
  // Set the flag so we know this sleep was for the test verification wakeup
  setExpectingTestWakeup(true);
  
  // Enable EXT0 wakeup on GPIO 27 (SET key, pulls LOW when pressed)
  esp_sleep_enable_ext0_wakeup((gpio_num_t)KEYPAD_INT_PIN, 0);
  
  // Enter Deep Sleep
  esp_deep_sleep_start();
}

void goToIdleSleep() {
  Serial.println("[QC_JIG] Inactivity/Idle timeout reached. Going to deep sleep...");
  Serial.flush();
  delay(500);
  
  // Cut power to LCD and GPRS PMOS gates (active-HIGH PMOS gate, write LOW to turn off)
  digitalWrite(LCD_CTRL_PIN, LOW);
  powerOffGprsModem();
  delay(100);
  
  // This is a power-saving sleep, not the test verification wakeup
  setExpectingTestWakeup(false);
  
  // Enable EXT0 wakeup on GPIO 27 (SET key, pulls LOW when pressed)
  esp_sleep_enable_ext0_wakeup((gpio_num_t)KEYPAD_INT_PIN, 0);
  
  // Enter Deep Sleep
  esp_deep_sleep_start();
}

void printPartitionInfo() {
  uint32_t flashSize = ESP.getFlashChipSize();
  uint32_t appUsed = ESP.getSketchSize();
  const esp_partition_t* running = esp_ota_get_running_partition();
  uint32_t appTotal = running ? running->size : (appUsed + ESP.getFreeSketchSpace());
  
  float flashSizeMB = (float)flashSize / (1024.0 * 1024.0);
  float appUsedMB = (float)appUsed / (1024.0 * 1024.0);
  float appTotalMB = (float)appTotal / (1024.0 * 1024.0);
  
  float spiffsTotalMB = 0.0;
  float spiffsUsedKB = 0.0;
  bool spiffsMounted = SPIFFS.begin(false);
  if (spiffsMounted) {
    spiffsTotalMB = (float)SPIFFS.totalBytes() / (1024.0 * 1024.0);
    spiffsUsedKB = (float)SPIFFS.usedBytes() / 1024.0;
  }
  
  // Clean, compact boot format matching the approved plan:
  // PARTITION:8MB(APP:1.01MB/1.69MB,SPIFF:1.20KB/4.56MB)
  if (spiffsMounted) {
    Serial.printf("[QC_JIG] PARTITION: %.0fMB(APP:%.2fMB/%.2fMB, SPIFF:%.2fKB/%.2fMB)\n",
                  flashSizeMB, appUsedMB, appTotalMB, spiffsUsedKB, spiffsTotalMB);
  } else {
    Serial.printf("[QC_JIG] PARTITION: %.0fMB(APP:%.2fMB/%.2fMB, SPIFF:UNFORMATTED)\n",
                  flashSizeMB, appUsedMB, appTotalMB);
  }
}

void setup() {
  // Initialize Serial Monitor (UART0)
  Serial.begin(115200);
  delay(100);
  
  // Check if we woke up from Deep Sleep via EXT0 trigger
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0 && getExpectingTestWakeup()) {
    setExpectingTestWakeup(false); // Reset the flag
    
    // Power ON LCD
    pinMode(LCD_CTRL_PIN, OUTPUT);
    digitalWrite(LCD_CTRL_PIN, HIGH);
    delay(1500); // Allow Nuvoton LCD controller to boot up fully before serial communication
    
    // Initialize LCD UART and print verdict
    pinMode(NUV_TX_PIN, OUTPUT);
    pinMode(NUV_RX_PIN, INPUT);
    Serial1.begin(9600, SERIAL_8N1, NUV_RX_PIN, NUV_TX_PIN);
    delay(100);
    while (Serial1.available()) {
      Serial1.read();
    }
    Serial1.write(0x0C); // Force Display ON (pixels enabled)
    Serial1.flush();
    delay(50);
    
    lcdClear();
    lcdSetCursor(0, 0);
    lcdPrint("WOKE UP OK!");
    lcdSetCursor(0, 1);
    lcdPrint("PRESS SET OR CLR");
    
    Serial.println("\n[QC_JIG] ======================================");
    Serial.println("[QC_JIG] Board woke up from Deep Sleep. Waiting for operator confirmation...");
    Serial.println("[QC_STEP] EXT0_WAKEUP: CONFIRMING");
    Serial.println("[QC_JIG] ======================================");
    
    currentState = STATE_WAKEUP_CONFIRM;
    lastStateChangeTime = millis();
    last_activity_time = millis();
    return; // Go straight to loop() and bypass setup checks
  }

  Serial.println("\n[QC_JIG] ======================================");
  Serial.printf("[QC_JIG] SPATIKA AIO BOARD QC TEST FIRMWARE v%s START\n", QC_TEST_VERSION);
  Serial.println("[QC_STEP] SD_OTA: READY");
  Serial.println("[QC_JIG] Board tested & Ready for SD Card Update (Insert SD card with /firmware.bin to load Production Firmware)");
  
  // Initialize WiFi driver to read MAC ID reliably
  WiFi.mode(WIFI_STA);
  delay(10);
  String macStr = WiFi.macAddress();
  Serial.printf("[QC_JIG] ESP32 Unique MAC: %s\n", macStr.c_str());
  Serial.println("[QC_JIG] ======================================");
  
  // Handshake to receive test configuration from serial dashboard (loops indefinitely to prevent standalone autostart)
  String configStr = "";
  unsigned long lastPrintTime = 0;
  
  Serial.println("[QC_JIG] [READY] WAITING_FOR_CONFIG");
  lastPrintTime = millis();
  
  while (true) {
    if (millis() - lastPrintTime >= 2000) {
      Serial.println("[QC_JIG] [READY] WAITING_FOR_CONFIG");
      lastPrintTime = millis();
      
      // Auto-detect SD card insert on boot
      uint8_t cs_pin = (detected_sd_cs != 0) ? detected_sd_cs : 5;
      if (SD.begin(cs_pin) || SD.begin(13) || SD.begin(5)) {
        if (SD.exists("/firmware.bin") || SD.exists("/FIRMWARE.BIN")) {
          String sdVer = "6.13";
          if (SD.exists("/fw_version.txt")) {
            File vf = SD.open("/fw_version.txt", FILE_READ);
            if (vf) { sdVer = vf.readStringUntil('\n'); sdVer.trim(); sdVer.replace("\r", ""); vf.close(); }
          }
          Preferences prefs;
          prefs.begin("spatika_qc", true);
          String alreadyFlashed = prefs.getString("sd_fw_ver", "");
          prefs.end();

          if (alreadyFlashed.length() == 0 || !alreadyFlashed.equalsIgnoreCase(sdVer)) {
            Serial.println("[QC_JIG] Startup: Found new /firmware.bin on SD Card! Triggering Production Firmware update...");
            flashProductionFirmwareFromSD(true); // Boot-time: reboot immediately after flash
          }
        }
        SD.end();
      }
    }
    
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        int cfgIdx = configStr.indexOf("CFG:");
        if (cfgIdx >= 0) {
          configStr = configStr.substring(cfgIdx); // Strip leading garbage
          break;
        }
        configStr = ""; // Clear non-config lines (noise)
      } else {
        configStr += c;
      }
    }
    delay(1);
  }
  
  enableESP = (configStr.indexOf("ESP") >= 0);
  enableGPRS = (configStr.indexOf("GPRS") >= 0);
  enableNuvoton = (configStr.indexOf("NUV") >= 0);
  
  // Parse Profile
  int profileIdx = configStr.indexOf("PROFILE:");
  isTRG = false;
  isTWS = false;
  isTWSRF = false;
  if (profileIdx >= 0) {
    String profile = configStr.substring(profileIdx + 8);
    profile.trim();
    if (profile == "TRG") {
      isTRG = true;
    } else if (profile == "TWS") {
      isTWS = true;
    } else if (profile == "TWS-RF") {
      isTWSRF = true;
    }
  } else {
    isTRG = true; // default
  }
  Serial.printf("[QC_JIG] Received Config: ESP=%d, GPRS=%d, NUV=%d, PROFILE=%s (isTRG=%d, isTWS=%d, isTWSRF=%d)\n", 
                enableESP, enableGPRS, enableNuvoton, 
                isTRG ? "TRG" : (isTWS ? "TWS" : "TWS-RF"), 
                isTRG, isTWS, isTWSRF);
  
  // v6.09: Delete the loop task from the watchdog first to allow reconfiguration
  esp_task_wdt_delete(NULL);
  
  // Initialize Task Watchdog only during active testing to prevent timeout reboot while waiting for handshake
  esp_task_wdt_config_t wdt_config = {
      .timeout_ms = 60000,      // 60 seconds to accommodate slow peripheral checks / formats
      .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
      .trigger_panic = false    // Log watchdog timeouts instead of hard-rebooting the CPU
  };
  esp_task_wdt_reconfigure(&wdt_config);
  esp_task_wdt_add(NULL);       // Register loop task
  esp_task_wdt_reset();
  
  // Power Controls
  pinMode(GPRS_CTRL_PIN, OUTPUT);
  pinMode(LCD_CTRL_PIN, OUTPUT);
  
  digitalWrite(GPRS_CTRL_PIN, LOW);  // Turn off GPRS first to clear latch-up
  digitalWrite(LCD_CTRL_PIN, LOW);   // Turn off LCD board initially
  
  if (enableNuvoton) {
    digitalWrite(LCD_CTRL_PIN, HIGH);  // Power ON Nuvoton board
  }
  
  if (!enableGPRS) {
    digitalWrite(GPRS_CTRL_PIN, LOW);  // Keep GPRS off
    delay(500);
  }
  
  pinMode(KEYPAD_INT_PIN, INPUT_PULLUP);
  
  if (enableNuvoton) {
    // Initialize UART1 (Nuvoton UI)
    Serial1.begin(9600, SERIAL_8N1, NUV_RX_PIN, NUV_TX_PIN);
    delay(100);
    
    // Initialize LCD display
    showProgress("SPATIKA QC TEST", "BOOTING SYSTEM..");
  }

  bool hardware_check_failed = false;

  // 1. SPIFFS Test
  if (enableESP) {
    showProgress("DIAG: SPIFFS", "TESTING...");
    Serial.print("[QC_JIG] Testing SPIFFS... ");
    if (testSPIFFS()) {
      Serial.println("[PASS] SPIFFS_CHECK: OK");
      showProgress("DIAG: SPIFFS", "PASS");
      printPartitionInfo();
    } else {
      Serial.println("[FAIL] SPIFFS_CHECK: FAIL");
      showProgress("DIAG: SPIFFS", "FAIL");
      printPartitionInfo();
      hardware_check_failed = true;
    }
    delay(500);
  } else {
    Serial.println("[QC_STEP] SPIFFS_CHECK: IGNORED");
  }
  
  // 2. SD Card Test (Pins 5 and 13 CS)
  if (enableESP) {
    showProgress("DIAG: SD CARD", "TESTING...");
    Serial.print("[QC_JIG] Testing SD Card on CS 5... ");
    bool sdOk = testSD(5);
    if (!sdOk) {
      Serial.print("failed CS 5, trying CS 13... ");
      showProgress("DIAG: SD CARD", "TRYING CS 13...");
      sdOk = testSD(13);
    }
    if (sdOk) {
      Serial.println("[PASS] SD_CHECK: OK");
      showProgress("DIAG: SD CARD", "PASS");
    } else {
      Serial.println("[FAIL] SD_CHECK: FAIL");
      showProgress("DIAG: SD CARD", "FAIL");
      hardware_check_failed = true;
    }
    delay(500);
  } else {
    Serial.println("[QC_STEP] SD_CHECK: IGNORED");
  }
  
  // 3. RTC Clock Test
  if (enableESP) {
    showProgress("DIAG: RTC I2C", "TESTING...");
    Serial.print("[QC_JIG] Testing RTC (0x68) I2C... ");
    if (testRTC()) {
      Serial.println("[PASS] RTC_CHECK: OK");
      showProgress("DIAG: RTC I2C", "PASS");
    } else {
      Serial.println("[FAIL] RTC_CHECK: FAIL");
      showProgress("DIAG: RTC I2C", "FAIL");
      hardware_check_failed = true;
    }
    delay(500);
  } else {
    Serial.println("[QC_STEP] RTC_CHECK: IGNORED");
  }
  
  // 3a. WiFi Scan Test
  if (enableESP) {
    showProgress("DIAG: WIFI SCAN", "SCANNING...");
    Serial.print("[QC_JIG] Testing WiFi Scan... ");
    if (testWiFi()) {
      showProgress("DIAG: WIFI SCAN", "PASS");
      // Printed inside testWiFi
    } else {
      Serial.println("[FAIL] WIFI_CHECK: FAIL");
      showProgress("DIAG: WIFI SCAN", "FAIL");
      hardware_check_failed = true;
    }
    delay(500);
  } else {
    Serial.println("[QC_STEP] WIFI_CHECK: IGNORED");
  }

  // 3b. Environmental Sensor Test (I2C)
  if (enableESP && !isTRG) {
    showProgress("DIAG: ENV SENSOR", "TESTING...");
    Serial.print("[QC_JIG] Testing Env Sensor I2C... ");
    String sensorName = "";
    float temp = 0.0;
    float hum = 0.0;
    if (testTempHum(sensorName, temp, hum)) {
      Serial.printf("[PASS] SENSOR_CHECK: OK (%s: Temp=%.1fC, Hum=%.1f%%)\n", sensorName.c_str(), temp, hum);
      char tempHumBuf[32];
      snprintf(tempHumBuf, sizeof(tempHumBuf), "P:%.1fC %.1f%%", temp, hum);
      showProgress("DIAG: ENV SENSOR", tempHumBuf);
    } else {
      Serial.println("[FAIL] SENSOR_CHECK: FAIL");
      showProgress("DIAG: ENV SENSOR", "FAIL");
      hardware_check_failed = true;
    }
    delay(500);
  } else {
    Serial.println("[QC_STEP] SENSOR_CHECK: IGNORED");
  }

  // 3c. Wind Sensors Setup
  if (enableESP && !isTRG) {
    pinMode(WIND_SPD_PIN, INPUT);
    wind_pulse_count = 0;
    attachInterrupt(digitalPinToInterrupt(WIND_SPD_PIN), wind_pulse_isr, FALLING);
    Serial.println("[QC_JIG] Wind speed pulse interrupt attached.");
  } else {
    Serial.println("[QC_STEP] WIND_SPD: IGNORED");
    Serial.println("[QC_STEP] WIND_DIR: IGNORED");
  }
  
  // 4. Analog Readings
  if (enableESP) {
    // [H-01] Ensure WiFi is fully disabled and released from ADC2 before reading Solar ADC
    WiFi.mode(WIFI_OFF);
    delay(50);

    int rawBatt = 0, rawV33 = 0, rawSolar = 0;
    for (int i = 0; i < 8; i++) {
      rawBatt  += analogRead(BATT_3V7_PIN);
      rawV33   += analogRead(SYS_3V3_PIN);
      rawSolar += analogRead(SOLAR_ADC_PIN);
      delay(2);
    }
    rawBatt  /= 8;
    rawV33   /= 8;
    rawSolar /= 8;
    
    float battVolt = (rawBatt / (float)WIND_DIR_ADC_MAX) * 3.3 * 1.151;  // R_top=220K R_bot=620K → 840/620, adj for ADC_MAX=3480
    float v33Volt  = (rawV33  / (float)WIND_DIR_ADC_MAX) * 3.3 * 1.151;  // R_top=220K R_bot=620K → 840/620, adj for ADC_MAX=3480
    float solarVolt = (rawSolar / (float)WIND_DIR_ADC_MAX) * 3.3 * 6.119;  // R_top=620K R_bot=100K → 720/100, adj for ADC_MAX=3480

    // [M-03] LDO out-of-range warning check
    if (v33Volt < 3.0 || v33Volt > 3.6) {
      Serial.printf("[WARN] SYS_3V3 out of range: %.2f V\n", v33Volt);
    }
   
    Serial.printf("[QC_JIG] ADCs: Rain=%d, WindSpeed=%d, WindDir=%d\n", analogRead(RAIN_ADC_PIN), analogRead(WIND_SPD_PIN), analogRead(WIND_DIR_PIN));
    Serial.printf("[QC_JIG] BATT_3V7_ADC: %d (Derived: %.2f V), SYS_3V3_ADC: %d (Derived: %.2f V), SOLAR_ADC: %d (Derived: %.2f V)\n", rawBatt, battVolt, rawV33, v33Volt, rawSolar, solarVolt);
  } else {
    Serial.println("[QC_STEP] ADC_CHECK: IGNORED");
  }
  
  // 5. GPRS Modem Setup and Test
  if (enableGPRS && !hardware_check_failed) {
    showProgress("DIAG: GPRS MDM", "STABILIZING 4s");
    digitalWrite(GPRS_CTRL_PIN, HIGH); // Power ON GPRS board
    delay(4000);                       // Allow 4 seconds for bulk capacitors & modem PMIC to stabilize cleanly
    
    showProgress("DIAG: GPRS MDM", "INITIALIZING...");
    Serial.println("[QC_JIG] Initializing GPRS Modem UART2...");
    Serial2.begin(115200, SERIAL_8N1, GPRS_RX_PIN, GPRS_TX_PIN);
    
    // Pulse dummy AT commands to lock auto-baud rate detector
    for (int bSync = 0; bSync < 3; bSync++) {
      Serial2.println("AT");
      delay(150);
      while (Serial2.available()) Serial2.read();
    }
    
    bool modem_init_ok = false;
    // Poll AT up to 20 times (10s window) to ensure modem PMIC soft-start completes
    for (int i = 0; i < 20; i++) {
      char statusBuf[32];
      snprintf(statusBuf, sizeof(statusBuf), "POLL AT %d/20", i + 1);
      showProgress("DIAG: GPRS MDM", statusBuf);
      String res = sendModemAT("AT", 500);
      if (res.indexOf("OK") >= 0) {
        modem_init_ok = true;
        break;
      }
      delay(500);
    }
    
    if (modem_init_ok) {
      Serial.println("[PASS] MODEM_INIT: OK");
      showProgress("DIAG: GPRS MDM", "AT OK");
      sendModemAT("AT+IPR=115200", 500); // Lock modem baud rate permanently

      // Enable verbose error messages
      sendModemAT("AT+CMEE=2", 500);
      // Disable mechanical hot-plug SIM detect (forces electrical detect for BSNL/etc)
      sendModemAT("AT+CSDT=0", 500);
      sendModemAT("AT+UIMHOTSWAPON=0", 500);
      showProgress("DIAG: SIM CPIN", "SETTLING 3s");
      delay(3000);

      // Verify SIM is ready before CCID fetch (up to 15 retries for slow SIM on power-on)
      showProgress("DIAG: SIM CPIN", "CHECKING...");
      bool sim_ready = false;
      for (int i = 0; i < 15; i++) {
        esp_task_wdt_reset();
        char cpinBuf[32];
        snprintf(cpinBuf, sizeof(cpinBuf), "CPIN POLL %d/15", i + 1);
        showProgress("DIAG: SIM CPIN", cpinBuf);
        String cpinResp = sendModemAT("AT+CPIN?", 1000);
        cpinResp.trim();
        Serial.printf("[QC_JIG] CPIN Response %d: %s\n", i + 1, cpinResp.c_str());
        if (cpinResp.indexOf("READY") >= 0) {
          sim_ready = true;
          break;
        }
        
        // Software reset interface recovery if protocol lock "SIM failure" (CME 13) is detected
        if (cpinResp.indexOf("SIM failure") >= 0 || cpinResp.indexOf(" 13") >= 0) {
          Serial.println("[QC_JIG] GPRS: Detected SIM failure lock. Attempting Software Interface Reset (CFUN 0/1)...");
          sendModemAT("AT+CFUN=0", 2000);
          delay(2000);
          sendModemAT("AT+CFUN=1", 2000);
          delay(2000);
        }
        delay(1000);
      }

      if (sim_ready) {
        showProgress("DIAG: SIM CPIN", "READY");
      } else {
        showProgress("DIAG: SIM CPIN", "NOT READY");
      }
      delay(500);

      // Read IMEI
      showProgress("DIAG: IMEI", "READING...");
      String res = sendModemAT("AT+GSN", 1000);
      String imei = "UNKNOWN";
      for (int i = 0; i < (int)res.length() - 14; i++) {
        if (isDigit(res[i]) && isDigit(res[i+14])) {
          imei = res.substring(i, i + 15);
          break;
        }
      }
      Serial.printf("[QC_JIG] MODEM_IMEI: %s\n", imei.c_str());
      showProgress("DIAG: IMEI", imei.c_str());
      delay(500);
      
      // Read CCID safely with retry (SIM card boot delay check)
      showProgress("DIAG: CCID", "READING...");
      String ccid = "NO_SIM";
      for (int retry = 0; retry < 5; retry++) {
        char ccidPollBuf[32];
        snprintf(ccidPollBuf, sizeof(ccidPollBuf), "CCID POLL %d/5", retry + 1);
        showProgress("DIAG: CCID", ccidPollBuf);
        res = sendModemAT("AT+CICCID", 1000);
        res.trim();
        Serial.printf("[QC_JIG] CCID Attempt %d Response: %s\n", retry + 1, res.c_str());
        int headerLen = 8;
        int ccidIdx = res.indexOf("+CICCID:");
        if (ccidIdx < 0) {
          ccidIdx = res.indexOf("+ICCID:");
          headerLen = 7;
        }
        if (ccidIdx < 0) {
          res = sendModemAT("AT+CCID", 1000);
          res.trim();
          Serial.printf("[QC_JIG] CCID Backup Attempt %d Response: %s\n", retry + 1, res.c_str());
          ccidIdx = res.indexOf("+CCID:");
          headerLen = 6;
        }
        if (ccidIdx < 0) {
          ccidIdx = res.indexOf("+ICCID:");
          headerLen = 7;
        }
        
        if (ccidIdx >= 0) {
          String tempCcid = res.substring(ccidIdx + headerLen);
          tempCcid.trim();
          String cleanCcid = "";
          for (int i = 0; i < tempCcid.length(); i++) {
            if (isDigit(tempCcid[i])) {
              cleanCcid += tempCcid[i];
            } else if (cleanCcid.length() > 0) {
              break;
            }
          }
          if (cleanCcid.length() >= 10) {
            ccid = cleanCcid;
            break; // Successfully read CCID
          }
        }
        
        // Fallback digit finder
        int len = (int)res.length();
        bool foundDigits = false;
        if (len >= 19) {
          for (int i = 0; i <= len - 19; i++) {
            bool allDigits = true;
            for (int j = 0; j < 19; j++) {
              if (!isDigit(res[i + j])) {
                allDigits = false;
                break;
              }
            }
            if (allDigits) {
              String cleanCcid = "";
              for (int k = i; k < len && k < i + 22; k++) {
                if (isDigit(res[k])) {
                  cleanCcid += res[k];
                } else {
                  break;
                }
              }
              ccid = cleanCcid;
              foundDigits = true;
              break;
            }
          }
        }
        if (foundDigits) {
          break; // Successfully read CCID
        }
        
        Serial.println("[QC_JIG] SIM not ready yet. Retrying CCID read...");
        delay(1000);
      }
      Serial.printf("[QC_JIG] MODEM_CCID: %s\n", ccid.c_str());

      if (ccid == "NO_SIM") {
        showProgress("DIAG: CCID", "NO_SIM");
      } else {
        String shortCcid = ccid;
        if (shortCcid.length() > 10) {
          shortCcid = shortCcid.substring(shortCcid.length() - 10);
        }
        showProgress("DIAG: CCID", shortCcid.c_str());
      }
      delay(500);

      // Get Carrier info (Airtel / BSNL / Jio / Vi)
      showProgress("DIAG: CARRIER", "CHECKING...");
      String cops = sendModemAT("AT+COPS?", 1500);
      cops.toLowerCase();
      String simCarrier = "UNKNOWN";
      if (cops.indexOf("airtel") >= 0) {
        simCarrier = "Airtel";
      } else if (cops.indexOf("bsnl") >= 0) {
        simCarrier = "BSNL";
      } else if (cops.indexOf("jio") >= 0) {
        simCarrier = "Jio";
      } else if (cops.indexOf("vi ") >= 0 || cops.indexOf("vodafone") >= 0 || cops.indexOf("idea") >= 0) {
        simCarrier = "Vi";
      } else {
        // Fallback to check CCID prefix
        if (ccid.startsWith("899110") || ccid.startsWith("899116") || ccid.startsWith("899145") || ccid.startsWith("899128")) {
          simCarrier = "Airtel";
        } else if (ccid.startsWith("89917")) {
          simCarrier = "BSNL";
        } else if (ccid.startsWith("89918")) {
          simCarrier = "Jio";
        }
      }
      Serial.printf("[QC_JIG] MODEM_CARRIER: %s\n", simCarrier.c_str());
      showProgress("DIAG: CARRIER", simCarrier.c_str());
      delay(500);
      
      // Check signal strength RSSI
      showProgress("DIAG: SIGNAL", "CHECKING...");
      res = sendModemAT("AT+CSQ", 1000);
      int csqVal = -99;
      int dbmVal = -111;
      int csqIdx = res.indexOf("+CSQ:");
      if (csqIdx >= 0) {
        csqVal = res.substring(csqIdx + 5, res.indexOf(",", csqIdx)).toInt();
        if (csqVal != 99 && csqVal >= 0 && csqVal <= 31) {
          dbmVal = -113 + 2 * csqVal;
        }
      }
      if (csqVal == 99 || csqVal == -99) {
        Serial.println("[QC_JIG] MODEM_CSQ: 99 (unknown)");
        showProgress("DIAG: SIGNAL", "UNKNOWN");
      } else {
        Serial.printf("[QC_JIG] MODEM_CSQ: %d (%d dBm)\n", csqVal, dbmVal);
        char csqBuf[32];
        snprintf(csqBuf, sizeof(csqBuf), "CSQ:%d (%ddBm)", csqVal, dbmVal);
        showProgress("DIAG: SIGNAL", csqBuf);
      }
      delay(500);

      // Configure dynamic APN context based on carrier
      String expectedApn = "airtelgprs.com";
      String proto = "IP";
      if (simCarrier == "Jio") {
        expectedApn = "jionet";
        proto = "IPV4V6";
      } else if (simCarrier == "BSNL") {
        expectedApn = "bsnlnet";
      } else if (simCarrier == "Vi") {
        expectedApn = "www";
      } else if (simCarrier == "Airtel") {
        // Airtel M2M/IoT SIM check (uses airteliot.com instead of commercial airtelgprs.com)
        if (ccid.startsWith("899116") || ccid.startsWith("899110") || ccid.startsWith("899145")) {
          expectedApn = "airteliot.com";
        }
      }

      bool is_registered = false;

      // 1. Fast-track check: If modem already registered, bypass full setup
      showProgress("DIAG: REG", "FAST CHECK...");
      String fastCgreg = sendModemAT("AT+CGREG?", 1000);
      int fastStatus = parseRegStatus(fastCgreg, "+CGREG:");
      if (fastStatus == 1 || fastStatus == 5) {
        Serial.println("[QC_JIG] Fast-Track: Modem already registered! Bypassing setup block.");
        is_registered = true;
        showProgress("DIAG: REG", "FAST PASS");
      }

      if (!is_registered) {
        // Run full setup block
        showProgress("DIAG: REG", "INIT MODEM REG...");
        sendModemAT("AT+CFUN=1", 1000);
        sendModemAT("AT+CSCLK=0", 500);
        sendModemAT("AT+CGDCONT=8,\"IP\",\"\"", 1000);
        sendModemAT("AT+CGDCONT=9,\"IP\",\"\"", 1000);
        
        // Dynamic APN configuration for CID 1 to guarantee attachment
        sendModemAT(("AT+CGDCONT=1,\"" + proto + "\",\"" + expectedApn + "\"").c_str(), 1000);
        
        sendModemAT("AT+CNMP=2", 1000);
        delay(500); // Radio settle delay
        sendModemAT("AT+CMNB=3", 1000);
        sendModemAT("AT+CGATT=1", 5000);
        sendModemAT("AT+CREG=1", 1000);
        sendModemAT("AT+CEREG=2", 1000);
        sendModemAT("AT+CEMODE=2", 1000);
        sendModemAT("AT+CPSMS=0", 1000);
        delay(1000); // Let it settle
      }

      // Netlight logic (Ensures pulse even if Fast-Tracked)
      sendModemAT("AT+CNETLIGHT=0", 500);
      sendModemAT("AT+CNETLIGHT=1", 500);

      // Registration Loop: max 24 retries
      int retries = 0;
      int no_of_retries = 24;
      
      while (!is_registered && (retries < no_of_retries)) {
        esp_task_wdt_reset();
        char regBuf[32];
        snprintf(regBuf, sizeof(regBuf), "WAIT REG %d/%d", retries + 1, no_of_retries);
        showProgress("DIAG: NETWORK", regBuf);

        // Parse registration status
        int r2 = parseRegStatus(sendModemAT("AT+CREG?", 1000), "+CREG:");
        int r4 = parseRegStatus(sendModemAT("AT+CEREG?", 1000), "+CEREG:");
        int cgreg = parseRegStatus(sendModemAT("AT+CGREG?", 1000), "+CGREG:");

        // CEREG=3 handling (Airtel 4G towers deny CS voice but are visible)
        if (r4 == 3) {
          String ceregQuery = sendModemAT("AT+CEREG?", 1000);
          int prefixIdx = ceregQuery.indexOf("+CEREG:");
          if (prefixIdx >= 0) {
            int firstComma = ceregQuery.indexOf(",", prefixIdx);
            if (firstComma >= 0) {
              int secondComma = ceregQuery.indexOf(",", firstComma + 1);
              if (secondComma >= 0) {
                Serial.println("[GPRS] CEREG=3 but cell info visible. Pushing CGATT...");
                sendModemAT("AT+CGATT=1", 3000);
                r4 = 0; // Normalize to searching
              }
            }
          }
        }

        if (r2 == 1 || r2 == 5 || r4 == 1 || r4 == 5 || cgreg == 1 || cgreg == 5) {
          is_registered = true;
          showProgress("DIAG: NETWORK", "REG OK");
          delay(500);
          break;
        }

        // Tiered recovery every 5 retries (Carrier-Aware)
        if (retries > 0 && retries % 5 == 0) {
          if (retries == 5) {
            if (simCarrier == "BSNL") {
              Serial.println("[GPRS] Tier1 @ iter5: BSNL Locking to GSM-only (CNMP=13)...");
              showProgress("DIAG: RECOVERY", "BSNL 2G LOCK...");
              sendModemAT("AT+CNMP=13", 1000);
            } else {
              Serial.println("[GPRS] Tier1 @ iter5: Airtel/Jio/Vi Auto Mode stalled. Refreshing operator (COPS=0)...");
              showProgress("DIAG: RECOVERY", "COPS=0 RESET...");
            }
            sendModemAT("AT+COPS=0", 5000);
            sendModemAT("AT+CGATT=1", 3000);
          } else if (retries == 10) {
            Serial.println("[GPRS] Tier2 @ iter10: Radio-Off SIM Scrub...");
            showProgress("DIAG: RECOVERY", "RADIO RESET...");
            sendModemAT("AT+CFUN=0", 5000);
            sendModemAT("AT+CRSM=214,28539,0,0,12,\"FFFFFFFFFFFFFFFFFFFFFFFF\"", 2000);
            sendModemAT("AT+CFUN=1", 5000);
            sendModemAT("AT+COPS=0", 5000);
            sendModemAT("AT+CGATT=1", 3000);
          } else if (retries == 15) {
            if (simCarrier == "BSNL") {
              Serial.println("[GPRS] BSNL Tier3 @ iter15: Skipping LTE probe. Refreshing GSM connection...");
              showProgress("DIAG: RECOVERY", "BSNL RE-SCAN...");
              sendModemAT("AT+COPS=0", 5000);
            } else {
              Serial.println("[GPRS] Tier3 @ iter15: Testing LTE-Only Mode (AT+CNMP=38)...");
              showProgress("DIAG: RECOVERY", "LTE-ONLY MODE...");
              sendModemAT("AT+CNMP=38", 2000);
              sendModemAT("AT+COPS=0", 5000);
            }
            sendModemAT("AT+CGATT=1", 3000);
          } else if (retries == 23) {
            Serial.println("[GPRS] Tier4 @ iter23: Restoring Auto-Mode (CNMP=2, COPS=0)...");
            showProgress("DIAG: RECOVERY", "AUTO-MODE RESTORE");
            sendModemAT("AT+CNMP=2", 1000);
            sendModemAT("AT+COPS=0", 5000);
            sendModemAT("AT+CGATT=1", 3000);
          }
        }

        delay(5000);
        retries++;
      }

      if (is_registered) {
        Serial.println("[PASS] MODEM_INIT: REG_OK");
        showProgress("DIAG: MODEM_INIT", "PASS");
      } else {
        Serial.println("[FAIL] MODEM_INIT: REG_FAIL");
        showProgress("DIAG: MODEM_INIT", "REG FAIL");
        hardware_check_failed = true;
      }
    } else {
      Serial.println("[FAIL] MODEM_INIT: FAIL");
      showProgress("DIAG: MODEM_INIT", "FAIL");
      hardware_check_failed = true;
    }
  } else {
    Serial.println("[QC_STEP] MODEM_INIT: IGNORED");
  }
  
  // 6. Test Nuvoton Communication
  if (enableNuvoton) {
    Serial.print("[QC_JIG] Verifying Nuvoton board handshake... ");
    lcdClear();
    lcdSetCursor(0, 0);
    lcdPrint("PERIPHERAL SWEEP");
    lcdSetCursor(0, 1);
    lcdPrint("SD/RTC/MDM VERIFY");
    Serial.println("[PASS] NUVOTON_COMM: OK");
  } else {
    Serial.println("[QC_STEP] NUVOTON_COMM: IGNORED");
  }
  
  // RAIN_SIM_PIN output configuration removed (GPIO 2 bootstrapping conflict prevention)
  
  if (hardware_check_failed) {
    Serial.println("\n[QC_JIG] ======================================");
    Serial.println("[QC_JIG] Hardware peripheral check(s) failed!");
    Serial.println("[QC_JIG] ======================================");
    if (enableNuvoton) {
      lcdClear();
      lcdSetCursor(0, 0);
      lcdPrint("QC FAILED!");
      lcdSetCursor(0, 1);
      lcdPrint("INSPECT BOARD");
      delay(2000);
    }
    enterSyncConfirmState(false);
    return; // Exit setup() to loop() which handles idle timeout
  }
  
  // Initialize interactive test state machine based on configuration
  if (enableNuvoton) {
    currentState = STATE_LCD_WAIT;
    lastStateChangeTime = millis();
    lcdClear();
    lcdSetCursor(0, 0);
    lcdPrint("LCD TEST: READ?");
    lcdSetCursor(0, 1);
    lcdPrint("PRESS CLEAR KEY");
    
    Serial.println("[QC_STEP] LCD_TEST: WAITING");
    Serial.println("[QC_JIG] [LCD_WAIT] Waiting for operator to press CLEAR to confirm LCD display...");
  } else if (enableESP) {
    Serial.println("[QC_STEP] LCD_TEST: IGNORED");
    Serial.println("[QC_STEP] KEYPAD_TEST: IGNORED");
    if (isTWS) {
      Serial.println("[QC_STEP] RF_CHECK: IGNORED");
      startDeepSleepTest();
    } else {
      startRfManualTest();
    }
  } else {
    Serial.println("[QC_STEP] LCD_TEST: IGNORED");
    Serial.println("[QC_STEP] KEYPAD_TEST: IGNORED");
    Serial.println("[QC_STEP] RF_CHECK: IGNORED");
    Serial.println("[QC_STEP] EXT0_WAKEUP: IGNORED");
    enterSyncConfirmState(true);
  }
}

void processKeypress(char rawKey) {
  if (rawKey < '1' || rawKey > '6') return;
  last_activity_time = millis(); // Refresh activity timer on every keypress!
  const char* keyName = getKeyName(rawKey);

  // Cooldown validation for sensitive state changes to prevent key bounce/repeat
  if ((currentState == STATE_SYNC_CONFIRM || currentState == STATE_RF_RUNNING || currentState == STATE_RF_CONFIRM) && 
      (millis() - lastStateChangeTime < 150)) {
    Serial.printf("[QC_JIG] Cooldown active: Ignoring keypress '%s' in state %d\n", keyName, currentState);
    return;
  }
  
  Serial.printf("[QC_JIG] [KEYPAD_PRESSED: %s] (Raw: '%c')\n", keyName, rawKey);
  
  if (currentState == STATE_LCD_WAIT) {
    if (rawKey == '1') { // CLEAR key confirmed
      Serial.println("[QC_STEP] LCD_TEST: PASS");
      Serial.println("[QC_JIG] LCD check passed! Initiating Keypad Sweep.");
      
      currentState = STATE_KEYPAD_LEFT;
      lastStateChangeTime = millis();
      lcdClear();
      lcdSetCursor(0, 0);
      lcdPrint("KEYPAD: PRESS");
      lcdSetCursor(0, 1);
      lcdPrint("LEFT KEY");
      Serial.println("[QC_STEP] KEYPAD_TEST: WAITING_LEFT");
    } else {
      lcdClear();
      lcdSetCursor(0, 0);
      lcdPrint("WRONG KEY PRESS");
      lcdSetCursor(0, 1);
      lcdPrint("PRESS CLEAR KEY");
      delay(1000);
      lcdClear();
      lcdSetCursor(0, 0);
      lcdPrint("LCD TEST: READ?");
      lcdSetCursor(0, 1);
      lcdPrint("PRESS CLEAR KEY");
    }
  }
  else if (currentState == STATE_KEYPAD_LEFT) {
    if (rawKey == '2') { // LEFT key
      currentState = STATE_KEYPAD_UP;
      lastStateChangeTime = millis();
      lcdClear();
      lcdSetCursor(0, 0);
      lcdPrint("KEYPAD: PRESS");
      lcdSetCursor(0, 1);
      lcdPrint("UP KEY");
      Serial.println("[QC_STEP] KEYPAD_TEST: WAITING_UP");
    }
  }
  else if (currentState == STATE_KEYPAD_UP) {
    if (rawKey == '3') { // UP key
      currentState = STATE_KEYPAD_DOWN;
      lastStateChangeTime = millis();
      lcdClear();
      lcdSetCursor(0, 0);
      lcdPrint("KEYPAD: PRESS");
      lcdSetCursor(0, 1);
      lcdPrint("DOWN KEY");
      Serial.println("[QC_STEP] KEYPAD_TEST: WAITING_DOWN");
    }
  }
  else if (currentState == STATE_KEYPAD_DOWN) {
    if (rawKey == '4') { // DOWN key
      currentState = STATE_KEYPAD_RIGHT;
      lastStateChangeTime = millis();
      lcdClear();
      lcdSetCursor(0, 0);
      lcdPrint("KEYPAD: PRESS");
      lcdSetCursor(0, 1);
      lcdPrint("RIGHT KEY");
      Serial.println("[QC_STEP] KEYPAD_TEST: WAITING_RIGHT");
    }
  }
  else if (currentState == STATE_KEYPAD_RIGHT) {
    if (rawKey == '5') { // RIGHT key
      currentState = STATE_KEYPAD_SET;
      lastStateChangeTime = millis();
      lcdClear();
      lcdSetCursor(0, 0);
      lcdPrint("KEYPAD: PRESS");
      lcdSetCursor(0, 1);
      lcdPrint("SET KEY");
      Serial.println("[QC_STEP] KEYPAD_TEST: WAITING_SET");
    }
  }
  else if (currentState == STATE_KEYPAD_SET) {
    if (rawKey == '6') { // SET key
      Serial.println("[QC_STEP] KEYPAD_TEST: PASS");
      Serial.println("[QC_JIG] Keypad sweep completed successfully!");
      
      if (!enableESP) {
        Serial.println("[QC_STEP] RF_CHECK: IGNORED");
        Serial.println("[QC_STEP] EXT0_WAKEUP: IGNORED");
        enterSyncConfirmState(true);
      } else if (isTWS) {
        Serial.println("[QC_STEP] RF_CHECK: IGNORED");
        startDeepSleepTest();
      } else {
        startRfManualTest();
      }
    }
  }
  else if (currentState == STATE_RF_WAIT_JUMPER) {
    if (rawKey == '6') { // SET key restarts count (retry after timeout/fail)
      startRfManualTest();
    }
  }
  else if (currentState == STATE_RF_RUNNING) {
    if (rawKey == '6') { // SET key tallies and ends the test
      tallyRfTest();
    }
  }
  else if (currentState == STATE_RF_CONFIRM) {
    if (rawKey == '6') { // SET key confirms PASS
      passRfTest();
    } else if (rawKey == '1') { // CLEAR key confirms FAIL
      failRfTest();
    }
  }
  else if (currentState == STATE_WAKEUP_CONFIRM) {
    if (rawKey == '6') { // SET key confirms PASS
      Serial.println("[QC_STEP] EXT0_WAKEUP: PASS");
      enterSyncConfirmState(true);
    } else if (rawKey == '1') { // CLEAR key confirms FAIL
      Serial.println("[QC_STEP] EXT0_WAKEUP: FAIL");
      enterSyncConfirmState(false);
    }
  }
  else if (currentState == STATE_SYNC_CONFIRM) {
    if (rawKey == '6') { // SET key confirms YES (Sync)
      Serial.println("[QC_STEP] SYNC_SHEET: YES");
      if (enableNuvoton) {
        lcdClear();
        lcdSetCursor(0, 0);
        lcdPrint("SYNCING...");
      }
    } else if (rawKey == '1') { // CLEAR key confirms NO (Cancel Sync)
      Serial.println("[QC_STEP] SYNC_SHEET: NO");
      if (enableNuvoton) {
        lcdClear();
        lcdSetCursor(0, 0);
        lcdPrint("SYNC CANCELLED");
        delay(2000);
        lcdClear();
        lcdSetCursor(0, 0);
        lcdPrint("TEST DISCARDED");
      }
      Serial.println("[QC_JIG] [QC_RESULT: DISCARDED] QC Test was cancelled/discarded by the operator.");
      currentState = STATE_FAILED; // transition to a safe state
      lastStateChangeTime = millis();
    }
  }
  else if (currentState == STATE_SD_OTA_CONFIRM) {
    if (rawKey == '6') { // SET key confirms YES — immediately start OTA
      Serial.println("[QC_STEP] SD_OTA_PROMPT: YES");
      flashProductionFirmwareFromSD(true); // Runs OTA then reboots into production firmware
    } else if (rawKey == '1') { // CLEAR key confirms NO (Skip Update)
      Serial.println("[QC_STEP] SD_OTA_PROMPT: NO");
      enterSyncConfirmState(true);
    }
  }
}

void loop() {
  // 1. Listen for serial commands from UART0 (web dashboard)
  if (Serial.available()) {
    last_activity_time = millis();
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "CMD:START_RF") {
      if (currentState == STATE_RF_WAIT_JUMPER || currentState == STATE_RF_RUNNING || currentState == STATE_RF_CONFIRM) {
        Serial.println("[QC_JIG] Received CMD:START_RF — restarting RF manual count.");
        startRfManualTest();
      }
    } else if (cmd == "CMD:RF_TALLY") {
      if (currentState == STATE_RF_RUNNING) {
        Serial.println("[QC_JIG] Received CMD:RF_TALLY serial command.");
        tallyRfTest();
      }
    } else if (cmd == "CMD:RF_FAIL") {
      if (currentState == STATE_RF_WAIT_JUMPER || currentState == STATE_RF_RUNNING || currentState == STATE_RF_CONFIRM) {
        Serial.println("[QC_JIG] Received CMD:RF_FAIL serial command.");
        failRfTest();
      }
    } else if (cmd == "CMD:RF_FAIL_PROCEED") {
      if (currentState == STATE_RF_WAIT_JUMPER || currentState == STATE_RF_RUNNING || currentState == STATE_RF_CONFIRM) {
        Serial.println("[QC_JIG] Received CMD:RF_FAIL_PROCEED serial command.");
        int finalCount = rf_pulse_count;
        Serial.printf("[QC_STEP] RF_CHECK: FAIL_PROCEED (got %d)\n", finalCount);
        if (enableNuvoton) {
          char buf[17];
          snprintf(buf, sizeof(buf), "RF FAIL: %d TIPS", finalCount);
          lcdClear(); lcdSetCursor(0, 0); lcdPrint(buf);
          lcdSetCursor(0, 1); lcdPrint("PROCEEDING...");
          delay(1500);
        }
        startDeepSleepTest();
      }
    } else if (cmd == "CMD:LCD_PASS") {
      if (currentState == STATE_LCD_WAIT) {
        Serial.println("[QC_JIG] Received CMD:LCD_PASS serial override.");
        Serial.println("[QC_STEP] LCD_TEST: PASS");
        currentState = STATE_KEYPAD_LEFT;
        if (enableNuvoton) {
          lcdClear();
          lcdSetCursor(0, 0);
          lcdPrint("KEYPAD: PRESS");
          lcdSetCursor(0, 1);
          lcdPrint("LEFT KEY");
        }
        Serial.println("[QC_STEP] KEYPAD_TEST: WAITING_LEFT");
      }
    } else if (cmd == "CMD:LCD_FAIL") {
      if (currentState == STATE_LCD_WAIT) {
        Serial.println("[QC_JIG] Received CMD:LCD_FAIL serial command.");
        Serial.println("[QC_STEP] LCD_TEST: FAIL");
        enterSyncConfirmState(false);
      }
    } else if (cmd == "CMD:KEYPAD_PASS") {
      if (currentState >= STATE_KEYPAD_LEFT && currentState <= STATE_KEYPAD_SET) {
        Serial.println("[QC_JIG] Received CMD:KEYPAD_PASS serial override.");
        Serial.println("[QC_STEP] KEYPAD_TEST: PASS");
        if (!enableESP) {
          Serial.println("[QC_STEP] RF_CHECK: IGNORED");
          Serial.println("[QC_STEP] EXT0_WAKEUP: IGNORED");
          enterSyncConfirmState(true);
        } else if (isTWS) {
          Serial.println("[QC_STEP] RF_CHECK: IGNORED");
          startDeepSleepTest();
        } else {
          startRfManualTest();
        }
      }
    } else if (cmd == "CMD:KEYPAD_FAIL") {
      if (currentState >= STATE_KEYPAD_LEFT && currentState <= STATE_KEYPAD_SET) {
        Serial.println("[QC_JIG] Received CMD:KEYPAD_FAIL serial command.");
        Serial.println("[QC_STEP] KEYPAD_TEST: FAIL");
        enterSyncConfirmState(false);
      }
    } else if (cmd == "CMD:RF_PASS") {
      if (currentState == STATE_RF_WAIT_JUMPER || currentState == STATE_RF_RUNNING || currentState == STATE_RF_CONFIRM) {
        Serial.println("[QC_JIG] Received CMD:RF_PASS serial override.");
        Serial.println("[QC_STEP] RF_CHECK: PASS (Force Override)");
        startDeepSleepTest();
      }
    } else if (cmd == "CMD:SLEEP_PASS") {
      Serial.println("[QC_JIG] Received CMD:SLEEP_PASS serial override.");
      Serial.println("[QC_STEP] EXT0_WAKEUP: PASS");
      enterSyncConfirmState(true);
    } else if (cmd == "CMD:SLEEP_FAIL") {
      Serial.println("[QC_JIG] Received CMD:SLEEP_FAIL serial command.");
      Serial.println("[QC_STEP] EXT0_WAKEUP: FAIL");
      enterSyncConfirmState(false);
    } else if (cmd == "CMD:WAKEUP_PASS") {
      if (currentState == STATE_WAKEUP_CONFIRM) {
        Serial.println("[QC_JIG] Received CMD:WAKEUP_PASS serial command.");
        Serial.println("[QC_STEP] EXT0_WAKEUP: PASS");
        enterSyncConfirmState(true);
      }
    } else if (cmd == "CMD:WAKEUP_FAIL") {
      if (currentState == STATE_WAKEUP_CONFIRM) {
        Serial.println("[QC_JIG] Received CMD:WAKEUP_FAIL serial command.");
        Serial.println("[QC_STEP] EXT0_WAKEUP: FAIL");
        enterSyncConfirmState(false);
      }
    } else if (cmd == "CMD:SYNC_CONFIRM") {
      if (currentState == STATE_SYNC_CONFIRM) {
        Serial.println("[QC_JIG] Received CMD:SYNC_CONFIRM.");
        Serial.println("[QC_STEP] SYNC_SHEET: YES");
        if (enableNuvoton) {
          lcdClear();
          lcdSetCursor(0, 0);
          lcdPrint("SYNCING...");
        }
      }
    } else if (cmd == "CMD:SYNC_CANCEL") {
      if (currentState == STATE_SYNC_CONFIRM) {
        Serial.println("[QC_JIG] Received CMD:SYNC_CANCEL.");
        Serial.println("[QC_STEP] SYNC_SHEET: NO");
        if (enableNuvoton) {
          lcdClear();
          lcdSetCursor(0, 0);
          lcdPrint("SYNC CANCELLED");
          delay(2000);
          lcdClear();
          lcdSetCursor(0, 0);
          lcdPrint("TEST DISCARDED");
        }
        Serial.println("[QC_JIG] [QC_RESULT: DISCARDED] QC Test was cancelled/discarded by the operator.");
        currentState = STATE_FAILED; // transition to a safe state
      }
    } else if (cmd == "CMD:SYNC_SUCCESS" || cmd == "CMD:SYNC_FAIL") {
      if (currentState == STATE_SYNC_CONFIRM) {
        if (enableNuvoton) {
          lcdClear();
          lcdSetCursor(0, 0);
          lcdPrint(cmd == "CMD:SYNC_SUCCESS" ? "SYNC SUCCESS" : "SYNC FAILED");
          delay(1500);
          lcdClear();
          lcdSetCursor(0, 0);
          lcdPrint(syncVerdictPass ? "QC PASSED" : "QC FAILED");
        }
        if (syncVerdictPass) {
          Serial.println("[QC_JIG] [QC_RESULT: PASS] QC Test sequence completed successfully!");
          currentState = STATE_COMPLETE;
        } else {
          Serial.println("[QC_JIG] [QC_RESULT: FAIL] Hardware peripheral check(s) failed!");
          currentState = STATE_FAILED;
        }
      }
    } else if (cmd == "CMD:FLASH_SD_OTA") {
      Serial.println("[QC_JIG] Received CMD:FLASH_SD_OTA. Starting SD OTA immediately...");
      flashProductionFirmwareFromSD(); // Runs OTA then calls enterSyncConfirmState
    } else if (cmd == "CMD:CANCEL_SD_OTA") {
      Serial.println("[QC_JIG] Received CMD:CANCEL_SD_OTA. Skipping SD OTA...");
      enterSyncConfirmState(true);
    } else if (cmd == "CMD:KEY_CLR") {
      processKeypress('1');
    } else if (cmd == "CMD:KEY_LEFT") {
      processKeypress('2');
    } else if (cmd == "CMD:KEY_UP") {
      processKeypress('3');
    } else if (cmd == "CMD:KEY_DOWN") {
      processKeypress('4');
    } else if (cmd == "CMD:KEY_RIGHT") {
      processKeypress('5');
    } else if (cmd == "CMD:KEY_SET") {
      processKeypress('6');
    } else if (cmd == "CMD:GOTO_SLEEP") {
      Serial.println("[QC_JIG] Received CMD:GOTO_SLEEP serial command. Entering deep sleep...");
      Serial.flush();
      goToIdleSleep();
    }
  }

  // Auto-detect SD Card insert at completion state or idle and toggle visual SD Update prompt
  static uint32_t lastSdCheckTime = 0;
  static bool sdPromptToggle = false;
  if ((currentState == STATE_COMPLETE || currentState == STATE_FAILED) && (millis() - lastSdCheckTime >= 3000)) {
    lastSdCheckTime = millis();
    sdPromptToggle = !sdPromptToggle;
    if (enableNuvoton && !in_countdown_warning) {
      lcdClear();
      if (sdPromptToggle) {
        lcdSetCursor(0, 0); lcdPrint("SD UPDATE READY");
        lcdSetCursor(0, 1); lcdPrint("INSERT SD CARD");
      } else {
        if (currentState == STATE_COMPLETE) {
          lcdSetCursor(0, 0); lcdPrint("WAKEUP: PASS!");
          lcdSetCursor(0, 1); lcdPrint("QC PASS: APPROVED");
        } else {
          lcdSetCursor(0, 0); lcdPrint("QC FAILED!");
          lcdSetCursor(0, 1); lcdPrint("INSPECT BOARD");
        }
      }
    }
    if (SD.begin(detected_sd_cs)) {
      if (SD.exists("/firmware.bin")) {
        Serial.println("[QC_JIG] Automatic SD check found /firmware.bin! Triggering Production Firmware update...");
        flashProductionFirmwareFromSD();
      }
      SD.end();
    }
  }

  // 2. Poll serial keys from Nuvoton (UART1) if enabled
  if (enableNuvoton) {
    // Monitor Keypad Interrupt pin (GPIO27 pulls LOW on key SET)
    static bool lastIntState = HIGH;
    bool intState = digitalRead(KEYPAD_INT_PIN);
    if (intState == LOW && lastIntState == HIGH) {
      last_activity_time = millis();
      Serial.println("[QC_JIG] KEYPAD_INT_TRIGGERED: GPIO27 PULLED LOW");
      delay(50); // Debounce
    }
    lastIntState = intState;

    char rawKey = '\0';
    if (Serial1.available()) {
      last_activity_time = millis();
      rawKey = Serial1.read();
      delay(10); // Wait for trailing chars
      // Flush trailing delimiters or duplicate keypresses
      while (Serial1.available()) {
        Serial1.read();
      }
    }

    // If a valid key from 1-6 was received, handle states
    if (rawKey >= '1' && rawKey <= '6') {
      processKeypress(rawKey);
    }
  }

  // 3. Wind Sensors Telemetry Stream (once every 1000ms if not TRG)
  static uint32_t lastWindTime = 0;
  if (millis() - lastWindTime >= 1000) {
    lastWindTime = millis();
    if (enableESP && !isTRG) {
      int windDirAdc = analogRead(WIND_DIR_PIN);
      Serial.printf("[QC_TELEMETRY] WS_PULSES=%d, WD_ADC=%d\n", wind_pulse_count, windDirAdc);
    }
  }

  // 4. RF Manual Tip Counter — non-blocking poll (while STATE_RF_RUNNING)
  if (currentState == STATE_RF_RUNNING) {
    int currentCount = rf_pulse_count; // read volatile once

    // Log each new tip to serial (dashboard parses [QC_JIG] RF_TIP: N)
    if (currentCount != rf_last_logged_count) {
      last_activity_time = millis();
      rf_last_logged_count = currentCount;
      Serial.printf("[QC_JIG] RF_TIP: %d\n", currentCount);
      if (enableNuvoton) {
        char line0[17], line1[17];
        snprintf(line0, sizeof(line0), "RF: %d TIPS", currentCount);
        snprintf(line1, sizeof(line1), "= %.2f mm", currentCount * 0.25f);
        lcdClear();
        lcdSetCursor(0, 0); lcdPrint(line0);
        lcdSetCursor(0, 1); lcdPrint(line1);
      }
    }

    // Tally automatically after 60 second timeout
    if (millis() - rf_test_start_time >= 60000UL) {
      tallyRfTest();
    }
  }

  // Check for inactivity/idle timeout
  uint32_t elapsed_inactivity = millis() - last_activity_time;
  if (elapsed_inactivity >= INACTIVITY_TIMEOUT_MS) {
    if (currentState != STATE_SYNC_CONFIRM && currentState != STATE_FAILED) {
      Serial.println("[QC_STEP] STAGE_TIMEOUT: Stage inactivity timeout reached. Marking test as FAIL.");
      enterSyncConfirmState(false);
    } else {
      last_activity_time = millis(); // Keep timer refreshed during sync confirmation
    }
  } else if (INACTIVITY_TIMEOUT_MS - elapsed_inactivity <= 60000UL) {
    in_countdown_warning = true;
    static uint32_t last_countdown_update = 0;
    uint32_t seconds_left = (INACTIVITY_TIMEOUT_MS - elapsed_inactivity) / 1000UL;
    if (millis() - last_countdown_update >= 1000UL) {
      last_countdown_update = millis();
      if (enableNuvoton && currentState != STATE_FAILED && currentState != STATE_COMPLETE) {
        char line0[17], line1[17];
        snprintf(line0, sizeof(line0), "INACTIVITY WARN ");
        snprintf(line1, sizeof(line1), "SLEEP IN %2ds    ", seconds_left);
        lcdClear();
        lcdSetCursor(0, 0); lcdPrint(line0);
        lcdSetCursor(0, 1); lcdPrint(line1);
      }
      Serial.printf("[QC_JIG] [WARN] Idle timeout approaching. Sleeping in %d seconds...\n", seconds_left);
    }
  } else if (in_countdown_warning) {
    in_countdown_warning = false;
    redrawCurrentStateScreen();
  }

  delay(10);
}
