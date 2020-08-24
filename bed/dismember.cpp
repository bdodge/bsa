
#include "bedx.h"

//**************************************************************************
ERRCODE Bview::GetVarname(
										int			line,
										int&		col,
										LPTSTR		pVarname,
										int&		nVarname,
										TCHAR&		delim,
										TCHAR&		leftdelim,
										int&		leftdelcol,
										bool&		isFunc
								)
{
	ERRCODE		ec;
	LPCTSTR 	lpText;
	TCHAR       c;
	int			nText, scol, i;
	int			brace, paren;
	bool		nonwhite, sawspace;

	ec = m_buffer->GetLineText(line, lpText, nText);
	if(ec != errOK) return ec;

	isFunc = false;
	leftdelim = _T('\0');
	leftdelcol = 0;
	pVarname[0] = _T('\0');

	// get the token at the current column col
	//
	// if the text exactly under col is ( or . or > then
	// assume the token is some place off to the left
	//
	// otherwise, assume the token is around col, delimited
	// on the left and right by some sort of delimiter.
	//
	// for the purpose of delimiting the token white space
	// is ignored (mostly)
	//
	// this function is used to handle getting tokens in
	// text like
	//
	//    a = b ( c, d ) -> e
	//
	// where the column could be anywhere.  if the column
	// STARTS on white-space, assume there is no token
	// otherwise, parse out the token, ignoring parenthetical
	// expressions to the left

	// first look for a ( or a > or a . at or left of the current column
	// for the case this is called to dismember the leftward token
	//
	col--; // convert from 1 based index to 0 based
	scol = col;
	nonwhite = false;
	delim = _T('\0');
	leftdelim = _T('\0');

	if (col < 0 || col >= nText)
		return errFAILURE;
	if (lpText[col] == _T(' ') || lpText[col] == _T('\t'))
		return errFAILURE;
	if (lpText[col] == _T('\n') || lpText[col] == _T('\r'))
		return errFAILURE;

	// look to the right for . or ( or -> or ;
	// ignoring white space
	//
	while(col < nText)
	{
		if(lpText[col] == _T('\'') || lpText[col] == _T('\"'))
			return errFAILURE;
		if((delim = lpText[col]) == _T('.'))
			break;
		if((delim = lpText[col]) == _T('('))
			break;
		if((delim = lpText[col]) == _T(';'))
			break;
		if((delim = lpText[col]) == _T('>'))
		{
			if(col > 1 && lpText[col - 1] == _T('-'))
			{
				if (scol == col)
					scol-= 2;
				break;
			}
			else
			{
				delim = _T('\0');
				break; // > but not ->, so end looking
			}
		}
		if((delim = lpText[col]) == _T('-'))
		{
			if(col < (nText - 1) && lpText[col + 1] == _T('>'))
			{
				if (scol == col)
					scol--;
				col--;
				delim = '>';
				break;
			}
			else
			{
				delim = _T('\0');
				break; // - but not ->, so end looking
			}
		}
		if (lpText[col] != _T(' ') && lpText[col] != _T('\t'))
		{
			// any other delimiter stops this except white space
			if(IsDelim(lpText[col]))
				break;
		}
		col++;
	}
	if(delim == _T('\0'))
	{
		// nothing interesting to the right
		return errFAILURE;
	}
	if (scol == col)
	{
		// started right on delim, so go one left
		scol--;
	}
	// now got back and look to the left for the first delimiter
	// to the left, ignoring white space and braces/parentheses
	//
	col = scol;
	brace = 0;
	paren = 0;

	nonwhite = false;
	sawspace = false;

	while(col >= 0 && leftdelcol == 0)
	{
		c = lpText[col];
		if (paren > 0)
		{
			if (c == ')')
			{
				paren++;
			}
			else if (c == '(')
			{
				paren--;
			}
		}
		else if (brace > 0)
		{
			if (c == ']')
			{
				brace++;
			}
			else if (c == '[')
			{
				brace--;
			}
		}
		else
		{
			switch (c)
			{
			case _T(' '): case _T('\t'):
				if (nonwhite)
					sawspace = true;
				break;

			case _T(')'):
				paren++;
				break;

			default:
				if(IsDelim(c))
				{
					leftdelim = c;
					leftdelcol = col + 1;
				}
				else
				{
					if(! sawspace)
					{
						nonwhite = true;
					}
					else
					{
						// regular text after a space after regular text
						// so stop here
						leftdelim = _T(' ');
						leftdelcol = col + 2;
					}
				}
				break;
			}
		}
		if (leftdelcol == 0)
		{
			col--;
		}
	}

	// if we didn't find a left delimiter, and had no non-white token chars
	// there isn't anything there
	//
	if(leftdelcol == 0)
	{
		if(nonwhite == false)
			return errFAILURE;
		leftdelcol = 1;
	}

	// skip any white space until token starts
	///
	while (++col < scol)
	{
		if (lpText[col] != _T(' ') && lpText[col] != _T('\t'))
			break;
	}

	// capture token up to, but not including right delim
	// and at same time exclude () and [] expression prior to
	// -> or .
	//
	for(i = 0; col <= nText && i < nVarname-1; i++, col++)
	{
		if(
				lpText[col] == delim	||
				lpText[col] == _T('(')	||
				lpText[col] == _T('[')	||
				lpText[col] == _T(' ')	||
				lpText[col] == _T('\t')	||
				lpText[col] == _T('-')
		)
		{
			if(lpText[col] == _T('('))
				isFunc = true;
			break;
		}
		pVarname[i] = lpText[col];
	}
	nVarname    = i;
	pVarname[i] = _T('\0');

	if(col >= nText)
	{
		// went off end of token (and line) without a delimiter
		// so can't set var delim type yet
		//
		return errFAILURE;
	}
	return (i > 0) ? errOK : errFAILURE;
}

