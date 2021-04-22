
#include "bedx.h"

#define VIEW_LEFT_MARGIN 10
#define VIEW_LINO_WIDTH  60
#define VIEW_LINO_MARGIN 14

// keyword name table
//
LPCTSTR Bview::view_keyword_names[] =
{
	_T("Plain"),
	_T("Quoted"),
	_T("Comment"),
	_T("Builtin"),
	_T("Builtin-Type"),
	_T("Builtin-Func"),
	_T("Addon-Type"),
	_T("Addon-Func"),
	_T("Macro"),
	_T("Operator"),
	_T("Special"),
	_T("Selected"),
	_T("Highlight"),
	_T("LinoBackground"),
	_T("Background")
};

// table of basic default fonts
//
FONTSPEC Bview::view_init_fonts[] =
{
#ifdef X11
#ifdef UNICODE
#ifdef Linux
	{ _T("Ubuntu Mono"), 14, false, false, true },	// kwPlain,
	{ _T("Ubuntu Mono"), 14, false, false, true },	// kwQuoted,
	{ _T("Ubuntu Mono"), 14, false, false, true },	// kwComment,
	{ _T("Ubuntu Mono"), 14, false, false, true },	// kwBuiltin,
	{ _T("Ubuntu Mono"), 14, false, false, true },	// kwBuiltinType,
	{ _T("Ubuntu Mono"), 14, false, false, true },	// kwBuiltinFunc,
	{ _T("Ubuntu Mono"), 14, false, false, true },	// kwAddonType,
	{ _T("Ubuntu Mono"), 14, false, false, true },	// kwAddonFunc,
	{ _T("Ubuntu Mono"), 14, false, false, true },	// kwMacro,
	{ _T("Ubuntu Mono"), 14, false, false, true },	// kwOperator,
	{ _T("Ubuntu Mono"), 14, true,  false, true }		// kwSpecial
#else
	{ _T("bitstream vera sans mono"), 12, false, false, true },	// kwPlain,
	{ _T("bitstream vera sans mono"), 12, false, false, true },	// kwQuoted,
	{ _T("bitstream vera sans mono"), 12, false, false, true },	// kwComment,
	{ _T("bitstream vera sans mono"), 12, false, false, true },	// kwBuiltin,
	{ _T("bitstream vera sans mono"), 12, false, false, true },	// kwBuiltinType,
	{ _T("bitstream vera sans mono"), 12, false, false, true },	// kwBuiltinFunc,
	{ _T("bitstream vera sans mono"), 12, false, false, true },	// kwAddonType,
	{ _T("bitstream vera sans mono"), 12, false, false, true },	// kwAddonFunc,
	{ _T("bitstream vera sans mono"), 12, false, false, true },	// kwMacro,
	{ _T("bitstream vera sans mono"), 12, false, false, true },	// kwOperator,
	{ _T("bitstream vera sans mono"), 12, true,  false, true }		// kwSpecial
#endif
#else
	{ _T("lucidatypewriter"), 10, false, false, false },	// kwPlain,
	{ _T("lucidatypewriter"), 10, false, false, false },	// kwQuoted,
	{ _T("lucidatypewriter"), 10, false, false, false },	// kwComment,
	{ _T("lucidatypewriter"), 10, false, false, false },	// kwBuiltin,
	{ _T("lucidatypewriter"), 10, false, false, false },	// kwBuiltinType,
	{ _T("lucidatypewriter"), 10, false, false, false },	// kwBuiltinFunc,
	{ _T("lucidatypewriter"), 10, false, false, false },	// kwAddonType,
	{ _T("lucidatypewriter"), 10, false, false, false },	// kwAddonFunc,
	{ _T("lucidatypewriter"), 10, false, false, false },	// kwMacro,
	{ _T("lucidatypewriter"), 10, false, false, false },	// kwOperator,
	{ _T("lucidatypewriter"), 10, true,  false, false }		// kwSpecial
#endif
#else
	{ _T("Courier"), 10, false, false, false },	// kwPlain,
	{ _T("Courier"), 10, false, false, false },	// kwQuoted,
	{ _T("Courier"), 10, false, false, false },	// kwComment,
	{ _T("Courier"), 10, false, false, false },	// kwBuiltin,
	{ _T("Courier"), 10, false, false, false },	// kwBuiltinType,
	{ _T("Courier"), 10, false, false, false },	// kwBuiltinFunc,
	{ _T("Courier"), 10, false, false, false },	// kwAddonType,
	{ _T("Courier"), 10, false, false, false },	// kwAddonFunc,
	{ _T("Courier"), 10, false, false, false },	// kwMacro,
	{ _T("Courier"), 10, false, false, false },	// kwOperator,
	{ _T("Courier"), 10, true,  false, false }	// kwSpecial,
#endif
};

#define KWOPAQUE (255 << 24)

// table of default colors
//
COLORREF Bview::view_init_colors [] =
{
	KWOPAQUE |	RGB(	0,	0,	0),	// kwPlain
	KWOPAQUE |	RGB(  163, 31, 85),	// kwQuoted,
	KWOPAQUE |	RGB(   85,	0, 85),	// kwComment,
	KWOPAQUE |	RGB(	0,	0,191),	// kwBuiltin,
	KWOPAQUE |	RGB(	0,	0,191),	// kwBuiltinType,
	KWOPAQUE |	RGB(	0,	0,191),	// kwBuiltinFunc,
	KWOPAQUE |	RGB(	0,	0,153),	// kwAddonType,
	KWOPAQUE |	RGB(	0,	0,153),	// kwAddonFunc,
	KWOPAQUE |	RGB(	0,127,	0),	// kwMacro,
	KWOPAQUE |	RGB(	0,	0,	0),	// kwOperator,
	KWOPAQUE |	RGB(  255,127,255),	// kwSpecial
	KWOPAQUE |	RGB(250, 250, 200),	// selected bkg
	KWOPAQUE |	RGB(  127,255,255),	// highlight
	KWOPAQUE |	RGB(  245,245,245),	// linobackground
	KWOPAQUE |	RGB(  255,255,255)	// background
};

#define NUM_VIEW_FONTS  (sizeof(view_init_fonts)  / sizeof(struct tag_font_init))
#define NUM_VIEW_COLORS (sizeof(view_init_colors) / sizeof(COLORREF))

/* if only one font set per exe
HFONT		Bview::m_view_fonts[MAX_VIEW_FONTS];
*/
FONTSPEC	Bview::m_def_fonts[MAX_VIEW_FONTS];
COLORREF	Bview::m_def_colors[MAX_VIEW_COLORS];

//**************************************************************************
LRESULT CALLBACK ViewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC			hdc;
	int			mla, mlb, mca, mcb, mpx, mpy;
	static bool	shift = false;
	static bool ctrl  = false;

	Bview*		pView = (Bview*)GetWindowLong(hWnd, GWL_USERDATA);

	if(pView && pView->GetClosed())
	{
		return 0;
	}
	switch (message)
	{
	case WM_CREATE:
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)((LPCREATESTRUCT)lParam)->lpCreateParams);
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		if(pView)
		{
			//if(ps.rcPaint.right > (ps.rcPaint.left + 8))

			//_tprintf(_T("paint view %d,%d\n"), ps.rcPaint.top, ps.rcPaint.bottom);
			pView->Draw(hdc, hWnd, &ps.rcPaint, dtDraw);
		}
		EndPaint(hWnd, &ps);
		if(pView->GetLineInfoChanged())
		{
			// redraw page
			pView->SetLineInfoChanged(false);
			pView->UpdateView(1, 1, MAX_LINE_LEN, MAX_LINE_LEN);
		}
		break;

	case WM_COMMAND:

		switch(wParam)
		{
		case ID_VIEW_EVENT:
			if(pView)
				pView->Event(lParam);
			break;

		default:
			break;
		}
		break;

	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_PRIOR:	case VK_NEXT:	case VK_END:	case VK_HOME:
		case VK_LEFT:	case VK_UP:		case VK_RIGHT:	case VK_DOWN:
		case VK_SELECT:
		case VK_PRINT:
		case VK_EXECUTE:
		case VK_SNAPSHOT:
		case VK_HELP:

		case VK_F1:	 case VK_F2:  case VK_F3:  case VK_F4:	case VK_F5:  case VK_F6:
		case VK_F7:	 case VK_F8:  case VK_F9:  case VK_F10: case VK_F11: case VK_F12:
		case VK_F13: case VK_F14: case VK_F15: case VK_F16: case VK_F17: case VK_F18:
		case VK_F19: case VK_F20: case VK_F21: case VK_F22: case VK_F23: case VK_F24:
			wParam = BkeyTranslate[wParam];
			pView->Key(wParam);
			break;

		case VK_SHIFT:
			shift = true;
			break;

		case VK_CONTROL:
			ctrl = true;
			break;

		case VK_INSERT:
			if(shift)
				pView->Dispatch(Paste);
			else if(ctrl)
				pView->Dispatch(Copy);
			else
				pView->Key(BkeyTranslate[wParam]);
			break;

		case VK_DELETE:
			if(shift)
				pView->Dispatch(Cut);
			else if(ctrl)
				pView->Dispatch(DeleteBlock);
			else
				pView->Key(BkeyTranslate[wParam]);
			break;

		case VK_TAB:
			if(pView->GetMarkedRegion(mla, mca, mlb, mcb))
			{
				if(mca == 1 || mcb == 1)
				{
					if(shift)
						pView->Dispatch(IndentLess);
					else
						pView->Dispatch(IndentMore);
				}
				else
				{
					pView->Key(wParam);
				}
			}
			else
			{
				pView->Key(wParam);
			}
			break;

		default:
			break;
		}
		break;

	case WM_SYSKEYDOWN:
		switch(wParam)
		{
		case 8:
			pView->Dispatch(Undo);
			break;

		default:
			break;
		}
		break;

	case WM_SYSKEYUP:
		switch(wParam)
		{
		case VK_SHIFT:
			shift = false;
			break;

		case VK_CONTROL:
			ctrl = false;
			break;

		default:
			break;
		}
		break;

	case WM_SYSCHAR:
		break;

	case WM_KEYUP:
		switch(wParam)
		{
		case VK_SHIFT:
			shift = false;
			break;

		case VK_CONTROL:
			ctrl = false;
			break;

		default:
			break;
		}
		break;

	case WM_CHAR:
		//if(wParam == 13)
		//	wParam = 10;
		if(wParam == 9) // tab handled as VK_TAB
			break;
		pView->Key(wParam);
		break;

	case WM_LBUTTONDOWN:
		mpx = (int)(short)LOWORD(lParam);
		mpy = (int)(short)HIWORD(lParam);
		pView->MouseDown(mpx, mpy);
		break;

	case WM_LBUTTONUP:
		mpx = (int)(short)LOWORD(lParam);
		mpy = (int)(short)HIWORD(lParam);
		pView->MouseUp(mpx, mpy);
		break;

	case WM_MOUSEMOVE:
		mpx = (int)(short)LOWORD(lParam);
		mpy = (int)(short)HIWORD(lParam);
		pView->MouseMove(mpx, mpy);
		break;

	case WM_MOUSEWHEEL:
		mpy = (int)(short)HIWORD(wParam);
		mpy /= WHEEL_DELTA;
		mpy *= pView->GetWheelIncrement();
		mpx = pView->GetCurLine() - mpy;
		pView->PushParm(mpx, ptNumber);
		pView->Dispatch(MoveToLine);
		break;

	case WM_RBUTTONDOWN:
		mpx = (int)(short)LOWORD(lParam);
		mpy = (int)(short)HIWORD(lParam);
		if(wParam & MK_LBUTTON)
			return ViewWndProc(hWnd, WM_MBUTTONDOWN, wParam, lParam);
		else
			pView->MouseMenu(mpx, mpy);
		break;

	case WM_MBUTTONDOWN:
		mpx = (int)(short)LOWORD(lParam);
		mpy = (int)(short)HIWORD(lParam);
		pView->MouseSpecial(mpx, mpy);
		break;

	case WM_SETFOCUS:
		//shift = (0x80 & GetAsyncKeyState(VK_SHIFT)) != 0;
		//ctrl  = (0x80 & GetAsyncKeyState(VK_CONTROL)) != 0;
		pView->CheckExternallyModified();
		CreateCaret(hWnd, NULL, pView->GetCaretWidth(), pView->GetCaretHeight());
		hdc = GetDC(hWnd);
		pView->Draw(hdc, hWnd, NULL, dtSetCaret);
		ShowCaret(hWnd);
		ReleaseDC(hWnd, hdc);
		break;

	case WM_KILLFOCUS:
		shift = ctrl = false;
		HideCaret(hWnd);
		DestroyCaret();
		break;

	case WM_VSCROLL:

		switch(LOWORD(wParam))
		{
		case SB_BOTTOM:
			pView->ScrollView(SB_VERT, 0x7fffff00);
			break;
		case SB_ENDSCROLL:
			break;
		case SB_LINEDOWN:
			pView->ScrollView(SB_VERT, pView->GetTopLine() + 1);
			break;
		case SB_LINEUP:
			pView->ScrollView(SB_VERT, pView->GetTopLine() - 1);
			break;
		case SB_PAGEDOWN:
			pView->ScrollView(SB_VERT, pView->GetTopLine() + pView->GetViewLines());
			break;
		case SB_PAGEUP:
			pView->ScrollView(SB_VERT, pView->GetTopLine() - pView->GetViewLines());
			break;
		case SB_THUMBPOSITION:
			{
				SCROLLINFO info;

				info.cbSize = sizeof(SCROLLINFO);
				info.fMask = SIF_TRACKPOS;
				GetScrollInfo(hWnd, SB_VERT, &info);
				mpy = info.nTrackPos;
			}
			pView->ScrollView(SB_VERT, mpy);
			break;
		case SB_THUMBTRACK:
			{
				SCROLLINFO info;

				info.cbSize = sizeof(SCROLLINFO);
				info.fMask = SIF_TRACKPOS;
				GetScrollInfo(hWnd, SB_VERT, &info);
				mpy = info.nTrackPos;
			}
			pView->ScrollView(SB_VERT, mpy);
			break;
		case SB_TOP:
			pView->ScrollView(SB_VERT, 1);
			break;
		}
		break;

	case WM_HSCROLL:

		switch(LOWORD(wParam))
		{
		case SB_RIGHT:
			break;
		case SB_ENDSCROLL:
			break;
		case SB_LINERIGHT:
			pView->ScrollView(SB_HORZ, pView->GetLeftCol() + 1);
			break;
		case SB_LINELEFT:
			pView->ScrollView(SB_HORZ, pView->GetLeftCol() - 1);
			break;
		case SB_PAGERIGHT:
			pView->ScrollView(SB_HORZ, pView->GetLeftCol() + pView->GetViewCols());
			break;
		case SB_PAGELEFT:
			pView->ScrollView(SB_HORZ, pView->GetLeftCol() - pView->GetViewCols());
			break;
		case SB_THUMBPOSITION:
			mpx = HIWORD(wParam);
			pView->ScrollView(SB_HORZ, mpx);
			break;
		case SB_THUMBTRACK:
			mpx = HIWORD(wParam);
			pView->ScrollView(SB_HORZ, mpx);
			break;
		case SB_LEFT:
			pView->ScrollView(SB_HORZ, 1);
			break;
		}
		break;

	case WM_DESTROY:
		pView->SetClosed(1);
		break;

	case WM_SIZE:
		pView->ClearMetricsCache();
		break;

	case WM_NOTIFY:

		{
			LPNMHDR			pHdr = (LPNMHDR)lParam;
#if ! defined(Windows) || ! defined(__GNUC__)
			LPNMTVKEYDOWN	nmTreeKey;
#endif
			switch(pHdr->code)
			{
			case NM_DBLCLK:

				pView->SelectProtoItem();
				break;

#if ! defined(WIN32) || ! defined(__GNUC__)
			case TVN_KEYDOWN:

				nmTreeKey = (LPNMTVKEYDOWN)pHdr;
				if(nmTreeKey->wVKey == VK_RETURN)
				{
					pView->SelectProtoItem();
					return 1;
				}
				else if(nmTreeKey->wVKey == VK_ESCAPE)
				{
					pView->CheckCloseProtoWindow();
					return 1;
				}
				return 0;
#endif
			case NM_RCLICK:

				break;

			default:
				break;
			}
		}
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

