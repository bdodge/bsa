
#include "bcalcx.h"

#define PLOT_BUT_MAP_W	6
#define PLOT_BUT_MAP_H	2


BCbutton g_plotbuttons[PLOT_BUT_MAP_H][PLOT_BUT_MAP_W] =
{
	{
		{ IDC_STATIC,	_T(" X from"), 	2*BUT_DIG_W, BUT_DIG_H, bcLabel },
		{ ID_PRG_XMIN,	_T("XMIN"), 	3*BUT_DIG_W, BUT_DIG_H, bcEdit },
		{ IDC_STATIC,	_T(" to"),	 	1*BUT_DIG_W, BUT_DIG_H, bcLabel },
		{ ID_PRG_XMAX,	_T("XMAX"),		3*BUT_DIG_W, BUT_DIG_H, bcEdit },
		{ IDC_STATIC,	_T(" by"),	 	1*BUT_DIG_W, BUT_DIG_H, bcLabel },
		{ ID_PRG_XSTEP,	_T("XSTP"),		3*BUT_DIG_W, BUT_DIG_H, bcEdit },
	},
	{
		{ IDC_STATIC,	_T(" Y from"), 	2*BUT_DIG_W, BUT_DIG_H, bcLabel },
		{ ID_PRG_YMIN,	_T("YMIN"), 	3*BUT_DIG_W, BUT_DIG_H, bcEdit },
		{ IDC_STATIC,	_T(" to"),	 	1*BUT_DIG_W, BUT_DIG_H, bcLabel },
		{ ID_PRG_YMAX,	_T("YMAX"),		3*BUT_DIG_W, BUT_DIG_H, bcEdit },
		{ IDC_STATIC,	_T(" by"),	 	1*BUT_DIG_W, BUT_DIG_H, bcLabel },
		{ ID_PRG_YSTEP,	_T("YSTP"),		3*BUT_DIG_W, BUT_DIG_H, bcEdit },
	}
};

//**************************************************************************
Bplot::Bplot(Bcalc* pCalc, BappPanel* pParent)
	:
	BappPane(_T("Plot"), pParent),
	m_bcalc(pCalc),
	m_pnts(NULL),
	m_cpnt(NULL),
	m_xaxis(NULL),
	m_yaxis(NULL),
	m_zaxis(NULL),
	m_firstpoint(true),
	m_dim(2),
	m_sx(-10.0),
	m_sy(-10.0),
	m_ex(10.0),
	m_ey(10.0),
	m_xs(1.0),
	m_ys(1.0),
	m_ax(2.0 * 3.14159 * 30.0 / 360.0),
	m_ay(2.0 * 3.14159 * 30.0 / 360.0)
{
	m_hop = NULL;
	m_pens[penBlack]	= CreatePen(PS_SOLID, 1, RGB(10,10,10));
	m_pens[penBlue]		= CreatePen(PS_SOLID, 1, RGB(10,10,200));
	m_pens[penRed]		= CreatePen(PS_SOLID, 1, RGB(200,10,10));
	m_pens[penGreen]	= CreatePen(PS_SOLID, 1, RGB(10,200,10));
	m_sinxa = sin(m_ax);
	m_cosxa = cos(m_ax);
	m_sinya = sin(m_ay);
	m_cosya = cos(m_ay);
}

//**************************************************************************
Bplot::~Bplot()
{
	DeleteObject(m_pens[penBlack]);
	DeleteObject(m_pens[penBlue]);
	DeleteObject(m_pens[penRed]);
	DeleteObject(m_pens[penGreen]);
	DeletePoints();
}

//**************************************************************************
void Bplot::FitToPanel()
{
	RECT rc;

	GetParentPanel()->GetPanelClientRect(&rc);
	if(! m_hwnd)
	{

		m_hwnd = CreateWindow(
								_T("bcalc_plot_pane"),
								_T("Plot"),
								WS_CHILD | WS_CLIPCHILDREN,
								0, 0, rc.right-rc.left, rc.bottom-rc.top,
								GetParentPanel()->GetWindow(),
								NULL,
								GetParentPanel()->GetInstance(),
								(LPVOID)this
							 ); 
		SetupWindows(true);
	}
	GetClientRect(GetParentPanel()->GetWindow(), &rc);
	m_bcalc->GetPersist()->SetNvInt(_T("display/plot/H"), rc.bottom - rc.top);
	SetupWindows(false);
	BappPane::FitToPanel();
}


