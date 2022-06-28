#ifdef COCOA

#import <Cocoa/Cocoa.h>

#include "winapix.h"
#include "mackeycodes.h"


static int g_screenw, g_screenh, g_screend;
static int g_dech_top, g_dech_bot, g_dech;
static int g_decw;

static int g_indraw = 0;

extern LPZWND _zg_lastFocus;

#define zXGC(z)  ((NSGraphicsContext *)((z)->drvrData[0]))
#define zZWND(z) ((LPZWND)((z)->drvrData[2]))
#define zCURX(z) ((int)((z)->drvrData[4]))
#define zCURY(z) ((int)((z)->drvrData[5]))

CGColorSpaceRef space = NULL;
CGColorRef foreground = NULL, background = NULL;

static NSAutoreleasePool *pool;
static NSApplication *application;
static CGContextRef s_bitmapcontext;

@interface ZWindow : NSView {
NSTrackingArea *trackingArea;
unsigned keymods;
}
@property BOOL needsErase;
@property (assign, nonatomic) ZWindow *myParent;
@end

//**************************************************************************
void _cocoa_setfrg(LPZDRVR zDrvr, HBRUSH hbrFrg)
{
	LPZGDIOBJ zObj = (LPZGDIOBJ)hbrFrg;
	LPZCOLOR zcolor;
	CGFloat components[4];
	CGFloat fr, fg, fb, fa;

	if(! zObj || (zObj->tag != ZGDIO_TAG) || (zObj->type != gdiBrush))
		return;
	zcolor = ((LPZCOLOR)zObj->obj);
	fr = (CGFloat)zcolor->r / 255.0;
	fg = (CGFloat)zcolor->g / 255.0;
	fb = (CGFloat)zcolor->b / 255.0;
	fa = (CGFloat)zcolor->c / 255.0;

	if(! space)
		space = CGColorSpaceCreateDeviceRGB();

	if(foreground)
	{
		CFRelease(foreground);
		foreground = NULL;
	}
	components[0] = fr;
	components[1] = fg;
	components[2] = fb;
	components[3] = fa;

	foreground = CGColorCreate(space, components);
}

//**************************************************************************
void _cocoa_setbkg(LPZDRVR zDrvr,  HBRUSH hbrBkg)
{
	LPZGDIOBJ zObj = (LPZGDIOBJ)hbrBkg;
	LPZCOLOR zcolor;
	CGFloat components[4];
	CGFloat fr, fg, fb, fa;

	if(! zObj || (zObj->tag != ZGDIO_TAG) || (zObj->type != gdiBrush))
		return;
	zcolor = ((LPZCOLOR)zObj->obj);
	fr = (CGFloat)zcolor->r / 255.0;
	fg = (CGFloat)zcolor->g / 255.0;
	fb = (CGFloat)zcolor->b / 255.0;
	fa = (CGFloat)zcolor->c / 255.0;

	if(! space)
		space = CGColorSpaceCreateDeviceRGB();

	if(background)
	{
		CFRelease(background);
		background = NULL;
	}
	components[0] = fr;
	components[1] = fg;
	components[2] = fb;
	components[3] = fa;

	background = CGColorCreate(space, components);
}

//**************************************************************************
BOOL _cocoa_setFont(LPZDRVR zDrvr, HFONT hFont)
{
	LPZFONT		zFont;

	if(hFont)
	{
		zFont = (LPZFONT)(((LPZGDIOBJ)hFont)->obj);
		return TRUE;
	}
	return FALSE;
}

//**************************************************************************
BOOL _cocoa_setPen(LPZDRVR zDrvr, HPEN hPen)
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
//		line_style = LineSolid;
		break;
	case PS_DASH:
	case PS_DOT:
	case PS_DASHDOT:
	case PS_DASHDOTDOT:
//		line_style = LineOnOffDash;
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
//	cap_style = CapNotLast;
//	join_style = JoinMiter;

	return TRUE;
}

extern	HFONT _zg_sysFont;

//**************************************************************************
static int _fillRect(NSGraphicsContext *context, CGColorRef color, CGBlendMode blendMode,
		int x, int y, int w, int h)
{
	CGRect fr;

	if(! g_indraw)
	{
		printf("draw outside drawrect!\n");
		return 0;
	}
	fr.origin.x = (CGFloat)x;
	fr.origin.y = (CGFloat)y;
	fr.size.width = (CGFloat)w;
	fr.size.height = (CGFloat)h;
	CGContextRef cgcontext = [context graphicsPort];
	CGContextSetFillColorWithColor(cgcontext, color);
	CGContextSetBlendMode(cgcontext, blendMode);
	CGContextFillRect(cgcontext, fr);
	return 0;
}

//**************************************************************************
BOOL _cocoa_textOut(LPZDRVR zDrvr, int x, int y, LPCTSTR lpText, int nText)
{
	LPZFONT		zFont;
	int			h;

	if(! zDrvr)	      return FALSE;
	if(! zDrvr->zGC)  return FALSE;
	if(! lpText || (nText <= 0)) return FALSE;

	if(! g_indraw)
	{
		printf("draw outside drawrect!\n");
		return FALSE;
	}
	zFont = NULL;
	if(zDrvr->zGC->hFONT)
		zFont = (LPZFONT)(((LPZGDIOBJ)(zDrvr->zGC->hFONT))->obj);
	else
		zFont = (LPZFONT)(((LPZGDIOBJ)(_zg_sysFont))->obj);

	if(zFont)
	{
		CGContextRef context = [zXGC(zDrvr) graphicsPort];

		if (! context)
		{
			return FALSE;
		}
		CGContextSetFontSize(context, CTFontGetSize((CTFontRef)(zFont->font)));

		//_tprintf(_T("text %d,%d %ls\n"), x, y, lpText);

		// Set the text matrix.
		CGContextSetTextMatrix(context, CGAffineTransformMake(1.0,0.0, 0.0, -1.0, 0.0, 0.0));

		// Initialize a string.
		CFStringRef textString = CFStringCreateWithBytes(
									NULL,
									(const UInt8*)lpText,
							#ifdef UNICODE
									nText * sizeof(WCHAR),
									kCFStringEncodingUTF32LE,
							#else
									nText,
									kCFStringEncodingUTF8,
							#endif
									false
								);

		if (textString == NULL)
		{
			LPTSTR lpCopy;
			int i;

			// this happens when garbage unicode chars slip in, so sanitize it
			//
			lpCopy = (LPTSTR)malloc(nText * sizeof(TCHAR));
			if (! lpCopy)
			{
				return FALSE;
			}
			for (i = 0; i < nText; i++)
			{
				if (lpText[i] > 32767)
				{
					lpCopy[i] = (TCHAR)'?';
				}
				else
				{
					lpCopy[i] = lpText[i];
				}
			}
			textString = CFStringCreateWithBytes(
												NULL,
												(const UInt8*)lpCopy,
										#ifdef UNICODE
												nText * sizeof(WCHAR),
												kCFStringEncodingUTF32LE,
										#else
												nText,
												kCFStringEncodingUTF8,
										#endif
												false
											);
			free(lpCopy);
			if (textString == NULL)
			{
				return FALSE;
			}
		}
		// Create a mutable attributed string with a max length of 0.
		// The max length is a hint as to how much internal storage to reserve.
		// 0 means no hint.
		CFMutableAttributedStringRef attrString =
		         CFAttributedStringCreateMutable(kCFAllocatorDefault, 0);

		// Copy the textString into the newly created attrString
		CFAttributedStringReplaceString (attrString, CFRangeMake(0, 0), textString);

		// set color attributes for all chars in the string
		CFAttributedStringSetAttribute(attrString,
				CFRangeMake(0, nText),
				kCTForegroundColorAttributeName, foreground);

		// set font attribute
		CFAttributedStringSetAttribute(attrString,
				CFRangeMake(0, nText),
				kCTFontAttributeName, (CTFontRef)(zFont->font));

		CTLineRef line = CTLineCreateWithAttributedString(attrString);

		// fill enclosing rectangle with background color
		CGFloat lw, la, ld, ll;
		lw = CTLineGetTypographicBounds(line, &la, &ld, &ll);
		_fillRect(zXGC(zDrvr), background, kCGBlendModeNormal, x, y, (int)(lw + 0.9), (int)(la + ld + 0.8));
		CGContextSetTextPosition(context, x, y + zFont->tm.tmAscent);

		CTLineDraw(line, context);

		CFRelease(line);
		CFRelease(attrString);
		CFRelease(textString);
	}
	return TRUE;
}

