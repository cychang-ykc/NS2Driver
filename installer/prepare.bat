@echo off
setlocal

set ROOT=B:\Vibe_Coding\ns2_pro_con
set BUILD_DIR=%ROOT%\build\Release
set FILES_DIR=%ROOT%\installer\files

echo [1/2] Copying build output...
copy /Y "%BUILD_DIR%\ns2drv.exe"      "%FILES_DIR%\ns2drv.exe"
copy /Y "%BUILD_DIR%\hidapi.dll"      "%FILES_DIR%\hidapi.dll"
copy /Y "%BUILD_DIR%\libusb-1.0.dll"  "%FILES_DIR%\libusb-1.0.dll"

if not exist "%FILES_DIR%\ViGEmBusSetup.exe" (
    echo.
    echo [!] Missing: installer\files\ViGEmBusSetup.exe
    echo     Download from: https://github.com/nefarius/ViGEmBus/releases/latest
    echo     Save as: %FILES_DIR%\ViGEmBusSetup.exe
    echo.
    pause
    exit /b 1
)

if not exist "%FILES_DIR%\HidHideSetup.exe" (
    echo.
    echo [!] Missing: installer\files\HidHideSetup.exe
    echo     Download from: https://github.com/nefarius/HidHide/releases/latest
    echo     Save as: %FILES_DIR%\HidHideSetup.exe
    echo.
    pause
    exit /b 1
)

echo [2/2] Running Inno Setup compiler...
"%LOCALAPPDATA%\Programs\Inno Setup 6\ISCC.exe" "%ROOT%\installer\ns2drv_setup.iss"

if %ERRORLEVEL% == 0 (
    echo.
    echo Done! Installer is at: %ROOT%\installer\dist\NS2DriverSetup.exe
) else (
    echo.
    echo Build failed.
)
pause
endlocal
