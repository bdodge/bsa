#include "xmlx.h"

static struct _tag_entities
{
    LPCTSTR ename;
    WCHAR   eval;
}
g_entitytab[] = 
{
    { _T("quot"),  	34 },
    { _T("amp"), 	38 },
    { _T("lt"), 	60 },
    { _T("gt"), 	62 },
    { _T("OElig"), 	338 },
    { _T("oelig"), 	339 },
    { _T("Scaron"), 352 },
    { _T("scaron"), 353 },
    { _T("Yuml"), 	376 },
    { _T("circ"), 	710 },
    { _T("tilde"), 	732 },
    { _T("ensp"), 	8194 },
    { _T("emsp"), 	8195 },
    { _T("thinsp"), 8201 },
    { _T("zwnj"), 	8204 },
    { _T("zwj"), 	8205 },
    { _T("lrm"), 	8206 },
    { _T("rlm"), 	8207 },
    { _T("ndash"), 	8211 },
    { _T("mdash"), 	8212 },
    { _T("lsquo"), 	8216 },
    { _T("rsquo"), 	8217 },
    { _T("sbquo"), 	8218 },
    { _T("ldquo"), 	8220 },
    { _T("rdquo"), 	8221 },
    { _T("bdquo"), 	8222 },
    { _T("dagger"), 8224 },
    { _T("Dagger"), 8225 },
    { _T("permil"), 8240 },
    { _T("lsaquo"), 8249 },
    { _T("rsaquo"), 8250 },
    { _T("euro"), 	8364 }
};

//***********************************************************************
LPTSTR DupString(LPCTSTR pSrc)
{
	LPTSTR pr = new TCHAR [ _tcslen(pSrc) + 2 ];
	_tcscpy(pr, pSrc);
	return pr;
}
#ifdef UNICODE
//***********************************************************************
LPSTR DupString(LPCSTR pSrc)
{
	LPSTR pr = new char [ strlen(pSrc) + 2 ];
	strcpy(pr, pSrc);
	return pr;
}
#endif

//***********************************************************************
XMLelement::XMLelement(LPCTSTR pName, XMLelement* pParent)
			:
			m_pParent(pParent),
			m_pChild(NULL),
			m_pSibling(NULL),
			m_pAttribs(NULL),
			m_pValues(NULL)
{
	m_pName = new TCHAR [ _tcslen (pName) + 2 ];
	_tcscpy(m_pName, pName);
}

//***********************************************************************
XMLelement::~XMLelement()
{
	// free sub nodes if any
	//
	if(m_pChild)
		delete m_pChild;
	if(m_pSibling)
		delete m_pSibling;
	if(m_pName)
		delete [] m_pName;
	if(m_pAttribs)
		delete m_pAttribs;
	if(m_pValues)
		delete m_pValues;
}

//***********************************************************************
XMLelement* XMLelement::FindNextElement(LPCTSTR pName)
{
	XMLelement* pNext = this;

	do
	{
		if(! _tcscmp(pName, pNext->m_pName))
			return pNext;
		if(pNext->m_pChild)			pNext = pNext->m_pChild;
		else if(pNext->m_pSibling)	pNext = pNext->m_pSibling;
		else while((pNext = pNext->m_pParent) != NULL)
			if(pNext->m_pSibling) { pNext = pNext->m_pSibling; break; }
	}
	while(pNext);
	return NULL;
}

//***********************************************************************
XMLelement* XMLelement::FindSiblingElement(LPCTSTR pName)
{
	XMLelement* pNext;

	for(pNext = this; pNext; pNext = pNext->m_pSibling)
	{
		if(! _tcscmp(pName, pNext->m_pName))
			return pNext;
	}
	return NULL;
}

//***********************************************************************
XMLelement* XMLelement::FindChildElement(LPCTSTR pName)
{
	XMLelement* pNext;

	for(pNext = m_pChild; pNext; pNext = pNext->m_pChild)
	{
		if(! _tcscmp(pName, pNext->m_pName))
			return pNext;
	}
	return NULL;
}

