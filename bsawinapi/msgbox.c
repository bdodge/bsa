//--------------------------------------------------------------------
//
// File: msgbox.c
// Desc: messagebox for winapi
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------

#include "winapix.h"

#define BSA_MIN_MSGB_W	120
#define BSA_DEF_MSGB_W	220
#define BSA_MAX_MSGB_W	600

#define BSA_MIN_MSGB_H	60
#define BSA_DEF_MSGB_H	90
#define BSA_MAX_MSGB_H	400

#define BSA_MB_BUT_W	54
#define BSA_MB_BUT_H	24
#define BSA_MB_BUT_MARG	8

#define BSA_MB_TEXT_X	4
#define BSA_MB_TEXT_Y	14

#define BSA_MBT_SIZE (sizeof(DLGTEMPLATE) + 512 + 5*sizeof(DLGITEMTEMPLATE))

BYTE 		_zg_mb[BSA_MBT_SIZE];
extern BOOL _zg_keynav;
extern HFONT _zg_sysFont;


//**************************************************************************
int WINAPI MessageBox(
						HWND hWnd,
						LPCTSTR lpText,
						LPCTSTR lpCaption,
						UINT uType
					)
{
	LPDLGTEMPLATE 	  ptemp;
	LPDLGITEMTEMPLATE pitem;
	LPWORD			  ps;
	LONG			  llps;
	LPTSTR			  pt;
	
	RECT 	rcp;
	int  	nb = 0;
	LPCTSTR bt[3] = { NULL, NULL, NULL };
	int     bi[3] = { 0, 0, 0 };
	int		type, icon, i;
	int		x, y, w, h;
	
	if(! hWnd)
		GetClientRect(GetDesktopWindow(), &rcp);
	else
		GetClientRect(hWnd, &rcp);

	type = uType & 0x07;
	icon = uType & 0xF0;

	switch(type)
	{
	case MB_OK:
		bi[nb]   = IDOK;
		bt[nb++] = _T("OK");
		break;
	case MB_OKCANCEL:
		bi[nb]   = IDOK;
		bt[nb++] = _T("OK");
		bi[nb]   = IDCANCEL;
		bt[nb++] = _T("Cancel");
		break;
	case MB_ABORTRETRYIGNORE:
		bi[nb]   = IDABORT;
		bt[nb++] = _T("Abort");
		bi[nb]   = IDRETRY;
		bt[nb++] = _T("Retry");
		bi[nb]   = IDIGNORE;
		bt[nb++] = _T("Ignore");
		break;
	case MB_YESNOCANCEL:
		bi[nb]   = IDYES;
		bt[nb++] = _T("Yes");
		bi[nb]   = IDNO;
		bt[nb++] = _T("No");
		bi[nb]   = IDCANCEL;
		bt[nb++] = _T("Cancel");
		break;
	case MB_YESNO:
		bi[nb]   = IDYES;
		bt[nb++] = _T("Yes");
		bi[nb]   = IDNO;
		bt[nb++] = _T("No");
		break;
	case MB_RETRYCANCEL:
		bi[nb]   = IDRETRY;
		bt[nb++] = _T("Retry");
		bi[nb]   = IDCANCEL;
		bt[nb++] = _T("Cancel");
		break;
	default:
		return IDCANCEL;
	}

	w = BSA_DEF_MSGB_W;
	h = BSA_DEF_MSGB_H;

	x = _tcslen(lpText) * 6;
	{
		HFONT hOldFont;
		HDC   hdc;
		SIZE  sizeText;
		int	  maxw;
		TCHAR* ptb;
		
		hdc = GetDC(hWnd);
		hOldFont = SelectObject(hdc, _zg_sysFont);
		for(pt = ptb = (LPTSTR)lpText, maxw = 0; pt && *pt; pt++)
		{
			if(*pt == _T('\n') || ! *pt)
			{
				GetTextExtentPoint32(hdc, ptb, pt - ptb, &sizeText);
				if(sizeText.cx > maxw)
					maxw = sizeText.cx;
				ptb = pt;
			}
		}
		if(pt > ptb)
		{
			GetTextExtentPoint32(hdc, ptb, pt - ptb, &sizeText);
			if(sizeText.cx > maxw)
				maxw = sizeText.cx;
		}
		SelectObject(hdc, hOldFont);
		ReleaseDC(hWnd, hdc);
		x = maxw;
	}
	if((x + 24 + (icon ? 32 : 8)) > w)
	{
		w = x + 24 + (icon ? 32 : 8);
	}
	if(w > BSA_MAX_MSGB_W)
		w = BSA_MAX_MSGB_W;
	x = (rcp.right - rcp.left - w) / 2;
	y = (rcp.bottom - rcp.top - h) / 2;
	
	// create a dialog template
	//
	ptemp = (LPDLGTEMPLATE)_zg_mb;
	
	ptemp->style = WS_POPUPWINDOW;
	ptemp->dwExtendedStyle = 0;
	ptemp->cdit = 1+nb;
	ptemp->x 	= BSA_UNSCALE_DLG_X(x);
	ptemp->y 	= BSA_UNSCALE_DLG_Y(y);
	ptemp->cx 	= BSA_UNSCALE_DLG_X(w);
	ptemp->cy 	= BSA_UNSCALE_DLG_Y(h);
	
	ps = (LPWORD)(_zg_mb + 18);
	
	*ps++ = 0;	// menu
	*ps++ = 0;	// class
	
	// title;
	for(pt = (LPTSTR)lpCaption; pt && *pt;)
		*ps++ = (WORD)*pt++;
	*ps++ = 0;

	llps = (LONG)ps;
	llps += 3;
	llps &= ~3;
	ps = (LPWORD)llps;
	
	pitem = (LPDLGITEMTEMPLATE)ps;
	pitem->dwExtendedStyle = 0;
	pitem->style = WS_CHILD /*| ((icon) ? BS_LEFTTEXT : BS_CENTER)*/;
	pitem->x 	 = BSA_UNSCALE_DLG_X(BSA_MB_TEXT_X + (icon ? 32 : 0));
	pitem->y 	 = BSA_UNSCALE_DLG_Y(BSA_MB_TEXT_Y);
	pitem->cx 	 = BSA_UNSCALE_DLG_X((w - (icon? 32 : 8)));
	pitem->cy 	 = BSA_UNSCALE_DLG_Y(24);
	pitem->id	 = IDC_STATIC;
	ps += 18/2;

	*ps++ = 0xffff;
	*ps++ = 0x82; // static (label) class
	
	for(pt = (LPTSTR)lpText; pt && *pt;)
	{
		if(*pt == _T('\n'))
		{
			pitem->cy += BSA_UNSCALE_DLG_Y(BSA_MB_BUT_H);
			ptemp->cy += BSA_UNSCALE_DLG_Y(BSA_MB_BUT_H);
			h += BSA_UNSCALE_DLG_Y(BSA_MB_BUT_H);
		}
		*ps++ = (WORD)*pt++;
	}
	*ps++ = 0;
	
	// createparm len
	*ps++ = 0;

	y = nb * BSA_MB_BUT_W + (nb - 1) * BSA_MB_BUT_MARG;
	x = (w - y) / 2;
	if(x < 4) x = 4;
	y = h - BSA_MB_BUT_H - BSA_MB_BUT_MARG;
	if(y < 4) y = 4;
	
	for(i = 0; i < nb; i++)
	{
		
		llps = (LONG)ps;
		llps += 3;
		llps &= ~3;
		ps = (LPWORD)llps;
		
		pitem = (LPDLGITEMTEMPLATE)ps;
		pitem->dwExtendedStyle = 0;
		pitem->style = WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON;
		pitem->x 	 = BSA_UNSCALE_DLG_X(x);
		pitem->y 	 = BSA_UNSCALE_DLG_Y(y);
		pitem->cx 	 = BSA_UNSCALE_DLG_X(BSA_MB_BUT_W);
		pitem->cy 	 = BSA_UNSCALE_DLG_Y(BSA_MB_BUT_H);
		
		pitem->id 	 = bi[i];

		x  += BSA_MB_BUT_W + BSA_MB_BUT_MARG;
		ps += 18/2;
		
		*ps++ = 0xffff;
		*ps++ = 0x80; // button class

		for(pt = (LPTSTR)bt[i]; pt && *pt;)
		{
			*ps++ = (WORD)*pt++;
		}
		*ps++ = 0;
		
		// createparm len
		*ps++ = 0;
	}	
	_zg_keynav = TRUE;
	return DialogBoxIndirect((HINSTANCE)0xbeef, (LPDLGTEMPLATE)_zg_mb,
			hWnd, NULL);
}


