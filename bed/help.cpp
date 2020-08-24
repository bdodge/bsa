
#include "bedx.h"

//**************************************************************************
ERRCODE Bview::HelpCommands()
{
	Bbuffer*  pBuf;
	BcmdName* pCmd;
	int		  line, col;

	pBuf = new BtextBuffer(_T("Command Help"), btText, txtANSI, txtANSI, m_editor);
	if(! pBuf) return errNO_MEMORY;

	pBuf->SetHasUndo(false);
	line = col = 1;
	pBuf->Insert(line, col, _T("\n"), 1);
	line = col = 1;

	for(pCmd = commandNames ; pCmd->cmd != MaxEditCommand; pCmd++)
	{
		if(pCmd->cmd != UnknownCommand)
		{
			pBuf->Insert(line, col, pCmd->name, _tcslen(pCmd->name));
			pBuf->Insert(line, col, _T("\n"), 1);
		}
	}
	
	pBuf->SetModified(false);
	pBuf->SetReadOnly(true);
	pBuf->SetRead(true);
	pBuf->SetUntitled(true);
	m_editor->EditBuffer(pBuf);
	return errOK;
}

//**************************************************************************
LPCTSTR CommandNameOf(EditCommand cmd)
{
	BcmdName* pCmd;

	for(pCmd = commandNames ; pCmd->cmd != cmd && pCmd->cmd != MaxEditCommand; pCmd++)
		;
	return pCmd->name;
}

//**************************************************************************
LPCTSTR KeyNameOf(int k)
{
	static TCHAR kb[8];

	if(k == 0)
	{
		return _T("    ");
	}
	if(k >= 1 && k <= 26)
	{
		_sntprintf(kb, 8, _T("^%c   "), k + 'A' - 1);
		return kb;
	}
	if(k > 0x20 && k <= 0x7E)
	{
		_sntprintf(kb, 8, _T("%c    "), k);
		return kb;
	}		
	if(k >= keyF1 && k <= keyF24)
	{
		_sntprintf(kb, 8, _T("F%-2d  "), k-keyF1+1);
		return kb;
	}
	if(k >= 0x1C && k <= 0x1F)
	{
		switch(k)
		{
		case 28: return _T("FS   ");
		case 29: return _T("GS   ");
		case 30: return _T("RS   ");
		case 31: return _T("US   ");
		}
	}
	if(k == 0x20)
	{
		return _T("space");
	}
	switch(k)
	{
	case keyEscape:			return _T("Esc  ");

	case keyDelete:			return _T("Del  ");
	case keyCursorLeft:		return _T("Left ");
	case keyCursorRight:	return _T("Rght ");
	case keyCursorUp:		return _T("Up   ");
	case keyCursorDown:		return _T("Down ");
	case keyCursorHome:		return _T("Home ");
	case keyCursorPrev:		return _T("Prev ");
	case keyCursorNext:		return _T("Next ");
	case keyCursorEnd:		return _T("End  ");
	case keyCursorPageUp:	return _T("PgUp ");
	case keyCursorUpLeft:	return _T("UpLf ");
	case keyCursorDownLeft:	return _T("DnLf ");
	case keyCursorPageDown:	return _T("PgDn ");

	case keySelect:		return _T("Sel  ");
	case keyPrint:		return _T("Prt  ");
	case keyExec:		return _T("Exec ");
	case keyInsert:		return _T("Ins  ");
	case keyUndo:		return _T("Undo ");
	case keyRedo:		return _T("Redo ");
	case keyMenu:		return _T("Menu ");
	case keySearch:		return _T("Find ");
	case keyStop:		return _T("Stop ");
	case keyHelp:		return _T("Help ");
	case keyBreak:		return _T("Brk  ");
	case keyModeSwitch:	return _T("Mode ");
	case keyNumLock:	return _T("Num  ");
	
	case keyShift:		return _T("Shft ");
	case keyCtrl:		return _T("Ctrl ");
	case keyAlt:		return _T("Alt  ");
	case keyCapsLock:	return _T("Caps ");
	case keyPause:		return _T("Ps   ");
	default:
		return _T("     ");
	}
}

//**************************************************************************
ERRCODE Bview::HelpKeyBinds()
{
	Bbuffer*		pBuf;
	BkeyBindItem*	pBinding;
	LPCTSTR			pName;
	int				line, col;
	int				i, j;

	pBuf = new BtextBuffer(_T("Key Bindings Help"), btText, txtANSI, txtANSI, m_editor);
	if(! pBuf) return errNO_MEMORY;

	pBuf->SetHasUndo(false);
	line = col = 1;
	pBuf->Insert(line, col, _T("\n"), 1);
	line = col = 1;

	pBinding = m_keyBindings->GetKeySequences();
	for(i = 0; pBinding[i].code != MaxEditCommand; i++)
	{
		pName = KeyNameOf(0);
		pBuf->Insert(line, col, pName, _tcslen(pName));
		for(j = 0; j < MAX_KEYSEQUENCE;)
		{
			pName = KeyNameOf(pBinding[i].seq[j]);
			pBuf->Insert(line, col, pName, _tcslen(pName));
			j++;
			if(! pBinding[i].seq[j]) 
			{
				pBuf->Insert(line, col, _T(" "), 1);
				break;
			}
			else
				pBuf->Insert(line, col, _T("+"), 1);
		}
		for(pName = KeyNameOf(0); j < MAX_KEYSEQUENCE; j++)
		{
			pBuf->Insert(line, col, pName, _tcslen(pName));
			pBuf->Insert(line, col, _T(" "), 1);
		}
		if(pBinding[i].code == UnknownCommand)
			pName = _T("Not Bound");
		else
			pName = CommandNameOf(pBinding[i].code);
		pBuf->Insert(line, col, pName, _tcslen(pName));
		pBuf->Insert(line, col, _T("\n"), 1);
	}
	
	pBuf->SetModified(false);
	pBuf->SetReadOnly(true);
	pBuf->SetRead(true);
	pBuf->SetUntitled(true);
	m_editor->EditBuffer(pBuf);
	return errOK;
}
