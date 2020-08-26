
#include "bedx.h"

//**************************************************************************
BviewRS232::BviewRS232(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel)
		:
		m_baud(9600),
		m_bits(8),
		m_stops(1),
		m_parity(0),
		m_flow(0),
		BviewStream(pBuf, pEditor, pPanel)
{
	Bpersist* pp;
	int		  port;
	pp = m_editor->GetPersist();
	
	_tcscpy(m_port, pBuf->GetName());
	
	// if port is a simple number, convert index to name
	// else use name directly
	//
	if (m_port[0] >= '0' && m_port[0] <= '9')
	{
		port = BserialStream::GetIndexOfPort(m_port);
	}
	else
	{
		port = BSASERCOM_USERPORT;
	}
	if (port != BSASERCOM_USERPORT)
	{
		// sanitize port name
		//
		BserialStream::GetActualPortName(port, m_port, MAX_PATH);
	}
	m_zeroreaderr = false; // allow pend == true and read == 0
	ReopenPort();
}

//**************************************************************************
BviewRS232::~BviewRS232()
{
	if(m_running)
		Stop();
}

//**************************************************************************
void BviewRS232::Activate()
{
	bool issetup = false;

	if(GetEditor()->GetPersist())
		GetEditor()->GetPersist()->GetNvBool(_T("RS232/SetupValid"), issetup, false);
	if(! issetup)
	{
		if(SettingsDialog(GetWindow()) != IDOK)
		{
			Close();
			return;
		}
		ReopenPort();
	}
	if(GetEditor()->GetPersist())
		GetEditor()->GetPersist()->SetNvBool(_T("RS232/SetupValid"), true);
	BviewTerminal::Activate();
}

//**************************************************************************
void BviewRS232::Event(LPARAM lParam)
{
	BviewTerminal::Event(lParam);
}

//**************************************************************************
ERRCODE BviewRS232::GetPortSettings()
{
	Bpersist* pp;
	TCHAR	  pparm[MAX_PATH+256];
	LPTSTR    px;
	int       ppl;

	pp = m_editor->GetPersist();
	if(pp)
	{
		ppl = _sntprintf(pparm, MAX_PATH+250, _T(""_Pfs_"/Port"_Pfs_"/Settings/"), GetTypeName(), m_port);
		px = pparm + ppl;
		_tcscpy(px, _T("Baud"));
		pp->GetNvInt(pparm, m_baud, m_baud);
		_tcscpy(px, _T("Bits"));
		pp->GetNvInt(pparm, m_bits, m_bits);
		_tcscpy(px, _T("StopBits"));
		pp->GetNvInt(pparm, m_stops, m_stops);
		_tcscpy(px, _T("Parity"));
		pp->GetNvInt(pparm, m_parity, m_parity);
		_tcscpy(px, _T("FlowControl"));
		pp->GetNvInt(pparm, m_flow, m_flow);		
	}
	return errOK;
}

//**************************************************************************
ERRCODE BviewRS232::ApplyPortSettings()
{
	TCHAR   vname[MAX_PATH + 128];
	ERRCODE ec;
	int     len;

	m_iochanging = true;
	if(m_io)
	{
		ec = m_io->Close();
	}
	else
	{
		m_io = new BserialStream();
	}
	ec = ((BserialStream*)m_io)->Open(m_port, m_baud, m_bits, m_stops, m_parity, m_flow);

	if(ec != errOK)
	{
		_sntprintf(vname, MAX_PATH + 128, _T("Can't open: "_Pfs_", check permissions"), m_port);
		delete m_io;
		m_io = NULL;
		MessageBox(NULL, vname, _T("BED 6.0 - Port Error"), MB_OK);
	}
	else
	{
		_sntprintf(vname, MAX_PATH, _T("RS232 - "_Pfs_""), m_port);
		len = _tcslen(vname);
		_sntprintf(vname + len, MAX_PATH-len, _T(":%d,%d,%d"), m_baud, m_bits, m_stops);
		m_buffer->SetName(vname);
		SetName(vname);
	}
	m_iochanging = false;
	m_editor->UpdateInfoPanes(this);
	return ec;
}

