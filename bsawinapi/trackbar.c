//-----------------------------------------TBS_VERT--------------------
//
// File: scroller.c
// Desc: scroll bars for winapi
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------

#ifndef Windows
#include "winapix.h"

typedef struct tagZTRACKBAR
{
    int     nMin;
    int     nMax;
    UINT    nPage;
    int     nPos;
    int     nTrackPos;
	LPDWORD	pdwTicks;
    BOOL	firsttimer;
    UINT_PTR idtimer;
    int		type;
    int		uppushed;
    int		downpushed;
    int		ipos;
	int		ext;
    HWND	hwndThumb;
    HWND	hwndBar;
    HWND	hwndParent;
}
__ztc;

#define BSA_SCROLL_REPEAT_DELAY	450
#define BSA_SCROLL_REPEAT		50

#define BSA_TB_THUMB_H		8
#define BSA_TB_THUMB_W		8


//-----------------------------------------------------------------------------

extern LPZWND _zg_windows;
extern HBRUSH __w_solidbrush(DWORD xcolor);

//**************************************************************************
void __ztc_pos(__ztc* psc, HWND hWnd, LPRECT rcbar)
{
	RECT rc;
	
	GetClientRect(hWnd, &rc);
	
	switch(psc->type)
	{
	case TBS_VERT:
		rcbar->left  	= 0;
		rcbar->top   	= 0;
		rcbar->right 	= rc.right - rc.left;
		rcbar->bottom	= psc->ext;
		break;
		
	case TBS_HORZ:
		rcbar->top   	= 0;
		rcbar->bottom	= rc.bottom - rc.top;
		rcbar->left  	= 0;
		rcbar->right 	= psc->ext;
		break;			
	}
}

//**************************************************************************
__ztc* __new_ztc(HWND hWndParent, int type)
{
	RECT   rc;
	__ztc* nsc;
	
	if(! hWndParent) { SetLastError(ERROR_INVALID_PARAMETER); return NULL; }

	nsc = (__ztc*)malloc(sizeof(__ztc));
	if(! nsc) return NULL;

	nsc->hwndParent = GetParent(hWndParent);
	nsc->hwndBar    = hWndParent;
	nsc->type       = type;
	nsc->nMin       = 1;
	nsc->nMax       = 100;
	nsc->nPage      = 10;
	nsc->nPos       = 1;
	nsc->nTrackPos  = 1;
	nsc->idtimer	= NULL;
	nsc->firsttimer = TRUE;
	nsc->uppushed	= 0;
	nsc->downpushed = 0;
	nsc->ext		= BSA_TB_THUMB_W;
	
	__ztc_pos(nsc, hWndParent, &rc);
	
	nsc->hwndThumb = CreateWindowEx(
							0,
							_T("TrackThumb"),
							_T(""),
							WS_CHILD,
							rc.left, rc.top,
							rc.right-rc.left, rc.bottom-rc.top,
							nsc->hwndBar,
							NULL, (HINSTANCE)0xbeef,
							NULL
							);
	SetWindowLong(nsc->hwndThumb, GWL_USERDATA, (LONG)nsc);

	ShowWindow(nsc->hwndBar, SW_SHOW);
	ShowWindow(nsc->hwndThumb, SW_SHOW);

	PostMessage(nsc->hwndBar, WM_SIZE, 0, 0);
	return nsc;
}

//**************************************************************************
static void __w_set_trackbar_pos(__ztc* psc, DWORD newpos)
{
	RECT rc;
	int x, y, s, r;

	psc->nTrackPos = newpos;
	
	if(psc->nPage < 1)
		psc->nPage = 1;
	if(psc->nPage > psc->nMax - psc->nMin)
		psc->nPage = psc->nMax - psc->nMin;

	if(psc->nTrackPos < psc->nMin)
		psc->nTrackPos = psc->nMin;
	if(psc->nTrackPos > psc->nMax)
		psc->nTrackPos = psc->nMax;
	
	GetClientRect(psc->hwndBar, &rc);
		
	if(psc->type == TBS_VERT)
	{
		// space to scroll in
		s = rc.bottom - rc.top - psc->ext;
		if(s < 1) s = 1;
		
		// range of scroller
		r = psc->nMax - psc->nMin;
		if(r < 1) r = 1;
		
		// unmap to get clipped y
		y = psc->nTrackPos * s / r;
		MoveWindow(psc->hwndThumb, 0, y, psc->ext, rc.right - rc.left, FALSE);
	}
	else
	{
		// space to scroll in
		s = rc.right - rc.left - psc->ext;
		if(s < 1) s = 1;
		
		// range of scroller
		r = psc->nMax - psc->nMin;
		if(r < 1) r = 1;
		
		// unmap to get clipped x
		x = (psc->nTrackPos - psc->nMin) * s / r;
		MoveWindow(psc->hwndThumb, x, 0, psc->ext, rc.bottom - rc.top, FALSE);		
	}
	psc->nPos = psc->nTrackPos;
}

