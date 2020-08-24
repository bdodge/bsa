
#include "bcalcx.h"


#define PROG_BUT_MAP_W	1
#define PROG_BUT_MAP_H	11

BOOL WINAPI FileIoProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

BCbutton g_progbuttons[PROG_BUT_MAP_H][PROG_BUT_MAP_W] =
{
	{
		{ ID_PRG_REC,	_T("Rec"), 		BUT_DIG_W, BUT_DIG_H, bcModeButton },
	},
	{
		{ ID_PRG_RUN,	_T("Run"),		BUT_DIG_W, BUT_DIG_H, bcProgram },
	},
	{
		{ ID_PRG_PLOT2,	_T("2D"), 		BUT_DIG_W, BUT_DIG_H, bcProgram },
	},
	{
		{ ID_PRG_PLOT3,	_T("3D"), 		BUT_DIG_W, BUT_DIG_H, bcProgram },
	},
	{
		{ ID_PRG_STOP,	_T("Stop"),		BUT_DIG_W, BUT_DIG_H, bcProgram },
	},
	{
		{ ID_PRG_STEP,	_T("Step"),		BUT_DIG_W, BUT_DIG_H, bcProgram },
	},
	{
		{ ID_PRG_INS,	_T("Ins"),		BUT_DIG_W, BUT_DIG_H, bcProgram },
	},
	{
		{ ID_PRG_DEL,	_T("Del"),		BUT_DIG_W, BUT_DIG_H, bcProgram },
	},
	{
		{ ID_PRG_CLR,	_T("Clr"),		BUT_DIG_W, BUT_DIG_H, bcProgram },
	},
	{
		{ ID_PRG_LOAD,	_T("Load"),		BUT_DIG_W, BUT_DIG_H, bcProgram },
	},
	{
		{ ID_PRG_SAVE,	_T("Save"),		BUT_DIG_W, BUT_DIG_H, bcProgram }
	}
};

//**************************************************************************
Bprog::Bprog(Bcalc* pCalc, BappPanel* pParent)
		:
	BappPane(_T("Program"), pParent),
	m_bcalc(pCalc),
	m_prog(NULL),
	m_cur(NULL),
	m_err(NULL),
	m_yoff(0),
	m_th(16),
	m_wh(16)
{
	m_filename[0] = _T('\0');
}

//**************************************************************************
Bprog::~Bprog()
{
	PPRGENTRY pe, px;

	for(pe = m_prog; pe;)
	{
		px = pe->m_next;
		delete pe;
		pe = px;
	}
}

//**************************************************************************
void Bprog::SetEditScroller()
{
	SCROLLINFO	sinfo;
	PPRGENTRY	pe;
	int			i, j, k;
	int			y;

	for(pe = m_prog, i = j = k = y = 0; pe; pe = pe->m_next, i++, y += m_th)
	{
		if(pe == m_cur)
		{
			j = i;
			if(y < m_yoff)
			{
				m_yoff = y;
				k = 1;
			}
			else if(y > m_yoff + m_wh - m_th)
			{
				m_yoff = y - m_wh + m_th;
				k = 1;
			}
		}
	}
	sinfo.cbSize	= sizeof(SCROLLINFO);
	sinfo.fMask		= SIF_PAGE | SIF_POS | SIF_RANGE;
	sinfo.nMin		= 1;
	sinfo.nMax		= i;
	sinfo.nPage		= m_wh / (m_th ? m_th : 16);
	sinfo.nPos		= j;

	SetScrollInfo(m_hwndEdit, SB_VERT, &sinfo, TRUE);
	if(k) InvalidateRect(m_hwndEdit, NULL, TRUE);
}

//**************************************************************************
ERRCODE Bprog::Insert(Bnumber* pn)
{
	PPRGENTRY pe;

	pe = new PRGENTRY(pn);
	if(! m_cur)
	{
		m_cur = m_prog = pe;
		pe->m_prev = pe->m_next = NULL;
	}
	else
	{
		pe->m_prev = m_cur;
		pe->m_next = m_cur->m_next;
		m_cur->m_next = pe;
		if(pe->m_next)
			pe->m_next->m_prev = pe;
		m_cur = pe;
	}
	SetEditScroller();
	InvalidateRect(m_hwndEdit, NULL, FALSE);
	return errOK;
}

//**************************************************************************
ERRCODE Bprog::Insert(CODE o)
{
	PPRGENTRY pe;

	pe = new PRGENTRY(o);
	if(! m_cur)
	{
		m_cur = m_prog = pe;
		pe->m_prev = pe->m_next = NULL;
	}
	else
	{
		pe->m_prev = m_cur;
		pe->m_next = m_cur->m_next;
		m_cur->m_next = pe;
		if(pe->m_next)
			pe->m_next->m_prev = pe;
		m_cur = pe;
	}
	SetEditScroller();
	InvalidateRect(m_hwndEdit, NULL, FALSE);
	return errOK;
}

//**************************************************************************
ERRCODE Bprog::InsertControl()
{
	HMENU	hMenu;
	int		rc;
	POINT   mousepos;

	PPRGENTRY pe;

	GetCursorPos(&mousepos);

	hMenu = CreatePopupMenu();

	AppendMenu(hMenu, 0, ID_PRG_LOOP,	CodeName(ID_PRG_LOOP));
	AppendMenu(hMenu, 0, ID_PRG_POOL,	CodeName(ID_PRG_POOL));
	AppendMenu(hMenu, 0, ID_PRG_BREAK,	CodeName(ID_PRG_BREAK));

	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

	AppendMenu(hMenu, 0, ID_PRG_LT,	CodeName(ID_PRG_LT));
	AppendMenu(hMenu, 0, ID_PRG_GT,	CodeName(ID_PRG_GT));
	AppendMenu(hMenu, 0, ID_PRG_LE,	CodeName(ID_PRG_LE));
	AppendMenu(hMenu, 0, ID_PRG_GE,	CodeName(ID_PRG_GE));
	AppendMenu(hMenu, 0, ID_PRG_EQ,	CodeName(ID_PRG_EQ));
	AppendMenu(hMenu, 0, ID_PRG_FI,	CodeName(ID_PRG_FI));

#ifdef Windows
	SetForegroundWindow(m_hwnd);
#endif
	
	rc = TrackPopupMenu(
						hMenu, 
						TPM_RETURNCMD, 
						mousepos.x,
						mousepos.y,
						0,
						m_hwnd,
						NULL
					   );
                       
	DestroyMenu(hMenu);

	pe = new PRGENTRY(rc);
	if(! m_cur)
	{
		m_cur = m_prog = pe;
		pe->m_prev = pe->m_next = NULL;
	}
	else
	{
		pe->m_prev = m_cur;
		pe->m_next = m_cur->m_next;
		m_cur->m_next = pe;
		if(pe->m_next)
			pe->m_next->m_prev = pe;
		m_cur = pe;
	}
	SetEditScroller();
	InvalidateRect(m_hwndEdit, NULL, FALSE);
	return errOK;
}

