#!/bin/bash

# Spatika JIG Build and Packaging Script
# Automates the compilation of:
#   1. QC Self-Test Firmware (4MB, 8MB, 16MB)
#   2. Production Application (4MB, 8MB, 16MB)
# And bundles them into the self-contained WEB_FLASH_FILES/ directory.

# Ensure we run from the TEST_JIG directory or workspace root dynamically
WORKSPACE_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TEST_JIG_DIR="$WORKSPACE_ROOT/TEST_JIG"
OUT_DIR="$TEST_JIG_DIR/WEB_FLASH_FILES"
ARDUINO_CLI="/usr/local/bin/arduino-cli"

echo "=================================================="
echo "          SPATIKA TEST JIG BUILD SYSTEM           "
echo "=================================================="
echo "Workspace Root: $WORKSPACE_ROOT"
echo "Output Directory: $OUT_DIR"
echo "=================================================="

# Check arduino-cli
if [ ! -f "$ARDUINO_CLI" ]; then
    echo "❌ Error: arduino-cli not found at $ARDUINO_CLI"
    exit 1
fi

QC_ONLY=false
BUILD_8MB=true
BUILD_16MB=true
CONFIGS_VAL="KSNDMC_TRG"
UI_VAL="both"

while [[ $# -gt 0 ]]; do
  case $1 in
    --qc-only|qc-only)
      QC_ONLY=true
      shift
      ;;
    --8mb-only|--8mb|8mb-only)
      BUILD_8MB=true
      BUILD_16MB=false
      shift
      ;;
    --16mb-only|--16mb|16mb-only)
      BUILD_8MB=false
      BUILD_16MB=true
      shift
      ;;
    --configs)
      CONFIGS_VAL="$2"
      shift 2
      ;;
    --ui)
      UI_VAL="$2"
      shift 2
      ;;
    *)
      # Ignore or skip unknown args
      shift
      ;;
  esac
done


if [ "$QC_ONLY" = true ]; then
    echo "⚡ QC-Only flag detected. Skipping production application and config builds."
fi
if [ "$BUILD_8MB" = true ] && [ "$BUILD_16MB" = false ]; then
    echo "⚡ 8MB-Only flag detected. Skipping 16MB builds."
fi
if [ "$BUILD_16MB" = true ] && [ "$BUILD_8MB" = false ]; then
    echo "⚡ 16MB-Only flag detected. Skipping 8MB builds."
fi

# Create target directories
mkdir -p "$OUT_DIR"

# Temporary build directory paths
QC_BUILD_BASE="/tmp/qc_jig_build"
APP_BUILD_BASE="/tmp/app_jig_build"
rm -rf "$QC_BUILD_BASE" "$APP_BUILD_BASE"

# --- 1. COMPILE QC TEST FIRMWARE (4MB, 8MB, 16MB) ---
echo "--- Compiling QC Self-Test Firmware ---"

# Compile 4MB QC Test - skipped (not required)
# if [ "$EIGHT_MB_ONLY" = false ]; then
#   echo "→ Building 4MB QC Test..."
#   cp "$WORKSPACE_ROOT/partitions_4mb.csv" "$TEST_JIG_DIR/qc_test/partitions.csv"
#   $ARDUINO_CLI compile \
#       --fqbn "esp32:esp32:esp32:FlashSize=4M,PartitionScheme=custom" \
#       --build-property "build.partitions=custom" \
#       --build-property "upload.maximum_size=1310720" \
#       --build-path "$QC_BUILD_BASE/4mb" \
#       "$TEST_JIG_DIR/qc_test/qc_test.ino"
# 
#   if [ $? -ne 0 ]; then
#       echo "❌ QC 4MB compile failed!"
#       rm -f "$TEST_JIG_DIR/qc_test/partitions.csv"
#       exit 1
#   fi
# fi

