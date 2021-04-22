
#include "bedx.h"

int		Bbuffer::m_refs = 0;
int		Bbuffer::m_nextuntitled = 0;
TCHAR	Bbuffer::m_untitledName[MAX_PATH];
TCHAR*	Bbuffer::m_sandbox = NULL;
int		Bbuffer::m_sandalloc = 0;
TCHAR*	Bbuffer::m_scratch;
int		Bbuffer::m_scratchalloc = 0;
int		Bbuffer::m_sandlen = 0;

#if defined(Windows)&&!defined(__GNUC__)
#include <crtdbg.h>
#endif

//**************************************************************************
Bbuffer::Bbuffer(
						LPCTSTR name,
						BufType type,
						TEXTENCODING encoding,
						TEXTENCODING defaultencoding,
						Bed* editor
				)
		:
		m_buf(NULL),
		m_editor(editor),
		m_type(type),
		m_undos(NULL),
		m_redos(NULL),
		m_encoding(encoding),
		m_defaultencoding(defaultencoding),
		m_readonly(false),
		m_suppressundo(false),
		m_noundo(false),
		m_lines(NULL),
		m_linecnt(0),
		m_filesize(0),
		m_columns(16),
		m_modtime(0),
		m_modified(false),
		m_read(false),
		m_curline(1),
		m_curcol(1),
		m_pcurline(NULL),
		m_hasCR(false),
		m_careformods(true),
		m_new(true),
		m_untitled(false),
		m_changedlines(false)
{
	m_refs++;

	TCHAR uname[MAX_PATH];

	if(! name || ! name[0])
	{
		// no name supplied, pull an "untitled" type name out of our hat
		//
		GetUntitledName(type, NULL, uname, MAX_PATH);
		name = uname;
		m_untitled = true;
	}
	m_name		  = new TCHAR [ _tcslen(name) + 1 ];
	_tcscpy(m_name, name);
	_tcsncpy(m_localName, name, MAX_PATH-2);
}

//**************************************************************************
Bbuffer::~Bbuffer()
{
	if(m_buf)
	{
		delete [] m_buf;
	}
	if(m_undos)
	{
		LPUNDOINFO pu;

		for(pu = m_undos; pu;)
		{
			m_undos = pu->m_next;
			delete pu;
			pu = m_undos;
		}
	}
	if(m_redos)
	{
		LPUNDOINFO pu;

		for(pu = m_redos; pu;)
		{
			m_redos = pu->m_next;
			delete pu;
			pu = m_redos;
		}
	}
	if(m_name)
	{
		delete [] m_name;
	}
	if(m_lines)
	{
		Bline* pLine, *pCurLine;

		for(pLine = pCurLine = m_lines; pLine; pLine = pCurLine)
		{
			pCurLine = pLine->m_next;
			delete pLine;
		}
	}
	if(--m_refs == 0)
	{
		if(m_sandbox)
		{
			delete [] m_sandbox;
			m_sandbox = NULL;
			m_sandalloc = 0;
		}
		if(m_scratch)
		{
			delete [] m_scratch;
			m_scratch = NULL;
			m_scratchalloc = 0;
		}
	}
}

