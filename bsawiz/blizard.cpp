#include "blizardx.h"

HINSTANCE gbliz_hInstance;

#if defined(WIN32)&&defined(BSAWIZ_DLL)
//***********************************************************************
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			gbliz_hInstance = (HINSTANCE)hModule;
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

#endif

#ifdef WIN32
#define RUNAS_DIALOG 1
#endif

//***********************************************************************
static 
#ifdef RUNAS_DIALOG
BOOL
#else
LRESULT
#endif
CALLBACK BlizProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Bwizard* 	pwiz;
	int 		wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC 		hdc;

	pwiz = (Bwizard*)GetWindowLong(hWnd, GWL_USERDATA);

	switch (message) 
	{
	#ifdef RUNAS_DIALOG
	case WM_INITDIALOG:
		return TRUE;
	#endif
	case WM_COMMAND:

		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 

		switch(wmId)
		{
		case IDC_BACK:
			if(pwiz) pwiz->StepBack();
			break;
		case IDC_NEXT:
			if(pwiz) pwiz->StepForward();
			break;
		case IDC_CANCEL:
			{
				HWND  bwnd = GetDlgItem(hWnd, IDC_CANCEL);
				TCHAR wt[256];
				
				wt[0] = _T('\0');
				GetWindowText(bwnd, wt, sizeof(wt)/sizeof(TCHAR));
				if(pwiz) pwiz->End((_tcscmp(wt, _T("Cancel")) == 0) ? errFAILURE : errOK);
			}
			pwiz = NULL;
			break;
		default:
			if(pwiz)
			{
				if(wmEvent == 0)
					pwiz->OnControl(wmId);
				break;
			}
			#ifdef RUNAS_DIALOG
			return FALSE;
			#else
			return DefWindowProc(hWnd, message, wParam, lParam);
			#endif
		}
		break;

	case WM_KEYDOWN:
		
		switch(wParam)
		{
		case VK_TAB:
			pwiz->OnKey(wParam);
			break;
			
		default:
			pwiz->OnKey(wParam);
			break;
		}
		break;
		
	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		{
			static HFONT nf = NULL, of = NULL;
			int nx, ny, dx, dy;

			// fill middle region gray and add line on its top
			RECT   rt;
			HBRUSH hbr;

			if(pwiz && (pwiz->CurrentStep() > 1))
			{
				nf = pwiz->GetNameFont();

				// fill top white
				hbr = GetSysColorBrush(COLOR_WINDOW);
				GetClientRect(hWnd, &rt);
			    ny = rt.bottom;
				rt.bottom = rt.top + 58;
				FillRect(hdc, &rt, hbr);
				rt.bottom = ny;

				rt.top += 58;
				FillRect(hdc, &rt, hbr);
				// fill upper region gray and add line on its top
				hbr = GetSysColorBrush(COLOR_BTNFACE);
				GetClientRect(hWnd, &rt);
				rt.top += 58;
				FillRect(hdc, &rt, hbr);

				hbr = (HBRUSH)GetStockObject(GRAY_BRUSH);
				rt.bottom = rt.top + 1;
				FillRect(hdc, &rt, hbr);

				hbr = (HBRUSH)GetStockObject(WHITE_BRUSH);
				rt.top+= 1;
				rt.bottom = rt.top + 1;
				FillRect(hdc, &rt, hbr);

				// place name and description on top
				//
				nx = 10;
				ny = 10;
				dx = 40;
				dy = 30;
			}
			if(pwiz)
			{
				GetClientRect(hWnd, &rt);
				rt.top = rt.bottom - 48;

				// fill bottom region gray
				if(pwiz->CurrentStep() <= 1)
				{
					nf = pwiz->GetTitleFont();

					// fill top white
					hbr = GetSysColorBrush(COLOR_WINDOW);
					GetClientRect(hWnd, &rt);
					rt.bottom -= 48;
					FillRect(hdc, &rt, hbr);
					rt.bottom += 48;
					rt.top = rt.bottom - 48;

					hbr = GetSysColorBrush(COLOR_BTNFACE);
					FillRect(hdc, &rt, hbr);

					// place name and description in middle
					//
					nx = 166;
					ny = 40;
					dx = 166;
					dy = 80;
				}
				hbr = (HBRUSH)GetStockObject(GRAY_BRUSH);
				rt.bottom = rt.top + 1;
				FillRect(hdc, &rt, hbr);

				hbr = (HBRUSH)GetStockObject(WHITE_BRUSH);
				rt.top+= 1;
				rt.bottom = rt.top + 1;
				FillRect(hdc, &rt, hbr);

			}
			if(nf)
				of = (HFONT)SelectObject(hdc, nf);
			TextOut(hdc, nx, ny, pwiz->GetStepName(), _tcslen(pwiz->GetStepName()));
			if(pwiz && pwiz->GetDescFont()) SelectObject(hdc, pwiz->GetDescFont());
			TextOut(hdc, dx, dy, pwiz->GetDescName(), _tcslen(pwiz->GetDescName()));
			if(pwiz && (pwiz->CurrentStep() <= 1))
				TextOut(hdc, dx, dy + 20, _T("Click Next to continue"), 22);
			if(nf)
				SelectObject(hdc, of);
		}
		EndPaint(hWnd, &ps);
		break;

	case WM_CLOSE:

		if(pwiz) pwiz->End(errFAILURE);
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:

		break;

	default:

		#ifdef RUNAS_DIALOG
		return FALSE;
		#else
		return DefWindowProc(hWnd, message, wParam, lParam);
		#endif
   }
	#ifdef RUNAS_DIALOG
   return TRUE;
	#else
   return 0;
	#endif
}