# Compile 8MB QC Test
if [ "$BUILD_8MB" = true ]; then
  echo "→ Building 8MB QC Test..."
  cp "$WORKSPACE_ROOT/partitions.csv" "$TEST_JIG_DIR/qc_test/partitions.csv"
  $ARDUINO_CLI compile \
      --fqbn "esp32:esp32:esp32:FlashSize=8M,FlashMode=dio,PartitionScheme=custom" \
      --build-property "build.partitions=custom" \
      --build-property "build.custom_partitions=$TEST_JIG_DIR/qc_test/partitions.csv" \
      --build-property "upload.maximum_size=1769472" \
      --build-path "$QC_BUILD_BASE/8mb" \
      "$TEST_JIG_DIR/qc_test/qc_test.ino"

  if [ $? -ne 0 ]; then
      echo "❌ QC 8MB compile failed!"
      rm -f "$TEST_JIG_DIR/qc_test/partitions.csv"
      exit 1
  fi
fi

# Compile 16MB QC Test
if [ "$BUILD_16MB" = true ]; then
  echo "→ Building 16MB QC Test..."
  cp "$WORKSPACE_ROOT/partitions_16mb.csv" "$TEST_JIG_DIR/qc_test/partitions.csv"
  $ARDUINO_CLI compile \
      --fqbn "esp32:esp32:esp32:FlashSize=16M,FlashMode=dio,FlashFreq=80,PartitionScheme=custom" \
      --build-property "build.partitions=custom" \
      --build-property "build.custom_partitions=$TEST_JIG_DIR/qc_test/partitions.csv" \
      --build-property "upload.maximum_size=2097152" \
      --build-path "$QC_BUILD_BASE/16mb" \
      "$TEST_JIG_DIR/qc_test/qc_test.ino"

  if [ $? -ne 0 ]; then
      echo "❌ QC 16MB compile failed!"
      rm -f "$TEST_JIG_DIR/qc_test/partitions.csv"
      exit 1
  fi
fi

# Clean up QC partitions
rm -f "$TEST_JIG_DIR/qc_test/partitions.csv"


# --- 2. COMPILE PRODUCTION APP ---
if [ "$QC_ONLY" = false ]; then
  echo "--- Compiling Production Application ---"
  
  if [ "$BUILD_8MB" = true ]; then
    # Build 8MB Production App
    echo "→ Building 8MB Production Application..."
    $ARDUINO_CLI compile \
        --fqbn "esp32:esp32:esp32:FlashSize=8M,FlashMode=dio,PartitionScheme=custom" \
        --build-property "build.partitions=custom" \
        --build-property "build.custom_partitions=$WORKSPACE_ROOT/partitions.csv" \
        --build-property "upload.maximum_size=1769472" \
        --build-path "$APP_BUILD_BASE/8mb" \
        "$WORKSPACE_ROOT"
    
    if [ $? -ne 0 ]; then
        echo "❌ App 8MB compile failed!"
        exit 1;
    fi
  fi

  if [ "$BUILD_16MB" = true ]; then
    # Build 16MB Production App
    echo "→ Building 16MB Production Application..."
    $ARDUINO_CLI compile \
        --fqbn "esp32:esp32:esp32:FlashSize=16M,FlashMode=dio,FlashFreq=80,PartitionScheme=custom" \
        --build-property "build.partitions=custom" \
        --build-property "build.custom_partitions=$WORKSPACE_ROOT/partitions_16mb.csv" \
        --build-property "upload.maximum_size=2097152" \
        --build-path "$APP_BUILD_BASE/16mb" \
        "$WORKSPACE_ROOT"
    if [ $? -ne 0 ]; then
        echo "❌ App 16MB compile failed!"
        exit 1
    fi
  fi
else
  echo "--- Skipping Production Application Compilation ---"
fi


# --- 3. GATHER AND BUNDLE TARGET ARTIFACTS ---
echo "--- Gathering and Bundling Artifacts ---"

# Copy common boot sector files
if [ "$BUILD_8MB" = true ]; then
  cp "$QC_BUILD_BASE/8mb/qc_test.ino.bootloader.bin" "$OUT_DIR/bootloader_8mb.bin"
  cp "$QC_BUILD_BASE/8mb/qc_test.ino.bootloader.bin" "$OUT_DIR/bootloader.bin"
