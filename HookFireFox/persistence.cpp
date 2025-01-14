#include "pch.h"
#include "persistence.h"

bool WriteRegistryRun(const std::string& appName, const std::string& appPath, const std::string& commandArgs)
{
    // 1. RegOpenKeyEx �}�� HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Run
    HKEY hKey;
    LONG result =
        RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey);
    if (result != ERROR_SUCCESS)
    {
        return false;
    }

    // 2. RegSetValueEx �]�w Registry Key ���� Value�BType�BData
    std::string fullPath = "\"" + appPath + "\" " + commandArgs;
    result = RegSetValueExA(hKey, appName.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(fullPath.c_str()),
        fullPath.size() + 1);
    if (result != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return false;
    }

    // 3. RegCloseKey ���� Registry Key �� Handle
    RegCloseKey(hKey);
    return true;
}
