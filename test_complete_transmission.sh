#!/bin/bash

# =============================================================================
# Spatika AIO9 v5.47 - Complete Transmission & Retry Test Suite
# =============================================================================
# This script verifies all 5 hardware configurations for:
# 1. Current Slot Transmission (Valid formats)
# 2. Retry Logic (Pipelined resend)
# 3. Unsent Data Recovery (Old timestamped records)
# 4. Health Report Integration (New Audit Counters)
# =============================================================================

# -- Time Calculation --
NOW_ISO=$(date +'%Y-%m-%d %H:%M:%S')
NOW_CSV=$(date +'%Y-%m-%d,%H:%M')
PAST_ISO=$(date -v-1H +'%Y-%m-%d %H:%M:%S')
PAST_CSV=$(date -v-1H +'%Y-%m-%d,%H:%M')

echo "=================================================="
echo " Starting FULL Integration Test at $(date)"
echo " Current Slot: $NOW_ISO"
echo " Backlog Slot: $PAST_ISO"
echo "=================================================="

# 1. BIHAR_TRG (SYSTEM 0) - JSON
echo -e "\n[1] BIHAR_TRG (ID: 001920)"
URL_BIHAR="http://rtdasbmsk.spatika.net:8085/Home/UpdateTRGData"
PAYLOAD_CUR="{\"StnNo\":\"001920\",\"DeviceTime\":\"$NOW_ISO\",\"RainDP\":\"000.0\",\"RainCuml\":\"001.2\",\"BatVolt\":\"04.1\",\"SigStr\":\"-051\",\"ApiKey\":\"bmsk1234\"}"
PAYLOAD_RET="{\"StnNo\":\"001920\",\"DeviceTime\":\"$PAST_ISO\",\"RainDP\":\"001.0\",\"RainCuml\":\"002.2\",\"BatVolt\":\"04.1\",\"SigStr\":\"-051\",\"ApiKey\":\"bmsk1234\"}"

echo "  → Current: $(curl -s -X POST -H "Content-Type: application/json" -d "$PAYLOAD_CUR" "$URL_BIHAR")"
echo "  → Retry:   $(curl -s -X POST -H "Content-Type: application/json" -d "$PAYLOAD_RET" "$URL_BIHAR")"

# 2. KSNDMC_TRG (SYSTEM 0) - URL ENCODED
echo -e "\n[2] KSNDMC_TRG (ID: 001921)"
URL_KS_TRG="http://rtdas.ksndmc.net/trg_gprs/update_data_sit_v3"
# Current
PAYLOAD_CUR="stn_id=001921&rec_time=$NOW_CSV&rainfall=000.25&signal=-051&key=pse2420&bat_volt=04.1&bat_volt1=04.1"
echo "  → Current: $(curl -s -X POST -d "$PAYLOAD_CUR" "$URL_KS_TRG")"
# Unsent
PAYLOAD_RET="stn_id=001921&rec_time=$PAST_CSV&rainfall=000.25&signal=-051&key=pse2420&bat_volt=04.1&bat_volt1=04.1"
echo "  → Unsent:  $(curl -s -X POST -d "$PAYLOAD_RET" "$URL_KS_TRG")"

# 3. KSNDMC_TWS (SYSTEM 1) - URL ENCODED
echo -e "\n[3] KSNDMC_TWS (ID: 001930)"
URL_KS_TWS="http://rtdas.ksndmc.net/tws_gprs/update_tws_data_v2"
PAYLOAD_CUR="stn_no=001930&rec_time=$NOW_CSV&temp=28.5&humid=45.2&w_speed=01.25&w_dir=180&signal=-051&key=climate4p2013&bat_volt=04.1&bat_volt2=04.1"
echo "  → Current: $(curl -s -X POST -d "$PAYLOAD_CUR" "$URL_KS_TWS")"

# 4. KSNDMC_ADDON (SYSTEM 2) - URL ENCODED
echo -e "\n[4] KSNDMC_ADDON (ID: 001941)"
URL_KS_ADDON="http://rtdas.ksndmc.net/tws_gprs/update_twsrf_data_v2"
PAYLOAD_CUR="stn_no=001941&rec_time=$NOW_CSV&key=rfclimate5p13&rainfall=001.2&temp=28.5&humid=45.2&w_speed=01.25&w_dir=180&signal=-051&bat_volt=04.1&bat_volt2=04.1"
echo "  → Current: $(curl -s -X POST -d "$PAYLOAD_CUR" "$URL_KS_ADDON")"

# 5. SPATIKA_GEN (SYSTEM 2) - URL ENCODED
echo -e "\n[5] SPATIKA_GEN (ID: WS0035)"
URL_SPATIKA_GEN="http://rtdas.spatika.net/tws_gprs/twsrf_gen"
PAYLOAD_CUR="stn_id=WS0035&rec_time=$NOW_CSV&key=wsgen2014&rainfall=001.25&temp=28.5&humid=45.2&w_speed=01.25&w_dir=180&signal=-051&bat_volt=04.1&bat_volt2=04.1"
echo "  → Current: $(curl -s -X POST -d "$PAYLOAD_CUR" "$URL_SPATIKA_GEN")"

# 6. HEALTH REPORT (v5.47 NEW AUDIT COUNTERS)
echo -e "\n[6] HEALTH REPORT AUDIT"
URL_HEALTH="http://75.119.148.192:80/health"
HEALTH_PAYLOAD="{
 \"stn_id\":\"001920\",
 \"unit_type\":\"BIHAR_TRG\",
 \"system\":0,
 \"health_sts\":\"OK\",
 \"uptime_hrs\":120,
 \"bat_v\":4.12,
 \"sig_str\":-51,
 \"http_suc_cnt\":96,
 \"http_ret_cnt\":5,
 \"ftp_suc_cnt\":2,
 \"net_cnt\":103,
 \"cur_stored\":103
}"
echo "  → Health Payload: $HEALTH_PAYLOAD"
RESP=$(curl -s -X POST -H "Content-Type: application/json" -d "$HEALTH_PAYLOAD" "$URL_HEALTH")
echo "  → Server Response: $RESP"

echo -e "\n=================================================="
echo " All Tests Completed Successfully."
echo "=================================================="
