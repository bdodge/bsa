
#include "bedx.h"

bool		BviewC::m_haveckeywords		= false;
LPKEYWORD	BviewC::m_ckeywords			= NULL;

bool		BviewC::m_havecppkeywords	= false;
LPKEYWORD	BviewC::m_cppkeywords		= NULL;

bool		BviewC::m_havecskeywords	= false;
LPKEYWORD	BviewC::m_cskeywords		= NULL;

bool		BviewC::m_haveobjckeywords	= false;
LPKEYWORD	BviewC::m_objckeywords		= NULL;

//**************************************************************************
BviewC::BviewC(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel)
		:
		Bview(pBuf, pEditor, pPanel),
		m_iscpp(false)
{
	m_statement_term	= _T(';');
	m_macro_prefix		= _T('#');

	AddTags();

	m_haveckeywords		= true;
	m_ckeywords			= m_keywords;
}

//**************************************************************************
BviewC::~BviewC()
{
	if(m_init == 1)
	{
		m_haveckeywords		= false;
		m_havecppkeywords	= false;
		if(m_objckeywords)
			delete m_objckeywords;
		else if(m_cppkeywords)
			delete m_cppkeywords;
		else if(m_ckeywords)
			delete m_ckeywords;
		m_objckeywords = NULL;
		m_cppkeywords  = NULL;
		m_ckeywords    = NULL;
	}
	m_keywords = NULL;
}

//**************************************************************************
void BviewC::SpecificMouseMenu(HMENU hMenu)
{
	Bpdb*	pPDB;
	LPBSYM	pSym;
	ERRCODE ec;
	bool	isFunc;
	int		line, col;

	AppendMenu(hMenu, 0, ID_EDIT_FINDINFILE, _T("&Find in Files..."));
	if(m_editor->GetProject())
	{
		pPDB = m_editor->GetProject()->GetPDB();
		if(pPDB)
		{
			line = m_curline;
			col  = m_curcol;
			pSym = NULL;

			ec = GetDefinitionOfTokenAtPoint(line, col, pSym, pPDB, isFunc);
			if(ec == errOK)
			{
				//_tprintf(_T("Defined on Line %d\n"), line);
				AppendMenu(hMenu, 0, ID_EDIT_FINDDEFOF, _T("Goto &Declaration"));
			}
			AppendMenu(hMenu, 0, ID_EDIT_FINDREFSOF, _T("Find &References"));
		}
	}
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
}

//**************************************************************************
ERRCODE BviewC::Key(WORD key)
{
	EditCommand command;
	ERRCODE		ec;
	int 		keylen = 1;

	switch(key)
	{
	case keyF5: case keyF9: case keyF10: case keyF11:
		if(m_editor->GetDebugger())
		{
			switch(key)
			{
			case keyF5:
				command = Start;
				break;
			case keyF9:
				command = ToggleBreak;
				break;
			case keyF10:
				command = Step;
				break;
			case keyF11:
				command = StepIn;
				break;
			}
		}
		else
		{
			return Bview::Key(key);
		}
		break;

	default:
		command = m_keyBindings->Translate(key);
		break;
	}
	if(command == SelfInsert)
	{
		TCHAR keytext[2] = { key & 0xff, 0 };
		int   sl, sc;

		sl = m_curline;
		sc = m_curcol;

		PushParm(keytext, 1, ptString);
		ec = Dispatch(command);

		switch(key)
		{
		case '\r': case '\n':
		case '{':  case '}':
			ec = Dispatch(::Indent);
			break;
		case ':': // only if a "case" comes before it
			ec = Dispatch(::Indent);
			break;
		case '#': // only if first token on line
			ec = Dispatch(::Indent);
			break;

		case '.':
		case '>':
		case '(':
			ec = Dispatch(::Dismember);
			break;

		case ')':
		//	ec = Dispatch(::Indent);
			// fall into
		case ';':
			CheckCloseProtoWindow();
			break;

		default:
			break;
		}
		switch(key)
		{
		case ')': case ']': case '}':
			PushParm(keytext, 1, ptString);
			ec = Dispatch(::Match);
			break;

		default:
			break;
		}
		return ec;
	}
	else
	{
		return Bview::Key(key);
	}
}


