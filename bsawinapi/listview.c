//--------------------------------------------------------------------
//
// File: listview.c
// Desc: ListView controls for winapi
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------


#ifndef Windows
#include "winapix.h"

extern HWND	_zg_dialogWnd[];
extern int	_zg_dialogTop;

		
//**************************************************************************
LRESULT WINAPI __wproc_ListView(HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{
	PAINTSTRUCT ps;
	RECT		rc;
	HDC         hdc;
	LPZWND		zWnd;
	DWORD		style;
	DWORD		exstyle;

	if(! (zWnd = Zwindow(hWnd))) return -1;
	style  	= GetWindowLong(hWnd, GWL_STYLE);
	exstyle	= GetWindowLong(hWnd, GWL_EXSTYLE);

	GetClientRect(hWnd, &rc);

	switch(message)
	{
	case WM_CREATE:
		
		break;
		
	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		if(exstyle & WS_EX_CLIENTEDGE)
			__w_checkbox(hdc, 0, 0, !(style & WS_DISABLED), &rc);
		EndPaint(hWnd, &ps);
		break;

	case WM_LBUTTONDOWN:

		if(GetFocus() != hWnd)
			SetFocus(hWnd);
		break;

	case WM_LBUTTONUP:

		ReleaseCapture();
		break;

	case WM_LBUTTONDBLCLK:

		break;

	case WM_MOUSEMOVE:

		break;

	case WM_SETFOCUS:

		break;

	case WM_KILLFOCUS:

		break;
		
	case WM_KEYDOWN:
		
		switch(wParam)
		{
		case VK_LEFT:	
			break;
			
		case VK_RIGHT:	
			break;
			
		case VK_HOME:	
			break;
			
		case VK_END:	
			break;
						
		case VK_RETURN:
		case VK_TAB:

			// pass this up to the dialog parent if present
			//
			if(_zg_dialogTop >= 0 && _zg_dialogWnd[_zg_dialogTop])
			{
				SendMessage(_zg_dialogWnd[_zg_dialogTop], WM_KEYDOWN, wParam, lParam);
				break;
			}
			break;

		default:
			break;
			
		}
		break;
		
	case WM_CHAR:
		
		switch(wParam)
		{
		case 8:
		case 10:
		case 13:
			break;
		default:
			break;
		}
		InvalidateRect(hWnd, NULL, TRUE);
		break;

	case WM_CLOSE:

		break;
		
	default:
		
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

#endif
