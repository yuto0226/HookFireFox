#pragma once
#include "pch.h"

std::string toUTF8(const std::wstring& input);
std::wstring toUTF16(const std::string& input);
bool IsAscii(const char* buf, int len);
bool IsPrintable(const char* buf, int len);
std::string GetCurrentDllPath();
std::vector<DWORD> GetProcessIDByName(const std::wstring& processName);
std::string GetCurrentProcessPath();
DWORD_PTR GetModuleBaseAddress(const char* moduleName);
BOOL Log(const std::string& filename, const std::string& content);
bool ClearFile(const std::string& filename);
bool CaseInsensitiveCompare(char a, char b);
size_t FindCaseInsensitive(const std::string& str, const std::string& subStr);
std::string GetDirectoryFromPath(const std::string& fullPath);
BOOL StrContain(const std::string str, const std::string token);