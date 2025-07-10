//--------------------------------------------------------------------
//
// File: treeview.c
// Desc: TreeView controls for winapi
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------


#ifndef Windows
#include "winapix.h"

extern HWND		_zg_dialogWnd[];
extern int		_zg_dialogTop;

extern HFONT	_zg_treeFont;

//-----------------------------------------------------------------------------

typedef struct tagTreeViewItem
{
	int		state;
	LPTSTR	lpText;
	LPARAM	cookie;
	int		img, selimg;
	struct	tagTreeViewItem* parent;
	struct	tagTreeViewItem* sibling;
	struct	tagTreeViewItem* child;
}
_TREEITEM, *_LPTREEITEM;

#define BSA_TV_MAX_ISL	32

//-----------------------------------------------------------------------------

typedef struct tagTreeView
{
	HWND		hwnd;
	int			indent;
	int			yi;
	int			mousex;
	int			mousey;
	DWORD		style;
	DWORD		exstyle;

	RECT		cliRect;

	HIMAGELIST	himlNormal;
	HIMAGELIST  himlState;

	COLORREF	crTextFrg;
	COLORREF	crTextBkg;

	TCHAR		sbuf[BSA_TV_MAX_ISL + 2];
	int			slen;

	_LPTREEITEM	root;
	_LPTREEITEM	cur;
	_LPTREEITEM	firstvis;
	_LPTREEITEM	lastvis;

	int			nVis;
	int			nOffset;
	int			nLeftOff;
	int			nTotal;
}
__tv, *__ptv;

typedef enum { tvi_any, tvi_vis, tvi_force } __tvi_vis;

//**************************************************************************
_LPTREEITEM __next_tvi(_LPTREEITEM pi, __tvi_vis vis)
{
	if(! pi) return NULL;

	if(pi->child)
	{
		if(vis == tvi_any || (pi->state & TVIS_EXPANDED))
		{
			return pi->child;
		}
		if(vis == tvi_force)
		{
			pi->state |= TVIS_EXPANDED;
			return pi->child;
		}
	}
	if(pi->sibling)
	{
		return pi->sibling;
	}
	while((pi = pi->parent) != NULL)
	{
		if(pi->sibling)
			return pi->sibling;
	}
	return NULL;
}

//**************************************************************************
_LPTREEITEM __prev_tvi(_LPTREEITEM pi, __ptv pTree)
{
	_LPTREEITEM prev, next;

	if(! pi || ! pTree) return NULL;
	if(pi == pTree->root) return NULL;

	if(! pi->parent)
	{
		prev = pTree->root;
	}
	else
	{
		if(pi->parent->child == pi)
			return pi->parent;

		prev = pi->parent;
	}
	if(! prev) return NULL;

	for(next = __next_tvi(prev, tvi_any); next && next != pi;)
	{
		prev = next;
		next = __next_tvi(next, tvi_any);
	}
	return prev;
}

//**************************************************************************
_LPTREEITEM __special_tvi(_LPTREEITEM ai, _LPTREEITEM ni, __ptv pTree)
{
	_LPTREEITEM pi;

	if((HTREEITEM)ai == TVI_ROOT)
		return pTree->root;
	else if((HTREEITEM)ai == TVI_FIRST)
		return pTree->root;
	else if((HTREEITEM)ai == TVI_LAST)
	{
		ai = pTree->root;
		do
		{
			pi = ai;
			ai = __next_tvi(ai, tvi_any);
		}
		while(ai);
		return pi;
	}
	else if((HTREEITEM)ai == TVI_SORT)
	{
		if(! ni || ! ni->lpText)
			return __special_tvi((_LPTREEITEM)TVI_LAST, ni, pTree);
		ai = pTree->root;
		pi = NULL;
		while(ai)
		{
			if(_tcsicmp(ni->lpText, (ai->lpText ? ai->lpText : _T(""))) < 0)
				return pi;
			pi = ai;
			ai = __next_tvi(ai, tvi_vis);
		}
		return pi;
	}
	else
	{
		return ai;
	}
}

//**************************************************************************
BOOL __set_tvi(_LPTREEITEM ni, LPTVITEM pItem)
{
	if(pItem->mask & TVIF_STATE)
	{
		//int oldstate = ni->state;

		ni->state &= pItem->stateMask;
		ni->state |= pItem->state;
	}
	if(pItem->mask & TVIF_TEXT && pItem->pszText != NULL)
	{
		ni->lpText = (LPTSTR)malloc(sizeof(TCHAR)*(_tcslen(pItem->pszText) + 2));
		if(ni->lpText)
		{
			_tcscpy(ni->lpText, pItem->pszText);
		}
	}
	if(pItem->mask & TVIF_PARAM)
		ni->cookie = pItem->lParam;
	if(pItem->mask & TVIF_IMAGE)
		ni->img = pItem->iImage;
	if(pItem->mask & TVIF_SELECTEDIMAGE)
		ni->selimg = pItem->iSelectedImage;
	return TRUE;
}

//**************************************************************************
BOOL __get_tvi(_LPTREEITEM ni, LPTVITEM pItem)
{
	if(! ni || ! pItem)
		return FALSE;
	if(pItem->mask & TVIF_CHILDREN)
		pItem->cChildren = ni->child != NULL;
	if(pItem->mask & TVIF_PARAM)
		pItem->lParam = ni->cookie;
	if(pItem->mask & TVIF_STATE)
		pItem->state = ni->state;
	if(pItem->mask & TVIF_TEXT && pItem->pszText && ni->lpText)
	{
		_tcsncpy(pItem->pszText, ni->lpText, pItem->cchTextMax);
		if(_tcslen(ni->lpText) >= pItem->cchTextMax)
			pItem->pszText[pItem->cchTextMax - 1] = _T('\0');
	}
	if(pItem->mask & TVIF_IMAGE)
		pItem->iImage = ni->img;
	if(pItem->mask & TVIF_SELECTEDIMAGE)
		pItem->iSelectedImage = ni->selimg;
	return TRUE;
}

