#include "genos.h"
#include <math.h>
#include "bsautils.h"
#include "bsabs.h"
#ifndef Windows
#include "bsawinapi.h"
#include <dirent.h>
#else
#ifdef __GNUC__
    #define _WIN32_IE 0x600
#endif
#include <commctrl.h>
//#include <afxres.h>
#include <direct.h>
#include <zmouse.h>
#endif
#include "bsashell.h"
#include "bsaxml.h"
#include "bsapersist.h"
#include "bsaframe.h"
#include "bsaopenfile.h"
#include "bsawiz.h"


#include "bcalc.h"

#ifdef Windows
#include "shellapi.h"
#endif

#include "resource.h"

LPCTSTR		g_appname	= _T("BCALC");
int			g_vermaj	= 3;
int			g_vermin	= 0;

HICON		g_hIcon		= NULL;
HINSTANCE	g_hInstance = NULL;

static ERRCODE			RunSetupWizard		(Bpersist* g_persist);


#if defined(Windows)&&defined(_DEBUG)
#define DBG_MEM_USE 1
	#ifdef DBG_MEM_USE
	#include <crtdbg.h>
		_CrtMemState memstate;
	#endif
#endif


class BcalcApp : public BappFrame
{
public:
	BcalcApp(HINSTANCE hi) : BappFrame(hi) { };
public:
	virtual LRESULT OnMessage(HWND, UINT, WPARAM, LPARAM);
};

BcalcApp*	g_bcalcApp	= NULL;
Bcalc*		g_bcalc		= NULL;
Bpersist*	g_persist	= NULL;
BappPanel*	g_panel		= NULL;

//***********************************************************************
BOOL CALLBACK AboutBcalc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:

		return TRUE;

	case WM_COMMAND:

		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
    return FALSE;
}

extern "C" {
	
//**************************************************************************
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	//LPSTR	lpCmd;
	RECT	rc;
	MSG		msg;
	int		wx, wy, ww, wh;
	ERRCODE ec;

	int scAppBkg = COLOR_BACKGROUND;

	bool isdark = true;
	
#ifdef DBG_MEM_USE
	_CrtMemCheckpoint(&memstate);
#endif
	//_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_CHECK_ALWAYS_DF);

	g_hInstance = hInstance; 

	if (isdark)
	{
		scAppBkg = COLOR_3DDKSHADOW;
	}
	// Ensure that the common control DLL is loaded. 
	InitCommonControls(); 
	
	// get size of screen to form default window
	//
	GetClientRect(GetDesktopWindow(), &rc);
	
	// get window size from persistance
	//
	g_persist = new Bpersist(_T("bcalc"), _T("BSA Software"));
	g_bcalcApp= new BcalcApp(hInstance);
	g_panel   = new BappPanel(_T("bcalc"), g_bcalcApp);
	g_bcalc   = new Bcalc(g_persist, g_panel, isdark);

	g_bcalc->RestoreSettings();

	ww = 4 * (rc.right - rc.left) / 12;
	wh = 4 * (rc.bottom - rc.top) / 10;
	wy = ((rc.bottom - rc.top) - wh) / 2;
	wx = 30;
	if (ww > 480)
		ww = 480;
	if (wh > 260)
		wh = 260;

	if(g_persist)
	{
		g_persist->GetNvInt(_T("Appl X"), wx, wx);
		g_persist->GetNvInt(_T("Appl Y"), wy, wy);
		g_persist->GetNvInt(_T("Appl W"), ww, ww);
		g_persist->GetNvInt(_T("Appl H"), wh, wh);
	}
	ec = g_bcalcApp->Init(hInstance,
				MAKEINTRESOURCE(0),
				MAKEINTRESOURCE(IDI_BCALCICON),
				MAKEINTRESOURCE(IDI_BCALCICON),
				scAppBkg);
	if(ec != errOK)
	{
		int err = GetLastError();
		return FALSE;
	}
	ec = g_bcalcApp->Open(_T("bcalc 6.0"), wx, wy, ww, wh);
	g_bcalcApp->Dock(g_panel, frRight, NULL, true, true);
	g_panel->ActivatePane(g_bcalc);
	SetFocus(g_bcalc->GetWindow());
	if(ec != errOK)
	{
		int err = GetLastError();
		return FALSE;
	}
	while(GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	delete g_bcalcApp;
	delete g_persist;
	g_persist = NULL;

#ifdef DBG_MEM_USE
	_CrtMemDumpAllObjectsSince(&memstate);
#endif
	return 0;
}
}

//**************************************************************************
LRESULT BcalcApp::OnMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC			hdc;
	BcalcApp*	pc;
	RECT		rc;
	bool		bShowit = true;

	pc = (BcalcApp*)GetWindowLong(hWnd, GWL_USERDATA);

	switch (message) 
	{
	case WM_CREATE:

		{ 
			LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;

			SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)pcs->lpCreateParams);
		}
		break;

	case WM_COMMAND:

		break;

	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);

		GetClientRect(hWnd, &rc);
		EndPaint(hWnd, &ps);
		break;

	case WM_SIZE:
	case WM_MOVE:

		if(wParam == SIZE_MINIMIZED)
			return FALSE;

		if(g_bcalc)
		{
			RECT		rc;

			GetClientRect(hWnd, &rc);

			if(rc.bottom - rc.top < 32 || rc.right - rc.left < 32)
				return FALSE;

			GetWindowRect(hWnd, &rc);

			if(g_persist != NULL)
			{
				int w, h;
				bool forceprimary = false;

				w = rc.right - rc.left;
				h = rc.bottom - rc.top;

				if(w < 32) w = 32;
				if(h < 32) h = 32;
		
				if(forceprimary)
				{
					// this opens the window in the primary screen
					// regardless of where it closed
					//
					if(rc.top  < 0) { rc.top  = 0; rc.bottom = h; }
					if(rc.left < 0) { rc.left = 0; rc.right = w;  }
				}
				if((rc.right  - rc.left) < 32) rc.right  = rc.left + 32;
				if((rc.bottom - rc.top)  < 32) rc.bottom = rc.top  + 32;

				g_persist->SetNvInt(_T("Appl X"), rc.left);
				g_persist->SetNvInt(_T("Appl Y"), rc.top);
				g_persist->SetNvInt(_T("Appl W"), w);
				g_persist->SetNvInt(_T("Appl H"), h);

				// do the panel processing for wm_size
				BappPanel::OnMessage(hWnd, message, wParam, lParam);
			}
		}
		break;

	case WM_SETFOCUS:

		break;

	case WM_KILLFOCUS:

		break;

	case WM_DESTROY:

		if(g_bcalc)
			DestroyWindow(g_bcalc->GetWindow());
		PostQuitMessage(0);
		break;

	case WM_INITMENU:
		
		break;
		
	case WM_INITMENUPOPUP:

		break;

	case WM_LBUTTONDOWN:

		SetFocus(hWnd);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

