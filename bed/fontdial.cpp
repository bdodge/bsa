
#include "bedx.h"

static LPCTSTR	 g_pFace = NULL;
static HTREEITEM g_selItem;
static HFONT	 g_hFont = NULL;

//**************************************************************************
static void SetFontHeight(HWND hWnd, int height)
{
	TCHAR	szBuffer[32];

	_sntprintf(szBuffer, 32, _T("%d"), height);
	SetDlgItemText(hWnd, IDC_FONTSIZE, szBuffer);
}

//**************************************************************************
static void ShowFont(HWND hWnd, LPCTSTR pFace, int height, bool bold, bool italic, bool antialias)
{
	HFONT hFont;
	int   resy;

	HDC hdc = GetDC(NULL);

	if(hdc)
	{
		resy = GetDeviceCaps(hdc, LOGPIXELSY);
		DeleteDC(hdc);
	}
	else
	{
		resy = 90;
	}
	//printf("hin=%d hout=%d\n", height, (height*resy + 36)/72);
	height = (height * resy + 36) / 72;

	hFont = CreateFont(
				height,0,0,0,
				(bold ? 700 : 400),
				(italic ? TRUE : FALSE),
				FALSE,FALSE,ANSI_CHARSET,
				OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
				(antialias ? ANTIALIASED_QUALITY : DEFAULT_QUALITY),
				FF_SWISS,
				pFace
			);
	if(hFont)
	{
		if(g_hFont)
			DeleteObject(g_hFont);
		g_hFont = hFont;
		SendMessage(GetDlgItem(hWnd, IDC_FONTTEST), WM_SETFONT, (WPARAM)hFont, 1);
		InvalidateRect(GetDlgItem(hWnd, IDC_FONTTEST), NULL, TRUE);
	}
}

//**************************************************************************
static int CALLBACK OnFontEnum(CONST LOGFONT* lplf, CONST TEXTMETRIC* lptm, DWORD ft, LPARAM lpCookie)
{
	TVINSERTSTRUCT  tvItem;
	HTREEITEM		hItem;

	if(! _tcscmp(lplf->lfFaceName, (LPCTSTR)GetWindowLong((HWND)lpCookie, GWL_USERDATA)))
		return 1;
	_tcsncpy((LPTSTR)GetWindowLong((HWND)lpCookie, GWL_USERDATA), lplf->lfFaceName, 127);
		
	tvItem.hParent		= NULL;
	tvItem.hInsertAfter = TVI_SORT;

	tvItem.item.mask	= TVIF_TEXT | TVIF_PARAM;
	tvItem.item.pszText = (LPTSTR)lplf->lfFaceName;
	tvItem.item.lParam	= 0;
	
	hItem = TreeView_InsertItem((HWND)lpCookie, &tvItem);
	if(g_pFace && ! _tcscmp(tvItem.item.pszText, g_pFace))
		g_selItem = hItem;
	return 1;
}

//**************************************************************************
static void ListFonts(HWND hWnd, LPCTSTR pCurFace)
{
	HDC		hdc;
	HWND	hWndTree;
	TCHAR	lastfact[128];

	hWndTree = GetDlgItem(hWnd, IDC_FONTLIST);
	TreeView_DeleteAllItems(hWndTree);

	// list all available fonts
	hdc = GetDC(hWnd);
	g_pFace   = pCurFace;
	g_selItem = NULL;
	SetWindowLong(hWndTree, GWL_USERDATA, (LONG)lastfact);
	lastfact[0] = '\0';
	EnumFonts(hdc, NULL, OnFontEnum, (LPARAM)hWndTree); 
	ReleaseDC(hWnd, hdc);
	if(g_selItem)
		TreeView_SelectItem(hWndTree, g_selItem);
}

