
#include "openfilex.h"

#ifdef Darwin
#define WORDSWAP(n) (((WORD)n >> 8)|(((WORD)n << 8) & 0xFF00))
#else
#define WORDSWAP(n) n
#endif



#define IDC_BF_PATH		0xbef8
#define IDC_BF_DIRTREE	0xbef9
#define IDC_BF_FILETREE	0xbefa
#define IDC_BF_FILENAME	0xbefb

static BOFBMH IconHeaders[] = 
{
	{ BOF_IW, BOF_IH, BOF_ID, _bsa_of_bmfile_icon },
	{ BOF_IW, BOF_IH, BOF_ID, _bsa_of_bmrofile_icon },
	{ BOF_IW, BOF_IH, BOF_ID, _bsa_of_bmdir_icon },
	{ BOF_IW, BOF_IH, BOF_ID, _bsa_of_bmopendir_icon }
};

static int g_bmswap = 0;

//**************************************************************************
static HBITMAP MakeBitmap(HINSTANCE hInst, int bm)
{
	PBOFBMH pbmh;
	HBITMAP hbm;
	HDC		hdc;

	BITMAPINFO bmh;

	if(bm < 1 || bm > 4) return NULL;

	pbmh = &IconHeaders[bm - 1];

	bmh.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
	bmh.bmiHeader.biWidth			= pbmh->w;
	bmh.bmiHeader.biHeight			= pbmh->h;
	bmh.bmiHeader.biPlanes			= 1;
	bmh.bmiHeader.biBitCount		= pbmh->d;
	bmh.bmiHeader.biCompression		= BI_RGB;
	bmh.bmiHeader.biSizeImage		= 0;
	bmh.bmiHeader.biXPelsPerMeter	= 0;
	bmh.bmiHeader.biYPelsPerMeter	= 0;
	bmh.bmiHeader.biClrImportant	= 0;
	bmh.bmiHeader.biClrUsed			= 0;

#ifdef Darwin
	if(! g_bmswap)
	{
		LPWORD pb;
		int n;
		
		for(pb = (LPWORD)pbmh->bits, n = pbmh->w * pbmh->h * pbmh->d / 8;
			n > 0;
			n-= 2
		)
		{
			*pb = WORDSWAP(*pb);
			pb++;
		}
		
	}
#endif
	hdc = GetDC(NULL); 
	hbm = CreateDIBitmap(hdc, &bmh.bmiHeader, CBM_INIT, pbmh->bits, &bmh, DIB_RGB_COLORS);
	ReleaseDC(NULL, hdc);
	return hbm;
}