bool		Bview::m_recording	= false;
bool		Bview::m_playing	= false;
bool		Bview::m_suppressmacro = false;

bool		Bview::m_runTCL		= true;
bool		Bview::m_runPerl	= true;

bool		Bview::m_overstrike	= false;
bool		Bview::m_casesensi	= false;
bool		Bview::m_trimonwrite = false;
bool		Bview::m_initabsasspaces= false;
int			Bview::m_initabspace	= 4;
int			Bview::m_iniindentspace	= 4;
bool		Bview::m_iniautotabs	= true;
int			Bview::m_binradix	= 16;
int			Bview::m_binformat	= 1;

LPMACROREC	Bview::m_macro		= NULL;
LPMACROREC	Bview::m_macroend	= NULL;
bool		Bview::m_searchrev	= false;
TCHAR		Bview::m_searchstr[MAX_SEARCH_STR];
int			Bview::m_searchlen	= 0;
TCHAR		Bview::m_replacestr[MAX_SEARCH_STR];
int			Bview::m_replacelen	= 0;

long		Bview::m_init = 0;
TCHAR 		Bview::m_tabBuf[MAX_TAB_SPACE * 4];

Bprinter*	Bview::m_printer	= NULL;

// STATIC ******************************************************************
void Bview::InitializeViews(Bpersist* pPersist, HWND hwndParent)
{
	struct tag_font_init* pdf;

	TCHAR		szParmName[64];
	LPCTSTR		lpKeywordName;
	ERRCODE		ec;
	LPFONTSPEC	pf;
	int			i;
	int			r, g, b, a;

	if(! pPersist) return;

	for(i = 0; i < NUM_VIEW_FONTS; i++)
	{
		lpKeywordName = view_keyword_names[i];

		pdf = &view_init_fonts[i];
		pf  = &m_def_fonts[i];

		_sntprintf(szParmName, 64, _T("Keyword/"_Pfs_"/Display/Font/Face"), lpKeywordName);
		ec = pPersist->GetNvStr(szParmName, pf->face, 256, pdf->face);

		_sntprintf(szParmName, 64, _T("Keyword/"_Pfs_"/Display/Font/Height"), lpKeywordName);
		ec = pPersist->GetNvInt(szParmName, pf->height, pdf->height);

		_sntprintf(szParmName, 64, _T("Keyword/"_Pfs_"/Display/Font/Bold"), lpKeywordName);
		ec = pPersist->GetNvBool(szParmName, pf->bold, pdf->bold);

		_sntprintf(szParmName, 64, _T("Keyword/"_Pfs_"/Display/Font/Italic"), lpKeywordName);
		ec = pPersist->GetNvBool(szParmName, pf->italic, pdf->italic);

		_sntprintf(szParmName, 64, _T("Keyword/"_Pfs_"/Display/Font/AntiAlias"), lpKeywordName);
		ec = pPersist->GetNvBool(szParmName, pf->antialias, pdf->antialias);
	}
	for(i = 0; i < NUM_VIEW_COLORS; i++)
	{
		lpKeywordName = view_keyword_names[i];

		r = (view_init_colors[i] >>  0) & 0xff;
		g = (view_init_colors[i] >>  8) & 0xff;
		b = (view_init_colors[i] >> 16) & 0xff;
		a = (view_init_colors[i] >> 24) & 0xff;

		_sntprintf(szParmName, 64, _T("Keyword/"_Pfs_"/Display/Color/Red"), lpKeywordName);
		ec = pPersist->GetNvInt(szParmName, r, r);
		_sntprintf(szParmName, 64, _T("Keyword/"_Pfs_"/Display/Color/Green"), lpKeywordName);
		ec = pPersist->GetNvInt(szParmName, g, g);
		_sntprintf(szParmName, 64, _T("Keyword/"_Pfs_"/Display/Color/Blue"), lpKeywordName);
		ec = pPersist->GetNvInt(szParmName, b, b);
		_sntprintf(szParmName, 64, _T("Keyword/"_Pfs_"/Display/Color/Alpha"), lpKeywordName);
		ec = pPersist->GetNvInt(szParmName, a, a);

		m_def_colors[i] = KWOPAQUE | RGB(r,g,b);
	}
}

// STATIC ******************************************************************
void Bview::DeInitializeViews()
{
	/* if static font set
	int i;

	for(i = 0; i < NUM_VIEW_FONTS; i++)
	{
		if(m_view_fonts[i])
			DeleteObject(m_view_fonts[i]);
		m_view_fonts[i] = NULL;
	}
	*/
	if(m_macro)
		KillMacro(m_macro);
	m_macro = NULL;
	if(m_printer)
		delete m_printer;
	m_printer = NULL;
	m_init = 0;
}

// STATIC ******************************************************************
ERRCODE Bview::ParseFontSpec(LPCTSTR pSpec, LPTSTR pFace, int nFace, int& height, bool& bold, bool& italic, bool& antialias)
{
	int		len;
	LPCTSTR pStart;

	if(nFace < 1 || ! pSpec)
		return errFAILURE;
	for(len = 0, pStart = pSpec; *pSpec && *pSpec != _T(':'); pSpec++)
		len++;
	if(len >= nFace)
		len = nFace - 1;
	_tcsncpy(pFace, pStart, len);
	pFace[len] = 0;
	if(*pSpec) pSpec++;
	height = _tcstol(pSpec, (LPTSTR*)&pSpec, 0);
	if(*pSpec) pSpec++;
	bold   = _tcstol(pSpec, (LPTSTR*)&pSpec, 0) != 0;
	if(*pSpec) pSpec++;
	italic = _tcstol(pSpec, (LPTSTR*)&pSpec, 0) != 0;
	if(*pSpec) pSpec++;
	antialias = _tcstol(pSpec, (LPTSTR*)&pSpec, 0) != 0;
	if(height <= 0)
		height = 14;
	return errOK;
}

//******************************************************************
LPCTSTR Bview::GetKeywordFont(BkwType type, LPTSTR pszSpec, int nSpec)
{
	LPFONTSPEC pSpec;

	if(type < kwPlain) type = kwPlain;
	if(type > kwHighlight) type = kwPlain;

	pSpec = &m_view_fspec[type];

	_sntprintf(pszSpec, nSpec, _T(""_Pfs_":%d:%d:%d:%d"),
				pSpec->face,
				pSpec->height,
				pSpec->bold,
				pSpec->italic,
				pSpec->antialias
				);
	return pszSpec;
}

//******************************************************************
ERRCODE Bview::SetKeywordFont(BkwType type, LPCTSTR pFontSpec)
{
	HFONT	hFont;
	HDC		hDC;
	TCHAR	szFace[128];
	int		height		= 16;
	int		resy		= 90;
	bool	bold		= false;
	bool	italic		= false;
	bool	antialias	= false;

	if(! pFontSpec)
		return errFAILURE;
	if(ParseFontSpec(pFontSpec, szFace, 128, height, bold, italic, antialias) != errOK)
		return errFAILURE;

	hDC = GetDC(NULL);
	if(hDC)
	{
		resy = GetDeviceCaps(hDC, LOGPIXELSY);
		DeleteDC(hDC);
	}
	hFont = CreateFont(
				height * resy / 72,0,0,0,
				(bold ? 700 : 400),
				(italic ? TRUE : FALSE),
				FALSE,FALSE,ANSI_CHARSET,
				OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
				antialias ? ANTIALIASED_QUALITY : DEFAULT_QUALITY,
				FF_SWISS,
				szFace
			);
	if(hFont)
	{
		if(m_view_fonts[type])
			DeleteObject(m_view_fonts[type]);
		_tcscpy(m_view_fspec[type].face, szFace);
		m_view_fspec[type].height	 = height;
		m_view_fspec[type].bold		 = bold;
		m_view_fspec[type].italic	 = italic;
		m_view_fspec[type].antialias = antialias;
		m_view_fonts[type] = hFont;
	}
	return errOK;
}

