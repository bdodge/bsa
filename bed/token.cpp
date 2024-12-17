
#include "bedx.h"

//**************************************************************************
bool Bview::IsDelim(TCHAR ic)
{
	if(
		ic == _T(' ')  || ic == _T('\t') || ic == _T('\n') || ic == _T('(')  ||
		ic == _T(')')  || ic == _T(',')  || ic == _T(';')  || ic == _T('+')  ||
		ic == _T(':')  || ic == _T('<')  || ic == _T('>')  || ic == _T('.')  ||
		ic == _T('-')  || ic == _T('/')  || ic == _T('\\') || ic == _T('*')  ||
		ic == _T('^')  || ic == _T('|')  || ic == _T('"')  || ic == _T('\'') ||
		ic == _T('{')  || ic == _T('}')  || ic == _T('[')  || ic == _T(']')  ||
		ic == _T('&')  || ic == _T('=')  || ic == _T('%')  || ic == _T('@')
	)
		return true;
	else
		return false;
}

//**************************************************************************
bool Bview::StartsComment(LPCTSTR lpText, int nText, int& nComment, TokenState& state)
{
	BcommentList* pComment;
	int			  len;
	TokenState	  nstate;

	for(pComment = m_comments; pComment; pComment = pComment->GetNext(pComment))
	{
		len    = _tcslen(pComment->GetName());
		nstate = *pComment->GetValue();

		if(
				(state == tsSpanningComment && nstate == tsBase)	||
				(state == tsBase && (nstate == tsSpanningComment || nstate == tsComment))
		)
		{
			if(len <= nText)
			{
				if(! _tcsncmp(lpText, pComment->GetName(), len))
				{
					nComment = len;
					state	 = *pComment->GetValue();
					return true;
				}
			}
		}
	}
	return false;
}

//**************************************************************************
bool Bview::EndsComment(LPCTSTR lpText, int nText, int& nComment, TokenState& state)
{
	return StartsComment(lpText, nText, nComment, state);
}

//**************************************************************************
void Bview::AddComment(LPCTSTR lpText, TokenState state)
{
	BcommentList* pComment;
	TokenState*   nstate;

	nstate = new TokenState [ 1 ];

	*nstate  = state;
	pComment = new BcommentList(lpText, nstate);
	m_comments = BcommentList::AddToList(pComment, m_comments);
}

//**************************************************************************
void Bview::AddPrefix(LPCTSTR lpPrefix, BkwType type)
{
	BprefixList* pPref;
	BkwType*     ntype;

	ntype = new BkwType [ 1 ];

	*ntype  = type;
	pPref	= new BprefixList(lpPrefix, ntype);
	m_prefixes = BprefixList::AddToList(pPref, m_prefixes);
}

