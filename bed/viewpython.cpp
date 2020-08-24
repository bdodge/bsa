
#include "bedx.h"

//**************************************************************************
BviewPython::BviewPython(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel)
		:
		BviewCPP(pBuf, pEditor, pPanel)
{
	m_keywords = NULL; // dont inherit C's static keywords

	if(m_comments)
		BcommentList::FreeList(m_comments);
	m_comments = NULL;
	if(m_prefixes)
		BprefixList::FreeList(m_prefixes);
	m_prefixes = NULL;
	AddComment(_T("#"), tsComment);
	AddTags();
}

//**************************************************************************
BviewPython::~BviewPython()
{
	delete m_keywords;
	m_keywords = NULL;
}

//**************************************************************************
void BviewPython::AddTags(void)
{
	AddKeyword(_T("and"), kwBuiltin, true);
	AddKeyword(_T("del"), kwBuiltin, true);
	AddKeyword(_T("from"), kwBuiltin, true);
	AddKeyword(_T("not"), kwBuiltin, true);
	AddKeyword(_T("while"), kwBuiltin, true);
	AddKeyword(_T("as"), kwBuiltin, true);
	AddKeyword(_T("elif"), kwBuiltin, true);
	AddKeyword(_T("global"), kwBuiltin, true);
	AddKeyword(_T("or"), kwBuiltin, true);
	AddKeyword(_T("with"), kwBuiltin, true);
	AddKeyword(_T("assert"), kwBuiltin, true);
	AddKeyword(_T("else"), kwBuiltin, true);
	AddKeyword(_T("if"), kwBuiltin, true);
	AddKeyword(_T("pass"), kwBuiltin, true);
	AddKeyword(_T("yield"), kwBuiltin, true);
	AddKeyword(_T("break"), kwBuiltin, true);
	AddKeyword(_T("except"), kwBuiltin, true);
	AddKeyword(_T("import"), kwBuiltin, true);
	AddKeyword(_T("print"), kwBuiltin, true);
	AddKeyword(_T("class"), kwBuiltin, true);
	AddKeyword(_T("exec"), kwBuiltin, true);
	AddKeyword(_T("in"), kwBuiltin, true);
	AddKeyword(_T("raise"), kwBuiltin, true);
	AddKeyword(_T("continue"), kwBuiltin, true);
	AddKeyword(_T("finally"), kwBuiltin, true);
	AddKeyword(_T("is"), kwBuiltin, true);
	AddKeyword(_T("return "), kwBuiltin, true);
	AddKeyword(_T("def"), kwBuiltin, true);
	AddKeyword(_T("for"), kwBuiltin, true);
	AddKeyword(_T("lambda"), kwBuiltin, true);
	AddKeyword(_T("try"), kwBuiltin, true);
}

#define MAX_PY_NEST 32
#define MAX_PY_NAME 64