//**************************************************************************
ERRCODE Bview::GetFirstStatementToken(
										int&		line,
										int&		col,
										LPCTSTR		pVarname,
										LPTSTR		pToken,
										int&		nToken
								)
{
	TokenRet	tr;
	ERRCODE		ec;
	TCHAR		statementTerm[2];
	LPCTSTR 	lpText;
	int			nText;
	LPTSTR		lpToken;
	int			nReadToken;
	TokenState 	state;
	int			incol, outcol, tokcol;
	int			startline, startcol;
	int			ltline, ltcol, lsline, lscol, lpline, lpcol;
	bool		arglist;
	BlineInfo 	info;
	BkwType		kw;

	statementTerm[0] = m_statement_term;
	statementTerm[1] = _T('\0');

	startline = ltline = lsline = lpline = line;
	startcol  = ltcol  = lscol  = lpcol  = col;

	arglist = false;

	// look back for end of previous statement or start of scope
	// or start of argument list, (or start of file)
	//
	ec = m_buffer->Locate(lsline, lscol, _T("{"), 1, true, true);
	if(ec != errOK)
	{
		lsline = 1;
		lscol = -1;
	}
	ec = m_buffer->Locate(ltline, ltcol, statementTerm, 1, true, true);
	if(ec != errOK)
	{
		ltline = 1;
		ltcol = -1;
	}
	if(lsline > ltline)
	{
		line = lsline;
		col  = lscol;
	}
	else if(ltline > lsline)
	{
		line = ltline;
		col  = ltcol;
	}
	else
	{
		line = ltline;
		if(lscol > ltcol)
		{
			col = lscol;
		}
		else
		{
			col = ltcol;
		}
	}
	lsline = line;
	lscol  = col;

	ec = m_buffer->Locate(lpline, lpcol, _T("("), 1, true, true);
	if(ec != errOK)
	{
		lpline = 1;
		lpcol = -1;
	}

	if(lsline > lpline)
	{
		line = lsline;
		col  = lscol;
	}
	else if(lpline > lsline)
	{
		arglist = true;
		line = lpline;
		col  = lpcol;
	}
	else
	{
		line = lpline;
		if(lscol > lpcol)
		{
			col = lscol;
		}
		else
		{
			arglist = true;
			col = lpcol;
		}
	}

	// if var found in an argument list, look back for later of
	// parenthesis and comma and start from there, else start
	// from the found statement end or scope start
	//
	if(arglist)
	{
		ltline = startline;
		ltcol  = startcol;
		ec = m_buffer->Locate(ltline, ltcol, _T(","), 1, true, true);
		if(ec != errOK)
		{
			ltline = 1;
			ltcol = -1;
		}
		if(ltline > line)
		{
			line = ltline;
			col  = ltcol;
		}
		else if(ltline == line)
		{
			if(ltcol > col)
			{
				col = ltcol;
			}
		}
	}
	while(line <= startline)
	{
		// skip tokens on this line before statement terminator
		//
		ec = m_buffer->GetLineText(line, lpText, nText);
		if(ec != errOK) return ec;

		info = m_buffer->GetLineCommentInfo(line);

		incol	 = 1;
		outcol	 = 1;

		if(info == liInSpanning || info == liEndsSpanning)
			state = tsSpanningComment;
		else
			state = tsBase;

		do
		{
			tokcol = incol;

			tr = GetToken(
						(LPTSTR&)lpText,
						nText,
						line,
						incol,
						outcol,
						state,
						lpToken,
						nReadToken,
						kw
					);

			if(tr == trOK && nReadToken > 0 && state != tsComment)
			{
				if(kw == kwPlain)
					kw = KeyWord(lpToken, nReadToken);
				if(kw == kwMacro)
					break;
				if(
						(incol > col)
					&&	(kw == kwPlain || kw == kwBuiltinType || kw == kwAddonType)
					&&	NonWhite(lpToken, nReadToken)
				)
				{
					if(pToken && nToken > 0)
					{
						if(nToken < nReadToken)
							return errOVERFLOW;
						_tcsncpy(pToken, lpToken, nReadToken);
						pToken[nReadToken] = _T('\0');
						nToken = nReadToken;
						col = tokcol;
					}
					return errOK;
				}
			}
		}
		while(tr == trOK);

		// got to end of line without a real token, move to next
		// line unless already at starting line
		//
		line++;
		col = -1;
	}
	return errFAILURE;
}



