#pragma once
#include "stdafx.h"
#define INJECT_PROCESS_NAME L"WeChat.exe"
DWORD ProcessNameToPID(LPCWSTR processName);
VOID injectDll(char * dllPath);
VOID readMemory();
VOID setWindow(HWND thisWindow);
VOID runWechat(TCHAR * dllPath, TCHAR * wechatPath);