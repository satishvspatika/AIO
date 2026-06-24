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

for arg in "$@"; do
    if [ "$arg" == "--qc-only" ] || [ "$arg" == "qc-only" ]; then
        QC_ONLY=true
    elif [ "$arg" == "--8mb-only" ] || [ "$arg" == "--8mb" ] || [ "$arg" == "8mb-only" ]; then
        BUILD_8MB=true
        BUILD_16MB=false
    elif [ "$arg" == "--16mb-only" ] || [ "$arg" == "--16mb" ] || [ "$arg" == "16mb-only" ]; then
        BUILD_8MB=false
        BUILD_16MB=true
    fi
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
      --fqbn "esp32:esp32:esp32:FlashSize=8M,PartitionScheme=custom" \
      --build-property "build.partitions=custom" \
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
      --fqbn "esp32:esp32:esp32:FlashSize=16M,PartitionScheme=custom" \
      --build-property "build.partitions=custom" \
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
        --fqbn "esp32:esp32:esp32:FlashSize=8M,PartitionScheme=custom" \
        --build-property "build.partitions=custom" \
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
    cp "$WORKSPACE_ROOT/partitions.csv" /tmp/partitions_backup.csv 2>/dev/null || true
    cp "$WORKSPACE_ROOT/partitions_16mb.csv" "$WORKSPACE_ROOT/partitions.csv"
    $ARDUINO_CLI compile \
        --fqbn "esp32:esp32:esp32:FlashSize=16M,PartitionScheme=custom" \
        --build-property "build.partitions=custom" \
        --build-property "upload.maximum_size=2097152" \
        --build-path "$APP_BUILD_BASE/16mb" \
        "$WORKSPACE_ROOT"
    COMPILE_STATUS=$?
    cp /tmp/partitions_backup.csv "$WORKSPACE_ROOT/partitions.csv" 2>/dev/null || true
    if [ $COMPILE_STATUS -ne 0 ]; then
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

# Copy Production app binaries
if [ "$QC_ONLY" = false ]; then
  if [ "$BUILD_8MB" = true ]; then
    cp "$APP_BUILD_BASE/8mb/AIO9_5.0.bin" "$OUT_DIR/production_8mb.bin" 2>/dev/null || cp "$APP_BUILD_BASE/8mb/AIO9_5.0.ino.bin" "$OUT_DIR/production_8mb.bin"
  fi
  if [ "$BUILD_16MB" = true ]; then
    cp "$APP_BUILD_BASE/16mb/AIO9_5.0.bin" "$OUT_DIR/production_16mb.bin" 2>/dev/null || cp "$APP_BUILD_BASE/16mb/AIO9_5.0.ino.bin" "$OUT_DIR/production_16mb.bin"
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

# Copy the HTML serial portal dashboard
cp "$TEST_JIG_DIR/factory_tool.html" "$OUT_DIR/factory_tool.html"

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
  
  if [ -f "$WORKSPACE_ROOT/build_all_configs.py" ]; then
      FLASH_ARG="8mb"
      if [ "$BUILD_8MB" = false ] && [ "$BUILD_16MB" = true ]; then
          FLASH_ARG="16mb"
      fi

      echo "→ Running build_all_configs.py (${FLASH_ARG} targets)..."
      cd "$WORKSPACE_ROOT"
      python3 build_all_configs.py --flash $FLASH_ARG --configs KSNDMC_TRG
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
  
              if [ -f "$src_bin" ]; then
                  if [ "$FLASH_ARG" == "8mb" ]; then
                      # Package 8MB Folder
                      dest8="$OUT_DIR/$config_name"
                      mkdir -p "$dest8"
                      cp "$src_bin" "$dest8/firmware.bin"
                      [ -f "$src_ver"  ] && cp "$src_ver"  "$dest8/fw_version.txt"
                      [ -f "$src_meta" ] && cp "$src_meta" "$dest8/metadata.json"

                      # Package 16MB Folder (reusing the same binaries)
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
                          echo "  ✓ $config_name ($SIZE) and ${base_name}_16mb ($SIZE)"
                      else
                          SIZE=$(du -sh "$src_bin" | cut -f1)
                          echo "  ✓ $config_name ($SIZE)"
                      fi
                  else
                      # FLASH_ARG is 16mb
                      dest16="$OUT_DIR/$config_name"
                      mkdir -p "$dest16"
                      cp "$src_bin" "$dest16/firmware.bin"
                      [ -f "$src_ver"  ] && cp "$src_ver"  "$dest16/fw_version.txt"
                      [ -f "$src_meta" ] && cp "$src_meta" "$dest16/metadata.json"
                      SIZE=$(du -sh "$src_bin" | cut -f1)
                      echo "  ✓ $config_name ($SIZE)"
                  fi
              fi
          done
          echo "→ Named configs packaged."
      fi
  else
      echo "⚠  build_all_configs.py not found — skipping named configs."
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
