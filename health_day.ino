// =============================================================================================
// Daily health report (v6.57), as the LPC units send it (AIO_LPC design 7.7, v6.9.62): once a day
// the OTA check carries the last whole MET day (08:45 to 08:30, named by its closing date) as
// extra fields. The office page (lpc_ota.py HEALTH_FIELDS / HEALTH_RULES) stores and checks them.
// It replaces the separate Spatika health server (Contabo), which is no longer used.
//
//   day  YYYYMMDD    the MET day reported (its closing date)
//   sent, bk         records of that day delivered (in their own slot or later), and of them by backlog
//   bd, bn, bmin     battery average 10:00-16:00, 22:00-04:00, and lowest, that day (V)
//   ne, err          live sends that failed that day, and the newest failure reason
//   dur              average time awake per wake-up that day (s)
//   bv, sv, sig, net, op, q, sd, gps   now: battery, solar, signal (dBm), 2/4 G, operator,
//                    records waiting, SD card, position
//   rst, wdt, clk    fault and watchdog restarts, and the last clock correction (s), since the last report
//   pon              v6.61: power-ons since the last report (battery changed or ran flat, power cut)
//   hs               OK, or codes joined by '.' (BROWNOUT, RTC_FAIL, NO_GPS, STOPPED, RAIN_SPIKE, ...)
//   sens             TWS units: sensor state (TH-OK,WS-OK,WD-OK or ...-ERR)
//
// Kept in RTC memory (over deep sleep and restarts; a power cut starts it afresh).
// =============================================================================================


#include "esp_ota_ops.h"   // esp_app_get_description()
#include <Preferences.h>

// v6.61: "HD5" + the record's size: firmware with the same layout keeps the figures over an update
// (was: the same image only, so the first report after every OTA was a part day)
#define V657_RTC_MAGIC (0x48443500UL ^ (uint32_t)sizeof(v657_rtc_t))
RTC_NOINIT_ATTR v657_rtc_t v657_rtc;
static hd_day_t &hd_cur = v657_rtc.cur;
static hd_day_t &hd_prev = v657_rtc.prev;
static uint32_t &hd_reported_day = v657_rtc.reported_day;
static uint16_t &hd_rst = v657_rtc.rst;
static uint16_t &hd_wdt = v657_rtc.wdt;
static long &hd_clk = v657_rtc.clk;

static uint32_t hd_image_id() {
  const esp_app_desc_t *d = esp_app_get_description();
  return ((uint32_t)d->app_elf_sha256[0] << 24) | ((uint32_t)d->app_elf_sha256[1] << 16) |
         ((uint32_t)d->app_elf_sha256[2] << 8) | d->app_elf_sha256[3];
}

// MET day of a record time: 08:45 and later belong to the day that closes at 08:30 tomorrow
static uint32_t hd_met_day(int y, int mo, int d, int hh, int mi) {
  if (y < 2024 || mo < 1 || mo > 12 || d < 1 || d > 31) return 0;
  DateTime t(y, mo, d, 0, 0, 0);
  if (hh * 60 + mi > 8 * 60 + 30) t = t + TimeSpan(1, 0, 0, 0);
  return (uint32_t)t.year() * 10000UL + t.month() * 100UL + t.day();
}

// The day's record, rolling over to a new day when a later one comes; NULL for an older day
static hd_day_t *hd_for(uint32_t day) {
  if (!day) return NULL;
  if (day == hd_cur.day) return &hd_cur;
  if (day == hd_prev.day) return &hd_prev;
  if (day > hd_cur.day) {
    if (hd_cur.day) hd_prev = hd_cur;
    memset(&hd_cur, 0, sizeof(hd_cur));
    hd_cur.day = day;
    hd_cur.bmin = 99.0f;
    return &hd_cur;
  }
  return NULL;
}

static uint32_t hd_now_day() {
  int y, mo, d, h, mi;
  portENTER_CRITICAL(&rtcTimeMux);
  y = current_year; mo = current_month; d = current_day; h = current_hour; mi = current_min;
  portEXIT_CRITICAL(&rtcTimeMux);
  return hd_met_day(y, mo, d, h, mi);
}

// v6.58: the figures are also kept in NVS (saved at every deep sleep and after a report), so a
// power-on (battery change, SD card swap with the power off, power cut) no longer loses the day and
// the next day's report (seen 21/11/2026 on 001992). New firmware still starts afresh (image check).
static void health_save() {
  Preferences p;
  if (p.begin("health", false)) {
    p.putBytes("rtc", &v657_rtc, sizeof(v657_rtc));
    p.end();
  }
}

