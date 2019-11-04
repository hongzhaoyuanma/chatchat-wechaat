#pragma once
#include <Windows.h>
VOID HookWechatQrcode(HWND hwndDlg, DWORD HookAdd);
VOID openApps(HWND hwndDlg, DWORD HookAdd);
VOID UnHook(DWORD HookAdd);
DWORD getWechatWin();
DWORD getKernel32();
