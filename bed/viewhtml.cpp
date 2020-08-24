
#include "bedx.h"

LPCTSTR BviewHTML::m_keywords[] = 
{
	_T("!DOCTYPE"), 	//  Defines the document type 	  	 
	_T("a"),			// 	Defines an anchor 	3.0 	3.0
	_T("abbr"), 		// 	Defines an abbreviation 	6.2 	 
	_T("acronym"),  	// 	Defines an acronym 	6.2 	4.0
	_T("address"),  	// 	Defines an address element 	4.0 	4.0
	_T("applet"),   	// 	Deprecated. Defines an applet 	2.0 	3.0
	_T("area"), 		// 	Defines an area inside an image map 	3.0 	3.0
	_T("b"),			// 	Defines bold text 	3.0 	3.0
	_T("base"), 		// 	Defines a base URL for all the links in a page 	3.0 	3.0
	_T("basefont"), 	// 	Deprecated. Defines a base font 	3.0 	3.0
	_T("bdo"),  		// 	Defines the direction of text display 	6.2 	5.0
	_T("big"),  		// 	Defines big text 	3.0 	3.0
	_T("blockquote"),   // 	Defines a long quotation 	3.0 	3.0
	_T("body"), 		// 	Defines the body element 	3.0 	3.0
	_T("br"),   		// 	Inserts a single line break 	3.0 	3.0
	_T("button"),   	// 	Defines a push button 	6.2 	4.0
	_T("caption"),  	// 	Defines a table caption 	3.0 	3.0
	_T("center"),   	// 	Deprecated. Defines centered text 	3.0 	3.0
	_T("cite"), 		// 	Defines a citation 	3.0 	3.0
	_T("code"), 		// 	Defines computer code text 	3.0 	3.0
	_T("col"),  		// 	Defines attributes for table columns  	  	3.0
	_T("colgroup"), 	// 	Defines groups of table columns 	  	3.0
	_T("dd"),   		// 	Defines a definition description 	3.0 	3.0
	_T("del"),  		// 	Defines deleted text 	6.2 	4.0
	_T("dir"),  		// 	Deprecated. Defines a directory list 	3.0 	3.0
	_T("div"),  		// 	Defines a section in a document 	3.0 	3.0
	_T("dfn"),  		// 	Defines a definition term 	  	3.0
	_T("dl"),   		// 	Defines a definition list 	3.0 	3.0
	_T("dt"),   		// 	Defines a definition term 	3.0 	3.0
	_T("em"),   		// 	Defines emphasized text  	3.0 	3.0
	_T("fieldset"), 	// 	Defines a fieldset 	6.2 	4.0
	_T("font"), 		// 	Deprecated. Defines the font face, size, and color of text 	3.0 	3.0
	_T("form"), 		// 	Defines a form  	3.0 	3.0
	_T("frame"),		// 	Defines a sub window (a frame) 	3.0 	3.0
	_T("frameset"), 	// 	Defines a set of frames 	3.0 	3.0
	_T("h1"),   		//	<h1> to <h6> 	Defines header 1 to header 6 	3.0 	3.0
	_T("h2"),   		//	<h1> to <h6> 	Defines header 1 to header 6 	3.0 	3.0
	_T("h3"),   		//	<h1> to <h6> 	Defines header 1 to header 6 	3.0 	3.0
	_T("h4"),   		//	<h1> to <h6> 	Defines header 1 to header 6 	3.0 	3.0
	_T("h5"),   		//	<h1> to <h6> 	Defines header 1 to header 6 	3.0 	3.0
	_T("h6"),   		//	<h1> to <h6> 	Defines header 1 to header 6 	3.0 	3.0
	_T("head"), 		// 	Defines information about the document 	3.0 	3.0
	_T("hr"),   		// 	Defines a horizontal rule 	3.0 	3.0
	_T("html"), 		// 	Defines an html document 	3.0 	3.0
	_T("i"),			// 	Defines italic text 	3.0 	3.0
	_T("iframe"),   	// 	Defines an inline sub window (frame) 	6.0 	4.0
	_T("img"),  		// 	Defines an image 	3.0 	3.0
	_T("input"),		// 	Defines an input field 	3.0 	3.0
	_T("ins"),  		// 	Defines inserted text 	6.2 	4.0
	_T("isindex"),  	// 	Deprecated. Defines a single-line input field. Use <input> instead 	3.0 	3.0
	_T("kbd"),  		// 	Defines keyboard text 	3.0 	3.0
	_T("label"),		// 	Defines a label for a form control 	6.2 	4.0
	_T("legend"),   	// 	Defines a title in a fieldset 	6.2 	4.0
	_T("li"),   		// 	Defines a list item 	3.0 	3.0
	_T("link"), 		// 	Defines a resource reference 	4.0 	3.0
	_T("map"),  		// 	Defines an image map  	3.0 	3.0
	_T("menu"), 		// 	Deprecated. Defines a menu list 	3.0 	3.0
	_T("meta"), 		// 	Defines meta information 	3.0 	3.0
	_T("noframes"), 	// 	Defines a noframe section 	3.0 	3.0
	_T("noscript"), 	// 	Defines a noscript section 	3.0 	3.0
	_T("object"),   	// 	Defines an embedded object 	  	3.0
	_T("ol"),   		// 	Defines an ordered list 	3.0 	3.0
	_T("optgroup"), 	// 	Defines an option group 	6.0 	6.0
	_T("option"),   	// 	Defines an option in a drop-down list 	3.0 	3.0
	_T("p"),			// 	Defines a paragraph 	3.0 	3.0
	_T("param"),		// 	Defines a parameter for an object 	3.0 	3.0
	_T("pre"),  		// 	Defines preformatted text 	3.0 	3.0
	_T("q"),			// 	Defines a short quotation 	6.2 	 
	_T("s"),			// 	Deprecated. Defines strikethrough text 	3.0 	3.0
	_T("samp"), 		// 	Defines sample computer code 	3.0 	3.0
	_T("script"),   	// 	Defines a script 	3.0 	3.0
	_T("select"),   	// 	Defines a selectable list 	3.0 	3.0
	_T("small"),		// 	Defines small text 	3.0 	3.0
	_T("span"), 		// 	Defines a section in a document 	4.0 	3.0
	_T("strike"),   	// 	Deprecated. Defines strikethrough text 	3.0 	3.0
	_T("strong"),   	// 	Defines strong text 	3.0 	3.0
	_T("style"),		// 	Defines a style definition 	4.0 	3.0
	_T("sub"),  		// 	Defines subscripted text 	3.0 	3.0
	_T("sup"),  		// 	Defines superscripted text 	3.0 	3.0
	_T("table"),		// 	Defines a table 	3.0 	3.0
	_T("tbody"),		// 	Defines a table body 	  	4.0
	_T("td"),   		// 	Defines a table cell 	3.0 	3.0
	_T("textarea"), 	// 	Defines a text area 	3.0 	3.0
	_T("tfoot"),		// 	Defines a table footer 	  	4.0
	_T("th"),   		// 	Defines a table header 	3.0 	3.0
	_T("thead"),		// 	Defines a table header 	  	4.0
	_T("title"),		// 	Defines the document title 	3.0 	3.0
	_T("tr"),   		// 	Defines a table row 	3.0 	3.0
	_T("tt"),   		// 	Defines teletype text 	3.0 	3.0
	_T("u"),			// 	Deprecated. Defines underlined text 	3.0 	3.0
	_T("ul"),   		// 	Defines an unordered list 	3.0 	3.0
	_T("var"),   		// 	Defines a variable 	3.0 	3.0
	NULL
};

