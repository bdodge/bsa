
#include "bedx.h"

#define TERM_UPDATE_CHUNK 20000

//#define LOG_VT 1

//**************************************************************************
BviewTerminal::BviewTerminal(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel)
		:
		Bview(pBuf, pEditor, pPanel),
		m_head(0),
		m_tail(0),
		m_cnt(0),
		m_tokcnt(0),
		m_tokbase(0),
		m_emulation(emulXTERM),
		//m_emulation(emulVT100),
		m_view_scheme(0),
		m_closeonexit(true),
		m_noexitprompt(true),
		m_insertcount(0),
		m_log(NULL),
		m_totalcnt(0),
		m_logthresholdsize(20000),
		m_logthresholdtime(5)
{
	if(m_buffer && m_buffer->GetName() && ! *m_buffer->GetName())
		m_buffer->SetName(_T("terminal"));

#ifdef LOG_VT
	m_log = new Blog(5, logtoCONSOLE, NULL);
	m_log->SetLevel(5);
#endif
	BUtil::GetHomePath(m_logdir, MAX_PATH);

	ResetSettings();

	// disable window scrolling horizontally if
	// emulation is not off, since apps may do it themeselves
	//
	m_colscrollok = false;
}

//**************************************************************************
BviewTerminal::~BviewTerminal()
{
	if(m_running)
		Stop();
}

//**************************************************************************
void BviewTerminal::ResetSettings()
{
	m_ansiLevel = emulVT220;
	m_vtState = vtpsINIT;
	m_vtrows = 24;
	m_vtcols = 80;
	m_vtmode = vtmdANSI;
	m_frameline = 1;
	m_charset = 0;
	m_vtrow = 1;
	m_vtcol = 1;
	m_vvtrow = 1;
	m_vvtcol = 1;
	m_svvtrow = 1;
	m_svvtcol = 1;
	m_minvtrow = 1;
	m_maxvtrow = 1;
	m_prevvtcol = 1;
	m_scrlt = 1;
	m_scrlb = 0;
	m_curfrg = 0;
	m_curbkg = 7;
	m_deffrg = true;
	m_defbkg = true;
	m_attribs = vtdaNONE;
	m_ctrlBits = 8;

	m_charSets[0] = vtcsDefault;
	m_charSets[1] = vtcsDefault;
	m_charSets[2] = vtcsDefault;
	m_charSets[3] = vtcsDefault;
}

//**************************************************************************
void BviewTerminal::Activate()
{
	Bview::Activate();

	if(! m_running)
	{
		TCHAR	pparm[MAX_PATH+256];
		LPTSTR  px;
		int     ppl, emul;
		bool	crnl, echo, wrap, nobell, log;
		Bpersist* pp;

		BUtil::GetHomePath(m_logdir, MAX_PATH);

		// set noundo in buffer
		m_buffer->SetHasUndo(false);

		// set forget about mods in buffer
		m_buffer->SetCareForMods(false);

		if((pp = GetEditor()->GetPersist()) != NULL)
		{
			// get emulation settings
			ppl = _sntprintf(pparm, 128, _T("" _Pfs_ "/Emulation/Settings/"), GetTypeName());
			px = pparm + ppl;
			_tcscpy(px, _T("Echo"));
			pp->GetNvBool(pparm, echo, (m_vtmode & vtmdECHO) != 0);
			_tcscpy(px, _T("LineWrap"));
			pp->GetNvBool(pparm,wrap, (m_vtmode & vtmdWRAP) != 0);
			_tcscpy(px, _T("Emulation"));
			pp->GetNvInt(pparm, emul, m_emulation);
			m_emulation = (BvtEmulation)emul;
			_tcscpy(px, _T("TreatCRasNL"));
			pp->GetNvBool(pparm, crnl, (m_vtmode & vtmdLFNL) != 0);
			_tcscpy(px, _T("NoBell"));
			pp->GetNvBool(pparm, nobell, (m_vtmode & vtmdBEEP) != 0);

			_tcscpy(px, _T("LogToFile"));
			pp->GetNvBool(pparm, log, (m_vtmode & vtmdLOG) != 0);

			_tcscpy(px, _T("Log/Directory"));
			pp->GetNvStr(pparm, m_logdir, MAX_PATH, m_logdir);

			_tcscpy(px, _T("Log/SizeThresholdBytes"));
			pp->GetNvInt(pparm, m_logthresholdsize, m_logthresholdsize);
			_tcscpy(px, _T("Log/TimeThresholdSeconds"));
			pp->GetNvInt(pparm, m_logthresholdtime, m_logthresholdtime);

			ClearModeBit(0xFFFF);
			if(crnl)
				SetModeBit(vtmdLFNL);
			if(wrap)
				SetModeBit(vtmdWRAP);
			if(nobell)
				SetModeBit(vtmdBEEP);
			if(echo)
				SetModeBit(vtmdECHO);
			if(log)
				SetModeBit(vtmdLOG);

			if(log)
			{
				char    logn[MAX_PATH*2];
				char	typn[MAX_PATH];
				time_t  now;

				time(&now);
				TCharToChar(logn, m_logdir);
				TCharToChar(typn, GetTypeName());
				_snprintf(logn+strlen(logn), sizeof(logn), "%s.%s.log", ctime(&now), typn);
				CharToTChar(m_logName, logn);
				for(px = m_logName; *px; px++)
					if(*px == _T(' ') || *px == _T('\n') || *px == _T('\r') || (*px == _T(':') && (px > (m_logName+1))))
						*px = _T('_');
			}
		}
		// setup a timer to read shell data and handle
		// quiescent detection
		//
		SetWindowLong(m_hwnd, GWL_USERDATA, (LONG)this);
		m_idTimer = SetTimer(m_hwnd, 0xFACE, 30, OnTimer);

		// start the shell reader thread
		Start(RunStub, this);
	}
}

//**************************************************************************
ERRCODE BviewTerminal::WriteLog()
{
	ERRCODE ec;

	if(m_buffer)
	{
		if(m_emulation == emulNONE)
		{
			BfileStream fs;
			LPCTSTR	pText;
			LPBYTE	pPlain;
			int		nText, nPlain;
			int		maxline, maxlen;
			int		line, i;

			ec = fs.Open(m_logName, _T("w"));
			if(ec == errOK)
			{
				maxline = m_buffer->GetLineCount(maxlen);
				if(maxline > 0 && maxlen > 0)
				{
					pPlain = new BYTE[maxlen + 2];

					for(line = 1; line < maxline && ec == errOK; line++)
					{
						ec = m_buffer->GetLineText(line, pText, nText);
						if(ec == errOK)
						{
							for(i = nPlain = 0; i < nText; i += 3)
							{
								pPlain[nPlain++] = (BYTE)pText[i];
							}
							ec = fs.Write(pPlain, nPlain);
						}
					}
					delete [] pPlain;
				}
				fs.Close();
			}
		}
		else
		{
			ec = m_buffer->Write(m_logName);
			if(ec == errOK)
			{
				ec = m_buffer->Read(m_logName);
			}
		}
		return ec;
	}
	m_totalcnt = 0;
	return errFAILURE;
}

//**************************************************************************
ERRCODE BviewTerminal::Dispatch(EditCommand command)
{
	int mla, mca, mlb, mcb;
	ERRCODE ec;

	switch(command)
	{
	case Paste:
		if(OpenClipboard(m_hwnd))
		{
			HANDLE hMem = GetClipboardData(CF_TEXT);
			LPSTR  pcp;

			if(hMem)
			{
				if((pcp = (LPSTR)GlobalLock(hMem)) != NULL)
				{
					int cblen = strlen(pcp);

					if(cblen > 0)
						cblen = SendData((LPBYTE)pcp, cblen);
				}
			}
			CloseClipboard();
			return errOK;
		}
		return errFAILURE;

	case Copy:

		ec = errOK;

		if(GetMarkedRegion(mla, mca, mlb, mcb))
		{
			LPTSTR	lpCopied;
			int		nCopy;
			int		temp, dex;
			bool    sameline = false;

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
				sameline = true;
				if(mca > mcb)
				{
					temp = mcb;
					mcb  = mca;
					mca  = temp;
				}
			}
			mca = mca * 3 - 2;
			mcb = mcb * 3 - 2;
			nCopy = CountRegionChars(m_buffer, mla, mca, mlb, mcb);
			ClearMark();

			if(nCopy > 0)
			{
				ec = m_buffer->Copy(mla, mca, nCopy, lpCopied);

				if(nCopy > 0)
				{
					OpenClipboard(m_hwnd);

					// set copied to the cut buffer (alloc a bunch more than needed since
					// newlines dont have attributes and better to not count lines)
					//
					int alloc_len = ((nCopy / 2) + 1) * sizeof(char);
					HANDLE hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, alloc_len);
					LPSTR  pcp = (char*)GlobalLock(hMem);

					EmptyClipboard();

					for(temp = dex = 0; dex < nCopy; temp++)
					{
						pcp[temp] = (char)lpCopied[dex];
						if(pcp[temp] == '\n') {
							if(sameline && (dex == nCopy - 1))
								pcp[temp] = '\0';
							dex++;
						}
						else
							dex += 3;
					}

					while (temp < alloc_len)
					{
						pcp[temp++] = '\0';
					}

					GlobalUnlock(hMem);
					SetClipboardData(CF_TEXT, hMem);
					CloseClipboard();
				}
				if(lpCopied)
					delete [] lpCopied;
			}
		}
		return ec;

	case EmptyBuffer:
		m_vtrow = m_vtcol = 1;
		m_frameline = 1;
		ec = Bview::Dispatch(command);
		return ec;

	default:
		return Bview::Dispatch(command);
	}
}

//**************************************************************************
void BviewTerminal::SpecificMouseMenu(HMENU hMenu)
{
	AppendMenu(hMenu, 0, ID_TOOLS_CLEAR, _T("C&lear"));
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
}

//**************************************************************************
void BviewTerminal::MouseSpecial(int x, int y)
{
	ERRCODE ec;

	if(m_qla > 0 && m_qlb > 0)
	{
		ec = Dispatch(Copy);
		if(ec == errOK)
			ec = Dispatch(Paste);
	}
}

// STATIC
//**************************************************************************
ERRCODE BviewTerminal::RunStub(LPVOID thisptr)
{
	return ((BviewTerminal*)thisptr)->ShellThread();
}

//**************************************************************************
ERRCODE BviewTerminal::ShellThread()
{
	ERRCODE ec = errOK;
	m_head = m_tail = 0;
	m_size = sizeof(m_iobuf) - 1;
	m_cnt = 0;

	while(m_running)
	{
		Bthread::Sleep(150);
	}
	return ec;
}

//**************************************************************************
void BviewTerminal::FitToPanel(void)
{
	BappPane::FitToPanel();
	m_vtrows = GetViewLines();
	m_vtcols = GetViewCols() - 1;
	if(m_vtrows < 1) m_vtrows = 1;
	if(m_vtcols < 1) m_vtcols = 1;
}

//**************************************************************************
int BviewTerminal::VTformatToken(LPCTSTR intok, int nintok, LPTSTR tok, int ntok)
{
	int i, j;

	for(i = j = 0; i < nintok && j < ntok - 3; i++)
	{
		tok[j++] = intok[i];
		if(intok[i] == '\n')
		{
			if(i == nintok - 1)
				break;
		}
		else
		{
			tok[j++] = (TCHAR)(((unsigned char)m_curfrg << 4) | (unsigned char)m_curbkg);
			tok[j++] = (TCHAR)m_attribs;
		}
	}
	tok[j] = 0;
	return j;
}

//**************************************************************************
static void VTdumpToken(Blog* plog, LPCTSTR tok, int ntok)
{
#ifdef LOG_VTX
	int i;
	unsigned char c, a, b;

	for(i = 0; i < ntok; i+= 3)
	{
		if(tok[i] == '\n')
		{
			break;
		}
		b = tok[i];
		c = tok[i+1];
		a = tok[i+2];

		_tprintf(_T("%3d (%02X:%02X:%02X)%02X %c\n"), i + 1,
			c >> 4, c & 0xf, a, b, b);
	}
#endif
}

