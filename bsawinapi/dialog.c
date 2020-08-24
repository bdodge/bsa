//--------------------------------------------------------------------
//
// File: dialog.c
// Desc: dialogs for winapi
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------


#ifndef Windows
#include "winapix.h"

int	_w_dlg_baseunitX = 6;
int _w_dlg_baseunitY = 13;

//-----------------------------------------------------------------------------

HWND	_zg_dialogWnd[BSA_MAX_NESTED_DIALOGS];
int		_zg_dialogTop = -1;

LPZWND	_zg_dialogCtrl = NULL;	// last ctrl that became "current"
BOOL	_zg_keynav 	 = FALSE; 	// true if keyboard navigating dialog ctrls

//**************************************************************************
//**************************************************************************
//**************************************************************************

//**************************************************************************
HWND WINAPI CreateDialogParam(
								    HINSTANCE		hInstance,
									LPCTSTR			lpTemplateName,
								    HWND			hWndParent,
								    DLGPROC			lpDialogFunc,
								    LPARAM			dwInitParam
						 )
{
	LPBYTE  res;
	DWORD	cbdata;
	
	// find dialog in resource file
	//
	res = (LPBYTE)__FindResource(RT_Dialog, lpTemplateName, _zg_resources, _cb_zg_resources);
	
	if(! res) 
	{
		SetLastError(ERROR_RESOURCE_DATA_NOT_FOUND);
		return NULL;
	}
	// extract the resource header info
	//
	cbdata = RESDWORD(res);			// data size
	cbdata = RESDWORD(res);			// hdr size
	res -= 8;
	res += ((cbdata + 3) & ~3);
	
	return CreateDialogIndirectParam(
								hInstance,
								(LPDLGTEMPLATE)res,
								hWndParent,
								lpDialogFunc,
								dwInitParam
							);
}


