//--------------------------------------------------------------------
//
// File: winapi.c
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------


#ifndef Windows
#include "winapix.h"

//#define GDI_LEAK_DETECT 1
#ifdef GDI_LEAK_DETECT
#define GDILEAK printf
#else
#define GDILEAK
#endif

//-----------------------------------------------------------------------------
#ifdef X11
Display*	_zg_display 	= NULL;
int 		_zg_screen;
Visual*		_zg_visual		= NULL;
#endif

char		_zg_bsaappname[MAX_PATH] = { 0 };
int			_zg_bsaappargc	 		= 0;
char*		_zg_bsaappargv[] 		= { NULL };

#ifdef X11
XVisualInfo _zg_vinfo;
#endif
int			_zg_compositing	= 0;
DWORD		_zg_alpha 		= 0xff;

#ifdef X11
Atom		wm_delete_window_atom;
Atom		wm_strin_zg_atom;
Atom		wm_length_atom;
#endif

int			_zg_defx 	= 50;
int			_zg_defy 	= 50;
int			_zg_defw 	= 400;
int			_zg_defh 	= 300;

LPTSTR		_zg_error;

LPZCOLOR 	_zg_colors		= NULL;
LPZMEMH	 	_zg_memhandles 	= NULL;

DWORD		_zg_White;
DWORD		_zg_Black;

ZCOLOR		_zg_zc_white	= { 255, 255, 255, 1, 0, NULL };
ZCOLOR		_zg_zc_black	= { 0, 	0,   0, 1, 0, NULL };

BOOL		_zg_preferGray  = FALSE;

HBRUSH		_zg_hbrWhite;
HBRUSH		_zg_hbrBlack;
HBRUSH		_zg_hbrBlue;
HBRUSH		_zg_hbrLightGray;
HBRUSH		_zg_hbrGray;
HBRUSH		_zg_hbrDarkGray;
HBRUSH		_zg_hbrVDarkGray;
HBRUSH		_zg_hbrWindowsGray;

HPEN		_zg_BlackPen	= NULL;
HPEN		_zg_WhitePen	= NULL;

HFONT		_zg_sysFont		= NULL;
HFONT		_zg_menuFont	= NULL;
HFONT       _zg_treeFont    = NULL;
HFONT       _zg_treeFontBold= NULL;

HCURSOR		_zg_hOldCursor	= NULL;

int 		_zg_exit 		= FALSE;
int			_zg_exitcode 	= 0;

LPSTR		_zg_xtext		= NULL;

LPZCLASS 	_zg_classes 	= _zg_builtin_classes;
LPZWND	 	_zg_windows 	= NULL;
LPZWND	 	_zg_zWndRoot 	= NULL;

LPZWND		_zg_zWndFocal	= NULL;
LPZGDIOBJ	_zg_zgdi_objects= NULL;

HWND		_zg_hwndMouse	= NULL;
DWORD	 	_zg_lasterror 	= 0;

TRACKMOUSEEVENT _zg_mouseTrack = { 0, 0, NULL, 0 };

HANDLE	 	_zg_clipboard 	= NULL;
HWND		_zg_clipowner	= NULL;
int		 	_zg_clipformat	= 0;

LPZTIMER	_zg_timers		= NULL;

struct ta_zg_zcaret
{
	HWND	owner;
	int		w;
	int		h;
	int		blink;
	time_t  nextblinksecs;
	WORD	nextblinkms;
	int		scnt;
	int		x;
	int		y;
	int		lx;
	int		ly;
	int		state;
	int		prepaintstate;
}
_zg_caret = { NULL, 0, 0, 500, 0, 0, 0, 0, 0, 0, 0 };

LPZWND _zg_lastFocus = NULL;

#ifdef Windows
#define  _PTC_  _T('\\')
#define  _PTS_  _T("\\")
#else
#define  _PTC_  _T('/')
#define  _PTS_  _T("/")
#endif

//**************************************************************************
TCHAR* CharToTChar(LPTSTR pRes, const char* pSrc)
{
	while(*pSrc) *pRes++ = (WCHAR)*pSrc++;
	*pRes++ = '\0';
	return pRes;
}

//**************************************************************************
char* TCharToChar(char* pRes, LPCTSTR pSrc)
{
	while(*pSrc) *pRes++ = (char)*pSrc++;
	*pRes = '\0';
	return pRes;
}

//-----------------------------------------------------------------------------

//**************************************************************************
DWORD _z_x_colorofbrush(LPZGDIOBJ zObj)
{
	if(! zObj || (zObj->tag != ZGDIO_TAG) || (zObj->type != gdiBrush))
		return _zg_Black;
	return ((LPZCOLOR)zObj->obj)->xcolor;
}

//**************************************************************************
COLORREF _z_rgb_colorofbrush(LPZGDIOBJ zObj)
{
	LPZCOLOR zcolor;

	if(! zObj || (zObj->tag != ZGDIO_TAG) || (zObj->type != gdiBrush))
		return _zg_Black;
	zcolor = ((LPZCOLOR)zObj->obj);
	return RGB(zcolor->r, zcolor->g, zcolor->b);
}

//**************************************************************************
BYTE _z_x_getbrushalpha(LPZGDIOBJ zObj)
{
	LPZCOLOR zcolor;
	BYTE olda;

	if(! zObj || (zObj->tag != ZGDIO_TAG) || (zObj->type != gdiBrush))
		return 0;
	zcolor = ((LPZCOLOR)zObj->obj);
	return zcolor->c;
}

//**************************************************************************
BYTE _z_x_setbrushalpha(LPZGDIOBJ zObj, BYTE alpha)
{
	LPZCOLOR zcolor;
	BYTE olda;

	if(! zObj || (zObj->tag != ZGDIO_TAG) || (zObj->type != gdiBrush))
		return 0;
	zcolor = ((LPZCOLOR)zObj->obj);
	olda = zcolor->c;
	zcolor->c = alpha;
	return olda;
}

//**************************************************************************
LPZCLASS _w_newZCLASS(CONST WNDCLASS *lpWndClass)
{
	static int _zg_classcount = 0;
	LPZCLASS class;

	class = (LPZCLASS)malloc(sizeof(ZCLASS));
	if(! class) return NULL;

	class->proc = lpWndClass->lpfnWndProc;
	class->name = (LPTSTR)malloc((_tcslen(lpWndClass->lpszClassName) + 1) * sizeof(TCHAR));
	_tcscpy(class->name, lpWndClass->lpszClassName);
	class->menu	= NULL;

	if(lpWndClass->lpszMenuName)
		class->menu = __FindResource(RT_Menu, lpWndClass->lpszMenuName, _zg_resources, _cb_zg_resources);

	class->hIcon   = lpWndClass->hIcon;
	class->hCursor = lpWndClass->hCursor;

	if(lpWndClass->hbrBackground)
	{
		if(lpWndClass->hbrBackground < (void*)100)
			class->hbrBkg = GetSysColorBrush((int)(intptr_t)lpWndClass->hbrBackground - 1);
		else
			class->hbrBkg = lpWndClass->hbrBackground;
	}
	class->ordinal	= _zg_classcount++;
	class->next 	= _zg_classes;
	_zg_classes   	= class;
	return class;
}

//**************************************************************************
void _w_deleteZCLASS(LPZCLASS class)
{
	if(class)
	{
		if(class->name)
			free((LPTSTR)class->name);
		free(class);
	}
}

//**************************************************************************
LPZCLASS Zclass(LPCTSTR classname)
{
	LPZCLASS class = _zg_classes;

	if(! classname) return NULL;
	for(class = _zg_classes; class; class = class->next)
		if(! _tcsicmp(classname, class->name))
			return class;
	return NULL;
}

//**************************************************************************
LPZWND _w_newZWND(LPCTSTR classname)
{
	LPZCLASS class = Zclass(classname);

	if(class)
	{
		LPZWND wind = (LPZWND)malloc(sizeof(ZWND));

		wind->xWnd  	= 0;
		wind->class 	= class;
		wind->menu		= NULL;
	#ifdef COCOA
		wind->name		= NULL;
	#endif
		wind->next  	= _zg_windows;
		wind->cnext		= NULL;
		wind->cprev		= NULL;
		wind->msgq  	= NULL;
		wind->ps.hdc 	= NULL;
	#ifdef COCOA
		wind->ps.fErase = 1;
	#else
		wind->ps.fErase = 0;
	#endif
		wind->ps.rcPaint.top 	= 0x7fff;
		wind->ps.rcPaint.bottom = 0;
		wind->ps.rcPaint.right 	= 0;
		wind->ps.rcPaint.left 	= 0x7fff;
		if(class->hbrBkg)
			wind->bkg = class->hbrBkg;
		else
			wind->bkg = GetSysColorBrush(COLOR_WINDOW);
		wind->key   = _zg_alpha << 24;
		wind->font 	= _zg_sysFont;
		wind->frg 	= _zg_hbrBlack; //GetSysColorBrush(COLOR_TEXT);
		_zg_windows	= wind;
		memset(wind->misc, 0, sizeof(wind->misc));
		SetWindowLong(wind, GWL_WNDPROC, (LONG)class->proc);
		return wind;
	}
	else
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return NULL;
	}
}

//**************************************************************************
void _w_deleteZWND(LPZWND zWnd)
{
	LPZWND dWnd;
	LPMSG  pmsg, pdel;

	if(! zWnd) return;

	// deref mouse
	if(zWnd == _zg_hwndMouse)
		_zg_hwndMouse = NULL;

	// delete msgq
	for(pmsg = zWnd->msgq; pmsg; )
	{
		pdel = pmsg;
		pmsg = pmsg->next;
		free(pdel);
	}
	zWnd->msgq = NULL;

	if(zWnd->ps.hdc)
	{
		ReleaseDC(zWnd, zWnd->ps.hdc);
		zWnd->ps.hdc = NULL;
	}
	// remove from global  list
	if(_zg_windows == zWnd)
	{
		_zg_windows = zWnd->next;
		dWnd = zWnd;
	}
	else
	{
		for(dWnd = _zg_windows; dWnd->next != zWnd; dWnd = dWnd->next)
			;
		if(dWnd) dWnd->next = zWnd->next;
	}
	// only free memory if actually was in global list
	// (protects against double deletion)
	if(dWnd)
	{
		// unlink as part of control chain in dialog
		if(zWnd->cprev)
			zWnd->cprev->cnext = zWnd->cnext;
		if(zWnd->cnext)
			zWnd->cnext->cprev = zWnd->cprev;
	#ifdef COCOA
		if(zWnd->name)
			free(zWnd->name);
	#endif
		free(zWnd);
	}
}

//**************************************************************************
LPZWND Zwindow(HWND hWnd)
{
	LPZWND wind = _zg_windows;

	while(wind)
	{
		if(wind == (LPZWND)hWnd) return wind;
		wind = wind->next;
	}
	return NULL;
}

//**************************************************************************
LPZWND ZXwindow(Window xWind)
{
	LPZWND wind = _zg_windows;

	while(wind)
	{
		if(wind->xWnd == xWind) return wind;
		wind = wind->next;
	}
	return NULL;
}

//**************************************************************************
LPZGC _w_newZGC(LPZWND zWnd, LPZDRVR zDrvr)
{
	LPZGC     zGC;
	LPZGDIOBJ zObj;

	// create context
	zGC = malloc(sizeof(ZGC));
	if(! zGC)  return NULL;

	if(! (zGC->zDrvr = zDrvr))
	{
		// create default screen driver
#ifdef X11
		zGC->zDrvr = _w_newX11DRVR(zGC, zWnd);
#endif
#ifdef COCOA
		zGC->zDrvr = _w_newCOCOADRVR(zGC, zWnd);
#endif
		if(! zGC->zDrvr) { free(zGC); return NULL; }
	}

	// default settings
	SetMapMode(zGC, MM_TEXT);
	zGC->x			= 0;
	zGC->y			= 0;
	zGC->xoff		= 0;
	zGC->yoff		= zWnd ? MENU_HEIGHT(zWnd) : 0;
	zGC->zWnd 		= zWnd;
	zGC->hBRUSH 	= (HBRUSH)GetStockObject(WHITE_BRUSH);
	zGC->hPEN 		= (HPEN)GetStockObject(BLACK_PEN);
	zGC->hFONT 		= (zWnd && zWnd->font) ? zWnd->font : _zg_sysFont;
	zGC->hBITMAP	= NULL;
	zGC->textfrg	= zWnd ? zWnd->frg : _zg_hbrBlack;
	zGC->textbkg	= zWnd ? zWnd->bkg : _zg_hbrWhite;

	zObj = (LPZGDIOBJ)zGC->hFONT;
	if(zObj && zObj->tag == ZGDIO_TAG && zObj->type == gdiFont)
		zGC->zDrvr->_setFont(zGC->zDrvr, zObj);
	return zGC;
}

//**************************************************************************
void _w_deleteZGC(LPZGC zGC)
{
	if(! zGC) return;
	if(zGC->zDrvr)
		zGC->zDrvr->_delete(zGC->zDrvr);
	zGC->zDrvr = NULL;
	free(zGC);
}

#ifdef GDI_LEAK_DETECT
static void __countzgdi()
{
	LPZGDIOBJ p = _zg_zgdi_objects;
	int i = 0;

	while(p) { p = p->next; i++; }
	printf("%d z\n", i);
}
#endif

