#include "bedx.h"

//**************************************************************************
BdbgInfo::BdbgInfo(LPCTSTR name, BappPanel* pPanel, Bed* pParent)
	:
	BshellInfo(name, pPanel, pParent)
{
}

//**************************************************************************
BdbgInfo::~BdbgInfo()
{
}

//**************************************************************************
LPCTSTR BdbgInfo::GetPathPrefix()
{
	return BshellInfo::GetPathPrefix();
}

//**************************************************************************
void BdbgInfo::AppendItem(LPCTSTR pLine)
{
	Bdbg* pdbg;

	pdbg = m_editor->GetDebugger();
	if(pdbg)
	{
		if(pdbg->OnDebuggerData(pLine) == errOK)
			return;
	}
	BshellInfo::AppendItem(pLine);
}

//**************************************************************************
ERRCODE BdbgInfo::Startup(void)
{
	Bdbg* pdbg;
	
	if(! Exited())
	{
		if(m_view)
		{
			int rc = MessageBox(NULL, _T("Cancel Current Debugger ?"),
				m_view->GetBuffer()->GetEditor()->GetTitle(m_view->GetBuffer()), MB_YESNO);
			if(rc == IDNO) return errFAILURE;
		}
		Stop();
	}
	Clear();
	
	TCHAR szShellName		[256];
	TCHAR szSwitches		[32];
	TCHAR szShellCommandSep	[32]		= _T("&");
	TCHAR szDebugDir		[MAX_PATH];
	TCHAR szDebugProgram	[256]		= _T("gdb -f");
	TCHAR szDebugTarget		[MAX_PATH]	= _T("bed");

	BdbgProgram prog;

	pdbg = m_editor->GetDebugger();
	if(! pdbg) return errFAILURE;
	
	pdbg->GetDebugTarget	(szDebugTarget,		MAX_PATH);
	_tcscpy(szDebugDir, _T("."));
	pdbg->GetDebugDir		(szDebugDir,		MAX_PATH);
	pdbg->GetDebugProgram	(prog);
	switch(prog)
	{
	case dbgGDB:
	default:
		break;
		
	case dbgDEBUG:
		_tcscpy(szDebugProgram, _T("debug"));
		break;
	}
	pdbg->GetShell			(szShellName,		256);
	pdbg->GetShellSwitches	(szSwitches,		32);
	pdbg->GetShellSep		(szShellCommandSep,	32);

	m_toklen = 0;
	
	#ifdef UNICODE
	_sntprintf(m_szShellCommand, 1024, L"%ls %ls \"cd %ls %ls %ls %ls\"\n",
			szShellName, szSwitches, szDebugDir,
			szShellCommandSep,
			szDebugProgram, szDebugTarget);
	#else
	_snprintf(m_szShellCommand, 1024, "%s %s \"cd %s %s %s %s\"\n",
			szShellName, szSwitches, szDebugDir,
			szShellCommandSep,
			szDebugProgram, szDebugTarget);
	#endif
	_tcscpy(m_szShellDir, _T(""));
	return BshellInfo::Startup();
}


//**************************************************************************
//**************************************************************************
//**************************************************************************


//**************************************************************************
void BstackInfo::FitToPanel()
{
	if(! m_hwnd)
	{
		m_hwnd = m_hwndTree = MakeNewTree();
		if(m_hwnd)
		{
			ShowWindow(m_hwnd, m_active ? SW_SHOW : SW_HIDE);
			if(m_editor)
			{
				Bdbg* dbg = m_editor->GetDebugger();

				if(dbg)
				{
					BeginStackFrame();
					if(dbg->GetRunning())
						AddStackLine(_T("target running..."));
					else
						AddStackLine(_T("target not started"));
				}
			}
		}
	}
}

//**************************************************************************
void BstackInfo::OnExec(BdbgRunCode rc)
{
	if(rc != rcStop && rc != rcQuit && rc != rcBreak)
	{
		BeginStackFrame();
		AddStackLine(_T("target running..."));
	}
}

//**************************************************************************
void BstackInfo::BeginStackFrame()
{
	TreeView_DeleteAllItems(m_hwndTree);
	m_nStackDex = 0;
}

//**************************************************************************
void BstackInfo::AddStackLine(LPCTSTR pLine)
{
	TVINSERTSTRUCT tvItem;

	tvItem.hParent		= NULL;
	tvItem.hInsertAfter = TVI_LAST;

	tvItem.item.mask	= TVIF_TEXT | TVIF_PARAM ;//| TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvItem.item.pszText = (LPTSTR)pLine;
	tvItem.item.lParam	= m_nStackDex;
	tvItem.item.iImage	= 2;
	tvItem.item.iSelectedImage = tvItem.item.iImage;
	TreeView_InsertItem(m_hwndTree, &tvItem);
	m_nStackDex++;
}

