#include "bsx.h"

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define GET_CTX() ((struct ssk_st*)m_sslctx))

#define KEYFILE_PRIV 	"/home/bdodge/.ssh/id_rsa"
#define KEYFILE_PUB 	"~/.ssh/id_rsa"

#define CERTFILE 	 	"cert.pem"

//***********************************************************************
//***********************************************************************
//***********************************************************************
// io bytestream class based on TLS on connection oriented TCP/IP socket
//
static SSL_CTX *s_ssl_ctx;
bool BtlsStream::m_ssh_inited;

static void TLSerror(const TCHAR *blurb)
{
	char errbuf[128];
	int sslerr = ERR_get_error();
	if (sslerr && (sslerr != SSL_ERROR_WANT_READ && sslerr != SSL_ERROR_WANT_WRITE))
	{
		_tprintf(_T("" _Pfs_ " %4d " _Pfs_ "\n"), blurb, sslerr, ERR_error_string(sslerr, errbuf));
	}
	else
	{
		_tprintf(_T("Unknown SSL error: %d\n"), sslerr);
	}
	fflush(stdout);
}

static ERRCODE TLSinit()
{
	const SSL_METHOD *meth;

	// do this one time only!
	//
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	ERR_load_BIO_strings();

	meth        = SSLv23_method();
	s_ssl_ctx   = SSL_CTX_new((SSL_METHOD *)meth);
	if (s_ssl_ctx == NULL)
	{
	    printf("Can't create SSL context\n");
	    return errFAILURE;
	}
	const long flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3;
	SSL_CTX_set_options(s_ssl_ctx, flags);

	if (! (SSL_CTX_use_certificate_file(s_ssl_ctx, CERTFILE, SSL_FILETYPE_PEM)))
	{
		TLSerror(_T("Can't read key file"));
		//return errFAILURE;
	}
	//SSL_CTX_set_default_passwd_cb(s_ssl_ctx, GetSSLPW);
#if 0
	// point to key file
	if (! (SSL_CTX_use_PrivateKey_file(s_ssl_ctx, KEYFILE_PRIV, SSL_FILETYPE_PEM)))
	{
		TLSerror(_T("Can't read key file"));
		//return errFAILURE;
	}
#endif	
	return errOK;
}

//***********************************************************************
BtlsStream::BtlsStream(bool nbio, bool ndelay) : BtcpStream(nbio, ndelay)
{
	if (! BtlsStream::m_ssh_inited)
    {
		TLSinit();
        BtlsStream::m_ssh_inited = true;
	}
}

//***********************************************************************
BtlsStream::BtlsStream(SOCKET sock, LPCSTR pIP, short port) : BtcpStream(sock, pIP, port)
{
	if (! BtlsStream::m_ssh_inited)
    {
		TLSinit();
        BtlsStream::m_ssh_inited = true;
	}
	if(sock != INVALID_SOCKET)
	{
		// already have a TCP connection, wrap it in TLS layer
		//
		// [TODO]
	}
}

//***********************************************************************
BtlsStream::~BtlsStream()
{
}

//***********************************************************************
ERRCODE BtlsStream::Close(void)
{
	return BtcpStream::Close();
}

//***********************************************************************
ERRCODE BtlsStream::Read(BYTE* pBuf, int& cnt)
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
ERRCODE BtlsStream::Write(BYTE* pBuf, int& cnt)
{
	int		ret;
	return errOK;
}

//***********************************************************************
ERRCODE BtlsStream::Pend(int to_secs, int to_usecs)
{
	// need to call read/write to force protocol fwd
	return errSTREAM_DATA_PENDING;
}

//***********************************************************************
ERRCODE BtlsStream::Connect(int to_secs, int to_usecs)
{
	struct ssl_st *ssl;
    BIO *pbio;
    char errbuf[80];
    int  sslerr;
	int ret;
	
	if (m_hSock == INVALID_SOCKET)
	{
		return errSOCKET_CONNECT_FAILURE;
	}
	ret = BtcpStream::Connect(to_secs, to_usecs);
	if(ret < 0)
	{
		return errSOCKET_CONNECT_FAILURE;
	}
	// read any un-encrytped data
	do
	{
		ret = BtcpStream::Pend(0, 100000);
		if (ret > 0)
		{
			int count;
			BYTE junk[32];
			
			count = sizeof(junk);
			BtcpStream::Read(junk, count);
			if (count <= 0)
			{
				break;
			}
			_tprintf(_T("Read %d junk bytes\n"), count);
		}
	}
	while (ret > 0);
	
	ssl = SSL_new(s_ssl_ctx);
	m_sslctx = ssl;
	
	// setup an openssh BIO with the original tcp socket
	//
#if 1
	pbio = BIO_new_socket(m_hSock, BIO_NOCLOSE);
	BIO_set_nbio(pbio, 1);
	SSL_set_bio(ssl, pbio, pbio);
#else
	SSL_set_fd(ssl, m_hSock);
#endif
	// Connect the SSL socket to tcp socket using connect for
	// client side
	//
	if(SSL_connect(ssl) <= 0)
	{
		int sslerr = ERR_get_error();
		_tprintf(_T("" _Pfs_ " %4d " _Pfs_ "\n"), _T("oh no\n"), sslerr, ERR_error_string(sslerr, errbuf));
		TLSerror(_T("SSL Connect"));
		return errSOCKET_CONNECT_FAILURE;
	}
	else
	{
	 	printf("SSL connected!\n");
		if (1 /*require_server_auth*/)
		{
			X509* peer;
			char  peer_CN[256];
			
			if (SSL_get_verify_result(ssl) != X509_V_OK)
			{
				sslerr = ERR_get_error();
				printf("SSL Certificate doesn't verify\n");
				
				// Check the common name
				//
				peer = SSL_get_peer_certificate(ssl);
				X509_NAME_get_text_by_NID(
						X509_get_subject_name(peer),
						NID_commonName,
						peer_CN,
						256
				);
			}
		}
	}
	if (sslerr)
	{
		if (ssl)
		{
			SSL_free(ssl);
			ssl = NULL;
		}
	}
	
	return errOK;
}