//**************************************************************************
TokenRet Bview::GetToken(
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
	bool  gotToken	 = false;
	bool  whitespace = false;
	int   inc		 = incol - 1;
	int   nComment;

	lpToken = lpText + inc;
	nToken  = 0;

	if(! lpText || inc < 0) return trFAILURE;
	if(inc >= nText)		return trEOLLINE;

	if(state == tsComment || state == tsSpanningComment)
		kw = kwComment;
	else if(state == tsQuotedString || state == tsQuotedLiteral)
		kw = kwQuoted;
	else if(state == tsMacro)
		kw = kwMacro;
	else
		kw = kwPlain;

	while(! gotToken && (inc < nText))
	{
		switch(lpText[inc])
		{
		case _T(' '):

			if(state != tsQuotedString && state != tsQuotedLiteral)
			{
				if(nToken)
				{
					gotToken = true;
					break;
				}
				whitespace = true;
				if(m_showtabs)
				{
					if (nToken >= MAX_TAB_SPACE)
					{
						gotToken = true;
						break;
					}
					lpToken = m_tabBuf;
					m_tabBuf[nToken] = m_showtabs ? _T('-') : _T(' ');
				}
			}
			nToken++;
			outcol++;
			inc++;
			break;

		case _T('\t'):

			gotToken = true;
			if(nToken)	break;

			if(m_showtabs)
			{
				lpToken = m_tabBuf;
				m_tabBuf[nToken++] = m_showtabs ? _T('^') : _T(' ');
				outcol++;
				while((outcol - 1) % m_tabspace && nToken < MAX_TAB_SPACE)
				{
					m_tabBuf[nToken++] = m_showtabs ? _T('-') : _T(' ');
					outcol++;
				}
				m_tabBuf[nToken] = _T('\0');
			}
			else
			{
				nToken++;
				outcol++;
			}
			inc++;
			break;

		case _T('\"'):

			if(state != tsQuotedString && state != tsQuotedLiteral)
			{
				if(nToken)
				{
					gotToken = true;
					break;
				}
			}
			// only views that can have comments can have quotes (keeps quote parsing out of plain text)
			if(m_comments && (state != tsQuotedLiteral))
			{
				if(state == tsQuotedString)
					if(inc <= 0 || lpText[inc - 1] != _T('\\') || (inc < 2 || (lpText[inc - 2] == _T('\\'))))
						state = tsBase;
				if(kw == kwPlain)
				{
					state = tsQuotedString;
					kw = kwQuoted;
				}
			}
			inc++;
			outcol++;
			nToken++;
			break;

		case _T('\''):

			if(state != tsQuotedString && state != tsQuotedLiteral)
			{
				if(nToken)
				{
					gotToken = true;
					break;
				}
			}
			// only views that can have comments can have quotes (keeps quote parsing out of plain text)
			if(m_comments && (state != tsQuotedString))
			{
				gotToken = true;
				if(state == tsQuotedLiteral)
					if(inc <= 0 || lpText[inc - 1] != _T('\\') || (inc < 2 || (lpText[inc - 2] == _T('\\'))))
						state = tsBase;
				if(kw == kwPlain)
				{
					state = tsQuotedLiteral;
					kw = kwQuoted;
				}
			}
			inc++;
			outcol++;
			nToken++;
			break;

		case _T('\n'): case _T('\r'): case _T('\0'):

			if(nToken)
			{
				gotToken = true;
				break;
			}
			if(state == tsComment || state == tsMacro)
				state = tsBase;
			if(1 && (state == tsQuotedString || state == tsQuotedLiteral))
				state = tsBase;
			return 	trEOLLINE;

		case 0x1b: // esc

			gotToken = true;
			if(nToken)
				break;
			kw = kwSpecial;
			_tcscpy(m_tabBuf, _T("[esc]"));
			lpToken = m_tabBuf;
			nToken = 5;
			outcol++;
			inc++;
			break;

		case 0xc: // formfeed

			gotToken = true;
			if(nToken)
				break;
			kw = kwSpecial;
			_tcscpy(m_tabBuf, _T("[formfeed]"));
			lpToken = m_tabBuf;
			nToken = 10;
			outcol++;
			inc++;
			break;

		case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8:
		case 11: case 14: case 15: case 16: case 17: case 18: case 19:
		case 20: case 21: case 22: case 23: case 24: case 25: case 26:

			gotToken = true;
			if(nToken)
				break;
			kw = kwSpecial;
			m_tabBuf[0] = _T('^');
			m_tabBuf[1] = lpText[inc] - 1 + 'A';
			lpToken = m_tabBuf;
			nToken = 2;
			outcol++;
			inc++;
			break;

		case 28: case 29: case 30: case 31:

			gotToken = true;
			if(nToken)
				break;
			kw = kwSpecial;
			switch(lpText[inc])
			{
			case 28: _tcscpy(m_tabBuf, _T("FS")); break;
			case 29: _tcscpy(m_tabBuf, _T("GS")); break;
			case 30: _tcscpy(m_tabBuf, _T("RS")); break;
			case 31: _tcscpy(m_tabBuf, _T("US")); break;
			}
			lpToken = m_tabBuf;
			nToken = 2;
			outcol++;
			inc++;
			break;

		default:

			// strange comments that are non-delims
			if(whitespace)
			{
				gotToken = true;
			}
			else if(IsDelim(lpText[inc]) && state != tsQuotedString && state != tsQuotedLiteral)
			{
				if(nToken)
				{
					gotToken = true;
					break;
				}
				if(state == tsBase && m_comments &&
						StartsComment(lpText + inc, nText - inc, nComment, state))
				{
					kw	   = kwComment;
					nToken = nComment;
					inc	   += nComment;
					outcol += nComment;
				}
				else if(state == tsSpanningComment && m_comments &&
						EndsComment(lpText + inc, nText - inc, nComment, state))
				{
					gotToken = true;
					kw		 = kwComment;
					nToken	 = nComment;
					inc		+= nComment;
					outcol	+= nComment;
				}
				else
				{
					gotToken = true;
					if(kw == kwPlain)
						kw = kwOperator;
					else if(state == tsMacro)
						kw = kwMacro;
					outcol++;
					inc++;
					nToken++;
				}
			}
			else
			{
				if(state == tsBase && m_comments &&
						StartsComment(lpText + inc, nToken + 1, nComment, state))
				{
					kw	   = kwComment;
					nToken = nComment;
					inc	   += nComment;
					outcol += nComment;
				}
				else
				{
					nToken++;
					outcol++;
					inc++;
				}
			}
			break;
		}
	}

	incol   = inc + 1;

	//_tprintf(_T("tok=" _Pfs_ "=%d\n"), lpToken, nToken);

	return trOK;
}

