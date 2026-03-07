from sqlalchemy import Column, Integer, String, Float, DateTime, func
from .database import Base


class FirmwareRegistry(Base):
    __tablename__ = "firmware_registry"
    category_id  = Column(Integer, primary_key=True)
    name         = Column(String(32))
    display_name = Column(String(64), default="")
    unit_type    = Column(String(32))
    system_mode  = Column(Integer)
    current_ver  = Column(String(16), default="5.77")
    filename     = Column(String(128), default="firmware.bin")
    total_target = Column(Integer, default=0)
    updated_at   = Column(DateTime, server_default=func.now(), onupdate=func.now())


class HealthReport(Base):
    __tablename__ = "health_reports"
    id               = Column(Integer, primary_key=True, index=True)
    stn_id           = Column(String, index=True)
    unit_type        = Column(String)
    system           = Column(Integer, default=-1)
    health_sts       = Column(String)
    sensor_sts       = Column(String)
    reset_reason     = Column(Integer, default=0)
    rtc_ok           = Column(Integer, default=1)
    uptime_hrs       = Column(Integer, default=0)
    bat_v            = Column(Float)
    sol_v            = Column(Float)
    signal           = Column(Integer, default=0)
    reg_fails        = Column(Integer, default=0)
    reg_avg          = Column(Float, default=0)
    reg_worst        = Column(Integer, default=0)
    reg_fail_type    = Column(String, default="NONE")
    http_fails       = Column(Integer, default=0)
    http_fail_reason = Column(String, default="NONE")
    http_avg         = Column(Float, default=0)
    net_cnt          = Column(Integer, default=0)
    net_cnt_prev     = Column(Integer, default=0)
    cur_stored       = Column(Integer, default=0)
    prev_stored      = Column(Integer, default=0)
    http_suc_cnt     = Column(Integer, default=0)
    http_suc_cnt_prev= Column(Integer, default=0)
    http_ret_cnt     = Column(Integer, default=0)
    http_ret_cnt_prev= Column(Integer, default=0)
    ftp_suc_cnt      = Column(Integer, default=0)
    ftp_suc_cnt_prev = Column(Integer, default=0)
    ndm_cnt          = Column(Integer, default=0)
    pd_cnt           = Column(Integer, default=0)
    cdm_sts          = Column(String, default="OK")
    first_http       = Column(Integer, default=0)
    spiffs_kb        = Column(Integer, default=0)
    spiffs_total_kb  = Column(Integer, default=4640)
    sd_sts           = Column(String, default="FAIL")
    calib            = Column(String, default="NA")
    ver              = Column(String)
    carrier          = Column(String)
    iccid            = Column(String)
    gps              = Column(String)
    ota_fails        = Column(Integer, default=0)
    ota_fail_reason  = Column(String, default="NONE")
    consec_reg_fails = Column(Integer, default=0)
    consec_http_fails= Column(Integer, default=0)
    consec_sim_fails = Column(Integer, default=0)
    spiffs_free_kb   = Column(Integer, default=0)
    free_heap_kb     = Column(Integer, default=0)
    unsent_count     = Column(Integer, default=0)
    slot_no          = Column(Integer, default=0)
    ws_same_cnt      = Column(Integer, default=0)
    # v7.59: Deep Diagnostics Tracking
    stack_rtc        = Column(Integer, default=0)
    stack_sched      = Column(Integer, default=0)
    stack_gprs       = Column(Integer, default=0)
    stack_ui         = Column(Integer, default=0)
    i2c_errs         = Column(Integer, default=0)
    m_temp           = Column(Integer, default=0)
    net_type         = Column(String, default="NONE")
    min_rssi         = Column(Integer, default=0)
    max_rssi         = Column(Integer, default=0)
    reported_at      = Column(DateTime, server_default=func.now())


class CommandQueue(Base):
    __tablename__ = "command_queue"
    id          = Column(Integer, primary_key=True, index=True)
    stn_id      = Column(String, index=True)
    cmd         = Column(String)
    cmd_param   = Column(String, default="")
    created_at  = Column(DateTime, server_default=func.now())
    executed_at = Column(DateTime, nullable=True)


class StationSettings(Base):
    __tablename__ = "station_settings"
    stn_id      = Column(String, primary_key=True, index=True)
    ota_exempt  = Column(Integer, default=0) # 1 means OTA is disabled for this station
    updated_at  = Column(DateTime, server_default=func.now(), onupdate=func.now())
