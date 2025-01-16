// IPControl.cpp
#include "pch.h"
#include "IPControl.h"

// 全局變數定義
std::atomic<bool> g_clipboardLoggerEnabled{ false };
std::vector<std::string> g_targetIPs = {
	"224.166.102.103", 
	"240.166.102.103",
	"43.3.22.104",
	"192.168.0.0"
};