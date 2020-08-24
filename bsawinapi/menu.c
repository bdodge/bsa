//--------------------------------------------------------------------
//
// File: menu.c
// Desc: menus for winapi
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------

#include "winapix.h"

#define BSA_MENU_TEXT_HEIGHT	18
#define BSA_MENU_TEXT_TMARG		1
#define BSA_MENU_TEXT_TL_LMARG	5
#define BSA_MENU_TEXT_TL_RMARG	5
#define BSA_MENU_TEXT_LMARG		14
#define BSA_MENU_BUTTON_LMARG	3
#define BSA_MENU_TEXT_RMARG		14
#define BSA_POPUP_ITEM_TMARG	3
#define BSA_POPUP_ITEM_BMARG	3
#define BSA_POPUP_ARROW_WIDTH	6

#define BSA_MAX_NESTED_POPUPS	64

HWND		 _zg_menuWnd[BSA_MAX_NESTED_POPUPS];
LPZMENU		 _zg_menuMen[BSA_MAX_NESTED_POPUPS];
int			 _zg_menuID[BSA_MAX_NESTED_POPUPS];
int			 _zg_menuTop  = -1;
BOOL		 _zg_endMenus = FALSE;
extern HFONT _zg_menuFont;
extern int   _zg_dialogTop;
extern void __w_button_box(HDC hdc, int pushed, LPRECT rc);
extern void __w_drawbutton(HDC hdc, HWND hWnd, DWORD style, LPCTSTR text, BOOL pushed, BOOL iscur);


//**************************************************************************
LPZMENU _w_newZMENU(LPCTSTR text, WORD opts, WORD id)
{
	LPZMENU menu = (LPZMENU)malloc(sizeof(ZMENU));
	
	if(text) 
	{
		menu->text = (LPTSTR)malloc((_tcslen(text) + 2) * sizeof(TCHAR));
		if(menu->text)
			_tcscpy(menu->text, text);
	}
	else
	{
		opts |= MF_SEPARATOR;
		menu->text = NULL;
	}
	menu->opts    = opts | MF_ENABLED;
	menu->id      = id;
	menu->width	  = 0;
	menu->parent  = NULL;
	menu->sibling = NULL;
	menu->child   = NULL;
	menu->cur	  = NULL;
	menu->entered = FALSE;
	menu->pushed  = FALSE;
	menu->zWnd	  = NULL;
	return menu;
}

//**************************************************************************
void _w_deleteZMENU(LPZMENU menu)
{
	// recursively delete all sub/sib menus
	// in place, cause don't want any dangling messages to them
	//
	if(menu)
	{
		if(menu->sibling)
		{
			_w_deleteZMENU(menu->sibling);
			menu->sibling = NULL;
		}			
		if(menu->child)
		{
			_w_deleteZMENU(menu->child);
			menu->child = NULL;
		}
		if(menu->text)
		{
			free(menu->text);
			menu->text = NULL;
		}
		if(menu->zWnd)
		{
			DestroyWindow(menu->zWnd);
			menu->zWnd = NULL;
		}
		free(menu);
	}
}

//**************************************************************************
static LPZMENU __menu_from_pos(LPZMENU zMenu, UINT pos)
{
	int ipos = 0;
	
	if((! zMenu->text || (zMenu->opts & MF_POPUP)) && zMenu->child)
		zMenu = zMenu->child;

	while(zMenu)
	{
		if(ipos == pos)
			return zMenu;
		ipos++;
		zMenu = zMenu->sibling;
	}
	return NULL;
}

//**************************************************************************
static int __pos_from_menu(LPZMENU zMenu)
{
	LPZMENU zItem;
	int 	ipos;

	if(! zMenu) return 0;
	if(! zMenu->parent) return 0;

	for(
			ipos = 0,

			zItem = zMenu->parent->child;
			zItem != zMenu;

			ipos++
	)
		 zItem = zItem->sibling;
	return ipos;
}

//**************************************************************************
static LPZMENU __menu_from_id(LPZMENU zMenu, UINT id)
{
	while(zMenu)
	{
		if(zMenu->id == id)
			return zMenu;
		if(zMenu->child)
			zMenu = zMenu->child;
		else if(zMenu->sibling)
			zMenu = zMenu->sibling;
		else 
		{
			do
			{
				zMenu = zMenu->parent;
				if(zMenu && zMenu->sibling)
				{
					zMenu = zMenu->sibling;
					break;
				}
			}
			while(zMenu);
		}
	}
	return NULL;
}

//**************************************************************************
static LPZMENU __menu_from_poscmd(LPZMENU zMenu, UINT poscmd, BOOL bypos)
{
	if(bypos)
		return __menu_from_pos(zMenu, poscmd);
	else
		return __menu_from_id(zMenu, poscmd);
}

//**************************************************************************
void __endpopups(int id)
{
	if(_zg_menuTop >= 0)
	{
		//_tprintf(_T("endpops id=%d top=%d\n"), id, _zg_menuTop);
		_zg_menuID[_zg_menuTop] = id;
		_zg_menuWnd[_zg_menuTop] = NULL;
		_zg_endMenus = TRUE;
	}
}

