# Build Instructions

## Quick Build

### Option 1: PowerShell (Recommended)
```powershell
.\build.ps1
```

### Option 2: Command Prompt
```cmd
build.bat
```

### Option 3: Manual Command
```cmd
C:\msys64\ucrt64\bin\g++.exe -O2 -s -static-libgcc -static-libstdc++ *.cpp -o .\bin\main.exe -lole32 -loleaut32 -lwbemuuid -lwininet -lssl -lcrypto -lbcrypt -lcrypt32 -lgdi32 -lws2_32 -L. -I.\include .\WebView2Loader.dll.lib
```

## What the Build Script Does

1. **Creates `./bin/` directory** - Clean output location
2. **Compiles with optimizations** - Release build with `-O2` and `-s` (strip symbols)
3. **Static linking** - Includes MinGW runtime statically to reduce dependencies
4. **Copies all required files**:
   - `main.exe` - The compiled application
   - `WebView2Loader.dll` - WebView2 runtime loader
   - `libssl-3-x64.dll` & `libcrypto-3-x64.dll` - OpenSSL libraries
   - `libgcc_s_seh-1.dll`, `libstdc++-6.dll`, `libwinpthread-1.dll` - MinGW runtime
   - `public_key.pem` - RSA public key for encryption
   - Additional WebView2 static libraries
5. **Cleans up cache files** - Removes WebView2 cache directories (auto-generated at runtime)

## Output

After building, the `./bin/` directory will contain:
- **main.exe** - Your executable (~1.1 MB)
- **All required DLLs** - Runtime dependencies (~9 MB total)
- **Configuration files** - Keys and libraries

The entire `./bin/` folder can be distributed as a standalone package.

**Note:** When the application runs, it creates a hidden `.webview2` directory for WebView2 cache. This directory is automatically hidden and should not be distributed.

## Configuration

To change application settings:
1. Edit constants in `config.h`
2. Run build script again
3. New settings are compiled into the executable

## Dependencies

- **MinGW-w64 with MSYS2** - C++ compiler and libraries
- **WebView2 SDK** - For web interface
- **OpenSSL** - For encryption
- **Windows SDK** - For system APIs

## Notes

- The build uses static linking where possible to minimize external dependencies
- OpenSSL and WebView2 DLLs are still required at runtime
- All paths are relative to the project root
- Build output includes file sizes for easy verification
- **WebView2 cache directories** (`*.WebView2/`, `.webview2/`) are automatically created when the app runs and should NOT be distributed
- The build script automatically cleans these cache directories for a clean distribution package
- The application automatically creates and hides the `.webview2` cache directory to keep the bin folder clean