//**************************************************************************
BufType Bbuffer::SniffType(LPCTSTR pFilename, TEXTENCODING& encoding)
{
	LPTSTR pExt;

	for(pExt = (LPTSTR)pFilename + _tcslen(pFilename) - 1; pExt && pExt >= pFilename && *pExt && *pExt != _T('.');)
		pExt--;

	if(*pExt == _T('.'))
		pExt++;

	switch(*pExt)
	{
	case 'a': case 'A':
		if(! _tcsicmp(pExt, _T("asm")))	return btASM;
		break;

	case 'c': case 'C':
		if(! _tcsicmp(pExt, _T("c")))	return btC;
		if(! _tcsicmp(pExt, _T("cc")))	return btCPP;
		if(! _tcsicmp(pExt, _T("cpp")))	return btCPP;
		if(! _tcsicmp(pExt, _T("com")))	return btTerm;
		if(! _tcsicmp(pExt, _T("cs")))	return btCS;
		if(! _tcsicmp(pExt, _T("css")))	return btHTML;
		break;

	case 'f': case 'F':
		if(! _tcsicmp(pExt, _T("fs")))	return btASM;
		if(! _tcsicmp(pExt, _T("finc")))return btASM;
		break;

	case 'h': case 'H':
		if(! _tcsicmp(pExt, _T("h")))	return btCPP;
		if(! _tcsicmp(pExt, _T("hpp")))	return btCPP;
		if(! _tcsicmp(pExt, _T("htm")))	return btHTML;
		if(! _tcsicmp(pExt, _T("html")))return btHTML;
		break;

	case 'i': case 'I':
		if(! _tcsicmp(pExt, _T("inc")))	return btASM;
		break;

	case 'j': case 'J':
		if(! _tcsicmp(pExt, _T("java")))return btJava;
		if(! _tcsicmp(pExt, _T("jav")))	return btJava;
		if(! _tcsicmp(pExt, _T("js")))	return btJavaScript;
		if(! _tcsicmp(pExt, _T("j")))	return btJava;
		break;

	case 'm': case 'M':
		if(! _tcsicmp(pExt, _T("m")))	return btOBJC;
		break;

	case 'p': case 'P':
		if(! _tcsicmp(pExt, _T("php")))	return btPHP;
		if(! _tcsicmp(pExt, _T("py")))	return btPython;
		break;

	case 'q': case 'Q':
		if(! _tcsicmp(pExt, _T("qs")))	return btASM;
		if(! _tcsicmp(pExt, _T("qc")))	return btC;
		if(! _tcsicmp(pExt, _T("qinc")))return btASM;
		break;

	case 's': case 'S':
		if(! _tcsicmp(pExt, _T("s")))	return btASM;
		if(! _tcsicmp(pExt, _T("shtm")))return btHTML;
		if(! _tcsicmp(pExt, _T("shtml")))return btHTML;
		if(! _tcsicmp(pExt, _T("swift")))return btSwift;
		break;

	case 't': case 'T':
		if(! _tcsicmp(pExt, _T("tcl")))	return btTCL;
		if(! _tcsicmp(pExt, _T("tk")))	return btTCL;
		break;

	case 'v': case 'V':
		if(! _tcsicmp(pExt, _T("v")))	return btVerilog;
		break;

	case 'x': case 'X':
		if(! _tcsicmp(pExt, _T("xml")))	return btXML;

	case 0:
	default:
		break;
	}
	// look at the first 256 bytes to determine if its
	// best viewed in binary format or not...
	//
	{
		BfileStream fs;
		BtextStream ts;
		ERRCODE		ec;
		int 		i, j, k, u;

		TCHAR		 sniffbuf[256];

		ec = fs.Open(pFilename, _T("r"));

		if(ec != errOK)
			return btAny;

		ec = ts.Open(&fs);
		if(ec != errOK)
			return btAny;

#ifdef UNICODE
		ts.GetEncoding(encoding);
		if(encoding != (TEXTENCODING)-1 && encoding != txtANSI)
			return btText;
#endif

		for(i = 0; (ec == errOK) && (i < 256);)
			if((ec = ts.GetChar(sniffbuf[i])) == errOK)
				i++;

		ts.Close();
		fs.Close();

		if(i < 1) return btAny;

		// check for unicode encodings without byte-order-marks
		// in a few common formats.  These tests only work for
		// English and other SBCS encodings, but other languages
		// are found typically with BOM in the files anyway...
		//

		// test for UCS2LE
		for(j = 0; j < i; j+=2)
		{
			if(!sniffbuf[j] || sniffbuf[j+1])
				break;
		}
		if(j >= i)
		{
			encoding = txtUCS2SWAP;
			return btText;
		}
		// test for UCS2BE
		for(j = 0; j < i; j+=2)
		{
			if(sniffbuf[j] || !sniffbuf[j+1])
				break;
		}
		if(j >= i)
		{
			encoding = txtUCS2;
			return btText;
		}
		// test for UCS4LE
		for(j = 0; j < i-4; j+=4)
		{
			if(!sniffbuf[j] || sniffbuf[j+1] || sniffbuf[j+2] || sniffbuf[j+3])
				break;
		}
		if(j >= i)
		{
			encoding = txtUCS4SWAP;
			return btText;
		}

		// test for UCS4BE
		for(j = 0; j < i-4; j+=4)
		{
			if(sniffbuf[j] || sniffbuf[j+1] || sniffbuf[j+2] || !sniffbuf[j+3])
				break;
		}
		if(j >= i)
		{
			encoding = txtUCS4;
			return btText;
		}
		// not UNICODE, so see how raw it is
		//
		for(j = 0; j < i; j++)
		{
			k = sniffbuf[j];
			if(k < 32 && k != 10 && k != 13 && k != 9)
				return btRaw;
		}
		// check for upper bits set in some bytes and if so, assime encoding is utf8
		//
		for(j = u = 0; j < i; j++)
		{
			k = sniffbuf[j];
			if(k & 0x80) u++;
		}
		if(u > 3)
		{
			encoding = txtUTF8;
		}
	}
	return btAny;
}

#define BED_TAB_SNIFF_CNT 64