//**************************************************************************
CGContextRef _createBitmapContext (int pixelsWide, int pixelsHigh)
{
    CGContextRef    context = NULL;
    CGColorSpaceRef colorSpace;
    void *          bitmapData;
    int             bitmapByteCount;
    int             bitmapBytesPerRow;

    bitmapBytesPerRow   = (pixelsWide * 4);
    bitmapByteCount     = (bitmapBytesPerRow * pixelsHigh);

    colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    bitmapData = calloc( bitmapByteCount, sizeof(uint8_t) );
    if (bitmapData == NULL)
    {
        return NULL;
    }
    context = CGBitmapContextCreate(bitmapData,
                                    pixelsWide,
                                    pixelsHigh,
                                    8,      // bits per component
                                    bitmapBytesPerRow,
                                    colorSpace,
                                    kCGImageAlphaPremultipliedLast);
    if (context == NULL)
    {
        free (bitmapData);
        return NULL;
    }
    CGColorSpaceRelease(colorSpace);

    return context;
}


//**************************************************************************
int _w_textExtents(LPZGC zGC, LPZFONT zFont, LPCTSTR lpText, int nText, int *width, int *height)
{
	if(! zGC)  return FALSE;
	if(! zGC->zDrvr)  return FALSE;
	if(! lpText || (nText <= 0)) return FALSE;

	*width = 10 * nText;
	*height = 10;

	if(zFont)
	{
		if(zGC->hFONT)
		{
			if(zFont != (LPZFONT)(((LPZGDIOBJ)(zGC->hFONT))->obj))
			{
				//printf("ntfont\n");
			}
		}
		CGContextRef context = [zXGC(zGC->zDrvr) graphicsPort];

		if (! context)
		{
			// when getting text extent outside of a draw operation
			// use a bitmap context to draw into
			//
			context = s_bitmapcontext;
		}
		if (! context)
		{
			return FALSE;
		}
        CGContextSetFontSize(context, CTFontGetSize((CTFontRef)(zFont->font)));

		// Set the text matrix.
		CGContextSetTextMatrix(context, CGAffineTransformMake(1.0,0.0, 0.0, -1.0, 0.0, 0.0));

		// Initialize a string.
		CFStringRef textString = CFStringCreateWithBytes(
									NULL,
									(const UInt8*)lpText,
							#ifdef UNICODE
									nText * sizeof(WCHAR),
									kCFStringEncodingUTF32LE,
							#else
									nText,
									kCFStringEncodingUTF8,
							#endif
									false
								);
		if (textString == NULL)
		{
			LPTSTR lpCopy;
			int i;

			// this happens when garbage unicode chars slip in, so sanitize it
			//
			lpCopy = (LPTSTR)malloc(nText * sizeof(TCHAR));
			if (! lpCopy)
			{
				return FALSE;
			}
			for (i = 0; i < nText; i++)
			{
				if (lpText[i] > 32767)
				{
					lpCopy[i] = (TCHAR)'?';
				}
				else
				{
					lpCopy[i] = lpText[i];
				}
			}
			textString = CFStringCreateWithBytes(
												NULL,
												(const UInt8*)lpCopy,
										#ifdef UNICODE
												nText * sizeof(WCHAR),
												kCFStringEncodingUTF32LE,
										#else
												nText,
												kCFStringEncodingUTF8,
										#endif
												false
											);
			free(lpCopy);
			if (textString == NULL)
			{
				return FALSE;
			}
		}
		#if 0
		CFMutableAttributedStringRef attrString =
		         CFAttributedStringCreateMutable(kCFAllocatorDefault, 0);

		CFAttributedStringReplaceString (attrString, CFRangeMake(0, 0), textString);
		#else
		CFMutableAttributedStringRef attrString =
		         CFAttributedStringCreate(kCFAllocatorDefault, textString, 0);
		#endif
		CFAttributedStringSetAttribute(attrString,
				CFRangeMake(0, nText),
				kCTFontAttributeName, (CTFontRef)(zFont->font));

		CTLineRef line = CTLineCreateWithAttributedString(attrString);

		CGFloat lw, la, ld, ll;
		lw = CTLineGetTypographicBounds(line, &la, &ld, &ll);

		if(width)
		{
			*width = (int)(lw + 0.5);
		}
		if(height)
		{
			*height = (int)(la + ld + 0.5);
		}
		//_tprintf(_T("text %d,%d ==%ls==\n"), *width, *height, lpText);
		CFRelease(line);
		CFRelease(attrString);
		CFRelease(textString);
	}
	return TRUE;
}

//**************************************************************************
BOOL _cocoa_moveTo(LPZDRVR zDrvr, int x, int y)
{
	zDrvr->drvrData[4] = (LPVOID)(long)x;
	zDrvr->drvrData[5] = (LPVOID)(long)y;
	return TRUE;
}

//**************************************************************************
BOOL _cocoa_lineTo(LPZDRVR zDrvr, int x, int y)
{
	if(! zDrvr || ! zDrvr->zGC) return FALSE;

	if(! g_indraw)
	{
		printf("draw outside drawrect!\n");
		return 0;
	}
	NSBezierPath* aPath = [NSBezierPath bezierPath];
	CGContextRef cgcontext = [zXGC(zDrvr) graphicsPort];
	CGContextSetFillColorWithColor(cgcontext, foreground);
	CGContextSetStrokeColorWithColor(cgcontext, foreground);

	[aPath moveToPoint:NSMakePoint(zCURX(zDrvr), zCURY(zDrvr))];
	[aPath lineToPoint:NSMakePoint(x, y)];
//	[aPath setLineJoinStyle:NSRoundLineJoinStyle];
	[aPath stroke];

	zDrvr->drvrData[4] = (LPVOID)(long)x;
	zDrvr->drvrData[5] = (LPVOID)(long)y;
	return TRUE;
}

//**************************************************************************
int	 _cocoa_fillRect(LPZDRVR zDrvr, LPRECT lprc, HBRUSH hbrush)
{
	LPZGDIOBJ zObj = (LPZGDIOBJ)hbrush;
	LPZCOLOR zcolor;
	CGFloat components[4];
	CGFloat fr, fg, fb, fa;
	CGColorRef color;

	if(! zObj || (zObj->tag != ZGDIO_TAG) || (zObj->type != gdiBrush))
		return -1;
	zcolor = ((LPZCOLOR)zObj->obj);
	fr = (CGFloat)zcolor->r / 255.0;
	fg = (CGFloat)zcolor->g / 255.0;
	fb = (CGFloat)zcolor->b / 255.0;
	fa = (CGFloat)zcolor->c / 255.0;

	if(! space)
		space = CGColorSpaceCreateDeviceRGB();

	components[0] = fr;
	components[1] = fg;
	components[2] = fb;
	components[3] = fa;

	color = CGColorCreate(space, components);

	_fillRect(zXGC(zDrvr), color, kCGBlendModeNormal, lprc->left, lprc->top,
			lprc->right - lprc->left + 1, lprc->bottom - lprc->top + 1);

	CFRelease(color);
	return 1;
}

//**************************************************************************
BOOL _cocoa_bitblt(LPZDRVR zDrvr, int x, int y, int w, int h, HDC hdcSrc, int sx, int sy, DWORD rop)
{
	return TRUE;
}

//**************************************************************************
int _w_drawImage(LPZDRVR zDrvr, LPZBITMAP zImage, int x, int y, int w, int h)
{
#if 1
	NSRect ri;
#else
	CGRect ri;
#endif
	if(! zImage || ! zImage->ximg)
		return -1;

	if(! g_indraw)
	{
		printf("draw outside drawrect!\n");
		return 0;
	}
	CGContextRef cgcontext = [zXGC(zDrvr) graphicsPort];
	CGImageRef cgImage = CGBitmapContextCreateImage((CGContextRef)zImage->ximg);

	ri.origin.x = (CGFloat)x;
	ri.origin.y = (CGFloat)y;
	ri.size.width = (CGFloat)w;
	ri.size.height = (CGFloat)h;
#if 1
	NSImage *iImage;
	iImage = [[NSImage alloc] initWithCGImage:cgImage size:NSZeroSize];

	[iImage drawInRect:ri];

	[iImage release];
#else
	CGContextDrawImage(cgcontext, ri, cgImage);
#endif
	CFRelease(cgImage);
	return 0;
}

