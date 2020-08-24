
#include "bedx.h"

// This set of functions is responsible for taking apart one "C" or "CPP"
// type source file and enumerating any declarations of functions, structures
// classes, and typedefs.  It also reports any inclusions of other files and
// reports arguments to functions with types
//
// The callback types are defined in pdb.h
//
//
#define MAX_C_TOKEN 256

//#define PDB_LOG 1

//**************************************************************************
ERRCODE BviewC::NextToken(BefState* state, LPTSTR pToken, int ccToken, BkwType& kwType)
{
	ERRCODE		ec = errOK;
	TokenRet	tr;
	LPTSTR		pTok;
	int			nToken;

	do
	{
		if(! state->m_pToken)
		{
			// no current line to parse, get next line
			//
			ec = m_buffer->GetLineText(state->m_tokLine, (LPCTSTR&)state->m_pToken, state->m_nTokText);
			if(ec != errOK)
			{
				pToken[0] = _T('\0');
				state->m_pToken = NULL;
				return ec;
			}
			state->m_tokOutCol	= 1;
			state->m_tokInCol	= 1;
		}
		do
		{
			tr = GetToken(
							state->m_pToken,
							state->m_nTokText,
							state->m_tokLine,
							state->m_tokInCol,
							state->m_tokOutCol,
							state->m_tokState,
							pTok,
							nToken,
							kwType
					);
			if(tr == trOK || tr == trEOLLINE) 
			{
				if(nToken < ccToken && state->m_tokState != tsSpanningComment && kwType != kwComment)
				{
					// trim trailing white space
					//
					while(nToken > 0 && (pTok[nToken-1] == _T(' ') || pTok[nToken-1] == _T('\t')))
					{
						nToken--;
					}
					if(nToken > 0)
					{
						// trim leading white space
						//
						while(nToken > 0 && (*pTok == _T(' ') || *pTok == _T('\t')))
						{
							pTok++;
							nToken--;
						}
						if(nToken > 0)
						{
							if(nToken >= ccToken)
								nToken = ccToken - 1;
							_tcsncpy(pToken, pTok, nToken);
							pToken[nToken] = _T('\0');
							if(kwType == kwPlain)
								kwType = KeyWord(pToken, nToken);
							return errOK;
						}
						else if(tr == trEOLLINE)
						{
							kwType = kwSpecial;
							pToken[0] = _T('\0');
							state->m_pToken = NULL;
							state->m_tokLine++;
							return errOK;
						}
					}
					else if(tr == trEOLLINE)
					{
						kwType = kwSpecial;
						pToken[0] = _T('\0');
						state->m_pToken = NULL;
						state->m_tokLine++;
						return errOK;
					}
				}
				else if(nToken >= ccToken)
				{
					; //OutputDebugString(pToken);
				}
			}
		}
		while(tr == trOK);
		state->m_pToken = NULL;
		state->m_tokLine++;
	}
	while(ec == errOK);
	pToken[0] = _T('\0');
	return ec;
}

//**************************************************************************
ERRCODE BviewC::ParseStatement(BefState* state, LPCTSTR pFirst, bool& popscope)
{
	TCHAR   token[MAX_C_TOKEN];
	BkwType kw;
	ERRCODE ec;
	int     braceCnt = 0;
	int     semiCnt  = 0;
	int     brackCnt = 0;
	bool	isFor	 = pFirst && ! _tcscmp(pFirst, _T("for"));

	state->m_typedefPending = false;
	popscope = false;

	if (pFirst && pFirst[0] == '[')
		brackCnt = 1;
	
	do
	{
		ec = NextToken(state, token, MAX_C_TOKEN, kw);
		if(ec == errOK)
		{
			if(state->m_log) state->m_log->Log(logDebug, 4, _T("prs Statement tok="_Pfs_"\n"), token);

			switch(kw)
			{
			case kwOperator:
				if(token[0] == _T('{'))
				{
					return ParseScope(state, false);
				}
				if(token[0] == _T(';'))
				{
					if(! isFor)
						return errOK;
					else
						semiCnt++;
				}
				else if(token[0] == _T(')'))
				{
					if(semiCnt > 1)
						isFor = false;
				}
				else if(token[0] == _T(':'))
				{
					if(! _tcscmp(pFirst, _T("case")))
						return errOK;
				}
				else if(token[0] == _T('}'))
				{
					popscope = true;
					return errOK;
				}
				else if (token[0] == _T('['))
				{
					brackCnt++;
				}
				else if (token[0] == _T(']'))
				{
					// hack for c# attributes
					if (--brackCnt == 0)
						return errOK;
				}
				break;
			case kwPlain:
			case kwBuiltinType:
			case kwAddonType:
			case kwQuoted:
			case kwBuiltin:
			case kwBuiltinFunc:
			case kwAddonFunc:
			case kwMacro:
			case kwComment:
			default:
				break;
			}
		}
	}
	while(ec == errOK);
	return ec;
}

