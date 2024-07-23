#include "bedx.h"

//**************************************************************************
BfuncInfo::BfuncInfo(LPCTSTR name, BappPanel* pPanel, Bed* pParent)
	:
	BinfoPane(name, pPanel, pParent, true, true),
	m_hCurItem(NULL),
	m_funcstartline(1),
	m_funcendline(100000)
{
}

//**************************************************************************
BfuncInfo::~BfuncInfo()
{
}

//**************************************************************************
void BfuncInfo::LoadImages()
{
	HICON hIcon;

	if(! m_ilist) return;

	hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_FUNCTION));
	ImageList_AddIcon(m_ilist, hIcon);
	DestroyIcon(hIcon);
	hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_CLASS));
	ImageList_AddIcon(m_ilist, hIcon);
	DestroyIcon(hIcon);
}

//**************************************************************************
void BfuncInfo::SetView(Bview* pView)
{
	HWND hWnd;
	HWND hOldWnd;

	if(! pView || ! m_parent) return;

	m_ignoreNotify = true;

	// build the new tree hidden, cause visible
	// tv inserts make for real slow filling
	//
	hWnd	 = MakeNewTree();
	hOldWnd  = m_hwndTree;
	m_hwndTree = hWnd;

	BinfoPane::SetView(pView);

	// enum functions into hidden tree
	m_hCurParent = NULL;
	m_hCurItem	 = NULL;

	pView->EnumFunctions(EnumFuncsStub, this);

	m_hCurParent = NULL;
	m_hCurItem	 = NULL;

	// show it now and kill the old tree
	TreeView_SelectItem(m_hwndTree, NULL);
	ShowWindow(m_hwndTree, m_active ? SW_SHOW : SW_HIDE);
	DestroyWindow(hOldWnd);
	m_ignoreNotify = false;
}

//**************************************************************************
void BfuncInfo::Select(LPTVITEM pItem)
{
	// selecting a new tree item
	if(! pItem) return;
	if(pItem->mask & LVIF_PARAM)
	{
		// all we need in the line number
		//
		if(m_view)
		{
			m_view->ClearLineInfo(liIsSearchTarget);
			m_view->PushParm(liIsSearchTarget, ptNumber);
			m_view->PushParm(pItem->lParam, ptNumber);
			m_view->Dispatch(SetLineType);
			m_view->SetFocus(true);
		}
	}
	return;
}

//**************************************************************************
void BfuncInfo::SetToLine(int line)
{
	TVITEM tvitem;
	TCHAR tvtext[MAX_PATH];
	int funcstartline, funcendline;
	bool fwd;
	HTREEITEM hpi;
	
	if(line < 1 || ! m_hwndTree)
		return;

	// if line within cached region, forget it
	//
	if(m_hCurItem)
	{
		if(line >= m_funcstartline && line < m_funcendline)
			return;
	}
	if(! m_hCurItem)
	{
		if(! (m_hCurItem = TreeView_GetNextItem(m_hwndTree, NULL, TVGN_ROOT)))
			return;
		fwd = true;
		funcstartline = 1;
	}
	else
	{
		fwd = line > m_funcstartline;
		funcstartline = m_funcstartline;
	}
	if(! m_hCurItem)
		return;

	hpi 			= m_hCurItem;
	
	while(m_hCurItem)
	{
		tvitem.hItem		= m_hCurItem;
		tvitem.mask			= TVIF_TEXT | TVIF_PARAM;
		tvitem.cchTextMax	= MAX_PATH;
		tvitem.pszText		= tvtext;

		funcendline			= MAX_LINE_LEN;
		if(! TreeView_GetItem(m_hwndTree, &tvitem))
			return;
		if(fwd)
		{
			if(tvitem.lParam > line)
			{
				funcendline = tvitem.lParam - 1;
				break;
			}
		}
		else
		{
			if(tvitem.lParam < line)
			{
				funcendline   = funcstartline - 1;
				funcstartline = tvitem.lParam;
				break;
			}
		}
		hpi = m_hCurItem;
		funcstartline = tvitem.lParam;
		if(! (m_hCurItem = TreeView_GetNextItem(m_hwndTree, m_hCurItem, fwd ? TVGN_NEXT : TVGN_PREVIOUS)))
		{
			if(fwd)
			{
				m_hCurItem = hpi;
				m_funcstartline = funcstartline;
				m_funcendline   = MAX_LINE_LEN;
			}
			else
			{
				m_hCurItem = hpi;
				m_funcstartline = 1;
				m_funcendline   = funcendline;
			}
			return;
		}
	}
	m_funcstartline = funcstartline;
	m_funcendline   = funcendline;
	TreeView_SelectItem(m_hwndTree, hpi);
}


