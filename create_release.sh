#!/bin/bash
# Automated Release Script for AIO9_5.0
# Usage: ./create_release.sh "Release summary message"
# 
# Prerequisites:
# 1. Update FIRMWARE_VERSION in globals.h
# 2. Run this script
# 3. Script will auto-generate release notes, build, package, and commit

set -e  # Exit on error

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Helper functions
print_header() {
    echo -e "\n${BLUE}========================================${NC}"
    echo -e "${BLUE}  $1${NC}"
    echo -e "${BLUE}========================================${NC}\n"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

# 1. Extract version from globals.h
print_header "Step 1: Reading Version from globals.h"

VERSION=$(grep '#define FIRMWARE_VERSION' globals.h | sed 's/.*"\(.*\)".*/\1/')

if [ -z "$VERSION" ]; then
    print_error "Could not extract version from globals.h"
    exit 1
fi

print_success "Detected version: v$VERSION"

# 2. Get release summary (from argument or prompt)
SUMMARY="$1"
if [ -z "$SUMMARY" ]; then
    echo -e "${YELLOW}Enter release summary (e.g., 'Health Report Reliability & GPS Auto-Fix'):${NC}"
    read -r SUMMARY
fi

if [ -z "$SUMMARY" ]; then
    print_error "Release summary is required"
    exit 1
fi

print_success "Release summary: $SUMMARY"

# 3. Check if release notes exist, create template if not
print_header "Step 2: Checking Release Notes"

RELEASE_NOTES="RELEASE_NOTES_v${VERSION}.md"

if [ -f "$RELEASE_NOTES" ]; then
    print_success "Release notes found: $RELEASE_NOTES"
else
    print_warning "Release notes not found. Creating template..."
    
    cat > "$RELEASE_NOTES" << EOF
# Release Notes: v${VERSION} ($(date +"%b %d, %Y"))

## 🎯 Overview
${SUMMARY}

---

## ✨ New Features

### 1. **Feature Name** 📝
- Description of feature
- Use case

---

## 🔧 Bug Fixes

### 1. **Fix Name** 🔧
- **Problem:** Description
- **Solution:** Description
- **Impact:** Description

---

## 📋 Technical Details

### Modified Files
- \`file1.ino\` - Description
- \`file2.h\` - Description

### Code Size Impact
- **Program:** TBD bytes
- **RAM:** TBD bytes

---

## 🧪 Testing Recommendations

1. ✅ Test feature 1
2. ✅ Test feature 2

---

## 🔄 Upgrade Path

### From v5.3:
- **Direct upgrade** - Flash v${VERSION} firmware
- **No configuration changes** required

---

**v${VERSION} is production-ready!** 🚀
EOF
    
    print_success "Template created: $RELEASE_NOTES"
    print_warning "Please edit $RELEASE_NOTES before continuing"
    echo -e "${YELLOW}Press Enter when ready to continue...${NC}"
    read -r
fi

# 4. Git commit and tag
print_header "Step 3: Git Commit & Tag"

git add .

# Check if there are changes to commit
if git diff --cached --quiet; then
    print_warning "No changes to commit (working tree clean)"
else
    git commit -m "Release v${VERSION}: ${SUMMARY}"
    print_success "Git commit created"
fi

# Create tag (or update if exists)
if git tag -l | grep -q "^v${VERSION}$"; then
    print_warning "Tag v${VERSION} already exists, skipping tag creation"
else
    git tag -a "v${VERSION}" -m "Version ${VERSION}"
    print_success "Git tag created: v${VERSION}"
fi

# 5. Build all configurations
print_header "Step 4: Building All Configurations"

python3 build_all_configs.py

if [ $? -ne 0 ]; then
    print_error "Build failed"
    exit 1
fi

print_success "All configurations built successfully"

# 6. Verify ZIP was created
print_header "Step 5: Verifying Release Package"

RELEASE_DIR="/Users/satishkripavasan/Documents/Arduino/ESP32_NEW_DESIGN/RELEASE/AIO9_5"
ZIP_FILE="${RELEASE_DIR}/AIO9_5.0_v${VERSION}.zip"

if [ -f "$ZIP_FILE" ]; then
    ZIP_SIZE=$(du -h "$ZIP_FILE" | cut -f1)
    print_success "ZIP archive created: AIO9_5.0_v${VERSION}.zip ($ZIP_SIZE)"
else
    print_warning "ZIP file not found at: $ZIP_FILE"
fi

# 7. Git push (with confirmation)
print_header "Step 6: Push to GitHub"

echo -e "${YELLOW}Ready to push to GitHub?${NC}"
echo -e "  - Commit: Release v${VERSION}: ${SUMMARY}"
echo -e "  - Tag: v${VERSION}"
echo -e "\n${YELLOW}Push to origin? (y/n):${NC}"
read -r CONFIRM

if [ "$CONFIRM" = "y" ] || [ "$CONFIRM" = "Y" ]; then
    git push origin main --tags
    print_success "Pushed to GitHub successfully"
else
    print_warning "Skipped git push. Run manually: git push origin main --tags"
fi

# 8. Summary
print_header "Release Complete!"

echo -e "${GREEN}✓ Version: v${VERSION}${NC}"
echo -e "${GREEN}✓ Summary: ${SUMMARY}${NC}"
echo -e "${GREEN}✓ Release Notes: ${RELEASE_NOTES}${NC}"
echo -e "${GREEN}✓ Configurations: 6 built${NC}"
echo -e "${GREEN}✓ Package: ${RELEASE_DIR}/v${VERSION}/${NC}"

if [ -f "$ZIP_FILE" ]; then
    echo -e "${GREEN}✓ ZIP Archive: ${ZIP_FILE} (${ZIP_SIZE})${NC}"
fi

echo -e "\n${BLUE}Next Steps:${NC}"
echo -e "  1. Verify release package: ${RELEASE_DIR}/v${VERSION}/"
echo -e "  2. Test firmware on device"
echo -e "  3. Upload ZIP to GitHub Releases (optional)"

# 9. Send email notification (with confirmation)
print_header "Step 7: Email Release Package"

echo -e "${YELLOW}Send release package via email?${NC}"
echo -e "  To: production.spatika@gmail.com, rajesh.spatika@gmail.com"
echo -e "  CC: ssraghavan.spatika@gmail.com"
echo -e "  Attachment: AIO9_5.0_v${VERSION}.zip (${ZIP_SIZE})"
echo -e "\n${YELLOW}Send email? (y/n):${NC}"
read -r SEND_EMAIL

if [ "$SEND_EMAIL" = "y" ] || [ "$SEND_EMAIL" = "Y" ]; then
    if [ -f "$ZIP_FILE" ]; then
        python3 send_release_email.py "$VERSION" "$ZIP_FILE" "$RELEASE_NOTES" "$SUMMARY"
        
        if [ $? -eq 0 ]; then
            print_success "Email sent successfully"
        else
            print_warning "Email preparation completed. Check /tmp for manual sending instructions."
        fi
    else
        print_error "Cannot send email: ZIP file not found"
    fi
else
    print_warning "Skipped email. Run manually: python3 send_release_email.py $VERSION $ZIP_FILE $RELEASE_NOTES \"$SUMMARY\""
fi

echo -e "\n${GREEN}🎉 Release v${VERSION} is ready for deployment!${NC}\n"