//**************************************************************************
ERRCODE BviewC::ParsePreProc(BefState* state, LPCTSTR pDirective)
{
	TCHAR   token[MAX_C_TOKEN];
	TCHAR   target[MAX_C_TOKEN];
	TCHAR   directive[MAX_C_TOKEN + 2];
	BkwType kw;
	ERRCODE ec;
	bool	noDirective;

	if(! pDirective || ! pDirective[0] || ! pDirective[1])
	{
		directive[0] = m_macro_prefix;
		directive[1] = _T('\0');
		noDirective = true;
		pDirective = directive;
	}
	else
	{
		noDirective = false;
	}
	target[0] = '\0';
	do
	{
		ec = NextToken(state, token, MAX_C_TOKEN, kw);
		if(ec == errOK)
		{
			if(state->m_log) state->m_log->Log(logDebug, 4, _T("prs PreProc  tok="_Pfs_"\n"), token);

			switch(kw)
			{
			case kwSpecial:
				if(token[0] == _T('\0'))
				{
					// EOL == End of Macro
					//
					if(! _tcscmp(pDirective + 1, _T("include")) || ! _tcscmp(pDirective + 1, _T("import")))
					{
						if(state->m_log) state->m_log->Log(logDebug, 3, _T("prs Include File="_Pfs_"\n"), target);
						ec = state->m_pCallback(state->m_cookie, target, NULL, 0, state->m_tokLine, efInclude);
						if(ec != errOK) return ec;
					}
					return errOK;
				}
				break;
			case kwPlain:
				if(noDirective)
				{
					_tcscat(directive, token);
					noDirective = false;
				}
				break;
			case kwQuoted:
				_tcscpy(target, token);
				break;
			case kwMacro:
			case kwBuiltinType:
			case kwAddonType:
			case kwBuiltin:
			case kwBuiltinFunc:
			case kwAddonFunc:
			case kwOperator:
			case kwComment:
			default:
				break;
			}
		}
	}
	while(ec == errOK);

	return ec;
}

//**************************************************************************
static void AddTypeMod(BefState* state, LPTSTR type, LPCTSTR pMod, int ccType)
{
	int len  = _tcslen(type);
	int room = ccType - len - 1;

	if(room <= 0) return;

	if(len > 0)
	{
		type[len++] = _T(' ');
		room--;
	}
	_tcsncpy(type + len, pMod, room);
	type[len + room] = _T('\0');
}

//**************************************************************************
ERRCODE BviewC::ParseInitializer(BefState* state, int inbrace, LPTSTR pEndChar)
{
	TCHAR   token[MAX_C_TOKEN];
	BkwType kw;
	ERRCODE ec;
	int     braceCnt = inbrace ? 1 : 0;

	do
	{
		ec = NextToken(state, token, MAX_C_TOKEN, kw);
		if(ec == errOK)
		{
			if(state->m_log) state->m_log->Log(logDebug, 4, _T("prs Initializer tok="_Pfs_"\n"), token);

			switch(kw)
			{
			case kwOperator:
				if(token[0] == _T('{'))
				{
					braceCnt++;
				}
				else if(token[0] == _T('}'))
				{
					braceCnt--;
				}
				else if(token[0] == _T(';') || token[0] == ')')
				{
					*pEndChar = token[0];
					return errOK;
				}
				else if(token[0] == _T(','))
				{
					if(braceCnt <= 0)
					{
						*pEndChar = token[0];
						return errOK;
					}
				}
				break;
			case kwPlain:
			case kwBuiltinType:
			case kwAddonType:
			case kwQuoted:
			case kwBuiltin:
			case kwBuiltinFunc:
			case kwAddonFunc:
			case kwMacro:
			case kwComment:
			default:
				break;
			}
		}
	}
	while(ec == errOK);
	*pEndChar = _T('\0');
	return ec;
}

