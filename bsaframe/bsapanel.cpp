//--------------------------------------------------------------------
//
// File: bsapane.cpp
// Desc: application framework
// Auth: Brian Dodge
//
// (C)opyright 2003 - 2005 - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------
#include "framex.h"

//***********************************************************************
BappPanel::BappPanel(LPCTSTR pName, PBAFPANEL pParent, BAF_BORDER border)
	:
	BappComponent(pParent),
	m_panes(NULL),
	m_sizer(NULL),
	m_name(NULL),
	m_edge(frFloat),
	m_border(border),
	m_primary(false),
	m_sizable(true),
	m_next(NULL),
	m_prev(NULL)
{
	m_panels[frLeft]	= NULL;
	m_panels[frTop]		= NULL;
	m_panels[frRight]	= NULL;
	m_panels[frBottom]	= NULL;
	m_fulledged[frLeft]		= false;
	m_fulledged[frTop]		= false;
	m_fulledged[frRight]	= false;
	m_fulledged[frBottom]	= false;

	SetName(pName);
}

//***********************************************************************
BappPanel::~BappPanel()
{
	PBAFPANE  pp, px, pm;
	PBAFPANEL pl, plx;

	// delete all sub panes
	//
	pm = GetPanes();
	m_panes = NULL;

	for(pp = pm; pp;)
	{
		px =  pp->GetNext();
		delete pp;
		pp = px;
	}
	m_panes = NULL;

	// delete all sub panels
	//
	for(pl = m_panels[frLeft]; pl;)
	{
		plx = pl->GetNext();
		delete pl;
		pl = plx;
	}
	for(pl = m_panels[frRight]; pl;)
	{
		plx = pl->GetNext();
		delete pl;
		pl = plx;
	}
	for(pl = m_panels[frTop]; pl;)
	{
		plx = pl->GetNext();
		delete pl;
		pl = plx;
	}
	for(pl = m_panels[frBottom]; pl;)
	{
		plx = pl->GetNext();
		delete pl;
		pl = plx;
	}
	// remove ourselves from the parent's panel list
	//
	if(GetParentPanel())
	{
		GetParentPanel()->RemovePanel(this);
	}
	if(m_sizer)
		delete m_sizer;
	m_sizer = NULL;

	// kill our Window
	//
	if(GetWindow())
	{
		DestroyWindow(GetWindow());
		m_hwnd = NULL;
	}
	if(m_name) delete [] m_name;
}

//***********************************************************************
void BappPanel::SetName(LPCTSTR pName)
{
	int len;

	if(! pName) pName = _T("temp");
	len = _tcslen(pName) + 2;
	if(m_name) delete [] m_name;
	m_name = new TCHAR [ len ];
	_tcscpy(m_name, pName);
}

//***********************************************************************
void BappPanel::SetPaneTabs()
{
	HWND		hWnd;
	HDC			hdc;
	HFONT		hOldFont;
	RECT		rc;
	PBAFPANE	pp;

	PBAFPANE	pPane;
	RECT		rctab, rclast;
	SIZE		sizeText;
	LPCTSTR		lpText;
	bool		topTabs;
	bool		botTabs;
	int			nText;
	int			tab;
	int			x, y;

	hWnd = GetWindow();

	hdc = GetDC(hWnd);
	GetClientRect(hWnd, &rc);

	if(m_sizer)
	{
		switch(GetEdge())
		{
		case frLeft:
			rc.right	-= VSIZER_WIDTH;
			break;
		case frRight:
			rc.left		+= VSIZER_WIDTH;
			break;
		case frTop:
			rc.bottom	-= HSIZER_HEIGHT;
			break;
		case frBottom:
			rc.top		+= HSIZER_HEIGHT;
			break;
		case frFloat:
			break;
		}
	}
	topTabs = GetBorder() == frTopTabs && HasMultiplePanes();
	botTabs = GetBorder() == frBottomTabs && HasMultiplePanes();

	rctab.top		= rc.top + TABPANE_TOPMARG;
	rctab.bottom	= rc.top + TABPANE_TABHEIGHT;
	rctab.left		= rc.left + TABPANE_LEFTMARG * 4;
	if (GetBorder() == frBottomTabs)
	{
		rctab.top = rc.bottom - TABPANE_TABHEIGHT - TABPANE_BORDER - TABPANE_BOTTOMMARG + 2;
		rctab.bottom = rc.bottom - TABPANE_BOTTOMMARG;
		rctab.left = rc.left + TABPANE_LEFTMARG * 4;
	}
	for(pPane = m_panes, tab = 0; pPane; pPane = pPane->GetNext(), tab++)
	{
		lpText = pPane->GetName();
		nText  = _tcslen(lpText);

		hOldFont = (HFONT)SelectObject(hdc, pPane->GetActive() ? m_hFont /*m_hBoldFont*/ : m_hFont);

		GetTextExtentPoint32(hdc, lpText, nText, &sizeText);
		rctab.right = rctab.left + 2 * TABTEXT_LEFTMARG + sizeText.cx;

		rclast = pPane->m_tabrc;
		pPane->m_tabrc = rctab;

		rctab.left = rctab.right;
		if(hOldFont) SelectObject(hdc, hOldFont);
		hOldFont = NULL;
	}
	ReleaseDC(hWnd, hdc);
}

