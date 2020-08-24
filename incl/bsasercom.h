//--------------------------------------------------------------------
//
// File: bsasercom.h
// Desc: RS232 Serial Comm
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------

#ifndef _BSASERCOM_H_
#define _BSASERCOM_H_ 1

#ifdef SERIAL_EXPORTS
	#define BSASERCOM_API __declspec(dllexport)
#elif defined(BSASERCOM_DLL)
	#define BSASERCOM_API __declspec(dllimport)
#else
	#define BSASERCOM_API
#endif

#define BSASERCOM_USERPORT 888

//***********************************************************************
class BSASERCOM_API BserialStream : public Bstream
{
public:
	BserialStream();
	virtual ~BserialStream();

public:
	virtual ERRCODE		Open		(LPCTSTR pPort = NULL, int baud = 9600, int bits = 8, int stopbits = 1, int parity = 0, int flow = 1);
	virtual ERRCODE		Close		(void);
	virtual ERRCODE		Read		(BYTE* pBuf, int& cnt);
	virtual ERRCODE		Write		(BYTE* pBuf, int& cnt);
	virtual ERRCODE		Pend		(int to_s, int to_us);

public:
	LPCTSTR				GetPortName			(void)	{ return m_portname; }
	void				SetCustomPort		(LPCTSTR portName) {
								_tcsncpy(m_portname, portName, MAX_PATH); m_port = BSASERCOM_USERPORT; };
	static LPCTSTR		GetActualPortName	(int port, LPTSTR portbuf, int nbuf);
	static int			GetIndexOfPort		(LPCTSTR portName);
	static bool			IsSerialPort		(LPCTSTR portName);

protected:

public:

protected:
#ifdef Windows
	HANDLE				m_osport;
	HANDLE				m_event_handle;
	OVERLAPPED			m_roverlap;
	OVERLAPPED			m_woverlap;
	bool				m_read_pending;
	bool				m_write_pending;
#else
	int					m_osport;
#endif
	int					m_port;
	TCHAR				m_portname[MAX_PATH];
	int					m_baud;
	int					m_bits;
	int					m_stops;
	int					m_parity;
};


#endif

