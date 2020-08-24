
#include "bedx.h"

static struct tag_fd_parms
{
	bool			foropen;
	bool			mustexist;

	LINETERMINATION	lineterm;
	TEXTENCODING	encoding;
	BufType			bufType;
}
g_parms;

#define OF_LM	8
#define OF_TM	-6
#define OF_TYP_W	72
#define OF_ENC_W	172
#define OF_LT_W		108
#define OF_H	16
#define OF_BW	10
#define OF_BH	16

#define IDC_OF_TYP		0xfee8
#define IDC_OF_TYP_DOWN	0xfee0
#define IDC_OF_TYP_UP	0xfee1

#define IDC_OF_ENC		0xfee9
#define IDC_OF_ENC_DOWN	0xfee2
#define IDC_OF_ENC_UP	0xfee3

#define IDC_OF_LT		0xfee10
#define IDC_OF_LT_DOWN	0xfee4
#define IDC_OF_LT_UP	0xfee5

//**************************************************************************
static void SetTypeText(HWND hWnd)
{
	LPCTSTR lptext;

	if(! hWnd) return;

	switch(g_parms.bufType)
	{
	default:
	case btText:
		lptext = _T(" Text");
		break;
	case btRaw:
		lptext = _T(" Binary");
		break;
	case btAny:
		lptext = _T(" Auto");
		break;
	}
	SetWindowText(hWnd, (LPTSTR)lptext);
}

//**************************************************************************
static void SetEncodingText(HWND hWnd)
{
	LPCTSTR lptext;

	if(! hWnd) return;

	switch(g_parms.encoding)
	{
	case txtANSI:
		lptext = _T(" ANSI (ascii)");
		break;
	case txtUTF8:
		lptext = _T(" UTF-8");
		break;
	case txtUCS2:
		lptext = _T(" UCS-2 (Windows x86)");
		break;
	case txtUCS2SWAP:
		lptext = _T(" UCS-2 (big endian)");
		break;
	case txtUCS4:
		lptext = _T(" UCS-4 (Linux x86)");
		break;
	case txtUCS4SWAP:
		lptext = _T(" UCS-4 BE (PPC Mac/Linux)");
		break;
//	case (TEXTENCODING)-1:
	default:
		lptext = _T(" Auto");
		break;
	}
	SetWindowText(hWnd, (LPTSTR)lptext);
}

//**************************************************************************
static void SetLineTermText(HWND hWnd)
{
	LPCTSTR lptext;

	if(! hWnd) return;

	switch(g_parms.lineterm)
	{
	case ltCRLF:
		lptext = _T(" CR-LF (DOS)");
		break;
	default:
	case ltLF:
		lptext = _T(" LF (Unix)");
		break;
	}
	SetWindowText(hWnd, (LPTSTR)lptext);
}

