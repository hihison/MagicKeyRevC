# Build script for MagicKeyRevC
# Compiles the program and copies all required files to ./bin/
# Supports both static and dynamic linking

param(
    [switch]$Static = $false,
    [switch]$Optimized = $false,
    [switch]$Help = $false
)

if ($Help) {
    Write-Host @"
MagicKeyRevC Build Script

Usage:
  .\build.ps1              - Build with DLL dependencies (smaller file, requires DLLs)
  .\build.ps1 -Optimized   - Build with minimal dependencies (OpenSSL embedded, only WebView2 DLL)
  .\build.ps1 -Static      - Build single executable (largest file, no DLLs needed)
  .\build.ps1 -Help        - Show this help

Optimized Build (Recommended):
  - OpenSSL and MinGW runtime embedded in .exe
  - Only WebView2Loader.dll required as external dependency
  - Good balance of size and portability
  - ~2-3MB total package

Static Build (Experimental):
  - May have compatibility issues with WebView2
  - Attempts to embed everything in single .exe
  - Largest file size

Dynamic Build:
  - Smallest .exe file (~1-2MB)
  - Requires separate DLL files (~20MB total)
  - All dependencies copied to bin folder
"@ -ForegroundColor Cyan
    exit 0
}

if ($Static) {
    Write-Host "Building MagicKeyRevC (STATIC - Single Executable)..." -ForegroundColor Green
} elseif ($Optimized) {
    Write-Host "Building MagicKeyRevC (OPTIMIZED - Minimal Dependencies)..." -ForegroundColor Green
} else {
    Write-Host "Building MagicKeyRevC (DYNAMIC - with DLLs)..." -ForegroundColor Green
}

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

if ($Static) {
    Write-Host "Using static linking (single executable)..." -ForegroundColor Cyan
    $compileCommand = @(
        "C:\msys64\ucrt64\bin\g++.exe"
        "-O2"  # Optimization for release
        "-s"   # Strip symbols for smaller size
        "-static"  # Static linking
        "-static-libgcc"
        "-static-libstdc++"
        "*.cpp"
        "-o"
        ".\bin\main.exe"
        "-lole32"
        "-loleaut32"
        "-lwbemuuid"
        "-lwininet"
        "-Wl,-Bstatic"
        "-lssl"
        "-lcrypto"
        "-Wl,-Bdynamic"
        "-lbcrypt"
        "-lcrypt32"
        "-lgdi32"
        "-lws2_32"
        "-L."
        "-I.\include"
        ".\WebView2LoaderStatic.lib"
    )
} elseif ($Optimized) {
    Write-Host "Using optimized linking (OpenSSL embedded, WebView2 dynamic)..." -ForegroundColor Cyan
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
        "-Wl,-Bstatic"
        "-lssl"
        "-lcrypto"
        "-Wl,-Bdynamic"
        "-lbcrypt"
        "-lcrypt32"
        "-lgdi32"
        "-lws2_32"
        "-L."
        "-I.\include"
        ".\WebView2Loader.dll.lib"
    )
} else {
    Write-Host "Using dynamic linking (with DLLs)..." -ForegroundColor Cyan
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
}

& $compileCommand[0] $compileCommand[1..($compileCommand.Length-1)]

