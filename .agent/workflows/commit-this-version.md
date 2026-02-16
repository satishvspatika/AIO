---
description: Formal process to commit a new version, build all configurations, and package the release.
---

1. **Verify Version:** Check `globals.h` for the correct `FIRMWARE_VERSION`.
2. **Review/Create Release Notes:** Ensure `RELEASE_NOTES_v<VERSION>.md` is present and describes all changes.
3. **Commit Code:**
   // turbo
   `git add . && git commit -m "Release v<VERSION>: <Summary>" && git tag -a v<VERSION> -m "Version <VERSION>"`
4. **Build All Configurations:**
   // turbo
   `python3 build_all_configs.py`
5. **Verify Package:** Check that the folder `EXTERNAL_RELEASE_BASE/v<VERSION>` contains:
   - Configuration folders (e.g., `KSNDMC_TRG`, `SPATIKA_GEN`, etc.)
   - `flash_files/` (bootloader, partitions)
   - `RELEASE_NOTES.md`
