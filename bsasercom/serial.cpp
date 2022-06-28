#include "serialx.h"

#ifndef Windows
static long  unixBaud(long rate);
static int	 unixBits(int bits);
static int	 unixStopBits(int sbits);
static int	 unixParity(int par);
#ifdef SERCOM_USEPOLL
#include <poll.h>
#endif
#endif // ndef Windows

//***********************************************************************
BserialStream::BserialStream()
			:
			Bstream(),
			m_baud(9600),
			m_bits(8),
			m_stops(1),
			m_parity(0)
{
#ifdef Windows
	m_osport		= INVALID_HANDLE_VALUE;
	m_read_pending  = false;
	m_write_pending = false;
	_tcscpy(m_portname, _T("COM1"));
#else
	m_osport = -1;
	_tcscpy(m_portname, _T("/dev/ttyS0"));
#endif
	m_port = 0;
}

//***********************************************************************
BserialStream::~BserialStream()
{
	if(m_bOpen)
		Close();
}

static int m_flow;

//***********************************************************************
ERRCODE BserialStream::Open(LPCTSTR pPort, int baud, int bits, int stops, int parity, int flow)
{
#ifdef Windows
	COMMTIMEOUTS	timeouts;
	DCB				dev;
	TCHAR			portparms[256];
	int				rv;
	BOOL			bSet;

	m_baud		= baud;
	m_bits		= bits;
	m_stops		= stops;
	m_parity	= parity;

	m_read_pending  = false;
	m_write_pending = false;

	// form port name from port canonical number
	//
	if (pPort[0] >= '0' && pPort[0] <= '9')
	{
		m_port = GetIndexOfPort(pPort);
	}
	else
	{
		m_port = BSASERCOM_USERPORT;
	}
	if(m_port != BSASERCOM_USERPORT)
	{
		_sntprintf(m_portname, 128, _T("COM%d"), m_port + 1);
	}
	else
	{
		LPTSTR pc;

		_tcsncpy(m_portname, pPort, MAX_PATH);
		pc = _tcsstr(m_portname, _T(":"));
		if (pc)
		{
			*pc = 0;
		}
	}
	m_osport = CreateFile(
							m_portname,
							GENERIC_READ | GENERIC_WRITE,
							0,
							NULL,
							OPEN_EXISTING,
							/*FILE_ATTRIBUTE_NORMAL |*/ FILE_FLAG_OVERLAPPED,
							NULL
						);
	if(m_osport == INVALID_HANDLE_VALUE)
	{
		return errSTREAM_OPEN;
	}
	m_bOpen = true;

	// setup port timeout
	//
	memset(&timeouts, 0, sizeof(timeouts));
	timeouts.ReadIntervalTimeout		 = 0xFFFFFFFF;
	timeouts.ReadTotalTimeoutMultiplier  = 0;
	timeouts.ReadTotalTimeoutConstant	 = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant	 = 500;
	SetCommTimeouts(m_osport, &timeouts);

	// build port parms string
	//
	_sntprintf(portparms, 256, _T("baud=%d parity=%c data=%d stop=%d"),
			m_baud, m_parity ? m_parity == 2 ? 'E' : 'O' : 'N',
			m_bits, m_stops);

	dev.DCBlength = sizeof(DCB);
	GetCommState(m_osport, &dev);

	dev.BaudRate		= m_baud;
	dev.ByteSize		= m_bits;
	dev.Parity			= m_parity;
	dev.fParity			= m_parity != 0 ? 1 : 0;
	dev.StopBits		= m_stops - 1;
	dev.fDsrSensitivity = 0;
	dev.fAbortOnError	= 0;

	// setup h/w flow control
	//
	bSet = (flow & 1) != 0;
	dev.fOutxDsrFlow = bSet;
	if (bSet)
		dev.fDtrControl = DTR_CONTROL_HANDSHAKE;
	else
		dev.fDtrControl = DTR_CONTROL_ENABLE;

	bSet = (flow & 1) != 0;; //(BYTE) ((FLOWCTRL( npTTYInfo) & FC_RTSCTS) != 0);
	dev.fOutxCtsFlow = bSet;
	if (bSet)
		dev.fRtsControl = RTS_CONTROL_HANDSHAKE;
	else
		dev.fRtsControl = RTS_CONTROL_ENABLE;

	// setup s/w flow control
	//
	bSet = (flow & 2) != 0;
	dev.fInX		= dev.fOutX = bSet;
	dev.XonChar		= 0x11;
	dev.XoffChar	= 0x13;
	dev.XonLim		= 100;
	dev.XoffLim		= 100;

	// other various settings
	dev.fBinary = TRUE;

	dev.DCBlength = sizeof(DCB);
	rv = SetCommState(m_osport, &dev);
	rv = GetLastError();

	rv = SetCommMask(m_osport, EV_RXCHAR);
	rv = GetLastError();

    SetupComm(m_osport, 4096, 4096);

	// create event for overlapped I/O
	//
#else // Windows

	struct  termios termctl;
	int		modemlines;
	char	portname[MAX_PATH];
	int		nbio, rv;

	m_baud		= baud;
	m_bits		= bits;
	m_stops		= stops;
	m_parity	= parity;
	m_flow		= flow;

	// form port name from port canonical number
	//
	if (pPort[0] >= '0' && pPort[0] <= '9')
	{
		m_port = GetIndexOfPort(pPort);
	}
	else
	{
		m_port = BSASERCOM_USERPORT;
	}
	if(m_port != BSASERCOM_USERPORT)
		GetActualPortName(m_port, m_portname, MAX_PATH);
	else
		_tcsncpy(m_portname, pPort, MAX_PATH);
	if(m_port < 0)
	{
		return errBAD_PARAMETER;
	}
	TCharToChar(portname, m_portname);

#if defined(O_NDELAY) && defined(F_SETFL)
	m_osport = open(portname, O_RDWR | O_NDELAY | O_NOCTTY);
	if (m_osport >= 0) {
		/* Cancel the O_NDELAY flag. */
		rv = fcntl(m_osport, F_GETFL, 0);
		fcntl(m_osport, F_SETFL, rv & ~O_NDELAY);
	}
	//printf("open %s = %d  %d\n", portname, m_osport, errno);
#else
	m_osport = open(portname, O_RDWR | O_NOCTTY);
#endif
	if(m_osport <= 0)
	{
		return errSTREAM_OPEN;
	}
	nbio = 1;
	ioctl(m_osport, FIONBIO, &nbio);
	m_bOpen = true;

	/*	fcntl(comm_port, F_SETFL, FNDELAY); */

	// get current port settings
	//
	rv = tcgetattr(m_osport, &termctl);
	if(rv < 0)
		rv = errno;
	termctl.c_iflag = /*ICRNL |*/ IGNBRK | ((flow & 2) ? (IXON|IXOFF) : IXANY);
	termctl.c_oflag = 0; /* ONLCR */;

	termctl.c_cflag =	0 /*unixBaud(m_baud)*/	| /* use cfset[io]speeed instead of bits here */
						unixBits(m_bits)		|
						unixStopBits(m_stops)	|
						unixParity(m_parity);

	termctl.c_cflag |= CREAD | CLOCAL | CS8;

    cfsetospeed(&termctl, (speed_t)unixBaud(m_baud));
    cfsetispeed(&termctl, (speed_t)unixBaud(m_baud));

	if(flow & 1)
		termctl.c_cflag |= CRTSCTS;

	termctl.c_lflag = 0L;
#if !defined(Darwin) && !defined(OSX)
	termctl.c_line	= 0;
#endif
	termctl.c_cc[VMIN]	= 1;
	termctl.c_cc[VTIME] = 0;

	// set out settings back to port
	rv = tcsetattr(m_osport, 0,  &termctl);
	if(rv < 0)
		rv = errno;
#ifdef HPUX
#else
	// set RTS and DTR
	modemlines = TIOCM_RTS | TIOCM_DTR;
	ioctl(m_osport, TIOCMBIS, &modemlines);
#endif

#endif	// Windows

	return errOK;
}