static bool health_load() {
  Preferences p;
  bool ok = false;
  if (p.begin("health", true)) {
    v657_rtc_t tmp;
    ok = p.getBytes("rtc", &tmp, sizeof(tmp)) == sizeof(tmp) && tmp.magic == V657_RTC_MAGIC;
    if (ok) v657_rtc = tmp;
    p.end();
  }
  return ok;
}

// Once per start: restarts that were faults (RTC memory survives them)
void health_note_boot() {
  const esp_reset_reason_t r = esp_reset_reason();
  // RTC copy usable unless power-on (RTC_NOINIT memory holds anything) or other firmware left it
  const bool rtc_ok = (r != ESP_RST_POWERON && v657_rtc.magic == V657_RTC_MAGIC);
  if (!rtc_ok && health_load()) {
    debugln("[Health] Daily figures restored from flash (RTC memory was lost).");
    if (r == ESP_RST_POWERON) v657_rtc.pon++;
    if (r == ESP_RST_BROWNOUT || r == ESP_RST_PANIC) hd_rst++;
  } else if (!rtc_ok) {
    memset(&v657_rtc, 0, sizeof(v657_rtc));
    v657_rtc.magic = V657_RTC_MAGIC;
    v657_rtc.image = hd_image_id();
    if (r == ESP_RST_POWERON) v657_rtc.pon = 1;
    debugln("[Health] Daily figures start afresh (nothing saved).");
  } else if (r == ESP_RST_PANIC || r == ESP_RST_BROWNOUT) {
    hd_rst++;
  } else if (r == ESP_RST_INT_WDT || r == ESP_RST_TASK_WDT || r == ESP_RST_WDT) {
    hd_wdt++;
  }
}

// A slot's record was made ("NN,YYYY-MM-DD,HH:MM,...")
void health_note_slot(const char *record, float bat) {
  int sn, y, mo, d, hh, mi;
  if (!record || sscanf(record, "%d,%d-%d-%d,%d:%d", &sn, &y, &mo, &d, &hh, &mi) != 6) return;
  const uint32_t day = hd_met_day(y, mo, d, hh, mi);
  // v6.61: records are made in time order, so a record of an older day than the current one means the
  // clock went back (a wrong date put right). Without this every later day counted as "older" and the
  // reports stopped for good.
  if (day && hd_cur.day && day < hd_cur.day) {
    memset(&hd_cur, 0, sizeof(hd_cur));
    memset(&hd_prev, 0, sizeof(hd_prev));
    hd_reported_day = 0;
    debugf("[Health] Clock went back: day figures start again at %lu\n", (unsigned long)day);
  }
  hd_day_t *t = hd_for(day);
  if (!t || bat <= 0.5f || bat > 20.0f) return;
  if (hh >= 10 && hh < 16) { t->bd_sum += bat; t->bd_n++; }
  if (hh >= 22 || hh < 4) { t->bn_sum += bat; t->bn_n++; }
  if (bat < t->bmin) t->bmin = bat;
}

// A record reached the data server (its own time), live or from the backlog
void health_note_delivered(int y, int mo, int d, int hh, int mi, bool backlog) {
  hd_day_t *t = hd_for(hd_met_day(y, mo, d, hh, mi));
  if (!t) return;
  t->sent++;
  if (backlog) t->bk++;
}

// The slot's own record could not be sent (it goes to the backlog)
void health_note_failed(const char *why) {
  hd_day_t *t = hd_for(hd_now_day());
  if (!t) return;
  t->ne++;
  snprintf(t->err, sizeof(t->err), "%s", (why && why[0]) ? why : "SEND_FAIL");
}

// Time awake this wake-up, at the start of deep sleep
void health_note_awake(unsigned long ms) {
  hd_day_t *t = hd_for(hd_now_day());
  if (t) {
    t->awake_s += (ms + 500) / 1000;
    t->awake_n++;
  }
  health_save();   // v6.58: called at every deep sleep, so the day's figures survive a power-on
}

// The clock was moved by this many seconds (server correction or SMS SET_TIME)
void health_note_clock_change(long seconds) {
  hd_clk = seconds;
}

// ---------------------------------------------------------------- the report
// Values: letters, digits and . , : - _ only (the server takes [0-9A-Za-z._,:+ -]); a space goes as %20
static void hd_add(char *out, size_t n, const char *key, const char *val) {
  size_t len = strlen(out);
  if (len + strlen(key) + 3 >= n) return;
  len += snprintf(out + len, n - len, "&%s=", key);
  for (const char *p = val; *p && len + 4 < n; p++) {
    const char c = *p;
    if (isalnum((unsigned char)c) || c == '.' || c == ',' || c == ':' || c == '-' || c == '_') {
      out[len++] = c;
    } else if (c == ' ') {
      memcpy(out + len, "%20", 3);
      len += 3;
    }
  }
  out[len] = '\0';
}

