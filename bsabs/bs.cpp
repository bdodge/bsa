#include "bsx.h"

//***********************************************************************
// io bytestream class abstract base class
//
Bstream::Bstream()
	 :
	m_access(strmReadWrite),
	m_lastErr(errOK),
	m_bOpen(false),
	m_to_s(15),
	m_to_us(0)
{
}

//***********************************************************************
Bstream::~Bstream()
{
}


//***********************************************************************
//***********************************************************************
//***********************************************************************
// io bytestream class based on socket (generic, base class)
//
BsocketStream::BsocketStream()
		:
		m_addr(NULL),
		m_port(0),
		m_bound(false),
		m_hSock(INVALID_SOCKET),
		m_pHostAddr(NULL),
		m_nonblocking(true),
		m_bufferSend(0xFFFFFFFF),
		m_bufferRecv(0xFFFFFFFF)
{
}

//***********************************************************************
BsocketStream::~BsocketStream()
{
	if(m_bOpen) Close();
	if(m_addr)	delete [] m_addr;
}

//***********************************************************************
ERRCODE BsocketStream::SetSocketOptions()
{
	int toms		= m_to_s * 1000 + m_to_us / 1000;
	int linger		= 0;
	int reuse		= 1;
	int zero 		= 0;
	DWORD nbio		= m_nonblocking ? 1 : 0;

	ioctlsocket(m_hSock, FIONBIO, &nbio);

	setsockopt(m_hSock, SOL_SOCKET , SO_RCVTIMEO, (char*)&toms, sizeof(toms));
	setsockopt(m_hSock, SOL_SOCKET , SO_SNDTIMEO, (char*)&toms, sizeof(toms));
	setsockopt(m_hSock, SOL_SOCKET , SO_REUSEADDR, (char*)&reuse, sizeof(reuse));

	if(m_bufferSend != (DWORD)0xFFFFFFFF)
		setsockopt(m_hSock, SOL_SOCKET , SO_SNDBUF, (char*)&m_bufferSend, sizeof(m_bufferSend));
	if(m_bufferRecv != (DWORD)0xFFFFFFFF)
		setsockopt(m_hSock, SOL_SOCKET , SO_RCVBUF, (char*)&m_bufferRecv, sizeof(m_bufferRecv));

	//setsockopt(m_hSock, SOL_SOCKET , SO_LINGER,   (char*)&linger, sizeof(linger));
	return errOK;
}

//***********************************************************************
ERRCODE BsocketStream::SetTimeout(int tos, int tous)
{
	m_to_s  = tos;
	m_to_us = tous;

	return SetSocketOptions();
}

//***********************************************************************
ERRCODE BsocketStream::SetBlocking(bool block)
{
	if(m_nonblocking == ! block)
		return errOK;
	m_nonblocking = ! block;
	return SetSocketOptions();
}

//***********************************************************************
ERRCODE BsocketStream::SetBuffering(DWORD bufSend, DWORD bufRecv)
{
	if(m_bufferSend == bufSend && m_bufferRecv == bufRecv)
		return errOK;
	m_bufferSend = bufSend;
	m_bufferRecv = bufRecv;
	return SetSocketOptions();
}

//***********************************************************************
ERRCODE BsocketStream::Open(LPCSTR pAddress, short port)
{
	m_port = port;
	if(! pAddress)
	{
		// ??
		return errSOCKET_NO_ADDRESS;
	}
	m_addr = new char [ strlen(pAddress) + 2 ];
	strcpy(m_addr, pAddress);
	return errOK;
}

//***********************************************************************
ERRCODE BsocketStream::Close(void)
{
	if(m_hSock != INVALID_SOCKET)
	{
#ifdef Windows
		shutdown(m_hSock, 2);
#endif
		int ret = closesocket(m_hSock);
	}
	m_hSock = INVALID_SOCKET;
	m_bOpen = false;
	return errOK;
}

//***********************************************************************
ERRCODE BsocketStream::Read(BYTE* pBuf, int& cnt)
{
	int		ret;

	if(! m_bOpen) return errSTREAM_NOT_OPEN;

	ret = recv(m_hSock, (char*)pBuf, cnt, 0);
	cnt = ret;

	if(ret == SOCKET_ERROR)
	{
		cnt = 0;
#ifdef Windows
		ret = WSAGetLastError();
			if(ret == WSAEWOULDBLOCK)
#else
		if(errno == EWOULDBLOCK)
#endif
			return errOK;
		return errSTREAM_READ;
	}
	return errOK;
}


//***********************************************************************
ERRCODE BsocketStream::ReadFrom(BYTE* pBuf, int& cnt, DWORD& ipaddr, short& port)
{
	struct sockaddr_in fromaddr;

	int			ret;
	socklen_t	fromlen;

	if(! m_bOpen) return errSTREAM_NOT_OPEN;

	memset(&fromaddr, 0, sizeof(struct sockaddr_in));

	fromaddr.sin_family		 = AF_INET;
	fromlen 				 = sizeof(struct sockaddr_in);

	ret = recvfrom(m_hSock, (char*)pBuf, cnt, 0, (struct sockaddr*)&fromaddr, &fromlen);
	cnt = ret;

	if(ret == SOCKET_ERROR)
	{
		cnt = 0;
		port = 0;
		ipaddr = 0;
		ret = WSAGetLastError();
		return errSTREAM_READ;
	}
	else
	{
		ipaddr = fromaddr.sin_addr.s_addr;
		port   = htons(fromaddr.sin_port);
	}
	return errOK;
}
//#define DBG_WRITE_TIME 1

