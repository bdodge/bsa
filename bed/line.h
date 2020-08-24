#ifndef _LINE_H_
#define _LINE_H_ 1

#define MAX_LINE_LEN	0x7ffffff

typedef DWORD BlineInfo;

#define	liUnknown			0
#define	liNone				1
#define	liStartsSpanning	2
#define	liInSpanning		4
#define	liEndsSpanning		8
#define liCommentMask		0xF

#define liIsBreakpoint		0x10
#define liIsExecCurrent		0x20
#define liIsInError			0x40
#define liIsSearchTarget	0x80
#define liIsMask			0xF0


enum LINETERMINATION { ltLF, ltCRLF };

// line object
//**************************************************************************
class Bline
{
public:
	Bline									(LPTSTR pData, int len);
	~Bline									();

public:
	virtual ERRCODE			GetText			(LPCTSTR& pText, int& nText);
	virtual ERRCODE			SetText			(LPTSTR pText, int nText);
	virtual BlineInfo		GetCommentInfo	(void)			{ return m_info & liCommentMask; }
	virtual BlineInfo		GetIsLineInfo	(void)			{ return m_info & liIsMask; 	 }
	virtual ERRCODE			SetCommentInfo	(BlineInfo info){ m_info &= ~liCommentMask; m_info |= (info & liCommentMask); return errOK; }
	virtual ERRCODE			SetIsLineInfo	(BlineInfo info){ m_info &= ~liIsMask; m_info |= (info & liIsMask); return errOK; }

protected:

public:
	Bline*					m_next;
	Bline*					m_prev;

	LPTSTR					m_text;
	int						m_len;
	int						m_alen;
	BlineInfo				m_info;
};


#endif

