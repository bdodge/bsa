//--------------------------------------------------------------------
//
// File: bsaframe.h
// Desc: application framework 
// Auth: Brian Dodge
//
// (C)opyright 2003 - 2005 - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------

#ifndef _BSAFRAME_H_
#define _BSAFRAME_H_ 1

#ifdef FRAME_EXPORTS
#define FRAME_API __declspec(dllexport)
#elif defined(BSAFRAME_DLL)
#define FRAME_API __declspec(dllimport)
#else
#define FRAME_API
#endif

// number of dimensions for docking
#define BAF_DIMENSIONS	4
typedef enum { frLeft, frRight, frTop, frBottom, frFloat } BAF_EDGE;

class BappComponent;
typedef BappComponent BAFCOMP, *PBAFCOMP;

class BappPane;
typedef class BappPane BAFPANE, *PBAFPANE;

class BappPanel;
typedef class BappPanel BAFPANEL, *PBAFPANEL;

class BappFrame;
typedef class BappFrame BAFFRAME, *PBAFFRAME;

#define VSIZER_WIDTH	5
#define VSIZER_MARG		1

#define HSIZER_HEIGHT	5
#define HSIZER_MARG		1


//--------------------------------------------------------------------
// common window proc can be used for all app frame derived classes
//
LRESULT CALLBACK BAFframeProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

