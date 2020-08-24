#include "bedx.h"

//**************************************************************************
BbuildInfo::BbuildInfo(LPCTSTR name, BappPanel* pPanel, Bed* pParent)
	:
	BshellInfo(name, pPanel, pParent),
	m_shownResults(false)
{
}

//**************************************************************************
BbuildInfo::~BbuildInfo()
{
}

//**************************************************************************
LPCTSTR BbuildInfo::GetPathPrefix()
{
	if(! m_srcDir[0])
		return BshellInfo::GetPathPrefix();
	return m_srcDir;
}

//**************************************************************************
void BbuildInfo::Select(LPTVITEM pItem)
{
	ERRCODE ec;
	TCHAR	fName[MAX_PATH];
	int     line;
	bool	exists;

	if(m_ignoreNotify)					return;
	if(! m_view || ! m_editor)			return;
	if(! (pItem->mask & TVIF_TEXT))		return;

	ec = BbuildInfo::GetErrorLine(pItem->pszText, fName, MAX_PATH, &line);

	if(ec == errOK)
	{
		Bproject*	pProj;
		TCHAR		path[MAX_PATH];
		int			plen;

		_tcscpy(path, fName);
		if(path[0] == _PTC_ || path[1] == ':')
		{
			// file specified in error has an absolute path, use it plain
			//
			exists = BUtil::FileExists(path) == errOK;
		}
		else
		{
			// file is relative path, so try it from current directory first
			//
			exists = BUtil::FileExists(path) == errOK;
		
			if(! exists)
			{
				// file not in current directory, see if it is relative 
				// to project build directory (or root dir if no build dir)
				//
				if((pProj = m_editor->GetProject()) != NULL)
				{
					pProj->GetBuildDirectory(path, MAX_PATH);
				}
				else
				{
					path[0] = _T('\0');
				}
				plen = _tcslen(path);
				if(plen > 0 && plen < MAX_PATH-1 && path[plen - 1] != _PTC_)
				{
					path[plen++] = _PTC_;
					path[plen] = _T('\0');
				}
				_tcscat(path, fName);

				exists = BUtil::FileExists(path) == errOK;

				if(! exists)
				{
					// still no file, try it from src dir
					//
					pProj->GetSourceDirectory(path, MAX_PATH);
					plen = _tcslen(path);
					if(plen > 0 && plen < MAX_PATH-1 && path[plen - 1] != _PTC_)
					{
						path[plen++] = _PTC_;
						path[plen] = _T('\0');
					}
					_tcscat(path, fName);

					exists = BUtil::FileExists(path) == errOK;
				}
			}
		}
		if(exists)
		{
			m_editor->EditBuffer(path, true, btAny);
			if(m_editor->GetCurrentView())
			{
				m_editor->GetCurrentView()->PushParm(line, ptNumber);
				if(m_editor->GetCurrentView()->Dispatch(MoveToLine) == errOK)
				{
					BlineInfo type = GetLineIsType();
					
					m_editor->GetCurrentView()->PushParm(type, ptNumber);
					m_editor->GetCurrentView()->PushParm(line, ptNumber);
					m_editor->GetCurrentView()->Dispatch(SetLineType);
				}
			}
		}
		else
		{
			TCHAR emsg[MAX_PATH + 64];

			_sntprintf(emsg, MAX_PATH + 64, _T("Can't find error item: %s"), fName);
			m_editor->SetStatus(emsg);
		}
	}
}

