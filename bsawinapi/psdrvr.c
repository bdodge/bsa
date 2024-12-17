
#include "winapix.h"

#define zEB(z)   ((LPSTR)((z)->drvrData[0]))
#define zFILE(z)  ((FILE*)((z)->drvrData[1]))
#define zZWND(z) ((LPZWND)((z)->drvrData[2]))
#define zCOLOR(z) ((int)(uintptr_t)((z)->drvrData[3]))
#define zPAGES(z) ((int)(uintptr_t)((z)->drvrData[4]))

//**************************************************************************
void _emit(LPZDRVR zDrvr, char* lpcmd)
{
	FILE* pjf = zFILE(zDrvr);

	if(! pjf) return;
	fputs(lpcmd, pjf);
}

//**************************************************************************
void _ps_setfrg(LPZDRVR zDrvr, HBRUSH hbrFrg)
{
	LPZGDIOBJ zObj;
	LPZCOLOR  zColor;
	char*     emitBuf;

	if(! zDrvr || ! (zObj = (LPZGDIOBJ)hbrFrg)) return;
	if(! (zColor = (LPZCOLOR)zObj->obj))        return;
	if(! (emitBuf = zEB(zDrvr)))			    return;

	if(zCOLOR(zDrvr))
		sprintf(emitBuf, "%f %f %f C\r\n",
						(double)zColor->r / 255.0,
						(double)zColor->g / 255.0,
						(double)zColor->b / 255.0
				);
	else
		sprintf(emitBuf, "%f G\r\n",
						(double)(zColor->r + zColor->g +  zColor->b) / (3.0 * 255.0)
				);
	_emit(zDrvr, emitBuf);
}

//**************************************************************************
void _ps_setbkg(LPZDRVR zDrvr,  HBRUSH hbrBkg)
{
}

//**************************************************************************
BOOL _ps_setFont(LPZDRVR zDrvr, HFONT hFont)
{
	LPZFONT		zFont;

	if(hFont)
	{
		char*     emitBuf;

		if(! zDrvr)						return FALSE;
		if(! (emitBuf = zEB(zDrvr))) 	return FALSE;

		zFont = (LPZFONT)(((LPZGDIOBJ)hFont)->obj);

		sprintf(emitBuf, "/Courier FF %ld CF SF\r\n",
				BSA_SCALE_CHAR_TO_PS(zFont->tm.tmAscent));

		_emit(zDrvr, emitBuf);
		return TRUE;
	}
	return FALSE;
}

//**************************************************************************
BOOL _ps_setPen(LPZDRVR zDrvr, HPEN hPen)
{
//	LPZGDIOBJ zObj;
//	LPZPEN	  zPen;

	return TRUE;
}

//**************************************************************************
BOOL _ps_textOut(LPZDRVR zDrvr, int x, int y, LPCTSTR lpText, int nText)
{
	LPZFONT		zFont;
	LPSTR 		pAnsi;
	int			h;
	char*		emitBuf;

	if(nText == 0 || ! lpText)	 return FALSE;
	if(! zDrvr)	      			 return FALSE;
	if(! (emitBuf = zEB(zDrvr))) return FALSE;

	zFont = NULL;
	if(zDrvr->zGC && zDrvr->zGC->hFONT)
		zFont = (LPZFONT)(((LPZGDIOBJ)(zDrvr->zGC->hFONT))->obj);

	if(! zFont)
		h = 13;
	else
		h = zFont->tm.tmAscent;

	if((pAnsi = (LPSTR)malloc(3*nText + sizeof(TCHAR))) != NULL)
	{
		int i, j;

		for(i = j = 0; i < nText; i++)
		{
			switch((char)lpText[i])
			{
			case '%':
			case '(':
			case ')':
			case '{':
			case '}':
			case '<':
			case '>':
			case '\\':
			case '[':
			case ']':
				j += sprintf(pAnsi + j, "\\%03o", lpText[i]);
				break;
			default:
				pAnsi[j++] = (char)lpText[i];
				break;
			}
		}
		pAnsi[j] = 0;
	}
	else
		return FALSE;

	y += h;
	sprintf(emitBuf, "%d %d M (", x, y);
	_emit(zDrvr, emitBuf);
	_emit(zDrvr, pAnsi);
	_emit(zDrvr, ") T\r\n");

	free(pAnsi);
	return TRUE;
}

//**************************************************************************
BOOL _ps_moveTo(LPZDRVR zDrvr, int x, int y)
{
	char*		emitBuf;

	if(! zDrvr)	      			 return FALSE;
	if(! (emitBuf = zEB(zDrvr))) return FALSE;

	sprintf(emitBuf, "%d %d M\r\n", x, y);
	_emit(zDrvr, emitBuf);
	return TRUE;
}