//**************************************************************************
static void __addmenuitems(LPZMENU zMenu, HWND hWnd, BOOL horiz)
{
	LPZMENU zItem;
	int		maxwidth;
	int		width, height;
	int		x, y;
	BOOL	popups = FALSE;
	
	HFONT hOldFont;
	HDC   hdc;
	
	hdc = GetDC(hWnd);
	hOldFont = SelectObject(hdc, _zg_menuFont);

	for(zItem = zMenu, maxwidth = 0; zItem; zItem = zItem->sibling)
	{
		if(! zItem->width)
		{
			if(zItem->text && zItem->text[0])
			{
				SIZE  sizeText;
				
				zItem->opts &= ~MF_SEPARATOR;
				GetTextExtentPoint32(hdc, zItem->text, _tcslen(zItem->text), &sizeText);
				zItem->width  = sizeText.cx;
				zItem->height = sizeText.cy;
			}
			else
			{
				zItem->width  = 0;
				zItem->height = 4;
				zItem->opts |= MF_SEPARATOR;
			}
			if(zItem->opts & MF_POPUP)
				popups = TRUE;
		}
		if(zItem->width > maxwidth)
			maxwidth = zItem->width;
	}
	SelectObject(hdc, hOldFont);
	ReleaseDC(hWnd, hdc);
	
	if(horiz)
	{
		x = BSA_MENU_TEXT_TL_LMARG;
		y = 1;
	}
	else
	{
		x = BSA_MENU_BUTTON_LMARG;
		y = 3;
	}
	
	for(zItem = zMenu; zItem; zItem = zItem->sibling)
	{
		if(horiz)
		{
			width = BSA_MENU_TEXT_TL_LMARG + zItem->width + BSA_MENU_TEXT_TL_RMARG;
			height = zItem->height +  2 * BSA_MENU_TEXT_TMARG;
		}
		else
		{
			width = BSA_MENU_TEXT_LMARG + maxwidth + BSA_MENU_TEXT_RMARG;
			height = zItem->height + BSA_POPUP_ITEM_TMARG + BSA_POPUP_ITEM_BMARG;
		}
		zItem->zWnd = CreateWindowEx(
										0,
										_T("MenuButton"),
										(zItem->text ? zItem->text : _T("")),
											WS_CHILD | BS_FLAT |
											((horiz) ? BS_LEFTTEXT : BS_CHECKBOX) |
											((zItem->opts & MF_GRAYED) ? WS_DISABLED : 0),
										x, y,
										width, height,
										hWnd, NULL, (HINSTANCE)0xbeef,
										zItem
									);
		zItem->zWnd->font = _zg_menuFont;
		SetWindowLong(zItem->zWnd, GWL_ID, zItem->id);
		
		ShowWindow(zItem->zWnd, SW_SHOW);

		if(horiz)
			x += width;
		else
			y += BSA_POPUP_ITEM_TMARG + 
					zItem->height + BSA_POPUP_ITEM_BMARG;
	}
}

//**************************************************************************
BOOL WINAPI SetMenu(HWND hWnd, HMENU hMenu)
{
	LPZWND  zWnd = Zwindow(hWnd);
	LPZMENU zMenu = (LPZMENU)hMenu;	
	
	if(zWnd && zMenu)
	{
		RECT  rc;
		HFONT hOldFont;
		HDC   hdc;
		SIZE  sizeText;
		int   menuheight;
		
		if(zWnd->menu)
			DestroyMenu(zWnd->menu);
		
		zWnd->menu = zMenu;
		GetClientRect(hWnd, &rc);

		hdc = GetDC(hWnd);
		hOldFont = SelectObject(hdc, _zg_menuFont);
		GetTextExtentPoint32(hdc, _T("Menu"), 4, &sizeText);
		SelectObject(hdc, hOldFont);
		menuheight = sizeText.cy + 4 * BSA_MENU_TEXT_TMARG;
		if(menuheight < BSA_MENU_HEIGHT)
			menuheight = BSA_MENU_HEIGHT;
		ReleaseDC(hWnd, hdc);

		zMenu->zWnd = CreateWindowEx(
								0,
								_T("Menu"), _T("menu"), 
								WS_CHILD,
								rc.left, rc.top,
								rc.right-rc.left, menuheight,
								hWnd, NULL, (HINSTANCE)0xbeef,
								zMenu
								);
		zMenu->zWnd->font = _zg_menuFont;

		SendMessage(hWnd, WM_INITMENU, (WPARAM)zMenu, 0);		
		ShowWindow(zMenu->zWnd, SW_SHOW);
	
		__addmenuitems(zMenu->child, zMenu->zWnd, TRUE);	

		return TRUE;
	}
	SetLastError(ERROR_INVALID_PARAMETER);
	return FALSE;
}

//**************************************************************************
HMENU WINAPI GetMenu(HWND hWnd)
{
	LPZWND zWnd = Zwindow(hWnd);
	
	if(zWnd) return (HMENU)zWnd->menu;
	SetLastError(ERROR_INVALID_PARAMETER);
	return NULL;
}

//**************************************************************************
HMENU WINAPI CreateMenu(void)
{
	LPZMENU zMenu = _w_newZMENU(NULL, 0, -1);
	return (HMENU)zMenu;
}

