//--------------------------------------------------------------------
//
// File: button.c
// Desc: button (label, menu buttons) controls for winapi
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------


#ifndef Windows
#include "winapix.h"

//-----------------------------------------------------------------------------

#define ALIGN_LEFT		1
#define ALIGN_RIGHT		2
#define ALIGN_CENTER	4
#define ALIGN_TOP		8
#define ALIGN_BOTTOM	16
#define ALIGN_VCENTER	32

#define BSA_BUT_PAD		24
#define BSA_BUT_MARG	4
#define BSA_CHECK_W		12
#define BSA_CHECK_H		12
#define BSA_CHECK_X		2
#define BSA_CTB_MARGX	2
#define BSA_CTB_MARGY	0
#define BSA_GROUPBOX_X	8
#define BSA_GROUPBOX_O	4

#define BSA_RADIO_ARC_H 14

extern LPZWND _zg_windows;
extern LPZWND _zg_dialogCtrl;
extern BOOL   _zg_keynav;

extern HBRUSH __w_solidbrush(DWORD xcolor);

//**************************************************************************
static void  __w_button_textout(
						HDC 	hdc,
						HWND 	hWnd,
						int 	alignflags,
						int 	lpad,
						int 	rpad,
						int		dent,
						int		iscur,
						LPCTSTR ftext
						)
{
	LPZWND		zWnd;
	int 		x, y;
	int 		l, lc, cp, ul, en;
	TEXTMETRIC 	tm;
	SIZE 		size;
	RECT		rcClient;
	TCHAR		text[256];
	
	GetTextMetrics(hdc, &tm);
	GetClientRect(hWnd, &rcClient);

	for(l = lc = cp = 0, ul = -1; ftext[cp] && cp < 255; cp++)
	{
		switch(ftext[cp])
		{
		default:
			text[l++] = ftext[cp];
			break;
		case _T('\r'):
			break;
		case _T('\n'):
			text[l++] = _T('\n');
			lc++;
			break;
		case _T('&'):
			ul = l;
			break;
		}
	}
	text[l] = 0;

	x = y = 0;
	
	zWnd = Zwindow(hWnd);
	
	GetTextExtentPoint32(hdc, text, l, &size);
	
	if(alignflags & ALIGN_RIGHT)
	{
		x = rcClient.right - rcClient.left + lpad - size.cx - rpad;
		if(x < 0) x = 0;
	}
	if(alignflags & ALIGN_LEFT)
	{
		x = lpad;
		if(x < 0) x = 0;
	}
	if(alignflags & ALIGN_CENTER)
	{
		x = lpad + (rcClient.right - rcClient.left - size.cx + 1) / 2;
		if(x < 0) x = 0;
	}
	if(alignflags & ALIGN_BOTTOM)
	{
		y = rcClient.bottom - rcClient.top - size.cy;
		if(y < 0) y = 0;
	}
	if(alignflags & ALIGN_VCENTER)
	{
		y = (rcClient.bottom - rcClient.top - size.cy * (lc + 1)) / 2;
		if(y < 0) y = 0;
	}
	if(x < 0) x = 0;
	if(y < 0) y = 0;
	
	// dotted lines around dialog item if key nav in use
	if((iscur > 0) && _zg_keynav)
	{
		HPEN hPen;
		HPEN hOld;

		hPen = CreatePen(PS_DOT, 0, RGB(0, 0, 0));
		hOld = SelectObject(hdc, hPen);

		// upper left to lower left
		MoveToEx(hdc, 		x - BSA_CTB_MARGX, 				y - BSA_CTB_MARGY - 1, NULL);
		LineTo(hdc, 		x - BSA_CTB_MARGX, 				y + size.cy + BSA_CTB_MARGY);
		
		// lower left to lower right
		MoveToEx(hdc, 		x - BSA_CTB_MARGX + 1, 			y + size.cy + BSA_CTB_MARGY, NULL);
		LineTo(hdc,			x + size.cx + BSA_CTB_MARGX - 1,y + size.cy + BSA_CTB_MARGY);
		
		// lower right to upper right
		MoveToEx(hdc,		x + size.cx + BSA_CTB_MARGX,	y + size.cy + BSA_CTB_MARGY, NULL);
		LineTo(hdc, 		x + size.cx + BSA_CTB_MARGX, 	y - BSA_CTB_MARGY - 1);
		
		// upper right to upper left
		MoveToEx(hdc, 		x + size.cx + BSA_CTB_MARGX - 1,y - BSA_CTB_MARGY - 1, NULL);
		LineTo(hdc, 		x - BSA_CTB_MARGX - 1,			y - BSA_CTB_MARGY - 1);
		
		SelectObject(hdc, hOld);
		DeleteObject(hPen);
	}
	if(dent) { x+= 2; y+= 2; }

	if(zWnd && (GetWindowLong(zWnd, GWL_STYLE) & WS_DISABLED))
	{
		en = FALSE;
		SetTextColor(hdc, _z_rgb_colorofbrush(GetStockObject(DKGRAY_BRUSH)));
	}
	else
		en = TRUE;
	
	if(lc > 0)
	{
		LPTSTR pt, pxt;
		
		pt = text;
		do
		{
			pxt = _tcsstr(pt, _T("\n"));
			if(pxt)
			{
				*pxt = _T('\0');
				TextOut(hdc, x, y, pt, (pxt - pt));
				y += size.cy;
				pt = pxt + 1;
			}
		}
		while(pxt);
		if(pt && *pt)
		{
			TextOut(hdc, x, y, pt, _tcslen(pt));
		}
	}
	else
	{
		TextOut(hdc, x, y, text, l);
	}
	if(ul >= 0)
	{
		HPEN hPen;
		HPEN hOld;
		SIZE sizeX;

		if(ul > 0)
			GetTextExtentPoint32(hdc, text, ul, &sizeX);
		else
			sizeX.cx = 0;
		GetTextExtentPoint32(hdc, &text[ul], 1, &size);
		hPen = CreatePen(PS_SOLID, 0, GetSysColor((en?COLOR_BTNTEXT:DKGRAY_BRUSH)));
		hOld = SelectObject(hdc, hPen);
		MoveToEx(hdc, x + sizeX.cx, y + size.cy, NULL);
		LineTo(hdc, x + sizeX.cx + size.cx, y + size.cy);
		SelectObject(hdc, hOld);
		DeleteObject(hPen);
	}
}

