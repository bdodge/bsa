
#include "bedx.h"

#define MAX_NAMEBUF	256
#define MAX_PROTO_HEIGHT 220

//**************************************************************************
void Bview::CheckCloseProtoWindow()
{
	if(m_hWndProto)
	{
		HWND hWnd = m_hWndProto;

		m_hWndProto = NULL;
		PostMessage(hWnd, WM_CLOSE, 0, 0);
	}
}


//**************************************************************************
void Bview::SelectProtoItem()
{
	if(m_hWndProto)
	{
		HTREEITEM hItem;
		TVITEM    tvItem;
		TCHAR	  nameBuf[MAX_NAMEBUF];
		bool	  oktokill;

		hItem = TreeView_GetSelection(m_hWndProto);

		tvItem.hItem		= hItem;
		tvItem.mask			= TVIF_TEXT;
		tvItem.pszText		= nameBuf;
		tvItem.cchTextMax	= MAX_NAMEBUF;

		oktokill = true;

		if(TreeView_GetItem(m_hWndProto, &tvItem))
		{
			if(tvItem.pszText)
			{
				int len = _tcslen(tvItem.pszText);

				if(len > 0)
				{
					PushParm(tvItem.pszText, len, ptString);
					Dispatch(::Insert);
					if(_tcsstr(tvItem.pszText, _T(",")))
						oktokill = false;
				}
			}
		}
		if(oktokill)
		{
			CheckCloseProtoWindow();
		}
	}
}

//**************************************************************************
void Bview::ZoomProtoItem(int key)
{
#if 0 // use in case windows can't sort/incremental search
	HTREEITEM hItem, hFirstItem;
	TVITEM    tvi;
	TCHAR	  nameBuf[MAX_NAMEBUF];

	if(! m_hWndProto)
		return;
	
	hFirstItem = TreeView_GetRoot(m_hWndProto);
	if(! hFirstItem)
		return;

	if(m_protzoomx >= MAX_PROTO_ZOOM)
		return;

	if(key > 32 && key != 0x7f)
	{
		m_protzoomb[m_protzoomx++] = (TCHAR)key;
		m_protzoomb[m_protzoomx]   = _T('\0');
	}
	else if(key == 8 || key == 0x7f)
	{
		if(m_protzoomx > 0)
		{
			m_protzoomb[--m_protzoomx] = _T('\0');
		}
	}
	// find closest match in sorted member tree to chars typed so far
	for(hItem = hFirstItem; hItem;)
	{
		tvi.mask		= TVIF_TEXT;
		tvi.pszText		= nameBuf;
		tvi.cchTextMax	= MAX_NAMEBUF;
		tvi.hItem		= hItem;

		if(TreeView_GetItem(m_hWndProto, &tvi))
		{
			LPCTSTR lpa = tvi.pszText;
			LPCTSTR lpb = m_protzoomb;
			TCHAR   a, b;
			int i, j, cmp;

			j = _tcslen(lpa);
			if(j >= m_protzoomx)
			{
				for(i = 0, cmp = 0; i < j && i < m_protzoomx && cmp == 0 ; i++)
				{
					a = lpa[i];
					b = lpb[i];

					if(a == _T('~')) a = _T(' ');
					if(b == _T('~')) b = _T(' ');
					if(a >= 'a' && a <= 'z')
						a = a + 'A' - 'a';
					if(b >= 'a' && b <= 'z')
						b = b + 'A' - 'a';

					if(a > b)
					{
						cmp = 1; 
					}
					else if(a < b)
					{
						cmp = -1;
					}
				}
			}
			else
			{
				cmp = -1;
			}
			_tprintf(_T("zoom "_Pfs_" to %s = %d\n"), lpa, lpb, cmp);
			if(cmp == 0)
			{
				// zoom buffer matches this item
				TreeView_SelectItem(m_hWndProto, hItem);
				if(i == j)
				{
					// complete match
					SelectProtoItem();
				}
				return;
			}
			else if(cmp < 0) 
			{
				// zoom buffer < current item 
				hItem = TreeView_GetNextSibling(m_hWndProto, hItem);
			}
			else
			{
				// zoom buffer already > current, so can't exist!
				TreeView_SelectItem(m_hWndProto, hItem);
				return;
			}
		}
		else
		{
			return;
		}
	}
#endif
}

