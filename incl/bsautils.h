//--------------------------------------------------------------------
//
// File: bsautils.h
// Desc: 
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------
#ifndef _BSAUTILS_H_
#define _BSAUTILS_H_ 1

#ifdef UTILS_EXPORTS
	#define UTILS_API __declspec(dllexport)
#elif defined(BSAUTIL_DLL)
	#define UTILS_API __declspec(dllimport)
#else
	#define UTILS_API
#endif

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef max
#define max(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef GWL_USERDATA
#define GWL_USERDATA GWLP_USERDATA
#endif


//-----------------------------------------------------------------------
// utility classes to wrap OS native mutex/event/threads

//**************************************************************************
class UTILS_API Bmutex
{
public:
	Bmutex();
    ~Bmutex();
    
    void				Lock		();
    void				Unlock		();

#if defined(Windows)
private:
    CRITICAL_SECTION	cs;
#elif defined(PTHREADS)
public:
    operator			pthread_mutex_t*() { return &cs; }    
private:
    pthread_mutex_t		cs;
#endif
};

//**************************************************************************
class UTILS_API Block
{
public:
	Block(Bmutex* pmutex) : m_mutex(pmutex)
									{ m_mutex->Lock(); }
	~Block()						{ m_mutex->Unlock(); }

private:
	Bmutex* m_mutex;
};

//**************************************************************************
class UTILS_API Bevent
{
public:
#if defined(Windows)
    Bevent(LPTSTR pName = NULL)		{ m_hEvent = CreateEvent(NULL, false, false, pName); }
    ~Bevent()						{ CloseHandle(m_hEvent);							 }
    
    ERRCODE				Wait		(DWORD dwTimeout)
						{
							if(WaitForSingleObject(m_hEvent, dwTimeout) == WAIT_TIMEOUT)
								return errTIMEOUT;
							else
							{
								ResetEvent(m_hEvent);
								return errOK;
							}
						}
    void				Signal		()
						{
							SetEvent(m_hEvent);
						}
    
private:
    HANDLE				m_hEvent;
#elif defined(PTHREADS)
    Bevent(LPTSTR pName = NULL)		{ pthread_cond_init(&m_cond, NULL);	}
    ~Bevent()						{ pthread_cond_destroy(&m_cond);	}
    
    ERRCODE				Wait		(DWORD dwTimeout)
    					{
    						Block lock(&m_mutex);
    						struct timespec timer;
  						
							pthread_cond_init(&m_cond, NULL);
   
							if(dwTimeout != (DWORD)-1L)
							{
#if 0
    							struct timeb    mtime;
								
								ftime(&mtime);
  	    						mtime.millitm += dwTimeout;
	    						while(mtime.millitm > 1000)
	    						{
	    							mtime.millitm -= 1000;
	    							mtime.time += 1;
	    						}
	    						
	    						timer.tv_sec = mtime.time;
	    						timer.tv_nsec = mtime.millitm * 1000000;
#else
								struct timespec walltime;
								
								clock_gettime(CLOCK_REALTIME, &walltime);
	    						timer.tv_sec = walltime.tv_sec;
	    						timer.tv_nsec = walltime.tv_nsec;
#endif
	    						if(pthread_cond_timedwait(&m_cond, m_mutex, &timer) == ETIMEDOUT)
	    							return errTIMEOUT;
	    						else return errOK;
							}
							else
							{
								pthread_cond_wait(&m_cond, m_mutex);
								return errOK;
							}
    					}
    void				Signal		()	
						{ 
				   			Block lock(&m_mutex);
				   			pthread_cond_signal(&m_cond);
						}
    
private:
	pthread_cond_t		m_cond;
    Bmutex				m_mutex;
#else
	#pragma Warning("No Bevent");
#endif
};

typedef ERRCODE (WINAPI *LPTHREAD_FUNC)(void*);

//**************************************************************************
class UTILS_API Bthread
{
public:
#if defined(Windows)
    Bthread(LPTSTR pName = NULL) : m_hThread(NULL), m_threadID(0), m_running(false)
									{ }
    virtual ~Bthread()				{ if(m_running) Stop(); if(m_hThread) CloseHandle(m_hThread); }
    