//**************************************************************************
HWND WINAPI CreateDialogIndirectParam(
								    HINSTANCE		hInstance,
								    LPCDLGTEMPLATE	pDialogTemplate,
								    HWND			hWndParent,
								    DLGPROC			lpDialogFunc,
								    LPARAM			dwInitParam
								 )
{
	LPBYTE  		res;
	LPZWND			zWnd;
	LPZWND			zWndParent;
	HWND			hDlg, hWnd;
	int				i, len;
	BOOL			rv;
	DWORD			isEx;
	
	DLGTEMPLATE 	dlgHdr;
	DLGITEMTEMPLATE itmHdr;
	
	LPTSTR			lpszClass;
	LPTSTR			lpszTitle;
	LPTSTR			lpszMenu;
	LPTSTR			lpszFont;
	LPBYTE			lpCreate;
	WORD			idMenu, idClass, idTitle;
	WORD			wv, pointsize;
	
	TCHAR			classBuff[MAX_PATH];
	TCHAR			titleBuff[MAX_PATH];
	TCHAR			menuBuff[MAX_PATH];
	TCHAR			fontBuff[MAX_PATH];

	// make sure not nested too deep
	//
	if(_zg_dialogTop >= BSA_MAX_NESTED_DIALOGS)
	{
		SetLastError(ERROR_OUT_OF_STRUCTURES);
		return NULL;
	}

	if(! (res = (LPBYTE)pDialogTemplate))
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return NULL;
	}
	isEx =	RESXDWORD(res);
	if(isEx != 0xFFFF0001)
		isEx = 0;
	if(isEx)
	{
		RESDWORD(res);	// skip sig
		RESDWORD(res);	// skip help id
	    dlgHdr.dwExtendedStyle	= RESDWORD(res);
	    dlgHdr.style			= RESDWORD(res);
	    dlgHdr.cdit				= RESWORD(res);
	}
	else
	{
	    dlgHdr.style			= RESDWORD(res);
	    dlgHdr.dwExtendedStyle  = RESDWORD(res);
	    dlgHdr.cdit				= RESWORD(res);
	}
	dlgHdr.x		= RESWORD(res);
	dlgHdr.y		= RESWORD(res);
	dlgHdr.cx		= RESWORD(res);
	dlgHdr.cy		= RESWORD(res);

	// menu
	lpszMenu = _T("");
	idMenu   = 0;
	wv       = RESWORD(res);
	if(wv == 0xffff)
	{
		idMenu   = RESWORD(res);
	}
	else if(wv != 0)
	{
		lpszMenu = menuBuff;
		
		lpszMenu[0] = wv;
		for(len = 1; len < MAX_PATH - 1; len++)
		{
			lpszMenu[len] = RESWORD(res);
			if(lpszMenu[len] == 0)
				break;
		}
		lpszMenu[len] = 0;
	}
	// class
	lpszClass = _T("Dialog");
	idClass   = 0;
	wv        = RESWORD(res);
	if(wv == 0xffff)
	{
		idClass   = RESWORD(res);
	}
	else if(wv != 0)
	{
		lpszClass = classBuff;
		
		lpszClass[0] = (TCHAR)wv;
		for(len = 1; len < MAX_PATH - 1; len++)
		{
			lpszClass[len] = RESWORD(res);
			if(lpszClass[len] == 0)
				break;
		}
		lpszClass[len] = 0;
	}
	// title
	lpszTitle = _T("");
	wv       = RESWORD(res);
	if(wv != 0)
	{
		lpszTitle = titleBuff;

		lpszTitle[0] = (TCHAR)wv;		
		for(len = 1; len < MAX_PATH - 1; len++)
		{
			lpszTitle[len] = RESWORD(res);
			if(lpszTitle[len] == 0)
				break;
		}
		lpszTitle[len] = 0;
	}
	// font
	if(dlgHdr.style & DS_SETFONT)
	{
		DWORD fontweight, fontital, fontcharset;
		
		pointsize = RESWORD(res);
		if(isEx)
		{
			fontweight  = RESWORD(res);
			fontital    = RESBYTE(res);
			fontcharset = RESBYTE(res);
		}
		lpszFont = fontBuff;

		for(len = 0; len < MAX_PATH - 1; len++)
		{
			lpszFont[len] = RESWORD(res);
			if(lpszFont[len] == 0)
				break;
		}
		lpszFont[len] = 0;
	}
	// create a window from this header information
	hDlg = CreateWindowEx(
					dlgHdr.dwExtendedStyle,
					lpszClass,
					lpszTitle,
					dlgHdr.style,
					BSA_SCALE_DLG_X(dlgHdr.x),
					BSA_SCALE_DLG_Y(dlgHdr.y),
					BSA_SCALE_DLG_X(dlgHdr.cx),
					BSA_SCALE_DLG_Y(dlgHdr.cy),
					hWndParent,
					NULL,
					hInstance,
					0
					);

	if(! hDlg)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return NULL;
	}
	SetWindowLong(hDlg, DWL_DLGPROC, (LONG)lpDialogFunc);
	zWndParent = Zwindow(hDlg);
	
	// create all the dialog controls
	//
	for(i = 0; i < dlgHdr.cdit; i++)
	{
		LONG llres = (LONG)res;
		llres += 3;
		llres &= ~3;
		res = (LPBYTE)llres;
		
		lpCreate = NULL;

		if(isEx)
		{
			RESDWORD(res); // help id
			itmHdr.dwExtendedStyle	 = RESDWORD(res);
			itmHdr.style	 = RESDWORD(res);
		}
		else
		{
			itmHdr.style	 = RESDWORD(res);
			itmHdr.dwExtendedStyle	 = RESDWORD(res);
		}
		itmHdr.x		= RESWORD(res);
		itmHdr.y		= RESWORD(res);
		itmHdr.cx		= RESWORD(res);
		itmHdr.cy		= RESWORD(res);
		if(isEx)
		{
			itmHdr.id	= RESDWORD(res);
		}
		else
		{
			itmHdr.id	= RESWORD(res);
		}
		// control class
		wv = RESWORD(res);		
		if(wv  == 0xffff)
		{
			wv = RESWORD(res);
			switch(wv)
			{
			case 0x0080:
				lpszClass = _T("Button"); break;
			case 0x0081:
				if(itmHdr.style & WS_BORDER)
				{
					// I *think* this is what win32 does
					itmHdr.style &= ~ WS_BORDER;
					itmHdr.dwExtendedStyle |= WS_EX_CLIENTEDGE;
				}
				lpszClass = _T("Edit"); break;
		//	case 0x0085:
		//		lpszClass = _T("Combobox"); break;
			default:
				//_tprintf(_T("uc=%d\n"),wv); 
			case 0x0082:
				if(itmHdr.style & WS_BORDER)
				{
					// I *think* this is what win32 does
					itmHdr.style &= ~ WS_BORDER;
					itmHdr.dwExtendedStyle |= WS_EX_CLIENTEDGE;
				}
				lpszClass = _T("Static"); break;
			case 0x0083:
				lpszClass = _T("List box"); break;
			case 0x0084:
				lpszClass = _T("Scrollbar"); break;
			}
		}
		else
		{
			lpszClass = classBuff;

			lpszClass[0] = wv;
			for(len = 1; len < MAX_PATH - 1; len++)
			{
				lpszClass[len] = RESWORD(res);
				if(lpszClass[len] == 0)
					break;
			}
			lpszClass[len] = 0;
			
			if(! _tcsicmp(lpszClass, _T("SysTreeView32")))
			{
				// give treeview windows vertical scrollbars by default
				// until auto-vscrolling works
				itmHdr.style |= WS_VSCROLL;
				
				if(itmHdr.style & WS_BORDER)
				{
					// I *think* this is what win32 does
					itmHdr.style &= ~ WS_BORDER;
					itmHdr.dwExtendedStyle |= WS_EX_CLIENTEDGE;
				}
			}
			else if(! _tcsicmp(lpszClass, TRACKBAR_CLASS))
			{
				;
			}
			//_tprintf(_T("cc=%ls\n"), lpszClass);
		}
		// control title
		lpszTitle = _T("");
		idTitle   = 0;
		wv        = RESWORD(res);
		if(wv == 0xffff)
		{
			idTitle = RESWORD(res);
			
			// hmmm, the title is an ID, not a string, figure
			// out what kind of weird control this is
			//			
			if(! _tcsicmp(lpszClass, _T("Static")))
			{
				HICON   hIcon;
				HBITMAP hBitmap;
				
				if((hIcon = LoadIcon((HINSTANCE)0xbeef, MAKEINTRESOURCE((long)idTitle))) != NULL)
				{
					lpszClass = _T("StaticIcon");
					lpCreate = (LPVOID)hIcon;
				}
				else if((hBitmap = LoadBitmap((HINSTANCE)0xbeef,  MAKEINTRESOURCE((long)idTitle))) != NULL)
				{
					lpszClass = _T("StaticBitmap");
					lpCreate = (LPVOID)hBitmap;
				}
			}
		}
		else if(wv != 0)
		{
			lpszTitle = titleBuff;
			
			lpszTitle[0] = wv;
			for(len = 1; len < MAX_PATH - 1; len++)
			{
				lpszTitle[len] = RESWORD(res);
				if(lpszTitle[len] == 0)
					break;
			}
			lpszTitle[len] = 0;
		}
		len = RESWORD(res);
		if(len > 0)
		{
			lpCreate = res + sizeof(WORD);
			res += len;
		}
		/*
		_tprintf(_T("control=%ls %ls %d,%d %d,%d\n"),
				lpszClass, lpszTitle, itmHdr.x, itmHdr.y, itmHdr.cx, itmHdr.cy);
		*/
		// create a window from this header information
		hWnd = CreateWindowEx(
						itmHdr.dwExtendedStyle,
						lpszClass,
						lpszTitle,
						itmHdr.style,
						BSA_SCALE_DLG_X(itmHdr.x),
						BSA_SCALE_DLG_Y(itmHdr.y),
						BSA_SCALE_DLG_X(itmHdr.cx),
						BSA_SCALE_DLG_Y(itmHdr.cy),
						hDlg,
						NULL,
						hInstance,
						(LPVOID)lpCreate
						);
		if(hWnd)
		{
		    // link into control structure of parent window
		    //
		    if(zWndParent)
		    {
		        LPZWND zCtrl;
		        
				zWnd = (LPZWND)hWnd;
				
		        if(zWndParent->cnext)
		        {
		            for(zCtrl = zWndParent->cnext; zCtrl->cnext;)
		                zCtrl = zCtrl->cnext;
			        zCtrl->cnext = zWnd;
			        zWnd->cprev  = zCtrl;
		        }
		        else
		        {
		            zWnd->cprev = NULL;
		            zWnd->cnext = NULL;
		            zWndParent->cnext = zWnd;
		        }
		    }
			SetWindowLong(hWnd, GWL_ID, itmHdr.id);
		}
	}
	zWnd = Zwindow(hDlg); 

	// set current ctrl to first in tab order
	while(zWnd->cnext)
	{
		if((GetWindowLong(zWnd->cnext, GWL_STYLE) & WS_TABSTOP))
		{
			int idCurrent = GetWindowLong(zWnd->cnext, GWL_ID);
			
			SetWindowLong(hDlg, GWL_ID, idCurrent);
			_zg_dialogCtrl = zWnd->cnext;
			break;
		}
		zWnd = zWnd->cnext;
	}

	// send the init message
	rv = SendMessage(hDlg, WM_INITDIALOG, 0, (LPARAM)dwInitParam);
			
	// if initdialog returns false, proc has set focus by hand, 
	// if true, then we set focus to first control
	//
	if(rv)
	{
		SetFocus(hDlg);
	}
	// show it now
	if(dlgHdr.style & WS_VISIBLE)
		ShowWindow(hDlg, SW_SHOW);

	return hDlg;
}

