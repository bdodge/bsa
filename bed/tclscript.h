
#ifndef _TCLSCRIPT_H_
#define _TCLSCRIPT_H_ 1

extern "C" {

typedef struct Tcl_Interp
{
	char* result;
	void  (*freefunc)(char*);
	int   errorLine;
}
Tcl_Interp;

struct Tcl_Command_;
typedef struct Tcl_Command_ *Tcl_Command;
struct Tcl_Encoding_;
typedef struct Tcl_Encoding_ *Tcl_Encoding;
struct Tcl_EncodingState_;
typedef struct Tcl_EncodingState_ *Tcl_EncodingState;
typedef void *ClientData;

typedef void (Tcl_CmdDeleteProc)(ClientData clientData);
typedef int (Tcl_CmdProc)(ClientData clientData, Tcl_Interp *interp, int argc, const char *argv[]);


struct Tcl_Interp;
}

#define MAX_TK_STACK 64

KEY_VAL_LIST_TEMPLATE(BtclCommandList, LPTSTR, NO_API);

// tcl stript execution wrapper
//
//***********************************************************************
class Btcl : public Bthread
{
public:
	Btcl							(Bview* pView);
	virtual ~Btcl					();
	
public:

public:
	ERRCODE			Interpret		(const TCHAR* pName, bool bFromFile, bool bIsTK);
	bool			IsTCLcommand	(LPCTSTR lpCommandname, LPCTSTR& lpCommandPath);
	bool			IsTCLfile		(LPCTSTR lpFilename);
	LPCTSTR			FindTCLfile		(LPCTSTR nameIn, LPTSTR nameOut, int& useTk);
	static
	 ERRCODE WINAPI	RunStub			(LPVOID thisptr);

protected:
	LPCTSTR			Utf8Decode		(LPCSTR src);
	LPCSTR			Utf8Encode		(LPCTSTR src);
	void			BuildTCLfileList(void);
	void			EvalError		(LPCTSTR pMsg1, const char* pMsg2);
	void			EvalMessage		(LPCTSTR pMsg1, LPCTSTR pMsg2);
	static int		TclBkgError		(ClientData pThis, Tcl_Interp *pTerp, int argc, const char* argv[]);
	static int		TclWrapper		(ClientData pThis, Tcl_Interp *pTerp, int argc, const char* argv[]);
	int				TclExec			(Tcl_Interp *pTerp, int argc, const char* argv[]);

protected:
	bool			m_evalerror;
	int				m_tkStack;

	Tcl_Interp*		m_pTkInterps[MAX_TK_STACK];

	Bview*			m_pView;
	Blog*			m_log;

	int				m_utf8_alloc;
	LPTSTR			m_utf8_buf;

	static int		m_refs;
	static BtclCommandList* m_tclCommandFiles;
	static bool		m_tcl_is_loaded;
	static bool		m_tk_is_loaded;
};

#endif /* _TCLSCRIPT_H_ */