//**************************************************************************
BOOL __select_tvi(_LPTREEITEM pi, __ptv pTree, int why)
{
	NMTREEVIEW nm;

	if(! pTree)	 		 return FALSE;
	if(pi == pTree->cur) return TRUE;

	// send notifies
	//
	nm.hdr.hwndFrom = pTree->hwnd;
	nm.hdr.idFrom   = GetWindowLong(pTree->hwnd, GWL_ID);
	nm.hdr.code		= TVN_SELCHANGING;

	nm.action		= why;
	if(pTree->cur)
	{
		nm.itemOld.hItem 	= (HTREEITEM)pTree->cur;
		nm.itemOld.lParam 	= pTree->cur->cookie;
		nm.itemOld.mask		= TVIF_HANDLE | TVIF_PARAM;
	}
	else
	{
		nm.itemOld.hItem	= NULL;
		nm.itemOld.lParam	= 0;
		nm.itemOld.mask		= 0;
	}
	if(pi)
	{
		nm.itemNew.hItem 	= (HTREEITEM)pi;
		nm.itemNew.lParam 	= pi->cookie;
		nm.itemNew.mask		= TVIF_HANDLE | TVIF_PARAM;
	}
	else
	{
		nm.itemNew.hItem	= NULL;
		nm.itemNew.lParam	= 0;
		nm.itemNew.mask		= 0;
	}
	pTree->cur = pi;
	SendMessage(GetParent(pTree->hwnd), WM_NOTIFY, 0, (LRESULT)&nm);
	nm.hdr.code = TVN_SELCHANGED;
	SendMessage(GetParent(pTree->hwnd), WM_NOTIFY, 0, (LRESULT)&nm);
	return TRUE;
}

//**************************************************************************
_LPTREEITEM __new_tvi(__ptv pTree, LPTVITEM pItem, HTREEITEM pAfter, HTREEITEM pParent)
{
	_LPTREEITEM ni, ai;

	if(! pTree || ! pItem)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return NULL;
	}
	ni = (_LPTREEITEM)malloc(sizeof(_TREEITEM));
	if(! ni) return NULL;

	ni->state   = TVIS_EXPANDED;
	ni->lpText  = NULL;
	ni->img		= 0;
	ni->selimg	= 0;

	__set_tvi(ni, pItem);

	ni->parent 	= NULL;
	ni->sibling	= NULL;
	ni->child 	= NULL;

	if(pAfter == TVI_ROOT)
	{
		pParent = TVI_ROOT;
		pAfter  = NULL;
	}
	else if(pAfter == TVI_FIRST)
	{
		pParent = TVI_FIRST;
		pAfter  = NULL;
	}
	if(pParent == TVI_ROOT)
	{
		ni->child = pTree->root;
		pTree->root = ni;
	}
	else if(pParent == TVI_FIRST)
	{
		ni->sibling = pTree->root;
		pTree->root = ni;
	}
	else
	{
		ni->parent = (_LPTREEITEM)pParent;
		ai = __special_tvi((_LPTREEITEM)pAfter, ni, pTree);

		if(ni->parent)
		{
			if(ai && ai->parent == ni->parent)
			{
				ni->sibling = ai->sibling;
				ai->sibling = ni;
				//_tprintf(_T("insert %ls as sib of %ls\n"), ni->lpText, ai->lpText);
			}
			else
			{
				if(ni->parent->child)
					ni->sibling = ni->parent->child;
				ni->parent->child = ni;
				//_tprintf(_T("insert %ls as kid of %ls\n"), ni->lpText, ni->parent->lpText);
			}
		}
		else if(ai)
		{
			ni->sibling = ai->sibling;
			ai->sibling = ni;
			//_tprintf(_T("insert %ls after %ls\n"), ni->lpText, ai->lpText);
		}
		else
		{
			ni->sibling = pTree->root;
			pTree->root = ni;
			//_tprintf(_T("insert %ls as root\n"), ni->lpText);
		}
	}
	if(! pTree->root)
	{
		pTree->root = ni;
	}
	pTree->cur = ni;
	pTree->nTotal++;
	return ni;
}


//**************************************************************************
BOOL __del_tvi(_LPTREEITEM pItem, __ptv pTree, int recurse)
{
	NMTREEVIEW  nm;
	_LPTREEITEM pi;

	if((HTREEITEM)pItem == TVI_ROOT)
	{
		recurse |= 1;
	}
	pItem = __special_tvi(pItem, NULL, pTree);

	if(! pItem || ! pTree) return FALSE;

	// first delete any kids of this item
	//
	if(pItem->child)
		__del_tvi(pItem->child, pTree, 1);
	pItem->child = NULL;

	// next, delete all the siblings of this item
	// if recursing down only though
	//
	if(recurse && pItem->sibling)
	{
		__del_tvi(pItem->sibling, pTree, 1);
		pItem->sibling = NULL;
	}
	// send notify about the deletion
	//
	nm.hdr.hwndFrom = pTree->hwnd;
	nm.hdr.idFrom   = GetWindowLong(pTree->hwnd, GWL_ID);
	nm.hdr.code		= TVN_DELETEITEM;

	nm.action			= 0;
	nm.itemNew.hItem 	= (HTREEITEM)pItem;
	nm.itemNew.lParam 	= pItem->cookie;
	nm.itemNew.mask		= TVIF_HANDLE | TVIF_PARAM;

	SendMessage(GetParent(pTree->hwnd), WM_NOTIFY, 0, (LRESULT)&nm);

	// find item in tree, and pointer to prev item
	//
	if(pItem == pTree->root)
	{
		if(recurse)
		{
			pTree->root = NULL;
			pTree->cur = NULL;
			pTree->nTotal = pTree->nVis = pTree->nOffset = 0;
			pTree->firstvis = NULL;
		}
		else
		{
			pTree->root = pItem->sibling;
			if(pItem == pTree->cur)
				pTree->cur = pTree->root;
			if(pItem == pTree->firstvis)
				pTree->firstvis = pTree->root;
			if(pItem == pTree->lastvis)
				pTree->lastvis = NULL;
		}
	}
	else
	{
		pi = __prev_tvi(pItem, pTree);

		if(pItem == pTree->cur)
			pTree->cur = pi;
		if(pItem == pTree->firstvis)
			pTree->firstvis = pi;
		if(pItem == pTree->lastvis)
			pTree->lastvis = NULL;

		if(pi)
		{
			if(pi->sibling == pItem)
				pi->sibling = recurse ? NULL : pItem->sibling;
			else if(pi->child == pItem)
				pi->child = NULL;
		}
	}
	// free text allocated and item itself
	//
	if(pItem->lpText)
		free(pItem->lpText);
	free(pItem);
	if(pTree->nTotal > 0)
		pTree->nTotal--;
	return FALSE;
}