//**************************************************************************
void __w_button_box(HDC hdc, int pushed, LPRECT rc)
{
	HPEN		hPen;
	HPEN		hOld;
	
	// br outside
	hPen = CreatePen(PS_SOLID, 0, GetSysColor(pushed ? COLOR_3DLIGHT : COLOR_3DDKSHADOW));
	hOld = (HPEN)SelectObject(hdc, hPen);
	MoveToEx(hdc, rc->left, rc->bottom, NULL);
	LineTo(hdc, rc->right, rc->bottom);
	LineTo(hdc, rc->right, rc->top-1);
	SelectObject(hdc, hOld);	
	DeleteObject(hPen);
	
	// ul outside
	hPen = CreatePen(PS_SOLID, 0, GetSysColor(pushed ? COLOR_3DDKSHADOW : COLOR_BTNHIGHLIGHT));
	hOld = (HPEN)SelectObject(hdc, hPen);
	MoveToEx(hdc, rc->right-1, rc->top, NULL);
	LineTo(hdc, rc->left, rc->top);
	LineTo(hdc, rc->left, rc->bottom);
	SelectObject(hdc, hOld);	
	DeleteObject(hPen);
	
	// br inside
	hPen = CreatePen(PS_SOLID, 0, GetSysColor(pushed ? COLOR_BTNHIGHLIGHT : COLOR_BTNSHADOW));
	hOld = (HPEN)SelectObject(hdc, hPen);
	MoveToEx(hdc, rc->left + 1, rc->bottom - 1, NULL);
	LineTo(hdc, rc->right - 1, rc->bottom - 1);
	LineTo(hdc, rc->right - 1, rc->top);
	SelectObject(hdc, hOld);	
	DeleteObject(hPen);
	
	// ul inside
	hPen = CreatePen(PS_SOLID, 0, GetSysColor(pushed ? COLOR_BTNSHADOW : COLOR_3DLIGHT));
	hOld = (HPEN)SelectObject(hdc, hPen);
	MoveToEx(hdc, rc->right - 2, rc->top + 1, NULL);
	LineTo(hdc, rc->left + 1, rc->top + 1);
	LineTo(hdc, rc->left + 1, rc->bottom - 1);
	SelectObject(hdc, hOld);
	DeleteObject(hPen);
}