fi
if [ "$BUILD_16MB" = true ]; then
  cp "$QC_BUILD_BASE/16mb/qc_test.ino.bootloader.bin" "$OUT_DIR/bootloader_16mb.bin"
  if [ "$BUILD_8MB" = false ]; then
    cp "$QC_BUILD_BASE/16mb/qc_test.ino.bootloader.bin" "$OUT_DIR/bootloader.bin"
  fi
fi
cp "$WORKSPACE_ROOT/flash_files/boot_app0.bin" "$OUT_DIR/boot_app0.bin"

# Copy compiled partition binaries
if [ "$BUILD_8MB" = true ]; then
  cp "$QC_BUILD_BASE/8mb/qc_test.ino.partitions.bin" "$OUT_DIR/partitions_8mb.bin"
fi
if [ "$BUILD_16MB" = true ]; then
  cp "$QC_BUILD_BASE/16mb/qc_test.ino.partitions.bin" "$OUT_DIR/partitions_16mb.bin"
fi

# Copy QC self-test app binaries
if [ "$BUILD_8MB" = true ]; then
  cp "$QC_BUILD_BASE/8mb/qc_test.ino.bin" "$OUT_DIR/qc_test_8mb.bin"
fi
if [ "$BUILD_16MB" = true ]; then
  cp "$QC_BUILD_BASE/16mb/qc_test.ino.bin" "$OUT_DIR/qc_test_16mb.bin"
fi

# Copy Production app binaries and verify version
if [ "$QC_ONLY" = false ]; then
  EXPECTED_VER=$(grep '#define FIRMWARE_VERSION' "$WORKSPACE_ROOT/user_config.h" | sed 's/.*"\(.*\)".*/\1/')
  if [ "$BUILD_8MB" = true ]; then
    cp "$APP_BUILD_BASE/8mb/AIO9_5.0.bin" "$OUT_DIR/production_8mb.bin" 2>/dev/null || cp "$APP_BUILD_BASE/8mb/AIO9_5.0.ino.bin" "$OUT_DIR/production_8mb.bin"
    BIN_VER=$(strings "$OUT_DIR/production_8mb.bin" 2>/dev/null | grep -E '^[0-9]+\.[0-9]+$' | head -1)
    if [ "$BIN_VER" != "$EXPECTED_VER" ]; then
      echo "❌ FATAL: production_8mb.bin has version '$BIN_VER' inside but user_config.h says '$EXPECTED_VER'!"
      exit 1
    fi
    echo "✅ production_8mb.bin verified: v$BIN_VER"
  fi
  if [ "$BUILD_16MB" = true ]; then
    cp "$APP_BUILD_BASE/16mb/AIO9_5.0.bin" "$OUT_DIR/production_16mb.bin" 2>/dev/null || cp "$APP_BUILD_BASE/16mb/AIO9_5.0.ino.bin" "$OUT_DIR/production_16mb.bin"
    BIN_VER=$(strings "$OUT_DIR/production_16mb.bin" 2>/dev/null | grep -E '^[0-9]+\.[0-9]+$' | head -1)
    if [ "$BIN_VER" != "$EXPECTED_VER" ]; then
      echo "❌ FATAL: production_16mb.bin has version '$BIN_VER' inside but user_config.h says '$EXPECTED_VER'!"
      exit 1
    fi
    echo "✅ production_16mb.bin verified: v$BIN_VER"
  fi
else
  echo "→ QC-Only mode: retaining existing production application binaries."
fi

# Verify the HTML structure and unbalanced diffs before packaging
echo "--------------------------------------------------"
echo "          VERIFYING FACTORY_TOOL.HTML            "
echo "--------------------------------------------------"
# Change to workspace root for Python script files search to work
cd "$WORKSPACE_ROOT"
python3 "$TEST_JIG_DIR/verify_html.py"
HTML_VERIFY_STATUS=$?
if [ $HTML_VERIFY_STATUS -ne 0 ]; then
    echo "❌ Error: HTML structure verification failed!"
    exit 1
fi

python3 "$TEST_JIG_DIR/find_unbalanced_diff.py"
HTML_DIFF_STATUS=$?
if [ $HTML_DIFF_STATUS -ne 0 ]; then
    echo "❌ Error: HTML unbalanced braces/diff markers check failed!"
    exit 1
