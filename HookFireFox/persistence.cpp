#include "pch.h"
#include "persistence.h"

bool WriteRegistryRun(const std::string& appName, const std::string& appPath, const std::string& commandArgs)
{
    // 1. RegOpenKeyEx 開啟 HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Run
    HKEY hKey;
    LONG result =
        RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey);
    if (result != ERROR_SUCCESS)
    {
        return false;
    }

    // 2. RegSetValueEx 設定 Registry Key 中的 Value、Type、Data
    std::string fullPath = "\"" + appPath + "\" " + commandArgs;
    result = RegSetValueExA(hKey, appName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(fullPath.c_str()),
        fullPath.size() + 1);
    if (result != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return false;
    }

    // 3. RegCloseKey 關閉 Registry Key 的 Handle
    RegCloseKey(hKey);
    return true;
}
