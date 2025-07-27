#pragma once
#include <string>
#include <windows.h>

inline std::string get_machine_guid() {
    HKEY hKey;
    const wchar_t* subkey = L"SOFTWARE\\Microsoft\\Cryptography";
    const wchar_t* valueName = L"MachineGuid";
    wchar_t value[256];
    DWORD value_length = sizeof(value);

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, subkey, 0, KEY_READ | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
        return "";

    LONG ret = RegQueryValueExW(hKey, valueName, nullptr, nullptr, (LPBYTE)value, &value_length);
    RegCloseKey(hKey);

    if (ret != ERROR_SUCCESS)
        return "";

    // Convert wide char to UTF-8 std::string
    int len = WideCharToMultiByte(CP_UTF8, 0, value, -1, NULL, 0, NULL, NULL);
    std::string guid(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, value, -1, &guid[0], len - 1, NULL, NULL);

    return guid;
}