//--------------------------------------------------------------------
//
// File: editbox.c
// Desc: edit controls for winapi
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------


#ifndef Windows
#include "winapix.h"

#define dtDraw			1
#define dtSetCaret		2
#define dtSetPosToMouse	4

extern HWND	_zg_dialogWnd[];
extern int	_zg_dialogTop;

#define BSA_MAX_EB_TEXT	2048 /* 32K according to Windows! */

//-----------------------------------------------------------------------------

typedef struct tagEditBox
{
	int		curline;
	int		curcol;
	int		topline;
	int		leftcol;
	int		qla, qlb, qca, qcb;
	int		mousex, mousey;
	int		align, lpad, rpad, tpad;
	int		shift, ctrl;
	DWORD	style;
	DWORD	exstyle;
	TCHAR	text[BSA_MAX_EB_TEXT];
	int		nText;
}
_w_editbox;

//**************************************************************************
static void  _w_edit_textout(
						HDC 	hdc,
						HWND 	hWnd,
						int		type,
						_w_editbox* pBox
						)
{
	LPZWND		zWnd;
	RECT		rcc;
	RECT		rcl;
	RECT*		lprcUpdate;
	TEXTMETRIC	tm;
	LPCTSTR		lpText;
	LPCTSTR		lpBase;
	COLORREF	curFrgColor, curBkgColor;
	COLORREF	frgColor, bkgColor, tbkgColor, tfrgColor;
	int			nText;
	int			lines, cols;
	int			line, incol;
	int			qla, qlb,  qca,    qcb;
	int			x, y;
	int			caretX, caretY, caretH;
	int			setline, setcol;
	int			caretLine;
	int			yi, yo;
	int 		mca, mcb;

	int			nPending;
	int			pwidth, lwidth;
	SIZE		sizeChar;

	TCHAR		pwText[BSA_MAX_EB_TEXT];
	LPTSTR		pText;

	if(! hdc || ! hWnd) return;
	if(! (zWnd = Zwindow(hWnd))) return;

	GetClientRect(hWnd, &rcc);
	lprcUpdate = &rcc;

	GetTextMetrics(hdc, &tm);
	yi = tm.tmHeight;

	if(pBox->style & ES_PASSWORD)
	{
		x = pBox->nText - 1;
		if(x < 0) x = 0;
		pwText[x] = 0;
		while(--x >= 0)
			pwText[x] = _T('*');
		pText = pwText;
	}
	else
	{
		pText = pBox->text;
	}
	if(type == dtSetCaret)
	{
		if (pBox->curcol >= pBox->leftcol)
			GetTextExtentPoint32(hdc, pText + pBox->leftcol - 1, pBox->curcol - pBox->leftcol + 1, &sizeChar);
		else
			sizeChar.cx = 1;
		if(sizeChar.cx < 1)
			sizeChar.cx = 1;
		lines = 1;

		if((pBox->curcol < pBox->leftcol) || (sizeChar.cx >= (rcc.right - rcc.left)))
		{
			GetTextExtentPoint32(hdc, _T("m"), 1, &sizeChar);
			if(sizeChar.cx < 1)
				sizeChar.cx = 1;
			cols = (rcc.right - rcc.left) / sizeChar.cx;
			pBox->leftcol = pBox->curcol - cols / 2;
			if(pBox->leftcol < cols / 2)
				pBox->leftcol = 1;
			InvalidateRect(hWnd, NULL, FALSE);
		}
	}

	yo = (rcc.bottom - rcc.top - yi) / 2;
	if(yo < 0) yo = 0;
	yo += pBox->tpad;

	caretX = caretY = -1;
	setline = setcol = -1;

	//_tprintf(_T("xqla %d,%d  xqlb %d,%d\n"), pBox->qla, pBox->qca, pBox->qlb, pBox->qcb);
	if(pBox->qla <= pBox->qlb)
	{
		qla = pBox->qla;
		qlb = pBox->qlb;
		if(pBox->qla < pBox->qlb || pBox->qca <= pBox->qcb)
		{
			qca = pBox->qca;
			qcb = pBox->qcb;
		}
		else
		{
			qca = pBox->qcb;
			qcb = pBox->qca;
		}
	}
	else
	{
		qla = pBox->qlb;
		qlb = pBox->qla;
		qca = pBox->qcb;
		qcb = pBox->qca;
	}
	//_tprintf(_T("qla %d,%d  qlb %d,%d\n"), qla, qca, qlb, qcb);

	curFrgColor = _z_rgb_colorofbrush(zWnd->frg);
	if(GetWindowLong(hWnd, GWL_STYLE) & WS_DISABLED)
		curBkgColor = _z_rgb_colorofbrush(GetSysColorBrush(COLOR_BTNFACE));
	else
		curBkgColor = _z_rgb_colorofbrush(zWnd->bkg);

	bkgColor = curBkgColor;
	frgColor = curFrgColor;

	SetTextColor(hdc, curFrgColor);
	SetBkColor(hdc, curBkgColor);

	incol  = 1;

	lpText 	= pText;
	nText  	= pBox->nText;
	lpBase	= lpText;

	x = rcc.left + pBox->lpad;

	caretLine = pBox->topline == pBox->curline;
	if(caretLine) caretY = rcc.top + yo;

	for(y = rcc.top + yo, line = pBox->topline; y < rcc.bottom && incol < nText; y += yi, line++)
	{
		caretLine = (line == pBox->curline);
		if(caretLine) caretY = y;

		pwidth 	= 0;
		lwidth = 0;
		mca		= 0;
		mcb		= 0;
		x 		= rcc.left + pBox->lpad;

		if(pBox->qla > 0 && pBox->qlb > 0)
		{
			if(line == qla)
				mca = qca;
			else if(line > qla && line <= qlb)
				mca = 1;
			if(line == qlb)
				mcb = qcb;
			else if(line >= qla && line < qlb)
				mcb = 0x7fffffff;
		}
		nPending = 0;

		while(incol < nText)
		{
			if(incol >= mca && incol < mcb)
			{
				tbkgColor = RGB(0, 0, 127);
				tfrgColor = RGB(255, 255, 255);
			}
			else
			{
				tbkgColor = bkgColor;
				tfrgColor = frgColor;
			}

			if(tbkgColor != curBkgColor || tfrgColor != curFrgColor)
			{
				if(nPending > 0)
				{
					if(type == dtDraw && y >= lprcUpdate->top && y < lprcUpdate->bottom)
					{
						TextOut(hdc, x, y, lpBase, nPending);
					}
					x += pwidth;
					lpBase = lpText + incol - 1;
					pwidth = 0;
					nPending = 0;
				}
				SetTextColor(hdc, tfrgColor);
				SetBkColor(hdc, tbkgColor);
				curBkgColor = tbkgColor;
				curFrgColor = tfrgColor;
			}
			if(caretLine && caretX < 0 && incol == pBox->curcol)
			{
				//_tprintf(_T("tc=%d  mcc=%d x=%d pw=%d\n"), incol, pBox->curcol, x, pwidth);
				caretX = x + pwidth;
				caretH = sizeChar.cy;
			}

			GetTextExtentPoint32(hdc, lpText, incol, &sizeChar);

			if(type == dtSetPosToMouse && y <= pBox->mousey && (y + yi) > pBox->mousey)
			{
				if((x + pwidth) <= pBox->mousex && (x + (pwidth - lwidth) + sizeChar.cx) > pBox->mousex)
				{
					setline = line;
					setcol  = incol;
					break; // no need to go on
				}
			}
			if(incol >= pBox->leftcol)
			{
				pwidth += sizeChar.cx - lwidth;
				nPending++;
			}
			else
			{
				lpBase = lpText + incol;
			}
			lwidth = sizeChar.cx;
			incol++;
			if(! lpText[incol-1] || lpText[incol-1] == _T('\r') || lpText[incol-1] == _T('\n'))
				break;
			if(type == dtSetPosToMouse && setcol >= 0)
				break;
			if(type == dtSetCaret && caretX >= 0)
				break;
		}
		// end of line

		if(nPending > 0 && type == dtDraw && y >= lprcUpdate->top && y < lprcUpdate->bottom)
		{
			TextOut(hdc, x, y, lpBase, nPending);
		}
		x += pwidth;

		if(type == dtSetPosToMouse && y <= pBox->mousey && (y + yi) > pBox->mousey)
		{
			setline = line;
			if(setcol < 0)
				setcol = incol;
			break;
		}
		//printf("caretl=%d l=%d pbl=%d %d,%d (was %d,%d)\n", caretLine, line, pBox->curline, x, y, caretX,caretY);
		if(type == dtSetCaret && caretLine)
		{
			if(caretY < 0)
				caretY = y;
			if(caretX < 0)
				caretX = x;
			if(caretH < 0)
				caretH = yi;
			break;
		}
		if(type == dtDraw && y >= lprcUpdate->top && y < lprcUpdate->bottom)
		{
			HBRUSH hbrBkg = CreateSolidBrush(bkgColor);

			rcl.top		= y;
			rcl.bottom	= y + yi;
			rcl.left	= x;
			rcl.right	= rcc.right - pBox->rpad + 2;

			FillRect(hdc, &rcl, hbrBkg);
			DeleteObject(hbrBkg);
		}
		if(lpText[incol] == _T('\r') || lpText[incol] == _T('\n'))
		{
			line++;
			incol++;
		}
		lpBase = lpText + incol - 1;
	}
	// end of all lines

	if(type == dtSetCaret && caretLine)
	{
		if(caretY < 0)
			caretY = y;
		if(caretX < 0)
			caretX = x;
	}
	/*
	if(type == dtDraw && y >= lprcUpdate->top && y < lprcUpdate->bottom)
	{
		rcl.top    = max(y, lprcUpdate->top);
		rcl.left   = rcc.left;
		rcl.right  = rcc.right;
		rcl.bottom = rcc.bottom;
		FillRect(hdc, &rcl, 1?GetSysColorBrush(COLOR_WINDOW):(HBRUSH)GetStockObject(LTGRAY_BRUSH));
	}
	*/
	if(type == dtSetPosToMouse)
	{
		if(setline < 1) setline = 1;
		if(setcol < 1)  setcol  = 1;
		pBox->curline = setline;
		pBox->curcol  = setcol;
		//_tprintf(_T("s2m %d,%d\n"), setline, setcol);
	}
	if(type == dtSetCaret)
	{
		// if the caret is outside the display region
		// then recenter the view
		//
		SetCaretPos(caretX, caretY);
	}

}

