
#ifndef _PDB_H_
#define _PDB_H_ 1

typedef enum
{
		efInclude,
		efFunction, efArgument,
		efClass, efInterface,
		efTypeName,
		efVar,
		efSetClass,
		efEndClass, efEndInterface
}
EnumFuncsType;

typedef ERRCODE (*EnumFuncsCallback)(LPVOID cookie, LPCTSTR pName, LPCTSTR pTypeName, int isPtr, int line, EnumFuncsType type);

class BpdbInfo;

// binary tree symbol table
//
//***********************************************************************
typedef struct _tag_symbol
{
public:
	_tag_symbol(LPCTSTR name, _tag_symbol *pFile, int line);
	~_tag_symbol();
	void			AddMember			(LPCTSTR pMember, _tag_symbol* pFile, int line, _tag_symbol* pSym = NULL);
	void			SetType				(_tag_symbol* pSym);

public:
	LPTSTR					f_name;
	int						f_line;
	bool					f_isptr;
	BpdbInfo*				f_parent;
	BpdbInfo*				f_members;
	struct _tag_symbol*		f_type;
	struct _tag_symbol*		f_file;

	struct _tag_symbol*		f_left;
	struct _tag_symbol*		f_right;
}
Bsym, BSYM, *LPBSYM;

//list of args, or members
KEY_VAL_LIST_TEMPLATE(BpdbInfo, LPBSYM, NO_API); // list of member or function info

//**************************************************************************

KEY_ARRYVAL_LIST_TEMPLATE(BpdbTodo,  LPTSTR,	NO_API); // list of files to work on
KEY_ARRYVAL_LIST_TEMPLATE(BpdbIncls, LPTSTR,	NO_API); // list of include file paths
KEY_VAL_LIST_TEMPLATE(BpdbBufs,  Bbuffer*,	NO_API); // list of buffers to work on


// program data base object
//
//***********************************************************************
class Bpdb : public Bthread
{
public:
	Bpdb							(LPCTSTR lpRoot);
	virtual ~Bpdb					();

public:
	virtual ERRCODE AddIncludePath	(LPCTSTR pInclPath);
	virtual ERRCODE	SetIncludePaths	(LPCTSTR lpInclPaths, bool inclIncls);
	virtual ERRCODE	AddSourceTree	(LPCTSTR lpRoot, LPCTSTR extensions = _T("*.*"));
	virtual ERRCODE	AddFile			(LPCTSTR lpFilename, bool suppressCheck = false);
	virtual ERRCODE	AddFile			(Bbuffer* pBuf);
	virtual bool	ExploreBusy		(void);

public:
	static LPBSYM	AddSym			(LPBSYM sym, LPBSYM& tree);
	static void		DumpSym			(LPBSYM tree);
	static void		DeleteSym		(LPBSYM sym);
	static LPBSYM	FindSym			(LPCTSTR name, LPBSYM tree);
	static LPBSYM	FindClassSym	(LPCTSTR classname, LPCTSTR name, LPBSYM tree);
	static LPBSYM	FindMember		(LPCTSTR name, LPBSYM tree);

	virtual LPBSYM	GetTypes		(void)	{ return m_types;		}
	virtual LPBSYM	GetFunctions	(void)	{ return m_functions;	}
	virtual LPBSYM	GetVars			(void)	{ return m_vars;		}
	virtual LPBSYM	GetBaseType		(LPCTSTR pTypename);

	static ERRCODE	OnEnumFuncStub	(LPVOID cookie, LPCTSTR pName, LPCTSTR pTypeName, int isPtr, int line, EnumFuncsType type);
	static
	 ERRCODE WINAPI	RunStub			(LPVOID thisptr);

protected:
	virtual ERRCODE Explore			(LPCTSTR pFilename, bool checkDone);
	virtual ERRCODE Explore			(Bbuffer* pBuf, LPBSYM pFileSym);
	virtual ERRCODE	ExploreThread	(void);
	virtual ERRCODE OnEnumFunc		(LPCTSTR pName, LPCTSTR pTypeName, int isPtr, int line, EnumFuncsType type);

protected:
	// list of func, class, etc. decls
	LPBSYM			m_functions;
	LPBSYM			m_types;
	LPBSYM			m_vars;
	LPBSYM			m_manifests;

	// current symbols
	LPBSYM			m_pSym;
	LPBSYM			m_pType;
	LPBSYM			m_pClass;

	// sorted list of files already explored
	LPBSYM			m_files;

	// current file being explored (symbol version)
	LPBSYM			m_filesym;

	// list of files to explore
	BpdbTodo*		m_filestodo;
	BpdbTodo*		m_filestodonow;
	BpdbBufs*		m_bufstodo;
	// mutex to serialize access to todo list
	Bmutex			m_todex;

	// file currently being explored
	TCHAR			m_file[MAX_PATH];

	// recursion level
	int				m_level;

	// flag that there is a current subscriber
	bool			m_enumerating;

	// list of include file paths to use (in order!)
	BpdbIncls*		m_inclPaths;

	Blog*			m_log;

};

#endif /* PDB_H_ */
