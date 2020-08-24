
#include "bedx.h"

//bool		BviewJava::m_havejkeywords		= false;
//LPKEYWORD	BviewJava::m_jkeywords			= NULL;

//**************************************************************************
BviewJava::BviewJava(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel)
		:
		BviewCPP(pBuf, pEditor, pPanel)
{
	m_keywords = NULL; // dont inherit C's static keywords
	AddTags();
}

//**************************************************************************
BviewJava::~BviewJava()
{
	//if(m_init == 1)
	//{
	//	m_havejkeywords	= false;
	//	m_havejkeywords	= false;
		delete m_keywords;
	//}
	m_keywords = NULL;
}

//**************************************************************************
void BviewJava::AddTags(void)
{
	//if(m_havejkeywords)
	//{
	//	m_keywords = m_jkeywords;
	//	return;
	//}
	if(m_buffer->GetType() == btJavaScript || m_buffer->GetType() == btPHP)
	{
		Bview::AddTags(BviewHTML::m_keywords, kwBuiltinType, false);
	}
	AddKeyword(_T("abstract"), kwBuiltinType, true);
	AddKeyword(_T("boolean"), kwBuiltinType, true);
	AddKeyword(_T("byte"), kwBuiltinType, true);
	AddKeyword(_T("break"), kwBuiltin, true);
	AddKeyword(_T("byvalue"), kwBuiltin, true);
	AddKeyword(_T("char"), kwBuiltinType, true);
	AddKeyword(_T("case"), kwBuiltin, true);
	AddKeyword(_T("cast"), kwBuiltin, true);
	AddKeyword(_T("continue"), kwBuiltin, true);
	AddKeyword(_T("const"), kwBuiltinType, true);
	AddKeyword(_T("class"), kwBuiltinType, true);
	AddKeyword(_T("catch"), kwBuiltin, true);
	AddKeyword(_T("do"), kwBuiltin, true);
	AddKeyword(_T("default"), kwBuiltin, true);
	AddKeyword(_T("double"), kwBuiltinType, true);
	AddKeyword(_T("else"), kwBuiltin, true);
	AddKeyword(_T("extends"), kwBuiltin, true);
	AddKeyword(_T("false"), kwBuiltin, true);
	AddKeyword(_T("for"), kwBuiltin, true);
	AddKeyword(_T("float"), kwBuiltinType, true);
	AddKeyword(_T("final"), kwBuiltin, true);
	AddKeyword(_T("finally"), kwBuiltin, true);
	AddKeyword(_T("future"), kwBuiltin, true);
	AddKeyword(_T("goto"), kwBuiltin, true);
	AddKeyword(_T("generic"), kwBuiltin, true);
	AddKeyword(_T("if"), kwBuiltin, true);
	AddKeyword(_T("implements"), kwBuiltin, true);
	AddKeyword(_T("import"), kwBuiltin, true);
	AddKeyword(_T("int"), kwBuiltinType, true);
	AddKeyword(_T("instanceof"), kwBuiltin, true);
	AddKeyword(_T("interface"), kwBuiltinType, true);
	AddKeyword(_T("long"), kwBuiltinType, true);
	AddKeyword(_T("new"), kwBuiltin, true);
	AddKeyword(_T("null"), kwBuiltin, true);
	AddKeyword(_T("operator"), kwBuiltinType, true);
	AddKeyword(_T("outer"), kwBuiltin, true);
	AddKeyword(_T("public"), kwBuiltinType, true);
	AddKeyword(_T("private"), kwBuiltinType, true);
	AddKeyword(_T("protected"), kwBuiltinType, true);
	AddKeyword(_T("package"), kwBuiltin, true);
	AddKeyword(_T("return"), kwBuiltin, true);
	AddKeyword(_T("rest"), kwBuiltin, true);
	AddKeyword(_T("switch"), kwBuiltin, true);
	AddKeyword(_T("static"), kwBuiltinType, true);
	AddKeyword(_T("super"), kwBuiltin, true);
	AddKeyword(_T("short"), kwBuiltinType, true);
	AddKeyword(_T("synchronized"), kwBuiltin, true);
	AddKeyword(_T("true"), kwBuiltin, true);
	AddKeyword(_T("try"), kwBuiltin, true);
	AddKeyword(_T("transient"), kwBuiltin, true);
	AddKeyword(_T("this"), kwBuiltin, true);
	AddKeyword(_T("throw"), kwBuiltin, true);
	AddKeyword(_T("throws"), kwBuiltin, true);
	AddKeyword(_T("void"), kwBuiltinType, true);
	AddKeyword(_T("volatile"), kwBuiltinType, true);
	AddKeyword(_T("while"), kwBuiltin, true);

	//m_havejkeywords = true;
	//m_jkeywords = m_keywords;
}


