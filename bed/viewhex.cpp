
#include "bedx.h"


//**************************************************************************
BviewHex::BviewHex(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel)
		:
		Bview(pBuf, pEditor, pPanel),
		m_nDumpBuf(0),
		m_viewcols(16),
		m_dispaddr(true)
{
}

//**************************************************************************
TokenRet BviewHex::GetToken(
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
	TokenRet	rv;
	BkwType		kw;
	int			i, k;
	int			n;
	int			len;
	DWORD		bc, xc;
	TCHAR		c;

	kw = kwPlain;
	if(incol < 0)		return trFAILURE;
	if(nText <= 0)		return trEOLLINE;
	
	switch (state)
	{
	case tsBase:

		if (incol > nText)	return trEOLLINE;

		state = tsSpanningElement;
		if (m_buffer->GetRaw())
		{
			m_viewcols = m_buffer->GetColumns();
			if (m_viewcols < 1)
				m_viewcols = 1;
			m_address = (line - 1) * m_viewcols;

			m_dispaddr = (m_binformat & 0x10) ? false : true;

			if (m_dispaddr)
			{
				// first column of line, return address
				nToken = _sntprintf(m_dumpBuf, 256, _T("%08X "), m_address);
				outcol += nToken;
				lpToken = m_dumpBuf;
				kw = kwBuiltinFunc;
				rv = trOK;
				incol = 0;
			}
			break;
		}
		incol = 1;
		return GetToken(lpText, nText, line, incol, outcol, state, lpToken, nToken,
			hFont, frgColor, bkgColor);
		break;

	case tsSpanningElement:

		// inside a line of text	
		if (incol == 0)
			incol = 1;
		if (m_buffer->GetRaw())
		{
			n = incol % m_viewcols;

			if (n == 1)
			{
				for (i = 0; i < 255; i++)
					m_dumpBuf[i] = _T(' ');
				m_dumpBuf[i] = _T('\0');
				m_nDumpBuf = (m_viewcols - nText) * 3;
			}
		}
		else
		{
			m_nDumpBuf = 0;
			n = nText;
		}
		c = lpText[incol-1];
		lpToken  = m_tabBuf;

		switch(m_binradix)
		{
		case 2:
			switch(m_binformat & 0xF)
			{
			case 4:
				bc = (incol - 1) & ~0x3;
				xc = (m_binformat & 0x20) ? 3 - bc : bc;
				if((incol + 2) <= nText)
				{
					for(k = 0; k < 8; k++)
						m_tabBuf[k] = (lpText[(incol - 1) + xc] & (0x80 >> k)) ? _T('1') : _T('0');
					m_tabBuf[k++] = _T(' ');
					m_tabBuf[k]   = _T('\0');
					if(((incol-1) & 0x3) == 3)
					{
						len = 9;
						lpToken[8] = _T(' ');
					}
					else
					{
						len = 8;
					}
					incol++;
					break;
				}
				// else FALL THROUGH
			case 2:
				bc = (incol - 1) & ~0x1;
				xc = (m_binformat & 0x20) ? 1 - bc : bc;
				if((incol + 1) <= nText)
				{
					for(k = 0; k < 8; k++)
						m_tabBuf[k] = (lpText[(incol - 1) + xc] & (0x80 >> k)) ? _T('1') : _T('0');
					m_tabBuf[k++] = _T(' ');
					m_tabBuf[k]   = _T('\0');
					if(((incol-1) & 0x1) == 1)
					{
						len = 9;
						lpToken[8] = _T(' ');
					}
					else
					{
						len = 8;
					}
					incol++;
					break;
				}
				// else FALL THROUGH
			case 1:
				for(k = 0; k < 8; k++)
					m_tabBuf[k] = (c & (0x80 >> k)) ? _T('1') : _T('0');
				m_tabBuf[k++] = _T(' ');
				m_tabBuf[k]   = _T('\0');
				len = k;
				incol++;
				break;
			}
			break;
			break;
		case 8:
			len = _sntprintf(m_tabBuf, 32, _T("%03o "), (unsigned)c);
			incol+= 1;
			break;
		case 10:
			len = _sntprintf(m_tabBuf, 32, _T("%03ud "), (unsigned)c);
			incol+= 1;
			break;
		default:
		case 16:
			switch(m_binformat & 0xF)
			{
			case 4:
				bc = (incol - 1) & ~0x3;
				if((incol + 2) <= nText)
				{
					if(m_binformat & 0x20)
					{
						_sntprintf(m_tabBuf, 32, _T("%08X "), 
							((DWORD)lpText[bc+3] << 24) | (((DWORD)lpText[bc+2]) << 16) |
							(((DWORD)lpText[bc+1]) << 8) | (((DWORD)lpText[bc]))); 
					}
					else
					{
						_sntprintf(m_tabBuf, 32, _T("%08X "), 
							((DWORD)lpText[bc] << 24) | (((DWORD)lpText[bc+1]) << 16) |
							(((DWORD)lpText[bc+2]) << 8) | (((DWORD)lpText[bc+3]))); 
					}
					lpToken += 2 * ((incol-1) & 0x3);
					if(((incol-1) & 0x3) == 3)
					{
						len = 3;
						lpToken[2] = _T(' ');
					}
					else
					{
						len = 2;
					}
					incol++;
					break;
				}
				// else FALL THROUGH
			case 2:
				bc = (incol - 1) & ~0x1;
				if((incol + 1) <= nText)
				{
					if(m_binformat & 0x20)
					{
						_sntprintf(m_tabBuf, 32, _T("%04X "), (((WORD)lpText[bc+1] << 8) | (WORD)lpText[bc]));
					}
					else
					{
						_sntprintf(m_tabBuf, 32, _T("%04X "), (((WORD)lpText[bc] << 8) | (WORD)lpText[bc+1]));
					}
					lpToken += 2 * ((incol-1) & 0x1);
					len = (((incol-1) & 0x1) == 3) ? 3 : 2;
					if(((incol-1) & 0x1) == 1)
					{
						len = 3;
						lpToken[2] = _T(' ');
					}
					else
					{
						len = 2;
					}
					incol++;
					break;
				}
				// else FALL THROUGH
			case 1:
				len = _sntprintf(m_tabBuf, 32, _T("%02X "), (BYTE)c);
				incol++;
				break;
			}
			break;
		}
		if (m_buffer->GetRaw())
		{
			if (c < 127 && c > 19)
				m_dumpBuf[m_nDumpBuf++] = c;
			else
				m_dumpBuf[m_nDumpBuf++] = _T('.');
		}
		nToken  = len;
		outcol += len;

		if(incol > nText || n == 0)
			state = tsPostText;
		rv = trOK;
		break;

	case tsPostText:

		// text version of line
		if (m_buffer->GetRaw())
		{
			nToken = m_nDumpBuf;
			lpToken = m_dumpBuf;
		}
		else
		{
			if(nText > 0)
				if(lpText[nText - 1] == _T('\n'))
					nText--;
			if (nText == 0)
			{
				m_dumpBuf[0] = _T(' ');
				m_dumpBuf[1] = _T('\0');
				lpText = m_dumpBuf;
				nText = 1;
			}
			nToken = nText;
			lpToken = lpText;
		}
		outcol += nToken;
		incol   = 0;
		state   = tsComment;
		kw		= kwMacro;
		rv		= trOK;
		break;
		
	case tsComment:

		// after text is returned
		state	= tsBase;
		rv		= trEOLLINE;
		break;
	}
	frgColor = m_view_colors[kw];
	bkgColor = m_view_colors[kwBackground];
	hFont    = m_view_fonts[kw];

	return rv;
}

//**************************************************************************
void BviewHex::SpecificMouseMenu(HMENU hMenu)
{
	AppendMenu(hMenu, 0, ID_VIEW_VIEWRADIX, _T("View &Radix..."));
	AppendMenu(hMenu, 0, ID_VIEW_VIEWFORMAT, _T("&View Format..."));
	AppendMenu(hMenu, 0, ID_VIEW_VIEWCOLUMNS, _T("B&ytes Per Line..."));
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
}

