
#include "utilsx.h"

#ifdef PTHREADS
	typedef void*(*PTHREADFUNC)(void*);
#endif

#if defined(Windows)
//**************************************************************************
Bmutex::Bmutex()
{
	InitializeCriticalSection(&cs);
}

//**************************************************************************
Bmutex::~Bmutex()
{
	DeleteCriticalSection(&cs);
}
    
//**************************************************************************
void Bmutex::Lock()
{
	EnterCriticalSection(&cs);
}

//**************************************************************************
void Bmutex::Unlock()
{ 
	LeaveCriticalSection(&cs);
}

#elif defined(PTHREADS)
//**************************************************************************
Bmutex::Bmutex()
{
	int s;
	
#ifdef Linux
    pthread_mutexattr_t attr;

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
	s = pthread_mutex_init(&cs, &attr);
#else
	s = pthread_mutex_init(&cs, NULL);
#endif
}

//**************************************************************************
Bmutex::~Bmutex()
{
	pthread_mutex_destroy(&cs);
}
    
//**************************************************************************
void Bmutex::Lock()
{
	int s = pthread_mutex_lock(&cs);
	if(s)
		_tprintf(_T("Mutex lock failed: %d\n"), s);
}

//**************************************************************************
void Bmutex::Unlock()
{
	pthread_mutex_unlock(&cs);
}

#else
	#pragma Warning("No Bmutex")
#endif

//**************************************************************************
ERRCODE	Bthread::Start(LPTHREAD_FUNC pThreadFunc, LPVOID thisparm, bool createThread)
{
	m_running = true;

	if(createThread)
	{
#ifdef Windows
		m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pThreadFunc, thisparm, 0, &m_threadID);
		return (m_hThread == NULL) ? errTHREAD_CREATE : errOK;
#else
		if(pthread_create(&m_hThread, NULL, (PTHREADFUNC)pThreadFunc, thisparm) == 0)
			return errOK;
		else
			return errTHREAD_CREATE;
#endif
	}
	else
	{
		return pThreadFunc(thisparm);
	}
}

//**************************************************************************
ERRCODE	Bthread::Stop(void)
{
	m_running = false;
#ifdef Windows
	if(WaitForSingleObject(m_hThread, 1000) == WAIT_TIMEOUT)
		return errTHREAD_STOP;
#endif
	return errOK;
}

//**************************************************************************
Blog::Blog(int level, logDest dest, LPCTSTR logfile)
		:
		m_level(level),
		m_logdest(dest),
		m_plf(NULL)
{
	if(logfile)
	{
		SetLogfile(logfile);
	}
	else
	{
		m_plf = stdout;
	}
}

//**************************************************************************
Blog::~Blog()
{
	if(m_plf && m_plf != stdout) fclose(m_plf);
}

//**************************************************************************
void Blog::Log(logType type, int level, LPCTSTR format, ...)
{
	va_list  arg;

	if(level > m_level || m_logdest == logtoNOWHERE)
	{
		return;
	}
	//Block lock(&m_logex);

	va_start(arg, format);
#ifndef SOLARIS
	_vsntprintf(m_emsg, sizeof(m_emsg)/sizeof(TCHAR), format, arg);
#else
	vsprintf(m_emsg, format, arg);
#endif
	va_end ( arg );

	LPCTSTR pszType = _T("");

	switch (type)
	{
	case logDebug:
		pszType = _T("DEBUG  ");
		break;
	case logInfo:
		pszType = _T("INFO   ");
		break;
	case logError:
		pszType = _T("ERROR--");
		break;
	}
	switch(m_logdest)
	{
#ifdef Windows
	case logtoWINDOW:
		MessageBox(0, m_emsg, pszType, 0);
		break;
	default:
	case logtoCONSOLE:
		OutputDebugString(pszType);
		OutputDebugString(m_emsg);
		break;
#else
	default:
	case logtoCONSOLE:
#endif

	case logtoFILE:
	case logtoFILEAPPEND:
#ifdef UNICODE
		fwprintf(m_plf,  L"%ls %ls", pszType, m_emsg);	
#else
		fprintf(m_plf,  "%s %s", pszType, m_emsg);	
#endif
		fflush(m_plf);
		break;

	case logtoNOWHERE:
		break;
	}
}

//**************************************************************************
void Blog::SetLogfile(LPCTSTR logfile)
{
	if(m_plf && m_plf != stdout)
		fclose(m_plf);
	m_plf = NULL;
	if(logfile)
	{
#ifdef Windows
		m_plf = _tfopen(logfile, (m_logdest == logtoFILEAPPEND) ? _T("a") : _T("w"));
#else
#ifdef UNICODE
		char alfn[MAX_PATH];
		
		BUtil::Utf8Encode(alfn, logfile, false);
		m_plf = fopen(alfn, (m_logdest == logtoFILEAPPEND) ? "a" : "w");
#else
		m_plf = fopen(logfile, (m_logdest == logtoFILEAPPEND) ? "a" : "w");
#endif
#endif
		if(m_logdest == logtoFILEAPPEND)
		{
			// timestamp the start of the log
			time_t now;
			char   *atime;
#ifdef UNICODE
			TCHAR  szTime[300];
#else
			char*  szTime;
#endif
			time(&now);
			atime = ctime(&now);
#ifdef UNICODE
			BUtil::CharToWChar(szTime, atime);
#else
			szTime = atime;
#endif
			Log(logInfo, 0, _T("------------ Log Start: " _Pfs_ ""), szTime);
		}
	}
	if(! m_plf)
		m_plf = stdout;
}

//**************************************************************************
ERRCODE BUtil::LoadLibrary(HDYNLIB& hLib, LPCTSTR pName)
{
#ifdef Windows
	hLib = ::LoadLibrary(pName);
	if(! hLib) 
	{
		int werr= GetLastError();
		int a = werr;
	}
#else
#ifdef UNICODE
	char* pname = new char [ _tcslen(pName) + 2 ];
	WCharToChar(pname, pName);
	hLib = dlopen(pname, RTLD_NOW);
	delete [] pname;
#else
	hLib = dlopen(pName, RTLD_NOW);
#endif
	if(! hLib)
	{
		char* err = dlerror();
		char* e = err;
	}
#endif
	if(! hLib) return errFAILURE;
	return errOK;
}

