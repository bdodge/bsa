
#ifndef INFODBG_H_
#define INFODBG_H_

// base class for debugger subpane
//**************************************************************************
class BdbgPaneAbstract
{
public:
					BdbgPaneAbstract(LPCTSTR name, BappPanel* pPanel, Bed* pParent) : m_exec(rcStop) {};
	virtual			~BdbgPaneAbstract() {};

public:
	virtual	void	OnExec(BdbgRunCode rc) { m_exec = rc; }

protected:
	BdbgRunCode		m_exec;
};

// base class for debugger subpane based on infopane (treeview)
//**************************************************************************
class BdbgPane : public BinfoPane, public BdbgPaneAbstract
{
public:
					BdbgPane		(LPCTSTR name, BappPanel* pPanel, Bed* pParent)
									: BinfoPane(name, pPanel, pParent),
									  BdbgPaneAbstract(name, pPanel, pParent) {}; 
	virtual			~BdbgPane		() {};

public:
	virtual void	LoadImages		(void) { BinfoPane::LoadImages(); }
	virtual	void	SetView			(Bview* pView) { BinfoPane::SetView(pView); }
	virtual void	Select			(LPTVITEM pItem) { BinfoPane::Select(pItem); };
};


// watching vars view object
//**************************************************************************
class BwatchInfo : public BdbgPane
{
public:
					BwatchInfo		(LPCTSTR name, BappPanel* pPanel, Bed* pParent)
									: BdbgPane(name, pPanel, pParent) {};
	virtual			~BwatchInfo		() {};

public:

public:

protected:

protected:
};


// registers view object
//**************************************************************************
class BregisterInfo : public BdbgPane
{
public:
					BregisterInfo	(LPCTSTR name, BappPanel* pPanel, Bed* pParent)
									: BdbgPane(name, pPanel, pParent) {};
	virtual			~BregisterInfo	() {};

public:

public:

protected:

protected:
};

// stack frame view object
//**************************************************************************
class BstackInfo : public BdbgPane
{
public:
					BstackInfo		(LPCTSTR name, BappPanel* pPanel, Bed* pParent)
									: BdbgPane(name, pPanel, pParent), m_nStackDex(0) {};
	virtual			~BstackInfo		() {};

public:
	virtual	void	FitToPanel		();
	virtual void	Select			(LPTVITEM pItem) { BinfoPane::Select(pItem); };

	virtual	void	OnExec(BdbgRunCode rc);

public:
	virtual void	BeginStackFrame	(void);
	virtual void	AddStackLine	(LPCTSTR pLine);

public:

protected:

protected:
	int				m_nStackDex;
};

// threads view object
//**************************************************************************
class BthreadInfo : public BdbgPane
{
public:
					BthreadInfo		(LPCTSTR name, BappPanel* pPanel, Bed* pParent)
									: BdbgPane(name, pPanel, pParent) {};
	virtual			~BthreadInfo	() {};

public:

public:

protected:

protected:
};

// locals view object
//**************************************************************************
class BvarInfo : public BdbgPane
{
public:
					BvarInfo		(LPCTSTR name, BappPanel* pPanel, Bed* pParent)
									: BdbgPane(name, pPanel, pParent) {};
	virtual			~BvarInfo		() {};

public:

public:

protected:

protected:
};

// memory view object
//**************************************************************************
class BmemInfo : public BdbgPane
{
public:
					BmemInfo		(LPCTSTR name, BappPanel* pPanel, Bed* pParent) 
									: BdbgPane(name, pPanel, pParent) {};
	virtual			~BmemInfo		() {};

public:

public:

protected:

protected:
};



#endif
