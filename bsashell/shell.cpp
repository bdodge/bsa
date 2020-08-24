
#include "shellx.h"

#ifndef Windows
#include <sys/termios.h>
#include <sys/ioctl.h>
#if defined(Darwin)||defined(OSX)
	#include <util.h>
#else
	#include <pty.h>
#endif
#else
static HWND  s_hin;
#endif

// STATIC
//**************************************************************************
void Bshell::SetupTTY(HSIO masterFile, int rows, int cols, BvtEmulation emul)
{
#ifndef Windows
	struct winsize tty_winsize;
	struct termios tty_termio;

	// window size
	//
	tty_winsize.ws_row = rows;		/* rows, in characters */
	tty_winsize.ws_col = cols;		/* columns, in characters */

	tty_winsize.ws_xpixel = 8;		/* font size ? */
	tty_winsize.ws_ypixel = 13;

	/*
	 * set env for emulation
	 */
	switch(emul)
	{
	case emulNONE:
		putenv((char*)"TERM=ansi");
		break;
	case emulVT100:
	default:
		putenv((char*)"TERM=vt100");
		break;
	case emulVT102:
		putenv((char*)"TERM=vt102");
		break;
	case emulVT220:
		putenv((char*)"TERM=vt220");
		break;
	case emulXTERM:
		putenv((char*)"TERM=xterm");
		break;
	case emulVT320:
		putenv((char*)"TERM=vt320");
		break;
	case emulVT420:
		putenv((char*)"TERM=vt420");
		break;
	}
	if(masterFile >= 0)
	{
		if(ioctl(masterFile, TIOCSWINSZ, (char*)&tty_winsize) < 0)
			return;
	}
#endif // UNIX
}


// STATIC
//**************************************************************************
LPCTSTR Bshell::GetDefaultShellName(void)
{
	const char* rp = NULL;

#ifdef Windows
	rp = getenv("ComSpec");
#else
	rp = getenv("SHELL");
#endif
	if(! rp)
	{
#ifdef Windows
		if(GetVersion() & 0x80000000L)
			rp = "command.com";
		else
			rp = "cmd.exe";
#else
		rp = "bash";
#endif
	}
#ifdef UNICODE
	{
		static TCHAR defPath[MAX_PATH + 32];

		CharToTChar(defPath, rp);
		return defPath;
	}
#else
	return rp;
#endif
}


//**************************************************************************
bool Bshell::Exited(void)
{
#ifndef Windows
	pid_t stat;
	int   status;

	/* we already know it is dead, pid-less shells
	 * should use a pid == -1
	 */
	if(m_pid == 0) return true;
	if(m_pid < 0)  return false;

	status = 0;

#if 1
	{
	stat = wait4(m_pid, &status, WNOHANG, NULL);
	//printf("pid=%d stat=%d, errno=%d\n", m_pid, stat, errno);
	if(stat == m_pid)
	{
		if(WIFEXITED(status) && (stat == m_pid))
			return true;
	}
	else if(stat < 0)
	{
		return true;
	}
	}
#else
	stat = waitpid(m_pid, &status, WNOHANG);
	//printf("pid=%d stat=%d, status=%d errno=%d\n", m_pid, stat, status, errno);
	if(stat == m_pid)
	{
		if(WIFEXITED(status) && (stat == m_pid))
			return true;
	}
	else if(stat < 0)
	{
		return true;
	}
#endif
#endif
#ifdef Windows
	if(m_pid == INVALID_HANDLE_VALUE)
		return false;
	if(! GetExitCodeProcess(m_pid, &m_exitcode))
		return true;
	if(m_exitcode != STILL_ACTIVE)
		return true;
#endif
	return false;
}

#ifdef Windows

//**************************************************************************
ERRCODE Bshell::Poll(HANDLE fd, int tos, int tous)
{
	BOOL    sv;
	DWORD   rv;
	int     to = tos * 1000 + tous / 1000;

	do
	{
		sv = PeekNamedPipe(fd, NULL, 0, NULL, &rv, NULL);

		if(sv)
		{
			if(rv > 0)
				return errSTREAM_DATA_PENDING;
		}
		else
		{
			return errFAILURE;
		}
		if(to > 0)
		{
			to-= 10;
			Sleep(10);
		}
	}
	while(to > 0);

	return errSTREAM_TIMEOUT;
}

//**************************************************************************
int Bshell::Read(HANDLE fd, char* buffer, int nRead)
{
	DWORD n = 0;

	ReadFile(fd, buffer, nRead, &n, NULL);
	return n;
}