//**************************************************************************
ERRCODE BUtil::ResolveFunction(HDYNLIB hLib, LPCTSTR pName, PDYNFUNCTION& pFunc)
{
	char* pname = new char [ _tcslen(pName) + 2 ];
	TCharToChar(pname, pName);
#ifdef Windows
	pFunc = ::GetProcAddress(hLib, pname);
#else
	pFunc = (PDYNFUNCTION)dlsym(hLib, pname);
#endif
	delete [] pname;
	if(! pFunc) return errFAILURE;
	return errOK;
}

//**************************************************************************
ERRCODE BUtil::CloseLibrary(HDYNLIB hLib)
{
#ifdef Windows
#else
	dlclose(hLib);
#endif
	return errOK;
}

//**************************************************************************
ERRCODE BUtil::UrlParse(LPCSTR pURL, LPSTR proto, LPSTR host, short* port, LPSTR path, LPSTR parms)
{
	char*  phost;
	char*  phostend = NULL;
	char*  purl	 = (LPSTR)pURL;
	char*  pport;
	char*  ppath;
	char*  pps;

	int     i;

	// break "URL" into protocol://host:port/path?parms
	//
	if(port)	*port  = 80;
	if(proto)	*proto = '\0';
	if(host)	*host  = '\0';
	if(path)	*path  = '\0';
	if(parms)	*parms = '\0';

	if(strlen(pURL) > MAX_URL_LEN) return errURL_TOO_LONG;

	// find location of parameter sep.
	//
	pps = strstr(purl, "?");

	// find "//" as start of hostname portion of url
	//
	phost = strstr(purl, "//");
	if(phost && pps && phost > pps) phost = NULL;

	if(phost)
	{
		// got the "//" see if there is a protocol in front of it
		//
		if((phost - purl) > 0)
		{
			// make sure url protocol is sane
			if((phost - purl) >= MAX_URL_PROTO_LEN) return errURL_BAD_PROTOCOL;
			if(proto)
			{
				memcpy(proto, purl, (phost - purl));
				proto[(phost - purl)] = '\0';
			}
		}
		else
		{
			if(proto)
				strcpy(proto, "http:");
		}
		phost+= 2;
		purl = phost;
	}
	else
	{
		phost = purl;
		if(proto) strcpy(proto, "http:");
	}

	// find :port
	//
	if((pport = strstr(purl, ":")) != NULL && (! pps || (pport < pps)))
	{
		char portbuf[32];

		// remember end of phost
		phostend = pport++;
		for(purl = pport, i = 0; *purl >= '0' && *purl <= '9'; i++)
		{
			if(i >= sizeof(portbuf)/sizeof(char)) return errURL_BAD_PORT;
			portbuf[i] = *purl++;
		}
		portbuf[i] = '\0';
		if(port)
			*port = (short)strtol(portbuf, NULL, 0);
	}

	// find "/" at end of host:port, or null (no url path)
	//
	if((ppath = strstr(purl, "/")) != NULL && (! pps || (ppath < pps)))
	{
		// set end of hostname if no port specified
		if(! phostend)
			phostend = ppath;
		ppath++;
	}
	else
	{
		// there is no /, so assume all the rest is the hostname
		// if a protocol is specified, else assume the rest is
		// the actual path

		if(purl > pURL)
		{
			// all of the rest is unterminated host name
			while(*purl) 
				purl++;
			if(! phostend)
				phostend = purl;
			ppath = purl;
		}
		else
		{
			// all of the rest is unterminated path
			if(! phostend)
				phostend = purl;
			ppath = purl;
			while(*purl && *purl != '?') 
				purl++;
		}
	}
	// copy hostname to buffer now that it is a known length
	for(i = 0; phost < phostend; i++, phost++)
	{
		if(i >= MAX_HOST_NAME) return errURL_BAD_HOSTNAME;
		if(host) host[i] = *phost;
	}
	if(host) host[i] = '\0';

	// find '?' in url as start of parameter region
	for(i = 0; *ppath && *ppath != '?'; i++, ppath++)
	{
		if(i >= MAX_URL_LEN) return errURL_BAD_PATH;
		if(path) path[i] = *ppath;
	}
	if(path) path[i] = '\0';

	if(*ppath != '?')
		return errOK;
	else 
		ppath++;

	// copy parm string over
	for(i = 0; *ppath; i++, ppath++)
	{
		if(i >= MAX_URL_LEN) return errURL_BAD_PATH;
		if(parms) parms[i] = *ppath;
	}
	if(parms) parms[i] = '\0';
	return errOK;
}

