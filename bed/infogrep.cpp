#include "bedx.h"

//**************************************************************************
static bool IsDelim(TCHAR ic)
{
	if(
		ic == _T(' ')  || ic == _T('\t') || ic == _T('\n') || ic == _T('(')  ||
		ic == _T(')')  || ic == _T(',')  || ic == _T(';')  || ic == _T('+')  ||
		ic == _T(':')  || ic == _T('<')  || ic == _T('>')  || ic == _T('.')  ||
		ic == _T('-')  || ic == _T('/')  || ic == _T('\\') || ic == _T('*')  ||
		ic == _T('^')  || ic == _T('|')  || ic == _T('"')  || ic == _T('\'') ||
		ic == _T('{')  || ic == _T('}')  || ic == _T('[')  || ic == _T(']')  ||
		ic == _T('&')  || ic == _T('=')  || ic == _T('%')  || ic == _T('@')
	)
		return true;
	else
		return false;
}

//**************************************************************************
static BOOL CALLBACK GrepProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BgrepInfo* pInfo = NULL;

	RECT	 rc;
	RECT	 rcme;
	HWND	 hwndParent;
	PARMPASS dp;
	TCHAR	 szBuffer[MAX_PATH + 2];
	TCHAR	 szPath[MAX_PATH + 2];
	int		 bdrc;

	switch (message)
	{
	case WM_INITDIALOG:

		pInfo = (BgrepInfo*)lParam;
		if(! pInfo) break;

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
		SetDlgItemText(hWnd, IDC_GREPPAT, (LPTSTR)pInfo->GetPattern());
		SetDlgItemText(hWnd, IDC_GREPDIR, (LPTSTR)pInfo->GetPath());
		SetDlgItemText(hWnd, IDC_GREPFILEPAT, (LPTSTR)pInfo->GetFilePattern());
		CheckDlgButton(hWnd, IDC_GREPRECURSIVE, pInfo->GetRecurseDirs()   ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_GREPCASESENSI, pInfo->GetCaseSensi() ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_GREPWHOLEWORD, pInfo->GetWholeWord() ? BST_CHECKED : BST_UNCHECKED);
		return TRUE;

	case WM_COMMAND:

		switch(wParam)
		{
		case IDC_BROWSEDIR:

			GetDlgItemText(hWnd, IDC_GREPDIR, szBuffer, MAX_PATH);
			BfileInfo::SimplifyFilePath(szBuffer, szPath);
			dp.lpTitle	 = _T("BED 6.0 - Pick Directory");
			dp.lpString  = szPath;
			dp.nString   = MAX_PATH;
			dp.lpInitVal = szPath;
			bdrc = PickDirectoryDialog(&dp, GetParent(hWnd));
			if(bdrc == IDOK)
			{
				bdrc = _tcslen(dp.lpString);
				if(bdrc > 0)
				{
					if(dp.lpString[bdrc-1] != _PTC_)
					{
						dp.lpString[bdrc++] = _PTC_;
						dp.lpString[bdrc] = _T('\0');
					}
				}
				else
				{
					bdrc = 0;
					dp.lpString[bdrc++] = _T('.');
					dp.lpString[bdrc++] = _PTC_;
					dp.lpString[bdrc] = _T('\0');
				}
				SetDlgItemText(hWnd, IDC_GREPDIR, dp.lpString);
			}
			break;

		case IDOK:

			GetDlgItemText(hWnd, IDC_GREPPAT, szBuffer, 128);
			pInfo->SetPattern(szBuffer);
			GetDlgItemText(hWnd, IDC_GREPFILEPAT, szBuffer, 128);
			pInfo->SetFilePattern(szBuffer);
			pInfo->SetRecurseDirs(IsDlgButtonChecked(hWnd, IDC_GREPRECURSIVE) != 0);
			pInfo->SetCaseSensi(IsDlgButtonChecked(hWnd, IDC_GREPCASESENSI) != 0);
			pInfo->SetWholeWord(IsDlgButtonChecked(hWnd, IDC_GREPWHOLEWORD) != 0);
			GetDlgItemText(hWnd, IDC_GREPDIR, szBuffer, MAX_PATH);
			BfileInfo::SimplifyFilePath(szBuffer, szPath);
			pInfo->SetPath(szPath);
			EndDialog(hWnd, IDOK);
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
BgrepInfo::BgrepInfo(LPCTSTR name, BappPanel* pPanel, Bed* pParent)
	:
	BshellInfo(name, pPanel, pParent),
	m_fileList(NULL),
	m_curFile(NULL),
	m_ftypeList(NULL),
	m_casesensi(false),
	m_recursive(false),
	m_wholeword(false),
	m_nPat(0),
	m_occurenceCount(0),
	m_occurFiles(0),
	m_fileCount(0)
{
	m_filePattern = new TCHAR [2];
	m_filePattern[0] = _T('*');
	m_filePattern[1] = _T('\0');
}

//**************************************************************************
BgrepInfo::~BgrepInfo()
{
	Stop();
	BgrepList::FreeList(m_fileList);
	BfpatList::FreeList(m_ftypeList);
	if(m_filePattern)
		delete [] m_filePattern;
}

//**********************************************************************
ERRCODE BgrepInfo::InsertGrepLine(LPCTSTR grepLine, int maxtimems)
{
	int nRoom;
	int nNeeded;
	int tlen;
	int timeout;

	// wait for room available for string
	//
	nNeeded = _tcslen(grepLine) * 3 + 2; // need extra to encode and a newline
	if(nNeeded > SHELL_IO_SIZE)
		return errOVERFLOW;
	timeout = 0;

	do
	{
		if (m_cnt < m_size)
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
		if(nRoom < nNeeded)
		{
			// tell reader thread there's data available
#ifdef TIMER_BASED_SHELLPOLL
#elif defined SHELL_BASED_SHELLPOLL
			m_devent.Signal();
#else
			Event(0);
#endif
			Bthread::Sleep(100);
			timeout++;
		}
	}
	while(nRoom < nNeeded && (timeout < (maxtimems / 100)));

	if(nRoom < nNeeded)
		return errFAILURE;

	m_bufex.Lock();
#ifdef UNICODE
	BUtil::Utf8Encode(m_iobuf + m_head, grepLine, false);
#else
	strcpy(m_iobuf + m_head, grepLine);
#endif
	tlen = strlen(m_iobuf + m_head);
	strcpy(m_iobuf + m_head + tlen, "\r\n");
	m_head += tlen + 2;
	m_cnt += tlen + 2;
	m_bufex.Unlock();
	return errOK;
}

//**********************************************************************
ERRCODE BgrepInfo::ShellThread()
{
	ERRCODE	ec = errOK;
	int		nRoom;
	int		endwait = 0;

	BfpatList* pFilePat;

	m_head = 0;
	m_tail = 0;
	m_size = sizeof(m_iobuf) - 1;
	m_cnt  = 0;

	if(GetInsertHeader())
	{
		TCHAR grepLine[256];

		_sntprintf(grepLine, 256, _T("Search "_Pfs_" "_Pfs_" for: "_Pfs_" in "_Pfs_""),
				m_wholeword ? _T("whole-word") : _T(""),
				m_recursive ? _T("recursive") : _T(""),
				m_grepPat, m_grepDir);
		InsertGrepLine(grepLine, 5000);
	}
	// build a list of files to look at
	//
	BgrepList::FreeList(m_fileList);
	m_fileList = NULL;
	BuildFileList(m_grepDir);
	m_curFile = m_fileList;

	while(m_curFile && m_running)
	{
		if(m_ftypeList)
		{
			LPCTSTR pRem, filePart;

			for(pFilePat = m_ftypeList; pFilePat && m_running; pFilePat = pFilePat->GetNext(pFilePat))
			{
				filePart = m_curFile->GetName();
				if(filePart)
				{
					LPCTSTR pfp;

					pfp = filePart + _tcslen(filePart) - 1;
					while(pfp > filePart)
					{
						if(*pfp == _PTC_)
						{
							pfp++;
							break;
						}
						pfp--;
					}

					pRem = BUtil::SimplePatternMatch(pfp, pFilePat->GetName());

					//_tprintf(_T("compare="_Pfs_" pat "_Pfs_"= res=%d\n"),
					//		m_curFile->GetName(), pFilePat->GetName(), pRem ? 1 : 0);

					if(pRem && ! *pRem)
					{
						// get the next file in the list of files to pa
						//
						ec = CheckFile(m_curFile->GetName());
						break;
					}
				}
			}
		}
		else
		{
			ec = CheckFile(m_curFile->GetName());
		}
		if(ec == errOK)
			m_occurFiles++;

		if(! m_running)
			break;
		// tell reader thread theres data available
#ifdef TIMER_BASED_SHELLPOLL
#elif defined SHELL_BASED_SHELLPOLL
		m_devent.Signal();
#else
		Event(0);
#endif
		m_curFile = m_curFile->GetNext(m_curFile);
	}
	if(GetInsertHeader())
	{
		TCHAR grepLine[256];

		_sntprintf(grepLine, 256, _T("Search complete: %d occurances in %d files"), m_occurenceCount, m_fileCount);
		InsertGrepLine(grepLine, 5000);
	}

	// wait for reader to catch up (but not forever)
	//
	do
	{
		if(m_cnt)
		{
#ifdef TIMER_BASED_SHELLPOLL
#elif defined SHELL_BASED_SHELLPOLL
			m_devent.Signal();
#else
			Event(0);
#endif
			endwait++;
			Bthread::Sleep(100);
		}
	}
	while(m_running && endwait < 6 && m_cnt);

#ifdef SHELL_BASED_SHELLPOLL
	m_devent.Signal();
#endif
	m_running = false;
	Finished();
	return ec;
}

//**************************************************************************
void BgrepInfo::Finished()
{
	BgrepList::FreeList(m_fileList);
	BfpatList::FreeList(m_ftypeList);
	m_fileList = NULL;
	m_ftypeList = NULL;
	if(m_filePattern)
		delete [] m_filePattern;
	m_filePattern = NULL;
}

//**************************************************************************
void BgrepInfo::SetPattern(LPCTSTR pattern)
{
	_tcsncpy(m_grepPat, pattern, MAX_PATH-1);
	m_grepPat[MAX_PATH - 1] = _T('\0');
}

//**************************************************************************
void BgrepInfo::SetPath(LPCTSTR path)
{
	int len;

	_tcsncpy(m_grepDir, path, MAX_PATH-2);
	m_grepDir[MAX_PATH - 2] = _T('\0');
	len = _tcslen(m_grepDir);
	if(len == 0) len = 1;
	if(m_grepDir[len - 1] != _PTC_)
	{
		m_grepDir[len] = _PTC_;
		m_grepDir[len + 1] = _T('\0');
	}
}

//**************************************************************************
void BgrepInfo::SetFilePattern(LPCTSTR pattern)
{
	if(m_filePattern)
		delete [] m_filePattern;
	m_filePattern = new TCHAR [ _tcslen(pattern) + 2 ];
	_tcscpy(m_filePattern, pattern);
	BfpatList::FreeList(m_ftypeList);
	m_ftypeList = NULL;
	BuildTypeList(m_filePattern);
}

//**************************************************************************
ERRCODE BgrepInfo::BuildTypeList(LPCTSTR pattern)
{
	LPCTSTR pp = pattern;
	LPCTSTR pe;
	TCHAR   pat[128];
	int		i;

	if(! pp) return errFAILURE;
	do
	{
		for(pe = pp, i = 0; *pe && *pe != _T(';') && *pe != _T(',') && i < 127; i++)
			pat[i] = *pe++;
		pat[i] = _T('\0');
		m_ftypeList = BfpatList::AddToList(new BfpatList(pat, NULL), m_ftypeList);
		pp = pe + 1;
	}
	while(*pe == _T(',') || *pe == _T(';'));
	return errOK;
}

//**************************************************************************
ERRCODE BgrepInfo::BuildFileList(LPCTSTR path)
{
	HDIRLIST	hDir;
	ERRCODE		ec;
	LPCTSTR		lpName;
	bool		isDir, isLink, isReadOnly;

	if (m_recurse_level >= GREP_MAX_RECURSE)
	{
		return errOK;
	}

	if((ec = BfileInfo::ListDirectory(hDir, path)) != errOK)
		return ec;

	m_recurse_level++;

	while((ec = BfileInfo::NextFile(hDir, lpName, isDir, isLink, isReadOnly)) == errOK)
	{
		if(isDir)
		{
			if(m_recursive && (_tcslen(lpName) > 0))
			{
				TCHAR npath[MAX_PATH];
				TCHAR ct;

				ct = lpName[_tcslen(lpName) - 1];
				if(ct != _T('.'))
				{
					_tcscpy(npath, lpName);
					if(ct != _PTC_)
						_tcscat(npath, _PTS_);
					ec = BuildFileList(npath);
				}
			}
		}
		else
		{
			m_fileList = BgrepList::AddToList(new BgrepList(lpName, NULL), m_fileList);
		}

		if(IsDone())
		{
			m_recurse_level--;
			return errFAILURE;
		}
	}
	m_recurse_level--;
	return ec;
}

//**************************************************************************
ERRCODE BgrepInfo::CheckFile(LPCTSTR name)
{
	Bbuffer* pBuf;
	ERRCODE  ec;
	bool	 foundlines = false;

	pBuf = Bed::ProperBufferForFile(name, btAny, (TEXTENCODING)-1, txtANSI, m_editor);
	if(! pBuf) return errFAILURE;

	ec = pBuf->Read();
	if(ec == errOK)
	{
		LPCTSTR lpText;
		int		nText;

		int line = 1;
		int col  = 1;
		int pline, pcol;

		m_fileCount++;
		pline = -1;
		pcol  = -1;

		while(m_running && ((ec = pBuf->Locate(line, col, m_grepPat, m_nPat, m_casesensi, false)) == errOK))
		{
			if(line < pline)
				break;
			if(line == pline)
			{
				if(col <= pcol)
					break;
			}
			else
			{
				ec = pBuf->GetLineText(line, lpText, nText);

				if(ec == errOK)
				{
					TCHAR tempText[512];
					int   tlen, room;
					bool leftok = false;
					bool rightok = false;
					TCHAR c;

					if(m_wholeword)
					{
						// make sure the char to the left and right of text
						// is an operator or space or bol or eol
						//
						if(col == 1)
						{
							leftok = true;
						}
						else
						{
							c = lpText[col - 2];
							if(IsDelim(c))
							{
								leftok = true;
							}
						}
						col += m_nPat;
						if (col >= nText)
						{
							rightok = true;
						}
						else
						{
							c = lpText[col - 1];
							if(IsDelim(c))
							{
								rightok = true;
							}
						}
					}
					else
					{
						leftok = rightok = true;
					}
					if (leftok && rightok)
					{
						m_occurenceCount++;

						tlen = _sntprintf(tempText, 512, _T(""_Pfs_" (%d) "), name, line);
						room = 510 - tlen;
						if(nText > 128)
							nText = 128;
						if(nText > room)
							nText = room;
						_tcsncpy(tempText + tlen, lpText, nText);
						tempText[tlen + nText] = '\0';

						for(room = tlen; room < tlen + nText; room++)
						{
							if(tempText[room] == _T('\n') || tempText[room] == _T('\r'))
							{
								tempText[room] = _T('\0');
								break;
							}
							else if(tempText[room] == _T('\t'))
							{
								tempText[room] = _T(' ');
							}
						}
						foundlines = true;
						InsertGrepLine(tempText, 5000);
					}
				}
				if(! m_running || IsDone())
					break;
			}
			pline = line;
			pcol  = col;

			col++;
		}
	}
	delete pBuf;
	return foundlines ? errOK : errFAILURE;
}

//**************************************************************************
ERRCODE BgrepInfo::Startup(
						   LPCTSTR lpIniPattern,
						   LPCTSTR lpIniPath,
						   LPCTSTR lpIniFileTypes,
						   bool matchWholeWord
						  )
{
	TCHAR szFilePattern[MAX_PATH];
	int rc, len;

	if(m_running)
	{
		if(m_view)
		{
			int rc = MessageBox(NULL, _T("Cancel Current Find-In-File ?"),
				m_view->GetBuffer()->GetEditor()->GetTitle(m_view->GetBuffer()), MB_YESNO);
			if(rc == IDNO) return errFAILURE;
		}
		Stop();
	}
	Clear();

	m_recurse_level = 0;
	m_fileCount = m_occurFiles = m_occurenceCount = 0;

	// Get the starting directory
	//
	if(! lpIniPath)	lpIniPath = _T(".");
	GetFullPathName(lpIniPath, MAX_PATH, m_grepDir, NULL);

	len = _tcslen(m_grepDir);
	if(len > 0 && len < MAX_PATH-1)
	{
		if(m_grepDir[len-1] != _PTC_)
		{
			m_grepDir[len++] = _PTC_;
			m_grepDir[len]   = _T('\0');
		}
	}
	// starting file type templates
	//
	if(! lpIniFileTypes) lpIniFileTypes = _T("*");

	if(! lpIniPattern || ! lpIniPattern[0])
		m_editor->GetPersist()->GetNvStr(_T("FindInFile/SearchFor"), m_grepPat, MAX_PATH, _T(""));
	else
		_tcsncpy(m_grepPat, lpIniPattern, MAX_PATH-1);
	m_grepPat[MAX_PATH-1] = _T('\0');

	m_editor->GetPersist()->GetNvStr(_T("FindInFile/StartingPath"),	 m_grepDir, MAX_PATH, m_grepDir);
	m_editor->GetPersist()->GetNvStr(_T("FindInFile/FilePattern"),	 szFilePattern, MAX_PATH, lpIniFileTypes);
	m_editor->GetPersist()->GetNvBool(_T("FindInFile/Recursive"),	 m_recursive, true);
	m_editor->GetPersist()->GetNvBool(_T("FindInFile/CaseSensitive"),m_casesensi, true);
	m_editor->GetPersist()->GetNvBool(_T("FindInFile/MatchWholeWord"),m_wholeword, matchWholeWord);

	SetFilePattern(szFilePattern);

	// do the dialog to get real parms
	//
	rc = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_FINDINFILES), NULL, GrepProc, (LPARAM)this);
	if(rc != IDOK) return errFAILURE;

	m_nPat = Bview::UnescapeString(m_searchstr, m_grepPat, false);

	// persist the directory and file pattern
	//
	m_editor->GetPersist()->SetNvStr(_T("FindInFile/SearchFor"),	 m_grepPat);
	m_editor->GetPersist()->SetNvStr(_T("FindInFile/StartingPath"),	 m_grepDir);
	m_editor->GetPersist()->SetNvStr(_T("FindInFile/FilePattern"),	 m_filePattern);
	m_editor->GetPersist()->SetNvBool(_T("FindInFile/Recursive"),	 m_recursive);
	m_editor->GetPersist()->SetNvBool(_T("FindInFile/CaseSensitive"),m_casesensi);
	m_editor->GetPersist()->SetNvBool(_T("FindInFile/MatchWholeWord"),m_wholeword);

	// start the search
	//
	_tcscpy(m_szShellCommand, _T(""));
	return BshellInfo::Startup();
}

