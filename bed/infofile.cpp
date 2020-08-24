#include "bedx.h"


//**************************************************************************
BfileInfo::BfileInfo(LPCTSTR name, BappPanel* pPanel, Bed* pParent)
	:
	BinfoPane(name, pPanel, pParent, true, true)
{
	_tcscpy(m_path, _T("zzzzzzzaaaaaaa\2\2\2\2\2\2\2"));
}

//**************************************************************************
BfileInfo::~BfileInfo()
{
}

//**************************************************************************
void BfileInfo::LoadImages()
{
#if 1
	HICON hIcon;

	if(! m_ilist) return;

	hIcon = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_FILE16), IMAGE_ICON, 16, 16, 0);
	ImageList_AddIcon(m_ilist, hIcon);
	hIcon = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_FILERO16), IMAGE_ICON, 16, 16, 0);
	ImageList_AddIcon(m_ilist, hIcon);
	hIcon = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_DIR16), IMAGE_ICON, 16, 16, 0);
	ImageList_AddIcon(m_ilist, hIcon);
	hIcon = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_DIROPEN16), IMAGE_ICON, 16, 16, 0);
	ImageList_AddIcon(m_ilist, hIcon);
#else
	HBITMAP hBitmap;

	if(! m_ilist) return;

	hBitmap = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_FILE16));
	ImageList_Add(m_ilist, hBitmap, NULL);
	hBitmap = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_FILERO16));
	ImageList_Add(m_ilist, hBitmap, NULL);
	hBitmap = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_DIR16));
	ImageList_Add(m_ilist, hBitmap, NULL);
	hBitmap = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_DIROPEN16));
	ImageList_Add(m_ilist, hBitmap, NULL);
#endif
}

//**************************************************************************
void BfileInfo::SetView(Bview* pView)
{
	HWND	 hWnd;
	HWND	 hOldWnd;
	Bbuffer* buffer;
	LPCTSTR  fname;
	LPTSTR	 filepart;
	TCHAR	 fullname[MAX_PATH];
	RECT	 rc;

	if(! pView || ! m_parent) return;
	m_ignoreNotify = true;

	BinfoPane::SetView(pView);

	buffer = m_view->GetBuffer();
	fname  = buffer->GetName();

	// build the new tree hidden, cause visible
	// tv inserts make for real slow filling
	//
	GetClientRect(m_hwndTree, &rc);
	hWnd		= MakeNewTree();
	hOldWnd		= m_hwndTree;
	m_hwndTree	= hWnd;

	// if remote path, get remote listing [TODO]
	//
	if(! buffer->GetIsFTP())
	{
		// get the path of the view's buffer
		//
		GetFullPathName(fname, MAX_PATH, fullname, &filepart);
		if(filepart) *filepart = _T('\0');

		// if blank path, or nonsensical path, use cwd as path
		if(_tcslen(fullname) == 0 || BUtil::DirectoryExists(fullname) != errOK)
		{
			char cwd[MAX_PATH];
			
			if((getcwd(cwd, MAX_PATH)) != NULL)
			{
				CharToTChar(fullname, cwd);
				if(fullname[0])
					if(fullname[_tcslen(fullname) - 1] != _PTC_)
						_tcscat(fullname, _PTS_);
			}
		}
	}
	else
	{
		// get the path of the view's buffer, including hostname portion
		//
		GetFullPathName(fname, MAX_PATH, fullname, &filepart);
		if(filepart) *filepart = _T('\0');
	}

	// list dir into tree (returns false if no change)
	//
	if(SetPath(fullname))
	{
		TreeView_SelectItem(m_hwndTree, NULL);
		
		// finally show the new tree, kill the old tree
		ShowWindow(m_hwndTree, m_active ? SW_SHOW : SW_HIDE);
		DestroyWindow(hOldWnd);
	}
	else
	{
		// nothing added to new tree, leave old one intact
		//
		m_hwndTree = hOldWnd;
		DestroyWindow(hWnd);
	}
	m_ignoreNotify = false;
}