//**************************************************************************
void BbuildInfo::Event(LPARAM lParam)
{		
	BshellInfo::Event(lParam);

	if(m_pid == 0 && ! m_shownResults)
	{
		BbuildResultsInfo* pResults;
		BappPanel*		   pInfoPanel;
		TCHAR     		   textBuf[128];

#ifdef TIMER_BASED_SHELLPOLL
		if(m_idTimer)
			KillTimer(NULL, m_idTimer);
#elif defined SHELL_BASED_SHELLPOLL
		m_reader.Stop();
#endif
		SetWindowLong(m_hwndTree, GWL_USERDATA, (LONG)NULL);
		
		pInfoPanel = m_editor->GetPanel(_T("Info"));
		if(m_editor && pInfoPanel)
		{
			pResults = (BbuildResultsInfo*)m_editor->GetPane(_T("Info"), _T("Results"));

			if(! pResults)
			{
				pResults = new BbuildResultsInfo(_T("Results"), m_editor->GetStatusPanel(), m_editor);
				pInfoPanel->AddPane(pResults);
				pResults->SetView(m_view);
			}
			if(pResults)
			{
				TCHAR     errBuf[128];
				TCHAR	  fName[MAX_PATH];
				TVITEM    tvitem;
				HTREEITEM hItem;
				int       line;
				int		  nAdded = 0;
				
				// activate to make window at least
				pInfoPanel->ActivatePane(pResults);
				pResults->Clear();

				hItem = TreeView_GetNextItem(m_hwndTree, NULL, TVGN_ROOT);
				
				while(hItem)
				{
					tvitem.hItem 		= hItem;
					tvitem.mask			= TVIF_TEXT;
					tvitem.cchTextMax	= 128;
					tvitem.pszText		= textBuf;
					
					if(TreeView_GetItem(m_hwndTree, &tvitem))
					{
						_tcscpy(errBuf, textBuf);
						if(GetErrorLine(errBuf, fName, MAX_PATH, &line) == errOK)
						{
							if(_tcsstr(textBuf, _T("Warning")))
								m_warnings++;
							else if(_tcsstr(textBuf, _T("warning")))
								m_warnings++;
							else if(_tcsstr(fName, _T(".")))
								m_errors++;
							nAdded++;
							pResults->AppendItem(textBuf);
						}
					}
					hItem = TreeView_GetNextItem(m_hwndTree, hItem, TVGN_NEXT);
				}
				_sntprintf(textBuf, 128, _T("Build Complete - %d Errors  %d Warnings"),
							m_errors, m_warnings);
				AppendItem(textBuf);
		
				if(nAdded)
				{
					pResults->AppendItem(textBuf);
				}
				else
				{
					// go back to build results, since nothing real to show
					pInfoPanel->ActivatePane(this);
				}
			}
		}
		m_shownResults = true;
	}
}

//**************************************************************************
ERRCODE BbuildInfo::Startup(void)
{
	Bproject* pProject;
	
	if(! Exited())
	{
		int rc = MessageBox(NULL, _T("Cancel Current Build?"),
			m_view->GetBuffer()->GetEditor()->GetTitle(m_view->GetBuffer()), MB_YESNO);
		if(rc == IDNO) return errFAILURE;

		Stop();
	}
	Clear();
	m_errors = m_warnings = 0;
	m_shownResults = false;

	// get project from editor
	//
	if(! (pProject = m_editor->GetProject()))
	{
		Bproject::OpenDefaultProject(NULL, m_editor, false, true);
	}
	if(! (pProject = m_editor->GetProject()))
	{
		return errFAILURE;
	}
	TCHAR szShellName		[128];
	TCHAR szSwitches		[128];
	TCHAR szShellCommandSep	[128];
	TCHAR szBuildDir		[256];
	TCHAR szBuildProgram	[128];
	TCHAR szBuildTarget		[256];

	pProject->GetBuildCommand	(szBuildProgram,	128);
	pProject->GetBuildTarget	(szBuildTarget,		256);
	pProject->GetBuildDirectory (szBuildDir,		256);
	pProject->GetBuildShell		(szShellName,		128);
	pProject->GetShellSwitches	(szSwitches,		128);
	pProject->GetShellCommandSep(szShellCommandSep,	128);
	m_srcDir[0] = _T('\0');
	pProject->GetSourceDirectory(m_srcDir,			MAX_PATH);
	
	m_hwndParent = GetParent(m_hwndTree);
	m_toklen = 0;
	
	#ifdef UNICODE
	_sntprintf(m_szShellCommand, 1024, L"%ls %ls \"cd %ls %ls %ls %ls\"\n",
			szShellName, szSwitches, szBuildDir,
			szShellCommandSep,
			szBuildProgram, szBuildTarget);
	#else
	_snprintf(m_szShellCommand, 1024, "%s %s \"cd %s %s %s %s\"\n",
			szShellName, szSwitches, szBuildDir,
			szShellCommandSep,
			szBuildProgram, szBuildTarget);
	#endif
	_tcscpy(m_szShellDir, szBuildDir);
	return BshellInfo::Startup();
}