//**************************************************************************
ERRCODE BviewC::ParseDimension(BefState* state)
{
	TCHAR   token[MAX_C_TOKEN];
	BkwType kw;
	ERRCODE ec;

	if(state->m_log) state->m_log->Log(logDebug, 3, _T("prs Dimension\n"), token);

	do
	{
		ec = NextToken(state, token, MAX_C_TOKEN, kw);
		if(ec == errOK)
		{
			if(state->m_log) state->m_log->Log(logDebug, 4, _T("prs Dimension tok="_Pfs_"\n"), token);

			switch(kw)
			{
			case kwOperator:
				if(token[0] == _T(']'))
					return errOK;
				break;
			case kwPlain:
			case kwBuiltinType:
			case kwAddonType:
			case kwQuoted:
			case kwBuiltin:
			case kwBuiltinFunc:
			case kwAddonFunc:
			case kwMacro:
			case kwComment:
			default:
				break;
			}
		}
	}
	while(ec == errOK);
	return ec;
}

//**************************************************************************
ERRCODE BviewC::ParseVarDecl(BefState* state, LPCTSTR pType, bool allowFuncDecls, bool& popscope)
{
	TCHAR   token[MAX_C_TOKEN];
	TCHAR   type[MAX_C_TOKEN * 4];
	TCHAR	prevtok[MAX_C_TOKEN];
	BkwType kw;
	ERRCODE ec;
	TCHAR	eoiChar;
	int     ptrTo;
	bool	noTarget = true;

	// accumulate type modifiers in type
	//
	type[0]		= _T('\0');
	prevtok[0]	= _T('\0');
	popscope    = false;

	AddTypeMod(state, type, pType, MAX_C_TOKEN*4);
	ptrTo = 0;

	if(state->m_log) state->m_log->Log(logDebug, 3, _T("prs Var Decl type="_Pfs_" allow=%d\n"), pType, allowFuncDecls);

	do
	{
		ec = NextToken(state, token, MAX_C_TOKEN, kw);
		if(ec == errOK)
		{
			if(state->m_log) state->m_log->Log(logDebug, 4, _T("prs Var Decl tok="_Pfs_"\n"), token);

			switch(kw)
			{
			case kwOperator:
				if(token[0] == _T('{'))
				{
					if(! _tcscmp(pType, _T("class")))
					{
						;
					}
					else if(! _tcscmp(pType, _T("struct")))
					{
						;
					}
					else if(! _tcscmp(pType, _T("interface")))
					{
						;
					}
					else if(! _tcscmp(pType, _T("enum")))
					{
						state->m_typedefPending = false;
						return ParseInitializer(state, 1, prevtok);
					}
					else
					{
						state->m_typedefPending = false;
						return ParseStatement(state, token, popscope);
					}
				}
				else if(token[0] == _T('}'))
				{
					if(state->m_log)
						state->m_log->Log(logDebug, 6, _T("prs unexpected close brace in decl\n"), token);
					popscope = true;
					return errOK;
				}
				else if(token[0] == _T(';'))
				{
					if(noTarget) return errOK;

					// complete type, end of statement
					if(state->m_log)
						state->m_log->Log(logDebug, 4, _T("prs add "_Pfs_" of type "_Pfs_"\n"), prevtok, type);
					ec = state->m_pCallback(state->m_cookie, prevtok, type, ptrTo, state->m_tokLine, state->m_typedefPending ? efTypeName : efVar);
					state->m_typedefPending = false;
					return ec;
				}
				else if(token[0] == _T(','))
				{
					if(noTarget)
					{
						state->m_typedefPending = false;
						return ParseStatement(state, token, popscope);
					}
					// complete type, and more to follow
					if(state->m_log)
						state->m_log->Log(logDebug, 4, _T("prs add "_Pfs_" of type "_Pfs_"\n"), prevtok, type);
					ec = state->m_pCallback(state->m_cookie, prevtok, type, ptrTo, state->m_tokLine, state->m_typedefPending ? efTypeName : efVar);
					prevtok[0] = _T('\0');
					ptrTo      = 0;
				}
				else if(token[0] == _T('='))
				{
					if(noTarget)
						return ParseStatement(state, token, popscope);

					// complete type, and initializer to follow
					if(state->m_log)
						state->m_log->Log(logDebug, 4, _T("prs add "_Pfs_" of type "_Pfs_" with initializer\n"), prevtok, type);
					ec = state->m_pCallback(state->m_cookie, prevtok, type, ptrTo, state->m_tokLine, state->m_typedefPending ? efTypeName : efVar);
					prevtok[0] = _T('\0');
					ptrTo      = 0;

					ec = ParseInitializer(state, 0, &eoiChar);
					if(eoiChar == _T(';'))
						return ec;
				}
				else if(token[0] == _T('('))
				{
					if(allowFuncDecls)
					{
						return ParseFunctionDecl(state, type, ptrTo, prevtok);
					}
					else
					{
						state->m_typedefPending = false;
						return ParseStatement(state, token, popscope);
					}
				}
				else if(token[0] == _T('*'))
				{
					ptrTo++;
				}
				else if(token[0] == _T('['))
				{
					ec = ParseDimension(state);
				}
				else if(token[0] == _T(':'))
				{
					state->m_typedefPending = false;
					ec = NextToken(state, token, MAX_C_TOKEN, kw);
					if(ec != errOK) break;
					if(kw == kwOperator && token[0] == _T(':'))
					{
						// name space function decl
						ec = NextToken(state, token, MAX_C_TOKEN, kw);
						if(ec != errOK) break;
						if(kw == kwPlain && token[0])
						{
							_tcscpy(prevtok, token);
						}
						else
						{
							return ParseStatement(state, token, popscope);
						}
					}
				}
				else
				{
					state->m_typedefPending = false;
					return ParseStatement(state, prevtok, popscope);
				}
				break;
			case kwPlain:
			case kwBuiltinType:
			case kwAddonType:
				if(prevtok[0])
					AddTypeMod(state, type, prevtok, MAX_C_TOKEN*4);
				_tcscpy(prevtok, token);
				noTarget = false;
				break;
			case kwQuoted:
			case kwBuiltin:
			case kwBuiltinFunc:
			case kwAddonFunc:
			case kwMacro:
			case kwComment:
			default:
				state->m_typedefPending = false;
				return ParseStatement(state, token, popscope);
			case kwSpecial:
				break;
			}
		}
	}
	while(ec == errOK);

	state->m_typedefPending = false;
	return ec;
}

