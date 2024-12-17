#include "bedx.h"

extern "C" {

typedef int	FAR (FAR *PTcl_EvalFile)(
										Tcl_Interp FAR * interp,
										char FAR* string
									);
extern PTcl_EvalFile pTcl_EvalFile;
extern PTcl_EvalFile pTcl_Eval;

typedef int	FAR (FAR *PTcl_Init)	(
										Tcl_Interp FAR * interp
									);
typedef Tcl_Command	(FAR *PTcl_CreateCommand)
									(
										Tcl_Interp FAR * interp,
										char FAR* cmdName,
										Tcl_CmdProc FAR * proc,
										ClientData clientData,
										Tcl_CmdDeleteProc FAR * deleteProc
									);
typedef Tcl_Interp FAR *(FAR *PTcl_CreateInterp)(void);
typedef void FAR (FAR *PTcl_DeleteInterp)(Tcl_Interp FAR * interp);
typedef int FAR (FAR* PTk_GetNumMainWindows)(void);
typedef int FAR (FAR* PTk_DoOneEvent)(int);

}

#define TCL_OK			0
#define TCL_ERROR		1
#define TCL_RETURN		2
#define TCL_BREAK		3
#define TCL_CONTINUE	4

int					Btcl::m_refs			= 0;
BtclCommandList*	Btcl::m_tclCommandFiles = NULL;

bool				Btcl::m_tcl_is_loaded	= false;
bool				Btcl::m_tk_is_loaded	= false;

//**************************************************************************
// function pointers to TCL sytem
//
PTcl_EvalFile			pTcl_EvalFile;
PTcl_EvalFile			pTcl_Eval;
PTcl_Init				pTcl_Init, pTk_Init;
PTcl_CreateCommand		pTcl_CreateCommand;
PTcl_CreateInterp		pTcl_CreateInterp;
PTcl_DeleteInterp		pTcl_DeleteInterp;
PTk_GetNumMainWindows	pTk_GetNumMainWindows;
PTk_DoOneEvent			pTk_DoOneEvent;

//**************************************************************************
Btcl::Btcl(Bview* pView)
		 :
		m_pView(pView),
		m_log(NULL)
{
	m_refs++;

	m_utf8_alloc	= 0;
	m_utf8_buf		= NULL;

	m_evalerror		= false;
	m_tkStack		= 0;
}

//**************************************************************************
Btcl::~Btcl()
{
	if(m_tk_is_loaded)
	{
		while(m_tkStack > 0)
		{
			pTcl_DeleteInterp(m_pTkInterps[m_tkStack]);
			m_pTkInterps[m_tkStack--] = NULL;
		}
	}
	if(m_utf8_buf)
		delete [] m_utf8_buf;

	if(--m_refs == 0)
	{
		if(m_tclCommandFiles)
			m_tclCommandFiles = BtclCommandList::FreeList(m_tclCommandFiles);
	}
}

//**************************************************************************
LPCTSTR Btcl::Utf8Decode(const char* src)
{
#ifdef UNICODE
	int len;

	len = strlen(src) * 2;

	if(len > m_utf8_alloc)
	{
		if(m_utf8_buf) delete [] m_utf8_buf;
		m_utf8_buf = new TCHAR [ len ];
		m_utf8_alloc = len;
	}
	BUtil::Utf8Decode(m_utf8_buf, src);
	return m_utf8_buf;
#else
	return src;
#endif
}

//**************************************************************************
LPCSTR Btcl::Utf8Encode(LPCTSTR src)
{
#ifdef UNICODE
	int len;

	len = _tcslen(src) * 3 + 32;

	if(len > m_utf8_alloc)
	{
		if(m_utf8_buf) delete [] m_utf8_buf;
		m_utf8_buf = new TCHAR [ len ];
		m_utf8_alloc = len;
	}
	BUtil::Utf8Encode((LPSTR)m_utf8_buf, src, false);
	return (LPCSTR)m_utf8_buf;
#else
	return src;
#endif
}