//**************************************************************************
void __w_checkbox(HDC hdc, int pushed, int flat, int enabled, LPRECT rc)
{
	HPEN		hpen;
	HPEN		hpenold;

	if(! flat)
	{
		// br outside
		hpen = (HPEN)GetStockObject(WHITE_PEN);
		hpenold = (HPEN)SelectObject(hdc, hpen);
		MoveToEx(hdc, rc->left, rc->bottom, NULL);
		LineTo(hdc, rc->right, rc->bottom);
		LineTo(hdc, rc->right, rc->top);
		SelectObject(hdc, hpenold);	
		
		// ul outside
		hpen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNSHADOW));
		hpenold = (HPEN)SelectObject(hdc, hpen);
		MoveToEx(hdc, rc->right, rc->top, NULL);
		LineTo(hdc, rc->left, rc->top);
		LineTo(hdc, rc->left, rc->bottom);
		SelectObject(hdc, hpenold);	
		DeleteObject(hpen);
		
		// br inside
		hpen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNFACE));
		hpenold = (HPEN)SelectObject(hdc, hpen);
		MoveToEx(hdc, rc->left + 1, rc->bottom - 1, NULL);
		LineTo(hdc, rc->right - 1, rc->bottom - 1);
		LineTo(hdc, rc->right - 1, rc->top);
		SelectObject(hdc, hpenold);	
		DeleteObject(hpen);
		
		// ul inside
		hpen = (HPEN)GetStockObject(BLACK_PEN);
		hpenold = (HPEN)SelectObject(hdc, hpen);
		MoveToEx(hdc, rc->right - 2, rc->top + 1, NULL);
		LineTo(hdc, rc->left + 1, rc->top + 1);
		LineTo(hdc, rc->left + 1, rc->bottom - 1);
		SelectObject(hdc, hpenold);
		
		// middle box
		rc->top += 2;
		rc->left += 2;
		rc->bottom -= enabled ? 1 : 2;
		rc->right -= enabled ? 1 : 2;
		FillRect(hdc, rc, enabled ? (HBRUSH)GetStockObject(WHITE_BRUSH) : GetSysColorBrush(COLOR_BTNFACE));
	}
	// check if pushed
	if(pushed)
	{
		int x = rc->left + 1, y = rc->top + (rc->bottom - rc->top - 2) / 2, h = 3;
		
		hpen = (HPEN)GetStockObject(BLACK_PEN);
		hpenold = (HPEN)SelectObject(hdc, hpen);
		
		MoveToEx(hdc, x, y, NULL);
		LineTo(hdc, x++, y++ + h);
		MoveToEx(hdc, x, y, NULL);
		LineTo(hdc, x++, y++ + h);
		MoveToEx(hdc, x, y, NULL);
		LineTo(hdc, x++, y + h);
		MoveToEx(hdc, x, --y, NULL);
		LineTo(hdc, x++, y-- + h);
		MoveToEx(hdc, x, y, NULL);
		LineTo(hdc, x++, y-- + h);
		MoveToEx(hdc, x, y, NULL);
		LineTo(hdc, x++, y-- + h);
		MoveToEx(hdc, x, y, NULL);
		LineTo(hdc, x++, y-- + h);
		SelectObject(hdc, hpenold);
	}
}	

