//--------------------------------------------------------------------
//
// File: bsapersist.h
// Desc: persistence
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------

#ifndef _BSAPERSIST_H_
#define _BSAPERSIST_H_ 1

#ifdef PERSIST_EXPORTS
#define PERSIST_API __declspec(dllexport)
#elif defined(BSAPERSIST_DLL)
#define PERSIST_API __declspec(dllimport)
#else
#define PERSIST_API
#endif

//-----------------------------------------------------------------------
// persistent (non-volatile) config params

// there are 4 ways this is implemented
//
//  define BS_PERST_INI    - uses .ini file format, using windows API to r/w
//  define BS_PERST_RCFILE - uses ini style file format through an .rc file 
//  define BS_PERST_REG    - uses windows registry API 
//  define BS_PERST_XML    - uses xml files and xml dom model
//
// only one style is compiled at a time
// defaults are "registry" for windows
// and "xml" for non-windows
//
// but XML is always compiled in at least, so it can be used ala card
//
class Bperstreg;
class Bperstxml;
class Bperstrc;
class Bperstini;

//#define BS_PERST_INI 1
#define BS_PERST_XML 1

#ifdef Windows
	#ifndef BS_PERST_INI
		#define BS_PERST_REG 1
		typedef Bperstreg Bpersist;
	#else
		typedef Bperstini Bpersist;
	#endif
#else
	#ifndef BS_PERST_RCFILE
		typedef Bperstxml Bpersist;
	#else
		typedef Bperstrc Bpersist;
	#endif 
#endif

#if defined(BS_PERST_XML) && ! defined(_BSAXML_H_)
	#include "bsaxml.h"
#endif

#define errPERSIST_PARM_NOT_FOUND	errFAILURE
#define errPERSIST_NO_MORE_PARMS	errFAILURE

//***********************************************************************
// persistence object

class PERSIST_API BpersistBase
{
public:
						BpersistBase		(LPCTSTR pName, LPCTSTR pManuf = NULL, bool setifnew = true);
	virtual				~BpersistBase		();

public:				
	virtual ERRCODE		GetNvInt			(LPCTSTR pName, int&    val,  int defval = 0);
	virtual ERRCODE		GetNvBool			(LPCTSTR pName, bool&   val,  bool defval = false);
	virtual ERRCODE		GetNvStr			(LPCTSTR pName, LPTSTR pval, int cbVal, LPCTSTR defval = NULL);

	virtual ERRCODE		SetNvInt			(LPCTSTR pName, int    val);
	virtual ERRCODE		SetNvBool			(LPCTSTR pName, bool   val);
	virtual ERRCODE		SetNvStr			(LPCTSTR pName, LPCTSTR pval);

	virtual ERRCODE		GetNextNvInt		(LPCTSTR pName, int&    val,  int defval = 0);
	virtual ERRCODE		GetNextNvBool		(LPCTSTR pName, bool&   val,  bool defval = false);
	virtual ERRCODE		GetNextNvStr		(LPCTSTR pName, LPTSTR pval, int cbVal, LPCTSTR defval = NULL);

	virtual ERRCODE		SetNextNvInt		(LPCTSTR pName, int    val);
	virtual ERRCODE		SetNextNvBool		(LPCTSTR pName, bool   val);
	virtual ERRCODE		SetNextNvStr		(LPCTSTR pName, LPCTSTR pval);

	virtual ERRCODE		Flush				(LPCTSTR pNewName = NULL);

protected:
	virtual ERRCODE		ParseName			(LPCTSTR pName);

public:

protected:
	int					m_iter;
	int					m_setiter;
	bool				m_dirty;
	bool				m_setifnew;
	TCHAR				m_path[MAX_PATH+5];
	TCHAR				m_section[MAX_PATH];
	TCHAR				m_parm[MAX_PATH];
	Bmutex				m_perstex;
};


#ifdef BS_PERST_REG

//***********************************************************************
// persistence object via windows registry

class PERSIST_API Bperstreg : public BpersistBase
{
public:
						Bperstreg			(LPCTSTR pName, LPCTSTR pManuf = NULL, bool setifnew = true, bool allusers = false);
	virtual				~Bperstreg			();

public:				
	virtual ERRCODE		GetNvInt			(LPCTSTR pName, int&    val,  int defval = 0);
	virtual ERRCODE		GetNvStr			(LPCTSTR pName, LPTSTR pval, int cbVal, LPCTSTR defval = NULL);

