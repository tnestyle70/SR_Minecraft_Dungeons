@echo off
setlocal

set "ROOT=%~dp0"
set "SERVER_DIR=%ROOT%Server\Server\x64\Debug"
set "SERVER_EXE=%SERVER_DIR%\Server.exe"
set "CLIENT_DIR=%ROOT%Client\Include"
set "CLIENT_EXE=%ROOT%Client\Bin\Client.exe"

if not exist "%SERVER_EXE%" (
    echo Server.exe was not found. Build Debug x64 first.
    pause
    exit /b 1
)

if not exist "%CLIENT_EXE%" (
    echo Client.exe was not found. Build Debug x64 first.
    pause
    exit /b 1
)

start "Minecraft Dungeons Server" /D "%SERVER_DIR%" "%SERVER_EXE%"
timeout /t 1 /nobreak >nul
start "Minecraft Dungeons Client" /D "%CLIENT_DIR%" "%CLIENT_EXE%"

endlocal