//**************************************************************************
void __w_radiobox(HDC hdc, int pushed, int flat, int enabled, LPRECT rc)
{
	HPEN		hpen;
	HPEN		hpenold;
	int			x,y, w, h;
	
	LPZGC		zGC;
	
	zGC = (LPZGC)hdc;
	
	if(flat) 
	{
		__w_checkbox(hdc, pushed, flat, enabled, rc);
		return;
	}
	x = rc->left;
	y = rc->top;
	h = rc->bottom - rc->top;
	w = h;
	
	do 
	{
		x++;
		y++;
		h -= 2;
		w -= 2;
	}
	while(h > BSA_RADIO_ARC_H);

	if(! flat)
	{
		// middle filled circle
		hpen = (HPEN)GetStockObject(WHITE_PEN);
		hpenold = (HPEN)SelectObject(hdc, hpen);
#if 1
		if(zGC->zDrvr && zGC->zDrvr->drvrData[0])
		{
#ifdef X11
			GC xGC;
			
			xGC = (GC)zGC->zDrvr->drvrData[0];
			XSetArcMode(_zg_display, xGC, ArcPieSlice);
			XFillArc(_zg_display, zGC->zWnd->xWnd, xGC, x, y - 1, w, h, 0, 64*360);
	   		XSetArcMode(_zg_display, xGC, ArcChord);
#endif
		}
#endif
		h += y;
				
		// br outside
		MoveToEx(hdc, x + 2, h - 2, NULL);
		LineTo(hdc, x + 4, h - 2);
		MoveToEx(hdc, x + 4, h - 1, NULL);
		LineTo(hdc, x + 8, h - 1);
		MoveToEx(hdc, x + 8, h - 2, NULL);
		LineTo(hdc, x + 10, h - 2);
		MoveToEx(hdc, x + 10, h - 3, NULL);
		LineTo(hdc, x + 10, h - 5);
		MoveToEx(hdc, x + 11, h - 5, NULL);
		LineTo(hdc, x + 11, h - 9);
		MoveToEx(hdc, x + 10, h - 9, NULL);
		LineTo(hdc, x + 10, h - 11);
		
		SelectObject(hdc, hpenold);	
		
		// ul outside
		hpen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNSHADOW));
		hpenold = (HPEN)SelectObject(hdc, hpen);
		MoveToEx(hdc, x + 9, h - 11, NULL);
		LineTo(hdc, x + 7, h - 11);
		MoveToEx(hdc, x + 7, h - 12, NULL);
		LineTo(hdc, x + 3, h - 12);
		MoveToEx(hdc, x + 3, h - 11, NULL);
		LineTo(hdc, x + 1, h - 11);
		MoveToEx(hdc, x + 1, h - 10, NULL);
		LineTo(hdc, x + 1, h - 8);
		MoveToEx(hdc, x, h - 8, NULL);
		LineTo(hdc, x, h - 4);
		MoveToEx(hdc, x + 1, h - 4, NULL);
		LineTo(hdc, x + 1, h - 2);
		
		SelectObject(hdc, hpenold);	
		DeleteObject(hpen);

		// br inside
		hpen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNFACE));
		hpenold = (HPEN)SelectObject(hdc, hpen);
		MoveToEx(hdc, x + 2, h - 3, NULL);
		LineTo(hdc, x + 4, h - 3);
		MoveToEx(hdc, x + 4, h - 2, NULL);
		LineTo(hdc, x + 8, h - 2);
		MoveToEx(hdc, x + 8, h - 3, NULL);
		LineTo(hdc, x + 10, h - 3);
		MoveToEx(hdc, x + 9, h - 3, NULL);
		LineTo(hdc, x + 9, h - 5);
		MoveToEx(hdc, x + 10, h - 5, NULL);
		LineTo(hdc, x + 10, h - 9);
		MoveToEx(hdc, x + 9, h - 9, NULL);
		LineTo(hdc, x + 9, h - 10);
		SelectObject(hdc, hpenold);	
		DeleteObject(hpen);
		
		// ul inside
		hpen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DDKSHADOW));
		hpenold = (HPEN)SelectObject(hdc, hpen);
		MoveToEx(hdc, x + 9, h - 10, NULL);
		LineTo(hdc, x + 7, h - 10);
		MoveToEx(hdc, x + 7, h - 11, NULL);
		LineTo(hdc, x + 3, h - 11);
		MoveToEx(hdc, x + 3, h - 10, NULL);
		LineTo(hdc, x + 1, h - 10);
		MoveToEx(hdc, x + 2, h - 10, NULL);
		LineTo(hdc, x + 2, h - 8);
		MoveToEx(hdc, x + 1, h - 8, NULL);
		LineTo(hdc, x + 1, h - 4);
		MoveToEx(hdc, x + 2, h - 4, NULL);
		LineTo(hdc, x + 2, h - 3);
		SelectObject(hdc, hpenold);	
		DeleteObject(hpen);
	}
	// check if pushed
	if(pushed)
	{
		x += w / 2;
		y = h / 2 - 1;
		
		hpen = (HPEN)GetStockObject(BLACK_PEN);
		hpenold = (HPEN)SelectObject(hdc, hpen);
		MoveToEx(hdc, x, y, NULL);
		LineTo(hdc, x+2, y++);
		MoveToEx(hdc, x-1, y, NULL);
		LineTo(hdc, x+3, y++);
		MoveToEx(hdc, x-1, y, NULL);
		LineTo(hdc, x+3, y++);
		MoveToEx(hdc, x, y, NULL);
		LineTo(hdc, x+2, y++);
		SelectObject(hdc, hpenold);
	}
}	
	
