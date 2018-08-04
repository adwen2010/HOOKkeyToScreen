#include "StdAfx.h"
#include "Screen.h"
#include <atltypes.h>

#define STR_CLASSNAME		L"ScW"
#define STR_CLASSPOINTER	L"CLASSPOINTER"
#define TIMERID_TOPWND		0x100

CScreen::CScreen(void):m_hThread(NULL), m_fontsize(30)
{
	GdiplusStartupInput gdiplusStartupInput;
	m_gdiplusToken = 0;
	// Initialize GDI+.
	GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
	m_format.SetAlignment(StringAlignmentNear);
}

CScreen::~CScreen(void)
{
	GdiplusShutdown(m_gdiplusToken);
}

LRESULT CALLBACK CScreen::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE h = ::GetProp(hWnd, STR_CLASSPOINTER);
	if (NULL == h)
	{
		return ::DefWindowProc(hWnd, message, wParam, lParam);
	}

	return reinterpret_cast<CScreen*>(h)->OnWndProc(message, wParam, lParam);
}

void CScreen::ShowSCWater(const wchar_t* lpwstr, HDESK hDesk)
{
	if (NULL == m_hThread)
	{
		m_wstrText = lpwstr;
		m_hDesk = hDesk;
		m_hThread = ::CreateThread(NULL, 0, ThreadFun, this, 0, NULL);
	}
}

