
#include "winapix.h"

#ifdef X11
#ifdef XFT_FONT
	#define zXFTD(z) ((XftDraw*)((z)->drvrData[3]))
#else
	#define zXFTD(z) NULL
#endif
#include <X11/cursorfont.h>
#include "xkeycodes.h"

#define zXGC(z)  ((GC)((z)->drvrData[0]))
#define zZWND(z) ((LPZWND)((z)->drvrData[2]))
#define zCURX(z) ((long)((z)->drvrData[4]))
#define zCURY(z) ((long)((z)->drvrData[5]))

int g_xx, g_yx, g_zx, g_gotx = 0;

extern LPZWND _zg_lastFocus;

DWORD		_zg_eventmask;
DWORD		_zg_keymask;

Time		_zg_lb_time		= 0;
Time		_zg_mb_time		= 0;
Time		_zg_rb_time		= 0;

extern DWORD	_zg_White;
extern DWORD	_zg_Black;

extern Atom	wm_delete_window_atom;
extern Atom	wm_strin_zg_atom;
extern Atom	wm_length_atom;

LPBYTE		_zg_palette		= NULL;
int			_zg_palsize		= 0;

//**************************************************************************
void _x11_setfrg(LPZDRVR zDrvr, HBRUSH hbrFrg)
{
	XSetForeground(_zg_display, zXGC(zDrvr),
			((DWORD)_z_x_getbrushalpha(hbrFrg) << 24) | _z_x_colorofbrush(hbrFrg));
}

//**************************************************************************
void _x11_setbkg(LPZDRVR zDrvr,  HBRUSH hbrBkg)
{
	XSetBackground(_zg_display, zXGC(zDrvr),
			((DWORD)_z_x_getbrushalpha(hbrBkg) << 24) | _z_x_colorofbrush(hbrBkg));
}

//**************************************************************************
BOOL _x11_setFont(LPZDRVR zDrvr, HFONT hFont)
{
	LPZFONT		zFont;

	if(hFont)
	{
		zFont = (LPZFONT)(((LPZGDIOBJ)hFont)->obj);
		if(! zFont->isxft)
			XSetFont(_zg_display, zXGC(zDrvr), zFont->font->fid);
		return TRUE;
	}
	return FALSE;
}

//**************************************************************************
BOOL _x11_setPen(LPZDRVR zDrvr, HPEN hPen)
{
	LPZGDIOBJ zObj;
	LPZPEN	  zPen;
	int		  line_style, cap_style, join_style;
	int		  ndotpat;
	char*     pdotpat;

	static char dp_dots[] = { 1,1 };
	static char dp_dash[] = { 3,1 };
	static char dp_dashdot[] = { 3,1,1,1 };
	static char dp_dashdd[]  = { 3,1,1,1,1,1 };

	if(! zDrvr) return FALSE;
	if(! (zObj = (LPZGDIOBJ)hPen)) return FALSE;
	if(zObj->type != gdiPen)       return FALSE;
	if(! (zPen = (LPZPEN)zObj->obj)) return FALSE;

	switch(zPen->style)
	{
	default:
	case PS_SOLID:
		line_style = LineSolid;
		break;
	case PS_DASH:
	case PS_DOT:
	case PS_DASHDOT:
	case PS_DASHDOTDOT:
		line_style = LineOnOffDash;
		break;
	}
	switch(zPen->style)
	{
	default:
	case PS_SOLID:
		ndotpat = 0;
		break;
	case PS_DASH:
		ndotpat = 2;
		pdotpat = dp_dash;
		break;
	case PS_DOT:
		ndotpat = 2;
		pdotpat = dp_dots;
		break;
	case PS_DASHDOT:
		ndotpat = 2;
		pdotpat = dp_dashdot;
		break;
	case PS_DASHDOTDOT:
		ndotpat = 2;
		pdotpat = dp_dashdd;
		break;
	}
	cap_style = CapNotLast;
	join_style = JoinMiter;

	XSetLineAttributes(_zg_display, zXGC(zDrvr), zPen->width,
						line_style, cap_style, join_style);
	if(ndotpat)
		XSetDashes(_zg_display, zXGC(zDrvr), 0, pdotpat, ndotpat);
	return TRUE;
}

extern	HFONT _zg_sysFont;

//**************************************************************************
BOOL _x11_textOut(LPZDRVR zDrvr, int x, int y, LPCTSTR lpText, int nText)
{
	LPZFONT		zFont;
	int			h;

	if(! zDrvr)	      return FALSE;
	if(! zDrvr->zGC)  return FALSE;

	zFont = NULL;
	if(zDrvr->zGC->hFONT)
		zFont = (LPZFONT)(((LPZGDIOBJ)(zDrvr->zGC->hFONT))->obj);
	else
		zFont = (LPZFONT)(((LPZGDIOBJ)(_zg_sysFont))->obj);

	if(zFont)
	{
		if(! zFont->isxft)
		{
			LPSTR 		pAnsi;
#ifdef UNICODE
			if((pAnsi = (LPSTR)malloc(nText + sizeof(TCHAR))) != NULL)
			{
				int i;

				for(i = 0; i < nText; i++)
					pAnsi[i] = (char)lpText[i];
			}
			else
				return FALSE;
#else
			pAnsi = (LPSTR)lpText;
#endif
			if(! zFont)
				h = 13;
			else
				h = zFont->tm.tmAscent;

			y += h;
			XDrawImageString(_zg_display, zZWND(zDrvr)->xWnd, zXGC(zDrvr), x, y, pAnsi, nText);
#ifdef UNICODE
			free(pAnsi);
#endif
		}
		else
		{
			XftDraw* hDraw;

			hDraw = zXFTD(zDrvr);
			if(hDraw)
			{
				XftFont  	*hFont;
				XftColor	 frgColor, bkgColor;
				XRenderColor colorX;
				HBRUSH		 frg, bkg;
				LPZCOLOR     zbr;
				int rc;

				frg = zDrvr->zGC->textfrg;
				if(! frg)
				{
					frg = _zg_hbrBlack;
				}
				zbr = (LPZCOLOR)(((LPZGDIOBJ)frg)->obj);
				colorX.red   = (WORD)zbr->r << 8;
				colorX.green = (WORD)zbr->g << 8;
				colorX.blue  = (WORD)zbr->b << 8;
				colorX.alpha = (WORD)zbr->c << 8;

				rc = XftColorAllocValue(
								_zg_display,
								_zg_visual,
								XftDrawColormap(hDraw),
								&colorX,
								&frgColor
								);
				bkg = zDrvr->zGC->textbkg;
				if(! bkg)
				{
					bkg = _zg_hbrWhite;
				}
				zbr = (LPZCOLOR)(((LPZGDIOBJ)bkg)->obj);
				colorX.red   = (WORD)zbr->r << 8;
				colorX.green = (WORD)zbr->g << 8;
				colorX.blue  = (WORD)zbr->b << 8;
				colorX.alpha = (WORD)zbr->c << 8;

				rc = XftColorAllocValue(
								_zg_display,
								_zg_visual,
								XftDrawColormap(hDraw),
								&colorX,
								&bkgColor
								);
				hFont = zFont->xftfont;
				if(hFont)
				{
					SIZE txtext;

					h = zFont->tm.tmAscent;

					GetTextExtentPoint32(zDrvr->zGC, (LPTSTR)lpText, nText, &txtext);
					XftDrawRect(hDraw, &bkgColor, x, y, txtext.cx, txtext.cy);
					//_tprintf(_T("y=%d cy=%d s=%s\n"), y, txtext.cy, lpText);
					y += h;
	#ifdef UNICODE
					XftDrawString32(hDraw, &frgColor, hFont, x, y, (unsigned int*)lpText, nText);
	#else
					XftDrawString8(hDraw, &frgColor, hFont, x, y, lpText, nText);
	#endif
				}
				XftColorFree(_zg_display, _zg_visual, XftDrawColormap(hDraw), &frgColor);
				XftColorFree(_zg_display, _zg_visual, XftDrawColormap(hDraw), &bkgColor);
			}
		}
	}
	return TRUE;
}