//**************************************************************************
BOOL WINAPI PickFileWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND	hWnd, hWndParent;
	HFONT	hFont;
	RECT	rc;
	RECT	rcme;
	int		x, y, w, h;
	BOOL	lten;

	switch (message) 
	{
	case WM_INITDIALOG:

		hWndParent = GetParent(hDlg);
		if(! hWndParent)
			hWndParent = GetDesktopWindow();
		GetClientRect(hWndParent, &rc);
		GetWindowRect(hDlg, &rcme);
		
		rcme.bottom += 32;
		x = ((rc.right - rc.left) - (rcme.right - rcme.left)) / 2;
		y = ((rc.bottom - rc.top) - (rcme.bottom - rcme.top)) / 2;
		w = rcme.right - rcme.left;
		h = rcme.bottom - rcme.top;

		if(x < 0) x = 0;
		if(y < 0) y = 0;

		GetClientRect(hDlg, &rc);

		MoveWindow(hDlg, x, y, w, h, FALSE);

		// add encoding, type, and lineterm controls to end of dialog
		//
		y = rc.bottom - rc.top + OF_TM;
		x = OF_LM;

		hFont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);

		hWnd = CreateWindow(_T("Static"), _T("Open As"), WS_CHILD | WS_VISIBLE, x, y, OF_TYP_W, OF_H, hDlg, NULL, g_hInstance, NULL);
		SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);
		ShowWindow(hWnd, SW_SHOW);
		hWnd = CreateWindow(_T("Static"), _T("Encoding"), WS_CHILD | WS_VISIBLE, x + OF_TYP_W, y, OF_ENC_W, OF_H, hDlg, NULL, g_hInstance, NULL);
		SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);
		ShowWindow(hWnd, SW_SHOW);
		hWnd = CreateWindow(_T("Static"), _T("Line-term"), WS_CHILD | WS_VISIBLE, x + OF_TYP_W + OF_ENC_W, y, OF_LT_W, OF_H, hDlg, NULL, g_hInstance, NULL);
		SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);
		ShowWindow(hWnd, SW_SHOW);

		x = OF_LM;
		y += OF_H;
		hWnd = CreateWindow(_T("Button"), _T("<"), WS_CHILD | BS_PUSHBUTTON, x, y, OF_BW, OF_BH, hDlg, NULL, NULL, NULL);
		SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);
		SetWindowLong(hWnd, GWL_ID, 0xfee0);
		ShowWindow(hWnd, SW_SHOW);
		x += OF_BW + 2;
		hWnd = CreateWindow(_T("Button"), _T(">"), WS_CHILD | BS_PUSHBUTTON, x, y, OF_BW, OF_BH, hDlg, NULL, NULL, NULL);
		SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);
		SetWindowLong(hWnd, GWL_ID, 0xfee1);
		ShowWindow(hWnd, SW_SHOW);

		x = OF_LM + OF_TYP_W;
		hWnd = CreateWindow(_T("Button"), _T("<"), WS_CHILD | BS_PUSHBUTTON, x, y, OF_BW, OF_BH, hDlg, NULL, NULL, NULL);
		SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);
		SetWindowLong(hWnd, GWL_ID, 0xfee2);
		ShowWindow(hWnd, SW_SHOW);
		x += OF_BW + 2;
		hWnd = CreateWindow(_T("Button"), _T(">"), WS_CHILD | BS_PUSHBUTTON, x, y, OF_BW, OF_BH, hDlg, NULL, NULL, NULL);
		SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);
		SetWindowLong(hWnd, GWL_ID, 0xfee3);
		ShowWindow(hWnd, SW_SHOW);

		x = OF_LM + OF_TYP_W + OF_ENC_W;
		hWnd = CreateWindow(_T("Button"), _T("<"), WS_CHILD | BS_PUSHBUTTON, x, y, OF_BW, OF_BH, hDlg, NULL, NULL, NULL);
		SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);
		SetWindowLong(hWnd, GWL_ID, 0xfee4);
		ShowWindow(hWnd, SW_SHOW);
		x += OF_BW + 2;
		hWnd = CreateWindow(_T("Button"), _T(">"), WS_CHILD | BS_PUSHBUTTON, x, y, OF_BW, OF_BH, hDlg, NULL, NULL, NULL);
		SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);
		SetWindowLong(hWnd, GWL_ID, 0xfee5);
		ShowWindow(hWnd, SW_SHOW);


		x = OF_LM + 2*OF_BW + 4;
		hWnd = CreateWindow(_T("Static"), _T("type"), WS_CHILD | WS_BORDER, x, y, OF_TYP_W - 2*OF_BW - 6, OF_BH, hDlg, NULL, NULL, NULL);
		SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);
		SetWindowLong(hWnd, GWL_ID, 0xfee8);
		ShowWindow(hWnd, SW_SHOW);

		x = OF_LM + OF_TYP_W + 2*OF_BW + 4;
		hWnd = CreateWindow(_T("Static"), _T("encoding"), WS_CHILD | WS_BORDER, x, y, OF_ENC_W - 2*OF_BW - 6, OF_BH, hDlg, NULL, NULL, NULL);
		SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);
		SetWindowLong(hWnd, GWL_ID, 0xfee9);
		ShowWindow(hWnd, SW_SHOW);

		x = OF_LM + OF_TYP_W + OF_ENC_W + 2*OF_BW + 4;
		hWnd = CreateWindow(_T("Static"), _T("lineterm"), WS_CHILD | WS_BORDER, x, y, OF_LT_W - 2*OF_BW - 6, OF_BH, hDlg, NULL, NULL, NULL);
		SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);
		SetWindowLong(hWnd, GWL_ID, 0xfee10);
		ShowWindow(hWnd, SW_SHOW);
		// encoding
		//
#ifndef UNICODE

		g_parms.encoding = txtANSI;

		EnableWindow(GetDlgItem(hDlg, IDC_OF_ENC),		FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_OF_ENC_UP),	FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_OF_ENC_DOWN), FALSE);