//**************************************************************************
COLORREF BviewTerminal::VTrgbColor(char color, int isbold)
{
	COLORREF cr;
	int isrev = 0;

	switch(m_view_scheme)
	{
	case 0: // "tango"
		if(! isbold)
		{
			switch(color)
			{
			default:
			case 0: cr = RGB(46, 52, 54); break;
			case 1: cr = RGB(204, 0, 0); break;
			case 2: cr = RGB(78, 154, 6); break;
			case 3: cr = RGB(196, 160, 0); break;
			case 4: cr = RGB(52, 101, 164); break;
			case 5: cr = RGB(117, 80, 123); break;
			case 6: cr = RGB(6, 152, 154); break;
			case 7: cr = RGB(211, 215, 207); break;
			}
		}
		else
		{
			switch(color)
			{
			case 0: cr = RGB(85, 87, 83); break;
			case 1: cr = RGB(240, 41, 41); break;
			case 2: cr = RGB(88, 186, 52); break;
			case 3: cr = RGB(252, 233, 79); break;
			case 4: cr = RGB(114, 159, 207); break;
			case 5: cr = RGB(173, 127, 168); break;
			case 6: cr = RGB(52, 226, 226); break;
			case 7: cr = RGB(238, 238, 236); break;
			}
		}
		break;
	case 1: // linux console
		if(! isbold)
		{
			switch(color)
			{
			default:
			case 0: cr = m_view_colors[kwPlain]; break;
			case 1: cr = RGB(170, 0, 0); break;
			case 2: cr = RGB(0, 170, 0); break;
			case 3: cr = RGB(170, 170, 0); break;
			case 4: cr = RGB(0, 0, 170); break;
			case 5: cr = RGB(170, 0, 170); break;
			case 6: cr = RGB(0, 170, 170); break;
			case 7: cr = m_view_colors[kwBackground]; break;
			}
		}
		else
		{
			switch(color)
			{
			default:
			case 0: cr = m_view_colors[kwPlain]; break;
			case 1: cr = RGB(255, 85, 85); break;
			case 2: cr = RGB(85, 255, 85); break;
			case 3: cr = RGB(255, 255, 85); break;
			case 4: cr = RGB(85, 85, 255); break;
			case 5: cr = RGB(255, 85, 255); break;
			case 6: cr = RGB(85, 255, 255); break;
			case 7: cr = m_view_colors[kwBackground]; break;
			}
		}
		break;
	case 2:	// xterm
	default:
		switch(color)
		{
		default:
		case 0: cr = m_view_colors[kwPlain]; break;
		case 1: cr = RGB(200, 0, 0); break;
		case 2: cr = RGB(0, 200, 0); break;
		case 3: cr = RGB(200, 200, 0); break;
		case 4: cr = RGB(0, 0, 200); break;
		case 5: cr = RGB(200, 0, 200); break;
		case 6: cr = RGB(0, 200, 200); break;
		case 7: cr = m_view_colors[kwBackground]; break;
		}
		break;
	}
	return cr;
}

//**************************************************************************
bool BviewTerminal::IsPlainText()
{
	if(
		! (m_vtmode & vtmdKEYPAD)
	&&	! (m_vtmode & vtmdCURKEY)
	)
	{
		return true;
	}
	return false;
}

//**************************************************************************
TokenRet BviewTerminal::GetToken(
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
	TokenRet tr = trOK;
	BkwType  kw = kwPlain;
	TCHAR	 ic, colors, attribs;
	HFONT	 nhf;

	bool  gotToken	 = false;
	bool  whitespace = false;
	int   inc		 = incol - 1;
	bool  reverse;

	COLORREF frg, bkg, nfrg, nbkg;

	if(! lpText || inc < 0) return trFAILURE;
	if(inc >= nText)		return trEOLLINE;

	lpToken = lpText + inc;
	nToken  = 0;
	lpToken = m_outtok;

	reverse = (m_vtmode & vtmdSCREEN) != 0;

	// each character has a color byte and an attribute byte
	// after it, except a newline.  tokens break at color/font
	// change boundaries, and at token break points
	//
	// if this is the start of a new line, set the color based
	// on the first bytes attributes,
	//
	while(! gotToken && (inc < nText) && nToken < sizeof(m_outtok)/sizeof(TCHAR))
	{
		ic		= lpText[inc];

		if(ic == '\n' || ic == '\r' || ! ic)
		{
			// end of the line
			//
			if(nToken)
			{
				gotToken = true;
				break;
			}
			return 	trEOLLINE;
		}
		// not end of line, so expect a color and attribute byte
		//
		if(inc > (nText - 2))
		{
			// complain bitterly about this
			//
			colors = attribs = 0;
		}
		else
		{
			colors  = lpText[inc+1];
			attribs	= lpText[inc+2];
		}

		// break color byte into frg and bkg indexes
		//
		frg = VTrgbColor(colors >> 4, (attribs & vtdaBOLD));
		bkg = VTrgbColor(colors & 0xF, 0/*(attribs & vtdaBOLD)*/);

		nfrg = frg;
		nbkg = bkg;

		nhf = m_view_fonts[kwPlain];

		// modify colors based on attributes
		//
		if(attribs & vtdaBOLD)
		{
			nhf = m_view_fonts[kwSpecial];
		}
		if(attribs & vtdaREVERSE)
		{
			nfrg = bkg;
			nbkg = frg;
		}
		else /* not reverse video */
		{
			// if plain old text and def color, use view bkg, not vt bkg
			if(m_defbkg && ((colors & 0xf) == 7))
			{
				nbkg = m_view_colors[kwBackground];
			}
			if(m_deffrg && ((colors & 0xf0) == 0))
			{
				nfrg = m_view_colors[kwPlain];
			}
		}
		if(attribs & vtdaUNDER)
		{
			;
		}
		if(attribs & vtdaBLINK)
		{
			;
		}
		// if new frg or bkg is not the same as
		// input, end the token here
		//
		if(nfrg != frgColor || nbkg != bkgColor || nhf != hFont)
		{
			if(nToken > 0)
			{
				gotToken = true;
				break;
			}
			frgColor = nfrg;
			bkgColor = nbkg;
			hFont 	 = nhf;
		}
		// now see what to do based on char code
		//
		switch(ic)
		{
		case _T(' '):

			if(nToken && ! whitespace)
			{
				gotToken = true;
				break;
			}
			whitespace = true;
			m_outtok[nToken++] = ic;
			inc+= 3;
			break;

		case _T('\t'):

			gotToken = true;
			if(nToken)	break;
			nToken++;
			inc+= 3;
			gotToken = true;
			break;

		case _T('\n'): case _T('\r'): case _T('\0'):

			if(nToken)
			{
				gotToken = true;
				break;
			}
			return 	trEOLLINE;

		default:

			if(whitespace)
			{
				gotToken = true;
			}
			else if(IsDelim(ic))
			{
				if(nToken)
				{
					gotToken = true;
					break;
				}
				else
				{
					gotToken = true;
					if(kw == kwPlain)
						kw = kwOperator;
					inc+= 3;
					m_outtok[nToken++] = ic;
				}
			}
			else
			{
				m_outtok[nToken++] = ic;
				inc+= 3;
			}
			break;
		}
	}
	incol   = inc + 1;
	outcol += nToken;
	return tr;
}