//**************************************************************************
int _w_textExtents(LPZGC zGC, LPZFONT zFont, LPCTSTR lpText, int nText, int *width, int *height)
{
	char *texta;
	int	i;
	int	w, h;

	if(! zGC)  return FALSE;
	if(! zGC->zDrvr)  return FALSE;
	if(! lpText || (nText <= 0)) return FALSE;

	if(! zFont->isxft)
	{
		texta = (char*)malloc(nText + 2);

		if(! texta)
		{
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return FALSE;
		}
		if(nText > 0)
		{
			for(i = 0; i < nText; i++)
				texta[i] = (char)lpText[i];
		}
		else
		{
			i = 0;
			texta[i++] = 'm';
		}
		texta[i] = '\0';
	}
	if(! zFont->isxft)
	{
		int dir, ascent, descent;
		XCharStruct xchr;

		if(! zFont->font)
		{
			// cant happen ?
			h = zFont->tm.tmAscent + zFont->tm.tmDescent;
			w = h * nText * 7 / 8;
		}
		else
		{
			XTextExtents(zFont->font, texta, nText, &dir, &ascent, &descent, &xchr);
			w = (nText > 0) ? xchr.width : 0;
			h = ascent + descent;
		}
	}
	else
	{
		XGlyphInfo extents;

		if(! zFont->xftfont)
		{
			// cant happen ?
			h = zFont->tm.tmAscent + zFont->tm.tmDescent;
			w = h * nText * 7 / 8;
		}
		else
		{
	#ifdef UNICODE
			XftTextExtents32(_zg_display, zFont->xftfont, (unsigned int*)lpText, nText, &extents);
	#else
			XftTextExtents8(_zg_display, zFont->xftfont, lpText, nText, &extents);
	#endif
			//_tprintf(_T("tx=%c =%d nt=%d\n"), (char*)lpText[0], extents.xOff, nText);
			w = (nText > 0) ? extents.xOff : 0;
			h = zFont->tm.tmAscent + zFont->tm.tmDescent;
		}
	}
	if(! zFont->isxft)
	{
		free(texta);
	}
	if(width) *width = w;
	if(height) *height = h + X_TRA_PADDING;
	return 0;
}

//**************************************************************************
BOOL _x11_moveTo(LPZDRVR zDrvr, int x, int y)
{
	zDrvr->drvrData[4] = (LPVOID)(long)x;
	zDrvr->drvrData[5] = (LPVOID)(long)y;
	return TRUE;
}

//**************************************************************************
BOOL _x11_lineTo(LPZDRVR zDrvr, int x, int y)
{
	if(! zDrvr || ! zDrvr->zGC) return FALSE;

	XDrawLine(
			_zg_display,
			zDrvr->zGC->zWnd->xWnd,
			zXGC(zDrvr),
			zCURX(zDrvr),
			zCURY(zDrvr),
			x,
			y
			);
	zDrvr->drvrData[4] = (LPVOID)(long)x;
	zDrvr->drvrData[5] = (LPVOID)(long)y;
	return TRUE;
}

//**************************************************************************
int	 _x11_fillRect(LPZDRVR zDrvr, LPRECT lprc, HBRUSH hbrush)
{
	if(! zDrvr || ! zDrvr->zGC) return FALSE;

	if (_zg_compositing)
	{
		XftDraw* hDraw;

		hDraw = zXFTD(zDrvr);
		if(hDraw)
		{
			XftColor	 frgColor;
			XRenderColor colorX;
			LPZCOLOR     zbr;
			int rc;

			zbr = (LPZCOLOR)(((LPZGDIOBJ)hbrush)->obj);
			colorX.red   = (WORD)zbr->r << 8;
			colorX.green = (WORD)zbr->g << 8;
			colorX.blue  = (WORD)zbr->b << 8;
			colorX.alpha = (WORD)zbr->c << 8;

			rc = XftColorAllocValue(
							_zg_display,
							_zg_visual,
							XftDrawColormap(hDraw),
							&colorX,
							&frgColor
							);
			XftDrawRect(
								hDraw,
								&frgColor,
								lprc->left,
								lprc->top,
								lprc->right -  lprc->left + 1,
								lprc->bottom - lprc->top + 1
					  );
			XftColorFree(_zg_display, _zg_visual, XftDrawColormap(hDraw), &frgColor);
		}
	}
	else
	{
		XFillRectangle(
					_zg_display,
					zDrvr->zGC->zWnd->xWnd,
					zXGC(zDrvr),
					lprc->left,
					lprc->top,
					lprc->right -  lprc->left + 1,
					lprc->bottom - lprc->top + 1
					);
	}
	return 1;
}

//**************************************************************************
BOOL _x11_bitblt(LPZDRVR zDrvr, int x, int y, int w, int h, HDC hdcSrc, int sx, int sy, DWORD rop)
{
	return TRUE;
}