// STATIC ******************************************************************
ERRCODE Bview::ParseColorSpec(LPCTSTR pSpec, int& r, int& g, int& b, int& a)
{
	a = 255;
	r = g = b = 128;
	if(! pSpec)
		return errFAILURE;
	r = _tcstol(pSpec, (LPTSTR*)&pSpec, 0);
	if(*pSpec) pSpec++;
	g = _tcstol(pSpec, (LPTSTR*)&pSpec, 0);
	if(*pSpec) pSpec++;
	b = _tcstol(pSpec, (LPTSTR*)&pSpec, 0);
	if(*pSpec)
	{
		pSpec++;
		a = _tcstol(pSpec, (LPTSTR*)&pSpec, 0);
	}

	if(r < 0) r = 0;
	if(g < 0) g = 0;
	if(b < 0) b = 0;
	if(a < 0) a = 0;
	if(r > 255) r = 255;
	if(g > 255) g = 255;
	if(b > 255) b = 255;
	if(a > 255) a = 255;
	return errOK;
}

//******************************************************************
LPCTSTR Bview::GetKeywordColor(BkwType type, LPTSTR pszSpec, int nSpec)
{
	COLORREF color;
	int		 r, g, b, a;

	if(type < kwPlain) type = kwPlain;
	if(type > kwBackground) type = kwPlain;

	color = m_view_colors[type];
	r = (color >>  0) & 0xff;
	g = (color >>  8) & 0xff;
	b = (color >> 16) & 0xff;
	a = (color >> 24) & 0xff;
	_sntprintf(pszSpec, nSpec, _T("%d:%d:%d:%d"), r, g, b, a);
	return pszSpec;
}

//******************************************************************
ERRCODE Bview::SetKeywordColor(BkwType type, LPCTSTR pColorSpec)
{
	int	r = 127, g = 127, b = 127, a = 255;

	if(! pColorSpec)
		return errFAILURE;
	if(ParseColorSpec(pColorSpec, r, g, b, a) != errOK)
		return errFAILURE;
	m_view_colors[type] = (a << 24) | RGB(r, g, b);
	return errOK;
}

//**************************************************************************
Bview::Bview(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel)
		:
		BappPane(pBuf ? pBuf->GetShortName() : _T(""), pPanel),
		m_closed(0),
		m_buffer(pBuf),
		m_editor(pEditor),
		m_hWndProto(NULL),
		m_protimgs(NULL),
		m_next(NULL),
		m_tcl(NULL),
		m_hWndCacheFuncs(NULL),
		m_hWndCacheFiles(NULL),
		m_parmstack(NULL),
		m_oldparms(NULL),
		m_keywords(NULL),
		m_comments(NULL),
		m_prefixes(NULL),
		m_topline(1),
		m_leftcol(1),
		m_colscrollok(true),
		m_curline(1),
		m_curcol(1),
		m_lastmodcol(1),
		m_lastcaretcol(1),
		m_caretWidth(2),
		m_caretHeight(16),
		m_recursedraw(false),
		m_qla(0), m_qlb(0),
		m_qca(0), m_qcb(0),
		m_ola(0), m_olb(0),
		m_oca(0), m_ocb(0),
		m_rla(0), m_rlb(0),
		m_rca(0), m_rcb(0),
		m_regional(false),
		m_bmla(0), m_bmlb(0),
		m_bmca(0), m_bmcb(0),
		m_handmarked(false),
		m_tabspace(4),
		m_indentspace(4),
		m_wheelinc(3),
		m_statement_term(0),
		m_macro_prefix(0),
		m_showLineNums(true),
		m_showtabs(false),
		m_showCommentInfo(false),
		m_mouseclicks(0),
		m_lineinfoChanged(false),
		m_pmx(-1), m_pmy(-1),
		m_suppressrefresh(false),
		m_cacheCols(0),
		m_cacheLines(0),
		m_cacheFontHeight(0),
		m_cacheFontWidth(0)
{
	if(! m_init++)
	{
		InitializeViews(pEditor ? pEditor->GetPersist() : NULL, pPanel ? pPanel->GetWindow() : NULL);
		m_init = true;
	}
	m_cacheRect.top = m_cacheRect.bottom = -1;
}

//**************************************************************************
Bview::~Bview()
{
	FreeOldParms();

	m_closed = 1;
	m_buffer = NULL;
	if(m_hwnd)
	{
		DestroyWindow(m_hwnd);
		m_hwnd = NULL;
	}
	if(m_keywords)
	{
		delete m_keywords;
	}
	if(m_keyBindings)
	{
		delete m_keyBindings;
		m_keyBindings = NULL;
	}
	if(m_tcl)
		delete m_tcl;
	if(m_comments)
		BcommentList::FreeList(m_comments);
	if(m_prefixes)
		BprefixList::FreeList(m_prefixes);
	if(! --m_init)
	{
		DeInitializeViews();
	}
}

//**************************************************************************
ERRCODE Bview::PersistSettings()
{
	ERRCODE   ec = errOK;

	if(m_editor)
	{
		Bpersist* pPersist = m_editor->GetPersist();

		if(pPersist)
		{
			LPFONTSPEC	pf;
			LPCTSTR		lpKeywordName;
			TCHAR		szParmName[256];
			ERRCODE		ec;
			int			i;
			int			r, g, b, a;
			int	kb = (int)kbNative;

			if(m_keyBindings)
				kb = (int)m_keyBindings->GetBindings();

			if(m_init == 1)
			{
				// these are static, so only need one persist
				//
				pPersist->SetNvBool(_T("Text/Edit/CaseSensitive"),			m_casesensi);
				pPersist->SetNvBool(_T("Text/Edit/Overstrike"),				m_overstrike);
				pPersist->SetNvBool(_T("Text/Edit/AutoTabDetect"),			m_iniautotabs);
				pPersist->SetNvBool(_T("Text/Edit/DefaultTabsAsSpaces"),	m_initabsasspaces);
				pPersist->SetNvInt(_T("Text/Edit/DefaultTabsSpace"),		m_initabspace);
				pPersist->SetNvInt(_T("Text/Edit/DefaultIndentSpace"),		m_iniindentspace);
				pPersist->SetNvBool(_T("Text/Edit/TrimOnWrite"),			m_trimonwrite);
				pPersist->SetNvInt(_T("Text/Edit/KeyBindings"),				kb);
			}
			// form buffer-type specific name for tab/indent space defaults
			//
			_sntprintf(szParmName, 256, _T("Text/Edit/"_Pfs_"/AutoTabs"), GetTypeName());
			pPersist->SetNvBool(szParmName, m_autotabs);
			_sntprintf(szParmName, 256, _T("Text/Edit/"_Pfs_"/TabsSpace"), GetTypeName());
			pPersist->SetNvInt(szParmName, m_tabspace);
			_sntprintf(szParmName, 256, _T("Text/Edit/"_Pfs_"/IndentSpace"), GetTypeName());
			pPersist->SetNvInt(szParmName, m_indentspace);
			_sntprintf(szParmName, 256, _T("Text/Edit/"_Pfs_"/TabsAsSpaces"), GetTypeName());
			pPersist->SetNvBool(szParmName, m_autotabs ? m_initabsasspaces : m_tabsasspaces);
			_sntprintf(szParmName, 256, _T("Text/Edit/"_Pfs_"/ShowLineNumbers"), GetTypeName());
			pPersist->SetNvBool(szParmName, m_showLineNums);

			for(i = 0; i < NUM_VIEW_FONTS; i++)
			{
				lpKeywordName = view_keyword_names[i];

				pf = &m_view_fspec[i];

				_sntprintf(szParmName, 64, _T("Keyword/"_Pfs_"/"_Pfs_"/Display/Font/Face"), lpKeywordName, GetTypeName());
				ec = pPersist->SetNvStr(szParmName, pf->face);

				_sntprintf(szParmName, 64, _T("Keyword/"_Pfs_"/"_Pfs_"/Display/Font/Height"), lpKeywordName, GetTypeName());
				ec = pPersist->SetNvInt(szParmName, pf->height);

				_sntprintf(szParmName, 64, _T("Keyword/"_Pfs_"/"_Pfs_"/Display/Font/Bold"), lpKeywordName, GetTypeName());
				ec = pPersist->SetNvBool(szParmName, pf->bold);

				_sntprintf(szParmName, 64, _T("Keyword/"_Pfs_"/"_Pfs_"/Display/Font/Italic"), lpKeywordName, GetTypeName());
				ec = pPersist->SetNvBool(szParmName, pf->italic);

				_sntprintf(szParmName, 64, _T("Keyword/"_Pfs_"/"_Pfs_"/Display/Font/AntiAlias"), lpKeywordName, GetTypeName());
				ec = pPersist->SetNvBool(szParmName, pf->antialias);
			}
			for(i = 0; i < NUM_VIEW_COLORS; i++)
			{
				lpKeywordName = view_keyword_names[i];

				r = (m_view_colors[i] >>  0) & 0xff;
				g = (m_view_colors[i] >>  8) & 0xff;
				b = (m_view_colors[i] >> 16) & 0xff;
				a = (m_view_colors[i] >> 24) & 0xff;

				_sntprintf(szParmName, 64, _T("Keyword/"_Pfs_"/"_Pfs_"/Display/Color/Red"), lpKeywordName, GetTypeName());
				ec = pPersist->SetNvInt(szParmName, r);
				_sntprintf(szParmName, 64, _T("Keyword/"_Pfs_"/"_Pfs_"/Display/Color/Green"), lpKeywordName, GetTypeName());
				ec = pPersist->SetNvInt(szParmName, g);
				_sntprintf(szParmName, 64, _T("Keyword/"_Pfs_"/"_Pfs_"/Display/Color/Blue"), lpKeywordName, GetTypeName());
				ec = pPersist->SetNvInt(szParmName, b);
				_sntprintf(szParmName, 64, _T("Keyword/"_Pfs_"/"_Pfs_"/Display/Color/Alpha"), lpKeywordName, GetTypeName());
				ec = pPersist->SetNvInt(szParmName, a);
			}
			// write settings now, so they appear for other instances of app
			// without closing this instance
			//
			pPersist->Flush();
		}
	}
	return ec;
}

