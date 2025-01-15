#pragma once
#include <Windows.h>
#include <string>
#include "utils.h"

// kernel32.dll 的函數
typedef HANDLE(WINAPI* OpenProcess_t)(DWORD, BOOL, DWORD);

// firefox nss3.dll 的函數
typedef int (*PR_Read_t)(void* fd, void* buf, int amount);
typedef int (*PR_Write_t)(void* fd, char* buf, int amount);

template<typename function_t>
function_t GetProcFromDll(std::string proc, std::string dll)
{
    HINSTANCE hdLL = LoadLibraryW(toUTF16(dll).data());
    if (hdLL == NULL) {
        DWORD error = GetLastError();
        Log("log.txt", "[-] Get " + dll + " handle failed, error=" + std::to_string(error) + ".\n");
        return nullptr;
    }

    function_t fpFunc = reinterpret_cast<function_t>(GetProcAddress(hdLL, proc.data()));
    if (fpFunc == NULL) {
        DWORD error = GetLastError();
        Log("log.txt", "[-] Get " + proc + "() from " + dll + " failed, error=" + std::to_string(error) + ".\n");
        FreeLibrary(hdLL);
        return nullptr;
    }
    return fpFunc;
}