//**************************************************************************
HPEN _w_newZPEN(int style, int width, LPZGDIOBJ zObj)
{
	LPZPEN pen = (LPZPEN)malloc(sizeof(ZPEN));

	if(pen)
	{
		pen->style = style;
		pen->width = width;
		pen->zObj  = zObj;
		return (HPEN)pen;
	}
	else
	{
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
}

//**************************************************************************
void _w_deleteZPEN(HPEN pen)
{
	LPZPEN zPen =  (LPZPEN)pen;

	if(! zPen) return;
	DeleteObject(zPen->zObj);
	free(zPen);
}

//**************************************************************************
LPZCOLOR _w_newZCOLOR(DWORD r, DWORD g, DWORD b)
{
	LPZCOLOR zcolor;
#ifdef X11
	extern LPBYTE	_zg_palette;
	extern int		_zg_palsize;

	XColor	 xcolor;
	Colormap map;
	Status   stat;
#endif

	if((zcolor = (LPZCOLOR)malloc(sizeof(ZCOLOR))) == NULL)
		return 0;

	zcolor->next = _zg_colors;
	_zg_colors = zcolor;

	zcolor->r = r;
	zcolor->g = g;
	zcolor->b = b;
	zcolor->c = 0xFF;

#ifdef X11
	xcolor.red   = r << 8;	// x11 colors
	xcolor.green = g << 8;	// are 0-65536
	xcolor.blue  = b << 8;	//

	map  = DefaultColormap(_zg_display, _zg_screen);
	stat = XAllocColor(_zg_display, map, &xcolor);

	if(stat)
	{
		//_tprintf(_T("Alloced color %d,%d,%d\n"), r, g, b);
		zcolor->xcolor = /*(0x80 << 24) |*/ xcolor.pixel;
		if(_zg_palette && /*xcolor.pixel >= 0 &&*/ xcolor.pixel < _zg_palsize)
		{
			int i = xcolor.pixel * 3;

			_zg_palette[i]   = r;
			_zg_palette[i+1] = g;
			_zg_palette[i+2] = b;
		}
	}
	else
	{
		//_tprintf(_T("match color %d,%d,%d\n"), r, g, b);
		zcolor->xcolor = _w_closestcolor(r, g, b, _zg_preferGray, map);
	}
#endif
	return zcolor;
}

//**************************************************************************
void _w_deleteZCOLOR(LPZCOLOR zcolor)
{
#ifdef X11
	Colormap map;

	if(! zcolor)  return;
	if(zcolor->c) return;

	if(zcolor == _zg_colors)
	{
		_zg_colors = zcolor->next;
	}
	else
	{
		LPZCOLOR zc;

		for(zc = _zg_colors; zc && zc->next && zc->next != zcolor;)
			zc = zc->next;
		if(zc) zc->next = zcolor->next;
	}
	map = DefaultColormap(_zg_display, _zg_screen);
	XFreeColors(_zg_display, map, (unsigned long*)&zcolor->xcolor, 1, 0);
#endif
	free(zcolor);
}

//**************************************************************************
BOOL _w_equalZCOLOR(LPZCOLOR zcolor, BYTE r, BYTE g, BYTE b)
{
	if(! zcolor) return FALSE;
	return (zcolor->r == r && zcolor->g == g && zcolor->b == b);
}

#ifdef GDI_LEAK_DETECT
void _w_countGDIOBJS(LPCTSTR where)
{
	static int nold = 0;
	int no;
	LPZGDIOBJ  zGDIO;
	for(zGDIO = _zg_zgdi_objects, no = 0; zGDIO; zGDIO = zGDIO->next)
		no++;
	_tprintf(_T("NG %d at %ls\n"), no, where);
}
#endif

//**************************************************************************
LPZGDIOBJ _w_newZGDIOBJ(_gdio_type type, void* obj)
{
	LPZGDIOBJ  zGDIO;

#ifdef GDI_LEAK_DETECT
	_w_countGDIOBJS(_T("new"));
#endif
	zGDIO = malloc(sizeof(ZGDIOBJ));
	if(! zGDIO) return NULL;

	zGDIO->refs = 1;
	zGDIO->next = _zg_zgdi_objects;
	zGDIO->tag	= ZGDIO_TAG;
	zGDIO->type = type;
	zGDIO->obj  = obj;
	_zg_zgdi_objects = zGDIO;
	return zGDIO;
}

//**************************************************************************
LPZGDIOBJ Zgdiobj(void* obj)
{
	LPZGDIOBJ  zGDIO;

	for(zGDIO = _zg_zgdi_objects; zGDIO; zGDIO = zGDIO->next)
		if(zGDIO == obj)
			return obj;
	return NULL;
}

//**************************************************************************
LPZGDIOBJ _w_refZGDIOBJ(LPZGDIOBJ zGDIO)
{
	if(! zGDIO) return NULL;
	if(zGDIO->tag != ZGDIO_TAG) return NULL;
	if(zGDIO->refs != ZGDIO_PERMANENT)
		zGDIO->refs++;
	return zGDIO;
}

//**************************************************************************
void _w_deleteZGDIOBJ(LPZGDIOBJ zGDIO)
{
	LPZGDIOBJ	obj;
	LPZFONT		font;

	if(! zGDIO) return;
	if(zGDIO->tag != ZGDIO_TAG) return;

	// gdi objects are reference counted
	if(zGDIO->refs == ZGDIO_PERMANENT) return;

	zGDIO->refs--;
	if(zGDIO->refs > 0) return;

	if(zGDIO == _zg_zgdi_objects)
	{
		_zg_zgdi_objects = _zg_zgdi_objects->next;
	}
	else
	{
		for(obj = _zg_zgdi_objects; obj && obj->next != zGDIO; obj = obj->next)
			;
		if(obj)
			obj->next = zGDIO->next;
	}
#ifdef GDI_LEAK_DETECT
	_w_countGDIOBJS(_T("del"));
#endif
	switch(zGDIO->type)
	{
	case gdiBrush:
		_w_deleteZCOLOR((LPZCOLOR)zGDIO->obj);
#ifdef GDI_LEAK_DETECT
		printf("del brush\n");
#endif
		break;
	case gdiPen:
		_w_deleteZPEN((LPZPEN)zGDIO->obj);
#ifdef GDI_LEAK_DETECT
		printf("del pen\n");
#endif
		break;
	case gdiFont:
		font = (LPZFONT)zGDIO->obj;
		if(font)
		{
			if(font->font)
            {
				#ifdef X11
                #ifdef XFT_FONT
				if(font->isxft)
	                XftFontClose(_zg_display, font->xftfont);
				else
                #endif
					XFreeFont(_zg_display, font->font);
				#endif
            }
			free(font);
		}
		break;
	case gdiBitmap:
		_w_deleteZBITMAP((LPZBITMAP)zGDIO->obj);
		break;
	default:
		break;
	}
	free(zGDIO);
}

//**************************************************************************
LPZTIMER _w_newZTIMER(HWND hWnd, UINT id, UINT elapse, TIMERPROC proc)
{
	LPZTIMER timer;
	struct timeval ftv;

	if(! (timer = (LPZTIMER)malloc(sizeof(ZTIMER))))
		return NULL;

	gettimeofday(&ftv, NULL);

	timer->firetime	   = ftv.tv_sec + elapse / 1000;
	timer->firemillitm = (ftv.tv_usec / 1000) + (elapse - ((elapse / 1000) * 1000));

	timer->tag		= ZTIMER_TAG;
	timer->hWnd 	= hWnd;
	timer->id		= id;
	timer->elapse	= elapse;
	timer->proc		= proc;

	timer->next		= _zg_timers;
	_zg_timers		= timer;

	return			timer;
}

//**************************************************************************
void _w_deleteZTIMER(LPZTIMER zTimer)
{
	if(_zg_timers == zTimer)
		_zg_timers = _zg_timers->next;
	else
	{
		LPZTIMER next = _zg_timers;
		while(next->next != zTimer)
			next = next->next;
		if(next) next->next = zTimer->next;
	}
	free(zTimer);
}

//-----------------------------------------------------------------------------

#if 0
//**************************************************************************
void __w_dumprect(LPCTSTR lpText, LPRECT lpRect)
{
	_tprintf(_T("%ls t=%d l=%d b=%d r=%d\n"), lpText,
		lpRect->top, lpRect->left, lpRect->bottom, lpRect->right);
}
#endif

//**************************************************************************
void __w_caret(int state)
{
	LPZWND zWnd;
	HDC hdc;
	RECT rc;

	zWnd = Zwindow(_zg_caret.owner);
	if(! zWnd) return;

	if(! ZXwindow(zWnd->xWnd))
	{
		if(_zg_caret.owner == zWnd)
			_zg_caret.owner = NULL;
		return;
	}

	if(! _zg_caret.owner || (_zg_caret.state == cstOFF && _zg_caret.scnt <= 0))
		return;

	//_tprintf(_T("state=%d cs=%d\n"), state, _zg_caret.state);

	if(state == _zg_caret.state)
		return;

	rc.left 	= _zg_caret.x;
	rc.top  	= _zg_caret.y;
	rc.right	= _zg_caret.x + _zg_caret.w;
	rc.bottom	= _zg_caret.y + _zg_caret.h;

	_zg_caret.state = state;

	#ifdef COCOA
	hdc = zWnd->ps.hdc;
	if(! hdc)
	{
		// cocoa needs to draw in the drawrect function, so
		// just invalidate the rect in the cursors owners window
		// which will redraw and call toggle, so for "on" here
		// really set "off" and reset the toggle time to it goes on
		//
		if (state == cstON)
		{
			_zg_caret.state = cstOFF;
			_zg_caret.nextblinksecs = 0;
			_zg_caret.nextblinkms   = 0;
			InvalidateRect(zWnd, &rc, FALSE);
		}
		return;
	}
	#endif

	_w_drawCaret(hdc, zWnd, &rc, (_zg_caret.state == cstON) ? 1 : 0);

	#if 0
		_tprintf(_T("%ls state to %ls at %d,%d\n"),
				state == cstTOGGLE ? _T("toggle") : _T("set"),
				_zg_caret.state == cstOFF ? _T("Off") : _T("On"),
					_zg_caret.x, _zg_caret.y);
	#endif
}

//**************************************************************************
void __w_toggle_caret()
{
	LPZWND zWnd;
	struct timeval ftv;

	zWnd = Zwindow(_zg_caret.owner);
	if(! zWnd) return;

	if(! ZXwindow(zWnd->xWnd))
	{
		if(_zg_caret.owner == zWnd)
			_zg_caret.owner = NULL;
		return;
	}

	if(! _zg_caret.owner || (_zg_caret.state == cstOFF && _zg_caret.scnt <= 0))
		return;

	gettimeofday(&ftv, NULL);

	if(ftv.tv_sec < _zg_caret.nextblinksecs)
		return;
	if(ftv.tv_sec == _zg_caret.nextblinksecs)
		if((ftv.tv_usec / 1000) < (_zg_caret.nextblinkms))
			return;
	/*
	_tprintf(_T("toggle now=%d.%d then=%d.%d\n"),
		_zg_caret.nextblinksecs, _zg_caret.nextblinkms, ftv.tv_sec, (ftv.tv_usec / 1000));
	*/
	gettimeofday(&ftv, NULL);
	_zg_caret.nextblinksecs = ftv.tv_sec;
	_zg_caret.nextblinkms   = (ftv.tv_usec / 1000) + _zg_caret.blink;

	while(_zg_caret.nextblinkms > 1000)
	{
		_zg_caret.nextblinkms -= 1000;
		_zg_caret.nextblinksecs++;
	}
	__w_caret((_zg_caret.state == cstON) ? cstOFF : cstON);
}

//**************************************************************************
int __w_timer_window(LPZTIMER* ppTimer)
{
	LPZTIMER 	   ztimer;
	struct timeval ftv;
	BOOL   	  	   gott = FALSE;
	time_t		   mins = 10000;
	time_t		   minm = 10000;
	time_t		   ts;
	time_t		   tm;

	*ppTimer = NULL;

	for(ztimer = _zg_timers; ztimer; ztimer = ztimer->next)
	{
		if(ztimer->firetime > 0 || ztimer->firemillitm  > 0)
		{
			if(! gott)
			{
				gettimeofday(&ftv, NULL);
				gott = TRUE;
			}
			ts = ztimer->firetime - ftv.tv_sec;
			tm = ztimer->firemillitm - (ftv.tv_usec / 1000);

			if(ts < 0 || (ts == 0 && tm < 0))
			{
				ztimer->firetime 	= ftv.tv_sec + ztimer->elapse / 1000;
				ztimer->firemillitm = (ftv.tv_usec / 1000) +
						(ztimer->elapse - ((ztimer->elapse / 1000) * 1000));
				ts = ztimer->firetime - ftv.tv_sec;
				tm = ztimer->firemillitm - (ftv.tv_usec / 1000);
				*ppTimer = ztimer;
				return 0;
			}
			if(ts < mins)
			{
				mins = ts;
				minm = tm;
			}
			else if(ts == mins)
			{
				if(tm < minm)
					minm = tm;
			}
		}
	}
	if(mins == 0)
		return minm;
	else if(ztimer)
		return ztimer->elapse;
	else
		return 1000;
}

//**************************************************************************
void _w_client_rect_offsets(HWND hWnd, LPRECT lpRect)
{
	DWORD  style;
	HWND   scrollbar;
	RECT   rcs;

	lpRect->left = lpRect->top = lpRect->right = lpRect->bottom = 0;

	style = GetWindowLong(hWnd, GWL_STYLE);

	if(style & WS_VSCROLL)
	{
		scrollbar = _w_getScrollBar(hWnd, SB_VERT);

		if(scrollbar)
		{
			GetClientRect(scrollbar, &rcs);
			lpRect->right  = (rcs.right - rcs.left);
		}
	}
	if(style & WS_HSCROLL)
	{
		scrollbar = _w_getScrollBar(hWnd, SB_HORZ);

		if(scrollbar)
		{
			GetClientRect(scrollbar, &rcs);
			lpRect->bottom  = (rcs.bottom - rcs.top);
		}
	}
	lpRect->top += MENU_HEIGHT(Zwindow(hWnd));
}

//**************************************************************************
BOOL WINAPI GetPeekMessage(
							LPMSG lpMsg,
							HWND  hWnd,
							UINT  wMsgFilterMin,
						    UINT  wMsgFilterMax,
							BOOL  wait,
							BOOL  remove
					   )
{
	LPZWND		zWnd;
	int			gotxevent;

	// wait for an event
	//
	do
	{
		int ztimerwait = 1000;
		int zcaretwait = _zg_caret.blink / 2;
		int nq;

		if(_zg_exit /*|| ! _zg_windows*/)
		{
			// shutdown, return false with a quit message
			lpMsg->message = WM_QUIT;
			lpMsg->wParam  = 0;
			lpMsg->lParam  = _zg_exitcode;
			return wait ? FALSE : TRUE;
		}
		lpMsg->hwnd 	= NULL;
		lpMsg->message 	= 0;
		lpMsg->time 	= 0;
		lpMsg->wParam   = 0;
		lpMsg->wParam   = 0;

		// look for any messages pending in any windows q
		// or any update regions valid.
		//
		if(! hWnd)
		{
			int nWin = 0;

			for(zWnd = _zg_windows; zWnd; zWnd = zWnd->next, nWin++)
				if(zWnd->msgq)
					break;
			if(! zWnd)
			{
				//printf("%d wind\n", nWin);
				for(zWnd = _zg_windows; zWnd; zWnd = zWnd->next)
				if(
					zWnd->ps.rcPaint.left < zWnd->ps.rcPaint.right &&
					zWnd->ps.rcPaint.top  < zWnd->ps.rcPaint.bottom
				)
				{
					//__w_dumprect(_T("GEN Paint"), &zWnd->ps.rcPaint);
					PostMessage(zWnd, WM_PAINT, 0, 0);
					break;
				}
			}
		}
		else
		{
			zWnd = Zwindow(hWnd);
		}
		// if anything in this window's Q, use it,
		//
		if(zWnd)
		{
			LPMSG	pmsg;

			if(zWnd->msgq)
			{
				pmsg = zWnd->msgq;

				if(remove)
					zWnd->msgq = zWnd->msgq->next;

				lpMsg->message = pmsg->message;
				lpMsg->wParam  = pmsg->wParam;
				lpMsg->lParam  = pmsg->lParam;
				lpMsg->hwnd    = pmsg->hwnd;
				if(remove)
					free(pmsg);
				return TRUE;
			}
		}
		if(_zg_timers)
		{
			LPZTIMER ztimer;

			ztimerwait = __w_timer_window(&ztimer);
			if(ztimerwait <= 0 && ztimer)
			{
				zWnd = Zwindow(ztimer->hWnd);
				if(zWnd)
				{
					if(ztimer->proc)
					{
						time_t now;

						time(&now);
						ztimer->proc(ztimer->hWnd, WM_TIMER, ztimer->id, now * 1000);
						return TRUE;
					}
					else
					{
						lpMsg->message = WM_TIMER;
						lpMsg->wParam  = ztimer->id;
						lpMsg->lParam  = (LPARAM)ztimer->proc;
						lpMsg->hwnd    = ztimer->hWnd;
						return TRUE;
					}
				}
			}
			if(ztimerwait < 10)
				ztimerwait = 10;
			if(ztimerwait > 500)
				ztimerwait = 500;
		}
		if(wait)
		{
			// run slice will check os and add msgs to the window's msg queue
			//
			nq = _w_runSlice(min(ztimerwait, zcaretwait));
			if(nq < 0)
			{
				_zg_exit = 1;
			}
		}
	}
	while(wait && ! _zg_exit);

	return wait ? (_zg_exit ? FALSE : TRUE) : FALSE;
}

//**************************************************************************
BOOL WINAPI GetMessage(
							LPMSG lpMsg,
							HWND  hWnd,
							UINT  wMsgFilterMin,
						    UINT  wMsgFilterMax
					   )
{
	return GetPeekMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, TRUE, TRUE);
}