//***********************************************************************
ERRCODE BserialStream::Pend(int to_secs, int to_usecs)
{
#ifdef Windows
	int		sv;
	DWORD	mask;
    COMSTAT comStat;
    DWORD   dwErrors;

	if(m_read_pending)
		return errSTREAM_DATA_PENDING;
#if 0
	sv = WaitCommEvent(m_osport, &mask, NULL);
#else

	int toms = to_usecs / 1000;

	mask = 0;
	toms += to_secs * 1000;

	do
	{
		if(ClearCommError(m_osport, &dwErrors, &comStat))
		{
			sv = 1;
			if(comStat.cbInQue > 0)
			{
				mask = EV_RXCHAR;
			}
			else if(toms > 0)
			{
				int st = min(100, (toms / 100));

				st = max(100, st);

				Sleep(st);
				toms -= st;
				mask = 0;
			}
		}
		else
		{
			return errFAILURE;
		}
	}
	while(! (mask & EV_RXCHAR) && (toms > 0));

	ClearCommError(m_osport, &dwErrors, NULL);
#endif
	if(sv)
	{
		if(mask & EV_RXCHAR)
			return errSTREAM_DATA_PENDING;
	}
	else
	{
		sv = GetLastError();
		mask = sv;
		return errFAILURE;
	}
	return errOK;
#else
	int		sv;

#ifndef SERCOM_USEPOLL
	fd_set  rfds;

	struct  timeval timeout;

	// for hot-plugable ports, always report readable, even
	// if port's not open, so the read gets called and tries
	//
	if(! m_bOpen || m_osport < 0)
	{
		Open(m_portname, m_baud, m_bits, m_stops, m_parity, m_flow);
		//return errSTREAM_NOT_OPEN;
		return errOK;
	}

	FD_ZERO (&rfds);
	FD_SET  (m_osport, &rfds);

	timeout.tv_sec  = to_secs;
	timeout.tv_usec = to_usecs;

	sv = select(m_osport + 1, &rfds, NULL, NULL, (to_secs >= 0 || to_usecs >= 0) ? &timeout : NULL);
#else
	struct pollfd fds;
	int to = to_secs * 1000 + ((to_usecs + 999) / 1000);

	fds.fd = m_osport;
	fds.events = POLLIN;

	sv = poll(&fds, 1, to);
#endif
	if (sv < 0)
		if(errno == EINTR)
			sv = 0;
	if(sv < 0)
		return errSOCKET_SELECT_FAILURE;
	else if(sv > 0)
		return errSTREAM_DATA_PENDING;
	else
		return errOK;
#endif
}

