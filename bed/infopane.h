#ifndef _INFOPANE_H_
#define _INFOPANE_H_ 1

// container class for a window in an info pane tab control
//**************************************************************************
class BinfoPane : public BappPane
{
public:
					BinfoPane		(LPCTSTR name, BappPanel* pPanel, Bed* pEditor, bool lines = true, bool images = false);
	virtual			~BinfoPane		();

public:
	// BappPane
	virtual LRESULT	OnMessage		(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
	virtual void	FitToPanel		(void);

	virtual void	LoadImages		(void);
	virtual void	Activate		(void);
	virtual void	Deactivate		(void);
	virtual bool	GetActive		(void) { return m_active;   }
	virtual void	SetupWindow		(void);
	virtual	void	SetView			(Bview* pView);
	virtual	Bview*	GetView			(void) { return m_view;     }
	virtual HWND	GetWindow		(void) { return m_hwndTree; }
	virtual void	MouseMenu		(int x, int y);
	virtual bool	HasMenu			(void)			{ return false; }
	virtual void	SpecificMenu	(HMENU hMenu)	{ return;		}
	virtual void	Select			(LPTVITEM pItem);
	virtual void	Event			(LPARAM lParam) { }
	virtual bool	IgnoreNotify	() { return m_ignoreNotify; }

protected:
	virtual HWND	MakeNewTree		(void);

protected:
	Bed*			m_editor;
	Bview*			m_view;
	HWND			m_hwndTree;
	RECT			m_viewrc;
	bool			m_ignoreNotify;
	DWORD			m_style;
	bool			m_hasimages;
	HIMAGELIST		m_ilist;
};

// file picker
//**************************************************************************
class BpickInfo : public BinfoPane
{
public:
					BpickInfo		(LPCTSTR name, BappPanel* pPanel, Bed* pParent);
	virtual			~BpickInfo		();

public:
	virtual void	LoadImages		(void);
	virtual	void	SetView			(Bview* pView);
	virtual void	Select			(LPTVITEM pItem);

protected:

protected:
};


// function view object
//**************************************************************************
class BfuncInfo : public BinfoPane
{
public:
					BfuncInfo		(LPCTSTR name, BappPanel* pPanel, Bed* pParent);
	virtual			~BfuncInfo		();

public:
	virtual void	LoadImages		(void);
	virtual	void	SetView			(Bview* pView);
	virtual void	Select			(LPTVITEM pItem);
	virtual void	SetToLine		(int line);

protected:
	static ERRCODE	EnumFuncsStub	(LPVOID cookie, LPCTSTR pName, LPCTSTR pTypeName, int isPtr, int line, EnumFuncsType type);
	virtual ERRCODE EnumFunc		(LPCTSTR pName, LPCTSTR pTypeName, int isPtr, int line, EnumFuncsType type);

protected:
	HTREEITEM		m_hCurItem;
	HTREEITEM		m_hCurParent;
	int				m_funcstartline;
	int				m_funcendline;
};

typedef struct tag_dirinfo
{
public:
	tag_dirinfo()
	{
#ifndef Windows
		pDir = NULL;
		dp   = NULL;
#endif
	}
public:
	TCHAR			szName[MAX_PATH];
	TCHAR			szBase[MAX_PATH];
	bool			first;
#ifdef Windows
	WIN32_FIND_DATA fdata;
	HANDLE			hFind;
	TCHAR			path[MAX_PATH + 32];
#else
	DIR*			pDir;
	struct dirent*	dp;
	char			path[MAX_PATH + 32];
	TCHAR			pattern[MAX_PATH + 32];
#endif
}
BDIRINFO, *LPBDIRINFO;

typedef LPVOID HDIRLIST;


// file view object
//**************************************************************************
class BfileInfo : public BinfoPane
{
public:
					BfileInfo		(LPCTSTR name, BappPanel* pPanel, Bed* pParent);
	virtual			~BfileInfo		();

public:
	virtual void	LoadImages		(void);
	virtual	void	SetView			(Bview* pView);
	virtual bool	SetPath			(LPCTSTR path);
	virtual void	Select			(LPTVITEM pItem);

public:
	static LPTSTR	FilePart		(LPTSTR path);
	static ERRCODE	ListDirectory	(HDIRLIST& hDir, LPCTSTR lpPath);
	static ERRCODE	EndListDirectory(HDIRLIST& hDir);
	static ERRCODE	NextFile		(HDIRLIST& hDir, LPCTSTR& fName, bool& isDir, bool& isLink, bool& isReadOnly);
	static bool		OnFtpFile		(LPCTSTR path, WORD mode, LPVOID cookie);
	static ERRCODE	SimplifyFilePath(LPCTSTR path, LPTSTR respath);

protected:

protected:
	TCHAR			m_path[MAX_PATH];
};

#define SHELL_IO_SIZE (4096)

#ifndef Windows
	// for X11 systems, need to update windows in a timer
	// fashion, since X is NOT reentrant.
	//
	#define TIMER_BASED_SHELLPOLL 1
#endif

// base class for shell based info panes
//**************************************************************************
class BshellInfo : public BinfoPane, public Bshell, public Bthread
{
public:
					BshellInfo		(LPCTSTR name, BappPanel* pPanel, Bed* pParent);
	virtual			~BshellInfo		();

public:
	virtual	void	SetView			(Bview* pView);
	virtual void	Select			(LPTVITEM pItem);

