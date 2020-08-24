#include "bedx.h"

//**************************************************************************
BsccsInfo::BsccsInfo(LPCTSTR name, BappPanel* pPanel, Bed* pParent)
	:
	BshellInfo(name, pPanel, pParent)
{
}

//**************************************************************************
BsccsInfo::~BsccsInfo()
{
}


//**************************************************************************
ERRCODE BsccsInfo::Startup(LPCTSTR cmd)
{
	if(! cmd)
		return errBAD_PARAMETER;

	if(! Exited())
	{
		int rc = MessageBox(NULL, _T("Cancel Current Source Control Shell ?"),
			m_view->GetBuffer()->GetEditor()->GetTitle(m_view->GetBuffer()), MB_YESNO);
		if(rc == IDNO) return errFAILURE;

		Stop();
	}
	_tcscpy(m_szShellCommand, cmd);
	_tcscpy(m_szShellDir, _T(""));
	return BshellInfo::Startup();
}

