#include "bedx.h"


//**************************************************************************
BinfoPane::BinfoPane(LPCTSTR name, BappPanel* pPanel, Bed* pParent, bool lines, bool images)
		:
		BappPane(name, pPanel),
		m_editor(pParent),
		m_view(NULL),
		m_hwndTree(NULL),
		m_ignoreNotify(false),
		m_hasimages(images),
		m_ilist(NULL),
		m_style(WS_CHILD | WS_VSCROLL | TVS_HASBUTTONS /*| TVS_SHOWSELALWAYS*/ | TVS_HASLINES | TVS_LINESATROOT)
{
	if(! lines)
		m_style &= ~ (TVS_HASLINES | TVS_LINESATROOT);
}

//**************************************************************************
BinfoPane::~BinfoPane()
{
	if(m_hwndTree)
	{
		DestroyWindow(m_hwndTree);
		m_hwndTree = NULL;
	}
	if(m_ilist)
	{
		ImageList_Destroy(m_ilist);
		m_ilist = NULL;
	}
}

//**************************************************************************
LRESULT BinfoPane::OnMessage(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	TVITEM	tvitem;
	TCHAR	tvtext[256];

	switch(msg)
	{
	case WM_NOTIFY:

	{
		LPNMHDR			pHdr = (LPNMHDR)lp;
#if ! defined(Windows) || ! defined(__GNUC__)
		LPNMTVKEYDOWN	nmTreeKey;
#endif
		if(IgnoreNotify())
			return 0;

		switch(pHdr->code)
		{
		case NM_DBLCLK:

			tvitem.hItem		= TreeView_GetSelection(pHdr->hwndFrom);
			if(! tvitem.hItem) break;
			tvitem.mask		= TVIF_TEXT | TVIF_PARAM;
			tvitem.cchTextMax	= 256;
			tvitem.pszText		= tvtext;
			TreeView_GetItem(GetWindow(), &tvitem);
			Select(&tvitem);
			break;

#if ! defined(Windows) || ! defined(__GNUC__)
		case TVN_KEYDOWN:

			nmTreeKey = (LPNMTVKEYDOWN)pHdr;
			if(nmTreeKey->wVKey == VK_RETURN)
			{
				tvitem.hItem		= TreeView_GetSelection(GetWindow());
				tvitem.mask		= TVIF_TEXT | TVIF_PARAM;
				tvitem.cchTextMax	= 256;
				tvitem.pszText		= tvtext;
				TreeView_GetItem(GetWindow(), &tvitem);
				Select(&tvitem);
			}
			break;
#endif
		case NM_RCLICK:

			{
				RECT  rw;
				POINT mp;

				GetCursorPos(&mp);
				GetWindowRect(GetWindow(), &rw);
				mp.x -= rw.left;
				mp.y -= rw.top;
				MouseMenu(mp.x, mp.y);
			}
			break;

		default:
			break;
		}
		break;
	}

	default:
		return BappPane::OnMessage(hWnd, msg, wp, lp);
	}
	return 0;
}

//**************************************************************************
void BinfoPane::FitToPanel(void)
{
	BappPane::FitToPanel();

	if(m_editor)
	{
		if(GetParentPanel() == m_editor->GetStatusPanel())
			m_editor->FitStatusWindows();
	}
}

//**************************************************************************
void BinfoPane::LoadImages(void)
{
}

//**************************************************************************
void BinfoPane::Activate(void)
{
	if(! m_hwndTree)
		SetupWindow();
	BappPane::Activate();
}

//**************************************************************************
void BinfoPane::Deactivate(void)
{
	BappPane::Deactivate();
}

//**************************************************************************
void BinfoPane::SetView(Bview* pView)
{
	COLORREF bkg;
	BYTE     alpha;
	
	if(! pView)
		return;
	m_view = pView;
	
	if (m_hwndTree)
	{
		LONG frg;
		BYTE r,g,b;
		
		bkg = pView->GetBackground(alpha);
					
		b = bkg & 0xFF;
		g = (bkg & 0xFF00) >> 8;
		r = (bkg & 0xFF0000) >> 16;
		
		// use light text for dark bkg
		if (r < 127 && g < 127 && b < 127)
		{
			frg = RGB(255, 255, 255);
			SendMessage(m_hwndTree, TVM_SETTEXTCOLOR, 0, frg);
		}
		else
		{
			frg = RGB(0, 0, 0);
			SendMessage(m_hwndTree, TVM_SETTEXTCOLOR, 0, frg);
		}
		SendMessage(m_hwndTree, TVM_SETBKCOLOR, 0, bkg);
		SetWindowLong(m_hwndTree, GWL_EXSTYLE, GetWindowLong(m_hwndTree, GWL_EXSTYLE) | WS_EX_LAYERED);
		SetLayeredWindowAttributes(m_hwndTree, 0, alpha, LWA_ALPHA);
	}
}

//**************************************************************************
void BinfoPane::SetupWindow()
{
	if(! m_hwndTree)
	{
		m_hwndTree = MakeNewTree();
	}
	else
	{
		if(m_parent)
			m_parent->GetPanelClientRect(&m_viewrc);

		MoveWindow(
					m_hwndTree, 
					m_viewrc.left, m_viewrc.top,
					m_viewrc.right - m_viewrc.left - 1,
					m_viewrc.bottom - m_viewrc.top - 1,
					FALSE
				  );
	}
}

//**************************************************************************
HWND BinfoPane::MakeNewTree()
{
	HWND hwnd, hwndParent;
	RECT rc;

	if(m_parent)
	{
		m_parent->GetPanelClientRect(&rc);
		hwndParent = m_parent->GetWindow();
	}
	else
	{
		return NULL;
	}
	hwnd = CreateWindow(
								WC_TREEVIEW,
								_T("itree"),
								WS_CHILD | WS_VSCROLL | m_style,
								rc.left, rc.top,
								rc.right - rc.left - 1,
								rc.bottom - rc.top - 1,
								hwndParent,
								NULL,
								GetInstance(),
								NULL
							);
	if(m_hasimages)
	{
		if(! m_ilist)
		{
			m_ilist = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 3, 3);
			LoadImages();
		}
		TreeView_SetImageList(hwnd, m_ilist, TVSIL_NORMAL);
	}
	if(m_view && hwnd)
	{
		COLORREF bkg;
		BYTE alpha;

		bkg = m_view->GetBackground(alpha);
		SendMessage(hwnd, TVM_SETBKCOLOR, 0, bkg);
		SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
	}
	return hwnd;
}


//**************************************************************************
void BinfoPane::MouseMenu(int x, int y)
{
	HWND	hWnd;
	HMENU	hMenu;
	RECT	rectw;
	int		rc;

	if(! HasMenu())
		return;

	hWnd = GetParent(m_hwndTree);
	if(! hWnd)
		return;

	hMenu = CreatePopupMenu();

	SpecificMenu(hMenu);

	GetWindowRect(hWnd, &rectw);

	rc = TrackPopupMenu(
						hMenu, 
						TPM_RETURNCMD, 
						x + rectw.left,
						y + rectw.top,
						0,
						hWnd,
						NULL
					   );
                       
	DestroyMenu(hMenu);
	switch(rc)
	{
	case ID_TOOLS_CLEAR:
		TreeView_DeleteAllItems(m_hwndTree);
		break;
	}
}


//**************************************************************************
void BinfoPane::Select(LPTVITEM pItem)
{
	return;
}