//**************************************************************************
void BfileInfo::Select(LPTVITEM pItem)
{
	TCHAR fullname[MAX_PATH];
	Bbuffer* buffer;

	if(m_ignoreNotify)					return;
	if(! m_view || ! m_editor)			return;
	if(! (pItem->mask & TVIF_TEXT))		return;

	if((_tcslen(m_path) + _tcslen(pItem->pszText)) >= MAX_PATH) return;

	buffer = m_view->GetBuffer();
	if(! buffer->GetIsFTP())
	{
		_tcscpy(fullname, m_path);
	}
	else
	{
		LPTSTR  pRemotePath;

		pRemotePath = (LPTSTR)buffer->GetName();
		_tcscpy(fullname, pRemotePath);
		pRemotePath = fullname + _tcslen(fullname) - 1;
		while(pRemotePath >= fullname)
		{
			if(*pRemotePath == _T('/') || *pRemotePath == _T('\\'))
				break;
			pRemotePath--;
		}
		if(*pRemotePath)
			*++pRemotePath = _T('\0');
	}

	if(pItem->lParam & 1)
	{
		if(! _tcscmp(pItem->pszText, _T("..")))
		{
			LPTSTR pp = fullname + _tcslen(fullname);

			if(pp > fullname && *(pp-1) == _PTC_)
				pp-= 2;

			while(pp >= fullname)
			{
				if(*--pp == _PTC_)
					break;
			}
			if(*pp == _PTC_)
				*++pp = _T('\0');
		}
		else
		{
			_tcscat(fullname, pItem->pszText);
		}
		// build the new tree hidden, cause visible
		// tv inserts make for real slow filling
		//
		HWND hOldWnd = m_hwndTree;
		HWND hWnd	 = MakeNewTree();

		m_hwndTree	 = hWnd;

		SetPath(fullname);

		TreeView_SelectItem(m_hwndTree, NULL);

		ShowWindow(m_hwndTree, SW_SHOW);
		DestroyWindow(hOldWnd);
	}
	else
	{
		_tcscat(fullname, pItem->pszText);

		m_view->PushParm((int)btAny, ptBufferType);
		m_view->PushParm((int)ltLF, ptTextLineTerm);
		m_view->PushParm((int)-1, ptTextEncoding);
		m_view->PushParm(fullname, _tcslen(fullname), ptString);
		m_view->Dispatch(NewBuffer);
	}
}

// STATIC
//**************************************************************************
ERRCODE BfileInfo::ListDirectory(HDIRLIST& hDir, LPCTSTR path)
{
	LPBDIRINFO pDirlist;
	TCHAR	   rpath[MAX_PATH*2];
	TCHAR      rpattern[MAX_PATH];
	int		   len, plen;

	pDirlist = new BDIRINFO;

	_tcsncpy(rpath, path, MAX_PATH-1);
	rpath[MAX_PATH-1] = _T('\0');
	len = plen = _tcslen(rpath);

	// extract path and pattern from path spec
	// (e.g. "a/*.c" becomes "a/ and *.c")
	//
	rpattern[0] = _T('\0');
	while(len > 0)
	{
		if(rpath[len-1] == _PTC_)
		{
			if(len < plen)
			{
				// dangling portion of path is file pattern
				_tcscpy(rpattern, rpath+len);
				rpath[len] = _T('\0');
				break;
			}
			else
			{
				// path ends in PTC, so no pattern spec
				_tcscpy(rpattern, _T("*"));
				break;
			}
		}
		len--;
	}
	if(len == 0)
	{
		// its all pattern
		_tcscpy(rpattern, rpath);
		rpath[0] = _T('\0');
	}
	len = _tcslen(rpath);;
	if(! rpattern[0])
		_tcscpy(rpattern, _T("*"));
	if(len > 0)
	{
		if(rpath[len-1] != _PTC_)
		{
			rpath[len++] = _PTC_; 
			rpath[len] = _T('\0');
		}
	}
	else
	{
		rpath[0] = _T('.');
		rpath[1] = _PTC_;
		rpath[2] = _T('\0');
	}
	_tcscpy(pDirlist->szBase, rpath);
#ifdef Windows
	_tcscpy(pDirlist->path, rpath);
	_tcscat(pDirlist->path, rpattern);
	pDirlist->hFind = NULL;
	pDirlist->first = true;
#else
	_tcscpy(pDirlist->pattern, rpattern);
	TCharToChar(pDirlist->path, rpath);
	if((pDirlist->pDir = opendir(pDirlist->path)) == NULL)
		return errFAILURE;
	pDirlist->first = false;
#endif
	hDir = (HDIRLIST)pDirlist;
	return errOK;
}