//**************************************************************************
Cursor _w_createCursor(LPCTSTR std, LPZBITMAP zImage, int x, int y)
{
	if (std == IDC_ARROW)
		return (void*)[NSCursor arrowCursor];
	if (std == IDC_IBEAM)
		return (void*)[NSCursor IBeamCursor];
	if (std == IDC_WAIT)
		return (void*)[NSCursor IBeamCursor];
	if (std == IDC_CROSS)
		return (void*)[NSCursor crosshairCursor];
	if (std == IDC_SIZENS)
		return (void*)[NSCursor resizeUpDownCursor];
	if (std == IDC_SIZEWE)
		return (void*)[NSCursor resizeLeftRightCursor];
	return NULL;
}

//**************************************************************************
Font _w_createFont(LPCTSTR face, LPTEXTMETRIC ptm, LONG *ascent, LONG *descent)
{
	int nText, size;
	CGFloat fa, fd;
	TCHAR szFace[256];
	LPCTSTR weight, angle;

	if(ptm->tmWeight > FW_MEDIUM)
		weight = _T(" Bold");
	else
		weight = _T("");

	if(ptm->tmItalic)
		angle = _T(" Italic");
	else
		angle = _T("");

	/*
	if(0)
		setwidth = _T("semicondensed");
	else
		setwidth = _T("");
	*/
#ifdef UNICODE
	nText = _sntprintf(szFace, 256, _T("%ls%ls%ls"), face, weight, angle);
#else
	nText = _snprintf(szFace, 256, "%s%s%s", face, weight, angle);
#endif
	CFStringRef fontFace = CFStringCreateWithBytes(
								NULL,
								(const UInt8*)szFace,
						#ifdef UNICODE
								nText * sizeof(WCHAR),
								kCFStringEncodingUTF32LE,
						#else
								nText,
								kCFStringEncodingUTF8,
						#endif
								false
							);

	if(ptm->tmHeight < 0)				// char "em" height spec
		size = -ptm->tmHeight;
	else if(ptm->tmHeight == 0)			// default font size
		size = BSA_DEFAULT_FONTSIZE;
	else								// char cell height spec
		size = ptm->tmHeight;

	CTFontRef font = CTFontCreateWithName(fontFace, (double)size, NULL);

	/*
	CFStringRef fname = CTFontCopyFullName(font);
	char buffer[1024];
	if (CFStringGetCString(fname, buffer, 1024, kCFStringEncodingUTF8)) {
		_tprintf(_T("FONT %ls -> %s:%d\n"), szFace, buffer, size);
	}
	CFRelease(fname);
	*/
	fa = CTFontGetAscent(font);
	fd = CTFontGetDescent(font);
	if (ascent)
		*ascent = (int)(fa + 0.5);
	if (descent)
		*descent = (int)(fd + 0.5);
	return (Font*)font;
}

//**************************************************************************
int _w_enumFonts(FONTENUMPROC enumFunc, LONG lParam)
{
	NSArray *fonts =	[[NSFontManager sharedFontManager] availableFontFamilies];
	int i, listed;
	LOGFONT		logFont;
	TEXTMETRIC	textmex;

	listed = 0;
	for (i = 0; i < fonts.count; i++)
	{
		NSString *fontname = [fonts objectAtIndex:i];
		const char *face;
		int rv;

		if(fontname)
		{
			memset(&logFont, 0, sizeof(logFont));
			memset(&textmex, 0, sizeof(textmex));
			face = [fontname UTF8String];
			CharToTChar(logFont.lfFaceName, face);
			rv = enumFunc(&logFont, &textmex, TRUETYPE_FONTTYPE, lParam);
			if(! rv)
			{
				printf("callback returns 0\n");
				break;
			}
			listed++;
		}
	}
	return listed;
}

//**************************************************************************
int _w_mousePosition(int *x, int *y, int *buttons)
{
	NSRect screenSize = [[NSScreen mainScreen] frame];
	NSPoint loc = [NSEvent mouseLocation];

	if(x) *x = loc.x;
	if(y) *y = (screenSize.size.height - loc.y);
	return 0;
}

//**************************************************************************
void _w_drawCaret(HDC hdc, LPZWND zWnd, LPRECT lprc, int onoff)
{
	HBRUSH hbrush =	GetStockObject(BLACK_BRUSH);
	LPZGC zGC = (LPZGC)hdc;

	if (! zGC || ! zGC->zDrvr)
		return;

	if (! onoff)
		return;

#if	 1
	_cocoa_fillRect(zGC->zDrvr, lprc, hbrush);
#else
	NSGraphicsContext *context = zXGC(zGC->zDrvr);
//	[context saveGraphicsState];

	CGContextRef cgcontext = [context graphicsPort];

	LPZGDIOBJ zObj = (LPZGDIOBJ)hbrush;
	LPZCOLOR zcolor;
	CGFloat components[4];
	CGFloat fr, fg, fb, fa;
	CGColorRef color;

	if(! zObj || (zObj->tag != ZGDIO_TAG) || (zObj->type != gdiBrush))
		return -1;
	zcolor = ((LPZCOLOR)zObj->obj);
	fr = (CGFloat)zcolor->r / 255.0;
	fg = (CGFloat)zcolor->g / 255.0;
	fb = (CGFloat)zcolor->b / 255.0;
	fa = (CGFloat)zcolor->c / 255.0;

	if(! space)
		space = CGColorSpaceCreateDeviceRGB();

	components[0] = fr;
	components[1] = fg;
	components[2] = fb;
	components[3] = fa;

	color = CGColorCreate(space, components);

	_fillRect(zXGC(zGC->zDrvr), color, kCGBlendModeXOR, lprc->left, lprc->top,
			lprc->right - lprc->left + 1, lprc->bottom - lprc->top + 1);
	CFRelease(color);
//	[context restoreGraphicsState];
#endif
}

//**************************************************************************
int _cocoa_devCaps(LPZDRVR zDrvr, int cap)
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
		return g_screend;
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
void _w_deleteCOCOADRVR(LPZDRVR zDrvr)
{
	NSGraphicsContext *context;

	if(! zDrvr) return;
	// pop state
	context = zXGC(zDrvr);
	if (context)
		[context restoreGraphicsState];
	free(zDrvr);
}

//**************************************************************************
LPZDRVR _w_newCOCOADRVR(LPZGC zGC, LPZWND zWnd)
{
	LPZDRVR zDrvr;
	NSGraphicsContext *context;
	static NSGraphicsContext *s_context = NULL;

	if(! zGC) return NULL;

	if(! (zDrvr = (LPZDRVR)malloc(sizeof(ZDRVR))))
		return NULL;
	memset(zDrvr, 0, sizeof(ZDRVR));
	zDrvr->zGC = zGC;

	zDrvr->yorigTop = 1; // force cocoa to be org upper left

	zDrvr->xres = 75;
	zDrvr->yres = zDrvr->xres;
	zDrvr->xmar = 0;
	zDrvr->ymar = 0;
	zDrvr->vpw  = g_screenw;
	zDrvr->vph  = g_screenh;

	zDrvr->drvrData[1] = zGC;
	zDrvr->drvrData[2] = zWnd;

	// create a grahics context, using current, if inside a
	// paint message, or creating one
	//
	context = [NSGraphicsContext currentContext];

	// this bit is deprectated, I used a bitmap context where needed now
	#if 0
	if (context == NULL && zWnd)
	{
		NSView *nv = (ZWindow *)zWnd->xWnd;
		NSWindow *nw = [nv window];

		context = [NSGraphicsContext graphicsContextWithWindow:nw];
	}
	#endif
	zDrvr->drvrData[0] = (LPVOID)context;
	// push state
	[context saveGraphicsState];

	zDrvr->_delete 	= _w_deleteCOCOADRVR;

	zDrvr->_setFrg 	= _cocoa_setfrg;
	zDrvr->_setBkg 	= _cocoa_setbkg;
	zDrvr->_setFont	= _cocoa_setFont;
	zDrvr->_setPen	= _cocoa_setPen;
	zDrvr->_textOut	= _cocoa_textOut;
	zDrvr->_moveTo	= _cocoa_moveTo;
	zDrvr->_lineTo	= _cocoa_lineTo;
	zDrvr->_fillRect= _cocoa_fillRect;
	zDrvr->_bitblt	= _cocoa_bitblt;
	zDrvr->_devCaps	= _cocoa_devCaps;

	return zDrvr;
}

@implementation ZWindow : NSView
- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
	{
		//[self setHidden:YES];
 	    NSTrackingAreaOptions options =
					(NSTrackingActiveAlways | NSTrackingInVisibleRect |
                     NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved);
        trackingArea = [[NSTrackingArea alloc] initWithRect:[self visibleRect]
	            options:options
	            owner:self userInfo:nil];
        [self addTrackingArea:trackingArea];
		[[self window] setAcceptsMouseMovedEvents:YES];
		keymods = 0;
    }
    return self;
}

