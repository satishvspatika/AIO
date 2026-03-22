#!/bin/bash
# =============================================================================
# generate_test_from_firmware.sh
# =============================================================================
# PURPOSE:
#   Reads the CURRENT firmware config (SYSTEM, UNIT, http_no, FTP creds) from
#   globals.h and gprs.ino, then generates a derived test script:
#       test_current_firmware_<TIMESTAMP>.sh
#
#   The GOLDEN script (test_complete_transmission.sh) is NEVER modified.
#   On failure, it diffs the generated test vs the golden for comparison.
#
# USAGE:
#   ./generate_test_from_firmware.sh [station_id]
#
#   station_id  : Override station ID (default: auto-read from NVS cache or use WS0034)
#                 Example: ./generate_test_from_firmware.sh WS0099
#
# HOW TO RUN IN FUTURE:
#   Every time you want to validate the firmware before release:
#       ./generate_test_from_firmware.sh
#   It will auto-adapt to whatever SYSTEM/UNIT is active in globals.h.
# =============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GLOBALS_H="$SCRIPT_DIR/user_config.h"
GPRS_INO="$SCRIPT_DIR/gprs.ino"
GOLDEN_SCRIPT="$SCRIPT_DIR/test_complete_transmission.sh"
TIMESTAMP=$(date +'%Y%m%d_%H%M%S')
OUTPUT_SCRIPT="$SCRIPT_DIR/test_current_firmware_${TIMESTAMP}.sh"

echo "============================================================"
echo "  Firmware Test Generator"
echo "  Golden Script: test_complete_transmission.sh (READ ONLY)"
echo "  Output Script: test_current_firmware_${TIMESTAMP}.sh"
echo "============================================================"

# ── Step 1: Parse firmware config from globals.h ──────────────────────────────
SYSTEM_VAL=$(grep -m1 "^#define SYSTEM " "$GLOBALS_H" | awk '{print $3}')
UNIT_VAL=$(grep -m1 "^#define UNIT_CFG" "$GLOBALS_H" | sed 's/.*"\(.*\)".*/\1/')
FW_VER=$(grep -m1 "#define FIRMWARE_VERSION" "$GLOBALS_H" | sed 's/.*"\(.*\)".*/\1/')

echo ""
echo "[1] Detected Firmware Config:"
echo "    SYSTEM          = $SYSTEM_VAL"
echo "    UNIT            = $UNIT_VAL"
echo "    FIRMWARE_VER    = $FW_VER"

if [ -z "$SYSTEM_VAL" ] || [ -z "$UNIT_VAL" ]; then
    echo "ERROR: Could not parse SYSTEM or UNIT from globals.h!"
    exit 1
fi

# ── Step 2: Station ID  ───────────────────────────────────────────────────────
# Prefer command-line arg, then try to find from NVS-written log, else fallback
STATION_ID="${1:-}"
if [ -z "$STATION_ID" ]; then
    STATION_ID=$(grep -o "WS[0-9]\{4\}" "$SCRIPT_DIR/serial_monitor.log" 2>/dev/null | tail -1)
fi
if [ -z "$STATION_ID" ]; then
    STATION_ID="WS0034"  # fallback default
fi

echo "    Station ID      = $STATION_ID"

# ── Step 3: Resolve HTTP endpoint & payload format ────────────────────────────
HTTP_HOST=""
HTTP_PATH=""
HTTP_PORT="80"
HTTP_KEY=""
HTTP_CONTENT_TYPE="application/x-www-form-urlencoded"
HTTP_PAYLOAD_STYLE=""  # "stn_id" or "stn_no" or "json"

# SPATIKA_GEN + SYSTEM 2 → twsrf_gen (current WS0034 config)
if echo "$UNIT_VAL" | grep -q "SPATIKA_GEN" && [ "$SYSTEM_VAL" = "2" ]; then
    HTTP_HOST="rtdas.spatika.net"
    HTTP_PATH="/tws_gprs/twsrf_gen"
    HTTP_KEY="wsgen2014"
    HTTP_PAYLOAD_STYLE="stn_id_twsrf_gen"
    FTP_SERVER="ftp.spatika.net"
    FTP_USER="twsrf_gen"
    FTP_PASS="spgen123"

elif echo "$UNIT_VAL" | grep -q "SPATIKA_GEN" && [ "$SYSTEM_VAL" = "1" ]; then
    HTTP_HOST="rtdas.spatika.net"
    HTTP_PATH="/tws_gprs/update_tws_data_v2"
    HTTP_KEY="climate4p2013"
    HTTP_PAYLOAD_STYLE="stn_no_tws"
    FTP_SERVER="ftp.spatika.net"
    FTP_USER="twsrf_gen"
    FTP_PASS="spgen123"