// STATIC
//**************************************************************************
ERRCODE BfileInfo::NextFile(HDIRLIST& hDir, LPCTSTR& lpName, bool& isDir, bool& isLink, bool& isReadOnly)
{
	LPBDIRINFO pDirlist;

	if(! (pDirlist = (LPBDIRINFO)hDir))
		return errBAD_PARAMETER;

#ifndef Windows
	do
	{
#endif
	if(pDirlist->first)
	{
#ifdef Windows
		if((pDirlist->hFind = FindFirstFile(pDirlist->path, &pDirlist->fdata)) == INVALID_HANDLE_VALUE)
#else
		if(pDirlist->pDir == NULL)
#endif
		{
			delete pDirlist;
			hDir = NULL;
			return errFAILURE;
		}
		pDirlist->first = false;
	}
#ifdef Windows
	else
#endif
	{
#ifdef Windows
		if(! FindNextFile(pDirlist->hFind, &pDirlist->fdata))
#else
		if((pDirlist->dp = readdir(pDirlist->pDir)) == NULL)
#endif
		{
#ifdef Windows
			FindClose(pDirlist->hFind);
#else
			closedir(pDirlist->pDir);
#endif
			delete pDirlist;
			hDir = NULL;
			return errFAILURE;
		}
	}
	_tcscpy(pDirlist->szName, pDirlist->szBase);
	lpName		= pDirlist->szName;
#ifdef Windows
	_tcscat(pDirlist->szName, pDirlist->fdata.cFileName);
	isDir		= (pDirlist->fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	isReadOnly	= (pDirlist->fdata.dwFileAttributes & FILE_ATTRIBUTE_READONLY)  != 0;
	isLink		= false;
#else
	CharToTChar(pDirlist->szName + _tcslen(pDirlist->szName), pDirlist->dp->d_name);
	isLink		= false;

	// if this file doesn't match our pattern ignore it 
	if(! BUtil::SimplePatternMatch(pDirlist->szName, pDirlist->pattern))
		continue;

	// if our pattern 
	char fp[MAX_PATH + 64];
	struct stat finfo;

	strcpy(fp, pDirlist->path);
	strcat(fp, pDirlist->dp->d_name);

	if(! stat(fp, &finfo))
	{
		int mode;
		
		isDir = ((mode = finfo.st_mode) & S_IFDIR) != 0;

		{
			if(finfo.st_uid != getuid())
			{
				mode = mode & 0077;
				mode |= ((mode & 0070) << 3);
			}
			if(! (mode & 0200))
			{
				isReadOnly = true;
			}
			else
			{
				isReadOnly = false;
			}
		}
	}
	break;
#endif
#ifndef Windows
	}
	while(1);
#endif
	return errOK;
}

//**************************************************************************
ERRCODE BfileInfo::EndListDirectory(HDIRLIST& hDir)
{
	LPBDIRINFO pDirlist;

	if(! (pDirlist = (LPBDIRINFO)hDir))
		return errBAD_PARAMETER;
	delete pDirlist;
	return errOK;
}


// STATIC
//**************************************************************************
LPTSTR BfileInfo::FilePart(LPTSTR path)
{
	LPTSTR pf = path;
	int    l;
	
	if(! path) return (LPTSTR)_T("<nil>");
	l = _tcslen(path);
	if(l == 0) return path;
	while(l > 0)
	{
		if(path[l-1] == _PTC_)
			return path + l;
		l--;
	}
	return path;
}

// STATIC
//**************************************************************************
bool BfileInfo::OnFtpFile(LPCTSTR path, WORD mode, LPVOID cookie)
{
	TVINSERTSTRUCT  tvItem;
	BfileInfo* pf = (BfileInfo*)cookie;

	if(! pf) return false;
	tvItem.hParent		= NULL;
	tvItem.hInsertAfter = TVI_LAST;

	tvItem.item.mask	= TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvItem.item.pszText = FilePart((LPTSTR)path);
	tvItem.item.lParam	= 0;
	
	if(! (mode & 0200))
	{
		tvItem.item.lParam |= 2;
	}
	if(mode & 0x8000)
	{
		tvItem.item.lParam |= 1;
		tvItem.item.iImage = 2;
	}
	else
		tvItem.item.iImage = (mode & 0200) ? 0 : 1;
	tvItem.item.iSelectedImage = tvItem.item.iImage;
	if(_tcscmp(path, _T(".")))
		TreeView_InsertItem(pf->m_hwndTree, &tvItem);
	return true;
}

//**************************************************************************
bool BfileInfo::SetPath(LPCTSTR path)
{
	HDIRLIST		hDir;
	LPCTSTR			lpFilename;
	Bbuffer*		buffer;
	bool			isDir, isLink, isReadOnly;
	ERRCODE			ec;
	TVINSERTSTRUCT  tvItem;
	TCHAR			spath[MAX_PATH + 4];
	int				len;

	if(! m_hwndTree)			return false;
	if(! _tcscmp(path, m_path)) return false;

	_tcscpy(m_path, path);
	len = _tcslen(m_path);
	if(len > 0)
	{
		if(m_path[len-1] != _PTC_)
		{
			m_path[len++] = _PTC_; 
			m_path[len] = _T('\0');
		}
	}
	else
	{
		m_path[0] = _T('.');
		m_path[1] = _PTC_;
		m_path[2] = _T('\0');
	}
	TreeView_DeleteAllItems(m_hwndTree);

	buffer = m_view->GetBuffer();
	if(! buffer->GetIsFTP())
	{
		HTREEITEM hFirstFileItem, hLastDirItem, hItem;
		HTREEITEM hPrevItem;
		TVITEM	  tvSort;

		_tcscpy(spath, m_path);
		_tcscat(spath, _T("*"));

		// initialize dir list
		ec = ListDirectory(hDir, spath);
		
		hLastDirItem = NULL;
		
		// list directory, looking only at directories
		//
		while(NextFile(hDir, lpFilename, isDir, isLink, isReadOnly) == errOK)
		{
			if(isDir)
			{
				tvItem.hParent		= NULL;
				tvItem.hInsertAfter = TVI_SORT;

				tvItem.item.mask	= TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
				tvItem.item.pszText = FilePart((LPTSTR)lpFilename);
				tvItem.item.lParam	= 0;
				
				if(isReadOnly)
				{
					tvItem.item.lParam |= 2;
				}
				tvItem.item.lParam |= 1;
				tvItem.item.iImage = 2;
				tvItem.item.iSelectedImage = tvItem.item.iImage;
				if(_tcscmp(FilePart((LPTSTR)lpFilename), _T(".")))
				{
					TreeView_InsertItem(m_hwndTree, &tvItem);
				}
			}
		}
		hItem = TreeView_GetRoot(m_hwndTree);
		hLastDirItem = NULL;
		while(hItem)
		{
			hLastDirItem = hItem;
			hItem = TreeView_GetNextItem(m_hwndTree, hLastDirItem, TVGN_NEXT);
		}

		// initialize dir list
		ec = ListDirectory(hDir, spath);

		// get last item in tree (if any) to start inserting files into
		//
		hFirstFileItem = NULL;
			
		// list directory, looking only at files
		//
		while(NextFile(hDir, lpFilename, isDir, isLink, isReadOnly) == errOK)
		{
			if(! isDir)
			{
				LPTSTR filePart;
				
				filePart  = FilePart((LPTSTR)lpFilename);
				
				// sort by name inside of extension
				//
				if(hFirstFileItem == NULL)
				{
					tvItem.hInsertAfter = TVI_LAST;
				}
				else
				{
					hPrevItem = hLastDirItem;
					hItem     = hFirstFileItem;
										
					do
					{
						tvSort.mask			= TVIF_TEXT;
						tvSort.hItem		= hItem; 
						tvSort.cchTextMax	= sizeof(spath)/sizeof(TCHAR);
						tvSort.pszText		= spath;
						
						if(TreeView_GetItem(m_hwndTree, &tvSort) == FALSE)
						{
							break;
						}
						if(_tcscmp(filePart, tvSort.pszText) <= 0)
						{
							//_tprintf(_T("bk %ls before %ls\n"), filePart, spath);
							
							if(hPrevItem)
							{
								tvItem.hInsertAfter = hPrevItem;
								if(hItem == hFirstFileItem)
									hFirstFileItem = NULL;
							}
							else
							{
								tvItem.hInsertAfter = hLastDirItem ? hLastDirItem : TVI_FIRST;
								hFirstFileItem = NULL;
							}
							break;
						}
						hPrevItem = hItem;
						hItem = TreeView_GetNextItem(m_hwndTree, hItem, TVGN_NEXT);
					}
					while(hItem);
					
					if(! hItem)
					{
						tvItem.hInsertAfter = TVI_LAST;
					}
				}
				tvItem.hParent		= NULL;

				tvItem.item.mask	= TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
				tvItem.item.pszText = filePart;
				tvItem.item.lParam	= 0;
				
				if(isReadOnly)
				{
					tvItem.item.iImage = 1;
					tvItem.item.lParam |= 2;
				}
				else
				{
					tvItem.item.iImage = 0;
				}
				tvItem.item.iSelectedImage = tvItem.item.iImage;
				hItem = TreeView_InsertItem(m_hwndTree, &tvItem);
				if(hFirstFileItem == NULL)
				{
					hFirstFileItem = hItem;
				}
			}
		}
	}
	else
	{
		// get remote listing
		//
		Bftp	ftp(BED_FTP_LOGLEV);
		LPCTSTR userName;
		LPCTSTR passWord;
		LPTSTR  pRemotePath;

		pRemotePath = (LPTSTR)buffer->GetName();
		if(_tcsstr(pRemotePath, _T("ftp:")))
			pRemotePath += 4;
		while(*pRemotePath == _T('/') || *pRemotePath == _T('\\'))
			pRemotePath++;
		_tcscpy(spath, pRemotePath);
		pRemotePath = spath + _tcslen(spath) - 1;
		while(pRemotePath >= spath)
		{
			if(*pRemotePath == _T('/') || *pRemotePath == _T('\\'))
				break;
			pRemotePath--;
		}
		*pRemotePath = _T('\0');

		do
		{
			if((ec = m_editor->GetFTPUserName(userName)) != errOK)
				break;
			if((ec = m_editor->GetFTPPassword(passWord)) != errOK)
				break;

			ec = ftp.ListRemoteDirectory(
								spath,
								OnFtpFile,
								(LPVOID)this,
								userName,
								passWord
								);
			switch(ec)
			{
			case errOBJECT_NOT_FOUND:
				ec = errOK; // ok on read
				break;
			case errPERMISSION:
				ec = m_editor->GetFTPAuthorization(true);
				if(ec != errOK) break;
				ec = errPERMISSION;
				break;
			case errOK:
				break;
			default:
				break;
			}
		}
		while(ec == errPERMISSION);
	}
	return true;
}

//STATIC
//**************************************************************************
ERRCODE BfileInfo::SimplifyFilePath(LPCTSTR path, LPTSTR respath)
{
	TCHAR		xpath[MAX_PATH];
	LPTSTR		px, pr;

	// combine a path and a pattern to form a new path
	// and a new pattern, or at least a path
	//
	if(! respath || ! path)
		return errBAD_PARAMETER;

	// [1] expand/contract new path portion into the tabpattern
	//
	if(path[0] == '~')
	{
		// path needs home dir replacement
		//
		_tcscpy(xpath, path+1);
		BUtil::GetHomePath(respath, MAX_PATH);
		if(_tcslen(respath) > 0)
		{
			if(respath[_tcslen(respath) - 1] == _PTC_)
			{
				if(xpath[0] == _PTC_)
					_tcscat(respath, xpath + 1);
				else
					_tcscat(respath, xpath);
			}
		}				
	}
	else
	{
		_tcscpy(respath, path);
	}
	if(
			_tcsstr(respath, _PTS_)
		|| _tcsstr(respath, _T(".."))
#ifdef _Windows
		|| (respath[0] && respath[1] == _T(':'))
#endif
	)
	{
		// collapse every "dirname/.." pattern to ""
		//
		while((pr = _tcsstr(respath, _T(".."))) != NULL)
		{
			// get portion of path left of ..
			//
			_tcsncpy(xpath, respath, (pr - respath));
			
			// remove one path dir
			//
			px = xpath + (pr - respath) - 1;
			if(px > xpath && px[0] == _PTC_)
				px--;
			else
				break;
			for(; px >= xpath; px--)
				if(*px == _PTC_)
					break;
			if(px < xpath)
				break;
			pr += 2;
			if(pr[0] != _PTC_)
				px++;
			_tcscpy(px, pr);
			_tcscpy(respath, xpath);
		}
	}
	return errOK;
}
