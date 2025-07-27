#pragma once
#include <string>
#include <windows.h>
#include <wininet.h>
#include "config.h"

// Sends encrypted data via HTTP GET and returns server reply as string.
// Returns true on success, false on failure.
inline bool send_data(const std::string& encrypted_data, std::string& server_reply) {
    server_reply.clear();

    // Configuration must be loaded - no fallback to production URLs for security
    if (!g_config) {
        server_reply = "Configuration not loaded";
        return false;
    }

    std::string base_url = g_config->get_backend_url();
    std::string url = base_url + "?message=" + encrypted_data;

    std::string user_agent = g_config->get_user_agent();
    
    HINTERNET hInternet = InternetOpenA(user_agent.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) return false;

    HINTERNET hConnect = InternetOpenUrlA(
        hInternet,
        url.c_str(),
        NULL,
        0,
        INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE,
        0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return false;
    }

    // Read the server reply
    char buffer[4096];
    DWORD bytes_read = 0;
    std::string reply;
    while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytes_read) && bytes_read != 0) {
        reply.append(buffer, bytes_read);
    }

    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    server_reply = reply;
    return !server_reply.empty();
}