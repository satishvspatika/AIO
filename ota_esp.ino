/* v6.56: Firmware over the air, the LPC way (AIO_LPC ota.c, server lpc_ota.py).
 *
 * Once a day after the slot's record has gone (every slot while a download is under way, or
 * every slot on a bench unit with /ota_now.txt), the unit asks the server
 *   GET /lpc/check?id=E<stn>&p=<type>&cc=00000000&v=<version>&c=<running image>&s=<0|1|3>&t=<failed>
 *       &w=<0 | b<battery V> | q<records waiting>> (v6.60: can it download now, for the office page)
 *       [&e=<C|N|B|T|K|D>] (v6.61: with s=3, why that image was given up)
 * and gets "OTA NONE" or "OTA FW <bytes> <file crc> <image id>". It then fetches the .bin in
 * pieces of up to OTA_PIECE_MAX bytes (smaller after failures),
 *   GET /lpc/fw?id=E<stn>&c=<file crc>&o=<offset>&n=<bytes>  ->  "@<crc32 of piece>\n" + piece
 * each checked, straight into the spare app partition. How far it got is kept in NVS, so a
 * download goes on at the next slot. When the whole file is in, its CRC-32 is checked, the
 * bootloader is told to boot it (esp_ota_set_boot_partition checks the image too) and the unit
 * restarts.
 *
 * The new image runs on trial. It is kept once it has delivered a record. It goes back to the old
 * firmware (the slot it was installed from) after OTA_MAX_TRIAL_CRASHES crash restarts, or if it
 * has delivered nothing after OTA_CONFIRM_SLOTS slots. This is done by the firmware itself (the
 * bootloader on the units in the field has no rollback, and OTA cannot change a bootloader), very
 * early in setup(), so an image that crashes later in start-up is still caught.
 * The next check tells the server what happened (s=3, t=<image> for "went back").
 *
 * The image is known by the first 4 bytes of its ELF SHA-256 (esp_app_desc_t), which the server
 * reads from the uploaded .bin. The station type is compiled into the AIOOTA1 block below, from
 * UNIT_CFG, SYSTEM and USE_NUVOTON_UI, by the same rules as UNIT_VER.
 */
#include "globals.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include <Preferences.h>

#define OTA_PIECE_MAX 4096U             // server allows up to 4096
#define OTA_PIECE_MIN 1024U             // after failed pieces the size halves down to this
#define OTA_STOP_BEFORE_SLOT_S 120      // no new piece this close to the next slot boundary (the unit
                                        // wakes 2 min after it, so the next slot's record is not delayed)
#define OTA_INSTALL_NEEDS_S 120         // check + restart need this much before the boundary
#define OTA_CONFIRM_SLOTS 8             // a new image that delivers nothing in 2 h goes back
#define OTA_MAX_TRIAL_CRASHES 3         // crash restarts (panic, watchdog) of a new image before it goes back
#define OTA_MIN_BATTERY_V 3.7f          // GPRS battery: no download below this
#define OTA_RESP_MAX (OTA_PIECE_MAX + 1024) // headers + "@xxxxxxxx\n" + piece

// ---------------------------------------------------------------- station type (compile time)
static constexpr bool cx_has(const char *s, const char *w) {
  for (; *s; s++) {
    const char *a = s, *b = w;
    while (*a && *b && *a == *b) { a++; b++; }
    if (!*b) return true;
  }
  return false;
}
// Same order and rules as the UNIT_VER selection in setup(). 0 = no OTA type.
static constexpr int ota_type_of(const char *u, int sys, int nuv) {
  return (cx_has(u, "KSNDMC_OLD") && sys == 0) ? 0
       : (cx_has(u, "KSNDMC_TRG") && sys == 0) ? 10 + !nuv
       : ((cx_has(u, "BIHAR_TRG") || cx_has(u, "BIHAR_TEST")) && sys == 0) ? 12 + !nuv
       : (cx_has(u, "KSNDMC_TWS") && sys == 1) ? (cx_has(u, "-AP") ? 16 : 14) + !nuv
       : (cx_has(u, "KSNDMC_ADDON") && sys == 2) ? 18 + !nuv
       : ((cx_has(u, "SPATIKA_GEN") || cx_has(u, "SPATIKA_TRG")) && sys == 0) ? 20 + !nuv
       : ((cx_has(u, "SPATIKA_GEN") || cx_has(u, "SPATIKA_TWS")) && sys == 1) ? 22 + !nuv
       : ((cx_has(u, "SPATIKA_GEN") || cx_has(u, "SPATIKA_ADDON")) && sys == 2) ? 24 + !nuv
       : ((cx_has(u, "SPATIKA_GEN") || cx_has(u, "SPATIKA_TWSRP")) && sys == 3) ? 26 + !nuv
       : 0;
}