// STATIC
static void CheckPortButton(HWND hWnd, int port, LPCTSTR portname)
{
	TCHAR		pparm[MAX_PATH + 256];

	CheckDlgButton(hWnd, IDC_PORT0, (port == 0) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_PORT1, (port == 1) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_PORT2, (port == 2) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_PORT3, (port == 3) ? BST_CHECKED : BST_UNCHECKED);
#ifdef Windows
	CheckDlgButton(hWnd, IDC_PORT4, (port == 4) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_PORT5, (port == 5) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_PORTX, (port > 5) ? BST_CHECKED : BST_UNCHECKED);
#else
	CheckDlgButton(hWnd, IDC_PORT4, (port == 64) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_PORT5, (port == 65) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hWnd, IDC_PORTX, (port == BSASERCOM_USERPORT) ? BST_CHECKED : BST_UNCHECKED);
#endif

	SetDlgItemText(hWnd, IDC_PORT0, (LPTSTR)BserialStream::GetActualPortName(0, pparm, 64));
	SetDlgItemText(hWnd, IDC_PORT1, (LPTSTR)BserialStream::GetActualPortName(1, pparm, 64));
	SetDlgItemText(hWnd, IDC_PORT2, (LPTSTR)BserialStream::GetActualPortName(2, pparm, 64));
	SetDlgItemText(hWnd, IDC_PORT3, (LPTSTR)BserialStream::GetActualPortName(3, pparm, 64));
#ifdef Windows
	SetDlgItemText(hWnd, IDC_PORT4, (LPTSTR)BserialStream::GetActualPortName(4, pparm, 64));
	SetDlgItemText(hWnd, IDC_PORT5, (LPTSTR)BserialStream::GetActualPortName(5, pparm, 64));
	_sntprintf(pparm, sizeof(pparm), _T("Parm: %s"), portname);
	SetDlgItemText(hWnd, IDC_PORTX, pparm);
#else
	SetDlgItemText(hWnd, IDC_PORT4, (LPTSTR)BserialStream::GetActualPortName(64, pparm, 64));
	SetDlgItemText(hWnd, IDC_PORT5, (LPTSTR)BserialStream::GetActualPortName(65, pparm, 64));
	SetDlgItemText(hWnd, IDC_PORTX, _T("Custom"));
#endif
	if (portname[0] >= '0' && portname[0] <= '9')
	{
		if (BserialStream::GetIndexOfPort(portname) != BSASERCOM_USERPORT)
		{
			SetDlgItemText(hWnd, IDC_PORTN, (LPTSTR)BserialStream::GetActualPortName(port, pparm, 64));
		}
		else
		{
			SetDlgItemText(hWnd, IDC_PORTN, portname);
		}
	}
	else
	{
		SetDlgItemText(hWnd, IDC_PORTN, portname);
	}
}