//**************************************************************************
ERRCODE Bview::GetTypeOf(
										LPCTSTR		pVarname,
										int&		line,
										int&		col,
										LPTSTR		pTypename,
										int&		nTypename,
										bool&		isPtr,
										Bpdb*		pPDB
								)
{
	ERRCODE		ec = errOK;
	int			nVar, nType;

	int			initLine, initCol;
	int			scopeLine, scopeCol;
	int			nextScopeLine, nextScopeCol;
	int			declLine, declCol;
	int			prevDeclLine, prevDeclCol;

	// search for an instance of "pVarname" that is followed by a
	// comma, an equal, or a semicolon.
	//
	// search begins after the start of innermost scope, then expands
	// each scope to function scope, then to argument list, and finally to filescope
	//
	initLine  = line;
	initCol   = col;

	scopeLine = line;
	scopeCol  = col;

	isPtr     = false;
	pTypename[0] = 0;

	prevDeclLine = m_buffer->GetLineCount(prevDeclCol);

	nVar = _tcslen(pVarname);

	// look back to start of local scope, or filescope
	//
	while(scopeLine >= 1 && ec == errOK)
	{
		nextScopeLine = scopeLine;
		nextScopeCol  = scopeCol;

		ec = m_buffer->Locate(nextScopeLine, nextScopeCol, _T("{"), 1, true, true);

		if(ec != errOK)
		{
			// cant find any scope start, move to file start (if not there yet)
			//
			if(scopeLine > 1 || scopeCol > 1)
			{
				scopeLine = 1;
				scopeCol  = 1;
				ec = errOK;
			}
		}
		else
		{
			scopeLine = nextScopeLine;
			scopeCol  = nextScopeCol;
		}
		if(ec == errOK)
		{
			// from top of scope, forward find pVarname at least until initline
			//
			declLine = scopeLine;
			declCol  = scopeCol;

			ec = m_buffer->Locate(declLine, declCol, pVarname, nVar, true, false);

			if(ec != errOK)
			{
				// didn't find var after this scope, so, as a little hack, look
				// back for the var to see if it is in a function arg list just above
				//
				declLine = scopeLine;
				declCol  = scopeCol;

				ec = m_buffer->Locate(declLine, declCol, pVarname, nVar, true, true);
				if(ec == errOK)
				{
					// found var above this scope start, make sure it is at least
					// not in another scope (no '}' between here and found '{'
					//
					int fscopeLine = declLine;
					int fscopeCol  = declCol;

					ec = m_buffer->Locate(fscopeLine, fscopeCol, pVarname, nVar, true, false);

					if(ec == errOK)
					{
						if(fscopeLine < scopeLine || (fscopeLine == scopeLine && fscopeCol < scopeCol))
						{
							// scope intervenes
							//
							ec = errFAILURE;
						}
					}
				}
			}
			while(ec == errOK)
			{
				LPCTSTR lineText;
				int     nLine, xCol;
				bool	skipDecl = false;

				// if located the text at, or past a location we've already been too
				// just get out
				//
				if (declLine > prevDeclLine)
					return errFAILURE;

				if (declLine == prevDeclLine && declCol >= prevDeclCol)
					return errFAILURE;

				prevDeclLine = declLine;
				prevDeclCol = declCol;

				// make sure the token found is an exact match for var name
				// (not just located sub-text inside a larger string)
				//
				ec = m_buffer->GetLineText(declLine, lineText, nLine);

				if(ec == errOK)
				{
					if(declCol > 1)
					{
						if(! IsDelim(lineText[declCol-2]))
						{
							// located subtext
							skipDecl = true;
						}
						else
						{
							// the token to the LEFT of the var name needs to be either a
							// comma or a typename for this to be a declaration
							//
							for(xCol = declCol-2; ! skipDecl && xCol >= 0; xCol--)
							{
								if(IsDelim(lineText[xCol]))
								{
									if(NonWhite(lineText + xCol, 1))
									{
										if(lineText[xCol] == '*')
										{
											isPtr = true;
										}
										else
										{
											if(lineText[xCol] != ',')
											{
												// this is NOT a declaration
												//
												skipDecl = true;
											}
										}
										break;
									}
									else
									{
										while (xCol > 0
											&& (lineText[xCol] == _T(' ') || lineText[xCol] == _T('\t')))
										{
											xCol--;
										}
										if(lineText[xCol] == '*')
										{
											isPtr = true;
										}
									}
								}
								else
								{
									if(NonWhite(lineText + xCol, 1))
									{
										// found plain text to left of var name,
										// which is OK enough to confirm this is
										// a declaration (for now, perhaps care about comments?)
										//
										break;
									}
								}
							}
						}
					}
					if(ec == errOK && ! skipDecl && ((declCol + nVar) <= nLine))
					{
						if(! IsDelim(lineText[declCol + nVar - 1]))
						{
							// located subtext
							skipDecl = true;
						}
					}
				}
				if(ec == errOK && ! skipDecl)
				{
					if (declLine > initLine)
						return errFAILURE;

					if (declLine == initLine && declCol >= initCol)
						return errFAILURE;

					// this is a possible declaration of pVarname, see if the first
					// token in the statement this var is found in is a type
					//
					nType = nTypename;
					ec = GetFirstStatementToken(declLine, declCol, pVarname, pTypename, nType);

					if(ec == errOK)
					{
						// got a possible type name
						if(pPDB->GetBaseType(pTypename) != NULL)
						{
							/*
							_tprintf(_T("Var decl of "_Pfs_" at %d, type "_Pfs_"\n"),
									pVarname, declLine, pTypename);
							*/
							return ec;
						}
					}
				}
				if (ec == errOK)
				{
					// keep searching from current point + 1, but if got to initial point give up
					if(declCol >= nLine)
						declLine++;
					else
						declCol++;

					if (declLine > initLine)
						return errFAILURE;

					if (declLine == initLine && declCol >= initCol)
						return errFAILURE;

					// find next occurance (forward)
					ec = m_buffer->Locate(declLine, declCol, pVarname, nVar, true, false);
				}
			}
		}
		if(scopeCol > 1)
			scopeCol--;
		else
			scopeLine--;
	}
	return errFAILURE;
}