- (void)updateTrackingAreas
{
    [self removeTrackingArea:trackingArea];
    [trackingArea release];
	NSTrackingAreaOptions options = (NSTrackingActiveAlways | NSTrackingInVisibleRect |
                     NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved);
    trackingArea = [[NSTrackingArea alloc] initWithRect:[self visibleRect]
            options:options
            owner:self userInfo:nil];
    [self addTrackingArea:trackingArea];
}

- (BOOL)isFlipped
{
	return YES;
}

/*
- (BOOL)acceptsFirstMouse
{
	return YES;
}
*/

- (void)drawRect:(NSRect)dirtyRect
{
	LPZWND zWnd = ZXwindow(self);

	if (zWnd)
	{
		RECT rcd;
		WNDPROC  proc;
		long	 rv;
		int      fErase;

		rcd.left   = (int)dirtyRect.origin.x;
		rcd.top    = (int)dirtyRect.origin.y;
		rcd.right  = rcd.left + (int)dirtyRect.size.width;
		rcd.bottom = rcd.top  + (int)dirtyRect.size.height;
		fErase = [self needsErase] ? TRUE : FALSE;
		[self setNeedsErase:NO];
		InvalidateRect(zWnd, &rcd, fErase);

		// hand-call the window's proc directly, as
		// the message dispatcher just triggers this
		//
		// SendMessage((HWND)zWnd, WM_PAINT, 0, 0);

		proc = (WNDPROC)GetWindowLong(zWnd, GWL_WNDPROC);

		if(! proc || ! zWnd || ! zWnd->class)
			return;

		g_indraw = 1;
		rv = proc((HWND)zWnd, WM_PAINT, 0, 0);
		g_indraw = 0;
	}
}

- (void)mouseDown:(NSEvent *)theEvent
{
	NSPoint eventloc = [theEvent locationInWindow];
	NSPoint pt = [self convertPoint:eventloc fromView:nil];
	LPZWND   zWnd = ZXwindow(self);

	if (zWnd)
	{
		int bx = (int)pt.x;
		int by = (int)pt.y;
		int mods = [theEvent modifierFlags];
		int clicks = [theEvent clickCount];

		int  wParam  = MK_LBUTTON;
		LONG lParam = (by << 16) | (bx & 0xffff);
		//_tprintf(_T("%ls mouse down %d %08X   %d,%d\n"), zWnd->name, clicks, mods, bx, by);

		wParam |= (mods & NSShiftKeyMask) ? MK_SHIFT : 0;
		wParam |= (mods & NSControlKeyMask) ? MK_CONTROL : 0;

		PostMessage((HWND)zWnd, WM_LBUTTONDOWN, wParam, lParam);
		if(clicks > 1)
			PostMessage((HWND)zWnd, WM_LBUTTONDBLCLK, wParam, lParam);
	}
}

- (void)mouseUp:(NSEvent *)theEvent
{
	NSPoint eventloc = [theEvent locationInWindow];
	NSPoint pt = [self convertPoint:eventloc fromView:nil];
	LPZWND  zWnd = ZXwindow(self);

	if (zWnd)
	{
		int bx = (int)pt.x;
		int by = (int)pt.y;
		int mods = [theEvent modifierFlags];

		int  wParam  = MK_LBUTTON;
		LONG lParam = (by << 16) | (bx & 0xffff);

		//_tprintf(_T("%ls mouse UP %d,%d\n"), zWnd->name, bx, by);
		wParam |= (mods & NSShiftKeyMask) ? MK_SHIFT : 0;
		wParam |= (mods & NSControlKeyMask) ? MK_CONTROL : 0;
		PostMessage((HWND)zWnd, WM_LBUTTONUP, wParam, lParam);
	}
}

- (void)rightMouseDown:(NSEvent *)theEvent
{
	NSPoint eventloc = [theEvent locationInWindow];
	NSPoint pt = [self convertPoint:eventloc fromView:nil];
	LPZWND  zWnd = ZXwindow(self);

	// note that right-clicks get sent relative to window, not screen
	// and that's the same for windows, so cool
	if(zWnd)
	{
		RECT rcd;

		int bx = (int)pt.x;
		int by = (int)pt.y;
		int mods = [theEvent modifierFlags];

		//_tprintf(_T("%f %f %ls Rmouse down %d,%d\n"), pt.x, pt.y, zWnd->name, bx, by);
		LONG lParam = (by << 16) | (bx & 0xffff);
		int  wParam  = MK_RBUTTON;

		wParam |= (mods & NSShiftKeyMask) ? MK_SHIFT : 0;
		wParam |= (mods & NSControlKeyMask) ? MK_CONTROL : 0;
		PostMessage((HWND)zWnd, WM_RBUTTONDOWN, wParam, lParam);
	}
}

- (void)rightMouseUp:(NSEvent *)theEvent
{
	NSPoint eventloc = [theEvent locationInWindow];
	NSPoint pt = [self convertPoint:eventloc fromView:nil];
	LPZWND  zWnd = ZXwindow(self);

	if (zWnd)
	{
		RECT rcd;

		int bx = (int)pt.x;
		int by = (int)pt.y;
		int mods = [theEvent modifierFlags];

		GetWindowRect((HWND)zWnd, &rcd);
		bx -= rcd.left;
		by -= rcd.top;
		//_tprintf(_T("%ls Rmouse UP %d,%d\n"), zWnd->name, bx, by);
		LONG lParam = (by << 16) | (bx & 0xffff);
		int  wParam  = MK_RBUTTON;

		wParam |= (mods & NSShiftKeyMask) ? MK_SHIFT : 0;
		wParam |= (mods & NSControlKeyMask) ? MK_CONTROL : 0;
		PostMessage((HWND)zWnd, WM_RBUTTONUP, wParam, lParam);
	}
}

- (void)mouseMoved:(NSEvent *)theEvent
{
	NSPoint eventloc = [theEvent locationInWindow];
	NSPoint pt = [self convertPoint:eventloc fromView:nil];
	LPZWND  zWnd = ZXwindow(self);

	if(zWnd)
	{
		RECT rcd;

		int bx = (int)pt.x;
		int by = (int)pt.y;
		int mods = [theEvent modifierFlags];

		//_tprintf(_T("%ls mouse Move %d,%d\n"), zWnd->name, bx, by);
		int lParam = (by << 16) | (bx & 0xffff);
		int wParam  = 0;

		wParam |= (mods & NSShiftKeyMask) ? MK_SHIFT : 0;
		wParam |= (mods & NSControlKeyMask) ? MK_CONTROL : 0;
		PostMessage((HWND)zWnd, WM_MOUSEMOVE, wParam, lParam);
	}
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	NSPoint eventloc = [theEvent locationInWindow];
	NSPoint pt = [self convertPoint:eventloc fromView:nil];
	LPZWND   zWnd = ZXwindow(self);

	if(zWnd)
	{
		RECT rcd;

		int bx = (int)pt.x;
		int by = (int)pt.y;
		int mods = [theEvent modifierFlags];

		//_tprintf(_T("%ls mouse Move %d,%d\n"), zWnd->name, bx, by);
		int lParam = (by << 16) | (bx & 0xffff);
		int wParam  = MK_LBUTTON;

		wParam |= (mods & NSShiftKeyMask) ? MK_SHIFT : 0;
		wParam |= (mods & NSControlKeyMask) ? MK_CONTROL : 0;
		PostMessage((HWND)zWnd, WM_MOUSEMOVE, wParam, lParam);
	}
}

- (void)mouseExited:(NSEvent *)theEvent
{
	NSPoint eventloc = [theEvent locationInWindow];
	NSPoint pt = [self convertPoint:eventloc fromView:nil];
	LPZWND  zWnd = ZXwindow(self);

	if(zWnd)
	{
		RECT rcd;

		extern TRACKMOUSEEVENT _zg_mouseTrack;

		int bx = (int)pt.x;
		int by = (int)pt.y;
		int mods = [theEvent modifierFlags];

		//_tprintf(_T("%ls mouse Leave %d,%d\n"), zWnd->name, bx, by);
		LONG lParam = (by << 16) | (bx & 0xffff);
		int  wParam  = 0;

		wParam |= (mods & NSShiftKeyMask) ? MK_SHIFT : 0;
		wParam |= (mods & NSControlKeyMask) ? MK_CONTROL : 0;

		if(_zg_mouseTrack.hwndTrack == (HWND)zWnd)
		{
			if(_zg_mouseTrack.dwFlags & TME_LEAVE)
			{
				// leave cancels further tracking
				_zg_mouseTrack.hwndTrack = NULL;
				PostMessage((HWND)zWnd, WM_MOUSELEAVE, wParam, lParam);
			}
		}
	}
}

