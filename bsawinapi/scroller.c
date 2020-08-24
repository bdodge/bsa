//--------------------------------------------------------------------
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

#define BSA_SCROLL_REPEAT_DELAY	450
#define BSA_SCROLL_REPEAT		50

//-----------------------------------------------------------------------------

extern LPZWND _zg_windows;
extern HBRUSH __w_solidbrush(DWORD xcolor);

//**************************************************************************
void __zsc_pos(LPZSBAR psc, HWND hWnd, LPRECT rcbar)
{
	RECT rc;
	DWORD  style;
	
	GetClientRect(hWnd, &rc);
	style = GetWindowLong(hWnd, GWL_STYLE);	
	
	switch(psc->type)
	{
	case SB_VERT:
		rcbar->left  	= rc.right - rc.left;
		rcbar->top   	= rc.top;
		rcbar->right 	= rcbar->left + BSA_VS_WIDTH - 1;
		rcbar->bottom	= rcbar->top + rc.bottom - rc.top;
		break;
		
	case SB_HORZ:
		rcbar->left		= rc.left;
		rcbar->top  	= rc.bottom - rc.top;
		rcbar->right	= rcbar->left + rc.right - rc.left;
		if(style & WS_VSCROLL)
			rcbar->right += BSA_VS_WIDTH;
		rcbar->bottom	= rcbar->top + BSA_HS_HEIGHT - 1;
		break;			
	}
}

//**************************************************************************
LPZSBAR __new_zsc(HWND hwndParent, int type)
{
	RECT   rc;
	LPZSBAR nsc;
	int    x, y, w, h;
	long   style;
	
	if(! hwndParent) { SetLastError(ERROR_INVALID_PARAMETER); return NULL; }

	nsc = (LPZSBAR)malloc(sizeof(ZSBAR));
	if(! nsc) return NULL;
	memset(nsc, 0, sizeof(ZSBAR));

	nsc->tag		= ZSBAR_TAG;
	nsc->hwndParent = hwndParent;
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
	
	__zsc_pos(nsc, hwndParent, &rc);
	
	nsc->hwndBar = CreateWindowEx(
							0,
							_T("ScrollBar"),
							(type == SB_VERT) ? _T("vsc") : _T("hsc"),
							WS_CHILD,
							rc.left, rc.top,
							rc.right - rc.left + 1,
							rc.bottom - rc.top + 1,
							hwndParent,
							NULL, (HINSTANCE)0xbeef,
							NULL
							);
	SetWindowLong(nsc->hwndBar, GWL_USERDATA, (LPARAM)nsc);
	GetClientRect(nsc->hwndBar, &rc);

    style = GetWindowLong(nsc->hwndParent, GWL_STYLE);

	switch(type)
	{
	case SB_VERT:
		x = 0;
		y = BSA_VSX_HEIGHT;
		w = BSA_VS_WIDTH;
		h = BSA_VSX_HEIGHT;
		style |= WS_VSCROLL;
		break;
		
	case SB_HORZ:
		x = BSA_HSX_WIDTH;
		y = 0;
		w = BSA_HSX_WIDTH;
		h = BSA_HS_HEIGHT;
		style |= WS_HSCROLL;
		break;
	}
	SetWindowLong(nsc->hwndParent, GWL_STYLE, style);
	
	nsc->hwndThumb = CreateWindowEx(
							0,
							_T("Scroller"),
							(type == SB_VERT) ? _T("vst") : _T("hst"),
							WS_CHILD,
							x, y,
							w, h,
							nsc->hwndBar,
							NULL, (HINSTANCE)0xbeef,
							NULL
							);
	SetWindowLong(nsc->hwndThumb, GWL_USERDATA, (LPARAM)nsc);

	switch(type)
	{
	case SB_VERT:
		x = 0;
		y = 0;
		w = BSA_VS_WIDTH;
		h = BSA_VSX_HEIGHT;
		break;
		
	case SB_HORZ:
		x = 0;
		y = 0;
		w = BSA_HSX_WIDTH;
		h = BSA_HS_HEIGHT;
		break;
	}
	nsc->hwndUp = CreateWindowEx(
							0,
							_T("ScrollCtrl"),
							(type == SB_VERT) ? _T("vsu") : _T("hsu"),
							WS_CHILD,
							x, y,
							w, h,
							nsc->hwndBar,
							NULL, (HINSTANCE)0xbeef,
							NULL
							);
	SetWindowLong(nsc->hwndUp, GWL_USERDATA, (LPARAM)nsc);

	switch(type)
	{
	case SB_VERT:
		x = 0;
		y = rc.bottom - rc.top - BSA_VSX_HEIGHT;
		w = BSA_VS_WIDTH;
		h = BSA_VSX_HEIGHT;
		break;
		
	case SB_HORZ:
		x = rc.right - rc.left - BSA_HSX_WIDTH;
		y = 0;
		w = BSA_HSX_WIDTH;
		h = BSA_HS_HEIGHT;
		break;
	}
	nsc->hwndDown = CreateWindowEx(
							0,
							_T("ScrollCtrl"),
							(type == SB_VERT) ? _T("vsd") : _T("hsd"),
							WS_CHILD,
							x, y,
							w, h,
							nsc->hwndBar,
							NULL, (HINSTANCE)0xbeef,
							NULL
							);
	SetWindowLong(nsc->hwndDown, GWL_USERDATA, (LPARAM)nsc);
					
	ShowWindow(nsc->hwndBar, SW_SHOW);
	ShowWindow(nsc->hwndThumb, SW_SHOW);
	ShowWindow(nsc->hwndUp, SW_SHOW);
	ShowWindow(nsc->hwndDown, SW_SHOW);

	PostMessage(hwndParent, WM_SIZE, 0, 0);
	PostMessage(nsc->hwndBar, WM_SIZE, 0, 0);
	return nsc;
}

