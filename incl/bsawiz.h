//--------------------------------------------------------------------
//
// File: bsawiz.h
// Desc: "wizard"
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------

#ifndef _BSAWIZ_H_
#define _BSAWIZ_H_ 1

#ifdef BSAWIZ_EXPORTS
	#define BSAWIZ_API __declspec(dllexport)
#elif defined(BSAWIZ_DLL)
	#define BSAWIZ_API __declspec(dllimport)
#else
	#define BSAWIZ_API
#endif


#define EB_FINISH	0x8
#define EB_BACK		0x4
#define EB_NEXT		0x2
#define EB_CANCEL	0x1

#define IDC_BACK	200
#define IDC_NEXT	201
#define IDC_CANCEL	202


#define IDC_FIRST_CONTROL	300

//-----------------------------------------------------------------------
//
enum BwizItemType { wwiLabel, wwiChoice,  wwiChoiceGroup, wwiCheckbox, wwiEdit, wwiInitialBitmap, wwiIconBitmap };

//***********************************************************************
typedef struct _tag_wwi
{
	_tag_wwi(HWND hWnd) { f_hwnd = hWnd; }
	~_tag_wwi() { DestroyWindow(f_hwnd); }
	BwizItemType	f_type;
	int				f_ival;
	int				f_panel;
	TCHAR			f_sval[MAX_PATH*2];
	TCHAR			f_varname[MAX_PATH];
#if defined(Windows)||defined(BSA_WINAPI)
	HWND			f_hwnd;
#endif
}
BwwiItem, *LPWWITEM;

// list template for body parts (raw bytes or ansi/utf-8 text)
//
KEY_VAL_LIST_TEMPLATE(BwizList, LPWWITEM, BWIZ_API);


enum wizMethod { wizGetValue, wizSetValue, wizStepBack, wizStepForward };

typedef bool (*BWIZCALLBACK)(LPVOID cookie, int* step, wizMethod method, LPCTSTR var, LPTSTR val);

//***********************************************************************
class BSAWIZ_API Bwizard
{
public:
	Bwizard(); 
	~Bwizard();

public:
	ERRCODE		Begin		(LPCTSTR title, LPCTSTR script, BWIZCALLBACK callback, LPVOID cookie, HINSTANCE hInstance = NULL);
	ERRCODE		StepBack	(void);
	ERRCODE		StepForward	(void);
	ERRCODE		Step		(int from, int to);
	ERRCODE		End			(ERRCODE exitCode);
	int			CurrentStep	(void)	{ return m_step; }
	LPCTSTR		GetStepName	(void)	{ return m_name; }
	LPCTSTR		GetDescName	(void)	{ return m_desc; }
	ERRCODE		GetStatus	(void)	{ return m_status; }
	void		GetControl	(int id);
	void		OnControl	(int id);

	HFONT		GetTitleFont(void)	{ return m_hfBig; }
	HFONT		GetNameFont	(void)	{ return m_hfName; }
	HFONT		GetDescFont	(void)	{ return m_hfDesc; }

	void		SetDefaultIconID	(int id) { m_defaultIconID = id; }
	void		SetDefaultBitmapID	(int id) { m_defaultBitmapID = id; }

	void		OnKey		(WPARAM key);
	
protected:
	int			ScanIntParm(LPCTSTR pp, int def);
	LPCTSTR		ScanStringParm(LPTSTR pr, LPCTSTR pp, LPCTSTR def);
	ERRCODE		ScanToken(void);
	ERRCODE		Panel		(int step, LPCTSTR name);
	ERRCODE		AddItem		(BwizItemType type, LPCTSTR name);
	void		SendPanel	(void);

protected:
	bool		m_done;
	ERRCODE		m_status;
	BWIZCALLBACK m_callback;
	LPVOID		m_cookie;

	LPTSTR		m_script;
	TCHAR		m_title[MAX_PATH];
	LPCTSTR		m_ptr;
	int			m_step;
	int			m_from_step;
	int			m_next_step;
	
	BYTE		m_enabled_mask;
	BYTE		m_default_mask;

	TCHAR		m_name[MAX_PATH];
	TCHAR		m_desc[MAX_PATH];

	TCHAR		m_atype[MAX_PATH];
	TCHAR		m_aname[MAX_PATH];
	int			m_afontnum;
	int			m_aisdefault;
	int			m_apanel;
	TCHAR		m_avar[MAX_PATH];
	int			m_panw;
	int			m_panh;
	int			m_awidth;
	int			m_aheight;
	int			m_asameline;
	int			m_anumeric;
	int			m_apasswd;
	int			m_ctrl_id;

	BwizList*	m_items;
	BwizList*	m_curedit;
	
	int			m_fh[4];

	int			m_ix;
	int			m_iy;
	int			m_iw;
	int			m_ih;
	int			m_ii;

	int			m_defaultIconID;
	int			m_defaultBitmapID;

	HINSTANCE	m_hInstance;

	HWND		m_hwndPanel;

	HWND		m_hwndBack;
	HWND		m_hwndNext;
	HWND		m_hwndCancel;

	HFONT		m_hfBig;
	HFONT		m_hfName;
	HFONT		m_hfDesc;
};

#endif
