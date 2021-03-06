// key.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include<windows.h>
#include<stdio.h>
#include "detours.h"
#pragma comment(lib,"detours.lib") 
#include "Screen.h"

#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )//隐藏窗口


HHOOK MyHook;                  //接收由SetWindowsHookEx返回的旧的钩子
int CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam);
#include "vector"
using namespace std;
vector<char> vk;
//主函数
int main()
{
	//安装钩子
	MyHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)&KeyboardProc, GetModuleHandle(NULL), 0);
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) != -1)                   //消息循环
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	};
	UnhookWindowsHookEx(MyHook);
	return 0;
}

//回调函数，用于处理截获的按键消息
int CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{

	if (code >= HC_ACTION && wParam == WM_KEYDOWN) //有键按下
	{
		DWORD vk_code = ((KBDLLHOOKSTRUCT*)lParam)->vkCode;
		printf("lParam = %d code = %d HC_ACTION = %d WM_KEYDOWN=%d wParam = %d vk_code = %d\n", lParam, code, HC_ACTION, WM_KEYDOWN, wParam, vk_code);
		BYTE ks[256];
 		GetKeyboardState(ks);
		WORD w;
		ToAscii(vk_code, 0, ks, &w, 0);
		char ch = char(w);
		printf("%c\n", ch);
		if (ch >= '0' && ch <= 'z')
		{
			vk.push_back(ch);
			string str(vk.end()-vk.begin()>100?vk.end()-100:vk.begin(),vk.end());
			if (string::npos != str.find("love"))
			{
				CScreen::getCScreen().ShowSCWater(L"I LOVE YOU TOO!",NULL);
				vk.clear();
			}
			else if (string::npos != str.find("aini"))
			{
				CScreen::getCScreen().ShowSCWater(L"我也爱你哟", NULL);
				vk.clear();
			}
			else if(string::npos != str.find("clear"))
			{
				CScreen::getCScreen().HideSCWater();
				vk.clear();
			}
		}
	}
	return CallNextHookEx(MyHook, code, wParam, lParam);  //将消息还给钩子链，不要影响别人
}



