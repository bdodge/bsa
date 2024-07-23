
#include "bedx.h"

BOOL CALLBACK AboutBed(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


//**************************************************************************
int Bview::CountRegionChars(Bbuffer* pBuf, int mla, int mca, int mlb, int mcb)
{
	ERRCODE ec;
	LPCTSTR lpText;
	int		nText;
	int		nTotal = 0;
	int		nDelete;
	int		line, col;
	int		temp;
	int		nCols;

	if(mla > mlb)
	{
		temp = mla;

		mla  = mlb;
		mlb  = temp;
		temp = mca;
		mca  = mcb;
		mcb  = temp;
	}
	else if(mla == mlb)
	{
		if(mca > mcb)
		{
			temp = mcb;
			mcb  = mca;
			mca  = temp;
		}
		else if(mca == mcb)
		{
			return 0;
		}
	}
	if(mca < 1)	mca = 1;
	if(mcb < 1)	mcb = 1;

	line = mla;
	col  = mca;
	temp = 0;

	do
	{
		ec = pBuf->GetLineText(line, lpText, nText);
		if(ec != errOK) break;

		nDelete = nText;

		if(line == mla)
		{
			if(mca > nText) nCols = nText;
			else			nCols = mca - 1;
			nDelete -= nCols;
		}
		if(line == mlb)
		{
			if(mcb > nText) nCols = nText;
			else			nCols = mcb - 1;
			nDelete -= (nText - nCols);
		}
		if(nDelete > 0)
			nTotal += nDelete;
		temp = line;
		line++;
	}
	while(line <= mlb);

	return nTotal;
}

//**************************************************************************
void Bview::UpdateStatus()
{
	if(m_buffer)
		m_editor->UpdateStatusInfo();
}

//**************************************************************************
ERRCODE Bview::CheckReadOnly()
{
	Bsccs*		pSC;
	Bproject*	pPr;
	ERRCODE		ec;
	int			rc;
	TCHAR		msg[MAX_PATH + 64];

	// if file is writeable ok
	//
	if(! m_buffer || ! m_buffer->GetReadOnly())
		return errOK;

	if(! m_editor)
		return errPERMISSION;

	// if there is an open project [why need a project??]
	// and sccs object, ask to
	// check out file before continuing
	//
	pSC = m_editor->GetSCCS();
	pPr = m_editor->GetProject();

	if(! pSC /*|| ! pPr this isnt right is it?*/)
		return errPERMISSION;

	if(pSC->GetRunning())
		return errPERMISSION;

	_sntprintf(msg, MAX_PATH + 64, _T("Check Out " _Pfs_ " ?"), m_buffer->GetShortName());
	rc = MessageBox(NULL, msg, m_editor->GetVersion(), MB_YESNOCANCEL);

	if(rc != IDYES)
	{

		return errPERMISSION;
	}
	ec = Dispatch(CheckOut);
	return ec;
}

//**************************************************************************
ERRCODE Bview::TabifyText(LPCTSTR pSrc, int nSrc, LPTSTR& pDst, int& nDst)
{
	LPTSTR pRes;
	int    i;
	int	   j;
	TCHAR  c;
	bool   reptab;

	pDst = new TCHAR [ nSrc + 4 ];
	nDst = nSrc;

	for(pRes = pDst, reptab = false, i = 0; i < nSrc;)
	{
		c = pSrc[i];

		if(c == _T(' ') && ! (i % m_tabspace))
		{
			for(j = i+1; j < nSrc; j++)
			{
				if(pSrc[j] != _T(' ') || ! (j % m_tabspace))
					break;
			}
			if(! (j % m_tabspace))
			{
				reptab = true;
				*pRes++ = _T('\t');
				i = j;
			}
			else
			{
				*pRes++ = c;
				i++;
			}
		}
		else
		{
			*pRes++ = c;
			i++;
		}
	}
	if(reptab)
	{
		nDst = pRes - pDst;
		*pRes = _T('\0');
	}
	else
	{
		delete [] pDst;
		nDst = 0;
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bview::UntabText(LPCTSTR pSrc, int nSrc, LPTSTR& pExp, int& nExp)
{
	LPCTSTR pCheck;
	int cnt, nTabs, scol;

	for(cnt = nSrc, pCheck = pSrc, nTabs = 0; cnt > 0; cnt--)
		if(*pCheck++ == _T('\t'))
			nTabs++;

	if(nTabs > 0)
	{
		int nAlloc = nSrc + nTabs * m_tabspace + m_tabspace;
		LPTSTR pDst = new TCHAR [ nAlloc ];

		pExp = pDst;
		nExp = 0;

		if(pExp)
		{
			for(cnt = nSrc, scol = m_curcol; cnt > 0; cnt--, pSrc++)
			{
				if(*pSrc == _T('\t'))
				{
					if(! ((scol - 1) % m_tabspace))
					{
						for(nTabs = 0; nTabs < m_tabspace; nTabs++)
							*pDst++ = _T(' ');
						scol += m_tabspace;
					}
					else while(((scol - 1) % m_tabspace) && (pDst - pExp) < nAlloc)
					{
						*pDst++ = _T(' ');
						scol++;
					}
				}
				else
				{
					if(*pSrc == _T('\n'))
						scol = 1;
					else
						scol++;
					*pDst++ = *pSrc;
				}
			}
			*pDst = _T('\0');
			nExp = pDst - pExp;
		}
	}
	else
	{
		nExp = 0;
		pExp = NULL;
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bview::WhiteTrimText(LPCTSTR pSrc, int nSrc, LPTSTR& pDst, int& nDst)
{
	LPTSTR pRes;
	int    i;
	bool   xspace;
	bool   hasnl;

	// look back from end of line for first non-blank
	for(i = nSrc - 1, xspace = false; i >= 0; i--)
	{
		if (pSrc[i] == _T('\n'))
		{
			hasnl = true;
		}
		else if(pSrc[i] != _T(' ') && pSrc[i] != _T('\t'))
		{
			break;
		}
		else
		{
			xspace = true;
		}
	}
	// alloc and copy only non-blank left of string
	if (xspace) {
		nDst = i + 1;
		pDst = new TCHAR [nDst + 3];

		_tcsncpy(pDst, pSrc, nDst);
		if (hasnl)
		{
			pDst[nDst++] = _T('\n');
		}
		pDst[nDst] = _T('\0');
	}
	else
	{
		nDst = 0;
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bview::CheckKillBlock()
{
	ERRCODE ec = errOK;
	int		mla, mca, mlb, mcb;

	if(GetMarkedRegion(mla, mca, mlb, mcb))
	{
		int sl = m_curline;
		int sc = m_curcol;

		ClearSearchHighlight();
		ec = Dispatch(DeleteBlock);
	}
	return ec;
}

//**************************************************************************
ERRCODE Bview::SetOpRegionFromSelection()
{
	ERRCODE ec = errOK;
	int		mla, mca, mlb, mcb;

	if(GetMarkedRegion(mla, mca, mlb, mcb))
	{
		m_orsvl = m_curline;
		m_orsvc = m_curcol;

		m_ola = mla;
		m_oca = mca;
		m_olb = mlb;
		m_ocb = mcb;
		m_regional = true;

		if(mla < mlb)
		{
			m_curline = mla;
			m_curcol  = mca;
		}
		else if(mla > mlb)
		{
			m_curline = mlb;
			m_curcol  = mcb;
		}
		else
		{
			m_curline = mla;
			if(mca <= mcb)
				m_curcol = mca;
			else
				m_curcol = mcb;
		}
	}
	else
	{
		m_ola = m_olb = -1;
		m_oca = m_ocb = -1;
	}
	return ec;
}

//**************************************************************************
ERRCODE Bview::ClearOpRegion()
{
	if(m_regional)
	{
		m_ola = m_olb = -1;
		m_oca = m_ocb = -1;
		m_regional = false;
		if(m_orsvl > 0 && m_orsvc > 0)
			MoveAbs(m_orsvl, m_orsvc);
		else if(m_qlb > 0 && m_qcb > 0)
			MoveAbs(m_qlb, m_qcb);
		m_orsvl = m_orsvc = -1;
	}
	ClearSearchHighlight();
	return errOK;
}

//**************************************************************************
ERRCODE Bview::ClearSearchHighlight()
{
	if(m_rla > 0)
	{
		int mla = min(m_rla, m_rlb);
		int mlb = max(m_rla, m_rlb);
		m_rla = m_rlb = m_rca = m_rcb = 0;
		UpdateView(mla, 1, mlb, MAX_LINE_LEN);
	}
	return errOK;
}

//**************************************************************************
bool Bview::InRegion(int line, int col)
{
	if(m_ola <= 0 || m_olb <= 0 || m_oca <= 0 || m_ocb <= 0)
		m_regional = false;

	if(! m_regional) return true;

	if(m_ola < m_olb)
	{
		if(line == m_ola)
			return col >= m_oca;
		else if(line == m_olb)
			return col < m_ocb;
		else
			return line > m_ola && line < m_olb;
	}
	else if(m_ola > m_olb)
	{
		if(line == m_olb)
			return col >= m_ocb;
		else if(line == m_ola)
			return col < m_oca;
		else
			return line > m_olb && line < m_ola;
	}
	else
	{
		if(line == m_ola)
		{
			if(m_oca <= m_ocb)
				return col >= m_oca && col < m_ocb;
			else
				return col < m_oca && col >= m_ocb;
		}
		else
			return false;
	}
}

//**************************************************************************
ERRCODE Bview::ClearMouseSelection()
{
	ERRCODE ec = errOK;
	int		mla, mca, mlb, mcb;

	if(! GetHandMarked() && GetMarkedRegion(mla, mca, mlb, mcb))
		ec = Dispatch(::ClearMark);
	m_locl = m_locc = 0;
	ClearSearchHighlight();
	//CheckCloseProtoWindow();
	m_ola = m_olb = m_oca = m_ocb = 0;
	return ec;
}

//**************************************************************************
void Bview::SetLastModifiedCol(int col)
{
	m_lastmodcol = m_lastcaretcol;
}

//**************************************************************************
int Bview::GetLastModifiedCol(void)
{
	int x, y, c;

	//return m_lastmodcol;
	c = DisplayColumn(NULL, m_curline, m_lastmodcol, false, x, y);
	if(c <= 0) return m_lastmodcol;
	return c;
}

//**************************************************************************
void Bview::ClearLineInfo(BlineInfo info)
{
	m_buffer->ClearLinesInfo(info);
}

#define STATUS_LEN 256

//**************************************************************************
ERRCODE Bview::Dispatch(EditCommand command)
{
	ERRCODE     ec = errOK;
	LPCTSTR		pParm;
	int			nParm;
	LPCTSTR		lpText;
	int			nText;
	BbufList* 	pEntry;
	Bbuffer*	pNext;
	Bview*		pView;

	bool		multiLine	= false;
	bool		vertical	= false;
	bool		needvu		= GetHandMarked();
	bool		modified;
	int			prevline	= m_curline;
	int			prevcol		= m_curcol;
	int			svline		= m_curline;
	int			svcol		= m_curcol;
	int			mla, mca, mlb, mcb;

	TCHAR		status[MAX_PATH + 64];

	if(! m_buffer) return errFAILURE;

	modified = m_buffer->GetModified();

	if(
			command == MoveUp 			||
			command == MoveDown			||
			command == MovePageUp		||
			command == MovePageDown		||
			command == MoveToTOB		||
			command == MoveToEOB		||
			command == MoveToLine
	)
	{
		vertical = true;
		CheckCloseProtoWindow();
	}

	if(m_bmla || m_bmlb)
	{
		if(m_bmla)
			prevline = min(m_curline, m_bmla);
		if(m_bmlb)
			prevline = min(prevline, m_bmlb);

		// clear brace match unless its a vertical
		// move and there is a multiline brace showing
		//
		if(m_bmlb <= 0 || ! vertical)
		{
			m_bmla = 0;
			m_bmlb = 0;
			needvu	  = true;
			multiLine = true;
		}
	}
	switch(command)
	{
	/*
	 * Cursor Movement
	 */
	case MoveToCol:

		nParm = 0;
		if(PopParm(_T("Column"), ptNumber, nParm) != errOK)
			break;
		ClearMouseSelection();
		MoveAbs(m_curline, nParm);
		SetLastModifiedCol(m_curcol);
		break;

	case MoveToLine:

		nParm = 0;
		if(PopParm(_T("Line"), ptNumber, nParm) != errOK)
			break;
		ClearMouseSelection();
		MoveAbs(nParm, GetLastModifiedCol());
		break;

	case MoveLeft:

		m_curcol--;
		ClearMouseSelection();
		MoveAbs(m_curline, m_curcol);
		SetLastModifiedCol(m_curcol);
		break;

	case MoveRight:

		m_curcol++;
		ClearMouseSelection();
		MoveAbs(m_curline, m_curcol);
		SetLastModifiedCol(m_curcol);
		break;

	case MoveDown:

		m_curline++;
		if(m_qla <= 0 && m_bmla <= 0)
			prevline = m_curline;
		ClearMouseSelection();
		MoveAbs(m_curline, GetLastModifiedCol());
		break;

	case MoveUp:

		m_curline--;
		if(m_qla <= 0 && m_bmla <= 0)
			prevline = m_curline;
		ClearMouseSelection();
		MoveAbs(m_curline, GetLastModifiedCol());
		break;

	case MoveWordLeft:
	case MoveWordRight:

		break;

	case MoveToBOL:

		m_curcol = 1;
		ClearMouseSelection();
		MoveAbs(m_curline, m_curcol);
		SetLastModifiedCol(m_curcol);
		break;

	case MoveToEOL:

		m_curcol = 0x7fffffff;
		ClearMouseSelection();
		MoveAbs(m_curline, m_curcol);
		SetLastModifiedCol(m_curcol);
		break;

	case MoveToTOB:

		m_curline = 1;
		m_curcol = 1;
		ClearMouseSelection();
		MoveAbs(m_curline, GetLastModifiedCol());
		break;

	case MoveToEOB:

		m_curline = 0x7fffffff;
		m_curcol  = 1;
		ClearMouseSelection();
		MoveAbs(m_curline, GetLastModifiedCol());
		break;

	case MovePageLeft:

		m_curcol -=  GetViewCols();
		ClearMouseSelection();
		MoveAbs(m_curline, m_curcol);
		SetLastModifiedCol(m_curcol);
		break;

	case MovePageRight:
		m_curcol += GetViewCols();
		ClearMouseSelection();
		MoveAbs(m_curline , m_curcol);
		SetLastModifiedCol(m_curcol);
		break;

	case MovePageUp:

		m_curline -= GetViewLines();
		ClearMouseSelection();
		MoveAbs(m_curline, GetLastModifiedCol());
		break;

	case MovePageDown:

		m_curline += GetViewLines();
		ClearMouseSelection();
		MoveAbs(m_curline, GetLastModifiedCol());
		break;

	/*
	 * Locating
	 */
	case FindForward:

		ec = PushParm(0, ptNumber);
		ec = Dispatch(SetSearchDirection);
		m_searchlen = 0;
		m_locl = m_curline;
		m_locc = m_curcol;
		return Dispatch(FindAgain);

	case FindReverse:

		ec = PushParm(1, ptNumber);
		ec = Dispatch(SetSearchDirection);
		m_searchlen = 0;
		m_locl = m_curline;
		m_locc = m_curcol;
		return Dispatch(FindAgain);

	case FindNonZero:

		break;

	case FindAgain:

		if(m_searchlen <= 0)
		{
			ec = Dispatch(SetSearch);
			if(ec != errOK) break;
		}
		if(m_locl > 0 && m_locc > 0)
		{
			// move one position forward from last found position
			//
			ec = m_buffer->GetLineText(m_locl, lpText, nText);
			if(ec != errOK) break;
			if(nText <= 0) break;

			if(m_locc >= nText)
			{
				mla = m_locl + 1;
				mca = 1;
			}
			else
			{
				mla = m_locl;
				mca = m_locc + 1;
			}
		}
		else
		{
			mla = m_curline;
			mca = m_curcol;
		}
		m_locl = m_locc = 0;
		m_rla = m_rlb = 0;
		m_rca = m_rcb = 0;

		ec = m_buffer->Locate(
								mla,
								mca,
								m_searchstr,
								m_searchlen,
								m_casesensi,
								m_searchrev
							  );
		if(ec == errOK)
		{
			int i;

			m_locl = mla;
			m_locc = mca;

			SetLastModifiedCol(mca);

			// highlight the length of the search string
			//
			if((ec = m_buffer->GetLineText(mla, lpText, nText)) != errOK)
				break;
			if(mca > nText)
				mca = nText;

			for(mcb = mca, mlb = mla, i = m_searchlen; i > 0; i--)
			{
				if(++mcb > nText)
				{
					mcb = 1;
					if((ec = m_buffer->GetLineText(++mlb, lpText, nText)) != errOK)
						break;
				}
			}
			// highlight the text found
			m_rla = mla;
			m_rca = mca;
			m_rlb = mlb;
			m_rcb = mcb;

			// if regular search, select the text also
			//
			if(! m_playing && ! m_recording && ! m_regional)
			{
				if((m_qla <= 0 && m_qlb <= 0)||(! GetHandMarked()))
					SetMarkedRegion(mla, mca, mlb, mcb);
				MoveAbs(mlb, mcb);
			}
			else
			{
				MoveAbs(mla, mca);
			}
			needvu = true;
		}
		else
		{
			MoveAbs(svline, svcol);
			_sntprintf(status, STATUS_LEN, _T("String " _Pfs_ " not found"), m_searchstr);
			m_editor->SetStatus(status);
			ec = errOBJECT_NOT_FOUND;
		}
		break;

	case Replace:

		ec = CheckReadOnly();
		if(ec != errOK) break;
		ec = Dispatch(DeleteBlock);
		ec = m_buffer->Insert(m_curline, m_curcol, m_replacestr, m_replacelen);
		break;

	case SetSearch:

		ec = PopParm(_T("Search For"), ptString, pParm, nParm);
		if(ec != errOK || nParm <= 0) break;
		if(nParm > MAX_SEARCH_STR - 1)
			nParm = MAX_SEARCH_STR - 1;
		_tcsncpy(m_searchstr, pParm, nParm);
		m_searchstr[nParm] = 0;
		m_searchlen = nParm;
		break;

	case SetReplace:

		ec = PopParm(_T("Replace With"), ptString, pParm, nParm);
		if(ec != errOK || nParm < 0) break;
		if(nParm > MAX_SEARCH_STR - 1)
			nParm = MAX_SEARCH_STR - 1;
		_tcsncpy(m_replacestr, pParm, nParm);
		m_replacestr[nParm] = 0;
		m_replacelen = nParm;
		break;

	case ReplaceFoundText:

		ec = CheckReadOnly();
		if(ec != errOK) break;

		if(m_locl > 0 && m_locc > 0)
		{
			if(! m_playing && ! m_recording && ! m_regional && GetMarkedRegion(mla, mca, mlb, mcb))
				CheckKillBlock();
			else
				ec = m_buffer->Delete(m_locl, m_locc, m_searchlen);
			ec = m_buffer->Insert(m_locl, m_locc, m_replacestr, m_replacelen);
			m_curline = m_locl;
			m_curcol  = m_locc;
			prevline = min(prevline, m_locl);
			needvu = true;
		}
		break;

	case ::SearchAndReplace:

		ec = SearchAndReplace();
		break;

	case SetSearchDirection:

		nParm = m_searchrev ? 1 : 0;
		if(PopParm(_T("Search Direction (0-fwd, 1-rev)"), ptNumber, nParm) != errOK)
			break;
		m_searchrev = nParm != 0;
		break;

	case SetSearchMode:

		break;

	case FindFile:

		break;

	case FindInFile:

		{
			BgrepInfo* pInfo;
			int		   i;

			// if there is a simple (word) selection, use it as initial pattern
			//
			if(GetMarkedRegion(mla, mca, mlb, mcb))
			{
				if(mla == mlb && mcb > mca)
				{
					if((ec = m_buffer->GetLineText(mla, lpText, nText)) == errOK)
					{
						for(i = 0; mca < mcb && i < MAX_SEARCH_STR; i++, mca++)
						{
							m_searchstr[i] = lpText[mca-1];
						}
						m_searchstr[i] = _T('\0');
						m_searchlen = i;
					}
				}
			}
			if(m_searchlen <= 0)
				m_searchstr[0] = _T('\0');

			// clear any previously found marks in views
			//
			for(pEntry = m_editor->GetBuffers(); pEntry; pEntry = pEntry->GetNext(pEntry))
			{
				pEntry->GetValue()->ClearLinesInfo(liIsSearchTarget);
			}

			// use pane 2 if active, else pane 1
			//
			pInfo = (BgrepInfo*)m_editor->GetPane(_T("Info"), _T("Find in Files 2"));
			if(! pInfo || ! pInfo->GetActive())
				pInfo = (BgrepInfo*)m_editor->GetPane(_T("Info"), _T("Find in Files"));
			if(pInfo)
			{
				pInfo->Startup(m_searchstr, NULL, NULL, (m_searchstr[0] != _T('\0')));
			}
			else
			{
				MessageBox(NULL, _T("Find-in-File not configured"), m_editor->GetVersion(), MB_OK);
				ec = errFAILURE;
			}
		}
		break;

	case ReplaceInFile:

		break;

	case FindDefinitionOf:

		{
			TCHAR searchstr[MAX_SEARCH_STR];
			TCHAR searchdir[MAX_SEARCH_STR];
			TCHAR searchfiles[MAX_SEARCH_STR];
			BgrepInfo* pInfo;

			Bpdb*	pPDB;
			LPBSYM	pSym;
			ERRCODE ec;
			int		tline, tcol;
			bool	isFunc;

			if(m_editor->GetProject())
			{
				pPDB = m_editor->GetProject()->GetPDB();
				if(pPDB)
				{
					tline = m_curline;
					tcol  = m_curcol;
					pSym = NULL;

					ec = GetDefinitionOfTokenAtPoint(tline, tcol, pSym, pPDB, isFunc);
					if(ec != errOK || ! pSym || ! pSym->f_file)
					{
						break;
					}
					// open file at line
					//_tprintf(_T("open file " _Pfs_ " at line %d\n"), pSym->f_file->f_name, pSym->f_line);

					ec = m_editor->EditBuffer(pSym->f_file->f_name, true, btAny, (TEXTENCODING)-1, pSym->f_line);
				}
			}
		}
		break;

	case FindReferencesOf:

		{
			TCHAR searchstr[MAX_SEARCH_STR];
			TCHAR searchdir[MAX_SEARCH_STR];
			TCHAR searchfiles[MAX_SEARCH_STR];
			BgrepInfo* pInfo;

			Bpdb*	pPDB;
			LPBSYM	pSym;
			ERRCODE ec;
			int		tline, tcol;

			TCHAR	varName[256];
			TCHAR	typeName[256];
			TCHAR	className[256];
			int		nVar, nType, nClass;
			TCHAR	delim, leftdelim;
			int		leftdelcol;
			bool	isFunc;

			if(m_editor->GetProject())
			{
				pPDB = m_editor->GetProject()->GetPDB();
				if(pPDB)
				{
					tline = m_curline;
					tcol  = m_curcol;
					pSym = NULL;

					nVar  = 256;
					ec = GetVarname(tline, tcol, varName, nVar, delim, leftdelim, leftdelcol, isFunc);
					if(ec != errOK)
					{
						break;
					}
					pParm = varName;
					nParm = nVar;
					if(nParm > MAX_SEARCH_STR - 1)
						nParm = MAX_SEARCH_STR - 1;
					_tcsncpy(searchstr, pParm, nParm);
					searchstr[nParm] = _T('\0');

					// use project directory for search path
					ec = m_editor->GetProject()->GetSourceDirectory(searchdir, MAX_SEARCH_STR - 1);
					if (ec != errOK)
					{
						pParm = _T(".");
						nParm = 2;
						if(nParm > MAX_SEARCH_STR - 1)
							nParm = MAX_SEARCH_STR - 1;
						_tcsncpy(searchdir, pParm, nParm);
						searchdir[nParm] = _T('\0');
					}
					// use type file family for pattern?
					pParm = GetSearchExtensions();
					nParm = _tcslen(pParm);
					if(nParm > MAX_SEARCH_STR - 1)
						nParm = MAX_SEARCH_STR - 1;
					_tcsncpy(searchfiles, pParm, nParm);
					searchfiles[nParm] = _T('\0');
				}
			}

			pInfo = (BgrepInfo*)m_editor->GetPane(_T("Info"), _T("Find in Files 2"));
			if(! pInfo || ! pInfo->GetActive())
				pInfo = (BgrepInfo*)m_editor->GetPane(_T("Info"), _T("Find in Files"));
			if(pInfo)
			{
				pInfo->StartupDirect(searchstr, searchdir, searchfiles);
			}
			else
			{
				MessageBox(NULL, _T("Find-in-File not configured"), m_editor->GetVersion(), MB_OK);
				ec = errFAILURE;
			}
		}
		break;

	/*
	 * Item insertion/deletion
	 */
	case SelfInsert:
	case Insert:

		{
			LPTSTR pExp = NULL;
			LPTSTR pRnl = NULL;
			int    nExp = 0;

			ec = CheckReadOnly();
			if(ec != errOK)
			{
				PopParm(NULL, ptString, pParm, nParm);
				break;
			}

			CheckKillBlock();

			// get text to insert
			if((ec = PopParm(_T("Insert"), ptString, pParm, nParm)) != errOK)
				break;

			// if converting tabs to spaces, do that now
			if(m_tabsasspaces)
			{
				ec = UntabText(pParm, nParm, pExp, nExp);

				if(ec == errOK && nExp > 0)
				{
					pParm = pExp;
					nParm = nExp;
				}
			}
			// if the buffer is not raw, then change CRLF->LF in text,
			// and change dangling CR to LF since text in editor is always just LF
			//
			if(m_buffer && ! m_buffer->GetRaw())
			{
				int i, j;

				pRnl = new TCHAR [ nParm + 1 ];

				for(i = j = 0; j < nParm;)
				{
					if(pParm[j] == _T('\r'))
					{
						pRnl[i++] = _T('\n');
						j++;
						if(j < nParm-1)
						{
							if(pParm[j] == _T('\n'))
								j++;
						}
					}
					else
					{
						pRnl[i++] = pParm[j++];
					}
				}
				pRnl[i] = _T('\0');
				nParm = i;
				pParm = pRnl;
			}
			// if overstrike, remove that many
			if(m_overstrike)
			{
				ec = m_buffer->AddUndoInfo(utPosition, m_curline, m_curcol, m_curline, m_curcol, NULL, 0);
				ec = m_buffer->Delete(m_curline, m_curcol, nParm);
			}
			// insert the text
			SetLastModifiedCol(m_curcol);
			ec = m_buffer->Insert(m_curline, m_curcol, pParm, nParm);
			if(pExp) delete [] pExp;
			if(pRnl) delete [] pRnl;
		}
		needvu = true;
		break;

	case Paste:

		ec = CheckReadOnly();
		if(ec != errOK) break;

		ec = errFAILURE;
		m_suppressmacro = true;
		if(OpenClipboard(m_hwnd))
		{
			HANDLE hMem = NULL;
			int    isunicode = 0;
			LPTSTR pcp;
#ifdef UNICODE
			hMem = GetClipboardData(CF_UNICODETEXT);
			if(hMem)
			{
				isunicode = 1;
			}
#endif
			if(! hMem)
			{
				hMem = GetClipboardData(CF_TEXT);
			}
			if(hMem)
			{
				if((pcp = (LPTSTR)GlobalLock(hMem)) != NULL)
				{
					int cblen;

					if(isunicode)
					{
						cblen = _tcslen(pcp);
						lpText = new TCHAR [ cblen + 4 ];
						_tcscpy((LPTSTR)lpText, pcp);
					}
					else
					{
						cblen = strlen((LPSTR)pcp);
						lpText = new TCHAR [ cblen + 4 ];
						CharToTChar((LPTSTR)lpText, (LPSTR)pcp);
					}
					CheckKillBlock();
					ec = PushParm(lpText, _tcslen(lpText), ptString);
					ec = Dispatch(Insert);
					delete [] (LPTSTR)lpText;
				}
			}
			CloseClipboard();
		}
		m_suppressmacro = false;
		break;

	case Delete:

		ec = CheckReadOnly();
		if(ec == errOK)
		{
			int ndel = 1;

			SetLastModifiedCol(m_curcol);

			// if converting tabs to spaces, convert a delete of
			// all spaces at the start of a tab-stop to delete of a spaced-tab
			//
			if(m_tabsasspaces && (((m_curcol - 1) % m_tabspace) == 0))
			{
				ec = m_buffer->GetLineText(m_curline, lpText, nText);
				if(ec == errOK && nText > 0)
				{
					int spacecol = m_curcol;

					// count spaces from here forward to next tab stop to the right
					//
					while(spacecol <= nText && lpText[spacecol-1] == ' ' && (spacecol % m_tabspace))
					{
						ndel++;
						spacecol++;
					}
					// if the search forward stopped right before the next tab stop then
					// delete all the spaces, else just the 1
					//
					if(lpText[spacecol-1] != ' ' || spacecol > nText)
					{
						ndel = 1;
					}
				}
			}
			ec = m_buffer->AddUndoInfo(utPosition, m_curline, m_curcol, m_curline, m_curcol, NULL, 0);
			ec = m_buffer->Delete(m_curline, m_curcol, ndel);
			needvu = true;
		}
		break;

	case DeleteBack:

		ec = CheckReadOnly();
		if(ec != errOK) break;

		CheckKillBlock();
		if(m_curcol > 1)
		{
			int ndel = 1;
			int sc = m_curcol;

			m_curcol--;

			// if converting tabs to spaces, convert a delete-back
			// of a space to (kind of) a delete-back of a tab
			//
			if(m_tabsasspaces)
			{
				ec = m_buffer->GetLineText(m_curline, lpText, nText);
				if(ec == errOK && nText > 0)
				{
					int spacecol = m_curcol - 1;

					// count spaces from here back to first tab stop to the left
					//
					while(spacecol >= 0 && lpText[spacecol] == ' ' && (spacecol % m_tabspace))
					{
						ndel++;
						spacecol--;
					}
					// if the search back stopped on a tab-stop and there is still a
					// space character there, then delete all the spaces, else just the 1
					//
					if(lpText[spacecol] != ' ')
					{
						ndel = 1;
					}
					else
					{
						m_curcol = spacecol + 1;
					}
				}
			}
			ec = m_buffer->Delete(m_curline, m_curcol, ndel);
			SetLastModifiedCol(m_curcol);
		}
		else if(m_curline > 1)
		{
			m_curline--;
			m_curcol = 0x7fffffff;
			ec = m_buffer->AddUndoInfo(utPosition, m_curline, m_curcol, m_curline, m_curcol, NULL, 0);
			ec = m_buffer->Delete(m_curline, m_curcol, 1);
			SetLastModifiedCol(m_curcol);
		}
		needvu = true;
		break;

	case DeleteEOL:

		ec = CheckReadOnly();
		if(ec != errOK) break;

		CheckKillBlock();
		ec = MoveAbs(m_curline, m_curcol);
		ec = m_buffer->GetLineText(m_curline, lpText, nText);
		if((nText -= m_curcol) > 0)
		{
			ec = m_buffer->AddUndoInfo(utPosition, m_curline, m_curcol, m_curline, m_curcol, NULL, 0);
			ec = m_buffer->Delete(m_curline, m_curcol, nText);
		}
		needvu = true;
		break;

	case DeleteEolOrLineOrBlock:

		ec = CheckReadOnly();
		if(ec != errOK) break;

		// if view has marked region, delete that region
		//
		m_suppressmacro = true;
		if(GetMarkedRegion(mla, mca, mlb, mcb))
		{
			ec = Dispatch(Cut);
		}
		else
		{
			// if the column is one, delete the line
			//
			ec = m_buffer->GetLineText(m_curline, lpText, nText);

			if(m_curcol == 1)
			{
				if((nText - (m_curcol - 1)) > 0)
				{
					SetMarkedRegion(m_curline, 1, m_curline, nText + 1);
					ec = Dispatch(Cut);
				}
			}
			else
			{
				// just delete to end of line
				//
				SetMarkedRegion(m_curline, m_curcol, m_curline, nText);
				ec = Dispatch(Cut);
			}
		}
		m_suppressmacro = false;
		needvu = false;
		break;

	case DeleteBlock:

		ec = CheckReadOnly();
		if(ec != errOK) break;

		if(GetMarkedRegion(mla, mca, mlb, mcb))
		{
			int nDelete;
			int temp;

			nDelete = CountRegionChars(m_buffer, mla, mca, mlb, mcb);
			SetHandMarked(false);
			ClearMark();
			ClearSearchHighlight();

			if(nDelete > 0)
			{
				if(mla > mlb)
				{
					temp = mla;

					mla  = mlb;
					mlb  = temp;
					temp = mca;
					mca  = mcb;
					mcb  = temp;
				}
				else if(mla == mlb)
				{
					if(mca > mcb)
					{
						temp = mcb;
						mcb  = mca;
						mca  = temp;
					}
				}
				ec = m_buffer->AddUndoInfo(utPosition, m_curline, m_curcol, m_curline, m_curcol, NULL, 0);
				ec = m_buffer->Delete(mla, mca, nDelete);
				m_curline = mla;
				m_curcol  = mca;
			}
		}
		else
		{
			m_suppressmacro = true;
			ec = Dispatch(::Delete);
			m_suppressmacro = false;
		}
		needvu = true;
		break;

	case Cut:

		ec = CheckReadOnly();
		if(ec != errOK) break;

		if(! GetMarkedRegion(mla, mca, mlb, mcb))
			break;
		ec = Dispatch(Copy);
		SetMarkedRegion(mla, mca, mlb, mcb);
		return Dispatch(DeleteBlock);

	case Copy:

		if(GetMarkedRegion(mla, mca, mlb, mcb))
		{
			LPTSTR	lpCopied;
			int		nCopy;
			int		temp;

			nCopy = CountRegionChars(m_buffer, mla, mca, mlb, mcb);
			ClearMark();

			if(nCopy > 0)
			{
				if(mla > mlb)
				{
					temp = mla;

					mla  = mlb;
					mlb  = temp;
					temp = mca;
					mca  = mcb;
					mcb  = temp;
				}
				else if(mla == mlb)
				{
					if(mca > mcb)
					{
						temp = mcb;
						mcb  = mca;
						mca  = temp;
					}
				}
				ec = m_buffer->Copy(mla, mca, nCopy, lpCopied);

				if(nCopy > 0)
				{
					OpenClipboard(m_hwnd);
					EmptyClipboard();

					// set copied to the cut buffer
					//
#ifdef UNICODE
					HANDLE hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (nCopy + 1) * sizeof(WCHAR));
					LPWSTR  pcp = (LPWSTR)GlobalLock(hMem);

					memcpy(pcp, lpCopied, nCopy * sizeof(WCHAR));
					pcp[nCopy] = _T('\0');
					SetClipboardData(CF_UNICODETEXT, hMem);
#else
					HANDLE hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (nCopy + 1) * sizeof(char));
					LPSTR  pcp = (char*)GlobalLock(hMem);

					for(temp = 0; temp < nCopy; temp++)
						pcp[temp] = (char)lpCopied[temp];
					pcp[temp] = '\0';
					SetClipboardData(CF_TEXT, hMem);
#endif
					GlobalUnlock(hMem);
					CloseClipboard();
				}
				if(lpCopied)
				{
					delete [] lpCopied;
				}
			}
			MoveAbs(m_curline, m_curcol);
			needvu = true;
		}
		break;

	case Type:

		break;

	/*
	 * Marking
	 */
	case SetMark:

		SetHandMarked(true);
		SetMarkedRegion(m_curline, m_curcol, m_curline, m_curcol);
		break;

	case ::ClearMark:

		SetHandMarked(false);
		ClearMark();
		needvu = true;
		break;

	case SwapMark:

		if(GetMarkedRegion(mla, mca, mlb, mcb))
			SetMarkedRegion(mlb, mcb, mla, mca);
		needvu = true;
		break;

	case ToggleMark:

		if(GetMarkedRegion(mla, mca, mlb, mcb) || GetHandMarked())
			return Dispatch(::ClearMark);
		else
			return Dispatch(::SetMark);
		needvu = true;
		break;

	case SelectChar:

		SetHandMarked(false);
		SetMarkedRegion(m_curline, m_curcol, m_curline, m_curcol+1);
		needvu = true;
		break;


	case SelectWord:

		SetHandMarked(false);
		ec = m_buffer->GetLineText(m_curline, lpText, nText);
		if(ec != errOK) break;
		if(nText <= 0) break;
		if(IsDelim(lpText[m_curcol - 1])) break;
		mla = mlb = m_curline;
		mca = mcb = m_curcol;
		while(mca >= 1)
		{
			if(! IsDelim(lpText[mca - 1]))
				mca--;
			else
				break;
		}
		mca++;
		while(mcb <= nText)
		{
			if(IsDelim(lpText[mcb - 1]))
			{
				break;
			}
			mcb++;
		}
		SetMarkedRegion(mla, mca, mlb, mcb);
		needvu = true;
		break;


	case SelectLine:

		SetHandMarked(false);
		SetMarkedRegion(m_curline, 1, m_curline, MAX_LINE_LEN);
		needvu = true;
		break;


	case SelectAll:

		SetHandMarked(false);
		mla = m_buffer->GetLineCount(mca);
		SetMarkedRegion(1, 1, mla, mca);
		needvu = true;
		break;


	/*
	 * Buffer
	 */
	case NewBuffer:
	case OpenBuffer:

		ec = PopParm(
						_T("File To Edit"),
						command == NewBuffer ? ptOpenFilename : ptOpenExistingFilename,
						pParm,
						nParm
					 );
		if(ec == errOK)
		{
			TEXTENCODING    textEncoding;
			BufType		    bufType;
			LINETERMINATION lineterm;

			textEncoding = txtANSI;
			if((ec = PopParm(_T("Text Encoding"), ptTextEncoding, (int&)textEncoding)) == errOK)
			{
				lineterm = ltLF;
				if((ec = PopParm(_T("Line Termination (0=LF, 1=CRLF)"), ptTextLineTerm, (int&)lineterm)) == errOK)
				{
					bufType = btAny;
					if((ec = PopParm(_T("Open As (0=Auto, 1=Text, 2=Binary"), ptBufferType, (int&)bufType)) == errOK)
					{
						ec = m_editor->EditBuffer(pParm, true, bufType, textEncoding);
					}
				}
			}
		}
		return ec;

	case ReadBuffer:

		if(PopParm(_T("File To Insert"), ptOpenFilename, pParm, nParm) != errOK)
			break;
		break;

	case NextBuffer:

		for(pEntry = m_editor->GetBuffers(); pEntry; pEntry = pEntry->GetNext(pEntry))
		{
			if(pEntry->GetValue() == m_buffer)
			{
				if(pEntry->GetNext(pEntry))
					pNext = pEntry->GetNext(pEntry)->GetValue();
				else
					pNext = m_editor->GetBuffers()->GetValue();
				ec = m_editor->EditBuffer(pNext);
				return ec;
			}
		}
		break;

	case PreviousBuffer:

		for(pEntry = m_editor->GetBuffers(); pEntry->GetNext(pEntry); pEntry = pEntry->GetNext(pEntry))
		{
			if(pEntry->GetNext(pEntry)->GetValue() == m_buffer)
			{
				pNext = pEntry->GetValue();
				ec = m_editor->EditBuffer(pNext);
				return ec;
			}
		}
		for(pEntry = m_editor->GetBuffers(); pEntry->GetNext(pEntry);)
			pEntry = pEntry->GetNext(pEntry);

		if(pEntry->GetValue() != m_buffer)
		{
			pNext = pEntry->GetValue();
			ec = m_editor->EditBuffer(pNext);
			return ec;
		}
		break;

	case SaveBuffer:

		ClearMouseSelection();
		if(m_buffer->GetUntitled())
		{
			return Dispatch(SaveAs);
		}
		if(GetTrimOnWrite() && ! m_buffer->GetRaw())
		{
			ec = Dispatch(TrimWhitespace);
		}
		ec = m_buffer->Write();
		if(ec != errOK)
		{
			int rc;

			_sntprintf(status, MAX_PATH + 64, _T("Could not write " _Pfs_ ", Write to Alternate ?"), m_buffer->GetName());
			rc = MessageBox(NULL, status, m_editor->GetVersion(), MB_OKCANCEL);
			if(rc == IDCANCEL) break;
			return Dispatch(SaveAs);
		}
		// pass off the buffer saved to the project/program database
		// thread to explore (if enabled)
		//
		if(m_editor->GetProject() && m_editor->GetProject()->GetPDB())
		{
		//	m_editor->GetProject()->GetPDB()->AddFile(GetBuffer());
			m_editor->GetProject()->GetPDB()->AddFile(GetBuffer()->GetName(), true);
		}
		needvu = true;
		break;

	case SaveAs:

		ClearMouseSelection();
		{
			bool respecify;

			do
			{
				respecify = false;
				if((ec = PopParm(_T("Save As File"), ptSaveFilename, pParm, nParm)) == errOK)
				{
					LINETERMINATION lineterm;
					TEXTENCODING	textEncoding;
					BufType			bufType;

					if((ec = PopParm(_T("Text Encoding"), ptTextEncoding, (int&)textEncoding)) == errOK)
					{
						if(textEncoding == (TEXTENCODING)-1)
						{
							textEncoding = m_buffer->GetEncoding();
							if(textEncoding == (TEXTENCODING)-1)
							{
								textEncoding = txtANSI;
							}
						}
					#ifdef _WIN32
						lineterm = ltCRLF;
					#else
						lineterm = ltLF;
					#endif
						if((ec = PopParm(_T("Line Termination (0=LF, 1=CRLF)"), ptTextLineTerm, (int&)lineterm)) == errOK)
						{
							bufType = btAny;
							if((ec = PopParm(_T("Save As (0=Auto, 1=Text, 2=Binary"), ptBufferType, (int&)bufType)) == errOK)
							{
								int    rc;

								if(! m_buffer->GetIsFTP() && BUtil::FileExists(pParm) == errOK)
								{
									_sntprintf(status, MAX_PATH + 64, _T("Overwrite Existing " _Pfs_ ""), pParm);
									rc = MessageBox(NULL, status, m_editor->GetVersion(), MB_OKCANCEL);
								}
								else
								{
									rc = IDOK;
								}
								if(rc == IDOK)
								{
									TCHAR newpath[MAX_PATH];

									// save path cause pparm can be freed in dispatch
									_tcsncpy(newpath, pParm, MAX_PATH - 1);
									newpath[MAX_PATH - 1] = _T('\0');

									if(! m_buffer->GetNew()	&& ! m_buffer->GetRaw() &&
										((m_buffer->GetHasCR()  && lineterm == ltLF)	||
										 (!m_buffer->GetHasCR() && lineterm == ltCRLF))
									)
									{
										if(m_buffer->GetHasCR())
											_sntprintf(status, MAX_PATH + 64,
												_T("Change CR-LF endings to LF for " _Pfs_ " ?"), newpath);
										else
											_sntprintf(status, MAX_PATH + 64,
												_T("Change LF endings to CR-LF for " _Pfs_ " ?"), newpath);

										rc = MessageBox(NULL, status, m_editor->GetVersion(), MB_OKCANCEL);

										if(rc == IDCANCEL) break;
										m_buffer->SetHasCR(lineterm == ltCRLF);
									}
									else if(m_buffer->GetNew() && ! m_buffer->GetRaw())
									{
										m_buffer->SetHasCR(lineterm == ltCRLF);
									}
								#if 0 // maybe user wants trailing white space in new buffer, not current ?
									if(GetTrimOnWrite() && ! m_buffer->GetRaw())
									{
										ec = Dispatch(TrimWhitespace);
									}
								#endif
									// save state of current buffer
									bool ismod = m_buffer->GetModified();
									bool isread = m_buffer->GetRead();

									ec = m_buffer->Write(newpath, textEncoding);
									m_buffer->SetModified(ismod);
									m_buffer->SetRead(isread);

									if(ec != errOK)
									{
										_sntprintf(status, MAX_PATH + 64, _T("Could not write " _Pfs_ ", Reenter Name ?"), newpath);
										rc = MessageBox(NULL, status, m_editor->GetVersion(), MB_OKCANCEL);
										if(rc == IDCANCEL) break;
										respecify = true;
									}
									else
									{
										ec = m_editor->EditBuffer(newpath, true, bufType, textEncoding);

										// close
									}
								}
							}
						}
					}
				}
			}
			while(respecify);
		}
		break;

	case SaveAll:

		ClearMouseSelection();
		if(m_buffer->GetModified())
			ec = Dispatch(SaveBuffer);

		for(
				pEntry = m_editor->GetBuffers(), mla = 0;
				pEntry && ec == errOK;
				pEntry = pEntry->GetNext(pEntry)
		)
		{
			pNext = pEntry->GetValue();

			if(pNext && pNext->GetModified() && pNext != m_buffer)
			{
				if(pNext->GetNew())
				{
					ec = pNext->GetEditor()->EditBuffer(pNext);
					mla = 1;
					if(ec == errOK)
						ec = Dispatch(SaveAs);
				}
				else
				{
					ec = pNext->Write();
					if(ec != errOK)
					{
						_sntprintf(status, 256, _T("Couldn't Write Buffer " _Pfs_ "\n"), pNext->GetName());
						m_editor->SetStatus(status);
					}
				}
			}
		}
		if(mla)
			m_editor->EditBuffer(m_buffer);
		needvu = true;
		break;

	case KillBuffer:

		ClearMouseSelection();
		return Dispatch(KillWindow);

	case EmptyBuffer:

		ec = Dispatch(SelectAll);
		ec = Dispatch(DeleteBlock);
		return ec;

	case ListBuffers:
	case MergeBuffers:

		break;
	/*
	 * Window
	 */
	case SplitWindow:

		nParm = 0;
		if(PopParm(_T("Split Horizontal (0) or Vertical (1)"), ptNumber, nParm) == errOK)
			return Dispatch(nParm ? SplitVertical : SplitHorizontal);
		break;

	case SplitVertical:

		{
			PBAFPANEL	pl, pr;
			PBAFPANE	px;
			Bview*		pMyView;
			RECT		rcp, rcl;

			// find an edit panel
			BappPanel* pe = GetParentPanel();

			if(! pe) return errFAILURE;
			GetClientRect(pe->GetWindow(), &rcp);

			// create two subpanels
			pl = new BappPanel(_T("EditL"), pe, frTopTabs);
			pr = new BappPanel(_T("EditR"), pe, frTopTabs);

			// dock them into the current panel
			rcl = rcp;
			rcl.right /= 2;
			rcl.right -= 18;
			pe->Dock(pl, frLeft, &rcl, true, false);

			rcp.left = rcl.right;
			pe->Dock(pr, frLeft, &rcp, true, true);

			pView = pMyView = NULL;

			// take all existing panes (including us!) out of the parent panels pane list
			// and put them in the left side list, and duplicate them on the right side
			//
			while((px = pe->GetPanes()) != NULL)
			{
				pe->RemovePane(px);

				pl->AddPane(px);

				// create a new view on the pane's buffer for the right panel;
				pView = m_editor->ProperViewForBuffer(((Bview*)px)->GetBuffer(), m_editor, pr);
				// add to views list to insure buffer isnt deleted on view removal
				m_editor->AddView(pView);

				// add it as the pane for the right panel
				pr->AddPane(pView);
			}
			// activate the new location of us on the left side
			pl->ActivatePane(this);

			// and set the view that ended up on the right side as the current view
			m_editor->SetCurrentView(pView, false);

			PostMessage(m_editor->GetWindow(), WM_SIZE, 0, 0);
		}
		break;

	case SplitHorizontal:

		{
			PBAFPANEL	pt, pb;
			PBAFPANE	px;
			RECT		rcp, rct;

			// find an edit panel
			BappPanel* pe = GetParentPanel();

			if(! pe) return errFAILURE;
			GetClientRect(pe->GetWindow(), &rcp);

			// create two subpanels
			pt = new BappPanel(_T("EditT"), pe, frTopTabs);
			pb = new BappPanel(_T("EditB"), pe, frTopTabs);

			// dock them into the current panel
			rct = rcp;
			rct.bottom /= 2;
			pe->Dock(pt, frTop, &rct, true, false);

			rcp.top = rct.bottom;
			pe->Dock(pb, frTop, &rcp, true, true);

			// take all existing panes (including us!) out of the parent panels pane list
			// and put them in the top side list, and dupicate them to the bottom list
			//
			while((px = pe->GetPanes()) != NULL)
			{
				pe->RemovePane(px);

				pt->AddPane(px);

				// create a new view on the pane's buffer for the top panel;
				pView = m_editor->ProperViewForBuffer(((Bview*)px)->GetBuffer(), m_editor, pb);
				// add to views list to insure buffer isnt deleted on view removal
				m_editor->AddView(pView);

				// add it as the pane for the tpop panel
				pb->AddPane(pView);
			}
			// activate the new location of us on the top side
			pt->ActivatePane(this);

			// and set the view that ended up on the right side as the current view
			m_editor->SetCurrentView(pView, false);

			PostMessage(m_editor->GetWindow(), WM_SIZE, 0, 0);
		}
		break;

	case NextWindow:

		ClearMouseSelection();
		if(! (pView = m_next))
			pView = m_editor->GetViews();
		if(pView != this)
			return m_editor->SetCurrentView(pView, true);
		break;

	case PreviousWindow:

		ClearMouseSelection();
		for(pView = m_editor->GetViews(); pView->GetNext(pView); pView = pView->GetNext(pView))
			if(pView->GetNext(pView) == this)
				return m_editor->SetCurrentView(pView, true);
		if((pView = m_editor->GetViews()) != this)
			return m_editor->SetCurrentView(pView, true);
		break;

	case RefreshWindow:

		UpdateView(1, 1, 0x7fffffff, 0x7fffffff);
		break;

	case RecenterWindow:

		RecenterView();
		break;

	case KillWindow:

		for(pView = m_editor->GetViews(), mca = mcb = 0; pView; pView = pView->GetNext(pView))
		{
			if(pView->GetBuffer() == m_buffer)
				mcb++; // num views on buffer
			mca++; // num views total
		}
		//
		if(mcb <= 1)
		{
			// no more views on this buffer, so close the buffer
			//
			if(CheckModified(true))
			{
				m_editor->RemoveBuffer(m_buffer);

				// if no more views period, exit
				if(mca <= 1)
					PostQuitMessage(0);
			}
		}
		// set current view to next view IN the parent
		// panel (if any) or the master list
		//
		if(mca > 1)
		{
			PBAFPANEL pp;
			PBAFPANE  pa, px;

			pp = GetParentPanel();
			if(pp)
			{
				px = NULL;
				for(pa = pp->GetPanes(); pa; pa = pa->GetNext())
				{
					if(pa == this)
						break;
					px = pa;
				}
				if(px == NULL)
					px = pp->GetPanes();
				if(px == this)
				{
					// there is only one pane, and it will be killed
					// below, so just move to the next view
					pView = GetNext(this);
				}
				else
				{
					pView = (Bview*)px;
				}
			}
			else
			{
				pView = GetNext(this);
			}
			if(! pView)
			{
				pView = m_editor->GetViews();
			}
		}
		{
			PBAFPANEL pp, px;
			PBAFPANE  pa;

			pp = GetParentPanel();

			Close();

			if(pp)
			{
				// close the panel if no more panes
				//
				if(pp->GetPanes() == NULL)
				{
					px = pp->GetParentPanel();
					if(px)
					{
						int np = 0;

						// unlink panel from parent panel
						px->RemovePanel(pp);
						delete pp;

						// if parent panel is left with only one sub panel, then
						// repane it with the panes from sub panel and kill that too
						//
						for(pp = px->GetPanels(frLeft);		pp; pp = pp->GetNext()) np++;
						for(pp = px->GetPanels(frRight);	pp; pp = pp->GetNext()) np++;
						for(pp = px->GetPanels(frTop);		pp; pp = pp->GetNext()) np++;
						for(pp = px->GetPanels(frBottom);	pp; pp = pp->GetNext()) np++;

						if(np == 1)
						{
							// only one panel left
							//
							if(! (pp = px->GetPanels(frLeft)))
								if(! (pp = px->GetPanels(frRight)))
									if(! (pp = px->GetPanels(frTop)))
										pp = px->GetPanels(frBottom);
							if(pp)
							{
								while((pa = pp->GetPanes()) != NULL)
								{
									pp->RemovePane(pa);
									px->AddPane(pa);
								}
								px->RemovePanel(pp);
								pView = (Bview*)px->GetPanes();
								delete pp;
							}
						}
					}
					else
					{
						// no parent panel and no pane?  this means no views at all
						// which should have been handled above
						delete pp;
					}
				}
			}
		}
		if(mca > 1)
		{
			if(pView)
			{
				m_editor->SetCurrentView(pView, true);
				pView->Activate(); // incase it is already current

				// tell editor to re-layout views
				PostMessage(m_editor->GetWindow(), WM_SIZE, 0, 0);
				InvalidateRect(pView->GetWindow(), NULL, TRUE);
			}
		}
		return errOK;

	case KillAllWindows:

		PostQuitMessage(0);
		break;

	case SetBinRadix:

		if(m_buffer->GetRaw() || ! _tcscmp(GetTypeName(), _T("Binary")))
		{
			nParm = GetViewRadix();
			if((ec = PopParm(_T("Radix"), ptNumber, nParm)) != errOK)
				break;
			switch(nParm)
			{
			case 2:
			case 8:
			case 10:
			case 16:
				SetViewRadix(nParm);
				needvu = true;
				multiLine = true;
				break;
			default:
				MessageBox(NULL, _T("Bad Radix, choose 1, 8, 10, 16"), m_editor->GetVersion(), MB_OK);
				ec = errFAILURE;
				break;
			}
		}
		else
		{
			MessageBox(NULL, _T("Not a Binary View"), m_editor->GetVersion(), MB_OK);
			ec = errFAILURE;
		}
		break;

	case SetBinFormat:

		if (m_buffer->GetRaw() || !_tcscmp(GetTypeName(), _T("Binary")))
		{
			nParm = GetViewFormat();
			if((ec = PopParm(_T("View Format"), ptBinFormat, nParm)) != errOK)
				break;

			switch(nParm & 0xF)
			{
			case 1:
			case 2:
			case 4:
				SetViewFormat(nParm);
				needvu = true;
				multiLine = true;
				break;
			default:
				MessageBox(NULL, _T("Bad Format, choose (1, 2, or 4)"), m_editor->GetVersion(), MB_OK);
				ec = errFAILURE;
				break;
			}
		}
		else
		{
			MessageBox(NULL, _T("Not a Binary View"), m_editor->GetVersion(), MB_OK);
			ec = errFAILURE;
		}
		break;

	case SetBinColumns:

		if(m_buffer->GetRaw())
		{
			nParm = m_buffer->GetColumns();
			if((ec = PopParm(_T("View Columns"), ptNumber, nParm)) != errOK)
				break;
			if(nParm > 0)
				ec = m_buffer->SetColumns(nParm);
			needvu = true;
			multiLine = true;
		}
		else
		{
			MessageBox(NULL, _T("Not a Binary View"), m_editor->GetVersion(), MB_OK);
			ec = errFAILURE;
		}
		break;
	/*
	 * Undo
	 */
	case Undo:

		ClearMouseSelection();
		ec = m_buffer->Undo(m_curline, m_curcol);
		needvu = true;
		break;

	case Redo:

		ClearMouseSelection();
		ec = m_buffer->Redo(m_curline, m_curcol);
		needvu = true;
		break;
	/*
	 * Macros
	 */
	case ::DefineMacro:

		ClearMouseSelection();
		m_suppressmacro = false;
		if(m_recording)
		{
			m_recording = false;
			if(m_macro)
				m_editor->SetStatus(_T("Macro Defined"));
		}
		else
		{
			if(m_macro)
			{
				int rc = MessageBox(NULL, _T("Overwrite Existing Macro?"),
					m_editor->GetVersion(), MB_OKCANCEL);

				if(rc != IDOK)
					break;
				KillMacro(m_macro);
			}
			m_recording = true;
			m_editor->SetStatus(_T("Defining Macro"));
		}
		break;

	case ::PlayMacro:

		ClearMouseSelection();
		m_suppressmacro = false;
		m_recording = false;
		return PlayMacro(m_macro);

	case ::KillMacro:

		ec = KillMacro(m_macro);
		break;

	case ::SaveMacro:
	case ::RecallMacro:

		break;

	case PushString:
	case PushNumber:
	case PopString:
	case PopNumber:

		break;
	/*
	 * Misc
	 */
	case OverstrikeOn:

		SetOverstrike(true);
		break;

	case OverstrikeOff:

		SetOverstrike(false);
		break;

	case ToggleOverstrike:

		SetOverstrike(! GetOverstrike());
		break;

	case SetTab:
	case ClearTab:

		break;

	case SetTabSpace:

		nParm = m_tabspace;
		if(PopParm(_T("Tab Space"), ptNumber, nParm) != errOK)
			break;
		if(nParm > 1 && nParm < MAX_TAB_SPACE)
		{
			m_tabspace	  = nParm;
		}
		else
			m_editor->SetStatus(_T("Bad Tab Space"));
		prevline = 1;
		multiLine = true;
		needvu = true;
		break;

	case Tabify:

		ec = CheckReadOnly();
		if(ec != errOK) break;

		SetOpRegionFromSelection();
		if(m_regional)
		{
			SetHandMarked(false);
			mla = m_ola;
			mca = 1; // only whole lines... m_oca;
		}
		else
		{
			mla = 1;
			mca = 1;
		}
		mlb = 0;
		while(ec == errOK && (! m_regional || InRegion(mla, mca)))
		{
			ec = m_buffer->GetLineText(mla, lpText, nText);
			if(ec == errOK)
			{
				LPTSTR lpResult = NULL;
				int    nResult;

				ec = TabifyText(lpText, nText, lpResult, nResult);

				if(ec == errOK && nResult > 0)
				{
					needvu = true;
					ec = m_buffer->ReplaceLine(mla, lpResult, nResult);
					delete [] lpResult;
				}
			}
			if(ec != errOK || mla == mlb) break;
			mlb = mla;
			mla++;
		}
		ClearOpRegion();
		if(GetShowTabs())
		{
			multiLine = true;
			needvu = true;
		}
		MoveAbs(svline, svcol);
		break;

	case Untab:

		ec = CheckReadOnly();
		if(ec != errOK) break;

		SetOpRegionFromSelection();
		if(m_regional)
		{
			SetHandMarked(false);
			mla = m_ola;
			mca = 1; // only whole lines... m_oca;
		}
		else
		{
			mla = 1;
			mca = 1;
		}
		mlb = 0;
		while(ec == errOK && (! m_regional || InRegion(mla, mca)))
		{
			ec = m_buffer->GetLineText(mla, lpText, nText);
			if(ec == errOK)
			{
				LPTSTR lpResult = NULL;
				int    nResult;

				ec = UntabText(lpText, nText, lpResult, nResult);

				if(ec == errOK && nResult > 0)
				{
					needvu = true;
					ec = m_buffer->ReplaceLine(mla, lpResult, nResult);
					delete [] lpResult;
				}
			}
			if(ec != errOK || mla == mlb) break;
			mlb = mla;
			mla++;
		}
		ClearOpRegion();
		if(GetShowTabs())
		{
			multiLine = true;
			needvu = true;
		}
		MoveAbs(svline, svcol);
		break;

	case ToggleCarriageReturns:

		ec = CheckReadOnly();
		if(ec != errOK) break;
		m_buffer->SetHasCR(! m_buffer->GetHasCR());
		m_buffer->SetModified(true);
		UpdateStatus();
		break;

	case MakeWritable:

		ec = m_buffer->SetReadOnly(false);
		UpdateStatus();
		break;

	case ShowLineNums:
		if(GetShowLineNums())
		{
			break;
		}
		SetShowLineNums(true);
		break;

	case HideLineNums:
		if(! GetShowLineNums())
		{
			break;
		}
		SetShowLineNums(false);
		break;

	case ShowWhitespace:
		if(GetShowTabs())
		{
			break;
		}
		SetShowTabs(true);
		break;

	case HideWhitespace:
		if(! GetShowTabs())
		{
			break;
		}
		SetShowTabs(false);
		break;

	case TrimWhitespace:

		ec = CheckReadOnly();
		if(ec != errOK) break;

		SetOpRegionFromSelection();
		if(m_regional)
		{
			SetHandMarked(false);
			mla = m_ola;
			mca = 1; // only whole lines... m_oca;
		}
		else
		{
			mla = 1;
			mca = 1;
		}
		mlb = 0;
		while(ec == errOK && (! m_regional || InRegion(mla, mca)))
		{
			ec = m_buffer->GetLineText(mla, lpText, nText);
			if(ec == errOK)
			{
				LPTSTR lpResult = NULL;
				int    nResult;

				ec = WhiteTrimText(lpText, nText, lpResult, nResult);

				if(ec == errOK && nResult > 0)
				{
					needvu = true;
					ec = m_buffer->ReplaceLine(mla, lpResult, nResult);
					delete [] lpResult;
				}
			}
			if(ec != errOK || mla == mlb) break;
			mlb = mla;
			mla++;
		}
		ClearOpRegion();
		if(GetShowTabs())
		{
			multiLine = true;
			needvu = true;
		}
		MoveAbs(svline, svcol);
		break;

	case TabsAsSpaces:

		nParm = m_tabsasspaces;
		if(PopParm(_T("Tabs As Spaces"), ptNumber, nParm) != errOK)
			break;
		m_tabsasspaces = nParm != 0;
		UpdateStatus();
		break;

	case CaseOn:

		SetCaseSensi(true);
		break;

	case CaseOff:

		SetCaseSensi(false);
		break;

	case ToggleCase:

		SetCaseSensi(! GetCaseSensi());
		break;

	case ToggleTrimOnWrite:

		SetTrimOnWrite(! GetTrimOnWrite());
		break;

	case EnableRefresh:

		m_suppressrefresh = false;
		needvu = multiLine = true;
		break;

	case DisableRefresh:

		m_suppressrefresh = true;
		break;

	case SetDirty:

		m_buffer->SetModified(true);
		break;

	case ClearDirty:

		m_buffer->SetModified(false);
		break;

	case ViewHex:

		// create a new view on the buffer
		pView = new BviewHex(GetBuffer(), m_editor, m_editor->GetEditPanel());
		if(pView)
		{
			pView->DepersistParms();

			// add to views list to insure buffer isnt deleted on view removal
			m_editor->AddView(pView);
			m_editor->SetCurrentView(pView, true);
		}
		break;

	case BindKey:
	case ShowKeyBinds:

		break;

	case BedBind:
		ec = m_keyBindings->SetBindings(kbNative);
		break;

	case EmacsBind:
		ec = m_keyBindings->SetBindings(kbEmacs);
		break;

	case MSVCBind:
		ec = m_keyBindings->SetBindings(kbMSVC);
		break;

	case CWBind:
		ec = m_keyBindings->SetBindings(kbCW);
		break;

	case SetKeyBinds:

		nParm = m_keyBindings->GetBindings();
		if((ec = PopParm(_T("Key Bindings"), ptKeyBindings, nParm)) == errOK)
		{
			ec = m_keyBindings->SetBindings((aKeyBind)nParm);
		}
		break;

	case DoMenu:
	case Help:

		break;

	case HelpAbout:

		DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUT), m_hwnd, AboutBed);
		break;

	case ShowCommands:

		return HelpCommands();

	case ShowKeyBindings:

		return HelpKeyBinds();

	/*
	 * Commands
	 */
	case DoCommand:

		nParm = 0;
		ec = PopParm(_T("Execute Command"), ptString, pParm, nParm);
		if(ec != errOK || nParm <= 0) break;

		// lookup command
		command = CommandFromName(pParm);
		if(command != UnknownCommand)
			return Dispatch(command);

		// command not builtin, check scripts if configured
		if(m_tcl && m_runTCL)
		{
			LPCTSTR pParmData;

			if(m_tcl->IsTCLcommand(pParm, pParmData))
			{
				ec = m_tcl->Interpret(pParmData, true, false);
				break;
			}
		}
		_sntprintf(status, 256, _T("Unknown Command: " _Pfs_ ""), pParm);
		m_editor->SetStatus(status);
		ec = errFAILURE;
		break;

	case ReadCommandFile:

		break;

	/*
	 * Tags
	 */
	case SetTagFile:
	case FindTag:

		break;

	/*
	 * Projects
	 */
	case NewProject:

		ec = m_editor->NewProject();
		break;

	case OpenProject:

		if((ec = PopParm(_T("Open Build Project"), ptOpenExistingFilename, pParm, nParm)) == errOK)
		{
			LINETERMINATION lineterm = ltLF;
			TEXTENCODING	textEncoding;
			BufType			bufType = btAny;

			ec = PopParm(_T("Text Encoding"), ptTextEncoding, (int&)textEncoding);
			ec = PopParm(_T("Line Termination (0=LF, 1=CRLF)"), ptTextLineTerm, (int&)lineterm);
			ec = PopParm(_T("Open As (0=Auto, 1=Text, 2=Binary"), ptBufferType, (int&)bufType);

			ClearLineInfo(liIsInError | liIsBreakpoint | liIsSearchTarget);
			ec = m_editor->OpenProject(pParm, true);
		}
		break;

	case CloseProject:

		ClearLineInfo(liIsInError | liIsBreakpoint | liIsSearchTarget);
		ec = m_editor->CloseProject();
		break;

	/*
	 * Programs
	 */
	case ::Indent:

		ec = CheckReadOnly();
		if(ec != errOK) break;

		SetOpRegionFromSelection();
		if(m_regional)
			SetHandMarked(false);
		if(m_olb <= 0)
		{
			mlb = m_curline;
		}
		else
		{
			if(m_ola < m_olb)
				mlb = (m_ocb == 1) ? (m_olb - 1) : m_olb;
			else if(m_ola > m_olb)
				mlb = (m_oca == 1) ? (m_ola - 1) : m_ola;
			else
				mlb = m_ola;
		}
		mla = m_curline;
		mca = m_curcol;
		if(m_curline != mlb || m_regional)
		{
			multiLine = true;
			prevline = min(mla, mlb);
		}
		needvu = true;
		while(ec == errOK && mla <= mlb)
		{
			ec	= Indent(0, mla, mca);
			if(mla == mlb && ! m_regional)
			{
				MoveAbs(mla, mca);
				break;
			}
			mca = 1;
			mla++;
		}
		ClearOpRegion();
		break;

	case IndentLine:

		ec = CheckReadOnly();
		if(ec != errOK) break;

		ec = Dispatch(::Indent);
		if(m_qla > 0 && m_qlb > 0)
			MoveAbs(m_qlb, m_qcb);
		else
			MoveAbs(m_curline + 1, 1);
		break;

	case IndentMore:

		ec = CheckReadOnly();
		if(ec != errOK) break;

		SetOpRegionFromSelection();
		if(m_regional)
			SetHandMarked(false);
		if(m_olb <= 0)
		{
			mlb = m_curline;
		}
		else
		{
			if(m_ola < m_olb)
				mlb = (m_ocb == 1) ? (m_olb - 1) : m_olb;
			else if(m_ola > m_olb)
				mlb = (m_oca == 1) ? (m_ola - 1) : m_ola;
			else
				mlb = m_ola;
		}
		if(m_ola != m_olb)
			multiLine = true;
		prevline = min(m_ola, m_olb);
		needvu = true;
		{
			TCHAR	space[64];
			int		nSpace;
			int		indent;

			for(nSpace = 0, indent = 0; nSpace < 63 && indent < m_indentspace;)
			{
				if(! m_tabsasspaces && m_tabspace <= (m_indentspace - indent))
				{
					space[nSpace++] = _T('\t');
					indent += m_tabspace;
				}
				else
				{
					space[nSpace++] = _T(' ');
					indent++;
				}
			}
			space[nSpace] = _T('\0');
			mla = m_curline;

			while(ec == errOK && mla <= mlb)
			{
				mca = 1;
				ec = m_buffer->Insert(mla, mca, space, nSpace);
				mla++;
			}
			ClearOpRegion();
		}
		break;

	case IndentLess:

		ec = CheckReadOnly();
		if(ec != errOK) break;

		SetOpRegionFromSelection();
		if(m_regional)
			SetHandMarked(false);
		if(m_olb <= 0)
		{
			mlb = m_curline;
		}
		else
		{
			if(m_ola < m_olb)
				mlb = (m_ocb == 1) ? (m_olb - 1) : m_olb;
			else if(m_ola > m_olb)
				mlb = (m_oca == 1) ? (m_ola - 1) : m_ola;
			else
				mlb = m_ola;
		}
		if(m_olb != m_ola)
			multiLine = true;
		prevline = min(m_ola, m_olb);
		needvu = true;

		mla = m_curline;

		while(ec == errOK && mla <= mlb)
		{
			ec = m_buffer->GetLineText(mla, lpText, nText);
			if(ec != errOK)
				break;
			mca = 1;
			if(nText > 0 && (lpText[0] == _T('\t') || lpText[0] == _T(' ')))
			{
				int ndel = nText > 0 ? 1 : 0;

				if(m_tabsasspaces)
				{
					if(nText > 0)
					{
						int spacecol = mca;

						// count spaces from here forward to next tab stop to the right
						//
						while(spacecol <= nText && lpText[spacecol-1] == ' ' && (spacecol % m_tabspace))
						{
							ndel++;
							spacecol++;
						}
						// if the search forward stopped right before the next tab stop then
						// delete all the spaces, else just the 1
						//
						if(lpText[spacecol-1] != ' ')
						{
							ndel = 1;
						}
					}
				}
				ec = m_buffer->Delete(mla, mca, ndel);
			}
			mla++;
		}
		ClearOpRegion();
		break;

	case IndentLevel:

		break;

	case IndentSpace:

		nParm = m_indentspace;
		if(PopParm(_T("Indent Space"), ptNumber, nParm) != errOK)
			break;
		if(nParm > 1 && nParm < MAX_TAB_SPACE)
		{
			m_indentspace	 = nParm;
		}
		else
		{
			m_editor->SetStatus(_T("Bad Indent Space"));
		}
		break;

	case ::Dismember:

		ec = Dismember();
		break;

	case ::Match:

		if((ec = PopParm(_T("Match Brace"), ptString, pParm, nParm)) != errOK)
		{
			break;
		}
		Match(pParm);
		needvu = true;
		break;

	case Build:

		{
			BbuildInfo* pBuild;

			pBuild    = (BbuildInfo*)m_editor->GetPane(_T("Info"), _T("Build"));
			if(pBuild)
			{
				ClearLineInfo(liIsInError);

				ec = Dispatch(SaveAll);
				if(ec == errOK)
				{
					pBuild->Startup();
				}
			}
			else
			{
				MessageBox(NULL, _T("Build not configured"), m_editor->GetVersion(), MB_OK);
				ec = errFAILURE;
			}
		}
		break;

	case Execute:
	case Target:
	case CancelBuild:
	case NextError:

		break;

	case CheckIn:
	case CheckOut:
	case Revert:

		if(! m_editor->GetSCCS())
		{
			ec = m_editor->SetupSCCS(false);
			if(ec != errOK) break;
		}
		if(m_buffer->GetModified())
		{
			int rc;

			_sntprintf(
						status,
						MAX_PATH*2,
						(command == CheckIn ?
							_T("SCCS: Include current modifications to " _Pfs_ " ?") :
							_T("SCCS: Save current modifications to " _Pfs_ " in another file ?")
						),
						m_buffer->GetShortName()
					  );
			rc = MessageBox(NULL, status, m_editor->GetVersion(), MB_YESNOCANCEL);
			if(rc == IDYES)
				ec = Dispatch(SaveAs);
			else if(rc == IDCANCEL)
				break;
		}
		if(ec == errOK)
		{
			if(m_editor->GetSCCS())
			{
				switch(command)
				{
				case CheckIn:
					ec = m_editor->GetSCCS()->CheckIn(m_buffer->GetName(), GetEditWindow());
					_sntprintf(status, STATUS_LEN, _T("" _Pfs_ " Checked In"), m_buffer->GetShortName());
					break;
				case CheckOut:
					ec = m_editor->GetSCCS()->CheckOut(m_buffer->GetName(), GetEditWindow());
					_sntprintf(status, STATUS_LEN, _T("" _Pfs_ " Checked Out"), m_buffer->GetShortName());
					needvu = multiLine = true;
					break;
				case Revert:
					ec = m_editor->GetSCCS()->Revert(m_buffer->GetName(), GetEditWindow());
					_sntprintf(status, STATUS_LEN, _T("" _Pfs_ " Reverted"), m_buffer->GetShortName());
					needvu = multiLine = true;
					break;
				default:
					break;
				}
				UpdateStatus();
				if(ec == errOK)
				{
					ec = m_buffer->ReadAgain();
					m_editor->SetStatus(status);
				}
				else
				{
					m_editor->SetStatus(_T("SCCS Failed"));
				}
			}
			else
			{
				m_editor->SetStatus(_T("SCCS Not Integrated"));
				ec = errFAILURE;
			}
		}
		break;

	case Debug:

		if(! m_editor->GetDebugger())
		{
			ClearLineInfo(liIsInError | liIsExecCurrent);
			ec = m_editor->SetupDebug(false);
		}
		break;

	case StopDebug:

		if(m_editor->GetDebugger())
		{
			ClearLineInfo(liIsExecCurrent);
			ec = m_editor->Debug();
		}
		break;

	case Start:
	case Stop:
	case StepIn:
	case StepOut:
	case Step:
	case SetBreak:
	case ClearBreak:
	case ToggleBreak:
	case RunToCursor:
	case ReadVar:
	case WriteVar:
	case SetWatchVar:
	case ClearWatchVar:

		if(! m_editor->GetDebugger())
		{
			if(command == Start)
			{
				ec = Dispatch(Debug);
			}
			if(! m_editor->GetDebugger())
			{
				MessageBox(NULL, _T("Not Currently Debugging"), m_editor->GetVersion(), MB_OK);
				ec = errFAILURE;
				break;
			}
		}
		ClearLineInfo(liIsInError | liIsExecCurrent);
		switch(command)
		{
		case Start:
			ec = m_editor->GetDebugger()->Run(rcGo);
			break;
		case Stop:
			ec = m_editor->GetDebugger()->Run(rcStop);
			break;
		case StepIn:
			ec = m_editor->GetDebugger()->Run(rcStepIn);
			break;
		case StepOut:
			ec = m_editor->GetDebugger()->Run(rcStepOut);
			break;
		case Step:
			ec = m_editor->GetDebugger()->Run(rcStep);
			break;
		case SetBreak:
			if(m_editor->GetDebugger())
				ec = m_editor->GetDebugger()->SetBreak(m_buffer->GetName(),GetCurLine(), mla);
			else
				ec = errOK;
			if(ec == errOK)
				ec = m_buffer->SetLineIsInfo(GetCurLine(), liIsBreakpoint);
			needvu = true;
			break;
		case ClearBreak:
			if(m_editor->GetDebugger())
				ec = m_editor->GetDebugger()->ClearBreak(m_buffer->GetName(), GetCurLine());
			else
				ec = errOK;
			if(ec == errOK)
			{
				mlb = m_buffer->GetLineIsInfo(GetCurLine());
				ec = m_buffer->SetLineIsInfo(GetCurLine(), mlb & ~liIsBreakpoint);
			}
			needvu = true;
			break;
		case ToggleBreak:
			mlb = m_buffer->GetLineIsInfo(GetCurLine());
			if(mlb & liIsBreakpoint)
				ec = Dispatch(ClearBreak);
			else
				ec = Dispatch(SetBreak);
			break;
		case RunToCursor:
			ec = Dispatch(SetBreak);
			break;
		case ReadVar:
		case WriteVar:
		case SetWatchVar:
		case ClearWatchVar:
			break;
		default:
			break;
		}
		break;

	/*
	 * Program
	 */
	case ShellFinished:
	case ExecShell:

		break;

	case ExitBed:

		if((! m_editor) || m_editor->Close())
			PostQuitMessage(0);
		break;

	/*
	 * Inquire
	 */
	case LineNum:
	case ColumnNum:
	case Found:
	case BufferName:
	case BufferType:
	case BufferTypeAsString:
	case IsModified:
	case IsUntitled:
	case IsNew:
	case IsEmpty:
	case LineLen:
	case LineContent:
	case CurrentChar:
	case CurrentCharAsString:
	case SelectionStartLine:
	case SelectionStartColumn:
	case SelectionEndLine:
	case SelectionEndColumn:

		break;

	/*
	 * Dialog
	 */
	case InquireNumber:
	case InquireString:
	case InquireFilename:
	case InquireYesNo:
	case InquireYesNoCancel:
	case InquireYesNoSkipAll:
	case Message:
	case SetStatus:

		break;

	/*
	 * Set Line Type
     */
	case SetLineType:

		ec = Dispatch(MoveToLine);
		if(ec == errOK)
		{
			mlb = 0;
			if(PopParm(_T("Type for Line"), ptNumber, mlb) != errOK)
				break;
			ec = m_buffer->SetLineIsInfo(m_buffer->GetCurLine(), mlb);
			needvu = true;
		}
		break;

	/*
	 * Misc
	 */
	case SetFrgColor:
	case SetBkgColor:

		nParm = 0;
		ec = PopParm(_T("Enter Keyword Level for Set Frg Color"), ptNumber, nParm);
		if(ec == errOK)
		{
			TCHAR	szSpec[128];
			int		kw = nParm;

			pParm = GetKeywordColor((BkwType)kw, szSpec, 128);
			ec = PopParm(_T("Color"), ptColor, pParm, nParm);
			if(ec == errOK)
			{
				if(kw >= 0)
				{
					ec = SetKeywordColor((BkwType)kw, pParm);
				}
				else
				{
					for(kw = 0; kw < MAX_VIEW_FONTS; kw++)
						ec = SetKeywordColor((BkwType)kw, pParm);
				}
				if(command == SetBkgColor)
				{
					COLORREF bkg;
					BYTE     alpha;

				    bkg = GetBackground(alpha);
					SetLayeredWindowAttributes(GetEditWindow(), 0, alpha, LWA_ALPHA);
#ifdef Windows
					// Windows 7 - only top level windows is xparent
					{
						HWND hwnd;

						for(hwnd = GetParent(GetEditWindow()); hwnd; hwnd = GetParent(hwnd))
						SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
					}
#endif
					m_editor->UpdateInfoPanes(this);
				}
				PersistSettings();
			}
			needvu = multiLine = true;
			ClearMetricsCache();
		}
		break;

	case SetFont:

		nParm = 0;
		ec = PopParm(_T("Enter Keyword Level for Set Font"), ptNumber, nParm);
		if(ec == errOK)
		{
			TCHAR	szSpec[128];
			int		kw = nParm;

			pParm = GetKeywordFont((BkwType)kw, szSpec, 128);
			ec = PopParm(_T("Font"), ptFontSpec, pParm, nParm);
			if(ec == errOK)
			{
				if(kw >= 0)
					ec = SetKeywordFont((BkwType)kw, pParm);
				else
				{
					for(kw = 0; kw < MAX_VIEW_FONTS; kw++)
					{
						if(kw == kwSpecial && 0)
						{
							TCHAR xParm[128];
							int        len;

							// force special to be bold regardless of user ?

							_tcscpy(xParm, pParm);
							len = _tcslen(xParm);
							if(len > 4)
							{
								xParm[len-5] = _T('1');
							}
							ec = SetKeywordFont((BkwType)kw, xParm);
						}
						else
						{
							ec = SetKeywordFont((BkwType)kw, pParm);
						}
					}
				}
			}
			needvu = multiLine = true;
			ClearMetricsCache();
		}
		break;

	case SetDefaultFonts:

		{
			int kw;
			LPFONTSPEC pf;
			LPCTSTR lpKeywordName;
			TCHAR	szParmName[64];

			Bpersist* pPersist = m_editor->GetPersist();

			for(kw = 0; pPersist && kw < MAX_VIEW_FONTS; kw++)
			{
				pf = &m_view_fspec[kw];

				memcpy(&m_def_fonts[kw], pf, sizeof(FONTSPEC));

				lpKeywordName = view_keyword_names[kw];

				_sntprintf(szParmName, 64, _T("Keyword/" _Pfs_ "/Display/Font/Face"), lpKeywordName);
				ec = pPersist->SetNvStr(szParmName, pf->face);

				_sntprintf(szParmName, 64, _T("Keyword/" _Pfs_ "/Display/Font/Height"), lpKeywordName);
				ec = pPersist->SetNvInt(szParmName, pf->height);

				_sntprintf(szParmName, 64, _T("Keyword/" _Pfs_ "/Display/Font/Bold"), lpKeywordName);
				ec = pPersist->SetNvBool(szParmName, pf->bold);

				_sntprintf(szParmName, 64, _T("Keyword/" _Pfs_ "/Display/Font/Italic"), lpKeywordName);
				ec = pPersist->SetNvBool(szParmName, pf->italic);

				_sntprintf(szParmName, 64, _T("Keyword/" _Pfs_ "/Display/Font/AntiAlias"), lpKeywordName);
				ec = pPersist->SetNvBool(szParmName, pf->antialias);
			}
		}
		break;

	/*
	* Printing
	*/
	case PageSetup:

		if(! m_printer)
			m_printer = new Bprinter(m_editor);
		ec = m_printer->SetupPage();
		break;

	case PageRange:
	case SetPrinterName:
		break;

	case PrintScreen:

		ec = PrintView(m_topline, m_topline + GetViewLines(), prWindow);
		break;

	case PrintSelection:

		if(! GetMarkedRegion(mla, mca, mlb, mcb))
		{
			MessageBox(NULL, _T("No Selection for Print"), m_editor->GetVersion(), MB_OK);
			ec = errFAILURE;
		}
		else
		{
			ec = PrintView(min(mla, mlb), max(mla, mlb), prSelection);
		}
		break;

	case PrintDocument:

		PrintView(1, m_buffer->GetLineCount(mca), prDocument);
		break;

	default:

		break;
	}

	// Add to Macro maybe
	//
	if(m_recording && ! m_playing && ! m_suppressmacro && command != ::DefineMacro)
	{
		AddToMacro(new MACROREC(command));
	}

	// udpate the view
	//
	FreeOldParms();

	if(needvu && ! m_suppressrefresh && command != UnknownCommand)
	{
		multiLine |= (prevline != m_curline) 	||
					  m_buffer->ChangedLines()	||
					  m_buffer->GetRaw();

		if(m_curline != prevline || m_curcol != prevcol)
			UpdatePos(m_curline, m_curcol);

		UpdateAllViews(
						min(prevline, m_curline),
						multiLine ? 1 : prevcol,
						multiLine ? 32767 : max(prevline, m_curline),
						multiLine ? MAX_LINE_LEN : m_curcol,
						m_buffer
				);
	}
	if(m_buffer->GetModified() != modified)
	{
		m_editor->UpdateInfoPanes(this);
	}
	if(vertical || multiLine)
	{
		BfuncInfo* pInfo;

		if((pInfo = (BfuncInfo*)m_editor->GetPane(_T("Functions"), _T("Functions"))) != NULL)
		{
			pInfo->SetToLine(m_curline);
		}
	}
	return ec;
}
