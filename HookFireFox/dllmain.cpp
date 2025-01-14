// dllmain.cpp : 定義 DLL 應用程式的進入點。
#include "pch.h"
#include "HookFireFox.h"
#include "utils.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    if (IsDebuggerPresent()) {
        return FALSE;
    }

    DWORD pid = GetCurrentProcessId();
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        Log("log.txt", "[+] dllmain: HookFireFox.dll loaded, pid=" + std::to_string(pid) + ".\n");
        HookFireFox();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
