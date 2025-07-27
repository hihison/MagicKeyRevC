# MagicKeyRevC

A secure WebView2-based application with hardware fingerprinting and encrypted data transmission.

## � Quick Start

1. **Clone the repository**
   ```bash
   git clone <your-repo-url>
   cd MagicKeyRevC
   ```

2. **Setup configuration**
   ```bash
   cp config.h.example config.h
   cp public_key.pem.example public_key.pem
   ```

3. **Edit your settings**
   - Open `config.h` and replace placeholder URLs with your actual endpoints
   - Replace `public_key.pem` with your actual RSA public key

4. **Build the application**
   ```powershell
   .\build.ps1
   ```

5. **Run**
   ```bash
   cd bin
   .\main.exe
   ```

📖 **See [SETUP.md](SETUP.md) for detailed instructions**

## ⚠️ Important Security Notes

- **Never commit `config.h`** - Contains your actual configuration
- **Never commit `public_key.pem`** - Contains your actual public key  
- **Never commit private keys** - Keep them secure and local only
- The `.gitignore` is configured to protect these files automatically

## 🔧 Features

- **Hardware Fingerprinting** - System UUID, HDD ID, Machine GUID
- **Encrypted Communication** - RSA encryption for sensitive data
- **Secure WebView** - Disabled developer tools, context menu, copy/paste
- **Network Detection** - IP and proxy checking
- **Configurable** - Compile-time configuration for security

## 📁 Build Output

The `./bin/` directory contains everything needed for distribution:
- Application executable
- Required DLLs and libraries  
- Configuration files
- Clean, portable package

## 🛡️ Security Features

- Compile-time configuration (no runtime config files)
- RSA encrypted data transmission
- WebView2 security restrictions
- Hidden cache directories
- No sensitive data in source code

## 📋 Requirements

- **Windows 10/11** - 64-bit
- **MinGW-w64 with MSYS2** - C++ compiler
- **WebView2 Runtime** - Usually pre-installed
- **OpenSSL** - For encryption (included with MSYS2)

## 🔨 Build Scripts

- `build.ps1` - PowerShell build script (recommended)
- `build.bat` - Batch file for Command Prompt
- Manual build commands available in [BUILD.md](BUILD.md)

## 🆘 Support

- Check [SETUP.md](SETUP.md) for detailed setup instructions
- See [BUILD.md](BUILD.md) for build troubleshooting
- Review `.gitignore` to understand what files are protected

## 📄 License

[Add your license information here]
```bash
# Development mode (debug enabled, security relaxed)
switch-config.bat dev

# Production mode (debug disabled, security strict)
switch-config.bat prod

# Reset to defaults
switch-config.bat default
```

## 🔒 Security Features

- **Separated Sensitive Data** - Real URLs/keys never in source code
- **Configurable WebView2 Security**:
  - Disable developer tools
  - Block right-click context menu
  - Prevent text selection
  - Disable copy/paste shortcuts
- **Encrypted Data Transmission** - RSA public key encryption
- **Debug Data Control** - Configurable logging levels

## 🏗️ Build Configuration

### Development Build
```bash
# Enable debug output and relaxed security
g++ *.cpp -o main_dev.exe [compile flags] -DDEBUG_MODE
```

### Production Build
```bash
# Optimized build with strict security
g++ *.cpp -o main.exe [compile flags] -O2 -DNDEBUG
```

## 📚 Documentation

- [Configuration Guide](CONFIG_README.md) - Complete configuration documentation
- [Setup Guide](SETUP.md) - Detailed setup instructions
- [Security Guide](GITHUB_SAFETY.md) - Security best practices

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Follow the security guidelines in `GITHUB_SAFETY.md`
4. Never commit sensitive configuration files
5. Test with both development and production configurations
6. Submit a pull request

## ⚠️ Security Notes

- **Never commit** `config_defaults.cpp` (contains real URLs/IDs)
- **Never commit** `*.pem` files (cryptographic keys)
- **Never commit** `config.json` (runtime configuration)
- Use the provided templates for sharing configuration structure
- Review `.gitignore` before committing

## 📄 License

[Add your license information here]

## 🐛 Troubleshooting

### Build Issues
- Ensure all dependencies are installed
- Check WebView2 SDK paths in compile command
- Verify OpenSSL libraries are available

### Configuration Issues
- Run `setup.bat` to create required files
- Check that `config_defaults.cpp` exists and has real values
- Ensure `public_key.pem` is present

### Runtime Issues
- Enable debug mode in configuration for detailed logging
- Check network connectivity for API endpoints
- Verify WebView2 runtime is installed

## 📞 Support

For issues and questions:
1. Check the documentation in the `docs/` directory
2. Review common issues in this README
3. Create an issue with detailed error information
