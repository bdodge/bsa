
#ifndef _PRINT_H_
#define _PRINT_H_ 1

typedef enum
{
	prSelection,
	prWindow,
	prPageRange,
	prDocument
}
BprintRange;

typedef struct
{
	int			code;	// corresponding DM paper code
	int			w, h;	// dimensions in 1/10 mm
	int			x, y;	// dimensions in pixels at 1200 dpi
	int			t,b;	// margins in pixels top/bottom
	int			l,r;	// margins in pixels left/right
	LPCTSTR		pName;	// string ID of name of paper
}
PTAB, *LPPTAB;


class Bprinter
{
public:
	Bprinter(Bed* pBed);
	~Bprinter();

public:
	virtual	ERRCODE		SetupPage		(void);
	virtual ERRCODE		SetupPrinter	(BprintRange range);
	virtual int			GetStartPage	(void)	{ return m_startpage;	  }
	virtual int			GetEndPage		(void)	{ return m_endpage;		  }
	static LPCTSTR		GetDriver		(void)	{ return m_printerDriver; }
	static LPCTSTR		GetPrinterName	(void)	{ return m_printerName;   }
	static int			GetPaperCode	(void)	{ return m_papersize;	  }
	static void			SetPaperCode	(int code) { m_papersize = code;  }

public:
	static PTAB			m_paperTable[];

protected:
	Bed*				m_editor;

	static int			m_startpage;
	static int			m_endpage;
	static int			m_copies;
	static int			m_pagecount;
	static int			m_papersize;
	static int			m_resX;
	static int			m_resY;
	static int			m_resZ;
	static int			m_extX;
	static int			m_extY;
	static TCHAR		m_printerDriver[MAX_PATH];
	static TCHAR		m_printerName[MAX_PATH];
	static TCHAR		m_printerPort[MAX_PATH];
};

#endif