//***********************************************************************
XMLelement* XMLelement::AddChild(LPCTSTR pName, XMLelement* pAfter)
{
	XMLelement* pNew;

	pNew = new XMLelement(pName, this);
	if(! pAfter) pAfter = this;
	pNew->m_pChild   = pAfter->m_pChild;
	pAfter->m_pChild = pNew;
	pNew->m_pParent = pAfter;
	if(pNew->m_pChild) pNew->m_pChild->m_pParent = pNew;
	return pNew;
}

//***********************************************************************
XMLelement* XMLelement::AddSibling(LPCTSTR pName, XMLelement* pAfter)
{
	XMLelement* pNew;

	pNew = new XMLelement(pName, this);
	if(! pAfter) pAfter = this;
	pNew->m_pSibling   = pAfter->m_pSibling;
	pNew->m_pParent    = pAfter->m_pParent;
	pAfter->m_pSibling = pNew;
	return pNew;
}

//***********************************************************************
ERRCODE XMLelement::ChangeValue(XMLvalue* pValue, LPCTSTR pVal)
{
	int newlen, oldlen;

	if(! pValue || ! pVal) return errBAD_PARAMETER;
	newlen = _tcslen(pVal);

	oldlen = _tcslen(pValue->m_pValue);

	if(newlen > oldlen)
	{
		delete [] pValue->m_pValue;
		pValue->m_pValue = NULL;
	}
	if(! pValue->m_pValue)
	{
		pValue->m_pValue = new TCHAR [ newlen + 2 ];
	}
	_tcscpy(pValue->m_pValue, pVal);
	return errOK;
}

//***********************************************************************
ERRCODE XMLelement::AddValue(LPCTSTR pVal, XMLvalue* pAfter)
{
	XMLvalue* pValue;

	if(! pVal) return errBAD_PARAMETER;

	pValue = new XMLvalue(_T(""), DupString(pVal));

	if(pAfter)
	{
		pValue->m_pNext = NULL;
		pAfter->m_pNext = pValue;
	}
	else
	{
		pValue->m_pNext = m_pValues;
		m_pValues = pValue;
	}
	return errOK;
}


//***********************************************************************
XMLparser::XMLparser() :

	m_parseState(psBase),
	m_token(NULL),
	m_tokLen(0),
	m_tokAlloc(0),
	m_cleanToken(NULL),
	m_ctAlloc(0),
	m_column(0),
	m_lines(0),
	m_depth(0),
	m_attribName(NULL),
	m_anAlloc(0)
{
}

//***********************************************************************
XMLparser::~XMLparser()
{
	if(m_token)		 delete [] m_token;
	if(m_cleanToken) delete [] m_cleanToken;
	if(m_attribName) delete [] m_attribName;
}

//***********************************************************************
LPTSTR XMLparser::DepthString(void)
{
	static TCHAR ds[256];

	int len = m_depth * 2;

	ds[len] = _T('\0');
	while(--len >= 0) 
		ds[len] = _T(' ');
	return ds;
}

//***********************************************************************
ERRCODE XMLparser::AddToToken(TCHAR nc)
{
	if(((m_tokLen+1) >= m_tokAlloc) || ! m_token)
	{
		TCHAR* pnt;

		m_tokAlloc = m_tokLen * 4 + 2;
		pnt = new TCHAR [ m_tokAlloc ];
		if(m_token) {
			memcpy(pnt, m_token, m_tokLen * sizeof(TCHAR));
			delete [] m_token;
		}
		m_token = pnt;
	}
	m_token[m_tokLen++] = nc;
	return errOK;
}

