# How to Send Release Email

## 📧 Email Workflow

The release script now **opens Mail.app** with a pre-filled draft instead of sending directly. This ensures:
- ✅ Email appears in your Gmail sent folder
- ✅ You can review before sending
- ✅ Attachments are properly included
- ✅ Full control over the send process

---

## 🚀 Automated Steps (Done by Script)

When you run `./create_release.sh` and confirm email sending:

1. ✅ Opens Mail.app with new email
2. ✅ Pre-fills recipients and subject
3. ✅ Creates email body file: `/tmp/release_email_v5.31_body.txt`
4. ✅ Lists attachment paths

---

## ✋ Manual Steps (You Do)

After the script opens Mail.app:

### **1. Add Recipients**
- **To:** production.spatika@gmail.com, rajesh.spatika@gmail.com
- **CC:** ssraghavan.spatika@gmail.com

### **2. Verify Subject**
```
AIO9_5.0 Firmware Release v5.31 - Health Report Reliability & GPS Auto-Fix
```

### **3. Attach Files**
Drag and drop these files into the email:
- `/Users/satishkripavasan/Documents/Arduino/ESP32_NEW_DESIGN/RELEASE/AIO9_5/AIO9_5.0_v5.31.zip`
- `RELEASE_NOTES_v5.31.md` (from project directory)

### **4. Copy Email Body**
```bash
# Open the body file
open -a TextEdit /tmp/release_email_v5.31_body.txt

# Copy all content (Cmd+A, Cmd+C)
# Paste into Mail.app email body (Cmd+V)
```

### **5. Send**
Click **Send** button in Mail.app

---

## 📋 Quick Checklist

Before sending, verify:
- [ ] To: production.spatika@gmail.com, rajesh.spatika@gmail.com
- [ ] CC: ssraghavan.spatika@gmail.com
- [ ] Subject contains version number
- [ ] ZIP file attached (~4.2 MB)
- [ ] Release notes attached
- [ ] Email body is complete (not just preview text)
- [ ] Sending from: satishv.spatika@gmail.com

---

## 🎯 Why Manual Sending?

**Pros:**
- ✅ Email appears in Gmail sent folder (synced across devices)
- ✅ You can review attachments before sending
- ✅ No need for app-specific passwords or OAuth setup
- ✅ Uses your existing Gmail account
- ✅ Full control over send timing

**Cons:**
- ⚠️ Requires 2-3 manual steps (attach files, copy body, send)

---

## 🔧 Alternative: Fully Automated (Future)

To make it fully automated, we would need:
1. Gmail API credentials
2. OAuth2 authentication setup
3. App-specific password configuration

**Current approach is simpler and more reliable for now.** ✅

---

## 📧 Email Template

The email includes:
- Version number and release date
- Complete release notes
- Package contents list
- Deployment instructions
- Contact information

**Example Subject:**
```
AIO9_5.0 Firmware Release v5.31 - Health Report Reliability & GPS Auto-Fix
```

**Attachments:**
- `AIO9_5.0_v5.31.zip` (4.16 MB) - Complete firmware package
- `RELEASE_NOTES_v5.31.md` - Detailed release notes

---

## 💡 Pro Tip

Create a Mail.app template for future releases:
1. Send the first release email manually
2. Save as template in Mail.app
3. For future releases, just update version number and attachments

---

**Total time: ~2 minutes to send the email manually** ⏱️
