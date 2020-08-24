
#ifndef _INFOPANE_H_
#define _INFOPANE_H_ 1

// wrapper class for one tab pane inside a tab view class
//**************************************************************************
class BtabPane
{
public:
					BtabPane		(LPCTSTR pName);
	virtual			~BtabPane		();

public:
	virtual void	Activate		(void);	
	virtual void	Deactivate		(void);
	virtual bool	GetActive		(void)			{ return m_active;   }
	virtual void	SetViewport		(HWND hwndParent, LPRECT prcView);
	virtual void	ContextMenu		(int x, int y);
	virtual bool	HasConextMenu	(void)			{ return m_hasctxmenu; }
	virtual LPRECT	GetViewRect		(void)			{ return &m_viewrc;  }

protected:
	static LRESULT CALLBACK	TabPaneProc	(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	LPTSTR			m_name;
	bool			m_active;
	bool			m_hasctxmenu;
	RECT			m_viewrc;
	DWORD			m_style;

	class BtabPane*	m_next;
	class BtabPane*	m_prev;
};


KEY_VAL_LIST_TEMPLATE(BtabList, BtabPane*, NO_API);

// wrapper class implementing a self-docking tabbed control
//**************************************************************************
class BtabView
{
public:
						BtabView		();
						~BtabView		();

public:
	virtual void		AddTab			(BtabPane* pPane, LPRECT prcTab = NULL);
	virtual void		AddTab			(LPCTSTR pName, LPRECT prcTab = NULL);
	virtual void		RemoveTab		(int ntab);
	virtual void		RemoveTabByName	(LPCTSTR pName);
	virtual void		SelectTab		(int ntab);
	virtual void		SelectTabByName	(LPCTSTR pName);
	virtual BtabPane*	GetTabByName	(LPCTSTR pName);
	virtual BtabPane*	GetSelectedTab	(void);
	virtual int			GetTabNumber	(BtabPane* pPane);
	virtual LPCTSTR		GetTabName		(BtabPane* pPane);

	virtual void		SetHorizSizerPos(int z);
	virtual int			GetHorizSizerPos(void) { return m_hsizerPos; }
	virtual void		SetVertSizerPos	(int z);
	virtual int			GetVertSizerPos	(void) { return m_vsizerPos; }

	virtual void		SetViewport		(HWND hwndParent, LPRECT prcView);
	virtual void		GetViewportSize	(int& x, int& y, int& w, int& h, LPRECT rc);
	virtual HWND		GetWindow		(void) { return m_hWnd; }

	virtual BtabList*	GetTabs			(void) { return m_tabs; }
	virtual bool		HasTabs			(void) { return m_tabs && m_tabs->GetNext(m_tabs); }

	virtual void		OnMouse			(int x, int y);
	virtual void		Notify			(LPNMHDR pHdr);

protected:
	virtual void		DrawTabs		(HDC hdc, LPRECT rc);

	static	void		InitTabView			(void);
	static LRESULT CALLBACK	TabViewProc		(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK	HorizSizerProc	(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK	VertSizerProc	(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
	BtabView*			m_prev;
	BtabView*			m_next;

protected:
	int					m_tab;
	int*				m_leftStop;
	int*				m_rightStop;
	bool				m_hsizer;
	int					m_hsizerPos;
	bool				m_vsizer;
	int					m_vsizerPos;
	HWND				m_hWnd;
	HWND				m_hWndhSizer;
	HWND				m_vWndhSizer;
	BtabList*			m_tabs;

	static bool			m_init;
};


#endif