//**************************************************************************
LRESULT WINAPI __wproc_Trackbar(HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{
	PAINTSTRUCT ps;
	HDC         hdc;
	HBRUSH		hbr;
	LPZWND		zWnd;
	RECT		rc;
	POINT		pt;
	__ztc*      psc;
	int			opos;

	if(! (zWnd = Zwindow(hWnd))) return -1;
	psc = (__ztc*)GetWindowLong(hWnd, GWL_USERDATA);

	switch(message)
	{
	case WM_CREATE:
		
		psc = __new_ztc(hWnd, TBS_HORZ);
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)psc);
		break;
		
	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		// draw slider bar
		hbr = CreateSolidBrush(GetBkColor(hdc));
		FillRect(hdc, &ps.rcPaint, hbr);
		DeleteObject(hbr);
		// draw slider bar		
		GetClientRect(hWnd, &rc);
		rc.top = (rc.bottom - rc.top ) / 2 - 2;
		rc.bottom = rc.top + 4;
		__w_button_box(hdc, 1, &rc);
		EndPaint(hWnd, &ps);
		break;

	case WM_LBUTTONDOWN:

		SetCapture(hWnd);
		// fall into ...
		
	case WM_TIMER:
		
		if(! psc) return -1;
		GetCursorPos(&pt);
		GetWindowRect(psc->hwndThumb, &rc);
		opos = psc->nPos;
		if(psc->type == TBS_VERT)
		{
			__w_set_trackbar_pos(psc, psc->nPos + (((rc.top + psc->ext / 2) < pt.y) ? psc->nPage : -psc->nPage));
			PostMessage(psc->hwndParent, WM_VSCROLL,
				(((WPARAM)(opos < psc->nPos) ? SB_PAGEDOWN : SB_PAGEUP) | (WPARAM)(psc->nTrackPos<<16)), 0);
		}
		else
		{
			__w_set_trackbar_pos(psc, psc->nPos + (((rc.left + psc->ext / 2) < pt.x) ? psc->nPage : -psc->nPage));
			PostMessage(psc->hwndParent, WM_HSCROLL,
				(((WPARAM)(opos < psc->nPos) ? SB_PAGERIGHT : SB_PAGELEFT) | (WPARAM)(psc->nTrackPos<<16)), 0);
		}
		if(message == WM_LBUTTONDOWN)
		{
			psc->firsttimer = TRUE;
			psc->idtimer = SetTimer(hWnd, 0xbeaf, BSA_SCROLL_REPEAT_DELAY, NULL);
		}
		else if(psc->idtimer && psc->firsttimer)
		{
			KillTimer(hWnd, (UINT_PTR)0xbeaf);
			psc->firsttimer = FALSE;
			if(GetCapture() == hWnd)
				psc->idtimer = SetTimer(hWnd, 0xbeaf, BSA_SCROLL_REPEAT, NULL);
		}
		else if(psc->idtimer)
		{
			if(GetCapture() != hWnd)
				KillTimer(hWnd, (UINT_PTR)0xbeaf);
		}
		break;
		
	case WM_LBUTTONUP:

		KillTimer(hWnd, (UINT_PTR)0xbeaf);
		ReleaseCapture();
		break;
		
	case WM_MOUSEMOVE:

		break;
		
	case WM_SIZE:

		PostMessage(psc->hwndThumb, WM_SIZE, 0, 0);
		break;

	case TBM_GETPOS:
		return psc->nPos;		
	case TBM_GETRANGEMIN:
		return psc->nMin;
	case TBM_GETRANGEMAX:
		return psc->nMax;
	case TBM_GETTIC:
	case TBM_SETTIC:
		break;
	case TBM_SETPOS:
		__w_set_trackbar_pos(psc, lParam);
		break;
	case TBM_SETRANGE:
		psc->nMin = LOWORD(lParam);
		psc->nMax = HIWORD(lParam);
		__w_set_trackbar_pos(psc, psc->nPos);
		break;
	case TBM_SETRANGEMIN:
		psc->nMin = lParam;
		__w_set_trackbar_pos(psc, psc->nPos);
		break;
	case TBM_SETRANGEMAX:
		psc->nMax = lParam;
		__w_set_trackbar_pos(psc, psc->nPos);
		break;
	case TBM_CLEARTICS:
	case TBM_SETSEL:
	case TBM_SETSELSTART:
	case TBM_SETSELEND:
	case TBM_GETPTICS:
	case TBM_GETTICPOS:
	case TBM_GETNUMTICS:
	case TBM_GETSELSTART:
	case TBM_GETSELEND:
	case TBM_CLEARSEL:
	case TBM_SETTICFREQ:
		break;
	case TBM_SETPAGESIZE:
		psc->nPage = lParam;
		break;
	case TBM_GETPAGESIZE:
		return psc->nPage;
	case TBM_SETLINESIZE:
	case TBM_GETLINESIZE:
		break;
	case TBM_GETTHUMBRECT:
	case TBM_GETCHANNELRECT:
		break;
	case TBM_SETTHUMBLENGTH:
		psc->ext = lParam;
		break;
	case TBM_GETTHUMBLENGTH:
		return psc->ext;

	default:

		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


//**************************************************************************
LRESULT WINAPI __wproc_TrackThumb(HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{
	PAINTSTRUCT ps;
	HDC         hdc;
	RECT		rc;
	HBRUSH		hbr;
	LPZWND		zWnd;
	__ztc*      psc;
	int			x, y;
	int			r, s;

	if(! (zWnd = Zwindow(hWnd))) return -1;
	if(! (psc = (__ztc*)GetWindowLong(hWnd, GWL_USERDATA))) return -1;
		
	switch(message)
	{
	case WM_CREATE:
		
		break;
		
	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rc);
		hbr = CreateSolidBrush(GetBkColor(hdc));
		FillRect(hdc, &ps.rcPaint, hbr);
		rc.top++; rc.left++;
		rc.bottom--; rc.right--;
		__w_button_box(hdc, 0, &rc);
		DeleteObject(hbr);
		
		switch(psc->type)
		{
		case TBS_HORZ:
			
			break;
			
		case TBS_VERT:

			break;
		}
		EndPaint(hWnd, &ps);
		break;

	case WM_LBUTTONDOWN:

		SetCapture(hWnd);
		psc->ipos = (psc->type == TBS_VERT) ?  HIWORD(lParam) : LOWORD(lParam);
		break;
		
	case WM_LBUTTONUP:

		ReleaseCapture();
		break;
		
	case WM_MOUSEMOVE:

		if(GetCapture() != hWnd)
			break;
		
		GetClientRect(psc->hwndBar, &rc);
		if(psc->nPage < 1) psc->nPage = 1;
		if(psc->nPage > psc->nMax - psc->nMin)
			psc->nPage = psc->nMax - psc->nMin;
		
		if(psc->type == TBS_VERT)
		{
			// position of mouse relative to widget
			y = (int)(short)HIWORD(lParam) - psc->ipos;
			
			// space to scroll in
			s = rc.bottom - rc.top;
			if(s < 1) s = 1;
			
			// range of scroller
			r = psc->nMax - psc->nMin;
			if(r < 1) r = 1;
			
			if(s < 1) return 0;
			__w_set_trackbar_pos(psc, psc->nTrackPos + (y * r / s));			
		}
		else
		{
			// position of mouse relative to widget
			x = (int)(short)LOWORD(lParam) - psc->ipos;
			
			// space to scroll in
			s = rc.right - rc.left;
			if(s < 1) s = 1;
			
			// range of scroller
			r = psc->nMax - psc->nMin;
			if(r < 1) r = 1;
			
			if(s < 1) return 0;
			__w_set_trackbar_pos(psc, psc->nTrackPos + (x * r / s));			
		}
		PostMessage(psc->hwndParent,
				(psc->type == TBS_VERT ? WM_VSCROLL : WM_HSCROLL),
				((WPARAM)SB_THUMBTRACK | (WPARAM)(psc->nTrackPos<<16)), 0);
		break;

	default:

		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


#endif
