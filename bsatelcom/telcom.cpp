#include "telcomx.h"

#define ESCFOUND		5
#define IACFOUND		6
#define NEGOTIATE		1

#define tnPS_SNLINEMODE 41

#define LOG if(m_log) m_log->Log

static LPCTSTR telcodes[]  = {
	_T("TEL_EOF"),
	_T("SUSP"),
	_T("ABORT"),
	_T("??"),
	_T("SE"),
	_T("NOP"),
	_T("DataMark"),
	_T("BREAK"),
	_T("IP"),
	_T("AO"),
	_T("AYT"),
	_T("EC"),
	_T("EL"),
	_T("GA"),
	_T("SB"),
	_T("WILL"),
	_T("WON\'T"),
	_T("DO"),
	_T("DON\'T"),
	_T("IAC_T")
};

/* from NCSA telnet */
static LPCTSTR telstates[]={
    _T("EOF"),
    _T("Suspend Process"),
    _T("Abort Process"),
    _T("Unknown (239)"),
    _T("Subnegotiation End"),
    _T("NOP"),
    _T("Data Mark"),
    _T("Break"),
    _T("Interrupt Process"),
    _T("Abort Output"),
    _T("Are You There"),
    _T("Erase Character"),
    _T("Erase Line"),
    _T("Go Ahead"),
    _T("Subnegotiate"),
	_T("Will"),
	_T("Won't"),
	_T("Do"),
	_T("Don't_T")
};

static LPCTSTR teloptions[256]={      /* ascii strings for Telnet options */
	_T("Binary"),				/* 0 */
	_T("Echo"),
	_T("Reconnection"),
	_T("Supress Go Ahead"),
	_T("Message Size Negotiation"),
	_T("Status"),				/* 5 */
	_T("Timing Mark"),
	_T("Remote Controlled Trans and Echo"),
	_T("Output Line Width"),
	_T("Output Page Size"),
	_T("Output Carriage-Return Disposition"),	/* 10 */
	_T("Output Horizontal Tab Stops"),
	_T("Output Horizontal Tab Disposition"),
	_T("Output Formfeed Disposition"),
	_T("Output Vertical Tabstops"),
	_T("Output Vertical Tab Disposition"),		/* 15 */
	_T("Output Linefeed Disposition"),
	_T("Extended ASCII"),
	_T("Logout"),
	_T("Byte Macro"),
	_T("Data Entry Terminal"),					/* 20 */
	_T("SUPDUP"),
	_T("SUPDUP Output"),
	_T("Send Location"),
	_T("Terminal Type"),
	_T("End of Record"),						/* 25 */
	_T("TACACS User Identification"),
	_T("Output Marking"),
	_T("Terminal Location Number"),
	_T("3270 Regime"),
	_T("X.3 PAD"),								/* 30 */
	_T("Negotiate About Window Size"),
	_T("Terminal Speed"),
	_T("Toggle Flow Control"),
	_T("Linemode"),
	_T("X Display Location"),					/* 35 */
    _T("Environment"),
    _T("Authentication"),
    _T("Data Encryption"),
    _T("39"),
	_T("40"),_T("41"),_T("42"),_T("43"),_T("44"),_T("45"),_T("46"),_T("47"),_T("48"),_T("49"),
	_T("50"),_T("51"),_T("52"),_T("53"),_T("54"),_T("55"),_T("56"),_T("57"),_T("58"),_T("59"),
	_T("60"),_T("61"),_T("62"),_T("63"),_T("64"),_T("65"),_T("66"),_T("67"),_T("68"),_T("69"),
	_T("70"),_T("71"),_T("72"),_T("73"),_T("74"),_T("75"),_T("76"),_T("77"),_T("78"),_T("79"),
	_T("80"),_T("81"),_T("82"),_T("83"),_T("84"),_T("85"),_T("86"),_T("87"),_T("88"),_T("89"),
	_T("90"),_T("91"),_T("92"),_T("93"),_T("94"),_T("95"),_T("96"),_T("97"),_T("98"),_T("99"),
	_T("100"),_T("101"),_T("102"),_T("103"),_T("104"),_T("105"),_T("106"),_T("107"),_T("108"),_T("109"),
	_T("110"),_T("111"),_T("112"),_T("113"),_T("114"),_T("115"),_T("116"),_T("117"),_T("118"),_T("119"),
	_T("120"),_T("121"),_T("122"),_T("123"),_T("124"),_T("125"),_T("126"),_T("127"),_T("128"),_T("129"),
	_T("130"),_T("131"),_T("132"),_T("133"),_T("134"),_T("135"),_T("136"),_T("137"),_T("138"),_T("139"),
	_T("140"),_T("141"),_T("142"),_T("143"),_T("144"),_T("145"),_T("146"),_T("147"),_T("148"),_T("149"),
	_T("150"),_T("151"),_T("152"),_T("153"),_T("154"),_T("155"),_T("156"),_T("157"),_T("158"),_T("159"),
	_T("160"),_T("161"),_T("162"),_T("163"),_T("164"),_T("165"),_T("166"),_T("167"),_T("168"),_T("169"),
	_T("170"),_T("171"),_T("172"),_T("173"),_T("174"),_T("175"),_T("176"),_T("177"),_T("178"),_T("179"),
	_T("180"),_T("181"),_T("182"),_T("183"),_T("184"),_T("185"),_T("186"),_T("187"),_T("188"),_T("189"),
	_T("190"),_T("191"),_T("192"),_T("193"),_T("194"),_T("195"),_T("196"),_T("197"),_T("198"),_T("199"),
	_T("200"),_T("201"),_T("202"),_T("203"),_T("204"),_T("205"),_T("206"),_T("207"),_T("208"),_T("209"),
	_T("210"),_T("211"),_T("212"),_T("213"),_T("214"),_T("215"),_T("216"),_T("217"),_T("218"),_T("219"),
	_T("220"),_T("221"),_T("222"),_T("223"),_T("224"),_T("225"),_T("226"),_T("227"),_T("228"),_T("229"),
	_T("230"),_T("231"),_T("232"),_T("233"),_T("234"),_T("235"),_T("236"),_T("237"),_T("238"),_T("239"),
	_T("240"),_T("241"),_T("242"),_T("243"),_T("244"),_T("245"),_T("246"),_T("247"),_T("248"),_T("249"),
	_T("250"),_T("251"),_T("252"),_T("253"),_T("254"),
	_T("Extended Options List_T")		/* 255 */
};