//**************************************************************************
__ptv __new_tv(HWND hwndOwner, DWORD style, DWORD exstyle)
{
	__ptv pTree;

	pTree = (__ptv)malloc(sizeof(__tv));
	if(! pTree) return NULL;

	pTree->hwnd		= hwndOwner;
	pTree->indent	= 20;
	pTree->yi		= 0;
	pTree->mousex	= 0;
	pTree->mousey	= 0;
	pTree->style	= style;
	pTree->exstyle	= exstyle;

	pTree->himlNormal	= NULL;
	pTree->himlState	= NULL;

	pTree->crTextFrg   = GetSysColor(COLOR_BTNTEXT);
	pTree->crTextBkg   = GetSysColor(COLOR_WINDOW);

	pTree->slen		= 0;

	pTree->root		= NULL;
	pTree->cur		= NULL;

	pTree->firstvis	= NULL;
	pTree->lastvis	= NULL;
	pTree->nVis		= 0;
	pTree->nOffset	= 0;
	pTree->nTotal	= 0;
	pTree->nLeftOff = 0;
	return pTree;
}

//**************************************************************************
void __del_tv(__ptv ptv)
{
	if(ptv->root)
		__del_tvi(ptv->root, ptv, 1);
	free(ptv);
}


#define tiLOFFSET	6
#define tiCONNECT	1
#define tiBOXH		8
#define tiTOFFSET	(tiLOFFSET + tiCONNECT + tiBOXH)


//**************************************************************************
void __treescroll(__ptv pTree, int delta)
{
	_LPTREEITEM pItem;
	int 		noff, off;
	BOOL		sawCur;

	noff = pTree->nOffset + delta;

	if(noff < 0) noff = 0;

	for(pItem = pTree->root, off = 0, sawCur = FALSE; pItem && off < noff; off++)
	{
		if(pItem == pTree->cur)
			sawCur = TRUE;
		pItem = __next_tvi(pItem, tvi_vis);
	}
	if(pItem)
	{
		if(sawCur)
			pTree->cur = NULL; // current was above new top, so unselect it
		else
		{
			pTree->firstvis = pItem;

			for(off = 0; pItem && ! sawCur && off < pTree->nVis; off++)
			{
				if(pItem == pTree->cur)
					sawCur = TRUE;
				pItem = __next_tvi(pItem, tvi_vis);
			}
			if(! sawCur)
				pTree->cur = NULL; // current below new visible area
		}
		InvalidateRect(pTree->hwnd, NULL, FALSE);
	}
	else
	{
		noff = pTree->nTotal - pTree->nVis;
		if(noff < 0) noff = 0;
		for(pItem = pTree->root, off = 0; pItem && off < noff; off++)
			pItem = __next_tvi(pItem, tvi_vis);
		if(pItem)
		{
			pTree->firstvis = pItem;
			InvalidateRect(pTree->hwnd, NULL, FALSE);
		}
	}
}

