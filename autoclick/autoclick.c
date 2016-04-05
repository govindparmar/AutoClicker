#include <Windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <strsafe.h>
#include <stdlib.h>

#include <CommCtrl.h>
#pragma comment(lib,"Comctl32.lib")

#define IDC_SYSLINK 0x4000
#define IDT_TIMER1 1001


#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif


const TCHAR g_szClassName[] = _T("AutoTyperWndClass2");
HWND hWnd, hEditInt, hEditMS, hRandChk, hStart, hIntExplS, hStop, hDonateLnk;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SysLinkHandler(HWND hWnd, INT nSrcCtrl, LPNMHDR pNMHdr);
BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam);

VOID CALLBACK TimerProc(HWND hwSrc, UINT Msg, UINT_PTR puTID, DWORD dwTime);
ATOM RegisterWCEX(HINSTANCE hInstance);


__forceinline VOID CreateWindows(HINSTANCE hInstance)
{
	INITCOMMONCONTROLSEX iccx;
	iccx.dwICC = ICC_STANDARD_CLASSES | ICC_LINK_CLASS;
	iccx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCommonControlsEx(&iccx);

	hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, g_szClassName, _T("AutoClicker"), WS_VISIBLE | WS_SYSMENU, 100, 100, 400, 200, NULL, NULL, hInstance, NULL);
	//hACExplS = CreateWindow(_T("Static"), _T("When the autoclicker is started, press F8 to stop it."), WS_VISIBLE | WS_CHILD | SS_LEFTNOWORDWRAP, 10, 90, 360, 20, hWnd, NULL, hInstance, NULL);
	hIntExplS = CreateWindow( _T("Static"), _T("Enter the base interval between clicks (seconds; 1-15):"), WS_VISIBLE | WS_CHILD | SS_LEFTNOWORDWRAP, 10, 10, 260, 20, hWnd, NULL, hInstance, NULL);
	hEditInt = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, _T("Edit"), _T("5"), WS_VISIBLE | WS_CHILD | ES_NUMBER | ES_MULTILINE, 290, 10, 60, 20, hWnd, NULL, hInstance, NULL);
	hRandChk = CreateWindow(_T("Button"), _T("Add additional random milliseconds to interval? (1-5000):"), WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | BS_LEFTTEXT | BST_UNCHECKED, 10, 30, 280, 20, hWnd, NULL, hInstance, NULL);
	hEditMS = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, _T("Edit"), _T("50"), WS_VISIBLE | WS_CHILD | ES_NUMBER | ES_MULTILINE | WS_DISABLED, 300, 30, 50, 20, hWnd, NULL, hInstance, NULL);
	hStart = CreateWindow(_T("Button"), _T("Start"), WS_VISIBLE | WS_CHILD | BS_LEFTTEXT, 10, 60, 170, 30, hWnd, NULL, hInstance, NULL);
	hStop = CreateWindow(_T("Button"), _T("Stop"), WS_VISIBLE | WS_CHILD | BS_LEFTTEXT | WS_DISABLED, 180, 60, 170, 30, hWnd, NULL, hInstance, NULL);
	hDonateLnk = CreateWindowEx(0, _T("SysLink"), _T("This program is 100% free.\r\nIf you want to donate to me via PayPal, please <A HREF=\"https://www.paypal.me/govind\">follow this link</A>."), WS_VISIBLE | WS_CHILD, 10, 110, 360, 40, hWnd, (HMENU)IDC_SYSLINK, hInstance, NULL);
	// Old link: <A HREF=\"https://www.paypal.com/cgi-bin/webscr/?cmd=_s-xclick&hosted_button_id=X8EGYQNZ7VBPJ\">follow this link</A>
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd)
{
	ATOM aRes = RegisterWCEX(hInstance);
	MSG Msg;
	if (aRes <= 0)
	{
		MessageBox(0, _T("Window registration failed"), _T("Error"), MB_OK | MB_ICONSTOP);
		return -1;
	}

	CreateWindows(hInstance);

	if (hWnd <= (HWND)0)
	{
		MessageBox(0, _T("Window creation failed"), _T("Error"), MB_OK | MB_ICONSTOP);
		return -1;
	}

	ShowWindow(hWnd, SW_SHOW);
	EnumChildWindows(hWnd, EnumChildProc, 0L);
	UpdateWindow(hWnd);

	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}

BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam)
{
	HFONT hfDefault = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	SendMessage(hWnd, WM_SETFONT, (WPARAM)hfDefault, 0L);
	return TRUE;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	HANDLE_MSG(hWnd, WM_NOTIFY, SysLinkHandler);
	case WM_COMMAND:
	{
		HWND hwSrc = (HWND)lParam;
		if (hwSrc == hRandChk)
		{
			if ((GetWindowLong(hEditMS, GWL_STYLE)&WS_DISABLED) == WS_DISABLED)
			{
				EnableWindow(hEditMS, TRUE);
			}
			else
			{
				EnableWindow(hEditMS, FALSE);
			}
		}
		else if (hwSrc == hStart)
		{
			TCHAR *szBaseIV;
			HANDLE hHeap = GetProcessHeap();
			INT nBLen = GetWindowTextLength(hEditInt), secInt;
			szBaseIV = (TCHAR*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, (nBLen + 2)*sizeof(TCHAR));
			GetWindowText(hEditInt, szBaseIV, nBLen + 1);
			_stscanf_s(szBaseIV, _T("%d"), &secInt);
			if (secInt < 1 || secInt > 15) secInt = 5;
			HeapFree(hHeap, 0, szBaseIV);
			CloseHandle(hHeap);
			MessageBox(0, _T("Program will begin in 5 seconds after pressing OK..."), _T("Information"), MB_OK | MB_ICONASTERISK);
			Sleep(5000);
			EnableWindow(hStart, FALSE);
			EnableWindow(hStop, TRUE);
			SetTimer(hWnd, IDT_TIMER1, secInt * 1000, TimerProc);
		}
		else if (hwSrc == hStop)
		{
			EnableWindow(hStart, TRUE);
			EnableWindow(hStop, FALSE);
			KillTimer(hWnd, IDT_TIMER1);
			
		}
	}
	break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	return 0L;
}

ATOM RegisterWCEX(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hInstance = hInstance;
	wcex.lpfnWndProc = WindowProc;
	wcex.lpszClassName = g_szClassName;

	return RegisterClassEx(&wcex);
}

LRESULT CALLBACK SysLinkHandler(HWND hWnd, INT nSrcCtrl, LPNMHDR pNMHdr)
{
	if (nSrcCtrl != IDC_SYSLINK)
	{
		return 0;
	}
	else
	{
		switch (pNMHdr->code)
		{
		case NM_CLICK:
		case NM_RETURN:
		{
			PNMLINK pNMLink = (PNMLINK)pNMHdr;
			LITEM lItem = pNMLink->item;
			if (lItem.iLink == 0)
			{
				ShellExecute(NULL, _T("open"), lItem.szUrl, NULL, NULL, SW_SHOW);
			}
		}
		}
	}
	return 0;
}
VOID CALLBACK TimerProc(HWND hwSrc, UINT Msg, UINT_PTR puTID, DWORD dwTime)
{
	if (!((GetWindowLong(hEditMS, GWL_STYLE)&WS_DISABLED) == WS_DISABLED))
	{
		TCHAR *szRMS;
		HANDLE hHeap = GetProcessHeap();
		INT nRLen = GetWindowTextLength(hEditMS), msInt;
		srand(GetTickCount());
		szRMS = (TCHAR*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, (nRLen + 2)*sizeof(TCHAR));
		GetWindowText(hEditMS, szRMS, nRLen + 1);
		_stscanf_s(szRMS, _T("%d"), &msInt);
		if (msInt < 0 || msInt > 5000) msInt = 1000;
		HeapFree(hHeap, 0, szRMS);
		Sleep(rand() % msInt);
		CloseHandle(hHeap);
	}
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}