static LPCTSTR LMoptions[]={      /* ascii strings for Linemode sub-options */
    _T("None"),
    _T("MODE"),
    _T("FORWARDMASK"),
    _T("SLC_T")
};

static LPCTSTR ModeOptions[]={      /* ascii strings for Linemode edit options */
    _T("None"),
    _T("EDIT"),
    _T("TRAPSIG"),
    _T("ACK"),
    _T("SOFT TAB"),
    _T("LIT ECHO_T")
};

static LPCTSTR SLCoptions[]={     /* ascii strings for Linemode SLC characters */
	_T("None"),
	_T("SYNCH"),
	_T("BREAK"),
	_T("IP"),
	_T("ABORT OUTPUT"),
	_T("AYT"),
	_T("EOR"),
	_T("ABORT"),
	_T("EOF"),
	_T("SUSP"),
	_T("EC"),
	_T("EL"),
	_T("EW"),
	_T("RP"),
	_T("LNEXT"),
	_T("XON"),
	_T("XOFF"),
	_T("FORW1"),
	_T("FORW2"),
	_T("MCL"),
	_T("MCR"),
	_T("MCWL"),
	_T("MCWR"),
	_T("MCBOL"),
	_T("MCEOL"),
	_T("INSRT"),
	_T("OVER"),
	_T("ECR"),
	_T("EWR"),
	_T("EBOL"),
	_T("EEOL_T")
};

static LPCTSTR SLCflags[]={      /* ascii strings for Linemode SLC flags */
    _T("SLC_NOSUPPORT"),
    _T("SLC_CANTCHANGE"),
    _T("SLC_VALUE"),
    _T("SLC_DEFAULT")
};