//**************************************************************************
static bool BrowseSetPath(LPCTSTR path, LPFILEDIALPARMS lpParms)
{
	TCHAR			spath[MAX_PATH + 32];
	TCHAR			szIniPath[32];
	TCHAR			szDir[MAX_PATH*2+32];
	LPTSTR			filepart;
	TVINSERTSTRUCT  tvItem;
	HTREEITEM		hItem, firstmatch = NULL;
	ERRCODE			ec;
	LPCTSTR			lpFilename;
	BdirInfo		dir;
	bool			isDir, isLink, isReadOnly;
	int				len;

	if(! lpParms)						return false;

	if(! lpParms->ilist)
	{
		HBITMAP hBitmap;

		lpParms->ilist = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 3, 3);
		
		hBitmap = MakeBitmap(lpParms->hInst, FILE_ICON);
		ImageList_Add(lpParms->ilist, hBitmap, NULL);
		hBitmap = MakeBitmap(lpParms->hInst, FILERO_ICON);
		ImageList_Add(lpParms->ilist, hBitmap, NULL);
		hBitmap = MakeBitmap(lpParms->hInst, DIR_ICON);
		ImageList_Add(lpParms->ilist, hBitmap, NULL);
		hBitmap = MakeBitmap(lpParms->hInst, DIROPEN_ICON);
		ImageList_Add(lpParms->ilist, hBitmap, NULL);

		g_bmswap = 1; // no need to byteswap bitmaps again
		TreeView_SetImageList(lpParms->hwndDirs, lpParms->ilist, TVSIL_NORMAL);
		TreeView_SetImageList(lpParms->hwndFiles, lpParms->ilist, TVSIL_NORMAL);		
	}
	if(! path || ! path[0])
	{
		_tcscpy(szIniPath, _T("."));
		_tcscat(szIniPath, _PTS_);
		path = szIniPath;
	}
	GetFullPathName(path, MAX_PATH, szDir, &filepart);

	lpParms->bpattern[0] = _T('\0');

	if(filepart)
	{
		_tcscpy(lpParms->bpattern, filepart);
		*filepart = _T('\0');
	}

	// if blank path, use cwd as path
	if(_tcslen(szDir) == 0)
	{
		char cwd[MAX_PATH];
		
		if((getcwd(cwd, MAX_PATH)) != NULL)
		{
			CharToTChar(szDir, cwd);
			if(szDir[0])
				if(szDir[_tcslen(szDir) - 1] != _PTC_)
					_tcscat(szDir, _PTS_);
		}
	}
	if(_tcscmp(lpParms->bpath, szDir))
	{
		// actually changing path
		if(lpParms->filelist)
			Bflentry::DeleteFileEntry(lpParms->filelist);
		lpParms->filelist = NULL;
		if(lpParms->sublist)
			Bflentry::DeleteFileEntry(lpParms->sublist);
		lpParms->sublist = NULL;
	}
	_tcscpy(lpParms->bpath, szDir);
	len = _tcslen(lpParms->bpath);
	if(len > 0)
	{
		if(lpParms->bpath[len-1] != _PTC_)
		{
			lpParms->bpath[len++] = _PTC_; 
			lpParms->bpath[len] = _T('\0');
		}
	}
	else
	{
		lpParms->bpath[0] = _PTC_;
		lpParms->bpath[1] = _T('\0');
	}

	_tcscpy(spath, lpParms->bpath);
	_tcscat(spath, _T("*"));
	
	if(lpParms->hwndDirs)
	{
		ShowWindow(lpParms->hwndDirs, SW_HIDE);
		TreeView_DeleteAllItems(lpParms->hwndDirs);
	}
	if(lpParms->hwndFiles)
	{
		ShowWindow(lpParms->hwndFiles, SW_HIDE);
		TreeView_DeleteAllItems(lpParms->hwndFiles);
	}
	_tcscpy(szDir, lpParms->bpath);
	_tcscat(szDir, lpParms->bpattern);
	SetWindowText(lpParms->hwndPath, szDir);
	InvalidateRect(lpParms->hwndPath, NULL, TRUE);

	// initialize dir list
	ec = dir.ListDirectory(spath);
	
	// list directory
	//
	while(dir.NextFile(lpFilename, isDir, isLink, isReadOnly) == errOK)
	{
		hItem = NULL;

		tvItem.hParent		= NULL;
		tvItem.hInsertAfter = TVI_SORT;

		tvItem.item.mask	= TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvItem.item.pszText = BdirInfo::FilePart((LPTSTR)lpFilename);
		tvItem.item.lParam	= 0;

		tvItem.item.mask |= TVIF_STATE;
		tvItem.item.state		= 0;
		tvItem.item.stateMask	= TVIS_BOLD;

		if(lpParms->sublist)
		{
			if(Bflentry::FindFileEntry(tvItem.item.pszText, lpParms->sublist))
				tvItem.item.state = TVIS_BOLD;
		}
		if(isReadOnly)
		{
			tvItem.item.lParam |= 2;
		}
		if(isDir)
		{
			tvItem.item.lParam |= 1;
			tvItem.item.iImage = 2;
			tvItem.item.iSelectedImage = tvItem.item.iImage;
			if(_tcscmp(tvItem.item.pszText, _T(".")))
			{
				HWND hwndDest = lpParms->hwndDirs;

				if(lpParms->dironly)
				{
					if(_tcscmp(tvItem.item.pszText, _T("..")))
						hwndDest = lpParms->hwndFiles;
				}
				hItem = TreeView_InsertItem(hwndDest, &tvItem);
			}
		}
		else if(! lpParms->dironly && BUtil::SimplePatternMatch(lpFilename, lpParms->bpattern))
		{
			if(isReadOnly)
				tvItem.item.iImage = 1;
			else
				tvItem.item.iImage = 0;
			tvItem.item.iSelectedImage = tvItem.item.iImage;
			hItem = TreeView_InsertItem(lpParms->hwndFiles, &tvItem);
			if(hItem && tvItem.item.state & TVIS_BOLD && ! firstmatch)
				firstmatch = hItem;
		}
	}
	dir.EndListDirectory();
	TreeView_SelectItem(lpParms->hwndDirs, NULL);
	TreeView_SelectItem(lpParms->hwndFiles, firstmatch);
	ShowWindow(lpParms->hwndDirs, SW_SHOW);
	ShowWindow(lpParms->hwndFiles, SW_SHOW);
	return true;
}