// STATIC
//**************************************************************************
vtParseState BviewTerminal::ParseVT100(
									char ic,
									vtParseState parstate,
									BvtEmulation emulation,
									BvtAction&   action,
									int loglevel
									)
{
	int i, attribs, offattribs;
	unsigned char frg, bkg;

#ifdef LOG_VT
	if(loglevel >= 5)
	{
#ifdef Windows
		TCHAR dbb[64];

		_sntprintf(dbb, 64, _T("vtc=%02X (%c)\n"), ic, ic >= '0' ? ic : ' ');
		OutputDebugString(dbb);
#else
		_tprintf(_T("vtc=%02X (%c)\n"), ic, ic >= '0' ? ic : ' ');
#endif
	}
#endif

	action.f_action = vtactUnknown;

	switch(parstate)
	{
	case vtpsINIT:
		action.f_parms[0] = action.f_parms[1] = action.f_parms[2] =
		action.f_parms[3] = action.f_parms[4] = action.f_parms[5] = 0;
		action.f_valparm[0] = action.f_valparm[1] = action.f_valparm[2] =
		action.f_valparm[3] = action.f_valparm[4] = action.f_valparm[5] = 0;
		action.f_parmnum = 0;

		switch(ic) {
		case 0:		// NUL
			action.f_action = vtactIgnore;
			break;
		case 5:		// ENQ
			break;
		case 7:		// BELL
			break;
		case 8:		// BS
			action.f_action = vtactBackSpace;
			action.f_parms[0] = 0; action.f_parms[1] = -1;
			break;
		case 9:		// HT
			action.f_action = vtactTab;
			break;
		case '\n':	// LF (10) (return)
		case 11:	// VT (vert tab) (interpreted as LF)
		case 12:	// FF (formfeed) (interpreted as LF)
			/* this is in here to hack newline mode, there is NO esc \n */
			action.f_action = vtactNewline;
			break;
		case '\r':	// CR (13)
			/* this is in here to hack CR which moves cursor back to SOL */
			action.f_action = vtactRelPos;
			action.f_parms[0] = 0; action.f_parms[1] = -MAX_LINE_LEN;
			break;
		case 14:	// SO G1 char set
			action.f_action = vtactGxSELECT;
			action.f_parms[0] = 1;
			break;
		case 15:	// SI G0 char set
			action.f_action = vtactGxSELECT;
			action.f_parms[0] = 0;
			break;
		case 17:	// XON
			break;
		case 19:	// XOFF
			break;
		case 24:	// CAN
		case 26:	// SUB (interpret as CAN) (cancel)
			break;
		case 27:	// ESC
			if(emulation != emulNONE)
				parstate = vtpsESCAPE;
			else
				action.f_action = vtactNone;
			break;
		case 127:	// DEL
			action.f_action = vtactIgnore;
			break;
		default:
  			action.f_action = vtactNone;
			break;
		}
		break;

	case vtpsESCAPE:
		switch(ic) {
		case '[':
			parstate = vtpsESBRA;
			break;
		case ']':
			if(emulation == emulXTERM)
				parstate = vtpsESARB;
			break;
		case '^':
			if(emulation == emulXTERM)
				parstate = vtpsESCARET;
			break;
		case '_':
			if(emulation == emulXTERM)
				parstate = vtpsESUND;
			break;
		case '(':
			parstate = vtpsESPAR;
			break;
		case ')':
			parstate = vtpsESRAP;
			break;
		case '*':
			if(emulation == emulXTERM)
				parstate = vtpsESSTAR;
			break;
		case '+':
			if(emulation == emulXTERM)
				parstate = vtpsESPLUS;
			break;
		case '#':
			parstate = vtpsPOUND;
			break;
		case ' ':
			if(emulation == emulXTERM)
				parstate = vtpsSPACE;
			break;
		case 'c':
			action.f_action = vtactReset;
			break;
		case 'D':
			action.f_action = vtactIndex;
			action.f_parms[0] = 1; action.f_parms[1] = 0;
			break;
		case 'E':
			action.f_action = vtactIndex;
			action.f_parms[0] = 0; action.f_parms[1] = 1;
			break;
		case 'M':
			action.f_action = vtactIndex;
			action.f_parms[0] = -1; action.f_parms[1] = 0;
			break;
		case 'N':
			if(emulation == emulXTERM)
			{
				action.f_action = vtactGxSELECT;
				action.f_parms[0] = 2;
			}
			else
				action.f_action = vtactIgnore;
			break;
		case 'O':
			if(emulation == emulXTERM)
			{
				action.f_action = vtactGxSELECT;
				action.f_parms[0] = 3;
			}
			else
				action.f_action = vtactIgnore;
			break;
		case 'H':
			action.f_action = vtactSetTab;
			break;
		case 'V':
			if(emulation == emulXTERM)
				action.f_action = vtactStartGuard;
			else
				action.f_action = vtactIgnore;
			break;
		case 'W':
			if(emulation == emulXTERM)
				action.f_action = vtactEndGuard;
			else
				action.f_action = vtactIgnore;
			break;
		case 'X':
			if(emulation == emulXTERM)
				action.f_action = vtactStartString;
			else
				action.f_action = vtactIgnore;
			break;
		case '\\':
			if(emulation == emulXTERM)
				action.f_action = vtactEndString;
			else
				action.f_action = vtactIgnore;
			break;
		case '1':
			action.f_action = vtactMode;
			action.f_parms[0] = vtmdGRAPH;
			action.f_parms[1] = 0;
			break;
		case '2':
			action.f_action = vtactMode;
			action.f_parms[0] = 0;
			action.f_parms[1] = vtmdGRAPH;
			break;
		case '7':
			action.f_action = vtactSavePos;
			break;
		case '8':
			action.f_action = vtactRestorePos;
			break;
		case '=':
			action.f_action = vtactMode;
			action.f_parms[0] = vtmdKEYPAD;
			action.f_parms[1] = 0;
			break;
		case '>':
			action.f_action = vtactMode;
			action.f_parms[0] = 0;
			action.f_parms[1] = vtmdKEYPAD;
			break;
		case '<':
			action.f_action = vtactMode;
			action.f_parms[0] = vtmdANSI;
			action.f_parms[1] = 0;
			break;
		case 'n':
			if(emulation == emulXTERM)
			{
				action.f_action = vtactGxSELECT;
				action.f_parms[0] = 2;
			}
			else
				action.f_action = vtactIgnore;
			break;
		case 'o':
			if(emulation == emulXTERM)
			{
				action.f_action = vtactGxSELECT;
				action.f_parms[0] = 3;
			}
			else
				action.f_action = vtactIgnore;
			break;
		case 'Z':
			action.f_action = vtactIdentify;
			break;
		case '|': case '}': case '~':
			action.f_action = vtactIgnore;
			break;
		default:
			/*printf("avoiding Illegal seq=%c=\n", ic);*/
			parstate = vtpsDONE;
			break;
		}
		break;

	case vtpsESBRA:
		switch(ic) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			action.f_valparm[action.f_parmnum] = 1;
			action.f_parms[action.f_parmnum] = action.f_parms[action.f_parmnum] * 10 + (ic & 0xF);
			break;
		case '?':
			parstate = vtpsESBRAQ;
			break;
		case '/':
			action.f_action = vtactResetMode;
			break;
		case ';':
			if(++action.f_parmnum >= sizeof(action.f_parms)/sizeof(int))
				parstate = vtpsDONE;
			break;

		case 'A': /* Up */
		case 'F': /* PrevLine */
			if(! action.f_valparm[0]) action.f_parms[0] = 1;
			action.f_parms[0] = -action.f_parms[0]; action.f_parms[1] = 0;
			action.f_action = vtactRelPos;
			break;

		case 'B': /* Down */
		case 'E': /* NextLine */
			if(! action.f_valparm[0]) action.f_parms[0] = 1;
			action.f_parms[1] = 0;
			action.f_action = vtactRelPos;
			break;

		case 'C': /* Right */
			if(! action.f_valparm[0]) action.f_parms[1] = 1;
			else action.f_parms[1] = action.f_parms[0];
			action.f_parms[0] = 0;
			action.f_action = vtactRelPos;
			break;

		case 'D': /* Left */
			if(! action.f_valparm[0]) action.f_parms[1] = 1;
			else action.f_parms[1] = action.f_parms[0];
			action.f_parms[0] = 0; action.f_parms[1] = -action.f_parms[1];
			action.f_action = vtactRelPos;
			break;

		case 'G':
		case '`':
			if(emulation == emulXTERM)
			{
				if(! action.f_valparm[0]) action.f_parms[0] = 1;
				action.f_action = vtactHorzPos;
			}
			else
				action.f_action = vtactIgnore;
			break;

		case 'H': case 'f':
			if(action.f_valparm[0] == 0 || action.f_parms[0] == 0) action.f_parms[0] = 1;
			if(action.f_valparm[1] == 0 || action.f_parms[1] == 0) action.f_parms[1] = 1;
			action.f_action = vtactPosition;
			break;

		case 'I':
			if(! action.f_valparm[0]) action.f_parms[1] = 1;
			else action.f_parms[1] = action.f_parms[0];
			action.f_parms[1] *= 8;
			action.f_parms[0] = 0;
			action.f_action = vtactRelPos;
			break;

		case 'J': /* erase in screen */
			action.f_action  = vtactClearScr;
			break;

		case 'K': /* erase in line */
			action.f_action = vtactClearLine;
			break;

		case 'L': /* insert (pad) lines */
			if(emulation == emulXTERM)
			{
				if(action.f_valparm[0] == 0 || action.f_parms[0] == 0) action.f_parms[0] = 1;
				action.f_parms[1] = 1;
				action.f_action = vtactVertPad;
			}
			else
				action.f_action = vtactIgnore;
			break;

		case 'M': /* remove lines */
			if(emulation == emulXTERM)
			{
				if(! action.f_valparm[0]) action.f_parms[1] = 1;
				else action.f_parms[1] = action.f_parms[0];
				action.f_parms[0] = 3; // internal
				action.f_action = vtactClearScr;
			}
			else
				action.f_action = vtactIgnore;
			break;

		case '@': /* insert (pad) columns */
			if(emulation == emulXTERM)
			{
				if(! action.f_valparm[0]) action.f_parms[0] = 1;
				action.f_action = vtactHorzPad;
			}
			else
				action.f_action = vtactIgnore;
			break;

		case 'P': /* remove columns */
			if(emulation == emulXTERM)
			{
				if(! action.f_valparm[0]) action.f_parms[1] = 1;
				else action.f_parms[1] = action.f_parms[0];
				action.f_parms[0] = 3;
				action.f_action = vtactClearLine;
			}
			else
				action.f_action = vtactIgnore;
			break;

		case 'S':
		case 'T':
			if(emulation == emulXTERM)
			{
				if(! action.f_valparm[0]) action.f_parms[0] = 1;
				if(ic == 'T') action.f_parms[0] = -action.f_parms[0];
				action.f_action = vtactScroll;
			}
			else
				action.f_action = vtactIgnore;
			break;

		case 'X':
			if(emulation == emulXTERM)
			{
				if(! action.f_valparm[0])
					action.f_parms[1] = 1;
				else
					action.f_parms[1] = action.f_parms[0];
				action.f_parms[0] = 4;
				action.f_action = vtactClearLine;
			}
			else
				action.f_action = vtactIgnore;
			break;

		case 'b':
			if(emulation == emulXTERM)
			{
				if(! action.f_valparm[0]) action.f_parms[0] = 1;
				action.f_parms[1] = (int)'-'; // fix to remember last graphics char
				action.f_action = vtactRepeatChar;
			}
			else
				action.f_action = vtactIgnore;
			break;

		case 'c':
			action.f_action = vtactIdentify;
			break;

		case 'd':	/* vertical position absolute */
			if(action.f_valparm[0] == 0 || action.f_parms[0] == 0) action.f_parms[0] = 1;
			action.f_action = vtactVertPos;
			break;

		case 'g':
			if(action.f_valparm[0] == 0) action.f_parms[0] = 0;
			action.f_action = vtactClearTab;
			break;

		case 'h': case 'l':
			for(i = 0, attribs = 0; i <= action.f_parmnum && action.f_valparm[i]; i++)
			{
	 			//_tprintf(_T("action=%d\n"), action.f_parms[i]);
				switch(action.f_parms[i]) {
				case 20:	attribs |= vtmdLFNL;		break;
				case 1:		attribs |= vtmdCURKEY;		break;
				case 2:		attribs |= vtmdANSI;		break;
				case 3:		attribs |= vtmdCOLS;		break;
				case 4:		attribs |= vtmdSCROLL;		break;
				case 5:		attribs |= vtmdSCREEN;		break;
				case 6:		attribs |= vtmdORIGIN;		break;
				case 7:		attribs |= vtmdWRAP;		break;
				case 8:		attribs |= vtmdAUTO;		break;
				case 9:		attribs |= vtmdINTER;		break;
				default: break;
				}
			}
			if(ic == 'h') {
				action.f_parms[0] = attribs;
				action.f_parms[1] = 0;
			} else {
				action.f_parms[0] = 0;
				action.f_parms[1] = attribs;
			}
			action.f_action = vtactMode;
			break;

		case 'i': /* print screen */
			action.f_action = vtactIgnore;
			break;

		case 'm': // set attributes
			attribs    = 0;
			offattribs = 0;
			if(! action.f_valparm[0])
			{
				action.f_valparm[0] = 1;
				action.f_parms[action.f_parmnum++] = 0;
			}
			frg = 0;
			bkg = 0;
			for(i = 0; i <= action.f_parmnum && action.f_valparm[i]; i++)
			{
#ifdef LOG_VT
				if(loglevel > 5)
				{
					_tprintf(_T("process attrib %d\n"), action.f_parms[i]);
				}
#endif
				switch(action.f_parms[i]) {
				case 0:	attribs = vtdaNONE; offattribs = 0xFFFF;
						frg = 0xFF; bkg = 0xFF; break;
				case 1:	attribs |= vtdaBOLD;		break;
				case 4:	attribs |= vtdaUNDER;		break;
				case 5:	attribs |= vtdaBLINK;		break;
				case 7:	attribs |= vtdaREVERSE;		break;
				case 8: /* invisible */				break;
				case 22:offattribs |= vtdaBOLD;		break;
				case 24:offattribs |= vtdaUNDER;	break;
				case 25:offattribs |= vtdaBLINK;	break;
				case 27:offattribs |= vtdaREVERSE;	break;
				case 28: /* visible */				break;
				default:
					if(emulation == emulXTERM)
					{
						switch(action.f_parms[i])
						{
						case 30: case 31: case 32: case 33:
						case 34: case 35: case 36: case 37:
							frg = (action.f_parms[i] - 30) | 0x80;
							break;
						case 39:
							frg = 0xFF;
							break;
						case 40: case 41: case 42: case 43:
						case 44: case 45: case 46: case 47:
							bkg = (action.f_parms[i] - 40) | 0x80;
							break;
						case 49:
							bkg = 0xFF;
							break;
						default:
							break;
						}
					}
				}
			}
			action.f_action   = vtactAttribute;
			action.f_parms[0] = attribs;
			action.f_parms[1] = offattribs;
			action.f_parms[2] = frg;
			action.f_parms[3] = bkg;
			break;

		case 'n':
			if(action.f_parms[0] == 6)
				action.f_action = vtactPosReport;
			else
				action.f_action = vtactStatReport;
			break;

		case 'q':
			action.f_action = vtactSetLED; /* LED settings */
			break;

		case 'r':
			if(! action.f_valparm[0] || action.f_parms[0] == 0) action.f_parms[0] = 1;
			if(! action.f_valparm[1] || action.f_parms[1] == 0) action.f_parms[1] = 1;
			action.f_action = vtactSetScroll;
			break;

		case 'x':
			action.f_action = vtactParmReport; /* Report Params */
			break;

		case 'y':
			action.f_action = vtactSelfTest; /* Self Test */
			break;

		default:
			parstate = vtpsDONE;
			break;
		}
		break;

	case vtpsESBRAQ:
		switch(ic) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			action.f_valparm[action.f_parmnum] = 1;
			action.f_parms[action.f_parmnum] = action.f_parms[action.f_parmnum] * 10 + (ic & 0xF);
			break;
		case ';':
			if(++action.f_parmnum >= sizeof(action.f_parms)/sizeof(int))
				parstate = vtpsDONE;
			break;
		case 'J': /* erase in screen */
			action.f_action  = vtactClearScr;
			break;
		case 'K': /* erase in line */
			action.f_action = vtactClearLine;
			break;
		case 'h':
		case 'l': /* DEC private mode */
			action.f_action = (ic == 'h') ?	vtactDECmodeSet : vtactDECmodeReset;
			break;
		default:
			parstate = vtpsDONE;
			break;
		}
		break;

	case vtpsESARB:
		switch(ic) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			action.f_valparm[action.f_parmnum] = 1;
			action.f_parms[action.f_parmnum] = action.f_parms[action.f_parmnum] * 10 + (ic & 0xF);
			break;
		case ';':
			if(++action.f_parmnum >= sizeof(action.f_parms)/sizeof(int))
				parstate = vtpsDONE;
			break;
		case '\07':
			// set window title
			action.f_action = vtactRetitle;
			parstate = vtpsDONE;
			break;
		}
		break;

	case vtpsESCARET:
		switch(ic) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			action.f_valparm[action.f_parmnum] = 1;
			action.f_parms[action.f_parmnum] = action.f_parms[action.f_parmnum] * 10 + (ic & 0xF);
			break;
		case ';':
			if(++action.f_parmnum >= sizeof(action.f_parms)/sizeof(int))
				parstate = vtpsDONE;
			break;
		default:
			action.f_action = vtactIgnore;
			parstate = vtpsDONE;
			break;
		}
		break;

	case vtpsESUND:
		switch(ic) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			action.f_valparm[action.f_parmnum] = 1;
			action.f_parms[action.f_parmnum] = action.f_parms[action.f_parmnum] * 10 + (ic & 0xF);
			break;
		case ';':
			if(++action.f_parmnum >= sizeof(action.f_parms)/sizeof(int))
				parstate = vtpsDONE;
			break;
		default:
			action.f_action = vtactIgnore;
			parstate = vtpsDONE;
			break;
		}
		break;

	case vtpsESPERCENT:
		action.f_action = vtactSetCharset;
		switch(ic) {
		case '@': action.f_parms[0] = vtcsDefault;	break;
		case 'G': action.f_parms[0] = vtcsUTF8;		break;
		default:
			action.f_action = vtactIgnore;
			parstate = vtpsDONE;
			break;
		}
		break;

	case vtpsESPAR:
	case vtpsESRAP:
	case vtpsESSTAR:
	case vtpsESPLUS:
		action.f_action = vtactGxSET;
		switch(parstate) {
		case vtpsESPAR:
			action.f_parms[0] = 0;
			break;
		case vtpsESRAP:
			action.f_parms[0] = 1;
			break;
		case vtpsESSTAR:
			action.f_parms[0] = 2;
			break;
		case vtpsESPLUS:
			action.f_parms[0] = 3;
			break;
		default:
			break;
		}
		switch(ic) {
		case 'A': action.f_parms[1] = vtcsUK;		break;
		case 'B': action.f_parms[1] = vtcsUSASCII;	break;
		case '0': action.f_parms[1] = vtcsGraphics;	break;
		case '1': action.f_parms[1] = vtcsDefault;	break;
		case '2': action.f_parms[1] = vtcsGraphics;	break;
		case '4': action.f_parms[1] = vtcsDutch;	break;
		case '5':
		case 'C': action.f_parms[1] = vtcsFinnish;	break;
		case 'R': action.f_parms[1] = vtcsFrench;	break;
		case 'Q': action.f_parms[1] = vtcsCanuck;	break;
		case 'K': action.f_parms[1] = vtcsGerman;	break;
		case 'Y': action.f_parms[1] = vtcsItalian;	break;
		case '6':
		case 'E': action.f_parms[1] = vtcsNorwegian;break;
		case 'Z': action.f_parms[1] = vtcsSpanish;	break;
		case '7':
		case 'H': action.f_parms[1] = vtcsSwedish;	break;
		case '=': action.f_parms[1] = vtcsSwiss;	break;
		default:
			parstate = vtpsDONE;
			break;
		}
		break;

	case vtpsPOUND:
		switch(ic) {
		case 3: case 4: case 5: case 6: /* line height */
			action.f_action = vtactIgnore;
			break;
		case 8:							/* E's */
			action.f_action = vtactIgnore;
			break;
		default:
			parstate = vtpsDONE;
		}
		break;

	case vtpsSPACE:
		switch(ic) {
		case 'F':	action.f_parms[0] = 7; action.f_action = vtactBits; break;
		case 'G':	action.f_parms[0] = 8; action.f_action = vtactBits; break;
		case 'L':	action.f_parms[0] =  emulVT100; action.f_action = vtactANSIlevel; break;
		case 'M':	action.f_parms[0] =  emulVT220; action.f_action = vtactANSIlevel; break;
		case 'N':	action.f_parms[0] =  emulVT320; action.f_action = vtactANSIlevel; break;
		default:
			parstate = vtpsDONE;
		}
		break;

	default:
		parstate = vtpsDONE;
		break;
	}
	if(action.f_action != vtactUnknown)
		parstate = vtpsDONE;
	return parstate;
}