//***********************************************************************
ERRCODE BsocketStream::Write(BYTE* pBuf, int& cnt)
{
	int		ret;
	int		xcnt = 0;
	int		vcnt;
	int		chunks = 0;

	if(! m_bOpen) return errSTREAM_NOT_OPEN;

	#ifdef DBG_WRITE_TIME
	struct timeb st, wst, wet;
	DWORD wdiff, wtot = 0;
	ftime(&st);
	#endif

	do
	{
		// check writeability of socket
		{
			int		sv;
			fd_set  wfds;

			struct  timeval timeout;
				
			FD_ZERO (&wfds);
			FD_SET  (m_hSock, &wfds);

			timeout.tv_sec  = m_to_s;
			timeout.tv_usec = m_to_us;

			sv = select(m_hSock + 1, NULL, &wfds, NULL, ((m_to_s >= 0 || m_to_us >= 0) ? &timeout : NULL));
			#ifndef Windows
			if (sv < 0) 
				if(errno == EINTR)
					sv = 0;
			#endif
			//if(sv < 0) MessageBox(0, 0, _T("Sock write select fail"), 0);
			//if(sv == 0) MessageBox(0, 0, _T("sock pend = 0"), 0);
			if(sv < 0) 
				return errSOCKET_SELECT_FAILURE;
			else if(sv == 0)
				return errSTREAM_TIMEOUT;
		}
		// limit send size to 65k, since windows can't seem to handle
		// any more than that at once.
		//
		//ftime(&wst);
		vcnt = min((cnt-xcnt), 65536);
		ret = send(m_hSock, (const char*)pBuf + xcnt, vcnt, 0);
		//ftime(&wet);
		//wdiff = (wet.time - wst.time) * 1000;
		//wdiff += wet.millitm - wst.millitm;
		//wtot += wdiff;
		
		if(ret > 0)	xcnt += ret;
		chunks++;
	}
	while(ret != SOCKET_ERROR && xcnt < cnt);

	#ifdef DBG_WRITE_TIME
	struct timeb xt;
	DWORD  diff;
	TCHAR  xm[256];

	ftime(&xt);
	diff = (xt.time - st.time) * 1000;
	diff += xt.millitm - st.millitm;
				
	_sntprintf(xm, 256, _T("BSABS %d bytes in %d ms  %dms writing (%d chunks)\n"), xcnt, diff, wtot, chunks);
	OutputDebugString(xm);
	#endif

	if(ret == SOCKET_ERROR)
	{
		#if 0
		TCHAR xxb[256];
		int err = WSAGetLastError();
		_sntprintf(xxb,256,_T("Socket Write=r=%d c=%d e=%d"), ret, cnt, err);
		MessageBox(0, 0, xxb, 0);
		#endif
		cnt = 0;
		return errSTREAM_WRITE;
	}
	return errOK;
}

//***********************************************************************
ERRCODE BsocketStream::Pend(int to_secs, int to_usecs)
{
	int		sv;
	fd_set  rfds;
	//fd_set  efds;

	struct  timeval timeout;

	if(! m_bOpen) return errSTREAM_NOT_OPEN;
			
	FD_ZERO (&rfds);
	FD_SET  (m_hSock, &rfds);

	#if 0
	FD_ZERO (&efds);
	FD_SET  (m_hSock, &efds);

	timeout.tv_sec  = 0;
	timeout.tv_usec = 0;

	// first check for errors
	sv = select(m_hSock + 1, NULL, NULL, &efds, &timeout);
	#ifndef Windows
	if (sv < 0) 
		if(errno == EINTR)
			sv = 0;
	#endif
	if(sv != 0)
		return errSOCKET_SELECT_FAILURE;
	#endif

	timeout.tv_sec  = to_secs;
	timeout.tv_usec = to_usecs;

	// next check for readability
	sv = select(m_hSock + 1, &rfds, NULL, NULL, (to_secs >= 0) ? &timeout : NULL);
	#ifndef Windows
	if (sv < 0) 
		if(errno == EINTR)
			sv = 0;
	#endif

	if(sv < 0) 
		return errSOCKET_SELECT_FAILURE;
	else if(sv > 0)
		return errSTREAM_DATA_PENDING;
	else
		return errOK;
}

//***********************************************************************
ERRCODE BsocketStream::Connected(int to_secs, int to_usecs)
{
	int		sv;
	fd_set  efds;

	struct  timeval timeout;

	if(! m_bOpen) return errSTREAM_NOT_OPEN;
			
	FD_ZERO (&efds);
	FD_SET  (m_hSock, &efds);

	timeout.tv_sec  = 0;
	timeout.tv_usec = 0;

	// first check for errors
	sv = select(m_hSock + 1, NULL, NULL, &efds, &timeout);
	#ifndef Windows
	if (sv < 0) 
		if(errno == EINTR)
			sv = 0;
	#endif
	if(sv != 0)
		return errSOCKET_SELECT_FAILURE;
	return errOK;
}

//***********************************************************************
ERRCODE BsocketStream::Connect(int to_secs, int to_usecs)
{
	return errNOT_IMPLEMENTED;
}

//***********************************************************************
ERRCODE BsocketStream::Accept(Bstream*& pConnection, int tos, int tous)
{
	pConnection = NULL;
	return errNOT_IMPLEMENTED;
}

//***********************************************************************
ERRCODE BsocketStream::GetHostInfo(TCHAR* host, short& port)
{
	if(host) CharToTChar(host, m_addr);
	port = m_port;
	return errOK;
}

//***********************************************************************
//***********************************************************************
//***********************************************************************
// io bytestream class based on connection oriented TCP/IP socket
//

BtcpStream::BtcpStream(bool nbio, bool ndelay) 
		:
		BsocketStream(),
		m_nodelay(ndelay)
{
	m_nonblocking = nbio;
}

