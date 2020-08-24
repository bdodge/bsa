
#include "persistx.h"

#ifdef BS_PERSIST_RC

//**************************************************************************
Bperstrc::Bperstrc(LPCTSTR pName, LPCTSTR pManuf)
{
}

//**************************************************************************
Bperstrc::~Bperstrc()
{
}

//**************************************************************************
ERRCODE Bperstrc::Flush(LPCTSTR pName)
{
	Block   lock(&m_perstex);
	ERRCODE ec;

	return errOK;
}


//**************************************************************************
ERRCODE Bperstrc::GetNvInt(LPCTSTR pName, int& val, int defval)
{
	Block   lock(&m_perstex);
	ERRCODE ec;

	m_iter = 0;
	val = defval;
	if((ec = ParseName(pName)) != errOK) return ec;
	return errOK;
}

//**************************************************************************
ERRCODE Bperstrc::GetNvStr(LPCTSTR pName, LPTSTR pVal, int cbVal, LPCTSTR defval)
{
	Block   lock(&m_perstex);
	ERRCODE ec;

	m_iter = 0;
	if((ec = BpersistBase::GetNvStr(pName, pVal, cbVal, defval)) != errOK) return ec;
	if((ec = ParseName(pName)) != errOK) return ec;
	return errOK;
}

//**************************************************************************
ERRCODE Bperstrc::SetNvInt(LPCTSTR pName, int val)
{
	Block   lock(&m_perstex);
	ERRCODE ec;

	m_iter = 0;
	if((ec = ParseName(pName)) != errOK) return ec;
	return errOK;
}

//**************************************************************************
ERRCODE Bperstrc::SetNvStr(LPCTSTR pName, LPCTSTR pVal)
{
	Block   lock(&m_perstex);
	ERRCODE ec;

	m_iter = 0;
	if(! pName || ! pVal) return errBAD_PARAMETER;

	if((ec = ParseName(pName)) != errOK) return ec;
	return errOK;
}

#endif

