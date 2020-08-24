//--------------------------------------------------------------------
//
// File: winapi.h
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------

#ifdef X11
extern Display* _zg_display;
extern Visual*  _zg_visual;
extern XVisualInfo _zg_vinfo;
#endif
#ifdef COCOA
typedef void *Window, *Cursor, *XImage, *Font;
#endif
extern int      _zg_screen;
extern int 		_zg_compositing;
extern DWORD	_zg_alpha;

extern LPTSTR   _zg_error;

extern HBRUSH	_zg_hbrWhite;
extern HBRUSH	_zg_hbrBlack;
extern HBRUSH	_zg_hbrBlue;
extern HBRUSH	_zg_hbrLightGray;
extern HBRUSH	_zg_hbrGray;
extern HBRUSH	_zg_hbrDarkGray;
extern HBRUSH	_zg_hbrWindowsGray;

extern HWND		_zg_hwndMouse;
extern TRACKMOUSEEVENT _zg_mouseTrack;
extern HWND		_zg_dialogWnd[];
extern int		_zg_dialogTop;

extern HWND		_zg_menuWnd[];
extern int		_zg_menuTop;

extern char		_zg_bsaappname[MAX_PATH];
extern int		_zg_bsaappargc;
extern char*	_zg_bsaappargv[];

extern int 		_zg_exit;
extern int		_zg_exitcode;

// app supplied resource
extern BYTE _zg_resources[];
extern int  _cb_zg_resources;

#define	BSA_X11_TITLEBAR_H	32
#define BSA_MOUSE_DBLCLK_MS	600
#define BSA_MOUSE_HOVER_MS	500

#define BSA_MENU_HEIGHT		22

#ifndef BSA_DEFAULT_FONTSIZE
	#define BSA_DEFAULT_FONTSIZE 12
#endif
#ifndef BSA_DEVICE_LOGPIXELSX
	#define BSA_DEVICE_LOGPIXELSX	90	// pixels per inch, horiz
#endif
#ifndef BSA_DEVICE_LOGPIXELSY
	#define BSA_DEVICE_LOGPIXELSY	90	// pixels per inch, vert
#endif
#define BSA_SCALE_CHAR_TO_PS(h) (((long)(h) * 72 + (BSA_DEVICE_LOGPIXELSY / 2)) / BSA_DEVICE_LOGPIXELSY)

#define PANIC(a) fprintf(stderr, a)


#define ALIGN_LEFT		1
#define ALIGN_RIGHT		2
#define ALIGN_CENTER	4
#define ALIGN_TOP		8
#define ALIGN_BOTTOM	16
#define ALIGN_VCENTER	32

#define BSA_VS_WIDTH	16
#define BSA_VSX_WIDTH	16
#define BSA_VSX_HEIGHT	16
#define BSA_HS_HEIGHT	16
#define BSA_HSX_WIDTH	16
#define BSA_HSX_HEIGHT	16

#define BSA_SC_ARROW_WIDTH 4

#define dtDraw			1
#define dtSetCaret		2
#define dtSetPosToMouse	4

#define cstOFF		0
#define cstON		1
#define cstTOGGLE	2

#define RT_Cursor			0x0001
#define RT_Bitmap			0x0002
#define RT_Icon				0x0003
#define RT_Menu				0x0004
#define RT_Dialog			0x0005
#define RT_StringTable		0x0006
#define RT_FontDirectory	0x0007
#define RT_Font				0x0008
#define RT_Accelerators		0x0009
#define RT_RCData			0x000A
#define RT_MessageTable		0x000B
#define RT_GroupCursor		0x000C
#define RT_GroupIcon		0x000E
#define RT_Version			0x0010
#define RT_DialogInclude	0x0011
#define RT_PlugNPlay		0x0013
#define RT_VXD 				0x0014
#define RT_Animated Cursor	0x0015
#define RT_BitmapNew		0x2002
#define RT_MenuNew			0x2004
#define RT_DialogNew		0x2005
#define RT_TypeMask			0x0FFF

//-----------------------------------------------------------------------------
#define BSA_PUSHED		1
#define BSA_ENTERED		2
#define BSA_HOVERED		4
#define BSA_BUTTON_PARM	GWL_PRIVDATA

#define PUSHED(w)  (GetWindowLong(w, BSA_BUTTON_PARM) & BSA_PUSHED)
#define ENTERED(w) (GetWindowLong(w, BSA_BUTTON_PARM) & BSA_ENTERED)
#define HOVERED(w) (GetWindowLong(w, BSA_BUTTON_PARM) & BSA_HOVERED)