//**************************************************************************
static bool SimplifyFilePath(LPCTSTR path, LPFILEDIALPARMS lpParm, bool& isdir, bool& exists)
{
	TCHAR		xpath[MAX_PATH];
	struct stat finfo;
	char		fb[MAX_PATH+2];
	LPTSTR		px, pr, ppat;
	int			len;
	bool		haspath = false;

	// combine a path and a pattern to form a new path
	// and a new pattern, or at least a path
	//
	isdir = false;


	// [0] first set path chars to local convention
	//
	for(ppat = (LPTSTR)path; *ppat; ppat++)
	{
		if(*ppat == _T('\\') || *ppat == _T('/')) 
		{
			*ppat = _PTC_;
		}
	}

	// [1] expand/contract new path portion into the tabpattern
	//
	if(path[0] == '~')
	{
		// path needs home dir replacement
		//
		_tcscpy(xpath, path+1);
		BUtil::GetHomePath(lpParm->tabpattern, MAX_PATH);
		if(_tcslen(lpParm->tabpattern) > 0)
		{
			if(lpParm->tabpattern[_tcslen(lpParm->tabpattern) - 1] == _PTC_)
			{
				if(xpath[0] == _PTC_)
					_tcscat(lpParm->tabpattern, xpath + 1);
				else
					_tcscat(lpParm->tabpattern, xpath);
			}
		}				
	}
	else
	{
		_tcscpy(lpParm->tabpattern, path);
	}
	if(
			_tcsstr(lpParm->tabpattern, _PTS_)
		|| _tcsstr(lpParm->tabpattern, _T(".."))
#ifdef _WIN32
		|| (lpParm->tabpattern[0] && lpParm->tabpattern[1] == _T(':'))
#endif
	)
	{
		// pattern contains a path or path component
		//
		haspath = true;
		if(
				(lpParm->tabpattern[0] == _PTC_)				// absolute path
			||	(lpParm->tabpattern[1] == _T(':'))				// abspath with drive
		)
		{
			// current pattern contains an absolute path and perhaps a partial pattern,
			//
			_tcscpy(lpParm->bpath, lpParm->tabpattern);
		}
		else
		{
			// current pattern has a relative path and perhaps a partial pattern
			//
			_tcscat(lpParm->bpath, lpParm->tabpattern);
		}
		// [2] - base path is set to full path, plus pattern
		//
		// collapse every "dirname/.." pattern to ""
		//
		while((pr = _tcsstr(lpParm->bpath, _T(".."))) != NULL)
		{
			// get portion of path left of ..
			//
			_tcsncpy(xpath, lpParm->bpath, (pr - lpParm->bpath));
			
			// remove one path dir
			//
			px = xpath + (pr - lpParm->bpath) - 1;
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
			_tcscpy(lpParm->bpath, xpath);
		}
		// [3] - split the path into path and pattern portions
		//
		for(
				ppat = lpParm->bpath + _tcslen(lpParm->bpath);
				ppat > lpParm->bpath;
				ppat--
		)
		{
			if(*ppat == _PTC_) 
			{
				break;
			}
			else if(*ppat == _T('.') && ppat > lpParm->bpath && ppat[-1] == _T('.'))
			{
				break;
			}
#ifdef _WIN32
			else if(*ppat == _T(':'))
			{
				break;
			}
#endif
		}
		_tcscpy(lpParm->tabpattern, ++ppat);
		*ppat = _T('\0');
	}
	// if no pattern portion, pattern was all path and no pattern
	//
	if(! lpParm->tabpattern[0])
	{
		isdir = true;
	}
	else
	{
		// if the current path, plus the pattern, is a directory
		// then move the pattern plus a ptc over to the listing path
		//
		_tcscpy(xpath, lpParm->bpath);
		len = _tcslen(xpath);
		if(len && (xpath[len-1] != _PTC_))
		{
			xpath[len++] = _PTC_;
			xpath[len] = _T('\0');
		}
		_tcscpy(xpath + len, lpParm->tabpattern);
		len = _tcslen(xpath);
#ifdef UNICODE
		BUtil::Utf8Encode(fb, xpath, false);
#else
		strcpy(fb, xpath);
#endif
		if(! stat(fb, &finfo))
		{
			if((finfo.st_mode & S_IFDIR) != 0)
			{
				_tcscpy(lpParm->bpath, xpath);
				_tcscat(lpParm->bpath, _PTS_);
				lpParm->tabpattern[0] = _T('\0');
				isdir = true;
			}
		}
	}
	return false;
}

//**************************************************************************
static BOOL CALLBACK OpenFilenameProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message == WM_GETDLGCODE)
	{
		return DLGC_WANTALLKEYS;
	}
	if(
		(message == WM_CHAR || message == WM_KEYDOWN)
	&& (wParam == 8))
	{
		volatile int aa = wParam;
		wParam = aa;
	}
	if(message == WM_KEYDOWN)
	{
		if(wParam == VK_ESCAPE)
			SendMessage(GetParent(hWnd), WM_CLOSE, 0, 0);
		else
			SendMessage(GetParent(hWnd), WM_COMMAND, (wParam == VK_TAB ? 0xdead1 : (wParam == VK_RETURN ? 0xdead3 : 0xdead2)), wParam);
	}
	if(message == WM_CHAR && (wParam == 13 || wParam == 10 || wParam == 27 || wParam == 9))
		return TRUE;

	return CallWindowProc((WNDPROC)GetClassLong(hWnd, GCL_WNDPROC), hWnd, message, wParam, lParam);
}

	static LPFILEDIALPARMS  lpParm = NULL;