//**************************************************************************
void BviewC::AddTags(void)
{
	AddComment(_T("/*"), tsSpanningComment);
	AddComment(_T("*/"), tsBase);
	AddComment(_T("//"), tsComment);

	AddPrefix(_T("#"),  kwMacro);

	if(m_iscpp ? m_havecppkeywords : m_haveckeywords)
	{
		m_keywords = m_iscpp ? m_cppkeywords : m_ckeywords;
		return;
	}
	AddKeyword(_T("auto"), kwBuiltin, true);
	AddKeyword(_T("asm"),  kwBuiltin, true);
	AddKeyword(_T("break"), kwBuiltin, true);
	AddKeyword(_T("char"), kwBuiltinType, true);
	AddKeyword(_T("case"), kwBuiltin, true);
	AddKeyword(_T("continue"), kwBuiltin, true);
	AddKeyword(_T("const"), kwBuiltinType, true);
	AddKeyword(_T("do"), kwBuiltin, true);
	AddKeyword(_T("default"), kwBuiltin, true);
	AddKeyword(_T("double"), kwBuiltinType, true);
	AddKeyword(_T("else"), kwBuiltin, true);
	AddKeyword(_T("enum"), kwBuiltinType, true);
	AddKeyword(_T("extern"), kwBuiltinType, true);
	AddKeyword(_T("entry"), kwBuiltin, true);
	AddKeyword(_T("for"), kwBuiltin, true);
	AddKeyword(_T("float"), kwBuiltin, true);
	AddKeyword(_T("goto"), kwBuiltin, true);
	AddKeyword(_T("int"), kwBuiltinType, true);
	AddKeyword(_T("if"), kwBuiltin, true);
	AddKeyword(_T("long"), kwBuiltinType, true);
	AddKeyword(_T("return"), kwBuiltin, true);
	AddKeyword(_T("register"), kwBuiltinType, true);
	AddKeyword(_T("switch"), kwBuiltin, true);
	AddKeyword(_T("static"), kwBuiltinType, true);
	AddKeyword(_T("struct"), kwBuiltinType, true);
	AddKeyword(_T("short"), kwBuiltinType, true);
	AddKeyword(_T("sizeof"), kwBuiltin, true);
	AddKeyword(_T("signed"), kwBuiltinType, true);
	AddKeyword(_T("typedef"), kwBuiltinType, true);
	AddKeyword(_T("unsigned"), kwBuiltinType, true);
	AddKeyword(_T("union"), kwBuiltinType, true);
	AddKeyword(_T("void"), kwBuiltinType, true);
	AddKeyword(_T("volatile"), kwBuiltinType, true);
	AddKeyword(_T("while"), kwBuiltin, true);
}

//**************************************************************************
BviewCPP::BviewCPP(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel)
	:
	BviewC(pBuf, pEditor, pPanel)
{
	m_iscpp = true;
	AddTags();
	m_havecppkeywords	= true;
	m_cppkeywords		= m_keywords;
}

//**************************************************************************
void BviewCPP::AddTags(void)
{
	if(m_havecppkeywords)
	{
		m_keywords = m_cppkeywords;
		return;
	}
	AddKeyword(_T("bool"), kwBuiltinType, true);
	AddKeyword(_T("class"), kwBuiltinType, true);
	AddKeyword(_T("catch"), kwBuiltin, true);
	AddKeyword(_T("delete"), kwBuiltin, true);
	AddKeyword(_T("false"), kwBuiltin, true);
	AddKeyword(_T("friend"), kwBuiltinType, true);
	AddKeyword(_T("inline"), kwBuiltinType, true);
	AddKeyword(_T("new"), kwBuiltin, true);
	AddKeyword(_T("operator"), kwBuiltinType, true);
	AddKeyword(_T("public"), kwBuiltinType, true);
	AddKeyword(_T("private"), kwBuiltinType, true);
	AddKeyword(_T("protected"), kwBuiltinType, true);
	AddKeyword(_T("template"), kwBuiltinType, true);
	AddKeyword(_T("this"), kwBuiltin, true);
	AddKeyword(_T("throw"), kwBuiltin, true);
	AddKeyword(_T("true"), kwBuiltin, true);
	AddKeyword(_T("try"), kwBuiltin, true);
	AddKeyword(_T("virtual"), kwBuiltinType, true);
}