//***********************************************************************
static LRESULT CALLBACK BitmapProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HBITMAP 	hBM;
	PAINTSTRUCT ps;
	HDC 		hdc;

	hBM = (HBITMAP)GetWindowLong(hWnd, GWL_USERDATA);

	switch (message) 
	{
		
	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		if(hdc && hBM)
		{
			HDC     hdcc = CreateCompatibleDC(hdc);
			HBITMAP ho = (HBITMAP)SelectObject(hdcc, hBM);
			BITMAP	bmp;

			if(GetObject(hBM, sizeof(bmp), &bmp))
			{
				BitBlt(hdc, 0, 0, bmp.bmWidth, bmp.bmHeight, hdcc, 0, 0, SRCCOPY);
			}
			SelectObject(hdcc, ho);
			DeleteDC(hdcc);
		}
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:

		break;

	default:

		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

//***********************************************************************
static LRESULT CALLBACK IconProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HICON		hIcon;
	PAINTSTRUCT ps;
	HDC 		hdc;

	hIcon = (HICON)GetWindowLong(hWnd, GWL_USERDATA);

	switch (message) 
	{
		
	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		DrawIcon(hdc, 0, 0, hIcon);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:

		break;

	default:

		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}



//***********************************************************************
Bwizard::Bwizard()
		:
		m_status(errOK),
		m_cookie(NULL),
		m_step(0),
		m_done(false),
		m_next_step(0),
		m_from_step(0),
		m_ptr(NULL),
		m_script(NULL),
		m_default_mask(0),
		m_enabled_mask(0),
		m_items(NULL),
		m_curedit(NULL),
		m_defaultIconID(0),
		m_defaultBitmapID(0),
		m_panw(0),
		m_panh(0)
{
	m_title[0] = _T('\0');

	m_fh[0] = 22;
	m_fh[1] = 16;
	m_fh[2] = 14;

	m_hfName = NULL;
	m_hfBig  = NULL;
	m_hfDesc = NULL;
	
	m_hInstance = gbliz_hInstance;
	m_hwndPanel = NULL;
}

//***********************************************************************
Bwizard::~Bwizard()
{
	if(m_script)
		delete [] m_script;
	if(m_hwndPanel)
	{
		DestroyWindow(m_hwndPanel);
		m_hwndPanel = NULL;
	}
	if(m_items)
		m_items = BwizList::FreeList(m_items);
	DeleteObject(m_hfBig);
	DeleteObject(m_hfName);
	DeleteObject(m_hfDesc);
}

//***********************************************************************
ERRCODE Bwizard::Begin(LPCTSTR title, LPCTSTR script, BWIZCALLBACK callback, LPVOID cookie, HINSTANCE hInstance)
{
	ERRCODE ec;

	if(! title || ! script) 
		return errBAD_PARAMETER;

	m_done   = false;
	m_cookie = cookie;
	
	m_hInstance = hInstance;

	m_script = new TCHAR [ _tcslen(script) + 4 ];
	_tcscpy(m_script, script);

	_tcscpy(m_title, title);
	m_script = m_script;
	m_ptr    = m_script;

	m_callback = callback;

	// parse till "Begin"
	//
	m_ptr--;
	do
	{
		m_ptr++;
		while(*m_ptr && *m_ptr != _T('B'))
			m_ptr++;
		if(! *m_ptr)
		{
			delete [] m_script;
			return errFAILURE;
		}
	}
	while(_tcsncmp(m_ptr, _T("Begin"), 5) != 0);

	m_from_step = 1;
	m_next_step = 1;
	m_step      = 1;

	if(! m_hwndPanel)
	{
		WNDCLASS	wc;

		// register app class
		//
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= (WNDPROC)BlizProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= m_hInstance;
		wc.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(101));
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= _T("blizpanel");

		int e;
		int rc = RegisterClass(&wc);
		if(rc == 0)
		{
			e = GetLastError();
			if(e != ERROR_CLASS_ALREADY_EXISTS)
				return errFAILURE;
		}

		// register bitmap window class
		//
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= (WNDPROC)BitmapProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= m_hInstance;
		wc.hIcon			= NULL;
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= _T("blizbitmap");

		rc = RegisterClass(&wc);
		if(rc == 0)
		{
			e = GetLastError();
			if(e != ERROR_CLASS_ALREADY_EXISTS)
				return errFAILURE;
		}
		// register icon window class
		//
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= (WNDPROC)IconProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= m_hInstance;
		wc.hIcon			= NULL;
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= _T("blizicon");

		rc = RegisterClass(&wc);
		if(rc == 0)
		{
			e = GetLastError();
			if(e != ERROR_CLASS_ALREADY_EXISTS)
				return errFAILURE;
		}

		RECT	rScreen;
		
		GetWindowRect(GetDesktopWindow(), &rScreen);

		int w = m_panw > 0 ? m_panw : 540;
		int h = m_panh  > 0 ? m_panh : 380;

		int x = rScreen.right /2 - w / 2;
		int y = rScreen.bottom / 2 - h / 2;

#ifdef RUNAS_DIALOG

		BYTE			db[2048];
		LPDLGTEMPLATE 	ptemp;
		LPWORD			ps;
		LPTSTR			pt;
		short			hu, vu;
	
		hu = (short)LOWORD(GetDialogBaseUnits());
		vu = (short)HIWORD(GetDialogBaseUnits());

		// create a dialog template
		//
		ptemp = (LPDLGTEMPLATE)db;
		
		ptemp->style           = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		ptemp->dwExtendedStyle = 0;
		ptemp->cdit = 0;
		ptemp->x 	= 4*x / hu;
		ptemp->y 	= 8*y / vu;
		ptemp->cx 	= 4*w / hu;
		ptemp->cy 	= 8*h / vu;
		
		ps = (LPWORD)(db + 18);
		
		*ps++ = 0;	// menu
		*ps++ = 0;	// class
		
		// title;
		for(pt = (LPTSTR)m_title; pt && *pt;)
			*ps++ = (WORD)*pt++;
		*ps++ = 0;
		
		// DWORD align
		while((DWORD)ps & 3)
			ps++;

		m_hwndPanel = CreateDialogIndirect(m_hInstance, ptemp, NULL, BlizProc);

#else

		m_hwndPanel = CreateWindow(
								_T("blizpanel"),
								m_title, 
								WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
								x, y, 
								w, h,
								NULL, NULL,
								m_hInstance,
								this
								);
#endif
		if(! m_hwndPanel)
		{
			e = GetLastError();
			return errFAILURE;
		}
		RECT rPanel;

		SetWindowLong(m_hwndPanel, GWL_USERDATA, (LONG)this);
		GetClientRect(m_hwndPanel, &rPanel);
		
		x = 240;
		y = rPanel.bottom - rPanel.top - 24 - 10;
		w = 80;
		h = 24;

		m_hwndBack = CreateWindow(
								_T("Button"),
								_T("< Back"), 
								WS_CHILD | ((m_default_mask & EB_BACK) ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON),
								x, y, 
								w, h,
								m_hwndPanel,
								NULL,
								m_hInstance,
								NULL
								);

		SetWindowLong(m_hwndBack, GWL_ID, IDC_BACK);
		ShowWindow(m_hwndBack, SW_SHOW);

		x = 320;
		w = 80;
		h = 24;

		m_hwndNext = CreateWindow(
								_T("Button"),
								_T("Next >"), 
								WS_CHILD | ((m_default_mask & EB_NEXT) ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON),
								x, y, 
								w, h,
								m_hwndPanel,
								NULL,
								m_hInstance,
								NULL
								);

		SetWindowLong(m_hwndNext, GWL_ID, IDC_NEXT);
		ShowWindow(m_hwndNext, SW_SHOW);

		x = 420;
		w = 80;
		h = 24;

		m_hwndCancel = CreateWindow(
								_T("Button"),
								_T("Cancel"), 
								WS_CHILD | ((m_default_mask & EB_CANCEL) ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON),
								x, y, 
								w, h,
								m_hwndPanel,
								NULL,
								m_hInstance,
								NULL
								);

		SetWindowLong(m_hwndCancel, GWL_ID, IDC_CANCEL);
		ShowWindow(m_hwndCancel, SW_SHOW);

	//	SetWindowText(m_hwndPanel, m_title);

		m_hfBig  = CreateFont(
							m_fh[0],0,0,0,700,FALSE,FALSE,FALSE,ANSI_CHARSET,
							OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY,FF_SWISS,_T("Arial"));

		m_hfName = CreateFont(
							m_fh[1],0,0,0,500,FALSE,FALSE,FALSE,ANSI_CHARSET,
							OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY,FF_SWISS,_T("Arial"));

		m_hfDesc = CreateFont(
							m_fh[2],0,0,0,400,FALSE,FALSE,FALSE,ANSI_CHARSET,
							OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY,FF_SWISS,_T("Arial"));


		
		ShowWindow(m_hwndPanel, SW_SHOW);
		UpdateWindow(m_hwndPanel);
	}

	// open panel 1
	//
	ec = StepForward();
	
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		#ifdef RUNAS_DIALOG
		if(! m_hwndPanel || ! IsDialogMessage(m_hwndPanel, &msg))
		#endif
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if(m_done) break;
	}
	ec = GetStatus();
	if(m_script)
	{
		delete [] m_script;
		m_script = NULL;
	}
	if(m_items)
		m_items = BwizList::FreeList(m_items);
	return ec;
}

