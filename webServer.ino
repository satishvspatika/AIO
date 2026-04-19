#include "globals.h"
#include <ESPmDNS.h>
#include <WebServer.h>

#if ENABLE_WEBSERVER == 1
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASS;
WebServer server(80);

// Helper to update activity time
void updateActivity() { last_wifi_activity_time = millis(); }

void handleRoot();
void handleData();
void handleFileList();
void handleFileDownload();
void handleFileView();
void handleViewLog();
void handleDisconnect();

void webServer(void *pvParameters) {
  esp_task_wdt_add(NULL);

  // Configure ESP32 as an Access Point dynamically with Station ID
  debugln("Configuring access point...");
  // Frequency set by task launcher in lcdkeypad.ino
  
  char ap_name[32];
  snprintf(ap_name, sizeof(ap_name), "SpatikaWeb");
  
  // Phase 10 Fix: Assure wifi_active is declared TRUE *before* spinning the radio
  // so concurrent sensor tasks don't accidentally execute ADC2 hardware reads
  // into the collapsing voltage lane!
  wifi_active = true; 
  last_wifi_activity_time = millis();

  // v5.90: Load AP password dynamically from SPIFFS; fall back to compiled default.
  char ap_pass_buf[64];
  strncpy(ap_pass_buf, AP_PASS, sizeof(ap_pass_buf) - 1);
  ap_pass_buf[sizeof(ap_pass_buf) - 1] = '\0';
  if (SPIFFS.exists("/wifi_pass.txt")) {
    if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(3000)) == pdTRUE) {
      File pf = SPIFFS.open("/wifi_pass.txt", FILE_READ);
      if (pf) {
        int n = pf.readBytes(ap_pass_buf, sizeof(ap_pass_buf) - 1);
        ap_pass_buf[n] = '\0';
        // Trim trailing whitespace / newlines
        for (int i = n - 1; i >= 0 && (ap_pass_buf[i] == '\r' || ap_pass_buf[i] == '\n' || ap_pass_buf[i] == ' '); i--)
          ap_pass_buf[i] = '\0';
        pf.close();
        debugf1("[WiFi] Custom AP pass loaded from SPIFFS: %s\n", ap_pass_buf);
      }
      xSemaphoreGive(fsMutex);
    }
  }

  // Use a completely unique password to instantly break any OS caching bugs
  WiFi.softAP(ap_name, ap_pass_buf);
  IPAddress IP = WiFi.softAPIP();
  debug("AP IP address: ");
  debugln(IP);

  if (MDNS.begin("spatika")) {
    debugln("MDNS responder started");
  }

  // Set up the web server routes
  server.on("/", []() {
    updateActivity();
    handleRoot();
  });
  server.on("/data", []() {
    updateActivity();
    handleData();
  });
  server.on("/files", []() {
    updateActivity();
    handleFileList();
  });
  server.on("/download", []() {
    updateActivity();
    handleFileDownload();
  });
  server.on("/view", []() {
    updateActivity();
    handleViewLog();
  });
  server.on("/viewfile", []() {
    updateActivity();
    handleFileView();
  });
  server.on("/extend", []() {
    // Add 60s extension (shift start time forward), max cap at now (full
    // duration)
    unsigned long now = millis();
    if (now > last_wifi_activity_time) { // Safety check
      unsigned long elapsed = now - last_wifi_activity_time;
      if (elapsed > 60000) {
        last_wifi_activity_time += 60000;
      } else {
        last_wifi_activity_time = now;
      }
    } else {
      last_wifi_activity_time = now;
    }
    server.send(200, "text/plain", "Extended 1m");
  });
  server.on("/disconnect", []() {
    updateActivity();
    handleDisconnect();
  });

  // Start the server
  server.begin();
  debugln("************** WEB SERVER STARTED **************");

  for (;;) {
    esp_task_wdt_reset();
    server.handleClient();

    // Auto-disconnect if idle for 3 minutes (180000 ms) OR if disabled
    // externally
    if ((millis() - last_wifi_activity_time > 180000) || !wifi_active) {
      if (wifi_active) {
        debugln("WiFi Idle Timeout. Disconnecting...");
      } else {
        debugln("WiFi Disabled externally. Stopping WebServer task...");
      }
      WiFi.softAPdisconnect(true);
      setCpuFrequencyMhz(80); // Back to power-save mode
      wifi_active = false;
      webServerStarted = false; // Allow re-creation if logic permits (though
                                // this kills the task)
      esp_task_wdt_delete(
          NULL);         // Unsubscribe from the Task Watchdog to prevent panic
      vTaskDelete(NULL); // Kill this task
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// --- Handle Root (/) ---
void handleRoot() { // v5.70 STREAMING
  if (!wifi_active) {
    server.send(200, "text/html",
                "<html><body><h1>WiFi is OFF</h1></body></html>");
    return;
  }

  // Kannada Translation Setup
  bool isKan = (strstr(UNIT, "KSNDMC") != NULL);

  // --- Parse Last Recorded Log Data ---
  String rec_rf = "--", rec_temp = "--", rec_hum = "--", rec_ws = "--",
         rec_wd = "--";
  // Determine the most likely active file (the one that closes at next 8:45 AM)
  int rDay = current_day, rMonth = current_month, rYear = current_year;
  int sNo = (current_hour * 4 + current_min / 15 + 61) % 96;
  if (sNo <= 60) {
    next_date(&rDay, &rMonth, &rYear);
  }

  String filesToTry[2];
  char dateStr[20];
  snprintf(dateStr, sizeof(dateStr), "%04d%02d%02d", rYear, rMonth, rDay);
  filesToTry[0] = "/" + String(station_name) + "_" + String(dateStr) + ".txt";

  // Previous file as fallback
  previous_date(&rDay, &rMonth, &rYear);
  snprintf(dateStr, sizeof(dateStr), "%04d%02d%02d", rYear, rMonth, rDay);
  filesToTry[1] = "/" + String(station_name) + "_" + String(dateStr) + ".txt";

  char timeStr[64] = "--:--";
  bool foundData = false;
  String matchLine = "";
  String matchDate = "--", matchTime = "--";

  if (current_year > 2020) {
    for (int fIdx = 0; fIdx < 2; fIdx++) {
      if (SPIFFS.exists(filesToTry[fIdx])) {
        File f = SPIFFS.open(filesToTry[fIdx], FILE_READ);
        String lastValidTime = "";
        matchLine = ""; // Reset for each file check

        while (f.available()) {
          String line = f.readStringUntil('\n');
          line.trim();
          if (line.length() < 10)
            continue;

          // Flexible parsing to find first Date and first Time in the line
          int tokenP = 0;
          String curDate = "--", curTime = "";
          for (int i = 0; i < 8; i++) {
            int nextP = line.indexOf(',', tokenP);
            String val = (nextP == -1) ? line.substring(tokenP)
                                       : line.substring(tokenP, nextP);
            val.trim();
            if (curDate == "--" && val.indexOf('-') != -1 && val.length() >= 8)
              curDate = val;
            if (curTime == "" && val.indexOf(':') != -1 && val.length() >= 4)
              curTime = val;
            if (curTime != "" && curDate != "--")
              break;
            if (nextP == -1)
              break;
            tokenP = nextP + 1;
          }

          if (curTime != "" && curTime.indexOf(':') != -1) {
            matchLine = line;
            matchDate = curDate;
            matchTime = curTime;
          }
          esp_task_wdt_reset();
        }
        f.close();

        if (matchLine != "") {
          foundData = true;
          // Dynamic CSV Parsing of the best match
          int p = 0;
          String tokens[15];
          int tokenCount = 0;
          int timeIdx = -1;

          for (int i = 0; i < 15; i++) {
            int nextP = matchLine.indexOf(',', p);
            if (nextP == -1)
              nextP = matchLine.length();
            String val = matchLine.substring(p, nextP);
            val.trim();
            tokens[i] = val;
            if (val.indexOf(':') != -1 && timeIdx == -1)
              timeIdx = i;
            tokenCount++;
            p = nextP + 1;
            if (nextP == matchLine.length())
              break;
          }

          if (timeIdx != -1) {
            int d = timeIdx + 1;
            while (d < tokenCount && tokens[d].indexOf(':') != -1) {
              d++;
            }

            bool isBihar = (strstr(station_name, "BIHAR_TRG") != NULL);

#if SYSTEM == 0
            if (isBihar) {
              rec_rf = (d < tokenCount) ? tokens[d] : "--";
            } else {
              rec_rf = (d + 1 < tokenCount) ? tokens[d + 1] : "--";
            }
#elif SYSTEM == 1
            rec_temp = (d < tokenCount) ? tokens[d] : "--";
            rec_hum = (d + 1 < tokenCount) ? tokens[d + 1] : "--";
            rec_ws = (d + 2 < tokenCount) ? tokens[d + 2] : "--";
            rec_wd = (d + 3 < tokenCount) ? tokens[d + 3] : "--";
#elif SYSTEM == 2
            rec_rf = (d < tokenCount) ? tokens[d] : "--";
            rec_temp = (d + 1 < tokenCount) ? tokens[d + 1] : "--";
            rec_hum = (d + 2 < tokenCount) ? tokens[d + 2] : "--";
            rec_ws = (d + 3 < tokenCount) ? tokens[d + 3] : "--";
            rec_wd = (d + 4 < tokenCount) ? tokens[d + 4] : "--";
#endif
          }
          snprintf(timeStr, sizeof(timeStr), "%s %s", matchDate.c_str(),
                   matchTime.c_str());
          break; // EXIT loop: We found the latest available file with data
        }
      }
    }
  }

  // Final fallback if NO data was found in either file
  if (!foundData) {
    if (last_recorded_hr == 0 && last_recorded_min == 0)
      snprintf(timeStr, sizeof(timeStr), "--:--");
    else
      snprintf(timeStr, sizeof(timeStr), "%02d:%02d", last_recorded_hr,
               last_recorded_min);
  }
  // v5.70 MEMORY FIX: Stream HTML to avoid large String allocations
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");

  server.sendContent("<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>SPATIKA SENSOR DATA</title>");
  server.sendContent("<style>body{font-family:Helvetica,Arial,sans-serif;text-align:center;background-color:#f0f8ff;color:#333;margin:0;padding:10px;}.container{max-width:800px;margin:0 auto;}.btn{background-color:#17a2b8;color:white;padding:12px 24px;text-align:center;text-decoration:none;display:inline-block;font-size:15px;margin:10px 5px;cursor:pointer;border-radius:5px;border:none;transition:background 0.2s;white-space:nowrap;}.btn:hover{opacity:0.9;}.btn-danger{background-color:#dc3545;}input[type=submit]{background-color:#007bff;color:white;border:none;padding:10px 20px;border-radius:5px;cursor:pointer;font-size:15px;transition:background 0.2s;white-space:nowrap;}input[type=submit]:hover{opacity:0.9;}.card{background:white;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.05);padding:10px;margin:5px;display:flex;flex-direction:column;justify-content:center;min-width:140px;flex:1 1 45%;box-sizing:border-box;word-wrap:break-word;white-space:normal;}.value{font-size:1.2em;font-weight:700;color:#007bff;margin-top:5px;}.label{font-size:0.85em;color:#6c757d;text-transform:uppercase;letter-spacing:0.5px;line-height:1.2;}.section-title{font-size:1.1em;color:#555;margin:20px 0 10px;font-weight:bold;text-align:left;border-bottom:2px solid #ddd;padding-bottom:5px;}@media (max-width: 400px) { .card { min-width: 45%; } }.warning-modal {display:none;position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(0,0,0,0.8);z-index:999;text-align:center;padding-top:30%;}.warning-box {background:white;padding:20px;border-radius:10px;display:inline-block;width:80%;max-width:300px;}</style>");

  server.sendContent("<script>setInterval(function() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { var data = JSON.parse(this.responseText);");
  server.sendContent("if(document.getElementById('live_rf')) document.getElementById('live_rf').innerHTML = data.rf_inst + ' <span style=\"font-size:0.6em\">mm</span>';");
  server.sendContent("if(document.getElementById('live_temp')) document.getElementById('live_temp').innerHTML = data.temperature + ' <span style=\"font-size:0.6em\">&deg;C</span>';");
  server.sendContent("if(document.getElementById('live_hum')) document.getElementById('live_hum').innerHTML = data.humidity + ' <span style=\"font-size:0.6em\">%</span>';");
  server.sendContent("if(document.getElementById('live_ws')) document.getElementById('live_ws').innerHTML = data.windSpeed + ' <span style=\"font-size:0.6em\">m/s</span>';");
  server.sendContent("if(document.getElementById('live_wd')) document.getElementById('live_wd').innerHTML = data.windDir + ' <span style=\"font-size:0.6em\">&deg;</span>';");
  server.sendContent("if(data.bat_v) document.getElementById('live_bat').innerHTML = data.bat_v + ' <span style=\"font-size:0.6em\">V</span>';");
  server.sendContent("if(data.sol_v) document.getElementById('live_sol').innerHTML = data.sol_v + ' <span style=\"font-size:0.6em\">V</span>';");
  server.sendContent("if(data.gps_lat) { if(Math.abs(data.gps_lat) < 0.0001) document.getElementById('live_gps').innerHTML = 'SEARCHING...'; else document.getElementById('live_gps').innerHTML = data.gps_lat.toFixed(6) + ', ' + data.gps_lon.toFixed(6); }");
  server.sendContent("if(data.calib) document.getElementById('live_calib').innerHTML = data.calib;");
  server.sendContent("if(data.pressure && document.getElementById('live_pres')) { var val = data.pressure.toFixed(2); if(data.mslp) val += ' | ' + data.mslp.toFixed(2); document.getElementById('live_pres').innerHTML = val + ' <span style=\"font-size:0.6em\">hPa</span>'; }");
  server.sendContent("if(data.wifi_left < 15) { document.getElementById('warnModal').style.display='block'; document.getElementById('timeLeft').innerText = data.wifi_left; } else { document.getElementById('warnModal').style.display='none'; }");
  server.sendContent("} else if (this.readyState == 4 && (this.status == 0 || this.status == 500)) { document.body.innerHTML = '<div style=\"text-align:center;margin-top:20vh;\"><h1>Offline</h1></div>'; } }; xhttp.open('GET', '/data?t=' + Date.now(), true); xhttp.send(); }, 3000);");
  server.sendContent("function extendInfo() { fetch('/extend').then(()=>{ document.getElementById('warnModal').style.display='none'; }); }</script></head><body>");

  String sysType = "";
#if SYSTEM == 0
  sysType = "<i>Varsha TRG</i>";
#elif SYSTEM == 1
  sysType = "<i>Varsha TWS</i>";
#elif SYSTEM == 2
  sysType = "<i>Varsha TWS-RF</i>";
#endif

  String stationValue =
      "<h1 style='font-size: 1.8em;color:#333;margin-bottom:5px;'>" +
      String(station_name) + "</h1>" +
      "<div style='font-size: "
      "1.0em;color:#666;font-weight:bold;margin-bottom:20px;'>( " +
      sysType + " )</div>";

  server.sendContent("<h2 style='color:#007bff;font-size:1.5em;margin-bottom:5px;'>Spatika Web Portal</h2>");
  server.sendContent(stationValue);

  // Define translation labels once
  const char* s_live = isKan ? "\xE0\xB2\xB2\xE0\xB3\x88\xE0\xB2\xB5\xE0\xB3\x8D \xE0\xB2\xAE\xE0\xB2\xBE\xE0\xB2\xB9\xE0\xB2\xBF\xE0\xB2\xA1\xE0\xB2\xBF (Live Monitor)" : "Live Monitor";
  const char* s_rf = isKan ? "\xE0\xB2\xAE\xE0\xB2\xB3\xE0\xB3\x86 (Instant RF)" : "Instant RF";
  const char* s_bat = isKan ? "\xE0\xB2\xAC\xE0\xB3\x8D\xE0\xB2\xAF\xE0\xB2\xBE\xE0\xB2\x9F\xE0\xB2\xB0\xE0\xB2\xBF (Battery)" : "Battery";
  const char* s_sol = isKan ? "\xE0\xB2\xB8\xE0\xB3\x8C\xE0\xB2\xB0 (Solar)" : "Solar";
  const char* s_gps = isKan ? "\xE0\xB2\xB8\xE0\xB3\x8D\xE0\xB2\xA5\xE0\xB2\xB2 (GPS - Lat, Lon)" : "GPS (Lat, Lon)";

  server.sendContent("<div class='container'>");

  // --- SECTION 1: LIVE MONITOR ---
  server.sendContent("<div class='section-title'>" + String(s_live) + " <span style='font-size:0.6em;color:#28a745;vertical-align:middle;'>&#9679; updating</span></div>");
  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(I2C_MUTEX_WAIT_TIME)) == pdTRUE) {
    server.sendContent("<div style='display:flex;flex-wrap:wrap;justify-content:center;gap:5px;'>");
#if SYSTEM == 0 || SYSTEM == 2
    server.sendContent("<div class='card'><div class='label'>" + String(s_rf) + "</div><div id='live_rf' class='value'>" + String((float)rf_count.val * RF_RESOLUTION, 2) + " <span style='font-size:0.6em'>mm</span></div></div>");
#endif
#if SYSTEM == 1 || SYSTEM == 2
    server.sendContent("<div class='card'><div class='label'>Temp</div><div id='live_temp' class='value'>" + String(temperature, 1) + " &deg;C</div></div>");
    server.sendContent("<div class='card'><div class='label'>Humidity</div><div id='live_hum' class='value'>" + String(humidity, 1) + " %</div></div>");
#endif

    server.sendContent("<div class='card'><div class='label'>" + String(s_bat) + "</div><div id='live_bat' class='value'>" + String(li_bat_val, 2) + " <span style='font-size:0.6em'>V</span></div></div>");
    server.sendContent("<div class='card'><div class='label'>" + String(s_sol) + "</div><div id='live_sol' class='value'>" + String(solar_val, 2) + " <span style='font-size:0.6em'>V</span></div></div>");
    
    String gps_display = (abs(gps_latitude) < 0.0001) ? "SEARCHING..." : (String(gps_latitude, 6) + ", " + String(gps_longitude, 6));
    server.sendContent("<div class='card'><div class='label'>" + String(s_gps) + "</div><div id='live_gps' class='value' style='font-size:0.9em;'>" + gps_display + "</div></div>");
    
    char clb_info[32] = "N/A";
    if (calib_year > 2000) {
        snprintf(clb_info, sizeof(clb_info), "%s (%02d/%02d/%02d)", 
                 (calib_sts == 1 ? "PASS" : "FAIL"), calib_day, calib_month, calib_year % 100);
    }
    server.sendContent("<div class='card'><div class='label'>RF Calibration</div><div id='live_calib' class='value' style='font-size:0.9em;'>" + String(clb_info) + "</div></div>");
    server.sendContent("</div>");
    xSemaphoreGive(i2cMutex);
  }

  // --- SECTION 2: LAST RECORDED ---
  server.sendContent("<div class='section-title'>Last Recorded<br><span style='font-size:0.85em;color:#777;'>@ " + String(timeStr) + "</span></div>");
  server.sendContent("<div style='display:flex;flex-wrap:wrap;justify-content:center;gap:5px;'>");
#if SYSTEM == 0 || SYSTEM == 2
  server.sendContent("<div class='card'><div class='label'>Logged RF</div><div class='value' style='color:#666'>" + rec_rf + " mm</div></div>");
#endif
  server.sendContent("</div>");

  // --- ACTIONS ---
  server.sendContent("<div class='section-title'>System Control</div>");
  server.sendContent("<div style='background:white;padding:20px;border-radius:8px;box-shadow:0 1px 3px rgba(0,0,0,0.1);max-width:500px;margin:10px auto;'>");
  server.sendContent("<a href='/files' class='btn' style='display:block;width:100%;box-sizing:border-box;'>Browse Explorer</a>");
  server.sendContent("<br><a href='#' onclick=\"if(confirm('Disconnect?')) window.location='/disconnect';\" class='btn btn-danger' style='width:auto;padding:10px 20px;'>Close WiFi</a>");
  server.sendContent("</div></div></body></html>");
  server.sendContent(""); // End
}

