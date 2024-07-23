
#include "bedx.h"

LPCTSTR		g_appname	= _T("BED");
int			g_vermaj	= 6;
int			g_vermin	= 0;

HICON		g_hIcon		= NULL;
HINSTANCE	g_hInstance = NULL;

Bed*		g_editor	= NULL;

extern LRESULT CALLBACK ApplWndProc			(HWND, UINT, WPARAM, LPARAM);
extern LRESULT CALLBACK ViewWndProc			(HWND, UINT, WPARAM, LPARAM);
extern LRESULT CALLBACK BedStatbarProc		(HWND, UINT, WPARAM, LPARAM);

static ERRCODE			RunSetupWizard		(Bpersist* pPersist);


#if defined(Windows)&&defined(_DEBUG)
#define DBG_MEM_USE 1
	#ifdef DBG_MEM_USE
	#include <crtdbg.h>
		_CrtMemState memstate;
	#endif
#endif


//**************************************************************************
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	HWND	hWnd;
	LPSTR	lpCmd, lpArg;
	TCHAR	szFile[MAX_PATH*2];
	TCHAR	szArg[MAX_PATH];
	LPCTSTR	lps;
	LPTSTR  lpe;
	RECT	rc;
	MSG		msg;
	int		wx, wy, ww, wh;
	
	bool	showComments 		= false;
	bool	showLineNums		= false;
	bool	showTabs			= false;
	bool	suppressTabPanes	= false;
	bool	openDefaultProject	= false;
	bool	viewallBuffers		= true;
	bool	badswitch			= true;
	bool	skiparg;
	
	BufType		 forceType		= btAny;
	TEXTENCODING forceEncoding	= (TEXTENCODING)-1;

	int scTextBkg	= COLOR_WINDOW;
	int scAppBkg	= COLOR_3DLIGHT;

	WNDCLASS wc;

#ifdef DBG_MEM_USE
	_CrtMemCheckpoint(&memstate);
#endif
	g_hInstance = hInstance; 

	// Ensure that the common control DLL is loaded. 
	InitCommonControls(); 
	
	g_hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_BED);

	// register class for text display window
	//
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= (WNDPROC)ViewWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon			= NULL;
	wc.hCursor			= LoadCursor(NULL, IDC_IBEAM);
	wc.hbrBackground	= (HBRUSH)(intptr_t)(scTextBkg+1);
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= _T("bedview");

	ATOM aViewClass = RegisterClass(&wc);

	
	// window class for status bar entry
	//
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= (WNDPROC)BedStatbarProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon			= NULL;
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)(intptr_t)(scAppBkg+1);
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= _T("bed-statbar");

	RegisterClass(&wc);

	// get size of screen to form default window
	//
	GetClientRect(GetDesktopWindow(), &rc);
	
	// get command line to find application name invoked
	//
	_tcsncpy(szFile, GetCommandLine(), MAX_PATH);
	szFile[MAX_PATH] = _T('\0');

	// the command line is "exename" argument list
	//
	lps = szFile;
	while(*lps == _T('\"'))
		lps++;
	if(*lps)
	{
		lpe = (LPTSTR)lps + 1;
		if(szFile[0] == _T('\"'))
		{
			while(*lpe && *lpe != _T('\"'))
				lpe++;
		}
		else
		{
			while(*lpe && *lpe != _T(' ') && *lpe != _T('\t'))
				lpe++;
		}
		*lpe = _T('\0');
	}
	if(! lps || ! *lps)
	{
		lps = _T("bed");
	}
	else
	{
		while(lpe > lps)
		{
			if(lpe[-1] == _T('\\') || lpe[-1] == _T('/'))
			{
				lps = lpe;
				break;
			}
			lpe--;
		}
		while(*lpe)
		{
			if(*lpe == _T('.'))
			{
				*lpe = _T('\0');
				break;
			}
			lpe++;
		}
	}
	// create the editor application object
	//
	g_editor = new Bed(hInstance, lps);

	// get window size from persistance
	//
	Bpersist* pPersist = g_editor->GetPersist();
	
	// default window placement
	ww = 8 * (rc.right - rc.left) / 12;
	wh = 9 * (rc.bottom - rc.top) / 10;
	wy = ((rc.bottom - rc.top) - wh) / 2;
	wx = 30;

	bool isSetup = false;
	ERRCODE ec;

	ec = pPersist->GetNvBool(_T("Setup"), isSetup, false);
	
	if(! isSetup || ec != errOK)
	{
		if(RunSetupWizard(pPersist) == errOK)
			pPersist->SetNvBool(_T("Setup"), true);
	}
	if(pPersist)
	{
		pPersist->GetNvInt(_T("Appl X"), wx, wx);
		pPersist->GetNvInt(_T("Appl Y"), wy, wy);
		pPersist->GetNvInt(_T("Appl W"), ww, ww);
		pPersist->GetNvInt(_T("Appl H"), wh, wh);
	}
	// check for degenerate cases like window off the screen
	if ((wx > rc.right) || ((wx + ww) < rc.left))
	{
		wx = (rc.right - rc.left) / 2;
	}
	if ((wy > rc.bottom) || ((wy + wh) < rc.top))
	{
		wy = (rc.bottom - rc.top) / 2;
	}
	if (ww < 10)
		ww = 10;
	if (wh < 10)
		wh = 10;

	ec = g_editor->Init(hInstance, MAKEINTRESOURCE(IDR_BEDMENU), MAKEINTRESOURCE(IDI_BED), MAKEINTRESOURCE(IDI_BED));
	if(ec != errOK)
	{
		int err = GetLastError();
		return FALSE;
	}
	ec = g_editor->Open(_T("bed 6.0"), wx, wy, ww, wh);
	if(ec != errOK)
	{
		int err = GetLastError();
		return FALSE;
	}
	hWnd = g_editor->GetWindow();
	
	suppressTabPanes = (strstr(lpCmdLine, "-s") != NULL);
	viewallBuffers   = (strstr(lpCmdLine, "-1") == NULL);

	// first see if the whole commandline consists of one filename
	// to take care of cases where another program (or OS shell)
	// is calling bed with a filepath that contains spaces and the
	// line isn't enquoted
	//
	if(strlen(lpCmdLine) < MAX_PATH && strstr(lpCmdLine, " "))
	{
#ifdef UNICODE
		BUtil::Utf8Decode(szFile, lpCmdLine);
#else
		strcpy(szFile, lpCmdLine);
#endif
		if(BUtil::FileExists(szFile) == errOK)
		{
			g_editor->EditBuffer(szFile, true, btAny);
			lpCmdLine = NULL;
		}
	}

	// load buffers from command line
	//
	for(lpCmd = lpCmdLine, badswitch = false; lpCmd && *lpCmd;)
	{
		while(*lpCmd == ' ' || *lpCmd == '\t')
			lpCmd++;

		// not one file, open by commandline
		//
		if(*lpCmd == '-')
		{
			LPSTR lpSwitch;

			for(lpCmd++, skiparg = false; *lpCmd != ' ' && *lpCmd != '\t' && *lpCmd != '\0'; lpCmd++)
			{
				lpSwitch = lpCmd;
				
				for(lpArg = lpSwitch; *lpArg != ' ' && *lpArg != '\t' && *lpArg != '\0';)
					lpArg++;
				for(; *lpArg == ' ' || *lpArg == '\t';)
					lpArg++;
				if(strlen(lpArg) < MAX_PATH)
					CharToTChar(szArg, lpArg);
				else
					_tcscpy(szArg, _T("cmd line too long"));
				
				switch(lpSwitch[0])
				{
				case 'r':
					skiparg = true;
					g_editor->EditBuffer(szArg, true, btTelnet);
					break;
	
				case 'h':
					skiparg = true;
					g_editor->EditBuffer(szArg, true, btSSH);
					break;
	
				case 't':
					forceType = btText;
					break;
	
				case 'x':
					forceType = btRaw;
					break;
	
				case 'c':
					forceType = btTerm;
					break;
	
				case 'I':
					showComments = true;
					break;
	
				case 'B':
					showTabs = true;
					break;
					
				case 'N':
				case 'n':
					showLineNums = true;
					break;
					
				case 'L':
				case 'l':
					g_editor->EditBuffer(NULL, true, btShell);
					break;
	
				case 's':
					suppressTabPanes = true;
					break;
					
				case 'p':
					openDefaultProject = true;
					g_editor->SetProjectRestore(true);
					break;
	
				default:
					badswitch = true;
					break;
				}
			}
			if(skiparg)
			{
				// ate up a string after current batch of chars
				// so advance past it
				//
				for(lpCmd = lpArg; *lpCmd != ' ' && *lpCmd != '\t' && *lpCmd != '\0';)
					lpCmd++;
			}
		}
		else
		{
			LPSTR  lpFile;
			LPTSTR lpExt;
			char   xc;

			if(*lpCmd == '\"')
			{
				for(lpFile = ++lpCmd; *lpCmd != '\"' && *lpCmd != '\0';)
					lpCmd++;
			}
			else
			{
				for(lpFile = lpCmd; *lpCmd != ' ' && *lpCmd != '\t' && *lpCmd != '\0';)
					lpCmd++;
			}
			xc = *lpCmd;
			*lpCmd = '\0';

			CharToTChar(szFile, lpFile);

			*lpCmd = xc;
			if(xc != '\0')
				lpCmd++;

			for(lpExt = szFile + _tcslen(szFile) - 1; lpExt >= szFile; lpExt--)
				if(*lpExt == _T('.'))
					break;
			if(lpExt >= szFile)
				lpExt++;
			else
				lpExt = NULL;

			// check for project file
			if(g_editor->GetProject() == NULL && forceType == btAny && lpExt && ! _tcscmp(lpExt, _T("bpf")))
			{
				ec = g_editor->OpenProject(szFile, g_editor->GetProjectRestore() || openDefaultProject || *lpCmd == _T('\0'));
				if(ec == errOK) continue;
			}
#if Windows
			if(_tcsstr(szFile, _T("*")) != NULL || _tcsstr(szFile, _T("?")) != NULL)
			{
				HDIRLIST hDir;
				LPCTSTR  lpFilename;
				bool	 isDir, isLink, isReadOnly;

				BfileInfo::ListDirectory(hDir, szFile);
				while((ec = BfileInfo::NextFile(hDir, lpFilename, isDir, isLink, isReadOnly)) == errOK)
				{
					if(! isDir)
					{
						g_editor->EditBuffer(lpFilename, false, forceType, forceEncoding);
					}
				}
			}
			else
#endif
			{
				g_editor->EditBuffer(szFile, false, forceType, forceEncoding);
			}
		}
	}
	if(badswitch)
	{
#ifndef _Windows
		fprintf(stderr, "Use: bed -cBhIlLprstx [filename]\n"
				"        -r  telnet, filename specifies [hostname | ipaddress][:port]\n"
				"        -t  force view as plain text\n"
				"        -x  force hex view\n"
				"        -c  open serial comm port, filename is 0,1,2 (com1,2,3 or dev/ttyS0,1,2)\n"
				"        -n  show line numbers\n"
				"        -B  show tab spaces\n"
				"        -I  show comment lines\n"
				"        -L  default shell\n"
				"        -l  shell, filename specifies program to run in shell view\n"
				"        -s  suppress all other tab panes, just show view\n"				
				"        -p  open project file\n\n"
			);
#endif
		return 1;
	}
	if(g_editor->GetBuffers())
	{
		BbufList* pbe = g_editor->GetBuffers();

		// buffers specified, so make a view for each one
		//
		while(pbe)
		{
			g_editor->EditBuffer(*pbe);
			pbe = pbe->GetNext(pbe);
		}
	}
	else
	{
		// if no buffers specified, open an untitled text buffer
		//
		g_editor->EditBuffer((LPCTSTR)NULL, true, forceType);
	}
	// reset all the windows now that views are here
	//
	g_editor->SetupWindows(suppressTabPanes);

	// add each view created to the main edit panel (setupwindows adds the first view)
	//
	if(viewallBuffers && g_editor->GetEditPanel())
	{
		Bview*		pv;
		BappPane*	pa;

		for(pv = g_editor->GetViews(); pv; pv = pv->GetNext(pv))
		{
			if(! pv->GetParentPanel())
			{
				g_editor->GetEditPanel()->AddPane(pv);
			}
			// apply cmdline overrides only if set
			//
			if(showLineNums)
				pv->SetShowLineNums(showLineNums);
			if(showComments)
				pv->SetShowComments(showComments);
			if(showTabs)
				pv->SetShowTabs(showTabs);
		}
		for(pa = g_editor->GetEditPanel()->GetPanes(); pa; pa = pa->GetNext())
			pa->FitToPanel();
	}
	if(g_editor->GetCurrentView())
	{
		g_editor->UpdateInfoPanes(g_editor->GetCurrentView());
		if(g_editor->GetCurrentView()->GetBuffer())
			g_editor->SetBufferStatus(g_editor->GetCurrentView()->GetBuffer());
	}
	// if "always" run pdb, that means open project on program load, not 
	// project load, so open a default project now (quietly) that there
	// is most likely a view to get search paths from
	//
	if(g_editor->GetProject() == NULL && (g_editor->GetPDBenable() == pdbAlways || openDefaultProject))
	{
		// if project was restored, it would add buffers perhaps as
		// well as those perhaps on the cmd line
		//
		Bproject::OpenDefaultProject(NULL, g_editor, g_editor->GetProjectRestore(), false);
	}
	while(GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	delete g_editor;
#ifdef DBG_MEM_USE
	_CrtMemDumpAllObjectsSince(&memstate);
#endif
	return 0;
}

