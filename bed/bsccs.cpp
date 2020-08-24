
#include "bedx.h"

// STATIC
//**************************************************************************
BOOL CALLBACK SCCSDialog(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Bsccs* pSCCS = NULL;

	RECT	rc;
	RECT	rcme;
	HWND	hwndString;

	TCHAR tmp[MAX_PATH];

	switch (message) 
	{
	case WM_INITDIALOG:

		pSCCS = (Bsccs*)lParam;
		if(! pSCCS) return FALSE;
		hwndString = GetParent(hWnd);
		if(!hwndString) hwndString = GetDesktopWindow();
		GetClientRect(hwndString, &rc);
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
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)pSCCS);

		pSCCS->GetShell(tmp, sizeof(tmp)/sizeof(TCHAR));
		SetDlgItemText(hWnd, IDC_SCCSSHELL, tmp);
		
		pSCCS->GetShellSwitches(tmp, sizeof(tmp)/sizeof(TCHAR));
		SetDlgItemText(hWnd, IDC_SCCSSHELLSWITCH, tmp);

		pSCCS->GetCheckoutCommand(tmp, sizeof(tmp)/sizeof(TCHAR));
		SetDlgItemText(hWnd, IDC_SCCS_CO, tmp);
		
		pSCCS->GetCheckinCommand(tmp, sizeof(tmp)/sizeof(TCHAR));
		SetDlgItemText(hWnd, IDC_SCCS_CI, tmp);
		
		pSCCS->GetRevertCommand(tmp, sizeof(tmp)/sizeof(TCHAR));
		SetDlgItemText(hWnd, IDC_SCCS_RV, tmp);
		
		SetFocus(GetDlgItem(hWnd, IDC_SCCSSHELL));
		return FALSE;

	case WM_COMMAND:

		switch(wParam)
		{
		case IDOK:

			pSCCS = (Bsccs*)GetWindowLong(hWnd, GWL_USERDATA);
			if(pSCCS)
			{
				TCHAR emsg[MAX_PATH * 2];

				emsg[0] = _T('\0');

				GetDlgItemText(hWnd, IDC_SCCS_CO, tmp, sizeof(tmp)/sizeof(TCHAR));
				if(! tmp[0])
					_sntprintf(emsg, MAX_PATH*2, _T("Check-Out Command not specified"));

				GetDlgItemText(hWnd, IDC_SCCS_CI, tmp, sizeof(tmp)/sizeof(TCHAR));
				if(! tmp[0])
					_sntprintf(emsg, MAX_PATH*2, _T("Check-In Command not specified"));


				GetDlgItemText(hWnd, IDC_SCCSSHELL, tmp, sizeof(tmp)/sizeof(TCHAR));
				if(! tmp[0])
					_sntprintf(emsg, MAX_PATH*2, _T("SCCS Shell program must be specified"));
				else if(BUtil::FileExists(tmp) != errOK)
					_sntprintf(emsg, MAX_PATH*2, _T("SCCS Shell %s not found, use FULL path"), tmp);

				GetDlgItemText(hWnd, IDC_SHELLSWITCH, tmp, sizeof(tmp)/sizeof(TCHAR));

				if(emsg[0])
				{
					int mc = MessageBox(NULL, emsg, _T("SCCS Settings Error"), MB_OK);
					break;
				}
				GetDlgItemText(hWnd, IDC_SCCSSHELL, tmp, sizeof(tmp)/sizeof(TCHAR));
				pSCCS->SetShell(tmp);

				GetDlgItemText(hWnd, IDC_SCCSSHELLSWITCH, tmp, sizeof(tmp)/sizeof(TCHAR));
				pSCCS->SetShellSwitches(tmp);

				GetDlgItemText(hWnd, IDC_SCCS_CO, tmp, sizeof(tmp)/sizeof(TCHAR));
				pSCCS->SetCheckoutCommand(tmp);

				GetDlgItemText(hWnd, IDC_SCCS_CI, tmp, sizeof(tmp)/sizeof(TCHAR));
				pSCCS->SetCheckinCommand(tmp);

				GetDlgItemText(hWnd, IDC_SCCS_RV, tmp, sizeof(tmp)/sizeof(TCHAR));
				pSCCS->SetRevertCommand(tmp);
			}
			EndDialog(hWnd, wParam);
			break;

		case IDCANCEL:

			EndDialog(hWnd, wParam);
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

// STATIC
//**************************************************************************
BOOL CALLBACK PerforceDialog(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Bsccs* pSCCS = NULL;

	RECT	rc;
	RECT	rcme;
	HWND	hwndString;
	bool	envclient, envserver, envauth, auth;
	int		checked, eachecked;

	TCHAR tmp[MAX_PATH];

	switch (message) 
	{
	case WM_INITDIALOG:

		pSCCS = (Bsccs*)lParam;
		if(! pSCCS) return FALSE;
		hwndString = GetParent(hWnd);
		if(!hwndString) hwndString = GetDesktopWindow();
		GetClientRect(hwndString, &rc);
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
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)pSCCS);

		pSCCS->GetP4client(tmp, sizeof(tmp)/sizeof(TCHAR));
		SetDlgItemText(hWnd, IDC_P4_CLIENT, tmp);
		
		pSCCS->GetP4port(tmp, sizeof(tmp)/sizeof(TCHAR));
		SetDlgItemText(hWnd, IDC_P4_PORT, tmp);

		pSCCS->GetP4user(tmp, sizeof(tmp)/sizeof(TCHAR));
		SetDlgItemText(hWnd, IDC_P4_USER, tmp);
		
		pSCCS->GetP4passwd(tmp, sizeof(tmp)/sizeof(TCHAR));
		SetDlgItemText(hWnd, IDC_P4_PASSWD, tmp);
		
		pSCCS->GetP4envclient(envclient);
		EnableWindow(GetDlgItem(hWnd, IDC_P4_CLIENT), envclient ? FALSE : TRUE);
		CheckDlgButton(hWnd, IDC_P4_ENV_CLIENT, envclient ? BST_CHECKED : BST_UNCHECKED);

		pSCCS->GetP4envport(envserver);
		EnableWindow(GetDlgItem(hWnd, IDC_P4_PORT), envserver ? FALSE : TRUE);
		CheckDlgButton(hWnd, IDC_P4_ENV_PORT, envserver ? BST_CHECKED : BST_UNCHECKED);

		pSCCS->GetP4envauth(envauth);
		pSCCS->GetP4auth(auth);
		EnableWindow(GetDlgItem(hWnd, IDC_P4_USER), (auth & ! envauth) ? TRUE : FALSE);
		EnableWindow(GetDlgItem(hWnd, IDC_P4_PASSWD), (auth & ! envauth) ? TRUE : FALSE);
		CheckDlgButton(hWnd, IDC_P4_AUTH, (auth) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_P4_ENV_AUTH, (envauth) ? BST_CHECKED : BST_UNCHECKED);

		SetFocus(GetDlgItem(hWnd, IDC_P4_CLIENT));
		return FALSE;

	case WM_COMMAND:

		switch(wParam)
		{
		case IDC_P4_ENV_CLIENT:

			checked = IsDlgButtonChecked(hWnd, IDC_P4_ENV_CLIENT) != 0;
			EnableWindow(GetDlgItem(hWnd, IDC_P4_CLIENT), checked ? FALSE : TRUE);
			break;

		case IDC_P4_ENV_PORT:

			checked = IsDlgButtonChecked(hWnd, IDC_P4_ENV_PORT) != 0;
			EnableWindow(GetDlgItem(hWnd, IDC_P4_PORT), checked ? FALSE : TRUE);
			break;

		case IDC_P4_ENV_AUTH:
		case IDC_P4_AUTH:

			checked = IsDlgButtonChecked(hWnd, IDC_P4_AUTH) != 0;
			eachecked = IsDlgButtonChecked(hWnd, IDC_P4_ENV_AUTH) != 0;
			EnableWindow(GetDlgItem(hWnd, IDC_P4_USER), (checked && ! eachecked) ? TRUE : FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_P4_PASSWD), (checked && ! eachecked) ? TRUE : FALSE);
			break;

		case IDC_P4SHELL:

			pSCCS = (Bsccs*)GetWindowLong(hWnd, GWL_USERDATA);
			if(pSCCS)
			{
				int rcv;

				rcv = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_SCCS), hWnd, SCCSDialog, (LPARAM)pSCCS);
			}
			break;

		case IDOK:

			pSCCS = (Bsccs*)GetWindowLong(hWnd, GWL_USERDATA);
			if(pSCCS)
			{
				TCHAR emsg[MAX_PATH];

				emsg[0] = _T('\0');

				if(IsDlgButtonChecked(hWnd, IDC_P4_ENV_CLIENT) == 0)
				{
					GetDlgItemText(hWnd, IDC_P4_CLIENT, tmp, sizeof(tmp)/sizeof(TCHAR));
					if(! tmp[0])
						_sntprintf(emsg, MAX_PATH, _T("Perforce Client not specified"));
				}
				if(IsDlgButtonChecked(hWnd, IDC_P4_ENV_PORT) == 0)
				{
					GetDlgItemText(hWnd, IDC_P4_PORT, tmp, sizeof(tmp)/sizeof(TCHAR));
					if(! tmp[0])
						_sntprintf(emsg, MAX_PATH, _T("Perforce Port not specified"));
				}
				if(IsDlgButtonChecked(hWnd, IDC_P4_AUTH) != 0)
				{
					if(IsDlgButtonChecked(hWnd, IDC_P4_ENV_AUTH) == 0)
					{
						GetDlgItemText(hWnd, IDC_P4_USER, tmp, sizeof(tmp)/sizeof(TCHAR));
						if(! tmp[0])
							_sntprintf(emsg, MAX_PATH, _T("Perforce User must be specified"));
						GetDlgItemText(hWnd, IDC_P4_PASSWD, tmp, sizeof(tmp)/sizeof(TCHAR));
						if(! tmp[0])
							_sntprintf(emsg, MAX_PATH, _T("Perforce Password must be specified"));
					}
				}
				if(emsg[0])
				{
					int mc = MessageBox(NULL, emsg, _T("Perforce Settings Error"), MB_OK);
					break;
				}
				if(IsDlgButtonChecked(hWnd, IDC_P4_ENV_CLIENT) == 0)
				{
					GetDlgItemText(hWnd, IDC_P4_CLIENT, tmp, sizeof(tmp)/sizeof(TCHAR));
					pSCCS->SetP4client(tmp);
					pSCCS->SetP4envclient(false);
				}
				else
				{
					pSCCS->SetP4envclient(true);
				}
				if(IsDlgButtonChecked(hWnd, IDC_P4_ENV_PORT) == 0)
				{
					GetDlgItemText(hWnd, IDC_P4_PORT, tmp, sizeof(tmp)/sizeof(TCHAR));
					pSCCS->SetP4port(tmp);
					pSCCS->SetP4envport(false);
				}
				else
				{
					pSCCS->SetP4envport(true);
				}
				if(IsDlgButtonChecked(hWnd, IDC_P4_AUTH) != 0)
				{
					if(IsDlgButtonChecked(hWnd, IDC_P4_ENV_AUTH) == 0)
					{
						GetDlgItemText(hWnd, IDC_P4_USER, tmp, sizeof(tmp)/sizeof(TCHAR));
						pSCCS->SetP4user(tmp);
						GetDlgItemText(hWnd, IDC_P4_PASSWD, tmp, sizeof(tmp)/sizeof(TCHAR));
						pSCCS->SetP4passwd(tmp);
						pSCCS->SetP4envauth(false);
					}
					else
					{
						pSCCS->SetP4envauth(true);
					}
					pSCCS->SetP4auth(true);
				}
				else
				{
					pSCCS->SetP4envauth((IsDlgButtonChecked(hWnd, IDC_P4_ENV_AUTH) != 0) ? true : false);
					pSCCS->SetP4auth(false);
				}
			}
			EndDialog(hWnd, wParam);
			break;

		case IDCANCEL:

			EndDialog(hWnd, wParam);
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
Bsccs::	Bsccs(Bed* pParent)
		:
		m_editor(pParent),
		m_p4_envport(true),
		m_p4_envclient(true),
		m_p4_auth(false)
{
	m_p4_passwd[0] = _T('\0');
	RestoreSettings();
}