// STATIC
//**************************************************************************
ERRCODE BfuncInfo::EnumFuncsStub(LPVOID cookie, LPCTSTR pName, LPCTSTR pTypeName, int isPtr, int line, EnumFuncsType type)
{
	if(! cookie) return errBAD_PARAMETER;

	return ((BfuncInfo*)cookie)->EnumFunc(pName, pTypeName, isPtr, line, type);
}

//**************************************************************************
ERRCODE BfuncInfo::EnumFunc(LPCTSTR pName, LPCTSTR pTypeName, int isPtr, int line, EnumFuncsType type)
{
	TVINSERTSTRUCT tvItem;

	//_tprintf(_T("%d " _Pfs_ " " _Pfs_ "\n"), type, pName, pTypeName);
	
	switch(type)
	{
	case efVar:
		break;
	case efFunction:

		tvItem.hParent		= m_hCurParent;
		tvItem.hInsertAfter = m_hCurItem;

		tvItem.item.mask	= TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvItem.item.lParam	= line;
		tvItem.item.pszText = (LPTSTR)pName;
		tvItem.item.iImage			= 0;
		tvItem.item.iSelectedImage	= 0;

		m_hCurItem = TreeView_InsertItem(m_hwndTree, &tvItem);
	
		break;

	case efClass:
	case efInterface:

		tvItem.hParent		= m_hCurParent;
		tvItem.hInsertAfter = m_hCurItem;

		tvItem.item.mask	= TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvItem.item.lParam	= line;
		tvItem.item.pszText = (LPTSTR)pName;
		tvItem.item.iImage			= 1;
		tvItem.item.iSelectedImage	= 1;

		m_hCurItem   = TreeView_InsertItem(m_hwndTree, &tvItem);
	
		m_hCurParent = m_hCurItem;
		m_hCurItem   = NULL;

		break;

	case efTypeName:

		if(! m_hCurItem && m_hCurParent)
		{
			tvItem.item.hItem   = m_hCurParent;
			tvItem.item.mask	= TVIF_TEXT;
			tvItem.item.lParam	= line;
			tvItem.item.pszText = (LPTSTR)pName;
			TreeView_SetItem(m_hwndTree, &tvItem.item);
		}
		break;

	case efSetClass:
		if (! pName || ! pName[0]) 
		{
			HTREEITEM classparent;

			// out of class scope, set current item to last top level item
			do
			{
				classparent = TreeView_GetParent(m_hwndTree, m_hCurItem);
				if(classparent != NULL)
				{
					m_hCurItem = classparent;
					m_hCurParent = TreeView_GetParent(m_hwndTree, classparent);
				}
				else
				{
					m_hCurParent = NULL;
				}
			}
			while (classparent != NULL);
		}
		break;

	case efEndClass:
	case efEndInterface:

		if(m_hCurItem)
		{
			HTREEITEM classparent = TreeView_GetParent(m_hwndTree, m_hCurItem);

			// new parent is class's parent and new item is the class we're closing
			//
			if(classparent != NULL)
			{
				m_hCurItem = classparent;
				m_hCurParent = TreeView_GetParent(m_hwndTree, classparent);
			}
			else
			{
				m_hCurParent = NULL;
			}
		}
		else
		{
			// empty class (no items) so new parent is class's paretn
			// and new cur item is the empty class
			//
			if (m_hCurParent)
			{
				m_hCurItem = m_hCurParent;
				m_hCurParent = TreeView_GetParent(m_hwndTree, m_hCurItem);
			}
		}
		break;
		
	default:
		
		break;
	}
	return errOK;
}

