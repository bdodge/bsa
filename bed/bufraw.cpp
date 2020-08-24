
#include "bedx.h"

//**************************************************************************
BrawBuffer::BrawBuffer(
						LPCTSTR name,
						BufType type,
						TEXTENCODING encoding,
						TEXTENCODING defaultencoding,
						Bed* editor
				)
		:
		Bbuffer(name, type, encoding, defaultencoding, editor)
{
}

//**************************************************************************
BrawBuffer::~BrawBuffer()
{
}

//**************************************************************************
ERRCODE BrawBuffer::FiddlePos(int& line, int& col)
{
	if(line < 1) line = 1;
	if(col < 1) col = 1;
	if(col > m_columns)
		col = m_columns;
	col += (line - 1) * m_columns;
	line = 1;
	return errOK;
}
	
//**************************************************************************
ERRCODE BrawBuffer::UnFiddlePos(int& line, int& col)
{
	if(line < 1) line = 1;
	if(col < 1) col = 1;
	if(m_columns < 1)
		m_columns = 1;
	line = (col - 1) / m_columns;
	col = col - (line * m_columns);
	line += 1;
	return errOK;
}

//**************************************************************************
int BrawBuffer::GetLineCount(int& maxlen)
{
	int     count;

	maxlen = m_columns;
		
	if(m_lines)
		count = (m_lines->m_len + m_columns - 1) / m_columns;
	else
		count = 0;
	return count;
}

//**************************************************************************
ERRCODE BrawBuffer::CheckAllocSize(Bline* pLine, int size)
{
	if(size > MAX_LINE_LEN)
		return errFAILURE;

	if(size > pLine->m_alen)
	{
		LPTSTR	prevtext  = pLine->m_text;
		int		prevlen   = pLine->m_len;
		int		prevalloc = pLine->m_alen;

		pLine->m_alen = size;
		if(pLine->m_alen < 512)
			pLine->m_alen = 512;
		if(pLine->m_alen > MAX_LINE_LEN)
			pLine->m_alen = MAX_LINE_LEN;
		pLine->m_text = new TCHAR [ pLine->m_alen ];

		if(! pLine->m_text)
		{
			pLine->m_text = prevtext;
			pLine->m_alen = prevlen;
			LineToLong(_T("Realloc for Edit"));
			return errFAILURE;
		}
		if(prevtext)
		{
			if(prevlen)
			{
				memcpy(pLine->m_text, prevtext, sizeof(TCHAR)*prevlen);
			}
			if(m_buf == prevtext)
			{
				m_buf = NULL;
				if(! prevalloc)
					delete [] prevtext;
			}
			if(prevalloc)
			{
				delete [] prevtext;
			}
		}
	}
	return errOK;
}

//**************************************************************************
ERRCODE BrawBuffer::GetLineText(int line, LPCTSTR& lpText, int& nText)
{
	ERRCODE ec;
	int		offset;

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
	if(ec != errOK)
	{
		lpText = NULL;
		nText = 0;
		return ec;
	}
	offset = (line - 1) * m_columns;

	if(offset >= nText)
	{
		lpText = NULL;
		nText = 0;
		return errFAILURE;
	}
	if(offset < 0)
		offset = 0;
		
	lpText += offset;
	nText  = min(m_columns, (nText - offset));

	return errOK;
}

//**************************************************************************
ERRCODE BrawBuffer::Insert(int& line, int& col, LPCTSTR lpText, int nText)
{
	ERRCODE ec;

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
	
	// make sure can fit it all
	//
	ec = CheckAllocSize(m_lines, m_lines->m_len +  nText + 8192);
	if(ec != errOK)
		return ec;

	int	   sl = line;
	int	   sc = col;

	// get existing line and size
	//
	int		lineLen;
	LPCTSTR	pl;
	
	ec = m_lines->GetText(pl, lineLen);
	if(ec != errOK)
		return ec;

	// convert external space to internal space
	//
	FiddlePos(line, col);
	
	// make a hole to the right of the insert spot
	//
	if(m_lines->m_len > (col - 1))
		memmove((LPTSTR)pl + col - 1 + nText, pl + col - 1, sizeof(TCHAR) * (m_lines->m_len - (col - 1)));

	// copy text into hole
	//
	if(nText > 0)
		memcpy((LPTSTR)pl + col - 1, lpText, sizeof(TCHAR) * nText);

	m_modified = true;

	// update column 
	col += nText;

	// and line length
	m_lines->m_len += nText;

	// Convert resulting postion to external space again
	//
	UnFiddlePos(line, col);
	
	// add undo info
	//
	ec = AddUndoInfo(utInsert, sl, sc, line, col, lpText, nText);

	return errOK;
}

//**************************************************************************
ERRCODE BrawBuffer::Delete(int& line, int& col, int nText)
{
	ERRCODE ec;

	if(nText <= 0)
		return errBAD_PARAMETER;
	if(m_readonly)
		return errPERMISSION;

	int		sl = line;
	int		sc = col;

	// get existing line and size
	//
	int		lineLen;
	LPCTSTR	pl;
	
	ec = m_lines->GetText(pl, lineLen);
	if(ec != errOK)
		return ec;

	// Convert postion to internal space
	//
	FiddlePos(line, col);

	if(nText > (m_lines->m_len - col + 1))
		nText = m_lines->m_len - col + 1;

	if(nText > 0)
	{
		// calculate ending position 
		//
		int		nl = line;
		int		nc = col;

		UnFiddlePos(nl, nc);

		// add undo info (before its gone)
		//
		ec = AddUndoInfo(utDelete, sl, sc, nl, nc, pl + col - 1, nText);

		// copy text over deleted spot
		//
		memmove((LPTSTR)pl + col - 1, pl + col - 1 + nText, sizeof(TCHAR) * (m_lines->m_len - (col - 1 + nText)));

		m_modified = true;

		// update length 
		m_lines->m_len -= nText;
	}
	// Convert resulting postion to external space again
	//
	UnFiddlePos(line, col);

	return errOK;
}

//**************************************************************************
ERRCODE BrawBuffer::Copy(int& line, int& col, int& nText, LPTSTR& lpCopied)
{
	ERRCODE ec;

	lpCopied = NULL;

	if(nText <= 0)
		return errBAD_PARAMETER;

	int		lineLen;
	LPCTSTR	pl;
	
	ec = m_lines->GetText(pl, lineLen);
	if(ec != errOK)
		return ec;

	// Convert postion to internal space
	//
	FiddlePos(line, col);

	// copy up to nText bytes
	//
	if(nText > (m_lines->m_len - col + 1))
		nText = m_lines->m_len - col + 1;
	if(nText <= 0)
		return errOK;

	lpCopied = new TCHAR [ nText + 2  ];
	memcpy(lpCopied, pl + col - 1, nText * sizeof(TCHAR));

	return errOK;
}

//**************************************************************************
ERRCODE BrawBuffer::MoveTo(int line)
{
	ERRCODE ec;

	if(line < 1) line = 1;
	
	if(! m_read)
		if((ec = Read()) != errOK)
			return ec;

	m_pcurline = m_lines;
	if(m_lines)
	{
		int zl = m_lines->m_len;
		int xl = (line - 1) * m_columns;
		
		if(xl > zl || (xl < 0))
		{
			line = 1 + (zl / m_columns);
			m_curline = line;
			m_curcol = 1;
			return errOVERFLOW;
		}
	}
	else
	{
		line = 1;
	}
	m_curline = line;
	m_curcol  = 1;
	return errOK;
}

