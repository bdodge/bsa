//--------------------------------------------------------------------
//
// File: genos.h
// Desc: generic os common defines
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------

#ifndef _GEN_OS_H_
#define _GEN_OS_H_

#ifdef _UNICODE
#ifndef UNICODE
#define UNICODE 1
#endif
#elif defined(UNICODE)
#define _UNICODE 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/stat.h>

//-----------------------------------------------------------------------
//
// os specific headers
//
#ifdef Windows
	#ifdef __GNUC__
	    #define _WIN32_WINNT 0x501
	#endif
	#define Windows_LEAN_AND_MEAN
	#include <windows.h>
	#include <process.h>
	#include <winsock.h>
	#include <wininet.h>
	#include <tchar.h>
	#define WIN9X ((LOWORD(GetVersion()) == 4) && (GetVersion() & 0x80000000L))
	#define getcwd _getcwd
	#define stat   _stat
	#define getpid _getpid
#else
#ifdef X11
	#include <X11/Xlib.h>
	#include <X11/Xutil.h>
#endif
	#include <pthread.h>
	#include <errno.h>
	#include <stdarg.h>
	#include <unistd.h>
	#include <sys/ioctl.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <sys/un.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netinet/tcp.h>
	#include <net/if.h>
	#include <dlfcn.h>
	#define __USE_ISOC99 1
	#include <wchar.h>
	#include <math.h>
	#ifndef min 
		#define min(a,b) (((a)<=(b))?(a):(b))
		#define max(a,b) (((a)>(b))?(a):(b))
	#endif
	#define  MAX_PATH	1024
#endif


#ifdef Solaris
	#include <sys/filio.h>
	#ifdef __cplusplus
	extern "C" {
	#endif
	/* for some reason, these aren't protod */
	extern int ftime(struct timeb*);
	extern int usleep(unsigned int);
	/* and these weren't around */
	#ifdef UNICODE
	extern int _sntprintf(wchar_t* pd, size_t n, const wchar_t* f, ...);
	#else
	extern int snprintf(char* pd, size_t n, const char* f, ...);
	#endif
	#ifdef __cplusplus
	}
	#endif
#endif

//-----------------------------------------------------------------------
//
// basic types 
//
#ifdef Windows
	typedef int				socklen_t;
#else
	typedef unsigned char	BYTE,  *LPBYTE, *PBYTE;
	typedef unsigned short	WORD,  *LPWORD, *PWORD;
	typedef unsigned 		DWORD, *LPDWORD;
	typedef unsigned long	ULONG;
	typedef char			CHAR,  *LPSTR;
	typedef const char	    *LPCSTR;
	typedef void			*LPVOID;	
	typedef int				SOCKET;

	#define INVALID_SOCKET	-1
	#define SOCKET_ERROR	-1
	
	#define __declspec(a)
	#define CALLBACK
	#define WINAPI
#endif

//-----------------------------------------------------------------------
//
// dynamic library types 
//
#ifdef Windows
	typedef HMODULE HDYNLIB;
	typedef FARPROC	PDYNFUNCTION;
	typedef FARPROC	LPDYNFUNCTION;
#else
	typedef void*	HDYNLIB;
	typedef void 	(*PDYNFUNCTION)(void);
	typedef void 	(*LPDYNFUNCTION)(void);
#endif

//-----------------------------------------------------------------------
//
// char types 
//
#ifdef Windows
#else
	typedef wchar_t WCHAR, *LPWSTR;
	typedef const wchar_t  *LPCWSTR;
#ifdef UNICODE
	typedef wchar_t	tchar_t;
	#define _T(a)	L ## a
#else
	typedef char	tchar_t;
	#define _T(a)	a
#endif
typedef tchar_t		TCHAR, *LPTSTR;
typedef const tchar_t	   *LPCTSTR;
#endif