//**************************************************************************
BOOL _ps_lineTo(LPZDRVR zDrvr, int x, int y)
{
	char*		emitBuf;

	if(! zDrvr)	      			 return FALSE;
	if(! (emitBuf = zEB(zDrvr))) return FALSE;

	sprintf(emitBuf, "%d %d M\r\n", x, y);
	_emit(zDrvr, emitBuf);
	return TRUE;
}

//**************************************************************************
int	 _ps_fillRect(LPZDRVR zDrvr, LPRECT lprc, HBRUSH hbrush)
{
	char*		emitBuf;

	if(! zDrvr)	      			 return 0;
	if(! (emitBuf = zEB(zDrvr))) return 0;

	sprintf(emitBuf, "%d %d M\r\n", lprc->left, lprc->top);
	_emit(zDrvr, emitBuf);
	sprintf(emitBuf, "%d %d L %d %d L %d %d L closepath F N\r\n",
			lprc->right, lprc->top,
			lprc->right, lprc->bottom,
			lprc->left, lprc->bottom
			);
	_emit(zDrvr, emitBuf);
	return 1;
}

//**************************************************************************
BOOL _ps_bitblt(LPZDRVR zDrvr, int x, int y, int w, int h, HDC hdcSrc, int sx, int sy, DWORD rop)
{
	return TRUE;
}

