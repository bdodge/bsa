
#include "persistx.h"

#ifdef BS_PERST_XML

//**************************************************************************
Bperstxml::Bperstxml(LPCTSTR pName, LPCTSTR pManuf, bool setifnew)
		:
		BpersistBase(pName, pManuf, setifnew)
{
	ERRCODE ec;

	Block   lock(&m_perstex);

	m_pFileStream = NULL;
	m_pTree		  = NULL;

	_tcscat(m_path, _T(".xml"));

	m_pFileStream = new BfileStream();
	ec = m_pFileStream->Open(m_path, _T("r"));

	if(ec == errOK)
	{
		XMLparser parser;

		ec = parser.XMLdomParse(m_pFileStream, m_pTree);
	}
	m_pFileStream->Close();
}

//**************************************************************************
Bperstxml::~Bperstxml()
{
	Flush();
	if(m_pTree) delete m_pTree;
	if(m_pFileStream) delete m_pFileStream;
}

//**************************************************************************
ERRCODE Bperstxml::RestoreFromFile()
{
	ERRCODE ec;

	Block   lock(&m_perstex);

	ec = m_pFileStream->Open(m_path, _T("r"));

	if(ec == errOK)
	{
		XMLparser parser;

		if(m_pTree)
		{
			delete m_pTree;
			m_pTree = NULL;
		}
		ec = parser.XMLdomParse(m_pFileStream, m_pTree);
	}
	m_pFileStream->Close();
	return ec;
}

