
#define BC_MAX_STAGE	256

enum BCbuttonClass	{ 
	bcDigit,
	bcUnOperator, bcBinOperator, bcSciTrigUnOperator, bcSciTrigOperator,
	bcProgram, bcControl, bcDisplay, bcAppControl, bcModeButton, bcEdit, bcLabel
};

typedef struct tag_bcalc_cv* PBCALC_CV;

class Bnumber;
class Bstack;
class Bprog;
class Bplot;

enum NumberFormat	{ kbInteger, kbUnsignedInteger, kbFloat, kbArbitrary };
enum DisplayFocus	{ kbDisplay, kbXdisplay };
enum DisplayFormat	{ kbDefault, kbScientific, kbEngineer };
enum ProgramMode	{ pmEdit, pmRecord, pmRun };

typedef unsigned short CODE;

//**************************************************************************
class Bcalc : public BappPane
{
public:
	Bcalc(Bpersist* pPersist, BappPanel* pPanel, bool isdark);
	~Bcalc();

public:
	virtual void		OnDrawDisplay	(HDC hdc, HWND hWnd, LPRECT prc);
	virtual ERRCODE		OnKey			(CODE dc);
	virtual void 		MouseMenu		(int x, int y);
	virtual LPCSTR		GetStage		(void)	{ return m_stage;			};
	virtual Bnumber*	GetTop			(void);
	virtual	void		FitToPanel		(void);
	virtual LRESULT		OnMessage		(HWND, UINT, WPARAM, LPARAM);
	virtual ERRCODE		RestoreSettings	(void);
	virtual ERRCODE		SaveSettings	(void);
	virtual void		GetButtonSize	(int* pw, int* ph);
	virtual Bpersist*	GetPersist		(void)	{ return m_persist;			};
	virtual int			GetRadix		(void)	{ return m_radix;			};
	virtual ProgramMode	GetProgramMode	(void)	{ return m_progmode;		};
	virtual void		SetProgramMode	(ProgramMode mode)	{ m_progmode = mode; SetupWindows(false); };
	virtual Bnumber*	MakeNumber		(LPCSTR digits, int radix, CODE code);
	virtual BappPanel*	GetProgPanel	(void)	{ return m_progpanel;		};
	virtual BappPanel*	GetPlotPanel	(void)	{ return m_plotpanel;		};
	virtual ERRCODE		Store			(Bnumber*  pn, int id);
	virtual ERRCODE		Recall			(Bnumber** pn, int id);
	virtual ERRCODE		Record			(Bnumber* pn);
	virtual ERRCODE		Record			(CODE o);
	virtual ERRCODE		Play			(Bnumber* pn);
	virtual ERRCODE		Play			(CODE o);
	virtual void		AddConstant		(LPCTSTR pcv);
	virtual void		AddConversion	(LPCTSTR pcv);

protected:
	virtual LPCTSTR		MemMenuStr		(int id, LPCTSTR label, LPTSTR buf, DWORD nbuf);
	virtual ERRCODE		DoMemAccess		(CODE dc);
	virtual ERRCODE		DoConst			(void);
	virtual ERRCODE		SetupWindows	(bool docreate);
	virtual void		UpdateDisplay	(HWND hWnd);
	virtual Bnumber*	ReformatNumber	(Bnumber* pn, NumberFormat from, NumberFormat to);
	virtual ERRCODE		ReformatStack	(NumberFormat from, NumberFormat to);
	virtual ERRCODE		CommitStage		(CODE o);
	virtual ERRCODE		StageDigit		(CHAR d);
	virtual int			Precedence		(CODE o);
	virtual bool		IsLegalKey		(CODE id, int radix, NumberFormat format);

public:
	bool				m_isdark;
	int					m_scTextBkg;
	int					m_scAppBkg;
	COLORREF			m_dispFrg1, m_dispFrg2, m_dispBkg;
	HBRUSH				m_hbrDisplay;
	HBRUSH				m_hbrDigit1;
	HBRUSH				m_hbrDigit2;
	HBRUSH				m_hbrPlot;
	HBRUSH				m_hbrProg;
	HFONT				m_hf_c;
	HFONT				m_hf_n;

protected:
	enum				{ kbBlank, kbNumber, kbFraction, kbExponent, kbError } m_state; 
	CHAR				m_stage[BC_MAX_STAGE];
	int					m_cursor;
	CHAR				m_expstage[BC_MAX_STAGE];
	int					m_expcursor;
	NumberFormat		m_mathformat;
	DisplayFormat		m_dispformat;
	DisplayFocus		m_focus;
	int					m_radix;
	int					m_xradix;
	int					m_bitsize;
	ProgramMode			m_progmode;
	Bnumber*			m_mem[26];
	Bstack*				m_ps;
	Bpersist*			m_persist;
	BappPanel*			m_progpanel;
	BappPanel*			m_plotpanel;
	PBCALC_CV			m_constants;
	PBCALC_CV			m_conversions;
	bool				m_view_scitrig;
	bool				m_view_program;
	HWND				m_hwndDisplay;
	HWND				m_hwndXdisplay;
	HWND				m_hwndXinfo, m_hwndFinfo;
	HWND				m_hwnd_drup, m_hwnd_xrup;
	HWND				m_hwnd_drdn, m_hwnd_xrdn;
	HWND				m_hwnd_nfrmtup, m_hwnd_nfrmtdn;
	HWND				m_hwnd_dfrmtup, m_hwnd_dfrmtdn;
};