- (void)mouseEntered:(NSEvent *)theEvent
{
	NSPoint eventloc = [theEvent locationInWindow];
	NSPoint pt = [self convertPoint:eventloc fromView:nil];
	LPZWND   zWnd = ZXwindow(self);

	[[self window] invalidateCursorRectsForView:self];

	if (zWnd)
	{
		RECT rcd;

		int bx = (int)pt.x;
		int by = (int)pt.y;
		int mods = [theEvent modifierFlags];

		//_tprintf(_T("%ls mouse Enter %d,%d\n"), zWnd->name, bx, by);
		LONG lParam = (by << 16) | (bx & 0xffff);
		int  wParam  = 0;

		wParam |= (mods & NSShiftKeyMask) ? MK_SHIFT : 0;
		wParam |= (mods & NSControlKeyMask) ? MK_CONTROL : 0;
		PostMessage((HWND)zWnd, WM_MOUSEMOVE, wParam, lParam);
	}
}

- (void)scrollWheel:(NSEvent *)theEvent
{
	LPZWND   zWnd = ZXwindow(self);

	if(zWnd)
	{
		int bx = (int)[theEvent scrollingDeltaX];
		int by = (int)[theEvent scrollingDeltaY];
		/*
		_tprintf(_T("%ls mouse wheel %d,%d %f,%f\n"),
				zWnd->name, bx, by, [theEvent deltaX], [theEvent deltaY]);
		*/
		int mods = [theEvent modifierFlags];
		if([theEvent deltaY] < 0.0)
		{
			if(by == 0)
				by = -WHEEL_DELTA;
			else
				by *= WHEEL_DELTA;
		}
		if([theEvent deltaY] > 0.0)
		{
			if(by == 0)
				by = WHEEL_DELTA;
			else
				by *= WHEEL_DELTA;
		}
		//_tprintf(_T("%ls mouse wheel %d,%d\n"), zWnd->name, bx, by);
		LONG lParam = (by << 16) | (bx & 0xffff);
		int  wParam = (by << 16);

		wParam |= (mods & NSShiftKeyMask) ? MK_SHIFT : 0;
		wParam |= (mods & NSControlKeyMask) ? MK_CONTROL : 0;
		PostMessage((HWND)zWnd, WM_MOUSEWHEEL, wParam, lParam);
	}
}

static unsigned _nskeyToWindows(unsigned kcode)
{
	if (kcode < sizeof(MACkeyTranslations)/sizeof(unsigned))
	{
		return MACkeyTranslations[kcode];
	}
	return 0;
}

/*
   NSAlphaShiftKeyMask = 1 << 16,
   NSShiftKeyMask      = 1 << 17,
   NSControlKeyMask    = 1 << 18,
   NSAlternateKeyMask  = 1 << 19,
   NSCommandKeyMask    = 1 << 20,
   NSNumericPadKeyMask = 1 << 21,
   NSHelpKeyMask       = 1 << 22,
   NSFunctionKeyMask   = 1 << 23,
   NSDeviceIndependentModifierFlagsMask = 0xffff0000U
*/
#define HackNumLock (1<<26)

- (void)keyDown:(NSEvent *)theEvent
{
	LPZWND   zWnd = ZXwindow(self);

	if (zWnd)
	{
		unsigned mods;
		int wParam, lParam;

		mods = [theEvent modifierFlags] & NSDeviceIndependentModifierFlagsMask;
		if(mods != (keymods & ~HackNumLock))
		{
			if((mods ^ keymods) & NSShiftKeyMask)
			{
				wParam = VK_SHIFT;
				lParam = 0x04000000;
				PostMessage((HWND)zWnd, (mods & NSShiftKeyMask) ? WM_KEYDOWN : WM_KEYUP, wParam, lParam);
			}
			if((mods ^ keymods) & NSControlKeyMask)
			{
				wParam = VK_CONTROL;
				lParam = 0x04000000;
				PostMessage((HWND)zWnd, (mods & NSControlKeyMask) ? WM_KEYDOWN : WM_KEYUP, wParam, lParam);
			}
			if((mods ^ keymods) & NSCommandKeyMask)
			{
				wParam = VK_CONTROL;
				lParam = 0x04000000;
				PostMessage((HWND)zWnd, (mods & NSCommandKeyMask) ? WM_KEYDOWN : WM_KEYUP, wParam, lParam);
			}
			keymods = mods | (keymods & HackNumLock);
		}
		lParam = 0;
		wParam = _nskeyToWindows([theEvent keyCode]);
		if(wParam & 0x1000)
		{
			lParam |= 0x04000000;
			wParam &= ~0x1000;
			if (wParam == VK_NUMLOCK)
			{
				keymods ^= HackNumLock;
			}
			if (keymods & NSNumericPadKeyMask)
			{
				if(! (keymods & HackNumLock))
				{
					switch(wParam)
					{
					case VK_NUMPAD0:	wParam = VK_INSERT; break;
					case VK_NUMPAD1:	wParam = VK_END;  	break;
					case VK_NUMPAD2:	wParam = VK_DOWN; 	break;
					case VK_NUMPAD3:	wParam = VK_NEXT; 	break;
					case VK_NUMPAD4:	wParam = VK_LEFT;   break;
					case VK_NUMPAD5:	break;
					case VK_NUMPAD6:	wParam = VK_RIGHT;	break;
					case VK_NUMPAD7:	wParam = VK_HOME; 	break;
					case VK_NUMPAD8:	wParam = VK_UP;   	break;
					case VK_NUMPAD9:	wParam = VK_PRIOR;	break;
					case VK_MULTIPLY:	wParam = '*'; lParam = 0; break;
					case VK_ADD:		wParam = '+'; lParam = 0; break;
					case VK_SEPARATOR:	break;
					case VK_SUBTRACT:	wParam = '-'; lParam = 0; break;
					case VK_DECIMAL:	wParam = '.'; lParam = 0; break;
					case VK_DIVIDE:		wParam = '/'; lParam = 0; break;
					default:
						break;
					}
				}
			}
		}
		else if(wParam >= 'A' && wParam <= 'Z')
		{
			if(keymods & (NSControlKeyMask | NSCommandKeyMask))
				wParam &= 0x1F;
			else if(! ((keymods & NSShiftKeyMask) ^ (keymods & NSAlphaShiftKeyMask)))
				wParam += 'a' - 'A';
		}
		else if(keymods & NSShiftKeyMask)
		{
			switch(wParam)
			{
			case '`':	wParam = '~'; break;
			case '1':	wParam = '!'; break;
			case '2':	wParam = '@'; break;
			case '3':	wParam = '#'; break;
			case '4':	wParam = '$'; break;
			case '5':	wParam = '%'; break;
			case '6':	wParam = '^'; break;
			case '7':	wParam = '&'; break;
			case '8':	wParam = '*'; break;
			case '9':	wParam = '('; break;
			case '0':	wParam = ')'; break;
			case '-':	wParam = '_'; break;
			case '=':	wParam = '+'; break;
			case ',':	wParam = '<'; break;
			case '.':	wParam = '>'; break;
			case '/':	wParam = '?'; break;
			case ';':	wParam = ':'; break;
			case '\'':	wParam = '\"'; break;
			case '[':	wParam = '{'; break;
			case ']':	wParam = '}'; break;
			case '\\':	wParam = '|'; break;
			}
		}
		//_tprintf(_T("KeyD %ls %d %d %c\n"), zWnd->name, [theEvent keyCode], wParam, wParam);
		PostMessage((HWND)zWnd, (keymods & NSAlternateKeyMask) ?
					WM_SYSKEYDOWN : WM_KEYDOWN, wParam, lParam);
	}
}

- (void)keyUp:(NSEvent *)theEvent
{
	LPZWND   zWnd = ZXwindow(self);

	if (zWnd)
	{
		int wParam, lParam;

		lParam = 0;
		wParam = _nskeyToWindows([theEvent keyCode]);
		if(wParam & 0x1000)
		{
			lParam |= 0x04000000;
			wParam &= ~0x1000;
		}
		else if(wParam >= 'A' && wParam <= 'Z')
		{
			if(keymods & NSControlKeyMask)
				wParam &= 0x1F;
			else if(keymods & NSShiftKeyMask)
				wParam += 'a' - 'A';
		}
		//_tprintf(_T("KeyU %ls %d %d %c\n"), zWnd->name, [theEvent keyCode], wParam, wParam);
		PostMessage((HWND)zWnd, (keymods & NSAlternateKeyMask) ?
					WM_SYSKEYUP : WM_KEYUP, wParam, lParam);
	}
}

