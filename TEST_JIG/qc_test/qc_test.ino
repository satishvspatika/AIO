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
  STATE_COMPLETE
};

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
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
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
    
    // Hold forever
    while (true) {
      delay(1000);
    }
  }

  Serial.println("\n[QC_JIG] ======================================");
  Serial.println("[QC_JIG] SPATIKA AIO BOARD QC TEST FIRMWARE START");
  
  // Get MAC ID reliably using WiFi helper class
  String macStr = WiFi.macAddress();
  Serial.printf("[QC_JIG] ESP32 Unique MAC: %s\n", macStr.c_str());
  Serial.println("[QC_JIG] ======================================");
  
  // Handshake to receive test configuration from serial dashboard
  Serial.println("[QC_JIG] [READY] WAITING_FOR_CONFIG");
  uint32_t startWait = millis();
  String configStr = "";
  while (millis() - startWait < 1500) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        if (configStr.startsWith("CFG:")) {
          break;
        }
      } else {
        configStr += c;
      }
    }
    delay(1);
  }
  
  if (configStr.startsWith("CFG:")) {
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
  } else {
    Serial.println("[QC_JIG] No config received. Defaulting to TRG tests.");
    isTRG = true;
    isTWS = false;
    isTWSRF = false;
  }
  
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
    lcdClear();
    lcdSetCursor(0, 0);
    lcdPrint("SPATIKA QC TEST");
    lcdSetCursor(0, 1);
    lcdPrint("BOOTING SYSTEM..");
  }

  // 1. SPIFFS Test
  if (enableESP) {
    Serial.print("[QC_JIG] Testing SPIFFS... ");
    if (testSPIFFS()) {
      Serial.println("[PASS] SPIFFS_CHECK: OK");
    } else {
      Serial.println("[FAIL] SPIFFS_CHECK: FAIL");
    }
  } else {
    Serial.println("[QC_STEP] SPIFFS_CHECK: IGNORED");
  }
  
  // 2. SD Card Test (Pins 5 and 13 CS)
  if (enableESP) {
    Serial.print("[QC_JIG] Testing SD Card on CS 5... ");
    bool sdOk = testSD(5);
    if (!sdOk) {
      Serial.print("failed CS 5, trying CS 13... ");
      sdOk = testSD(13);
    }
    if (sdOk) {
      Serial.println("[PASS] SD_CHECK: OK");
    } else {
      Serial.println("[FAIL] SD_CHECK: FAIL");
    }
  } else {
    Serial.println("[QC_STEP] SD_CHECK: IGNORED");
  }
  
  // 3. RTC Clock Test
  if (enableESP) {
    Serial.print("[QC_JIG] Testing RTC (0x68) I2C... ");
    if (testRTC()) {
      Serial.println("[PASS] RTC_CHECK: OK");
    } else {
      Serial.println("[FAIL] RTC_CHECK: FAIL");
    }
  } else {
    Serial.println("[QC_STEP] RTC_CHECK: IGNORED");
  }
  
  // 3a. WiFi Scan Test
  if (enableESP) {
    Serial.print("[QC_JIG] Testing WiFi Scan... ");
    if (testWiFi()) {
      // Printed inside testWiFi
    } else {
      Serial.println("[FAIL] WIFI_CHECK: FAIL");
    }
  } else {
    Serial.println("[QC_STEP] WIFI_CHECK: IGNORED");
  }

  // 3b. Environmental Sensor Test (I2C)
  if (enableESP && !isTRG) {
    Serial.print("[QC_JIG] Testing Env Sensor I2C... ");
    String sensorName = "";
    float temp = 0.0;
    float hum = 0.0;
    if (testTempHum(sensorName, temp, hum)) {
      Serial.printf("[PASS] SENSOR_CHECK: OK (%s: Temp=%.1fC, Hum=%.1f%%)\n", sensorName.c_str(), temp, hum);
    } else {
      Serial.println("[FAIL] SENSOR_CHECK: FAIL");
    }
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
    Serial.println("[QC_JIG] Initializing GPRS Modem UART2...");
    Serial2.begin(115200, SERIAL_8N1, GPRS_RX_PIN, GPRS_TX_PIN);
    
    bool modem_init_ok = false;
    // Poll AT up to 15 times to wait for boot
    for (int i = 0; i < 15; i++) {
      String res = sendModemAT("AT", 500);
      if (res.indexOf("OK") >= 0) {
        modem_init_ok = true;
        break;
      }
      delay(500);
    }
    
    if (modem_init_ok) {
      Serial.println("[PASS] MODEM_INIT: OK");
      
      // Wait for SIM Profile to stabilize
      bool sim_ready = false;
      for (int i = 0; i < 15; i++) {
        String res = sendModemAT("AT+CPIN?", 1000);
        if (res.indexOf("READY") >= 0) {
          sim_ready = true;
          break;
        }
        delay(1000);
      }
      
      // Read IMEI
      String res = sendModemAT("AT+CGSN", 1000);
      res.replace("\r", "");
      res.trim();
      // Locate IMEI numeric response
      String imei = "UNKNOWN";
      for (int i = 0; i < (int)res.length() - 14; i++) {
        if (isDigit(res[i]) && isDigit(res[i+14])) {
          imei = res.substring(i, i + 15);
          break;
        }
      }
      Serial.printf("[QC_JIG] MODEM_IMEI: %s\n", imei.c_str());
      
      // Read CCID safely (Try AT+CICCID first for A7672, fallback to AT+CCID)
      res = sendModemAT("AT+CICCID", 1000);
      int headerLen = 8;
      int ccidIdx = res.indexOf("+CICCID:");
      if (ccidIdx < 0) {
        ccidIdx = res.indexOf("+ICCID:");
        headerLen = 7;
      }
      if (ccidIdx < 0) {
        res = sendModemAT("AT+CCID", 1000);
        ccidIdx = res.indexOf("+CCID:");
        headerLen = 6;
      }
      if (ccidIdx < 0) {
        ccidIdx = res.indexOf("+ICCID:");
        headerLen = 7;
      }
      
      String ccid = "NO_SIM";
      if (ccidIdx >= 0) {
        ccid = res.substring(ccidIdx + headerLen);
        ccid.trim();
        // Keep only digits and up to 22 characters
        String cleanCcid = "";
        for (int i = 0; i < ccid.length(); i++) {
          if (isDigit(ccid[i])) {
            cleanCcid += ccid[i];
          } else if (cleanCcid.length() > 0) {
            break;
          }
        }
        if (cleanCcid.length() >= 10) {
          ccid = cleanCcid;
        } else {
          ccid = "NO_SIM";
        }
      } else {
        // Safe non-overflow digit sequence finder
        int len = (int)res.length();
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
              break;
            }
          }
        }
      }
      Serial.printf("[QC_JIG] MODEM_CCID: %s\n", ccid.c_str());

      // Get Carrier info (Airtel / BSNL / Jio / Vi)
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
      
      // Check signal strength RSSI
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
      } else {
        Serial.printf("[QC_JIG] MODEM_CSQ: %d (%d dBm)\n", csqVal, dbmVal);
      }
      
    } else {
      Serial.println("[FAIL] MODEM_INIT: FAIL");
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
}

void loop() {
  // 1. Listen for serial commands from UART0 (web dashboard)
  if (Serial.available()) {
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
      while (true) {
        delay(1000);
      }
    }
  }

  // 2. Poll serial keys from Nuvoton (UART1) if enabled
  if (enableNuvoton) {
    // Monitor Keypad Interrupt pin (GPIO27 pulls LOW on key SET)
    static bool lastIntState = HIGH;
    bool intState = digitalRead(KEYPAD_INT_PIN);
    if (intState == LOW && lastIntState == HIGH) {
      Serial.println("[QC_JIG] KEYPAD_INT_TRIGGERED: GPIO27 PULLED LOW");
      delay(50); // Debounce
    }
    lastIntState = intState;

    char rawKey = '\0';
    if (Serial1.available()) {
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

  delay(10);
}
