#ifndef _BED_H_
#define _BED_H_ 1

extern HINSTANCE	g_hInstance;
extern LPCTSTR		g_appname;
extern int			g_vermaj;
extern int			g_vermin;

// recents lists
//**************************************************************************

KEY_ARRYVAL_LIST_TEMPLATE(BrecentList, LPTSTR, NO_API);

typedef enum { pdbNever, pdbOnLoad, pdbAlways } PDBENABLE;

// list of sub panels
KEY_VAL_LIST_TEMPLATE(BpanelList, PBAFPANEL, NO_API);

// editor object
//**************************************************************************
class Bed : public BappFrame
{
public:
						Bed				(HINSTANCE hInstance, LPCTSTR lpAppName);
						~Bed			();

public:
	// BappPanel
	virtual LRESULT		OnMessage		(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

	virtual BOOL		Close			(void);
	virtual void		SetFocus		(bool inout);
	virtual ERRCODE		SetCurrentView	(Bview* pView, bool replace);
	virtual Bview*		GetCurrentView	(void)   { return m_curview; }
	virtual void		UpdateInfoPanes	(Bview* pView);
	virtual LPCTSTR		GetEncodingName	(TEXTENCODING enc);
	virtual LPCTSTR		GetTypeName		(BufType type);
	virtual void		SetTitle		(Bbuffer* pBuf);
	virtual LPCTSTR		GetTitle		(Bbuffer* pBuf);
	virtual LPCTSTR		GetVersion		(void);
	virtual Blayout		GetViewLayout	(void)		{ return m_layout;		}
	virtual void		SetViewLayout	(Blayout l)	{ m_layout = l;		}
	virtual Bview*		GetViews		(void)		{ return m_views;		}
	virtual void		AddView			(Bview* pView);
	virtual void		AddOldView		(Bview* pView);
	virtual void		RemoveView		(Bview* pView);
	static  Bbuffer*	ProperBufferForFile(LPCTSTR pName, BufType type, TEXTENCODING encoding, TEXTENCODING defaultencoding, Bed* editor);
	static  Bview*		ProperViewForBuffer(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel);
	virtual void		AddBufferToPDB	(Bbuffer* pBuf);
	virtual void		SetBufferStatus	(Bbuffer* pBuf);
	virtual ERRCODE		EditBuffer		(Bbuffer* pBuf, int startLine = 0);
	virtual ERRCODE		EditBuffer		(LPCTSTR pName, bool frg, BufType type = btAny, TEXTENCODING encoding = (TEXTENCODING)-1, int startLine = 0);
	virtual ERRCODE		RemoveBuffer	(Bbuffer* pBuf);

	virtual ERRCODE		SetupWindows	(bool suppressTabPanes);
	virtual ERRCODE		ShowPanel		(LPCTSTR pPanelName, LPCTSTR pPaneName, bool bShowit);
	virtual ERRCODE		DepersistPanel	(LPCTSTR pPanelName, RECT& rc, BAF_EDGE& pe, bool& docked);
	virtual ERRCODE		PersistPanel	(PBAFPANEL pPanel);
	virtual bool		GetSuppressTabPanes(void)	{ return m_suppressTabPanes; }
	virtual PBAFPANEL	GetStatusPanel	(void)		{ return m_pStatusPanel; }
	virtual PBAFPANEL	GetInfoPanel	(void)		{ return m_pInfoPanel; }
	virtual PBAFPANEL	GetEditPanel	(void);

	virtual ERRCODE		MoveTo			(int& line, int& col, Bbuffer* pBuf);
	virtual void		UpdateStatusLino(void);
	virtual void		UpdateStatusInfo(void);
	virtual void		UpdateStatusText(void);
	virtual ERRCODE		Dispatch		(
										EditCommand command,
										LPCTSTR 	pParm,
										int			nParm,
										ParmType	parmType,
										Bbuffer* 	pBuf,
										Bview* 		pView
									);
	virtual Bpersist*	GetPersist		(void)		{ return m_persist;			}
	virtual Bsccs*		GetSCCS			(void)		{ return m_sccs;			}
	virtual ERRCODE		NewProject		(void);
	virtual ERRCODE		OpenProject		(LPCTSTR pName, bool restoreit);
	virtual void		SetProjectRestore(bool r)	{ m_restoreProjects = r;	}
	virtual bool		GetProjectRestore(void)		{ return m_restoreProjects; }
	virtual ERRCODE		CloseProject	(void);
	virtual ERRCODE		SetupDebug		(bool resetup);
	virtual Bdbg*		GetDebugger		(void)		{ return m_dbg;				}
	virtual ERRCODE		Debug			(void);
	virtual PDBENABLE	GetPDBenable	(void)		{ return m_pdbenable;		}
	virtual void		SetPDBenable	(PDBENABLE pdbe) {
		m_pdbenable = pdbe; m_persist->SetNvInt(_T("PDB/Enable"), m_pdbenable); }
	virtual ERRCODE		SetupSCCS		(bool resetup);
	virtual Bproject*	GetProject		(void)		{ return m_project;	}
	virtual BbufList*	GetBuffers		(void)		{ return m_buffers;	}
	virtual ERRCODE		GetRecentFile	(int index, LPTSTR name, int nName);
	virtual ERRCODE		AddRecentFile	(LPCTSTR name);
	virtual ERRCODE		GetRecentProject(int index, LPTSTR name, int nName);
	virtual ERRCODE		AddRecentProject(LPCTSTR name);
	virtual ERRCODE		GetFTPUserName	(LPCTSTR& pu);
	virtual ERRCODE		GetFTPPassword	(LPCTSTR& pp);
	virtual ERRCODE		GetFTPAuthorization(bool hadfailed = false);
	virtual PBAFPANEL	GetPanel		(LPCTSTR pPanelName);
	virtual PBAFPANE	GetPane			(LPCTSTR pPanelName, LPCTSTR pPaneName);
	virtual void		DrawStatusItem	(HDC hdc, HWND hWnd);
	virtual void		SetStatus		(LPCTSTR lpText);
	virtual HWND		GetAppWindow	(void)		{ return m_hwnd; }

	virtual void		FitStatusWindows(void);
	virtual bool		HasStatbar		(void)		{ return NULL == BpanelList::FindKey(_T("Status"), m_subpanels); }

protected:
	virtual void		UpdateInfoPanel	 (Bview* pView, BappPanel* pp);
	virtual void		GetStatusInfoSize(int& x, int& y, int& w, int& h, LPRECT rc);
	virtual void		GetStatusLinoSize(int& x, int& y, int& w, int& h, LPRECT rc);
	virtual void		GetStatusTextSize(int& x, int& y, int& w, int& h, LPRECT rc);
	virtual ERRCODE		Encrypt			 (LPTSTR pd, int nd, LPCTSTR ps, int ns, DWORD key);
	virtual ERRCODE		Decrypt			 (LPTSTR pd, int nd, LPCTSTR ps, int ns, DWORD key);

protected:
	TCHAR				m_title[MAX_PATH*2];
	Bpersist*			m_persist;
	BbufList*			m_buffers;
	Blayout				m_layout;
	PBAFPANEL			m_pInfoPanel;
	PBAFPANEL			m_pEditPanel;
	PBAFPANEL			m_pStatusPanel;
	PBAFPANEL			m_pDebugPanel;
	Bview*				m_views;
	Bview*				m_oldviews;
	Bview*				m_curview;
	BpanelList*			m_subpanels;
	Bdbg*				m_dbg;
	Bproject*			m_project;
	Bsccs*				m_sccs;
	TCHAR				m_ftpUser[128];
	TCHAR				m_ftpPassword[128];
	bool				m_ftpAnonymous;
	bool				m_perstPass;
	TEXTENCODING		m_defaultencoding;
	bool				m_showStatbar;
	bool				m_showPicker;
	bool				m_showCalculator;
	bool				m_showFunctions;
	bool				m_showFiles;
	bool				m_showInfo;
	bool				m_showInfoBuild;
	bool				m_showInfoBuildResults;
	bool				m_showInfoDebug;
	bool				m_showInfoGrep;
	bool				m_showInfoGrep2;
	bool				m_showInfoSCCS;
	bool				m_showWatch;
	bool				m_showRegisters;
	bool				m_showStack;
	bool				m_showThreads;
	bool				m_showVariables;
	bool				m_showMemory;
	bool				m_restoreProjects;
	bool				m_suppressTabPanes;
	bool				m_firstdebug;
	PDBENABLE			m_pdbenable;
	HWND				m_hwndStatusInfo;
	HWND				m_hwndStatusLino;
	HWND				m_hwndStatusText;
	int					m_recentFileIndex;
	int					m_maxRecentFiles;
	int					m_recentProjectIndex;
	int					m_maxRecentProjects;
	BrecentList*		m_recentFiles;
	BrecentList*		m_recentProjects;
};


#ifdef _DEBUG
#define BED_FTP_LOGLEV 4
#else
#define BED_FTP_LOGLEV 0
#endif


#define VSIZER_WIDTH	5
#define VSIZER_MARG		1
#define VSTRIP_WIDTH	8

#define HSIZER_HEIGHT	5
#define HSIZER_MARG		1

#define EDITPANE_TOPMARG	0
#define EDITPANE_LEFTMARG	0
#define EDITPANE_RIGHTMARG	0
#define EDITPANE_BOTTOMMARG	0

#define STATBAR_SEP			 2
#define STATBAR_HEIGHT		20
#define STATBAR_TEXTMARG	32

#define STATBAR_INFO_WIDTH	42
#define STATBAR_LINO_WIDTH	116


extern "C"
{
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow);
}


#endif

