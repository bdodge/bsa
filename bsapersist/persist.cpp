
#include "persistx.h"


//**************************************************************************
BpersistBase::BpersistBase(LPCTSTR pName, LPCTSTR pManuf, bool setifnew)
		:
		m_setifnew(setifnew), m_iter(0), m_setiter(0), m_dirty(false)
{
	ERRCODE ec;
	int		room;
	LPTSTR  pn, px;

	if(! pName)	pName = _T("bsaDefaultPersist");
	ec =  BUtil::GetHomePath(m_path, MAX_PATH);

	room = MAX_PATH - _tcslen(m_path);	
	pn = m_path + _tcslen(m_path);
	
	if((int)_tcslen(pName) >= room)
	{
		_tcsncpy(pn, pName, room - 1);
		m_path[MAX_PATH-1] = _T('\0');
	}
	else
	{
		_tcscpy(pn, pName);
	}
#ifndef Windows
	for(px = m_path; *px; px++)
	{
		if(*px == _T(' '))
			*px = _T('_');
		if(px >= pn && (*px == _T('/') || *px == _T('\\')))
			*px = _T('_');
	}
#endif
}

//**************************************************************************
BpersistBase::~BpersistBase()
{
}

//**************************************************************************
ERRCODE BpersistBase::Flush(LPCTSTR pName)
{
	return errOK;
}

///**************************************************************************
ERRCODE BpersistBase::ParseName(LPCTSTR pName)
{
	ERRCODE ec = errOK;
	TCHAR*  pn;

	if(! pName || (_tcslen(pName) >= MAX_PATH)) return errBAD_PARAMETER;

	// split "path" in name into "section":"parmname"
	//
	_tcscpy(m_section, pName);
	for(pn = m_section + _tcslen(m_section) - 1; pn >= m_section; pn--)
	{
		if(*pn == _T('/') || *pn == _T('\\'))
		{
			*pn++ = _T('\0');
			_tcscpy(m_parm, pn);
			break;
		}
	}
	if(pn <= m_section)
	{
		_tcscpy(m_parm, pName);
		m_section[0] = _T('\0');
	}
	else
	{
		for(pn = m_section; *pn; pn++)
#ifdef Windows
			if(*pn == _T('/')) *pn = '\\';
#else
			if(*pn == _T('\\')) *pn = '/';
#endif
	}
	return ec;
}

//**************************************************************************
ERRCODE BpersistBase::GetNvInt(LPCTSTR pName, int& val, int defval)
{
	m_iter = 0;
	val = defval;
	return errOK;
}
#if 0
//**************************************************************************
ERRCODE BpersistBase::GetNvInt(LPCTSTR pName, short& val, short defval)
{
	ERRCODE ec;
	int     iv;

	val = (short)defval;
	ec = GetNvInt(pName, iv, defval);
	if(ec != errOK) return ec;
	if((DWORD)iv > 0xffffUL) return errOVERFLOW;
	val = (short)iv;
	return errOK;
}
#endif

//**************************************************************************
ERRCODE BpersistBase::GetNextNvInt(LPCTSTR pName, int& val, int defval)
{
	TCHAR   pnn[MAX_PATH + 32];
	ERRCODE ec;

	m_setiter = ++m_iter;
	if(_tcslen(pName) >= MAX_PATH) return errBAD_PARAMETER;
	_sntprintf(pnn, MAX_PATH + 32, _T("%s_%d"), pName, m_setiter);

	ec = GetNvInt(pnn, val, defval);
	m_iter = m_setiter;
	return ec;
}

//**************************************************************************
ERRCODE BpersistBase::GetNvBool(LPCTSTR pName, bool& val, bool defval)
{
	ERRCODE ec;
	int     ival;

	val = defval;
	if((ec = GetNvInt(pName, ival, defval ? 1 : 0)) != errOK) return ec;
	val = ival != 0;
	return errOK;
}

//**************************************************************************
ERRCODE BpersistBase::GetNextNvBool(LPCTSTR pName, bool& val, bool defval)
{
	ERRCODE ec;
	int     ival;

	val = defval;
	if((ec = GetNextNvInt(pName, ival, defval ? 1 : 0)) != errOK) return ec;
	val = ival != 0;
	return errOK;
}

//**************************************************************************
ERRCODE BpersistBase::GetNvStr(LPCTSTR pName, LPTSTR pVal, int cbVal, LPCTSTR defval)
{
	if(! pName || ! pVal || ! cbVal) return errBAD_PARAMETER;

	m_iter = 0;
	if(defval)
	{
		if(_tcslen(defval) >= (size_t)cbVal) 
		{
			_tcsncpy(pVal, defval, cbVal - 1);
			pVal[cbVal - 1] = _T('\0');
		}
		else if(pVal != defval)
		{
			_tcscpy(pVal, defval);
		}
	}
	return errOK;
}

//**************************************************************************
ERRCODE BpersistBase::GetNextNvStr(LPCTSTR pName, LPTSTR pVal, int cbVal, LPCTSTR defval)
{
	TCHAR   pnn[MAX_PATH + 32];
	ERRCODE	ec;

	m_setiter = ++m_iter;
	if(_tcslen(pName) >= MAX_PATH) return errBAD_PARAMETER;
	_sntprintf(pnn, MAX_PATH + 32, _T("%s_%d"), pName, m_setiter);

	ec = GetNvStr(pnn, pVal, cbVal, defval);
	m_iter = m_setiter;
	return ec;
}

//**************************************************************************
ERRCODE BpersistBase::SetNvInt(LPCTSTR pName, int val)
{
	m_iter = 0;
	return errOK;
}

//**************************************************************************
ERRCODE BpersistBase::SetNextNvInt(LPCTSTR pName, int val)
{
	TCHAR   pnn[MAX_PATH + 32];
	ERRCODE ec;

	m_setiter = ++m_iter;
	if(_tcslen(pName) >= MAX_PATH) return errBAD_PARAMETER;
	_sntprintf(pnn, MAX_PATH + 32, _T("%s_%d"), pName, m_setiter);

	ec = SetNvInt(pnn, val);
	m_iter = m_setiter;
	return ec;
}

//**************************************************************************
ERRCODE BpersistBase::SetNvBool(LPCTSTR pName, bool val)
{
	return SetNvInt(pName, val ? 1 : 0);
}

//**************************************************************************
ERRCODE BpersistBase::SetNextNvBool(LPCTSTR pName, bool val)
{
	return SetNextNvInt(pName, val ? 1 : 0);
}

//**************************************************************************
ERRCODE BpersistBase::SetNvStr(LPCTSTR pName, LPCTSTR pVal)
{
	m_iter = 0;
	return errOK;
}

//**************************************************************************
ERRCODE BpersistBase::SetNextNvStr(LPCTSTR pName, LPCTSTR val)
{
	TCHAR   pnn[MAX_PATH + 32];
	ERRCODE ec;

	m_setiter = ++m_iter;
	if(_tcslen(pName) >= MAX_PATH) return errBAD_PARAMETER;
	_sntprintf(pnn, MAX_PATH + 32, _T("%s_%d"), pName, m_setiter);

	ec = SetNvStr(pnn, val);
	m_iter = m_setiter;
	return ec;
}