//***********************************************************************
ERRCODE Bwizard::StepForward()
{
	BwizList* pList;
	BwwiItem* pItem;
	bool	  setnext = false;

	for(pList = m_items; pList; pList = pList->GetNext(pList))
	{
		pItem = pList->GetValue();
		if(pItem->f_type == wwiChoice && pItem->f_panel >= 0)
		{
			if((pItem->f_ival = SendMessage(pItem->f_hwnd, BM_GETCHECK, 0, 0)) != 0)
			{
				if(pItem->f_panel > 0)
				{
					m_next_step = pItem->f_panel;
					setnext = true;
				}
				else
				{
					End(errFAILURE);
					return errFAILURE;
				}
			}
		}
	}
	SendPanel();

	// ask owner how to step
	if(m_callback)
	{
		int step = m_next_step;

		if(m_callback(m_cookie, &step, wizStepForward, NULL, NULL))
		{
			if(step <= 0)
			{
				End(errFAILURE);
				return errFAILURE;
			}
			m_next_step = step;
		}
	}
	return Step(m_step, m_next_step);
}

//***********************************************************************
ERRCODE Bwizard::StepBack()
{
	ERRCODE ec = Step(m_step, m_from_step);
	if(ec == errOK)
		SendPanel();
	return ec;
}