//**************************************************************************
void BviewTerminal::InsertToken(bool insert)
{
	ERRCODE ec;
	LPCTSTR lpText;
	int     nText;

	if (m_tokcnt <= 0)
		return;

	m_insertcount++;

	m_intok[m_tokcnt] = 0;

	m_minvtrow	= min(m_minvtrow, m_vtrow);
	m_maxvtrow	= max(m_maxvtrow, m_vtrow);
	m_prevvtcol = min(m_prevvtcol, m_vtcol);

	#ifdef LOG_VT
	m_log->Log(logDebug, 5, _T("insert %d,%d =" _Pfs_ "=\n"), m_vtrow, m_vtcol, m_intok);
	#endif
	// make sure insertion point is actual vt pos, not curline/curcol, since
	// that is the actual caret pos, which user could move, but only if the curlin/col
	// is viewable, otherwise assume user has scrolled up, etc.
	//
	if (InsertPointVisible())
	{
		if(m_curline != m_vtrow)
		{
			#ifdef LOG_VT
			m_log->Log(logDebug, 5, _T("insert reset line from %d to %d\n"),
				m_curline, m_vtrow);
			#endif
			m_curline = m_vtrow;
		}
		if(m_curcol != m_vtcol)
		{
			#ifdef LOG_VT
			m_log->Log(logDebug, 5, _T("insert reset col from %d to %d\n"),
				m_curcol, m_vtcol);
			#endif
			m_curcol = m_vtcol;
		}
	}
	if(m_tokcnt > 0)
	{
		// remap special graphics chars
		//
		if(m_charSets[m_charset] == vtcsGraphics)
		{
			int n;

			for(n = 0; n < m_tokcnt; n++)
			{
				if(m_intok[n] >= 0 && m_intok[n] < 32)
					m_intok[n] += 95;
				if(m_intok[n] >= 95 && m_intok[n] <= 126)
				{
					TCHAR c;
#ifdef UNICODE	/* convert special graphics codes to UNICODE based on cp437 */
					switch(m_intok[n])
					{
					case 95:		c = 0x25AE;	break;	// blank
					case 96:		c = 0x25C6;	break;	// diamond
					case 97:		c = 0x2592; break;	// checker
					case 98:		c = 0x2409; break;  // tab
					case 99:		c = 0x240C; break;  // formfeed

					case 100:		c = 0x240D; break;  // carriage ret
					case 101:		c = 0x240A; break;  // line feed
					case 102:		c = 0x00B0; break;	// degree
					case 103:		c = 0x00B1; break;	// +/-
					case 104:		c = 0x2424; break;	// newline

					case 105:		c = 0x240B; break;	// vert tab
					case 106:		c = 0x2518; break;	// lower right corner
					case 107:		c = 0x2510; break;	// upper right corner
					case 108:		c = 0x250C; break;	// upper left
					case 109:		c = 0x2514; break;	// lower left
					case 110:		c = 0x253C; break;	// crossing lines
					case 111:		c = 0x23BA; break;	// h line
					case 112:		c = 0x23BB; break;	// h line
					case 113:		c = 0x2500; break;	// h line
					case 114:		c = 0x23BC; break;	// h line
					case 115:		c = 0x23BD; break;	// h line
					case 116:		c = 0x251C; break;	// left T
					case 117:		c = 0x2524; break;	// right T
					case 118:		c = 0x2534; break;	// bottom T
					case 119:		c = 0x252C; break;	// top T
					case 120:		c = 0x2502; break;	// vertical bar
					case 121:		c = 0x2264; break;	// <=
					case 122:		c = 0x2265; break;	// >=
					case 123:		c = 0x03C0; break;	// pi
					case 124:		c = 0x2260; break;	// !=
					case 125:		c = 0x00A3; break;	// brit pound
					case 126:		c = 0x00B7; break;	// centered dot
					}
					m_intok[n] = c;
#else		   /* not unicode, use cp437 representation ? */
#endif
				}
			}
		}
	}
	// get the line's text, since we really are probably over-writing existing text
	//
	ec = m_buffer->GetLineText(m_vtrow, lpText, nText);
	if (ec != errOK)
	{
		lpText = _T("");
		nText = 0;
	}
	// convert the text token into text-attribute triplets, and re-map to
	// graphics chars if special graphics charset selected
	//
	TCHAR token[1024];
	int   tokcnt = 1024;

	tokcnt = VTformatToken(m_intok, m_tokcnt, token, tokcnt);

	if(tokcnt)
	{
		// map m_vtcol to position in buffer
		//
		int curcol = m_vtcol * 3 - 2;

		if(! insert)
		{
			bool hasNL;
			int  nDelete;

			// remove (replace) extant text with this text, but only as much
			// as we are inserting (overstrike basically), and be careful
			// not to remove a newline at the end of the line if there is one
			// already there
			//
			if(nText > 0)
			{
				hasNL = lpText[nText - 1] == _T('\n') || lpText[nText - 1] == _T('\r');

				// count of bytes to right of current insert point
				//
				nDelete = nText - (curcol - 1);

				if(nDelete > tokcnt)
				{
					// if more than tokcnt, just take out tokcnt
					//
					nDelete = tokcnt;
				}
				else
				{
					// less or equal, take out only what's there, or
					// one less if there is a newline
					//
					if(hasNL)
					{
						nDelete--;
					}
				}
				if(nDelete)
				{
#ifdef LOG_VT
					VTdumpToken(m_log, lpText, nText);
					m_log->Log(logDebug, 5, _T("removing %d bytes at buffer %d for insert\n"), nDelete, curcol);
#endif
					m_buffer->Delete(m_vtrow, curcol, nDelete);
#ifdef LOG_VT
					ec = m_buffer->GetLineText(m_vtrow, lpText, nText);
					VTdumpToken(m_log, lpText, nText);
					m_log->Log(logDebug, 5, _T("done delete\n"));
#endif
				}
			}
			else
			{
				// make sure there's a new line at the end of each line in buffer, since there was
				// no text in this line, reset col to 1
				//
				int xcol = 1;
				int xrow = m_vtrow;

				m_buffer->Insert(xrow, xcol, _T("\n"), 1);
				m_vtcol = 1;

				// update current position of visible
				//
				if (InsertPointVisible())
				{
					m_curline = m_vtrow;
					m_curcol = m_vtcol;
				}
			}
		}
#ifdef LOG_VT
		m_log->Log(logDebug, 5, _T("insert tok =" _Pfs_ "= at buffer %d,%d\n"), m_intok, m_vtrow, curcol);
#endif
		m_buffer->Insert(m_vtrow, curcol, token, tokcnt);
#ifdef LOG_VT
		ec = m_buffer->GetLineText(m_vtrow, lpText, nText);
		VTdumpToken(m_log, lpText, nText);
		m_log->Log(logDebug, 5, _T("inserted tok\n"));
#endif
		m_vtcol = (curcol + 2) / 3;
		m_totalcnt+= tokcnt;

		if(m_vtmode & vtmdWRAP)
		{
			ec = m_buffer->GetLineText(m_vtrow, lpText, nText);

			// if inserted past view cols, insert a newline at viewcols
			//
			if( ((nText + 1) / 3) > (m_vtcols))
			{
				int wrapcol = m_vtcols * 3 + 1;
#ifdef LOG_VT
				m_log->Log(logDebug, 5,_T("wrap cols=%d and view=%d\n"), nText, m_vtcols);
#endif
				m_buffer->Insert(m_vtrow, wrapcol, _T("\n"), 1);

			}
		}
		if((m_vtmode & vtmdLOG) && (m_totalcnt > m_logthresholdsize))
		{
			m_totalcnt = 0;
			WriteLog();
		}
	}
	m_tokcnt = 0;
}

//**************************************************************************
void BviewTerminal::SetCurrentAttributes(int attron, int attroff, char frg, char bkg)
{
	if(m_tokcnt > 0)
	{
		InsertToken(false);
	}

	m_attribs &= ~attroff;
	m_attribs |= attron;

	if(frg & 0x80)
	{
		if((unsigned char)frg == 0xFF)
		{
			m_deffrg = true;
			m_curfrg = 0;
		}
		else
		{
			m_deffrg = false;
			m_curfrg = frg & 0x7F;
		}
#ifdef LOG_VT
		m_log->Log(logDebug, 5,_T("new frg color %d\n"), m_curfrg);
#endif
	}

	if(bkg & 0x80)
	{
		if((unsigned char)bkg == 0xFF)
		{
			m_defbkg = true;
			m_curbkg = 7;
		}
		else
		{
			m_defbkg = false;
			m_curbkg = bkg & 0x7F;
		}
#ifdef LOG_VT
		m_log->Log(logDebug, 5,_T("new bkg color %d\n"), m_curbkg);
#endif
	}
}

//**************************************************************************
bool BviewTerminal::InsertPointVisible()
{
	bool    isVis;

	// see if insertion position is visible and dont do updates except in
	//
	isVis = (m_minvtrow >= m_topline) && m_minvtrow < (m_topline + m_vtrows + 2);
	isVis |= (m_maxvtrow >= m_topline) && m_maxvtrow < (m_topline + m_vtrows + 2);
	if(! isVis)
	{
#ifdef LOG_VT
		m_log->Log(logDebug, 5, _T("chkfrm insert point not visible, no udpate\n"));
#endif
	}
	return isVis;
}