// --- Handle File List (/files) ---
void handleFileList() {
  String year = server.arg("year");
  String month = server.arg("month");
  String query = server.arg("search");

  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta "

                "name='viewport' "
                "content='width=device-width, initial-scale=1.0'>";
  html += "<title>LOG FILES</title>";
  html +=
      "<style>body{font-family:Helvetica,Arial,sans-serif;background:#f0f8ff;"
      "color:#333;margin:0;padding:20px;} "
      ".file-item{background:white;padding:15px;margin-bottom:12px;border-"
      "radius:8px;box-shadow:0 1px 3px "
      "rgba(0,0,0,0.1);display:flex;justify-content:space-between;align-"
      "items:"
      "center;flex-wrap:wrap;}"
      ".folder-item{background:#e9ecef;padding:20px;margin-bottom:12px;"
      "border-"
      "radius:8px;text-align:center;font-size:1.2em;font-weight:bold;cursor:"
      "pointer;color:#007bff;text-decoration:none;display:block;}"
      ".folder-item:hover{background:#dee2e6;}"
      ".btn{text-decoration:none;padding:8px "
      "16px;color:white;border-radius:5px;margin-left:5px;font-size:14px;"
      "font-"
      "weight:bold;border:none;white-space:nowrap;}"
      ".btn-view{background-color:#007bff;} "
      ".btn-dl{background-color:#28a745;}"
      "input[type=text]{padding:10px;font-size:16px;width:100%;max-width:"
      "300px;"
      "border:1px solid #ced4da;border-radius:5px;margin-right:10px;}"
      "input[type=submit]{padding:10px "
      "20px;font-size:16px;background:#007bff;color:white;border:none;border-"
      "radius:5px;cursor:pointer;}"
      "form{margin-bottom:20px;}"
      "@media (max-width: 480px) { .file-item { display:block; } .btn { "
      "display:inline-block; margin:5px 5px 0 0; } }"
      "</style></head><body>";

  bool isKan = (strstr(UNIT, "KSNDMC") != NULL);
  String s_lf = isKan ? "ಲಾಗ್ ಫೈಲ್ಸ್ (Log Files)" : "Log Files";
  String s_sy = isKan ? "ವರ್ಷ ಆಯ್ಕೆ ಮಾಡಿ (Select Year)" : "Select Year";
  String s_sm = isKan ? "ತಿಂಗಳು ಆಯ್ಕೆ ಮಾಡಿ (Select Month)" : "Select Month";
  String s_sr = isKan ? "ಫಲಿತಾಂಶಗಳು (Search Results for)" : "Search Results for";
  String s_nf = isKan ? "ಫೈಲ್ಸ್ ಸಿಗಲಿಲ್ಲ (No matching files found)"
                      : "No matching files found.";
  String s_filter = isKan ? "\xE0\xB2\xB9\xE0\xB3\x81\xE0\xB2\xA1\xE0\xB3\x81\xE0\xB2\x95\xE0\xB2\xBF (Search)" : "Search";
  String s_bhome =
      isKan ? "ಹೋಮ್ ಪೇಜ್‌ಗೆ ಹೋಗಿ (Back to Home)"
            : "Back to Home";
  String s_view = isKan ? "ನೋಡಿ (View)" : "View";
  String s_dl = isKan ? "ಡೌನ್ಲೋಡ್ (Download)" : "Download";
  String s_byears = isKan ? "ವರ್ಷಗಳಿಗೆ ಹಿಂತಿರುಗಿ (Back to Years)" : "Back to Years";

  html += "<h2>" + s_lf + "</h2>";

  // Search Form
  html += "<form action='/files' method='GET'>";
  html += "<input type='text' name='search' placeholder='Global Search (e.g. "
          "20260125)' value='" +
          query + "'>";
  html += "<input type='submit' value='" + s_filter +
          "' "
          "style='margin-top:5px;background-color:#007bff;'>";
  html += "</form>";

  // --- Logic for Hierarchy vs Search ---

  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  int count = 0;

  if (query.length() > 0) {
    // --- MODE: GLOBAL SEARCH ---
    html += "<h3>Search Results for '" + query + "'</h3>";
    html += "</div>";
    count++;
  }
  file.close(); // v5.49 Build 5: FIX LEAK
  file = root.openNextFile();

  if (count == 0 && query.length() > 0)
    html += "<p>" + s_nf + "</p>";

  if (year == "") {
    // --- MODE: YEAR LIST (ROOT) ---
    html += "<h3>" + s_sy + "</h3>";
    bool years[100] = {0}; // 2000-2099 map

    while (file) {
      String fileName = file.name();
      if (fileName.endsWith(".txt") && fileName.indexOf(station_name) != -1) {
        // Extract YYYY: Station_YYYYMMDD.txt
        int idx = fileName.lastIndexOf('_');
        if (idx != -1 && fileName.length() > idx + 4) {
          String yStr = fileName.substring(idx + 1, idx + 5);
          int yVal = yStr.toInt();
          if (yVal >= 2000 && yVal < 2100)
            years[yVal - 2000] = true;
        }
      }
      file.close();
      file = root.openNextFile();
    }

    for (int i = 0; i < 100; i++) {
      if (years[i]) {
        int yVal = 2000 + i;
        html += "<a href='/files?year=" + String(yVal) +
                "' class='folder-item'>" + String(yVal) + "</a>";
        count++;
      }
    }
    if (count == 0) html += "<p>" + s_nf + "</p>";
  } else if (year != "" && month == "") {
    // --- MODE: MONTH LIST (YEAR SELECTED) ---
    html += "<h3>" + year + " > " + s_sm + "</h3>";
    bool months[13] = {0}; // 1-12

    while (file) {
      String fileName = file.name();
      if (fileName.endsWith(".txt") && fileName.indexOf(station_name) != -1) {
        int idx = fileName.lastIndexOf('_');
        if (idx != -1 && fileName.length() > idx + 6) {
          String fYear = fileName.substring(idx + 1, idx + 5);
          if (fYear == year) {
            String mStr = fileName.substring(idx + 5, idx + 7);
            int mVal = mStr.toInt();
            if (mVal >= 1 && mVal <= 12)
              months[mVal] = true;
          }
        }
      }
      file.close();
      file = root.openNextFile();
    }

    const char *monthNames[] = {
        "", "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"};
    for (int i = 1; i <= 12; i++) {
      if (months[i]) {
        String mPad = (i < 10) ? "0" + String(i) : String(i);
        html += "<a href='/files?year=" + year + "&month=" + mPad +
                "' class='folder-item'>" + String(monthNames[i]) + " (" + mPad +
                ")</a>";
        count++;
      }
    }
    if (count == 0) html += "<p>No logs found for " + year + ".</p>";
  } else if (year != "" && month != "") {
    // --- MODE: FILE LIST (YEAR + MONTH SELECTED) ---
    html += "<h3>" + year + " > " + month + "</h3>";
    String filter = "_" + year + month; 

    while (file) {
      String fileName = file.name();
      if (fileName.endsWith(".txt") && fileName.indexOf(filter) != -1) {
        html += "<a href='/viewfile?file=" + fileName + "' class='folder-item'>" + fileName + "</a>";
        count++;
      }
      file.close();
      file = root.openNextFile();
    }
    
    if (count == 0) html += "<p>No logs found for " + year + "-" + month + ".</p>";
  }
  
  // Cleanup any active root traversers
  if (file) file.close();


html += "<br>";
if (month != "") {
  String btn_back_year =
      isKan ? "ಹಿಂದಕ್ಕೆ ಹೋಗಿ (Back to " + year + ")" : "Back to " + year;
  html += "<a href='/files?year=" + year +
          "' class='btn' style='background-color:#17a2b8;'>" + btn_back_year +
          "</a> ";
}
if (year != "")
  html += "<a href='/files' class='btn' "
          "style='background-color:#007bff;'>" +
          s_byears + "</a> ";
html += "<a href='/' class='btn' style='background-color:#28a745;'>" + s_bhome +
        "</a>";

html += "</body></html>";
server.send(200, "text/html", html);
}

