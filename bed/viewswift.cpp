
#include "bedx.h"

//**************************************************************************
BviewSwift::BviewSwift(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel)
		:
		BviewC(pBuf, pEditor, pPanel)
{
	m_keywords = NULL; // dont inherit C's static keywords
	AddTags();
}

//**************************************************************************
BviewSwift::~BviewSwift()
{
	m_keywords = NULL;
}

//**************************************************************************
void BviewSwift::AddTags(void)
{
	AddKeyword(_T("associatedtype"), kwBuiltinType, true);
	AddKeyword(_T("class"), kwBuiltinType, true);
	AddKeyword(_T("deinit"), kwBuiltinType, true);
	AddKeyword(_T("enum"), kwBuiltinType, true);
	AddKeyword(_T("extension"), kwBuiltinType, true);
	AddKeyword(_T("fileprivate"), kwBuiltinType, true);
	AddKeyword(_T("func"), kwBuiltinType, true);
	AddKeyword(_T("import"), kwBuiltinType, true);
	AddKeyword(_T("init"), kwBuiltinType, true);
	AddKeyword(_T("inout"), kwBuiltinType, true);
	AddKeyword(_T("internal"), kwBuiltinType, true);
	AddKeyword(_T("let"), kwBuiltinType, true);
	AddKeyword(_T("open"), kwBuiltinType, true);
	AddKeyword(_T("operator"), kwBuiltinType, true);
	AddKeyword(_T("private"), kwBuiltinType, true);
	AddKeyword(_T("protocol"), kwBuiltinType, true);
	AddKeyword(_T("public"), kwBuiltinType, true);
	AddKeyword(_T("static"), kwBuiltinType, true);
	AddKeyword(_T("struct"), kwBuiltinType, true);
	AddKeyword(_T("subscript"), kwBuiltinType, true);
	AddKeyword(_T("typealias"), kwBuiltinType, true);
	AddKeyword(_T("var"), kwBuiltinType, true);

	AddKeyword(_T("break"), kwBuiltin, true);
	AddKeyword(_T("case"), kwBuiltin, true);
	AddKeyword(_T("continue"), kwBuiltin, true);
	AddKeyword(_T("default"), kwBuiltin, true);
	AddKeyword(_T("defer"), kwBuiltin, true);
	AddKeyword(_T("do"), kwBuiltin, true);
	AddKeyword(_T("else"), kwBuiltin, true);
	AddKeyword(_T("fallthrough"), kwBuiltin, true);
	AddKeyword(_T("for"), kwBuiltin, true);
	AddKeyword(_T("guard"), kwBuiltin, true);
	AddKeyword(_T("if"), kwBuiltin, true);
	AddKeyword(_T("in"), kwBuiltin, true);
	AddKeyword(_T("repeat"), kwBuiltin, true);
	AddKeyword(_T("return"), kwBuiltin, true);
	AddKeyword(_T("switch"), kwBuiltin, true);
	AddKeyword(_T("where"), kwBuiltin, true);
	AddKeyword(_T("while"), kwBuiltin, true);

	AddKeyword(_T("as"), kwBuiltin, true);
	AddKeyword(_T("Any"), kwBuiltin, true);
	AddKeyword(_T("catch"), kwBuiltin, true);
	AddKeyword(_T("false"), kwBuiltin, true);
	AddKeyword(_T("is"), kwBuiltin, true);
	AddKeyword(_T("nil"), kwBuiltin, true);
	AddKeyword(_T("rethrows"), kwBuiltin, true);
	AddKeyword(_T("super"), kwBuiltin, true);
	AddKeyword(_T("self"), kwBuiltin, true);
	AddKeyword(_T("Self"), kwBuiltin, true);
	AddKeyword(_T("throw"), kwBuiltin, true);
	AddKeyword(_T("throws"), kwBuiltin, true);
	AddKeyword(_T("true"), kwBuiltin, true);
	AddKeyword(_T("try"), kwBuiltin, true);

	AddKeyword(_T("associativity"), kwBuiltin, true);
	AddKeyword(_T("convenience"), kwBuiltin, true);
	AddKeyword(_T("dynamic"), kwBuiltin, true);
	AddKeyword(_T("didSet"), kwBuiltin, true);
	AddKeyword(_T("final"), kwBuiltin, true);
	AddKeyword(_T("get"), kwBuiltin, true);
	AddKeyword(_T("infix"), kwBuiltin, true);
	AddKeyword(_T("indirect"), kwBuiltin, true);
	AddKeyword(_T("lazy"), kwBuiltin, true);
	AddKeyword(_T("left"), kwBuiltin, true);
	AddKeyword(_T("mutating"), kwBuiltin, true);
	AddKeyword(_T("none"), kwBuiltin, true);
	AddKeyword(_T("nonmutating"), kwBuiltin, true);
	AddKeyword(_T("optional"), kwBuiltin, true);
	AddKeyword(_T("override"), kwBuiltin, true);
	AddKeyword(_T("postfix"), kwBuiltin, true);
	AddKeyword(_T("precedence"), kwBuiltin, true);
	AddKeyword(_T("prefix"), kwBuiltin, true);
	AddKeyword(_T("Protocol"), kwBuiltin, true);
	AddKeyword(_T("required"), kwBuiltin, true);
	AddKeyword(_T("right"), kwBuiltin, true);
	AddKeyword(_T("set"), kwBuiltin, true);
	AddKeyword(_T("Type"), kwBuiltin, true);
	AddKeyword(_T("unowned"), kwBuiltin, true);
	AddKeyword(_T("weak"), kwBuiltin, true);
	AddKeyword(_T("willSet"), kwBuiltin, true);
	AddKeyword(_T("_."), kwBuiltin, true);
}