//**************************************************************************
ERRCODE Bbuffer::SniffTabs(int& tabspace, int& indentspace, bool& spacetabs)
{
	ERRCODE ec;
	LPCTSTR text;
	int     ntext;
	int svl, svc;
	int line, col, lines, maxl;
	int fl;		// lines with tab count
	int sc, tc;	// space count, tab count
	int prevsc; //
	int sd[9];	// space distribution (histogram)

	svl = m_curline;
	svc = m_curcol;

	tabspace = 4;
	indentspace = 4;
	spacetabs = false;

	// locate the first N tabs starting at the middle of the file
	// (past any comment headers which can mess us up for example)
	// and see how many spaces are after each tab.  For 4 spaces, we
	// assume tabspace = 8, indent = 4, for 2 spaces, assume tab = 4
	// and indent = 2
	//
	lines = GetLineCount(maxl);
	line = lines / 2;

	if(line < 4) return errUNDERFLOW; // not enough text to make a decision

	//line  = 1;

	for(fl = 0; fl < 9; fl++)
		sd[fl] = 0;

	for(fl = tc = 0; fl < BED_TAB_SNIFF_CNT && line < lines; fl++)
	{
		col = 1;

		// locate the first tab in the line, more likely the "indent"
		//
		if(Locate(line, col, _T("\t"), 1, false, false) != errOK)
		{
			// no tabs left in file
			break;
		}
		// found a tab
		//
		tc++;

		// count any space chars after the last tab char
		//
		ec = GetLineText(line, text, ntext);
		if(ec != errOK)
			break;
		text += (col - 1);

		while(*text == _T('\t'))
			text++;

		for(sc = 0; *text == _T(' '); text++)
		{
			sc++;
		}
		if(sc <= 8)
		{
			// add space count to histogram
			//
			sd[sc]++;
		}
		line++;
	}
	if(tc == 0)
	{
		// found no tabs, so doing tabs-as-spaces
		//
		spacetabs = true;

		//  set indent based on line indent deltas only
		//
		line = 1;
		col = 1;

		for(fl = prevsc = 0; fl < BED_TAB_SNIFF_CNT && line < lines; fl++)
		{
			// count any space chars on left side of line
			//
			ec = GetLineText(line, text, ntext);
			if(ec != errOK)
				break;

			for(sc = 0; *text == _T(' '); text++)
			{
				sc++;
			}
			// if different than last space put delta into histogram
			//
			if(sc != prevsc)
			{
				int delta = sc - prevsc;

				if (delta < 0)
					delta = -delta;

				if (delta < 9)
				{
					// add space count to histogram
					//
					sd[delta]++;
				}
				prevsc = sc;
			}
			line++;
		}
		// find most probable space delta and use that for indent and tab
		//
		for(fl = sc = maxl = 0; fl < 9; fl++)
		{
			if(sd[fl] > sc)
			{
				maxl = fl;
				sc = sd[fl];
			}
		}
		switch(maxl)
		{
		case 0:  // no deltas, but there are lines, so at least set tabs-as-spaces
			return errOK;
		case 1:
			tabspace		= 1;
			indentspace		= 1;
			break;
		case 2:
			tabspace		= 2;
			indentspace		= 2;
			break;
		case 3:  // 3 spaces delta prolly 3-3
			tabspace		= 3;
			indentspace		= 3;
			break;
		case 4:  // 4 space delta
			tabspace		= 4;
			indentspace		= 4;
			break;
		case 5:
		case 6:  // mostly have no clue
		case 7:
			return errFAILURE;
		case 8: // mostly 8 space deltas
			tabspace		= 8;
			indentspace		= 8;
			break;
		}
	}
	else
	{
		// check distribution of spaces-after-tabs counts
		//
		for(fl = sc = maxl = 0; fl < 9; fl++)
		{
			if(sd[fl] > sc)
			{
				maxl = fl;
				sc = sd[fl];
			}
		}
		switch(maxl)
		{
		case 0: // no spaces after any tabs, assume regular tab-4/ind-4 or 8/8
			// Linux kernel sources are 8/8 so if there is a good indication this
			// is a linux source use 8/8
			//
			line = 1;
			col = 1;
			tabspace = 4;
			indentspace = 4;

			if(Locate(line, col, _T("\"linux/"), 7, false, false) == errOK)
			{
				if (line < 40)
				{
					tabspace		= 8;
					indentspace		= 8;
				}
			}
			break;
		case 1:  // nonsense
			return errFAILURE;
		case 2:
			tabspace		= 4;
			indentspace		= 2;
			break;
		case 3:  // 3 spaces? prolly 3-3
			tabspace		= 3;
			indentspace		= 3;
			break;
		case 4:  // 4 spaces after a tab, assume 8-4
			tabspace		= 8;
			indentspace		= 4;
			break;
		case 5:
		case 6:  // mostly have no clue
		case 7:
			return errFAILURE;
		case 8: // mostly 8 spaces after tabs, strange
			tabspace		= 8;
			indentspace		= 4;
			break;
		}
	}
	return errOK;
}

//**************************************************************************
void Bbuffer::SetName(LPCTSTR pName)
{
	if(! pName) return;
	if(m_name)
		delete [] m_name;
	m_name = new TCHAR [ _tcslen(pName) + 2 ];
	_tcscpy(m_name, pName);
}

//**************************************************************************
LPCTSTR Bbuffer::GetShortName(void)
{
	LPCTSTR sn = m_name;

	if(! sn)
		return _T("<nil>");
	sn += _tcslen(sn);
	while(sn > m_name)
	{
		if(*--sn == _PTC_)
			return sn + 1;
	}
	return m_name;
}

//**************************************************************************
ERRCODE Bbuffer::GetLineText(int line, LPCTSTR& lpText, int& nText)
{
	ERRCODE ec;

	if(line < 1) line = 1;

	if((ec = MoveTo(line)) != errOK)
	{
		lpText = NULL;
		nText = 0;
		return ec;
	}
	if(! m_pcurline)
	{
		lpText = NULL;
		nText = 0;
		return errFAILURE;
	}
	ec = m_pcurline->GetText(lpText, nText);

	return ec;
}

//**************************************************************************
int Bbuffer::GetLineCount(int& maxlen)
{
	Bline*  pLine;
	int     count;

	maxlen = 0;

	for(count = 0, pLine = m_lines; pLine; pLine = pLine->m_next)
	{
		if(pLine->m_len > maxlen)
			maxlen = pLine->m_len;
		count++;
	}
	m_linecnt = count;
	return count;
}

//**************************************************************************
int Bbuffer::GetLineCountToEndFromLine(int startline)
{
	Bline*  pLine;
	int     count;

	pLine = m_pcurline;
	if (! pLine)
		return 0;

	if (startline <= m_curline)
	{
		count = m_curline - startline;
	}
	else
	{
		int curline = m_curline;

		count = 0;

		while(curline < startline)
		{
			if(pLine->m_next)
			{
				curline++;
				pLine = pLine->m_next;
			}
			else
			{
				break;
			}
		}
	}
	while (pLine->m_next)
	{
		pLine = pLine->m_next;
		count++;
	}
	return count;
}

