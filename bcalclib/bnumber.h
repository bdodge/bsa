
#ifdef Windows
typedef __int64 uberlong;
typedef unsigned __int64 unsigneduberlong;
#else
typedef long long uberlong;
typedef unsigned long long unsigneduberlong;
#endif

typedef enum { bcPlain, bcExp, bcSci } BCNUMFORMAT;

//**************************************************************************
class Bnumber
{
public: 	Bnumber(CODE code) : m_next(NULL), m_code(code)		{		};
	Bnumber(LPCSTR pt)				{ 							};
	virtual ~Bnumber()				{							};

public:
	virtual LPCSTR		Format		(int maxdigits, int radix, DisplayFormat format);
	virtual bool		IsNegative	(void)						{ return false; };
	virtual bool		IsZero		(void)						{ return false; };

	virtual	Bnumber*	Negate		(Bnumber* pn, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	BitInvert	(Bnumber* pn, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	Invert		(Bnumber* pn, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	Floor		(Bnumber* pn, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	Frac		(Bnumber* pn, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	Add			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	Sub			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	Mul			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	Div			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	Mod			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	And			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	Or			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	Xor			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	ShiftLeft	(Bnumber* pn, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	ShiftRight	(Bnumber* pn, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	Sqrt		(Bnumber* pn, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };

	virtual Bnumber*	Sin			(Bnumber* pn, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	Cos			(Bnumber* pn, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	Tan			(Bnumber* pn, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	aSin		(Bnumber* pn, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	aCos		(Bnumber* pn, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	aTan		(Bnumber* pn, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	Exp			(Bnumber* pn, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	nLog		(Bnumber* pn, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	TenX		(Bnumber* pn, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	Log			(Bnumber* pn, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };
	virtual Bnumber*	XtoY		(Bnumber* pna, Bnumber* pnb, ERRCODE& ec) { ec = errNOT_IMPLEMENTED; return NULL; };

protected:

public:
	class Bnumber*	m_next;
	CODE			m_code;

protected:
	static LPSTR	m_rb;
	static DWORD	m_nrb;

};

//**************************************************************************
class BintNumber : public Bnumber
{
public:
	BintNumber(uberlong n, CODE code);
	BintNumber(LPCSTR pt, int radix, CODE code);
	virtual ~BintNumber()			{							};

public:
	virtual LPCSTR		Format		(int digits, int radix, DisplayFormat format);
	virtual bool		IsNegative	(void);
	virtual bool		IsZero		(void);

	virtual	Bnumber*	Negate		(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	BitInvert	(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	Invert		(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	Add			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	Sub			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	Mul			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	Div			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	Mod			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	And			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	Or			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	Xor			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	ShiftLeft	(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	ShiftRight	(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	XtoY		(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);

protected:
public:
protected:
	uberlong		m_val;
};

//**************************************************************************
class BuintNumber : public Bnumber
{
public:
	BuintNumber(uberlong n, CODE code);
	BuintNumber(LPCSTR pt, int radix, CODE code);
	virtual ~BuintNumber()			{							};

public:
	virtual LPCSTR		Format		(int digits, int radix, DisplayFormat format);
	virtual bool		IsNegative	(void);
	virtual bool		IsZero		(void);

	virtual Bnumber*	BitInvert	(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	Invert		(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	Add			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	Sub			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	Mul			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	Div			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	Mod			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	And			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	Or			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	Xor			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	ShiftLeft	(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	ShiftRight	(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	XtoY		(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);

protected:
public:
protected:
	unsigneduberlong	m_val;
};

//**************************************************************************
class BdblNumber : public Bnumber
{
public:
	BdblNumber(double n, CODE code);
	BdblNumber(LPCSTR pt, int radix, CODE code);
	virtual ~BdblNumber()			{							};

public:
	virtual LPCSTR		Format		(int digits, int radix, DisplayFormat format);
	virtual bool		IsNegative	(void);
	virtual bool		IsZero		(void);

	virtual	Bnumber*	Negate		(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	Invert		(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	Add			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	Sub			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	Mul			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	Div			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	Floor		(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	Frac		(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	Sqrt		(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	Sin			(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	Cos			(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	Tan			(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	aSin		(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	aCos		(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	aTan		(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	Exp			(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	nLog		(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	TenX		(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	Log			(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	XtoY		(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);

protected:
public:
protected:
	double			m_val;
};

//**************************************************************************
class BarbNumber : public Bnumber
{
public:
	BarbNumber(BarbNumber* n, CODE code);
	BarbNumber(LPCSTR pt, int radix, CODE code);
	virtual ~BarbNumber()			{	if(m_val) delete [] m_val;	};

public:
	virtual LPCSTR		Format		(int digits, int radix, DisplayFormat format);
	virtual bool		IsNegative	(void);
	virtual bool		IsZero		(void);

	virtual	Bnumber*	Negate		(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	Invert		(Bnumber* pn, ERRCODE& ec);
	virtual Bnumber*	Add			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	Sub			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	Mul			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	Div			(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);
	virtual Bnumber*	XtoY		(Bnumber* pna, Bnumber* pnb, ERRCODE& ec);

protected:
	virtual int			Compare		(BarbNumber* pna, BarbNumber* pnb);

public:
protected:
	CHAR*				m_val;
	long				m_alloc;
	long				m_len;
	int					m_radix;
	bool				m_neg;
};

#define BC_MAX_STACK 128;

typedef enum { toTop, toBottom, toParen } execRange;

//**************************************************************************
class Bstack
{
public:
	Bstack() : m_top(NULL)			{							};
	~Bstack()						{							};

public:
	virtual Bnumber*	Push		(Bnumber* pn);
	virtual Bnumber*	Pop			(CODE& code);
	virtual ERRCODE		Execute		(execRange range);
	virtual Bnumber*	GetTop		(void)	{ return m_top; }
	virtual void		Clear		(void);
	virtual void		Dump		(LPCTSTR msg);

protected:

protected:
	Bnumber*			m_top;
};