//**************************************************************************
int _x11_devCaps(LPZDRVR zDrvr, int cap)
{
	switch(cap)
	{
	case DRIVERVERSION: 	    // Device driver version
		return 1;
	case TECHNOLOGY:    	    // Device classification
		return DT_RASDISPLAY;
	case HORZSIZE:      	    // Horizontal size in millimeters
		return (zDrvr->vpw * 254) / (zDrvr->xres * 10);
	case VERTSIZE:      	    // Vertical size in millimeters
		return (zDrvr->vph * 254) / (zDrvr->yres * 10);
	case HORZRES:       	    // Horizontal width in pixels
		return zDrvr->vpw;
	case VERTRES:       	    // Vertical height in pixels
		return zDrvr->vph;
	case BITSPIXEL:     	    // Number of bits per pixel
		return g_zx;
	case PLANES:        	    // Number of planes
		return 1;
	case NUMBRUSHES:    	    // Number of brushes the device has
	case NUMPENS:       	    // Number of pens the device has
	case NUMMARKERS:    	    // Number of markers the device has
	case NUMFONTS:      	    // Number of fonts the device has
	case NUMCOLORS:     	    // Number of colors the device supports
		return 0;
	case PDEVICESIZE:   	    // Size required for device descriptor
		return 0;
	case CURVECAPS:     	    // Curve capabilities
	case LINECAPS:      	    // Line capabilities
	case POLYGONALCAPS: 	    // Polygonal capabilities
	case TEXTCAPS:      	    // Text capabilities
	case CLIPCAPS:      	    // Clipping capabilities
		return 0;
	case RASTERCAPS:    	    // Bitblt capabilities
		return 0xff;
	case ASPECTX:       	    // Length of the X leg
		return 4;
	case ASPECTY:       	    // Length of the Y leg
		return 3;
	case ASPECTXY:      	    // Length of the hypotenuse
		return 10;
	case LOGPIXELSX:   		    // Logical pixels/inch in X
		return BSA_DEVICE_LOGPIXELSX;
	case LOGPIXELSY:   		    // Logical pixels/inch in Y
		return BSA_DEVICE_LOGPIXELSY;
	case SIZEPALETTE:  		    // Number of entries in physical palette
		return 256;
	case NUMRESERVED:  		    // Number of reserved entries in palette
		return 100;
	case COLORRES:     		   	// Actual color resolution
		return 24;
	case PHYSICALWIDTH:   		// Physical Width in device units
		return zDrvr->vpw;
	case PHYSICALHEIGHT:  		// Physical Height in device units
		return zDrvr->vph;
	case PHYSICALOFFSETX: 		// Physical Printable Area x margin
		return zDrvr->xmar;
	case PHYSICALOFFSETY: 		// Physical Printable Area y margin
		return zDrvr->ymar;
	case SCALINGFACTORX:  		// Scaling factor x
	case SCALINGFACTORY:  		// Scaling factor y
		return 1;
	default:
		return -1;
	}
}

//**************************************************************************
void _w_deleteX11DRVR(LPZDRVR zDrvr)
{
	if(! zDrvr) return;
	if(zXGC(zDrvr))
		XFreeGC(_zg_display, zXGC(zDrvr));
	#ifdef XFT_FONT
	{
		XftDraw* hDraw = zXFTD(zDrvr);

		if(hDraw)
		{
			XftDrawDestroy(hDraw);
		}
	}
	#endif
	free(zDrvr);
}

//**************************************************************************
LPZDRVR _w_newX11DRVR(LPZGC zGC, LPZWND zWnd)
{
	LPZDRVR zDrvr;

	if(! zGC) return NULL;

	if(! (zDrvr = (LPZDRVR)malloc(sizeof(ZDRVR))))
		return NULL;
	memset(zDrvr, 0, sizeof(ZDRVR));
	zDrvr->zGC = zGC;

	if(! g_gotx)
	{
		int myX, myY;
		unsigned int myWidth, myHeight, myBorderWidth, myDepth;
		Window       myMommy /*, myRoot, *myKids */ ;

		XGetGeometry(
					_zg_display,
					(zWnd ? zWnd->xWnd : DefaultRootWindow(_zg_display)),
					&myMommy,
					&myX, &myY,
					&myWidth, &myHeight,
					&myBorderWidth,
					&myDepth
				  );

         g_gotx = 1;
         g_xx = myWidth;
         g_yx = myHeight;
         g_zx = myDepth;
	}
	zDrvr->yorigTop = 1; // x11 org is upper left

	zDrvr->xres = 75;
	zDrvr->yres = zDrvr->xres;
	zDrvr->xmar = 0;
	zDrvr->ymar = 0;
	zDrvr->vpw  = g_xx;
	zDrvr->vph  = g_yx;

	zDrvr->drvrData[1] = zGC;
	zDrvr->drvrData[2] = zWnd;

	// create an x11 gc
	zDrvr->drvrData[0] = (LPVOID)XCreateGC(_zg_display, (zWnd && zWnd->xWnd) ? zWnd->xWnd : DefaultRootWindow(_zg_display), 0, 0);

	// create xft draw context
	#ifdef XFT_FONT
	{
		XftDraw* hDraw;
		Colormap colormap;
		XWindowAttributes wattribs;
		int screen;

		screen = DefaultScreen(_zg_display);
		colormap = DefaultColormap(_zg_display, screen);

		// use window's colormap if different from default
		//
		if(XGetWindowAttributes(
								_zg_display,
								(zWnd && zWnd->xWnd) ? zWnd->xWnd : DefaultRootWindow(_zg_display),
								&wattribs
							)
		)
		{
			if(wattribs.colormap != None)
			{
				colormap = wattribs.colormap;
			}
		}
		hDraw = XftDrawCreate(
								_zg_display,
								(zWnd && zWnd->xWnd) ? zWnd->xWnd : DefaultRootWindow(_zg_display),
								_zg_visual,
								colormap
							  );
		zDrvr->drvrData[3] = (LPVOID)hDraw;
	}
	#endif
	zDrvr->_delete 	= _w_deleteX11DRVR;
	zDrvr->_setFrg 	= _x11_setfrg;
	zDrvr->_setBkg 	= _x11_setbkg;
	zDrvr->_setFont	= _x11_setFont;
	zDrvr->_setPen	= _x11_setPen;
	zDrvr->_textOut	= _x11_textOut;
	zDrvr->_moveTo	= _x11_moveTo;
	zDrvr->_lineTo	= _x11_lineTo;
	zDrvr->_fillRect= _x11_fillRect;
	zDrvr->_bitblt	= _x11_bitblt;
	zDrvr->_devCaps	= _x11_devCaps;

	return zDrvr;
}

//**************************************************************************
int _w_drawImage(LPZDRVR zDrvr, LPZBITMAP zImage, int x, int y, int w, int h)
{
	XPutImage(
					_zg_display,
					zDrvr->zGC->zWnd->xWnd,
					(GC)zDrvr->drvrData[0],
					zImage->ximg,
					0, 0,
					x, y,
					w, h
				);
	return 0;
}

//**************************************************************************
Cursor _w_createCursor(LPCTSTR std, LPZBITMAP zImage, int x, int y)
{
	int stdCursor;

	if (std == IDC_IBEAM)
	{
		stdCursor = XC_xterm;
	}
	else if (std == IDC_ARROW)
	{
		stdCursor = XC_left_ptr;
	}
	else
	{
		stdCursor = 0;
	}
	if(stdCursor)
	{
		Cursor cursor;

		cursor = XCreateFontCursor(
									_zg_display,
									stdCursor
									);
		return cursor;
	}
	return 0;
}

//**************************************************************************
Font _w_createFont(LPCTSTR face, LPTEXTMETRIC ptm, LONG *ascent, LONG *descent)
{
	return 0;
}

//**************************************************************************
int _w_enumFonts(FONTENUMPROC enumFunc, LONG lParam)
{
	return 0;
}

//**************************************************************************
int _w_mousePosition(int *x, int *y, int *buttons)
{
	return 0;
}

