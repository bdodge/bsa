//--------------------------------------------------------------------
//
// File: bhsizer.cpp
// Desc: application framework
// Auth: Brian Dodge
//
// (C)opyright 2003 - 2005 - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------
#include "framex.h"

//***********************************************************************
Bsizer::Bsizer(BAF_EDGE edge)
	:
	BappComponent(NULL),
	m_edge(edge)
{
}

//***********************************************************************
Bsizer::~Bsizer()
{
}

//***********************************************************************
ERRCODE Bsizer::Attach(PBAFPANEL pc)
{
	int x, y, w, h;

	if(! pc)				return errBAD_PARAMETER;
	if(! pc->GetWindow())	return errBAD_PARAMETER;

	m_parent	= pc;
	m_hInstance = pc->GetInstance();

	GetFit(x, y, w, h);

	if(m_hwnd)
	{
		MoveWindow(m_hwnd, x, y, w, h, TRUE);
	}
	else
	{
		// create window for resizer bar
		//
		m_hwnd = CreateWindow(
								(m_edge == frLeft || m_edge == frRight) ?
										_T("baf_vsizer") :
										_T("baf_hsizer"),
								(m_edge == frLeft || m_edge == frRight) ?
										_T("baf_v") :
										_T("baf_h"),
								WS_CHILD | WS_CLIPCHILDREN,
								x, y, w, h,
								pc->GetWindow(),
								NULL,
								pc->GetInstance(),
								(LPVOID)this
							);
		if(m_hwnd)
		{
			ShowWindow(m_hwnd, SW_SHOW);
			return errOK;
		}
	}
	return errFAILURE;
}

//***********************************************************************
ERRCODE Bsizer::Dettach()
{
	return errOK;
}


POINT m_ip;

//***********************************************************************
LRESULT Bsizer::OnMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC			hdc;
	PAINTSTRUCT ps;
	RECT		rc;

	switch(msg)
	{
	case WM_LBUTTONDOWN:

		SetCapture(hWnd);
		GetCursorPos(&m_ip);
		break;

	case WM_LBUTTONUP:

		ReleaseCapture();
		break;

	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		GetClientRect	(hWnd, &rc);
		OutlineBox		(hdc, &rc, false);
		EndPaint		(hWnd, &ps);
		break;

	case WM_MOUSEMOVE:

		break;

	default:
		return BappComponent::OnMessage(hWnd, msg, wParam, lParam);
	}
	return 0;
}

//***********************************************************************
// VERTICAL SPECIFIC
//***********************************************************************

//***********************************************************************
BvertSizer::BvertSizer(BAF_EDGE edge)
	:
	Bsizer(edge)
{
}

//***********************************************************************
BvertSizer::~BvertSizer()
{
}

//***********************************************************************
ERRCODE	BvertSizer::GetFit(int& x, int&y, int&w, int&h)
{
	HWND	hwndParent;
	RECT	rc;

	x = y = w = h = 1;

	if(! m_parent)		return errFAILURE;
	hwndParent = m_parent->GetWindow();
	if(! hwndParent)	return errFAILURE;

	GetClientRect(hwndParent, &rc);

	x = m_edge == frRight ? rc.left : rc.right - VSIZER_WIDTH;
	y = rc.top;
	w = VSIZER_WIDTH;
	h = rc.bottom - rc.top;

	return errOK;
}