//**************************************************************************
ERRCODE Bview::DepersistParms()
{
	ERRCODE ec = errOK;
	int     keyBinding = kbNative;

	if(m_editor)
	{
		Bpersist* pPersist = m_editor->GetPersist();

		if(pPersist)
		{
			LPFONTSPEC	pf, pdf;
			LPCTSTR		lpKeywordName;
			TCHAR		szParmName[256];
			ERRCODE		ec;
			int			i;
			int			r, g, b, a;

			if(m_init == 1)
			{
				// these are static, so only need one persist
				//
				pPersist->GetNvBool(_T("Text/Edit/CaseSensitive"),		 m_casesensi,		false);
				pPersist->GetNvBool(_T("Text/Edit/TrimOnWrite"),		 m_trimonwrite,		false);
				//pPersist->GetNvBool(_T("Text/Edit/Overstrike"),		 m_overstrike,		false);
				pPersist->GetNvBool(_T("Text/Edit/DefaultAutoTabDetect"),m_iniautotabs,		true);
				pPersist->GetNvBool(_T("Text/Edit/DefaultTabsAsSpaces"), m_initabsasspaces, false);
				pPersist->GetNvInt(_T("Text/Edit/DefaultTabsSpace"),	 m_initabspace,		4);
				pPersist->GetNvInt(_T("Text/Edit/DefaultIndentSpace"),	 m_iniindentspace,	4);

				pPersist->GetNvBool(_T("Script/TCL/Enable TCL"),	m_runTCL, true);
				pPersist->GetNvBool(_T("Script/Perl/Enable Perl"),	m_runPerl, true);
			}
			// these are view specific
			// setup default tabs/indents/keybinds
			//
			pPersist->GetNvInt(_T("Text/Edit/KeyBindings"),		keyBinding,		kbNative);

			m_autotabs		= m_iniautotabs;
			m_tabspace		= m_initabspace;
			m_indentspace	= m_iniindentspace;
			m_tabsasspaces	= m_initabsasspaces;

			// form buffer-type specific name for tab/indent space
			//
			_sntprintf(szParmName, 256, _T("Text/Edit/"_Pfs_"/AutoTabs"), GetTypeName());
			pPersist->GetNvBool(szParmName, m_autotabs, m_autotabs);
			_sntprintf(szParmName, 256, _T("Text/Edit/"_Pfs_"/TabsSpace"), GetTypeName());
			pPersist->GetNvInt(szParmName, m_tabspace, m_tabspace);
			_sntprintf(szParmName, 256, _T("Text/Edit/"_Pfs_"/IndentSpace"), GetTypeName());
			pPersist->GetNvInt(szParmName, m_indentspace, m_indentspace);
			_sntprintf(szParmName, 256, _T("Text/Edit/"_Pfs_"/TabsAsSpaces"), GetTypeName());
			pPersist->GetNvBool(szParmName, m_tabsasspaces, m_tabsasspaces);
			_sntprintf(szParmName, 256, _T("Text/Edit/"_Pfs_"/ShowLineNumbers"), GetTypeName());
			pPersist->GetNvBool(szParmName, m_showLineNums, m_showLineNums);

			for(i = 0; i < NUM_VIEW_FONTS; i++)
			{
				lpKeywordName = view_keyword_names[i];

				pf  = &m_view_fspec[i];
				pdf = &m_def_fonts[i];

				_sntprintf(szParmName, 256, _T("Keyword/"_Pfs_"/"_Pfs_"/Display/Font/Face"), lpKeywordName, GetTypeName());
				ec = pPersist->GetNvStr(szParmName, pf->face, 63, pdf->face);

				_sntprintf(szParmName, 256, _T("Keyword/"_Pfs_"/"_Pfs_"/Display/Font/Height"), lpKeywordName, GetTypeName());
				ec = pPersist->GetNvInt(szParmName, pf->height, pdf->height);

				_sntprintf(szParmName, 256, _T("Keyword/"_Pfs_"/"_Pfs_"/Display/Font/Bold"), lpKeywordName, GetTypeName());
				ec = pPersist->GetNvBool(szParmName, pf->bold, pdf->bold);

				_sntprintf(szParmName, 256, _T("Keyword/"_Pfs_"/"_Pfs_"/Display/Font/Italic"), lpKeywordName, GetTypeName());
				ec = pPersist->GetNvBool(szParmName, pf->italic, pdf->italic);

				_sntprintf(szParmName, 256, _T("Keyword/"_Pfs_"/"_Pfs_"/Display/Font/AntiAlias"), lpKeywordName, GetTypeName());
				ec = pPersist->GetNvBool(szParmName, pf->antialias, pdf->antialias);

				m_view_fonts[i] = NULL;
				GetKeywordFont((BkwType)i, szParmName, 256);
				SetKeywordFont((BkwType)i, szParmName);
			}
			while(i < MAX_VIEW_FONTS)
			{
				m_view_fonts[i] = NULL;
				m_view_fspec[i].face[0] = _T('\0');
				i++;
			}
			for(i = 0; i < NUM_VIEW_COLORS; i++)
			{
				lpKeywordName = view_keyword_names[i];

				r = (m_def_colors[i] >>  0) & 0xff;
				g = (m_def_colors[i] >>  8) & 0xff;
				b = (m_def_colors[i] >> 16) & 0xff;
				a = (m_def_colors[i] >> 24) & 0xff;

				_sntprintf(szParmName, 256, _T("Keyword/"_Pfs_"/"_Pfs_"/Display/Color/Red"), lpKeywordName, GetTypeName());
				ec = pPersist->GetNvInt(szParmName, r, r);
				_sntprintf(szParmName, 256, _T("Keyword/"_Pfs_"/"_Pfs_"/Display/Color/Green"), lpKeywordName, GetTypeName());
				ec = pPersist->GetNvInt(szParmName, g, g);
				_sntprintf(szParmName, 256, _T("Keyword/"_Pfs_"/"_Pfs_"/Display/Color/Blue"), lpKeywordName, GetTypeName());
				ec = pPersist->GetNvInt(szParmName, b, b);
				_sntprintf(szParmName, 256, _T("Keyword/"_Pfs_"/"_Pfs_"/Display/Color/Alpha"), lpKeywordName, GetTypeName());
				ec = pPersist->GetNvInt(szParmName, a, a);

				_sntprintf(szParmName, 256, _T("%d:%d:%d:%d"), r, g, b, a);
				SetKeywordColor((BkwType)i, szParmName);
			}
			while(i < MAX_VIEW_COLORS)
			{
				m_view_colors[i] = (255 << 24) | RGB(0,0,0);
				i++;
			}
		}
	}
	m_keyBindings = new BkeyBinds((aKeyBind)keyBinding);

	if(m_runTCL)
	{
		// create tcl object to run tcl script in
		// (doesnt do much till first use)
		//
		m_tcl = new Btcl(this);
	}
	// sniff tab spacing if enabled
	//
	if(m_editor && GetBuffer() && m_autotabs)
	{
		int tabspace, indentspace;
		bool spacetabs;

		tabspace	= m_tabspace;
		indentspace = m_indentspace;

		ec = GetBuffer()->SniffTabs(tabspace, indentspace, spacetabs);

		if(ec == errOK)
		{
			m_tabspace		= tabspace;
			m_indentspace	= indentspace;
			m_tabsasspaces	= spacetabs;
		}
	}
	return ec;
}

//******************************************************************
COLORREF Bview::GetBackground(BYTE& alpha)
{
	alpha = (m_view_colors[kwBackground] >> 24) & 0xFF;
	return m_view_colors[kwBackground] & 0x00FFFFFF;
}

//**************************************************************************
void Bview::AddTags(LPCTSTR* pTags, BkwType type, bool casesensi)
{
	while(pTags && *pTags)
	{
		AddKeyword(*pTags++, type, casesensi);
	}
}

