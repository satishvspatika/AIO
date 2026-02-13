#!/usr/bin/env python3
"""
Release Notes Generator for AIO9
Compares current version with previous release to generate changelog
"""

import os
import sys
import re
from pathlib import Path
from datetime import datetime
import difflib

# Configuration
SKETCH_DIR = Path(__file__).parent
RELEASES_DIR = SKETCH_DIR / "releases"
GLOBALS_H = SKETCH_DIR / "globals.h"

def get_current_version():
    """Extract current version from globals.h"""
    with open(GLOBALS_H, 'r') as f:
        content = f.read()
    
    match = re.search(r'#define FIRMWARE_VERSION "([^"]+)"', content)
    if match:
        return match.group(1)
    return "UNKNOWN"

def get_previous_version():
    """Find the most recent previous release"""
    if not RELEASES_DIR.exists():
        return None
    
    versions = []
    for release_file in RELEASES_DIR.glob("RELEASE_NOTES_v*.md"):
        match = re.search(r'v(\d+\.\d+)', release_file.name)
        if match:
            versions.append((match.group(1), release_file))
    
    if not versions:
        return None
    
    # Sort by version number
    versions.sort(key=lambda x: [int(n) for n in x[0].split('.')], reverse=True)
    return versions[0]

def analyze_code_changes():
    """Analyze code changes since last release"""
    changes = {
        'files_modified': [],
        'lines_added': 0,
        'lines_removed': 0,
        'new_features': [],
        'bug_fixes': [],
        'improvements': []
    }
    
    # This would ideally use git diff, but for now we'll use a simple approach
    # You can enhance this to use git if available
    
    return changes

def extract_changes_from_docs():
    """Extract changes from documentation files"""
    changes = {
        'critical_fixes': [],
        'new_features': [],
        'improvements': [],
        'bug_fixes': []
    }
    
    # Look for recent documentation
    doc_files = [
        'FINAL_FIXES_COMPLETE.md',
        'CRITICAL_FIXES_IMPLEMENTED.md',
        'COMPREHENSIVE_PROJECT_REVIEW.md'
    ]
    
    for doc_file in doc_files:
        doc_path = SKETCH_DIR / doc_file
        if doc_path.exists():
            with open(doc_path, 'r') as f:
                content = f.read()
                
                # Extract critical fixes
                if 'CRITICAL' in content or 'Critical' in content:
                    critical_section = re.findall(r'(?:CRITICAL|Critical)[^\n]*\n([^\n]+)', content)
                    changes['critical_fixes'].extend(critical_section[:3])
                
                # Extract new features
                if 'NEW' in content or 'New Feature' in content:
                    new_section = re.findall(r'(?:NEW|New Feature)[^\n]*\n([^\n]+)', content)
                    changes['new_features'].extend(new_section[:3])
    
    return changes

def generate_release_notes(current_version, previous_version_info=None):
    """Generate release notes"""
    
    template = f"""# Release Notes - AIO9 v{current_version}

**Release Date**: {datetime.now().strftime('%Y-%m-%d')}  
**Previous Version**: {previous_version_info[0] if previous_version_info else 'N/A'}  
**Status**: Production Ready

---

## 🎯 Overview

Version {current_version} includes critical improvements and new features.

---

## 🔥 Critical Fixes

"""
    
    # Extract changes from documentation
    changes = extract_changes_from_docs()
    
    if changes['critical_fixes']:
        for fix in changes['critical_fixes'][:5]:
            template += f"- ✅ {fix.strip()}\n"
    else:
        template += "- No critical fixes in this release\n"
    
    template += """
---

## 🆕 New Features

"""
    
    if changes['new_features']:
        for feature in changes['new_features'][:5]:
            template += f"- ✅ {feature.strip()}\n"
    else:
        template += "- No new features in this release\n"
    
    template += """
---

## 🔧 Improvements

"""
    
    if changes['improvements']:
        for improvement in changes['improvements'][:5]:
            template += f"- ✅ {improvement.strip()}\n"
    else:
        template += "- Minor improvements and optimizations\n"
    
    template += f"""
---

## 📊 Comparison with v{previous_version_info[0] if previous_version_info else 'Previous'}

"""
    
    if previous_version_info:
        template += f"""
### Changes Since v{previous_version_info[0]}

See detailed comparison below.

"""
    else:
        template += """
This is the first release with automated release notes.

"""
    
    template += """
---

## 🚀 Deployment

### Pre-Deployment Checklist
- [ ] Compile and verify
- [ ] Test on pilot devices
- [ ] Monitor for 48 hours
- [ ] Full deployment

### Deployment Strategy
1. **Pilot** (Week 1): Deploy to 2-3 test devices
2. **Limited** (Week 2-3): Deploy to 10-20 devices
3. **Full** (Week 4+): Deploy to all devices

---

## 📞 Support

For issues or questions, refer to project documentation.

---

**Version**: {current_version}  
**Release Date**: {datetime.now().strftime('%Y-%m-%d')}  
**Status**: ✅ Production Ready

"""
    
    return template