//**************************************************************************
int _ps_devCaps(LPZDRVR zDrvr, int cap)
{
	switch(cap)
	{
	case DRIVERVERSION: 	     // Device driver version
		return 1;
	case TECHNOLOGY:    	     // Device classification
		return DT_RASPRINTER;
	case HORZSIZE:      	     // Horizontal size in millimeters
		return (zDrvr->vpw * 254) / (zDrvr->xres * 10);
	case VERTSIZE:      	     // Vertical size in millimeters
		return (zDrvr->vph * 254) / (zDrvr->yres * 10);
	case HORZRES:       	     // Horizontal width in pixels
		return zDrvr->vpw;
	case VERTRES:       	     // Vertical height in pixels
		return zDrvr->vph;
	case BITSPIXEL:     	     // Number of bits per pixel
		return zDrvr->vpd;
	case PLANES:        	     // Number of planes
		return 1;
	case NUMBRUSHES:    	     // Number of brushes the device has
	case NUMPENS:       	     // Number of pens the device has
	case NUMMARKERS:    	     // Number of markers the device has
	case NUMFONTS:      	     // Number of fonts the device has
	case NUMCOLORS:     	     // Number of colors the device supports
		return 0;
	case PDEVICESIZE:   	     // Size required for device descriptor
		return 0;
	case CURVECAPS:     	     // Curve capabilities
	case LINECAPS:      	     // Line capabilities
	case POLYGONALCAPS: 	     // Polygonal capabilities
	case TEXTCAPS:      	     // Text capabilities
	case CLIPCAPS:      	     // Clipping capabilities
		return 0;
	case RASTERCAPS:    	     // Bitblt capabilities
		return 0xff;
	case ASPECTX:       	     // Length of the X leg
		return 1;
	case ASPECTY:       	     // Length of the Y leg
		return 1;
	case ASPECTXY:      	     // Length of the hypotenuse
		return 1;
	case LOGPIXELSX:   		    // Logical pixels/inch in X
		return 72;
	case LOGPIXELSY:   		    // Logical pixels/inch in Y
		return 72;
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
BOOL _ps_startDoc(LPZDRVR zDrvr, CONST DOCINFO* pInfo)
{
	// output file
	zDrvr->drvrData[1] = stdout;
	// colordevice
	zDrvr->drvrData[3] = (LPVOID)(long)((zDrvr->vpd > 1) ? 1 : 0);
	// page count
	zDrvr->drvrData[4] = (LPVOID)(long)0;

	if(pInfo && pInfo->lpszOutput)
	{
		FILE* outf;
		char  fname[MAX_PATH];

		TCharToChar(fname, pInfo->lpszOutput);

		outf = fopen(fname, "wb");
		if(outf) zDrvr->drvrData[1] = outf;
	}

	// emit ps header
	//
	_emit(zDrvr, "%!PS-Adobe-3.0\r\n%%Creator: bsawinapi\r\n%%\r\n");
	_emit(zDrvr, "/M { moveto } def\r\n");
	_emit(zDrvr, "/R { rmoveto } def\r\n");
	_emit(zDrvr, "/L { lineto } def\r\n");
	_emit(zDrvr, "/C { setrgbcolor } def\r\n");
	_emit(zDrvr, "/G { setgray } def\r\n");
	_emit(zDrvr, "/S { stroke } def\r\n");
	_emit(zDrvr, "/F { fill } def\r\n");
	_emit(zDrvr, "/N { newpath } def\r\n");
	_emit(zDrvr, "/T { show } def\r\n");
	_emit(zDrvr, "/FF { findfont } def\r\n");
	_emit(zDrvr, "/CF { scalefont } def\r\n");
	_emit(zDrvr, "/SF { setfont } def\r\n");
	_emit(zDrvr, "%% - end of header\r\n");
	return TRUE;
}

//**************************************************************************
BOOL _ps_endDoc(LPZDRVR zDrvr)
{
	if(zFILE(zDrvr))
		if(zFILE(zDrvr) != stdout)
			fclose(zFILE(zDrvr));
	return TRUE;
}

//**************************************************************************
BOOL _ps_startPage(LPZDRVR zDrvr)
{
	char* emitBuffer;
	int page;

	emitBuffer 	= zEB(zDrvr);
	page		= zPAGES(zDrvr);
	zDrvr->drvrData[4] = (LPVOID)(long)++page;
	sprintf(emitBuffer, "%% - page %d\r\n", page);
	_emit(zDrvr, emitBuffer);
	return TRUE;
}

//**************************************************************************
BOOL _ps_endPage(LPZDRVR zDrvr)
{
	_emit(zDrvr, "showpage\r\n");
	return TRUE;
}

//**************************************************************************
void _w_deletePSDRVR(LPZDRVR zDrvr)
{
	if(! zDrvr) return;
	if(zEB(zDrvr)) free(zEB(zDrvr));
	free(zDrvr);
}

//**************************************************************************
LPZDRVR _w_newPSDRVR(LPZGC zGC, LPZWND zWnd)
{
	LPZDRVR zDrvr;
	int		papx, papy;
	int		papt, papb;
	int		papl, papr;

	if(! (zDrvr = (LPZDRVR)malloc(sizeof(ZDRVR))))
		return NULL;
	memset(zDrvr, 0, sizeof(ZDRVR));

	// postscript base units are 72 dpi (1/10 points)
	// which is fine really... paper table units are
	// in mm, so need to convert.  since yorg is flipped
	// top margin is already accounted for by using only
	// printable area of paper for vph  bottom margin
	// is used as a negative top margin, to move printed
	// area up on the page
	//
	papx = 800 * 254 / 1000;  	// 8.0 inches
	papy = 1050 * 254 / 1000;  	// 10.5 inches
	papt = 25 * 254 / 1000;		// top margin = 0.25 inches
	papb = 25 * 254 / 1000;		// bot margin = 0.25 inches
	papl = 25 * 254 / 1000;		// left margin = 0.25 inches
	papr = 25 * 254 / 1000;		// rght margin = 0.25 inches

	zDrvr->yorigTop = 0; // postscript org is bottom left

	zDrvr->xres = 72;
	zDrvr->yres = 72;
	zDrvr->xmar = papl;
	zDrvr->ymar = papb;
	zDrvr->vpw  =  papx * zDrvr->xres * 10 / 254;
	zDrvr->vph  =  papy * zDrvr->yres * 10 / 254;
	zDrvr->vpd  =  24; // color device

	zDrvr->drvrData[0] = malloc(256);
	zDrvr->drvrData[1] = zGC;
	zDrvr->drvrData[2] = zWnd;

	zDrvr->_delete 	= _w_deletePSDRVR;
	zDrvr->_setFrg 	= _ps_setfrg;
	zDrvr->_setBkg 	= _ps_setbkg;
	zDrvr->_setFont	= _ps_setFont;
	zDrvr->_setPen	= _ps_setPen;
	zDrvr->_textOut	= _ps_textOut;
	zDrvr->_moveTo	= _ps_moveTo;
	zDrvr->_lineTo	= _ps_lineTo;
	zDrvr->_fillRect= _ps_fillRect;
	zDrvr->_bitblt	= _ps_bitblt;
	zDrvr->_devCaps	= _ps_devCaps;

	zDrvr->_startDoc 	= _ps_startDoc;
	zDrvr->_endDoc 		= _ps_endDoc;
	zDrvr->_startPage 	= _ps_startPage;
	zDrvr->_endPage 	= _ps_endPage;

	return zDrvr;
}