#endif
		// use crlf
		//
		lten = (g_parms.foropen && g_parms.mustexist) ? FALSE : TRUE;

		EnableWindow(GetDlgItem(hDlg, IDC_OF_LT),		lten);
		EnableWindow(GetDlgItem(hDlg, IDC_OF_LT_UP),	lten);
		EnableWindow(GetDlgItem(hDlg, IDC_OF_LT_DOWN),	lten);

		SetTypeText(GetDlgItem(hDlg, IDC_OF_TYP));
		SetEncodingText(GetDlgItem(hDlg, IDC_OF_ENC));
		SetLineTermText(GetDlgItem(hDlg, IDC_OF_LT));

		return FALSE;

	case WM_COMMAND:

		switch(wParam)
		{
		case IDC_OF_TYP_UP:
			switch(g_parms.bufType)
			{
			case btAny:		g_parms.bufType = btText;	break;
			case btText:	g_parms.bufType = btRaw;	break;
			default:
			case btRaw:		g_parms.bufType = btAny;	break;
			}
			SetTypeText(GetDlgItem(hDlg, IDC_OF_TYP));
			break;
		case IDC_OF_TYP_DOWN:
			switch(g_parms.bufType)
			{
			case btAny:		g_parms.bufType = btRaw;	break;
			default:
			case btText:	g_parms.bufType = btAny;	break;
			case btRaw:		g_parms.bufType = btText;	break;
			}
			SetTypeText(GetDlgItem(hDlg, IDC_OF_TYP));
			break;

		case IDC_OF_ENC_UP:
			switch(g_parms.encoding)
			{
			case txtANSI:	g_parms.encoding = txtUTF8;				break;
			case txtUTF8:	g_parms.encoding = txtUCS2;				break;
			case txtUCS2:	g_parms.encoding = txtUCS2SWAP;			break;
			case txtUCS2SWAP:g_parms.encoding= txtUCS4;				break;
			case txtUCS4:	g_parms.encoding = txtUCS4SWAP;			break;
			case txtUCS4SWAP:g_parms.encoding = (TEXTENCODING)-1;	break;
			//case -1:
			default:		g_parms.encoding = txtANSI;				break;
			}
			SetEncodingText(GetDlgItem(hDlg, IDC_OF_ENC));
			break;
		case IDC_OF_ENC_DOWN:
			switch(g_parms.encoding)
			{
			case txtANSI:	g_parms.encoding = (TEXTENCODING)-1;	break;
			case txtUTF8:	g_parms.encoding = txtANSI;				break;
			case txtUCS2:	g_parms.encoding = txtUTF8;				break;
			case txtUCS2SWAP:g_parms.encoding= txtUCS2;				break;
			case txtUCS4:	g_parms.encoding = txtUCS2SWAP;			break;
			case txtUCS4SWAP:g_parms.encoding = txtUCS4;			break;
			//case -1:
			default:		g_parms.encoding = txtUCS4SWAP;			break;
			}
			SetEncodingText(GetDlgItem(hDlg, IDC_OF_ENC));
			break;

		case IDC_OF_LT_UP:
		case IDC_OF_LT_DOWN:
			switch(g_parms.lineterm)
			{
			case ltLF:		g_parms.lineterm = ltCRLF;	break;
			default:
			case ltCRLF:	g_parms.lineterm = ltLF;	break;
			}
			SetLineTermText(GetDlgItem(hDlg, IDC_OF_LT));
			break;

		default:
			break;
		}
	}
    return FALSE;
}

//**************************************************************************
int PickFileDialog(
				   LPCTSTR			lpTitle,
				   bool				openExisting,
				   bool				openorsave,
				   LPTSTR			lpName,
				   int				nName,
				   TEXTENCODING&	encoding,
				   LINETERMINATION& lineterm,
				   BufType&			bufType,
				   HWND				hwndParent
				  )
{
	int rc;

		
#ifdef UNICODE
	g_parms.encoding	= encoding;
#else
	g_parms.encoding	= txtANSI;
#endif
	g_parms.bufType		= bufType;
	g_parms.lineterm	= lineterm;
	g_parms.foropen		= openorsave;
	g_parms.mustexist	= openExisting;

	rc = OpenFileDialog(g_hInstance, lpTitle, openExisting, openorsave, false, true, lpName, nName, NULL, PickFileWndProc);
	if(rc == IDOK)
	{
		encoding = g_parms.encoding;
		lineterm = g_parms.lineterm;
		bufType	 = g_parms.bufType;
	}
	return rc;
}

//**************************************************************************
int PickDirectoryDialog(
						LPPARMPASS	pParm,
						HWND		hwndParent
						)
{
	int rc;

	rc = OpenFileDialog(g_hInstance, pParm->lpTitle, true, true, true, true, pParm->lpString, pParm->nString, NULL, NULL);
	return rc;
}