//**************************************************************************
ERRCODE BviewC::ParseFunctionArgs(BefState* state)
{
	TCHAR   token[MAX_C_TOKEN];
	TCHAR   type[MAX_C_TOKEN * 4];
	TCHAR	prevtok[MAX_C_TOKEN];
	BkwType kw;
	ERRCODE ec;
	TCHAR	eoiChar;
	int     ptrTo;

	// accumulate type modifiers in type
	//
	type[0]		= _T('\0');
	prevtok[0]	= _T('\0');

	ptrTo = 0;

	do
	{
		ec = NextToken(state, token, MAX_C_TOKEN, kw);
		if(ec == errOK)
		{
			if(state->m_log) state->m_log->Log(logDebug, 4, _T("prs FuncArgs tok="_Pfs_"\n"), token);

			switch(kw)
			{
			case kwOperator:
				if(token[0] == _T(')'))
				{
					if(state->m_log)
						state->m_log->Log(logDebug, 4, _T("prs add last arg "_Pfs_" of type "_Pfs_"\n"), prevtok, type);
					ec = state->m_pCallback(state->m_cookie, prevtok, type, ptrTo, state->m_tokLine, efArgument);
					return ec;
				}
				else if(token[0] == _T(','))
				{
					// complete arg, and more to follow
					if(state->m_log)
						state->m_log->Log(logDebug, 4, _T("prs add arg "_Pfs_" of type "_Pfs_"\n"), prevtok, type);
					ec = state->m_pCallback(state->m_cookie, prevtok, type, ptrTo, state->m_tokLine, efArgument);
					prevtok[0] = _T('\0');
					type[0]    = _T('\0');
					ptrTo      = 0;
				}
				else if(token[0] == _T('='))
				{
					// complete arg, and initializer to follow
					if(state->m_log)
						state->m_log->Log(logDebug, 4, _T("prs add arg "_Pfs_" of type "_Pfs_" with initializer\n"), prevtok, type);
					ec = state->m_pCallback(state->m_cookie, prevtok, type, ptrTo, state->m_tokLine, efArgument);
					prevtok[0] = _T('\0');
					type[0]    = _T('\0');
					ptrTo      = 0;

					ec = ParseInitializer(state, 0, &eoiChar);
					if(eoiChar == _T(';') || eoiChar == _T(')'))
						return ec;
				}
				else if(token[0] == _T('*'))
				{
					ptrTo++;
				}
				else
				{
					if(state->m_log)
						state->m_log->Log(logDebug, 6, _T("prs unexpected operator "_Pfs_" in func arglist\n"), token);
				}
				break;
			case kwPlain:
			case kwBuiltinType:
			case kwAddonType:
				if(prevtok[0])
					AddTypeMod(state, type, prevtok, MAX_C_TOKEN*4);
				_tcscpy(prevtok, token);
				break;
			case kwQuoted:
			case kwBuiltin:
			case kwBuiltinFunc:
			case kwAddonFunc:
			case kwMacro:
			case kwComment:
			default:
				break;
			}
		}
	}
	while(ec == errOK);

	return ec;
}

