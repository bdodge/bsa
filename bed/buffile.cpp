
#include "bedx.h"


//**************************************************************************
ERRCODE Bbuffer::ReadFromFile(LPCTSTR lpFilename)
{
	BfileStream  fs;
	BtextStream  ts;
	Bline*		 pLine;
	Bline*		 pCurLine;
	ERRCODE		 ec = errOK;
	TCHAR		 ch;
	TEXTENCODING encoding;
	bool		 statOK;

	if(m_read) return errOK;

	// remove any lines 
	//
	for(pLine = pCurLine = m_lines; pLine; pLine = pCurLine)
	{
		pCurLine = pLine->m_next;
		delete pLine;
	}
	m_pcurline = NULL;
	m_curline  = 1;
	m_curcol   = 1;
	m_lines    = 0;
	
	if(m_buf) delete [] m_buf;
	m_buf = NULL;

	if(! lpFilename)
		lpFilename = m_name;

	// see how big the file is
	//
	struct stat finfo;
	char   aname[MAX_PATH];

	TCharToChar(aname, lpFilename);
	statOK = ! stat(aname, &finfo);

	if(statOK)
	{
		int mode;

		m_filesize = finfo.st_size;
		m_new      = false;
		
		// remember the timestamp when file was read
		m_modtime  = finfo.st_mtime;
		
		mode = finfo.st_mode;

#ifndef WIN32
		if(finfo.st_uid != getuid())
		{
			mode = mode & 0077;
			mode |= ((mode & 0070) << 3);
		}
#endif
		m_readonly = ! (mode & 0200);

		// allocate source buffer
		m_buf = new TCHAR [ m_filesize + 4 ];

		ec = fs.Open(lpFilename, _T("r"));
		
		// create a textstream to wrap file stream
		// which provides text decoding (utf8, etc.)
		//
		if(ec == errOK)
		{
			// open a text stream on the file stream for decoding
			//
			ts.Open(&fs);
		}
		else
		{
			return ec;
		}
		if(GetRaw())
		{
			ts.SetEncoding(txtANSI);
		}
		else
		{
#ifdef UNICODE
			// save the text encoding
			//
			ts.GetEncoding(encoding);

			if(m_encoding == (TEXTENCODING)-1)
			{
				m_encoding = encoding;
			}
			else if(m_encoding != encoding)
			{
				if(! m_editor)
				{
					m_encoding = encoding;
				}
				else
				{
					// if the encoding was specifed, and the text stream open
					// was certain about the text encoding, bark about the
					// mismatch, except in the case of txtANSI being overridden
					// by Unicode, since that can happen if there is no BOM
					//
					if(encoding != txtANSI || 
							(m_encoding != txtUCS2 && m_encoding != txtUCS2SWAP &&
							 m_encoding != txtUCS4 && m_encoding != txtUCS4SWAP &&
							 m_encoding != txtUTF8)
					)
					{
						int  rc;

						rc = MessageBox(
										NULL,
										m_editor->GetVersion(),
										_T("Override Actual Text Encoding ?"),
										MB_YESNO
									   );
						if(rc  == IDYES)
							ts.SetEncoding(m_encoding);
					}
					else
					{
						ts.SetEncoding(m_encoding);
					}
				}
			}
#else
			m_encoding = txtANSI;
			ts.SetEncoding(m_encoding);
#endif
		}
		// read the file in byte by byte to decode possible
		// text encodings, and create line structure
		//
		if(m_filesize > 0)
		{
			LPTSTR	bp		 = m_buf;
			LPTSTR  basep	 = bp;
			bool	gotCR    = false;
			
			pCurLine = pLine = NULL;
			m_linecnt = 0;
			
			do
			{
				ec = ts.GetChar(ch);
				
				if(ec == errOK)
				{
					if(m_type != btRaw)
					{
						if(ch == _T('\r'))
						{
							ch    = _T('\n');
							gotCR = true;
							m_hasCR = true;
						}
						else
						{
							if((ch == _T('\n')) && gotCR)
							{
								gotCR = false;
								continue;
							}
							gotCR = false;
						}
					}
					
					*bp++ = ch;
					
					if(ch == _T('\n') && m_type != btRaw)
					{
						pLine = new Bline(basep, bp - basep);

						if(! pCurLine)
						{
							m_lines = pLine;
						}
						else
						{
							pLine->m_prev    = pCurLine;
							pCurLine->m_next = pLine;
						}
						m_linecnt++;
						pCurLine = pLine;
						basep = bp;
					}
				}
			}
			while((ec == errOK) && ((bp - m_buf) < m_filesize));
			*bp = _T('\0');

			if(m_type == btRaw || bp > basep)
			{
				pLine = new Bline(basep, bp - basep);
				if(! pCurLine)
				{
					m_lines = pLine;
				}
				else
				{
					pLine->m_prev    = pCurLine;
					pCurLine->m_next = pLine;
				}
				pCurLine = pLine;
				m_linecnt++;
			}
		}
		else
		{
			// 0 length file, make a line for it, and if not raw, add
			// at least one newline
			//
			if(m_type != btRaw)
			{
#ifdef Windows
				m_hasCR = true;
#endif
				m_buf[0] = _T('\n');
				pLine = new Bline(m_buf, 1);
				pCurLine = m_lines = pLine;
			}
		}
	}
	else
	{
		// can't stat file, assume its new and make a blank file
		//
		m_filesize = 1;
		m_linecnt  = 1;
		m_modtime  = 0;
		m_readonly = false;
		m_new	   = true;
		m_buf = new TCHAR [ 2 ];

		m_buf[0] = _T('\n');
		m_buf[1] = 0;
		
#ifdef Windows
		if(m_type != btRaw)
			m_hasCR	= true;
#endif
		pLine   = new Bline(m_buf, 1);
		m_lines = pLine;
	}
	m_pcurline = m_lines;
	m_curline  = 1;
	m_curcol   = 1;
	
	if(ec == errSTREAM_EOF)
		ec = errOK;
	
	m_read = true;
	return ec;
}