//**************************************************************************
ERRCODE Bprog::Delete()
{
	if(! m_cur) return errOK;

	if(m_cur == m_prog)
	{
		m_prog = m_cur->m_next;
		if(m_prog)
			m_prog->m_prev = NULL;
		delete m_cur;
		m_cur = m_prog;
	}
	else
	{
		PPRGENTRY pn;

		m_cur->m_prev->m_next = pn = m_cur->m_next;
		if(m_cur->m_next)
			m_cur->m_next->m_prev = m_cur->m_prev;
		else
			pn = m_cur->m_prev;
		delete m_cur;
		m_cur = pn;
	}
	SetEditScroller();
	InvalidateRect(m_hwndEdit, NULL, FALSE);
	return errOK;
}

//**************************************************************************
ERRCODE Bprog::ClearProgram()
{
	PPRGENTRY px;

	for(m_cur = m_prog; m_cur;)
	{
		px = m_cur->m_next;
		delete m_cur;
		m_cur = px;
	}
	m_prog = m_cur = NULL;
	SetEditScroller();
	InvalidateRect(m_hwndEdit, NULL, FALSE);
	return errOK;
}

//**************************************************************************
ERRCODE Bprog::RunProgram(bool issub)
{
	PPRGENTRY px;
	ERRCODE   ec;

	if(! m_prog) return errFAILURE;

	if(! issub)
	{
		m_bcalc->SetProgramMode(pmRun);
		SetupWindows(false);
	}
	px = m_cur;
	m_cur = m_prog;
	m_err = NULL;

	do
	{
		ec = StepProgram(true);
	}
	while(ec == errOK && m_cur != m_prog && m_bcalc->GetProgramMode() == pmRun);

	if(ec != errOK)
		m_err = m_cur;
	else
		m_cur = px;

	if(! issub)
	{
		m_bcalc->SetProgramMode(pmEdit);
		SetupWindows(false);
		InvalidateRect(m_hwndEdit, NULL, FALSE);
	}
	return ec;
}

//**************************************************************************
bool Bprog::EvalCondition(CODE cond)
{
	Bnumber* pn = m_bcalc->GetTop();

	if(! pn)
	{
		switch(cond)
		{
		case ID_PRG_LE:
		case ID_PRG_GE:
		case ID_PRG_EQ:
			return true;

		case ID_PRG_LT:
		case ID_PRG_GT:
		default:
			return false;
		}
	}
	else
	{
		switch(cond)
		{
		case ID_PRG_LT:
			return pn->IsNegative();

		case ID_PRG_GT:
			return ! pn->IsNegative();

		case ID_PRG_LE:
			return pn->IsNegative() || pn->IsZero();

		case ID_PRG_GE:
			return (! pn->IsNegative()) || pn->IsZero();

		case ID_PRG_EQ:
				return pn->IsZero();

		default:
			return false;
		}
	}
}