//***********************************************************************
LRESULT BappPanel::OnMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC			hdc;
	PAINTSTRUCT ps;
	HFONT		hOldFont;
	HPEN		hPen;
	HPEN		hOldPen;
	RECT		rc;
	PBAFPANE	pp;

	PBAFPANE	pPane;
	RECT		rctab, rclast;
	SIZE		sizeText;
	LPCTSTR		lpText;
	bool		topTabs;
	bool		botTabs;
	int			nText;
	int			tab;
	int			x, y;

	switch(msg)
	{
	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rc);

		if(m_sizer)
		{
			switch(GetEdge())
			{
			case frLeft:
				rc.right	-= VSIZER_WIDTH;
				break;
			case frRight:
				rc.left		+= VSIZER_WIDTH;
				break;
			case frTop:
				rc.bottom	-= HSIZER_HEIGHT;
				break;
			case frBottom:
				rc.top		+= HSIZER_HEIGHT;
				break;
			case frFloat:
				break;
			}
		}
		topTabs = GetBorder() == frTopTabs && HasMultiplePanes();
		botTabs = GetBorder() == frBottomTabs && HasMultiplePanes();

		if(GetBorder() != frNoBorder)
		{
			RECT rcb = rc;

			rcb.left	+= TABPANE_LEFTMARG;
			rcb.right	-= TABPANE_RIGHTMARG;
			rcb.top		+= topTabs ? TABPANE_TABHEIGHT : TABPANE_TOPMARG;
			rcb.bottom	-= botTabs ? TABPANE_TABHEIGHT : TABPANE_BOTTOMMARG;
			OutlineBox(hdc, &rcb, false);
			rcb.left	+= TABPANE_BORDER;
			rcb.right	-= TABPANE_BORDER + 1;
			rcb.top		+= TABPANE_BORDER;
			rcb.bottom	-= TABPANE_BORDER + 1;
			OutlineBox(hdc, &rcb, true);
		}
		if(topTabs)
		{
			rctab.top		= rc.top + TABPANE_TOPMARG;
			rctab.bottom	= rc.top + TABPANE_TABHEIGHT;
			rctab.left		= rc.left + TABPANE_LEFTMARG * 4;

			for(pPane = m_panes, tab = 0; pPane; pPane = pPane->GetNext(), tab++)
			{
				lpText = pPane->GetName();
				nText  = _tcslen(lpText);

				hOldFont = (HFONT)SelectObject(hdc, pPane->GetActive() ? m_hFont /*m_hBoldFont*/ : m_hFont);

				GetTextExtentPoint32(hdc, lpText, nText, &sizeText);
				rctab.right = rctab.left + 2 * TABTEXT_LEFTMARG + sizeText.cx;

				rclast = pPane->m_tabrc;
				pPane->m_tabrc = rctab;

				UnionRect(&rclast, &rclast, &pPane->m_tabrc);
				rclast.left = rctab.left;
				SetBkColor(hdc, GetSysColor(COLOR_3DLIGHT));
				FillRect(hdc, &rclast, GetSysColorBrush(COLOR_3DLIGHT));

				if(! pPane->GetActive())
				{
					hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DDKSHADOW));
					hOldPen = (HPEN)SelectObject(hdc, hPen);
					MoveToEx(hdc, rctab.left, rctab.bottom + TABPANE_BORDER + 1, NULL);
					LineTo(hdc, rctab.right, rctab.bottom  + TABPANE_BORDER + 1);
					SelectObject(hdc, hOldPen);
					DeleteObject(hPen);
					hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DSHADOW));
					hOldPen = (HPEN)SelectObject(hdc, hPen);
					MoveToEx(hdc, rctab.left, rctab.bottom  + TABPANE_BORDER, NULL);
					LineTo(hdc, rctab.right, rctab.bottom  + TABPANE_BORDER);
					SelectObject(hdc, hOldPen);
					DeleteObject(hPen);
					hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DLIGHT));
					hOldPen = (HPEN)SelectObject(hdc, hPen);
					MoveToEx(hdc, rctab.right - 2, rctab.bottom + 1, NULL);
					LineTo(hdc, rctab.right, rctab.bottom + 1);
					SelectObject(hdc, hOldPen);
					DeleteObject(hPen);
				}
				if(pPane->GetActive())
				{
					hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DLIGHT));
					hOldPen = (HPEN)SelectObject(hdc, hPen);
					MoveToEx(hdc, rctab.left, rctab.bottom, NULL);
					LineTo(hdc, rctab.right, rctab.bottom);
					MoveToEx(hdc, rctab.left, rctab.bottom + TABPANE_BORDER + 1, NULL);
					LineTo(hdc, rctab.right, rctab.bottom  + TABPANE_BORDER + 1);
					MoveToEx(hdc, rctab.left, rctab.bottom  + TABPANE_BORDER, NULL);
					LineTo(hdc, rctab.right, rctab.bottom  + TABPANE_BORDER);
					SelectObject(hdc, hOldPen);
					DeleteObject(hPen);
				}
				hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DDKSHADOW));
				hOldPen = (HPEN)SelectObject(hdc, hPen);
				MoveToEx(hdc, rctab.right - 1, rctab.bottom - (pPane->GetActive() ? -1 : 1), NULL);
				LineTo(hdc, rctab.right - 1, rctab.top + 2);
				LineTo(hdc, rctab.right - 3, rctab.top);
				if(pPane->GetActive())
				{
					MoveToEx(hdc, rctab.right - 2, rctab.bottom + 3, NULL);
					LineTo(hdc, rctab.right, rctab.bottom + 3);
				}
				SelectObject(hdc, hOldPen);
				DeleteObject(hPen);

				hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DSHADOW));
				hOldPen = (HPEN)SelectObject(hdc, hPen);
				MoveToEx(hdc, rctab.right - 2, rctab.bottom - (pPane->GetActive() ? -3 : 1), NULL);
				LineTo(hdc, rctab.right - 2, rctab.top + 2);
				LineTo(hdc, rctab.right - 3, rctab.top);
				if(pPane->GetActive())
				{
					MoveToEx(hdc, rctab.right - 1, rctab.bottom + 2, NULL);
					LineTo(hdc, rctab.right, rctab.bottom + 2);
				}
				SelectObject(hdc, hOldPen);
				DeleteObject(hPen);

				hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNHIGHLIGHT));
				hOldPen = (HPEN)SelectObject(hdc, hPen);
				MoveToEx(hdc, rctab.right - 3, rctab.top, NULL);
				LineTo(hdc, rctab.left + 2, rctab.top);
				LineTo(hdc, rctab.left, rctab.top + 2);
				LineTo(hdc, rctab.left, rctab.bottom - 1);
				SelectObject(hdc, hOldPen);
				DeleteObject(hPen);

				TextOut(hdc, rctab.left + TABTEXT_LEFTMARG,
					rctab.top + TABPANE_BORDER + TABTEXT_TOPMARG - 2, lpText, nText);

				rctab.left = rctab.right;
				if(hOldFont) SelectObject(hdc, hOldFont);
				hOldFont = NULL;
			}
		}
		else if(botTabs)
		{
			rctab.top		= rc.bottom - TABPANE_TABHEIGHT - TABPANE_BORDER - TABPANE_BOTTOMMARG + 2;
			rctab.bottom	= rc.bottom - TABPANE_BOTTOMMARG;
			rctab.left		= rc.left + TABPANE_LEFTMARG * 4;

			for(pPane = m_panes, tab = 0; pPane; pPane = pPane->GetNext(), tab++)
			{
				lpText = pPane->GetName();
				nText  = _tcslen(lpText);

				hOldFont = (HFONT)SelectObject(hdc, pPane->GetActive() ? m_hFont /*m_hBoldFont*/ : m_hFont);

				GetTextExtentPoint32(hdc, lpText, nText, &sizeText);

				rctab.right = rctab.left + 2 * TABTEXT_LEFTMARG + sizeText.cx;
				rclast = pPane->m_tabrc;
				pPane->m_tabrc = rctab;

				UnionRect(&rclast, &rclast, &pPane->m_tabrc);
				rclast.left = rctab.left;
				SetBkColor(hdc, GetSysColor(COLOR_3DLIGHT));
				FillRect(hdc, &rclast, GetSysColorBrush(COLOR_3DLIGHT));

				if(! pPane->GetActive())
				{
					hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DDKSHADOW));
					hOldPen = (HPEN)SelectObject(hdc, hPen);
					MoveToEx(hdc, rctab.left, rctab.top + TABPANE_BORDER, NULL);
					LineTo(hdc, rctab.right, rctab.top + TABPANE_BORDER);
					SelectObject(hdc, hOldPen);
					DeleteObject(hPen);
					hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DSHADOW));
					hOldPen = (HPEN)SelectObject(hdc, hPen);
					MoveToEx(hdc, rctab.left, rctab.top + TABPANE_BORDER - 1, NULL);
					LineTo(hdc, rctab.right, rctab.top + TABPANE_BORDER - 1);
					SelectObject(hdc, hOldPen);
					DeleteObject(hPen);
				}
				hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DDKSHADOW));
				hOldPen = (HPEN)SelectObject(hdc, hPen);
				MoveToEx(hdc, rctab.right - 1, rctab.top + (pPane->GetActive() ? 1 : TABPANE_BORDER) + 1, NULL);
				LineTo(hdc, rctab.right - 1, rctab.bottom - 2);
				LineTo(hdc, rctab.right - 3, rctab.bottom );
				LineTo(hdc, rctab.left + 2, rctab.bottom);
				SelectObject(hdc, hOldPen);
				DeleteObject(hPen);

				hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DSHADOW));
				hOldPen = (HPEN)SelectObject(hdc, hPen);
				if(pPane->GetActive())
				{
					MoveToEx(hdc, rctab.right, rctab.top + 1, NULL);
					LineTo(hdc, rctab.right - 2, rctab.top + 1);
				}
				else
				{
					MoveToEx(hdc, rctab.right - 2, rctab.top + TABPANE_BORDER + 1, NULL);
				}
				LineTo(hdc, rctab.right - 2, rctab.bottom - 2);
				LineTo(hdc, rctab.right - 3, rctab.bottom - 2);
				LineTo(hdc, rctab.right - 3, rctab.bottom - 1);
				LineTo(hdc, rctab.left + 2, rctab.bottom - 1);
				LineTo(hdc, rctab.left, rctab.bottom - 3);
				SelectObject(hdc, hOldPen);
				DeleteObject(hPen);


				hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNHIGHLIGHT));
				hOldPen = (HPEN)SelectObject(hdc, hPen);
				LineTo(hdc, rctab.left, rctab.top + (pPane->GetActive() ? 0 : TABPANE_BORDER));
				SelectObject(hdc, hOldPen);
				DeleteObject(hPen);

				TextOut(hdc, rctab.left + TABTEXT_LEFTMARG,
					rctab.top + TABPANE_BORDER + TABTEXT_TOPMARG, lpText, nText);

				rctab.left = rctab.right;
				if(hOldFont) SelectObject(hdc, hOldFont);
				hOldFont = NULL;
			}
		}
		EndPaint		(hWnd, &ps);
		break;

	case WM_LBUTTONDOWN:

		x = (int)((short)LOWORD(lParam));
		y = (int)((short)HIWORD(lParam));

		for(pPane = m_panes; pPane; pPane = pPane->GetNext())
		{
			if(
				x >= pPane->m_tabrc.left	&&
				x <= pPane->m_tabrc.right	&&
				y >= pPane->m_tabrc.top		&&
				y <= pPane->m_tabrc.bottom
			)
			{
				ActivatePane(pPane);
			}
		}
		break;

	case WM_SIZE:

		if(m_panels[frLeft])
			OnResized(m_panels[frLeft]);
		if(m_panels[frTop])
			OnResized(m_panels[frTop]);
		if(m_panels[frRight])
			OnResized(m_panels[frRight]);
		if(m_panels[frBottom])
			OnResized(m_panels[frBottom]);

		for(pp = GetPanes(); pp; pp = pp->GetNext())
			pp->FitToPanel();

		if(m_sizer)
			m_sizer->Attach(this);

		break;

	case WM_NOTIFY:

		{
			LPNMHDR phdr = (LPNMHDR)lParam;

			// find which pane is a pain
			for(pPane = m_panes; phdr && pPane; pPane = pPane->GetNext())
			{
				if(pPane->GetWindow() == phdr->hwndFrom)
				{
					pPane->OnMessage(pPane->GetWindow(), msg, wParam, lParam);
					return 0;
				}
			}
		}
		break;

	default:
		return BappComponent::OnMessage(hWnd, msg, wParam, lParam);
	}
	return 0;
}


