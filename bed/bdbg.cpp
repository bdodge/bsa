
#include "bedx.h"

// STATIC
//**************************************************************************
BOOL CALLBACK DBGdialog(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Bdbg*	pdbg = NULL;

	RECT	rc;
	RECT	rcme;
	HWND	hwndString;
	bool	usetarget;
	BdbgProgram program;
	TCHAR	tmp[MAX_PATH];

	switch (message) 
	{
	case WM_INITDIALOG:

		pdbg = (Bdbg*)lParam;
		if(! pdbg) return FALSE;
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
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)pdbg);

		pdbg->GetUseProjectTarget(usetarget);
		SendMessage(GetDlgItem(hWnd, IDC_USEPROJECTFORTARGET), BM_SETCHECK, usetarget, 0);
		if(usetarget)
		{
			EnableWindow(GetDlgItem(hWnd, IDC_DBGTARGET), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_DBGSRCDIR), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_DBGDIR), FALSE);
		}

		pdbg->GetDebugTarget(tmp, sizeof(tmp)/sizeof(TCHAR));
		SetDlgItemText(hWnd, IDC_DBGTARGET, tmp);

		pdbg->GetDebugDir(tmp, sizeof(tmp)/sizeof(TCHAR));
		SetDlgItemText(hWnd, IDC_DBGDIR, tmp);

		pdbg->GetDebugSourceDir(tmp, sizeof(tmp)/sizeof(TCHAR));
		SetDlgItemText(hWnd, IDC_DBGSRCDIR, tmp);

		pdbg->GetDebugProgram(program);
		switch(program)
		{
		case dbgGDB:
		default:
			SendMessage(GetDlgItem(hWnd, IDC_USEGDB), BM_SETCHECK, 1, 0);
			break;

		case dbgDEBUG:
			SendMessage(GetDlgItem(hWnd, IDC_USEDEBUG), BM_SETCHECK, 1, 0);
			break;
		}
		pdbg->GetShell(tmp, sizeof(tmp)/sizeof(TCHAR));
		SetDlgItemText(hWnd, IDC_DBGSHELL, tmp);
		
		pdbg->GetShellSwitches(tmp, sizeof(tmp)/sizeof(TCHAR));
		SetDlgItemText(hWnd, IDC_DBGSHELLSWITCH, tmp);
	
		pdbg->GetShellSep(tmp, sizeof(tmp)/sizeof(TCHAR));
		SetDlgItemText(hWnd, IDC_DBGSHELLSEP, tmp);

		SetFocus(GetDlgItem(hWnd, IDC_DBGSHELL));
		return FALSE;

	case WM_COMMAND:

		switch(wParam)
		{
		case IDOK:

			pdbg = (Bdbg*)GetWindowLong(hWnd, GWL_USERDATA);
			if(pdbg)
			{
				TCHAR emsg[MAX_PATH * 2];

				emsg[0] = _T('\0');

				GetDlgItemText(hWnd, IDC_DBGSHELL, tmp, sizeof(tmp)/sizeof(TCHAR));
				if(! tmp[0])
					_sntprintf(emsg, MAX_PATH*2, _T("Debugger Shell program must be specified"));
				else if(BUtil::FileExists(tmp) != errOK)
					_sntprintf(emsg, MAX_PATH*2, _T("Debugger Shell %s not found, use FULL path"), tmp);
				if(emsg[0])
				{
					int mc = MessageBox(NULL, emsg, _T("Debugger Settings Error"), MB_OK);
					break;
				}
				pdbg->SetShell(tmp);

				if(SendMessage(GetDlgItem(hWnd, IDC_USEGDB), BM_GETCHECK, 0, 0) != 0)
					pdbg->SetDebugProgram(dbgGDB);
				else
					pdbg->SetDebugProgram(dbgDEBUG);

				if(SendMessage(GetDlgItem(hWnd, IDC_USEPROJECTFORTARGET), BM_GETCHECK, 0, 0) != 0)
				{
					pdbg->SetUseProjectTarget(true);
				}
				else
				{
					pdbg->SetUseProjectTarget(false);

					GetDlgItemText(hWnd, IDC_DBGTARGET, tmp, sizeof(tmp)/sizeof(TCHAR));
					pdbg->SetDebugTarget(tmp);

					GetDlgItemText(hWnd, IDC_DBGSRCDIR, tmp, sizeof(tmp)/sizeof(TCHAR));
					if(! tmp[0])
						_tcscpy(tmp, _T("."));
					pdbg->SetDebugSourceDir(tmp);

					GetDlgItemText(hWnd, IDC_DBGDIR, tmp, sizeof(tmp)/sizeof(TCHAR));
					if(! tmp[0])
						_tcscpy(tmp, _T("."));
					pdbg->SetDebugDir(tmp);
				}
				GetDlgItemText(hWnd, IDC_DBGSHELLSWITCH, tmp, sizeof(tmp)/sizeof(TCHAR));
				pdbg->SetShellSwitches(tmp);

				GetDlgItemText(hWnd, IDC_DBGSHELLSEP, tmp, sizeof(tmp)/sizeof(TCHAR));
				pdbg->SetShellSep(tmp);
			}
			EndDialog(hWnd, wParam);
			break;

		case IDCANCEL:

			EndDialog(hWnd, wParam);
			break;

		case IDC_USEPROJECTFORTARGET:

			usetarget = SendMessage(GetDlgItem(hWnd, IDC_USEPROJECTFORTARGET), BM_GETCHECK, 0, 0) != 0;
			EnableWindow(GetDlgItem(hWnd, IDC_DBGTARGET), !usetarget);
			EnableWindow(GetDlgItem(hWnd, IDC_DBGSRCDIR), !usetarget);
			EnableWindow(GetDlgItem(hWnd, IDC_DBGDIR), !usetarget);
			pdbg = (Bdbg*)GetWindowLong(hWnd, GWL_USERDATA);
			if(usetarget)
			{
				if(pdbg) pdbg->SetTargetFromProject();
			}
			else
			{
				if(pdbg) pdbg->RestoreTarget();
			}
			pdbg->GetDebugTarget(tmp, sizeof(tmp)/sizeof(TCHAR));
			SetDlgItemText(hWnd, IDC_DBGTARGET, tmp);

			pdbg->GetDebugDir(tmp, sizeof(tmp)/sizeof(TCHAR));
			SetDlgItemText(hWnd, IDC_DBGDIR, tmp);

			pdbg->GetDebugSourceDir(tmp, sizeof(tmp)/sizeof(TCHAR));
			SetDlgItemText(hWnd, IDC_DBGSRCDIR, tmp);
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
Bdbg::Bdbg(Bed* pParent, BdbgInfo* pInfo)
	:
	m_editor(pParent),
	m_infopane(pInfo),
	m_persist(NULL),
	m_breaks(NULL),
	m_running(false),
	m_started(false)
{
	if(pParent)
		m_persist = pParent->GetPersist();
	RestoreSettings();
}

//**************************************************************************
Bdbg::~Bdbg()
{
	Bproject* pProj;

	Run(rcStop);
	if(m_infopane)
	{
		m_infopane->Stop();
	}
	if(m_editor)
	{
		pProj = m_editor->GetProject();
		if(pProj) SaveBreakpoints(pProj);
	}
}

//**************************************************************************
ERRCODE Bdbg::RestoreSettings()
{
#ifdef Windows
	_tcscpy(m_dbgswitch, _T("/C"));
	_tcscpy(m_dbgsep, _T("&"));
#else
	_tcscpy(m_dbgswitch, _T("-c"));
	_tcscpy(m_dbgsep, _T(";"));
#endif
	if(! (m_persist = m_editor->GetPersist()))
		return errFAILURE;

	m_persist->GetNvStr(_T("Debug/Shell"), m_dbgshell, MAX_PATH, Bshell::GetDefaultShellName());
	m_persist->GetNvStr(_T("Debug/ShellSwitch"), m_dbgswitch, 32, m_dbgswitch);
	m_persist->GetNvStr(_T("Debug/ShellSep"), m_dbgsep, 32, m_dbgsep);
	m_persist->GetNvInt(_T("Debug/Program"), (int&)m_dbgprog, dbgGDB);
	m_persist->GetNvBool(_T("Debug/UseProjectForTarget"), m_useproject_target, true);
	RestoreTarget();
	if(m_useproject_target)
		SetTargetFromProject();
	return errOK;
}

//**************************************************************************
ERRCODE Bdbg::RestoreTarget()
{
	m_persist->GetNvStr(_T("Debug/SourceDirectory"), m_dbgsrcdir, MAX_PATH, _T("."));
	m_persist->GetNvStr(_T("Debug/Directory"), m_dbgdir, MAX_PATH, _T("."));
	m_persist->GetNvStr(_T("Debug/Target"), m_dbgtarget, MAX_PATH, _T("."));
	return errOK;
}

//**************************************************************************
ERRCODE Bdbg::SaveSettings()
{
	if(! (m_persist = m_editor->GetPersist()))
		return errFAILURE;

	m_persist->SetNvStr(_T("Debug/Shell"), m_dbgshell);
	m_persist->SetNvStr(_T("Debug/ShellSwitch"), m_dbgswitch);
	m_persist->SetNvStr(_T("Debug/ShellSep"), m_dbgsep);
	m_persist->SetNvInt(_T("Debug/Program"), (int)m_dbgprog);
	m_persist->SetNvBool(_T("Debug/UseProjectForTarget"), m_useproject_target);
	if(! m_useproject_target)
	{
		m_persist->SetNvStr(_T("Debug/SourceDirectory"), m_dbgsrcdir);
		m_persist->SetNvStr(_T("Debug/Directory"), m_dbgdir);
		m_persist->SetNvStr(_T("Debug/Target"), m_dbgtarget);
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bdbg::EditSettings(HWND hWnd)
{
	int     rc;

	rc = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_DEBUG), hWnd, DBGdialog, (LPARAM)this);
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
ERRCODE Bdbg::GetShell(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_dbgshell, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bdbg::GetShellSwitches(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_dbgswitch, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bdbg::GetShellSep(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_dbgsep, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bdbg::GetDebugDir(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_dbgdir, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bdbg::GetDebugSourceDir(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_dbgsrcdir, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bdbg::GetDebugTarget(LPTSTR pStr, int nStr)
{
	if(! pStr || nStr <= 0)  return errBAD_PARAMETER;
	_tcsncpy(pStr, m_dbgtarget, nStr-1);
	pStr[nStr-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bdbg::GetUseProjectTarget(bool& usept)
{
	usept = m_useproject_target;
	return errOK;
}

//**************************************************************************
ERRCODE Bdbg::SetShell(LPCTSTR pStr)
{
	if(! pStr )  return errBAD_PARAMETER;
	_tcsncpy(m_dbgshell, pStr, MAX_PATH-1);
	m_dbgshell[MAX_PATH-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bdbg::SetShellSwitches(LPCTSTR pStr)
{
	if(! pStr)  return errBAD_PARAMETER;
	_tcsncpy(m_dbgswitch, pStr, 32-1);
	m_dbgswitch[32-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bdbg::SetShellSep(LPCTSTR pStr)
{
	if(! pStr)  return errBAD_PARAMETER;
	_tcsncpy(m_dbgsep, pStr, 32-1);
	m_dbgswitch[32-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bdbg::SetDebugDir(LPCTSTR pStr)
{
	if(! pStr)  return errBAD_PARAMETER;
	_tcsncpy(m_dbgdir, pStr, MAX_PATH-1);
	m_dbgdir[MAX_PATH-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bdbg::SetDebugSourceDir(LPCTSTR pStr)
{
	if(! pStr)  return errBAD_PARAMETER;
	_tcsncpy(m_dbgsrcdir, pStr, MAX_PATH-1);
	m_dbgdir[MAX_PATH-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bdbg::SetDebugTarget(LPCTSTR pStr)
{
	if(! pStr)  return errBAD_PARAMETER;
	_tcsncpy(m_dbgtarget, pStr, MAX_PATH-1);
	m_dbgtarget[MAX_PATH-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bdbg::SetUseProjectTarget(bool usept)
{
	m_useproject_target = usept;
	return errOK;
}

//**************************************************************************
ERRCODE Bdbg::SetTargetFromProject()
{
	Bed*		pb;
	Bproject*	pp;
	TCHAR		tmp[MAX_PATH];
	ERRCODE		ec;

	pb = m_editor;
	if(! pb) return errFAILURE;

	pp = pb->GetProject();
	if(! pp) return errFAILURE;

	ec = pp->GetSourceDirectory(tmp, MAX_PATH);
	if(ec != errOK) return ec;
	SetDebugSourceDir(tmp);

	ec = pp->GetBuildDirectory(tmp, MAX_PATH);
	if(ec != errOK) return ec;
	SetDebugDir(tmp);

	ec = pp->GetBuildTarget(tmp, MAX_PATH);
	if(ec != errOK) return ec;
	SetDebugTarget(tmp);

	return errOK;
}


//**************************************************************************
ERRCODE	Bdbg::Init()
{
	return errOK;
}

//**************************************************************************
ERRCODE	Bdbg::ConfirmBreak(Bbreak* pbp)
{
	if(m_editor && m_editor->GetViews())
	{
		Bview* pView;
		
		for(pView = m_editor->GetViews(); pView; pView = pView->GetNext(pView))
		{
			Bbuffer* pBuf;
		
			pBuf = pView->GetBuffer();
			if(pBuf && ! _tcscmp(pBuf->GetName(), pbp->m_name))
			{
				BlineInfo info;
				
				info = pBuf->GetLineIsInfo(pbp->m_line);
				if(info & liIsBreakpoint)
				{
					return errOK;
				}
				return errFAILURE;
			}
		}
	}
	return errOK;
}

//**************************************************************************
ERRCODE	Bdbg::AddBreakpoint(LPCTSTR pName, LPCTSTR pCond, BbreakType type, int line, bool en)
{
	Bbreak* pbp;
	
	for(pbp = m_breaks; pbp; pbp = pbp->m_next)
	{
		if(pbp->m_line == line && pbp->m_type == type)
		{
			if(pName && pbp->m_name)
			{
				if(! _tcscmp(pName, pbp->m_name))
				{
					pbp->m_enabled = en;
					break;
				}
			}
		}
	}
	if(! pbp)
	{
		pbp = new Bbreak;

		pbp->m_name = new TCHAR [ _tcslen(pName) + 2 ];
		_tcscpy(pbp->m_name, pName);

		pbp->m_cond = new TCHAR [ _tcslen(pCond) + 2 ];
		_tcscpy(pbp->m_name, pCond);

		pbp->m_valid   = true;
		pbp->m_enabled = en;
		pbp->m_type    = type;

		if(! m_breaks)
		{
			m_breaks = pbp;
		}
		else
		{
			pbp->m_next = m_breaks;
			m_breaks = pbp;
		}
	}
	if(m_editor && m_editor->GetViews())
	{
		Bview* pView;
		
		for(pView = m_editor->GetViews(); pView; pView = pView->GetNext(pView))
		{
			Bbuffer* pBuf;
		
			pBuf = pView->GetBuffer();
			if(pBuf && ! _tcscmp(pBuf->GetName(), pbp->m_name))
			{
				BlineInfo info;
				
				info = pBuf->GetLineIsInfo(pbp->m_line);
				if(info & liIsBreakpoint)
				{
					return errOK;
				}
				pBuf->SetLineIsInfo(pbp->m_line, info | liIsBreakpoint);
				return errOK;
			}
		}
	}
	return errOK;
}

//**************************************************************************
ERRCODE	Bdbg::RestoreBreakpoints(Bproject* pProj, bool recheck)
{
	ERRCODE ec;
	int     n, nbad;
	Bbreak  bp;

	// create list of breakpoints from project
	//
	for(n = nbad = 0; pProj && (n < BED_MAX_BREAKPOINTS); n++)
	{
		memset(&bp, 0, sizeof(bp));

		ec = pProj->RestoreBreakpoint(n, &bp);

		if(ec == errOK && bp.m_enabled && bp.m_valid)
		{
			if(bp.m_type == bpProgram)
				ec = SetBreak(bp.m_name, bp.m_line, bp.m_ref);
			else
				ec = SetWatch(bp.m_name, bp.m_ref);
			if(ec != errOK)
			{
				nbad++;
				bp.m_valid = false;
				pProj->SaveBreakpoint(n, &bp);
			}
			else
			{
				if(! recheck || (ConfirmBreak(&bp) == errOK))
				{
					AddBreakpoint(bp.m_name, bp.m_cond, bp.m_type, bp.m_line, bp.m_enabled);
				}
			}
		}
		else if(! bp.m_valid)
		{
			break;
		}
	}
	if(nbad)
	{
		TCHAR emsg[128];

		_sntprintf(emsg, 128, _T("Could not set %d Breakpoint%c"), nbad, ((nbad > 1) ? _T('s') : _T('\0')));
		m_editor->SetStatus(emsg);
	}
	// add breaks that are in viewable buffers
	//
	if(m_editor && m_editor->GetViews())
	{
		Bview* pView;
		
		for(pView = m_editor->GetViews(); pView; pView = pView->GetNext(pView))
		{
			Bbuffer* pBuf;

			pBuf = pView->GetBuffer();
			if(pBuf)
			{
				int			lines, lino, maxcol;
				int			minl, maxl;
				BlineInfo	info;
				
				lines = pBuf->GetLineCount(maxcol);
				minl  = lines;
				maxl  = 1;

				for(lino = 1; lino < lines; lino++)
				{
					info = pBuf->GetLineIsInfo(lino);
					
					if(info & liIsBreakpoint)
					{
						if(lino < minl) minl = lino;
						if(lino > maxl) maxl = lino;

						AddBreakpoint(pBuf->GetName(), _T(""), bpProgram, lino, true);
					}
				}
				if(minl >= maxl)
				{
					pView->UpdateAllViews(minl, 1, maxl, 1, pBuf);
				}
			}
		}
	}
	return errOK;
}

//**************************************************************************
ERRCODE	Bdbg::SaveBreakpoints(Bproject* pProj)
{
	ERRCODE ec = errOK;
	int     n;
	Bbreak	bp, *pbp;

	if(! pProj) return errOK;

	for(pbp = m_breaks, n = 0; pbp; n++, pbp = pbp->m_next)
	{
		pbp->m_index = n;
		ec = pProj->SaveBreakpoint(pbp->m_index, pbp);
	}
	memset(&bp, 0, sizeof(bp));
	bp.m_name = NULL;
	bp.m_cond = NULL;
	pProj->SaveBreakpoint(n, &bp);
	return ec;
}

//**************************************************************************
ERRCODE	Bdbg::SetBreak(LPCTSTR filename, DWORD line, int& refNum)
{
	ERRCODE ec = errOK;
	Bbreak	bp, *pbp;
	int		n;
	
	for(pbp = m_breaks, n = 0; pbp; n++, pbp = pbp->m_next)
	{
		if(pbp->m_line == (int)line && ! _tcscmp(pbp->m_name, filename))
			return errOK;
	}
	pbp = new Bbreak;
	pbp->m_name = new TCHAR [ _tcslen(filename) + 2 ];
	_tcscpy(pbp->m_name, filename);
	pbp->m_line		= line;
	pbp->m_index	= n;
	pbp->m_valid	= true;
	pbp->m_enabled	= true;
	if(! m_breaks)
	{
		m_breaks = pbp;
	}
	else
	{
		pbp->m_next = m_breaks;
		m_breaks = pbp;
	}
	return ec;
}

//**************************************************************************
ERRCODE	Bdbg::SetBreak(LPCTSTR filename, LPCTSTR funcname, int& refNum)
{
	return errOK;
}

//**************************************************************************
ERRCODE	Bdbg::ClearBreak(int refNum)
{
	return errOK;
}

//**************************************************************************
ERRCODE	Bdbg::ClearBreak(LPCTSTR filename, DWORD line)
{
	ERRCODE ec = errOK;
	Bbreak	*vbp, *pbp;

	for(vbp = NULL, pbp = m_breaks; pbp; pbp = pbp->m_next)
	{
		if(pbp->m_line == (int)line)
		{
			if(pbp->m_name && filename)
			{
				if(! _tcscmp(pbp->m_name, filename))
				{
					if(vbp)
					{
						vbp->m_next = pbp->m_next;
					}
					else
					{
						m_breaks = NULL;
					}
					delete pbp;
					return errOK;
				}
			}
		}
		vbp = pbp;
	}
	return errFAILURE;
}

//**************************************************************************
ERRCODE	Bdbg::ClearAllBreaks(void)
{
	return errOK;
}

//**************************************************************************
ERRCODE	Bdbg::SetWatch(LPCTSTR varname, int& refNum)
{
	return errOK;
}

//**************************************************************************
ERRCODE	Bdbg::ClearWatch(int refNum)
{
	return errOK;
}

//**************************************************************************
ERRCODE	Bdbg::ClearWatch(LPCTSTR varname)
{
	return errOK;
}

//**************************************************************************
ERRCODE	Bdbg::ClearAllWatchs(void)
{
	return errOK;
}

//**************************************************************************
ERRCODE	Bdbg::Run(BdbgRunCode to)
{
	switch(to)
	{
	case rcStop:
		m_running = false;
		break;
	case rcGo:
	case rcStepIn:
	case rcStepOut:
	case rcStep:
		m_started = true;
		m_running = true;
		break;
	default:
		break;
	}
	return errOK;
}

// STATIC
//**************************************************************************
char Bdbg::dbgWizScript[] =

"Begin;"
"  Panel[panel(1), height(300)]:BED Debugger Setup Wizard;"
"    Description:Use this wizard to integrate a Debug Program;"
"    LeftImage;"
"    Next[default];Cancel;"
"  EndPanel;"

"  Panel[panel(2)]:Debug Setup;"
"    Description:Choose how to specify debugger settings;"
"    ChoiceGroup:Select how to enter settings;"
"       Choice[default,panel(3),                       ]:I want to continue using the wizard;"
"       Choice[panel(10),var(bool Debug/Wizard/Expert) ]:I want to use the debug dialog;"
"    EndChoiceGroup;"
"    Back;Next[default];Cancel;"
"  EndPanel;"

"  Panel[panel(3)]:Debugger Program;"
"    Description:Pick Debugger;"
"    IconImage;"
"    ChoiceGroup:Select which program you use for debugging;"
"       Choice[default,panel(4),var(bool progIsGDB)]:gdb;"
"       Choice[panel(4)                            ]:debug;"
"    EndChoiceGroup;"
"    Back;Next[default];Cancel;"
"  EndPanel;"

"  Panel[panel(4)]:Debug Target Specification;"
"    ChoiceGroup:Select how the debug target is specified;"
"       Choice[default,panel(6),var(bool Debug/UseProjectForTarget)]:Use Current Project;"
"       Choice[panel(5)                                            ]:Specify Target;"
"    EndChoiceGroup;"
"    Back;Next[default];Cancel;"
"  EndPanel;"

"  Panel[panel(5)]:Debug Target;"
"    Description:Setup Program to Debug;"
"    IconImage;"
"    Label:Debug Directory;"
"    Edit[sameline(120),width(200),var(string Debug/Directory)];"
"    Label:Debug Source Directory;"
"    Edit[sameline(120),width(200),var(string Debug/SourceDirectory)];"
"    Label:Debug Target;"
"    Edit[sameline(120),width(200),var(string Debug/Target)];"
"    Label:;"
"    Back;Next[default];Cancel;"
"  EndPanel;"

"  Panel[panel(6)]:Debug Shell;"
"    Description:Enter the shell program used for debugging;"
"    IconImage;"
"    Label:Debug Shell;"
"    Edit[sameline(100),width(200),var(string Debug/Shell)];"
"    Label:;"
"    Label:Use full path to shell program, e.g. /bin/bash for;"
"    Label:Linux or \\Windows\\System32\\cmd.exe for Windows;"
"    Back;Next[default];Cancel;"
"  EndPanel;"

"  Panel[panel(7)]:Debug Shell Switche(s);"
"    Description:Enter the shell switches for shell;"
"    IconImage;"
"    Label:Build Switche(s);"
"    Edit[sameline(100),width(200),var(string Debug/ShellSwitch)];"
"    Label:;"
"    Label:Enter any command line switches needed to have the build;"
"    Label:shell read from the command line arguments, e.g, use -c;"
"    Label:for bash - Linux, or /c for cmd.exe - Windows.;"
"    Back;Next[default];Cancel;"
"  EndPanel;"

"  Panel[panel(8)]:Debug Shell Command Separator;"
"    Description:Enter the command line command separator string;"
"    IconImage;"
"    Label:Command Separator;"
"    Edit[sameline(140),width(200),var(string Build/ShellSep)];"
"    Label:;"
"    Label:Enter any special charater(s) used to separate;"
"    Label:individual commands to the shell program, e.g.;"
"    Label:a semicolon for bash - Linux, or \"&&&&\" for cmd.exe - Windows.;"
"    Back;Next[default];Cancel;"
"  EndPanel;"


"  Panel[panel(9)]:Done;"
"    IconImage;"
"    Label:Click Finish to begin debugging;"
"    Label:;"
"    Back;Finish[default];"
"  EndPanel;"

"  Panel[panel(10)]:Exit;"
"  EndPanel;"

"End;";

//**************************************************************************
bool Bdbg::OnWiz(int* step, wizMethod method, LPCTSTR typecode, LPCTSTR var, LPTSTR val)
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
			if(! _tcscmp(var, _T("progIsGDB")))
			{
				m_persist->GetNvInt(_T("Debug/Program"), iv, dbgGDB);
				bv = iv == dbgGDB;
			}
			else
				m_persist->GetNvBool(var, bv, false);
			_sntprintf(val, 32, _T("%d"), bv ? 1 : 0);
			break;
		case _T('s'):
			m_persist->GetNvStr(var, val, MAX_PATH);
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
			bv = _tcstol(val, NULL, 0) != 0;
			if(! _tcscmp(var, _T("progIsGDB")))
				m_persist->SetNvBool(_T("Debug/Program"), bv ? dbgGDB : dbgDEBUG);
			else
				m_persist->SetNvBool(var, bv);
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
		if(*step == 7)
		{
			TCHAR shell[MAX_PATH];

			m_persist->GetNvStr(_T("Debug/Shell"), shell, MAX_PATH, _T(""));
			
			if(shell[0] == _T('\0'))
			{
				MessageBox(NULL, _T("Shell Program must be specified"), _T("BED - Debug Setup"), MB_OK);
				*step = 7;
				return true;
			}
			else
			{
#if 0 // assume the user knows enough to use the default and that forking the shell can find the program
				TCHAR msg[MAX_PATH*2];

				// if file doesn't exists, complain
				//
				if(errOK != BUtil::FileExists(shell))
				{
					_sntprintf(msg, MAX_PATH*2, _T("Program "_Pfs_" not found.  Use full path to program"), shell);
					MessageBox(NULL, msg, _T("BED - Debug Setup"), MB_OK);
					*step = 7;
					return true;
				}
#else
				return true;
#endif
			}
		}
		break;
	}
	return false;
}

// STATIC
//**************************************************************************
bool Bdbg::OnDebugSetupData(LPVOID cookie, int* step, wizMethod method, LPCTSTR var, LPTSTR val)
{
	LPCTSTR tc;
	LPTSTR  td;
	Bdbg*  pdbg;

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
	pdbg = (Bdbg*)cookie;
	
	if(pdbg)
	{
		return pdbg->OnWiz(step, method, typecode, tc, val);
	}
	return false;
}

//**************************************************************************
ERRCODE Bdbg::DebugWizard()
{
	ERRCODE  ec = errOK;
	Bwizard* pwiz = NULL;
	LPTSTR   pdat;

	// create the wizard and start it off
	//
	pwiz = new Bwizard();
	pdat = new TCHAR [ sizeof(Bdbg::dbgWizScript) + 2 ];
	CharToTChar(pdat, Bdbg::dbgWizScript);

	pwiz->SetDefaultBitmapID(IDB_WIZMAP);
	pwiz->SetDefaultIconID(IDI_BED);
	ec = pwiz->Begin(_T("BED - Debug Setup"), pdat, OnDebugSetupData, (LPVOID)this, g_hInstance);

	if(ec == errOK)
	{
		m_persist->SetNvBool(_T("Debug/IsSetup"), true);
		RestoreSettings();
	}
	else
	{
		SaveSettings();
	}
	delete [] pdat;
	return ec;
}


//**************************************************************************
//**************************************************************************
//**************************************************************************

//**************************************************************************
BdbgGDB::BdbgGDB(Bed* pParent, BdbgInfo* pInfo)
	:
	Bdbg(pParent, pInfo)
{
}

//**************************************************************************
BdbgGDB::~BdbgGDB()
{
}

//**************************************************************************
ERRCODE	BdbgGDB::Init()
{
	if(! m_infopane) 
	{
		return errFAILURE;
	}
	return m_infopane->Startup();
}

//**************************************************************************
ERRCODE	BdbgGDB::SetBreak(LPCTSTR filename, DWORD line, int& refNum)
{
	CHAR cb[MAX_PATH * 2];

	strcpy(cb, "break ");
#ifdef UNICODE
	BUtil::Utf8Encode(cb+6, BfileInfo::FilePart((LPTSTR)filename), false);
#else
	strcpy(cb + 6, BfileInfo::FilePart((LPTSTR)filename));
#endif
	strcat(cb, ":");
	sprintf(cb + strlen(cb), "%u\n", line);
	if(RunCommand(cb) == errOK)
		return Bdbg::SetBreak(filename, line, refNum);
	return errFAILURE;
}

//**************************************************************************
ERRCODE	BdbgGDB::SetBreak(LPCTSTR filename, LPCTSTR funcname, int& refNum)
{
	return Bdbg::SetBreak(filename, funcname, refNum);
}

//**************************************************************************
ERRCODE	BdbgGDB::ClearBreak(int refNum)
{
	return Bdbg::ClearBreak(refNum);
}

//**************************************************************************
ERRCODE	BdbgGDB::ClearBreak(LPCTSTR filename, DWORD line)
{
	CHAR cb[MAX_PATH * 2];

	strcpy(cb, "clear ");
#ifdef UNICODE
	BUtil::Utf8Encode(cb+6,  BfileInfo::FilePart((LPTSTR)filename), false);
#else
	strcpy(cb+6, BfileInfo::FilePart((LPTSTR)filename));
#endif
	strcat(cb, ":");
	sprintf(cb + strlen(cb), "%u\n", line);
	if(RunCommand(cb) == errOK)
		return Bdbg::ClearBreak(filename, line);
	return errFAILURE;
}

//**************************************************************************
ERRCODE	BdbgGDB::ClearAllBreaks(void)
{
	RunCommand("clear all\n");
	return Bdbg::ClearAllBreaks();
}

//**************************************************************************
ERRCODE	BdbgGDB::SetWatch(LPCTSTR varname, int& refNum)
{
	return Bdbg::SetWatch(varname, refNum);
}

//**************************************************************************
ERRCODE	BdbgGDB::ClearWatch(int refNum)
{
	return Bdbg::ClearWatch(refNum);
}

//**************************************************************************
ERRCODE	BdbgGDB::ClearWatch(LPCTSTR varname)
{
	return Bdbg::ClearWatch(varname);
}

//**************************************************************************
ERRCODE	BdbgGDB::ClearAllWatchs(void)
{
	return Bdbg::ClearAllWatchs();
}

//**************************************************************************
ERRCODE	BdbgGDB::Run(BdbgRunCode to)
{
	LPCSTR cmd = NULL;
	
	switch(to)
	{
	case rcStop:
		cmd = "\03";
		break;
	case rcGo:
		if(m_started)
			cmd = "continue\n";
		else
			cmd = "run\n";
		break;
	case rcStep:
		cmd = "next\n";
		break;
	case rcStepIn:
		cmd = "step\n";
		break;
	case rcStepOut:
		cmd = "finish\n";
		break;
	default:
		cmd = NULL;
		break;
	}	
	RunCommand(cmd);
	return Bdbg::Run(to);
}

//**************************************************************************
ERRCODE	BdbgGDB::RunCommand(LPCSTR cmd)
{
	if(cmd && m_infopane)
	{
		DWORD ncmd = strlen(cmd);

		return m_infopane->SendShell((PBYTE)cmd, ncmd);
	}
	return errFAILURE;
}

//**************************************************************************
ERRCODE	BdbgGDB::OnDebuggerData(LPCTSTR pLine)
{
	LPCTSTR px;
	TCHAR   fName[MAX_PATH];
	ERRCODE ec;
	int     line;

	Bview*	pView;
	if(! pLine || pLine[0] == _T('\0')) return errOK;
	px = pLine;

	if(px[0] == 0x1A)
	{
		px += 2;
		
		// breakpoint
		ec = BshellInfo::GetErrorLine(px, fName, MAX_PATH, &line);
		if(ec == errOK && m_editor && m_editor->GetCurrentView())
		{
			pView = m_editor->GetCurrentView();
			
			ec = pView->PushParm(btAny, ptBufferType);
			ec = pView->PushParm(ltLF, ptTextLineTerm);
			ec = pView->PushParm(-1, ptTextEncoding);
			ec = pView->PushParm(fName, _tcslen(fName), ptOpenExistingFilename);
			ec = pView->Dispatch(OpenBuffer);
			if(ec == errOK)
			{
				pView = m_editor->GetCurrentView();
				if(pView && pView->GetBuffer())
				{
					if(_tcsstr(pView->GetBuffer()->GetName(), fName))
					{
						ec = pView->PushParm(liIsExecCurrent, ptNumber);
						ec = pView->PushParm(line, ptNumber);
						ec = pView->Dispatch(SetLineType);
					}
				}
			}
		}
		return errOK;
	}
	else if(px[0] == _T('$'))
	{
		// watchpoint
		return errOK;
	}
	else
	{
	}
	return errFAILURE;
}