	virtual ERRCODE		Start		(LPTHREAD_FUNC pThreadFunc, void* thisparm, bool createThread = true);
	virtual ERRCODE		Stop		(void);
	virtual bool		GetRunning	(void)		{ return m_running; }
	static  ERRCODE		Sleep		(int ms)	{ ::Sleep(ms); return errOK; }
    
protected:
    HANDLE				m_hThread;
	DWORD				m_threadID;
	bool				m_running;
#elif defined(PTHREADS)
    Bthread(LPTSTR pName = NULL) : m_hThread(0), m_running(false)
									{ }
	virtual ~Bthread() 				{ if(m_running) Stop(); }
    
	virtual ERRCODE		Start		(LPTHREAD_FUNC pThreadFunc, void* thisparm, bool createThread = true);
	virtual ERRCODE		Stop		(void);
	virtual bool		GetRunning	(void)		{ return m_running; }
	static ERRCODE		Sleep		(int ms) { usleep(1000 * ms); return errOK; }
    
protected:
    pthread_t			m_hThread;
	bool				m_running;
#else
	#pragma Warning("No Bthread")
#endif
};


//-----------------------------------------------------------------------
// base class for logging, all other major classes also derive from BLog
// so this is NOT exported from bsautils.dll, and is expanded inline here
// so it can be instantiated in each dll

//***********************************************************************
enum logType
{
	logInfo,
	logDebug,
	logWarning,
	logError
};

enum logDest
{
	logtoCONSOLE,
	logtoFILE,
	logtoFILEAPPEND,
	logtoWINDOW,
	logtoNOWHERE
};

//***********************************************************************
class UTILS_API Blog
{
public:
	Blog(int level, logDest dest = logtoCONSOLE, LPCTSTR logfile = NULL);
	virtual ~Blog();

public:
	virtual int		GetLevel	(void)	{ return m_level; }
	virtual void	SetLevel	(int l) { m_level = l; }
	virtual int		GetDest		(void)	{ return m_logdest; }
	virtual void	SetDest		(logDest d) { m_logdest = d; }
	virtual LPCTSTR GetLogfile	(void)	{ return m_logfile; }
	virtual void	SetLogfile	(LPCTSTR logfile);
	virtual void	Log			(logType type, int level, LPCTSTR format, ...);

protected:
	int				m_level;
	logDest			m_logdest;
	TCHAR			m_emsg[256];
	LPCTSTR			m_logfile;
	FILE*			m_plf;
};

//***********************************************************************
// linked list of generic name->value(value) 
//
class UTILS_API BlistElement
{
public:
	BlistElement(LPCTSTR pName, BlistElement* pNext = NULL);
	virtual ~BlistElement();

public:
	LPTSTR					GetName			(void) { return m_pName; }
	static	BlistElement*	GetNext			(BlistElement* pList);
	static	BlistElement*	SetNext			(BlistElement* pList);
	static	BlistElement*	AddToList		(BlistElement* pItem, BlistElement* pList);
	static	BlistElement*	RemoveFromList	(BlistElement* pItem, BlistElement* pList);
	static	BlistElement*	FreeList		(BlistElement* pList);													\
	static  BlistElement*	FindKey			(LPCTSTR pName, const BlistElement* pList);

public:
	LPTSTR					m_pName;

public:
	BlistElement*			m_pNext;
};

// key-value dictionary macro from values which are arrays
//
#define KEY_ARRYVAL_LIST_TEMPLATE(_type_, _valtype_, _apitype_)						\
																					\