//**************************************************************************
BviewCS::BviewCS(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel)
	:
	BviewC(pBuf, pEditor, pPanel)
{
	AddTags();
	m_havecskeywords	= true;
	m_cskeywords		= m_keywords;
}

//**************************************************************************
void BviewCS::AddTags(void)
{
	if(m_havecskeywords)
	{
		m_keywords = m_cskeywords;
		return;
	}
	AddKeyword(_T("abstract"), kwBuiltinType, true);
	AddKeyword(_T("async"), kwBuiltinType, true);
	AddKeyword(_T("event"), kwBuiltinType, true);
	AddKeyword(_T("override"), kwBuiltinType, true);
	AddKeyword(_T("partial"), kwBuiltinType, true);
	AddKeyword(_T("readonly"), kwBuiltinType, true);
	AddKeyword(_T("sealed"), kwBuiltinType, true);
	AddKeyword(_T("unsafe"), kwBuiltinType, true);

	AddKeyword(_T("null"), kwBuiltin, true);
	AddKeyword(_T("false"), kwBuiltin, true);
	AddKeyword(_T("true"), kwBuiltin, true);
	AddKeyword(_T("value"), kwBuiltin, true);

	AddKeyword(_T("add"), kwSpecial, true);
	AddKeyword(_T("var"), kwSpecial, true);
	AddKeyword(_T("dynamic"), kwSpecial, true);
	AddKeyword(_T("global"), kwSpecial, true);
	AddKeyword(_T("set"), kwSpecial, true);
	AddKeyword(_T("add"), kwSpecial, true);
	AddKeyword(_T("value"), kwSpecial, true);

	AddKeyword(_T("from"), kwSpecial, true);
	AddKeyword(_T("where"), kwSpecial, true);
	AddKeyword(_T("select"), kwSpecial, true);
	AddKeyword(_T("group"), kwSpecial, true);
	AddKeyword(_T("into"), kwSpecial, true);
	AddKeyword(_T("orderby"), kwSpecial, true);
	AddKeyword(_T("join"), kwSpecial, true);
	AddKeyword(_T("let"), kwSpecial, true);
	AddKeyword(_T("in"), kwSpecial, true);
	AddKeyword(_T("on"), kwSpecial, true);
	AddKeyword(_T("equals"), kwSpecial, true);
	AddKeyword(_T("by"), kwSpecial, true);
	AddKeyword(_T("ascending"), kwSpecial, true);
	AddKeyword(_T("descending"), kwSpecial, true);

	AddKeyword(_T("params"), kwBuiltin, true);
	AddKeyword(_T("red"), kwBuiltin, true);
	AddKeyword(_T("out"), kwBuiltin, true);

	AddKeyword(_T("internal"), kwBuiltinType, true);
	AddKeyword(_T("private"), kwBuiltinType, true);
	AddKeyword(_T("protected"), kwBuiltinType, true);
	AddKeyword(_T("public"), kwBuiltinType, true);

	AddKeyword(_T("foreach"), kwBuiltin, true);
	AddKeyword(_T("in"), kwBuiltin, true);
	AddKeyword(_T("yield"), kwBuiltin, true);
	AddKeyword(_T("try"), kwBuiltin, true);
	AddKeyword(_T("throw"), kwBuiltin, true);
	AddKeyword(_T("catch"), kwBuiltin, true);
	AddKeyword(_T("finally"), kwBuiltin, true);
	AddKeyword(_T("checked"), kwBuiltin, true);
	AddKeyword(_T("unchecked"), kwBuiltin, true);
	AddKeyword(_T("fixed"), kwBuiltin, true);
	AddKeyword(_T("lock"), kwBuiltin, true);

	AddKeyword(_T("namespace"), kwBuiltin, true);
	AddKeyword(_T("using"), kwBuiltin, true);
	AddKeyword(_T("operator"), kwBuiltin, true);

	AddKeyword(_T("bool"), kwBuiltinType, true);
	AddKeyword(_T("byte"), kwBuiltinType, true);
	AddKeyword(_T("class"), kwBuiltinType, true);
	AddKeyword(_T("decimal"), kwBuiltinType, true);
	AddKeyword(_T("sbyte"), kwBuiltinType, true);
	AddKeyword(_T("string"), kwBuiltinType, true);
	AddKeyword(_T("uint"), kwBuiltinType, true);
	AddKeyword(_T("ulong"), kwBuiltinType, true);
	AddKeyword(_T("uhort"), kwBuiltinType, true);

	AddKeyword(_T("base"), kwBuiltin, true);
	AddKeyword(_T("this"), kwBuiltin, true);
}

