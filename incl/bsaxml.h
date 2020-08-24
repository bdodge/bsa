//--------------------------------------------------------------------
//
// File: bsaxml.h
// Desc: XML
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------

#ifndef _XML_H_
#define _XML_H_ 1

#ifdef XML_EXPORTS
	#define XML_API __declspec(dllexport)
#elif defined(BSAXML_DLL)
	#define XML_API __declspec(dllimport)
#else
	#define XML_API
#endif

enum XMLtokenType
{
	xmlStart,
	xmlVersion,
	xmlElement,
	xmlValue,
	xmlAttributeName, xmlAttributeValue,
	xmlComment,
	xmlEnd
};


//***********************************************************************
// saxish client vector
typedef ERRCODE (*ON_XMLTOKEN)(LPCTSTR token, XMLtokenType tokType, void* pCookie);
 
//***********************************************************************
// xml parsing tree nodes

// list of name, char value pairs
//
KEY_ARRYVAL_LIST_TEMPLATE(XMLattribute, LPTSTR, XML_API);

// list of name, char value pairs
//
KEY_ARRYVAL_LIST_TEMPLATE(XMLvalue, LPTSTR, XML_API);

//***********************************************************************
class XML_API XMLelement
{
public:
	XMLelement(LPCTSTR pName, XMLelement* pParent = NULL);
	virtual ~XMLelement();

public:
	inline LPCTSTR		GetName				(void)		{ return m_pName;    }
	inline XMLattribute*GetAttributes		(void)		{ return m_pAttribs; }
	inline XMLvalue*	GetValues			(void)		{ return m_pValues; }
	inline XMLelement*	GetParent			(void)		{ return m_pParent;  }
	inline XMLelement*	GetSibling			(void)		{ return m_pSibling; }
	inline XMLelement*	GetChild			(void)		{ return m_pChild;   }
	inline void			SetParent			(XMLelement* pElement)	{ m_pParent  = pElement; }
	inline void			SetSibling			(XMLelement* pElement)	{ m_pSibling = pElement; }
	inline void			SetChild			(XMLelement* pElement)	{ m_pChild   = pElement; }

	virtual XMLelement* FindNextElement		(LPCTSTR pName);
	virtual XMLelement* FindSiblingElement	(LPCTSTR pName);
	virtual XMLelement* FindChildElement	(LPCTSTR pName);

	virtual XMLelement* AddChild			(LPCTSTR pName, XMLelement* pAfter = NULL);
	virtual XMLelement* AddSibling			(LPCTSTR pName, XMLelement* pAfter = NULL);

	virtual ERRCODE		ChangeValue			(XMLvalue* pValue, LPCTSTR pVal);
	virtual ERRCODE		AddValue			(LPCTSTR pVal, XMLvalue* pAfter = NULL);
protected:

public:
	XMLattribute*		m_pAttribs;
	XMLvalue*			m_pValues;

protected:
	LPTSTR				m_pName;

	XMLelement*			m_pParent;
	XMLelement*			m_pSibling;
	XMLelement*			m_pChild;

};

//***********************************************************************
// xml object

class XML_API XMLparser
{
public:
	XMLparser();
	virtual				~XMLparser();

public:				
	virtual ERRCODE		XMLparse			(Bstream* pStream, ON_XMLTOKEN pCallback, void* pCookie);
	virtual ERRCODE		XMLdomParse			(Bstream* pStream, XMLelement*& pResult);
	static  ERRCODE		XMLtoStream			(XMLelement* pTree, Bstream* pStream, TEXTENCODING encoding = txtUTF8);

protected:
	virtual ERRCODE		AddToToken			(TCHAR nc);
	virtual ERRCODE		OnDomXmlToken		(LPCTSTR token, XMLtokenType tokType);

	virtual LPTSTR		FormatToken			(LPCTSTR token, XMLtokenType tokType);

	static  ERRCODE		OnXmlToken			(LPCTSTR token, XMLtokenType tokType, void* pCookie);
	static  ERRCODE		OnDomXmlTokenStub	(LPCTSTR token, XMLtokenType tokType, void* pCookie);
	LPTSTR				DepthString			(void);
	static  ERRCODE		XMLrelist			(XMLelement* pTree, BtextStream* pStream);
public:

protected:
	enum {
			psBase, psEntity, psNonWhite, psVersion,
			psElement1, psElement, psSingularElement,
			psComment1, psComment2, psComment, psCommentNL,
			psAttribLeft, psAttribRight, psAttribValue,
		 }
						m_parseState;

	XMLelement*			m_pList;

	XMLelement*			m_pCurElement;
	XMLattribute*		m_pCurAttrib;

	LPTSTR				m_token;
	int					m_tokLen;
	int					m_entityStart;
	int					m_tokAlloc;
	LPTSTR				m_cleanToken;
	int					m_ctAlloc;
	LPTSTR				m_attribName;
	int					m_anAlloc;
	int					m_lines;
	int					m_column;
	int					m_depth;
};

#endif