//***********************************************************************
ERRCODE BappPanel::OnDock(BAF_EDGE edge, int x, int y, int w, int h)
{
	m_edge = edge;

	if(m_sizer)
	{
		delete m_sizer;
		m_sizer = NULL;
	}
	if(! m_hwnd)
	{
		// first time open
		//
		m_hwnd = CreateWindow(
								_T("baf_panel"),
								m_name,
								WS_CHILD | WS_CLIPCHILDREN,
								x, y, w, h,
								m_parent->GetWindow(),
								NULL,
								GetInstance(),
								(LPVOID)this
							);
		if(m_hwnd) ShowWindow(m_hwnd, SW_SHOW);
		else       return errFAILURE;
	}
	else
	{
		MoveWindow(m_hwnd, x, y, w, h, TRUE);
	}
	if(! GetPrimary() && GetSizable())
	{
		if(edge == frLeft || edge == frRight)
		{
			m_sizer = new BvertSizer(edge);
		}
		else
		{
			m_sizer = new BhorzSizer(edge);
		}
		if(m_sizer)
		{
			m_sizer->Attach(this);
		}
	}
	return m_parent->OnResized(this);
}

//***********************************************************************
ERRCODE BappPanel::SetPrimaryPanel(PBAFPANEL pp)
{
	PBAFPANEL	pc;
	PBAFPANEL*	pList;

	// figure out which list the panel is in
	do
	{
		for(pList = &m_panels[frLeft], pc = *pList; pc; pc = pc->GetNext())
			if(pc == pp) break;
		if(pc) break;
		for(pList = &m_panels[frTop], pc = *pList; pc; pc = pc->GetNext())
			if(pc == pp) break;
		if(pc) break;
		for(pList = &m_panels[frRight], pc = *pList; pc; pc = pc->GetNext())
			if(pc == pp) break;
		if(pc) break;
		for(pList = &m_panels[frBottom], pc = *pList; pc; pc = pc->GetNext())
			if(pc == pp) break;
	}
	while(0);

	if(! pc) return errFAILURE;

	// ensure primary panel is last in food chain
	//
	if(pp->GetNext())
	{
		if(pp->GetPrev())
		{
			pp->GetPrev()->SetNext(pp->GetNext());
			pp->GetNext()->SetPrev(pp->GetPrev());
		}
		else
		{
			*pList = pp->GetNext();
			pp->GetNext()->SetPrev(NULL);
		}
		for(pc = pp; pc->GetNext();)
			pc = pc->GetNext();
		pc->SetNext(pp);
		pp->SetPrev(pc);
		pp->SetNext(NULL);
	}

	// send a resize-message to our window
	//
	PostMessage(m_hwnd, WM_SIZE, 0, 0);

	return errOK;
}

