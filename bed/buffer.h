#ifndef _BUFFER_H_
#define _BUFFER_H_ 1

class Bview;
class Bed;

enum Blayout { voHorizontal, voVertical };

// undo info object list
//**************************************************************************

enum UndoType
{
	utBegin,
	utInsert,
	utDelete,
	utPosition,
	utEnd
};

typedef struct tagUndo
{
public:
	tagUndo(
				UndoType type,
				int line, int col,
				int eline, int ecol,
				LPCTSTR lpText,
				int ntext
			)
			{
				m_type = type;
				m_line = line; m_col = col;
				m_eline = eline, m_ecol = ecol;
				if(lpText && ntext > 0)
				{
					m_ptext = new TCHAR [ ntext + 2];
					memcpy(m_ptext, lpText, ntext * sizeof(TCHAR));
					m_ptext[ntext] = _T('\0');
					m_ntext = ntext;
				}
				else
				{
					m_ptext = NULL;
					m_ntext = 0;
				}
			}
	~tagUndo()
			{
				if(m_ptext) delete [] m_ptext;
			}

public:
	int				m_line;
	int				m_col;
	int				m_eline;
	int				m_ecol;
	LPTSTR			m_ptext;
	int				m_ntext;
	UndoType		m_type;
	struct tagUndo* m_next;
}
UNDOINFO, *LPUNDOINFO;


// buffer object
//**************************************************************************
class Bbuffer
{
public:
					Bbuffer			(
										LPCTSTR name,
										BufType type,
								  		TEXTENCODING encoding,
								  		TEXTENCODING defaultencoding,
							   			Bed* editor
									);
	virtual			~Bbuffer		();

public:
	virtual ERRCODE	Read			(LPCTSTR lpFilename = NULL);
	virtual ERRCODE	Write			(LPCTSTR lpFilename = NULL, TEXTENCODING encoding = (TEXTENCODING)-1);

	virtual ERRCODE	ReadFromFile	(LPCTSTR lpFilename = NULL);
	virtual ERRCODE	WriteToFile		(LPCTSTR lpFilename = NULL, TEXTENCODING encoding = (TEXTENCODING)-1);
	virtual ERRCODE	ReadFtpFile		(LPCTSTR lpFilename = NULL);
	virtual ERRCODE	WriteFtpFile	(LPCTSTR lpFilename = NULL, TEXTENCODING encoding = (TEXTENCODING)-1);

	virtual ERRCODE	ReadAgain		(void) { m_read = false; m_modified = false; return Read(); }

	virtual LPCTSTR	GetName			(void)	{ return m_name;		}
	virtual void	SetName			(LPCTSTR pName);
	virtual LPCTSTR	GetShortName	(void);
	virtual Bed*	GetEditor		(void)	{ return m_editor;		}
	virtual LPTSTR  GetBuffer		(void)	{ return m_buf;			}
	virtual BufType	GetType			(void)	{ return m_type;		}
	virtual void	SetType			(BufType type) { m_type = type; }
	virtual int		GetLine			(void)	{ return m_curline;		}
	virtual int		GetColumn		(void)	{ return m_curcol;		}
	virtual int		GetColumns		(void)	{ return m_columns;		}
	virtual ERRCODE	SetColumns		(int c)	{ m_columns = c; return errOK;	}
	virtual ERRCODE	SniffTabs		(int& tabspace, int& indentspace, bool& spacetabs);
	virtual ERRCODE GetLineText		(int line, LPCTSTR& lpText, int& nText);
	virtual ERRCODE Insert			(int& line, int& col, LPCTSTR lpText, int nText);
	virtual ERRCODE Delete			(int& line, int& col, int nText);
	virtual ERRCODE ReplaceLine		(int line, LPCTSTR lpText, int nText);
	virtual ERRCODE Copy			(int& line, int& col, int& nText, LPTSTR& lpCopied);
	virtual ERRCODE	Locate			(int& line, int& col, LPCTSTR pText, int nText, bool casesensi, bool rev);
	virtual ERRCODE AddUndoInfo		(
										UndoType type,
										int line, int col,
										int eline, int ecol,
										LPCTSTR pParm = NULL, int nText = 0
									);
	virtual ERRCODE Undo			(int& line, int& col);
	virtual ERRCODE Redo			(int& line, int& col);
	virtual ERRCODE CheckSandboxSize(int size);
	virtual ERRCODE EditLine		(int& line, int& col);
	virtual int		GetCurLine		(void)	{ return m_curline; }
	virtual int		GetCurCol		(void)	{ return m_curcol;  }
	virtual int		GetLineCount	(int& maxlinelen);
	virtual int		GetLineCountToEndFromLine(int startline);
	virtual TEXTENCODING
					GetEncoding		(void)	{ return m_encoding;}
	virtual void	SetEncoding		(TEXTENCODING e)	{ m_encoding = e;}
	virtual bool	GetModified		(void)	{ return m_modified;}
	virtual void	SetModified		(bool m){ m_modified = m;	}
	virtual bool	GetRead			(void)	{ return m_read;	}
	virtual bool	GetReadOnly		(void)	{ return m_readonly;}
	virtual void	SetRead			(bool r){ m_read = r;	}
	virtual ERRCODE	SetReadOnly		(bool r);
	virtual bool	GetNew			(void)	{ return m_new;		}
	virtual bool	GetUntitled		(void)	{ return m_untitled;}
	virtual void	SetUntitled		(bool u){ m_untitled = u;}
	virtual bool	GetRaw			(void)	{ return m_type == btRaw;	 }
	virtual int		GetReadLines	(void)	{ return m_linecnt;	}
	virtual int		GetReadSize		(void)	{ return m_filesize;}
	virtual bool	GetHasCR		(void)	{ return m_hasCR;}
	virtual void	SetHasCR		(bool cr){ m_hasCR = cr;}
	virtual bool	GetHasUndo		(void)  { return !m_noundo; }
	virtual void	SetHasUndo		(bool u){ m_noundo = !u;}
	virtual bool	GetCareForMods	(void)	{ return m_careformods;	}
	virtual void	SetCareForMods	(bool c){ m_careformods = c;	}
	virtual bool	GetIsFTP		(void)	{ return false; };
	virtual bool	ChangedLines	(void)	{ bool mc = m_changedlines; m_changedlines = false; return mc; }
	virtual bool	IsFileNewer		(void);
	virtual void	UpdateModTime	(void);