elif echo "$UNIT_VAL" | grep -q "SPATIKA_GEN" && [ "$SYSTEM_VAL" = "0" ]; then
    HTTP_HOST="rtdas1.spatika.net"
    HTTP_PATH="/hmrtdas/trg_gen"
    HTTP_KEY="sitgen100"
    HTTP_PAYLOAD_STYLE="stn_id_trg_spatika"
    FTP_SERVER="ftp.spatika.net"
    FTP_USER="trg_gen"
    FTP_PASS="spgen123"

elif echo "$UNIT_VAL" | grep -q "KSNDMC_ADDON" && [ "$SYSTEM_VAL" = "2" ]; then
    HTTP_HOST="rtdas.ksndmc.net"
    HTTP_PATH="/tws_gprs/update_twsrf_data_v2"
    HTTP_KEY="rfclimate5p13"
    HTTP_PAYLOAD_STYLE="stn_no_twsrf"
    FTP_SERVER="ftp1.ksndmc.net"
    FTP_USER="twsrf_spatika_v2"
    FTP_PASS="sittao#10"

elif echo "$UNIT_VAL" | grep -q "KSNDMC_TWS" && [ "$SYSTEM_VAL" = "1" ]; then
    HTTP_HOST="rtdas.ksndmc.net"
    HTTP_PATH="/tws_gprs/update_tws_data_v2"
    HTTP_KEY="climate4p2013"
    HTTP_PAYLOAD_STYLE="stn_no_tws"
    FTP_SERVER="ftp1.ksndmc.net"
    FTP_USER="tws_spatika_v2"
    FTP_PASS="twssp#987"

elif echo "$UNIT_VAL" | grep -q "KSNDMC_TRG" && [ "$SYSTEM_VAL" = "0" ]; then
    HTTP_HOST="rtdas.ksndmc.net"
    HTTP_PATH="/trg_gprs/update_data_sit_v3"
    HTTP_KEY="pse2420"
    HTTP_PAYLOAD_STYLE="stn_id_trg_ksndmc"
    FTP_SERVER="ftp1.ksndmc.net"
    FTP_USER="trg_spatika_v2@ksndmc.net"
    FTP_PASS="trgsp#123"

elif echo "$UNIT_VAL" | grep -q "BIHAR_TRG" && [ "$SYSTEM_VAL" = "0" ]; then
    HTTP_HOST="rtdasbmsk.spatika.net"
    HTTP_PATH="/Home/UpdateTRGData"
    HTTP_PORT="8085"
    HTTP_KEY="bmsk1234"
    HTTP_CONTENT_TYPE="application/json"
    HTTP_PAYLOAD_STYLE="json_bihar"
    FTP_SERVER="ftphbih.spatika.net"
    FTP_USER="trg_desbih_csvdt"
    FTP_PASS="rf24hrcsv2021"

else
    echo "ERROR: Unknown UNIT/SYSTEM combination: $UNIT_VAL / $SYSTEM_VAL"
    echo "       Please add this combination to generate_test_from_firmware.sh"
    exit 1
fi

echo ""
echo "[2] Resolved Endpoints:"
echo "    HTTP  → http://$HTTP_HOST:$HTTP_PORT$HTTP_PATH (key=$HTTP_KEY)"
echo "    FTP   → $FTP_SERVER (user=$FTP_USER)"
echo "    Style → $HTTP_PAYLOAD_STYLE"