//**************************************************************************
BviewObjectiveC::BviewObjectiveC(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel)
	:
	BviewC(pBuf, pEditor, pPanel)
{
	AddTags();
	m_haveobjckeywords	= true;
	m_objckeywords		= m_keywords;
}

//**************************************************************************
void BviewObjectiveC::AddTags(void)
{
	if(m_haveobjckeywords)
	{
		m_keywords = m_objckeywords;
		return;
	}
	AddKeyword(_T("id"), kwBuiltinType, true);
	AddKeyword(_T("Class"), kwBuiltinType, true);
	AddKeyword(_T("SEL"), kwBuiltinType, true);
	AddKeyword(_T("IMP"), kwBuiltinType, true);
	AddKeyword(_T("BOOL"), kwBuiltinType, true);

	AddKeyword(_T("nil"), kwBuiltin, true);
	AddKeyword(_T("Nil"), kwBuiltin, true);
	AddKeyword(_T("NO"), kwBuiltin, true);
	AddKeyword(_T("YES"), kwBuiltin, true);

	AddKeyword(_T("interface"), kwBuiltin, true);
	AddKeyword(_T("implementation"), kwBuiltin, true);
	AddKeyword(_T("protocol"), kwBuiltin, true);
	AddKeyword(_T("end"), kwBuiltin, true);

	AddKeyword(_T("private"), kwBuiltin, true);
	AddKeyword(_T("protected"), kwBuiltin, true);
	AddKeyword(_T("public"), kwBuiltin, true);

	AddKeyword(_T("try"), kwBuiltin, true);
	AddKeyword(_T("throw"), kwBuiltin, true);
	AddKeyword(_T("catch"), kwBuiltin, true);
	AddKeyword(_T("finally"), kwBuiltin, true);

	AddKeyword(_T("property"), kwBuiltin, true);
	AddKeyword(_T("synthesize"), kwBuiltin, true);
	AddKeyword(_T("dynamic"), kwBuiltin, true);

	AddKeyword(_T("class"), kwBuiltinType, true);
	AddKeyword(_T("selector"), kwBuiltin, true);
	AddKeyword(_T("synchronized"), kwBuiltinType, true);
	AddKeyword(_T("encode"), kwBuiltin, true);

	AddKeyword(_T("super"), kwBuiltin, true);
	AddKeyword(_T("self"), kwBuiltin, true);
}

//**************************************************************************
TokenRet BviewC::GetToken(
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
	TokenRet tr;

	if(incol == 1)
		m_first_token = true;

	tr = Bview::GetToken(lpText, nText, line, incol, outcol, state, lpToken, nToken, kw);

	if(tr != trFAILURE)
	{
		if(lpToken[0] == m_macro_prefix)
		{
			if(! m_first_token)
			{
				// dont let this become a macro
				kw = kwSpecial;
			}
		}
		if(m_first_token && incol != 1 && nToken > 0)
		{
			int w;

			for(w = 0; m_first_token && (w < nToken); w++)
			{
				if(lpToken[w] != _T(' ') && lpToken[w] != '\t')
				{
					m_first_token = false;
				}
			}
		}
	}
	return tr;
}