//**************************************************************************
void _w_edit_gettext(HWND hWnd, _w_editbox* pBox)
{
	GetWindowText(hWnd, pBox->text, BSA_MAX_EB_TEXT);
	pBox->nText = _tcslen(pBox->text);
	pBox->text[pBox->nText++] = _T('\n');
	pBox->text[pBox->nText] = _T('\0');
	if(pBox->curcol > pBox->nText)
		pBox->curcol = pBox->nText;
	if(pBox->qla > 0)
	{
		if(pBox->qca > pBox->curcol)
		{
			//_tprintf(_T("reset selection\n"));
			pBox->qca = pBox->qcb = -1;
			pBox->qla = pBox->qlb = -1;
		}
	}
}

//**************************************************************************
void _w_edit_settext(HWND hWnd, _w_editbox* pBox)
{
	if(pBox->nText >= BSA_MAX_EB_TEXT - 1)
		pBox->nText = BSA_MAX_EB_TEXT - 2;
	if(pBox->nText > 0 && pBox->text[pBox->nText - 1] == _T('\n'))
		pBox->text[pBox->nText - 1] = _T('\0');
	SetWindowText(hWnd, pBox->text);
}

//**************************************************************************
void _w_edit_setcaret(HWND hWnd, _w_editbox* pBox)
{
	HDC	hdc = GetDC(hWnd);

	_w_edit_textout(hdc, hWnd, dtSetCaret, pBox);
	ReleaseDC(hdc, hWnd);
}