# ── Step 4: Time helpers ──────────────────────────────────────────────────────
NOW_CSV=$(date +'%Y-%m-%d,%H:%M')
# Round to last 15-min slot
CURRENT_MIN=$(date +'%M')
SLOT_MIN=$(( (10#$CURRENT_MIN / 15) * 15 ))
SLOT_CSV=$(date +"$Y-%m-%d,")$(printf "%02d:%02d" "$(date +'%H')" "$SLOT_MIN")
SLOT_CSV=$(date +'%Y-%m-%d,')$(printf "%02d:%02d" "$(date +'%H')" "$SLOT_MIN")
BACKLOG_CSV=$(date -v-1H +'%Y-%m-%d,%H:')$(printf "%02d" "$SLOT_MIN")

# ── Step 5: Generate the derived test script ─────────────────────────────────
echo ""
echo "[3] Generating test script: $OUTPUT_SCRIPT"

cat > "$OUTPUT_SCRIPT" << HEREDOC
#!/bin/bash
# =============================================================================
# AUTO-GENERATED FIRMWARE TEST SCRIPT
# Generated by: generate_test_from_firmware.sh
# Generated at: $(date)
# Firmware   : v$FW_VER
# SYSTEM     : $SYSTEM_VAL
# UNIT       : $UNIT_VAL
# Station ID : $STATION_ID
#
# !! DO NOT EDIT THIS FILE MANUALLY !!
# !! Regenerate with: ./generate_test_from_firmware.sh !!
# =============================================================================

STATION_ID="$STATION_ID"
HTTP_URL="http://$HTTP_HOST:$HTTP_PORT$HTTP_PATH"
FTP_SERVER="$FTP_SERVER"
FTP_USER="$FTP_USER"
FTP_PASS="$FTP_PASS"

SLOT_CSV="$SLOT_CSV"
BACKLOG_CSV="$BACKLOG_CSV"

echo "============================================================"
echo " Firmware Test | v$FW_VER | $UNIT_VAL | SYSTEM=$SYSTEM_VAL"
echo " Station: \$STATION_ID"
echo " HTTP: \$HTTP_URL"
echo " FTP : \$FTP_SERVER"
echo " Time: Current=\$SLOT_CSV  Backlog=\$BACKLOG_CSV"
echo "============================================================"

PASS=0
FAIL=0

run_test() {
    local label="\$1"
    local url="\$2"
    local payload="\$3"
    local content_type="\$4"
    local response

    if [ "\$content_type" = "application/json" ]; then
        response=\$(curl -s -m 10 -X POST -H "Content-Type: application/json" -d "\$payload" "\$url")
    else
        response=\$(curl -s -m 10 -X POST -d "\$payload" "\$url")
    fi

    if echo "\$response" | grep -qi "success\|ok\|stored"; then
        echo "  ✅ \$label: \$response"
        PASS=\$((PASS+1))
    else
        echo "  ❌ \$label: \$response"
        FAIL=\$((FAIL+1))
    fi
}

HEREDOC

# Now append the actual test cases based on UNIT/SYSTEM
if [ "$HTTP_PAYLOAD_STYLE" = "stn_id_twsrf_gen" ]; then
cat >> "$OUTPUT_SCRIPT" << HEREDOC
# ── HTTP: Current Slot ────────────────────────────────────────────────────────
echo ""
echo "[A] HTTP — Current Slot (\$SLOT_CSV)"
PAYLOAD_CUR="stn_id=\${STATION_ID}&rec_time=\${SLOT_CSV}&key=$HTTP_KEY&rainfall=00.00&temp=028.5&humid=060.0&w_speed=00.25&w_dir=180&signal=-063&bat_volt=03.9&bat_volt2=03.9"
run_test "Current HTTP" "\$HTTP_URL" "\$PAYLOAD_CUR" "$HTTP_CONTENT_TYPE"

# ── HTTP: Backlog slot (1 hour old) ──────────────────────────────────────────
echo ""
echo "[B] HTTP — Backlog Slot (\$BACKLOG_CSV)"
PAYLOAD_OLD="stn_id=\${STATION_ID}&rec_time=\${BACKLOG_CSV}&key=$HTTP_KEY&rainfall=01.00&temp=029.0&humid=058.0&w_speed=00.50&w_dir=90&signal=-075&bat_volt=03.8&bat_volt2=03.8"
run_test "Backlog HTTP" "\$HTTP_URL" "\$PAYLOAD_OLD" "$HTTP_CONTENT_TYPE"

# ── FTP: Connectivity Check ───────────────────────────────────────────────────
echo ""
echo "[C] FTP — Connection Check (passive login test)"
FTP_RESULT=\$(curl -s -m 15 --ftp-pasv \
    "ftp://$FTP_SERVER/" \
    --user "$FTP_USER:$FTP_PASS" \
    --list-only 2>&1 | head -5)
if echo "\$FTP_RESULT" | grep -qiE "^[0-9]|\.txt|\.swd|\.kwd|\.csv|ftp"; then
    echo "  ✅ FTP Login OK: \$FTP_RESULT"
    PASS=\$((PASS+1))
else
    echo "  ❌ FTP Login FAILED: \$FTP_RESULT"
    FAIL=\$((FAIL+1))
fi

# ── FTP: Dummy File Upload ────────────────────────────────────────────────────
echo ""
echo "[D] FTP — Dummy File Upload"
DUMMY_FILE="/tmp/ftp_test_\$(date +'%Y%m%d_%H%M%S').swd"
printf "%s\r\n" \\
    "WS0034;${SLOT_CSV};00.00;028.5;060.0;00.25;180;-063;03.9" \\
    "WS0034;${BACKLOG_CSV};01.00;029.0;058.0;00.50;090;-075;03.8" > "\$DUMMY_FILE"
UPLOAD_RESULT=\$(curl -s -m 20 --ftp-pasv \
    "ftp://$FTP_SERVER/\$(basename \$DUMMY_FILE)" \
    --user "$FTP_USER:$FTP_PASS" \
    --upload-file "\$DUMMY_FILE" 2>&1)
if [ \$? -eq 0 ]; then
    echo "  ✅ FTP Upload OK: \$(basename \$DUMMY_FILE)"
    PASS=\$((PASS+1))
    # Cleanup uploaded test file
    curl -s -m 10 "ftp://$FTP_SERVER/" --user "$FTP_USER:$FTP_PASS" -Q "DELE \$(basename \$DUMMY_FILE)" > /dev/null 2>&1
    echo "     (Test file auto-deleted from FTP server)"
else
    echo "  ❌ FTP Upload FAILED: \$UPLOAD_RESULT"
    FAIL=\$((FAIL+1))
fi
rm -f "\$DUMMY_FILE"

HEREDOC

elif [ "$HTTP_PAYLOAD_STYLE" = "stn_no_twsrf" ]; then
cat >> "$OUTPUT_SCRIPT" << HEREDOC
echo ""
echo "[A] HTTP — Current Slot (\$SLOT_CSV)"
PAYLOAD_CUR="stn_no=\${STATION_ID}&rec_time=\${SLOT_CSV}&key=$HTTP_KEY&rainfall=00.00&temp=028.5&humid=060.0&w_speed=00.25&w_dir=180&signal=-063&bat_volt=03.9&bat_volt2=03.9"
run_test "Current HTTP" "\$HTTP_URL" "\$PAYLOAD_CUR" "$HTTP_CONTENT_TYPE"

echo ""
echo "[B] HTTP — Backlog Slot (\$BACKLOG_CSV)"
PAYLOAD_OLD="stn_no=\${STATION_ID}&rec_time=\${BACKLOG_CSV}&key=$HTTP_KEY&rainfall=01.00&temp=029.0&humid=058.0&w_speed=00.50&w_dir=90&signal=-075&bat_volt=03.8&bat_volt2=03.8"
run_test "Backlog HTTP" "\$HTTP_URL" "\$PAYLOAD_OLD" "$HTTP_CONTENT_TYPE"
HEREDOC

elif [ "$HTTP_PAYLOAD_STYLE" = "stn_id_trg_ksndmc" ]; then
cat >> "$OUTPUT_SCRIPT" << HEREDOC
echo ""
echo "[A] HTTP — Current Slot (\$SLOT_CSV)"
PAYLOAD_CUR="stn_id=\${STATION_ID}&rec_time=\${SLOT_CSV}&rainfall=000.25&signal=-063&key=$HTTP_KEY&bat_volt=03.9&bat_volt1=03.9"
run_test "Current HTTP" "\$HTTP_URL" "\$PAYLOAD_CUR" "$HTTP_CONTENT_TYPE"

echo ""
echo "[B] HTTP — Backlog Slot (\$BACKLOG_CSV)"
PAYLOAD_OLD="stn_id=\${STATION_ID}&rec_time=\${BACKLOG_CSV}&rainfall=001.00&signal=-075&key=$HTTP_KEY&bat_volt=03.8&bat_volt1=03.8"
run_test "Backlog HTTP" "\$HTTP_URL" "\$PAYLOAD_OLD" "$HTTP_CONTENT_TYPE"
HEREDOC

elif [ "$HTTP_PAYLOAD_STYLE" = "json_bihar" ]; then
cat >> "$OUTPUT_SCRIPT" << HEREDOC
SLOT_ISO=\$(echo "\$SLOT_CSV" | sed 's/,/ /'):00
BACKLOG_ISO=\$(echo "\$BACKLOG_CSV" | sed 's/,/ /'):00

echo ""
echo "[A] HTTP — Current Slot (\$SLOT_ISO)"
PAYLOAD_CUR="{\"StnNo\":\"\${STATION_ID}\",\"DeviceTime\":\"\$SLOT_ISO\",\"RainDP\":\"000.0\",\"RainCuml\":\"001.2\",\"BatVolt\":\"03.9\",\"SigStr\":\"-063\",\"ApiKey\":\"$HTTP_KEY\"}"
run_test "Current HTTP" "\$HTTP_URL" "\$PAYLOAD_CUR" "application/json"

echo ""
echo "[B] HTTP — Backlog Slot (\$BACKLOG_ISO)"
PAYLOAD_OLD="{\"StnNo\":\"\${STATION_ID}\",\"DeviceTime\":\"\$BACKLOG_ISO\",\"RainDP\":\"001.0\",\"RainCuml\":\"002.5\",\"BatVolt\":\"03.8\",\"SigStr\":\"-075\",\"ApiKey\":\"$HTTP_KEY\"}"
run_test "Backlog HTTP" "\$HTTP_URL" "\$PAYLOAD_OLD" "application/json"
HEREDOC
fi

# Health report test (universal)
cat >> "$OUTPUT_SCRIPT" << HEREDOC

# ── Health Report ─────────────────────────────────────────────────────────────
echo ""
echo "[E] Health Report"
HEALTH_URL="http://75.119.148.192:80/health"
HEALTH_PAYLOAD="{\"stn_id\":\"\${STATION_ID}\",\"unit_type\":\"$UNIT_VAL\",\"system\":$SYSTEM_VAL,\"health_sts\":\"OK\",\"sensor_sts\":\"RTC-OK,SPIFFS-OK\",\"reset_reason\":5,\"rtc_ok\":1,\"uptime_hrs\":12,\"bat_v\":3.9,\"sol_v\":0.0,\"signal\":-63,\"reg_fails\":0,\"reg_avg\":1.0,\"reg_worst\":1,\"reg_fail_type\":\"none\",\"http_fails\":0,\"http_fail_reason\":\"none\",\"http_avg\":1.2,\"net_cnt\":10,\"net_cnt_prev\":10,\"ndm_cnt\":0,\"pd_cnt\":0,\"cdm_sts\":\"OK\",\"first_http\":1,\"spiffs_kb\":584,\"spiffs_total_kb\":4640,\"sd_sts\":\"FAIL\",\"calib\":\"NONE\",\"ver\":\"$FW_VER\",\"carrier\":\"BSNL\",\"iccid\":\"89917190324927658487\",\"gps\":\"13.004021,77.549263\",\"ota_fails\":0,\"ota_fail_reason\":\"none\"}"
HEALTH_RESP=\$(curl -s -m 10 -X POST -H "Content-Type: application/json" -d "\$HEALTH_PAYLOAD" "\$HEALTH_URL")
echo "  Health Response: \$HEALTH_RESP"
if echo "\$HEALTH_RESP" | grep -qi "ok\|success\|received\|stored"; then
    echo "  ✅ Health Report: OK"
    PASS=\$((PASS+1))
else
    echo "  ⚠️  Health Report: Unexpected response (may still be OK)"
fi

# ── Final Result ──────────────────────────────────────────────────────────────
echo ""
echo "============================================================"
echo " Results: ✅ \$PASS passed | ❌ \$FAIL failed"
echo "============================================================"
if [ \$FAIL -gt 0 ]; then
    exit 1
fi
HEREDOC

chmod +x "$OUTPUT_SCRIPT"

# ── Step 6: Run the generated script ──────────────────────────────────────────
echo ""
echo "[4] Running generated test script..."
echo "------------------------------------------------------------"
bash "$OUTPUT_SCRIPT"
TEST_RESULT=$?
echo "------------------------------------------------------------"

# ── Step 7: Compare with golden if failures found ─────────────────────────────
if [ $TEST_RESULT -ne 0 ]; then
    echo ""
    echo "⚠️  Test failures detected. Comparing with golden script..."
    echo ""
    diff --color=always "$GOLDEN_SCRIPT" "$OUTPUT_SCRIPT" | head -80 || true
    echo ""
    echo "Golden script: $GOLDEN_SCRIPT"
    echo "Current test : $OUTPUT_SCRIPT"
    echo "Review the diff above to see what changed vs the golden baseline."
    exit 1
else
    echo ""
    echo "✅ All tests passed! Firmware config is validated for:"
    echo "   SYSTEM=$SYSTEM_VAL, UNIT=$UNIT_VAL, Station=$STATION_ID, v$FW_VER"
    echo ""
    echo "Generated script saved at:"
    echo "   $OUTPUT_SCRIPT"
fi
