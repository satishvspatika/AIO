#!/bin/bash
# ============================================================
# Spatika Factory Jig — macOS Launcher
# Double-click this file to start the dashboard.
# Requires: macOS 12.3+ (Python 3 built-in)
# ============================================================

# Move to the folder this script lives in (works when double-clicked from Finder)
cd "$(dirname "$0")"

PORT=8000

# ── Kill any stale server on the same port ──────────────────
lsof -ti tcp:$PORT | xargs kill -9 2>/dev/null || true

# ── Find a Python 3 interpreter ────────────────────────────
if command -v python3 &>/dev/null; then
  PY=python3
elif command -v python &>/dev/null && python --version 2>&1 | grep -q "Python 3"; then
  PY=python
else
  osascript -e 'display alert "Spatika Jig" message "Python 3 is required but was not found.\nInstall it from https://www.python.org or via Homebrew:\n  brew install python3" as warning'
  exit 1
fi

echo ""
echo "╔══════════════════════════════════════════════╗"
echo "║   Spatika Factory Jig  —  Local HTTP Server  ║"
echo "╚══════════════════════════════════════════════╝"
echo ""
echo "  URL  : http://localhost:$PORT/factory_tool.html"
echo "  Press Ctrl+C in this window to stop the server."
echo ""

# ── Start the server in background ─────────────────────────
$PY -m http.server $PORT --bind 127.0.0.1 &
SERVER_PID=$!

# ── Wait for server to be ready ────────────────────────────
for i in {1..10}; do
  if curl -s "http://localhost:$PORT/" > /dev/null 2>&1; then
    break
  fi
  sleep 0.3
done

# ── Open in Chrome if available, else default browser ──────
if [ -d "/Applications/Google Chrome.app" ]; then
  open -a "Google Chrome" "http://localhost:$PORT/factory_tool.html"
elif [ -d "/Applications/Microsoft Edge.app" ]; then
  open -a "Microsoft Edge" "http://localhost:$PORT/factory_tool.html"
else
  open "http://localhost:$PORT/factory_tool.html"
  echo "  ⚠  WebSerial requires Chrome or Edge."
  echo "     If the page opened in Safari, please copy the URL into Chrome."
fi

# ── Wait for server process — Ctrl+C will stop it ──────────
wait $SERVER_PID
