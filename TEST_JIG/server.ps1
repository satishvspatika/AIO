# ============================================================
# Spatika Factory Jig — PowerShell HTTP Server
# Called by launch_windows.bat
# Uses only built-in .NET / System.Net — no extra installs.
# ============================================================

$Port      = 8000
$Root      = $PSScriptRoot      # folder this script lives in
$IndexFile = "factory_tool.html"

$MimeTypes = @{
    ".html" = "text/html; charset=utf-8"
    ".css"  = "text/css"
    ".js"   = "application/javascript"
    ".json" = "application/json"
    ".bin"  = "application/octet-stream"
    ".txt"  = "text/plain; charset=utf-8"
    ".png"  = "image/png"
    ".ico"  = "image/x-icon"
}

# ── Start the HTTP Listener ─────────────────────────────────
$listener = New-Object System.Net.HttpListener
$listener.Prefixes.Add("http://localhost:$Port/")

try {
    $listener.Start()
} catch {
    Write-Host ""
    Write-Host "  ERROR: Could not start server on port $Port" -ForegroundColor Red
    Write-Host "  Another process may be using that port." -ForegroundColor Yellow
    Write-Host "  Close other instances and try again." -ForegroundColor Yellow
    Write-Host ""
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host ""
Write-Host "  [OK] Server running  ->  http://localhost:$Port/$IndexFile" -ForegroundColor Green
Write-Host "  Close this window to stop the server." -ForegroundColor DarkGray
Write-Host ""

# ── Open Chrome / Edge (whichever comes first) ─────────────
$url = "http://localhost:$Port/$IndexFile"
$browsers = @(
    "C:\Program Files\Google\Chrome\Application\chrome.exe",
    "C:\Program Files (x86)\Google\Chrome\Application\chrome.exe",
    "C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe",
    "C:\Program Files\Microsoft\Edge\Application\msedge.exe"
)
$launched = $false
foreach ($b in $browsers) {
    if (Test-Path $b) {
        Start-Process $b $url
        $launched = $true
        break
    }
}
if (-not $launched) {
    Start-Process $url   # fallback: default browser
    Write-Host "  NOTE: WebSerial requires Chrome or Edge." -ForegroundColor Yellow
    Write-Host "  If another browser opened, copy the URL into Chrome." -ForegroundColor Yellow
}

# ── Request handler loop ────────────────────────────────────
while ($listener.IsListening) {
    try {
        $ctx      = $listener.GetContext()
        $req      = $ctx.Request
        $resp     = $ctx.Response

        # Resolve URL path to a local file
        $urlPath  = $req.Url.LocalPath.TrimStart('/')
        if ($urlPath -eq '') { $urlPath = $IndexFile }

        # Sanitise path: prevent directory traversal
        $filePath = [System.IO.Path]::GetFullPath(
                        [System.IO.Path]::Combine($Root, $urlPath))

        if (-not $filePath.StartsWith($Root)) {
            $resp.StatusCode = 403
            $resp.Close()
            continue
        }

        if ([System.IO.File]::Exists($filePath)) {
            $ext         = [System.IO.Path]::GetExtension($filePath).ToLower()
            $mime        = if ($MimeTypes.ContainsKey($ext)) { $MimeTypes[$ext] } else { "application/octet-stream" }
            $bytes       = [System.IO.File]::ReadAllBytes($filePath)

            $resp.StatusCode        = 200
            $resp.ContentType       = $mime
            $resp.ContentLength64   = $bytes.Length
            # CORS header so fetch() inside the page always works
            $resp.Headers.Add("Access-Control-Allow-Origin", "*")
            $resp.OutputStream.Write($bytes, 0, $bytes.Length)
        } else {
            $resp.StatusCode = 404
            $body = [System.Text.Encoding]::UTF8.GetBytes("404 Not Found: $urlPath")
            $resp.ContentType = "text/plain"
            $resp.ContentLength64 = $body.Length
            $resp.OutputStream.Write($body, 0, $body.Length)
        }

        $resp.OutputStream.Flush()
        $resp.Close()

    } catch [System.Net.HttpListenerException] {
        # Listener was stopped (window closed) — exit cleanly
        break
    } catch {
        # Log other errors but keep running
        Write-Host "  [WARN] $($_.Exception.Message)" -ForegroundColor Yellow
    }
}

$listener.Stop()
Write-Host "  Server stopped." -ForegroundColor DarkGray