//***********************************************************************
ERRCODE BUtil::UrlExtractParms(LPCSTR pParmBuf, BkeyVal*& pList)
{
	char* pterm;
	char* pvalue;
	char* pparm;
	char* pfv;
	char* pdv;
	int   ps;

	// convert %xx and + escapes in-situ, I also convert natural '&'
	// characters to 001, and natural '=' characters to '002' so they
	// can be distinguished from ones that were escaped.  The alternative
	// would be to do this unescaping for each parm name and value
	// after they are separated, but this is faster.
	//
	for(pdv = pfv = (char*)pParmBuf, ps = 0; *pfv; pfv++)
	{
		switch(*pfv)
		{
		case '+':
			*pdv++ = ' ';
			break;

		case '%':
			{
			BYTE tv;
			BYTE av;
			TCHAR ac = *++pfv;

			if(ac >= 'a' && ac <= 'f')		av = (BYTE)(ac - ('a' - 10));
			else if(ac >= 'A' && ac <= 'F') av = (BYTE)(ac - ('A' - 10));
			else									av = (BYTE)(ac - '0');

			tv = av << 4;
			ac = *++pfv;

			if(ac >= 'a' && ac <= 'f')		av = (BYTE)(ac - ('a' - 10));
			else if(ac >= 'A' && ac <= 'F') av = (BYTE)(ac - ('A' - 10));
			else									av = (BYTE)(ac - '0');

			tv |= av;
			*pdv++ = tv;
			}
			break;

		case '&':
			ps = 0;
			*pdv++ = '\01';
			break;

		case '=':
			if(ps == 0)
			{
				ps = 1;
				*pdv++ = '\02';
			}
			else
			{
				*pdv++ = '=';
			}
			break;

		default:
			*pdv++ = *pfv;
			break;
		}
	}
	*pdv = '\0';

	// parse any parms into list of parms
	//
	for(pterm = (char*)pParmBuf; *pterm;)
	{
		pparm = pterm;

		// skip to end of parm=value
		while(*pterm && (*pterm != '\01'))
		{
			pterm++;
		}
		if(*pterm == '\01')
		{
			*pterm++ = '\0';
		}
		// find '=' in parm=value
		for(pvalue = pparm; *pvalue && *pvalue != '\02'; pvalue++)
			;

		if(*pvalue == '\02') 
		{
			*pvalue++ = '\0';
		}
		{
			TCHAR pname[MAX_URL_LEN];

			UTF8DECODE(pname, pparm);

			// first one in wins (so defaults can come later e.g. in checkbox forms
			//
			if(! pList || ! pList->FindKey(pname, pList))
			{
				TCHAR*   pval = new TCHAR[strlen(pvalue) * 6 + 2 ];

				UTF8DECODE(pval, pvalue);

				BkeyVal* plparm = new BkeyVal(pname, pval); // claims ownership of pval now

				pList = BkeyVal::AddToList(plparm, pList);
			}
		}
	}
	return errOK;
}

//**************************************************************************
WCHAR* BUtil::CharToWChar(LPWSTR pRes, const char* pSrc)
{
	while(*pSrc)
	{
		*pRes++ = (WCHAR)*pSrc++;
	}
	*pRes++ = '\0';
	return pRes;
}

//**************************************************************************
char* BUtil::WCharToChar(char* pRes, LPCWSTR pSrc)
{
	while(*pSrc)
	{
		*pRes++ = (char)*pSrc++;
	}
	*pRes = '\0';
	return pRes;
}

static char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

//***********************************************************************
int BUtil::DecodeBase64(BYTE* pDst, LPCSTR pSrc)
{
	BYTE* pBase = pDst;
    static char inalphabet[256], decoder[256];
	static bool alpha_setup = false;

    int i, bits, c, char_count, errors = 0;

	if(! alpha_setup)
	{
		alpha_setup = true;
		for(i = strlen(alphabet); i >= 0 ; i--)
		{
			inalphabet[alphabet[i]] = 1;
			decoder[alphabet[i]]    = i;
		}
	}

    char_count = 0;
    bits = 0;

    while((c = *pSrc++) != _T('\0'))
	{
		if(c == _T('\r') || c == _T('\n'))
			break;
		if(c == _T('='))
			break;
		if(c > 255 || ! inalphabet[c])
			continue;

		bits += decoder[c];
		char_count++;

		if (char_count == 4)
		{
			*pDst++ =(bits >> 16);
			*pDst++ =((bits >> 8) & 0xff);
			*pDst++ =(bits & 0xff);
			bits = 0;
			char_count = 0;
		}
		else
		{
			bits <<= 6;
		}
	}
	switch(char_count)
	{
	case 1:
		errors++;
		break;
	case 2:
		*pDst++ =( bits >> 10);
		break;
	case 3:
		*pDst++ =( bits >> 16);
		*pDst++ =((bits >> 8) & 0xff);
		break;
	}
	*pDst = '\0';
	return errors ? -1 : (int)(pDst - pBase);
}

//***********************************************************************
int BUtil::EncodeBase64(LPSTR pDst, LPBYTE pSrc, int cbSrc, bool hexescape)
{
	char* pBase = pDst;
	BYTE  b1, b2, b3;
	DWORD d;
	int   k = 0;

	while(k < cbSrc)
	{
		b1 = pSrc[k];
		b2 = (k+1 < cbSrc) ? pSrc[k+1] : 0;
		b3 = (k+2 < cbSrc) ? pSrc[k+2] : 0;

		d = (b1 << 16) | (b2 << 8) | b3;
        
		if(hexescape)
		{
			char b;

			b = alphabet[(d>>18)];
			if(b == '+')
			{
				*pDst++ = '%';
				*pDst++ = '2';
				*pDst++ = 'B';
			}
			else if(b == '/')
			{
				*pDst++ = '%';
				*pDst++ = '2';
				*pDst++ = 'F';
			}
			else
			{
				*pDst++ = b;
			}
			b = alphabet[(d>>12)&0x3f];
			if(b == '+')
			{
				*pDst++ = '%';
				*pDst++ = '2';
				*pDst++ = 'B';
			}
			else if(b == '/')
			{
				*pDst++ = '%';
				*pDst++ = '2';
				*pDst++ = 'F';
			}
			else
			{
				*pDst++ = b;
			}
			if(k+1 < cbSrc)
			{
				b = alphabet[(d>>6)&0x3f];
				if(b == '+')
				{
					*pDst++ = '%';
					*pDst++ = '2';
					*pDst++ = 'B';
				}
				else if(b == '/')
				{
					*pDst++ = '%';
					*pDst++ = '2';
					*pDst++ = 'F';
				}
				else
				{
					*pDst++ = b;
				}
			}
			else
			{
				*pDst++ = '%';
				*pDst++ = '3';
				*pDst++ = 'D';
			}
			if(k+2 < cbSrc)
			{
				b = alphabet[(d)&0x3f];
				if(b == '+')
				{
					*pDst++ = '%';
					*pDst++ = '2';
					*pDst++ = 'B';
				}
				else if(b == '/')
				{
					*pDst++ = '%';
					*pDst++ = '2';
					*pDst++ = 'F';
				}
				else
				{
					*pDst++ = b;
				}
			}
			else
			{
				*pDst++ = '%';
				*pDst++ = '3';
				*pDst++ = 'D';
			}
		}
		else
		{
			*pDst++ =                  alphabet[((d>>18)        )];
			*pDst++ =                  alphabet[((d>>12) & 0x3f )];
			*pDst++ = (k+1 <  cbSrc) ? alphabet[((d>> 6) & 0x3f )] : '=';
			*pDst++ = (k+2 <  cbSrc) ? alphabet[((d    ) & 0x3f )] : '=';
		}
		k+= 3;
	}
	*pDst = '\0';
	return pDst - pBase;
}

