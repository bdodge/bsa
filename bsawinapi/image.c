
#include "winapix.c"
#ifdef COCOA
#include <ApplicationServices/ApplicationServices.h>
#endif
#ifndef RES_ICON
#define RES_ICON 	1
#define RES_CURSOR  2
#endif

// default icon size request
//
#if 1
int _zg_cx_icon 	= 128;
int _zg_cx_cursor	= 128;
#else
int _zg_cx_icon 	= 32;
int _zg_cx_cursor	= 32;
#endif

extern LPZGDIOBJ _zg_zgdi_objects;
HIMAGELIST		 _zg_imagelists 	= NULL;

// colororder bits 
//
//  0x3 = pad = 0=none, 1=right, 2=left, 3=none
//  0x4 = ord = 0=rgb,  1=bgr
//  0x8 = swp = 0=none, 1=byteswap
//
//  RGBQUAD, in byte_swapped arch = 0,r,g,b if in a long
//  all colors treated as MSB first by code
//
typedef enum { 
				rGb  = 0x0,
				bGr  = 0x4,
				rGbw = 0x8,
				bGrw = 0xC,
				xrgb = 0x2,
				xbgr = 0x6,
				rgbx = 0x1,
				bgrx = 0x5
			  }
			  _wcorder;

_wcorder _zg_colororder = 2;
int      _zg_scrdepth   = 32;
int      _zg_imgdepth   = 24;

typedef ZBITMAP ZCURSOR, FAR* LPZCURSOR, ZICON, FAR* LPZICON;
	
#define _w_deleteZICON  	_w_deleteZBITMAP
#define _w_deleteZCURSOR 	_w_deleteZBITMAP

static BOOL _w_setIconBkg(LPZICON zIcon, DWORD pixel, BOOL invert);
			
//**************************************************************************
void _w_imageInit(void)
{
#ifdef X11
	Window	myMommy;
	int		myX, myY;
	unsigned myWidth, myHeight;
	unsigned myBorderWidth, myDepth;
	
	XGetGeometry(
				_zg_display, 
				DefaultRootWindow(_zg_display),
				&myMommy,
				&myX,
				&myY,
				&myWidth,
				&myHeight, 
				&myBorderWidth,
				&myDepth
				);

	_zg_scrdepth	= myDepth;
	_zg_imgdepth	= myDepth;
	_zg_colororder	= rgbx;
    
	if(_zg_scrdepth == 24 || _zg_scrdepth == 16 || _zg_scrdepth == 15)
	{
		XImage *ximg;

		ximg = XCreateImage(
							_zg_display,
							_zg_visual,
							_zg_scrdepth,
							ZPixmap,
							0,
							NULL,
							32, 32, 32, 0
						   );
	    
		if(ximg)
		{
			_zg_imgdepth = ximg->bits_per_pixel;
		    
			if(ximg->bits_per_pixel == 24 || ximg->bits_per_pixel == 32)
			{
				switch(ximg->red_mask)
				{
				case 0x000000FFUL:
					_zg_colororder = xbgr; /*xbgr*/
					break;
				case 0x0000FF00UL:
					_zg_colororder = bgrx; /*bgrx*/
					break;
				case 0x00FF0000UL:
					_zg_colororder = xrgb; /*xrgb*/
					break;
				case 0xFF000000UL:
					_zg_colororder = rgbx; /*rgbx*/
					break;
				}
			}
			else if(ximg->bits_per_pixel == 16)
			{
				switch(ximg->red_mask)
				{
				case 0x001F:
					_zg_colororder = bGr; /*bGr*/
					if(ximg->green_mask == 0x3E0)
						_zg_colororder = xbgr; /* no, xbgr */
					break;
				case 0x003E:
				case 0x003C:
					_zg_colororder = bgrx; /*bgrx*/
					break;
				case 0x7C00:
					_zg_colororder = xrgb; /*xrgb*/
					break;
				case 0xF800:
					_zg_colororder = rGb; /*rGb*/
					if(ximg->green_mask == 0x3E0)
						_zg_colororder = rgbx; /* no, rgbx */
					break;
				}
			}
			// if the byte order is LSB first, and the architecture
			// is not byte swapped, then switch everything (xrgb goes to bgrx, etc)
			// (and vs. vc).  If the BIT order is LSB first, then padding only
			// gets swapped
			//
			// the 16 bit format can't be inverted so easily, so it
			// has "swapped" formats
			//
#if BYTE_SWAPPED
			if(ximg->byte_order != LSBFirst)
			{
				if(_zg_colororder == rGb)
					_zg_colororder = rGbw;
				else if(_zg_colororder == bGr)
					_zg_colororder = bGrw;
				else
					_zg_colororder ^= 7;
			}
#else
			if(ximg->byte_order == LSBFirst)
			{
				if(_zg_colororder == rGb)
					_zg_colororder = rGbw;
				else if(_zg_colororder == bGr)
					_zg_colororder = bGrw;
				else
					_zg_colororder ^= 7;
			}
#endif
			if((_zg_colororder & 0x3) == 3)
				_zg_colororder &= ~3;
			
			XDestroyImage(ximg);
		}
	}
#endif
#ifdef COCOA
	int myDepth = 32;

	_zg_scrdepth	= myDepth;
	_zg_imgdepth	= myDepth;
	_zg_colororder	= bgrx; // seems to be the native order
#endif
}

//**************************************************************************
void _ifr_4_8(
				LPBYTE dst, int dststride,
				LPBYTE src, int srcstride,
				int w, int h,
				BOOL invert,
				LPDWORD palette
			   )
{
	LPBYTE  in;
	LPBYTE  out, pout;
	DWORD   c;
	int     x, p;
	BYTE    r, g, b;
	
	if(invert)
		in = src + (h - 1) * srcstride;
	else
		in = src;
	out = dst;
	
	while(h-- > 0)
	{
		pout = out;
		
		for(x = 0; x < w; x++)
		{
			p = in[x / 2];
			if(x & 1)
				p >>= 4;
			else
				p &= 0xf;
			c = palette[p & 0xF];
			r = c >> 16;
			g = c >> 8;
			b = c >> 0;
#ifdef X11
			*pout++ = _w_closestcolor(r, g, b, 0, DefaultColormap(_zg_display, _zg_screen));
#endif
		}
		out += dststride;
		in  += (invert ? -srcstride : srcstride);
	}
}

//**************************************************************************
void _ifr_4_16(
				LPBYTE dst, int dststride,
				LPBYTE src, int srcstride,
				int w, int h,
				BOOL invert,
				LPDWORD palette
			   )
{
	LPBYTE  in;
	LPBYTE  out;
	LPWORD  pout;
	DWORD   c;
	int     x, p;
	BYTE    r, g, b;
	WORD	pel;
	
	if(invert)
		in = src + (h - 1) * srcstride;
	else
		in = src;
	out = dst;
	
	while(h-- > 0)
	{
		pout = (LPWORD)out;
		
		for(x = 0; x < w; x++)
		{
			p = in[x / 2];
			if(x & 1)
				p >>= 4;
			else
				p &= 0xf;
			c = palette[p];
			r = c >> 16;
			g = c >> 8;
			b = c >> 0;
			switch(_zg_colororder)
			{
			case xrgb:
				pel = (((WORD)r & 0xF8) << 7) | (((WORD)g & 0xF8) << 2) | ((WORD)b >> 3);
				break;
			case xbgr:
				pel = (((WORD)b & 0xF8) << 7) | (((WORD)g & 0xF8) << 2) | ((WORD)r >> 3);
				break;
			case rgbx:
				pel = (((WORD)r & 0xF8) << 8) | (((WORD)g & 0xF8) << 3) | (((WORD)b & 0xF8) >> 2);
				break;
			case bgrx:
				pel = (((WORD)b & 0xF8) << 8) | (((WORD)g & 0xF8) << 3) | (((WORD)r & 0xF8) >> 2);
				break;
			case rGb:
				pel = (((WORD)r & 0xF8) << 8) | (((WORD)g & 0xFC) << 3) | ((WORD)b >> 3);
				break;
			case bGr:
				pel = (((WORD)b & 0xF8) << 8) | (((WORD)g & 0xFC) << 3) | ((WORD)r >> 3);
				break;
			case rGbw:
				pel = (((WORD)r & 0xF8) << 8) | (((WORD)g & 0xFC) << 3) | ((WORD)b >> 3);
				pel = (pel >> 8) | (pel << 8);
				break;
			case bGrw:
				pel = (((WORD)b & 0xF8) << 8) | (((WORD)g & 0xFC) << 3) | ((WORD)r >> 3);
				pel = (pel >> 8) | (pel << 8);
				break;
			}
			*pout++ = pel;
		}
		out += dststride;
		in  += (invert ? -srcstride : srcstride);
	}
}