#define SETPUSHED(w, b)													\
	SetWindowLong(														\
					w,													\
					BSA_BUTTON_PARM,									\
					(GetWindowLong(w, BSA_BUTTON_PARM) & ~BSA_PUSHED)	\
					| ((b) ? BSA_PUSHED : 0))
				
#define SETENTERED(w, b)												\
	SetWindowLong(														\
					w,													\
					BSA_BUTTON_PARM,									\
					(GetWindowLong(w, BSA_BUTTON_PARM) & ~BSA_ENTERED)	\
					| ((b) ? BSA_ENTERED : 0))

#define SETHOVERED(w, b)												\
	SetWindowLong(														\
					w,													\
					BSA_BUTTON_PARM,									\
					(GetWindowLong(w, BSA_BUTTON_PARM) & ~BSA_HOVERED)	\
					| ((b) ? BSA_HOVERED : 0))
				
//-----------------------------------------------------------------------------

extern int _w_dlg_baseunitX;
extern int _w_dlg_baseunitY;

#define BSA_SCALE_DLG_X(x) (((x) * _w_dlg_baseunitX + 2) / 4)
#define BSA_SCALE_DLG_Y(y) (((y) * _w_dlg_baseunitY + 4) / 8)
#define BSA_UNSCALE_DLG_X(x) (((x) * 4) / _w_dlg_baseunitX)
#define BSA_UNSCALE_DLG_Y(y) (((y) * 8) / _w_dlg_baseunitY)

//-----------------------------------------------------------------------------

#define RESBYTE(a)	*a++
#define RESWORD(a)	((WORD)a[0])|((WORD)a[1] << 8);a+=2
#define RESXDWORD(a)((DWORD)a[0])|((DWORD)a[1] << 8)|((DWORD)a[2] << 16)|((DWORD)a[3] << 24)
#define RESDWORD(a)	((DWORD)a[0])|((DWORD)a[1] << 8)|((DWORD)a[2] << 16)|((DWORD)a[3] << 24);a+=4

//-----------------------------------------------------------------------------

//**************************************************************************
typedef struct tagClass
{
	LPTSTR				name;
	WNDPROC				proc;
	int					cbClsExtra;
	int					cbWndExtra;
	HICON				hIcon;
	HCURSOR				hCursor;
	HBRUSH				hbrBkg;
	LPDWORD				menu;
	int					refs;
	int					ordinal;

	struct tagClass*	next;
}
ZCLASS, *PZCLASS, FAR *LPZCLASS;

extern ZCLASS _zg_builtin_classes[];

#define BSA_MAX_WINDOW_NAME	64

//**************************************************************************
typedef struct tagWindow 
{
#ifdef X11
	Window				xWnd;	// X11 window
#endif
#ifdef COCOA
	void			   *xWnd;
	LPTSTR				name;
#endif
	PAINTSTRUCT			ps;
	LPMSG				msgq;
	HBRUSH				frg;
	HBRUSH				bkg;
	COLORREF			key; 	// upper byte is windows alpha, lower 3 rgb chroma key
	HFONT				font;
	LONG				misc[GWL_MAX - (GWL_MIN - 4)];
	struct tagClass* 	class;
	struct tagzMenu*	menu;
	struct tagWindow*	next;	// list in stacking order
	struct tagWindow*	cnext;	// list in control-chain
	struct tagWindow*	cprev;	// double link
}
ZWND, *PZWIND, FAR *LPZWND;

#define GWL_PRIVDATA	(GWL_MIN - 4)

#define BSA_MAX_NESTED_DIALOGS	8

extern LPZWND 	_zg_zWndRoot;
extern LPZWND 	_zg_windows;
extern LPZWND 	_zg_dialogCtrl;
extern BOOL   	_zg_keynav;

typedef enum { gdiBrush, gdiPen, gdiFont, gdiBitmap } _gdio_type;

//**************************************************************************
typedef struct tagGDIO
{
	DWORD		tag;
	int			refs;
	_gdio_type	type;
	LPVOID		obj;
	struct tagGDIO* next;
}
ZGDIOBJ, FAR *LPZGDIOBJ;

#define ZGDIO_TAG		0xABCD2222
#define ZGDIO_PERMANENT	-100

#define ZGDIO_MAKE_PERMANENT(z) (((z)&&(((LPZGDIOBJ)z)->tag==ZGDIO_TAG))?(((LPZGDIOBJ)z)->refs=ZGDIO_PERMANENT):0)

