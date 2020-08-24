
#include "bedx.h"

#define MAX_TCL_TOKEN 128

//**************************************************************************
BviewTCL::BviewTCL(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel)
		:
		Bview(pBuf, pEditor, pPanel)
{
	AddTags();
}
		
//**************************************************************************
void BviewTCL::AddTags(void)
{
	m_statement_term = _T(';');
	m_macro_prefix = _T('#');
		
	AddComment(_T("#"), tsComment);
		
	AddKeyword(_T("always"), kwBuiltin, true);
	AddKeyword(_T("after"), kwBuiltin, true);
	AddKeyword(_T("alarm"), kwBuiltin, true);
	AddKeyword(_T("append"), kwBuiltin, true);
	AddKeyword(_T("array"), kwBuiltin, true);
	AddKeyword(_T("auto_execok"), kwBuiltin, true);
	AddKeyword(_T("auto_load"), kwBuiltin, true);
	AddKeyword(_T("auto_load_pkg"), kwBuiltin, true);
	AddKeyword(_T("auto_mkindex"), kwBuiltin, true);
	AddKeyword(_T("auto_reset"), kwBuiltin, true);
	AddKeyword(_T("break"), kwBuiltin, true);
	AddKeyword(_T("bsearch"), kwBuiltin, true);
	AddKeyword(_T("case"), kwBuiltin, true);
	AddKeyword(_T("catch"), kwBuiltin, true);
	AddKeyword(_T("catclose"), kwBuiltin, true);
	AddKeyword(_T("catgets"), kwBuiltin, true);
	AddKeyword(_T("catopen"), kwBuiltin, true);
	AddKeyword(_T("ccollate"), kwBuiltin, true);
	AddKeyword(_T("cd"), kwBuiltin, true);
	AddKeyword(_T("cequal"), kwBuiltin, true);
	AddKeyword(_T("chgrp"), kwBuiltin, true);
	AddKeyword(_T("chmod"), kwBuiltin, true);
	AddKeyword(_T("chown"), kwBuiltin, true);
	AddKeyword(_T("chroot"), kwBuiltin, true);
	AddKeyword(_T("cindex"), kwBuiltin, true);
	AddKeyword(_T("clength"), kwBuiltin, true);
	AddKeyword(_T("clock"), kwBuiltin, true);
	AddKeyword(_T("close"), kwBuiltin, true);
	AddKeyword(_T("cmdtrace"), kwBuiltin, true);
	AddKeyword(_T("commandloop"), kwBuiltin, true);
	AddKeyword(_T("concat"), kwBuiltin, true);
	AddKeyword(_T("continue"), kwBuiltin, true);
	AddKeyword(_T("copyfile"), kwBuiltin, true);
	AddKeyword(_T("crange"), kwBuiltin, true);
	AddKeyword(_T("csubstr"), kwBuiltin, true);
	AddKeyword(_T("ctoken"), kwBuiltin, true);
	AddKeyword(_T("ctype"), kwBuiltin, true);
	AddKeyword(_T("dup"), kwBuiltin, true);
	AddKeyword(_T("echo"), kwBuiltin, true);
	AddKeyword(_T("eof"), kwBuiltin, true);
	AddKeyword(_T("error"), kwBuiltin, true);
	AddKeyword(_T("eval"), kwBuiltin, true);
	AddKeyword(_T("exec"), kwBuiltin, true);
	AddKeyword(_T("execl"), kwBuiltin, true);
	AddKeyword(_T("exit"), kwBuiltin, true);
	AddKeyword(_T("expr"), kwBuiltin, true);
	AddKeyword(_T("fblocked"), kwBuiltin, true);
	AddKeyword(_T("fcntl"), kwBuiltin, true);
	AddKeyword(_T("fconfigure"), kwBuiltin, true);
	AddKeyword(_T("file"), kwBuiltin, true);
	AddKeyword(_T("fileevent"), kwBuiltin, true);
	AddKeyword(_T("flock"), kwBuiltin, true);
	AddKeyword(_T("flush"), kwBuiltin, true);
	AddKeyword(_T("for"), kwBuiltin, true);
	AddKeyword(_T("foreach"), kwBuiltin, true);
	AddKeyword(_T("fork"), kwBuiltin, true);
	AddKeyword(_T("format"), kwBuiltin, true);
	AddKeyword(_T("frename"), kwBuiltin, true);
	AddKeyword(_T("fstat"), kwBuiltin, true);
	AddKeyword(_T("ftruncate"), kwBuiltin, true);
	AddKeyword(_T("funlock"), kwBuiltin, true);
	AddKeyword(_T("gets"), kwBuiltin, true);
	AddKeyword(_T("glob"), kwBuiltin, true);
	AddKeyword(_T("global"), kwBuiltin, true);
	AddKeyword(_T("history"), kwBuiltin, true);
	AddKeyword(_T("host_info"), kwBuiltin, true);
	AddKeyword(_T("id"), kwBuiltin, true);
	AddKeyword(_T("if"), kwBuiltin, true);
	AddKeyword(_T("incr"), kwBuiltin, true);
	AddKeyword(_T("info"), kwBuiltin, true);
	AddKeyword(_T("infox"), kwBuiltin, true);
	AddKeyword(_T("interp"), kwBuiltin, true);
	AddKeyword(_T("join"), kwBuiltin, true);
	AddKeyword(_T("keyldel"), kwBuiltin, true);
	AddKeyword(_T("keylget"), kwBuiltin, true);
	AddKeyword(_T("keylkeys"), kwBuiltin, true);
	AddKeyword(_T("keylset"), kwBuiltin, true);
	AddKeyword(_T("kill"), kwBuiltin, true);
	AddKeyword(_T("lappend"), kwBuiltin, true);
	AddKeyword(_T("lassign"), kwBuiltin, true);
	AddKeyword(_T("lempty"), kwBuiltin, true);
	AddKeyword(_T("lgets"), kwBuiltin, true);
	AddKeyword(_T("lindex"), kwBuiltin, true);
	AddKeyword(_T("link"), kwBuiltin, true);
	AddKeyword(_T("linsert"), kwBuiltin, true);
	AddKeyword(_T("list"), kwBuiltin, true);
	AddKeyword(_T("llength"), kwBuiltin, true);
	AddKeyword(_T("lmatch"), kwBuiltin, true);
	AddKeyword(_T("load"), kwBuiltin, true);
	AddKeyword(_T("loadlibindex"), kwBuiltin, true);
	AddKeyword(_T("loop"), kwBuiltin, true);
	AddKeyword(_T("lrange"), kwBuiltin, true);
	AddKeyword(_T("lreplace"), kwBuiltin, true);
	AddKeyword(_T("lsearch"), kwBuiltin, true);
	AddKeyword(_T("lsort"), kwBuiltin, true);
	AddKeyword(_T("lvarcat"), kwBuiltin, true);
	AddKeyword(_T("lvarpop"), kwBuiltin, true);
	AddKeyword(_T("lvarpush"), kwBuiltin, true);
	AddKeyword(_T("max"), kwBuiltin, true);
	AddKeyword(_T("min"), kwBuiltin, true);
	AddKeyword(_T("mkdir"), kwBuiltin, true);
	AddKeyword(_T("nice"), kwBuiltin, true);
	AddKeyword(_T("open"), kwBuiltin, true);
	AddKeyword(_T("package"), kwBuiltin, true);
	AddKeyword(_T("pid"), kwBuiltin, true);
	AddKeyword(_T("pipe"), kwBuiltin, true);
	AddKeyword(_T("pkg_mkIndex"), kwBuiltin, true);
	AddKeyword(_T("proc"), kwBuiltin, true);
	AddKeyword(_T("profile"), kwBuiltin, true);
	AddKeyword(_T("puts"), kwBuiltin, true);
	AddKeyword(_T("pwd"), kwBuiltin, true);
	AddKeyword(_T("random"), kwBuiltin, true);
	AddKeyword(_T("read"), kwBuiltin, true);
	AddKeyword(_T("readdir"), kwBuiltin, true);
	AddKeyword(_T("regexp"), kwBuiltin, true);
	AddKeyword(_T("regsub"), kwBuiltin, true);
	AddKeyword(_T("rename"), kwBuiltin, true);
	AddKeyword(_T("replicate"), kwBuiltin, true);
	AddKeyword(_T("return"), kwBuiltin, true);
	AddKeyword(_T("rmdir"), kwBuiltin, true);
	AddKeyword(_T("scan"), kwBuiltin, true);
	AddKeyword(_T("scancontext"), kwBuiltin, true);
	AddKeyword(_T("scanfile"), kwBuiltin, true);
	AddKeyword(_T("scanmatch"), kwBuiltin, true);
	AddKeyword(_T("seek"), kwBuiltin, true);
	AddKeyword(_T("select"), kwBuiltin, true);
	AddKeyword(_T("server_accept"), kwBuiltin, true);
	AddKeyword(_T("server_create"), kwBuiltin, true);
	AddKeyword(_T("set"), kwBuiltin, true);
	AddKeyword(_T("signal"), kwBuiltin, true);
	AddKeyword(_T("sleep"), kwBuiltin, true);
	AddKeyword(_T("socket"), kwBuiltin, true);
	AddKeyword(_T("source"), kwBuiltin, true);
	AddKeyword(_T("split"), kwBuiltin, true);
	AddKeyword(_T("string"), kwBuiltin, true);
	AddKeyword(_T("subst"), kwBuiltin, true);
	AddKeyword(_T("switch"), kwBuiltin, true);
	AddKeyword(_T("sync"), kwBuiltin, true);
	AddKeyword(_T("system"), kwBuiltin, true);
	AddKeyword(_T("tclPkgSetup"), kwBuiltin, true);
	AddKeyword(_T("tclPkgUnknown"), kwBuiltin, true);
	AddKeyword(_T("tell"), kwBuiltin, true);
	AddKeyword(_T("time"), kwBuiltin, true);
	AddKeyword(_T("times"), kwBuiltin, true);
	AddKeyword(_T("trace"), kwBuiltin, true);
	AddKeyword(_T("translit"), kwBuiltin, true);
	AddKeyword(_T("umask"), kwBuiltin, true);
	AddKeyword(_T("unknown"), kwBuiltin, true);
	AddKeyword(_T("unlink"), kwBuiltin, true);
	AddKeyword(_T("unset"), kwBuiltin, true);
	AddKeyword(_T("unsupported0"), kwBuiltin, true);
	AddKeyword(_T("update"), kwBuiltin, true);
	AddKeyword(_T("uplevel"), kwBuiltin, true);
	AddKeyword(_T("upvar"), kwBuiltin, true);
	AddKeyword(_T("vwait"), kwBuiltin, true);
	AddKeyword(_T("wait"), kwBuiltin, true);
	AddKeyword(_T("while"), kwBuiltin, true);	
}

