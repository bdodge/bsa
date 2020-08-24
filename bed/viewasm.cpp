
#include "bedx.h"


//**************************************************************************
BviewASM::BviewASM(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel)
		:
		Bview(pBuf, pEditor, pPanel)
{
	m_statement_term = _T('\0');
	m_macro_prefix = _T('.');
		
	m_statement_term	= _T(';');
	AddComment(_T("/*"), tsSpanningComment);
	AddComment(_T("*/"), tsBase);
	AddComment(_T("//"), tsComment);
	AddComment(_T(";"), tsComment);
}

//**************************************************************************
ERRCODE BviewASM::Key(WORD key)
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
			ec = Dispatch(::Indent);
			break;
		case ':': 
			ec = Dispatch(::Indent);
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
bool BviewASM::IsDelim(TCHAR ic)
{
	// colons are not delims, they make lables!
	if(ic != ':')
		return Bview::IsDelim(ic);
	else
		return false;
}

//**************************************************************************
TokenRet BviewASM::GetToken(
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

	if(kw == kwOperator && nToken && lpToken[0] == m_macro_prefix)
	{
		if(m_first_token)
			kw = kwMacro;
		else
			kw = kwSpecial;
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
		if(
				((lpToken[nToken - 1] == ':')  ||  ((incol - nToken) <= 1))
			&&	state == tsBase
		)
		{
			kw = kwBuiltinType;
		}
	}
	return tr;
}

//**************************************************************************
ERRCODE BviewASM::EnumFunctions(EnumFuncsCallback pCallback, LPVOID cookie, Blog* pLog)
{
	ERRCODE		ec;
	TokenRet	tr;
	LPTSTR		lpText, lpToken, lpPrevToken;
	int			nText,  nToken,  nPrevToken;
	int			line;
	int			incol, outcol, tokcol;
	TokenState	state;
	BkwType		kw;
	TCHAR		svc;
	
	line	= 1;

	enum
	{
		psBase
	}
	parseState	= psBase;	
	state 		= tsBase;

	do
	{
		ec = m_buffer->GetLineText(line, (LPCTSTR&)lpText, nText);
		if(ec != errOK) return errOK;
		
		incol	 = 1;
		outcol	 = 1;

		lpPrevToken = NULL;
		nPrevToken = 0;
		
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
					case kwBuiltinType:

						if (nToken > 1)
						{
							svc = lpToken[nToken - 1];
							lpToken[nToken - 1] = _T('\0');
							ec = pCallback(cookie, lpToken, _T(""), 0, line, efFunction);
							lpToken[nToken - 1] = svc;
						}
						break;

					case kwPlain:
					case kwMacro:
					case kwOperator:

						switch(lpToken[0])
						{
						case _T(':'):
							if(nPrevToken && lpPrevToken)
							{
								svc = lpPrevToken[nPrevToken];
								lpPrevToken[nPrevToken] = _T('\0');
								ec = pCallback(cookie, lpPrevToken, _T(""), 0, line, efFunction);
								lpPrevToken[nPrevToken] = svc;
							}
							break;
						default:
							break;
						}

					default:
						break;
					}
					break;
					
				}
				if(NonWhite(lpToken, nToken))
				{
					nPrevToken  = nToken;
					lpPrevToken = lpToken;
				}
			}
		}
		while(ec == errOK);

		if(ec != errOK) 
		{
			break;
		}
		line++;

	} while(ec == errOK);

	return errOK;
}


//**************************************************************************
ERRCODE BviewASM::HasLabel(int line, int& colons)
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
					for(sc = 1;	sc < nToken; sc++)
					{
						if(lpToken[sc] == _T(':'))
						{
							sc++;
							if(sc >= nToken)
							{
								colons++;
								break;
							}
							if(lpToken[sc] == _T(' ') || lpToken[sc] == _T('\t') || lpToken[sc] == _T('\n'))
							{
								colons++;
								break;
							}
						}
					}
				}
			}
		}
	}
	while(tr == trOK && colons == 0);

	return errOK;
}

//**************************************************************************
ERRCODE BviewASM::Indent(WORD key, int& line, int& col)
{
	ERRCODE 	ec;
	int  		sl, sc;
	bool 		nonblank, prevnb;
	int			prevfirstcol;
	TCHAR		prevlastdelim;
	TCHAR		prevfirstdelim;
	int			prevfirstdelimcol;
	int			parens, braces, colons;
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
	
	// if this line has a label, just get rid of white space
	//
	colons = 0;
	ec = HasLabel(sl, colons);
	if(ec != errOK) return ec;
	
	if(colons > 0)
	{
		protoline = line;
		protocol  = 1;
		prevnb = nonblank = true;
		netdent = 0;
	}
	else
	{
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
			// look back in buffer for last non-blank, non-comment, non-label
			// line and get the column number of the first non-white
			//
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
					// line non-blank, see if its got a colon
					//
					ec = HasLabel(line, colons);
					if(ec != errOK)
					{
						line = sl; col = sc;
						return ec;
					}
					if(! colons)
					{
						prevnb = true;
						break;
					}
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
	}
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

