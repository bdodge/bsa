
#define IDC_BF_PATH		0xbef8
#define IDC_BF_DIRTREE	0xbef9
#define IDC_BF_FILETREE	0xbefa
#define IDC_BF_FILENAME	0xbefb

//***********************************************************************
class Bflentry
{
public:
	Bflentry(LPCTSTR name, int type);
	~Bflentry();

	static Bflentry*		AddFileEntry(Bflentry* sym, Bflentry*& tree);
	static void				DumpFileEntry(Bflentry* tree);
	static void				DeleteFileEntry(Bflentry* sym);
	static Bflentry*		FindFileEntry(LPCTSTR name, Bflentry* tree);
	static Bflentry*		GetFileEntryNumber(Bflentry* tree, int num);
	static void				PruneFileEntry(Bflentry* tree, LPCTSTR pattern, Bflentry*& subtree, bool dironly);
	static void				InsertFileEntry(Bflentry* tree, HWND hWnd);
	
public:
	LPTSTR					m_name;
	int						m_type;

	Bflentry*				m_left;
	Bflentry*				m_right;
};

//**************************************************************************
class BdirInfo
{
public:
	BdirInfo()
	{
#ifndef Windows
		m_pDir = NULL;
		m_dp   = NULL;
#endif
	}
public:
	virtual ERRCODE	ListDirectory(LPCTSTR path);
	virtual ERRCODE NextFile(LPCTSTR& lpName, bool& isDir, bool& isLink, bool& isReadOnly);
	virtual ERRCODE	EndListDirectory(void);

	static LPTSTR	FilePart(LPTSTR path);
	
public:		
	TCHAR			m_szName[MAX_PATH];
	TCHAR			m_szBase[MAX_PATH];
	bool			m_first;
#ifdef Windows
	WIN32_FIND_DATA m_fdata;
	HANDLE			m_hFind;
	TCHAR			m_path[MAX_PATH + 32];
#else
	DIR*			m_pDir;
	struct dirent*	m_dp;
	char			m_path[MAX_PATH + 32];
	TCHAR			m_pattern[MAX_PATH + 32];
#endif
};


//***********************************************************************
typedef struct tagFileParms
{
public:
	tagFileParms(
					HINSTANCE	hInst,
					LPCTSTR		ptit,
					LPCTSTR		pName,
					DLGPROC		msgHook,
					bool		mustExist,
					bool		opening,
					bool		dirsonly,
					bool		qpenabled
				)
	{
		_tcsncpy(name, pName, MAX_PATH-1);
		name[MAX_PATH-1] = _T('\0');
		tabpattern[0]	 = _T('\0');
		messageHook	= msgHook;
		mustexist	= mustExist;
		foropen		= opening;
		qpok		= qpenabled;
		dironly		= dirsonly;
		tabcnt		= 0;
		pTitle		= ptit;
		ilist 		= NULL;
		hwndPath	= NULL;
		hwndDirs	= NULL;
		hwndFiles	= NULL;
		hwndFilename= NULL;
		filelist	= NULL;
		sublist		= NULL;
	}
	~tagFileParms()
	{
		if(hwndPath)	DestroyWindow(hwndPath);
		if(hwndDirs)	DestroyWindow(hwndDirs);
		if(hwndFiles)	DestroyWindow(hwndFiles);
		if(ilist)
			ImageList_Destroy(ilist);
		if(filelist)
			Bflentry::DeleteFileEntry(filelist);
		if(sublist)
			Bflentry::DeleteFileEntry(sublist);
	}
	HINSTANCE		hInst;
	LPCTSTR			pTitle;
	TCHAR			name[MAX_PATH];
	DLGPROC			messageHook;
	bool			mustexist;
	bool			foropen;
	bool			qpok;
	bool			dironly;
	int				tabcnt;

	TCHAR			bpath[MAX_PATH];
	TCHAR			bpattern[MAX_PATH];
	TCHAR			tabpattern[MAX_PATH];
	HIMAGELIST		ilist;
	HWND			hwndPath;
	HWND			hwndDirs;
	HWND			hwndFiles;
	HWND			hwndFilename;
	Bflentry*		filelist;
	Bflentry*		sublist;
}
FILEDIALPARMS, FAR *LPFILEDIALPARMS;



extern WORD _bsa_of_bmfile_icon[];
extern WORD _bsa_of_bmrofile_icon[];
extern WORD _bsa_of_bmdir_icon[];
extern WORD _bsa_of_bmopendir_icon[];

#define FILE_ICON		1
#define FILERO_ICON		2
#define DIR_ICON		3
#define DIROPEN_ICON	4

#define BOF_IW	16
#define BOF_IH	16
#define BOF_ID	24

typedef struct _tag_ofbmh
{
	int w, h, d;
	LPVOID bits;
}
BOFBMH, *PBOFBMH;

