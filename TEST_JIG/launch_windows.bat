@echo off
:: ============================================================
:: Spatika Factory Jig — Windows Launcher
:: Double-click this file to start the dashboard.
:: Requires: Windows 10 / 11 (PowerShell built-in — no extras)
:: ============================================================
title Spatika Factory Jig

echo.
echo  ╔══════════════════════════════════════════════╗
echo  ║   Spatika Factory Jig  -  Local HTTP Server  ║
echo  ╚══════════════════════════════════════════════╝
echo.
echo  Starting server on http://localhost:8000/factory_tool.html
echo  Close this window to stop the server.
echo.

:: Run the PowerShell server script in the same folder
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0server.ps1"

pause