//**************************************************************************
void Bplot::SetupWindows(bool docreate)
{
	RECT rc;
	int  x, y, w, h;
	int  ix, iy, bx, by, bw, bh;
	int  aw, ah;
	int  max_h;
	int	 enabled, wasenabled;
	TCHAR v[128];

	if(! m_hwnd) return;

	GetParentPanel()->GetPanelClientRect(&rc);
	aw = rc.right - rc.left;
	ah = rc.bottom - rc.top;

	GetParentPanel()->GetPanelClientRect(&rc);
	aw = rc.right - rc.left;
	ah = rc.bottom - rc.top;

	m_bcalc->GetButtonSize(&bw, &bh);

	x = BCALC_DISP_LM;
	y = BCALC_DISP_TM;
	w = aw - 2 * BCALC_DISP_LM;
	h = ah - 2 * BCALC_DISP_TM;

	// button map
	//
	x = BCALC_BUT_LM;
	y = BCALC_DISP_TM;

	for(iy = 0, by = y; iy < PLOT_BUT_MAP_H; iy++)
	{
		for(ix = max_h = 0, bx = x; ix < PLOT_BUT_MAP_W; ix++)
		{
			PBCBUTTON pb;

			pb = &g_plotbuttons[iy][ix];
			w = pb->m_w;
			h = pb->m_h;

			w = (pb->m_w * aw) / (BUT_DIG_W * (PLOT_BUT_MAP_W + 1));
			h = (pb->m_h * (ah - y)) / (BUT_DIG_H * PLOT_BUT_MAP_H);
			if(w < BCALC_BUT_MIN_W)
				w = BCALC_BUT_MIN_W;
			if(w > bw * pb->m_w / BUT_DIG_W)
				w = bw * pb->m_w / BUT_DIG_W;
			if(h < BCALC_BUT_MIN_H)
				h = BCALC_BUT_MIN_H;
			if(h > bh)
				h = bh;
			if(h > 16)
				h = 16;
			w = w - BCALC_BUT_MX;
			h = h - BCALC_BUT_MY;

			if(pb->m_id)
			{
				LPCTSTR pcn;

				switch(pb->m_class)
				{
				case bcEdit:	pcn = _T("Edit");			break;
				case bcLabel:	pcn = _T("Static");			break;
				default:		pcn = _T("bcalc_button");	break;
				}
				if(docreate || ! pb->m_hwnd)
				{
					pb->m_hwnd = CreateWindow(
											pcn,
											pb->m_name,
											WS_CHILD | (pb->m_class == bcEdit ? (WS_BORDER) : 0),
											bx, by, w, h,
											GetWindow(),
											NULL,
											GetParentPanel()->GetInstance(),
											(LPVOID)pb
											);
					SetWindowLong(pb->m_hwnd, GWL_ID, pb->m_id);
					enabled    = TRUE;
					EnableWindow(pb->m_hwnd, enabled);
					SendMessage(pb->m_hwnd, WM_SETFONT, (WPARAM)m_bcalc->m_hf_c, (LPARAM)m_bcalc->m_hf_c);
					switch(pb->m_id)
					{
					case ID_PRG_XMIN:
						m_bcalc->GetPersist()->GetNvStr(_T("plot/xmin"), v, 128, _T("-10.0"));
						SetWindowText(pb->m_hwnd, v);
						break;
					case ID_PRG_XMAX:
						m_bcalc->GetPersist()->GetNvStr(_T("plot/xmax"), v, 128, _T("10.0"));
						SetWindowText(pb->m_hwnd, v);
						break;
					case ID_PRG_XSTEP:
						m_bcalc->GetPersist()->GetNvStr(_T("plot/xstep"), v, 128, _T("1.0"));
						SetWindowText(pb->m_hwnd, v);
						break;
					case ID_PRG_YMIN:
						m_bcalc->GetPersist()->GetNvStr(_T("plot/ymin"), v, 128, _T("-10.0"));
						SetWindowText(pb->m_hwnd, v);
						break;
					case ID_PRG_YMAX:
						m_bcalc->GetPersist()->GetNvStr(_T("plot/ymax"), v, 128, _T("10.0"));
						SetWindowText(pb->m_hwnd, v);
						break;
					case ID_PRG_YSTEP:
						m_bcalc->GetPersist()->GetNvStr(_T("plot/ystep"), v, 128, _T("1.0"));
						SetWindowText(pb->m_hwnd, v);
						break;
					}
					ShowWindow(pb->m_hwnd, SW_SHOW);
				}
				else if(pb->m_hwnd)
				{
					MoveWindow(pb->m_hwnd, bx, by, w, h, TRUE);

					wasenabled = IsWindowEnabled(pb->m_hwnd);
					enabled    = TRUE;
					EnableWindow(pb->m_hwnd, enabled);
					if(enabled != wasenabled)
						InvalidateRect(pb->m_hwnd, NULL, FALSE);
				}
			}
			bx += w + BCALC_BUT_MX;
			if(h > max_h) max_h = h;
		}
		by += max_h + BCALC_BUT_MY;
	}
	SetPlotExtents(m_dim, m_sx, m_sy, m_ex, m_ey, m_xs, m_ys);
}

