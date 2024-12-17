
#include "bcalcx.h"


#define BUT_MAP_W	8
#define BUT_MAP_H	8
#define BUT_MAP_SCITRIG_H 2


BCbutton g_buttons[BUT_MAP_H][BUT_MAP_W] =
{
	// top row
	{
		{ ID_NUM_D,		_T("D"), 		BUT_DIG_W,	 BUT_DIG_H, bcDigit },
		{ ID_NUM_E,		_T("E"),		BUT_DIG_W,	 BUT_DIG_H, bcDigit },
		{ ID_NUM_F,		_T("F"),		BUT_DIG_W,	 BUT_DIG_H, bcDigit },
		{ ID_CTL_CLR,	_T("Clr"), 		BUT_DIG_W,	 BUT_DIG_H, bcDisplay },
		{ ID_CTL_CLRA,	_T("Clear All"),2*BUT_DIG_W, BUT_DIG_H, bcDisplay },
		{ ID_CTL_DEL,	_T("Del"), 		BUT_DIG_W,	 BUT_DIG_H, bcDisplay },
		{ ID_CTL_STR,	_T("Store"), 	2*BUT_DIG_W, BUT_DIG_H, bcControl },
		{ 0,			_T(""),		 	BUT_DIG_W,	 BUT_DIG_H, bcControl },
	},
	{
		{ ID_NUM_A,		_T("A"), 		BUT_DIG_W,	 BUT_DIG_H, bcDigit },
		{ ID_NUM_B,		_T("B"),		BUT_DIG_W,	 BUT_DIG_H, bcDigit },
		{ ID_NUM_C,		_T("C"),		BUT_DIG_W,	 BUT_DIG_H, bcDigit },
		{ ID_OPR_OR,	_T("|"),	 	BUT_DIG_W,	 BUT_DIG_H, bcBinOperator },
		{ ID_OPR_AND,	_T("&"),		BUT_DIG_W,	 BUT_DIG_H, bcBinOperator },
		{ ID_OPR_SHFTL,	_T("<<"), 		BUT_DIG_W,	 BUT_DIG_H, bcBinOperator },
		{ ID_OPR_SHFTR,	_T(">>"), 		BUT_DIG_W,	 BUT_DIG_H, bcBinOperator },
		{ ID_CTL_RCL,	_T("Recall"), 	2*BUT_DIG_W, BUT_DIG_H, bcControl },
	},
	{
		{ ID_NUM_7,		_T("7"), 		BUT_DIG_W,	 BUT_DIG_H, bcDigit },
		{ ID_NUM_8,		_T("8"),		BUT_DIG_W,	 BUT_DIG_H, bcDigit },
		{ ID_NUM_9,		_T("9"),		BUT_DIG_W,	 BUT_DIG_H, bcDigit },
		{ ID_OPR_XOR,	_T("^"),	 	BUT_DIG_W,	 BUT_DIG_H, bcBinOperator },
		{ ID_OPR_NOT,	_T("~"),		BUT_DIG_W,	 BUT_DIG_H, bcBinOperator },
		{ ID_OPR_INT,	_T("Int"), 		BUT_DIG_W,	 BUT_DIG_H, bcBinOperator },
		{ ID_OPR_FRAC,	_T("Frac"),		BUT_DIG_W,	 BUT_DIG_H, bcBinOperator },
		{ ID_CTL_PRG,	_T("Program"), 	2*BUT_DIG_W, BUT_DIG_H, bcProgram },
	},
	{
		{ ID_NUM_4,		_T("4"), 		BUT_DIG_W,	 BUT_DIG_H, bcDigit },
		{ ID_NUM_5,		_T("5"),		BUT_DIG_W,	 BUT_DIG_H, bcDigit },
		{ ID_NUM_6,		_T("6"),		BUT_DIG_W,	 BUT_DIG_H, bcDigit },
		{ ID_OPR_MUL,	_T("X"),	 	BUT_DIG_W,	 BUT_DIG_H, bcBinOperator },
		{ ID_OPR_DIV,	_T("/"),		BUT_DIG_W,	 BUT_DIG_H, bcBinOperator },
		{ ID_OPR_MOD,	_T("%"), 		BUT_DIG_W,	 BUT_DIG_H, bcBinOperator },
		{ ID_OPR_INV,	_T("1/X"), 		BUT_DIG_W,	 BUT_DIG_H, bcBinOperator },
		{ ID_CTL_SCI,	_T("Sci/Trig"),	2*BUT_DIG_W, BUT_DIG_H, bcControl },
	},
	{
		{ ID_NUM_1,		_T("1"), 		BUT_DIG_W,	 BUT_DIG_H, bcDigit },
		{ ID_NUM_2,		_T("2"),		BUT_DIG_W,	 BUT_DIG_H, bcDigit },
		{ ID_NUM_3,		_T("3"),		BUT_DIG_W,	 BUT_DIG_H, bcDigit },
		{ ID_OPR_ADD,	_T("+"),	 	BUT_DIG_W,	 BUT_DIG_H, bcBinOperator },
		{ ID_OPR_SUB,	_T("-"),		BUT_DIG_W,	 BUT_DIG_H, bcBinOperator },
		{ ID_OPR_SQ,	_T("x^2"), 		BUT_DIG_W,	 BUT_DIG_H, bcBinOperator },
		{ ID_OPR_SQRT,	_T("sqrt"), 	BUT_DIG_W,	 BUT_DIG_H, bcBinOperator },
		{ ID_CTL_CONST,	_T("Constants"),2*BUT_DIG_W, BUT_DIG_H, bcControl },
	},
	{
		{ ID_NUM_0,		_T("0"), 		BUT_DIG_W,	 BUT_DIG_H, bcDigit },
		{ ID_CTL_DEC,	_T("."),		BUT_DIG_W,	 BUT_DIG_H, bcDigit },
		{ ID_OPR_NEG,	_T("+/-"),		BUT_DIG_W,	 BUT_DIG_H, bcUnOperator },
		{ ID_OPR_EQUAL,	_T("="),	 	2*BUT_DIG_W, BUT_DIG_H, bcUnOperator},
		{ ID_OPR_LPAREN,_T("("),		BUT_DIG_W,	 BUT_DIG_H, bcUnOperator },
		{ ID_OPR_RPAREN,_T(")"), 		BUT_DIG_W,	 BUT_DIG_H, bcUnOperator },
		{ ID_CTL_EXP,	_T("exp"), 		BUT_DIG_W,	 BUT_DIG_H, bcUnOperator },
#ifdef _LIB
		{ ID_CTL_COPY,	_T("Cpy"),		BUT_DIG_W,	 BUT_DIG_H, bcAppControl },
#else
		{ ID_CTL_QUIT,	_T("Quit"), 	BUT_DIG_W,	 BUT_DIG_H, bcAppControl },
#endif
	},
	{
		{ ID_OPR_SIN,	_T("sin"), 		BUT_DIG_W,	 BUT_DIG_H, bcSciTrigUnOperator },
		{ ID_OPR_COS,	_T("cos"), 		BUT_DIG_W,	 BUT_DIG_H, bcSciTrigUnOperator },
		{ ID_OPR_TAN,	_T("tan"), 		BUT_DIG_W,	 BUT_DIG_H, bcSciTrigUnOperator },
		{ ID_OPR_EXPO,	_T("e^x"), 		BUT_DIG_W,	 BUT_DIG_H, bcSciTrigUnOperator },
		{ ID_OPR_NLOG,	_T("ln"), 		BUT_DIG_W,	 BUT_DIG_H, bcSciTrigUnOperator },
		{ ID_OPR_XEXPY,	_T("x^y"), 		BUT_DIG_W,	 BUT_DIG_H, bcSciTrigOperator	},
	},
	{
		{ ID_OPR_ASIN,	_T("asin"),		BUT_DIG_W,	 BUT_DIG_H, bcSciTrigUnOperator },
		{ ID_OPR_ACOS,	_T("acos"),		BUT_DIG_W,	 BUT_DIG_H, bcSciTrigUnOperator },
		{ ID_OPR_ATAN,	_T("atan"),		BUT_DIG_W,	 BUT_DIG_H, bcSciTrigUnOperator },
		{ ID_OPR_10EXP,	_T("10^x"), 	BUT_DIG_W,	 BUT_DIG_H, bcSciTrigUnOperator },
		{ ID_OPR_LOG,	_T("log"), 		BUT_DIG_W,	 BUT_DIG_H, bcSciTrigUnOperator },
	}
};

BCbutton g_ctrls[] =
{
	{ ID_RADIX_UP,	_T(">"),		BCALC_RADBUT_W, BCALC_RADBUT_H, bcAppControl },
	{ ID_RADIX_DOWN,_T("<"),		BCALC_RADBUT_W, BCALC_RADBUT_H, bcAppControl },
	{ ID_XRADIX_UP,	_T(">"),		BCALC_RADBUT_W, BCALC_RADBUT_H, bcAppControl },
	{ ID_XRADIX_DOWN,_T("<"),		BCALC_RADBUT_W, BCALC_RADBUT_H, bcAppControl },
	{ ID_FORMAT_UP,	_T(">"),		BCALC_RADBUT_W, BCALC_RADBUT_H, bcAppControl },
	{ ID_FORMAT_DOWN,_T("<"),		BCALC_RADBUT_W, BCALC_RADBUT_H, bcAppControl },
	{ ID_DFRMT_UP,	_T(">"),		BCALC_RADBUT_W, BCALC_RADBUT_H, bcAppControl },
	{ ID_DFRMT_DOWN,_T("<"),		BCALC_RADBUT_W, BCALC_RADBUT_H, bcAppControl },
	{ 0,			_T(""),		 	BUT_DIG_W,	 BUT_DIG_H, bcControl },
};

//**************************************************************************
Bcalc::Bcalc(Bpersist* pPersist, BappPanel* pPanel, bool isdark)
	:
	BappPane(_T("bcalc"), pPanel),
	m_persist(pPersist),
	m_cursor(0),
	m_expcursor(0),
	m_state(kbBlank),
	m_focus(kbDisplay),
	m_mathformat(kbInteger),
	m_dispformat(kbDefault),
	m_bitsize(32),
	m_radix(10),
	m_xradix(16),
	m_progmode(pmEdit),
	m_progpanel(NULL),
	m_plotpanel(NULL),
	m_view_scitrig(false),
	m_view_program(false),
	m_constants(NULL),
	m_conversions(NULL)
{
	m_hf_n = CreateFont(
							14,0,0,0,700,FALSE,FALSE,FALSE,ANSI_CHARSET,
							OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
							ANTIALIASED_QUALITY/*DEFAULT_QUALITY*/,FF_SWISS,_T("Arial")
							);
	m_hf_c = CreateFont(
							12,0,0,0,500,FALSE,FALSE,FALSE,ANSI_CHARSET,
							OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
							ANTIALIASED_QUALITY/*DEFAULT_QUALITY*/,FF_SWISS,_T("Arial")
							);
	m_isdark = isdark;
	if(isdark)
	{
		m_dispFrg1	 = RGB(20,240,230);
		m_dispFrg2	 = RGB(0,200,180);
		m_dispBkg	 = RGB(20,20,20);
		m_scTextBkg	= COLOR_WINDOW;
		m_scAppBkg	= COLOR_WINDOW;
	}
	else
	{
		m_dispFrg1	 = RGB(0,160,140);
		m_dispFrg2	 = RGB(0,110,90);
		m_dispBkg	 = RGB(245,245,245);
		m_scTextBkg	= COLOR_WINDOW;
		m_scAppBkg	= COLOR_3DLIGHT;
	}
	m_hbrDigit1  = CreateSolidBrush(m_dispFrg1);
	m_hbrDigit2  = CreateSolidBrush(m_dispFrg2);
	m_hbrDisplay = CreateSolidBrush(m_dispBkg);

	m_hbrProg	 = CreateSolidBrush(RGB(240, 230, 230));
	m_hbrPlot	 = CreateSolidBrush(RGB(250, 245, 240));

	memset(m_mem, 0, sizeof(m_mem));

	m_ps		 = new Bstack();
	m_stage[0]	 = 0;
}