struct OtaIdent {
  char magic[8];      // "AIOOTA1\0": the server finds the block by this
  int32_t type;
  char version[16];
};
extern "C" const OtaIdent aio_ota_ident __attribute__((used, aligned(4))) = {
    "AIOOTA1", ota_type_of(UNIT_CFG, SYSTEM, USE_NUVOTON_UI), FIRMWARE_VERSION};
// Read through a volatile pointer: a direct read is folded to a constant at compile time and the
// linker then drops the block, which the server needs to find in the .bin.
static const OtaIdent *volatile ota_ident_ref = &aio_ota_ident;
static int ota_type() { return ota_ident_ref->type; }

// Arduino core: the new image is confirmed by us (ota_confirm_step), not at start-up
bool verifyRollbackLater() { return true; }

// ---------------------------------------------------------------- state
struct OtaState {
  uint32_t file_crc, image_id, size, have;
  char phase;         // '-' idle, 'D' downloading, 'I' install started, 'T' new image on trial,
                      // 'F' that image failed here (not taken again)
  uint32_t prev;      // flash address of the app slot the old firmware is in (for going back)
  uint32_t crashes;   // crash restarts while on trial (phase D: whole-file CRC failures, v6.61)
  char why;           // v6.61: why phase F (OTA_WHY_*), sent as e= so the office can say it
};
static OtaState st;
// v6.61: why an image was given up (the office page words them; DWLR and LPC send the same letters)
#define OTA_WHY_CRASH   'C'   // crashed again and again on trial: went back
#define OTA_WHY_NODATA  'N'   // delivered no record for 2 h on trial: went back
#define OTA_WHY_NOBOOT  'B'   // the new image did not start (the boot check refused it)
#define OTA_WHY_TYPE    'T'   // for another station type: not installed
#define OTA_WHY_CHECK   'K'   // failed the unit's image check: not installed
#define OTA_WHY_DAMAGED 'D'   // arrived damaged 3 times (whole-file CRC): not fetched again
#define OTA_MAX_REFETCH 3
static Preferences ota_prefs;
static uint8_t ota_buf[OTA_RESP_MAX + 16];
static int ota_buf_len = 0;
static bool ota_sock_open = false;

static void ota_load() {
  st = {0, 0, 0, 0, '-', 0, 0, 0};
  if (ota_prefs.begin("espota", true)) {
    st.file_crc = ota_prefs.getUInt("fcrc", 0);
    st.image_id = ota_prefs.getUInt("icrc", 0);
    st.size = ota_prefs.getUInt("size", 0);
    st.have = ota_prefs.getUInt("have", 0);
    st.phase = (char)ota_prefs.getUChar("phase", '-');
    st.prev = ota_prefs.getUInt("prev", 0);
    st.crashes = ota_prefs.getUInt("crashes", 0);
    st.why = (char)ota_prefs.getUChar("why", 0);
    ota_prefs.end();
  }
  if (!strchr("-DITF", st.phase)) st.phase = '-';
}

static void ota_save() {
  if (ota_prefs.begin("espota", false)) {
    ota_prefs.putUInt("fcrc", st.file_crc);
    ota_prefs.putUInt("icrc", st.image_id);
    ota_prefs.putUInt("size", st.size);
    ota_prefs.putUInt("have", st.have);
    ota_prefs.putUChar("phase", (uint8_t)st.phase);
    ota_prefs.putUInt("prev", st.prev);
    ota_prefs.putUInt("crashes", st.crashes);
    ota_prefs.putUChar("why", (uint8_t)st.why);
    ota_prefs.end();
  }
}

