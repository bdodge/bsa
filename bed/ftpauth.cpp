
#include "bedx.h"

static LPTSTR g_ftpUser;
static LPTSTR g_ftpPass;
static bool   g_ftpAnon;
static bool   g_perstPass;

// STATIC
//**************************************************************************
BOOL CALLBACK FTPProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static Bed* pBed = NULL;

	RECT	rc;
	RECT	rcme;
	HWND	hwndParent;

	switch (message) 
	{
	case WM_INITDIALOG:

		pBed = (Bed*)lParam;
		if(pBed)
		{
			SetDlgItemText(hWnd, IDC_FTPUSER,	  g_ftpUser);
			SetDlgItemText(hWnd, IDC_FTPPASSWORD, g_ftpPass);
			CheckDlgButton(hWnd, IDC_FTPPERSTPASS,g_perstPass ? BST_CHECKED : BST_UNCHECKED);
			if(g_ftpAnon)
			{
				CheckDlgButton(hWnd, IDC_FTPANON, g_ftpAnon ? BST_CHECKED : BST_UNCHECKED);
				EnableWindow(GetDlgItem(hWnd, IDC_FTPUSER),     FALSE);
				EnableWindow(GetDlgItem(hWnd, IDC_FTPPASSWORD), FALSE);
				EnableWindow(GetDlgItem(hWnd, IDC_FTPPERSTPASS),FALSE);
			}
		}
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
		SetFocus(GetDlgItem(hWnd, (g_ftpUser[0] ? IDC_FTPPASSWORD : IDC_FTPUSER)));
		return FALSE;

	case WM_COMMAND:

		switch(wParam)
		{
		case IDC_FTPANON:

			if(pBed)
			{
				bool isAnon = IsDlgButtonChecked(hWnd, IDC_FTPANON) == 0;
			
				EnableWindow(GetDlgItem(hWnd, IDC_FTPUSER),      isAnon);
				EnableWindow(GetDlgItem(hWnd, IDC_FTPPASSWORD),  isAnon);
				EnableWindow(GetDlgItem(hWnd, IDC_FTPPERSTPASS), isAnon);
			}
			break;

		case IDOK:

			if(pBed)
			{
				GetDlgItemText(hWnd, IDC_FTPUSER,	  g_ftpUser, 127);
				GetDlgItemText(hWnd, IDC_FTPPASSWORD, g_ftpPass, 127);
				g_ftpAnon   = IsDlgButtonChecked(hWnd, IDC_FTPANON) != 0;
				g_perstPass = IsDlgButtonChecked(hWnd, IDC_FTPPERSTPASS) != 0;
				EndDialog(hWnd, IDOK);
			}
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

#define FTP_CRYPT_KEY 0xDEADFACE

//**************************************************************************
ERRCODE Bed::Encrypt(LPTSTR pd, int nd, LPCTSTR ps, int ns, DWORD key)
{
	int xd = 0;

	while(xd < nd-1 && xd < ns)
	{
		pd[xd] = ps[xd] ^ (TCHAR)key;
		xd++;
	}
	pd[xd] = _T('\0');
	if(xd < ns) return errOVERFLOW;
	return errOK;
}

//**************************************************************************
ERRCODE Bed::Decrypt(LPTSTR pd, int nd, LPCTSTR ps, int ns, DWORD key)
{
	int xd = 0;

	while(xd < nd-1 && xd < ns)
	{
		pd[xd] = ps[xd] ^ (TCHAR)key;
		xd++;
	}
	pd[xd] = _T('\0');
	if(xd < ns) return errOVERFLOW;
	return errOK;
}

//**************************************************************************
ERRCODE Bed::GetFTPUserName	(LPCTSTR& pu)
{
	ERRCODE ec;

	if(m_ftpAnonymous)
	{
		pu = _T("anonymous");
		return errOK;
	}
	if(! m_ftpUser[0])
	{
		ec = GetFTPAuthorization(false);
		if(ec != errOK)
			return ec;
		if(m_ftpAnonymous)
			return GetFTPUserName(pu);
	}
	pu = m_ftpUser;
	return errOK;
}

//**************************************************************************
ERRCODE Bed::GetFTPPassword	(LPCTSTR& pp)
{
	ERRCODE ec;

	if(m_ftpAnonymous)
	{
		pp = _T("beduser@");
		return errOK;
	}
	if(! m_ftpPassword[0])
	{
		ec = GetFTPAuthorization(false);
		if(ec != errOK)
			return ec;
		if(m_ftpAnonymous)
			return GetFTPPassword(pp);
	}
	pp = m_ftpPassword;
	return errOK;
}

//**************************************************************************
ERRCODE Bed::GetFTPAuthorization(bool hadfailed)
{
	TCHAR key[MAX_PATH];
	TCHAR pwd[64];
	int	  rc;

	g_ftpUser	= m_ftpUser;
	g_ftpPass	= m_ftpPassword;
	g_ftpAnon	= m_ftpAnonymous;
	g_perstPass = m_perstPass;

	if(m_perstPass && m_ftpPassword[0] == _T('\0') && m_ftpUser[0] != _T('\0'))
	{
		_sntprintf(key, MAX_PATH, _T("FTP/Users/" _Pfs_ "/Password"), m_ftpUser);
		m_persist->GetNvStr(key, pwd, 64, _T(""));
		Decrypt(m_ftpPassword, 128, pwd, _tcslen(pwd), FTP_CRYPT_KEY);
	}
	rc = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_FTPAUTH), m_hwnd, FTPProc, (LPARAM)this);

	g_ftpPass = NULL;

	if(rc == IDOK)
	{
		m_ftpAnonymous = g_ftpAnon;
		m_perstPass    = g_perstPass;
		m_persist->SetNvStr(_T("FTP/Username"), m_ftpUser);
		if(m_perstPass)
		{
			TCHAR key[MAX_PATH];
			TCHAR pwd[64];

			Encrypt(pwd, 64, m_ftpPassword, _tcslen(m_ftpPassword), FTP_CRYPT_KEY);
			_sntprintf(key, MAX_PATH, _T("FTP/Users/" _Pfs_ "/Password"), m_ftpUser);
			m_persist->SetNvStr(key, pwd);
		}
		m_persist->SetNvBool(_T("FTP/AnonymousLogin"), m_ftpAnonymous);
		m_persist->SetNvBool(_T("FTP/RememberPassword"), m_perstPass);
		g_perstPass = false;
		return errOK;
	}
	return errFAILURE;
}