// STATIC
//**************************************************************************
static BOOL CALLBACK RS232proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	BviewRS232*	pTerm = NULL;
	Bpersist*	pp;
	HWND		hwndParent;
	RECT		rc, rcme;
	TCHAR		pparm[MAX_PATH + 256];
	LPTSTR		px;
	int			ppl;
	
	static int   port, baud, bits, stops, parity, flow;
	static TCHAR portname[MAX_PATH];

	pTerm = (BviewRS232*)GetWindowLong(hWnd, GWL_USERDATA);

	switch (message)
	{
	case WM_INITDIALOG:

		pTerm = (BviewRS232*)lParam;
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)pTerm);
		if (pTerm)
		{
			_tcsncpy(portname, pTerm->GetPort(), MAX_PATH);
			port = BserialStream::GetIndexOfPort(portname);
		}
		else
		{
			portname[0] = 0;
			port = 0;
		}
		hwndParent = GetParent(hWnd);
		if (!hwndParent) hwndParent = GetDesktopWindow();
		GetClientRect(hwndParent, &rc);
		GetWindowRect(hWnd, &rcme);
		{
			int x = ((rc.right - rc.left) - (rcme.right - rcme.left)) / 2;
			int y = ((rc.bottom - rc.top) - (rcme.bottom - rcme.top)) / 2;
			int w = rcme.right - rcme.left;
			int h = rcme.bottom - rcme.top;

			if (x < 0) x = 0;
			if (y < 0) y = 0;

			MoveWindow(hWnd, x, y, w, h, FALSE);
		}
		baud = 9600;
		bits = 8;
		stops = 1;
		parity = 0;
		flow = 0;
		pp = pTerm->GetEditor()->GetPersist();
		if (pp)
		{
			_sntprintf(pparm, 128, _T(""_Pfs_"/CurrentPort"), pTerm->GetTypeName());

			ppl = _sntprintf(pparm, 128, _T(""_Pfs_"/Port"_Pfs_"/Settings/"), pTerm->GetTypeName(), portname);
			px = pparm + ppl;
			_tcscpy(px, _T("Baud"));
			pp->GetNvInt(pparm, baud, baud);
			_tcscpy(px, _T("Bits"));
			pp->GetNvInt(pparm, bits, bits);
			_tcscpy(px, _T("StopBits"));
			pp->GetNvInt(pparm, stops, stops);
			_tcscpy(px, _T("Parity"));
			pp->GetNvInt(pparm, parity, parity);
			_tcscpy(px, _T("FlowControl"));
			pp->GetNvInt(pparm, flow, flow);
		}
		// port
		CheckPortButton(hWnd, port, portname);

		// baud
		CheckDlgButton(hWnd, IDC_BR300,  (baud == 300) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_BR600,  (baud == 600) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_BR1200, (baud == 1200) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_BR2400, (baud == 2400) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_BR4800, (baud == 4800) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_BR9600, (baud == 9600) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_BR19K,  (baud == 19200) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_BR38K,  (baud == 38400) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_BR57K,	 (baud == 57600) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_BR115K, (baud == 115200) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_BR230K, (baud == 230400) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_BR460K, (baud == 460800) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_BR921K, (baud == 921600) ? BST_CHECKED : BST_UNCHECKED);

		// bits
		CheckDlgButton(hWnd, IDC_BI7, (bits == 7) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_BI8, (bits == 8) ? BST_CHECKED : BST_UNCHECKED);

		// stops
		CheckDlgButton(hWnd, IDC_SB1, (stops == 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_SB2, (stops == 2) ? BST_CHECKED : BST_UNCHECKED);

		// parity
		CheckDlgButton(hWnd, IDC_PAR0, (parity == 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_PAR1, (parity == 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_PAR2, (parity == 2) ? BST_CHECKED : BST_UNCHECKED);

		// flow
		CheckDlgButton(hWnd, IDC_FCNONE, (flow == 0) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_FCHW,   (flow == 1) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_FCSW,   (flow == 2) ? BST_CHECKED : BST_UNCHECKED);

		return TRUE;

	case WM_COMMAND:

		switch(wParam)
		{
		case IDOK:

			if(! pTerm) { EndDialog(hWnd, IDCANCEL); break; }

			_tcsncpy(portname, pTerm->GetPort(), MAX_PATH);
			port = BserialStream::GetIndexOfPort(portname);
#ifdef Windows
			if (port > 5)
				port = BSASERCOM_USERPORT;
#endif

			if(IsDlgButtonChecked(hWnd, IDC_PORT0))
				port = 0;
			else if(IsDlgButtonChecked(hWnd, IDC_PORT1))
				port = 1;
			else if(IsDlgButtonChecked(hWnd, IDC_PORT2))
				port = 2;
			else if(IsDlgButtonChecked(hWnd, IDC_PORT3))
				port = 3;
#ifdef Windows
			else if(IsDlgButtonChecked(hWnd, IDC_PORT4))
				port = 4;
			else if(IsDlgButtonChecked(hWnd, IDC_PORT5))
				port = 5;
#else
			else if(IsDlgButtonChecked(hWnd, IDC_PORT4))
				port = 64;
			else if(IsDlgButtonChecked(hWnd, IDC_PORT5))
				port = 65;
#endif
			else if (IsDlgButtonChecked(hWnd, IDC_PORTX))
			{
				port = BSASERCOM_USERPORT;
			}
			GetDlgItemText(hWnd, IDC_PORTN, portname, MAX_PATH);
			pTerm->SetPort(portname);

			// baud
			if(IsDlgButtonChecked(hWnd, IDC_BR300))
				baud = 300;
			else if(IsDlgButtonChecked(hWnd, IDC_BR600))
				baud = 600;
			else if(IsDlgButtonChecked(hWnd, IDC_BR1200))
				baud = 1200;
			else if(IsDlgButtonChecked(hWnd, IDC_BR2400))
				baud = 2400;
			else if(IsDlgButtonChecked(hWnd, IDC_BR4800))
				baud = 4800;
			else if(IsDlgButtonChecked(hWnd, IDC_BR9600))
				baud = 9600;
			else if(IsDlgButtonChecked(hWnd, IDC_BR19K))
				baud = 19200;
			else if(IsDlgButtonChecked(hWnd, IDC_BR38K))
				baud = 38400;
			else if(IsDlgButtonChecked(hWnd, IDC_BR57K))
				baud = 57600;
			else if(IsDlgButtonChecked(hWnd, IDC_BR115K))
				baud = 115200;
			else if (IsDlgButtonChecked(hWnd, IDC_BR230K))
				baud = 230400;
			else if (IsDlgButtonChecked(hWnd, IDC_BR460K))
				baud = 460800;
			else if (IsDlgButtonChecked(hWnd, IDC_BR921K))
				baud = 921600;

			// bits
			if(IsDlgButtonChecked(hWnd, IDC_BI7))
				bits = 7;
			else if(IsDlgButtonChecked(hWnd, IDC_BI8))
				bits = 8;

			// stops
			if(IsDlgButtonChecked(hWnd, IDC_SB1))
				stops = 1;
			else if(IsDlgButtonChecked(hWnd, IDC_SB2))
				stops = 2;

			// parity
			if(IsDlgButtonChecked(hWnd, IDC_PAR0))
				parity = 0;
			else if(IsDlgButtonChecked(hWnd, IDC_PAR1))
				parity = 1;
			else if(IsDlgButtonChecked(hWnd, IDC_PAR2))
				parity = 2;

			// flow
			if(IsDlgButtonChecked(hWnd, IDC_FCNONE))
				flow = 0;
			else if(IsDlgButtonChecked(hWnd, IDC_FCHW))
		    	flow = 1;
			else if(IsDlgButtonChecked(hWnd, IDC_FCSW))
				flow = 2;

			pp = pTerm->GetEditor()->GetPersist();
			if(pp)
			{
				_sntprintf(pparm, 128, _T(""_Pfs_"/CurrentPort"), pTerm->GetTypeName());
				pp->SetNvStr(pparm, portname);
				ppl = _sntprintf(pparm, MAX_PATH + 32, _T(""_Pfs_"/Port"_Pfs_"/Settings/"), pTerm->GetTypeName(), portname);
				px = pparm + ppl;
				_tcscpy(px, _T("Baud"));
				pp->SetNvInt(pparm, baud);
				_tcscpy(px, _T("Bits"));
				pp->SetNvInt(pparm, bits);
				_tcscpy(px, _T("StopBits"));
				pp->SetNvInt(pparm, stops);
				_tcscpy(px, _T("Parity"));
				pp->SetNvInt(pparm, parity);
				_tcscpy(px, _T("FlowControl"));
				pp->SetNvInt(pparm, flow);
			}
			EndDialog(hWnd, wParam);
			break;

		case IDC_PORT0: case IDC_PORT1: case IDC_PORT2: case IDC_PORT3:
		case IDC_PORT4:	case IDC_PORT5:
			switch (wParam)
			{
			default:
			case IDC_PORT0:	port = 0; break;
			case IDC_PORT1:	port = 1; break;
			case IDC_PORT2:	port = 2; break;
			case IDC_PORT3:	port = 3; break;
			case IDC_PORT4:	port = 4; break;
			case IDC_PORT5:	port = 5; break;
			}
			BserialStream::GetActualPortName(port, portname, MAX_PATH);
			CheckPortButton(hWnd, port, portname);
			break;

		case IDCANCEL:

			EndDialog(hWnd, wParam);
			break;

		case IDC_EMULATION:

			if(pTerm) pTerm->EmulationDialog(pTerm, GetParent(hWnd));
			break;

		case IDC_SETPORT:

			{
				LPCTSTR		pParm;
				int			nParm;
				int			ec;

				if (pTerm)
				{ 
					if ((ec = pTerm->PopParm(_T("Port Name"), ptString, pParm, nParm)) == errOK)
					{
						port = BserialStream::GetIndexOfPort(pParm);
						CheckPortButton(hWnd, port, pParm);
					}
				}
			}
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
int BviewRS232::SettingsDialog(HWND hWndParent)
{
	return DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_RS232CONN), hWndParent, RS232proc, (LPARAM)this);
}