//**************************************************************************
Bcalc::~Bcalc()
{
	int i, j;

	for(i = 0; i < sizeof(m_mem)/sizeof(Bnumber*); i++)
		if(m_mem[i])
			delete m_mem[i];

	PBCALC_CV xcv, pcv;

	for(pcv = m_constants; pcv;)
	{
		xcv = pcv;
		pcv = pcv->m_next;
		delete xcv;
	}
	for(pcv = m_conversions; pcv;)
	{
		xcv = pcv;
		pcv = pcv->m_next;
		delete xcv;
	}
	DestroyWindow(m_hwndDisplay);
	DestroyWindow(m_hwndXdisplay);
	DestroyWindow(m_hwndXinfo);
	DestroyWindow(m_hwndFinfo);
	DestroyWindow(m_hwnd_drup);
	DestroyWindow(m_hwnd_xrup);
	DestroyWindow(m_hwnd_drdn);
	DestroyWindow(m_hwnd_xrdn);
	DestroyWindow(m_hwnd_nfrmtup);
	DestroyWindow(m_hwnd_nfrmtdn);
	DestroyWindow(m_hwnd_dfrmtup);
	DestroyWindow(m_hwnd_dfrmtdn);

	for(i = 0; i < (m_view_scitrig ? BUT_MAP_H : (BUT_MAP_H - BUT_MAP_SCITRIG_H)); i++)
		for(j = 0; j < BUT_MAP_W; j++)
			DestroyWindow(g_buttons[i][j].m_hwnd);

	DeleteObject(m_hbrDisplay);
	DeleteObject(m_hbrDigit1);
	DeleteObject(m_hbrDigit2);
	DeleteObject(m_hbrPlot);
	DeleteObject(m_hbrProg);
}

static LPCTSTR const_table[] =
{
	_T("pi 3.14159265358979323846"),
	_T("e  2.71828182845904523536"),
	_T("c  2.99792458e08"),
	_T("G  6.6742"),
	_T("h  6.6260693e-34"),
	_T("Na 6.0221415e23"),
	_T("R  8.314472")
};

static LPCTSTR conv_table[] =
{
	_T("degF -> degC:- 32 = * 5 / 9 ="),
	_T("degC -> degF:* 9 / 5 + 32 ="),
	_T("lbs -> kg:* .4536 ="),
	_T("kg -> lbs:/ .4536 ="),
	_T("in -> cm:* 2.54 ="),
	_T("cm -> in:/ 2.54 ="),
	_T("mile -> km:/ .621 ="),
	_T("km -> mile:* .621 ="),
	_T("kW -> HP:* 1.34 ="),
	_T("HP -> kW:/ 1.34 =")
};

//**************************************************************************
ERRCODE Bcalc::RestoreSettings()
{
	ERRCODE   ec;
	TCHAR	  key[MAX_PATH], txt[MAX_PATH];
	int		  i, j;

	if(! m_persist) return errFAILURE;

	ec = m_persist->GetNvInt(_T("math/format"),		(int&)m_mathformat, kbFloat);
	ec = m_persist->GetNvInt(_T("display/radix"),	m_radix, 10);
	ec = m_persist->GetNvInt(_T("xdisplay/radix"),	m_xradix, 16);
	ec = m_persist->GetNvInt(_T("display/format"),	(int&)m_dispformat, kbDefault);
	ec = m_persist->GetNvBool(_T("display/viewscitrig"),m_view_scitrig, false);
	ec = m_persist->GetNvBool(_T("display/viewprogram"),m_view_program, false);


	j = sizeof(const_table)/sizeof(LPCTSTR);
	for(i = 0; i < 32; i++)
	{
		_sntprintf(key, MAX_PATH, _T("constants/const_%02d"), i);
		txt[0] = _T('\0');
		ec = m_persist->GetNvStr(key, txt, MAX_PATH, (i < j) ? const_table[i] : _T(""));
		if(_tcslen(txt) > 0)
			AddConstant(txt);
	}

	j = sizeof(conv_table)/sizeof(LPCTSTR);
	for(i = 0; i < 32; i++)
	{
		_sntprintf(key, MAX_PATH, _T("conversions/conv_%02d"), i);
		txt[0] = _T('\0');
		ec = m_persist->GetNvStr(key, txt, MAX_PATH, (i < j) ? conv_table[i] : _T(""));
		if(_tcslen(txt) > 0)
			AddConversion(txt);
	}

	return ec;
}

//**************************************************************************
ERRCODE Bcalc::SaveSettings()
{
	ERRCODE ec;

	if(! m_persist) return errFAILURE;

	ec = m_persist->SetNvInt(_T("math/format"),		(int)m_mathformat);
	ec = m_persist->SetNvInt(_T("display/radix"),	m_radix);
	ec = m_persist->SetNvInt(_T("xdisplay/radix"),	m_xradix);
	ec = m_persist->SetNvInt(_T("display/format"),	(int)m_dispformat);
	ec = m_persist->SetNvBool(_T("display/viewscitrig"),m_view_scitrig);
	ec = m_persist->SetNvBool(_T("display/viewprogram"),m_view_program);
	return ec;
}


//**************************************************************************
void BoxOutline(HDC hdc, LPRECT rc, int pushed)
{
	HPEN		hPen;
	HPEN		hOld;

	// br outside
	hPen = CreatePen(PS_SOLID, 0, GetSysColor(pushed ? COLOR_3DLIGHT : COLOR_3DDKSHADOW));
	hOld = (HPEN)SelectObject(hdc, hPen);
	MoveToEx(hdc, rc->left, rc->bottom, NULL);
	LineTo(hdc, rc->right, rc->bottom);
	LineTo(hdc, rc->right, rc->top-1);
	SelectObject(hdc, hOld);
	DeleteObject(hPen);

	// ul outside
	hPen = CreatePen(PS_SOLID, 0, GetSysColor(pushed ? COLOR_3DDKSHADOW : COLOR_BTNHIGHLIGHT));
	hOld = (HPEN)SelectObject(hdc, hPen);
	MoveToEx(hdc, rc->right-1, rc->top, NULL);
	LineTo(hdc, rc->left, rc->top);
	LineTo(hdc, rc->left, rc->bottom);
	SelectObject(hdc, hOld);
	DeleteObject(hPen);

	// br inside
	hPen = CreatePen(PS_SOLID, 0, GetSysColor(pushed ? COLOR_BTNHIGHLIGHT : COLOR_BTNSHADOW));
	hOld = (HPEN)SelectObject(hdc, hPen);
	MoveToEx(hdc, rc->left + 1, rc->bottom - 1, NULL);
	LineTo(hdc, rc->right - 1, rc->bottom - 1);
	LineTo(hdc, rc->right - 1, rc->top);
	SelectObject(hdc, hOld);
	DeleteObject(hPen);

	// ul inside
	hPen = CreatePen(PS_SOLID, 0, GetSysColor(pushed ? COLOR_BTNSHADOW : COLOR_3DLIGHT));
	hOld = (HPEN)SelectObject(hdc, hPen);
	MoveToEx(hdc, rc->right - 2, rc->top + 1, NULL);
	LineTo(hdc, rc->left + 1, rc->top + 1);
	LineTo(hdc, rc->left + 1, rc->bottom - 1);
	SelectObject(hdc, hOld);
	DeleteObject(hPen);
}

//***********************************************************************
LRESULT CALLBACK AppProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Bcalc*	pc;

	pc = (Bcalc*)GetWindowLong(hWnd, GWL_USERDATA);

	switch(message)
	{
	case WM_CREATE:

		// creation, copy cookie into userdata as "this"
		{
			LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;

			SetWindowLong(hWnd, GWL_USERDATA, (LONG)pcs->lpCreateParams);

			pc = (Bcalc*)GetWindowLong(hWnd, GWL_USERDATA);
		}
		// 	FALL into default
	default:

		if(pc)	return pc->OnMessage(hWnd, message, wParam, lParam);
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

void DrawDigit(
				HDC		hdc,
				int		x,
				int		y,
				int		size,
				int		solid,
				int		code,
				int		decpoint,
				int		comma,
				HBRUSH	hbrFrg,
				HBRUSH	hbrBkg
			  );

//***********************************************************************
LRESULT CALLBACK DispProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Bcalc*		pc;
	HDC			hdc;
	PAINTSTRUCT	ps;
	RECT		rc;

	pc = (Bcalc*)GetWindowLong(hWnd, GWL_USERDATA);

	switch(message)
	{
	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rc);

		//rc.left;
		//rc.top;
		rc.right--;
		rc.bottom--;

		BoxOutline(hdc, &rc, 1);
		if(pc) pc->OnDrawDisplay(hdc, hWnd, &rc);
		EndPaint(hWnd, &ps);
		break;

	case WM_CREATE:

		// creation, copy cookie into userdata as "this"
		{
			LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;

			SetWindowLong(hWnd, GWL_USERDATA, (LONG)pcs->lpCreateParams);

			pc = (Bcalc*)GetWindowLong(hWnd, GWL_USERDATA);

		}
		// 	FALL into default
	default:

		if(pc)	return pc->OnMessage(hWnd, message, wParam, lParam);
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}