//**************************************************************************
void _ifr_4_24(
				LPBYTE dst, int dstride,
				LPBYTE src, int srcstride,
				int w, int h,
				BOOL invert,
				LPDWORD palette
			   )
{
}

//**************************************************************************
void _ifr_4_32(
				LPBYTE dst, int dststride,
				LPBYTE src, int srcstride,
				int w, int h,
				BOOL invert,
				LPDWORD palette
			   )
{
	LPBYTE  in;
	LPBYTE  out;
	LPDWORD pout;
	DWORD   c;
	DWORD   pel;
	BYTE    p;
	int     x;
	
	if(invert)
		in = src + (h - 1) * srcstride;
	else
		in = src;
	out = dst;
	
	while(h-- > 0)
	{
		pout = (LPDWORD)out;
		
		for(x = 0; x < w; x++)
		{
			p = in[x / 2];
			if(x & 1)
				p >>= 4;
			else
				p &= 0xf;
			c = palette[p];
			
			switch(_zg_colororder)
			{
			case xrgb:
			case rgbx:
				pel =  c >> 16;
				pel <<= 8;
				pel |= c >> 8;
				pel <<= 8;
				pel |= c >> 0;
				break;
			case bgrx:
			case xbgr:
				pel =  c >> 0;
				pel <<= 8;
				pel |= c >> 8;
				pel <<= 8;
				pel |= c >> 16;
				break;
			default:
				break;
			}
			if((_zg_colororder & 3) == 1)
			{
				pel <<= 8;			
				pel |= _zg_alpha;
			}
			else
			{
				pel |= (DWORD)_zg_alpha << 24;
			}
			*pout++ = pel;
		}
		out += dststride;
		in  += (invert ? -srcstride : srcstride);
	}
}

//**************************************************************************
void _ifr_8_8(
				LPBYTE dst, int dststride,
				LPBYTE src, int srcstride,
				int w, int h,
				BOOL invert,
				LPDWORD palette
			   )
{
	LPBYTE  in;
	LPBYTE  out, pout;
	DWORD   c;
	int     x, p;
	BYTE    r, g, b;
	
	if(invert)
		in = src + (h - 1) * srcstride;
	else
		in = src;
	out = dst;
	
	while(h-- > 0)
	{
		pout = out;
		
		for(x = 0; x < w; x++)
		{
			p = in[x];
			c = palette[p];
			r = c >> 16;
			g = c >> 8;
			b = c >> 0;
#ifdef X11
			*pout++ = _w_closestcolor(r, g, b, 0, DefaultColormap(_zg_display, _zg_screen));
#endif
		}
		out += dststride;
		in  += (invert ? -srcstride : srcstride);
	}
}

//**************************************************************************
void _ifr_8_16(
				LPBYTE dst, int dststride,
				LPBYTE src, int srcstride,
				int w, int h,
				BOOL invert,
				LPDWORD palette
			   )
{
	LPBYTE  in;
	LPBYTE  out;
	LPWORD  pout;
	DWORD   c;
	int     x, p;
	BYTE    r, g, b;
	WORD	pel;
	
	if(invert)
		in = src + (h - 1) * srcstride;
	else
		in = src;
	out = dst;
	
	while(h-- > 0)
	{
		pout = (LPWORD)out;
		
		for(x = 0; x < w; x++)
		{
			p = in[x];
			c = palette[p];
			r = c >> 16;
			g = c >> 8;
			b = c >> 0;
			switch(_zg_colororder)
			{
			case xrgb:
				pel = (((WORD)r & 0xF8) << 7) | (((WORD)g & 0xF8) << 2) | ((WORD)b >> 3);
				break;
			case xbgr:
				pel = (((WORD)b & 0xF8) << 7) | (((WORD)g & 0xF8) << 2) | ((WORD)r >> 3);
				break;
			case rgbx:
				pel = (((WORD)r & 0xF8) << 8) | (((WORD)g & 0xF8) << 3) | (((WORD)b & 0xF8) >> 2);
				break;
			case bgrx:
				pel = (((WORD)b & 0xF8) << 8) | (((WORD)g & 0xF8) << 3) | (((WORD)r & 0xF8) >> 2);
				break;
			case rGb:
				pel = (((WORD)r & 0xF8) << 8) | (((WORD)g & 0xFC) << 3) | ((WORD)b >> 3);
				break;
			case bGr:
				pel = (((WORD)b & 0xF8) << 8) | (((WORD)g & 0xFC) << 3) | ((WORD)r >> 3);
				break;
			case rGbw:
				pel = (((WORD)r & 0xF8) << 8) | (((WORD)g & 0xFC) << 3) | ((WORD)b >> 3);
				pel = (pel >> 8) | (pel << 8);
				break;
			case bGrw:
				pel = (((WORD)b & 0xF8) << 8) | (((WORD)g & 0xFC) << 3) | ((WORD)r >> 3);
				pel = (pel >> 8) | (pel << 8);
				break;
			}
			*pout++ = pel;
		}
		out += dststride;
		in  += (invert ? -srcstride : srcstride);
	}
}

//**************************************************************************
void _ifr_8_24(
				LPBYTE dst, int dstride,
				LPBYTE src, int srcstride,
				int w, int h,
				BOOL invert,
				LPDWORD palette
			   )
{
}

//**************************************************************************
void _ifr_8_32(
				LPBYTE dst, int dststride,
				LPBYTE src, int srcstride,
				int w, int h,
				BOOL invert,
				LPDWORD palette
			   )
{
	LPBYTE  in;
	LPBYTE  out;
	LPDWORD pout;
	DWORD   c;
	DWORD   pel;
	int     x, p;
	
	if(invert)
		in = src + (h - 1) * srcstride;
	else
		in = src;
	out = dst;
	
	while(h-- > 0)
	{
		pout = (LPDWORD)out;
		
		for(x = 0; x < w; x++)
		{
			p = in[x];
			c = palette[p];
			
			switch(_zg_colororder)
			{
			case xrgb:
			case rgbx:
				/*
				pel = 0xff & (c >> 16);
				pel <<= 8;
				pel |= 0xff & (c >> 8);
				pel <<= 8;
				pel |= 0xff & (c >> 0);
				*/
				pel = c;
				break;
			case bgrx:
			case xbgr:
				pel = 0xff & (c >> 0);
				pel <<= 8;
				pel |= 0xff & (c >> 8);
				pel <<= 8;
				pel |= 0xff & (c >> 16);
				break;
			default:
				break;
			}
			if((_zg_colororder & 3) == 1)
			{
				pel <<= 8;			
				pel |= _zg_alpha;
			}
			else
			{
				pel |= (DWORD)_zg_alpha << 24;
			}
			*pout++ = pel;
		}
		out += dststride;
		in  += (invert ? -srcstride : srcstride);
	}
}

//**************************************************************************
void _ifr_24_8(
				LPBYTE dst, int dststride,
				LPBYTE src, int srcstride,
				int w, int h,
				BOOL invert,
				LPDWORD palette
			   )
{
	LPBYTE  in, pin;
	LPBYTE  out;
	LPWORD  pout;
	int     x;
	BYTE    r, g, b;
	
	if(invert)
		in = src + (h - 1) * srcstride;
	else
		in = src;
	out = dst;
	
	while(h-- > 0)
	{
		pout = (LPWORD)out;
		pin  = in;
		
		for(x = 0; x < w; x++)
		{
			b = *pin++;
			g = *pin++;
			r = *pin++;
#ifdef X11
			*pout++ = _w_closestcolor(r, g, b, 0, DefaultColormap(_zg_display, _zg_screen));
#endif
		}
		out += dststride;
		in  += (invert ? -srcstride : srcstride);
	}
}