//**************************************************************************
static void __w_groupbox(HDC hdc, LPCTSTR text, LPRECT rc)
{
	HPEN		hpen;
	HPEN		hpenold;

	int 		x, y;
	int 		l, h;
	SIZE 		size;
	
	x = y = 0;
	l = _tcslen(text);
	
	// get text length
	//
	GetTextExtentPoint32(hdc, text, l, &size);
	
	h = size.cy / 2;
	
	// outside
	hpen = CreatePen(PS_SOLID, 0, RGB(63,63,63));
	hpenold = (HPEN)SelectObject(hdc, hpen);
	MoveToEx(hdc, rc->left + BSA_GROUPBOX_X, rc->top + h, NULL);
	LineTo(hdc, rc->left, rc->top + h);
	LineTo(hdc, rc->left, rc->bottom - 1);
	LineTo(hdc, rc->right - 1, rc->bottom - 1);
	LineTo(hdc, rc->right - 1, rc->top + h);
	LineTo(hdc, rc->left + BSA_GROUPBOX_X + 2 * BSA_GROUPBOX_O + size.cx, rc->top + h);
	SelectObject(hdc, hpenold);	
	DeleteObject(hpen);
	
	// inside
	hpen = (HPEN)GetStockObject(WHITE_PEN);
	hpenold = (HPEN)SelectObject(hdc, hpen);
	MoveToEx(hdc, rc->left + 1 + BSA_GROUPBOX_X, rc->top + h + 1, NULL);
	LineTo(hdc, rc->left + 1, rc->top + h + 1);
	LineTo(hdc, rc->left + 1, rc->bottom - 2);
	MoveToEx(hdc, rc->left, rc->bottom, NULL);
	LineTo(hdc, rc->right, rc->bottom);
	LineTo(hdc, rc->right, rc->top + h);
	MoveToEx(hdc, rc->right - 2, rc->top + h + 1, NULL);
	LineTo(hdc, rc->left + BSA_GROUPBOX_X + 2 * BSA_GROUPBOX_O + size.cx, rc->top + h + 1);
	SelectObject(hdc, hpenold);	
	
	// text
	TextOut(hdc, rc->left + BSA_GROUPBOX_O + BSA_GROUPBOX_X, rc->top, text, l);

}

//**************************************************************************
static void __w_setcheck(LPZWND zWnd, BOOL check)
{
	LPZWND group;
	LPZWND ctrl;
	DWORD  cstyle;

	SETPUSHED((HWND)zWnd, check);
	
	// uncheck any other buttons in the group.  For dialogs
	// this is easy, since the controls are linked, but for
	// child controls in non-dialog windows, need to go
	// through the whole list
	//
	if(zWnd->cnext || zWnd->cprev)
	{
		for(group = NULL, ctrl = zWnd; ctrl; ctrl = ctrl->cprev)
		{
			cstyle = GetWindowLong((HWND)ctrl, GWL_STYLE);
			if(((cstyle & WS_GROUP) == WS_GROUP)||(! ctrl->cprev))
			{
				group = ctrl;
				break;
			}
		}
		if(group)
		{
			ctrl = group;
			cstyle = GetWindowLong((HWND)ctrl, GWL_STYLE);
			
			while(ctrl)
			{
				if(ctrl != zWnd)
				{
					if(
							((cstyle & 0xf) == BS_AUTORADIOBUTTON) ||
							((cstyle & 0xf) == BS_RADIOBUTTON)
					)
					{
						SetWindowLong(ctrl, GWL_PRIVDATA, 0);
						InvalidateRect(ctrl, NULL, FALSE);
					}
				}
				ctrl = ctrl->cnext;
				if(ctrl)
				{
					cstyle = GetWindowLong((HWND)ctrl, GWL_STYLE);
					if((cstyle & WS_GROUP) == WS_GROUP) break;
				}
			}
		}
	}
	else
	{
		LPZWND parent;
		
		group = zWnd;
		for(ctrl = _zg_windows; ctrl && ctrl != zWnd; ctrl = ctrl->next)
		{
			cstyle = GetWindowLong((HWND)ctrl, GWL_STYLE);
			if((cstyle & WS_GROUP) == WS_GROUP)
			{
				group = ctrl;
				break;
			}
		}
		// window list is from newest to oldest, so find the
		// newest window that shares the same parent as group
		//
		for(ctrl = _zg_windows, parent = (HWND)GetWindowLong(group, GWL_HWNDPARENT); ctrl; ctrl = ctrl->next)
		{
			if((HWND)GetWindowLong(ctrl, GWL_HWNDPARENT) == parent)
				break;
			if(ctrl == group)
				break;
		}
		for(;ctrl; ctrl = ctrl->next)
		{
			cstyle = GetWindowLong((HWND)ctrl, GWL_STYLE);
			
			if(ctrl != group)
			{
				if((cstyle & WS_GROUP) == WS_GROUP)
					break;
				if(GetParent(ctrl) != parent)
					break;
			}
			if(ctrl != zWnd)
			{
				if(
						((cstyle & 0xf) == BS_AUTORADIOBUTTON) ||
						((cstyle & 0xf) == BS_RADIOBUTTON)
				)
				{
					SetWindowLong(ctrl, GWL_PRIVDATA, 0);
					InvalidateRect(ctrl, NULL, FALSE);
				}
			}
		}
	}
}