//***********************************************************************
int Bwizard::ScanIntParm(LPCTSTR pp, int def)
{
	if(pp && *pp == _T('('))
	{
		return _tcstol(pp+1, NULL, 0);
	}
	return def;
}

//***********************************************************************
LPCTSTR Bwizard::ScanStringParm(LPTSTR pr, LPCTSTR pp, LPCTSTR def)
{
	TCHAR* sp;
	TCHAR* dp;

	_tcscpy(pr, def);

	if(! pp || *pp != _T('('))
		return pr;

	for(sp = (LPTSTR)pp+1, dp = pr; *sp && *sp != _T(')') && (dp - pr) < MAX_PATH-4;)
		*dp++ = *sp++;
	*dp = _T('\0');
	return pr;
}

//***********************************************************************
ERRCODE Bwizard::ScanToken()
{
	TCHAR* sp;
	TCHAR* dp;
	TCHAR* ap;

	m_atype[0]		= _T('\0');
	m_aname[0]		= _T('\0');
	m_avar[0]		= _T('\0');
	m_awidth		= 0;
	m_aheight		= 0;
	m_asameline		= 0;
	m_afontnum		= 2;
	m_aisdefault	= 0;
	m_apanel		= -1;
	m_anumeric		= 0;
	m_apasswd		= 0;

	if(! m_ptr || ! *m_ptr) return errFAILURE;

	// type[attrib(va),attrib...]:name:

	dp = m_atype;
	sp = (LPTSTR)m_ptr;

	while(*sp && (*sp == _T(':') || *sp == _T(';') || *sp == _T(' ') || *sp == (',') || *sp == _T('\t')))
		sp++;

	while(*sp && *sp != _T(':') && *sp != _T(';') && ((dp - m_atype) < (MAX_PATH - 4)))
		*dp++ = *sp++;
	*dp = _T('\0');

	if((ap = _tcsstr(m_atype, _T("["))) != NULL)
	{
		if((dp = _tcsstr(ap, _T("default"))) != NULL)
			m_aisdefault = ScanIntParm(dp+7, 1);
		if((dp = _tcsstr(ap, _T("font"))) != NULL)
			m_afontnum = ScanIntParm(dp+4, 0);
		if((dp = _tcsstr(ap, _T("panel"))) != NULL)
			m_apanel = ScanIntParm(dp+5, m_step);
		if((dp = _tcsstr(ap, _T("var"))) != NULL)
			ScanStringParm(m_avar, dp+3, _T(""));
		if((dp = _tcsstr(ap, _T("width"))) != NULL)
			m_awidth = ScanIntParm(dp+5, 0);
		if((dp = _tcsstr(ap, _T("height"))) != NULL)
			m_aheight = ScanIntParm(dp+6, 0);
		if((dp = _tcsstr(ap, _T("sameline"))) != NULL)
			m_asameline = ScanIntParm(dp+8, 1);
		if((dp = _tcsstr(ap, _T("numeric"))) != NULL)
			m_anumeric = ScanIntParm(dp+7, 1);
		if((dp = _tcsstr(ap, _T("password"))) != NULL)
			m_apasswd = ScanIntParm(dp+8, 1);
		*ap = _T('\0');
	}
	m_ptr = *sp ? sp+1 : sp;
	if(*sp && *sp == _T(';'))
		return errOK;
	if(*sp && *sp == _T(':'))
		sp++;
	dp = m_aname;
	while(*sp && *sp != _T(':') && *sp != _T(';') && ((dp - m_aname) < (MAX_PATH - 4)))
		*dp++ = *sp++;
	*dp = _T('\0');
	m_ptr = *sp ? sp+1 : sp;
	return errOK;
}