//**************************************************************************
ERRCODE Bbuffer::MoveTo(int line)
{
	ERRCODE ec;

	if(line < 1) line = 1;

	if(! m_read)
		if((ec = Read()) != errOK)
			return ec;

	if(! m_pcurline)
	{
		m_pcurline = m_lines;
		m_curline = 1;
		m_curcol  = 1;
	}
	if(! m_pcurline)
	{
		return errUNDERFLOW;
	}

	if(line > m_curline)
	{
		while(line > m_curline)
		{
			if(m_pcurline->m_next)
			{
				m_pcurline = m_pcurline->m_next;
				m_curline++;
			}
			else
			{
				return errOVERFLOW;
			}
		}
	}
	else if(line < m_curline)
	{
		while(line < m_curline)
		{
			if(m_pcurline->m_prev)
			{
				m_pcurline = m_pcurline->m_prev;
				m_curline--;
			}
			else
			{
				return errUNDERFLOW;
			}
		}
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bbuffer::CheckSandboxSize(int size)
{
	if(size > MAX_LINE_LEN)
		return errFAILURE;

	if(size > m_sandalloc || ! m_sandbox)
	{
		LPTSTR m_prevbox   = m_sandbox;
		int    m_prevalloc = m_sandalloc;

		m_sandalloc = size * 2;
		if(m_sandalloc < 512)
			m_sandalloc = 512;
		if(m_sandalloc > MAX_LINE_LEN)
			m_sandalloc = MAX_LINE_LEN;
		m_prevbox = m_sandbox;
		m_sandbox = new TCHAR [ m_sandalloc ];
		if(! m_sandbox)
		{
			m_sandbox = m_prevbox;
			LineToLong(_T("Realloc for Edit"));
			return errFAILURE;
		}
		if(m_prevbox && m_prevalloc)
		{
			memcpy(m_sandbox, m_prevbox, sizeof(TCHAR)*min(m_prevalloc, m_sandalloc));
			delete [] m_prevbox;
		}
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bbuffer::EditLine(int& line, int& col)
{
	LPCTSTR lpText;
	int		nText;
	ERRCODE ec;

	if(! m_read)
		if((ec = Read()) != errOK)
			return ec;

	if((ec = GetLineText(line, lpText, nText)) != errOK)
	{
		if(line == 1 && ec == errUNDERFLOW && ! m_lines)
		{
			// empty buffer, that's OK
			;
		}
		else
		{
			return ec;
		}
	}
	if(nText > MAX_LINE_LEN)
	{
		nText = MAX_LINE_LEN - 2;
		LineToLong(_T("Open for Edit"));
	}
	CheckSandboxSize(nText + 2);

	// copy text to sand box and set curcol
	//
	m_sandlen = nText;

	if(nText > 0)
	{
		memcpy(m_sandbox, lpText, nText * sizeof(TCHAR));
	}
	m_sandbox[nText] = _T('\0'); // helps debug

	if(col > nText)
		col = nText;
	if(col <= 0)
		col = 1;
	line = m_curline;

	return errOK;
}

//**************************************************************************
ERRCODE Bbuffer::CommitLine()
{
	bool wasmod = m_modified;

	if(! m_pcurline)
	{
		if(!m_lines)
		{
			m_lines = m_pcurline = new Bline(NULL, 0);
			m_lines->m_prev = m_lines->m_next = NULL;
		}
		else
			return errFAILURE;
	}
	if(m_careformods)
		m_modified = true;

	if(m_sandlen > 0)
	{
		if(m_sandlen >= MAX_LINE_LEN)
		{
			LineToLong(_T("Commit from Edit"));
			m_sandlen = MAX_LINE_LEN - 2;
		}
		if((! m_pcurline->m_alen) || (m_pcurline->m_alen < m_sandlen))
		{
			// new line won't fit in allocated area, realloc
			if(m_pcurline->m_text)
			{
				if(m_pcurline->m_alen)
				{
					delete [] m_pcurline->m_text;
					m_pcurline->m_alen = 0;
				}
				m_pcurline->m_text = NULL;
			}
		}
		if(! m_pcurline->m_text)
		{
			if(m_sandlen <= 127)
				m_pcurline->m_alen = 127;
			else
				m_pcurline->m_alen = m_sandlen * 2;
			m_pcurline->m_text = new TCHAR [ m_pcurline->m_alen + 1];
		}
		if(! m_pcurline->m_text)
			return errNO_MEMORY;
		m_pcurline->m_len  = m_sandlen;
		memcpy(m_pcurline->m_text, m_sandbox, m_sandlen * sizeof(TCHAR));
		m_pcurline->m_text[m_sandlen] = 0; // this helps debug only
	}
	else
	{
		m_pcurline->m_len  = m_sandlen;
		if(m_pcurline->m_text)
			m_pcurline->m_text[m_sandlen] = 0; // this helps debug only
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bbuffer::SetScratch(int size)
{
	if(! m_scratch || (size >= m_scratchalloc || m_scratchalloc == 0))
	{
		size *= 2;
		if(size < 4096)
			size = 4096;
		if(m_scratch)
			delete [] m_scratch;
		m_scratch = new TCHAR [ size ];
		if(! m_scratch)
			return errNO_MEMORY;
		m_scratchalloc = size;
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bbuffer::Insert(int& line, int& col, LPCTSTR lpText, int nText)
{
	ERRCODE ec;
	LPCTSTR lpBaseText;
	Bline*  pLine;

	if(! lpText || nText <= 0)
		return errBAD_PARAMETER;
	if(m_readonly)
		return errPERMISSION;

	lpBaseText = lpText;

	// make sure there's a line to insert into
	//
	if(! m_lines)
	{
		if(! m_buf)
			m_buf = new TCHAR [ 4 ];
		m_buf[0] = _T('\n');
#ifdef Windows
		m_hasCR = true;
#endif
		m_lines = m_pcurline = new Bline(m_buf, 1);
	}

	int	   sl = line;
	int	   sc = col;

	// move line at line num to sandbox, this updates
	// line num and col too, so they are valid no matter what here
	//
	if((ec = EditLine(line, col)) != errOK)
		return ec;

	LPTSTR pi = m_sandbox + col - 1;
	LPTSTR pn = pi;
	LPTSTR px;
	int    nExtra = m_sandlen - col + 1;
	int    i;
	int	   nAdded = 0;


	// move extant text into scratch area
	SetScratch(nExtra);
	for(px = m_scratch, i = 0; i < nExtra; i++)
		*px++ = *pn++;

	// set sandbox length to temporary truncated len
	m_sandlen = col - 1;

	// insert text byte at a time
	while(nText > 0)
	{
		i = pi - m_sandbox;
		CheckSandboxSize(m_sandlen + 6);
		pi = m_sandbox + i;

		*pi++ = *lpText;
		nText--;
		m_sandlen++;
		nAdded++;

		if(m_sandlen >= MAX_LINE_LEN)
		{
			LineToLong(_T("Insert"));
			break;
		}
		if(m_type != btRaw && *lpText == _T('\n'))
		{
			// commit the current line
			CommitLine();

			// create a new line and link it in
			pLine = new Bline(NULL, 0);
			pLine->m_prev = m_pcurline;
			if(m_pcurline)
			{
				pLine->m_next = m_pcurline->m_next;
				if(pLine->m_next)
					pLine->m_next->m_prev = pLine;
				m_pcurline->m_next = pLine;
			}
			else
			{
				m_pcurline = m_lines = pLine;
				pLine->m_next = NULL;
			}
			MoveTo(++line);
			col = 1;
			m_changedlines = true;

			pi = m_sandbox + col - 1;
			m_sandlen = 0;
		}
		lpText++;
	}
	if(nExtra + m_sandlen >= MAX_LINE_LEN)
	{
		LineToLong(_T("Insert"));
		nExtra = 0;
	}
	// move scratch text back to line
	i = pi - m_sandbox;
	CheckSandboxSize(m_sandlen + nExtra);
	pi = m_sandbox + i;
	for(px = pi, pn = m_scratch, i = 0; i < nExtra; i++)
		*px++ = *pn++;
	m_sandlen += nExtra;

	// update column
	col = pi - m_sandbox + 1;

	// commit
	CommitLine();

	// add undo info
	//
	ec = AddUndoInfo(utInsert, sl, sc, line, col, lpBaseText, nAdded);

	return errOK;
}

//**************************************************************************
ERRCODE Bbuffer::Delete(int& line, int& col, int nText)
{
	ERRCODE ec;
	LPTSTR  lpRemoved;
	int		nRemoved;
	int		nTotal;

	if(nText <= 0)
		return errBAD_PARAMETER;
	if(m_readonly)
		return errPERMISSION;

	nTotal = nText;
	lpRemoved = new TCHAR [ nTotal + 2  ];
	nRemoved  = 0;

	int		sl = line;
	int		sc = col;

	if((ec = EditLine(line, col)) != errOK)
		return ec;

	if(col < sc)
		sc = col;

	while(nText > 0 && m_lines)
	{
		LPTSTR pi	  = m_sandbox + col - 1;
		LPTSTR px	  = pi + nText;
		LPTSTR pn;

		int	   nExtra = m_sandlen - col + 1 - nText;

		if(nExtra > 0)
		{
			// some left on line, just copy it on top of deleted text
			// saving lost chars to the undo buffer
			//
			m_sandlen -= nText;
			nText -= nExtra;
			while(nExtra-- > 0)
			{
				if(nRemoved < nTotal)
					lpRemoved[nRemoved++] = *pi;
				*pi++ = *px++;
			}
			// move the rest of the deleted bytes into the undo buffer
			//
			while(nText-- > 0)
			{
				if(nRemoved < nTotal)
					lpRemoved[nRemoved++] = *pi++;
			}
		}
		else if(m_lines)
		{
			Bline*	pLine;
			int		nExtant = col - 1;
			int		nDelete = m_sandlen - nExtant;
			int		i;

			// nothing left, delete to EOL, then join successive lines
			// until no more lines or no more bytes to delete
			//
			nText    -= nDelete;
			m_sandlen = nExtant;

			// copy the deleted (right) contents to the undo buffer
			//
			for(i = 0, pn = m_sandbox + nExtant; i < nDelete && nRemoved < nTotal; i++)
				lpRemoved[nRemoved++] = *pn++;

			// if there is a next line, delete this one and insert the extant
			// text into the beginning of next line, else can't go on
			//
			if(m_pcurline->m_next)
			{
				// copy the extant (left) contents of this line to scratch buffer
				//
				SetScratch(nExtant);
				for(i = 0, px = m_scratch, pn = m_sandbox; i < nExtant; i++)
					*px++ = *pn++;
				*px = _T('\0'); // helps for debug

				// delete this line (line num stays the same)
				//
				pLine = m_pcurline;
				if(m_pcurline == m_lines && m_lines)
					m_lines = m_lines->m_next;
				if(m_pcurline)
					m_pcurline = m_pcurline->m_next;

				if(pLine->m_prev) pLine->m_prev->m_next = pLine->m_next;
				if(pLine->m_next) pLine->m_next->m_prev = pLine->m_prev;

				if(pLine)
					delete pLine;

				col = 1;
				m_changedlines = true;

				ec = EditLine(line, col);

				ec = CheckSandboxSize(m_sandlen + nExtant + 2);

				if(m_sandlen + nExtant >= MAX_LINE_LEN)
				{
					LineToLong(_T("Delete"));
					nExtant = 0;
				}

				// move extant text over to make room for scratch
				//
				if(ec == errOK && m_sandlen > 0)
				{
					px = m_sandbox + m_sandlen + nExtant - 1;
					pn = m_sandbox + m_sandlen - 1;

					for(i = 0; i < m_sandlen; i++)
						*px-- = *pn--;
				}
				// insert old contents
				px = m_sandbox;
				pn = m_scratch;

				for(i = 0; i < nExtant; i++)
					*px++ = *pn++;

				m_sandlen += nExtant;
				CommitLine();

				col = nExtant + 1;
				if(col > m_pcurline->m_len)
					col = m_pcurline->m_len;
				if(nText > 0)
				{
					if((ec = EditLine(line, col)) != errOK)
						return ec;
				}
			}
			else
			{
				m_sandlen = nExtant;
				CommitLine();
				col = nExtant;
				break;
			}
		}
	}
	CommitLine();

	// add undo info
	ec = AddUndoInfo(utDelete, sl, sc, line, col, lpRemoved, nRemoved);
	delete [] lpRemoved;
	return errOK;
}

//**************************************************************************
ERRCODE Bbuffer::ReplaceLine(int line, LPCTSTR lpText, int nText)
{
	ERRCODE ec;
	int		col;

	if(! lpText || nText <= 0)
		return errBAD_PARAMETER;
	if(m_readonly)
		return errPERMISSION;

	// make sure there's a line to insert into
	//
	if(! m_lines)
	{
		if(! m_buf)
			m_buf = new TCHAR [ 4 ];
		m_buf[0] = _T('\n');
#ifdef Windows
		m_hasCR = true;
#endif
		m_lines = m_pcurline = new Bline(m_buf, 1);
	}
	// move line at line num to sandbox, this updates
	// line num and col too, so they are valid no matter what here
	//
	col = 1;
	if((ec = EditLine(line, col)) != errOK)
		return ec;

	// add deletion undo info
	//
	if(m_sandlen > 0)
		ec = AddUndoInfo(utDelete, line, 1, line, 1, m_sandbox, m_sandlen);

	// overwrite the line
	//
	ec = CheckSandboxSize(nText + 2);
	if(ec == errOK)
	{
		memcpy(m_sandbox, lpText, nText*sizeof(TCHAR));
		m_sandlen = nText;
	}
	// commit
	CommitLine();

	// add undo info
	//
	ec = AddUndoInfo(utInsert, line, 1, line, 1, lpText, nText);

	return errOK;
}

//**************************************************************************
ERRCODE Bbuffer::Copy(int& line, int& col, int& nText, LPTSTR& lpCopied)
{
	ERRCODE ec;
	int		nCopied;
	int		nTotal;

	nTotal	 = nText;
	nText	 = 0;
	lpCopied = NULL;

	if(nTotal <= 0)
		return errBAD_PARAMETER;

	int		sl = line;
	int		sc = col;

	if((ec = EditLine(line, col)) != errOK)
		return ec;

	nText = nTotal;
	lpCopied = new TCHAR [ nTotal + 2  ];
	nCopied  = 0;

	while(nText > 0)
	{
		if((ec = EditLine(line, col)) != errOK)
			return ec;

		LPTSTR pi	  = m_sandbox + col - 1;
		int	   nExtra = m_sandlen - col + 1 - nText;

		if(nExtra > 0)
		{
			// only partial line
			//
			while(nText-- > 0)
				if(nCopied < nTotal)
					lpCopied[nCopied++] = *pi++;
		}
		else
		{
			int		nExtant = col - 1;
			int		nRemain = m_sandlen - nExtant;
			int		i;

			// rest of entire line, possibly next line too
			//
			nText    -= nRemain;

			// copy the remainder (right) contents to the undo buffer
			//
			for(i = 0; i < nRemain && nCopied < nTotal; i++)
				lpCopied[nCopied++] = *pi++;

			// move to start of next line
			//
			i = line;
			line++;
			MoveTo(line);
			col = 1;
			m_changedlines = true;

			ec = EditLine(line, col);

			// if reached end of buffer no more looping
			if(i >= line)
				break;
		}
	}
	nText  = nCopied;
	return errOK;
}

//**************************************************************************
ERRCODE Bbuffer::Locate(int& line, int& col, LPCTSTR lpText, int nText, bool casesensi, bool rev)
{
	TCHAR	lpPattern[MAX_SEARCH_STR];
	int		nPattern;
	ERRCODE	ec;
	bool	match;
	TCHAR	sc, pc;
	int		pcurs;
	int		lastline;
	int		fline, fcol;

	if(! lpText || nText <= 0)
		return errBAD_PARAMETER;

	MoveTo(line);

	nPattern = nText;

	for(pcurs = 0; pcurs < nPattern; pcurs++)
	{
		sc = lpText[pcurs];
		if(! casesensi)
			if(sc >= 'a' && sc <= 'z')
				sc -= ('a' - 'A');
		lpPattern[pcurs] = sc;
	}

	ec = GetLineText(line, lpText, nText);
	if(ec != errOK) return ec;

	if(col > nText) col = nText;
	fline = 0;
	fcol  = 0;

	do
	{
		pcurs = 0;
		fline = line;
		fcol  = col;

		// attempt to find pattern
		//
		do
		{
			// look at byte from source and pattern
			//
			sc = lpText[col - 1];
			pc = lpPattern[pcurs];

			if(! casesensi)
				if(sc >= 'a' && sc <= 'z')
					sc -= ('a' - 'A');

			// is src the same as in pattern ?
			//
			if(! (match = (sc == pc)))
				break;

			// exit when end of pattern is reached
			//
			if(++pcurs == nPattern)
				break;

			lastline = m_curline;

			// move to next position in buffer
			//
			if(++col > nText)
			{
				col = 1;
				line = m_curline + 1;
			}
			while(line != lastline)
			{
			 	MoveTo(line);
				if(line != m_curline)
					return errFAILURE;
				ec = GetLineText(line, lpText, nText);
				if(ec != errOK) return ec;
				lastline = line;
				if(nText == 0) line++;
			 }

		} while(match);

		lastline = m_curline;

		// if found, boot out here
		//
		if(match && pcurs == nPattern)
		{
			line = fline;
			col  = fcol;

			return errOK;
		}
		col  = fcol;
		line = fline;

		// not found, so move one position from start
		//
		if(rev)
		{
			if(--col <= 0)
			{
				if(line <= 1)
					return errFAILURE;
				ec = GetLineText(line - 1, lpText, nText);
				line = m_curline;
				if(ec != errOK) return ec;
				col = nText;
			}
		}
		else
		{
			if(++col > nText)
			{
				lastline = line;
				ec = GetLineText(line + 1, lpText, nText);
				if(ec != errOK) return ec;
				line = m_curline;
				if(m_curline <= lastline)
					return errFAILURE;
				col = 1;
			}
		}
	}
	while(! match);

	return errFAILURE;
}

//**************************************************************************
ERRCODE Bbuffer::AddUndoInfo(
								UndoType type,
								int line, int col,
								int eline, int ecol,
								LPCTSTR pParm,
								int nText
							)
{
	LPUNDOINFO pInfo;

	if(m_noundo) return errOK;
	pInfo = new UNDOINFO(type, line, col, eline, ecol, pParm, nText);
	if(! pInfo) return errNO_MEMORY;
	if(m_suppressundo)
	{
		// inside an undo operation, so push on redo stack
		pInfo->m_next = m_redos;
		m_redos = pInfo;
	}
	else
	{
		// push on undo stack
		pInfo->m_next = m_undos;
		m_undos = pInfo;
	}
	return errOK;
}

//**************************************************************************
ERRCODE Bbuffer::InfoDo(LPUNDOINFO pInfo, int& line, int& col)
{
	ERRCODE ec = errOK;

	switch(pInfo->m_type)
	{
	case utInsert:
		line = pInfo->m_line;
		col  = pInfo->m_col;
		ec   = Delete(line, col, pInfo->m_ntext);
		break;

	case utDelete:
		line = pInfo->m_line;
		col  = pInfo->m_col;
		ec   = Insert(line, col, pInfo->m_ptext, pInfo->m_ntext);
		break;

	case utPosition:
		line = pInfo->m_line;
		col  = pInfo->m_col;
		ec   = AddUndoInfo(utPosition, line, col, line, col, NULL, 0);
		break;

	default:
		break;
	}
	return ec;
}

//**************************************************************************
ERRCODE Bbuffer::Redo(int& line, int& col)
{
	LPUNDOINFO pInfo;
	ERRCODE    ec;

	if(! m_redos) return errOK;

	do
	{
		pInfo	= m_redos;
		m_redos = m_redos->m_next;

		ec = InfoDo(pInfo, line, col);

		delete pInfo;
	}
	while(m_undos && (m_undos->m_type == utPosition));

	return ec;
}

//**************************************************************************
ERRCODE Bbuffer::Undo(int& line, int& col)
{
	LPUNDOINFO pInfo;
	ERRCODE    ec;

	if(! m_undos) return errOK;
	m_suppressundo = true;

	do
	{
		pInfo	= m_undos;
		m_undos = m_undos->m_next;

		ec = InfoDo(pInfo, line, col);

		if(! m_undos)
			SetModified(false);

		delete pInfo;
	}
	while(m_undos && (m_undos->m_type == utPosition));

	m_suppressundo = false;
	return ec;
}

//**************************************************************************
ERRCODE Bbuffer::LineToLong(LPCTSTR where)
{
	// whine
	return errOVERFLOW;
}

//**************************************************************************
ERRCODE Bbuffer::Read(LPCTSTR lpFilename)
{
	if(! lpFilename)
		lpFilename = m_name;

	if(lpFilename)
	{
		// check if name is a "//hostname" type file
		//
		if(
				_tcsstr(lpFilename, _T("//"))
			||	(_tcsstr(lpFilename, _T("\\\\")) && ! _tcsstr(lpFilename, _T("\\\\.")))
		)
		{
			if(
					(lpFilename[0] == _PTC_ && lpFilename[1] == _PTC_)
				||
					! _tcsncmp(lpFilename, _T("ftp:"), 4)
			)
				return Bbuffer::ReadFtpFile(lpFilename);
			if(! _tcsncmp(lpFilename, _T("http:"), 5))
				return Bbuffer::ReadFtpFile(lpFilename);
		}
		return Bbuffer::ReadFromFile(lpFilename);
	}
	return errBAD_PARAMETER;
}

//**************************************************************************
ERRCODE Bbuffer::Write(LPCTSTR lpFilename, TEXTENCODING encoding)
{
	// lpFilename is specified in the call ONLY if the file
	// to write is NOT the buffered file
	//
	if(lpFilename)
	{
		// check if name is a "//hostname" type file
		//
		if(_tcsstr(lpFilename, _T("//")))
		{
			if(! _tcsncmp(lpFilename, _T("ftp:"), 4))
				return Bbuffer::WriteFtpFile(lpFilename);
			if(! _tcsncmp(lpFilename, _T("http:"), 5))
				return Bbuffer::WriteFtpFile(lpFilename);
		}
		return Bbuffer::WriteToFile(lpFilename, encoding);
	}
	else
	{
		return Bbuffer::WriteToFile();
	}
	return errBAD_PARAMETER;
}

//**************************************************************************
BlineInfo Bbuffer::GetLineCommentInfo(int line)
{
	ERRCODE ec;

	ec = MoveTo(line);
	// ignore ec
	if(m_pcurline)
		return m_pcurline->GetCommentInfo();
	return liUnknown;
}

//**************************************************************************
ERRCODE Bbuffer::SetLineCommentInfo(int line, BlineInfo info)
{
	ERRCODE ec;

	ec = MoveTo(line);
	// ignore ec
	if(m_pcurline)
		return m_pcurline->SetCommentInfo(info);
	return errFAILURE;
}

//**************************************************************************
BlineInfo Bbuffer::GetLineIsInfo(int line)
{
	ERRCODE ec;

	ec = MoveTo(line);
	// ignore ec
	if(m_pcurline)
		return m_pcurline->GetIsLineInfo();
	return liUnknown;
}

//**************************************************************************
ERRCODE Bbuffer::SetLineIsInfo(int line, BlineInfo info)
{
	ERRCODE ec;

	ec = MoveTo(line);
	// ignore ec
	if(m_pcurline)
		return m_pcurline->SetIsLineInfo(info);
	return errFAILURE;
}

ERRCODE Bbuffer::ClearLinesInfo(BlineInfo info)
{
	ERRCODE ec;
	int		line = 1;

	do
	{
		ec = GetNextLineInfo(line, info);
		if(ec == errOK)
		{
			if(info & liCommentMask)
				SetLineCommentInfo(line, GetLineCommentInfo(line) & ~(info & liCommentMask));
			if(info & liIsMask)
				SetLineIsInfo(line, GetLineCommentInfo(line) & ~(info & liIsMask));
		}
	}
	while(ec == errOK);
	
	return errOK;
}

//**************************************************************************
ERRCODE Bbuffer::GetNextLineInfo(int& line, BlineInfo info)
{
	ERRCODE ec;

	do
	{
		ec = MoveTo(line + 1);
		if(m_pcurline && (GetCurLine() > line))
		{
			line++;
			if(m_pcurline->m_info & info)
			{
				return errOK;
			}
		}
		else
		{
			return errSTREAM_EOF;
		}
	}
	while(ec == errOK);
	return ec;
}

//**************************************************************************
ERRCODE Bbuffer::SetReadOnly(bool readonly)
{
	ERRCODE ec;

	ec = errOK;

	if(m_readonly == readonly)
		return ec;

	m_readonly = readonly;

	if(! readonly && ! m_untitled && m_name && m_name[0])
	{
		char		aname[MAX_PATH];
		struct stat finfo;

		// make it writable, like for real
		//
		if(GetIsFTP())
			return errFAILURE;

		TCharToChar(aname, m_name);

		if(! stat(aname, &finfo))
		{
			// chmod file (mod | 0220)
			//
			finfo.st_mode |= 0220; //S_IWUSR | S_IWGRP;

			chmod(aname, finfo.st_mode);
		}
		else
		{
			ec = errFAILURE;
		}
	}
	return ec;
}

//**************************************************************************
time_t Bbuffer::GetFileModTime(void)
{
	char		aname[MAX_PATH];
	struct stat finfo;

	if(GetIsFTP())
		return 0;

	TCharToChar(aname, m_name);

	if(! stat(aname, &finfo))
	{
		return finfo.st_mtime;
	}
	return 0;
}

//**************************************************************************
time_t Bbuffer::GetModTime(void)
{
	return m_modtime;
}

//**************************************************************************
bool Bbuffer::IsFileNewer()
{
	return GetFileModTime() > GetModTime();
}

//**************************************************************************
void Bbuffer::UpdateModTime(void)
{
	m_modtime = GetFileModTime();
}

// STATIC
//**************************************************************************
ERRCODE Bbuffer::GetUntitledName(BufType type, LPCTSTR extension, LPTSTR pName, int nName)
{
	TCHAR un[32];
	LPCTSTR tn =
		#ifdef _Windows
				_T("Untitled");
		#else
				_T("untitled");
		#endif
	LPCTSTR dext = NULL;

	switch(type)
	{
	case btC:
		dext = _T("c");
		break;
	case btCPP:
		dext = _T("cpp");
		break;
	case btCS:
		dext = _T("cs");
		break;
	case btJava:
		dext = _T("java");
		break;
	case btJavaScript:
		dext = _T("js");
		break;
	case btOBJC:
		dext = _T("m");
		break;
	case btPHP:
		dext = _T("php");
		break;
	case btPython:
		dext = _T("py");
		break;
	case btTCL:
		dext = _T("tcl");
		break;
	case btVerilog:
		dext = _T("vl");
		break;
	case btASM:
		#ifdef _Windows
		dext = _T("asm");
		#else
		dext = _T("s");
		#endif
		break;
	case btPerl:
		dext = _T("pl");
		break;
	case btHTML:
		dext = _T("html");
		break;
	case btXML:
		dext = _T("xml");
		break;
	case btTelnet:
		tn = _T("Remote");
		dext = _T("");
		break;
	case btSSH:
		tn = _T("SecureShell");
		dext = _T("");
		break;
	case btTerm:
		tn = _T("Terminal");
		dext = _T("");
		break;
	case btShell:
		tn = _T("Shell");
		dext = _T("");
		break;
	default:
		dext = _T("txt");
		break;
	}
	if(! extension)
		extension = dext;

	_sntprintf(un, 32, _T("%d"), m_nextuntitled);
	_sntprintf(pName, nName, _T(""_Pfs_""_Pfs_""_Pfs_""_Pfs_""),
			tn,
			m_nextuntitled ? un : _T(""),
			(extension && extension[0]) ? _T(".") : _T(""),
			extension ? extension : _T("txt"));
	m_nextuntitled++;
	return errOK;
}

//STATIC
//**************************************************************************
ERRCODE Bbuffer::GetTempFileName(LPTSTR lpName, int nName)
{
	ERRCODE ec;
	int     len;
	DWORD	threadID;
	static  int tnum = 1;

	lpName[0] = _T('\0');

	ec = BUtil::GetTempPath(lpName, nName);
	len = _tcslen(lpName);
	if(len >= (nName - 16)) return errOVERFLOW;
	threadID = getpid();
	_sntprintf(lpName + len, 16, _T("%04X%3d.tmp"), threadID, tnum++);
	return errOK;
}


//STATIC
//**************************************************************************
bool Bbuffer::IsSameFile(LPCTSTR lpA, LPCTSTR lpB)
{
#ifdef Windows
	return ! _tcscmp(lpA, lpB);
#else
	struct stat finfo_a, finfo_b;
	char		aname[MAX_PATH];
	char		bname[MAX_PATH];
	int			stata_ok, statb_ok;

	TCharToChar(aname, lpA);
	TCharToChar(bname, lpB);

	stata_ok = stat(aname, &finfo_a);
	statb_ok = stat(bname, &finfo_b);

	if(!stata_ok && !statb_ok)
	{
		return (finfo_a.st_dev == finfo_b.st_dev) && (finfo_a.st_ino == finfo_b.st_ino);
	}
	else if(stata_ok && statb_ok)
	{
		return ! _tcscmp(lpA, lpB);
	}
	else
	{
		return false;
	}
#endif
}

//**************************************************************************
BtextBuffer::BtextBuffer(
						LPCTSTR name,
						BufType type,
						TEXTENCODING encoding,
						TEXTENCODING defaultencoding,
						Bed* editor
				)
		: Bbuffer(name, type, encoding, defaultencoding, editor)
{
}

//**************************************************************************
BtextBuffer::~BtextBuffer()
{
}