- (void) resetCursorRects
{
	LPZWND   zWnd = ZXwindow(self);

    [super resetCursorRects];
	if (zWnd && zWnd->class && zWnd->class->hCursor)
	{
		LPZGDIOBJ zObj;
		LPZBITMAP zCursor;

		if((zObj = (LPZGDIOBJ)zWnd->class->hCursor) == NULL) return;
		if(zObj->tag != ZGDIO_TAG) return;

		if((zCursor = (LPZBITMAP)(zObj->obj)) != NULL)
		{
			if(zCursor->type == RT_Cursor)
			{
	    		[self addCursorRect: [self bounds]
	        		  cursor: (NSCursor *)zCursor->cursor];
				[(NSCursor *)zCursor->cursor setOnMouseEntered:YES];
				return;
			}
		}
	}
}

- (BOOL)resignFirstResponder
{
	LPZWND   zWnd = ZXwindow(self);

	if (zWnd)
	{
		//_tprintf(_T("osx says %ls losing focus\n"), zWnd->name);
		_zg_lastFocus = NULL;
		SendMessage((HWND)zWnd, WM_KILLFOCUS, 0, 0);
	}
	return YES;
}

- (BOOL)acceptsFirstResponder
{
	LPZWND   zWnd = ZXwindow(self);

	if (zWnd)
	{
		return YES;
	}
	return NO;
}

- (void)dealloc
{
    [super dealloc];
}

@end

//**************************************************************************
int _w_setFocus(Window nw)
{
	ZWindow *nv = (ZWindow *)nw;

	if(! nw) return -1;

	if([nv acceptsFirstResponder] == YES)
	{
        [[nv window] makeKeyAndOrderFront:nil];
		if([[nv window] makeFirstResponder:nv] == YES)
		{
			/*
			LPZWND zWnd = ZXwindow(nv);

			if(zWnd)
			{
				_tprintf(_T("WE says focus on %ls\n"), zWnd->name);
			}
			*/
		}
	}
	return 0;
}

//**************************************************************************
Window _w_getFocus()
{
	if(_zg_lastFocus)
		return _zg_lastFocus->xWnd;
	return NULL;
}

//**************************************************************************
int _w_setParent(Window nw, Window nwparent)
{
	ZWindow *nv  = (ZWindow *)nw;
	ZWindow *nvp = (ZWindow *)nwparent;
	ZWindow *nvop;

	if(! nv || ! nvp) return -1;

	nvop = (ZWindow *)[nv superview];
	if(nvop)
	{
		[nv removeFromSuperview];
	}
	[nv setMyParent:nvp];
	[nvp addSubview:nv];
	return 0;
}

//**************************************************************************
HWND _w_getScrollBar(HWND hwndParent, int type)
{
	LPZWND	zWnd;
	LPZSBAR	psc;

	#if 1
	zWnd = Zwindow(hwndParent);
	if(! zWnd) return NULL;

	NSArray *mysubs = [(ZWindow *)zWnd->xWnd subviews];
	int wc;

	for(wc = 0; wc < [mysubs count]; wc++)
	{
		zWnd = ZXwindow((ZWindow *)[mysubs objectAtIndex:wc]);

		psc = (LPZSBAR)GetWindowLong(zWnd, GWL_USERDATA);

		if(psc && psc->tag == ZSBAR_TAG && psc->type == type)
			return (HWND)zWnd;
	}
	#else
	int wc = 0;

	for(zWnd = _zg_windows; zWnd; zWnd = zWnd->next)
	{
		wc++;
		if(zWnd->class && ((void*)zWnd->class->proc == (void*)&__wproc_Scrollbar))
		{
			if(GetParent(zWnd) == hwndParent)
			{
				psc = (LPZSBAR)GetWindowLong(zWnd, GWL_USERDATA);

				if(psc && psc->type == type)
				{
					return (HWND)zWnd;
				}
			}
		}
	}
	//printf("wc=%d\n", wc);
	#endif
	return NULL;
}

//**************************************************************************
Window _w_getParent(Window nw)
{
	ZWindow *nv  = (ZWindow *)nw;

	/*
	 * use this to see of windows are leaking
	LPZWND zWnd;
	int nwin;
	for(zWnd = _zg_windows, nwin = 0; zWnd; zWnd = zWnd->next)
	{
		nwin++;
	}
	printf("%d totw\n", nwin);
	*/
	if(! nv)
		return NULL;
	if(! [nv myParent])
		[nv setMyParent:(ZWindow*)[nv superview]];
	return [nv myParent];
}

//**************************************************************************
int _w_setTitle(Window nw, const char *title)
{
	ZWindow *nv = (ZWindow *)nw;

	if(! nw) return -1;

	NSWindow *nsw = [nv window];

	if(nsw)
	{
		NSString *titString = [[NSString alloc] initWithUTF8String:title];

		[nsw setTitle:titString];
	}
	return 0;
}

//**************************************************************************
int _w_windowRect(Window nw, Window *parent, LPRECT lprcc, unsigned *bw, unsigned *d)
{
	ZWindow *nv = (ZWindow *)nw;
	// get screen dimensions
	NSRect rs = [[NSScreen mainScreen] frame];

	if(parent)
		*parent = NULL;
	if (bw) *bw = 0;
	if (d) *d = 8;

	if(! nw)
	{
		if(lprcc)
		{
			lprcc->left   = (int)0;
			lprcc->top    = (int)0;
			lprcc->right  = (int)(rs.size.width + 0.5);
			lprcc->bottom = (int)(rs.size.height + 0.5);
		}
		return 0;
	}
	if(parent)
		*parent = (Window)[nv superview];

	if(! lprcc)
		return 0; // only wanted the parent

	LPZWND zWnd = ZXwindow(nw);

	// get position of window in screen
	NSRect rw = [[nv window] frame];

	// convert window position upper left to 0,0 top left world
	rw.origin.y += rw.size.height;
	rw.origin.y = rs.size.height - rw.origin.y;
	/*
	DWORD dwStyle = GetWindowLong(zWnd, GWL_STYLE);
	if(dwStyle & (WS_CAPTION | WS_SYSMENU))
	{
		// add decoration of top level window for windows with titbars
		rw.origin.y += g_dech;
		rw.origin.x += g_decw;
		rw.size.width -= g_decw;
		rw.size.height -= g_dech;
	}
	*/
	// get frame of view
	NSRect rf = [nv frame];

	// add upper left of window to it
	rf.origin.x += rw.origin.x;
	rf.origin.y += rw.origin.y;

	// for each parent view, add in their offset from their parent
	do
	{
		nv = (ZWindow*)[nv superview];
		if(nv)
		{
			NSRect rp = [nv frame];

			rf.origin.x += rp.origin.x;
			rf.origin.y += rp.origin.y;
		}
	}
	while(nv);
	lprcc->left   = (int)(rf.origin.x);
	lprcc->top    = (int)(rf.origin.y);
	lprcc->right  = (int)(rf.origin.x + rf.size.width);
	lprcc->bottom = (int)(rf.origin.y + rf.size.height);
	return 0;
}

//**************************************************************************
int _w_clientRect(Window nw, LPRECT lprcc)
{
	ZWindow *nv = (ZWindow *)nw;

	if(! nv)
	{
		return _w_windowRect(nw, NULL, lprcc, NULL, NULL);
	}
	// get frame of view
	NSRect rf = [nv frame];

	lprcc->left   = (int)0;
	lprcc->top    = (int)0;
	lprcc->right  = (int)(rf.size.width + 0.5);
	lprcc->bottom = (int)(rf.size.height + 0.5);
	return 0;
}

//**************************************************************************
int _w_redrawWindow(Window nw, int x, int y, int w, int h, int erase)
{
	ZWindow *nv = (ZWindow *)nw;
	NSRect dirtyRect;

	if(! nw) return -1;

	dirtyRect.origin.x = (CGFloat)x;
	dirtyRect.origin.y = (CGFloat)y;
	dirtyRect.size.width = (CGFloat)w;
	dirtyRect.size.height = (CGFloat)h;
	[nv setNeedsErase:(erase ? YES : NO)];
	[nv setNeedsDisplayInRect:dirtyRect];
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
	ZWindow *nv = (ZWindow *)nw;

	if(! nw) return -1;

	[nv setHidden:(show ? NO : YES)];
	_resetAttachedViews(ZXwindow(nw));
	if(show)
		InvalidateRect(ZXwindow(nv), NULL, TRUE);
	return 0;
}