//***********************************************************************
ERRCODE BserialStream::Read(LPBYTE pBuf, int& cnt)
{
#ifdef Windows
	COMSTAT ComStat;
	DWORD   dwErrorFlags;
	int		rv;
	DWORD	nRead;

	nRead = 0;

	if(! m_read_pending)
	{
		memset(&m_roverlap, 0, sizeof(OVERLAPPED));

		// no read pending, start an overlapped readfile on the port
		//
		if (ClearCommError(m_osport, &dwErrorFlags, &ComStat))
		{
			rv = ComStat.cbInQue;
			if (! rv)
			{
				cnt = 0;
				return errOK;
			}
			if (rv < cnt)
			{
				cnt = rv;
			}
		}
		else
		{
			nRead = 0;
			return errFAILURE;
		}
		rv = ReadFile(m_osport, pBuf, cnt, &nRead, &m_roverlap);
		if(! rv)
		{
			rv = GetLastError();
			if(rv == ERROR_IO_PENDING || rv == ERROR_IO_INCOMPLETE)
			{
				m_read_pending = true;
				cnt = 0;
				return errOK;
			}
			else
			{
				cnt = 0;
				return errSTREAM_READ;
			}
		}
		else
		{
			cnt = nRead;
			return errOK;
		}
	}
	else
	{
		// there is a read pending, see if it finished
		//
		rv = GetOverlappedResult(m_osport, &m_roverlap,	&nRead, FALSE);
		if(! rv)
		{
			cnt = 0;
			rv = GetLastError();
			if(rv == ERROR_IO_PENDING || ERROR_IO_INCOMPLETE)
			{
				m_read_pending = true;
				return errOK;
			}
			CancelIo(m_osport);
			ClearCommError(m_osport, &dwErrorFlags, &ComStat);
			m_read_pending = false;
			return errSTREAM_READ;
		}
		else
		{
			// read finished
			CancelIo(m_osport);
			m_read_pending = false;
		}
	}

#else // not Windows

	int nRead;

	if (m_osport < 0)
		return errSTREAM_NOT_OPEN;

	nRead = read(m_osport, (char*)pBuf, cnt);
	if(nRead < 0)
	{
		if (m_bOpen)
		{
			Close();
		}
		cnt = 0;
		return errSTREAM_READ;
	}
	else if(nRead == 0)
	{
		// read of 0 after poll of 1 means stream closed, attempt to re-open it
		// next time in the pend call
		//
		if (m_bOpen)
		{
			Close();
		}
		cnt = 0;
	}
	else
	{
		#if 0
		//pBuf[nRead] = 0;
		for (int i = 0; i < nRead; i++)
		{
			printf("r %02X  %c\n", pBuf[i], pBuf[i]);
		}
		//printf("Read %d=%s=\n", nRead, pBuf);
		#endif
	}
#endif

	if(nRead > 0 && m_bits < 8)
	{
		// clean up upper bits since windows doesn't
		//
		int		i;
		LPBYTE	p;
		BYTE	m;

		for(i = 0, m = (0xFF >> (8 - m_bits)), p = pBuf; i < (int)nRead; i++, p++)
		{
			*p &= m;
		}
	}
	cnt = nRead;
	return errOK;
}