//**************************************************************************
TokenRet Bview::GetToken(
					LPTSTR&		lpText,
					int&		nText,
					int&		line,
					int&		incol,
					int&		outcol,
					TokenState&	state,
					LPTSTR&		lpToken,
					int&		nToken,
					HFONT&		hFont,
					COLORREF&	frgColor,
					COLORREF&	bkgColor
					)
{
	TokenRet tr;
	BkwType kw;

	tr = GetToken(lpText, nText, line, incol, outcol, state,
			lpToken, nToken, kw);

	if(tr != trOK)
		return tr;

	if(kw == kwPlain)
		kw = KeyWord(lpToken, nToken);

	if(kw == kwMacro)
		state = tsMacro;

	// suppress color changes for quoted text if plaintext buffer
	//
	if(kw == kwQuoted && m_buffer && m_buffer->GetType() == btText)
	{
		kw = kwPlain;
	}
	frgColor = m_view_colors[kw];
	bkgColor = m_view_colors[kwBackground];

	if(kw > kwSpecial)
		return trOK;

	hFont = m_view_fonts[kw];
	return tr;
}


// STATIC ******************************************************************
int Bview::KeywordCompare(LPCTSTR lpA, LPCTSTR lpB, int nA, int nB, bool casesensi)
{
	while(nA > 0 && nB > 0)
	{
		TCHAR a, b;

		a = *lpA;
		b = *lpB;

		if(! casesensi)
		{
			if(a >= 'A' && a <= 'Z')
				a = a + (_T('a') - _T('A'));
			if(b >= 'A' && a <= 'Z')
				b = b + (_T('a') - _T('A'));
		}
		if(! a || ! b)   break;
		if(a < b)		 return -1;
		if(a > b)		 return  1;
		lpA++;
		lpB++;
		nA--;
		nB--;
	}
	if(nA == 0 && nB == 0) return 0;
	if(nA > nB) return 1;
	else return -1;
}

//**************************************************************************
tagKeyword::tagKeyword(LPCTSTR lpWord, BkwType type, bool casesensi)
{
	m_word  = new TCHAR [ _tcslen(lpWord) + 1];
	_tcscpy(m_word, lpWord);
	m_type  = type;
	m_sensi = casesensi;
	m_left  = m_right = NULL;
}

