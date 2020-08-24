
#ifndef _BSASHELL_H_
#define _BSASHELL_H_ 1

#ifdef SHELL_EXPORTS
	#define SHELL_API __declspec(dllexport)
#elif defined(BSASHELL_DLL)
	#define SHELL_API __declspec(dllimport)
#else
	#define SHELL_API
#endif

#ifdef Windows
	typedef HANDLE HSIO;
	typedef HANDLE PIDT;
#endif
#if defined(Unix)||defined(Linux)||defined(Solaris)||defined(Darwin)||defined(OSX)

	#include <unistd.h>
	#include <termios.h>

	typedef int HSIO;
	typedef int PIDT;
#endif
#include <fcntl.h>

enum BvtEmulation{ emulNONE, emulVT100, emulVT102, emulVT220, emulXTERM, emulVT320, emulVT420 };

#define MAX_INSERT_COUNT 	1024
#define MAX_PER_UPDATE 		10

#define PTY_BASE_NAME		"/dev/pty"
#define TTY_BASE_NAME		"/dev/tty"

//**************************************************************************
class SHELL_API Bshell
{
public:
	Bshell(LPCTSTR shellName = NULL, int rows = 24, int cols = 80, BvtEmulation emul = emulNONE);
	~Bshell();

public:
	virtual ERRCODE	Init				(LPCTSTR cmdLine = NULL);
	virtual void	Deinit				(void);
	virtual void 	SetRowsCols			(int rows, int cols);
	virtual ERRCODE	GetExitCode			(DWORD& code) { code = m_exitcode; return errOK; }

	static LPCTSTR	GetDefaultShellName	(void);

protected:
	virtual ERRCODE	Poll				(HSIO hio, int tos, int tous);
	virtual int		Read				(HSIO hio, char* buffer, int nRead);
	virtual int		Write				(HSIO hio, char* buffer, int nWrite);
	virtual bool	Exited				(void);

	static void		SetupTTY			(HSIO masterFile, int rows, int cols, BvtEmulation emul);
#ifndef Windows
	static int		OpenPseudoTerm		(LPSTR masterFilename);
#endif

public:

protected:
	HSIO			m_hsioRead;
	HSIO			m_hsioErrRead;
	HSIO			m_hsioWrite;

	PIDT			m_pid;

#ifdef Windows
	HANDLE			m_fd1[2];
	HANDLE			m_fd2[2];
	HANDLE			m_fd3[2];
	DWORD			m_exitcode;
#else
	int				m_masterFile;
	int				m_slaveFile;
	int				m_exitcode;
#endif

	char			m_shellName[MAX_PATH + 32];
	int				m_rows;
	int				m_cols;
	BvtEmulation	m_emul;
};


#endif