//**************************************************************************
typedef struct tagzColor	// gdiBrush object
{
	BYTE		r, g, b, c;
	DWORD		xcolor;
	struct tagzColor* next;
}
ZCOLOR, *PZCOLOR, FAR* LPZCOLOR;

//**************************************************************************
typedef struct tagzPen		// gd Pen objcet
{
	int			style;
	int			width;
	LPZGDIOBJ	zObj;
}
ZPEN, *PZPEN, FAR *LPZPEN;

//**************************************************************************
typedef struct tagzFont		// gdiFont object
{
	TCHAR		 face[BSA_MAX_FONTFACE_LEN];
	LPWORD		 widths;
	int			 size;
	TEXTMETRIC	 tm;
	int			 antialias;
#ifdef X11
	XFontStruct* font;
	#ifdef XFT_FONT
	int			 isxft;
	XftFont*	 xftfont;
	#endif
#endif
#ifdef COCOA
	Font 		*font;
#endif
}
ZFONT, *PZFONT, FAR *LPZFONT;

//**************************************************************************
typedef struct tagZBitmap	// gdiBitmap object (used for HICON and HCURSOR too)
{
	LPCTSTR	id;
	int		type;
	Cursor	cursor;
	XImage* ximg;
	char*	data;
	char*	mask;
	DWORD	ibkg;
	BOOL	ibkset;
	int		w, h, d;
	int		xHot, yHot;
}
ZBITMAP, *PZBITMAP, FAR *LPZBITMAP;

//**************************************************************************
typedef struct tagzMenu
{
	DWORD		tag;
	LPTSTR		text;
	WORD		opts;
	WORD		id;
	WORD		width;
	WORD		height;
	BOOL		pushed;
	BOOL		entered;
	LPZWND		zWnd;
	struct tagzMenu* parent;
	struct tagzMenu* sibling;
	struct tagzMenu* child;
	struct tagzMenu* cur;
}
ZMENU, *PZMENU, FAR *LPZMENU;

#define ZMENU_TAG		0xFEDC1234
#define MENU_HEIGHT(z) ((z) ? ((z)->menu ? BSA_MENU_HEIGHT : 0) : 0)

//**************************************************************************
typedef struct tagzTimer
{
	DWORD		tag;
	HWND		hWnd;
	UINT		id;
	UINT		elapse;
	TIMERPROC	proc;
	int			firetime;
	int			firemillitm;
	struct tagzTimer* next;
}
ZTIMER, *PTIMER, FAR *LPZTIMER;

#define ZTIMER_TAG	0xDEF09876

//**************************************************************************
typedef struct tagzMEMH
{
	DWORD		tag;
	HWND		hWnd;
	int			len;
	LPBYTE		data;
	struct tagzMEMH* next;
}
ZMEMH, *PZMMEH, FAR *LPZMEMH;

#define ZMEMH_TAG	0xEEEE0123

typedef struct tagZSCROLLER
{
	DWORD		tag;
    int     nMin;
    int     nMax;
    UINT    nPage;
    int     nPos;
    int     nTrackPos;
    BOOL	firsttimer;
    UINT_PTR idtimer;
    int		type;
    int		uppushed;
    int		downpushed;
    int		ipos;
    HWND	hwndBar;
    HWND	hwndThumb;
    HWND	hwndUp;
    HWND	hwndDown;
    HWND	hwndParent;
}
ZSBAR, *PZSBAR, *LPZSBAR;

#define ZSBAR_TAG 0xEFEF3232

struct tagzGC;

//**************************************************************************
typedef struct tagzGDIDRVR
{
	int				xres, yres;
	int				xmar, ymar;
	int				yorigTop;
	int				xorigLeft;
	int				vpw, vph, vpd;
	struct tagzGC*	zGC;
	LPVOID			drvrData[8];
	
	void (*_delete)		(struct tagzGDIDRVR*);

	void (*_setFrg)		(struct tagzGDIDRVR*, HBRUSH);
	void (*_setBkg)		(struct tagzGDIDRVR*, HBRUSH);
	int  (*_devCaps)	(struct tagzGDIDRVR*, int);
	BOOL (*_setPen)		(struct tagzGDIDRVR*, HPEN);
	BOOL (*_setFont)	(struct tagzGDIDRVR*, HFONT);
	BOOL (*_textOut)	(struct tagzGDIDRVR*, int, int, LPCTSTR, int);
	BOOL (*_moveTo)		(struct tagzGDIDRVR*, int, int);
	BOOL (*_lineTo)		(struct tagzGDIDRVR*, int, int);
	int	 (*_fillRect)	(struct tagzGDIDRVR*, LPRECT, HBRUSH);
	BOOL (*_bitblt)		(struct tagzGDIDRVR*, int, int, int, int, HDC, int, int, DWORD);
	
	BOOL (*_startDoc)	(struct tagzGDIDRVR*, CONST DOCINFO*);
	BOOL (*_endDoc)		(struct tagzGDIDRVR*);
	BOOL (*_startPage)	(struct tagzGDIDRVR*);
	BOOL (*_endPage)	(struct tagzGDIDRVR*);
}
ZDRVR, *PZDRVR, FAR *LPZDRVR;