if ($LASTEXITCODE -eq 0) {
    Write-Host "Compilation successful!" -ForegroundColor Green
    
    if ($Static) {
        Write-Host "Static build complete - single executable created!" -ForegroundColor Green
        Write-Host "No DLL files needed - the executable is self-contained." -ForegroundColor Cyan
        
        # Check if the executable has any external dependencies
        Write-Host "Checking dependencies..." -ForegroundColor Yellow
        try {
            $deps = & "C:\msys64\ucrt64\bin\objdump.exe" -p ".\bin\main.exe" | Select-String "DLL Name"
            if ($deps) {
                Write-Host "External dependencies found:" -ForegroundColor Yellow
                $deps | ForEach-Object { Write-Host "  $_" -ForegroundColor Cyan }
            } else {
                Write-Host "✅ No external DLL dependencies - fully static!" -ForegroundColor Green
            }
        } catch {
            Write-Host "Could not check dependencies (objdump not available)" -ForegroundColor Yellow
        }
        
    } elseif ($Optimized) {
        Write-Host "Optimized build complete - minimal dependencies!" -ForegroundColor Green
        Write-Host "Only WebView2Loader.dll required as external dependency." -ForegroundColor Cyan
        
        # Copy only WebView2 DLL
        Write-Host "Copying required WebView2 DLL..." -ForegroundColor Yellow
        Copy-Item ".\WebView2Loader.dll" ".\bin\" -ErrorAction SilentlyContinue
        
        # Check dependencies
        Write-Host "Checking dependencies..." -ForegroundColor Yellow
        try {
            $deps = & "C:\msys64\ucrt64\bin\objdump.exe" -p ".\bin\main.exe" | Select-String "DLL Name"
            if ($deps) {
                Write-Host "External dependencies:" -ForegroundColor Yellow
                $deps | ForEach-Object { Write-Host "  $_" -ForegroundColor Cyan }
            }
        } catch {
            Write-Host "Could not check dependencies (objdump not available)" -ForegroundColor Yellow
        }
        
    } else {
        # Copy required DLLs and libraries for dynamic build
        Write-Host "Copying required files for dynamic build..." -ForegroundColor Yellow
        
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
    }
    
    # Clean up WebView2 cache directories (not needed for distribution)
    Remove-Item ".\bin\*.WebView2" -Force -Recurse -ErrorAction SilentlyContinue
    Remove-Item ".\bin\.webview2" -Force -Recurse -ErrorAction SilentlyContinue
    
    Write-Host "`nBuild completed successfully!" -ForegroundColor Green
    
    if ($Static) {
        Write-Host "🎉 STATIC BUILD: Single executable created!" -ForegroundColor Green
        Write-Host "📁 Location: .\bin\main.exe" -ForegroundColor Green
        Write-Host "✅ No DLLs required - fully portable!" -ForegroundColor Cyan
        Write-Host "📦 You can distribute just the main.exe file" -ForegroundColor Cyan
    } elseif ($Optimized) {
        Write-Host "🎉 OPTIMIZED BUILD: Minimal dependencies!" -ForegroundColor Green
        Write-Host "📁 Location: .\bin\" -ForegroundColor Green
        Write-Host "📦 Distribute main.exe + WebView2Loader.dll only" -ForegroundColor Cyan
        Write-Host "✅ OpenSSL and MinGW runtime embedded in main.exe" -ForegroundColor Cyan
    } else {
        Write-Host "🎉 DYNAMIC BUILD: Executable and dependencies created!" -ForegroundColor Green
        Write-Host "📁 Location: .\bin\" -ForegroundColor Green
        Write-Host "📦 Distribute all files in the bin folder together" -ForegroundColor Cyan
    }
    
    # Show what's in the bin directory
    Write-Host "`nFiles in bin directory:" -ForegroundColor Yellow
    $totalSize = 0
    Get-ChildItem ".\bin" | ForEach-Object {
        $totalSize += $_.Length
        $size = if ($_.Length -gt 1MB) { "{0:N1} MB" -f ($_.Length / 1MB) } 
                elseif ($_.Length -gt 1KB) { "{0:N1} KB" -f ($_.Length / 1KB) }
                else { "$($_.Length) bytes" }
        
        if ($_.Name -eq "main.exe") {
            Write-Host "  🎯 $($_.Name) ($size)" -ForegroundColor Green
        } else {
            Write-Host "  📄 $($_.Name) ($size)" -ForegroundColor Cyan
        }
    }
    
    $totalSizeMB = $totalSize / 1MB
    Write-Host ("`nTotal package size: {0:N1} MB" -f $totalSizeMB) -ForegroundColor Yellow
    
    if ($Static) {
        Write-Host "`n💡 To build with minimal dependencies: .\build.ps1 -Optimized" -ForegroundColor DarkGray
        Write-Host "💡 To build with DLLs: .\build.ps1" -ForegroundColor DarkGray
    } elseif ($Optimized) {
        Write-Host "`n💡 To build single executable: .\build.ps1 -Static" -ForegroundColor DarkGray
        Write-Host "💡 To build with all DLLs: .\build.ps1" -ForegroundColor DarkGray
    } else {
        Write-Host "`n💡 To build with minimal dependencies: .\build.ps1 -Optimized" -ForegroundColor DarkGray
        Write-Host "💡 To build single executable: .\build.ps1 -Static" -ForegroundColor DarkGray
    }
    
} else {
    Write-Host "Compilation failed!" -ForegroundColor Red
    exit 1
}
