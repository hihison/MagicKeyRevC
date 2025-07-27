#pragma once
#include <string>
#include <windows.h>

inline std::string BstrToUtf8(BSTR bstr)
{
    if (!bstr) return "";
    int wslen = SysStringLen(bstr);
    if (wslen == 0) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, bstr, wslen, NULL, 0, NULL, NULL);
    std::string str(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, bstr, wslen, &str[0], len, NULL, NULL);
    return str;
}