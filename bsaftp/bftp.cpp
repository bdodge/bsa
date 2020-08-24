
#include "bftpx.h"
 
#define FTP_CTRL_PORT 21
#define FTP_DATA_PORT 20

#ifndef Windows
#define _tfopen bsafopen
#endif

#define FTP_LOG 1

//**************************************************************************
bool Bftp::m_initialized = false;

//**************************************************************************
Bftp::Bftp(int loglevel)
		:
		m_ctrl(NULL),
		m_data(NULL),
		m_tos(15),
		m_tous(0),
		Blog(loglevel)
{
	if(! m_initialized)
	{
		BsocketStream::Init();
		m_initialized = true;
	}
		
}

//**************************************************************************
Bftp::~Bftp()
{
}

//**************************************************************************
ERRCODE Bftp::CheckResponse(int* code, int& rcode)
{
	int		nRead;
	ERRCODE ec;

	if(! m_ctrl) return errFAILURE;

	ec = m_ctrl->Pend(m_tos, m_tous);
	if(ec != errSTREAM_DATA_PENDING)
		return ec;
	nRead = sizeof(m_response) - 2;
	ec = m_ctrl->Read(m_response, nRead);
	if(ec != errOK) return ec;
	if(nRead <= 0)  return errFAILURE;
	m_response[nRead] = 0;

	TCHAR szResp[1024];
	CharToTChar(szResp, (LPSTR)m_response);
#ifdef FTP_LOG
	Log(logDebug, 4, _T("FTP: Response:"_Pfs_"\n"), szResp);
#endif

	rcode = atoi((char*)m_response);
	
	for(; code && *code; code++)
		if(*code == rcode || *code == -1)
			return errOK;
#ifdef FTP_LOG
		Log(logDebug, 2, _T("FTP: code %d not expected\n"), rcode);
#endif
	return errFAILURE;
}

//**************************************************************************
ERRCODE Bftp::SendCommand(LPCSTR command, LPCTSTR param)
{
	ERRCODE ec;
	char*   aParm;
	int		nParm;

	aParm = new char [ _tcslen(param) + strlen(command) + 32 ];
	strcpy(aParm, command);
	if(param && param[0] != _T('\0'))
	{
		strcat(aParm, " ");
		UTF8ENCODE(aParm + strlen(aParm), param);
	}
	strcat(aParm, "\r\n");

	nParm = strlen(aParm);
	ec = m_ctrl->Write((LPBYTE)aParm, nParm); 

	delete [] aParm;
	return ec;
}

//**************************************************************************
ERRCODE Bftp::Login(LPCTSTR hostname, LPCTSTR username, LPCTSTR password)
{
	ERRCODE ec;
	char	ahost[MAX_PATH];
	int     rcode;
	int		codes[32];

	
	TCharToChar(ahost, hostname);
	m_ctrl = new BtcpStream();

	do
	{
#ifdef FTP_LOG
		Log(logDebug, 2, _T("FTP: Opening "_Pfs_"\n"), hostname);
#endif
		ec = m_ctrl->Open(ahost, FTP_CTRL_PORT);
		if(ec != errOK) break;

		ec = m_ctrl->Connect(m_tos, m_tous);
		if(ec != errOK) break;

#ifdef FTP_LOG
		Log(logDebug, 2, _T("FTP: Connected to "_Pfs_"\n"), hostname);
#endif

		// connected on ftp ctrl channel, read welcome msg
		//
		codes[0] = 220; codes[1] = 0;
		ec = CheckResponse(codes, rcode);
		if(ec != errOK) break;

		// make sure all data in welcome message is read
		//
		do
		{
			ec = m_ctrl->Pend(0, 80000);
			if(ec == errSTREAM_DATA_PENDING)
			{
				int nRead = sizeof(m_response) - 2;
				m_ctrl->Read(m_response, nRead);
			}
		}
		while(ec == errSTREAM_DATA_PENDING);

		// authenticate with User/Password
		//
		ec = SendCommand("USER", username);
		if(ec != errOK) break;

		codes[0] = 331; codes[1] = 230; codes[2] = 332; codes[3] = 530; codes[4] = 0;
		ec = CheckResponse(codes, rcode);
		if(ec != errOK) break;

		if(rcode == 331 || rcode == 332)
		{
			// need a password
			ec = SendCommand("PASS", password);
			if(ec != errOK) break;

			codes[0] = 230; codes[1] = 202; codes[2] = 332; codes[3] = 500;
			codes[4] = 530; codes[5] = 0;
			ec = CheckResponse(codes, rcode);
			if(ec != errOK) break;

			if(rcode == 332)
			{
				// need an account (this is never used in practice)
				ec = SendCommand("ACCT", _T("noaccount"));
				codes[0] = 230; codes[1] = 202; codes[2] = 0;
				ec = CheckResponse(codes, rcode);
				if(ec != errOK) break;
			}
		}
		if(rcode >= 500)
		{
			ec = errPERMISSION;
			break;
		}
#ifdef FTP_LOG
		Log(logDebug, 2, _T("FTP: "_Pfs_" logged in\n"), username);
#endif
	}
	while(0);

	return ec;
}

