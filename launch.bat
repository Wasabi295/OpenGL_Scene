@echo off
title Padure Mistica - Launcher
color 0A

echo ================================================
echo   PADURE MISTICA - OpenGL Scene
echo ================================================
echo.

:: Verifică dacă executabilul există
if not exist "PadureMistica.exe" (
    color 0C
    echo [ERROR] PadureMistica.exe not found!
    echo Make sure you run this script from the correct folder.
    pause
    exit /b 1
)

:: Verifică folderele necesare
echo Checking resources...
if not exist "shaders\" (
    color 0C
    echo [ERROR] Folder 'shaders' not found!
    echo.
    echo Directory structure should be:
    echo   PadureMistica.exe
    echo   shaders\
    echo   models\
    echo   textures\
    pause
    exit /b 1
)
echo [OK] shaders\ found

if not exist "models\" (
    color 0C
    echo [ERROR] Folder 'models' not found!
    pause
    exit /b 1
)
echo [OK] models\ found

if not exist "textures\" (
    color 0C
    echo [ERROR] Folder 'textures' not found!
    pause
    exit /b 1
)
echo [OK] textures\ found

echo.
echo All resources found!
echo Starting application...
echo.
echo ================================================
echo   CONTROLS:
echo   W/A/S/D   - Move camera
echo   Mouse     - Rotate camera
echo   L         - Toggle flashlight
echo   F         - Toggle fog
echo   R         - Toggle rain
echo   T         - Auto tour
echo   ESC       - Exit
echo ================================================
echo.

:: Rulează executabilul și așteaptă să se închidă
PadureMistica.exe

:: Verifică exit code
if %ERRORLEVEL% NEQ 0 (
    color 0C
    echo.
    echo ================================================
    echo [ERROR] Application crashed with code %ERRORLEVEL%
    echo ================================================
    echo.
    echo Check debug_log.txt for details.
    echo.
    
    :: Arată log-ul dacă există
    if exist "debug_log.txt" (
        echo Last 20 lines from debug_log.txt:
        echo ------------------------------------------------
        powershell -Command "Get-Content debug_log.txt -Tail 20"
        echo ------------------------------------------------
    )
)

echo.
pause