//**************************************************************************
int Bshell::Write(HANDLE fd, char* buffer, int nWrite)
{
	DWORD n = 0;
	DWORD x, ret;

	for (n = 0; n < nWrite; n++)
	{
		if (buffer[n] == 13)
		{
			ret = SendMessage(s_hin, WM_CHAR, buffer[n], buffer[n]);

			//WriteFile(fd, buffer + n, 1, &x, NULL);
			buffer[n] = 10;
		}
		ret = SendMessage(s_hin, WM_CHAR, buffer[n], buffer[n]);
		//ret = SendMessage(s_hin, WM_KEYUP, buffer[n], buffer[n]);
	}
	//WriteFile(fd, buffer, nWrite, &n, NULL);
	return n;
}

void ErrorExit(const char* msg)
{
	fprintf(stderr, msg);
}

#else

//**************************************************************************
ERRCODE Bshell::Poll(int fd, int tos, int tous)
{
	fd_set rfds;
	struct timeval timeout;
	int sv;

	FD_ZERO (&rfds);
	FD_SET (fd, &rfds);
	timeout.tv_sec = tos;
	timeout.tv_usec = tous;
	sv=select(fd + 1, &rfds, NULL, NULL, &timeout);
	if (sv < 0)
		if(errno == EINTR)
			sv = 0;
	if(sv > 0)
		return errSTREAM_DATA_PENDING;
	else if(sv == 0)
		return errSTREAM_TIMEOUT;
	else if(sv < 0)
		return errFAILURE;
}

//**************************************************************************
int Bshell::Read(HSIO fd, char* buffer, int nRead)
{
	return read(fd, buffer, nRead);
}

//**************************************************************************
int Bshell::Write(HSIO fd, char* buffer, int nWrite)
{
	return write(fd, buffer, nWrite);
}


#endif


//**************************************************************************
Bshell::Bshell(LPCTSTR shellName, int rows, int cols, BvtEmulation emul)
	:
	m_pid(0)
{
	if(! shellName)
		shellName = GetDefaultShellName();

	TCHAR szShell[MAX_PATH];

	_tcsncpy(szShell, shellName, MAX_PATH-1);
	szShell[MAX_PATH - 1] = _T('\0');
	TCharToChar(m_shellName, szShell);

	m_rows = rows;
	m_cols = cols;
	m_emul = emul;
}

//**************************************************************************
void Bshell::SetRowsCols(int rows, int cols)
{
	if(rows > 0 && cols > 0 && 1/*(rows != m_rows || cols != m_cols)*/)
	{
		m_rows = rows;
		m_cols = cols;
		SetupTTY(m_hsioWrite, m_rows, m_cols, m_emul);
	}
}