	virtual ERRCODE		SetNvInt			(LPCTSTR pName, int    val);
	virtual ERRCODE		SetNvStr			(LPCTSTR pName, LPCTSTR pval);

protected:
	virtual ERRCODE		PerstGetRegKey		(HKEY& hKey);
	virtual ERRCODE		PerstReleaseRegKey	(HKEY hKey);

public:

protected:
	HKEY				m_hKey;
};

#endif


#ifdef BS_PERST_XML

//***********************************************************************
// persistence object via windows registry

class PERSIST_API Bperstxml : public BpersistBase
{
public:
						Bperstxml			(LPCTSTR pName, LPCTSTR pManuf = NULL, bool setifnew = true);
	virtual				~Bperstxml			();

public:				
	virtual ERRCODE		GetNvInt			(LPCTSTR pName, int&    val,  int defval = 0);
	virtual ERRCODE		GetNvStr			(LPCTSTR pName, LPTSTR pval, int cbVal, LPCTSTR defval = NULL);

	virtual ERRCODE		SetNvInt			(LPCTSTR pName, int    val);
	virtual ERRCODE		SetNvStr			(LPCTSTR pName, LPCTSTR pval);

	virtual ERRCODE		GetNextNvInt		(LPCTSTR pName, int&    val,  int defval = 0);
	virtual ERRCODE		GetNextNvStr		(LPCTSTR pName, LPTSTR pval, int cbVal, LPCTSTR defval = NULL);

	virtual ERRCODE		SetNextNvInt		(LPCTSTR pName, int    val);
	virtual ERRCODE		SetNextNvStr		(LPCTSTR pName, LPCTSTR pval);

	virtual ERRCODE		Flush				(LPCTSTR pNewName = NULL, TEXTENCODING = txtUTF8);

	virtual ERRCODE		GetFilePath			(LPTSTR pPath, int nPath);
	virtual ERRCODE		SetFilePath			(LPCTSTR pPath);

	virtual XMLelement* GetTree				(void) { return m_pTree; }

protected:
	virtual ERRCODE		RestoreFromFile		(void);
	virtual ERRCODE		ParseName			(LPCTSTR pName);
	virtual ERRCODE		PerstGetXmlNode		(XMLelement*& pElement);
	virtual ERRCODE		PerstSetXmlNode		(XMLelement*& pElement);

public:

protected:
	BfileStream*		m_pFileStream;
	XMLelement*			m_pTree;
};

#endif

#ifdef BS_PERST_INI

//***********************************************************************
// persistence object via windows .ini file

class PERSIST_API Bperstini : public BpersistBase
{
public:
						Bperstini			(LPCTSTR pName, LPCTSTR pManuf = NULL, bool setifnew = true);
	virtual				~Bperstini			();

public:				
	virtual ERRCODE		GetNvInt			(LPCTSTR pName, int&    val,  int defval = 0);
	virtual ERRCODE		GetNvStr			(LPCTSTR pName, LPTSTR pval, int cbVal, LPCTSTR defval = NULL);

	virtual ERRCODE		SetNvInt			(LPCTSTR pName, int    val);
	virtual ERRCODE		SetNvStr			(LPCTSTR pName, LPCTSTR pval);

protected:
	virtual ERRCODE		ParseName			(LPCTSTR pName);

public:

protected:
};

#endif

#ifdef BS_PERST_RC

//***********************************************************************
// persistence object via .rc file

class PERSIST_API Bperstrc : public BpersistBase
{
public:
						Bperstrc			(LPCTSTR pName, LPCTSTR pManuf = NULL, bool setifnew = true);
	virtual				~Bperstrc			();

public:				
	virtual ERRCODE		GetNvInt			(LPCTSTR pName, int&    val,  int defval = 0);
	virtual ERRCODE		GetNvStr			(LPCTSTR pName, LPTSTR pval, int cbVal, LPCTSTR defval = NULL);

	virtual ERRCODE		SetNvInt			(LPCTSTR pName, int    val);
	virtual ERRCODE		SetNvStr			(LPCTSTR pName, LPCTSTR pval);

protected:
	virtual ERRCODE		ParseName			(LPCTSTR pName);
	virtual ERRCODE		PerstGetRegKey		(HKEY& hKey);

public:

protected:
	BfileStream*		m_pFileStream;
};

#endif

#endif