//**************************************************************************
void _ifr_24_16(
				LPBYTE dst, int dststride,
				LPBYTE src, int srcstride,
				int w, int h,
				BOOL invert,
				LPDWORD palette
			   )
{
	LPBYTE  in, pin;
	LPBYTE  out;
	LPWORD  pout;
	int     x;
	BYTE    r, g, b;
	WORD	pel;
	
	if(invert)
		in = src + (h - 1) * srcstride;
	else
		in = src;
	out = dst;
	
	while(h-- > 0)
	{
		pout = (LPWORD)out;
		pin  = in;
		
		for(x = 0; x < w; x++)
		{
			b = *pin++;
			g = *pin++;
			r = *pin++;
			switch(_zg_colororder)
			{
			case xrgb:
				pel = (((WORD)r & 0xF8) << 7) | (((WORD)g & 0xF8) << 2) | ((WORD)b >> 3);
				break;
			case xbgr:
				pel = (((WORD)b & 0xF8) << 7) | (((WORD)g & 0xF8) << 2) | ((WORD)r >> 3);
				break;
			case rgbx:
				pel = (((WORD)r & 0xF8) << 8) | (((WORD)g & 0xF8) << 3) | (((WORD)b & 0xF8) >> 2);
				break;
			case bgrx:
				pel = (((WORD)b & 0xF8) << 8) | (((WORD)g & 0xF8) << 3) | (((WORD)r & 0xF8) >> 2);
				break;
			case rGb:
				pel = (((WORD)r & 0xF8) << 8) | (((WORD)g & 0xFC) << 3) | ((WORD)b >> 3);
				break;
			case bGr:
				pel = (((WORD)b & 0xF8) << 8) | (((WORD)g & 0xFC) << 3) | ((WORD)r >> 3);
				break;
			case rGbw:
				pel = (((WORD)r & 0xF8) << 8) | (((WORD)g & 0xFC) << 3) | ((WORD)b >> 3);
				pel = (pel >> 8) | (pel << 8);
				break;
			case bGrw:
				pel = (((WORD)b & 0xF8) << 8) | (((WORD)g & 0xFC) << 3) | ((WORD)r >> 3);
				pel = (pel >> 8) | (pel << 8);
				break;
			}
			*pout++ = pel;
		}
		out += dststride;
		in  += (invert ? -srcstride : srcstride);
	}
}

//**************************************************************************
void _ifr_24_24(
				LPBYTE dst, int dststride,
				LPBYTE src, int srcstride,
				int w, int h,
				BOOL invert,
				LPDWORD palette
			   )
{
	LPBYTE  in, pin;
	LPBYTE  out, pout;
	DWORD   c;
	DWORD   pel;
	int     x;
	
	if(invert)
		in = src + (h - 1) * srcstride;
	else
		in = src;
	out = dst;
	
	while(h-- > 0)
	{
		for(x = 0, pin = in, pout = out; x < w; x++)
		{
			c = *pin++;
			c |= *pin++ << 8;
			c |= *pin++ << 16;
			switch(_zg_colororder)
			{
			case xrgb:
			case rgbx:
				pel =  (c >> 16) & 0xff;
				pel <<= 8;
				pel |= (c >> 8) & 0xff;
				pel <<= 8;
				pel |= (c & 0xff);
				break;
			case bgrx:
			case xbgr:
				pel =  (c & 0xff);
				pel <<= 8;
				pel |= (c >> 8) & 0xff;
				pel <<= 8;
				pel |= (c >> 16) & 0xff;
				break;
			default:
				break;
			}
			*pout++ = pel >> 16;
			*pout++ = pel >> 8;
			*pout++ = pel & 0xff;
		}
		out += dststride;
		in  += (invert ? -srcstride : srcstride);
	}
}

//**************************************************************************
void _ifr_24_32(
				LPBYTE dst, int dststride,
				LPBYTE src, int srcstride,
				int w, int h,
				BOOL invert,
				LPDWORD palette
			   )
{
	LPBYTE  in, pin;
	LPBYTE  out;
	LPDWORD pout;
	DWORD   c;
	DWORD   pel;
	int     x;
	
	if(invert)
		in = src + (h - 1) * srcstride;
	else
		in = src;
	out = dst;
	
	while(h-- > 0)
	{
		pout = (LPDWORD)out;
		
		for(x = 0, pin = in; x < w; x++)
		{
			c = *pin++;
			c |= *pin++ << 8;
			c |= *pin++ << 16;
			switch(_zg_colororder)
			{
			case xrgb:
			case rgbx:
				pel = (c >> 16) & 0xff;
				pel <<= 8;
				pel |= (c >> 8) & 0xff;
				pel <<= 8;
				pel |= (c & 0xff);
				break;
			case bgrx:
			case xbgr:
				pel = (c & 0xff);
				pel <<= 8;
				pel |= (c >> 8) & 0xff;
				pel <<= 8;
				pel |= (c >> 16) & 0xff;
				break;
			default:
				break;
			}
			if((_zg_colororder & 3) == 1)
			{
				pel <<= 8;			
				pel |= _zg_alpha;
			}
			else
			{
				pel |= (DWORD)_zg_alpha << 24;
			}
			*pout++ = pel;
		}
		out += dststride;
		in  += (invert ? -srcstride : srcstride);
	}
}

//**************************************************************************
LPZGDIOBJ _w_findGDIimage(int type, LPCTSTR id)
{
	LPZGDIOBJ	zGDIO;
	LPZBITMAP	zBitmap;
	
	for(zGDIO = _zg_zgdi_objects; zGDIO; zGDIO = zGDIO->next)
		if(zGDIO->type == gdiBitmap)
			if((zBitmap = (LPZBITMAP)(zGDIO->obj)) != NULL)
				if(zBitmap->type == type)
					if(zBitmap->id == id)
						return zGDIO;
	return NULL;
}

//**************************************************************************
LPZBITMAP _w_newZBITMAP(LPCTSTR id, int type, char* idata, char* mdata, int w, int h, int d, int xHot, int yHot, void* xdata)
{
	LPZBITMAP zBitmap;
	
	if((zBitmap = (LPZBITMAP)malloc(sizeof(ZBITMAP))) == NULL)
		return NULL;
	
	zBitmap->type 	= type;
	zBitmap->id		= id;
	zBitmap->w		= w;
	zBitmap->h		= h;
	zBitmap->d		= d;
	zBitmap->xHot	= xHot;
	zBitmap->yHot 	= yHot;
	zBitmap->ibkset = FALSE;
	zBitmap->ibkg	= 0;

	switch(type)
	{
	case RT_Cursor:
		zBitmap->cursor	= (Cursor)xdata;
		zBitmap->ximg	= NULL;
		break;
	case RT_Icon:
	case RT_Bitmap:
#ifdef X11
		zBitmap->cursor	= None;
#else
		zBitmap->cursor	= NULL;
#endif
		zBitmap->ximg	= xdata;
		break;
	}
	zBitmap->data	= idata;
	zBitmap->mask	= mdata;

	return zBitmap;			
}

//**************************************************************************
void _w_deleteZBITMAP(LPZBITMAP zBitmap)
{
	if(! zBitmap) 
		return;
#ifdef X11
	if(zBitmap->cursor && (zBitmap->type == RT_Cursor))
		XFreeCursor(_zg_display, zBitmap->cursor);

	if(zBitmap->ximg)
		XFree(zBitmap->ximg);
#endif
#ifdef COCOA
	if(zBitmap->ximg)
		CGImageRelease((CGImageRef)zBitmap->ximg);
#endif
	if(zBitmap->data) free(zBitmap->data);
	if(zBitmap->mask) free(zBitmap->mask);
	free(zBitmap);
}

//**************************************************************************
LPZCURSOR _w_newZCURSOR(LPCTSTR id, char* idata, char* mdata, int w, int h, int d, int xHot, int yHot, Cursor cursor)
{
	LPZCURSOR zCursor;
	
	zCursor = (LPZCURSOR)_w_newZBITMAP(id, RT_Cursor, idata, mdata, w, h, d, xHot, yHot, (void*)cursor);
	return zCursor;
}

//**************************************************************************
LPZICON _w_newZICON(LPCTSTR id, char* idata, char* mdata, int w, int h, int d, int xHot, int yHot, XImage* ximg)
{
	LPZICON zIcon;
	
	zIcon = (LPZICON)_w_newZBITMAP(id, RT_Icon, idata, mdata, w, h, d, xHot, yHot, ximg);
	return zIcon;
}

int SetDIBits(HDC hdc, HBITMAP hbm, UINT fl, UINT ll, CONST void* lpbBits, CONST BITMAPINFO* lpbmi, UINT fuColorUse);