// ---------------------------------------------------------------- server address
// Built in (OTA_SERVER_HOST / OTA_SERVER_TCP_PORT), or set by SMS (VARSHA SET_OTA_SERVER host [port],
// OFF for the built-in one; v6.57, as the LPC unit's OTASRV.DAT). Kept in NVS "ota-srv".
static char ota_srv_host[48] = "";
static int ota_srv_port = 0;

static void ota_server_load() {
  if (ota_srv_port) return;
  snprintf(ota_srv_host, sizeof(ota_srv_host), "%s", OTA_SERVER_HOST);
  ota_srv_port = OTA_SERVER_TCP_PORT;
  Preferences p;
  if (p.begin("ota-srv", true)) {
    char h[48] = "";
    p.getString("host", h, sizeof(h));
    const int port = p.getInt("port", 0);
    p.end();
    if (h[0] && port > 0 && port < 65536) {
      snprintf(ota_srv_host, sizeof(ota_srv_host), "%s", h);
      ota_srv_port = port;
    }
  }
}

const char *ota_server_host() { ota_server_load(); return ota_srv_host; }
int ota_server_port() { ota_server_load(); return ota_srv_port; }

// host NULL: back to the built-in address
bool ota_server_set(const char *host, int port) {
  Preferences p;
  if (!p.begin("ota-srv", false)) return false;
  bool ok;
  if (host) {
    ok = p.putString("host", host) > 0 && p.putInt("port", port) > 0;
  } else {
    ok = p.clear();
  }
  p.end();
  ota_srv_port = 0;            // read again at the next use
  return ok;
}

static uint32_t ota_crc32(uint32_t crc, const uint8_t *p, size_t n) {   // zlib's CRC-32
  crc = ~crc;
  while (n--) {
    crc ^= *p++;
    for (int b = 0; b < 8; b++) crc = (crc >> 1) ^ (0xEDB88320UL & (0UL - (crc & 1UL)));
  }
  return ~crc;
}

static uint32_t running_image_id() {
  const esp_app_desc_t *d = esp_app_get_description();
  return ((uint32_t)d->app_elf_sha256[0] << 24) | ((uint32_t)d->app_elf_sha256[1] << 16) |
         ((uint32_t)d->app_elf_sha256[2] << 8) | d->app_elf_sha256[3];
}

static int seconds_to_boundary() {
  int mi, se;
  portENTER_CRITICAL(&rtcTimeMux);
  mi = current_min;
  se = current_sec;
  portEXIT_CRITICAL(&rtcTimeMux);
  return (15 - (mi % 15)) * 60 - se;
}

static void ota_station_id(char *out, size_t n) {
  char digits[16] = "";
  const char *src = (strlen(ftp_station) > 0) ? ftp_station : station_name;
  if (strlen(src) == 4 && isDigitStr(src)) snprintf(digits, sizeof(digits), "00%s", src);
  else snprintf(digits, sizeof(digits), "%s", src);
  snprintf(out, n, "E%.10s", digits);
}

// ---------------------------------------------------------------- one HTTP GET (keep-alive)
// Reads the modem stream: "+IPD<n>\r\n" followed by n raw bytes (binary safe), anything else is a
// URC. Returns the body length, or -1. *status gets the HTTP status.
static void ota_sock_close() {
  if (ota_sock_open) {
    SerialSIT.println("AT+CIPCLOSE=0");
    waitForResponse("OK", 2000);
  }
  ota_sock_open = false;
}

static bool ota_sock_connect() {
  SerialSIT.println("AT+NETOPEN?");
  waitForResponse("OK", 2000);
  if (strstr(modem_response_buf, "+NETOPEN: 1") == NULL) {
    SerialSIT.println("AT+NETOPEN");
    waitForResponseNoFlush("+NETOPEN:", 15000);
  }
  SerialSIT.println("AT+CIPCLOSE=0");
  waitForResponse("OK", 1000);
  char cmd[96];
  snprintf(cmd, sizeof(cmd), "AT+CIPOPEN=0,\"TCP\",\"%s\",%d", ota_server_host(), ota_server_port());
  SerialSIT.println(cmd);
  ota_sock_open = waitForResponse("+CIPOPEN: 0,0", 20000);
  return ota_sock_open;
}

