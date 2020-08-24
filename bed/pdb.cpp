
#include "bedx.h"

//#define PDB_LOG 1
#ifdef PDB_LOG
	#define PDBLOG m_log->Log
#else
	#define PDBLOG stublog
static void stublog(int type, int level, LPCTSTR fmt, ...)
{
}
#endif

//***********************************************************************
_tag_symbol::_tag_symbol(LPCTSTR name, _tag_symbol *pFile, int line)
{
	f_name			 = new TCHAR [ _tcslen(name)  + 2 ];
	_tcscpy(f_name, name);
	f_file			 = pFile;
	f_line			 = line;
	f_members		 = NULL;
	f_type			 = NULL;
	f_parent		 = NULL;
	f_left = f_right = NULL;
}

//***********************************************************************
_tag_symbol::~_tag_symbol()
{
	delete [] f_name;
	if(f_members)
	{
		BpdbInfo* pMember;

		// remove symbol references from f_members
		// to avoid deleting them
		//
		for(pMember = f_members; pMember; pMember = pMember->GetNext(pMember))
			pMember->SetValue(NULL);
		BpdbInfo::FreeList(f_members);
	}
	f_members = NULL;
}

//***********************************************************************
void _tag_symbol::AddMember(LPCTSTR pMember, _tag_symbol *pFile, int line, _tag_symbol* pSym)
{
	LPBSYM memsym = new Bsym(pMember, pFile, line);
	memsym->f_type = pSym;
	f_members = BpdbInfo::AddToList(new BpdbInfo(pMember, memsym), f_members);
}

//***********************************************************************
void _tag_symbol::SetType(_tag_symbol* pSym)
{
	f_type = pSym;
}

// STATIC
//***********************************************************************
LPBSYM Bpdb::AddSym(LPBSYM sym, LPBSYM& tree)
{
	if(! tree) return tree = sym;

	int cmp = _tcscmp(sym->f_name, tree->f_name);

	if(cmp < 0)
		return AddSym(sym, tree->f_left);
	if(cmp > 0)
		return AddSym(sym, tree->f_right);

	// duplicate addition, replace item?
	//
	if(tree->f_members)
	{
		BpdbInfo* pMember;

		// remove symbol references from f_members
		// to avoid deleting them
		//
		for(pMember = tree->f_members; pMember; pMember = pMember->GetNext(pMember))
			pMember->SetValue(NULL);
		tree->f_members = BpdbInfo::FreeList(tree->f_members);
	}
	tree->f_members = sym->f_members;
	sym->f_members = NULL;
	delete sym;
	return tree;
}

// STATIC
//***********************************************************************
void Bpdb::DumpSym(LPBSYM tree)
{
	if(! tree) return;
	if(tree->f_left)
		DumpSym(tree->f_left);
	DumpSym(tree->f_right);
}

// STATIC
//***********************************************************************
void Bpdb::DeleteSym(LPBSYM sym)
{
	if(! sym) return;
	DeleteSym(sym->f_left);
	sym->f_left = NULL;
	DeleteSym(sym->f_right);
	sym->f_right = NULL;
	delete sym;
}

// STATIC
//***********************************************************************
LPBSYM Bpdb::FindSym(LPCTSTR name, LPBSYM tree)
{
	if(! tree || ! name) return NULL;

	int cmp = _tcscmp(name, tree->f_name);

	if(cmp < 0)
		return FindSym(name, tree->f_left);
	if(cmp > 0)
		return FindSym(name, tree->f_right);
	return tree;
}

// STATIC
//***********************************************************************
LPBSYM Bpdb::FindMember(LPCTSTR name, LPBSYM tree)
{
	BpdbInfo* pMember;
	LPBSYM pSym;
	int cmp;

	if(! tree || ! name) return NULL;

	for (pMember = tree->f_members; pMember; pMember = pMember->GetNext(pMember))
	{
		cmp = _tcscmp(name, pMember->m_pName);
		if (! cmp)
		{
			return pMember->GetValue();
		}
	}

	pSym = FindMember(name, tree->f_left);
	if (! pSym)
	{
		pSym = FindMember(name, tree->f_right);
	}
	return pSym;
}