//**************************************************************************
LPZBITMAP _w_createImage(int type, BITMAPINFOHEADER* phdr, LPBYTE imgsrc, LPBYTE masksrc,  LPDWORD palette, int colors)
{	
	int		bpp;
	int		stride, mstride;
	int		imgbytes, maskbytes;
	
	char*	imgdata;
	char*	maskdata;
	
	int		x, y, w, h, invert, hasmask;	
	int		i;

	LPZICON zIcon;
	XImage*	ximg;
	int		ostride;
	int		obytes;
	
//	BYTE  	bmib[sizeof(BITMAPINFO) + 256*sizeof(RGBQUAD)];
	LPBYTE 	bmib = (LPBYTE)malloc(sizeof(BITMAPINFO) + 256*sizeof(RGBQUAD) + 64);
	LPBITMAPINFO pbmi = (LPBITMAPINFO)bmib;
	
	if(! pbmi) return NULL;

	pbmi->bmiHeader = *phdr;
	memset(pbmi->bmiColors, 0, sizeof(RGBQUAD) * 256);
	for(i = 0; i < colors && i < 256 && palette; i++)
	{
		*(((LPDWORD)pbmi->bmiColors) + i) = palette[i];
	}
	
	w = phdr->biWidth;
	h = phdr->biHeight;
				
	if(h < 0)
	{
		h = - h;
		invert = FALSE;
	}
	else
	{	
		invert = TRUE;
	}
	if(h == 2 * w)
	{
		h = h / 2;
		hasmask = TRUE;
	}
	else
	{
		hasmask = FALSE;
	}
	// calculate number of color components needed
	//
	bpp 	 = phdr->biBitCount;
	stride	 = (w * bpp + 7) / 8;
	mstride  = (w + 7) / 8;

	// windows images are always dword aligned regardless of depth
	//
	stride += 3;
	stride &= ~3;
	mstride += 3;
	mstride &= ~3;

	imgbytes = h * stride;
	maskbytes= h * mstride;
	
	if(masksrc)
	{
		maskdata = (char*)malloc(maskbytes);
		if(! maskdata)
		{
			free(bmib);
			return NULL;
		}
	}
	else
	{
		maskdata = NULL;
	}
	ostride = (w * _zg_imgdepth + 7) / 8;
	ostride += 3;
	ostride &= ~3;
	obytes  = ostride * h;
	
	imgdata = (char*)malloc(obytes);
	if(! imgdata)
	{
		free(maskdata);
		free(bmib);
		return NULL;
	}	
	// save mask data
	if(maskdata)
	{
		int z = invert ? (h-1) : 0;
		
		for(y = 0; y < h; y++, z += (invert ? -1 : 1))
			for(x = 0; x < mstride; x++)
				maskdata[y*mstride + x] = masksrc[z*mstride + x];
	}
#if 0 // this debug code places the mask image into the regular image
	{
		BYTE mask, mb;
		LPDWORD pi;
		
		memset(imgdata, 0, ostride * h);
		
		for(y = 0; y < h; y++)
		{
			mb = mask = 0;
			pi = (LPDWORD)(imgdata + y*ostride);
			
			for(x = 0; x < w; x++, mask >>= 1)
			{
				if(! mask)
				{
					mb = maskdata[y*mstride + x/8];
					mask = 0x80;
				}
				pi[x] = (mb & mask)? 0xffffffff : 0;
			}
		}
	}
#endif
#ifdef X11	
	// create an Ximage that
	// has local depth and format, and format
	// the pixel bits into an allocated backing
	//
	ximg = XCreateImage(
						_zg_display,
						_zg_visual,
						_zg_compositing ?
							_zg_vinfo.depth :
							DefaultDepth(_zg_display, DefaultScreen(_zg_display)),
						ZPixmap,
						0,
						imgdata,
						w, h,
						_zg_imgdepth > 8 ? 32 : 8,
						0
					);
#else
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	ximg = (void *)CGBitmapContextCreate(
					    imgdata,
					    w,
					    h,
					    8,
					    ostride,
					    colorSpace,
						(hasmask ? kCGImageAlphaPremultipliedFirst : kCGImageAlphaNoneSkipFirst)
					);
	CFRelease(colorSpace);
#endif
	zIcon = _w_newZICON(_T(""), imgdata, maskdata, w, h, _zg_imgdepth, 0, 0, ximg);
	zIcon->type = type;
	
	if(imgsrc)
	{
		ZGDIOBJ zo;
		
		memset(&zo, 0, sizeof(zo));
		zo.tag = ZGDIO_TAG;
		zo.obj = (void*)zIcon;
		zo.type = gdiBitmap;
		
		SetDIBits(NULL, (HBITMAP)&zo, 0, h, imgsrc, pbmi, (colors && palette) ? DIB_PAL_COLORS : DIB_RGB_COLORS);
		if(hasmask)
		{
			_w_setIconBkg(zIcon, 0, !invert);
		}
	}
	if(bmib)
		free(bmib);
	return (LPZBITMAP)zIcon;
}