//**************************************************************************
BOOL WINAPI PeekMessage(
							LPMSG lpMsg,
							HWND  hWnd,
							UINT  wMsgFilterMin,
						    UINT  wMsgFilterMax,
							UINT  wRemoveMsg
					   )
{
	return GetPeekMessage(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, FALSE,
		(wRemoveMsg & PM_REMOVE) ? TRUE : FALSE);
}

//**************************************************************************
BOOL WINAPI TranslateMessage(CONST MSG *lpMsg)
{
	BOOL down;

	if(! lpMsg) return FALSE;

	if(lpMsg->message != WM_KEYDOWN && lpMsg->message != WM_KEYUP)
	{
		LPZWND zMenuWnd;
		HMENU  hMenu;

		if(lpMsg->message != WM_SYSKEYDOWN)
			return FALSE;

		// if window, or ancestor window, has a menu, hit it
		//
		for(zMenuWnd = lpMsg->hwnd, hMenu = NULL; zMenuWnd; zMenuWnd = Zwindow(GetParent(zMenuWnd)))
			if((hMenu = GetMenu(zMenuWnd)) != NULL)
				break;
		if(hMenu)
			__w_menukey(hMenu, lpMsg->wParam);
		return FALSE;
	}
	down = lpMsg->message == WM_KEYDOWN;

	//_tprintf(_T("tm k=%08X %08X\n"), lpMsg->wParam, lpMsg->lParam);

	// only most of the VK codes get through, some we don't
	// translate.  Note that X11 has already applied "shift"
	// and "capslock" but not control, which is why
	// this function exits, and  is applied in the getmessage.
	// lParam bit 26 says that the code is really a VK not a regular key
	//
	if(! (lpMsg->lParam & 0x04000000))
	{
		if(down)
		{
			PostMessage(lpMsg->hwnd, WM_CHAR, lpMsg->wParam, lpMsg->lParam);
		}
		// need to "unshift" and "uncontrol" the character for the keyup/down
		// message since it expects raw unshifed keys
		//

		// converts CTRL-A through CTRL-Z to A-Z
		if(lpMsg->wParam >= 1 && lpMsg->wParam <= 0x1a)
			lpMsg->wParam += 'A' - 1;

		// converts 21-2F to 31-3F (bad probably, but out of VK range
		else if(lpMsg->wParam >= 0x21 && lpMsg->wParam <= 0x2F)
			lpMsg->wParam |= 0x10;

		// converts upper case letters to lower case
		else if(lpMsg->wParam >= 0x61 && lpMsg->wParam <= 0x7A)
			lpMsg->wParam &= ~0x20;

		// blows these away, what to map to?
		else if(lpMsg->wParam > 0x5A && lpMsg->wParam <= 0x60)
			lpMsg->wParam = 0x30;

		// blows these away, what to map to?
		else if(lpMsg->wParam > 0x7A && lpMsg->wParam <= 0x7F)
			lpMsg->wParam = 0x30;
		return TRUE;
	}

	switch(lpMsg->wParam)
	{
	case VK_LBUTTON: case VK_RBUTTON: case VK_CANCEL: case VK_MBUTTON:
		break;

	case VK_CLEAR:
		break;

	case VK_SHIFT:
	case VK_CONTROL:
	case VK_MENU:
	case VK_PAUSE:
		break;

	case VK_CAPITAL:
		break;

	case VK_KANA:
	case VK_JUNJA:
	case VK_FINAL:
	case VK_HANJA:
		break;

	case VK_CONVERT:
	case VK_NONCONVERT:
	case VK_ACCEPT:
	case VK_MODECHANGE:
		break;

	case VK_PRIOR:
	case VK_NEXT:
	case VK_END:
	case VK_HOME:
	case VK_LEFT:
	case VK_UP:
	case VK_RIGHT:
	case VK_DOWN:
	case VK_SELECT:
	case VK_PRINT:
	case VK_EXECUTE:
	case VK_SNAPSHOT:
	case VK_INSERT:
	case VK_DELETE:
	case VK_HELP:
		break;

	case VK_LWIN:
	case VK_RWIN:
	case VK_APPS:
		break;

	case VK_NUMPAD0: case VK_NUMPAD1: case VK_NUMPAD2:
	case VK_NUMPAD3: case VK_NUMPAD4: case VK_NUMPAD5:
	case VK_NUMPAD6: case VK_NUMPAD7: case VK_NUMPAD8:
	case VK_NUMPAD9:
		if(down)
			PostMessage(lpMsg->hwnd, WM_CHAR, '0' + (lpMsg->wParam - VK_NUMPAD0), lpMsg->lParam);
		break;

	case VK_MULTIPLY: case VK_ADD:
	case VK_SEPARATOR: case VK_SUBTRACT: case VK_DECIMAL:
	case VK_DIVIDE:
		if(down)
			PostMessage(lpMsg->hwnd, WM_CHAR, '0' + (lpMsg->wParam - VK_NUMPAD0), lpMsg->lParam);
		break;

	case VK_F1:	case VK_F2: case VK_F3:	case VK_F4:	case VK_F5:	case VK_F6:
	case VK_F7:	case VK_F8: case VK_F9:	case VK_F10: case VK_F11: case VK_F12:
	case VK_F13: case VK_F14: case VK_F15: case VK_F16: case VK_F17: case VK_F18:
	case VK_F19: case VK_F20: case VK_F21: case VK_F22: case VK_F23: case VK_F24:
		break;

	case VK_NUMLOCK:
	case VK_SCROLL:
		break;

	case VK_LSHIFT:
	case VK_RSHIFT:
	case VK_LCONTROL:
	case VK_RCONTROL:
	case VK_LMENU:
	case VK_RMENU:
		break;

	case VK_PROCESSKEY:
	case VK_ATTN:
	case VK_CRSEL:
	case VK_EXSEL:
	case VK_EREOF:
	case VK_PLAY:
	case VK_ZOOM:
	case VK_NONAME:
	case VK_PA1:
	case VK_OEM_CLEAR:
		break;

	case VK_RETURN:
	case VK_BACK:
	case VK_TAB:
	case VK_ESCAPE:
	case VK_SPACE:
	default:
		if(down)
		{
			PostMessage(lpMsg->hwnd, WM_CHAR, lpMsg->wParam, lpMsg->lParam);
		}
		return TRUE;
	}
	return FALSE;
}

//**************************************************************************
LONG WINAPI DispatchMessage(CONST MSG *lpMsg)
{
	LPZWND   wind;
	WNDPROC  proc;
	long	 rv;

	// find class from window in message
	//
	if(! lpMsg || ! lpMsg->hwnd)
		return -1;

	wind = Zwindow(lpMsg->hwnd);

	if(! wind)
		return -1;

	proc = (WNDPROC)GetWindowLong(wind, GWL_WNDPROC);

	if(! proc || ! wind || ! wind->class)
		return -1;

	#ifdef COCOA
	// all drawing is done directly in the drawRect handler of the view
	// this window is, since that's how cocoa likes it, so for paint messages
	// just cause a redrawm indirectly
	//
	if (lpMsg->message == WM_PAINT)
	{
		_w_redrawWindow(
							wind->xWnd,
							wind->ps.rcPaint.left, wind->ps.rcPaint.top,
							(wind->ps.rcPaint.right - wind->ps.rcPaint.left),
							(wind->ps.rcPaint.bottom - wind->ps.rcPaint.top),
							wind->ps.fErase
							);
		ValidateRect((HWND)wind, &wind->ps.rcPaint);
		return 0;
	}
	#endif

	rv = proc((HWND)wind, lpMsg->message, lpMsg->wParam, lpMsg->lParam);

	if(lpMsg->message == WM_DESTROY)
		_w_deleteZWND(wind);

	return rv;
}

//**************************************************************************
LRESULT WINAPI SendMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	LPZWND  zWnd = Zwindow(hWnd);
	MSG		msg;

	if(! zWnd) return FALSE;

	msg.hwnd 	= zWnd;
	msg.wParam 	= wParam;
	msg.lParam 	= lParam;
	msg.message = Msg;
	msg.next   	= NULL;
	return DispatchMessage(&msg);
}

//**************************************************************************
BOOL WINAPI PostMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	LPZWND zWnd = Zwindow(hWnd);
	LPMSG	msg = (LPMSG)malloc(sizeof(MSG));
	LPMSG	pmsg;

	if(! zWnd || ! msg) return FALSE;

	msg->hwnd 	= zWnd;
	msg->wParam = wParam;
	msg->lParam = lParam;
	msg->message= Msg;
	msg->next   = NULL;

	if(! (pmsg = zWnd->msgq))
	{
		zWnd->msgq = msg;
	}
	else
	{
		while(pmsg->next)
			pmsg = pmsg->next;
		pmsg->next = msg;
	}
	return TRUE;
}

//**************************************************************************
LRESULT WINAPI DefWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	LPZWND zWnd = Zwindow(hWnd);

	if(! zWnd) return 0;

	switch(Msg)
	{
	case WM_CREATE:
	case WM_DESTROY:
	case WM_MOVE:
	case WM_SIZE:
		break;
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		break;
	case WM_ENABLE:
		break;
	case WM_SETTEXT:
		return SetWindowText(hWnd, (LPTSTR)lParam);
	case WM_GETTEXT:
		return GetWindowText(hWnd, (LPTSTR)lParam, 256);
	case WM_GETTEXTLENGTH:
		return 0;
	case WM_PAINT:
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_QUIT:
		break;
	case WM_ERASEBKGND:
	case WM_SHOWWINDOW:
		break;
	case WM_SETFONT:
		if(zWnd->font)
			DeleteObject(zWnd->font);
		zWnd->font = _zg_sysFont;
		if(wParam != 0)
		{
			LPZGDIOBJ zObj = (LPZGDIOBJ)wParam;

			if(zObj->tag == ZGDIO_TAG && zObj->type == gdiFont)
				zWnd->font = _w_refZGDIOBJ((HGDIOBJ)wParam);
		}
		if(lParam != 0) InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_GETFONT:
		break;
	case WM_NOTIFY:
	case WM_GETICON:
	case WM_SETICON:
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_CHAR:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_SYSCHAR:
	case WM_INITDIALOG:
	case WM_COMMAND:
	case WM_SYSCOMMAND:
	case WM_TIMER:
	case WM_HSCROLL:
	case WM_VSCROLL:
	case WM_INITMENU:
	case WM_INITMENUPOPUP:
	case WM_MENUSELECT:
	case WM_MENUCHAR:
	case WM_ENTERIDLE:
	case WM_MOUSEMOVE:
	case WM_CUT:
	case WM_COPY:
	case WM_PASTE:
	case WM_CLEAR:
	case WM_UNDO:
	default:
		break;
	}
	return 0;
}

//**************************************************************************
void WINAPI    PostQuitMessage(int nExitCode)
{
	_zg_exit 		= TRUE;
	_zg_exitcode  = nExitCode;
}

//**************************************************************************
ATOM WINAPI    RegisterClass(CONST WNDCLASS *lpWndClass)
{
	LPZCLASS class;

	if((class = Zclass(lpWndClass->lpszClassName)) != NULL)
	{
		class->refs++;
		return (ATOM)class->ordinal;
	}
	class = _w_newZCLASS(lpWndClass);
	if(! class) return (ATOM)0;
	return (ATOM)class->ordinal;
}

//**************************************************************************
BOOL WINAPI   UnregisterClass(LPCTSTR lpClassName, HINSTANCE hInstance)
{
	LPZCLASS class;

	if(! (class = Zclass(lpClassName))) return FALSE;
	if(class->refs-- <= 0)
		_w_deleteZCLASS(class);
	return TRUE;
}


extern DWORD		_zg_eventmask;

