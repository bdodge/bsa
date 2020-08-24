
#include "openfilex.h"

//**************************************************************************
ERRCODE BdirInfo::ListDirectory(LPCTSTR path)
{
	TCHAR	   rpath[MAX_PATH*2];
	TCHAR      rpattern[MAX_PATH];
	int		   len, plen;

	_tcsncpy(rpath, path, MAX_PATH-1);
	rpath[MAX_PATH-1] = _T('\0');
	len = plen = _tcslen(rpath);

	// extract path and pattern from path spec
	// (e.g. "a/*.c" becomes "a/ and *.c")
	//
	rpattern[0] = _T('\0');
	while(len > 0)
	{
		if(rpath[len-1] == _PTC_)
		{
			if(len < plen)
			{
				// dangling portion of path is file pattern
				_tcscpy(rpattern, rpath+len);
				rpath[len] = _T('\0');
				break;
			}
			else
			{
				// path ends in PTC, so no pattern spec
				_tcscpy(rpattern, _T("*"));
				break;
			}
		}
		len--;
	}
	if(len == 0)
	{
		// its all pattern
		_tcscpy(rpattern, rpath);
		rpath[0] = _T('\0');
	}
	len = _tcslen(rpath);;
	if(! rpattern[0])
		_tcscpy(rpattern, _T("*"));
	if(len > 0)
	{
		if(rpath[len-1] != _PTC_)
		{
			rpath[len++] = _PTC_; 
			rpath[len] = _T('\0');
		}
	}
	else
	{
		rpath[0] = _T('.');
		rpath[1] = _PTC_;
		rpath[2] = _T('\0');
	}
	_tcscpy(m_szBase, rpath);
#ifdef WIN32
	_tcscpy(m_path, rpath);
	_tcscat(m_path, rpattern);
	m_hFind = NULL;
	m_first = true;
#else
	_tcscpy(m_pattern, rpattern);
	TCharToChar(m_path, rpath);
	if((m_pDir = opendir(m_path)) == NULL)
		return errFAILURE;
	m_first = false;
#endif
	return errOK;
}

//**************************************************************************
ERRCODE BdirInfo::NextFile(LPCTSTR& lpName, bool& isDir, bool& isLink, bool& isReadOnly)
{
#ifndef WIN32
	do
	{
#endif
	if(m_first)
	{
#ifdef WIN32
		if((m_hFind = FindFirstFile(m_path, &m_fdata)) == INVALID_HANDLE_VALUE)
#else
		if(m_pDir == NULL)
#endif
		{
			return errFAILURE;
		}
		m_first = false;
	}
#ifdef WIN32
	else
#endif
	{
#ifdef WIN32
		if(! FindNextFile(m_hFind, &m_fdata))
#else
		if(m_pDir == NULL)
			return errFAILURE;	
		if((m_dp = readdir(m_pDir)) == NULL)
#endif
		{
#ifdef WIN32
			FindClose(m_hFind);
#else
			closedir(m_pDir);
#endif
			return errFAILURE;
		}
	}
	_tcscpy(m_szName, m_szBase);
	lpName		= m_szName;
#ifdef WIN32
	_tcscat(m_szName, m_fdata.cFileName);
	isDir		= (m_fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	isReadOnly	= (m_fdata.dwFileAttributes & FILE_ATTRIBUTE_READONLY)  != 0;
	isLink		= false;
#else
	CharToTChar(m_szName + _tcslen(m_szName), m_dp->d_name);
	isLink		= false;

	if(! BUtil::SimplePatternMatch(m_szName, m_pattern))
		continue;

	char fp[MAX_PATH + 64];
	struct stat finfo;

	strcpy(fp, m_path);
	strcat(fp, m_dp->d_name);

	if(! stat(fp, &finfo))
	{
		int mode;
		
		isDir = ((mode = finfo.st_mode) & S_IFDIR) != 0;

		{
			if(finfo.st_uid != getuid())
			{
				mode = mode & 0077;
				mode |= ((mode & 0070) << 3);
			}
			if(! (mode & 0200))
			{
				isReadOnly = true;
			}
			else
			{
				isReadOnly = false;
			}
		}
	}
	break;
#endif
#ifndef WIN32
	}
	while(1);
#endif
	return errOK;
}

//**************************************************************************
ERRCODE BdirInfo::EndListDirectory()
{
	return errOK;
}

//**************************************************************************
/*static*/ LPTSTR BdirInfo::FilePart(LPTSTR path)
{
	LPTSTR pf = path;
	int    l;
	
	if(! path) return (LPTSTR)_T("<nil>");
	l = _tcslen(path);
	if(l == 0) return path;
	while(l > 0)
	{
		if(path[l-1] == _PTC_)
			return path + l;
		l--;
	}
	return path;
}