//**************************************************************************
void __w_drawbutton(HDC hdc, HWND hWnd, DWORD style, LPCTSTR text, BOOL pushed, BOOL iscur)
{
	LPZGC		zGC;
	HBRUSH		hBrush;
	int			alignment;
	RECT		rc;
	RECT		brc;
	int			lpad;
	int			rpad;
	int			dent;
	BOOL		notext;
	
	if(! (zGC = (LPZGC)hdc)) return;
	
	alignment 	= ALIGN_VCENTER;
	lpad = rpad = 0;
	dent 		= 0;
	notext		= FALSE;

	GetClientRect(hWnd, &rc);
	
	switch(style & 0xF)
	{
	case BS_PUSHBUTTON:
	case BS_DEFPUSHBUTTON:
		
		alignment = ALIGN_VCENTER;
		if(style & BS_LEFTTEXT)
		{
			alignment |= ALIGN_LEFT;
			lpad = 3;
		}
		else
			alignment |= ALIGN_CENTER;
		dent      = (style & BS_FLAT) ? 0 : pushed;
		break;
		
	case BS_CHECKBOX:
	case BS_AUTOCHECKBOX:
	case BS_RADIOBUTTON:
	case BS_AUTORADIOBUTTON:
		
		if(style & BS_LEFTTEXT)
		{
			alignment = ALIGN_LEFT | ALIGN_VCENTER;
			rpad = 0;
			lpad = BSA_BUT_MARG;
		}
		else
		{
			alignment = ALIGN_LEFT | ALIGN_VCENTER;
			lpad = BSA_BUT_PAD;
			if(style & BS_FLAT)
				lpad -= 6;
			rpad = BSA_BUT_MARG;
		}
		break;
		
	default:
		
		notext = TRUE;
		break;
	}
	//printf("st=%08X align=%08X\n", style, alignment);
	
	switch(style & 0xF)
	{
	case BS_PUSHBUTTON:
	case BS_DEFPUSHBUTTON:
		
		rc.right--;
		rc.bottom--;
		
		brc.left = rc.left;
		brc.top  = rc.top;
		brc.right = rc.right;
		brc.bottom = rc.bottom;
		if(! (style & BS_FLAT) || pushed)
		{
			brc.left+=2;
			brc.top +=2;
			brc.right -=2;
			brc.bottom -= 2;
			__w_button_box(hdc, pushed, &rc);
		}
		hBrush =  CreateSolidBrush(GetBkColor(hdc));
		FillRect(hdc, &brc, hBrush);
		DeleteObject(hBrush);
		break;
		
	case BS_CHECKBOX:
	case BS_AUTOCHECKBOX:
	case BS_RADIOBUTTON:
	case BS_AUTORADIOBUTTON:
		
		hBrush =  CreateSolidBrush(GetBkColor(hdc));
		FillRect(hdc, &rc, hBrush);
		DeleteObject(hBrush);
		brc.top = (rc.bottom - rc.top - BSA_CHECK_H) / 2;
		brc.bottom = brc.top + BSA_CHECK_H;
		if(style & BS_LEFTTEXT)
			brc.left = rc.right - rc.left - BSA_CHECK_W - BSA_CHECK_X;
		else
			brc.left = BSA_CHECK_X;
		brc.right = brc.left + BSA_CHECK_W;
		switch(style & 0xF)
		{
		case BS_CHECKBOX:
		case BS_AUTOCHECKBOX:
			__w_checkbox(hdc, pushed, (style & BS_FLAT), !(style & WS_DISABLED), &brc);
			break;
		case BS_RADIOBUTTON:
		case BS_AUTORADIOBUTTON:
			__w_radiobox(hdc, pushed, (style & BS_FLAT), !(style & WS_DISABLED), &brc);
			break;
		}
		break;
		
	case BS_GROUPBOX:
		
		__w_groupbox(hdc, text, &rc);
		break;
		
	default:
		break;
	}
	if(! notext)
		__w_button_textout(hdc, hWnd, alignment, lpad, rpad, dent, iscur, text);
}