class _type_ : public BlistElement													\
{																					\
public:																				\
	_type_(LPCTSTR pName, _valtype_ pVal, _type_* pNext = NULL) :					\
				BlistElement(pName, pNext), m_pValue(pVal) {}						\
	virtual ~_type_() { delete [] (_valtype_)m_pValue; }							\
																					\
public:																				\
	static _type_*		GetNext			(_type_* pItem)	{ return pItem ? (_type_*)pItem->m_pNext : NULL; }	\
	static _type_*		SetNext			(_type_* pItem, _type_* pNext)										\
														{ pItem->m_pNext = pNext; return pNext; }			\
	static _type_*		FindKey			(LPCTSTR pName, const _type_* pList)								\
		{ return (_type_*)BlistElement::FindKey(pName, (const BlistElement*)pList); }						\
	static _type_*		FreeList		(_type_* pList)														\
		{ return (_type_*)BlistElement::FreeList((BlistElement*)pList); }									\
	static _type_*		AddToList		(_type_* pItem, _type_* pList)										\
		{ return (_type_*)BlistElement::AddToList((BlistElement*)pItem, (BlistElement*)pList); }			\
	static _type_*		RemoveFromList	(_type_* pItem, _type_* pList)										\
		{ return (_type_*)BlistElement::RemoveFromList((BlistElement*)pItem, (BlistElement*)pList); }		\
																											\
public:																				\
	_valtype_			GetValue		(void)										\
		{ return (_valtype_)m_pValue; }												\
	void				SetValue		(_valtype_ newvalue)						\
		{ m_pValue = newvalue; }													\
	operator _valtype_()															\
		{ return (_valtype_)m_pValue;}												\
																					\
public	:																			\
	_valtype_			m_pValue;													\
}

// key-value dictonary template macro for values which are objects
//
#define KEY_VAL_LIST_TEMPLATE(_type_, _valtype_, _apitype_)							\
																					\
class _type_ : public BlistElement													\
{																					\
public:																				\
	_type_(LPCTSTR pName, _valtype_ pVal, _type_* pNext = NULL) :					\
				BlistElement(pName, pNext), m_pValue(pVal) {}						\
	virtual ~_type_() { delete (_valtype_)m_pValue; }								\
																					\
public:																				\
	static _type_*		GetNext			(_type_* pItem)	{ return pItem ? (_type_*)pItem->m_pNext : NULL; }	\
	static _type_*		SetNext			(_type_* pItem, _type_* pNext)										\
														{ pItem->m_pNext = pNext; return pNext; }			\
	static _type_*		FindKey			(LPCTSTR pName, const _type_* pList)								\
		{ return (_type_*)BlistElement::FindKey(pName, (const BlistElement*)pList); }						\
	static _type_*		FreeList		(_type_* pList)														\
		{ return (_type_*)BlistElement::FreeList((BlistElement*)pList); }									\
	static _type_*		AddToList		(_type_* pItem, _type_* pList)										\
		{ return (_type_*)BlistElement::AddToList((BlistElement*)pItem, (BlistElement*)pList); }			\
	static _type_*		RemoveFromList	(_type_* pItem, _type_* pList)										\
		{ return (_type_*)BlistElement::RemoveFromList((BlistElement*)pItem, (BlistElement*)pList); }		\
																											\
public:																				\
	_valtype_			GetValue		(void)										\
		{ return (_valtype_)m_pValue; }												\
	void				SetValue		(_valtype_ newvalue)						\
		{ m_pValue = newvalue; }													\
	operator _valtype_()															\
		{ return (_valtype_)m_pValue;}												\
																					\
public	:																			\
	_valtype_			m_pValue;													\
}

KEY_VAL_LIST_TEMPLATE(BkeyVal, LPTSTR, UTILS_API);


//**************************************************************************
class UTILS_API Bmsg 
{
public:
	Bmsg(int initSize = 1024);
	Bmsg(LPSTR pMsg, int size);
	virtual ~Bmsg();

public:
	virtual LPBYTE			GetMsg				(void)	{ return m_pBuf;	}
	virtual int				GetMsgLen			(void)	{ return m_ioCnt;	}
	virtual ERRCODE			SkipBYTES			(int nBytes);
	virtual inline ERRCODE	AddBYTE				(BYTE);
	virtual inline ERRCODE	AddWORD				(WORD);
	virtual inline ERRCODE	AddDWORD			(DWORD);
	virtual inline ERRCODE	AddSwappedWORD		(WORD);
	virtual inline ERRCODE	AddSwappedDWORD		(DWORD);
	virtual inline ERRCODE	AddSTRING			(LPCSTR);
#ifdef UNICODE
	virtual inline ERRCODE	AddSTRING			(LPCTSTR);
#endif
	virtual inline ERRCODE	GetBYTE				(BYTE&);
	virtual inline ERRCODE	GetWORD				(WORD&);
	virtual inline ERRCODE	GetDWORD			(DWORD&);
	virtual inline ERRCODE	GetSwappedWORD		(WORD&);
	virtual inline ERRCODE	GetSwappedDWORD		(DWORD&);

protected:
	LPBYTE					m_pBuf;
	int						m_bufAlloc;
	int						m_ioCnt;
	int						m_msgSize;
};
 

