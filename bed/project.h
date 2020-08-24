
#ifndef PROJECT_H_
#define PROJECT_H_

class Bbreak;

#define BED_MAX_BREAKPOINTS	256

//**************************************************************************
class Bproject : public Bthread
{
public:
	Bproject(Bed* pBed);
	~Bproject();

public:
	virtual	LPCTSTR	GetName				(void)	{ return m_name; }
	virtual	bool	GetModified			(void)	{ return m_modified; }
	virtual	bool	IsNew				(void)	{ return m_new; }

	virtual ERRCODE Open				(LPCTSTR pName);
	virtual ERRCODE Save				(void);
	virtual ERRCODE RestoreViews		(void);
	virtual ERRCODE SaveViews			(void);
	virtual ERRCODE RestoreBreakpoint	(int n, Bbreak* pBreak);
	virtual ERRCODE SaveBreakpoint		(int n, Bbreak* pBreak);

	virtual ERRCODE	ProjectWizard		(Bpersist* pPerst);
	virtual ERRCODE ProjectSetup		(HWND hWnd);

	virtual ERRCODE	GetSourceDirectory	(LPTSTR pStr, int nStr);
	virtual ERRCODE	GetBuildDirectory	(LPTSTR pStr, int nStr);
	virtual ERRCODE	GetBuildCommand		(LPTSTR pStr, int nStr);
	virtual ERRCODE	GetBuildTarget		(LPTSTR pStr, int nStr);
	virtual ERRCODE	GetBuildShell		(LPTSTR pStr, int nStr);
	virtual ERRCODE	GetShellSwitches	(LPTSTR pStr, int nStr);
	virtual ERRCODE	GetShellCommandSep	(LPTSTR pStr, int nStr);
	virtual ERRCODE	GetIncludePaths		(LPTSTR pStr, int nStr, bool& useEnvAlso);
	virtual ERRCODE	GetProjectNamePath	(LPTSTR pStr, int nStr);

	virtual	ERRCODE	SetName				(LPCTSTR pStr);
	virtual ERRCODE	SetSourceDirectory	(LPCTSTR pStr);
	virtual ERRCODE	SetBuildDirectory	(LPCTSTR pStr);
	virtual ERRCODE	SetBuildCommand		(LPCTSTR pStr);
	virtual ERRCODE	SetBuildTarget		(LPCTSTR pStr);
	virtual ERRCODE	SetBuildShell		(LPCTSTR pStr);
	virtual ERRCODE	SetShellSwitches	(LPCTSTR pStr);
	virtual ERRCODE	SetShellCommandSep	(LPCTSTR pStr);
	virtual ERRCODE	SetIncludePaths		(LPCTSTR pStr, const bool useEnvAlso);

	virtual Bpdb*	GetPDB				(void)		{ return m_pdb; }

	virtual bool	OnWiz(int* step, wizMethod method, LPCTSTR typecode, LPCTSTR var, LPTSTR val);

	static ERRCODE	FormFilePath(LPCTSTR pName, LPCTSTR pSrcPath, LPTSTR pPath, int nPath);
	static ERRCODE	OpenDefaultProject	(LPCTSTR pPath, Bed* pEditor, bool restoreIt = false, bool forceOpen = false);

protected:
	LPCTSTR			PlatformStr(LPCTSTR src);
	static bool		OnProjSetupData(LPVOID cookie, int* step, wizMethod method, LPCTSTR var, LPTSTR val);

protected:
	Bed*			m_editor;
	Bpdb*			m_pdb;
	Bperstxml*		m_persist; 
	Bbreak*			m_breaks;
	TCHAR			m_name[MAX_PATH];
	TCHAR			m_path[MAX_PATH];
	TCHAR			m_srcdir[MAX_PATH];
	TCHAR			m_blddir[MAX_PATH];
	TCHAR			m_bldcmd[MAX_PATH];
	TCHAR			m_bldtarg[MAX_PATH];
	TCHAR			m_bldshell[MAX_PATH];
	TCHAR			m_bldswitch[MAX_PATH];
	TCHAR			m_bldshellsep[MAX_PATH];
	TCHAR			m_platbuf[MAX_PATH + 128];
	LPTSTR			m_inclPaths;
	bool			m_inclincls;
	bool			m_modified;
	bool			m_usewiz;
	bool			m_new;

	static char		projectScript[];
};

#endif