//***********************************************************************
BtcpStream::BtcpStream(SOCKET sock, LPCSTR pIP, short port) : BsocketStream()
{
	m_hSock = sock;

	// these are defaults	
	m_nonblocking = false;
	m_nodelay     = false;
	m_bufferSend  = 0;
	m_bufferRecv  = 0;
	
	// get options from already open socket
	if(sock != INVALID_SOCKET)
	{
		int rc;
		int ibuf;
		socklen_t ibuflen;
		
		ibuflen = sizeof(ibuf);
		rc = getsockopt(m_hSock, IPPROTO_TCP, TCP_NODELAY, (char*)&ibuf, &ibuflen);
		if(! rc) m_nodelay = ibuf != 0;		
	#if 0 //  let snd ad rcv buf alone if not explicitly set by SetBuffering
		ibuflen = sizeof(ibuf);
		rc = getsockopt(m_hSock, SOL_SOCKET , SO_SNDBUF, (char*)&ibuf, &ibuflen);
		if(! rc) m_bufferSend = ibuf;
		ibuflen = sizeof(ibuf);
		rc = getsockopt(m_hSock, SOL_SOCKET , SO_RCVBUF, (char*)&ibuf, &ibuflen);
		if(! rc) m_bufferRecv = ibuf;
	#endif
	}
	SetSocketOptions();
	m_bOpen = true;
	
	if(pIP)
	{
		char address[32];

		if(pIP[0] < '0' || pIP[0] > '9')
		{
			// host name, use DNS to resolve name to an IP address
			//
			struct sockaddr_in	tcp_addr;
			struct hostent*		hname;
			
			if((hname = gethostbyname(pIP)) != NULL)
			{
				memcpy(&tcp_addr.sin_addr, hname->h_addr, hname->h_length);
				strcpy(address, inet_ntoa(tcp_addr.sin_addr));
				pIP = address;
			}
		}
		m_port = port;
		m_addr = new char [ strlen(pIP) + 2 ];
		strcpy(m_addr, pIP);

		memset(&m_hostaddr, 0, sizeof(struct sockaddr_in));

		m_hostaddr.sin_family		= AF_INET;
		m_hostaddr.sin_addr.s_addr  = inet_addr(m_addr);
		m_hostaddr.sin_port			= htons(m_port);
		m_pHostAddr					= (struct sockaddr*)&m_hostaddr;
		m_hostaddr_len				= sizeof(struct sockaddr_in);
	}
}

//***********************************************************************
BtcpStream::~BtcpStream()
{
}

//***********************************************************************
ERRCODE BtcpStream::SetSocketOptions()
{
	int nodelay		= m_nodelay ? 1 : 0;
	int	zero		= 65536*4;

	setsockopt(m_hSock, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));

	return BsocketStream::SetSocketOptions();
}

//***********************************************************************
ERRCODE BtcpStream::SetNODELAY(bool nodelay)
{
	if(m_nodelay == nodelay)
		return errOK;
	return SetSocketOptions();
}

//***********************************************************************
ERRCODE BtcpStream::Open(LPCSTR  pIP, short port)
{
	ERRCODE ec;
	char    address[MAX_HOST_NAME + 2];

	if(m_bOpen) return errOK;

	// if the address is in hostname form, try and resolve it
	// to an IP address
	//
	if(! pIP) pIP = "127.0.0.1";

	if(pIP[0] < '0' || pIP[0] > '9')
	{
		// host name, use DNS to resolve name to an IP address
		//
		struct sockaddr_in	tcp_addr;
		struct hostent*		hname;
		
		if((hname = gethostbyname(pIP)) != NULL)
		{
			memcpy(&tcp_addr.sin_addr, hname->h_addr, hname->h_length);
			strcpy(address, inet_ntoa(tcp_addr.sin_addr));
			pIP = address;
		}
		else
		{
			return errSOCKET_ADDR_NOT_RESOLVED;
		}
	}	
	// open base socket class
	//
	ec = BsocketStream::Open(pIP, port);
	if(ec != errOK) return ec;

	memset(&m_hostaddr, 0, sizeof(struct sockaddr_in));

	m_hostaddr.sin_family		= AF_INET;
	m_hostaddr.sin_addr.s_addr  = inet_addr(m_addr);
	m_hostaddr.sin_port			= htons(m_port);
	m_pHostAddr					= (struct sockaddr*)&m_hostaddr;
	m_hostaddr_len				= sizeof(struct sockaddr_in);

	if((m_hSock = socket(PF_INET, SOCK_STREAM, 0)) != INVALID_SOCKET)
	{
		SetSocketOptions();
	}
	else
	{
		m_hSock = INVALID_SOCKET;
		return errSOCKET_OPEN_FAILURE;
	}
	m_bOpen = true;
	return errOK;
}


//***********************************************************************
ERRCODE BtcpStream::Connect(int to_secs, int to_usecs)
{
	int ret = connect(m_hSock, m_pHostAddr, m_hostaddr_len);

	if(ret < 0)
	{
		if(m_nonblocking)
		{
#ifdef Windows
			if(WSAGetLastError() == WSAEWOULDBLOCK)
#else
			if(errno == EWOULDBLOCK || errno == EINPROGRESS)
#endif
			{
				int		sv;
				fd_set  wfds;

				struct  timeval timeout;

				if(! m_bOpen) return errSTREAM_NOT_OPEN;
						
				FD_ZERO (&wfds);
				FD_SET  (m_hSock, &wfds);

				timeout.tv_sec  = to_secs;
				timeout.tv_usec = to_usecs;

				sv = select(m_hSock + 1, NULL, &wfds, NULL, &timeout);
				#ifndef Windows
				if (sv < 0) 
					if(errno == EINTR)
						sv = 0;
				#endif
	
				if(sv < 0) 
					return errSOCKET_SELECT_FAILURE;
				else if(sv == 0)
					return errSTREAM_TIMEOUT;
				m_bound = true;
				return errOK;
			}
		}
		return errSOCKET_CONNECT_FAILURE;
	}
	m_bound = true;
	return errOK;
}

