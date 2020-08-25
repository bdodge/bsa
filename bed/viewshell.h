#ifndef _VIEWSHELL_H_
#define _VIEWSHELL_H_ 1


typedef enum
{
	vtactNone,
	vtactPosition,
	vtactRelPos,
	vtactVertPos,
	vtactHorzPos,
	vtactIndex,
	vtactAttribute,
	vtactClearLine,
	vtactClearScr,
	vtactLineHeight,
	vtactLineWidth,
	vtactSavePos,
	vtactRestorePos,
	vtactMode,
	vtactResetMode,
	vtactIdentify,
	vtactSetScroll,
	vtactIgnore,
	vtactSetTab,
	vtactClearTab,
	vtactSetLED,
	vtactBits,
	vtactANSIlevel,
	vtactGxSELECT,
	vtactGxSET,
	vtactSetCharset,
	vtactReset,
	vtactNewline,
	vtactPosReport,
	vtactStatReport,
	vtactParmReport,
	vtactBackSpace,
	vtactTab,
	vtactSelfTest,
	vtactRetitle,
	vtactStartGuard,
	vtactEndGuard,
	vtactStartString,
	vtactEndString,
	vtactScroll,
	vtactVertPad,
	vtactHorzPad,
	vtactRepeatChar,
	vtactDECmodeSet,
	vtactDECmodeReset,
	vtactUnknown
}
vtactCode;

typedef struct tag_vtact
{
	vtactCode	f_action;
	int			f_parms[6];
	int			f_valparm[6];
	int			f_parmnum;
}
BvtAction;

typedef enum
{
	vtcsDefault,		// 8859-1
	vtcsUTF8,			// UTF-8
	vtcsGraphics,
	vtcsUK,
	vtcsUSASCII,
	vtcsDutch,
	vtcsFinnish,
	vtcsFrench,
	vtcsCanuck,
	vtcsGerman,
	vtcsItalian,
	vtcsNorwegian,
	vtcsSpanish,
	vtcsSwedish,
	vtcsSwiss
}
BvtCharSet;

#define MAX_VT_CHARMODES	4

// VT100 update extents
//
#define vteaTOEOL		0
#define vteaFRBOL		1
#define vteaLINE		2

#define vteaTOEOS		0
#define vteaFRBOS		1
#define vteaSCREEN		2

// VT100 mode mask
//
#define vtmdLFNL		0x1		// send CRLF for Return if 1, just CR if 0
#define vtmdCURKEY		0x2		//
#define vtmdANSI		0x4		//
#define vtmdCOLS		0x8		//
#define vtmdSCROLL		0x10	//
#define vtmdSCREEN		0x20	// reverse video if 1
#define vtmdORIGIN		0x40	//
#define vtmdWRAP		0x80	// line wrap if 1
#define vtmdAUTO		0x100	//
#define vtmdINTER		0x200	//
#define vtmdGRAPH		0x400	// graphics mode
#define vtmdKEYPAD		0x800	// keypad = application
#define vtmdBEEP		0x1000	// beep disable if 1
#define vtmdECHO		0x2000	// echo typed chars if 1
#define vtmdBSDEL		0x4000 	// send DEL for ^H if 1
#define vtmdXTERM		0x8000	//
#define vtmdLOG			0x10000	// log to file if 1

// VT100 char display attribute mask
//
#define vtdaNONE		0x00
#define vtdaBOLD		0x80
#define vtdaUNDER		0x40
#define vtdaBLINK		0x20
#define vtdaREVERSE		0x10
#define vtdaSTOPUNDER	0x01

// VT100 parse state
//
enum vtParseState
{
	vtpsINIT,
	vtpsESCAPE,
	vtpsESBRA,
	vtpsESPAR,
	vtpsESRAP,
	vtpsGETY,
	vtpsGETX,
	vtpsPOUND,
	vtpsSPACE,
	vtpsDONE,
	vtpsESARB,
	vtpsESSTAR,
	vtpsESPLUS,
	vtpsESPERCENT,
	vtpsESCARET,
	vtpsESBRAQ,
	vtpsESUND
};

// VT100 region affect flag
//
#define vtrcPOSITION	1
#define vtrcDISPLAY		2
#define vtrcLINE		4
#define vtrcFRAMESET	8
#define vtrcSCROLLREG	0x10
#define vtrcRESTOFSCR	0x20
#define vtrcSCREEN		0x30

// token attribute markers
//
#define VT_ATTRIB_MARKER 17
#define VT_FRG_MARKER 	 18
#define VT_BKG_MARKER 	 19
	