//**************************************************************************
ERRCODE Bprog::StepProgram(bool issub)
{
	PPRGENTRY px, pz;
	ERRCODE   ec;
	bool	  exec;
	MSG		  msg;
	int		  mc;

	if(! m_prog || ! m_cur) return errFAILURE;

	if(! issub)
	{
		m_bcalc->SetProgramMode(pmRun);
	}
	px = m_cur;
	ec = errOK;
	if(px->IsNumber())
	{
		ec = m_bcalc->Play(px->m_num);
		if(ec == errOK)
			px = px->m_next;
	}
	else if(px->IsCode())
	{
		if(px->m_code >= ID_PRG_LOOP && px->m_code <= ID_PRG_FI)
		{
			switch(px->m_code)
			{
			case ID_PRG_LOOP:
			case ID_PRG_FI:
				px = px->m_next;
				break;
			case ID_PRG_POOL:
				for(pz = px; pz->m_prev; pz = pz->m_prev)
				{
					if(pz->IsCode() && pz->m_code == ID_PRG_LOOP)
					{
						px = pz;
						break;
					}
				}
				if(px != pz)
				{
					// missing loop
					px = pz;
				}
				break;
			case ID_PRG_BREAK:
				for(pz = px; pz->m_next; pz = pz->m_next)
				{
					if(pz->IsCode() && pz->m_code == ID_PRG_POOL)
					{
						px = pz->m_next;
						break;
					}
				}
				if(px != pz->m_next)
				{
					// missing pool
					px = pz->m_next;
				}
				break;
			case ID_PRG_LT:
			case ID_PRG_GT:
			case ID_PRG_LE:
			case ID_PRG_GE:
			case ID_PRG_EQ:
				exec = EvalCondition(px->m_code);
				if(exec)
				{
					px = px->m_next;
				}
				else
				{
					for(pz = px; pz->m_next; pz = pz->m_next)
					{
						if(pz->IsCode() && pz->m_code == ID_PRG_FI)
						{
							px = pz->m_next;
							break;
						}
					}
					if(px != pz->m_next)
					{
						// missing fi
						px = pz->m_next;
					}
				}
				break;
			default:
				px = px->m_next;
				break;
			}
		}
		else
		{
			ec = m_bcalc->Play(px->m_code);
			if(ec == errOK)
				px = px->m_next;
		}
	}
	if(px)
		m_cur = px;
	else
		m_cur = m_prog;

	for(mc = 0; mc < 40 && PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE); mc++)
	{
		if(GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	if(! issub)
	{
		m_bcalc->SetProgramMode(pmEdit);
		InvalidateRect(m_hwndEdit, NULL, FALSE);
	}
	else
	{
		if(m_bcalc->GetProgramMode() != pmRun)
			return errFAILURE;
	}
	return ec;
}

//**************************************************************************
ERRCODE Bprog::PlotProgram(int d)
{
	Bplot*  pplt;
	ERRCODE ec;

	double	sx, sy, ex, ey, xs, ys, x, y, z, xz, yz;
	int		dd;

	Bnumber* xx;
	Bnumber* yy;
	Bnumber* xxz;
	Bnumber* yyz;

	CHAR	 nb[128];

	pplt = NULL;

	if(m_bcalc && m_bcalc->GetPlotPanel())
		pplt = (Bplot*)m_bcalc->GetPlotPanel()->FindPane(_T("Plot"));
	if(! pplt) return errFAILURE;

	//if(! m_bcalc->GetPlotPanel())
	//	return errFAILURE;

	m_bcalc->SetProgramMode(pmRun);
	SetupWindows(false);

	pplt->GetPlotExtents(dd, sx, sy, ex, ey, xs, ys);
	pplt->StartPlot(d);
	m_bcalc->OnKey(ID_CTL_CLRA);

	ec = errOK;

	if(d == 3)
	{
		for(y = sy, xz = sx; y <= ey && ec == errOK; y += ys, xz += xs)
		{
			pplt->Anchor();

			_snprintf(nb, 128, "%.6f", y);
			yy = m_bcalc->MakeNumber(nb, m_bcalc->GetRadix(), kbDefault);

			for(x = sx; x <= ex && ec == errOK; x += xs)
			{
				_snprintf(nb, 128, "%.6f", x);
				xx = m_bcalc->MakeNumber(nb, m_bcalc->GetRadix(), kbDefault);

				m_bcalc->Store(xx, ID_STR_X);
				delete xx;
				m_bcalc->Store(yy, ID_STR_Y);
				ec = RunProgram(true);
				if(ec != errOK)
					break;
				//m_bcalc->Store(m_bcalc->GetTop(), ID_STR_Z);
				if(m_bcalc->GetTop())
					z = strtod(m_bcalc->GetTop()->Format(512, 10, kbDefault), NULL);
				else
					z = 0.0;
				pplt->PlotZ(x, y, z);
				m_bcalc->OnKey(ID_CTL_CLRA);
			}

			_snprintf(nb, 128, "%.6f", xz);
			xxz = m_bcalc->MakeNumber(nb, m_bcalc->GetRadix(), kbDefault);
			m_bcalc->Store(xxz, ID_STR_X);
			pplt->Anchor();

			for(yz = sy; yz <= ey && ec == errOK; yz += ys)
			{
				_snprintf(nb, 128, "%.6f", yz);
				yyz = m_bcalc->MakeNumber(nb, m_bcalc->GetRadix(), kbDefault);

				m_bcalc->Store(yyz, ID_STR_Y);
				delete yyz;
				ec = RunProgram(true);
				if(ec != errOK)
					break;
				//m_bcalc->Store(m_bcalc->GetTop(), ID_STR_Z);
				if(m_bcalc->GetTop())
					z = strtod(m_bcalc->GetTop()->Format(512, 10, kbDefault), NULL);
				else
					z = 0.0;
				pplt->PlotZ(xz, yz, z);
				m_bcalc->OnKey(ID_CTL_CLRA);
			}
			delete xxz;
			delete yy;
		}
	}
	else
	{
		pplt->Anchor();

		for(x = sx; x <= ex && ec == errOK; x += xs)
		{
			_snprintf(nb, 128, "%.6f", x);
			xx = m_bcalc->MakeNumber(nb, m_bcalc->GetRadix(), kbDefault);
			m_bcalc->Store(xx, ID_STR_X);
			delete xx;
			ec = RunProgram(true);
			if(ec != errOK)
				break;
			//m_bcalc->Store(m_bcalc->GetTop(), ID_STR_Y);
			if(m_bcalc->GetTop())
				y = strtod(m_bcalc->GetTop()->Format(512, 10, kbDefault), NULL);
			else
				y = 0.0;
			pplt->PlotY(x, y);
			m_bcalc->OnKey(ID_CTL_CLRA);
		}
	}
	m_bcalc->SetProgramMode(pmEdit);
	SetupWindows(false);
	InvalidateRect(m_hwndEdit, NULL, FALSE);
	return errOK;
}

//**************************************************************************
ERRCODE Bprog::StopProgram()
{
	m_bcalc->SetProgramMode(pmEdit);
	InvalidateRect(m_hwndEdit, NULL, FALSE);
	return errOK;
}

#ifdef UNICODE
#define GETFS fgetws
#define PUTFS fputws
#else
#define GETFS fgets
#define PUTFS fputs
#endif


//**************************************************************************
ERRCODE Bprog::LoadProgram()
{
	int		  rc;
	TCHAR	  tb[1024];
	CHAR	  xb[3096];
	LPTSTR	  pt, px;

	rc = OpenFileDialog(m_hInstance, _T("BCALC - Load Program File"), true, true, false, true, m_filename, MAX_PATH, NULL, FileIoProc);
	if(rc == IDOK)
	{
		FILE* pf;

#ifdef Windows
		if((pf = _tfopen(m_filename, _T("r"))) == NULL)
#else
		if((pf = bsafopen(m_filename, _T("r"))) == NULL)
#endif
		{
			MessageBox(NULL, m_filename, _T("BCALC - File Not Found"), MB_OK);
			return errFAILURE;
		}
		while((pt = GETFS(tb, 512, pf)) != NULL)
		{
			for(px = pt; *px; px++)
				if(*px == _T('\n') || *px == _T('\r'))
					*px = _T('\0');
			if(*pt >= _T('0') && *pt <= _T('9'))
			{
#ifdef UNICODE
				BUtil::Utf8Encode(xb, pt, false);
#else
				strcpy(xb, pt);
#endif
				Insert(m_bcalc->MakeNumber(xb, 0, ID_OPR_EQUAL));
			}
			else
			{
				Insert(CodeFromName(pt));
			}
		}
	}
	SetEditScroller();
	InvalidateRect(m_hwndEdit, NULL, FALSE);
	return errOK;
}

//**************************************************************************
ERRCODE Bprog::SaveProgram()
{
	PPRGENTRY px;
	int		  rc;
	TCHAR	  tb[1024];
	LPCTSTR	  pt;
	int		  textlen;

	if(! m_prog)
	{
		MessageBox(NULL, _T("No Program Recorded"), _T("BCALC - Save Program"), MB_OK);
		return errFAILURE;
	}
	rc = OpenFileDialog(m_hInstance, _T("BCALC - Save Program File"), false, false, false, true, m_filename, MAX_PATH, NULL, FileIoProc);
	if(rc == IDOK)
	{
		FILE* pf;

#ifdef Windows
		if((pf = _tfopen(m_filename, _T("w"))) == NULL)
#else
		if((pf = bsafopen(m_filename, _T("w"))) == NULL)
#endif
		{
			MessageBox(NULL, m_filename, _T("BCALC - Can't Create File"), MB_OK);
			return errFAILURE;
		}
		for(px = m_prog; px; px = px->m_next)
		{
			if(px->IsNumber() && px->m_num)
			{
				const char* pnum = px->m_num->Format(512, 10, kbDefault);
#ifdef UNICODE
				BUtil::Utf8Decode(tb, pnum);
#else
				strcpy(tb, pnum);
#endif
				pt = tb;
			}
			else if(px->IsCode())
			{
				pt = CodeName(px->m_code);
			}
			textlen = _tcslen(pt);
			PUTFS(pt, pf);
			PUTFS(_T("\n"), pf);
		}
		fclose(pf);
	}
	SetEditScroller();
	InvalidateRect(m_hwndEdit, NULL, FALSE);
	return errOK;
}

//**************************************************************************
ERRCODE Bprog::StopRecording()
{
	if(m_bcalc->GetProgramMode() == pmRecord)
	{
		SetButtonState(ID_PRG_REC, false);
		m_bcalc->SetProgramMode(pmEdit);
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bprog::MoveRel(int to)
{
	PPRGENTRY pe = m_cur;

	if(to > 0)
	{
		for(; m_cur && m_cur->m_next && to > 0; to--)
		{
			m_cur = m_cur->m_next;
		}
	}
	else if(to < 0)
	{
		for(; m_cur && m_cur->m_prev && to < 0; to++)
		{
			m_cur = m_cur->m_prev;
		}
	}
	if(m_cur != pe)
	{
		SetEditScroller();
		InvalidateRect(m_hwndEdit, NULL, TRUE);
	}
	return errOK;
}

//**************************************************************************
LPCTSTR Bprog::CodeName(CODE o)
{
	switch(o)
	{
	case ID_NUM_0:		return _T("0");
	case ID_NUM_1:		return _T("1");
	case ID_NUM_2:		return _T("2");
	case ID_NUM_3:		return _T("3");
	case ID_NUM_4:		return _T("4");
	case ID_NUM_5:		return _T("5");
	case ID_NUM_6:		return _T("6");
	case ID_NUM_7:		return _T("7");
	case ID_NUM_8:		return _T("8");
	case ID_NUM_9:		return _T("9");
	case ID_NUM_A:		return _T("A");
	case ID_NUM_B:		return _T("B");
	case ID_NUM_C:		return _T("C");
	case ID_NUM_D:		return _T("D");
	case ID_NUM_E:		return _T("E");
	case ID_NUM_F:		return _T("F");

	case ID_OPR_ADD: 	return _T("+");
	case ID_OPR_SUB:	return _T("-");
	case ID_OPR_MUL:	return _T("*");
	case ID_OPR_DIV:	return _T("/");
	case ID_OPR_MOD:	return _T("mod");
	case ID_OPR_NEG:	return _T("negate");
	case ID_OPR_AND:	return _T("&");
	case ID_OPR_OR:		return _T("|");
	case ID_OPR_XOR:	return _T("^");
	case ID_OPR_NOT:	return _T("~");
	case ID_OPR_SHFTL:	return _T("<<");
	case ID_OPR_SHFTR:	return _T(">>");
	case ID_OPR_EQUAL:	return _T("=");
	case ID_OPR_LPAREN:	return _T("(");
	case ID_OPR_RPAREN:	return _T(")");
	case ID_OPR_INT:	return _T("integer");
	case ID_OPR_FRAC:	return _T("fraction");
	case ID_OPR_SQ:		return _T("X^2");
	case ID_OPR_SQRT:	return _T("sqrt");
	case ID_OPR_INV:	return _T("1/X");
	case ID_OPR_NOOP:	return _T(" ");

	case ID_OPR_SIN:	return _T("sin");
	case ID_OPR_COS:	return _T("cos");
	case ID_OPR_TAN:	return _T("tan");
	case ID_OPR_ASIN:	return _T("arcsin");
	case ID_OPR_ACOS:	return _T("arccod");
	case ID_OPR_ATAN:	return _T("arctan");
	case ID_OPR_EXPO:	return _T("exp");
	case ID_OPR_NLOG:	return _T("ln");
	case ID_OPR_XEXPY:	return _T("X^Y");
	case ID_OPR_10EXP:	return _T("10^X");
	case ID_OPR_LOG:	return _T("log");

	case ID_CTL_CLR:	return _T("clear");
	case ID_CTL_CLRA:	return _T("clear all");
	case ID_CTL_DEL:	return _T("delete");
	case ID_CTL_STR:	return _T("store");
	case ID_CTL_RCL:	return _T("recall");
	case ID_CTL_EXP:	return _T("exp");
	case ID_CTL_DEC:	return _T(".");
	case ID_CTL_SCI:	return _T("sci/trig");
	case ID_CTL_CONST:	return _T("constant");
	case ID_CTL_PRG:	return _T("program");
	case ID_CTL_VPLOT:	return _T("vplt");
	case ID_CTL_QUIT:	return _T("quit");
	case ID_CTL_COPY:	return _T("copy");
	case ID_CTL_PASTE:	return _T("paste");

	case ID_RADIX_UP:	return _T("radix up");
	case ID_RADIX_DOWN:	return _T("radix down");
	case ID_XRADIX_UP:	return _T("xradix up");
	case ID_XRADIX_DOWN:return _T("xradic down");
	case ID_FORMAT_UP:	return _T("format up");
	case ID_FORMAT_DOWN:return _T("format down");
	case ID_DFRMT_UP:	return _T("xformat up");
	case ID_DFRMT_DOWN:	return _T("xformat down");

	case ID_PRG_REC:	return _T("record");
	case ID_PRG_RUN:	return _T("run");
	case ID_PRG_STOP:	return _T("stop");
	case ID_PRG_STEP:	return _T("step");
	case ID_PRG_INS:	return _T("insert control");
	case ID_PRG_DEL:	return _T("delete line");
	case ID_PRG_CLR:	return _T("clear program");
	case ID_PRG_PLOT2:	return _T("2D plot");
	case ID_PRG_PLOT3:	return _T("3D plot");
	case ID_PRG_LOAD:	return _T("load program");
	case ID_PRG_SAVE:	return _T("save program");

	case ID_PRG_LOOP:	return _T("loop {");
	case ID_PRG_POOL:	return _T("} loop");
	case ID_PRG_BREAK:	return _T("exit loop");
	case ID_PRG_LT:		return _T("if(<0) {");
	case ID_PRG_GT:		return _T("if(>0) {");
	case ID_PRG_LE:		return _T("if(<=0) {");
	case ID_PRG_GE:		return _T("if(>=0) {");
	case ID_PRG_EQ:		return _T("if(==0) {");
	case ID_PRG_FI:		return _T("} if");

	case ID_PRG_XMIN:
	case ID_PRG_XMAX:
	case ID_PRG_XSTEP:
		return _T("- x");

	case ID_PRG_YMIN:
	case ID_PRG_YMAX:
	case ID_PRG_YSTEP:
		return _T("- y");

	case ID_RCL_X:		return _T("recall X");
	case ID_RCL_Y:		return _T("recall Y");
	case ID_RCL_Z:		return _T("recall Z");
	case ID_RCL_A:		return _T("recall A");
	case ID_RCL_B:		return _T("recall B");
	case ID_RCL_C:		return _T("recall C");
	case ID_RCL_D:		return _T("recall D");
	case ID_RCL_E:		return _T("recall E");
	case ID_RCL_F:		return _T("recall F");
	case ID_RCL_G:		return _T("recall G");
	case ID_RCL_H:		return _T("recall H");
	case ID_RCL_I:		return _T("recall I");
	case ID_RCL_J:		return _T("recall J");
	case ID_RCL_K:		return _T("recall K");
	case ID_RCL_L:		return _T("recall L");
	case ID_RCL_M:		return _T("recall M");
	case ID_RCL_N:		return _T("recall N");
	case ID_RCL_O:		return _T("recall O");
	case ID_RCL_P:		return _T("recall P");
	case ID_RCL_Q:		return _T("recall Q");
	case ID_RCL_R:		return _T("recall R");
	case ID_RCL_S:		return _T("recall S");
	case ID_RCL_T:		return _T("recall T");
	case ID_RCL_U:		return _T("recall U");
	case ID_RCL_V:		return _T("recall V");
	case ID_RCL_W:		return _T("recall W");

	case ID_STR_X:		return _T("store X");
	case ID_STR_Y:		return _T("store Y");
	case ID_STR_Z:		return _T("store Z");
	case ID_STR_A:		return _T("store A");
	case ID_STR_B:		return _T("store B");
	case ID_STR_C:		return _T("store C");
	case ID_STR_D:		return _T("store D");
	case ID_STR_E:		return _T("store E");
	case ID_STR_F:		return _T("store F");
	case ID_STR_G:		return _T("store G");
	case ID_STR_H:		return _T("store H");
	case ID_STR_I:		return _T("store I");
	case ID_STR_J:		return _T("store J");
	case ID_STR_K:		return _T("store K");
	case ID_STR_L:		return _T("store L");
	case ID_STR_M:		return _T("store M");
	case ID_STR_N:		return _T("store N");
	case ID_STR_O:		return _T("store O");
	case ID_STR_P:		return _T("store P");
	case ID_STR_Q:		return _T("store Q");
	case ID_STR_R:		return _T("store R");
	case ID_STR_S:		return _T("store S");
	case ID_STR_T:		return _T("store T");
	case ID_STR_U:		return _T("store U");
	case ID_STR_V:		return _T("store V");
	case ID_STR_W:		return _T("store W");

	case ID_MEM_X:		return _T("X");
	case ID_MEM_Y:		return _T("Y");
	case ID_MEM_Z:		return _T("Z");
	case ID_MEM_A:		return _T("A");
	case ID_MEM_B:		return _T("B");
	case ID_MEM_C:		return _T("C");
	case ID_MEM_D:		return _T("D");
	case ID_MEM_E:		return _T("E");
	case ID_MEM_F:		return _T("F");
	case ID_MEM_G:		return _T("G");
	case ID_MEM_H:		return _T("H");
	case ID_MEM_I:		return _T("I");
	case ID_MEM_J:		return _T("J");
	case ID_MEM_K:		return _T("K");
	case ID_MEM_L:		return _T("L");
	case ID_MEM_M:		return _T("M");
	case ID_MEM_N:		return _T("N");
	case ID_MEM_O:		return _T("O");
	case ID_MEM_P:		return _T("P");
	case ID_MEM_Q:		return _T("Q");
	case ID_MEM_R:		return _T("R");
	case ID_MEM_S:		return _T("S");
	case ID_MEM_T:		return _T("T");
	case ID_MEM_U:		return _T("U");
	case ID_MEM_V:		return _T("V");
	case ID_MEM_W:		return _T("W");
		break;
	}
	return _T("???");
}

//**************************************************************************
CODE Bprog::CodeFromName(LPCTSTR n)
{
	switch(*n)
	{
	case _T('+'):	return ID_OPR_ADD;
	case _T('-'):	return ID_OPR_SUB;
	case _T('*'):	return ID_OPR_MUL;
	case _T('/'):	return ID_OPR_DIV;
	case _T('&'):	return ID_OPR_AND;
	case _T('|'):	return ID_OPR_OR;
	case _T('^'):	return ID_OPR_XOR;
	case _T('~'):	return ID_OPR_NOT;
	case _T('='):	return ID_OPR_EQUAL;
	case _T('('):	return ID_OPR_LPAREN;
	case _T(')'):	return ID_OPR_RPAREN;
	case _T(' '):	return ID_OPR_NOOP;
	case _T('.'):	return ID_CTL_DEC;

	case _T('<'):	return ID_OPR_SHFTL;
	case _T('>'):	return ID_OPR_SHFTR;

	case _T('}'):	
		if(! _tcscmp(n, _T("} loop")))		return ID_PRG_POOL;
		if(! _tcscmp(n, _T("} if")))		return ID_PRG_FI;
		break;
		
	case _T('1'):
		if(! _tcscmp(n, _T("1/X")))			return 	ID_OPR_INV;
		if(! _tcscmp(n, _T("10^X")))		return ID_OPR_10EXP;
		break;
		
	case _T('a'):
		if(! _tcscmp(n, _T("arcsin")))		return ID_OPR_ASIN;
		if(! _tcscmp(n, _T("arccod")))		return ID_OPR_ACOS;
		if(! _tcscmp(n, _T("arctan")))		return ID_OPR_ATAN;
		break;
	case _T('c'):
		if(! _tcscmp(n, _T("clear")))		return ID_CTL_CLR;
		if(! _tcscmp(n, _T("clear all")))	return ID_CTL_CLRA;
		if(! _tcscmp(n, _T("constant")))	return ID_CTL_CONST;
		if(! _tcscmp(n, _T("cos")))			return ID_OPR_COS;
		if(! _tcscmp(n, _T("copy")))		return ID_CTL_COPY;
		break;

	case _T('d'):
		if(! _tcscmp(n, _T("delete")))		return ID_CTL_DEL;
		break;

	case _T('e'):
		if(! _tcscmp(n, _T("exp")))			return ID_OPR_EXPO;
		if(! _tcscmp(n, _T("exit loop")))	return ID_PRG_BREAK;
		break;

	case _T('f'):
		if(! _tcscmp(n, _T("fraction")))	return ID_OPR_FRAC;
		if(! _tcscmp(n, _T("format up")))	return ID_FORMAT_UP;
		if(! _tcscmp(n, _T("format down")))	return ID_FORMAT_DOWN;
		break;

	case _T('i'):
		if(! _tcscmp(n, _T("if(<0)")))		return ID_PRG_LT;
		if(! _tcscmp(n, _T("if(>0)")))		return ID_PRG_GT;
		if(! _tcscmp(n, _T("if(<=0)")))		return ID_PRG_LE;
		if(! _tcscmp(n, _T("if(>=0)")))		return ID_PRG_GE;
		if(! _tcscmp(n, _T("if(==0)")))		return ID_PRG_EQ;
		if(! _tcscmp(n, _T("integer")))		return ID_OPR_INT;
		break;

	case _T('l'):
		if(! _tcscmp(n, _T("ln")))			return ID_OPR_NLOG;
		if(! _tcscmp(n, _T("log")))			return ID_OPR_LOG;
		if(! _tcscmp(n, _T("loop {")))		return ID_PRG_LOOP;
		break;

	case _T('m'):
		if(! _tcscmp(n, _T("mod")))			return ID_OPR_MOD;
		break;

	case _T('n'):
		if(! _tcscmp(n, _T("negate")))		return ID_OPR_NEG;
		break;

	case _T('p'):
		if(! _tcscmp(n,  _T("program")))	return ID_CTL_PRG;
		if(! _tcscmp(n,  _T("paste")))		return ID_CTL_PASTE;
		break;

	case _T('r'):
		if(! _tcscmp(n, _T("radix up")))	return ID_RADIX_UP;
		if(! _tcscmp(n, _T("radix down")))	return ID_RADIX_DOWN;
		if(! _tcscmp(n, _T("record")))		return ID_PRG_REC;
		if(! _tcscmp(n, _T("run")))			return ID_PRG_RUN;
		if(! _tcscmp(n,	_T("recall X")))	return ID_RCL_X;
		if(! _tcscmp(n,	_T("recall Y")))	return ID_RCL_Y;
		if(! _tcscmp(n,	_T("recall Z")))	return ID_RCL_Z;
		if(! _tcscmp(n,	_T("recall A")))	return ID_RCL_A;
		if(! _tcscmp(n,	_T("recall B")))	return ID_RCL_B;
		if(! _tcscmp(n,	_T("recall C")))	return ID_RCL_C;
		if(! _tcscmp(n,	_T("recall D")))	return ID_RCL_D;
		if(! _tcscmp(n,	_T("recall E")))	return ID_RCL_E;
		if(! _tcscmp(n,	_T("recall F")))	return ID_RCL_F;
		if(! _tcscmp(n,	_T("recall G")))	return ID_RCL_G;
		if(! _tcscmp(n,	_T("recall H")))	return ID_RCL_H;
		if(! _tcscmp(n,	_T("recall I")))	return ID_RCL_I;
		if(! _tcscmp(n,	_T("recall J")))	return ID_RCL_J;
		if(! _tcscmp(n,	_T("recall K")))	return ID_RCL_K;
		if(! _tcscmp(n,	_T("recall L")))	return ID_RCL_L;
		if(! _tcscmp(n,	_T("recall M")))	return ID_RCL_M;
		if(! _tcscmp(n,	_T("recall N")))	return ID_RCL_N;
		if(! _tcscmp(n,	_T("recall O")))	return ID_RCL_O;
		if(! _tcscmp(n,	_T("recall P")))	return ID_RCL_P;
		if(! _tcscmp(n,	_T("recall Q")))	return ID_RCL_Q;
		if(! _tcscmp(n,	_T("recall R")))	return ID_RCL_R;
		if(! _tcscmp(n,	_T("recall S")))	return ID_RCL_S;
		if(! _tcscmp(n,	_T("recall T")))	return ID_RCL_T;
		if(! _tcscmp(n,	_T("recall U")))	return ID_RCL_U;
		if(! _tcscmp(n,	_T("recall V")))	return ID_RCL_V;
		if(! _tcscmp(n,	_T("recall W")))	return ID_RCL_W;
		break;
		
	case _T('s'):
		if(! _tcscmp(n, _T("sin")))			return ID_OPR_SIN;
		if(! _tcscmp(n, _T("stop")))		return ID_PRG_STOP;
		if(! _tcscmp(n, _T("sci/trig")))	return ID_CTL_SCI;
		if(! _tcscmp(n, _T("step")))		return ID_PRG_STEP;
		if(! _tcscmp(n,	_T("store X")))		return ID_STR_X;
		if(! _tcscmp(n,	_T("store Y")))		return ID_STR_Y;
		if(! _tcscmp(n,	_T("store Z")))		return ID_STR_Z;
		if(! _tcscmp(n,	_T("store A")))		return ID_STR_A;
		if(! _tcscmp(n,	_T("store B")))		return ID_STR_B;
		if(! _tcscmp(n,	_T("store C")))		return ID_STR_C;
		if(! _tcscmp(n,	_T("store D")))		return ID_STR_D;
		if(! _tcscmp(n,	_T("store E")))		return ID_STR_E;
		if(! _tcscmp(n,	_T("store F")))		return ID_STR_F;
		if(! _tcscmp(n,	_T("store G")))		return ID_STR_G;
		if(! _tcscmp(n,	_T("store H")))		return ID_STR_H;
		if(! _tcscmp(n,	_T("store I")))		return ID_STR_I;
		if(! _tcscmp(n,	_T("store J")))		return ID_STR_J;
		if(! _tcscmp(n,	_T("store K")))		return ID_STR_K;
		if(! _tcscmp(n,	_T("store L")))		return ID_STR_L;
		if(! _tcscmp(n,	_T("store M")))		return ID_STR_M;
		if(! _tcscmp(n,	_T("store N")))		return ID_STR_N;
		if(! _tcscmp(n,	_T("store O")))		return ID_STR_O;
		if(! _tcscmp(n,	_T("store P")))		return ID_STR_P;
		if(! _tcscmp(n,	_T("store Q")))		return ID_STR_Q;
		if(! _tcscmp(n,	_T("store R")))		return ID_STR_R;
		if(! _tcscmp(n,	_T("store S")))		return ID_STR_S;
		if(! _tcscmp(n,	_T("store T")))		return ID_STR_T;
		if(! _tcscmp(n,	_T("store U")))		return ID_STR_U;
		if(! _tcscmp(n,	_T("store V")))		return ID_STR_V;
		if(! _tcscmp(n,	_T("store W")))		return ID_STR_W;
		if(! _tcscmp(n,	_T("sqrt")))		return ID_OPR_SQRT;
		break;
		
	case _T('t'):
		if(! _tcscmp(n, _T("tan")))			return ID_OPR_TAN;
		break;
		
	case _T('x'):
		if(! _tcscmp(n, _T("xradix up")))	return ID_XRADIX_UP;
		if(! _tcscmp(n, _T("xradic down")))	return ID_XRADIX_DOWN;
		if(! _tcscmp(n, _T("xformat up")))	return ID_DFRMT_UP;
		if(! _tcscmp(n, _T("xformat down")))return ID_DFRMT_DOWN;
		break;

	case _T('X'):
		if(! _tcscmp(n, _T("X^2")))			return ID_OPR_SQ;
		if(! _tcscmp(n, _T("X^Y")))			return ID_OPR_XEXPY;
		break;

	default:
		return ID_BAD_CODE;
	}
	return ID_BAD_CODE;
}

//**************************************************************************
void Bprog::FitToPanel()
{
	if(! m_hwnd)
	{
		RECT rc;

		GetParentPanel()->GetPanelClientRect(&rc);
		m_hwnd = CreateWindow(
								_T("bcalc_program_pane"),
								_T("HI"),
								WS_CHILD | WS_CLIPCHILDREN,
								0, 0, rc.right-rc.left, rc.bottom-rc.top,
								GetParentPanel()->GetWindow(),
								NULL,
								GetParentPanel()->GetInstance(),
								(LPVOID)this
							 ); 
		SetupWindows(true);
	}
	SetupWindows(false);
	BappPane::FitToPanel();
}

//**************************************************************************
void Bprog::SetupWindows(bool docreate)
{
	RECT rc;
	int  x, y, w, h;
	int  ix, iy, bx, by, bw, bh;
	int  aw, ah;
	int  max_h, max_w;
	int	 enabled, wasenabled;

	if(! m_hwnd) return;

	GetParentPanel()->GetPanelClientRect(&rc);
	aw = rc.right - rc.left;
	ah = rc.bottom - rc.top;

	m_bcalc->GetButtonSize(&bw, &bh);

	// button map
	//
	x = BCALC_BUT_LM;
	y = BCALC_DISP_TM;

	for(iy = max_w = 0, by = y; iy < PROG_BUT_MAP_H; iy++)
	{
		for(ix = max_h = 0, bx = x; ix < PROG_BUT_MAP_W; ix++)
		{
			PBCBUTTON pb;

			pb = &g_progbuttons[iy][ix];
			w = pb->m_w;
			h = pb->m_h;

			w = (pb->m_w * aw) / (BUT_DIG_W * (PROG_BUT_MAP_W));
			h = (pb->m_h * (ah - y)) / (BUT_DIG_H * PROG_BUT_MAP_H);
			if(w < BCALC_BUT_MIN_W)
				w = BCALC_BUT_MIN_W;
			if(w > bw * pb->m_w / BUT_DIG_W)
				w = bw * pb->m_w / BUT_DIG_W;
			if(h < BCALC_BUT_MIN_H)
				h = BCALC_BUT_MIN_H;
			if(h > bh)
				h = bh;
			w = w - BCALC_BUT_MX;
			h = h - BCALC_BUT_MY;

			if(m_bcalc->GetProgramMode() == pmRun && pb->m_id != ID_PRG_STOP)
				enabled = false;
			else
				enabled = true;

			if(pb->m_id)
			{
				if(docreate || ! pb->m_hwnd)
				{
					pb->m_hwnd = CreateWindow(
											_T("bcalc_button"),
											pb->m_name,
											WS_CHILD,
											bx, by, w, h,
											GetWindow(),
											NULL,
											GetParentPanel()->GetInstance(),
											(LPVOID)pb
											);
					SetWindowLong(pb->m_hwnd, GWL_ID, pb->m_id);
					SetWindowLong(pb->m_hwnd, 0, (LONG)m_bcalc);
					EnableWindow(pb->m_hwnd, enabled);
					ShowWindow(pb->m_hwnd, SW_SHOW);
				}
				else if(pb->m_hwnd)
				{
					MoveWindow(pb->m_hwnd, bx, by, w, h, TRUE);

					wasenabled = IsWindowEnabled(pb->m_hwnd);
					EnableWindow(pb->m_hwnd, enabled);
					if(enabled != wasenabled)
						InvalidateRect(pb->m_hwnd, NULL, FALSE);
				}
			}
			bx += w + BCALC_BUT_MX;
			if(h > max_h) max_h = h;
		}
		by += max_h + BCALC_BUT_MY;
		if(bx > max_w)
			max_w = bx;
	}
	x = max_w + BCALC_DISP_LM;
	y = BCALC_DISP_TM;
	w = aw - x - 2 * BCALC_DISP_LM;
	h = ah - 2 * BCALC_DISP_TM;

	// program display
	//
	if(docreate)
	{
		m_hwndEdit = 
					CreateWindowEx(
								WS_EX_STATICEDGE,
								_T("bcalc_program"),
								_T("program"),
								WS_CHILD,
								x, y, w, h,
								GetWindow(),
								NULL,
								GetParentPanel()->GetInstance(),
								(LPVOID)this
								);
		ShowWindow(m_hwndEdit, SW_SHOW);
	}
	else
	{
		MoveWindow(m_hwndEdit, x, y, w, h, FALSE);
	}
	GetClientRect(GetParentPanel()->GetWindow(), &rc);
	m_bcalc->GetPersist()->SetNvInt(_T("display/program/W"), rc.right - rc.left);
}

//**************************************************************************
bool Bprog::GetButtonState(int id)
{
	int ix, iy;

	for(iy = 0; iy < PROG_BUT_MAP_H; iy++)
	{
		for(ix = 0; ix < PROG_BUT_MAP_W; ix++)
		{
			PBCBUTTON pb;

			pb = &g_progbuttons[iy][ix];
			if(pb->m_id == id)
			{
				return pb->m_on ? true : false;
			}
		}
	}
	return false;
}

//**************************************************************************
void Bprog::SetButtonState(int id, bool state)
{
	int ix, iy;

	for(iy = 0; iy < PROG_BUT_MAP_H; iy++)
	{
		for(ix = 0; ix < PROG_BUT_MAP_W; ix++)
		{
			PBCBUTTON pb;

			pb = &g_progbuttons[iy][ix];
			if(pb->m_id == id)
			{
				bool pstate = pb->m_on ? true : false;

				if(pstate != state)
				{
					pb->m_on = state;
					InvalidateRect(pb->m_hwnd, NULL, TRUE);
					return;
				}
			}
		}
	}
}

//**************************************************************************
LRESULT Bprog::OnMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC			hdc;
	RECT		rc;
	Bprog*		pp;
	int			id = 0;

	pp = (Bprog*)GetWindowLong(hWnd, GWL_USERDATA);

	switch (message) 
	{
	case WM_CREATE:

		{ 
			LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;

			SetWindowLong(hWnd, GWL_USERDATA, (LONG)pcs->lpCreateParams);
		}
		break;

	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rc);
		EndPaint(hWnd, &ps);
		break;

	case WM_COMMAND:

		if(m_bcalc->GetProgramMode() == pmRun)
		{
			if(wParam != ID_PRG_STOP)
			{
				if(IDYES == MessageBox(NULL, _T("Stop running program"), _T("bcalc - Program"), MB_YESNO))
				{
					StopProgram();
				}
				else
				{
					break;
				}
			}
		}
		switch(wParam)
		{
		case ID_PRG_REC:
			if(m_bcalc)
			{
				bool rec = GetButtonState(ID_PRG_REC);

				m_bcalc->SetProgramMode(rec ? pmRecord : pmEdit);
				InvalidateRect(m_hwndEdit, NULL, TRUE);
			}
			break;

		case ID_PRG_RUN:
			StopRecording();
			RunProgram(false);
			break;

		case ID_PRG_PLOT2:
			StopRecording();
			PlotProgram(2);
			break;

		case ID_PRG_PLOT3:
			StopRecording();
			PlotProgram(3);
			break;

		case ID_PRG_STOP:
			StopRecording();
			StopProgram();
			break;

		case ID_PRG_STEP:
			StopRecording();
			StepProgram(false);
			break;

		case ID_PRG_INS:
			InsertControl();
			break;

		case ID_PRG_DEL:
			Delete();
			break;

		case ID_PRG_CLR:
			StopRecording();
			ClearProgram();
			break;

		case ID_PRG_LOAD:
			StopRecording();
			LoadProgram();
			break;

		case ID_PRG_SAVE:
			StopRecording();
			SaveProgram();
			break;

		default:
			break;
		}
		break;

	case WM_KEYDOWN:
	case WM_MOUSEWHEEL:

		PostMessage(pp->m_hwndEdit, message, wParam, lParam);
		break;

	default:
		return BappPane::OnMessage(hWnd, message, wParam, lParam);
   }
   return 0;
}

#define BCALC_EW_X 16
#define BCALC_COLOR_CURSOR		RGB(80,200,255)
#define BCALC_COLOR_RECCURSOR	RGB(255,255,80)
#define BCALC_COLOR_ERRCURSOR	RGB(255,0,0)
#define BCALC_COLOR_STPCURSOR	RGB(255,0,255)

void DrawCursor(HDC hdc, int x, int y, int w, COLORREF fill)
{
	HPEN hpen, hop;
	int  h = 10;

	hop = (HPEN)SelectObject(hdc, GetStockObject(BLACK_PEN));

	// outline
	MoveToEx(hdc, 3, y + 2, NULL);
	LineTo(hdc, w - 6, y + 2);
	LineTo(hdc, w - 2, y + 1 + h / 2);
	LineTo(hdc, w - 6, y + h);
	LineTo(hdc, 3, y + h);
	LineTo(hdc, 7, y + 1 + h / 2);
	LineTo(hdc, 3, y + 2);

	hpen = CreatePen(PS_SOLID, 0, fill);
	SelectObject(hdc, hpen);

	// fill
	MoveToEx(hdc, 5, y + 3, NULL);
	LineTo(hdc, w - 5, y + 3);
	MoveToEx(hdc, 6, y + 4, NULL);
	LineTo(hdc, w - 4, y + 4);
	MoveToEx(hdc, 7, y + 5, NULL);
	LineTo(hdc, w - 3, y + 5);
	MoveToEx(hdc, 8, y + 6, NULL);
	LineTo(hdc, w - 2, y + 6);
	MoveToEx(hdc, 5, y + 9, NULL);
	LineTo(hdc, w - 5, y + 9);
	MoveToEx(hdc, 6, y + 8, NULL);
	LineTo(hdc, w - 4, y + 8);
	MoveToEx(hdc, 7, y + 7, NULL);
	LineTo(hdc, w - 3, y + 7);

	SelectObject(hdc, hop);
}

//**************************************************************************
LRESULT ProgEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC			hdc;
	RECT		rc;
	HFONT		hof;
	Bprog*		pp;
	PPRGENTRY	pe;
	TCHAR       tb[1024];
	LPCTSTR		pt;
	COLORREF	bfrg = RGB(0,0,0);
	SIZE		size;
	int			x, y, yy, i;
	int			textlen;

	pp = (Bprog*)GetWindowLong(hWnd, GWL_USERDATA);

	switch (message) 
	{
	case WM_CREATE:

		{ 
			LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;

			SetWindowLong(hWnd, GWL_USERDATA, (LONG)pcs->lpCreateParams);

			pp = (Bprog*)GetWindowLong(hWnd, GWL_USERDATA);
			if(pp)
			{
				hdc = GetDC(hWnd);
				GetTextExtentPoint32(hdc, _T("Hello"), 5, &size);	
				pp->m_th = size.cy;
				ReleaseDC(hWnd, hdc);
			}
		}
		break;

	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rc);
		FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
		y = rc.bottom - rc.top;
		if(y != pp->m_wh)
		{
			pp->m_wh = y;
			pp->SetEditScroller();
		}
		if(! pp->GetProgram() && pp->GetCalculator()->GetProgramMode() != pmRecord)
		{
			y = 0;
			SetTextColor(hdc, bfrg);
			TextOut(hdc, 0, y, _T("Press REC to"), 12);
			TextOut(hdc, 0, y + pp->m_th, _T("begin recording"), 15);
			TextOut(hdc, 0, y + pp->m_th + pp->m_th, _T("your program"), 12);
		}
		for(pe = pp->GetProgram(), y = 0; pe; pe = pe->m_next)
		{
			x = BCALC_EW_X + 8;
			bfrg = RGB(0,0,0);

			if(pe->IsNumber() && pe->m_num)
			{
				const char* pnum = pe->m_num->Format(512, 10, kbDefault);
#ifdef UNICODE
				BUtil::Utf8Decode(tb, pnum);
#else
				strcpy(tb, pnum);
#endif
				pt = tb;
			}
			else if(pe->IsCode())
			{
				pt = pp->CodeName(pe->m_code);
				if(pe->m_code >= ID_PRG_LOOP && pe->m_code <= ID_PRG_FI)
				{
					x = BCALC_EW_X;
					bfrg = RGB(0, 0, 127);
				}
			}
			if(pe == pp->GetErrorCursor()) 
				bfrg = RGB(240, 20, 20);
			hof = (HFONT)SelectObject(hdc, pp->GetCalculator()->m_hf_n);
			textlen = _tcslen(pt);
			if(y - pp->m_yoff >= 0 && y - pp->m_yoff < pp->m_wh)
			{
				SetTextColor(hdc, bfrg);
				TextOut(hdc, x, y - pp->m_yoff, pt, textlen);
			}
			if(pe == pp->GetProgramCursor())
			{
				COLORREF cfrg;

				if(pp->GetCalculator()->GetProgramMode() == pmRecord)
					cfrg = BCALC_COLOR_RECCURSOR;
				else if(pe == pp->GetErrorCursor())
					cfrg = BCALC_COLOR_ERRCURSOR;
				else
					cfrg = BCALC_COLOR_CURSOR;

				DrawCursor(hdc, 0, y - pp->m_yoff + 1, BCALC_EW_X, cfrg);
			}
			y += pp->m_th;
		}
		EndPaint(hWnd, &ps);
		break;

	case WM_COMMAND:

		break;

	case WM_LBUTTONDOWN:

		yy = HIWORD(lParam);
		for(pe = pp->GetProgram(), y = 0, i = 0, x = 0; pe; pe = pe->m_next)
		{
			if(pe == pp->GetProgramCursor())
				x = 1;
			if(y - pp->m_yoff <= yy && yy < y - pp->m_yoff + pp->m_th)
			{
				if(! x)
				{
					for(i = 0; pe; pe = pe->m_next, i--)
					{
						if(pe == pp->GetProgramCursor())
							break;
					}
				}
				pp->MoveRel(i);	
				break;
			}
			if(x) i++;
			y += pp->m_th;
		}
		break;

	case WM_MOUSEWHEEL:

		y = (int)(short)HIWORD(wParam);
		y /= WHEEL_DELTA;
		pp->MoveRel(-y);
		break;

	case WM_KEYDOWN:

		switch(wParam)
		{
		case VK_PRIOR:
			pp->MoveRel(-(pp->m_wh / (pp->m_th ? pp->m_th : 16)));
			break;
		case VK_NEXT:
			pp->MoveRel(pp->m_wh / (pp->m_th ? pp->m_th : 16));
			break;
		case VK_END:
			pp->MoveRel(0x7fffffff);
			break;
		case VK_HOME:
			pp->MoveRel(-0x7fffffff);
			break;
		case VK_LEFT:
		case VK_UP:
			pp->MoveRel(-1);
			break;
		case VK_RIGHT:
		case VK_DOWN:
			pp->MoveRel(1);
			break;

		default:
			break;
		}
		break;

	case WM_VSCROLL:

		switch(LOWORD(wParam))
		{
		case SB_BOTTOM:
			pp->MoveRel(0x7fffffff);
			break;
		case SB_ENDSCROLL:
			break;
		case SB_LINEDOWN: 
			pp->MoveRel(1);
			break;
		case SB_LINEUP:
			pp->MoveRel(-1);
			break;
		case SB_PAGEDOWN:
			pp->MoveRel(pp->m_wh / (pp->m_th ? pp->m_th : 16));
			break;
		case SB_PAGEUP:
			pp->MoveRel(-(pp->m_wh / (pp->m_th ? pp->m_th : 16)));
			break;
		case SB_THUMBPOSITION:
			y = HIWORD(wParam);
			pp->MoveRel(y / (pp->m_th ? pp->m_th : 16));
			break;
		case SB_THUMBTRACK: 
			y = HIWORD(wParam);
			pp->MoveRel(-(y / (pp->m_th ? pp->m_th : 16)));
			break;
		case SB_TOP:
			pp->MoveRel(-0x7fffffff);
			break;
		}
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

//***********************************************************************
BOOL CALLBACK FileIoProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static Bprog* pprog = NULL;

	switch(message)
	{
	case WM_INITDIALOG:
		
		pprog = (Bprog*)lParam;
		SetFocus(GetDlgItem(hDlg, IDC_FILENAME));
		SetDlgItemText(hDlg, IDC_FILENAME, pprog->m_filename);
		return FALSE;

	case WM_COMMAND:

		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			if(LOWORD(wParam) == IDOK)
			{
				GetDlgItemText(hDlg, IDC_FILENAME, pprog->m_filename, MAX_PATH);
			}
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
    return FALSE;
}