//**************************************************************************
LPZBITMAP _w_loadImage(HINSTANCE hInstance, LPDWORD pData, int type, int resW, int resH, int resD)
{
	LPBYTE		res;
	DWORD		resHdrSize;
	DWORD		resDataSize;
	int			w, h, nCursors;
	int			xHot, yHot;
	WORD		rttype, reserved;

	LPZBITMAP	prb;
	
	BITMAPINFOHEADER bi;
	BOOL			 invert, hasmask;

	int			planes;
	int			bytes;
	LPCTSTR 	id, minid;
	int			bpp;
	int			stride, mstride;
	int			colors, ccount;
	
	char*	imgsrc;
	char*	masksrc;
	
	DWORD	palette[256];
	int		i;
	
	int		diff, mindiff;
	
	res = (LPBYTE)pData;
	
	// read resource header
	//
	resDataSize = RESDWORD(res);
	resHdrSize  = RESDWORD(res);
	
	res = (LPBYTE)pData + resHdrSize;

	if(type != RT_Bitmap)
	{
		unsigned long long idi;

		// read cursor/icon group header
		//
		reserved = RESWORD(res);			// reserved
		rttype   = RESWORD(res);	// RES_CURSOR or RES_ICON
		nCursors = RESWORD(res);
		
		if(resW <= 0) resW = (rttype == RES_CURSOR) ? _zg_cx_cursor : _zg_cx_icon;
		if(resH <= 0) resH = (rttype == RES_CURSOR) ? _zg_cx_cursor : _zg_cx_icon;
		if(resD <= 0) resD = _zg_imgdepth;
		
		mindiff = 0x7fff;
		minid   = NULL;

		// find the cursor/icon that best matches the depth requested
		// and if same depth, the closest size, or if no depth, the largest size
		//
		while(nCursors-- > 0)
		{
			// RESDIR struct
			// CURSORDIR/ICONDIR struct
			w 	   = RESBYTE(res);
			h 	   = RESBYTE(res);
			ccount = RESBYTE(res);
			reserved = (WORD)RESBYTE(res);
			planes = RESWORD(res);
			bpp    = RESWORD(res);
			bytes  = RESDWORD(res);
			idi	   = RESWORD(res);
	
			id = (LPCTSTR)idi;
			if(minid == NULL) minid = id;	// fallback if none match
			
			// ok, the ID here is the id of the real
			// cursor resource, so load that if it
			// matches the resolution required
			//
			//_tprintf(_T("looked icon %d=%d,%d,%d  for req %d,%d,%d\n"),
			//		id, w, h, bpp, resW, resH, resD);
			
			if((w == resW) && (h == resH || h == 0) && resD != 0)
			{
				// size specified, find closes match on depth
				//
				if(bpp < _zg_imgdepth)
					diff = resD - bpp;
				else
					diff = bpp - resD;
				
				if(diff < mindiff)
				{
					minid = id;
					mindiff = diff;
				}
				// exact depth match and width match, all set
				if(diff == 0)
					break;
			}
			else if(resD == 0)
			{
				diff = w * h;
				if(diff > mindiff || mindiff == 0x7fff)
				{
					minid = id;
					mindiff = diff;
				}
			}
		}	
		if(minid)
		{
			//_tprintf(_T("minid=%d diff=%d\n"), minid, mindiff);
			if(! (pData = __FindResource(
				(rttype == RES_CURSOR ? RT_Cursor : RT_Icon), minid,
				_zg_resources, _cb_zg_resources)))
			{
				return NULL;
			}
			// read resource header
			//
			res = (LPBYTE)pData;
			resDataSize = RESDWORD(res);
			resHdrSize  = RESDWORD(res);
			res = (LPBYTE)pData + resHdrSize;
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		id = _T("_xx_");
	}
	if(type == RT_Cursor)
	{
		// there is an RT_CURSOR or RT_ICON data section now
		//
		if(rttype == RES_CURSOR)
		{
			// get hot spot
			//
			xHot = RESWORD(res);
			yHot = RESWORD(res);
		}
	}
	// get the bitmap info header
	//
	bi.biSize			= RESDWORD(res);
	bi.biWidth			= (LONG)RESDWORD(res);
	bi.biHeight			= (LONG)RESDWORD(res);
	bi.biPlanes			= RESWORD(res);
	bi.biBitCount		= RESWORD(res);
	bi.biCompression	= RESDWORD(res);
	bi.biSizeImage		= RESDWORD(res);
	bi.biXPelsPerMeter	= RESDWORD(res);
	bi.biYPelsPerMeter	= RESDWORD(res);
	bi.biClrUsed		= RESDWORD(res);
	bi.biClrImportant	= RESDWORD(res);

	w = bi.biWidth;
	h = bi.biHeight;
				
	if(h < 0)
	{
		h = - h;
		invert = FALSE;
	}
	else
	{	
		invert = TRUE;
	}
	if(h == 2 * w)
	{
		h = h / 2;
		hasmask = TRUE;
	}
	else
	{
		hasmask = FALSE;
	}
				
	// calculate number of color components needed
	//
	bpp 	 = bi.biBitCount;
	stride	 = (w * bpp + 7) / 8;

	//_tprintf(_T("selected icon =%d,%d,%d\n"), w, h, bpp);

	mstride = (w + 7) / 8;

	// windows images are always dword aligned regardless of depth
	//
	stride += 3;
	stride &= ~3;
	mstride += 3;
	mstride &= ~3;

	switch(bpp)
	{
	case 1:	colors = 2;   	break;
	case 4: colors = 16; 	break;
	case 8: colors = 256;	break;
	case 16:
	case 24:
	case 32:
	default: colors = 0; 	break;
	}
	
	// read in color componenents as RGB QUADS
	//
	if(colors > 0)
	{
		LPDWORD pPal = (LPDWORD)palette;
		BYTE r, g, b, v;
		DWORD pc;

		for(i = 0; i < colors; i++)
		{
			b = RESBYTE(res);
			g = RESBYTE(res);
			r = RESBYTE(res);
			v = RESBYTE(res);
			pc = ((DWORD)r << 16) | ((DWORD)g << 8) | ((DWORD)b);
			*pPal++ = pc;
		}
	}
	// point to image data in resource
	imgsrc  = (char*)res;
	
	if(type != RT_Bitmap)
		masksrc = (char*)res + (h * stride);
	else
		masksrc = NULL;
	
	// create bitmap or cursor for image
	//
	if(type != RT_Bitmap && rttype == RES_CURSOR)
		prb = CreateCursor(hInstance, xHot, yHot, w, h, imgsrc, masksrc);
	else
		prb = _w_createImage(type, &bi, (LPBYTE)imgsrc, (LPBYTE)masksrc, palette, colors);
	if(prb)
	{
		prb->id = id;
	}
	return prb;
}

//**************************************************************************
HANDLE WINAPI LoadImage(HINSTANCE hInstance, LPCTSTR lpImageName, UINT itype, int w, int h, UINT n)
{
	HBITMAP		hBitmap;
	LPZBITMAP	zBitmap;
	HICON		hIcon;
	LPZCURSOR	zIcon;
	LPDWORD 	pData;
	LPCTSTR		stdIcon;
	HCURSOR		hCursor;
	LPZCURSOR	zCursor;
	LPCTSTR 	stdCursor;

	int         d = _zg_imgdepth;
	
	switch(itype)
	{
	case IMAGE_BITMAP:
		if((hBitmap = (HBITMAP)_w_findGDIimage(RT_Bitmap, lpImageName)) != NULL)
		{
			return (HANDLE)_w_refZGDIOBJ(hBitmap);
		}
		if(! (pData = __FindResource(RT_Bitmap, lpImageName, _zg_resources, _cb_zg_resources)))
		{
			return NULL;
		}
		zBitmap = _w_loadImage(hInstance, pData, RT_Bitmap, 0, 0, 0);
		if(zBitmap)
		{
			zBitmap->id = lpImageName;
			hBitmap = (HBITMAP)_w_newZGDIOBJ(gdiBitmap, zBitmap);
			return (HANDLE)hBitmap;
		}
		return NULL;
	case IMAGE_ICON:
		if((hIcon = (HICON)_w_findGDIimage(RT_Icon, lpImageName)) != NULL)
		{
			return (HANDLE)_w_refZGDIOBJ(hIcon);
		}
		// see if its a built in icon
		//
		stdIcon = NULL;
	#if 0	
		switch((DWORD)lpImageName)
		{
		case (DWORD)IDC_IBEAM:
			stdCursor = (LPCTSTR)XC_xterm;
			break;
		case (DWORD)IDC_ARROW:
			stdCursor = (LPCTSTR)XC_left_ptr;
			break;
		default:
			break;
		}
	#endif
		if(! (pData = __FindResource(RT_GroupIcon, lpImageName, _zg_resources, _cb_zg_resources)))
		{
			return NULL;
		}
		zIcon = _w_loadImage(hInstance, pData, RT_Icon, w, h, 0);
		if(zIcon)
		{
			hIcon = (HICON)_w_newZGDIOBJ(gdiBitmap, zIcon);
			return (HANDLE)hIcon;
		}
		return NULL;
	case IMAGE_CURSOR:
		if((hCursor = (HCURSOR)_w_findGDIimage(RT_Cursor, lpImageName)) != NULL)
		{
			return (HANDLE)_w_refZGDIOBJ(hCursor);
		}
		// see if its a built in cursor
		//
		stdCursor = lpImageName;
		if(stdCursor < (LPCTSTR)IDC_ARROW || stdCursor > (LPCTSTR)IDC_SIZENS)
			stdCursor = NULL;
		if(stdCursor)
		{
			Cursor cursor;
			cursor = _w_createCursor(stdCursor, NULL, 0, 0);
			zCursor = _w_newZCURSOR(lpImageName, NULL, NULL, 0, 0, _zg_imgdepth, 0, 0, cursor);
		}
		else
		{
			if(! (pData = __FindResource(RT_GroupCursor, lpImageName, _zg_resources, _cb_zg_resources)))
			{
				return NULL;
			}
			zCursor= _w_loadImage(hInstance, pData, RT_Cursor, 0, 0, 1);
		}
		if(zCursor)
		{
			hCursor = (HCURSOR)_w_newZGDIOBJ(gdiBitmap, zCursor);
			return (HANDLE)hCursor;
		}
		return NULL;
	default:
		break;
	}
	return NULL;
}

//**************************************************************************
HCURSOR WINAPI CreateCursor(HINSTANCE hInstance, int xHot, int yHot, int w, int h, const void* pAND, const void* pXOR)
{
	HCURSOR		hCursor;
	LPZCURSOR	zCursor;
	char		*imgdata = NULL, *maskdata = NULL;
	int			i, j, k, ii, y, l, invert;
	Cursor		cursor;
#ifdef X11
	Pixmap		source, mask;
	XColor		fg, bg;
#endif
	invert = 0;
	ii = (w + 7) / 8;
	imgdata  = (char*)malloc(ii * h);
	maskdata = (char*)malloc(ii * h);
	if(invert)		k = ii * (h - 1);
	else			k = 0;

#ifdef X11	
	if(imgdata && maskdata)
	{
		for(y = l = 0; y < h; y++, k += ((invert) ? -ii : ii), l += ii)
		{
			for(i = 0, j = k + ii - 1; i < w / 8; i++, j--)
			{
				imgdata[l + i]  = ((char*)pXOR)[j];
				maskdata[l + i] = ((char*)pAND)[j];
			}
		}
		// create pixmaps from the data
		source	= XCreatePixmapFromBitmapData(_zg_display, DefaultRootWindow(_zg_display), imgdata, w, h, 0, 1, 1);
		mask	= XCreatePixmapFromBitmapData(_zg_display, DefaultRootWindow(_zg_display), maskdata, w, h, 0, 1, 1);
		fg.red = fg.green = fg.blue = 0;
		bg.red = bg.green = bg.blue = 0xffff;
		// create a cursor from the pixmaps
		cursor = XCreatePixmapCursor(
									_zg_display,
									source,
									mask,
									&fg,
									&bg,
									xHot,
									yHot
									);
		free(imgdata);
		free(maskdata);
	}																					
	if(source) XFreePixmap(_zg_display, source);
	if(mask)   XFreePixmap(_zg_display, mask);
#else
	cursor = NULL;
#endif
	zCursor = _w_newZCURSOR(_T("xxxx"), (char*)pXOR, (char*)pAND, w, h, _zg_imgdepth, xHot, yHot, cursor);
	if(zCursor)
	{
		hCursor = (HCURSOR)_w_newZGDIOBJ(gdiBitmap, zCursor);
		return hCursor;
	}
	return NULL;
}

//**************************************************************************
HBITMAP WINAPI LoadBitmap(HINSTANCE hInstance, LPCTSTR lpBitmapName)
{
	return (HICON)LoadImage(hInstance, lpBitmapName, IMAGE_BITMAP, 0, 0, 0);
}

//**************************************************************************
HICON WINAPI LoadIcon(HINSTANCE hInstance, LPCTSTR lpIconName)
{
	return (HICON)LoadImage(hInstance, lpIconName, IMAGE_ICON, 0, 0, 0);
}

//**************************************************************************
HCURSOR WINAPI LoadCursor(HINSTANCE hInstance, LPCTSTR lpCursorName)
{
	return (HICON)LoadImage(hInstance, lpCursorName, IMAGE_CURSOR, 0, 0, 0);
}		

//**************************************************************************
HBITMAP WINAPI CreateBitmap(int w, int h, UINT planes, UINT bpp, CONST void* lpvBits)
{
	HBITMAP		hBitmap;
	LPZBITMAP	zBitmap;
	BITMAPINFOHEADER bi;

	if(w == 0 || h == 0) { w = h = 1; }

	bi.biSize			= sizeof(bi);
	bi.biWidth			= w;
	bi.biHeight			= h;
	bi.biPlanes			= planes;
	bi.biBitCount		= bpp;
	bi.biCompression	= BI_RGB;
	bi.biSizeImage		= 0;
	bi.biXPelsPerMeter	= 0;
	bi.biYPelsPerMeter	= 0;
	bi.biClrUsed		= 0;
	bi.biClrImportant	= 0;

	zBitmap = _w_createImage(RT_Bitmap, &bi, lpvBits, NULL, NULL, 0);
	if(zBitmap)
	{
		hBitmap = (HBITMAP)_w_newZGDIOBJ(gdiBitmap, zBitmap);
		return hBitmap;
	}
	return NULL;
}

//**************************************************************************
HBITMAP WINAPI CreateCompatibleBitmap(HDC hdc, int w, int h)
{
	LPBYTE pimg;
	int    d, s;
	
	if(w == 0 || h == 0) { w = h = 1; }

	d = GetDeviceCaps(hdc, BITSPIXEL);
	s = (w * d + 7) / 8;
	s += 3;
	s &= ~3;
	pimg = (LPBYTE)malloc(s * h);
	return CreateBitmap(w, h, 1, d, pimg); 
}

//**************************************************************************
LONG WINAPI GetBitmapBits(HBITMAP hbm, LONG nBytes, void *lpvBits)
{
	LPZGDIOBJ zGDIO;
	LPZBITMAP zBitmap = NULL;
	int		  w, h, d, s, tot;
	
	if(! hbm)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	if((zGDIO = (LPZGDIOBJ)hbm) != NULL)
		if(zGDIO->type == gdiBitmap)
			zBitmap = (LPZBITMAP)(zGDIO->obj);
	w = zBitmap->w;
	h = zBitmap->h;
	d = zBitmap->d;
	s = (w * d + 7) / 8;
	s += 3;
	s &= ~ 3;
	
	tot = s * h;
	
	if(nBytes > tot)
		nBytes = tot;
	if(nBytes > 0)
		memcpy(lpvBits, zBitmap->data, nBytes);
	return nBytes;
}


//**************************************************************************
int SetDIBits(HDC hdc, HBITMAP hbm, UINT fl, UINT ll, CONST void* lpbBits, CONST BITMAPINFO* lpbmi, UINT fuColorUse)
{
	LPZGDIOBJ zGDIO;
	LPZBITMAP zBitmap = NULL;
	int		  w, h, d, s;
	int		  sw, sh, sd, ss;
	int		  invert, colors;
	LPBYTE    sp, dp;
	LPDWORD	  palette;
	
	if((zGDIO = (LPZGDIOBJ)hbm) != NULL)
		if(zGDIO->type == gdiBitmap)
			zBitmap = (LPZBITMAP)(zGDIO->obj);

	if(! zBitmap)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	w = zBitmap->w;
	h = zBitmap->h;
	d = zBitmap->d;
	s = (w * d + 7) / 8;
	s += 3;
	s &= ~ 3;
	
	sw = lpbmi->bmiHeader.biWidth;
	sh = lpbmi->bmiHeader.biHeight;
	if(sh == 2*sw)
		sh /= 2;
	sd = lpbmi->bmiHeader.biBitCount;
	ss = (sw * sd + 7) / 8;
	ss += 3;
	ss &= ~ 3;

	if(sw < w) w = sw;
	if(sh < h) h = sh;
	
	if(h < 0)
	{
		h = -h;
		invert = FALSE;
	}
	else
	{
		invert = TRUE;
	}
#ifdef INV_COCOA
	invert = !invert;
#endif
	palette = (LPDWORD)&lpbmi->bmiColors[0];
	colors  = lpbmi->bmiHeader.biClrUsed;
	
	if(! zBitmap->data)
	{
		zBitmap->data = (char*)malloc(s * h);
#ifdef X11		
		if(! zBitmap->ximg)
		{
			zBitmap->ximg = XCreateImage(
						_zg_display,
						_zg_visual,
						_zg_compositing ?
							_zg_vinfo.depth :
							DefaultDepth(_zg_display, DefaultScreen(_zg_display)),
						ZPixmap,
						0,
						zBitmap->data,
						w, h,
						_zg_imgdepth > 8 ? 32 : 8,
						0
					);
		}
#endif
	}
	sp = (LPBYTE)lpbBits;
	dp = (LPBYTE)zBitmap->data;

	switch(sd)
	{
	case 1:
		break;
	case 4:
		switch(d)
		{
		case 1:
			break;
		case 8:
			_ifr_4_8(dp, s, sp, ss, w, h, invert, palette);
			break;
		case 16:
			_ifr_4_16(dp, s, sp, ss, w, h, invert, palette);
			break;
		case 24:
			_ifr_4_24(dp, s, sp, ss, w, h, invert, palette);
			break;
		case 32:
			_ifr_4_32(dp, s, sp, ss, w, h, invert, palette);
			break;
		}
		break;
	case 8:
		switch(_zg_imgdepth)
		{
		case 1:
			break;
		case 8:
			_ifr_8_8(dp, s, sp, ss, w, h, invert, palette);
			break;
		case 16:
			_ifr_8_16(dp, s, sp, ss, w, h, invert, palette);
			break;
		case 24:
			_ifr_8_24(dp, s, sp, ss, w, h, invert, palette);
			break;
		case 32:
			_ifr_8_32(dp, s, sp, ss, w, h, invert, palette);
			break;
		}
		break;
	case 16:
		break;
	case 24:
		switch(_zg_imgdepth)
		{
		case 1:
			break;
		case 8:
			_ifr_24_8(dp, s, sp, ss, w, h, invert, palette);
			break;
		case 16:
			_ifr_24_16(dp, s, sp, ss, w, h, invert, palette);
			break;
		case 24:
			_ifr_24_24(dp, s, sp, ss, w, h, invert, palette);
			break;
		case 32:
			_ifr_24_32(dp, s, sp, ss, w, h, invert, palette);
			break;
		}
		break;
	}
	return 0;
}

//**************************************************************************
HBITMAP WINAPI CreateDIBitmap(HDC hdc, CONST BITMAPINFOHEADER *lpbmih, DWORD fdwInit, CONST void *lpbInit, CONST BITMAPINFO *lpbmi, UINT fuUsage)
{
	HBITMAP		hBitmap;

	if(! hdc || ! lpbmih || ((fdwInit & CBM_INIT)&& (!lpbInit || !lpbInit)))
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return NULL;
	}
	hBitmap = CreateCompatibleBitmap(hdc, lpbmih->biWidth, lpbmih->biHeight); 

	if(fdwInit & CBM_INIT)
	{
		SetDIBits(hdc, hBitmap, 0, lpbmi->bmiHeader.biHeight, lpbInit, lpbmi, fuUsage);
	}
	return hBitmap;
}

