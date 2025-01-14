#include "pch.h"
#include "utils.h"
#include <vector>
#include <string>
#include <tlhelp32.h>
#include <fstream>
#include <windows.h>
#include <algorithm>
#include <cctype>

std::string toUTF8(const std::wstring& input)
{
    int length = WideCharToMultiByte(CP_UTF8, NULL, input.c_str(), input.size(), NULL, 0, NULL, NULL);
    if (length <= 0)
        return "";
    else
    {
        std::string result;
        result.resize(length);

        if (WideCharToMultiByte(CP_UTF8, NULL, input.c_str(), input.size(), &result[0], result.size(), NULL, NULL) > 0)
            return result;
        else
            return "";
    }
}

std::wstring toUTF16(const std::string& input)
{
    int length = MultiByteToWideChar(CP_UTF8, NULL, input.c_str(), input.size(), NULL, 0);
    if (!(length > 0))
        return std::wstring();
    else
    {
        std::wstring result;
        result.resize(length);

        if (MultiByteToWideChar(CP_UTF8, NULL, input.c_str(), input.size(), &result[0], result.size()) > 0)
            return result;
        else
            return L"";
    }
}

bool IsAscii(const char* buf, int len)
{
    for (int i = 0; i < len; ++i)
    {
        if (static_cast<unsigned char>(buf[i]) > 0x7F)
        {
            return false;
        }
    }

    return true;
}

bool IsPrintable(const char* buf, int len)
{
    for (int i = 0; i < len; ++i)
    {
        unsigned char c = static_cast<unsigned char>(buf[i]);
        if (c < 0x20 || c > 0x7E)
        {
            return false;
        }
    }
    return true;
}

std::string GetCurrentDllPath()
{
    char path[MAX_PATH] = { 0 };
    HMODULE hModule = NULL;

    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCTSTR)GetCurrentDllPath, &hModule))
    {
        if (GetModuleFileNameA(hModule, path, MAX_PATH) == 0)
        {
            return "";
        }
    }

    return std::string(path);
}

std::vector<DWORD> GetProcessIDByName(const std::wstring& processName)
{
    std::vector<DWORD> result;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return result;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnapshot, &pe32))
    {
        do
        {
            if (std::wstring(pe32.szExeFile).find(processName) != std::wstring::npos)
            {
                result.push_back(pe32.th32ProcessID);
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return result;
}

std::string GetCurrentProcessPath()
{
    char path[MAX_PATH] = { 0 };
    if (GetModuleFileNameA(NULL, path, MAX_PATH) == 0)
    {
        return "";
    }

    return std::string(path);
}

DWORD_PTR GetModuleBaseAddress(const char* moduleName)
{
    HMODULE hModule = GetModuleHandleA(moduleName);
    if (hModule == NULL)
    {
        return 0;
    }
    return (DWORD_PTR)hModule;
}

// 新增內文到檔案中
BOOL Log(const std::string& filename, const std::string& content)
{
    std::ofstream file;
    file.open(GetDirectoryFromPath(GetCurrentDllPath()) + "\\" + filename, std::ios::out | std::ios::app | std::ios::binary);
    if (!file.is_open())
    {
        return FALSE;
    }

    file << content;
    file.close();

    return TRUE;
}

// 清除檔案內容
bool ClearFile(const std::string& filename)
{
    std::ofstream file;
    std::string fullPath = GetDirectoryFromPath(GetCurrentDllPath()) + "\\" + filename;
    file.open(fullPath, std::ios::out);  // 使用 std::ios::out，不使用 std::ios::app 和 std::ios::binary
    if (!file.is_open())
    {
        return false;
    }

    // 不需要寫入任何內容，檔案會被截斷
    file.close();

    return true;
}

bool CaseInsensitiveCompare(char a, char b)
{
    return std::tolower(a) == std::tolower(b);
}

size_t FindCaseInsensitive(const std::string& str, const std::string& subStr)
{
    auto it = std::search(str.begin(), str.end(), subStr.begin(), subStr.end(), CaseInsensitiveCompare);

    if (it != str.end())
    {
        return std::distance(str.begin(), it);
    }
    else
    {
        return std::string::npos;
    }
}

std::string GetDirectoryFromPath(const std::string& fullPath)
{
    size_t lastSlashIndex = fullPath.find_last_of("\\/");
    if (lastSlashIndex != std::string::npos)
    {
        return fullPath.substr(0, lastSlashIndex);
    }
    return "";
}

BOOL StrContain(const std::string str, const std::string token)
{
    return std::search(str.begin(), str.end(), token.begin(), token.end()) != str.end();
}