//***********************************************************************
ERRCODE BtcpStream::Accept(Bstream*& pConnection, int tos, int tous)
{
	int attempts;

	pConnection = NULL;

	if(! m_bound)
	{
		// bind socket to our host IP address, at m_port
		//
		for(attempts = 0; attempts < 10; attempts++)
		{
    		if(bind(m_hSock, m_pHostAddr, m_hostaddr_len) != 0)
    		{
#ifdef Windows
   				return errSOCKET_BIND_FAILURE;
#else
    			if(errno == EADDRINUSE || errno == EWOULDBLOCK)
    				Bthread::Sleep(250);
    			else
    				return errSOCKET_BIND_FAILURE;
#endif
    		}
    		else
    		{
    			break;
    		}
		}
    
		// start listening for connections on this port
		//
		if(listen(m_hSock, 1) != 0)
		{
			return errSOCKET_LISTEN_FAILURE;
		}
		m_bound = true;
	}

	// accept on the listening socket, saving ip address 
	//
	{
		SOCKET				hSock;
		struct sockaddr_in	tcp_client_addr;
		socklen_t			slen = sizeof(tcp_client_addr);
		LPSTR				pIP;
		short				port;

		if(m_nonblocking)
		{
			ERRCODE ec;

			ec = Pend(tos, tous);
			if(ec != errSTREAM_DATA_PENDING) 
				return ec;
			if(ec == errOK)
				return errSTREAM_TIMEOUT;
		}
		hSock = accept(m_hSock, (struct sockaddr *)&tcp_client_addr, &slen);

		if(hSock == INVALID_SOCKET)
			return errSOCKET_ACCEPT_FAILURE;

		// build a BtcpStream with socket
		//
		pIP = inet_ntoa(tcp_client_addr.sin_addr);
		port = tcp_client_addr.sin_port;

		pConnection = new BtcpStream(hSock, pIP, port);
	}
	return errOK;
}

//***********************************************************************
//***********************************************************************
//***********************************************************************
// io bytestream class based on connectionles UDP (IP datagram) socket
//
BudpStream::BudpStream(bool nbio) : BtcpStream(nbio)
{
}

//***********************************************************************
BudpStream::~BudpStream()
{
	if(m_bOpen) Close();
}

//***********************************************************************
ERRCODE BudpStream::Open(LPCSTR  pIP, short port)
{
	ERRCODE ec;
	char    address[MAX_HOST_NAME + 2];
	
	if(m_bOpen) return errOK;

	// if the address is in hostname form, try and resolve it
	// to an IP address
	//
	if(! pIP) pIP = "127.0.0.1";

	if(pIP[0] < '0' || pIP[0] > '9')
	{
		// host name, use DNS to resolve name to an IP address
		//
		struct sockaddr_in	tcp_addr;
		struct hostent*		hname;
		
		if((hname = gethostbyname(pIP)) != NULL)
		{
			memcpy(&tcp_addr.sin_addr, hname->h_addr, hname->h_length);
			strcpy(address, inet_ntoa(tcp_addr.sin_addr));
			pIP = address;
		}
		else
		{
			return errSOCKET_ADDR_NOT_RESOLVED;
		}
	}
	// open base socket class
	//	
	ec = BsocketStream::Open(pIP, port);
	if(ec != errOK) return ec;

	memset(&m_hostaddr, 0, sizeof(struct sockaddr_in));

	m_hostaddr.sin_family		= AF_INET;
	m_hostaddr.sin_addr.s_addr  = inet_addr(m_addr);
	m_hostaddr.sin_port			= htons(m_port);
	m_pHostAddr					= (struct sockaddr*)&m_hostaddr;
	m_hostaddr_len				= sizeof(struct sockaddr_in);

	if((m_hSock = socket(PF_INET, SOCK_DGRAM, 0)) != INVALID_SOCKET)
	{
		SetSocketOptions();
	}
	else
	{
		int err = GetLastError();

		closesocket(m_hSock);
		m_hSock = INVALID_SOCKET;
		return errSOCKET_OPEN_FAILURE;
	}
	m_bOpen = true;
	return errOK;
}

//***********************************************************************
ERRCODE BudpStream::Accept(Bstream*& pConnection, int tos, int tous)
{
	// udp is connectionless
	pConnection = NULL;
	return errNOT_IMPLEMENTED;
}

//***********************************************************************
ERRCODE BudpStream::Broadcast(BYTE* pBuf, int& cnt)
{
	int optval = 1; 
	int optlen = sizeof(int); 
	int rv;

	// setup to broadcast
	//
	if(setsockopt(m_hSock, SOL_SOCKET, SO_BROADCAST, (char *)&optval, optlen))
	{ 
		return errSOCKET_CONNECT_FAILURE; 
	}
	// do a sendto to the address specified in open
	//
	rv = sendto(m_hSock, (char*)pBuf, cnt, 0, (struct sockaddr*)&m_hostaddr, m_hostaddr_len);
	if(rv >= 0)
	{
		cnt = rv;
		return errOK;
	}
	else
	{
		cnt = 0;
		return errSTREAM_WRITE;
	}
}

#if BSA_IPX
//***********************************************************************
//***********************************************************************
//***********************************************************************
// io bytestream class based on connectionles ipx (IP datagram) socket
//
BipxStream::BipxStream(bool nbio) : BudpStream(nbio)
{
	m_pHostAddr = (struct sockaddr*)new sockaddr_ipx;
	m_hostaddr_len = sizeof(struct sockaddr_ipx);
}

//***********************************************************************
BipxStream::~BipxStream()
{
	if(m_pHostAddr) delete [] m_pHostAddr;
	if(m_bOpen) Close();
}

#ifdef Windows
	#define sipx_family 	sa_family
    #define sipx_port   	sa_socket
    #define sipx_network	sa_netnum
	#define sipx_node		sa_nodenum
	//#define PF_IPX			NSPROTO_IPX
	#define PF_SPX			NSPROTO_SPX
#else
	#define PF_SPX			PF_IPX
#endif

