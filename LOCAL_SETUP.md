# Local Development Setup

## Setting up your Public Key

For security reasons, the actual public key is not stored in the repository. You need to set it up locally:

### Option 1: Create embedded_key.h directly (Recommended)
1. Copy your real public key content
2. Create `embedded_key.h` with this format:
```cpp
#pragma once
#include <string>

namespace EmbeddedKey {
    static const std::string PUBLIC_KEY_PEM = R"(-----BEGIN PUBLIC KEY-----
YOUR_ACTUAL_PUBLIC_KEY_CONTENT_HERE
-----END PUBLIC KEY-----)";
}
```

### Option 2: Use public_key.pem file
1. Place your real public key in `public_key.pem`
2. Run `./build.ps1` - it will automatically generate `embedded_key.h`

### For Testing Only
If you don't have a real key yet, the build script will use `public_key.pem.example` as a fallback.

## Security Notes

- `embedded_key.h` is added to `.gitignore` - your real key will NEVER be uploaded to GitHub
- The CI/CD pipeline generates its own `embedded_key.h` from GitHub secrets
- Always keep your private keys secure and never commit them to version control

## Building

Run the build script:
```powershell
.\build.ps1
```

The script will:
1. Check for existing `embedded_key.h` (your real key)
2. If not found, generate it from `public_key.pem` or `public_key.pem.example`
3. Compile the application with the embedded key
4. Package all dependencies in the `bin/` directory