//**************************************************************************
TokenRet BviewTCL::GetToken(
					LPTSTR&		lpText,
					int&		nText,
					int&		line,
					int&		incol,
					int&		outcol,
					TokenState&	state,
					LPTSTR&		lpToken,
					int&		nToken,
					BkwType&	kw
					)
{
	return Bview::GetToken(lpText, nText, line, incol, outcol, state, lpToken, nToken, kw);
}

//**************************************************************************
ERRCODE BviewTCL::EnumFunctions(EnumFuncsCallback pCallback, LPVOID cookie, Blog* pLog)
{
	ERRCODE		ec;
	TokenRet	tr;
	LPTSTR		lpText, lpToken;
	int			nText,  nToken;
	TCHAR		prevToken[MAX_TCL_TOKEN];
	int			prevNtoken;
	int			braces;
	int			line;
	int			incol, outcol, tokcol;
	int			i;
	TokenState	state;
	BkwType		kw;

	braces	= 0;
	line	= 1;
	prevNtoken = 0;
	
	enum
	{
		psBase, psInFuncDecl, psFuncParms, psInFunc
	}
	parseState	= psBase;	
	state 		= tsBase;

	do
	{
		ec = m_buffer->GetLineText(line, (LPCTSTR&)lpText, nText);
		if(ec != errOK) return errOK;
		
		incol	 = 1;
		outcol	 = 1;

		do
		{
			tokcol = outcol;
			
			tr = GetToken(
						(LPTSTR&)lpText,
						nText,
						line,
						incol,
						outcol,
						state,
						lpToken,
						nToken,
						kw
					);
			
			if(tr != trOK) 
			{
				ec = errOK;
				break;
			}

			if(state != tsSpanningComment && kw != kwComment)
			{
				switch(parseState) 
				{
				case psBase:

					switch(kw)
					{
					case kwPlain:
					case kwMacro:
					case kwOperator:
					case kwBuiltinType:

						switch(lpToken[0])
						{
						case _T('{'):
							braces = 1;
							break;
						case _T('}'):
							braces--;
							if(braces< 0) braces = 0;
							break;
						default:
							if(! KeywordCompare(lpToken, _T("proc"), nToken, 4, true))
							{
								parseState = psInFuncDecl;
								braces = 0;
							}
							break;
						}
						break;

					default:
						break;
					}
					break;
					
				case psInFuncDecl:

					switch(kw)
					{
					case kwPlain:
					case kwOperator:

						switch(lpToken[0])
						{
						case _T('{'):
							ec = pCallback(cookie, prevToken, _T(""), 0, line, efFunction);
							parseState = psFuncParms;
							braces = 1;
							break;
						case _T('}'):
							braces--;
							if(braces < 0) braces = 0;
							break;
						default:
							break;
						}
						break;

					default:

						break;
					}
					break;

				case psFuncParms:

					switch(kw)
					{
					case kwPlain:
					case kwOperator:

						switch(lpToken[0])
						{
						case _T('}'):
							braces--;
							if(prevNtoken)
								ec = pCallback(cookie, prevToken, _T(""), 0, line, efArgument);
							if(braces <= 0)
								parseState = psInFunc;
							break;
						case _T(','):
							if(prevNtoken)
								ec = pCallback(cookie, prevToken, _T(""), 0, line, efArgument);
							break;
						}
						break;

					default:
						break;
					}
					break;
				
				case psInFunc:

					switch(kw)
					{
					case kwPlain:
					case kwOperator:

						switch(lpToken[0])
						{
						case _T('{'):
							braces++;
							break;
						case _T('}'):
							braces--;
							if(braces <= 0)
								parseState = psBase;
							break;
						default:
							break;
						}
						break;

					default:
						break;
					}
					break;		
				}
				if(NonWhite(lpToken, nToken))
				{
					if (nToken > (MAX_TCL_TOKEN - 2))
					{
						nToken = MAX_TCL_TOKEN - 2;
					}
					_tcsncpy(prevToken, lpToken, nToken);
					prevToken[nToken] = _T('\0');
				}
				prevNtoken = nToken;
			}
		}
		while(ec == errOK);

		if(ec != errOK) break;
		line++;

	} while(ec == errOK);

	return errOK;
}