fi
echo "✅ HTML verification passed successfully!"

# Copy the HTML serial portal dashboard (force physical file copy, not symlink)
rm -f "$OUT_DIR/factory_tool.html"
cp -f "$TEST_JIG_DIR/factory_tool.html" "$OUT_DIR/factory_tool.html"

# Extract Firmware Version from user_config.h and write to version.txt
FW_VER=$(grep '#define FIRMWARE_VERSION' "$WORKSPACE_ROOT/user_config.h" | sed 's/.*"\(.*\)".*/\1/')
echo "$FW_VER" > "$OUT_DIR/version.txt"
echo "→ Packaged version.txt: $FW_VER"

# Clean up temp
rm -rf "$QC_BUILD_BASE" "$APP_BUILD_BASE"


# --- 4. BUILD ALL NAMED CONFIGS (email-ZIP structure) ---
if [ "$QC_ONLY" = false ]; then
  echo ""
  echo "=================================================="
  echo "  STEP 4: Building Named Release Configurations   "
  echo "=================================================="

  # Read current user_config.h to determine if production_Xmb.bin already matches target config
  CURRENT_UNIT=$(grep '#define UNIT_CFG' "$WORKSPACE_ROOT/user_config.h" | sed 's/.*"\(.*\)".*/\1/')
  CURRENT_UI=$(grep '#define USE_NUVOTON_UI' "$WORKSPACE_ROOT/user_config.h" | awk '{print $3}')
  CURRENT_UI_SUFFIX=$([ "$CURRENT_UI" = "1" ] && echo "NUV" || echo "MAT")
  CURRENT_UI_NAME=$([ "$CURRENT_UI" = "1" ] && echo "nuvoton" || echo "matrix")

  echo "→ Current config in user_config.h: UNIT=${CURRENT_UNIT}, UI=${CURRENT_UI_SUFFIX}"

  # Check if the requested --configs and --ui match what was already compiled in Phase 1
  SINGLE_CONFIG_MATCH=false
  if [ "$CONFIGS_VAL" = "$CURRENT_UNIT" ] && [ "$UI_VAL" = "$CURRENT_UI_NAME" ] || \
     [ "$CONFIGS_VAL" = "$CURRENT_UNIT" ] && [ "$UI_VAL" = "nuvoton" ] && [ "$CURRENT_UI" = "1" ] || \
     [ "$CONFIGS_VAL" = "$CURRENT_UNIT" ] && [ "$UI_VAL" = "matrix" ] && [ "$CURRENT_UI" = "0" ]; then
    SINGLE_CONFIG_MATCH=true
  fi

  if [ "$SINGLE_CONFIG_MATCH" = true ]; then
    echo "✓ Config '${CONFIGS_VAL} (${CURRENT_UI_SUFFIX})' matches already-compiled production binary."
    echo "  → Reusing production_Xmb.bin directly (skipping redundant recompile)."
    echo ""

    # Generate metadata using Python from current user_config.h
    FLASH_ARG="8mb"
    if [ "$BUILD_8MB" = false ] && [ "$BUILD_16MB" = true ]; then
      FLASH_ARG="16mb"
    fi

    cd "$WORKSPACE_ROOT"
    python3 - <<PYEOF
import json, re, sys, os, shutil
from datetime import datetime
from pathlib import Path

ws = Path("$WORKSPACE_ROOT")
out_dir = Path("$OUT_DIR")
fw_ver = "$FW_VER"
unit = "$CURRENT_UNIT"
ui_val = $CURRENT_UI
current_ui_suffix = "$CURRENT_UI_SUFFIX"
build_8mb = "$BUILD_8MB" == "true"
build_16mb = "$BUILD_16MB" == "true"

cfg_h = ws / "user_config.h"
raw = cfg_h.read_text()
joined = re.sub(r'\\\s*\n\s*', ' ', raw)

def get_int(name):
    m = re.search(rf'#define {name}\s+(\d+)', joined)
    return int(m.group(1)) if m else None

