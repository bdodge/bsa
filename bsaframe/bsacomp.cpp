//--------------------------------------------------------------------
//
// File: bsacomp.cpp
// Desc: application framework
// Auth: Brian Dodge
//
// (C)opyright 2003 - 2005 - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------
#include "framex.h"


HFONT BappComponent::m_hFont	 = NULL;
HFONT BappComponent::m_hBoldFont = NULL;


//***********************************************************************
BappComponent::BappComponent(PBAFPANEL pPanel)
	:
	m_parent(pPanel),
	m_hInstance(NULL),
	m_hwnd(NULL)
{
	if(pPanel)
		m_hInstance = pPanel->GetInstance();
}

//***********************************************************************
BappComponent::~BappComponent()
{
	if(m_hwnd)
	{
		DestroyWindow(m_hwnd);
		m_hwnd = NULL;
	}
}

//***********************************************************************
ERRCODE BappComponent::SetParentPanel(PBAFPANEL p)
{
	m_parent = p;
	if(p)
	{
		SetParent(m_hwnd, p->GetWindow());
	}
	return errOK;
};

//***********************************************************************
ERRCODE BappComponent::GetWindowPos(HWND hWnd, LPRECT prc)
{
	POINT pt;

	if(! prc || ! hWnd) return errBAD_PARAMETER;

	GetClientRect(hWnd, prc);

	pt.x = pt.y = 0;
	MapWindowPoints(hWnd, GetParent(hWnd), &pt, 1);

	prc->left   += pt.x;
	prc->top    += pt.y;
	prc->right  += pt.x;
	prc->bottom += pt.y;

	return errOK;
}

//***********************************************************************
ERRCODE BappComponent::OnResized(PBAFCOMP by)
{
	return errOK;
}

//***********************************************************************
ERRCODE BappComponent::OutlineBox(HDC hdc, LPRECT rc, bool in)
{
	HPEN		hpen;
	HPEN		hpenold;

	if(! rc || ! hdc)
	{
		return errBAD_PARAMETER;
	}

	rc->right--;
	rc->bottom--;

	// br outside
	hpen	= CreatePen(PS_SOLID, 0, GetSysColor(in ? COLOR_BTNHIGHLIGHT : COLOR_3DDKSHADOW));
	hpenold = (HPEN)SelectObject(hdc, hpen);
	MoveToEx(hdc, rc->left, rc->bottom, NULL);
	LineTo(hdc, rc->right, rc->bottom);
	LineTo(hdc, rc->right, rc->top - (in ? 0 : 1));
	SelectObject(hdc, hpenold);
	DeleteObject(hpen);

	// br inside
	hpen	= CreatePen(PS_SOLID, 0, GetSysColor(in ? COLOR_3DLIGHT : COLOR_BTNSHADOW));
	hpenold = (HPEN)SelectObject(hdc, hpen);
	MoveToEx(hdc, rc->left + 1, rc->bottom - 1, NULL);
	LineTo(hdc, rc->right - 1, rc->bottom - 1);
	LineTo(hdc, rc->right - 1, rc->top);
	SelectObject(hdc, hpenold);
	DeleteObject(hpen);

	// ul outside
	hpen	= CreatePen(PS_SOLID, 0, GetSysColor(in ? COLOR_BTNSHADOW : COLOR_BTNHIGHLIGHT));
	hpenold = (HPEN)SelectObject(hdc, hpen);
	MoveToEx(hdc, rc->right - (in ? 0 : 1), rc->top, NULL);
	LineTo(hdc, rc->left, rc->top);
	LineTo(hdc, rc->left, rc->bottom);
	SelectObject(hdc, hpenold);
	DeleteObject(hpen);

	// ul inside
	if(in)
	{
		hpen	= CreatePen(PS_SOLID, 0, GetSysColor(in ? COLOR_3DDKSHADOW : COLOR_3DLIGHT));
		hpenold = (HPEN)SelectObject(hdc, hpen);
		MoveToEx(hdc, rc->right - 1 - (in ? 0 : 1), rc->top + 1, NULL);
		LineTo(hdc, rc->left + 1, rc->top + 1);
		LineTo(hdc, rc->left + 1, rc->bottom - 1);
		SelectObject(hdc, hpenold);
		DeleteObject(hpen);
	}
	return errOK;
}
