@echo off
echo Building Raylib Matrix Desktop for Windows...

REM Check if build directory exists, if not create it
if not exist "build" mkdir build

cd build

REM Configure with CMake
echo Configuring with CMake...
cmake .. -G "Visual Studio 17 2022" -A x64
if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

REM Build the project
echo Building project...
cmake --build . --config Release
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo Executable location: build\bin\Release\RaylibMatrixDesktop.exe
echo Assets location: build\bin\assets\
echo.
echo Run the application? (y/n)
set /p choice=
if /i "%choice%"=="y" (
    cd bin
    if exist "Release\RaylibMatrixDesktop.exe" (
        start Release\RaylibMatrixDesktop.exe
    ) else if exist "Debug\RaylibMatrixDesktop.exe" (
        start Debug\RaylibMatrixDesktop.exe
    ) else (
        echo Executable not found in Release or Debug directories!
    )
    cd ..
)

pause 