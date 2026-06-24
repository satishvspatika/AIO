
    import { ESPLoader, Transport } from "https://unpkg.com/esptool-js/bundle.js";

    // --- State variables ---
    let port = null;
    let transport = null;
    let esploader = null;
    let isConnected = false;
    let activeFlow = 'factory'; // 'factory' or 'upgrade'
    let currentChipSize = 8; // Auto-detected chip size (4, 8, 16) in MB
    let readLoopPromise = null;
    let cancelReadLoop = true;
    let activeReader = null;
    let isFlashing = false;
    let rawSerialLog = "";
    let lastVerdict = "PENDING";
    let testStartTime = null;
    let testDuration = 0; // in seconds
    let isSyncingQueue = false;
    let vaultAutoLoadAttempted = false; // tracks if auto-load was tried

    // Binary file database (stores ArrayBuffers loaded by drag/drop)
    const fileVault = {
      "bootloader.bin": null,
      "boot_app0.bin": null,
      // Partitions
      "partitions_4mb.bin": null,
      "partitions_8mb.bin": null,
      "partitions_16mb.bin": null,
      // QC Tests
      "qc_test_4mb.bin": null,
      "qc_test_8mb.bin": null,
      "qc_test_16mb.bin": null,
      // Production Apps (default compiled binaries)
      "production_4mb.bin": null,
      "production_8mb.bin": null,
      "production_16mb.bin": null
    };

    // Profile → valid config names mapping
    const PROFILE_CONFIGS = {
      "TRG":    ["KSNDMC_TRG", "BIHAR_TRG", "SPATIKA_GEN"],
      "TWS":    ["KSNDMC_TWS", "KSNDMC_TWS-AP"],
      "TWS-RF": ["KSNDMC_ADDON", "SPATIKA_GEN"]
    };

    // App partition size limits per detected flash size (bytes)
    const APP_SIZE_LIMIT_BYTES = {
      4:  1310720,   // 4MB chip: 1.25MB app slot
      8:  1769472,   // 8MB chip: ~1.68MB app slot
      16: 2097152    // 16MB chip: 2.00MB app slot
    };

    // Custom config binary vault: keyed by config folder name
    // Each entry: { binary: Uint8Array|null, version: string|null, metadata: object|null }
    const customConfigVault = {};
    for (const configs of Object.values(PROFILE_CONFIGS))
      for (const c of configs)
        customConfigVault[c] = { binary: null, version: null, metadata: null };

    // DOM References
    const browserWarning = document.getElementById("browserWarning");
    const connBadge = document.getElementById("connBadge");
    const btnConnToggle = document.getElementById("btnConnToggle");
    const btnStart = document.getElementById("btnStart");
    const btnStartDiag = document.getElementById("btnStartDiag");
    const btnProceedApp = document.getElementById("btnProceedApp");
    const btnExportLog = document.getElementById("btnExportLog");
    const btnConsoleClear = document.getElementById("btnConsoleClear");
    const consoleDiv = document.getElementById("console");
    const progressContainer = document.getElementById("progressContainer");
    const progressBar = document.getElementById("progressBar");
    const fileList = document.getElementById("fileList");

    const cardVault = document.getElementById("cardVault");
    const vaultStepBadge = document.getElementById("vaultStepBadge");
    const cardConnection = document.getElementById("cardConnection");
    const cardProgramming = document.getElementById("cardProgramming");


    const btnSelectFolderAPI = document.getElementById("btnSelectFolderAPI");
    const btnSelectFolderFallback = document.getElementById("btnSelectFolderFallback");
    const btnResetBoard = document.getElementById("btnResetBoard");

    // Tabs References
    const tabBoardQC = document.getElementById("tabBoardQC");
    const tabIntegration = document.getElementById("tabIntegration");
    const tabRepair = document.getElementById("tabRepair");
    const tabFlows = document.getElementById("tabFlows");
    const panelFlows = document.getElementById("panelFlows");

    // Board QC Force Flash Checkbox
    const chkEraseFlash = document.getElementById("chkEraseFlash");

    // Integration DOM References
    const cardIntegrationControls = document.getElementById("cardIntegrationControls");
    const cardIntegrationChecklist = document.getElementById("cardIntegrationChecklist");
    const box1Barcode = document.getElementById("box1Barcode");
    const box2Barcode = document.getElementById("box2Barcode");
    const assocEspBarcode = document.getElementById("assocEspBarcode");
    const assocGprsBarcode = document.getElementById("assocGprsBarcode");
    const assocNuvBarcode = document.getElementById("assocNuvBarcode");
    const assocBatt1Barcode = document.getElementById("assocBatt1Barcode");
    const assocBatt2Barcode = document.getElementById("assocBatt2Barcode");
    const assocMpptBarcode = document.getElementById("assocMpptBarcode");
    const assocStatusText = document.getElementById("assocStatusText");
    const btnStartIntegration = document.getElementById("btnStartIntegration");
    const btnAbortIntegration = document.getElementById("btnAbortIntegration");
    const integrationFwVersion = document.getElementById("integrationFwVersion");
    const integrationMac = document.getElementById("integrationMac");
    const integrationProgressContainer = document.getElementById("integrationProgressContainer");
    const integrationProgressBar = document.getElementById("integrationProgressBar");

    // Repair DOM References
    const cardRepairControls = document.getElementById("cardRepairControls");
    const repairBox1Barcode = document.getElementById("repairBox1Barcode");
    const repairBox2Barcode = document.getElementById("repairBox2Barcode");
    const repairEspBarcode = document.getElementById("repairEspBarcode");
    const repairGprsBarcode = document.getElementById("repairGprsBarcode");
    const repairNuvBarcode = document.getElementById("repairNuvBarcode");
    const repairBatt1Barcode = document.getElementById("repairBatt1Barcode");
    const repairBatt2Barcode = document.getElementById("repairBatt2Barcode");
    const repairMpptBarcode = document.getElementById("repairMpptBarcode");
    const repairReason = document.getElementById("repairReason");
    const repairStatusText = document.getElementById("repairStatusText");
    const btnStartRepair = document.getElementById("btnStartRepair");
    const repairProgressContainer = document.getElementById("repairProgressContainer");
    const repairProgressBar = document.getElementById("repairProgressBar");

    // Repair Swap Badges
    const badgeRepairEsp = document.getElementById("badgeRepairEsp");
    const badgeRepairGprs = document.getElementById("badgeRepairGprs");
    const badgeRepairNuv = document.getElementById("badgeRepairNuv");
    const badgeRepairBox2 = document.getElementById("badgeRepairBox2");
    const badgeRepairBatt1 = document.getElementById("badgeRepairBatt1");
    const badgeRepairBatt2 = document.getElementById("badgeRepairBatt2");
    const badgeRepairMppt = document.getElementById("badgeRepairMppt");

    // Integration Checklist items
    const intChkBoot = document.getElementById("intChkBoot");
    const intValBoot = document.getElementById("intValBoot");
    const intChkGprs = document.getElementById("intChkGprs");
    const intValGprs = document.getElementById("intValGprs");
    const intChkBatt = document.getElementById("intChkBatt");
    const intValBatt = document.getElementById("intValBatt");
    const intChkSolar = document.getElementById("intChkSolar");
    const intValSolar = document.getElementById("intValSolar");
    const intChkSync = document.getElementById("intChkSync");
    const intValSync = document.getElementById("intValSync");

    // Columns & Repair Bin selectors
    const colLeft = document.getElementById("colLeft");
    const colMiddle = document.getElementById("colMiddle");
    const colRight = document.getElementById("colRight");
    const colRepairBin = document.getElementById("colRepairBin");
    const tabRepairBin = document.getElementById("tabRepairBin");
    const btnRefreshRepairBin = document.getElementById("btnRefreshRepairBin");
    const repairBinBody = document.getElementById("repairBinBody");
    const btnOfflineSync = document.getElementById("btnOfflineSync");
    const offlineSyncCount = document.getElementById("offlineSyncCount");

    const integrationAlertBox = document.getElementById("integrationAlertBox");
    const integrationAlertText = document.getElementById("integrationAlertText");
    const repairAlertBox = document.getElementById("repairAlertBox");
    const repairAlertText = document.getElementById("repairAlertText");

    let currentTab = "boardqc"; // "boardqc", "integration", "repair", or "repairbin"
    let isIntegrating = false;
    let loadedRepairConfig = { box1: "", box2: "", esp: "", gprs: "", nuv: "", batt1: "", batt2: "", mppt: "" };

    // Verdict Overlay Elements
    const verdictOverlay = document.getElementById("verdictOverlay");
    const verdictTitle = document.getElementById("verdictTitle");
    const verdictSub = document.getElementById("verdictSub");
    const verdictBox = document.getElementById("verdictBox");
    const btnCloseVerdict = document.getElementById("btnCloseVerdict");
    
    // Checkboxes / Status Displays
    const chkSpiffs = document.getElementById("chkSpiffs");
    const chkSd = document.getElementById("chkSd");
    const chkRtc = document.getElementById("chkRtc");
    const chkBatt = document.getElementById("chkBatt");
    const chkSys3v3 = document.getElementById("chkSys3v3");
    const chkSolar = document.getElementById("chkSolar");
    const chkGprs = document.getElementById("chkGprs");
    const chkWifi = document.getElementById("chkWifi");
    const chkSensor = document.getElementById("chkSensor");
    const chkWindSpd = document.getElementById("chkWindSpd");
    const chkWindDir = document.getElementById("chkWindDir");
    const chkLcd = document.getElementById("chkLcd");
    const chkKeypad = document.getElementById("chkKeypad");
    const chkRf = document.getElementById("chkRf");
    const chkSleep = document.getElementById("chkSleep");
    
    // Configuration checkboxes
    const testCfgEsp = document.getElementById("testCfgEsp");
    const testCfgGprs = document.getElementById("testCfgGprs");
    const testCfgNuvoton = document.getElementById("testCfgNuvoton");
    const fallbackFolderInput = document.getElementById("fallbackFolderInput");
    const fallbackFilesInput = document.getElementById("fallbackFilesInput");
    const linkSelectFiles = document.getElementById("linkSelectFiles");
    
    const valSpiffs = document.getElementById("valSpiffs");
    const valSd = document.getElementById("valSd");
    const valRtc = document.getElementById("valRtc");
    const valBatt = document.getElementById("valBatt");
    const valSys3v3 = document.getElementById("valSys3v3");
    const valSolar = document.getElementById("valSolar");
    const valGprs = document.getElementById("valGprs");
    const valWifi = document.getElementById("valWifi");
    const valSensor = document.getElementById("valSensor");
    const valWindSpd = document.getElementById("valWindSpd");
    const valWindDir = document.getElementById("valWindDir");
    const valLcd = document.getElementById("valLcd");
    const valKeypad = document.getElementById("valKeypad");
    const valRf = document.getElementById("valRf");
    const valSleep = document.getElementById("valSleep");
    
    const operatorGuideBox = document.getElementById("operatorGuideBox");
    const operatorGuideText = document.getElementById("operatorGuideText");
    const btnConsoleCopy = document.getElementById("btnConsoleCopy");

    // ADC voltage conversion constants (adjust to match hardware divider ratios)
    const ADC_VREF = 3.3;   // ESP32 ADC reference voltage (V)
    const ADC_MAX  = 4095;  // 12-bit ADC
    const BATT_MULT  = 1.3548; // Multiply computed voltage by this to get real battery voltage
    const SOLAR_MULT = 7.2; // Multiply computed voltage by this to get real solar voltage

    const metaMac = document.getElementById("metaMac");
    const metaFlashSize = document.getElementById("metaFlashSize");
    const metaImei = document.getElementById("metaImei");
    const metaCcid = document.getElementById("metaCcid");
    const metaCarrier = document.getElementById("metaCarrier");
    const metaNuvoton = document.getElementById("metaNuvoton");
    const metaFwVersion = document.getElementById("metaFwVersion");
    const barcodeEsp = document.getElementById("barcodeEsp");
    const barcodeGprs = document.getElementById("barcodeGprs");
    const barcodeNuvoton = document.getElementById("barcodeNuvoton");

    const flowOptions = document.querySelectorAll(".flow-option");

    // Google Sheets Integration DOM & Logic
    const sheetUrlInput = document.getElementById("sheetUrl");
    const btnSyncSetup = document.getElementById("btnSyncSetup");
    const btnInitializeSheet = document.getElementById("btnInitializeSheet");
    const codeModal = document.getElementById("codeModal");
    const btnCloseModal = document.getElementById("btnCloseModal");
    const btnCopyCode = document.getElementById("btnCopyCode");
    const preCode = document.getElementById("preCode");
    
    // Help Modal DOM
    const helpModal = document.getElementById("helpModal");
    const btnHelp = document.getElementById("btnHelp");
    const btnCloseHelp = document.getElementById("btnCloseHelp");

    // Spec Modal DOM
    const specModal = document.getElementById("specModal");
    const btnSpec = document.getElementById("btnSpec");
    const btnCloseSpec = document.getElementById("btnCloseSpec");

    const appsScriptCode = `function doPost(e) {
  var ss;
  try {
    ss = SpreadsheetApp.getActiveSpreadsheet();
    if (!ss) throw new Error();
  } catch(err) {
    ss = SpreadsheetApp.openById("1Dw3UJEFdewThGpf8DRei69Vfrt_uU_yo438Z5m9xUUs");
  }
  var logSheet = ss.getSheetByName("Test_Logs") || ss.getSheets()[0];
  if (logSheet.getName() !== "Test_Logs") {
    logSheet.setName("Test_Logs");
  }
  var masterSheet = ss.getSheetByName("Master_Status");
  if (!masterSheet) {
    masterSheet = ss.insertSheet("Master_Status");
  }

  var data = JSON.parse(e.postData.contents);

  // A. Handle Update Repair status request
  if (data.action === "updateRepairStatus") {
    var repairSheet = ss.getSheetByName("Repair_Bin") || ss.insertSheet("Repair_Bin");
    var rowIndex = parseInt(data.rowIndex);
    if (rowIndex > 1 && rowIndex <= repairSheet.getLastRow()) {
      repairSheet.getRange(rowIndex, 7).setValue(data.status); // Status is column 7
      return ContentService.createTextOutput("UPDATED");
    }
    return ContentService.createTextOutput("ERROR");
  }

  // Define Standard Columns & Headers (New 18-Column Schema - Rearranged)
  var headers = [
    "Timestamp", "Local Test Time", "Barcode", "DUT Type", "Co-tested Barcodes", "MAC Address", "Flash Size", "GPRS IMEI", "SIM CCID", "Tests Run", "Test Details", "QC Verdict", "Voltages", "Firmware Ver", "Verification Mode", "Duration (s)", "Tester", "Approved By"
  ];

  if (logSheet.getLastRow() === 0) {
    logSheet.appendRow(headers);
    logSheet.getRange(1, 1, 1, headers.length).setFontWeight("bold").setBackground("#1e3a8a").setFontColor("#ffffff");
    logSheet.setFrozenRows(1);
  } else {
    // Overwrite headers to ensure correctness
    logSheet.getRange(1, 1, 1, headers.length).setValues([headers]);
    if (logSheet.getLastColumn() > headers.length) {
      logSheet.deleteColumns(headers.length + 1, logSheet.getLastColumn() - headers.length);
    }
  }

  if (masterSheet.getLastRow() === 0) {
    masterSheet.appendRow(headers);
    masterSheet.getRange(1, 1, 1, headers.length).setFontWeight("bold").setBackground("#115e59").setFontColor("#ffffff");
    masterSheet.setFrozenRows(1);
  } else {
    masterSheet.getRange(1, 1, 1, headers.length).setValues([headers]);
    if (masterSheet.getLastColumn() > headers.length) {
      masterSheet.deleteColumns(headers.length + 1, masterSheet.getLastColumn() - headers.length);
    }
  }

  // B. Handle initialization
  if (data.initOnly) {
    var intSheet = ss.getSheetByName("Integration_Logs") || ss.insertSheet("Integration_Logs");
    var intHeaders = [
      "Timestamp", "Local Test Time", "Box 1 Barcode", "Box 2 Barcode", "ESP32 Barcode", "GPRS Barcode", "Nuvoton Barcode", 
      "Battery 1 Barcode", "Battery 2 Barcode", "MPPT Barcode", "MAC Address", "Firmware Ver", "Integration Verdict", "Tester", "Duration (s)", "Action Type", "Swap Reason"
    ];
    intSheet.getRange(1, 1, 1, intHeaders.length).setValues([intHeaders]);
    intSheet.getRange(1, 1, 1, intHeaders.length).setFontWeight("bold").setBackground("#4338ca").setFontColor("#ffffff");
    intSheet.setFrozenRows(1);

    var repairBinSheet = ss.getSheetByName("Repair_Bin") || ss.insertSheet("Repair_Bin");
    var repairBinHeaders = ["Timestamp", "Board Barcode", "Board Type", "Source Box 1", "Source Box 2", "Failure Reason", "Status", "Tester"];
    repairBinSheet.getRange(1, 1, 1, repairBinHeaders.length).setValues([repairBinHeaders]);
    repairBinSheet.getRange(1, 1, 1, repairBinHeaders.length).setFontWeight("bold").setBackground("#b91c1c").setFontColor("#ffffff");
    repairBinSheet.setFrozenRows(1);

    var archiveSheet = ss.getSheetByName("Archived_Logs") || ss.insertSheet("Archived_Logs");
    var archiveHeaders = ["Archived Date", "Archived By", "Reason for Archive"].concat(headers);
    archiveSheet.getRange(1, 1, 1, archiveHeaders.length).setValues([archiveHeaders]);
    archiveSheet.getRange(1, 1, 1, archiveHeaders.length).setFontWeight("bold").setBackground("#6b7280").setFontColor("#ffffff");
    archiveSheet.setFrozenRows(1);
    if (archiveSheet.getLastColumn() > archiveHeaders.length) {
      archiveSheet.deleteColumns(archiveHeaders.length + 1, archiveSheet.getLastColumn() - archiveHeaders.length);
    }

    return ContentService.createTextOutput("INITIALIZED");
  }

  // C. Handle Integration Sync Payload
  if (data.integrationSync) {
    var intSheet = ss.getSheetByName("Integration_Logs") || ss.insertSheet("Integration_Logs");
    var intHeaders = [
      "Timestamp", "Local Test Time", "Box 1 Barcode", "Box 2 Barcode", "ESP32 Barcode", "GPRS Barcode", "Nuvoton Barcode", 
      "Battery 1 Barcode", "Battery 2 Barcode", "MPPT Barcode", "MAC Address", "Firmware Ver", "Integration Verdict", "Tester", "Duration (s)", "Action Type", "Swap Reason"
    ];
    if (intSheet.getLastRow() === 0) {
      intSheet.appendRow(intHeaders);
      intSheet.getRange(1, 1, 1, intHeaders.length).setFontWeight("bold").setBackground("#4338ca").setFontColor("#ffffff");
      intSheet.setFrozenRows(1);
    }
    
    var intRow = [
      new Date(),
      data.testTime || "--",
      data.box1 || "--",
      data.box2 || "--",
      data.espBarcode || "--",
      data.gprsBarcode || "--",
      data.nuvotonBarcode || "--",
      data.batt1Barcode || "--",
      data.batt2Barcode || "--",
      data.mpptBarcode || "--",
      data.mac || "--",
      data.version || "--",
      data.verdict || "--",
      data.tester || "--",
      data.duration || 0,
      data.actionType || "ORIGINAL_BUILD",
      data.swapReason || ""
    ];
    intSheet.appendRow(intRow);
    
    var verdictCell = intSheet.getRange(intSheet.getLastRow(), 13);
    if (data.verdict && data.verdict.indexOf("PASS") === 0) {
      verdictCell.setBackground("#14532d").setFontColor("#4ade80").setFontWeight("bold");
    } else {
      verdictCell.setBackground("#450a0a").setFontColor("#f87171").setFontWeight("bold");
    }

    if (data.swappedBoards && data.swappedBoards.length > 0) {
      var repairBinSheet = ss.getSheetByName("Repair_Bin") || ss.insertSheet("Repair_Bin");
      for (var i = 0; i < data.swappedBoards.length; i++) {
        var item = data.swappedBoards[i];
        repairBinSheet.appendRow([
          new Date(),
          item.barcode || "--",
          item.type || "--",
          data.box1 || "--",
          data.box2 || "--",
          item.failureReason || "--",
          "NEEDS_REPAIR",
          data.tester || "--"
        ]);
      }
    }

    return ContentService.createTextOutput("SUCCESS");
  }

  // D. Core Board QC Diagnostics Sync (Write separate rows per active board)
  var espBc = (data.espBarcode || "").trim();
  var gprsBc = (data.gprsBarcode || "").trim();
  var nuvBc = (data.nuvotonBarcode || "").trim();

  var boardsToLog = [];

  if (espBc && espBc !== "--") {
    var comp = [gprsBc, nuvBc].filter(function(x) { return x && x !== "--"; }).join(", ");
    boardsToLog.push({
      barcode: espBc,
      type: "ESP32",
      coTested: comp || "--",
      testsRun: "SPIFFS, SD Card, RTC, WiFi, Sensor",
      details: "SPIFFS: " + (data.spiffs || "--") + 
               ", SD: " + (data.sd || "--") + 
               ", RTC: " + (data.rtc || "--") + 
               ", WiFi: " + (data.wifi || "--") + 
               ", Sensor: " + (data.sensor || "--"),
      verdict: evaluateBoardVerdict("ESP32", data)
    });
  }

  if (gprsBc && gprsBc !== "--") {
    var comp = [espBc, nuvBc].filter(function(x) { return x && x !== "--"; }).join(", ");
    boardsToLog.push({
      barcode: gprsBc,
      type: "GPRS",
      coTested: comp || "--",
      testsRun: "Cellular",
      details: "Cellular: " + (data.cellular || "--"),
      verdict: evaluateBoardVerdict("GPRS", data)
    });
  }

  if (nuvBc && nuvBc !== "--") {
    var comp = [espBc, gprsBc].filter(function(x) { return x && x !== "--"; }).join(", ");
    boardsToLog.push({
      barcode: nuvBc,
      type: "Nuvoton",
      coTested: comp || "--",
      testsRun: "Nuvoton Comms, LCD UI, Keypad, Rainfall",
      details: "Nuvoton: " + (data.nuvoton || "--") + 
               ", LCD: " + (data.lcd || "--") + 
               ", Keypad: " + (data.keypad || "--") + 
               ", Rainfall: " + (data.rainRf || "--"),
      verdict: evaluateBoardVerdict("Nuvoton", data)
    });
  }

  for (var k = 0; k < boardsToLog.length; k++) {
    var b = boardsToLog[k];
    
    var rowData = [
      new Date(),
      data.testTime || "--",
      b.barcode,
      b.type,
      b.coTested,
      data.mac || "--",
      b.type === "ESP32" ? (data.flashSize || "--") : "--",
      b.type === "GPRS" ? (data.imei || "--") : "--",
      b.type === "GPRS" ? (data.ccid || "--") : "--",
      b.testsRun,
      b.details,
      b.verdict,
      "Battery: " + (data.battery || "--") + " | Solar: " + (data.solar || "--"),
      data.version || "Unknown",
      data.auditMode || "--",
      data.duration || "--",
      data.tester || "--",
      "Pending Review"
    ];

    // 1. Log to history
    logSheet.appendRow(rowData);
    colorVerdict(logSheet, logSheet.getLastRow(), b.verdict);

    // 2. Log to Master_Status (Upsert based on exact Barcode)
    var mFound = false;
    var mLast = masterSheet.getLastRow();
    if (mLast > 1) {
      var mBarcodes = masterSheet.getRange(2, 3, mLast - 1, 1).getValues(); // Barcode is Col 3
      for (var r = 0; r < mBarcodes.length; r++) {
        if (mBarcodes[r][0] && mBarcodes[r][0].toString().trim().toLowerCase() === b.barcode.toLowerCase()) {
          masterSheet.getRange(r + 2, 1, 1, rowData.length).setValues([rowData]);
          colorVerdict(masterSheet, r + 2, b.verdict);
          mFound = true;
          break;
        }
      }
    }
    if (!mFound) {
      masterSheet.appendRow(rowData);
      colorVerdict(masterSheet, masterSheet.getLastRow(), b.verdict);
    }
  }

  return ContentService.createTextOutput("SUCCESS");
}

function evaluateBoardVerdict(type, data) {
  if (data.verdict && data.verdict.indexOf("PASS") === 0) {
    return "PASS";
  }
  var failReason = "";
  if (type === "ESP32") {
    if (isFail(data.spiffs)) failReason = "SPIFFS Fail";
    else if (isFail(data.sd)) failReason = "SD Card Fail";
    else if (isFail(data.rtc)) failReason = "RTC Fail";
    else if (isFail(data.wifi)) failReason = "WiFi Fail";
    else if (isFail(data.sensor)) failReason = "Sensor Fail";
  } else if (type === "GPRS") {
    if (isFail(data.cellular)) failReason = "Cellular Reg Fail";
  } else if (type === "Nuvoton") {
    if (isFail(data.nuvoton)) failReason = "Nuvoton Comms Fail";
    else if (isFail(data.lcd)) failReason = "LCD Verification Fail";
    else if (isFail(data.keypad)) failReason = "Keypad Sweep Fail";
    else if (isFail(data.rainRf)) failReason = "Rainfall Test Fail";
  }
  
  if (failReason) {
    var profile = data.profile || "TRG";
    return "FAIL: " + failReason + " (" + profile + ")";
  }
  return "PASS";
}

function isFail(val) {
  if (!val) return false;
  var valStr = val.toString().toUpperCase();
  return valStr.indexOf("FAIL") !== -1 || valStr.indexOf("REG_FAIL") !== -1 || valStr.indexOf("REG FAIL") !== -1;
}

function doGet(e) {
  var ss;
  try {
    ss = SpreadsheetApp.getActiveSpreadsheet();
    if (!ss) throw new Error();
  } catch(err) {
    ss = SpreadsheetApp.openById("1Dw3UJEFdewThGpf8DRei69Vfrt_uU_yo438Z5m9xUUs");
  }

  // Handle action parameter
  if (e.parameter.action === "getRepairBin") {
    var repairSheet = ss.getSheetByName("Repair_Bin");
    var items = [];
    if (repairSheet) {
      var rows = repairSheet.getDataRange().getValues();
      if (rows.length > 1) {
        var headers = rows[0];
        for (var i = 1; i < rows.length; i++) {
          var row = rows[i];
          var record = {};
          for (var j = 0; j < headers.length; j++) {
            record[headers[j]] = (row[j] instanceof Date) ? row[j].toLocaleString() : row[j];
          }
          record["rowIndex"] = i + 1;
          items.push(record);
        }
      }
    }
    return ContentService.createTextOutput(JSON.stringify(items))
                         .setMimeType(ContentService.MimeType.JSON);
  }

  var q = e.parameter.q;
  if (!q) {
    return ContentService.createTextOutput(JSON.stringify({ error: "No search query provided" }))
                         .setMimeType(ContentService.MimeType.JSON);
  }

  var matches = [];

  // Search in Test_Logs (Board QC)
  var logSheet = ss.getSheetByName("Test_Logs");
  if (logSheet) {
    var rows = logSheet.getDataRange().getValues();
    var headers = rows[0];
    for (var i = 1; i < rows.length; i++) {
      var row = rows[i];
      var isMatch = false;
      var searchIndices = [2, 5, 7]; // Barcode (Col 3), MAC (Col 6), IMEI (Col 8)
      for (var k = 0; k < searchIndices.length; k++) {
        var idx = searchIndices[k];
        if (row[idx] && row[idx].toString().toLowerCase() === q.toLowerCase()) {
          isMatch = true;
          break;
        }
      }
      if (isMatch) {
        var record = { logType: "board_qc" };
        for (var j = 0; j < headers.length; j++) {
          record[headers[j]] = (row[j] instanceof Date) ? row[j].toLocaleString() : row[j];
        }
        matches.push(record);
      }
    }
  }

  // Search in Integration_Logs
  var intSheet = ss.getSheetByName("Integration_Logs");
  if (intSheet) {
    var rows = intSheet.getDataRange().getValues();
    var headers = rows[0];
    for (var i = 1; i < rows.length; i++) {
      var row = rows[i];
      var isMatch = false;
      var searchIndices = [2, 3, 4, 5, 6, 7, 8, 9, 10]; // Box1, Box2, ESP, GPRS, NUV, Batt1, Batt2, MPPT, MAC
      for (var k = 0; k < searchIndices.length; k++) {
        var idx = searchIndices[k];
        if (row[idx] && row[idx].toString().toLowerCase() === q.toLowerCase()) {
          isMatch = true;
          break;
        }
      }
      if (isMatch) {
        var record = { logType: "integration" };
        for (var j = 0; j < headers.length; j++) {
          record[headers[j]] = (row[j] instanceof Date) ? row[j].toLocaleString() : row[j];
        }
        matches.push(record);
      }
    }
  }

  return ContentService.createTextOutput(JSON.stringify(matches))
                       .setMimeType(ContentService.MimeType.JSON);
}

function colorVerdict(sheet, row, verdict) {
  var verdictCell = sheet.getRange(row, 12); // QC Verdict is column 12
  if (verdict && verdict.indexOf("PASS") === 0) {
    verdictCell.setBackground("#14532d").setFontColor("#4ade80").setFontWeight("bold");
  } else {
    verdictCell.setBackground("#450a0a").setFontColor("#f87171").setFontWeight("bold");
  }
}

function onOpen() {
  var ui = SpreadsheetApp.getUi();
  ui.createMenu('Spatika Tools')
      .addItem('Initialize Sheet Tabs & Headers', 'initializeSheets')
      .addItem('Manage & Archive Records...', 'openManageRecordsModal')
      .addToUi();
}

function initializeSheets() {
  var ss;
  try {
    ss = SpreadsheetApp.openById("1Dw3UJEFdewThGpf8DRei69Vfrt_uU_yo438Z5m9xUUs");
  } catch(err) {
    ss = SpreadsheetApp.getActiveSpreadsheet();
  }
  
  var logSheet = ss.getSheetByName("Test_Logs") || ss.insertSheet("Test_Logs");
  var masterSheet = ss.getSheetByName("Master_Status") || ss.insertSheet("Master_Status");
  var intSheet = ss.getSheetByName("Integration_Logs") || ss.insertSheet("Integration_Logs");
  var repairBinSheet = ss.getSheetByName("Repair_Bin") || ss.insertSheet("Repair_Bin");
  var archiveSheet = ss.getSheetByName("Archived_Logs") || ss.insertSheet("Archived_Logs");

  var headers = [
    "Timestamp", "Local Test Time", "Barcode", "DUT Type", "Co-tested Barcodes", "MAC Address", "Flash Size", "GPRS IMEI", "SIM CCID", "Tests Run", "Test Details", "QC Verdict", "Voltages", "Firmware Ver", "Verification Mode", "Duration (s)", "Tester", "Approved By"
  ];

  logSheet.getRange(1, 1, 1, headers.length).setValues([headers]);
  logSheet.getRange(1, 1, 1, headers.length).setFontWeight("bold").setBackground("#1e3a8a").setFontColor("#ffffff");
  logSheet.setFrozenRows(1);
  if (logSheet.getLastColumn() > headers.length) {
    logSheet.deleteColumns(headers.length + 1, logSheet.getLastColumn() - headers.length);
  }

  masterSheet.getRange(1, 1, 1, headers.length).setValues([headers]);
  masterSheet.getRange(1, 1, 1, headers.length).setFontWeight("bold").setBackground("#115e59").setFontColor("#ffffff");
  masterSheet.setFrozenRows(1);
  if (masterSheet.getLastColumn() > headers.length) {
    masterSheet.deleteColumns(headers.length + 1, masterSheet.getLastColumn() - headers.length);
  }

  var intHeaders = [
    "Timestamp", "Local Test Time", "Box 1 Barcode", "Box 2 Barcode", "ESP32 Barcode", "GPRS Barcode", "Nuvoton Barcode", 
    "Battery 1 Barcode", "Battery 2 Barcode", "MPPT Barcode", "MAC Address", "Firmware Ver", "Integration Verdict", "Tester", "Duration (s)", "Action Type", "Swap Reason"
  ];
  intSheet.getRange(1, 1, 1, intHeaders.length).setValues([intHeaders]);
  intSheet.getRange(1, 1, 1, intHeaders.length).setFontWeight("bold").setBackground("#4338ca").setFontColor("#ffffff");
  intSheet.setFrozenRows(1);

  var repairBinHeaders = ["Timestamp", "Board Barcode", "Board Type", "Source Box 1", "Source Box 2", "Failure Reason", "Status", "Tester"];
  repairBinSheet.getRange(1, 1, 1, repairBinHeaders.length).setValues([repairBinHeaders]);
  repairBinSheet.getRange(1, 1, 1, repairBinHeaders.length).setFontWeight("bold").setBackground("#b91c1c").setFontColor("#ffffff");
  repairBinSheet.setFrozenRows(1);

  var archiveHeaders = ["Archived Date", "Archived By", "Reason for Archive"].concat(headers);
  archiveSheet.getRange(1, 1, 1, archiveHeaders.length).setValues([archiveHeaders]);
  archiveSheet.getRange(1, 1, 1, archiveHeaders.length).setFontWeight("bold").setBackground("#6b7280").setFontColor("#ffffff");
  archiveSheet.setFrozenRows(1);
  if (archiveSheet.getLastColumn() > archiveHeaders.length) {
    archiveSheet.deleteColumns(archiveHeaders.length + 1, archiveSheet.getLastColumn() - archiveHeaders.length);
  }
}

var ADMIN_DELETE_PASSCODE = "SpatikaAdmin2026";
var AUTHORIZED_EMAIL = "satishv.spatika@gmail.com";

function openManageRecordsModal() {
  var ui = SpreadsheetApp.getUi();
  var activeUserEmail = Session.getActiveUser().getEmail();
  
  if (!activeUserEmail || activeUserEmail.toLowerCase() !== AUTHORIZED_EMAIL.toLowerCase()) {
    ui.alert(
      'Access Denied', 
      'Only ' + AUTHORIZED_EMAIL + ' is authorized to manage and delete records. Current user: ' + (activeUserEmail || 'Unknown/Unauthorized'), 
      ui.ButtonSet.OK
    );
    return;
  }

  var htmlString = getManageRecordsHtml();
  var html = HtmlService.createHtmlOutput(htmlString)
      .setWidth(750)
      .setHeight(500)
      .setTitle('Spatika Quality Record Manager');
  SpreadsheetApp.getUi().showModalDialog(html, 'Spatika Quality Record Manager');
}

function searchBoardLogs(passcode, barcode) {
  var activeUserEmail = Session.getActiveUser().getEmail();
  if (!activeUserEmail || activeUserEmail.toLowerCase() !== AUTHORIZED_EMAIL.toLowerCase()) {
    return { error: "Access Denied: You are not authorized to perform search queries." };
  }
  if (passcode !== ADMIN_DELETE_PASSCODE) {
    return { error: "Incorrect admin passcode." };
  }
  if (barcode === "TEST_CONNECTION") {
    return { success: true };
  }
  var ss = SpreadsheetApp.getActiveSpreadsheet();
  var logSheet = ss.getSheetByName("Test_Logs");
  if (!logSheet) return [];
  
  var lastRow = logSheet.getLastRow();
  if (lastRow < 2) return [];
  
  var rows = logSheet.getRange(2, 1, lastRow - 1, logSheet.getLastColumn()).getValues();
  var matches = [];
  
  for (var i = 0; i < rows.length; i++) {
    var row = rows[i];
    var rowBarcode = row[2] ? row[2].toString().trim() : "";
    var rowMac = row[5] ? row[5].toString().trim() : "";
    
    if (rowBarcode.toLowerCase() === barcode.toLowerCase() || rowMac.toLowerCase() === barcode.toLowerCase()) {
      matches.push({
        rowNum: i + 2,
        timestamp: row[0] instanceof Date ? row[0].toLocaleString() : row[0].toString(),
        localTime: row[1] || "--",
        barcode: rowBarcode,
        type: row[3] || "--",
        verdict: row[11] || "--",
        tester: row[16] || "--"
      });
    }
  }
  return matches;
}

function archiveRecordRow(passcode, rowNum, reason, adminName) {
  var activeUserEmail = Session.getActiveUser().getEmail();
  if (!activeUserEmail || activeUserEmail.toLowerCase() !== AUTHORIZED_EMAIL.toLowerCase()) {
    return { error: "Access Denied: You are not authorized to perform deletions." };
  }
  if (passcode !== ADMIN_DELETE_PASSCODE) {
    return { error: "Authorization failed." };
  }
  
  var ss = SpreadsheetApp.getActiveSpreadsheet();
  var logSheet = ss.getSheetByName("Test_Logs");
  var archiveSheet = ss.getSheetByName("Archived_Logs") || ss.insertSheet("Archived_Logs");
  
  if (!logSheet) return { error: "Test_Logs sheet not found." };
  
  var headers = [
    "Timestamp", "Local Test Time", "Barcode", "DUT Type", "Co-tested Barcodes", "MAC Address", "Flash Size", "GPRS IMEI", "SIM CCID", "Tests Run", "Test Details", "QC Verdict", "Voltages", "Firmware Ver", "Verification Mode", "Duration (s)", "Tester", "Approved By"
  ];
  var archiveHeaders = ["Archived Date", "Archived By", "Reason for Archive"].concat(headers);
  if (archiveSheet.getLastRow() === 0) {
    archiveSheet.appendRow(archiveHeaders);
    archiveSheet.getRange(1, 1, 1, archiveHeaders.length).setFontWeight("bold").setBackground("#6b7280").setFontColor("#ffffff");
    archiveSheet.setFrozenRows(1);
  }
  
  rowNum = parseInt(rowNum);
  if (rowNum < 2 || rowNum > logSheet.getLastRow()) {
    return { error: "Invalid row number: " + rowNum };
  }
  
  var rowRange = logSheet.getRange(rowNum, 1, 1, logSheet.getLastColumn());
  var rowValues = rowRange.getValues()[0];
  var barcode = rowValues[2] ? rowValues[2].toString().trim() : "";
  
  var archiveRow = [new Date(), adminName || "Admin", reason || "No reason"].concat(rowValues);
  archiveSheet.appendRow(archiveRow);
  
  logSheet.deleteRow(rowNum);
  updateMasterStatusForBarcode(ss, barcode);
  
  return { success: true, barcode: barcode };
}

function updateMasterStatusForBarcode(ss, barcode) {
  if (!barcode || barcode === "--" || barcode === "") return;
  
  var logSheet = ss.getSheetByName("Test_Logs");
  var masterSheet = ss.getSheetByName("Master_Status");
  if (!logSheet || !masterSheet) return;
  
  var logLastRow = logSheet.getLastRow();
  var masterLastRow = masterSheet.getLastRow();
  
  var remainingRuns = [];
  if (logLastRow > 1) {
    var logs = logSheet.getRange(2, 1, logLastRow - 1, logSheet.getLastColumn()).getValues();
    for (var i = 0; i < logs.length; i++) {
      if (logs[i][2] && logs[i][2].toString().trim().toLowerCase() === barcode.toLowerCase()) {
        remainingRuns.push({
          rowValues: logs[i],
          timestamp: logs[i][0] instanceof Date ? logs[i][0].getTime() : 0
        });
      }
    }
  }
  
  var masterRowIndex = -1;
  if (masterLastRow > 1) {
    var masterBarcodes = masterSheet.getRange(2, 3, masterLastRow - 1, 1).getValues();
    for (var m = 0; m < masterBarcodes.length; m++) {
      if (masterBarcodes[m][0] && masterBarcodes[m][0].toString().trim().toLowerCase() === barcode.toLowerCase()) {
        masterRowIndex = m + 2;
        break;
      }
    }
  }
  
  if (remainingRuns.length === 0) {
    if (masterRowIndex !== -1) {
      masterSheet.deleteRow(masterRowIndex);
    }
  } else {
    remainingRuns.sort(function(a, b) { return b.timestamp - a.timestamp; });
    var latestRun = remainingRuns[0].rowValues;
    
    if (masterRowIndex !== -1) {
      masterSheet.getRange(masterRowIndex, 1, 1, latestRun.length).setValues([latestRun]);
      colorVerdict(masterSheet, masterRowIndex, latestRun[11]);
    } else {
      masterSheet.appendRow(latestRun);
      colorVerdict(masterSheet, masterSheet.getLastRow(), latestRun[11]);
    }
  }
}

function getManageRecordsHtml() {
  return '<!DOCTYPE html><html><head><style>' +
    'body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif; background: #0f172a; color: #f8fafc; margin: 0; padding: 20px; }' +
    'h3 { margin-top: 0; color: #60a5fa; font-weight: 800; border-bottom: 1px solid #334155; padding-bottom: 10px; }' +
    '.card { background: #1e293b; border: 1px solid #334155; border-radius: 8px; padding: 15px; margin-bottom: 15px; box-shadow: 0 4px 6px -1px rgba(0,0,0,0.1); }' +
    'label { display: block; margin-bottom: 6px; font-size: 0.85rem; color: #94a3b8; font-weight: 600; }' +
    'input[type="text"], input[type="password"] { width: 100%; padding: 8px 12px; background: #0f172a; border: 1px solid #334155; border-radius: 6px; color: #f8fafc; font-size: 0.9rem; box-sizing: border-box; margin-bottom: 12px; }' +
    'input[type="text"]:focus, input[type="password"]:focus { border-color: #3b82f6; outline: none; }' +
    'button { background: #3b82f6; border: none; color: white; padding: 8px 16px; font-size: 0.9rem; font-weight: 600; border-radius: 6px; cursor: pointer; transition: background 0.2s; }' +
    'button:hover { background: #2563eb; }' +
    'button.archive-btn { background: #ef4444; padding: 4px 10px; font-size: 0.8rem; }' +
    'button.archive-btn:hover { background: #dc2626; }' +
    '.spinner { display: none; margin: 15px auto; width: 30px; height: 30px; border: 3px solid rgba(255,255,255,0.1); border-radius: 50%; border-top-color: #3b82f6; animation: spin 1s ease-in-out infinite; }' +
    '@keyframes spin { to { transform: rotate(360deg); } }' +
    'table { width: 100%; border-collapse: collapse; margin-top: 15px; font-size: 0.85rem; }' +
    'th, td { padding: 8px 12px; text-align: left; border-bottom: 1px solid #334155; }' +
    'th { background: #0f172a; color: #94a3b8; font-weight: 600; }' +
    '.badge { display: inline-block; padding: 2px 6px; border-radius: 4px; font-size: 0.75rem; font-weight: bold; }' +
    '.badge-pass { background: rgba(74,222,128,0.2); color: #4ade80; border: 1px solid rgba(74,222,128,0.4); }' +
    '.badge-fail { background: rgba(248,113,113,0.2); color: #f87171; border: 1px solid rgba(248,113,113,0.4); }' +
    '#authSection { max-width: 400px; margin: 40px auto; }' +
    '</style></head><body>' +
    '<div id="authSection" class="card">' +
    '<h3>🔒 Admin Verification</h3>' +
    '<label for="passcode">Enter Admin Passcode:</label>' +
    '<input type="password" id="passcode" placeholder="••••••••">' +
    '<button onclick="verifyPasscode()">Continue</button>' +
    '<div id="authError" style="color:#ef4444; margin-top:10px; font-size:0.85rem; font-weight:bold;"></div>' +
    '</div>' +
    '<div id="managerSection" style="display:none;">' +
    '<div class="card">' +
    '<h3>🛠️ Spatika Quality Record Manager</h3>' +
    '<div style="display:flex; gap:10px; align-items:flex-end;">' +
    '<div style="flex:1;">' +
    '<label for="searchBarcode">Scan or Type Board Barcode / MAC:</label>' +
    '<input type="text" id="searchBarcode" placeholder="e.g. 3001-E-06491" onkeydown="if(event.key===\\\'Enter\\\')searchLogs()">' +
    '</div>' +
    '<button onclick="searchLogs()" style="height:38px; margin-bottom:12px;">Search History</button>' +
    '</div>' +
    '</div>' +
    '<div class="spinner" id="loadingSpinner"></div>' +
    '<div id="resultsCard" class="card" style="display:none;">' +
    '<h4 style="margin: 0 0 10px; color: #94a3b8; font-size: 0.9rem;">Test Run History for Board</h4>' +
    '<div style="overflow-x:auto;">' +
    '<table id="historyTable">' +
    '<thead><tr><th>Date & Time</th><th>DUT Type</th><th>Tester</th><th>Verdict</th><th>Action</th></tr></thead>' +
    '<tbody id="historyBody"></tbody>' +
    '</table>' +
    '</div>' +
    '</div>' +
    '</div>' +
    '<script>' +
    'var currentPasscode = "";' +
    'function verifyPasscode() {' +
    '  var pass = document.getElementById("passcode").value.trim();' +
    '  if(!pass) return;' +
    '  currentPasscode = pass;' +
    '  document.getElementById("authError").textContent = "";' +
    '  document.getElementById("loadingSpinner").style.display = "block";' +
    '  google.script.run.withSuccessHandler(function(res) {' +
    '    document.getElementById("loadingSpinner").style.display = "none";' +
    '    if(res && res.error) {' +
    '      document.getElementById("authError").textContent = res.error;' +
    '    } else {' +
    '      document.getElementById("authSection").style.display = "none";' +
    '      document.getElementById("managerSection").style.display = "block";' +
    '      document.getElementById("searchBarcode").focus();' +
    '    }' +
    '  }).searchBoardLogs(currentPasscode, "TEST_CONNECTION");' +
    '}' +
    'function searchLogs() {' +
    '  var barcode = document.getElementById("searchBarcode").value.trim();' +
    '  if(!barcode) return;' +
    '  document.getElementById("resultsCard").style.display = "none";' +
    '  document.getElementById("loadingSpinner").style.display = "block";' +
    '  google.script.run.withSuccessHandler(function(runs) {' +
    '    document.getElementById("loadingSpinner").style.display = "none";' +
    '    var tbody = document.getElementById("historyBody");' +
    '    tbody.innerHTML = "";' +
    '    if(runs.error) {' +
    '      alert(runs.error);' +
    '      return;' +
    '    }' +
    '    if(!runs || runs.length === 0) {' +
    '      alert("No records found for that barcode.");' +
    '      return;' +
    '    }' +
    '    runs.forEach(function(run) {' +
    '      var tr = document.createElement("tr");' +
    '      var isPass = run.verdict.indexOf("PASS") === 0;' +
    '      var badgeClass = isPass ? "badge badge-pass" : "badge badge-fail";' +
    '      tr.innerHTML = ' +
    '        "<td>" + run.timestamp + "</td>" +' +
    '        "<td><b style=\\\'color:#e2e8f0; text-transform:uppercase;\\\'>" + run.type + "</b></td>" +' +
    '        "<td>" + run.tester + "</td>" +' +
    '        "<td><span class=\\\'" + badgeClass + "\\\'>" + run.verdict + "</span></td>" +' +
    '        "<td><button class=\\\'archive-btn\\\' onclick=\\\'archiveRun(" + run.rowNum + ", \\\\\"" + run.barcode + "\\\\\")\\\'>Archive Run</button></td>";' +
    '      tbody.appendChild(tr);' +
    '    });' +
    '    document.getElementById("resultsCard").style.display = "block";' +
    '  }).searchBoardLogs(currentPasscode, barcode);' +
    '}' +
    'function archiveRun(rowNum, barcode) {' +
    '  var admin = prompt("Enter your Name (for audit logs):");' +
    '  if(admin === null) return;' +
    '  admin = admin.trim();' +
    '  if(!admin) { alert("Admin name is required to archive."); return; }' +
    '  var reason = prompt("Enter reason for archiving this record:");' +
    '  if(reason === null) return;' +
    '  reason = reason.trim();' +
    '  if(!reason) { alert("Archive reason is required."); return; }' +
    '  if(!confirm("Are you sure you want to ARCHIVE row #" + rowNum + " from active logs? This action will update Master Status.")) return;' +
    '  document.getElementById("loadingSpinner").style.display = "block";' +
    '  google.script.run.withSuccessHandler(function(res) {' +
    '    document.getElementById("loadingSpinner").style.display = "none";' +
    '    if(res.error) {' +
    '      alert("Failed to archive: " + res.error);' +
    '    } else {' +
    '      alert("Record archived successfully. Master Status has been updated.");' +
    '      searchLogs();' +
    '    }' +
    '  }).archiveRecordRow(currentPasscode, rowNum, reason, admin);' +
    '}' +
    '</' + 'script></body></html>';
}
`;

    // Load URL from localstorage
    sheetUrlInput.value = localStorage.getItem("spatika_sheet_url") || "https://script.google.com/macros/s/AKfycbzWghxRhxCJYxrFIux2RoHXWJqD1kvRJ_dfCij_WtwAiLUE-TXbCHKK6mctjcaQ56af/exec";
    sheetUrlInput.addEventListener("change", (e) => {
      localStorage.setItem("spatika_sheet_url", e.target.value.trim());
      fetchRepairBinInventory();
      processOfflineQueue();
    });
    fetchRepairBinInventory();
    updateOfflineQueueUI();
    processOfflineQueue();

    // Monitor connectivity status and poll queue
    window.addEventListener("online", () => {
      logToConsole("🌐 Internet connected. Syncing offline records...", "info");
      processOfflineQueue();
    });
    setInterval(() => {
      const queue = getOfflineQueue();
      if (queue.length > 0 && navigator.onLine) {
        processOfflineQueue();
      }
    }, 20000);

    // Tester Info Inputs Setup
    const testerNameSelect = document.getElementById("testerNameSelect");
    const testerNameInput = document.getElementById("testerName");
    const auditModeSelect = document.getElementById("auditMode");

    if (testerNameSelect && testerNameInput) {
      // Load saved name or default to Guest
      const savedName = localStorage.getItem("spatika_tester_name") || "Guest";
      
      // Check if savedName is one of the dropdown options
      const isKnownOption = Array.from(testerNameSelect.options).some(opt => opt.value === savedName);
      
      if (isKnownOption) {
        testerNameSelect.value = savedName;
        testerNameInput.value = savedName;
        testerNameInput.style.display = "none";
      } else {
        testerNameSelect.value = "custom";
        testerNameInput.value = savedName;
        testerNameInput.style.display = "block";
      }

      testerNameSelect.addEventListener("change", (e) => {
        const val = e.target.value;
        if (val === "custom") {
          testerNameInput.value = "";
          testerNameInput.style.display = "block";
          testerNameInput.focus();
        } else {
          testerNameInput.value = val;
          testerNameInput.style.display = "none";
        }
        localStorage.setItem("spatika_tester_name", testerNameInput.value.trim());
        checkPreconditions();
      });

      testerNameInput.addEventListener("input", (e) => {
        localStorage.setItem("spatika_tester_name", e.target.value.trim());
        checkPreconditions();
      });
    }

    if (auditModeSelect) {
      auditModeSelect.value = localStorage.getItem("spatika_audit_mode") || "Standard QC Test";
      auditModeSelect.addEventListener("change", (e) => {
        localStorage.setItem("spatika_audit_mode", e.target.value);
      });
    }

    // ── Config Dropdown ─────────────────────────────────────────────────────
    const releaseConfigSel = document.getElementById("releaseConfig");
    const configLoadedBadge = document.getElementById("configLoadedBadge");

    function populateConfigDropdown() {
      const profile = document.getElementById("deviceProfile")?.value || "TRG";
      const options = PROFILE_CONFIGS[profile] || [];
      releaseConfigSel.innerHTML = options
        .map(c => `<option value="${c}">${c}</option>`)
        .join('');
      renderConfigMetadata();
    }

    function renderConfigMetadata() {
      const cfg = releaseConfigSel?.value;
      const entry = cfg ? customConfigVault[cfg] : null;

      // Update the LOADED badge in the navbar
      if (configLoadedBadge) {
        configLoadedBadge.style.display = (entry?.binary) ? "inline" : "none";
      }

      if (!entry || (!entry.binary && !entry.metadata)) {
        // No data yet — prompt user in console
        logBox([
          { text: `Config selected: ${cfg}`, type: "info" },
          `→ Select WEB_FLASH_FILES folder to load config.`
        ], "normal", 67);
        return;
      }

      const meta   = entry.metadata || {};
      const ver    = entry.version  || meta.full_version || "--";
      const yn     = v => (v === null || v === undefined) ? "--" : (v ? "YES" : "NO");
      const binKB  = entry.binary ? (entry.binary.byteLength / 1024).toFixed(1) + " KB" : "--";

      // Size check
      const binSize = entry.binary ? entry.binary.byteLength : (meta.binary_size_bytes || 0);
      const limit   = APP_SIZE_LIMIT_BYTES[currentChipSize] || APP_SIZE_LIMIT_BYTES[8];
      const sizeOK  = !currentChipSize || binSize === 0 || binSize <= limit;

      const isHealthReportOff = meta.enable_health_report === false;
      const healthFreqStr = isHealthReportOff ? "Don't Care" : (meta.health_report_freq || "--");

      const lines = [
        { text: `FIRMWARE BUILD INFO — ${cfg}  [${ver}]`, type: "info" },
        "---",
        `Debug Logs (DEBUG)                      : ${yn(meta.debug)}`,
        `Web Server (ENABLE_WEBSERVER)           : ${yn(meta.enable_webserver)}`,
        `Nuvoton UI (USE_NUVOTON_UI)             : ${yn(meta.use_nuvoton_ui)}`,
        `Health Report (ENABLE_HEALTH_REPORT)    : ${yn(meta.enable_health_report)}`,
        `Health Freq (TEST_HEALTH_DEFAULT)       : ${healthFreqStr}`,
        `RF Resolution (DEFAULT_RF_RESOLUTION)   : ${meta.rf_resolution_mm ?? "--"} mm`,
        `Pressure Sensor (ENABLE_PRESSURE_SENSOR): ${yn(meta.enable_pressure_sensor)}`,
        `Binary Size                             : ${binKB}`,
        `Build Time                              : ${meta.build_timestamp || "--"}`
      ];

      if (!sizeOK) {
        lines.push("---");
        lines.push({ text: `⚠ BINARY TOO LARGE for ${currentChipSize}MB chip! ${(binSize/1024).toFixed(0)}KB > ${(limit/1024).toFixed(0)}KB limit.`, type: "error" });
        lines.push({ text: `Use an 8MB or 16MB board for this config.`, type: "error" });
      }

      logBox(lines, "normal", 67);
    }

    // Server Scan hooks removed as auto-load from "./" is automatic

    // Force use of standard fallback directory picker (insecure/secure unified)
    const useDirectoryPicker = false;

    if (useDirectoryPicker) {
      btnSelectFolderAPI.style.display = "inline-flex";
      btnSelectFolderFallback.style.display = "none";

      btnSelectFolderAPI.addEventListener("click", async () => {
        await selectLocalDirectory();
      });
    } else {
      btnSelectFolderAPI.style.display = "none";
      btnSelectFolderFallback.style.display = "inline-flex";

      btnSelectFolderFallback.addEventListener("click", () => {
        logToConsole("Opening folder selection dialog...", "normal");
      });
    }

    // Fallback file input change listener (reads chosen folder files sequentially)
    fallbackFolderInput.addEventListener("change", async (e) => {
      const files = e.target.files;
      if (!files || files.length === 0) return;

      logToConsole(`Reading ${files.length} selected files...`, "normal");
      let loadedCount = 0;
      const configFileMap = {};

      for (const file of files) {
        const name = file.name;
        const path = (file.webkitRelativePath || "").replace(/\\/g, '/'); // Normalize separators

        // 1. Match version.txt at the root
        if (name === "version.txt") {
          try {
            const verText = (await file.text()).trim();
            metaFwVersion.textContent = verText;
            logToConsole(`✓ Detected firmware version: ${verText}`, "info");
          } catch (err) {
            logToConsole(`Failed to read version.txt: ${err.message}`, "error");
          }
          continue;
        }

        // 2. Match Top-Level Vault Files (by exact filename match)
        if (fileVault.hasOwnProperty(name)) {
          try {
            const buf = await file.arrayBuffer();
            fileVault[name] = new Uint8Array(buf);
            logToConsole(`✓ Loaded: ${name} (${(file.size / 1024).toFixed(1)} KB)`, "info");
            loadedCount++;
          } catch (err) {
            logToConsole(`Failed to read ${name}: ${err.message}`, "error");
          }
          continue;
        }

        // 2. Match Sub-Directory Configuration Files (e.g. KSNDMC_TRG_8mb/firmware.bin)
        if (path) {
          const parts = path.split('/');
          if (parts.length >= 2) {
            const parentFolder = parts[parts.length - 2];
            const configBase = parentFolder.replace(/_?(4|8|16)mb$/i, '');
            if (customConfigVault.hasOwnProperty(configBase)) {
              if (!configFileMap[configBase]) configFileMap[configBase] = {};
              if (name === 'firmware.bin')   configFileMap[configBase].firmware = file;
              if (name === 'fw_version.txt') configFileMap[configBase].version  = file;
              if (name === 'metadata.json')  configFileMap[configBase].metadata = file;
            }
          }
        }
      }

      // Load config-specific binaries
      let loadedConfigsCount = 0;
      for (const [cfg, cfgFiles] of Object.entries(configFileMap)) {
        if (cfgFiles.firmware) {
          try {
            const buf = await cfgFiles.firmware.arrayBuffer();
            customConfigVault[cfg].binary = new Uint8Array(buf);
            logToConsole(`✓ Config loaded: ${cfg}/firmware.bin (${(cfgFiles.firmware.size/1024).toFixed(1)} KB)`, "info");
            loadedConfigsCount++;
          } catch (err) {
            logToConsole(`Failed to read config ${cfg}: ${err.message}`, "error");
          }
        }
        if (cfgFiles.version) {
          try {
            customConfigVault[cfg].version = (await cfgFiles.version.text()).trim();
          } catch(_) {}
        }
        if (cfgFiles.metadata) {
          try {
            customConfigVault[cfg].metadata = JSON.parse(await cfgFiles.metadata.text());
          } catch(_) {}
        }
      }

      updateFileVaultDisplay();
      checkPreconditions();
      renderConfigMetadata();
      
      if (loadedCount > 0 || loadedConfigsCount > 0) {
        logToConsole(`Vault refresh completed. Loaded ${loadedCount} core files and ${loadedConfigsCount} config binaries.`, "info");
      } else {
        logToConsole("No matching firmware files found in the folder. Please select the WEB_FLASH_FILES folder.", "warning");
      }
      fallbackFolderInput.value = "";
    });

    // Manual individual files selection link click listener
    linkSelectFiles.addEventListener("click", (e) => {
      e.preventDefault();
      logToConsole("Opening file selection dialog for manual file selection...", "normal");
      fallbackFilesInput.click();
    });

    // Fallback files selection change listener
    fallbackFilesInput.addEventListener("change", async (e) => {
      const files = e.target.files;
      if (!files || files.length === 0) return;

      logToConsole(`Reading ${files.length} manually selected files...`, "normal");
      let loadedCount = 0;

      for (const file of files) {
        const name = file.name;
        if (name === "version.txt") {
          try {
            const verText = (await file.text()).trim();
            metaFwVersion.textContent = verText;
            logToConsole(`✓ Detected firmware version: ${verText}`, "info");
            loadedCount++;
          } catch (err) {
            logToConsole(`Failed to read version.txt: ${err.message}`, "error");
          }
        } else if (fileVault.hasOwnProperty(name)) {
          try {
            const buf = await file.arrayBuffer();
            fileVault[name] = new Uint8Array(buf);
            logToConsole(`✓ Loaded: ${name} (${(file.size / 1024).toFixed(1)} KB)`, "info");
            loadedCount++;
          } catch (err) {
            logToConsole(`Failed to read ${name}: ${err.message}`, "error");
          }
        } else {
          logToConsole(`⚠️ Ignored unexpected file: ${name}`, "warning");
        }
      }

      updateFileVaultDisplay();
      checkPreconditions();
      renderConfigMetadata();
      logToConsole(`Vault refresh completed. Loaded ${loadedCount} core binary files manually.`, "info");
      fallbackFilesInput.value = "";
    });

    // Drag and Drop support on the Step 1 card (cardVault)
    cardVault.addEventListener("dragover", (e) => {
      e.preventDefault();
      cardVault.style.border = "2px dashed var(--primary)";
      cardVault.style.background = "rgba(99, 102, 241, 0.08)";
    });

    cardVault.addEventListener("dragleave", (e) => {
      e.preventDefault();
      cardVault.style.border = "";
      cardVault.style.background = "";
    });

    cardVault.addEventListener("drop", async (e) => {
      e.preventDefault();
      cardVault.style.border = "";
      cardVault.style.background = "";
      
      const items = e.dataTransfer.items;
      if (!items || items.length === 0) return;

      logToConsole("Processing dropped files/folders...", "normal");
      
      const entries = [];
      for (let i = 0; i < items.length; i++) {
        const item = items[i];
        if (item.kind === 'file') {
          const entry = item.webkitGetAsEntry();
          if (entry) entries.push(entry);
        }
      }

      let loadedCount = 0;
      const configFileMap = {}; // configName -> { firmware: File, version: File, metadata: File }

      async function traverse(entry, relativePath = "") {
        if (entry.isFile) {
          const file = await new Promise((resolve, reject) => entry.file(resolve, reject));
          const name = file.name;
          const fullPath = relativePath ? relativePath + "/" + name : name;
          const parts = fullPath.split('/');

           // Top-level vault files (parts.length === 1 or parts.length === 2 with folder prefix like WEB_FLASH_FILES/)
           const isTopLevel = (parts.length === 1) || (parts.length === 2 && parts[0].toLowerCase() === 'web_flash_files');
           if (isTopLevel && name === "version.txt") {
             try {
               const verText = (await file.text()).trim();
               metaFwVersion.textContent = verText;
               logToConsole(`✓ Detected firmware version (Drop): ${verText}`, "info");
               loadedCount++;
             } catch (err) {
               logToConsole(`Failed to read version.txt: ${err.message}`, "error");
             }
           } else if (isTopLevel && fileVault.hasOwnProperty(name)) {
             try {
               const buf = await file.arrayBuffer();
               fileVault[name] = new Uint8Array(buf);
               logToConsole(`✓ Loaded (Drop): ${name} (${(file.size / 1024).toFixed(1)} KB)`, "info");
               loadedCount++;
             } catch (err) {
               logToConsole(`Failed to read ${name}: ${err.message}`, "error");
             }
           }

          // Sub-directory config files (e.g. BIHAR_TRG_8mb/firmware.bin)
          // parts.length === 2 or parts.length === 3 with folder prefix
          const isConfig = (parts.length === 2 && parts[0].toLowerCase() !== 'web_flash_files') || 
                           (parts.length === 3 && parts[0].toLowerCase() === 'web_flash_files');
          if (isConfig) {
            const configFolder = parts[parts.length - 2];
            const configBase = configFolder.replace(/_?(4|8|16)mb$/i, '');
            if (customConfigVault.hasOwnProperty(configBase)) {
              if (!configFileMap[configBase]) configFileMap[configBase] = {};
              if (name === 'firmware.bin')   configFileMap[configBase].firmware = file;
              if (name === 'fw_version.txt') configFileMap[configBase].version  = file;
              if (name === 'metadata.json')  configFileMap[configBase].metadata = file;
            }
          }
        } else if (entry.isDirectory) {
          const dirReader = entry.createReader();
          const readEntriesBatch = () => new Promise((resolve, reject) => {
            dirReader.readEntries(resolve, reject);
          });
          let dirEntries = [];
          let batch = await readEntriesBatch();
          while (batch.length > 0) {
            dirEntries = dirEntries.concat(batch);
            batch = await readEntriesBatch();
          }
          for (const subEntry of dirEntries) {
            await traverse(subEntry, relativePath ? relativePath + "/" + entry.name : entry.name);
          }
        }
      }

      for (const entry of entries) {
        await traverse(entry);
      }

      // Load config-specific binaries
      for (const [cfg, cfgFiles] of Object.entries(configFileMap)) {
        if (cfgFiles.firmware) {
          const buf = await cfgFiles.firmware.arrayBuffer();
          customConfigVault[cfg].binary = new Uint8Array(buf);
          logToConsole(`✓ Config loaded (Drop): ${cfg}/firmware.bin (${(cfgFiles.firmware.size/1024).toFixed(1)} KB)`, "info");
        }
        if (cfgFiles.version) {
          customConfigVault[cfg].version = (await cfgFiles.version.text()).trim();
        }
        if (cfgFiles.metadata) {
          try { customConfigVault[cfg].metadata = JSON.parse(await cfgFiles.metadata.text()); } catch(_) {}
        }
      }

      updateFileVaultDisplay();
      checkPreconditions();
      renderConfigMetadata();
      logToConsole(`Drop import completed. Loaded ${loadedCount} core binary files.`, "info");
    });

    chkEraseFlash.addEventListener("change", (e) => {
      logToConsole("[UI_EVENT] Erase Entire Flash option changed to: " + (e.target.checked ? "ENABLED" : "DISABLED"), "info");
      checkPreconditions();
    });

    // ── Helper: focus first barcode input for current tab after connect ──
    function focusFirstBarcodeInput() {
      if (!isConnected) return;
      setTimeout(() => {
        if (currentTab === "boardqc") {
          if (testCfgEsp.checked && document.getElementById("barcodeGroupEsp").style.display !== 'none') {
            barcodeEsp.focus();
            barcodeEsp.select();
          } else if (testCfgGprs.checked && document.getElementById("barcodeGroupGprs").style.display !== 'none') {
            barcodeGprs.focus();
            barcodeGprs.select();
          } else if (testCfgNuvoton.checked) {
            barcodeNuvoton.focus();
            barcodeNuvoton.select();
          }
        } else if (currentTab === "integration") {
          box1Barcode.focus();
          box1Barcode.select();
        } else if (currentTab === "repair") {
          repairBox1Barcode.focus();
          repairBox1Barcode.select();
        }
      }, 300);
    }

    // ── Guidance Banner: plain-language next action ────────────────
    function updateGuidanceBanner() {
      const banner  = document.getElementById("guidanceBanner");
      const icon    = document.getElementById("guidanceIcon");
      const main    = document.getElementById("guidanceMain");
      const sub     = document.getElementById("guidanceSub");
      if (!banner || !main) return;

      // Remove theme classes
      banner.className = "guidance-banner";

      if (currentTab === "flows" || currentTab === "repairbin") {
        banner.style.display = "none";
        return;
      }
      banner.style.display = "";

      const vault = getVaultStatus();
      const hasEspBarcode = barcodeEsp?.value.trim();
      const hasGprsBarcode = barcodeGprs?.value.trim();
      const hasNuvBarcode = barcodeNuvoton?.value.trim();
      const hasAllBarcodes = (!testCfgEsp?.checked || hasEspBarcode)
        && (!testCfgGprs?.checked || hasGprsBarcode)
        && (!testCfgNuvoton?.checked || hasNuvBarcode);

      if (!vault.ready && currentTab === "boardqc") {
        if (window.location.protocol === "file:") {
          // file:// — auto-load blocked by browser security, must pick folder manually
          banner.classList.add("guidance-action");
          icon.textContent = "📁";
          main.textContent = "Click \"📁 Select Firmware Folder\" in Step 1 on the left";
          sub.textContent = "Select the WEB_FLASH_FILES folder on this computer. The button is in the Step 1 card below.";
        } else if (!vaultAutoLoadAttempted) {
          // http:// — still loading, show spinner
          banner.classList.add("guidance-wait");
          icon.textContent = "⏳";
          main.textContent = "Loading test software automatically…";
          sub.textContent = "This takes a few seconds. If nothing happens after 10 seconds, click \"📁 Select Firmware Folder\" in the left panel.";
        } else {
          // http:// — load was attempted but failed (files missing or wrong path)
          banner.classList.add("guidance-action");
          icon.textContent = "⚠️";
          main.textContent = "Software did not load — click \"📁 Select Firmware Folder\" in Step 1";
          sub.textContent = "Auto-load failed. Click the button in the Step 1 card (left panel) and select your WEB_FLASH_FILES folder.";
        }
      } else if (!isConnected) {
        banner.classList.add("guidance-info");
        icon.textContent = "🔌";
        main.textContent = "Connect the board to this computer";
        sub.textContent = "Plug the board into the USB port, then click the \"Connect Board\" button on the left.";
      } else if (currentTab === "boardqc" && !hasAllBarcodes) {
        banner.classList.add("guidance-info");
        icon.textContent = "🏷️";
        main.textContent = "Scan the barcode stickers on each board";
        sub.textContent = "Use your barcode scanner on the white sticker. The cursor moves automatically to the next field.";
      } else if (currentTab === "boardqc" && hasAllBarcodes && !isFlashing) {
        banner.classList.add("guidance-action");
        icon.textContent = "🚀";
        main.textContent = "All set! Press \"Start Programming\" to begin";
        sub.textContent = "Make sure the correct option is selected (New Board = Full Test, Updated = Software Only).";
      } else if (isFlashing) {
        banner.classList.add("guidance-wait");
        icon.textContent = "⌛";
        main.textContent = "Testing in progress — please wait";
        sub.textContent = "Do not disconnect the board or close this window. Follow any instructions that appear on screen.";
      } else if (currentTab === "integration" && !box1Barcode?.value.trim()) {
        banner.classList.add("guidance-info");
        icon.textContent = "📦";
        main.textContent = "Scan Box 1 (DL Box) barcode first";
        sub.textContent = "This automatically loads the previous component history from the database.";
      } else if (currentTab === "repair" && !repairBox1Barcode?.value.trim()) {
        banner.classList.add("guidance-info");
        icon.textContent = "📦";
        main.textContent = "Scan Box 1 barcode to load the repair record";
        sub.textContent = "The previous component information will be auto-loaded so you only need to re-scan changed parts.";
      } else {
        banner.classList.add("guidance-ready");
        icon.textContent = "✅";
        main.textContent = "Everything is ready!";
        sub.textContent = "Complete the remaining fields, then click the action button in the left panel.";
      }
    }

    // ── Result Banner: big PASS/FAIL slide-up ──────────────────
    function showResultBanner(verdict) {
      // Build or retrieve the banner element
      let banner = document.getElementById("resultBanner");
      if (!banner) {
        banner = document.createElement("div");
        banner.id = "resultBanner";
        document.body.appendChild(banner);
      }

      window._lastQcResult = verdict;

      if (verdict === "PASS") {
        banner.className = "result-pass visible";
        banner.innerHTML = `
          <div class="result-icon">✅</div>
          <div>
            <div>BOARD PASSED — TEST COMPLETE</div>
            <div class="result-sub">Click \"Proceed to Flash App\" to install production software &bull; Click anywhere to dismiss</div>
          </div>`;
      } else {
        banner.className = "result-fail visible";
        banner.innerHTML = `
          <div class="result-icon">❌</div>
          <div>
            <div>BOARD FAILED — DO NOT PROCEED</div>
            <div class="result-sub">Download the QC Log and inform a supervisor &bull; Click anywhere to dismiss</div>
          </div>`;
      }

      // Dismiss on click
      banner.onclick = () => { banner.className = banner.className.replace(" visible", ""); };

      updateStepTracker();
      updateGuidanceBanner();
    }

    // ── Scan pulse animation on barcode inputs ─────────────────
    function addScanPulse(input) {
      if (!input) return;
      input.addEventListener("change", () => {
        if (input.value.trim()) {
          input.classList.add("scan-just-done");
          setTimeout(() => input.classList.remove("scan-just-done"), 700);
        }
      });
    }
    // Wire up on page load (after elements exist)
    addScanPulse(barcodeEsp);
    addScanPulse(barcodeGprs);
    addScanPulse(barcodeNuvoton);
    addScanPulse(box1Barcode);
    addScanPulse(box2Barcode);
    addScanPulse(assocEspBarcode);
    addScanPulse(assocGprsBarcode);
    addScanPulse(assocNuvBarcode);
    addScanPulse(assocMpptBarcode);
    addScanPulse(repairBox1Barcode);
    addScanPulse(repairEspBarcode);
    addScanPulse(repairGprsBarcode);
    addScanPulse(repairNuvBarcode);

    function updateStepTracker() {
      const tracker = document.getElementById("stepTracker");
      if (!tracker) return;
      
      const sep = document.querySelector(".control-sep");
      const vault = getVaultStatus();
      const testerNameVal = (document.getElementById("testerName")?.value || "").trim();

      let steps = [];

      if (currentTab === "boardqc") {
        const espEnabled = testCfgEsp?.checked;
        const hasBarcode = (espEnabled ? barcodeEsp.value.trim() : true)
          && (testCfgGprs?.checked ? barcodeGprs.value.trim() : true)
          && (testCfgNuvoton?.checked ? barcodeNuvoton.value.trim() : true);
        const isRunning = !cancelReadLoop && isConnected;
        const isDone = window._lastQcResult === 'PASS' || window._lastQcResult === 'FAIL';

        steps = [
          { label: "Firmware\nVault",  icon: "📦", done: vault.ready,       active: !vault.ready },
          { label: "Connect\nBoard",   icon: "🔌", done: isConnected,        active: vault.ready && !isConnected },
          { label: "Scan\nBarcodes",   icon: "🏷️", done: isConnected && hasBarcode, active: isConnected && !hasBarcode },
          { label: "Flash\n& Test",    icon: "🚀", done: isDone,            active: isConnected && hasBarcode && !isRunning && !isDone, running: isRunning },
          { label: "Done\n/ Sync",     icon: "✅", done: isDone,            active: false }
        ];
      } else if (currentTab === "integration") {
        const hasBox1 = box1Barcode?.value.trim();
        const hasBox2 = box2Barcode?.value.trim();
        const hasComps = assocEspBarcode?.value.trim() && assocGprsBarcode?.value.trim() && assocNuvBarcode?.value.trim() && assocMpptBarcode?.value.trim();
        const isDone = window._lastIntegrationResult === 'PASS';
        steps = [
          { label: "Connect\nBoard",   icon: "🔌", done: isConnected,          active: !isConnected },
          { label: "Scan\nBox 1",      icon: "📦", done: !!hasBox1,             active: isConnected && !hasBox1 },
          { label: "Scan\nBox 2 +\nParts", icon: "🏷️", done: !!hasComps,     active: !!hasBox1 && !hasComps },
          { label: "Flash\nProduction",icon: "⚡", done: isDone,               active: !!hasBox2 && !!hasComps && !isDone },
          { label: "Verify\n& Sync",   icon: "✅", done: isDone,               active: false }
        ];
      } else if (currentTab === "repair") {
        const hasBox1 = repairBox1Barcode?.value.trim();
        const hasComps = repairEspBarcode?.value.trim() && repairGprsBarcode?.value.trim() && repairNuvBarcode?.value.trim();
        const hasReason = repairReason?.value.trim().length >= 3;
        const isDone = window._lastRepairResult === 'PASS';
        steps = [
          { label: "Connect\nBoard",   icon: "🔌", done: isConnected,          active: !isConnected },
          { label: "Scan\nBox 1",      icon: "📦", done: !!hasBox1,             active: isConnected && !hasBox1 },
          { label: "Update\nComponents", icon: "🔄", done: !!hasComps,         active: !!hasBox1 && !hasComps },
          { label: "Re-Flash\nApp",    icon: "⚡", done: isDone,               active: !!hasComps && hasReason && !isDone },
          { label: "Done\n& Archive",  icon: "✅", done: isDone,               active: false }
        ];
      } else {
        tracker.innerHTML = "";
        if (sep) sep.style.display = "none";
        tracker.style.display = "none";
        return;
      }

      if (sep) sep.style.display = "";
      tracker.style.display = "flex";

      // Compute cumulative states: once a step is done, all prior are also done
      let pastActive = false;
      const rendered = steps.map((s, i) => {
        const isDoneStep = s.done;
        const isActiveStep = !pastActive && s.active;
        if (isActiveStep) pastActive = true;
        return { ...s, isDoneStep, isActiveStep };
      });

      let html = "";
      rendered.forEach((s, i) => {
        let circleClass = "";
        let content = s.icon;
        if (s.isDoneStep) circleClass = "done";
        else if (s.isActiveStep) circleClass = "active";
        else if (s.running) circleClass = "active";

        const labelHtml = s.label.replace(/\n/g, " ");
        html += `<div class="step-item ${circleClass}"><div class="step-circle">${content}</div><div class="step-label">${labelHtml}</div></div>`;
        if (i < rendered.length - 1) {
          html += `<div class="step-line"></div>`;
        }
      });
      tracker.innerHTML = html;
    }

    // Tab switching event handlers
    tabBoardQC.addEventListener("click", () => {
      if (isFlashing || isIntegrating) return;
      currentTab = "boardqc";
      tabBoardQC.classList.add("active");
      tabIntegration.classList.remove("active");
      tabRepair.classList.remove("active");
      tabRepairBin.classList.remove("active");
      if (tabFlows) tabFlows.classList.remove("active");
      if (panelFlows) panelFlows.classList.remove("visible");
      document.querySelector(".container").style.display = "";
      
      // Restore default columns
      colLeft.style.display = "";
      colMiddle.style.display = "";
      colRight.style.display = "";
      colRepairBin.style.display = "none";
      document.getElementById("stepTracker").style.display = "";
      
      // Toggle Left Panel Cards
      document.getElementById("cardSubsystems").style.display = "block";
      document.getElementById("cardBarcode").style.display = "block";
      cardProgramming.style.display = "block";
      cardIntegrationControls.style.display = "none";
      cardRepairControls.style.display = "none";
      document.getElementById("diagControlsGroup").style.display = "";
      
      // Toggle Right Panel Cards
      const cardChecklist = document.getElementById("cardChecklist");
      cardChecklist.style.display = "flex";
      cardChecklist.style.flexDirection = "column";
      cardIntegrationChecklist.style.display = "none";
      
      updateInventoryStatusBadges();
      checkPreconditions();
      updateStepTracker();
      if (isConnected) focusFirstBarcodeInput();
      logToConsole("Switched to Tab: Board-Level QC Mode", "info");
    });

    tabIntegration.addEventListener("click", () => {
      if (isFlashing || isIntegrating) return;
      currentTab = "integration";
      tabIntegration.classList.add("active");
      tabBoardQC.classList.remove("active");
      tabRepair.classList.remove("active");
      tabRepairBin.classList.remove("active");
      if (tabFlows) tabFlows.classList.remove("active");
      if (panelFlows) panelFlows.classList.remove("visible");
      document.querySelector(".container").style.display = "";
      
      // Restore default columns
      colLeft.style.display = "";
      colMiddle.style.display = "";
      colRight.style.display = "";
      colRepairBin.style.display = "none";
      document.getElementById("stepTracker").style.display = "";
      
      // Toggle Left Panel Cards
      document.getElementById("cardSubsystems").style.display = "none";
      document.getElementById("cardBarcode").style.display = "none";
      cardProgramming.style.display = "none";
      cardIntegrationControls.style.display = "block";
      cardRepairControls.style.display = "none";
      
      // Toggle Right Panel Cards
      document.getElementById("cardChecklist").style.display = "none";
      cardIntegrationChecklist.style.display = "flex";
      cardIntegrationChecklist.style.flexDirection = "column";
      cardIntegrationChecklist.querySelector('.card-title').textContent = "Integration & Boot Diagnostics";
      
      updateInventoryStatusBadges();
      fetchRepairBinInventory();
      checkPreconditions();
      updateStepTracker();
      if (isConnected) focusFirstBarcodeInput();
      logToConsole("Switched to Tab: Enclosure Integration Mode", "info");
    });

    tabRepair.addEventListener("click", () => {
      if (isFlashing || isIntegrating) return;
      currentTab = "repair";
      tabRepair.classList.add("active");
      tabBoardQC.classList.remove("active");
      tabIntegration.classList.remove("active");
      tabRepairBin.classList.remove("active");
      if (tabFlows) tabFlows.classList.remove("active");
      if (panelFlows) panelFlows.classList.remove("visible");
      document.querySelector(".container").style.display = "";
      
      // Restore default columns
      colLeft.style.display = "";
      colMiddle.style.display = "";
      colRight.style.display = "";
      colRepairBin.style.display = "none";
      document.getElementById("stepTracker").style.display = "";
      
      // Toggle Left Panel Cards
      document.getElementById("cardSubsystems").style.display = "none";
      document.getElementById("cardBarcode").style.display = "none";
      cardProgramming.style.display = "none";
      cardIntegrationControls.style.display = "none";
      cardRepairControls.style.display = "block";
      
      // Toggle Right Panel Cards
      document.getElementById("cardChecklist").style.display = "none";
      cardIntegrationChecklist.style.display = "flex";
      cardIntegrationChecklist.style.flexDirection = "column";
      cardIntegrationChecklist.querySelector('.card-title').textContent = "Repair & Boot Diagnostics";
      
      updateInventoryStatusBadges();
      fetchRepairBinInventory();
      checkPreconditions();
      updateStepTracker();
      if (isConnected) focusFirstBarcodeInput();
      logToConsole("Switched to Tab: Servicing & Repair Mode", "info");
    });

    tabRepairBin.addEventListener("click", () => {
      if (isFlashing || isIntegrating) return;
      currentTab = "repairbin";
      tabRepairBin.classList.add("active");
      tabBoardQC.classList.remove("active");
      tabIntegration.classList.remove("active");
      tabRepair.classList.remove("active");
      if (tabFlows) tabFlows.classList.remove("active");
      if (panelFlows) panelFlows.classList.remove("visible");
      document.querySelector(".container").style.display = "";
      document.getElementById("stepTracker").style.display = "none";
      
      // Toggle layout for Repair Bin (Full Width)
      colLeft.style.display = "none";
      colMiddle.style.display = "none";
      colRight.style.display = "none";
      colRepairBin.style.display = "flex";
      
      refreshRepairBinInventory();
      logToConsole("Switched to Tab: Failed Boards Bin", "info");
    });

    if (tabFlows) {
      tabFlows.addEventListener("click", () => {
        if (isFlashing || isIntegrating) return;
        currentTab = "flows";
        tabFlows.classList.add("active");
        tabBoardQC.classList.remove("active");
        tabIntegration.classList.remove("active");
        tabRepair.classList.remove("active");
        tabRepairBin.classList.remove("active");
        // Hide 3-col grid and repair bin; show flows panel
        document.querySelector(".container").style.display = "none";
        colRepairBin.style.display = "none";
        document.getElementById("stepTracker").style.display = "none";
        if (panelFlows) panelFlows.classList.add("visible");
        logToConsole("Switched to Tab: Flows Guide", "info");
      });
    }

    async function lookupBoxConfiguration(boxBarcode, isBox1 = true) {
      const url = sheetUrlInput.value.trim();
      if (!url || !boxBarcode) return;

      assocStatusText.innerHTML = `<span style="color:var(--text-muted);">🔍 Querying database for Box history...</span>`;
      
      try {
        const fetchUrl = `${url}?q=${encodeURIComponent(boxBarcode)}`;
        const response = await fetch(fetchUrl);
        if (!response.ok) throw new Error(`HTTP ${response.status}`);
        const results = await response.json();
        if (results.error) throw new Error(results.error);

        if (results.length === 0) {
          assocStatusText.innerHTML = `<span style="color:var(--warning);">⚠ No previous history found for Box "${boxBarcode}". Please enter barcodes manually.</span>`;
          return;
        }

        // Find the newest record with non-empty board barcodes
        const newestRecord = results.find(r => r["ESP32 Barcode"] || r["GPRS Barcode"] || r["Nuvoton Barcode"]);

        if (newestRecord) {
          // If we scanned Box 1, Box 2 comes from history. If we scanned Box 2, Box 1 comes from history.
          const finalBox1 = isBox1 ? boxBarcode : (newestRecord["Box 1 Barcode"] || "");
          const finalBox2 = !isBox1 ? boxBarcode : (newestRecord["Box 2 Barcode"] || "");

          box1Barcode.value = finalBox1;
          box2Barcode.value = finalBox2;
          assocEspBarcode.value = newestRecord["ESP32 Barcode"] || "";
          assocGprsBarcode.value = newestRecord["GPRS Barcode"] || "";
          assocNuvBarcode.value = newestRecord["Nuvoton Barcode"] || "";
          assocBatt1Barcode.value = newestRecord["Battery 1 Barcode"] || "";
          assocBatt2Barcode.value = newestRecord["Battery 2 Barcode"] || "";
          assocMpptBarcode.value = newestRecord["MPPT Barcode"] || "";

          assocStatusText.innerHTML = `<span style="color:var(--success);">✓ Loaded config from database (Last test: ${newestRecord["Local Test Time"] || "unknown"}).</span>`;
          logToConsole(`✓ Box: History loaded. ESP=${assocEspBarcode.value}, GPRS=${assocGprsBarcode.value}, NUV=${assocNuvBarcode.value}, B1=${assocBatt1Barcode.value}, B2=${assocBatt2Barcode.value}, MPPT=${assocMpptBarcode.value}`, "info");
        }
      } catch (err) {
        assocStatusText.innerHTML = `<span style="color:#f87171;">❌ History lookup failed: ${err.message}</span>`;
      } finally {
        checkPreconditions();
      }
    }

    box1Barcode.addEventListener("change", (e) => {
      const val = e.target.value.trim();
      if (val) lookupBoxConfiguration(val, true);
    });

    box1Barcode.addEventListener("keydown", (e) => {
      if (e.key === "Enter") {
        const val = e.target.value.trim();
        if (val) lookupBoxConfiguration(val, true);
      }
    });

    box2Barcode.addEventListener("change", (e) => {
      const val = e.target.value.trim();
      if (val) lookupBoxConfiguration(val, false);
    });

    box2Barcode.addEventListener("keydown", (e) => {
      if (e.key === "Enter") {
        const val = e.target.value.trim();
        if (val) lookupBoxConfiguration(val, false);
      }
    });

    assocEspBarcode.addEventListener("input", checkPreconditions);
    assocGprsBarcode.addEventListener("input", checkPreconditions);
    assocNuvBarcode.addEventListener("input", checkPreconditions);
    assocBatt1Barcode.addEventListener("input", checkPreconditions);
    assocBatt2Barcode.addEventListener("input", checkPreconditions);
    assocMpptBarcode.addEventListener("input", checkPreconditions);


    async function lookupRepairBoxConfiguration(boxBarcode, isBox1 = true) {
      const url = sheetUrlInput.value.trim();
      if (!url || !boxBarcode) return;

      repairStatusText.innerHTML = `<span style="color:var(--text-muted);">🔍 Querying database for Box history...</span>`;
      
      try {
        const fetchUrl = `${url}?q=${encodeURIComponent(boxBarcode)}`;
        const response = await fetch(fetchUrl);
        if (!response.ok) throw new Error(`HTTP ${response.status}`);
        const results = await response.json();
        if (results.error) throw new Error(results.error);

        if (results.length === 0) {
          repairStatusText.innerHTML = `<span style="color:var(--warning);">⚠ No previous history found for Box "${boxBarcode}". Please enter barcodes manually.</span>`;
          if (isBox1) {
            loadedRepairConfig.box1 = boxBarcode;
          } else {
            loadedRepairConfig.box2 = boxBarcode;
          }
          updateInventoryStatusBadges();
          return;
        }

        const newestRecord = results.find(r => r["ESP32 Barcode"] || r["GPRS Barcode"] || r["Nuvoton Barcode"]);

        if (newestRecord) {
          loadedRepairConfig = {
            box1: newestRecord["Box 1 Barcode"] || "",
            box2: newestRecord["Box 2 Barcode"] || "",
            esp: newestRecord["ESP32 Barcode"] || "",
            gprs: newestRecord["GPRS Barcode"] || "",
            nuv: newestRecord["Nuvoton Barcode"] || "",
            batt1: newestRecord["Battery 1 Barcode"] || "",
            batt2: newestRecord["Battery 2 Barcode"] || "",
            mppt: newestRecord["MPPT Barcode"] || ""
          };

          if (isBox1) {
            loadedRepairConfig.box1 = boxBarcode;
          } else {
            loadedRepairConfig.box2 = boxBarcode;
          }

          repairBox1Barcode.value = loadedRepairConfig.box1;
          repairBox2Barcode.value = loadedRepairConfig.box2;
          repairEspBarcode.value = loadedRepairConfig.esp;
          repairGprsBarcode.value = loadedRepairConfig.gprs;
          repairNuvBarcode.value = loadedRepairConfig.nuv;
          repairBatt1Barcode.value = loadedRepairConfig.batt1;
          repairBatt2Barcode.value = loadedRepairConfig.batt2;
          repairMpptBarcode.value = loadedRepairConfig.mppt;

          repairStatusText.innerHTML = `<span style="color:var(--success);">✓ Loaded config from database (Last test: ${newestRecord["Local Test Time"] || "unknown"}).</span>`;
          logToConsole(`✓ Repair: History loaded. ESP=${loadedRepairConfig.esp}, GPRS=${loadedRepairConfig.gprs}, NUV=${loadedRepairConfig.nuv}`, "info");
        } else {
          repairStatusText.innerHTML = `<span style="color:var(--warning);">⚠ No complete component history found for Box "${boxBarcode}".</span>`;
          if (isBox1) {
            loadedRepairConfig.box1 = boxBarcode;
          } else {
            loadedRepairConfig.box2 = boxBarcode;
          }
        }
      } catch (err) {
        repairStatusText.innerHTML = `<span style="color:#f87171;">❌ History lookup failed: ${err.message}</span>`;
      } finally {
        updateInventoryStatusBadges();
        checkPreconditions();
      }
    }

    let repairBinInventory = [];
    let hasInventoryBlocker = false;

    async function fetchRepairBinInventory() {
      const url = sheetUrlInput.value.trim();
      if (!url) return;
      try {
        const fetchUrl = `${url}?action=getRepairBin`;
        const response = await fetch(fetchUrl);
        if (response.ok) {
          repairBinInventory = await response.json();
          updateInventoryStatusBadges();
          checkPreconditions();
        }
      } catch (err) {
        console.error("Failed to fetch Repair Bin inventory in background:", err);
      }
    }

    function checkBarcodeInventoryStatus(barcode) {
      if (!barcode) return null;
      const clean = barcode.trim();
      if (!clean) return null;
      
      const matches = repairBinInventory.filter(item => 
        (item["Board Barcode"] && item["Board Barcode"].trim() === clean)
      );
      
      if (matches.length === 0) return null;
      
      // Sort matches to find the newest entry
      matches.sort((a, b) => {
        const idxA = parseInt(a.rowIndex) || 0;
        const idxB = parseInt(b.rowIndex) || 0;
        return idxB - idxA;
      });
      return matches[0];
    }

    function updateInventoryStatusBadges() {
      hasInventoryBlocker = false;
      
      // Clear alert boxes by default
      if (integrationAlertBox) integrationAlertBox.style.display = "none";
      if (repairAlertBox) repairAlertBox.style.display = "none";

      const validateField = (inputEl, badgeEl, originalVal, isRepairTab = false, typeName = "") => {
        if (!badgeEl) return;
        const val = inputEl.value.trim();
        if (!val) {
          badgeEl.textContent = "";
          badgeEl.style.background = "none";
          badgeEl.style.color = "transparent";
          badgeEl.style.border = "none";
          return;
        }

        // 1. Check if in Repair Bin
        const repairItem = checkBarcodeInventoryStatus(val);
        if (repairItem) {
          const status = repairItem["Status"] || "NEEDS_REPAIR";
          if (status === "NEEDS_REPAIR") {
            badgeEl.textContent = "⚠️ NEEDS REPAIR";
            badgeEl.style.background = "rgba(239,68,68,0.2)";
            badgeEl.style.color = "#f87171";
            badgeEl.style.border = "1px solid rgba(239,68,68,0.5)";
            
            // Show alert box
            const alertText = `Blocked: Scanned board <b>${val}</b> (${typeName}) is in the Repair Bin as <b>NEEDS REPAIR</b>.<br>Failure Reason: <i>${repairItem["Failure Reason"] || "No details provided"}</i>.<br>Please swap with a clean card or repair it first.`;
            if (isRepairTab) {
              repairAlertText.innerHTML = alertText;
              repairAlertBox.style.display = "block";
            } else {
              integrationAlertText.innerHTML = alertText;
              integrationAlertBox.style.display = "block";
            }
            hasInventoryBlocker = true;
            return;
          } else if (status === "SCRAPPED") {
            badgeEl.textContent = "❌ SCRAPPED";
            badgeEl.style.background = "rgba(127,29,29,0.3)";
            badgeEl.style.color = "#ef4444";
            badgeEl.style.border = "1px solid rgba(127,29,29,0.6)";
            
            // Show alert box
            const alertText = `Blocked: Scanned board <b>${val}</b> (${typeName}) is marked as <b>SCRAPPED</b>. This board is decommissioned and cannot be reused in any assembly.`;
            if (isRepairTab) {
              repairAlertText.innerHTML = alertText;
              repairAlertBox.style.display = "block";
            } else {
              integrationAlertText.innerHTML = alertText;
              integrationAlertBox.style.display = "block";
            }
            hasInventoryBlocker = true;
            return;
          } else if (status === "REPAIRED") {
            badgeEl.textContent = "✅ REPAIRED";
            badgeEl.style.background = "rgba(34,197,94,0.15)";
            badgeEl.style.color = "#4ade80";
            badgeEl.style.border = "1px solid rgba(34,197,94,0.4)";
            return;
          }
        }

        // 2. If not in Repair Bin, show normal badges
        if (isRepairTab) {
          if (originalVal && val !== originalVal) {
            badgeEl.textContent = "SWAPPED!";
            badgeEl.style.background = "rgba(239,68,68,0.2)";
            badgeEl.style.color = "#f87171";
            badgeEl.style.border = "1px solid rgba(239,68,68,0.4)";
          } else if (originalVal && val === originalVal) {
            badgeEl.textContent = "Original";
            badgeEl.style.background = "rgba(34,197,94,0.15)";
            badgeEl.style.color = "#4ade80";
            badgeEl.style.border = "1px solid rgba(34,197,94,0.3)";
          } else {
            badgeEl.textContent = "New Scan";
            badgeEl.style.background = "rgba(59,130,246,0.15)";
            badgeEl.style.color = "#60a5fa";
            badgeEl.style.border = "1px solid rgba(59,130,246,0.3)";
          }
        } else {
          badgeEl.textContent = "";
          badgeEl.style.background = "none";
          badgeEl.style.color = "transparent";
          badgeEl.style.border = "none";
        }
      };

      if (currentTab === "integration") {
        validateField(assocEspBarcode, document.getElementById("badgeAssocEsp"), "", false, "ESP32 Board");
        validateField(assocGprsBarcode, document.getElementById("badgeAssocGprs"), "", false, "GPRS Modem");
        validateField(assocNuvBarcode, document.getElementById("badgeAssocNuv"), "", false, "Nuvoton LCD");
        validateField(box2Barcode, document.getElementById("badgeAssocBox2"), "", false, "PS Enclosure");
        validateField(assocBatt1Barcode, document.getElementById("badgeAssocBatt1"), "", false, "Battery 1");
        validateField(assocBatt2Barcode, document.getElementById("badgeAssocBatt2"), "", false, "Battery 2");
        validateField(assocMpptBarcode, document.getElementById("badgeAssocMppt"), "", false, "MPPT Board");
      } else if (currentTab === "repair") {
        validateField(repairEspBarcode, badgeRepairEsp, loadedRepairConfig.esp, true, "ESP32 Board");
        validateField(repairGprsBarcode, badgeRepairGprs, loadedRepairConfig.gprs, true, "GPRS Modem");
        validateField(repairNuvBarcode, badgeRepairNuv, loadedRepairConfig.nuv, true, "Nuvoton LCD");
        validateField(repairBox2Barcode, badgeRepairBox2, loadedRepairConfig.box2, true, "PS Enclosure");
        validateField(repairBatt1Barcode, badgeRepairBatt1, loadedRepairConfig.batt1, true, "Battery 1");
        validateField(repairBatt2Barcode, badgeRepairBatt2, loadedRepairConfig.batt2, true, "Battery 2");
        validateField(repairMpptBarcode, badgeRepairMppt, loadedRepairConfig.mppt, true, "MPPT Board");
      }
    }

    let currentRepairBinItems = [];

    async function refreshRepairBinInventory() {
      const url = sheetUrlInput.value.trim();
      if (!url) {
        repairBinBody.innerHTML = `<tr><td colspan="8" style="text-align: center; padding: 2rem; color: var(--warning);">Please configure your Google Sheet Script URL.</td></tr>`;
        return;
      }
      
      repairBinBody.innerHTML = `<tr><td colspan="8" style="text-align: center; padding: 2rem; color: var(--text-muted);">🔍 Fetching failed boards inventory from Google Sheets...</td></tr>`;
      
      try {
        const fetchUrl = `${url}?action=getRepairBin`;
        const response = await fetch(fetchUrl);
        if (!response.ok) throw new Error(`HTTP ${response.status}`);
        const items = await response.json();
        
        currentRepairBinItems = items;
        repairBinInventory = items; // Update cache

        // Compute counts of active items (status === "NEEDS_REPAIR")
        const activeItems = items.filter(item => (item["Status"] || "NEEDS_REPAIR") === "NEEDS_REPAIR");
        const countAll = activeItems.length;
        const countEsp = activeItems.filter(item => item["Board Type"] && item["Board Type"].toLowerCase() === "esp32").length;
        const countGprs = activeItems.filter(item => item["Board Type"] && item["Board Type"].toLowerCase() === "gprs").length;
        const countNuv = activeItems.filter(item => item["Board Type"] && item["Board Type"].toLowerCase() === "nuvoton").length;
        const countMppt = activeItems.filter(item => item["Board Type"] && item["Board Type"].toLowerCase() === "mppt").length;
        const countBatt = activeItems.filter(item => item["Board Type"] && item["Board Type"].toLowerCase().includes("battery")).length;

        document.getElementById("countBinAll").textContent = countAll;
        document.getElementById("countBinEsp").textContent = countEsp;
        document.getElementById("countBinGprs").textContent = countGprs;
        document.getElementById("countBinNuv").textContent = countNuv;
        document.getElementById("countBinMppt").textContent = countMppt;
        document.getElementById("countBinBatt").textContent = countBatt;

        renderRepairBinTable(items);
        updateInventoryStatusBadges();
      } catch (err) {
        repairBinBody.innerHTML = `<tr><td colspan="8" style="text-align: center; padding: 2rem; color: #f87171;">❌ Failed to load inventory: ${err.message}</td></tr>`;
      }
    }

    function renderRepairBinTable(items) {
      if (!items || items.length === 0) {
        repairBinBody.innerHTML = `<tr><td colspan="8" style="text-align: center; padding: 2rem; color: var(--text-muted);">No failed boards currently in the Repair Bin.</td></tr>`;
        return;
      }

      // Render items based on active category filter from bin cards
      const activeFilterBtn = document.querySelector("#colRepairBin .active-bin");
      const activeFilterType = activeFilterBtn ? activeFilterBtn.dataset.type : "all";

      const filtered = items.filter(item => {
        if (activeFilterType === "all") return true;
        if (activeFilterType === "Battery") {
          return item["Board Type"] && item["Board Type"].toLowerCase().includes("battery");
        }
        return item["Board Type"] && item["Board Type"].toLowerCase() === activeFilterType.toLowerCase();
      });

      if (filtered.length === 0) {
        repairBinBody.innerHTML = `<tr><td colspan="8" style="text-align: center; padding: 2rem; color: var(--text-muted);">No failed boards in this category filter.</td></tr>`;
        return;
      }

      // Sort items by Timestamp descending
      filtered.sort((a, b) => {
        const timeA = new Date(a["Timestamp"]).getTime() || 0;
        const timeB = new Date(b["Timestamp"]).getTime() || 0;
        return timeB - timeA;
      });

      repairBinBody.innerHTML = filtered.map(item => {
        const dateStr = item["Timestamp"] || "--";
        const barcode = item["Board Barcode"] || "--";
        const type = item["Board Type"] || "--";
        const boxes = `Box 1: ${item["Source Box 1"] || "--"}<br>Box 2: ${item["Source Box 2"] || "--"}`;
        const reason = item["Failure Reason"] || "--";
        const status = item["Status"] || "NEEDS_REPAIR";
        const tester = item["Tester"] || "--";
        const rIndex = item["rowIndex"];

        let statusBadgeColor = "";
        let statusBadgeBg = "";
        if (status === "NEEDS_REPAIR") {
          statusBadgeColor = "#f87171";
          statusBadgeBg = "rgba(239,68,68,0.15)";
        } else if (status === "REPAIRED") {
          statusBadgeColor = "#4ade80";
          statusBadgeBg = "rgba(34,197,94,0.15)";
        } else {
          statusBadgeColor = "#94a3b8";
          statusBadgeBg = "rgba(148,163,184,0.15)";
        }

        return `
          <tr style="border-bottom: 1px solid rgba(255,255,255,0.05); hover:background:rgba(255,255,255,0.01);">
            <td style="padding: 0.6rem 0.45rem; color: var(--text-muted); font-size: 0.72rem;">${dateStr}</td>
            <td style="padding: 0.6rem 0.45rem; font-family: monospace; font-weight: 700; color: #fff;">${barcode}</td>
            <td style="padding: 0.6rem 0.45rem;"><span style="background:rgba(59,130,246,0.1); color:#60a5fa; padding:0.1rem 0.35rem; border-radius:0.25rem; font-size:0.7rem; font-weight:600;">${type}</span></td>
            <td style="padding: 0.6rem 0.45rem; font-size: 0.72rem; line-height: 1.3;">${boxes}</td>
            <td style="padding: 0.6rem 0.45rem; max-width: 220px; overflow: hidden; text-overflow: ellipsis; white-space: nowrap;" title="${reason}">${reason}</td>
            <td style="padding: 0.6rem 0.45rem;">
              <span style="color: ${statusBadgeColor}; background: ${statusBadgeBg}; padding: 0.15rem 0.45rem; border-radius: 0.3rem; font-weight: bold; border: 1px solid ${statusBadgeColor}33;">
                ${status}
              </span>
            </td>
            <td style="padding: 0.6rem 0.45rem; color: var(--text-muted); font-size: 0.72rem;">${tester}</td>
            <td style="padding: 0.6rem 0.45rem; text-align: center;">
              <select class="btn btn-secondary" style="font-size: 0.7rem; padding: 0.2rem 0.4rem; background: #0f172a; border-color: var(--card-border); color: #fff; width: auto; font-family: inherit;" onchange="updateRepairBinItemStatus(${rIndex}, this.value)">
                <option value="NEEDS_REPAIR" ${status === "NEEDS_REPAIR" ? "selected" : ""}>⚠️ Needs Repair</option>
                <option value="REPAIRED" ${status === "REPAIRED" ? "selected" : ""}>✅ Repaired</option>
                <option value="SCRAPPED" ${status === "SCRAPPED" ? "selected" : ""}>🗑️ Scrapped</option>
              </select>
            </td>
          </tr>
        `;
      }).join("");
    }

    async function updateRepairBinItemStatus(rowIndex, newStatus) {
      const url = sheetUrlInput.value.trim();
      if (!url) return;

      logToConsole(`Updating board repair status to ${newStatus} in sheet row ${rowIndex}...`, "info");
      
      try {
        const payload = {
          action: "updateRepairStatus",
          rowIndex: rowIndex,
          status: newStatus
        };

        await fetch(url, {
          method: "POST",
          mode: "no-cors",
          headers: {
            "Content-Type": "application/json"
          },
          body: JSON.stringify(payload)
        });

        logToConsole(`✓ Successfully updated status to ${newStatus}. Refreshing inventory...`, "info");
        
        // Update local items array
        if (currentRepairBinItems) {
          const item = currentRepairBinItems.find(it => it.rowIndex === rowIndex);
          if (item) item["Status"] = newStatus;
          repairBinInventory = currentRepairBinItems; // Sync cache
          renderRepairBinTable(currentRepairBinItems);
          updateInventoryStatusBadges();
          checkPreconditions();
        }
      } catch (err) {
        logToConsole(`Failed to update status: ${err.message}`, "error");
      }
    }

    window.updateRepairBinItemStatus = updateRepairBinItemStatus;

    repairBox1Barcode.addEventListener("change", (e) => {
      const val = e.target.value.trim();
      if (val) lookupRepairBoxConfiguration(val, true);
    });

    repairBox1Barcode.addEventListener("keydown", (e) => {
      if (e.key === "Enter") {
        const val = e.target.value.trim();
        if (val) lookupRepairBoxConfiguration(val, true);
      }
    });

    repairBox2Barcode.addEventListener("change", (e) => {
      const val = e.target.value.trim();
      if (val) lookupRepairBoxConfiguration(val, false);
    });

    repairBox2Barcode.addEventListener("keydown", (e) => {
      if (e.key === "Enter") {
        const val = e.target.value.trim();
        if (val) lookupRepairBoxConfiguration(val, false);
      }
    });

    // Update input change listeners to monitor repair bin blockers
    repairBox2Barcode.addEventListener("input", () => { updateInventoryStatusBadges(); checkPreconditions(); });
    repairEspBarcode.addEventListener("input", () => { updateInventoryStatusBadges(); checkPreconditions(); });
    repairGprsBarcode.addEventListener("input", () => { updateInventoryStatusBadges(); checkPreconditions(); });
    repairNuvBarcode.addEventListener("input", () => { updateInventoryStatusBadges(); checkPreconditions(); });
    repairBatt1Barcode.addEventListener("input", () => { updateInventoryStatusBadges(); checkPreconditions(); });
    repairBatt2Barcode.addEventListener("input", () => { updateInventoryStatusBadges(); checkPreconditions(); });
    repairMpptBarcode.addEventListener("input", () => { updateInventoryStatusBadges(); checkPreconditions(); });

    assocEspBarcode.addEventListener("input", () => { updateInventoryStatusBadges(); checkPreconditions(); });
    assocGprsBarcode.addEventListener("input", () => { updateInventoryStatusBadges(); checkPreconditions(); });
    assocNuvBarcode.addEventListener("input", () => { updateInventoryStatusBadges(); checkPreconditions(); });
    box2Barcode.addEventListener("input", () => { updateInventoryStatusBadges(); checkPreconditions(); });
    assocBatt1Barcode.addEventListener("input", () => { updateInventoryStatusBadges(); checkPreconditions(); });
    assocBatt2Barcode.addEventListener("input", () => { updateInventoryStatusBadges(); checkPreconditions(); });
    assocMpptBarcode.addEventListener("input", () => { updateInventoryStatusBadges(); checkPreconditions(); });

    repairReason.addEventListener("input", checkPreconditions);

    // Refresh Repair Bin listener
    if (btnRefreshRepairBin) {
      btnRefreshRepairBin.addEventListener("click", () => {
        refreshRepairBinInventory();
      });
    }

    // Bin Card category filters click event listeners
    document.querySelectorAll("#colRepairBin .bin-card").forEach(card => {
      card.addEventListener("click", () => {
        // Remove active class from all bin cards
        document.querySelectorAll("#colRepairBin .bin-card").forEach(c => c.classList.remove("active-bin"));
        
        // Add active class to clicked card
        card.classList.add("active-bin");
        
        // Render filtered table
        renderRepairBinTable(currentRepairBinItems);
      });
    });

    btnSyncSetup.addEventListener("click", () => {
      preCode.textContent = appsScriptCode;
      codeModal.style.display = "flex";
    });

    btnInitializeSheet.addEventListener("click", async () => {
      const url = sheetUrlInput.value.trim();
      if (!url) {
        alert("Please configure your Google Apps Script Web App URL first!");
        return;
      }
      
      if (!confirm("Are you sure you want to initialize/reset the headers in your Google Sheet? This will check or create 'Test_Logs' and 'Master_Status' tabs and format their header rows.")) {
        return;
      }

      btnInitializeSheet.disabled = true;
      const originalText = btnInitializeSheet.textContent;
      btnInitializeSheet.textContent = "⚡ Initializing Sheets...";
      logToConsole("Sending initialization signal to Google Sheets Web App...", "normal");

      try {
        await fetch(url, {
          method: "POST",
          mode: "no-cors",
          headers: {
            "Content-Type": "application/json"
          },
          body: JSON.stringify({ initOnly: true })
        });
        
        logToConsole("✓ Initialization signal sent! The 'Test_Logs' and 'Master_Status' tabs will now be populated/refreshed with styled headers.", "info");
        alert("Initialization command sent successfully! Please check your Google Sheet tabs.");
      } catch (err) {
        logToConsole(`Failed to send initialization command: ${err.message}`, "error");
        alert(`Failed to initialize: ${err.message}`);
      } finally {
        btnInitializeSheet.disabled = false;
        btnInitializeSheet.textContent = originalText;
      }
    });

    btnCloseModal.addEventListener("click", () => {
      codeModal.style.display = "none";
    });

    codeModal.addEventListener("click", (e) => {
      if (e.target === codeModal) {
        codeModal.style.display = "none";
      }
    });

    // --- Help Modal Events ---
    btnHelp.addEventListener("click", () => {
      helpModal.style.display = "flex";
    });

    btnCloseHelp.addEventListener("click", () => {
      helpModal.style.display = "none";
    });

    helpModal.addEventListener("click", (e) => {
      if (e.target === helpModal) {
        helpModal.style.display = "none";
      }
    });

    // --- History Search Logic ---
    const txtSearchQuery   = document.getElementById("txtSearchQuery");
    const btnSearchHistory = document.getElementById("btnSearchHistory");
    const searchStatusLine = document.getElementById("searchStatusLine");
    const historyModal     = document.getElementById("historyModal");
    const historyModalSub  = document.getElementById("historyModalSubtitle");
    const historyModalBody = document.getElementById("historyModalBody");
    const btnCloseHistory  = document.getElementById("btnCloseHistory");
    function buildHistoryCard(rec, index, total) {
      if (rec.logType === "integration") {
        const verdict = rec["Integration Verdict"] || "UNKNOWN";
        const isPass = verdict.startsWith("PASS");
        const vColor = verdictColor(verdict);
        const borderCol = isPass ? "rgba(99,102,241,0.4)" : "rgba(248,113,113,0.4)";
        const bgGlow = isPass ? "rgba(99,102,241,0.04)" : "rgba(248,113,113,0.04)";
        const action = rec["Action Type"] || "ORIGINAL_BUILD";
        
        return `
          <div style="background:${bgGlow}; border:1.5px solid ${borderCol}; border-radius:0.65rem; padding:1.1rem; position:relative; display:flex; flex-direction:column; gap:0.6rem; box-shadow:0 4px 15px rgba(0,0,0,0.15);">
            <!-- Header -->
            <div style="display:flex; align-items:center; gap:0.6rem; flex-wrap:wrap;">
              <span style="font-size:0.95rem; font-weight:800; color:${vColor}; padding:0.15rem 0.5rem; background:rgba(255,255,255,0.03); border-radius:0.3rem; border:1px solid ${borderCol};">📦 INTEGRATION ${verdict}</span>
              <span style="background:rgba(167,139,250,0.15); color:#c084fc; font-size:0.64rem; font-weight:700; padding:0.15rem 0.5rem; border-radius:99px; border:1px solid rgba(167,139,250,0.3); letter-spacing:0.04em;">${action}</span>
              <span style="margin-left:auto; font-size:0.72rem; color:#64748b;">${rec["Local Test Time"] || rec["Timestamp"] || "--"}</span>
            </div>
            
            <!-- Box Identifiers -->
            <div style="display:flex; gap:1.2rem; flex-wrap:wrap; font-size:0.78rem; border-bottom: 1px solid rgba(255,255,255,0.05); padding-bottom: 0.5rem;">
              <div><span style="color:#64748b;">Box 1 (Datalogger): </span><code style="color:#fff; font-weight:700; font-size:0.75rem;">${rec["Box 1 Barcode"] || "--"}</code></div>
              <div><span style="color:#64748b;">Box 2 (Power Unit):  </span><code style="color:#fff; font-weight:700; font-size:0.75rem;">${rec["Box 2 Barcode"] || "--"}</code></div>
            </div>

            <!-- Genealogy Tree View -->
            <div style="font-size:0.75rem; color:#94a3b8; font-family:monospace; background:rgba(0,0,0,0.22); border-radius:0.4rem; padding:0.6rem 0.8rem; border:1px solid rgba(255,255,255,0.04);">
              <div style="color:#fff; font-weight:700; margin-bottom:0.25rem;">📦 Integrated System Parts:</div>
              <div style="margin-left:0.5rem; color:#a5b4fc; font-weight:600; margin-bottom:0.15rem;">📦 DL Box (Box 1):</div>
              <div style="margin-left:0.5rem;">├── 🧠 ESP32 Board: <code style="color:#38bdf8;">${rec["ESP32 Barcode"] || "--"}</code></div>
              <div style="margin-left:0.5rem;">├── 📶 GPRS Board:  <code style="color:#34d399;">${rec["GPRS Barcode"] || "--"}</code></div>
              <div style="margin-left:0.5rem; margin-bottom:0.35rem;">└── 📟 Nuvoton LCD: <code style="color:#fbbf24;">${rec["Nuvoton Barcode"] || "--"}</code></div>
              
              <div style="margin-left:0.5rem; color:#4ade80; font-weight:600; margin-bottom:0.15rem;">⚡ PS Box (Box 2):</div>
              <div style="margin-left:0.5rem;">├── 🔋 Battery 1:   <code style="color:#cbd5e1;">${rec["Battery 1 Barcode"] || "--"}</code></div>
              <div style="margin-left:0.5rem;">├── 🔋 Battery 2:   <code style="color:#cbd5e1;">${rec["Battery 2 Barcode"] || "--"}</code></div>
              <div style="margin-left:0.5rem;">└── 🔌 MPPT Board:  <code style="color:#cbd5e1;">${rec["MPPT Barcode"] || "--"}</code></div>
            </div>

            <!-- Meta details -->
            <div style="display:flex; gap:1.2rem; flex-wrap:wrap; font-size:0.72rem; color:#64748b; margin-top:0.15rem;">
              <div>Tester: <b style="color:#e2e8f0;">${rec["Tester"] || "--"}</b></div>
              <div>MAC: <code style="color:#e2e8f0; font-size:0.7rem;">${rec["MAC Address"] || "--"}</code></div>
              <div>Firmware: <b style="color:#e2e8f0;">${rec["Firmware Ver"] || "--"}</b></div>
              <div>Duration: <b style="color:#fbbf24;">${rec["Duration (s)"] ? rec["Duration (s)"] + " s" : "--"}</b></div>
            </div>
          </div>
        `;
      }

      // Check if it's the new individual board schema
      const isNewSchema = rec["Barcode"] !== undefined || rec["DUT Type"] !== undefined;
      
      // Perform backward-compatibility mapping for old records
      if (!isNewSchema) {
        rec["DUT Type"] = rec["ESP32 Barcode"] && rec["ESP32 Barcode"] !== "--" ? "ESP32" : 
                          rec["GPRS Barcode"] && rec["GPRS Barcode"] !== "--" ? "GPRS" : "Nuvoton";
        rec["Barcode"] = rec["ESP32 Barcode"] && rec["ESP32 Barcode"] !== "--" ? rec["ESP32 Barcode"] : 
                         rec["GPRS Barcode"] && rec["GPRS Barcode"] !== "--" ? rec["GPRS Barcode"] : rec["Nuvoton Barcode"];
        rec["Tests Run"] = rec["DUT Type"] === "ESP32" ? "SPIFFS, SD Card, RTC, WiFi, Sensor" :
                           rec["DUT Type"] === "GPRS" ? "Cellular" : "Nuvoton LCD, Keypad, Rainfall";
        
        if (rec["DUT Type"] === "ESP32") {
          rec["Test Details"] = `SPIFFS: ${rec["SPIFFS"] || "--"}, SD: ${rec["SD Card"] || "--"}, RTC: ${rec["RTC"] || "--"}, WiFi: ${rec["WiFi Scanner"] || "--"}, Sensor: ${rec["Env Sensor"] || "--"}`;
        } else if (rec["DUT Type"] === "GPRS") {
          rec["Test Details"] = `Cellular: ${rec["Cellular"] || "--"}`;
        } else {
          rec["Test Details"] = `Nuvoton: ${rec["Nuvoton LCD"] || "--"}, LCD: ${rec["LCD / UI"] || "--"}, Keypad: ${rec["Keypad"] || "--"}, Rainfall: ${rec["Rainfall (Tips)"] || "--"}`;
        }
        rec["Co-tested Barcodes"] = [rec["ESP32 Barcode"], rec["GPRS Barcode"], rec["Nuvoton Barcode"]]
                                    .filter(x => x && x !== "--" && x !== rec["Barcode"])
                                    .join(", ") || "--";
        rec["Voltages"] = `ESP32: ${rec["Battery (V)"] || "--"} | GPRS: ${rec["Solar (V)"] || "--"}`;
      }

      const isPass    = rec["QC Verdict"] && rec["QC Verdict"].startsWith("PASS");
      const verdict   = rec["QC Verdict"] || "UNKNOWN";
      const vColor    = verdictColor(verdict);
      const isLatest  = index === 0;
      const borderCol = isPass ? "rgba(74,222,128,0.3)" : "rgba(248,113,113,0.3)";
      const bgGlow    = isPass ? "rgba(74,222,128,0.04)" : "rgba(248,113,113,0.04)";

      return `
        <div style="background:${bgGlow}; border:1.5px solid ${borderCol}; border-radius:0.65rem; padding:1rem 1.1rem; position:relative; display:flex; flex-direction:column; gap:0.6rem;">
          
          <!-- Top row: verdict badge + timestamp + Latest tag -->
          <div style="display:flex; align-items:center; gap:0.6rem; flex-wrap:wrap;">
            <span style="font-size:1.05rem; font-weight:800; color:${vColor}; padding:0.15rem 0.5rem; background:rgba(255,255,255,0.02); border-radius:0.3rem; border:1px solid ${borderCol};">${verdict}</span>
            ${isLatest ? `<span style="background:rgba(59,130,246,0.2); color:#60a5fa; font-size:0.64rem; font-weight:700; padding:0.15rem 0.5rem; border-radius:99px; border:1px solid rgba(59,130,246,0.4); letter-spacing:0.05em;">LATEST</span>` : `<span style="background:rgba(255,255,255,0.05); color:#64748b; font-size:0.64rem; font-weight:600; padding:0.15rem 0.5rem; border-radius:99px;">#${total - index} of ${total}</span>`}
            <span style="margin-left:auto; font-size:0.72rem; color:#64748b;">${rec["Local Test Time"] || rec["Timestamp"] || "--"}</span>
          </div>

          <!-- Identity block -->
          <div style="display:grid; grid-template-columns:repeat(auto-fill, minmax(200px,1fr)); gap:0.3rem 1.5rem; padding:0.6rem 0.75rem; background:rgba(255,255,255,0.03); border-radius:0.4rem; border:1px solid rgba(255,255,255,0.04);">
            <div style="font-size:0.72rem;"><span style="color:#64748b;">DUT Type: </span><b style="color:#e2e8f0; text-transform:uppercase;">${rec["DUT Type"] || "--"}</b></div>
            <div style="font-size:0.72rem;"><span style="color:#64748b;">Barcode: </span><code style="color:#a5b4fc; font-weight:700; font-size:0.75rem;">${rec["Barcode"] || "--"}</code></div>
            <div style="font-size:0.72rem;"><span style="color:#64748b;">Tester: </span><b style="color:#e2e8f0;">${rec["Tester"] || "--"}</b></div>
            <div style="font-size:0.72rem;"><span style="color:#64748b;">Mode: </span><b style="color:#e2e8f0;">${rec["Verification Mode"] || "--"}</b></div>
            <div style="font-size:0.72rem;"><span style="color:#64748b;">Duration: </span><b style="color:#fbbf24;">${rec["Duration (s)"] && rec["Duration (s)"] !== "--" ? rec["Duration (s)"] + " s" : "--"}</b></div>
            <div style="font-size:0.72rem;"><span style="color:#64748b;">Firmware: </span><b style="color:#e2e8f0;">${rec["Firmware Ver"] || "--"}</b></div>
            ${rec["MAC Address"] && rec["MAC Address"] !== "--" ? `<div style="font-size:0.72rem;"><span style="color:#64748b;">MAC: </span><code style="color:#38bdf8; font-size:0.7rem;">${rec["MAC Address"]}</code></div>` : ""}
            ${rec["Flash Size"] && rec["Flash Size"] !== "--" ? `<div style="font-size:0.72rem;"><span style="color:#64748b;">Flash: </span><b style="color:#e2e8f0;">${rec["Flash Size"]}</b></div>` : ""}
            ${rec["GPRS IMEI"] && rec["GPRS IMEI"] !== "--" ? `<div style="font-size:0.72rem;"><span style="color:#64748b;">IMEI: </span><code style="color:#e2e8f0; font-size:0.7rem;">${rec["GPRS IMEI"]}</code></div>` : ""}
            ${rec["SIM CCID"] && rec["SIM CCID"] !== "--" ? `<div style="font-size:0.72rem;"><span style="color:#64748b;">SIM CCID: </span><code style="color:#e2e8f0; font-size:0.7rem;">${rec["SIM CCID"]}</code></div>` : ""}
          </div>

          <!-- Co-tested Companion Info -->
          <div style="font-size:0.72rem; padding:0.4rem 0.75rem; background:rgba(99,102,241,0.06); border:1px solid rgba(99,102,241,0.15); border-radius:0.4rem; color:#a5b4fc;">
            💬 <span style="font-weight:700;">Co-tested Barcodes:</span> <code style="color:#fff; font-size:0.75rem;">${rec["Co-tested Barcodes"] || "--"}</code>
          </div>

          <!-- Diagnostics details -->
          <div style="font-size:0.75rem; background:rgba(0,0,0,0.18); border:1px solid rgba(255,255,255,0.04); border-radius:0.4rem; padding:0.7rem 0.9rem; display:flex; flex-direction:column; gap:0.4rem;">
            <div style="color:#94a3b8; font-weight:700; font-size:0.68rem; text-transform:uppercase; letter-spacing:0.04em;">📋 Test Details:</div>
            <div><span style="color:#64748b;">Tests Performed:</span> <span style="color:#e2e8f0; font-weight:600;">${rec["Tests Run"] || "--"}</span></div>
            <div><span style="color:#64748b;">Diagnostic Logs:</span> <span style="color:#38bdf8; font-family:monospace; font-size:0.72rem;">${rec["Test Details"] || "--"}</span></div>
            <div><span style="color:#64748b;">Voltages Logged:</span> <span style="color:#fbbf24; font-family:monospace; font-size:0.72rem;">${rec["Voltages"] || rec["Voltages Logged"] || "--"}</span></div>
          </div>

          <!-- Approved By footer -->
          ${rec["Approved By"] ? `<div style="font-size:0.7rem; color:#64748b; border-top:1px solid rgba(255,255,255,0.04); padding-top:0.4rem;">✅ Approved by: <b style="color:#a78bfa;">${rec["Approved By"]}</b></div>` : ""}
        </div>
      `;
    }


    async function executeHistorySearch() {
      const q   = txtSearchQuery.value.trim();
      const url = sheetUrlInput.value.trim();

      if (!url) {
        searchStatusLine.innerHTML = `<span style="color:#f87171;">⚠ No Script URL configured.</span>`;
        return;
      }
      if (!q) {
        searchStatusLine.innerHTML = `<span style="color:#fbbf24;">Enter a barcode, MAC, or IMEI.</span>`;
        return;
      }

      searchStatusLine.innerHTML = `<span style="color:var(--text-muted);">🔍 Searching...</span>`;

      try {
        const fetchUrl = `${url}?q=${encodeURIComponent(q)}`;
        const response = await fetch(fetchUrl);
        if (!response.ok) throw new Error(`HTTP ${response.status}`);
        const results = await response.json();
        if (results.error) throw new Error(results.error);

        if (results.length === 0) {
          searchStatusLine.innerHTML = `<span style="color:#fca5a5;">⚠ No records found for "${q}".</span>`;
          return;
        }

        // Build modal content
        const total = results.length;
        historyModalSub.textContent = `${total} record${total > 1 ? "s" : ""} found for "${q}" — newest first`;
        historyModalBody.innerHTML  = results.map((rec, i) => buildHistoryCard(rec, i, total)).join("");

        // Update status line with summary + reopen link
        const passCount = results.filter(r => r["QC Verdict"]?.startsWith("PASS")).length;
        const failCount = total - passCount;
        searchStatusLine.innerHTML =
          `<span style="color:#4ade80; font-weight:700;">${passCount} PASS</span>` +
          (failCount ? `&nbsp;·&nbsp;<span style="color:#f87171; font-weight:700;">${failCount} FAIL</span>` : "") +
          `&nbsp;·&nbsp;<span style="color:#60a5fa; cursor:pointer; text-decoration:underline;" id="lnkOpenHistory">View all ▸</span>`;

        historyModal.style.display = "flex";

        setTimeout(() => {
          document.getElementById("lnkOpenHistory")?.addEventListener("click", () => {
            historyModal.style.display = "flex";
          });
        }, 50);

      } catch (err) {
        searchStatusLine.innerHTML = `<span style="color:#f87171;">❌ Search failed: ${err.message}</span>`;
      }
    }

    btnSearchHistory.addEventListener("click", executeHistorySearch);
    txtSearchQuery.addEventListener("keydown", (e) => {
      if (e.key === "Enter") executeHistorySearch();
    });

    btnCloseHistory.addEventListener("click", () => {
      historyModal.style.display = "none";
    });

    historyModal.addEventListener("click", (e) => {
      if (e.target === historyModal) {
        historyModal.style.display = "none";
      }
    });


    // --- Spec Modal Events ---
    btnSpec.addEventListener("click", () => {
      specModal.style.display = "flex";
    });

    btnCloseSpec.addEventListener("click", () => {
      specModal.style.display = "none";
    });

    specModal.addEventListener("click", (e) => {
      if (e.target === specModal) {
        specModal.style.display = "none";
      }
    });

    // --- Audio Synthesis Chimes ---
    let audioCtx = null;
    function playTone(freq, type, duration, delayTime = 0) {
      try {
        if (!audioCtx) {
          audioCtx = new (window.AudioContext || window.webkitAudioContext)();
        }
        setTimeout(() => {
          if (audioCtx.state === 'suspended') {
            audioCtx.resume();
          }
          const osc = audioCtx.createOscillator();
          const gain = audioCtx.createGain();
          
          osc.type = type || 'sine';
          osc.frequency.setValueAtTime(freq, audioCtx.currentTime);
          
          gain.gain.setValueAtTime(0.08, audioCtx.currentTime);
          gain.gain.exponentialRampToValueAtTime(0.0001, audioCtx.currentTime + duration);
          
          osc.connect(gain);
          gain.connect(audioCtx.destination);
          
          osc.start();
          osc.stop(audioCtx.currentTime + duration);
        }, delayTime);
      } catch (e) {
        console.warn("Audio synthesis not supported or blocked: ", e);
      }
    }

    function playSuccessChime() {
      playTone(523.25, 'sine', 0.15, 0);   // C5
      playTone(659.25, 'sine', 0.15, 80);  // E5
      playTone(783.99, 'sine', 0.15, 160); // G5
      playTone(1046.50, 'sine', 0.35, 240); // C6
    }

    function playFailureChime() {
      playTone(220.00, 'sawtooth', 0.22, 0);   // A3
      playTone(220.00, 'sawtooth', 0.22, 120); // A3
    }

    function playAlertChime() {
      playTone(587.33, 'triangle', 0.15, 0);  // D5
      playTone(880.00, 'triangle', 0.25, 80); // A5
    }

    // --- Tester Session Tally Counters ---
    let passCount = parseInt(localStorage.getItem("spatika_tally_pass") || "0");
    let failCount = parseInt(localStorage.getItem("spatika_tally_fail") || "0");

    function updateTallyDisplay() {
      document.getElementById("tallyPass").textContent = passCount;
      document.getElementById("tallyFail").textContent = failCount;
      const total = passCount + failCount;
      const yieldPct = total > 0 ? Math.round((passCount / total) * 100) : 0;
      document.getElementById("tallyYield").textContent = `${yieldPct}%`;
    }

    document.getElementById("btnResetTally").addEventListener("click", (e) => {
      e.stopPropagation();
      if (confirm("Reset yield counters for this session?")) {
        passCount = 0;
        failCount = 0;
        localStorage.setItem("spatika_tally_pass", "0");
        localStorage.setItem("spatika_tally_fail", "0");
        updateTallyDisplay();
      }
    });

    function incrementPassTally() {
      passCount++;
      localStorage.setItem("spatika_tally_pass", passCount);
      updateTallyDisplay();
    }

    function incrementFailTally() {
      failCount++;
      localStorage.setItem("spatika_tally_fail", failCount);
      updateTallyDisplay();
    }

    // Initialize display on load
    updateTallyDisplay();

    btnCloseVerdict.addEventListener("click", () => {
      verdictOverlay.style.display = "none";
    });

    // --- Barcode Input Usability Helpers ---
    function setupFocusChain(inputs) {
      inputs.forEach((input, index) => {
        if (!input) return;
        
        // Auto-select text on focus (operator re-scans directly to overwrite)
        input.addEventListener("focus", () => {
          input.select();
        });
        
        // Advance to next visible input on Enter keypress
        input.addEventListener("keydown", (e) => {
          if (e.key === "Enter") {
            e.preventDefault(); // Stop default action (avoid triggering starts)
            for (let i = index + 1; i < inputs.length; i++) {
              const nextInput = inputs[i];
              if (nextInput && nextInput.getBoundingClientRect().width > 0) {
                nextInput.focus();
                nextInput.select();
                return;
              }
            }
          }
        });
      });
    }

    // --- Operator Interactive Action Modal Helpers ---
    let currentOperatorPromptKeyHandler = null;

    function showOperatorPrompt(htmlContent, attachListenersFn) {
      const modal = document.getElementById("operatorPromptModal");
      const content = document.getElementById("operatorPromptContent");
      if (!modal || !content) return;
      
      content.innerHTML = htmlContent;
      modal.style.display = "flex";
      
      // Clean up previous event listener if any
      if (currentOperatorPromptKeyHandler) {
        window.removeEventListener("keydown", currentOperatorPromptKeyHandler);
        currentOperatorPromptKeyHandler = null;
      }

      // Map computer keyboard keys (Y / N / Space / Enter / Escape) to prompt override buttons
      currentOperatorPromptKeyHandler = function(e) {
        // Avoid intercepting if user is focused inside a barcode input or text area
        if (document.activeElement && (document.activeElement.tagName === "INPUT" || document.activeElement.tagName === "TEXTAREA")) {
          return;
        }

        const btns = content.querySelectorAll("button");
        if (btns.length === 0) return;

        const key = e.key.toLowerCase();
        
        // Y / Enter / Space -> Approve / Force Pass / Yes
        if (key === "y" || e.key === "Enter" || e.key === " ") {
          e.preventDefault();
          btns[0].click();
        }
        // N / Escape -> Reject / Force Fail / No
        else if (key === "n" || e.key === "Escape") {
          e.preventDefault();
          if (btns.length > 1) {
            btns[1].click();
          } else {
            // For single button modals (e.g. Deep sleep force wakeup), trigger the main override
            btns[0].click();
          }
        }
      };

      window.addEventListener("keydown", currentOperatorPromptKeyHandler);
      
      if (typeof attachListenersFn === "function") {
        setTimeout(attachListenersFn, 50);
      }
    }

    function hideOperatorPrompt() {
      const modal = document.getElementById("operatorPromptModal");
      if (modal) {
        modal.style.display = "none";
      }
      if (currentOperatorPromptKeyHandler) {
        window.removeEventListener("keydown", currentOperatorPromptKeyHandler);
        currentOperatorPromptKeyHandler = null;
      }
    }

    // --- Dynamic Interactive Overrides Helpers ---
    function getKeypadSweepHTML(keyName) {
      return `
        ⌨️ <b>Keypad Sweep</b>: Press the <b>${keyName}</b> key on the keypad.<br><br>
        <div style="display:flex; gap:0.4rem; align-items:center; margin-top:0.4rem;">
          <button class="btn btn-secondary" id="btnOverrideKeypadPass" style="font-size:0.72rem; padding:0.25rem 0.5rem; width:auto; text-transform:none; border-color:var(--success-glow); color:#a7f3d0;">✓ Skip/Pass Sweep [Y]</button>
          <button class="btn btn-secondary" id="btnOverrideKeypadFail" style="font-size:0.72rem; padding:0.25rem 0.5rem; width:auto; text-transform:none; border-color:var(--danger-glow); color:#fca5a5;">✗ Force Fail [N]</button>
        </div>
      `;
    }

    function attachKeypadOverrideListeners() {
      setTimeout(() => {
        document.getElementById("btnOverrideKeypadPass")?.addEventListener("click", () => {
          logToConsole("Dashboard Override: Keypad Pass sent.", "info");
          sendSerialCommand("CMD:KEYPAD_PASS\n");
          hideOperatorPrompt();
        });
        document.getElementById("btnOverrideKeypadFail")?.addEventListener("click", () => {
          logToConsole("Dashboard Override: Keypad Fail.", "error");
          window._qcHasFailedInSession = true;
          setCheckState(chkKeypad, valKeypad, "fail", "FAILED (Manual)");
          showVerdict("FAIL", "Keypad sweep failed manually by operator override.");
          autoSyncToGoogleSheets("FAIL: Keypad Manual");
          hideOperatorPrompt();
        });
      }, 50);
    }

    function showVerdict(verdict, details) {
      hideOperatorPrompt();
      if (testStartTime) {
        testDuration = Math.round((Date.now() - testStartTime) / 1000);
      }
      
      verdictTitle.textContent = verdict === "PASS" ? "BOARD PASS" : "BOARD FAIL";
      
      const timeStr = `Test completed in <b>${testDuration}s</b>.`;
      verdictSub.innerHTML = `${details ? details + '<br><br>' : ''}${timeStr}`;
      
      lastVerdict = verdict;
      
      if (verdict === "PASS") {
        verdictTitle.style.color = "var(--success)";
        verdictBox.style.borderColor = "var(--success)";
        verdictBox.style.boxShadow = "0 0 40px var(--success-glow)";
        playSuccessChime();
        incrementPassTally();
      } else {
        verdictTitle.style.color = "var(--danger)";
        verdictBox.style.borderColor = "var(--danger)";
        verdictBox.style.boxShadow = "0 0 40px var(--danger-glow)";
        playFailureChime();
        incrementFailTally();
      }
      verdictOverlay.style.display = "flex";
    }

    btnCopyCode.addEventListener("click", async () => {
      try {
        await navigator.clipboard.writeText(appsScriptCode);
        btnCopyCode.textContent = "✓ Copied to Clipboard!";
        btnCopyCode.className = "btn btn-success";
        setTimeout(() => {
          btnCopyCode.textContent = "📋 Copy Code to Clipboard";
          btnCopyCode.className = "btn btn-primary";
        }, 2000);
      } catch (err) {
        logToConsole(`Failed to copy: ${err.message}`, "error");
      }
    });

    // Offline Sync Queue Helpers
    function getOfflineQueue() {
      try {
        return JSON.parse(localStorage.getItem("spatika_sync_queue")) || [];
      } catch (e) {
        return [];
      }
    }

    function saveOfflineQueue(queue) {
      localStorage.setItem("spatika_sync_queue", JSON.stringify(queue));
      updateOfflineQueueUI();
    }

    function updateOfflineQueueUI() {
      if (!btnOfflineSync || !offlineSyncCount) return;
      const queue = getOfflineQueue();
      if (queue.length > 0) {
        offlineSyncCount.textContent = queue.length;
        btnOfflineSync.style.display = "inline-flex";
      } else {
        btnOfflineSync.style.display = "none";
      }
    }

    function addToOfflineQueue(payload, syncType, url) {
      const queue = getOfflineQueue();
      const serialized = JSON.stringify(payload);
      if (queue.some(item => JSON.stringify(item.payload) === serialized)) {
        return;
      }
      queue.push({
        id: Date.now() + "_" + Math.random().toString(36).substring(2, 6),
        payload: payload,
        syncType: syncType,
        url: url
      });
      saveOfflineQueue(queue);
      logToConsole(`⚠️ Network offline. Sync queued locally (Pending: ${queue.length}).`, "warning");
    }

    async function processOfflineQueue() {
      if (isSyncingQueue) return;
      const queue = getOfflineQueue();
      if (queue.length === 0) return;
      
      isSyncingQueue = true;
      logToConsole(`🔄 Connecting... Syncing ${queue.length} offline records...`, "info");
      
      let successfulIds = [];
      for (const item of queue) {
        try {
          await fetch(item.url, {
            method: "POST",
            mode: "no-cors",
            headers: {
              "Content-Type": "application/json"
            },
            body: JSON.stringify(item.payload)
          });
          successfulIds.push(item.id);
          logToConsole(`✓ Offline record synced to Google Sheets.`, "info");
        } catch (err) {
          logToConsole(`❌ Sync failed: ${err.message}. Offline queue paused.`, "error");
          break;
        }
      }
      
      if (successfulIds.length > 0) {
        const remaining = queue.filter(item => !successfulIds.includes(item.id));
        saveOfflineQueue(remaining);
      }
      isSyncingQueue = false;
    }

    // Auto-Sync Function
    async function autoSyncToGoogleSheets(verdict) {
      const url = sheetUrlInput.value.trim();
      if (!url) {
        logToConsole("Google Sheet sync skipped (No Script URL configured).", "warning");
        return false;
      }
      
      logToConsole("Syncing test results to Google Sheet...", "normal");
      
      const espVal = testCfgEsp.checked ? "ESP" : "";
      const gprsVal = testCfgGprs.checked ? "GPRS" : "";
      const nuvVal = testCfgNuvoton.checked ? "NUV" : "";
      const profileVal = document.getElementById("deviceProfile")?.value || "TRG";
      const subsystemsTested = [espVal, gprsVal, nuvVal].filter(Boolean).join("+");
      const finalVerdict = `${verdict} (${profileVal})`;
 
      const payload = {
        testTime:  new Date().toLocaleString(),
        espBarcode: testCfgEsp.checked ? barcodeEsp.value.trim() : "",
        gprsBarcode: testCfgGprs.checked ? barcodeGprs.value.trim() : "",
        nuvotonBarcode: testCfgNuvoton.checked ? barcodeNuvoton.value.trim() : "",
        mac:       metaMac.textContent,
        flashSize: metaFlashSize.textContent,
        imei:      metaImei.textContent,
        ccid:      metaCcid.textContent,
        nuvoton:   metaNuvoton.textContent,
        spiffs:    valSpiffs.textContent,
        sd:        valSd.textContent,
        rtc:       valRtc.textContent,
        wifi:      valWifi.textContent,
        sensor:    valSensor.textContent,
        windSpeed: valWindSpd.textContent,
        windDir:   valWindDir.textContent,
        cellular:  valGprs.textContent,
        lcd:       valLcd.textContent,
        keypad:    valKeypad.textContent,
        rainRf:    valRf.textContent,
        sleepWake: valSleep.textContent,
        battery:   `ESP32: ${valSys3v3.textContent} | GPRS: ${valBatt.textContent}`,
        solar:     valSolar.textContent,
        version:   metaFwVersion.textContent,
        verdict:   finalVerdict,
        profile:   profileVal,
        tester:    document.getElementById("testerName")?.value.trim() || "--",
        auditMode: document.getElementById("auditMode")?.value || "--",
        duration:  testDuration || 0,
        boardsUnderTest: subsystemsTested
      };
      
      try {
        await fetch(url, {
          method: "POST",
          mode: "no-cors",
          headers: {
            "Content-Type": "application/json"
          },
          body: JSON.stringify(payload)
        });
        logToConsole("✓ Successfully synced data to Google Sheet!", "info");
        return true;
      } catch (err) {
        logToConsole(`Google Sheets sync failed: ${err.message}`, "error");
        addToOfflineQueue(payload, "board_qc", url);
        return false;
      }
    }

    // Check for WebSerial support
    if (!("serial" in navigator)) {
      browserWarning.style.display = "flex";
      btnConnToggle.disabled = true;
    }

    // --- UI Helper: Terminal Logger ---
    // Phase heading rules: [triggerSubstring, headingLabel, color]
    const PHASE_TRIGGERS = [
      ["SPATIKA AIO BOARD QC TEST",  "⚡ PHASE 1 — BOARD BOOT",            "#60a5fa"],
      ["Testing SPIFFS",              "🔬 PHASE 2 — PERIPHERAL SWEEP",      "#34d399"],
      ["BATT_3V7_ADC",               "⚡ PHASE 2b — ADC / POWER RAILS",    "#fbbf24"],
      ["Initializing GPRS Modem",    "📡 PHASE 3 — CELLULAR MODEM",        "#a78bfa"],
      ["Verifying Nuvoton",          "📟 PHASE 4 — LCD & NUVOTON BOARD",   "#f472b6"],
      ["[KEYPAD_WAIT]",              "⌨️  PHASE 5 — OPERATOR INTERACTION",  "#fb923c"],
      ["QC_RESULT: PASS",            "✅ QC COMPLETE — BOARD PASSED",      "#4ade80"],
    ];
    const _shownPhases = new Set();

    function logToConsole(message, type = "normal") {
      // Inject phase separator heading if this line triggers a new phase
      for (const [trigger, label, color] of PHASE_TRIGGERS) {
        if (!_shownPhases.has(trigger) && message.includes(trigger)) {
          _shownPhases.add(trigger);
          const sep = document.createElement("div");
          sep.style.cssText = `margin: 0.6rem 0 0.25rem; padding: 0.25rem 0.5rem; border-left: 3px solid ${color}; color:${color}; font-weight:700; font-size:0.72rem; letter-spacing:0.06em; background:rgba(255,255,255,0.03); border-radius:0 3px 3px 0;`;
          sep.textContent = label;
          consoleDiv.appendChild(sep);
          break;
        }
      }

      const p = document.createElement("div");
      if (type === "error") p.className = "console-err";
      else if (type === "warning") p.className = "console-warn";
      else if (type === "info") p.className = "console-info";

      // Color YES green and NO red if they appear next to a colon
      if (message.includes(": YES") || message.includes(": NO")) {
        let lastIdx = 0;
        const regex = /(:\s*)(YES|NO)\b/g;
        let match;
        while ((match = regex.exec(message)) !== null) {
          if (match.index > lastIdx) {
            p.appendChild(document.createTextNode(message.substring(lastIdx, match.index)));
          }
          p.appendChild(document.createTextNode(match[1]));
          const span = document.createElement("span");
          span.textContent = match[2];
          span.style.fontWeight = "700";
          if (match[2] === "YES") {
            span.style.color = "hsl(140, 80%, 65%)"; // Vibrant green
          } else {
            span.style.color = "hsl(3, 85%, 65%)"; // Vibrant red
          }
          p.appendChild(span);
          lastIdx = regex.lastIndex;
        }
        if (lastIdx < message.length) {
          p.appendChild(document.createTextNode(message.substring(lastIdx)));
        }
      } else {
        p.textContent = message;
      }
      
      consoleDiv.appendChild(p);
      consoleDiv.scrollTop = consoleDiv.scrollHeight;
      
      // Save for export logs
      const timestamp = new Date().toISOString().substring(11, 19);
      rawSerialLog += `[${timestamp}] ${message}\n`;
    }

    function logBox(lines, type = "normal", width = 78) {
      const top = "┌" + "─".repeat(width - 2) + "┐";
      const bottom = "└" + "─".repeat(width - 2) + "┘";
      const sep = "├" + "─".repeat(width - 2) + "┤";

      logToConsole(top, type);
      for (const line of lines) {
        let lineText = typeof line === "object" ? line.text : line;
        let lineType = typeof line === "object" ? line.type : type;

        if (lineText === "---") {
          logToConsole(sep, type);
        } else {
          const innerWidth = width - 6; // │  ...  │
          let cleanLine = lineText;
          let leftBorder = "│  ";
          let rightBorder = "  │";
          
          if (cleanLine.startsWith("│  ")) {
            cleanLine = cleanLine.substring(3);
          } else if (cleanLine.startsWith("│ ")) {
            cleanLine = cleanLine.substring(2);
          }
          if (cleanLine.endsWith("  │")) {
            cleanLine = cleanLine.slice(0, -3);
          } else if (cleanLine.endsWith(" │")) {
            cleanLine = cleanLine.slice(0, -2);
          } else if (cleanLine.endsWith("│")) {
            cleanLine = cleanLine.slice(0, -1);
          }
          
          cleanLine = cleanLine.trimEnd();
          if (cleanLine.length > innerWidth) {
            cleanLine = cleanLine.substring(0, innerWidth);
          }
          const padded = cleanLine.padEnd(innerWidth, " ");
          logToConsole(leftBorder + padded + rightBorder, lineType);
        }
      }
      logToConsole(bottom, type);
    }

    btnConsoleClear.addEventListener("click", () => {
      consoleDiv.innerHTML = "";
      rawSerialLog = "";
      _shownPhases.clear();
    });

    btnConsoleCopy.addEventListener("click", async () => {
      // Collect text from all child nodes (strips HTML, preserves phase labels)
      const lines = Array.from(consoleDiv.childNodes).map(n => n.textContent).join("\n");
      try {
        await navigator.clipboard.writeText(lines);
        btnConsoleCopy.textContent = "✅ Copied!";
        setTimeout(() => { btnConsoleCopy.textContent = "📋 Copy"; }, 1800);
      } catch (e) {
        btnConsoleCopy.textContent = "⚠ Failed";
        setTimeout(() => { btnConsoleCopy.textContent = "📋 Copy"; }, 1800);
      }
    });

    // --- Drag & Drop Binary Handling ---
    function updateFileVaultDisplay() {
      fileList.innerHTML = "";
      Object.keys(fileVault).forEach(filename => {
        const item = document.createElement("div");
        item.className = "file-item";
        
        const nameSpan = document.createElement("span");
        nameSpan.className = "file-name";
        nameSpan.textContent = filename;
        
        const statusSpan = document.createElement("span");
        statusSpan.className = "file-status";
        if (fileVault[filename]) {
          statusSpan.textContent = "LOADED";
          statusSpan.className += " status-loaded";
        } else {
          statusSpan.textContent = "MISSING";
          statusSpan.className += " status-missing";
        }
        
        item.appendChild(nameSpan);
        item.appendChild(statusSpan);
        fileList.appendChild(item);
      });
      
      checkPreconditions();
    }

    // Initialize Vault display and trigger auto-load
    updateFileVaultDisplay();
    tryAutoLoadBinaries();

    // Restore Google Sheet card collapsed state (default to collapsed if not set)
    (function() {
      const sheetCollapsedVal = localStorage.getItem('sheetCardCollapsed');
      const collapsed = (sheetCollapsedVal === null || sheetCollapsedVal === '1');
      const body = document.getElementById('sheetCardBody');
      const arrow = document.getElementById('sheetCardArrow');
      if (body) {
        body.style.display = collapsed ? 'none' : '';
        if (arrow) arrow.textContent = collapsed ? '▶' : '▼';
      }
    })();

    // Drag & drop logic removed for simplicity

    // Option selection UI toggles
    flowOptions.forEach(opt => {
      opt.addEventListener("click", () => {
        flowOptions.forEach(o => o.classList.remove("active"));
        opt.classList.add("active");
        opt.querySelector("input").checked = true;
        activeFlow = opt.getAttribute("data-flow");
        const flowNames = { factory: 'Option 1 (Factory Flash)', upgrade: 'Option 2 (App Update)', gprs_debug: 'Option 3 (GPRS Debug)' };
        logToConsole(`Selected Mode: ${flowNames[activeFlow] || activeFlow}`, "info");
        // Hide force-flash checkbox for non-factory flows
        forceFlashContainer.style.display = (activeFlow === 'factory') ? 'flex' : 'none';
        checkPreconditions();
      });
    });

    function getVaultStatus() {
      const hasCore = fileVault["bootloader.bin"] && fileVault["boot_app0.bin"];
      const has4 = fileVault["partitions_4mb.bin"] && fileVault["qc_test_4mb.bin"] && fileVault["production_4mb.bin"];
      const has8 = fileVault["partitions_8mb.bin"] && fileVault["qc_test_8mb.bin"] && fileVault["production_8mb.bin"];
      const has16 = fileVault["partitions_16mb.bin"] && fileVault["qc_test_16mb.bin"] && fileVault["production_16mb.bin"];
      return {
        hasCore,
        has4,
        has8,
        has16,
        ready: hasCore && (has4 || has8 || has16)
      };
    }

    function checkPreconditions() {
      updateStepTracker();
      updateGuidanceBanner();
      // 1. If we are currently in an active flash or integration run
      if (isFlashing || isIntegrating) {
        btnStart.disabled = true;
        btnStartDiag.disabled = true;
        btnConnToggle.disabled = true;
        btnStartIntegration.disabled = true;
        btnStartRepair.disabled = true;
        if (useDirectoryPicker) {
          btnSelectFolderAPI.disabled = true;
        } else {
          btnSelectFolderFallback.disabled = true;
        }
        return;
      }

      btnConnToggle.disabled = false;
      if (useDirectoryPicker) {
        btnSelectFolderAPI.disabled = false;
      } else {
        btnSelectFolderFallback.disabled = false;
      }

      const vault = getVaultStatus();
      const testerNameVal = (document.getElementById("testerName")?.value || "").trim();
      
      // Update Step 1 (Vault) Card and Badge
      const vaultStatusMessage = document.getElementById("vaultStatusMessage");
      if (vault.ready) {
        cardVault.classList.remove("pulse-blue-card");
        cardVault.classList.add("vault-complete-card");
        vaultStepBadge.textContent = "LOADED";
        vaultStepBadge.className = "step-indicator badge badge-vault-loaded";
        
        vaultStatusMessage.textContent = "✓ Firmware binaries successfully loaded.";
        vaultStatusMessage.style.color = "var(--success)";
        
        if (useDirectoryPicker) {
          btnSelectFolderAPI.textContent = "🔄 Change Firmware Folder";
          btnSelectFolderAPI.className = "btn btn-secondary";
          btnSelectFolderAPI.classList.remove("pulse-blue-button");
          btnSelectFolderAPI.style.display = "inline-flex";
        } else {
          btnSelectFolderFallback.textContent = "🔄 Change Firmware Folder";
          btnSelectFolderFallback.className = "btn btn-secondary";
          btnSelectFolderFallback.classList.remove("pulse-blue-button");
          btnSelectFolderFallback.style.display = "inline-flex";
        }
      } else {
        cardVault.classList.add("pulse-blue-card");
        cardVault.classList.remove("vault-complete-card");
        vaultStepBadge.textContent = "PENDING";
        vaultStepBadge.className = "step-indicator badge badge-disconnected";
        
        if (window.location.protocol === "file:") {
          vaultStatusMessage.textContent = "📁 Please click the button below and select the WEB_FLASH_FILES folder on this computer.";
        } else if (vaultAutoLoadAttempted) {
          vaultStatusMessage.textContent = "⚠️ Software files could not be loaded automatically. Click \"Select Firmware Folder\" below.";
        } else {
          vaultStatusMessage.textContent = "Loading test software… please wait a moment.";
        }
        vaultStatusMessage.style.color = "var(--warning)";
        
        if (useDirectoryPicker) {
          btnSelectFolderAPI.textContent = "📁 Select Firmware Folder";
          btnSelectFolderAPI.className = "btn btn-primary pulse-blue-button";
          btnSelectFolderAPI.style.display = "inline-flex";
        } else {
          btnSelectFolderFallback.textContent = "📁 Select Firmware Folder";
          btnSelectFolderFallback.className = "btn btn-primary pulse-blue-button";
          btnSelectFolderFallback.style.display = "inline-flex";
        }
      }

      // Update Step 2 (Connection) Card and Badge
      if (isConnected) {
        cardConnection.classList.remove("pulse-blue-card");
        cardConnection.classList.add("conn-complete-card");
        connBadge.textContent = "CONNECTED";
        connBadge.className = "badge badge-conn-connected";
        btnConnToggle.textContent = "🔌 Disconnect";
        btnConnToggle.className = "btn btn-secondary";
        btnConnToggle.classList.remove("pulse-blue-button");
      } else {
        cardConnection.classList.remove("conn-complete-card");
        connBadge.textContent = "DISCONNECTED";
        connBadge.className = "badge badge-disconnected";
        btnConnToggle.textContent = "🔌 Connect Board";
        btnConnToggle.className = "btn btn-primary";
        
        if (vault.ready) {
          btnConnToggle.classList.add("pulse-blue-button");
        } else {
          btnConnToggle.classList.remove("pulse-blue-button");
        }
      }

      if (currentTab === "boardqc") {
        // --- Tab 1: Board QC Preconditions ---
        const espEnabled = testCfgEsp.checked;
        const gprsEnabled = testCfgGprs.checked;
        const nuvotonEnabled = testCfgNuvoton.checked;

        // 1. Validate Barcodes for Selected Subsystems
        let barcodesValid = true;
        let missingBarcodeMsg = "";

        if (espEnabled && !barcodeEsp.value.trim()) {
          barcodesValid = false;
          missingBarcodeMsg = "⚠️ Scan ESP32 Serial";
        } else if (gprsEnabled && !barcodeGprs.value.trim()) {
          barcodesValid = false;
          missingBarcodeMsg = "⚠️ Scan GPRS Serial";
        } else if (nuvotonEnabled && !barcodeNuvoton.value.trim()) {
          barcodesValid = false;
          missingBarcodeMsg = "⚠️ Scan Nuvoton Serial";
        } else if (!espEnabled && !gprsEnabled && !nuvotonEnabled) {
          barcodesValid = false;
          missingBarcodeMsg = "⚠️ Select Subsystem";
        }

        // Check for duplicate scanned board serials in Board QC tab
        if (barcodesValid) {
          const scannedValues = [];
          if (espEnabled && barcodeEsp.value.trim()) scannedValues.push(barcodeEsp.value.trim());
          if (gprsEnabled && barcodeGprs.value.trim()) scannedValues.push(barcodeGprs.value.trim());
          if (nuvotonEnabled && barcodeNuvoton.value.trim()) scannedValues.push(barcodeNuvoton.value.trim());
          const uniqueValues = new Set(scannedValues);
          if (scannedValues.length !== uniqueValues.size) {
            barcodesValid = false;
            missingBarcodeMsg = "⚠️ Duplicate board serials";
          }
        }

        // 2. Validate Firmware Files if ESP32 Flashing is Active
        let canStart = false;
        if (isConnected && cancelReadLoop && barcodesValid) {
          if (espEnabled) {
            // ESP32 is checked, we must have files in vault
            if (activeFlow === 'factory') {
              const partKey = `partitions_${currentChipSize}mb.bin`;
              const testKey = `qc_test_${currentChipSize}mb.bin`;
              if (fileVault["bootloader.bin"] && fileVault["boot_app0.bin"] && fileVault[partKey] && fileVault[testKey]) {
                canStart = true;
              }
            } else {
              const appKey = `production_${currentChipSize}mb.bin`;
              if (fileVault[appKey]) {
                canStart = true;
              }
            }
          } else {
            // ESP32 is unchecked. No flashing needed; just run diagnostics.
            canStart = true;
          }
        }
        
        if (canStart) {
          btnStart.disabled = false;
          btnStart.classList.add("pulse-green-button");
          cardProgramming.classList.add("step-ready-card");
        } else {
          btnStart.disabled = true;
          btnStart.classList.remove("pulse-green-button");
          cardProgramming.classList.remove("step-ready-card");
        }

        // Diagnostics Button (Right Panel widget)
        if (!isConnected) {
          btnStartDiag.disabled = true;
          btnStartDiag.className = "btn btn-primary";
          btnStartDiag.textContent = "▶ Start Diagnostics";
        } else if (!cancelReadLoop) {
          btnStartDiag.disabled = false;
          btnStartDiag.className = "btn btn-danger";
          btnStartDiag.textContent = "⏹ Stop Diagnostics";
        } else {
          btnStartDiag.disabled = !barcodesValid;
          btnStartDiag.className = "btn btn-primary";
          btnStartDiag.textContent = missingBarcodeMsg ? missingBarcodeMsg : "▶ Start Diagnostics";
        }

        // Action Text and States Resolution based on Tester Name, Barcodes, and Connection
        if (!testerNameVal) {
          btnStart.disabled = true;
          btnStart.textContent = "🚀 Enter Tester Name";
          btnStart.classList.remove("pulse-green-button");
          if (cancelReadLoop) {
            btnStartDiag.disabled = true;
            btnStartDiag.textContent = "▶ Enter Tester Name";
          }
        } else if (!barcodesValid) {
          btnStart.disabled = true;
          btnStart.textContent = missingBarcodeMsg;
          btnStart.classList.remove("pulse-green-button");
          if (cancelReadLoop) {
            btnStartDiag.disabled = true;
            btnStartDiag.textContent = missingBarcodeMsg;
          }
        } else if (!isConnected) {
          btnStart.disabled = true;
          btnStart.textContent = "🔌 Connect Board First";
          btnStart.classList.remove("pulse-green-button");
        } else if (espEnabled && !vault.ready) {
          btnStart.disabled = true;
          btnStart.textContent = "⚠️ Load Firmware in Step 1";
          btnStart.classList.remove("pulse-green-button");
        } else {
          // All conditions passed
          if (espEnabled) {
            if (activeFlow === 'factory') {
              btnStart.textContent = "🚀 Start Factory Program & Test";
            } else {
              btnStart.textContent = "🚀 Flash Production Firmware";
            }
          } else {
            btnStart.textContent = "▶ Start Diagnostics Only";
          }
          
          if (cancelReadLoop) {
            btnStartDiag.textContent = "▶ Start Diagnostics";
          }
        }
        
        btnStartIntegration.disabled = true;
        btnStartRepair.disabled = true;
      } else if (currentTab === "integration") {
        // --- Tab 2: Integration Preconditions ---
        btnStart.disabled = true;
        btnStartDiag.disabled = true;
        btnStartRepair.disabled = true;

        const b1 = box1Barcode.value.trim();
        const b2 = box2Barcode.value.trim();
        const eBar = assocEspBarcode.value.trim();
        const gBar = assocGprsBarcode.value.trim();
        const nBar = assocNuvBarcode.value.trim();
        const mpptBar = assocMpptBarcode.value.trim();

        // Must be connected, have tester name, have box barcodes, and required components filled
        const hasInputs = b1 && b2 && eBar && gBar && nBar && mpptBar;

        // Check for duplicate scanned boxes or boards in Integration
        const isDuplicateBox = (b1 && b2 && b1 === b2);
        let isDuplicateBoard = false;
        const boardBarcodes = [eBar, gBar, nBar, mpptBar].filter(Boolean);
        const uniqueBoards = new Set(boardBarcodes);
        if (boardBarcodes.length !== uniqueBoards.size) {
          isDuplicateBoard = true;
        }
        
        // Check if selected config firmware is available in vault (either customConfigVault or production app)
        const activeConfig = releaseConfigSel?.value;
        const configData = activeConfig ? customConfigVault[activeConfig] : null;
        const hasProductionBin = (configData && configData.binary) || fileVault[`production_${currentChipSize}mb.bin`];

        const canIntegrate = isConnected && testerNameVal && hasInputs && hasProductionBin && !hasInventoryBlocker && !isDuplicateBox && !isDuplicateBoard;

        if (canIntegrate) {
          btnStartIntegration.disabled = false;
          btnStartIntegration.classList.add("pulse-green-button");
          cardIntegrationControls.classList.add("step-ready-card");
        } else {
          btnStartIntegration.disabled = true;
          btnStartIntegration.classList.remove("pulse-green-button");
          cardIntegrationControls.classList.remove("step-ready-card");
        }

        if (!testerNameVal) {
          btnStartIntegration.textContent = "⚡ Enter Tester Name";
        } else if (!isConnected) {
          btnStartIntegration.textContent = "🔌 Connect Board First";
        } else if (isDuplicateBox) {
          btnStartIntegration.textContent = "⚠️ Box 1 & 2 must be different";
        } else if (isDuplicateBoard) {
          btnStartIntegration.textContent = "⚠️ Duplicate board serials";
        } else if (hasInventoryBlocker) {
          btnStartIntegration.textContent = "⚠️ Failed Board Detected";
        } else if (!hasProductionBin) {
          btnStartIntegration.textContent = "⚠️ Load Production Firmware";
        } else if (!hasInputs) {
          btnStartIntegration.textContent = "⚠️ Fill Box & Board Barcodes";
        } else {
          btnStartIntegration.textContent = "⚡ Flash Production & Run Verify";
        }
      } else {
        // --- Tab 3: Repair Preconditions ---
        btnStart.disabled = true;
        btnStartDiag.disabled = true;
        btnStartIntegration.disabled = true;

        const b1 = repairBox1Barcode.value.trim();
        const b2 = repairBox2Barcode.value.trim();
        const eBar = repairEspBarcode.value.trim();
        const gBar = repairGprsBarcode.value.trim();
        const nBar = repairNuvBarcode.value.trim();
        const mpptBar = repairMpptBarcode.value.trim();
        const reasonVal = repairReason.value.trim();

        // Must be connected, have tester name, have box barcodes, mppt, and reason filled
        const hasInputs = b1 && b2 && eBar && gBar && nBar && mpptBar && reasonVal.length >= 3;

        // Check for duplicate scanned boxes or boards in Repair
        const isDuplicateBox = (b1 && b2 && b1 === b2);
        let isDuplicateBoard = false;
        const boardBarcodes = [eBar, gBar, nBar, mpptBar].filter(Boolean);
        const uniqueBoards = new Set(boardBarcodes);
        if (boardBarcodes.length !== uniqueBoards.size) {
          isDuplicateBoard = true;
        }

        // Check if selected config firmware is available in vault
        const activeConfig = releaseConfigSel?.value;
        const configData = activeConfig ? customConfigVault[activeConfig] : null;
        const hasProductionBin = (configData && configData.binary) || fileVault[`production_${currentChipSize}mb.bin`];

        const canRepair = isConnected && testerNameVal && hasInputs && hasProductionBin && !hasInventoryBlocker && !isDuplicateBox && !isDuplicateBoard;

        if (canRepair) {
          btnStartRepair.disabled = false;
          btnStartRepair.classList.add("pulse-green-button");
          cardRepairControls.classList.add("step-ready-card");
        } else {
          btnStartRepair.disabled = true;
          btnStartRepair.classList.remove("pulse-green-button");
          cardRepairControls.classList.remove("step-ready-card");
        }

        if (!testerNameVal) {
          btnStartRepair.textContent = "⚡ Enter Tester Name";
        } else if (!isConnected) {
          btnStartRepair.textContent = "🔌 Connect Board First";
        } else if (isDuplicateBox) {
          btnStartRepair.textContent = "⚠️ Box 1 & 2 must be different";
        } else if (isDuplicateBoard) {
          btnStartRepair.textContent = "⚠️ Duplicate board serials";
        } else if (hasInventoryBlocker) {
          btnStartRepair.textContent = "⚠️ Failed Board Detected";
        } else if (!hasProductionBin) {
          btnStartRepair.textContent = "⚠️ Load Production Firmware";
        } else if (!hasInputs) {
          if (!reasonVal) {
            btnStartRepair.textContent = "✍️ Enter Servicing Reason";
          } else {
            btnStartRepair.textContent = "⚠️ Fill Box & Component Barcodes";
          }
        } else {
          btnStartRepair.textContent = "⚡ Flash Production & Run Verify";
        }
      }
    }

    function updateSubsystemUI() {
      if (isFlashing || (isConnected && !cancelReadLoop)) {
        return;
      }
      const espEnabled = testCfgEsp.checked;
      const gprsEnabled = testCfgGprs.checked;
      const nuvotonEnabled = testCfgNuvoton.checked;
      
      const profile = document.getElementById("deviceProfile")?.value || "TRG";
      const sensorEnabled = espEnabled && (profile !== "TRG");
      const windEnabled = espEnabled && (profile !== "TRG");
      const rfEnabled = espEnabled && (profile !== "TWS");

      function toggleItemIgnore(chkElem, valElem, enabled) {
        if (!chkElem) return;
        if (!enabled) {
          setCheckState(chkElem, valElem, "ignored", "Disabled");
        } else {
          const parent = chkElem.closest('.check-item');
          if (parent && parent.classList.contains('check-item-ignored')) {
            setCheckState(chkElem, valElem, "default", "Waiting...");
          }
        }
      }

      // Map checklist items to their subsystems
      toggleItemIgnore(chkSpiffs, valSpiffs, espEnabled);
      toggleItemIgnore(chkSd, valSd, espEnabled);
      toggleItemIgnore(chkRtc, valRtc, espEnabled);
      toggleItemIgnore(chkWifi, valWifi, espEnabled);
      toggleItemIgnore(chkSys3v3, valSys3v3, espEnabled);
      toggleItemIgnore(chkSolar, valSolar, espEnabled);
      toggleItemIgnore(chkSleep, valSleep, espEnabled);

      toggleItemIgnore(chkSensor, valSensor, sensorEnabled);
      toggleItemIgnore(chkWindSpd, valWindSpd, windEnabled);
      toggleItemIgnore(chkWindDir, valWindDir, windEnabled);
      toggleItemIgnore(chkRf, valRf, rfEnabled);

      toggleItemIgnore(chkGprs, valGprs, gprsEnabled);
      toggleItemIgnore(chkBatt, valBatt, gprsEnabled);

      toggleItemIgnore(chkLcd, valLcd, nuvotonEnabled);
      toggleItemIgnore(chkKeypad, valKeypad, nuvotonEnabled);

      // Toggle barcode group visibility based on subsystems to test
      const groupEsp = document.getElementById("barcodeGroupEsp");
      const groupGprs = document.getElementById("barcodeGroupGprs");
      const groupNuv = document.getElementById("barcodeGroupNuvoton");
      if (groupEsp) groupEsp.style.display = espEnabled ? "block" : "none";
      if (groupGprs) groupGprs.style.display = gprsEnabled ? "block" : "none";
      if (groupNuv) groupNuv.style.display = nuvotonEnabled ? "block" : "none";

      // Toggle Step 3 Programming options based on ESP32 checked status
      const flowSelector = document.querySelector(".flow-selector");
      const forceFlashContainer = document.getElementById("forceFlashContainer");
      const cardProgrammingTitle = document.querySelector("#cardProgramming .card-title span");

      if (flowSelector) {
        flowSelector.style.display = espEnabled ? "block" : "none";
      }
      if (forceFlashContainer) {
        forceFlashContainer.style.display = (espEnabled && activeFlow === 'factory') ? "flex" : "none";
      }
      if (cardProgrammingTitle) {
        cardProgrammingTitle.textContent = espEnabled ? "Step 3: Run Programming" : "Step 3: Run Diagnostics Only";
      }

      // Automatically recalculate button states and warnings on toggling checkboxes
      checkPreconditions();
    }

    testCfgEsp.addEventListener("change", (e) => {
      logToConsole("[UI_EVENT] ESP32 subsystem selection changed to: " + (e.target.checked ? "ENABLED" : "DISABLED"), "info");
      updateSubsystemUI();
    });
    testCfgGprs.addEventListener("change", (e) => {
      logToConsole("[UI_EVENT] GPRS subsystem selection changed to: " + (e.target.checked ? "ENABLED" : "DISABLED"), "info");
      updateSubsystemUI();
    });
    testCfgNuvoton.addEventListener("change", (e) => {
      logToConsole("[UI_EVENT] Nuvoton subsystem selection changed to: " + (e.target.checked ? "ENABLED" : "DISABLED"), "info");
      updateSubsystemUI();
    });
    
    // Add input event listeners to Board QC barcode fields for instant validation response
    barcodeEsp?.addEventListener("input", checkPreconditions);
    barcodeGprs?.addEventListener("input", checkPreconditions);
    barcodeNuvoton?.addEventListener("input", checkPreconditions);
    document.getElementById("deviceProfile")?.addEventListener("change", (e) => {
      logToConsole("[UI_EVENT] Device Profile changed to: " + e.target.value, "info");
      populateConfigDropdown();
      updateSubsystemUI();
    });
    releaseConfigSel?.addEventListener("change", (e) => {
      logToConsole("[UI_EVENT] Release Config changed to: " + e.target.value, "info");
      renderConfigMetadata();
      checkPreconditions();
    });

    // Call once initially to set up correct start state
    populateConfigDropdown();
    updateSubsystemUI();

    // Register Auto-Advance and Auto-Select focus chains on inputs
    setupFocusChain([
      barcodeEsp,
      barcodeGprs,
      barcodeNuvoton
    ]);

    setupFocusChain([
      box1Barcode,
      box2Barcode,
      assocEspBarcode,
      assocGprsBarcode,
      assocNuvBarcode,
      assocBatt1Barcode,
      assocBatt2Barcode,
      assocMpptBarcode
    ]);

    setupFocusChain([
      repairBox1Barcode,
      repairBox2Barcode,
      repairEspBarcode,
      repairGprsBarcode,
      repairNuvBarcode,
      repairBatt1Barcode,
      repairBatt2Barcode,
      repairMpptBarcode,
      repairReason
    ]);

    // --- Auto-loader from Server ---
    async function tryAutoLoadBinaries() {
      if (window.location.protocol === "file:") {
        // file:// protocol — browser security blocks fetch(). Show manual folder picker immediately.
        logToConsole("📁 Opened via file:// — auto-load is not possible here.", "warning");
        logToConsole("  ► Click \"Select Firmware Folder\" in Step 1 to pick the WEB_FLASH_FILES folder.", "info");
        logToConsole("  ► Or use launch_mac.command / launch_windows.bat for fully automatic loading.", "info");

        // Update the vault card message to be clear and friendly
        const vsm = document.getElementById("vaultStatusMessage");
        if (vsm) {
          vsm.textContent = "📁 Please click the button below to select the WEB_FLASH_FILES folder on your computer.";
          vsm.style.color = "var(--warning)";
        }

        vaultAutoLoadAttempted = true;
        checkPreconditions();
        return;
      }
      logToConsole(`Attempting to auto-load firmware from dashboard directory...`, "normal");
      
      // Helper to fetch files trying both current path and WEB_FLASH_FILES/ fallback
      async function fetchFile(filename) {
        let response = await fetch("./" + filename);
        if (response.ok) return { response, path: "./" };
        
        response = await fetch("./WEB_FLASH_FILES/" + filename);
        if (response.ok) return { response, path: "./WEB_FLASH_FILES/" };
        
        throw new Error("Not found");
      }

      const filesToLoad = Object.keys(fileVault);
      let loadedCount = 0;
      let detectedPath = "./";
      
      const loadPromises = filesToLoad.map(async (filename) => {
        try {
          const { response, path } = await fetchFile(filename);
          detectedPath = path;
          const blob = await response.blob();
          const buffer = await blob.arrayBuffer();
          fileVault[filename] = new Uint8Array(buffer);
          loadedCount++;
        } catch (err) {
          // Expected for optional sizes / offline access
        }
      });

      // Attempt to load version.txt
      const loadVerPromise = (async () => {
        try {
          const { response } = await fetchFile("version.txt");
          const verText = (await response.text()).trim();
          metaFwVersion.textContent = verText;
          logToConsole(`✓ Detected firmware version: ${verText}`, "info");
        } catch (err) {}
      })();

      // Attempt to auto-load all config releases from server
      const configKeys = Object.keys(customConfigVault);
      let loadedConfigsCount = 0;
      const configPromises = configKeys.map(async (cfg) => {
        const folderName = `${cfg}_8mb`;
        try {
          // Resolve correct prefix path
          let prefix = `./${folderName}`;
          let binRes = await fetch(`${prefix}/firmware.bin`);
          if (!binRes.ok) {
            prefix = `./WEB_FLASH_FILES/${folderName}`;
            binRes = await fetch(`${prefix}/firmware.bin`);
          }

          if (binRes.ok) {
            const [verRes, metaRes] = await Promise.all([
              fetch(`${prefix}/fw_version.txt`),
              fetch(`${prefix}/metadata.json`)
            ]);

            if (verRes.ok && metaRes.ok) {
              const [binBuf, verTxt, metaJson] = await Promise.all([
                binRes.arrayBuffer(),
                verRes.text(),
                metaRes.json()
              ]);

              customConfigVault[cfg].binary = new Uint8Array(binBuf);
              customConfigVault[cfg].version = verTxt.trim();
              customConfigVault[cfg].metadata = metaJson;
              loadedConfigsCount++;
              logToConsole(`✓ Auto-loaded config "${cfg}" from "${prefix}/"`, "info");
            }
          }
        } catch (err) {
          // Quietly ignore if this config isn't on the server
        }
      });

      await Promise.all([...loadPromises, loadVerPromise, ...configPromises]);
      
      if (loadedCount > 0) {
        logToConsole(`✓ Successfully auto-loaded ${loadedCount} core files from "${detectedPath}".`, "info");
      }
      if (loadedConfigsCount > 0) {
        logToConsole(`✓ Successfully auto-loaded ${loadedConfigsCount} config releases from server.`, "info");
      }
      
      if (loadedCount === 0 && loadedConfigsCount === 0) {
        logToConsole("No firmware binaries found in dashboard directory. Please select local folder.", "warning");
      }
      
      updateFileVaultDisplay();
      renderConfigMetadata();
      checkPreconditions();
    }

    // --- Directory Scanner (File System Access API) ---
    async function selectLocalDirectory() {
      try {
        logToConsole("Requesting local folder access...", "normal");
        const dirHandle = await window.showDirectoryPicker();
        await readFilesFromDirectory(dirHandle);
      } catch (err) {
        if (err.name !== 'AbortError') {
          logToConsole(`Directory access failed: ${err.message}`, "error");
        }
      }
    }

    async function readFilesFromDirectory(dirHandle) {
      let loadedCount = 0;
      let targetDirHandle = dirHandle;

      // Iteratively drill down if the directory does not contain core files, but contains TEST_JIG or WEB_FLASH_FILES
      for (let depth = 0; depth < 3; depth++) {
        let hasCoreFiles = false;
        for await (const entry of targetDirHandle.values()) {
          if (entry.kind === 'file' && (entry.name === 'bootloader.bin' || entry.name === 'boot_app0.bin')) {
            hasCoreFiles = true;
            break;
          }
        }
        if (hasCoreFiles) {
          break;
        }
        // If no core files, check for child directories to drill down
        let foundSubdir = null;
        for await (const entry of targetDirHandle.values()) {
          if (entry.kind === 'directory') {
            const lowerName = entry.name.toLowerCase();
            if (lowerName === 'web_flash_files' || lowerName === 'test_jig') {
              foundSubdir = entry.name;
              break;
            }
          }
        }
        if (foundSubdir) {
          try {
            targetDirHandle = await targetDirHandle.getDirectoryHandle(foundSubdir);
            logToConsole(`📂 Automatically navigating into subdirectory: ${foundSubdir}`, "info");
          } catch (err) {
            break;
          }
        } else {
          break;
        }
      }

      // --- Pass 1: Load top-level core vault files ---
      for await (const entry of targetDirHandle.values()) {
        if (entry.kind === 'file' && fileVault.hasOwnProperty(entry.name)) {
          const file = await entry.getFile();
          const buffer = await file.arrayBuffer();
          fileVault[entry.name] = new Uint8Array(buffer);
          loadedCount++;
        }
      }
      
      // Attempt to load version.txt from selected folder
      try {
        const verFileHandle = await targetDirHandle.getFileHandle("version.txt");
        const verFile = await verFileHandle.getFile();
        const verText = (await verFile.text()).trim();
        metaFwVersion.textContent = verText;
        logToConsole(`✓ Detected firmware version from folder: ${verText}`, "info");
      } catch (e) {
        // version.txt not found in folder
      }

      // --- Pass 2: Scan subdirectories for named release configs ---
      let configCount = 0;
      for await (const entry of targetDirHandle.values()) {
        if (entry.kind !== 'directory') continue;
        // Normalise: strip flash suffix like _8mb
        const configBase = entry.name.replace(/_?(4|8|16)mb$/i, '');
        if (!customConfigVault.hasOwnProperty(configBase)) continue;

        // Scan files inside this config subdirectory
        try {
          const subDir = await targetDirHandle.getDirectoryHandle(entry.name);
          for await (const subEntry of subDir.values()) {
            if (subEntry.kind !== 'file') continue;
            const subFile = await subEntry.getFile();
            if (subEntry.name === 'firmware.bin') {
              const buf = await subFile.arrayBuffer();
              customConfigVault[configBase].binary = new Uint8Array(buf);
              logToConsole(`✓ Config loaded: ${configBase}/firmware.bin (${(subFile.size/1024).toFixed(1)} KB)`, "info");
              configCount++;
            }
            if (subEntry.name === 'fw_version.txt') {
              customConfigVault[configBase].version = (await subFile.text()).trim();
            }
            if (subEntry.name === 'metadata.json') {
              try { customConfigVault[configBase].metadata = JSON.parse(await subFile.text()); } catch(_) {}
            }
          }
        } catch (err) {
          logToConsole(`⚠ Could not read config folder ${entry.name}: ${err.message}`, "warning");
        }
      }
      
      if (loadedCount > 0) {
        logToConsole(`✓ Loaded ${loadedCount} firmware files from folder.`, "info");
      } else {
        logToConsole("No matching firmware files found in the folder.", "warning");
      }
      if (configCount > 0) {
        logToConsole(`✓ Loaded ${configCount} named config firmware binaries.`, "info");
      }
      updateFileVaultDisplay();
      checkPreconditions();
      renderConfigMetadata();
    }

    // --- WebSerial Connection Toggle ---
    btnConnToggle.addEventListener("click", async () => {
      if (isConnected) {
        await disconnectPort();
      } else {
        await connectPort();
      }
    });

    // Handle physical USB unplug events automatically
    if (navigator.serial) {
      navigator.serial.addEventListener("disconnect", (event) => {
        if (port && event.target === port) {
          logToConsole("🔌 Serial device unplugged physically. Resetting connection.", "warning");
          disconnectPort();
        }
      });
    }

    async function connectPort() {
      try {
        logToConsole("Requesting port access...", "normal");
        port = await navigator.serial.requestPort();
        transport = new Transport(port, true);
        
        const terminal = {
          writeLine: (data) => logToConsole(data),
          write: (data) => logToConsole(data),
          clean: () => consoleDiv.innerHTML = ""
        };

        esploader = new ESPLoader({
          transport: transport,
          baudrate: 115200,
          terminal: terminal
        });
        logToConsole("Connecting to chip...", "normal");
        
        const chip = await esploader.main();
        logToConsole(`Chip type detected: ${chip}`, "info");
        
        // Read MAC Address
        const macStr = (await esploader.chip.readMac(esploader)).toUpperCase();
        metaMac.textContent = macStr;
        logToConsole(`Chip MAC: ${macStr}`, "info");
        
        // Auto-detect chip flash size
        const flashId = await esploader.readFlashId();
        const flashSizeHex = (flashId >> 16) & 0xFF;
        
        if (flashSizeHex === 0x16) {
          currentChipSize = 4;
        } else if (flashSizeHex === 0x18) {
          currentChipSize = 16;
        } else {
          currentChipSize = 8;
        }
        metaFlashSize.textContent = `${currentChipSize} MB`;
        logToConsole(`Auto-detected flash capacity: ${currentChipSize}MB (FlashID: 0x${flashId.toString(16)})`, "info");
        
        isConnected = true;
        btnResetBoard.style.display = "block";
        const statusDiv = document.getElementById("headerShortStatus");
        if (statusDiv) {
          statusDiv.innerHTML = `<span>MAC: <strong style="color: var(--success);">${macStr}</strong></span> | <span>Size: <strong style="color: var(--primary);">${currentChipSize}MB</strong></span>`;
        }
        
        checkPreconditions();
        updateStepTracker();
        focusFirstBarcodeInput();
      } catch (err) {
        logToConsole(`Connection error: ${err.message}`, "error");
      }
    }

    btnResetBoard.addEventListener("click", async () => {
      if (!port) {
        logToConsole("Board is not connected. Cannot reset.", "error");
        return;
      }
      logToConsole("Performing hardware reset...", "warning");
      try {
        if (transport && esploader) {
          await esploader.after("hard_reset");
        } else if (port) {
          // If the port is closed (readable is null), open it temporarily to send the reset signals
          let needClose = false;
          if (!port.readable) {
            try {
              await port.open({ baudRate: 115200 });
              needClose = true;
            } catch (openErr) {
              logToConsole(`Reset failed (could not open port): ${openErr.message}`, "error");
              return;
            }
          }
          // If we are in direct diagnostics mode, we toggle RTS/DTR manually
          await port.setSignals({ dataTerminalReady: false, requestToSend: true });
          await new Promise(r => setTimeout(r, 100));
          await port.setSignals({ dataTerminalReady: true, requestToSend: false });
          await new Promise(r => setTimeout(r, 100));
          await port.setSignals({ dataTerminalReady: false, requestToSend: false });
          
          if (needClose) {
            await port.close();
          }
        }
        logToConsole("Hardware reset signal sent.", "info");
      } catch (err) {
        logToConsole(`Reset failed: ${err.message}`, "error");
      }
    });

    async function stopDiagnostics() {
      cancelReadLoop = true;
      if (activeReader) {
        try {
          await activeReader.cancel();
        } catch (e) {}
      }
      
      // Wait for loop to yield/exit
      await new Promise(r => setTimeout(r, 150));

      if (port) {
        try { await port.close(); } catch (e) {}
      }
      
      logToConsole("Diagnostics stopped. Serial connection remains open.", "normal");
      checkPreconditions();
    }

    async function disconnectPort() {
      cancelReadLoop = true;
      if (activeReader) {
        try {
          await activeReader.cancel();
        } catch (e) {}
      }
      
      // Wait for loop to yield/exit
      await new Promise(r => setTimeout(r, 150));

      if (transport) {
        try { await transport.disconnect(); } catch (e) {}
      }

      if (port) {
        try { await port.close(); } catch (e) {}
      }

      port = null;
      transport = null;
      esploader = null;
      isConnected = false;
      metaMac.textContent = "--:--:--:--:--:--";
      metaFlashSize.textContent = "-- MB";
      if (barcodeEsp) barcodeEsp.value = "";
      if (barcodeGprs) barcodeGprs.value = "";
      if (barcodeNuvoton) barcodeNuvoton.value = "";
      btnResetBoard.style.display = "none";
      btnStart.disabled = true;
      btnProceedApp.style.display = "none";
      progressContainer.style.display = "none";
      logToConsole("Disconnected serial port.", "info");
      const statusDiv = document.getElementById("headerShortStatus");
      if (statusDiv) {
        statusDiv.innerHTML = "";
      }
      checkPreconditions();
    }

    function setCheckState(badgeEl, valueEl, state, valueText) {
      const item = badgeEl.closest(".check-item");
      if (item && item.classList.contains("check-item-ignored") && state !== "ignored" && state !== "default") {
        return;
      }
      if (state === "pass") {
        badgeEl.className = "check-status status-pass";
        badgeEl.textContent = "✓";
        valueEl.textContent = valueText || "PASS";
        if (item) {
          item.className = "check-item check-item-pass";
        }
      } else if (state === "fail") {
        badgeEl.className = "check-status status-fail";
        badgeEl.textContent = "✗";
        valueEl.textContent = valueText || "FAIL";
        if (item) {
          item.className = "check-item check-item-fail";
        }
      } else if (state === "wait") {
        badgeEl.className = "check-status status-wait";
        badgeEl.textContent = "⏳";
        valueEl.textContent = valueText || "Waiting...";
        if (item) {
          item.className = "check-item check-item-wait";
        }
      } else if (state === "warn") {
        badgeEl.className = "check-status status-warn";
        badgeEl.textContent = "⚠";
        valueEl.textContent = valueText || "Warning";
        if (item) {
          item.className = "check-item check-item-warn";
        }
      } else if (state === "ignored") {
        badgeEl.className = "check-status status-ignored";
        badgeEl.textContent = "➖";
        valueEl.textContent = valueText || "IGNORED";
        if (item) {
          item.className = "check-item check-item-ignored";
        }
      } else {
        badgeEl.className = "check-status";
        badgeEl.textContent = "⚪";
        valueEl.textContent = valueText || "Waiting...";
        if (item) {
          item.className = "check-item";
        }
      }
    }

    // Helper to send serial commands to the board
    async function sendSerialCommand(text) {
      if (!port || !port.writable) {
        logToConsole("Cannot send serial command: Port not writable.", "error");
        return;
      }
      try {
        const writer = port.writable.getWriter();
        const encoder = new TextEncoder();
        await writer.write(encoder.encode(text));
        writer.releaseLock();
      } catch (err) {
        logToConsole(`Error sending command to serial: ${err.message}`, "error");
      }
    }

    // --- Enclosure Integration Diagnostics & Helpers ---
    function setIntegrationCheckState(widget, badge, status, text) {
      if (!widget || !badge) return;
      badge.textContent = text;
      widget.classList.remove("widget-a", "widget-b", "widget-c", "pulse-border");
      if (status === "pass") {
        widget.classList.add("widget-b");
        badge.className = "badge badge-conn-connected";
      } else if (status === "fail") {
        widget.classList.add("widget-c");
        badge.className = "badge badge-disconnected";
      } else if (status === "running") {
        widget.classList.add("widget-a", "pulse-border");
        badge.className = "badge badge-disconnected";
      } else {
        widget.classList.add("widget-a");
        badge.className = "badge badge-disconnected";
      }
    }

    async function autoSyncIntegrationToSheets(verdict) {
      const url = sheetUrlInput.value.trim();
      if (!url) {
        logToConsole("Google Sheet integration sync skipped (No Script URL configured).", "warning");
        return false;
      }
      
      const isRepair = (currentTab === "repair");
      
      let swappedBoards = [];
      if (isRepair && loadedRepairConfig) {
        const checkSwap = (oldVal, newVal, boardType) => {
          if (oldVal && newVal && oldVal.trim() !== newVal.trim()) {
            swappedBoards.push({
              barcode: oldVal.trim(),
              type: boardType,
              failureReason: repairReason.value.trim() || "Swapped during servicing"
            });
          }
        };
        checkSwap(loadedRepairConfig.esp, repairEspBarcode.value, "ESP32");
        checkSwap(loadedRepairConfig.gprs, repairGprsBarcode.value, "GPRS");
        checkSwap(loadedRepairConfig.nuv, repairNuvBarcode.value, "Nuvoton");
        checkSwap(loadedRepairConfig.batt1, repairBatt1Barcode.value, "Battery 1");
        checkSwap(loadedRepairConfig.batt2, repairBatt2Barcode.value, "Battery 2");
        checkSwap(loadedRepairConfig.mppt, repairMpptBarcode.value, "MPPT");
      }

      const payload = {
        integrationSync: true,
        testTime: new Date().toLocaleString(),
        box1: (isRepair ? repairBox1Barcode.value : box1Barcode.value).trim(),
        box2: (isRepair ? repairBox2Barcode.value : box2Barcode.value).trim(),
        espBarcode: (isRepair ? repairEspBarcode.value : assocEspBarcode.value).trim(),
        gprsBarcode: (isRepair ? repairGprsBarcode.value : assocGprsBarcode.value).trim(),
        nuvotonBarcode: (isRepair ? repairNuvBarcode.value : assocNuvBarcode.value).trim(),
        batt1Barcode: (isRepair ? repairBatt1Barcode.value : assocBatt1Barcode.value).trim(),
        batt2Barcode: (isRepair ? repairBatt2Barcode.value : assocBatt2Barcode.value).trim(),
        mpptBarcode: (isRepair ? repairMpptBarcode.value : assocMpptBarcode.value).trim(),
        mac: integrationMac.textContent,
        version: integrationFwVersion.textContent,
        verdict: verdict,
        tester: document.getElementById("testerName")?.value.trim() || "--",
        duration: testDuration || 0,
        actionType: isRepair ? "REPAIR_SWAP" : "ORIGINAL_BUILD",
        swapReason: isRepair ? repairReason.value.trim() : "",
        swappedBoards: swappedBoards
      };

      try {
        await fetch(url, {
          method: "POST",
          mode: "no-cors",
          headers: {
            "Content-Type": "application/json"
          },
          body: JSON.stringify(payload)
        });
        logToConsole("✓ Successfully synced integration record to Google Sheet!", "info");
        return true;
      } catch (err) {
        logToConsole(`Failed to sync integration to sheets: ${err.message}`, "error");
        addToOfflineQueue(payload, "integration", url);
        return false;
      }
    }

    async function completeIntegrationSuccess() {
      cancelReadLoop = true;
      if (activeReader) {
        try { await activeReader.cancel(); } catch (e) {}
      }
      if (port) {
        try { await port.close(); } catch (e) {}
      }
      
      btnAbortIntegration.style.display = "none";
      isIntegrating = false;
      
      testDuration = Math.round((Date.now() - testStartTime) / 1000);
      setIntegrationCheckState(intChkSync, intValSync, "running", "Syncing...");
      
      logToConsole("📊 All integration diagnostics passed! Syncing data to Google Sheets...", "info");
      const synced = await autoSyncIntegrationToSheets("PASS");
      
      if (synced) {
        setIntegrationCheckState(intChkSync, intValSync, "pass", "SUCCESS");
        showVerdict("PASS", `Integration Verification Successful!<br>Box 1 and Box 2 fully commissioned.<br>Production telemetry verified.`);
      } else {
        setIntegrationCheckState(intChkSync, intValSync, "fail", "FAILED");
        showVerdict("FAIL", `Integration check passed but database sync failed.<br>Please check network connection and spreadsheet URL.`);
      }
      
      checkPreconditions();
    }

    async function startIntegrationDiagnostics() {
      cancelReadLoop = false;
      checkPreconditions();
      const serialPort = port;
      
      testStartTime = Date.now();
      testDuration = 0;
      
      setIntegrationCheckState(intChkBoot,  intValBoot,  "default", "Waiting...");
      setIntegrationCheckState(intChkGprs,  intValGprs,  "default", "Waiting...");
      setIntegrationCheckState(intChkBatt,  intValBatt,  "default", "Waiting...");
      setIntegrationCheckState(intChkSolar, intValSolar, "default", "Waiting...");
      setIntegrationCheckState(intChkSync,  intValSync,  "default", "Waiting...");

      integrationFwVersion.textContent = "--";
      integrationMac.textContent = (currentTab === "repair" ? repairEspBarcode.value : assocEspBarcode.value) || "--";
      
      logToConsole("Starting integration boot diagnostics listener...", "info");
      btnAbortIntegration.style.display = "block";
      
      let bootDetected = false;
      let gprsConnected = false;
      let batteryChecked = false;
      let solarChecked = false;
      window._integrationSuccessTriggered = false;
      
      while (serialPort.readable && !cancelReadLoop) {
        activeReader = serialPort.readable.getReader();
        try {
          let buffer = "";
          while (true) {
            const { value, done } = await activeReader.read();
            if (done || cancelReadLoop) break;
            
            const text = new TextDecoder().decode(value);
            buffer += text;
            
            let lines = buffer.split("\n");
            buffer = lines.pop();
            
            for (const line of lines) {
              const cleaned = line.trim();
              if (cleaned) {
                logToConsole(cleaned);
                
                // Telemetry line parser
                // e.g. [SYS_BOOT] MAC: 48:E7:... | VER: 6.07 | SD: OK | GPRS: ONLINE | BATT: 12.6V | SOLAR: 17.5V
                // Or standard boot log outputs
                
                const cleanedUpper = cleaned.toUpperCase();
                
                if (cleanedUpper.includes("SYS_BOOT") || 
                    cleanedUpper.includes("BOOTING") || 
                    cleanedUpper.includes("SETUP STARTED") || 
                    cleanedUpper.includes("SYSTEM STARTING") || 
                    cleanedUpper.includes("[BOOT]") || 
                    cleanedUpper.includes("QC TEST FIRMWARE START")) {
                  if (!bootDetected) {
                    bootDetected = true;
                    setIntegrationCheckState(intChkBoot, intValBoot, "pass", "DETECTED");
                    const verMatch = cleaned.match(/VER:\s*([^\s|]+)/i) || 
                                     cleaned.match(/Version:\s*([^\s|]+)/i) ||
                                     cleaned.match(/Current binary:\s*([^\s|]+)/i);
                    if (verMatch) integrationFwVersion.textContent = verMatch[1];
                    const macMatch = cleaned.match(/MAC:\s*([^\s|]+)/i) ||
                                     cleaned.match(/Unique MAC:\s*([^\s|]+)/i);
                    if (macMatch) integrationMac.textContent = macMatch[1];
                  }
                }
                
                if (cleanedUpper.includes("GPRS: ONLINE") || 
                    cleanedUpper.includes("CELLULAR: ONLINE") || 
                    cleanedUpper.includes("MODEM READY") || 
                    cleanedUpper.includes("ONLINE (CSQ") ||
                    cleanedUpper.includes("REGISTERED VIA") ||
                    cleanedUpper.includes("BYPASSING SETUP BLOCK") ||
                    cleanedUpper.includes("MODEM_INIT: REG_OK")) {
                  if (!gprsConnected) {
                    gprsConnected = true;
                    setIntegrationCheckState(intChkGprs, intValGprs, "pass", "ONLINE");
                  }
                }
                
                if (cleanedUpper.includes("BATT:") || 
                    cleanedUpper.includes("BATTERY:") || 
                    cleanedUpper.includes("BATTERY :") || 
                    cleanedUpper.includes("GPRS BATTERY")) {
                  const voltMatch = cleaned.match(/BATT\s*:\s*([0-9.]+)/i) || 
                                   cleaned.match(/Battery\s*:\s*([0-9.]+)/i) ||
                                   cleaned.match(/GPRS Battery\s*:\s*([0-9.]+)/i);
                  if (voltMatch) {
                    const voltage = parseFloat(voltMatch[1]);
                    if (voltage >= 3.0) {
                      batteryChecked = true;
                      setIntegrationCheckState(intChkBatt, intValBatt, "pass", `${voltage.toFixed(2)} V`);
                    } else {
                      setIntegrationCheckState(intChkBatt, intValBatt, "fail", `${voltage.toFixed(2)} V (LOW)`);
                    }
                  }
                }

                if (cleanedUpper.includes("SOLAR:") || 
                    cleanedUpper.includes("SOLAR :") || 
                    cleanedUpper.includes("SOLAR VOLTAGE")) {
                  const voltMatch = cleaned.match(/SOLAR\s*:\s*([0-9.]+)/i) || 
                                   cleaned.match(/Solar\s*:\s*([0-9.]+)/i);
                  if (voltMatch) {
                    const voltage = parseFloat(voltMatch[1]);
                    solarChecked = true;
                    setIntegrationCheckState(intChkSolar, intValSolar, "pass", `${voltage.toFixed(2)} V`);
                  }
                }

                // Finish integration if boot, gprs, and battery are verified
                if (bootDetected && gprsConnected && batteryChecked) {
                  if (!window._integrationSuccessTriggered) {
                    window._integrationSuccessTriggered = true;
                    setTimeout(async () => {
                      await completeIntegrationSuccess();
                    }, 1500);
                  }
                }
              }
            }
          }
        } catch (err) {
          logToConsole(`Read error: ${err.message}`, "error");
        } finally {
          activeReader.releaseLock();
        }
      }
    }

    // --- Live serial parser (Diagnostics monitor) ---
    async function startLiveDiagnostics() {
      cancelReadLoop = false;
      checkPreconditions();
      const serialPort = port; // Obtain web serial port directly
      
      testStartTime = Date.now();
      testDuration = 0;
      window._qcHasFailedInSession = false; // Reset session failure tracker
      
      // Reset checklist displays
      setCheckState(chkSpiffs,  valSpiffs,  "default", "Waiting...");
      setCheckState(chkSd,      valSd,      "default", "Waiting...");
      setCheckState(chkRtc,     valRtc,     "default", "Waiting...");
      setCheckState(chkWifi,    valWifi,    "default", "Waiting...");
      setCheckState(chkSensor,  valSensor,  "default", "Waiting...");
      setCheckState(chkWindSpd, valWindSpd, "default", "Waiting...");
      setCheckState(chkWindDir, valWindDir, "default", "Waiting...");
      setCheckState(chkBatt,    valBatt,    "default", "Waiting...");
      setCheckState(chkSys3v3,  valSys3v3,  "default", "Waiting...");
      setCheckState(chkSolar,   valSolar,   "default", "Waiting...");
      setCheckState(chkGprs,    valGprs,    "default", "Waiting...");
      setCheckState(chkLcd,     valLcd,     "default", "Waiting...");
      setCheckState(chkKeypad,  valKeypad,  "default", "Waiting...");
      setCheckState(chkRf,      valRf,      "default", "Waiting...");
      setCheckState(chkSleep,   valSleep,   "default", "Waiting...");
      
      // Re-apply subsystem ignores
      updateSubsystemUI();
      
      operatorGuideBox.style.display = "block";
      operatorGuideText.innerHTML = "🔍 Running automatic peripheral sweep. Please wait...";
      
      metaImei.textContent = "---------------";
      metaCcid.textContent = "--------------------";
      metaCarrier.textContent = "UNKNOWN";
      metaNuvoton.textContent = "---------";
      _shownPhases.clear();

      logToConsole("Starting live serial diagnostics listener...", "info");
      
      // Loop reading incoming logs
      while (serialPort.readable && !cancelReadLoop) {
        activeReader = serialPort.readable.getReader();
        try {
          let buffer = "";
          while (true) {
            const { value, done } = await activeReader.read();
            if (done) {
              logToConsole("Serial stream closed by device.", "warning");
              cancelReadLoop = true;
              stopDiagnostics();
              break;
            }
            if (cancelReadLoop) {
              break;
            }
            // Decode value to text
            const text = new TextDecoder().decode(value);
            buffer += text;
            
            // Split into lines
            let lines = buffer.split("\n");
            buffer = lines.pop(); // Keep partial line in buffer
            
            for (const line of lines) {
              const cleaned = line.trim();
              if (cleaned) {
                // Log to terminal console UI
                logToConsole(cleaned);
                
                // Parse checklist status updates
                parseLogLine(cleaned);
              }
            }
          }
        } catch (e) {
          logToConsole(`Read loop error: ${e.message}`, "error");
          cancelReadLoop = true;
          stopDiagnostics();
          break;
        } finally {
          if (activeReader) {
            try { activeReader.releaseLock(); } catch (_) {}
            activeReader = null;
          }
          checkPreconditions();
        }
      }
    }

    function parseLogLine(line) {
      // Handshake parser
      if (line.includes("WAITING_FOR_CONFIG")) {
        const espVal = testCfgEsp.checked ? "ESP" : "";
        const gprsVal = testCfgGprs.checked ? "GPRS" : "";
        const nuvVal = testCfgNuvoton.checked ? "NUV" : "";
        const profileVal = document.getElementById("deviceProfile")?.value || "TRG";
        const configStr = `CFG:${[espVal, gprsVal, nuvVal].filter(Boolean).join(",")};PROFILE:${profileVal}\n`;
        logToConsole(`🔄 Handshaking with board: Sending selected test config: ${configStr.trim()}`, "info");
        sendSerialCommand(configStr);
      }

      // Check for ignored items
      if (line.includes("SPIFFS_CHECK: IGNORED")) setCheckState(chkSpiffs, valSpiffs, "ignored", "SKIPPED");
      if (line.includes("SD_CHECK: IGNORED"))     setCheckState(chkSd, valSd, "ignored", "SKIPPED");
      if (line.includes("RTC_CHECK: IGNORED"))    setCheckState(chkRtc, valRtc, "ignored", "SKIPPED");
      if (line.includes("WIFI_CHECK: IGNORED"))   setCheckState(chkWifi, valWifi, "ignored", "SKIPPED");
      if (line.includes("SENSOR_CHECK: IGNORED")) setCheckState(chkSensor, valSensor, "ignored", "SKIPPED");
      if (line.includes("WIND_SPD: IGNORED"))     setCheckState(chkWindSpd, valWindSpd, "ignored", "SKIPPED");
      if (line.includes("WIND_DIR: IGNORED"))     setCheckState(chkWindDir, valWindDir, "ignored", "SKIPPED");
      if (line.includes("EXT0_WAKEUP: IGNORED"))  setCheckState(chkSleep, valSleep, "ignored", "SKIPPED");
      if (line.includes("ADC_CHECK: IGNORED")) {
        setCheckState(chkBatt, valBatt, "ignored", "SKIPPED");
        setCheckState(chkSys3v3, valSys3v3, "ignored", "SKIPPED");
        setCheckState(chkSolar, valSolar, "ignored", "SKIPPED");
      }
      if (line.includes("MODEM_INIT: IGNORED"))   setCheckState(chkGprs, valGprs, "ignored", "SKIPPED");
      if (line.includes("NUVOTON_COMM: IGNORED") || line.includes("LCD_TEST: IGNORED")) setCheckState(chkLcd, valLcd, "ignored", "SKIPPED");
      if (line.includes("KEYPAD_TEST: IGNORED"))  setCheckState(chkKeypad, valKeypad, "ignored", "SKIPPED");
      if (line.includes("RF_CHECK: IGNORED"))     setCheckState(chkRf, valRf, "ignored", "SKIPPED");

      // SPIFFS
      if (line.includes("SPIFFS_CHECK: OK")) {
        setCheckState(chkSpiffs, valSpiffs, "pass", "PASS");
      } else if (line.includes("SPIFFS_CHECK: FAIL")) {
        setCheckState(chkSpiffs, valSpiffs, "fail", "FAIL");
      }
      
      // SD Card
      if (line.includes("SD_CHECK: OK")) {
        setCheckState(chkSd, valSd, "pass", "PASS");
      } else if (line.includes("SD_CHECK: FAIL")) {
        setCheckState(chkSd, valSd, "fail", "FAIL");
      }
      
      // RTC
      if (line.includes("RTC_CHECK: OK")) {
        setCheckState(chkRtc, valRtc, "pass", "PASS");
      } else if (line.includes("RTC_CHECK: FAIL")) {
        setCheckState(chkRtc, valRtc, "fail", "FAIL");
      }
      
      // WiFi Scan
      if (line.includes("WIFI_CHECK: OK")) {
        const match = line.match(/\((\d+)\s+APs/);
        const count = match ? match[1] : "OK";
        setCheckState(chkWifi, valWifi, "pass", `PASS (${count} APs)`);
      } else if (line.includes("WIFI_CHECK: FAIL")) {
        setCheckState(chkWifi, valWifi, "fail", "FAIL");
      }
      
      // Environmental Sensor
      if (line.includes("SENSOR_CHECK: OK")) {
        const matchName = line.match(/\(([^:]+):/);
        const name = matchName ? matchName[1] : "OK";
        const matchVal = line.match(/Temp=([^,]+),\s*Hum=([^)]+)/);
        const values = matchVal ? `${matchVal[1]} | ${matchVal[2]}` : "PASS";
        setCheckState(chkSensor, valSensor, "pass", `${name} (${values})`);
      } else if (line.includes("SENSOR_CHECK: FAIL")) {
        setCheckState(chkSensor, valSensor, "fail", "FAIL");
      }

      // Wind Telemetry
      if (line.includes("[QC_TELEMETRY]")) {
        const wsMatch = line.match(/WS_PULSES=(\d+)/);
        const wdMatch = line.match(/WD_ADC=(\d+)/);
        if (wsMatch && chkWindSpd) {
          const pulses = parseInt(wsMatch[1], 10);
          setCheckState(chkWindSpd, valWindSpd, "pass", `ACTIVE (${pulses} pulses)`);
        }
        if (wdMatch && chkWindDir) {
          const adc = parseInt(wdMatch[1], 10);
          const volts = ((adc / 4095.0) * 3.3).toFixed(2);
          setCheckState(chkWindDir, valWindDir, "pass", `ACTIVE (${adc} / ${volts}V)`);
        }
      }
      
      // Modem
      if (line.includes("MODEM_INIT: OK")) {
        setCheckState(chkGprs, valGprs, "wait", "AT OK — Registering...");
      } else if (line.includes("MODEM_INIT: FAIL")) {
        setCheckState(chkGprs, valGprs, "fail", "NO COMM");
      } else if (line.includes("MODEM_INIT: REG_OK")) {
        // Only set pass when network registration is actually confirmed
        const csqHint = valGprs.textContent.includes("CSQ") ? ` (${valGprs.textContent.split("(")[1]?.replace(")","") || ""})` : "";
        setCheckState(chkGprs, valGprs, "pass", `REGISTERED${csqHint}`);
      } else if (line.includes("MODEM_INIT: REG_FAIL")) {
        setCheckState(chkGprs, valGprs, "fail", "REG FAIL");
      }
      if (line.includes("MODEM_IMEI:")) {
        const parts = line.split("MODEM_IMEI:");
        if (parts.length > 1) {
          const imei = parts[1].trim();
          metaImei.textContent = imei;
        }
      }
      if (line.includes("MODEM_CCID:")) {
        const parts = line.split("MODEM_CCID:");
        if (parts.length > 1) {
          const ccid = parts[1].trim();
          metaCcid.textContent = ccid;
        }
      }
      if (line.includes("MODEM_CARRIER:")) {
        const parts = line.split("MODEM_CARRIER:");
        if (parts.length > 1) {
          const carrier = parts[1].trim();
          metaCarrier.textContent = carrier;
        }
      }
      if (line.includes("MODEM_CSQ:")) {
        const parts = line.split("MODEM_CSQ:");
        if (parts.length > 1) {
          const csq = parts[1].trim();
          // Show signal strength but keep 'wait' state — registration hasn't passed yet
          setCheckState(chkGprs, valGprs, "wait", `Signal OK (CSQ: ${csq}) — Registering...`);
        }
      }

      // ADC / Power Rails — parse raw values and compute voltages
      // Line format: [QC_JIG] BATT_3V7_ADC: 3706, SYS_3V3_ADC: 2877, SOLAR_ADC: 431
      if (line.includes("BATT_3V7_ADC:") && line.includes("SOLAR_ADC:")) {
        const battMatch  = line.match(/BATT_3V7_ADC:\s*(\d+)/);
        const sysMatch   = line.match(/SYS_3V3_ADC:\s*(\d+)/);
        const solarMatch = line.match(/SOLAR_ADC:\s*(\d+)/);

        if (battMatch) {
          const raw  = parseInt(battMatch[1]);
          const volt = ((raw / ADC_MAX) * ADC_VREF * BATT_MULT).toFixed(2);
          const ok   = parseFloat(volt) >= 3.5 && parseFloat(volt) <= 4.4; // nom 3.7V GPRS battery
          setCheckState(chkBatt, valBatt, ok ? "pass" : "warn", `${volt} V  (raw ${raw})`);
        }
        if (solarMatch) {
          const raw  = parseInt(solarMatch[1]);
          const volt = ((raw / ADC_MAX) * ADC_VREF * SOLAR_MULT).toFixed(2);
          const present = parseFloat(volt) > 0.1;
          setCheckState(chkSolar, valSolar, present ? "pass" : "warn",
            present ? `${volt} V  (raw ${raw})` : `${volt} V  — No panel?`);
        }
        if (sysMatch) {
          const raw  = parseInt(sysMatch[1]);
          const volt = ((raw / ADC_MAX) * ADC_VREF * BATT_MULT).toFixed(2);
          const ok   = parseFloat(volt) >= 3.0 && parseFloat(volt) <= 3.6; // nom 3.2V ESP32 battery
          setCheckState(chkSys3v3, valSys3v3, ok ? "pass" : "warn", `${volt} V  (raw ${raw})`);
          logToConsole(`  ↳ ESP32 Battery ADC: ${raw} → ${volt} V`, "info");
        }
      }

      // Nuvoton Handshake
      if (line.includes("NUVOTON_COMM: OK")) {
        setCheckState(chkLcd, valLcd, "pass", "PASS");
        metaNuvoton.textContent = "LCD OK";
      }

      // Interactive Test Steps Parsing
      if (line.includes("[QC_STEP] LCD_TEST: WAITING")) {
        setCheckState(chkLcd, valLcd, "wait", "Verify LCD Display...");
        const html = `
          👀 <b>LCD Verification</b>: Check the Nuvoton LCD screen.<br>
          It should show <i>'LCD TEST: READ? PRESS CLEAR KEY'</i>.<br>
          If you can read it, press the <b>CLEAR</b> key on the keypad.<br><br>
          <div style="display:flex; gap:0.5rem; align-items:center;">
            <button class="btn btn-success" id="btnOverrideLcdPass" style="font-size:0.78rem; padding:0.4rem 0.8rem; width:auto; text-transform:none;">✓ Force Pass [Y]</button>
            <button class="btn btn-danger" id="btnOverrideLcdFail" style="font-size:0.78rem; padding:0.4rem 0.8rem; width:auto; text-transform:none;">✗ Force Fail [N]</button>
          </div>
        `;
        operatorGuideText.innerHTML = html;
        showOperatorPrompt(html, () => {
          document.getElementById("btnOverrideLcdPass")?.addEventListener("click", () => {
            logToConsole("Dashboard Override: LCD Pass sent.", "info");
            sendSerialCommand("CMD:LCD_PASS\n");
            hideOperatorPrompt();
          });
          document.getElementById("btnOverrideLcdFail")?.addEventListener("click", () => {
            logToConsole("Dashboard Override: LCD Fail.", "error");
            window._qcHasFailedInSession = true;
            setCheckState(chkLcd, valLcd, "fail", "FAILED (Manual)");
            showVerdict("FAIL", "LCD display check failed manually by operator override.");
            autoSyncToGoogleSheets("FAIL: LCD Manual");
            hideOperatorPrompt();
          });
        });
        playAlertChime();
      }
      if (line.includes("[QC_STEP] LCD_TEST: PASS")) {
        setCheckState(chkLcd, valLcd, "pass", "PASS");
        hideOperatorPrompt();
      }

      if (line.includes("[QC_STEP] KEYPAD_TEST: WAITING_LEFT")) {
        setCheckState(chkKeypad, valKeypad, "wait", "Press LEFT...");
        const html = getKeypadSweepHTML("LEFT");
        operatorGuideText.innerHTML = html;
        showOperatorPrompt(html, attachKeypadOverrideListeners);
        playAlertChime();
      }
      if (line.includes("[QC_STEP] KEYPAD_TEST: WAITING_UP")) {
        setCheckState(chkKeypad, valKeypad, "wait", "Press UP...");
        const html = getKeypadSweepHTML("UP");
        operatorGuideText.innerHTML = html;
        showOperatorPrompt(html, attachKeypadOverrideListeners);
      }
      if (line.includes("[QC_STEP] KEYPAD_TEST: WAITING_DOWN")) {
        setCheckState(chkKeypad, valKeypad, "wait", "Press DOWN...");
        const html = getKeypadSweepHTML("DOWN");
        operatorGuideText.innerHTML = html;
        showOperatorPrompt(html, attachKeypadOverrideListeners);
      }
      if (line.includes("[QC_STEP] KEYPAD_TEST: WAITING_RIGHT")) {
        setCheckState(chkKeypad, valKeypad, "wait", "Press RIGHT...");
        const html = getKeypadSweepHTML("RIGHT");
        operatorGuideText.innerHTML = html;
        showOperatorPrompt(html, attachKeypadOverrideListeners);
      }
      if (line.includes("[QC_STEP] KEYPAD_TEST: WAITING_SET")) {
        setCheckState(chkKeypad, valKeypad, "wait", "Press SET...");
        const html = getKeypadSweepHTML("SET");
        operatorGuideText.innerHTML = html;
        showOperatorPrompt(html, attachKeypadOverrideListeners);
      }
      if (line.includes("[QC_STEP] KEYPAD_TEST: PASS")) {
        setCheckState(chkKeypad, valKeypad, "pass", "PASS");
        hideOperatorPrompt();
      }

      if (line.includes("[QC_STEP] RF_CHECK: WAITING_JUMPER")) {
        setCheckState(chkRf, valRf, "wait", "Give RF Tips");
        playAlertChime();
        
        let html = "";
        if (!testCfgNuvoton.checked) {
          html = `
            🌧️ <b>Rainfall (RF) Test (No Keypad)</b>:<br>
            1. Ensure the rainfall sensor (or wire to GND) is connected to <b>GPIO 34</b>.<br>
            2. Click the button below to start the test and count tips:<br><br>
            <div style="display:flex; gap:0.5rem; align-items:center;">
              <button class="btn btn-primary" id="btnStartRfSerial" style="font-size: 0.78rem; padding: 0.4rem 0.8rem; width: auto; text-transform: none; font-weight: 600;">⚡ Start RF Counting [Y]</button>
              <button class="btn btn-success" id="btnOverrideRfPass" style="font-size:0.78rem; padding:0.4rem 0.8rem; width:auto; text-transform:none;">✓ Force Pass [N]</button>
            </div>
          `;
          operatorGuideText.innerHTML = html;
          showOperatorPrompt(html, () => {
            const btn = document.getElementById("btnStartRfSerial");
            if (btn) {
              btn.addEventListener("click", () => {
                btn.disabled = true;
                btn.textContent = "Counting...";
                // Keep modal open, let counting step update it
                logToConsole("Sending CMD:START_RF to board via serial...", "info");
                sendSerialCommand("CMD:START_RF\n");
              });
            }
            document.getElementById("btnOverrideRfPass")?.addEventListener("click", () => {
              logToConsole("Dashboard Override: RF Pass sent.", "info");
              sendSerialCommand("CMD:RF_PASS\n");
              hideOperatorPrompt();
            });
          });
        } else {
          html = `
            🌧️ <b>Rainfall (RF) Test</b>:<br>
            1. Ensure the rainfall sensor is connected to <b>GPIO 34</b>.<br>
            2. Press the <b>SET</b> key on the keypad to start the tipping count.<br><br>
            <div style="display:flex; gap:0.5rem; align-items:center;">
              <button class="btn btn-success" id="btnOverrideRfPass" style="font-size:0.78rem; padding:0.4rem 0.8rem; width:auto; text-transform:none;">✓ Skip/Pass RF Test [Y]</button>
              <button class="btn btn-danger" id="btnOverrideRfFail" style="font-size:0.78rem; padding:0.4rem 0.8rem; width:auto; text-transform:none;">✗ Force Fail [N]</button>
            </div>
          `;
          operatorGuideText.innerHTML = html;
          showOperatorPrompt(html, () => {
            document.getElementById("btnOverrideRfPass")?.addEventListener("click", () => {
              logToConsole("Dashboard Override: RF Pass sent.", "info");
              sendSerialCommand("CMD:RF_PASS\n");
              hideOperatorPrompt();
            });
            document.getElementById("btnOverrideRfFail")?.addEventListener("click", () => {
              logToConsole("Dashboard Override: RF Fail.", "error");
              window._qcHasFailedInSession = true;
              setCheckState(chkRf, valRf, "fail", "FAILED (Manual)");
              showVerdict("FAIL", "Rainfall RF check failed manually by operator override.");
              autoSyncToGoogleSheets("FAIL: RF Manual");
              hideOperatorPrompt();
            });
          });
        }
      }

      // RF COUNT IN PROGRESS — operator manually tips the rain gauge
      if (line.includes("[QC_STEP] RF_CHECK: COUNTING")) {
        setCheckState(chkRf, valRf, "wait", "Counting... 0 tips (0.00 mm)");
        playAlertChime();
        let html = "";
        if (!testCfgNuvoton.checked) {
          html = `
            🌧️ <b>Rainfall RF Count in Progress</b>:<br>
            Give rainfall tips manually (e.g. tip the rain gauge bucket).<br>
            ⏱️ Wait ~100ms between each contact — the 50ms debounce must clear first.<br>
            Each tip = <b>0.25 mm</b>. Live count updates in the checklist.<br>
            When done, click the button below to tally, or wait 1 minute for automatic tally:<br><br>
            <div style="display:flex; gap:0.5rem; align-items:center;">
              <button class="btn btn-primary" id="btnTallyRf" style="font-size:0.78rem; padding:0.4rem 0.8rem; width:auto; text-transform:none; font-weight:600;">Finish & Tally RF [Y]</button>
              <button class="btn btn-success" id="btnOverrideRfPass" style="font-size:0.78rem; padding:0.4rem 0.8rem; width:auto; text-transform:none;">✓ Force Pass [N]</button>
            </div>
          `;
          operatorGuideText.innerHTML = html;
          showOperatorPrompt(html, () => {
            document.getElementById("btnTallyRf")?.addEventListener("click", () => {
              logToConsole("Sending CMD:RF_TALLY to board...", "info");
              sendSerialCommand("CMD:RF_TALLY\n");
              // Keep modal open for confirmation step
            });
            document.getElementById("btnOverrideRfPass")?.addEventListener("click", () => {
              logToConsole("Dashboard Override: RF Force Pass sent.", "info");
              sendSerialCommand("CMD:RF_PASS\n");
              hideOperatorPrompt();
            });
          });
        } else {
          html = `
            🌧️ <b>Rainfall RF Count in Progress</b>:<br>
            Give rainfall tips manually (e.g. tip the rain gauge bucket).<br>
            ⏱️ Wait ~100ms between each contact — the 50ms debounce must clear first.<br>
            Each tip = <b>0.25 mm</b>. Live count updates in the checklist.<br>
            <b>Press the SET key on the keypad when done</b>, or wait 1 minute for automatic tally.<br><br>
            <div style="display:flex; gap:0.5rem; align-items:center;">
              <button class="btn btn-success" id="btnOverrideRfPass" style="font-size:0.78rem; padding:0.4rem 0.8rem; width:auto; text-transform:none;">✓ Force Pass [Y]</button>
              <button class="btn btn-danger" id="btnOverrideRfFail" style="font-size:0.78rem; padding:0.4rem 0.8rem; width:auto; text-transform:none;">✗ Force Fail [N]</button>
            </div>
          `;
          operatorGuideText.innerHTML = html;
          showOperatorPrompt(html, () => {
            document.getElementById("btnOverrideRfPass")?.addEventListener("click", () => {
              logToConsole("Dashboard Override: RF Force Pass sent.", "info");
              sendSerialCommand("CMD:RF_PASS\n");
              hideOperatorPrompt();
            });
            document.getElementById("btnOverrideRfFail")?.addEventListener("click", () => {
              logToConsole("Dashboard Override: RF Force Fail.", "error");
              window._qcHasFailedInSession = true;
              setCheckState(chkRf, valRf, "fail", "FAILED (Manual)");
              showVerdict("FAIL", "Rainfall RF check failed manually by operator override.");
              autoSyncToGoogleSheets("FAIL: RF Force Manual");
              hideOperatorPrompt();
            });
          });
        }
      }

      // RF COUNT TALLIED — operator confirms pass/fail
      if (line.includes("[QC_STEP] RF_CHECK: CONFIRMING")) {
        const tipMatch = line.match(/CONFIRMING\s*\((\d+)\s*tips\)/);
        const tipCount = tipMatch ? parseInt(tipMatch[1], 10) : 0;
        const RF_MM_PER_TIP = 0.25;
        const mmVal = (tipCount * RF_MM_PER_TIP).toFixed(2);
        setCheckState(chkRf, valRf, "wait", `Confirming... ${tipCount} tips (${mmVal} mm)`);
        playAlertChime();
        const html = `
          🌧️ <b>Confirm Rainfall RF Count</b>:<br>
          We counted <b>${tipCount} tips</b> (${mmVal} mm) on the gauge.<br>
          Is this count correct?<br><br>
          <div style="display:flex; gap:0.5rem; align-items:center;">
            <button class="btn btn-success" id="btnConfirmRfPass" style="font-size:0.78rem; padding:0.4rem 0.8rem; width:auto; text-transform:none; font-weight:600;">✓ Yes (SET) [Y]</button>
            <button class="btn btn-danger" id="btnConfirmRfFail" style="font-size:0.78rem; padding:0.4rem 0.8rem; width:auto; text-transform:none; font-weight:600;">✗ No (CLEAR) [N]</button>
          </div>
        `;
        operatorGuideText.innerHTML = html;
        showOperatorPrompt(html, () => {
          document.getElementById("btnConfirmRfPass")?.addEventListener("click", () => {
            logToConsole("Confirming RF Pass via serial...", "info");
            sendSerialCommand("CMD:RF_PASS\n");
            hideOperatorPrompt();
          });
          document.getElementById("btnConfirmRfFail")?.addEventListener("click", () => {
            logToConsole("Confirming RF Fail via serial...", "info");
            sendSerialCommand("CMD:RF_FAIL\n");
            hideOperatorPrompt();
          });
        });
      }

      // Live RF tip counter — parse each [QC_JIG] RF_TIP: N line
      if (line.includes("[QC_JIG] RF_TIP:")) {
        const tipMatch = line.match(/RF_TIP:\s*(\d+)/);
        if (tipMatch) {
          const tipCount = parseInt(tipMatch[1], 10);
          const RF_MM_PER_TIP = 0.25;
          const mmVal = (tipCount * RF_MM_PER_TIP).toFixed(2);
          setCheckState(chkRf, valRf, "wait", `Counting... ${tipCount} tips (${mmVal} mm)`);
          // Update modal prompt if open to reflect live count
          const promptContent = document.getElementById("operatorPromptContent");
          if (promptContent && document.getElementById("operatorPromptModal").style.display === "flex") {
            const countBtn = document.getElementById("btnTallyRf");
            if (countBtn) {
              // Tally flow (serial command based)
              promptContent.querySelector("b").textContent = `Counting... ${tipCount} tips (${mmVal} mm)`;
            }
          }
        }
      }
      if (line.includes("[QC_STEP] RF_CHECK: PASS")) {
        const passMatch = line.match(/(\d+)\s*tips/);
        const tipCount = passMatch ? parseInt(passMatch[1], 10) : 0;
        const RF_MM_PER_TIP = 0.25;
        const mmVal = (tipCount * RF_MM_PER_TIP).toFixed(2);
        setCheckState(chkRf, valRf, "pass", `PASS — ${tipCount} tips = ${mmVal} mm`);
        operatorGuideText.innerHTML = `🌧️ Rainfall RF test passed! <b>${tipCount} tips tallied = ${mmVal} mm</b>. Entering Deep Sleep...`;
        hideOperatorPrompt();
      }

      // Sleep Wake Test
      if (line.includes("EXT0_WAKEUP: WAITING_SLEEP")) {
        setCheckState(chkSleep, valSleep, "wait", "Press SET key to wake");
        operatorGuideBox.style.display = "block";
        const html = `
          💤 <b>Deep Sleep Test</b>: ESP32 has entered deep sleep and the LCD is off.<br>
          Press the <b>SET key</b> on the keypad (or the wake button) to wake the board up and finish the test.<br><br>
          <div style="display:flex; gap:0.5rem; align-items:center;">
            <button class="btn btn-success" id="btnOverrideSleepPass" style="font-size:0.78rem; padding:0.4rem 0.8rem; width:auto; text-transform:none;">✓ Force Wakeup Pass [Y]</button>
          </div>
        `;
        operatorGuideText.innerHTML = html;
        showOperatorPrompt(html, () => {
          document.getElementById("btnOverrideSleepPass")?.addEventListener("click", () => {
            logToConsole("Dashboard Override: Sleep Wakeup Pass sent.", "info");
            sendSerialCommand("CMD:SLEEP_PASS\n");
            hideOperatorPrompt();
          });
        });
      }
      if (line.includes("EXT0_WAKEUP: PASS")) {
        setCheckState(chkSleep, valSleep, "pass", "PASS");
        operatorGuideText.innerHTML = "✅ Deep sleep wakeup verified successfully!";
        hideOperatorPrompt();
      }
      if (line.includes("[QC_STEP] RF_CHECK: FAIL")) {
        const match = line.match(/got\s*(\d+)/);
        const gotVal = match ? match[1] : "0";
        setCheckState(chkRf, valRf, "fail", `FAIL (got ${gotVal} tips)`);
        playFailureChime();
        
        let html = "";
        if (!testCfgNuvoton.checked) {
          html = `
            ❌ <b>RF Check Mismatch</b> (Counted ${gotVal} tips).<br>
            Make sure the sensor is connected, then click below to retry:<br><br>
            <div style="display:flex; gap:0.5rem; align-items:center;">
              <button class="btn btn-primary" id="btnStartRfSerial" style="font-size: 0.78rem; padding: 0.4rem 0.8rem; width: auto; text-transform: none; font-weight: 600;">⚡ Retry RF Test [Y]</button>
              <button class="btn btn-success" id="btnOverrideRfPass" style="font-size:0.78rem; padding:0.4rem 0.8rem; width:auto; text-transform:none;">✓ Force Pass [N]</button>
            </div>
          `;
          operatorGuideText.innerHTML = html;
          showOperatorPrompt(html, () => {
            const btn = document.getElementById("btnStartRfSerial");
            if (btn) {
              btn.addEventListener("click", () => {
                btn.disabled = true;
                btn.textContent = "Running pulses...";
                logToConsole("Sending CMD:START_RF retry command...", "info");
                sendSerialCommand("CMD:START_RF\n");
              });
            }
            document.getElementById("btnOverrideRfPass")?.addEventListener("click", () => {
              logToConsole("Dashboard Override: RF Pass sent.", "info");
              sendSerialCommand("CMD:RF_PASS\n");
              hideOperatorPrompt();
            });
          });
        } else {
          html = `
            ❌ <b>RF Check Mismatch</b> (Counted ${gotVal} tips).<br>
            Make sure the sensor is connected, then press <b>SET</b> to retry.<br><br>
            <div style="display:flex; gap:0.5rem; align-items:center;">
              <button class="btn btn-success" id="btnOverrideRfPass" style="font-size:0.78rem; padding:0.4rem 0.8rem; width:auto; text-transform:none;">✓ Force Pass [Y]</button>
            </div>
          `;
          operatorGuideText.innerHTML = html;
          showOperatorPrompt(html, () => {
            document.getElementById("btnOverrideRfPass")?.addEventListener("click", () => {
              logToConsole("Dashboard Override: RF Pass sent.", "info");
              sendSerialCommand("CMD:RF_PASS\n");
              hideOperatorPrompt();
            });
          });
        }
      }

      // PERIPHERAL FAILURES — auto-sync immediately with FAIL verdict
      const criticalFails = [
        "SPIFFS_CHECK: FAIL", "RTC_CHECK: FAIL", "MODEM_INIT: FAIL", "MODEM_INIT: REG_FAIL", "WIFI_CHECK: FAIL", "SENSOR_CHECK: FAIL"
      ];
      if (criticalFails.some(f => line.includes(f))) {
        window._qcHasFailedInSession = true; // Mark session as failed
        btnExportLog.disabled = false;
        // Debounce: only fire once per diagnostics session
        if (!window._qcFailSynced) {
          window._qcFailSynced = true;
          const failedItem = criticalFails.find(f => line.includes(f))
            .replace("_CHECK: FAIL", "")
            .replace("MODEM_INIT: REG_FAIL", "GPRS Net Reg")
            .replace("MODEM_INIT: FAIL", "GPRS Modem")
            .replace("WIFI: FAIL", "WiFi Radio")
            .replace("SENSOR: FAIL", "Environmental Sensor")
            .replace("SPIFFS", "Internal SPIFFS")
            .replace("SD", "SD Card")
            .replace("RTC", "Real-Time Clock");

          // Compute duration NOW before syncing so Google Sheet gets the real value
          if (testStartTime) testDuration = Math.round((Date.now() - testStartTime) / 1000);
          logToConsole("⚠ Critical peripheral FAIL detected — syncing FAIL result to Google Sheet...", "warn");
          setTimeout(() => autoSyncToGoogleSheets(`FAIL: ${failedItem}`), 500);
          
          showVerdict("FAIL", `Critical hardware peripheral test failed: <b>${failedItem}</b>.<br>Please inspect the board's routing, power supply, and chip soldering.`);
        }
      }

      if (line.includes("QC_RESULT: FAIL")) {
        window._qcHasFailedInSession = true;
        showResultBanner("FAIL");
      }

      // FULL PASS — keypad SET confirmed
      if (line.includes("QC_RESULT: PASS")) {
        if (window._qcHasFailedInSession) {
          logToConsole("⚠️ Warning: Received QC_RESULT: PASS from board, but a critical failure was recorded earlier in this session! Verdict remains FAIL.", "error");
          return;
        }
        window._qcFailSynced = false; // reset for next run
        logToConsole("🎉 ALL SELF-TESTS PASSED!", "info");
        btnProceedApp.style.display = "block";
        btnExportLog.disabled = false;
        // Compute duration NOW before syncing so Google Sheet gets the real value
        if (testStartTime) testDuration = Math.round((Date.now() - testStartTime) / 1000);
        logToConsole("📊 Auto-syncing PASS result to Google Sheet...", "info");
        autoSyncToGoogleSheets("PASS");
        
        const espVal = testCfgEsp.checked ? "ESP" : "";
        const gprsVal = testCfgGprs.checked ? "GPRS" : "";
        const nuvVal = testCfgNuvoton.checked ? "NUV" : "";
        const subsystemsTested = [espVal, gprsVal, nuvVal].filter(Boolean).join(" + ");
        showVerdict("PASS", `All active hardware diagnostics passed successfully for:<br><b>${subsystemsTested}</b>`);
        showResultBanner("PASS");
      }
    }

    // --- Flashing Engine Orchestrator ---
    btnStart.addEventListener("click", async () => {
      hideOperatorPrompt();
      if (!isConnected) {
        logToConsole("Error: Board is not connected. Please connect the board first.", "error");
        return;
      }
      if (!testCfgEsp.checked) {
        await runDiagnosticsOnly();
        return;
      }
      isFlashing = true;
      checkPreconditions();
      progressContainer.style.display = "block";
      progressBar.style.width = "0%";
      
      try {
        if (activeFlow === 'factory') {
          await runFactoryFlashingFlow();
        } else if (activeFlow === 'upgrade') {
          await runUpgradeFlashingFlow();
        } else {
          await runGprsDebugFlashFlow();
        }
      } catch (err) {
        logToConsole(`Flashing failed: ${err.message}`, "error");
        progressContainer.style.display = "none";
      } finally {
        isFlashing = false;
        checkPreconditions();
      }
    });

    // --- Integration Testing Orchestrator ---
    btnStartIntegration.addEventListener("click", async () => {
      if (!isConnected) {
        logToConsole("Error: Board is not connected. Please connect the board first.", "error");
        return;
      }
      isIntegrating = true;
      window._integrationSuccessTriggered = false;
      checkPreconditions();
      integrationProgressContainer.style.display = "block";
      integrationProgressBar.style.width = "0%";
      
      try {
        const activeConfig = releaseConfigSel?.value;
        const configData = activeConfig ? customConfigVault[activeConfig] : null;
        const productionBin = (configData && configData.binary) || fileVault[`production_${currentChipSize}mb.bin`];

        if (!productionBin) {
          throw new Error("No production binary found. Please load firmware files first.");
        }

        logToConsole("--- STARTING ENCLOSURE INTEGRATION FLASH & TEST ---", "info");
        await ensureEsploaderConnected();
        
        logToConsole(`Flashing Production binary (${(productionBin.byteLength/1024).toFixed(1)} KB) to offset 0x10000...`, "normal");
        await esploader.writeFlash({
          fileArray: [{ data: productionBin, address: 0x10000 }],
          flashSize: "keep",
          compress: true,
          reportProgress: (fileIndex, writtenBytes, totalBytes) => {
            if (totalBytes > 0) {
              const percentage = Math.round((writtenBytes / totalBytes) * 100);
              integrationProgressBar.style.width = `${percentage}%`;
            }
          }
        });
        integrationProgressBar.style.width = "100%";
        
        logToConsole("Flashing complete! Resetting board...", "info");
        await esploader.after("hard_reset");
        
        logToConsole("Releasing flasher connection...", "normal");
        if (transport) {
          try { await transport.disconnect(); } catch (e) {}
        }
        transport = null;
        esploader = null;

        logToConsole("Opening serial diagnostics channel at 115200 baud...", "normal");
        try {
          await port.open({ baudRate: 115200 });
          await port.setSignals({ dataTerminalReady: false, requestToSend: true });
          await new Promise(r => setTimeout(r, 200));
          await port.setSignals({ dataTerminalReady: false, requestToSend: false });
        } catch (err) {
          logToConsole(`Failed to open diagnostics channel: ${err.message}`, "error");
          isIntegrating = false;
          checkPreconditions();
          return;
        }

        setTimeout(startIntegrationDiagnostics, 500);

      } catch (err) {
        logToConsole(`Integration flashing failed: ${err.message}`, "error");
        integrationProgressContainer.style.display = "none";
        isIntegrating = false;
        checkPreconditions();
      }
    });

    // --- Servicing & Repair Orchestrator ---
    btnStartRepair.addEventListener("click", async () => {
      if (!isConnected) {
        logToConsole("Error: Board is not connected. Please connect the board first.", "error");
        return;
      }
      isIntegrating = true;
      window._integrationSuccessTriggered = false;
      checkPreconditions();
      repairProgressContainer.style.display = "block";
      repairProgressBar.style.width = "0%";
      
      try {
        const activeConfig = releaseConfigSel?.value;
        const configData = activeConfig ? customConfigVault[activeConfig] : null;
        const productionBin = (configData && configData.binary) || fileVault[`production_${currentChipSize}mb.bin`];

        if (!productionBin) {
          throw new Error("No production binary found. Please load firmware files first.");
        }

        logToConsole("--- STARTING SERVICING & REPAIR FLASH & TEST ---", "info");
        
        logToConsole(`Flashing Production binary (${(productionBin.byteLength/1024).toFixed(1)} KB) to offset 0x10000...`, "normal");
        await esploader.writeFlash({
          fileArray: [{ data: productionBin, address: 0x10000 }],
          flashSize: "keep",
          compress: true,
          reportProgress: (fileIndex, writtenBytes, totalBytes) => {
            if (totalBytes > 0) {
              const percentage = Math.round((writtenBytes / totalBytes) * 100);
              repairProgressBar.style.width = `${percentage}%`;
            }
          }
        });
        repairProgressBar.style.width = "100%";
        
        logToConsole("Flashing complete! Resetting board...", "info");
        await esploader.after("hard_reset");
        
        logToConsole("Releasing flasher connection...", "normal");
        if (transport) {
          try { await transport.disconnect(); } catch (e) {}
        }
        transport = null;
        esploader = null;

        logToConsole("Opening serial diagnostics channel at 115200 baud...", "normal");
        try {
          await port.open({ baudRate: 115200 });
          await port.setSignals({ dataTerminalReady: false, requestToSend: true });
          await new Promise(r => setTimeout(r, 200));
          await port.setSignals({ dataTerminalReady: false, requestToSend: false });
        } catch (err) {
          logToConsole(`Failed to open diagnostics channel: ${err.message}`, "error");
          isIntegrating = false;
          checkPreconditions();
          return;
        }

        setTimeout(startIntegrationDiagnostics, 500);

      } catch (err) {
        logToConsole(`Repair flashing failed: ${err.message}`, "error");
        repairProgressContainer.style.display = "none";
        isIntegrating = false;
        checkPreconditions();
      }
    });

    btnAbortIntegration.addEventListener("click", async () => {
      logToConsole("--- ABORTING INTEGRATION TEST ---", "warning");
      cancelReadLoop = true;
      if (activeReader) {
        try { await activeReader.cancel(); } catch (e) {}
      }
      await new Promise(r => setTimeout(r, 150));
      if (port) {
        try { await port.close(); } catch (e) {}
      }
      btnAbortIntegration.style.display = "none";
      isIntegrating = false;
      integrationProgressContainer.style.display = "none";
      repairProgressContainer.style.display = "none";
      
      setIntegrationCheckState(intChkBoot,  intValBoot,  "fail", "ABORTED");
      setIntegrationCheckState(intChkGprs,  intValGprs,  "fail", "ABORTED");
      setIntegrationCheckState(intChkBatt,  intValBatt,  "fail", "ABORTED");
      setIntegrationCheckState(intChkSolar, intValSolar, "fail", "ABORTED");
      setIntegrationCheckState(intChkSync,  intValSync,  "fail", "ABORTED");
      
      checkPreconditions();
    });

    // --- Start Diagnostics Without Reflashing ---
    btnStartDiag.addEventListener("click", async () => {
      hideOperatorPrompt();
      if (!cancelReadLoop) {
        await stopDiagnostics();
      } else {
        if (!isConnected) {
          logToConsole("Error: Board is not connected. Please connect the board first.", "error");
          return;
        }
        await runDiagnosticsOnly();
      }
    });

    async function runDiagnosticsOnly() {
      logToConsole("--- STARTING LIVE DIAGNOSTICS (no reflash) ---", "info");
      btnStartDiag.disabled = true;
      
      // Step 1: If esptool transport is still holding the port, release it
      if (transport) {
        logToConsole("Releasing esptool transport lock...", "normal");
        try { await transport.disconnect(); } catch (e) {}
        transport = null;
        esploader = null;
      }

      // Step 2: Close port if it is open
      if (port) {
        try {
          cancelReadLoop = true;
          if (activeReader) {
            try { await activeReader.cancel(); } catch (e) {}
          }
          await new Promise(r => setTimeout(r, 150));
          try { await port.close(); } catch (e) {}
        } catch (e) {}
      }

      // Step 3: Reopen port cleanly at 115200 for serial monitoring
      logToConsole("Opening port at 115200 baud for diagnostics...", "normal");
      try {
        await port.open({ baudRate: 115200 });
      } catch (err) {
        logToConsole(`Could not open port: ${err.message}`, "error");
        btnStartDiag.disabled = false;
        return;
      }

      // Step 4: Send reset pulse — RTS high (EN low) then RTS low (EN released)
      // This forces the ESP32 to reboot so we capture all QC serial output from the start
      logToConsole("Sending reset pulse to board...", "normal");
      try {
        await port.setSignals({ dataTerminalReady: false, requestToSend: true });  // EN low → hold reset
        await new Promise(r => setTimeout(r, 200));
        await port.setSignals({ dataTerminalReady: false, requestToSend: false }); // EN high → boot
        logToConsole("Board reset. Waiting for QC firmware to boot...", "normal");
      } catch (e) {
        logToConsole("Reset signal failed (may not affect operation): " + e.message, "warn");
      }

      // Step 5: Wait for board to boot, then start reading serial output
      setTimeout(() => {
        cancelReadLoop = false;
        startLiveDiagnostics();
      }, 2500);
    }

    async function ensureEsploaderConnected() {
      if (esploader) return;
      
      logToConsole("Serial port in diagnostics mode. Re-connecting to board in bootloader mode...", "normal");
      
      // Step 1: Stop the diagnostic read loop gracefully
      cancelReadLoop = true;
      if (activeReader) {
        try { await activeReader.cancel(); } catch (e) {}
      }
      await new Promise(r => setTimeout(r, 200));
      
      // Step 2: Close the serial port
      if (port) {
        try { await port.close(); } catch (e) {}
      }
      await new Promise(r => setTimeout(r, 150));
      
      // Step 3: Re-open the port in esptool mode
      if (!port) {
        throw new Error("Serial port reference lost. Please refresh and reconnect.");
      }
      transport = new Transport(port, true);
      const terminal = {
        writeLine: (data) => logToConsole(data),
        write:     (data) => logToConsole(data),
        clean:     ()     => {}
      };
      esploader = new ESPLoader({ transport, baudrate: 115200, terminal });
      logToConsole("Connecting to chip...", "normal");
      await esploader.main();
    }

    async function runFactoryFlashingFlow() {
      logToConsole("--- OPTION 1: FACTORY FLASHING SEQUENCE START ---", "info");
      await ensureEsploaderConnected();
      
      // Check files for auto-detected size
      const partKey = `partitions_${currentChipSize}mb.bin`;
      const testKey = `qc_test_${currentChipSize}mb.bin`;
      
      if (!fileVault[partKey] || !fileVault[testKey]) {
        throw new Error(`Missing required flash-size configurations for ${currentChipSize}MB! Load '${partKey}' and '${testKey}' in Vault first.`);
      }

      if (chkEraseFlash.checked) {
        logToConsole("Erasing entire flash memory chip...", "warning");
        await esploader.eraseFlash();
        logToConsole("Flash erased successfully. Writing binaries...", "info");
      } else {
        logToConsole("Bypassing full chip erase (preserving SPIFFS/filesystems). Writing binaries directly...", "info");
      }

      // Build flash payload array
      const flashArray = [
        { data: fileVault["bootloader.bin"], address: 0x1000 },
        { data: fileVault[partKey], address: 0x8000 },
        { data: fileVault["boot_app0.bin"], address: 0xe000 },
        { data: fileVault[testKey], address: 0x10000 }
      ];

      logToConsole("Flashing QC Test environment binaries...", "normal");
      
      await esploader.writeFlash({
        fileArray: flashArray,
        flashSize: "keep",
        compress: true,
        reportProgress: (fileIndex, writtenBytes, totalBytes) => {
          if (totalBytes > 0) {
            const percentage = Math.round((writtenBytes / totalBytes) * 100);
            progressBar.style.width = `${percentage}%`;
          }
        }
      });
      
      progressBar.style.width = "100%";
      logToConsole("Flashing complete! Resetting board...", "info");
      
      // Perform hard reset
      await esploader.after("hard_reset");
      
      // Release flasher port locks and trigger diagnostic logging read-loop
      logToConsole("Releasing flasher connection for diagnostics...", "normal");
      if (transport) {
        try {
          await transport.disconnect();
        } catch (e) {}
      }
      transport = null;
      esploader = null;

      // Reopen port directly for serial reading
      logToConsole("Opening serial diagnostics channel at 115200 baud...", "normal");
      try {
        await port.open({ baudRate: 115200 });
        // Send a clean reset pulse to reboot the ESP32
        logToConsole("Sending reset pulse to board...", "normal");
        await port.setSignals({ dataTerminalReady: false, requestToSend: true });   // EN low (hold reset)
        await new Promise(r => setTimeout(r, 200));
        await port.setSignals({ dataTerminalReady: false, requestToSend: false });  // EN high (release reset)
      } catch (err) {
        logToConsole(`Failed to open diagnostics channel: ${err.message}`, "error");
        return;
      }

      cancelReadLoop = false;
      // Start serial monitor read loop
      setTimeout(startLiveDiagnostics, 500);
    }

    async function runUpgradeFlashingFlow() {
      logToConsole("--- OPTION 2: APPLICATION UPGRADE FLOW START ---", "info");
      await ensureEsploaderConnected();
      
      const appKey = `production_${currentChipSize}mb.bin`;
      if (!fileVault[appKey]) {
        throw new Error(`Missing production binary for ${currentChipSize}MB. Load '${appKey}' in Vault first.`);
      }

      logToConsole(`Writing application updates to offset 0x10000...`, "normal");
      await esploader.writeFlash({
        fileArray: [{ data: fileVault[appKey], address: 0x10000 }],
        flashSize: "keep",
        compress: true,
        reportProgress: (fileIndex, writtenBytes, totalBytes) => {
          if (totalBytes > 0) {
            const percentage = Math.round((writtenBytes / totalBytes) * 100);
            progressBar.style.width = `${percentage}%`;
          }
        }
      });
      
      progressBar.style.width = "100%";
      logToConsole("Application updated! Performing hard reset...", "info");
      await esploader.after("hard_reset");
      
      // Release flasher port locks and trigger diagnostic logging read-loop
      logToConsole("Releasing flasher connection...", "normal");
      if (transport) {
        try {
          await transport.disconnect();
        } catch (e) {}
      }
      transport = null;
      esploader = null;

      logToConsole("Opening serial diagnostics channel at 115200 baud...", "normal");
      try {
        await port.open({ baudRate: 115200 });
        // Send a clean reset pulse to reboot the ESP32
        logToConsole("Sending reset pulse to board...", "normal");
        await port.setSignals({ dataTerminalReady: false, requestToSend: true });   // EN low (hold reset)
        await new Promise(r => setTimeout(r, 200));
        await port.setSignals({ dataTerminalReady: false, requestToSend: false });  // EN high (release reset)
      } catch (err) {
        logToConsole(`Failed to open diagnostics channel: ${err.message}`, "error");
        return;
      }

      cancelReadLoop = false;
      // Start serial monitor read loop
      setTimeout(startLiveDiagnostics, 500);
    }

    async function runGprsDebugFlashFlow() {
      logToConsole("--- OPTION 3: GPRS DEBUG FLASH START ---", "info");
      await ensureEsploaderConnected();
      logToConsole("⚠ This flashes Production firmware. Serial output is RAW — watch for GPRS registration manually.", "warn");

      const appKey = `production_${currentChipSize}mb.bin`;
      if (!fileVault[appKey]) {
        throw new Error(`Missing production binary for ${currentChipSize}MB. Load '${appKey}' in Vault first.`);
      }

      logToConsole(`Writing production firmware to offset 0x10000 (${currentChipSize}MB flash)...`, "normal");
      await esploader.writeFlash({
        fileArray: [{ data: fileVault[appKey], address: 0x10000 }],
        flashSize: "keep",
        compress: true,
        reportProgress: (fileIndex, writtenBytes, totalBytes) => {
          if (totalBytes > 0) {
            const percentage = Math.round((writtenBytes / totalBytes) * 100);
            progressBar.style.width = `${percentage}%`;
          }
        }
      });

      progressBar.style.width = "100%";
      logToConsole("Production firmware written. Resetting board...", "info");
      await esploader.after("hard_reset");

      // Release flasher
      logToConsole("Releasing flasher connection for raw serial monitor...", "normal");
      if (transport) {
        try { await transport.disconnect(); } catch (e) {}
      }
      transport = null;
      esploader = null;

      logToConsole("Opening raw serial monitor at 115200 baud...", "normal");
      logToConsole("👁 Watch for GPRS registration below. Look for 'REG OK', 'GSM:Home', 'LTE:Home' or signal lines.", "info");
      try {
        await port.open({ baudRate: 115200 });
        await port.setSignals({ dataTerminalReady: false, requestToSend: true });
        await new Promise(r => setTimeout(r, 200));
        await port.setSignals({ dataTerminalReady: false, requestToSend: false });
      } catch (err) {
        logToConsole(`Failed to open serial monitor: ${err.message}`, "error");
        return;
      }

      cancelReadLoop = false;
      logToConsole("📡 Raw serial stream active. Press Stop / disconnect to end.", "info");
      // Reuse the live diagnostics reader — it streams all lines to console
      setTimeout(startLiveDiagnostics, 500);
    }

    // Triggered after QC passes to flash the final app
    btnProceedApp.addEventListener("click", async () => {
      btnProceedApp.style.display = "none";
      progressContainer.style.display = "block";
      progressBar.style.width = "0%";
      
      try {
        await runUpgradeFlashingFlow();
        
        logToConsole("✨ FACTORY JIG SEQUENCE COMPLETED SUCCESSFULLY!", "info");
        progressContainer.style.display = "none";
        
        // Sync final "FLASHED" status back to Google Sheet
        logToConsole("📊 Syncing FLASHED status to Google Sheet...", "info");
        const synced = await autoSyncToGoogleSheets("PASS + APP FLASHED");
        if (synced) {
          logToConsole("✓ Google Sheet updated — unit fully commissioned.", "info");
        } else {
          logToConsole("⚠ Google Sheet not updated (Sync skipped or failed).", "warning");
        }
        
      } catch (err) {
        logToConsole(`Final app flash failed: ${err.message}`, "error");
        btnProceedApp.style.display = "block";
        progressContainer.style.display = "none";
      }
    });

    // --- Log Downloader Exporter ---
    btnExportLog.addEventListener("click", () => {
      const mac = metaMac.textContent.replace(/:/g, "-");
      const timestamp = new Date().toISOString().replace(/:/g, "-").substring(0, 19);
      const filename = `QC_REPORT_${mac}_${timestamp}.txt`;
      
      const tester = document.getElementById("testerName")?.value.trim() || "--";
      const mode = document.getElementById("auditMode")?.value || "--";
      const profile = document.getElementById("deviceProfile")?.value || "TRG";
      
      let logContent = `==================================================\n`;
      logContent += `         SPATIKA FACTORY QC TESTING REPORT         \n`;
      logContent += `==================================================\n`;
      logContent += `Timestamp:     ${new Date().toLocaleString()}\n`;
      logContent += `Tester Name:   ${tester}\n`;
      logContent += `Audit Mode:    ${mode}\n`;
      logContent += `Device Profile: ${profile}\n`;
      logContent += `Test Duration:  ${testDuration}s\n`;
      logContent += `Firmware Ver:  ${metaFwVersion.textContent}\n`;
      logContent += `ESP32 MAC:     ${metaMac.textContent}\n`;
      logContent += `GPRS IMEI:     ${metaImei.textContent}\n`;
      logContent += `SIM CCID:      ${metaCcid.textContent}\n`;
      logContent += `Nuvoton LCD:   ${metaNuvoton.textContent}\n`;
      logContent += `QC Verdict:    ${lastVerdict} (${profile})\n`;
      logContent += `--------------------------------------------------\n`;
      logContent += `CHECKLIST RESULTS:\n`;
      logContent += `- SPIFFS:       ${valSpiffs.textContent}\n`;
      logContent += `- SD Card:      ${valSd.textContent}\n`;
      logContent += `- RTC Clock:    ${valRtc.textContent}\n`;
      logContent += `- WiFi Scanner: ${valWifi.textContent}\n`;
      logContent += `- Env Sensor:   ${valSensor.textContent}\n`;
      logContent += `- Wind Speed:   ${valWindSpd.textContent}\n`;
      logContent += `- Wind Direct:  ${valWindDir.textContent}\n`;
      logContent += `- Battery Volt: ${valBatt.textContent}\n`;
      logContent += `- Sys 3.3V Rail: ${valSys3v3.textContent}\n`;
      logContent += `- Solar Input:  ${valSolar.textContent}\n`;
      logContent += `- Cellular:     ${valGprs.textContent}\n`;
      logContent += `- Nuvoton LCD:  ${valLcd.textContent}\n`;
      logContent += `- Keypad:       ${valKeypad.textContent}\n`;
      logContent += `- Rainfall RF:  ${valRf.textContent}\n`;
      logContent += `- Sleep Wakeup: ${valSleep.textContent}\n`;
      logContent += `==================================================\n\n`;
      logContent += `DETAILED LOG CONSOLE:\n`;
      logContent += rawSerialLog;
      
      const blob = new Blob([logContent], { type: "text/plain;charset=utf-8" });
      const url = URL.createObjectURL(blob);
      const a = document.createElement("a");
      a.href = url;
      a.download = filename;
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(url);
      
      logToConsole(`Exported QC report file: ${filename}`, "info");
    });
  