static BYTE LMdefaults[NUMLMODEOPTIONS+1]={   /* Linemode default character for each function */
    (BYTE)-1,   /* zero isn't used */
    (BYTE)-1,   /* we don't support SYNCH */
    3,          /* ^C is default for BRK */
    3,          /* ^C is default for IP */
    15,         /* ^O is default for AO */
    25,         /* ^Y is default for AYT */             /* 5 */
    (BYTE)-1,   /* we don't support EOR */
    3,          /* ^C is default for ABORT */
    4,          /* ^D is default for EOF */
    26,         /* ^Z is default for SUSP */
    8,          /* ^H is default for EC */              /* 10 */
    21,         /* ^U is default for EL */
    23,         /* ^W is default for EW */
    18,         /* ^R is default for RP */
    22,         /* ^V is default for LNEXT */
    17,         /* ^Q is default for XON */             /* 15 */
    19,         /* ^S is default for XOFF */
    22,         /* ^V is default for FORW1 */
    5,          /* ^E is default for FORW2 */
    (BYTE)-1,   /* we don't support MCL */
    (BYTE)-1,   /* we don't support MCR */              /* 20 */
    (BYTE)-1,   /* we don't support MCWL */
    (BYTE)-1,   /* we don't support MCWR */
    (BYTE)-1,   /* we don't support MCBOL */
    (BYTE)-1,   /* we don't support MCEOL */
    (BYTE)-1,   /* we don't support INSRT */            /* 25 */
    (BYTE)-1,   /* we don't support OVER */
    (BYTE)-1,   /* we don't support ECR */
    (BYTE)-1,   /* we don't support EWR */
    (BYTE)-1,   /* we don't support EBOL */
    (BYTE)-1    /* we don't support EEOL */             /* 30 */
};


//***********************************************************************
BtelnetStream::BtelnetStream()
			:
			Bstream(),
			m_ps(NULL),
			m_parseState(tnPS_NORM),
			m_log(NULL)
{
#ifdef LOG_TELNET
	m_log = new Blog(5, logtoCONSOLE, NULL);
#endif
}

//***********************************************************************
BtelnetStream::~BtelnetStream()
{
	if(m_ps)
	{
		m_ps->Close();
		delete m_ps;
	}
	if(m_bOpen)
		Close();
	if(m_log)
		delete m_log;
}

//***********************************************************************
ERRCODE BtelnetStream::Open(char* host, short port)
{
	ERRCODE ec;
	int		i;

	if(m_bOpen)
		return errFAILURE;

	for(i = 0; i < MAX_TELOPTCODE; i++)
		m_opts[i] = tnUnsure;
	m_parseState = tnPS_NORM;

	m_ps = new BtcpStream();
	ec = m_ps->Open(host, port);
	return ec;
}

//***********************************************************************
ERRCODE BtelnetStream::Pend(int to_secs, int to_usecs)
{
	if(m_ps) return m_ps->Pend(to_secs, to_usecs);
	return errFAILURE;
}

//***********************************************************************
ERRCODE BtelnetStream::Connect(int to_secs, int to_usecs)
{
	if(m_ps) return m_ps->Connect(to_secs, to_usecs);
	return errFAILURE;
}

/*
 * telnetReply - reply to a telnet command
 */
//***********************************************************************
ERRCODE BtelnetStream::TelnetReply(BYTE otype, BYTE ecode)
{
	BYTE rep[4];
	int  wcnt;

	if(otype == WILLTEL || otype == WONTTEL)
	{
		if(ecode >= 0 && ecode < MAX_TELOPTCODE)
		{
			if(m_opts[ecode] == tnWill || m_opts[ecode] == tnWont)
			{
				LOG(logDebug, 3, _T("->SKIPPING "_Pfs_" "_Pfs_" (sent already)\n"), telcodes[otype - LOW_TEL_OPT],
						teloptions[ecode]);
				return errOK;
			}
			m_opts[ecode] = otype == WILLTEL ? tnWill : tnWont;
		}
	}
	else if(otype == DOTEL || otype == DONTTEL)
	{
		if(ecode >= 0 && ecode < MAX_TELOPTCODE)
		{
			if(m_opts[ecode] == tnDo || m_opts[ecode] == tnDont)
			{
				LOG(logDebug, 3, _T("->SKIPPING "_Pfs_" "_Pfs_" (sent already)\n"), telcodes[otype - LOW_TEL_OPT],
						teloptions[ecode]);
				return errOK;
			}
			m_opts[ecode] = otype == WILLTEL ? tnWill : tnWont;
		}
	}
	LOG(logDebug, 3, _T("->"_Pfs_" "_Pfs_"\n"), telcodes[otype - LOW_TEL_OPT], teloptions[ecode]);

	rep[0] = IAC;
	rep[1] = otype;
	rep[2] = ecode;
	wcnt = 3;
	return Write(rep, wcnt);
}

/*
 * telnetEscape - parse telnet command
 */