//**************************************************************************
void _w_ebchecksel(HWND hWnd, _w_editbox* pBox)
{
	int len;
	LPTSTR pe, pt;
	if(! pBox) return;
	if(pBox->qla <= 0 || pBox->qlb <= 0)
		return;

	// delete chars from a to b
	pBox->curcol = pBox->qca;

	len = pBox->qcb - pBox->qca;
	pBox->nText-= len;
	pe  = pBox->text + pBox->qca - 1;
	pt  = pBox->text + pBox->qcb - 1;
	while(len-- > 0) *pe++ = *pt++;
	pBox->qla = pBox->qlb = -1;
	pBox->qca = pBox->qcb = -1;
	InvalidateRect(hWnd, NULL, FALSE);
}

//**************************************************************************
static LRESULT _w_edit_copy(_w_editbox *pBox, LPTSTR lpCopied, int nCopy)
{
	HANDLE hMem;
	LPSTR  pcp;
	int i;

	// set copied to the cut buffer
	//
	hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (nCopy + 2) * sizeof(char));
	pcp = (char*)GlobalLock(hMem);

	EmptyClipboard();

	for(i = 0; i < nCopy; i++)
	{
		pcp[i] = (char)lpCopied[i];
	}
	pcp[i] = '\0';
	GlobalUnlock(hMem);
	SetClipboardData(CF_TEXT, hMem);
	CloseClipboard();
	return TRUE;
}