//**************************************************************************
ERRCODE BviewJava::EnumFunctions(EnumFuncsCallback pCallback, LPVOID cookie, Blog *pLog)
{
	ERRCODE		ec;
	TokenRet	tr;
	LPTSTR		lpText, lpToken, lpPrevToken;
	int			nText,  nToken,  nPrevToken;
	int			braces;
	int			line;
	int			incol, outcol, tokcol;
	TokenState	state;
	BkwType		kw;

	braces	= 0;
	line	= 1;

	enum { csClass, csInterface } classState;

	enum {	psBase,
			psInFunc, psPostFunc, psInProto,
			psClassSpec, psInterfaceSpec, 
			psClass, psInterface,
			psFirstInClass, psInClass, psInInterface,
			psImplements, psExtends
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
							braces = 1;
							parseState = psInFunc;
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
							parseState = psClassSpec;
							braces = 0;
						}
						else if(! KeywordCompare(lpToken, _T("interface"), nToken, 9, true))
						{
							parseState = psInterfaceSpec;
							braces = 0;
						}
						break;

					default:
						break;
					}
					break;
					
				case psClassSpec:

					if(kw == kwPlain)
					{
						parseState = psClass;
						classState = csClass;
						ec = pCallback(cookie, lpToken, lpToken, 0, line, efClass);
					}
					break;

				case psInterfaceSpec:

					if(kw == kwPlain)
					{
						parseState = psInterface;
						classState = csInterface;
						ec = pCallback(cookie, lpToken, lpToken, 0, line, efInterface);
					}
					break;

				case psInterface:
				case psClass:
				case psExtends:
				case psImplements:

					switch(kw)
					{
					case kwOperator:

						switch(lpToken[0])
						{
						case '{':
							braces++;
							parseState = psFirstInClass;
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

					case kwBuiltinType:

						if(! KeywordCompare(lpToken, _T("implements"), nToken, 10, true))
						{
							parseState = psImplements;
						}
						else if(! KeywordCompare(lpToken, _T("extends"), nToken, 7, true))
						{
							parseState = psExtends;
						}
						break;

					default:
						break;
					}
					break;

				case psFirstInClass:
				case psInClass:
				case psInInterface:
				case psPostFunc:

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
							if(braces <= 0)
							{
								ec = pCallback(cookie, lpPrevToken, _T(""), 0, line, 
									(classState = csClass) ? efEndClass : efEndInterface);
                                parseState = psBase;
							}
							break;

						case _T('('):

							if(braces == 1)
								ec = pCallback(cookie, lpPrevToken, _T(""), 0, line, efFunction);

							if(classState == csClass)
								parseState = psInFunc;
							else
								parseState = psInProto;
							break;

						default:
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
					case kwOperator:

						switch(lpToken[0])
						{
						case _T('{'):
							braces++;
							break;

						case _T('}'):
							braces--;
							if(braces <= 1)
								parseState = psPostFunc;
							break;

						default:
							break;
						}
						break;

					default:
						break;
					}
					break;
					
				case psInProto:

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
								parseState = psPostFunc;
							break;

						case ')':
							parseState = psPostFunc;
							break;

						default:
							break;
						}
						break;

					default:
						break;
					}
					break;
					
				default:
					break;
				}
				nPrevToken  = nToken;
				lpPrevToken = lpToken;
			}
		}
		while(ec == errOK);

		if(ec != errOK) break;
		line++;

	} while(ec == errOK);

	return errOK;
}

