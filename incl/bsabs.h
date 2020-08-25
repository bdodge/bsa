//--------------------------------------------------------------------
//
// File: bsabs.h
// Desc: bytestream class
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------
#ifndef _BSABS_H_
#define _BSABS_H_ 1

#ifdef BS_EXPORTS
#define BS_API __declspec(dllexport)
#elif defined(BSABS_DLL)
#define BS_API __declspec(dllimport)
#else
#define BS_API
#endif

#ifndef Windows
	#define closesocket(s) close(s)
	#define ioctlsocket(s, a, b) ioctl(s,a,b)
	#define GetLastError() errno
	#define WSAGetLastError() errno
	extern FILE* bsafopen(LPCTSTR, LPCTSTR);
	
	#ifdef SOLARIS
		typedef int socklen_t;
	#endif
#endif
	
enum STREAMACCESS { strmRead = 1, strmWrite = 2, strmReadWrite = 3 };

#define BS_MAX_NICS 8

//***********************************************************************
// io byte-stream abstract base class
//
class BS_API Bstream
{
public:
	Bstream();
	virtual ~Bstream();

public:
	virtual ERRCODE		Close		(void) = 0;
	virtual ERRCODE		Read		(char* pBuf, int& cnt)		{ return Read((BYTE*)pBuf, cnt);    }
	virtual ERRCODE		Read		(BYTE* pBuf, int& cnt) = 0;
	virtual ERRCODE		Write		(char* pBuf, int& cnt)		{ return Write((BYTE*)pBuf, cnt);   }
	virtual ERRCODE		Write		(BYTE* pBuf, int& cnt) = 0;
	virtual ERRCODE		Pend		(int to_s, int to_us) = 0;
	virtual ERRCODE		Encoding	(TEXTENCODING& encoding)	{ encoding = txtANSI; return errOK; }
	virtual ERRCODE		GetAccess	(STREAMACCESS& access)		{ access = m_access; return errOK;  }
	virtual ERRCODE		Connect		(int to_s, int to_us)		{ return errNOT_IMPLEMENTED; }
	virtual ERRCODE		Accept		(Bstream*& pConnection, int tos = -1, int tous = 0)	{ return errNOT_IMPLEMENTED; }
	virtual ERRCODE		Rewind		(void)						{ return errNOT_IMPLEMENTED; }

protected:

protected:
	STREAMACCESS		m_access;
	ERRCODE				m_lastErr;
	bool				m_bOpen;
	int					m_to_s;
	int					m_to_us;
};

//***********************************************************************
// io bytestream class based on network connection
//
class BS_API BsocketStream : public Bstream
{
public:
	BsocketStream(void);
	virtual ~BsocketStream();

public:
	virtual ERRCODE		Open		(LPCSTR pAddress, short port);
	virtual ERRCODE		Close		(void);
	virtual ERRCODE		Read		(BYTE* pBuf, int& cnt);
	virtual ERRCODE		ReadFrom	(BYTE* pBuf, int& cnt, DWORD& ipaddr, short& port);
	virtual ERRCODE		Write		(BYTE* pBuf, int& cnt);
	virtual ERRCODE		Pend		(int to_s, int to_us);
	virtual ERRCODE		Connected	(int to_s, int to_us);
	virtual ERRCODE		Connect		(int to_s, int to_us);
	virtual ERRCODE		Accept		(Bstream*& pConnection, int tos = -1, int tous = 0);
	virtual ERRCODE		SetTimeout	(int tos, int tous);
	virtual ERRCODE		SetBlocking (bool block);
	virtual ERRCODE		SetBuffering(DWORD bufSend, DWORD bufRecv);
	virtual ERRCODE		GetHostInfo	(TCHAR* host, short& port);

	static ERRCODE		GetIP		(DWORD& ipAddr, int inter); 
	static ERRCODE		GetBroadcastIP(DWORD& ipAddr, int inter); 
	static ERRCODE		Init		(void);

protected:
	virtual ERRCODE		SetSocketOptions(void);

protected:
	char*				m_addr;
	short				m_port;
	bool				m_bound;

	SOCKET				m_hSock;
	struct sockaddr*	m_pHostAddr;
	int					m_hostaddr_len;

	bool				m_nonblocking;
	DWORD				m_bufferSend;
	DWORD				m_bufferRecv;

	static bool			m_isinited;
	static DWORD*		m_inet_addr;
	static DWORD*		m_inet_bcast_addr;
//	static TCHAR		m_inet_mac_addr[16*BS_MAX_NICS];
	static int			m_numInterfaces;
	static LPCSTR		m_protocolsSupported[];
};