//-----------------------------------------------------------------------
// class to just hold all the global helper functions for 
#if !defined(Windows)&&defined(UNICODE)
#define MAX_BSA_LFCVT 2048
#define MAX_BSA_LFX	  4 /* reentrancy max */
#endif

extern int wcscasecmp(LPCWSTR pA, LPCWSTR pB);

//***********************************************************************
class UTILS_API BUtil
{
public:
	// OS utils
	static ERRCODE	LoadLibrary			(HDYNLIB& hLib, LPCTSTR pLib);
	static ERRCODE	ResolveFunction		(HDYNLIB hLib, LPCTSTR pName, PDYNFUNCTION& pFunc);
	static ERRCODE	CloseLibrary		(HDYNLIB hLib);
	static ERRCODE	GetHomePath			(LPTSTR pbuf, int nBuf);
	static ERRCODE	GetTempPath			(LPTSTR pbuf, int nBuf);
	static ERRCODE	FileExists			(LPCTSTR pName);
	static ERRCODE	DirectoryExists		(LPCTSTR pName);

	// UNICODE utils
	static WCHAR*	CharToWChar			(LPWSTR pRes, LPCSTR pSrc);
	static const WCHAR*	CharToTempWChar		(LPCSTR pSrc);
	static char*	WCharToChar			(char* pRes, LPCWSTR pSrc);
	static bool		Utf8Encode			(char* dst, LPCWSTR pSrc, bool bQuoteIt);
	static bool		Utf8Decode			(WCHAR* dst, LPCSTR pSrc);
#if !defined(Windows)&&defined(UNICODE)
	static WCHAR*	LinuxFormatConvert	(LPCSTR frmt);
#endif
	// URL utils
	static ERRCODE	UrlParse			(LPCSTR pUrl, LPSTR proto, LPSTR host, short* port, LPSTR path, LPSTR parms);
	static ERRCODE	UrlExtractParms		(LPCSTR parms, BkeyVal*& pList);

	// Misc utils
	static LPCTSTR	ExplainError		(ERRCODE ec);
	static int		EscapeString		(LPSTR pDst, LPCSTR pSrc);
	static int		EscapeDWORD			(LPSTR pDst, DWORD dwSrc);
	static int		UnescapeString		(LPSTR pDst, LPCSTR pSrc);
#ifdef UNICODE
	static int 		UnescapeString		(LPTSTR pDst, LPCTSTR pSrc);
#endif
	static int		DecodeBase64		(BYTE* pDst, LPCSTR pSrc);
	static int		EncodeBase64		(LPSTR pDst, LPBYTE pSrc, int cbSrc, bool hexescape);
	static BYTE		HexNibble			(TCHAR src);
	static LPCTSTR	SimplePatternMatch	(LPCTSTR string, LPCTSTR pattern);
public:
protected:
};

//-----------------------------------------------------------------------
//
// unicode/string defs 
//
#ifdef UNICODE
	#define TCharToChar(a, b) 	BUtil::WCharToChar(a, b)
	#define CharToTChar(a, b) 	BUtil::CharToWChar(a, b)
	#define UNICVT(a)		  	BUtil::CharToTempWChar(a)
	#define UTF8DECODE(a, b) 	BUtil::Utf8Decode(a, b)
	#define UTF8ENCODE(a, b) 	BUtil::Utf8Encode(a, b, false)
	#define _Pfs_				"%ls"
#else
	#define TCharToChar(a, b) 	strcpy(a, b)
	#define CharToTChar(a, b) 	strcpy(a, b)
	#define UNICVT(a)		  	a
	#define UTF8DECODE(a, b) 	strcpy(a, b)
	#define UTF8ENCODE(a, b) 	strcpy(a, b)
	#define _Pfs_				"%s"
#endif

#endif // _UTILS_H_