//**************************************************************************
LPCTSTR BUtil::SimplePatternMatch(LPCTSTR string, LPCTSTR pattern)
{
	if(string == NULL && pattern == NULL)
		return NULL;
	if(pattern == NULL)
		return string;
	if(! _tcscmp(pattern, _T("*")))
		return string + _tcslen(string);

	while(*string && *pattern)
	{
		if(*pattern == _T('?'))
		{
			pattern++;
			string++;
		}
		else if(*pattern == _T('*'))
		{
			pattern++;
			if(! *pattern)
				return string;
			while(*string)
			{
				if(*string == *pattern)
				{
					LPCTSTR pm = SimplePatternMatch(string + 1, pattern + 1);
				
					if(pm) return pm;
				}
				string++;
			}
			return NULL;
		}
		else if(*pattern++ != *string++)
		{
			return NULL;
		}
	}
	if(! *pattern)
		return string;
	if(! *string && ! _tcscmp(pattern, _T("*"))) return string;
	return NULL;
}


#ifdef Windows
#define BYTE_SWAPPED 1
#endif

/*
    bdd - unicode->utf8->quoted-printable encode the data
*/
//**************************************************************************
bool BUtil::Utf8Encode(char* dst, const WCHAR* pSrc, bool bQuoteIt)
{
    unsigned char  utfbuf[8], ic, xd;
    unsigned char* src = (unsigned char*)pSrc;
    unsigned short ca, cb, cc, cd;
    wchar_t  uc, nc;
    int  j, k;
    size_t i, len;

#ifdef UNICODE
    len = wcslen((WCHAR*)pSrc);
#else
	WCHAR* ps = (WCHAR*)pSrc;
	
	for(len = 0; ps && *ps; ps++)
		len++;
#endif
    for(i = 0; i < len; i++)
    {
        /* first get the unicode character
        */
        ca = (unsigned char)*src++;
        cb = (unsigned char)*src++;
#if BYTE_SWAPPED
        nc = ca | (cb << 8);
#else
        nc = (ca << 8) | cb;
#endif        
        if(sizeof(WCHAR) == 4) // it is on Linux, etc.
        {
         cc = (unsigned char)*src++;
         cd = (unsigned char)*src++;
#if BYTE_SWAPPED
         uc = cc | (cd << 8);
         nc = nc | (uc << 16);
#else
         uc = (cc << 8) | cd;
         nc = (nc << 8) | uc;
#endif
        }
        /* next, UTF-8 encode it
        */
        j = 0;
        
        if (nc < 0x80) {
            utfbuf[j++] = (unsigned char)nc;
        }
        else if (nc < 0x800) {
            utfbuf[j++] = 0xC0 | (nc >> 6);
            utfbuf[j++] = 0x80 | (nc & 0x3F);
        }
        else if (nc < 0x10000) {
            utfbuf[j++] = 0xE0 | (nc >> 12);
            utfbuf[j++] = 0x80 | ((nc >> 6) & 0x3F);
            utfbuf[j++] = 0x80 | (nc  & 0x3F);
        }
        else if (nc < 0x200000) {
            utfbuf[j++] = 0xF0 | (nc >> 18);
            utfbuf[j++] = 0x80 | ((nc >> 12) & 0x3F);
            utfbuf[j++] = 0x80 | ((nc >> 6) & 0x3F);
            utfbuf[j++] = 0x80 | (nc  & 0x3F);
        }
 
        /* next, quoted-printable encode it or copy to output
        */
        if(bQuoteIt)
        {
            for(k = 0; k < j; k++)
            {
                ic = utfbuf[k];
            
                if(
                    (ic >= 33 && ic <= 60)          ||
                    (ic >= 62 && ic <= 126)
                    )
                {
                    *dst++ = ic;
                }
                else
                {
                    *dst++ = '=';                        
                    xd = ic >> 4;
                    if(xd < 10) xd += '0';
                    else        xd += ('A' - 10);
                    *dst++ = xd;
                    xd = ic &= 0xf;
                    if(xd < 10) xd += '0';
                    else        xd += ('A' - 10);
                    *dst++ = xd;
                }
            }
        }
        else
        {
            for(k = 0; k < j; k++)
            {
                *dst++ = (char)utfbuf[k];
            }
        }
    }
    *dst = '\0';
    return false;
}
 
//**************************************************************************
bool  BUtil::Utf8Decode(WCHAR* dst, const char* src)
{
	unsigned long b, c;
	
	while(*src)
	{
		b = *src++;
		
		if(b & 0x80)
		{
			if(b & 0x20)
			{
				if(b & 0x10)
				{
					b &= 0x7;
					b <<= 18;
					c = *src++;
					b |= (c & 0x3f) << 12;
					c = *src++;
					b |= (c & 0x3f) << 6;
					c = *src++;
					b |= (c & 0x3f);
				}
				else
				{
					b &= 0xf;
					b <<= 12;
					c = *src++;
					b |= (c & 0x3f) << 6;
					c = *src++;
					b |= (c & 0x3f);
				}
			}
			else
			{
				b &= 0x1f;
				b <<= 6;
				c = *src++;
				b |= c & 0x3f;
			}
		}
		*dst++ = (WCHAR)b;
	}
	*dst++ = 0;
	return false;
}

//**************************************************************************
int BUtil::UnescapeString(LPSTR pDst, LPCSTR pSrc)
{
	LPSTR pBase = pDst;

	if(! pSrc || ! pDst) return 0;

	while(*pSrc)
	{
		if(*pSrc == '%')
		{
			BYTE tv;
			BYTE av;
			char ac = *++pSrc;

			if(ac >= 'a' && ac <= 'f')		av = (BYTE)(ac - ('a' - 10));
			else if(ac >= 'A' && ac <= 'F') av = (BYTE)(ac - ('A' - 10));
			else							av = (BYTE)(ac - '0');

			tv = av << 4;
			ac = *++pSrc;
			pSrc++;

			if(ac >= 'a' && ac <= 'f')		av = (BYTE)(ac - ('a' - 10));
			else if(ac >= 'A' && ac <= 'F') av = (BYTE)(ac - ('A' - 10));
			else							av = (BYTE)(ac - '0');

			tv |= av;
			*pDst++ = tv;
		}
		else
		{
			*pDst++ = *pSrc++;
		}
	}
	*pDst = '\0';
	return (int)(pDst - pBase);
}