//**************************************************************************
tagKeyword::~tagKeyword()
{
	if(m_left)  delete m_left;
	if(m_right) delete m_right;
	delete [] m_word;
}

//**************************************************************************
LPKEYWORD tagKeyword::AddKeyword(LPKEYWORD pKey, LPKEYWORD& pList)
{
	int cmp;

	if(! pList)
		return pList = pKey;

	cmp = _tcscmp(pKey->m_word, pList->m_word);

	if(cmp < 0)
		return AddKeyword(pKey, pList->m_left);
	else if(cmp > 0)
		return AddKeyword(pKey, pList->m_right);
	else
		delete pKey;
	return pList;
}

//**************************************************************************
LPKEYWORD tagKeyword::FindKeyword(LPCTSTR lpWord, int nWord, LPKEYWORD pList)
{
	int cmp;

	if(! pList || ! pList->m_word)
		return NULL;

	cmp = _tcslen(pList->m_word);
	cmp = Bview::KeywordCompare(lpWord, pList->m_word, nWord, cmp, pList->m_sensi);

	if(cmp < 0)
		return FindKeyword(lpWord, nWord, pList->m_left);
	else if(cmp > 0)
		return FindKeyword(lpWord, nWord, pList->m_right);
	else
		return pList;
}

//**************************************************************************
void Bview::AddKeyword(LPCTSTR lpWord, BkwType type, bool casesensi)
{
	LPKEYWORD pKey = new KEYWORD(lpWord, type, casesensi);

	pKey->AddKeyword(pKey, m_keywords);
}

//**************************************************************************
BkwType Bview::KeyWord(LPCTSTR lpText, int nText)
{
	BprefixList* pPref;
	LPKEYWORD	 pKey;

	if(! lpText || ! nText)
		return kwPlain;

	for(pPref = m_prefixes; pPref; pPref = pPref->GetNext(pPref))
	{
		if(pPref->GetName()[0] == lpText[0])
			return *pPref->GetValue();
	}
	if(m_keywords)
		if((pKey = m_keywords->FindKeyword(lpText, nText, m_keywords)) != NULL)
			return pKey->m_type;
	if(GetEditor() && GetEditor()->GetProject() && GetEditor()->GetProject()->GetPDB())
	{
		Bpdb*	pdb = GetEditor()->GetProject()->GetPDB();
		LPBSYM	psym;
		TCHAR	szText[128];

		if(nText > 127) nText = 127;
		_tcsncpy(szText, lpText, nText);
		szText[nText] = _T('\0');

		psym = pdb->FindSym(szText, pdb->GetTypes());
		if(psym)	return kwAddonType;
#if 0
		psym = pdb->FindSym(szText, pdb->GetFunctions());
		if(psym)	return kwAddonFunc;

		psym = pdb->FindSym(szText, pdb->GetVars());
		if(psym)	return kwSpecial;
#endif
	}
	return kwPlain;
}

//**************************************************************************
bool Bview::NonWhite(LPCTSTR lpToken, int nToken)
{
	int i;

	for(i = 0; i < nToken; i++)
		if(lpToken[i] != _T(' ') && lpToken[i] != _T('\t'))
			return true;
	return false;
}

//**************************************************************************
ERRCODE Bview::GetMatchingPhrase(
										LPCTSTR	lpPhrase,
										LPTSTR	lpMatch,
										int		nMatch
									)
{
	TCHAR	ob;

	if(! lpPhrase || ! lpMatch || nMatch <= 1)
		return errBAD_PARAMETER;

	if((IsDelim(lpPhrase[0]) && lpPhrase[1] == _T('\0')) || lpPhrase[0] == m_macro_prefix)
	{
		switch(lpPhrase[0])
		{
		case _T(')'):
			ob = _T('(');
			break;
		case _T('}'):
			ob = _T('{');
			break;
		case _T(']'):
			ob = _T('[');
			break;
		case _T('>'):
			ob = _T('<');
			break;
		case _T('#'):
			ob = _T('#');
			break;
		default:
			return errBAD_PARAMETER;
		}
		lpMatch[0] = ob;
		lpMatch[1] = _T('\0');
		return errOK;
	}
	else
	{
		lpMatch[0] = _T('\0');
		return errFAILURE;
	}
}

