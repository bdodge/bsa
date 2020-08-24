#ifndef _BSCCS_H_
#define _BSCCS_H_ 1

#define MAX_SCCS_CMD 128


// base class for source control integration
//**************************************************************************
class Bsccs : public Bshell, public Bthread
{
public:
	Bsccs(Bed* pParent);
	~Bsccs();

public:
	virtual ERRCODE	CheckOut			(LPCTSTR filename, HWND hWnd);
	virtual ERRCODE	CheckIn				(LPCTSTR filename, HWND hWnd);
	virtual ERRCODE	Revert				(LPCTSTR filename, HWND hWnd);

	virtual ERRCODE	SCCSWizard			(void);
	virtual ERRCODE	EditSettings		(HWND);

public:
	virtual ERRCODE	GetShell			(LPTSTR pStr, int nStr);
	virtual ERRCODE	GetShellSwitches	(LPTSTR pStr, int nStr);
	virtual ERRCODE	GetCheckoutCommand	(LPTSTR pStr, int nStr);
	virtual ERRCODE	GetCheckinCommand	(LPTSTR pStr, int nStr);
	virtual ERRCODE	GetRevertCommand	(LPTSTR pStr, int nStr);

	virtual ERRCODE	SetShell			(LPCTSTR pStr);
	virtual ERRCODE	SetShellSwitches	(LPCTSTR pStr);
	virtual ERRCODE	SetCheckoutCommand	(LPCTSTR pStr);
	virtual ERRCODE	SetCheckinCommand	(LPCTSTR pStr);
	virtual ERRCODE	SetRevertCommand	(LPCTSTR pStr);

	virtual ERRCODE	GetP4client			(LPTSTR pStr, int nStr);
	virtual ERRCODE	GetP4port			(LPTSTR pStr, int nStr);
	virtual ERRCODE	GetP4user			(LPTSTR pStr, int nStr);
	virtual ERRCODE	GetP4passwd			(LPTSTR pStr, int nStr);
	virtual ERRCODE	GetP4envclient		(bool& envcli);
	virtual ERRCODE	GetP4envport		(bool& envsrv);
	virtual ERRCODE	GetP4envauth		(bool& envauth);
	virtual ERRCODE	GetP4auth			(bool& authreq);

	virtual ERRCODE	SetP4client			(LPCTSTR pStr);
	virtual ERRCODE	SetP4port			(LPCTSTR pStr);
	virtual ERRCODE	SetP4user			(LPCTSTR pStr);
	virtual ERRCODE	SetP4passwd			(LPCTSTR pStr);
	virtual ERRCODE	SetP4envclient		(bool envcli);
	virtual ERRCODE	SetP4envport		(bool envsrv);
	virtual ERRCODE	SetP4envauth		(bool envauth);
	virtual ERRCODE	SetP4auth			(bool authreq);

	virtual bool	OnWiz				(int* step, wizMethod method, LPCTSTR typecode, LPCTSTR var, LPTSTR val);

	static ERRCODE WINAPI
					RunStub			(LPVOID thisptr);
	ERRCODE			ShellThread		(void);

protected:
	virtual ERRCODE	RestoreSettings		(void);
	virtual ERRCODE	SaveSettings		(void);
	virtual bool	IsReadonly			(LPCTSTR lpFilename);
	virtual ERRCODE RunCommand			(LPCTSTR filename, LPCTSTR cmd);
	virtual ERRCODE FormatCommand		(LPTSTR ncmd, LPCTSTR cmd);

	static bool		OnSCCSSetupData		(LPVOID cookie, int* step, wizMethod method, LPCTSTR var, LPTSTR val);
	static char		projectScript[];

public:

protected:
	Bed*			m_editor;
	Bpersist*		m_persist;
	TCHAR			m_sccs[1024];
	char			m_shresp[2048];
	int				nResp;
	TCHAR			m_cocmd[MAX_SCCS_CMD];
	TCHAR			m_cicmd[MAX_SCCS_CMD];
	TCHAR			m_rvcmd[MAX_SCCS_CMD];
	TCHAR			m_sccsshell[MAX_PATH];
	TCHAR			m_sccsswitch[32];
	TCHAR			m_sccssep[32];
	bool			m_p4_envclient;
	bool			m_p4_envport;
	bool			m_p4_envauth;
	bool			m_p4_auth;
	TCHAR			m_p4_client[MAX_PATH];
	TCHAR			m_p4_port[MAX_PATH];
	TCHAR			m_p4_user[MAX_PATH];
	TCHAR			m_p4_passwd[MAX_PATH];

};

#endif // _BSCCS_H_