//**************************************************************************
static LRESULT _w_edit_paste(HWND hWnd, _w_editbox *pBox)
{
	if(OpenClipboard(hWnd))
	{
		HANDLE hMem = GetClipboardData(CF_TEXT);
		LPSTR  pcp;

		if(hMem)
		{
			if((pcp = (LPSTR)GlobalLock(hMem)) != NULL)
			{
				int cblen = strlen(pcp);

				if(cblen > 0)
				{
					LPTSTR pe, pt;
					int len;

					if(pBox->curcol >= pBox->nText) pBox->curcol = pBox->nText;
					if(pBox->curcol < 1) pBox->curcol = 1;

					// limit insertion to max that will fit
					if (pBox->nText + cblen >= BSA_MAX_EB_TEXT)
					{
						cblen = BSA_MAX_EB_TEXT - pBox->nText - 1;
					}
					// move text to right of insertion to end of text
					len = pBox->nText - pBox->curcol + 1;
					if (len > 0)
					{
						pe  = pBox->text + pBox->curcol - 2 + len + cblen;
						pt  = pBox->text + pBox->curcol - 2 + len;
						while(len-- > 0)
							*pe-- = *pt--;
					}
					// insert selection
					while (cblen-- > 0)
					{
						pBox->text[pBox->curcol - 1] = (TCHAR)*pcp++;
						pBox->curcol++;
						pBox->nText++;
					}
				}
			}
		}
		CloseClipboard();
		return TRUE;
	}
	return FALSE;
}