//**************************************************************************
void __w_tv_is(__ptv pTree)
{
	_LPTREEITEM pItem;

	if(! pTree) return;

	// find closest match in sorted tree to chars typed so far

	for(pItem = pTree->root; pItem;)
	{
		LPCTSTR lpa = pItem->lpText;
		LPCTSTR lpb = pTree->sbuf;
		TCHAR   a, b;
		int i, j, cmp;

		j = _tcslen(lpa);
		if(j >= pTree->slen)
		{
			for(i = 0, cmp = 0; i < j && i < pTree->slen && cmp == 0 ; i++)
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
		//_tprintf(_T("zoom %ls to %ls = %d\n"), lpa, lpb, cmp);
		if(cmp == 0)
		{
			// zoom buffer matches this item
			__select_tvi(pItem, pTree, TVC_BYKEYBOARD);
			return;
		}
		else if(cmp < 0)
		{
			// zoom buffer < current item
			pItem = __next_tvi(pItem, tvi_any);
		}
		else
		{
			// zoom buffer already > current, so can't exist!
			__select_tvi(NULL, pTree, TVC_BYKEYBOARD);
			return;
		}
	}
}

//**************************************************************************
void __w_drawTree(HDC hdc, HWND hWnd, LPRECT lprcUpdate, __ptv pTree, int type)
{
	_LPTREEITEM pItem;
	_LPTREEITEM pMom;
	RECT		rcc;
	RECT		rcl;
	TEXTMETRIC	tm;
	LPCTSTR		lpText;
	int			nText;
	int			x, y, yi, icony, sicony, xoff, yoff;
	int			items;
	int			indent, lindent, iindent, sindent, tindent, lmarg;
	int			llen, connectHeight = tiCONNECT;
	BOOL		visibleYet, curVisible;
	static BOOL recursing = 0;
	HPEN		hPenLines;
	HPEN		hPenBox;
	HPEN		hPenPlus;
	HPEN		hOldPen = NULL;
	HBRUSH		hbrBkg = NULL;
	HFONT		hOldFont = NULL;
	HFONT		hFont, hCurFont = NULL;
	int			iImage;

	if(! hdc || ! hWnd) return;

	GetClientRect(hWnd, &rcc);
	if(lprcUpdate == NULL)
		lprcUpdate = &rcc;

	if(pTree->exstyle & WS_EX_CLIENTEDGE)
	{
		rcc.top += 2;
		rcc.left += 2;
		rcc.right -= 4;
		rcc.bottom -= 4;
	}
	else if(pTree->style & WS_BORDER)
	{
		rcc.top += 1;
		rcc.left += 1;
		rcc.right -= 2;
		rcc.bottom -= 2;
	}
	if(hdc && hWnd)
	{
		hCurFont = ((LPZWND)hWnd)->font;
		hOldFont = (HFONT)SelectObject(hdc, hCurFont);
	}
	if(pTree->yi <= 0)
	{
		GetTextMetrics(hdc, &tm);
		pTree->yi = tm.tmHeight;
	}
	yi = pTree->yi;

	if(pTree->himlNormal)
		ImageList_GetIconSize(pTree->himlNormal, &iindent, &icony);
	else
		iindent = icony = 0;
	if(pTree->himlState)
		ImageList_GetIconSize(pTree->himlState, &sindent, &sicony);
	else
		sindent = sicony = 0;
	icony = max(icony, sicony);

	if(icony > yi)
	{
		yoff = (icony - yi) / 2;
		yi = icony;
	}
	else
	{
		yoff = 0; // [TODO] use font height vs. yi
	}
	if(iindent > 0) iindent += 4;

	if(pTree->style & TVS_HASLINES)
	{
		tindent = tiTOFFSET;
		if(! sindent && ! iindent)
			yi += 2 * connectHeight;
		lmarg = 4;
	}
	else
	{
		tindent = 0;
		lmarg = 2;
	}
	if(type == dtDraw)
	{
		if(pTree->style & TVS_HASLINES)
		{
			hPenLines = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_GRAYTEXT));
			hPenBox   = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DDKSHADOW));
			hPenPlus  = CreatePen(PS_SOLID, 0, RGB(0,0,0));
		}
		hbrBkg	  = CreateSolidBrush(pTree->crTextBkg);
	}
	if(! pTree->firstvis) pTree->firstvis = pTree->root;
	pTree->nVis = 0;

	for(
			y = rcc.top, pItem = pTree->root, indent = 0, items = 0,
			visibleYet = curVisible = FALSE;

			(y + yi) <= rcc.bottom;

			items++
	)
	{
		if(pItem)
		{
			// skip items above first vis
			if(! visibleYet)
			{
				if(pItem == pTree->firstvis)
				{
					pTree->nOffset = items;
					visibleYet = TRUE;
				}
			}
			if(visibleYet)
			{
				pTree->lastvis = pItem;
				if(y + yi <= rcc.bottom)
					pTree->nVis++;

				x = rcc.left + tindent + iindent + sindent;

				if(! pItem->lpText)
				{
					// int i = nText;
				}
				lpText = pItem->lpText;
				if(! lpText) lpText = _T("");
				nText  = _tcslen(lpText);

				lindent = lmarg + indent * pTree->indent;

				if(type == dtSetPosToMouse && y <= pTree->mousey &&
						(y + yi) > pTree->mousey)
				{
					pTree->cur = pItem;

					if(pTree->mousex >= lindent - tiBOXH / 2 &&
							pTree->mousex <= lindent + tiBOXH / 2)
					{
						TreeView_Expand(hWnd, (HTREEITEM)pItem, TVE_TOGGLE);
					}
				}

				if(type == dtDraw && y >= lprcUpdate->top && y < lprcUpdate->bottom)
				{
					SIZE sizeText;
					RECT rcx;

					rcl.top		= y;
					rcl.bottom	= y + yi;
					if(pTree->style & TVS_HASLINES)
					{
						rcl.top -= connectHeight;
						rcl.bottom += connectHeight;
					}
					rcl.left	= rcc.left;
					rcl.right	= x + lindent + tindent;

					// fill left edge to text start
					FillRect(hdc, &rcl, hbrBkg);

					if(pItem->state & TVIS_BOLD)
					{
						extern HFONT _zg_treeFontBold;

						hFont = _zg_treeFontBold;
					}
					else
					{
						hFont = ((LPZWND)hWnd)->font;
					}
					if(hFont != hCurFont)
					{
						SelectObject(hdc, hFont);
						hCurFont = hFont;
					}
					if(pItem != pTree->cur)
					{
						SetTextColor(hdc, pTree->crTextFrg);
						SetBkColor(hdc, pTree->crTextBkg);
					}
					else
					{
						BOOL	 haveFocus = (GetFocus() == hWnd);
						COLORREF selBkg, selFrg;

						if(visibleYet)
							curVisible = TRUE;

						if(haveFocus)
						{
							selBkg = RGB(0,0,127);
							selFrg = RGB(255,255,255);
						}
						else
						{
							selBkg = RGB(240,240,255);
							selFrg = pTree->crTextFrg;
						}
						SetTextColor(hdc, selFrg);
						SetBkColor(hdc, selBkg);
					}
					//---------- TEXT HERE --------------------
					TextOut(hdc, x + lindent, y + yoff, lpText, nText);

					GetTextExtentPoint32(hdc, (LPTSTR)lpText, nText, &sizeText);

					// fill to EOL of text
					//
					rcx.left 	= x + lindent + sizeText.cx;
					rcx.top  	= rcl.top;
					rcx.right 	= rcc.right;
					rcx.bottom 	= rcl.bottom;

					FillRect(hdc, &rcx, hbrBkg);

					// bkg fill above and below text if height different
					// than raw text height
					//
					if(pTree->style & TVS_HASLINES)
					{
						rcx.left 	= x + lindent;
						rcx.top  	= y + yoff - connectHeight;
						rcx.right 	= x + lindent + sizeText.cx;
						rcx.bottom 	= y + yoff;

						FillRect(hdc, &rcx, hbrBkg);

						rcx.left 	= x + lindent;
						rcx.top  	= y + yoff + sizeText.cy;
						rcx.right 	= x + lindent + sizeText.cx;
						rcx.bottom 	= y + yoff + yi;

						FillRect(hdc, &rcx, hbrBkg);
					}
					/*
					rcl.left	= x + lindent + sizeText.cx;
					rcl.right	= rcc.right;

					FillRect(hdc, &rcl, hbrBkg);
					*/
#ifdef __W_SHOWLINEBOX_FOR_TREE_CUR
					if(pItem == pTree->cur && GetFocus() != hWnd)
					{
						HPEN hPen = CreatePen(PS_SOLID, 0, RGB(0,0,127));
						HPEN hPP;

						hPP = SelectObject(hdc, hPen);

						MoveToEx(hdc, x + lindent - 2, y - 1, NULL);
						LineTo(hdc, x + lindent + sizeText.cx + 2, y - 1);
						LineTo(hdc, x + lindent + sizeText.cx + 2, y + yi + 1);
						LineTo(hdc, x + lindent - 2, y + yi + 1);
						LineTo(hdc, x + lindent - 2, y - 1);
						SelectObject(hdc, hPP);
						DeleteObject(hPen);
					}
#endif
					if(pTree->style & TVS_HASLINES)
					{
						// draw connecting lines
						//
						llen = yi / 2;

						hOldPen = SelectObject(hdc, hPenBox);

						if(pItem->child)
						{
							/* box to left of item */

							llen -= tiBOXH / 2;
							MoveToEx(hdc, lindent - tiBOXH / 2, y + yi / 2 - tiBOXH / 2, NULL);
							LineTo(hdc, lindent - tiBOXH / 2, y + yi / 2 + tiBOXH / 2);
							LineTo(hdc, lindent + tiBOXH / 2, y + yi / 2 + tiBOXH / 2);
							LineTo(hdc, lindent + tiBOXH / 2, y + yi / 2 - tiBOXH / 2);
							LineTo(hdc, lindent - tiBOXH / 2, y + yi / 2 - tiBOXH / 2);

							SelectObject(hdc, hPenPlus);

							if(pItem->state & TVIS_EXPANDED)
							{
								/* minus sign inside box */
								MoveToEx(hdc, lindent - tiBOXH / 2 + 2, y + yi / 2, NULL);
								LineTo(hdc, lindent + tiBOXH / 2 - 1, y + yi / 2);
							}
							else
							{
								/* plus sign inside box */
								MoveToEx(hdc, lindent, y + yi / 2 - tiBOXH / 2 + 2, NULL);
								LineTo(hdc, lindent, y + yi / 2 + tiBOXH / 2 - 1);
								MoveToEx(hdc, lindent - tiBOXH / 2 + 2, y + yi / 2, NULL);
								LineTo(hdc, lindent + tiBOXH / 2 - 1, y + yi / 2);
							}
						}
						SelectObject(hdc, hPenLines);

						/* line up to parent */
						if(pItem->parent && pItem->parent->child == pItem) {
							MoveToEx(hdc, lindent, y + llen - 1, NULL);
							LineTo(hdc, lindent, y - connectHeight);
						}
						/* vert line to prev item */
						if(__prev_tvi(pItem, pTree))
						{
							MoveToEx(hdc, lindent, y - connectHeight, NULL);
							LineTo(hdc, lindent, y + llen);
						}
						/* vert line to next item */
						if(pItem->sibling) {
							MoveToEx(hdc, lindent, y + yi + connectHeight - 1, NULL);
							LineTo(hdc, lindent, y + yi - llen);
						}
						/* line to child */
						if(pItem->child && (pItem->child->state & TVIS_EXPANDED)) {
							MoveToEx(hdc, lindent + pTree->indent, y + yi, NULL);
							LineTo(hdc, lindent + pTree->indent, y + yi + connectHeight - 1);
						}
						{
							int lw = tindent - 2;
							if(lw < 2) lw = 2;

							/* horiz line from vert line to box, and images
							*/
							MoveToEx(hdc, (pItem->child) ? lindent + tiBOXH/2 + 1 : lindent, y + yi / 2, NULL);
							LineTo(hdc,  lindent + lw, y + yi / 2);
						}

						SelectObject(hdc, hPenLines);

						/*  draw other structure to left of item (ancestral lines)
						 */
						for(
								pMom = pItem->parent,
								xoff = lindent - pTree->indent;

								pMom;

								pMom = pMom->parent,
								xoff -= pTree->indent
						)
						{
							if(pMom->sibling)
							{
								MoveToEx(hdc, xoff, y - connectHeight, NULL);
								LineTo(hdc, xoff, y + yi + connectHeight);
							}
						}
						SelectObject(hdc, hOldPen);
					}
				}
				// Images
				if(pTree->himlState)
				{
					; //[TODO]
				}
				if(type == dtDraw && pTree->himlNormal)
				{
					if(pItem == pTree->cur)
						iImage = pItem->selimg;
					else
						iImage = pItem->img;
					ImageList_Draw(pTree->himlNormal, iImage, hdc, lindent + x - iindent , y, 0);
				}
				y += yi;
			}
			// move to next item
			//
			if(pItem->state & TVIS_EXPANDED && pItem->child)
			{
				indent++;
				pItem = pItem->child;
			}
			else if(pItem->sibling)
			{
				pItem = pItem->sibling;
			}
			else if(pItem->parent)
			{
				while((pItem = pItem->parent) != NULL)
				{
					indent--;
					if(pItem->sibling)
					{
						pItem = pItem->sibling;
						break;
					}
				}
			}
			else
			{
				pItem = NULL;
			}
		}
		else
		{
			break;
		}
	}
	if(! curVisible && pTree->cur && type == dtDraw && ! recursing)
	{
		// current pos off the screen, so recenter
		int items = (rcc.bottom - rcc.top) / yi;

		recursing = TRUE;

		for(pItem = pTree->cur; items > 0 && pItem; items -= 2)
			pItem = __prev_tvi(pItem, pTree);
		if(! pItem) pItem = pTree->root;
		pTree->firstvis = pItem;
		__w_drawTree(hdc, hWnd, lprcUpdate, pTree, type);
		recursing = FALSE;
	}
	if(type == dtDraw)
	{
		// fill rect after last item if
		if(y >= lprcUpdate->top && y < lprcUpdate->bottom)
		{
			rcl.top    = max(y, lprcUpdate->top);
			rcl.left   = rcc.left;
			rcl.right  = rcc.right;
			rcl.bottom = rcc.bottom;
			FillRect(hdc, &rcl, hbrBkg);
		}
		if(pTree->style & TVS_HASLINES)
		{
			if(hOldPen)
				SelectObject(hdc, hOldPen);
			DeleteObject(hPenLines);
			DeleteObject(hPenBox);
			DeleteObject(hPenPlus);
		}
		if(pTree->exstyle & WS_EX_CLIENTEDGE)
		{
			rcc.top -= 2;
			rcc.left-= 2;
			rcc.bottom += 4;
			rcc.right += 4;
			__w_button_box(hdc, 1, &rcc);
		}
		else if(pTree->style & WS_BORDER)
		{
			hPenLines = GetStockObject(BLACK_PEN);
			hOldPen = (HPEN)SelectObject(hdc, hPenLines);
			MoveToEx(hdc, rcc.left, rcc.top, NULL);
			LineTo(hdc, rcc.right, rcc.top);
			LineTo(hdc, rcc.right, rcc.bottom);
			LineTo(hdc, rcc.left, rcc.bottom);
			LineTo(hdc, rcc.left, rcc.top);
			SelectObject(hdc, hOldPen);
		}
		DeleteObject(hbrBkg);
	}
	if(hdc && hOldFont)
	{
		SelectObject(hdc, hOldFont);
	}
}