#define BCALC_AXIS_XM	6
#define BCALC_AXIS_YM	6

//**************************************************************************
void Bplot::DeletePoints()
{
	PPNT pp, px;

	for(pp = m_pnts; pp;)
	{
		px = pp;
		pp = pp->m_next;
		delete px;
	}
	m_pnts = m_cpnt = NULL;

	if(m_xaxis)
	{
		delete m_xaxis->m_next;
		delete m_xaxis;
		m_xaxis = NULL;
	}
	if(m_yaxis)
	{
		delete m_yaxis->m_next;
		delete m_yaxis;
		m_yaxis = NULL;
	}
	if(m_zaxis)
	{
		delete m_zaxis->m_next;
		delete m_zaxis;
		m_zaxis = NULL;
	}
}

//**************************************************************************
ERRCODE Bplot::GetPlotExtents(int& d, double& sx, double& sy, double& ex, double& ey, double& xs, double& ys)
{
	int		ix, iy;
	TCHAR	vb[128];
	double	v;
	bool	ok;

	d = m_dim;

	for(iy = 0; iy < PLOT_BUT_MAP_H; iy++)
	{
		for(ix = 0; ix < PLOT_BUT_MAP_W; ix++)
		{
			PBCBUTTON pb;
			LPCTSTR   pkn;

			pb = &g_plotbuttons[iy][ix];

			GetWindowText(pb->m_hwnd, vb, 128);
			v = _tcstod(vb, NULL);

			switch(pb->m_id)
			{
			case ID_PRG_XMIN:	m_sx = sx = v; pkn = _T("X Start");	ok = true; break;
			case ID_PRG_XMAX:	m_ex = ex = v; pkn = _T("X End"); 	ok = true; break;
			case ID_PRG_XSTEP:	m_xs = xs = v; pkn = _T("X Step"); 	ok = true; break;
			case ID_PRG_YMIN:	m_sy = sy = v; pkn = _T("Y Start");	ok = true; break;
			case ID_PRG_YMAX:	m_ey = ey = v; pkn = _T("Y End");	ok = true; break;
			case ID_PRG_YSTEP:	m_ys = ys = v; pkn = _T("Y Step"); 	ok = true; break;
			default:												ok = false;break;
			}
			if(ok)
			{
				int iv;

				_sntprintf(vb, 128, _T("display/plot/" _Pfs_ ""), pkn);
				iv = (int)(v * 1000.0);
				m_bcalc->GetPersist()->SetNvInt(vb, iv);
			}
		}
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bplot::SetPlotExtents(int d, double sx, double sy, double ex, double ey, double xs, double ys)
{
	int		ix, iy;
	TCHAR	vb[128];
	double  v;
	bool    ok;

	m_sx = sx;
	m_sy = sy;
	m_ex = ex;
	m_ey = ey;
	m_xs = xs;
	m_ys = ys;

	m_dim = d;

	for(iy = 0; iy < PLOT_BUT_MAP_H; iy++)
	{
		for(ix = 0; ix < PLOT_BUT_MAP_W; ix++)
		{
			PBCBUTTON pb;

			pb = &g_plotbuttons[iy][ix];

			switch(pb->m_id)
			{
			case ID_PRG_XMIN:	v = m_sx; ok = true; break;
			case ID_PRG_XMAX:	v = m_ex; ok = true; break;
			case ID_PRG_XSTEP:	v = m_xs; ok = true; break;
			case ID_PRG_YMIN:	v = m_sy; ok = true; break;
			case ID_PRG_YMAX:	v = m_ey; ok = true; break;
			case ID_PRG_YSTEP:	v = m_ys; ok = true; break;
			default:					  ok = false;break;
			}
			if(ok && pb->m_hwnd)
			{
				_sntprintf(vb, 128, _T("%f"), v);
				SetWindowText(pb->m_hwnd, vb);
			}
		}
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bplot::StartPlot(int d)
{
	double	w, h, xr, yr;
	RECT	rc;

	m_dim = d;

	GetClientRect(m_hwnd, &rc);
	w = (double)(rc.right - rc.left);
	h = (double)(rc.bottom - rc.top);
	xr = fabs(m_ex - m_sx);
	yr = fabs(m_ey - m_sy);

	m_hdc = GetDC(m_hwnd);
	m_hop = (HPEN)SelectObject(m_hdc, m_pens[penBlack]);

	DeletePoints();
	InvalidateRect(m_hwnd, NULL, TRUE);

	if(d == 2)
	{
		if(w < 2.0 * BCALC_AXIS_XM) w = 2.0 * BCALC_AXIS_XM;
		m_xscale  = (w - 2.0 * BCALC_AXIS_XM) / (xr > 0.0 ? xr : 1.0);
		m_yscale  = 1.0;
		m_xoffset = w / 2.0;
		m_yoffset = h / 2.0;

		m_xaxis = new PNT((int)BCALC_AXIS_XM, (int)m_yoffset);
		m_xaxis->m_anchor = true;
		m_xaxis->m_next = new PNT((int)(w - 2.0 * BCALC_AXIS_XM), (int)m_yoffset);

		m_yaxis = new PNT((int)m_xoffset, (int)BCALC_AXIS_YM);
		m_yaxis->m_anchor = true;
		m_yaxis->m_next = new  PNT((int)m_xoffset, (int)(h - 2.0 * BCALC_AXIS_XM)); 
	}
	else
	{
		if(w < 2.0 * BCALC_AXIS_XM) w = 2.0 * BCALC_AXIS_XM;
		if(h < 2.0 * BCALC_AXIS_YM) h = 2.0 * BCALC_AXIS_YM;
		m_xscale  = (w - 2.0 * BCALC_AXIS_XM) / (xr > 0.0 ? xr : 1.0);
		m_yscale  = (h - 2.0 * BCALC_AXIS_XM) / (xr > 0.0 ? xr : 1.0);
		m_xoffset = 0;
		m_yoffset = h + 80;
		m_xoffset = w / 2.0;
		m_yoffset = h / 2.0;
	}
	m_firstpoint = true;
	return errOK;
}

//**************************************************************************
ERRCODE Bplot::EndPlot(void)
{
	if(m_hdc) 
	{
		SelectObject(m_hdc, m_hop);
		ReleaseDC(m_hwnd, m_hdc);
	}
	m_hdc = NULL;
	return errOK;
}

//**************************************************************************
ERRCODE Bplot::PlotY(double x, double y)
{
	int		 xx, yy;
	PPNT	 np;

	if(m_hdc)
	{
		yy = (int)(m_yoffset - m_yscale * y);
		xx = (int)(m_xscale * x + m_xoffset);
		np = new PNT(xx, yy);
		if(! m_cpnt)
		{
			m_cpnt = m_pnts = np;
		}
		else
		{
			m_cpnt->m_next = np;
			m_cpnt = np;
		}
		if(m_firstpoint)
		{
			m_firstpoint = false;
			np->m_anchor = true;
			MoveToEx(m_hdc, xx, yy, NULL);
		}
		else
		{
			LineTo(m_hdc, xx, yy);
		}
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bplot::PlotZ(double x, double y, double z)
{
	double	 xv, yv, zx, zy;
	int		 xx, yy;
	PPNT	 np;

	if(m_hdc)
	{
		zy = m_yscale * y;
		zx = m_xscale * x;
		xv = m_xoffset + zy * m_cosya + zx * m_cosxa;
		yv = m_yoffset - (zy * m_sinya - zx * m_sinxa + z);
		xx = (int)xv;
		yy = (int)yv;
		np = new PNT(xx, yy);
		if(! m_cpnt)
		{
			m_cpnt = m_pnts = np;
		}
		else
		{
			m_cpnt->m_next = np;
			m_cpnt = np;
		}
		if(m_firstpoint)
		{
			m_firstpoint = false;
			np->m_anchor = true;
			MoveToEx(m_hdc, xx, yy, NULL);
		}
		else
		{
			LineTo(m_hdc, xx, yy);
		}
	}
	return errOK;
}


//**************************************************************************
LRESULT Bplot::OnMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC			hdc;
	HPEN		hop;
	RECT		rc;
	Bplot*		pplot;
	PPNT		pp;
	int			id = 0;

	pplot = (Bplot*)GetWindowLong(hWnd, GWL_USERDATA);

	switch (message) 
	{
	case WM_CREATE:

		{ 
			LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;

			SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)pcs->lpCreateParams);
		}
		break;

	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rc);
		hop = (HPEN)SelectObject(hdc, m_pens[penBlack]);
		if((pp = pplot->m_pnts) != NULL)
		{
			MoveToEx(hdc, pp->m_x, pp->m_y, NULL);
			for(pp = pp->m_next; pp; pp = pp->m_next)
			{
				if(pp->m_anchor)
					MoveToEx(hdc, pp->m_x, pp->m_y, NULL);
				else
					LineTo(hdc, pp->m_x, pp->m_y);
			}
		}
		if((pp = pplot->m_xaxis) != NULL)
		{
			SelectObject(hdc, m_pens[penBlue]);
			MoveToEx(hdc, pplot->m_xaxis->m_x, pplot->m_xaxis->m_y, NULL);
			LineTo(hdc, pplot->m_xaxis->m_next->m_x, pplot->m_xaxis->m_next->m_y); 
		}
		if((pp = pplot->m_yaxis) != NULL)
		{
			SelectObject(hdc, m_pens[penGreen]);
			MoveToEx(hdc, pplot->m_yaxis->m_x, pplot->m_yaxis->m_y, NULL);
			LineTo(hdc, pplot->m_yaxis->m_next->m_x, pplot->m_yaxis->m_next->m_y); 
		}
		if((pp = pplot->m_zaxis) != NULL)
		{
			SelectObject(hdc, m_pens[penRed]);
			MoveToEx(hdc, pplot->m_zaxis->m_x, pplot->m_zaxis->m_y, NULL);
			LineTo(hdc, pplot->m_zaxis->m_next->m_x, pplot->m_zaxis->m_next->m_y); 
		}
		SelectObject(hdc, hop);
		EndPaint(hWnd, &ps);
		break;

	case WM_COMMAND:

		break;

	case WM_CHAR:

		break;

	default:
		return BappPane::OnMessage(hWnd, message, wParam, lParam);
   }
   return 0;
}

