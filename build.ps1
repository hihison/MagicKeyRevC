# Build script for MagicKeyRevC
# Compiles the program and copies all required files to ./bin/

Write-Host "Building MagicKeyRevC..." -ForegroundColor Green

# Create bin directory if it doesn't exist
if (!(Test-Path ".\bin")) {
    New-Item -Path ".\bin" -ItemType Directory -Force | Out-Null
    Write-Host "Created bin directory" -ForegroundColor Yellow
}

# Clean previous build
Remove-Item ".\bin\*" -Force -Recurse -ErrorAction SilentlyContinue
Write-Host "Cleaned bin directory" -ForegroundColor Yellow

# Create embedded key header if public key exists
Write-Host "Creating embedded key header..." -ForegroundColor Yellow

# Check if embedded_key.h already exists (user's real key)
if (Test-Path ".\embedded_key.h") {
    Write-Host "Using existing embedded_key.h (your real key)" -ForegroundColor Green
} else {
    # Create embedded_key.h from available sources
    if (Test-Path ".\public_key.pem") {
        $publicKeyContent = Get-Content ".\public_key.pem" -Raw
        Write-Host "Using public_key.pem" -ForegroundColor Yellow
    } elseif (Test-Path ".\public_key.pem.example") {
        $publicKeyContent = Get-Content ".\public_key.pem.example" -Raw
        Write-Host "Using public_key.pem.example (this is just an example!)" -ForegroundColor Red
    } else {
        Write-Host "Warning: No public key file found, using placeholder" -ForegroundColor Red
        $publicKeyContent = @"
-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA1234567890abcdef
EXAMPLE_KEY_REPLACE_WITH_ACTUAL_KEY
-----END PUBLIC KEY-----
"@
    }

    # Create embedded_key.h
    $embeddedKeyHeader = @"
#pragma once
#include <string>

namespace EmbeddedKey {
    static const std::string PUBLIC_KEY_PEM = R"($publicKeyContent)";
}
"@

    Set-Content -Path ".\embedded_key.h" -Value $embeddedKeyHeader -Encoding UTF8
    Write-Host "Generated embedded_key.h from available sources" -ForegroundColor Green
}

# Compile the program
Write-Host "Compiling..." -ForegroundColor Yellow
$compileCommand = @(
    "C:\msys64\ucrt64\bin\g++.exe"
    "-O2"  # Optimization for release
    "-s"   # Strip symbols for smaller size
    "-static-libgcc"
    "-static-libstdc++"
    "*.cpp"
    "-o"
    ".\bin\main.exe"
    "-lole32"
    "-loleaut32"
    "-lwbemuuid"
    "-lwininet"
    "-lssl"
    "-lcrypto"
    "-lbcrypt"
    "-lcrypt32"
    "-lgdi32"
    "-lws2_32"
    "-L."
    "-I.\include"
    ".\WebView2Loader.dll.lib"
)

& $compileCommand[0] $compileCommand[1..($compileCommand.Length-1)]