//**************************************************************************
Bsccs::	~Bsccs()
{
}

//**************************************************************************
ERRCODE Bsccs::RestoreSettings()
{

#ifdef Windows
	_tcscpy(m_sccsswitch, _T("/C"));
	_tcscpy(m_sccssep, _T("&"));
#else
	_tcscpy(m_sccsswitch, _T("-c"));
	_tcscpy(m_sccssep, _T(";"));
#endif
	if(! (m_persist = m_editor->GetPersist()))
		return errFAILURE;

	m_persist->GetNvStr(_T("SCCS/Shell"), m_sccsshell, MAX_PATH, Bshell::GetDefaultShellName());
	m_persist->GetNvStr(_T("SCCS/ShellSwitch"), m_sccsswitch, 32, m_sccsswitch);
	m_persist->GetNvStr(_T("SCCS/ShellSep"), m_sccssep, 32, m_sccssep);
	m_persist->GetNvStr(_T("SCCS/Commands/CheckOut"), m_cocmd, MAX_SCCS_CMD, _T(""));
	m_persist->GetNvStr(_T("SCCS/Commands/CheckIn"), m_cicmd, MAX_SCCS_CMD, _T(""));
	m_persist->GetNvStr(_T("SCCS/Commands/CheckRevert"), m_rvcmd, MAX_SCCS_CMD, _T(""));

	// if perforce, setup perforce
	//
	if(_tcsstr(m_cocmd, _T("p4")))
	{
		TCHAR szDefault[MAX_PATH];
		TCHAR szParm[MAX_PATH];
		bool  noauth;
		bool  useenvclient, useenvserver, useenvauth;

		m_persist->GetNvBool(_T("SCCS/Perforce/ClientFromEnv"), useenvclient, 1);
		m_persist->GetNvBool(_T("SCCS/Perforce/PortFromEnv"), useenvserver, 1);
		m_persist->GetNvBool(_T("SCCS/Perforce/AuthFromEnv"), useenvauth, 1);

		SetP4envclient(useenvclient);
		SetP4envport(useenvserver);
		SetP4envauth(useenvauth);
		
#ifndef Windows
		char  aParm[256*3+2];
#endif

		int ok = 1;
		szDefault[0] = _T('\0');
#ifdef Windows
		if(! GetEnvironmentVariable(_T("P4CLIENT"), szDefault, 256))
			ok = 0;
#else
		char* p4client = getenv("P4CLIENT");
		if(! p4client)
			ok = 0;
		else
			CharToTChar(szDefault, p4client);				
#endif
		if(! ok)
		{
//			MessageBox(NULL, _T("Perforce client isn't set\nin your environment"),
//				_T("Bed - p4 integration"), MB_OK);
		}
		if(! useenvclient)
		{
			m_persist->GetNvStr(_T("SCCS/Perforce/Client"), szParm, 256, szDefault);
#ifdef Windows
			SetEnvironmentVariable(_T("P4CLIENT"), szParm);
#else
			UTF8ENCODE(aParm, szParm);
			setenv("P4CLIENT", aParm, 1);
#endif
		}
		else
		{
			_tcsncpy(szParm, szDefault, MAX_PATH-1);
			szParm[MAX_PATH-1] = _T('\0');
		}
		_tcsncpy(m_p4_client, szParm, MAX_PATH-1);
		m_p4_client[MAX_PATH-1] = _T('\0');

		ok = 1;
		szDefault[0] = _T('\0');

#ifdef Windows
		if(! GetEnvironmentVariable(_T("P4PORT"), szDefault, 256))
			ok = 0;
#else
		char* p4port = getenv("P4PORT");
		if(! p4port)
			ok = 0;
		else
			CharToTChar(szDefault, p4port);				
#endif
		if(! ok)
		{
//			MessageBox(NULL, _T("Perforce port isn't set\nin your environment"),
//				_T("Bed - p4 integration"), MB_OK);
		}
		if(! useenvserver)
		{
			m_persist->GetNvStr(_T("SCCS/Perforce/Port"), szParm, 256, szDefault);
#ifdef Windows
			SetEnvironmentVariable(_T("P4PORT"), szParm);
#else
			UTF8ENCODE(aParm, szParm);
			setenv("P4PORT", aParm, 1);
#endif
		}
		else
		{
			_tcsncpy(szParm, szDefault, MAX_PATH-1);
			szParm[MAX_PATH-1] = _T('\0');
		}
		_tcsncpy(m_p4_port, szParm, MAX_PATH-1);
		m_p4_port[MAX_PATH-1] = _T('\0');

		ok = 1;
		szDefault[0] = _T('\0');
#ifdef Windows
		if(! GetEnvironmentVariable(_T("P4USER"), szDefault, 256))
			szDefault[0] = _T('\0');
#else
		char* p4user = getenv("P4USER");
		if(p4user)
			CharToTChar(szDefault, p4user);				
#endif
		if(! useenvauth)
		{
			m_persist->GetNvStr(_T("SCCS/Perforce/User"), szParm, 256, szDefault);
#ifdef Windows
			SetEnvironmentVariable(_T("P4USER"), szParm);
#else
			UTF8ENCODE(aParm, szParm);
			setenv("P4USER", aParm, 1);
#endif
		}
		else
		{
			_tcsncpy(szParm, szDefault, MAX_PATH-1);
			szParm[MAX_PATH-1] = _T('\0');
		}
		_tcsncpy(m_p4_user, szParm, MAX_PATH-1);
		m_p4_user[MAX_PATH-1] = _T('\0');

		ok = 1;
		szDefault[0] = _T('\0');

#ifdef Windows
		if(! GetEnvironmentVariable(_T("P4PASSWD"), szDefault, 256))
			szDefault[0] = _T('\0');
#else
		char* p4passwd = getenv("P4PASSWD");
		if(p4passwd)
			CharToTChar(szDefault, p4passwd);				
#endif
		if(! useenvauth)
		{
			m_persist->GetNvStr(_T("SCCS/Perforce/Password"), szParm, MAX_PATH, szDefault);
#ifdef Windows
			SetEnvironmentVariable(_T("P4PASSWD"), szParm);
#else
			UTF8ENCODE(aParm, szParm);
			setenv("P4PASSWD", aParm, 1);
#endif
		}
		else
		{
			_tcsncpy(szParm, szDefault, MAX_PATH-1);
			szParm[MAX_PATH-1] = _T('\0');
		}
		_tcsncpy(m_p4_passwd, szParm, MAX_PATH-1);
		m_p4_passwd[MAX_PATH-1] = _T('\0');
	
		m_persist->GetNvBool(_T("SCCS/Perforce/NoAuth"), noauth, true);
		m_p4_auth = ! noauth;
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::SaveSettings()
{
	if(! (m_persist = m_editor->GetPersist()))
		return errFAILURE;

	m_persist->SetNvStr(_T("SCCS/Shell"), m_sccsshell);
	m_persist->SetNvStr(_T("SCCS/ShellSwitch"), m_sccsswitch);
	m_persist->SetNvStr(_T("SCCS/ShellSep"), m_sccssep);
	m_persist->SetNvStr(_T("SCCS/Commands/CheckOut"), m_cocmd);
	m_persist->SetNvStr(_T("SCCS/Commands/CheckIn"), m_cicmd);
	m_persist->SetNvStr(_T("SCCS/Commands/CheckRevert"), m_rvcmd);

	m_persist->SetNvStr(_T("SCCS/Perforce/Client"), m_p4_client);
	m_persist->SetNvStr(_T("SCCS/Perforce/Port"),   m_p4_port);
	m_persist->SetNvStr(_T("SCCS/Perforce/User"),   m_p4_user);
	m_persist->SetNvStr(_T("SCCS/Perforce/Password"), m_p4_passwd);
	m_persist->SetNvBool(_T("SCCS/Perforce/ClientFromEnv"), m_p4_envclient);
	m_persist->SetNvBool(_T("SCCS/Perforce/PortFromEnv"), m_p4_envport);
	m_persist->SetNvBool(_T("SCCS/Perforce/AuthFromEnv"), m_p4_envauth);
	m_persist->SetNvBool(_T("SCCS/Perforce/NoAuth"), ! m_p4_auth);

	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::EditSettings(HWND hWnd)
{
	int     rc;

	if(_tcsstr(m_cocmd, _T("p4")))
		rc = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_PERFORCE), hWnd, PerforceDialog, (LPARAM)this);
	else
		rc = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_SCCS), hWnd, SCCSDialog, (LPARAM)this);
	if(rc == IDOK)
	{
		SaveSettings();
		return errOK;
	}
	else
	{
		RestoreSettings();
		return errFAILURE;
	}
}

