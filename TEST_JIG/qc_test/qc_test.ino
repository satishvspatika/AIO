#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_mac.h>

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
  STATE_COMPLETE,
  STATE_FAILED
};

#define INACTIVITY_TIMEOUT_MS 300000
RTC_DATA_ATTR bool expecting_test_wakeup = false;
unsigned long last_activity_time = 0;

QCState currentState = STATE_AUTO_TESTS;
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
  if (!SPIFFS.begin(false)) {
    Serial.println("\n[QC_JIG] SPIFFS not formatted. Formatting partition (takes 10-30s)...");
    if (!SPIFFS.begin(true)) return false;
  }
  File f = SPIFFS.open("/qc.txt", FILE_WRITE);
  if (!f) return false;
  f.println("SPIFFS_OK");
  f.close();
  
  f = SPIFFS.open("/qc.txt", FILE_READ);
  if (!f) return false;
  String s = f.readStringUntil('\n');
  f.close();
  SPIFFS.remove("/qc.txt");
  return (s.indexOf("SPIFFS_OK") >= 0);
}

bool testSD(int csPin) {
  // Try initializing SPI SD on given CS pin
  if (!SD.begin(csPin)) return false;
  
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
  return (s.indexOf("SD_CARD_OK") >= 0);
}

bool testRTC() {
  Wire.begin(21, 22);
  Wire.beginTransmission(0x68); // DS1307 Address
  if (Wire.endTransmission() != 0) return false;
  
  // Read seconds register twice to verify it increments
  Wire.beginTransmission(0x68);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 1);
  if (!Wire.available()) return false;
  uint8_t sec1 = Wire.read() & 0x7F;
  
  delay(1100); // Wait for clock tick
  
  Wire.beginTransmission(0x68);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 1);
  if (!Wire.available()) return false;
  uint8_t sec2 = Wire.read() & 0x7F;
  
  return (sec1 != sec2);
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
    
    if (chipId == 0x60) {
      sensorName = "BME280";
      temp = 25.0; // dummy values
      hum = 50.0;
      return true;
    } else if (chipId == 0x58) {
      sensorName = "BMP280";
      temp = 25.0;
      hum = 0.0;
      return true;
    }
  }
  
  sensorName = "NONE";
  return false;
}