//***********************************************************************
TelParseState BtelnetStream::TelnetEscape(BYTE ecode, BYTE* otype)
{
	LOG(logDebug, 3, _T("<-"_Pfs_" \n"), telcodes[ecode - LOW_TEL_OPT]);

	*otype = ecode;

	if(ecode >= WILLTEL && ecode <= DONTTEL)
	{
		return tnPS_OPTION;
	}
	else if(ecode == SB)
	{
		return tnPS_SUBNEG;
	}
	else if(ecode == IAC)
	{
		return tnPS_ESCIAC;
	}
	else if(ecode == EC)
	{
		return tnPS_ERASEC;
	}
	else
	{
		return tnPS_NORM;
	}
}

/*
 * telnetOption - handle telnet options
 */
//***********************************************************************
TelParseState BtelnetStream::TelnetOption(BYTE otype, BYTE ecode)
{
	LOG(logDebug, 4, _T(""_Pfs_"\n"), teloptions[ecode]);

	/*
	 * WILL - side wants to self-enable option
	 * DO   - side wants other side to enable option
	 * WONT - side wants to self-disable option
	 * DONT - side wants other side to disable option
	 */
	if(ecode >= 0 && ecode < MAX_TELOPTCODE)
	{
		switch(otype)
		{
		case DOTEL:
		case DONTTEL:
			break;
		case WILLTEL:
			break;
		case WONTTEL:
			/* if we said do, and they say no, indicate they dont */
			if(m_opts[ecode] == tnDo)
				m_opts[ecode] = tnDont;
			break;
		}
	}

	switch(ecode)
	{
	/* BINARY we wont negotiate BINARY */
	case BINARY:
		switch(otype)
		{
		case DOTEL:
			TelnetReply(WONTTEL, ecode); /* no binary */
			break;
		case DONTTEL:
			TelnetReply(WONTTEL, ecode);
			break;
		case WILLTEL:
			TelnetReply(DONTTEL, ecode);
			break;
		case WONTTEL:
			TelnetReply(DONTTEL, ecode);
			break;
		}
		break;
	/* Suppress Go-ahead - we want server to, and we will too */
	case SGA:
		switch(otype)
		{
		case DOTEL:
			TelnetReply(WILLTEL, ecode);
			break;
		case DONTTEL:
			TelnetReply(WONTTEL, ecode);
			break;
		case WILLTEL:
			TelnetReply(DOTEL, ecode);
			break;
		case WONTTEL:
			TelnetReply(DONTTEL, ecode);
			break;
		}
		break;
	/* ECHO - we want server to always ECHO, and we wont */
	case ECHOTEL:
		switch(otype)
		{
		case DOTEL:
		case DONTTEL:
			TelnetReply(WONTTEL, ecode);
			break;
		case WILLTEL:
			TelnetReply(DOTEL, ecode);
			break;
		case WONTTEL:
			TelnetReply(DONTTEL, ecode);
			break;
		}
		break;
	/* TERMTYPE - we WILL tell our terminal type */
	case TERMTYPE:
		switch(otype)
		{
		case DOTEL:
			TelnetReply(WILLTEL, ecode);
			break;
		case DONTTEL:
			TelnetReply(WONTTEL, ecode);
			break;
		case WILLTEL:
		case WONTTEL:
			TelnetReply(DONTTEL, ecode);
			break;
		}
		break;
	/* NAWS - we WILL negotiate about window size */
	case NAWS:
		switch(otype)
		{
		case DOTEL:
			TelnetReply(WILLTEL, ecode);
			//telnetSendWindowSize(pComm);
			break;
		case DONTTEL:
			TelnetReply(WONTTEL, ecode);
			break;
		case WILLTEL:
		case WONTTEL:
			TelnetReply(DONTTEL, ecode);
			break;
		}
		break;

	case TFLOWCNTRL:
	case RECONNECT:
	case AMSN:
	case STATUS:
	case TIMING:
	case RCTAN:
	case OLW:
	case OPS:
	case OCRD:
	case OHTS:
	case OHTD:
	case OFFD:
	case OVTS:
	case OVTD:
	case OLFD:
	case XASCII:
	case LOGOUT:
	case BYTEM:
	case DET:
	case SUPDUP:
	case SUPDUPOUT:
	case SENDLOC:
	case EOR:
	case TACACSUID:
	case OUTPUTMARK:
	case TERMLOCNUM:
	case REGIME3270:
	case X3PAD:
	case TERMSPEED:
	case LINEMODE:
	case XDISPLOC:
	case ENVIRONMENT:
	case AUTHENTICATION:
	case DATA_ENCRYPTION:
	default:
		switch(otype)
		{
		case DOTEL:
		case DONTTEL:
			TelnetReply(WONTTEL, ecode);
			break;
		case WILLTEL:
		case WONTTEL:
			TelnetReply(DONTTEL, ecode);
			break;
		}
		break;
	}
	return tnPS_NORM;
}