//**************************************************************************
int _w_moveWindow(Window nw, int x, int y, int w, int h)
{
	NSRect rw;
	ZWindow *nv = (ZWindow *)nw;

	if(! nw) return -1;

	rw.origin.x = x;
	rw.origin.y = y;
	rw.size.width = w;
	rw.size.height = h;

	NSRect cw = [nv frame];

	LPZWND zWnd = ZXwindow(nv);

	// tell every child of ours we're moving but don't recurse
	// to ourselves if we're handling a wm_size message, the test
	// for same-size might not be enough to block that
	//
	if(zWnd)
	{
		if(
				cw.origin.x != rw.origin.x
			||	cw.origin.y != rw.origin.y
			||	cw.size.width != rw.size.width
			||	cw.size.height != rw.size.height
		)
		{
			// cocoa top-level windows move differently than views
			DWORD dwStyle = GetWindowLong(zWnd, GWL_STYLE);
			int noframe = (! (dwStyle & (WS_SYSMENU | WS_CAPTION))) || (dwStyle & WS_CHILD);

			if(nv == [[nv window] contentView])
				noframe = 0;

			if(noframe)
			{
				[nv setFrame:rw];
			}
			else
			{
				if(dwStyle & (WS_SYSMENU | WS_CAPTION))
				{
					rw.origin.y -= g_dech_top;
					rw.size.height += g_dech;
					rw.origin.x -= g_decw / 2;
					rw.size.width += g_decw;
				}
				NSRect rs = [[NSScreen mainScreen] frame];
				NSRect rv = [[NSScreen mainScreen] visibleFrame];

				// apply offset of screen we're on
				rw.origin.x += rv.origin.x;

				// and flip to cartesian for y
				rw.origin.y = rv.size.height + rv.origin.y - rw.origin.y - rw.size.height;
				[[nv window] setFrame:rw display:YES];
			}
			PostMessage((HWND)zWnd, WM_SIZE, 0, 0);
		}
		_resetAttachedViews(zWnd);
	}
	return 0;
}

//**************************************************************************
int _w_destroyWindow(Window nw)
{
	ZWindow *nv = (ZWindow *)nw;
	ZWindow *nvp;

	if(! nw) return -1;

	LPZWND zWnd = ZXwindow(nv);
	if(zWnd)
	{
		DWORD dwStyle = GetWindowLong(zWnd, GWL_STYLE);
		int noframe = (! (dwStyle & (WS_SYSMENU | WS_CAPTION))) || (dwStyle & WS_CHILD);

		if(nv == [[nv window] contentView])
			noframe = 0;

		if(! noframe)
		{
			[[nv window] close];
		}
		else
		{
			nvp = (ZWindow*)[nv superview];
			if(nvp)
			{
				[nv removeFromSuperview];

				NSArray *mysubs = [nv subviews];
				int wc;

				for(wc = 0; wc < [mysubs count]; wc++)
				{
					zWnd = ZXwindow((ZWindow *)[mysubs objectAtIndex:wc]);

					if(zWnd)
					{
						// recursive destruction!
						DestroyWindow(zWnd);
					}
				}
				[nv release];
				InvalidateRect(ZXwindow(nvp), NULL, TRUE);
			}
		}
	}
	return 0;
}

@interface MyWindowDelegate : NSObject <NSWindowDelegate, NSWindowDelegate> {
}
@end

@implementation MyWindowDelegate : NSObject
- (id)init
{
    if(self = [super init])
    {
    }
    return self;
}

- (void)windowDidResize:(NSNotification *)notification
{
	NSWindow *nw = [notification object];

	if(nw)
	{
		ZWindow *nv = (ZWindow*)[nw contentView];
		LPZWND zWnd;

		// post a size message to top-most view in window
		//
		zWnd = ZXwindow((Window)nv);
		if(zWnd)
		{
			PostMessage((HWND)zWnd, WM_SIZE, 0, 0);
			_resetAttachedViews(zWnd);
		}
	}
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
	NSWindow *nw = [notification object];

	if(nw)
	{
		ZWindow *nv = (ZWindow*)[nw contentView];
		LPZWND zWnd;

		// post a set focus message to top-most view in window
		//
		zWnd = ZXwindow((Window)nv);
		if(zWnd)
		{
			//_tprintf(_T("osx says focus on %ls\n"), zWnd->name);
			SetFocus((HWND)zWnd);
		}
	}
}

-(void)windowDidResignKey:(NSNotification *)notification
{
	LPZWND   zWnd = _zg_lastFocus;

	if (zWnd)
	{
		//_tprintf(_T("osx says no focus on %ls\n"), zWnd->name);
		_zg_lastFocus = NULL;
		SendMessage((HWND)zWnd, WM_KILLFOCUS, 0, 0);
	}
}

- (BOOL)windowShouldClose:(id)sender
{
	BOOL rc;
	int i;

	// send close message to top-most window and see what it says
	//
	rc = TRUE;
	NSArray *topwins = [[NSApplication sharedApplication] windows];
	for(i = 0; (i < topwins.count) && rc; i++)
	{
		NSWindow *topWin = [topwins objectAtIndex:i];
		NSView *topView = [topWin contentView];

		if(topView)
		{
			LPZWND zWnd = ZXwindow((ZWindow *)topView);

			if(zWnd)
			{
				// tell top window of each top level system window
				// that its closing. it can cancel it by returning 0
				//
				rc = SendMessage((HWND)zWnd, WM_CLOSE, 0, 0);
			}
		}
	}
	return rc ? YES : NO;
}

- (void)windowWillClose:(id)sender
{
#if 0 // too early, let dialog code set focus when dialog closes
	// set focus on remaining top level window if any if focus was lost
	//
	NSArray *topwins = [[NSApplication sharedApplication] windows];
	int i;

	for(i = 0; i < topwins.count; i++)
	{
		NSWindow *topWin = [topwins objectAtIndex:i];
		NSView *topView = [topWin contentView];

		if(topView && (topWin != sender))
		{
			LPZWND zWnd = ZXwindow((Window)topView);

			if(zWnd)
				SetFocus((HWND)zWnd);
			break;
		}
	}
#endif
}

- (void)dealloc
{
    [super dealloc];
}

@end

//**************************************************************************
void _w_setWindowAlpha(Window nw, BYTE alpha)
{
	ZWindow *nv = (ZWindow *)nw;
	NSWindow *topwin;

	if(! nv) return;

	topwin = [nv window];
	if(topwin)
	{
		if(alpha < 255)
		{
			// clear lets the view's color set the real alpha
			[topwin setBackgroundColor:[NSColor clearColor]];
			[topwin setOpaque:NO];
		}
		else
		{
			[topwin setOpaque:YES];
		}
	}
}