//***********************************************************************
ERRCODE BappPanel::SetSizable(bool z)
{
	m_sizable = z;

	if(m_sizable)
	{
		if(! m_sizer)
		{
			if(m_edge == frLeft || m_edge == frRight)
			{
				m_sizer = new BvertSizer(m_edge);
			}
			else
			{
				m_sizer = new BhorzSizer(m_edge);
			}
			m_sizer->Attach(this);

			OnResized(m_sizer);
		}
	}
	else
	{
		if(m_sizer)
		{
			delete m_sizer;
			m_sizer = NULL;
		}
	}
	return errOK;
}

//***********************************************************************
ERRCODE BappPanel::SetPrimary(bool primary)
{
	PBAFPANEL pl, pc;

	// find out which list we are in the parent panel
	//
	pl = GetParentPanel();
	if(! pl) return errFAILURE;

	if(primary)
	{
		// if this is a primary panel, reset primary status of any other
		// panels in group
		//
		for(pc = this; pc->GetPrev();)
			pc = pc->GetPrev();
		for(;pc; pc = pc->GetNext())
		{
			if(pc->GetPrimary() && pc != this)
			{
				pc->SetPrimary(false);
			}
		}
		m_primary = primary;

		if(m_sizer)
		{
			delete m_sizer;
			m_sizer = NULL;
		}
		// now tell parent about the new primary panel since
		// it needs to be at the end of the list and we dont
		// have acess to parent panels head of list ptr
		//
		return pl->SetPrimaryPanel(this);
	}
	else if(m_primary)
	{
		m_primary = primary;

		// was a primary panel, reset our status
		//
		if(! m_sizer)
		{
			if(m_edge == frLeft || m_edge == frRight)
			{
				m_sizer = new BvertSizer(m_edge);
			}
			else
			{
				m_sizer = new BhorzSizer(m_edge);
			}
			m_sizer->Attach(this);

			OnResized(m_sizer);
		}
	}
	return errOK;
}

//***********************************************************************
ERRCODE BappPanel::Dock(BappPanel* pPanel, BAF_EDGE edge, LPRECT pprc, bool extend, bool primary)
{
	int		 x, y, w, h;
	int		 defw, defh;
	RECT	 rc, rcp;

	if(! pPanel) return errBAD_PARAMETER;

	GetPanelClientRect(&rc);

	if(pprc)
		IntersectRect(&rc, &rc, pprc);

	defw = (rc.right - rc.left);
	defh = (rc.bottom - rc.top);
	if(! pprc)
	{
		if(primary)	defw *= 3;
		if(primary)	defh *= 3;
		defw /= 4;
		defh /= 4;
	}
	if(defw <= 8) defw = 8;
	if(defh <= 8) defh = 8;

	switch(edge)
	{
	default:
	case frLeft:
		x = rc.left;
		w = defw;
		y = rc.top;
		h = rc.bottom - rc.top;
		if(m_fulledged[frLeft])
		{
			extend |= true;
		}
		if(! extend)
		{
			if(m_panels[frTop] && m_fulledged[frTop])
			{
				GetPanelsRect(m_panels[frTop], &rcp);
				y = rcp.bottom;
				h = rc.bottom - y;
			}
			if(m_panels[frBottom] && m_fulledged[frBottom])
			{
				GetPanelsRect(m_panels[frBottom], &rcp);
				h = rcp.top - y;
			}
		}
		else
		{
			m_fulledged[frLeft]		= true;
			m_fulledged[frRight]	= true;
			m_fulledged[frTop]		= false;
			m_fulledged[frBottom]	= false;
		}
		break;
	case frRight:
		x = rc.right - defw;
		y = rc.top;
		w = defw;
		h = rc.bottom - rc.top;
		if(m_fulledged[frRight])
		{
			extend |= true;
		}
		if(! extend)
		{
			if(m_panels[frTop] && m_fulledged[frTop])
			{
				GetPanelsRect(m_panels[frTop], &rcp);
				y = rcp.bottom;
				h = rc.bottom - y;
			}
			if(m_panels[frBottom] && m_fulledged[frBottom])
			{
				GetPanelsRect(m_panels[frBottom], &rcp);
				h = rcp.top - y;
			}
		}
		else
		{
			m_fulledged[frRight]	= true;
			m_fulledged[frLeft]		= true;
			m_fulledged[frTop]		= false;
			m_fulledged[frBottom]	= false;
		}
		break;
	case frTop:
		x = rc.left;
		y = rc.top;
		w = rc.right - rc.left;
		h = defh;
		if(m_fulledged[frTop])
		{
			extend |= true;
		}
		if(! extend)
		{
			if(m_panels[frLeft] && m_fulledged[frLeft])
			{
				GetPanelsRect(m_panels[frLeft], &rcp);
				x = rcp.right;
				w = rc.right - x;
			}
			if(m_panels[frRight] && m_fulledged[frRight])
			{
				GetPanelsRect(m_panels[frRight], &rcp);
				h = rcp.left - x;
			}
		}
		else
		{
			m_fulledged[frTop]		= true;
			m_fulledged[frBottom]	= true;
			m_fulledged[frLeft]		= false;
			m_fulledged[frRight]	= false;
		}
		break;
	case frBottom:
		x = rc.left;
		y = rc.bottom - defh;
		w = rc.right - rc.left;
		h = defh;
		if(m_fulledged[frBottom])
		{
			extend |= true;
		}
		if(! extend)
		{
			if(m_panels[frLeft] && m_fulledged[frLeft])
			{
				GetPanelsRect(m_panels[frLeft], &rcp);
				x = rcp.right;
				w = rc.right - x;
			}
			if(m_panels[frRight] && m_fulledged[frRight])
			{
				GetPanelsRect(m_panels[frRight], &rcp);
				h = rcp.left - x;
			}
		}
		else
		{
			m_fulledged[frBottom]	= true;
			m_fulledged[frTop]		= true;
			m_fulledged[frLeft]		= false;
			m_fulledged[frRight]	= false;
		}
		break;
	}
	// dock the panel at the end of the list unless there is a
	// previous primary panel already in the list, in which case
	// dock it right before the current primary panel
	//
	pPanel->SetNext(NULL);
	pPanel->SetPrev(NULL);

	if(! m_panels[edge])
	{
		m_panels[edge] = pPanel;
	}
	else
	{
		PBAFPANEL px;

		for(px = m_panels[edge]; px->GetNext(); px = px->GetNext())
			if(px->GetPrimary() && ! primary)
				break;
		if(px->GetPrimary() && ! primary)
		{
			// we need to insert ourselves between last panel
			// (which is primary) and last panels prev panel
			//
			pPanel->SetNext(px);
			if(px == m_panels[edge])
			{
				m_panels[edge] = pPanel;
			}
			else
			{
				pPanel->SetPrev(px->GetPrev());
				px->GetPrev()->SetNext(pPanel);
			}
			px->SetPrev(pPanel);
		}
		else
		{
			// insert at end of list
			//
			px->SetNext(pPanel);
			pPanel->SetPrev(px);
		}
	}
	pPanel->SetParentPanel(this);
	if(primary)
		pPanel->SetPrimary(true);

	return pPanel->OnDock(edge, x, y, w, h);
}