def get_float(name):
    m = re.search(rf'#define {name}\s+([0-9.]+)', joined)
    return float(m.group(1)) if m else None

health_val = get_int('TEST_HEALTH_DEFAULT')
health_en_val = get_int('ENABLE_HEALTH_REPORT')
health_freq_map = {0: 'Daily (11am)', 1: 'Every 15 mins', 2: 'Disabled'}

# Full version string
system_val = get_int('SYSTEM') or 0
if unit == "KSNDMC_TRG":
    base_ver = f"TRG9-DMC-{fw_ver}"
elif unit == "BIHAR_TRG":
    base_ver = f"TRG9-BIH-{fw_ver}"
elif unit == "KSNDMC_TWS":
    base_ver = f"TWS9-DMC-{fw_ver}"
elif unit == "KSNDMC_ADDON":
    base_ver = f"TWSRF9-DMC-{fw_ver}"
elif unit == "SPATIKA_GEN":
    base_ver = f"TWSRF9-GEN-{fw_ver}"
else:
    base_ver = f"UNK-{fw_ver}"
ui_suffix = "-N" if ui_val == 1 else "-M"
full_version = base_ver + ui_suffix

system_type = ['TRG','TWS','TWS-RF'][system_val] if system_val in [0,1,2] else str(system_val)

sizes_to_build = []
if build_8mb:
    sizes_to_build.append(("8mb", out_dir / "production_8mb.bin"))
if build_16mb:
    sizes_to_build.append(("16mb", out_dir / "production_16mb.bin"))

for flash_size, src_bin in sizes_to_build:
    if not src_bin.exists():
        print(f"  ⚠ {src_bin.name} not found, skipping {flash_size}")
        continue

    config_name = f"{unit}_{current_ui_suffix}_{flash_size}"
    dest = out_dir / config_name
    dest.mkdir(parents=True, exist_ok=True)

    shutil.copy(src_bin, dest / "firmware.bin")
    (dest / "fw_version.txt").write_text(full_version)

    bin_size = src_bin.stat().st_size
    metadata = {
        "config": unit,
        "unit_cfg": unit,
        "system_type": system_type,
        "flash_size": flash_size,
        "full_version": full_version,
        "firmware_version": fw_ver,
        "binary_size_bytes": bin_size,
        "build_timestamp": datetime.now().isoformat(timespec='seconds'),
        "debug": bool(get_int('DEBUG')),
        "enable_webserver": bool(get_int('ENABLE_WEBSERVER')),
        "use_nuvoton_ui": bool(ui_val),
        "enable_health_report": bool(health_en_val) if health_en_val is not None else False,
        "health_report_freq": health_freq_map.get(health_val, 'Unknown') if health_val is not None else 'Unknown',
        "rf_resolution_mm": get_float('DEFAULT_RF_RESOLUTION'),
        "wind_teeth_count": get_float('WIND_TEETH_COUNT'),
        "enable_pressure_sensor": bool(get_int('ENABLE_PRESSURE_SENSOR')),
        "enable_calib_test": bool(get_int('ENABLE_CALIB_TEST')),
    }
    (dest / "metadata.json").write_text(json.dumps(metadata, indent=2))
    size_mb = bin_size / (1024*1024)
    print(f"  ✓ {config_name} ({size_mb:.2f} MB) — {full_version} (reused from production binary)")