//**************************************************************************
HCURSOR WINAPI SetCursor(HCURSOR hCursor)
{
	extern HCURSOR _zg_hOldCursor;
	
	return _zg_hOldCursor;
}

//**************************************************************************
void __set_wnd_cursor(HWND hWnd, HCURSOR hCursor)
{
	LPZGDIOBJ zObj;
	LPZCURSOR zCursor;
	LPZWND	  zWnd;
	
	if(! (zWnd = (LPZWND)hWnd))				return;
	if((zObj = (LPZGDIOBJ)hCursor) == NULL) return;
	if(zObj->tag != ZGDIO_TAG) 				return;
	
	if((zCursor = (LPZCURSOR)(zObj->obj)) != NULL)
	{
		if(zCursor->type == RT_Cursor)
		{
#ifdef X11
			XDefineCursor(_zg_display, zWnd->xWnd, zCursor->cursor);
#endif
		}
	}
}

//**************************************************************************
static BOOL _w_setIconBkg(LPZICON zIcon, DWORD pixel, BOOL invert)
{
	LPBYTE 		pib, pm, pmb;
	BYTE    	mask, mb;
	int			istride, mstride;
	int			x, y;

	if(zIcon->type != RT_Icon)    		 	return FALSE;
	if(! zIcon->mask) 			   			return FALSE;	

	if(zIcon->ibkset && zIcon->ibkg == pixel) return TRUE;
	
	istride = (zIcon->w * zIcon->d + 7) / 8;
	mstride = (zIcon->w + 7) / 8;

	istride += 3;
	istride &= ~3;
	mstride += 3;
	mstride &= ~3;
	
	pib = (LPBYTE)zIcon->data;
	if(invert)
	{
		pib += (zIcon->h - 1) * istride;
		istride = -istride;
	}
	pmb = (LPBYTE)zIcon->mask;
	
	if(! pmb) return FALSE;
	
	switch(zIcon->d)
	{
	case 1:
		{
			LPBYTE po;
			
			for(y = 0; y < zIcon->h; y++)
			{			
				po = pib;
				pm = pmb;
				
				for(x = 0; x < zIcon->w / 8; x++, mask >>= 1, po++, pm++)
				{
					*po &= ~*pm;
					if(pixel) *po |= *pm;
				}
				pib += istride;
				pmb += mstride;
			}
		}
		break;
	case 4:
	case 8:
		{
			LPBYTE po;
			
			for(y = 0; y < zIcon->h; y++)
			{			
				po = pib;
				pm = pmb;
				mask = 0x0;
				
				for(x = 0; x < zIcon->w; x++, mask >>= 1, po++)
				{
					if(! mask)
					{
						mask = 0x80;
						mb = *pm++;
					}
					if(mb & mask)
						*po = (BYTE)pixel;
				}
				pib += istride;
				pmb += mstride;
			}
		}
		break;
	case 16:
		{
			LPWORD po;
			
			for(y = 0; y < zIcon->h; y++)
			{			
				po = (LPWORD)pib;
				pm = pmb;
				mask = 0x0;
				
				for(x = 0; x < zIcon->w; x++, mask >>= 1, po++)
				{
					if(! mask)
					{
						mask = 0x80;
						mb = *pm++;
					}
					if(mb & mask)
						*po = (WORD)pixel;
				}
				pib += istride;
				pmb += mstride;
			}
		}
		break;
	case 24:
		{
			LPBYTE po;
			
			for(y = 0; y < zIcon->h; y++)
			{			
				po = pib;
				pm = pmb;
				mask = 0x0;
				
				for(x = 0; x < zIcon->w; x++, mask >>= 1, po++)
				{
					if(! mask)
					{
						mask = 0x80;
						mb = *pm++;
					}
					if(mb & mask)
					{
						*po++ = (BYTE)(pixel >> 16);
						*po++ = (BYTE)(pixel >> 8);
						*po++ = (BYTE)pixel;
					}
				}
				pib += istride;
				pmb += mstride;
			}
		}
		break;
	case 32:
		{
			LPDWORD po;
			
			for(y = 0; y < zIcon->h; y++)
			{			
				po = (LPDWORD)pib;
				pm = pmb;
				mask = 0x0;
				
				for(x = 0; x < zIcon->w; x++, mask >>= 1, po++)
				{
					if(! mask)
					{
						mask = 0x80;
						mb = *pm++;
					}
					if(mb & mask)
						*po = pixel;
				}
				pib += istride;
				pmb += mstride;
			}
		}
		break;
	}
	zIcon->ibkset = TRUE;
	zIcon->ibkg    = pixel;
	return TRUE;
}

