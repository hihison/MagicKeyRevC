#include <windows.h>
#include <wrl/client.h>
#include <iostream>
#include <string>
#include <vector>
#include "WebView2.h"
#include "getuuid.h"
#include "gethddid.h"
#include "getmachineguid.h"
#include "getipinfo.h"
#include "json.hpp"
#include "encrypt_data.h"
#include "send_data.h"
#include "config.h"

using Microsoft::WRL::ComPtr;

// Window procedure for WebView2 window
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// Helper: Disable context menu, F12, highlight/copy/paste for WebView2
void RestrictWebView2(ComPtr<ICoreWebView2>& webview) {
    if (!g_config) return;
    
    bool disable_context_menu = g_config->should_disable_context_menu();
    bool disable_text_selection = g_config->should_disable_text_selection();
    bool disable_copy_paste = g_config->should_disable_copy_paste();
    
    if (!disable_context_menu && !disable_text_selection && !disable_copy_paste) {
        return; // No restrictions needed
    }
    
    std::string script = "// WebView2 Security Restrictions\n";
    
    if (disable_context_menu) {
        script += "document.addEventListener('contextmenu', event => event.preventDefault());\n";
    }
    
    script += R"(
            // Disable F12 and Ctrl+Shift+I (DevTools)
            document.addEventListener('keydown', function(e) {
                if (e.keyCode === 123 || (e.ctrlKey && e.shiftKey && e.key.toLowerCase() === 'i')) {
                    e.preventDefault();
                }
    )";
    
    if (disable_copy_paste) {
        script += R"(
                // Disable Ctrl+C, Ctrl+V, Ctrl+X (Copy/Paste/Cut)
                if ((e.ctrlKey && (e.key.toLowerCase() === 'c' || e.key.toLowerCase() === 'v' || e.key.toLowerCase() === 'x'))) {
                    e.preventDefault();
                }
        )";
    }
    
    if (disable_text_selection) {
        script += R"(
                // Disable text selection (mouse and keyboard)
                if (e.key === "ArrowLeft" || e.key === "ArrowRight" || e.key === "ArrowUp" || e.key === "ArrowDown" || e.key.toLowerCase() == "a") {
                    if (e.ctrlKey || e.shiftKey) e.preventDefault();
                }
        )";
    }
    
    script += "});\n";
    
    if (disable_text_selection) {
        script += R"(
            // Disable selection by mouse
            document.addEventListener('selectstart', function(e) { e.preventDefault(); });
            // Additional CSS to block selection and highlight
            const css = `
                * {
                    user-select: none !important;
                    -webkit-user-select: none !important;
                    -moz-user-select: none !important;
                    -ms-user-select: none !important;
                }
                ::selection { background: transparent !important; }
            `;
            const style = document.createElement('style');
            style.appendChild(document.createTextNode(css));
            document.head.appendChild(style);
        )";
    }

    webview->AddScriptToExecuteOnDocumentCreated(
        std::wstring(script.begin(), script.end()).c_str(),
        nullptr
    );
}

// COM callback for WebView2Controller
class ControllerHandler : public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler {
public:
    HWND hwnd;
    std::wstring url;
    ComPtr<ICoreWebView2Controller>& controller;
    ComPtr<ICoreWebView2>& webview;

    ControllerHandler(HWND hwnd, const std::wstring& url,
                      ComPtr<ICoreWebView2Controller>& controller,
                      ComPtr<ICoreWebView2>& webview)
        : hwnd(hwnd), url(url), controller(controller), webview(webview) {}