static int ota_http_get(const char *path, const uint8_t **body, int *status) {
  *status = 0;
  *body = NULL;
  for (int attempt = 0; attempt < 2; attempt++) {
    if (!ota_sock_open && !ota_sock_connect()) continue;
    char req[900];             // v6.57: the day check carries the health report
    const int len = snprintf(req, sizeof(req), "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: aio9\r\n\r\n", path,
                             ota_server_host());
    if (len <= 0 || len >= (int)sizeof(req)) return -1;
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=0,%d", len);
    flushSerialSIT();
    SerialSIT.println(cmd);
    if (!waitForResponse(">", 5000)) {
      ota_sock_close();
      continue;
    }
    SerialSIT.write((const uint8_t *)req, len);

    // Collect the HTTP response from +IPD frames
    ota_buf_len = 0;
    char line[48];
    int ll = 0;
    int ipd_left = 0;
    bool closed = false;
    int hdr_end = -1, content_len = -1;
    const uint32_t t0 = millis();
    uint32_t last_wdt = t0;
    while (millis() - t0 < 20000) {
      if (!SerialSIT.available()) {
        vTaskDelay(2 / portTICK_PERIOD_MS);
        continue;
      }
      const uint8_t c = (uint8_t)SerialSIT.read();
      if (ipd_left > 0) {
        if (ota_buf_len < OTA_RESP_MAX) ota_buf[ota_buf_len++] = c;
        ipd_left--;
      } else if (c == '\n') {
        line[ll] = '\0';
        if (strncmp(line, "+IPD", 4) == 0) ipd_left = atoi(line + 4);
        else if (strstr(line, "+IPCLOSE") || strstr(line, "CLOSED")) closed = true;
        ll = 0;
      } else if (c != '\r' && ll < (int)sizeof(line) - 1) {
        line[ll++] = (char)c;
      }
      if (hdr_end < 0 && ota_buf_len >= 4) {
        ota_buf[ota_buf_len] = 0;
        const char *e = strstr((const char *)ota_buf, "\r\n\r\n");
        if (e) {
          hdr_end = (e - (const char *)ota_buf) + 4;
          const char *cl = strstr((const char *)ota_buf, "Content-Length:");
          if (!cl) cl = strstr((const char *)ota_buf, "content-length:");
          content_len = cl ? atoi(cl + 15) : -1;
        }
      }
      if (hdr_end >= 0 && content_len >= 0 && ota_buf_len >= hdr_end + content_len) break;
      if (closed && ipd_left == 0) break;
      if (millis() - last_wdt > 2000) { esp_task_wdt_reset(); last_wdt = millis(); }   // v6.61: was % 2000 == 0 (hit by luck)
    }
    esp_task_wdt_reset();
    if (closed) ota_sock_open = false;
    if (hdr_end < 0 || strncmp((const char *)ota_buf, "HTTP/1.", 7) != 0) {
      ota_sock_close();
      continue;   // nothing usable: reconnect once
    }
    *status = atoi((const char *)ota_buf + 9);
    const int blen = (content_len >= 0) ? content_len : ota_buf_len - hdr_end;
    if (hdr_end + blen > ota_buf_len) {
      ota_sock_close();
      continue;   // cut short
    }
    *body = ota_buf + hdr_end;
    if (strstr((const char *)ota_buf, "Connection: close")) ota_sock_close();
    return blen;
  }
  return -1;
}

// ---------------------------------------------------------------- trial, confirm, go back
// Back to the firmware the new image was installed from: boot that slot, mark this image failed
// here, restart. esp_ota_set_boot_partition checks the old image before switching.
static void ota_go_back(const char *why, char code) {
  const esp_partition_t *prev = NULL;
  esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);
  for (; it != NULL; it = esp_partition_next(it)) {
    const esp_partition_t *p = esp_partition_get(it);
    if (p->address == st.prev && p != esp_ota_get_running_partition()) prev = p;
  }
  esp_partition_iterator_release(it);
  st.phase = 'F';
  st.why = code;
  st.have = 0;
  ota_save();
  if (!prev || esp_ota_set_boot_partition(prev) != ESP_OK) {
    debugf("[OTA] New image %s, but the old slot cannot be booted: staying on this one.\n", why);
    return;
  }
  debugf("[OTA] New image %s: going back to the old firmware.\n", why);
  delay(300);
  esp_restart();
}

