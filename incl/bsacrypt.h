#ifndef CRYPT_H_
#define CRYPT_H_ 1
/* crypto/rc4/rc4.h */
/* Copyright (C) 1995-1997 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */


#ifdef  __cplusplus
extern "C" {
#endif

//------------------------ MD5 -------------------------------

/*
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * ! MD5_LONG has to be at least 32 bits wide. If it's wider, then !
 * ! MD5_LONG_LOG2 has to be defined along.			   !
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

#if defined(WIN16) || defined(__LP32__)
#define MD5_LONG unsigned long
#elif defined(_CRAY) || defined(__ILP64__)
#define MD5_LONG unsigned long
#define MD5_LONG_LOG2 3
/*
 * _CRAY note. I could declare short, but I have no idea what impact
 * does it have on performance on none-T3E machines. I could declare
 * int, but at least on C90 sizeof(int) can be chosen at compile time.
 * So I've chosen long...
 *					<appro@fy.chalmers.se>
 */
#else
#define MD5_LONG unsigned int
#endif

#define MD5_CBLOCK	64
#define MD5_LBLOCK	(MD5_CBLOCK/4)
#define MD5_DIGEST_LENGTH 16

typedef struct MD5state_st
	{
	MD5_LONG A,B,C,D;
	MD5_LONG Nl,Nh;
	MD5_LONG data[MD5_LBLOCK];
	int num;
	} MD5_CTX;

void MD5_Init(MD5_CTX *c);
void MD5_Update(MD5_CTX *c, const void *data, unsigned long len);
void MD5_Final(unsigned char *md, MD5_CTX *c);
unsigned char *MD5(const unsigned char *d, unsigned long n, unsigned char *md);
void MD5_Transform(MD5_CTX *c, const unsigned char *b);

//------------------------ SHA -------------------------------


/*
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * ! SHA_LONG has to be at least 32 bits wide. If it's wider, then !
 * ! SHA_LONG_LOG2 has to be defined along.                        !
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

#if defined(WIN16) || defined(__LP32__)
#define SHA_LONG unsigned long
#elif defined(_CRAY) || defined(__ILP64__)
#define SHA_LONG unsigned long
#define SHA_LONG_LOG2 3
#else
#define SHA_LONG unsigned int
#endif

#define SHA_LBLOCK	16
#define SHA_CBLOCK	(SHA_LBLOCK*4)	/* SHA treats input data as a
					 * contiguous array of 32 bit
					 * wide big-endian values. */
#define SHA_LAST_BLOCK  (SHA_CBLOCK-8)
#define SHA_DIGEST_LENGTH 20

typedef struct SHAstate_st
	{
	SHA_LONG h0,h1,h2,h3,h4;
	SHA_LONG Nl,Nh;
	SHA_LONG data[SHA_LBLOCK];
	int num;
	} SHA_CTX;

#ifndef NO_SHA0
void SHA_Init(SHA_CTX *c);
void SHA_Update(SHA_CTX *c, const void *data, unsigned long len);
void SHA_Final(unsigned char *md, SHA_CTX *c);
unsigned char *SHA(const unsigned char *d, unsigned long n,unsigned char *md);
void SHA_Transform(SHA_CTX *c, const unsigned char *data);
#endif
#ifndef NO_SHA1
void SHA1_Init(SHA_CTX *c);
void SHA1_Update(SHA_CTX *c, const void *data, unsigned long len);
void SHA1_Final(unsigned char *md, SHA_CTX *c);
unsigned char *SHA1(const unsigned char *d, unsigned long n,unsigned char *md);
void SHA1_Transform(SHA_CTX *c, const unsigned char *data);
#endif

//------------------------ RC4 -------------------------------

/* using int types make the structure larger but make the code faster
 * on most boxes I have tested - up to %20 faster. */
/*
 * I don't know what does "most" mean, but declaring "int" is a must on:
 * - Intel P6 because partial register stalls are very expensive;
 * - elder Alpha because it lacks byte load/store instructions;
 */
#define RC4_INT unsigned int

/*
 * This enables code handling data aligned at natural CPU word
 * boundary. See rc4_enc.c for further details.
 */
#undef RC4_CHUNK

/* if this is defined data[i] is used instead of *data, this is a %20
 * speedup on x86 */
#define RC4_INDEX

typedef struct rc4_key_st
	{
	RC4_INT x,y;
	RC4_INT data[256];
	} RC4_KEY;

 
const char *RC4_options(void);
void RC4_set_key(RC4_KEY *key, int len, const unsigned char *data);
void RC4(RC4_KEY *key, unsigned long len, const unsigned char *indata,
		unsigned char *outdata);

//------------------------ RSA -------------------------------

typedef struct rsa_st RSA;

#define RSA_METHOD_FLAG_NO_CHECK	0x01 /* don't check pub/private match */

#define RSA_FLAG_CACHE_PUBLIC		0x02
#define RSA_FLAG_CACHE_PRIVATE		0x04
#define RSA_FLAG_BLINDING			0x08
#define RSA_FLAG_THREAD_SAFE		0x10

/* This flag means the private key operations will be handled by rsa_mod_exp
 * and that they do not depend on the private key components being present:
 * for example a key stored in external hardware. Without this flag bn_mod_exp
 * gets called when private key components are absent.
 */
#define RSA_FLAG_EXT_PKEY			0x20

/* This flag in the RSA_METHOD enables the new rsa_sign, rsa_verify functions.
 */	
#define RSA_FLAG_SIGN_VER			0x40

#define RSA_FLAG_NO_BLINDING		0x80 /* new with 0.9.6j and 0.9.7b; the built-in
                                              * RSA implementation now uses blinding by
                                              * default (ignoring RSA_FLAG_BLINDING),
                                              * but other engines might not need it
                                              */

#define RSA_PKCS1_PADDING	1
#define RSA_SSLV23_PADDING	2
#define RSA_NO_PADDING		3
#define RSA_PKCS1_OAEP_PADDING	4

#define RSA_PKCS1_PADDING_SIZE	11

#define RSA_set_app_data(s,arg)         RSA_set_ex_data(s,0,arg)
#define RSA_get_app_data(s)             RSA_get_ex_data(s,0)

RSA *	RSA_new(void);
int	RSA_size(const RSA *);
RSA *	RSA_generate_key(int bits, unsigned long e,void
		(*callback)(int,int,void *),void *cb_arg);
int	RSA_check_key(const RSA *);
	/* next 4 return -1 on error */
int	RSA_public_encrypt(int flen, const unsigned char *from,
		unsigned char *to, RSA *rsa,int padding);
int	RSA_private_encrypt(int flen, const unsigned char *from,
		unsigned char *to, RSA *rsa,int padding);
int	RSA_public_decrypt(int flen, const unsigned char *from, 
		unsigned char *to, RSA *rsa,int padding);
int	RSA_private_decrypt(int flen, const unsigned char *from, 
		unsigned char *to, RSA *rsa,int padding);
void	RSA_free (RSA *r);
/* "up" the RSA object's reference count */
int	RSA_up_ref(RSA *r);

int	RSA_flags(const RSA *r);

int RSA_get_exponent(const RSA *r, unsigned char* exp, int sz);
int RSA_get_modulus(const RSA *r, unsigned char* mod, int sz);

/* The following 2 functions sign and verify a X509_SIG ASN1 object
 * inside PKCS#1 padded RSA encryption */
int RSA_sign(int type, const unsigned char *m, unsigned int m_length,
	unsigned char *sigret, unsigned int *siglen, RSA *rsa);
int RSA_verify(int type, const unsigned char *m, unsigned int m_length,
	unsigned char *sigbuf, unsigned int siglen, RSA *rsa);

/* The following 2 function sign and verify a ASN1_OCTET_STRING
 * object inside PKCS#1 padded RSA encryption */
int RSA_sign_ASN1_OCTET_STRING(int type,
	const unsigned char *m, unsigned int m_length,
	unsigned char *sigret, unsigned int *siglen, RSA *rsa);
int RSA_verify_ASN1_OCTET_STRING(int type,
	const unsigned char *m, unsigned int m_length,
	unsigned char *sigbuf, unsigned int siglen, RSA *rsa);

int RSA_padding_add_PKCS1_type_1(unsigned char *to,int tlen,
	const unsigned char *f,int fl);
int RSA_padding_check_PKCS1_type_1(unsigned char *to,int tlen,
	const unsigned char *f,int fl,int rsa_len);
int RSA_padding_add_PKCS1_type_2(unsigned char *to,int tlen,
	const unsigned char *f,int fl);
int RSA_padding_check_PKCS1_type_2(unsigned char *to,int tlen,
	const unsigned char *f,int fl,int rsa_len);
int RSA_padding_add_PKCS1_OAEP(unsigned char *to,int tlen,
	const unsigned char *f,int fl,
	const unsigned char *p,int pl);
int RSA_padding_check_PKCS1_OAEP(unsigned char *to,int tlen,
	const unsigned char *f,int fl,int rsa_len,
	const unsigned char *p,int pl);
int RSA_padding_add_SSLv23(unsigned char *to,int tlen,
	const unsigned char *f,int fl);
int RSA_padding_check_SSLv23(unsigned char *to,int tlen,
	const unsigned char *f,int fl,int rsa_len);
int RSA_padding_add_none(unsigned char *to,int tlen,
	const unsigned char *f,int fl);
int RSA_padding_check_none(unsigned char *to,int tlen,
	const unsigned char *f,int fl,int rsa_len);

RSA *RSAPublicKey_dup(RSA *rsa);
RSA *RSAPrivateKey_dup(RSA *rsa);

/* BEGIN ERROR CODES */
/* The following lines are auto generated by the script mkerr.pl. Any changes
 * made after this point may be overwritten when the script is next run.
 */
void ERR_load_RSA_strings(void);

/* Error codes for the RSA functions. */

/* Function codes. */
#define RSA_F_MEMORY_LOCK						100
#define RSA_F_RSA_CHECK_KEY						123
#define RSA_F_RSA_EAY_PRIVATE_DECRYPT			101
#define RSA_F_RSA_EAY_PRIVATE_ENCRYPT			102
#define RSA_F_RSA_EAY_PUBLIC_DECRYPT			103
#define RSA_F_RSA_EAY_PUBLIC_ENCRYPT			104
#define RSA_F_RSA_GENERATE_KEY					105
#define RSA_F_RSA_NEW_METHOD					106
#define RSA_F_RSA_NULL							124
#define RSA_F_RSA_PADDING_ADD_NONE				107
#define RSA_F_RSA_PADDING_ADD_PKCS1_OAEP		121
#define RSA_F_RSA_PADDING_ADD_PKCS1_TYPE_1		108
#define RSA_F_RSA_PADDING_ADD_PKCS1_TYPE_2		109
#define RSA_F_RSA_PADDING_ADD_SSLV23			110
#define RSA_F_RSA_PADDING_CHECK_NONE			111
#define RSA_F_RSA_PADDING_CHECK_PKCS1_OAEP		122
#define RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_1	112
#define RSA_F_RSA_PADDING_CHECK_PKCS1_TYPE_2	113
#define RSA_F_RSA_PADDING_CHECK_SSLV23			114
#define RSA_F_RSA_PRINT							115
#define RSA_F_RSA_PRINT_FP						116
#define RSA_F_RSA_SIGN							117
#define RSA_F_RSA_SIGN_ASN1_OCTET_STRING		118
#define RSA_F_RSA_VERIFY						119
#define RSA_F_RSA_VERIFY_ASN1_OCTET_STRING		120

/* Reason codes. */
#define RSA_R_ALGORITHM_MISMATCH				100
#define RSA_R_BAD_E_VALUE						101
#define RSA_R_BAD_FIXED_HEADER_DECRYPT			102
#define RSA_R_BAD_PAD_BYTE_COUNT				103
#define RSA_R_BAD_SIGNATURE						104
#define RSA_R_BLOCK_TYPE_IS_NOT_01				106
#define RSA_R_BLOCK_TYPE_IS_NOT_02				107
#define RSA_R_DATA_GREATER_THAN_MOD_LEN			108
#define RSA_R_DATA_TOO_LARGE					109
#define RSA_R_DATA_TOO_LARGE_FOR_KEY_SIZE		110
#define RSA_R_DATA_TOO_LARGE_FOR_MODULUS		132
#define RSA_R_DATA_TOO_SMALL					111
#define RSA_R_DATA_TOO_SMALL_FOR_KEY_SIZE		122
#define RSA_R_DIGEST_TOO_BIG_FOR_RSA_KEY		112
#define RSA_R_DMP1_NOT_CONGRUENT_TO_D			124
#define RSA_R_DMQ1_NOT_CONGRUENT_TO_D			125
#define RSA_R_D_E_NOT_CONGRUENT_TO_1			123
#define RSA_R_INVALID_MESSAGE_LENGTH			131
#define RSA_R_IQMP_NOT_INVERSE_OF_Q				126
#define RSA_R_KEY_SIZE_TOO_SMALL				120
#define RSA_R_NULL_BEFORE_BLOCK_MISSING			113
#define RSA_R_N_DOES_NOT_EQUAL_P_Q				127
#define RSA_R_OAEP_DECODING_ERROR				121
#define RSA_R_PADDING_CHECK_FAILED				114
#define RSA_R_P_NOT_PRIME						128
#define RSA_R_Q_NOT_PRIME						129
#define RSA_R_RSA_OPERATIONS_NOT_SUPPORTED		130
#define RSA_R_SSLV3_ROLLBACK_ATTACK				115
#define RSA_R_THE_ASN1_OBJECT_IDENTIFIER_IS_NOT_KNOWN_FOR_THIS_MD 116
#define RSA_R_UNKNOWN_ALGORITHM_TYPE			117
#define RSA_R_UNKNOWN_PADDING_TYPE				118
#define RSA_R_WRONG_SIGNATURE_LENGTH			119

#ifdef  __cplusplus
}
#endif


#endif // CRYPT_H
 