	virtual	void	Clear			(void);
	virtual void	AppendItem		(LPCTSTR pLine);

	virtual bool	HasMenu			(void)			{ return true; }
	virtual void	SpecificMenu	(HMENU hMenu);

	virtual bool	GetInsertHeader	(void) { return false; }
	virtual LPCTSTR	GetOperation	(void) { return _T(""); }
	virtual ERRCODE	SendShell		(LPBYTE cmd, DWORD& ncmd);
	virtual void	Finished		(void);
	virtual ERRCODE	Startup			(void);

	virtual void	Event			(LPARAM lParam);

	static ERRCODE WINAPI
					RunStub			(LPVOID thisptr);
	virtual ERRCODE	ShellThread		(void);

#ifdef TIMER_BASED_SHELLPOLL
	static void CALLBACK OnTimer(HWND hWnd, UINT message, UINT id, DWORD now);
#elif defined SHELL_BASED_SHELLPOLL
	static ERRCODE CALLBACK	ReaderThreadStub(LPVOID thisptr);
	virtual ERRCODE	ReaderThread	(void);
#endif

	static ERRCODE	GetErrorLine(LPCTSTR lpText, LPTSTR fName, int nfName, int* pLine);

protected:
	virtual LPCTSTR	GetPathPrefix	(void) { return m_szShellDir; }
	virtual BlineInfo GetLineIsType	(void) { return liNone; }

protected:
	TCHAR   		m_szShellCommand[1024];
	TCHAR			m_szShellDir[MAX_PATH];

	HWND			m_hwndParent;

	TCHAR			m_token[SHELL_IO_SIZE];
	int				m_toklen;

#ifdef TIMER_BASED_SHELLPOLL
	UINT_PTR		m_idTimer;
#elif defined THREAD_BASED_SHELLPOLL
	Bthread			m_reader;

	// event for letting reader know shell data has been added
	Bevent			m_devent;
#endif
	// ring buffer for shell->parse comm
	Bmutex			m_bufex;
	char			m_iobuf[SHELL_IO_SIZE + 2];
	int				m_head;
	int				m_tail;
	int				m_cnt;
	int				m_size;
};


// build view object
//**************************************************************************
class BbuildInfo : public BshellInfo
{
public:
					BbuildInfo		(LPCTSTR name, BappPanel* pPanel, Bed* pParent);
	virtual			~BbuildInfo		();

public:
	virtual bool	GetInsertHeader	(void) { return true; }
	virtual LPCTSTR	GetOperation	(void) { return _T("Building With: "); }
	virtual ERRCODE	Startup			(void);
	virtual void	Event			(LPARAM lParam);

protected:
	virtual void	Select			(LPTVITEM pItem);
	virtual LPCTSTR	GetPathPrefix	(void);
	virtual BlineInfo GetLineIsType	(void) { return liIsInError; }

protected:
	TCHAR			m_srcDir[MAX_PATH];
	int				m_errors;
	int				m_warnings;
	bool			m_shownResults;
};

// build results view object
//**************************************************************************
class BbuildResultsInfo : public BinfoPane
{
public:
					BbuildResultsInfo		(LPCTSTR name, BappPanel* pPanel, Bed* pParent);
	virtual			~BbuildResultsInfo		();

protected:
	virtual LPCTSTR	GetPathPrefix	(void);
	virtual BlineInfo GetLineIsType	(void) { return liIsInError; }

public:
	virtual	void	SetView			(Bview* pView);
	virtual void	Select			(LPTVITEM pItem);