//***********************************************************************
LPTSTR XMLparser::FormatToken(LPCTSTR pToken, XMLtokenType type)
{
	int    len = _tcslen(pToken) + 2;
	LPTSTR pt;
	LPTSTR pe;

	if(len > m_ctAlloc || ! m_cleanToken)
	{
		m_ctAlloc = len * 6;
		if(m_cleanToken) delete [] m_cleanToken;
		m_cleanToken = new TCHAR [ m_ctAlloc ];
	}

	// first unescape the pToken to remove "%xx" escaped text
	// this is ok to do in-situ
	//
	BUtil::UnescapeString(m_cleanToken, pToken);
	//_tcscpy(m_cleanToken, pToken);

	// next, clean up the token 
	//
	if(type == xmlElement)
	{
		// strip formatting
		for(pt = m_cleanToken; *pt && (*pt == _T('<') || *pt == _T('/'));)
			pt++;
		for(pe = pt + _tcslen(pt) - 1; pe >= pt && (*pe == _T('>') || *pe == _T('/'));)
			pe--;
		*++pe = _T('\0');
	}
	else if(type == xmlComment)
	{
		// strip formatting
		for(pt = m_cleanToken; *pt && (*pt == _T('<') || *pt == _T('-') || *pt == _T('!'));)
			pt++;
		while(*pt == _T(' ') || *pt == _T('\t'))
			pt++;
		for(pe = pt + _tcslen(pt) - 1; pe >= pt && (*pe == _T('>') || *pe == _T('-'));)
			pe--;
		while(pe > pt && (*pe == _T(' ') || *pe == _T('\t')))
			pe--;
		*++pe = _T('\0');
	}
	else
	{
		pt = m_cleanToken;
		for(pe = pt + _tcslen(pt) - 1; pe >= pt &&
				(*pe == _T(' ') || *pe == _T('\t') || *pe == _T('\r') || *pe == _T('\n'));)
			pe--;
		*++pe = _T('\0');
	}
	return pt;
}