//--------------------------------------------------------------------
// common code for all app frame classes
//
class BappComponent
{
public:
	BappComponent(PBAFPANEL pParent);
	virtual ~BappComponent();
	
public:
	virtual LRESULT		OnMessage		(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
												{ return DefWindowProc(hWnd, msg, wp, lp); }
	virtual ERRCODE		OnResized		(PBAFCOMP pby);
	virtual HWND		GetWindow		(void)	{ return m_hwnd; }

	virtual ERRCODE		GetWindowPos	(HWND hWnd, LPRECT prc);
	virtual ERRCODE		OutlineBox		(HDC hdc, LPRECT rc, bool in);
	virtual HINSTANCE	GetInstance		(void)	{ return m_hInstance; };
	virtual PBAFPANEL	GetParentPanel	(void)	{ return m_parent; };
	virtual ERRCODE		SetParentPanel	(PBAFPANEL p);
	
protected:
protected:
	HWND				m_hwnd;	
	PBAFPANEL			m_parent;
	HINSTANCE			m_hInstance;

	static HFONT		m_hFont;
	static HFONT		m_hBoldFont;
};


//--------------------------------------------------------------------
// base class for a resizer bar
//
class Bsizer : public BappComponent
{
public:
	Bsizer(BAF_EDGE edge);
	virtual ~Bsizer();
	
public:
	virtual LRESULT		OnMessage		(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
	virtual ERRCODE		Attach			(PBAFPANEL pp);
	virtual ERRCODE		Dettach			(void);
	virtual ERRCODE		GetFit			(int& x, int&y, int&w, int&h) = 0;

protected:

protected:
	int					m_ix, m_iy;
	BAF_EDGE			m_edge;
};

//--------------------------------------------------------------------
// base class for a vertical resizer bar
//
class BvertSizer : public Bsizer
{
public:
	BvertSizer(BAF_EDGE edge);
	virtual ~BvertSizer();
	
public:
	virtual LRESULT		OnMessage		(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
	virtual ERRCODE		GetFit			(int& x, int&y, int&w, int&h);

protected:
protected:
};

//--------------------------------------------------------------------
// base class for a horizontal resizer bar
//
class BhorzSizer : public Bsizer
{
public:
	BhorzSizer(BAF_EDGE edge);
	virtual ~BhorzSizer();
	
public:
	virtual LRESULT		OnMessage		(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
	virtual ERRCODE		GetFit			(int& x, int&y, int&w, int&h);

protected:
protected:
};

#define TABPANE_LEFTMARG	3
#define TABPANE_RIGHTMARG	3
#define TABPANE_TOPMARG		3
#define TABPANE_BOTTOMMARG	3
#define TABPANE_BORDER		2
#define TABPANE_TABHEIGHT	20


#define TABTEXT_LEFTMARG	6
#define TABTEXT_TOPMARG		1

typedef enum { frNoBorder, frBorder, frTopTabs, frBottomTabs } BAF_BORDER;

//--------------------------------------------------------------------
// container window for one pane
//
class BappPane : public BappComponent
{
public:
	BappPane(LPCTSTR pName, PBAFPANEL pPanel);
	virtual ~BappPane();
	
public:
	virtual LRESULT		OnMessage		(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
	virtual LPCTSTR		GetName			(void)	{ return m_name; }
	virtual void		SetName			(LPCTSTR pName);
	virtual ERRCODE		SetNext			(PBAFPANE pc)
												{ m_next = pc; return errOK; }
	virtual ERRCODE		SetPrev			(PBAFPANE pc)
												{ m_prev = pc; return errOK; }
	virtual PBAFPANE	GetPrev			(void)	{ return m_prev; }
	virtual PBAFPANE	GetNext			(void)	{ return m_next; }

	virtual bool		GetActive		(void)	{ return m_active; }
	virtual	void		Activate		(void);
	virtual void		Deactivate		(void);
	virtual void		FitToPanel		(void);

protected:

public:
	RECT				m_tabrc;

protected:
	LPTSTR				m_name;
	PBAFPANE			m_prev;
	PBAFPANE			m_next;
	bool				m_active;
};

//--------------------------------------------------------------------
// recursively resizable docking panels
//
class BappPanel : public BappComponent
{
public:
	BappPanel(LPCTSTR pName, PBAFPANEL pParent, BAF_BORDER border = frNoBorder);
	virtual ~BappPanel();
	
public:
	virtual LRESULT		OnMessage		(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
	virtual ERRCODE		OnDock			(BAF_EDGE edge, int x, int y, int w, int h);

	virtual LPCTSTR		GetName			(void)	{ return m_name; }
	virtual void		SetName			(LPCTSTR pName);
	virtual void		SetPaneTabs		(void);

	virtual ERRCODE		Dock			(
											PBAFPANEL	pPanel,
											BAF_EDGE	edge,
											LPRECT		pprc = NULL,
											bool		extend = false,
											bool		primary = false
										);
	virtual ERRCODE		Undock			(PBAFPANEL pPanel);
	virtual ERRCODE		RemovePanel		(PBAFPANEL pPanel);
	virtual PBAFPANEL	GetPanels		(BAF_EDGE edge);
	virtual ERRCODE		GetPanelsRect	(PBAFPANEL pp, LPRECT prc);
	virtual ERRCODE		FitPanel		(PBAFPANEL pm, LPRECT prc, BAF_EDGE edge);
	virtual ERRCODE		OnResized		(PBAFCOMP pby);

	virtual BAF_EDGE	GetEdge			(void)	{ return m_edge; }
	virtual BAF_BORDER	GetBorder		(void)	{ return m_border; }
	virtual ERRCODE		SetBorder		(BAF_BORDER b)
												{ m_border = b; return errOK; }
	virtual bool		HasMultiplePanes(void)	{ return m_panes ? (m_panes->GetNext() != NULL) : false; }
	virtual bool		GetPrimary		(void)	{ return m_primary; }
	virtual ERRCODE		SetPrimary		(bool p);
	virtual bool		GetSizable		(void)	{ return m_sizable; }
	virtual ERRCODE		SetSizable		(bool z);
	virtual ERRCODE		SetNext			(PBAFPANEL pc)
												{ m_next = pc; return errOK; }
	virtual ERRCODE		SetPrev			(PBAFPANEL pc)
												{ m_prev = pc; return errOK; }
	virtual PBAFPANEL	GetPrev			(void)	{ return m_prev; }
	virtual PBAFPANEL	GetNext			(void)	{ return m_next; }
	virtual PBAFPANEL	FindPanel		(LPCTSTR pName);
	virtual PBAFPANE	FindPane		(LPCTSTR pName);

	virtual ERRCODE		GetPanelClientRect(LPRECT prc);

	virtual ERRCODE		AddPane			(PBAFPANE pp);
	virtual ERRCODE		RemovePane		(PBAFPANE pp);
	virtual PBAFPANE	GetPanes		(void)	{ return m_panes; }
	virtual ERRCODE		ActivatePane	(int pane_num);
	virtual ERRCODE		ActivatePane	(PBAFPANE pp);
	virtual ERRCODE		ActivatePane	(LPCTSTR  pname);

protected:
	virtual ERRCODE		SetPrimaryPanel	(PBAFPANEL pp);

protected:
	PBAFPANE			m_panes;
	Bsizer*				m_sizer;
	LPTSTR				m_name;
	BAF_EDGE			m_edge;
	BAF_BORDER			m_border;
	bool				m_primary;
	bool				m_sizable;
	PBAFPANEL			m_prev;
	PBAFPANEL			m_next;

	PBAFPANEL			m_panels		[BAF_DIMENSIONS];
	bool				m_fulledged		[BAF_DIMENSIONS];
};

//--------------------------------------------------------------------
// container window for application
//
class BappFrame : public BappPanel
{
public:
	BappFrame(HINSTANCE hInst);
	virtual ~BappFrame();
	
public:
	virtual ERRCODE		Init			(
											HINSTANCE hInstance,
											LPCTSTR id_menu,
											LPCTSTR id_icon1,
											LPCTSTR id_icon2,
											int bkg = COLOR_3DLIGHT
										);
	virtual ERRCODE		Open			(LPCTSTR title, int x, int y, int w, int h);
//	virtual LRESULT		OnMessage		(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
	
protected:
	
protected:
private:
	HICON				m_hIcon;
	HMENU				m_hMenu;
	bool				m_init;
	static DWORD		m_initcount;
};

	
#endif // _BSAFRAME_H_