//**************************************************************************
LPCTSTR Btcl::FindTCLfile(LPCTSTR nameIn, LPTSTR nameOut, int& useTk)
{
	TCHAR	fname[MAX_PATH];

	m_pView->UnescapeString(nameOut, nameIn, true);
	_tcscpy(nameOut, fname);

	if(BUtil::FileExists(nameOut) != errOK)
	{
		LPTSTR bcpath;

		// file as specified not there, try same name with ".tcl" appended
		//
		_tcscat(nameOut, _T(".tcl"));
		if(BUtil::FileExists(nameOut) != errOK)
		{
			// no such file in the dir, try environment path
			//
			char* spath = getenv(Utf8Encode(_T("BED_SCRIPT_PATH")));

			if(! spath)
			{
				BUtil::GetHomePath(nameOut, MAX_PATH);
				_tcscat(nameOut, _T("bed"));
				bcpath = nameOut;
			}
			else
			{
				CharToTChar(nameOut, spath);
			}
			_tcscat(nameOut, _PTS_);
			_tcscat(nameOut, nameIn);
			if(BUtil::FileExists(nameOut) != errOK)
			{
				_tcscat(nameOut, _T(".tcl"));
				if(BUtil::FileExists(nameOut) != errOK)
				{
					return NULL;
				}
			}
		}
	}
	useTk = _tcsstr(nameOut, _T(".tk")) || _tcsstr(nameOut, _T(".TK"));
	return nameOut;
}

//**************************************************************************
bool Btcl::IsTCLfile(LPCTSTR lpFilename)
{
	bool istcl = false;

	if(_tcsstr(lpFilename, _T(".tcl")))
		istcl = true;
	else if(_tcsstr(lpFilename, _T(".TCL")))
		istcl = true;
	else if(_tcsstr(lpFilename, _T(".Tcl")))
		istcl = true;
	else if(_tcsstr(lpFilename, _T(".tk")))
		istcl = true;
	else if(_tcsstr(lpFilename, _T(".TK")))
		istcl = true;
	return istcl;
}

//**************************************************************************
bool Btcl::IsTCLcommand(LPCTSTR lpCommandname, LPCTSTR& pCommandPath)
{
	BtclCommandList* pEntry;

	if(! m_tclCommandFiles)
		BuildTCLfileList();
	if(! m_tclCommandFiles)
		return false;
	pEntry = BtclCommandList::FindKey(lpCommandname, m_tclCommandFiles);
	if(pEntry)
	{
		pCommandPath = pEntry->GetValue();
		return true;
	}
	else
	{
		pCommandPath = NULL;
		return false;
	}
}

//**************************************************************************
void Btcl::BuildTCLfileList(void)
{
	HDIRLIST		hDir;
	LPCTSTR			lpFilename;
	bool			isDir, isLink, isReadOnly;
	ERRCODE			ec;
	char*			spath;
	TCHAR			tclPath[MAX_PATH];

	if(m_tclCommandFiles)
		m_tclCommandFiles = BtclCommandList::FreeList(m_tclCommandFiles);

	// enumerate files in current directory
	//
	_tcscpy(tclPath, _T("."));
	_tcscat(tclPath, _PTS_);

	ec = BfileInfo::ListDirectory(hDir, tclPath);

	while(BfileInfo::NextFile(hDir, lpFilename, isDir, isLink, isReadOnly) == errOK)
	{
		if(! isDir && IsTCLfile(lpFilename))
		{
			TCHAR	entry[MAX_PATH];
			LPCTSTR fp;
			LPTSTR  xp;

			fp = BfileInfo::FilePart((LPTSTR)lpFilename);
			if(fp)
			{
				_tcscpy(entry, fp);
				for(xp = entry; *xp && *xp != _T('.');)
					xp++;
				*xp = _T('\0');
				xp = new TCHAR [ _tcslen(fp) + 2 ];
				_tcscpy(xp, fp);
				m_tclCommandFiles = BtclCommandList::AddToList(new BtclCommandList(entry, xp), m_tclCommandFiles);
			}
		}
	}
	BfileInfo::EndListDirectory(hDir);

	// enumerate files in TCL path
	//
	spath = getenv("BED_SCRIPT_PATH");
	if(spath)
	{
		CharToTChar(tclPath, spath);
		_tcscat(tclPath, _PTS_);

		ec = BfileInfo::ListDirectory(hDir, tclPath);

		while(BfileInfo::NextFile(hDir, lpFilename, isDir, isLink, isReadOnly) == errOK)
		{
			if(! isDir && IsTCLfile(lpFilename))
			{
				TCHAR	entry[MAX_PATH];
				LPCTSTR fp;
				LPTSTR  xp;

				fp = BfileInfo::FilePart((LPTSTR)lpFilename);
				if(fp)
				{
					_tcscpy(entry, fp);
					for(xp = entry; *xp && *xp != _T('.');)
						xp++;
					*xp = _T('\0');
					xp = new TCHAR [ _tcslen(fp) + 2 ];
					_tcscpy(xp, fp);
					m_tclCommandFiles = BtclCommandList::AddToList(new BtclCommandList(entry, xp), m_tclCommandFiles);
				}
			}
		}
		BfileInfo::EndListDirectory(hDir);
	}
	return;
}