//**************************************************************************
BOOL WINAPI __w_attach_scroller(HWND hwndParent, int type)
{
	__new_zsc(hwndParent, type);
	return TRUE;
}

//**************************************************************************
BOOL WINAPI __w_detach_scroller(HWND hwndBar)
{
	LPZSBAR psc = (LPZSBAR)GetWindowLong(hwndBar, GWL_USERDATA);

	if(psc)
	{
		if(psc->hwndThumb)	DestroyWindow(psc->hwndThumb);
		psc->hwndThumb = NULL;
		if(psc->hwndUp)		DestroyWindow(psc->hwndUp);
		psc->hwndUp = NULL;
		if(psc->hwndDown)	DestroyWindow(psc->hwndDown);
		psc->hwndDown = NULL;
		if(psc->hwndBar)	DestroyWindow(psc->hwndBar);
		psc->hwndBar = NULL;
		if(psc->hwndParent)
		{
			long style = GetWindowLong(psc->hwndParent, GWL_STYLE);
			
			switch(psc->type)
			{
			case SB_VERT:
				style &= ~WS_VSCROLL;
				break;
				
			case SB_HORZ:
				style &= ~WS_HSCROLL;
				break;
			}
			SetWindowLong(psc->hwndParent, GWL_STYLE, style);
			PostMessage(psc->hwndParent, WM_SIZE, 0, 0);
		}
	}
	return TRUE;
}

