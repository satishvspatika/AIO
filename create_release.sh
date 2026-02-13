#!/bin/bash

# AIO Project Release Script
# Usage: ./create_release.sh <VERSION>
# Example: ./create_release.sh 5.1

VERSION=$1
DATE=$(date +"%b %d, %y")
FULL_DATE=$(date +"%b %d, %Y")

if [ -z "$VERSION" ]; then
    echo "Usage: ./create_release.sh <VERSION>"
    echo "Example: ./create_release.sh 5.1"
    exit 1
fi

echo "🚀 Preparing Release v$VERSION..."

# 1. Update globals.h
sed -i '' "s/#define FIRMWARE_VERSION \".*\"/#define FIRMWARE_VERSION \"$VERSION\"/" globals.h
echo "✓ Updated globals.h to v$VERSION"

# 2. Update PROJECT_STATUS.md header
sed -i '' "1s/.*/# ESP32 AIO v$VERSION Project Status - $FULL_DATE/" PROJECT_STATUS.md
echo "✓ Updated PROJECT_STATUS.md date/version"

# 3. Git Operations
git add globals.h PROJECT_STATUS.md
git commit -m "Release v$VERSION: $FULL_DATE"
git tag -a "v$VERSION" -m "Release version $VERSION"

echo "📤 Pushing to GitHub..."
git push origin main
git push origin --tags

echo ""
echo "========================================="
echo "  Release v$VERSION Created Successfully! "
echo "========================================="
echo "Now run 'python3 build_all_configs.py' to generate the binaries."
