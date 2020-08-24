
#include "bedx.h"

static HBRUSH hbrPreview = NULL;
static HWND	  hwndPreview = NULL;

//**************************************************************************
void RGBtoHSV(int ir, int ig, int ib, int& rh, int& rs, int& rv)
{
	double r, g, b;
	double mc, xc;
	double h,s,v,d;
	
	// convert 0-255 range to 0-1.0 range
	r = (double)ir / 255.0;
	g = (double)ig / 255.0;
	b = (double)ib / 255.0;
	
	// find min and max color component
	if(r < g) {
		if(r < b)
			mc = r;
		else
			mc = b;
		if(b < g)
			xc = g;
		else
			xc = b;
	} else {
		if(g < b)
			mc = g;
		else
			mc = b;
		if(r < b)
			xc = b;
		else
			xc = r;
	}
	// brightness
	v = xc;
	d = xc - mc;

	// saturation
	if(xc != 0.0)
		s = d / xc;
	else
		s = 0.0;
	
	if(s == 0.0)
	{
		h = 0.0;	/* undefined */
	}
	else {
		d = xc - mc;
		if(r == xc)
			h = (g - b) / d;
		else if(g == xc)
			h = 2.0 + (b - r) / d;
		else
			h = 4.0 + (r - g) / d;
		h *= 60;
		if(h < 0.0) 
			h += 360.0;
	}
	rh = (int)(h * 10.0);
	rs = (int)(s * 1000.0);
	rv = (int)(v * 1000.0);
}

//**************************************************************************
void HSVtoRGB(int ih, int is, int iv, int& rr, int& rg, int& rb)
{
	double r, g, b;
	double ht, p, q, t, f;
	double h, s, v;
	int    i;

	h = (double)ih / 10.0;
	s = (double)is / 1000.0;
	v = (double)iv / 1000.0;

	if(s == 0.0)
	{
		r = g = b = v;
	}
	else
	{
		if(h == 360.0)
			ht = 0.0;
		else
			ht = h;

		ht /= 60.0;
		i = (int)ht;
		f = ht - i;
		p = v * (1.0 - s);
		q = v * (1.0 - (s * f));
		t = v * (1.0 - (s * (1.0 - f)));

		switch(i)
		{
		case 0:
			r = v; g = t; b = p;
			break;
		case 1:
			r = q; g = v; b = p;
			break;
		case 2:
			r = p; g = v; b = t;
			break;
		case 3:
			r = p; g = q; b = v;
			break;
		case 4:
			r = t; g = p; b = v;
			break;
		case 5:
			r = v; g = p; b = q;
			break;
		}
	}

	rr = (int)(r * 255.0);
	rg = (int)(g * 255.0);
	rb = (int)(b * 255.0);
}


//**************************************************************************
static LRESULT CALLBACK PreviewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message == WM_PAINT)
	{
		PAINTSTRUCT ps;
		HDC			hdc;

		hdc = BeginPaint(hWnd, &ps);

		if(hbrPreview)
			FillRect(hdc, &ps.rcPaint, hbrPreview);
		EndPaint(hWnd, &ps);
		return 0;
	}
	else
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}

//**************************************************************************
static void ShowColor(HWND hWnd, int h, int s, int v)
{
	int r, g, b;
	
	HSVtoRGB(h, s, v, r, g, b);

	if(hbrPreview != NULL)
		DeleteObject(hbrPreview);
	hbrPreview = CreateSolidBrush(RGB(r, g, b));

	if(! hwndPreview)
	{
		HWND	hwndP = GetDlgItem(hWnd, IDC_COLOR);
		RECT	rectColor;

		WNDCLASS wc;

		// register class for preview color window
		//
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= PreviewWndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= g_hInstance;
		wc.hIcon			= NULL;
		wc.hCursor			= NULL;
		wc.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= _T("BedPreviewColor");

		ATOM aCPClass = RegisterClass(&wc);

		GetWindowRect(hwndP, &rectColor);

		hwndPreview = CreateWindow(
								_T("BedPreviewColor"),
								_T("ColorPreview"),
								WS_CHILD | WS_VISIBLE, 
								4,
								16,
								rectColor.right-rectColor.left - 8,
								rectColor.bottom-rectColor.top - 16 - 4,
								hwndP,
								NULL,
								g_hInstance,
								NULL
								);
		ShowWindow(hwndPreview, SW_SHOW);
	}
	InvalidateRect(hwndPreview, NULL, FALSE);
}


