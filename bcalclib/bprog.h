
//**************************************************************************
typedef struct tag_bcalc_cv
{
public:
	tag_bcalc_cv(LPCTSTR pcv)	{ m_next = NULL; m_cv = new TCHAR [ _tcslen(pcv) + 1 ]; _tcscpy(m_cv, pcv); };
	~tag_bcalc_cv()				{ if(m_cv) delete [] m_cv; };

public:
	LPTSTR					m_cv;
	struct tag_bcalc_cv*	m_next;
}
BCALC_CV, *PBCALC_CV;

//**************************************************************************
typedef struct tag_prg_e
{
public:
	enum { petNumber, petCode } m_type;

public:
	tag_prg_e(Bnumber* pn)
	{
		m_num  = pn;
		m_code = ' ';
		m_type = petNumber;
	};
	tag_prg_e(CODE o)
	{
		m_num  = NULL;
		m_code = o;
		m_type = petCode;
	};
	~tag_prg_e()
	{
		if(m_num) delete m_num;
	};
	bool IsNumber	(void)		{ return m_type == petNumber; };
	bool IsCode		(void)		{ return m_type == petCode;   };

public:
	Bnumber*			m_num;
	CODE				m_code;
	struct tag_prg_e*	m_prev;
	struct tag_prg_e*	m_next;
}
PRGENTRY, *PPRGENTRY;

//**************************************************************************
class Bprog : public BappPane
{
public:
	Bprog(Bcalc* pCalc, BappPanel* pParent);
	virtual ~Bprog();

public:
	virtual void		FitToPanel		(void);
	virtual LRESULT		OnMessage		(HWND, UINT, WPARAM, LPARAM);

	virtual PPRGENTRY	GetProgram		(void)				{ return m_prog; }
	virtual PPRGENTRY	GetProgramCursor(void)				{ return m_cur;	 }
	virtual PPRGENTRY	GetErrorCursor	(void)				{ return m_err;	 }
	virtual ERRCODE		Insert			(Bnumber* pn);
	virtual ERRCODE		Insert			(CODE o);
	virtual ERRCODE		InsertControl	(void);
	virtual ERRCODE		Delete			(void);
	virtual ERRCODE		ClearProgram	(void);
	virtual ERRCODE		RunProgram		(bool issub);
	virtual ERRCODE		StepProgram		(bool issub);
	virtual ERRCODE		StopProgram		(void);
	virtual ERRCODE		PlotProgram		(int d);
	        LRESULT		FileNameDialog	(HINSTANCE hinst, LPCTSTR titleText);
	virtual ERRCODE		LoadProgram		(void);
	virtual ERRCODE		SaveProgram		(void);
	virtual ERRCODE		MoveRel			(int to);
	static	LPCTSTR		CodeName		(CODE o);
	static	CODE		CodeFromName	(LPCTSTR name);
	virtual void		SetEditScroller	(void);
	virtual Bcalc*		GetCalculator	(void)				{ return m_bcalc; }
	virtual ERRCODE		StopRecording	(void);

protected:
	virtual bool		EvalCondition	(CODE cond);
	virtual void		SetupWindows	(bool docreate);
	virtual bool		GetButtonState	(int id);
	virtual void		SetButtonState	(int id, bool on);

public:
	TCHAR				m_filename[MAX_PATH];
	int					m_yoff;
	int					m_th;
	int					m_wh;

protected:
	PPRGENTRY			m_prog;
	PPRGENTRY			m_cur;
	PPRGENTRY			m_err;
	bool				m_dirty;
	Bcalc*				m_bcalc;
	HWND				m_hwndEdit;
};

#define penBlack	0
#define penBlue		1
#define penRed		2
#define penGreen	3

#define BCALC_NUM_PENS 4

//**************************************************************************
typedef struct tag_pnt
{
public:
	tag_pnt(int x, int y) { m_x = x; m_y = y; m_anchor = false; m_next = NULL; };

public:
	int				m_x, m_y;
	bool			m_anchor;
	struct tag_pnt* m_next;
}
PNT, *PPNT;


//**************************************************************************
class Bplot : public BappPane
{
public:
	Bplot(Bcalc* pCalc, BappPanel* pParent);
	virtual ~Bplot();

public:
	virtual ERRCODE		SetPlotExtents	(int d, double sx, double sy, double ex, double ey, double xs, double ys);
	virtual ERRCODE		GetPlotExtents	(int& d, double& sx, double& sy, double& ex, double& ey, double& xs, double& ys);
	virtual ERRCODE		StartPlot		(int d);
	virtual ERRCODE		EndPlot			(void);
	virtual ERRCODE		PlotY			(double x, double y);
	virtual ERRCODE		PlotZ			(double x, double y, double z);
	virtual void		Anchor			(void) { m_firstpoint = true; }
	virtual LRESULT		OnMessage		(HWND, UINT, WPARAM, LPARAM);
	virtual void		FitToPanel		(void);

protected:
	virtual void		DeletePoints	(void);
	virtual void		SetupWindows	(bool docreate);

public:
protected:
	Bcalc*				m_bcalc;
	PPNT				m_pnts;
	PPNT				m_cpnt;
	PPNT				m_xaxis;
	PPNT				m_yaxis;
	PPNT				m_zaxis;
	HDC					m_hdc;
	HPEN				m_hop;
	HPEN				m_pens[BCALC_NUM_PENS];
	bool				m_firstpoint;
	int					m_dim;
	double				m_sx, m_sy, m_ex, m_ey, m_xs, m_ys;
	double				m_xscale, m_yscale, m_zscale;
	double				m_xoffset, m_yoffset;
	double				m_sinxa, m_cosxa;
	double				m_sinya, m_cosya;
	double				m_ax, m_ay;
};