//**************************************************************************
typedef struct tagzGC
{
	int			mapmode;
	int			tx, fx;
	int			ty, fy;
	int			x;
	int			y;
	int			xoff;
	int			yoff;
	LPZWND		zWnd;
	HGDIOBJ		hBRUSH;
	HGDIOBJ		hPEN;
	HGDIOBJ		hFONT;
	HGDIOBJ		hBITMAP;
	HBRUSH		textfrg;
	HBRUSH		textbkg;
	LPZDRVR		zDrvr;
}
ZGC, *PZGC, FAR *LPZGC;

//**************************************************************************
DWORD 		_z_x_colorofbrush	(LPZGDIOBJ zObj);
COLORREF 	_z_rgb_colorofbrush	(LPZGDIOBJ zObj);
BYTE 		_z_x_getbrushalpha	(LPZGDIOBJ zObj);
BYTE 		_z_x_setbrushalpha	(LPZGDIOBJ zObj, BYTE alpha);

LPZCLASS	_w_newZCLASS	(LPWNDCLASS class);
void		_w_deleteZCLASS	(LPZCLASS zClass);

LPZWND		_w_newZWND		(LPCTSTR class);
void		_w_deleteZWND	(LPZWND zWnd);

LPZWND		Zwindow		(HWND hWnd);
LPZWND		ZXwindow	(Window xWind);

LPZGC		_w_newZGC		(LPZWND zWnd, LPZDRVR zDrvr);
void		_w_deleteZGC	(LPZGC zGC);

void		__w_caret		(int state);
void 		__w_toggle_caret(void);
HWND		_w_caretOwner(int *x, int *y, int *w, int *h);

LPZGDIOBJ	_w_newZGDIOBJ	(_gdio_type type, LPVOID obj);
LPZGDIOBJ	_w_refZGDIOBJ	(LPZGDIOBJ zGDIO);
void		_w_deleteZGDIOBJ(LPZGDIOBJ zGDIO);

HPEN		_w_newZPEN		(int style, int width, LPZGDIOBJ zObj);
void		_w_deleteZPEN	(HPEN pen);

HFONT		_w_newFont		(LPCTSTR pFace, LPTEXTMETRIC ptm, BOOL antialias);
void		_w_deleteFont	(HFONT hFont);

LPZMENU 	_w_newZMENU		(LPCTSTR text, WORD opts, WORD id);
void		_w_deleteZMenu	(LPZMENU zmenu);

LPZTIMER	_w_newZTIMER	(HWND hWnd, UINT id, UINT elapse, TIMERPROC proc);
void		_w_deleteZTIMER	(LPZTIMER zTimer);

TCHAR*		CharToTChar	(LPTSTR pRes, const char* pSrc);
char*		TCharToChar	(char* pRes, LPCTSTR pSrc);

#ifdef X11
DWORD 		_w_closestcolor	(DWORD r, DWORD g, DWORD b, int prefergray, Colormap map);
#endif
void 		_w_client_rect_offsets(HWND hWnd, LPRECT lpRect);

// resource.c
extern BOOL		__LoadResource(LPCTSTR resFile);
extern LPDWORD	__FindResource(WORD type, LPCTSTR rname, LPBYTE ressrc, DWORD ressize);

// button.c
extern void		__w_button_box(HDC hdc, int pushed, LPRECT rc);
extern void		__w_checkbox(HDC hdc, int pushed, int flat, int enabled, LPRECT rc);
extern void		__w_drawbutton(HDC hdc, HWND hWnd, DWORD style, LPCTSTR text, BOOL pushed, BOOL iscur);
extern LRESULT WINAPI __wproc_Button(HWND hWnd, UINT message, WPARAM wParam, LONG lParam);