//***********************************************************************
ERRCODE Bwizard::Step(int from, int to)
{
	int panel;

	// reset init coords for next panel
	//
	m_ix = 166;
	m_iy = 80;
	m_iw = 300;
	m_ih = 32;

	m_items = BwizList::FreeList(m_items);

	m_from_step = from > to ? to - 1 : from;
	m_step		= to;
	m_next_step = to + 1;

	// scan script for Panel:step
	//
	m_ptr = m_script;

	do
	{
		do
		{
			if(ScanToken() != errOK) return errFAILURE;
		}
		while(_tcscmp(m_atype, _T("Panel")) != 0);

		panel = m_apanel;
	}
	while(panel < m_step);

	m_panw = m_awidth;
	m_panh = m_aheight;

	if(panel != m_step)
		return errFAILURE;

	// get panel spec
	//
	//
	m_name[0]		= _T('\0');
	m_desc[0]		= _T('\0');
	m_enabled_mask	= 0;
	m_default_mask	= 0;
	m_ctrl_id		= IDC_FIRST_CONTROL;
	_tcscpy(m_name, m_aname);

	// d) scan for panel attributes
	//

	do
	{
		ScanToken();

		switch(*m_atype)
		{
		case _T('\0'):
			return errFAILURE;

		case _T('E'):
			if(! _tcscmp(m_atype, _T("Edit")))
			{
				AddItem(wwiEdit, m_name);
			}
			break;

		case _T('B'):
			if(! _tcsncmp(m_atype, _T("Back"), 4))
			{
				m_enabled_mask |= EB_BACK;
				if(m_apanel > 0)
					m_from_step = m_apanel;
			}
			break;

		case _T('D'):
			if(! _tcscmp(m_atype, _T("Description")))
			{
				_tcscpy(m_desc, m_aname);
			}
			break;

		case _T('I'):
			if(! _tcscmp(m_atype, _T("IconImage")))
			{
				AddItem(wwiIconBitmap, m_aname);
			}
			break;

		case _T('L'):
			if(! _tcsncmp(m_atype, _T("Label"), 5))
			{
				AddItem(wwiLabel, m_name);
			}
			else if(! _tcscmp(m_atype, _T("LeftImage")))
			{
				AddItem(wwiInitialBitmap, m_aname);
			}
			break;

		case _T('N'):
			if(! _tcsncmp(m_atype, _T("Next"), 4))
			{
				m_enabled_mask |= EB_NEXT;
				if(m_apanel > 0)
					m_next_step = m_apanel;
			}
			break;

		case _T('C'):
			if(! _tcsncmp(m_atype, _T("Cancel"), 6))
			{
				m_enabled_mask |= EB_CANCEL;
			}
			else if(! _tcscmp(m_atype, _T("ChoiceGroup")))
			{
				AddItem(wwiChoiceGroup, m_name);
			}
			else if(! _tcscmp(m_atype, _T("Choice")))
			{
				AddItem(wwiChoice, m_name);
			}
			else if(! _tcscmp(m_atype, _T("Checkbox")))
			{
				AddItem(wwiCheckbox, m_name);
			}
			else if(! _tcscmp(m_atype, _T("Condition")))
			{
				TCHAR  val[512];

				val[0] = _T('\0');
				if(m_callback && m_callback(m_cookie, &m_step, wizGetValue, m_avar, val))
				{
					bool set = _tcstol(val, NULL, 0) != 0;

					if(set)
					{
						if(m_apanel >= 0 && m_apanel != m_step)
						{
							return Step(m_step, m_apanel);
						}
					}
				}
				return Step(m_step, m_next_step);
			}
			break;

		case _T('F'):
			if(! _tcsncmp(m_atype, _T("Finish"), 6))
			{
				m_enabled_mask |= EB_FINISH;
			}
			break;

		default:
			break;
		}
	}
	while((_tcscmp(m_atype, _T("EndPanel")) != 0)&&(_tcscmp(m_atype, _T("Panel")) != 0));

	// c) for each added item, get its value
	//
	BwizList* pItem;

	for(pItem = m_items; pItem; pItem = pItem->GetNext(pItem))
	{
		GetControl(_tcstol(pItem->GetName(), NULL, 0));
	}

	// d) open the panel window
	//
	Panel(m_step, m_name);

	return errOK;
}