//**************************************************************************
HWND WINAPI	CreateWindowEx(
								DWORD 	dwExStyle,
								LPCTSTR lpClassName,
								LPCTSTR lpWindowName,
								DWORD 	dwStyle,
								int 	X,
								int 	Y,
								int 	nWidth,
								int 	nHeight,
								HWND 	hWndParent,
								HMENU 	hMenu,
								HINSTANCE hInstance,
								void* lpParam
								)
{
	LPZCLASS			 zClass;
	LPZWND				 zWnd;
	LPZWND				 zWndParent;
	Window				 xWnd;
	Window				 xWndParent;
	char 				 aWindowName[256];
	CREATESTRUCT		 create;

	// dont create the first top-level window of an app off-screen
	// (except on macs where a left screen is "negative")
	//
	if (! hWndParent)
	{
	#ifndef COCOA
		if(X < 0 || Y < 0 || nWidth < 0 || nHeight < 0)
		{
			X = _zg_defx;
			Y = _zg_defy;
			if (nWidth < 0 || nHeight < 0)
			{
				nWidth = _zg_defw;
				nHeight = _zg_defh;
			}
		}
	#endif
	}
	zWnd = _w_newZWND(lpClassName);
	if(! zWnd) return NULL;

	zClass = Zclass(lpClassName);
	if(! zClass)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return NULL;
	}
	if(hWndParent)
	{
		zWndParent = Zwindow(hWndParent);
		xWndParent = zWndParent->xWnd;

		if(zWndParent)
			Y += MENU_HEIGHT(zWndParent);
	}
	else
	{
		zWndParent = NULL;
		#ifdef X11
		xWndParent = DefaultRootWindow(_zg_display);
		#else
		xWndParent = NULL;
		#endif
	}
	TCharToChar(aWindowName, lpWindowName);
	xWnd = _w_createWindow(aWindowName, dwStyle, zWnd->bkg, (zWnd->key >> 24), X, Y, nWidth, nHeight, xWndParent);
	if(xWnd == 0)
	{
		_w_deleteZWND(zWnd);
		PANIC("Null Window");
		return NULL;
	}
	zWnd->xWnd = xWnd;
	SetWindowText(zWnd, lpWindowName);
	//InvalidateRect(zWnd, NULL, TRUE);

	if(zClass && zClass->hIcon)
		_w_setIcon(zWnd, zClass->hIcon, lpWindowName);

	SetWindowLong(zWnd, GWL_STYLE, 		(LONG)dwStyle);
	SetWindowLong(zWnd, GWL_EXSTYLE, 	(LONG)dwExStyle);
	SetWindowLong(zWnd, GWL_HWNDPARENT, (LONG)hWndParent);
	SetWindowLong(zWnd, GWL_ID,			0);

	// create menu if class specifies a menu
	if(! hMenu && zClass->menu)
		hMenu = LoadMenuIndirect((LPMENUTEMPLATE)zClass->menu);
	if(hMenu)
		SetMenu((HWND)zWnd, hMenu);

	// set windows cursor if in class
	if(zClass && zClass->hCursor)
		__set_wnd_cursor(zWnd, zClass->hCursor);

	create.lpCreateParams = lpParam;
	create.hInstance	  = hInstance;
	create.hMenu		  = hMenu;
	create.hwndParent	  = hWndParent;
	create.cy 			  = nHeight;
	create.cx			  = nWidth;
	create.y			  = Y;
	create.x			  = X;
	create.style		  = dwStyle;
	create.lpszName		  = lpWindowName;
	create.lpszClass	  = lpClassName;
	create.dwExStyle	  = dwExStyle;

    SendMessage(zWnd, WM_CREATE, 0, (LPARAM)&create);

	// attach scrollbars
	if(dwStyle & WS_VSCROLL)
		__w_attach_scroller(zWnd, SB_VERT);
	if(dwStyle & WS_HSCROLL)
		__w_attach_scroller(zWnd, SB_HORZ);
	return (HWND)zWnd;
}

//**************************************************************************
BOOL WINAPI DestroyWindow(HWND hWnd)
{
	LPZWND zWnd = Zwindow(hWnd);
	DWORD dwStyle;
	HWND hwndScroll;

	if(! zWnd) return FALSE;

	dwStyle = GetWindowLong(zWnd, GWL_STYLE);

	if(_zg_mouseTrack.hwndTrack == hWnd)
	{
		_zg_mouseTrack.hwndTrack = NULL;
		_zg_mouseTrack.dwFlags = 0;
	}
	if(zWnd->menu && zWnd->menu->zWnd)
		PostMessage(zWnd->menu->zWnd, WM_CLOSE, 0, 0);

	if(dwStyle & WS_VSCROLL)
		if((hwndScroll = _w_getScrollBar(zWnd, SB_VERT)) != NULL)
			__w_detach_scroller(hwndScroll);
	if(dwStyle & WS_HSCROLL)
		if((hwndScroll = _w_getScrollBar(zWnd, SB_HORZ)) != NULL)
			__w_detach_scroller(hwndScroll);
	if(zWnd->xWnd)
		CloseWindow(zWnd);
	SendMessage(zWnd, WM_DESTROY, 0, 0);
	return TRUE;
}

//**************************************************************************
BOOL WINAPI ShowWindow(HWND hWnd, int nCmdShow)
{
	LPZWND zWnd = Zwindow(hWnd);

	if(! zWnd) return FALSE;

	_w_showWindow(zWnd->xWnd, (nCmdShow == SW_SHOW));
	return TRUE;
}

//**************************************************************************
BOOL WINAPI CloseWindow(HWND hWnd)
{
	LPZWND zWnd = Zwindow(hWnd);
	Window xWnd;

	if(! zWnd) return FALSE;
	if(hWnd == _zg_hwndMouse) ReleaseCapture();
    if(hWnd == GetFocus())
	{
		// setfocus might set this via messages so just null it for now
		_zg_lastFocus = NULL;
		SetFocus(GetParent(hWnd));
	}
	xWnd = zWnd->xWnd;
	#ifndef COCOA
	// x11 xwnd is already closed to get here?
	zWnd->xWnd = 0;
	#endif
	if (xWnd)
		_w_destroyWindow(xWnd);
	PostMessage(hWnd, WM_CLOSE, 0, 0);
	return TRUE;
}

//**************************************************************************
BOOL WINAPI MoveWindow(
						HWND hWnd,
						int X,
						int Y,
						int nWidth,
						int nHeight,
						BOOL bRepaint)
{
	LPZWND zWnd = Zwindow(hWnd);
	LPZWND zParent;

	if(! zWnd) return FALSE;

	zParent = Zwindow(GetParent(hWnd));
	if(zParent)
		Y += MENU_HEIGHT(zParent);
	//_tprintf(_T("Move %ls to %d,%d %d,%d\n"), zWnd->name, X, Y, nWidth, nHeight);
	_w_moveWindow(zWnd->xWnd, X, Y, nWidth, nHeight);
	if(bRepaint)
	{
		RECT rcWind;

		GetClientRect(hWnd, &rcWind);
		InvalidateRect(hWnd, &rcWind, TRUE);
	}
	return TRUE;
}

//**************************************************************************
BOOL SetLayeredWindowAttributes(HWND hWnd, COLORREF key, BYTE alpha, DWORD flags)
{
	LPZWND zWnd = Zwindow(hWnd);
	DWORD style;

	if(! zWnd) return FALSE;
	style = GetWindowLong(zWnd, GWL_EXSTYLE);
	if(! (style & WS_EX_LAYERED)) return FALSE;
	if (flags & LWA_ALPHA)
	{
		zWnd->key &= 0x00FFFFFF;
		zWnd->key |= ((DWORD)alpha << 24);
	}
	if (flags & LWA_COLORKEY)
	{
		zWnd->key &= 0xFF000000;
		zWnd->key |= key & 0x00FFFFFF;
	}
	#ifdef COCOA
	if(flags & LWA_ALPHA)
	{
		// need to set top level window to alpha
		//
		_w_setWindowAlpha(zWnd->xWnd, alpha);
	}
	#endif
	return TRUE;
}

//**************************************************************************
BOOL WINAPI OpenClipboard(HWND hWndNewOwner)
{
	_zg_clipowner = hWndNewOwner;
	return TRUE;
}

//**************************************************************************
BOOL WINAPI CloseClipboard(void)
{
	GlobalUnlock(_zg_clipboard);
	return TRUE;
}

//**************************************************************************
HANDLE WINAPI SetClipboardData(UINT uFormat, HANDLE hMem)
{
	LPZMEMH zMem;

	if(! (zMem = (LPZMEMH)hMem) || (zMem->tag != ZMEMH_TAG))
		return NULL;
	if(_zg_clipboard)
		EmptyClipboard();
	_zg_clipboard  = hMem;
	_zg_clipformat = uFormat;
	#ifdef X11
	XStoreBytes(_zg_display, (char*)zMem->data, zMem->len);
	#endif
	#ifdef COCOA
	_w_clipSetClipboardData((char*)zMem->data, zMem->len, uFormat);
	#endif
	_w_clipOwnClipboard(uFormat);
	return hMem;
}

//**************************************************************************
HANDLE WINAPI GetClipboardData(UINT uFormat)
{
	HANDLE hMem;
	char*  bytes;
	char*  nb;
	int	len;

	bytes = _w_clipGetClipboardData(&len, uFormat);
	if(bytes == NULL || len <= 0)
		return NULL;
	hMem = GlobalAlloc(0, len + 4);
	if(! hMem)
		return NULL;
	nb = GlobalLock(hMem);
	if(nb)
	{
		memcpy(nb, bytes, len);
		nb[len] = 0;
		nb[len+1] = 0;
		nb[len+2] = 0;
		nb[len+3] = 0;
	}
	GlobalUnlock(hMem);
	return hMem;
}

//**************************************************************************
BOOL WINAPI EmptyClipboard(void)
{
	GlobalUnlock(_zg_clipboard);
	GlobalFree(_zg_clipboard);
	_zg_clipboard = NULL;
	_w_clipDisownClipboard();
	return TRUE;
}

//**************************************************************************
HWND WINAPI SetFocus(HWND hWnd)
{
	LPZWND zWnd = Zwindow(hWnd);
	LPZWND rv   = Zwindow(GetFocus());
	LPZWND cv;

	if(_zg_dialogTop >= 0)
	{
		// this makes sure that only children
		// of the top dialog can set focus
		//
		LPZWND dw = _zg_dialogWnd[_zg_dialogTop];

		for(cv = zWnd; cv; cv = Zwindow(GetParent(cv)))
			if(cv == dw)
				break;
		if(! cv)
			return rv;
	}
	if(_zg_menuTop >= 0 && ! GetCapture())
	{
		if(! __menu_from_popup(hWnd))
			__endpopups(0);
	}
	if(! zWnd)
		return rv;
	if(zWnd == _zg_lastFocus)
		return rv;
	//_tprintf(_T("set=%ls %p\n"), zWnd->class->name, zWnd);

	if(zWnd != _zg_lastFocus)
	{
		// set this in os level.  For X11, that will generate its own
		// focus out and focus in events which we have to handle for top-level
		// windows so we do the message sending there instead of here
		//
		//_tprintf(_T("xsf %p %ls\n"), zWnd, zWnd->class->name);

		_w_setFocus(zWnd->xWnd);

		// grabbing the keyboard will generate
		// a focusin event in x11, which will send
		// the messages so don't do it here
		#ifndef X11
		if(_zg_lastFocus)
			SendMessage((HWND)_zg_lastFocus, WM_KILLFOCUS, 0, 0);
		_zg_lastFocus = zWnd;
		SendMessage((HWND)zWnd, WM_SETFOCUS, 0, 0);
		#endif
	}
	return rv;
}

//**************************************************************************
HWND WINAPI GetActiveWindow(void)
{
	return NULL;
}

//**************************************************************************
HWND WINAPI GetFocus(void)
{
	return ZXwindow(_w_getFocus());
}

//**************************************************************************
HWND WINAPI GetCapture(void)
{
	return _zg_hwndMouse;
}

//**************************************************************************
HWND WINAPI SetCapture(HWND hWnd)
{
//	LPZWND cw   = Zwindow(hWnd);

	_zg_hwndMouse = hWnd;
#if 0
	if(cw)
	{
		int stat = XGrabPointer(
					_zg_display,
					cw->xWnd,
					False,
					_zg_eventmask,
					GrabModeAsync,
					GrabModeAsync,
					None,
					None,
					0
				   );
		int a = stat;
	}
#endif
	return NULL;
}
//**************************************************************************
BOOL WINAPI ReleaseCapture(void)
{
	_zg_hwndMouse = NULL;
#if 0
	XUngrabPointer(_zg_display, 0);
#endif
	return TRUE;
}

//**************************************************************************
BOOL WINAPI TrackMouseEvent(LPTRACKMOUSEEVENT pe)
{
	if(pe && Zwindow(pe->hwndTrack))
	{
		if(pe->dwFlags & TME_QUERY)
		{
			pe->hwndTrack = _zg_mouseTrack.hwndTrack;
			if(_zg_mouseTrack.hwndTrack != NULL)
			{
				pe->dwFlags = _zg_mouseTrack.dwFlags;
				pe->dwHoverTime = _zg_mouseTrack.dwHoverTime;
			}
			else
			{
				pe->dwFlags = 0;
				pe->dwHoverTime = 0;
			}
		}
		else if(pe->dwFlags & TME_CANCEL)
		{
			if(pe->dwFlags & TME_HOVER)
				_zg_mouseTrack.dwFlags &= ~ TME_HOVER;
			if(pe->dwFlags & TME_LEAVE)
				_zg_mouseTrack.dwFlags &= ~ TME_LEAVE;
		}
		else
		{
			_zg_mouseTrack.dwFlags   = pe->dwFlags;
			_zg_mouseTrack.hwndTrack = pe->hwndTrack;
			if(pe->dwFlags & TME_HOVER)
				if(pe->dwHoverTime == HOVER_DEFAULT)
					_zg_mouseTrack.dwHoverTime = BSA_MOUSE_HOVER_MS;
				else
					_zg_mouseTrack.dwHoverTime = pe->dwHoverTime;
			else
				_zg_mouseTrack.dwHoverTime = 0;
		}
	}
	return FALSE;
}

//**************************************************************************
BOOL WINAPI IsWindowEnabled(HWND hWnd)
{
	LPZWND zWnd;
	long   style;

	zWnd = Zwindow(hWnd);
	if(! zWnd) return FALSE;
	style = GetWindowLong(zWnd, GWL_STYLE);
	return (style & WS_DISABLED) ? FALSE : TRUE;
}

//**************************************************************************
BOOL WINAPI EnableWindow(HWND hWnd, BOOL bEnable)
{
	LPZWND zWnd;
	long   style, prevstyle;

	zWnd = Zwindow(hWnd);
	if(! zWnd) return FALSE;
	style = prevstyle = (long)GetWindowLong(zWnd, GWL_STYLE);
	style &= ~WS_DISABLED;
	if(! bEnable)
		style |= WS_DISABLED;
	SetWindowLong(zWnd, GWL_STYLE, (LONG)style);
	if(style != prevstyle)
		InvalidateRect(hWnd, NULL, TRUE);
	return TRUE;
}

//**************************************************************************
BOOL WINAPI UpdateWindow(HWND hWnd)
{
	PostMessage(hWnd, WM_PAINT, 0, 0);
	return TRUE;
}

//**************************************************************************
HDC WINAPI GetDC(HWND hWnd)
{
	LPZWND zWnd;
	LPZGC  zGC;

    if(hWnd == NULL)
    {
        // special case "root device dc"
        zGC = _w_newZGC(NULL, NULL);
    }
    else
    {
    	zWnd = Zwindow(hWnd);
    	if(! zWnd) return NULL;

    	zGC = _w_newZGC(zWnd, NULL);
    }
	return zGC;
}

