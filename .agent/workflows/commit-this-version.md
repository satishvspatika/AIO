---
description: Formal process to commit a new version, build all configurations, and package the release.
---

1. **Update Firmware Version**: Ensure `FIRMWARE_VERSION` in `globals.h` is incremented.
2. **Review Changes**: Verify all modifications are correct and tested.
3. **Execute Automated Release**:
   // turbo
   `./create_release.sh "Your detailed release summary here" --non-interactive`

This single command will:
- Generate/Verify Release Notes
- Commit code and tag the version
- Build all 6 configurations
- Package the release (ZIP)
- Push to GitHub (Tags & Main)
- Send the Release Email with attachments.

