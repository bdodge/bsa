
#include "bedx.h"

//**************************************************************************
tagMacroEntry::tagMacroEntry(EditCommand command, LPCTSTR lpData, int nData, ParmType parmType)
{
	m_command = command;
	m_type	  = parmType;
	m_nParm   = nData;
	m_pParm   = new TCHAR [ nData + 2 ];
	memcpy(m_pParm, lpData, nData*sizeof(TCHAR));
	m_next	  = NULL;
}

//**************************************************************************
tagMacroEntry::tagMacroEntry(EditCommand command, int nParm, ParmType parmType)
{
	m_command = command;
	m_type	  = parmType;
	m_nParm   = nParm;
	m_pParm   = NULL;
	m_next	  = NULL;
}

//**************************************************************************
tagMacroEntry::tagMacroEntry(EditCommand command)
{
	m_command = command;
	m_nParm   = 0;
	m_type	  = ptNone;
	m_pParm   = NULL;
	m_next	  = NULL;
}

//**************************************************************************
tagMacroEntry::~tagMacroEntry()
{
	if(m_pParm) delete [] m_pParm;
}


//**************************************************************************
ERRCODE Bview::AddToMacro(LPMACROREC pRec)
{
	if(! m_macro || ! m_macroend)
		m_macro = m_macroend = pRec;
	else
		m_macroend->m_next = pRec;
	m_macroend = pRec;
	return errOK;
}

//**************************************************************************
ERRCODE Bview::KillMacro(LPMACROREC pRec)
{
	LPMACROREC pNext;

	while(pRec)
	{
		pNext = pRec->m_next;
		if(pRec == m_macro)
			m_macro = m_macroend = NULL;
		delete pRec;
		pRec = pNext;
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bview::PlayMacro(LPMACROREC pRec)
{
	ERRCODE ec = errOK;

	m_playing = true;
	
	if(m_buffer)
		m_buffer->GetEditor()->SetStatus(_T("Playing Macro"));
	
	while(pRec && ec == errOK)
	{
		switch(pRec->m_type)
		{
		case ptNone:
			break;

		case ptString:
		case ptOpenFilename:
		case ptSaveFilename:
		case ptDirectory:

			PushParm(pRec->m_pParm, pRec->m_nParm, ptString);
			break;

		case ptTextEncoding:
		case ptTextLineTerm:
		case ptNumber:
		case ptColor:

			PushParm(pRec->m_nParm, pRec->m_type);
			break;
		}
		ec = Dispatch(pRec->m_command);
		pRec = pRec->m_next;
	}
	m_playing = false;
	return ec;
}