#ifndef OTA_BUILD_TAG
#define OTA_BUILD_TAG ""
#endif
// First thing in setup(): starts the trial of a freshly installed image, counts its crash restarts,
// and goes back after too many.
void ota_boot_init() {
  ota_load();
  const uint32_t running = running_image_id();
  debugf("[OTA] Running image %08lX (type %d)%s%s\n", (unsigned long)running, ota_type(),
         OTA_BUILD_TAG[0] ? " build tag " : "", OTA_BUILD_TAG);
  if (st.phase == 'I') {
    if (running == st.image_id) {
      st.phase = 'T';
      st.crashes = 0;
      ota_save();
      heal_slot_delivered = false;    // RTC counters: the old firmware's last slot must not count
      heal_undelivered_slots = 0;
      debugln("[OTA] New image installed: on trial until it delivers a record.");
    } else {
      st.phase = 'F';                 // the switch did not happen (refused at boot)
      st.why = OTA_WHY_NOBOOT;
      ota_save();
      debugln("[OTA] The new image did not start: not taken again.");
    }
    return;
  }
  if (st.phase != 'T') return;
  if (running != st.image_id) {       // went back already (bootloader or another path)
    st.phase = 'F';
    st.why = OTA_WHY_NOBOOT;
    ota_save();
    return;
  }
  const esp_reset_reason_t r = esp_reset_reason();
  if (r == ESP_RST_PANIC || r == ESP_RST_INT_WDT || r == ESP_RST_TASK_WDT || r == ESP_RST_WDT) {
    st.crashes++;
    ota_save();
    debugf("[OTA] New image crashed (%u of %u allowed).\n", (unsigned)st.crashes, (unsigned)OTA_MAX_TRIAL_CRASHES);
    if (st.crashes >= OTA_MAX_TRIAL_CRASHES) ota_go_back("crashed at start-up", OTA_WHY_CRASH);
  }
}

// Every slot (from the scheduler, never inside a critical section: this reads flash).
void ota_confirm_step() {
  ota_load();
  if (st.phase != 'T') return;
  if (heal_slot_delivered) {
    st.phase = '-';
    st.crashes = 0;
    ota_save();
    esp_ota_mark_app_valid_cancel_rollback();   // for a bootloader that does have rollback
    debugln("[OTA] New firmware delivered data: confirmed.");
  } else if (heal_undelivered_slots >= OTA_CONFIRM_SLOTS) {
    ota_go_back("delivered nothing in 2 h", OTA_WHY_NODATA);
  }
}

// ---------------------------------------------------------------- the slot step
// 22/11/2026 (v6.60): can a download run now? Sent in every check as w= so the office page can say why
// an offered file is not coming ("0" ready, "b3.62" battery low, "q5" records still to send), and used
// for the same decision after an offer, so the page and the unit never disagree.
static void ota_wait_code(char *out, size_t n) {
  const int q = get_total_backlogs(false);
  if (li_bat_val > 0.5f && li_bat_val < OTA_MIN_BATTERY_V) snprintf(out, n, "b%.2f", li_bat_val);
  else if (q > 0) snprintf(out, n, "q%d", q);
  else snprintf(out, n, "0");
}

static bool ota_due(int slot) {
  if (st.phase == 'D' || st.phase == 'I' || st.phase == 'T') return true;   // T: on trial (v6.56c)
  if (OTA_CHECK_EVERY_SLOT || SPIFFS.exists("/ota_now.txt")) return true;   // bench unit: every slot
  char id[16];
  ota_station_id(id, sizeof(id));
  const int offset = atoi(id + 1) % 4;
  if (slot == OTA_DAY_SLOT + offset || slot == OTA_NIGHT_SLOT + offset) return true;
  // v6.61: the day check did not get through: asked again hourly until the health report has gone,
  // rather than lose that day's report
  return health_report_due() && slot > OTA_DAY_SLOT + offset && (slot - OTA_DAY_SLOT - offset) % 4 == 0;
}

