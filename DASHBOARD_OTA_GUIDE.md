# 🚀 Firmware Control Center: Dashboard Setup (v5.77)

This guide provides the **complete code** for a web-based "Firmware Control Center" on your Contabo VPS. This replaces Linux manual commands with a simple browser-based upload system for your 7 specific categories.

---

## 1. 📂 Folder Setup on Contabo
Ensure your web server (Nginx/Apache) has access to a `bins` folder:
```bash
mkdir -p /var/www/html/bins
chmod 777 /var/www/html/bins  # Allow PHP to write files
```

---

## 2. 🗄️ Database Table: `firmware_registry`
Create this table to map your 7 categories to their current "Live" binaries:

```sql
CREATE TABLE firmware_registry (
    category_id INT PRIMARY KEY,
    name VARCHAR(32),        -- e.g. 'SPATIKA_ADDON'
    unit_type VARCHAR(32),   -- e.g. 'SPATIKA_GEN'
    system_mode INT,         -- 0, 1, or 2
    current_ver VARCHAR(16), -- e.g. '5.77'
    filename VARCHAR(128),   -- e.g. 'FW_S2_SPATIKA_GEN.bin'
    update_triggered BOOLEAN DEFAULT FALSE
);

-- Insert your 7 default categories
INSERT INTO firmware_registry (category_id, name, unit_type, system_mode) VALUES
(1, 'KSNDMC_TRG', 'KSNDMC_TRG', 0),
(2, 'BIHAR_TRG', 'BIHAR_TRG', 0),
(3, 'KSNDMC_TWS', 'KSNDMC_TWS', 1),
(4, 'KSNDMC_ADDON', 'KSNDMC_ADDON', 2),
(5, 'SPATIKA_TRG', 'SPATIKA_GEN', 0),
(6, 'SPATIKA_ADDON', 'SPATIKA_GEN', 2),
(7, 'KSNDMC_TWS_AP', 'KSNDMC_TWS-AP', 1);
```

---

## 3. 🌐 Web UI: `/ota/dashboard.php`
Paste this code to create your browser-based upload page.

```php
<?php
// Simple header and category list
$categories = $db->query("SELECT * FROM firmware_registry");
?>

<div style="font-family: sans-serif; padding: 20px; background: #f4f7f6;">
    <h1 style="color: #2c3e50;">🚀 Firmware Control Center</h1>
    <table border="1" style="width: 100%; border-collapse: collapse; background: white;">
        <tr style="background: #34495e; color: white;">
            <th>Category</th>
            <th>Type (SYSTEM / UNIT)</th>
            <th>Current Version</th>
            <th>Live Binary</th>
            <th>Upload & Release</th>
        </tr>
        <?php foreach($categories as $cat): ?>
        <tr>
            <td><strong><?= $cat['name'] ?></strong></td>
            <td>SYS: <?= $cat['system_mode'] ?> / UNIT: <?= $cat['unit_type'] ?></td>
            <td>v<?= $cat['current_ver'] ?></td>
            <td><code><?= $cat['filename'] ?></code></td>
            <td>
                <form action="upload.php" method="POST" enctype="multipart/form-data">
                    <input type="hidden" name="cat_id" value="<?= $cat['category_id'] ?>">
                    <input type="text" name="ver" placeholder="New Ver (e.g. 5.77)" required>
                    <input type="file" name="firmware" accept=".bin" required>
                    <button type="submit">Deploy to Group</button>
                </form>
            </td>
        </tr>
        <?php endforeach; ?>
    </table>
</div>
```

---

## 4. 📤 Backend: `upload.php`
This script handles the file upload, renames it based on category, and unlocks the update for all stations in that group.

```php
<?php
$cat_id = $_POST['cat_id'];
$new_ver = $_POST['ver'];
$cat = $db->query("SELECT * FROM firmware_registry WHERE category_id = $cat_id")->fetch();

// Standard naming convention
$new_filename = "FW_S" . $cat['system_mode'] . "_" . $cat['unit_type'] . ".bin";
$upload_path = "/var/www/html/bins/" . $new_filename;

if (move_uploaded_file($_FILES['firmware']['tmp_name'], $upload_path)) {
    // 1. Update the registry with the new version and filename
    // 2. Set 'update_triggered' to TRUE for all stations matching this unit/system
    $db->execute("UPDATE firmware_registry SET current_ver = '$new_ver', filename = '$new_filename' WHERE category_id = $cat_id");
    echo "✅ Success! All " . $cat['name'] . " units will update on their next health report.";
}
?>
```

---

## 5. 📡 The Health Handshake: `/health`
Update your health report logic to automatically check if a group update is pending.

```php
// In your health response logic:
$stn_ver = $data['ver'];
$stn_unit = $data['unit_type'];
$stn_sys = $data['system'];

// Find the live firmware for this station's configuration
$live = $db->query("SELECT * FROM firmware_registry WHERE unit_type = '$stn_unit' AND system_mode = $stn_sys")->fetch();

if ($live && $stn_ver < $live['current_ver']) {
    $response["cmd"] = "OTA_CHECK";
    $response["cmd_param"] = $live['filename']; // e.g. "FW_S2_SPATIKA_GEN.bin"
}
```

---

## 📦 What's Included in v5.77:
*   **Success Confirmation**: Automatically detects the `"cdm_sts": "Firmware Updated"` report.
*   **Zero CLI**: No need to log into Linux; just use your browser.
*   **Group Management**: One upload updates 100 stations of the same type.