//**************************************************************************
HDC WINAPI CreateDC(LPCTSTR lpszDriver, LPCTSTR lpszDevice, LPCTSTR lpszOut, CONST DEVMODE* devMode)
{
	LPZDRVR zDrvr = NULL;
	LPZGC   zGC;

	if(lpszDriver)
	{
		if(! _tcscmp(lpszDriver, _T("PostScript")))
			zDrvr = _w_newPSDRVR(NULL, NULL);
		else if(! _tcscmp(lpszDriver, _T("PCL")))
			zDrvr = _w_newPCLDRVR(NULL, NULL);
		else
			zDrvr = NULL;
	}
	if(! zDrvr) return NULL;
	zGC = _w_newZGC(NULL, zDrvr);
	zDrvr->zGC = zGC;
	return (HDC)zGC;
}

//**************************************************************************
HDC WINAPI CreateCompatibleDC(HDC hdc)
{
	LPZGC   zGC;

	if(! (zGC = (LPZGC)hdc))	return NULL;

	zGC = _w_newZGC(NULL, zGC->zDrvr);
	zGC->zDrvr = NULL;
	return (HDC)zGC;
}

//**************************************************************************
int WINAPI ReleaseDC(HWND hWnd, HDC hDC)
{
	LPZWND zWnd;
	LPZGC  zGC;

	zWnd = Zwindow(hWnd);
	if(! zWnd) return 0;

	if (! hDC)
	{
		zGC = (LPZGC)zWnd->ps.hdc;
		if(! zGC)
			return 0;
	}
	else
	{
		zGC = (LPZGC)hDC;
	}
	_w_deleteZGC(zGC);
	if (zGC && zGC == (LPZGC)zWnd->ps.hdc)
		zWnd->ps.hdc = NULL;
	return 1;
}

//**************************************************************************
BOOL WINAPI DeleteDC(HDC hDC)
{
	LPZGC zGC;

	zGC = (LPZGC)hDC;
	if(! zGC) return FALSE;

	_w_deleteZGC(zGC);
	return TRUE;
}

//**************************************************************************
int WINAPI GetDeviceCaps(HDC hDC, int cap)
{
	LPZGC zGC;

	if(! (zGC = (LPZGC)hDC)) return FALSE;
	if(! zGC->zDrvr) 		 return FALSE;
	if(zGC->zDrvr->_devCaps)
		return zGC->zDrvr->_devCaps(zGC->zDrvr, cap);
	return 0;
}

//**************************************************************************
int WINAPI StartDoc(HDC hDC, CONST DOCINFO* pInfo)
{
	LPZGC zGC;
	static int _zg_printJob = 1;

	if(! (zGC = (LPZGC)hDC)) return FALSE;
	if(! zGC->zDrvr) 		 return FALSE;
	if(zGC->zDrvr->_startDoc(zGC->zDrvr, pInfo))
		return _zg_printJob++;
	else
		return 0;
}

//**************************************************************************
int WINAPI EndDoc(HDC hDC)
{
	LPZGC zGC;

	if(! (zGC = (LPZGC)hDC)) return FALSE;
	if(! zGC->zDrvr) 		 return FALSE;
	return (int)zGC->zDrvr->_endDoc(zGC->zDrvr);
}

//**************************************************************************
int WINAPI StartPage(HDC hDC)
{
	LPZGC zGC;

	if(! (zGC = (LPZGC)hDC)) return FALSE;
	if(! zGC->zDrvr) 		 return FALSE;
	return (int)zGC->zDrvr->_startPage(zGC->zDrvr);
}

//**************************************************************************
int WINAPI EndPage(HDC hDC)
{
	LPZGC zGC;

	if(! (zGC = (LPZGC)hDC)) return FALSE;
	if(! zGC->zDrvr) 		 return FALSE;
	return (int)zGC->zDrvr->_endPage(zGC->zDrvr);
}

//**************************************************************************
HDC WINAPI BeginPaint(HWND hWnd, LPPAINTSTRUCT lpPaint)
{
	LPZWND zWnd;

	#ifdef X11
	if(_zg_caret.owner == hWnd)
	{
		_zg_caret.prepaintstate = _zg_caret.state;
		__w_caret(cstOFF);
	}
	#endif
	zWnd = Zwindow(hWnd);
	if(! zWnd) return NULL;

	zWnd->ps.hdc = GetDC(hWnd);
	*lpPaint = zWnd->ps;

	#ifdef COCOA
	if(_zg_caret.owner == hWnd)
	{
		// cocoa needs to draw cursor with same DC as
		// the window underneath, so draw it off now
		_zg_caret.prepaintstate = _zg_caret.state;
		__w_caret(cstOFF);
	}
	#endif
	if(lpPaint->fErase && zWnd->class && zWnd->class->hbrBkg)
	{
		lpPaint->fErase = FALSE;
		FillRect(zWnd->ps.hdc, &zWnd->ps.rcPaint, zWnd->class->hbrBkg);
	}
	return zWnd->ps.hdc;
}

//**************************************************************************
BOOL WINAPI EndPaint(HWND hWnd, CONST PAINTSTRUCT *lpPaint)
{
	LPZWND zWnd;

	zWnd = Zwindow(hWnd);
	if(! zWnd) return FALSE;
	//__w_dumprect(_T("Valid"), &zWnd->ps.rcPaint);
	ValidateRect(hWnd, &zWnd->ps.rcPaint);
	zWnd->ps.fErase = FALSE;

	#ifdef COCOA
	if(_zg_caret.owner == hWnd)
	{
		// restore caret state inside this gc
		__w_caret(_zg_caret.prepaintstate);
		// and toggle it if needed now
		__w_toggle_caret();
	}
	#endif
	ReleaseDC(hWnd, zWnd->ps.hdc);
	#ifdef X11
	if(_zg_caret.owner == hWnd)
		__w_caret(_zg_caret.prepaintstate);
	#endif
	return TRUE;
}

//**************************************************************************
BOOL WINAPI GetUpdateRect(HWND hWnd, LPRECT lpRect, BOOL bErase)
{
	LPZWND zWnd;

	zWnd = Zwindow(hWnd);
	if(! zWnd || ! lpRect) return FALSE;
	lpRect->top	= zWnd->ps.rcPaint.top;
	lpRect->left   = zWnd->ps.rcPaint.left;
	lpRect->bottom = zWnd->ps.rcPaint.bottom;
	lpRect->right  = zWnd->ps.rcPaint.right;
	if(bErase) zWnd->ps.fErase = TRUE;
	return TRUE;
}

//**************************************************************************
BOOL WINAPI InvalidateRect(HWND hWnd, CONST RECT *lpRect, BOOL bErase)
{
	LPZWND  zWnd;
	RECT	rc;

	zWnd = Zwindow(hWnd);
	if(! zWnd) return FALSE;

	// add rect to update region
	if(!lpRect)
	{
		GetClientRect(hWnd, &rc);
		lpRect = &rc;
	}
	UnionRect(&zWnd->ps.rcPaint, &zWnd->ps.rcPaint, lpRect);
	zWnd->ps.fErase |= (bErase ? TRUE : FALSE);
	//__w_dumprect(_T("Inval"), &zWnd->ps.rcPaint);
	return TRUE;
}

//**************************************************************************
BOOL WINAPI UnionRect(LPRECT lpDest, CONST RECT* lpA, CONST RECT* lpB)
{
	if (lpA->top < lpB->top)
		lpDest->top = lpA->top;
	else
		lpDest->top = lpB->top;
	if (lpA->left < lpB->left)
		lpDest->left = lpA->left;
	else
		lpDest->left = lpB->left;
	if (lpA->bottom > lpB->bottom)
		lpDest->bottom = lpA->bottom;
	else
		lpDest->bottom = lpB->bottom;
	if (lpA->right > lpB->right)
		lpDest->right = lpA->right;
	else
		lpDest->right = lpB->right;
	return lpDest->top != lpDest->bottom || lpDest->left != lpDest->right;
}

//**************************************************************************
BOOL WINAPI IntersectRect(LPRECT lpDest, CONST RECT* lpA, CONST RECT* lpB)
{
	if (lpA->top > lpB->top)
		lpDest->top = lpA->top;
	else
		lpDest->top = lpB->top;
	if (lpA->left > lpB->left)
		lpDest->left = lpA->left;
	else
		lpDest->left = lpB->left;
	if (lpA->bottom < lpB->bottom)
		lpDest->bottom = lpA->bottom;
	else
		lpDest->bottom = lpB->bottom;
	if (lpA->right < lpB->right)
		lpDest->right = lpA->right;
	else
		lpDest->right = lpB->right;
	if(lpDest->bottom < lpDest->top)
		lpDest->bottom = lpDest->top;
	if(lpDest->right < lpDest->left)
		lpDest->right = lpDest->left;
	return lpDest->top < lpDest->bottom && lpDest->left < lpDest->right;
}

//**************************************************************************
BOOL WINAPI ValidateRect(HWND hWnd, CONST RECT *lpRect)
{
	LPZWND zWnd;

	zWnd = Zwindow(hWnd);
	if(! zWnd) return FALSE;

	// remove rect from update region unless the current
	// region isn't eqaul
	//
	if(
			zWnd->ps.rcPaint.top 	 == lpRect->top 	&&
			zWnd->ps.rcPaint.bottom  == lpRect->bottom	&&
			zWnd->ps.rcPaint.left	 == lpRect->left	&&
			zWnd->ps.rcPaint.right	 == lpRect->right
	)
	{
		zWnd->ps.rcPaint.top = zWnd->ps.rcPaint.left = 32767;
		zWnd->ps.rcPaint.bottom = zWnd->ps.rcPaint.right = 0;
	}
	return TRUE;
}

//**************************************************************************
int WINAPI GetWindowText(
					    HWND hWnd,
					    LPTSTR lpString,
					    int nMaxCount
					    )
{
	LPZWND  zWnd;
	int		len;

	if(! (zWnd = Zwindow(hWnd)) || ! lpString || nMaxCount <= 0)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
#ifdef X11
	XFetchName(_zg_display, zWnd->xWnd, &_zg_xtext);
	if(_zg_xtext)
	{
		len = strlen(_zg_xtext);
		if(len > nMaxCount)
		{
			len = nMaxCount;
			_zg_xtext[nMaxCount - 1] = 0;
		}
#ifdef UNICODE
		CharToTChar(lpString, _zg_xtext);
#else
		strcpy(lpString, _zg_xtext);
#endif
		if(_zg_xtext)	XFree(_zg_xtext);
	}
	else
	{
		len = 0;
		lpString[0] = 0;
	}
#endif
#ifdef COCOA
	if (zWnd->name)
	{
		_tcsncpy(lpString, zWnd->name, nMaxCount - 1);
		lpString[nMaxCount - 1] = _T('\0');
	}
	else
	{
		lpString[0] = _T('\0');
	}
#endif
	return len;
}

//**************************************************************************
int WINAPI SetWindowText(
						HWND hWnd,
						LPCTSTR lpString
						)
{
	LPZWND 	  zWnd;

	if(! (zWnd = Zwindow(hWnd)) || ! lpString)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
#ifdef X11
#ifdef UNICODE
	{
		char* strA = (char*)malloc(_tcslen(lpString) * 3 + 2);
		if(! strA)
		{
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return FALSE;
		}
		TCharToChar(strA, lpString);
		XStoreName(_zg_display, zWnd->xWnd, strA);
		free(strA);
	}
#else
	XStoreName(_zg_display, zWnd->xWnd, lpString);
#endif
#endif
#ifdef COCOA
	{
		DWORD dwStyle = GetWindowLong(zWnd, GWL_STYLE);
		int noframe = (! (dwStyle & (WS_SYSMENU | WS_CAPTION))) || (dwStyle & WS_CHILD);

		if(zWnd->name)
			free(zWnd->name);
		zWnd->name = (LPTSTR)malloc((_tcslen(lpString) + 1) * sizeof(TCHAR));
		if(zWnd->name)
			_tcscpy(zWnd->name, lpString);
		if(! noframe)
		{
			#ifdef UNICODE
			char* strA = (char*)malloc(_tcslen(lpString) * 3 + 2);
			if(! strA)
			{
				SetLastError(ERROR_NOT_ENOUGH_MEMORY);
				return FALSE;
			}
			TCharToChar(strA, lpString);
			_w_setTitle(zWnd->xWnd, strA);
			free(strA);
			#else
			_w_setTitle(zWnd->xWnd, zWnd->name);
			#endif
		}
	}
#endif
	InvalidateRect(zWnd, NULL, FALSE);
	return TRUE;
}

//**************************************************************************
BOOL WINAPI GetClientRect(HWND hWnd, LPRECT lpRect)
{
	LPZWND 	   		zWnd;
	RECT   			rcc;

	if(lpRect)
		lpRect->top = lpRect->left = lpRect->bottom = lpRect->right = 0;

	if(! (zWnd = Zwindow(hWnd)) || ! lpRect)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	_w_client_rect_offsets(hWnd, &rcc);
	_w_clientRect(zWnd->xWnd, lpRect);
	lpRect->right -= (rcc.left + rcc.right);
	lpRect->bottom -= (rcc.top + rcc.bottom);
//	_tprintf(_T("ClientRect %ls %d,%d  %d,%d\n"), zWnd->name,
//			lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
	return TRUE;
}


//**************************************************************************
HWND WINAPI GetDesktopWindow()
{
	return (HWND)_zg_zWndRoot;
}

//**************************************************************************
BOOL WINAPI GetWindowRect(HWND hWnd, LPRECT lpRect)
{
	LPZWND 	zWnd;
	unsigned int myBorderWidth, myDepth;

	if(lpRect)
		lpRect->top = lpRect->left = lpRect->bottom = lpRect->right = 0;

	if(! (zWnd = Zwindow(hWnd)) || ! lpRect)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	_w_windowRect(zWnd->xWnd, NULL, lpRect, &myBorderWidth, &myDepth);
	return TRUE;
}

