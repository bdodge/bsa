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
BappPane::BappPane(LPCTSTR pName, PBAFPANEL pPanel)
	:
	BappComponent(pPanel),
	m_name(NULL),
	m_active(false),
	m_next(NULL),
	m_prev(NULL)
{
	SetName(pName);

	m_tabrc.left = m_tabrc.right = m_tabrc.top = m_tabrc.bottom = 0;

	if(pPanel)
	{
		pPanel->AddPane(this);
	}
}

//***********************************************************************
BappPane::~BappPane()
{
	if(m_parent)
		m_parent->RemovePane(this);
	if(m_name) delete [] m_name;
}

//***********************************************************************
void BappPane::SetName(LPCTSTR pName)
{
	int len;

	if(! pName) pName = _T("temp");
	len = _tcslen(pName) + 2;
	if(m_name) delete [] m_name;
	m_name = new TCHAR [ len ];
	_tcscpy(m_name, pName);
	if(GetParentPanel())
		InvalidateRect(GetParentPanel()->GetWindow(), NULL, FALSE);
}

//***********************************************************************
LRESULT BappPane::OnMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return BappComponent::OnMessage(hWnd, msg, wParam, lParam);
}


//***********************************************************************
void BappPane::Activate()
{
	m_active = true;

	if(GetParentPanel())
	{
		FitToPanel();
	}
	if(GetWindow())
	{
		ShowWindow(GetWindow(), SW_SHOW);
	}
	InvalidateRect(GetWindow(), NULL, TRUE);
}

//***********************************************************************
void BappPane::Deactivate()
{
	m_active = false;
	if(GetWindow()) ShowWindow(GetWindow(), SW_HIDE);
}

//***********************************************************************
void BappPane::FitToPanel()
{
	if(m_active && GetParentPanel())
	{
		RECT rc;

		GetParentPanel()->GetPanelClientRect(&rc);
		MoveWindow(GetWindow(), rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, TRUE);
	}
}