//***********************************************************************
BOOL CALLBACK AboutBed(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:

		return TRUE;

	case WM_COMMAND:

		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
    return FALSE;
}

#define ID_RCNT_FILES 0x3125
#define ID_RCNT_PROJECTS 0x3225

//**************************************************************************
LRESULT Bed::OnMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC			hdc;
	Bed*		pBed;
	Bview*		pView;
	HPEN		hpen;
	HPEN		hpenold;
	RECT		rc;
	bool		bShowit = true;
	HMENU		hMenu;

	pBed = (Bed*)GetWindowLong(hWnd, GWL_USERDATA);

	switch (message) 
	{
	case WM_CREATE:

		{ 
			LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;

			SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)pcs->lpCreateParams);
		}
		break;

	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);

		GetClientRect(hWnd, &rc);

		if(pBed)
		{
			// br outside
			hpen	= CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNHIGHLIGHT));
			hpenold = (HPEN)SelectObject(hdc, hpen);
			MoveToEx(hdc, rc.left, rc.bottom, NULL);
			LineTo(hdc, rc.right, rc.bottom);
			LineTo(hdc, rc.right, rc.top);
			SelectObject(hdc, hpenold);	
			DeleteObject(hpen);
			
			// br inside
			hpen	= CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DLIGHT));
			hpenold = (HPEN)SelectObject(hdc, hpen);
			MoveToEx(hdc, rc.left + 1, rc.bottom - 1, NULL);
			LineTo(hdc, rc.right - 1, rc.bottom - 1);
			LineTo(hdc, rc.right - 1, rc.top);
			SelectObject(hdc, hpenold);	
			DeleteObject(hpen);

			// ul outside
			hpen	= CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNSHADOW));
			hpenold = (HPEN)SelectObject(hdc, hpen);
			MoveToEx(hdc, rc.left, rc.top, NULL);
			LineTo(hdc, rc.right, rc.top);
			SelectObject(hdc, hpenold);	
			DeleteObject(hpen);

			// ul inside
			hpen	= CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DDKSHADOW));
			hpenold = (HPEN)SelectObject(hdc, hpen);
			MoveToEx(hdc, rc.left, rc.top + 1, NULL);
			LineTo(hdc, rc.right, rc.top + 1);
			SelectObject(hdc, hpenold);	
			DeleteObject(hpen);
		}
		EndPaint(hWnd, &ps);
		break;

	case WM_SIZE:
	case WM_MOVE:

		if(wParam == SIZE_MINIMIZED)
			return FALSE;

		if(pBed)
		{
			RECT		rc;
			Bpersist*	pPersist;

			GetClientRect(hWnd, &rc);

			if(rc.bottom - rc.top < 32 || rc.right - rc.left < 32)
				return FALSE;

			GetWindowRect(hWnd, &rc);

			if((pPersist = pBed->GetPersist()) != NULL)
			{
				int w, h;
				bool forceprimary = false;

				w = rc.right - rc.left;
				h = rc.bottom - rc.top;

				if(w < 32) w = 32;
				if(h < 32) h = 32;
		
				if(forceprimary)
				{
					// this opens the window in the primary screen
					// regardless of where it closed
					//
					if(rc.top  < 0) { rc.top  = 0; rc.bottom = h; }
					if(rc.left < 0) { rc.left = 0; rc.right = w;  }
				}
				if((rc.right  - rc.left) < 32) rc.right  = rc.left + 32;
				if((rc.bottom - rc.top)  < 32) rc.bottom = rc.top  + 32;
				pPersist->SetNvInt(_T("Appl X"), rc.left);
				pPersist->SetNvInt(_T("Appl Y"), rc.top);
				pPersist->SetNvInt(_T("Appl W"), w);
				pPersist->SetNvInt(_T("Appl H"), h);

				// do the panel processing for wm_size
				BappPanel::OnMessage(hWnd, message, wParam, lParam);

				// then make sure the statusbar is fitted
				pBed->FitStatusWindows();
			}
		}
		break;

	case WM_SETFOCUS: // set focus on our window, pass focus to view

		pBed->SetFocus(TRUE);
		break;

	case WM_KILLFOCUS: // losing focus

		pBed->SetFocus(FALSE);
		break;

	case WM_CLOSE:

		// save sizes of all panels
		PersistPanel(this);

		if(pBed->Close())
			return DefWindowProc(hWnd, message, wParam, lParam);
		else
			return 0;

	case WM_DESTROY:

		PostQuitMessage(0);
		break;

	case WM_INITMENU:
		
		/* build button prompts for default project
		hMenu = (HMENU)wParam;
		if(! pBed) pBed = g_editor;
		if(hMenu && pBed)
		{
			EnableMenuItem(hMenu, 3, MF_BYPOSITION | (pBed->GetProject() ? MF_ENABLED : MF_GRAYED));
			EnableMenuItem(hMenu, 4, MF_BYPOSITION | (pBed->GetProject() ? MF_ENABLED : MF_GRAYED));
		}
		*/
		break;
		
	case WM_INITMENUPOPUP:

		hMenu = (HMENU)wParam;
		if(hMenu && pBed)
		{
			if(LOWORD(lParam) == 0)
			{
				TCHAR menuName[MAX_PATH + 8];
				TCHAR fileName[MAX_PATH + 8];
				ERRCODE ec;
				bool   hasSCCS = pBed && (pBed->GetSCCS() != NULL);
				bool   hasProj = pBed && (pBed->GetProject() != NULL);
				int i;

				// file menu
			
				// enable close file buttons
				//
				EnableMenuItem(hMenu, ID_FILE_CLOSE        , MF_BYCOMMAND | (pBed->GetCurrentView() ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem(hMenu, ID_FILE_CLOSE_PROJECT, MF_BYCOMMAND | (hasProj ? MF_ENABLED : MF_GRAYED));
				
				EnableMenuItem(hMenu, ID_FILE_CHECKOUT, MF_BYCOMMAND | (hasSCCS ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem(hMenu, ID_FILE_CHECKIN,  MF_BYCOMMAND | (hasSCCS ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem(hMenu, ID_FILE_REVERT,   MF_BYCOMMAND | (hasSCCS ? MF_ENABLED : MF_GRAYED));

				// populate recent lists
				//
				HMENU hRecent = GetSubMenu(hMenu, 19);

				if(hRecent)
				{
					TCHAR  shortName[MAX_PATH];
					LPTSTR pName;
					
					for(i = 1; i < 64; i++)
						DeleteMenu(hRecent, 1, MF_BYPOSITION);
					for(i = 1; i <= 64; i++)
					{
						ec = pBed->GetRecentFile(i, fileName, MAX_PATH);
						if(ec != errOK || fileName[0] == _T('\0'))
							break;
						GetFullPathName(fileName, MAX_PATH, shortName, &pName);
						_sntprintf(menuName, MAX_PATH + 8,  _T("&%d " _Pfs_ ""), i, pName);
						AppendMenu(hRecent, MF_STRING, ID_RCNT_FILES + i, menuName);

					}
				}
				hRecent = GetSubMenu(hMenu, 20);

				if(hRecent)
				{
					LPTSTR pName;
					
					for(i = 1; i < 64; i++)
						DeleteMenu(hRecent, 1, MF_BYPOSITION);
					for(i = 1; i <= 64; i++)
					{
						ec = pBed->GetRecentProject(i, fileName, MAX_PATH);
						if(ec != errOK || fileName[0] == _T('\0'))
							break;
						pName = fileName;
						_sntprintf(menuName, MAX_PATH + 8,  _T("&%d " _Pfs_ ""), i, pName);
						AppendMenu(hRecent, MF_STRING, ID_RCNT_PROJECTS + i, menuName);
					}
				}
			}
			else if(LOWORD(lParam) == 1)
			{
				LPCTSTR	   szMenuText;
				BgrepInfo* pInfo;
				bool	   grepactive = false;

				pInfo = (BgrepInfo*)pBed->GetPane(_T("Info"), _T("Find in Files 2"));
				if(pInfo) grepactive |= ! pInfo->IsDone();

				pInfo = (BgrepInfo*)pBed->GetPane(_T("Info"), _T("Find in Files"));
				if(pInfo) grepactive |= ! pInfo->IsDone();

				// edit menu

				// set text of find-in-files properly
				//
				if(grepactive)
					szMenuText = _T("Stop Find in Files");
				else
					szMenuText = _T("Find in Files...");

				ModifyMenu(hMenu, ID_EDIT_FINDINFILE, MF_BYCOMMAND | MF_STRING, 
					ID_EDIT_FINDINFILE, szMenuText);

				pView = pBed ? pBed->GetCurrentView() : NULL;
				
				if(pView)
				{
					if(pView->GetBuffer())
					{
						if(pView->GetBuffer()->GetHasCR())
							szMenuText = _T("Drop CRs from Line-ends");
						else
							szMenuText = _T("Add CRs to Line-ends");

						ModifyMenu(hMenu, ID_EDIT_ADVANCED_DROPCR, MF_BYCOMMAND | MF_STRING, 
							ID_EDIT_ADVANCED_DROPCR, szMenuText);

						EnableMenuItem(hMenu, ID_EDIT_ADVANCED_MAKEWRITABLE, 
								MF_BYCOMMAND | (pView->GetBuffer()->GetReadOnly() ? MF_ENABLED : MF_GRAYED));
					}
					else
					{
						EnableMenuItem(hMenu, ID_EDIT_ADVANCED_MAKEWRITABLE,  MF_BYCOMMAND | MF_GRAYED);
						EnableMenuItem(hMenu, ID_EDIT_ADVANCED_DROPCR,  MF_BYCOMMAND | MF_GRAYED);
					}
					// setup checks for 
					//
					CheckMenuItem(
									hMenu,
									ID_EDIT_CASE_SENSITIVE,
									pView->GetCaseSensi() ? MF_CHECKED : MF_UNCHECKED
								);
					CheckMenuItem(
									hMenu,
									ID_EDIT_OVERSTRIKE,
									pView->GetOverstrike() ? MF_CHECKED : MF_UNCHECKED
								);
					CheckMenuItem(
									hMenu,
									ID_EDIT_TRIMWHITESPACE,
									pView->GetTrimOnWrite() ? MF_CHECKED : MF_UNCHECKED
								);
					CheckMenuItem(
									hMenu,
									ID_EDIT_ADVANCED_TABSASSPACES,
									pView->GetTabsAsSpaces() ? MF_CHECKED : MF_UNCHECKED
								);
					CheckMenuItem(
									hMenu,
									ID_EDIT_ADVANCED_AUTOTABDETECT,
									pView->GetAutoTabDetect() ? MF_CHECKED : MF_UNCHECKED
								);
				}
			}
			else if(LOWORD(lParam) == 2)
			{
				bool isRaw, isHex;

				// view menu

				pView = pBed->GetCurrentView();

				// setup checks for view windows
				//
				CheckMenuItem(
								hMenu,
								ID_VIEW_WINDOWS_PICKER,
								pBed->GetPane(_T("Picker"), _T("Picker")) ? MF_CHECKED : MF_UNCHECKED
							);
				CheckMenuItem(
								hMenu,
								ID_VIEW_WINDOWS_CALCULATOR,
								pBed->GetPane(_T("Picker"), _T("bcalc")) ? MF_CHECKED : MF_UNCHECKED
							);
				CheckMenuItem(
								hMenu,
								ID_VIEW_WINDOWS_FUNCTIONS,
								pBed->GetPanel(_T("Functions")) ? MF_CHECKED : MF_UNCHECKED
							);
				CheckMenuItem(
								hMenu,
								ID_VIEW_WINDOWS_FILES,
								pBed->GetPanel(_T("Files")) ? MF_CHECKED : MF_UNCHECKED
							);
				CheckMenuItem(
								hMenu,
								ID_VIEW_WINDOWS_INFO,
								pBed->GetPanel(_T("Info")) ? MF_CHECKED : MF_UNCHECKED
							);
				CheckMenuItem(
								hMenu,
								ID_VIEW_SHOWLINENUMS,
								pView->GetShowLineNums() ? MF_CHECKED : MF_UNCHECKED
							);
				CheckMenuItem(
								hMenu,
								ID_VIEW_SHOWWHITESPACE,
								pView->GetShowTabs() ? MF_CHECKED : MF_UNCHECKED
							);

				isRaw = pView ? (pView->GetBuffer() && pView->GetBuffer()->GetRaw()) : false;
				isHex = pView ? (0 == _tcscmp(pView->GetTypeName(), _T("Binary"))) : false;
				EnableMenuItem(hMenu, ID_VIEW_VIEWASHEX, MF_BYCOMMAND | ((isRaw || isHex) ? MF_GRAYED : MF_ENABLED));
				EnableMenuItem(hMenu, ID_VIEW_VIEWRADIX, MF_BYCOMMAND | ((isRaw || isHex) ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem(hMenu, ID_VIEW_VIEWFORMAT, MF_BYCOMMAND | ((isRaw || isHex) ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem(hMenu, ID_VIEW_VIEWCOLUMNS, MF_BYCOMMAND | (isRaw ? MF_ENABLED : MF_GRAYED));
			}
			else if(LOWORD(lParam) == 3)
			{
				// tools menu
				bool   hasProj	= pBed->GetProject() != NULL;
				bool   hasSCCS	= pBed->GetSCCS() != NULL;
				bool   isTerm	= FALSE;
				bool   isSerial = FALSE;

				PDBENABLE pdbe = pBed->GetPDBenable();

				if((pView = pBed->GetCurrentView()) != NULL)
				{
					if(! _tcscmp(pView->GetTypeName(), _T("RS232")))
						isTerm = isSerial = true;
					if(! _tcscmp(pView->GetTypeName(), _T("Shell")))
						isTerm = true;
				}

				EnableMenuItem(hMenu, ID_TOOLS_PROJECT_EDIT, MF_BYCOMMAND | (hasProj ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem(hMenu, ID_TOOLS_PROJECT_EXPORTMAKE, MF_BYCOMMAND | (hasProj ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem(hMenu, ID_TOOLS_SOURCECONTROL_ENABLE, MF_BYCOMMAND | (hasSCCS ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem(hMenu, ID_TOOLS_SOURCECONTROL_EDITSETTINGS, MF_BYCOMMAND | (hasSCCS ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem(hMenu, ID_TOOLS_PROJECT_EXPORTMAKE, MF_BYCOMMAND | (hasProj ? MF_ENABLED : MF_GRAYED));

				EnableMenuItem(hMenu, ID_TOOLS_CLEAR, MF_BYCOMMAND | (isTerm ? MF_ENABLED : MF_GRAYED));

				CheckMenuItem(hMenu, ID_TOOLS_PROGRAMDATABASE_ENABLE_ALWAYS,		pdbe == pdbAlways ? MF_CHECKED : MF_UNCHECKED);
				CheckMenuItem(hMenu, ID_TOOLS_PROGRAMDATABASE_ENABLE_ONPROJECTLOAD, pdbe == pdbOnLoad ? MF_CHECKED : MF_UNCHECKED);
				CheckMenuItem(hMenu, ID_TOOLS_PROGRAMDATABASE_ENABLE_NEVER,			pdbe == pdbNever  ? MF_CHECKED : MF_UNCHECKED);

				EnableMenuItem(hMenu, ID_TOOLS_TERMINAL_SETTINGS, MF_BYCOMMAND | (isSerial ? MF_ENABLED : MF_GRAYED));
			}
			else if(LOWORD(lParam) == 5)
			{
				// debug menu
				bool   hasDbg = (pBed->GetDebugger() != NULL);

				EnableMenuItem(hMenu, ID_DEBUG_STARTDEBUGGING, MF_BYCOMMAND | (!hasDbg ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem(hMenu, ID_DEBUG_STOPDEBUGGING,  MF_BYCOMMAND | (hasDbg ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem(hMenu, ID_DEBUG_STOPDEBUGGING,  MF_BYCOMMAND | (hasDbg ? MF_ENABLED : MF_GRAYED));

				EnableMenuItem(hMenu, ID_DEBUG_RUN_STOP,       MF_BYCOMMAND | (hasDbg ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem(hMenu, ID_DEBUG_RUN_STEPIN,     MF_BYCOMMAND | (hasDbg ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem(hMenu, ID_DEBUG_RUN_STEPOUT,    MF_BYCOMMAND | (hasDbg ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem(hMenu, ID_DEBUG_RUN_RUNTOCURSOR,MF_BYCOMMAND | (hasDbg ? MF_ENABLED : MF_GRAYED));

				//EnableMenuItem(hMenu, ID_DEBUG_WINDOWS_WATCH,  MF_BYCOMMAND | (hasDbg ? MF_ENABLED : MF_GRAYED));
				//EnableMenuItem(hMenu, ID_DEBUG_WINDOWS_STACK,  MF_BYCOMMAND | (hasDbg ? MF_ENABLED : MF_GRAYED));
				//EnableMenuItem(hMenu, ID_DEBUG_WINDOWS_VARIABLES,MF_BYCOMMAND | (hasDbg ? MF_ENABLED : MF_GRAYED));
				//EnableMenuItem(hMenu, ID_DEBUG_WINDOWS_MEMORY, MF_BYCOMMAND | (hasDbg ? MF_ENABLED : MF_GRAYED));

				CheckMenuItem(
								hMenu,
								ID_DEBUG_WINDOWS_WATCH,
								pBed->FindPane( _T("Watch")) ? MF_CHECKED : MF_UNCHECKED
							);
				CheckMenuItem(
								hMenu,
								ID_DEBUG_WINDOWS_REGISTERS,
								pBed->FindPane(_T("Registers")) ? MF_CHECKED : MF_UNCHECKED
							);
				CheckMenuItem(
								hMenu,
								ID_DEBUG_WINDOWS_STACK,
								pBed->FindPane(_T("Stack")) ? MF_CHECKED : MF_UNCHECKED
							);
				CheckMenuItem(
								hMenu,
								ID_DEBUG_WINDOWS_THREADS,
								pBed->FindPane(_T("Threads")) ? MF_CHECKED : MF_UNCHECKED
							);
				CheckMenuItem(
								hMenu,
								ID_DEBUG_WINDOWS_VARIABLES,
								pBed->FindPane(_T("Variables")) ? MF_CHECKED : MF_UNCHECKED
							);
				CheckMenuItem(
								hMenu,
								ID_DEBUG_WINDOWS_MEMORY,
								pBed->FindPane(_T("Memory")) ? MF_CHECKED : MF_UNCHECKED
							);
			}
			else if(LOWORD(lParam) == 6)
			{
				// window menu
				Bview* pView = pBed->GetViews();
				bool   multiView = pView ? (pView->GetNext(pBed->GetViews()) != NULL) : false;

				// setup checks for view windows
				//
				EnableMenuItem(hMenu, ID_WINDOW_NEXT, MF_BYCOMMAND | (multiView ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem(hMenu, ID_WINDOW_PREVIOUS, MF_BYCOMMAND | (multiView ? MF_ENABLED : MF_GRAYED));
			}
		}
		break;

	case WM_COMMAND:

		if(pBed)
			pView = pBed->GetCurrentView();
		else
			pView = NULL;

		switch(wParam)
		{
		case ID_FILE_NEW:
			if(pView) pView->Dispatch(NewBuffer);
			break;
		case ID_FILE_OPEN:
			if(pView) pView->Dispatch(OpenBuffer);
			break;
		case ID_FILE_CLOSE:
			if(pView) pView->Dispatch(KillBuffer);
			break;
		case ID_FILE_CHECKOUT:
			if(pView) pView->Dispatch(CheckOut);
			break;
		case ID_FILE_CHECKIN:
			if(pView) pView->Dispatch(CheckIn);
			break;
		case ID_FILE_REVERT:
			if(pView) pView->Dispatch(Revert);
			break;
		case ID_TOOLS_PROJECT_WIZARD:
		case ID_FILE_NEW_PROJECT:
			if(pView) pView->Dispatch(::NewProject);
			break;
		case ID_FILE_OPEN_PROJECT:
			if(pView) pView->Dispatch(::OpenProject);
			break;
		case ID_FILE_SAVE:
			if(pView) pView->Dispatch(SaveBuffer);
			break;
		case ID_FILE_SAVEAS:
			if(pView) pView->Dispatch(SaveAs);
			break;
		case ID_FILE_SAVEALL:
			if(pView) pView->Dispatch(SaveAll);
			break;
		case ID_FILE_PRINT_DOCUMENT:
			if(pView) pView->Dispatch(PrintDocument);
			break;
		case ID_FILE_PRINT_WINDOW:
			if(pView) pView->Dispatch(PrintScreen);
			break;
		case ID_FILE_PRINT_SELECTION:
			if(pView) pView->Dispatch(PrintSelection);
			break;
		case ID_FILE_PAGESETUP:
			if(pView) pView->Dispatch(PageSetup);
			break;
		case ID_FILE_EXIT:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		case ID_EDIT_UNDO:
			if(pView) pView->Dispatch(Undo);
			break;
		case ID_EDIT_REDO:
			if(pView) pView->Dispatch(Redo);
			break;
		case ID_EDIT_CUT:
			if(pView) pView->Dispatch(Cut);
			break;
		case ID_EDIT_PASTE:
			if(pView) pView->Dispatch(Paste);
			break;
		case ID_EDIT_COPY:
			if(pView) pView->Dispatch(Copy);
			break;
		case ID_EDIT_DELETE:
			if(pView) pView->Dispatch(DeleteBlock);
			break;
		case ID_EDIT_SELECTALL:
			if(pView) pView->Dispatch(SelectAll);
			break;
		case ID_EDIT_GOTO:
			if(pView) pView->Dispatch(MoveToLine);
			break;
		case ID_EDIT_FIND:
			if(pView) pView->Dispatch(FindForward);
			break;
		case ID_EDIT_REPLACE:
			if(pView) pView->Dispatch(SearchAndReplace);
			break;
		case ID_EDIT_FINDINFILE:
			if(pView) pView->Dispatch(FindInFile);
			break;
		case ID_EDIT_REPLACEINFILE:
			if(pView) pView->Dispatch(ReplaceInFile);
			break;
		case ID_EDIT_FINDDEFOF:
			if(pView) pView->Dispatch(FindDefinitionOf);
			break;
		case ID_EDIT_FINDREFSOF:
			if(pView) pView->Dispatch(FindReferencesOf);
			break;
		case ID_EDIT_CASE_SENSITIVE:
			if(pView) pView->Dispatch(ToggleCase);
			break;
		case ID_EDIT_OVERSTRIKE:
			if(pView) pView->Dispatch(ToggleOverstrike);
			break;
		case ID_EDIT_TRIMWHITESPACE:
			if(pView) pView->Dispatch(ToggleTrimOnWrite);
			break;
		case ID_ADVANCED_TRIMWHITESPACE:
			if(pView) pView->Dispatch(TrimWhitespace);
			break;
		case ID_EDIT_ADVANCED_TABIFY:
			if(pView) pView->Dispatch(Tabify);
			break;
		case ID_EDIT_ADVANCED_UNTAB:
			if(pView) pView->Dispatch(Untab);
			break;
		case ID_EDIT_ADVANCED_MAKEWRITABLE:
			if(pView) pView->Dispatch(MakeWritable);
			break;
		case ID_EDIT_ADVANCED_DROPCR:
			if(pView) pView->Dispatch(ToggleCarriageReturns);
			break;
		case ID_EDIT_ADVANCED_TABSASSPACES:
			if(pView)
			{
				pView->PushParm(! pView->GetTabsAsSpaces(), ptNumber);
				pView->Dispatch(TabsAsSpaces);
			}
			break;
		case ID_EDIT_ADVANCED_AUTOTABDETECT:
			if(pView) pView->SetAutoTabDetect(! pView->GetAutoTabDetect());
			break;
		case ID_WINDOW_CLOSE:
			if(pView) pView->Dispatch(KillWindow);
			break;
		case ID_WINDOW_CLOSEALL:
			if(pView) pView->Dispatch(KillAllWindows);
			break;
		case ID_WINDOW_SPLITVERTICAL:
			if(pView) pView->Dispatch(SplitVertical);
			break;
		case ID_WINDOW_SPLITHORIZONTAL:
			if(pView) pView->Dispatch(SplitHorizontal);
			break;
		case ID_WINDOW_NEXT:
			if(pView) pView->Dispatch(NextWindow);
			break;
		case ID_WINDOW_PREVIOUS:
			if(pView) pView->Dispatch(PreviousWindow);
			break;

		case ID_VIEW_WINDOWS_PICKER:
			bShowit = 0 != (GetMenuState(GetMenu(hWnd), ID_VIEW_WINDOWS_PICKER, MF_BYCOMMAND) ^ MF_CHECKED);
			if(pBed) pBed->ShowPanel(_T("Picker"), _T("Picker"), bShowit);
			break;
		case ID_VIEW_WINDOWS_CALCULATOR:
			bShowit = 0 != (GetMenuState(GetMenu(hWnd), ID_VIEW_WINDOWS_CALCULATOR, MF_BYCOMMAND) ^ MF_CHECKED);
			if(pBed) pBed->ShowPanel(_T("Picker"), _T("bcalc"), bShowit);
			break;
		case ID_VIEW_WINDOWS_FUNCTIONS:
			bShowit = 0 != (GetMenuState(GetMenu(hWnd), ID_VIEW_WINDOWS_FUNCTIONS, MF_BYCOMMAND) ^ MF_CHECKED);
			if(pBed) pBed->ShowPanel(_T("Functions"), NULL, bShowit);
			break;
		case ID_VIEW_WINDOWS_FILES:
			bShowit = 0 != (GetMenuState(GetMenu(hWnd), ID_VIEW_WINDOWS_FILES, MF_BYCOMMAND) ^ MF_CHECKED);
			if(pBed) pBed->ShowPanel(_T("Files"), NULL, bShowit);
			break;
		case ID_VIEW_WINDOWS_INFO:
			bShowit = 0 != (GetMenuState(GetMenu(hWnd), ID_VIEW_WINDOWS_INFO, MF_BYCOMMAND) ^ MF_CHECKED);
			if(pBed) pBed->ShowPanel(_T("Info"), NULL, bShowit);
			break;
		case ID_VIEW_TABSPACE:
			if(pView) pView->Dispatch(SetTabSpace);
			break;
		case ID_VIEW_INDENTSPACE:
			if(pView) pView->Dispatch(IndentSpace);
			break;
		case ID_VIEW_KEYBINDINGS:
			if(pView) pView->Dispatch(SetKeyBinds);
			break;
		case ID_VIEW_SHOWLINENUMS:
			pView->Dispatch(pView->GetShowLineNums() ? HideLineNums : ShowLineNums);
			break;
		case ID_VIEW_SHOWWHITESPACE:
			pView->Dispatch(pView->GetShowTabs() ? HideWhitespace : ShowWhitespace);
			break;
		case ID_VIEW_VIEWASHEX:
			if(pView) pView->Dispatch(ViewHex);
			break;
		case ID_VIEW_VIEWRADIX:
			if(pView) pView->Dispatch(SetBinRadix);
			break;
		case ID_VIEW_VIEWFORMAT:
			if(pView) pView->Dispatch(SetBinFormat);
			break;
		case ID_VIEW_VIEWCOLUMNS:
			if(pView) pView->Dispatch(SetBinColumns);
			break;
		case ID_VIEW_COLORS_LINOBACKGROUND:
			if(pView) { pView->PushParm(kwLinoBackground, ptColor); pView->Dispatch(SetBkgColor); }
			break;
		case ID_VIEW_COLORS_BACKGROUND:
			if(pView) { pView->PushParm(kwBackground, ptColor); pView->Dispatch(SetBkgColor); }
			break;
		case ID_VIEW_COLORS_NORMAL:
			if(pView) { pView->PushParm(kwPlain, ptColor); pView->Dispatch(SetFrgColor); }
			break;
		case ID_VIEW_COLORS_QUOTED:
			if(pView) { pView->PushParm(kwQuoted, ptColor); pView->Dispatch(SetFrgColor); }
			break;
		case ID_VIEW_COLORS_COMMENT:
			if(pView) { pView->PushParm(kwComment, ptColor); pView->Dispatch(SetFrgColor); }
			break;
		case ID_VIEW_COLORS_BUILTIN:
			if(pView) { pView->PushParm(kwBuiltin, ptColor); pView->Dispatch(SetFrgColor); }
			break;
		case ID_VIEW_COLORS_BUILTINTYPE:
			if(pView) { pView->PushParm(kwBuiltinType, ptColor); pView->Dispatch(SetFrgColor); }
			break;
		case ID_VIEW_COLORS_BUILTINFUNCTION:
			if(pView) { pView->PushParm(kwBuiltinFunc, ptColor); pView->Dispatch(SetFrgColor); }
			break;
		case ID_VIEW_COLORS_ADDINTYPE:
			if(pView) { pView->PushParm(kwAddonType, ptColor); pView->Dispatch(SetFrgColor); }
			break;
		case ID_VIEW_COLORS_ADDINFUNCTION:
			if(pView) { pView->PushParm(kwAddonFunc, ptColor); pView->Dispatch(SetFrgColor); }
			break;
		case ID_VIEW_COLORS_MACRO:
			if(pView) { pView->PushParm(kwMacro, ptColor); pView->Dispatch(SetFrgColor); }
			break;
		case ID_VIEW_COLORS_OPERATOR:
			if(pView) { pView->PushParm(kwOperator, ptColor); pView->Dispatch(SetFrgColor); }
			break;
		case ID_VIEW_COLORS_SPECIALTEXT:
			if(pView) { pView->PushParm(kwSpecial, ptColor); pView->Dispatch(SetFrgColor); }
			break;
		case ID_VIEW_COLORS_SELECTEDBACKGROUND:
			if(pView) { pView->PushParm(kwSelected, ptColor); pView->Dispatch(SetFrgColor); }
			break;
		case ID_VIEW_COLORS_HIGHLIGHTBACKGROUND:
			if(pView) { pView->PushParm(kwHighlight, ptColor); pView->Dispatch(SetFrgColor); }
			break;
		case ID_VIEW_FONTS_NORMAL: 
		case ID_VIEW_FONTS_QUOTED:
		case ID_VIEW_FONTS_COMMENT:
		case ID_VIEW_FONTS_BUILTIN:
		case ID_VIEW_FONTS_BUILTINTYPE:
		case ID_VIEW_FONTS_BUILTINFUNCTION:
		case ID_VIEW_FONTS_ADDINTYPE:
		case ID_VIEW_FONTS_ADDINFUNCTION:
		case ID_VIEW_FONTS_MACRO:
		case ID_VIEW_FONTS_OPERATOR:
		case ID_VIEW_FONTS_SPECIALTEXT:
		case ID_VIEW_FONT:
			{
				int font;
				
				switch(wParam)
				{
				case ID_VIEW_FONTS_NORMAL:				font = kwPlain;			break;
				case ID_VIEW_FONTS_QUOTED:				font = kwQuoted;		break;
				case ID_VIEW_FONTS_COMMENT:				font = kwComment;		break;
				case ID_VIEW_FONTS_BUILTIN:				font = kwBuiltin;		break;
				case ID_VIEW_FONTS_BUILTINTYPE:			font = kwBuiltinType;	break;
				case ID_VIEW_FONTS_BUILTINFUNCTION:		font = kwBuiltinFunc;	break;
				case ID_VIEW_FONTS_ADDINTYPE:			font = kwAddonType;		break;
				case ID_VIEW_FONTS_ADDINFUNCTION:		font = kwAddonFunc;		break;
				case ID_VIEW_FONTS_MACRO:				font = kwMacro;			break;
				case ID_VIEW_FONTS_OPERATOR:			font = kwOperator;		break;
				case ID_VIEW_FONTS_SPECIALTEXT:			font = kwSpecial;		break;
				default:	
				case ID_VIEW_FONT:
														font = -1;				break;
				}
				if(pView)
				{
					pView->PushParm(font, ptNumber);
					pView->Dispatch(SetFont);
				}
			}
			break;
		case ID_VIEW_FONT_SETDEFAULT:

			if(pView)
				pView->Dispatch(SetDefaultFonts);
			break;

		case ID_TOOLS_PROJECT_EDIT:
			if(pBed && pBed->GetProject())
				pBed->GetProject()->ProjectSetup(hWnd);
			break;
		case ID_TOOLS_PROJECT_EXPORTMAKE:
			break;
		case ID_TOOLS_SOURCECONTROL_ENABLE:
			if(pBed) pBed->SetupSCCS(false);
			break;
		case ID_TOOLS_SOURCECONTROL_EDITSETTINGS:
			if(pBed && pBed->GetSCCS())
				pBed->GetSCCS()->EditSettings(hWnd);
			break;
		case ID_TOOLS_SOURCECONTROL_SETUPWIZARD:
			if(pBed) pBed->SetupSCCS(true);
			break;

		case ID_TOOLS_PROGRAMDATABASE_ENABLE_ALWAYS:
			if(pBed) pBed->SetPDBenable(pdbAlways);
			break;
		case ID_TOOLS_PROGRAMDATABASE_ENABLE_ONPROJECTLOAD:
			if(pBed) pBed->SetPDBenable(pdbOnLoad);
			break;
		case ID_TOOLS_PROGRAMDATABASE_ENABLE_NEVER:
			if(pBed) pBed->SetPDBenable(pdbNever);
			break;

		case ID_TOOLS_SHELL:
			if(pBed) pBed->EditBuffer(NULL, true, btShell);
			break;
		
		case ID_TOOLS_REMOTESHELL:
			if(pBed) pBed->EditBuffer(NULL, true, btTelnet);
			break;

		case ID_TOOLS_SECURESHELL:
			if(pBed) pBed->EditBuffer(NULL, true, btSSH);
			break;

		case ID_TOOLS_TERMINAL:
			if(pBed)
			{
				TCHAR port[MAX_PATH];

				if(pBed->GetPersist()->GetNvStr(_T("RS232/CurrentPort"), port, MAX_PATH, _T("0")) == errOK)
				{
					// if any current rs232 views in this port ignore
					//
					for(pView = pBed->GetViews(); pView; pView = pView->GetNext(pView))
					{
						if(pView->GetBuffer() && ! _tcscmp(pView->GetTypeName(), _T("RS232")))
						{
							if(! _tcscmp(((BviewRS232*)pView)->GetPort(), port))
							{
								TCHAR msgtext[MAX_PATH + 128];

								_sntprintf(msgtext, MAX_PATH+128, _T("Port " _Pfs_ " already open\n"), port);
								MessageBox(NULL, msgtext, _T("BED 6.0 - Terminal"), MB_OK);
								return 0;
							}
						}
					}
				}
			}
			if(pBed) pBed->EditBuffer(NULL, true, btTerm);
			break;

		case ID_TOOLS_SHELLSETTINGS_LOCAL:
			if(pBed)
			{
				pView = pBed->GetCurrentView();
				if(pView && ! _tcscmp(pView->GetTypeName(), _T("Shell")))
				{
					BviewTerminal::EmulationDialog((BviewTerminal*)pView, hWnd);
				}
				else
				{
					BviewTerminal::EmulationDialog(pBed, NULL, _T("Shell"), hWnd);
				}
			}
			break;

		case ID_TOOLS_SHELLSETTINGS_REMOTE:
			if(pBed)
			{
				pView = pBed->GetCurrentView();
				if(pView && ! _tcscmp(pView->GetTypeName(), _T("Telnet")))
				{
					// save host/port and reopen if they change
					//
					Bpersist*	 pp;
					BviewTelnet* pt = (BviewTelnet*)pView;
					TCHAR		 hst[MAX_PATH], nhst[MAX_PATH];
					int			 prt, nprt;

					_tcscpy(hst, pt->GetHost());
					prt = pt->GetPort();

					BviewTelnet::SettingsDialog(pBed, (BviewTerminal*)pView, hWnd);

					pp = pBed->GetPersist();
					if(pp)
					{
						pp->GetNvInt(_T("Telnet/CurrentPort"), nprt, prt);
						pp->GetNvStr(_T("Telnet/CurrentHost"), nhst, MAX_PATH, hst);

						if(_tcscmp(nhst, hst) || nprt != prt)
						{
							pt->ReopenPort();
						}
					}
				}
				else
				{
					BviewTelnet::SettingsDialog(pBed, NULL, hWnd);
				}
			}
			break;

		case ID_TOOLS_SHELLSETTINGS_SERIAL:
			if(pBed)
			{
				LPCTSTR port;

				// setup the "current" port for settings to actual port
				// of current view if current view is rs232
				//
				pView = pBed->GetCurrentView();
				if(pView && _tcscmp(pView->GetTypeName(), _T("RS232")))
				{
					for(pView = pBed->GetViews(); pView; pView = pView->GetNext(pView))
					{
						if(pView->GetBuffer() && ! _tcscmp(pView->GetTypeName(), _T("RS232")))
						{
							break;
						}
					}
				}
				if(pView &&  ! _tcscmp(pView->GetTypeName(), _T("RS232")))
				{
					port = ((BviewRS232*)pView)->GetPort();
					pBed->GetPersist()->SetNvStr(_T("RS232/CurrentPort"), port);

					if(((BviewRS232*)pView)->SettingsDialog(hWnd) == IDOK)
					{
						((BviewRS232*)pView)->ReopenPort();
						return 0;
					}
				}
			}
			break;

		case ID_TOOLS_CLEAR:
			{
				bool   isTerm = FALSE;
				
				if(pView != NULL)
				{
					if(! _tcscmp(pView->GetTypeName(), _T("RS232")))
						isTerm = true;
					if(! _tcscmp(pView->GetTypeName(), _T("Shell")))
						isTerm = true;
					if(! _tcscmp(pView->GetTypeName(), _T("Telnet")))
						isTerm = true;
				}
				if(isTerm)
				{
					pView->Dispatch(EmptyBuffer);
					pView->MoveAbs(1, 1);
				}
			}
			break;

		case ID_BUILD:
			if(pView) pView->Dispatch(Build);
			break;
		case ID_DEBUG_STOPDEBUGGING:
			if(pView) pView->Dispatch(StopDebug);
			break;
		case ID_DEBUG_STARTDEBUGGING:
			if(pView) pView->Dispatch(::Debug);
			break;
		case ID_DEBUG_EDITSETTINGS:
			if(pBed)
			{
				if(! pBed->GetDebugger())
				{
					pBed->SetupDebug(false);
				}
				if(pBed->GetDebugger())
				{
					pBed->GetDebugger()->EditSettings(hWnd);
				}
			}
			break;
		case ID_DEBUG_RUN_GO:
			if(pView) pView->Dispatch(Start);
			break;
		case ID_DEBUG_RUN_STEPIN:
			if(pView) pView->Dispatch(StepIn);
			break;
		case ID_DEBUG_RUN_STEPOUT:
			if(pView) pView->Dispatch(StepOut);
			break;
		case ID_DEBUG_RUN_RUNTOCURSOR:
			if(pView) pView->Dispatch(Step);
			break;
		case ID_DEBUG_WINDOWS_WATCH:
			bShowit = 0 != (GetMenuState(GetMenu(hWnd), ID_DEBUG_WINDOWS_WATCH, MF_BYCOMMAND) ^ MF_CHECKED);
			if(pBed) pBed->ShowPanel(_T("Debug"), _T("Watch"), bShowit);
			break;
		case ID_DEBUG_WINDOWS_REGISTERS:
			bShowit = 0 != (GetMenuState(GetMenu(hWnd), ID_DEBUG_WINDOWS_REGISTERS, MF_BYCOMMAND) ^ MF_CHECKED);
			if(pBed) pBed->ShowPanel(_T("Debug"), _T("Registers"), bShowit);
			break;
		case ID_DEBUG_WINDOWS_STACK:
			bShowit = 0 != (GetMenuState(GetMenu(hWnd), ID_DEBUG_WINDOWS_STACK, MF_BYCOMMAND) ^ MF_CHECKED);
			if(pBed) pBed->ShowPanel(_T("Debug"), _T("Stack"), bShowit);
			break;
		case ID_DEBUG_WINDOWS_THREADS:
			bShowit = 0 != (GetMenuState(GetMenu(hWnd), ID_DEBUG_WINDOWS_THREADS, MF_BYCOMMAND) ^ MF_CHECKED);
			if(pBed) pBed->ShowPanel(_T("Debug"), _T("Threads"), bShowit);
			break;
		case ID_DEBUG_WINDOWS_VARIABLES:
			bShowit = 0 != (GetMenuState(GetMenu(hWnd), ID_DEBUG_WINDOWS_VARIABLES, MF_BYCOMMAND) ^ MF_CHECKED);
			if(pBed) pBed->ShowPanel(_T("Debug"), _T("Variables"), bShowit);
			break;
		case ID_DEBUG_WINDOWS_MEMORY:
			bShowit = 0 != (GetMenuState(GetMenu(hWnd), ID_DEBUG_WINDOWS_MEMORY, MF_BYCOMMAND) ^ MF_CHECKED);
			if(pBed) pBed->ShowPanel(_T("Debug"), _T("Memory"), bShowit);
			break;
			
		case ID_HELP_COMMANDS:
			if(pView) pView->Dispatch(ShowCommands);
			break;
		case ID_HELP_KEYBINDINGS:
			if(pView) pView->Dispatch(ShowKeyBindings);
			break;
		case ID_HELP_ABOUT:
			if(pView) pView->Dispatch(HelpAbout);
			break;
		default:
			if(wParam > ID_RCNT_FILES && wParam <= ID_RCNT_FILES + 64)
			{
				TCHAR	fileName[MAX_PATH];
				int 	id = wParam -ID_RCNT_FILES;
				ERRCODE ec;
				
				ec = pBed->GetRecentFile(id, fileName, MAX_PATH);
				
				if(ec == errOK)
					pBed->EditBuffer(fileName, true, btAny);
			}
			else if(wParam > ID_RCNT_PROJECTS && wParam <= ID_RCNT_PROJECTS + 64)
			{
				TCHAR	fileName[MAX_PATH];
				int 	id = wParam -ID_RCNT_PROJECTS;
				ERRCODE ec;
				
				ec = pBed->GetRecentProject(id, fileName, MAX_PATH);
				
				if(ec == errOK)
					pBed->OpenProject(fileName, true);
			}
			break;
		}
		break;

	case WM_LBUTTONDOWN:
		if(::GetFocus() != hWnd)
			::SetFocus(hWnd);
		if(pBed)
			pView = pBed->GetCurrentView();
		else
			pView = NULL;
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}


char setupScriptFormat[] =

"Begin;"
"  Panel[panel(1)]:Welcome to the BED Setup Wizard;"
"    Description:This Wizard is used for initial setup of BED;"
"    LeftImage:%d;"
"    Next[default];Cancel;"
"  EndPanel;"

"  Panel[panel(2)]:Default Encoding Setup;"
"    Description:Choose a default text encoding;"
"    IconImage:%d;"
"    ChoiceGroup:Select encoding to assume if none is detected in a file;"
"       Choice[panel(3),var(bool Encode ANSI)]:ANSI (US/ASCII);"
"       Choice[panel(3),var(bool Encode UTF8)]:Unicode UTF-8;"
"       Choice[panel(3),var(bool Encode UCS2L)]:Unicode UCS-2 (Little Endian);"
"       Choice[panel(3),var(bool Encode UCS2B)]:Unicode UCS-2 (Big Endian);"
"       Choice[panel(3),var(bool Encode UCS4L)]:Unicode UCS-4 (Little Endian);"
"       Choice[panel(3),var(bool Encode UCS4B)]:Unicode UCS-4 (Big Endian);"
"    EndChoiceGroup;"
"    Back;Next[default];Cancel;"
"  EndPanel;"

"  Panel[panel(3)]:Setup Complete;"
"    IconImage:%d;"
"    Label:Click Finish to Complete Installation;"
"    Back;Finish[default];"
"  EndPanel;"

"End;";

//**************************************************************************
bool OnSetupData(LPVOID cookie, int* step, wizMethod method, LPCTSTR var, LPTSTR val)
{
	Bpersist* persist;
	LPCTSTR   tc;
	LPTSTR    td;

	TCHAR   typecode[32] = { 0 };

	TEXTENCODING setenc, defenc;


	if(! (persist = (Bpersist*)cookie))
		return false;

	if(var)
	{
		for(tc = var, td = typecode; *tc && *tc != _T(' ') && ((td - typecode) < 30);)
			*td++ = *tc++;
		while(*tc && *tc == _T(' '))
			tc++;
		*td = _T('\0');
		var = tc;
		while(*var == _T(' '))
			var++;
	}
	int iv;
	bool bv;

	switch(method)
	{
	case wizGetValue:
		if(! val) return false;
		switch(typecode[0])
		{
		case _T('i'):
			persist->GetNvInt(var, iv, 0);
			_sntprintf(val, 32, _T("%d"), iv);
			break;
		case _T('b'):
			setenc = (TEXTENCODING)-1;
			if(!  _tcscmp(var, _T("Encode ANSI")))
				setenc = txtANSI;
			if(!  _tcscmp(var, _T("Encode UTF8")))
				setenc = txtUTF8;
			if(!  _tcscmp(var, _T("Encode UCS2L")))
				setenc = txtUCS2SWAP;
			if(!  _tcscmp(var, _T("Encode UCS2B")))
				setenc = txtUCS2;
			if(!  _tcscmp(var, _T("Encode UCS4L")))
				setenc = txtUCS4SWAP;
			if(!  _tcscmp(var, _T("Encode UCS4B")))
				setenc = txtUCS4;
			else
				persist->GetNvBool(var, bv, false);

			if(setenc != (TEXTENCODING)-1)
			{
				persist->GetNvInt(_T("Default Text Encoding"), (int&)defenc, txtANSI);
				bv = (setenc == defenc);
			}
			_sntprintf(val, 32, _T("%d"), bv ? 1 : 0);
			break;
		case _T('s'):
			persist->GetNvStr(var, val, MAX_SCCS_CMD);
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
			persist->SetNvInt(var, _tcstol(val, NULL, 0));
			break;
		case _T('b'):
			setenc = (TEXTENCODING)-1;
			if(!  _tcscmp(var, _T("Encode ANSI")))
				setenc = txtANSI;
			if(!  _tcscmp(var, _T("Encode UTF8")))
				setenc = txtUTF8;
			if(!  _tcscmp(var, _T("Encode UCS2L")))
				setenc = txtUCS2SWAP;
			if(!  _tcscmp(var, _T("Encode UCS2B")))
				setenc = txtUCS2;
			if(!  _tcscmp(var, _T("Encode UCS4L")))
				setenc = txtUCS4SWAP;
			if(!  _tcscmp(var, _T("Encode UCS4B")))
				setenc = txtUCS4;
			else
				persist->SetNvBool(var, _tcstol(val, NULL, 0) != 0);

			if(setenc != (TEXTENCODING)-1 && (_tcstol(val, NULL, 0) != 0))
			{
				persist->SetNvInt(_T("Default Text Encoding"), (int)setenc);
			}
			break;
		case _T('s'):
			persist->SetNvStr(var, val);
			break;
		default:
			return false;
		}
		return true;

	case wizStepBack:
		break;

	case wizStepForward:
		break;
	}
	return false;
}

//**************************************************************************
ERRCODE RunSetupWizard(Bpersist* pPersist)
{
	ERRCODE  ec = errOK;
	Bwizard* pwiz = NULL;
	TCHAR* setupScript;
	int len;

	// create the wizard and start it off
	//
	TCHAR* pdat;

	pwiz = new Bwizard();
	len = 256 + strlen(setupScriptFormat);
	pdat = new TCHAR [ len ];
	setupScript = new TCHAR [ len ];
	CharToTChar(pdat, setupScriptFormat);
	_sntprintf(setupScript, len, pdat, IDB_WIZMAP, IDI_BED, IDI_BED);

	ec = pwiz->Begin(_T("BED - Setup"), setupScript, OnSetupData, (LPVOID)pPersist, g_hInstance);

	delete pwiz;
	delete [] pdat;
	delete [] setupScript;
	
	return ec;
}