//**************************************************************************
void _w_drawCaret(HDC hdc, LPZWND zWnd, LPRECT lprc, int onoff)
{
	if(! zWnd) return;

	hdc = GetDC(zWnd);
	if(! hdc) return;

	XSetFunction(_zg_display, (GC)(((LPZGC)hdc)->zDrvr->drvrData[0]), GXequiv);
#if 1
	// avoid compositing, this need to plain-xor
	((ZGC*)hdc)->zDrvr->_setFrg(((ZGC*)hdc)->zDrvr, GetStockObject(BLACK_BRUSH));
	XFillRectangle(
				_zg_display,
				((ZGC*)hdc)->zWnd->xWnd,
				(GC)((ZGC*)hdc)->zDrvr->drvrData[0],
				lprc->left,
				lprc->top,
				lprc->right -  lprc->left + 1,
				lprc->bottom - lprc->top + 1
				);
	ReleaseDC(zWnd, hdc);
#else
	FillRect(hdc, &rc, GetStockObject(BLACK_BRUSH));
#endif
}

//**************************************************************************
int _w_setFocus(Window nw)
{
	XSetInputFocus(_zg_display, nw, RevertToParent, CurrentTime);
	return 0;
}

//**************************************************************************
Window _w_getFocus()
{
	Window focal;
	LPZWND zWnd;
	int	   revert;

	XGetInputFocus(_zg_display, &focal, &revert);
	if (focal != 0)
	{
	    zWnd = ZXwindow(focal);
	    //_tprintf(_T("get=%ls %p\n"), zWnd ? zWnd->class->name : _T("none"), zWnd);
		if (! zWnd)
			if(_zg_lastFocus)
				zWnd = _zg_lastFocus;
		if (zWnd)
			return zWnd->xWnd;
	}
	return 0;
}

extern LRESULT WINAPI __wproc_Scrollbar(HWND hWnd, UINT message, WPARAM wParam, LONG lParam);

//**************************************************************************
HWND _w_getScrollBar(HWND hwndParent, int type)
{
	LPZWND	zWnd;
	LPZSBAR	psc;
	int wc = 0;

	for(zWnd = _zg_windows; zWnd; zWnd = zWnd->next)
	{
		wc++;
		if(zWnd->class && ((void*)zWnd->class->proc == (void*)&__wproc_Scrollbar))
		{
			if(GetParent(zWnd) == hwndParent)
			{
				psc = (LPZSBAR)GetWindowLong(zWnd, GWL_USERDATA);

				if(psc && psc->tag == ZSBAR_TAG && psc->type == type)
				{
					return (HWND)zWnd;
				}
			}
		}
	}
	//printf("wc=%d\n", wc);
	return NULL;
}

//**************************************************************************
int _w_setParent(Window nw, Window nwparent)
{
	XReparentWindow(_zg_display, nw, nwparent, 0, 0);
	return 0;
}

//**************************************************************************
Window _w_getParent(Window nw)
{
	Window   root;
	Window   parent;
	Window*  kids;
	unsigned nKids;

	if(XQueryTree(
					_zg_display,
					nw,
					&root,
					&parent,
					&kids,
					&nKids
				  )
	)
	{
		if(kids) XFree(kids);
		return parent;
	}
	return 0;
}

//**************************************************************************
int _w_setTitle(Window nw, const char *title)
{
	return 0;
}

//**************************************************************************
int _w_windowRect(Window nw, Window *parent, LPRECT lprcc, unsigned *bw, unsigned *d)
{
	unsigned int myOrigWidth, myOrigHeight;
	unsigned int myBorderWidth, myDepth;
	int			 myX, myY;
	unsigned int myWidth, myHeight;
	Window 	     myMommy, myRoot;

	XGetGeometry(
				_zg_display,
				nw,
				&myRoot,
				&myX, &myY,
				&myWidth, &myHeight,
				&myBorderWidth,
				&myDepth
				);

	if(parent)
		*parent = myRoot;
	if(bw)
		*bw = myBorderWidth;
	if(myDepth)
		*d = myDepth;

	myOrigWidth  = myWidth;
	myOrigHeight = myHeight;

	lprcc->left 	= myX;
	lprcc->top 		= myY;
	lprcc->right 	= myX + myWidth;
	lprcc->bottom 	= myY + myHeight;

	//printf("\nST %d,%d %d,%d\n", lprcc->top, lprcc->left, lprcc->right-lprcc->left, lprcc->bottom-lprcc->top);

	Window* 		myKids;
	unsigned int	numKids;

	myMommy = nw;

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
		if(myMommy == myRoot)
			break;
		if(myMommy)
		{
			XGetGeometry(
					_zg_display,
					myMommy,
					&myRoot,
					&myX, &myY,
					&myWidth, &myHeight,
					&myBorderWidth,
					&myDepth
					);
			//printf("parent=%d,%d %d,%d\n", myY, myX, myWidth, myHeight);
			//if(GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD)
			{
				// add offset of parent to child offset
				lprcc->left 	+= myX;
				lprcc->top 		+= myY;
				lprcc->right 	+= myX;
				lprcc->bottom 	+= myY;
			}
		}
	}
	while(myMommy);

	if(0)
	{
		LPZWND zWnd = ZXwindow(nw);

		if(! (GetWindowLong(zWnd, GWL_STYLE) & WS_CHILD))
		{
			// parentless (top level) windows get position
			// of largest decoration but retain original size

			lprcc->left 	= myX;
			lprcc->top 		= myY;
			lprcc->right 	= myX + myOrigWidth;
			lprcc->bottom 	= myY + myOrigHeight;
		}
	}
	//printf("wr=%d,%d %d,%d\n", myY, myX, myOrigWidth, myOrigHeight);
	//printf("IS %d,%d %d,%d\n", lprcc->top, lprcc->left, lprcc->right-lprcc->left, lprcc->bottom-lprcc->top);
	return 0;
}

//**************************************************************************
int _w_clientRect(Window nw, LPRECT lprcc)
{
	int				myX, myY;
	unsigned int	myWidth, myHeight, myBorderWidth, myDepth;
	Window 			myMommy /*, myRoot, *myKids */ ;
	Status			status;
	
	status = XGetGeometry(
				_zg_display,
				nw,
				&myMommy,
				&myX, &myY,
				&myWidth, &myHeight,
				&myBorderWidth,
				&myDepth
				);

	if (! status) 
	{
		myWidth = myHeight = 0;
		myBorderWidth = 0;
	}
#if defined(Darwin) || defined(OSX)
	// x11 on mac/darwin gives 61k for border!!
	if(myBorderWidth > 4)
		myBorderWidth = 0;
#endif
	lprcc->top 		= 0;
	lprcc->left 	= 0;
	lprcc->right 	= myWidth - 2 * myBorderWidth;
	lprcc->bottom 	= myHeight - 2 * myBorderWidth;
	return 0;
}

//**************************************************************************
int _w_redrawWindow(Window nw, int x, int y, int w, int h, int erase)
{
	return 0;
}

