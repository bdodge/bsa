//--------------------------------------------------------------------
//
// File: bsatelcom.h
// Desc: RS232 Serial Comm
// Auth: Brian Dodge
//
// (C)opyright 2004  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------

#ifndef _BSATELCOM_H_
#define _BSATELCOM_H_ 1

#ifdef TELNET_EXPORTS
	#define BSATELCOM_API __declspec(dllexport)
#elif defined(BSATELCOM_DLL)
	#define BSATELCOM_API __declspec(dllimport)
#else
	#define BSATELCOM_API
#endif

#define NUMLMODEOPTIONS 30

/* Definitions for telnet protocol */

#define STNORM      0

/* Definition of the lowest telnet byte following an IAC byte */
#define LOW_TEL_OPT 236

#define TEL_EOF     236
#define SUSP        237
#define ABORT       238

#define SE			240
#define NOP			241
#define DM			242
#define BREAK		243
#define IP			244
#define AO			245
#define AYT			246
#define EC			247
#define EL			248
#define GOAHEAD 	249
#define SB			250
#define WILLTEL 	251
#define WONTTEL 	252
#define DOTEL	 	253
#define DONTTEL 	254
#define IAC		 	255

/* Assigned Telnet Options */
#define BINARY	 			0
#define ECHOTEL				1
#define RECONNECT			2
#define SGA 				3
#define AMSN				4
#define STATUS				5
#define TIMING				6
#define RCTAN				7
#define OLW					8
#define OPS					9
#define OCRD				10
#define OHTS				11
#define OHTD				12
#define OFFD				13
#define OVTS				14
#define OVTD				15
#define OLFD				16
#define XASCII				17
#define LOGOUT				18
#define BYTEM				19
#define DET					20
#define SUPDUP				21
#define SUPDUPOUT			22
#define SENDLOC				23
#define TERMTYPE 			24
	#define SENDTTYPE            1
	#define REPLYTTYPE           0
#define EOR					25
#define TACACSUID			26
#define OUTPUTMARK			27
#define TERMLOCNUM			28
#define REGIME3270			29
#define X3PAD				30
#define NAWS				31
#define TERMSPEED			32
#define TFLOWCNTRL			33
#define LINEMODE 			34
	#define MODE 1
        #define MODE_EDIT       1
        #define MODE_TRAPSIG    2
        #define MODE_ACK        4
        #define MODE_SOFT_TAB   8
        #define MODE_LIT_ECHO   16

	#define FORWARDMASK 2

	#define SLC 3 
        #define SLC_DEFAULT     3
        #define SLC_VALUE       2
        #define SLC_CANTCHANGE  1
        #define SLC_NOSUPPORT   0
        #define SLC_LEVELBITS   3

        #define SLC_ACK         128
        #define SLC_FLUSHIN     64
        #define SLC_FLUSHOUT    32

		#define SLC_SYNCH		1
		#define SLC_BRK			2
		#define SLC_IP			3
		#define SLC_AO			4
		#define SLC_AYT			5
		#define SLC_EOR			6
		#define SLC_ABORT		7
		#define SLC_EOF			8
		#define SLC_SUSP		9
		#define SLC_EC			10
		#define SLC_EL   		11
		#define SLC_EW   		12
		#define SLC_RP			13
		#define SLC_LNEXT		14
		#define SLC_XON			15
		#define SLC_XOFF		16
		#define SLC_FORW1		17
		#define SLC_FORW2		18
        #define SLC_MCL         19
        #define SLC_MCR         20
        #define SLC_MCWL        21
        #define SLC_MCWR        22
        #define SLC_MCBOL       23
        #define SLC_MCEOL       24
        #define SLC_INSRT       25
        #define SLC_OVER        26
        #define SLC_ECR         27
        #define SLC_EWR         28
        #define SLC_EBOL        29
        #define SLC_EEOL        30

#define XDISPLOC			35
#define ENVIRONMENT         36
#define AUTHENTICATION      37
#define DATA_ENCRYPTION     38

#define MAX_TELOPTCODE		38

#define XOPTIONS			255

#define LINEMODE_MODES_SUPPORTED    0x1B
#define SLC_SUPPORTED				0x10    
	
	
typedef enum
{
	tnPS_NORM,
	tnPS_IAC,
	tnPS_ESCIAC,
	tnPS_OPTION,
	tnPS_SUBNEG,
	tnPS_ERASEC,
	tnPS_SNTERMTYPE
}
TelParseState;

typedef enum
{
	tnWill, tnWont, tnDo, tnDont, tnUnsure
}
TelOptState;

//***********************************************************************
class BSATELCOM_API BtelnetStream : public Bstream
{
public:
	BtelnetStream();
	virtual ~BtelnetStream();

public:
	virtual ERRCODE			Open				(char* host, short port = 23);
	virtual ERRCODE			Close				(void);
	virtual ERRCODE			Read				(BYTE* pBuf, int& cnt);
	virtual ERRCODE			Write				(BYTE* pBuf, int& cnt);
	virtual ERRCODE			Pend				(int to_s, int to_us);
	virtual ERRCODE			Connect				(int to_s, int to_us);

public:
	virtual ERRCODE			TelnetSetWindowSize	(int w, int h);

protected:
	virtual ERRCODE			TelnetReply			(BYTE otype, BYTE ecode);
	virtual TelParseState	TelnetEscape		(BYTE ecode, BYTE* otype);
	virtual TelParseState	TelnetOption		(BYTE otype, BYTE ecode);
	virtual TelParseState	TelnetSubnegTermType(BYTE ecode);
	virtual TelParseState	TelnetSubnegotiate	(BYTE ecode);

public:

protected:
	// tcp stream
	BtcpStream*			m_ps;

	// parsing state
	TelParseState		m_parseState;

	// state of negotiated options
	TelOptState			m_opts[MAX_TELOPTCODE];

	Blog*				m_log;
};


#endif

