
#include "winapix.h"

#define zEB(z)   ((LPSTR)((z)->drvrData[0]))
#define zFILE(z)  ((FILE*)((z)->drvrData[1]))
#define zZWND(z) ((LPZWND)((z)->drvrData[2]))
#define zCOLOR(z) ((int)(uintptr_t)((z)->drvrData[3]))
#define zPAGES(z) ((int)(uintptr_t)((z)->drvrData[4]))

//**************************************************************************
extern void _emit(LPZDRVR zDrvr, char* lpcmd);

//**************************************************************************
void _pcl_setfrg(LPZDRVR zDrvr, HBRUSH hbrFrg)
{
}

//**************************************************************************
void _pcl_setbkg(LPZDRVR zDrvr,  HBRUSH hbrBkg)
{
}

//**************************************************************************
BOOL _pcl_setFont(LPZDRVR zDrvr, HFONT hFont)
{
	LPZFONT		zFont;

	if(hFont)
	{
		char*     emitBuf;

		if(! zDrvr)						return FALSE;
		if(! (emitBuf = zEB(zDrvr))) 	return FALSE;

		zFont = (LPZFONT)(((LPZGDIOBJ)hFont)->obj);

		sprintf(emitBuf, "\033(s%ldV",
				BSA_SCALE_CHAR_TO_PS(zFont->tm.tmAscent));
		_emit(zDrvr, emitBuf);
		sprintf(emitBuf, "\033(s%dS", zFont->tm.tmItalic ? 1 : 0);
		_emit(zDrvr, emitBuf);
		sprintf(emitBuf, "\033(s%dB", zFont->tm.tmWeight > 500 ? 3 : 0);
		_emit(zDrvr, emitBuf);
		//sprintf(emitBuf, "\033(s%dT", family);
		//_emit(zDrvr, emitBuf);
		return TRUE;
	}
	return FALSE;
}

//**************************************************************************
BOOL _pcl_setPen(LPZDRVR zDrvr, HPEN hPen)
{
//	LPZGDIOBJ zObj;
//	LPZPEN	  zPen;

	return TRUE;
}

//**************************************************************************
BOOL _pcl_textOut(LPZDRVR zDrvr, int x, int y, LPCTSTR lpText, int nText)
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
			pAnsi[j++] = (char)lpText[i];
		}
		pAnsi[j] = 0;
	}
	else
		return FALSE;

	_emit(zDrvr, pAnsi);

	free(pAnsi);
	return TRUE;
}

//**************************************************************************
BOOL _pcl_moveTo(LPZDRVR zDrvr, int x, int y)
{
	char*		emitBuf;

	if(! zDrvr)	      			 return FALSE;
	if(! (emitBuf = zEB(zDrvr))) return FALSE;

	sprintf(emitBuf, "\033*p%dX\033*p%dY", x, y);
	_emit(zDrvr, emitBuf);
	return TRUE;
}

//**************************************************************************
BOOL _pcl_lineTo(LPZDRVR zDrvr, int x, int y)
{
	return TRUE;
}

//**************************************************************************
int	 _pcl_fillRect(LPZDRVR zDrvr, LPRECT lprc, HBRUSH hbrush)
{
	return 1;
}

//**************************************************************************
BOOL _pcl_bitblt(LPZDRVR zDrvr, int x, int y, int w, int h, HDC hdcSrc, int sx, int sy, DWORD rop)
{
	return TRUE;
}

