
#include "persistx.h"

#ifdef BS_PERST_REG

//**************************************************************************
Bperstreg::Bperstreg(LPCTSTR pName, LPCTSTR pManuf, bool setifnew, bool allusers)
	 :
	 BpersistBase(pName, pManuf, setifnew)
{
	TCHAR keyname[MAX_PATH*2];

	m_hKey = NULL;

	Block   lock(&m_perstex);

	_tcscpy(keyname, _T("Software\\"));
	if(pManuf)
	{
		if((_tcslen(pManuf) + _tcslen(keyname)) < MAX_PATH)
		{
			_tcscat(keyname, pManuf);
			_tcscat(keyname, _T("\\"));
		}
	}
	if((_tcslen(pName) + _tcslen(keyname)) < MAX_PATH)
	{
		_tcscat(keyname, pName);
	}
	int err = RegCreateKey((allusers ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER), keyname, &m_hKey);
	if(err != ERROR_SUCCESS) m_hKey = NULL;
}

//**************************************************************************
Bperstreg::~Bperstreg()
{
	if(m_hKey) RegCloseKey(m_hKey);
}


//**************************************************************************
ERRCODE Bperstreg::PerstGetRegKey(HKEY& hKey)
{
	if(m_section[0])
	{
		int err = RegCreateKey(m_hKey, m_section, &hKey);
		if(err != ERROR_SUCCESS)
		{
			return errFAILURE;
		}
	}
	else
	{
		hKey = m_hKey;
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bperstreg::PerstReleaseRegKey(HKEY hKey)
{
	if(hKey != m_hKey)
	{
		RegCloseKey(hKey);
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bperstreg::GetNvInt(LPCTSTR pName, int& val, int defval)
{
	Block   lock(&m_perstex);
	ERRCODE ec;

	m_iter = 0;
	val = defval;
	if((ec = ParseName(pName)) != errOK) return ec;

	HKEY hKey;
	int  err;

	ec = PerstGetRegKey(hKey);
	if(ec != errOK)  return ec;

	BYTE  pData[32];
	DWORD dwType = REG_DWORD;
	DWORD cbData = sizeof(pData);

	err = RegQueryValueEx(hKey, m_parm, NULL, &dwType, pData, &cbData);
	PerstReleaseRegKey(hKey);

	if(err != ERROR_SUCCESS)
	{
		ec = (err == 2) ? errPERSIST_PARM_NOT_FOUND : errFAILURE;
	}
	else
	{
		if(dwType == REG_SZ)
			val = _tcstol((LPTSTR)pData, NULL, 0);
		else
			memcpy(&val, pData, sizeof(DWORD));
		return errOK;
	}
	if(ec == errPERSIST_PARM_NOT_FOUND && m_setifnew)
	{
		SetNvInt(pName, defval);
	}
	return ec;
}

//**************************************************************************
ERRCODE Bperstreg::GetNvStr(LPCTSTR pName, LPTSTR pVal, int cbVal, LPCTSTR defval)
{
	Block   lock(&m_perstex);
	ERRCODE ec;

	m_iter = 0;
	if((ec = BpersistBase::GetNvStr(pName, pVal, cbVal, defval)) != errOK) return ec;
	if((ec = ParseName(pName)) != errOK) return ec;

	HKEY hKey;
	int  err;

	ec = PerstGetRegKey(hKey);
	if(ec != errOK)  return ec;

	DWORD dwType = REG_SZ;

	err = RegQueryValueEx(hKey, m_parm, NULL, &dwType, (LPBYTE)pVal, (LPDWORD)&cbVal);
	PerstReleaseRegKey(hKey);

	if(err != ERROR_SUCCESS)
	{
		ec = (err == 2) ? errPERSIST_PARM_NOT_FOUND : errFAILURE;
	}
	if(ec == errPERSIST_PARM_NOT_FOUND && m_setifnew)
	{
		SetNvStr(pName, defval);
	}
	return ec;
}


//**************************************************************************
ERRCODE Bperstreg::SetNvInt(LPCTSTR pName, int val)
{
	Block   lock(&m_perstex);
	ERRCODE ec;

	m_iter = 0;
	if((ec = ParseName(pName)) != errOK) return ec;

	HKEY hKey;
	int  err;

	ec = PerstGetRegKey(hKey);
 	if(ec != errOK)  return ec;

	err = RegSetValueEx(hKey, m_parm, NULL, REG_DWORD, (LPBYTE)&val, sizeof(DWORD));
	PerstReleaseRegKey(hKey);
	if(err != ERROR_SUCCESS) return errFAILURE;

	return errOK;
}

//**************************************************************************
ERRCODE Bperstreg::SetNvStr(LPCTSTR pName, LPCTSTR pVal)
{
	Block   lock(&m_perstex);
	ERRCODE ec;

	m_iter = 0;
	if(! pName || ! pVal) return errBAD_PARAMETER;

	if((ec = ParseName(pName)) != errOK) return ec;

	HKEY hKey;
	int  err;

	ec = PerstGetRegKey(hKey);
 	if(ec != errOK)  return ec;

	err = RegSetValueEx(hKey, m_parm, NULL, REG_SZ, (LPBYTE)pVal, (_tcslen(pVal) + 1) * sizeof(TCHAR));
	PerstReleaseRegKey(hKey);
	if(err != ERROR_SUCCESS) return errFAILURE;

	return errOK;
}


#endif
