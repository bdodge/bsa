#include "bedx.h"

//**************************************************************************
BshellInfo::BshellInfo(LPCTSTR name, BappPanel* pPanel, Bed* pParent)
	 :
	BinfoPane(name, pPanel, pParent),
	m_toklen(0),
	m_head(0),
	m_tail(0),
	m_cnt(0)
{
#ifdef TIMER_BASED_SHELLPOLL
	m_idTimer = NULL;
#endif
	m_szShellDir[0] = _T('\0');
}

//**************************************************************************
BshellInfo::~BshellInfo()
{
#ifdef TIMER_BASED_SHELLPOLL
	if(m_idTimer)
		KillTimer(NULL, m_idTimer);
#elif defined SHELL_BASED_SHELLPOLL
	m_reader.Stop();
#endif
	Stop();
}

//**************************************************************************
void BshellInfo::SetView(Bview* pView)
{
	if(! pView) return;
	m_ignoreNotify = true;
	BinfoPane::SetView(pView);
	TreeView_SelectItem(m_hwndTree, NULL);
	m_ignoreNotify = false;
}

//**************************************************************************
void BshellInfo::SpecificMenu(HMENU hMenu)
{
	AppendMenu(hMenu, 0, ID_TOOLS_CLEAR, _T("C&lear"));
	//AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	//AppendMenu(hMenu, 0, ID_FILE_SAVE,	_T("&Save"));
	//AppendMenu(hMenu, 0, ID_FILE_CLOSE, _T("&Close"));
}