//**************************************************************************
void BviewTerminal::CheckFrame(bool extend_update_region)
{
#ifdef LOG_VT
	m_log->Log(logDebug, 5, _T("checkframe line=%d col=%d tl=%d fl=%d\n"), m_vtrow, m_vtcol, m_topline, m_frameline);
#endif
	if(m_vtrows > m_cacheLines && m_cacheLines > 0)
	{
		m_vtrows = m_cacheLines;
	}
	if(InsertPointVisible())
	{
		if((m_vtrow - m_frameline) >= m_vtrows)
		{
	#ifdef LOG_VT
			m_log->Log(logDebug, 5, _T("chkfrm update view (off screen)\n"));
	#endif
			m_topline = m_vtrow - m_vtrows + 1;
		}
		if(m_frameline != m_topline)
		{
	#ifdef LOG_VT
			m_log->Log(logDebug, 5, _T("chkfrm reset frame top to %d\n"), m_topline);
	#endif
			m_frameline = m_topline;
			m_minvtrow = min(m_minvtrow, m_frameline);
		}
	}
	if(m_insertcount > TERM_UPDATE_CHUNK)
	{
#ifdef LOG_VT
		m_log->Log(logDebug, 5, _T("chkfrm update view cnt=%d linerange=%d,%d\n"),
				m_insertcount, min(m_minvtrow, m_vtrow),max(m_maxvtrow, m_vtrow));
#endif
		m_insertcount = 0;
		if(InsertPointVisible())
		{
			UpdateView(
							min(m_minvtrow, m_vtrow),		// line 1
							1,								// col  1
							extend_update_region ? 			// line 2
									MAX_LINE_LEN :
									max(m_maxvtrow, m_vtrow),
							MAX_LINE_LEN					// col 2
					  );
			MoveAbs(m_vtrow, m_vtcol);
		}
#ifdef LOG_VT
		else
		{
			m_log->Log(logDebug, 5, _T("chkfrm update view not visibile\n"));
		}
#endif
		SetScrollers(SB_VERT);
		m_minvtrow	= m_vtrow;
		m_maxvtrow	= m_vtrow;
		m_prevvtcol	= m_vtcol;
	}
}

//**************************************************************************
void BviewTerminal::UpdateVtPos(int line, int col)
{
	ERRCODE ec;
	LPCTSTR	lpText;
	int ll, nLineText;
	int lines_to_make, lines_to_end, bottom_line, line_count;

	if(col < 1)  col = 1;
	if(line < 1) line = 1;

#ifdef LOG_VT
	m_log->Log(logDebug, 5, _T("pre updatevtpos=%d,%d fl=%d= goto %d,%d (%d,%d)\n"),
			m_vtrow, m_vtcol, m_frameline,
			line, col, line + m_frameline - 1, col);
#endif
	// convert vt100 coords to buffer coords
	//
	// all Y coords are relative to the "frame" line, which is generally the
	// top line of the current view window.
	//
	line = line + m_frameline - 1;

	// all X coords are relative to the windows left edge offset
	//
	//col  = col - m_leftcol + 1;

	// all X coords are shifted right by 2 for each attribute marker
	// to the left of the coord, but we keep the VT col in one-char space
	// so only the actual buffer operations translate

	if(line < 1) line = 1;
	if(col < 1)  col  = 1;

	// probe the buffer to see if line exists and if not, pad
	// buffer out.  assume there could be many thousands of lines
	// and we're at or near the end of the buffer, which is the typical
	// case, so avoid "GetLineCount()" which could slow us down
	//
	lines_to_end = m_buffer->GetLineCountToEndFromLine(m_vtrow);
	if (lines_to_end == 0)
	{
		// no lines to end (typical) OR, vtrow is off the bottom and has to be clamped
		ec = m_buffer->GetLineText(m_vtrow, lpText, nLineText);
		if (ec != errOK)
		{
			// row is bad, do this the hard way
			int maxlen;

			line_count = m_buffer->GetLineCount(maxlen);
			lines_to_make = line - line_count;
			bottom_line = line_count;
		}
		else
		{
			// vtrow is at last row of buffer
			lines_to_make = line - m_vtrow;
			bottom_line = m_vtrow;

			// insist the last line has a newline before padding
			if(nLineText == 0)
				lines_to_make++;
		}
	}
	else
	{
		// make the dangly part after bottom of buffer
		lines_to_make = line - m_vtrow - lines_to_end;
		bottom_line = m_vtrow + lines_to_end;
	}
#ifdef LOG_VT
	m_log->Log(logDebug, 5, _T("Updatevt pad %d lines\n"), lines_to_make);
#endif

	// move to line in the buffer, padding with
	// blank lines if needed to fill the space above
	//
	ll = MAX_LINE_LEN;
	if (lines_to_make > 0)
	{
		while (lines_to_make-- > 0)
		{
			m_buffer->Insert(bottom_line, ll, _T("\n"), 1);
			//_tprintf(_T("pad line %d to %d\n"), bottom_line, line);
		}
	}
	else if (m_scrlt > 1 && m_scrlb >= m_scrlt)
	{
		int scrolldist;
		int topline;
		int botline;

		topline = m_frameline + m_scrlt - 1; // line in buffer at top of scroll region
		botline = m_frameline + m_scrlb - 1; // line in buffer past bottom of scroll region

		if (line > botline)
		{
			int linecol, tl, bl;

			// moving to a line within an existing scroll region
			// so check if scrolling off the bottom and if so
			// scroll the contents of the window up (by deleting
			// the line at the top of the scroll window)
			//
#ifdef LOG_VT
			m_log->Log(logDebug, 3, _T("move below to %d in window %d,%d\n"),
					line, topline, botline);
#endif
			scrolldist = line - botline; // how many lines to scroll
			m_minvtrow = topline;
			while (scrolldist > 0)
			{
				// delete the top line in the scroll window
#ifdef LOG_VT
				m_log->Log(logDebug, 5, _T("del srt line %d\n"), topline);
#endif
				// delete the line at the top of the scroll area
				ec = m_buffer->GetLineText(topline, lpText, nLineText);
				tl = topline;
				linecol = 1;
				m_buffer->Delete(tl, linecol, nLineText);

				// insert a new line at end of bottom scroll area
				bl = botline;
				ll = 1;
				m_buffer->Insert(bl, ll, _T("\n"), 1);

				scrolldist--;
			}
			// and keep cursor on bottom line
			line = botline;
			m_maxvtrow = botline;
		}
		else if (line < topline)
		{
			int linecol, tl, bl;

			// moving to a line above the scroll region, so delete bottom
			// line and insert blank line on top
#ifdef LOG_VT
			m_log->Log(logDebug, 3, _T("move above to %d in window %d,%d\n"),
				line, topline, botline);
#endif
			scrolldist = topline - line; // how many lines to scroll
			m_minvtrow = topline;
			while (scrolldist > 0)
			{
				// insert a new line at start of scroll area
				tl = topline;
				ll = 1;
				m_buffer->Insert(tl, ll, _T("\n"), 1);

				// delete the bottom line in the scroll window
#ifdef LOG_VT
				m_log->Log(logDebug, 5, _T("del srb line %d\n"), botline);
#endif
				// delete the line at the bottom of the scroll area
				bl = botline + 1;
				ec = m_buffer->GetLineText(bl, lpText, nLineText);
				linecol = 1;
				m_buffer->Delete(bl, linecol, nLineText);
				scrolldist--;
			}
			// and keep cursor on top line
			line = topline;
			m_maxvtrow = botline;
		}
	}
	// now pad line with spaces to get to col,
	// if the line is shorter than col
	//
	{
		int	pad, prevnlt = 0;

		ec = m_buffer->GetLineText(line, lpText, nLineText);
		if (nLineText == 0)
		{
			// empty line, put a newline in and redo
			//
			int iline = line, icol = 1;

			m_buffer->Insert(iline, icol, _T("\n"), 1);
			ec = m_buffer->GetLineText(line, lpText, nLineText);
		}
		pad = col - ((nLineText + 2) / 3);

		if(ec == errOK && pad > 0)
		{
#ifdef LOG_VT
			m_log->Log(logDebug, 5, _T("upvt pad %d cols from %d to %d\n"), pad, (nLineText + 2) / 3, col);
#endif
			for(m_tokcnt = 0; m_tokcnt < pad && m_tokcnt < sizeof(m_intok)/sizeof(TCHAR); m_tokcnt++)
				m_intok[m_tokcnt] = _T(' ');
			m_intok[m_tokcnt] = 0;

			m_vtrow = line;
			m_vtcol = (nLineText + 2) / 3;
			InsertToken(true);
		}
#ifdef LOG_VT
		else
		{
			m_log->Log(logDebug, 5, _T("upvt ec=%d pad=%d\n"), ec, pad);
		}
#endif
	}
	// save the line/col cause this is actual vt cursor pos
	//
	m_vtrow = line;
	m_vtcol = col;

	// check that the current line is visible the way we
	// (as a vt100) want it to be (smooth scroll e.g.)
	//
	CheckFrame(false);

#ifdef LOG_VT
	m_log->Log(logDebug, 5, _T("post updatevtpos=%d,%d fl=%d=\n"), m_vtrow, m_vtcol, m_frameline);
#endif
}