//**************************************************************************
LRESULT WINAPI __wproc_Label(HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{
	PAINTSTRUCT ps;
	HDC         hdc;
	LPZWND		zWnd;
	TCHAR		text[256];
	DWORD		style;
	int			align, border;
	
	if(! (zWnd = Zwindow(hWnd))) return -1;
	
	switch(message)
	{
	case WM_CREATE:
		
		break;
		
	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		style = GetWindowLong(hWnd, GWL_STYLE);
		FillRect(hdc, &ps.rcPaint, zWnd->bkg);
		if(GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_CLIENTEDGE)
		{
			RECT rc;
			
			GetClientRect(hWnd, &rc);
			rc.right--;
			rc.bottom--;
			__w_button_box(hdc, 1, &rc);
			border = 1;
		}
		else if(style & WS_BORDER)
		{
			RECT rc;
			HPEN hop;
			
			hop = (HPEN)SelectObject(hdc, GetStockObject(BLACK_PEN));
			GetClientRect(hWnd, &rc);
			MoveToEx(hdc, 0, 0, NULL);
			LineTo(hdc, rc.right - rc.left - 1, 0);
			LineTo(hdc, rc.right - rc.left - 1, rc.bottom - rc.top - 1);
			LineTo(hdc, 0, rc.bottom - rc.top - 1);
			LineTo(hdc, 0, 0);
			SelectObject(hdc, hop);
			border = 1;
		}
		else
		{
			border = 0;
		}
		GetWindowText(hWnd, text, 256);
		align = ALIGN_VCENTER;
		if(style & BS_CENTER)
			align |= ALIGN_CENTER;
		else if(! (style & BS_RIGHT))
			align |= ALIGN_LEFT;
		__w_button_textout(hdc, hWnd, align, border ? 2 : 0, 0, 0, 0, text);
		EndPaint(hWnd, &ps);
		break;
		
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//**************************************************************************
LRESULT WINAPI __wproc_Icon(HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{
	PAINTSTRUCT ps;
	HDC         hdc;
	LPZWND		zWnd;
	
	if(! (zWnd = Zwindow(hWnd))) return -1;
	
	switch(message)
	{
	case WM_CREATE:
		
		SetWindowLong(hWnd, GWL_USERDATA,
				(LPARAM)((LPCREATESTRUCT)lParam)->lpCreateParams);
		break;
		
	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		DrawIcon(hdc, 0, 0,  (HICON)GetWindowLong(hWnd, GWL_USERDATA));
		EndPaint(hWnd, &ps);
		break;
		
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//**************************************************************************
LRESULT WINAPI __wproc_Bitmap(HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{
	PAINTSTRUCT ps;
	HDC         hdc;
	LPZWND		zWnd;
	
	if(! (zWnd = Zwindow(hWnd))) return -1;
	
	switch(message)
	{
	case WM_CREATE:
		
		SetWindowLong(hWnd, GWL_USERDATA,
				(LPARAM)((LPCREATESTRUCT)lParam)->lpCreateParams);
		break;
		
	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		{
			HBITMAP hbm  = (HBITMAP)GetWindowLong(hWnd, GWL_USERDATA);
			HDC     hdcc = CreateCompatibleDC(hdc);
			HBITMAP ho = (HBITMAP)SelectObject(hdcc, hbm);
			BITMAP	bmp;

			if(GetObject(hbm, sizeof(bmp), &bmp))
			{
				BitBlt(hdc, 0, 0, bmp.bmWidth, bmp.bmHeight, hdcc, 0, 0, SRCCOPY);
			}
			SelectObject(hdcc, ho);
			DeleteDC(hdcc);
		}
		EndPaint(hWnd, &ps);
		break;
		
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//**************************************************************************
LRESULT WINAPI __wproc_Button(HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{
	PAINTSTRUCT ps;
	HDC         hdc;
	int			pushed;
	LPZWND		zWnd;
	TCHAR		text[256];
	DWORD		style;
	int			iscur;
	int			enabled;
	TRACKMOUSEEVENT me;
	
	if(! (zWnd = Zwindow(hWnd))) return -1;
	pushed  = PUSHED(hWnd);
	style  	= GetWindowLong(hWnd, GWL_STYLE);
	enabled = ! (style & WS_DISABLED);

	switch(message)
	{
	case WM_CREATE:
		
		SetWindowLong(hWnd, GWL_PRIVDATA, 0);
		break;
		
	case BM_GETCHECK:
		
		return pushed ? BST_CHECKED : BST_UNCHECKED;
		
	case BM_SETCHECK:

		switch(style & 0xF)
		{
		case BS_CHECKBOX:
		case BS_AUTOCHECKBOX:
		case BS_RADIOBUTTON:
		case BS_AUTORADIOBUTTON:

			if(wParam == BST_CHECKED)
			{
				if(! pushed)
					__w_setcheck(zWnd, 1);
			}
			else
			{
				if(pushed)
					__w_setcheck(zWnd, 0);
			}
			InvalidateRect(hWnd, NULL, FALSE);
		}
		return 0;
		
	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		GetWindowText(hWnd, text, 256);
	
		// see if this is the "current" control of
		// any active dialog
		//
		iscur = (zWnd == _zg_dialogCtrl);	
		__w_drawbutton(hdc, hWnd, style, text, pushed, iscur);		
		EndPaint(hWnd, &ps);
		break;

	case WM_KEYDOWN:
	case WM_KEYUP:
		
		PostMessage(GetParent(hWnd), message, wParam, lParam);
		break;
		
	case WM_LBUTTONDOWN:

		if(enabled)
		{	
			SETENTERED(hWnd, TRUE);
			
			switch(style & 0xf)
			{
			case BS_AUTORADIOBUTTON:
	
				__w_setcheck(zWnd, 1);
				break;
					
			case BS_AUTOCHECKBOX:
				
				__w_setcheck(zWnd, ! pushed);
				break;
				
			case BS_CHECKBOX:
			case BS_RADIOBUTTON:
			default:
			
				SETPUSHED(hWnd, ! pushed);
			}
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
		
	case WM_LBUTTONUP:

		if(enabled)
		{
			HWND hwndParent = GetParent(hWnd);
			
			if(ENTERED(hWnd) && (PUSHED(hWnd) || (style & BS_CHECKBOX)))
				PostMessage(hwndParent, WM_COMMAND, GetWindowLong(hWnd, GWL_ID), 0);

			if((style & 0xF) <= 1)
			{
				SETPUSHED(hWnd, FALSE);
				InvalidateRect(hWnd, NULL, FALSE);
			}
		}
		break;
		
	case WM_MOUSEMOVE:

		if(enabled)
		{
			me.dwFlags = TME_QUERY;
			me.hwndTrack = NULL;
			TrackMouseEvent(&me);
			
			if(! (me.dwFlags & TME_LEAVE))
			{
				//printf("in\n");
				SETENTERED(hWnd, TRUE);
				me.dwFlags = TME_HOVER | TME_LEAVE;
				me.dwHoverTime = HOVER_DEFAULT;
				me.hwndTrack = hWnd;
				TrackMouseEvent(&me);
			}
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
		
	case WM_MOUSELEAVE:
	
		if(
				((style & 0xf) != BS_AUTORADIOBUTTON)	&&
				((style & 0xf) != BS_AUTOCHECKBOX)
		)
			SETPUSHED(hWnd, FALSE);
		SETENTERED(hWnd, FALSE);
		//printf("out\n");
		me.dwFlags = TME_CANCEL | TME_HOVER | TME_LEAVE;
		me.hwndTrack = NULL;
		TrackMouseEvent(&me);
		InvalidateRect(hWnd, NULL, FALSE);
		break;
		
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

#endif