//**************************************************************************
void BshellInfo::Select(LPTVITEM pItem)
{
	ERRCODE ec;
	TCHAR	fName[MAX_PATH];
	LPTSTR	pFname;
	int     line;

	if(m_ignoreNotify)					return;
	if(! m_editor)						return;
	if(! (pItem->mask & TVIF_TEXT))		return;

	ec = BbuildInfo::GetErrorLine(pItem->pszText, fName, MAX_PATH, &line);

	if(ec == errOK && fName[0])
	{
		TCHAR		path[MAX_PATH];
		int			plen;

		path[0] = _T('\0');

		// if relative path, prepend directory in which the shell ran
		//
		if(fName && fName[0] != _PTC_ && fName[1] != _T(':'))
		{
			_tcscpy(path, GetPathPrefix());
			plen = _tcslen(path);
			if(plen > 0 && plen < MAX_PATH-1 && path[plen - 1] != _PTC_)
			{
				path[plen++] = _PTC_;
				path[plen] = _T('\0');
			}
		}
		pFname = fName;

		while(*pFname == _T(' ') || *pFname == _T('\t'))
			pFname++;
		_tcscat(path, pFname);
		plen = _tcslen(path);

		if(plen > 0)
		{
			for(pFname = path + plen - 1; pFname >= path; pFname--)
			{
				if(*pFname != _T(' ') && *pFname != _T('\t'))
					break;
			}
			if(pFname >= path)
			*++pFname = _T('\0');
		}
		if(m_editor->EditBuffer(path, true, btAny) == errOK)
		{
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
	}
}

//**************************************************************************
ERRCODE WINAPI BshellInfo::RunStub(LPVOID thisptr)
{
	return ((BshellInfo*)thisptr)->ShellThread();
}

//**********************************************************************
ERRCODE BshellInfo::ShellThread()
{
	ERRCODE	ec;
	int		nRead;
	int		nRoom;
	int		endwait = 0;
	int		empty   = 5;
	
	ec = Init(m_szShellCommand);
	if(ec != errOK) return ec;

	m_head = 0;
	m_tail = 0;
	m_size = sizeof(m_iobuf) - 1;
	m_cnt = 0;

	if(GetInsertHeader())
	{
		TCharToChar(m_iobuf, GetOperation());
		TCharToChar(m_iobuf + strlen(m_iobuf), m_szShellCommand);
		m_head = strlen(m_iobuf);
	}
	do
	{
		if(Poll(m_hsioRead, 0, 10000) == errSTREAM_DATA_PENDING)
		{
			if(m_cnt < m_size)
			{	
				if(m_head >= m_tail)
					nRoom = m_size - m_head;
				else
					nRoom = m_tail - m_head;
			}
			else
			{
				nRoom = 0;
			}
			if(nRoom > 0)
			{	
				Block lock(&m_bufex);

				if((nRead = Read(m_hsioRead, m_iobuf + m_head, nRoom)) > 0)
				{				
					m_head += nRead;
					if (m_head >= m_size)
						m_head = 0;
					m_cnt += nRead;
					m_iobuf[m_head] = 0;
				}
				//printf("stdout got =%s= %d room=%d\n", m_iobuf + m_head - nRead, nRead, nRoom);
			}
			else
			{
				Bthread::Sleep(150);
			}
		}
		if(m_hsioErrRead != m_hsioRead && Poll(m_hsioErrRead, 0, 10000) == errSTREAM_DATA_PENDING)
		{
			if(m_cnt < m_size)
			{				
				Block lock(&m_bufex);

				if(m_head >= m_tail)
					nRoom = m_size - m_head;
				else
					nRoom = m_tail - m_head;
				
				if((nRead = Read(m_hsioErrRead, m_iobuf + m_head, nRoom)) > 0)
				{			
					m_head += nRead;
					if (m_head >= m_size)
						m_head = 0;
					m_cnt += nRead;
					m_iobuf[m_head] = 0;
				}
				//printf("stderr got =%s= %d room=%d\n", m_iobuf + m_head - nRead, nRead, nRoom);
			}
			else
			{
				Bthread::Sleep(150);
			}
		}
		if(m_cnt > 0)
		{
#ifdef TIMER_BASED_SHELLPOLL
#elif defined SHELL_BASED_SHELLPOLL
			m_devent.Signal();
#else
			Event(0);
#endif
		}
	}
	while((! Exited() && m_running) || empty-- > 0); // this is tricky, empty doesn't decrement till exit

	// wait for reader to catch up (but not forever)
	//
	do
	{
		if(m_cnt)
		{
#ifdef TIMER_BASED_SHELLPOLL
#elif defined (SHELL_BASED_SHELLPOLL)
			m_devent.Signal();
#else
			Event(0);
#endif
			endwait++;
			Bthread::Sleep(100);
		}
	}
	while(endwait < 6 && m_cnt);

	m_running = false;
	Finished();
	return ec;
}

//**************************************************************************
void BshellInfo::Clear(void)
{
	TreeView_DeleteAllItems(m_hwndTree);
}

//**************************************************************************
void BshellInfo::AppendItem(LPCTSTR pLine)
{
	TVINSERTSTRUCT	tvItem;
	HTREEITEM		hItem;

	tvItem.hParent		= NULL;
	tvItem.hInsertAfter = TVI_LAST;
	tvItem.item.mask	= TVIF_TEXT;
	tvItem.item.pszText = (LPTSTR)pLine;

	hItem = TreeView_InsertItem(m_hwndTree, &tvItem);
	TreeView_SelectItem(m_hwndTree, hItem);
	
//	if((ni++ & 0x1f) == 0x1f)
//	{
//		SendMessage(m_hwndTree, WM_PAINT, 0, 0);
//	}
}

//**************************************************************************
void BshellInfo::Event(LPARAM lParam)
{		
	TCHAR 	szBuffer[SHELL_IO_SIZE * 2 + 2];
	int     nText;
	char    c;
	int		i;

	if (m_cnt > 0)
	{	
		if(m_tail >= m_head)
			nText = m_size - m_tail;
		else
			nText = m_head - m_tail;
	}
	else
	{
		nText = 0;
	}
	// got data from the shell thread
	//
	if(nText > 0)
	{
		m_bufex.Lock();

		c = m_iobuf[m_tail + nText];
		m_iobuf[m_tail + nText] = 0;

		//printf("event, tok=%s n=%d\n", m_iobuf+m_tail, nText);
		
#ifdef UNICODE
		BUtil::Utf8Decode(szBuffer, m_iobuf + m_tail);
#else
		strcpy(szBuffer, m_iobuf + m_tail);
#endif
#if 0
		for(i = 0; i < nText; i++)
		{
			printf("%02X %c\n", m_iobuf[i + m_tail], m_iobuf[i + m_tail]);
			//printf("%02X %c\n", szBuffer[i], szBuffer[i]);
		}
#endif	
		m_iobuf[m_tail + nText] = c;
		m_tail += nText;
		m_cnt -= nText;
		if(m_tail >= m_size)
			m_tail = 0;
		if (m_cnt == 0)
		{
			m_tail = 0;
			m_head = 0;
		}
		m_bufex.Unlock();
		
		// parse strings into lines
		nText = _tcslen(szBuffer);

		for(i = 0; i < nText; i++)
		{
			if(
					(szBuffer[i] == '\n')		 	||
					(m_toklen >= SHELL_IO_SIZE - 1)	||
					(i >= nText && ! m_running)
			)
			{
				m_token[m_toklen] = 0;
				//_tprintf(_T("tok="_Pfs_"\n"), m_token);
				AppendItem(m_token);
				m_toklen = 0;
			}
			if(szBuffer[i] != '\n' && szBuffer[i] != '\r')
			{
				m_token[m_toklen++] = szBuffer[i];
			}
		}
	}
}

//**************************************************************************
ERRCODE BshellInfo::SendShell(PBYTE cmd, DWORD& ncmd)
{
	int nbytes;

	nbytes = Write(m_hsioWrite, (LPSTR)cmd, ncmd);

	return nbytes == (int)ncmd ? errOK : errFAILURE;
}

//**************************************************************************
void BshellInfo::Finished()
{
	m_pid = 0;
#ifdef THREAD_BASED_SHELLPOLL
	m_devent.Signal();	
#endif
	Deinit();
}

//**************************************************************************
ERRCODE BshellInfo::Startup(void)
{
	if(! Exited())
	{
		int rc = MessageBox(NULL, _T("Cancel Current Shell Operation?"),
			m_view->GetBuffer()->GetEditor()->GetTitle(m_view->GetBuffer()), MB_YESNO);
		if(rc == IDNO) return errFAILURE;

		Stop();
	}
	if(GetParentPanel())
		GetParentPanel()->ActivatePane(this);

	SetWindowLong(m_hwndTree, GWL_USERDATA, (LONG)this);

#ifdef TIMER_BASED_SHELLPOLL
	m_idTimer = SetTimer(m_hwndTree, 0xFACE, 100, OnTimer);
#elif defined SHELL_BASED_SHELLPOLL
	m_reader.Start(ReaderThreadStub, this);
#endif
	return Start(RunStub, this);
}

#ifdef TIMER_BASED_SHELLPOLL
// STATIC
//**************************************************************************
void CALLBACK BshellInfo::OnTimer(HWND hWnd, UINT msg, UINT id, DWORD now)
{
	BshellInfo* pShellTab = (BshellInfo*)GetWindowLong(hWnd, GWL_USERDATA);
	
	if(pShellTab)
		pShellTab->Event(0);
}
#elif defined SHELL_BASED_SHELLPOLL

// STATIC
//**************************************************************************
ERRCODE BshellInfo::ReaderThreadStub(LPVOID thisptr)
{
	BshellInfo* pShellTab = (BshellInfo*)thisptr;

	if(pShellTab)
		return pShellTab->ReaderThread();
	return errFAILURE;
}

//**************************************************************************
ERRCODE BshellInfo::ReaderThread(void)
{
	while(m_reader.GetRunning())
	{
		m_devent.Wait(500);
		Event(0);
	}
	return errOK;
}
#endif

bool IsPathChar(TCHAR c)
{
	return	(c == '.' || c == '/') 	||
#ifdef Windows
			(c == ':' || c == '\\') ||
#endif
			(c >= 'a' && c <= 'z')	||
			(c >= 'A' && c <= 'Z')	||
			(c == ' ' || c == '_')	||
			(c == '-' || c == '~')	||
			(c >= '0' && c <= '9');
}

//**************************************************************************
ERRCODE BshellInfo::GetErrorLine(LPCTSTR lpText, LPTSTR fName, int nfName, int* pLine)
{
	int		n, lino;
	LPTSTR	fp, dp;
	bool	useQuote;

	// look for first digit in text after the first non-path char found
	//
	for(fp = (LPTSTR)lpText; IsPathChar(*fp);)
		fp++;

	while(*fp && (*fp < '0' || *fp > '9'))
		fp++;

	//printf("linotxt =%s=\n", fp);

	if(*fp < '0' || *fp > '9')
	{
		// no error number 
		//
		return errFAILURE;
	}
	lino = _tcstol(fp, NULL, 0);
	if(pLine) *pLine = lino;

	// Look back from here for first "." which indicates
	// the middle of a filename, or first path delimiter
	//
	while(*fp != _T('.') && *fp != _T('\\') && *fp != _T('/') && fp >= lpText)
		fp--;
	
	// now find the end of the filename
	//
	while(IsPathChar(*fp))
		fp++;


	// if terminated on a dbl quote, remember that
	// to look for open quote to the left (allows white space in filenames)
	//
	useQuote = *fp == _T('\"');

	/*printf("begin=%s\n", fp);*/
	// Now find the beginning of a filename
	//	
	// this assumes there is a least one non-white space
	// between filename and line number text
	//
	if(! *fp)
	{
		return errFAILURE;
	}

	// ignore trailing white space in filename
	//
	fp--;
	if(! useQuote)
	{
		while(*fp == ' ' || * fp == '\t' || (*fp == ':' && *(fp+1) != _PTC_))
			fp--;
	}
	while(fp >= lpText)
	{
		if(useQuote)
		{
			if(*fp == _T('\"'))
				break;
		}
		else
		{
			if(*fp == ' ' || *fp == '\t' || (*fp == ':' && *(fp+1) != _PTC_))
				break;
		}
		fp--;
	}
	fp++;

	// now copy filename to buffer
	//
	for(dp = fName, n = 0; n < (nfName - 1) && (IsPathChar(*fp)); n++)
	{
		*dp++ = *fp++;
	}
	*dp = _T('\0');

	if(lino && fName[0])
		return errOK;
	return errFAILURE;
}	