// --- Handle File Download (/download?file=...) ---
// --- Handle File Download (/download?file=...) ---
void handleFileDownload() {
  if (server.hasArg("file")) {
    String fileName = server.arg("file");

    // v5.70 SECURITY FIX: Prevent path traversal (..)
    if (fileName.indexOf("..") != -1) {
       server.send(403, "text/plain", "Forbidden: Invalid Path");
       return;
    }

    if (!fileName.startsWith("/"))
      fileName = "/" + fileName;

    if (SPIFFS.exists(fileName)) {
      File file = SPIFFS.open(fileName, "r");
      // Force filename for download
      String downloadName = fileName;
      if (downloadName.startsWith("/"))
        downloadName = downloadName.substring(1);

      // Prepend Station ID for external context
      if (!downloadName.startsWith(String(station_name))) {
        downloadName = String(station_name) + "_" + downloadName;
      }

      server.sendHeader("Content-Disposition",
                        "attachment; filename=\"" + downloadName + "\"");
      server.streamFile(file, "text/plain"); // Send as text/plain
      file.close();
      return;
    }
  }
  server.send(404, "text/plain", "File Not Found");
}

// --- Handle File View (/viewfile?file=...) ---
void handleFileView() {
  if (server.hasArg("file")) {
    String fileName = server.arg("file");

    // v5.70 SECURITY FIX: Prevent path traversal (..)
    if (fileName.indexOf("..") != -1) {
       server.send(403, "text/plain", "Forbidden: Invalid Path");
       return;
    }

    if (!fileName.startsWith("/"))
      fileName = "/" + fileName;

    if (SPIFFS.exists(fileName)) {
      File file = SPIFFS.open(fileName, "r");

      // Header
      server.setContentLength(CONTENT_LENGTH_UNKNOWN);
      server.send(200, "text/html", "");
      server.sendContent(
          "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta "
          "name='viewport' "
          "content='width=device-width, initial-scale=1.0'>"
          "<style>"
          "body{font-family:Helvetica,Arial,sans-serif;padding:20px;"
          "background:"
          "#f4f4f9;color:#333;}"
          ".btn{text-decoration:none;padding:12px "
          "20px;color:white;border:none;border-radius:5px;margin:10px "
          "5px;display:inline-block;font-size:15px;cursor:pointer;font-"
          "weight:"
          "bold;text-align:center;}"
          ".btn-dl{background-color:#28a745;}"
          ".btn-copy{background-color:#17a2b8;}"
          ".btn-back{background-color:#138496;}"
          "pre{background:white;padding:15px;border-radius:5px;overflow-x:"
          "auto;"
          "border:1px solid #ddd;font-size:12px;line-height:1.4;}"
          ".tip{font-size:13px;color:#666;background:#e9ecef;padding:10px;"
          "border-radius:5px;margin-top:10px;border-left:4px solid #17a2b8;}"
          "</style>"
          "<script>"
          "function copyContent() {"
          "  var temp = document.createElement('textarea');"
          "  var pre = document.getElementById('fileContent');"
          "  temp.value = 'Station ID: " +
          String(station_name) +
          "\\n\\n' + pre.innerText;"
          "  document.body.appendChild(temp);"
          "  temp.select();"
          "  try { document.execCommand('copy'); alert('Content Copied! "
          "Switch "
          "to WhatsApp and Paste.'); } catch(e) { alert('Copy failed.'); }"
          "  document.body.removeChild(temp);"
          "}"
          "</script>"
          "</head><body>");

      bool isKan = (strstr(UNIT, "KSNDMC") != NULL);
      String s_dl =
          isKan ? "ಫೈಲ್ ಡೌನ್ಲೋಡ್ (Download File)" : "Download File (Save)";
      String s_copy = isKan ? "ಕಾಪಿ ಮಾಡಿ (Copy Content)" : "Copy Content";
      String s_tip = isKan ? "ಸಲಹೆ (Tip)" : "Tip";
      String s_tip_txt =
          isKan ? "ವಾಟ್ಸಾಪ್‌ನಲ್ಲಿ "
                  "ಕಳುಹಿಸಲು: "
                  "ಫೈಲ್ "
                  "ಡೌನ್ಲೋಡ್ "
                  "ಮಾಡಿ, "
                  "ವೈ-ಫೈ ಆಫ್ ಮಾಡಿ, ಆಮೇಲೆ ಶೇರ್ ಮಾಡಿ."
                : "To attach this file in WhatsApp: Download it, disconnect "
                  "WiFi (to enable Internet), then share it from your "
                  "Downloads folder.";

      server.sendContent("<h3>" + fileName + "</h3>");

      server.sendContent("<div>");
      server.sendContent("<a href='/download?file=" + fileName +
                         "' class='btn btn-dl'>" + s_dl + "</a>");
      server.sendContent("<button onclick='copyContent()' class='btn "
                         "btn-copy'>" +
                         s_copy + "</button>");
      server.sendContent("</div>");

      server.sendContent("<div class='tip'><strong>" + s_tip + ":</strong> " +
                         s_tip_txt + "</div><hr>");

      server.sendContent("<pre id='fileContent'>");

      // Stream content in chunks to preserve heap
      char buf[512];
      while (file.available()) {
        int len = file.readBytes(buf, sizeof(buf)-1);
        buf[len] = 0; 
        server.sendContent(buf);
      }

      // --- CSV LEGEND HEADER AT BOTTOM ---
      String legend =
          isKan ? "ಮೌಲ್ಯಗಳ ಅರ್ಥ (Values Meaning): " : "Values Meaning: ";
#if SYSTEM == 0
      legend += isKan
                    ? "ಸ್ಯಾಂಪಲ್ (SampleNo), ದಿನಾಂಕ (Date), ಸಮಯ (Time), ಇನ್ಸ್ಟ್ ಮಳೆ "
                      "(Inst RF), "
                      "ಸಂಚಿತ ಮಳೆ (Cum RF), ಸಿಗ್ನಲ್ (Signal), ಬ್ಯಾಟರಿ (Battery)"
                    : "SampleNo, Date, Time, Inst RF, Cum RF, Signal, Battery";
#elif SYSTEM == 1
      legend +=
          isKan
              ? "ಸ್ಯಾಂಪಲ್ (SampleNo), ದಿನಾಂಕ (Date), ಸಮಯ (Time), ತಾಪಮಾನ (Temp), "
                "ತೇವಾಂಶ (Humidity), ಗಾಳಿಯ ವೇಗ (Wind Spd), ಗಾಳಿಯ ದಿಕ್ಕು (Wind Dir), "
                "ಸಿಗ್ನಲ್ (Signal), ಬ್ಯಾಟರಿ (Battery)"
              : "SampleNo, Date, Time, Temp, Humidity, Wind Spd, Wind Dir, "
                "Signal, Battery";
#elif SYSTEM == 2
      legend += isKan ? "ಸ್ಯಾಂಪಲ್ (SampleNo), ದಿನಾಂಕ (Date), ಸಮಯ (Time), ಸಂಚಿತ ಮಳೆ "
                        "(Cum RF), ತಾಪಮಾನ (Temp), ತೇವಾಂಶ (Humidity), ಗಾಳಿಯ ವೇಗ "
                        "(Wind Spd), ಗಾಳಿಯ ದಿಕ್ಕು (Wind Dir), ಸಿಗ್ನಲ್ (Signal), "
                        "ಬ್ಯಾಟರಿ (Battery)"
                      : "SampleNo, Date, Time, Cum RF, Temp, Humidity, Wind "
                        "Spd, Wind Dir, Signal, Battery";
#endif
      server.sendContent("</pre>");
      server.sendContent(
          "<div style='font-size:12px;color:#666;margin-top:10px;'><strong>" +
          legend + "</strong></div>");

      String s_blist =
          isKan ? "ಲಿಸ್ಟ್‌ಗೆ ಹಿಂತಿರುಗಿ "
                  "(Back "
                  "to "
                  "List)"
                : "Back to List";
      String s_bhome =
          isKan ? "ಹೋಮ್ ಪೇಜ್‌ಗೆ ಹೋಗಿ (Back to "
                  "Home)"
                : "Back to Home";

      server.sendContent("</pre>");
      server.sendContent("<hr><a href='/files' class='btn btn-back'>" +
                         s_blist + "</a>");
      server.sendContent(" <a href='/' class='btn btn-back'>" + s_bhome +
                         "</a></body></html>");
      file.close();
      return;
    }
  }
  server.send(404, "text/plain", "File Not Found");
}