#ifdef UNICODE
//**************************************************************************
int BUtil::UnescapeString(LPTSTR pDst, LPCTSTR pSrc)
{
	LPTSTR pBase = pDst;

	if(! pSrc || ! pDst) return 0;

	while(*pSrc)
	{
		if(*pSrc == _T('%'))
		{
			BYTE tv;
			BYTE av;
			char ac = *++pSrc;

			if(ac >= _T('a') && ac <= _T('f'))		av = (BYTE)(ac - (_T('a') - 10));
			else if(ac >= _T('A') && ac <= _T('F')) av = (BYTE)(ac - (_T('A') - 10));
			else							av = (BYTE)(ac - _T('0'));

			tv = av << 4;
			ac = *++pSrc;
			pSrc++;

			if(ac >= _T('a') && ac <= _T('f'))		av = (BYTE)(ac - (_T('a') - 10));
			else if(ac >= _T('A') && ac <= _T('F')) av = (BYTE)(ac - (_T('A') - 10));
			else							av = (BYTE)(ac - _T('0'));

			tv |= av;
			*pDst++ = tv;
		}
		else
		{
			*pDst++ = *pSrc++;
		}
	}
	*pDst = _T('\0');
	return (int)(pDst - pBase) / sizeof(TCHAR);
}
#endif

//**************************************************************************
int BUtil::EscapeString(LPSTR pDst, LPCSTR pSrc)
{
	LPSTR pBase = pDst;

	if(! pSrc || ! pDst) return 0;

	while(*pSrc)
	{
		if(*pSrc == '%')
		{
			*pDst++ = '%';
			*pDst++ = '2';
			*pDst++ = '5';
		}
		else if(*pSrc < 0x20 || *pSrc >= 0x7f)
		{
			BYTE un, ln;

			un = *pSrc >> 4;
			ln = *pSrc & 0xf;

			*pDst++ = '%';
			*pDst++ = un >= 10 ? ('A' + (un - 10)) : (un + '0');
			*pDst++ = ln >= 10 ? ('A' + (ln - 10)) : (ln + '0');
		}
		else
		{
			*pDst++ = *pSrc;
		}
		pSrc++;
	}
	*pDst = '\0';
	return pDst - pBase;
}

//**************************************************************************
int BUtil::EscapeDWORD(LPSTR pDst, DWORD dwSrc)
{
	BYTE srcStr[6];

	memcpy(srcStr, &dwSrc, sizeof(DWORD));
	srcStr[4] = 0;
	return EscapeString(pDst, (LPSTR)srcStr);
}

//**************************************************************************
ERRCODE BUtil::GetHomePath(LPTSTR pbuf, int nBuf)
{
	const char* rv;
	int   len;
#ifdef Windows
	rv = "c:\\";
#else
	if((rv = getenv("HOME")) == NULL)
	{
		if((rv = getenv("home")) == NULL)
		{
#ifdef UNIX
			rv = "/home/";
#else
			rv = "/users/";
#endif
		}
	}
#endif
	if(! pbuf)
		return errBAD_PARAMETER;
	len = strlen(rv);
	if(nBuf <= len+1)
	{
		pbuf[0] = _T('\0');
		return errOVERFLOW;
	}
	CharToTChar(pbuf, rv);
	len = _tcslen(pbuf);
	if(len > 0 && pbuf[len-1] != _PTC_)
	{
		pbuf[len] = _PTC_;
		pbuf[len+1] = _T('\0');
	}
	return errOK;
}

//**************************************************************************
ERRCODE BUtil::GetTempPath(LPTSTR pbuf, int nBuf)
{
#ifdef Windows
	if(::GetTempPath(nBuf, pbuf))
		return errOK;
	return errFAILURE;
#else
	const char* rv;
	int   len;
	
	if((rv = getenv("TEMP")) == NULL)
	{
		if((rv = getenv("TMP")) == NULL)
		{
			rv = "/tmp";
		}
	}
	if(! pbuf)
		return errBAD_PARAMETER;
	len = strlen(rv);
	if(nBuf <= len+1)
	{
		pbuf[0] = _T('\0');
		return errOVERFLOW;
	}
	CharToTChar(pbuf, rv);
	len = _tcslen(pbuf);
	if(len > 0 && pbuf[len-1] != _PTC_)
	{
		pbuf[len] = _PTC_;
		pbuf[len+1] = _T('\0');
	}
	return errOK;
#endif
}

//**************************************************************************
ERRCODE BUtil::FileExists(LPCTSTR pName)
{
	struct stat finfo;
	int    srep;

	if (! pName || ! pName[0])
	{
		return errFAILURE;
	}
#ifdef UNICODE
	char* aname = new char [ _tcslen(pName) * 3 + 2 ];
	
	WCharToChar(aname, pName);
	srep = stat(aname, &finfo);
	
	delete [] aname;
#else	
	srep = stat(pName, &finfo);
#endif
	if(srep) 					return errFAILURE;
	if(finfo.st_mode & S_IFDIR)	return errFAILURE;
	return errOK;
}

//**************************************************************************
ERRCODE BUtil::DirectoryExists(LPCTSTR pName)
{
	struct stat finfo;
	int    srep, len;

	len = _tcslen(pName);
	char* aname = new char [ len * 2 + 322 ];
	
	TCharToChar(aname, pName);
	len = strlen(aname);
	if(len > 0 && aname[len-1] == '/' || aname[len-1] == '\\')
		aname[len-1] = '\0';
	srep = stat(aname, &finfo);
	delete [] aname;
	if(srep) 						return errFAILURE;
	if(! (finfo.st_mode & S_IFDIR))	return errFAILURE;
	return errOK;
}