static void ota_download(const char *stn, const esp_partition_t *part) {
  // Resume at a sector start: the sector there is erased again
  st.have -= st.have % 4096;
  if (st.have > st.size) st.have = 0;
  debugf("[OTA] Downloading %u bytes from %u\n", (unsigned)st.size, (unsigned)st.have);
  ota_writing_active = true;
  int bad = 0, since_save = 0, good_run = 0;
  uint32_t piece = OTA_PIECE_MAX;     // adapts: halves after a failed piece, doubles after 16 good ones
  char path[160];
  while (st.have < st.size && bad < 4) {
    if (seconds_to_boundary() < OTA_STOP_BEFORE_SLOT_S || pending_manual_status || pending_manual_gps) break;
    esp_task_wdt_reset();
    // A piece never crosses a 4 KB sector boundary (each sector is erased when its first piece arrives)
    uint32_t n = piece - (st.have % piece);
    if (n > st.size - st.have) n = st.size - st.have;
    snprintf(path, sizeof(path), "/lpc/fw?id=%s&c=%08lX&o=%lu&n=%lu", stn, (unsigned long)st.file_crc,
             (unsigned long)st.have, (unsigned long)n);
    int status = 0;
    const uint8_t *body = NULL;
    const int blen = ota_http_get(path, &body, &status);
    if (blen > 0 && status == 404) {
      debugln("[OTA] The server no longer has that file: download dropped.");
      st.phase = '-';
      st.have = 0;
      break;
    }
    if (blen != (int)n + 10 || status != 200 || body[0] != '@' || body[9] != '\n' ||
        strtoul((const char *)body + 1, NULL, 16) != ota_crc32(0, body + 10, n)) {
      bad++;
      good_run = 0;
      if (piece > OTA_PIECE_MIN) piece /= 2;
      continue;
    }
    if (st.have % 4096 == 0 && esp_partition_erase_range(part, st.have, 4096) != ESP_OK) {
      debugln("[OTA] Flash erase failed.");
      bad = 4;
      break;
    }
    if (esp_partition_write(part, st.have, body + 10, n) != ESP_OK) {
      debugln("[OTA] Flash write failed.");
      bad = 4;
      break;
    }
    bad = 0;
    st.have += n;
    if (++good_run >= 16 && piece < OTA_PIECE_MAX) {
      piece *= 2;
      good_run = 0;
    }
    if (++since_save >= 16) {      // every 16 pieces
      ota_save();
      since_save = 0;
      debugf("[OTA] %u of %u bytes\n", (unsigned)st.have, (unsigned)st.size);
    }
  }
  ota_writing_active = false;
  ota_save();
  if (st.have < st.size) {
    debugf("[OTA] Stopped at %u of %u (%s): goes on next slot\n", (unsigned)st.have, (unsigned)st.size,
           bad >= 4 ? "no good reply" : "slot time");
  }
}

// 21/11/2026 (v6.59): the station type in the downloaded image's AIOOTA1 block (the first one, as the
// server reads it), or -1 if it has none. OTA keeps the unit's own type; only an SD card update changes it.
static int32_t ota_image_type(const esp_partition_t *part, uint32_t size) {
  const char *magic = ota_ident_ref->magic;        // the unit's own block: no second copy of the marker in the image
  const uint32_t keep = sizeof(OtaIdent) - 1;       // overlap so a block across two reads is found
  for (uint32_t off = 0; off < size; off += 2048 - keep) {
    const uint32_t n = (size - off > 2048) ? 2048 : size - off;
    if (esp_partition_read(part, off, ota_buf, n) != ESP_OK) return -1;
    for (uint32_t i = 0; i + sizeof(OtaIdent) <= n; i++) {
      if (memcmp(ota_buf + i, magic, 8) == 0) {
        int32_t t;
        memcpy(&t, ota_buf + i + 8, 4);
        return t;
      }
    }
    if ((off & 0x3FFFF) < 2048) esp_task_wdt_reset();
    if (n < 2048) break;
  }
  return -1;
}