//***********************************************************************
ERRCODE BipxStream::Open(BYTE netnum[4], BYTE nodenum[6], short port)
{
	ERRCODE ec;
	char    raddr[64];
	int		i;
	struct sockaddr_ipx* pAddr;

	if(m_bOpen) return errOK;

	// form char version of net address
	//
    sprintf(raddr, "%02X%02X%02X%02X : %02X-%02X-%02X-%02X-%02X-%02X : %d",
		netnum[0], netnum[1], netnum[2], netnum[3],
		nodenum[0], nodenum[1], nodenum[2], nodenum[3], nodenum[4],	nodenum[5],
		port);

	// open base socket class
	//	
	ec = BsocketStream::Open(raddr, port);
	if(ec != errOK) return ec;

	memset(m_pHostAddr, 0, sizeof(struct sockaddr_ipx));
	pAddr = (struct sockaddr_ipx*)m_pHostAddr;

	pAddr->sipx_family			= AF_IPX;
#ifdef Windows
	for(i = 0; i < 4; i++)
		pAddr->sipx_network[i]	= netnum[i];
#else
	memcpy(&pAddr->sipx_network, netnum, 4);
#endif
	for(i = 0; i < 6; i++)
		pAddr->sipx_node[i]		= nodenum[i];
	pAddr->sipx_port			= htons(m_port);

	if((m_hSock = socket(AF_IPX, SOCK_DGRAM, PF_IPX)) != INVALID_SOCKET)
	{
		SetSocketOptions();
	}
	else
	{
		int err = GetLastError();

		closesocket(m_hSock);
		m_hSock = INVALID_SOCKET;
		return errSOCKET_OPEN_FAILURE;
	}
	m_bOpen = true;
	return errOK;
}

//***********************************************************************
//***********************************************************************
//***********************************************************************
// io bytestream class based on connectionles ipx (IP datagram) socket
//
BspxStream::BspxStream(bool nbio) : BipxStream(nbio)
{
}

//***********************************************************************
BspxStream::~BspxStream()
{
}

//***********************************************************************
ERRCODE BspxStream::Open(BYTE netnum[4], BYTE nodenum[6], short port)
{
	ERRCODE ec;
	char    raddr[64];
	int		i;
	struct sockaddr_ipx* pAddr;

	if(m_bOpen) return errOK;

	// form char version of net address
	//
    sprintf(raddr, "%02X%02X%02X%02X : %02X-%02X-%02X-%02X-%02X-%02X : %d",
		netnum[0], netnum[1], netnum[2], netnum[3],
		nodenum[0], nodenum[1], nodenum[2], nodenum[3], nodenum[4],	nodenum[5],
		port);

	// open base socket class
	//	
	ec = BsocketStream::Open(raddr, port);
	if(ec != errOK) return ec;

	memset(m_pHostAddr, 0, sizeof(struct sockaddr_ipx));
	pAddr = (struct sockaddr_ipx*)m_pHostAddr;

	pAddr->sipx_family			= AF_IPX;
#ifdef Windows
	for(i = 0; i < 4; i++)
		pAddr->sipx_network[i]	= netnum[i];
#else
	memcpy(&pAddr->sipx_network, netnum, 4);
#endif
	for(i = 0; i < 6; i++)
		pAddr->sipx_node[i]		= nodenum[i];
	pAddr->sipx_port			= htons(m_port);

	if((m_hSock = socket(AF_IPX, SOCK_DGRAM, PF_SPX)) != INVALID_SOCKET)
	{
		SetSocketOptions();
	}
	else
	{
		int err = GetLastError();

		closesocket(m_hSock);
		m_hSock = INVALID_SOCKET;
		return errSOCKET_OPEN_FAILURE;
	}
	m_bOpen = true;
	return errOK;
}

#endif

//***********************************************************************
//***********************************************************************
//***********************************************************************
// io bytestream class based on buffered file I/O
//
BfileStream::BfileStream()
	:
	m_pFile(NULL)
{
}

//***********************************************************************
BfileStream::~BfileStream()
{
	Close();
}

//***********************************************************************
FILE* bsafopen(LPCTSTR pName, LPCTSTR pAccess)
{
#if defined(Windows)
	return _tfopen(pName, pAccess);
#else
#ifdef UNICODE
	char* pname   = new char [ (_tcslen(pName) + 2) * sizeof(WCHAR) ];
	char* paccess = new char [ (_tcslen(pAccess) + 2) * sizeof(WCHAR) ];
	BUtil::WCharToChar(pname, pName);
	BUtil::WCharToChar(paccess, pAccess);
	
	FILE* pFile = fopen(pname, paccess);
	
	delete [] pname;
	delete [] paccess;
#else
	FILE* pFile = fopen(pName, pAccess);
#endif
	return pFile;
#endif
}

//***********************************************************************
ERRCODE BfileStream::Open(LPCTSTR pName, LPCTSTR pAccess)
{
	LPCTSTR astr;
	bool   crf = false;

	if(m_bOpen) return errOK;

	if(_tcsstr(pAccess, _T("rw")))
	{
		astr = _T("rb+");
		m_access = strmReadWrite;
		crf  = true;
	}
	else if(_tcsstr(pAccess, _T("w")))
	{
		if(_tcsstr(pAccess, _T("+")))
			astr = _T("wb+");
		else
			astr = _T("wb");
		m_access = strmWrite;
	}
	else if(_tcsstr(pAccess, _T("r")))
	{
		if(_tcsstr(pAccess, _T("+")))
		{
			astr = _T("rb+");
			m_access = strmReadWrite;
		}
		else
		{
			astr = _T("rb");
			m_access = strmRead;
		}
		astr = _T("rb");
	}
	else
	{
		return errBAD_PARAMETER;
	}

	m_pFile = bsafopen(pName, astr);

	if(! m_pFile && crf)
	{
		m_pFile = bsafopen(pName, _T("wb+"));
	}
	if(! m_pFile)
	{
		return errSTREAM_OPEN;
	}
	m_bOpen = true;

	return errOK;
}

//***********************************************************************
ERRCODE BfileStream::Close(void)
{
	if(m_bOpen && m_pFile)
	{
		fflush(m_pFile);
		fclose(m_pFile);
	}
	m_bOpen = false;
	m_pFile = NULL;
	return errOK;
}

//***********************************************************************
ERRCODE BfileStream::Read(BYTE* pBuf, int& cnt)
{
	int		ret;

	if(! m_bOpen) return errSTREAM_NOT_OPEN;

	ret = fread(pBuf, 1, cnt,  m_pFile);
	cnt = ret;

	if(ret < 0)
	{
		cnt = 0;
		return errSTREAM_READ;
	}
	return errOK;
}