//**************************************************************************
const WCHAR* BUtil::CharToTempWChar(LPCSTR pSrc)
{
	static WCHAR bsaCVT[MAX_URL_LEN];

	if(! pSrc || (strlen(pSrc) >= MAX_URL_LEN))
		return L"bsaToTempWChar: String Too Big To Convert";
	Utf8Decode(bsaCVT, pSrc);
	return bsaCVT;
}

//***********************************************************************
BlistElement::BlistElement(LPCTSTR pName, BlistElement* pNext)
{
	m_pName  = new TCHAR [ _tcslen(pName) + 2 ];
	_tcscpy(m_pName, pName);
	m_pNext  = pNext;
}

//***********************************************************************
BlistElement::~BlistElement()
{
	if(m_pName) delete [] m_pName;
}

//***********************************************************************
BlistElement* BlistElement::GetNext(BlistElement* pList)
{
	return pList ? pList->m_pNext : pList;
}

//***********************************************************************
BlistElement* BlistElement::FreeList(BlistElement* pList)
{
	BlistElement* pNext;

	while(pList)
	{
		pNext = pList->m_pNext;
		delete pList;
		pList = pNext;
	}
	return NULL;
}

//***********************************************************************
BlistElement* BlistElement::AddToList(BlistElement* pItem, BlistElement* pList)
{
	BlistElement* pNext;

	if(! pItem) return NULL;
	pItem->m_pNext = NULL;
	if(! pList)
		pList = pItem;
	else
	{
		for(pNext = pList; pNext->m_pNext; pNext = pNext->m_pNext)
			;
		pNext->m_pNext = pItem;
	}
	return pList;
}

//***********************************************************************
BlistElement* BlistElement::RemoveFromList(BlistElement* pItem, BlistElement* pList)
{
	BlistElement* pNext;

	if(! pItem || ! pList) return NULL;
	if(pItem == pList)
	{
		pList = pItem->m_pNext;
		return pList;
	}
	else
	{
		for(pNext = pList; pNext->m_pNext; pNext = pNext->m_pNext)
		{
			if(pNext->m_pNext == pItem)
			{
				pNext->m_pNext = pItem->m_pNext;
				break;
			}
		}
		return pList;
	}
}

//***********************************************************************
BlistElement* BlistElement::FindKey(LPCTSTR pName, const BlistElement* pList)
{
	BlistElement* pItem;

	for(pItem = (BlistElement*)pList; pItem; pItem = pItem->m_pNext)
		if(! _tcscmp(pName, pItem->m_pName)) return pItem;
	return NULL;
}


//**************************************************************************
Bmsg::Bmsg(int initSize) 
			:
			m_bufAlloc(initSize)
{
	if(initSize < 128)
		initSize = 128;

	m_pBuf		= new BYTE [ m_bufAlloc ];
	m_msgSize   = 0;
	m_ioCnt		= 0;
}

//**************************************************************************
Bmsg::Bmsg(LPSTR pMsg, int initSize)
			:
			m_pBuf((LPBYTE)pMsg),
			m_bufAlloc(0),
			m_msgSize(initSize)
{
	m_ioCnt		= 0;
}

//**************************************************************************
Bmsg::~Bmsg()
{
	if(m_pBuf && m_bufAlloc) delete [] m_pBuf;
}

//**************************************************************************
ERRCODE Bmsg::SkipBYTES(int nBytes)
{
	/*
	if(m_ioCnt + nBytes > m_bufAlloc)
	{
		m_ioCnt = m_bufAlloc;
		return errOVERFLOW;
	}
	*/
	m_ioCnt += nBytes;
	return errOK;
}

//**************************************************************************
ERRCODE Bmsg::AddBYTE(BYTE b)
{
	m_pBuf[m_ioCnt++] = b;
	return errOK;
}

//**************************************************************************
ERRCODE Bmsg::AddWORD(WORD w)
{
	m_pBuf[m_ioCnt++] = (BYTE) (w >> 8);
	m_pBuf[m_ioCnt++] = (BYTE) (w);
	return errOK;
}

//**************************************************************************
ERRCODE Bmsg::AddDWORD(DWORD w)
{
	m_pBuf[m_ioCnt++] = (BYTE) (w >> 24);
	m_pBuf[m_ioCnt++] = (BYTE) (w >> 16);
	m_pBuf[m_ioCnt++] = (BYTE) (w >> 8);
	m_pBuf[m_ioCnt++] = (BYTE) (w);
	return errOK;
}

//**************************************************************************
ERRCODE Bmsg::AddSwappedWORD(WORD w)
{
	m_pBuf[m_ioCnt++] = (BYTE) (w);
	m_pBuf[m_ioCnt++] = (BYTE) (w >> 8);
	return errOK;
}

//**************************************************************************
ERRCODE Bmsg::AddSwappedDWORD(DWORD w)
{
	m_pBuf[m_ioCnt++] = (BYTE) (w);
	m_pBuf[m_ioCnt++] = (BYTE) (w >> 8);
	m_pBuf[m_ioCnt++] = (BYTE) (w >> 16);
	m_pBuf[m_ioCnt++] = (BYTE) (w >> 24);
	return errOK;
}

//**************************************************************************
ERRCODE Bmsg::AddSTRING(LPCSTR s)
{
	ERRCODE ec;
	DWORD   len = strlen(s);

	ec = AddDWORD(len);

	while(ec == errOK && len-- > 0)
	{
		ec = AddBYTE((BYTE)*s++);
	}
	return ec;
}
#ifdef UNICODE
 //**************************************************************************
ERRCODE Bmsg::AddSTRING(LPCTSTR s)
{
	ERRCODE ec;
	LPSTR ps = new char [ _tcslen(s) * sizeof(TCHAR) + 4 ];

	UTF8ENCODE(ps, s);
	ec = AddSTRING(ps);
	delete [] ps;
	return ec;
}
#endif

//**************************************************************************
ERRCODE Bmsg::GetBYTE(BYTE& b)
{
	b = m_pBuf[m_ioCnt++];
	return errOK;
}

//**************************************************************************
ERRCODE Bmsg::GetWORD(WORD& w)
{
	w  = (WORD)m_pBuf[m_ioCnt++] << 8;
	w |= (WORD)m_pBuf[m_ioCnt++];
	return errOK;
}