// scroller.c
extern BOOL		__w_attach_scroller(HWND hwndParent, int type);
extern BOOL		__w_detach_scroller(HWND hwndBar);

// menu.c
extern LPZMENU	__menu_from_popup(HWND);
extern void 	__w_menukey(HMENU menu, WPARAM key);
extern int      _zg_menuTop;
extern void		__endpopups(int id);

// image.c
extern void		__set_wnd_cursor(HWND hWnd, HCURSOR hCursor);
extern LPZGDIOBJ _w_findGDIimage(int type, LPCTSTR id);
extern void		_w_deleteZBITMAP(LPZBITMAP zBitmap);
extern BOOL		_w_setIcon(LPZWND zWnd, HICON hIcon, LPCTSTR lpWindowName);

// iccm.c
extern HANDLE	_zg_clipboard;
extern HWND		_zg_clipowner;
extern int	 	_zg_clipformat;
extern BYTE*	_w_xclip_data(DWORD *len);
extern int 		_w_xclip_utf8_encode(BYTE* data, int datalen);
extern int 		_w_xclip_utf8_decode(BYTE* src, int datalen);
extern int 		_w_xclip_copy(BYTE* data, int datalen);
#ifdef X11
extern void		_w_clipSelectionRequest(XEvent event);
extern void		_w_clipSelectionNotify(XEvent event);
extern void		_w_clipSelectionClear(XEvent event);
extern void		_w_clipPropertyNotify(XEvent event);
#endif
extern char*	_w_clipGetClipboardData(int* lenret, UINT uFormat);
extern void		_w_clipSetClipboardData(char *data, int len, UINT uFormat);
extern void		_w_clipOwnClipboard(UINT format);
extern void		_w_clipDisownClipboard(void);
extern void		_w_clipInit(void);
extern void		_w_clipDeinit(void);

// window/os system driver
extern int 		_w_init(void);
extern void		_w_finish(void);
extern int 		_w_runSlice(int toms);
extern int 		_w_textExtents(LPZGC zGC, LPZFONT zFont, LPCTSTR text, int nText, int *width, int *height);
extern Font 	_w_createFont(LPCTSTR face, LPTEXTMETRIC ptm, LONG *ascent, LONG *descent);
extern int 		_w_enumFonts(FONTENUMPROC enumFunc, LONG lParam);
extern Cursor 	_w_createCursor(LPCTSTR std, LPZBITMAP zImage, int x, int y);
extern int		_w_mousePosition(int *x, int *y, int *buttons);
extern int 		_w_drawImage(LPZDRVR zDrvr, LPZBITMAP zImage, int x, int y, int w, int h);
extern void		_w_drawCaret(HDC hdc, LPZWND zWnd, LPRECT lprc, int onoff);
extern Window 	_w_getFocus(void);
extern int 		_w_setFocus(Window window);
extern HWND		_w_getScrollBar(HWND hwndParent, int type);
extern Window 	_w_getParent(Window window);
extern int 		_w_setParent(Window me, Window parent);
extern int 		_w_setTitle(Window window, const char *lpTitle);
extern int 		_w_setAppIcon(LPZWND zWnd, LPZBITMAP zIcon, LPCTSTR lpWindowName);
extern void		_w_setWindowAlpha(Window, BYTE alpha);
extern Window 	_w_createWindow(const char *title, DWORD dwStyle, HBRUSH bkg, int bkga,
							int x, int y, int w, int h, Window parent);
extern int 		_w_destroyWindow(Window window);
extern int 		_w_showWindow(Window window, int show);
extern int 		_w_redrawWindow(Window window, int x, int y, int w, int h, int erase);
extern int 		_w_moveWindow(Window window, int x, int y, int w, int h);
extern int 		_w_windowRect(Window window, Window *parent, LPRECT lprcc, unsigned *bw, unsigned *d);
extern int 		_w_clientRect(Window nw, LPRECT rcc);

// gdi/drawing driver

//x11drvr.c
extern LPZDRVR	_w_newX11DRVR(LPZGC zGC, LPZWND zWnd);

//cocoadrvr.c
extern LPZDRVR	_w_newCOCOADRVR(LPZGC zGC, LPZWND zWnd);

//pcldrvr.c
extern LPZDRVR	_w_newPCLDRVR(LPZGC zGC, LPZWND zWnd);

//psdrvr.c
extern LPZDRVR	_w_newPSDRVR(LPZGC zGC, LPZWND zWnd);