/*
 * telnetSubnegTermtype - parse telnet subnegotiation of terminal type
 */
//***********************************************************************
TelParseState BtelnetStream::TelnetSubnegTermType(BYTE ecode)
{
	BYTE reply[128];
	int  wcnt;

	const char* termType = "xterm"; //"VT100";

	switch(ecode)
	{
	case SENDTTYPE:
		/* send terminal type */
		wcnt = 0;
		reply[wcnt++] = IAC;
		reply[wcnt++] = SB;
		reply[wcnt++] = TERMTYPE;
		reply[wcnt++] = REPLYTTYPE;
		strcpy((char*)&reply[wcnt], termType);
		wcnt += strlen(termType);
		reply[wcnt++] = IAC;
		reply[wcnt++] = SE;
		Write(reply, wcnt);
		return tnPS_NORM;
	default:
		return tnPS_NORM;
	}
}

/*
 * telneSubnegotiate - parse telnet subnegotiation
 */
//***********************************************************************
TelParseState BtelnetStream::TelnetSubnegotiate(BYTE ecode)
{
	LOG(logDebug, 5, _T(" subneg "_Pfs_"\n"), teloptions[ecode]);

	switch(ecode)
	{
	case TERMTYPE:
		return tnPS_SNTERMTYPE;
		break;
	default:
		break;
	}
	return tnPS_NORM;
}

//***********************************************************************
ERRCODE BtelnetStream::TelnetSetWindowSize(int w, int h)
{
	BYTE reply[32];
	int  wcnt = 0;

	reply[wcnt++] = IAC;
	reply[wcnt++] = SB;
	reply[wcnt++] = NAWS;
	reply[wcnt++] = (w >> 8) & 0xff;
	reply[wcnt++] = w & 0xff;
	reply[wcnt++] = (h >> 8) & 0xff;
	reply[wcnt++] = h & 0xff;
	reply[wcnt++] = IAC;
	reply[wcnt++] = SE;
	return Write(reply, wcnt);
}


//***********************************************************************
ERRCODE BtelnetStream::Read(LPBYTE pBuf, int& cnt)
{
	ERRCODE ec;
	int		i;
	BYTE	ic, cmd;
	LPBYTE	p, x;


	if(! m_ps)
		return errFAILURE;

	// read from the understream
	ec = m_ps->Read(pBuf, cnt);
	if(ec != errOK)
		return ec;

	// parse out any controls
	//
	for(i = 0, p = x = pBuf; i < cnt; i++)
	{
		switch(m_parseState)
		{
		case tnPS_IAC:			/* just received an iac */
			m_parseState = TelnetEscape(*p, &cmd);
			p++;
			break;
		case tnPS_ESCIAC:		/* just received an iac-iac */
			m_parseState = tnPS_NORM;
			*x++ = IAC;
			break;
		case tnPS_OPTION:		/* option */
			m_parseState = TelnetOption(cmd, *p);
			p++;
			break;
		case tnPS_SUBNEG:		/* subnegotiatoin */
			m_parseState = TelnetSubnegotiate(*p);
			p++;
			break;
		case tnPS_SNTERMTYPE:	/* subnegotiation terminal type */
			m_parseState = TelnetSubnegTermType(*p);
			p++;
			break;
		case tnPS_ERASEC:
			*x++ = 8;
			m_parseState = tnPS_NORM;
			break;
		case tnPS_NORM:
			if(*p == IAC)
			{
				p++;
				m_parseState = tnPS_IAC;
			}
			else
			{
				ic = *p++;

				switch(ic)
				{
				case '\0':
					break;
				case '\r':
					*x++ = ic;
					break;
				case '\n':
					*x++ = ic;
					break;
				default:
					*x++ = ic;
					break;
				}
			}
			break;
		}
	}
	cnt = (x - pBuf);
	return ec;
}

//***********************************************************************
ERRCODE BtelnetStream::Write(LPBYTE pBuf, int& cnt)
{
	if(m_ps) return m_ps->Write(pBuf, cnt);
	return errFAILURE;
}

//***********************************************************************
ERRCODE BtelnetStream::Close()
{
	if(m_ps) m_ps->Close();
	return errOK;
}