//***********************************************************************
ERRCODE BserialStream::Write(LPBYTE pBuf, int& cnt)
{
	int   rv;
	DWORD nWrote;

#ifdef Windows
	if(m_write_pending)
	{
		rv = GetOverlappedResult(
							m_osport,
							&m_woverlap,
							&nWrote,
							FALSE
							);
		if(! rv)
		{
			rv = GetLastError();

			if(rv != ERROR_IO_PENDING)
			{
				return errSTREAM_WRITE;
			}
			cnt = 0;
			return errOK;
		}
		else
		{
			m_write_pending = false;
		}
	}
	m_woverlap.Offset = m_woverlap.OffsetHigh = 0;
	m_woverlap.hEvent = NULL;
	rv = WriteFile(m_osport, pBuf, cnt, &nWrote, &m_woverlap);
	if(rv == 0)
	{
		cnt = 0;
		rv = GetLastError();
		if(rv == ERROR_IO_PENDING)
		{
			m_write_pending = true;
			return errOK;
		}
		else
		{
			return errSTREAM_WRITE;
		}
	}
#else
	int wc;

	if (m_osport < 0)
		return errSTREAM_NOT_OPEN;

	for(nWrote = 0; nWrote < cnt;)
	{
		wc = write(m_osport, pBuf + nWrote, cnt - nWrote);
		//printf("Write %d=%s=\n", wc, pBuf + nWrote);
		if (wc < 0)
			return errSTREAM_WRITE;
		nWrote += wc;
	}
#endif
	cnt = nWrote;
	return errOK;
}

//***********************************************************************
ERRCODE BserialStream::Close()
{
#ifdef Windows
	if(m_osport != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_osport);
		m_osport = INVALID_HANDLE_VALUE;
	}
#else
	if(m_osport >= 0)
	{
		close(m_osport);
		m_osport = -1;
	}
#endif
	m_bOpen = false;
	return errOK;
}

// STATIC
//**************************************************************************
LPCTSTR BserialStream::GetActualPortName(int port, LPTSTR portbuf, int npb)
{
	if(! portbuf) return _T("No Buffer");
#ifdef Windows
	if(port != BSASERCOM_USERPORT)
	{
		port = port + 1;
		_sntprintf(portbuf, npb, _T("COM%d:"), port);
	}
	else
	{
		_sntprintf(portbuf, npb, _T("COM%d:"), port);
	}
#elif defined(Linux) || defined(OSX)
	if(port < 64)
	{
		_sntprintf(portbuf, npb, _T("/dev/ttyS%d"), port);
	}
	else if(port < 128)
	{
		_sntprintf(portbuf, npb, _T("/dev/ttyUSB%d"), port - 64);
	}
	else if (port == BSASERCOM_USERPORT)
	{
		_sntprintf(portbuf, npb, _T("custom"));
	}
#else
	_sntprintf(portbuf, npb, _T("/dev/tty%c"), (port - 1 + 'a'));
#endif
	return portbuf;
}

