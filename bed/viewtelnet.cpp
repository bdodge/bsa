
#include "bedx.h"

//**************************************************************************
BviewTelnet::BviewTelnet(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel)
		:
		BviewStream(pBuf, pEditor, pPanel)
{
	BsocketStream::Init();

	m_host[0] = _T('\0');
	m_port = 23;

	// if host name specified on command line, use it instead of
	// remembered host
	//
	if(pBuf && pBuf->GetName() && _tcslen(pBuf->GetName()) > 0 && _tcscmp(pBuf->GetName(), _T("Remote")))
	{
		LPTSTR pp, pb;

		pb = (LPTSTR)pBuf->GetName();
		pp = pb + _tcslen(pb) - 1;
		for(;pp >= pb; pp--)
		{
			if(*pp == _T('/') || *pp == _T('\\'))
			{
				pp++;
				break;
			}
		}
		if(pp < pb)
			pp++;
		if(_tcslen(pp))
		{
			_tcsncpy(m_host, pp, MAX_PATH);
			m_host[MAX_PATH - 1] = _T('\0');

			pBuf->SetName(pp);

			// extract any port spec from hostname
			//
			for(pp = m_host; *pp; pp++)
			{
				if(*pp == _T(':'))
				{
					*pp++ = _T('\0');
					m_port = _tcstol(pp, NULL, 0);
					break;
				}
			}
		}
	}
}

//**************************************************************************
BviewTelnet::~BviewTelnet()
{
	if(m_running)
		Stop();
}

//**************************************************************************
void BviewTelnet::Activate()
{
	bool issetup = false;

	if(GetEditor()->GetPersist())
	{
		GetEditor()->GetPersist()->GetNvBool(_T("Telnet/SetupValid"), issetup, false);
		if(m_host[0])
		{
			GetEditor()->GetPersist()->SetNvInt(_T("Telnet/CurrentPort"), m_port);
			GetEditor()->GetPersist()->SetNvStr(_T("Telnet/CurrentHost"), m_host);
		}
	}
	if(! issetup || ! m_host[0])
	{
		if(SettingsDialog(GetEditor(), (BviewTerminal*)this, GetWindow()) != IDOK)
		{
			Close();
			return;
		}
		GetEditor()->GetPersist()->GetNvInt(_T("Telnet/CurrentPort"), m_port, m_port);
		GetEditor()->GetPersist()->GetNvStr(_T("Telnet/CurrentHost"), m_host, MAX_PATH, _T(""));

	}
	ReopenPort();
	if(GetEditor()->GetPersist())
		GetEditor()->GetPersist()->SetNvBool(_T("Telnet/SetupValid"), true);
	BviewTerminal::Activate();
}

//**************************************************************************
void BviewTelnet::FitToPanel()
{
	BviewStream::FitToPanel();
	if(m_io)
	{
		((BtelnetStream*)m_io)->TelnetSetWindowSize(GetViewCols(), GetViewLines());
	}
}

//**************************************************************************
ERRCODE BviewTelnet::GetPortSettings()
{
	Bpersist* pp;

	pp = m_editor->GetPersist();
	if(pp)
	{
		pp->GetNvInt(_T("Telnet/CurrentPort"), m_port, m_port);
		pp->GetNvStr(_T("Telnet/CurrentHost"), m_host, MAX_PATH, _T(""));
	}
	return errOK;
}