//***********************************************************************
// io bytestream class based on connection oriented TCP/IP socket
//
class BS_API BtcpStream : public BsocketStream
{
public:
	BtcpStream(bool nbio = true, bool ndelay = true);
	BtcpStream(SOCKET sock, LPCSTR pIP = NULL, short port = 0);
	virtual ~BtcpStream();

public:
	virtual ERRCODE		Open		(LPCSTR pAddress, short port);
	virtual ERRCODE		Close		(void)  { return BsocketStream::Close(); }
	virtual ERRCODE		Read		(BYTE* pBuf, int& cnt) { return BsocketStream::Read(pBuf, cnt); }
	virtual ERRCODE		Write		(BYTE* pBuf, int& cnt) { return BsocketStream::Write(pBuf, cnt); }
	virtual ERRCODE		Pend		(int to_s, int to_us) { return BsocketStream::Pend(to_s, to_us); }
	virtual ERRCODE		Connect		(int to_s, int to_us);
	virtual ERRCODE		Accept		(Bstream*& pConnection, int tos = -1, int tous = 0);
	virtual ERRCODE		SetNODELAY	(bool nodelay);

protected:
	virtual ERRCODE		SetSocketOptions(void);

protected:
	struct sockaddr_in	m_hostaddr;
	DWORD				m_ip;
	bool				m_nodelay;
};

//***********************************************************************
// io bytestream class based on connectionles UDP (IP datagram) socket
//
class BS_API BudpStream : public BtcpStream
{
public:
	BudpStream(bool nbio = true);
	virtual ~BudpStream();

public:
	virtual ERRCODE		Open		(LPCSTR pAddress, short port);
	virtual ERRCODE		Accept		(Bstream*& pConnection, int tos = -1, int tous = 0);
	virtual ERRCODE		Broadcast	(BYTE* pBuf, int& cnt);

protected:

protected:
	
};

//***********************************************************************
// io bytestream class based on connectionless UDP on IPX socket
//
class BS_API BipxStream : public BudpStream
{
public:
	BipxStream(bool nbio = true);
	virtual ~BipxStream();

public:
	virtual ERRCODE		Open		(BYTE netnum[4], BYTE nodenum[8], short port);

protected:

protected:
};

//***********************************************************************
// io bytestream class based on SPX socket
//
class BS_API BspxStream : public BipxStream
{
public:
	BspxStream(bool nbio = true);
	virtual ~BspxStream();

public:
	virtual ERRCODE		Open		(BYTE netnum[4], BYTE nodenum[8], short port);

protected:

protected:
};

//***********************************************************************
// io bytestream class based on file i/o
//
class BS_API BfileStream : public Bstream
{
public:
	BfileStream();
	virtual ~BfileStream();

public:
	virtual ERRCODE		Open		(LPCTSTR pAddress, LPCTSTR pAccess);
	virtual ERRCODE		Close		(void);
	virtual ERRCODE		Read		(BYTE* pBuf, int& cnt);
	virtual ERRCODE		Write		(BYTE* pBuf, int& cnt);
	virtual ERRCODE		Pend		(int to_s, int to_us);
	virtual ERRCODE		Flush		(void);
	virtual ERRCODE		Rewind		(void);

protected:

protected:
	FILE*				m_pFile;
};

//***********************************************************************
// bytestream wrapper class for buffered text i/o
//
class BS_API BtextStream : public BfileStream
{
public:
	BtextStream();
	virtual ~BtextStream();

public:
	virtual ERRCODE		Open		(Bstream* pStream, TEXTENCODING enc = txtANSI);
	virtual ERRCODE		Close		(void);
	virtual ERRCODE		Read		(BYTE* pBuf, int& cnt);
	virtual ERRCODE		Write		(BYTE* pBuf, int& cnt);
	virtual ERRCODE		Pend		(int to_s, int to_us);
	virtual ERRCODE		Rewind		(void);

	virtual ERRCODE		GetEncoding	(TEXTENCODING& encoding);
	virtual ERRCODE		SetEncoding	(TEXTENCODING encoding);
	virtual ERRCODE		GetByte		(BYTE& nc);
	virtual ERRCODE		GetChar		(TCHAR& nc);
	virtual ERRCODE		PutByte		(BYTE nc);
	virtual ERRCODE		PutChar		(TCHAR nc);

protected:

protected:
	Bstream*			m_pStream;

	TEXTENCODING		m_encoding;

	char				m_rdBuf[BS_STREAM_BUFFER_SIZE];
	int					m_rdCnt;
	int					m_rdDex;

	char				m_wrBuf[BS_STREAM_BUFFER_SIZE];
	int					m_wrCnt;
	int					m_wrDex;
};

//***********************************************************************
// text to bytestream wrapper
//
class BS_API BtextStreamWrapper : public BfileStream
{
public:
	BtextStreamWrapper();
	virtual ~BtextStreamWrapper();

public:
	virtual ERRCODE		Open		(char* pText);
	virtual ERRCODE		Close		(void);
	virtual ERRCODE		Read		(BYTE* pBuf, int& cnt);
	virtual ERRCODE		Write		(BYTE* pBuf, int& cnt);
	virtual ERRCODE		Rewind		(void);

	virtual ERRCODE		GetEncoding	(TEXTENCODING& encoding) { encoding = txtANSI; return errOK; }
	virtual ERRCODE		GetByte		(BYTE& nc);
	virtual ERRCODE		GetChar		(TCHAR& nc);
	virtual ERRCODE		PutByte		(BYTE nc);
	virtual ERRCODE		PutChar		(TCHAR nc);

protected:

protected:
	char*				m_pText;

	TEXTENCODING		m_encoding;

	int					m_rdCnt;
	int					m_rdDex;

	int					m_wrCnt;
	int					m_wrDex;
};


#endif