//**************************************************************************
ERRCODE Bbuffer::WriteToFile(LPCTSTR lpFilename, TEXTENCODING encoding)
{
	BfileStream fs;
	BfileStream bfs;
	BtextStream ts;
	ERRCODE		ec;
	TCHAR		backupName[MAX_PATH];
	LPCTSTR		pText;
	int			nText;
	int			i;

	if(! m_read)   return errFAILURE;	
	if(m_readonly)
	{
		if(! lpFilename)
			return errFAILURE;
		if(! _tcscmp(lpFilename, m_name))
			return errFAILURE;
	}
	if(! m_new && ! lpFilename)
	{
		// copy byte-by-byte original file to backup copy
		//
		_sntprintf(backupName, MAX_PATH, _T("" _Pfs_ ".bak"), m_name);
		ec = bfs.Open(backupName, _T("w"));
		
		if(ec == errOK)
		{
			BYTE iob[8192];
			int  nRead;
			
			ec = fs.Open(m_name, _T("r"));
			
			while(ec == errOK)
			{
				nRead = sizeof(iob);
				ec = fs.Read(iob, nRead);
				
				if(ec == errOK && nRead > 0)
					ec = bfs.Write(iob, nRead);
				else
					break;
			}
			fs.Close();
			bfs.Close();
		}
	}
	if(! lpFilename)
	{
		lpFilename = m_name;
	}

	if(encoding == (TEXTENCODING)-1)
	{
		if(m_encoding == (TEXTENCODING)-1)
			encoding = m_defaultencoding;
		else
			encoding = m_encoding;
	}

	ec = fs.Open(lpFilename, _T("w"));

	// create a textstream to wrap file stream
	// which provides text coding (utf8, etc.)
	//
	if(ec == errOK)
	{
		ts.Open(&fs, encoding);
	}
	else
	{
		return ec;
	}
	
	// write the file out byte by byte to encode
	//
	Bline*	pLine;

	for(pLine = m_lines, m_linecnt = 0; pLine; pLine = pLine->m_next, m_linecnt++)
	{
		ec = pLine->GetText(pText, nText);

		for(i = 0; ec == errOK && i < nText; i++)
		{
			if(m_type != btRaw)
				if(pText[i] == _T('\n') && m_hasCR && (i > 0 || pText[i-1] != _T('\r')))
					ec = ts.PutChar(_T('\r'));
			ec = ts.PutChar(pText[i]);
		}
	}
	ts.Close();
	fs.Close();

	m_pcurline = m_lines;
	m_curline  = 1;
	m_curcol   = 1;

	m_modified = false;
	m_read     = false;

	return ec;
}