DWORD CScreen::RealThreadFun( LPVOID pParam )
{
	::SetThreadDesktop(m_hDesk);
	if (NULL == RegisterWindowClass())
	{
	}

	CreateSCWindow();

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{	
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

DWORD WINAPI CScreen::ThreadFun( LPVOID pParam )
{
	CScreen* pSC = reinterpret_cast<CScreen*>(pParam);
	return pSC->RealThreadFun(pParam);
}

ATOM CScreen::RegisterWindowClass()
{
	WNDCLASSEX wcex;

	wcex.cbSize         = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW|CS_NOCLOSE;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= m_s_hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= STR_CLASSNAME;
	wcex.hIconSm		= NULL;

	return RegisterClassEx(&wcex);
}

int CScreen::CreateSCWindow()
{
	m_Blend.BlendOp				= 0; //theonlyBlendOpdefinedinWindows2000
	m_Blend.BlendFlags			= 0; //nothingelseisspecial...
	m_Blend.AlphaFormat			= 1; //...
	m_Blend.SourceConstantAlpha = 60;//AC_SRC_ALPHA

	CRect rtWorkArea;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rtWorkArea, 0);// 获得工作区大小

	m_nWidth  = rtWorkArea.Width();
	m_nHeight = rtWorkArea.Height(); 

	m_hWnd = ::CreateWindowEx((WS_EX_TOOLWINDOW/*|WS_EX_LAYERED*/|
								WS_EX_TRANSPARENT|WS_EX_TOPMOST)&~WS_EX_APPWINDOW,
								STR_CLASSNAME, L"",
								WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP/*WS_OVERLAPPEDWINDOW*/,
								rtWorkArea.left, rtWorkArea.top, m_nWidth, m_nHeight, NULL, NULL, m_s_hInstance, NULL);

	::SetProp(m_hWnd, STR_CLASSPOINTER, this);

	::PostMessage(m_hWnd, WM_PAINT, 0, 0);
	
	::SetTimer(m_hWnd, TIMERID_TOPWND, 500, (TIMERPROC)NULL);

	return 0;
}

LRESULT CScreen::OnWndProc( UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC          hdc;
	PAINTSTRUCT  ps;

	BringWindowToTop(m_hWnd);
	switch(message)
	{
	case WM_PAINT:
		hdc = ::BeginPaint(m_hWnd, &ps);
		OnPaint(hdc);
		EndPaint(m_hWnd, &ps);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_TIMER:
		BringWindowToTop(m_hWnd);
	//case WM_INITDIALOG:
	//	OnInitDialog();
	//	break;
	default:
		return DefWindowProc(m_hWnd, message, wParam, lParam);
	}

	return DefWindowProc(m_hWnd, message, wParam, lParam);
}

void CScreen::OnPaint(HDC hdc)
{
	HDC          hdcMemory = CreateCompatibleDC(hdc);
	HBITMAP      hBitMap   = CreateCompatibleBitmap(hdc,m_nWidth,m_nHeight);
	GraphicsPath path;
	
	SelectObject(hdcMemory,hBitMap);
	//
	// Graphics 定义不能放在SelectObject函数之前，否则无法显示水印
	//
	Graphics     graphics(hdcMemory);

	graphics.SetSmoothingMode(SmoothingModeAntiAlias);
	graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);

	//graphics.RotateTransform(10);

	vector<Point> vecPt;
	GenerateVecPt(vecPt, m_wstrText.c_str());

	FontFamily fontFamily(L"Arial");
	
	for (vector<Point>::iterator iterVec = vecPt.begin(); vecPt.end() != iterVec; ++iterVec)
	{
		path.AddString(m_wstrText.c_str(), m_wstrText.length(), &fontFamily,
			           FontStyleRegular, m_fontsize, *iterVec, &m_format);
	}

	Pen pen(Color(255,0,0,0),3);
	graphics.DrawPath(&pen, &path);
	/*这里也可以添加图片*/
	Image* im = Image::FromFile(L"d:\\rose.jpg");
	printf("(%d,%d)(%d,%d)",m_nHeight,m_nWidth,im->GetHeight(),im->GetWidth());
	Point impoint((m_nWidth - im->GetWidth()) / 2,(m_nHeight-im->GetHeight())/2);
	graphics.DrawImage(im, impoint);

	LinearGradientBrush linGrBrush(
									Point(0, 0), Point(0, 90),
									Color(255, 255, 125, 255),
									Color(0, 255, 128, 255));//color第一个参数是透明度255不透明，设置起始颜色和结束颜色均为255,128,255

	LinearGradientBrush linGrBrushW(
									Point(0, 10), Point(0, 60),
									Color(255, 255, 128, 255),
									Color(0, 255, 128, 255));

	graphics.FillPath(&linGrBrush, &path);
	graphics.FillPath(&linGrBrushW, &path);

	LONG lWindowLong = ::GetWindowLong(m_hWnd, GWL_EXSTYLE);
	if ((lWindowLong&WS_EX_LAYERED) != WS_EX_LAYERED)
	{
		lWindowLong = lWindowLong| WS_EX_LAYERED;
		::SetWindowLong(m_hWnd, GWL_EXSTYLE, lWindowLong);
	}

	POINT ptSrc      = {0, 0};
	POINT ptWinPos   = {0, 0};
	HDC   hdcScreen  = ::GetDC(m_hWnd);
	SIZE  sizeWindow = {m_nWidth, m_nHeight};
	BOOL  bRet       = UpdateLayeredWindow(m_hWnd, hdcScreen, &ptWinPos,
											&sizeWindow, hdcMemory, &ptSrc,
											0, &m_Blend, ULW_ALPHA);

	::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, m_nWidth, m_nHeight, 
					SWP_SHOWWINDOW);

	::ReleaseDC(m_hWnd, hdcScreen);
	::ReleaseDC(m_hWnd, hdc);
	
	hdcScreen = NULL;
	hdc = NULL;

	DeleteObject(hBitMap);
	DeleteDC(hdcMemory);
	hdcMemory = NULL;
}
//
//int CScreen::GetDisplayLen( Graphics& graphics ,StringFormat& format,
//									 const wchar_t* lpwstr)
//{
//	SolidBrush  brush(Color(150, 228, 228, 228));
//	FontFamily	fontFamily(L"Arial");
//	Font        font(&fontFamily, 38, FontStyleRegular, UnitPoint);
//	PointF      origin(0.0f, 0.0f);
//	RectF       boundRect;
//
//	// Measure the string.
//	graphics.MeasureString(lpwstr, -1, &font, origin, &format, &boundRect);
//	origin.X += 200;
//	origin.Y += 200;
//	graphics.DrawString(lpwstr, -1, &font, origin, &format,&brush);
//
//	return boundRect.Width;
//}

void CScreen::HideSCWater()
{
	PostMessage(m_hWnd, WM_QUIT, 0, 0);

	while (::IsWindow(m_hWnd))
	{
		::Sleep(100);
	}
	m_hThread = NULL;
}

size_t CScreen::GenerateVecPt( vector<Point>& vecPt, const wchar_t* lpwstr )
{
	Point        pt;
	RectF        boundRect;
	GraphicsPath pathTemp;
	FontFamily   fontFamily(L"Arial");

	pathTemp.AddString(lpwstr, -1, &fontFamily,
						FontStyleRegular, m_fontsize, pt, &m_format);
	pathTemp.GetBounds(&boundRect);

	const INT    offset   = 20;
	int          iCount   = 4/*1.0*(m_nWidth - boundRect.Width - offset*2)/cx*/;
	REAL         cx       = (m_nWidth - boundRect.Width )/2;
	REAL         cyoffset = 1.0*(m_nHeight - boundRect.Height - offset*2) / REAL(iCount-1);

	vecPt.push_back(Point(cx,m_nHeight/2));

	return vecPt.size();
}


HINSTANCE CScreen::m_s_hInstance = NULL;
