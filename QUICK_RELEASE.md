# Quick Release Guide

## 🚀 How to Create a Release

### **Step 1: Update Version**
Edit `globals.h` and change the version:
```c
#define FIRMWARE_VERSION "5.32"  // Change this
```

### **Step 2: Run Release Script**
```bash
./create_release.sh "Brief summary of changes"
```

**Example:**
```bash
./create_release.sh "Health Report Reliability & GPS Auto-Fix"
```

### **Step 3: Done!**
The script will automatically:
1. ✅ Read version from `globals.h`
2. ✅ Create release notes template (if needed)
3. ✅ Git commit and tag
4. ✅ Build all 6 configurations
5. ✅ Create ZIP archive
6. ✅ Ask to push to GitHub

---

## 📋 What the Script Does

### **Automatic Steps:**
- Extracts version from `globals.h`
- Creates `RELEASE_NOTES_v5.32.md` template (if not exists)
- Commits code: `"Release v5.32: <summary>"`
- Tags: `v5.32`
- Builds all configurations (KSNDMC_TRG, KSNDMC_TWS, etc.)
- Creates ZIP: `AIO9_5.0_v5.32.zip`
- Copies to: `/RELEASE/AIO9_5/v5.32/`

### **Manual Confirmation:**
- Git push (asks for confirmation)

---

## 📦 Output

After running, you'll have:

```
/RELEASE/AIO9_5/
├── v5.32/
│   ├── KSNDMC_TRG/
│   ├── KSNDMC_TWS/
│   ├── BIHAR_TRG/
│   ├── SPATIKA_TRG/
│   ├── KSNDMC_ADDON/
│   ├── SPATIKA_ADDON/
│   ├── flash_files/
│   └── RELEASE_NOTES.md
└── AIO9_5.0_v5.32.zip (4.2 MB)
```

---

## 🎯 Complete Workflow Example

```bash
# 1. Edit version
vim globals.h  # Change to "5.32"

# 2. (Optional) Edit release notes
vim RELEASE_NOTES_v5.32.md  # Add details

# 3. Create release
./create_release.sh "Bug fixes and improvements"

# 4. Confirm git push when prompted
# Press 'y' to push to GitHub

# Done! ✅
```

---

## 🔧 Advanced Usage

### Skip Release Notes Prompt
If release notes already exist, script continues automatically:
```bash
# Create notes first
vim RELEASE_NOTES_v5.32.md

# Then run script (no prompt)
./create_release.sh "Summary"
```

### Manual Git Push
If you skip the git push prompt, push manually later:
```bash
git push origin main --tags
```

---

## ✅ Verification

After release, verify:

1. **Git:** Check tag exists
   ```bash
   git tag -l | grep v5.32
   ```

2. **Binaries:** Check all configs built
   ```bash
   ls -lh /RELEASE/AIO9_5/v5.32/*/firmware.bin
   ```

3. **ZIP:** Verify archive created
   ```bash
   ls -lh /RELEASE/AIO9_5/AIO9_5.0_v5.32.zip
   ```

---

## 🐛 Troubleshooting

### "Could not extract version"
- Check `globals.h` has: `#define FIRMWARE_VERSION "5.32"`
- Ensure quotes are present

### "Build failed"
- Check `arduino-cli` is installed: `brew install arduino-cli`
- Verify ESP32 board is installed

### "ZIP not created"
- Check `build_all_configs.py` completed successfully
- Verify `/RELEASE/AIO9_5/` directory exists

---

## 📞 Need Help?

Just say: **"Create release for v5.32"** and I'll guide you through it! 🎯