//**************************************************************************
HMENU WINAPI CreatePopupMenu(void)
{
	LPZMENU zMenu = (LPZMENU)CreateMenu();
	zMenu->opts = MF_POPUP;
	zMenu->zWnd = NULL;
	return zMenu;
}

//**************************************************************************
BOOL  WINAPI DestroyMenu(HMENU hMenu)
{
	LPZMENU zMenu = (LPZMENU)hMenu;
	LPZMENU zNext;
	
	while(zMenu)
	{
		zNext = zMenu->sibling;
		_w_deleteZMENU(zMenu);
		zMenu = zNext;
	}
	return TRUE;
}


//**************************************************************************
DWORD WINAPI CheckMenuItem(
							HMENU	hMenu,
							UINT	uIDCheckItem,
							UINT	uCheck)
{
	LPZMENU zMenu;
	LPZMENU zItem;
	UINT    ps;
	
	if(! (zMenu = (LPZMENU)hMenu))
		return -1;
	zItem = __menu_from_poscmd(zMenu, uIDCheckItem, uCheck & MF_BYPOSITION);
	if(! zItem)
		return -1;
	ps = zItem->opts & MF_CHECKED;
	zItem->opts &= ~MF_CHECKED;
	zItem->opts |= (uCheck & MF_CHECKED);
	if(zItem->zWnd && (ps != zItem->opts))
		InvalidateRect(zItem->zWnd, NULL, FALSE);
	return ps;
}


//**************************************************************************
BOOL WINAPI EnableMenuItem(
							HMENU	hMenu,
							UINT	uIDEnableItem,
							UINT	uEnable)
{
	LPZMENU zMenu;
	LPZMENU zItem;
	UINT    ps;
	
	if(! (zMenu = (LPZMENU)hMenu))
		return FALSE;
	zItem = __menu_from_poscmd(zMenu, uIDEnableItem, uEnable & MF_BYPOSITION);
	if(! zItem)
		return FALSE;
	ps = zItem->opts & (MF_DISABLED|MF_GRAYED);
	zItem->opts &= ~(MF_DISABLED|MF_GRAYED);
	zItem->opts |= (uEnable & (MF_DISABLED|MF_GRAYED));
	if(zItem->zWnd && (ps != (zItem->opts & (MF_DISABLED|MF_GRAYED))))
	{
		EnableWindow(zItem->zWnd, (zItem->opts & (MF_DISABLED|MF_GRAYED)) ? FALSE :	TRUE);
	}
	return TRUE;
}


//**************************************************************************
HMENU WINAPI GetSubMenu(HMENU hMenu, int nPos)
{
	LPZMENU zMenu;
	
	if(! (zMenu = (LPZMENU)hMenu))
		return NULL;
	zMenu =  __menu_from_poscmd(zMenu->child, nPos, TRUE);
	return zMenu;
}


//**************************************************************************
UINT WINAPI GetMenuItemID(HMENU hMenu, int nPos)
{
	LPZMENU zMenu;
	
	if(! (zMenu = (LPZMENU)hMenu))
		return -1;
	return zMenu->id;
}

//**************************************************************************
UINT WINAPI GetMenuState(HMENU hMenu, UINT uId, UINT uFlags)
{
	LPZMENU zMenu;
	LPZMENU zItem;
	
	if(! (zMenu = (LPZMENU)hMenu))
		return -1;
	zItem = __menu_from_poscmd(zMenu, uId, uFlags & MF_BYPOSITION);
	if(! zItem)
		return -1;
	return zItem->opts & MF_CHECKED;
}

//**************************************************************************
int WINAPI GetMenuItemCount(HMENU hMenu)
{
	LPZMENU zMenu;
	LPZMENU zItem;
	int		i;
		
	if(! (zMenu = (LPZMENU)hMenu))
		return -1;
	for(i = 0, zItem = zMenu; zItem; zItem = zItem->sibling)
		i++;
	return i;
}


//**************************************************************************
BOOL WINAPI InsertMenu(
							HMENU	hMenu,
							UINT	uPosition,
							UINT	uFlags,
							UINT	uIDNewItem,
							LPCTSTR lpNewItem
							)
{
	return TRUE;
}


//**************************************************************************
BOOL WINAPI AppendMenu(
							HMENU	hMenu,
							UINT	uFlags,
							UINT	uIDNewItem,
							LPCTSTR lpNewItem
							)
{
	LPZMENU zMenu;
	LPZMENU zItem;
	
	if(! (zMenu = (LPZMENU)hMenu))
		return FALSE;
	zItem = _w_newZMENU(lpNewItem, 0, uIDNewItem);
	if(zMenu->child)
	{
		for(zMenu = zMenu->child; zMenu->sibling;)
			zMenu = zMenu->sibling;
		zMenu->sibling = zItem;
		zItem->parent = zMenu->parent;
	}
	else
	{
		zMenu->child = zItem;
		zItem->parent = zMenu;
	}
	return TRUE;
}

