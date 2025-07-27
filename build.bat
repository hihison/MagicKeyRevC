@echo off
REM Build script for MagicKeyRevC
REM Compiles the program and copies all required files to ./bin/

echo Building MagicKeyRevC...

REM Create bin directory if it doesn't exist
if not exist ".\bin" (
    mkdir ".\bin"
    echo Created bin directory
)

REM Clean previous build
rd /S /Q ".\bin" 2>nul
mkdir ".\bin"
echo Cleaned bin directory

REM Compile the program
echo Compiling...
C:\msys64\ucrt64\bin\g++.exe ^
    -O2 ^
    -s ^
    -static-libgcc ^
    -static-libstdc++ ^
    *.cpp ^
    -o .\bin\main.exe ^
    -lole32 ^
    -loleaut32 ^
    -lwbemuuid ^
    -lwininet ^
    -lssl ^
    -lcrypto ^
    -lbcrypt ^
    -lcrypt32 ^
    -lgdi32 ^
    -lws2_32 ^
    -L. ^
    -I.\include ^
    .\WebView2Loader.dll.lib

if %ERRORLEVEL% EQU 0 (
    echo Compilation successful!
    
    REM Copy required files
    echo Copying required files...
    
    REM WebView2 files
    copy ".\WebView2Loader.dll" ".\bin\" >nul 2>&1
    copy ".\WebView2LoaderStatic.lib" ".\bin\" >nul 2>&1
    copy ".\libWebView2Loader.a" ".\bin\" >nul 2>&1
    
    REM Copy public key
    copy ".\public_key.pem" ".\bin\" >nul 2>&1
    
    REM Copy MinGW runtime DLLs
    copy ".\libgcc_s_seh-1.dll" ".\bin\" >nul 2>&1
    copy ".\libstdc++-6.dll" ".\bin\" >nul 2>&1
    copy ".\libwinpthread-1.dll" ".\bin\" >nul 2>&1
    
    REM Try to copy from MSYS2 if local copies don't exist
    if exist "C:\msys64\ucrt64\bin\libgcc_s_seh-1.dll" (
        copy "C:\msys64\ucrt64\bin\libgcc_s_seh-1.dll" ".\bin\" >nul 2>&1
    )
    if exist "C:\msys64\ucrt64\bin\libstdc++-6.dll" (
        copy "C:\msys64\ucrt64\bin\libstdc++-6.dll" ".\bin\" >nul 2>&1
    )
    if exist "C:\msys64\ucrt64\bin\libwinpthread-1.dll" (
        copy "C:\msys64\ucrt64\bin\libwinpthread-1.dll" ".\bin\" >nul 2>&1
    )
    
    REM Copy OpenSSL DLLs
    if exist "C:\msys64\ucrt64\bin\libssl-3-x64.dll" (
        copy "C:\msys64\ucrt64\bin\libssl-3-x64.dll" ".\bin\" >nul 2>&1
    )
    if exist "C:\msys64\ucrt64\bin\libcrypto-3-x64.dll" (
        copy "C:\msys64\ucrt64\bin\libcrypto-3-x64.dll" ".\bin\" >nul 2>&1
    )
    
    REM Clean up WebView2 cache directories (not needed for distribution)
    rd /S /Q ".\bin\*.WebView2" 2>nul
    rd /S /Q ".\bin\.webview2" 2>nul
    
    echo.
    echo Build completed successfully!
    echo Executable and dependencies are in: .\bin\
    echo.
    echo Files in bin directory:
    dir /B ".\bin\"
    
) else (
    echo Compilation failed!
    pause
    exit /b 1
)

pause