//**************************************************************************
static BOOL CALLBACK BrowseFileWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	HWND	hWndParent;
	RECT	rc;
	RECT	rcme;

	TCHAR	xpath[MAX_PATH];

	LPNMHDR	nmhdr;
	TVITEM  tvitem;
	TCHAR	tmp[MAX_PATH];
	LPTSTR	fp;
	bool	isdir, exists;
	
	switch (message) 
	{
	case WM_INITDIALOG:

		lpParm = (LPFILEDIALPARMS)lParam;
		if(! lpParm) return FALSE;
	
		// title
		SetWindowText(hDlg, (LPTSTR)lpParm->pTitle);

		hWndParent = GetParent(hDlg);
		if(! hWndParent)
			hWndParent = GetDesktopWindow();
		GetClientRect(hWndParent, &rc);
		GetWindowRect(hDlg, &rcme);
		{
			int x = ((rc.right - rc.left) - (rcme.right - rcme.left)) / 2;
			int y = ((rc.bottom - rc.top) - (rcme.bottom - rcme.top)) / 2;
			int w = rcme.right - rcme.left;
			int h = rcme.bottom - rcme.top;

			if(x < 0) x = 0;
			if(y < 0) y = 0;

			MoveWindow(hDlg, x, y, w, h, FALSE);
		}
		lpParm->hwndPath	= GetDlgItem(hDlg, IDC_BF_PATH);
		lpParm->hwndDirs	= GetDlgItem(hDlg, IDC_BF_DIRTREE);
		lpParm->hwndFiles	= GetDlgItem(hDlg, IDC_BF_FILETREE);
		fp = NULL;
		GetFullPathName(lpParm->name, MAX_PATH, lpParm->bpath, &fp);
		if(fp)
			*fp = 0;
		else if(! lpParm->name[0])
			lpParm->bpath[0] = _T('\0');
		BrowseSetPath(lpParm->bpath, lpParm);
		lpParm->tabcnt = 0;
		if(! lpParm->dironly)
		{
			SetWindowLong(GetDlgItem(hDlg, IDC_BF_FILENAME), GWL_WNDPROC, (LONG)OpenFilenameProc);
			//SetFocus(GetDlgItem(hDlg, IDC_BF_FILENAME));
		}
		else
		{
			ShowWindow(GetDlgItem(hDlg, IDC_BF_FILENAME), SW_HIDE);
		}
		if(lpParm->messageHook)
		{
			lpParm->messageHook(hDlg, message, wParam, lParam);
		}
		//return  lpParm->dironly ? TRUE : FALSE;
		return TRUE;

	case WM_DESTROY:
		
		if(! lpParm) return FALSE;
		DestroyWindow(lpParm->hwndPath);
		DestroyWindow(lpParm->hwndDirs);
		DestroyWindow(lpParm->hwndFiles);
		lpParm->hwndPath  = NULL;
		lpParm->hwndDirs  = NULL;
		lpParm->hwndFiles = NULL;
		if(lpParm->ilist)
			ImageList_Destroy(lpParm->ilist);
		lpParm->ilist = NULL;
		if(lpParm->filelist)
			Bflentry::DeleteFileEntry(lpParm->filelist);
		lpParm->filelist = NULL;
		if(lpParm->sublist)
			Bflentry::DeleteFileEntry(lpParm->sublist);
		lpParm->sublist = NULL;
		if(lpParm->messageHook)
		{
			lpParm->messageHook(hDlg, message, wParam, lParam);
		}
		break;
		
	case WM_COMMAND:

		switch(wParam)
		{
		case 0xbeef1:
			if(lpParm->filelist)
				Bflentry::DeleteFileEntry(lpParm->filelist);
			lpParm->filelist = NULL;
			if(lpParm->sublist)
				Bflentry::DeleteFileEntry(lpParm->sublist);
			lpParm->sublist = NULL;
			BrowseSetPath(lpParm->bpath, lpParm);
			break;

		case IDOK:
			if(! lpParm->dironly)
			{
				GetDlgItemText(hDlg, IDC_BF_FILENAME, xpath, MAX_PATH);
				SimplifyFilePath(xpath, lpParm, isdir, exists);

				if(isdir)
				{
					lpParm->tabcnt = 0;
					SetDlgItemText(hDlg, IDC_BF_FILENAME, _T(""));
					BrowseSetPath(lpParm->bpath, lpParm);
					return 0;
				}
				_tcscpy(lpParm->name, lpParm->bpath);
				_tcscat(lpParm->name, lpParm->tabpattern);
			}
			else
			{
				_tcscpy(lpParm->name, lpParm->bpath);
			}
			EndDialog(hDlg, wParam);
			break;

		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;

		case 0xdead1:

			// TAB on editbox, so expand to closest fit or
			// popup possible matches
			{
				BdirInfo dir;
				ERRCODE  ec;
				LPCTSTR	 fname;
				bool	 islink, isro;

				if(lpParm->tabcnt == 0)
				{
					TCHAR prevbase[MAX_PATH];

					// remember starting base path
					//
					_tcscpy(prevbase, lpParm->bpath);

					// get pattern in filename box
					//
					GetDlgItemText(hDlg, IDC_BF_FILENAME, xpath, MAX_PATH - 2);
					
					// combine pattern with base path
					//
					SimplifyFilePath(xpath, lpParm, isdir, exists);

					// set new pattern
					//
					_tcscat(lpParm->tabpattern, _T("*"));

					// if base path changed during simplify, reset listing
					// of matching files
					//
					if(_tcscmp(prevbase, lpParm->bpath))
					{
						Bflentry::DeleteFileEntry(lpParm->filelist);
						lpParm->filelist = NULL;
						Bflentry::DeleteFileEntry(lpParm->sublist);
						lpParm->sublist = NULL;
						BrowseSetPath(lpParm->bpath, lpParm);

						if(isdir)
						{
							// path is only a dir, so reset pattern
							//
							SetDlgItemText(hDlg, IDC_BF_FILENAME, _T(""));
						}
					}
				}
				if(! lpParm->filelist && _tcscmp(lpParm->tabpattern, _T("*")))
				{
					ec = dir.ListDirectory(lpParm->bpath);

					while(ec == errOK)
					{
						ec = dir.NextFile(fname, isdir, islink, isro);

						if(ec == errOK)
						{
							Bflentry::AddFileEntry(new Bflentry(dir.FilePart((LPTSTR)fname), (isdir ? 8 : 0) | (islink ? 4 : 0) | (isro ? 1 : 0)), lpParm->filelist);
						}
					}
					dir.EndListDirectory();
				}
				if(lpParm->filelist)
				{
					Bflentry::DeleteFileEntry(lpParm->sublist);
					lpParm->sublist = NULL;

					Bflentry::PruneFileEntry(lpParm->filelist, lpParm->tabpattern, lpParm->sublist, lpParm->dironly);

					if(lpParm->sublist)
					{
						Bflentry* pe = Bflentry::GetFileEntryNumber(lpParm->sublist, lpParm->tabcnt);
						int       l  = _tcslen(pe->m_name);
						SetDlgItemText(hDlg, IDC_BF_FILENAME, pe->m_name);
						if(l > 0)
						{
							// use setsel to put cursor at end of text
							SendMessage(GetDlgItem(hDlg, IDC_BF_FILENAME), EM_SETSEL, l, l );
							// and then to remove selection
							SendMessage(GetDlgItem(hDlg, IDC_BF_FILENAME), EM_SETSEL, (unsigned)-1, 0 );
						}
						BrowseSetPath(lpParm->bpath, lpParm);
						lpParm->tabcnt++;
					}
					else
					{
						// there is no file like pattern to show, so at least remove
						// any path component from current pattern
						//
						for(fp = xpath + _tcslen(xpath); fp > xpath; fp--)
						{
							if(fp[-1] == _PTC_)
								break;
						}
						int       l  = _tcslen(fp);
						SetDlgItemText(hDlg, IDC_BF_FILENAME, fp);
						if(l > 0)
						{
							// use setsel to put cursor at end of text
							SendMessage(GetDlgItem(hDlg, IDC_BF_FILENAME), EM_SETSEL, l, l );
							// and then to remove selection
							SendMessage(GetDlgItem(hDlg, IDC_BF_FILENAME), EM_SETSEL, (unsigned)-1, 0 );
						}
						lpParm->tabcnt = 0;
					}
				}
			}
			break;

		case 0xdead2:

			// non-tab on eb, clear state
			#if 0
			if(lpParm->tabcnt > 0)
			{
				TCHAR newtext[MAX_PATH];

				GetDlgItemText(hDlg, IDC_BF_FILENAME, newtext, MAX_PATH);
				if(lParam == VK_DIVIDE || lParam == 0xdc /*VK_BACKSLASH*/)
				{
					// leave path chars appended to filename
					;
				}
				else
				{
					// remov
					int l = _tcslen(newtext) - 1;
					SetDlgItemText(hDlg, IDC_BF_FILENAME, newtext + l);
				}
			}
			#endif
			if(lpParm->filelist)
				Bflentry::DeleteFileEntry(lpParm->filelist);
			lpParm->filelist = NULL;
			if(lpParm->sublist)
				Bflentry::DeleteFileEntry(lpParm->sublist);
			lpParm->sublist = NULL;
			lpParm->tabcnt = 0;
			break;

		case 0xdead3:

			// enter on eb, select if valid file (name is in file list if open-existing, or
			// non-blank if open any
			//
			GetDlgItemText(hDlg, IDC_BF_FILENAME, lpParm->name, MAX_PATH);
			if(lpParm->mustexist)
			{
				;
			}
			SendMessage(hDlg, WM_COMMAND, IDOK, 0);
			break;

		default:
			if(lpParm->messageHook)
			{
				lpParm->messageHook(hDlg, message, wParam, lParam);
			}
			break;
		}
		break;

	case WM_NOTIFY:

		nmhdr = (LPNMHDR)lParam;
		if(! nmhdr) break;
		
		switch(nmhdr->code)
		{
		case NM_DBLCLK:

			tvitem.hItem		= TreeView_GetSelection(nmhdr->hwndFrom);
			if(! tvitem.hItem) break;
			tvitem.mask			= TVIF_TEXT | TVIF_PARAM;
			tvitem.cchTextMax	= MAX_PATH;
			tvitem.pszText		= tmp;
			TreeView_GetItem(nmhdr->hwndFrom, &tvitem);
			if(1)
			{
				if(nmhdr->hwndFrom == lpParm->hwndDirs)
				{
					if(! _tcscmp(tvitem.pszText, _T("..")))
					{
						LPTSTR pp = lpParm->bpath + _tcslen(lpParm->bpath);
						
						if(pp > lpParm->bpath && *(pp-1) == _PTC_)
							pp-= 2;
						
						while(pp >= lpParm->bpath)
						{
							if(*--pp == _PTC_)
								break;
						}
						if(*pp == _PTC_)
							*++pp = _T('\0');
					}
					else
					{
						// append text to path and redisplay
						_tcscat(lpParm->bpath, tvitem.pszText);
						_tcscat(lpParm->bpath, _PTS_);
					}
					if(lpParm->bpattern[0])
					{
						_tcscat(lpParm->bpath, lpParm->bpattern);
					}
					PostMessage(hDlg, WM_COMMAND, 0xbeef1, 0);
				}
				else
				{
					if(lpParm->dironly)
					{
						_tcscat(lpParm->bpath, tvitem.pszText);
						_tcscat(lpParm->bpath, _PTS_);
						PostMessage(hDlg, WM_COMMAND, 0xbeef1, 0);
					}
					else
					{
						// selected a file
						_tcscpy(lpParm->name, lpParm->bpath);
						_tcscat(lpParm->name, tvitem.pszText);
						EndDialog(hDlg, IDOK);
					}
				}
			}
			if(lpParm->messageHook)
			{
				lpParm->messageHook(hDlg, message, wParam, lParam);
			}
			break;		
		
		default:
			if(lpParm->messageHook)
			{
				lpParm->messageHook(hDlg, message, wParam, lParam);
			}
			break;
		}
		break;
	}
    return FALSE;
}