//***********************************************************************
ERRCODE BfileStream::Write(BYTE* pBuf, int& cnt)
{
	int		ret;

	if(! m_bOpen) return errSTREAM_NOT_OPEN;

	ret = fwrite(pBuf, 1, cnt, m_pFile);
	cnt = ret;

	if(ret != cnt)
	{
		return errSTREAM_WRITE;
	}
	return errOK;
}

//***********************************************************************
ERRCODE BfileStream::Pend(int to_secs, int to_usecs)
{
	int	sv;

	if(! m_bOpen) return errSTREAM_NOT_OPEN;

	sv = feof(m_pFile);

	if(sv == 0)
		return errSTREAM_DATA_PENDING;
	else
		return errOK;
}

//***********************************************************************
ERRCODE BfileStream::Flush()
{
	if(! m_bOpen) return errSTREAM_NOT_OPEN;

	fflush(m_pFile);
	return errOK;
}

//***********************************************************************
ERRCODE BfileStream::Rewind()
{
	if(! m_bOpen) return errSTREAM_NOT_OPEN;

	fseek(m_pFile, 0L, SEEK_SET);
	return errOK;
}

//***********************************************************************
//***********************************************************************
//***********************************************************************
// io bytestream wrapper class for buffered text file I/O
//
BtextStream::BtextStream()
	:
	m_rdCnt(0),
	m_rdDex(0),
	m_wrCnt(0),
	m_wrDex(0),
	m_encoding(txtANSI)
{
}

//***********************************************************************
BtextStream::~BtextStream()
{
	Close();
	// leaves stream member open!
}


//***********************************************************************
ERRCODE BtextStream::Open(Bstream* pStream, TEXTENCODING encoding)
{
	ERRCODE ec;
	int		rc;
	long	fudge;

	if(! pStream) return errBAD_PARAMETER;

	m_pStream = pStream;
	m_encoding = encoding;

	m_rdCnt = 0;
	m_rdDex = 0;
	m_wrCnt = 0;
	m_wrDex = 0;

	// if the stream underneath is open for read or r/w, decode the encoding
	// else set the encoding
	//
	STREAMACCESS access;

	ec = pStream->GetAccess(access);
	if(ec != errOK) return ec;

	if(access == strmRead)
	{
		// determine encoding by looking for a BOM (byte order mark)
		//
		rc = sizeof(m_rdBuf);
		ec = m_pStream->Read(m_rdBuf, rc);
		if(ec != errOK) return ec;

		if(rc >= 0)
			m_rdCnt = rc;
		else
			m_rdCnt = 0;

		if(rc >= 2)
		{
			if((BYTE)m_rdBuf[0] == 0xFE && (BYTE)m_rdBuf[1] == 0xFF)
			{
				m_encoding = txtUCS2; // motorola utf-16 format (?)
			}
			else if((BYTE)m_rdBuf[0] == 0xFF && (BYTE)m_rdBuf[1] == 0xFE)
			{
				if(rc >= 4 && ((BYTE)m_rdBuf[2] == 0x0 && (BYTE)m_rdBuf[3] == 0x0))
					m_encoding = txtUCS4SWAP; // utf-32 on intel (e.g. linux i386)
				else if(rc >= 4)
					m_encoding = txtUCS2SWAP; // utf-16 on intel (e.g. windows)
				else
					m_encoding = txtANSI;
			}
			else if((BYTE)m_rdBuf[0] == 0x0 && (BYTE)m_rdBuf[1] == 0x0)
			{
				if(rc >= 4 && ((BYTE)m_rdBuf[2] == 0xFE && (BYTE)m_rdBuf[3] == 0xFF))
					m_encoding = txtUCS4; // utf-32 on motorola (linux ppc)
				else
					m_encoding = txtANSI;
			}
			else if((BYTE)m_rdBuf[0] == 0xEF)
			{
				if(rc >= 3 && ((BYTE)m_rdBuf[1] == 0xBB && (BYTE)m_rdBuf[2] == 0xBF))
					m_encoding = txtUTF8;
				else
					m_encoding = txtANSI;
			}
		}
		switch(m_encoding)
		{
		case txtANSI:		fudge = 0;	break;
		case txtUTF8:		fudge = 3;	break;
		case txtUCS2:		fudge = 2;	break;
		case txtUCS2SWAP:	fudge = 2;	break;
		case txtUCS4:		fudge = 4;	break;
		case txtUCS4SWAP:	fudge = 4;	break;
		default:			fudge = 0;  break;
			break;
		}
		// skip over BOM
		m_rdDex += fudge;
	}
	else
	{
		switch(m_encoding)
		{
		case txtANSI:
			break;
		case txtUCS2:
			PutByte(0xFE);
			PutByte(0xFF);
			break;
		case txtUCS2SWAP:
			PutByte(0xFF);
			PutByte(0xFE);
			break;
		case txtUCS4SWAP:
			PutByte(0xFF);
			PutByte(0xFE);
			PutByte(0x0);
			PutByte(0x0);
			break;
		case txtUCS4:
			PutByte(0x0);
			PutByte(0x0);
			PutByte(0xFE);
			PutByte(0xFF);
			break;
		case txtUTF8:
			PutByte(0xEF);
			PutByte(0xBB);
			PutByte(0xBF);
			break;
		}
	}
	return errOK;
}

//***********************************************************************
ERRCODE BtextStream::Close(void)
{
	if(m_wrCnt > 0)
		return m_pStream->Write(m_wrBuf, m_wrCnt);
	return errOK;
}

//***********************************************************************
ERRCODE BtextStream::Read(BYTE* pBuf, int& cnt)
{
	int avail = m_rdCnt - m_rdDex;

	if(avail > 0)
	{
		if(cnt > avail)
			cnt = avail;

		memcpy(pBuf, m_rdBuf + m_rdDex, cnt);
		m_rdDex += cnt;
		return errOK;
	}
	else
	{
		return m_pStream->Read(pBuf, cnt);
	}
}

//***********************************************************************
ERRCODE BtextStream::Write(BYTE* pBuf, int& cnt)
{
	ERRCODE ec = errOK;

	if(m_wrCnt > 0)
	{
		ec = m_pStream->Write(m_wrBuf, m_wrCnt);
		m_wrCnt = 0;
		m_wrDex = 0;
	}
	if(ec == errOK)
		return m_pStream->Write(pBuf, cnt);
	else
		return ec;
}