// STATIC
//***********************************************************************
LPBSYM Bpdb::FindClassSym(LPCTSTR classname, LPCTSTR name, LPBSYM tree)
{
	LPCTSTR sn;
	LPBSYM  fnd;

	if(! tree || ! name) return NULL;

	if(! classname || classname[0] == _T('*'))
	{
		sn = _tcsstr(tree->f_name, _T("::"));
		if(sn)
		{
			sn+= 2;
			if(! _tcscmp(name, sn))
				return tree;
		}
	}
	else
	{
		return NULL; // TODO - use class comparison!!!
	}
	fnd = FindClassSym(classname, name, tree->f_left);
	if(fnd) return fnd;
	fnd = FindClassSym(classname, name, tree->f_right);
	return fnd;
}


//**************************************************************************
Bpdb::Bpdb(LPCTSTR lpRoot)
		:
		m_filestodo(NULL),
		m_filestodonow(NULL),
		m_bufstodo(NULL),
		m_filesym(NULL),
		m_files(NULL),
		m_functions(NULL),
		m_types(NULL),
		m_vars(NULL),
		m_manifests(NULL),
		m_inclPaths(NULL),
		m_log(NULL),
		m_level(0)
{
#ifdef PDB_LOG
	m_log = new Blog(8, logtoFILE, _T("pdb.log"));
#endif
}
//**************************************************************************
Bpdb::~Bpdb()
{
	int wait;

	Stop();

	// wait until callbacks complete
	for(wait = 0; (m_level > 0) && (wait < 100); wait++)
		Bthread::Sleep(100);

	if(m_inclPaths)
		m_inclPaths = BpdbIncls::FreeList(m_inclPaths);
	if(m_files)
		DeleteSym(m_files);
	if(m_functions)
		DeleteSym(m_functions);
	if(m_types)
		DeleteSym(m_types);
	if(m_vars)
		DeleteSym(m_vars);
	if(m_manifests)
		DeleteSym(m_manifests);
	if(m_log)
		delete m_log;
}