// 22/11/2026 (v6.62): the display a firmware file on the SD card is built for: 1 Nuvoton, 0 Matrix, -1 not
// known or not AIO9 firmware. An SD card may change the station type (TRG, BIHAR TRG, TWS...), never the
// hardware: Nuvoton firmware on a Matrix unit (or the reverse) leaves it with no display or keypad.
// From the AIOOTA1 block (types 10-27: even Nuvoton, odd Matrix; 28-31 DWLR boards, refused). Firmware
// without a block, type 0, or a type added later: the "-N"/"-M" ending of fw_version.txt decides.
int ota_sd_file_ui(File &f, const char *sd_ver) {
  const char *magic = ota_ident_ref->magic;        // no second copy of the marker in this image
  const uint32_t keep = sizeof(OtaIdent) - 1;
  const uint32_t size = f.size();
  int32_t type = -1;
  for (uint32_t off = 0; off < size && type < 0; off += 2048 - keep) {
    if (!f.seek(off)) break;
    const int n = f.read(ota_buf, 2048);
    if (n <= 0) break;
    for (int i = 0; i + (int)sizeof(OtaIdent) <= n; i++) {
      if (memcmp(ota_buf + i, magic, 8) == 0) {
        memcpy(&type, ota_buf + i + 8, 4);
        break;
      }
    }
    if ((off & 0x3FFFF) < 2048) esp_task_wdt_reset();
    if (n < 2048) break;
  }
  f.seek(0);
  if (type >= 10 && type <= 27) return (type & 1) ? 0 : 1;
  if (type >= 28 && type <= 31) return -1;          // DWLR board firmware (28 SPATIKA, 30 KSNDMC; 29/31 kept for it)
  // No block, type 0, or an AIO9 type added after this firmware: fw_version.txt decides
  const size_t len = sd_ver ? strlen(sd_ver) : 0;
  if (len >= 2 && sd_ver[len - 2] == '-') {
    if (sd_ver[len - 1] == 'N' || sd_ver[len - 1] == 'n') return 1;
    if (sd_ver[len - 1] == 'M' || sd_ver[len - 1] == 'm') return 0;
  }
  return -1;
}

static void ota_install(const esp_partition_t *part) {
  if (seconds_to_boundary() < OTA_INSTALL_NEEDS_S) {
    debugln("[OTA] File complete: installing next slot (too close to the boundary).");
    return;
  }
  // The whole file, as written, against the file CRC the server gave
  uint32_t crc = 0;
  for (uint32_t off = 0; off < st.size; off += 2048) {
    const uint32_t n = (st.size - off > 2048) ? 2048 : st.size - off;
    if (esp_partition_read(part, off, ota_buf, n) != ESP_OK) {
      crc = ~st.file_crc;
      break;
    }
    crc = ota_crc32(crc, ota_buf, n);
    if ((off & 0x3FFFF) == 0) esp_task_wdt_reset();
  }
  if (crc != st.file_crc) {
    // v6.61: at most OTA_MAX_REFETCH times: a flash fault must not mean a 1.3 MB download every slot
    if (++st.crashes >= OTA_MAX_REFETCH) {
      debugln("[OTA] Whole-file CRC wrong again: not fetched again.");
      st.phase = 'F';
      st.why = OTA_WHY_DAMAGED;
    } else {
      debugln("[OTA] Whole-file CRC wrong: downloading again.");
    }
    st.have = 0;
    ota_save();
    return;
  }
  const int32_t itype = ota_image_type(part, st.size);
  if (itype != ota_type()) {
    // firmware of another station type (or none): not over the air. Change type with an SD card update.
    debugf("[OTA] Image is for station type %ld, this unit is %d: refused (use an SD card to change type).\n",
           (long)itype, ota_type());
    st.phase = 'F';
    st.why = OTA_WHY_TYPE;
    ota_save();
    return;
  }
  const esp_err_t r = esp_ota_set_boot_partition(part);   // checks the image as well
  if (r != ESP_OK) {
    debugf("[OTA] Image refused by the bootloader check (%d): not taken again.\n", (int)r);
    st.phase = 'F';
    st.why = OTA_WHY_CHECK;
    ota_save();
    return;
  }
  st.phase = 'I';
  st.prev = esp_ota_get_running_partition()->address;   // to go back to if the new image fails
  st.crashes = 0;
  ota_save();
  debugf("[OTA] Image %08lX checked and set to boot: restarting.\n", (unsigned long)st.image_id);
  ota_sock_close();
  // No file write in flight when the power goes
  if (xSemaphoreTake(fsMutex, pdMS_TO_TICKS(10000)) == pdTRUE) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
  digitalWrite(26, LOW);   // modem off
  vTaskDelay(1500 / portTICK_PERIOD_MS);
  ESP.restart();
}