// --- Handle View Log (/view?date=...&time=...) ---
void handleViewLog() {
  String dateInput = server.arg("date"); // YYYYMMDD
  String timeInput = server.arg("time"); // HH:MM

  bool isKan = (strstr(UNIT, "KSNDMC") != NULL);
  String s_sres = isKan ? "ಫಲಿತಾಂಶ (Search Result)" : "Search Result";
  String s_dnf =
      isKan ? "ಮಾಹಿತಿ ಸಿಗಲಿಲ್ಲ (Data not found for)" : "Data not found for";
  String s_rf = isKan ? "ರೆಕಾರ್ಡ್ ಸಿಕ್ಕಿದೆ (Record Found):" : "Record Found:";
  String s_bhome =
      isKan ? "ಹೋಮ್ ಪೇಜ್‌ಗೆ ಹೋಗಿ (Back to Home)"
            : "Back to Home";

  // --- Parse the input date and time ---
  int qYear = dateInput.substring(0, 4).toInt();
  int qMonth = dateInput.substring(4, 6).toInt();
  int qDay = dateInput.substring(6, 8).toInt();

  int qHour = 0;
  int qMin = 0;
  int colonIdx = timeInput.indexOf(':');
  if (colonIdx != -1) {
    qHour = timeInput.substring(0, colonIdx).toInt();
    qMin = timeInput.substring(colonIdx + 1).toInt();
  }

  // --- Convert Real Date/Time to internal rf_close_date ---
  // If time is 00:00 to 08:30, it belongs to today's rf_close_date.
  // If time is 08:45 to 23:45, it belongs to tomorrow's rf_close_date.
  int search_rf_year = qYear;
  int search_rf_month = qMonth;
  int search_rf_day = qDay;

  bool goes_to_next_day = false;

  if (qHour > 8 || (qHour == 8 && qMin > 30)) {
    goes_to_next_day = true;
  }

  if (goes_to_next_day && search_rf_day > 0) {
    next_date(&search_rf_day, &search_rf_month, &search_rf_year);
  }

  char rfFileDate[10];
  snprintf(rfFileDate, sizeof(rfFileDate), "%04d%02d%02d", search_rf_year,
           search_rf_month, search_rf_day);

  String searchFileName =
      "/" + String(station_name) + "_" + String(rfFileDate) + ".txt";
  String result = s_dnf + " Date: " + dateInput + " Time: " + timeInput +
                  " (Searched in: " + searchFileName + ")";

  if (SPIFFS.exists(searchFileName)) {
    File f = SPIFFS.open(searchFileName, FILE_READ);
    String searchPattern =
        "," + timeInput +
        ","; // Format in CSV depends on log type but usually ,HH:MM,
    while (f.available()) {
      String line = f.readStringUntil('\n');
      if (line.indexOf(searchPattern) != -1) {

        // --- Parse CSV Line ---
        int p = 0;
        String tokens[15];
        int tokenCount = 0;

        for (int i = 0; i < 15; i++) {
          int nextP = line.indexOf(',', p);
          if (nextP == -1)
            nextP = line.length();
          String val = line.substring(p, nextP);
          val.trim();
          tokens[i] = val;
          tokenCount++;
          p = nextP + 1;
          if (nextP == line.length())
            break;
        }

        // Token 0: SampleNo
        // Token 1: Date
        // Token 2: Time

        String parsedResult = "<b>" + s_rf + "</b><br><br>";

        parsedResult += (isKan ? "ದಿನಾಂಕ (Date): " : "Date: ") +
                        (tokenCount > 1 ? tokens[1] : "--") + "<br>";
        parsedResult += (isKan ? "ಸಮಯ (Time): " : "Time: ") +
                        (tokenCount > 2 ? tokens[2] : "--") + "<br>";

        // Look for the index that holds the time to offset others (in case
        // Date is missing or extra strings)
        int d = 3; // default offset after Time
        for (int i = 0; i < tokenCount; i++) {
          if (tokens[i].indexOf(':') != -1) {
            d = i + 1;
            break;
          }
        }

        bool isBihar = (strstr(station_name, "BIHAR_TRG") != NULL);

#if SYSTEM == 0
        String p_rf = (isBihar && tokenCount > d)
                          ? tokens[d]
                          : ((tokenCount > d + 1) ? tokens[d + 1] : "--");
        parsedResult +=
            (isKan ? "ಸಂಚಿತ ಮಳೆ (Cum RF): " : "Cum RF: ") + p_rf + " mm<br>";
#elif SYSTEM == 1
        String p_temp = (tokenCount > d) ? tokens[d] : "--";
        String p_hum = (tokenCount > d + 1) ? tokens[d + 1] : "--";
        String p_ws = (tokenCount > d + 2) ? tokens[d + 2] : "--";
        String p_wd = (tokenCount > d + 3) ? tokens[d + 3] : "--";

        parsedResult +=
            (isKan ? "ತಾಪಮಾನ (Temp): " : "Temp: ") + p_temp + " &deg;C<br>";
        parsedResult +=
            (isKan ? "ತೇವಾಂಶ (Humidity): " : "Humidity: ") + p_hum + " %<br>";
        parsedResult += (isKan ? "ಗಾಳಿಯ ವೇಗ (Wind Speed): " : "Wind Speed: ") +
                        p_ws + " m/s<br>";
        parsedResult += (isKan ? "ಗಾಳಿಯ ದಿಕ್ಕು (Wind Dir): " : "Wind Dir: ") +
                        p_wd + " &deg;<br>";
        if (pressure > 300.0) {
          parsedResult +=
              (isKan ? "ವಾತಾವರಣದ ಒತ್ತಡ (Pressure): " : "Pressure: ") +
              String(pressure, 2) + " hPa<br>";
        }
#elif SYSTEM == 2
        String p_rf = (tokenCount > d) ? tokens[d] : "--";
        String p_temp = (tokenCount > d + 1) ? tokens[d + 1] : "--";
        String p_hum = (tokenCount > d + 2) ? tokens[d + 2] : "--";
        String p_ws = (tokenCount > d + 3) ? tokens[d + 3] : "--";
        String p_wd = (tokenCount > d + 4) ? tokens[d + 4] : "--";

        parsedResult +=
            (isKan ? "ಸಂಚಿತ ಮಳೆ (Cum RF): " : "Cum RF: ") + p_rf + " mm<br>";
        parsedResult +=
            (isKan ? "ತಾಪಮಾನ (Temp): " : "Temp: ") + p_temp + " &deg;C<br>";
        parsedResult +=
            (isKan ? "ತೇವಾಂಶ (Humidity): " : "Humidity: ") + p_hum + " %<br>";
        parsedResult += (isKan ? "ಗಾಳಿಯ ವೇಗ (Wind Speed): " : "Wind Speed: ") +
                        p_ws + " m/s<br>";
        parsedResult += (isKan ? "ಗಾಳಿಯ ದಿಕ್ಕು (Wind Dir): " : "Wind Dir: ") +
                        p_wd + " &deg;<br>";
#endif

        // Find Signal and Battery if they exist at the end of the line
        // Usually: ..., Signal, BatV1 [, BatV2]
        if (tokenCount >= d + 2) {
          String p_bat = tokens[tokenCount - 1];
          String p_sig = tokens[tokenCount - 2];
          // Just double check we aren't grabbing sensor values instead of
          // signal/bat if formatting is tight
          if (p_sig.indexOf('-') != -1 || p_sig.length() <= 4) {
            // Seems like a signal
            parsedResult += String("<br>") +
                            (isKan ? "ಸಿಗ್ನಲ್ (Signal): " : "Signal: ") + p_sig +
                            " dBm<br>";
            parsedResult += String(isKan ? "ಬ್ಯಾಟರಿ (Battery): " : "Battery: ") +
                            p_bat + " V<br>";
          }
        }

        result = parsedResult;
        break;
      }
    }
    f.close();
  }

  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta "
                "name='viewport' "
                "content='width=device-width, initial-scale=1.0'>"
                "<style>"
                "body{font-family:Helvetica,Arial,sans-serif;padding:20px;"
                "background:#f8f9fa;color:#333;}"
                ".btn{background-color:#17a2b8;color:white;padding:12px "
                "24px;text-decoration:none;display:inline-block;border-radius:"
                "5px;font-size:15px;margin-top:20px;}"
                ".btn:hover{background-color:#138496;}"
                "</style></head><body>";
  html += "<h2>" + s_sres + "</h2>";
  html += "<p>" + result + "</p>";
  html += "<br><a href='/' class='btn' style='background:#17a2b8;'>" + s_bhome +
          "</a>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// --- Handle Disconnect ---
void handleDisconnect() {
  bool isKan = (strstr(UNIT, "KSNDMC") != NULL);
  String dmsg = isKan ? "ವೈ-ಫೈ ಆಫ್ ಆಗುತ್ತಿದೆ... (WiFi Disconnecting!)"
                      : "WiFi Disconnecting...";
  server.send(200, "text/html",
              "<!DOCTYPE html><html><head><meta charset='UTF-8'></head><body>"
              "<h1>" +
                  dmsg +
                  "</h1><p>You can now close this "
                  "tab.</p><script>setTimeout(function(){window.close();}, "
                  "2000);</script></body></html>");

  vTaskDelay(1000 / portTICK_PERIOD_MS); // Allow HTTP response to flush
  WiFi.softAPdisconnect(true);
  wifi_active = false;
  webServerStarted = false;

  // Update LCD directly so it jumps back to Station ID
  strcpy(ui_data[FLD_WIFI_ENABLE].bottomRow, "ENABLE WIFI     ");

  cur_fld_no = 0; // FLD_STATION
  show_now = 1;
  lcdkeypad_start = 1; // Wake up UI loop if it was snoozing

  esp_task_wdt_delete(
      NULL); // Unsubscribe from the Task Watchdog to prevent panic
  vTaskDelete(NULL);
}

void handleData() {
  String json = "{";
  // v5.87 Hardening: [M-01] Ghost-Lock Removal (Read-Only Global Access)
  json += "\"time\": \"" + String(current_hour) + ":" + String(current_min) + "\"";

#if SYSTEM == 0
    json += ", \"rf_inst\": " + String((float)rf_count.val * RF_RESOLUTION, 2);
#endif

#if SYSTEM == 1
    json += ", \"temperature\": " + String(temperature, 1) +
            ", \"humidity\": " + String(humidity, 1) +
            ", \"windSpeed\": " + String(cur_wind_speed, 2) +
            ", \"windDir\": " + String(windDir);
    if (bmeType != BME_UNKNOWN && pressure > 300.0) {
      json += ", \"pressure\": " + String(pressure, 2) +
              ", \"mslp\": " + String(sea_level_pressure, 2);
    }
#endif

#if SYSTEM == 2
    json += ", \"rf_inst\": " + String((float)rf_count.val * RF_RESOLUTION, 2) +
            ", \"rf_cum\": " + String(new_current_cumRF, 2) +
            ", \"temperature\": " + String(temperature, 1) +
            ", \"humidity\": " + String(humidity, 1) +
            ", \"windSpeed\": " + String(cur_wind_speed, 2) +
            ", \"windDir\": " + String(windDir);
    if (bmeType != BME_UNKNOWN && pressure > 300.0) {
      json += ", \"pressure\": " + String(pressure, 2) +
              ", \"mslp\": " + String(sea_level_pressure, 2);
    }
#endif

    unsigned long elapsed = millis() - last_wifi_activity_time;
    long left = (elapsed < 180000) ? (180000 - elapsed) / 1000 : 0;
    json += ", \"bat_v\": " + String(li_bat_val, 2);
    json += ", \"sol_v\": " + String(solar_val, 2);
    json += ", \"gps_lat\": " + String(gps_latitude, 8);
    json += ", \"gps_lon\": " + String(gps_longitude, 8);
    
    char clb_json[48] = "N/A";
    if (calib_year > 2000) {
        snprintf(clb_json, sizeof(clb_json), "%s (%02d/%02d/%02d)", 
                 (calib_sts == 1 ? "PASS" : "FAIL"), calib_day, calib_month, calib_year % 100);
    }
    json += ", \"calib\": \"" + String(clb_json) + "\"";

    // Lock removed in v5.87 to prevent UI lag
  json += "}";
  server.send(200, "application/json", json);
}
#endif