//***********************************************************************
ERRCODE XMLparser::OnDomXmlToken(LPCTSTR pToken, XMLtokenType tokType)
{
	XMLelement*		pElement;
	XMLelement*		pParent;
	XMLelement*		pChild;
	LPTSTR			pCleanToken;

	pCleanToken = FormatToken(pToken, tokType);

    //_tprintf(_T("%s tok=%ls\n"), DepthString(), pCleanToken);
    
	switch(tokType)
	{
	case xmlStart:

		// setup current state
		//
		m_pCurElement = NULL;
		m_pCurAttrib  = NULL;
		m_pList = NULL;
		break;

	case xmlEnd:

		break;

	case xmlVersion:

		break;

	case xmlElement:

		switch(pToken[1])
		{
		case _T('/'):

			// end element, this should match the current element
			//
			if(! m_pCurElement || (pToken[0] == _T('<') && _tcscmp(m_pCurElement->GetName(), pCleanToken)))
				return errXML_MISSING_OPEN_ELEMENT;
			if(m_pCurElement == m_pList)
			{
				// technically this is the end of the xml and nothing more
				// should come after this
				//
				m_pCurElement = NULL;
			}
			else
			{
				m_pCurElement = m_pCurElement->GetParent();
				if(! m_pCurElement)
					return errXML_MISSING_OPEN_ELEMENT;
			}
			m_depth--;
			break;
			
		default:

			//_ftprintf(stdout, _T("%seletok=%s\n"), DepthString(), pCleanToken);

			// new element, add as sibling of last child of currently open element or
			// as sibling of the last top level element
			//
			pParent  = m_pCurElement;
			pElement = new XMLelement(pCleanToken, pParent);
			m_pCurElement = pElement;
			if(! pParent)
			{
				if(! m_pList)
				{
					m_pList = pElement;
				}
				else
				{
					for(pParent = m_pList; pParent->GetSibling();)
						pParent = pParent->GetSibling();
					pParent->SetSibling(pElement);
				}
			}
			else
			{
				pChild = pParent->GetChild();
				if(! pChild)
				{
					pParent->SetChild(pElement);
				}
				else
				{
					while(pChild->GetSibling())
						pChild = pChild->GetSibling();
					pChild->SetSibling(pElement);
				}
			}
			m_depth++;
			break;
		}
		break;

	case xmlValue:

		//_tprintf(_T(""_Pfs_"valtok="_Pfs_"\n"), DepthString(), pCleanToken);
		if(m_pCurElement)
		{
			XMLvalue* nv = new XMLvalue(_T(""), DupString(pCleanToken));

			m_pCurElement->m_pValues = XMLvalue::AddToList(nv, m_pCurElement->m_pValues);
		}
		break;

	case xmlAttributeName:

		{
			int len;
			
			len = _tcslen(pToken);

			if(len >= m_anAlloc || ! m_attribName)
			{
				m_anAlloc = len * 2 + 2;
				if(m_attribName) delete [] m_attribName;
				m_attribName = new TCHAR [ m_anAlloc ];
			}
			_tcscpy(m_attribName, pCleanToken);
		}
		break;

	case xmlAttributeValue:

		//_ftprintf(stdout, _T(""_Pfs_"atttok="_Pfs_"="_Pfs_"\n"), DepthString(), m_attribName, pCleanToken);

		if(m_pCurElement)
		{
			XMLattribute* nv = new XMLattribute(m_attribName, DupString(pCleanToken));

			m_pCurElement->m_pAttribs = XMLattribute::AddToList(nv, m_pCurElement->m_pAttribs);
		}
		m_attribName[0] = _T('\0');
		break;

	case xmlComment:

		//_ftprintf(stdout, _T(""_Pfs_"comtok="_Pfs_"\n"), DepthString(), pCleanToken);

		//XMLattribute::AddToList(m_pCurElement ? m_pCurElement->GetName() : _T(""),
		//	BUtil::DupString(pCleanToken), m_pCurElement->m_pComments);
		break;
	}
	return errOK;
}

//***********************************************************************
ERRCODE XMLparser::OnDomXmlTokenStub(LPCTSTR pToken, XMLtokenType tokType, LPVOID pCookie) // STATIC
{
	if(! pToken || ! pCookie)
		return errBAD_PARAMETER;

	XMLparser* _this = (XMLparser*)pCookie;

	return _this->OnDomXmlToken(pToken, tokType);
}

//***********************************************************************
ERRCODE	XMLparser::XMLdomParse(Bstream* pStream, XMLelement*&pList)
{
	ERRCODE ec;

	m_pList = NULL;
	ec = XMLparse(pStream, OnDomXmlTokenStub, this);
	pList = m_pList;
	return ec;
}

