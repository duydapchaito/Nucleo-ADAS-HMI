@echo off
chcp 65001 >nul
title Radar ECU - Tool Launcher

:: Kích hoạt môi trường ảo
if exist ".venv\Scripts\activate.bat" (
    call .venv\Scripts\activate.bat
)

:Menu
cls
echo ==================================================
echo       RADAR ECU - CHỌN CHẾ ĐỘ HOẠT ĐỘNG
echo ==================================================
echo 1. Full Mode: Chay ca Gateway va Diagnostic Tester
echo 2. COM Mode:  Chi chay Gateway (Cho Qt Dashboard)
echo 3. TP Mode:   Chi chay Tester (Neu Gateway da mo)
echo 0. Thoat
echo ==================================================

choice /C 1230 /M "Nhap lua chon cua ban:"

if errorlevel 4 goto End
if errorlevel 3 goto RunTester
if errorlevel 2 goto RunGateway
if errorlevel 1 goto RunAll

:RunAll
echo [*] Dang khoi dong Gateway (Khong hien CMD do)...
start "" pythonw gateway.py

echo [*] Doi 2 giay de Gateway mo Socket UDP...
timeout /t 2 /nobreak >nul

echo [*] Dang khoi dong Diagnostic Tester...
start "" pythonw main_tester.py
goto End

:RunGateway
echo [*] Dang khoi dong Gateway...
start "" pythonw gateway.py
goto End

:RunTester
echo [*] Dang khoi dong Diagnostic Tester...
start "" pythonw main_tester.py
goto End

:End