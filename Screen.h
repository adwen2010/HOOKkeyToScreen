#pragma once

#include <ObjBase.h>
#include <GdiPlus.h>
using namespace Gdiplus;

class CScreen
{
private:
	HANDLE			 m_hThread;
	HDESK			 m_hDesk ;
	HWND			 m_hWnd;
	int				 m_nWidth;
	int				 m_nHeight;
	ULONG_PTR		 m_gdiplusToken;
	BLENDFUNCTION	 m_Blend;
	static HINSTANCE        m_s_hInstance;
	REAL			 m_fontsize ;
	StringFormat	 m_format;
	wstring	 m_wstrText;

private:
	static DWORD WINAPI ThreadFun(LPVOID pParam);
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	DWORD	 RealThreadFun(LPVOID pParam);
	ATOM	 RegisterWindowClass();
	int      CreateSCWindow();
	LRESULT  OnWndProc( UINT, WPARAM, LPARAM);
	void     OnPaint(HDC hdc);
	size_t   GenerateVecPt(vector<Point>& vecPt, const wchar_t* lpwstr);
	//void OnInitDialog();
	CScreen(void);
	~CScreen(void);
	CScreen(CScreen&) {}
	CScreen&  operator=(CScreen&) {}
public:
	void ShowSCWater(const wchar_t* lpwstr, HDESK hDesk);
	void HideSCWater();
	static CScreen& getCScreen()
	{
		static CScreen m_screen;
		return m_screen;
	}
	//int GetDisplayLen(Graphics& graphics ,StringFormat& format,
	//				  const wchar_t* lpwstr);
};