void ota_slot_step(int slot) {
#if ENABLE_ESP_OTA == 1
  // v6.61: a unit stopped by SMS (VARSHA STOP) still checks in: the office sees it (health "STOPPED")
  // instead of a station gone silent, and can still update it
  if (ota_type() == 0) return;
  // A new image on trial that has just delivered this slot's record is confirmed now, so the check
  // below already reports it kept (otherwise the office showed "On trial" for one more slot).
  // v6.56c: while a new image is on trial the unit checks every slot, and once more in the slot it is
  // confirmed, so the office sees "On trial" and then "Updated" at once instead of at the next
  // twice-a-day check (up to 12 h later).
  ota_load();
  const bool was_on_trial = (st.phase == 'T');
  ota_confirm_step();
  ota_load();
  const uint32_t running = running_image_id();
  if (!was_on_trial && !ota_due(slot)) return;
  if (xSemaphoreTake(modemMutex, pdMS_TO_TICKS(10000)) != pdTRUE) {
    debugln("[OTA] Modem busy: check put off.");
    return;
  }
  char stn[16];
  static char path[760];
  ota_station_id(stn, sizeof(stn));
  const bool failed = (st.phase == 'F');
  char wait[12];
  ota_wait_code(wait, sizeof(wait));
  snprintf(path, sizeof(path), "/lpc/check?id=%s&p=%d&cc=00000000&v=%s&c=%08lX&s=%c&t=%08lX&w=%s", stn,
           ota_type(), FIRMWARE_VERSION, (unsigned long)running,
           st.phase == 'D' ? '1' : failed ? '3' : st.phase == 'T' ? '2' : '0',   // 2: new image on trial
           (unsigned long)(failed ? st.image_id : 0), wait);
  if (failed && st.why) {                         // v6.61: why it was given up
    const size_t l = strlen(path);
    snprintf(path + l, sizeof(path) - l, "&e=%c", st.why);
  }
  // v6.57: the daily health report rides on the day check (health_day.ino), as on the LPC units
  const bool with_report = health_report_due();
  if (with_report) health_append_report(path, sizeof(path));
  int status = 0;
  const uint8_t *body = NULL;
  const int blen = ota_http_get(path, &body, &status);
  char answer[80] = "";
  if (blen > 0 && status == 200) {
    snprintf(answer, sizeof(answer), "%.*s", blen < 79 ? blen : 79, (const char *)body);
    if (with_report && strncmp(answer, "OTA ", 4) == 0) health_report_sent();
  }
  if (strncmp(answer, "OTA ", 4) != 0) {
    debugf("[OTA] No answer from %s (HTTP %d)\n", ota_server_host(), status);
  } else {
    unsigned long size = 0, fcrc = 0, icrc = 0;
    const esp_partition_t *part = esp_ota_get_next_update_partition(NULL);
    if (strncmp(answer, "OTA FW ", 7) != 0 || sscanf(answer + 7, "%lu %8lx %8lx", &size, &fcrc, &icrc) != 3) {
      debugln("[OTA] Nothing new.");
      if (st.phase == 'D') {
        st.phase = '-';
        st.have = 0;
        ota_save();
      }
    } else if (!part || size < 1000 || size > part->size || icrc == running) {
      debugln("[OTA] Offer ignored (size, no spare slot, or already running).");
    } else if (failed && st.image_id == icrc) {
      debugln("[OTA] The offered firmware failed here before: not taken.");
    } else if (st.phase == 'T') {
      // v6.61: a new image on trial keeps its trial (and its way back) until it has delivered a record
      debugln("[OTA] New firmware on trial: another download waits until it is confirmed.");
    } else if (wait[0] == 'b') {
      debugf("[OTA] Download put off: battery %s V.\n", wait + 1);
    } else if (wait[0] == 'q') {
      debugf("[OTA] Download put off: %s record(s) still to send first.\n", wait + 1);
    } else {
      if (st.phase != 'D' || st.file_crc != fcrc || st.size != size) {
        st = {(uint32_t)fcrc, (uint32_t)icrc, (uint32_t)size, 0, 'D', 0, 0, 0};
        ota_save();
      }
      ota_download(stn, part);
      if (st.phase == 'D' && st.have >= st.size) ota_install(part);
    }
  }
  ota_sock_close();
  xSemaphoreGive(modemMutex);
#else
  (void)slot;
#endif
}