//**************************************************************************
ERRCODE Bpdb::AddSourceTree(LPCTSTR lpRoot, LPCTSTR pExtensions)
{
	HDIRLIST	hDir;
	ERRCODE		ec;
	LPCTSTR		lpName;
	bool		isDir, isLink, isReadOnly;

	if((ec = BfileInfo::ListDirectory(hDir, lpRoot)) != errOK)
		return ec;

	while((ec = BfileInfo::NextFile(hDir, lpName, isDir, isLink, isReadOnly)) == errOK)
	{
		if(isDir)
		{
			if(_tcslen(lpName) > 0)
			{
				TCHAR npath[MAX_PATH];
				TCHAR ct;

				ct = lpName[_tcslen(lpName) - 1];
				if(ct != _T('.'))
				{
					_tcscpy(npath, lpName);
					if(ct != _PTC_)
						_tcscat(npath, _PTS_);
					
					// recurse tree
					ec = AddSourceTree(npath, pExtensions);
				}
			}
		}
		else
		{
			// match this file against each pattern in extensions
			// where each pattern is like "*.ext"
			//
			bool matched = false;
			LPCTSTR pp = pExtensions;
			LPCTSTR pe;
			LPCTSTR  pRem;
			TCHAR   pat[128];
			int		i;
		
			do
			{
				// extract any pattern up till next ; or ,
				//
				while (*pp && (*pp == _T(' ') || *pp == '\t'))
					pp++;			
				for(pe = pp, i = 0; *pe && *pe != _T(';') && *pe != _T(',') && *pe != _T(' ') && i < 127; i++)
					pat[i] = *pe++;
				pat[i] = _T('\0');
				pp = pe + 1;
				while (*pp && (*pp == _T(' ') || *pp == '\t'))
					pp++;
				
				// if the pattern matches files extension, all set
				//
				pRem = BUtil::SimplePatternMatch(lpName, pat);

				/*
				_tprintf(_T("compare="_Pfs_" pat "_Pfs_"= res=%d\n"),
						lpName, pat, pRem ? 1 : 0);
				*/
				if(pRem && ! *pRem)
				{
					//_tprintf(_T("Add "_Pfs_" cause "_Pfs_"\n"), lpName, pat);
					AddFile(lpName);
				}
			}
			while(! matched && *pp && (*pe == _T(',') || *pe == _T(';')));
			
			if (matched)
			{
				AddFile(lpName, false);
			}
		}
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bpdb::AddFile(LPCTSTR lpFilename, bool suppressCheck)
{
	BpdbTodo* pTodo;
	Block	  lock(&m_todex);

	PDBLOG(logDebug, 4, _T("PDB Adding File TODO: "_Pfs_"\n"), lpFilename);

	pTodo = new BpdbTodo(lpFilename, NULL);
	if(! suppressCheck)
	{
		m_filestodo = BpdbTodo::AddToList(pTodo, m_filestodo);
	}
	else
	{
		m_filestodonow = BpdbTodo::AddToList(pTodo, m_filestodonow);
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bpdb::AddFile(Bbuffer* pBuf)
{
	BpdbBufs* pTodo;
	Block	  lock(&m_todex);

	PDBLOG(logDebug, 4, _T("PDB Adding Buffer TODO: "_Pfs_"\n"), pBuf->GetName());

	// add name to list of files seen
	AddSym(new Bsym(pBuf->GetName(), NULL, 0), m_files);

	pTodo = new BpdbBufs(pBuf->GetName(), pBuf);
	m_bufstodo = BpdbBufs::AddToList(pTodo, m_bufstodo);
	return errOK;
}

//**************************************************************************
ERRCODE Bpdb::AddIncludePath(LPCTSTR pInclPath)
{
	BpdbIncls* pIncl;

	PDBLOG(logDebug, 6, _T("PDB Adding Incl Path: "_Pfs_"\n"), pInclPath);

	pIncl = new BpdbIncls(pInclPath, NULL);
	m_inclPaths = BpdbIncls::AddToList(pIncl, m_inclPaths);
	return errOK;
}

//**************************************************************************
ERRCODE Bpdb::SetIncludePaths(LPCTSTR lpIncls, bool inclIncls)
{
	TCHAR   path[MAX_PATH];
	LPCTSTR pp;
	LPTSTR  pd;

	if(inclIncls)
	{
		// add common include file paths
		//

		// add paths from the environment
		//
	}
	// parse delimter separated list of paths
	// into individual paths and add each
	//
	pp = lpIncls;
	if(! pp) return errBAD_PARAMETER;

	while(*pp)
	{
		for(pd = path; ((pd - path) < MAX_PATH-1) && *pp && *pp != _T(';') && *pp != _T(',');)
			*pd++ = *pp++;
		*pd = _T('\0');
		if(*pp) pp++;

		if(pd > path)
			AddIncludePath(path);
	}
	return errOK;
}

//**************************************************************************
LPBSYM Bpdb::GetBaseType(LPCTSTR pTypename)
{
	LPBSYM pSym;
	int    iter;

	pSym = FindSym(pTypename, m_types);
	if(! pSym) return NULL;

	// this is a type, see if there is a referred type
	// in the member
	//
	iter = 30;
	while(pSym->f_type && iter-- > 0)
	{
		pSym = pSym->f_type;
	}
	return pSym;
}

// Mutually RECURSIVE
//**************************************************************************
ERRCODE Bpdb::Explore(LPCTSTR pFilename, bool checkDone)
{
	Bbuffer* pBuf;
	LPBSYM	 pSym;
	ERRCODE  ec;

	if(checkDone)
	{
		// ignore files seen already (todo - check time stamp?)
		//
		if((pSym = FindSym(pFilename, m_files)) != NULL)
		{
			PDBLOG(logDebug, 8, _T("PDB Already Explored "_Pfs_"\n"), pSym->f_name);
			return errOK;
		}
	}
	// let it be known we've done this file
	//
	pSym = AddSym(new Bsym(pFilename, NULL, 0), m_files);

	PDBLOG(logDebug, 7, _T("PDB Exploring  File: "_Pfs_"\n"), pFilename);

	// open a private buffer on this file, read it, and explore the buffer
	//
	pBuf = Bed::ProperBufferForFile(pFilename, btAny, (TEXTENCODING)-1, (TEXTENCODING)-1, NULL);
	ec = pBuf->Read();
	if(ec == errOK)
	{
		ec = Explore(pBuf, pSym);
	}
	delete pBuf;
	return ec;
}

typedef struct tag_enum_ctx
{
	Bpdb *self;
	Bsym *file;
}
ENCTX, *LPENCTX;

// Mutually RECURSIVE, depth protected
//**************************************************************************
ERRCODE Bpdb::Explore(Bbuffer* pBuf, LPBSYM pFileSym)
{
	Bview*	pView;
	ERRCODE	ec = errOK;

	m_level++;

	if(! m_running) return errFAILURE;

	PDBLOG(logDebug, 4, _T("PDB Exploring: "_Pfs_" (%d deep)\n"), pBuf->GetName(), m_level);

	// create a view on the buffer of the appropriate type
	//
	pView = Bed::ProperViewForBuffer(pBuf, NULL, NULL);

	m_pSym   = NULL;
	m_pClass = NULL;

	if(m_running)
	{
		LPENCTX ectx;

		// do the full enumeration dance on this file
		//
		ectx = new ENCTX();
		ectx->self = this;
		ectx->file = pFileSym;

		ec = pView->EnumFunctions(OnEnumFuncStub, ectx, m_log);

		delete(ectx);
	}
	// dont let buffer go down with the view
	//
	pView->SetBuffer(NULL);
	delete pView;
	m_level--;

	return ec;
}

// STATIC
//**************************************************************************
ERRCODE Bpdb::OnEnumFuncStub(LPVOID cookie, LPCTSTR pName, LPCTSTR pTypeName, int isPtr, int line, EnumFuncsType type)
{
	if(! cookie) return errBAD_PARAMETER;

	Bpdb* self = ((LPENCTX)cookie)->self;
	Bsym* file = ((LPENCTX)cookie)->file;

	self->m_filesym = file;
	return self->OnEnumFunc(pName, pTypeName, isPtr, line, type);
}

//**************************************************************************
ERRCODE Bpdb::OnEnumFunc(LPCTSTR pName, LPCTSTR pTypeName, int isPtr, int line, EnumFuncsType type)
{
	TCHAR  tmp[MAX_PATH];
	LPBSYM pType, pDefType;
	bool   useSysPath;
	bool   isForward;

	if(! m_running) return errFAILURE;

	switch(type)
	{
	case efInclude:

		// strip quotes or braces, prepend all include paths
		// and if file exists, explore it as well
		//
		if(pName)
		{
			TCHAR		path[MAX_PATH];
			BpdbIncls*	pIncl;
			int			plen = _tcslen(pName);

			if(plen >= MAX_PATH)
				plen = MAX_PATH - 1;

			if(pName[0] == _T('\"'))
			{
				if(plen >= 2) plen -= 2;
				else          plen--;
				_tcsncpy(tmp, pName + 1, plen);
				tmp[plen] = _T('\0');
				useSysPath = false;
			}
			else if(plen > 0)
			{
				useSysPath = true;
				_tcsncpy(tmp, pName, plen);
			}
			else
			{
				tmp[0] = _T('\0');
			}
			pName = tmp;

			if(pName[0] == _PTC_ || (pName[0] != _T('\0') && pName[1] == _T(':')))
			{
				// absolute path
				if(BUtil::FileExists(pName) == errOK)
					Explore(pName, true);
			}
			else
			{
				// relative path
				if(BUtil::FileExists(pName) == errOK)
				{
					Explore(pName, true);
				}
				else
				{
					for(pIncl = m_inclPaths; pIncl; pIncl = pIncl->GetNext(pIncl))
					{
						int len;

						path[0] = _T('\0');
						_tcsncpy(path, pIncl->GetName(), MAX_PATH - 3);
						path[MAX_PATH - 3] = _T('\0');
						len = _tcslen(path);
						if(len > 0)
						{
							if(path[len-1] != _PTC_)
							{
								path[len++]   = _PTC_;
								path[len] = _T('\0');
							}
						}
						_tcsncpy(path + len, pName, MAX_PATH - len - 1);
						path[MAX_PATH - 1] = _T('\0');

						if(BUtil::FileExists(path) == errOK)
						{
							Explore(path, true);
							break;
						}
					}
				}
			}
		}
		break;

	case efTypeName:

		// add a named entry using current type
		//
		if(m_pClass)
		{
			if(! _tcscmp(m_pClass->f_name, _T("<forward>")))
			{
				delete [] m_pClass->f_name;
				m_pClass->f_name = new TCHAR [ _tcslen(pName) + 2 ];
				_tcscpy(m_pClass->f_name, pName);
				m_pClass = AddSym(m_pClass, m_types);
				PDBLOG(logDebug, 6, _T("PDB   Identify current class as "_Pfs_"\n"), pName);
			}
			else
			{
				pType = AddSym(new Bsym(pName, m_filesym, line), m_types);
				pType->SetType(m_pClass);
				PDBLOG(logDebug, 6, _T("PDB   Point type "_Pfs_" to existing class "_Pfs_"\n"), pName, m_pClass->f_name);
			}
		}
		else if(pTypeName)
		{
			pDefType = FindSym(pTypeName, m_types);
			if(pDefType)
			{
				pType = AddSym(new Bsym(pName, m_filesym, line), m_types);
				pType->SetType(pDefType);
				PDBLOG(logDebug, 6, _T("PDB   Point type "_Pfs_" to existing type "_Pfs_"\n"), pName, pDefType->f_name);
			}
			else
			{
				PDBLOG(logDebug, 6, _T("PDB   type "_Pfs_" is baseless since "_Pfs_" missing\n"), pName, pTypeName);
			}
		}
		else
		{
			PDBLOG(logDebug, 6, _T("PDB   type "_Pfs_" is baseless\n"), pName);
		}
		break;

	case efVar:

		// lookup the type name of the var
		// to get a pointer to existing type
		//
		if(! pTypeName) pTypeName = _T("<forward>");

		if(! _tcsncmp(pTypeName, _T("class"), 5))
		{
			// skip over "class" in typename
			pTypeName += 5;
		}
		else if(! _tcsncmp(pTypeName, _T("struct"), 6))
		{
			// skip over "struct" in typename
			pTypeName += 6;
		}
		// skip leading white space in typename too
		//
		while(*pTypeName == ' ' || *pTypeName == '\t' || *pTypeName == '*')
			pTypeName++;

		// see if know this type
		pType = FindSym(pTypeName, m_types);
		if(pType)
		{
			PDBLOG(logDebug, 6, _T("PDB   Defining "_Pfs_" using existing type "_Pfs_"\n"),	pName, pType->f_name);

			// yes, this is a new var based on an existing
			// type, so setup type ptr to it
			//
			pDefType = pType;
		}
		else
		{
			PDBLOG(logDebug, 6, _T("PDB   Defining Forward Type "_Pfs_"\n"), pTypeName);

			// this is a new type based on an unknown type
			// perhaps a forward reference?
			//
			pDefType = AddSym(new Bsym(pTypeName, m_filesym, line), m_types);
		}
		if(m_pClass)
		{
			// The var is in a class, so it is a member var
			//
			m_pClass->AddMember(pName, m_filesym, line, pType);
			PDBLOG(logDebug, 8, _T("PDB   Add Member "_Pfs_" of type "_Pfs_"\n"), pName, pType ? pType->f_name : _T("??"));
		}
		else
		{
			// the var is a regular var
			//
			pType = AddSym(new Bsym(pName, m_filesym, line), m_vars);
			pType->f_isptr = isPtr != 0;
			if(pDefType)
				pType->SetType(pDefType);
			PDBLOG(logDebug, 6, _T("PDB   Add var "_Pfs_" based on "_Pfs_"\n"), pName, (pDefType ? pDefType->f_name : _T("<nil>")));
		}
		break;

	case efFunction:

		// look up type
		//
		if(! pTypeName) pTypeName = _T("int");
		pType = FindSym(pTypeName, m_types);

		if(m_pClass)
		{
			TCHAR mangledName[MAX_PATH*2];

			// mangle the name
			_sntprintf(mangledName, MAX_PATH*2, _T(""_Pfs_"::"_Pfs_""),
				m_pClass->f_name, pName);
			mangledName[MAX_PATH*2 - 1] = _T('\0');
			// add name mangled func to sym tab
			m_pSym = AddSym(new Bsym(mangledName, m_filesym, line), m_functions);
		}
		else
		{
			// add global name func to sym tab
			m_pSym = AddSym(new Bsym(pName, m_filesym, line), m_functions);
		}
		// if type is in table link type of func (return type)
		if(m_pSym)
			m_pSym->f_type = pType;

		// link simple member func entry to actual (mangled) def of symbol
		if(m_pClass)
			m_pClass->AddMember(pName, m_filesym, line, m_pSym);

#ifdef PDB_LOG
		PDBLOG(logDebug, 6, _T("PDB Add "_Pfs_"Function "_Pfs_" returning "_Pfs_"\n"),
				(m_pClass ? _T("member ") : _T("")),
				m_pSym->f_name,
				(pType ? pType->f_name : _T("??")));
#endif
		break;

	case efArgument:

		// lookup the type name of the argument var
		// to get a pointer to existing type
		//
		pDefType = NULL;
		if(pTypeName && pTypeName[0])
		{
			if(! _tcsncmp(pTypeName, _T("class"), 5))
			{
				// skip over "class" in typename
				pTypeName += 5;
			}
			else if(! _tcsncmp(pTypeName, _T("struct"), 6))
			{
				// skip over "struct" in typename
				pTypeName += 6;
			}
			// see if know this type
			pType = FindSym(pTypeName, m_types);
			if(pType)
			{
				PDBLOG(logDebug, 6, _T("PDB   Arg "_Pfs_" declared with existing type "_Pfs_"\n"),	pName, pType->f_name);

				// yes, this is a new var based on an existing
				// type, so setup type ptr to it
				//
				pDefType = pType;
			}
			else if(pTypeName[0])
			{
				PDBLOG(logDebug, 6, _T("PDB   "_Pfs_" is not a type, adding since used for arg "_Pfs_"\n"), pTypeName, pName);

				// this is a new type based on an unknown type
				// perhaps a forward reference?
				//
				pDefType = AddSym(new Bsym(pTypeName, m_filesym, line), m_types);
			}
		}
		if(m_pSym)
		{
			m_pSym->AddMember(pName, m_filesym, line, pDefType);
			PDBLOG(logDebug, 8, _T("PDB   Add Function Arg "_Pfs_"\n"), pName);
		}
		break;

	case efClass:
	//case efStruct:
	case efInterface:

		if(! pName)
		{
			pName = _T("<forward>");
			isForward = true;
		}
		else
		{
			isForward = false;
		}
		PDBLOG(logDebug, 8, _T("PDB -- Add Class "_Pfs_"\n"), pName);
		m_pClass = new Bsym(pName, m_filesym, line);
		if(! isForward)
			m_pClass = AddSym(m_pClass, m_types);
		break;

	case efEndClass:
	//case efEndStruct:
	case efEndInterface:

		m_pClass = NULL;
		break;

	case efSetClass:

		if(! pName || ! pName[0])
		{
			PDBLOG(logDebug, 8, _T("PDB -- Reset current class\n"));
			m_pClass = NULL;
			break;
		}
		pType = FindSym(pName, m_types);
		if (pType) {
			PDBLOG(logDebug, 8, _T("PDB -- Reset current class to "_Pfs_"\n"), pName);
			m_pClass = pType;
		}
		else
		{
			PDBLOG(logDebug, 8, _T("PDB -- Can not reset current Class "_Pfs_"\n"), pName);
		}
		break;

	default:

		break;
	}
	if(m_running)
		return errOK;
	else
		return errFAILURE;
}

// STATIC
//**************************************************************************
ERRCODE WINAPI Bpdb::RunStub(LPVOID thisptr)
{
	return ((Bpdb*)thisptr)->ExploreThread();
}

//**************************************************************************
bool Bpdb::ExploreBusy(void)
{
	return m_running && m_filestodo != NULL;
}

//**************************************************************************
ERRCODE Bpdb::ExploreThread()
{
	BpdbTodo* pFileTodo;
	BpdbBufs* pBufTodo;

	PDBLOG(logDebug, 4, _T("PDB Starting"));

#if 0
	// set priority lower than main threads
	//
	#ifdef Windows
		SetThreadPriority(m_hThread, THREAD_PRIORITY_LOWEST);
	#endif
#endif
	while(m_running)
	{
		// is there any new buffer to explore ?
		//
		pBufTodo = m_bufstodo;
		m_level  = 0;

		if(pBufTodo && m_running)
		{
			LPBSYM pFileSym;

			Block lock(&m_todex);

			pFileSym = FindSym(pBufTodo->GetValue()->GetName(), m_files);

			// explore this buffer
			Explore(pBufTodo->GetValue(), pFileSym);

			// remove file from todo list
			m_bufstodo = BpdbBufs::RemoveFromList(pBufTodo, m_bufstodo);
			pBufTodo->SetValue(NULL); // dont delete buffer
			delete pBufTodo;
		}
		// is there any new file to explore ?
		//
		pFileTodo = m_filestodo;
		m_level   = 0;

		if(pFileTodo && m_running)
		{
			{
				Block lock(&m_todex);

				// remove file from todo list
				m_filestodo = BpdbTodo::RemoveFromList(pFileTodo, m_filestodo);
			}
			// explore this file
			Explore(pFileTodo->GetName(), true);

			delete pFileTodo;
		}
		// is there any new file just written to explore ?
		//
		pFileTodo = m_filestodonow;
		m_level   = 0;

		if(pFileTodo && m_running)
		{
			{
				Block lock(&m_todex);

				// remove file from todo list
				m_filestodonow = BpdbTodo::RemoveFromList(pFileTodo, m_filestodonow);
			}
			// explore this file
			Explore(pFileTodo->GetName(), false);

			delete pFileTodo;
		}
		m_level++;
		if(m_running && ! m_bufstodo && ! m_filestodo)
			Bthread::Sleep(100);
		m_level--;
	}
	m_level = 0;
	return errOK;
}

