
#include "bedx.h"

//**************************************************************************
BviewStream::BviewStream(Bbuffer* pBuf, Bed* pEditor, BappPanel* pPanel)
		:
		BviewTerminal(pBuf, pEditor, pPanel),
		m_io(NULL),
		m_iochanging(false),
		m_zeroreaderr(true)
{
	if(m_buffer)
		m_buffer->SetUntitled(true);
}

//**************************************************************************
BviewStream::~BviewStream()
{
	if(m_running)
		Stop();
}

// STATIC
//**************************************************************************
ERRCODE BviewStream::RunStub(LPVOID thisptr)
{
	return ((BviewStream*)thisptr)->ShellThread();
}

//**************************************************************************
void BviewStream::Activate()
{
	BviewTerminal::Activate();
}

//**************************************************************************
void BviewStream::FitToPanel(void)
{
	BviewTerminal::FitToPanel();
}

//**************************************************************************
void BviewStream::Event(LPARAM lParam)
{
	BviewTerminal::Event(lParam);

	if(! m_running)
	{
		// check for end of shell exec
		//
		if(m_closeonexit)
		{
			if(m_noexitprompt)
				m_buffer->SetModified(false);
			Dispatch(KillWindow);
		}
	}
}

//**************************************************************************
ERRCODE BviewStream::SendData(LPBYTE data, int len)
{
	int chunk;
	int attempts;
	ERRCODE ec;

	if(! m_io) return errSTREAM_WRITE;
	// send in blocks of 16 to not overflow serial fifo
	while (len > 0)
	{
		chunk = 1;
		if (chunk > len)
		{
			chunk = len;
		}
		// wait for no data pending before inserting
		//
		attempts = 0;
		do
		{
			ec = m_io->Pend(0, 25000);
		}
		while (attempts++ < 1000 && (ec == errSTREAM_DATA_PENDING));

		ec = m_io->Write(data, chunk);
		if (ec)
		{
			return ec;
		}
		len -= chunk;
		data += chunk;
	}
	return errOK;
}

//**************************************************************************
ERRCODE BviewStream::GetPortSettings()
{
	return errOK;
}

//**************************************************************************
ERRCODE BviewStream::ApplyPortSettings()
{
	return errOK;
}

//**************************************************************************
ERRCODE BviewStream::ReopenPort()
{
	GetPortSettings();
	return ApplyPortSettings();
}


//**************************************************************************
ERRCODE BviewStream::ShellThread()
{
	ERRCODE ec = errOK;
	int		nRead;
	int		nRoom;
	int		endwait;
	int		qwait = 0;
	int		timeouts;

	m_head = m_tail = 0;
	m_size = sizeof(m_iobuf) - 1;
	m_cnt = 0;

	if(! m_io) return errSTREAM_OPEN;

	timeouts = 0;
	while(m_running && m_io)
	{
		if (m_iochanging)
		{
			Bthread::Sleep(30);
			continue;
		}
		ec = m_io->Pend(0, 250000);
		if (ec == errSTREAM_DATA_PENDING)
		{
			if(! m_running)
			{
				break;
			}
			timeouts = 0;

			if(m_cnt < m_size)
			{
				if(m_head >= m_tail)
					nRoom = m_size - m_head;
				else
					nRoom = m_tail - m_head;

				nRead = nRoom;
				ec = m_io->Read((LPBYTE)(m_iobuf + m_head), nRead);

				if(nRead > 0)
				{
					qwait = 0;
					m_bufex.Lock();
					m_head += nRead;
					if (m_head >= m_size)
						m_head = 0;
					m_cnt += nRead;
					m_iobuf[m_head] = 0;
					m_bufex.Unlock();
				}
				else if(m_zeroreaderr)
				{
					// reading 0 on select > 1 means stream is closed
					//
					Stop();
				}
				else // strange, Pend returned Data but no data...
				{
					qwait++;
					if(qwait > 30)
						qwait = 30;
					Bthread::Sleep(qwait);
				}
			}
			else
			{
				qwait++;
				if(qwait > 30)
					qwait = 30;
				Bthread::Sleep(qwait);
			}
		}
		else if(ec == errSTREAM_TIMEOUT || ec == errOK)
		{
			if (timeouts++ > 10)
			{
				// this slows us down in case the pending timeout
				// isn't long enough (underlying io doesn't respect tos, tousec)
				//
				Bthread::Sleep(20);
			}
		}
		else
		{
			if (! m_iochanging)
			{
				// pend failed, connection is prolly closed
				break;
			}
		}
	}
	if(m_io)
	{
		delete m_io;
		m_io = NULL;
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
	return ec;
}



//**************************************************************************
int BviewStream::SettingsDialog(Bed* pEditor, HWND hWndParent)
{
	if(! pEditor) return IDCANCEL;
	return IDCANCEL;
}
