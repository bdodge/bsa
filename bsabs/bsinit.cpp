
#include "bsx.h"
#ifdef Windows
#include <rpc.h>
#endif

bool   BsocketStream::m_isinited = false;
DWORD* BsocketStream::m_inet_addr = NULL;
DWORD* BsocketStream::m_inet_bcast_addr = NULL;
//TCHAR  BsocketStream::m_inet_mac_addr[16*BS_MAX_NICS];
int    BsocketStream::m_numInterfaces = 0;

LPCSTR  BsocketStream::m_protocolsSupported[] =
		{
		"tcpip"
		};

#define BS_MAX_NET_INTERFACES	BS_MAX_NICS
#define BS_MAX_ADAPTER_NAME		256
#define BS_MAX_DEVICE_NAME		256
#define BS_MAX_BIND_LEN			256

#ifdef Windows
DWORD _ina_b[8];
DWORD _inb_b[8];
TCHAR _inm_b[16*8];
#endif

//***********************************************************************
ERRCODE BsocketStream::Init(void)
{
#ifdef Windows
	WORD	wVersionRequested;
	WSADATA wsaData;
	int		err;
	char	hostname[256];
	struct hostent* phost;

	if(m_isinited)
		return errOK;
	m_isinited = true;

	// init wsa
	wVersionRequested = MAKEWORD( 1, 1 );
	err = WSAStartup( wVersionRequested, &wsaData );

	m_inet_addr		  = _ina_b;
	m_inet_bcast_addr = _inb_b;

	// setup IP address
	m_numInterfaces = 0;

	if(1 || WIN9X)
	{
		err = gethostname(hostname, sizeof(hostname));
		if(err < 0) return errSOCKET_NO_INTERFACE;

		phost = gethostbyname(hostname);
		if(! phost) return errSOCKET_NO_INTERFACE;

		memcpy(&m_inet_addr[m_numInterfaces], phost->h_addr, 4);
		memcpy(&m_inet_bcast_addr[m_numInterfaces], phost->h_addr, 4);
		m_inet_bcast_addr[m_numInterfaces] |= htonl(255);

		m_numInterfaces++;
	}
	else
	{
#if 0
		IP_ADAPTERS_INFO* pInfo;
		DWORD rv, cnt;
		int   cbInfo;

		m_numInterfaces = 0;
		rv = GetNumberOfInterfaces(&cnt);

		if(cnt <= 0 || rv != NO_ERROR)
		{
			m_numInterfaces = 0;
			return errSOCKET_NO_INTERFACE;
		}
		cbInfo = cnt * sizeof(PIP_ADAPTERS_INFO);
		pInfo = new IP_ADAPTERS_INFO [cnt];
		rv = GetAdapterInfo(pInfo, &cbInfo);
		if(rv != NO_ERROR)
		{
			return errSOCKET_NO_INTERFACE;
		}
		for(n = 0; n < cnt; n++)
		{
			pInfo[n]. 
		}
#endif
	}
#elif defined(Linux)

	struct sockaddr_in *sin;
	struct ifreq* ifr;
	struct ifconf ifc;
	
	int	rv;
	int fd;
	
	int maxreqs = 30;
	int n;
	
	bool gotifcs;

	if(m_isinited)
		return errOK;
	m_isinited = true;
	
	// opens udp socket
	fd = socket(PF_INET, SOCK_DGRAM, 0);
	
	if (fd < 0)
		return errSOCKET_OPEN_FAILURE;
	
	memset(&ifc, 0, sizeof(ifc));
	ifc.ifc_buf = NULL;

	maxreqs = BS_MAX_NET_INTERFACES;
	gotifcs = false;

	ifc.ifc_len = sizeof(struct ifreq) * maxreqs;
	if(ifc.ifc_buf)
		delete [] ifc.ifc_buf;
	ifc.ifc_buf = new char [ ifc.ifc_len ];
	
	if((rv = ioctl(fd, SIOCGIFCONF, &ifc)) >= 0)
	{
		if(ifc.ifc_len < sizeof(struct ifreq) * maxreqs)
		{
			gotifcs = true;
		}
	}		
	if(! gotifcs)
	{
		close(fd);
		delete [] ifc.ifc_buf;
		return errSOCKET_IOCTL_FAILURE;
	}
	
	// if no interfaces, boot
	//
	m_numInterfaces = 0;
	if(ifc.ifc_len <= 0)
	{
		delete [] ifc.ifc_buf;
		close(fd);
		return errSOCKET_NO_INTERFACE;
	}
		
	ifr = ifc.ifc_req;
	
	m_inet_addr = new DWORD [ ifc.ifc_len / sizeof(struct ifreq) ];
	m_inet_bcast_addr = new DWORD [ ifc.ifc_len / sizeof(struct ifreq) ];
	
	for(n = 0; n < ifc.ifc_len; n+= sizeof(struct ifreq), ifr++)
	{
		// skip local interfaces, we know their IP
		//
		if(! strcmp(ifr->ifr_name, "lo"))
			continue;

		m_inet_addr[m_numInterfaces] = 0xffffffff;
		m_inet_bcast_addr[m_numInterfaces] = 0xffffffff;		
		
		// Get the flags for this interface
		//
		rv = ioctl(fd, SIOCGIFFLAGS, ifr);
		
		// Get the IP Address for this interface 
		//
		rv = ioctl(fd, SIOCGIFDSTADDR, ifr);
		if (rv == 0 )
		{
			if (ifr->ifr_broadaddr.sa_family == AF_INET)
			{
				sin = (struct sockaddr_in *)&ifr->ifr_dstaddr;
				m_inet_addr[m_numInterfaces] = sin->sin_addr.s_addr;
			}
		}
		
		// Get the BROADCAST address 
		//
		rv = ioctl(fd, SIOCGIFBRDADDR, ifr);
		if(rv == 0)
		{
			if (ifr->ifr_broadaddr.sa_family == AF_INET)
			{			
				sin = (struct sockaddr_in *)&ifr->ifr_broadaddr;
				m_inet_bcast_addr[m_numInterfaces] = sin->sin_addr.s_addr;
			}
		}
		m_numInterfaces++;
	}
	delete [] ifc.ifc_buf;
	close(fd);
#endif	
	return errOK;
}

//***********************************************************************
ERRCODE BsocketStream::GetIP(DWORD& ipAddress, int inter)
{
	if(inter < 0 || inter >= m_numInterfaces)
		return errBAD_PARAMETER;
	ipAddress = m_inet_addr[inter];
	return errOK;
}

//***********************************************************************
ERRCODE BsocketStream::GetBroadcastIP(DWORD& ipAddress, int inter)
{
	if(inter < 0 || inter >= m_numInterfaces)
		return errBAD_PARAMETER;
	ipAddress = m_inet_bcast_addr[inter];
	return errOK;
}



