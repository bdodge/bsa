
#include "bedx.h"
#ifdef Windows
#include <commdlg.h>
#include <winspool.h>
#define IS_WINNT() (!(GetVersion() & 0x80000000UL)) 
#endif

int		Bprinter::m_startpage	= -1;
int		Bprinter::m_endpage		= 1;
int		Bprinter::m_pagecount	= 1;
int		Bprinter::m_copies		= 1;
int		Bprinter::m_papersize	= DMPAPER_LETTER;
int		Bprinter::m_resX		= 600;
int		Bprinter::m_resY		= 600;
int		Bprinter::m_resZ		= 24;
int		Bprinter::m_extX		= 600 * 85 / 10;
int		Bprinter::m_extY		= 600 * 11;

TCHAR	Bprinter::m_printerDriver[MAX_PATH];
TCHAR	Bprinter::m_printerName[MAX_PATH] = _T("");
TCHAR	Bprinter::m_printerPort[MAX_PATH];

//**************************************************************************
Bprinter::Bprinter(Bed* pEditor)
		:
		m_editor(pEditor)
{
}

//**************************************************************************
Bprinter::~Bprinter()
{
}

//**************************************************************************
ERRCODE Bprinter::SetupPage()
{
	PRINTDLG		ps;
	HGLOBAL			hMode;
	LPDEVNAMES		pName;
	LPDEVMODE		pMode;

	// allocate a dev-mode
	//
	hMode = GlobalAlloc(GMEM_MOVEABLE, sizeof(DEVMODE));
	pMode = (LPDEVMODE)GlobalLock(hMode);
	memset((LPBYTE)pMode, 0, sizeof(DEVMODE));
	_tcscpy((LPTSTR)pMode->dmDeviceName, m_printerName);

	GlobalUnlock(hMode);

	memset((LPBYTE)&ps, 0, sizeof(ps));
	ps.lStructSize	= sizeof(ps);
	ps.hwndOwner	= NULL;
	ps.hDevMode		= hMode;
	ps.hDevNames	= NULL;
	ps.Flags		= 0;

	ps.Flags = PD_PRINTSETUP;

	if(PrintDlg((LPPRINTDLG)&ps))
	{
		pMode = (LPDEVMODE)GlobalLock(ps.hDevMode);
		if((pName = (LPDEVNAMES)GlobalLock(ps.hDevNames)) != NULL)
		{
			_tcscpy(m_printerName, (LPTSTR)pName + pName->wDeviceOffset);
			_tcscpy(m_printerDriver, (LPTSTR)pName + pName->wDriverOffset);
			//_tcscpy(m_printerDriver, _T("WINSPOOL")););
			GlobalUnlock(ps.hDevNames);
		}
		m_papersize = pMode->dmPaperSize;
		m_resX	= pMode->dmPrintQuality;
		m_resY	= pMode->dmYResolution;
		m_extX  = pMode->dmPelsWidth;
		m_extY  = pMode->dmPelsHeight;
		m_resZ	= pMode->dmBitsPerPel;
		GlobalUnlock(ps.hDevMode);
		GlobalFree(ps.hDevMode);
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bprinter::SetupPrinter(BprintRange range)
{
	return SetupPage();

#if 0
	PRINTDLG		ps;
	HGLOBAL			hMode;
	LPDEVNAMES		pName;
	LPDEVMODE		pMode;

	PRINTER_INFO_5*	pinfo = NULL;
	DWORD			dwNeeded, dwReturned;
	BOOL			rv;

	if(! m_printerName[0])
	{
		// get default printer
		//
		rv = EnumPrinters(
					IS_WINNT() ?
							PRINTER_ENUM_LOCAL :		/* win nt */
							PRINTER_ENUM_DEFAULT,		/* win 95 */
					NULL,
					5,
					NULL,
					0,
					&dwNeeded, &dwReturned
					);

		if(dwNeeded > 0)
		{
			pinfo = (PRINTER_INFO_5*)new BYTE [ dwNeeded + 2 ];

			rv = EnumPrinters(
							IS_WINNT() ?
									PRINTER_ENUM_LOCAL :		/* win nt */
									PRINTER_ENUM_DEFAULT,		/* win 95 */
							NULL,
							5,
							(LPBYTE)pinfo,
							dwNeeded,
							&dwNeeded, &dwReturned
							);

			if(rv && pinfo[0].pPrinterName)
				_tcscpy(m_printerName, pinfo[0].pPrinterName);
			delete [] pinfo;
		}
		else
		{
			return errFAILURE;
		}
		if(! rv)
			return errFAILURE;
	}

	// allocate a dev-mode
	//
	hMode = GlobalAlloc(GMEM_MOVEABLE, sizeof(DEVMODE));
	pMode = (LPDEVMODE)GlobalLock(hMode);
	memset((LPBYTE)pMode, 0, sizeof(DEVMODE));
	_tcscpy(pMode->dmDeviceName, m_printerName);

	GlobalUnlock(hMode);

	memset((LPBYTE)&ps, 0, sizeof(ps));
	ps.lStructSize	= sizeof(ps);
	ps.hwndOwner	= NULL;
	ps.hDevMode		= hMode;
	ps.hDevNames	= NULL;
	ps.Flags		= 0;

	switch(range)
	{
	case prSelection:
		ps.Flags = PD_SELECTION;
		break;
	case prWindow:
		ps.Flags = 0;
		break;
	case prPageRange:
		ps.Flags = PD_PAGENUMS;
		break;
	case prDocument:
		ps.Flags = PD_ALLPAGES;
		break;
	}
	ps.nFromPage	= (m_startpage > 0) ? m_startpage : 1;
	ps.nToPage		= (m_endpage >= m_startpage) ? m_endpage : ps.nFromPage;
	ps.nMinPage		= 1;
	ps.nMaxPage		= m_pagecount;
	ps.nCopies		= m_copies;

	if(PrintDlg((LPPRINTDLG)&ps))
	{
		pMode = (LPDEVMODE)GlobalLock(ps.hDevMode);
		if((pName = (LPDEVNAMES)GlobalLock(ps.hDevNames)) != NULL)
		{
			_tcscpy(m_printerName, (LPTSTR)pName + pName->wDeviceOffset);
			_tcscpy(m_printerDriver, (LPTSTR)pName + pName->wDriverOffset);
			//_tcscpy(m_printerDriver, _T("WINSPOOL")););
			GlobalUnlock(ps.hDevNames);
		}
		m_papersize = pMode->dmPaperSize;
		m_resX	= pMode->dmPrintQuality;
		m_resY	= pMode->dmYResolution;
		m_extX  = pMode->dmPelsWidth;
		m_extY  = pMode->dmPelsHeight;
		m_resZ	= pMode->dmBitsPerPel;
		GlobalUnlock(ps.hDevMode);
		GlobalFree(ps.hDevMode);
	}
	return errOK;
#endif /*  */
}