// STATIC
//**************************************************************************
bool BserialStream::IsSerialPort(LPCTSTR portName)
{
	if(! portName) return false;
#ifdef Windows
	if(portName[0] >= _T('0') && portName[0] <= _T('9'))
		return true;
	{
		LPCTSTR pp = _tcsstr(portName, _T("com"));
		if(! pp) pp = _tcsstr(portName, _T("COM"));
		if(! pp) pp = _tcsstr(portName, _T("Com"));

		if(pp)
			if(pp[3] >= _T('1') && pp[3] <= _T('9'))
				return true;
	}
	return false;
#elif defined(Linux)
	if(portName[0] >= _T('0') && portName[0] <= _T('9'))
		return true;
	if(! _tcsnicmp(portName, _T("/dev/tty"), 8))
		return true;
	return false;
#else
	if(portName[0] >= _T('0') && portName[0] <= _T('9'))
		return true;
	if(! _tcsnicmp(portName, _T("/dev/tty"), 8))
		return true;
	return false;
#endif
	return false;
}

// STATIC
//**************************************************************************
int BserialStream::GetIndexOfPort(LPCTSTR portName)
{
	if(! portName) return 0;
	if(! *portName) return 0;

	// short-cuts: numbers = port index (0=com1: or ttyS0, 1=com2: or ttyS1)
	//             letters = a=com1, ttyS0, etc.
	//
	if(portName[0] >= _T('0') && portName[0] <= _T('9'))
		return _tcstol(portName, NULL, 0);
	if(portName[0] >= _T('a') && portName[0] <= _T('z') && portName[1] == 0)
		return portName[0] - 'a';
	if(portName[0] >= _T('A') && portName[0] <= _T('Z') && portName[1] == 0)
		return portName[0] - 'A';
#ifdef Windows
	{
		LPCTSTR pp = _tcsstr(portName, _T("com"));
		int port;

		if(! pp) pp = _tcsstr(portName, _T("COM"));
		if(! pp) pp = _tcsstr(portName, _T("Com"));

		port = BSASERCOM_USERPORT;
		if(pp)
			if(pp[3] >= _T('1') && pp[3] <= _T('9'))
				port = _tcstol(pp + 3, NULL, 0) - 1;
		return port;
	}
	return 0; /* custom ports in windows are still comN: */
#else
	if(! _tcsnicmp(portName, _T("/dev/ttyS"), 9))
		return  _tcstol(portName + 9, NULL, 10);
	if(! _tcsnicmp(portName, _T("/dev/ttyUSB"), 11))
		return 64 + _tcstol(portName + 11, NULL, 10);
	if(! _tcsnicmp(portName, _T("/dev/tty"), 8))
	{
		if (portName[7] >= 'a' && portName[7] <= 'z' && portName[8] == 0)
			return portName[8] - 'a';
	}
	return BSASERCOM_USERPORT; /* custom port */
#endif
}



//***********************************************************************
//***********************************************************************
//
// Unix/Linux Specific functions

#ifndef Windows


//***********************************************************************
static long unixBaud(long rate)
{
	switch(rate) {
	case 50:	return B50;
	case 75:	return B75;
	case 110:	return B110;
	case 134:	return B134;
	case 150:	return B150;
	case 200:	return B200;
	case 300:	return B300;
	case 600:	return B600;
	case 1200:	return B1200;
	case 1800:	return B1800;
	case 2400:	return B2400;
	case 4800:	return B4800;
	default:
	case 9600:	return B9600;
	case 19200:	return B19200;
	case 38400:	return B38400;
	case 57600: return B57600;
	case 115200: return B115200;
	case 230400: return B230400;
#ifdef B460800
	case 460800: return B460800;
	case 500000: return B500000;
	case 576000: return B576000;
	case 921600: return B921600;
	case 1000000: return B1000000;
	case 1152000: return B1152000;
	case 1500000: return B1500000;
	case 2000000: return B2000000;
	case 2500000: return B2500000;
	case 3000000: return B3000000;
	case 3500000: return B3500000;
	case 4000000: return B4000000;
#endif
	}
}

//***********************************************************************
static int unixBits(int bits)
{
	switch(bits) {
	case 5:	return 0;
	case 6: return CS6;
	case 7:	return CS7;
	case 8: return CS8;
	default:
		return CS8;
	}
}

//***********************************************************************
static int unixStopBits(int sbits)
{
	switch(sbits) {
	case 1:
	default:
		return 0;
	case 2:
		return CSTOPB;
	}
}

//***********************************************************************
static int unixParity(int par)
{
	if(par == 1) return PARENB | PARODD;
	else if(par == 2) return PARENB;
	else return 0;
}

#endif /* ndef Windows */
