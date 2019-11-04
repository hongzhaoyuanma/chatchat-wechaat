#include "stdafx.h"
#include <stdio.h>
#include <Windows.h>
#include <wchar.h>
#include <TlHelp32.h>
#include "Inject.h"
//通过进程名称查找进程ID
DWORD ProcessNameToPID(LPCWSTR processName)
{
	wchar_t buffText[0x100] = {0};
	//创建进程快照
	HANDLE ProcessAll = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,NULL);
	PROCESSENTRY32 processInfo = {0};
	processInfo.dwSize = sizeof(PROCESSENTRY32);
	do
	{
		if (wcscmp(processName, processInfo.szExeFile) == 0) {
			swprintf_s(buffText, L"进程名称=%s 进程ID=%d \r\n", processInfo.szExeFile, processInfo.th32ProcessID);
			OutputDebugString(buffText);
			return processInfo.th32ProcessID;
		}
	} while (Process32Next(ProcessAll, &processInfo));
	
	return 0;
}

//注入dll
VOID injectDll(char * dllPath)
{
	wchar_t buff[0x100] = {0};
	//获取目标进程PID
	DWORD PID = ProcessNameToPID(INJECT_PROCESS_NAME);
	if (PID == 0) {
		MessageBox(NULL,L"没有找到该进程，可能为启动该软件",L"没有找到",MB_OK);
		return;
	}
	else {
		//找到pid我们就打开进程
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS,TRUE, PID);
		if (NULL == hProcess) {
			MessageBox(NULL,L"进程打开失败",L"错误",MB_OK);
			return;
		}
		else {
			DWORD strSize = strlen(dllPath) * 2;
			//进程打开后我们把我们的dll路径存进去
			//首先申请一片内存用于储存dll路径
			LPVOID allocRes = VirtualAllocEx(hProcess,NULL, strSize, MEM_COMMIT, PAGE_READWRITE);
			if (NULL == allocRes) {
				MessageBox(NULL,L"内存申请失败",L"错误",MB_OK);
				return;
			}

			//申请好后 我们写入路径到目标内存当中
			if (WriteProcessMemory(hProcess, allocRes, dllPath, strSize, NULL) == 0) {
				MessageBox(NULL, L"DLL路径写入失败", L"错误", MB_OK);
				return;
			}
			//路径写入 成功后我们现在获取LoadLibraryW 基址
			//LoadLibraryW 在Kernel32.dll里面 所以我们先获取这个dll的基址
			HMODULE hModule = GetModuleHandle(L"Kernel32.dll");
			LPVOID address = GetProcAddress(hModule,"LoadLibraryA");
			swprintf_s(buff,L"loadLibrary=%p path=%p", address, allocRes);
			OutputDebugString(buff);
			//通过远程线程执行这个函数 参数传入 我们dll的地址
			//开始注入dll
			HANDLE hRemote = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)address, allocRes,0, NULL);
			if (NULL == hRemote) {
				MessageBox(NULL, L"远程执行失败", L"错误", MB_OK);
				return;
			}
		}
	}
}

//读取内存
VOID readMemory()
{
	DWORD PID = ProcessNameToPID(INJECT_PROCESS_NAME);
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, PID);
	LPCVOID phoneAdd = (LPCVOID)0x10611E80;
	DWORD reSize = 0xB;
	char buff[0x100] = {0};
	wchar_t buffTest[0x100] = {0};
	ReadProcessMemory(hProcess, phoneAdd, buff, reSize,NULL);
	swprintf_s(buffTest,L"add=%p %s ", buff, buff);
	OutputDebugString(buffTest);
}


VOID setWindow(HWND thisWindow)
{
	HWND wechatWindow = FindWindow(L"WeChatMainWndForPC", NULL);
	//上：20 下：620 左：10 右：720
	//MoveWindow(wechatWindow, 10, 20, 100, 600, TRUE);
	
	RECT wechatHandle = {0};
	GetWindowRect(wechatWindow, &wechatHandle);
	LONG width = wechatHandle.right - wechatHandle.left;
	LONG height = wechatHandle.bottom - wechatHandle.top;
	MoveWindow(thisWindow, wechatHandle.left - 230, wechatHandle.top, 240, height, TRUE);
	SetWindowPos(thisWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	wchar_t buff[0x100] = {};
	swprintf_s(buff,L"上：%d 下：%d 左：%d 右：%d\r\n", wechatHandle.top, wechatHandle.bottom, wechatHandle.left, wechatHandle.right);
	
	
	OutputDebugString(buff);
}

//启动微信
//CreateProcess 创建目标进程 创建时即挂起该进程.
//然后注入
//然后再ResumeThread 让目标进程运行
VOID runWechat(TCHAR * dllPath, TCHAR * wechatPath)
{
	//TCHAR dllPath[0x200] = {L"D://code//c//TeachDemo//Release//GetQrcode.dll"};
	//injectDll(paths);
	//TCHAR szDll[] = dllPath;
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;//SW_SHOW
	//TCHAR szCommandLine[MAX_PATH] = TEXT("E:\\Program Files (x86)\\Tencent\\WeChat\\WeChat.exe");

	CreateProcess(NULL, wechatPath, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi);
	LPVOID Param = VirtualAllocEx(pi.hProcess, NULL, MAX_PATH, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	TCHAR add[0x100] = {0};
	
	WriteProcessMemory(pi.hProcess, Param, dllPath, wcslen(dllPath) * 2 + sizeof(char), NULL);
	//swprintf_s(add, L"地址=%p W=%p", Param, pi.hProcess);
	//MessageBox(NULL, add, L"aa", 0);
	//HANDLE hThread = CreateRemoteThread(pi.hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryW, Param, CREATE_SUSPENDED, NULL);

	TCHAR buff[0x100] = {0};
	HMODULE hModule = GetModuleHandle(L"Kernel32.dll");
	LPVOID address = GetProcAddress(hModule, "LoadLibraryW");
	//swprintf_s(buff, L"loadLibrary=%p path=%p", address, Param);
	//MessageBox(NULL,buff,L"aa",0);
	//OutputDebugString(buff);
	//通过远程线程执行这个函数 参数传入 我们dll的地址
	//开始注入dll
	HANDLE hRemote = CreateRemoteThread(pi.hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)address, Param, 0, NULL);
	if (NULL == hRemote) {
		MessageBox(NULL, L"远程执行失败", L"错误", MB_OK);
		return;
	}

	ResumeThread(pi.hThread);
}