//**************************************************************************
ERRCODE Bperstxml::GetFilePath(LPTSTR pPath, int nPath)
{
	if(nPath <= 0 || pPath == NULL)
		return errBAD_PARAMETER;
	_tcsncpy(pPath, m_path, nPath - 1);
	pPath[nPath-1] = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bperstxml::SetFilePath(LPCTSTR pPath)
{
	if(pPath == NULL)
		return errBAD_PARAMETER;
	_tcsncpy(m_path, pPath, MAX_PATH - 1);
	m_path[MAX_PATH-1] = _T('\0');
	return RestoreFromFile();
}


//**************************************************************************
ERRCODE Bperstxml::ParseName(LPCTSTR pName)
{
	LPTSTR  pn;
	ERRCODE ec;
	
	ec = BpersistBase::ParseName(pName);
	if(ec != errOK) return ec;
	
	// convert spaces in name and section to underscores
	// since xml is space sensitive in names
	//
	for(pn = m_section; *pn; pn++)
		if(*pn == _T(' '))
			*pn = _T('-');
	for(pn = m_parm; *pn; pn++)
		if(*pn == _T(' '))
			*pn = _T('-');
	return ec;
}

//**************************************************************************
ERRCODE Bperstxml::Flush(LPCTSTR pName, TEXTENCODING encoding)
{
	Block   lock(&m_perstex);
	ERRCODE ec;

	XMLparser    parser;
	BfileStream  stream;
	BfileStream* pFileStream;

	if (! m_dirty)
	{
		// don't bother if no changes
		//
		return errOK;
	}
	if(pName != NULL)
	{
		stream.Open(pName, _T("w"));
		pFileStream = &stream;
	}
	else
	{
		pFileStream = m_pFileStream;
		pFileStream->Open(m_path, _T("w"));
	}
	ec = parser.XMLtoStream(m_pTree, pFileStream, encoding);

	if(pFileStream)
		pFileStream->Flush();
	pFileStream->Close();
	return ec;
}


//**************************************************************************
ERRCODE Bperstxml::PerstGetXmlNode(XMLelement*& pElement)
{
	TCHAR  element[MAX_PATH];
	TCHAR* pp;
	TCHAR* en;

	pElement = NULL;

	if(m_section[0])
	{
		pp = m_section;

		do
		{
			if(*pp)
			{
				en = element;
				while(*pp && *pp != _T('/') && *pp != _T('\\'))
					*en++ = *pp++;
				*en++ = _T('\0');
				if(*pp) pp++;
			}
			if(pElement == NULL)
			{
				pElement = m_pTree ? m_pTree->FindNextElement(element) : NULL;
			}
			else if(pElement->GetChild())
			{
				pElement = pElement->GetChild()->FindSiblingElement(element);
			}
		}
		while(pElement && *pp);
	}
	else
	{
		pElement = m_pTree;
	}
	if(pElement)
	{
		if(pElement->GetChild())
			pElement = pElement->GetChild()->FindSiblingElement(m_parm);
		else
			pElement = NULL;
	}
	if(! pElement) return errPERSIST_PARM_NOT_FOUND;
	else           return errOK;
}

//**************************************************************************
ERRCODE Bperstxml::PerstSetXmlNode(XMLelement*& pElement)
{
	XMLelement* pNextElement;

	TCHAR  element[MAX_PATH];
	TCHAR* pp;
	TCHAR* en;

	pElement = NULL;

	if(m_section[0])
	{
		pp = m_section;

		// extract and subfind each element of path in section
		do
		{
			if(*pp)
			{
				en = element;
				while(*pp && *pp != _T('/') && *pp != _T('\\'))
					*en++ = *pp++;
				*en++ = _T('\0');
				if(*pp) pp++;
			}
			if(pElement == NULL)
			{
				if(m_pTree)
				{
					pElement = m_pTree->FindNextElement(element);
					if(! pElement)
					{
						pElement = m_pTree->GetChild();
						if(pElement)
							pElement = pElement->AddSibling(element, pElement);
						else
							pElement = m_pTree->AddChild(element);
					}
				}
				else
				{
					m_pTree = new XMLelement(_T("persisted"), NULL);
					pElement = m_pTree->AddChild(element);
				}
			}
			else
			{
				if(pElement->GetChild())
					pNextElement = pElement->GetChild()->FindSiblingElement(element);
				else
					pNextElement = NULL;

				if(! pNextElement)
				{
					pNextElement = pElement->GetChild();
					if(pNextElement)
						pElement = pNextElement->AddSibling(element, pNextElement);
					else
						pElement = pElement->AddChild(element);
				}
				else
				{
					pElement = pNextElement;
				}
			}
		}
		while(pElement && *pp);
	}
	else
	{
		pElement = m_pTree;
		if(! pElement)
		{
			m_pTree = new XMLelement(_T("persisted"), NULL);
			m_pTree->AddChild(m_parm);
			pElement = m_pTree;
		}
	}
	if(pElement)
	{
		pNextElement = pElement->GetChild();
		if(pNextElement)
		{
			pNextElement = pNextElement->FindSiblingElement(m_parm);
		}
		if(! pNextElement)
		{
			pNextElement = pElement->GetChild();
			if(pNextElement)
				pElement = pNextElement->AddSibling(m_parm, pNextElement);
			else
				pElement = pElement->AddChild(m_parm);
		}
		else
		{
			pElement = pNextElement;
		}
	}
	else
	{
		// cant happen
		return errPERSIST_PARM_NOT_FOUND;
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bperstxml::GetNvInt(LPCTSTR pName, int& val, int defval)
{
	ERRCODE ec;

	{
		Block   lock(&m_perstex);
		m_iter = 0;
		val = defval;
		if((ec = ParseName(pName)) != errOK) return ec;

		XMLelement* pNode;
		XMLvalue*   pValue;

		ec = PerstGetXmlNode(pNode);
		if(ec == errOK)
		{
			if(! pNode || ! (pValue = pNode->GetValues()))
				ec = errPERSIST_PARM_NOT_FOUND;
			else
				val = _tcstol(pValue->GetValue(), NULL, 0);
		}
	}
	if(ec == errPERSIST_PARM_NOT_FOUND && m_setifnew)
	{
		SetNvInt(pName, defval);
	}
	return ec;
}

//**************************************************************************
ERRCODE Bperstxml::GetNextNvInt(LPCTSTR pName, int& val, int defval)
{
	Block   lock(&m_perstex);
	ERRCODE ec;

	if((ec = ParseName(pName)) != errOK) return ec;
	return errOK;
}

//**************************************************************************
ERRCODE Bperstxml::GetNvStr(LPCTSTR pName, LPTSTR pVal, int cbVal, LPCTSTR defval)
{
	ERRCODE ec;

	{
		Block   lock(&m_perstex);
		m_iter = 0;
		ec = BpersistBase::GetNvStr(pName, pVal, cbVal, defval);
		if(ec != errOK) return ec;
		if((ec = ParseName(pName)) != errOK) return ec;

		XMLelement* pNode;
		XMLvalue*   pValue;

		ec = PerstGetXmlNode(pNode);
		if(ec == errOK)
		{
			if(! pNode || ! (pValue = pNode->GetValues()))
			{
				ec = errPERSIST_PARM_NOT_FOUND;
			}
			else
			{
				if(_tcslen(pValue->GetValue()) >= (size_t)cbVal)
				{
					_tcsncpy(pVal, pValue->GetValue(), cbVal - 1);
					pVal[cbVal - 1] = 0;
				}
				else
				{
					_tcscpy(pVal, pValue->GetValue());
				}
			}
		}
	}
	if(ec == errPERSIST_PARM_NOT_FOUND && m_setifnew)
	{
		SetNvStr(pName, defval);
	}
	return ec;
}

//**************************************************************************
ERRCODE Bperstxml::GetNextNvStr(LPCTSTR pName, LPTSTR pVal, int cbVal, LPCTSTR defval)
{
	Block   lock(&m_perstex);
	ERRCODE ec;

	if((ec = ParseName(pName)) != errOK) return ec;
	return errOK;
}

//**************************************************************************
ERRCODE Bperstxml::SetNvInt(LPCTSTR pName, int val)
{
	Block   lock(&m_perstex);
	ERRCODE ec;

	m_iter = 0;
	if((ec = ParseName(pName)) != errOK) return ec;

	XMLelement* pNode;
	XMLvalue*   pValue;

	ec = PerstSetXmlNode(pNode);
	if(ec != errOK)	return ec;
	if(! pNode)
		return errPERSIST_PARM_NOT_FOUND;

	TCHAR valStr[32];

	_sntprintf(valStr, 32, _T("%d"), val);

	pValue = pNode->GetValues();
	if(pValue)
		ec = pNode->ChangeValue(pValue, valStr);
	else
		ec = pNode->AddValue(valStr);
	m_dirty = true;
	return errOK;
}

//**************************************************************************
ERRCODE Bperstxml::SetNextNvInt(LPCTSTR pName, int val)
{
	Block   lock(&m_perstex);
	ERRCODE ec;

	if((ec = ParseName(pName)) != errOK) return ec;
	return errOK;
}

//**************************************************************************
ERRCODE Bperstxml::SetNvStr(LPCTSTR pName, LPCTSTR pVal)
{
	Block   lock(&m_perstex);
	ERRCODE ec;

	m_iter = 0;
	if(! pName || ! pVal) return errBAD_PARAMETER;

	if((ec = ParseName(pName)) != errOK) return ec;

	XMLelement* pNode;
	XMLvalue*   pValue;

	ec = PerstSetXmlNode(pNode);
	if(ec != errOK)	return ec;
	if(! pNode)
		return errPERSIST_PARM_NOT_FOUND;

	pValue = pNode->GetValues();
	if(pValue)
		ec = pNode->ChangeValue(pValue, pVal);
	else
		ec = pNode->AddValue(pVal);
	m_dirty = true;
	return errOK;
}

//**************************************************************************
ERRCODE Bperstxml::SetNextNvStr(LPCTSTR pName, LPCTSTR val)
{
	Block   lock(&m_perstex);
	ERRCODE ec;

	if((ec = ParseName(pName)) != errOK) return ec;

	return errOK;
}

#endif


