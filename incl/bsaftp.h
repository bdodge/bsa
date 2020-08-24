//--------------------------------------------------------------------
//
// File: bsaftp.h
// Desc: FTP
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------
#ifndef _BFTP_H_
#define _BFTP_H_ 1


#ifdef FTP_EXPORTS
#define FTP_API __declspec(dllexport)
#elif defined(BSAFTP_DLL)
#define FTP_API __declspec(dllimport)
#else
#define FTP_API
#endif

typedef bool (*LPFTPDIRCB)(LPCTSTR pName, WORD mode, LPVOID cookie);

// ftp object
//**************************************************************************
class FTP_API Bftp : Blog
{
public:
	Bftp(int loglevel = 0);
	~Bftp();

public:
	ERRCODE				GetRemoteFile		(
											LPCTSTR lpRemoteName,
											LPCTSTR lpLocalName,
											LPCTSTR lpUserName,
											LPCTSTR lpPassword
											);

	ERRCODE				PutRemoteFile		(
											LPCTSTR lpRemoteName,
											LPCTSTR lpLocalName,
											LPCTSTR lpUserName,
											LPCTSTR lpPassword
											);

	ERRCODE				ListRemoteDirectory	(
											LPCTSTR lpRemoteDirectory,
										 LPFTPDIRCB lpLocalName,
											LPVOID  cookie,
											LPCTSTR lpUserName,
											LPCTSTR lpPassword
											);

protected:
	ERRCODE				FileXfer			(
											LPCTSTR lpRemoteName,
											LPCTSTR lpLocalName,
											LPCTSTR lpUserName,
											LPCTSTR lpPassword,
											bool	putget
											);

	ERRCODE				CheckResponse		(int* code, int& rcode);
	ERRCODE				SendCommand			(LPCSTR command, LPCTSTR param);
	ERRCODE				Login				(LPCTSTR hostname, LPCTSTR username, LPCTSTR password);
	ERRCODE				SetupPassive		(void);
	ERRCODE				Logout				(void);
	ERRCODE				SplitRemotePath		(
											LPCTSTR lpRemotePath,
											LPTSTR  host,
											LPTSTR  path
											);

protected:
	BtcpStream*			m_ctrl;
	BtcpStream*			m_data;
	int					m_tos;
	int					m_tous;
	BYTE				m_response[1024];

	static bool			m_initialized;
};


#endif