//**************************************************************************
ERRCODE BviewC::ParseParentClass(BefState* state)
{
	TCHAR   token[MAX_C_TOKEN];
	TCHAR   type[MAX_C_TOKEN];
	BkwType kw;
	ERRCODE ec;

	do
	{
		ec = NextToken(state, type, MAX_C_TOKEN, kw);
		if(ec != errOK) return ec;
		if(state->m_log) state->m_log->Log(logDebug, 4, _T("prs Parent Access/Name 1 tok="_Pfs_"\n"), type);
	}
	while(kw != kwBuiltinType && kw != kwPlain);

	if (kw == kwBuiltinType)
	{
		do
		{
			ec = NextToken(state, token, MAX_C_TOKEN, kw);
			if(ec != errOK) return ec;
			if(state->m_log) state->m_log->Log(logDebug, 4, _T("prs Parent Name 2 tok="_Pfs_"\n"), token);
		}
		while(kw != kwPlain);
	}
	else
	{
		if(state->m_log) state->m_log->Log(logDebug, 4, _T("prs ParentAccess assumed public\n"));
		_tcscpy(token, type);
		_tcscpy(type, _T("public"));
	}
	if(state->m_log) state->m_log->Log(logDebug, 3, _T("prs Add Parent "_Pfs_" "_Pfs_"\n"), type, token);

	return ec;
}

//**************************************************************************
ERRCODE BviewC::ParseAccess(BefState* state, LPCTSTR pAccess)
{
	TCHAR   token[MAX_C_TOKEN];
	BkwType kw;
	ERRCODE ec;

	if(state->m_log) state->m_log->Log(logDebug, 3, _T("prs Set Access "_Pfs_"\n"), pAccess);

	do
	{
		ec = NextToken(state, token, MAX_C_TOKEN, kw);
		if(ec != errOK) return ec;
		if(state->m_log) state->m_log->Log(logDebug, 4, _T("prs Access tok="_Pfs_"\n"), token);
		
		// if nested classes are ok, this could be a class decl (c#)
		if (state->m_nestedclassok)
		{
			bool popscope;
			
			if(
					! _tcscmp(token, _T("class"))
				||	! _tcscmp(token, _T("struct"))
				||	! _tcscmp(token, _T("interface"))
			)
			{
				ec = ParseClassDecl(state, token, popscope);
				if(popscope) return ec;
			}
		}
	}
	while(kw != kwOperator || token[0] != _T(':'));

	return ec;
}