//**************************************************************************
void BviewTerminal::Event(LPARAM lParam)
{
	LPCTSTR lpText;
	ERRCODE ec;
	int     nText, nLineText, nPadText, n, xcol, i, j;
	int		iline, icol, lines;
	char	xc;
	TCHAR	c;
	int   	uv;
	bool	nonwhite, nonwhitedelim;

	uv = 0;

	// got data from the shell thread
	//
	while(m_cnt > 0)
	{
		m_bufex.Lock();

		if(m_tail >= m_head)
			nText = m_size - m_tail;
		else
			nText = m_head - m_tail;

		xc = m_iobuf[m_tail + nText];
		m_iobuf[m_tail + nText] = 0;
		{
			int ix;

			for(ix = 0; ix < nText; ix++)
			{
				if(m_iobuf[m_tail + ix] == 0)
				{
					m_iobuf[m_tail + ix] = ' ';
				}
			}
		}

#ifdef UNICODE
		BUtil::Utf8Decode(m_szInp, (char*)m_iobuf + m_tail);
#else
		strcpy(m_szInp, (char*)(m_iobuf + m_tail));
#endif
		m_iobuf[m_tail + nText] = xc;
		m_tail += nText;
		m_cnt -= nText;
		if(m_tail >= m_size)
			m_tail = 0;

		m_bufex.Unlock();

		nText = _tcslen(m_szInp);

		// pre-tokenize the text into visible text tokens
		// converting any vt100 escapes into actual insertion
		// point changes or formatting changes
		//
		// attribute escapes are converted into special tokens
		// which the regular view get-token understands and
		// sets the color/font appropriately
		//
		for(n = 0, uv = 0, nonwhite = false; n < nText; n++)
		{
			c = m_szInp[n];
			if(c == _T(' ') || c == _T('\t'))
			{
				nonwhitedelim = false;
			}
			else
			{
				nonwhite = true;
				nonwhitedelim = true;
			}
			m_vtState = ParseVT100(
					(char)c,
					m_vtState,
					m_emulation,
					m_action,
#ifdef LOG_VT
					m_log->GetLevel()
#else
					0
#endif
				);

			if (uv & vtrcSCREEN) {
				// ensure min/max dirty encompasses screen
				m_minvtrow = min(m_minvtrow, m_frameline);
				m_maxvtrow = max(m_maxvtrow, m_minvtrow + m_vtrows);
			}
			else if (uv & vtrcRESTOFSCR) {
				// ensure min/max dirty encompasses rest of screen
				m_minvtrow = min(m_minvtrow, m_vtrow);
				m_maxvtrow = max(m_maxvtrow, m_minvtrow + m_vtrows);
			}
			// if state is done, check action
			//
			if(m_vtState == vtpsDONE)
			{
				m_vtState = vtpsINIT;

				// flush out any dangling tokens before
				// executing a vt command
				//
				if(m_tokcnt > 0 && m_action.f_action != vtactNone)
				{
					InsertToken(false);
					nonwhite = false;
					CheckFrame(uv & (vtrcSCREEN | vtrcRESTOFSCR) ? true : false);
					uv = 0;
				}
				switch(m_action.f_action)
				{
				case vtactUnknown:
				case vtactNone:

					// the parse is non control, so add char to token
					// ending the token on delimiters and white space
					// to check for line wrapping
					//
					if(
							((m_tokcnt > 0) && IsDelim(c) && (nonwhite || nonwhitedelim))
						||	((m_tokcnt >= (sizeof(m_intok) / sizeof(TCHAR)) - 4))
					)
					{
#ifdef LOG_VT
						m_intok[m_tokcnt] = 0;
						m_log->Log(logDebug, 5, _T("vtnone = nw=%d nwd=%d  %d,%d =" _Pfs_ "=\n"),
								nonwhite, nonwhitedelim, m_vtrow, m_vtcol, m_intok);
#endif
						// got a token, insert into buffer
						//
						InsertToken(false);
						nonwhite = false;
						CheckFrame(true);
					}
					m_intok[m_tokcnt++] = c;
					break;

				case vtactNewline:

#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("newline at %d,%d\n"),
							m_vtrow, m_vtcol);
#endif
					if(m_tokcnt > 0)
					{
						InsertToken(false);
						nonwhite = false;
					}
					UpdateVtPos(
									m_vtrow - m_frameline + 2,
									(m_vtmode & vtmdLFNL) ? 1 : m_vtcol
							    );
					break;

				case vtactVertPos:

					m_action.f_parms[1] = m_vtcol;
					// fall
				case vtactPosition:
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("pos abs %d %d\n"), m_action.f_parms[0], m_action.f_parms[1]);
#endif
					UpdateVtPos(m_action.f_parms[0], m_action.f_parms[1]);
					uv |= vtrcDISPLAY | vtrcPOSITION;
					break;

				case vtactHorzPos:

					m_action.f_parms[1] = m_action.f_parms[0] - m_vtcol;
					m_action.f_parms[0] = 0;
					// fall
				case vtactRelPos:
				case vtactIndex:
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("pos rel %d %d\n"), m_action.f_parms[0], m_action.f_parms[1]);
#endif
					UpdateVtPos(m_vtrow - m_frameline + 1 + m_action.f_parms[0], m_vtcol + m_action.f_parms[1]);
					uv |= vtrcDISPLAY | vtrcPOSITION;
					break;

				case vtactAttribute:
					// insert an attribute marker into the current position
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("attrib %d %d\n"), m_action.f_parms[0], m_action.f_parms[1]);
#endif
					SetCurrentAttributes(m_action.f_parms[0], m_action.f_parms[1], m_action.f_parms[2], m_action.f_parms[3]);
					break;

				case vtactClearLine:
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("clearline: %d\n"), m_action.f_parms[0]);
#endif
					ec = m_buffer->GetLineText(m_vtrow, lpText, nLineText);

					// get actual buffer column of current view column
					//
					xcol = m_vtcol * 3 - 2;

					// if drawing in colors, clear really means replace or pad with
					// spaces of current colors, not the wipe out done normally
					//
					switch(m_action.f_parms[0])
					{
					case 0: // clear to eol (replace/pad if in color)
						if(IsPlainText())
						{
							// regular text, just chop it
							nLineText -= xcol;	// how many bytes to take out
							nPadText = 0;
							nLineText -= 1; 	// don't delete end of line newline
							if(nLineText <= 1) break;
#ifdef LOG_VT
							m_log->Log(logDebug, 5, _T("===eol== del %d chars from %d\n"),
									nLineText, xcol);
#endif
						}
						else
						{
							// colored text, replace
							nLineText -= xcol;	// how many bytes to take out
							nPadText = m_vtcols - m_vtcol;
							nLineText -= 1; 	// don't delete end of line newline
							if(nLineText <= 1 && nPadText < 1) break;
#ifdef LOG_VT
							m_log->Log(logDebug, 5, _T("===eol== del %d chars from %d, add %d\n"),
									nLineText, xcol, nPadText);
#endif
						}
						break;
					case 1: // clear to bol (replace only)
						nPadText = m_vtcol - 1;
						nLineText = xcol - 1;
						xcol = 1; // need to del from front
						m_vtcol = 1;
#ifdef LOG_VT
						m_log->Log(logDebug, 5, _T("===bol== del %d chars, add %d\n"),
								nLineText, nPadText);
#endif
						break;
					case 2: // clear line (replace/pad if in color)
						m_vtcol = 1;
						nLineText -= 1;
						if(m_curfrg != 0 || m_curbkg != 7)
						{
							// color, replace with colored space
							nPadText = m_vtcols;
						}
						else
						{
							nPadText = 0;
						}
#ifdef LOG_VT
						m_log->Log(logDebug, 5, _T("===line= del %d chars, add %d\n"),
								nLineText, nPadText);
#endif
						break;
					case 3: // (internal) delete in line (xterm [nP)
						nLineText = m_action.f_parms[1] * 3;
						nPadText = 0;
						break;
					case 4: // (internal) erase in line (xterm [nX)
						nPadText = m_action.f_parms[1];
#ifdef LOG_VT
						m_log->Log(logDebug, 5, _T("==erase= %d cols.  %d(%d) remain in line of %d(%d)\n"),
							nPadText, nLineText - xcol, ((nLineText - xcol) + 2) / 3,
							nLineText, (nLineText + 2) / 3);
#endif
						if((nPadText * 3) >= (nLineText - xcol - 1))
							nLineText = nLineText - xcol - 1;
						else
							nLineText = nPadText * 3;
						break;
					default:
						nLineText = nPadText = 0;
						break;
					}
					if(nLineText > 1)
					{
						// remove text in the way
						//
						m_buffer->Delete(m_vtrow, xcol, nLineText);
					}
					if(nPadText > 1)
					{
						int scol = m_vtcol;
						for(m_tokcnt = 0; m_tokcnt < nPadText && m_tokcnt < (sizeof(m_intok)/sizeof(TCHAR) - 1); m_tokcnt++)
						{
							m_intok[m_tokcnt] = ' ';
						}
						InsertToken(true);
						if(m_action.f_parms[0] == 0 || m_action.f_parms[0] == 2)
							m_vtcol = scol;
					}
					uv |= vtrcDISPLAY | vtrcLINE;
					break;

				case vtactClearScr:
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("clear screen %d at %d\n"),
							m_action.f_parms[0], m_vtcol);
#endif
					// get actual buffer column of current view column
					//
					xcol = m_vtcol * 3 - 2;

					switch(m_action.f_parms[0])
					{
					case 0: // eos
						if(IsPlainText())
						{
							// delete all between curcol and newline on this line, and all subsequent lines
							//
							i = m_vtrow;
							j = CountRegionChars(m_buffer, i, xcol, i, MAX_LINE_LEN);
							if(j > 1)
							{
#ifdef LOG_VT
								m_log->Log(logDebug, 5, _T("del %d from line %d, col %d\n"),
									nLineText - 1 - xcol, i, xcol);
#endif
								m_buffer->Delete(i, xcol, j - 1);
							}
							else
							{
#ifdef LOG_VT
								m_log->Log(logDebug, 5, _T("del none from line %d, col %d, nlt=%d\n"),
									i, xcol, nLineText);
#endif
							}
							i = m_vtrow + 1;
							xcol = 1;
							j = CountRegionChars(m_buffer, i, xcol, MAX_LINE_LEN, MAX_LINE_LEN);
							if(j > 1)
							{
#ifdef LOG_VT
								m_log->Log(logDebug, 5, _T("del %d MORE from line %d, col %d\n"),
									j, i, xcol);
#endif
								m_buffer->Delete(i, xcol, j);
							}
						}
						else
						{
							int srow = m_vtrow;
							int scol = m_vtcol;
							int n = 0;

							// erase, means replace chars with current color to end of screen
							// so just leave newlines for rest of screen, or out of lines

							while(n < m_vtrows)
							{
								i = srow + n;
								nLineText = CountRegionChars(m_buffer, i, xcol, i, MAX_LINE_LEN);
								if (nLineText <= 1)
									break;
								if(nLineText > xcol)
								{
#ifdef LOG_VT
									m_log->Log(logDebug, 5, _T("cls eos, delete %d at line %d (%d)\n"),
										nLineText - xcol - 1, m_vtrow, i);
#endif
									m_buffer->Delete(i, xcol, nLineText - xcol);
									for(
											m_tokcnt = 0;
											m_tokcnt < ((nLineText - xcol) + 2) / 3
										&&	m_tokcnt < (sizeof(m_intok)/sizeof(TCHAR) - 1);
											m_tokcnt++
									)
									{
										m_intok[m_tokcnt] = ' ';
									}
#ifdef LOG_VT
									m_log->Log(logDebug, 5, _T("cls eos, insert %d spaces at line %d\n"),
										nLineText - xcol, m_vtrow);
#endif
									m_vtrow = i;
									m_vtcol = xcol;
									InsertToken(true);
								}
								xcol = 1;
								n++;
							}
							m_vtrow = srow;
							m_vtcol = scol;
						}
						uv |= vtrcDISPLAY | vtrcRESTOFSCR;
						break;
					case 1: // bos
						if(m_vtrow < m_frameline)
							break;
						if(m_vtcol > 1)
						{
							nLineText = CountRegionChars(m_buffer, m_frameline, 1, m_vtrow, m_vtcol);
						}
						else if(m_vtrow > m_frameline)
						{
							nLineText = CountRegionChars(m_buffer, m_frameline, 1, m_vtrow - 1, MAX_LINE_LEN);
						}
						else
						{
							break;
						}
						m_vtcol = 1;
						m_buffer->Delete(m_frameline, m_vtcol, nLineText);
						uv |= vtrcDISPLAY | vtrcSCREEN;
						break;
					case 2: // all scr
						if(IsPlainText())
						{
							i = m_vtrow;
							nLineText = CountRegionChars(m_buffer, i, 1, m_frameline + m_vtrows, MAX_LINE_LEN);
							if(nLineText > 1)
							{
								xcol = 1;
								m_buffer->Delete(i, xcol, nLineText - 1);
							}
						}
						else
						{
							int srow = m_vtrow;
							int scol = m_vtcol;
							int n = 0;

							// erase, means replace chars with current color to end of screen. first make
							// sure there ARE lines to what we think is the end of the screen from the
							// frame line
							//
							UpdateVtPos(m_frameline + m_vtrows - 1, 1);
							UpdateVtPos(m_vtrow, m_vtcol);

							while(n < m_vtrows)
							{
								i = n + srow;
								nLineText = CountRegionChars(m_buffer, i, 1, i, MAX_LINE_LEN);
								if(nLineText > 0)
								{
#ifdef LOG_VT
									m_log->Log(logDebug, 5, _T("cls scr, delete %d at line %d (%d)\n"),
										nLineText - 1, m_vtrow, i);
#endif
									xcol = 1;
									m_buffer->Delete(i, xcol, nLineText - 1);
									for(
											m_tokcnt = 0;
											m_tokcnt < m_vtcols /*((nLineText - 1) + 2) / 3*/
										&&	m_tokcnt < (sizeof(m_intok)/sizeof(TCHAR) - 1);
											m_tokcnt++
									)
									{
										m_intok[m_tokcnt] = ' ';
									}
									m_vtrow = i;
									m_vtcol = 1;
#ifdef LOG_VT
									m_log->Log(logDebug, 5, _T("cls scr, insert %d spaces at line %d\n"),
										nLineText - 1, m_vtrow);
#endif
									InsertToken(true);
								}
								n++;
							}
							m_vtrow = srow;
							m_vtcol = 1;
						}
						uv |= vtrcDISPLAY | vtrcSCREEN;
						break;
					case 3: // (internal only) remove n lines starting at current line
						int eline;
						int ecol;

						lines = m_action.f_parms[1];
						eline = m_vtrow + lines;
						ecol = 1;
						iline = m_vtrow;
						icol = 1;

						j = CountRegionChars(m_buffer, iline, icol, eline, ecol);
						if(j > 1)
						{
							// if deleting lines inside a scroll region, pad bottom of scroll region with
							// same number of lines as removed first
							//
							if (m_vtrow >= (m_scrlt + m_frameline - 1) && m_vtrow < (m_scrlb + m_frameline))
							{
#ifdef LOG_VT
								m_log->Log(logDebug, 5, _T("clrln pad end of scroll reg %d by %d\n"),
									m_scrlb + m_frameline, lines);
#endif
								int xcol = m_vtcol;
								int xrow = m_vtrow;

								m_vtrow = m_scrlb + m_frameline;
								while(lines > 0)
								{
									m_vtcol = 1;
									for(
											m_tokcnt = 0;
											lines > 0 && m_tokcnt < (sizeof(m_intok)/sizeof(TCHAR) - 1);
											m_tokcnt++,
											lines--
									)
									{
										m_intok[m_tokcnt] = '\n';
									}
									InsertToken(true);
								}
								m_vtcol = xcol;
								m_vtrow = xrow;
							}
#ifdef LOG_VT
							m_log->Log(logDebug, 5, _T("clrln %d from line %d, to line %d\n"),
								j, iline, eline);
#endif
							icol = 1;
							m_buffer->Delete(iline, icol, j);
							uv |= vtrcDISPLAY | vtrcSCREEN;
						}
						else
						{
#ifdef LOG_VT
							m_log->Log(logDebug, 5, _T("clrln none from line %d to %d\n"),
								iline, eline);
#endif
						}
						break;
					}
					break;

				case vtactHorzPad:

					icol = m_vtcol;
					for(m_tokcnt = 0; m_tokcnt < (m_action.f_parms[0]) && m_tokcnt < (sizeof(m_intok)/sizeof(TCHAR) - 1); m_tokcnt++)
					{
						m_intok[m_tokcnt] = ' ';
					}
					InsertToken(true);
					nonwhite = false;
					UpdateVtPos(m_vtrow - m_frameline + 1, icol/* + m_action.f_parms[0]*/);
					uv |= vtrcDISPLAY | vtrcLINE;
					break;

				case vtactVertPad:

					iline = m_vtrow;
					icol  = m_vtcol;

					lines = m_action.f_parms[0];