//**************************************************************************
ERRCODE Bview::GetContainingClass(
										int&		line,
										int&		col,
										LPTSTR		pClassname,
										int&		nClassname
								)
{
	ERRCODE		ec = errOK;

	int			initLine, initCol;
	int			scopeLine, scopeCol;
	int			nextScopeLine, nextScopeCol;
	int			declLine, declCol;
	int			prevDeclLine, prevDeclCol;

	// search for an instance of "Class::Func()" that is followed by a "{"
	//
	initLine  = line;
	initCol   = col;

	scopeLine = line;
	scopeCol  = col;

	prevDeclCol = MAX_LINE_LEN;
	m_buffer->GetLineCount(prevDeclLine);

	while(scopeLine >= 1 && ec == errOK)
	{
		nextScopeLine = scopeLine;
		nextScopeCol  = scopeCol;

		// look back for "::"
		//
		ec = m_buffer->Locate(nextScopeLine, nextScopeCol, _T("::"), 2, true, true);
		if(ec != errOK)
		{
			// cant find class function decl
			//
			return ec;
		}
		else
		{
			scopeLine = nextScopeLine;
			scopeCol  = nextScopeCol;
		}
		if(ec == errOK)
		{
			int		braceLine, braceCol;
			int		semiLine, semiCol;
			ERRCODE fndBrace, fndSemi;

			// from :: find first token of '{' or ';'
			//
			declLine = scopeLine;
			declCol  = scopeCol;

			braceLine = scopeLine;
			braceCol  = scopeCol;
			semiLine  = scopeLine;
			semiCol   = scopeCol;

			fndBrace = m_buffer->Locate(braceLine, braceCol, _T("{"), 1, true, false);
			fndSemi  = m_buffer->Locate(semiLine, semiCol, _T(";"), 1, true, false);

			if(fndBrace == errOK)
			{
				if(fndSemi == errOK)
				{
					if(braceLine > semiLine || (braceLine == semiLine && braceCol > semiCol))
					{
						// semi before brace, this is not member func definition start
						//
						fndBrace = errFAILURE;
					}
				}
			}
			if(fndBrace == errOK)
			{
				LPCTSTR		lpText;
				int			nText;
				LPTSTR		lpToken;
				int			nToken;
				LPTSTR		lpPrevToken;
				int			nPrevToken;
				TokenState 	state;
				int			incol, outcol, tokcol;
				BlineInfo 	info;
				TokenRet	tr;
				BkwType		kw;

				// get first non-null token before the "::"
				//
				ec = m_buffer->GetLineText(scopeLine, lpText, nText);
				if(ec != errOK) return ec;

				info = m_buffer->GetLineCommentInfo(line);

				incol	 = 1;
				outcol	 = 1;

				if(info == liInSpanning || info == liEndsSpanning)
					state = tsSpanningComment;
				else
					state = tsBase;

				lpPrevToken = NULL;
				nPrevToken  = 0;

				do
				{
					tokcol = incol;

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
						if(state != tsSpanningComment && kw != kwComment)
						{
							if(kw == kwPlain)
								kw = KeyWord(lpToken, nToken);
							if(incol < scopeCol && kw == kwPlain && NonWhite(lpToken, nToken))
							{
								// plain token that is before the '::' token.
								// remember its place
								//
								lpPrevToken = lpToken;
								nPrevToken = nToken;
							}
							else if(kw == kwOperator && lpToken[0] == _T(':'))
							{
								if(pClassname && nClassname > 0)
								{
									if(nClassname < nPrevToken)
										return errOVERFLOW;
									_tcsncpy(pClassname, lpPrevToken, nPrevToken);
									pClassname[nPrevToken] = _T('\0');
									nClassname = nPrevToken;
								}
								return errOK;
							}
						}
					}
				}
				while(tr == trOK);

				return errFAILURE;
			}
			else
			{
				// not a member func definition, look back further
				;
			}
		}
		if(scopeCol > 1)
			scopeCol--;
		else
			scopeLine--;
	}
	return errFAILURE;
}