//**************************************************************************
ERRCODE Bsccs::GetShell(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_sccsshell, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::GetShellSwitches(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_sccsswitch, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::GetCheckoutCommand(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_cocmd, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::GetCheckinCommand(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_cicmd, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::GetRevertCommand(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_rvcmd, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::SetShell(LPCTSTR pStr)
{
	if(! pStr )  return errBAD_PARAMETER;
	_tcsncpy(m_sccsshell, pStr, MAX_PATH-1);
	m_sccsshell[MAX_PATH-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::SetShellSwitches(LPCTSTR pStr)
{
	if(! pStr)  return errBAD_PARAMETER;
	_tcsncpy(m_sccsswitch, pStr, 32-1);
	m_sccsswitch[32-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::SetCheckoutCommand(LPCTSTR pStr)
{
	if(! pStr)  return errBAD_PARAMETER;
	_tcsncpy(m_cocmd, pStr, MAX_SCCS_CMD-1);
	m_cocmd[MAX_SCCS_CMD-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::SetCheckinCommand(LPCTSTR pStr)
{
	if(! pStr)  return errBAD_PARAMETER;
	_tcsncpy(m_cicmd, pStr, MAX_SCCS_CMD-1);
	m_cicmd[MAX_SCCS_CMD-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::SetRevertCommand(LPCTSTR pStr)
{
	if(! pStr)  return errBAD_PARAMETER;
	_tcsncpy(m_rvcmd, pStr, MAX_SCCS_CMD-1);
	m_rvcmd[MAX_SCCS_CMD-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::GetP4client(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_p4_client, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::GetP4port(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_p4_port, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::GetP4user(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_p4_user, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::GetP4passwd(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_p4_passwd, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::SetP4client(LPCTSTR pStr)
{
	if(! pStr)  return errBAD_PARAMETER;
	_tcsncpy(m_p4_client, pStr, MAX_PATH-1);
	m_p4_client[MAX_PATH-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::SetP4port(LPCTSTR pStr)
{
	if(! pStr)  return errBAD_PARAMETER;
	_tcsncpy(m_p4_port, pStr, MAX_PATH-1);
	m_p4_port[MAX_PATH-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::SetP4user(LPCTSTR pStr)
{
	if(! pStr)  return errBAD_PARAMETER;
	_tcsncpy(m_p4_user, pStr, MAX_PATH-1);
	m_p4_user[MAX_PATH-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::SetP4passwd(LPCTSTR pStr)
{
	if(! pStr)  return errBAD_PARAMETER;
	_tcsncpy(m_p4_passwd, pStr, sizeof(m_p4_passwd)/sizeof(TCHAR)-1);
	m_p4_passwd[sizeof(m_p4_passwd)/sizeof(TCHAR)-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::GetP4envclient(bool& envcli)
{
	envcli = m_p4_envclient;
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::GetP4envport(bool& envsrv)
{
	envsrv = m_p4_envport;
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::GetP4envauth(bool& envauth)
{
	envauth = m_p4_envauth;
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::GetP4auth(bool& auth)
{
	auth = m_p4_auth;
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::SetP4envclient(bool envcli)
{
	m_p4_envclient = envcli;
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::SetP4envport(bool envsrv)
{
	m_p4_envport = envsrv;
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::SetP4envauth(bool envauth)
{
	m_p4_envauth = envauth;
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::SetP4auth(bool auth)
{
	m_p4_auth = auth;
	return errOK;
}

//**************************************************************************
//**************************************************************************
ERRCODE WINAPI Bsccs::RunStub(LPVOID thisptr)
{
	return ((Bsccs*)thisptr)->ShellThread();
}

//**************************************************************************
void PumpMessages()
{
	int rc;
	MSG msg;

	if((rc = PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) > 0)
	{
		if((rc = GetMessage(&msg, NULL, 0, 0)) > 0) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	else
	{
		Bthread::Sleep(100);
	}
}

//**********************************************************************
ERRCODE Bsccs::ShellThread()
{
	ERRCODE	ec;
	int		nRead;
	int		nRoom;
	int		empty = 100;

	ec = Init(m_sccs);
	if(ec != errOK) return ec;
	
	nResp = 0;
	m_shresp[0] = _T('\0');

	while((! Exited() || empty-- > 0) && m_running)
	{
		if(Poll(m_hsioRead, 0, 10000) == errSTREAM_DATA_PENDING)
		{			
			nRoom = sizeof(m_shresp) - nResp - 1;
			empty = 0;

			if(nRoom > 0)
			{				
				if((nRead = Read(m_hsioRead, m_shresp + nResp, nRoom)) > 0)
				{
					nResp += nRead;
					m_shresp[nResp] = '\0';
				}
			}
			else
			{
				PumpMessages();
			}
		}
		if(m_hsioErrRead != m_hsioRead && Poll(m_hsioErrRead, 0, 10000) == errSTREAM_DATA_PENDING)
		{
			empty = 0;
			if(nRoom > 0)
			{				
				nRoom = sizeof(m_shresp) - nResp - 1;

				if((nRead = Read(m_hsioErrRead, m_shresp + nResp, nRoom)) > 0)
				{
					nResp += nRead;
					m_shresp[nResp] = '\0';
				}
			}
			else
			{
				PumpMessages();
			}
		}
	}
	return ec;
}

//**************************************************************************
ERRCODE Bsccs::RunCommand(LPCTSTR filename, LPCTSTR cmd)
{
	TCHAR   szDir[MAX_PATH];
	TCHAR   szFile[MAX_PATH];
	LPTSTR  file = NULL;
	ERRCODE ec;
	DWORD	exitcode;

	if(filename)
	{
		GetFullPathName(filename, MAX_PATH, szDir, &file);
		if(file)
		{
			_tcscpy(szFile, file);
			*file = _T('\0');
		}
		else 
		{
			return errBAD_PARAMETER;
		}
		if(! szDir[0]) _tcscpy(szFile, _T("."));
	}
	else
	{
		szDir[0] = _T('\0');
		szFile[0] = _T('\0');
	}

#ifdef UNICODE
	_sntprintf(m_sccs, 1024, L"%ls %ls \"cd %ls %s %ls %ls\"\n",
			m_sccsshell, m_sccsswitch, szDir, m_sccssep, cmd, szFile);
#else
	_snprintf(m_sccs, 1024, "%s %s \"cd %s %ls %s %s\"\n",
			m_sccsshell, m_sccsswitch, szDir, m_sccssep, cmd, szFile);
#endif

	// If there is an SCCS info pane, run the command
	// in that pane, else run it locally here
	//
	BsccsInfo* pSCpane;

	nResp = 0;
	m_shresp[0] = _T('\0');

	pSCpane = (BsccsInfo*)m_editor->GetPane(_T("Info"), _T("Source Control"));
	if(pSCpane)
	{
		int wait = 0;

		ec = pSCpane->Startup(m_sccs);
		if(ec != errOK) return ec;

		// wait up to 60 seconds for sccs shell to finish
		// before returning
		//
		for(wait = 0; wait < 600; wait++)
		{
			if(pSCpane->IsDone())
				break;
			else
				PumpMessages();
		}
	}
	else
	{
		// run the shell in this thread
		//
		ec = Start(RunStub, this, false);
		ec = GetExitCode(exitcode);
	}
	return ec;
}

//**************************************************************************
bool Bsccs::IsReadonly(LPCTSTR lpFilename)
{
	struct stat finfo;
	char    aname[MAX_PATH];
	bool	readonly = true;

	TCharToChar(aname, lpFilename);

	if(! stat(aname, &finfo))
	{
		int mode;

		mode = finfo.st_mode;

#ifndef Windows
		if(finfo.st_uid != getuid())
		{
			mode = mode & 0077;
			mode |= ((mode & 0070) << 3);
		}
#endif
		readonly = ! (mode & 0200);
	}
	return readonly;
}

//**************************************************************************
ERRCODE Bsccs::FormatCommand(LPTSTR ncmd, LPCTSTR cmd)
{
	if(cmd[0] != 'p' || cmd[1] != '4' || (! m_p4_auth || m_p4_passwd[0] == _T('\0')))
	{
		_tcscpy(ncmd, cmd);
		return errOK;
	}
	_tcscpy(ncmd, _T("p4 -P "));
	_tcscat(ncmd, m_p4_passwd);
	_tcscat(ncmd, cmd + 2);
	return errOK;
}

//**************************************************************************
ERRCODE Bsccs::CheckOut(LPCTSTR filename, HWND hWnd)
{
	ERRCODE ec;
	TCHAR   emsg[MAX_PATH * 2];
	TCHAR	command[MAX_SCCS_CMD];

	if(! IsReadonly(filename))
	{
		_sntprintf(emsg, MAX_PATH*2, _T(""_Pfs_" is already writeable"), filename);
		MessageBox(hWnd, emsg, _T("BED SCCS Check-Out"), MB_OK);
		return errFAILURE;
	}
	FormatCommand(command, m_cocmd);
	ec = RunCommand(filename, command);

	if(ec == errOK)
	{
		if(IsReadonly(filename))
			ec = errFAILURE;
	}
	if(ec != errOK)
	{
		LPTSTR lpResp = new TCHAR [ nResp + 2 ];

		CharToTChar(lpResp, m_shresp);
		MessageBox(NULL, lpResp, _T("BED SCCS - Command Failed"), MB_OK);
		delete [] lpResp;
	}
	return ec;
}

//**************************************************************************
ERRCODE Bsccs::CheckIn(LPCTSTR filename, HWND hWnd)
{
	ERRCODE ec;
	TCHAR   emsg[MAX_PATH * 2];
	TCHAR	command[MAX_SCCS_CMD];

	if(IsReadonly(filename))
	{
		_sntprintf(emsg, MAX_PATH*2, _T(""_Pfs_" is not writeable"), filename);
		MessageBox(hWnd, emsg, _T("BED SCCS Check-In"), MB_OK);
		return errFAILURE;
	}

	FormatCommand(command, m_cicmd);
	ec = RunCommand(filename, command);

	if(ec == errOK)
	{
		if(! IsReadonly(filename))
			ec = errFAILURE;
	}
	if(ec != errOK)
	{
		LPTSTR lpResp = new TCHAR [ nResp + 2 ];

		CharToTChar(lpResp, m_shresp);
		MessageBox(NULL, lpResp, _T("BED SCCS - Command Failed"), MB_OK);
		delete [] lpResp;
	}
	return ec;
}


//**************************************************************************
ERRCODE Bsccs::Revert(LPCTSTR filename, HWND hWnd)
{
	ERRCODE ec;
	TCHAR   emsg[MAX_PATH * 2];
	TCHAR	command[MAX_SCCS_CMD];

	if(IsReadonly(filename))
	{
		_sntprintf(emsg, MAX_PATH*2, _T(""_Pfs_" is not writeable"), filename);
		MessageBox(hWnd, emsg, _T("BED SCCS Check-In"), MB_OK);
		return errFAILURE;
	}

	FormatCommand(command, m_rvcmd);
	ec = RunCommand(filename, command);

	if(ec == errOK)
	{
		if(! IsReadonly(filename))
			ec = errFAILURE;
	}
	if(ec != errOK)
	{
		LPTSTR lpResp = new TCHAR [ nResp + 2 ];

		CharToTChar(lpResp, m_shresp);
		MessageBox(NULL, lpResp, _T("BED SCCS - Command Failed"), MB_OK);
		delete [] lpResp;
	}
	return ec;
}


// STATIC
//**************************************************************************
char Bsccs::projectScript[] =

"Begin;"
"  Panel[panel(1), height(300)]:BED SCCS Setup Wizard;"
"    Description:Use this wizard to integrate a Source Code Control System;"
"    LeftImage;"
"    Next[default];Cancel;"
"  EndPanel;"

"  Panel[panel(2)]:SCCS Program;"
"    Description:Choose method for source control;"
"    IconImage;"
"    ChoiceGroup:Select which program you use for source control;"
"       Choice[default,panel(9) ]:SCCS;"
"       Choice[panel(3)         ]:Perforce;"
"       Choice[panel(7)         ]:Other;"
"    EndChoiceGroup;"
"    Back;Next[default];Cancel;"
"  EndPanel;"

"  Panel[panel(3)]:Perforce Setup;"
"    Description:Perforce Client and Port;"
"    IconImage;"
"    ChoiceGroup:Choose how to specify Client and Port?;"
"       Choice[default,panel(5) ]:Use Environment vars P4CLIENT and P4PORT;"
"       Choice[panel(4)         ]:Specify a client and port;"
"    EndChoiceGroup;"
"    Back;Next[panel(5),default];Cancel;"
"  EndPanel;"

"  Panel[panel(4)]:Perforce Setup;"
"    Description:Specify Perforce Client and Port;"
"    IconImage;"
"    Label:Perforce Client;"
"    Edit[sameline(100),width(200),var(string SCCS/Perforce/Client)];"
"    Label:Perforce Port;"
"    Edit[sameline(100),width(200),var(string SCCS/Perforce/Port)];"
"    Back;Next[panel(5),default];Cancel;"
"  EndPanel;"

"  Panel[panel(5)]:Perforce Setup;"
"    Description:Authentication;"
"    IconImage;"
"    ChoiceGroup:Does Server needs Authentication?;"
"       Choice[default,panel(9),var(bool SCCS/Perforce/NoAuth)]:No;"
"       Choice[panel(6)         ]:Yes;"
"    EndChoiceGroup;"
"    Back;Next[panel(9),default];Cancel;"
"  EndPanel;"

"  Panel[panel(6)]:Perforce Authentication;"
"    Description:Perforce User and Password;"
"    IconImage;"
"    Label:Perforce Username;"
"    Edit[sameline(100),width(200),var(string SCCS/Perforce/User)];"
"    Label:Perforce Password;"
"    Edit[sameline(100),width(200),var(string SCCS/Perforce/Password)];"
"    Back;Next[panel(9),default];Cancel;"
"  EndPanel;"

"  Panel[panel(7)]:Other SCCS Program - Shell;"
"    Description:Enter the shell to use for your SCCS;"
"    IconImage;"
"    Label:SCCS Shell;"
"    Edit[sameline(100),width(200),var(string SCCS/Shell)];"
"    Label:SCCS Switch;"
"    Edit[sameline(100),width(200),var(string SCCS/ShellSwitch)];"
"    Label:;"
"    Label:Shell Switch is any command line switch needed to force the;"
"    Label:shell your SCCS program will run in to read a command line.;"
"    Label:For example, enter -c for bash, or /C for dos (command.exe).;"
"    Back;Next[default];Cancel;"
"  EndPanel;"

"  Panel[panel(8)]:Other SCCS Program - Commands;"
"    Description:Enter the commands to use for your SCCS;"
"    IconImage;"
"    Label:SCCS Check-Out command;"
"    Edit[width(300),var(string SCCS/Commands/CheckOut)];"
"    Label:SCCS Check-In command;"
"    Edit[width(300),var(string SCCS/Commands/CheckIn)];"
"    Label:SCCS Revert command;"
"    Edit[width(300),var(string SCCS/Commands/CheckRevert)];"
"    Back;Next[default];Cancel;"
"  EndPanel;"

"  Panel[panel(9)]:Done;"
"    You have entered all needed information;"
"    IconImage;"
"    Label:Click Finish to begin using Source Control;"
"    Back;Finish[default];"
"  EndPanel;"

"  Panel[panel(8)]:Exit;"
"  EndPanel;"

"End;";

//**************************************************************************
bool Bsccs::OnWiz(int* step, wizMethod method, LPCTSTR typecode, LPCTSTR var, LPTSTR val)
{
	int iv;
	bool bv;

	switch(method)
	{
	case wizGetValue:
		if(! val) return false;
		switch(typecode[0])
		{
		case _T('i'):
			m_persist->GetNvInt(var, iv, 0);
			_sntprintf(val, 32, _T("%d"), iv);
			break;
		case _T('b'):
			m_persist->GetNvBool(var, bv, false);
			_sntprintf(val, 32, _T("%d"), bv ? 1 : 0);
			break;
		case _T('s'):
			if(! _tcscmp(var, _T("SCCS/Shell")))
				m_persist->GetNvStr(var, val, MAX_PATH);
			else
				m_persist->GetNvStr(var, val, MAX_SCCS_CMD);
			break;
		default:
			return false;
		}
		return true;

	case wizSetValue:
		if(! val) return false;
		switch(typecode[0])
		{
		case _T('i'):
			m_persist->SetNvInt(var, _tcstol(val, NULL, 0));
			break;
		case _T('b'):
			m_persist->SetNvBool(var, _tcstol(val, NULL, 0) != 0);
			break;
		case _T('s'):
			m_persist->SetNvStr(var, val);
			break;
		default:
			return false;
		}
		return true;

	case wizStepBack:
		break;

	case wizStepForward:
		if(*step == 2)
		{
			m_persist->SetNvStr(_T("SCCS/Commands/CheckIn"), _T("sccs delget"));
			m_persist->SetNvStr(_T("SCCS/Commands/CheckOut"), _T("sccs edit"));
			m_persist->SetNvStr(_T("SCCS/Commands/CheckRevert"), _T("sccs unedit"));
		}
		else if(*step == 3)
		{
			m_persist->SetNvStr(_T("SCCS/Commands/CheckIn"), _T("p4 submit"));
			m_persist->SetNvStr(_T("SCCS/Commands/CheckOut"), _T("p4 edit"));
			m_persist->SetNvStr(_T("SCCS/Commands/CheckRevert"), _T("p4 revert"));
			m_persist->SetNvBool(_T("SCCS/Perforce/NoAuth"), true);
		}
		else if(*step == 8)
		{
			TCHAR shell[MAX_PATH];

			m_persist->GetNvStr(_T("SCCS/Shell"), shell, MAX_PATH, _T(""));
			
			if(shell[0] == _T('\0'))
			{
				MessageBox(NULL, _T("Shell Program must be specified"), _T("BED - SCCS Setup"), MB_OK);
				*step = 4;
				return true;
			}
			else
			{
				TCHAR msg[MAX_PATH*2];

				// if file doesn't exists, complain
				//
				if(errOK != BUtil::FileExists(shell))
				{
					_sntprintf(msg, MAX_PATH*2, _T("Program "_Pfs_" not found.  Use full path to program"), shell);
					MessageBox(NULL, msg, _T("BED - SCCS Setup"), MB_OK);
					*step = 7;
					return true;
				}
			}
		}
		break;
	}
	return false;
}

// STATIC
//**************************************************************************
bool Bsccs::OnSCCSSetupData(LPVOID cookie, int* step, wizMethod method, LPCTSTR var, LPTSTR val)
{
	LPCTSTR tc;
	LPTSTR  td;
	Bsccs*  pSCCS;

	TCHAR   typecode[32] = { 0 };

	if(var)
	{
		// get type string;

		for(tc = var, td = typecode; *tc && *tc != _T(' ') && ((td - typecode) < 30);)
			*td++ = *tc++;
		while(*tc && *tc == _T(' '))
			tc++;
		*td = _T('\0');

	}
	pSCCS = (Bsccs*)cookie;
	
	if(pSCCS)
	{
		return pSCCS->OnWiz(step, method, typecode, tc, val);
	}
	return false;
}

//**************************************************************************
ERRCODE Bsccs::SCCSWizard()
{
	ERRCODE  ec = errOK;
	Bwizard* pwiz = NULL;
	LPTSTR   pdat;

	// create the wizard and start it off
	//
	pwiz = new Bwizard();
	pdat = new TCHAR [ sizeof(Bsccs::projectScript) + 2 ];
	CharToTChar(pdat, Bsccs::projectScript);

	pwiz->SetDefaultBitmapID(IDB_WIZMAP);
	pwiz->SetDefaultIconID(IDI_BED);
	ec = pwiz->Begin(_T("BED - SCCS Setup"), pdat, OnSCCSSetupData, (LPVOID)this, g_hInstance);

	if(ec == errOK)
	{
		m_persist->SetNvBool(_T("SCCS/Integration/IsSetup"), true);
		RestoreSettings();
	}
	else
	{
		SaveSettings();
	}
	delete [] pdat;
	return ec;
}