//**************************************************************************
ERRCODE Bview::FindMatchingPhrase(LPCTSTR pPhrase, LPCTSTR pMatch, int& line, int& col)
{
	ERRCODE ec;
	int     sline, scol;
	int		oline, ocol;
	int		mline, mcol;
	int		nPhrase, nMatch;
	int		nest = 1;

	sline = line;
	scol  = col;

	nPhrase = _tcslen(pPhrase);
	nMatch  = _tcslen(pMatch);

	do
	{
		// look back for first of original phrase or match.  Each occurance
		// of original phrase adds one to how many matches we need to find
		// for proper nesting
		//
		mline = sline;
		mcol  = scol;

		ec = m_buffer->Locate(mline, mcol, pMatch, nMatch, true, true);

		// if no match text, no match
		if(ec != errOK) return ec;

		oline = sline;
		ocol  = scol;

		ec = m_buffer->Locate(oline, ocol, pPhrase, nPhrase, true, true);

		if(ec == errOK && (oline > mline || (oline == mline && ocol > mcol)))
		{
			// phrase occurs before match, up nest count
			nest++;
			sline = oline;
			scol  = ocol;
		}
		else
		{
			// match occurs before phrase, down nest count
			nest--;
			sline = mline;
			scol  = mcol;
		}
		if(scol <= 1)
		{
			sline--;
			scol = MAX_LINE_LEN;
		}
		else
		{
			scol--;
		}
	}
	while(sline >= 1 && nest > 0);

	line = mline;
	col  = mcol;
	return nest > 0 ? errFAILURE : errOK;
}

//**************************************************************************
ERRCODE Bview::GetFirstNonBlankColumn(
										int		line,
										int&	col,
										bool	incComments,
										bool	incMacros,
										TCHAR&	lastdelim,
										TCHAR&	firstdelim,
										BkwType& firstdelimkw,
										int&	firstdelimcol,
										int&	parens,
										int&	braces,
										int&	colons
									)
{
	TokenRet	tr;
	ERRCODE		ec;
	LPCTSTR 	lpText;
	int			nText;
	LPTSTR		lpToken;
	int			nToken;
	TokenState 	state;
	int			incol, outcol, tokcol;
	BkwType		kw;
	BlineInfo	info;

	ec = m_buffer->GetLineText(line, lpText, nText);
	if(ec != errOK) return ec;

	incol	= 1;
	outcol	= 1;
	col 	= 0;
	parens  = 0;
	braces  = 0;
	colons	= 0;

	lastdelim		= firstdelim = _T('\0');
	firstdelimcol	= 0;

	info = m_buffer->GetLineCommentInfo(line);

	if(info == liInSpanning || info == liEndsSpanning)
		state = tsSpanningComment;
	else
		state = tsBase;

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
			if(! incComments && state == tsComment)
				break;
			if(! incMacros && state == tsMacro)
				break;

			if(
					(incComments || (state != tsSpanningComment && kw != kwComment))	&&
					(incMacros	 || (state != tsMacro))
			)
			{
				if(NonWhite(lpToken, nToken))
				{
					if(kw == kwPlain)
						kw = KeyWord(lpToken, nToken);
					if(kw == kwMacro)
						state = tsMacro;

					if(kw == kwOperator || incMacros || state != tsMacro)
					{
						if(! col) col = tokcol;

						if(firstdelimcol <= 0)
						{
							firstdelimkw  = kw;
							firstdelimcol = tokcol;
							firstdelim    = lpToken[0];
						}
						switch(lpToken[0])
						{
						case _T('('): parens++; break;
						case _T(')'): parens--; break;
						case _T('{'): braces++; break;
						case _T('}'): braces--; break;
						case _T(':'): colons++; break;
						}
						lastdelim = lpToken[0];
					}
				}
			}
		}
	}
	while(tr == trOK);

	return errOK;
}

