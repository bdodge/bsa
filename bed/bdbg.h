
#ifndef BDBG_H_
#define BDBG_H_

typedef enum { rcGo, rcStop, rcQuit, rcStep, rcStepIn, rcStepOut, rcBreak } BdbgRunCode;
typedef enum { dbgGDB, dbgDEBUG, dbgOTHER } BdbgProgram;
typedef enum { bpProgram, bpData } BbreakType;

// break/watchpoint 
//**************************************************************************
class Bbreak
{
public:
	Bbreak() :	m_name(NULL), m_line(0), 
				m_cond(NULL), m_cnt(0),
				m_enabled(false), m_valid(false),
				m_ref(0), m_index(0),
				m_type(bpProgram),
				m_next(NULL)
			{};
	Bbreak(LPCTSTR file, int line) {};
	~Bbreak() { if(m_name) delete [] m_name; if(m_cond) delete [] m_cond; };

public:
	LPTSTR			m_name;
	int				m_line;
	LPTSTR			m_cond;
	int				m_cnt;
	int				m_ref;
	bool			m_enabled;
	bool			m_valid;
	BbreakType		m_type;
	int				m_index;
	Bbreak*			m_next;
protected:
};

// base class for debug integration
//**************************************************************************
class Bdbg
{
public:
	Bdbg(Bed* pParent, BdbgInfo* pInfo);
	virtual ~Bdbg();

public:
	virtual ERRCODE	DebugWizard			(void);
	virtual ERRCODE	EditSettings		(HWND);

	virtual ERRCODE	GetShell			(LPTSTR pStr, int nStr);
	virtual ERRCODE	GetShellSwitches	(LPTSTR pStr, int nStr);
	virtual ERRCODE	GetShellSep			(LPTSTR pStr, int nStr);
	virtual ERRCODE	GetDebugProgram		(BdbgProgram& prog)		{ prog = m_dbgprog; return errOK; }
	virtual ERRCODE	GetDebugDir			(LPTSTR pStr, int nStr);
	virtual ERRCODE	GetDebugSourceDir	(LPTSTR pStr, int nStr);
	virtual ERRCODE	GetDebugTarget		(LPTSTR pStr, int nStr);
	virtual ERRCODE	GetUseProjectTarget	(bool& usept);

	virtual ERRCODE	SetShell			(LPCTSTR pStr);
	virtual ERRCODE	SetShellSwitches	(LPCTSTR pStr);
	virtual ERRCODE	SetShellSep			(LPCTSTR pStr);
	virtual ERRCODE	SetDebugProgram		(BdbgProgram prog)		{ m_dbgprog = prog; return errOK; }
	virtual ERRCODE	SetDebugDir			(LPCTSTR pStr);
	virtual ERRCODE	SetDebugSourceDir	(LPCTSTR pStr);
	virtual ERRCODE	SetDebugTarget		(LPCTSTR pStr);
	virtual ERRCODE	SetUseProjectTarget	(bool usept);

	virtual ERRCODE	SetTargetFromProject(void);
	virtual ERRCODE	RestoreTarget		(void);

	virtual ERRCODE Init				(void);
	virtual ERRCODE	ConfirmBreak		(Bbreak* pbp);
	virtual ERRCODE AddBreakpoint		(LPCTSTR pName, LPCTSTR pCond, BbreakType type, int line, bool en);
	virtual ERRCODE	RestoreBreakpoints	(Bproject* pProj, bool recheck);
	virtual ERRCODE	SaveBreakpoints		(Bproject* pProj);

	virtual ERRCODE	SetBreak			(LPCTSTR filename, DWORD line, int& refNum);
	virtual ERRCODE	SetBreak			(LPCTSTR filename, LPCTSTR funcname, int& refNum);
	virtual ERRCODE	ClearBreak			(LPCTSTR filename, DWORD line);
	virtual ERRCODE	ClearBreak			(int refNum);
	virtual ERRCODE	ClearAllBreaks		(void);

	virtual ERRCODE	SetWatch			(LPCTSTR varname, int& refNum);
	virtual ERRCODE	ClearWatch			(int refNum);
	virtual ERRCODE	ClearWatch			(LPCTSTR varname);
	virtual ERRCODE	ClearAllWatchs		(void);

	virtual ERRCODE	Run					(BdbgRunCode to);
	virtual bool	GetRunning			(void)					{ return m_running; };

	virtual ERRCODE	OnDebuggerData		(LPCTSTR pLine)			{ return errNOT_IMPLEMENTED;	};


	virtual bool	OnWiz				(int* step, wizMethod method, LPCTSTR typecode, LPCTSTR var, LPTSTR val);

protected:
	virtual ERRCODE	RestoreSettings		(void);
	virtual ERRCODE	SaveSettings		(void);
	static bool		OnDebugSetupData	(LPVOID cookie, int* step, wizMethod method, LPCTSTR var, LPTSTR val);

public:
protected:
	bool			m_running;
	bool			m_started;
	static char		dbgWizScript[];
	Bed*			m_editor;
	Bpersist*		m_persist;
	Bbreak*			m_breaks;
	BdbgInfo*		m_infopane;
	BdbgProgram		m_dbgprog;
	TCHAR			m_dbgshell[MAX_PATH];
	TCHAR			m_dbgswitch[32];
	TCHAR			m_dbgsep[32];
	TCHAR			m_dbgdir[MAX_PATH];
	TCHAR			m_dbgsrcdir[MAX_PATH];
	bool			m_useproject_target;
	TCHAR			m_dbgtarget[MAX_PATH];
};

// debug via GDB
//**************************************************************************
class BdbgGDB : public Bdbg
{
public:
	BdbgGDB(Bed* pParent, BdbgInfo* pInfo);
	virtual ~BdbgGDB();

public:
	virtual ERRCODE Init				(void);

	virtual ERRCODE	SetBreak			(LPCTSTR filename, DWORD line, int& refNum);
	virtual ERRCODE	SetBreak			(LPCTSTR filename, LPCTSTR funcname, int& refNum);
	virtual ERRCODE	ClearBreak			(LPCTSTR filename, DWORD line);
	virtual ERRCODE	ClearBreak			(int refNum);
	virtual ERRCODE	ClearAllBreaks		(void);

	virtual ERRCODE	SetWatch			(LPCTSTR varname, int& refNum);
	virtual ERRCODE	ClearWatch			(int refNum);
	virtual ERRCODE	ClearWatch			(LPCTSTR varname);
	virtual ERRCODE	ClearAllWatchs		(void);

	virtual ERRCODE	Run					(BdbgRunCode to);

public:
	virtual ERRCODE	OnDebuggerData		(LPCTSTR pLine);

protected:
	virtual ERRCODE RunCommand			(LPCSTR cmd);

public:

protected:
	char			m_gdbresp[2048];
	int				nResp;
	TCHAR			m_gdbshell[MAX_PATH];
	TCHAR			m_gdbswitch[32];

};

#endif
