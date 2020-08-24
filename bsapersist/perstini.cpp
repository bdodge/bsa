
#include "persistx.h"

#ifdef BS_PERST_INI

//**************************************************************************
Bperstini::Bperstini(LPCTSTR pName, LPCTSTR pManuf) : BpersistBase(pName, pManuf)
{
	_tcscat(m_path, _T(".ini"));
}

//**************************************************************************
Bperstini::~Bperstini()
{
}

///**************************************************************************
ERRCODE Bperstini::ParseName(LPCTSTR pName)
{
	ERRCODE ec;

	if((ec = BpersistBase::ParseName(pName)) != errOK) return ec;

	if(m_section[0] == _T('\0'))
		_tcscpy(m_section, _T("APPL"));
	return errOK;
}

//**************************************************************************
ERRCODE Bperstini::GetNvInt(LPCTSTR pName, int& val, int defval)
{
	Block   lock(&m_perstex);
	ERRCODE ec;
	TCHAR   result[32];
	TCHAR   defstr[32];

	val = defval;
	_sntprintf(defstr, 32, _T("%ld"), defval);
	
	ec = GetNvStr(pName, result, sizeof(result), defstr);	
	if(ec != errOK) return ec;
	val = _tcstol(result, NULL, 0);
	return errOK;
}

//**************************************************************************
ERRCODE Bperstini::GetNvStr(LPCTSTR pName, LPTSTR pVal, int cbVal, LPCTSTR defval)
{
	Block   lock(&m_perstex);
	ERRCODE ec;
	DWORD   cb;

	if((ec = ParseName(pName)) != errOK) return ec;
	if((ec = BpersistBase::GetNvStr(pName, pVal, cbVal, defval)) != errOK) return ec;

	cb  = GetPrivateProfileString(m_section, m_parm, defval, pVal, cbVal, m_path);
	return errOK;
}


//**************************************************************************
ERRCODE Bperstini::SetNvInt(LPCTSTR pName, int val)
{
	TCHAR   valstr[32];

	_sntprintf(valstr, 32, _T("%ld"), val);
	return SetNvStr(pName, valstr);
}


//**************************************************************************
ERRCODE Bperstini::SetNvStr(LPCTSTR pName, LPCTSTR pVal)
{
	Block   lock(&m_perstex);
	ERRCODE ec;
	DWORD	cb;

	if(! pName || ! pVal) return errBAD_PARAMETER;

	if((ec = ParseName(pName)) != errOK) return ec;

	cb  = WritePrivateProfileString(m_section, m_parm, pVal, m_path);
	return errOK;
}

#endif



