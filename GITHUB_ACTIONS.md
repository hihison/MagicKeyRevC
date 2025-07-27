# GitHub Actions Setup Guide

This guide shows you how to set up automated building of your MagicKeyRevC project using GitHub Actions with your own private configuration.

## 🔐 Setting Up GitHub Secrets

### Step 1: Go to Repository Settings

1. Go to your GitHub repository
2. Click **Settings** tab
3. In the left sidebar, click **Secrets and variables** → **Actions**
4. Click **New repository secret**

### Step 2: Add Required Secrets

Add these secrets one by one:

#### **Required Configuration Secrets:**

| Secret Name | Description | Example Value |
|-------------|-------------|---------------|
| `BACKEND_URL` | Your main API endpoint | `https://your-api.com/message` |
| `LOGIN_BASE_URL` | Your login page URL | `https://your-site.com/login` |
| `DCID` | Your unique identifier | `your-dcid` |
| `PUBLIC_KEY_PEM` | Your RSA public key | Your actual public key content |

#### **Optional Configuration Secrets (with defaults):**

| Secret Name | Description | Default Value |
|-------------|-------------|---------------|
| `IPCHECK_URL` | IP checking service | `https://ipcheck.siu4.workers.dev/` |
| `PROXYCHECK_URL` | Proxy detection service | `https://proxycheck.io/v2/` |
| `USER_AGENT` | Browser user agent | `Mozilla/5.0 (Windows NT 10.0; Win64; x64) WMMT/111.0.0.0` |
| `APP_VERSION` | Application version | `1.01C` |
| `APP_NAME` | Application name | `MagicKeyRevC` |
| `TIMEOUT_MS` | Network timeout | `30000` |
| `RETRY_ATTEMPTS` | Retry attempts | `3` |
| `WINDOW_WIDTH` | Window width | `900` |
| `WINDOW_HEIGHT` | Window height | `700` |

#### **Security Settings (defaults to true/false):**

| Secret Name | Description | Default |
|-------------|-------------|---------|
| `DISABLE_DEVTOOLS` | Disable F12 developer tools | `true` |
| `DISABLE_CONTEXT_MENU` | Disable right-click menu | `true` |
| `DISABLE_TEXT_SELECTION` | Disable text selection | `true` |
| `DISABLE_COPY_PASTE` | Disable copy/paste | `true` |

#### **Debug Settings (defaults to false):**

| Secret Name | Description | Default |
|-------------|-------------|---------|
| `DEBUG_ENABLED` | Enable debug output | `false` |
| `LOG_ENCRYPTED_DATA` | Log encrypted data | `false` |
| `LOG_SERVER_RESPONSES` | Log server responses | `false` |
| `LOG_SYSTEM_INFO` | Log system information | `false` |

### Step 3: Add Your Public Key

For the `PUBLIC_KEY_PEM` secret, copy your entire public key including the header and footer:

```
-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA...
...your actual public key data...
-----END PUBLIC KEY-----
```

## 🚀 How It Works

### Automatic Builds

The workflow triggers on:
- **Push to main/master branch** - Automatic build
- **Pull requests** - Test builds
- **Manual trigger** - You can run it manually from GitHub Actions tab

### Build Process

1. **Sets up Windows environment** with MSYS2 and MinGW-w64
2. **Creates config.h** from your GitHub secrets
3. **Creates public_key.pem** from your secret
4. **Compiles the application** with all dependencies
5. **Creates distribution package** with all required DLLs
6. **Uploads artifacts** (downloadable for 30 days)

### Download Built Application

After a successful build:
1. Go to **Actions** tab in your repository
2. Click on the latest workflow run
3. Download the **MagicKeyRevC-[commit-hash]** artifact
4. Extract and run!

## 🏷️ Creating Releases

To create a GitHub release:

1. **Create and push a tag:**
   ```bash
   git tag v1.0.0
   git push origin v1.0.0
   ```

2. **GitHub Actions will automatically:**
   - Build the application
   - Create a GitHub release
   - Attach the compiled binaries

## 🔒 Security Benefits

- ✅ **Your secrets stay in GitHub** - Never exposed in code
- ✅ **Automatic builds** - No need to build locally
- ✅ **Clean releases** - Professional distribution packages
- ✅ **No sensitive data in repository** - Complete security

## 🛠️ Local Development

For local development, you still use:
```bash
cp config.h.example config.h
# Edit config.h with your settings
.\build.ps1
```

## 📋 Troubleshooting

### Build Fails
- Check that all required secrets are set
- Verify PUBLIC_KEY_PEM format is correct
- Check Actions tab for detailed error logs

### Missing Dependencies
- The workflow automatically installs all dependencies
- If issues persist, check the MSYS2 setup step

### Secret Not Found Errors
- Ensure secret names match exactly (case-sensitive)
- Required secrets: `BACKEND_URL`, `LOGIN_BASE_URL`, `DCID`, `PUBLIC_KEY_PEM`

## ✅ Quick Setup Checklist

- [ ] Repository settings → Secrets and variables → Actions
- [ ] Add `BACKEND_URL` secret
- [ ] Add `LOGIN_BASE_URL` secret  
- [ ] Add `DCID` secret
- [ ] Add `PUBLIC_KEY_PEM` secret (full key with headers)
- [ ] Push code to trigger first build
- [ ] Check Actions tab for build status
- [ ] Download artifacts when build completes

**Your application will now build automatically with your private configuration!** 🎉