//***********************************************************************
LRESULT CALLBACK ButtonProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PBCBUTTON	pb;
	HDC			hdc;
	PAINTSTRUCT	ps;
	RECT		rc;
	HBRUSH		hBrush;
	HFONT		hof;
	TCHAR		text[256];
	COLORREF	bbkg, bfrg;
	SIZE		size;
	Bcalc*		pc;
	int			pushed;
	int			x, y, id;
	int			textlen;
	bool		isdark = true;

	pb = (PBCBUTTON)GetWindowLong(hWnd, GWL_USERDATA);

	switch(message)
	{
	case WM_PAINT:

		pc = (Bcalc*)GetWindowLong(hWnd, 0);
		isdark = pc ? pc->m_isdark : true;

		hdc = BeginPaint(hWnd, &ps);
		GetWindowText(hWnd, text, 256);

		bfrg = isdark ? RGB(0,0,0) : RGB(90, 90, 90);

		switch(pb->m_class)
		{
		case bcDigit:
			bbkg = isdark ? RGB(100,200,255) : RGB(180,255,255);
			break;
		case bcUnOperator:
			bbkg = isdark ? RGB(240,240,160) : RGB(250,250,235);
			break;
		case bcBinOperator:
			bbkg = isdark ? RGB(240,240,160) : RGB(250,250,235);
			break;
		case bcSciTrigOperator:
		case bcSciTrigUnOperator:
			bbkg = isdark ? RGB(255,200,200) : RGB(255,245,245);
			break;
		case bcProgram:
			bbkg = isdark ? RGB(220,255,220) : RGB(245,255,245);
			break;
		case bcModeButton:
			bbkg = isdark ? RGB(220,255,220) : RGB(245,255,245);
			if(pb->m_on)
				bfrg = isdark ? RGB(200, 100, 0) : RGB(245, 180, 80);
			break;
		case bcControl:
			bbkg = isdark ? RGB(255,200,200) : RGB(255,245,245);
			break;
		case bcAppControl:
			bbkg = isdark ? RGB(250,200,250) : RGB(255,245,255);
			break;
		case bcDisplay:
			bbkg = isdark ? RGB(220,220,250) : RGB(245,245,255);
			break;
		default:
			bbkg = isdark ? RGB(220,255,220) : RGB(245,255,245);
			break;
		}
		pushed = pb->m_on; //GetCapture() == hWnd;

		GetClientRect(hWnd, &rc);

		//rc.left;
		//rc.top;
		rc.right--;
		rc.bottom--;

		hBrush = CreateSolidBrush(bbkg);
		FillRect(hdc, &rc, hBrush);

		BoxOutline(hdc, &rc, pushed);

		id = GetWindowLong(hWnd, GWL_ID);

		if(pc)
		{
			if((rc.right-rc.left) < ((pb->m_w > BUT_DIG_W) ? 55 : 26) || ((rc.bottom-rc.top) < 16))
				hof = (HFONT)SelectObject(hdc, pc->m_hf_c);
			else
				hof = (HFONT)SelectObject(hdc, ((id < 20) ? pc->m_hf_c : pc->m_hf_n));

			textlen = _tcslen(text);
			GetTextExtentPoint32(hdc, text, textlen, &size);
			x = (rc.right - rc.left - size.cx + 1) / 2;
			if(x < 0) x = 0;
			y = (rc.bottom - rc.top - size.cy + 1) / 2;
			if(y < 0) y = 0;

			SetBkColor(hdc, bbkg);
			SetTextColor(hdc, IsWindowEnabled(hWnd) ? bfrg : RGB(163,163,163));
			TextOut(hdc, x, y, text, textlen);
			DeleteObject(hBrush);
			SelectObject(hdc, hof);
		}
		EndPaint(hWnd, &ps);
		if(! GetCapture() && pb->m_on && pb->m_class != bcModeButton)
		{
			pb->m_on = 0;
			InvalidateRect(hWnd, NULL, TRUE);
		}
		break;

	case WM_LBUTTONDOWN:

		SetCapture(hWnd);
		if(pb->m_class != bcModeButton)
		{
			pb->m_on = 1;
		}
		else
		{
			pb->m_on = ~ pb->m_on;
		}
		InvalidateRect(hWnd, NULL, TRUE);
		PostMessage(GetParent(hWnd), WM_COMMAND, GetWindowLong(hWnd, GWL_ID), 0);
		break;

	case WM_LBUTTONUP:

		ReleaseCapture();
		if(pb->m_class != bcModeButton)
		{
			pb->m_on = 0;
		}
		InvalidateRect(hWnd, NULL, TRUE);
		break;

	case WM_CREATE:

		// creation, copy cookie into userdata as "this"
		{
			LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;

			SetWindowLong(hWnd, GWL_USERDATA, (LONG)pcs->lpCreateParams);

			pb = (PBCBUTTON)GetWindowLong(hWnd, GWL_USERDATA);

		}
		// 	FALL into default
	default:

		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

//**************************************************************************
void Bcalc::OnDrawDisplay(HDC hdc, HWND hWnd, LPRECT prc)
{
	LPCSTR	pt;
	LPSTR   pe;
	int		i, yo, xo, rm;
	int		len, size, width, solid;
	int		explen;
	int		dpnt;
	CODE	dc;

	if(hWnd == m_hwndDisplay || hWnd == m_hwndXdisplay)
	{
		if(m_state == kbError)
		{
			pt = "?";
			len = 1;
		}
		else
		{
			if(GetTop() && m_state == kbBlank)
			{
				if(hWnd == m_hwndDisplay)
				{
					pt   = GetTop()->Format(256, m_radix, m_dispformat);
				}
				else
				{
					pt   = GetTop()->Format(256, m_xradix, m_dispformat);
				}
				len  = strlen(pt);
			}
			else if(m_cursor > 0)
			{
				if(hWnd == m_hwndDisplay)
				{
					pt  = GetStage();
					len = m_cursor;
				}
				else
				{
					Bnumber* px = MakeNumber(m_stage, m_radix, ID_OPR_NOOP);

					pt   = px->Format(256, m_xradix, m_dispformat);
					len  = strlen(pt);
					delete px;
				}
			}
			else
			{
				if(m_mathformat == kbInteger)
				{
					pt = "0";
					len = 1;
				}
				else
				{
					pt = "0.";
					len = 2;
				}
			}
		}
		// extract exponent
		//
		pe = strstr((LPSTR)pt, "e");
		if(pe)
		{
			CHAR* pev;
			bool isneg = false;
			long ev;

			len = pe - pt;
			pe++;
			if(*pe == '-')
			{
				isneg = true;
				pe++;
			}
			if(*pe == '+')
				pe++;
			for(pev = pe; *pev == '0';)
				pev++;
			ev = strtol(pev, NULL, 0);
			if(ev == 0)
			{
				if(m_state == kbExponent)
				{
					pe = (char*)"0";
					explen = 1;
				}
				else
				{
					explen = 0;
				}
			}
			else
			{
				if(isneg) pe--;
				explen = strlen(pe);
			}
		}
		else
		{
			explen   = 0;
		}
		if(hWnd == m_hwndDisplay)
			size = 3;
		else
			size = 2;

		// see if the whole number fits and if not, drop size of digits
		// until it does (or size gets to smallest)
		//
		do
		{
			width = (14 * (size)) / 2;
			if((width * (len + explen + (explen ? 1 : 0)) + 6) > (prc->right - prc->left))
				size--;
			else
				break;
		}
		while(size > 1);

		width = (14 * (size)) / 2;
		solid = size < 2;

		if(hWnd == m_hwndDisplay)
			yo = 2 + (3 - size)*6;
		else
			yo = 2 + (2 - size)*5;

		rm = ((size > 1 && ! explen) ? 8 : 4) + (explen + (explen ? 1 : 0)) * width;

		for(i = len-1, xo = prc->right - prc->left - width - rm; i >= 0; i--)
		{
			if(pt[i] == '.')
			{
				i--;
				dpnt = 1;
				if(i < 0)
					dc = '0';
				else
					dc = pt[i];
			}
			else
			{
				dpnt = 0;
				dc = pt[i];
			}
			DrawDigit(hdc, xo, yo, size, solid, dc, dpnt, 0, m_hbrDigit1, m_hbrDigit2);
			xo -= width;
		}
		if(explen > 0)
		{
			rm = 4;
			for(i = explen-1, xo = prc->right - prc->left - rm - width; i >= 0; i--)
			{
				DrawDigit(hdc, xo, yo, size, solid, pe[i], 0, 0, m_hbrDigit1, m_hbrDigit2);
				xo -= width;
			}
		}

	}
	else if(hWnd == m_hwndXinfo)
	{
		HFONT  hof;
		LPCTSTR rs;

		hof = (HFONT)SelectObject(hdc, m_hf_c);
		SetBkColor(hdc, m_dispBkg);
		SetTextColor(hdc, RGB(50,230,50));

		switch(m_radix)
		{
		case 2:  rs = _T("BIN"); break;
		case 8:  rs = _T("OCT"); break;
		case 10: rs = _T("DEC"); break;
		case 16: rs = _T("HEX"); break;
		}
		TextOut(hdc,4,2,rs,3);
		switch(m_xradix)
		{
		case 2:  rs = _T("BIN"); break;
		case 8:  rs = _T("OCT"); break;
		case 10: rs = _T("DEC"); break;
		case 16: rs = _T("HEX"); break;
		}
		TextOut(hdc,4,BCALC_XDISP_H / 2,rs,3);
		SelectObject(hdc, hof);
	}
	else if(hWnd == m_hwndFinfo)
	{
		HFONT  hof;
		LPCTSTR rs;

		hof = (HFONT)SelectObject(hdc, m_hf_c);
		SetBkColor(hdc, m_dispBkg);
		SetTextColor(hdc, RGB(50,230,50));

		switch(m_mathformat)
		{
		case kbInteger:			rs = _T("Int "); break;
		case kbUnsignedInteger:	rs = _T("Uint"); break;
		case kbFloat:			rs = _T("Dbl "); break;
		case kbArbitrary:		rs = _T("Arb "); break;
		}
		TextOut(hdc,6,2,rs,4);
		switch(m_dispformat)
		{
		case kbDefault:		rs = _T("   "); break;
		case kbScientific:  rs = _T("Sci"); break;
		case kbEngineer:	rs = _T("Eng"); break;
		}
		TextOut(hdc,6,BCALC_XDISP_H / 2,rs,3);
		SelectObject(hdc, hof);
	}
}

//**************************************************************************
ERRCODE Bcalc::SetupWindows(bool docreate)
{
	RECT rc;
	int  x, y, w, h;
	int  ix, iy, bx, by;
	int  aw, ah;
	int  max_h;
	int	 enabled, wasenabled;

	if(! m_hwnd) return errFAILURE;

	GetParentPanel()->GetPanelClientRect(&rc);
	aw = rc.right - rc.left;
	ah = rc.bottom - rc.top;

	x = BCALC_DISP_LM;
	y = BCALC_DISP_TM;
	w = aw - 2 * BCALC_DISP_LM;
	h = BCALC_DISP_H;

	// main digit display
	//
	if(docreate)
	{
		m_hwndDisplay =
					CreateWindow(
								_T("bcalc_disp"),
								_T("display"),
								WS_CHILD,
								x, y, w, h,
								GetWindow(),
								NULL,
								GetParentPanel()->GetInstance(),
								(LPVOID)this
								);
		ShowWindow(m_hwndDisplay, SW_SHOW);
	}
	else
	{
		MoveWindow(m_hwndDisplay, x, y, w, h, FALSE);
	}
	// sub radix display
	//
	x = BCALC_XDISP_X;
	y = BCALC_DISP_H + BCALC_DISP_TM;
	w = aw - BCALC_XDISP_X - BCALC_DISP_LM;
	h = BCALC_XDISP_H;

	if(docreate)
	{
		m_hwndXdisplay =
					CreateWindow(
								_T("bcalc_disp"),
								_T("xdisplay"),
								WS_CHILD,
								x, y, w, h,
								GetWindow(),
								NULL,
								GetParentPanel()->GetInstance(),
								(LPVOID)this
								);
		ShowWindow(m_hwndXdisplay, SW_SHOW);
	}
	else
	{
		MoveWindow(m_hwndXdisplay, x, y, w, h, TRUE);
	}
	// radix / focus control buttons
	//
	x  = BCALC_DISP_LM + BCALC_XDISP_X / 2;
	y  = BCALC_DISP_H + BCALC_DISP_TM;
	bx = BCALC_XDISP_X - BCALC_RADBUT_W;
	by = BCALC_DISP_H + BCALC_DISP_TM | BCALC_RADBUT_H;
	w  = BCALC_RADBUT_W;
	h  = BCALC_RADBUT_H;

	if(docreate)
	{
		m_hwnd_drup =
					CreateWindow(
								_T("bcalc_button"),
								_T(">"),
								WS_CHILD,
								bx, y, w, h,
								GetWindow(),
								NULL,
								GetParentPanel()->GetInstance(),
								(LPVOID)&g_ctrls[0]
								);
		SetWindowLong(m_hwnd_drup, GWL_ID, ID_RADIX_UP);
		SetWindowLong(m_hwnd_drup, 0, (LONG)this);
		enabled    = IsLegalKey(ID_RADIX_UP, m_focus == kbDisplay ? m_radix : m_xradix, m_mathformat);
		EnableWindow(m_hwnd_drup, enabled);
		ShowWindow(m_hwnd_drup, SW_SHOW);
	}
	else
	{
		wasenabled = IsWindowEnabled(m_hwnd_drup);
		enabled    = IsLegalKey(ID_RADIX_UP, m_focus == kbDisplay ? m_radix : m_xradix, m_mathformat);
		EnableWindow(m_hwnd_drup, enabled);
		if(enabled != wasenabled)
			InvalidateRect(m_hwnd_drup, NULL, FALSE);
		MoveWindow(m_hwnd_drup, bx, y, w, h, TRUE);
	}

	if(docreate)
	{
		m_hwnd_drdn =
					CreateWindow(
								_T("bcalc_button"),
								_T("<"),
								WS_CHILD,
								x, y, w, h,
								GetWindow(),
								NULL,
								GetParentPanel()->GetInstance(),
								(LPVOID)&g_ctrls[1]
								);
		SetWindowLong(m_hwnd_drdn, GWL_ID, ID_RADIX_DOWN);
		SetWindowLong(m_hwnd_drdn, 0, (LONG)this);
		enabled    = IsLegalKey(ID_RADIX_DOWN, m_focus == kbDisplay ? m_radix : m_xradix, m_mathformat);
		EnableWindow(m_hwnd_drdn, enabled);
		ShowWindow(m_hwnd_drdn, SW_SHOW);
	}
	else
	{
		wasenabled = IsWindowEnabled(m_hwnd_drdn);
		enabled    = IsLegalKey(ID_RADIX_DOWN, m_focus == kbDisplay ? m_radix : m_xradix, m_mathformat);
		EnableWindow(m_hwnd_drdn, enabled);
		if(enabled != wasenabled)
			InvalidateRect(m_hwnd_drdn, NULL, FALSE);
		MoveWindow(m_hwnd_drdn, x, y, w, h, TRUE);
	}
	if(docreate)
	{
		m_hwnd_xrup =
					CreateWindow(
								_T("bcalc_button"),
								_T(">"),
								WS_CHILD,
								bx, by, w, h,
								GetWindow(),
								NULL,
								GetParentPanel()->GetInstance(),
								(LPVOID)&g_ctrls[2]
								);
		SetWindowLong(m_hwnd_xrup, GWL_ID, ID_XRADIX_UP);
		SetWindowLong(m_hwnd_xrup, 0, (LONG)this);
		enabled    = IsLegalKey(ID_XRADIX_UP, m_focus == kbDisplay ? m_radix : m_xradix, m_mathformat);
		EnableWindow(m_hwnd_xrup, enabled);
		ShowWindow(m_hwnd_xrup, SW_SHOW);
	}
	else
	{
		wasenabled = IsWindowEnabled(m_hwnd_xrup);
		enabled    = IsLegalKey(ID_XRADIX_UP, m_focus == kbDisplay ? m_radix : m_xradix, m_mathformat);
		EnableWindow(m_hwnd_xrup, enabled);
		if(enabled != wasenabled)
			InvalidateRect(m_hwnd_xrup, NULL, FALSE);
		MoveWindow(m_hwnd_xrup, bx, by, w, h, TRUE);
	}
	if(docreate)
	{
		m_hwnd_xrdn =
					CreateWindow(
								_T("bcalc_button"),
								_T("<"),
								WS_CHILD,
								x, by, w, h,
								GetWindow(),
								NULL,
								GetParentPanel()->GetInstance(),
								(LPVOID)&g_ctrls[3]
								);
		SetWindowLong(m_hwnd_xrdn, GWL_ID, ID_XRADIX_DOWN);
		SetWindowLong(m_hwnd_xrdn, 0, (LONG)this);
		enabled    = IsLegalKey(ID_XRADIX_DOWN, m_focus == kbDisplay ? m_radix : m_xradix, m_mathformat);
		EnableWindow(m_hwnd_xrdn, enabled);
		ShowWindow(m_hwnd_xrdn, SW_SHOW);
	}
	else
	{
		wasenabled = IsWindowEnabled(m_hwnd_xrdn);
		enabled    = IsLegalKey(ID_XRADIX_DOWN, m_focus == kbDisplay ? m_radix : m_xradix, m_mathformat);
		EnableWindow(m_hwnd_xrdn, enabled);
		if(enabled != wasenabled)
			InvalidateRect(m_hwnd_xrdn, NULL, FALSE);
		MoveWindow(m_hwnd_xrdn, x, by, w, h, TRUE);
	}
	// num format up/down
	//
	x  = BCALC_DISP_LM;
	y  = BCALC_DISP_H + BCALC_DISP_TM;
	bx = BCALC_DISP_LM + BCALC_XDISP_X / 2 - BCALC_RADBUT_W;
	by = BCALC_DISP_H + BCALC_DISP_TM | BCALC_RADBUT_H;
	w  = BCALC_RADBUT_W;
	h  = BCALC_RADBUT_H;

	if(docreate)
	{
		m_hwnd_nfrmtup =
					CreateWindow(
								_T("bcalc_button"),
								_T(">"),
								WS_CHILD,
								bx, y, w, h,
								GetWindow(),
								NULL,
								GetParentPanel()->GetInstance(),
								(LPVOID)&g_ctrls[4]
								);
		SetWindowLong(m_hwnd_nfrmtup, GWL_ID, ID_FORMAT_UP);
		SetWindowLong(m_hwnd_nfrmtup, 0, (LONG)this);
		enabled    = IsLegalKey(ID_FORMAT_UP, m_focus == kbDisplay ? m_radix : m_xradix, m_mathformat);
		EnableWindow(m_hwnd_nfrmtup, enabled);
		ShowWindow(m_hwnd_nfrmtup, SW_SHOW);
	}
	else
	{
		wasenabled = IsWindowEnabled(m_hwnd_nfrmtup);
		enabled    = IsLegalKey(ID_FORMAT_UP, m_focus == kbDisplay ? m_radix : m_xradix, m_mathformat);
		EnableWindow(m_hwnd_nfrmtup, enabled);
		if(enabled != wasenabled)
			InvalidateRect(m_hwnd_nfrmtup, NULL, FALSE);
		MoveWindow(m_hwnd_nfrmtup, bx, y, w, h, TRUE);
	}
	if(docreate)
	{
		m_hwnd_nfrmtdn =
					CreateWindow(
								_T("bcalc_button"),
								_T("<"),
								WS_CHILD,
								x, y, w, h,
								GetWindow(),
								NULL,
								GetParentPanel()->GetInstance(),
								(LPVOID)&g_ctrls[5]
								);
		SetWindowLong(m_hwnd_nfrmtdn, GWL_ID, ID_FORMAT_DOWN);
		SetWindowLong(m_hwnd_nfrmtdn, 0, (LONG)this);
		enabled    = IsLegalKey(ID_FORMAT_DOWN, m_focus == kbDisplay ? m_radix : m_xradix, m_mathformat);
		EnableWindow(m_hwnd_nfrmtdn, enabled);
		ShowWindow(m_hwnd_nfrmtdn, SW_SHOW);
	}
	else
	{
		wasenabled = IsWindowEnabled(m_hwnd_nfrmtdn);
		enabled    = IsLegalKey(ID_FORMAT_DOWN, m_focus == kbDisplay ? m_radix : m_xradix, m_mathformat);
		EnableWindow(m_hwnd_nfrmtdn, enabled);
		if(enabled != wasenabled)
			InvalidateRect(m_hwnd_nfrmtdn, NULL, FALSE);
		MoveWindow(m_hwnd_nfrmtdn, x, y, w, h, TRUE);
	}
	// display format up/down
	//
	if(docreate)
	{
		m_hwnd_dfrmtup =
					CreateWindow(
								_T("bcalc_button"),
								_T(">"),
								WS_CHILD,
								bx, by, w, h,
								GetWindow(),
								NULL,
								GetParentPanel()->GetInstance(),
								(LPVOID)&g_ctrls[6]
								);
		SetWindowLong(m_hwnd_dfrmtup, GWL_ID, ID_DFRMT_UP);
		SetWindowLong(m_hwnd_dfrmtup, 0, (LONG)this);
		enabled    = IsLegalKey(ID_DFRMT_UP, m_focus == kbDisplay ? m_radix : m_xradix, m_mathformat);
		EnableWindow(m_hwnd_dfrmtup, enabled);
		ShowWindow(m_hwnd_dfrmtup, SW_SHOW);
	}
	else
	{
		wasenabled = IsWindowEnabled(m_hwnd_dfrmtup);
		enabled    = IsLegalKey(ID_DFRMT_UP, m_focus == kbDisplay ? m_radix : m_xradix, m_mathformat);
		EnableWindow(m_hwnd_dfrmtup, enabled);
		if(enabled != wasenabled)
			InvalidateRect(m_hwnd_dfrmtup, NULL, FALSE);
		MoveWindow(m_hwnd_dfrmtup, bx, by, w, h, TRUE);
	}
	if(docreate)
	{
		m_hwnd_dfrmtdn =
					CreateWindow(
								_T("bcalc_button"),
								_T("<"),
								WS_CHILD,
								x, by, w, h,
								GetWindow(),
								NULL,
								GetParentPanel()->GetInstance(),
								(LPVOID)&g_ctrls[7]
								);
		SetWindowLong(m_hwnd_dfrmtdn, GWL_ID, ID_DFRMT_DOWN);
		SetWindowLong(m_hwnd_dfrmtdn, 0, (LONG)this);
		enabled    = IsLegalKey(ID_DFRMT_DOWN, m_focus == kbDisplay ? m_radix : m_xradix, m_mathformat);
		EnableWindow(m_hwnd_dfrmtdn, enabled);
		ShowWindow(m_hwnd_dfrmtdn, SW_SHOW);
	}
	else
	{
		wasenabled = IsWindowEnabled(m_hwnd_dfrmtdn);
		enabled    = IsLegalKey(ID_DFRMT_DOWN, m_focus == kbDisplay ? m_radix : m_xradix, m_mathformat);
		EnableWindow(m_hwnd_dfrmtdn, enabled);
		if(enabled != wasenabled)
			InvalidateRect(m_hwnd_dfrmtdn, NULL, FALSE);
		MoveWindow(m_hwnd_dfrmtdn, x, by, w, h, TRUE);
	}
	// radix info display
	//
	x = BCALC_DISP_LM + BCALC_XDISP_X / 2 + BCALC_RADBUT_W;
	y = BCALC_DISP_H + BCALC_DISP_TM;
	w = BCALC_XDISP_X / 2 - 2 * BCALC_RADBUT_W;
	h = BCALC_XDISP_H;

	if(docreate)
	{
		m_hwndXinfo =
					CreateWindow(
								_T("bcalc_disp"),
								_T("xinfo"),
								WS_CHILD,
								x, y, w, h,
								GetWindow(),
								NULL,
								GetParentPanel()->GetInstance(),
								(LPVOID)this
								);
		ShowWindow(m_hwndXinfo, SW_SHOW);
	}
	else
	{
		MoveWindow(m_hwndXinfo, x, y, w, h, TRUE);
	}
	// format info display
	//
	x = BCALC_DISP_LM + BCALC_RADBUT_W;
	y = BCALC_DISP_H + BCALC_DISP_TM;
	w = BCALC_XDISP_X / 2 - 2 * BCALC_RADBUT_W;
	h = BCALC_XDISP_H;

	if(docreate)
	{
		m_hwndFinfo =
					CreateWindow(
								_T("bcalc_disp"),
								_T("finfo"),
								WS_CHILD,
								x, y, w, h,
								GetWindow(),
								NULL,
								GetParentPanel()->GetInstance(),
								(LPVOID)this
								);
		ShowWindow(m_hwndFinfo, SW_SHOW);
	}
	else
	{
		MoveWindow(m_hwndFinfo, x, y, w, h, TRUE);
	}
	// button map
	//
	x = BCALC_BUT_LM;
	y = BCALC_DISP_H + BCALC_DISP_TM + BCALC_XDISP_H + BCALC_BUT_TM;

	for(iy = 0, by = y; iy < (m_view_scitrig ? BUT_MAP_H : (BUT_MAP_H - BUT_MAP_SCITRIG_H)); iy++)
	{
		for(ix = max_h = 0, bx = x; ix < BUT_MAP_W; ix++)
		{
			PBCBUTTON pb;

			pb = &g_buttons[iy][ix];
			w = pb->m_w;
			h = pb->m_h;

			w = (pb->m_w * aw) / (BUT_DIG_W * (BUT_MAP_W + 1));
			h = (pb->m_h * (ah - y)) / (BUT_DIG_H * (m_view_scitrig ? BUT_MAP_H : (BUT_MAP_H - BUT_MAP_SCITRIG_H)));
			if(w < BCALC_BUT_MIN_W)
				w = BCALC_BUT_MIN_W;
			if(w > BCALC_BUT_MAX_W)
				w = BCALC_BUT_MAX_W;
			if(h < BCALC_BUT_MIN_H)
				h = BCALC_BUT_MIN_H;
			if(h > BCALC_BUT_MAX_H)
				h = BCALC_BUT_MAX_H;
			w = w - BCALC_BUT_MX;
			h = h - BCALC_BUT_MY;

			if(pb->m_id)
			{
				if(docreate || ! pb->m_hwnd)
				{
					pb->m_hwnd = CreateWindow(
											_T("bcalc_button"),
											pb->m_name,
											WS_CHILD,
											bx, by, w, h,
											GetWindow(),
											NULL,
											GetParentPanel()->GetInstance(),
											(LPVOID)pb
											);
					SetWindowLong(pb->m_hwnd, GWL_ID, pb->m_id);
					SetWindowLong(pb->m_hwnd, 0, (LONG)this);
					enabled    = IsLegalKey(pb->m_id, m_focus == kbDisplay ? m_radix : m_xradix, m_mathformat);
					EnableWindow(pb->m_hwnd, enabled);
					ShowWindow(pb->m_hwnd, SW_SHOW);
				}
				else if(pb->m_hwnd)
				{
					MoveWindow(pb->m_hwnd, bx, by, w, h, TRUE);

					wasenabled = IsWindowEnabled(pb->m_hwnd);
					enabled    = IsLegalKey(pb->m_id, m_focus == kbDisplay ? m_radix : m_xradix, m_mathformat);
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
	for(iy = BUT_MAP_H - BUT_MAP_SCITRIG_H; iy < BUT_MAP_H; iy++)
	{
		for(ix = 0; ix < BUT_MAP_W; ix++)
		{
			PBCBUTTON pb;

			pb = &g_buttons[iy][ix];
			if(pb->m_hwnd)
			{
				enabled    = IsLegalKey(pb->m_id, m_focus == kbDisplay ? m_radix : m_xradix, m_mathformat);
				EnableWindow(pb->m_hwnd, enabled);
				ShowWindow(pb->m_hwnd, m_view_scitrig ? SW_SHOW : SW_HIDE);
			}
		}
	}
	if(docreate)
	{
		if(m_view_program && ! m_progpanel)
		{
			m_view_program = false;
			OnKey(ID_CTL_PRG);
			if(m_progpanel && m_progpanel->GetPanes())
			{
				SetupWindows(false);
				m_progpanel->GetPanes()->FitToPanel();
			}
			if(m_plotpanel && m_plotpanel->GetPanes())
			{
				m_plotpanel->GetPanes()->FitToPanel();
			}
		}
	}
	else
	{
		UpdateDisplay(NULL);
	}
	return errOK;
}

//**************************************************************************
Bnumber* Bcalc::GetTop(void)
{
	return m_ps->GetTop();
}

//**************************************************************************
void Bcalc::FitToPanel(void)
{
	if(! m_hwnd)
	{
		RECT		rc;
		WNDCLASS	wc;

		// register class for app window
		//
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= (WNDPROC)AppProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= GetInstance();
		wc.hIcon			= NULL;
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)(intptr_t)(m_scAppBkg+1);
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= _T("bcalc_pane");

		ATOM aViewClass = RegisterClass(&wc);

		// register class for numeric display window
		//
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= (WNDPROC)DispProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= GetInstance();
		wc.hIcon			= NULL;
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= m_hbrDisplay;
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= _T("bcalc_disp");

		ATOM aDispClass = RegisterClass(&wc);

		// register class for keypad buttons
		//
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= (WNDPROC)ButtonProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 4;
		wc.hInstance		= GetInstance();
		wc.hIcon			= NULL;
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)(intptr_t)(m_scAppBkg+1);
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= _T("bcalc_button");

		ATOM aButtClass = RegisterClass(&wc);

		// register class for program window
		//
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= (WNDPROC)BAFframeProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= GetInstance();
		wc.hIcon			= NULL;
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= m_hbrProg;
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= _T("bcalc_program_pane");

		ATOM aProgClass = RegisterClass(&wc);

		extern LRESULT ProgEditProc(HWND, UINT, WPARAM, LPARAM);

		// register class for program list window
		//
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= (WNDPROC)ProgEditProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= GetInstance();
		wc.hIcon			= NULL;
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)(intptr_t)(m_scTextBkg+1);
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= _T("bcalc_program");

		ATOM aProgListClass = RegisterClass(&wc);

		// register class for plot window
		//
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= (WNDPROC)BAFframeProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= GetInstance();
		wc.hIcon			= NULL;
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= m_hbrPlot;
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= _T("bcalc_plot_pane");

		ATOM aPlotClass = RegisterClass(&wc);


		GetClientRect(GetParentPanel()->GetWindow(), &rc);
		m_hwnd = CreateWindow(
								_T("bcalc_pane"),
								_T("bcalc"),
								WS_CHILD | WS_CLIPCHILDREN,
								0, 0, rc.right-rc.left, rc.bottom-rc.top,
								GetParentPanel()->GetWindow(),
								NULL,
								GetParentPanel()->GetInstance(),
								(LPVOID)this
							 );
		SetupWindows(true);
	}
	BappPane::FitToPanel();
	SetupWindows(false);
}

//**************************************************************************
Bnumber* Bcalc::ReformatNumber(Bnumber* pn, NumberFormat from, NumberFormat to)
{
	switch(from)
	{
	case kbInteger:
		switch(to)
		{
		case kbInteger:
			return NULL;
		case kbUnsignedInteger:
			return new BuintNumber(pn->Format(512, 16, kbDefault), 16, pn->m_code);
			break;
		case kbFloat:
			return new BdblNumber(pn->Format(512, 10, kbDefault), 10, pn->m_code);
			break;
		case kbArbitrary:
			return new BarbNumber(pn->Format(512, 10, kbDefault), 10, pn->m_code);
			break;
		}
		break;
	case kbUnsignedInteger:
		switch(to)
		{
		case kbInteger:
			return new BintNumber(pn->Format(512, 16, kbDefault), 16, pn->m_code);
			break;
		case kbUnsignedInteger:
			return NULL;
		case kbFloat:
			return new BdblNumber(pn->Format(512, 10, kbDefault), 10, pn->m_code);
			break;
		case kbArbitrary:
			return new BarbNumber(pn->Format(512, 10, kbDefault), 10, pn->m_code);
			break;
		}
		break;
	case kbFloat:
		switch(to)
		{
		case kbInteger:
			return new BintNumber(pn->Format(512, 16, kbDefault), 16, pn->m_code);
			break;
		case kbUnsignedInteger:
			return new BuintNumber(pn->Format(512, 16, kbDefault), 16, pn->m_code);
			break;
		case kbFloat:
			return NULL;
		case kbArbitrary:
			return new BarbNumber(pn->Format(512, 10, kbDefault), 10, pn->m_code);
			break;
		}
		break;
	case kbArbitrary:
		switch(to)
		{
		case kbInteger:
			return new BintNumber(pn->Format(512, 16, kbDefault), 16, pn->m_code);
			break;
		case kbUnsignedInteger:
			return new BuintNumber(pn->Format(512, 16, kbDefault), 16, pn->m_code);
			break;
		case kbFloat:
			return new BdblNumber(pn->Format(512, 10, kbDefault), 10, pn->m_code);
			break;
		case kbArbitrary:
			return NULL;
		}
		break;
	}
	return NULL;
}
//**************************************************************************
ERRCODE Bcalc::ReformatStack(NumberFormat from, NumberFormat to)
{
	Bnumber* px;
	Bnumber* pn;
	Bstack*  ps;
	CODE	 code;
	int		 i;

	ps = new Bstack();

	while(m_ps->GetTop())
	{
		pn = m_ps->Pop(code);
		px = ReformatNumber(pn, from, to);
		if(! px) return errBAD_PARAMETER;
		ps->Push(px);
		delete pn;
	}
	delete m_ps;
	m_ps = ps;

	for(i = 0; i < (sizeof(m_mem) / sizeof(Bnumber*)); i++)
	{
		pn = m_mem[i];

		if(pn)
		{
			m_mem[i] = ReformatNumber(pn, from, to);
			delete pn;
		}
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bcalc::Record(Bnumber* pn)
{
	Bnumber* px;
	Bprog*   pp;

	if(m_progmode != pmRecord)
		return errOK;

	if(! m_progpanel)
		return errOK;

	pp = (Bprog*)m_progpanel->FindPane(_T("Program"));
	if(! pp)
		return errOK;

	px = MakeNumber(pn->Format(512, m_radix, kbDefault), m_radix, pn->m_code);
	return pp->Insert(px);
}

//**************************************************************************
ERRCODE Bcalc::Record(CODE o)
{
	Bprog*   pp;

	if(m_progmode != pmRecord)
		return errOK;

	if(! m_progpanel)
		return errOK;

	pp = (Bprog*)m_progpanel->FindPane(_T("Program"));
	if(! pp)
		return errOK;

	return pp->Insert(o);
}

//**************************************************************************
ERRCODE Bcalc::Play(Bnumber* pn)
{
	Bnumber* px;

	if(m_progmode != pmRun)
		return errBAD_PARAMETER;

	px = MakeNumber(pn->Format(512, m_radix, kbDefault), m_radix, ID_OPR_EQUAL);
	m_ps->Push(px);
	return errOK;
}

//**************************************************************************
ERRCODE Bcalc::Play(CODE o)
{
	return OnKey(o);
}


//**************************************************************************
Bnumber* Bcalc::MakeNumber(LPCSTR digits, int radix, CODE code)
{
	switch(m_mathformat)
	{
	case kbInteger:
		return new BintNumber(digits, radix, code);

	case kbUnsignedInteger:
		return new BuintNumber(digits, radix, code);

	case kbFloat:
		return new BdblNumber(digits, radix, code);

	case kbArbitrary:
		return new BarbNumber(digits, radix, code);
	}
	return NULL;
}

//**************************************************************************
ERRCODE Bcalc::CommitStage(CODE code)
{
	if(m_state == kbNumber || m_state == kbFraction || m_state == kbExponent)
	{
		Bnumber* pn = MakeNumber(m_stage, m_radix, code);

		m_ps->Push(pn);
		Record(pn);
		m_state  = kbBlank;
		m_cursor = 0;
		return errOK;
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bcalc::StageDigit(CHAR d)
{
	if(m_state == kbNumber || m_state == kbFraction || m_state == kbBlank)
	{
		if(m_cursor < (BC_MAX_STAGE-1))
		{
			if(m_state == kbBlank)
			{
				if(m_ps->GetTop() && m_ps->GetTop()->m_code == ID_OPR_EQUAL)
					m_ps->Clear();
			}
			if(
					(m_cursor == 1 && m_stage[0] == '0')
				||	(m_cursor == 2 && m_stage[0] == '-' && m_stage[1] == '0')
			)
			{
				if(d != ID_CTL_DEC)
				{
					m_cursor--;
				}
			}
			if(d == ID_CTL_DEC)
				m_state	= kbFraction;
			else if (m_state == kbBlank)
				m_state = kbNumber;

			m_stage[m_cursor++] = d;
			m_stage[m_cursor]   = 0;
			UpdateDisplay(NULL);
		}
		else
		{
			return errOVERFLOW;
		}
		return errOK;
	}
	else if(m_state == kbExponent)
	{
		if(m_expcursor < (BC_MAX_STAGE-1))
		{
			if(d == ID_NUM_0 && m_cursor == 0)
			{
				return errOK;
			}
			m_stage[m_expcursor++] = d;
			m_stage[m_expcursor]   = 0;
			UpdateDisplay(NULL);
		}
		else
		{
			return errOVERFLOW;
		}
		return errOK;
	}
	return errFAILURE;
}

//**************************************************************************
int Bcalc::Precedence(CODE o)
{
	switch(o)
	{
	case ID_OPR_ADD:	return 3;
	case ID_OPR_SUB:	return 3;
	case ID_OPR_MUL:	return 5;
	case ID_OPR_DIV:	return 5;
	case ID_OPR_MOD:	return 5;
	case ID_OPR_AND:	return 4;
	case ID_OPR_OR:		return 3;
	case ID_OPR_XOR:	return 3;
	default:
		return 0;
	}
}

//**************************************************************************
void Bcalc::UpdateDisplay(HWND hWnd)
{
	if(m_progmode == pmRun)
		return;

	if(hWnd)
	{
		InvalidateRect(hWnd, NULL, TRUE);
	}
	InvalidateRect(m_hwndDisplay, NULL, TRUE);
	InvalidateRect(m_hwndXdisplay, NULL, TRUE);

}

//**************************************************************************
void Bcalc::GetButtonSize(int* pw, int* ph)
{
	RECT rc;
	int  aw, ah;
	int  x, y;
	int  w = 0, h = 0;

	x = BCALC_BUT_LM;
	y = BCALC_DISP_H + BCALC_DISP_TM + BCALC_XDISP_H + BCALC_BUT_TM;

	if(m_hwnd && GetParentPanel())
	{
		GetParentPanel()->GetPanelClientRect(&rc);
		aw = rc.right - rc.left;
		ah = rc.bottom - rc.top;

		w = (aw) / ((BUT_MAP_W + 1));
		h = ((ah - y)) / ((m_view_scitrig ? BUT_MAP_H : (BUT_MAP_H - BUT_MAP_SCITRIG_H)));
		if(w < BCALC_BUT_MIN_W)
			w = BCALC_BUT_MIN_W;
		if(w > BCALC_BUT_MAX_W)
			w = BCALC_BUT_MAX_W;
		if(h < BCALC_BUT_MIN_H)
			h = BCALC_BUT_MIN_H;
		if(h > BCALC_BUT_MAX_H)
			h = BCALC_BUT_MAX_H;
		//w = w - BCALC_BUT_MX;
		//h = h - BCALC_BUT_MY;
	}
	if(pw) *pw = w;
	if(ph) *ph = h;
}
//**************************************************************************
bool Bcalc::IsLegalKey(CODE id, int radix, NumberFormat format)
{
	switch(id)
	{
	case ID_NUM_0:
	case ID_NUM_1:
		return true;

	case ID_NUM_2:
	case ID_NUM_3:
	case ID_NUM_4:
	case ID_NUM_5:
	case ID_NUM_6:
	case ID_NUM_7:
		return radix > 2;

	case ID_NUM_8:
	case ID_NUM_9:
		return radix > 8;

	case ID_NUM_A:
	case ID_NUM_B:
	case ID_NUM_C:
	case ID_NUM_D:
	case ID_NUM_E:
	case ID_NUM_F:
		return radix > 10;

	case ID_OPR_ADD :
	case ID_OPR_SUB:
	case ID_OPR_MUL:
	case ID_OPR_DIV:
		return true;

	case ID_OPR_NEG:
		return format != kbUnsignedInteger;

	case ID_OPR_MOD:
	case ID_OPR_AND:
	case ID_OPR_OR:
	case ID_OPR_XOR:
	case ID_OPR_NOT:
	case ID_OPR_SHFTL:
	case ID_OPR_SHFTR:
		return format == kbInteger || format == kbUnsignedInteger;

	case ID_OPR_EQUAL:
	case ID_OPR_LPAREN:
	case ID_OPR_RPAREN:
		return true;

	case ID_OPR_INT:
	case ID_OPR_FRAC:
	case ID_OPR_SQRT:
		return format == kbFloat;

	case ID_OPR_SQ:
	case ID_OPR_INV:
	case ID_OPR_XEXPY:
	case ID_OPR_NOOP:
		return true;

	case ID_CTL_DEC:
	case ID_CTL_EXP:
		return format == kbFloat && m_state != kbExponent;

	case ID_CTL_CLR:
	case ID_CTL_CLRA:
	case ID_CTL_DEL:
	case ID_CTL_STR:
	case ID_CTL_RCL:
	case ID_CTL_CONST:
	case ID_CTL_PRG:
	case ID_CTL_SCI:
	case ID_CTL_QUIT:
	case ID_CTL_COPY:
	case ID_CTL_PASTE:
		return true;

	case ID_OPR_SIN:
	case ID_OPR_COS:
	case ID_OPR_TAN:
	case ID_OPR_ASIN:
	case ID_OPR_ACOS:
	case ID_OPR_ATAN:
	case ID_OPR_EXPO:
	case ID_OPR_NLOG:
	case ID_OPR_10EXP:
	case ID_OPR_LOG:
		return format == kbFloat;

	case ID_RADIX_UP:
	case ID_RADIX_DOWN:
	case ID_XRADIX_UP:
	case ID_XRADIX_DOWN:
	case ID_FORMAT_UP:
	case ID_FORMAT_DOWN:
	case ID_DFRMT_UP:
	case ID_DFRMT_DOWN:
		return m_progmode != pmRecord;
	}
	return true;
}

//**************************************************************************
LPCTSTR Bcalc::MemMenuStr(int id, LPCTSTR label, LPTSTR buf, DWORD nbuf)
{
	TCHAR	numstr[64];
	LPCTSTR pnum;

	if(id < ID_MEM_X || id >= ID_MEM_X + (sizeof(m_mem)/sizeof(Bnumber*)))
	{
		return _T("range err");
	}
	if(m_mem[id - ID_MEM_X] != NULL)
	{
		const char* pn = m_mem[id - ID_MEM_X]->Format(32, m_radix, m_dispformat);

#ifdef UNICODE
		BUtil::Utf8Decode(numstr, pn);
#else
		strcpy(numstr, pn);
#endif
		pnum = numstr;
	}
	else
	{
		pnum = _T("empty");
	}
	_sntprintf(
				buf,
				nbuf,
				_T("" _Pfs_ " " _Pfs_ ""),
				label,
				pnum
			  );
	return buf;
}

//**************************************************************************
ERRCODE Bcalc::Store(Bnumber* pn, int id)
{
	if(id < ID_STR_X || id > ID_STR_W)
		return errBAD_PARAMETER;
	if(m_mem[id - ID_STR_X])
		delete m_mem[id - ID_STR_X];
	pn = MakeNumber(pn ? pn->Format(512, m_radix, kbDefault) : "0", m_radix, ID_OPR_EQUAL);
	m_mem[id - ID_STR_X] = pn;
	return errOK;
}

//**************************************************************************
ERRCODE Bcalc::Recall(Bnumber** pn, int id)
{
	if(id < ID_RCL_X || id > ID_RCL_W || ! pn)
		return errBAD_PARAMETER;
	if(m_mem[id - ID_RCL_X])
		*pn = MakeNumber(m_mem[id - ID_RCL_X]->Format(512, m_radix, kbDefault), m_radix, ID_OPR_EQUAL);
	else
		*pn = MakeNumber("0", m_radix, ID_OPR_EQUAL);
	return errOK;
}

//**************************************************************************
ERRCODE Bcalc::DoMemAccess(CODE dc)
{
	HMENU	hMenu;
	int		rc;
	TCHAR	mbuf[128];
	POINT   mousepos;

	GetCursorPos(&mousepos);

	hMenu = CreatePopupMenu();

	AppendMenu(hMenu, 0, ID_MEM_X, MemMenuStr(ID_MEM_X, _T("X"), mbuf, 128));
	AppendMenu(hMenu, 0, ID_MEM_Y, MemMenuStr(ID_MEM_Y, _T("Y"), mbuf, 128));
	AppendMenu(hMenu, 0, ID_MEM_Z, MemMenuStr(ID_MEM_Z, _T("Z"), mbuf, 128));

	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

	AppendMenu(hMenu, 0, ID_MEM_A, MemMenuStr(ID_MEM_A, _T("A"), mbuf, 128));
	AppendMenu(hMenu, 0, ID_MEM_B, MemMenuStr(ID_MEM_B, _T("B"), mbuf, 128));
	AppendMenu(hMenu, 0, ID_MEM_C, MemMenuStr(ID_MEM_C, _T("C"), mbuf, 128));
	AppendMenu(hMenu, 0, ID_MEM_D, MemMenuStr(ID_MEM_D, _T("D"), mbuf, 128));
	AppendMenu(hMenu, 0, ID_MEM_E, MemMenuStr(ID_MEM_E, _T("E"), mbuf, 128));
	AppendMenu(hMenu, 0, ID_MEM_F, MemMenuStr(ID_MEM_F, _T("F"), mbuf, 128));
	AppendMenu(hMenu, 0, ID_MEM_G, MemMenuStr(ID_MEM_G, _T("G"), mbuf, 128));
	AppendMenu(hMenu, 0, ID_MEM_H, MemMenuStr(ID_MEM_H, _T("H"), mbuf, 128));
	AppendMenu(hMenu, 0, ID_MEM_I, MemMenuStr(ID_MEM_I, _T("I"), mbuf, 128));
	AppendMenu(hMenu, 0, ID_MEM_J, MemMenuStr(ID_MEM_J, _T("J"), mbuf, 128));
	AppendMenu(hMenu, 0, ID_MEM_K, MemMenuStr(ID_MEM_K, _T("K"), mbuf, 128));
#ifdef Windows
	SetForegroundWindow(m_hwnd);
#endif

	rc = TrackPopupMenu(
						hMenu,
						TPM_RETURNCMD,
						mousepos.x,
						mousepos.y,
						0,
						m_hwnd,
						NULL
					   );

	DestroyMenu(hMenu);

	if(rc >= ID_MEM_X && rc <= ID_MEM_W)
	{
		rc -= ID_MEM_X;

		if(dc == ID_CTL_RCL)
			rc += ID_RCL_X;
		else
			rc += ID_STR_X;
		return OnKey(rc);
	}
	return errFAILURE;
}

//**************************************************************************
void Bcalc::AddConstant(LPCTSTR pcv)
{
	PBCALC_CV xcv;
	PBCALC_CV ncv = new BCALC_CV(pcv);

	if(m_constants)
	{
		for(xcv = m_constants; xcv->m_next; xcv = xcv->m_next)
			;
		xcv->m_next = ncv;
	}
	else
	{
		m_constants = ncv;
	}
}

//**************************************************************************
void Bcalc::AddConversion(LPCTSTR pcv)
{
	PBCALC_CV xcv;
	PBCALC_CV ncv = new BCALC_CV(pcv);

	if(m_conversions)
	{
		for(xcv = m_conversions; xcv->m_next; xcv = xcv->m_next)
			;
		xcv->m_next = ncv;
	}
	else
	{
		m_conversions = ncv;
	}
}

//**************************************************************************
ERRCODE Bcalc::DoConst()
{
	HMENU	hMenu;
	int		i, j, k, rc;
	POINT   mousepos;
	LPCTSTR ps;
	LPTSTR	pd;
	CHAR    nb[256];
	TCHAR	xb[256];
	ERRCODE	ec;

	PBCALC_CV pcv;

	GetCursorPos(&mousepos);

	hMenu = CreatePopupMenu();

	for(i = 0, pcv = m_constants; pcv; i++, pcv = pcv->m_next)
	{
		AppendMenu(hMenu, 0, i + 1, pcv->m_cv);
	}
	i++;
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	for(j = 0, pcv = m_conversions; pcv; j++, pcv = pcv->m_next)
	{
		_tcscpy(xb, pcv->m_cv);
		for(ps = xb; *ps && *ps != _T(':');)
			ps++;
		*(LPSTR)ps = _T('\0');
		AppendMenu(hMenu, 0, i + j, xb);
	}
#ifdef Windows
	SetForegroundWindow(m_hwnd);
#endif

	rc = TrackPopupMenu(
						hMenu,
						TPM_RETURNCMD,
						mousepos.x,
						mousepos.y,
						0,
						m_hwnd,
						NULL
					   );

	DestroyMenu(hMenu);

	if(rc >= 1 && rc < i)
	{
		for(pcv = m_constants, k = 1; pcv && k < rc; k++)
			pcv = pcv->m_next;
		ps = pcv->m_cv;

		while(*ps && *ps != _T(' '))
			ps++;
		while(*ps == _T(' '))
			ps++;
#ifdef UNICODE
		BUtil::Utf8Encode(nb, ps, false);
#else
		strcpy(nb, ps);
#endif
		m_ps->Push(MakeNumber(nb, 10, ID_OPR_EQUAL));
		UpdateDisplay(NULL);
		return errOK;
	}
	else if(rc >= i && rc < (i + j))
	{
		for(pcv = m_conversions, k = i; pcv && k < rc; k++)
			pcv = pcv->m_next;
		ps = pcv->m_cv;

		while(*ps && *ps != _T(':'))
			ps++;
		while(*ps == _T(':'))
			ps++;
		while(*ps)
		{
			while(*ps == _T(' '))
				ps++;
			pd = xb;
			while(*ps)
			{
				if (*ps != _T(' '))
					*pd++ = *ps++;
				else
					ps++;
			}
			*pd = _T('\0');

			for(pd = xb, ec = errOK; *pd && ec == errOK; pd++)
			{
				ec = OnKey(*pd);
			}
		}
		UpdateDisplay(NULL);
		return errOK;
	}
	return errFAILURE;
}
//**************************************************************************
ERRCODE Bcalc::OnKey(CODE dc)
{
	NumberFormat prevformat;
	ERRCODE		 ec = errOK;
	Bnumber*	 pn;

	//OutputDebugString(Bprog::CodeName(dc));

	if(m_state == kbError)
	{
		if(dc != ID_CTL_CLR && dc != ID_CTL_CLRA && dc != ID_CTL_QUIT)
			return errBAD_PARAMETER;
	}
	switch(dc)
	{
	case ID_NUM_0:
	case ID_NUM_1:
	case ID_NUM_2:
	case ID_NUM_3:
	case ID_NUM_4:
	case ID_NUM_5:
	case ID_NUM_6:
	case ID_NUM_7:
	case ID_NUM_8:
	case ID_NUM_9:
		return StageDigit((char)dc);

	case ID_NUM_A:
	case ID_NUM_B:
	case ID_NUM_C:
	case ID_NUM_D:
	case ID_NUM_E:
	case ID_NUM_F:
		return StageDigit((char)dc);

	case ID_CTL_DEC:
		if(m_state == kbNumber || m_state == kbBlank)
			return StageDigit((char)dc);
		else if(m_state == kbExponent)
			return errOK;
		else
			return errBAD_PARAMETER;

	case ID_OPR_LPAREN:
		if(m_state == kbBlank)
		{
			Bnumber* pbs = MakeNumber("0", 10, dc);

			Record(dc);
			m_ps->Push(pbs);
			UpdateDisplay(NULL);
			return errOK;
		}
		return errBAD_PARAMETER;

	case ID_OPR_NEG:
		if(m_state == kbExponent)
		{
			int j, k;

			for(
					j = m_expcursor++,
					k = m_expcursor - m_cursor - 1;

					k > 0;

					k--
			)
			{
				m_stage[j] = m_stage[j-1];
			}
			m_stage[m_cursor + 1] = '-';
			m_stage[m_expcursor] = 0;
			UpdateDisplay(NULL);
			break;
		}
		if(m_state != kbBlank)
		{
			CommitStage(dc);
		}
		else
		{
			if(m_ps->GetTop())
			{
				if(m_ps->GetTop()->m_code == ID_OPR_EQUAL)
				{
					m_ps->GetTop()->m_code = dc;
				}
				else
				{
					ec = errFAILURE; // overwrite code!
				}
			}
		}
		Record(dc);
		ec = m_ps->Execute(toTop);
		UpdateDisplay(NULL);
		break;

	case ID_OPR_ADD:
	case ID_OPR_SUB:
	case ID_OPR_MUL:
	case ID_OPR_DIV:
	case ID_OPR_MOD:
	case ID_OPR_AND:
	case ID_OPR_OR:
	case ID_OPR_XOR:
	case ID_OPR_NOT:
	case ID_OPR_SHFTL:
	case ID_OPR_SHFTR:
	case ID_OPR_EQUAL:
	case ID_OPR_RPAREN:
	case ID_OPR_INT:
	case ID_OPR_FRAC:
	case ID_OPR_SQ:
	case ID_OPR_SQRT:
	case ID_OPR_INV:
	case ID_OPR_SIN:
	case ID_OPR_COS:
	case ID_OPR_TAN:
	case ID_OPR_ASIN:
	case ID_OPR_ACOS:
	case ID_OPR_ATAN:
	case ID_OPR_EXPO:
	case ID_OPR_NLOG:
	case ID_OPR_XEXPY:
	case ID_OPR_10EXP:
	case ID_OPR_LOG:
		if(m_state != kbBlank)
		{
			CommitStage(dc);
		}
		else
		{
			if(m_ps->GetTop())
			{
				if(m_ps->GetTop()->m_code == ID_OPR_EQUAL)
				{
					m_ps->GetTop()->m_code = dc;
				}
				else
				{
					ec = errFAILURE; // overwrite code!
				}
			}
			else if(m_progmode != pmRecord)
			{
				// push a 0 for blank displays
				m_ps->Push(MakeNumber("0", 10, dc));
			}
		}
		Record(dc);
		switch(dc)
		{
		case ID_OPR_NEG:
		case ID_OPR_NOT:
		case ID_OPR_INT:
		case ID_OPR_FRAC:
		case ID_OPR_SQ:
		case ID_OPR_SQRT:
		case ID_OPR_INV:
		case ID_OPR_SHFTL:
		case ID_OPR_SHFTR:
		case ID_OPR_SIN:
		case ID_OPR_COS:
		case ID_OPR_TAN:
		case ID_OPR_ASIN:
		case ID_OPR_ACOS:
		case ID_OPR_ATAN:
		case ID_OPR_EXPO:
		case ID_OPR_NLOG:
		case ID_OPR_10EXP:
		case ID_OPR_LOG:
			ec = m_ps->Execute(toTop);
			break;
		case ID_OPR_RPAREN:
			ec = m_ps->Execute(toParen);
			break;
		case ID_OPR_EQUAL:
			if(m_ps->GetTop() && m_ps->GetTop()->m_next)
				ec = m_ps->Execute(toBottom);
			else if(m_ps->GetTop())
				m_ps->GetTop()->m_code = ID_OPR_EQUAL;
			break;
		default:
			while(GetTop() && GetTop()->m_next)
			{
				if(Precedence(GetTop()->m_next->m_code) >= Precedence(GetTop()->m_code))
				{
					GetTop()->m_code = ID_OPR_EQUAL;
					ec = m_ps->Execute(toTop);
					if(GetTop())
						GetTop()->m_code = dc;
					if(ec != errOK) break;
				}
				else
				{
					break;
				}
			}
			break;
		}
		UpdateDisplay(NULL);
		break;

	case ID_RCL_X:
	case ID_RCL_Y:
	case ID_RCL_Z:
	case ID_RCL_A:
	case ID_RCL_B:
	case ID_RCL_C:
	case ID_RCL_D:
	case ID_RCL_E:
	case ID_RCL_F:
	case ID_RCL_G:
	case ID_RCL_H:
	case ID_RCL_I:
	case ID_RCL_J:
	case ID_RCL_K:
	case ID_RCL_L:
	case ID_RCL_M:
	case ID_RCL_N:
	case ID_RCL_O:
	case ID_RCL_P:
	case ID_RCL_Q:
	case ID_RCL_R:
	case ID_RCL_S:
	case ID_RCL_T:
	case ID_RCL_U:
	case ID_RCL_V:
	case ID_RCL_W:

		if(m_state != kbBlank)
		{
			if(m_ps->GetTop() && m_ps->GetTop()->m_code == ID_OPR_EQUAL)
				m_ps->Clear();
			CommitStage(ID_OPR_EQUAL);
		}
		ec = Recall(&pn, dc);

		if(pn && ec == errOK)
		{
			m_ps->Push(pn);
		}
		m_state  = kbBlank;
		m_cursor = 0;
		Record(dc);
		UpdateDisplay(NULL);
		break;

	case ID_STR_X:
	case ID_STR_Y:
	case ID_STR_Z:
	case ID_STR_A:
	case ID_STR_B:
	case ID_STR_C:
	case ID_STR_D:
	case ID_STR_E:
	case ID_STR_F:
	case ID_STR_G:
	case ID_STR_H:
	case ID_STR_I:
	case ID_STR_J:
	case ID_STR_K:
	case ID_STR_L:
	case ID_STR_M:
	case ID_STR_N:
	case ID_STR_O:
	case ID_STR_P:
	case ID_STR_Q:
	case ID_STR_R:
	case ID_STR_S:
	case ID_STR_T:
	case ID_STR_U:
	case ID_STR_V:
	case ID_STR_W:
		if(m_state != kbBlank)
		{
			CommitStage(ID_OPR_EQUAL);
		}
		Store(m_ps->GetTop(), dc);
		Record(dc);
		UpdateDisplay(NULL);
		break;

	case ID_CTL_CLR:
		m_cursor	= 0;
		m_stage[0]	= 0;
		m_state		= kbBlank;
		Record(dc);
		UpdateDisplay(NULL);
		break;

	case ID_CTL_CLRA:
		m_cursor	= 0;
		m_stage[0]	= 0;
		m_state		= kbBlank;
		m_ps->Clear();
		Record(dc);
		UpdateDisplay(NULL);
		break;

	case ID_CTL_DEL:
		if(m_state == kbNumber)
		{
			if(m_cursor > 0)
				m_stage[--m_cursor] = 0;
		}
		else if(m_state == kbExponent)
		{
			if(m_expcursor > m_cursor)
				m_stage[--m_expcursor] = 0;
		}
		Record(dc);
		UpdateDisplay(NULL);
		break;

	case ID_CTL_STR:
	case ID_CTL_RCL:
		DoMemAccess(dc);
		break;

	case ID_CTL_COPY:
		{
			HANDLE hMem;
			LPCSTR pt;
			LPTSTR ptt;
			TCHAR  tb[130];

			if(GetTop())
				pt = GetTop()->Format(128, m_radix, m_dispformat);
			else
				pt = m_stage;
#ifdef UNICODE
			BUtil::Utf8Decode(tb, pt);
#else
			strcpy(tb, pt);
#endif
			hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (_tcslen(tb) + 4) * sizeof(TCHAR));
			if(hMem)
			{
				OpenClipboard(NULL);
				EmptyClipboard();
				ptt = (LPTSTR)GlobalLock(hMem);
				_tcscpy(ptt, tb);
				GlobalUnlock(hMem);
				SetClipboardData(
							#ifdef UNICODE
									CF_UNICODETEXT,
							#else
									CF_TEXT,
							#endif
									hMem
								);
				CloseClipboard();
			}
		}
		Record(dc);
		break;

	case ID_CTL_PASTE:

		if(OpenClipboard(m_hwnd))
		{
			HANDLE hMem = GetClipboardData(CF_TEXT);
			LPSTR  pcp;

			if(hMem)
			{
				if((pcp = (LPSTR)GlobalLock(hMem)) != NULL)
				{
					Bnumber* pn;
					int		 r = 10;

					pn = MakeNumber(pcp, r, ID_OPR_EQUAL);
					if(pn) m_ps->Push(pn);
					UpdateDisplay(NULL);
				}
			}
			CloseClipboard();
		}
		break;

	case ID_CTL_CONST:
		DoConst();
		break;

	case ID_CTL_PRG:
		m_view_program = ! m_view_program;
		if(m_view_program)
		{
			BappPanel* px;
			RECT       rc;
			int        w, h;

			px = GetParentPanel();
			if(px->GetParentPanel())
				px = px->GetParentPanel();
			px->GetPanelClientRect(&rc);

			if(! m_progpanel)
			{
				GetPersist()->GetNvInt(_T("display/program/W"), w);

				m_progpanel = new BappPanel(_T("Program"), px);
				rc.left = 0;
				rc.right = w;
				px->Dock(m_progpanel, frRight, &rc);
				m_progpanel->AddPane(new Bprog(this, m_progpanel));
				m_progpanel->ActivatePane(0);
			}
			else
			{
				m_progpanel->ActivatePane(0);
			}
			if(! m_plotpanel)
			{
				Bplot* pp;
				int    d, sx, sy, ex, ey, xs, ys;

				GetPersist()->GetNvInt(_T("display/plot/H"), h);
				GetPersist()->GetNvInt(_T("display/plot/Dimension"),d, 2);
				GetPersist()->GetNvInt(_T("display/plot/X Start"), sx, -10000);
				GetPersist()->GetNvInt(_T("display/plot/Y Start"), sy, -10000);
				GetPersist()->GetNvInt(_T("display/plot/X End"),   ex, 10000);
				GetPersist()->GetNvInt(_T("display/plot/Y End"),   ey, 10000);
				GetPersist()->GetNvInt(_T("display/plot/X Step"),  xs, 1000);
				GetPersist()->GetNvInt(_T("display/plot/Y Step"),  ys, 1000);

				m_plotpanel = new BappPanel(_T("Plot"), px);
				rc.top = rc.bottom - h;
				rc.left = 0;
				px->Dock(m_plotpanel, frBottom, &rc, true, false);
				pp = new Bplot(this, m_plotpanel);
				pp->SetPlotExtents(d,(double)sx/1000.0, (double)sy/1000.0,
					(double)ex/1000.0, (double)ey/1000.0,
					(double)xs/1000.0, (double)ys/1000.0);
				m_plotpanel->AddPane(pp);
				m_plotpanel->ActivatePane(0);
			}
			else
			{
				m_plotpanel->ActivatePane(0);
			}
		}
		else
		{
			BappPanel* px;

			px = GetParentPanel();
			if(px->GetParentPanel())
				px = px->GetParentPanel();

			if(m_progpanel)
			{
				px->RemovePanel(m_progpanel);
				delete m_progpanel;
				m_progpanel = NULL;
			}
			if(m_plotpanel)
			{
				px->RemovePanel(m_plotpanel);
				delete m_plotpanel;
				m_plotpanel = NULL;
			}
		}
		break;

	case ID_CTL_SCI:
		m_view_scitrig = ! m_view_scitrig;
		SetupWindows(false);
		break;

	case ID_CTL_EXP:
		if(m_state == kbNumber || m_state == kbFraction)
		{
			m_stage[m_cursor] = 'e';
			m_expcursor	= m_cursor + 1;
			m_stage[m_expcursor] = 0;
			m_state		= kbExponent;
			//SetupWindows(false);
			UpdateWindow(NULL);
		}
		break;

	case ID_RADIX_UP:
		CommitStage(ID_OPR_EQUAL);
		switch(m_radix)
		{
		case 2:	m_radix = 8;	break;
		case 8:	m_radix = 10;	break;
		case 10:m_radix = 16;	break;
		case 16:m_radix = 2;	break;
		}
		SetupWindows(false);
		UpdateDisplay(m_hwndXinfo);
		break;

	case ID_RADIX_DOWN:
		CommitStage(ID_OPR_EQUAL);
		switch(m_radix)
		{
		case 2: m_radix = 16;	break;
		case 8: m_radix = 2;	break;
		case 10:m_radix = 8;	break;
		case 16:m_radix = 10;	break;
		}
		SetupWindows(false);
		UpdateDisplay(m_hwndXinfo);
		break;

	case ID_XRADIX_UP:
		switch(m_xradix)
		{
		case 2:	m_xradix = 8;	break;
		case 8:	m_xradix = 10;	break;
		case 10:m_xradix = 16;	break;
		case 16:m_xradix = 2;	break;
		}
		UpdateDisplay(m_hwndXinfo);
		break;

	case ID_XRADIX_DOWN:
		switch(m_xradix)
		{
		case 2: m_xradix = 16;	break;
		case 8: m_xradix = 2;	break;
		case 10:m_xradix = 8;	break;
		case 16:m_xradix = 10;	break;
		}
		UpdateDisplay(m_hwndXinfo);
		break;

	case ID_FORMAT_UP:
		CommitStage(ID_OPR_EQUAL);
		prevformat = m_mathformat;
		switch(m_mathformat)
		{
		case kbInteger:			m_mathformat = kbUnsignedInteger;	break;
		case kbUnsignedInteger:	m_mathformat = kbFloat;				break;
		case kbFloat:			m_mathformat = kbArbitrary;			break;
		case kbArbitrary:		m_mathformat = kbInteger;			break;
		}
		ReformatStack(prevformat, m_mathformat);
		SetupWindows(false);
		UpdateDisplay(m_hwndFinfo);
		break;

	case ID_FORMAT_DOWN:
		CommitStage(ID_OPR_EQUAL);
		prevformat = m_mathformat;
		switch(m_mathformat)
		{
		case kbInteger:			m_mathformat = kbArbitrary;			break;
		case kbUnsignedInteger:	m_mathformat = kbInteger;			break;
		case kbFloat:			m_mathformat = kbUnsignedInteger;	break;
		case kbArbitrary:		m_mathformat = kbFloat;				break;
		}
		ReformatStack(prevformat, m_mathformat);
		SetupWindows(false);
		UpdateDisplay(m_hwndFinfo);
		break;

	case ID_DFRMT_UP:
		CommitStage(ID_OPR_EQUAL);
		if(m_mathformat == kbInteger || m_mathformat == kbUnsignedInteger)
		{
			m_dispformat = kbDefault;
		}
		else switch(m_dispformat)
		{
		case kbDefault:		m_dispformat = kbScientific;break;
		case kbScientific:	m_dispformat = kbEngineer;	break;
		case kbEngineer:	m_dispformat = kbDefault;	break;
		}
		UpdateDisplay(m_hwndFinfo);
		break;

	case ID_DFRMT_DOWN:
		CommitStage(ID_OPR_EQUAL);
		if(m_mathformat == kbInteger || m_mathformat == kbUnsignedInteger)
		{
			m_dispformat = kbDefault;
		}
		else switch(m_dispformat)
		{
		case kbDefault:		m_dispformat = kbEngineer;	break;
		case kbScientific:	m_dispformat = kbDefault;	break;
		case kbEngineer:	m_dispformat = kbScientific;break;
		}
		UpdateDisplay(m_hwndFinfo);
		break;

	case ID_CTL_QUIT:
		SaveSettings();
		PostQuitMessage(0);
		break;

	default:
		return errBAD_PARAMETER;
	}
	return ec;
}

//**************************************************************************
void Bcalc::MouseMenu(int x, int y)
{
	HMENU	hMenu;
	RECT	rectw;
	int		rc;

	hMenu = CreatePopupMenu();

	AppendMenu(hMenu, 0, ID_CTL_COPY, _T("&Copy"));
	AppendMenu(hMenu, 0, ID_CTL_PASTE,_T("&Paste"));
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenu, 0, ID_CTL_RCL, _T("&Recall"));
	AppendMenu(hMenu, 0, ID_CTL_STR, _T("&Store"));
	AppendMenu(hMenu, 0, ID_CTL_PRG, _T("Pr&ogram"));

	GetWindowRect(m_hwnd, &rectw);

	rc = TrackPopupMenu(
						hMenu,
						TPM_RETURNCMD,
						x + rectw.left,
						y + rectw.top,
						0,
						m_hwnd,
						NULL
					   );

	DestroyMenu(hMenu);
	if(rc)
	{
		SendMessage(m_hwnd, WM_COMMAND, rc, rc);
	}
}

//**************************************************************************
LRESULT Bcalc::OnMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC			hdc;
	Bcalc*		pc;
	RECT		rc;
	ERRCODE		ec;
	bool		bShowit = true;
	int			id = 0;
	int			mpx, mpy;

	static int	ctrl = 0, shft = 0;

	pc = (Bcalc*)GetWindowLong(hWnd, GWL_USERDATA);

	switch (message)
	{
	case WM_CREATE:

		{
			LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;

			SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)pcs->lpCreateParams);
		}
		break;

	case WM_DESTROY:

		pc->SaveSettings();
		return DefWindowProc(hWnd, message, wParam, lParam);

	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rc);
		EndPaint(hWnd, &ps);
		break;

	case WM_COMMAND:

		ec = OnKey(wParam);
		if(ec != errOK)
		{
			m_state = kbError;
			UpdateDisplay(NULL);
		}
		break;

	case WM_CHAR:

		switch(wParam)
		{
		case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9': case '0':
			id = ID_NUM_0 + (wParam - '0');
			break;

		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
			id = ID_NUM_A + (wParam - 'a');
			break;

		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
			id = ID_NUM_A + (wParam - 'A');
			break;

		case '.':	id = ID_CTL_DEC;	break;

		case '-':	id = ID_OPR_SUB;	break;
		case '+':	id = ID_OPR_ADD;	break;
		case '*':	id = ID_OPR_MUL;	break;
		case '/':	id = ID_OPR_DIV;	break;
		case '~':	id = ID_OPR_NOT;	break;
		case '%':	id = ID_OPR_MOD;	break;
		case '^':	id = ID_OPR_XOR;	break;
		case '&':	id = ID_OPR_AND;	break;
		case '|':	id = ID_OPR_OR;		break;
		case '(':	id = ID_OPR_LPAREN;	break;
		case ')':	id = ID_OPR_RPAREN;	break;
		case '=':	id = ID_OPR_EQUAL;	break;

		case 8: // 8
			id = ID_CTL_DEL;
			break;

		case 10:
		case 13:
			id = ID_OPR_EQUAL;
			break;

		case 'x':
			id = ID_RADIX_UP;
			break;
		case 'X':
			id = ID_XRADIX_UP;
			break;

		case 'C' - 'A' + 1: // ^C
			id = ID_CTL_COPY;
			break;
		case 'V' - 'A' + 1: // ^V
			id = ID_CTL_PASTE;
			break;
		}
		if(id)
			PostMessage(hWnd, WM_COMMAND, id, 0);
		break;

	case WM_KEYDOWN:

		switch(wParam)
		{
		case VK_PRIOR:
		case VK_NEXT:
		case VK_END:
		case VK_HOME:
		case VK_LEFT:
		case VK_UP:
		case VK_RIGHT:
		case VK_DOWN:
			{
				Bprog* pp;

				if(m_progpanel)
				{
					pp = (Bprog*)m_progpanel->FindPane(_T("Program"));
					if(pp) PostMessage(pp->GetWindow(), WM_KEYDOWN, wParam, lParam);
				}
			}
			break;

		case VK_CONTROL:
			ctrl = 1;
			break;

		case VK_SHIFT:
			shft = 1;
			break;

		case VK_INSERT:
			if(ctrl)
			{
				id =  ID_CTL_COPY;
			}
			else if(shft)
			{
				id = ID_CTL_PASTE;
			}
			break;

		/*
		case VK_NUMPAD0:	id = ID_NUM_0; break;
		case VK_NUMPAD1:	id = ID_NUM_1; break;
		case VK_NUMPAD2:	id = ID_NUM_2; break;
		case VK_NUMPAD3:	id = ID_NUM_3; break;
		case VK_NUMPAD4:	id = ID_NUM_4; break;
		case VK_NUMPAD5:	id = ID_NUM_5; break;
		case VK_NUMPAD6:	id = ID_NUM_6; break;
		case VK_NUMPAD7:	id = ID_NUM_7; break;
		case VK_NUMPAD8:	id = ID_NUM_8; break;
		case VK_NUMPAD9:	id = ID_NUM_9; break;
		*/
		case VK_MULTIPLY:	id = ID_OPR_MUL; break;
		case VK_ADD:		id = ID_OPR_ADD; break;
		case VK_SEPARATOR:	id = ID_OPR_EQUAL; break;
		case VK_SUBTRACT:	id = ID_OPR_SUB; break;
		case VK_DECIMAL:	id = ID_CTL_DEC; break;
		case VK_DIVIDE:		id = ID_OPR_DIV; break;

		default:
			id = 0;
			break;
		}
		if(id)
			PostMessage(hWnd, WM_COMMAND, id, 0);
		break;

	case WM_KEYUP:

		switch(wParam)
		{
		case VK_CONTROL:
			ctrl = 0;
			break;

		case VK_SHIFT:
			shft = 0;
			break;

		default:
			break;
		}
		break;

	case WM_LBUTTONDOWN:
		SetFocus(hWnd);
		break;

	case WM_RBUTTONDOWN:
		mpx = (int)(short)LOWORD(lParam);
		mpy = (int)(short)HIWORD(lParam);
		pc->MouseMenu(mpx, mpy);
		break;

	case WM_MOUSEWHEEL:

		{
			Bprog* pp;

			if(m_progpanel)
			{
				pp = (Bprog*)m_progpanel->FindPane(_T("Program"));
				if(pp) PostMessage(pp->GetWindow(), WM_MOUSEWHEEL, wParam, lParam);
			}
		}
		break;

	default:
		return BappPane::OnMessage(hWnd, message, wParam, lParam);
   }
   return 0;
}