//**************************************************************************
int WINAPI DialogBoxParam(
								    HINSTANCE		hInstance,
									LPCTSTR			lpTemplateName,
								    HWND			hWndParent,
								    DLGPROC			lpDialogFunc,
								    LPARAM			dwInitParam
						 )
{
	LPBYTE  res;
	DWORD	cbdata;
	
	// find dialog in resource file
	//
	res = (LPBYTE)__FindResource(RT_Dialog, lpTemplateName, _zg_resources, _cb_zg_resources);
	
	if(! res) 
	{
		SetLastError(ERROR_RESOURCE_DATA_NOT_FOUND);
		return -1;
	}
	// extract the resource header info
	//
	cbdata = RESDWORD(res);			// data size
	cbdata = RESDWORD(res);			// hdr size
	res -= 8;
	res += ((cbdata + 3) & ~3);
	
	return DialogBoxIndirectParam(
								hInstance,
								(LPDLGTEMPLATE)res,
								hWndParent,
								lpDialogFunc,
								dwInitParam
							);
}

//**************************************************************************
int WINAPI DialogBoxIndirectParam(
								    HINSTANCE		hInstance,
								    LPCDLGTEMPLATE	pDialogTemplate,
								    HWND			hWndParent,
								    DLGPROC			lpDialogFunc,
								    LPARAM			dwInitParam
								 )
{
	MSG				msg;
	HWND			hDlg;
	int				rc;
	HWND			lastFocus = GetFocus();
	
	hDlg = CreateDialogIndirectParam(
									hInstance,
									pDialogTemplate,
									hWndParent,
									lpDialogFunc,
									dwInitParam
									);
		
	if(! hDlg)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return -1;
	}
	ShowWindow(hDlg, SW_SHOW);

	// push dialog on the dialog stack
	_zg_dialogWnd[++_zg_dialogTop] = hDlg;
			
	// modal wait
	while(GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		rc = DispatchMessage(&msg);
		if(! _zg_dialogWnd[_zg_dialogTop]) break;
		SetWindowLong(hDlg, DWL_MSGRESULT, rc);
	}
	rc = GetWindowLong(hDlg, DWL_MSGRESULT);
	DestroyWindow(hDlg);

	// pop the dialog stack
	if(_zg_dialogTop >= 0)
		_zg_dialogTop--;

	if(_zg_dialogTop >= 0)
	{
		// give focus to next dialog in stack
		SetFocus(_zg_dialogWnd[_zg_dialogTop]);
	}
	else if(lastFocus)
	{
		// give focus to window who had it coming in
		SetFocus(lastFocus);
	}
	return rc;
}