//**************************************************************************
ERRCODE Bview::GetIsLabel(
										int		line,
										bool	&label
									)
{
	TokenRet	tr;
	ERRCODE		ec;
	LPCTSTR 	lpText;
	int			nText;
	LPTSTR		lpToken;
	int			nToken;
	TokenState 	state;
	int			incol, outcol, tokcol;
	int			lablen, labcol;
	BkwType		kw;
	BlineInfo	info;

	label = false;

	ec = m_buffer->GetLineText(line, lpText, nText);
	if(ec != errOK) return ec;

	incol	= 1;
	outcol	= 1;

	lablen = 0;
	labcol = 0;

	info = m_buffer->GetLineCommentInfo(line);

	if(info == liInSpanning || info == liEndsSpanning)
		state = tsSpanningComment;
	else
		state = tsBase;

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
			if(state == tsMacro)
				break;

			if(
					(state != tsSpanningComment && kw != kwComment)	&&
					(state != tsMacro)
			)
			{
				if(NonWhite(lpToken, nToken))
				{
					if(kw == kwPlain)
						kw = KeyWord(lpToken, nToken);
					if(kw == kwMacro)
						return errOK;
					if (kw == kwPlain)
					{
						if(lablen || labcol)
						{
							return errOK;
						}
						lablen = nToken;
						labcol = incol;
					}
					if(kw == kwOperator)
					{
						switch(lpToken[0])
						{
						case _T(':'):
							if (incol == (lablen + labcol))
							{
								label = true;
							}
							break;
						default:
							break;
						}
						return errOK;
					}
				}
			}
		}
	}
	while(tr == trOK);

	return errOK;
}

//**************************************************************************
ERRCODE Bview::GetStatement(
										int			line,
										int&		col,
										bool		last,
										BkwType&	kw,
										LPTSTR		pStatement,
										int&		nStatement
								)
{
	TokenRet	tr;
	ERRCODE		ec;
	LPCTSTR 	lpText;
	int			nText;
	LPTSTR		lpToken;
	int			nToken;
	TokenState 	state;
	int			incol, outcol, tokcol;
	BlineInfo	info;

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

			if(state != tsSpanningComment && kw != kwComment)
			{
				if(NonWhite(lpToken, nToken))
				{
					if(kw == kwPlain)
						kw = KeyWord(lpToken, nToken);
					if(kw == kwBuiltin || kw == kwBuiltinType)
					{
						col = tokcol;
						if(pStatement && nStatement > 0)
						{
							nStatement = min(nStatement, (nToken + 1));
							_tcsncpy(pStatement, lpToken, nStatement - 1);
							pStatement[nStatement - 1] = _T('\0');
						}
						if(! last)
							return errOK;
					}
				}
			}
		}
	}
	while(tr == trOK);

	return (col > 0) ? errOK : errFAILURE;
}