//**************************************************************************
BOOL WINAPI DrawIcon(HDC hdc, int x, int y, HICON hIcon)
{
	LPZGDIOBJ 	zObj;
	LPZGC 		zGC;
	LPZICON 	zIcon;
	DWORD		xcolor;
	
	if(! (zGC = (LPZGC)hdc))				return FALSE;
	if((zObj = (LPZGDIOBJ)hIcon) == NULL) 	return FALSE;
	if(zObj->tag != ZGDIO_TAG) 				return FALSE;

	if(! (zIcon = (LPZICON)(zObj->obj))) 	return FALSE;
	
	if(zIcon->type == RT_Icon)
	{
		xcolor = _z_x_colorofbrush(zGC->textbkg);
		if(zGC->zWnd && (xcolor & 0xFF000000) == 0)
			xcolor |= (zGC->zWnd->key & 0xFF000000);
		_w_setIconBkg(zIcon, xcolor, 0);
	}
	else if(zIcon->type == RT_Cursor)
	{
		return FALSE;
	}
	_w_drawImage(zGC->zDrvr, zIcon, x, y, zIcon->w, zIcon->h);
	return TRUE;
}

//**************************************************************************
BOOL WINAPI BitBlt(HDC hdc, int x, int y, int w, int h, HDC hdcs, int xs, int ys, DWORD rop)
{
	LPZGDIOBJ 	zObj;
	LPZGC 		zGC, zGCS;
	LPZBITMAP 	zBitmap;
	
	if(! (zGC  = (LPZGC)hdc))				return FALSE;
	if(! (zGCS = (LPZGC)hdcs))				return FALSE;
	
	if(! (zObj = (LPZGDIOBJ)(zGCS->hBITMAP)))	return FALSE;
	if(! (zBitmap = (LPZBITMAP)(zObj->obj)))	return FALSE;

	_w_drawImage(zGC->zDrvr, zBitmap, x, y, w, h);

	return TRUE;
}

//**************************************************************************
BOOL WINAPI GetIconInfo(HICON hIcon, PICONINFO pii)
{
	LPZGDIOBJ 	zObj;
	LPZICON 	zIcon;
	static		ZBITMAP imgObj  = { 0, 0, 0, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0 };
	static		ZGDIOBJ maskObj = { ZGDIO_TAG, 0, gdiBitmap, &imgObj, NULL };
	
	if(! pii)								return FALSE;	
	if((zObj = (LPZGDIOBJ)hIcon) == NULL) 	return FALSE;
	if(! (zIcon = (LPZICON)(zObj->obj))) 	return FALSE;
	
	pii->fIcon = zIcon->type == RT_Icon;
	pii->xHotspot = zIcon->xHot;
	pii->yHotspot = zIcon->yHot;
	imgObj = *zIcon;
	imgObj.data = zIcon->mask;
	imgObj.mask = NULL;
	pii->hbmMask  = (HBITMAP)&maskObj;
	pii->hbmColor = (HBITMAP)hIcon;

	return TRUE;
}

//**************************************************************************
BOOL WINAPI DestroyCursor(HCURSOR hCursor)
{
	return DeleteObject(hCursor);
}

//**************************************************************************
BOOL WINAPI DestroyIcon(HICON hIcon)
{
	return DeleteObject(hIcon);
}

//**************************************************************************
BOOL _w_setIcon(LPZWND zWnd, HICON hIcon, LPCTSTR lpWindowName)
{
	LPZGDIOBJ 	zObj;
	LPZICON 	zIcon;

	if(! zWnd || ! zWnd->xWnd)	 			return FALSE;
	if((zObj = (LPZGDIOBJ)hIcon) == NULL) 	return FALSE;
	if(zObj->tag != ZGDIO_TAG) 				return FALSE;

	if(! (zIcon = (LPZICON)(zObj->obj))) 	return FALSE;
	if(zIcon->type != RT_Icon)    		 	return FALSE;
	
	_w_setAppIcon(zWnd, zIcon, lpWindowName);
	return TRUE;
}

//-----------------------------------------------------------------------------