//**************************************************************************
BOOL WINAPI ModifyMenu(
							HMENU	hMenu,
						    UINT  	uPosition,
							UINT	uFlags,
							UINT	uIDNewItem,
							LPCTSTR lpNewItem
							)
{
	LPZMENU zMenu;
	LPZMENU zItem;
	
	if(! (zMenu = (LPZMENU)hMenu))
		return FALSE;
	zItem = __menu_from_poscmd(zMenu, uPosition, uFlags & MF_BYPOSITION);
	if(! zItem)
		return FALSE;
	zItem->id = uIDNewItem;
	if(! (uFlags & ~ (MF_BYCOMMAND | MF_BYPOSITION)))
	{
		if(zItem->text)
		{
			free(zItem->text);
			zItem->text = NULL;
		}
		zItem->text = (LPTSTR)malloc((_tcslen(lpNewItem) + 2) * sizeof(TCHAR));
		if(zItem->text)
			_tcscpy(zItem->text, lpNewItem);
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}


//**************************************************************************
BOOL WINAPI RemoveMenu(
						    HMENU hMenu,
						    UINT  uPosition,
						    UINT  uFlags)
{
	LPZMENU zMenu;
	LPZMENU zItem;
	
	if(! (zMenu = (LPZMENU)hMenu))
		return FALSE;
	zItem = __menu_from_poscmd(zMenu, uPosition, uFlags & MF_BYPOSITION);
	if(! zItem)
		return FALSE;
	
	// unlink menu tree from parent
	//
	if(zMenu->child == zItem)
	{
		zMenu->child = zItem->sibling;
	}
	else
	{
		zMenu = zMenu->child;
		while(zMenu->sibling != zItem)
			zMenu = zMenu->sibling;
		zMenu->sibling = zItem->sibling;
	}
	return TRUE;
}

//**************************************************************************
BOOL WINAPI DeleteMenu(
							HMENU	hMenu,
							UINT	uPosition,
							UINT	uFlags)
{
	LPZMENU zMenu;
	LPZMENU zItem;
	
	if(! (zMenu = (LPZMENU)hMenu))
		return FALSE;
	zItem = __menu_from_poscmd(zMenu, uPosition, uFlags & MF_BYPOSITION);
	if(! zItem)
		return FALSE;
	RemoveMenu(hMenu, uPosition, uFlags);
	zItem->child = zItem->sibling = NULL;
	_w_deleteZMENU(zItem);
	return TRUE;
}

/*
BOOL
WINAPI
SetMenuItemBitmaps(
	HMENU hMenu,
	UINT uPosition,
	UINT uFlags,
	HBITMAP hBitmapUnchecked,
	HBITMAP hBitmapChecked);
*/

//**************************************************************************
/*BOOL*/ int WINAPI TrackPopupMenu(
							HMENU	hMenu,
							UINT	uFlags,
							int		x,
							int		y,
							int		nReserved,
							HWND	hWnd,
							CONST RECT *prcRect)
{
	LPZWND	zPopupWnd;
	LPZMENU zMenu;
	LPZMENU zItem;
	HWND    hWndTop;
	MSG		msg;
	HDC	 hdc;
	SIZE    sizeText;
	int		maxwidth;
	int		height;
	int		rc;
	BOOL	popups = FALSE;
	
	if(! (zMenu = (LPZMENU)hMenu)) return FALSE;

	for(zItem = zMenu; zItem->parent && zItem->parent->zWnd;)
		zItem = zItem->parent;

	hWndTop = GetParent(zItem->zWnd);

	SendMessage(hWndTop, WM_INITMENUPOPUP, (WPARAM)zMenu, nReserved);
	
	// get tentative size for menu based on parent window's DC
	//
	for(zItem = zMenu->child, maxwidth = height = 0; zItem; zItem = zItem->sibling)
	{
		if(! zItem->width)
		{
			if(zItem->text && zItem->text[0])
			{
				hdc = GetDC(hWnd);
				GetTextExtentPoint32(hdc, zItem->text, _tcslen(zItem->text), &sizeText);
				zItem->width  = sizeText.cx;
				zItem->height = sizeText.cy;
				ReleaseDC(hWnd, hdc);
			}
			else
			{
				zItem->width  = 0;
				zItem->height = 4;
				zItem->opts |= MF_SEPARATOR;
			}
		}
		if(zItem->opts & MF_POPUP)
			popups = TRUE;
		if(zItem->width > maxwidth)
			maxwidth = zItem->width;
		height += BSA_POPUP_ITEM_TMARG + 
					zItem->height + BSA_POPUP_ITEM_BMARG;
	}
	maxwidth += BSA_MENU_TEXT_LMARG + BSA_MENU_BUTTON_LMARG + BSA_MENU_TEXT_RMARG + 6 /* + (popups ? BSA_POPUP_ARROW_WIDTH : 0)*/;
	height += 6;

	// create menu window from zMenu
	zPopupWnd = CreateWindowEx(
								0,
								_T("Menu"), _T("pu"),
								WS_CHILD,
								x, y,
								maxwidth, height,
								hWndTop,
								NULL,
								(HINSTANCE)0xbeef,
								zMenu
								);

	zPopupWnd->font = _zg_menuFont;
	ShowWindow(zPopupWnd, SW_SHOW);
	
	__addmenuitems(zMenu->child, zPopupWnd, FALSE);	
	//zPopupWnd->menu = zMenu;
	
	// push menu on top the dialog stack
	_zg_menuWnd[++_zg_menuTop] = zPopupWnd;
	_zg_menuMen[_zg_menuTop]   = zMenu;
	_zg_menuID[_zg_menuTop]	= 0;
	
	SetFocus(zPopupWnd);
	
	// modal wait
	while(GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		rc = DispatchMessage(&msg);
		if(! _zg_menuWnd[_zg_menuTop])
		{
			rc = _zg_menuID[_zg_menuTop];
			break;
		}
	}
	// pass focus up a level 
	if(_zg_menuTop > 0)
		SetFocus(_zg_menuWnd[_zg_menuTop - 1]);

	// pop off the popup stack
	DestroyWindow(zPopupWnd);
	_zg_menuTop--;
	
	//_tprintf(_T("end popup %d, em=%d rc=%d\n"), _zg_menuTop + 1, _zg_endMenus, rc);
	
	if(_zg_endMenus)
	{
		int i = _zg_menuTop;
				
		while(i >= 0)
		{
			DestroyWindow(_zg_menuWnd[i]);
			_zg_menuWnd[i--] = NULL;
		}
		_zg_endMenus = FALSE;
	}
	if(uFlags & TPM_RETURNCMD)
		return rc;
	else
		return TRUE;
}

//**************************************************************************
LPZMENU __menu_from_popup(HWND hWnd)
{
	int i;
	
	for(i = 0; i <= _zg_menuTop; i++)
		if(_zg_menuWnd[i] == hWnd)
			return _zg_menuMen[i];
	return NULL;
}	

//**************************************************************************
void __w_menukey(HMENU menu, WPARAM key)
{
	LPZMENU zMenu;
	LPZMENU zItem;
	LPTSTR  pt;
	
	if(! (zMenu = (LPZMENU)menu)) return;
	
	if(_zg_menuTop >= 0)
		zMenu = __menu_from_popup(_zg_menuWnd[_zg_menuTop]);

	for(zItem = zMenu->child; zItem; zItem = zItem->sibling)
	{
		// look for "&k" where k is key, in item text
		//
		for(pt = zItem->text; pt && *pt; pt++)
		{
			if(*pt == _T('&'))
				if(*(pt+1) == (TCHAR)key)
					break;
		}
		if(pt && *pt)
			break;
	}
	if(zItem && zItem->zWnd)
	{
		// got accel key, so hit it
		//
		PostMessage(GetParent(zItem->zWnd), WM_COMMAND, zItem->id, 0);
	}
}

//**************************************************************************
LRESULT WINAPI __wproc_Menu(
						    HWND hWnd,
						    UINT Msg,
						    WPARAM wParam,
						    LPARAM lParam
						  )
{
	PAINTSTRUCT ps;
	HDC		 hdc;
	LPZWND		zWnd;
	LPZMENU		zMenu;
	LPZMENU		zItem;
	RECT		rc, rcm;
	
	if(! (zWnd = Zwindow(hWnd))) return -1;
	
	switch(Msg)
	{
	case WM_CREATE:

		SetWindowLong(hWnd, GWL_USERDATA,
				(LONG)((LPCREATESTRUCT)lParam)->lpCreateParams);
		zMenu = (LPZMENU)GetWindowLong(hWnd, GWL_USERDATA);
		if(! zMenu) return -1;
		zMenu->cur = NULL;
		break;
		
	case WM_PAINT:

		zMenu = (LPZMENU)GetWindowLong(hWnd, GWL_USERDATA);
		
		hdc = BeginPaint(hWnd, &ps);
		{
			HBRUSH hBrush;
			HPEN hPen;
			HPEN hOld;
			
			GetClientRect(hWnd, &rc);
			hBrush =  CreateSolidBrush(GetBkColor(hdc));
			FillRect(hdc, &rc, hBrush);
			DeleteObject(hBrush);
			
			if(zMenu)
			{
				// br outside
				hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DDKSHADOW));
				hOld = (HPEN)SelectObject(hdc, hPen);
				MoveToEx(hdc, rc.left, rc.bottom, NULL);
				LineTo(hdc, rc.right, rc.bottom);
				LineTo(hdc, rc.right, rc.top-1);
				SelectObject(hdc, hOld);	
				DeleteObject(hPen);
				
				// ul outside
				hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DLIGHT));
				hOld = (HPEN)SelectObject(hdc, hPen);
				MoveToEx(hdc, rc.right-1, rc.top, NULL);
				LineTo(hdc, rc.left, rc.top);
				LineTo(hdc, rc.left, rc.bottom);
				SelectObject(hdc, hOld);	
				DeleteObject(hPen);
				
				// br inside
				hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DSHADOW));
				hOld = (HPEN)SelectObject(hdc, hPen);
				MoveToEx(hdc, rc.left + 1, rc.bottom - 1, NULL);
				LineTo(hdc, rc.right - 1, rc.bottom - 1);
				LineTo(hdc, rc.right - 1, rc.top);
				SelectObject(hdc, hOld);	
				DeleteObject(hPen);
				
				// ul inside
				hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DHILIGHT));
				hOld = (HPEN)SelectObject(hdc, hPen);
				MoveToEx(hdc, rc.right - 2, rc.top + 1, NULL);
				LineTo(hdc, rc.left + 1, rc.top + 1);
				LineTo(hdc, rc.left + 1, rc.bottom - 1);
				SelectObject(hdc, hOld);
				DeleteObject(hPen);
			}
			else
			{
				hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DSHADOW));
				hOld = SelectObject(hdc, hPen);
				MoveToEx(hdc, rc.left, rc.bottom - rc.top, NULL);
				LineTo(hdc, rc.right - rc.left, rc.bottom - rc.top);
				SelectObject(hdc, hOld);
				DeleteObject(hPen);
				hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DHIGHLIGHT));
				hOld = SelectObject(hdc, hPen);
				MoveToEx(hdc, rc.left, rc.top, NULL);
				LineTo(hdc, rc.right - rc.left, rc.top);
				SelectObject(hdc, hOld);
				DeleteObject(hPen);
			}
		}
		EndPaint(hWnd,  &ps);
		break;

	case WM_COMMAND:

		zMenu = GetMenu(GetParent(hWnd));
		if(! zMenu) zMenu = GetMenu(hWnd);
		if(! zMenu) zMenu = (LPZMENU)GetWindowLong(hWnd, GWL_USERDATA);
		if(! zMenu) break;
		
		zItem = __menu_from_id(zMenu, wParam);

		if(zItem)
		{
			if(zItem->opts & MF_POPUP)
			{
				HWND hParent  = GetParent(hWnd);
				HWND hControl = zItem->zWnd;
				RECT rcp, rcc;
				int  x, y;
				
				GetWindowRect(hControl, &rcc);
				GetWindowRect(hParent, &rcp);
				
				if(zItem->parent && (zItem->parent->opts & MF_POPUP))
				{
					LPZMENU zParent = zItem->parent;
					
					while(zParent && (zParent->opts & MF_POPUP))
						zParent = zParent->parent;
					if(zParent)
						GetWindowRect(zParent->zWnd, &rcm);
					else
						rcm.top = rcm.left = 0;
					x = rcc.right - rcm.left;
					y = rcc.top - rcp.top - (rcm.bottom - rcm.top);
				}
				else
				{
					if(_zg_menuTop >= 0)
					{
						_zg_menuWnd[_zg_menuTop] = NULL;
						PostMessage(hWnd, WM_COMMAND, wParam, 0);
						return 0;
					}
					x = rcc.left - rcp.left;
					GetWindowRect(hWnd, &rcm);
					y = rcc.top - rcp.top - rcm.top;
					y += (rcc.bottom - rcc.top);
					y = 0;
				}
				if(x < 0) x = 0;
				if(y < 0) y = 0;
				
				TrackPopupMenu(zItem, 0, x, y, __pos_from_menu(zItem), GetParent(hWnd), NULL);
			}
			else
			{
				__endpopups(zItem->id);

				hWnd = GetParent(hWnd);
				if(hWnd)
					PostMessage(hWnd, WM_COMMAND, zItem->id, 0);
				
			}
		}
		break;

	case WM_KEYDOWN:
		
		zMenu = (LPZMENU)GetWindowLong(hWnd, GWL_USERDATA);
		if(! zMenu) break;
		
		switch(wParam)
		{
		case VK_UP:

			if(! zMenu->cur)
			{
				for(zItem = zMenu->child; zItem && zItem->sibling;)
					zItem = zItem->sibling;
			}
			else
			{
				zItem = zMenu->child;
				if(zItem == zMenu->cur)
					while(zItem && zItem->sibling)
						zItem = zItem->sibling;
				else
					while(zItem && (zItem->sibling != zMenu->cur))
						zItem = zItem->sibling;
			}
			if(zItem)
			{
				if(zMenu->cur)
				{
					zMenu->cur->entered = FALSE;
					InvalidateRect(zMenu->cur->zWnd, NULL, FALSE);
				}
				zMenu->cur = zItem;
				zMenu->cur->entered = TRUE;
				InvalidateRect(zMenu->cur->zWnd, NULL, FALSE);
			}
			break;
		
		case VK_DOWN:

			if(! zMenu->cur)
			{
				zItem = zMenu->child;
			}
			else
			{
				if(zMenu->cur->sibling)
					zItem = zMenu->cur->sibling;
				else
					zItem = zMenu->child;
			}
			if(zItem)
			{
				if(zMenu->cur)
				{
					zMenu->cur->entered = FALSE;
					InvalidateRect(zMenu->cur->zWnd, NULL, FALSE);
				}
				zMenu->cur = zItem;
				zMenu->cur->entered = TRUE;
				InvalidateRect(zMenu->cur->zWnd, NULL, FALSE);
			}
			break;

		case VK_LEFT:

			if(zMenu->cur)
			{
				if(zMenu->cur->parent && _zg_menuTop >= 0)
				{
					zMenu->cur = zMenu->child;
					zMenu->cur->entered = TRUE;
					InvalidateRect(zMenu->cur->zWnd, NULL, FALSE);
					_zg_menuWnd[_zg_menuTop] = NULL;
				}
				else
				{
				}
			}
			break;

		case VK_RIGHT:

			zItem = NULL;
			if(zMenu->cur)
			{
				if(zMenu->cur->opts & MF_POPUP)
				{
					if(zMenu->cur->child && ! zMenu->cur->child->zWnd)
						PostMessage(hWnd, WM_COMMAND, zMenu->cur->id, 0);
					else
						zItem = zMenu->cur->sibling;
				}
				else
				{
					if(zMenu->sibling)
					{
						zItem = zMenu->sibling;
					}
				}
			}
			else
			{
				zItem = zMenu->child;
			}
			if(zItem)
			{
				if(zMenu->cur)
				{
					InvalidateRect(zMenu->cur->zWnd, NULL, FALSE);
					zMenu->cur->entered = FALSE;
				}
				zMenu->cur = zItem;				
				if(zMenu->cur->opts & MF_POPUP)
				{
					PostMessage(zMenu->cur->zWnd, WM_LBUTTONDOWN, 0, 0);
				}
				else
				{
					zMenu->cur->entered = TRUE;
					InvalidateRect(zMenu->cur->zWnd, NULL, FALSE);
				}
			}			
			break;
			
		case VK_RETURN:
			
			if(zMenu->cur)
			{
				if(! (zMenu->cur->opts & MF_POPUP))
				{
					PostMessage(hWnd, WM_COMMAND, zMenu->cur->id, 0);
					zMenu->cur->pushed = zMenu->cur->entered = FALSE;
				}
			}
			break;
			
		case VK_ESCAPE:
			
			__endpopups(0);
			break;
			
		default:
			break;
		}
		break;
	case WM_SIZE:
		
		zMenu = (LPZMENU)GetWindowLong(hWnd, GWL_USERDATA);
		if(! zMenu) break;

		
		if(! (zMenu->opts & MF_POPUP))
		{		
			//GetWindowRect(hWnd, &rcm);
			GetClientRect(hWnd, &rcm);
			GetClientRect(GetParent(hWnd), &rc);
			if(GetWindowLong(GetParent(hWnd), GWL_STYLE) & WS_VSCROLL)
				rc.right += BSA_VS_WIDTH;
			MoveWindow(hWnd, rc.left, rc.top - (rcm.bottom - rcm.top),
					rc.right - rc.left,  (rcm.bottom - rcm.top), TRUE);
		}
		break;
	
	default:
		
		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	return 0;
}