//**************************************************************************
void Bview::Activate()
{
	RECT rc;

	if(! m_parent)
		return;

	GetClientRect(m_parent->GetWindow(), &rc);

	if(! m_hwnd)
	{

		// create a view for this buffer in the application frame
		//
		m_hwnd = CreateWindow(
							_T("bedview"),
							_T("bv"),
							WS_CHILD | WS_VSCROLL | WS_HSCROLL,
							rc.left + 2,
							rc.top + 2,
							rc.right - rc.left - 2,
							rc.bottom - rc.top - 2,
							m_parent->GetWindow(),
							NULL,
							GetInstance(),
							this
							);
		SetWindowLong(m_hwnd, GWL_EXSTYLE, GetWindowLong(m_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	}
	else
	{
		MoveWindow(
							m_hwnd,
							rc.left + 2,
							rc.top + 2,
							rc.right - rc.left - 2,
							rc.bottom - rc.top - 2,
							TRUE
				);

	}
	ClearMetricsCache();
	SetFocus(true);
	BappPane::Activate();
	SetScrollers(SB_BOTH);
}

//**************************************************************************
void Bview::Deactivate()
{
	BappPane::Deactivate();
}

//**************************************************************************
BOOL Bview::CheckModified(bool cangoon)
{
	TCHAR szMessage[MAX_PATH * 2];
	int	  rc;

	if(! m_buffer) return TRUE;
	if(! m_buffer->GetModified()) return TRUE;

	_sntprintf(szMessage, MAX_PATH * 2, _T("Save Changes to "_Pfs_" ?"), m_buffer->GetName());
	rc = MessageBox(NULL, szMessage, m_buffer->GetEditor()->GetTitle(m_buffer), cangoon?MB_YESNOCANCEL:MB_YESNO);

	if(rc == IDCANCEL)
		return FALSE;
	if(rc == IDYES)
		Dispatch(SaveBuffer);
	return TRUE;
}

//**************************************************************************
BOOL Bview::CheckExternallyModified()
{
	if(m_buffer && m_buffer->IsFileNewer())
	{
		TCHAR	szMessage[MAX_PATH * 2];
		int		rc;
		int		mod;

		m_qla = m_qlb = 0;
		ReleaseCapture();

		// first reset the mod time so this isn't re-entered
		//
		m_buffer->UpdateModTime();

		mod = m_buffer->GetModified();

		_sntprintf(szMessage, MAX_PATH * 2, _T(""_Pfs_" has been modified outside the editor, Re-read file "_Pfs_"?"),
			m_buffer->GetName(), (mod ? _T(" (and loose current changes) ") : _T("")));
		rc = MessageBox(NULL, szMessage, m_buffer->GetEditor()->GetTitle(m_buffer), MB_YESNO);

		if(rc == IDYES)
		{
			m_buffer->ReadAgain();
			UpdateView(m_topline, 1, m_topline + 1000, MAX_LINE_LEN);
		}
		return TRUE;
	}
	return FALSE;
}

//**************************************************************************
BOOL Bview::Close()
{
	// take off the active view list
	//
	m_editor->RemoveView(this);

	Deactivate();
	if(GetParentPanel())
	{
		GetParentPanel()->RemovePane(this);
	}
	return TRUE;
}

//**************************************************************************
ERRCODE Bview::SetFocus(bool inout)
{
	if(inout)
	{
		if(m_editor->GetCurrentView() != this)
			m_editor->SetCurrentView(this, true);
		::SetFocus(GetWindow());
	}
	return errOK;
}

//**************************************************************************
void Bview::SetNext(Bview* pView)
{
	m_next = pView;
}

//**************************************************************************
ERRCODE Bview::Key(WORD key)
{
	EditCommand command;
	ERRCODE		ec;
	int 		keylen = 1;

	command = m_keyBindings->Translate(key);

	if(command == SelfInsert)
	{
		TCHAR keytext[2] = { key & 0xff, 0 };
		int   sl, sc;

		sl = m_curline;
		sc = m_curcol;

		PushParm(keytext, 1, ptString);
		ec = Dispatch(command);

		switch(key)
		{
		case '\r': case '\n':
			ec = Dispatch(::Indent);
			break;

		default:
			break;
		}
		switch(key)
		{
		case ')': case ']': case '}':
			PushParm(keytext, 1, ptString);
			ec = Dispatch(::Match);
			break;

		default:
			break;
		}
		return ec;
	}
	else
	{
		if(command != UnknownCommand)
			return Dispatch(command);
		return errOK;
	}
}


//**************************************************************************
ERRCODE Bview::ResetKey(WORD key)
{
	m_keyBindings->Reset(key);
	return errOK;
}

//**************************************************************************
EditCommand Bview::CommandFromName(LPCTSTR pName)
{
	return m_keyBindings->CommandFromName(pName);
}

//******************************************************************
int Bview::GetLeftMargin()
{
	if (m_showLineNums)
	{
		return VIEW_LEFT_MARGIN + VIEW_LINO_WIDTH;
	}
	return VIEW_LEFT_MARGIN;
}

//**************************************************************************
ERRCODE Bview::PrintView(int startline, int endline, BprintRange range)
{
#if 0
#ifdef Windows
	LPTSTR	pDrvr = _T("WINSPOOL");
#else
	LPTSTR	pDrvr  = _T("PostScript");
	//LPTSTR	pDrvr  = _T("PCL");
#endif
	LPTSTR	pPrt  = _T("IBM 4029");
	//LPTSTR	pPrt  = _T("Canon imageRUNNER 210 PS");
	//LPTSTR	pPrt  = _T("HP LaserJet 4L");
#endif

	DOCINFO docInfo;
	RECT	prtRect;
	HDC		hdcPrint;
	ERRCODE	ec	  = errOK;
	bool	setup = false;

	if(! m_printer)
	{
		m_printer = new Bprinter(m_editor);
	}
	if(! m_printer)
		return errFAILURE;

	// calculate rough page counts, etc.
	// based on current page size and
	// largest font in the view
	//

	ec = m_printer->SetupPrinter(range);
	if(ec != errOK) return ec;
	setup = true;

	LPCTSTR pDrvr = m_printer->GetDriver();
	LPCTSTR pPrt  = m_printer->GetPrinterName();

	hdcPrint = CreateDC(pDrvr, pPrt, NULL, NULL);

	docInfo.cbSize			= sizeof(DOCINFO);
	docInfo.lpszDocName		= m_buffer->GetShortName();
	docInfo.lpszOutput		= NULL; //_T("bs.txt");
	docInfo.lpszDatatype	= _T("RAW");
	docInfo.fwType			= DI_APPBANDING;

	if(hdcPrint)
	{
		int vpw, vph;
		int xres, yres;
		int fh, lo;
		int pagelines;
		int sl;

		StartDoc(hdcPrint, &docInfo);

		SetMapMode(hdcPrint, MM_LOENGLISH);

		// get the printer dc page extent
		vpw = GetDeviceCaps(hdcPrint, HORZRES);
		vph = GetDeviceCaps(hdcPrint, VERTRES);

		// get the resolution
		xres = GetDeviceCaps(hdcPrint, LOGPIXELSX);
		yres = GetDeviceCaps(hdcPrint, LOGPIXELSX);

		// calculate size of page in loenglish (.01 inch) units
		//
		vpw = vpw * 100 / xres;
		vph = vph * 100 / yres;

		// calculate view lines on paper, noting that fontheight
		// is always in "logical" units (which is close to points
		// since screens are close to 72 dpi, but not quite)
		//
		fh = GetFontHeight(hdcPrint);

		// scale font-height to mapped mode
		//
		HDC hdc   = GetDC(m_hwnd);
		int syres = GetDeviceCaps(hdc, LOGPIXELSY);

		fh = fh * yres / syres;
		pagelines = (vph * yres / 100) / fh;
		ReleaseDC(m_hwnd, hdc);

		// set the view rect to the page
		prtRect.top = prtRect.left = 0;
		prtRect.right  = vpw;
		prtRect.bottom = vph;

		if((lo = (fh * (endline - startline))) < vph)
			prtRect.bottom = lo;

		sl = m_topline;
		m_topline = startline;

		// print pages until nothing gets drawn
		do
		{
			StartPage(hdcPrint);
			Draw(hdcPrint, NULL, &prtRect, dtPrint);
			EndPage(hdcPrint);
			startline += pagelines;
		}
		while(startline < endline);

		m_topline = sl;

		EndDoc(hdcPrint);
		DeleteDC(hdcPrint);
		return errOK;
	}
	else
	{
		return errFAILURE;
	}
}

//**************************************************************************
void Bview::UpdateView(int line1, int col1, int line2, int col2)
{
	RECT rcl;
	RECT rcw;

	int  lineoff;
	int  linetot;
	int  ytot;
	int  minline, maxline;

	if(! m_hwnd)
		return;

	if(line1 <= line2)
	{
		minline = line1;
		maxline = line2;
	}
	else
	{
		minline = line2;
		maxline = line1;
	}
	lineoff = minline - m_topline;
	linetot = maxline - minline + 1;

	if(lineoff < 0) lineoff = 0;

	rcl.top 	= lineoff * GetFontHeight();
	rcl.left 	= 0; //min(col1, col2) - 1;
	ytot		= linetot * GetFontHeight();
	rcl.bottom  = rcl.top + ytot;
	if(rcl.bottom < rcl.top)
		rcl.bottom = 0x7fffffff; // overflowed
	rcl.right   = MAX_LINE_LEN;

#if 1
	// WIN32 seems to need to have the invalid region
	// contained inside the actual window...
	//
	GetViewClientRect(m_hwnd, &rcw);

	//_tprintf(_T("min=%d max=%d lo=%d fh=%d\n"), line1, line2,
	//		lineoff, fontheight);

	IntersectRect(&rcw, &rcw, &rcl);
	InvalidateRect(m_hwnd, &rcw, FALSE);
#else
	InvalidateRect(m_hwnd, &rcl, FALSE);
#endif
}

//**************************************************************************
void Bview::UpdateAllViews(int line1, int col1, int line2, int col2, Bbuffer* pBuf)
{
	Bview* pView;

	for(pView = m_editor->GetViews(); pView; pView = pView->GetNext(pView))
	{
		if(pView->GetBuffer() == pBuf || ! pBuf)
			pView->UpdateView(line1, col1, line2, col2);
	}
}

//**************************************************************************
void Bview::MouseDown(int x, int y)
{
	HDC hdc  = GetDC(m_hwnd);
	int ml, xl;

	if(m_editor->GetCurrentView() != this)
		m_editor->SetCurrentView(this, true);
	else
		if(GetFocus() != m_hwnd)
			::SetFocus(m_hwnd);

	int xqla = m_qla;
	int xqca = m_qca;
	int xqlb = m_qlb;
	int xqcb = m_qcb;

	m_handmarked = false;
	m_locl = m_locc = 0;
	SetCapture(m_hwnd);
	if(x < GetLeftMargin())
		m_mousex = GetLeftMargin();
	else
		m_mousex = x;
	m_mousey = y;
	Draw(hdc, m_hwnd, NULL, dtSetPosToMouse);

	if(abs(x - m_pmx) < 4 && abs(y - m_pmy) < 4)
		m_mouseclicks++;
	else if(x <= GetLeftMargin())
		m_mouseclicks = 8;
	else
		m_mouseclicks = 1;
	m_pmx = x;
	m_pmy = y;
	SetLastModifiedCol(m_lastcaretcol);

	switch(m_mouseclicks)
	{
	case 2:
		Dispatch(SelectChar);
		return;

	case 3:
		Dispatch(SelectWord);
		return;

	case 4:
		ClearMark();
		return;

	case 8:
		ml = m_curline;
		Dispatch(SelectLine);
		MoveAbs(m_qlb + 1, 1);
		if(m_curline == m_qla)
			MoveAbs(m_curline, MAX_LINE_LEN);
		m_lastmodcol = m_curcol;
		m_mouseclicks = 1;
		return;
	}
	// first mouse hit, cleanup and setup
	m_mouseclicks = 1;
	m_qla = m_curline;
	m_qca = m_curcol;
	m_qlb = m_qcb = 0;

	Draw(hdc, m_hwnd, NULL, dtSetCaret);
	ReleaseDC(m_hwnd, hdc);

	if(xqlb > 0 && xqla > 0)
	{
		ml = min(xqla, xqlb);
		if(m_bmla > 0)
			ml = min(ml, m_bmla);
		xl = max(xqla, xqlb);
		if(m_bmla > 0)
			xl = max(xl, m_bmla);
		if(m_bmla || m_bmlb)
		{
			m_bmla = 0;
			m_bmlb = 0;
		}
		UpdateView(ml, 1, xl, MAX_LINE_LEN);
	}
	else if(m_bmla > 0 || m_bmlb > 0)
	{
		if(m_bmla > 0)
			ml = xl = m_bmla;
		if(m_bmlb > 0)
		{
			ml = min(m_bmlb, m_bmla);
			xl = max(m_bmlb, m_bmla);
		}
		m_bmla = m_bmlb = 0;
		UpdateView(ml, 1, xl, MAX_LINE_LEN);
	}
}

//**************************************************************************
void Bview::MouseUp(int x, int y)
{
	ReleaseCapture();
}

//**************************************************************************
void Bview::MouseMove(int x, int y)
{
	if(GetCapture() == m_hwnd)
	{
		HDC hdc  = GetDC(m_hwnd);

		int xqlb = m_qlb;
		int xqcb = m_qcb;

		if(x < GetLeftMargin()) x = GetLeftMargin();

		m_mousex = x;
		m_mousey = y;

		Draw(hdc, m_hwnd, NULL, dtSetPosToMouse);

		if(m_curline == m_qlb && m_curcol == m_qcb)
			return;

		m_qlb = m_curline;
		m_qcb = m_curcol;
		//_tprintf(_T("a=%d,%d b=%d,%d\n"), m_qla,m_qca,m_qlb,m_qcb);
		if(m_curline == m_qla && m_curcol == m_qca)
			m_qlb = m_qcb = 0;
		ReleaseDC(m_hwnd, hdc);
		UpdateView(min(m_qlb, xqlb), 1, max(m_qlb, xqlb), 0x7fffffff);
	}
}


//**************************************************************************
void Bview::MouseMenu(int x, int y)
{
	HMENU	hMenu;
	RECT	rectw;
	int		rc;

	hMenu = CreatePopupMenu();

	AppendMenu(hMenu, 0, ID_EDIT_COPY, _T("&Copy"));
	AppendMenu(hMenu, 0, ID_EDIT_PASTE,_T("&Paste"));
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

	SpecificMouseMenu(hMenu);

	AppendMenu(hMenu, 0, ID_BUILD,		_T("&Build"));
	AppendMenu(hMenu, 0, ID_FILE_SAVE,	_T("&Save"));
	AppendMenu(hMenu, 0, ID_FILE_CLOSE, _T("C&lose"));

	GetWindowRect(m_hwnd, &rectw);

	rc = TrackPopupMenu(
						hMenu,
						TPM_RETURNCMD,
						x + rectw.left,
						y + rectw.top,
						0,
						m_hwnd,
						NULL
					   );

	DestroyMenu(hMenu);
	if(rc && m_editor)
	{
		SendMessage(m_editor->GetAppWindow(), WM_COMMAND, rc, rc);
	}
}

//**************************************************************************
void Bview::MouseSpecial(int x, int y)
{
}

//**************************************************************************
void Bview::ScrollView(int which, int where)
{
	int offset;

	if(where < 1) where = 1;

	if(which == SB_HORZ)
	{
		HDC hdc;

		offset = where - m_leftcol;
		m_leftcol = where;


		UpdateView(m_topline, 1, MAX_LINE_LEN, MAX_LINE_LEN);
		hdc = GetDC(m_hwnd);
		m_recursedraw = true; // suppresses recenter
		Draw(hdc, m_hwnd, NULL, dtSetCaret);
		m_recursedraw = false;
		ReleaseDC(m_hwnd, hdc);
		SetScrollers(SB_HORZ);
		m_buffer->GetEditor()->UpdateStatusLino();
	}
	else
	{
		offset = m_curline - m_topline;
		if(offset < 1) offset = 1;
		m_topline = where;
		MoveAbs(where + offset, m_curcol);
		UpdateView(m_topline, 1, MAX_LINE_LEN, MAX_LINE_LEN);
		SetScrollers(SB_VERT);
	}
}

//**************************************************************************
void Bview::SetScrollers(int which)
{
	SCROLLINFO sinfo;
	int lines;
	int maxlinelen;
	int pagelines;

	sinfo.cbSize	= sizeof(SCROLLINFO);
	sinfo.fMask		= SIF_PAGE | SIF_POS | SIF_RANGE;
	sinfo.nMin		= 1;

	lines = m_buffer->GetLineCount(maxlinelen);

	if(which == SB_VERT || which == SB_BOTH)
	{
		pagelines		= GetViewLines();

		sinfo.nMax		= lines;
		sinfo.nPage		= pagelines;
		sinfo.nPos		= m_topline;

		SetScrollInfo(m_hwnd, SB_VERT, &sinfo, TRUE);
	}
	if(which == SB_HORZ || which == SB_BOTH)
	{
		int	pagecols	= GetViewCols();

		if(m_buffer->GetRaw())
		{
			pagecols -= 8;
			switch(m_binradix)
			{
			case 2:
				maxlinelen *= 9;
				break;
			case 8:
			case 10:
				maxlinelen *= 4;
				break;
			case 16:
				maxlinelen *= 3;
				break;
			}
			maxlinelen += 32;
			if(pagecols < 1)
				pagecols = 1;
		}
		sinfo.nMax		= maxlinelen;
		sinfo.nPage		= pagecols;
		sinfo.nPos		= m_leftcol;

		SetScrollInfo(m_hwnd, SB_HORZ, &sinfo, TRUE);
	}
}

//**************************************************************************
void Bview::UpdatePos(int line, int col)
{
	MoveAbs(line, col);
}


//**************************************************************************
ERRCODE Bview::MoveAbs(int line, int col)
{
	ERRCODE ec;
	HDC     hdc;
	LPCTSTR lpText;
	int     nText;
	int		lines, cols;


	lines = GetViewLines();
	cols  = GetViewCols();

	if(col < 1)  col = 1;

	if(m_buffer)
	{
		if(line < 1) line = 1;

		ec = m_buffer->GetLineText(line, lpText, nText);
		if(ec == errOK)
		{
			if(col > nText)
				col = nText;
		}
		else
		{
			line = m_buffer->GetCurLine();

			ec = m_buffer->GetLineText(line, lpText, nText);
			if(ec == errOK)
			{
				if(col > nText)
					col = nText;
			}
		}
	}
	m_curline = line;
	m_curcol  = col;

	// update marked region if hand marked
	if(m_handmarked && m_qla > 0 && m_qca > 0)
	{
		m_qlb = m_curline;
		m_qcb = m_curcol;
	}
	hdc = GetDC(m_hwnd);
	if(! m_suppressrefresh)
		Draw(hdc, m_hwnd, NULL, dtSetCaret);
	ReleaseDC(m_hwnd, hdc);
	m_buffer->GetEditor()->UpdateStatusLino();
	return errOK;
}

//**************************************************************************
ERRCODE Bview::MoveRel(int lines, int cols)
{
	return MoveAbs(m_curline + lines, m_curcol + cols);
}

//**************************************************************************
int Bview::GetFontHeight(HDC hdc)
{
	TEXTMETRIC	tm;
	HFONT		hOldFont;
	HFONT		hFont;
	bool		gotdc;

	if(m_cacheFontHeight > 0) return m_cacheFontHeight;

	if(! hdc)
	{
		hdc = GetDC(m_hwnd);
		gotdc = true;
	}
	else
		gotdc = false;

	hFont	 = m_view_fonts[0];
	hOldFont = (HFONT)SelectObject(hdc, hFont);

	GetTextMetrics(hdc, &tm);

	if(tm.tmHeight == 0) tm.tmHeight = 13;
	SelectObject(hdc, hOldFont);
	if(gotdc)
		ReleaseDC(m_hwnd, hdc);
	m_cacheFontHeight = tm.tmHeight;
	return m_cacheFontHeight;
}

//**************************************************************************
void Bview::GetViewClientRect(HWND hWnd, LPRECT prc)
{
	if(! prc) return;
	if(m_cacheRect.top < 0 || m_cacheRect.bottom < m_cacheRect.top)
		GetClientRect(hWnd, &m_cacheRect);
	*prc = m_cacheRect;
}

//**************************************************************************
int Bview::GetViewLines(HDC hdc)
{
	RECT		rcc;
	int			fh;

	if(m_cacheLines > 0) return m_cacheLines;

	GetViewClientRect(m_hwnd, &rcc);
	fh = GetFontHeight(hdc);
	if(fh < 1) fh = 1;
	m_cacheLines = (rcc.bottom - rcc.top) / fh;
	return m_cacheLines;
}

//**************************************************************************
int Bview::GetFontWidth(HDC hdc)
{
	SIZE		sizeText;
	HFONT		hOldFont;
	HFONT		hFont;
	bool		gotdc;

	if(m_cacheFontWidth > 0) return m_cacheFontWidth;

	if(! hdc)
	{
		hdc = GetDC(m_hwnd);
		gotdc = true;
	}
	else
		gotdc = false;

	hFont	 = m_view_fonts[0];
	hOldFont = (HFONT)SelectObject(hdc, hFont);

	GetTextExtentPoint32(hdc, _T("abcmABCM12"), 10, &sizeText);
	if(sizeText.cx == 0) sizeText.cx = 80;

	SelectObject(hdc, hOldFont);
	if(gotdc)
		ReleaseDC(m_hwnd, hdc);
	m_cacheFontWidth = sizeText.cx / 10;
	return m_cacheFontWidth;
}

//**************************************************************************
int Bview::GetViewCols(HDC hdc)
{
	RECT		rcc;
	int			fw;

	if(m_cacheCols > 0) return m_cacheCols;

	GetViewClientRect(m_hwnd, &rcc);
	fw = GetFontWidth(hdc);
	if(fw < 1) fw = 1;
	rcc.left += GetLeftMargin();
	m_cacheCols = (rcc.right - rcc.left) / fw;
	return m_cacheCols;
}

//**************************************************************************
void Bview::ClearMetricsCache()
{
	m_cacheLines 	  = 0;
	m_cacheCols 	  = 0;
	m_cacheFontWidth  = 0;
	m_cacheFontHeight = 0;
	m_cacheRect.top = m_cacheRect.bottom = -1;
}

//**************************************************************************
bool Bview::GetMarkedRegion(int& mla, int& mca, int& mlb, int& mcb)
{
	if(m_qla > 0 && m_qlb > 0 && (m_qla != m_qlb || m_qca != m_qcb))
	{
		mla = m_qla;
		mca = m_qca;
		mlb = m_qlb;
		mcb = m_qcb;
		return true;
	}
	else
	{
		mla = mca = 0;
		mlb = mcb = 0;
		return false;
	}
}

//**************************************************************************
void Bview::SetMarkedRegion	(int mla, int mca, int mlb, int mcb)
{
	int ml, xl;

	if(m_qla <= m_qlb)
	{
		ml = m_qla;
		xl = m_qlb;
	}
	else
	{
		ml = m_qlb;
		xl = m_qla;
	}

	if(mla < mlb)
	{
		m_qla = mla;
		m_qca = mca;
		m_qlb = mlb;
		m_qcb = mcb;
	}
	else if(mla > mlb)
	{
		m_qla = mlb;
		m_qca = mcb;
		m_qlb = mla;
		m_qcb = mca;
	}
	else
	{
		m_qla = m_qlb = mla;
		if(mca <= mcb)
		{
			m_qca = mca;
			m_qcb = mcb;
		}
		else
		{
			m_qca = mcb;
			m_qcb = mca;
		}
	}
	UpdateView(min(ml, mla), 1, max(xl, mlb), 0x7fff);
}

//**************************************************************************
void Bview::ClearMark(void)
{
	SetMarkedRegion(0, 0, 0, 0);
}

//**************************************************************************
void Bview::RecenterView()
{
	HDC hdc;
	int lines;
	int curline;
	int curcol;
	int cols;

	lines = GetViewLines();
	cols  = GetViewCols();

	curline = m_curline;
	curcol  = m_curcol;

	m_topline = curline - lines / 2;

	hdc = GetDC(m_hwnd);

	if(m_topline <= 0)
		m_topline = 1;
	if(m_leftcol <= 0)
		m_leftcol = 1;

	SetScrollers(SB_BOTH);

	Draw(hdc, m_hwnd, NULL, dtSetCaret);
	ReleaseDC(m_hwnd, hdc);
	InvalidateRect(m_hwnd, NULL, FALSE);
}

//**************************************************************************
int Bview::DisplayColumn(HDC ihDC, int line, int col, bool inout, int& x, int& y)
{
	TokenRet	tr;
	ERRCODE		ec;
	LPCTSTR 	lpText;
	HDC			hdc;
	HFONT		hCurFont;
	HFONT		hOldFont;
	HFONT		hFont;
	int			nText;
	LPTSTR		lpToken;
	int			nToken;
	int			incol, outcol, totcol, tokcol, basecol;
	int			yi, cwidth;
	TokenState	state;
	BlineInfo	info;
	SIZE		tokenSize;
	int			dx;
	int			nIn, nParsed;
	COLORREF	frgColor, bkgColor;

	//	1) given line,col, what is the X and Y position in pixels of that position
	//	  a) interpreting col as the logical cusor position in the buffer (inout is 1 (out))
	//	  b) or interpr. col as the displayed column position in the window (inout is 0 (in))

	x = y = 0;

	ec = m_buffer->GetLineText(line, lpText, nText);
	if(ec != errOK) return 0;

	if(! ihDC)
		hdc	= GetDC(m_hwnd);
	else
		hdc = ihDC;

	yi = GetFontHeight(ihDC);

	//_tprintf(_T("DispC for %d,%d\n"), line, col);

	hCurFont = m_view_fonts[0];
	hOldFont = (HFONT)SelectObject(hdc, hCurFont);

	incol	= 1;
	outcol	= 1;
	totcol	= 1;
	nParsed = 0;
	basecol = 1;

	info = m_buffer->GetLineCommentInfo(line);

	if(info == liInSpanning || info == liEndsSpanning)
		state = tsSpanningComment;
	else
		state = tsBase;

	dx =  GetLeftMargin();
	nIn = 0;

	do
	{
		tokcol       = incol ? incol : 1;	// start buffer column index
		basecol		 = outcol;				// start logical buffer position

		tokenSize.cx = 0;
		tokenSize.cy = 0;
		nToken       = 0;

		tr = GetToken(
						(LPTSTR&)lpText,
						nText,
						line,
						incol,
						outcol,
						state,
						lpToken,
						nToken,
						hFont,
						frgColor,
						bkgColor
					  );
		if(tr == trOK)
		{
			int pwidth;

			if(!FORCE_LINE_MATCH && line == m_bmlb)
			{
				if(tokcol == m_bmcb)
				{
					hFont = m_view_fonts[kwSpecial];
				}
			}
			for(nIn = nParsed = pwidth = 0; nParsed < nToken; nParsed++)
			{
				if(hFont != hCurFont)
				{
					SelectObject(hdc, hFont);
					hCurFont = hFont;
				}
				if(inout)
				{
					if(
						(incol > 0 && outcol > incol && tokcol == m_curcol)
					||	(outcol <= incol && (basecol + nIn) == col)
					)
					{
						x = dx + pwidth;
						tr = trEOLTOK;
						break; // no need to go on
					}
				}
				else
				{
					if(totcol >= col)
					{
						x = dx + pwidth;
						tr = trEOLTOK;
						break; // no need to go on
					}
				}
				if(lpToken[0] == '\t')
				{
					int tw;

					GetTextExtentPoint32(hdc, _T(" "), 1, &tokenSize);

					if(tokenSize.cx <= 0) tokenSize.cx = 1;

					tw = m_tabspace - ((totcol - 1) % m_tabspace);
					for(cwidth = 0; tw-- > 0;)
					{
						if((basecol + nParsed) >= m_leftcol)
						{
							cwidth+= tokenSize.cx;
						}
						nParsed++;
						totcol++;
					}
				}
				else
				{
					GetTextExtentPoint32(hdc, lpToken + nParsed, 1, &tokenSize);
					cwidth = tokenSize.cx;

					if(tokenSize.cy > yi)
						yi = tokenSize.cy;
					totcol++;
				}
				if((basecol + nParsed) > m_leftcol)
				{
					pwidth += cwidth;
				}
				nIn++;
			}
			dx += pwidth;
		}
	}
	while(tr == trOK);

	if(x == 0)
		x = dx;
	y = (line - m_topline) * yi;

	SelectObject(hdc, hOldFont);
	if(! ihDC)
		ReleaseDC(m_hwnd, hdc);
	/*
	if(inout)
		_tprintf(_T("DC IN for %d,%d = %d\n"), line, col, basecol + nParsed);
	else
		_tprintf(_T("DC OU Tfor %d,%d = %d\n"), line, col, tokcol + nIn - 1);
	*/
	if(!inout)
		return tokcol + nIn;
	else
		return basecol + nParsed;
}

//**************************************************************************
static void DrawTypeMark(HDC hdc, int x, int y, int w, COLORREF fill)
{
	HPEN hpen, hop;
	int  h = 10;

	hop = (HPEN)SelectObject(hdc, GetStockObject(BLACK_PEN));

	// outline
	MoveToEx(hdc, 0, y + 2, NULL);
	LineTo(hdc, w - 6, y + 2);
	LineTo(hdc, w - 2, y + 1 + h / 2);
	LineTo(hdc, w - 6, y + h);
	LineTo(hdc, 0, y + h);
	LineTo(hdc, 4, y + 1 + h / 2);
	LineTo(hdc, 0, y + 2);

	hpen = CreatePen(PS_SOLID, 0, fill);
	SelectObject(hdc, hpen);

	// fill
	MoveToEx(hdc, 2, y + 3, NULL);
	LineTo(hdc, w - 5, y + 3);
	MoveToEx(hdc, 3, y + 4, NULL);
	LineTo(hdc, w - 4, y + 4);
	MoveToEx(hdc, 4, y + 5, NULL);
	LineTo(hdc, w - 3, y + 5);
	MoveToEx(hdc, 5, y + 6, NULL);
	LineTo(hdc, w - 2, y + 6);
	MoveToEx(hdc, 2, y + 9, NULL);
	LineTo(hdc, w - 5, y + 9);
	MoveToEx(hdc, 3, y + 8, NULL);
	LineTo(hdc, w - 4, y + 8);
	MoveToEx(hdc, 4, y + 7, NULL);
	LineTo(hdc, w - 3, y + 7);

	SelectObject(hdc, hop);
}

static TCHAR g_tabspace[32] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                                ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                                ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                                ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '
							  };
static TCHAR g_tabxspace[32] = { '^', '^', '^', '^', '^', '^', '^', '^',
                                '^', '^', '^', '^', '^', '^', '^', '^',
                                '^', '^', '^', '^', '^', '^', '^', '^',
                                '^', '^', '^', '^', '^', '^', '^', '^'
							  };

//#define DBG_SETCURS 1

//**************************************************************************
void Bview::Draw(HDC hdc, HWND hWnd, LPRECT lprcUpdate, DrawType type)
{
	RECT		rcc;
	RECT		rcl;
	HFONT		hOldFont;
	LPCTSTR		lpText;
	LPCTSTR		lpBase;
	LPTSTR		lpToken;
	ERRCODE		ec;
	TokenRet	tr;
	COLORREF	curFrgColor, curBkgColor;
	COLORREF	frgColor, bkgColor;
	HFONT		hCurFont;
	HFONT		hFont;
	HBRUSH		hbrBkg;
	int			nText, nToken;
	int			line, incol, outcol, totcol, startcol, tokcol;
	int			lasthiddencol;
	int			qla,  qlb,   qca,    qcb;
	int			rla,  rlb,   rca,    rcb;
	int			x, y;
	int			caretX, caretY, caretH;
	int			setline, setcol;
	bool		caretLine;
	int			yi, minyi;
	TokenState	state, initstate;
	SIZE		tokenSize;
	BlineInfo	info, oldinfo;

	if(! hdc)		return;
	if(! m_buffer)	return;

	hCurFont = hFont = m_view_fonts[0];
	hOldFont = (HFONT)SelectObject(hdc, hCurFont);

	minyi = GetFontHeight(hdc);

	if(type != dtPrint)
	{
		GetViewClientRect(hWnd, &rcc);

		if(lprcUpdate == NULL)
			lprcUpdate = &rcc;
	}
	else
	{
		HDC hdc   = GetDC(hWnd);
		int syres = GetDeviceCaps(hdc, LOGPIXELSY);
		int yres  = GetDeviceCaps(hdc, LOGPIXELSY);

		minyi = minyi * yres / syres;
		ReleaseDC(m_hwnd, hdc);

		rcc = *lprcUpdate;
	}

	if(m_qla <= m_qlb)
	{
		qla = m_qla;
		qlb = m_qlb;
		if(m_qla < m_qlb || m_qca <= m_qcb)
		{
			qca = m_qca;
			qcb = m_qcb;
		}
		else
		{
			qca = m_qcb;
			qcb = m_qca;
		}
	}
	else
	{
		qla = m_qlb;
		qlb = m_qla;
		qca = m_qcb;
		qcb = m_qca;
	}
	if(m_rla <= m_rlb)
	{
		rla = m_rla;
		rlb = m_rlb;
		if(m_rla < m_rlb || m_rca <= m_rcb)
		{
			rca = m_rca;
			rcb = m_rcb;
		}
		else
		{
			rca = m_rcb;
			rcb = m_rca;
		}
	}
	else
	{
		rla = m_rlb;
		rlb = m_rla;
		rca = m_rcb;
		rcb = m_rca;
	}
	if(type == dtDraw || type == dtPrint)
	{
		curFrgColor = frgColor = 0x00ffffff & m_view_colors[kwPlain];
		curBkgColor = bkgColor = 0x00ffffff & m_view_colors[kwBackground];

		SetTextColor(hdc, curFrgColor);
		SetBkColor(hdc, curBkgColor);

		hbrBkg = CreateSolidBrush(bkgColor);
		/*
		_tprintf(_T("rcu t=%d l=%d b=%d r=%d  tl=%d lc=%d\n"),
			lprcUpdate->top, lprcUpdate->left,
			lprcUpdate->bottom, lprcUpdate->right, m_topline, m_leftcol);
		*/
	}
	else
	{
		curFrgColor = frgColor = 0;
		curBkgColor = bkgColor = 0;
		hbrBkg = 0;
	}
	if(m_topline == 1)
	{
		info = m_buffer->GetLineCommentInfo(1);
		if(info != liUnknown)
			info = liNone;
	}
	else
		info = m_buffer->GetLineCommentInfo(m_topline - 1);

	if(info == liUnknown)
	{
		SetupCommentInfo(1);
		info = m_buffer->GetLineCommentInfo(m_topline - 1);
	}
	//_tprintf(_T("mtl=%d\n"), m_topline);
	if(info == liStartsSpanning || info == liInSpanning)
	{
		state = tsSpanningComment;
		//_tprintf(_T("line %d starts or is in spanning\n"), m_topline - 1);
	}
	else
		state = tsBase;

	caretX 	= caretY = -1;
	setline = setcol = -1;

	incol  	= 1;
	line	= m_topline;
	yi		= minyi;
	lasthiddencol = m_leftcol - 1;

	for(
			y = (type == dtPrint) ? rcc.top : rcc.top, tr = trOK;

			((type == dtPrint) ? (y <= rcc.bottom) : (y <= rcc.bottom - yi));

			y += (type == dtPrint) ? yi : yi
	)
	{
		caretLine = (line == m_curline);

		if(tr != trEOLTOK)
			ec = m_buffer->GetLineText(line, lpText, nText);
		else
			break;

		lpBase	= lpText;
		yi		= minyi;

		if(ec == errOK && lpText)
		{
			int  mca, mcb;
			int  sca, scb;

			if(type == dtDraw)
			{
				RECT rcmarg, rcint;

				rcmarg.left 	= rcc.left;
				rcmarg.top  	= y;
				rcmarg.right 	= rcc.left +  GetLeftMargin();
				rcmarg.bottom 	= rcmarg.top + yi;

				if(IntersectRect(&rcint, &rcmarg, lprcUpdate))
				{
					FillRect(hdc, &rcmarg, hbrBkg/*GetSysColorBrush(COLOR_3DLIGHT)*/);
					//_tprintf(_T("rcml=%d r=%d\n"), rcmarg.left, rcmarg.right);

					if (m_showLineNums && (type == dtDraw || type == dtPrint))
					{
						HBRUSH hbrLino;
						TCHAR lino[32];
						int linolen;

						linolen = _sntprintf(lino, 32, _T("%d"), line);
						GetTextExtentPoint32(hdc, lino, linolen, &tokenSize);

						frgColor = m_view_colors[kwBuiltinType] & 0x00ffffff;
						bkgColor = m_view_colors[kwLinoBackground] & 0x00ffffff;

						if (frgColor != curFrgColor)
							SetTextColor(hdc, frgColor);
						curFrgColor = frgColor;

						if (bkgColor != curBkgColor)
							SetBkColor(hdc, bkgColor);
						curBkgColor = bkgColor;

						rcl.top		= y;
						rcl.bottom	= y + yi;
						rcl.left	= 0;
						rcl.right	= GetLeftMargin();
	
						hbrLino = CreateSolidBrush(bkgColor);
						FillRect(hdc, &rcl, hbrLino);
						DeleteObject(hbrLino);
						TextOut(hdc, GetLeftMargin() - VIEW_LINO_MARGIN - tokenSize.cx,  y, lino, linolen);
					}
					if(m_showCommentInfo)
					{
						TCHAR cx = ' ';

						switch(m_buffer->GetLineCommentInfo(line))
						{
						case liNone: 			cx = _T(' '); break;
						case liInSpanning: 		cx = _T('|'); break;
						case liStartsSpanning: 	cx = _T('>'); break;
						case liEndsSpanning: 	cx = _T('<'); break;
						default:				cx = _T('$'); break;
						}
						SetBkColor(hdc, GetSysColor(COLOR_3DLIGHT));
						TextOut(hdc, rcc.left, (type == dtDraw ? y : -y), &cx, 1);
						SetBkColor(hdc, bkgColor);
					}
					else
					{
						// show marks
						int   li = m_buffer->GetLineIsInfo(line);

						if(li)
						{
							COLORREF cx;

							     if(li & liIsInError)		cx = m_view_colors[kwHighlight] & 0x00ffffff;
							else if(li & liIsBreakpoint)	cx = m_view_colors[kwSpecial] & 0x00ffffff;
							else if(li & liIsSearchTarget)	cx = m_view_colors[kwBuiltin] & 0x00ffffff;
							else if(li & liIsExecCurrent)	cx = m_view_colors[kwBuiltinType] & 0x00ffffff;
							else							cx = m_view_colors[kwMacro] & 0x00ffffff;
							DrawTypeMark(hdc, rcmarg.left, y, VIEW_LEFT_MARGIN, cx);
						}
					}
				}
			}
			x =  GetLeftMargin();

			// buffer/view current column is always in input space
			//
			outcol		= 1;		// column in output space
			totcol		= 1;		// total columns in output space so far

			initstate	= state;

			do
			{
				tokcol       = incol ? incol : 1;	// base of token in input space
				startcol	 = outcol;				// base of token in output space
				tokenSize.cx = 0;
				tokenSize.cy = 0;
				nToken       = 0;

				mca			 = 0;
				mcb			 = 0;
				sca			 = 0;
				scb			 = 0;

				if(m_rla > 0 && m_rlb > 0)
				{
					// "highlight" region, set extents of mark this line
					//
					if(line == rla)
						sca = rca;
					else if(line > rla && line <= rlb)
						sca = 1;
					if(line == rlb)
						scb = rcb;
					else if(line >= rla && line < rlb)
						scb = 0x7fffffff;
				}
				if(m_qla > 0 && m_qlb > 0)
				{
					// "marked" region, set extents of mark this line
					//
					if(line == qla)
						mca = qca;
					else if(line > qla && line <= qlb)
						mca = 1;
					if(line == qlb)
						mcb = qcb;
					else if(line >= qla && line < qlb)
						mcb = 0x7fffffff;
				}

				tr = GetToken(
								(LPTSTR&)lpText,
								nText,
								line,
								incol,
								outcol,
								state,
								lpToken,
								nToken,
								hFont,
								frgColor,
								bkgColor
							);
				frgColor &= 0x00ffffff;
				bkgColor &= 0x00ffffff;
				if(tr == trOK && lpText)
				{
					COLORREF tbkgColor;
					int		 nPending, nParsed, nIn;
					int		 pwidth, cwidth, prevcwidth;

					if(! FORCE_LINE_MATCH)
					{
						if(line == m_bmla && type == dtDraw)
						{
							if(startcol == m_bmca)
							{
								bkgColor = m_view_colors[kwSpecial] & 0x00ffffff;
								hFont    = m_view_fonts[kwSpecial];
							}
						}
						if(line == m_bmlb && type == dtDraw)
						{
							if(startcol == m_bmcb)
							{
								hFont = m_view_fonts[kwSpecial];
							}
						}
					}
					for(nPending = nParsed = pwidth = nIn = prevcwidth = 0; nParsed < nToken; nParsed++)
					{
						tbkgColor = bkgColor;

						if((m_qla <= 0 || m_ola > 0) && ((startcol + nParsed) >= sca) && (((startcol + nParsed) < scb) || ((outcol - 1) < scb)))
						{
							tbkgColor = m_view_colors[kwHighlight] & 0x00ffffff; //RGB(255, 120, 100);
						}
						else if(incol > 0 && outcol > 0)
						{
							if(
									(incol >= outcol)
								&&	(startcol + nParsed >= mca)
								&&  (startcol + nParsed < mcb)
							)
							{
								tbkgColor = m_view_colors[kwSelected] & 0x00ffffff;
							}
							if(
									(incol <= outcol)
								&&	((tokcol + nIn) >= mca)
								&&  ((tokcol + (nIn * (incol / outcol))) < mcb)
							)
							{
								tbkgColor = m_view_colors[kwSelected] & 0x00ffffff;
							}
						}

						if(
								frgColor != curFrgColor
							||	tbkgColor != curBkgColor
							||	hFont != hCurFont
						)
						{
							if(nPending > 0)
							{
								if((type == dtDraw || type == dtPrint) && (y + yi) >= lprcUpdate->top && y < lprcUpdate->bottom)
								{
									TextOut(hdc, x, (type == dtDraw ? y : -y), lpToken + nParsed - nPending, nPending);
								}
								x += pwidth;
								pwidth = 0;
								nPending = 0;
							}
							if(hFont != hCurFont)
							{
								SelectObject(hdc, hFont);
								hCurFont = hFont;
							}
							if((type == dtDraw || type == dtPrint) && frgColor != curFrgColor)
							{
								SetTextColor(hdc, frgColor);
							}
							curFrgColor = frgColor;
							if((type == dtDraw || type == dtPrint) && tbkgColor != curBkgColor)
							{
								SetBkColor(hdc, tbkgColor);
							}
							curBkgColor = tbkgColor;
						}
						#ifdef DBG_SETCURS
						if(type == dtSetCaret && caretLine)
						{
							_tprintf(_T("c=%c tokcol=%d incol=%d nin=%d startcol=%d outcol=%d curcol=%d ntok=%d x=%d pw=%d\n"), lpToken[nIn], tokcol, incol, nIn, startcol, outcol, m_curcol, nToken, x, pwidth);
						}
						#endif
						if(
									type == dtSetCaret 		&&
									caretLine && caretX < 0 &&
									incol > 0				&&
									(
										// when buffer is larger than view of it in bytes
										((outcol <= incol) && ((startcol + nIn) == m_curcol))
									||
										// when view is larger than buffer in bytes
										((outcol >= incol) && (tokcol == m_curcol))
									)
						)
						{
							#ifdef DBG_SETCURS
							_tprintf(_T("set tokcol=%d incol=%d nin=%d startcol=%d outcol=%d curcol=%d ntok=%d x=%d pw=%d\n"), tokcol, incol, nIn, startcol, outcol, m_curcol, nToken, x, pwidth);
							#endif
							caretX = x + pwidth;
							caretH = tokenSize.cy;
							caretY = y;
							m_lastcaretcol = totcol;
							break; // no need to go on
						}
						if(lpToken[0] == '\t')
						{
							int tw, vw;

							lpToken = m_showtabs ? g_tabxspace : g_tabspace;
							tw = m_tabspace - ((totcol - 1) % m_tabspace);
							if(tw >= sizeof(g_tabspace)/sizeof(TCHAR))
								tw = sizeof(g_tabspace)/sizeof(TCHAR) - 1;
							if(totcol >= m_leftcol)
								vw = tw;
							else
								vw = totcol + tw - m_leftcol;
							if(vw < 0)
								vw = 0;
							if(vw > 0)
							{
								GetTextExtentPoint32(hdc, lpToken + (tw - vw), vw, &tokenSize);
								cwidth = tokenSize.cx;
							}
							else
							{
								cwidth = 0;
							}
							prevcwidth = cwidth;
							totcol+= tw;
							nParsed+= vw;
							nPending+= vw;
							/*
							while(tw-- > 0)
							{
								if(totcol >= m_leftcol)
								{
									nPending++;
								}
								nParsed++;
								totcol++;
							}
							*/
						}
						else
						{
							#if 1
							GetTextExtentPoint32(hdc, lpToken, nParsed + 1, &tokenSize);
							cwidth = tokenSize.cx - prevcwidth;
							prevcwidth = tokenSize.cx;
							#else
							GetTextExtentPoint32(hdc, lpToken + nParsed, 1, &tokenSize);
							cwidth = tokenSize.cx;
							#endif
							if(totcol >= m_leftcol)
								nPending++;
							totcol++;
						}
						if(tokenSize.cy > yi)
							yi = tokenSize.cy;

						if(totcol > m_leftcol)
						{
							if(type == dtSetPosToMouse && y <= m_mousey && (y + yi) > m_mousey)
							{
								if((x + pwidth) <= m_mousex && (x + pwidth + cwidth) > m_mousex && (setcol < 0 || setline < 0))
								{
									setline = line;
									if(incol >= outcol)
										setcol = startcol + nIn;
									else
										setcol = tokcol;
									if(setcol > outcol)
										setcol = outcol - 1;
									break; // no need to go on
								}
							}
							pwidth += cwidth;
							#ifdef DBG_SETCURS
							if(type == dtSetCaret)
							_tprintf(_T(" add %d to get pw=%d\n"), cwidth, pwidth);
							#endif
						}
						else
						{
							lasthiddencol = startcol + nIn;
						}
						nIn++;
					}
					if((type == dtDraw || type == dtPrint) && nPending > 0 && (y + yi) >= lprcUpdate->top && y < lprcUpdate->bottom)
					{
						//_tprintf(_T("eot to="_Pfs_"=%d,%d\n"), lpToken + nParsed - nPending, nPending, nParsed);
						TextOut(hdc, x, (type == dtDraw ? y : -y), lpToken + nParsed - nPending, nPending);
					}
					x += pwidth;
				}
			}
			while(
					(tr == trOK )		&& lpText &&
					(type != dtSetPosToMouse || (setcol <= 0 || setline <= 0)) &&
					(type != dtSetCaret || caretX < 0)
			);

			if(type == dtDraw)
			{
				// update line flags based on tokenizing
				oldinfo = m_buffer->GetLineCommentInfo(line);
				if(initstate != tsSpanningComment && state == tsSpanningComment)
					info = liStartsSpanning;
				else if(initstate == tsSpanningComment && state != tsSpanningComment)
					info = liEndsSpanning;
				else if(state == tsSpanningComment)
					info = liInSpanning;
				else
					info = liNone;
				if(info != oldinfo && !((oldinfo == liUnknown)&&(info == liNone)))
				{
					//_tprintf(_T("changed commentinfo %d  %d->%d\n"), line, oldinfo, info);
					m_buffer->SetLineCommentInfo(line, info);
					lprcUpdate->bottom = rcc.bottom;
				}
			}
			if(type == dtSetPosToMouse && y <= m_mousey && (y + yi) > m_mousey)
			{
				if(setcol < 0)
				{
					setline = line;
					setcol  = incol;
				}
				break; // no need to go further
			}
			if(type == dtSetCaret && caretLine)
			{
				if(caretX < 0 && tr != trEOLTOK)
				{
					//_tprintf(_T("eol tc=%d mcc=%d x=%d oc=%d\n"), tokcol,  m_curcol, x, outcol);

					// got to end of line without setting caret, caret is at
					// the last positon on the line
					caretX = x;
					caretH = tokenSize.cy;
					caretY = y;
					m_lastcaretcol = totcol;
				}
				break; // no need to go past caret line
			}

			if(type == dtDraw)
			{
				if(y >= lprcUpdate->top && y < lprcUpdate->bottom)
				{
					rcl.top		= y;
					rcl.bottom	= y + yi;
					rcl.left	= x;
					rcl.right	= rcc.right;

					FillRect(hdc, &rcl, hbrBkg);
				}
			}
			if(tr != trEOLTOK)
			{
				incol = 1;
				line++;
			}
		}
		else
		{
			break;
		}
	}
	if(type == dtDraw)
	{
		if(y >= lprcUpdate->top && y < lprcUpdate->bottom)
		{
			rcl.top    = max(y, lprcUpdate->top);
			rcl.left   = rcc.left;
			rcl.right  = rcc.right;
			rcl.bottom = rcc.bottom;
			FillRect(hdc, &rcl, hbrBkg);
		}
		if(m_bmlb > 0 && m_bmcb > 0 && m_bmla > 0 && m_bmca > 0)
		{
			HPEN hPen = CreatePen(PS_SOLID, 0, m_view_colors[kwSpecial] & 0x00ffffff);
			HPEN hOld;

			int ax, ay, iay, bx, by, iby;

			DisplayColumn(hdc, m_bmla, m_bmca, true, ax, ay);
			DisplayColumn(hdc, m_bmlb, m_bmcb, true, bx, by);

			ax +=  VIEW_LEFT_MARGIN;
			bx +=  VIEW_LEFT_MARGIN;;
			iay = ay;
			if(ay < 0) ay = 0;
			iby = by;
			if(by < 0) by = 0;

			hOld = (HPEN)SelectObject(hdc, hPen);
			if(by > ay)
			{
				MoveToEx(hdc, bx, by + yi/2, NULL);
				LineTo(hdc, bx + 8, by + yi/2);
				LineTo(hdc, ax + 8, ay + yi/2);
				if(iay >= 0)
					LineTo(hdc, ax, ay + yi/2);
				else
					LineTo(hdc, ax + 8, 0);
			}
			else // by <= ay, has to be == really
			{
				// matching on same line, so subtract 1/2 a char width
				// from the positions so line shows up in middle of char box
				//
				GetTextExtentPoint32(hdc, _T("("), 1, &tokenSize);

				ax -= tokenSize.cx / 2;
				bx -= tokenSize.cx / 2;

				if(by >= y - 3*yi)
				{
					MoveToEx(hdc, bx, by, NULL);
					LineTo(hdc, bx, by);
					LineTo(hdc, ax, ay);
					LineTo(hdc, ax, ay);
				}
				else
				{
					MoveToEx(hdc, bx, by + yi, NULL);
					LineTo(hdc, bx, by + 3*yi/2);
					LineTo(hdc, ax, ay + 3*yi/2);
					LineTo(hdc, ax, ay + yi);
				}
			}
			SelectObject(hdc, hOld);
			DeleteObject(hPen);
		}
	}
	if(type == dtSetCaret)
	{
		incol = GetViewCols(hdc);
		line  = GetViewLines(hdc);

		if(! m_recursedraw)
		{
			if(caretY < 0)
			{
				// never saw the line with the caret, its off the screen
				// so, re adjust. When mouse is down, scroll slower
				//
				if(hWnd == GetCapture())
				{
					if(m_curline >= m_topline + line)
						m_topline = m_curline - line + 1;
					else
						m_topline = m_curline;
				}
				else
				{
					m_topline = m_curline - line / 2;
					if(m_topline < line / 2)
						m_topline = 1;
				}
				m_recursedraw = true;
			}
			//	printf("mlcc=%d lc=%d oc=%d ic=%d\n", m_lastcaretcol, m_leftcol, outcol, incol);
			if(
					(m_lastcaretcol < m_leftcol)				// caret off the left
				||	(m_lastcaretcol >= (m_leftcol + incol /*- (m_leftcol == 1 ? 1 : 2)*/))	// off the right
			)
			{
				if(m_colscrollok)
				{
					// check if the caret position is visible
					// and adjust upper left accordingly
					//
					if(hWnd == GetCapture())
					{
						if(m_lastcaretcol >= m_leftcol + incol - 1)
							m_leftcol = m_lastcaretcol - incol + 2;
						else
							m_leftcol = m_lastcaretcol;
					}
					else
					{
						m_leftcol = m_lastcaretcol - incol / 2;
						if(m_leftcol < incol / 2)
							m_leftcol = 1;
					}
					m_recursedraw = true;
				}
			}
			if(m_recursedraw)
			{
				SelectObject(hdc, hOldFont);
				Draw(hdc, hWnd, lprcUpdate, dtSetCaret);
				m_recursedraw = false;
				UpdateView(1, 1, MAX_LINE_LEN, MAX_LINE_LEN);
				SetScrollers(SB_BOTH);
				return;
			}
		}
		SetCaretPos(caretX, caretY);
	}
	if(type	== dtSetPosToMouse)
	{
		if(setline < 0)
		{
			if(m_mousey < 0)
				setline = m_topline - 1;
			else /*if(m_mousey > y)*/
				setline = line;
		}
		if(lasthiddencol < 1)
			lasthiddencol = 1;
		if(m_mousex <=  GetLeftMargin())
			setcol = lasthiddencol;
		if(setcol < 1)
			setcol = 1;
		if(setline < 1)
			setline = 1;
		//_tprintf(_T("mx=%d my=%d  l,c=%d,%d tl,lc=%d,%d\n"),
		//		 m_mousex, m_mousey, setline, setcol, m_topline, m_leftcol);
		MoveAbs(setline, setcol);
	}
	SelectObject(hdc, hOldFont);
	if(type == dtDraw || type == dtPrint)
	{
		DeleteObject(hbrBkg);
	}
}