//**************************************************************************
ERRCODE BviewC::ParseClassDecl(BefState* state, LPCTSTR pType, bool& popscope)
{
	TCHAR   token[MAX_C_TOKEN];
	TCHAR   type[MAX_C_TOKEN * 4];
	TCHAR	prevtok[MAX_C_TOKEN];
	TCHAR   identifier[MAX_C_TOKEN];
	BkwType kw;
	ERRCODE ec;
	TCHAR	eoiChar;
	int     ptrTo;
	bool	tdp;
	bool    postScope;

	// accumulate type modifiers in type
	//
	prevtok[0]		= _T('\0');
	identifier[0]	= _T('\0');
	popscope		= false;
	postScope		= false;
	
	ptrTo = 0;

	if(state->m_log) state->m_log->Log(logDebug, 3, _T("prs Add Class/Struct "_Pfs_"\n"), token);

	do
	{
		ec = NextToken(state, token, MAX_C_TOKEN, kw);
		if(ec == errOK)
		{
			if(state->m_log) state->m_log->Log(logDebug, 4, _T("prs ClassDecl tok="_Pfs_" inclass=%d\n"),
					token, postScope);

			switch(kw)
			{
			case kwOperator:
				if(token[0] == _T(':'))
				{
					ec = ParseParentClass(state);
				}
				else if(token[0] == _T('{'))
				{
					// start the class spec process rolling
					//
					ec = state->m_pCallback(state->m_cookie, NULL, NULL, 0, state->m_tokLine, efClass);
					if(ec != errOK) break;

					if(! identifier[0] && prevtok[0])
					{
						_tcscpy(identifier, prevtok);
					}
					if(identifier[0])
					{
						ec = state->m_pCallback(state->m_cookie, identifier, NULL, 0, state->m_tokLine, efTypeName);
						if(ec != errOK) break;
					}
					tdp = state->m_typedefPending;
					state->m_typedefPending = false;
					ec = ParseScope(state, true);
					postScope = true;
					state->m_typedefPending = tdp;
					prevtok[0] = _T('\0');
					ptrTo      = 0;
				}
				else if(token[0] == _T('}'))
				{
					if(state->m_log)
						state->m_log->Log(logDebug, 6, _T("prs unexpected close brace in decl\n"), token);
					popscope = true;
					return errOK;
				}
				else if(token[0] == _T(';'))
				{
					// complete class, end of decl
					//
					if (postScope)
					{
						// tell pdb class is closed
						ec = state->m_pCallback(state->m_cookie, NULL, NULL, 0, state->m_tokLine, efEndClass);
					}
					if(prevtok[0])
					{
						if(postScope)
						{
							if(state->m_log)
								state->m_log->Log(logDebug, 4, _T("prs typename "_Pfs_" of type "_Pfs_"\n"), prevtok, identifier);
							ec = state->m_pCallback(state->m_cookie, prevtok, identifier, ptrTo, state->m_tokLine, state->m_typedefPending ? efTypeName : efVar);
							if(ec != errOK) break;
						}
						else if(identifier[0])
						{
							ec = state->m_pCallback(state->m_cookie, prevtok, identifier, ptrTo, state->m_tokLine, state->m_typedefPending ? efTypeName : efVar);
						}
					}
					state->m_typedefPending = false;
					return ec;
				}
				else if(token[0] == _T(','))
				{
					if(! postScope)
					{
						if(identifier[0])
						{
							ec = state->m_pCallback(state->m_cookie, prevtok, identifier, ptrTo, state->m_tokLine, state->m_typedefPending ? efTypeName : efVar);
						}
						ec = ParseParentClass(state);
					}
					else
					{
						// complete class, and more of same type to follow
						//
						if(prevtok[0])
						{
							if(! identifier[0])
							{
								_tcscpy(identifier, prevtok);
						
								ec = state->m_pCallback(state->m_cookie, prevtok, NULL, ptrTo, state->m_tokLine, efTypeName);
								if(state->m_log)
									state->m_log->Log(logDebug, 4, _T("prs typename "_Pfs_" of type "_Pfs_"\n"), prevtok, identifier);
							}
							if(state->m_log)
								state->m_log->Log(logDebug, 4, _T("prs var "_Pfs_" of type "_Pfs_"\n"), prevtok, identifier);
							ec = state->m_pCallback(state->m_cookie, prevtok, identifier, ptrTo, state->m_tokLine, state->m_typedefPending ? efTypeName : efVar);
							prevtok[0] = _T('\0');
						}
					}
					ptrTo      = 0;
				}
				else if(token[0] == _T('='))
				{
					if(postScope)
					{
						// complete type, and initializer to follow
						//
						if(prevtok[0])
						{
							if(! identifier[0])
							{
								_tcscpy(identifier, prevtok);
								if(state->m_log)
									state->m_log->Log(logDebug, 4, _T("prs typename "_Pfs_" of type "_Pfs_" with initializer\n"), prevtok, identifier);
								ec = state->m_pCallback(state->m_cookie, prevtok, identifier, ptrTo, state->m_tokLine, efTypeName);
								if(ec != errOK) break;
							}
							if(state->m_log)
								state->m_log->Log(logDebug, 4, _T("prs var "_Pfs_" of type "_Pfs_"\n"), prevtok, identifier);
							ec = state->m_pCallback(state->m_cookie, prevtok, identifier, ptrTo, state->m_tokLine, efVar);
						}
						prevtok[0] = _T('\0');
						ptrTo      = 0;

						ec = ParseInitializer(state, 0, &eoiChar);
						if(ec != errOK) break;
						if(eoiChar == _T(';'))
						{
							ec = state->m_pCallback(state->m_cookie, NULL, NULL, 0, state->m_tokLine, efEndClass);
							state->m_typedefPending = false;
							return ec;
						}
					}
				}
				else if(token[0] == _T('*'))
				{
					ptrTo++;
				}
				else if(token[0] == _T('['))
				{
					ec = ParseDimension(state);
				}
				else
				{
					if(state->m_log)
						state->m_log->Log(logDebug, 6, _T("prs unexpected operator "_Pfs_" in class decl\n"), token);
				}
				break;
			case kwPlain:
			case kwAddonType:
				if(prevtok[0])
				{
					AddTypeMod(state, type, prevtok, MAX_C_TOKEN*4);
				}
				_tcscpy(prevtok, token);
				if(! _tcscmp(pType, _T("interface")))
				{
					_tcscpy(identifier, token);
					ec = state->m_pCallback(state->m_cookie, NULL, NULL, 0, state->m_tokLine, efClass);
					ec = state->m_pCallback(state->m_cookie, identifier, NULL, 0, state->m_tokLine, efTypeName);
					if(ec != errOK) break;
					state->m_ininterface = true;
					ec = ParseScope(state, true);
					state->m_ininterface = false;
					postScope = true;
				}
				break;
			case kwBuiltinType:
			case kwQuoted:	// error at global scope
			case kwBuiltin:				
			case kwBuiltinFunc:
			case kwAddonFunc:
			case kwMacro:	// preproc
			case kwComment:
			default:
				break;
			}
		}
	}
	while(ec == errOK);
	if(postScope)
		ec = state->m_pCallback(state->m_cookie, NULL, NULL, 0, state->m_tokLine, efEndClass);
	state->m_typedefPending = false;
	return ec;
}