static void hd_addf(char *out, size_t n, const char *key, const char *fmt, ...) {
  char v[48];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(v, sizeof(v), fmt, ap);
  va_end(ap);
  hd_add(out, n, key, v);
}

// True when the check now should carry the report: the last whole day not yet reported, and the
// day check's time (12:30 to 08:44 next morning: the 13:00 check, else the 01:00 one), so that day's
// late backlog has had the morning to go
bool health_report_due() {
  if (!hd_prev.day || hd_prev.day == hd_reported_day) return false;
  int h, mi;
  portENTER_CRITICAL(&rtcTimeMux);
  h = current_hour; mi = current_min;
  portEXIT_CRITICAL(&rtcTimeMux);
  const int m = h * 60 + mi;
  return m >= 12 * 60 + 30 || m < 8 * 60 + 45;
}

void health_append_report(char *out, size_t n) {
  const hd_day_t *t = &hd_prev;
  char hs[64] = "";
  auto code = [&](const char *c) {
    if (strlen(hs) + strlen(c) + 2 < sizeof(hs)) {
      if (hs[0]) strcat(hs, ".");
      strcat(hs, c);
    }
  };
  if (diag_last_reset_reason == 15) code("BROWNOUT");
  if (!diag_rtc_battery_ok || current_year < 2025) code("RTC_FAIL");
  if (lati == 0.0 && longi == 0.0) code("NO_GPS");
  if (live_post_muted) code("STOPPED");
  if (diag_rain_jump) code("RAIN_SPIKE");
  if (diag_rain_reset) code("RAIN_RESET");
  if (diag_rain_calc_invalid) code("RAIN_CALC");
#if SYSTEM == 1 || SYSTEM == 2 || SYSTEM == 3
  if (diag_temp_cv || diag_hum_cv) code("TH_STUCK");
  if (diag_temp_erv || diag_hum_erv || diag_temp_erz || diag_hum_erz) code("TH_UNREAL");
  if (diag_ws_cv || diag_ws_erv) code("WS_FAULT");
  if (diag_wd_fail) code("WD_FAIL");
#endif
  hd_add(out, n, "hs", hs[0] ? hs : "OK");
  hd_addf(out, n, "day", "%lu", (unsigned long)t->day);
  hd_addf(out, n, "sent", "%u", t->sent);
  hd_addf(out, n, "bk", "%u", t->bk);
  if (t->bd_n) hd_addf(out, n, "bd", "%.2f", t->bd_sum / t->bd_n);
  if (t->bn_n) hd_addf(out, n, "bn", "%.2f", t->bn_sum / t->bn_n);
  if (t->bmin < 90.0f) hd_addf(out, n, "bmin", "%.2f", t->bmin);
  hd_addf(out, n, "ne", "%u", t->ne);
  if (t->err[0]) hd_add(out, n, "err", t->err);
  if (t->awake_n) hd_addf(out, n, "dur", "%lu", (unsigned long)(t->awake_s / t->awake_n));
  hd_addf(out, n, "bv", "%.2f", li_bat_val);
  hd_addf(out, n, "sv", "%.2f", solar_val);
  if (signal_strength < 0 && signal_strength > -120) hd_addf(out, n, "sig", "%d", (int)signal_strength);
  hd_add(out, n, "net", isLTE ? "4" : "2");
  if (carrier[0]) hd_add(out, n, "op", carrier);
  hd_addf(out, n, "q", "%d", get_total_backlogs(false));
  hd_add(out, n, "sd", sd_card_ok ? "1" : "0");
  hd_addf(out, n, "rst", "%u", hd_rst);
  hd_addf(out, n, "wdt", "%u", hd_wdt);
  hd_addf(out, n, "clk", "%ld", hd_clk);
  hd_addf(out, n, "pon", "%u", v657_rtc.pon);
  if (lati != 0.0 || longi != 0.0) hd_addf(out, n, "gps", "%.5f,%.5f", lati, longi);
#if SYSTEM == 1 || SYSTEM == 2 || SYSTEM == 3
  hd_addf(out, n, "sens", "TH-%s,WS-%s,WD-%s",
          (diag_temp_cv || diag_hum_cv || diag_temp_erv || diag_hum_erv || diag_temp_erz || diag_hum_erz) ? "ERR" : "OK",
          (diag_ws_cv || diag_ws_erv) ? "ERR" : "OK", diag_wd_fail ? "ERR" : "OK");
#endif
}

// The server answered the check that carried the report
void health_report_sent() {
  hd_reported_day = hd_prev.day;
  hd_rst = hd_wdt = 0;
  v657_rtc.pon = 0;
  hd_clk = 0;
  health_save();
  debugf("[Health] Day %lu reported.\n", (unsigned long)hd_reported_day);
}
