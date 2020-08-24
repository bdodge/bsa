#include "bedx.h"


//**************************************************************************
Bline::Bline(LPTSTR lpText, int nText)
		:
		m_text(lpText),
		m_len(nText),
		m_next(NULL),
		m_prev(NULL),
		m_alen(0),
		m_info(liUnknown)
{
}

//**************************************************************************
Bline::~Bline()
{
	if(m_text && m_alen)
		delete [] m_text;
}

//**************************************************************************
ERRCODE	Bline::GetText(LPCTSTR& pText, int& nText)
{
	pText = m_text;
	nText = m_len;
	return errOK;
}

//**************************************************************************
ERRCODE	Bline::SetText(LPTSTR pText, int nText)
{
	m_text = pText;
	m_len  = nText;
	return errOK;
}