//**************************************************************************
ERRCODE Bview::Indent(WORD key, int& line, int& col)
{
	ERRCODE 	ec;
	int  		sl, sc;
	bool 		nonblank, prevnb;
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

		// look back in buffer for last non-blank non-comment
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
			if(prevfirstcol > 0)
			{
				// non blank line
				if(prevlastdelim == m_statement_term)
				{
					// saw a line ending with statement term so accumulate
					sawEOS = m_statement_term;
				}
				if(prevparens >= 0 || prevfirstdelim == ')')
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
		prevlastdelim = sawEOS;

		// as a bonus, look at previous lines to first prev nblank one, and
		// see if it is an "if", "else" or "for", etc. without a brace
		// in which case adjust indent back one
		//
		protoline = line;
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

	// assume indent exactly like prototype line
	//
	netdent = 0;

	// adjust target lines indent target
	//
	protocol += (netdent * m_tabspace);

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
			incol += m_tabspace - (incol % m_indentspace);
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


//**************************************************************************
ERRCODE Bview::SetupCommentInfo(int firstline, int lastline)
{
	ERRCODE 	ec;
	TokenRet	tr;
	LPCTSTR 	lpText;
	int			nText;
	LPTSTR		lpToken;
	int			nToken;
	TokenState 	state, initstate;
	int			incol, outcol, tokcol;
	int			line;
	BkwType		kw;
	BlineInfo	info;

	if(firstline < 1)
		firstline = 1;

	line = firstline;

	if(line == 1)
		info = liNone;
	else
		info = m_buffer->GetLineCommentInfo(line - 1);

	if(info == liUnknown)
	{
		return SetupCommentInfo(1);
	}
	if(info == liNone && line > 1)
	{
		firstline = line;
		for(line--; line > 1; line--)
		{
			info = m_buffer->GetLineCommentInfo(line);
			if(info != liNone && info != liUnknown)
				break;
		}
		line = firstline;
	}
	if(m_buffer->GetRaw())
	{
		m_buffer->SetLineCommentInfo(1, liNone);
		return errOK;
	}
	if(info == liStartsSpanning || info == liInSpanning)
		state = tsSpanningComment;
	else
		state = tsBase;

	do
	{
		ec = m_buffer->GetLineText(line, lpText, nText);
		if(ec != errOK) break;

		incol	 = 1;
		outcol	 = 1;

		initstate = state;

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
		}
		while(tr == trOK);

		if(state == tsSpanningComment)
		{
			volatile int a;
			a = line;
		}
		if(initstate != tsSpanningComment && state == tsSpanningComment)
			info = liStartsSpanning;
		else if(initstate == tsSpanningComment && state != tsSpanningComment)
			info = liEndsSpanning;
		else if(state == tsSpanningComment)
			info = liInSpanning;
		else
			info = liNone;
		//_tprintf(_T("line %d ->%d\n"), line, info);
		m_buffer->SetLineCommentInfo(line, info);
		line++;
	}
	while(lastline <= 0 || line < lastline);

	return errOK;
}

//**************************************************************************
ERRCODE Bview::Match(LPCTSTR pKey)
{
	TCHAR   key[256];
	ERRCODE ec;

	int sl, sc;
	int bl, bc;

	sl = m_curline;
	sc = m_curcol;
	bc = ((m_curcol > 2) ? m_curcol - 2 : MAX_LINE_LEN);
	bl = ((m_curcol > 2) ? m_curline : m_curline - 1);

	ec = GetMatchingPhrase(pKey, key, 256);
	if(ec != errOK) return ec;

	if((ec = FindMatchingPhrase(pKey, key, bl, bc)) == errOK)
	{
		int		mlb = m_curline, mcb = m_curcol;

		m_bmla = bl;
		m_bmca = bc;

		if(FORCE_LINE_MATCH || bl != m_curline)
		{
			// this causes lines to be drawm
			m_bmlb = m_curline;
			m_bmcb = m_curcol;

			// locate the nearest key back from current location
			//
			ec = m_buffer->Locate(
									mlb,
									mcb,
									pKey,
									1,
									false,
									true
								 );
			if(ec == errOK)
			{
				m_bmlb = mlb;
				m_bmcb = mcb;
			}
		}
		UpdateView(m_bmla, 1, ((m_bmlb > 0) ? ((m_bmlb == m_bmla) ? m_bmlb + 1 : m_bmlb) : m_bmla), MAX_LINE_LEN);
	}
	MoveAbs(sl, sc);
	return errOK;
}