//**************************************************************************
static BOOL CALLBACK FontProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static LPPARMPASS pParm = NULL;

	RECT	rc;
	RECT	rcme;
	HWND	hwndParent;
	LPNMHDR	nmhdr;
	TVITEM tvitem;

	ERRCODE	ec;
	TCHAR   szBuffer[32];

	static TCHAR	szFace[128];
	static int		height;
	static bool		bold, italic, antialias;

	switch (message) 
	{
	case WM_INITDIALOG:

		pParm = (LPPARMPASS)lParam;
		if(! pParm) break;
		ec = Bview::ParseFontSpec(pParm->lpInitVal, szFace, 128, height, bold, italic, antialias);

		if(g_hFont)
			DeleteObject(g_hFont);
		g_hFont = NULL;

		hwndParent = GetParent(hWnd);
		if(! hwndParent) hwndParent = GetDesktopWindow();
		GetClientRect(hwndParent, &rc);
		GetWindowRect(hWnd, &rcme);
		{
			int x = ((rc.right - rc.left) - (rcme.right - rcme.left)) / 2;
			int y = ((rc.bottom - rc.top) - (rcme.bottom - rcme.top)) / 2;
			int w = rcme.right - rcme.left;
			int h = rcme.bottom - rcme.top;

			if(x < 0) x = 0;
			if(y < 0) y = 0;

			MoveWindow(hWnd, x, y, w, h, FALSE);
		}
		CheckDlgButton(hWnd, IDC_FONTBOLD, bold   ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_FONTITAL, italic ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_FONTAA,   antialias ? BST_CHECKED : BST_UNCHECKED);
		ListFonts(hWnd, szFace);
		SetFontHeight(hWnd, height);
		ShowFont(hWnd, szFace, height, bold, italic, antialias);
		return TRUE;

	case WM_DESTROY:
		if(g_hFont)
			DeleteObject(g_hFont);
		g_hFont = NULL;
		break;

	case WM_COMMAND:

		switch(wParam)
		{
		case IDC_FONTBIGGER:

			height++;
			SetFontHeight(hWnd, height);
			ShowFont(hWnd, szFace, height, bold, italic, antialias);
			break;

		case IDC_FONTSMALLER:

			if(height > 6)
				height--;
			SetFontHeight(hWnd, height);
			ShowFont(hWnd, szFace, height, bold, italic, antialias);
			break;

		case IDC_FONTSIZESET:

			GetDlgItemText(hWnd, IDC_FONTSIZE, szBuffer, 32);
			height = _tcstol(szBuffer, NULL, 0);
			if(height < 6)
				height = 6;
			//SetFontHeight(hWnd, height);
			ShowFont(hWnd, szFace, height, bold, italic, antialias);
			break;

		case IDC_FONTBOLD:

			bold = IsDlgButtonChecked(hWnd, IDC_FONTBOLD) != 0;
			ShowFont(hWnd, szFace, height, bold, italic, antialias);
			break;

		case IDC_FONTITAL:

			italic = IsDlgButtonChecked(hWnd, IDC_FONTITAL) != 0;
			ShowFont(hWnd, szFace, height, bold, italic, antialias);
			break;

		case IDC_FONTAA:

			antialias = IsDlgButtonChecked(hWnd, IDC_FONTAA) != 0;
			ShowFont(hWnd, szFace, height, bold, italic, antialias);
			break;

		case IDOK:

			_sntprintf(pParm->lpString, pParm->nString, _T(""_Pfs_":%d:%d:%d:%d"),
					szFace, height, bold, italic, antialias);
			EndDialog(hWnd, IDOK);
			break;

		case IDCANCEL:

			EndDialog(hWnd, wParam);
			break;

		default:

			break;
		}
		break;

	case WM_NOTIFY:

		nmhdr = (LPNMHDR)lParam;
		if(! nmhdr) break;
		
		switch(nmhdr->code)
		{
		case NM_DBLCLK:			
			tvitem.hItem		= TreeView_GetSelection(nmhdr->hwndFrom);
			tvitem.mask			= TVIF_TEXT | TVIF_PARAM;
			tvitem.cchTextMax	= 128;
			tvitem.pszText		= szFace;
			TreeView_GetItem(nmhdr->hwndFrom, &tvitem);
			ShowFont(hWnd, szFace, height, bold, italic, antialias);
			break;	
		}
		break;

	default:
		break;
	}
	return FALSE;
}

//**************************************************************************
int PickFontDialog(LPPARMPASS pParm, HWND hwndParent)
{
	return DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_FONTPICK), hwndParent, FontProc, (LPARAM)pParm);
}