//**************************************************************************
struct BCbutton
{
public:

public:
	int					m_id;
	TCHAR				m_name[32];
	int					m_w;
	int					m_h;
	BCbuttonClass		m_class;
	int					m_on;
	HWND				m_hwnd;
};

typedef BCbutton* PBCBUTTON;


#define BCALC_DISP_H	44
#define BCALC_DISP_TM	4
#define BCALC_DISP_LM	2
#define BCALC_XDISP_X	124
#define BCALC_XDISP_H	30
#define BCALC_BUT_LM	2
#define BCALC_BUT_TM	1
#define BCALC_BUT_MX	2
#define BCALC_BUT_MY	2

#define BCALC_RADBUT_W	16
#define BCALC_RADBUT_H	BCALC_XDISP_H / 2


#define BUT_DIG_W 40
#define BUT_DIG_H 28

#define BCALC_BUT_MAX_W	60
#define BCALC_BUT_MIN_W	16
#define BCALC_BUT_MAX_H	30
#define BCALC_BUT_MIN_H	10


#define ID_NUM_0		'0'
#define ID_NUM_1		'1'
#define ID_NUM_2		'2'
#define ID_NUM_3		'3'
#define ID_NUM_4		'4'
#define ID_NUM_5		'5'
#define ID_NUM_6		'6'
#define ID_NUM_7		'7'
#define ID_NUM_8		'8'
#define ID_NUM_9		'9'
#define ID_NUM_A		'A'
#define ID_NUM_B		'B'
#define ID_NUM_C		'C'
#define ID_NUM_D		'D'
#define ID_NUM_E		'E'
#define ID_NUM_F		'F'

#define ID_OPR_ADD 		'+'
#define ID_OPR_SUB		'-'
#define ID_OPR_MUL		'*'
#define ID_OPR_DIV		'/'
#define ID_OPR_MOD		'm'
#define ID_OPR_NEG		'n'
#define ID_OPR_AND		'&'
#define ID_OPR_OR		'|'
#define ID_OPR_XOR		'^'
#define ID_OPR_NOT		'~'
#define ID_OPR_SHFTL	'<'
#define ID_OPR_SHFTR	'>'
#define ID_OPR_EQUAL	'='
#define ID_OPR_LPAREN	'('
#define ID_OPR_RPAREN	')'
#define ID_OPR_INT		'i'
#define ID_OPR_FRAC		'f'
#define ID_OPR_SQ		'q'
#define ID_OPR_SQRT		'u'
#define ID_OPR_INV		'v'
#define ID_OPR_NOOP		' '

#define ID_OPR_SIN		'S'
#define ID_OPR_COS		'O'
#define ID_OPR_TAN		'T'
#define ID_OPR_ASIN		'I'
#define ID_OPR_ACOS		'J'
#define ID_OPR_ATAN		'N'
#define ID_OPR_EXPO		'P'
#define ID_OPR_NLOG		'L'
#define ID_OPR_XEXPY	'X'
#define ID_OPR_10EXP	'Z'
#define ID_OPR_LOG		'G'

#define ID_CTL_CLR		'c'
#define ID_CTL_CLRA		'a'
#define ID_CTL_DEL		'd'
#define ID_CTL_STR		's'
#define ID_CTL_RCL		'r'
#define ID_CTL_EXP		'p'
#define ID_CTL_DEC		'.'
#define ID_CTL_SCI		'l'
#define ID_CTL_CONST	'?'
#define ID_CTL_PRG		'g'
#define ID_CTL_VPLOT	'!'
#define ID_CTL_QUIT		'z'
#define ID_CTL_COPY		'o'
#define ID_CTL_PASTE	't'