#ifndef Windows
#ifdef UNICODE
	#define _tcscpy	wcscpy
	#define _tcscat	wcscat
	#define _tcslen	wcslen
	#define _tcsstr wcsstr
	#define _tcscmp wcscmp
	#define _tcsicmp wcsicmp
	#define _tcsncpy wcsncpy
	#define _tcsncmp wcsncmp
	#define _tcsnicmp wcsncasecmp
	#if defined(Darwin) || defined(xOSX)
	/* hand implemented in utils */
	/*   update, Apple got off their asses and prototyped these in xcode 4.5 */
	#ifdef __cplusplus
	extern "C" {
	#endif
	extern int wcscasecmp(const wchar_t*, const wchar_t*);
	extern int wcsncasecmp(const wchar_t*, const wchar_t*, int);
	#ifdef __cplusplus
	}
	#endif
	#endif
	#define _tcstol  wcstol
	#define _tcstod  wcstod
	//#define _tfopen  bsafopen must include bsabs.h
	#define	_vsntprintf	vswprintf
	#ifndef Solaris /* not really solaris, just really old gcc */
		#define _sntprintf	swprintf
	#endif
	#define _tprintf	wprintf
	#define _snprintf	snprintf
	#define OutputDebugString(a) (fwprintf(stderr, a))
#else
	#define _tcscpy	strcpy
	#define _tcscat	strcat
	#define _tcslen	strlen
	#define _tcsstr strstr
	#define _tcscmp strcmp
	#define _tcsicmp stricmp
	#define _tcsncpy strncpy
	#define _tcsncmp strncmp
	#define _tcsncmp strncmp
	#define _tcsnicmp strncasecmp
	#define _tcstol  strtol
	#define _tcstod  strtod
	//#define _tfopen  fopen
	#define	_vsntprintf	vsnprintf
	#define _sntprintf	snprintf
	#define _snprintf	snprintf
	#define _tprintf	printf
	#define OutputDebugString(a) (fprintf(stderr, a))
#endif
	#define	stricmp		strcasecmp
	#define	strnicmp	strncasecmp
	#define	wcsicmp		wcscasecmp
	#define	wcnicmp		wcsncasecmp
#endif

//-----------------------------------------------------------------------
//
// byte orientation 
//
#ifdef Windows
	#ifndef ARCH
		#define ARCH	I386
		#define BYTE_SWAPPED	1
	#endif
#elif defined(Linux)
	#ifndef ARCH
		#define ARCH	I386
		#define BYTE_SWAPPED	1
	#endif
#elif defined(OSX)
	#ifndef ARCH
		#define ARCH	I386
		#define BYTE_SWAPPED	1
	#endif
#elif defined(Darwin)
	#ifndef ARCH
		#define ARCH	PPC
		#define BYTE_SWAPPED	0
	#endif
#elif defined(Solaris)
	#ifndef ARCH
		#define ARCH	SPARC
	#endif
#endif

#ifndef ARCH
	#pragma warning("Assuming Big-Endian Architecture")
#endif




//-----------------------------------------------------------------------
//
// file-path terminations
//
#ifdef Windows
#define  _PTC_  _T('\\')
#define  _PTS_  _T("\\") 
#else
#define  _PTC_  _T('/')
#define  _PTS_  _T("/")
#endif


//-----------------------------------------------------------------------
//
// basic limits
//
// max length of File Path
//
#ifdef Windows
	#define MAX_PATH_LEN	MAX_PATH
#else
	#define MAX_PATH_LEN	1024
#endif

// max length of a URL
//
// Here is some research as to why I set this:
//
//	Internet Explorer has a maximum uniform resource locator (URL) length of 2,083
//  characters, with a maximum path length of 2,048 characters. This limit applies
//  to both POST and GET request URLs. 
//
//  If you are using the GET method, you are limited to a maximum of 2,048 characters
//  (minus the number of characters in the actual path, of course).
//  POST, however, is not limited by the size of the URL for submitting name/value pairs,
//  because they are transferred in the header and not the URL. 
//
//  RFC 2616, Hypertext Transfer Protocol -- HTTP/1.1,
//            does not specify any requirement for URL length. 
//
#define MAX_URL_LEN			2048

// max size of protocol portion of url
//
#define MAX_URL_PROTO_LEN	32

// max length of a network hostname (generally is is 63 chars + term for most IP systems)
//
#define MAX_HOST_NAME		128

// max length of an IP address
//
#define MAX_IP_ADDR			16

// max length of a password string
//
#define MAX_PASSWORD_LEN	64

// size of buffered I/O buffers
//
#define BS_STREAM_BUFFER_SIZE	512

//-----------------------------------------------------------------------
//
// text encodings
//
enum TEXTENCODING
{
	txtANSI, txtUTF8, txtUCS2, txtUCS2SWAP, txtUCS4, txtUCS4SWAP
};

//-----------------------------------------------------------------------
//
// error codes
//
enum ERRCODE
{
	// all OK
	//
	errNONE			= 0,
	errOK			= errNONE,
	errSUCCESS		= errNONE,