#if 0
//**************************************************************************
bool Btcl::Check(arEvent& event, DWORD& param)
{
	bool bBusy;

	event = evtNOOP;
	param = 0;

	// do one slice of TCL/TK UI processing
	//
	bBusy = Check();

	if(m_pushedEvent != evtNOOP)
	{
		// UI pushed an event, grab it
		//
		event = m_pushedEvent;
		param = m_pushedParam;

		m_pushedEvent = evtNOOP;
	}
	return bBusy | (event != evtNOOP);
}

//**************************************************************************
bool Btcl::Check(void)
{
	if(m_tkStack > 0)
	{
		int n;

		// if there is no window for this interpreter, pop it off the stack
		//
		if((n = pTk_GetNumMainWindows()) > 0)
		{
			for(int i = 0; i < 8; i++)
			{
				if(! pTk_DoOneEvent(TK_ALL_EVENTS | TK_DONT_WAIT))
				{
					return false; // tcl/tk went quiet
				}
			}
			return false;
		}
		else
		{
			while(m_tkStack > 0)
			{
				pTcl_DeleteInterp(m_pTkInterps[m_tkStack]);
				m_pTkInterps[m_tkStack--] = NULL;
			}
			return false;
		}
	}
	return false;
}
#endif

//**************************************************************************
void Btcl::EvalError(LPCTSTR pMsg1, const char* pMsg2)
{
	if(! m_log) return;
	m_log->Log(logError, 0, _T("%s %s\n"), pMsg1, pMsg2);
	m_evalerror = true;
}

//**************************************************************************
void Btcl::EvalMessage(LPCTSTR pMsg1, LPCTSTR pMsg2)
{
	if(! m_log) return;
	m_log->Log(logInfo, 0, _T("%s %s\n"), pMsg1, pMsg2);
}

// STATIC
//**************************************************************************
int Btcl::TclWrapper(ClientData pThis, Tcl_Interp *pTerp, int argc, const char* argv[])
{
	if(pThis)
		return ((Btcl*)pThis)->TclExec(pTerp, argc, argv);
	return -1;
}

// STATIC
//**************************************************************************
int Btcl::TclBkgError(ClientData pThis, Tcl_Interp *pTerp, int argc, const char* argv[])

{
	if(pThis)
		((Btcl*)pThis)->EvalError(_T("TCL/TK err: "), argc > 0 ? *argv : "");
	return TCL_OK;
}

//**************************************************************************
int Btcl::TclExec(Tcl_Interp *pTerp, int argc, const char* argv[])
{
	Bview*		pView;
	EditCommand cmd;
	ERRCODE		ec;
	const char*	str;
	LPCTSTR		ucsstr;
	int			i, num, len;

	// resolve object to act on
	//
	pView = m_pView;

	if(! pView)
	{
		EvalError(_T("No View to exec TCL"), "");
		return TCL_ERROR;
	}

	// push any args on the stack in reverse order
	//
	while(argc-- > 1)
	{
		str = argv[argc];
		len = strlen(str);

		for(i = 0, num = 1; i < len && num; i++)
			if(str[i] < '0' || str[i] > '9')
				num = 0;
		if(len == 0) num = 0;
		if(num)
		{
			num = strtol(str, NULL, 0);
			pView->PushParm(num, ptNumber);
		}
		else
		{
			ucsstr = Utf8Decode(str);
			pView->PushParm(ucsstr, _tcslen(ucsstr), ptString);
		}
	}
	// decode command name to command code
	//
	cmd = pView->CommandFromName(Utf8Decode(argv[0]));
	if(cmd >= MaxEditCommand || cmd < UnknownCommand)
	{
		// this MUST be a (recursive) TCL invokation, since
		// the only way to get here is if we told tcl about
		// the name, and it isn't a command, so there
		//
		/*printf("TCL Command=%s\n", argv[0]);*/
		ucsstr = Utf8Decode(argv[0]);
		pView->PushParm(ucsstr, _tcslen(ucsstr), ptString);
		ec = pView->Dispatch(ReadCommandFile);
	}
	else
	{
		/*printf("Command=%s\n", CommandNames[cmd - UnknownCommand].name);*/
		ec = pView->Dispatch(cmd);
	}
	if(ec == errOK)
	{
		// for inquire type commands, set return code
		// to top of operand stack
		//
		if(cmd >= UnknownCommand && cmd < MaxEditCommand)
			cmd = (EditCommand)((int)cmd - (int)UnknownCommand);
		else
			return TCL_OK;
#if 0
		if(CommandNames[cmd].cmdType == cmdINQU) {
			if(pObj->pOpandStack) {
				Poperand pOP = stPopOperand(pObj);
				Pu8i tclStrRes = usetempa(256);
				if(pOP->type & stySTRING) {
					tclStrRes = usetempa(pOP->Len + 2);
					strncpy((Pchar)tclStrRes, (Pchar)pOP->val.cval, pOP->Len);
					tclStrRes[pOP->Len] = '\0';
					ip->result = (Pchar)tclStrRes;
					/*printf("set ip result=%s\n", ip->result);*/
				} else if(pOP->type & styNUMBER) {
					sprintf((Pchar)tclStrRes, "%d", pOP->val.ival);
					ip->result = (Pchar)tclStrRes;
				} else {
					ip->result = "operand type error";
					edError("exec return: ", "bad return operand type", pObj);
				}
				stFreeOperand(pOP, pObj);
			}
		}
#endif
		return TCL_OK;
	}
	else
	{
		EvalError(_T("No such command"), argv[0]);
	}
	return TCL_OK;
}

