@echo off

echo If you want your own version and not default you must run script like:
echo setup.bat {sdk_version}

set DEFAULT_SDK_VERSION=2.3.1
set "WORK_DIR=%cd%"

call :set_proper_sdk_version %*

if not exist %WORK_DIR%\ext-lib\ (
    mkdir %WORK_DIR%\ext-lib
)
cd %WORK_DIR%\ext-lib
call :should_fetch_new_sdk
if errorlevel 1 (
    echo Not fetching new SDK packages as hashes match
) else (
    echo "Fetching new SDK package"
    call :fetch_new_sdk
    call :unzip_sdk_package
)

cd %WORK_DIR%
call :build_cpp_injection_demo

goto :eof

:set_proper_sdk_version
set sdk_version=%1
if "%sdk_version%"=="" (
    set sdk_version=%DEFAULT_SDK_VERSION%
)
goto :eof

:should_fetch_new_sdk
if exist sdk\version.txt (
    set /p current_version=<sdk\version.txt
) else (
    set current_version=none
)

echo %current_version%
echo %sdk_version%

if %current_version% == %sdk_version% (
    exit /b 1
) else (
    if %current_version% neq "none" (
        rd /s /q sdk\
    )
    exit /b 0
)
goto :eof

:fetch_new_sdk
set platform=%PROCESSOR_ARCHITECTURE%
if "%platform%"=="AMD64" (
    set system=windows
) else (
    echo Unsupported platform
    exit /b 1
)
curl -L https://github.com/DolbyIO/comms-sdk-cpp/releases/download/%sdk_version%/cppsdk-%sdk_version%-%system%64.zip -O -J -L
goto :eof

:unzip_sdk_package
if not exist cppsdk-%sdk_version%-%system%64.zip (
    echo There is not sdk zip package here!
    exit /b 1
)

set package_name=sdk-release
mkdir sdk
powershell Expand-Archive cppsdk-%sdk_version%-%system%64.zip -DestinationPath sdk

xcopy /s /e /y sdk\%package_name%\* sdk\
rd /s /q sdk\%package_name%
del cppsdk-%sdk_version%-%system%64.zip
echo %sdk_version% > sdk\version.txt
goto :eof

:build_cpp_injection_demo
if not exist %WORK_DIR%\build\ (
    mkdir %WORK_DIR%\build
)
cd %WORK_DIR%\build
cmake ../ -A x64
cmake --build . -j 16 --config RelWithDebInfo
cmake --install .
goto :eof