//***********************************************************************
ERRCODE BappPanel::Undock(BappPanel* pPanel)
{
	PBAFPANEL pc;
	PBAFPANEL* pList;

	// figure out which list the panel is in
	do
	{
		for(pList = &m_panels[frLeft], pc = *pList; pc; pc = pc->GetNext())
			if(pc == pPanel) break;
		if(pc) break;
		for(pList = &m_panels[frTop], pc = *pList; pc; pc = pc->GetNext())
			if(pc == pPanel) break;
		if(pc) break;
		for(pList = &m_panels[frRight], pc = *pList; pc; pc = pc->GetNext())
			if(pc == pPanel) break;
		if(pc) break;
		for(pList = &m_panels[frBottom], pc = *pList; pc; pc = pc->GetNext())
			if(pc == pPanel) break;
	}
	while(0);

	if(! pc) return errFAILURE;

	// unlink the panel from the list
	//
	if(pPanel->GetNext())
		pPanel->GetNext()->SetPrev(pPanel->GetPrev());
	if(pPanel->GetPrev())
		pPanel->GetPrev()->SetNext(pPanel->GetNext());
	if(pPanel == *pList)
	{
		*pList = pPanel->GetNext();
	}
	pPanel->SetNext(NULL);
	pPanel->SetPrev(NULL);
	pPanel->SetParentPanel(NULL);

	// send a resize-message to our window
	//
	PostMessage(m_hwnd, WM_SIZE, 0, 0);

	return errOK;
}


//***********************************************************************
ERRCODE BappPanel::RemovePanel(BappPanel* pPanel)
{
	return Undock(pPanel);
}

//***********************************************************************
ERRCODE BappPanel::FitPanel(PBAFPANEL pm, LPRECT prc, BAF_EDGE edge)
{
	RECT    rcw, rcx;
	bool	clipped = false;

	if(! pm) return errOK;

	// Get panels window's postion in client area of parent
	//
	GetWindowPos(pm->GetWindow(), &rcw);

	switch(edge)
	{
	case frLeft:

		if(rcw.left != prc->left || (pm->GetPrimary() && (rcw.right != prc->right)))
		{
			int w;

			if(pm->GetPrimary())
				w = prc->right - prc->left;
			else
				w = rcw.right - rcw.left;

			if(w < VSIZER_WIDTH) w = VSIZER_WIDTH;
			rcw.left  = prc->left;
			rcw.right = rcw.left + w;
			clipped   = true;
		}
		if(rcw.right > prc->right)
		{
			rcw.right = prc->right;
			clipped = true;
		}
		break;

	case frRight:

		if(rcw.right != prc->right || (pm->GetPrimary() && (rcw.left != prc->left)))
		{
			int w;

			if(pm->GetPrimary())
				w = prc->right - prc->left;
			else
				w = rcw.right - rcw.left;

			if(w < VSIZER_WIDTH) w = VSIZER_WIDTH;
			rcw.right = prc->right;
			rcw.left  = rcw.right - w;
			clipped   = true;
		}
		if(rcw.left < prc->left)
		{
			rcw.left = prc->left;
			clipped = true;
		}
		break;

	case frTop:

		if(rcw.top != prc->top || (pm->GetPrimary() && (rcw.bottom != prc->bottom)))
		{
			int h;

			if(pm->GetPrimary())
				h = prc->bottom - prc->top;
			else
				h = rcw.bottom - rcw.top;

			if(h < HSIZER_HEIGHT) h = HSIZER_HEIGHT;
			rcw.top    = prc->top;
			rcw.bottom = rcw.top + h;
			clipped   = true;
		}
		if(rcw.bottom > prc->bottom)
		{
			rcw.bottom = prc->bottom;
			clipped = true;
		}
		break;

	case frBottom:

		if(rcw.bottom != prc->bottom || (pm->GetPrimary() && (rcw.top != prc->top)))
		{
			int h;

			if(pm->GetPrimary())
				h = prc->bottom - prc->top;
			else
				h = rcw.bottom - rcw.top;

			if(h < HSIZER_HEIGHT) h = HSIZER_HEIGHT;
			rcw.top    = prc->bottom - h;
			rcw.bottom = prc->bottom;
			clipped   = true;
		}
		if(rcw.top < prc->top)
		{
			rcw.top = prc->top;
			clipped = true;
		}
		break;

	case frFloat:
		break;
	}

	switch(edge)
	{
	case frLeft:
	case frRight:

		if(rcw.top != prc->top)
		{
			int h = rcw.bottom - rcw.top;

			rcw.top    = prc->top;
			rcw.bottom = rcw.top + h;
			clipped = true;
		}
		if(rcw.bottom != prc->bottom)
		{
			rcw.bottom = prc->bottom;
			clipped = true;
		}
		break;

	case frTop:
	case frBottom:

		if(rcw.left != prc->left)
		{
			int w = rcw.right - rcw.left;

			rcw.left   = prc->left;
			rcw.right  = rcw.left + w;
			clipped = true;
		}
		if(rcw.right != prc->right)
		{
			rcw.right = prc->right;
			clipped   = true;
		}
		break;

	case frFloat:
		break;
	}
	if(rcw.left > rcw.right)
	{
		rcw.right = rcw.left;
		clipped = true;
	}
	if(rcw.top > rcw.bottom)
	{
		rcw.bottom = rcw.top;
		clipped = true;
	}
	if(clipped)
	{
		if(rcw.left < 0)
			rcw.left = 0;
		if(rcw.right - rcw.left < VSIZER_WIDTH)
			rcw.right = rcw.left + VSIZER_WIDTH;
		if(rcw.top < 0)
			rcw.top = 0;
		if(rcw.bottom - rcw.top < HSIZER_HEIGHT)
			rcw.bottom = rcw.top + HSIZER_HEIGHT;

		if(pm->GetPrimary() && rcw.right != prc->right)
		{
			int a = prc->right;
			int b = rcw.right;

			a = b;
		}
		MoveWindow(
					pm->GetWindow(),
					rcw.left,
					rcw.top,
					rcw.right - rcw.left,
					rcw.bottom - rcw.top,
					TRUE
				  );
		GetWindowPos(pm->GetWindow(), &rcw);
	}
	switch(edge)
	{
	case frLeft:

		// fit left edge of any panel to the right
		//
		if(pm->GetNext())
		{
			rcx.top   = rcw.top;
			rcx.left  = rcw.right;
			rcx.right = prc->right;
			rcx.bottom= rcw.bottom;

			FitPanel((PBAFPANEL)pm->GetNext(), &rcx, edge);
		}
		break;

	case frRight:

		// fit right edge of any panel to the left
		//
		if(pm->GetNext())
		{
			rcx.top   = rcw.top;
			rcx.left  = prc->left;
			rcx.right = rcw.left;
			rcx.bottom= rcw.bottom;

			FitPanel((PBAFPANEL)pm->GetNext(), &rcx, edge);
		}
		break;

	case frTop:

		// fit top edge of any panel below
		//
		if(pm->GetNext())
		{
			rcx.top   = rcw.bottom;
			rcx.left  = rcw.left;
			rcx.right = rcw.right;
			rcx.bottom= prc->bottom;

			FitPanel((PBAFPANEL)pm->GetNext(), &rcx, edge);
		}
		break;

	case frBottom:

		// fit bottom edge of any panel above
		//
		if(pm->GetNext())
		{
			rcx.bottom= rcw.top;
			rcx.left  = rcw.left;
			rcx.right = rcw.right;
			rcx.top   = prc->top;

			FitPanel((PBAFPANEL)pm->GetNext(), &rcx, edge);
		}
		break;

	case frFloat:
		break;
	}
	return errOK;
}