//**************************************************************************
ERRCODE Bview::GetTypeOfTokenAtPoint(int& line, int& col, LPBSYM& pSym, Bpdb* pPDB, bool& isFunc)
{
	ERRCODE ec = errOK;
	TCHAR	msg[512];
	TCHAR	varName[256];
	TCHAR	typeName[256];
	TCHAR	className[256];
	int		nVar, nType, nClass;
	TCHAR	delim, leftdelim;
	int		leftdelcol;
	bool	isPtr;

	// get the variable name or function name to the left of
	// '(' or '.' or '->'
	//
	// assumes current point is somewhere inside the aaaa- part of
	// text like "aaaa->" or "aaaa." or aaaa(
	//
	nVar  = 256;
	ec = GetVarname(line, col, varName, nVar, delim, leftdelim, leftdelcol, isFunc);
	if(ec != errOK) return ec;			// cant get varname

	if(delim == _T('\0')) return errOK; // not a func, struct or ptr

	// if the stopping delimiter on the left was a > or . then this
	// var is a member of a containing class, so recursively get the
	// containing class
	//
	className[0] = _T('\0');
	if(leftdelim == _T('>') || leftdelim == _T('.'))
	{
		bool isParentFunc;

		// move the column back to the left delim, then recurse
		// to get the typename of that token to resolve the type
		//
		ec = GetTypeOfTokenAtPoint(line, leftdelcol, pSym, pPDB, isParentFunc);
		if(ec == errOK && pSym)
		{
			// found the containing class of me, so now find me in the member list
			// and that's that
			BpdbInfo* pMember;

			// found a class name encompassing this function reference
			// see if this function is a memba
			//
			pMember = BpdbInfo::FindKey(varName, pSym->f_members);

			if(pMember)
			{
				// yes, this is a member, get the type from the member item
				//
				pSym = pMember->GetValue();

				// go one more level of type depth if this is a mangled name
				//
				if(pSym && pSym->f_type)
				{
					pSym = pSym->f_type;
				}
			}
			else
			{
				pSym = NULL;
			}
			return pSym ? errOK : errFAILURE;
		}
	}
	if(delim == _T('('))
	{
		// function call, lookup arguments to function
		//
		nClass = 256;
		ec = GetContainingClass(line, col, className, nClass);
		if(ec != errOK)
		{
			// no class container or reference, maybe a global func
			//
			pSym = pPDB->FindSym(varName, pPDB->GetFunctions());
		}
		else
		{
			pSym = pPDB->FindSym(className, pPDB->GetTypes());
			if(pSym)
			{
				BpdbInfo* pMember;

				// found a class name encompassing this function reference
				// see if this function is a memba
				//
				pMember = BpdbInfo::FindKey(varName, pSym->f_members);

				if(pMember)
				{
					// yes, this is a member function, function args are found
					// in the function table under "Class::varName"
					//
					pSym = pMember->GetValue();
				}
				else
				{
					pSym = NULL;
				}
			}
		}
		if(! pSym)
		{
			// not in a container, look through class modified list of functions
			// and see if it matches and use (them/it) for the function
			//
			pSym = pPDB->FindClassSym(NULL, varName, pPDB->GetFunctions());
		}
		if(! pSym)
		{
			_sntprintf(msg, 512, _T("function "_Pfs_" not found"), varName);
			m_editor->SetStatus(msg);
			return errOK;
		}
	}
	else if(delim == _T('.') || delim == _T('>'))
	{
		// struct reference, lookup members of struct/class
		//
		// get the type of this variable if declared in this file
		//
		nType = 256;
		if(isFunc)
		{
			// function call being dereferenced, so lookup return type of function
			// and hope its a class we can enumerate
			//
			if((pSym = pPDB->FindSym(varName, pPDB->GetFunctions())) == NULL)
			{
				// not a global function, see if its a member func
				// of containing class or dereferenced class
				//
				if(ec != errOK || className[0] == _T('\0'))
				{
					nClass = 256;
					ec = GetContainingClass(line, col, className, nClass);
				}
				if(ec == errOK)
				{
					pSym = pPDB->FindSym(className, pPDB->GetTypes());

					if(pSym)
					{
						BpdbInfo* pMember;

						// found a class name encompassing this function reference
						// see if this function is a memba
						//
						pMember = BpdbInfo::FindKey(varName, pSym->f_members);

						if(pMember)
						{
							// yes, this is a member function, see what the return type is
							//
							pSym = pMember->GetValue(); // point to name mangled function
						}
						else
						{
							pSym = NULL;
						}
					}
				}
			}
			if(pSym && pSym->f_type)
			{
				pSym = pSym->f_type;
				_tcsncpy(typeName, pSym->f_name, nType - 1);
				typeName[nType - 1] = _T('\0');
			}
			else
			{
				pSym = NULL;
			}
			if(! pSym)
			{
				_sntprintf(msg, 512, _T("type for function "_Pfs_" not found"), varName);
				m_editor->SetStatus(msg);
				return errOK;
			}
		}
		else
		{
			// see if this variable is defined in the local scope or
			// as an argument to current function, or is a member of
			// a struct/class being dereferenced
			//
			ec = GetTypeOf(varName, line, col, typeName, nType, isPtr, pPDB);
			if(ec != errOK)
			{
				// no local spec on var, see if its a memba var
				//
				nClass = 256;
				ec = GetContainingClass(line, col, className, nClass);
				if(ec == errOK)
				{
					LPBSYM pSym;

					pSym = pPDB->FindSym(className, pPDB->GetTypes());

					if(pSym)
					{
						BpdbInfo* pMember;

						// found a class name encompassing this var reference
						// see if this varname is a memba var
						//
						pMember = BpdbInfo::FindKey(varName, pSym->f_members);

						if(pMember)
						{
							// this var is a member of the current class, see if
							// it is a compound type (its type has members)
							//
							pSym = pMember->GetValue();
							if(! pSym)
								ec = errFAILURE;
							else
								_tcscpy(typeName, pSym->f_name);
						}
					}
				}
				else
				{
					// see if the var was parsed in global scope
					//
					pSym = pPDB->FindSym(varName, pPDB->GetVars());
					if(pSym)
					{
						_tcscpy(typeName, pSym->f_type->f_name);
						ec = errOK;
					}
				}
			}
		}
		if(ec != errOK)
		{
			// see if the "var" is actually a type itself
			//
			if((pSym = pPDB->GetBaseType(varName)) == NULL)
			{
				// can't find type for this var, so indicate in status
				//
				_sntprintf(msg, 512, _T("No type for "_Pfs_" "_Pfs_""), varName, typeName);
				m_editor->SetStatus(msg);
				return errOK;
			}
			else
			{
				_tcscpy(typeName, varName);
			}
		}
		// lookup class/struct type in the program type database
		// and if not located, assume a typedef, and look at types
		// to see if there is a base class/struct
		//
		if((pSym = pPDB->GetBaseType(typeName)) == NULL)
		{
			_sntprintf(msg, 512, _T("Type "_Pfs_" not found"), typeName);
			m_editor->SetStatus(msg);
		}
	}
	return pSym ? errOK : errFAILURE;
}