//***********************************************************************
ERRCODE	XMLparser::XMLparse(Bstream* pStream, ON_XMLTOKEN pCallback, LPVOID pCookie)
{
	BtextStream stream;

	ERRCODE ec;
	TCHAR   nc;
	int		i;

	m_parseState = psBase;
	m_tokLen = 0;
	m_lines  = 1;
	m_column = 1;

	pCallback(_T(""), xmlStart, pCookie);

	ec = stream.Open(pStream);

	while(ec == errOK)
	{
		ec = stream.GetChar(nc);
		if(ec != errOK) break;

		m_column++;

		if(nc == _T('\r')) continue;
		if(nc == _T('\n')) {
			m_lines++;
			m_column = 1;
			if(m_parseState == psComment)
				m_parseState = psCommentNL;
			continue;
		}
		/*
		if (m_token)
			m_token[m_tokLen] = 0;
		_tprintf(_T("state=%d t="_Pfs_"=\n"), m_parseState, m_token ? m_token : _T("none"));
		*/
		switch(m_parseState)
		{
		case psBase:
			switch(nc)
			{
			case _T('<'):
				if(m_tokLen > 0)
				{
					m_token[m_tokLen] = _T('\0');
					ec = pCallback(m_token, xmlValue, pCookie);
				}
				m_tokLen = 0;
				ec = AddToToken(nc);
				m_parseState = psElement1;
				break;

			case _T(' '): case _T('\t'):
				break;

			case _T('&'):
				m_parseState = psEntity;
				m_entityStart = m_tokLen;
				break;
				
			default:
				ec = AddToToken(nc);
				m_parseState = psNonWhite;
				break;
			}
			break;

		case psNonWhite:
			switch(nc)
			{
			case _T('<'):
				m_token[m_tokLen] = _T('\0');
				ec = pCallback(m_token, xmlValue, pCookie);
				m_tokLen = 0;
				ec = AddToToken(nc);
				m_parseState = psElement1;
				break;

			case _T('&'):
				m_parseState = psEntity;
				m_entityStart = m_tokLen;
				break;
				
			default:
				ec = AddToToken(nc);
				break;
			}
			break;

		case psEntity:
			switch(nc)
			{
			case _T(';'):
				m_token[m_tokLen] = 0;
				for(i = 0; i < sizeof(g_entitytab)/sizeof(_tag_entities); i++)
				{
					if(! _tcscmp(g_entitytab[i].ename, m_token + m_entityStart))
					{
						m_tokLen = m_entityStart;
						ec = AddToToken(g_entitytab[i].eval);
						break;
					}
				}
				m_parseState = psNonWhite;
				break;

			default:
				ec = AddToToken(nc);
				break;
			}
			break;

		case psElement1:
			switch(nc)
			{
			case _T('!'):
				ec = AddToToken(nc);
				m_parseState = psComment1;
				break;

			case _T('?'):
				ec = AddToToken(nc);
				m_parseState = psVersion;
				break;

			case _T('>'):
				ec = AddToToken(nc);
				m_token[m_tokLen] = _T('\0');
				ec = pCallback(m_token, xmlElement, pCookie);
				m_tokLen = 0;
				m_parseState = psBase;
				break;

			default:
				ec = AddToToken(nc);
				m_parseState = psElement;
				break;
			}
			break;

		case psElement:
			switch(nc)
			{
			case _T('>'):
				ec = AddToToken(nc);
				m_token[m_tokLen] = _T('\0');
				ec = pCallback(m_token, xmlElement, pCookie);
				m_tokLen = 0;
				m_parseState = psBase;
				break;

			case _T('/'):

				m_token[m_tokLen] = _T('\0');
				ec = pCallback(m_token, xmlElement, pCookie);
				m_tokLen = 0;
				m_parseState = psSingularElement;
				break;

			case _T(' '): case _T('\t'):
				m_token[m_tokLen] = _T('\0');
				ec = pCallback(m_token, xmlElement, pCookie);
				m_tokLen = 0;
				m_parseState = psAttribLeft;
				break;

			default:
				ec = AddToToken(nc);
				break;
			}
			break;

		case psSingularElement:
			switch(nc)
			{
			case _T('>'):
				m_token[0] = _T('/');
				m_token[1] = _T('/');
				m_token[2] = _T('\0');
				ec = pCallback(m_token, xmlElement, pCookie);
				m_tokLen = 0;
				m_parseState = psBase;
				break;

			default:
				ec = errXML_BAD_ELEMENT;
				break;
			}
			break;

		case psComment1:
			switch(nc)
			{
			case _T('-'):
				ec = AddToToken(nc);
				m_parseState = psComment2;
				break;

			case _T('>'):
				ec = AddToToken(nc);
				m_token[m_tokLen] = _T('\0');
				ec = pCallback(m_token, xmlElement, pCookie);
				m_tokLen = 0;
				m_parseState = psBase;
				break;

			default:
				ec = AddToToken(nc);
				m_parseState = psElement;
				break;
			}
			break;

		case psComment2:
			switch(nc)
			{
			case _T('-'):
				ec = AddToToken(nc);
				m_parseState = psComment;
				break;

			case _T('>'):
				ec = AddToToken(nc);
				m_token[m_tokLen] = _T('\0');
				ec = pCallback(m_token, xmlElement, pCookie);
				m_tokLen = 0;
				m_parseState = psBase;
				break;

			default:
				ec = AddToToken(nc);
				m_parseState = psElement;
				break;
			}
			break;

		case psComment:
		case psCommentNL:
			switch(nc)
			{
			case _T('>'):
				ec = AddToToken(nc);
				if(m_tokLen >= 4)
				{
					if(m_token[m_tokLen - 2] == _T('-'))
					{
						if(m_token[m_tokLen - 3] == _T('-'))
						{
							m_token[m_tokLen] = _T('\0');
							ec = pCallback(m_token, xmlComment, pCookie);
							m_tokLen = 0;
							m_parseState = psBase;
						}
					}
				}
				break;

			case _T(' '): case _T('\t'):
				if(m_parseState != psCommentNL)
					ec = AddToToken(nc);
				break;

			default:
				if(m_parseState == psCommentNL)
				{
					ec = AddToToken(_T(' '));
					m_parseState = psComment;
				}
				ec = AddToToken(nc);
				break;
			}
			break;

		case psVersion:
			switch(nc)
			{
			case _T('>'):
				ec = AddToToken(nc);
				if(m_tokLen >= 2)
				{
					if(m_token[m_tokLen - 2] == _T('?'))
					{
						m_token[m_tokLen] = _T('\0');
						ec = pCallback(m_token, xmlVersion, pCookie);
						m_tokLen = 0;
						m_parseState = psBase;
					}
					else
					{
						ec = errXML_BAD_VERSION_SPEC;
					}
				}
				else
				{
					ec = errXML_BAD_VERSION_SPEC;
				}
				break;

			default:
				ec = AddToToken(nc);
				break;
			}
			break;

		case psAttribLeft:
			switch(nc)
			{
			case _T('='):
				m_token[m_tokLen] = _T('\0');
				ec = pCallback(m_token, xmlAttributeName, pCookie);
				m_tokLen = 0;
				m_parseState = psAttribRight;
				break;

			case _T('/'):
				m_token[m_tokLen] = _T('\0');
				m_parseState = psSingularElement;
				break;

			case _T(' '): case _T('\t'):
				break;

			case _T('>'):
				//if(m_attribName[0] != _T('\0'))
				//	ec = errXML_NO_ATTRIB_VALUE;
				m_parseState = psBase;
				break;

			default:
				ec = AddToToken(nc);
				break;
			}
			break;

		case psAttribRight:
			switch(nc)
			{
			case _T('\"'):
				m_parseState = psAttribValue;
				break;

			case _T(' '): case _T('\t'):
				break;

			case _T('>'):
				ec = errXML_NO_ATTRIB_VALUE;
				break;

			default:
				ec = errXML_NO_ATTRIB_QUOTE;
				break;
			}
			break;

		case psAttribValue:
			switch(nc)
			{
			case _T('\"'):
				m_token[m_tokLen] = _T('\0');
				ec = pCallback(m_token, xmlAttributeValue, pCookie);
				m_tokLen = 0;
				m_parseState = psAttribLeft;
				break;

			case _T('>'):
				ec = errXML_NO_ATTRIB_QUOTE;
				break;

			default:
				ec = AddToToken(nc);
				break;
			}
			break;
		}
	}

	if(ec == errSTREAM_READ)
		ec = errOK;

	pCallback(_T(""), xmlEnd, pCookie);

	return ec;
}