#define ID_RADIX_UP		'\01'
#define ID_RADIX_DOWN	'\02'
#define ID_XRADIX_UP	'\03'
#define ID_XRADIX_DOWN	'\04'
#define ID_FORMAT_UP	'\05'
#define ID_FORMAT_DOWN	'\06'
#define ID_DFRMT_UP		'\010'
#define ID_DFRMT_DOWN	'\011'

#define ID_PRG_REC		'@'
#define ID_PRG_RUN		'{'
#define ID_PRG_STEP		'$'
#define ID_PRG_STOP		'}'
#define ID_PRG_INS		'#'
#define ID_PRG_DEL		'\\'
#define ID_PRG_CLR		','
#define ID_PRG_LOAD		'\014'
#define ID_PRG_SAVE		'\015'
#define ID_PRG_PLOT2	'['
#define ID_PRG_PLOT3	']'

#define ID_PRG_XMIN		1024
#define ID_PRG_XMAX		1025
#define ID_PRG_XSTEP	1026

#define ID_PRG_YMIN		1028
#define ID_PRG_YMAX		1029
#define ID_PRG_YSTEP	1030

#define ID_PRG_LOOP		800
#define ID_PRG_POOL		801
#define ID_PRG_BREAK	802
#define ID_PRG_LT		803
#define ID_PRG_GT		804
#define ID_PRG_LE		805
#define ID_PRG_GE		806
#define ID_PRG_EQ		807
#define ID_PRG_FI		808

#define ID_MEM_X		2048
#define ID_MEM_Y		2049
#define ID_MEM_Z		2050
#define ID_MEM_A		2051
#define ID_MEM_B		2052
#define ID_MEM_C		2053
#define ID_MEM_D		2054
#define ID_MEM_E		2055
#define ID_MEM_F		2056
#define ID_MEM_G		2057
#define ID_MEM_H		2058
#define ID_MEM_I		2059
#define ID_MEM_J		2060
#define ID_MEM_K		2061
#define ID_MEM_L		2062
#define ID_MEM_M		2063
#define ID_MEM_N		2064
#define ID_MEM_O		2065
#define ID_MEM_P		2066
#define ID_MEM_Q		2067
#define ID_MEM_R		2068
#define ID_MEM_S		2069
#define ID_MEM_T		2070
#define ID_MEM_U		2071
#define ID_MEM_V		2072
#define ID_MEM_W		2073

#define ID_RCL_X		3048
#define ID_RCL_Y		3049
#define ID_RCL_Z		3050
#define ID_RCL_A		3051
#define ID_RCL_B		3052
#define ID_RCL_C		3053
#define ID_RCL_D		3054
#define ID_RCL_E		3055
#define ID_RCL_F		3056
#define ID_RCL_G		3057
#define ID_RCL_H		3058
#define ID_RCL_I		3059
#define ID_RCL_J		3060
#define ID_RCL_K		3061
#define ID_RCL_L		3062
#define ID_RCL_M		3063
#define ID_RCL_N		3064
#define ID_RCL_O		3065
#define ID_RCL_P		3066
#define ID_RCL_Q		3067
#define ID_RCL_R		3068
#define ID_RCL_S		3069
#define ID_RCL_T		3070
#define ID_RCL_U		3071
#define ID_RCL_V		3072
#define ID_RCL_W		3073

#define ID_STR_X		4048
#define ID_STR_Y		4049
#define ID_STR_Z		4050
#define ID_STR_A		4051
#define ID_STR_B		4052
#define ID_STR_C		4053
#define ID_STR_D		4054
#define ID_STR_E		4055
#define ID_STR_F		4056
#define ID_STR_G		4057
#define ID_STR_H		4058
#define ID_STR_I		4059
#define ID_STR_J		4060
#define ID_STR_K		4061
#define ID_STR_L		4062
#define ID_STR_M		4063
#define ID_STR_N		4064
#define ID_STR_O		4065
#define ID_STR_P		4066
#define ID_STR_Q		4067
#define ID_STR_R		4068
#define ID_STR_S		4069
#define ID_STR_T		4070
#define ID_STR_U		4071
#define ID_STR_V		4072
#define ID_STR_W		4073

#define ID_BAD_CODE		9000