//**************************************************************************
BOOL WINAPI EndDialog(HWND hDlg, int nResult)
{
	if(_zg_dialogTop < 0) return FALSE;
	if(hDlg != _zg_dialogWnd[_zg_dialogTop]) return FALSE;

	SetWindowLong(hDlg, DWL_MSGRESULT, nResult);
	_zg_dialogWnd[_zg_dialogTop] = NULL;
	return TRUE;
}

//**************************************************************************
HWND WINAPI GetDlgItem(HWND hDlg, int nIDDlgItem)
{
	LPZWND zDlg;
	
	if(! (zDlg = Zwindow(hDlg)))	return NULL;
	
	for(zDlg = zDlg->cnext; zDlg; zDlg = zDlg->cnext)
	{
		if(GetWindowLong(zDlg, GWL_ID) == nIDDlgItem)
			return (HWND)zDlg;
	}
	for(zDlg = _zg_windows; zDlg; zDlg = zDlg->next)
	{
		if(GetParent(zDlg) == hDlg)
		{
			if(GetWindowLong(zDlg, GWL_ID) == nIDDlgItem)
				return (HWND)zDlg;
		}
	}
	return NULL;
}

//**************************************************************************
int WINAPI GetDlgItemText(HWND hDlg, int nIDDlgItem, LPTSTR text, int nText)
{
	LPZWND zDlg = GetDlgItem(hDlg, nIDDlgItem);
	
	if(zDlg) return GetWindowText(zDlg, text, nText);
	else     return 0;
}