//**************************************************************************
ERRCODE BgrepInfo::StartupDirect(
						   LPCTSTR lpIniPattern,
						   LPCTSTR lpIniPath,
						   LPCTSTR lpIniFileTypes
						  )
{
	int rc, len;

	if(m_running)
	{
		Stop();
	}
	Clear();

	m_fileCount = m_occurFiles = m_occurenceCount = 0;

	// Get the starting directory
	//
	if(! lpIniPath)	lpIniPath = _T(".");
	GetFullPathName(lpIniPath, MAX_PATH, m_grepDir, NULL);

	len = _tcslen(m_grepDir);
	if(len > 0 && len < MAX_PATH-1)
	{
		if(m_grepDir[len-1] != _PTC_)
		{
			m_grepDir[len++] = _PTC_;
			m_grepDir[len]   = _T('\0');
		}
	}
	// starting file type templates
	//
	if(! lpIniFileTypes)
		lpIniFileTypes = _T("*");

	if(! lpIniPattern || ! lpIniPattern[0])
		return errFAILURE;

	_tcsncpy(m_grepPat, lpIniPattern, MAX_PATH-1);
	m_grepPat[MAX_PATH-1] = _T('\0');

	SetFilePattern(lpIniFileTypes);

	m_nPat = Bview::UnescapeString(m_searchstr, m_grepPat, false);

	m_wholeword = true;
	m_recursive = true;

	// start the search
	//
	_tcscpy(m_szShellCommand, _T(""));
	return BshellInfo::Startup();
}