//***********************************************************************
ERRCODE BappPanel::GetPanelsRect(PBAFPANEL pp, LPRECT prc)
{
	RECT rcp;

	if(! pp || ! prc) return errBAD_PARAMETER;

	GetWindowPos(pp->GetWindow(), &rcp);
	prc->left   = rcp.left;
	prc->top    = rcp.top;
	prc->right  = rcp.right;
	prc->bottom = rcp.bottom;

	for(pp = (PBAFPANEL)pp->GetNext(); pp; pp = (PBAFPANEL)pp->GetNext())
	{
		GetWindowPos(pp->GetWindow(), &rcp);
		prc->left   = min(prc->left, rcp.left);
		prc->top    = min(prc->top, rcp.top);
		prc->right  = max(prc->right, rcp.right);
		prc->bottom = max(prc->bottom, rcp.bottom);
	}
	return errOK;
}

//***********************************************************************
PBAFPANEL BappPanel::GetPanels(BAF_EDGE edge)
{
	return m_panels[edge];
}

//***********************************************************************
ERRCODE BappPanel::OnResized(PBAFCOMP pby)
{
	PBAFPANEL  pc;
	PBAFPANEL pp;
	BAF_EDGE  edge;
	RECT      rcc, rcp;
	RECT      rcr, rcl, rct, rcb;

	if(pby == m_sizer)
	{
		// we been resized by our own resizer bar
		// so need to tell our owner (frame) that
		// we are a new size.  Our subpanels will
		// be sized in response to the WM_SIZE msg
		// in our message handler when we resize
		//
		return m_parent ? m_parent->OnResized(this) : errOK;
	}
	// some panel in the list has resized, so resize
	// the other panels to fit the new size of pby
	// recursive-fitting the rest of the panels
	//
	// first, figure out which edge the panel is docked to
	//
	do
	{
		for(edge = frTop, pc = m_panels[frTop]; pc; pc = pc->GetNext())
			if(pc == pby)
				break;
		if(pc)
			break;
		for(edge = frLeft, pc = m_panels[frLeft]; pc; pc = pc->GetNext())
			if(pc == pby)
				break;
		if(pc)
			break;
		for(edge = frBottom, pc = m_panels[frBottom]; pc; pc = pc->GetNext())
			if(pc == pby)
				break;
		if(pc)
			break;
		for(edge = frRight, pc = m_panels[frRight]; pc; pc = pc->GetNext())
			if(pc == pby)
				break;
		if(pc)
			break;
	}
	while(0);

	if(! pc)	// no such panel
		return errFAILURE;

	GetPanelClientRect(&rcc);

	// here we know pby is a panel so upcast is ok
	//
	pp = (PBAFPANEL)pc;

	// also know where in the app frame the panel is docked
	//
	switch(edge)
	{
	case frLeft:

		// if there is a more left panel, use is right edge as our left
		//
		if(pp->GetPrev())
		{
			GetWindowPos(pp->GetPrev()->GetWindow(), &rcp);
			rcc.left = rcp.right;
		}
		if(m_panels[frTop] && m_fulledged[frTop])
		{
			GetPanelsRect(m_panels[frTop], &rcp);
			rcc.top = rcp.bottom;
		}
		if(m_panels[frBottom] && m_fulledged[frBottom])
		{
			GetPanelsRect(m_panels[frBottom], &rcp);
			rcc.bottom = rcp.top;
		}
		FitPanel(pp, &rcc, frLeft);

		// now, get bounding rectangle of all left panels
		//
		GetPanelsRect(m_panels[frLeft], &rcl);

		rcr = rcc;
		rcr.left = rcl.right;

		if(m_panels[frRight])
		{
			// fit any right-sided panels into remaining client area
			//
			FitPanel(m_panels[frRight], &rcr, frRight);

			// now get the space between left and right panels
			//
			GetPanelsRect(m_panels[frRight], &rcr);
		}
		else
		{
			rcr.left = rcr.right = rcc.right;
		}
		rct = rcc;
		rct.left = rcl.right;
		rct.right = rcr.left;

		if(m_panels[frTop] && m_fulledged[frLeft])
		{
			// and fit top panels to inside that space
			//
			FitPanel(m_panels[frTop], &rct, frTop);

			// finally, get the room left for bottom panels
			//
			GetPanelsRect(m_panels[frTop], &rct);
		}
		else
		{
			rct.bottom = rct.top = rcc.top;
		}
		rcb = rcc;
		rcb.left = rcl.right;
		rcb.right = rcr.left;
		rcb.top = rct.bottom;

		if(m_panels[frBottom] && m_fulledged[frLeft])
		{
			// and fit the bottom panels to that
			//
			FitPanel(m_panels[frBottom], &rcb, frBottom);
		}
		break;

	case frRight:

		// if there is a more right panel, use its left edge as our right
		//
		if(pp->GetPrev())
		{
			GetWindowPos(pp->GetPrev()->GetWindow(), &rcp);
			rcc.right = rcp.left;
		}
		if(m_panels[frTop] && m_fulledged[frTop])
		{
			GetPanelsRect(m_panels[frTop], &rcp);
			rcc.top = rcp.bottom;
		}
		if(m_panels[frBottom] && m_fulledged[frBottom])
		{
			GetPanelsRect(m_panels[frBottom], &rcp);
			rcc.bottom = rcp.top;
		}
		FitPanel(pp, &rcc, frRight);

		// now, get bounding rectangle of all right panels
		//
		GetPanelsRect(m_panels[frRight], &rcr);

		rcl = rcc;
		rcl.right = rcr.left;

		if(m_panels[frLeft])
		{
			// fit any left-sided panels into remaining client area
			//
			FitPanel(m_panels[frLeft], &rcl, frLeft);

			// now get the space between left and right panels
			//
			GetPanelsRect(m_panels[frLeft], &rcl);
		}
		else
		{
			rcl.left = rcl.right = rcc.left;
		}
		rct = rcc;
		rct.left = rcl.right;
		rct.right = rcr.left;

		if(m_panels[frTop] && m_fulledged[frRight])
		{
			// and fit top panels to inside that space
			//
			FitPanel(m_panels[frTop], &rct, frTop);

			// finally, get the room left for bottom panels
			//
			GetPanelsRect(m_panels[frTop], &rct);
		}
		else
		{
			rct.bottom = rct.top = rcc.top;
		}
		rcb = rcc;
		rcb.left = rcl.right;
		rcb.right = rcr.left;
		rcb.top = rct.bottom;

		if(m_panels[frBottom] && m_fulledged[frRight])
		{
			// and fit the bottom panels to that
			//
			FitPanel(m_panels[frBottom], &rcb, frBottom);
		}
		break;

	case frTop:

		// if there is a higher panel, use its bottom edge as our top
		//
		if(pp->GetPrev())
		{
			GetWindowPos(pp->GetPrev()->GetWindow(), &rcp);
			rcc.top = rcp.bottom;
		}
		if(m_panels[frLeft] && m_fulledged[frLeft])
		{
			GetPanelsRect(m_panels[frLeft], &rcp);
			rcc.left = rcp.right;
		}
		if(m_panels[frRight] && m_fulledged[frRight])
		{
			GetPanelsRect(m_panels[frRight], &rcp);
			rcc.right = rcp.left;
		}
		FitPanel(pp, &rcc, frTop);

		// now, get bounding rectangle of all top panels
		//
		GetPanelsRect(m_panels[frTop], &rct);

		rcb = rcc;
		rcb.top = rct.bottom;

		if(m_panels[frBottom])
		{
			// fit any bottom panels into remaining client area
			//
			FitPanel(m_panels[frBottom], &rcb, frBottom);

			// now get the space between top and bottom panels
			//
			GetPanelsRect(m_panels[frBottom], &rcb);
		}
		else
		{
			rcb.top = rcb.bottom = rcc.bottom;
		}
		rcl = rcc;
		rcl.top = rct.bottom;
		rcl.bottom = rcb.top;

		if(m_panels[frLeft] && m_fulledged[frTop])
		{
			// and fit left panels to inside that space
			//
			FitPanel(m_panels[frLeft], &rcl, frLeft);

			GetPanelsRect(m_panels[frLeft], &rcl);
		}
		else
		{
			rcl.left = rcl.right = rcc.left;
		}
		rcr = rcc;
		rcr.left = rcl.right;
		rcr.bottom = rcb.top;
		rcr.top = rct.bottom;

		if(m_panels[frRight] && m_fulledged[frTop])
		{
			// and fit the right panels to that
			//
			FitPanel(m_panels[frRight], &rcr, frRight);
		}
		break;

	case frBottom:

		// if there is a lower panel, use its top edge as our bottom
		//
		if(pp->GetPrev())
		{
			GetWindowPos(pp->GetPrev()->GetWindow(), &rcp);
			rcc.bottom = rcp.top;
		}
		if(m_panels[frLeft] && m_fulledged[frLeft])
		{
			GetPanelsRect(m_panels[frLeft], &rcp);
			rcc.left = rcp.right;
		}
		if(m_panels[frRight] && m_fulledged[frRight])
		{
			GetPanelsRect(m_panels[frRight], &rcp);
			rcc.right = rcp.left;
		}
		FitPanel(pp, &rcc, frBottom);

		// now, get bounding rectangle of all bottom panels
		//
		GetPanelsRect(m_panels[frBottom], &rcb);

		rct = rcc;
		rct.bottom = rcb.top;

		if(m_panels[frTop])
		{
			// fit any bottom panels into remaining client area
			//
			FitPanel(m_panels[frTop], &rct, frTop);

			// now get the space between top and bottom panels
			//
			GetPanelsRect(m_panels[frTop], &rct);
		}
		else
		{
			rct.top = rct.bottom = rcc.top;
		}
		rcl = rcc;
		rcl.top = rct.bottom;
		rcl.bottom = rcb.top;

		if(m_panels[frLeft] && m_fulledged[frBottom])
		{
			// and fit left panels to inside that space
			//
			FitPanel(m_panels[frLeft], &rcl, frLeft);

			GetPanelsRect(m_panels[frLeft], &rcl);
		}
		else
		{
			rcl.left = rcl.right = rcc.left;
		}
		rcr = rcc;
		rcr.left = rcl.right;
		rcr.bottom = rcb.top;
		rcr.top = rct.bottom;

		if(m_panels[frRight] && m_fulledged[frBottom])
		{
			// and fit the right panels to that
			//
			FitPanel(m_panels[frRight], &rcr, frRight);
		}
		break;

	case frFloat:
		break;
	}
	return errOK;
}