//**************************************************************************
class BviewTerminal : public Bview, public Bthread
{
public:
	BviewTerminal					(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel);
	virtual ~BviewTerminal			();

public:
	virtual void	ResetSettings	(void);
	virtual void	Activate		(void);
	virtual ERRCODE WriteLog		(void);
	virtual ERRCODE Dispatch		(EditCommand command);
	virtual void	SpecificMouseMenu(HMENU hMenu);
	virtual void	MouseSpecial	(int x, int y);
	virtual bool	IsPlainText		(void);
	virtual TokenRet GetToken		(
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
									);
	virtual ERRCODE SendData		(LPBYTE data, int len);
	virtual ERRCODE Key				(WORD key);
	virtual LPCTSTR	GetTypeName		(void)	{ return _T("Shell");	}
	virtual void	Event			(LPARAM lParam);
	virtual bool	InsertPointVisible(void);
	virtual DWORD	GetMode			(void)					{ return m_vtmode; }
	virtual void	SetModeBit		(unsigned long modebit) { m_vtmode |= modebit;  }
	virtual void	ClearModeBit	(unsigned long modebit) { m_vtmode &= ~modebit; }
	virtual BvtEmulation GetEmulation(void)					{ return m_emulation; }
	virtual void	SetEmulation	(BvtEmulation emul)		{ m_emulation = emul; UpdateView(1, 1, MAX_LINE_LEN, MAX_LINE_LEN); }

	static int		EmulationDialog	(Bed* pBed, BviewTerminal* pTerm, LPCTSTR pType, HWND hWndParent);
	static int		EmulationDialog	(BviewTerminal* pTerm, HWND hWndParent);
	static
	 ERRCODE WINAPI	RunStub			(LPVOID thisptr);

	static vtParseState 
					ParseVT100		(
										char ic,
										vtParseState parstate,
										BvtEmulation emulation,
										BvtAction&   action,
										int loglevel
									);
	virtual void	FitToPanel		(void);

protected:
	virtual COLORREF VTrgbColor				(char color, int isbold);
	virtual int		VTformatToken			(LPCTSTR intok, int nintok, LPTSTR tok, int ntok);
	virtual void	SetCurrentAttributes	(int attron, int attroff, char frg, char bkg);
	virtual void	CheckFrame				(bool extend_udpate_region);
	virtual void	UpdateVtPos				(int line, int col);
	virtual void	InsertToken				(bool insertit = false);
	virtual ERRCODE	ShellThread				(void);

	static void CALLBACK OnTimer(HWND hWnd, UINT message, UINT id, DWORD now);

protected:
	// ring buffer for shell->parse comm
	Bmutex			m_bufex;
	unsigned char	m_iobuf[SHELL_IO_SIZE + 2];
	int				m_head;
	int				m_tail;
	int				m_cnt;
	int				m_size;

	// token buffer for tokenizing shell data
	TCHAR			m_outtok[SHELL_IO_SIZE * 3 + 2];
	TCHAR			m_intok[SHELL_IO_SIZE * 3 + 2];
	TCHAR			m_szInp[SHELL_IO_SIZE * 3 + 2];
	int				m_tokcnt;
	int				m_tokbase;

	UINT_PTR		m_idTimer;
	bool			m_closeonexit;
	bool			m_noexitprompt;
	int				m_insertcount;

	bool			m_LFeqNL;
	bool			m_BSeqDEL;
	bool			m_suppressBeep;
	vtParseState	m_vtState;
	BvtAction		m_action;
	BvtEmulation	m_emulation, m_ansiLevel;
	BvtCharSet		m_charSets[MAX_VT_CHARMODES];
	int				m_ctrlBits;

	Blog*			m_log;
	
	// virtual VT220 state
	int				m_vtrows;	// screen extent rows of text
	int				m_vtcols;	// screen extent columns of text
	int				m_vtrow;	// "vt cursor" postion Y
	int				m_vtcol;	// "vt cursor" postion X
	int				m_vvtrow;	// parms saved on "save cursor position"
	int				m_vvtcol;	// row and col
	int				m_svvtrow;	// saved "vt cursor" postion Y
	int				m_svvtcol;	// saved "vt cursor" postion X
	int				m_minvtrow; // previous min Y, for update region
	int				m_maxvtrow; // previous max Y, for update region
	int				m_prevvtcol;// previous X, for update region
	char			m_curfrg;	// current set frg color index
	char			m_curbkg;	// current set bkg color index
	bool			m_deffrg;	// use default frg color
	bool			m_defbkg;	// use default bkg color
	int				m_frameline;// offset into buffer of actual "top of screen"
	int				m_scrlt;	// top of scroll region
	int				m_scrlb;	// bottom of scroll region
	long			m_vtmode;	// vtmode as per vt100 spec
	int				m_charset;	// current char set index (G0, G1)
	int				m_attribs;	// draw attributes
	int				m_view_scheme;
	TCHAR			m_logdir[MAX_PATH+2];
	TCHAR			m_logName[MAX_PATH+2];
	int				m_totalcnt;
	int				m_logthresholdsize;
	int				m_logthresholdtime;
};