typedef enum { _zitIcon, _zitBitmap } zitType;

typedef struct tagIlistEntry
{
	LPZBITMAP	pImage;
	LPZBITMAP	pMask;
	struct tagIlistEntry* next;
}
_zile, FAR* _pzile;

//-----------------------------------------------------------------------------

typedef struct tagImageList
{
	DWORD		tag;
	int			w, h, d;
	int			n, f, g;
	_pzile  	pImages;
	COLORREF	frg, bkg;
	BOOL		frgset, bkgset;
	struct tagImageList* next;
}
ZIMGLIST, *PZIMGLIST, FAR *LPZIMGLIST;

#define ZIMGLIST_TAG 0xfeed000A

//**************************************************************************
LPZIMGLIST _w_newZIMGLIST(int cx, int cy, UINT flags, int cInitial, int cGrow)
{
	LPZIMGLIST zList;
	
	if(! (zList = (LPZIMGLIST)malloc(sizeof(ZIMGLIST))))
		return NULL;

	zList->tag	= ZIMGLIST_TAG;	
	zList->w 	= cx;
	zList->h	= cy;
	zList->d	= _zg_imgdepth;
	zList->f	= flags;
	zList->n	= 0;
	zList->g	= cGrow;
	zList->pImages 	= NULL;
	zList->frg	   	= 0;
	zList->bkg		= 0;
	zList->frgset	= FALSE;
	zList->bkgset	= FALSE;
	zList->next	= (LPZIMGLIST)_zg_imagelists;
	_zg_imagelists	= zList;
	return zList;
}

//**************************************************************************
void _w_deleteZIMGLIST(LPZIMGLIST zList)
{
	LPZIMGLIST zl;
	_pzile	   pl;
		
	if(! zList || (zList->tag != ZIMGLIST_TAG))
		return;

	if(_zg_imagelists == zList)
		_zg_imagelists = zList->next;
	else
	{
		for(zl = _zg_imagelists; zl && zl->next != zList;)
			zl = zl->next;
		if(zl) zl->next = zList->next;
	}
	for(pl = zList->pImages; pl; pl = pl->next)
	{
		if(pl->pImage)
			DeleteObject(pl->pImage);
		if(pl->pMask)
			DeleteObject(pl->pMask);
	}
	free(zList);
}

//**************************************************************************
HIMAGELIST WINAPI ImageList_Create(int cx, int cy, UINT flags, int cInitial, int cGrow)
{
	LPZIMGLIST zList;
	
	if(cx <= 0 || cy <= 0 || cInitial < 0)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
	}
	if(! (zList = _w_newZIMGLIST(cx, cy, flags, cInitial, cGrow)))
		return NULL;
	return (HIMAGELIST)zList;	
}

//**************************************************************************
BOOL WINAPI ImageList_Destroy(HIMAGELIST himl)
{
	LPZIMGLIST zList;
		
	if(! (zList = (LPZIMGLIST)himl))
		return FALSE;
   if(zList->tag != ZIMGLIST_TAG)
		return FALSE;
	_w_deleteZIMGLIST(zList);
	return TRUE;
}

//**************************************************************************
BOOL WINAPI ImageList_Draw(HIMAGELIST himl, int j, HDC hdc, int x, int y, UINT fStyle)
{
	LPZIMGLIST zList;
	_pzile	   pl;
	int		   i;
		
	if(! (zList = (LPZIMGLIST)himl))
		return FALSE;
	if(zList->tag != ZIMGLIST_TAG)
		return FALSE;
	if(zList->pImages && j >= 0)
	{
		for(pl = zList->pImages, i = 0; pl->next; pl = pl->next, i++)
			if(i == j)
				break;
		if((i == j) && pl)
			if(pl->pImage)
			{
				DrawIcon(hdc, x, y, (HICON)(pl->pImage));
				return TRUE;
			}
	}
	return FALSE;
}

//**************************************************************************
int WINAPI ImageList_GetImageCount(HIMAGELIST himl)
{
	LPZIMGLIST zList;
	_pzile	 	pl;
	int    		i;
	
		
	if(! (zList = (LPZIMGLIST)himl))
		return FALSE;
	if(zList->tag != ZIMGLIST_TAG)
		return FALSE;
	for(pl = zList->pImages, i = 0; pl; pl = pl->next)
		i++;
	return i;
}

//**************************************************************************
BOOL WINAPI ImageList_SetImageCount(HIMAGELIST himl, UINT uNewCount)
{
	return TRUE;
}

//**************************************************************************
int WINAPI ImageList_Add(HIMAGELIST himl, HBITMAP hbmImage, HBITMAP hbmMask)
{
	LPZIMGLIST zList;
	_pzile pn;
	_pzile pl;
	int    i;
			
	if(! (zList = (LPZIMGLIST)himl))
		return FALSE;
	if(zList->tag != ZIMGLIST_TAG)
		return FALSE;

	if(! (pn = (_pzile)malloc(sizeof(_zile))))
		return -1;
	pn->pImage 	= (LPZBITMAP)hbmImage;
	pn->pMask  	= (LPZBITMAP)hbmMask;
	pn->next 	= NULL;
	if(! zList->pImages)
	{
		zList->pImages = pn;
		i = 0;
	}
	else
	{
		for(pl = zList->pImages, i = 0; pl->next; pl = pl->next)
			i++;
		pl->next = pn;
	}
	_w_refZGDIOBJ(hbmImage);
	if(hbmMask)
		_w_refZGDIOBJ(hbmMask);
	return i;
}

//**************************************************************************
int WINAPI ImageList_ReplaceIcon(HIMAGELIST himl, int j, HICON hicon)
{
	LPZIMGLIST zList;
	_pzile	   pl;
	int		   i = 0;
	HBITMAP	   hbmOld;
			
	if(! (zList = (LPZIMGLIST)himl))
		return FALSE;
	if(zList->tag != ZIMGLIST_TAG)
		return FALSE;
	if(zList->pImages && j >= 0)
	{
		for(pl = zList->pImages; pl->next; pl = pl->next, i++)
			if(i == j)
				break;
	}
	if(i != j || ! zList->pImages)
		return ImageList_Add(himl, (HBITMAP)hicon, NULL);

	_w_refZGDIOBJ(hicon);
		
	hbmOld = pl->pImage;
	pl->pImage = (HBITMAP)hicon;
	if(hbmOld) DeleteObject(hbmOld);

	hbmOld = pl->pMask;
	pl->pMask = NULL;
	if(hbmOld) DeleteObject(hbmOld);
	return i;
}

//**************************************************************************
COLORREF WINAPI ImageList_SetBkColor(HIMAGELIST himl, COLORREF clrBk)
{
	LPZIMGLIST zList;
	COLORREF   oldBkg;
		
	if(! (zList = (LPZIMGLIST)himl))
		return FALSE;
    if(zList->tag != ZIMGLIST_TAG)
		return FALSE;
	oldBkg = zList->bkg;
	zList->bkg = clrBk;
	return oldBkg;;
}

//**************************************************************************
COLORREF WINAPI ImageList_GetBkColor(HIMAGELIST himl)
{
	LPZIMGLIST zList;
		
	if(! (zList = (LPZIMGLIST)himl))
		return FALSE;
	if(zList->tag != ZIMGLIST_TAG)
		return FALSE;
	return zList->bkg;
}

//**************************************************************************
BOOL WINAPI ImageList_SetOverlayImage(HIMAGELIST himl, int iImage, int iOverlay)
{
	return FALSE;
}

//**************************************************************************
BOOL WINAPI ImageList_GetIconSize(HIMAGELIST himl, int FAR *cx, int FAR *cy)
{
	LPZIMGLIST zList;
		
	if(! (zList = (LPZIMGLIST)himl))
		return FALSE;
	if(zList->tag != ZIMGLIST_TAG)
		return FALSE;
	if(cx) *cx = zList->w;
	if(cy) *cy = zList->h;
	return TRUE;
}

//**************************************************************************
BOOL WINAPI ImageList_SetIconSize(HIMAGELIST himl, int cx, int cy)
{
	LPZIMGLIST zList;
		
	if(! (zList = (LPZIMGLIST)himl))
		return FALSE;
	if(zList->tag != ZIMGLIST_TAG)
		return FALSE;
	zList->w = cx;
	zList->h = cy;
	return TRUE;
}

//**************************************************************************
BOOL WINAPI ImageList_GetImageInfo(HIMAGELIST himl, int i, IMAGEINFO FAR* pImageInfo)
{
	return FALSE;
}