//**************************************************************************
ERRCODE Btcl::Interpret(LPCTSTR pName, bool bFromFile, bool bIsTK)
{
	Tcl_Interp* pInterp;
	bool		isTK;
	char		ebuf[512];
	ERRCODE		ec;

	// see if this is a visual(.tk) versus batch(.tcl)
	//
	isTK = bIsTK || (_tcsstr(pName, _T("tk")) != NULL);

	// load TCL/TK as needed
	//
	if(! m_tcl_is_loaded)
	{
#ifndef STATIC_LINK_TCL

		HDYNLIB hModTcl;
		LPCTSTR pszTcl;

#ifdef Windows
		pszTcl = _T("tcl84.dll");
#else
		pszTcl = _T("libtcl");
#endif
		ec = BUtil::LoadLibrary(hModTcl, pszTcl);

#ifdef Windows
		if(ec != errOK)
		{
			ec = BUtil::LoadLibrary(hModTcl, _T("tcl83.dll"));
		}
#endif
		if(ec == errOK)
		{
			BUtil::ResolveFunction(hModTcl, _T("Tcl_CreateInterp"), (PDYNFUNCTION&)pTcl_CreateInterp);
			BUtil::ResolveFunction(hModTcl, _T("Tcl_Init"), 		(PDYNFUNCTION&)pTcl_Init);
			BUtil::ResolveFunction(hModTcl, _T("Tcl_DeleteInterp"), (PDYNFUNCTION&)pTcl_DeleteInterp);
			BUtil::ResolveFunction(hModTcl, _T("Tcl_Eval"), 		(PDYNFUNCTION&)pTcl_Eval);
			BUtil::ResolveFunction(hModTcl, _T("Tcl_EvalFile"), 	(PDYNFUNCTION&)pTcl_EvalFile);
			BUtil::ResolveFunction(hModTcl, _T("Tcl_CreateCommand"),(PDYNFUNCTION&)pTcl_CreateCommand);
			BUtil::ResolveFunction(hModTcl, _T("Tcl_DoOneEvent"), 	(PDYNFUNCTION&)pTk_DoOneEvent);

			if(pTcl_EvalFile &&	pTcl_CreateCommand && pTcl_CreateInterp && pTcl_DeleteInterp)
			{
				m_tcl_is_loaded = true;
			}
		}
#else
		m_tcl_is_loaded 	= true;
		pTcl_CreateInterp	= Tcl_CreateInterp;
		pTcl_DeleteInterp	= Tcl_DeleteInterp;
		pTcl_Eval			= Tcl_Eval;
		pTcl_EvalFile		= Tcl_EvalFile;
		pTcl_CreateCommand	= Tcl_CreateCommand;
		pTcl_Init			= Tcl_Init;
		pTk_DoOneEvent		= Tcl_DoOneEvent;
#endif
	}
	if(isTK && ! m_tk_is_loaded)
	{
#ifndef STATIC_LINK_TCL

		HDYNLIB hModTk;
		LPCTSTR pszTk;

#ifdef Windows
		pszTk = _T("tk84.dll");
#else
		pszTk = _T("libtk");
#endif
		ec = BUtil::LoadLibrary(hModTk, pszTk);

		if(ec == errOK)
		{
			ec = BUtil::ResolveFunction(hModTk, _T("Tk_Init"),	 		   (PDYNFUNCTION&)pTk_Init);
			ec = BUtil::ResolveFunction(hModTk, _T("Tk_GetNumMainWindows"),(PDYNFUNCTION&)pTk_GetNumMainWindows);
			if(ec == errOK && pTk_Init)
			{
				m_tk_is_loaded = true;
			}
		}
#else
		m_tk_is_loaded = true;
		pTk_Init	   = Tk_Init;
		pTk_GetNumMainWindows = Tk_GetNumMainWindows;
#endif
	}

	if(! m_tcl_is_loaded)
	{
		EvalError(_T("TCL cannot be loaded"), "");
		return errFAILURE;
	}
	m_evalerror = false;
	int rv;

	do // TRY
	{
		BcmdName*		 pCmd;
		BtclCommandList* pCmdFile;

		if((pInterp = pTcl_CreateInterp()) == NULL)
		{
			EvalError(_T("TCL init failed:"), pInterp->result);
			break;
		}
		if(isTK)
		{
			if(! m_tk_is_loaded)
			{
				EvalError(_T("TK cannot be loaded"), "");
				return errFAILURE;
			}
			// Initialize interpreter
			//
			if(pTcl_Init(pInterp) != TCL_OK)
			{
				EvalError(_T("TCL:"), pInterp->result);
				break;
			}
			// Initialize TK
			//
			if(pTk_Init(pInterp) != TCL_OK)
			{
				EvalError(_T("TCL/TK:"), pInterp->result);
				break;
			}
		}

		//pTcl_CreateCommand(pInterp, "error", TclBkgError, (ClientData)this, NULL);
		//pTcl_CreateCommand(pInterp, "bgerror", TclBkgError, (ClientData)this, NULL);

		// Create a TCL command for all the edit commands
		//
		for(pCmd = commandNames; pCmd && pCmd->cmd != MaxEditCommand; pCmd++)
		{
			pTcl_CreateCommand(pInterp, (char*)Utf8Encode(pCmd->name), TclWrapper, (ClientData)this, NULL);
		}
		// Create a TCL command for all the tcl files present
		//
		if(! m_tclCommandFiles)
			BuildTCLfileList();
		for(pCmdFile = m_tclCommandFiles; pCmdFile; pCmdFile = pCmdFile->GetNext(pCmdFile))
		{
			pTcl_CreateCommand(pInterp, (char*)Utf8Encode(pCmdFile->GetName()), TclWrapper, (ClientData)this, NULL);
		}
	}
	while(0); // CATCH

	if(isTK)
	{
		// stack this interpreter
		//
		m_pTkInterps[++m_tkStack] = pInterp;
	}
	if(bFromFile)
	{
#if 0
		// actually run the file
		//
		rv = pTcl_EvalFile(pInterp, (char*)Utf8Encode(pName));
#else
		{
			struct stat finfo;
			char		fname[MAX_PATH];

			TCharToChar(fname, pName);

			if(! stat(fname, &finfo))
			{
				int f, l;

				f = open(fname, O_RDONLY);

				if(f)
				{
					char* fb = new char [ finfo.st_size + 32 ];

					if(fb)
					{
						l = read(f, fb, finfo.st_size);
						if(l > 0)
						{
							fb[l] = 0;
							rv = pTcl_Eval(pInterp, fb);
						}
						delete [] fb;
					}
					close(f);
				}
			}
		}
#endif
	}
	else
	{
		// encode the string UTF8 and run it out of memory
		//
		rv = pTcl_Eval(pInterp, (char*)Utf8Encode(pName));
	}

	if(rv)
	{
		snprintf(ebuf, sizeof(ebuf), "%s Line:%d", pInterp->result, pInterp->errorLine);
		EvalError(_T("TCL Error: "), ebuf);
	}
	else
	{
		EvalMessage(_T("TCL Complete, no errors"), _T(""));
	}

	if(! isTK || m_evalerror)
	{
		pTcl_DeleteInterp(pInterp);

		if(isTK)
		{
			m_pTkInterps[m_tkStack--] = NULL;
		}
	}
	return m_evalerror ? errFAILURE : errOK;
}