//***********************************************************************
ERRCODE Bwizard::Panel(int step, LPCTSTR name)
{
	BwizList*	pItem;
	
	m_default_mask = EB_NEXT;

	// set default and enabled state of buttons
	//
	EnableWindow(m_hwndBack,   (m_enabled_mask & EB_BACK));
	EnableWindow(m_hwndNext,   (m_enabled_mask & EB_NEXT));
	EnableWindow(m_hwndCancel, (m_enabled_mask & (EB_CANCEL|EB_FINISH)));

	long style;

	style = GetWindowLong(m_hwndBack, GWL_STYLE);
	style &= ~ BS_DEFPUSHBUTTON;
	style |= (m_default_mask & EB_BACK) ? BS_DEFPUSHBUTTON : 0;
	SetWindowLong(m_hwndBack, GWL_STYLE, style);
	UpdateWindow(m_hwndBack);

	style = GetWindowLong(m_hwndNext, GWL_STYLE);
	style &= ~ BS_DEFPUSHBUTTON;
	style |= (m_default_mask & EB_NEXT) ? BS_DEFPUSHBUTTON : 0;
	SetWindowLong(m_hwndNext, GWL_STYLE, style);
	UpdateWindow(m_hwndNext);

	style = GetWindowLong(m_hwndCancel, GWL_STYLE);
	style &= ~ BS_DEFPUSHBUTTON;
	style |= (m_default_mask & EB_CANCEL) ? BS_DEFPUSHBUTTON : 0;
	SetWindowLong(m_hwndCancel, GWL_STYLE, style);
	UpdateWindow(m_hwndCancel);

	if(m_enabled_mask & EB_FINISH)
		SetWindowText(m_hwndCancel, _T("Finish"));
	else
		SetWindowText(m_hwndCancel, _T("Cancel"));

	// hand focus to the first editbox in the panel,
	// or the panel itself
	//
	for(pItem = m_items, m_curedit = NULL; pItem; pItem = pItem->GetNext(pItem))
	{
		BwwiItem* item = pItem->GetValue();
		if(! item) break;

		if(item->f_type == wwiEdit)
		{
			m_curedit = pItem;
			SetFocus(item->f_hwnd);
			break;
		}
	}
	if(! pItem)
		SetFocus(m_hwndPanel);
	
	InvalidateRect(m_hwndPanel, NULL, false);
	return errOK;
}

//***********************************************************************
ERRCODE Bwizard::End(ERRCODE ecode)
{
	m_status = ecode;
	DestroyWindow(m_hwndPanel);
	m_done = true;
	return errOK;
}