//**************************************************************************
ERRCODE BviewSwift::EnumFunctions(EnumFuncsCallback pCallback, LPVOID cookie, Blog* pLog)
{
	ERRCODE		ec;
	TokenRet	tr;
	LPTSTR		lpText, lpToken;
	int			nText,  nToken;
	int			braces;
	int			line;
	int			incol, outcol, tokcol;
	TokenState	state;
	TCHAR		svc;
	BkwType		kw;

	braces	= 0;
	line	= 1;

	enum {	psBase,
			psFuncDecl, psClassFuncDecl,
			psInFunc, psInClassFunc,
			psClassDecl, psInClass, 
			psProtocolDecl, psInProtocol,
			psExtensionDecl, psInExtension,
	} parseState = psBase;
	
	do
	{
		ec = m_buffer->GetLineText(line, (LPCTSTR&)lpText, nText);
		if(ec != errOK) return errOK;
		
		incol	 = 1;
		outcol	 = 1;
		state 	 = tsBase;

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

			if(
					(state != tsSpanningComment)		&&
					(kw != kwComment)					&& 
					(lpToken[0] != _T(' '))				&&
					(lpToken[0] != _T('\t'))
			)
			{
				switch(parseState) 
				{
				case psBase:

					switch(kw)
					{
					case kwOperator:

						switch(lpToken[0])
						{
						case _T('{'):
							braces++;
							break;

						case _T('}'):
							braces--;
							if(braces < 0) braces = 0;
							break;

						default:
							break;
						}
						break;

					case kwPlain:
					case kwBuiltinType:

						if(! KeywordCompare(lpToken, _T("class"), nToken, 5, true))
						{
							parseState = psClassDecl;
							braces = 0;
						}
						else if(! KeywordCompare(lpToken, _T("func"), nToken, 4, true))
						{
							parseState = psFuncDecl;
							braces = 0;
						}
						else if(! KeywordCompare(lpToken, _T("protocol"), nToken, 8, true))
						{
							parseState = psProtocolDecl;
							braces = 0;
						}
						else if(! KeywordCompare(lpToken, _T("extension"), nToken, 9, true))
						{
							parseState = psExtensionDecl;
							braces = 0;
						}
						break;

					default:
						break;
					}
					break;
					
				case psFuncDecl:
				case psClassFuncDecl:

					switch(kw)
					{
					case kwPlain:
						svc = lpToken[nToken];
						lpToken[nToken] = _T('\0');
						ec = pCallback(cookie, lpToken, lpToken, 0, line, efFunction);
						lpToken[nToken] = svc;
						break;
						
					case kwOperator:

						switch(lpToken[0])
						{
						case '{':
							if(parseState == psFuncDecl)
							{
								parseState = psInFunc;
							}
							else
							{
								parseState = psInClassFunc;
							}
							braces++;
							break;

						case '}':
							braces--;
							if(braces <= 0)
								parseState = psBase;
							break;

						default:
							break;
						}
						break;
					}
					break;

				case psClassDecl:

					switch(kw)
					{
					case kwPlain:
						svc = lpToken[nToken];
						lpToken[nToken] = _T('\0');
						ec = pCallback(cookie, lpToken, lpToken, 0, line, efClass);
						lpToken[nToken] = svc;
						break;
						
					case kwOperator:

						switch(lpToken[0])
						{
						case '{':
							parseState = psInClass;
							braces++;
							break;

						case '}':
							braces--;
							if(braces <= 0)
								parseState = psBase;
							break;

						default:
							break;
						}
						break;
					}
					break;

				case psProtocolDecl:
				case psExtensionDecl:

					switch(kw)
					{
					case kwPlain:
						svc = lpToken[nToken];
						lpToken[nToken] = _T('\0');
						ec = pCallback(cookie, lpToken, lpToken, 0, line, efInterface);
						lpToken[nToken] = svc;
						break;
						
					case kwOperator:

						switch(lpToken[0])
						{
						case '{':
							if (parseState == psProtocolDecl)
							{
								parseState = psInProtocol;
							}
							else
							{
								parseState = psInExtension;
							}
							braces++;
							break;

						case '}':
							braces--;
							if(braces <= 0)
							{
								parseState = psBase;
							}
							break;

						default:
							break;
						}
						break;
					}
					break;

				case psInFunc:
				case psInClassFunc:

					switch(kw)
					{
					case kwOperator:

						switch(lpToken[0])
						{
						case _T('{'):
							braces++;
							break;

						case _T('}'):
							braces--;
							if(braces <= 1)
							{
								if(parseState == psInFunc)
								{
									parseState = psBase;
								}
								else
								{
									parseState = psInClass;
								}
							}
							break;

						default:
							break;
						}
						break;

					default:
						break;
					}
					break;
					
				case psInClass:
				case psInExtension:
				case psInProtocol:

					switch(kw)
					{
					case kwOperator:

						switch(lpToken[0])
						{
						case '{':
							braces++;
							break;

						case '}':
							braces--;
							if(braces <= 0)
							{
								svc = lpToken[nToken];
								lpToken[nToken] = _T('\0');
								ec = pCallback(cookie, _T(""), _T(""), 0, line, 
									(parseState == psInClass) ? efEndClass : efEndInterface);
								lpToken[nToken] = svc;
								parseState = psBase;
							}
							break;

						default:
							break;
						}
						break;

					case kwPlain:
					case kwBuiltinType:

						if(! KeywordCompare(lpToken, _T("func"), nToken, 4, true))
						{
							parseState = psClassFuncDecl;
						}
						break;

					default:
						break;
					}
					break;

				default:
					break;
				}
			}
		}
		while(ec == errOK);

		if(ec != errOK) break;
		line++;

	} while(ec == errOK);

	return errOK;
}