//**************************************************************************
COLORREF WINAPI GetSysColor(int nIndex)
{
	//COLORREF color;
	HBRUSH	 hbr;

	switch(nIndex)
	{
	case COLOR_BACKGROUND:
		hbr = _zg_hbrWhite;
		break;
	case COLOR_ACTIVECAPTION:
		hbr = _zg_hbrBlue;
		break;
	case COLOR_INACTIVECAPTION:
		hbr = _zg_hbrGray;
		break;
	case COLOR_MENU:
		hbr = _zg_hbrWhite;
		break;
	case COLOR_WINDOW:
		hbr = _zg_hbrWhite;
		break;
	case COLOR_MENUTEXT:
	case COLOR_WINDOWFRAME:
	case COLOR_WINDOWTEXT:
	case COLOR_BTNTEXT:
	case COLOR_INFOTEXT:
	default:
		hbr = _zg_hbrBlack;
		break;
	case COLOR_CAPTIONTEXT:
		hbr = _zg_hbrWhite;
		break;
	case COLOR_ACTIVEBORDER:
	case COLOR_INACTIVEBORDER:
	case COLOR_APPWORKSPACE:
	case COLOR_BTNHIGHLIGHT:
	case COLOR_HIGHLIGHT:
	case COLOR_INFOBK:
		hbr = _zg_hbrWhite;
		break;
	case COLOR_BTNFACE:
		//hbr = _zg_hbrWindowsGray;
		hbr = _zg_hbrLightGray;
		break;
	case COLOR_GRAYTEXT:
	case COLOR_INACTIVECAPTIONTEXT:
		hbr = _zg_hbrGray;
		break;
	case COLOR_BTNSHADOW:
		hbr = _zg_hbrDarkGray;
		break;
	case COLOR_3DDKSHADOW:
		hbr = _zg_hbrVDarkGray;
		break;
	case COLOR_SCROLLBAR:
	case COLOR_HIGHLIGHTTEXT:
	case COLOR_3DLIGHT:
		hbr = _zg_hbrLightGray;
		break;
	}
	return _z_rgb_colorofbrush(hbr);
}

//**************************************************************************
HBRUSH __w_solidbrush(DWORD r, DWORD g, DWORD b)
{
	LPZGDIOBJ zObj;

	// if brush had been made, get reference, else make it
	//
	for(zObj = _zg_zgdi_objects; zObj; zObj = zObj->next)
		if(zObj->type == gdiBrush)
			if(_w_equalZCOLOR((LPZCOLOR)zObj->obj, r, g, b))
				return (HBRUSH)_w_refZGDIOBJ(zObj);

	// not made yet, so make it
	//
	zObj = _w_newZGDIOBJ(gdiBrush, (void*)_w_newZCOLOR(r, g, b));
#ifdef GDI_LEAK_DETECT
	printf("new brush\n");
#endif
	return (HBRUSH)zObj;
}

//**************************************************************************
HBRUSH WINAPI CreateSolidBrush(COLORREF color)
{
	DWORD r,g,b;

	r = color & 0xff;
	g = (color >> 8) & 0xff;
	b = (color >> 16) & 0xff;

	return __w_solidbrush(r, g, b);
}

//**************************************************************************
HBRUSH WINAPI CreatePen(int style, int width, COLORREF color)
{
	LPZGDIOBJ zObj;
	LPZPEN    zPen;

	zObj = (LPZGDIOBJ)CreateSolidBrush(color);
	if(! zObj) return NULL;

	zPen = _w_newZPEN(style, width, zObj);
	if(! zPen) return NULL;

#ifdef GDI_LEAK_DETECT
	printf("new pen\n");
#endif
	return (HPEN)_w_newZGDIOBJ(gdiPen, zPen);
}

//**************************************************************************
COLORREF WINAPI SetTextColor(HDC hdc, COLORREF color)
{
	LPZGDIOBJ zObj;
	LPZGC	  zGC = (LPZGC)hdc;

	if(! zGC) return 0;

	zObj = CreateSolidBrush(color);
	if(zObj)
	{
		zGC->textfrg = (HBRUSH)zObj;
		return _z_rgb_colorofbrush((HBRUSH)zObj);
	}
	return 0;
}

//**************************************************************************
COLORREF WINAPI SetBkColor(HDC hdc, COLORREF color)
{
	LPZGDIOBJ zObj;
	LPZGC	  zGC = (LPZGC)hdc;

	if(! zGC) return 0;

	zObj = CreateSolidBrush(color);
	if(zObj)
	{
		zGC->textbkg = (HBRUSH)zObj;
		return _z_rgb_colorofbrush((HBRUSH)zObj);
	}
	return 0;
}

//**************************************************************************
COLORREF WINAPI GetBkColor(HDC hdc)
{
	LPZGC	  	zGC = (LPZGC)hdc;

	if(! zGC) return 0;
	return _z_rgb_colorofbrush(zGC->textbkg);
}

//**************************************************************************
HBRUSH WINAPI GetSysColorBrush(int nIndex)
{
	LPZGDIOBJ zObj;
	COLORREF  c;

	c = GetSysColor(nIndex);

	// just create/use a standard brush
	zObj = (LPZGDIOBJ)CreateSolidBrush(c);

	// mark the zgdi obj as permanent (since its a sys color)
	if(zObj) zObj->refs = ZGDIO_PERMANENT;
	return (HBRUSH)zObj;
}

//**************************************************************************
HGDIOBJ WINAPI GetStockObject(int nIndex)
{
	switch(nIndex)
	{
	case NULL_PEN:
	case WHITE_PEN:
		return _zg_WhitePen;

	case WHITE_BRUSH:
	case NULL_BRUSH:
		return _zg_hbrWhite;

	case LTGRAY_BRUSH:
		return _zg_hbrLightGray;

	case GRAY_BRUSH:
		return _zg_hbrGray;

	case DKGRAY_BRUSH:
		return _zg_hbrDarkGray;

	case BLACK_PEN:
		return _zg_BlackPen;

	case BLACK_BRUSH:
		return _zg_hbrBlack;

	case OEM_FIXED_FONT:
	case ANSI_FIXED_FONT:
	case ANSI_VAR_FONT:
	case SYSTEM_FONT:
	case DEVICE_DEFAULT_FONT:
	case SYSTEM_FIXED_FONT:
		return _zg_sysFont;

	case DEFAULT_PALETTE:
	default:
		return NULL;
	}
}

//**************************************************************************
HGDIOBJ WINAPI SelectObject(HDC hDC, HGDIOBJ hObj)
{
	LPZGC  	  zGC;
	LPZGDIOBJ zObj;
	LPZGDIOBJ oldObj = NULL;

	if((zGC = (LPZGC)hDC) == NULL)	 		return NULL;
	if((zObj = (LPZGDIOBJ)hObj) == NULL) 	return NULL;
	if(zObj->tag != ZGDIO_TAG) 				return NULL;

	switch(zObj->type)
	{
	case gdiBrush:

		oldObj			= zGC->hBRUSH;
		zGC->hBRUSH 	= zObj;
		break;

	case gdiPen:

		{
		LPZPEN zPen = (LPZPEN)zObj->obj;

		oldObj			= zGC->hPEN;
		if(zPen)
		{
			LPZGDIOBJ po = zPen->zObj;

			if(po && po->tag == ZGDIO_TAG && po->type == gdiBrush)
				zGC->zDrvr->_setFrg(zGC->zDrvr, po);

			zGC->zDrvr->_setPen(zGC->zDrvr, zObj);
		}
		zGC->hPEN 	= zObj;
		}
		break;

	case gdiFont:

		oldObj			= zGC->hFONT;
		zGC->hFONT 		= zObj;
		zGC->zDrvr->_setFont(zGC->zDrvr, zObj);
		break;

	case gdiBitmap:

		oldObj  		= zGC->hBITMAP;
		zGC->hBITMAP	= zObj;
		break;

	default:

		SetLastError(ERROR_INVALID_PARAMETER);
		return NULL;
	}
	return oldObj;
}

//**************************************************************************
int WINAPI GetObject(HGDIOBJ hObj, int cbBuffer, void* lpvObject )
{
	LPZGDIOBJ zObj;

	if((zObj = (LPZGDIOBJ)hObj) == NULL)	return 0;
	if(zObj->tag != ZGDIO_TAG)  			return 0;

	switch(zObj->type)
	{
	case gdiBrush:

	{
		LOGBRUSH lb;
		//LPZCOLOR zBrush = (LPZCOLOR)zObj->obj;

		if(lpvObject == NULL)
			return sizeof(LOGBRUSH);
		if(cbBuffer < sizeof(LOGBRUSH))
		{
			SetLastError(ERROR_INSUFFICIENT_BUFFER);
			return 0;
		}
		lb.lbStyle  = 0; // BS_SOLID;
		lb.lbColor  = _z_rgb_colorofbrush(zObj);
		lb.lbHatch  = 0;
		memcpy(lpvObject, &lb, sizeof(LOGBRUSH));
		break;
	}

	case gdiPen:

	{
		LOGPEN lp;
		LPZPEN zPen = (LPZPEN)zObj->obj;

		if(lpvObject == NULL)
			return sizeof(LOGPEN);
		if(cbBuffer < sizeof(LOGPEN))
		{
			SetLastError(ERROR_INSUFFICIENT_BUFFER);
			return 0;
		}
		lp.lopnStyle   = zPen->style;
		lp.lopnWidth.x = zPen->width;
		if(zPen->zObj)
			lp.lopnColor = _z_rgb_colorofbrush(zPen->zObj);
		else
			lp.lopnColor = 0;
		memcpy(lpvObject, &lp, sizeof(LOGPEN));
		break;
	}

	case gdiFont:

	{
		LOGFONT lf;
		LPZFONT zFont = (LPZFONT)zObj->obj;

		if(lpvObject == NULL)
			return sizeof(LOGFONT);
		if(cbBuffer < sizeof(LOGFONT))
		{
			SetLastError(ERROR_INSUFFICIENT_BUFFER);
			return 0;
		}
		memset(&lf, 0, sizeof(LOGFONT));
		lf.lfHeight 		= zFont->tm.tmHeight;
		lf.lfWidth  		= zFont->tm.tmAveCharWidth;
		lf.lfEscapement 	= 0;
		lf.lfOrientation	= 0;
		lf.lfWeight 		= zFont->tm.tmWeight;
		lf.lfItalic 		= zFont->tm.tmItalic;
		lf.lfUnderline  	= zFont->tm.tmUnderlined;
		lf.lfStrikeOut  	= zFont->tm.tmStruckOut;
		lf.lfCharSet		= zFont->tm.tmCharSet;
		lf.lfOutPrecision   = 0;
		lf.lfClipPrecision  = 0;;
		lf.lfQuality		= 0;
		lf.lfPitchAndFamily = zFont->tm.tmPitchAndFamily;
		_tcscpy(lf.lfFaceName, zFont->face);
		memcpy(lpvObject, &lf, sizeof(LOGFONT));
		break;
	}

	case gdiBitmap:

	{
		BITMAP	bm;
		LPZBITMAP zbm = (LPZBITMAP)zObj->obj;

		if(lpvObject == NULL)
			return sizeof(BITMAP);
		if(cbBuffer < sizeof(BITMAP))
		{
			SetLastError(ERROR_INSUFFICIENT_BUFFER);
			return 0;
		}
		bm.bmType   	= 0;
		bm.bmWidth  	= zbm->w;
		bm.bmHeight 	= zbm->h;
		bm.bmWidthBytes = (zbm->w * zbm->d + 7) / 8;
		bm.bmPlanes 	= 1;
		bm.bmBitsPixel  = zbm->d;
		bm.bmBits   	= zbm->data;
		memcpy(lpvObject, &bm, sizeof(BITMAP));
		break;
	}

	default:

		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	return TRUE;
}

//**************************************************************************
BOOL WINAPI DeleteObject(HGDIOBJ hObj)
{
	LPZGDIOBJ zObj;

	if((zObj = (LPZGDIOBJ)hObj) == NULL)	return FALSE;
	if(zObj->tag != ZGDIO_TAG)  			return FALSE;

	_w_deleteZGDIOBJ(zObj);
	return TRUE;
}

//**************************************************************************
int	WINAPI SetMapMode(HDC hDC, int mode)
{
	LPZGC   zGC;
	LPZDRVR zDrvr;
	int	 oldmode;

	if(! (zGC = (LPZGC)hDC)) return FALSE;
	if(! (zDrvr = zGC->zDrvr)) return FALSE;

	oldmode = zGC->mapmode;
	if(mode >= 1 && mode <= 6)
	{
		zGC->mapmode = mode;
		switch(mode)
		{
		default:
		case MM_TEXT:
			zGC->tx = 0x10000;
			zGC->ty = 0x10000;
			zGC->fx = zDrvr->xmar;
			zGC->fy = zDrvr->ymar;
			break;
		case MM_LOMETRIC:
			zGC->tx = (zDrvr->xres << 16) / 254;
			zGC->ty = (zDrvr->yres << 16) / 254;
			zGC->fx = zDrvr->xmar;
			zGC->fy = zDrvr->ymar;
			break;
		case MM_HIMETRIC:
			zGC->tx = (zDrvr->xres << 16) / 2540;
			zGC->ty = (zDrvr->yres << 16) / 2540;
			zGC->fx = zDrvr->xmar;
			zGC->fy = zDrvr->ymar;
			break;
		case MM_LOENGLISH:
			zGC->tx = (zDrvr->xres << 16) / 100;
			zGC->ty = (zDrvr->yres << 16) / 100;
			zGC->fx = zDrvr->xmar;
			zGC->fy = zDrvr->ymar;
			break;
		case MM_HIENGLISH:
			zGC->tx = (zDrvr->xres << 16) / 1000;
			zGC->ty = (zDrvr->yres << 16) / 1000;
			zGC->fx = zDrvr->xmar;
			zGC->fy = zDrvr->ymar;
			break;
		case MM_TWIPS:
			zGC->tx = (zDrvr->xres << 16) / 1440;
			zGC->ty = (zDrvr->yres << 16) / 1440;
			zGC->fx = zDrvr->xmar;
			zGC->fy = zDrvr->ymar;
			break;
		}
		// if device has y origin at bottom (which implies here that
		// positive y is up, which it better be),  flip ty and offset
		// y by view extent, since things are backwards
		//
		if(! zDrvr->yorigTop)
		{
			if(mode == MM_TEXT)
				zGC->ty = -zGC->ty;
			zGC->fy += zDrvr->vph;
		}
		return oldmode;
	}
	SetLastError(ERROR_INVALID_PARAMETER);
	return 0;
}

//**************************************************************************
int	WINAPI GetMapMode(HDC hDC)
{
	LPZGC zGC;

	if(! (zGC = (LPZGC)hDC)) return FALSE;
	return zGC->mapmode;
}

//**************************************************************************
void _zMapPoint(LPZGC zGC, int* x, int* y)
{
	if(! zGC) return;
	if(x) *x = (((long)*x * zGC->tx) >> 16) + zGC->fx + zGC->xoff;
	if(y) *y = (((long)*y * zGC->ty) >> 16) + zGC->fy + zGC->yoff;
}

//**************************************************************************
int WINAPI FillRect(
					HDC hDC,
					CONST RECT *lprc,
					HBRUSH hbr
					)
{
	LPZGC  		zGC;
	LPZGDIOBJ 	zObj;
	RECT		xrc;
	BYTE 		prevalpha;
	DWORD 		style;
	int			rv;

	if(
			! (zGC = (LPZGC)hDC) 	  	||
			! (zObj = (LPZGDIOBJ)hbr)	||
			  (zObj->tag != ZGDIO_TAG)	||
			  (zObj->type != gdiBrush)	||
			! lprc
	)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	zGC->zDrvr->_setFrg(zGC->zDrvr, hbr);

	xrc.left = lprc->left;
	xrc.top  = lprc->top;
	_zMapPoint(zGC, &xrc.left, &xrc.top);

	xrc.right   = lprc->right - 1;
	xrc.bottom  = lprc->bottom - 1;
	_zMapPoint(zGC, &xrc.right, &xrc.bottom);

	if (zGC->zWnd)
	{
		int alpha;

		style = GetWindowLong(zGC->zWnd, GWL_EXSTYLE);
		if (style & WS_EX_LAYERED)
		{
			alpha = zGC->zWnd->key >> 24;
			//alpha *= _zg_alpha;
			//alpha /= 255;
			prevalpha = _z_x_setbrushalpha(hbr, (BYTE)alpha);
		}
	}
	rv = zGC->zDrvr->_fillRect(zGC->zDrvr, &xrc, hbr);

	if(zGC->zWnd)
	{
		if (style & WS_EX_LAYERED)
			_z_x_setbrushalpha(hbr, prevalpha);
	}
	return rv;
}

//**************************************************************************
BOOL  WINAPI TextOut(HDC hDC, int x, int y, LPCTSTR pText, int nText)
{
	LPZGC 		zGC = (LPZGC)hDC;
	BYTE 		prevalpha;
	DWORD 		style;

	if(! zGC) return FALSE;

	_zMapPoint(zGC, &x, &y);

	zGC->zDrvr->_setFrg(zGC->zDrvr, zGC->textfrg);
	if (zGC->zWnd)
	{
		int alpha;

		style = GetWindowLong(zGC->zWnd, GWL_EXSTYLE);
		if (style & WS_EX_LAYERED)
		{
			alpha = zGC->zWnd->key >> 24;
			//alpha *= _zg_alpha;
			//alpha /= 255;
			prevalpha = _z_x_setbrushalpha(zGC->textbkg, (BYTE)alpha);
		}
	}
	zGC->zDrvr->_setBkg(zGC->zDrvr, zGC->textbkg);
	zGC->zDrvr->_textOut(zGC->zDrvr, x, y, pText, nText);
	if(zGC->zWnd)
	{
		if (style & WS_EX_LAYERED)
			_z_x_setbrushalpha(zGC->textbkg, prevalpha);
	}
	return TRUE;
}

//**************************************************************************
BOOL WINAPI MoveToEx(HDC hDC, int x, int y, LPPOINT lpp)
{
	LPZGC zGC = (LPZGC)hDC;

	if(! zGC) return FALSE;

	if(lpp)
	{
		lpp->x = zGC->x;
		lpp->y = zGC->y;
	}
	zGC->x = x;
	zGC->y = y;

	_zMapPoint(zGC, &x, &y);
	return zGC->zDrvr->_moveTo(zGC->zDrvr, x, y);
}

//**************************************************************************
BOOL WINAPI LineTo(HDC hDC, int x, int y)
{
	LPZGC zGC = (LPZGC)hDC;

	if(! zGC) return FALSE;

	zGC->x = x;
	zGC->y = y;
	_zMapPoint(zGC, &x, &y);
	return zGC->zDrvr->_lineTo(zGC->zDrvr, x, y);
}

//**************************************************************************
LONG WINAPI GetWindowLong(HWND hWnd, int nIndex)
{
	LPZWND 	  zWnd;

	if(! (zWnd = Zwindow(hWnd)) || (nIndex < GWL_MIN - 4 || nIndex > GWL_MAX - 4))
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	return zWnd->misc[nIndex - (GWL_MIN - 4)];
}

//**************************************************************************
LONG WINAPI SetWindowLong(HWND hWnd, int nIndex, LONG dwNewLong)
{
	LPZWND 	  zWnd;

	if(! (zWnd = Zwindow(hWnd)) || (nIndex < GWL_MIN - 4 || nIndex > GWL_MAX - 4))
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return dwNewLong;
	}
	return (zWnd->misc[nIndex - (GWL_MIN - 4)] = dwNewLong);
}