//**************************************************************************
Window _w_createWindow(const char *title, DWORD dwStyle, HBRUSH bkg, int bkga,
		int x, int y, int w, int h, Window parent)
{
	NSString *titString = [[NSString alloc] initWithUTF8String:title ];
	NSWindow *nw;
	NSView *view;
	NSRect rcw;
	int style, noframe;

	rcw.origin.x = x;
	rcw.origin.y = y;
	rcw.size.width = w;
	rcw.size.height = h;

	noframe = (! (dwStyle & (WS_SYSMENU | WS_CAPTION))) || (dwStyle & WS_CHILD);
	if(! parent)
		noframe = 0;

	//printf("CreateWind %s %d,%d %d,%d\n", title, x, y, w, h);

	view = [[ZWindow alloc] initWithFrame:rcw];

	style = 0;
	if (dwStyle & (WS_SYSMENU | WS_CAPTION))
		style |= NSTitledWindowMask;

	if (dwStyle & WS_SYSMENU)
		style |= NSClosableWindowMask |
				 NSMiniaturizableWindowMask |
				 NSResizableWindowMask;

	if(noframe)
	{
		style = 0;
		if(parent)
		{
			[(ZWindow*)view setMyParent:(ZWindow *)parent];
			[(ZWindow *)parent addSubview:view];
			[view setFrameOrigin:rcw.origin];
		}
	}
	else
	{
		if(parent)
		{
			parent = NULL;
		}
		nw = [[[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, w, h)
					styleMask:style backing:NSBackingStoreBuffered defer:NO
					/*screen:[NSScreen mainScreen]*/]
	            autorelease];

		if(bkg)
		{
			LPZGDIOBJ zObj = (LPZGDIOBJ)bkg;
			LPZCOLOR zcolor;
			CGFloat components[4];
			CGFloat fr, fg, fb, fa;

			if(zObj && (zObj->tag == ZGDIO_TAG))
			{
				zcolor = ((LPZCOLOR)zObj->obj);
				fr = (CGFloat)zcolor->r / 255.0;
				fg = (CGFloat)zcolor->g / 255.0;
				fb = (CGFloat)zcolor->b / 255.0;
				fa = (CGFloat)zcolor->c / 255.0;

				if(! space)
					space = CGColorSpaceCreateDeviceRGB();

				components[0] = fr;
				components[1] = fg;
				components[2] = fb;
				components[3] = fa;
				NSColorSpace *nspace = [[NSColorSpace alloc] initWithCGColorSpace: space];
				NSColor *ncolor = [NSColor colorWithColorSpace: nspace components:components count:4];

				[nw setBackgroundColor:ncolor];
				//[nw setBackgroundColor: [NSColor clearColor]];

				[ncolor release];
				[nspace release];
			}
		}
    	[nw setOpaque:YES];

#if 1
		if(dwStyle & (WS_SYSMENU | WS_CAPTION))
		{
			y -= g_dech_top;
			h += g_dech;
			x -= g_decw / 2;
			w += g_decw;
		}
		else
		{
			// so borderless child windows line up to parents
			//
			y += g_dech;
		}
		// flip to cartesian
		y = [[NSScreen mainScreen] frame].size.height - y - h;

		[nw setFrame:NSMakeRect(x, y, w, h) display:NO];
#else
        [nw cascadeTopLeftFromPoint:NSMakePoint(x, y)];
#endif
        [nw setTitle:titString];
		[nw setParentWindow:(NSWindow *)parent];
		[nw setContentView:view];

        [nw makeKeyAndOrderFront:nil];

		MyWindowDelegate *winDelegate = [[[MyWindowDelegate alloc] init] autorelease];
	    [nw setDelegate:winDelegate];
	}
	return (Window*)view;
}

//**************************************************************************
int _w_setAppIcon(LPZWND zWnd, LPZBITMAP zIcon, LPCTSTR lpWindowName)
{
	NSImage *iconImage;

	CGImageRef cgImage = CGBitmapContextCreateImage((CGContextRef)zIcon->ximg);

	iconImage = [[NSImage alloc] initWithCGImage:cgImage size:NSZeroSize];
	[application setApplicationIconImage:iconImage];
	CFRelease(cgImage);
	[iconImage release];
	return 0;
}

//**************************************************************************
char* _w_clipGetClipboardData(int* lenret, UINT format)
{
    NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
    NSArray *classArray = [NSArray arrayWithObject:[NSString class]];
    NSDictionary *options = [NSDictionary dictionary];
 	BYTE *xdata;
	DWORD xlen;

    BOOL ok = [pasteboard canReadObjectForClasses:classArray options:options];
    if (ok) {
        NSArray *objectsToPaste = [pasteboard readObjectsForClasses:classArray options:options];
        NSString *string = [objectsToPaste objectAtIndex:0];
		NSData *data;
		BYTE *bytes;
        int len;

		data  = [string dataUsingEncoding:NSUTF8StringEncoding];
		len   = [data length];
		bytes = (BYTE*)[data bytes];

		if(format == CF_UNICODETEXT)
			_w_xclip_utf8_decode(bytes, len);
		else
			_w_xclip_copy(bytes, len);
	}
	xdata = _w_xclip_data(&xlen);
	if(lenret)
		*lenret = xlen;
	return (char*)xdata;
}

//**************************************************************************
void _w_clipSetClipboardData(char *data, int len, UINT format)
{
	BYTE *xdata;
	DWORD xlen;

	if (! data)
		return;

	// clip data is always stored in utf8 format
	//
	if(format == CF_UNICODETEXT)
		_w_xclip_utf8_encode((BYTE*)data, len);
	else
		_w_xclip_copy((BYTE*)data, len);

	xdata = _w_xclip_data(&xlen);
	if(xdata && xlen)
	{
		NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
	    [pasteboard clearContents];
		NSString *string = [[NSString alloc] initWithBytes:(const void*)xdata
				length:xlen
				encoding:NSUTF8StringEncoding
				];
	    NSArray *copiedObjects = [NSArray arrayWithObject:string];
	    [pasteboard writeObjects:copiedObjects];
	}
}

//**************************************************************************
void _w_clipOwnClipboard(UINT format)
{
}

//**************************************************************************
void _w_clipDisownClipboard(void)
{
}

//**************************************************************************
void _w_clipInit(void)
{
}

//**************************************************************************
void _w_clipDeinit(void)
{
}

static BOOL keepRunning;

@interface MyApplicationDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate> {
}
@end

@implementation MyApplicationDelegate : NSObject
- (id)init
{
    if(self = [super init])
    {
    }
    return self;
}

- (void)applicationWillFinishLaunching:(NSNotification *)notification
{
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)theApplication {
	return YES;
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
	printf("BYE\n");
}

- (void)dealloc
{
    [super dealloc];
}

@end

//**************************************************************************

@interface BSAapplication : NSApplication
{
}
@end

@implementation BSAapplication : NSApplication
- (id)init
{
    if(self = [super init])
    {
		keepRunning = YES;
    }
    return self;
}

- (void)runSlice:(NSDate *)timeout
{
	NSEvent *event;

	event = [self
                nextEventMatchingMask:NSAnyEventMask
                untilDate:timeout
                inMode:NSDefaultRunLoopMode
                dequeue:YES];
 	[self sendEvent:event];
	[self updateWindows];
}

- (void)terminate:(id)sender
{
	keepRunning = NO;
}

- (void)dealloc
{
    [super dealloc];
}

@end

//**************************************************************************
int _w_init(void)
{
	// need this to get keystrokes
 	ProcessSerialNumber psn = { 0, kCurrentProcess };
 	TransformProcessType(&psn, kProcessTransformToForegroundApplication);

	// get screen dimensions and titlebar/border metrics
	NSRect rs = [[NSScreen mainScreen] frame];

	g_screenw = (int)rs.size.width;
	g_screenh = (int)rs.size.height;

    NSRect frame = NSMakeRect (0, 0, 100, 100);
    NSRect contentRect;

    contentRect = [NSWindow contentRectForFrameRect: frame
                            styleMask: NSTitledWindowMask];

	g_dech = (int)(frame.size.height - contentRect.size.height);
	/*
	g_dech_top = (int)(frame.origin.y - contentRect.origin.y);
	g_dech_bot = g_dech - g_dech_top;
	*/
	// assume all decoration on top for now
	g_dech_top = g_dech;
	g_dech_bot = 0;

	g_decw = (int)((frame.size.width - contentRect.size.width) / 2.0);

	pool = [[NSAutoreleasePool alloc] init];

	// create a bitmap context or drawing outside drawRect (paint)
	s_bitmapcontext = _createBitmapContext(256, 256);

	application = [BSAapplication sharedApplication];
	[application activateIgnoringOtherApps:YES];

    MyApplicationDelegate *appDelegate = [[[MyApplicationDelegate alloc] init] autorelease];

    [application setDelegate:appDelegate];

	[application finishLaunching];

    [[NSRunningApplication currentApplication] activateWithOptions:
          (NSApplicationActivateAllWindows | NSApplicationActivateIgnoringOtherApps)];
	return 0;
}

//**************************************************************************
int _w_runSlice(int toms)
{
	NSDate *timer = [NSDate dateWithTimeIntervalSinceNow:((double)toms / 2000.0)];
	NSRect caretRect;

	[(BSAapplication *)application runSlice:timer];
	[timer release];

	int x, y, w, h;
	HWND hWnd = _w_caretOwner(&x, &y, &w, &h);
	if(hWnd)
	{
		LPZWND zWnd = Zwindow(hWnd);

		if(zWnd && zWnd->xWnd)
		{
			NSView *nv = (ZWindow *)zWnd->xWnd;
			caretRect.origin.x = (CGFloat)x;
			caretRect.origin.y = (CGFloat)y;
			caretRect.size.width = (CGFloat)w + 1;
			caretRect.size.height = (CGFloat)h + 1;
			[nv setNeedsDisplayInRect:caretRect];
		}
	}
	return keepRunning ? 0 : -1;
}

//**************************************************************************
void _w_finish(void)
{
	if(s_bitmapcontext)
	{
		CGContextRelease(s_bitmapcontext);
	}
    [pool drain];
}

#endif