//**************************************************************************
int WINAPI SetScrollInfo(HWND hwnd, int which, LPCSCROLLINFO pInfo, BOOL bRedraw)
{
	HWND   hwndBar;
	LPZSBAR psc;
	BOOL   changed = FALSE;
	BOOL   needbar = TRUE;
	
	if(! pInfo)
		return 0;
	if(pInfo->cbSize != sizeof(SCROLLINFO))
		return 0;
	
	if((pInfo->fMask & (SIF_RANGE | SIF_PAGE)) == (SIF_RANGE | SIF_PAGE))
	{
		int nr = pInfo->nMax - pInfo->nMin;
		
		if(nr < 0) nr = 0;
		if(nr < pInfo->nPage)
			needbar = FALSE;
	}
	hwndBar = _w_getScrollBar(hwnd, which);
	if(! hwndBar)
	{
		if(needbar)
		{
			__w_attach_scroller(hwnd, which);
			hwndBar = _w_getScrollBar(hwnd, which);
		}
	}
	if(! hwndBar)
	{
		return 0;
	}
	if(! (psc = (LPZSBAR)GetWindowLong(hwndBar, GWL_USERDATA)))
	{
		return 0;
	}
	if(! needbar)
	{
		__w_detach_scroller(hwndBar);
		return 0;
	}
	if(pInfo->fMask & SIF_PAGE)
		psc->nPage = pInfo->nPage;
	if(pInfo->fMask & SIF_RANGE)
	{
		changed |= pInfo->nMin != psc->nMin;
		changed |= pInfo->nMax != psc->nMax;
		psc->nMin = pInfo->nMin;
		psc->nMax = pInfo->nMax;
		if(psc->nMax < psc->nMin)
			psc->nMax = psc->nMin;
	}
	if(pInfo->fMask & SIF_POS)
	{
		changed |= pInfo->nPos != psc->nPos;
		psc->nPos = pInfo->nPos;
		if(psc->nPos < psc->nMin)
			psc->nPos = psc->nMin;
		if(psc->nPos > psc->nMax - psc->nPage)
			psc->nPos = psc->nMax - psc->nPage;
		if(changed)
			PostMessage(psc->hwndThumb, WM_SIZE, 0, 0);
	}
	if(changed)
		PostMessage(hwndBar, WM_SIZE, 0, 0);
	if(bRedraw)
		InvalidateRect(hwndBar, NULL, TRUE);
	return pInfo->nPos;
}

//**************************************************************************
BOOL WINAPI GetScrollInfo(HWND hwnd, int which, LPSCROLLINFO pInfo)
{
	HWND   hwndBar;
	LPZSBAR psc;
	
	hwndBar = _w_getScrollBar(hwnd, which);
	
	if(! hwndBar || ! pInfo)
		return FALSE;
	if(pInfo->cbSize != sizeof(SCROLLINFO))
		return FALSE;
	psc = (LPZSBAR)GetWindowLong(hwndBar, GWL_USERDATA);

	if(pInfo->fMask & SIF_PAGE)
		pInfo->nPage = psc->nPage;
	if(pInfo->fMask & SIF_RANGE)
	{
		pInfo->nMin = psc->nMin;
		pInfo->nMax = psc->nMax;
	}
	if(pInfo->fMask & SIF_POS)
		pInfo->nPos = psc->nPos;
	if(pInfo->fMask & SIF_TRACKPOS)
		pInfo->nTrackPos = psc->nTrackPos;
	return TRUE;
}


//**************************************************************************
int WINAPI SetScrollPos(HWND hwnd, int which, int nPos, BOOL bRedraw)
{
	HWND   hwndBar;
	LPZSBAR psc;

	hwndBar = _w_getScrollBar(hwnd, which);
	
	if(! hwndBar)
		return 0;
	if(! (psc = (LPZSBAR)GetWindowLong(hwndBar, GWL_USERDATA)))
		return 0;
	if(nPos < psc->nMin)
		nPos = psc->nMin;
	if(nPos > psc->nMax)
		nPos = psc->nMax;
	psc->nPos = nPos;
	PostMessage(psc->hwndThumb, WM_SIZE, 0, 0);
	if(bRedraw)
		InvalidateRect(hwndBar, NULL, TRUE);
	return nPos;
}

//**************************************************************************
int WINAPI GetScrollPos(HWND hwnd, int which)
{
	HWND   hwndBar;
	LPZSBAR psc;
	
	hwndBar = _w_getScrollBar(hwnd, which);
	
	if(! hwndBar)
		return 0;
	if(! (psc = (LPZSBAR)GetWindowLong(hwndBar, GWL_USERDATA)))
		return 0;
	return psc->nPos;
}

//**************************************************************************
BOOL WINAPI SetScrollRange(
						    HWND hwnd,
						    int nBar,
						    int nMinPos,
						    int nMaxPos,
						    BOOL bRedraw
						   )
{
	SCROLLINFO si;
	
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask  = SIF_RANGE;
	si.nMin   = nMinPos;
	si.nMax   = nMaxPos;
	SetScrollInfo(hwnd, nBar, &si, bRedraw);	
	return TRUE;
}