//**************************************************************************
ERRCODE Bview::ProtoWindow(LPBSYM pSym, EnumFuncsType type)
{
	TVINSERTSTRUCT	tvItem;
	HTREEITEM		hItem, hFirstItem = NULL;
	RECT			rc;
	BpdbInfo*		pMember;
	HWND			hWndParent;
	POINT			ptCaret;
	int				ulx, uly, w, h, maxh;
	int				nMembers;
	bool			multiline = false;
	TCHAR			namebuf[MAX_NAMEBUF];
	LPTSTR			pn, ps, pa;
	int				maxArgLen;
	int				fontwidth, fontheight;

	if(! pSym) return errBAD_PARAMETER;

	// close any extant popup
	//
	CheckCloseProtoWindow();

	m_protzoomx = 0;
	m_protzoomb[0] = _T('\0');

	// mak sure image list is initialized
	//
	if(! m_protimgs)
	{
		HICON hIcon;
	
		m_protimgs = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 2);

		if(m_protimgs)
		{	
			hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_FUNCTION));
			ImageList_AddIcon(m_protimgs, hIcon);
			DestroyIcon(hIcon);
			hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_DATA));
			ImageList_AddIcon(m_protimgs, hIcon);
			DestroyIcon(hIcon);
		}
	}
	// parent popup on edit window panel
	hWndParent = m_hwnd; //GetParent(m_hwnd);

	// position popup at caret pos in global space
	GetCaretPos(&ptCaret);
	ulx = ptCaret.x;
	uly = ptCaret.y + m_cacheFontHeight;

	fontwidth = 7;
	fontheight = 16;

	if(type == efFunction)
	{
		pn = namebuf;
		*pn++ = _T('(');

		for(nMembers = maxArgLen = 0, pMember = pSym->f_members; pMember; nMembers++)
		{
			if((pn - namebuf) >= MAX_NAMEBUF - 4)
				break;
			ps = pMember->GetName();
			pa = pn;

			// skip leading white space
			while(ps && *ps)
			{
				while(ps && *ps && (*ps == _T(' ') || *ps == _T('\t') || *ps == _T('\r') || *ps == _T('\n')))
					ps++;
				while(
						ps && ((pn - namebuf) < (MAX_NAMEBUF - 4)) &&
						*ps && *ps != _T(' ') && *ps != _T('\t') &&
						*ps != _T('\r') && *ps != _T('\n')
				)
					*pn++ = *ps++;
				if(*ps)
					*pn++ = _T(' ');
			}
			pMember = pMember->GetNext(pMember);
			if(pMember)
			{
				*pn++ = _T(',');
				*pn++ = _T(' ');
			}
			*pn = _T('\0');

			if((pn - pa) > maxArgLen)
				maxArgLen = (pn - pa);
		}
		*pn++ = _T(')');
		*pn   = _T('\0');

		w = fontwidth * (pn - namebuf) + 8;
		h = m_cacheFontHeight + 8;
		if(w > 400)
		{
			multiline = true;
			if(nMembers > 16)
				nMembers = 16;
			w = fontwidth * maxArgLen + 8;
			if(w > 260)
				w = 260;
			h = fontheight * nMembers + 8;
		}
	}
	else
	{
		for(nMembers = 0, pMember = pSym->f_members; pMember; nMembers++)
				pMember = pMember->GetNext(pMember);

		if(nMembers > 16)
			nMembers = 16;

		if(nMembers == 0)
		{
			TCHAR tt[256];

			_sntprintf(tt, 256, _T("No members in type "_Pfs_""), pSym->f_name);
			m_editor->SetStatus(tt);
			return errOK;
		}
		w = 240;
		h = fontheight * nMembers + 8;
	}
	maxh = MAX_PROTO_HEIGHT;
	GetClientRect(hWndParent, &rc);
	if(rc.bottom - rc.top - uly < maxh)
		maxh = rc.bottom - rc.top - uly;
	if(maxh < 2 * fontheight)
		maxh = 2 * fontheight;
	if(h > maxh)
		h = maxh;

	// create a pop-up window for displaying info
	//
	m_hWndProto = CreateWindow(
								WC_TREEVIEW,
								_T("Prototype"),
									WS_CHILD | WS_BORDER |
									(type != efFunction ?
										(TVS_SHOWSELALWAYS | WS_VSCROLL) : 0),
								ulx, uly, w, h, 
								hWndParent,
								NULL,
								g_hInstance,
								NULL
							  );

	if(! m_hWndProto) return errFAILURE;

	#if 0
	// set bkg and frg colors of window
	//
	if(type == efFunction)
		TreeView_SetBkColor(m_hWndProto, RGB(255,240,255));
	else
		TreeView_SetBkColor(m_hWndProto, RGB(250,250,255));
	TreeView_SetTextColor(m_hWndProto, m_view_colors[kwPlain]);
	#endif

	if(type == efFunction)
	{
		if(multiline)
		{
			for(pMember = pSym->f_members; pMember;)
			{
				pn = namebuf;
				ps = pMember->GetName();
				// skip leading white space
				while(ps && *ps)
				{
					while(ps && *ps && (*ps == _T(' ') || *ps == _T('\t') || *ps == _T('\r') || *ps == _T('\n')))
						ps++;
					while(
							ps && ((pn - namebuf) < (MAX_NAMEBUF - 4)) &&
							*ps && *ps != _T(' ') && *ps != _T('\t') &&
							*ps != _T('\r') && *ps != _T('\n')
					)
						*pn++ = *ps++;
					if(*ps)
						*pn++ = _T(' ');
				}
				pMember = pMember->GetNext(pMember);
				if(pMember)
				{
					*pn++ = _T(',');
					*pn++ = _T(' ');
				}
				*pn = _T('\0');

				tvItem.hParent		= NULL;
				tvItem.hInsertAfter = TVI_LAST;

				tvItem.item.mask	= TVIF_TEXT;
				tvItem.item.pszText	= namebuf;
				
				TreeView_InsertItem(m_hWndProto, &tvItem);
			}
		}
		else
		{
			tvItem.hParent		= NULL;
			tvItem.hInsertAfter = TVI_LAST;

			tvItem.item.mask	= TVIF_TEXT;
			tvItem.item.pszText	= namebuf;

			hItem = TreeView_InsertItem(m_hWndProto, &tvItem);
			if(! hFirstItem) hFirstItem = hItem;
		}
	}
	else
	{
		LPCTSTR membername;
		bool    ismemberFunc;

		TreeView_SetImageList(m_hWndProto, m_protimgs, TVSIL_NORMAL);

		pMember = pSym->f_members;

		while(pMember)
		{
			tvItem.hParent		= NULL;
			tvItem.hInsertAfter = TVI_SORT;
			#if 0 // this code sorts in case windows sort isn't right
 			tvItem.hParent		= NULL;
			tvItem.hInsertAfter = TVI_LAST;

			hItem = hPrevItem   = NULL;

			if(hFirstItem)
			{
				for(hItem = hFirstItem, hPrevItem = NULL; hItem;)
				{
					tvi.mask		= TVIF_TEXT;
					tvi.pszText		= namebuf;
					tvi.cchTextMax	= MAX_NAMEBUF;
					tvi.hItem		= hItem;

					if(TreeView_GetItem(m_hWndProto, &tvi))
					{
						LPCTSTR lpa = tvi.pszText;
						LPCTSTR lpb = pMember->GetName();

						if(*lpa == _T('~')) lpa++;
						if(*lpb == _T('~')) lpb++;

						int cmp = _tcsicmp(lpa, lpb);

						if(cmp > 0) 
						{
							_tprintf(_T(""_Pfs_" is > than "_Pfs_"\n"), lpa, lpb);
							if(hPrevItem == NULL)
							{
								tvItem.hInsertAfter = TVI_FIRST;
							}
							else
							{
								tvItem.hInsertAfter = hPrevItem;
							}
							break;
						}
						else
						{
							_tprintf(_T(""_Pfs_" is < than "_Pfs_"\n"), lpa, lpb);
						}
						hPrevItem = hItem;
						hItem = TreeView_GetNextSibling(m_hWndProto, hItem);
					}
					else
					{
						break;
					}
				}
			}
			#endif

			tvItem.item.mask	= TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			tvItem.item.pszText	= pMember->GetName();
			if(pMember->GetValue() && pMember->GetValue()->f_name)
				membername = pMember->GetValue()->f_name;
			else
				membername = NULL;
			if(membername)
				ismemberFunc = _tcsstr(membername, _T("::")) != NULL;
			else
				ismemberFunc = false;
			tvItem.item.iImage	= ismemberFunc ? 0 : 1;
			tvItem.item.iSelectedImage = tvItem.item.iImage;

			hItem = TreeView_InsertItem(m_hWndProto, &tvItem);
			
	#if 0
			if(! hFirstItem || (hItem && tvItem.hInsertAfter == TVI_FIRST))
				hFirstItem = hItem;
	#endif
			
			pMember = pMember->GetNext(pMember);
		}
	}
	hFirstItem = TreeView_GetRoot(m_hWndProto);
	if(hFirstItem)
		TreeView_SelectItem(m_hWndProto, hFirstItem);
	ShowWindow(m_hWndProto, SW_SHOW);
	if(type != efFunction)
		::SetFocus(m_hWndProto);
	return errOK;
}
