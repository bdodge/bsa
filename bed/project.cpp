
#include "bedx.h"

#define MAX_INCLS (MAX_PATH*4)
#define BED_PROJECT_EXTENSION _T(".bpf")

// STATIC
//**************************************************************************
BOOL CALLBACK ProjectDialog(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Bproject* pProj = NULL;

	RECT	rc;
	RECT	rcme;
	HWND	hwndString;
	bool	inclincls;
	
	TCHAR tmp[MAX_INCLS];

	switch (message) 
	{
	case WM_INITDIALOG:
		pProj = (Bproject*)lParam;
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
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)pProj);
		if(pProj)
		{
			hwndString = GetDlgItem(hWnd, IDC_PROJNAME);
			if(hwndString)
			{
				SetWindowText(hwndString, (LPTSTR)pProj->GetName());
				if(! pProj->IsNew())
				{
					SetDlgItemText(hWnd, IDOK, _T("Save"));
					EnableWindow(hwndString, FALSE);
					EnableWindow(GetDlgItem(hWnd, IDC_SRCDIR), FALSE);
				}
				else
				{
					SetDlgItemText(hWnd, IDOK, _T("Create"));
				}
			}

			pProj->GetSourceDirectory(tmp, sizeof(tmp)/sizeof(TCHAR));
			SetDlgItemText(hWnd, IDC_SRCDIR, tmp);

			pProj->GetBuildDirectory(tmp, sizeof(tmp)/sizeof(TCHAR));
			SetDlgItemText(hWnd, IDC_BLDDIR, tmp);

			pProj->GetBuildCommand(tmp, sizeof(tmp)/sizeof(TCHAR));
			SetDlgItemText(hWnd, IDC_BLDCMD, tmp);

			pProj->GetBuildTarget(tmp, sizeof(tmp)/sizeof(TCHAR));
			SetDlgItemText(hWnd, IDC_BLDTARGET, tmp);

			pProj->GetBuildShell(tmp, sizeof(tmp)/sizeof(TCHAR));
			SetDlgItemText(hWnd, IDC_BLDSHELL, tmp);

			pProj->GetShellSwitches(tmp, sizeof(tmp)/sizeof(TCHAR));
			SetDlgItemText(hWnd, IDC_SHELLSWITCH, tmp);

			pProj->GetShellCommandSep(tmp, sizeof(tmp)/sizeof(TCHAR));
			SetDlgItemText(hWnd, IDC_SHELLSEP, tmp);

			pProj->GetIncludePaths(tmp, sizeof(tmp)/sizeof(TCHAR), inclincls);
			SetDlgItemText(hWnd, IDC_INCLPATHS, tmp);
			CheckDlgButton(hWnd, IDC_INCLINCLS, inclincls);
		}
		SetFocus(GetDlgItem(hWnd, IDC_PROJNAME));
		return FALSE;

	case WM_COMMAND:

		switch(wParam)
		{
		case IDOK:

			pProj = (Bproject*)GetWindowLong(hWnd, GWL_USERDATA);
			if(pProj)
			{
				TCHAR emsg[MAX_PATH * 2];

				emsg[0] = _T('\0');

				GetDlgItemText(hWnd, IDC_SRCDIR, tmp, sizeof(tmp)/sizeof(TCHAR));
				if(tmp[0])
					if(BUtil::DirectoryExists(tmp) != errOK)
						_sntprintf(emsg, MAX_PATH*2, _T("Source Directory "_Pfs_" doesn't exist"), tmp);
			
				GetDlgItemText(hWnd, IDC_BLDDIR, tmp, sizeof(tmp)/sizeof(TCHAR));
				if(! tmp[0])
					_sntprintf(emsg, MAX_PATH*2, _T("Build Directory must be specified"));
				else if(BUtil::DirectoryExists(tmp) != errOK)
					_sntprintf(emsg, MAX_PATH*2, _T("Build Directory "_Pfs_" doesn't exist"), tmp);

				GetDlgItemText(hWnd, IDC_BLDCMD, tmp, sizeof(tmp)/sizeof(TCHAR));
				if(! tmp[0])
					_sntprintf(emsg, MAX_PATH*2, _T("Build Command not specified"));

				GetDlgItemText(hWnd, IDC_BLDSHELL, tmp, sizeof(tmp)/sizeof(TCHAR));
				if(! tmp[0])
					_sntprintf(emsg, MAX_PATH*2, _T("Build Shell program must be specified"));
				else if(BUtil::FileExists(tmp) != errOK)
					_sntprintf(emsg, MAX_PATH*2, _T("Build Shell "_Pfs_" not found, use FULL path"), tmp);

				if(emsg[0])
				{
					int mc = MessageBox(NULL, emsg, _T("Project Settings Error"), MB_OK);
					break;
				}
				hwndString = GetDlgItem(hWnd, IDC_PROJNAME);
				if(hwndString)
				{
					if(pProj->IsNew())
					{
						GetWindowText(hwndString, tmp, sizeof(tmp)/sizeof(TCHAR));
						pProj->SetName(tmp);
					}
				}
				GetDlgItemText(hWnd, IDC_SRCDIR, tmp, sizeof(tmp)/sizeof(TCHAR));
				pProj->SetSourceDirectory(tmp);
			
				GetDlgItemText(hWnd, IDC_BLDDIR, tmp, sizeof(tmp)/sizeof(TCHAR));
				pProj->SetBuildDirectory(tmp);

				GetDlgItemText(hWnd, IDC_BLDCMD, tmp, sizeof(tmp)/sizeof(TCHAR));
				pProj->SetBuildCommand(tmp);

				GetDlgItemText(hWnd, IDC_BLDTARGET, tmp, sizeof(tmp)/sizeof(TCHAR));
				pProj->SetBuildTarget(tmp);

				GetDlgItemText(hWnd, IDC_BLDSHELL, tmp, sizeof(tmp)/sizeof(TCHAR));
				pProj->SetBuildShell(tmp);

				GetDlgItemText(hWnd, IDC_SHELLSWITCH, tmp, sizeof(tmp)/sizeof(TCHAR));
				pProj->SetShellSwitches(tmp);

				GetDlgItemText(hWnd, IDC_SHELLSEP, tmp, sizeof(tmp)/sizeof(TCHAR));
				pProj->SetShellCommandSep(tmp);

				GetDlgItemText(hWnd, IDC_INCLPATHS, tmp, sizeof(tmp)/sizeof(TCHAR));
				inclincls = IsDlgButtonChecked(hWnd, IDC_INCLINCLS) != 0;
				pProj->SetIncludePaths(tmp, inclincls);

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
Bproject::Bproject(Bed* pBed)
		:
		m_editor(pBed),
		m_persist(NULL),
		m_pdb(NULL),
		m_modified(false),
		m_usewiz(true),
		m_inclincls(true),
		m_new(false)
{
	m_inclPaths = new TCHAR [ MAX_INCLS ];
	memset(m_inclPaths, 0, MAX_INCLS * sizeof(TCHAR));

	m_name[0]		= 0;
	m_path[0]		= 0;
	m_srcdir[0]		= 0;
	m_bldshell[0]	= 0;
	m_bldcmd[0]		= 0;
	m_bldtarg[0]	= 0;
	m_blddir[0]		= 0;
	m_bldswitch[0]	= 0;

}

//**************************************************************************
Bproject::~Bproject()
{
	if(m_pdb)
		delete m_pdb;
	if(m_persist)
		delete m_persist;
	if(m_inclPaths)
		delete [] m_inclPaths;
}

//**************************************************************************
ERRCODE Bproject::Open(LPCTSTR pName)
{
	struct stat finfo;
	char   aname[MAX_PATH];
	TCHAR  fullPath[MAX_PATH];
	LPTSTR pTag;

	GetFullPathName(pName, MAX_PATH, fullPath, &pTag);
	if(pTag)
	{
		_tcsncpy(m_name, pTag, MAX_PATH-1);
		m_name[MAX_PATH-1] = _T('\0');
		*pTag = _T('\0');
	}
	_tcscpy(m_srcdir, fullPath);
	FormFilePath(m_name, m_srcdir, fullPath, MAX_PATH);
	TCharToChar(aname, fullPath);

	// see if file exists
	if(stat(aname, &finfo))
	{
		return errFAILURE;
	}
	_tcscpy(m_path, fullPath);

	m_new = false;
	if(m_pdb)
	{
		delete m_pdb;
		m_pdb = NULL;
	}
	if(m_persist)
	{
		delete m_persist;
		m_persist = NULL;
	}
	m_persist = new Bperstxml(pName);
	m_persist->SetFilePath(fullPath);

	m_persist->GetNvStr(PlatformStr(_T("ProjectDir")),			m_srcdir,	MAX_PATH);
	m_persist->GetNvStr(PlatformStr(_T("Build/Command")),		m_bldcmd,	MAX_PATH);
	m_persist->GetNvStr(PlatformStr(_T("Build/Target")),		m_bldtarg,	MAX_PATH);
	m_persist->GetNvStr(PlatformStr(_T("Build/Directory")),		m_blddir,	MAX_PATH);
	m_persist->GetNvStr(PlatformStr(_T("Build/Shell")),			m_bldshell, MAX_PATH);
	m_persist->GetNvStr(PlatformStr(_T("Build/Switches")),		m_bldswitch,MAX_PATH);
	m_persist->GetNvStr(PlatformStr(_T("Build/CommandSep")),	m_bldshellsep,MAX_PATH);
	m_persist->GetNvStr(PlatformStr(_T("Build/Paths")),			m_inclPaths,MAX_PATH * 4);
	m_persist->GetNvBool(PlatformStr(_T("Build/UseEnvIncls")),	m_inclincls, true);

	m_pdb = new Bpdb(m_srcdir);
	m_pdb->SetIncludePaths(m_inclPaths, m_inclincls);
	m_pdb->Start(Bpdb::RunStub, m_pdb);
	
	return errOK;
}

//**************************************************************************
ERRCODE Bproject::Save()
{
	if(m_persist)
	{
		m_persist->SetNvStr(PlatformStr(_T("ProjectDir")),		m_srcdir);
		m_persist->SetNvStr(PlatformStr(_T("Build/Command")),	m_bldcmd);
		m_persist->SetNvStr(PlatformStr(_T("Build/Target")),	m_bldtarg);
		m_persist->SetNvStr(PlatformStr(_T("Build/Directory")),	m_blddir);
		m_persist->SetNvStr(PlatformStr(_T("Build/Shell")),		m_bldshell);
		m_persist->SetNvStr(PlatformStr(_T("Build/Switches")),	m_bldswitch);
		m_persist->SetNvStr(PlatformStr(_T("Build/CommandSep")),m_bldshellsep);
		m_persist->SetNvStr(PlatformStr(_T("Build/Paths")),		m_inclPaths);
		m_persist->SetNvBool(PlatformStr(_T("Build/UseEnvIncls")),m_inclincls);
		m_persist->Flush(NULL, txtANSI);
		m_modified = false;
		m_new      = false;
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bproject::SaveViews(void)
{
	Bview*  pv;
	ERRCODE ec;
	int     i;
	TCHAR   key[MAX_PATH + 128];

	if(m_persist)
	{
		for(i = 0, pv = m_editor ? m_editor->GetViews() : NULL; pv; pv = pv->GetNext(pv), i++)
		{
			_sntprintf(key, MAX_PATH + 128, _T("Project/View%d"), i);
			m_persist->SetNvStr(PlatformStr(key), pv->GetBuffer()->GetName());
		}
		ec = m_persist->SetNvInt(PlatformStr(_T("Project/Views")), i);
		return ec;
	}
	else
	{
		return errFAILURE;
	}
}

//**************************************************************************
ERRCODE Bproject::RestoreViews(void)
{
	ERRCODE ec;
	TCHAR  key[MAX_PATH + 128], filename[MAX_PATH];
	int     i, j;

	if(m_persist)
	{
		ec = m_persist->GetNvInt(PlatformStr(_T("Project/Views")), i, 0);
		if(ec != errOK) return ec;
		if(i == 0) return errOK;

		for(j = 0; j < i; j++)
		{
			_sntprintf(key, MAX_PATH + 128, _T("Project/View%d"), j);
			ec = m_persist->GetNvStr(PlatformStr(key), filename, MAX_PATH, _T(""));
			if(ec == errOK && filename[0] != _T('\0'))
			{
				ec = m_editor->EditBuffer(filename, false);
			}
		}
		return errOK;
	}
	else
	{
		return errFAILURE;
	}
}

//**************************************************************************
ERRCODE Bproject::SaveBreakpoint(int n, Bbreak* pbp)
{
	if(! pbp || n < 0 || n > BED_MAX_BREAKPOINTS)
	{
		return errBAD_PARAMETER;
	}
	if(m_persist)
	{
		ERRCODE ec;
		TCHAR   key[MAX_PATH];
		LPTSTR  pk;
		int		dex;

		dex = _sntprintf(key, MAX_PATH, _T("Build/Break%d/"), n);
		pk = key + dex;

		_tcscpy(pk, _T("Name"));
		ec = m_persist->SetNvStr(key,	pbp->m_name ? pbp->m_name : _T(""));
		_tcscpy(pk, _T("Line"));
		m_persist->SetNvInt(key,	pbp->m_line);
		_tcscpy(pk, _T("Condition"));
		m_persist->SetNvStr(key,	pbp->m_cond ? pbp->m_cond : _T(""));
		_tcscpy(pk, _T("Count"));
		m_persist->SetNvInt(key,	pbp->m_cnt);
		_tcscpy(pk, _T("valid"));
		m_persist->SetNvBool(key,	pbp->m_valid);
		_tcscpy(pk, _T("enabled"));
		m_persist->SetNvBool(key,	pbp->m_enabled);
		_tcscpy(pk, _T("Type"));
		m_persist->SetNvInt(key,	pbp->m_type);

		ec = m_persist->Flush(NULL, txtANSI);
		return ec;
	}
	return errFAILURE;
}

//**************************************************************************
ERRCODE Bproject::RestoreBreakpoint(int n, Bbreak* pbp)
{
	if(! pbp || n < 0 || n > BED_MAX_BREAKPOINTS)
	{
		return errBAD_PARAMETER;
	}
	if(m_persist)
	{
		ERRCODE ec;
		TCHAR   key[MAX_PATH * 2];
		TCHAR	txt[MAX_PATH];
		LPTSTR  pk;
		int		dex;

		dex = _sntprintf(key, MAX_PATH * 2, _T("Build/Break%d/"), n);
		pk = key + dex;

		_tcscpy(pk, _T("Name"));
		ec = m_persist->GetNvStr(key,	txt, MAX_PATH, _T(""));
		if(pbp->m_name) delete [] pbp->m_name;
		pbp->m_name = new TCHAR [ _tcslen(txt) + 2 ];
		_tcscpy(pbp->m_name, txt);

		_tcscpy(pk, _T("Line"));
		m_persist->GetNvInt(key,	pbp->m_line);

		_tcscpy(pk, _T("Condition"));
		m_persist->GetNvStr(key,	txt, MAX_PATH, _T(""));
		if(pbp->m_cond) delete [] pbp->m_cond;
		pbp->m_cond = new TCHAR [ _tcslen(txt) + 2 ];
		_tcscpy(pbp->m_cond, txt);

		_tcscpy(pk, _T("Count"));
		m_persist->GetNvInt(key,	pbp->m_cnt, 0);
		_tcscpy(pk, _T("valid"));
		m_persist->GetNvBool(key,	pbp->m_valid, false);
		_tcscpy(pk, _T("enabled"));
		m_persist->GetNvBool(key,	pbp->m_enabled, false);
		_tcscpy(pk, _T("Type"));
		m_persist->GetNvInt(key,	(int&)pbp->m_type, bpProgram);

		pbp->m_ref   = 0;
		pbp->m_index = n;
		return ec;
	}
	return errFAILURE;
}

//**************************************************************************
ERRCODE Bproject::ProjectSetup(HWND hWnd)
{
	int     rc;

	rc = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_PROJECT), hWnd, ProjectDialog, (LPARAM)this);
	if(rc == IDOK)
	{
		TCHAR fullPath[MAX_PATH];

		if(! m_persist)	m_persist = new Bperstxml(m_name);
		FormFilePath(m_name, m_srcdir, fullPath, MAX_PATH);
		_tcscpy(m_path, fullPath);
		m_persist->SetFilePath(fullPath);

		Save();
		return Open(m_name);
	}
	return errFAILURE;
}

//**************************************************************************
LPCTSTR Bproject::PlatformStr(LPCTSTR src)
{
	LPCTSTR platform;

#ifdef Windows
	platform = _T("Windows/");
#elif defined(Linux)
	platform = _T("Linux/");
#else
	platform = _T("Unix/");
#endif
	_sntprintf(m_platbuf, MAX_PATH + 128, _T(""_Pfs_""_Pfs_""), platform, src);
	return m_platbuf;
}

//**************************************************************************
ERRCODE Bproject::GetSourceDirectory(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_srcdir, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bproject::GetBuildDirectory(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_blddir, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bproject::GetBuildCommand(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_bldcmd, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bproject::GetBuildTarget(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_bldtarg, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bproject::GetBuildShell(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_bldshell, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bproject::GetShellSwitches(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_bldswitch, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bproject::GetShellCommandSep(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_bldshellsep, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bproject::GetIncludePaths(LPTSTR pStr, int nStr, bool &inclincls)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_inclPaths, nStr-1);
	pStr[nStr-1] = _T('\0');
	inclincls = m_inclincls;
	return errOK;
}

//**************************************************************************
ERRCODE Bproject::GetProjectNamePath(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_path, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bproject::SetName(LPCTSTR pStr)
{
	if(! pStr)  return errBAD_PARAMETER;
	_tcsncpy(m_name, pStr, MAX_PATH-1);
	m_name[MAX_PATH-1] = _T('\0');
	return errOK;
}


//**************************************************************************
ERRCODE Bproject::SetSourceDirectory(LPCTSTR pStr)
{
	if(! pStr)  return errBAD_PARAMETER;
	_tcsncpy(m_srcdir, pStr, MAX_PATH-1);
	m_srcdir[MAX_PATH-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bproject::SetBuildDirectory(LPCTSTR pStr)
{
	if(! pStr)  return errBAD_PARAMETER;
	_tcsncpy(m_blddir, pStr, MAX_PATH-1);
	m_blddir[MAX_PATH-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bproject::SetBuildCommand(LPCTSTR pStr)
{
	if(! pStr)  return errBAD_PARAMETER;
	_tcsncpy(m_bldcmd, pStr, MAX_PATH-1);
	m_bldcmd[MAX_PATH-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bproject::SetBuildTarget(LPCTSTR pStr)
{
	if(! pStr)  return errBAD_PARAMETER;
	_tcsncpy(m_bldtarg, pStr, MAX_PATH-1);
	m_bldtarg[MAX_PATH-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bproject::SetBuildShell(LPCTSTR pStr)
{
	if(! pStr )  return errBAD_PARAMETER;
	_tcsncpy(m_bldshell, pStr, MAX_PATH-1);
	m_bldshell[MAX_PATH-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bproject::SetShellSwitches(LPCTSTR pStr)
{
	if(! pStr)  return errBAD_PARAMETER;
	_tcsncpy(m_bldswitch, pStr, MAX_PATH-1);
	m_bldswitch[MAX_PATH-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bproject::SetShellCommandSep(LPCTSTR pStr)
{
	if(! pStr)  return errBAD_PARAMETER;
	_tcsncpy(m_bldshellsep, pStr, MAX_PATH-1);
	m_bldshellsep[MAX_PATH-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bproject::SetIncludePaths(LPCTSTR pStr, bool inclincls)
{
	if(! pStr)  return errBAD_PARAMETER;
	_tcsncpy(m_inclPaths, pStr, MAX_INCLS-1);
	m_inclPaths[MAX_INCLS-1] = _T('\0');
	m_inclincls = inclincls;
	return errOK;
}

// STATIC
//**************************************************************************
ERRCODE Bproject::FormFilePath(LPCTSTR pName, LPCTSTR pSrcPath, LPTSTR pPath, int nPath)
{
	TCHAR projPath[MAX_PATH];

	if(! pPath || nPath < 8) return errBAD_PARAMETER;
	if(pSrcPath)
	{
		LPTSTR pe;

		_tcscpy(projPath, pSrcPath);
		pe = (LPTSTR)projPath + _tcslen(projPath) - 1;
		if(pe >= projPath && pe[0] != _PTC_)
		{
			*++pe = _PTC_;
			*++pe = _T('\0');
		}
	}
	else
	{
		projPath[0] = _T('\0');
	}
	_tcscat(projPath, pName);
	GetFullPathName(projPath, nPath - 4, pPath, NULL);
	if(! _tcsstr(pPath, BED_PROJECT_EXTENSION))
		_tcscat(pPath, BED_PROJECT_EXTENSION);
	return errOK;
}

// STATIC
//**************************************************************************
ERRCODE Bproject::OpenDefaultProject(LPCTSTR pPath, Bed* pEditor, bool restoreIt, bool forceOpen)
{
	TCHAR   defname[MAX_PATH];
	TCHAR   basicdefname[MAX_PATH];
	TCHAR	projname[MAX_PATH];
	TCHAR	projpattern[MAX_PATH];
	char	cwd[MAX_PATH];
	LPTSTR	pname;
	ERRCODE ec;
	
	// see if there is a project file in the current
	// directory, and open it if there is
	//
	if(! pPath)
	{
		defname[0] = _T('\0');
		if((getcwd(cwd, MAX_PATH)) != NULL)
		{
			CharToTChar(defname, cwd);
		}
	}
	else
	{
		_tcsncpy(defname, pPath, MAX_PATH-1);
		defname[MAX_PATH-1] = _T('\0');
	}

	// get just the directory name in the path
	//
	if(defname[0])
	{
		HDIRLIST		hDir;
		bool			isDir, isLink, isReadonly;

		// take path term char off end of defname
		//
		pname = defname + _tcslen(defname);
		if(*--pname == _PTC_)
			*pname = _T('\0');

		// point to dirname portion of path 
		// as name for project file
		//
		while(pname >= defname)
			if(*--pname == _PTC_)
				break;
		if(*pname == _PTC_)
			pname++;

		_tcscpy(basicdefname, pname);

		// see if there is a project like that
		//
		Bproject::FormFilePath(pname, defname, projname, MAX_PATH);

		if(BUtil::FileExists(projname) == errOK)
		{
			ec = pEditor->OpenProject(pname, restoreIt);
			return ec;
		}
		do
		{
			// no project dirname.xxx in current dir, try any file *.xxx
			// and go up dirs until one is found
			//

			// form currentpath/*.xxx
			//
			Bproject::FormFilePath(_T("*"), defname, projpattern, MAX_PATH);

			// list all files like that
			//
			ec = BfileInfo::ListDirectory(hDir, projpattern);
			if(ec == errOK)
			{
				do
				{
					ec = BfileInfo::NextFile(hDir, (LPCTSTR&)pname, isDir, isLink, isReadonly);
					if(ec == errOK)
					{
						if(! isDir)
						{
							LPCTSTR pRem;
							
							// found a file path/*.xxx* in this dir
							//
							_tcscpy(projname, pname);

							// pattern match the name 
							pRem = BUtil::SimplePatternMatch(projname, projpattern);

							if(pRem && ! *pRem)
							{
								// get rid of extension on projname
								//
								for(pname = projname + _tcslen(projname); pname > projname; pname--)
								{
									if(*(pname - 1) == _T('.'))
									{
										*(pname - 1) = _T('\0');
										break;
									}
								}
								BfileInfo::EndListDirectory(hDir);
								ec = pEditor->OpenProject(projname, restoreIt);
								return ec;
							}
						}
					}
				}
				while(ec == errOK);

				// got to end of dir listing w/o any matches, so go up a level if possible
				//
				for(pname = defname + _tcslen(defname) - 1; pname > defname; pname--)
				{
					if(*pname == _PTC_)
					{
						*pname = _T('\0');
						ec = errOK;
						break;
					}
				}
			}
		}
		while(ec == errOK);
	}

	// no project files anywhere, and need one, so prompt
	//
	if(forceOpen)
	{
		TCHAR msg[MAX_PATH*2];
		int   rc;

		_sntprintf(msg, MAX_PATH*2, _T("No default project \""_Pfs_"\", Create ?"), basicdefname);
		rc = MessageBox(NULL, msg, pEditor->GetVersion(), MB_OKCANCEL);
		if(rc == IDCANCEL) return errFAILURE;

		ec = pEditor->NewProject();
		return ec;
	}
	return errFAILURE;
}



// STATIC
//**************************************************************************
char Bproject::projectScript[] =

"Begin;"
"  Panel[panel(1), height(300)]:BED Project Setup Wizard;"
"    Description:Use this wizard to create a build project;"
"    LeftImage;"
"    Next[default];Cancel;"
"  EndPanel;"

"  Panel[panel(2)]:Project Setup;"
"    Description:Choose name and setup method;"
"    IconImage;"
"    Label:Project Name:;"
"    Edit[sameline(100),width(160),var(string ProjectName)];"
"    Label:Source Directory:;"
"    Edit[sameline(100),width(160),var(string ProjectDir)];"
"    Label:;"
"    ChoiceGroup:Select how you would like to enter project details;"
"       Choice[default,panel(3),                      ]:I want to continue using the wizard;"
"       Choice[panel(10),var(bool Build/Wizard/Expert) ]:I want to use the project dialog;"
"    EndChoiceGroup;"
"    Back;Next[default];Cancel;"
"  EndPanel;"

"  Panel[panel(3)]:Project Build Method;"
"    Description:Enter command line used to build or make your project;"
"    IconImage;"
"    Label:Build Command;"
"    Edit[sameline(100),width(200),var(string Build/Command)];"
"    Label:Build Target;"
"    Edit[sameline(100),width(200),var(string Build/Target)];"
"    Label:;"
"    Label:Include any switches to your make program in the;"
"    Label:build command line, but do not include directory changes,;"
"    Label:You will get to specify the build directory next.;"
"    Back;Next[default];Cancel;"
"  EndPanel;"

"  Panel[panel(4)]:Project Build Directory;"
"    Description:Enter the directory where build/make takes place;"
"    IconImage;"
"    Label:Build Directory;"
"    Edit[sameline(100),width(200),var(string Build/Directory)];"
"    Back;Next[default];Cancel;"
"  EndPanel;"

"  Panel[panel(5)]:Project Build Shell;"
"    Description:Enter the shell program used for building;"
"    IconImage;"
"    Label:Build Shell;"
"    Edit[sameline(100),width(200),var(string Build/Shell)];"
"    Label:;"
"    Label:Use full path to shell program, e.g. /bin/bash for;"
"    Label:Linux or \\Windows\\System32\\cmd.exe for Windows;"
"    Back;Next[default];Cancel;"
"  EndPanel;"

"  Panel[panel(6)]:Build Shell Switche(s);"
"    Description:Enter the shell program used for building;"
"    IconImage;"
"    Label:Build Switche(s);"
"    Edit[sameline(100),width(200),var(string Build/Switches)];"
"    Label:;"
"    Label:Enter any command line switches needed to have the build;"
"    Label:shell read from the command line arguments, e.g, use -c;"
"    Label:for bash - Linux, or /c for cmd.exe - Windows.;"
"    Back;Next[default];Cancel;"
"  EndPanel;"

"  Panel[panel(7)]:Build Shell Command Separator;"
"    Description:Enter the command line command separator string;"
"    IconImage;"
"    Label:Command Separator;"
"    Edit[sameline(140),width(200),var(string Build/CommandSep)];"
"    Label:;"
"    Label:Enter any special charater(s) used to separate;"
"    Label:individual commands to the shell program, e.g.;"
"    Label:a semicolon for bash - Linux, or \"&&&&\" for cmd.exe - Windows.;"
"    Back;Next[default];Cancel;"
"  EndPanel;"

"  Panel[panel(8)]:Include Paths;"
"    Description:Enter additional include file paths;"
"    IconImage;"
"    Label:Include Path;"
"    Edit[sameline(100),width(200),var(string Build/Paths)];"
"    Checkbox[var(string Build/UseEnvPaths)]:Also use Environment Include Path;"
"    Label:;"
"    Label:Include file paths are used to generate program;"
"    Label:source browsing database information.  Use commas;"
"    Label:or semicolons to separate individual paths.;"
"    Back;Next[default];Cancel;"
"  EndPanel;"

"  Panel[panel(9)]:Done;"
"    IconImage;"
"    Label:Click Finish to begin using your project;"
"    Label:;"
"    Label:The project will be saved in the current directory as;"
"    Label[font(1),var(string ProjectFileName)]:;"
"    Back;Finish[default];"
"  EndPanel;"

"  Panel[panel(10)]:Exit;"
"  EndPanel;"

"End;";

//**************************************************************************
bool Bproject::OnWiz(int* step, wizMethod method, LPCTSTR typecode, LPCTSTR var, LPTSTR val)
{
	TCHAR fullPath[MAX_PATH + 6];
	TCHAR msg[MAX_PATH*2];

	switch(method)
	{
	case wizGetValue:
		if(! val) return false;
		if(! _tcscmp(var, _T("ProjectFileName")))
		{
			if(m_persist)
			{
				m_persist->GetFilePath(val, MAX_PATH);
				return true;
			}
		}
		if(! _tcscmp(var, _T("ProjectName")))
		{
			_tcsncpy(val, m_name, MAX_PATH-1);
			return true;
		}
		if(! _tcscmp(var, _T("ProjectDir")))
		{
			_tcsncpy(val, m_srcdir, MAX_PATH-1);
			return true;
		}
		else if(! _tcscmp(var, _T("Build/Command")))
		{
			_tcsncpy(val, m_bldcmd, MAX_PATH-1);
			return true;
		}
		else if(! _tcscmp(var, _T("Build/Target")))
		{
			_tcsncpy(val, m_bldtarg, MAX_PATH-1);
			return true;
		}
		else if(! _tcscmp(var, _T("Build/Directory")))
		{
			_tcsncpy(val, m_blddir, MAX_PATH-1);
			return true;
		}
		else if(! _tcscmp(var, _T("Build/Shell")))
		{
			_tcsncpy(val, m_bldshell, MAX_PATH-1);
			return true;
		}
		else if(! _tcscmp(var, _T("Build/Switches")))
		{
			_tcsncpy(val, m_bldswitch, MAX_PATH-1);
			return true;
		}
		else if(! _tcscmp(var, _T("Build/CommandSep")))
		{
			_tcsncpy(val, m_bldshellsep, MAX_PATH-1);
			return true;
		}
		else if(! _tcscmp(var, _T("Build/Paths")))
		{
			_tcsncpy(val, m_inclPaths, MAX_INCLS-1);
			return true;
		}
		else if(! _tcscmp(var, _T("Build/UseEnvPaths")))
		{
			_sntprintf(val, 32, _T("%d"), m_inclincls);
			return true;
		}
		else if(! _tcscmp(var, _T("Build/Wizard/Expert")))
		{
			_sntprintf(val, 32, _T("%d"), ! m_usewiz);
			return true;
		}
		return false;

	case wizSetValue:
		if(! val) return false;
		if(! _tcscmp(var, _T("ProjectName")))
		{
			_tcsncpy(m_name, val, MAX_PATH-1);
			m_name[MAX_PATH-1] = _T('\0');
			return true;
		}
		else if(! _tcscmp(var, _T("ProjectDir")))
		{
			_tcsncpy(m_srcdir, val, MAX_PATH-1);
			m_srcdir[MAX_PATH-1] = _T('\0');
			return true;
		}
		else if(! _tcscmp(var, _T("Build/Command")))
		{
			_tcsncpy(m_bldcmd, val, MAX_PATH-1);
			m_bldcmd[MAX_PATH-1] = _T('\0');
			return true;
		}
		else if(! _tcscmp(var, _T("Build/Target")))
		{
			_tcsncpy(m_bldtarg, val, MAX_PATH-1);
			m_bldcmd[MAX_PATH-1] = _T('\0');
			return true;
		}
		else if(! _tcscmp(var, _T("Build/Directory")))
		{
			_tcsncpy(m_blddir, val, MAX_PATH-1);
			m_blddir[MAX_PATH-1] = _T('\0');
			return true;
		}
		else if(! _tcscmp(var, _T("Build/Shell")))
		{
			_tcsncpy(m_bldshell, val, MAX_PATH-1);
			m_bldshell[MAX_PATH-1] = _T('\0');
			return true;
		}
		else if(! _tcscmp(var, _T("Build/Switches")))
		{
			_tcsncpy(m_bldswitch, val, MAX_PATH-1);
			m_bldswitch[MAX_PATH-1] = _T('\0');
			return true;
		}
		else if(! _tcscmp(var, _T("Build/CommandSep")))
		{
			_tcsncpy(m_bldshellsep, val, MAX_PATH-1);
			m_bldshellsep[MAX_PATH-1] = _T('\0');
			return true;
		}
		else if(! _tcscmp(var, _T("Build/Paths")))
		{
			_tcsncpy(m_inclPaths, val, MAX_INCLS-1);
			m_inclPaths[MAX_INCLS-1] = _T('\0');
			return true;
		}
		else if(! _tcscmp(var, _T("Build/UseEnvPaths")))
		{
			m_inclincls = _tcstol(val, NULL, 0) != 0;
			return true;
		}
		else if(! _tcscmp(var, _T("Build/Wizard/Expert")))
		{
			m_usewiz = !(_tcstol(val, NULL, 0) != 0);
			return true;
		}
		return false;

	case wizStepBack:
		break;

	case wizStepForward:
		if(*step == 3)
		{
			if(m_name[0] == _T('\0') && m_usewiz)
			{
				MessageBox(NULL, _T("Project name must be specified"), _T("Project Setup"), MB_OK);
				*step = 2;
				return true;
			}
			else
			{
				// if project file exists, ask to overwrite
				//
				FormFilePath(m_name, m_srcdir, fullPath, MAX_PATH);
				if(errOK == BUtil::FileExists(fullPath))
				{
					int rc;

					_sntprintf(msg, MAX_PATH*2, _T("Modify existing project "_Pfs_" ?"), fullPath);
					rc = MessageBox(NULL, msg, _T("BED - Project Setup"), MB_OKCANCEL);

					if(rc == IDOK)
					{
						LPTSTR pdot = fullPath;

						while(pdot && *pdot && *pdot != _T('.'))
							pdot++;
						if(*pdot)
							*pdot = _T('\0');
						Open(fullPath);
						return false;
					}
					else
					{
						m_name[0] = _T('\0');
						*step = 2;
						return true;
					}
				}
			}
		}
		else if(*step == 5)
		{
			//if(m_blddir[0] == _T('\0'))
			//	_tcscpy(m_blddir, _T("./"));
			
			// make sure build dir exists
			//
			GetFullPathName(m_blddir, MAX_PATH, fullPath, NULL);
			
			if(errOK != BUtil::DirectoryExists(fullPath))
			{
				_sntprintf(msg, MAX_PATH*2, _T("Directory "_Pfs_" doesn't exist"), fullPath);
				MessageBox(NULL, msg, _T("BED - Project Setup"), MB_OK);
				*step = 4;
				return true;
			}
			else
			{
				_tcscpy(m_blddir, fullPath);
			}
		}
		if(*step == 6)
		{
			if(m_bldshell[0] == _T('\0'))
			{
				MessageBox(NULL, _T("Build Command must be specified"), _T("BED - Project Setup"), MB_OK);
				*step = 5;
				return true;
			}
			else
			{
#if 0 // assume the user knows enough to use the default and that forking the shell can find the program
				// if file doesn't exists, complain
				//
				if(errOK != BUtil::FileExists(m_bldshell))
				{
					_sntprintf(msg, MAX_PATH*2, _T("Program "_Pfs_" not found.  Use full path to program"), m_bldshell);
					MessageBox(NULL, msg, _T("BED - Project Setup"), MB_OK);
					*step = 5;
					return true;
				}
#else
				return false;
#endif
			}
		}
		else if(*step == 9)
		{
			// all set, save changes
			//
			FormFilePath(m_name, m_srcdir, fullPath, MAX_PATH);
		}
		else if(*step == 10)
		{
			*step = 0;
			return true;
		}
	}
	return false;
}

// STATIC
//**************************************************************************
bool Bproject::OnProjSetupData(LPVOID cookie, int* step, wizMethod method, LPCTSTR var, LPTSTR val)
{
	LPCTSTR tc;
	LPTSTR  td;
	Bproject* pProject;

	TCHAR   typecode[32] = { 0 };

	if (var)
	{
		// get type string;

		for (tc = var, td = typecode; *tc && *tc != _T(' ') && ((td - typecode) < 30);)
			*td++ = *tc++;
		while (*tc && *tc == _T(' '))
			tc++;
		*td = _T('\0');

	}
	else
	{
		tc = NULL;
	}
	pProject = (Bproject*)cookie;
	
	if(pProject)
	{
		return pProject->OnWiz(step, method, typecode, tc, val);
	}
	return false;
}

//**************************************************************************
ERRCODE Bproject::ProjectWizard(Bpersist* pPerst)
{
	ERRCODE  ec = errOK;
	Bwizard* pwiz = NULL;
	TCHAR* 	 pdat;
	char	 cwd[MAX_PATH];
	TCHAR    defname[MAX_PATH];
	LPTSTR	 pname;
	
	m_new = true;

	// init default values 
	//
	if((getcwd(cwd, MAX_PATH)) != NULL)
	{
		CharToTChar(defname, cwd);

		_tcscpy(m_srcdir, defname);
		_tcscpy(m_blddir, defname);
		_tcscpy(m_inclPaths, defname);

		// get just the directory name
		if(defname[0])
		{
			pname = defname + _tcslen(defname);
			if(*--pname == _PTC_)
				*pname = _T('\0');
			while(pname >= defname)
				if(*--pname == _PTC_)
					break;
			if(*pname == _PTC_)
				pname++;
			_tcscpy(m_name, pname);
		}
	}
	_tcscpy(m_bldcmd, _T("make"));
	_tcscpy(m_bldshell, Bshell::GetDefaultShellName());
#ifdef Windows
	_tcscpy(m_bldswitch, _T("/c"));
	_tcscpy(m_bldshellsep, _T("&&"));
#else
	_tcscpy(m_bldswitch, _T("-c"));
	_tcscpy(m_bldshellsep, _T(";"));
#endif
	
	// create the wizard and start it off
	//
	pwiz = new Bwizard();
	pdat = new TCHAR [ sizeof(Bproject::projectScript) + 2 ];
	CharToTChar(pdat, Bproject::projectScript);

	pwiz->SetDefaultBitmapID(IDB_WIZMAP);
	pwiz->SetDefaultIconID(IDI_BED);
	ec = pwiz->Begin(_T("BED - Project Setup"), pdat, OnProjSetupData, (LPVOID)this, g_hInstance);

	if(ec == errOK)
	{
		TCHAR fullPath[MAX_PATH];

		if(! m_persist)	m_persist = new Bperstxml(m_name);
		FormFilePath(m_name, m_srcdir, fullPath, MAX_PATH);
		_tcscpy(m_path, fullPath);
		m_persist->SetFilePath(fullPath);
		Save();
		Open(m_name);
	}
	else
	{
		if(! m_usewiz)
		{
			ec = ProjectSetup(NULL);
		}
	}
	delete [] pdat;
	return ec;
}