	errFAILURE,

	// os errors
	//
	errNOT_IMPLEMENTED,
	errNO_MEMORY,
	errTIMEOUT,
	errTHREAD_CREATE,
	errTHREAD_STOP,

	// generic errors
	//
	errOVERFLOW,
	errUNDERFLOW,
	errPERMISSION,
	errBAD_PARAMETER,
	errBAD_INTEGER_FORMAT,
	errBAD_STRING_FORMAT,

	// SNMP errors
	errOID_ELEMENT_OVERFLOW,
	errOID_OVERFLOW,
	errBAD_OID_CHAR_FORMAT,
	errBAD_OID_FORMAT,
	errNO_OID,
	
	errBAD_RESPONSE_LENGTH,
	errBAD_RESPONSE_HEADER,
	errBAD_RESPONSE_VERSION_FORMAT,
	errBAD_RESPONSE_VERSION,
	errBAD_RESPONSE_COMMUNITY_FORMAT,
	errBAD_RESPONSE_CODE,
	errBAD_RESPONSE_NUMBER_FORMAT,
	errBAD_RESPONSE_ERRORCODE_FORMAT,
	errBAD_RESPONSE_ERROR_RETURNED,
	errBAD_RESPONSE_STRUCT_EXPECTED,
	errBAD_RESPONSE_INTEGER,
	errBAD_RESPONSE_STRING,
	errBAD_RESPONSE_OID,

	errOBJECT_NOT_FOUND,
	
	errUNHANDLED_TYPE,
	errUNHANDLED_VARBIND,
	errUNHANDLED_PDU,

	// stream io errors
	errSTREAM_READ,
	errSTREAM_WRITE,
	errSTREAM_OPEN,
	errSTREAM_IOCTL,
	errSTREAM_CLOSE,
	errSTREAM_EOF,
	errSTREAM_NOT_OPEN, 
	errSTREAM_TIMEOUT,
	errSTREAM_DATA_PENDING,
	errSTREAM_UNSUPPORTED_ENCODING,

	// socket errors
	errSOCKET_NO_INTERFACE,
	errSOCKET_NO_ADDRESS,
	errSOCKET_ADDR_NOT_RESOLVED,
	errSOCKET_OPEN_FAILURE,
	errSOCKET_IOCTL_FAILURE,
	errSOCKET_CONNECT_FAILURE,
	errSOCKET_SELECT_FAILURE,
	errSOCKET_BIND_FAILURE,
	errSOCKET_ACCEPT_FAILURE,
	errSOCKET_LISTEN_FAILURE,

	// database connector errors
	errDBC_CANT_CREATE_DB,
	errDBC_CANT_CREATE_DSN,
	errDBC_CANT_CONNECT,
	errDBC_NO_DATABASE,
	errDBC_NO_MORE_ROWS,
	errDBC_NO_MORE_COLUMNS,
	errDBC_NO_QUERY,
	errDBC_NO_TRANSACTION,
	errDBC_QUERY_FAILED,
	errDBC_COLUMN_ASCENSION,
	errDBC_ROW_ASCENSION,

	// url errors
	errURL_TOO_LONG,
	errURL_NO_ACCESS,
	errURL_BAD_PROTOCOL,
	errURL_BAD_HOSTNAME,
	errURL_BAD_PORT,
	errURL_BAD_PATH,
	errURL_NOT_FOUND,

	// http errors
	errHTTP_SERVER_START,
	errHTTP_OPEN,
	errHTTP_CONNECT,
	errHTTP_READ_HEADER,
	errHTTP_READ_DATA,
	errHTTP_READ_TIMEOUT,
	errHTTP_SEND_TIMEOUT,
	errHTTP_GET,
	errHTTP_POST,
	errHTTP_UNSUPPORTED_REQUEST,
	errHTTP_HEADERS_TOO_LONG,
	errHTTP_OVERFLOW,

	// xml errors
	errXML_NO_STREAM,
	errXML_BAD_ELEMENT,
	errXML_BAD_VERSION,
	errXML_BAD_VERSION_SPEC,
	errXML_BAD_ATTRIBUTE,
	errXML_MISSING_OPEN_ELEMENT,
	errXML_NO_ATTRIB_VALUE,
	errXML_NO_ATTRIB_QUOTE,
	errXML_NO_END_COMMENT
 };


#endif

