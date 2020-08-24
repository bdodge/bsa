//--------------------------------------------------------------------
//
// File: commctrl.c
// Desc: common controls for winapi
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------


#ifndef Windows
#include "winapix.h"

//-----------------------------------------------------------------------------

extern LRESULT WINAPI __wproc_Button(HWND hWnd, UINT message, WPARAM wParam, LONG lParam);
extern LRESULT WINAPI __wproc_MenuButton(HWND hWnd, UINT message, WPARAM wParam, LONG lParam);
extern LRESULT WINAPI __wproc_Label(HWND hWnd, UINT message, WPARAM wParam, LONG lParam);
extern LRESULT WINAPI __wproc_Icon(HWND hWnd, UINT message, WPARAM wParam, LONG lParam);
extern LRESULT WINAPI __wproc_Bitmap(HWND hWnd, UINT message, WPARAM wParam, LONG lParam);
extern LRESULT WINAPI __wproc_Edit(HWND hWnd, UINT message, WPARAM wParam, LONG lParam);
extern LRESULT WINAPI __wproc_Menu(HWND hWnd, UINT message, WPARAM wParam, LONG lParam);
extern LRESULT WINAPI __wproc_Scrollbar(HWND hWnd, UINT message, WPARAM wParam, LONG lParam);
extern LRESULT WINAPI __wproc_Scroller(HWND hWnd, UINT message, WPARAM wParam, LONG lParam);
extern LRESULT WINAPI __wproc_ScrollCtrl(HWND hWnd, UINT message, WPARAM wParam, LONG lParam);
extern LRESULT WINAPI __wproc_Trackbar(HWND hWnd, UINT message, WPARAM wParam, LONG lParam);
extern LRESULT WINAPI __wproc_TrackThumb(HWND hWnd, UINT message, WPARAM wParam, LONG lParam);

extern LRESULT WINAPI __wproc_TreeView(HWND hWnd, UINT message, WPARAM wParam, LONG lParam);
extern LRESULT WINAPI __wproc_ListView(HWND hWnd, UINT message, WPARAM wParam, LONG lParam);

extern HFONT _zg_treeFont;

//**************************************************************************
void InitCommonControls()
{
	if(! _zg_treeFont)
	{
		_zg_treeFont = CreateFont(
							15,0,0,0,400,FALSE,FALSE,FALSE,ANSI_CHARSET,
							OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY,FF_SWISS,_T("Arial")
						  );
	}
}

//**************************************************************************
BOOL WINAPI InitCommonControlsEx(LPINITCOMMONCONTROLSEX pccx)
{
	InitCommonControls();
	return TRUE;
}

//**************************************************************************
LRESULT WINAPI __wproc_ListBox(HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{
	return __wproc_Label(hWnd, message, wParam, lParam);
}

//**************************************************************************
LRESULT WINAPI __wproc_Dialog(HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{
	return (LRESULT)DefDlgProc(hWnd, message, wParam, lParam);
}

#define _CBF ((LPZGDIOBJ)(COLOR_BTNFACE+1))
#define _CSB ((LPZGDIOBJ)(COLOR_SCROLLBAR+1))
#define _XAR (IDC_ARROW)
#define _XIB (IDC_IBEAM)

// name              proc        extra,extra,icon,cursor,bkg,menu,refs
//**************************************************************************
ZCLASS _zg_builtin_classes[] =
{
{ (LPTSTR)_T("Button"), 	__wproc_Button,  	0, 0, 0, _XAR, _CBF, 	NULL, 0 	},
{ (LPTSTR)_T("MenuButton"), __wproc_MenuButton, 0, 0, 0, _XAR, _CBF, 	NULL, 0 	},
{ (LPTSTR)_T("Static"),		__wproc_Label,  	0, 0, 0, _XAR, _CBF, 	NULL, 0 	},
{ (LPTSTR)_T("StaticIcon"),	__wproc_Icon, 	 	0, 0, 0, _XAR, _CBF, 	NULL, 0 	},
{ (LPTSTR)_T("StaticBitmap"),__wproc_Bitmap,	0, 0, 0, _XAR, _CBF, 	NULL, 0 	},
{ (LPTSTR)_T("Edit"),  	 	__wproc_Edit,   	0, 0, 0, _XIB,	NULL, 	NULL, 0		},
{ (LPTSTR)_T("Listbox"), 	__wproc_ListBox, 	0, 0, 0, _XAR,	NULL,	NULL, 0		},
{ (LPTSTR)_T("SysTreeView32"),__wproc_TreeView,	0, 0, 0, _XAR,	NULL,	NULL, 0		},
{ (LPTSTR)_T("Dialog"), 	__wproc_Dialog,  	0, 0, 0, _XAR, _CBF, 	NULL, 0		},
{ (LPTSTR)_T("Menu"),	 	__wproc_Menu, 		0, 0, 0, _XAR, _CBF, 	NULL, 0		},
{ (LPTSTR)_T("ScrollBar"), 	__wproc_Scrollbar,	0, 0, 0, _XAR, _CSB, 	NULL, 0		},
{ (LPTSTR)_T("Scroller"),	__wproc_Scroller,  	0, 0, 0, _XAR, _CBF, 	NULL, 0		},
{ (LPTSTR)_T("ScrollCtrl"),	__wproc_ScrollCtrl,	0, 0, 0, _XAR, _CBF, 	NULL, 0		},
{ (LPTSTR)TRACKBAR_CLASS,	__wproc_Trackbar,  	0, 0, 0, _XAR, _CBF, 	NULL, 0		},
{ (LPTSTR)_T("TrackThumb"),	__wproc_TrackThumb,	0, 0, 0, _XAR, _CBF, 	NULL, 0		},
{ (LPTSTR)WC_LISTVIEW,		__wproc_ListView,	0, 0, 0, _XAR,	NULL,	NULL, 0		},
{ (LPTSTR)WC_TREEVIEW,		__wproc_TreeView,	0, 0, 0, _XAR,	NULL,	NULL, 0		},

{ NULL, 			NULL,			 	0, 0, 0, 0, 	NULL,			NULL, 0	}
};


#endif