//**************************************************************************
ERRCODE Bshell::Init(LPCTSTR lpCmdLine)
{
	do
	{
#ifndef Windows
		char 	masterFilename[MAX_PATH];
		int		pid;

		 /* Open a MASTER pseudo-terminal
		 */
		masterFilename[0] = '\0';

		openpty(&m_masterFile, &m_slaveFile, NULL, NULL, NULL);

		if(m_masterFile < 0 || m_slaveFile < 0)
		{
			fprintf(stderr, "No controlling terminals");
			break;
		}
		m_hsioRead		=
		m_hsioErrRead	=
		m_hsioWrite		= m_masterFile;

		if(m_rows > 0 && m_cols > 0)
			SetupTTY(m_hsioWrite, m_rows, m_cols, m_emul);

		/*  Fork a new process, the new process being the shell program
		 */
		pid = fork();

		if(pid == 0)
		{
			char		name[MAX_PATH * 2];
			char* 		pa;
			char*		args[16];
			int i;

			/* this is the new process, exec the shell over the process
			 */
			if(lpCmdLine)
			{
				TCHAR szShell[MAX_PATH * 2];

				_tcsncpy(szShell, lpCmdLine, MAX_PATH*2-2);
				szShell[MAX_PATH * 2 - 2] = _T('\0');
				TCharToChar(name, szShell);
			}
			else
			{
				strcpy(name, m_shellName);
			}
			/* create an argv,argc set to exec the shell with
			*/
			for(i = 0, pa = name; *pa && i < 14; i++)
			{
				// skip the white space
				while(*pa == ' ' || *pa == '\t') pa++;
				if(! *pa) break;
				// this is an arg
				args[i] 	= pa;
				args[i+1] 	= NULL;
				if(*pa == '\"')
				{
					args[i] = ++pa;
					// skip to next quote
					while(*pa && *pa != '\"') pa++;
					if(*pa == '\"') *pa++ = '\0';
				}
				else
					// skip to next white space
					while(*pa && *pa != ' ' && *pa != '\t') pa++;
				if(*pa)	*pa++ = '\0';
			}

			/* hack for "bash" which needs a "-i" to run interactive
			*/
			if(strstr(name, "bash") && ! strstr(name, "-c"))
				args[++i] = (char*)"-i";
			args[i+1] = NULL;

			/* remove the controlling terminal
			 */
			setsid();

			close(m_masterFile);	// no need for master terminal in the child proc
			dup2(m_slaveFile, 0);	// dup the slave on stdin
			dup2(m_slaveFile, 1);	//           and on stdout
			dup2(m_slaveFile, 2);	//           and on stderr
			close(m_slaveFile);		// no need for slave anymore now, its duped

			/* execute the shell program on top of this fork
			 */
			if(execv(args[0], args))
			{
				//Error("Shell: Exec failed");
				break; // this is NOT a return from this function!
			}
			// there is no return from exec, so execution ends here!
		}
		/* this is the PARENT process
		*/
		if((m_pid = pid) <= 0)
		{
			return errFAILURE;
		}
#endif /* SUNIX */

#ifdef Windows
		HANDLE hdupfd1, hdupfd2, hdupfd3;
		HANDLE hSaveStdout, hSaveStderr, hSaveStdin;
		BOOL   fSuccess;

		STARTUPINFOA		sui = {
											sizeof(STARTUPINFOA),
											NULL,
											NULL,
											NULL,
											0, 0,
											0, 0,
											0, 0,
											0,
											STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES,
											SW_SHOW,
											0,
											NULL,
											0,
											0,
											0
									};

		SECURITY_ATTRIBUTES	secure = {
											sizeof(SECURITY_ATTRIBUTES),
											NULL,
											TRUE
										};

		PROCESS_INFORMATION	ppi;


		// Save the handle to the current STDOUT.

		hSaveStdout = GetStdHandle(STD_OUTPUT_HANDLE);

		// Create a pipe for the child process's STDOUT.

		if (! CreatePipe(&m_fd2[0], &m_fd2[1], &secure, 0))
			ErrorExit("Stdout pipe creation failed\n");

		// Set a write handle to the pipe to be STDOUT.

		if (! SetStdHandle(STD_OUTPUT_HANDLE, m_fd2[1]))
			ErrorExit("Redirecting STDOUT failed");

		// Create noninheritable read handle and close the inheritable read
		// handle.

		fSuccess = DuplicateHandle(
									GetCurrentProcess(), m_fd2[0],
									GetCurrentProcess(), &hdupfd2 , 0,
									FALSE,
									DUPLICATE_SAME_ACCESS
								   );
		if( !fSuccess )
			ErrorExit("DuplicateHandle failed");
		CloseHandle(m_fd2[0]);
		m_fd2[0] = hdupfd2;

		// Save the handle to the current STDERR.

		hSaveStderr = GetStdHandle(STD_ERROR_HANDLE);

		// Create a pipe for the child process's STDERR.

		if (! CreatePipe(&m_fd3[0], &m_fd3[1], &secure, 0))
			ErrorExit("Stdout pipe creation failed\n");

		// Set a write handle to the pipe to be STDERR.

		if (! SetStdHandle(STD_ERROR_HANDLE, m_fd3[1]))
			ErrorExit("Redirecting STDERR failed");

		// Create noninheritable read handle and close the inheritable read
		// handle.

		fSuccess = DuplicateHandle(
									GetCurrentProcess(), m_fd3[0],
									GetCurrentProcess(), &hdupfd3 , 0,
									FALSE,
									DUPLICATE_SAME_ACCESS
								  );
		if( !fSuccess )
			ErrorExit("DuplicateHandle failed");
		CloseHandle(m_fd3[0]);
		m_fd3[0] = hdupfd3;

		// Save the handle to the current STDIN.

		hSaveStdin = GetStdHandle(STD_INPUT_HANDLE);

		// Create a pipe for the child process's STDIN.

		if (! CreatePipe(&m_fd1[0], &m_fd1[1], &secure, 0))
			ErrorExit("Stdin pipe creation failed\n");

		// Set a read handle to the pipe to be STDIN.

		if (! SetStdHandle(STD_INPUT_HANDLE, m_fd1[0]))
			ErrorExit("Redirecting Stdin failed");

		// Duplicate the write handle to the pipe so it is not inherited.

		fSuccess = DuplicateHandle(
									GetCurrentProcess(), m_fd1[1],
									GetCurrentProcess(), &hdupfd1, 0,
									FALSE,                  // not inherited
									DUPLICATE_SAME_ACCESS
								   );
		if (! fSuccess)
			ErrorExit("DuplicateHandle failed");

		CloseHandle(m_fd1[1]);
		m_fd1[1] = hdupfd1;

		// Now create the child process.

		memset(&ppi, 0, sizeof(ppi));

		//memset(&sui, 0, sizeof(sui));
		//sui.cb = sizeof(sui);

//		sui.hStdError  = m_fd3[1];  /* write end of stderr pipe */
//		sui.hStdOutput = m_fd2[1];  /* write end of stdout pipe */
//		sui.hStdInput  = m_fd1[0];  /* read end of stdin pipe   */

		sui.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		sui.hStdOutput = m_fd2[1];
		sui.hStdError = m_fd3[1];

		TCHAR szShell[MAX_PATH * 2];
		char  cmdline[MAX_PATH * 2];
		char* name;

		if(lpCmdLine)
		{
			_tcsncpy(szShell, lpCmdLine, MAX_PATH*2-2);
			szShell[MAX_PATH * 2 - 2] = _T('\0');
			TCharToChar(cmdline, szShell);
			name = cmdline;
		}
		else
		{
			name = m_shellName;
		}
		if(m_rows > 0 && m_cols > 0)
			SetupTTY(m_hsioWrite, m_rows, m_cols, m_emul);

		if(! CreateProcessA(
							NULL,
							name,
							NULL,
							NULL,
							FALSE,
							CREATE_NEW_CONSOLE,
							NULL,
							NULL,
							&sui,
							&ppi
							)
		)
		{
			int i = GetLastError();
			break;
		}
		else
		{
			m_pid = ppi.hProcess;
			CloseHandle(ppi.hThread);
		}
		m_hsioRead		= m_fd2[0]; /* read end of stdout pipe */
		m_hsioErrRead	= m_fd3[0]; /* read end of stderr pipe */
		m_hsioWrite		= m_fd1[1]; /* write end of stdin pipe */

		DWORD mode;
		HANDLE hin = m_fd1[0];
		BOOL ret = GetConsoleMode(hin, &mode);
		DWORD err = GetLastError();

		Sleep(150);

		ret = AttachConsole(ppi.dwProcessId);
		if (ret)
		{
			s_hin = GetConsoleWindow();
			/*
			ret = SetStdHandle(STD_INPUT_HANDLE, m_fd1[0]);
			ret = SetStdHandle(STD_OUTPUT_HANDLE, m_fd2[1]);
			ret = SetStdHandle(STD_ERROR_HANDLE, m_fd3[1]);
			hin = GetStdHandle(STD_INPUT_HANDLE);
			ret = GetConsoleMode(hin, &mode);
			if (ret)
			{
				mode |= ENABLE_PROCESSED_INPUT;
				ret = SetConsoleMode(hin, mode);
			}
			err = GetLastError();
			*/
			FreeConsole();
		}
		else
		{
			err = GetLastError();
		}

		//char ioBuffer[1024];
		//Read(m_hsioRead, ioBuffer, 1024);

		// After process creation, restore the saved STDIN and STDOUT.
		if (! SetStdHandle(STD_INPUT_HANDLE, hSaveStdin))
			ErrorExit("Re-redirecting Stdin failed\n");

		if (! SetStdHandle(STD_OUTPUT_HANDLE, hSaveStdout))
			ErrorExit("Re-redirecting Stdout failed\n");

		if (! SetStdHandle(STD_ERROR_HANDLE, hSaveStderr))
			ErrorExit("Re-redirecting Stderr failed\n");

//		CloseHandle(m_fd1[0]);
//		CloseHandle(m_fd2[1]);
//		CloseHandle(m_fd3[1]);
#endif
	}
	while(0); // catch
	return errOK;
}

//**************************************************************************
void Bshell::Deinit()
{
#ifndef Windows
	close(m_hsioRead);
	if(m_hsioErrRead != m_hsioRead)
		close(m_hsioErrRead);
	close(m_hsioWrite);
#else
	CloseHandle(m_fd1[1]);
	CloseHandle(m_fd2[0]);
	CloseHandle(m_fd3[0]);
#endif
}

//**************************************************************************
Bshell::~Bshell()
{
}