#ifdef LOG_VT
					m_log->Log(logDebug, 5, _T("padln insert %d at %d\n"),
						lines, m_vtrow, lines);
#endif
					while(lines > 0)
					{
						for(
								m_tokcnt = 0;
								lines > 0 && m_tokcnt < (sizeof(m_intok)/sizeof(TCHAR) - 1);
								m_tokcnt++,
								lines--
						)
						{
							m_intok[m_tokcnt] = '\n';
						}
						m_vtcol = 1;
						InsertToken(true);
					}
					m_vtcol = icol;
					m_vtrow = iline;
					nonwhite = false;

					// if padding lines inside a scroll region, remove that many from
					// bottom of scroll region
					//
					if (m_vtrow >= (m_scrlt + m_frameline - 1) && m_vtrow < (m_scrlb + m_frameline))
					{
						int xcol = m_vtcol;
						int xrow = m_vtrow;

						lines = m_action.f_parms[0];
#ifdef LOG_VT
						m_log->Log(logDebug, 5, _T("padln cut end of scroll reg %d by %d\n"),
							m_scrlb + m_frameline - 1, lines);
#endif
						while(lines > 0)
						{
							m_vtrow = m_scrlb + m_frameline - 1;
							m_vtcol = 1;

							ec = m_buffer->GetLineText(m_vtrow, lpText, nLineText);
							if (ec == errOK)
								m_buffer->Delete(m_vtrow, m_vtcol, nLineText);
							lines--;
						}
						m_vtcol = xcol;
						m_vtrow = xrow;
					}
					//UpdateVtPos(iline - m_frameline + lines + 1, 1);
					UpdateVtPos(iline - m_frameline + 1, icol);
					uv |= vtrcDISPLAY | vtrcRESTOFSCR;
					break;

				case vtactLineHeight:
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("lineheight\n"));
#endif
					break;

				case vtactLineWidth:
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("linewidth\n"));
#endif
					break;

				case vtactSavePos:
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("save pos\n"));
#endif
					m_vvtrow = m_vtrow;
					m_vvtcol = m_vtcol;
					break;

				case vtactRestorePos:
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("restore pos\n"));
#endif
					m_vtrow = m_vvtrow;
					m_vtcol = m_vvtcol;
					UpdateVtPos(m_vtrow - m_frameline + 1, m_vtcol);
					break;

				case vtactMode:
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("mode +%04X -%04X\n"),
							m_action.f_parms[0], m_action.f_parms[1]);
#endif
					m_vtmode |= m_action.f_parms[0];
					m_vtmode &=~m_action.f_parms[1];
					break;

				case vtactIdentify:
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("identify\n"));
#endif
					break;

				case vtactSetScroll:
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("setscrollregion: %d %d\n"), m_action.f_parms[0], m_action.f_parms[1]);
#endif
					m_scrlt = m_action.f_parms[0];
					m_scrlb = m_action.f_parms[1];

					// setting the scroll bottom to the end of screen is essentially removing it
					if (m_scrlt == 1 && (m_scrlb > m_vtrows || m_scrlb == 24))
					{
						// Note hack for 1:24 which means "full screen" for the lowest forms of
						// apps that run in VT mode
#ifdef LOG_VT
						m_log->Log(logDebug, 4, _T("setscrollregion: bottom at end of screen, remove sr\n"));
#endif
						m_scrlb = 0;
						m_scrlt = 1;
						break;
					}
					break;

				case vtactScroll:

					iline = m_vtrow;
					icol  = m_vtcol;

					lines = m_action.f_parms[0];
					if(m_scrlt <= 0 || m_scrlb < m_scrlt)
					{
						if(lines <= 0)
							break;
					}
					if(lines == 0)
						break;
					if (lines > (m_scrlb - m_scrlt - 1))
						lines = (m_scrlb - m_scrlt - 1);
					if(lines == 0)
						break;
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("scroll: %d do: %d\n"),
						m_action.f_parms[0], lines);
#endif
					// build string of newlines for inserting
					i = lines;
					if(i < 0)
						i = -i;
					for(
							m_tokcnt = 0;
							m_tokcnt < i && m_tokcnt < (sizeof(m_intok)/sizeof(TCHAR) - 1);
							m_tokcnt++
					)
					{
						m_intok[m_tokcnt] = '\n';
					}
					j = m_tokcnt;

					// insert blank lines to scroll
					i = lines;
					if(i < 0)
						i = -i;
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("scroll: insert %d at %d\n"),
						i, m_vtrow);
#endif
					while(i > 0)
					{
						if (lines < 0)
						{
							// scroll down: insert at top, delete from bottom
							if(m_scrlt > 0 && m_scrlb >= m_scrlt)
								m_vtrow = m_scrlt + m_frameline - 1;
						}
						else
						{
							// scroll up: insert at bottom, delete from top
							if(m_scrlt > 0 && m_scrlb >= m_scrlt)
								m_vtrow = m_scrlb + m_frameline;
						}
						m_vtcol = 1;
						InsertToken(true);
						i-= j;
						m_tokcnt = min(i, j);
					}
					m_tokcnt = 0;

					i = lines;
					if(i < 0)
						i = -i;
					while(i > 0)
					{
						if (lines < 0)
						{
							// scroll down: insert at top, delete from bottom
							if(m_scrlt > 0 && m_scrlb >= m_scrlt)
								m_vtrow = m_scrlb + m_frameline;
						}
						else
						{
							// scroll up: insert at bottom, delete from top
							if(m_scrlt > 0 && m_scrlb >= m_scrlt)
								m_vtrow = m_scrlt + m_frameline - 1;
						}
						m_vtcol = 1;
#ifdef LOG_VT
						m_log->Log(logDebug, 4, _T("scroll: del at %d\n"), m_vtrow);
#endif
						ec = m_buffer->GetLineText(m_vtrow, lpText, nLineText);
						if (ec == errOK)
							m_buffer->Delete(m_vtrow, m_vtcol, nLineText);
						i--;
					}

					m_vtcol = icol;
					m_vtrow = iline;
					nonwhite = false;

					uv |= vtrcDISPLAY | vtrcRESTOFSCR;
					break;

				case vtactSetTab:
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("set tab\n"));
#endif
					break;

				case vtactClearTab:
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("clear tab\n"));
#endif
					break;

				case vtactReset:
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("reset\n"));
#endif
					break;

				case vtactPosReport:
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("report position %d,%d\n"), m_vtrow, m_vtcol);
#endif
					{
						BYTE response[64];
						int  replen;

						replen = _snprintf((char*)response, sizeof(response),
								"%c[%d;%dR", 27, m_vtrow, m_vtcol);
						SendData(response, replen);
					}
					break;

				case vtactStatReport:
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("report status\n"));
#endif
					{
						BYTE response[64];
						int  replen;

						replen = _snprintf((char*)response, sizeof(response), "%c[%dn", 27, 0);
						SendData(response, replen);
					}
					break;

				case vtactBackSpace:
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("backspace at %d,%d\n"), m_vtrow, m_vtcol);
#endif
					if(m_vtcol > 1)
					{
						UpdateVtPos(m_vtrow - m_frameline + 1, m_vtcol - 1);
						uv |= vtrcDISPLAY | vtrcPOSITION;
					}
					break;

				case vtactTab:
					{
						int tabstop;

						tabstop = 1 + (((m_vtcol - 1) / m_tabspace) * m_tabspace) + m_tabspace;
#ifdef LOG_VT
						m_log->Log(logDebug, 4, _T("tab from %d to %d ts=%d\n"), m_vtcol, tabstop, m_tabspace);
#endif
						UpdateVtPos(m_vtrow - m_frameline + 1, tabstop);
						uv |= vtrcDISPLAY | vtrcPOSITION;
					}
					break;

				case vtactRetitle:
					{
						TCHAR tbuf[128 + 32];

						// title is string up to ';' after \07
						for(i = 0, j = n; j > 0 && i < 127; i++)
						{
							if(m_szInp[--j] == _T(';'))
								break;
						}
						if(m_szInp[j] == _T(';'))
						{
							_tcscpy(tbuf, _T("shell-"));
							_tcsncpy(tbuf+6, m_szInp + j + 1, i);
							tbuf[i+6] = _T('\0');
							if(_tcscmp(GetName(), tbuf))
							{
								SetName(tbuf);
								m_buffer->SetName(tbuf);

								if(m_editor->GetCurrentView() == this)
								{
									BpickInfo* pPick;

									m_editor->SetTitle(m_buffer);

									if((pPick = (BpickInfo*)m_editor->GetPane(_T("Picker"), _T("File"))) != NULL)
										pPick->SetView(this);
								}
							}
						}

					}
					break;

				case vtactGxSELECT:
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("use chars from G%d\n"), m_action.f_parms[0]);
#endif
					if(m_action.f_parms[0] >= 0 && m_action.f_parms[0] < MAX_VT_CHARMODES)
						m_charset = m_action.f_parms[0];
					break;

				case vtactGxSET:
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("apply charset %d to G%d\n"), m_action.f_parms[1], m_action.f_parms[0]);
#endif
					if(m_action.f_parms[0] >= 0 && m_action.f_parms[0] < MAX_VT_CHARMODES)
						m_charSets[m_action.f_parms[0]] = (BvtCharSet)m_action.f_parms[1];
					break;

				case vtactDECmodeSet:
				case vtactDECmodeReset:

					for(i = 0; i < m_action.f_parmnum; i++)
					{
#ifdef LOG_VT
						m_log->Log(logDebug, 4, _T("DEC MODE %d %s %d\n"), i, (m_action.f_action == vtactDECmodeSet ? "Set" : "Reset"), m_action.f_parms[i]);
#endif
						switch(m_action.f_parms[i])
						{
						case 1:	// app cursors
						case 2:	// reset all charsets and set vt100 mode
						case 3: // 132 col mode
						case 4:	// smooth scroll
						case 5:	// reverse video
						case 6:	// origin mode
						case 7:	// wrap mode
						case 8: // auto repeat mode
						case 9:	// x,y mouse track
						case 18:// print form feed
						case 19:// print extend full screen
						case 25:// show cursor
						case 30:// show scroll bar
						case 35:// enable shifted key funcs
						case 38:// enter tek mode
						case 40:// allow 80<>132 mode
						case 41:// more(1) fix
						case 42:// enable nat rep chars
						case 44:// turn on margin bell
						case 45:// reverse wrap mode
						case 46:// start logging
						case 47:// use alt screen buffer
						case 66:// app keyboard
						case 67:// backarrow key sends delete
						case 1000:// send mouse x,y on button press
						case 1002:// use cell motion mouse
						case 1003:// use all motion mouse
						case 1010://scroll to bottom on tty out
						case 1011:// scroll to bottom on keypress
						case 1035:// enable mods for alt and numlock
						case 1036:// send ESC when modfied key
						case 1037:// send DEL from edit pad Delete
						case 1047:// use alter screen buffer
						case 1048:// save cursor
						case 1049:// save cursor and use alter screen buffer
						case 1051:// set Sun func kbd
						case 1052:// set HP func kbd
						case 1060:// set legacy kbd
						case 1061:// set Sun/PC kbd emul of VT220
						default:
							break;
						}
					}
					break;

				case vtactIgnore:
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("ignoring VT esc %d\n"), m_action.f_action);
#endif
					break;

				default:
#ifdef LOG_VT
					m_log->Log(logDebug, 4, _T("not handling VT esc %d\n"), m_action.f_action);
#endif
					break;
				}
			}
		}
		if(m_tokcnt > 0 && (m_vtState == vtpsDONE || m_vtState == vtpsINIT))
		{
			InsertToken(false);
		}