//**************************************************************************
static BOOL CALLBACK ColorProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static LPPARMPASS pParm = NULL;

	RECT	rc;
	RECT	rcme;
	HWND	hwndParent;
	HWND	c;
	int		nv;
	bool	sc;
	ERRCODE	ec;

	static int r, g, b, a;
	static int h, s, v;
	sc = false;

	switch (message) 
	{
	case WM_INITDIALOG:

		pParm = (LPPARMPASS)lParam;
		if(! pParm) break;

		hwndParent = GetParent(hWnd);
		if(! hwndParent) hwndParent = GetDesktopWindow();
		GetClientRect(hwndParent, &rc);
		GetWindowRect(hWnd, &rcme);
		{
			int x = ((rc.right - rc.left) - (rcme.right - rcme.left)) / 2;
			int y = ((rc.bottom - rc.top) - (rcme.bottom - rcme.top)) / 2;
			int w = rcme.right - rcme.left;
			int h = rcme.bottom - rcme.top;

			if(x < 0) x = 0;
			if(y < 0) y = 0;

			MoveWindow(hWnd, x, y, w, h, FALSE);
		}
		ec = Bview::ParseColorSpec(pParm->lpInitVal, r, g, b, a);
		RGBtoHSV(r, g, b, h, s, v);
		c = GetDlgItem(hWnd, IDC_HUE);
		SendMessage(c, TBM_SETRANGE, FALSE, MAKELONG(0, 3600));
		SendMessage(c, TBM_SETPOS, 1, h);
		c = GetDlgItem(hWnd, IDC_SATURATION);
		SendMessage(c, TBM_SETRANGE, FALSE, MAKELONG(0, 1000));
		SendMessage(c, TBM_SETPOS, 1, s);
		c = GetDlgItem(hWnd, IDC_BRIGHTNESS);
		SendMessage(c, TBM_SETRANGE, FALSE, MAKELONG(0, 1000));
		SendMessage(c, TBM_SETPOS, 1, v);
		c = GetDlgItem(hWnd, IDC_TRANSPARENCY);
		SendMessage(c, TBM_SETRANGE, FALSE, MAKELONG(0, 255));
		SendMessage(c, TBM_SETPOS, 1, a);
		ShowColor(hWnd, h, s, v);
		return TRUE;

	case WM_COMMAND:

		switch(wParam)
		{
		case IDOK:

			HSVtoRGB(h, s, v, r, g, b);
			_sntprintf(pParm->lpString, pParm->nString, _T("%d:%d:%d:%d"), r, g, b, a);
			EndDialog(hWnd, IDOK);
			break;

		case IDCANCEL:

			EndDialog(hWnd, wParam);
			break;

		default:

			break;
		}
		break;

	case WM_HSCROLL:
		nv = SendMessage(GetDlgItem(hWnd, IDC_HUE), TBM_GETPOS, 0, 0);
		if(nv != h)
		{
			h = nv;
			sc = true;
		}
		nv = SendMessage(GetDlgItem(hWnd, IDC_SATURATION), TBM_GETPOS, 0, 0);
		if(nv != s)
		{
			s = nv;
			sc = true;
		}
		nv = SendMessage(GetDlgItem(hWnd, IDC_BRIGHTNESS), TBM_GETPOS, 0, 0);
		if(nv != v)
		{
			v = nv;
			sc = true;
		}
		nv = SendMessage(GetDlgItem(hWnd, IDC_TRANSPARENCY), TBM_GETPOS, 0, 0);
		if(nv != v)
		{
			a = nv;
			sc = true;
		}
		if(sc)
			ShowColor(hWnd, h, s, v);
		break;

	case WM_DESTROY:

		if(hbrPreview)
		{
			DeleteObject(hbrPreview);
			hbrPreview = NULL;
		}
		hwndPreview = NULL;
		break;

	default:
		break;
	}
	return FALSE;
}

//**************************************************************************
int PickColorDialog(LPPARMPASS pParm, HWND hwndParent)
{
	return DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_COLORPICK), hwndParent, ColorProc, (LPARAM)pParm);
}

