
#include "bedx.h"

//**************************************************************************
BviewShell::BviewShell(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel)
		:
		BviewTerminal(pBuf, pEditor, pPanel),
		Bshell(NULL, 24, 80, m_emulation)
{
	m_buffer->SetName(_T("shell"));
}

//**************************************************************************
BviewShell::~BviewShell()
{
	if(m_running)
		Stop();
}

// STATIC
//**************************************************************************
ERRCODE BviewShell::RunStub(LPVOID thisptr)
{
	return ((BviewShell*)thisptr)->ShellThread();
}

//**************************************************************************
void BviewShell::Activate()
{
	bool issetup = false;

	// first time called, make sure user has done first time
	// config stuff like pick emulation
	//
	if(GetEditor()->GetPersist())
		GetEditor()->GetPersist()->GetNvBool(_T("Shell/SetupValid"), issetup, false);
	if(! issetup)
	{
		if(EmulationDialog(GetEditor(), this, GetTypeName(), GetWindow()) != IDOK)
		{
			Close();
			return;
		}
	}
	if(GetEditor()->GetPersist())
		GetEditor()->GetPersist()->SetNvBool(_T("Shell/SetupValid"), true);
	
	if(! m_pid)
	{		
		// init shell
		//
		Init();
	}
	BviewTerminal::Activate();
}

//**************************************************************************
void BviewShell::FitToPanel(void)
{
	BviewTerminal::FitToPanel();
	SetRowsCols(m_vtrows, m_vtcols);
}

//**************************************************************************
void BviewShell::Event(LPARAM lParam)
{
	BviewTerminal::Event(lParam);

	if(Exited())
	{
		Stop();
		m_pid = 0;
	}
	// check for end of shell exec
	//
	if(m_pid == 0 && m_idTimer != 0)
	{
		KillTimer(NULL, m_idTimer);
		
		if(m_closeonexit)
		{
			if(m_noexitprompt)
				m_buffer->SetModified(false);

			Dispatch(KillWindow);
		}
	}
}

//**************************************************************************
ERRCODE BviewShell::SendData(LPBYTE data, int len)
{
	// pass key off to shell, it may echo, etc.
	//
	if(! Exited())
	{
		Block lock(&m_bufex);
		Write(m_hsioWrite, (char*)data, len);
		return errOK;
	}
	return errSTREAM_WRITE;
}

//**************************************************************************
ERRCODE BviewShell::ShellThread()
{
	ERRCODE ec = errOK;
	int		nRead;
	int		nRoom;
	int		endwait;

	m_head = m_tail = m_cnt = 0;
	m_size = sizeof(m_iobuf) - 1;

	while(! Exited() && m_running)
	{
		if (m_cnt >= m_size)
		{
			// full buffer
			//
			Bthread::Sleep(15);
		}
		if(Poll(m_hsioRead, 0, 100000) == errSTREAM_DATA_PENDING)
		{
			Block lock(&m_bufex);

			if (m_cnt < m_size)
			{
				if(m_head >= m_tail)
					nRoom = m_size - m_head;
				else
					nRoom = m_tail - m_head;
			}
			else
			{
				nRoom = 0;
			}
			if(nRoom > 0)
			{				
				if((nRead = Read(m_hsioRead, (char*)m_iobuf + m_head, nRoom)) > 0)
				{
					m_head += nRead;
					if (m_head >= m_size)
						m_head = 0;
					m_cnt += nRead;
					m_iobuf[m_head] = 0;
				}
				else
				{
					// reading 0 on select > 1 means stream is closed
					//
					Stop();
				}
				//printf("stdout got =%s= %d room=%d\n", m_iobuf + m_head - nRead, nRead, nRoom);
			}
		}
		if(m_hsioErrRead != m_hsioRead && Poll(m_hsioErrRead, 0, 10000) == errSTREAM_DATA_PENDING)
		{
			if(m_cnt < m_size)
			{				
				if(m_head >= m_tail)
					nRoom = m_size - m_head;
				else
					nRoom = m_tail - m_head;
			}
			else
			{
				nRoom = 0;
			}
			if((nRead = Read(m_hsioErrRead, (char*)m_iobuf + m_head, nRoom)) > 0)
			{					
				m_head += nRead;
				if (m_head >= m_size)
					m_head = 0;
				m_cnt += nRead;
				m_iobuf[m_head] = 0;
			}
			else
			{
				// reading 0 on select > 1 means stream is closed
				//
				Stop();
			}
			//printf("stderr got =%s= %d room=%d\n", m_iobuf + m_head - nRead, nRead, nRoom);
		}
	}
	// wait for reader to catch up (but not forever)
	//
	endwait = 0;
	do
	{
		if(m_cnt > 0)
		{
			endwait++;
			Bthread::Sleep(100);
		}
	}
	while(endwait < 6 && m_cnt > 0);
	m_pid = 0;
	return ec;
}