#if 1
		// udpate the view if display-wide change or smallish insertion
		// (very large insertions wait for quiet to avoid many redraws)
		//
		if(((m_insertcount > 0) && (m_insertcount < 100)) || (uv & vtrcDISPLAY))
		{
			m_insertcount = 400000;
			CheckFrame(uv & (vtrcSCREEN | vtrcRESTOFSCR) ? true : false);
		}
#endif
	}
	// redraw if pending insertions and no show
	//
	if(m_insertcount > 0)
	{
		m_insertcount = 400000;
		CheckFrame(uv & (vtrcSCREEN | vtrcRESTOFSCR) ? true : false);
	}
}


// STATIC
//**************************************************************************
void CALLBACK BviewTerminal::OnTimer(HWND hWnd, UINT msg, UINT id, DWORD now)
{
	BviewTerminal* pShell = (BviewTerminal*)GetWindowLong(hWnd, GWL_USERDATA);

	if(pShell)
	{
		pShell->Event(0);
	}
}

//**************************************************************************
ERRCODE BviewTerminal::SendData(LPBYTE data, int len)
{
	return errOK;
}

//**************************************************************************
ERRCODE BviewTerminal::Key(WORD key)
{
	EditCommand command;
	int 		keylen;
	char 		keybuf[32];
	char*		vtStr = NULL;

	keybuf[0] = (char)key;
	keybuf[1] = 0;

	command = m_keyBindings->Translate(key);

	// convert scroll keys to term escapes
	//
	if(1 /*m_vtmode & vtmdANSI*/)
	{
		switch(key)
		{
		case keyCursorLeft:
			vtStr = (char*)((m_vtmode & vtmdKEYPAD) ? "\033OD" : "\033[D");
			break;
		case keyCursorRight:
			vtStr = (char*)((m_vtmode & vtmdKEYPAD) ? "\033OC" : "\033[C");
			break;
		case keyCursorUp:
			vtStr = (char*)((m_vtmode & vtmdKEYPAD) ? "\033OA" : "\033[A");
			break;
		case keyCursorDown:
			vtStr = (char*)((m_vtmode & vtmdKEYPAD) ? "\033OB" : "\033[B");
			break;
		case keyCursorHome:
			vtStr = (char*)"\033[H";
			break;

		case keyCursorPrev:
		case keyCursorNext:
		case keyCursorEnd:
		case keyCursorPageUp:
		case keyCursorUpLeft:
		case keyCursorDownLeft:
		case keyCursorPageDown:
			return Dispatch(command);
			break;

		default:
			break;
		}
	}
	if(! vtStr)
	{
		vtStr = keybuf;
	}
	keylen = strlen(vtStr);
	if(InsertPointVisible() || command == SelfInsert)
	{
		MoveAbs(m_vtrow, m_vtcol);
		if(m_vtrow < m_topline || m_vtrow > (m_topline + GetViewLines()))
			RecenterView();
	}
	if(m_vtmode & vtmdECHO)
	{
		int sl, sc;
		int len;

		// insert any pending data
		if (m_tokcnt)
			InsertToken(true);
		// copy keybuffer to token
		m_tokcnt = strlen(vtStr);
		for (len = 0; len < m_tokcnt && len < sizeof(m_intok)/sizeof(TCHAR); len++)
			m_intok[len] = (TCHAR)vtStr[len];
		sl = m_vtrow;
		sc = m_vtcol;
		InsertToken(true);
		UpdateVtPos(m_vtrow - m_frameline + 1, m_vtcol);
		if(InsertPointVisible())
		{
			UpdateView(
						sl,
						sl == m_vtrow ? (sc + 2) / 3 : 1,
						m_vtrow,
						sl == m_vtrow ? m_vtcol : MAX_LINE_LEN
					  );
		}
	}
#ifdef xxLOG_VT
	{
		int i;

		for(i = 0; i < keylen; i++)
		{
			_tprintf(_T("%02X (%c)\n"), vtStr[i], vtStr[i]);
		}
	}
#endif
	SendData((LPBYTE)vtStr, keylen);
	return errOK;

}

typedef struct tag_emparm
{
	TCHAR			f_typename[128];
	Bed*			f_peditor;
	BviewTerminal*	f_pterm;
}
EMPARM, *PEMPARM;

// STATIC
//**************************************************************************
static BOOL CALLBACK VTtermproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PEMPARM		pem = NULL;

	Bpersist*	pp;
	HWND		hwndParent;
	RECT		rc, rcme;
	TCHAR		pparm[MAX_PATH + 256];
	LPTSTR		px;
	int			ppl, bdrc;
	PARMPASS	dp;

	static bool crnl, nobell, wrap, echo, log;
	static int emul;
	static TCHAR logdir[MAX_PATH + 2];

	pem = (PEMPARM)GetWindowLong(hWnd, GWL_USERDATA);

	switch(message)
	{
	case WM_INITDIALOG:

		pem = (PEMPARM)lParam;
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)pem);
		hwndParent = GetParent(hWnd);
		if(! hwndParent) hwndParent = GetDesktopWindow();
		GetClientRect(hwndParent, &rc);
		GetWindowRect(hWnd, &rcme);
		{
			int x = ((rc.right - rc.left) - (rcme.right - rcme.left)) / 2;
			int y = ((rc.bottom - rc.top) - (rcme.bottom - rcme.top)) / 2;
			int w = rcme.right - rcme.left;
			int h = rcme.bottom - rcme.top;

			if(x < 0) x = 0;
			if(y < 0) y = 0;

			MoveWindow(hWnd, x, y, w, h, FALSE);
		}
		if(! pem)
		{
			EndDialog(hWnd, IDCANCEL); break;
		}
		if(pem->f_pterm)
		{
			crnl	= (pem->f_pterm->GetMode() & vtmdLFNL) != 0;
			nobell	= (pem->f_pterm->GetMode() & vtmdBEEP) != 0;
			wrap	= (pem->f_pterm->GetMode() & vtmdWRAP) != 0;
			echo	= (pem->f_pterm->GetMode() & vtmdECHO) != 0;
			log		= (pem->f_pterm->GetMode() & vtmdLOG)  != 0;

			emul	= pem->f_pterm->GetEmulation();
		}
		else
		{
			crnl	= false;
			nobell	= false;
			wrap	= false;
			echo	= false;
			log		= false;

			emul	= emulXTERM;
		}
		BUtil::GetHomePath(logdir, MAX_PATH);

		pp = pem->f_peditor->GetPersist();
		if(pp)
		{
			ppl = _sntprintf(pparm, 128, _T("" _Pfs_ "/Emulation/Settings/"), pem->f_typename);
			px = pparm + ppl;

			_tcscpy(px, _T("Emulation"));
			pp->GetNvInt(pparm, (int&)emul, emul);
			_tcscpy(px, _T("Echo"));
			pp->GetNvBool(pparm, echo, echo);
			_tcscpy(px, _T("LineWrap"));
			pp->GetNvBool(pparm, wrap, wrap);
			_tcscpy(px, _T("TreatCRasNL"));
			pp->GetNvBool(pparm, crnl, crnl);
			_tcscpy(px, _T("NoBell"));
			pp->GetNvBool(pparm, nobell, nobell);
			_tcscpy(px, _T("LogToFile"));
			pp->GetNvBool(pparm, log, log);
			_tcscpy(px, _T("LogFileDirectory"));
			pp->GetNvStr(pparm, logdir, MAX_PATH, logdir);
		}
		CheckDlgButton(hWnd, IDC_VTNONE,  emul == emulNONE	? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_VT100,   emul == emulVT100	? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_VT102,   emul == emulVT102	? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_VT220,   emul == emulVT220	? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_VTXTERM, emul == emulXTERM	? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hWnd, IDC_CRNL,   (crnl)		? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_ECHO,   (echo)		? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_WRAP,   (wrap)		? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_QUIET,  (nobell)	? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hWnd, IDC_LOGGING,(log)		? BST_CHECKED : BST_UNCHECKED);

		EnableWindow(GetDlgItem(hWnd, IDC_LOGPATH), log);

		return TRUE;

	case WM_COMMAND:

		switch(wParam)
		{
		case IDOK:

			if(! pem) { EndDialog(hWnd, IDCANCEL); break; }

			if(IsDlgButtonChecked(hWnd, IDC_VTNONE))
				emul = emulNONE;
			else if(IsDlgButtonChecked(hWnd, IDC_VT100))
				emul = emulVT100;
			else if(IsDlgButtonChecked(hWnd, IDC_VT102))
				emul = emulVT102;
			else if(IsDlgButtonChecked(hWnd, IDC_VT220))
				emul = emulVT220;
			else if(IsDlgButtonChecked(hWnd, IDC_VTXTERM))
				emul = emulXTERM;
			else
				emul = emulXTERM;

			crnl 	= IsDlgButtonChecked(hWnd, IDC_CRNL) != 0;
			echo 	= IsDlgButtonChecked(hWnd, IDC_ECHO) != 0;
			wrap 	= IsDlgButtonChecked(hWnd, IDC_WRAP) != 0;
			nobell 	= IsDlgButtonChecked(hWnd, IDC_QUIET) != 0;
			log		= IsDlgButtonChecked(hWnd, IDC_LOGGING) != 0;

			pp = pem->f_peditor->GetPersist();
			if(pp)
			{
				ppl = _sntprintf(pparm, 128, _T("" _Pfs_ "/Emulation/Settings/"), pem->f_typename);
				px = pparm + ppl;

				_tcscpy(px, _T("Emulation"));
				pp->SetNvInt(pparm, emul);

				_tcscpy(px, _T("Echo"));
				pp->SetNvBool(pparm, echo);
				_tcscpy(px, _T("LineWrap"));
				pp->SetNvBool(pparm, wrap);
				_tcscpy(px, _T("TreatCRasNL"));
				pp->SetNvBool(pparm, crnl);
				_tcscpy(px, _T("NoBell"));
				pp->SetNvBool(pparm, nobell);
				_tcscpy(px, _T("LogToFile"));
				pp->SetNvBool(pparm, log);

				if(log)
				{
					_tcscpy(px, _T("LogFileDirectory"));
					pp->SetNvStr(pparm, logdir);
				}
				if(pem->f_pterm)
				{
					pem->f_pterm->SetEmulation((BvtEmulation)emul);

					pem->f_pterm->ClearModeBit(0xFFFF);
					if(crnl)
						pem->f_pterm->SetModeBit(vtmdLFNL);
					if(wrap)
						pem->f_pterm->SetModeBit(vtmdWRAP);
					if(nobell)
						pem->f_pterm->SetModeBit(vtmdBEEP);
					if(echo)
						pem->f_pterm->SetModeBit(vtmdECHO);
					if(log)
						pem->f_pterm->SetModeBit(vtmdLOG);
				}
			}
			EndDialog(hWnd, wParam);
			break;

		case IDCANCEL:

			EndDialog(hWnd, wParam);
			break;

		case IDC_LOGGING:

			log = IsDlgButtonChecked(hWnd, IDC_LOGGING) != 0;
			EnableWindow(GetDlgItem(hWnd, IDC_LOGPATH), log);
			break;

		case IDC_LOGPATH:

			dp.lpTitle	 = _T("BED 6.0 - Terminal Log File Directory");
			dp.lpString  = logdir;
			dp.nString   = MAX_PATH;
			dp.lpInitVal = logdir;

			bdrc = PickDirectoryDialog(&dp, GetParent(hWnd));
			if(bdrc == IDOK)
			{
				bdrc = _tcslen(dp.lpString);
				if(bdrc > 0)
				{
					if(dp.lpString[bdrc-1] != _PTC_)
					{
						dp.lpString[bdrc++] = _PTC_;
						dp.lpString[bdrc] = _T('\0');
					}
				}
				else
				{
					bdrc = 0;
					dp.lpString[bdrc++] = _T('.');
					dp.lpString[bdrc++] = _PTC_;
					dp.lpString[bdrc] = _T('\0');
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
	return FALSE;
}

//**************************************************************************
int BviewTerminal::EmulationDialog(Bed* pBed, BviewTerminal* pTerm, LPCTSTR pType, HWND hWndParent)
{
	EMPARM pp;

	_tcsncpy(pp.f_typename, pType, 127);
	pp.f_peditor = pBed;
	pp.f_pterm	 = pTerm;

	return DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_EMULSET), hWndParent, VTtermproc, (LPARAM)&pp);
}

//**************************************************************************
int BviewTerminal::EmulationDialog(BviewTerminal* pTerm, HWND hWndParent)
{
	return EmulationDialog(pTerm->GetEditor(), pTerm, pTerm->GetTypeName(), hWndParent);
}