//**************************************************************************
ERRCODE BviewTelnet::ApplyPortSettings()
{
	TCHAR   vname[256];
	char    ahost[MAX_PATH];
	ERRCODE ec;

	if(m_novt)
		m_emulation = emulNONE;
	else
		m_emulation = emulXTERM;

	m_iochanging = true;
	if(m_io)
	{
		ec = m_io->Close();
	}
	else
	{
		m_io = new BtelnetStream();
	}
	m_iochanging = false;

	TCharToChar(ahost, m_host);
	ec = ((BtelnetStream*)m_io)->Open(ahost, m_port);

	if(ec == errOK)
	{
		ec = m_io->Connect(5, 0);
	}
	if(ec != errOK)
	{
		int l;

		l = _sntprintf(vname, 200, _T("Can't open: " _Pfs_ ":%d"), m_host, m_port);
		MessageBox(NULL, vname, _T("BED 6.0 - Port Error"), MB_OK);
	}
	else
	{
		int fh, fw;
		RECT rc;

		fh = GetFontHeight();
		fw = GetFontWidth();

		GetClientRect(m_hwnd, &rc);
		((BtelnetStream*)m_io)->TelnetSetWindowSize(((rc.right - rc.left) * 15) / fw, (rc.bottom - rc.top) / fh);

		// send willtells
		if(0) {
			BYTE xx[32];

			xx[0] = 255;
			xx[1] = 251;
			xx[2] = 3;
			SendData(xx, 3);
			xx[0] = 255;
			xx[1] = 251;
			xx[2] = 1;
			SendData(xx, 3);
		}
	}
	_sntprintf(vname, 256, _T("" _Pfs_ ":%d"), m_host, m_port);
	m_buffer->SetName(vname);
	m_editor->UpdateInfoPanes(this);
	return ec;
}

typedef struct tag_tpparm
{
	Bed*			f_peditor;
	BviewTerminal*	f_pterm;
}
TPPARM, *PTPPARM;


// STATIC
//**************************************************************************
static BOOL CALLBACK Telnetproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PTPPARM		ptp = NULL;
	Bpersist*	pp;
	HWND		hwndParent;
	RECT		rc, rcme;
	TCHAR		pparm[MAX_PATH + 256];

	int			port;
	TCHAR		host[MAX_PATH];

	ptp = (PTPPARM)GetWindowLong(hWnd, GWL_USERDATA);

	switch(message)
	{
	case WM_INITDIALOG:

		ptp = (PTPPARM)lParam;
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)ptp);
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
		port	= 23;

		pp = ptp->f_peditor->GetPersist();
		if(pp)
		{
			pp->GetNvInt(_T("Telnet/CurrentPort"), port, port);
			pp->GetNvStr(_T("Telnet/CurrentHost"), host, MAX_PATH, _T(""));
		}
		// host
		SetDlgItemText(hWnd, IDC_TELNET_HOST, host);

		// port
		_sntprintf(pparm, 32, _T("%d"), port);
		SetDlgItemText(hWnd, IDC_TELNET_PORT, pparm);
		return TRUE;

	case WM_COMMAND:

		switch(wParam)
		{
		case IDOK:

			if(! ptp) { EndDialog(hWnd, IDCANCEL); break; }

			// host
			GetDlgItemText(hWnd, IDC_TELNET_HOST, host, MAX_PATH);

			// port
			GetDlgItemText(hWnd, IDC_TELNET_PORT, pparm, MAX_PATH);
			port = _tcstol(pparm, NULL, 0);

			pp = ptp->f_peditor->GetPersist();
			if(pp)
			{
				pp->SetNvInt(_T("Telnet/CurrentPort"), port);
				pp->SetNvStr(_T("Telnet/CurrentHost"), host);
			}
			EndDialog(hWnd, wParam);
			break;

		case IDCANCEL:

			EndDialog(hWnd, wParam);
			break;

		case IDC_EMULATION:

			if(ptp) BviewTerminal::EmulationDialog(ptp->f_peditor, ptp->f_pterm, _T("Telnet"), GetParent(hWnd));
			break;

		default:

			break;
		}
		break;

	default:
		break;
	}
	return FALSE;
}

//**************************************************************************
int BviewTelnet::SettingsDialog(Bed* pEditor, BviewTerminal* pTerm, HWND hWndParent)
{
	TPPARM tp;

	if(! pEditor) return IDCANCEL;

	tp.f_peditor = pEditor;
	tp.f_pterm	 = pTerm;

	return DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_TELNETCONN), hWndParent, Telnetproc, (LPARAM)&tp);
}