//***********************************************************************
PBAFPANEL BappPanel::FindPanel(LPCTSTR pName)
{
	PBAFPANEL pp, px;

	for(pp = this; pp->GetPrev();)
		pp = (PBAFPANEL)pp->GetPrev();

	for(; pp; pp = (PBAFPANEL)pp->GetNext())
	{
		if(! _tcscmp(pp->GetName(), pName))
			return pp;
		if(pp->m_panels[frLeft])
		{
			px = pp->m_panels[frLeft]->FindPanel(pName);
			if(px) return px;
		}
		if(pp->m_panels[frRight])
		{
			px = pp->m_panels[frRight]->FindPanel(pName);
			if(px) return px;
		}
		if(pp->m_panels[frTop])
		{
			px = pp->m_panels[frTop]->FindPanel(pName);
			if(px) return px;
		}
		if(pp->m_panels[frBottom])
		{
			px = pp->m_panels[frBottom]->FindPanel(pName);
			if(px) return px;
		}
	}
	return NULL;
}

//***********************************************************************
PBAFPANE BappPanel::FindPane(LPCTSTR pName)
{
	PBAFPANEL pp;
	PBAFPANE  pa;

	for(pp = this; pp->GetPrev();)
		pp = (PBAFPANEL)pp->GetPrev();

	for(; pp; pp = (PBAFPANEL)pp->GetNext())
	{
		pa = pp->GetPanes();

		while(pa)
		{
			if(! _tcscmp(pa->GetName(), pName))
				return pa;
			pa = pa->GetNext();
		}
		if(pp->m_panels[frLeft])
		{
			pa = pp->m_panels[frLeft]->FindPane(pName);
			if(pa) return pa;
		}
		if(pp->m_panels[frRight])
		{
			pa = pp->m_panels[frRight]->FindPane(pName);
			if(pa) return pa;
		}
		if(pp->m_panels[frTop])
		{
			pa = pp->m_panels[frTop]->FindPane(pName);
			if(pa) return pa;
		}
		if(pp->m_panels[frBottom])
		{
			pa = pp->m_panels[frBottom]->FindPane(pName);
			if(pa) return pa;
		}
	}
	return NULL;
}