//**************************************************************************
ERRCODE BviewC::ParseFunctionDecl(BefState* state, LPCTSTR pType, int ptrTo, LPCTSTR name)
{
	TCHAR   token[MAX_C_TOKEN];
	BkwType kw;
	ERRCODE ec;

	if(! name || (! name[0] && pType && pType[0]))
	{
		name = pType;
		pType = _T("void");
	}

	if(state->m_log) state->m_log->Log(logDebug, 3, _T("prs Function "_Pfs_" [%d] "_Pfs_"\n"), pType, ptrTo, name);

	ec = state->m_pCallback(state->m_cookie, name, pType, ptrTo, state->m_tokLine, efFunction);
	if(ec != errOK) return ec;

	ec = ParseFunctionArgs(state);
	if(ec != errOK) return ec;

	do
	{
		ec = NextToken(state, token, MAX_C_TOKEN, kw);
		if(ec == errOK)
		{
			if(state->m_log) state->m_log->Log(logDebug, 4, _T("prs FuncDecl tok="_Pfs_"\n"), token);

			switch(kw)
			{
			case kwOperator:
				if(token[0] == _T('{'))
				{
					return ParseScope(state, false);
				}
				else if(token[0] == _T(';'))
				{
					return errOK;
				}
				else
				{
					if(state->m_log)
						state->m_log->Log(logDebug, 6, _T("prs illegal operator "_Pfs_" int func decl\n"), token);
				}
				break;
			case kwBuiltinType:
			case kwAddonType:
			case kwPlain:
			case kwQuoted:
			case kwBuiltin:
			case kwBuiltinFunc:
			case kwAddonFunc:
			case kwMacro:
			case kwComment:
			default:
				break;
			}
		}
	}
	while(ec == errOK);

	return ec;
}