//**************************************************************************
LRESULT WINAPI __wproc_TreeView(HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{
	PAINTSTRUCT ps;
	HDC         hdc;
	LPZWND		zWnd;
	DWORD		style;
	DWORD		exstyle;
	__ptv		pTree;
	LPTVINSERTSTRUCT pIns;
	_LPTREEITEM	pTreeItem;
	LPTVITEM	pItem;
	int			mpx, mpy;

	if(! (zWnd = Zwindow(hWnd))) return -1;
	style  	= GetWindowLong(hWnd, GWL_STYLE);
	exstyle	= GetWindowLong(hWnd, GWL_EXSTYLE);
	pTree	= (__ptv)GetWindowLong(hWnd, GWL_PRIVDATA);

	switch(message)
	{
	case WM_CREATE:

		pTree = __new_tv(hWnd, style, exstyle);
		if(pTree)
			SetWindowLong(hWnd, GWL_PRIVDATA, (LONG)pTree);
		else
			return -1;
		if(_zg_treeFont)
			zWnd->font = _zg_treeFont;
		pTree->cliRect.left = pTree->cliRect.top = 0;
		pTree->cliRect.right = pTree->cliRect.bottom = 0;
		break;

	case WM_MOVE:
	case WM_SIZE:
		GetClientRect(hWnd, &pTree->cliRect);
		break;

	case WM_DESTROY:

		__del_tv(pTree);
		break;

	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		pTree->style = style;
		pTree->exstyle = exstyle;
		__w_drawTree(hdc, hWnd, &ps.rcPaint, pTree, dtDraw);
		EndPaint(hWnd, &ps);
		if(style & WS_VSCROLL)
		{
			SCROLLINFO sinfo;

			sinfo.cbSize	= sizeof(SCROLLINFO);
			sinfo.fMask		= SIF_PAGE | SIF_POS | SIF_RANGE;
			sinfo.nMin		= 0;

			sinfo.nMax		= pTree->nTotal + 1;
			sinfo.nPage		= pTree->nVis;
			sinfo.nPos		= pTree->nOffset;

			SetScrollInfo(hWnd, SB_VERT, &sinfo, TRUE);
		}
		break;

#if 0 // use this to cause selection to change without sel notify
	case WM_LBUTTONDOWN:

		if(GetFocus() != hWnd)
			SetFocus(hWnd);
		pTree->mousex = LOWORD(lParam);
		pTree->mousey = HIWORD(lParam);
		hdc = GetDC(hWnd);
		__w_drawTree(hdc, hWnd, NULL, pTree, dtSetPosToMouse);
		ReleaseDC(hWnd, hdc);
		InvalidateRect(hWnd, &pTree->cliRect, FALSE);
		break;
#endif

	case WM_LBUTTONUP:

		break;

 	case WM_LBUTTONDBLCLK:
		{
			NMHDR nm;

			// send notifY
			//
			nm.hwndFrom = hWnd;
			nm.idFrom   = GetWindowLong(hWnd, GWL_ID);
			nm.code		= NM_DBLCLK;

			SendMessage(GetParent(hWnd), WM_NOTIFY, 0, (LRESULT)&nm);
		}
		break;

 	case WM_LBUTTONDOWN:

 		if(GetFocus() != hWnd)
			SetFocus(hWnd);
		pTreeItem = pTree->cur;
		pTree->mousex = LOWORD(lParam);
		pTree->mousey = HIWORD(lParam);
		hdc = GetDC(hWnd);
		__w_drawTree(hdc, hWnd, NULL, pTree, dtSetPosToMouse);
		ReleaseDC(hWnd, hdc);
		/*
		if(pTree->cur && pTree->cur->child)
			TreeView_Expand(hWnd, (HTREEITEM)pTree->cur, TVE_TOGGLE);
		*/
		if(pTreeItem != pTree->cur)
		{
			pTreeItem = pTree->cur;
			pTree->cur = NULL;
			__select_tvi(pTreeItem, pTree, TVC_BYMOUSE);
		}
		InvalidateRect(hWnd, &pTree->cliRect, FALSE);
		break;

	case WM_MOUSEMOVE:

		break;

	case WM_MOUSEWHEEL:

		if((int)(short)HIWORD(wParam) > 0)
			pTreeItem = __prev_tvi(pTree->cur, pTree);
		else
			pTreeItem = __next_tvi(pTree->cur, tvi_vis);
		if(! pTreeItem)
			pTreeItem = pTree->cur;
		if(__select_tvi(pTreeItem, pTree, TVC_BYMOUSE))
			InvalidateRect(hWnd, &pTree->cliRect, FALSE);
		break;

	case WM_SETFOCUS:

		InvalidateRect(hWnd, &pTree->cliRect, FALSE);
		break;

	case WM_KILLFOCUS:

		InvalidateRect(hWnd, &pTree->cliRect, FALSE);
		break;

	case WM_KEYDOWN:

		switch(wParam)
		{
		case VK_UP:
			if(! (pTreeItem = __prev_tvi(pTree->cur, pTree)))
				pTreeItem = pTree->cur;
			if(__select_tvi(pTreeItem, pTree, TVC_BYKEYBOARD))
				InvalidateRect(hWnd, &pTree->cliRect, FALSE);
			break;

		case VK_DOWN:
			if(! (pTreeItem = __next_tvi(pTree->cur, tvi_vis)))
				pTreeItem = pTree->cur;
			if(__select_tvi(pTreeItem, pTree, TVC_BYKEYBOARD))
				InvalidateRect(hWnd, &pTree->cliRect, FALSE);
			break;

		case VK_LEFT:
			if(! (pTreeItem = pTree->cur->parent))
				pTreeItem = pTree->root;
			if(__select_tvi(pTreeItem, pTree, TVC_BYKEYBOARD))
				InvalidateRect(hWnd, &pTree->cliRect, FALSE);
			break;

		case VK_RIGHT:
			if(pTree->cur && pTree->cur->child)
			pTreeItem = __next_tvi(pTree->cur, tvi_force);
				pTreeItem = pTree->cur;
			if(__select_tvi(pTreeItem, pTree, TVC_BYKEYBOARD))
				InvalidateRect(hWnd, &pTree->cliRect, FALSE);
			break;

		case VK_NEXT:
			for(mpy = pTree->nVis; mpy > 0; mpy--)
			{
				if(! (pTreeItem = __next_tvi(pTree->cur, tvi_vis)))
				{
					pTreeItem = pTree->cur;
					break;
				}
			}
			if(__select_tvi(pTreeItem, pTree, TVC_BYKEYBOARD))
				InvalidateRect(hWnd, &pTree->cliRect, FALSE);
			break;

		case VK_PRIOR:
			for(mpy = pTree->nVis; mpy > 0; mpy--)
			{
				if(! (pTreeItem = __prev_tvi(pTree->cur, pTree)))
				{
					pTreeItem = pTree->cur;
					break;
				}
			}
			if(__select_tvi(pTreeItem, pTree, TVC_BYKEYBOARD))
				InvalidateRect(hWnd, &pTree->cliRect, FALSE);
			break;

		case VK_HOME:
			if(! (pTreeItem = __special_tvi((_LPTREEITEM)TVI_FIRST, NULL, pTree)))
				pTreeItem = pTree->root;
			if(__select_tvi(pTreeItem, pTree, TVC_BYKEYBOARD))
				InvalidateRect(hWnd, &pTree->cliRect, FALSE);
			break;

		case VK_END:
			if(! (pTreeItem = __special_tvi((_LPTREEITEM)TVI_LAST, NULL, pTree)))
				pTreeItem = pTree->cur;
			if(__select_tvi(pTreeItem, pTree, TVC_BYKEYBOARD))
				InvalidateRect(hWnd, &pTree->cliRect, FALSE);
			break;

		case VK_RETURN:

			if(pTree->cur)
			{
				pTreeItem = pTree->cur;
				pTree->cur = NULL;
				if(__select_tvi(pTreeItem, pTree, TVC_BYKEYBOARD))
					InvalidateRect(hWnd, &pTree->cliRect, FALSE);
			}
			break;

		case VK_TAB:

			// pass this up to the dialog parent if present
			//
			if(_zg_dialogTop >= 0 && _zg_dialogWnd[_zg_dialogTop])
			{
				SendMessage(_zg_dialogWnd[_zg_dialogTop], WM_KEYDOWN, wParam, lParam);
				break;
			}
			break;

		default:
			break;

		}
		{
			NMTVKEYDOWN nm;
			int         nr;

			// send notifY
			//
			nm.hdr.hwndFrom = hWnd;
			nm.hdr.idFrom   = GetWindowLong(hWnd, GWL_ID);
			nm.hdr.code		= TVN_KEYDOWN;

			nm.wVKey		= wParam;
			nm.flags		= 0;

			nr = SendMessage(GetParent(hWnd), WM_NOTIFY, 0, (LRESULT)&nm);
			if(nr)
			{
				// ignore next char in incremental search
				// [TODO]
			}
			else
			{
				if(wParam == VK_DELETE || wParam == VK_BACK)
				{
					if(pTree->slen > 0)
					{
						pTree->slen--;
						pTree->sbuf[pTree->slen] = _T('\0');
						__w_tv_is(pTree);
					}
				}
			}
		}
		break;

	case WM_CHAR:

		switch(wParam)
		{
		case 8:
		case 10:
		case 13:
			// handled in wm_keydown
			break;

		default:
			// incremental sort
			if(pTree->slen < BSA_TV_MAX_ISL)
			{
				pTree->sbuf[pTree->slen++] = (TCHAR)wParam;
				pTree->sbuf[pTree->slen]   = _T('\0');
				__w_tv_is(pTree);
			}
			break;
		}
		InvalidateRect(hWnd, &pTree->cliRect, TRUE);
		break;

	case TVM_INSERTITEM:

		pIns = (LPTVINSERTSTRUCT)lParam;
		if(! pIns) return 0;
		pItem = (LPTVITEM)&pIns->item;
		//_tprintf(_T("tvi %ls\n"), pItem->pszText);
		pTreeItem = __new_tvi(
							pTree,
							pItem,
							pIns->hInsertAfter,
							pIns->hParent
							);
		InvalidateRect(hWnd, &pTree->cliRect, FALSE);
		return (LRESULT)pTreeItem;

	case TVM_DELETEITEM:

		pTreeItem = (_LPTREEITEM)lParam;
		InvalidateRect(hWnd, &pTree->cliRect, FALSE);
		return (LRESULT)__del_tvi(pTreeItem, pTree, 0);

	case TVM_EXPAND:

		pTreeItem = (_LPTREEITEM)lParam;
		if(pTreeItem)
		{
			int state = pTreeItem->state;

			switch(wParam)
			{
			case TVE_COLLAPSE:
				pTreeItem->state &= ~ TVIS_EXPANDED;
				break;
			case TVE_EXPAND:
				pTreeItem->state |= TVIS_EXPANDED;
				break;
			case TVE_TOGGLE:
				pTreeItem->state ^= TVIS_EXPANDED;
				break;
			}
			InvalidateRect(hWnd, &pTree->cliRect, (pTreeItem->state != state) ? TRUE : FALSE);
		}
		return TRUE;

	case TVM_GETITEMRECT:
	case TVM_GETCOUNT:
		break;

	case TVM_GETINDENT:
		return (LRESULT)pTree->indent;

	case TVM_SETINDENT:
		return (LRESULT)(pTree->indent = wParam);

	case TVM_GETIMAGELIST:
		if(wParam == TVSIL_NORMAL)
			return (LRESULT)pTree->himlNormal;
		else
			return 0;
		break;

	case TVM_SETIMAGELIST:
		if(wParam == TVSIL_NORMAL)
			pTree->himlNormal = (HIMAGELIST)lParam;
		else
			pTree->himlState = (HIMAGELIST)lParam;
		break;

	case TVM_GETNEXTITEM:

		switch(wParam)
		{
		case TVGN_ROOT:
			return (LRESULT)pTree->root;
		case TVGN_NEXT:
			if(! (pTreeItem = (_LPTREEITEM)lParam))
				return (LRESULT)NULL;
			return (LRESULT)__next_tvi(pTreeItem, tvi_any);
		case TVGN_PREVIOUS:
			if(! (pTreeItem = (_LPTREEITEM)lParam))
				return (LRESULT)NULL;
			return (LRESULT)__prev_tvi(pTreeItem, pTree);
		case TVGN_PARENT:
			if(! (pTreeItem = (_LPTREEITEM)lParam))
				return (LRESULT)NULL;
			return (LRESULT)pTreeItem->parent;
		case TVGN_CHILD:
			if(! (pTreeItem = (_LPTREEITEM)lParam))
				return (LRESULT)NULL;
			return (LRESULT)pTreeItem->child;
		case TVGN_FIRSTVISIBLE:
			return (LRESULT)pTree->firstvis;
		case TVGN_NEXTVISIBLE:
			if(! (pTreeItem = (_LPTREEITEM)lParam))
				return (LRESULT)NULL;
			return (LRESULT)__next_tvi(pTreeItem, tvi_vis);
		case TVGN_PREVIOUSVISIBLE:
			if(! (pTreeItem = (_LPTREEITEM)lParam))
				return (LRESULT)NULL;
			return (LRESULT)__prev_tvi(pTreeItem, pTree);
		case TVGN_DROPHILITE:
			if(! (pTreeItem = (_LPTREEITEM)lParam))
				return (LRESULT)NULL;
			return (LRESULT)pTreeItem; // fix
		default:
		case TVGN_CARET:
			return (LRESULT)pTree->cur;
		case TVGN_LASTVISIBLE:
			return (LRESULT)pTree->lastvis; // fix
		}
		break;

	case TVM_SELECTITEM:
		pTreeItem = (_LPTREEITEM)lParam;
		InvalidateRect(hWnd, &pTree->cliRect, FALSE);
		return (LRESULT)__select_tvi(pTreeItem, pTree, TVC_UNKNOWN);

	case TVM_GETITEM:
		if(! (pItem = (LPTVITEM)lParam)) return FALSE;
		return __get_tvi((_LPTREEITEM)pItem->hItem, pItem);

	case TVM_SETITEM:
		if(! (pItem = (LPTVITEM)lParam)) return FALSE;
		InvalidateRect(hWnd, &pTree->cliRect, FALSE);
		return __set_tvi((_LPTREEITEM)pItem->hItem, pItem);

	case TVM_EDITLABEL:
	case TVM_GETEDITCONTROL:
	case TVM_GETVISIBLECOUNT:
	case TVM_CREATEDRAGIMAGE:
	case TVM_SORTCHILDREN:
	case TVM_ENSUREVISIBLE:
	case TVM_SORTCHILDRENCB:
	case TVM_ENDEDITLABELNOW:
	case TVM_SETTOOLTIPS:
	case TVM_GETTOOLTIPS:
	case TVM_SETITEMHEIGHT:
	case TVM_GETITEMHEIGHT:
		break;
	case TVM_SETBKCOLOR:
		pTree->crTextBkg = (COLORREF)lParam;
		InvalidateRect(hWnd, &pTree->cliRect, FALSE);
		break;
	case TVM_SETTEXTCOLOR:
		pTree->crTextFrg = (COLORREF)lParam;
		InvalidateRect(hWnd, &pTree->cliRect, FALSE);
		break;
	case TVM_GETBKCOLOR:
		return pTree->crTextBkg;
	case TVM_GETTEXTCOLOR:
		return pTree->crTextFrg;

	case WM_VSCROLL:

		switch(LOWORD(wParam))
		{
		case SB_BOTTOM:
			__treescroll(pTree, pTree->nTotal + 10);
			break;
		case SB_ENDSCROLL:
			break;
		case SB_LINEDOWN:
			__treescroll(pTree, 1);
			break;
		case SB_LINEUP:
			__treescroll(pTree, -1);
			break;
		case SB_PAGEDOWN:
			__treescroll(pTree, pTree->nVis);
			break;
		case SB_PAGEUP:
			__treescroll(pTree, -pTree->nVis);
			break;
		case SB_THUMBPOSITION:
			mpy = HIWORD(wParam);
			__treescroll(pTree, mpy - pTree->nOffset);
			break;
		case SB_THUMBTRACK:
			mpy = HIWORD(wParam);
			__treescroll(pTree, mpy - pTree->nOffset);
			break;
		case SB_TOP:
			__treescroll(pTree, -(pTree->nTotal + 10));
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
			break;
		case SB_LINELEFT:
			break;
		case SB_PAGERIGHT:
			break;
		case SB_PAGELEFT:
			break;
		case SB_THUMBPOSITION:
			mpx = HIWORD(wParam);
			break;
		case SB_THUMBTRACK:
			mpx = HIWORD(wParam);
			break;
		case SB_LEFT:
			break;
		}
		break;

	default:

		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

#endif
