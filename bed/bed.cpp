
#include "bedx.h"

//**************************************************************************
Bed::Bed(HINSTANCE hInstance, LPCTSTR lpAppName)
	:
	BappFrame(hInstance),

	m_buffers(NULL),
	m_views(NULL),
	m_oldviews(NULL),
	m_layout(voHorizontal),
	m_pInfoPanel(NULL),
	m_pEditPanel(NULL),
	m_pStatusPanel(NULL),
	m_pDebugPanel(NULL),
	m_curview(NULL),
	m_sccs(NULL),
	m_subpanels(NULL),
	m_dbg(NULL),
	m_project(NULL),
	m_pdbenable(pdbNever),
	m_hwndStatusText(NULL),
	m_hwndStatusInfo(NULL),
	m_hwndStatusLino(NULL),
	m_recentFileIndex(1),
	m_recentProjectIndex(1),
	m_maxRecentFiles(12),
	m_maxRecentProjects(6),
	m_recentFiles(NULL),
	m_recentProjects(NULL),
	m_restoreProjects(false),
	m_suppressTabPanes(false),
	m_firstdebug(true)
{
	m_persist = new Bpersist(lpAppName, _T("BSA Software"));

	m_persist->GetNvBool(_T("View/Windows/ShowStatusBar"),	m_showStatbar,		true);
	m_persist->GetNvBool(_T("View/Windows/ShowPicker"),		m_showPicker,		true);
	m_persist->GetNvBool(_T("View/Windows/ShowCalculator"),	m_showCalculator,	true);
	m_persist->GetNvBool(_T("View/Windows/ShowFunctions"),	m_showFunctions,	true);
	m_persist->GetNvBool(_T("View/Windows/ShowFiles"),		m_showFiles,		true);
	m_persist->GetNvBool(_T("View/Windows/ShowInfo"),		m_showInfo,			true);
	m_persist->GetNvBool(_T("View/Windows/ShowBuildInfo"),	m_showInfoBuild,	true);
	m_persist->GetNvBool(_T("View/Windows/ShowResultsInfo"),m_showInfoBuildResults,	true);
	m_persist->GetNvBool(_T("View/Windows/ShowDebugInfo"),	m_showInfoDebug,	false);
	m_persist->GetNvBool(_T("View/Windows/ShowGrepResults"),m_showInfoGrep,		true);
	m_persist->GetNvBool(_T("View/Windows/ShowGrepResults2"),m_showInfoGrep2,	true);
	m_persist->GetNvBool(_T("View/Windows/ShowSCCS"),		m_showInfoSCCS,		true);
	m_persist->GetNvBool(_T("View/Windows/ShowWatch"),		m_showWatch,		true);
	m_persist->GetNvBool(_T("View/Windows/ShowStack"),		m_showStack,		true);
	m_persist->GetNvBool(_T("View/Windows/ShowVariables"),	m_showVariables,	true);
	m_persist->GetNvBool(_T("View/Windows/ShowMemory"),		m_showMemory,		true);
	m_persist->GetNvBool(_T("View/Windows/ShowRegisters"),	m_showRegisters,	true);
	m_persist->GetNvBool(_T("View/Windows/ShowThreads"),	m_showThreads,		true);

	m_persist->GetNvInt(_T("Text/Encoding/DefaultEncoding"),(int&)m_defaultencoding,  txtANSI);
	m_persist->GetNvStr(_T("FTP/Username"), m_ftpUser, 128, _T(""));
	m_ftpPassword[0] = _T('\0');
	m_persist->GetNvBool(_T("FTP/AnonymousLogin"), m_ftpAnonymous, false);
	m_persist->GetNvBool(_T("FTP/RememberPassword"), m_perstPass, false);

	ERRCODE ec;
	TCHAR	fileName[MAX_PATH];
	bool	bv;

	m_persist->GetNvBool(_T("SCCS/Integration/IsSetup"), bv, false);
	if(bv)
	{
		m_sccs = new Bsccs(this);
	}
	for(
			m_recentFileIndex = 1, m_recentFiles = NULL;
			m_recentFileIndex < m_maxRecentFiles;
			m_recentFileIndex++
	)
	{
		ec = GetRecentFile(m_recentFileIndex, fileName, MAX_PATH);
		if(ec != errOK) break;

		m_recentFiles = BrecentList::AddToList(new BrecentList(fileName, NULL), m_recentFiles);
	}
	for(
			m_recentProjectIndex = 1, m_recentProjects = NULL;
			m_recentProjectIndex < m_maxRecentProjects;
			m_recentProjectIndex++
	)
	{
		ec = GetRecentProject(m_recentProjectIndex, fileName, MAX_PATH);
		if(ec != errOK) break;

		m_recentProjects = BrecentList::AddToList(new BrecentList(fileName, NULL), m_recentProjects);
	}
	m_persist->GetNvInt(_T("PDB/Enable"), (int&)m_pdbenable, m_pdbenable);
}

//**************************************************************************
Bed::~Bed()
{
	Bview* pView;
	Bview* pDel;

	if(m_project)
	{
		m_project->SaveViews();
	}
	// close open views
	for(pView = m_views; pView;)
	{
		pDel = pView;
		pView = pView->GetNext(pView);
		pDel->Close();
	}
	m_views = NULL;

	// delete orphaned views
	for(pView = m_oldviews; pView;)
	{
		pView->PersistSettings();
		pDel = pView;
		pView = pView->GetNext(pView);
		delete pDel;
	}
	m_oldviews = NULL;

	// delete recents lists
	if(m_recentFiles)
		BrecentList::FreeList(m_recentFiles);
	if(m_recentProjects)
		BrecentList::FreeList(m_recentProjects);

	// delete all buffers
	if(m_buffers)
		BbufList::FreeList(m_buffers);

	if(m_dbg)
		delete m_dbg;
	m_dbg = NULL;

	if(m_project)
	{
		delete m_project;
	}
	m_project = NULL;

	if(m_sccs)
		delete m_sccs;
	m_sccs = NULL;

	// delete list of panels
	//
	m_subpanels = BpanelList::FreeList(m_subpanels);

	// close down persistance
	if(m_persist)
	{
		m_persist->Flush();
		delete m_persist;
	}
}

//**************************************************************************
void Bed::SetFocus(bool inout)
{
	if(inout && m_curview)
	{
		//_tprintf(_T("app set focus => %p\n"), m_curview);
		m_curview->SetFocus(inout);
	}
}

//**************************************************************************
BOOL Bed::Close()
{
	Bview*    pView;

	for(pView = GetViews(); pView; pView = pView->GetNext(pView))
	{
		if(! pView->CheckModified(true))
			return FALSE;
	}
	return TRUE;
}

//**************************************************************************
ERRCODE Bed::GetRecentFile(int index, LPTSTR name, int nName)
{
	TCHAR menuName[MAX_PATH + 8];

	if(index < 1) index = 1;
	if(index > m_maxRecentFiles)
		return errFAILURE;

	_sntprintf(menuName, MAX_PATH + 8, _T("Recent/Files/%d"), index);
	m_persist->GetNvStr(menuName, name, nName, _T(""));
	if(name[0] == _T('\0'))
		return errFAILURE;
	else
		return errOK;
}

//**************************************************************************
ERRCODE Bed::AddRecentFile(LPCTSTR name)
{
	TCHAR menuName[MAX_PATH + 8];

	if(BrecentList::FindKey(name, m_recentFiles) != NULL)
		return errFAILURE;

	m_recentFiles = BrecentList::AddToList(new BrecentList(name, NULL), m_recentFiles);
	if(m_recentFileIndex > m_maxRecentFiles)
		m_recentFileIndex = 1;
	_sntprintf(menuName, MAX_PATH + 8, _T("Recent/Files/%d"), m_recentFileIndex);
	m_recentFileIndex++;
	m_persist->SetNvStr(menuName, name);
	return errOK;
}

//**************************************************************************
ERRCODE Bed::GetRecentProject(int index, LPTSTR name, int nName)
{
	TCHAR menuName[MAX_PATH + 8];

	if(index < 1) index = 1;
	if(index > m_maxRecentProjects)
		return errFAILURE;

	_sntprintf(menuName, MAX_PATH + 8, _T("Recent/Projects/%d"), index);
	m_persist->GetNvStr(menuName, name, nName, _T(""));
	if(name[0] == _T('\0'))
		return errFAILURE;
	else
		return errOK;
}

