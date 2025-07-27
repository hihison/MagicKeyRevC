#pragma once
#include "config.h"
#include <iostream>
#include <string>

// Configuration utility functions for compile-time config
namespace ConfigUtils {
    
    void print_current_config(ConfigCompat* config) {
        std::cout << "\n=== Current Configuration ===" << std::endl;
        std::cout << "Application:" << std::endl;
        std::cout << "  Name: " << config->get_app_name() << std::endl;
        std::cout << "  Version: " << config->get_app_version() << std::endl;
        std::cout << "  Window: " << config->get_window_width() << "x" << config->get_window_height() << std::endl;
        std::cout << "  DCID: " << config->get_dcid() << std::endl;
        
        std::cout << "\nURLs:" << std::endl;
        std::cout << "  IP Check: " << config->get_ipcheck_url() << std::endl;
        std::cout << "  Proxy Check: " << config->get_proxycheck_url() << std::endl;
        std::cout << "  Backend: " << config->get_backend_url() << std::endl;
        std::cout << "  Login: " << config->get_login_base_url() << std::endl;
        
        std::cout << "\nSecurity:" << std::endl;
        std::cout << "  DevTools Disabled: " << (config->should_disable_devtools() ? "YES" : "NO") << std::endl;
        std::cout << "  Context Menu Disabled: " << (config->should_disable_context_menu() ? "YES" : "NO") << std::endl;
        std::cout << "  Text Selection Disabled: " << (config->should_disable_text_selection() ? "YES" : "NO") << std::endl;
        std::cout << "  Copy/Paste Disabled: " << (config->should_disable_copy_paste() ? "YES" : "NO") << std::endl;
        
        std::cout << "\nDebug:" << std::endl;
        std::cout << "  Console Output: " << (config->is_debug_enabled() ? "ENABLED" : "DISABLED") << std::endl;
        std::cout << "  Log Server Responses: " << (config->should_log_server_responses() ? "YES" : "NO") << std::endl;
        std::cout << "  Log System Info: " << (config->should_log_system_info() ? "YES" : "NO") << std::endl;
        std::cout << "  Log Encrypted Data: " << (config->should_log_encrypted_data() ? "YES" : "NO") << std::endl;
        
        std::cout << "\nNetwork:" << std::endl;
        std::cout << "  User Agent: " << config->get_user_agent() << std::endl;
        std::cout << "  Timeout: " << config->get_timeout_ms() << "ms" << std::endl;
        std::cout << "  Retry Attempts: " << config->get_retry_attempts() << std::endl;
        std::cout << "============================\n" << std::endl;
    }
    
    void show_config_instructions() {
        std::cout << "\n=== Configuration Instructions ===" << std::endl;
        std::cout << "This application uses COMPILE-TIME configuration." << std::endl;
        std::cout << "To change settings, edit the constants in config.h and recompile:" << std::endl;
        std::cout << "\nExample changes in config.h:" << std::endl;
        std::cout << "  // Change backend URL:" << std::endl;
        std::cout << "  static const std::string BACKEND_URL = \"https://your-server.com/api\";" << std::endl;
        std::cout << "  " << std::endl;
        std::cout << "  // Enable debug mode:" << std::endl;
        std::cout << "  static const bool DEBUG_ENABLED = true;" << std::endl;
        std::cout << "  " << std::endl;
        std::cout << "  // Change window size:" << std::endl;
        std::cout << "  static const int WINDOW_WIDTH = 1200;" << std::endl;
        std::cout << "  static const int WINDOW_HEIGHT = 800;" << std::endl;
        std::cout << "\nAfter editing config.h, recompile with:" << std::endl;
        std::cout << "  PowerShell: .\\build.ps1" << std::endl;
        std::cout << "  CMD: build.bat" << std::endl;
        std::cout << "  Manual: g++ -O2 -s -static-libgcc -static-libstdc++ *.cpp -o .\\bin\\main.exe [flags...]" << std::endl;
        std::cout << "=====================================\n" << std::endl;
    }
}