//**************************************************************************
int WINAPI IsDlgButtonChecked(HWND hDlg, int nIDButton)
{
	LPZWND zWnd = GetDlgItem(hDlg, nIDButton);
	
	if(zWnd) return (SendMessage(zWnd, BM_GETCHECK, 0, 0) & BST_CHECKED) ? TRUE : FALSE;
	else     return 0;
}

//**************************************************************************
int WINAPI SetDlgItemText(HWND hDlg, int nIDDlgItem, LPCTSTR text)
{
	LPZWND zDlg = GetDlgItem(hDlg, nIDDlgItem);
	
	if(zDlg) return SetWindowText(zDlg, text);
	else     return 0;
}

//**************************************************************************
int WINAPI CheckDlgButton(HWND hDlg, int nIDButton, UINT check)
{
	LPZWND zWnd = GetDlgItem(hDlg, nIDButton);
	
	if(zWnd) return SendMessage(zWnd, BM_SETCHECK, ((check & BST_CHECKED) ? TRUE : FALSE), 0);
	else     return 0;
}

extern WNDPROC __wproc_Edit;

//**************************************************************************
LRESULT WINAPI DefDlgProc(
						    HWND hDlg,
						    UINT Msg,
						    WPARAM wParam,
						    LPARAM lParam
						  )
{
	PAINTSTRUCT ps;
	HDC         hdc;
	LPZWND		zWnd;
	DLGPROC		proc;
	DWORD		style;
	int			idCurrent;
	DWORD		code;
	
	if(! (zWnd = Zwindow(hDlg))) return -1;
	
	proc 	  = (DLGPROC)GetWindowLong(hDlg, DWL_DLGPROC);
	idCurrent = GetWindowLong(hDlg, GWL_ID);
	
	switch(Msg)
	{
	/*
	case WM_INITDIALOG:
		
		// set focus on current id
		zWnd = Zwindow(GetDlgItem(hDlg, idCurrent));
		if(zWnd)
			SetFocus(zWnd);
		return FALSE;
	*/
	case WM_INITDIALOG:
		// do smart focusing in WM_SETFOCUS
		if(proc) 
			return proc(hDlg, Msg, wParam, lParam);
		return TRUE;

	case WM_CREATE:
		
		// [TODO - figure out how win32 decides to show dots around buttons
		// _zg_keynav = FALSE;
		break;

	case WM_CLOSE:

		EndDialog(hDlg, IDCANCEL);
		break;
		
	case WM_PAINT:

		hdc = BeginPaint(hDlg, &ps);
		
		// draw a box around the "current" control
		// if it is a push button		
		if((zWnd = Zwindow(GetDlgItem(hDlg, idCurrent))) != NULL)
		{
			if((GetWindowLong(zWnd, GWL_STYLE) & 0xf) <= 1)
			{
#if 0
				HPEN hPen;
				HPEN hOld;
				RECT rcParent;
				
				GetWindowRect(zWnd, &rcCtrl);
				GetWindowRect(GetParent(zWnd), &rcParent);
				
				rcCtrl.left 	-= rcParent.left;
				rcCtrl.right 	-= rcParent.left;
				rcCtrl.top 		-= rcParent.top;
				rcCtrl.bottom	-= rcParent.top;
				
				hPen = GetStockObject(BLACK_PEN);
				hOld = SelectObject(hdc, hPen);
				MoveToEx(hdc, rcCtrl.left - 1, rcCtrl.top - 1, NULL);
				LineTo(hdc, rcCtrl.left - 1, rcCtrl.bottom + 1);
				LineTo(hdc, rcCtrl.right + 1, rcCtrl.bottom + 1);
				LineTo(hdc, rcCtrl.right + 1, rcCtrl.top - 1);
				LineTo(hdc, rcCtrl.left - 1, rcCtrl.top - 1);
				SelectObject(hdc, hOld);
#endif
			}
		}
		EndPaint(hDlg, &ps);
		if(proc) return proc(hDlg, Msg, wParam, lParam);
		break;

	case WM_COMMAND:

		if(proc) return proc(hDlg, Msg, wParam, lParam);

		if(
				wParam == IDOK 		|| wParam == IDCANCEL 	||
				wParam == IDNO 		|| wParam == IDYES 		||
				wParam == IDRETRY 	|| wParam == IDABORT 	||
				wParam == IDIGNORE
		)
		{
			EndDialog(hDlg, wParam);
			return wParam;
		}
		break;

	case WM_SETFOCUS:
		
		// give focus to first tabstop in tab order
		//
		zWnd = Zwindow(hDlg); 

		while(zWnd && zWnd->cnext)
		{			
			zWnd = zWnd->cnext;

			style = GetWindowLong(zWnd, GWL_STYLE);
			if(style & WS_TABSTOP && ! (style & WS_DISABLED))
			{
				if(zWnd->class && ! _tcscmp(zWnd->class->name, _T("Edit")))
				{
					SetFocus(zWnd);
					break;
				}
			}
		}
		if(! zWnd)
		{
			zWnd = Zwindow(hDlg); 
			
			while(zWnd && zWnd->cnext)
			{
				zWnd = zWnd->cnext;
				
				style = GetWindowLong(zWnd, GWL_STYLE);
				if(style & WS_TABSTOP && ! (style & WS_DISABLED))
				{
					SetFocus(zWnd);
					break;
				}
			}
		}
		if(! zWnd)
			SetFocus(hDlg);
		break;
		
	case WM_KEYDOWN:

		zWnd = GetDlgItem(hDlg, idCurrent);
		code = 0;
		if(zWnd)
		{
			code = SendMessage(zWnd, WM_GETDLGCODE, 0, 0);
		}
		if(
				(wParam == VK_TAB && (! (code & DLGC_WANTALLKEYS)))
			||	(
					(wParam == VK_RIGHT || wParam == VK_LEFT || wParam == VK_DOWN || wParam == VK_UP)
				&& ! (code & (DLGC_WANTALLKEYS | DLGC_WANTARROWS))
				)
		)
		{	
			if((wParam == VK_UP) || (wParam == VK_LEFT))
			{
				// move to prev tabstop in dialog, wrapping around
				// to last tab stop
				//
				zWnd = Zwindow(GetDlgItem(hDlg, idCurrent));
				if(zWnd) zWnd = zWnd->cprev;
				
				while(zWnd)
				{
					if(zWnd && (GetWindowLong(zWnd, GWL_STYLE) & WS_TABSTOP))
						break;
					zWnd = zWnd->cprev;
				}
				if(! zWnd)
				{
					for(zWnd = Zwindow(hDlg)->cnext; zWnd && zWnd->cnext;)
						zWnd = zWnd->cnext;
					while(zWnd)
					{
						if(zWnd && (GetWindowLong(zWnd, GWL_STYLE) & WS_TABSTOP))
							break;
						zWnd = zWnd->cprev;
					}
				}
			}
			else
			{
				// move to next tabstop in dialog, wrapping around
				// to first tab stop
				//
				zWnd = Zwindow(GetDlgItem(hDlg, idCurrent));
				if(zWnd) zWnd = zWnd->cnext;
				
				while(zWnd)
				{
					if(zWnd && (GetWindowLong(zWnd, GWL_STYLE) & WS_TABSTOP))
						break;
					zWnd = zWnd->cnext;
				}
				if(! zWnd)
				{
					zWnd = Zwindow(hDlg)->cnext;
					while(zWnd)
					{
						if(zWnd && (GetWindowLong(zWnd, GWL_STYLE) & WS_TABSTOP))
							break;
						zWnd = zWnd->cnext;
					}
				}
			}
			if(zWnd)
			{
				_zg_keynav 	 = TRUE;
				if(_zg_dialogCtrl)
					InvalidateRect(_zg_dialogCtrl, NULL, TRUE);
				_zg_dialogCtrl = zWnd;
				if(zWnd->class && ((void*)zWnd->class->proc == (void*)& __wproc_Edit))
				{
					SetFocus(zWnd);
				}
				SetWindowLong(hDlg, GWL_ID, GetWindowLong(zWnd, GWL_ID));
				InvalidateRect(hDlg, NULL, FALSE);
				InvalidateRect(zWnd, NULL, TRUE);
			}
		}
		else if((wParam == 10 || wParam == 13) && ! (code & DLGC_WANTALLKEYS))
		{
			// simulate hitting current or default button
			zWnd = Zwindow(GetDlgItem(hDlg, idCurrent));

			if(!zWnd || _tcscmp(zWnd->class->name, _T("Button")))
			{
				zWnd = Zwindow(hDlg);
				if(zWnd) zWnd = zWnd->cnext;

				while(zWnd)
				{
					if(! _tcscmp(zWnd->class->name, _T("Button")))
						if((GetWindowLong(zWnd, GWL_STYLE) & 0xf) == BS_DEFPUSHBUTTON)
							break;
					zWnd = zWnd->cnext;
				}
			}
			if(zWnd)
			{
				SETPUSHED(zWnd, TRUE);
				SETENTERED(zWnd, TRUE);
				SendMessage(zWnd, WM_LBUTTONUP, 0, 0);
			}
			else
			{
				EndDialog(hDlg, IDOK);
			}			
		}
		else if(wParam == VK_ESCAPE)
		{
			EndDialog(hDlg, IDCANCEL);
		}
		else
		{
			if(proc) return proc(hDlg, Msg, wParam, lParam);
		}
		break;
	
	default:
		
		if(proc) return proc(hDlg, Msg, wParam, lParam);
	}
	return 0;
}

//**************************************************************************

#endif