//**************************************************************************
ERRCODE Bmsg::GetDWORD(DWORD& w)
{
	w  = (WORD)m_pBuf[m_ioCnt++] << 24;
	w |= (WORD)m_pBuf[m_ioCnt++] << 16;
	w |= (WORD)m_pBuf[m_ioCnt++] << 8;
	w |= m_pBuf[m_ioCnt++];
	return errOK;
}

//**************************************************************************
ERRCODE Bmsg::GetSwappedWORD(WORD& w)
{
	w  = (WORD)m_pBuf[m_ioCnt++];
	w |= (WORD)m_pBuf[m_ioCnt++] << 8;
	return errOK;
}

//**************************************************************************
ERRCODE Bmsg::GetSwappedDWORD(DWORD& w)
{
	w  = (DWORD)m_pBuf[m_ioCnt++];
	w |= (DWORD)m_pBuf[m_ioCnt++] << 8;
	w |= (DWORD)m_pBuf[m_ioCnt++] << 16;
	w |= (DWORD)m_pBuf[m_ioCnt++] << 24;
	return errOK;
}


//**************************************************************************
//**************************************************************************


#if (defined(Linux) || defined(OSX) || defined(Darwin) || defined(Unix)) && defined(UNICODE)

//***********************************************************************
int wcscasecmp(LPCWSTR pA, LPCWSTR pB)
{
	WCHAR a, b;
	
	while(*pA && *pB)
	{
		a = *pA++;
		b = *pB++;
		
		if(! a)
			if(b)
				return -1;
			else
				return 0;
		else if(! b)
			return 1;
		
		if(a >= L'a' && a <= L'z')
			a = a - L'a' + L'A';
		if(b >= L'a' && b <= L'z')
			b = b - L'b' + L'B';
		if(a < b)
			return -1;
		else if(a > b)
			return 1;
	}
	if(*pA && ! *pB)
		return 1;
	if(! *pA && *pB)
		return -1;
	return 0;
}

//***********************************************************************
int wcsncasecmp(LPCWSTR pA, LPCWSTR pB, int n)
{
	WCHAR a, b;
	
	while(*pA && *pB && (n > 0))
	{
		a = *pA++;
		b = *pB++;
		n--;
		
		if(! a)
			if(b)
				return -1;
			else
				return 0;
		else if(! b)
			return 1;
		
		if(a >= L'a' && a <= L'z')
			a = a - L'a' + L'A';
		if(b >= L'a' && b <= L'z')
			b = b - L'b' + L'B';
		if(a < b)
			return -1;
		else if(a > b)
			return 1;
	}
	if(*pA && ! *pB)
		return 1;
	if(! *pA && *pB)
		return -1;
	return 0;
}

#endif

#if defined(SOLARIS)

//***********************************************************************
int snprintf(char* d, size_t n, const char* frmt, ...)
{
	va_list  arg;
    int      i;

	va_start(arg, frmt);
	i = vsprintf(d, frmt, arg);
	va_end ( arg );

	return i;
}
#endif