	virtual	void	Clear			(void);
	virtual void	AppendItem		(LPCTSTR pLine);

protected:
	TCHAR			m_srcDir[MAX_PATH];
};

// SCCS view object
//**************************************************************************
class BsccsInfo : public BshellInfo
{
public:
					BsccsInfo		(LPCTSTR name, BappPanel* pPanel, Bed* pParent);
	virtual			~BsccsInfo		();

protected:
	virtual BlineInfo GetLineIsType	(void) { return liIsInError; }

public:
	virtual bool	GetInsertHeader	(void) { return true; }
	virtual LPCTSTR	GetOperation	(void) { return _T("SCCS Operation: "); }
	virtual ERRCODE	Startup			(LPCTSTR cmd);
	virtual bool	IsDone			(void) { return ! m_running; }

protected:
};


// Debugger view object
//**************************************************************************
class BdbgInfo : public BshellInfo
{
public:
					BdbgInfo		(LPCTSTR name, BappPanel* pPanel, Bed* pParent);
	virtual			~BdbgInfo		();

public: // Bshell
	virtual void	AppendItem		(LPCTSTR pLine);

public: // BshellInfo
	virtual bool	GetInsertHeader	(void) { return true; }
	virtual LPCTSTR	GetOperation	(void) { return _T("Debug With: "); }
	virtual ERRCODE	Startup			(void);
	virtual bool	IsDone			(void) { return ! m_running; }

protected:
	virtual LPCTSTR	GetPathPrefix	(void);
	virtual BlineInfo GetLineIsType	(void) { return liIsBreakpoint; }

protected:
};

KEY_ARRYVAL_LIST_TEMPLATE(BgrepList, LPTSTR, NO_API);
KEY_ARRYVAL_LIST_TEMPLATE(BfpatList, LPTSTR, NO_API);

#define GREP_MAX_RECURSE 64

// GREP (Find-In-Files) view object
//**************************************************************************
class BgrepInfo : public BshellInfo
{
public:
					BgrepInfo		(LPCTSTR name, BappPanel* pPanel, Bed* pParent);
	virtual			~BgrepInfo		();

public:
	virtual ERRCODE	ShellThread		(void);
	virtual bool	GetInsertHeader	(void)	{ return true;	}
	virtual ERRCODE	Startup			(LPCTSTR lpIniPattern, LPCTSTR lpIniPath, LPCTSTR lpIniFileTypes, bool matchWholeWords);
	virtual ERRCODE	StartupDirect	(LPCTSTR lpIniPattern, LPCTSTR lpIniPath, LPCTSTR lpIniFileTypes);
	virtual void	Finished		(void);
	virtual bool	IsDone			(void)	{ return ! m_running;	}

	virtual LPCTSTR	GetPattern		(void)	{ return m_grepPat;		}
	virtual LPCTSTR	GetPath			(void)	{ return m_grepDir;		}
	virtual LPCTSTR GetFilePattern	(void)	{ return m_filePattern;	}
	virtual bool	GetRecurseDirs	(void)	{ return m_recursive;	}
	virtual bool	GetCaseSensi	(void)	{ return m_casesensi;	}
	virtual bool	GetWholeWord	(void)	{ return m_wholeword;	}

	virtual void	SetPattern		(LPCTSTR lpPattern);
	virtual void	SetPath			(LPCTSTR lpPath);
	virtual void	SetFilePattern	(LPCTSTR lpFilePattern);
	virtual void	SetRecurseDirs	(bool r)	{ m_recursive = r;	}
	virtual void	SetCaseSensi	(bool s)	{ m_casesensi = s;	}
	virtual void	SetWholeWord	(bool w)	{ m_wholeword = w;	}


protected:
	virtual BlineInfo GetLineIsType	(void) { return liIsSearchTarget; }
	virtual ERRCODE InsertGrepLine	(LPCTSTR grepLine, int maxtimems);
	virtual ERRCODE	CheckFile		(LPCTSTR name);
	virtual ERRCODE	BuildFileList	(LPCTSTR path);
	virtual ERRCODE	BuildTypeList	(LPCTSTR path);

public:

protected:
	TCHAR			m_grepDir[MAX_PATH];
	TCHAR			m_grepPat[MAX_PATH];
	TCHAR			m_searchstr[MAX_PATH];
	LPTSTR			m_filePattern;
	int				m_nPat;
	bool			m_casesensi;
	bool			m_recursive;
	int				m_recurse_level;
	bool			m_wholeword;
	BgrepList*		m_fileList;
	BgrepList*		m_curFile;
	BfpatList*		m_ftypeList;
	int				m_fileCount;
	int				m_occurFiles;
	int				m_occurenceCount;
};

#endif