//**************************************************************************
ERRCODE BviewPython::EnumFunctions(EnumFuncsCallback pCallback, LPVOID cookie, Blog* pLog)
{
	ERRCODE		ec;
	TokenRet	tr;
	LPTSTR		lpText, lpToken;
	int			nText,  nToken;
	int			line, level;
	int			incol, outcol, tokcol;
	int			indent[MAX_PY_NEST], curindent;
	TCHAR		classstack[MAX_PY_NEST][MAX_PY_NAME];
	bool		firsttok;
	TokenState	state;
	BkwType		kw;

	enum {	psBase,
			psFuncSpec,
			psFunc,
			psClassSpec,
			psClass
	} parseState[MAX_PY_NEST];

	// run the whole enum loop protected to avoid reentrant issues
	// (the buffer is shared among multiple views)
	static Bmutex lockenum;

	lockenum.Lock();

	parseState[0] = psBase;
	level = 0;

	line	= 1;
	memset(classstack, 0, sizeof(classstack));

	do
	{
		ec = m_buffer->GetLineText(line, (LPCTSTR&)lpText, nText);
		if(ec != errOK)
		{
			ec = errOK;
			break;
		}

		incol	 = 1;
		outcol	 = 1;
		state 	 = tsBase;

		curindent = 1;
		indent[0] = 1;
		firsttok = true;

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

			if(kw == kwPlain)
				kw = KeyWord(lpToken, nToken);

			if(
					(state != tsSpanningComment)		&&
					(kw != kwComment)					&&
					(lpToken[0] != _T(' '))				&&
					(lpToken[0] != _T('\t'))
			)
			{
				TCHAR x = lpToken[nToken];

				lpToken[nToken] = 0;
			/*
				_tprintf(_T("tok="_Pfs_" len=%d type=%d f=%d ci=%d,tc=%d state = %d[%d]\n"),
						lpToken, nToken, kw, firsttok, curindent, tokcol, parseState[level], level);
			*/
				// the first non-blank token on the line's outcol is the line's
				// indent level.  if that's changed, perhaps change state
				//
				if(firsttok)
				{
					int startlevel = level;

					//printf("tc=%d cd=%d\n", tokcol, curindent);
					curindent = tokcol;

					while(curindent <= indent[level] && level > 0)
					{
						switch(parseState[level])
						{
						case psClass:
							ec = pCallback(cookie, _T(""), _T(""), 0, line, efEndClass);
							break;
						default:
							break;
						}
						parseState[level] = psBase;
						if (level > 0)
						{
							level--;
							//_tprintf(_T(" pop to state %d[%d]\n"), parseState[level], level);
						}
					}
					if(level >= 0 && level < startlevel)
					{
						if(parseState[level] == psClass)
						{
							// set pdb's current class to what it was at lower level
							//
							ec = pCallback(cookie, classstack[level],
									classstack[level], 0, line, efSetClass);
						}
						else
						{
							// back to global scop
							//
							ec = pCallback(cookie, NULL, NULL, 0, line, efSetClass);
						}
					}
				}
				firsttok = false;

				switch(parseState[level])
				{
				case psBase:

					switch(kw)
					{
					case kwBuiltin:

						if(! KeywordCompare(lpToken, _T("class"), nToken, 5, true))
						{
							if (level < (MAX_PY_NEST - 1))
							{
								parseState[++level] = psClassSpec;
								indent[level]  = curindent;
							}
						}
						if(! KeywordCompare(lpToken, _T("def"), nToken, 3, true))
						{
							if (level < (MAX_PY_NEST - 1))
							{
								parseState[++level] = psFuncSpec;
								indent[level]   = curindent;
							}
						}
						break;

					default:
						break;
					}
					break;

				case psClassSpec:

					if(kw == kwPlain || kw == kwAddonType)
					{
						parseState[level] = psClass;
						_tcsncpy(classstack[level], lpToken, MAX_PY_NAME - 1);
						classstack[level][MAX_PY_NAME - 1] = _T('\0');
						ec = pCallback(cookie, lpToken, lpToken, 0, line, efClass);
					}
					break;

				case psClass:

					if (kw == kwBuiltin)
					{
						if(! KeywordCompare(lpToken, _T("class"), nToken, 5, true))
						{
							if (level < (MAX_PY_NEST - 1))
							{
								parseState[++level] = psClassSpec;
								indent[level]  = curindent;
							}
						}
						if(! KeywordCompare(lpToken, _T("def"), nToken, 3, true))
						{
							if (level < (MAX_PY_NEST - 1))
							{
								parseState[++level] = psFuncSpec;
								indent[level]   = curindent;
							}
						}
					}
					break;

				case psFuncSpec:

					if(kw == kwPlain)
					{
						parseState[level] = psFunc;
						ec = pCallback(cookie, lpToken, lpToken, 0, line, efFunction);
					}
					break;

				case psFunc:

					if (kw == kwBuiltin || kw == kwAddonFunc)
					{
						if(! KeywordCompare(lpToken, _T("class"), nToken, 5, true))
						{
							if (level < (MAX_PY_NEST - 1))
							{
								parseState[++level] = psClassSpec;
								indent[level]  = curindent;
							}
						}
						/*
						if(! KeywordCompare(lpToken, _T("def"), nToken, 3, true))
						{
							if (level < (MAX_PY_NEST - 1))
							{
								parseState[++level] = psFuncSpec;
								indent[level]   = curindent;
							}
						}
						*/
					}
					break;

				default:
					break;
				}
				lpToken[nToken] = x;
			}
		}
		while(ec == errOK);

		if(ec != errOK) break;
		line++;

	} while(ec == errOK);

	lockenum.Unlock();

	return errOK;
}