//**************************************************************************
BviewHTML::BviewHTML(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel)
		:
		Bview(pBuf, pEditor, pPanel)
{
	AddTags();
}

//**************************************************************************
void BviewHTML::AddTags()
{
	AddComment(_T("<!--"), tsSpanningComment);
	AddComment(_T("-->"), tsBase);

	Bview::AddTags(m_keywords, kwBuiltinType, false);
}

//**************************************************************************
TokenRet BviewHTML::GetToken(
					LPTSTR&		lpText,
					int&		nText,
					int&		line,
					int&		incol,
					int&		outcol,
					TokenState&	state,
					LPTSTR&		lpToken,
					int&		nToken,
					HFONT&		hFont,
					COLORREF&	frgColor,
					COLORREF&	bkgColor
					)
{
	TokenRet ec;
	BkwType kw;
	
	ec = Bview::GetToken(lpText, nText, line, incol, outcol, state,
			lpToken, nToken, kw);
	
	if(ec != trOK)
		return ec;
	
	if(kw == kwPlain || kw == kwOperator || kw == kwQuoted)
	{
		if(state != tsSpanningComment)
		{
			if(state == tsBase && kw == kwQuoted && m_attrib == 2)
			{
				state = tsSpanningElement;
				m_attrib = 1;
			}
			if(state == tsSpanningElement)
			{
				if(lpToken[0] == _T('>'))
				{
					state = tsBase;
					kw = kwMacro;
				}
				else if(lpToken[0] == _T('='))
				{
					m_attrib = 2;
					kw = kwMacro;
				}
			}
			else
			{
				if(lpToken[0] == _T('<'))
				{
					state = tsSpanningElement;
					m_attrib = 0;
					kw = kwMacro;
				}
			}
		}
		if(kw == kwPlain)
		{
			kw = KeyWord(lpToken, nToken);
		}
		if(kw == kwPlain)
		{
			if(state == tsSpanningElement)
			{
				switch(m_attrib)
				{
				default:
				case 0:
					kw = kwBuiltinType;
					m_attrib = 1;
					break;
				case 1:
					kw = kwAddonFunc;
					break;
				case 2:
					kw = kwAddonType;
					m_attrib = 1;
					break;
				}
			}
		}
	}

	frgColor = m_view_colors[kw];
	bkgColor = RGB(255, 255, 255);
	hFont = m_view_fonts[(kw == kwMacro) ? kwBuiltinType : kw];

	return trOK;
}