//**************************************************************************
LONG WINAPI GetClassLong(HWND hWnd, int nIndex)
{
	LPZWND 	  zWnd;

	if(! (zWnd = Zwindow(hWnd)))
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	switch(nIndex)
	{
	case GCW_ATOM:			return (LONG)zWnd->class;
	case GCL_CBCLSEXTRA:	return (LONG)zWnd->class->cbClsExtra;
	case GCL_CBWNDEXTRA:	return (LONG)zWnd->class->cbWndExtra;
	case GCL_HBRBACKGROUND:	return (LONG)zWnd->class->hbrBkg;
	case GCL_HCURSOR:		return (LONG)zWnd->class->hCursor;
	case GCL_HICON:			return (LONG)zWnd->class->hIcon;
	case GCL_HICONSM:		return (LONG)zWnd->class->hIcon;
	case GCL_HMODULE:		return (LONG)0xbeef;
	case GCL_MENUNAME:		return (LONG)zWnd->class->menu;
	case GCL_STYLE:			return (LONG)0;
	case GCL_WNDPROC:		return (LONG)zWnd->class->proc;
	default:
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
}

//**************************************************************************
LONG WINAPI SetClassLong(HWND hWnd, int nIndex, LONG dwNewLong)
{
	LPZWND 	 zWnd;
	LPZGDIOBJ	zObj;
	LONG		rv;

	if(! (zWnd = Zwindow(hWnd)))
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	switch(nIndex)
	{
	case GCL_CBCLSEXTRA:	rv = (LONG)zWnd->class->cbClsExtra; zWnd->class->cbClsExtra = dwNewLong; break;
	case GCL_CBWNDEXTRA:	rv = (LONG)zWnd->class->cbWndExtra; zWnd->class->cbWndExtra = dwNewLong; break;
	case GCL_HBRBACKGROUND:	rv = (LONG)zWnd->class->hbrBkg;
							zObj = (LPZGDIOBJ)(HBRUSH)dwNewLong;
							if(zObj && zObj->tag == ZGDIO_TAG && zObj->type == gdiBrush)
								zWnd->class->hbrBkg = _w_refZGDIOBJ(zObj);
							break;
	case GCL_HCURSOR:		rv = (LONG)zWnd->class->hCursor;
							zObj = (LPZGDIOBJ)(HCURSOR)dwNewLong;
							if(zObj && zObj->tag == ZGDIO_TAG && zObj->type == gdiBitmap)
								 zWnd->class->hCursor = (HCURSOR)_w_refZGDIOBJ(zObj);
							break;
	case GCL_HICON:			rv = (LONG)zWnd->class->hIcon; zWnd->class->hIcon = (HICON)dwNewLong;
							zObj = (LPZGDIOBJ)(HICON)dwNewLong;
							if(zObj && zObj->tag == ZGDIO_TAG && zObj->type == gdiBitmap)
								 zWnd->class->hIcon = (HICON)_w_refZGDIOBJ(zObj);
							break;
	case GCL_HICONSM:		rv = (LONG)zWnd->class->hIcon; zWnd->class->hIcon = (HICON)dwNewLong;
							zObj = (LPZGDIOBJ)(HICON)dwNewLong;
							if(zObj && zObj->tag == ZGDIO_TAG && zObj->type == gdiBitmap)
								 zWnd->class->hIcon = (HICON)_w_refZGDIOBJ(zObj);
							break;
	case GCL_MENUNAME:		rv = (LONG)zWnd->class->menu; zWnd->class->menu = (LPDWORD)dwNewLong; break;
	case GCL_STYLE:			rv = (LONG)0; break;
	case GCL_WNDPROC:		rv = (LONG)zWnd->class->proc; zWnd->class->proc = (WNDPROC)dwNewLong; break;
	default:
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	return 0;
}

//**************************************************************************
HWND WINAPI GetParent(HWND hWnd)
{
	LPZWND	 zWnd;
	Window   parent;

	zWnd = Zwindow(hWnd);
	if(zWnd)
	{
		parent = _w_getParent(zWnd->xWnd);
		if(parent)
		{
			return (HWND)ZXwindow(parent);
		}
	}
	return NULL;
}

//**************************************************************************
HWND WINAPI SetParent(HWND hWndChild, HWND hWndNewParent)
{
	LPZWND	 zWndChild, zWndParent;

	if(! (zWndChild = Zwindow(hWndChild)))
		return NULL;
	if(! (zWndParent = Zwindow(hWndNewParent)))
		return NULL;
	_w_setParent(zWndChild->xWnd, zWndParent->xWnd);
	return NULL;
}

//**************************************************************************
int WINAPI LoadString(HINSTANCE hInstance, UINT uID, LPTSTR lpBuffer, int nBufferMax)
{
	return 0;
}

//**************************************************************************
HANDLE WINAPI GlobalAlloc(int flags, int size)
{
	LPZMEMH zMem;

	zMem = (LPZMEMH)malloc(sizeof(ZMEMH));
	if(! zMem) return NULL;

	zMem->data = malloc(size);
	if(! zMem->data)
	{
		free(zMem);
		return NULL;
	}
	if(flags & GMEM_ZEROINIT)
		memset(zMem->data, 0, size);
	zMem->tag = ZMEMH_TAG;
	zMem->len = size;
	zMem->next = _zg_memhandles;
	_zg_memhandles = zMem->next;
	return (HANDLE)zMem;
}

//**************************************************************************
void*  WINAPI GlobalLock(HANDLE hMem)
{
	LPZMEMH zMem = (LPZMEMH)hMem;

	if(! zMem || ! (zMem->tag == ZMEMH_TAG))
		return NULL;
	return (void*)(zMem->data);
}

//**************************************************************************
void   WINAPI GlobalUnlock(HANDLE hMem)
{
}

//**************************************************************************
void   WINAPI GlobalFree(HANDLE hMem)
{
	LPZMEMH zMem = (LPZMEMH)hMem;
	LPZMEMH zm;

	if(! zMem || ! (zMem->tag == ZMEMH_TAG))
		return;
	if(zMem->data)
		free(zMem->data);
	if(zMem == _zg_memhandles)
		_zg_memhandles = _zg_memhandles->next;
	else
	{
		for(zm = _zg_memhandles; zm && zm->next && zm->next != zMem;)
			zm = zm->next;
		if(zm) zm->next = zMem->next;
	}
	free(zMem);
}

//**************************************************************************
BOOL WINAPI CreateCaret(
					    HWND hWnd,
					    HBITMAP hBitmap,
					    int nWidth,
					    int nHeight)
{
//	if(hWnd != GetFocus()) return FALSE;

	__w_caret(cstOFF);
	_zg_caret.owner	= hWnd;
	_zg_caret.w		= nWidth;
	_zg_caret.h		= nHeight;
	//_zg_caret.hbm	= hBitmap;
	_zg_caret.blink	= 500;
	_zg_caret.scnt 	= 0;
	_zg_caret.x		= 0;
	_zg_caret.y		= 0;
	_zg_caret.lx   	= 0;
	_zg_caret.ly   	= 0;
	_zg_caret.state	= cstOFF;
	return TRUE;
}

//**************************************************************************
HWND _w_caretOwner(int *x, int *y, int *w, int *h)
{
	*w = _zg_caret.w;
	*h = _zg_caret.h;
	*x = _zg_caret.x;
	*y = _zg_caret.y;
	return _zg_caret.owner;
}

//**************************************************************************
UINT WINAPI GetCaretBlinkTime(void)
{
	if(_zg_caret.owner)
		return _zg_caret.blink;
	return TRUE;
}

//**************************************************************************
BOOL WINAPI SetCaretBlinkTime(UINT uMSeconds)
{
	if(_zg_caret.owner)
		return _zg_caret.blink = uMSeconds;
	return TRUE;
}

//**************************************************************************
BOOL WINAPI DestroyCaret(void)
{
	if(_zg_caret.owner)
	{
		if(_zg_caret.scnt > 0)
		{
			_zg_caret.scnt = 1;
			HideCaret(NULL);
		}
		_zg_caret.owner = NULL;
	}
	return TRUE;
}

//**************************************************************************
BOOL WINAPI HideCaret(HWND hWnd)
{
	if((_zg_caret.owner && _zg_caret.owner == hWnd) || hWnd == NULL)
	{
		if(_zg_caret.scnt > 0)
		{
			_zg_caret.scnt--;
			if(! _zg_caret.scnt)
				__w_caret(cstOFF);
		}
	}
	return TRUE;
}

//**************************************************************************
BOOL WINAPI ShowCaret(HWND hWnd)
{
	if((_zg_caret.owner && _zg_caret.owner == hWnd) || hWnd == NULL)
	{
		_zg_caret.scnt++;
		if(_zg_caret.scnt > 0)
			__w_caret(cstON);
	}
	return TRUE;
}

//**************************************************************************
BOOL WINAPI SetCaretPos(int X, int Y)
{
	if(! _zg_caret.owner) return FALSE;
	__w_caret(cstOFF);
	_zg_caret.x = X;
	_zg_caret.y = Y;
	__w_caret(cstON);
	return TRUE;
}

//**************************************************************************
BOOL WINAPI GetCaretPos(LPPOINT lpPoint)
{
	if(! lpPoint) return FALSE;
	if(! _zg_caret.owner) return FALSE;
	lpPoint->x = _zg_caret.x;
	lpPoint->y = _zg_caret.y;
	return TRUE;
}

//**************************************************************************
BOOL WINAPI SetCursorPos(int x, int y)
{
#ifdef X11
	XWarpPointer(_zg_display, None, DefaultRootWindow(_zg_display),
				0, 0, 0, 0, x, y);
#endif
	return TRUE;
}

//**************************************************************************
BOOL WINAPI GetCursorPos(LPPOINT lpPoint)
{
	int cx, cy;
#ifdef X11
	int x, y;
	Window root, child;
	unsigned int buttons;

	if(! lpPoint) return FALSE;

	XQueryPointer(
					_zg_display,
					DefaultRootWindow(_zg_display),
					&root,
					&child,
					&x,
					&y,
					&cx,
					&cy,
					&buttons
					);
#endif
#ifdef COCOA
	_w_mousePosition(&cx, &cy, NULL);
#endif
	lpPoint->x = cx;
	lpPoint->y = cy;
	return TRUE;
}

//**************************************************************************
int WINAPI MapWindowPoints(HWND hWndFrom, HWND hWndTo, LPPOINT lpPoints, UINT cPoints)
{
	RECT rcf, rct;
	RECT rcfo, rcto;
	int  n;
	int  dx, dy;

	if(! lpPoints || ! cPoints)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	GetWindowRect(hWndFrom, &rcf);
	_w_client_rect_offsets(hWndFrom, &rcfo);

	GetWindowRect(hWndTo, &rct);
	_w_client_rect_offsets(hWndTo, &rcto);

	//rcto.left = rcto.top = rcto.bottom = rcto.right = 0;
	//rcfo.left = rcfo.top = rcfo.bottom = rcfo.right = 0;

	dx = rcf.left - rct.left - rcto.left + rcfo.left;
	dy = rcf.top  - rct.top - rcto.top + rcfo.top;

	//printf("dx=%d dy=%d  cro f=%d,%d t=%d,%d\n", dx,dy, rcfo.left, rcfo.top, rcto.left, rcto.top);
#if 0
	// for top level windows, the upper left part of window rect is wrong
	// by the size of the wm decoration placed on it, so adjust back by
	// the decoration amount
	//
	if(! (GetWindowLong(hWndTo, GWL_STYLE) & WS_CHILD))
	{
		LPZWND  	   zWnd;
		unsigned int   myX, myY, myWidth, myHeight, myBorderWidth, myDepth;
		unsigned int   momX, momY, momWidth, momHeight, momBorderWidth, momDepth;
		Window  	   myMommy, myRoot, *myKids;
		int 		   numKids;
		RECT		   rcc;

		if(! (zWnd = Zwindow(hWndTo)))
		{
			SetLastError(ERROR_INVALID_PARAMETER);
			return 0;
		}
		rcc.left = rcc.right = rcc.top = rcc.bottom = 0;

		myMommy = zWnd->xWnd;
		momX = momY = 0;

		do
		{
			XQueryTree(
					_zg_display,
					myMommy,
					&myRoot,
					&myMommy,
					&myKids,
					&numKids
					);
			if(myKids)
				XFree(myKids);
			//printf("x=%d y=%d mm=%p mr=%p\n", momX, momY, myMommy, myRoot);
			if(myMommy != myRoot)
			{
				rcc.left	+= momX;
				rcc.top 	+= momY;
			}
			if(myMommy)
			{
				XGetGeometry(
						_zg_display,
						myMommy,
						&myRoot,
						&momX, &momY,
						&momWidth, &momHeight,
						&momBorderWidth,
						&momDepth
					 );
			}
		}
		while(myMommy && myMommy != myRoot);
		dx -= rcc.left;
		dy -= rcc.top;
	}
#endif
	for(n = 0; n < cPoints; n++)
	{
		lpPoints->x += dx;
		lpPoints->y += dy;
		lpPoints++;
	}
	return 0;
}

//**************************************************************************
UINT_PTR WINAPI SetTimer(HWND hWnd, UINT id, UINT elapse, TIMERPROC proc)
{
	LPZTIMER zTimer;

	if(! hWnd && ! proc)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	for(zTimer = _zg_timers; zTimer; zTimer = zTimer->next)
    {
		if(zTimer->hWnd	== hWnd && zTimer->id == id)
		{
			KillTimer(hWnd, (UINT_PTR)(intptr_t)zTimer->id);
			break;
		}
	}
	zTimer = _w_newZTIMER(hWnd, id, elapse, proc);

	return (UINT_PTR)zTimer;
}

//**************************************************************************
BOOL WINAPI KillTimer(HWND hWnd, UINT_PTR id)
{
	LPZTIMER zTimer;

	if(hWnd)
	{
		for(zTimer = _zg_timers; zTimer; zTimer = zTimer->next)
	    {
			if(zTimer->hWnd	== hWnd && zTimer->id == (intptr_t)id)
			{
				break;
			}
		}
	}
	else
	{
		zTimer = (LPZTIMER)id;
	}
	if(! zTimer || zTimer->tag != ZTIMER_TAG)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	_w_deleteZTIMER(zTimer);
	return TRUE;
}

//**************************************************************************
DWORD GetFullPathName(LPCTSTR path, int cbfn, LPTSTR fullname, LPTSTR* pfp)
{
	char   cwd[MAX_PATH];
	TCHAR  dir[MAX_PATH];
	LPTSTR file, pfn, pd;
	int	dirl, filel;
	BOOL   needptc = FALSE;

	if(! path)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	if(path[0] != _PTC_)
	{
		getcwd(cwd, MAX_PATH);
		CharToTChar(dir, cwd);
	}
	else
	{
		dir[0] = _T('\0');
	}
	// for each ".." in front of path, remove one
	// dir from end of cwd
	//
	file = (LPTSTR)path;
	pd   = dir + _tcslen(dir) - 1;

	if(pd <= dir)
	{
		file = (LPTSTR)path;
		dir[0]  = _T('\0');
	}
	else if(path[0] != _PTC_)
	{
		pfn = file;

		while((pfn = _tcsstr(pfn, _T(".."))) != NULL)
		{
			pfn += 2;
			while(*pfn == _PTC_)
				pfn++;
			file = pfn;

			while(pd >= dir)
			{
				if(*pd == _PTC_)
				{
					*pd = _T('\0');
					break;
				}
				pd--;
			}
		}
	}
	else
	{
		file    = (LPTSTR)path;
		dir[0]  = _T('\0');
	}
	dirl  = _tcslen(dir);
	filel = _tcslen(file);
	if(file[0] != _PTC_ && (! dirl || (dir[dirl - 1] != _PTC_)))
	{
		filel++;
		needptc = TRUE;
	}
	if(dirl + filel < cbfn)
	{
		_tcscpy(fullname, dir);
		if(needptc)
			_tcscat(fullname, _PTS_);
		_tcscat(fullname, file);
		file = fullname;

		pfn = file + _tcslen(file) - 1;

		while(pfn >= file)
		{
			if(*pfn == _PTC_)
			{
				pfn++;
				break;
			}
			pfn--;
		}
		if(pfp)
			*pfp = pfn;
	}
	return dirl + filel;
}

//**************************************************************************
void WINAPI SetLastError(DWORD err)
{
	_zg_lasterror = err;
}

//**************************************************************************
DWORD WINAPI GetLastError(void)
{
	return _zg_lasterror;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

extern int APIENTRY WinMain(
								HINSTANCE hInstance,
								HINSTANCE hPrevInstance,
								LPSTR     lpCmdLine,
								int 	  nCmdShow
							);

extern void _w_imageInit(void);

static LPTSTR g_cmdLine = NULL;

//**************************************************************************
LPTSTR GetCommandLine()
{
    if(! g_cmdLine) return _T("");
    return g_cmdLine;
}

//**************************************************************************
void _w_SetSystemFont(HFONT hFont, int whichfont)
{
	LPZGDIOBJ zObj;
	LPZGDIOBJ* pzObj;
    int setTreeFont = 0;

	if(! hFont) return;

	switch(whichfont)
	{
	case 0: // sysfont
		pzObj = (LPZGDIOBJ*)&_zg_sysFont;
        setTreeFont = (_zg_sysFont == _zg_treeFont);
		break;
	case 1: // menufont and treefont
		pzObj = (LPZGDIOBJ*)&_zg_menuFont;
        setTreeFont = (_zg_menuFont == _zg_treeFont);
		break;
	default:
		pzObj = NULL;
		break;
	}
	if(! pzObj)
		return;
	zObj = *pzObj;
	if(zObj->refs == ZGDIO_PERMANENT)
	{
		zObj->refs = 0;
		DeleteObject(zObj);
	}
	ZGDIO_MAKE_PERMANENT(hFont);
	*pzObj = (LPZGDIOBJ)hFont;
    if(setTreeFont)
    {
        _zg_treeFont = (LPZGDIOBJ)hFont;
    }
}

//**************************************************************************
int main(int argc, LPSTR* argv)
{
	int	     	rv;
	int	     	narg;
	int	     	szcmds;
	LPSTR 	 	cmdLine;
	LPZCLASS 	pclass;

	_zg_zWndRoot = (LPZWND)malloc(sizeof(ZWND));
	memset(_zg_zWndRoot, 0, sizeof(ZWND));
	_zg_windows = _zg_zWndRoot;

	_w_init();

	_zg_zc_black.xcolor = _zg_Black;
	_zg_zc_black.r = _zg_zc_black.g = _zg_zc_black.b = 0;
	_zg_zc_black.c = 255;

	_zg_zc_white.xcolor = _zg_White;
	_zg_zc_white.r = _zg_zc_white.g = _zg_zc_white.b = 255;
	_zg_zc_white.c = 255;

	_zg_colors = &_zg_zc_black;
	_zg_zc_black.next = &_zg_zc_white;

	_zg_hbrWhite	 	= _w_newZGDIOBJ(gdiBrush, &_zg_zc_white);
	_zg_hbrBlack 		= _w_newZGDIOBJ(gdiBrush, &_zg_zc_black);

	//_zg_hbrWhite	 	= CreateSolidBrush(RGB(255,255,255));
	//_zg_hbrBlack	 	= CreateSolidBrush(RGB(0,0,0));

	_zg_preferGray		= TRUE;
	_zg_hbrLightGray 	= CreateSolidBrush(RGB(241, 239, 226));
	_zg_hbrVDarkGray 	= CreateSolidBrush(RGB(113, 111, 100));
	_zg_hbrDarkGray	 	= CreateSolidBrush(RGB(172, 168, 153));
	_zg_hbrGray		 	= CreateSolidBrush(RGB(127, 127, 127));
	_zg_hbrWindowsGray	= CreateSolidBrush(RGB(212, 208, 200));
	_zg_preferGray		= FALSE;
	_zg_hbrBlue		 	= CreateSolidBrush(RGB(0, 0, 127));

	ZGDIO_MAKE_PERMANENT(_zg_hbrWhite);
	ZGDIO_MAKE_PERMANENT(_zg_hbrBlack);
	ZGDIO_MAKE_PERMANENT(_zg_hbrLightGray);
	ZGDIO_MAKE_PERMANENT(_zg_hbrVDarkGray);
	ZGDIO_MAKE_PERMANENT(_zg_hbrDarkGray);
	ZGDIO_MAKE_PERMANENT(_zg_hbrGray);
	ZGDIO_MAKE_PERMANENT(_zg_hbrBlue);
	ZGDIO_MAKE_PERMANENT(_zg_hbrWindowsGray);

	_zg_WhitePen	 	= _w_newZGDIOBJ(gdiPen, _w_newZPEN(PS_SOLID, 0, _zg_hbrWhite));
	_zg_BlackPen 		= _w_newZGDIOBJ(gdiPen, _w_newZPEN(PS_SOLID, 0, _zg_hbrBlack));
	ZGDIO_MAKE_PERMANENT(_zg_WhitePen);
	ZGDIO_MAKE_PERMANENT(_zg_BlackPen);

	// make system / default font
	//
	_zg_sysFont = CreateFont(
							12,0,0,0,500,FALSE,FALSE,FALSE,ANSI_CHARSET,
							OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
							ANTIALIASED_QUALITY/*DEFAULT_QUALITY*/,FF_SWISS,_T("Arial")
							);
	if(! _zg_sysFont)
	{
		fprintf(stderr, "No system font, bailing\n");
		exit(-1);
	}
	ZGDIO_MAKE_PERMANENT(_zg_sysFont);

    _zg_menuFont = CreateFont(
                            12,0,0,0,500,FALSE,FALSE,FALSE,ANSI_CHARSET,
                            OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
                            ANTIALIASED_QUALITY/*DEFAULT_QUALITY*/,FF_SWISS,_T("Arial")
                            );
	_zg_treeFont = _zg_menuFont;

    ZGDIO_MAKE_PERMANENT(_zg_menuFont);

    _zg_treeFontBold = CreateFont(
                            12,0,0,0,700,FALSE,FALSE,FALSE,ANSI_CHARSET,
                            OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
                            ANTIALIASED_QUALITY/*DEFAULT_QUALITY*/,FF_SWISS,_T("Arial")
                            );
    ZGDIO_MAKE_PERMANENT(_zg_treeFontBold);

	// init builtin classes
	//
	for(pclass = _zg_builtin_classes; pclass && pclass->name; pclass++)
	{
		if(pclass->hbrBkg != 0)
			pclass->hbrBkg = GetSysColorBrush((intptr_t)pclass->hbrBkg - 1);
		else
			pclass->hbrBkg = GetSysColorBrush(COLOR_WINDOW);
		if(pclass->hCursor != NULL)
			pclass->hCursor = LoadCursor(NULL, pclass->hCursor);
		pclass->next = (pclass + 1)->name ? (pclass + 1) : NULL;
	}
	// init image subsystem
	_w_imageInit();

#if 0
	// open the resource file if present
	//
	{
		TCHAR resName[MAX_PATH + 4];

		CharToTChar(resName, argv[0]);
		_tcscat(resName, _T(".res"));

		__LoadResource(resName);
	}
#else
	{
		//our local pointer/cnt to it
		extern LPDWORD _zg_resource;
		extern long    _zg_cbresource;

		_zg_resource = (LPDWORD)_zg_resources;
		_zg_cbresource = (long)_cb_zg_resources;
	}
#endif
	// convert argv into command line and call winmain
	//
	for(narg = 0, szcmds = 0; narg < argc; narg++)
	{
		if(! argv[narg]) break;
		szcmds += strlen(argv[narg]);
	}
	cmdLine = (char*)malloc((szcmds + 1) * sizeof(char) + narg * sizeof(char) + 16);
	cmdLine[0] = _T('\0');

	g_cmdLine = (LPTSTR)malloc((szcmds + narg + 1) * sizeof(TCHAR) + 16);
	CharToTChar(g_cmdLine, argv[0]);
	if(narg > 1)
		_tcscat(g_cmdLine, _T(" "));

	for(narg = 1, szcmds = 0; narg < argc; narg++)
	{
		if(! argv[narg]) break;
		strcat(cmdLine, argv[narg]);
		CharToTChar(g_cmdLine + _tcslen(g_cmdLine), argv[narg]);
		if(narg < argc-1)
		{
			strcat(cmdLine, " ");
			_tcscat(g_cmdLine, _T(" "));
		}
	}
	strcpy(_zg_bsaappname, argv[0]);

	// init icccm (clipboard)
	//
	_w_clipInit();

	// call main
	//
	rv =  WinMain((void*)0xBEEF, (void*)NULL, cmdLine, SW_SHOW);

	// epilogs
	//
	_w_clipDeinit();

	free(cmdLine);
	return rv;
}


#endif /* Windows */