    HRESULT STDMETHODCALLTYPE Invoke(HRESULT, ICoreWebView2Controller* ctrl) override {
        if (ctrl) {
            controller = ctrl;
            ctrl->get_CoreWebView2(&webview);
            RECT bounds;
            GetClientRect(hwnd, &bounds);
            controller->put_Bounds(bounds);
            webview->Navigate(url.c_str());

            // Restrict actions (disable right-click, F12, highlight, copy, paste)
            RestrictWebView2(webview);

            // Disable native DevTools correctly
            ComPtr<ICoreWebView2Settings> settings;
            webview->get_Settings(&settings);
            bool disable_devtools = g_config ? g_config->should_disable_devtools() : true;
            settings->put_AreDevToolsEnabled(disable_devtools ? FALSE : TRUE);
        }
        return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
    ULONG STDMETHODCALLTYPE Release() override { return 1; }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
        if (riid == IID_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler || riid == IID_IUnknown) {
            *ppvObject = this;
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
};

// COM callback for WebView2Environment
class EnvironmentHandler : public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler {
public:
    HWND hwnd;
    std::wstring url;
    ComPtr<ICoreWebView2Controller>& controller;
    ComPtr<ICoreWebView2>& webview;

    EnvironmentHandler(HWND hwnd, const std::wstring& url,
                      ComPtr<ICoreWebView2Controller>& controller,
                      ComPtr<ICoreWebView2>& webview)
        : hwnd(hwnd), url(url), controller(controller), webview(webview) {}

    HRESULT STDMETHODCALLTYPE Invoke(HRESULT, ICoreWebView2Environment* env) override {
        if (env) {
            env->CreateCoreWebView2Controller(
                hwnd,
                new ControllerHandler(hwnd, url, controller, webview)
            );
        }
        return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
    ULONG STDMETHODCALLTYPE Release() override { return 1; }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
        if (riid == IID_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler || riid == IID_IUnknown) {
            *ppvObject = this;
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
};

// Show a WebView2 browser window to the given URL (URL is hidden in UI)
void ShowWebView2(const std::wstring& url) {
    const wchar_t CLASS_NAME[] = L"WebView2Window";
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = CLASS_NAME;
    RegisterClassW(&wc);

    std::string app_name = g_config ? g_config->get_app_name() : "MagicKeyRevC";
    int window_width = g_config ? g_config->get_window_width() : 900;
    int window_height = g_config ? g_config->get_window_height() : 700;
    
    std::wstring app_name_wide(app_name.begin(), app_name.end());

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, app_name_wide.c_str(),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, window_width, window_height,
        NULL, NULL, GetModuleHandleW(nullptr), NULL
    );
    ShowWindow(hwnd, SW_SHOW);

    ComPtr<ICoreWebView2Controller> controller;
    ComPtr<ICoreWebView2> webview;

    // Set WebView2 user data to hidden directory instead of ugly "main.exe.WebView2"
    std::wstring userDataFolder = L".webview2";
    
    // Create directory if it doesn't exist and set hidden attribute
    CreateDirectoryW(userDataFolder.c_str(), nullptr);
    SetFileAttributesW(userDataFolder.c_str(), FILE_ATTRIBUTE_HIDDEN);
    
    CreateCoreWebView2EnvironmentWithOptions(
        nullptr, userDataFolder.c_str(), nullptr,
        new EnvironmentHandler(hwnd, url, controller, webview)
    );

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

int main() {
    // Initialize configuration system
    init_config();
    
    bool debug_enabled = g_config->is_debug_enabled();
    bool log_system_info = g_config->should_log_system_info();
    
    std::string uuid = get_system_uuid();
    if (debug_enabled && log_system_info) {
        std::cout << "System UUID: " << uuid << std::endl;
    }

    std::string machine_guid = get_machine_guid();
    if (debug_enabled && log_system_info) {
        std::cout << "Machine GUID: " << machine_guid << std::endl;
    }

    std::vector<std::string> serials = get_hdd_serials();
    if (debug_enabled && log_system_info) {
        if (serials.empty()) {
            std::cout << "No HDD/SSD serial numbers found." << std::endl;
        } else {
            for (size_t i = 0; i < serials.size(); ++i) {
                std::cout << "HDD/SSD Serial " << (i + 1) << ": " << serials[i] << std::endl;
            }
        }
    }

    nlohmann::json ipinfo, ipinfo2;
    if (debug_enabled) {
        std::cout << "\nFetching IP info...\n";
    }
    if (fetch_ipinfo_pair(ipinfo, ipinfo2)) {
        if (debug_enabled) {
            std::cout << "IP Info (from " << g_config->get_ipcheck_url() << "):\n" << ipinfo.dump(4) << std::endl;
            std::cout << "\nProxyCheck Info (from " << g_config->get_proxycheck_url() << "):\n" << ipinfo2.dump(4) << std::endl;
        }

        nlohmann::json data_to_encrypt = {
            {"ip", ipinfo.value("IP", "Unknown")},
            {"hwid", uuid},
            {"hwserial", serials.empty() ? "" : serials[0]},
            {"country",
                ipinfo2.value("country", "") + "//"
                + ipinfo2.value("provider", "") + "//"
                + ipinfo2.value("organisation", "")
            },
            {"machineguid", machine_guid},
            {"dcid", g_config->get_dcid()},
            {"regdate", ipinfo.value("CheckTimeUTC", "")},
            {"version", g_config->get_app_version()}
        };

        if (debug_enabled) {
            std::cout << "\nData to encrypt:\n" << data_to_encrypt.dump(4) << std::endl;
        }

        std::string encrypted_data = encrypt_data(
            data_to_encrypt,
            g_config->get_public_key_file()
        );

        if (!encrypted_data.empty()) {
            if (debug_enabled && g_config->should_log_encrypted_data()) {
                std::cout << "\nEncrypted data (" << encrypted_data.size() << " chars, base64):\n" << encrypted_data << std::endl;
            }

            std::string server_reply;
            bool success = send_data(encrypted_data, server_reply);

            if (success) {
                if (debug_enabled && g_config->should_log_server_responses()) {
                    std::cout << "Data sent, server replied: " << server_reply << std::endl;
                }

                // Only try to parse JSON if reply looks like JSON
                if (!server_reply.empty() && server_reply[0] == '{') {
                    try {
                        auto j = nlohmann::json::parse(server_reply);
                        if (j.contains("randkey")) {
                            std::string token = j["randkey"];
                            std::string login_url = g_config->get_login_base_url() + "?token=" + token;
                            // Show in embedded browser with restrictions
                            ShowWebView2(std::wstring(login_url.begin(), login_url.end()));
                        } else {
                            if (debug_enabled) std::cout << "randkey not found in server reply." << std::endl;
                        }
                    } catch (std::exception& e) {
                        if (debug_enabled) std::cout << "Failed to parse server reply: " << e.what() << std::endl;
                    }
                } else {
                    if (debug_enabled) std::cout << "Server reply is not JSON: " << server_reply << std::endl;
                }
            } else {
                if (debug_enabled) std::cout << "Failed to send encrypted data." << std::endl;
            }
        } else {
            if (debug_enabled) {
                std::cout << "Size of data_to_encrypt: " << data_to_encrypt.dump().size() << std::endl;
                std::cout << "Encryption failed." << std::endl;
            }
        }
    } else {
        if (debug_enabled) std::cout << "Error fetching IP or proxy info." << std::endl;
    }

    // Cleanup configuration
    cleanup_config();
    return 0;
}