//**************************************************************************
int _resetAttachedViews(LPZWND zWnd)
{
	if(zWnd)
	{
		DWORD dwStyle = GetWindowLong(zWnd, GWL_STYLE);
		HWND  hwndScroll;

		if(zWnd->menu && zWnd->menu->zWnd)
			PostMessage(zWnd->menu->zWnd, WM_SIZE, 0, 0);

		if(dwStyle & WS_VSCROLL)
			if((hwndScroll = _w_getScrollBar(zWnd, SB_VERT)) != NULL)
				PostMessage(hwndScroll, WM_SIZE, 0, 0);
		if(dwStyle & WS_HSCROLL)
			if((hwndScroll = _w_getScrollBar(zWnd, SB_HORZ)) != NULL)
				PostMessage(hwndScroll, WM_SIZE, 0, 0);
	}
	return 0;
}

//**************************************************************************
int _w_showWindow(Window nw, int show)
{
	if(show)
	{
		XMapRaised(_zg_display, nw);
		XMapSubwindows(_zg_display, nw);
	}
	else
	{
		XUnmapWindow(_zg_display, nw);
	}
	return 0;
}

//**************************************************************************
int _w_moveWindow(Window nw, int x, int y, int w, int h)
{
	XMoveResizeWindow(_zg_display, nw, x, y, w, h);
	return 0;
}

//**************************************************************************
int _w_destroyWindow(Window nw)
{
	XUnmapWindow(_zg_display, nw);
	XDestroyWindow(_zg_display, nw);
	XFlush(_zg_display);
	return 0;
}

//**************************************************************************
Window _w_createWindow(const char *title, DWORD dwStyle, HBRUSH bkg, int bkgalpha, int x, int y, int w, int h, Window parent)
{
	XSetWindowAttributes xAttribs;
	XSizeHints			 xHint;
	Window				 xWnd;
	int					 noFrame;
	LPCSTR				 papp;
	char 				 aIconName[256];

	noFrame = (/*hWndParent != NULL &&*/ ! (dwStyle & (WS_SYSMENU | WS_CAPTION))) || (dwStyle & WS_CHILD);

	xAttribs.background_pixmap  = None;
	xAttribs.background_pixel	= _z_x_colorofbrush(bkg);
	xAttribs.border_pixmap 		= CopyFromParent;
	if(noFrame && ! (dwStyle & WS_BORDER))
		xAttribs.border_pixel = _z_x_colorofbrush(bkg);
	else
		xAttribs.border_pixel = _z_x_colorofbrush(_zg_hbrBlack);

	xAttribs.bit_gravity 			= ForgetGravity;
	xAttribs.win_gravity 			= NorthWestGravity;
	xAttribs.backing_store 			= WhenMapped;
	xAttribs.backing_planes 		= 0xFFFFFFFFL;
	xAttribs.backing_pixel 			= 0L;
	xAttribs.save_under				= 0;
	xAttribs.event_mask 			= _zg_eventmask;
	xAttribs.do_not_propagate_mask 	= 0L;
	xAttribs.override_redirect 		= noFrame;
	xAttribs.colormap 				= CopyFromParent;
	xAttribs.cursor 				= None;

	if (_zg_compositing)
	{
		int alpha;

	    xAttribs.colormap = XCreateColormap(
	        _zg_display, XDefaultRootWindow(_zg_display), _zg_vinfo.visual, AllocNone
	    );
		// composite global alpha and window alpha
		alpha = bkgalpha;
		//alpha *= _zg_alpha;
		//alpha /= 255;
		xAttribs.background_pixel = (alpha << 24) | (_z_x_colorofbrush(bkg) & 0x00FFFFFF);
	}
	xHint.x 		= x;
	xHint.y 		= y;
	xHint.width 	= w;
	xHint.height 	= h;
	xHint.flags  	= PPosition | PSize;

	xWnd = XCreateWindow(
						_zg_display,
						noFrame ? parent : DefaultRootWindow(_zg_display),
						xHint.x,
						xHint.y,
						xHint.width  + (noFrame ? 1 : 0),
						xHint.height + (noFrame ? 1 : 0),
						0,
						_zg_compositing ?
							_zg_vinfo.depth :
							XDefaultDepth(_zg_display, _zg_screen),
						InputOutput,
						_zg_visual,
						CWEventMask | CWBitGravity | CWWinGravity | CWOverrideRedirect | CWBackingStore | CWBackPixel | CWColormap | CWBorderPixel,
						&xAttribs
						);

	if(xWnd == 0)
	{
		return 0;
	}
	for (papp = _zg_bsaappname + strlen(_zg_bsaappname) - 1; papp >= _zg_bsaappname; papp--)
		if (*papp == '/' || *papp == '\\')
			break;
	papp++;
	strcpy(aIconName, papp);

	if(! noFrame)
	{
		XSetStandardProperties(
								_zg_display,
								xWnd,
								title,
								aIconName,
								None,
								_zg_bsaappargv,
								_zg_bsaappargc,
								&xHint
							 );

		if(dwStyle & WS_BORDER)
			XSetWindowBorderWidth(_zg_display, xWnd, 1);
		else
			XSetWindowBorderWidth(_zg_display, xWnd, 0);
	}
	if(! noFrame)
		XSetWMProtocols(_zg_display, xWnd, &wm_delete_window_atom, 1L);
	return xWnd;
}

//**************************************************************************
int _w_setAppIcon(LPZWND zWnd, LPZBITMAP zIcon, LPCTSTR lpWindowName)
{
	int			w, h, d, i;
	char		aWindowName[256];
	Pixmap 		iconPixmap;
	GC 			iconGC;

	w = zIcon->w;
	h = zIcon->h;
	d = zIcon->d;
	/*
	d = _zg_compositing ?
				_zg_vinfo.depth :
				DefaultDepth(_zg_display, DefaultScreen(_zg_display));
	*/
	for(i = 0; lpWindowName && i < 255; i++)
		if(! (aWindowName[i] = lpWindowName[i]))
			break;
	aWindowName[i] = '\0';

	if(d == 32 && _zg_compositing)
	{
	    Atom net_wm_icon = XInternAtom(_zg_display, "_NET_WM_ICON", False);
	    Atom cardinal = XInternAtom(_zg_display, "CARDINAL", False);

		// copy icon bytes
		//
		int x, y, s;

		s = (w * d + 7) / 8;
		unsigned long *pb = (unsigned long *)malloc((w * h + 2) * sizeof(unsigned long));
		unsigned int *ps = (unsigned int *)zIcon->data;

		for(y = 0; y < h; y++)
		{
			for(x = 0; x < w; x++)
			{
				pb[2 + x + y * w] = ps[x + y * s/4];
			}
		}
		pb[0] = w;
		pb[1] = h;

	    XChangeProperty(_zg_display, zWnd->xWnd, net_wm_icon, cardinal, 32,
	                     PropModeReplace, (const unsigned char*)pb, 2 + (w * h));
	}
	else
	{
		// create a pixmap to draw the Icon into
		iconPixmap = XCreatePixmap(
									_zg_display,
									DefaultRootWindow(_zg_display), //zWnd->xWnd,
									w,
									h,
									d
								  );

		// create a GC to draw in
		iconGC = XCreateGC(
									_zg_display,
									iconPixmap, 0, 0
						  );

		// draw the icon into the pixmap
		XPutImage(
									_zg_display,
									iconPixmap,
									iconGC,
									zIcon->ximg,
									0, 0, 0, 0,
									w, h
				);
		// tell window manager here's our icon
		XSetStandardProperties(
									_zg_display,
									zWnd->xWnd,
									aWindowName,
									aWindowName,
									iconPixmap,
									NULL,
									0,
									NULL
								);
	}
	return 0;
}