//**************************************************************************
ERRCODE Bed::AddRecentProject(LPCTSTR name)
{
	TCHAR menuName[MAX_PATH + 8];

	if(BrecentList::FindKey(name, m_recentProjects) != NULL)
		return errFAILURE;

	m_recentProjects = BrecentList::AddToList(new BrecentList(name, NULL), m_recentProjects);
	if(m_recentProjectIndex > m_maxRecentProjects)
		m_recentProjectIndex = 1;
	_sntprintf(menuName, MAX_PATH + 8, _T("Recent/Projects/%d"), m_recentProjectIndex);
	m_recentProjectIndex++;
	m_persist->SetNvStr(menuName, name);
	return errOK;
}

//**************************************************************************
void Bed::UpdateStatusLino()
{
	if(m_hwndStatusLino)
		InvalidateRect(m_hwndStatusLino, NULL, FALSE);
}

//**************************************************************************
void Bed::UpdateStatusInfo()
{
	if(m_hwndStatusInfo)
		InvalidateRect(m_hwndStatusInfo, NULL, FALSE);
}

//**************************************************************************
void Bed::UpdateStatusText()
{
	if(m_hwndStatusText)
		InvalidateRect(m_hwndStatusText, NULL, TRUE);
}

//**************************************************************************
void Bed::GetStatusLinoSize(int& x, int& y, int& w, int& h, LPRECT rc)
{
	x = rc->right - rc->left - STATBAR_LINO_WIDTH - 3 * STATBAR_SEP;
	y = rc->bottom - rc->top - STATBAR_HEIGHT + STATBAR_SEP;
	w = STATBAR_LINO_WIDTH;
	h = STATBAR_HEIGHT - 2 * STATBAR_SEP;
}

//**************************************************************************
void Bed::FitStatusWindows()
{
	RECT rcstat;
	int  ulx, uly, w, h;

	// status bar entries
	if(m_hwndStatusLino)
	{
		GetClientRect(GetParent(m_hwndStatusLino), &rcstat);

		GetStatusLinoSize(ulx, uly, w, h, &rcstat);
		MoveWindow(m_hwndStatusLino, ulx, uly, w, h, FALSE);

		GetStatusInfoSize(ulx, uly, w, h, &rcstat);
		MoveWindow(m_hwndStatusInfo, ulx, uly, w, h, FALSE);

		GetStatusTextSize(ulx, uly, w, h, &rcstat);
		MoveWindow(m_hwndStatusText, ulx, uly, w, h, FALSE);
	}
}

//**************************************************************************
void Bed::GetStatusInfoSize(int& x, int& y, int& w, int& h, LPRECT rc)
{
	int lx, ly, lw, lh;

	GetStatusLinoSize(lx, ly, lw, lh, rc);
	x = lx - STATBAR_INFO_WIDTH - STATBAR_SEP;
	y = rc->bottom - rc->top - STATBAR_HEIGHT + STATBAR_SEP;
	w = STATBAR_INFO_WIDTH;
	h = STATBAR_HEIGHT - 2 * STATBAR_SEP;
}

//**************************************************************************
void Bed::GetStatusTextSize(int& x, int& y, int& w, int& h, LPRECT rc)
{
	PBAFPANEL pl;
	PBAFPANE  pp;
	int lx, ly, lw, lh;

	if((pl = GetPanel(_T("Info"))) != NULL)
	{
		lx = rc->left;
		lw = 0;

		for(pp = pl->GetPanes(); pp; pp = pp->GetNext())
		{
			lw = max(lw, pp->m_tabrc.right);
		}
		x = lx + lw + STATBAR_TEXTMARG;
	}
	GetStatusInfoSize(lx, ly, lw, lh, rc);
	y = rc->bottom - rc->top - STATBAR_HEIGHT + STATBAR_SEP;
	w = lx - x - STATBAR_SEP;
	h = STATBAR_HEIGHT - 2 * STATBAR_SEP;
}

//**************************************************************************
BappPanel* Bed::GetPanel(LPCTSTR pPanelName)
{
	return FindPanel(pPanelName);
}

//**************************************************************************
BappPane* Bed::GetPane(LPCTSTR pPanelName, LPCTSTR pPaneName)
{
	PBAFPANEL	pp;

	pp = FindPanel(pPanelName);
	if(! pp) return NULL;
	return pp->FindPane(pPaneName);
}