//***********************************************************************
ERRCODE BappPanel::GetPanelClientRect(LPRECT prc)
{
	if(! prc) return errBAD_PARAMETER;
	prc->left = prc->right = prc->bottom = prc->top = 0;
	if(m_hwnd)
	{
		GetClientRect(m_hwnd, prc);

		if(GetBorder() != frNoBorder)
		{
			prc->left	+= TABPANE_LEFTMARG		+ TABPANE_BORDER * 2;
			prc->top	+= TABPANE_TOPMARG		+ TABPANE_BORDER * 2;
			prc->right	-= TABPANE_RIGHTMARG	+ TABPANE_BORDER * 2 + 2;
			prc->bottom	-= TABPANE_BOTTOMMARG	+ TABPANE_BORDER * 2 + 2;
		}
		if(GetBorder() == frTopTabs || GetBorder() == frBottomTabs)
		{
			if(HasMultiplePanes())
			{
				if(GetBorder() == frTopTabs)
					prc->top += TABPANE_TABHEIGHT;
				else
					prc->bottom -= TABPANE_TABHEIGHT;
			}
		}
		if(m_sizer)
		{
			switch(m_edge)
			{
			case frLeft:
				prc->right	-= VSIZER_WIDTH;
				break;
			case frRight:
				prc->left	+= VSIZER_WIDTH;
				break;
			case frTop:
				prc->bottom	-= HSIZER_HEIGHT;
				break;
			case frBottom:
				prc->top	+= HSIZER_HEIGHT;
				break;
			case frFloat:
				break;
			}
		}
	}
	return errOK;
}

//***********************************************************************
ERRCODE BappPanel::AddPane(PBAFPANE pp)
{
	int npanes = 0;

	if(! pp) return errBAD_PARAMETER;
	if(! m_panes)
	{
		pp->SetNext(NULL);
		pp->SetPrev(NULL);
		m_panes = pp;
	}
	else
	{
		PBAFPANE px;

		for(px = m_panes; px->GetNext(); px = px->GetNext())
			npanes++;
		px->SetNext(pp);
		pp->SetPrev(px);
		pp->SetNext(NULL);
	}
	pp->SetParentPanel(this);

	InvalidateRect(m_hwnd, NULL, TRUE);
	return errOK;
}

//***********************************************************************
ERRCODE BappPanel::RemovePane(PBAFPANE pp)
{
	PBAFPANE px = NULL;

	if(! pp) return errBAD_PARAMETER;
	if(! m_panes)
	{
		return errFAILURE;
	}
	else
	{
		// chose other pane to activate if any
		//
		if(pp->GetNext())
			px = pp->GetNext();
		else
			px = pp->GetPrev();

		if(pp == m_panes)
		{
			m_panes = pp->GetNext();
			if(m_panes) m_panes->SetPrev(NULL);
		}
		else
		{
			PBAFPANE px;

			for(px = m_panes; px->GetNext() != pp; px = px->GetNext())
				;
			if(px && px->GetNext() == pp)
			{
				px->SetNext(pp->GetNext());
				if(px->GetNext())
					px->GetNext()->SetPrev(px);
				pp->SetPrev(NULL);
				pp->SetNext(NULL);
			}
		}
	}
	pp->Deactivate();
	pp->SetParentPanel(NULL);
	if(px) px->Activate();
	InvalidateRect(m_hwnd, NULL, TRUE);
	return errOK;
}

//***********************************************************************
ERRCODE BappPanel::ActivatePane(int pane_num)
{
	PBAFPANE pp;

	for(pp = m_panes; pane_num > 0 && pp; pp = pp->GetNext())
		pane_num--;
	ActivatePane(pp);
	return errOK;
}

//***********************************************************************
ERRCODE BappPanel::ActivatePane(PBAFPANE pp)
{
	PBAFPANE px;

	if(pp) pp->Activate();

	// deactivate any other actives
	for(px = m_panes; px; px = px->GetNext())
	{
		if(px != pp)
		{
			if(px->GetActive() && px != pp)
				px->Deactivate();
			else
				if(px->GetWindow())
					ShowWindow(px->GetWindow(), SW_HIDE);
		}
	}
	InvalidateRect(GetWindow(), NULL, FALSE);
	return errOK;
}

//***********************************************************************
ERRCODE BappPanel::ActivatePane(LPCTSTR pname)
{
	PBAFPANE pp;

	if(! pname) return errBAD_PARAMETER;
	for(pp = m_panes; pp; pp = pp->GetNext())
		if(! _tcscmp(pname, pp->GetName()))
			break;
	ActivatePane(pp);
	return errOK;
}