def save_release_notes(version, content):
    """Save release notes to releases directory"""
    RELEASES_DIR.mkdir(exist_ok=True)
    
    output_file = RELEASES_DIR / f"RELEASE_NOTES_v{version}.md"
    with open(output_file, 'w') as f:
        f.write(content)
    
    # Also save to main directory
    main_file = SKETCH_DIR / f"RELEASE_NOTES_v{version}.md"
    with open(main_file, 'w') as f:
        f.write(content)
    
    return output_file, main_file

def create_changelog():
    """Create CHANGELOG.md with all versions"""
    changelog = "# Changelog - AIO9 Firmware\n\n"
    changelog += "All notable changes to this project will be documented in this file.\n\n"
    changelog += "---\n\n"
    
    if RELEASES_DIR.exists():
        # Get all release files sorted by version
        versions = []
        for release_file in RELEASES_DIR.glob("RELEASE_NOTES_v*.md"):
            match = re.search(r'v(\d+\.\d+)', release_file.name)
            if match:
                versions.append((match.group(1), release_file))
        
        versions.sort(key=lambda x: [int(n) for n in x[0].split('.')], reverse=True)
        
        for version, release_file in versions:
            changelog += f"## Version {version}\n\n"
            
            # Extract overview from release notes
            with open(release_file, 'r') as f:
                content = f.read()
                
                # Extract key sections
                overview = re.search(r'## 🎯 Overview\n\n(.*?)\n\n---', content, re.DOTALL)
                if overview:
                    changelog += overview.group(1) + "\n\n"
                
                critical = re.search(r'## 🔥 Critical Fixes\n\n(.*?)\n\n---', content, re.DOTALL)
                if critical:
                    changelog += "### Critical Fixes\n" + critical.group(1) + "\n\n"
                
                features = re.search(r'## 🆕 New Features\n\n(.*?)\n\n---', content, re.DOTALL)
                if features:
                    changelog += "### New Features\n" + features.group(1) + "\n\n"
            
            changelog += f"[Full Release Notes](releases/RELEASE_NOTES_v{version}.md)\n\n"
            changelog += "---\n\n"
    
    # Save changelog
    changelog_file = SKETCH_DIR / "CHANGELOG.md"
    with open(changelog_file, 'w') as f:
        f.write(changelog)
    
    return changelog_file

def main():
    print("=" * 60)
    print("  AIO9 Release Notes Generator")
    print("=" * 60)
    print()
    
    # Get current version
    current_version = get_current_version()
    print(f"Current Version: {current_version}")
    
    # Get previous version
    previous_version_info = get_previous_version()
    if previous_version_info:
        print(f"Previous Version: {previous_version_info[0]}")
        print(f"Previous Release Notes: {previous_version_info[1]}")
    else:
        print("Previous Version: None found (first release)")
    
    print()
    
    # Generate release notes
    print("Generating release notes...")
    release_notes = generate_release_notes(current_version, previous_version_info)
    
    # Save release notes
    output_file, main_file = save_release_notes(current_version, release_notes)
    print(f"✓ Release notes saved to:")
    print(f"  - {output_file}")
    print(f"  - {main_file}")
    
    # Create/update changelog
    print()
    print("Updating CHANGELOG.md...")
    changelog_file = create_changelog()
    print(f"✓ Changelog updated: {changelog_file}")
    
    print()
    print("=" * 60)
    print("  Release Notes Generation Complete!")
    print("=" * 60)
    print()
    print("Next steps:")
    print("1. Review and edit the generated release notes")
    print("2. Add specific details about changes")
    print("3. Commit the release notes to version control")
    print()

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)