//**************************************************************************
ERRCODE Bftp::SetupPassive()
{
	ERRCODE ec;
	int     rcode;
	int		codes[32];
	BYTE	ip[5];
	char	ipaddr[32];
	int		p1, p2;
	WORD	port;
	char*   rp;

	ec = SendCommand("PASV", _T(""));
	if(ec != errOK)	return ec;

	codes[0] = 227; codes[1] = 0;
	ec = CheckResponse(codes, rcode);
	if(ec != errOK)	return ec;

	// decode the response buffer into IP address and port
	//
	for(rp = (char*)m_response + 3; *rp; rp++)
		if(*rp >= '0' && *rp <= '9')
			break;

	if(! *rp)
	{
#ifdef FTP_LOG
		Log(logDebug, 1, _T("FTP: No IP address in PASV response\n"));
#endif
		ec = errFAILURE;
		return ec;
	}
	ip[0] = (BYTE)strtol(rp, &rp, 10);
	if(! rp) { ec = errFAILURE; return ec; }
	ip[1] = (BYTE)strtol(++rp, &rp, 10);
	if(! rp) { ec = errFAILURE; return ec; }
	ip[2] = (BYTE)strtol(++rp, &rp, 10);
	if(! rp) { ec = errFAILURE; return ec; }
	ip[3] = (BYTE)strtol(++rp, &rp, 10);

	if(! rp) { ec = errFAILURE; return ec; }
	p1 = strtol(++rp, &rp, 10);
	if(! rp) { ec = errFAILURE; return ec; }
	p2 = strtol(++rp, &rp, 10);
	
	port = ((WORD)((BYTE)p1) << 8) | (BYTE)p2;

	sprintf(ipaddr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

	// should have ip address and port for data now
	// open a TCP connection to that address
	//
	m_data = new BtcpStream();
	ec = m_data->Open(ipaddr, port);
	ec = m_data->Connect(m_tos, m_tous);

	if(ec != errOK)
	{
#ifdef FTP_LOG
		Log(logDebug, 1, _T("FTP: Can't connect to server PASV address\n"));
#endif
		return ec;
	}
	// ask for binary transmission
	//
	ec = SendCommand("TYPE", _T("I"));
	codes[0] = 200; codes[1] = 226; codes[2] = 0;
	ec = CheckResponse(codes, rcode);
	return ec;
}

//**************************************************************************
ERRCODE Bftp::Logout()
{
	ERRCODE ec;
	int     rcode;
	int		codes[32];

	//
	// send quit (ignore errors after here)
	//
	SendCommand("QUIT", _T(""));
	codes[0] = 221; codes[1] = 0;
	ec = CheckResponse(codes, rcode);
	return ec;
}

//**************************************************************************
ERRCODE Bftp::SplitRemotePath(
							LPCTSTR lpRemotePath,
							LPTSTR  host,
							LPTSTR  file
							)
{
	LPTSTR	pd, ps;

	pd = host;
	ps = (LPTSTR)lpRemotePath;

	while(*ps == _T('/') || *ps == _T('\\'))
		ps++;

	while(*ps && *ps != _T('/') && *ps != _T('\\'))
		*pd++ = *ps++;
	*pd++ = _T('\0');

	while(*ps == _T('/') || *ps == _T('\\'))
		ps++;

	pd = file;
	while(*ps)
	{
		if(*ps == _T('\\'))
			*pd++ = _T('/');
		else
			*pd++ = *ps;
		ps++;
	}
	*pd = _T('\0');
	return errOK;
}

//**************************************************************************
ERRCODE Bftp::FileXfer(
						LPCTSTR lpRemoteName,
						LPCTSTR lpLocalName,
						LPCTSTR lpUserName,
						LPCTSTR lpPassword,
						bool putget
						)
{
	TCHAR	host[MAX_PATH];
	TCHAR	file[MAX_PATH];
	LPCTSTR lpRemoteFile;
	ERRCODE ec, rec;
	int     rcode;
	int		codes[32];
	BYTE	fioBuf[2048];
	int		nRead;
	FILE*	lf;

	if(! lpRemoteName || ! lpLocalName)
		return errBAD_PARAMETER;

	// extract host name as first component of remote Name
	// and point to path portion of name relative to logged in dir
	//
	SplitRemotePath(lpRemoteName, host, file);
	lpRemoteFile = file;

	if(! host[0]) return errBAD_PARAMETER; 

	// open the local file
	//
	if(putget)
		lf = _tfopen(lpLocalName, _T("rb"));
	else
		lf = _tfopen(lpLocalName, _T("wb"));

	if(! lf)
	{
#ifdef FTP_LOG
		Log(logDebug, 1, _T("FTP: Can't open local file "_Pfs_"\n"), lpLocalName);
#endif
		return errBAD_PARAMETER;
	}
	ec = Login(host, lpUserName, lpPassword);
	if(ec != errOK)
	{
#ifdef FTP_LOG
		Log(logDebug, 2, _T("FTP: Login Failed\n"));
#endif
	}
	else
	{
		// ok, all logged in, use passive data transfer
		// for firewall friendly operation
		//
		ec = SetupPassive();
		if(ec == errOK)
		{
			do
			{
				// ok, now send the store/retrieve command for the file
				//
				if(putget)
				{
					ec = SendCommand("STOR", lpRemoteFile);
					if(ec != errOK)	break;
					codes[0] = 150; codes[1] = 0;
					rcode = 500;
					ec = CheckResponse(codes, rcode);
					if(ec != errOK)	
					{
						// normally a 150 (mark) is sent by the server, but as long
						// as its not an error, we continue
						//
						if(ec == errFAILURE)
						{
							if(rcode < 400)
								ec = errOK;
							else
								break;
						}
						else
						{
							// hard error (like tcp error)
							break;
						}
					}
					// pump file data to server 
					//
					do
					{
						nRead = sizeof(fioBuf);
						nRead = fread(fioBuf, 1, nRead, lf);
						if(nRead > 0)
						{
							rec = m_data->Write(fioBuf, nRead);
							if(nRead == 0)
								break;
						}
						else
							break;
					}
					while(rec == errOK);
				}
				else
				{
					ec = SendCommand("RETR", lpRemoteFile);
					if(ec != errOK)	break;
					codes[0] = 150; codes[1] = 0;
					rcode = 500;
					ec = CheckResponse(codes, rcode);
					if(ec != errOK)	
					{
						// normally a 150 (mark) is sent by the server, but as long
						// as its not an error, we continue
						//
						if(ec == errFAILURE)
						{
							if(rcode < 400)
								ec = errOK;
							else 
							{
								switch(rcode)
								{
								case 550:
									ec = errOBJECT_NOT_FOUND;
									break;
								}
								break;
							}
						}
						else
						{
							// hard error (like tcp error)
							break;
						}
					}
					// server should be pumping file data onto data stream
					//
					do
					{
						rec = m_data->Pend(m_tos, m_tous);
						if(rec == errSTREAM_DATA_PENDING)
						{
							nRead = sizeof(fioBuf);
							rec = m_data->Read(fioBuf, nRead);
							if(nRead == 0)
								break;
							fwrite(fioBuf, nRead, 1, lf);
						}
						else
							break;
					}
					while(rec == errOK);
				}

				fclose(lf);

				// close the data connection now (some servers need this b4 sending 226)
				//
				m_data->Close();

				codes[0] = 226; codes[1] = 0;
				ec = CheckResponse(codes, rcode);

				if(ec != errOK)
				{
#ifdef FTP_LOG
					Log(logDebug, 1, _T("FTP: Didn't get file err=%d\n"), rcode);
#endif
					break;
				}
			}
			while(0);
		}
		// send quit (ignore errors after here)
		//
		Logout();
	}
	while(0);

	if(m_ctrl) delete m_ctrl;
	if(m_data) delete m_data;
	m_ctrl = NULL;
	m_data = NULL;
	return ec;
}

//**************************************************************************
ERRCODE Bftp::GetRemoteFile(
								LPCTSTR lpRemoteName,
								LPCTSTR lpLocalName,
								LPCTSTR lpUserName,
								LPCTSTR lpPassword
						   )
{
	return FileXfer(lpRemoteName, lpLocalName, lpUserName, lpPassword, false);
}

//**************************************************************************
ERRCODE Bftp::PutRemoteFile(
								LPCTSTR lpRemoteName,
								LPCTSTR lpLocalName,
								LPCTSTR lpUserName,
								LPCTSTR lpPassword
						   )

{
	return FileXfer(lpRemoteName, lpLocalName, lpUserName, lpPassword, true);
}

//**************************************************************************
ERRCODE Bftp::ListRemoteDirectory(
								LPCTSTR lpRemoteDirectory,
								LPFTPDIRCB lpDirectoryCallback,
								LPVOID  cookie,
								LPCTSTR lpUserName,
								LPCTSTR lpPassword
						    )

{
	TCHAR	host[MAX_PATH];
	TCHAR	file[MAX_PATH];
	BYTE	fioBuf[2048];
	ERRCODE ec, rec;
	int     rcode;
	int		codes[32];
	int		nRead;

	if(! lpRemoteDirectory || ! lpDirectoryCallback)
		return errBAD_PARAMETER;

	// extract host name as first component of remote Name
	// and point to path portion of name relative to logged in dir
	//
	SplitRemotePath(lpRemoteDirectory, host, file);
	lpRemoteDirectory = file;

	if(! host[0]) return errBAD_PARAMETER; 

	ec = Login(host, lpUserName, lpPassword);
	if(ec != errOK)
	{
#ifdef FTP_LOG
		Log(logDebug, 2, _T("FTP: Login Failed\n"));
#endif
	}
	else 
	{
		ec = SetupPassive();
		if(ec == errOK)
		{
			do
			{
				char  lineBuf[128];
				int   lc, ic;
				char  xc;
				char* pf;
				WORD  mode;

				// ok, all logged in, get a listing
				// of the directory specified
				//
				ec = SendCommand("LIST", lpRemoteDirectory);
				if(ec != errOK)	break;

				codes[0] = 150; codes[1] = 200; codes[2] = 226; codes[3] = 0;
				ec = CheckResponse(codes, rcode);
				if(ec != errOK)	break;

				// keep reading the response(s) and calling back
				// the caller with each file listed
				//
				nRead = 0;
				lc    = 0;
				ic    = 0;
				
				do
				{
					while(ic < nRead)
					{
						xc = fioBuf[ic++];
						if(xc == '\r' || xc == '\n' || (lc >= sizeof(lineBuf)-2))
						{
							if(ic < nRead && fioBuf[ic] == '\n')
								ic++;

							lineBuf[lc] = 0;

							// process one line of directory listing
							// fodder.  only two types are supported
							// Unix "ls -l" and windows "DIR" formats
							// but in both cases the filename is last
							lc--;
							while(lc >= 0)
							{
								if(lineBuf[lc] == ' ' || lineBuf[lc] == '\t')
									break;
								lc--;
							}
							pf = lineBuf + lc + 1;

							if(lineBuf[0] >= '0' && lineBuf[0] <= '9')
							{
								// bill gates format
								//
								mode = 0777;
								if(strstr(lineBuf, "<DIR>") != NULL)
									mode |= 0x8000;
							}
							else if(lineBuf[0] == 'd' || lineBuf[0] == '-')
							{
								// Unix "ls -l" format
								//
								mode = 0;
								if(lineBuf[0] == 'd')
									mode |= 0x8000;
								if(lineBuf[1] == 'r')
									mode |= 0400;
								if(lineBuf[2] == 'w')
									mode |= 0200;
								if(lineBuf[3] == 'x')
									mode |= 0100;
								if(lineBuf[4] == 'r')
									mode |= 040;
								if(lineBuf[5] == 'w')
									mode |= 020;
								if(lineBuf[6] == 'x')
									mode |= 010;
								if(lineBuf[7] == 'r')
									mode |= 04;
								if(lineBuf[8] == 'w')
									mode |= 02;
								if(lineBuf[9] == 'x')
									mode |= 01;
							}
							else
							{
								mode = 0;
							}
#ifdef UNICODE
							if(strlen(pf) < MAX_PATH)
							{
								WCHAR fn[MAX_PATH];

								BUtil::Utf8Decode(fn, pf);
								lpDirectoryCallback(fn, mode, cookie);
							}
#else
							lpDirectoryCallback(pf, mode, cookie);
#endif
							lc = 0;
						}
						else
						{
							lineBuf[lc++] = xc;
						}
					}
					ic = 0;
					rec = m_data->Pend(m_tos, m_tous);
					if(rec == errSTREAM_DATA_PENDING)
					{
						nRead = sizeof(fioBuf);
						rec = m_data->Read(fioBuf, nRead);
						if(nRead == 0)
							break;
					}
					else
						break;
				}
				while(rec == errOK);
			}
			while(0);
			Logout();
		}
	}
	while(0);

	if(m_ctrl) delete m_ctrl;
	if(m_data) delete m_data;
	m_ctrl = NULL;
	m_data = NULL;
	return ec;
}