//**************************************************************************
ERRCODE BviewC::ParseScope(BefState* state, bool allowFuncDecls)
{
	TCHAR   token[MAX_C_TOKEN];
	BkwType kw;
	bool    popscope = false;
	ERRCODE ec;

	state->m_scope++;
	state->m_typedefPending = false;

	if(state->m_log)
		state->m_log->Log(logDebug, 3, _T("---prs Scope %2d allow=%d\n"), state->m_scope, allowFuncDecls);

	do
	{
		ec = NextToken(state, token, MAX_C_TOKEN, kw);
		if(ec == errOK)
		{
			if(state->m_log)
				state->m_log->Log(logDebug, 4, _T("prs Scope %2d tok="_Pfs_"= a=%d\n"), state->m_scope, token, allowFuncDecls);

			switch(kw)
			{
			case kwBuiltinType:
			case kwAddonType:
			case kwPlain:	// type name
				if(
					(
							! _tcscmp(token, _T("class"))
						||	! _tcscmp(token, _T("struct"))
						||	! _tcscmp(token, _T("interface"))
					)
					&&
						((state->m_scope == 1) || state->m_nestedclassok)
				)
				{
					ec = ParseClassDecl(state, token, popscope);
					if(popscope) return ec;
				}
				else if(state->m_ininterface && ! _tcscmp(token, _T("end")))
				{
					state->m_scope--;
					return ec;
				}
				else if(! _tcscmp(token, _T("typedef")))
				{
					state->m_typedefPending = true;
					ec = errOK;
				}
				else if(
							state->m_scope == 2 &&
							(
								! _tcscmp(token, _T("internal"))	||
								! _tcscmp(token, _T("public"))		||
								! _tcscmp(token, _T("protected"))	||
								! _tcscmp(token, _T("private"))
							)
				)
				{
					ec = ParseAccess(state, token);
				}
				else
				{
					ec = ParseVarDecl(state, token, allowFuncDecls, popscope);
					if(popscope) return ec;
				}
				break;
			case kwQuoted:	// error at scope
			case kwOperator:
				if(token[0] == _T('{'))
				{
					ParseScope(state, false);
				}
				else if(token[0] == _T('}'))
				{
					state->m_scope--;
					return ec;
				}
				else if(token[0] == _T(';'))
				{
					ec = errOK;
				}
				else if(token[0] == _T('*'))
				{
					ec = ParseStatement(state, token, popscope);
				}
				else if(token[0] == _T(','))
				{
					ec = errOK;
				}
				else
				{
					ec = ParseStatement(state, token, popscope);
				}
				break;
			case kwMacro:	// preproc
				ec = ParsePreProc(state, token);
				break;
			case kwBuiltin:
			case kwBuiltinFunc:
			case kwAddonFunc:
				ec = ParseStatement(state, token, popscope);
				break;
			case kwComment:
			case kwSpecial:
			default:
				break;
			}
		}
	}
	while(! popscope && ec == errOK);
	state->m_scope--;
	return ec;
}

//**************************************************************************
ERRCODE BviewC::EnumFunctions(EnumFuncsCallback pCallback, LPVOID cookie, Blog* pLog)
{
	ERRCODE ec;

	// create a parser for this call
	// to make it reentrant
	//
	BefState state(pCallback, cookie, pLog);

	do
	{
		ec = ParseScope(&state, true);
	}
	while(ec == errOK);

	if(! pLog && state.m_log)
		delete state.m_log;
	state.m_log = NULL;
	return errOK;
}

//**************************************************************************
ERRCODE BviewCS::EnumFunctions(EnumFuncsCallback pCallback, LPVOID cookie, Blog* pLog)
{
	ERRCODE ec;

	// create a parser for this call
	// to make it reentrant
	//
	BefState state(pCallback, cookie, pLog);

	state.m_nestedclassok = true;
	do
	{
		ec = ParseScope(&state, true);
	}
	while(ec == errOK);

	if(! pLog && state.m_log)
		delete state.m_log;
	state.m_log = NULL;
	return errOK;
}

//**************************************************************************
BefState::BefState(EnumFuncsCallback pCallback, LPVOID cookie, Blog* pLog)
	:
	m_pCallback(pCallback),
	m_cookie(cookie),
	m_log(pLog)
{
#ifdef PDB_LOG
	if(! m_log)
		m_log = new Blog(8, logtoCONSOLE, _T("tp.log"));
#endif
	m_pToken	  = NULL;
	m_tokState	  = tsBase;
	m_tokLine	  = 1;
	m_scope       = 0;
	m_ininterface = false;
	m_nestedclassok = false;
}

//**************************************************************************
BefState::~BefState()
{
}