//**************************************************************************
static int x11err(Display *d, XErrorEvent *e)
{
	return 0;
}

//**************************************************************************
static int x11ioerr(Display *d)
{
	return -1;
}

//**************************************************************************
void SigIgnore(int sig)
{
	return;
}

//**************************************************************************
void _w_read_palette()
{
	Visual*  v;
	Colormap map;
	XColor 	 xcolor;
	Status   stat;
	int	  	 i, d;
	int		 r, g, b;

	i = DisplayCells(_zg_display, _zg_screen);
	d = DefaultDepth(_zg_display, _zg_screen);
	v = _zg_visual;

	map  = DefaultColormap(_zg_display, _zg_screen);

	_zg_palsize = 0;
	if(v->class == PseudoColor || v->class == GrayScale)
		_zg_palsize = i;

	if(! _zg_palsize)
		return;
	if(_zg_palette)
		free(_zg_palette);
	_zg_palette = (LPBYTE)malloc(_zg_palsize * 3);

	for(i = 0; i < _zg_palsize; i++)
	{
		// get colormap entry
		xcolor.pixel = i;
		stat = XQueryColor(_zg_display, map, &xcolor);

		if(stat)
		{
			r = xcolor.red 		>> 8;
			g = xcolor.green 	>> 8;
			b = xcolor.blue 	>> 8;
			//_tprintf(_T("entry %d = %d,%d,%d\n"), i, r, g, b);
		}
		else
		{
			r = g = b = 0;
		}
		_zg_palette[i*3] 	= r;
		_zg_palette[i*3+1] 	= g;
		_zg_palette[i*3+2] 	= b;
	}
}

//**************************************************************************
DWORD _w_closestcolor(DWORD r, DWORD g, DWORD b, int preferGray, Colormap map)
{
	DWORD  xr, xg, xb;
	DWORD  mr, mg, mb;
	DWORD  diff, mindiff;
	int	i, mindex;
	BOOL   gray = (r == g)&&(g == b);
	BOOL   xgray;

	for(i = mindex = 0, mindiff = 0x7ffffff; i < _zg_palsize; i++)
	{
		xr = _zg_palette[i*3];
		xg = _zg_palette[i*3+1];
		xb = _zg_palette[i*3+2];

		if(xr > r) mr = xr - r;
		else  	 mr = r  - xr;
		if(xg > g) mg = xg - g;
		else  	 mg = g  - xg;
		if(xb > b) mb = xb - b;
		else  	 mb = b  - xb;
		/*
		mr *= r;
		mg *= g;
		mb *= b;
		*/
		xgray = xr == xg && xg == xb;
		diff = mr + mg + mb;

		if(diff < mindiff && (gray == xgray || preferGray))
		{
			mindiff = diff;
			mindex  = i;
		}
		#if 0
		_tprintf(_T(" Want:%d,%d,%d read:%d,%d,%d for pel:%d  e=%d me=%d min=%d\n"),
				r,g,b,
				xr, xg, xb,
				i,
				diff, mindiff, mindex
			    );
		#endif
	}
	return mindex;
}

//**************************************************************************
int _w_init(void)
{
	// open display
	//
	_zg_display = XOpenDisplay(NULL);
	if(! _zg_display)
	{
		PANIC("Can't open X display");
		return 1;
	}
	_zg_screen = DefaultScreen(_zg_display);

	_zg_zWndRoot->xWnd = DefaultRootWindow(_zg_display);
	/*
	 * Grab the error function so X wont kill us
	 */
#if 1
	XSetErrorHandler(x11err);
	XSetIOErrorHandler(x11ioerr);
#endif
	/*
	 * handle broken pipe signal
	 */
	signal(SIGPIPE, SigIgnore);

	_zg_eventmask =	ExposureMask 		| FocusChangeMask 	|
					ButtonPressMask 	| ButtonReleaseMask |
					PointerMotionMask 	| EnterWindowMask 	| LeaveWindowMask |
					KeyPressMask 		| KeyReleaseMask 	| KeymapStateMask |
					PropertyChangeMask 	|
					StructureNotifyMask;

	/*
	 * set nonblocking io on x11 socket
	fd = ConnectionNumber(_zg_display);
	rv = 1;
	ioctl(fd, FIONBIO, &rv);
	 */

	wm_delete_window_atom = XInternAtom (_zg_display, "WM_DELETE_WINDOW", False);
	wm_strin_zg_atom = XInternAtom (_zg_display, "STRING", False);
	wm_length_atom = XInternAtom (_zg_display, "LENGTH", False);

	_zg_compositing = 0;
	_zg_visual = DefaultVisual(_zg_display, _zg_screen);

	// check for XComposite extension
	//
	// probe visual to check for 32 bit argb
	//
	if (XMatchVisualInfo(_zg_display, XDefaultScreen(_zg_display), 32, TrueColor, &_zg_vinfo))
	{
		// A 32 bit visual, use ARGB
		//
		/*
		static const char *vic_name[] = {
			"StaticGray", "GrayScale", "StaticColor",
			"PseudoColor", "TrueColor", "DirectColor"
		};
		printf("Using visual=%ld, class=%s, depth=%d\n",
			_zg_vinfo.visualid,
			vic_name[_zg_vinfo.class],
			_zg_vinfo.depth
		);
		*/
		_zg_visual = _zg_vinfo.visual;
		_zg_compositing = 1;
	}
	// make basic system colors
	//
	_w_read_palette();

	_zg_White = WhitePixel(_zg_display, _zg_screen);
	_zg_Black = BlackPixel(_zg_display, _zg_screen);

	return 0;
}