//**************************************************************************
ERRCODE Bed::SetupWindows(bool suppressTabPanes)
{
	Bview*		pView;
	LPCTSTR		name;
	BinfoPane*	pInfo;
	PBAFPANEL   pp, px;
	PBAFPANE	pa;
	BpanelList* ppe;
	RECT		rcp, rcc;
	BAF_EDGE	edge;
	bool		docked;

	m_suppressTabPanes = suppressTabPanes;

	if(! m_suppressTabPanes)
	{
		if(m_showPicker || m_showFunctions || m_showFiles || m_showCalculator)
		{
			// create "picker" area
			//
			if(! m_pInfoPanel)
			{
				GetClientRect(m_hwnd, &rcc);

				edge	= frLeft;
				rcp		= rcc;
				rcp.right /= 4;
				docked	= true;
				DepersistPanel(_T("PickerPanel"), rcp, edge, docked);
				m_pInfoPanel = new BappPanel(_T("PickerPanel"), this);
				if(docked) Dock(m_pInfoPanel, edge, &rcp, false);
			}
		}
		pInfo = NULL;
		pp    = NULL;
		px	  = NULL;
		edge  = frTop;

		if(m_pInfoPanel)
		{
			GetClientRect(m_pInfoPanel->GetWindow(), &rcc);
		}
		name = _T("Picker");
		pp = GetPanel(name);

		if(m_showPicker || m_showCalculator)
		{
			BappPanel* pcalctainer;

			if(! pp)
			{
				// create a picker panel
				edge	= frTop;
				rcp		= rcc;
				rcp.bottom /= 5;
				docked	= true;
				DepersistPanel(name, rcp, edge, docked);
				pp = new BappPanel(name, m_pInfoPanel, (m_showPicker && m_showCalculator) ? frNoBorder : frBorder);
				if(docked) m_pInfoPanel->Dock(pp, edge, &rcp, false);

				// add panel to master panel list
				ppe = new BpanelList(name, pp);
				m_subpanels = BpanelList::AddToList(ppe, m_subpanels);
			}
			if(m_showPicker && ! m_showCalculator && ! GetPane(_T("Picker"), _T("Picker")))
			{
				if((pcalctainer = GetPanel(_T("Calculator"))) != NULL)
				{
					pp->Undock(pcalctainer);
					delete pcalctainer;
				}
				// load panel with a picker info window to start with
				pInfo = new BpickInfo(name, pp, this);
				pInfo->SetView(m_curview);
			}
			if(m_showCalculator)
			{
				Bcalc*   pcalc;

				pcalctainer = GetPanel(_T("Calculator"));

				if(! pcalctainer)
				{
					// make a panel to put inside picker panel
					//
					pcalctainer = new BappPanel(_T("Calculator"), pp, frBottomTabs);

					// dock the container panel into picker panel on the right
					pp->Dock(pcalctainer, frRight, NULL, true, true);
				}
				if(m_showPicker)
				{
					pa = pp->FindPane(_T("Picker"));

					if(pa)
					{
						pp->RemovePane(pa);
						pcalctainer->AddPane(pa);
					}
					else
					{
						// load it with a picker info window to start with
						pInfo = new BpickInfo(name, pcalctainer, this);
						pInfo->SetView(m_curview);
					}
				}
				else
				{
					if(pp->FindPane(_T("Picker")))
					{
						pa = pp->FindPane(_T("Picker"));
						pa->GetParentPanel()->RemovePane(pa);
						delete pa;
					}
				}
				if(! (pcalc = (Bcalc*)pp->FindPane(_T("bcalc"))))
				{
					// load it with a calculator panel loaded with a calc pane
					pcalc = new Bcalc(GetPersist(), pcalctainer, false);
					pcalc->RestoreSettings();
				}
				// force a paint of the panel to insure
				// rects of panes are valid
				//
				pcalctainer->SetPaneTabs();
				pcalctainer->ActivatePane(pcalc);
			}
			else
			{
				pcalctainer = GetPanel(_T("Calculator"));

				if(pcalctainer)
				{
					if(m_showPicker)
					{
						pa = pcalctainer->FindPane(_T("Picker"));
						if(pa)
						{
							pcalctainer->RemovePane(pa);
							pp->AddPane(pa);
						}
					}
					pcalctainer->GetParentPanel()->RemovePanel(pcalctainer);
					delete pcalctainer;
				}
			}
		}
		else
		{
			if(pp)
			{
				ppe = BpanelList::FindKey(name, m_subpanels);
				if(ppe)
				{
					m_subpanels = BpanelList::RemoveFromList(ppe, m_subpanels);
					delete ppe; // which also dels pp
				}
				else
				{
					delete pp;
				}
			}
			pp = NULL;
		}
		if(pp) px = pp;

		name = _T("Functions");
		pp = GetPanel(name);

		if(m_showFunctions)
		{
			if(! pp)
			{
				// create a functions panel
				edge	= frTop;
				rcp		= rcc;
				rcp.bottom /= 4;
				docked	= true;
				DepersistPanel(name, rcp, edge, docked);
				pp = new BappPanel(name, m_pInfoPanel, frBottomTabs);
				if(docked) m_pInfoPanel->Dock(pp, edge, &rcp, false);

				// add panel to master panel list
				ppe = new BpanelList(name, pp);
				m_subpanels = BpanelList::AddToList(ppe, m_subpanels);

				// load it with functions
				pInfo  = new BfuncInfo(name, pp, this);
				pInfo->SetView(m_curview);
			}
		}
		else
		{
			if(pp)
			{
				ppe = BpanelList::FindKey(name, m_subpanels);
				if(ppe)
				{
					m_subpanels = BpanelList::RemoveFromList(ppe, m_subpanels);
					delete ppe; // which also dels pp
				}
				else
				{
					delete pp;
				}
			}
			pp = NULL;
		}
		if(pp) px = pp;

		name = _T("Files");
		pp = GetPanel(name);

		if(m_showFiles)
		{

			if(! pp)
			{
				// create a file browser pane
				edge	= frTop;
				rcp		= rcc;
				rcp.bottom /= 3;
				docked	= true;
				DepersistPanel(name, rcp, edge, docked);
				pp = new BappPanel(name, m_pInfoPanel, frBottomTabs);
				if(docked) m_pInfoPanel->Dock(pp, edge, &rcp, false);

				// add panel to list
				ppe = new BpanelList(name, pp);
				m_subpanels = BpanelList::AddToList(ppe, m_subpanels);

				// load it with fileinfo
				pInfo  = new BfileInfo(name, pp, this);
				pInfo->SetView(m_curview);
			}
		}
		else
		{
			if(pp)
			{
				ppe = BpanelList::FindKey(name, m_subpanels);
				if(ppe)
				{
					m_subpanels = BpanelList::RemoveFromList(ppe, m_subpanels);
					delete ppe; // which also dels pp
				}
				else
				{
					delete pp;
				}
				pp = NULL;
			}
		}
		if(pp) px = pp;

		// last panel added is primary
		//
		if(px)
		{
			px->SetPrimary(true);
		}
		else if(m_pInfoPanel)
		{
			delete m_pInfoPanel;
			m_pInfoPanel = NULL;
		}
	}
	// create middle file area
	//
	if(! m_pEditPanel)
	{
		m_pEditPanel = new BappPanel(_T("Edit"), this, frTopTabs);
		Dock(m_pEditPanel, frLeft, NULL, false, true);
	}

	if(! m_suppressTabPanes)
	{
		//----------------------------------------------------------------------
		//  bottom tab-pane for status and info
		//
		name = _T("Info");

		if(m_showInfo)
		{
			// create status area
			//
			if(! m_pStatusPanel)
			{
				GetClientRect(m_hwnd, &rcc);

				edge	= frBottom;
				rcp		= rcc;
				rcp.top = rcp.bottom - (rcp.bottom - rcp.top) / 4;
				docked	= true;
				DepersistPanel(name, rcp, edge, docked);
				m_pStatusPanel = new BappPanel(name, this, frBottomTabs);
				Dock(m_pStatusPanel, edge, &rcp, true, false);

				// add panel to list
				ppe = new BpanelList(name, m_pStatusPanel);
				m_subpanels = BpanelList::AddToList(ppe, m_subpanels);
			}
			if(m_pStatusPanel)
			{
				struct
				{
					LPCTSTR pn;
					char    cn;
					bool*   en;
				}
				statPanes[] =
				{
					{	_T("Build"),			'b',	&m_showInfoBuild		},
					{	_T("Results"),			'r',	&m_showInfoBuildResults	},
					{	_T("Find in Files"),	'f',	&m_showInfoGrep			},
					{	_T("Find in Files 2"),	'f',	&m_showInfoGrep2		},
					{	_T("Source Control"),	's',	&m_showInfoSCCS			},
					{	_T("Debug"),			'd',	&m_showInfoDebug		},
					//{	_T("Calculator"),		'c',	&m_showCalculator		},
					{	NULL,					0,		NULL					}
				},
				*psp;

				for(psp = statPanes; psp->pn; psp++)
				{
					pInfo = (BinfoPane*)GetPane(name, psp->pn);

					if(psp->en)
					{
						if(! pInfo)
						{
							switch(psp->cn)
							{
							case 'b':
								pInfo = new BbuildInfo(psp->pn, m_pStatusPanel, this);
								break;
							case 'f':
								pInfo = new BgrepInfo(psp->pn, m_pStatusPanel, this);
								break;
							case 'r':
								pInfo = new BbuildResultsInfo(psp->pn, m_pStatusPanel, this);
								break;
							case 's':
								pInfo = new BsccsInfo(psp->pn, m_pStatusPanel, this);
								break;
							case 'd':
								pInfo = new BdbgInfo(psp->pn, m_pStatusPanel, this);
								break;
								/*
							case 'c':
								{
								Bcalc* pcalc = new Bcalc(GetPersist(), m_pStatusPanel);
								pcalc->RestoreSettings();
								}
								break;
								*/
							default:
								pInfo = NULL;
								break;
							}
							if(pInfo) pInfo->SetView(m_curview);
						}
					}
					else
					{
						if(pInfo)
						{
							m_pStatusPanel->RemovePane(pInfo);
							delete pInfo;
							pInfo = NULL;
						}
					}
				}
				if(m_pStatusPanel->GetPanes())
				{
					// force a paint of the panel to insure
					// rects of panes are valid
					//
					m_pStatusPanel->SetPaneTabs();
					//SendMessage(m_pStatusPanel->GetWindow(), WM_PAINT, 0, 0);
				}
			}
		}
		else if(m_pStatusPanel)
		{
			ppe = BpanelList::FindKey(name, m_subpanels);
			if(ppe)
			{
				m_subpanels = BpanelList::RemoveFromList(ppe, m_subpanels);
				delete ppe; // which also dels status panel
			}
			else
			{
				delete m_pStatusPanel;
			}
			m_pStatusPanel = NULL;

			if(m_hwndStatusLino)
				DestroyWindow(m_hwndStatusLino);
			m_hwndStatusLino = NULL;
			if(m_hwndStatusInfo)
				DestroyWindow(m_hwndStatusInfo);
			m_hwndStatusInfo = NULL;
			if(m_hwndStatusText)
				DestroyWindow(m_hwndStatusText);
			m_hwndStatusText = NULL;
		}
		//----------------------------------------------------------------------
		//  almost bottom tab-pane for debug info
		//
		name = _T("DebugFrame");

		if(
				(m_showWatch || m_showStack || m_showVariables
			||	 m_showMemory || m_showRegisters || m_showThreads)
			&&	(m_dbg)
		)
		{
			// create debug area
			//
			if(! m_pDebugPanel)
			{
				GetClientRect(m_hwnd, &rcc);

				edge	= frBottom;
				rcp		= rcc;
				rcp.top = rcp.bottom - (rcp.bottom - rcp.top) / 4;
				docked	= true;
				DepersistPanel(name, rcp, edge, docked);
				m_pDebugPanel = new BappPanel(name, this, frBottomTabs);
				Dock(m_pDebugPanel, edge, &rcp, true, false);

				// add panel to list
				//ppe = new BpanelList(name, m_pDebugPanel);
				//m_subpanels = BpanelList::AddToList(ppe, m_subpanels);
			}
			if(m_pDebugPanel)
			{
				struct
				{
					LPCTSTR pn;
					char    cn;
					bool*   en;
				}
				debugPanes[] =
				{
					{	_T("Stack"),			's',	&m_showStack			},
					{	_T("Registers"),		'r',	&m_showRegisters		},
					{	_T("Threads"),			't',	&m_showThreads			},
					{	_T("Watch"),			'w',	&m_showWatch			},
					{	_T("Variables"),		'v',	&m_showVariables		},
					{	NULL,					0,		NULL					}
				},
				*psp;

				name = _T("MemPanel");
				pp = GetPanel(name);

				if(m_showMemory)
				{

					if(! pp)
					{
						m_pDebugPanel->GetPanelClientRect(&rcp);

						// create a memory panel
						edge	= frRight;
						rcp		= rcc;
						rcp.left = rcp.right - (rcp.right / 3);
						docked	= true;
						DepersistPanel(name, rcp, edge, docked);
						pp = new BappPanel(name, m_pDebugPanel, frBottomTabs);
						if(docked) m_pDebugPanel->Dock(pp, edge, &rcp, false);

						// load it with initial memory pane
						pInfo  = new BmemInfo(_T("Memory"), pp, this);
						pInfo->SetView(m_curview);
					}
				}
				else
				{
					if(pp)
					{
						delete pp;
						pp = NULL;
					}
				}
				name = _T("Debug");
				pp = GetPanel(name);

				if(! pp)
				{
					m_pDebugPanel->GetPanelClientRect(&rcp);

					// create a memory panel
					edge	= frRight;
					rcp		= rcc;
					rcp.right = rcp.right - (rcp.right / 3);
					docked	= true;
					DepersistPanel(name, rcp, edge, docked);
					pp = new BappPanel(name, m_pDebugPanel, frBottomTabs);
					if(docked) m_pDebugPanel->Dock(pp, edge, &rcp, false, true);
				}
				for(psp = debugPanes; psp->pn; psp++)
				{
					pInfo = (BinfoPane*)GetPane(name, psp->pn);

					if(psp->en && *psp->en)
					{
						if(! pInfo)
						{
							switch(psp->cn)
							{
							case 'w':
								pInfo = new BwatchInfo(psp->pn, pp, this);
								break;
							case 'r':
								pInfo = new BregisterInfo(psp->pn, pp, this);
								break;
							case 's':
								pInfo = new BstackInfo(psp->pn, pp, this);
								break;
							case 't':
								pInfo = new BthreadInfo(psp->pn, pp, this);
								break;
							case 'v':
								pInfo = new BvarInfo(psp->pn, pp, this);
								break;
							default:
								pInfo = NULL;
								break;
							}
							if(pInfo) pInfo->SetView(m_curview);
						}
					}
					else
					{
						if(pInfo)
						{
							pInfo->GetParentPanel()->RemovePane(pInfo);
							delete pInfo;
							pInfo = NULL;
						}
					}
				}
				if(m_pDebugPanel)
				{
					// force a paint of the panel to insure
					// rects of panes are valid
					//
					SendMessage(m_pDebugPanel->GetWindow(), WM_PAINT, 0, 0);
				}
			}
		}
		else if(m_pDebugPanel)
		{
			ppe = BpanelList::FindKey(name, m_subpanels);
			if(ppe)
			{
				m_subpanels = BpanelList::RemoveFromList(ppe, m_subpanels);
				delete ppe; // which also dels debug panel
			}
			else
			{
				delete m_pDebugPanel;
			}
			m_pDebugPanel = NULL;
		}
		// activate the first/primary pane in each panel
		//
		for(ppe = m_subpanels; ppe; ppe = ppe->GetNext(ppe))
		{
			pp = *ppe;
			pp->ActivatePane((int)0);
		}
		// resize the panels once to get real sizes before allowing panes to
		// create windows for themselves
		//
		OnResized(m_pInfoPanel);
		OnResized(m_pStatusPanel);
		OnResized(m_pEditPanel);

		if(m_pDebugPanel)
		{
			OnResized(m_pDebugPanel);
			if(GetPanel(_T("Debug")))
				GetPanel(_T("Debug"))->ActivatePane(0);
			if(GetPanel(_T("MemPanel")))
				GetPanel(_T("MemPanel"))->ActivatePane(0);
			OnResized(m_pEditPanel);
		}
		if(m_showStatbar)
		{
			RECT rcstat;
			int  ulx, uly, w, h;

			HWND sbp;

			pp = GetPanel(_T("Info"));
			if(pp)
			{
				sbp = pp->GetWindow();
			}
			else
			{
				sbp = GetWindow();
			}
			GetClientRect(sbp, &rcstat);

			if(! m_hwndStatusLino)
			{
				GetStatusLinoSize(ulx, uly, w, h, &rcstat);
				m_hwndStatusLino = CreateWindow(_T("bed-statbar"), _T("lino"), WS_CHILD,
								ulx, uly, w, h, sbp, NULL, g_hInstance, (LPVOID)this);
				ShowWindow(m_hwndStatusLino, SW_SHOW);

				GetStatusInfoSize(ulx, uly, w, h, &rcstat);
				m_hwndStatusInfo = CreateWindow(_T("bed-statbar"), _T("binfo"), WS_CHILD,
								ulx, uly, w, h, sbp, NULL, g_hInstance, (LPVOID)this);
				ShowWindow(m_hwndStatusInfo, SW_SHOW);

				GetStatusTextSize(ulx, uly, w, h, &rcstat);
				m_hwndStatusText = CreateWindow(_T("bed-statbar"), _T("status"), WS_CHILD,
								ulx, uly, w, h, sbp, NULL, g_hInstance, (LPVOID)this);
				ShowWindow(m_hwndStatusText, SW_SHOW);
			}
			FitStatusWindows();
		}
		// make sure at least one view is active
		//
	}
	if((pView = GetViews()) != NULL)
	{
		int nViews;

		for(nViews = 0; pView; pView = pView->GetNext(pView))
		{
			if(pView->IsVisible())
			{
				nViews++;
			}
		}
		if(! nViews)
		{
			COLORREF bkg;
			BYTE     alpha;

			pView = GetViews();
			m_pEditPanel->AddPane(pView);
			m_pEditPanel->ActivatePane(0);
		    bkg = pView->GetBackground(alpha);
			SetLayeredWindowAttributes(pView->GetEditWindow(), 0, alpha, LWA_ALPHA);
#ifdef Windows
			// windows 7, only top level windows can be transparent, so apply alpha to that
			{
				HWND hwndp = pView->GetEditWindow();
				do
				{
					if (hwndp) {
						SetWindowLong(hwndp, GWL_EXSTYLE, GetWindowLong(hwndp, GWL_EXSTYLE) | WS_EX_LAYERED);
						SetLayeredWindowAttributes(hwndp, RGB(0,0,0), alpha, LWA_ALPHA);
						hwndp = GetParent(hwndp);
					}
				}
				while(hwndp);
			}
#endif
		}
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bed::ShowPanel(LPCTSTR pPanelName, LPCTSTR pPaneName, bool bShowit)
{
	bool bRemove = false;
	bool bReview = false;
	bool* pFlag = NULL;
	LPCTSTR pKey = _T("");

	if(pPaneName)
	{
		BappPane* pPane = GetPane(pPanelName, pPaneName);

		if(! bShowit && pPane)
			bRemove = bReview = true;
		else if(bShowit && ! pPane)
			bReview = true;
	}
	else
	{
		BappPanel* pPanel = GetPanel(pPanelName);

		if(! bShowit && pPanel)
			bRemove = bReview = true;
		else if(bShowit && ! pPanel)
			bReview = true;
	}
	if(! _tcscmp(pPanelName, _T("Picker")))
	{
		if(pPaneName)
		{
			if(! _tcscmp(pPaneName, _T("Picker")))
			{
				pFlag = &m_showPicker;
				pKey = _T("View/Windows/ShowPicker");
			}
			else if(! _tcscmp(pPaneName, _T("bcalc")))
			{
				pFlag = &m_showCalculator;
				pKey = _T("View/Windows/ShowCalculator");
			}
		}
	}
	else if(! _tcscmp(pPanelName, _T("Functions")))
	{
		pFlag = &m_showFunctions;
		pKey = _T("View/Windows/ShowFunctions");
	}
	else if(! _tcscmp(pPanelName, _T("Files")))
	{
		pFlag = &m_showFiles;
		pKey = _T("View/Windows/ShowFiles");
	}
	else if(! _tcscmp(pPanelName, _T("Info")))
	{
		pFlag = &m_showInfo;
		pKey = _T("View/Windows/ShowInfo");
	}
	else if(! _tcscmp(pPanelName, _T("Debug")))
	{
		pFlag = &m_showInfo;
		if(pPaneName)
		{
			if(! _tcscmp(pPaneName, _T("Watch")))
			{
				pFlag = &m_showWatch;
				pKey = _T("View/Windows/ShowWatch");
			}
			else if(! _tcscmp(pPaneName, _T("Registers")))
			{
				pFlag = &m_showRegisters;
				pKey = _T("View/Windows/ShowRegisters");
			}
			else if(! _tcscmp(pPaneName, _T("Stack")))
			{
				pFlag = &m_showStack;
				pKey = _T("View/Windows/ShowStack");
			}
			else if(! _tcscmp(pPaneName, _T("Threads")))
			{
				pFlag = &m_showThreads;
				pKey = _T("View/Windows/ShowThreads");
			}
			else if(! _tcscmp(pPaneName, _T("Variables")))
			{
				pFlag = &m_showVariables;
				pKey = _T("View/Windows/ShowVariables");
			}
			else if(! _tcscmp(pPaneName, _T("Memory")))
			{
				pFlag = &m_showMemory;
				pKey = _T("View/Windows/ShowMemory");
			}
		}
	}
	else
		return errFAILURE;

	if(pFlag)
	{
		*pFlag = bShowit;
		m_persist->SetNvBool(pKey, bShowit);
	}
	if(bReview)
	{
		SetupWindows(false);
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bed::DepersistPanel(LPCTSTR pPanelName, RECT& rc, BAF_EDGE& e, bool& docked)
{
	TCHAR		key[256];
	BappPanel*	pPanel = GetPanel(pPanelName);
	int			x, y, w, h;
	ERRCODE		ec;

	x = rc.left;
	y = rc.top;
	w = rc.right - rc.left;
	h = rc.bottom - rc.top;

	_sntprintf(key, 256, _T("View/Windows/"_Pfs_"/X"), pPanelName);
	ec = m_persist->GetNvInt(key, x, x);
	_sntprintf(key, 256, _T("View/Windows/"_Pfs_"/Y"), pPanelName);
	ec = m_persist->GetNvInt(key, y, y);
	_sntprintf(key, 256, _T("View/Windows/"_Pfs_"/W"), pPanelName);
	ec = m_persist->GetNvInt(key, w, w);
	_sntprintf(key, 256, _T("View/Windows/"_Pfs_"/H"), pPanelName);
	ec = m_persist->GetNvInt(key, h, h);
	_sntprintf(key, 256, _T("View/Windows/"_Pfs_"/E"), pPanelName);
	ec = m_persist->GetNvInt(key, (int&)e, e);

	rc.left		= x;
	rc.top		= y;
	rc.right	= x + w;
	rc.bottom	= y + h;

	return ec;
}

//**************************************************************************
ERRCODE Bed::PersistPanel(BappPanel* pPanel)
{
	TCHAR		key[256];
	int			x, y, w, h;
	ERRCODE		ec;
	RECT		rc;

	if(! pPanel) return errOK;

	GetClientRect(pPanel->GetWindow(), &rc);

	x = rc.left;
	y = rc.top;
	w = rc.right - rc.left;
	h = rc.bottom - rc.top;

	LPCTSTR pn = pPanel->GetName();

	_sntprintf(key, 256, _T("View/Windows/"_Pfs_"/X"), pn);
	ec = m_persist->SetNvInt(key, x);
	_sntprintf(key, 256, _T("View/Windows/"_Pfs_"/Y"), pn);
	ec = m_persist->SetNvInt(key, y);
	_sntprintf(key, 256, _T("View/Windows/"_Pfs_"/W"), pn);
	ec = m_persist->SetNvInt(key, w);
	_sntprintf(key, 256, _T("View/Windows/"_Pfs_"/H"), pn);
	ec = m_persist->SetNvInt(key, h);
	_sntprintf(key, 256, _T("View/Windows/"_Pfs_"/E"), pn);
	ec = m_persist->SetNvInt(key, pPanel->GetEdge());

	// recursive persistance of sub panels
	//
	PersistPanel(pPanel->GetNext());
	PersistPanel(pPanel->GetPanels(frLeft));
	PersistPanel(pPanel->GetPanels(frRight));
	PersistPanel(pPanel->GetPanels(frTop));
	PersistPanel(pPanel->GetPanels(frBottom));
	return ec;
}

//**************************************************************************
void Bed::DrawStatusItem(HDC hdc, HWND hWnd)
{
	TCHAR txtBuffer[MAX_PATH];
	HFONT hOldFont;

	int x = 4;
	int y = 1;

	if(! m_curview) return;

	hOldFont = (HFONT)SelectObject(hdc, m_hFont);

	if(hWnd == m_hwndStatusLino)
	{
		if(m_curview)
		{
			_sntprintf(txtBuffer, sizeof(txtBuffer)/sizeof(TCHAR), _T("Line: %d Col: %d      "),
				m_curview->GetCurLine(),
				m_curview->GetCurCol()
				);
			TextOut(hdc, x*2, y, txtBuffer, _tcslen(txtBuffer));
		}
		else
		{
		}
	}
	else if(hWnd == m_hwndStatusInfo)
	{
		TCHAR over, ro, q, mod;

		if(m_curview->GetCaseSensi())
		{
			over = m_curview->GetOverstrike() ? _T('o'):_T('i');
		}
		else
		{
			over = m_curview->GetOverstrike() ? _T('O'):_T('I');
		}
		ro  = m_curview->GetBuffer()->GetReadOnly() ? _T('R') : _T(' ');
		q   = m_curview->GetHandMarked() ? _T('M') : _T(' ');
		mod = m_curview->GetBuffer()->GetModified() ? _T('*') : _T(' ');

		_sntprintf(txtBuffer, 128, _T("%c%c%c%c %d%c%d "), over, q, ro, mod,
				m_curview->GetTabSpace(),
				m_curview->GetTabsAsSpaces() ? '_' : 't',
				m_curview->GetIndentSpace());
		TextOut(hdc, x, y, txtBuffer, _tcslen(txtBuffer));
	}
	else if(hWnd == m_hwndStatusText)
	{
		RECT   rc;
		HBRUSH hbrBkg;
		int    tlen;

		hbrBkg = CreateSolidBrush(GetBkColor(hdc));
		GetClientRect(m_hwndStatusText, &rc);
		rc.top += 2;
		rc.bottom -= 2;
		rc.left += 2;
		rc.right -= 2;
		FillRect(hdc, &rc, hbrBkg);
		DeleteObject(hbrBkg);
		tlen = GetWindowText(m_hwndStatusText, txtBuffer, sizeof(txtBuffer)/sizeof(TCHAR));
		if (! tlen)
			txtBuffer[0] = _T('\0');
		TextOut(hdc, x, y, txtBuffer, _tcslen(txtBuffer));
	}
	SelectObject(hdc, hOldFont);
}

//**************************************************************************
void Bed::SetStatus(LPCTSTR lpText)
{
	if(lpText && m_hwndStatusText)
	{
		SetWindowText(m_hwndStatusText, (LPTSTR)lpText);
		UpdateStatusText();
	}
}

//**************************************************************************
PBAFPANEL Bed::GetEditPanel(void)
{
	if(m_curview) return m_curview->GetParentPanel();
	else		  return m_pEditPanel;
}

//**************************************************************************
void Bed::AddOldView(Bview* pView)
{
	if(! pView) return;
	pView->SetClosed(1);
	pView->SetNext(m_oldviews);
	m_oldviews = pView;
}

//**************************************************************************
void Bed::AddView(Bview* pView)
{
	Bview* pLast;

	if(! pView) return;
	if(! m_views)
		m_views = pView;
	else
	{
		for(pLast = m_views; pLast->GetNext(pLast);)
			pLast = pLast->GetNext(pLast);
		pLast->SetNext(pView);
	}
	pView->SetNext(NULL);

	// check through old list and remove garbage
	//
	for(pLast = m_oldviews; pLast;)
	{
		m_oldviews = pLast->GetNext(pLast);
		delete pLast;
		pLast = m_oldviews;
	}
}

//**************************************************************************
void Bed::RemoveView(Bview* pView)
{
	Bview* pPrev;

	if(! pView) return;
	if(pView == m_views)
		m_views = pView->GetNext(pView);
	else
	{
		for(pPrev = m_views; pPrev->GetNext(pPrev); pPrev = pPrev->GetNext(pPrev))
		{
			if(pPrev->GetNext(pPrev) == pView)
			{
				pPrev->SetNext(pView->GetNext(pView));
				break;
			}
		}
	}
	if(m_curview == pView)
		m_curview = pView->GetNext(pView);
	if(! m_curview)
		m_curview = m_views;

	AddOldView(pView);
}

//**************************************************************************
ERRCODE Bed::SetCurrentView(Bview* pView, bool replace)
{
	Bview*  pPrevView = m_curview;

	if(m_curview && m_curview == pView && m_curview->GetEditWindow())
	{
		if(GetFocus() != m_curview->GetEditWindow())
			::SetFocus(m_curview->GetEditWindow());
		UpdateInfoPanes(pView);
		return errOK;
	}

	if(pView != NULL)
		m_curview = pView;

	if(! pView)
		return errFAILURE;

	if(! m_curview->GetEditWindow() || ! m_curview->IsVisible())
	{
		COLORREF bkg;
		BYTE alpha;

		if(
				pPrevView
			&&	pPrevView->IsVisible()
			&&	replace
			&&	(pPrevView->GetParentPanel() == pView->GetParentPanel())
		)
		{
			// replace current view as active pane in current panel
			//
			pPrevView->Deactivate();
		}
		else if(! pView->GetParentPanel())
		{
			// add this pane to active (or only) panel
			//
			if(m_curview && m_curview->GetParentPanel())
			{
				m_curview->GetParentPanel()->AddPane(pView);
			}
			else
			{
				if(m_pEditPanel)
				{
					m_pEditPanel->AddPane(pView);
				}
				else
				{
					// panic! no panels
					return errFAILURE;
				}
			}
		}
		pView->GetParentPanel()->ActivatePane(pView);

		bkg = pView->GetBackground(alpha);
		SetLayeredWindowAttributes(pView->GetEditWindow(), 0, alpha, LWA_ALPHA);

		// if more than one view, have to resize
		//
		if(pView != m_views || pView->GetNext(pView))
			if(m_parent)
				PostMessage(m_parent->GetWindow(), WM_SIZE, 0, 0);
	}
	else
	{
		RECT rc, rcp;

		// hide any views (besides this) that overlap
		//
		for(pPrevView = m_views; pPrevView; pPrevView = pPrevView->GetNext(pPrevView))
		{
			if((pPrevView != pView) && pPrevView->IsVisible())
			{
				GetWindowRect(pPrevView->GetEditWindow(), &rcp);

				if(IntersectRect(&rcp, &rcp, &rc))
				{
					pPrevView->Deactivate();
				}
			}
		}
		pView->Activate();
	}
	::SetFocus(m_curview->GetEditWindow());
	UpdateInfoPanes(m_curview);
	return errOK;
}

//**************************************************************************
void Bed::UpdateInfoPanel(Bview* pView, BappPanel* pp)
{
	BappPane*		p;

	for(; pp; pp = pp->GetNext())
	{
		for(p = pp->GetPanes(); p; p = p->GetNext())
		{
			if(_tcscmp(p->GetName(), _T("bcalc")))
				((BinfoPane*)p)->SetView(pView);
		}
		UpdateInfoPanel(pView, pp->GetPanels(frLeft));
		UpdateInfoPanel(pView, pp->GetPanels(frRight));
		UpdateInfoPanel(pView, pp->GetPanels(frTop));
		UpdateInfoPanel(pView, pp->GetPanels(frBottom));
	}
}

//**************************************************************************
void Bed::UpdateInfoPanes(Bview* pView)
{
	// tell subpanes about the new view
	//
	BpanelList*		ppe;


	if(! pView) return;

	SetTitle(pView->GetBuffer());

	for(ppe = m_subpanels; ppe; ppe = ppe->GetNext(ppe))
	{
		UpdateInfoPanel(pView, *ppe);
	}
	return;
}
// STATIC
//**************************************************************************
Bbuffer* Bed::ProperBufferForFile(LPCTSTR pName, BufType type, TEXTENCODING encoding, TEXTENCODING defaultencoding, Bed* editor)
{
	if(type == btAny && pName)
		type = Bbuffer::SniffType(pName, encoding);
	if(type == btRaw)
		return new BrawBuffer(pName, type, encoding, defaultencoding, editor);
	else
		return new BtextBuffer(pName, type, encoding, defaultencoding, editor);
}

//**************************************************************************
Bview* Bed::ProperViewForBuffer(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel)
{
	Bview* pView;

	switch(pBuf->GetType())
	{
	case btRaw:
		pView = new BviewHex(pBuf, pEditor, pPanel);
		break;

	case btC:
		pView = new BviewC(pBuf, pEditor, pPanel);
		break;

	case btCPP:
		pView = new BviewCPP(pBuf, pEditor, pPanel);
		break;

	case btCS:
		pView = new BviewCS(pBuf, pEditor, pPanel);
		break;

	case btOBJC:
		pView = new BviewObjectiveC(pBuf, pEditor, pPanel);
		break;

	case btJava:
	case btPHP:
	case btJavaScript:
		pView = new BviewJava(pBuf, pEditor, pPanel);
		break;

	case btASM:
		pView = new BviewASM(pBuf, pEditor, pPanel);
		break;

	case btPython:
		pView = new BviewPython(pBuf, pEditor, pPanel);
		break;

	case btTCL:
		pView = new BviewTCL(pBuf, pEditor, pPanel);
		break;

	case btHTML:
		pView = new BviewHTML(pBuf, pEditor, pPanel);
		break;

	case btXML:
		pView = new BviewHTML(pBuf, pEditor, pPanel);
		break;

	case btShell:
		pView = new BviewShell(pBuf, pEditor, pPanel);
		break;

	case btSwift:
		pView = new BviewSwift(pBuf, pEditor, pPanel);
		break;

	case btTelnet:
			pView = new BviewTelnet(pBuf, pEditor, pPanel);
		break;

	case btTerm:
	//	if(BserialStream::IsSerialPort(pBuf->GetName()))
			pView = new BviewRS232(pBuf, pEditor, pPanel);
		break;

	case btText:
	case btAny:
	default:
		pView = new Bview(pBuf, pEditor, pPanel);
		break;
	}
	// setup view-type specific things
	//
	if(pView)
	{
		pView->DepersistParms();
	}
	return pView;
}

//**************************************************************************
void Bed::SetBufferStatus(Bbuffer* pBuf)
{
	TCHAR estat[MAX_PATH];
	LPCTSTR name, szenc, szlf;
	if(! pBuf) return;

	name  = pBuf->GetName();
	szenc = GetEncodingName(pBuf->GetEncoding());
	szlf  =	pBuf->GetHasCR() ? _T("CR-LF") : _T("LF");

	_sntprintf(
				estat,
				sizeof(estat)/sizeof(TCHAR),
		#ifdef UNICODE
				_T("%ls - %d lines from %d bytes [%ls %ls]"),
		#else
				"%s - %d lines from %d bytes [%s %s]",
		#endif
				name,
				pBuf->GetReadLines(),
				pBuf->GetReadSize(),
				szenc,
				szlf
		);
	SetStatus(estat);
}


//**************************************************************************
void Bed::AddBufferToPDB(Bbuffer* pBuf)
{
	// pass off the buffer created to the project/program database
	// thread to explore (if enabled) (if buffer type is program)
	//
	if(m_project && m_project->GetPDB())
	{
		switch(pBuf->GetType())
		{
		default:
			break;

		case btC:
		case btCPP:
		case btCS:
		case btOBJC:
		case btJava:
		case btJavaScript:
		case btPHP:
		case btTCL:
		case btPython:
		case btVerilog:
		case btPerl:
		//	m_project->GetPDB()->AddFile(pBuf);
			m_project->GetPDB()->AddFile(pBuf->GetName());
			break;
		}
	}
}

//**************************************************************************
ERRCODE Bed::EditBuffer(Bbuffer* pBuf, int startLine)
{
	Bview*  pView;
	ERRCODE ec;

	if(! pBuf)	return errBAD_PARAMETER;

	// read in the buffer, unless its a device
	//
	if(pBuf->GetType() != btTerm)
	{
		ec = pBuf->Read();
	}
	else
	{
		ec = errOK;
	}
	if(ec != errOK)
	{
		SetStatus(_T("Can't read buffer"));
		return ec;
	}
	SetBufferStatus(pBuf);

	// if the buffer has a view just
	// use it directly
	//
	if((pView = GetViews()) != NULL)
	{
		while(pView)
		{
			if(pView->GetBuffer() == pBuf)
			{
				SetCurrentView(pView, true);
				if (startLine)
				{
					pView->PushParm(startLine, ptNumber);
					pView->Dispatch(MoveToLine);
				}
				return errOK;
			}
			pView = pView->GetNext(pView);
		}
	}
	// create a proper view of it depending on extension
	//
	pView = ProperViewForBuffer(pBuf, this, GetEditPanel());

	if(! BbufList::FindKey(pBuf->GetName(), m_buffers))
	{
		BbufList* pb;

		// add this buffer to our list if not there already
		// have to check against actual ptr since bufname could change
		//
		for(pb = m_buffers; pb; pb = pb->GetNext(pb))
			if(pb->GetValue() == pBuf)
				break;
		if(! pb)
			m_buffers = BbufList::AddToList(new BbufList(pBuf->GetName(), pBuf), m_buffers);
	}
	// add to recent files list
	//
	if(! pBuf->GetNew())
		AddRecentFile(pBuf->GetName());

	// add the view to list of views, replacing the current
	// view if any present
	//
	AddView(pView);
	SetCurrentView(pView, true);
	if (startLine)
	{
		pView->PushParm(startLine, ptNumber);
		pView->Dispatch(MoveToLine);
	}

	if(pBuf->GetType() != btTerm)
	{
		// add as PDB fodder
		AddBufferToPDB(pBuf);
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bed::EditBuffer(LPCTSTR pName, bool frg, BufType type, TEXTENCODING encoding, int startLine)
{
	BbufList* pEntry;
	Bbuffer*  pBuf;
	TCHAR	  fullpath[MAX_PATH + 32];

	fullpath[0] = _T('\0');
	if(pName && pName[0])
	{
		if(
				! _tcsncmp(pName, _T("ftp:"), 4)
			||	! _tcsncmp(pName, _T("http:"), 5)
		)
		{
			_tcscpy(fullpath, pName);
		}
		else
		{
			GetFullPathName(pName, MAX_PATH, fullpath, NULL);
		}
	}
	// check to see if any buffers share the same file
	// as this one (get stat and fullpath info)
	//
	for(pEntry = m_buffers; pEntry; pEntry = pEntry->GetNext(pEntry))
	{
		if(Bbuffer::IsSameFile(pEntry->GetValue()->GetName(), fullpath))
		{
			pBuf = *pEntry;

			// already editing this
			//
			return EditBuffer(pBuf, startLine);
		}
	}
	// create a buffer for the file
	//
	pBuf = Bed::ProperBufferForFile(pName ? fullpath : NULL, type, encoding, m_defaultencoding, this);

	// add this buffer to our list
	//
	m_buffers = BbufList::AddToList(new BbufList(pBuf->GetName(), pBuf), m_buffers);

	if(frg) return EditBuffer(pBuf, startLine);
	else    return errOK;
}

//**************************************************************************
ERRCODE Bed::RemoveBuffer(Bbuffer* pBuf)
{
	BbufList* pEntry;

	if(! (pEntry = BbufList::FindKey(pBuf->GetName(), m_buffers)))
		return errBAD_PARAMETER;
	m_buffers = BbufList::RemoveFromList(pEntry, m_buffers);
	delete pEntry;
	return errOK;
}

//**************************************************************************
ERRCODE Bed::NewProject()
{
	ERRCODE ec;
	TCHAR	fullPath[MAX_PATH];

	if((ec = CloseProject()) == errOK)
	{
		m_project = new Bproject(this);

		ec = m_project->ProjectWizard(GetPersist());

		if(ec != errOK)
		{
			delete m_project;
			m_project = NULL;
		}
		else
		{
			if((ec = m_project->GetProjectNamePath(fullPath, MAX_PATH)) == errOK)
			{
				LPTSTR pName = fullPath;

				while(*pName != _T('.') && *pName)
					pName++;
				if(*pName == _T('.'))
					*pName = _T('\0');
				AddRecentProject(fullPath);
			}
		}
	}
	return ec;
}

//**************************************************************************
ERRCODE Bed::OpenProject(LPCTSTR pName, bool restoreit)
{
	ERRCODE ec;
	TCHAR	fullPath[MAX_PATH];

	if((ec = CloseProject()) == errOK)
	{
		m_project = new Bproject(this);

	//	GetFullPathName(pName, MAX_PATH, fullPath, NULL);

		ec = m_project->Open(pName);

		if(ec != errOK)
		{
			MessageBox(NULL, _T("Project can't be opened"), GetVersion(), MB_OK);
			delete m_project;
			m_project = NULL;
		}
		else
		{
			if((ec = m_project->GetProjectNamePath(fullPath, MAX_PATH)) == errOK)
			{
				LPTSTR pName = fullPath;

				while(*pName != _T('.') && *pName)
					pName++;
				if(*pName == _T('.'))
					*pName = _T('\0');
				AddRecentProject(fullPath);
			}
			// opened a project, which may have started
			// a PDB thread.  If pdb is found, add at
			// least the current view to the pdb fodder
			// to get quick results for this file, then
			// add all the source files in the project tree
			// that match the view's file type pattern
			//
			if(m_project->GetPDB() && GetCurrentView())
			{
				Bbuffer* pBuf = GetCurrentView()->GetBuffer();
				TCHAR projectRoot[MAX_PATH];
				LPCTSTR pExtensions;
				
				if(pBuf)
				{
					// add as PDB fodder
					AddBufferToPDB(pBuf);
				}
				pExtensions = GetCurrentView()->GetSearchExtensions();
				if (pExtensions)
				{
					if(m_project->GetSourceDirectory(projectRoot, MAX_PATH - 4) == errOK)
					{	
						_tcscat(projectRoot, _PTS_ "*");
						m_project->GetPDB()->AddSourceTree(projectRoot, pExtensions);
					}
				}
			}
			// restore files in project as buffers if restoreit set
			//
			if(restoreit)
			{
				m_project->RestoreViews();
			}
		}
		HMENU hMenu = GetMenu(m_hwnd);

		if(hMenu)
			SendMessage(m_hwnd, WM_INITMENU, (WPARAM)(LPARAM)hMenu, 0);
	}
	return ec;
}

//**************************************************************************
ERRCODE Bed::CloseProject()
{
	if(m_project)
	{
		m_project->SaveViews();
		if(m_project->GetModified())
		{
			TCHAR pname[MAX_PATH + 64];
			int   rc;

			_sntprintf(pname, MAX_PATH + 64, _T("Save Changes to "_Pfs_" ?"), m_project->GetName());
			rc = MessageBox(NULL, pname, GetVersion(), MB_OKCANCEL);
			if(rc == IDCANCEL)
				return errFAILURE;
		}
		delete m_project;
		m_project = NULL;
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bed::SetupSCCS(bool resetup)
{
	ERRCODE ec;
	bool	bv;

	ec = m_persist->GetNvBool(_T("SCCS/Integration/IsSetup"), bv, false);
	if(! bv) resetup = true;

	if(resetup)
	{
		if(m_sccs)
		{
			delete m_sccs;
			m_sccs = NULL;
		}
		m_sccs = new Bsccs(this);

		ec = m_sccs->SCCSWizard();

		if(ec != errOK)
		{
			delete m_sccs;
			m_sccs = NULL;
		}
		else
		{
			;
		}
	}
	else
	{
		if(! m_sccs)
		{
			m_sccs = new Bsccs(this);
		}
		else
		{
			m_persist->SetNvBool(_T("SCCS/Integration/IsSetup"), false);
			delete m_sccs;
			m_sccs = NULL;
		}
		ec = errOK;
	}
	return ec;
}

//**************************************************************************
ERRCODE Bed::SetupDebug(bool resetup)
{
	ERRCODE ec;
	bool	bv;

	ec = m_persist->GetNvBool(_T("Debug/IsSetup"), bv, false);
	if(! bv) resetup = true;

	if(resetup)
	{
		if(m_dbg)
		{
			delete m_dbg;
			m_dbg = NULL;
		}
		m_dbg = new Bdbg(this, NULL);

		ec = m_dbg->DebugWizard();

		if(ec != errOK)
		{
			delete m_dbg;
			m_dbg = NULL;
		}
		else
		{
			delete m_dbg;
			m_dbg = NULL;
			ec = Debug();
		}
	}
	else
	{
		ec = Debug();
	}
	return ec;
}

//**************************************************************************
ERRCODE Bed::Debug(void)
{
	ERRCODE ec;

	if(m_dbg)
	{
		// stop debugging
		delete m_dbg;
		m_dbg = NULL;
		SetupWindows(m_suppressTabPanes);
		return errOK;
	}
	else
	{
		BdbgProgram prog;
		Bview*		pView;

		// prompt for changed buffers
		//
		for(pView = GetViews(); pView; pView = pView->GetNext(pView))
		{
			if(! pView->CheckModified(true))
				return errFAILURE;
		}

		// make sure debug console showing
		//
		BappPane* pPane = GetPane(_T("Info"), _T("Debug"));

		m_persist->GetNvInt(_T("Debug/Program"), (int&)prog, dbgGDB);

		switch(prog)
		{
		case dbgGDB:
			m_dbg = new BdbgGDB(this, (BdbgInfo*)pPane);
			break;
		default:
			m_dbg = NULL;
			return errNOT_IMPLEMENTED;
		}
		if(m_dbg)
		{
			SetupWindows(m_suppressTabPanes);
			ec = m_dbg->Init();
			if(ec != errOK)
			{
				SetStatus(_T("Can't Start Debugger"));
				return ec;
			}
			// if there is a project, add breakpoints in project
			//
			if(m_project)
			{
				ec = m_dbg->RestoreBreakpoints(m_project, ! m_firstdebug);
				m_firstdebug = false;
			}
			return errOK;
		}
		return errFAILURE;
	}
}

//**************************************************************************
void Bed::SetTitle(Bbuffer* pBuf)
{
	SetWindowText(m_hwnd, (LPTSTR)GetTitle(pBuf));
	UpdateStatusLino();
	UpdateStatusInfo();
}

//**************************************************************************
LPCTSTR Bed::GetVersion()
{
	_sntprintf(m_title, MAX_PATH, _T(""_Pfs_" %d.%d"),
		g_appname, g_vermaj, g_vermin);
	return m_title;
}

//**************************************************************************
LPCTSTR Bed::GetEncodingName(TEXTENCODING enc)
{
	switch(enc)
	{
	default:
	case txtANSI:		return _T("ANSI");
	case txtUTF8:		return _T("UTF-8");
	case txtUCS2:		return _T("UCS2");
	case txtUCS2SWAP:	return _T("UCS2 LE");
	case txtUCS4:		return _T("UCS4");
	case txtUCS4SWAP:	return _T("UCS4 LE");
	}
}

//**************************************************************************
LPCTSTR Bed::GetTypeName(BufType type)
{
	switch(type)
	{
	default:
	case btRaw:			return _T("Binary");
	case btAny:
	case btText:		return _T("Text");
	case btC:			return _T("C");
	case btCPP:			return _T("C++");
	case btCS:			return _T("C#");
	case btJava:		return _T("Java");
	case btJavaScript:	return _T("JavaScript");
	case btPython:		return _T("Python");
	case btTCL:			return _T("TCL");
	case btPHP:			return _T("PHP");
	case btVerilog:		return _T("Verilog");
	case btASM:			return _T("ASM");
	case btPerl:		return _T("Perl");
	case btHTML:		return _T("HTML");
	case btXML:			return _T("XML");
	case btShell:		return _T("Shell");
	case btSwift:		return _T("Swift");
	case btTelnet:		return _T("Telnet");
	case btTerm:		return _T("Terminal");
	}
}

//**************************************************************************
LPCTSTR Bed::GetTitle(Bbuffer* pBuf)
{
	LPCTSTR	pTitle;
	LPCTSTR pType;
	LPCTSTR pEncoding;
	bool	modified;

	// formulate title text
	//
	pTitle   = pBuf->GetName();
	modified = pBuf->GetModified();

	pType = GetTypeName(pBuf->GetType());
	pEncoding = GetEncodingName(pBuf->GetEncoding());

	_sntprintf(m_title, MAX_PATH * 2 - 1,
#ifdef UNICODE
				 _T("%ls%ls%ls %d.%d - %ls   %ls %ls %c"),
#else
				"%s%s%s %d.%d 0 %s    %s %s %c",
#endif
				m_project ? m_project->GetName() : _T(""),
				m_project ? _T(" - ") : _T(""),
				g_appname, g_vermaj, g_vermin,
				pTitle, pType, pEncoding, (modified ? _T('*') : _T(' '))
			);


	return m_title;
}

//**************************************************************************
ERRCODE Bed::MoveTo(int& line, int& col, Bbuffer* pBuf)
{
	ERRCODE ec;
	LPCTSTR lpText;
	int     nText;

	ec = pBuf->GetLineText(line, lpText, nText);

	if(ec != errOK)
	{
		line = pBuf->GetCurLine();

		ec = pBuf->GetLineText(line, lpText, nText);

		if(ec != errOK) return ec;
	}
	if(col < 0)		col = 1;
	if(col > nText) col = nText;
	return errOK;
}


//**************************************************************************
ERRCODE Bed::Dispatch(
						EditCommand command,
						LPCTSTR 	pParm,
						int			nParm,
						ParmType	parmType,
						Bbuffer* 	pBuf,
						Bview* 		pView
					  )
{
	ERRCODE		ec			= errOK;
	BbufList* 	pEntry		= NULL;

	if(! pBuf || ! pView)
		return errBAD_PARAMETER;

	return ec;
}


//***********************************************************************
LRESULT CALLBACK BedStatbarProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC			hdc;
	PAINTSTRUCT	ps;
	RECT		rc;
	Bed*		pBed;
	HPEN		hpen, hpenold;

	pBed = (Bed*)GetWindowLong(hWnd, GWL_USERDATA);

	switch (message)
	{
	case WM_CREATE:

		{
			LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;

			SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)pcs->lpCreateParams);
		}
		break;

	case WM_LBUTTONDOWN:

		SetCapture(hWnd);
		break;

	case WM_LBUTTONUP:

		ReleaseCapture();
		break;

	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rc);

		rc.right--;
		rc.bottom--;

		// br outside
		hpen	= CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNHIGHLIGHT));
		hpenold = (HPEN)SelectObject(hdc, hpen);
		MoveToEx(hdc, rc.left, rc.bottom, NULL);
		LineTo(hdc, rc.right, rc.bottom);
		LineTo(hdc, rc.right, rc.top);
		SelectObject(hdc, hpenold);
		DeleteObject(hpen);

		// ul outside
		hpen	= CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNSHADOW));
		hpenold = (HPEN)SelectObject(hdc, hpen);
		MoveToEx(hdc, rc.right, rc.top, NULL);
		LineTo(hdc, rc.left, rc.top);
		LineTo(hdc, rc.left, rc.bottom);
		SelectObject(hdc, hpenold);
		DeleteObject(hpen);

		SetBkColor(hdc, GetSysColor(COLOR_3DLIGHT));
		pBed->DrawStatusItem(hdc, hWnd);

		EndPaint(hWnd, &ps);
		break;

	default:

		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