//***********************************************************************
ERRCODE BtextStream::Rewind()
{
	m_pStream->Rewind();

	m_rdCnt = 0;
	m_rdDex = 0;
	m_wrCnt = 0;
	m_wrDex = 0;

	return errOK;
}

//***********************************************************************
ERRCODE BtextStream::GetEncoding(TEXTENCODING& encoding)
{
	encoding = m_encoding;
	return errOK;
}

//***********************************************************************
ERRCODE BtextStream::SetEncoding(TEXTENCODING encoding)
{
	// if stream is open for read, and a BOM has been read, and
	// encoding is being set to ANSI, then reset the read pointer
	//
	if(m_pStream && m_encoding != txtANSI && encoding == txtANSI)
	{
		ERRCODE      ec;
		STREAMACCESS access;

		ec = m_pStream->GetAccess(access);
		if(m_rdDex <= 4 && access == strmRead)
			Rewind();
	}
	m_encoding = encoding;
	return errOK;
}

//***********************************************************************
ERRCODE BtextStream::GetByte(BYTE& nc)
{
	ERRCODE ec = errOK;
	int     rc;

	if(! m_pStream) return errSTREAM_NOT_OPEN;

	if(m_rdDex >= m_rdCnt)
	{
		m_rdDex = 0;
		rc = sizeof(m_rdBuf);
		ec = m_pStream->Read(m_rdBuf, rc);
		if(ec == errOK) 
		{
			m_rdCnt = rc;
			if(rc == 0)
			{
				nc = 0xFF;
				return errSTREAM_EOF;
			}
		}
		else
		{
			m_rdCnt = 0;
			nc		= 0;
			return ec;
		}
	}
	nc = m_rdBuf[m_rdDex++];
	return ec;
}

//***********************************************************************
ERRCODE BtextStream::GetChar(TCHAR& nc)
{
	ERRCODE ec = errOK;
	BYTE	a;
#ifdef UNICODE
	DWORD	b, c;
#endif

	ec = GetByte(a);
	if(ec != errOK) return ec;
	
	switch(m_encoding)
	{
	case txtANSI:
		nc = (TCHAR)a;
		break;

	case txtUTF8:
#ifndef UNICODE
		nc = (TCHAR)a;
#else
		b = (DWORD)a;

		if(b & 0x80)
		{
			if(b & 0x20)
			{
				if(b & 0x10)
				{
					b &= 0x7;
					b <<= 18;
					ec = GetByte(a);
					if(ec) break;
					c = (DWORD)a;
					b |= (c & 0x3f) << 12;
					ec = GetByte(a);
					if(ec) break;
					c = (DWORD)a;
					b |= (c & 0x3f) << 6;
					ec = GetByte(a);
					if(ec) break;
					c = (DWORD)a;
					b |= (c & 0x3f);
				}
				else
				{
					b &= 0xf;
					b <<= 12;
					ec = GetByte(a);
					if(ec) break;
					c = (DWORD)a;
					b |= (c & 0x3f) << 6;
					ec = GetByte(a);
					if(ec) break;
					c = (DWORD)a;
					b |= (c & 0x3f);
				}
			}
			else
			{
				b &= 0x1f;
				b <<= 6;
				ec = GetByte(a);
				if(ec) break;
				c = (DWORD)a;
				b |= c & 0x3f;
			}
		}
		nc = (WCHAR)b;
#endif
		break;

	case txtUCS2:
#ifndef UNICODE
		nc = (TCHAR)a;
#else
		b = (DWORD)a << 8;
		ec = GetByte(a);
		if(ec) break;
		b |= a;
		nc = (WCHAR)b;
#endif
		break;

	case txtUCS2SWAP:
#ifndef UNICODE
		nc = (TCHAR)a;
#else
		b = a;
		ec = GetByte(a);
		if(ec) break;
		b |= (DWORD)a << 8;
		nc = (WCHAR)b;
#endif
		break;

	case txtUCS4:
#ifndef UNICODE
		nc = (TCHAR)a;
#else
		if(sizeof(WCHAR) == 4)
		{
			b = (DWORD)a << 24;
			ec = GetByte(a);
			if(ec) break;
			b |= (DWORD)a << 16;
		}
		else
		{
			ec = GetByte(a);
			if(ec) break;
			b = 0;
		}
		ec = GetByte(a);
		if(ec) break;
		b |= (DWORD)a << 8;
		ec = GetByte(a);
		if(ec) break;
		b |= a;
		nc = (WCHAR)b;
#endif
		break;

	case txtUCS4SWAP:
#ifndef UNICODE
		nc = (TCHAR)a;
#else
		b = a;
		ec = GetByte(a);
		if(ec) break;
		b |= (DWORD)a << 8;
		nc = (WCHAR)b;
		ec = GetByte(a);
		if(ec) break;
		if(sizeof(WCHAR) == 4)
		{
			b = (DWORD)a << 16;
			ec = GetByte(a);
			if(ec) break;
			b |= (DWORD)a << 24;
		}
		else
		{
			ec = GetByte(a);
		}
#endif
		break;
	default:

		return errSTREAM_UNSUPPORTED_ENCODING;
	}	
	return ec;
}

//***********************************************************************
ERRCODE BtextStream::PutByte(BYTE nc)
{
	ERRCODE ec = errOK;
	int     wc;

	if(! m_pStream) return errSTREAM_NOT_OPEN;

	m_wrBuf[m_wrCnt++] = nc;

	if(m_wrCnt >= sizeof(m_wrBuf))
	{
		wc = m_wrCnt;
		ec = m_pStream->Write(m_wrBuf, wc);
		m_wrCnt = 0;
	}
	return ec;
}