//**************************************************************************
ERRCODE BviewPython::StartsBlock(int line, int& colons)
{
	TokenRet	tr;
	ERRCODE		ec;
	LPCTSTR 	lpText;
	int			nText;
	LPTSTR		lpToken;
	int			nToken;
	TokenState 	state;
	int			incol, outcol, tokcol, col, sc;
	BlineInfo	info;
	BkwType		kw;

	ec = m_buffer->GetLineText(line, lpText, nText);
	if(ec != errOK) return ec;

	incol	 = 1;
	outcol	 = 1;
	col 	 = 0;


	info = m_buffer->GetLineCommentInfo(line);

	if(info == liInSpanning || info == liEndsSpanning)
		state = tsSpanningComment;
	else
		state = tsBase;

	colons = 0;

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

		if(tr == trOK && nToken > 0)
		{
			if(state == tsComment)
				break;

			if(state != tsSpanningComment && kw != kwComment && kw != kwQuoted)
			{
				if(NonWhite(lpToken, nToken))
				{
					// any non-white, non-comment token after a colon resets count
					colons = 0;
					for(sc = 0;	sc < nToken; sc++)
					{
						if(lpToken[sc] == _T(':') && (sc + 1) == nToken)
						{
							colons = 1;
						}
					}
				}
			}
		}
	}
	while(tr == trOK);

	return errOK;
}

//**************************************************************************
ERRCODE BviewPython::Indent(WORD key, int& line, int& col)
{
	ERRCODE 	ec;
	int  		sl, sc;
	bool 		nonblank, prevnb;
	int			prevfirstcol;
	TCHAR		prevlastdelim;
	TCHAR		prevfirstdelim;
	int			prevfirstdelimcol;
	int			colons;
	int			prevparens, prevbraces, prevcolons;
	int			incol, outcol, protoline, protocol;
	int			netdent;

	BkwType		firstdelimkw;

	bool		matching = false;

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

	line--;
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
										matching,
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
		if(prevfirstcol > 0)
		{
			nonblank = true;
		}
		// indicate 1-for-1 match of line's indent
		matching = true;
		prevnb = true;
		protoline = line;
	}
	else
	{
		// look back in buffer for last non-blank, non-comment, non-label
		// line and get the column number of the first non-white
		//
		colons = 0;
		for(prevnb = false; ! nonblank && line >= 1; line--)
		{
			ec = GetFirstNonBlankColumn(
										line,
										prevfirstcol,
										false,
										true,
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
			if(prevfirstcol > 0)
			{
				// line non-blank, see if its got a colon at end
				//
				ec = StartsBlock(line, colons);
				if(ec != errOK)
				{
					line = sl; col = sc;
					return ec;
				}
				prevnb = true;
				break;
			}
		}
		protoline = line;
	}
	if(protoline < 1)
	{
		line = sl; col = sc;
		return errOK;
	}
	// use the prototype line as a template to find the basal indent
	// and convert the leading white space to all spaces
	//
	ec = m_buffer->GetLineText(protoline, lpText, nText);
	if(ec != errOK) return ec;

	// calculate the net indentation by converting to all spaces
	//
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

	netdent = 0;

	if(! matching)
	{
		// add an indent space if the proto previous line has more right parens than left,
		// and subtract one if the current line has more close parens
		//
		if(prevparens > 0)
		{
			netdent++;
		}
		if(prevparens < 0)
		{
			netdent--;
		}
		// add an indent space if prev-nb ends in colon
		if (colons > 0)
		{
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
	if(protocol <= 1)
	{
		// no indent at all, just return
		col = sc + coldelta;
		return errOK;
	}
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