//**************************************************************************
int _pcl_devCaps(LPZDRVR zDrvr, int cap)
{
	switch(cap)
	{
	case DRIVERVERSION: 	    // Device driver version
		return 1;
	case TECHNOLOGY:    	    // Device classification
		return DT_RASPRINTER;
	case HORZSIZE:      	    // Horizontal size in millimeters
		return (zDrvr->vpw * 254) / (zDrvr->xres * 10);
	case VERTSIZE:      	    // Vertical size in millimeters
		return (zDrvr->vph * 254) / (zDrvr->yres * 10);
	case HORZRES:       	    // Horizontal width in pixels
		return zDrvr->vpw;
	case VERTRES:       	    // Vertical height in pixels
		return zDrvr->vph;
	case BITSPIXEL:     	    // Number of bits per pixel
		return zDrvr->vpd;
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
		return 1;
	case ASPECTY:       	    // Length of the Y leg
		return 1;
	case ASPECTXY:      	    // Length of the hypotenuse
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
BOOL _pcl_startDoc(LPZDRVR zDrvr, CONST DOCINFO* pInfo)
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
	return TRUE;
}

//**************************************************************************
BOOL _pcl_endDoc(LPZDRVR zDrvr)
{
	if(zFILE(zDrvr))
		if(zFILE(zDrvr) != stdout)
			fclose(zFILE(zDrvr));
	return TRUE;
}

//**************************************************************************
BOOL _pcl_startPage(LPZDRVR zDrvr)
{
	char* emitBuffer;
	int page;

	emitBuffer 	= zEB(zDrvr);
	page		= zPAGES(zDrvr);
	zDrvr->drvrData[4] = (LPVOID)(long)++page;

	// emit unit of measure, etc commands
	//
	_emit(zDrvr, "\033&u720D"); // decipoints
	return TRUE;
}

//**************************************************************************
BOOL _pcl_endPage(LPZDRVR zDrvr)
{
	_emit(zDrvr, "\033E");
	return TRUE;
}

//**************************************************************************
void _w_deletePCLDRVR(LPZDRVR zDrvr)
{
	if(! zDrvr) return;
	if(zEB(zDrvr)) free(zEB(zDrvr));
	free(zDrvr);
}

//**************************************************************************
LPZDRVR _w_newPCLDRVR(LPZGC zGC, LPZWND zWnd)
{
	LPZDRVR zDrvr;
	int		papx, papy;
	int		papt, papb;
	int		papl, papr;

	if(! (zDrvr = (LPZDRVR)malloc(sizeof(ZDRVR))))
		return NULL;
	memset(zDrvr, 0, sizeof(ZDRVR));

	// pcl base units are 1/300 inch, but I change that
	// to decipoints (1/10 point = 1/720 inch), which
	// is actually a more native pcl unit
	//
	// paper table units are
	// in mm, so need to convert.
	//
	papx = 800 * 254 / 1000;  	// 8.0 inches
	papy = 1050 * 254 / 1000;  	// 10.5 inches
	papt = 25 * 254 / 1000;		// top margin = 0.25 inches
	papb = 25 * 254 / 1000;		// bot margin = 0.25 inches
	papl = 25 * 254 / 1000;		// left margin = 0.25 inches
	papr = 25 * 254 / 1000;		// rght margin = 0.25 inches

	zDrvr->yorigTop = 1; // pcl org is upper left

	zDrvr->xres = 720;
	zDrvr->yres = 720;
	zDrvr->xmar = papl;
	zDrvr->ymar = papb;
	zDrvr->vpw  =  papx * zDrvr->xres * 10 / 254;
	zDrvr->vph  =  papy * zDrvr->yres * 10 / 254;
	zDrvr->vpd  =  24; // color device

	zDrvr->drvrData[0] = malloc(256);
	zDrvr->drvrData[1] = zGC;
	zDrvr->drvrData[2] = zWnd;

	zDrvr->_delete 	= _w_deletePCLDRVR;
	zDrvr->_setFrg 	= _pcl_setfrg;
	zDrvr->_setBkg 	= _pcl_setbkg;
	zDrvr->_setFont	= _pcl_setFont;
	zDrvr->_setPen	= _pcl_setPen;
	zDrvr->_textOut	= _pcl_textOut;
	zDrvr->_moveTo	= _pcl_moveTo;
	zDrvr->_lineTo	= _pcl_lineTo;
	zDrvr->_fillRect= _pcl_fillRect;
	zDrvr->_bitblt	= _pcl_bitblt;
	zDrvr->_devCaps	= _pcl_devCaps;

	zDrvr->_startDoc 	= _pcl_startDoc;
	zDrvr->_endDoc 		= _pcl_endDoc;
	zDrvr->_startPage 	= _pcl_startPage;
	zDrvr->_endPage 	= _pcl_endPage;

	return zDrvr;
}