//**************************************************************************
LRESULT WINAPI __wproc_MenuButton(
						    HWND hWnd,
						    UINT Msg,
						    WPARAM wParam,
						    LPARAM lParam
						  )
{
	PAINTSTRUCT ps;
	HDC		    hdc;
	LPZWND		zWnd;
	LPZMENU		zItem;
	DWORD		style;
	BOOL		intop;
	TCHAR		text[256];
	RECT		rc;
	int			enabled;
	TRACKMOUSEEVENT me;
	
	if(! (zWnd = Zwindow(hWnd))) return -1;
	zItem = (LPZMENU)GetWindowLong(hWnd, GWL_USERDATA);
	style = GetWindowLong(hWnd, GWL_STYLE);
	enabled = ! (style & WS_DISABLED);
	
	switch(Msg)
	{
	case WM_CREATE:
		
		SetWindowLong(hWnd, GWL_USERDATA,
				(LONG)((LPCREATESTRUCT)lParam)->lpCreateParams);
		break;
		
	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rc);
		GetWindowText(hWnd, text, 255);

		intop = FALSE;
		if(zItem && zItem->parent && zItem->parent->zWnd)
			if(GetParent(zItem->parent->zWnd))
				if(((LPZWND)GetParent(zItem->parent->zWnd))->menu)
					intop = TRUE;

		if(zItem)
		{
			if(zItem->opts & MF_SEPARATOR)
				zItem->pushed = zItem->entered = FALSE;

			if(zItem->parent && zItem->parent->cur)
			{
				if(zItem->parent->cur != zItem)
				{
					zItem->entered = FALSE;
					zItem->parent->cur->entered = TRUE;
				}
			}
			
			if(zItem->opts & MF_POPUP)
			{
				zItem->pushed = zItem->child && zItem->child->zWnd;
			}
			if(! intop)
			{
				if(zItem->entered && ! intop)
				{
					COLORREF bkg;
			
					bkg = GetSysColor(COLOR_ACTIVECAPTION);
					SetBkColor(hdc, bkg);
					SetTextColor(hdc, GetSysColor(COLOR_CAPTIONTEXT));
				}
			}
			__w_drawbutton(
						hdc,
						hWnd,
						style,
						text,
						(intop ? zItem->pushed : ((zItem->opts & MF_CHECKED) != 0)),
						FALSE
					);
		}
		if(zItem && ! intop)
		{
			HPEN hPen, hOld;
			RECT rcp;
			int x, y;
			
			if(zItem->opts & MF_SEPARATOR)
			{
				GetClientRect(GetParent(hWnd), &rcp);
				
				x = 4;
				y = (rc.bottom - rc.top) / 2;
				
				hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DSHADOW));
				hOld = SelectObject(hdc, hPen);
				MoveToEx(hdc, x, y, NULL);
				LineTo(hdc, rcp.right - rcp.left - 4, y);
				SelectObject(hdc, hOld);
				DeleteObject(hPen);
				y++;
				hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DHIGHLIGHT));
				hOld = SelectObject(hdc, hPen);
				MoveToEx(hdc, x, y, NULL);
				LineTo(hdc, rcp.right - rcp.left - 4, y);
				SelectObject(hdc, hOld);
				DeleteObject(hPen);
			}
			else if(zItem->opts & MF_POPUP)
			{
				x = rc.right - rc.left - BSA_POPUP_ARROW_WIDTH;
				y = (rc.bottom - rc.top) / 2;
				
				hPen = CreatePen(PS_SOLID, 0, 
						GetSysColor(zItem->entered ? COLOR_CAPTIONTEXT : COLOR_BTNTEXT));
				hOld = SelectObject(hdc, hPen);
				MoveToEx(hdc, x, y + 4, NULL);
				LineTo(hdc, x, y - 5);
				MoveToEx(hdc, ++x, y + 3, NULL);
				LineTo(hdc, x, y - 4);
				MoveToEx(hdc, ++x, y + 2, NULL);
				LineTo(hdc, x, y - 3);
				MoveToEx(hdc, ++x, y + 1, NULL);
				LineTo(hdc, x, y - 2);
				MoveToEx(hdc, ++x, y, NULL);
				LineTo(hdc, x, y - 1);
				SelectObject(hdc, hOld);
				DeleteObject(hPen);
			}
		}
		EndPaint(hWnd,  &ps);
		break;
	
	case WM_DESTROY:
	
		if(zItem)
		{
			zItem->zWnd = NULL;
			zItem->pushed = zItem->entered = FALSE;
			if(zItem->parent != NULL)
			{
				zItem->parent->pushed = FALSE;
				InvalidateRect(zItem->parent->zWnd, NULL, FALSE);
			}
		}
		break;
			
	case WM_LBUTTONDOWN:

		if(_zg_dialogTop < 0 && enabled && ! (zItem->opts & MF_SEPARATOR))
		{
			zItem->pushed = TRUE;
			// popup buttons work on down
			if(zItem->opts & MF_POPUP)
				PostMessage(GetParent(hWnd), WM_COMMAND, zItem->id, 0);
			if(zItem->parent)
				zItem->parent->cur = zItem;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
		
	case WM_LBUTTONUP:

		if(_zg_dialogTop < 0 && enabled && ! (zItem->opts & MF_POPUP))
		{
			if(zItem->pushed)
				PostMessage(GetParent(hWnd), WM_COMMAND, zItem->id, 0);
			else if(_zg_menuTop >= 0)
				_zg_menuWnd[_zg_menuTop] = NULL;
			zItem->pushed = zItem->entered = FALSE;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;

	case WM_MOUSEMOVE:
		
		if(enabled && ! zItem->entered)
		{
			me.dwFlags = TME_QUERY;
			me.hwndTrack = NULL;
			TrackMouseEvent(&me);
			
			if(! (me.dwFlags & TME_LEAVE))
			{
				me.dwFlags = TME_HOVER | TME_LEAVE;
				me.dwHoverTime = HOVER_DEFAULT;
				me.hwndTrack = hWnd;
				TrackMouseEvent(&me);
				zItem->entered = TRUE;
				if(zItem->parent)
				{
					if(zItem->parent->cur)
					{
						zItem->parent->cur->entered = FALSE;
						zItem->parent->cur->pushed = FALSE;
						InvalidateRect(zItem->parent->cur->zWnd, NULL, FALSE);
					}
					zItem->parent->cur = zItem;
				}
				InvalidateRect(hWnd, NULL, FALSE);
			}
			if(zItem && (_zg_menuTop >= 0))
			{
				// mouse over another menu item.  close any
				// popups that are at or below the same zItem
				// level as the menu item
				//
				if(_zg_menuTop > 0)
				{
					LPZMENU zBotItem = (LPZMENU)GetWindowLong(_zg_menuWnd[_zg_menuTop], GWL_USERDATA);
					
					if(zBotItem == zItem) break;
					
					while(zBotItem)
					{
						if(zBotItem->parent == zItem || zBotItem->parent == zItem->parent)
						{  					 
							_zg_menuWnd[_zg_menuTop] = NULL;
							break;
						}
						zBotItem = zBotItem->parent;
					}
				}
				if(zItem->opts & MF_POPUP)
				{
					if(zItem->child && ! zItem->child->zWnd)
					{
						// mouseover into another popup
						//
						PostMessage(GetParent(hWnd), WM_COMMAND, zItem->id, 0);
						return 0;
					}
				}
			}
		}
		break;

	case WM_MOUSELEAVE:

		zItem->entered = FALSE;
		zItem->pushed  = FALSE;
		me.dwFlags = TME_CANCEL | TME_HOVER | TME_LEAVE;
		me.hwndTrack = NULL;
		TrackMouseEvent(&me);
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	default:
		
		return __wproc_Button(hWnd, Msg, wParam, lParam);
	}
	return 0;
}