//**************************************************************************
ERRCODE BviewC::Indent(WORD key, int& line, int& col)
{
	ERRCODE 	ec;
	int  		sl, sc;
	bool 		nonblank, prevnb, statnb;
	int			prevfirstcol;
	int			curfirstcol;
	TCHAR		lastdelim;
	TCHAR		firstdelim;
	int			firstdelimcol;
	TCHAR		prevlastdelim;
	TCHAR		prevfirstdelim;
	int			prevfirstdelimcol;
	int			parens, braces, colons;
	int			prevparens, prevbraces, prevcolons;
	int			incol, outcol, protoline, protocol;
	int			netdent;

	BkwType		firstdelimkw, kw;
	int			statementcol;
	TCHAR		statement[128];
	int			nStatement;

	bool		matching = false;
	bool		label = false;

	LPCTSTR 	lpText;
	int			nText;
	int			nCut = 0;
	int			cutcol;
	int			coldelta;

	if(! m_buffer) return errFAILURE;

	// get the output column of the first non-blank token
	// on this line
	//
	sl = line;
	sc = col;

	ec = GetFirstNonBlankColumn(
									line,
									curfirstcol,
									true,
									true,
									lastdelim,
									firstdelim,
									firstdelimkw,
									firstdelimcol,
									parens,
									braces,
									colons
								);
	if(ec != errOK) return ec;
	if(curfirstcol == 0)
		curfirstcol = 1;

	// if the first thing is a close brace or close paren, find the
	// line that matches the brace and use its indent
	//
	if(
				(firstdelim == _T('}'))
			||	(firstdelim == _T(')'))
			|| ((firstdelim == m_macro_prefix) && (firstdelimcol == curfirstcol))
	)
	{
		int bline = line - 1, bcol = MAX_LINE_LEN;
		TCHAR ds[256], md[256];

		ds[0] = firstdelim;
		ds[1] = _T('\0');

		if((ec = GetMatchingPhrase(ds, md, 256)) == errOK)
		{
			if((ec = FindMatchingPhrase(ds, md, bline, bcol)) == errOK)
			{
				line = bline;
				matching = true;
			}
		}
	}
	if(! matching)
	{
		// can't cheat, so start looking from current line
		// else line is already set to matching line
		//
		line--;
	}
	if(line < 1)
	{
		line = sl, col = sc;
		return errOK;
	}
	// if the previous line is part of a spanning comment
	// then so are we, so just match the indent of the
	// previous line exactly
	//
	BlineInfo info;

	info	 = m_buffer->GetLineCommentInfo(line);
	nonblank = false;

	if(info == liInSpanning || info == liStartsSpanning || matching)
	{
		ec = GetFirstNonBlankColumn(
										line,
										prevfirstcol,
										true,
										matching && (firstdelim == m_macro_prefix),
										prevlastdelim,
										prevfirstdelim,
										firstdelimkw,
										prevfirstdelimcol,
										prevparens,
										prevbraces,
										prevcolons
										);
		if(ec != errOK)
		{
			line = sl; col = sc;
			return ec;
		}
		if(prevfirstcol > 0) nonblank = true;
		// turn off adjustments
		parens = braces = colons = 0;
		// indicate 1-for-1 match of line's indent
		matching = true;
		prevnb = true;
		protoline = line;
	}
	else
	{
		TCHAR sawEOS = _T('\0');

		// look back in buffer for last non-blank non-comment non-label
		// line and get the column number of the first non-white
		//
		for(prevnb = false, sawEOS = _T('\0'); ! nonblank && line >= 1; line--)
		{
			ec = GetFirstNonBlankColumn(
										line,
										prevfirstcol,
										false,
										firstdelim == m_macro_prefix,
										prevlastdelim,
										prevfirstdelim,
										firstdelimkw,
										prevfirstdelimcol,
										prevparens,
										prevbraces,
										prevcolons
										);
			if(ec != errOK)
			{
				line = sl; col = sc;
				return ec;
			}
			label = false;
			//_tprintf(_T("pl %d  pc=%d  fd=%c ld=%c\n"), line, prevcolons, prevfirstdelim, prevlastdelim);
			if(prevfirstcol > 0)
			{
				// non blank line
				if(prevlastdelim == m_statement_term)
				{
					// saw a line ending with statement term so accumulate
					sawEOS = m_statement_term;
				}
				else if (prevcolons == 1 && prevlastdelim == ':' &&
						prevfirstcol == 1 && prevparens == 0 && prevbraces == 0)
				{
					// saw a line ending with colon, if it starts at left assume label
					label = true;
				}
				if(! label && (prevparens >= 0 || prevfirstdelim == ')'))
				{
					// line has >= 0 parens, so could be the start line unless the
					// firstdelim is an )
					//
					sawEOS = prevlastdelim;
					prevnb = true;
					break;
				}
			}
		}
		label = false;
		prevlastdelim = sawEOS;

		// as a bonus, look at previous lines to first prev nblank one, and
		// see if it is an "if", "else" or "for", etc. without a brace
		// in which case adjust indent back one
		//
		protoline = line;

		if(! matching && prevnb && line > 1 && prevbraces == 0)
		{
			TCHAR	xprevlastdelim;
			TCHAR	xprevfirstdelim;
			int		xprevfirstcol, xprevfirstdelimcol;
			int		xparens, xbraces, xcolons, totparens;
			int		prevline = line;


			for(statnb = false, line--, totparens = 0; ! statnb && line >= 1;)
			{
				ec = GetFirstNonBlankColumn(
											line,
											xprevfirstcol,
											false,
											false,
											xprevlastdelim,
											xprevfirstdelim,
											firstdelimkw,
											xprevfirstdelimcol,
											xparens,
											xbraces,
											xcolons
											);
				if(ec != errOK)
					break;
				totparens += xparens;
				if(xprevfirstcol)
				{
					nStatement   = 128;
					statementcol = 1;

					if((xprevlastdelim == m_statement_term) || xbraces)
					{
						// a real statement seen, no chance
						break;
					}
					ec = GetStatement(line, statementcol, true, kw, statement, nStatement);

					if(ec == errOK)
					{
						if(totparens <= 0)
						{
							// got a statement that we arent part of above the line above
							// the current line, that is ok
							//
							statnb = true;
						}
						else
						{
							// got a statement that we are apparently part of, so
							// just use the regular line indent unless parens
							// are complete
							//
							if((totparens + prevparens) > 0)
								prevlastdelim = m_statement_term;
						}
						break;
					}
				}
				line--;
			}
			if(statnb)
			{
				// line had a builtin, see if there was a brace after it, and if
				// there is, or there is a statement term, ignore it
				//
				// also make sure the first previous line had no unbraced statement
				// on it,
				//
				nStatement   = 128;
				statementcol = 1;
				ec = GetStatement(prevline, statementcol, false, kw, statement, nStatement);
				if(ec == errOK)
				{
					// first prev line does have a statement on it, but disqualify it
					// if it has a line terminator
					//
					if(prevlastdelim == m_statement_term)
					{
						ec = errFAILURE;
					}
				}
				if(
						ec != errOK							&&
						xbraces == 0						&&
						xprevlastdelim != m_statement_term	&&
						xprevlastdelim != _T(':')
				)
				{
					// use THIS line as the prototype for indent
					// but disable missing line term redent
					// if full parens there
					//
					protoline = line;
					prevfirstcol = xprevfirstcol;
					if(totparens <= 0)
					{
						// accumulated even parens looking back, so
						// ignore mismatched parens on actual statement line
						//
						prevparens = 0;
					}
					prevfirstdelim = xprevfirstdelim;
					prevfirstdelimcol = xprevfirstdelimcol;
				}
				ec = errOK;
			}
		}
		if(prevnb && colons > 0)
		{
			// also, if there was a colon, see if the first statement is a "case"
			// in which case we indent based on the switch statement, which conveniently
			// has to have a { after it
			//
			nStatement   = 128;
			statementcol = 1;

			ec = GetStatement(sl, statementcol, false, kw, statement, nStatement);

			if(ec == errOK)
			{
				if(
					! _tcscmp(statement, _T("case")) || ! _tcscmp(statement, _T("default")) ||
					! _tcscmp(statement, _T("public")) || ! _tcscmp(statement, _T("private")) ||
					! _tcscmp(statement, _T("protected"))
				)
				{
					// got a case statement, so find the last open brace line
					// and use it as the prototype
					//
					int bline = sl - 1, bcol = MAX_LINE_LEN;

					if((ec = FindMatchingPhrase(_T("}"), _T("{"), bline, bcol)) == errOK)
					{
						line		= bline;
						bcol		= 1;
						matching	= true;
						ec = GetFirstNonBlankColumn(
													bline,
													prevfirstcol,
													true,
													false,
													prevlastdelim,
													prevfirstdelim,
													firstdelimkw,
													prevfirstdelimcol,
													prevparens,
													prevbraces,
													prevcolons
												);
						if(ec == errOK)
						{
							protoline = bline;
						}
					}
				}
			}
			else
			{
				// might be a label, if there is only one token on line
				// and no-white space betweeen it and colon, its a label
				//
				GetIsLabel(sl, label);
			}
		}
	}

	// use the prototype line as a template to find the basal indent
	// and convert the leading white space to all spaces
	//
	ec = m_buffer->GetLineText(protoline, lpText, nText);
	if(ec != errOK) return ec;

	for(incol = protocol = 0; incol < nText; incol++)
	{
		if(lpText[incol] == ' ')
		{
			protocol++;
		}
		else if(lpText[incol] == '\t')
		{
			protocol = ((protocol / m_tabspace) * m_tabspace) + m_tabspace;
		}
		else
		{
			break;
		}
		//_tprintf(_T("protocol =%d on c=%02X[%d]\n"), protocol, lpText[incol], incol);
	}

	// restore postion to target line
	//
	line = sl;
	col  = sc;

	if(! prevnb)
		return errOK;

	// assume indent exactly like prototype line
	//
	netdent = 0;

	if(label)
	{
		protocol = 0;
		netdent = 0;
	}
	else if(! matching)
	{
		// add an indent space if the proto previous line has more
		// right parens than left, and if the current line has more close parens
		//
		if(prevparens > 0)
			netdent++;
		if(parens < 0)
			netdent++;

		// subtract an indent space if a closing brace is first
		// and isn't matched
		//
		if(lastdelim == _T('}') && braces)
			netdent--;

		if(
			(m_statement_term && (prevlastdelim != m_statement_term))	&&
			prevlastdelim != _T(',')									&&
			prevlastdelim != _T('}')
		)
		{
			// if this line starts a block, don't indent
			// if the previous line ends a block, don't indent
			//
			if(
					firstdelimcol <= 0			||
					firstdelim != _T('{')		||
					prevlastdelim == _T('{')
			)
				netdent++;
		}
	}
	// adjust target lines indent target
	//
	protocol += (netdent * m_indentspace);

	// found the proper (hopefully) indentation for this line, first remove all
	// existing indentation, since it can be replaced cleaner
	//
	// remember offset of current column from first non-blank column
	//
	coldelta = 0;

	do
	{
		ec = m_buffer->GetLineText(line, lpText, nText);
		if(ec != errOK) return ec;

		if(nText <= 1 || (lpText[0] != ' ' && lpText[0] != _T('\t')))
			break;

		coldelta--;
		cutcol = 1;
		ec  = m_buffer->Delete(line, cutcol, 1);
		if(ec != errOK) return ec;
	}
	while(nText > 1);

	//_tprintf(_T("indent to col:%d adj=%d\n"), protocol, netdent);

	// now that the line is indent free, add tabs and/or spaces to
	// get first non-blank char at proper spot
	//
	for(incol = outcol = 0; incol < protocol && outcol < (sizeof(m_tabBuf)/sizeof(TCHAR));)
	{
		if(
				(protocol - incol) >= m_tabspace		// need to add at least a tab
			&&	! m_tabsasspaces						// and not using spaces for tabs
		)
		{
			//_tprintf(_T("  add tab out=%d in=%d\n"), outcol, incol);
			m_tabBuf[outcol++] = _T('\t');
			incol += m_tabspace - (incol % m_tabspace);
		}
		else
		{
			//_tprintf(_T("  add space out=%d in=%d\n"), outcol, incol);
			m_tabBuf[outcol++] = _T(' ');
			incol++;
		}
	}
	cutcol = 1;
	coldelta += outcol;
	ec = m_buffer->Insert(line, cutcol, m_tabBuf, outcol);
	col = coldelta + sc;
	return errOK;
}