//**************************************************************************
BOOL WINAPI GetScrollRange(
						    HWND hwnd,
						    int nBar,
						    LPINT lpMinPos,
						    LPINT lpMaxPos
						   )
{
	SCROLLINFO si;
	
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask  = SIF_RANGE;
	if(GetScrollInfo(hwnd, nBar, &si))
	{
		if(lpMinPos) *lpMinPos = si.nMin;
		if(lpMaxPos) *lpMaxPos = si.nMax;
		return TRUE;
	}
	return FALSE;
}

//**************************************************************************
LRESULT WINAPI __wproc_Scrollbar(HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{
	PAINTSTRUCT ps;
	HDC         hdc;
	HBRUSH		hbr;
	LPZWND		zWnd;
	RECT		rc;
	LPZSBAR      psc;
	int			x, y, w, h, r, s;
	
	if(! (zWnd = Zwindow(hWnd))) return -1;

	switch(message)
	{
	case WM_CREATE:
		
		break;
		
	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		hbr = CreateSolidBrush(GetBkColor(hdc));
		FillRect(hdc, &ps.rcPaint, hbr);
		DeleteObject(hbr);
		EndPaint(hWnd, &ps);
		break;

	case WM_LBUTTONDOWN:

		if(! (psc = (LPZSBAR)GetWindowLong(hWnd, GWL_USERDATA))) return -1;
		GetClientRect(hWnd, &rc);
		y = HIWORD(lParam);
		x = LOWORD(lParam);
		if(psc->type == SB_VERT)
		{
			y -= BSA_VSX_HEIGHT;
			
			// space to scroll in
			s = rc.bottom - rc.top - 2 * BSA_VSX_HEIGHT;
			if(s < 1) s = 1;
			
			// range of scroller
			r = psc->nMax - psc->nMin - psc->nPage;
			if(r < 1) r = 1;
			
			// height of scroller proportional to room avail
			h = s - r;			
			if(h < BSA_VSX_HEIGHT)
				h = BSA_VSX_HEIGHT;
			
			// take h out of room as new range of motion
			s -= h;
			if(s < 1) return 0;
			
			// map y in range to new nPos
			y = (y * r / s);
			
			if(y < psc->nPos)
			{
				if(psc->nPos > psc->nMin)
					PostMessage(psc->hwndParent, WM_VSCROLL, SB_PAGEUP, 0);
				else
					PostMessage(psc->hwndParent, WM_VSCROLL, SB_LINEUP, 0);
			}
			else if(y > psc->nPos)
			{
				if(psc->nPos < psc->nMax)
					PostMessage(psc->hwndParent, WM_VSCROLL, SB_PAGEDOWN, 0);
				else
					PostMessage(psc->hwndParent, WM_VSCROLL, SB_LINEDOWN, 0);
			}
		}
		else
		{
			x -= BSA_HSX_WIDTH;

			// space to scroll in
			s = rc.right - rc.left - 2 * BSA_HSX_WIDTH;
			if(s < 1) s = 1;
			
			// range of scroller
			r = psc->nMax - psc->nMin - psc->nPage;
			if(r < 1) r = 1;
			
			// width of scroller proportional to room avail
			w = s - r;			
			if(w < BSA_HSX_WIDTH)
				w = BSA_HSX_WIDTH;
			
			// take w out of room as new range of motion
			s -= w;
			if(s < 1) return 0;
			
			// map x in range to new nPos
			x = (x * r / s);

			if(x < psc->nPos)
			{
				if(psc->nPos > psc->nMin)
					PostMessage(psc->hwndParent, WM_HSCROLL, SB_PAGELEFT, 0);
				else
					PostMessage(psc->hwndParent, WM_HSCROLL, SB_LINELEFT, 0);
			}
			else if(x > psc->nPos)
			{
				if(psc->nPos < psc->nMax)
					PostMessage(psc->hwndParent, WM_HSCROLL, SB_PAGERIGHT, 0);
				else
					PostMessage(psc->hwndParent, WM_HSCROLL, SB_LINERIGHT, 0);
			}
		}
		break;
		
	case WM_LBUTTONUP:

		break;
		
	case WM_MOUSEMOVE:

		break;
		
	case WM_SIZE:

		if(! (psc = (LPZSBAR)GetWindowLong(hWnd, GWL_USERDATA))) return -1;
		__zsc_pos(psc, GetParent(hWnd), &rc);
		MoveWindow(hWnd, rc.left, rc.top,
				rc.right-rc.left+1, rc.bottom-rc.top+1, FALSE);
		PostMessage(psc->hwndDown, WM_SIZE, 0, 0);
		PostMessage(psc->hwndThumb, WM_SIZE, 0, 0);
		break;

	default:

		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


//**************************************************************************
LRESULT WINAPI __wproc_ScrollCtrl(HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{
	PAINTSTRUCT ps;
	HDC         hdc;
	RECT		rc;
	HPEN		hPen;
	HPEN		hOld;
	LPZWND		zWnd;
	LPZSBAR      psc;
	int			pushed;
	int			x, y, w, h;

	if(! (zWnd = Zwindow(hWnd))) return -1;
	if(! (psc = (LPZSBAR)GetWindowLong(hWnd, GWL_USERDATA))) return -1;
		
	switch(message)
	{
	case WM_CREATE:
		
		break;
		
	case WM_PAINT:

		if(zWnd == psc->hwndUp)
			pushed = psc->uppushed;
		else
			pushed = psc->downpushed;
		hdc = BeginPaint(hWnd, &ps);
		__w_drawbutton(hdc, hWnd, 0, _T(""), pushed, 0);
		GetClientRect(hWnd, &rc);
		hPen = GetStockObject(BLACK_PEN);
		hOld = SelectObject(hdc, hPen);

		switch(psc->type)
		{
		case SB_HORZ:
			
			y = (rc.bottom - rc.top) / 2;
			x = (rc.right - rc.left) / 2 - BSA_SC_ARROW_WIDTH / 2;
			
			if(zWnd == psc->hwndDown)
			{
				MoveToEx(hdc, ++x, y + 3, NULL);
				LineTo(hdc, x, y - 4);
				MoveToEx(hdc, ++x, y + 2, NULL);
				LineTo(hdc, x, y - 3);
				MoveToEx(hdc, ++x, y + 1, NULL);
				LineTo(hdc, x, y - 2);
				MoveToEx(hdc, ++x, y, NULL);
				LineTo(hdc, x, y - 1);
			}
			else
			{
				MoveToEx(hdc, x, y, NULL);
				LineTo(hdc, x, y - 1);
				MoveToEx(hdc, ++x, y + 1, NULL);
				LineTo(hdc, x, y - 2);
				MoveToEx(hdc, ++x, y + 2, NULL);
				LineTo(hdc, x, y - 3);
				MoveToEx(hdc, ++x, y + 3, NULL);
				LineTo(hdc, x, y - 4);
			}
			break;
			
		case SB_VERT:

			y = (rc.bottom - rc.top) / 2 - BSA_SC_ARROW_WIDTH / 2;
			x = (rc.right - rc.left) / 2;
			
			if(zWnd == psc->hwndDown)
			{
				MoveToEx(hdc, x + 3, ++y, NULL);
				LineTo(hdc, x - 4, y);
				MoveToEx(hdc, x + 2, ++y, NULL);
				LineTo(hdc, x - 3, y);
				MoveToEx(hdc, x + 1, ++y, NULL);
				LineTo(hdc, x - 2, y);
				MoveToEx(hdc, x, ++y, NULL);
				LineTo(hdc, x - 1, y);
			}
			else
			{
				MoveToEx(hdc, x, y, NULL);
				LineTo(hdc, x - 1, y);
				MoveToEx(hdc, x + 1, ++y, NULL);
				LineTo(hdc, x - 2, y);
				MoveToEx(hdc, x + 2, ++y, NULL);
				LineTo(hdc, x - 3, y);
				MoveToEx(hdc, x + 3, ++y, NULL);
				LineTo(hdc, x - 4, y);
			}
			break;
		}
		SelectObject(hdc, hOld);
		EndPaint(hWnd, &ps);
		break;

	case WM_LBUTTONDOWN:

		SetCapture(hWnd);
		// fall into ...
		
	case WM_TIMER:

		if(psc->type == SB_VERT)
		{
			if(psc->hwndDown == hWnd)
			{
				if(psc->nPos < psc->nMax - psc->nPage)
					PostMessage(psc->hwndParent, WM_VSCROLL, SB_LINEDOWN, 0);
			}
			else
			{
				if(psc->nPos > psc->nMin)
					PostMessage(psc->hwndParent, WM_VSCROLL, SB_LINEUP, 0);
			}
		}
		else
		{
			if(psc->hwndDown == hWnd)
			{
				if(psc->nPos < psc->nMax - psc->nPage)
					PostMessage(psc->hwndParent, WM_HSCROLL, SB_LINERIGHT, 0);
			}
			else
			{
				if(psc->nPos > psc->nMin)
					PostMessage(psc->hwndParent, WM_HSCROLL, SB_LINELEFT, 0);
			}
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

		ReleaseCapture();
		if(psc->idtimer)
		{
			KillTimer(hWnd, (UINT_PTR)0xbeaf);
			psc->idtimer = NULL;
		}
		break;
		
	case WM_MOUSEMOVE:

		break;

	case WM_SIZE:

		GetClientRect(psc->hwndBar, &rc);
		if(psc->nPage < 1) psc->nPage  = 1;
		if(psc->type == SB_VERT)
		{
			x = 0;
			w = BSA_VS_WIDTH;
			h = BSA_VSX_HEIGHT;

			if(psc->hwndDown == hWnd)
				y = rc.bottom - rc.top - BSA_VSX_HEIGHT;
			else
				y = 0;
		}
		else
		{
			y = 0;
			w = BSA_HSX_WIDTH;
			h = BSA_HS_HEIGHT;

			if(psc->hwndDown == hWnd)
				x = rc.right - rc.left - BSA_HSX_WIDTH;
			else
				x = 0;
		}
		MoveWindow(hWnd, x, y, w, h, FALSE);
		break;
		
	default:

		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//**************************************************************************
LRESULT WINAPI __wproc_Scroller(HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{
	PAINTSTRUCT ps;
	HDC         hdc;
	RECT		rc;
	LPZWND		zWnd;
	LPZSBAR      psc;
	int			x, y, w, h, s, r;
	
	if(! (zWnd = Zwindow(hWnd))) return -1;
	if(! (psc = (LPZSBAR)GetWindowLong(hWnd, GWL_USERDATA))) return -1;
		
	switch(message)
	{
	case WM_CREATE:
		
		break;
		
	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		__w_drawbutton(hdc, hWnd, 0, _T(""), 0, 0);
		EndPaint(hWnd, &ps);
		break;

	case WM_LBUTTONDOWN:

		SetCapture(hWnd);
		psc->ipos = (psc->type == TBS_VERT) ?  HIWORD(lParam) : LOWORD(lParam);
		//psc->ipos = HIWORD(lParam);
		break;
		
	case WM_LBUTTONUP:

		if(GetCapture() != hWnd)
			break;
		psc->nPos = psc->nTrackPos;
		PostMessage(psc->hwndParent,
				(psc->type == SB_VERT ? WM_VSCROLL : WM_HSCROLL),
				((WPARAM)SB_THUMBPOSITION | (WPARAM)(psc->nPos<<16)), 0);
		ReleaseCapture();		
		break;
		
	case WM_MOUSEMOVE:

		if(GetCapture() != hWnd)
			break;
		
		GetClientRect(psc->hwndBar, &rc);
		if(psc->nPage < 1) psc->nPage = 1;
		if(psc->nPage > psc->nMax - psc->nMin)
			psc->nPage = psc->nMax - psc->nMin;
		
		if(psc->type == SB_VERT)
		{
			// position of mouse relative to widget
			y = (int)(short)HIWORD(lParam) - psc->ipos;
			
			// space to scroll in
			s = rc.bottom - rc.top - 2 * BSA_VSX_HEIGHT;
			if(s < 1) s = 1;
			
			// range of scroller
			r = psc->nMax - psc->nMin - psc->nPage;
			if(r < 1) r = 1;
			
			// height of scroller proportional to room avail
			h = s - r;			
			if(h < BSA_VSX_HEIGHT)
				h = BSA_VSX_HEIGHT;
			
			// take h out of room as new range of motion
			s -= h;
			if(s < 1) return 0;

			// map y in range to new nPos
			psc->nTrackPos += (y * r / s);
			if(psc->nTrackPos < psc->nMin)
				psc->nTrackPos = psc->nMin;
			if(psc->nTrackPos > psc->nMax - psc->nPage)
				psc->nTrackPos = psc->nMax - psc->nPage;
		}
		else
		{
			// position of mouse relative to widget
			x = (int)(short)LOWORD(lParam) - psc->ipos;
			
			// space to scroll in
			s = rc.right - rc.left - 2 * BSA_HSX_WIDTH;
			if(s < 1) s = 1;
			
			// range of scroller
			r = psc->nMax - psc->nMin - psc->nPage;
			if(r < 1) r = 1;
			
			// width of scroller proportional to room avail
			w = s - r;			
			if(w < BSA_HSX_WIDTH)
				w = BSA_HSX_WIDTH;
			
			// take w out of room as new range of motion
			s -= w;
			if(s < 1) return 0;
			
			// map x in range to new nPos
			psc->nTrackPos += (x * r / s);
			if(psc->nTrackPos < psc->nMin)
				psc->nTrackPos = psc->nMin;
			if(psc->nTrackPos > psc->nMax - psc->nPage)
				psc->nTrackPos = psc->nMax - psc->nPage;
		}
		PostMessage(psc->hwndParent,
				(psc->type == SB_VERT ? WM_VSCROLL : WM_HSCROLL),
				((WPARAM)SB_THUMBTRACK | (WPARAM)(psc->nTrackPos<<16)), 0);
		break;
		
	case WM_SIZE:
		
		GetClientRect(psc->hwndBar, &rc);
		if(psc->nPage < 1) psc->nPage = 1;
		if(psc->nPage > psc->nMax - psc->nMin)
			psc->nPage = psc->nMax - psc->nMin;
		
		if(psc->type == SB_VERT)
		{
			x = 0;
			w = BSA_VS_WIDTH;
			
			// space to scroll in
			s = rc.bottom - rc.top - 2 * BSA_VSX_HEIGHT;
			if(s < 1) s = 1;
			
			// range of scroller
			r = psc->nMax - psc->nMin - psc->nPage;
			if(r < 1) r = 1;
			
			// height of scroller proportional to room avail
			h = s - r;			
			if(h < BSA_VSX_HEIGHT)
				h = BSA_VSX_HEIGHT;
			
			// take h out of room as new range of motion
			s -= h;
			if(s < 1) s = 1;
			
			// map nPos to new scoll range
			y = (psc->nPos - psc->nMin) * s / r;
			y += BSA_VSX_HEIGHT;
			if(y < 0) y = 0;		
			if(y > (rc.bottom - rc.top))
				y = rc.bottom - rc.top;
		}
		else
		{
			y = 0;
			h = BSA_HS_HEIGHT;
			
			// space to scroll in
			s = rc.right - rc.left - 2 * BSA_HSX_WIDTH;
			if(s < 1) s = 1;
			
			// range of scroller
			r = psc->nMax - psc->nMin - psc->nPage;
			if(r < 1) r = 1;
			
			// width of scroller proportional to room avail
			w = s - r;			
			if(w < BSA_HSX_WIDTH)
				w = BSA_HSX_WIDTH;
			
			// take w out of room as new range of motion
			s -= w;
			if(s < 1) s = 1;
			
			// map nPos to new scoll range
			x = (psc->nPos - psc->nMin) * s / r;
			x += BSA_HSX_WIDTH;
			if(x < 0) x = 0;		
			if(x > (rc.right - rc.left))
				x = rc.right - rc.left;
		}
		MoveWindow(hWnd, x, y, w, h, FALSE);
		break;
			
	default:

		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

#endif