//**************************************************************************
int _w_runSlice(int toms)
{
	LPZWND zWnd;
	int nq;
	int message, wParam;
	long lParam;

	int			bx, by;
	int			sv;

	XEvent 		xEvent;
	XEvent 		xNextEvent;
	KeySym 		aKey;
	DWORD		newkeymask;

	int			dodbl;

	nq = XEventsQueued(_zg_display, QueuedAfterReading);
	if(nq < 0)
		nq = 0;
	if(nq == 0)
	{
		struct   timeval timeout;
		int 	 fd = ConnectionNumber(_zg_display);
		fd_set   rfds;

		XFlush(_zg_display);

		FD_ZERO (&rfds);
		FD_SET  (fd, &rfds);
		timeout.tv_sec  = 0;
		timeout.tv_usec = toms * 1000;

		nq = select(fd + 1, &rfds, NULL, NULL, &timeout);

		__w_toggle_caret();
	}
	if(nq <= 0)
		return nq;

	XNextEvent(_zg_display, &xEvent);
	//		XPeekEvent(_zg_display, &xEvent);

	switch(xEvent.type)
	{
	case Expose:

		// add the exposure and paint the window
		//
		zWnd = ZXwindow(xEvent.xexpose.window);
		if(zWnd)
		{
			RECT rcExpose;

			rcExpose.left 	= xEvent.xexpose.x;
			rcExpose.top 	= xEvent.xexpose.y;
			rcExpose.right  = xEvent.xexpose.x + xEvent.xexpose.width;
			rcExpose.bottom = xEvent.xexpose.y + xEvent.xexpose.height;

			UnionRect(&zWnd->ps.rcPaint, &zWnd->ps.rcPaint, &rcExpose);

		//	_tprintf(_T("expose%d %d,%d\n"), xEvent.xexpose.count, rcExpose.top, rcExpose.bottom);
			if(xEvent.xexpose.count == 0)
			{
				PostMessage(zWnd, WM_PAINT, 0, 0);
			}
		}
		break;

	case ClientMessage:

		if(xEvent.xclient.data.l[0] == wm_delete_window_atom)
		{
			zWnd = ZXwindow(xEvent.xclient.window);
			if(zWnd)
			{
				PostMessage(zWnd, WM_CLOSE, 0, 0);
			}
		}
		break;

	case ConfigureNotify:

		zWnd = ZXwindow(xEvent.xconfigure.window);
		if(zWnd)
		{
			DWORD dwStyle = GetWindowLong(zWnd, GWL_STYLE);
			HWND  hwndScroll;

			PostMessage(zWnd, WM_SIZE, 0, 0);

			if(zWnd->menu && zWnd->menu->zWnd)
				PostMessage(zWnd->menu->zWnd, WM_SIZE, 0, 0);

			if(dwStyle & WS_VSCROLL)
				if((hwndScroll = _w_getScrollBar(zWnd, SB_VERT)) != NULL)
					PostMessage(hwndScroll, WM_SIZE, 0, 0);
			if(dwStyle & WS_HSCROLL)
				if((hwndScroll = _w_getScrollBar(zWnd, SB_HORZ)) != NULL)
					PostMessage(hwndScroll, WM_SIZE, 0, 0);
		}
		break;

	case DestroyNotify:

		zWnd = ZXwindow(xEvent.xdestroywindow.window);
		// dont re-destroy windows we killed ourselves
		if (Zwindow(zWnd))
			PostMessage(zWnd, WM_DESTROY, 0, 0);
		break;

	case KeyPress:
	case KeyRelease:

		zWnd = ZXwindow(xEvent.xkey.window);
		if(! zWnd)
			break;

		XLookupString(
						(XKeyEvent *)&xEvent,
						NULL,
						0,
						&aKey,
						0
					);

		#if 0
		_tprintf(_T("key=%08X state=%08X %c %ls\n"), aKey, xEvent.xkey.state, aKey,
				xEvent.type == KeyPress ? _T("Down") : _T("up"));
		#endif

		// set the async key state
		//
		newkeymask = xEvent.xkey.state & (ShiftMask | ControlMask | LockMask | Mod1Mask);
		if(newkeymask != _zg_keymask)
		{
			_zg_keymask = newkeymask;
		}
		if(aKey != 0)
		{
			lParam = 0;

			// map keysym to vk code
			//
			if(aKey & 0xFF00)
			{
				if(aKey > 0xFF00)
				{
					aKey &= 0xFF;

					if(aKey >= X11keyTableBase)
					{
						aKey -= X11keyTableBase;
						aKey  = X11keyTranslations[aKey];
					}
				}
				else if(aKey > 0xFE00)
				{
					aKey &= 0xFF;

					aKey -= X119995keyTableBase;
					aKey  = X119995keyTranslations[aKey];
				}
				else
				{
					aKey &= 0xFF;
				}
				// map key-pad VK codes to shifted codes if shift
				// key is down (X11 doesn't do this sometimes but
				// does apply shift to other keys.  If it does do
				// the shift, then these keycodes wont be seen so
				// no harm done
				//
				if(newkeymask & ShiftMask && ! (newkeymask & LockMask))
				{
					switch(aKey)
					{
					case VK_NUMPAD0:	aKey = VK_INSERT;   break;
					case VK_NUMPAD1:	aKey = VK_END;  	break;
					case VK_NUMPAD2:	aKey = VK_DOWN; 	break;
					case VK_NUMPAD3:	aKey = VK_NEXT; 	break;
					case VK_NUMPAD4:	aKey = VK_LEFT; 	break;
					case VK_NUMPAD5:	break;
					case VK_NUMPAD6:	aKey = VK_RIGHT;	break;
					case VK_NUMPAD7:	aKey = VK_HOME; 	break;
					case VK_NUMPAD8:	aKey = VK_UP;   	break;
					case VK_NUMPAD9:	aKey = VK_PRIOR;	break;
					default:
						break;
					}
				}
				// Windows uses bits 25-28 in lparam to indicate that
				// this is a VK code key and not a regular key, I didn't
				// decode its undocumented functioning, but this works ok
				//
				lParam |= 0x04000000;
			}
			else if(newkeymask & ControlMask)
			{
				aKey &= 0x1F;
			}
			if(aKey != keyIllegal)
			{
				if(newkeymask & Mod1Mask)
				{
					message = xEvent.type == KeyPress ? WM_SYSKEYDOWN : WM_SYSKEYUP;
					if(aKey >= 'a' && aKey <= 'z')
						aKey = aKey - 'a' + 'A';
				}
				else
				{
					message = xEvent.type == KeyPress ? WM_KEYDOWN : WM_KEYUP;
				}
				wParam  = aKey;
			}
			PostMessage(zWnd, message, wParam, lParam);
		}
		break;

	case MappingNotify:

		XRefreshKeyboardMapping((XMappingEvent *)&xEvent);
		break;

	case FocusIn:

		if(
			    xEvent.xfocus.detail == NotifyInferior
            ||  xEvent.xfocus.detail == NotifyAncestor
            ||  xEvent.xfocus.detail == NotifyNonlinear
		)
		{
			zWnd = ZXwindow(xEvent.xfocus.window);
			if(zWnd)
			{
				// if there is a modal dialog, only send message if
				// window is, or is a child of top dialog, else reshow dialog
				//
				if(_zg_dialogTop >= 0)
				{
					LPZWND cv, dw = _zg_dialogWnd[_zg_dialogTop];

					for(cv = zWnd; cv; cv = Zwindow(GetParent(cv)))
						if(cv == dw)
							break;
					if(! cv)
					{
						ShowWindow(_zg_dialogWnd[_zg_dialogTop], SW_SHOW);
						return 0;
					}
				}
				if(_zg_menuTop >= 0)
				{
					ShowWindow(_zg_menuWnd[_zg_menuTop], SW_SHOW);
					return 0;
				}
				if(zWnd != _zg_lastFocus)
				{
					//_tprintf(_T("X11 set=%ls %p\n"), zWnd->class->name, zWnd);
					if(_zg_lastFocus)
						SendMessage(_zg_lastFocus, WM_KILLFOCUS, 0, 0);
					_zg_lastFocus = zWnd;
					SendMessage(zWnd, WM_SETFOCUS, 0, 0);
				}
			}
		}
		break;

	case FocusOut:

		zWnd = ZXwindow(xEvent.xfocus.window);
		if(
				xEvent.xfocus.detail == NotifyInferior ||
				xEvent.xfocus.detail == NotifyAncestor ||
				xEvent.xfocus.detail == NotifyNonlinear
		)
		{
			if(zWnd && zWnd == _zg_lastFocus)
			{
				// this is kind-of optional since I keep track of internal focus
				// so if this gets in the way, only do this for top-level windows
				SendMessage(zWnd, WM_KILLFOCUS, 0, 0);
				_zg_lastFocus = NULL;
			}
		}
		break;

	case ButtonPress:

		bx = xEvent.xbutton.x;
		by = xEvent.xbutton.y;

		if(_zg_hwndMouse)
			zWnd = _zg_hwndMouse;
		else
			zWnd = ZXwindow(xEvent.xbutton.window);
		if(! zWnd)
			break;
		lParam = (by << 16) | (bx & 0xffff);
		wParam  = 0;
		if(xEvent.xbutton.state & Button1Mask)
			wParam |= MK_LBUTTON;
		if(xEvent.xbutton.state & Button2Mask)
			wParam |= MK_MBUTTON;
		if(xEvent.xbutton.state & Button3Mask)
			wParam |= MK_RBUTTON;
		dodbl = 0;
		switch(xEvent.xbutton.button)
		{
		case Button1:
			message = WM_LBUTTONDOWN;
			if((xEvent.xbutton.time - _zg_lb_time) < BSA_MOUSE_DBLCLK_MS)
			{
				dodbl = 1;
				_zg_lb_time = 0;
			}
			else
				_zg_lb_time = xEvent.xbutton.time;
			break;
		case Button2:
			message = WM_MBUTTONDOWN;
			_zg_mb_time = xEvent.xbutton.time;
			break;
		case Button3:
			message = WM_RBUTTONDOWN;
			_zg_rb_time = xEvent.xbutton.time;
			break;
		case Button4:
		case Button5:
			wParam |= (((xEvent.xbutton.button == Button4) ? WHEEL_DELTA : -WHEEL_DELTA) << 16);
			message = WM_MOUSEWHEEL;
			_zg_rb_time = xEvent.xbutton.time;
			break;
		default:
			message = -1;
			break;
		}
		if(message > 0)
		{
			PostMessage(zWnd, message, wParam, lParam);
			if(dodbl)
				PostMessage(zWnd, WM_LBUTTONDBLCLK, wParam, lParam);
		}
		break;

	case ButtonRelease:

		bx = xEvent.xbutton.x;
		by = xEvent.xbutton.y;


		if(_zg_hwndMouse)
			zWnd = _zg_hwndMouse;
		else
			zWnd = ZXwindow(xEvent.xbutton.window);
		if(! zWnd)
			break;
		lParam = (by << 16) | (bx & 0xffff);
		wParam  = 0;
		if(xEvent.xbutton.state & Button1Mask)
			wParam |= MK_LBUTTON;
		if(xEvent.xbutton.state & Button2Mask)
			wParam |= MK_MBUTTON;
		if(xEvent.xbutton.state & Button3Mask)
			wParam |= MK_RBUTTON;

		switch(xEvent.xbutton.button)
		{
		case Button1:
			message = WM_LBUTTONUP;
			break;
		case Button2:
			message = WM_MBUTTONUP;
			break;
		case Button3:
			message = WM_RBUTTONUP;
			break;
		case Button4:
		case Button5:
			/* only wheel on down.
			wParam |= (((xEvent.xbutton.button == Button4) ? WHEEL_DELTA : -WHEEL_DELTA) << 16);
			message = WM_MOUSEWHEEL;
			*/
			message = -1;
			break;
		default:
			message = -1;
			break;
		}
		if(message > 0)
			PostMessage(zWnd, message, wParam, lParam);
		break;

	case MotionNotify:

		while(XEventsQueued(_zg_display, QueuedAfterReading) > 0)
		{
			XPeekEvent(_zg_display, &xNextEvent);
			if(xNextEvent.type != MotionNotify)
				break;
			if(xNextEvent.xmotion.window != xEvent.xmotion.window)
				break;
			XNextEvent(_zg_display, &xEvent);
		}
		bx = xEvent.xbutton.x;
		by = xEvent.xbutton.y;

		if(_zg_hwndMouse)
		{
			if(_zg_hwndMouse != ZXwindow(xEvent.xbutton.window))
			{
				RECT rcm, rco;

				// mouse outside client window but we have capture and mousemove
				// messages are relative to windows client area
				//
				GetWindowRect(_zg_hwndMouse, &rcm);
				_w_client_rect_offsets(_zg_hwndMouse, &rco);
				bx = xEvent.xbutton.x_root - rcm.left - rco.left;
				by = xEvent.xbutton.y_root - rcm.top - rco.top;
			}
			zWnd = _zg_hwndMouse;
		}
		else
			zWnd = ZXwindow(xEvent.xbutton.window);
		if(! zWnd)
			break;
		message = WM_MOUSEMOVE;
		lParam  = (by << 16) | (bx & 0xffff);
		wParam  = 0;
		if(xEvent.xbutton.state & Button1Mask)
			wParam |= MK_LBUTTON;
		if(xEvent.xbutton.state & Button2Mask)
			wParam |= MK_MBUTTON;
		if(xEvent.xbutton.state & Button3Mask)
			wParam |= MK_RBUTTON;
		PostMessage(zWnd, message, wParam, lParam);
		break;

	case EnterNotify:


		if(xEvent.xcrossing.mode != NotifyNormal)
			break;

		if(! (zWnd = ZXwindow(xEvent.xcrossing.window)))
			break;
		break;

	case LeaveNotify:

		if(xEvent.xcrossing.mode != NotifyNormal)
			break;

		if(! (zWnd = ZXwindow(xEvent.xcrossing.window)))
			break;

		if(_zg_mouseTrack.hwndTrack == (HWND)zWnd)
		{
			if(_zg_mouseTrack.dwFlags & TME_LEAVE)
			{
				// leave cancels further tracking
				zWnd = _zg_mouseTrack.hwndTrack;
				_zg_mouseTrack.hwndTrack = NULL;
				PostMessage(zWnd, WM_MOUSELEAVE, 0, 0);
			}
		}
		break;

	case SelectionRequest:

		_w_clipSelectionRequest(xEvent);
		break;

	case SelectionNotify:

		_w_clipSelectionNotify(xEvent);
		break;

	case SelectionClear:

		_w_clipSelectionClear(xEvent);
		break;

	case PropertyNotify:

		_w_clipPropertyNotify(xEvent);
		break;

	default:
		break;
	}
	return 1;
}

//**************************************************************************
void _w_finish(void)
{
}


#endif