//***********************************************************************
ERRCODE BtextStream::PutChar(TCHAR xc)
{
	ERRCODE ec = errOK;

	if(! m_pStream) return errSTREAM_NOT_OPEN;

	switch(m_encoding)
	{
	case txtANSI:
		ec = PutByte((BYTE)xc);
		break;

	case txtUTF8:
#ifndef UNICODE
		ec = PutByte((BYTE)xc);
#else       
        if (xc < 0x80) {
			ec = PutByte((BYTE)xc);
        }
        else if (xc < 0x800) {
			ec = PutByte(0xC0 | (xc >> 6));
			ec = PutByte(0x80 | (xc & 0x3F));
        }
        else if (xc < 0x10000) {
			ec = PutByte(0xE0 | (xc >> 12));
			ec = PutByte(0x80 | ((xc >> 6) & 0x3F));
			ec = PutByte(0x80 | (xc & 0x3F));
        }
		// this part only happens if sizeof(TCHAR) > 2 (linux, etc)
        else if (xc < 0x200000) {
			ec = PutByte(0xF0 | (xc >> 18));
			ec = PutByte(0xE0 | ((xc >> 12) & 0x3F));
			ec = PutByte(0x80 | ((xc >> 6) & 0x3F));
			ec = PutByte(0x80 | (xc & 0x3F));
        }
#endif
		break;

	case txtUCS2SWAP:
#ifndef UNICODE
		ec = PutByte((BYTE)xc);
#else
		ec = PutByte((BYTE)xc);
		if(ec) break;
		ec = PutByte((BYTE)((WORD)xc >> 8));
#endif
		break;

	case txtUCS2:
#ifndef UNICODE
		ec = PutByte((BYTE)xc);
#else
		ec = PutByte((BYTE)((WORD)xc >> 8));
		if(ec) break;
		ec = PutByte((BYTE)xc);
#endif
		break;

	case txtUCS4SWAP:
#ifndef UNICODE
		ec = PutByte((BYTE)xc);
#else
		ec = PutByte((BYTE)xc);
		if(ec) break;
		ec = PutByte((BYTE)((WORD)xc >> 8));
		if(ec) break;
		if(sizeof(WCHAR) == 4)
		{
			ec = PutByte((BYTE)((WORD)xc >> 16));
			if(ec) break;
			ec = PutByte((BYTE)((WORD)xc >> 24));
		}
		else
		{
			ec = PutByte(0);
			if(ec) break;
			ec = PutByte(0);
		}
#endif
		break;

	case txtUCS4:
#ifndef UNICODE
		ec = PutByte((BYTE)xc);
#else
		if(sizeof(WCHAR) == 4)
		{
			ec = PutByte(0);
			if(ec) break;
			ec = PutByte(0);
			if(ec) break;
		}
		else
		{
			ec = PutByte((BYTE)((WORD)xc >> 24));
			if(ec) break;
			ec = PutByte((BYTE)((WORD)xc >> 16));
			if(ec) break;
		}
		ec = PutByte((BYTE)((WORD)xc >> 8));
		if(ec) break;
		ec = PutByte((BYTE)xc);
#endif
		break;

	default:

		return errSTREAM_UNSUPPORTED_ENCODING;
	}	
	return ec;
}

//***********************************************************************
ERRCODE BtextStream::Pend(int to_secs, int to_usecs)
{
	if(m_pStream) return m_pStream->Pend(to_secs, to_usecs);
	return errSTREAM_NOT_OPEN;
}


//***********************************************************************
//***********************************************************************
//***********************************************************************
// wrapper to turn text arrays into bytestreams
//
BtextStreamWrapper::BtextStreamWrapper()
	:
	m_rdCnt(0),
	m_rdDex(0),
	m_wrDex(0),
	m_encoding(txtANSI)
{
}

//***********************************************************************
BtextStreamWrapper::~BtextStreamWrapper()
{
	Close();
}


//***********************************************************************
ERRCODE BtextStreamWrapper::Open(char* pText)
{
	m_pText    = pText;
	m_encoding = txtANSI;

	m_rdCnt = strlen(pText);
	m_rdDex = 0;
	m_wrDex = 0;
	return errOK;
}

//***********************************************************************
ERRCODE BtextStreamWrapper::Close(void)
{
	return errOK;
}

//***********************************************************************
ERRCODE BtextStreamWrapper::Read(BYTE* pBuf, int& cnt)
{
	if(m_rdDex >= m_rdCnt)
	{
		cnt = 0;
		return errSTREAM_EOF;
	}
	if(cnt > (m_rdCnt - m_rdDex))
		cnt = (m_rdCnt - m_rdDex);

	memcpy(pBuf, m_pText + m_rdDex, cnt);
	m_rdDex += cnt;
	return errOK;
}

//***********************************************************************
ERRCODE BtextStreamWrapper::Write(BYTE* pBuf, int& cnt)
{
	int	room = m_rdCnt - m_wrDex;

	if(cnt > room)
		cnt = room;
	memcpy(m_pText + m_wrDex, pBuf, cnt);
	m_wrDex += cnt;
	return errOK;
}

//***********************************************************************
ERRCODE BtextStreamWrapper::Rewind()
{
	m_rdDex = 0;
	m_wrDex = 0;
	return errOK;
}

//***********************************************************************
ERRCODE BtextStreamWrapper::GetByte(BYTE& nc)
{
	ERRCODE ec = errOK;

	if(m_rdDex >= m_rdCnt)
	{
		nc = 0xFF;
		return errSTREAM_EOF;
	}
	else
	{
		nc = m_pText[m_rdDex++];
	}
	return ec;
}

//***********************************************************************
ERRCODE BtextStreamWrapper::GetChar(TCHAR& nc)
{
	ERRCODE ec = errOK;
	BYTE	a;

	ec = GetByte(a);
	if(ec != errOK) return ec;
	
	nc = (TCHAR)a;
	return ec;
}

//***********************************************************************
ERRCODE BtextStreamWrapper::PutByte(BYTE nc)
{
	if(m_wrDex < m_rdCnt)
	{
		m_pText[m_wrDex++] = nc;
		return errOK;
	}
	return errSTREAM_EOF;
}

//***********************************************************************
ERRCODE BtextStreamWrapper::PutChar(TCHAR xc)
{
	return PutByte((BYTE)xc);
}

