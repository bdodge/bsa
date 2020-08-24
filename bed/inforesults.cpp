#include "bedx.h"

//**************************************************************************
BbuildResultsInfo::BbuildResultsInfo(LPCTSTR name, BappPanel* pPanel, Bed* pParent)
	:
	BinfoPane(name, pPanel, pParent)
{
}

//**************************************************************************
BbuildResultsInfo::~BbuildResultsInfo()
{
}

//**************************************************************************
LPCTSTR BbuildResultsInfo::GetPathPrefix()
{
	if(! m_srcDir[0])
		return _T("");
	return m_srcDir;
}


//**************************************************************************
void BbuildResultsInfo::SetView(Bview* pView)
{
	if(! pView) return;
	m_ignoreNotify = true;
	BinfoPane::SetView(pView);
	TreeView_SelectItem(m_hwndTree, NULL);
	m_ignoreNotify = false;
}

//**************************************************************************
void BbuildResultsInfo::Select(LPTVITEM pItem)
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
void BbuildResultsInfo::Clear(void)
{
	TreeView_DeleteAllItems(m_hwndTree);
}

//**************************************************************************
void BbuildResultsInfo::AppendItem(LPCTSTR pLine)
{
	static TVINSERTSTRUCT tvItem;
	
	tvItem.hParent		= NULL;
	tvItem.hInsertAfter = TVI_LAST;
	tvItem.item.mask	= TVIF_TEXT;
	tvItem.item.pszText = (LPTSTR)pLine;

	TreeView_InsertItem(m_hwndTree, &tvItem);
}