	virtual BlineInfo
					GetLineCommentInfo(int line);
	virtual ERRCODE	SetLineCommentInfo(int line, BlineInfo info);
	virtual BlineInfo
					GetLineIsInfo	(int line);
	virtual ERRCODE	SetLineIsInfo	(int line, BlineInfo info);
	virtual ERRCODE	ClearLinesInfo	(BlineInfo info);
	virtual ERRCODE GetNextLineInfo (int& line, BlineInfo info);

	static BufType	SniffType		(LPCTSTR pFilename, TEXTENCODING& encoding);
	static ERRCODE	GetUntitledName	(BufType type, LPCTSTR ext, LPTSTR lpName, int nName);
	static ERRCODE	GetTempFileName	(LPTSTR lpName, int nName);
	static	bool	IsSameFile		(LPCTSTR lpFileA, LPCTSTR lpFileB);

protected:
	virtual ERRCODE SetScratch		(int size);
	virtual ERRCODE MoveTo			(int line);
	virtual ERRCODE CommitLine		(void);
	virtual ERRCODE InfoDo			(LPUNDOINFO pInfo, int& line, int& col);
	virtual ERRCODE	LineToLong		(LPCTSTR msg);
	virtual time_t	GetModTime		(void);
	virtual time_t	GetFileModTime	(void);

protected:
	Bed*			m_editor;
	BufType			m_type;
	TEXTENCODING	m_encoding;
	TEXTENCODING	m_defaultencoding;
	TCHAR			m_localName[MAX_PATH];
	bool			m_new;
	bool			m_untitled;
	bool			m_read;
	bool			m_modified;
	bool			m_readonly;
	int				m_columns;
	bool			m_changedlines;
	time_t			m_modtime;
	int				m_filesize;
	bool			m_hasCR;
	bool			m_careformods;
	int				m_curline;
	int				m_curcol;
	Bline*			m_lines;
	int				m_linecnt;
	LPTSTR			m_name;
	LPTSTR			m_typename;
	LPTSTR			m_buf;
	Bline*			m_pcurline;
	LPUNDOINFO		m_undos;
	LPUNDOINFO		m_redos;
	bool			m_noundo;
	bool			m_suppressundo;
	bool			m_inbox;

	static int		m_refs;
	static TCHAR*	m_sandbox;
	static TCHAR*	m_scratch;
	static int		m_sandlen;
	static int		m_sandalloc;
	static int		m_scratchalloc;
	static int		m_nextuntitled;
	static TCHAR	m_untitledName[MAX_PATH];
};

#define NO_API

//**************************************************************************
class BtextBuffer : public Bbuffer
{
public:
					BtextBuffer		(
										LPCTSTR name,
										BufType type,
								  		TEXTENCODING encoding,
								  		TEXTENCODING defaultencoding,
							   			Bed* editor
									);
	virtual			~BtextBuffer	();

public:
};

//**************************************************************************
class BrawBuffer : public Bbuffer
{
public:
					BrawBuffer		(
										LPCTSTR name,
										BufType type,
								  		TEXTENCODING encoding,
								  		TEXTENCODING defaultencoding,
							   			Bed* editor
									);
	virtual			~BrawBuffer		();

public:
	virtual int		GetLineCount	(int& maxline);
	virtual ERRCODE GetLineText		(int line, LPCTSTR& lpText, int& nText);
	virtual ERRCODE Insert			(int& line, int& col, LPCTSTR lpText, int nText);
	virtual ERRCODE Delete			(int& line, int& col, int nText);
	virtual ERRCODE Copy			(int& line, int& col, int& nText, LPTSTR& lpCopied);

protected:
	virtual ERRCODE CheckAllocSize	(Bline* pLine, int size);
	virtual ERRCODE MoveTo			(int line);
	virtual ERRCODE FiddlePos		(int& line, int& col);
	virtual ERRCODE UnFiddlePos		(int& line, int& col);
};


// list of buffers
//
KEY_VAL_LIST_TEMPLATE(BbufList, Bbuffer*, NO_API);


#endif