//**************************************************************************
class BviewShell : public BviewTerminal, public Bshell
{
public:
	BviewShell						(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel);
	virtual ~BviewShell				();

public:
	virtual void	Activate		(void);
	virtual ERRCODE SendData		(LPBYTE data, int len);
	virtual LPCTSTR	GetTypeName		(void)	{ return _T("Shell");	}
	virtual void	Event			(LPARAM lParam);
	static
	 ERRCODE WINAPI	RunStub			(LPVOID thisptr);
	virtual void	FitToPanel		(void);

protected:
	bool			m_destroying;
	virtual ERRCODE	ShellThread		(void);

protected:
};

// base class for a view on a r/w stream (e.g rs232, or telnet, etc)
//**************************************************************************
class BviewStream : public BviewTerminal
{
public:
	BviewStream						(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel);
	virtual ~BviewStream			();

public:
	virtual ERRCODE	ReopenPort		(void);
	virtual void	Activate		(void);
	virtual ERRCODE SendData		(LPBYTE data, int len);
	virtual LPCTSTR	GetTypeName		(void) = 0;
	virtual void	Event			(LPARAM lParam);
	static
	 ERRCODE WINAPI	RunStub			(LPVOID thisptr);
	virtual void	FitToPanel		(void);
	virtual int		SettingsDialog	(Bed* pEditor, HWND hWndParent);

protected:
	virtual ERRCODE	ApplyPortSettings(void);
	virtual ERRCODE	GetPortSettings	(void);
	virtual ERRCODE	ShellThread		(void);

protected:
	Bstream*  		m_io;
	bool			m_iochanging;
	bool			m_zeroreaderr;
};

//**************************************************************************
class BviewSSH : public BviewShell
{
public:
	BviewSSH						(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel);
	virtual ~BviewSSH				();

public:
	virtual void	Activate		(void);
	virtual void	FitToPanel		(void);
	virtual LPCTSTR	GetTypeName		(void)	{ return _T("SecureShell");	}
	virtual int     GetPort			(void)  { return m_port; }
	virtual LPCTSTR GetHost			(void)  { return m_host; }
	static	int		SettingsDialog	(Bed* pEditor, BviewTerminal* pTerm, HWND hWndParent);

protected:
	virtual ERRCODE	ApplyPortSettings(void);
	virtual ERRCODE	GetPortSettings	(void);

protected:
	bool			m_crnl, m_nobell, m_novt;
	TCHAR			m_host[MAX_PATH+2];
	int				m_port;
};


//**************************************************************************
class BviewRS232 : public BviewStream
{
public:
	BviewRS232						(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel);
	virtual ~BviewRS232				();

public:
	virtual void	Activate		(void);
	virtual LPCTSTR	GetTypeName		(void)	{ return _T("RS232");	}
	virtual void	Event			(LPARAM lParam);
	virtual LPCTSTR GetPort			(void)  { return m_port; }
	virtual void	SetPort			(LPCTSTR port)  { _tcsncpy(m_port, port, MAX_PATH); m_port[MAX_PATH - 1] = _T('\0'); }
	virtual int		SettingsDialog	(HWND hWndParent);

protected:
	virtual ERRCODE	ApplyPortSettings(void);
	virtual ERRCODE	GetPortSettings	(void);

protected:
	int				m_baud, m_bits, m_stops, m_parity, m_flow;
	TCHAR			m_port[MAX_PATH];
};

//**************************************************************************
class BviewTelnet : public BviewStream
{
public:
	BviewTelnet						(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel);
	virtual ~BviewTelnet			();

public:
	virtual void	Activate		(void);
	virtual void	FitToPanel		(void);
	virtual LPCTSTR	GetTypeName		(void)	{ return _T("Telnet");	}
	virtual int     GetPort			(void)  { return m_port; }
	virtual LPCTSTR GetHost			(void)  { return m_host; }
	static	int		SettingsDialog	(Bed* pEditor, BviewTerminal* pTerm, HWND hWndParent);

protected:
	virtual ERRCODE	ApplyPortSettings(void);
	virtual ERRCODE	GetPortSettings	(void);

protected:
	bool			m_crnl, m_nobell, m_novt;
	TCHAR			m_host[MAX_PATH+2];
	int				m_port;
};

#endif


