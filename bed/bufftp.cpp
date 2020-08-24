
#include "bedx.h"

//**************************************************************************
ERRCODE Bbuffer::ReadFtpFile(LPCTSTR lpFilename)
{
	Bftp	ftp(BED_FTP_LOGLEV);
	LPCTSTR userName;
	LPCTSTR passWord;
	LPCTSTR pRemoteName;
	ERRCODE	ec;

	if(! m_editor)
	{
		return errFAILURE;
	}

	GetTempFileName(m_localName, MAX_PATH);


	if(! lpFilename)
		lpFilename = m_name;

	pRemoteName = lpFilename;
	if(_tcsstr(pRemoteName, _T("ftp:")))
		pRemoteName += 4;
	while(*pRemoteName == _T('/') || *pRemoteName == _T('\\'))
		pRemoteName++;

	do
	{
		if((ec = m_editor->GetFTPUserName(userName)) != errOK)
			break;
		if((ec = m_editor->GetFTPPassword(passWord)) != errOK)
			break;

		ec = ftp.GetRemoteFile(
							pRemoteName,
							m_localName,
							userName,
							passWord
							);
		switch(ec)
		{
		case errOBJECT_NOT_FOUND:
			ec = errOK; // ok on read
			break;
		case errPERMISSION:
			ec = m_editor->GetFTPAuthorization(true);
			if(ec != errOK) return ec;
			ec = errPERMISSION;
			break;
		case errOK:
			break;
		default:
			return ec;
		}
	}
	while(ec == errPERMISSION);

	return Bbuffer::ReadFromFile(m_localName);
}

//**************************************************************************
ERRCODE Bbuffer::WriteFtpFile(LPCTSTR lpFilename, TEXTENCODING encoding)
{
	Bftp	ftp(BED_FTP_LOGLEV);
	LPCTSTR userName;
	LPCTSTR passWord;
	LPCTSTR pRemoteName;
	ERRCODE ec;

	if(! m_editor)
	{
		return errFAILURE;
	}
	// write the buffer to the local temp file
	//
	ec = Bbuffer::WriteToFile(m_localName, encoding);
	if(ec != errOK)
		return ec;

	if(! lpFilename)
		lpFilename = m_name;

	pRemoteName = lpFilename;
	if(_tcsstr(pRemoteName, _T("ftp:")))
		pRemoteName += 4;
	while(*pRemoteName == _T('/') || *pRemoteName == _T('\\'))
		pRemoteName++;

	do
	{
		if((ec = m_editor->GetFTPUserName(userName)) != errOK)
			break;
		if((ec = m_editor->GetFTPPassword(passWord)) != errOK)
			break;

		ec = ftp.PutRemoteFile(
							pRemoteName,
							m_localName,
							userName,
							passWord
							);
		switch(ec)
		{
		case errPERMISSION:
			ec = m_editor->GetFTPAuthorization(true);
			break;
		case errOK:
			break;
		default:
			return ec;
		}
	}
	while(ec != errOK);

	return ec;
}
