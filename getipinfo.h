#pragma once
#include <string>
#include <iostream>
#include <windows.h>
#include <wininet.h>
#include <vector>
#include "json.hpp" // Download from https://github.com/nlohmann/json/releases
#include "config.h"

#pragma comment(lib, "wininet.lib")

inline std::string http_get(const std::string& url, const std::string& user_agent = "") {
    std::string ua = user_agent;
    if (ua.empty() && g_config) {
        ua = g_config->get_user_agent();
    }
    if (ua.empty()) {
        ua = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) WMMT/111.0.0.0 WMMT/537.36";
    }
    HINTERNET hInternet = InternetOpenA(ua.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) return "";

    HINTERNET hConnect = InternetOpenUrlA(
        hInternet,
        url.c_str(),
        NULL,
        0,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE,
        0);

    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return "";
    }

    std::vector<char> buffer;
    DWORD bytesRead = 0;
    char temp[4096];
    while (InternetReadFile(hConnect, temp, sizeof(temp), &bytesRead) && bytesRead != 0) {
        buffer.insert(buffer.end(), temp, temp + bytesRead);
    }

    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return std::string(buffer.begin(), buffer.end());
}

// Returns ipinfo and ipinfo2 JSON objects
inline bool fetch_ipinfo_pair(nlohmann::json& ipinfo, nlohmann::json& ipinfo2) {
    ipinfo = nullptr;
    ipinfo2 = nullptr;
    try {
        std::string ipcheck_url = g_config ? g_config->get_ipcheck_url() : "https://ipcheck.siu4.workers.dev/";
        std::string proxycheck_url = g_config ? g_config->get_proxycheck_url() : "https://proxycheck.io/v2/";
        
        std::string res1 = http_get(ipcheck_url);
        ipinfo = nlohmann::json::parse(res1);
        std::string ip;
        if (ipinfo.contains("IP")) {
            ip = ipinfo["IP"].get<std::string>();
            std::string res2 = http_get(proxycheck_url + ip + "?vpn=1&asn=1");
            auto j2 = nlohmann::json::parse(res2);
            if (j2.contains(ip)) {
                ipinfo2 = j2[ip];
                return true;
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Error fetching IP info: " << e.what() << std::endl;
        return false;
    }
    return false;
}