print("→ Named configs packaged (fast reuse mode).")
PYEOF
    STEP4_STATUS=$?
    if [ $STEP4_STATUS -ne 0 ]; then
      echo "❌ Fast-reuse packaging failed."
    fi

  else
    # Config doesn't match — need build_all_configs.py to compile separately
    if [ -f "$WORKSPACE_ROOT/build_all_configs.py" ]; then
      FLASH_ARG="8mb"
      if [ "$BUILD_8MB" = false ] && [ "$BUILD_16MB" = true ]; then
          FLASH_ARG="16mb"
      fi

      echo "→ Config '${CONFIGS_VAL}' differs from compiled '${CURRENT_UNIT}' — running build_all_configs.py..."
      cd "$WORKSPACE_ROOT"
      python3 build_all_configs.py --flash $FLASH_ARG --configs "$CONFIGS_VAL" --ui "$UI_VAL"
      BUILD_STATUS=$?

      if [ $BUILD_STATUS -ne 0 ]; then
          echo "❌ build_all_configs.py failed. Named configs will not be included."
      else
          echo ""
          echo "--- Copying named config subdirs to WEB_FLASH_FILES ---"
          BUILDS_DIR="$WORKSPACE_ROOT/builds"

          for config_dir in "$BUILDS_DIR"/*/; do
              config_name=$(basename "$config_dir")
              src_bin="$config_dir/firmware.bin"
              src_ver="$config_dir/fw_version.txt"
              src_meta="$config_dir/metadata.json"

              if [ ! -f "$src_bin" ]; then
                  cache_bin=$(find "$WORKSPACE_ROOT/build/config_${config_name}" -name "*.ino.bin" 2>/dev/null | head -1)
                  if [ -n "$cache_bin" ]; then
                      echo "  ⚠ builds/$config_name/firmware.bin missing — using build cache: $cache_bin"
                      src_bin="$cache_bin"
                  fi
              fi

              if [ -f "$src_bin" ]; then
                  BIN_VER=$(strings "$src_bin" 2>/dev/null | grep -E '^[0-9]+\.[0-9]+$' | head -1)
                  if [ "$BIN_VER" != "$FW_VER" ]; then
                      echo "  ❌ SKIP $config_name: firmware.bin has version '$BIN_VER' inside, expected '$FW_VER'!"
                      echo "     → This binary is STALE. Run a full build to regenerate."
                      continue
                  fi

                  if [ "$FLASH_ARG" == "8mb" ]; then
                      dest8="$OUT_DIR/$config_name"
                      mkdir -p "$dest8"
                      cp "$src_bin" "$dest8/firmware.bin"
                      [ -f "$src_ver"  ] && cp "$src_ver"  "$dest8/fw_version.txt"
                      [ -f "$src_meta" ] && cp "$src_meta" "$dest8/metadata.json"
                      echo "  ✓ $config_name — v$BIN_VER verified inside binary"

                      if [ "$BUILD_16MB" = true ]; then
                          base_name=$(echo "$config_name" | sed 's/_8mb$//')
                          dest16="$OUT_DIR/${base_name}_16mb"
                          mkdir -p "$dest16"
                          cp "$src_bin" "$dest16/firmware.bin"
                          [ -f "$src_ver" ] && cp "$src_ver" "$dest16/fw_version.txt"
                          if [ -f "$src_meta" ]; then
                               python3 -c "
import json
with open('$src_meta') as f:
    m = json.load(f)
m['flash_size'] = '16mb'
with open('$dest16/metadata.json', 'w') as f:
    json.dump(m, f, indent=2)
"
                          fi
                          SIZE=$(du -sh "$src_bin" | cut -f1)
                          echo "  ✓ ${base_name}_16mb ($SIZE) — v$BIN_VER verified"
                      fi
                  else
                      dest16="$OUT_DIR/$config_name"
                      mkdir -p "$dest16"
                      cp "$src_bin" "$dest16/firmware.bin"
                      [ -f "$src_ver"  ] && cp "$src_ver"  "$dest16/fw_version.txt"
                      [ -f "$src_meta" ] && cp "$src_meta" "$dest16/metadata.json"
                      SIZE=$(du -sh "$src_bin" | cut -f1)
                      echo "  ✓ $config_name ($SIZE) — v$BIN_VER verified inside binary"
                  fi
              fi
          done
          echo "→ Named configs packaged."
      fi
    else
        echo "⚠  build_all_configs.py not found — skipping named configs."
    fi
  fi
else
  echo "--- Skipping Named Release Configurations ---"
fi


echo "=================================================="
echo "✅ SUCCESS: All build files packaged in:"
echo "   $OUT_DIR"
echo "=================================================="
echo ""
echo "Directory structure:"
ls -lh "$OUT_DIR"
echo ""
echo "Named config subdirectories:"
for d in "$OUT_DIR"/*/; do
    [ -d "$d" ] && echo "  📁 $(basename $d)/"
done
echo "=================================================="