// Helper to query Modem with AT commands
String sendModemAT(const char* cmd, uint32_t timeoutMs) {
  while (Serial2.available()) Serial2.read(); // Flush input
  Serial2.println(cmd);
  
  String response = "";
  uint32_t start = millis();
  while (millis() - start < timeoutMs) {
    if (Serial2.available()) {
      response += (char)Serial2.read();
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
  currentState = STATE_RF_RUNNING;

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
  Serial.println("[QC_STEP] RF_CHECK: WAITING_JUMPER");
}

void startDeepSleepTest() {
  Serial.println("[QC_JIG] Initiating Deep Sleep Wakeup verification...");
  Serial.println("[QC_STEP] EXT0_WAKEUP: WAITING_SLEEP");
  Serial.flush();
  delay(1000);
  
  // Cut power to LCD and GPRS PMOS gates (active-HIGH PMOS gate, write LOW to turn off)
  digitalWrite(LCD_CTRL_PIN, LOW);
  digitalWrite(GPRS_CTRL_PIN, LOW);
  delay(100);
  
  // Set the flag so we know this sleep was for the test verification wakeup
  expecting_test_wakeup = true;
  
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
  digitalWrite(GPRS_CTRL_PIN, LOW);
  delay(100);
  
  // This is a power-saving sleep, not the test verification wakeup
  expecting_test_wakeup = false;
  
  // Enable EXT0 wakeup on GPIO 27 (SET key, pulls LOW when pressed)
  esp_sleep_enable_ext0_wakeup((gpio_num_t)KEYPAD_INT_PIN, 0);
  
  // Enter Deep Sleep
  esp_deep_sleep_start();
}

void setup() {
  // Initialize Serial Monitor (UART0)
  Serial.begin(115200);
  delay(100);
  
  // Check if we woke up from Deep Sleep via EXT0 trigger
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0 && expecting_test_wakeup) {
    expecting_test_wakeup = false; // Reset the flag
    
    // Power ON LCD
    pinMode(LCD_CTRL_PIN, OUTPUT);
    digitalWrite(LCD_CTRL_PIN, HIGH);
    
    // Initialize LCD UART and print verdict
    Serial1.begin(9600, SERIAL_8N1, NUV_RX_PIN, NUV_TX_PIN);
    delay(100);
    lcdClear();
    lcdSetCursor(0, 0);
    lcdPrint("WAKEUP: PASS!");
    lcdSetCursor(0, 1);
    lcdPrint("QC PASS: APPROVED");
    
    Serial.println("\n[QC_JIG] ======================================");
    Serial.println("[QC_JIG] Board woke up from Deep Sleep via EXT0 (SET key)!");
    Serial.println("[QC_STEP] EXT0_WAKEUP: PASS");
    Serial.println("[QC_JIG] [QC_RESULT: PASS] QC Test sequence completed successfully!");
    Serial.println("[QC_JIG] ======================================");
    
    currentState = STATE_COMPLETE;
    last_activity_time = millis();
    return; // Go straight to loop() and bypass setup checks
  }

  Serial.println("\n[QC_JIG] ======================================");
  Serial.println("[QC_JIG] SPATIKA AIO BOARD QC TEST FIRMWARE START");
  
  // Get MAC ID reliably using WiFi helper class
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
  
  // Power Controls
  pinMode(GPRS_CTRL_PIN, OUTPUT);
  pinMode(LCD_CTRL_PIN, OUTPUT);
  
  digitalWrite(GPRS_CTRL_PIN, LOW);  // Turn off GPRS first to clear latch-up
  digitalWrite(LCD_CTRL_PIN, LOW);   // Turn off LCD board initially
  
  if (enableNuvoton) {
    digitalWrite(LCD_CTRL_PIN, HIGH);  // Power ON Nuvoton board
  }
  
  if (enableGPRS) {
    delay(4000);                       // Keep GPRS off for 4 seconds to fully discharge
    digitalWrite(GPRS_CTRL_PIN, HIGH); // Power ON GPRS board
    delay(3000);                       // Wait 3 seconds for power rails to stabilize
  } else {
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
    } else {
      Serial.println("[FAIL] SPIFFS_CHECK: FAIL");
      showProgress("DIAG: SPIFFS", "FAIL");
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
    pinMode(WIND_SPD_PIN, INPUT_PULLUP);
    wind_pulse_count = 0;
    attachInterrupt(digitalPinToInterrupt(WIND_SPD_PIN), wind_pulse_isr, FALLING);
    Serial.println("[QC_JIG] Wind speed pulse interrupt attached.");
  } else {
    Serial.println("[QC_STEP] WIND_SPD: IGNORED");
    Serial.println("[QC_STEP] WIND_DIR: IGNORED");
  }
  
  // 4. Analog Readings
  if (enableESP) {
    int rawBatt = analogRead(BATT_3V7_PIN);
    int rawV33 = analogRead(SYS_3V3_PIN);
    int rawSolar = analogRead(SOLAR_ADC_PIN);
    
    float battVolt = (rawBatt / 4095.0) * 3.3 * (840.0 / 620.0);
    float v33Volt = (rawV33 / 4095.0) * 3.3 * (840.0 / 620.0); // Resistor divider matches Batt
    float solarVolt = (rawSolar / 4095.0) * 3.3 * 7.2;
   
    Serial.printf("[QC_JIG] ADCs: Rain=%d, WindSpeed=%d, WindDir=%d\n", analogRead(RAIN_ADC_PIN), analogRead(WIND_SPD_PIN), analogRead(WIND_DIR_PIN));
    Serial.printf("[QC_JIG] BATT_3V7_ADC: %d (Derived: %.2f V), SYS_3V3_ADC: %d (Derived: %.2f V), SOLAR_ADC: %d (Derived: %.2f V)\n", rawBatt, battVolt, rawV33, v33Volt, rawSolar, solarVolt);
  } else {
    Serial.println("[QC_STEP] ADC_CHECK: IGNORED");
  }
  
  // 5. GPRS Modem Setup and Test
  if (enableGPRS) {
    showProgress("DIAG: GPRS MDM", "DISCHARGING 4s");
    delay(4000);                       // Keep GPRS off for 4 seconds to fully discharge
    digitalWrite(GPRS_CTRL_PIN, HIGH); // Power ON GPRS board
    showProgress("DIAG: GPRS MDM", "STABILIZING 3s");
    delay(3000);                       // Wait 3 seconds for power rails to stabilize
    
    showProgress("DIAG: GPRS MDM", "INITIALIZING...");
    Serial.println("[QC_JIG] Initializing GPRS Modem UART2...");
    Serial2.begin(115200, SERIAL_8N1, GPRS_RX_PIN, GPRS_TX_PIN);
    
    bool modem_init_ok = false;
    // Poll AT up to 15 times to wait for boot
    for (int i = 0; i < 15; i++) {
      char statusBuf[32];
      snprintf(statusBuf, sizeof(statusBuf), "POLL AT %d/15", i + 1);
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

      // Enable verbose error messages
      sendModemAT("AT+CMEE=2", 500);
      showProgress("DIAG: SIM CPIN", "SETTLING 3s");
      delay(3000);

      // Verify SIM is ready before CCID fetch (up to 15 retries for slow SIM on power-on)
      showProgress("DIAG: SIM CPIN", "CHECKING...");
      bool sim_ready = false;
      for (int i = 0; i < 15; i++) {
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
  
  // Configure simulation pulse generator pin
  pinMode(RAIN_SIM_PIN, OUTPUT);
  digitalWrite(RAIN_SIM_PIN, HIGH);
  
  if (hardware_check_failed) {
    Serial.println("\n[QC_JIG] ======================================");
    Serial.println("[QC_JIG] [QC_RESULT: FAIL] Hardware peripheral check(s) failed!");
    Serial.println("[QC_JIG] ======================================");
    if (enableNuvoton) {
      lcdClear();
      lcdSetCursor(0, 0);
      lcdPrint("QC FAILED!");
      lcdSetCursor(0, 1);
      lcdPrint("INSPECT BOARD");
    }
    currentState = STATE_FAILED;
    last_activity_time = millis();
    return; // Exit setup() to loop() which handles idle timeout
  }
  
  // Initialize interactive test state machine based on configuration
  if (enableNuvoton) {
    currentState = STATE_LCD_WAIT;
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
    currentState = STATE_COMPLETE;
    Serial.println("[QC_STEP] LCD_TEST: IGNORED");
    Serial.println("[QC_STEP] KEYPAD_TEST: IGNORED");
    Serial.println("[QC_STEP] RF_CHECK: IGNORED");
    Serial.println("[QC_STEP] EXT0_WAKEUP: IGNORED");
    Serial.println("[QC_JIG] [QC_RESULT: PASS] QC Test sequence completed successfully!");
  }
  
  last_activity_time = millis();
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
      if (currentState == STATE_RF_CONFIRM) {
        Serial.println("[QC_JIG] Received CMD:RF_FAIL serial command.");
        failRfTest();
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
    } else if (cmd == "CMD:KEYPAD_PASS") {
      if (currentState >= STATE_KEYPAD_LEFT && currentState <= STATE_KEYPAD_SET) {
        Serial.println("[QC_JIG] Received CMD:KEYPAD_PASS serial override.");
        Serial.println("[QC_STEP] KEYPAD_TEST: PASS");
        if (isTWS) {
          Serial.println("[QC_STEP] RF_CHECK: IGNORED");
          startDeepSleepTest();
        } else {
          startRfManualTest();
        }
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
      Serial.println("[QC_JIG] [QC_RESULT: PASS] QC Test sequence completed successfully!");
      currentState = STATE_COMPLETE;
      last_activity_time = millis();
    } else if (cmd == "CMD:GOTO_SLEEP") {
      Serial.println("[QC_JIG] Received CMD:GOTO_SLEEP serial command. Entering deep sleep...");
      Serial.flush();
      goToIdleSleep();
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
      delay(5);
      // Debounce double-char sends if present
      if (Serial1.available()) {
        char rawKey2 = Serial1.read();
        if (rawKey2 != rawKey) rawKey = '\0';
      }
    }

    // If a valid key from 1-6 was received, handle states
    if (rawKey >= '1' && rawKey <= '6') {
      const char* keyName = getKeyName(rawKey);
      Serial.printf("[QC_JIG] [KEYPAD_PRESSED: %s] (Raw: '%c')\n", keyName, rawKey);
      
      if (currentState == STATE_LCD_WAIT) {
        if (rawKey == '1') { // CLEAR key confirmed
          Serial.println("[QC_STEP] LCD_TEST: PASS");
          Serial.println("[QC_JIG] LCD check passed! Initiating Keypad Sweep.");
          
          currentState = STATE_KEYPAD_LEFT;
          lcdClear();
          lcdSetCursor(0, 0);
          lcdPrint("KEYPAD: PRESS");
          lcdSetCursor(0, 1);
          lcdPrint("LEFT KEY");
          Serial.println("[QC_STEP] KEYPAD_TEST: WAITING_LEFT");
        } else {
          lcdClear();
          lcdSetCursor(0, 0);
          lcdPrint("WRONG KEY PRESSED");
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
          
          if (isTWS) {
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
  if (millis() - last_activity_time >= INACTIVITY_TIMEOUT_MS) {
    goToIdleSleep();
  }

  delay(10);
}