if ($LASTEXITCODE -eq 0) {
    Write-Host "Compilation successful!" -ForegroundColor Green
    
    # Copy required DLLs and libraries
    Write-Host "Copying required files..." -ForegroundColor Yellow
    
    # WebView2 files
    Copy-Item ".\WebView2Loader.dll" ".\bin\" -ErrorAction SilentlyContinue
    Copy-Item ".\WebView2LoaderStatic.lib" ".\bin\" -ErrorAction SilentlyContinue
    Copy-Item ".\libWebView2Loader.a" ".\bin\" -ErrorAction SilentlyContinue
    
    # Note: public_key.pem is now embedded in the executable, not needed as separate file
    
    # Copy MinGW runtime DLLs if they exist in the current directory
    $mingwDlls = @(
        "libgcc_s_seh-1.dll"
        "libstdc++-6.dll"
        "libwinpthread-1.dll"
    )
    
    foreach ($dll in $mingwDlls) {
        if (Test-Path ".\$dll") {
            Copy-Item ".\$dll" ".\bin\"
            Write-Host "Copied $dll" -ForegroundColor Cyan
        }
    }
    
    # Try to copy MinGW DLLs from MSYS2 installation
    $msys2Path = "C:\msys64\ucrt64\bin"
    if (Test-Path $msys2Path) {
        foreach ($dll in $mingwDlls) {
            if (Test-Path "$msys2Path\$dll") {
                Copy-Item "$msys2Path\$dll" ".\bin\" -ErrorAction SilentlyContinue
                Write-Host "Copied $dll from MSYS2" -ForegroundColor Cyan
            }
        }
    }
    
    # Copy OpenSSL DLLs if available
    $opensslDlls = @(
        "libssl-3-x64.dll"
        "libcrypto-3-x64.dll"
    )
    
    Write-Host "Copying OpenSSL DLLs..." -ForegroundColor Yellow
    foreach ($dll in $opensslDlls) {
        if (Test-Path "$msys2Path\$dll") {
            Copy-Item "$msys2Path\$dll" ".\bin\" -ErrorAction SilentlyContinue
            Write-Host "Copied $dll from MSYS2" -ForegroundColor Cyan
        } else {
            Write-Host "Warning: $dll not found in $msys2Path" -ForegroundColor Red
        }
    }
    
    # Verify critical dependencies are present
    Write-Host "Verifying critical dependencies..." -ForegroundColor Yellow
    $criticalDlls = @("libcrypto-3-x64.dll", "libssl-3-x64.dll", "WebView2Loader.dll")
    foreach ($dll in $criticalDlls) {
        if (Test-Path ".\bin\$dll") {
            Write-Host "✓ $dll present" -ForegroundColor Green
        } else {
            Write-Host "✗ $dll MISSING!" -ForegroundColor Red
        }
    }
    
    # Clean up WebView2 cache directories (not needed for distribution)
    Remove-Item ".\bin\*.WebView2" -Force -Recurse -ErrorAction SilentlyContinue
    Remove-Item ".\bin\.webview2" -Force -Recurse -ErrorAction SilentlyContinue
    
    Write-Host "`nBuild completed successfully!" -ForegroundColor Green
    Write-Host "Executable and dependencies are in: .\bin\" -ForegroundColor Green
    
    # Show what's in the bin directory
    Write-Host "`nFiles in bin directory:" -ForegroundColor Yellow
    Get-ChildItem ".\bin" | ForEach-Object {
        $size = if ($_.Length -gt 1MB) { "{0:N1} MB" -f ($_.Length / 1MB) } 
                elseif ($_.Length -gt 1KB) { "{0:N1} KB" -f ($_.Length / 1KB) }
                else { "$($_.Length) bytes" }
        Write-Host "  $($_.Name) ($size)" -ForegroundColor Cyan
    }
    
    # Create ZIP package for easy distribution
    Write-Host "`nCreating ZIP package..." -ForegroundColor Yellow
    $timestamp = Get-Date -Format "yyyyMMdd-HHmm"
    $zipName = "MagicKeyRevC-v1.01C-$timestamp.zip"
    $zipPath = ".\$zipName"
    
    # Remove existing ZIP if it exists
    if (Test-Path $zipPath) {
        Remove-Item $zipPath -Force
    }
    
    # Create ZIP file with all bin contents
    Compress-Archive -Path ".\bin\*" -DestinationPath $zipPath -CompressionLevel Optimal
    
    if (Test-Path $zipPath) {
        $zipSize = (Get-Item $zipPath).Length
        $zipSizeMB = [math]::Round($zipSize / 1MB, 2)
        Write-Host "✓ ZIP package created: $zipName ($zipSizeMB MB)" -ForegroundColor Green
        Write-Host "`nRelease package ready for distribution!" -ForegroundColor Green
        Write-Host "Share this ZIP file - it contains everything needed to run the application." -ForegroundColor Cyan
    } else {
        Write-Host "✗ Failed to create ZIP package" -ForegroundColor Red
    }
    
} else {
    Write-Host "Compilation failed!" -ForegroundColor Red
    exit 1
}