//***********************************************************************
ERRCODE Bwizard::AddItem(BwizItemType type, LPCTSTR name)
{
	BwizList* item;
	TCHAR	  winname[32];
	TCHAR	  itemname[32];
	LPCTSTR   szClass;
	long	  style;
	HWND	  hwnd;
	HBITMAP   hbm;
	HICON	  hico;
	int		  fn, fh;
	int		  previx, previy;
	int		  xi,yi,iw, xo;

	fn      = m_afontnum;

	if(fn < 0) fn = 0;
	if(fn > 2) fn = 2;

	HFONT font;

	switch(fn)
	{
	case 0: font = m_hfBig; break;
	default:
	case 1: font = m_hfName; break;
	case 2: font = m_hfDesc; break;
	}
	fh = m_fh[fn];

	if(! name) return errBAD_PARAMETER;

	_sntprintf(winname, 32, _T("wwiw%d"), m_ctrl_id);
	_sntprintf(itemname, 32, _T("%d"), m_ctrl_id);

	item    = new BwizList(itemname, new BwwiItem(NULL));
	item->GetValue()->f_ival = 0;
	item->GetValue()->f_sval[0] = _T('\0');
	item->GetValue()->f_type = type;
	item->GetValue()->f_panel = m_apanel;

	_tcscpy(item->GetValue()->f_varname, m_avar);

	switch(type)
	{
	case wwiLabel:
		szClass = _T("Static");
		style = WS_CHILD | SS_LEFT;
		xi = 0; 
		yi = fh + 2;
		break;
	case wwiChoiceGroup:
		szClass = _T("Static");
		style = WS_CHILD | SS_LEFT | WS_GROUP;
		xi = 0; 
		yi = fh + 8;
		break;
	case wwiChoice:
		szClass = _T("Button");
		style = WS_CHILD | BS_AUTORADIOBUTTON | WS_TABSTOP;
		xi = 0; 
		yi = fh + 6;
		break;
	case wwiCheckbox:
		szClass = _T("Button");
		style = WS_CHILD | BS_AUTOCHECKBOX | WS_TABSTOP;
		xi = 0; 
		yi = fh + 6;
		break;
	case wwiEdit:
		szClass = _T("Edit");
		style = WS_CHILD | WS_BORDER | WS_TABSTOP;
		if(m_anumeric) style |= ES_NUMBER;
		if(m_apasswd)  style |= ES_PASSWORD;
		xi = 0; 
		yi = fh + 6;
		break;
	case wwiInitialBitmap:
	case wwiIconBitmap:
		szClass = (type == wwiIconBitmap) ? _T("blizicon") : _T("blizbitmap");
		style = WS_CHILD | SS_LEFT;
		previy = m_iy;
		previx = m_ix;
		xi = xo = 0;
		yi = m_iw = 0;
		hico = NULL;
		hbm  = NULL;
		{
			DWORD idb;
			
			if(name && name[0])
				idb = _tcstol(name, NULL, 0);
			else
				idb = (type == wwiIconBitmap) ? m_defaultIconID : m_defaultBitmapID;

			if(type == wwiIconBitmap)
			{
//				hico = LoadIcon(m_hInstance, MAKEINTRESOURCE((intptr_t)idb));
				hico = (HICON)LoadImage(m_hInstance, MAKEINTRESOURCE((intptr_t)idb), IMAGE_ICON, 32, 32, 0);
				if(hico)
				{
					ICONINFO ii;

					GetIconInfo(hico, &ii);

					hbm = ii.hbmColor;
				}
			}
			else
			{
				hbm = (HBITMAP)LoadBitmap(m_hInstance, MAKEINTRESOURCE((intptr_t)idb));
			}
			if(hbm)
			{
				BITMAP bmp;

				GetObject(hbm, sizeof(bmp), &bmp);

				m_iw = xi = bmp.bmWidth;
				yi   = bmp.bmHeight;
			}
		}
		if(type == wwiIconBitmap)
		{
			RECT rcp;
			
			GetClientRect(m_hwndPanel, &rcp);

			m_ix = rcp.right - m_iw - 32;
			m_iy = (56 - yi) / 2;
			if(m_iy < 0) m_iy = 0;
		}
		else
		{
			m_ix = 0;
			m_iy = 0;
		}
		break;
	}
	if(m_asameline && m_iy > 0)
		m_iy -= yi;

	if(m_awidth)
		iw = m_awidth;
	else
		iw = m_iw;

	if(m_asameline)
		xo = m_asameline;
	else
		xo = 0;

	item->GetValue()->f_hwnd = hwnd = 
		CreateWindow(
						szClass,
						winname,
						style,
						m_ix + xo, m_iy, iw, yi,
						m_hwndPanel,
						NULL,
						m_hInstance,
						this
					);

	if(! hwnd)
	{
		int e;

		e = GetLastError();
		return errFAILURE;
	}
	SetWindowLong(hwnd, GWL_ID, m_ctrl_id);
	m_ctrl_id++;
	switch(type)
	{
	case wwiLabel:
		break;
	case wwiChoiceGroup:
		break;
	case wwiChoice:
		if(m_aisdefault)
		{
			item->GetValue()->f_ival = 1;
			SendMessage(hwnd, BM_SETCHECK, 1, TRUE);
		}
		break;
	case wwiEdit:
		yi += 8;
		break;
	case wwiInitialBitmap:
	case wwiIconBitmap:
		SetWindowLong(hwnd, GWL_USERDATA, (type == wwiIconBitmap) ? (LONG)hico : (LONG)hbm);
		m_iy = previy;
		m_ix = previx;
		yi = xi = 0;
		m_iw = 300;
		m_ih = 32;
		break;
	}
	ShowWindow(hwnd, SW_SHOW);
	SetWindowText(hwnd, m_aname);
	SendMessage(hwnd, WM_SETFONT, (WPARAM)(LPARAM)font, 0);
	m_items = BwizList::AddToList(item, m_items);
	m_iy += yi;
	m_ix += xi;
	return errOK;
}

