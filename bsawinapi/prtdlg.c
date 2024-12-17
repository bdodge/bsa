
//--------------------------------------------------------------------
//
// File: prtdlg.c
// Desc: Printer/Page Setup dialog (like window PrintDlg)
//       also handles holding current "devmode" type stuff
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------


#ifndef Windows
#include "winapix.h"
#include "resource.h"

//-----------------------------------------------------------------------------

DEVMODE g_devMode;

typedef struct
{
	int			code;	// corresponding DM paper code
	int			w, h;	// dimensions in 1/10 mm
	int			x, y;	// dimensions in pixels at 1200 dpi
	int			t,b;	// margins in pixels top/bottom
	int			l,r;	// margins in pixels left/right
	LPCTSTR		pName;	// string ID of name of paper
}
PTAB, *LPPTAB;


PTAB	g_paperTable[] =
{
	{	DMPAPER_LETTER,	2160,	2794,	10200,	13200,	80,	600,301,301,_T("Letter")	},
	{	DMPAPER_LEGAL,	2160,	3556,	10200,	16800,	80,	600,160,160,_T("Legal")		},
	{	DMPAPER_A4,		2100,	2970,	9921,	14031,	80,	600,160,160,_T("A4")		},
	{	DMPAPER_A5,		1480,	2100,	6960,	9960,	80,	600,160,160,_T("A5")		},
	{	DMPAPER_B5,		1820,	2570,	8592,	12144,	80,	600,160,160,_T("B5")		},
	{	DMPAPER_EXECUTIVE,1842,	2667,	8700,	12600,	80,	600,160,160,_T("Executive")	}
};

#define NUM_PAPER_SIZES (sizeof(g_paperTable) / sizeof(PTAB))

#if 0
//**************************************************************************
static BOOL CALLBACK PrtMediaProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT	rc;
	RECT	rcme;
	HWND	hwndParent;
	HWND	hwndTree;
	LPNMHDR	nmhdr;
	int		i;

	TVITEM			tvi;
	TVINSERTSTRUCT  tvItem;
	HTREEITEM		hItem;

	switch (message)
	{
	case WM_INITDIALOG:

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
		// enumerate available papersizes into treeview
		//
		hwndTree = GetDlgItem(hWnd, IDC_PRTMEDIA);
		if(hwndTree)
		{
			BOOL noselect;

			for(i = 0, noselect = TRUE; i < NUM_PAPER_SIZES; i++)
			{
				tvItem.hParent		= NULL;
				tvItem.hInsertAfter = TVI_LAST;

				tvItem.item.mask	= TVIF_TEXT | TVIF_PARAM;
				tvItem.item.pszText = (LPTSTR)g_paperTable[i].pName;
				tvItem.item.lParam	= g_paperTable[i].code;

				hItem = TreeView_InsertItem(hwndTree, &tvItem);

				if(g_paperTable[i].code == g_devMode.dmPaperSize)
				{
					TreeView_SelectItem(hwndTree, hItem);
					noselect = FALSE;
				}
			}
			if(noselect)
				TreeView_SelectItem(hwndTree, TreeView_GetRoot(hwndTree));
		}
		return TRUE;

	case WM_COMMAND:

		switch(wParam)
		{
		case IDOK:

			hwndTree = GetDlgItem(hWnd, IDC_PRTMEDIA);
			if(hwndTree)
			{
				if((hItem = TreeView_GetSelection(hwndTree)) != NULL)
				{
					tvi.hItem = hItem;
					tvi.mask  = TVIF_PARAM;
					if(TreeView_GetItem(hwndTree, &tvi))
					{
						//pPrt->SetPaperCode(tvi.lParam);
					}
				}
			}
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
		case TVN_SELCHANGED:

			break;
		}
		break;

	default:
		break;
	}
	return FALSE;
}
#endif

//**************************************************************************
static BOOL CALLBACK PrtPrinterProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT	rc;
	RECT	rcme;
	HWND	hwndParent;
	LPNMHDR	nmhdr;
	int		mrc;

	static PRINTDLG* pps = NULL;

	switch (message)
	{
	case WM_INITDIALOG:

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
		pps = (PRINTDLG*)lParam;
		return TRUE;

	case WM_COMMAND:

		switch(wParam)
		{
		case IDC_MEDIA:

			{
			LPBYTE  res;
			DWORD	cbdata;

			// find dialog in resource file
			//
			res = (LPBYTE)__FindResource(RT_Dialog, MAKEINTRESOURCE(IDD_PRTMEDIA), _zg_winapires, _cb_zg_winapires);

			if(! res)
			{
				SetLastError(ERROR_RESOURCE_DATA_NOT_FOUND);
				return FALSE;
			}
			// extract the resource header info
			//
			cbdata = RESDWORD(res);			// data size
			cbdata = RESDWORD(res);			// hdr size
			res -= 8;
			res += ((cbdata + 3) & ~3);

			mrc = DialogBoxIndirectParam(
										(HINSTANCE)0xBEEF,
										(LPDLGTEMPLATE)res,
										pps->hwndOwner,
										PrtPrinterProc,
										(DWORD)(uintptr_t)pps
									);
			if(mrc == IDOK)
			{
			}
			}
			break;

		case IDOK:

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
		case TVN_SELCHANGED:

			break;
		}
		break;

	default:
		break;
	}
	return FALSE;
}

//**************************************************************************
BOOL WINAPI PrintDlg(LPPRINTDLG pps)
{
	LPBYTE  res;
	DWORD	cbdata;
	int		rc;

	// find dialog in resource file
	//
	res = (LPBYTE)__FindResource(RT_Dialog, MAKEINTRESOURCE(IDD_PRINTER), _zg_winapires, _cb_zg_winapires);

	if(! res)
	{
		SetLastError(ERROR_RESOURCE_DATA_NOT_FOUND);
		return FALSE;
	}
	// extract the resource header info
	//
	cbdata = RESDWORD(res);			// data size
	cbdata = RESDWORD(res);			// hdr size
	res -= 8;
	res += ((cbdata + 3) & ~3);

	rc = DialogBoxIndirectParam(
								(HINSTANCE)0xBEEF,
								(LPDLGTEMPLATE)res,
								pps->hwndOwner,
								PrtPrinterProc,
								(DWORD)(uintptr_t)pps
							);
	return rc == IDOK;
}

#endif