//**************************************************************************
ERRCODE Bview::GetDefinitionOfTokenAtPoint(int& line, int& col, LPBSYM& pSym, Bpdb* pPDB, bool& isFunc)
{
	ERRCODE ec = errOK;
	LPBSYM  pClass;
	TCHAR	msg[512];
	TCHAR	varName[256];
	TCHAR	typeName[256];
	TCHAR	className[256];
	int		nVar, nType, nClass;
	TCHAR	delim, leftdelim;
	int		leftdelcol;
	bool	isPtr;

	// get the variable name or function name at col
	//
	nVar  = 256;
	ec = GetVarname(line, col, varName, nVar, delim, leftdelim, leftdelcol, isFunc);
	if(ec != errOK) return ec;			// cant get varname

	//_tprintf(_T("Looking for def of "_Pfs_"\n"), varName);
	{
		className[0] = _T('\0');
		nType = 256;
		if(isFunc)
		{
			// function call being dereferenced, so lookup return type of function
			// and hope its a class we can enumerate
			//
			if((pSym = pPDB->FindSym(varName, pPDB->GetFunctions())) == NULL)
			{
				// not a global function, see if its a member func
				// of containing class or dereferenced class
				//
				if(ec != errOK || className[0] == _T('\0'))
				{
					nClass = 256;
					ec = GetContainingClass(line, col, className, nClass);
				}
				if(ec == errOK)
				{
					pSym = pPDB->FindSym(className, pPDB->GetTypes());

					if(pSym)
					{
						BpdbInfo* pMember;

						// found a class name encompassing this function reference
						// see if this function is a memba
						//
						pMember = BpdbInfo::FindKey(varName, pSym->f_members);

						if(pMember)
						{
							// yes, this is a member function (TOOD - store line in member copy sym?)
							//
							pSym = pMember->GetValue(); // point to name mangled function
						}
						else
						{
							pSym = NULL;
						}
					}
				}
			}
			if(pSym)
			{
				line = pSym->f_line;
				return errOK;
			}
		}
		else
		{
			// see if this variable is defined in the local scope or
			// as an argument to current function, or is a member of
			// a struct/class being dereferenced
			//
			ec = GetTypeOf(varName, line, col, typeName, nType, isPtr, pPDB);
			if(ec != errOK)
			{
				// no local spec on var, see if its a memba var
				//
				nClass = 256;
				ec = GetContainingClass(line, col, className, nClass);
				if(ec == errOK)
				{
					LPBSYM pSym;

					pSym = pPDB->FindSym(className, pPDB->GetTypes());

					if(pSym)
					{
						BpdbInfo* pMember;

						// found a class name encompassing this var reference
						// see if this varname is a memba var
						//
						pMember = BpdbInfo::FindKey(varName, pSym->f_members);

						if(pMember)
						{
							// this var is a member of the current class, see if
							// it is a compound type (its type has members)
							//
							pSym = pMember->GetValue();

							// Todo, store line in member?
						}
						else
						{
							pSym = NULL;
						}
					}
				}
				else
				{
					// see if the var was parsed in global scope
					//
					pSym = pPDB->FindSym(varName, pPDB->GetVars());
				}
				if (pSym)
				{
					line = pSym->f_line;
					ec = errOK;
				}
			}
			else
			{
				if (! _tcscmp(varName, typeName))
				{
					ec = errFAILURE;
				}
			}
		}
		if(ec != errOK)
		{
			// see if the "var" is actually a type itself
			//
			pSym = pPDB->FindSym(varName, pPDB->GetTypes());
			if (! pSym)
			{
				// not a type, look at each class to see if its a member
				//
				pSym = pPDB->FindMember(varName, pPDB->GetTypes());
				if (! pSym)
				{
					return errFAILURE;
				}
			}
			line = pSym->f_line;
		}
	}
	return pSym ? errOK : errFAILURE;
}

//**************************************************************************
ERRCODE Bview::Dismember()
{
	ERRCODE ec = errOK;
	Bpdb*	pPDB;
	LPBSYM	pSym;
	bool	isFunc;
	int		line, col;

	if(m_editor->GetProject())
		pPDB = m_editor->GetProject()->GetPDB();
	else
		return errOK;

	if(! pPDB)
		return errOK;

	line = m_curline;
	col  = m_curcol - 1;
	pSym = NULL;

	ec = GetTypeOfTokenAtPoint(line, col, pSym, pPDB, isFunc);
	if(ec != errOK)
		return ec;

	if(pSym)
	{
		if(isFunc)
		{
			// enumerate the list of function parms
			//
			ec = ProtoWindow(pSym, efFunction);
		}
		else
		{
			// enumerate the list of members functions and parms
			//
			ec = ProtoWindow(pSym, efClass);
		}
	}
	return ec;
}