//**************************************************************************
LRESULT WINAPI __wproc_Edit(HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{
	PAINTSTRUCT ps;
	HDC         hdc;
	LPZWND		zWnd;
	DWORD		style;
	DWORD		exstyle;
	int			len;
	LPTSTR		pe, pt;
	RECT		rc;
	_w_editbox* pBox;

	if(! (zWnd = Zwindow(hWnd))) return -1;
	style  	= GetWindowLong(hWnd, GWL_STYLE);
	exstyle	= GetWindowLong(hWnd, GWL_EXSTYLE);
	pBox    = (_w_editbox*)GetWindowLong(hWnd, GWL_PRIVDATA);

	GetClientRect(hWnd, &rc);

	switch(message)
	{
	case WM_CREATE:

		// for sunken edits, increase window area slightly
		//
		pBox = (_w_editbox*)malloc(sizeof(_w_editbox));

		pBox->curline = 1;
		pBox->curcol  = 1;
		pBox->topline = 1;
		pBox->leftcol = 1;
		pBox->text[0] = 10;
		pBox->text[1] = 0;
		pBox->nText   = 1;
		pBox->shift = pBox->ctrl = 0;
		_w_edit_settext(hWnd, pBox);
		pBox->qla = pBox->qlb = -1;
		pBox->qca = pBox->qcb = -1;
		pBox->style 	= style;
		pBox->exstyle 	= exstyle;

		if(exstyle & WS_EX_CLIENTEDGE)
		{
			pBox->lpad = pBox->rpad = 4;
			pBox->tpad = 1;
		}
		else
		{
			pBox->lpad = pBox->rpad = 2;
			pBox->tpad = 0;
		}

		pBox->align = ALIGN_LEFT | ALIGN_VCENTER;
		if(style & ES_LEFT)
			pBox->align = ALIGN_LEFT | ALIGN_VCENTER;
		if(style & ES_RIGHT)
			pBox->align = ALIGN_RIGHT | ALIGN_VCENTER;
		if(style & ES_RIGHT)
			pBox->align = ALIGN_CENTER | ALIGN_VCENTER;
		SetWindowLong(hWnd, GWL_PRIVDATA, (LPARAM)pBox);
		SetWindowText(hWnd, _T(""));
		break;

	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		if(exstyle & WS_EX_CLIENTEDGE)
			__w_checkbox(hdc, 0, 0, !(style & WS_DISABLED), &rc);
		if(pBox)
		{
			_w_edit_gettext(hWnd, pBox);
			_w_edit_textout(hdc, hWnd, dtDraw, pBox);
		}
		EndPaint(hWnd, &ps);
		break;

	case WM_LBUTTONDOWN:
		_w_edit_gettext(hWnd, pBox);
		pBox->mousex = LOWORD(lParam);
		pBox->mousey = HIWORD(lParam);
		{
			hdc  = GetDC(hWnd);

			SetCapture(hWnd);
			_w_edit_textout(hdc, hWnd, dtSetPosToMouse, pBox);
			pBox->qla = pBox->curline;
			pBox->qca = pBox->curcol;
			pBox->qlb = pBox->qla;
			pBox->qcb = pBox->qca;
			_w_edit_textout(hdc, hWnd, dtSetCaret, pBox);
			ReleaseDC(hWnd, hdc);
		}
		//if(GetFocus() != hWnd)
		{
			//_tprintf(_T("lbd sf %p  foc=%p\n"), hWnd,GetFocus());
			SetFocus(hWnd);
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_LBUTTONUP:
		ReleaseCapture();
		break;

	case WM_LBUTTONDBLCLK:
		pBox->qla = 1;
		pBox->qlb = 1;
		pBox->qca = 1;
		pBox->qcb = pBox->nText;
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_MOUSEMOVE:
		pBox->mousex = LOWORD(lParam);
		pBox->mousey = HIWORD(lParam);
		_w_edit_gettext(hWnd, pBox);
		if(GetCapture() == hWnd)
		{
			HDC hdc  = GetDC(hWnd);

			_w_edit_textout(hdc, hWnd, dtSetPosToMouse, pBox);

			pBox->qlb = pBox->curline;
			pBox->qcb = pBox->curcol;
			//if(pBox->curline == pBox->qla && pBox->curcol == pBox->qca)
			//	pBox->qlb = pBox->qcb = 0;
			//_w_edit_textout(hdc, hWnd, dtDraw, pBox);
			ReleaseDC(hWnd, hdc);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;

	case WM_CUT:
		if(pBox->qla > 0 && pBox->qla <= pBox->qlb)
		{
			// works only for one line for now
			_w_edit_copy(pBox, pBox->text + pBox->qca - 1, pBox->qcb - pBox->qca);
			_w_ebchecksel(hWnd, pBox);
			_w_edit_settext(hWnd, pBox);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;

	case WM_COPY:
		if(pBox->qla > 0 && pBox->qla <= pBox->qlb)
		{
			_w_edit_copy(pBox, pBox->text + pBox->qca - 1, pBox->qcb - pBox->qca);
		}
		break;

	case WM_PASTE:
		_w_ebchecksel(hWnd, pBox);
		_w_edit_paste(hWnd, pBox);
		_w_edit_settext(hWnd, pBox);
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_CLEAR:
		_w_ebchecksel(hWnd, pBox);
		_w_edit_settext(hWnd, pBox);
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_SETFOCUS:
		{
			TEXTMETRIC  tm;
			HDC  		hdc = GetDC(hWnd);

			GetTextMetrics(hdc, &tm);
			CreateCaret(hWnd, NULL, 2, tm.tmHeight);
			ShowCaret(hWnd);

			_w_edit_gettext(hWnd, pBox);
			if(0 && (pBox->nText > 1))
			{
				// this selects all text in box on focus
				pBox->qla = pBox->qlb = pBox->curline;
				pBox->qca = 1;
				pBox->qcb = pBox->nText;
			}
			_w_edit_textout(hdc, hWnd, dtSetCaret, pBox);
			ReleaseDC(hdc, hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;

	case WM_KILLFOCUS:
		if(pBox->qla > 0)
		{
			pBox->qla = pBox->qlb = -1;
			pBox->qca = pBox->qcb = -1;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		HideCaret(hWnd);
		DestroyCaret();
		break;

	case WM_KEYDOWN:

		_w_edit_gettext(hWnd, pBox);
		switch(wParam)
		{
		case VK_LEFT:
			if(pBox->curcol > 1) pBox->curcol--;
			break;

		case VK_RIGHT:
			if(pBox->curcol < pBox->nText) pBox->curcol++;
			break;

		case VK_HOME:
			pBox->curcol = 1;
			break;

		case VK_END:
			pBox->curcol = pBox->nText;
			break;

		case VK_BACK:
			if(pBox->qla > 0 && pBox->qlb > 0)
				_w_ebchecksel(hWnd, pBox);
			else
			{
				if(pBox->curcol > 1)
				{
					len = pBox->nText - pBox->curcol + 2;
					pe  = pBox->text + pBox->curcol - 2;
					pt  = pe + 1;
					while(len-- > 0) *pe++ = *pt++;
					pBox->curcol--;
					pBox->nText--;
				}
				//_tprintf(_T("xxt=%ls= n=%d cc=%d\n"), pBox->text, pBox->nText, pBox->curcol);
				InvalidateRect(hWnd, NULL, FALSE);
			}
			break;

		case VK_DELETE:
			if(pBox->qla > 0 && pBox->qlb > 0)
			{
				if(pBox->shift)
					SendMessage(hWnd, WM_CUT, 0, 0);
				else if(pBox->ctrl)
					SendMessage(hWnd, WM_CLEAR, 0, 0);
				else
					_w_ebchecksel(hWnd, pBox);
			}
			else
			{
				if(pBox->curcol < pBox->nText)
				{
					len = pBox->nText - pBox->curcol + 1;
					pe  = pBox->text + pBox->curcol - 1;
					pt  = pe + 1;
					while(len-- > 0) *pe++ = *pt++;
					pBox->nText--;
				}
				//_tprintf(_T("xxt=%ls= n=%d cc=%d\n"), pBox->text, pBox->nText, pBox->curcol);
				InvalidateRect(hWnd, NULL, FALSE);
			}
			break;

		case VK_RETURN:
		case VK_TAB:
		case VK_ESCAPE:
			_w_edit_settext(hWnd, pBox);
			if(wParam != VK_RETURN || ! (style & ES_MULTILINE))
			{
				// pass this up to the dialog parent if present
				//
				if(_zg_dialogTop >= 0 && _zg_dialogWnd[_zg_dialogTop])
				{
					SendMessage(_zg_dialogWnd[_zg_dialogTop], WM_KEYDOWN, wParam, lParam);
					break;
				}
				else if(GetParent(hWnd))
				{
					SendMessage(GetParent(hWnd), WM_KEYDOWN, wParam, lParam);
					break;
				}
			}
			// note return here!
			return 0;

		case VK_SHIFT:
			pBox->shift = 1;
			break;

		case VK_CONTROL:
			pBox->ctrl = 1;
			break;

		case VK_INSERT:
			if(pBox->shift)
				SendMessage(hWnd, WM_PASTE, 0, 0);
			else if(pBox->ctrl)
				SendMessage(hWnd, WM_COPY, 0, 0);
			break;

		default:
			break;

		}
		_w_edit_settext(hWnd, pBox);
		_w_edit_setcaret(hWnd, pBox);
		break;

	case WM_KEYUP:

		_w_edit_gettext(hWnd, pBox);
		switch(wParam)
		{
		case VK_SHIFT:
			pBox->shift = 0;
			break;

		case VK_CONTROL:
			pBox->ctrl = 0;
			break;

		default:
			break;
		}
		break;

	case WM_CHAR:

		if(style & WS_DISABLED)
			break;

		_w_edit_gettext(hWnd, pBox);
		switch(wParam)
		{
		case 8:
		case 9:
		case 10:
		case 13:
		case 27:
			return 0;

		case 3: // ^C copy
			SendMessage(hWnd, WM_COPY, 0, 0);
			break;

		case 22: // ^V paste
			SendMessage(hWnd, WM_PASTE, 0, 0);
			break;

		default:
			if(style & ES_NUMBER)
			{
				if(wParam < '0' || wParam > '9')
					break;
			}
			_w_ebchecksel(hWnd, pBox);
			if(pBox->curcol >= pBox->nText) pBox->curcol = pBox->nText;
			if(pBox->curcol < 1) pBox->curcol = 1;
			len = pBox->nText - pBox->curcol + 2;
			pe  = pBox->text + pBox->curcol - 1 + len;
			pt  = pe - 1;
			while(len-- > 0) *pe-- = *pt--;
			pBox->text[pBox->curcol - 1] = (TCHAR)wParam;
			pBox->curcol++;
			pBox->nText++;
			//_tprintf(_T("xxt=%ls= n=%d cc=%d\n"), pBox->text, pBox->nText, pBox->curcol);
		}
		_w_edit_settext(hWnd, pBox);
		InvalidateRect(hWnd, NULL, TRUE);
		_w_edit_setcaret(hWnd, pBox);
		break;

	case WM_CLOSE:
		if(pBox)
		{
			SetWindowLong(hWnd, GWL_PRIVDATA, 0);
			free(pBox);
		}
		break;

	case EM_GETSEL:
		if(pBox->qla > 0 && pBox->qla <= pBox->qlb)
		{
			if(wParam) *(LPDWORD)wParam = pBox->qca;
			if(lParam) *(LPDWORD)lParam = pBox->qcb + 1;
		}
		else
		{
			if(wParam) *(LPDWORD)wParam = -1;
			if(lParam) *(LPDWORD)lParam = 0;
		}
		break;

	case EM_SETSEL:
		_w_edit_gettext(hWnd, pBox);
		if(wParam <= lParam)
		{
			pBox->qca = wParam + 1;
			pBox->qcb = lParam + 1;
			if(pBox->qca > pBox->qcb)
				pBox->qca = pBox->qcb;
			if(pBox->qca > pBox->nText)
				pBox->qca = pBox->nText;
			if(pBox->qcb > pBox->nText)
				pBox->qcb = pBox->nText;
			pBox->curcol = pBox->qca;
			pBox->qla = pBox->qlb = pBox->curline;
		}
		else
		{
			pBox->qca = pBox->qcb = -1;
			pBox->qla = pBox->qlb = -1;
		}
		//_tprintf(_T("setsel %d,%d, %d,%d\n"), pBox->qla, pBox->qlb, pBox->qca, pBox->qcb);
		InvalidateRect(hWnd, NULL, TRUE);
		_w_edit_setcaret(hWnd, pBox);
		break;

	case EM_LINELENGTH:
		_w_edit_gettext(hWnd, pBox);
		return pBox->nText;

	case EM_CANUNDO:
		return FALSE;

	case EM_GETLINE:
		if (lParam)
		{
			len = (int)*(LPWORD)lParam;
			if (len <= 0)
			{
				SetLastError(ERROR_INVALID_PARAMETER);
				return FALSE;
			}
			_w_edit_gettext(hWnd, pBox);
			if (len < pBox->nText)
				len = pBox->nText;
			if (len > 0)
				memcpy((LPSTR)lParam, pBox->text, len);
			((LPSTR)lParam)[len] = 0;
		}
		return pBox->nText;

	case EM_UNDO:
	case EM_GETRECT:
	case EM_SETRECT:
	case EM_SETRECTNP:
	case EM_SCROLL:
	case EM_LINESCROLL:
	case EM_SCROLLCARET:
	case EM_GETMODIFY:
	case EM_SETMODIFY:
	case EM_GETLINECOUNT:
	case EM_LINEINDEX:
	case EM_SETHANDLE:
	case EM_GETHANDLE:
	case EM_GETTHUMB:
	case EM_REPLACESEL:
	case EM_LIMITTEXT:
	case EM_FMTLINES:
	case EM_LINEFROMCHAR:
	case EM_SETTABSTOPS:
	case EM_SETPASSWORDCHAR:
	case EM_EMPTYUNDOBUFFER:
	case EM_GETFIRSTVISIBLELINE:
	case EM_SETREADONLY:
		return 0;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

#endif