//***********************************************************************
LRESULT BvertSizer::OnMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_MOUSEMOVE:

		if((GetCapture() == hWnd) && (wParam & MK_LBUTTON))
		{
			RECT  rcz, rcp, rcc;
			POINT ptp[2], cp;
			int   x;
			int   w;

			GetWindowRect(m_hwnd, &rcz);

			// map deskop coords of parent window into client coords of its parent
			//
			GetWindowRect(GetParent(m_hwnd), &rcp);
			ptp[0].x = 0;
			ptp[0].y = 0;
			MapWindowPoints(GetParent(m_hwnd), GetParent(GetParent(m_hwnd)), ptp, 1);

			w = rcp.right - rcp.left;

			GetCursorPos(&cp);
			x = cp.x;
			if(m_edge == frRight)
			{
				x -= rcp.left;
				ptp[0].x += x;
				w -= x;
			}
			else
			{
				x -= rcp.right;
				w += x;
			}
			if(rcz.left < 1 && x < 0)
				return 0;
			if(w < (rcz.right - rcz.left))
				return 0;

			GetClientRect(GetParent(GetParent(m_hwnd)), &rcc);

			if((ptp[0].x + w) > (rcc.right - rcc.left))
				return 0;
			if(ptp[0].x > (rcc.right - rcc.left))
				return 0;
			if(ptp[0].x < 0)
				return 0;

			MoveWindow(GetParent(m_hwnd), ptp[0].x, ptp[0].y, w, rcp.bottom - rcp.top, TRUE);

			// tell upper structure to modify other windows to accomadate us
			//
			if(m_parent)
				m_parent->OnResized(this);

		}
		break;

	default:
		return Bsizer::OnMessage(hWnd, msg, wParam, lParam);
	}
	return 0;
}


//***********************************************************************
// HORIZONTAL SPECIFIC
//***********************************************************************

//***********************************************************************
BhorzSizer::BhorzSizer(BAF_EDGE edge)
	:
	Bsizer(edge)
{
}

//***********************************************************************
BhorzSizer::~BhorzSizer()
{
}

//***********************************************************************
ERRCODE	BhorzSizer::GetFit(int& x, int&y, int&w, int&h)
{
	HWND	hwndParent;
	RECT	rc;

	x = y = w = h = 1;

	if(! m_parent)		return errFAILURE;
	hwndParent = m_parent->GetWindow();
	if(! hwndParent)	return errFAILURE;

	GetClientRect(hwndParent, &rc);

	x = rc.left;
	y = m_edge == frBottom ? rc.top : rc.bottom - HSIZER_HEIGHT;
	w = rc.right - rc.left;
	h = HSIZER_HEIGHT;

	return errOK;
}

//***********************************************************************
LRESULT BhorzSizer::OnMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_MOUSEMOVE:

		if((GetCapture() == hWnd) && (wParam & MK_LBUTTON))
		{
			RECT  rcz, rcp, rcc;
			POINT ptp[2], cp;
			int   y;
			int   h;

			GetWindowRect(m_hwnd, &rcz);

			// map deskop coords of parent window into client coords of its parent
			//
			GetWindowRect(GetParent(m_hwnd), &rcp);
			ptp[0].x = 0;
			ptp[0].y = 0;
			MapWindowPoints(GetParent(m_hwnd), GetParent(GetParent(m_hwnd)), ptp, 1);

			h = rcp.bottom - rcp.top;
			GetCursorPos(&cp);
			y = cp.y;

			if(m_edge == frBottom)
			{
				y -= rcp.top;
				ptp[0].y += y;
				h -= y;
				/*
				printf("z=%d,%d  p=%d,%d y=%d h=%d ptp=%d,%d\n",
					rcz.left, rcz.top, rcp.left, rcp.top, y, h, ptp[0].x, ptp[0].y);
				*/
			}
			else
			{
				y -= rcp.bottom;
				h += y;
			}
			if(rcz.top < 1 && y < 0)
				return 0;
			if(h < (rcz.bottom - rcz.top))
				return 0;

			GetClientRect(GetParent(GetParent(m_hwnd)), &rcc);

			if((ptp[0].y + h) > (rcc.bottom - rcc.top))
				return 0;
			if(ptp[0].y > (rcc.bottom - rcc.top))
				return 0;
			if(ptp[0].y < 0)
				return 0;

			MoveWindow(GetParent(m_hwnd), ptp[0].x, ptp[0].y, rcp.right - rcp.left, h, TRUE);

			// tell upper structure to modify other windows to accomadate us
			//
			if(m_parent)
				m_parent->OnResized(this);
		}
		break;

	default:
		return Bsizer::OnMessage(hWnd, msg, wParam, lParam);
	}
	return 0;
}
