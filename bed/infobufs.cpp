#include "bedx.h"

//**************************************************************************
BpickInfo::BpickInfo(LPCTSTR name, BappPanel* pPanel, Bed* pParent)
	:
	BinfoPane(name, pPanel, pParent, true, true)
{
}

//**************************************************************************
BpickInfo::~BpickInfo()
{
}

//**************************************************************************
void BpickInfo::LoadImages()
{
#if 1
	HICON hIcon;

	if(! m_ilist) return;

	hIcon = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_BUF16), IMAGE_ICON, 16, 16, 0);
	ImageList_AddIcon(m_ilist, hIcon);
	hIcon = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_BUFMOD16), IMAGE_ICON, 16, 16, 0);
	ImageList_AddIcon(m_ilist, hIcon);
#else
	HBITMAP hBitmap;

	if(! m_ilist) return;

	hBitmap = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_BUF16));
	ImageList_Add(m_ilist, hBitmap, NULL);
	hBitmap = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_BUFMOD16));
	ImageList_Add(m_ilist, hBitmap, NULL);
#endif
}


//**************************************************************************
void BpickInfo::SetView(Bview* pView)
{
	Bed*			pBed;
	Bbuffer*		pBuf;
	Bbuffer*		pCurBuf;
	BbufList*		pList;
	TVINSERTSTRUCT	tvItem;
	HWND			hwndNewTree, hOldWnd;
	HTREEITEM		hItem, hCurItem;
	int				index;

	static Bmutex lock;

	if (!pView || !m_parent) return;

	BinfoPane::SetView(pView);

	if (!(pBuf = pCurBuf = pView->GetBuffer()))	{
		return;
	}
	if (!(pBed = pBuf->GetEditor())) {
		return;
	}
	m_ignoreNotify = true;

	hwndNewTree = MakeNewTree();

	hCurItem = NULL;
	for (pList = pBed->GetBuffers(), index = 0; pList; pList = pList->GetNext(pList), index++)
	{
		if(! (pBuf = *pList)) break;

		tvItem.hParent		= NULL;
		tvItem.hInsertAfter = TVI_LAST;

		tvItem.item.mask	= TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE;
		tvItem.item.pszText = (LPTSTR)pBuf->GetShortName();
		tvItem.item.lParam  = index;
		tvItem.item.iImage	= 0;

		//_tprintf(_T("buf=" _Pfs_ "\n"), pBuf->GetName());
		hItem = TreeView_InsertItem(hwndNewTree, &tvItem);

		if(pBuf == pCurBuf) 
			hCurItem = hItem;
	}
	hOldWnd = m_hwndTree;
	m_hwndTree = hwndNewTree;
	ShowWindow(m_hwndTree, m_active ? SW_SHOW : SW_HIDE);
	if (hCurItem)
		TreeView_SelectItem(hwndNewTree, hCurItem);
	DestroyWindow(hOldWnd);
	m_ignoreNotify = false;
}

//**************************************************************************
void BpickInfo::Select(LPTVITEM pItem)
{
	Bed*		pBed;
	Bbuffer*	pBuf;
	BbufList*	pList;
	int			index;

	if(m_ignoreNotify)					return;
	if(! m_view)						return;
	if(! (pBuf = m_view->GetBuffer()))	return;
	if(! (pBed = pBuf->GetEditor()))	return;
	if(pBed != m_editor)				return;
	if(! (pItem->mask & TVIF_TEXT))		return;

	for(pList = pBed->GetBuffers(), index = 0; pList; pList = pList->GetNext(pList), index++)
	{
		if(! (pBuf = *pList)) break;

		if(index == pItem->lParam)
		{
			pBed->EditBuffer(pBuf);
			return;
		}
		else
		{
		}
	}
}