//***********************************************************************
void Bwizard::GetControl(int id)
{
	BwizList* pItem;
	TCHAR     itemname[32];
	TCHAR	  val[5120];

	if(! m_callback) return;
	val[0] = _T('\0');

	_sntprintf(itemname, 32, _T("%d"), id);
	
	if((pItem = BwizList::FindKey(itemname, m_items)) != NULL)
	{
		BwwiItem* item = pItem->GetValue();
		if(! item) return;

		switch(item->f_type)
		{
		case wwiChoice:
		case wwiCheckbox:
			if(m_callback(m_cookie, &m_step, wizGetValue, item->f_varname, val))
			{
				bool set = _tcstol(val, NULL, 0) != 0;

				item->f_ival = set;
				SendMessage(item->f_hwnd, BM_SETCHECK, set, 0);
			}
			break;

		case wwiChoiceGroup:
			return;

		case wwiLabel:
		case wwiEdit:
			if(m_callback(m_cookie, &m_step, wizGetValue, item->f_varname, val))
			{
				_tcscpy(item->f_sval, val);
				SetWindowText(item->f_hwnd, item->f_sval);
			}
			break;
		}
	}
}


//***********************************************************************
void Bwizard::OnKey(WPARAM key)
{
	BwizList* pItem;
	
	switch(key)
	{
	case VK_TAB:
		for(pItem = m_items, m_curedit = NULL; pItem; pItem = pItem->GetNext(pItem))
		{
			BwwiItem* item = pItem->GetValue();
				
			if(item && item->f_type == wwiEdit)
			{
				if(item->f_hwnd == GetFocus())
				{
					m_curedit = pItem;
					break;
				}
			}
		}
		if(m_curedit)
			pItem = m_curedit->GetNext(m_curedit);
		else
			pItem = m_items;
		while(pItem)
		{
			BwwiItem* item = pItem->GetValue();
				
			if(item && item->f_type == wwiEdit)
			{
				m_curedit = pItem;
				SetFocus(item->f_hwnd);
				break;
			}
			pItem = pItem->GetNext(pItem);
			if(! pItem && m_curedit && (m_curedit != m_items))
			{
				m_curedit = NULL;
				pItem = m_items;
			}
		}
		if(! pItem)
			SetFocus(m_hwndPanel);
		break;

	case VK_RETURN:

		if(m_default_mask & EB_NEXT && m_enabled_mask & EB_NEXT)
		{
			PostMessage(m_hwndNext, WM_LBUTTONDOWN, 0, 0);
			PostMessage(m_hwndNext, WM_LBUTTONUP, 0, 0);
		}
		else if(m_default_mask & EB_CANCEL && m_enabled_mask & EB_CANCEL)
		{
			PostMessage(m_hwndCancel, WM_LBUTTONDOWN, 0, 0);
			PostMessage(m_hwndCancel, WM_LBUTTONUP, 0, 0);
		}
		else if(m_default_mask & EB_BACK && m_enabled_mask & EB_BACK)
		{
			PostMessage(m_hwndBack, WM_LBUTTONDOWN, 0, 0);
			PostMessage(m_hwndBack, WM_LBUTTONUP, 0, 0);
		}
		break;

	case VK_ESCAPE:

		End(errFAILURE);
		break;
		
	default:
		break;
	}
}

//***********************************************************************
void Bwizard::OnControl(int id)
{
	BwizList* pItem;
	TCHAR     itemname[32];
	LPTSTR	  val = (LPTSTR)_T("");

	_sntprintf(itemname, 32, _T("%d"), id);
	
	if((pItem = BwizList::FindKey(itemname, m_items)) != NULL)
	{
		BwwiItem* item = pItem->GetValue();
		if(! item) return;

		switch(item->f_type)
		{
		case wwiChoice:
		case wwiCheckbox:
			item->f_ival = SendMessage(item->f_hwnd, BM_GETCHECK, 0, 0);
			val = (LPTSTR)(item->f_ival  ? _T("1") : _T("0"));
			break;

		case wwiLabel:
		case wwiChoiceGroup:
			return;

		case wwiEdit:
			GetWindowText(item->f_hwnd, item->f_sval, MAX_PATH*2-4);
			val = item->f_sval;
			if(! val) val = (LPTSTR)_T("");
			break;
		}
		if(m_callback)
			m_callback(m_cookie, &m_step, wizSetValue, item->f_varname, val);
	}
}

//***********************************************************************
void Bwizard::SendPanel()
{
	BwizList* pItem;

	for(pItem = m_items; pItem; pItem = pItem->GetNext(pItem))
	{
		OnControl(_tcstol(pItem->GetName(), NULL, 0));
	}
}