// STATIC, RECURSIVE
//***********************************************************************
ERRCODE XMLparser::XMLrelist(XMLelement* pList, BtextStream* pStream)
{
	XMLattribute* pAttrib;
	XMLvalue*     pValue;

	static int	  depth = 0;
	int           i;
	TCHAR*        ps;

	if(! pList) return errOK;

	for(i = 0; i < depth * 4; i++)
		pStream->PutChar(_T(' '));

	pStream->PutChar(_T('<'));

	for(ps = (LPTSTR)pList->GetName(); ps && *ps;)
		pStream->PutChar(*ps++);

	if(pList->m_pAttribs)
	{
		for(pAttrib = pList->m_pAttribs; pAttrib; pAttrib = pAttrib->GetNext(pAttrib))
		{
			pStream->PutChar(_T(' '));
			for(ps = pAttrib->m_pName; ps && *ps;)
				pStream->PutChar(*ps++);
			pStream->PutChar(_T('='));
			pStream->PutChar(_T('\"'));
			for(ps = (LPTSTR)pAttrib->GetValue(); ps && *ps;)
				pStream->PutChar(*ps++);
			pStream->PutChar(_T('\"'));
		}
	}
	pStream->PutChar(_T('>'));
	pStream->PutChar(_T('\n'));

	if(pList->m_pValues)
	{
		for(pValue = pList->m_pValues; pValue; pValue = pValue->GetNext(pValue))
		{
			//_tprintf(_T("Add Val n="_Pfs_"= v="_Pfs_"=\n"),
			//		pList->GetName(), pValue->GetValue());
			for(i = 0; i < depth * 4 + 4; i++)
				pStream->PutChar(_T(' '));
			for(ps = pValue->GetValue(); ps && *ps; ps++)
			{
				switch(*ps)
				{
                case 34:
                case 38:
                case 60:
                case 62:
#ifdef UNICODE
                case 338:
                case 339:
                case 352:
                case 353:
                case 376:
                case 710:
                case 732:
                case 8194:
                case 8195:
                case 8201:
                case 8204:
                case 8205:
                case 8206:
                case 8207:
                case 8211:
                case 8212:
                case 8216:
                case 8217:
                case 8218:
                case 8220:
                case 8221:
                case 8222:
                case 8224:
                case 8225:
                case 8240:
                case 8249:
                case 8250:
                case 8364:
#endif
	                for(i = 0; i < sizeof(g_entitytab)/sizeof(_tag_entities); i++)
	                {
	                    if(g_entitytab[i].eval == *ps)
	                    {
	                        break;
	                    }
	                }
                    if(i < sizeof(g_entitytab)/sizeof(_tag_entities))
                    {
                        LPCTSTR pe = g_entitytab[i].ename;
                        
                        pStream->PutChar(_T('&'));
                        while(*pe)
                        {
                            pStream->PutChar(*pe++);
                        }
                        pStream->PutChar(_T(';'));
                    }
                    break;
				default:
					pStream->PutChar(*ps);
					break;
				}
			}
			pStream->PutChar(_T('\n'));
		}
	}
	depth++;
	XMLrelist(pList->GetChild(), pStream);
	depth--;

	for(i = 0; i < depth * 4; i++)
		pStream->PutChar(_T(' '));

	pStream->PutChar(_T('<'));
	pStream->PutChar(_T('/'));
	for(ps = (LPTSTR)pList->GetName(); ps && *ps;)
		pStream->PutChar(*ps++);
	pStream->PutChar(_T('>'));
	pStream->PutChar(_T('\n'));

	return XMLrelist(pList->GetSibling(), pStream);
}


// STATIC
//***********************************************************************
ERRCODE XMLparser::XMLtoStream(XMLelement* pList, Bstream* pStream, TEXTENCODING encoding)
{
	BtextStream textStream;
	ERRCODE    ec;

	if(! pList || ! pStream)
		return errBAD_PARAMETER;

	ec = textStream.Open(pStream, encoding);
	if(ec != errOK)
		return ec;

	return XMLrelist(pList, &textStream);
}