//**************************************************************************
static LPWORD lpwAlign ( LPWORD lpIn)
{
    ULONG ul;

    ul = (ULONG) lpIn;
    ul +=3;
    ul >>=2;
    ul <<=2;
    return (LPWORD) ul;
}

//**************************************************************************
static LRESULT ModalOpenFileDialog(HINSTANCE hinst, HWND hwndParent, DLGPROC proc, LPFILEDIALPARMS lpParms)
{
    HGLOBAL				hgbl;
    LPDLGTEMPLATE		lpdt;
    LPDLGITEMTEMPLATE	lpdit;
    LPWORD				lpw;
    LPCWSTR				butext;
    LRESULT				ret;
    int					i, nchar;

    hgbl = GlobalAlloc(GMEM_ZEROINIT, 2048);
    if (!hgbl)
        return -1;
 
    lpdt = (LPDLGTEMPLATE)GlobalLock(hgbl);
 
    //-----------------------
    // Define the dialog box.
	//
    lpdt->style = WS_POPUP | WS_SYSMENU | DS_MODALFRAME | DS_SETFONT | WS_CAPTION;
	lpdt->dwExtendedStyle = 0;
    lpdt->cdit	= 6;  // number of controls
    lpdt->x		= 0;
	lpdt->y		= 0;
    lpdt->cx	= 246;
	lpdt->cy	= 206;

    lpw = (LPWORD)((LPBYTE)lpdt + 18);
    *lpw++ = 0;             // no menu
    *lpw++ = 0;             // predefined dialog box class (by default)

    nchar = _tcslen(lpParms->pTitle);
	for(i = 0; i <= nchar; i++)
		*lpw++ = (WORD)lpParms->pTitle[i];

	butext = L"MS Sans Serif";
	*lpw++ = 8;
    nchar = wcslen(butext);
	for(i = 0; i <= nchar; i++)
		*lpw++ = (WORD)butext[i];

    //-----------------------
    // Define filename edit control.
    //
    lpw = lpwAlign (lpw);  // align DLGITEMTEMPLATE on DWORD boundary
    lpdit = (LPDLGITEMTEMPLATE) lpw;
    lpdit->x  = 8;
	lpdit->y  = 18;
    lpdit->cx = 132;
	lpdit->cy = 14;
    lpdit->id = IDC_BF_FILENAME; 
    lpdit->style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_AUTOHSCROLL | SS_LEFT;

    lpw = (LPWORD) (lpw + 9);
	*lpw++ = 0xFFFF;
	*lpw++ = 0x0081;       // editbox class
    lpw = lpwAlign (lpw);  // align creation data on DWORD boundary
    *lpw++ = 0;            // no creation data

    //-----------------------
    // Define Cancel button.
    //
    lpw = lpwAlign (lpw); // align DLGITEMTEMPLATE on DWORD boundary
    lpdit = (LPDLGITEMTEMPLATE) lpw;
    lpdit->x  = 198;
	lpdit->y  = 18;
    lpdit->cx = 42;
	lpdit->cy = 14;
    lpdit->id = IDCANCEL; 
    lpdit->style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON;

    lpw = (LPWORD) (lpw + 9);
	*lpw++ = 0xFFFF;
	*lpw++ = 0x0080;       // button class atom

	butext = L"Cancel";
    nchar = wcslen(butext);
	for(i = 0; i <= nchar; i++)
		*lpw++ = (WORD)butext[i];

    lpw = lpwAlign (lpw);  // align creation data on DWORD boundary
    *lpw++ = 0;            // no creation data

    //-----------------------
    // Define OK button.
	//
    lpw = lpwAlign (lpw);  // align DLGITEMTEMPLATE on DWORD boundary
    lpdit = (LPDLGITEMTEMPLATE) lpw;
    lpdit->x  = 148;
	lpdit->y  = 18;
    lpdit->cx = 42;
	lpdit->cy = 14;
    lpdit->id = IDOK;
    lpdit->style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON;

    lpw = (LPWORD) (lpw + 9);
	*lpw++ = 0xFFFF;
	*lpw++ = 0x0080;       // button class atom

	butext = L"OK";
    nchar = wcslen(butext);
	for(i = 0; i <= nchar; i++)
		*lpw++ = (WORD)butext[i];

    lpw = lpwAlign (lpw);  // align creation data on DWORD boundary
    *lpw++ = 0;            // no creation data

    //-----------------------
    // Define path text control.
    //
    lpw = lpwAlign (lpw);  // align DLGITEMTEMPLATE on DWORD boundary
    lpdit = (LPDLGITEMTEMPLATE) lpw;
    lpdit->x  = 8;
	lpdit->y  = 4;
    lpdit->cx = 232;
	lpdit->cy = 12;
    lpdit->id = IDC_BF_PATH; 
    lpdit->style = WS_CHILD | WS_VISIBLE | WS_BORDER | SS_LEFT;

    lpw = (LPWORD) (lpw + 9);
	*lpw++ = 0xFFFF;
	*lpw++ = 0x0082;       // static class atom
 
	butext = L"path";
    nchar = wcslen(butext);
	for(i = 0; i <= nchar; i++)
		*lpw++ = (WORD)butext[i];
    lpw = lpwAlign (lpw);  // align creation data on DWORD boundary
    *lpw++ = 0;            // no creation data

    //-----------------------
    // Define directory tree control
    //
    lpw = lpwAlign (lpw);  // align DLGITEMTEMPLATE on DWORD boundary
    lpdit = (LPDLGITEMTEMPLATE) lpw;
    lpdit->x  = 8;
	lpdit->y  = 36;
    lpdit->cx = 94;
	lpdit->cy = 164;
    lpdit->id = IDC_BF_DIRTREE; 
    lpdit->style = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | TVS_HASLINES | TVS_LINESATROOT | TVS_NOTOOLTIPS;
    lpw = (LPWORD) (lpw + 9);
	butext = L"SysTreeView32";
    nchar = wcslen(butext);
	for(i = 0; i <= nchar; i++)
		*lpw++ = (WORD)butext[i];
    lpw = lpwAlign (lpw);  // align creation data on DWORD boundary
    *lpw++ = 0;            // no creation data

    //-----------------------
    // Define file tree control
    //
    lpw = lpwAlign (lpw);  // align DLGITEMTEMPLATE on DWORD boundary
    lpdit = (LPDLGITEMTEMPLATE) lpw;
    lpdit->x  = 104;
	lpdit->y  = 36;
    lpdit->cx = 134;
	lpdit->cy = 164;
    lpdit->id = IDC_BF_FILETREE; 
    lpdit->style = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | TVS_HASLINES | TVS_LINESATROOT | TVS_NOTOOLTIPS;
    lpw = (LPWORD) (lpw + 9);
	butext = L"SysTreeView32";
    nchar = wcslen(butext);
	for(i = 0; i <= nchar; i++)
		*lpw++ = (WORD)butext[i];
    lpw = lpwAlign (lpw);  // align creation data on DWORD boundary
    *lpw++ = 0;            // no creation data

#ifdef Darwin
	// byteswap whole thing
	nchar = lpw - (LPWORD)lpdt;
	lpw = (LPWORD)lpdt;
	while(nchar-- > 0)
	{
		*lpw = WORDSWAP(*lpw);
		lpw++;
	}
#endif
	
    GlobalUnlock(hgbl); 
    ret = DialogBoxIndirectParam(hinst, lpdt, hwndParent, proc, (LPARAM)lpParms); 
	i   = GetLastError();
    GlobalFree(hgbl); 
    return ret; 
}