//***********************************************************************
LPCTSTR BUtil::ExplainError(ERRCODE ec)
{
	switch(ec)
	{
	case errOK:
	case errFAILURE:

	// os errors
	//
	case errNOT_IMPLEMENTED:			return _T("NOT_IMPLEMENTED");
	case errNO_MEMORY:					return _T("NO_MEMORY");
	case errTIMEOUT:					return _T("TIMEOUT");
	case errTHREAD_CREATE:				return _T("THREAD_CREATE");
	case errTHREAD_STOP:				return _T("THREAD_STOP");

	// generic errors
	//
	case errOVERFLOW:					return _T("OVERFLOW");
	case errUNDERFLOW:					return _T("UNDERFLOW");
	case errPERMISSION:					return _T("PERMISSION");
	case errBAD_PARAMETER:				return _T("BAD_PARAMETER");
	case errBAD_INTEGER_FORMAT:			return _T("BAD_INTEGER_FORMAT");
	case errBAD_STRING_FORMAT:			return _T("BAD_STRING_FORMAT");

	// SNMP errors
	case errOID_ELEMENT_OVERFLOW:		return _T("OID_ELEMENT_OVERFLOW");
	case errOID_OVERFLOW:				return _T("OID_OVERFLOW");
	case errBAD_OID_CHAR_FORMAT:		return _T("BAD_OID_CHAR_FORMAT");
	case errBAD_OID_FORMAT:				return _T("BAD_OID_FORMAT");
	case errNO_OID:						return _T("NO_OID");
	
	case errBAD_RESPONSE_LENGTH:		return _T("BAD_RESPONSE_LENGTH");
	case errBAD_RESPONSE_HEADER:		return _T("BAD_RESPONSE_HEADER");
	case errBAD_RESPONSE_VERSION_FORMAT:return _T("BAD_RESPONSE_VERSION_FORMAT");
	case errBAD_RESPONSE_VERSION:		return _T("BAD_RESPONSE_VERSION");
	case errBAD_RESPONSE_COMMUNITY_FORMAT:return _T("BAD_RESPONSE_COMMUNITY_FORMAT");
	case errBAD_RESPONSE_CODE:			return _T("BAD_RESPONSE_CODE");
	case errBAD_RESPONSE_NUMBER_FORMAT:	return _T("BAD_RESPONSE_NUMBER_FORMAT");
	case errBAD_RESPONSE_ERRORCODE_FORMAT:return _T("BAD_RESPONSE_ERRORCODE_FORMAT");
	case errBAD_RESPONSE_ERROR_RETURNED:return _T("BAD_RESPONSE_ERROR_RETURNED");
	case errBAD_RESPONSE_STRUCT_EXPECTED:return _T("BAD_RESPONSE_STRUCT_EXPECTED");
	case errBAD_RESPONSE_INTEGER:		return _T("BAD_RESPONSE_INTEGER");
	case errBAD_RESPONSE_STRING:		return _T("BAD_RESPONSE_STRING");
	case errBAD_RESPONSE_OID:			return _T("BAD_RESPONSE_OID");

	case errOBJECT_NOT_FOUND:			return _T("OBJECT_NOT_FOUND");
	
	case errUNHANDLED_TYPE:				return _T("UNHANDLED_TYPE");
	case errUNHANDLED_VARBIND:			return _T("UNHANDLED_VARBIND");
	case errUNHANDLED_PDU:				return _T("UNHANDLED_PDU");

	// stream io errors
	case errSTREAM_READ:				return _T("STREAM_READ");
	case errSTREAM_WRITE:				return _T("STREAM_WRITE");
	case errSTREAM_OPEN:				return _T("STREAM_OPEN");
	case errSTREAM_IOCTL:				return _T("STREAM_IOCTL");
	case errSTREAM_CLOSE:				return _T("STREAM_CLOSE");
	case errSTREAM_EOF:					return _T("STREAM_EOF");
	case errSTREAM_NOT_OPEN:			return _T("STREAM_NOT_OPEN");
	case errSTREAM_TIMEOUT:				return _T("STREAM_TIMEOUT");
	case errSTREAM_DATA_PENDING:		return _T("STREAM_DATA_PENDING");
	case errSTREAM_UNSUPPORTED_ENCODING:return _T("STREAM_UNSUPPORTED_ENCODING");

	// socket errors
	case errSOCKET_NO_INTERFACE:		return _T("SOCKET_NO_INTERFACE");
	case errSOCKET_NO_ADDRESS:			return _T("SOCKET_NO_ADDRESS");
	case errSOCKET_ADDR_NOT_RESOLVED:	return _T("SOCKET_ADDR_NOT_RESOLVED");
	case errSOCKET_OPEN_FAILURE:		return _T("SOCKET_OPEN_FAILURE");
	case errSOCKET_IOCTL_FAILURE:		return _T("SOCKET_IOCTL_FAILURE");
	case errSOCKET_CONNECT_FAILURE:		return _T("SOCKET_CONNECT_FAILURE");
	case errSOCKET_SELECT_FAILURE:		return _T("SOCKET_SELECT_FAILURE");
	case errSOCKET_BIND_FAILURE:		return _T("SOCKET_BIND_FAILURE");
	case errSOCKET_ACCEPT_FAILURE:		return _T("SOCKET_ACCEPT_FAILURE");
	case errSOCKET_LISTEN_FAILURE:		return _T("SOCKET_LISTEN_FAILURE");

	// database connector errors
	case errDBC_CANT_CREATE_DB:			return _T("DBC_CANT_CREATE_DB");
	case errDBC_CANT_CREATE_DSN:		return _T("DBC_CANT_CREATE_DSN");
	case errDBC_CANT_CONNECT:			return _T("DBC_CANT_CONNECT");
	case errDBC_NO_DATABASE:			return _T("DBC_NO_DATABASE");
	case errDBC_NO_MORE_ROWS:			return _T("DBC_NO_MORE_ROWS");
	case errDBC_NO_MORE_COLUMNS:		return _T("DBC_NO_MORE_COLUMNS");
	case errDBC_NO_QUERY:				return _T("DBC_NO_QUERY");
	case errDBC_NO_TRANSACTION:			return _T("DBC_NO_TRANSACTION");
	case errDBC_QUERY_FAILED:			return _T("DBC_QUERY_FAILED");
	case errDBC_COLUMN_ASCENSION:		return _T("DBC_COLUMN_ASCENSION");
	case errDBC_ROW_ASCENSION:			return _T("DBC_ROW_ASCENSION");
	
	// url errors
	case errURL_TOO_LONG:				return _T("URL_TOO_LONG");
	case errURL_NO_ACCESS:				return _T("URL_NO_ACCESS");
	case errURL_BAD_PROTOCOL:			return _T("URL_BAD_PROTOCOL");
	case errURL_BAD_HOSTNAME:			return _T("URL_BAD_HOSTNAME");
	case errURL_BAD_PORT:				return _T("URL_BAD_PORT");
	case errURL_BAD_PATH:				return _T("URL_BAD_PATH");
	case errURL_NOT_FOUND:				return _T("URL_NOT_FOUND");

	// http errors
	case errHTTP_SERVER_START:			return _T("HTTP_SERVER_START");
	case errHTTP_OPEN:					return _T("HTTP_OPEN");
	case errHTTP_CONNECT:				return _T("HTTP_CONNECT");
	case errHTTP_READ_HEADER:			return _T("HTTP_READ_HEADER");
	case errHTTP_READ_DATA:				return _T("HTTP_READ_DATA");
	case errHTTP_READ_TIMEOUT:			return _T("HTTP_READ_TIMEOUT");
	case errHTTP_SEND_TIMEOUT:			return _T("HTTP_SEND_TIMEOUT");
	case errHTTP_GET:					return _T("HTTP_GET");
	case errHTTP_POST:					return _T("HTTP_POST");
	case errHTTP_UNSUPPORTED_REQUEST:	return _T("HTTP_UNSUPPORTED_REQUEST");
	case errHTTP_HEADERS_TOO_LONG:		return _T("HTTP_HEADERS_TOO_LONG");
	case errHTTP_OVERFLOW:				return _T("HTTP_OVERFLOW");

	// xml errors
	case errXML_NO_STREAM:				return _T("XML_NO_STREAM");
	case errXML_BAD_ELEMENT:			return _T("XML_BAD_ELEMENT");
	case errXML_BAD_VERSION:			return _T("XML_BAD_VERSION");
	case errXML_BAD_VERSION_SPEC:		return _T("XML_BAD_VERSION_SPEC");
	case errXML_BAD_ATTRIBUTE:			return _T("XML_BAD_ATTRIBUTE");
	case errXML_MISSING_OPEN_ELEMENT:	return _T("XML_MISSING_OPEN_ELEMENT");
	case errXML_NO_ATTRIB_VALUE:		return _T("XML_NO_ATTRIB_VALUE");
	case errXML_NO_ATTRIB_QUOTE:		return _T("XML_NO_ATTRIB_QUOTE");
	case errXML_NO_END_COMMENT:			return _T("XML_NO_END_COMMENT");

	default:
		break;
	}
	return _T("Bad errCODE");
}