//**************************************************************************
int OpenFileDialog(
				   HINSTANCE	hInstance,
				   LPCTSTR		lpTitle,
				   bool			openexisting,
				   bool			openorsave,
				   bool			dironly,
				   bool			qpenabled,
				   LPTSTR		lpName,
				   int			nName,
				   HWND			hwndParent,
				   DLGPROC		msgHook
				   )
{
#ifdef xWIN32
	if(dironly)
	{
		LPMALLOC			pshMalloc;
		BROWSEINFO			bi;
		LPITEMIDLIST		dl;
		TCHAR				szDir[MAX_PATH+2];

		//static LPITEMIDLIST prevIDL = NULL;

		bi.hwndOwner		= hwndParent; 
		bi.pidlRoot			= NULL; //prevIDL; 
		bi.pszDisplayName	= szDir; 
		bi.lpszTitle		= (LPTSTR)lpTitle; 
	#ifdef BIF_EDITVOX
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_EDITBOX; 
	#else
		bi.ulFlags = 0;
	#endif
		bi.lpfn		= NULL; 
		bi.lParam	= 0; 
		bi.iImage	= 0; 
		
		dl = SHBrowseForFolder((LPBROWSEINFO)&bi);
		rc = IDCANCEL;
		if(dl)
		{
			if(SHGetPathFromIDList(dl, szDir))
			{
				//prevIDL	= dl;
				if(lpName && nName)
				{
					_tcsncpy(lpName, szDir, nName-1);
					lpName[nName-1] = _T('\0');
					rc = IDOK;
				}
				else
					rc = IDCANCEL;
			}
			SHGetMalloc(&pshMalloc);
			if(pshMalloc)
			{
				pshMalloc->Free(dl);
				pshMalloc->Release();
			}
		}
		return rc;
	}
	else
	{
		OPENFILENAME ofn;

		memset(&ofn, 0, sizeof(ofn));

		ofn.lStructSize		= sizeof(ofn);
		ofn.hwndOwner		= hwndParent;
		ofn.hInstance		= lpParms->hInst;
		ofn.lpstrFilter		= NULL;
		ofn.lpstrFile		= pfp->name;
		ofn.nMaxFile		= MAX_PATH;
		ofn.lpstrFileTitle	= NULL;
		ofn.nMaxFileTitle	= 0;
		ofn.lpstrTitle		= pfp->pTitle;
		ofn.Flags			= 0;
		ofn.lpstrDefExt		= NULL;
		ofn.lpstrInitialDir = pfp->bpath;
		ofn.lpstrCustomFilter	= NULL;
		ofn.nFilterIndex		= 0;

		if(GetOpenFileName(&ofn) != 0)
		{
			return IDOK;
		}
		return IDCANCEL;
	}
#else
	LPFILEDIALPARMS pfp;
	int				rc;


	pfp = new FILEDIALPARMS(hInstance, lpTitle, lpName, msgHook, openexisting, openorsave, dironly, qpenabled);

	rc = ModalOpenFileDialog(
							hInstance, 
							hwndParent,
							BrowseFileWndProc,
							pfp
						);
	if(rc == IDOK)
	{
		_tcsncpy(lpName, pfp->name, nName - 1);
		lpName[nName-1] = _T('\0');
	}
	delete pfp;
	return rc;
#endif
}

