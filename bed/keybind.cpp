
#include "bedx.h"

//**************************************************************************
BOOL CALLBACK PickBindingsWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static LPPARMPASS  lpParm = NULL;

	HWND	hWndParent;
	RECT	rc;
	RECT	rcme;
	int		x, y, w, h;

	switch (message)
	{
	case WM_INITDIALOG:

		lpParm = (LPPARMPASS)lParam;
		if(! lpParm) return FALSE;

		// title
		SetWindowText(hDlg, (LPTSTR)lpParm->lpTitle);

		switch(lpParm->nString)
		{
		case kbNative:
			CheckDlgButton(hDlg, IDC_BIND_NATIVE, BST_CHECKED);
			break;
		case kbEmacs:
			CheckDlgButton(hDlg, IDC_BIND_EMACS, BST_CHECKED);
			break;
		case kbMSVC:
			CheckDlgButton(hDlg, IDC_BIND_MSVC, BST_CHECKED);
			break;
		case kbCW:
			CheckDlgButton(hDlg, IDC_BIND_CW, BST_CHECKED);
			break;
		}

		hWndParent = GetParent(hDlg);
		if(! hWndParent)
			hWndParent = GetDesktopWindow();
		GetClientRect(hWndParent, &rc);
		GetWindowRect(hDlg, &rcme);

		x = ((rc.right - rc.left) - (rcme.right - rcme.left)) / 2;
		y = ((rc.bottom - rc.top) - (rcme.bottom - rcme.top)) / 2;
		w = rcme.right - rcme.left;
		h = rcme.bottom - rcme.top;

		if(x < 0) x = 0;
		if(y < 0) y = 0;

		MoveWindow(hDlg, x, y, w, h, FALSE);
		return TRUE;

	case WM_COMMAND:

		switch(wParam)
		{
		case IDOK:

			EndDialog(hDlg, wParam);
			break;

		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;

		case IDC_BIND_NATIVE:
			lpParm->nString = kbNative;
			break;

		case IDC_BIND_EMACS:
			lpParm->nString = kbEmacs;
			break;

		case IDC_BIND_MSVC:
			lpParm->nString = kbMSVC;
			break;

		case IDC_BIND_CW:
			lpParm->nString = kbCW;
			break;

		default:
			break;
		}
	}
    return FALSE;
}

//**************************************************************************
int PickKeyBindingDialog(LPPARMPASS pParm, HWND hwndParent)
{
	int rc = DialogBoxParam(
							g_hInstance,
							MAKEINTRESOURCE(IDD_KEYBIND),
							hwndParent,
							PickBindingsWndProc,
							(LPARAM)pParm
						);
	return rc;
}

enum { keyStateNormal = 0, keyStateSequence0, keyStateSequence1,
		keyStateSequence2, keyStateSequence3
};

//**************************************************************************
BkeyBinds::BkeyBinds(aKeyBind bind)
		:
		m_keyState(0)
{
	SetBindings(bind);
}

//**************************************************************************
BkeyBinds::~BkeyBinds()
{
}

//**************************************************************************
ERRCODE BkeyBinds::SetBindings(aKeyBind bind)
{
	BkeyBindItem* pBinds;

	switch(bind)
	{
	case kbNative:
		pBinds = nativeBindings;
		break;
	case kbEmacs:
		pBinds = emacsBindings;
		break;
	case kbMSVC:
		pBinds = msvcBindings;
		break;
	case kbCW:
		pBinds = codewarriorBindings;
		break;
	default:
		pBinds = nativeBindings;
		bind   = kbNative;
		break;
	}
	if(! pBinds)
		return errBAD_PARAMETER;
	m_bind = bind;
	m_keyState = 0;
	m_keySequences = pBinds;
	return errOK;
}

//**************************************************************************
EditCommand BkeyBinds::CommandFromName(LPCTSTR pName)
{
	BcmdName* pCmd = commandNames;

	while(pCmd->cmd != MaxEditCommand)
	{
		if(! _tcsicmp(pName, pCmd->name))
			return pCmd->cmd;
		pCmd++;
	}
	return UnknownCommand;
}

//**************************************************************************
//
// match an accumulated sequence to a complete sequence
//
/*
 *   0: no matching possible
 *   1: matches up to len, more bytes to compare
 *   2: mathces up to len, and len is length of pSeq
 */
int BkeyBinds::KeySequenceCompare(BkeyBindItem* pSeq, LPWORD pSoFar, int nSeq)
{
	int i;
	LPWORD pTo = pSeq->seq;

	for(i = 0; i < nSeq; i++)
	{
		if(! *pTo)		break;
		if(! *pSoFar)	break;

		if(*pSoFar++ != *pTo++)
			return 0;
	}

	if(! *pTo && ! *pSoFar && i == nSeq)
		return 2;
	else
		return 1;
}

//**************************************************************************
//
// accumulated a sequence and match to complete sequence from table
//
EditCommand BkeyBinds::MatchSequence(WORD key)
{
	int				nSeq, seqLen;
	int				match;
	int				retry;
	BkeyBindItem*	pSeq;

	/*
	 * advance sequence state, since it is
	 * used to calculate length of sequence, etc.
	 */
	if(m_keyState == keyStateNormal)
		m_keyState = keyStateSequence0;
	else
		m_keyState++;

	match  = 0;
	seqLen = ((int)m_keyState);

	if(seqLen > MAX_KEYSEQUENCE)
	{
		m_keyState = keyStateNormal;
		return UnknownCommand;
	}
	/*
	 * add this code and key state to the
	 * current sequence
	 */
	m_keyseq[seqLen - 1] = key;
	m_keyseq[seqLen]	 = '\0';

	do
	{
		pSeq   = m_keySequences;

		retry = 0;

		/*
		 * Look up in the sequence table to match seq so far
		 */
		while(! match && pSeq)
		{
			if(pSeq->code == MaxEditCommand) break;
			for(nSeq = 0; pSeq->seq[nSeq] && nSeq < MAX_KEYSEQUENCE;)
				nSeq++;
			match = KeySequenceCompare(pSeq, m_keyseq, nSeq);
			if(! match) pSeq++;
		}
		/*
		 * if no match, and the leading char is a shift/alt/ctrl,
		 * see if a match is had by throwing out the first key
		 */
		if(! match && seqLen > 1)
		{
			if(m_keyseq[0] == keyShift || m_keyseq[0] == keyCtrl)
			{
				int i;

				seqLen--;
				m_keyState = seqLen;

				for(i = 0; i < seqLen; i++)
					m_keyseq[i] = m_keyseq[i + 1];
				m_keyseq[i] = 0;
				retry = 1;
			}
		}
	}
	while(! match && retry);

	//printf("k=%02X %c match=%d\n", key, key, match);
#if 0
	if(pObj->keyBinding == keyBindEmacs)
	{
		keytextbuf[0] = '\0';
		switch(match)
		{
		case 0:
		default:
			/* not even partial match in sequence list */
			edReport(" ", pObj);
			break;
		case 1:
		case 2:
			for(i = 0; i < seqLen; i++)
			{
				strcat(keytextbuf, KeyNames[pObj->curkeyseq[i] & 0xFF]);
				if(i < (seqLen - 1) || match == 1)
					strcat(keytextbuf, "-");
			}
			edReport(keytextbuf, pObj);
			break;
		}
	}
#endif
	switch(match)
	{
	case 0:
	default:
		/* not even partial match in sequence list */
		m_keyState = keyStateNormal;
		return UnknownCommand;
	case 1:
		/* partial match */
		return UnknownCommand;
	case 2:
		/* exact match */
		m_keyState = keyStateNormal;
		return pSeq->code;
	}
}

//**************************************************************************
EditCommand BkeyBinds::Translate(WORD key)
{
	EditCommand rv;

	switch(m_keyState)
	{
	case keyStateNormal:
	default:
		/*
		 * first step through sequence table with
		 * this char code to see if object has
		 * key bound privately, or if it starts
		 * a sequence
		 */
		rv = MatchSequence(key);
		break;

	case keyStateSequence0:
	case keyStateSequence1:
	case keyStateSequence2:
	case keyStateSequence3:
		rv = MatchSequence(key);
		break;
	}
	return rv;
}


//**************************************************************************
//**************************************************************************
// map of commands with names and help
//
BcmdName commandNames[] =
{
	{ UnknownCommand, 	_T("UnknownCommand") },

	/*
	 * Cursor Movement
	 */
	{ MoveToCol, 		_T("MoveToCol") },
	{ MoveToLine,		_T("MoveToLine") },

	{ MoveLeft, 		_T("MoveLeft") },
	{ MoveRight, 		_T("MoveRight") },
	{ MoveUp, 			_T("MoveUp") },
	{ MoveDown, 		_T("MoveDown") },

	{ MoveWordLeft, 	_T("MoveWordLeft") },
	{ MoveWordRight, 	_T("MoveWordRight") },

	{ MovePageLeft, 	_T("MovePageLeft") },
	{ MovePageRight, 	_T("MovePageRight") },
	{ MovePageUp, 		_T("MovePageUp") },
	{ MovePageDown, 	_T("MovePageDown") },

	{ MoveToBOL, 		_T("MoveToBOL") },
	{ MoveToEOL, 		_T("MoveToEOL") },

	{ MoveToTOB, 		_T("MoveToTOB") },
	{ MoveToEOB, 		_T("MoveToEOB") },

	/*
	 * Locating
	 */
	{ FindForward, 		_T("FindForward") },
	{ FindReverse, 		_T("FindReverse") },
	{ FindNonZero, 		_T("FindNonZero") },
	{ FindAgain, 		_T("FindAgain") },
	{ Replace, 			_T("Replace") },
	{ SetSearch, 		_T("SetSearch") },
	{ SetReplace, 		_T("SetReplace") },
	{ ReplaceFoundText, _T("ReplaceFoundText") },
	{ SearchAndReplace, _T("SearchAndReplace") },
	{ SetSearchDirection,_T("SetSearchDirection") },
	{ SetSearchMode, 	_T("SetSearchMode") },
	{ FindFile, 		_T("FindFile") },
	{ FindInFile, 		_T("FindInFile") },
	{ ReplaceInFile, 	_T("ReplaceInFile") },

	/*
	 * Item insertion/deletion
	 */
	{ SelfInsert, 		_T("SelfInsert") },
	{ Insert, 			_T("Insert") },
	{ Paste, 			_T("Paste") },
	{ Delete, 			_T("Delete") },
	{ DeleteBack, 		_T("DeleteBack") },
	{ DeleteEOL, 		_T("DeleteEOL") },
	{ DeleteEolOrLineOrBlock, _T("DeleteEolOrLineOrBlock") },
	{ DeleteBlock, 		_T("DeleteBlock") },
	{ Cut, 				_T("Cut") },
	{ Copy, 			_T("Copy") },
	{ Type, 			_T("Type") },

	/*
	 * Marking
	 */
	{ SetMark, 			_T("SetMark") },
	{ ClearMark, 		_T("ClearMark") },
	{ SwapMark, 		_T("SwapMark") },
	{ ToggleMark, 		_T("ToggleMark") },
	{ SelectChar, 		_T("SelectChar") },
	{ SelectWord, 		_T("SelectWord") },
	{ SelectLine, 		_T("SelectLine") },
	{ SelectAll, 		_T("SelectAll") },

	/*
	 * Buffer
	 */
	{ NewBuffer, 		_T("NewBuffer") },
	{ OpenBuffer, 		_T("OpenBuffer") },
	{ ReadBuffer, 		_T("ReadBuffer") },
	{ MakeWritable,		_T("MakeWritable") },
	{ NextBuffer,		_T("NextBuffer") },
	{ PreviousBuffer, 	_T("PreviousBuffer") },
	{ SaveBuffer, 		_T("SaveBuffer") },
	{ SaveAs, 			_T("SaveAs") },
	{ SaveAll, 			_T("SaveAll") },
	{ KillBuffer, 		_T("KillBuffer") },
	{ EmptyBuffer, 		_T("EmptyBuffer") },
	{ ListBuffers, 		_T("ListBuffers") },
	{ MergeBuffers, 	_T("MergeBuffers") },

	/*
	 * Window
	 */
	{ SplitVertical, 	_T("SplitVertical") },
	{ SplitHorizontal, 	_T("SplitHorizontal") },
	{ SplitWindow, 		_T("SplitWindow") },
	{ NextWindow, 		_T("NextWindow") },
	{ PreviousWindow, 	_T("PreviousWindow") },
	{ RefreshWindow, 	_T("RefreshWindow") },
	{ RecenterWindow, 	_T("RecenterWindow") },
	{ KillWindow, 		_T("KillWindow") },
	{ KillAllWindows, 	_T("KillAllWindows") },
	{ SetBinRadix, 		_T("SetBinRadix") },
	{ SetBinFormat,		_T("SetBinFormat") },
	{ SetBinColumns, 	_T("SetBinColumns") },

	/*
	 * Undo
	 */
	{ Undo, 			_T("Undo") },
	{ Redo, 			_T("Redo") },

	/*
	 * Macros
	 */
	{ DefineMacro, 		_T("DefineMacro") },
	{ PlayMacro, 		_T("PlayMacro") },
	{ KillMacro, 		_T("KillMacro") },
	{ SaveMacro, 		_T("SaveMacro") },
	{ RecallMacro,		_T("RecallMacro") },

	{ PushString, 		_T("PushString") },
	{ PushNumber, 		_T("PushNumber") },
	{ PopString, 		_T("PopString") },
	{ PopNumber, 		_T("PopNumber") },

	/*
	 * Misc
	 */
	{ OverstrikeOn, 	_T("OverstrikeOn") },
	{ OverstrikeOff, 	_T("OverstrikeOff") },
	{ ToggleOverstrike, _T("ToggleOverstrike") },
	{ SetTabSpace, 		_T("SetTabSpace") },
	{ SetTab, 			_T("SetTab") },
	{ ClearTab, 		_T("ClearTab") },
	{ Tabify, 			_T("Tabify") },
	{ Untab, 			_T("Untab") },
	{ TabsAsSpaces,		_T("TabsAsSpaces") },
	{ ShowWhitespace,	_T("ShowWhitespace") },
	{ HideLineNums,		_T("HideLineNums") },
	{ ShowLineNums,		_T("ShowLineNums") },
	{ HideWhitespace,	_T("HideWhitespace") },
	{ TrimWhitespace,	_T("TrimWhitespace") },
	{ ToggleCarriageReturns, _T("ToggleCR") },
	{ CaseOn,		 	_T("CaseOn") },
	{ CaseOff, 			_T("CaseOff") },
	{ ToggleCase, 		_T("ToggleCase") },
	{ ToggleTrimOnWrite,_T("ToggleTrimOnWrite") },
	{ EnableRefresh, 	_T("EnableRefresh") },
	{ DisableRefresh, 	_T("DisableRefresh") },
	{ SetDirty, 		_T("SetDirty") },
	{ ClearDirty,		_T("ClearDirty") },
	{ ViewHex,			_T("ViewHex") },

	{ BindKey, 			_T("BindKey") },
	{ BindDefaultKey, 	_T("BindDefaultKey") },
	{ SetKeyBinds,		_T("SetKeyBinds") },
	{ ShowKeyBinds, 	_T("ShowKeyBinds") },
	{ BedBind, 			_T("BedBind") },
	{ EmacsBind, 		_T("EmacsBind") },
	{ MSVCBind, 		_T("MSVCBind") },
	{ CWBind,	 		_T("CWBind") },

	{ DoMenu, 			_T("DoMenu") },
	{ Help, 			_T("Help") },
	{ HelpAbout, 		_T("HelpAbout") },
	{ ShowCommands, 	_T("ShowCommands") },
	{ ShowKeyBindings, 	_T("ShowKeyBindings") },

	/*
	 * Commands
	 */
	{ DoCommand, 		_T("DoCommand") },
	{ ReadCommandFile, 	_T("ReadCommandFile") },

	/*
	 * Tags
	 */
	{ SetTagFile, 		_T("SetTagFile") },
	{ FindTag, 			_T("FindTag") },

	/*
	 * Programs
	 */
	{ NewProject, 		_T("NewProject") },
	{ OpenProject, 		_T("OpenProject") },
	{ CloseProject, 	_T("CloseProject") },

	/*
	 * Programs
	 */
	{ Indent, 			_T("Indent") },
	{ IndentLine, 		_T("IndentLine") },
	{ IndentMore, 		_T("IndentMore") },
	{ IndentLess, 		_T("IndentLess") },
	{ IndentLevel, 		_T("IndentLevel") },
	{ IndentSpace, 		_T("IndentSpace") },
	{ Dismember,		_T("Dismember") },
	{ Match, 			_T("Match") },
	{ Build, 			_T("Build") },
	{ Execute,			_T("Execute") },
	{ Target, 			_T("Target") },
	{ CancelBuild, 		_T("CancelBuild") },
	{ NextError, 		_T("NextError") },
	{ CheckIn, 			_T("CheckIn") },
	{ CheckOut, 		_T("CheckOut") },
	{ Revert, 			_T("Revert") },

	{ Debug, 			_T("Debug") },
	{ StopDebug, 		_T("StopDebug") },
	{ Start, 			_T("Start") },
	{ Stop, 			_T("Stop") },
	{ StepIn, 			_T("StepIn") },
	{ StepOut, 			_T("StepOut") },
	{ Step, 			_T("Step") },
	{ SetBreak, 		_T("SetBreak") },
	{ ClearBreak, 		_T("ClearBreak") },
	{ ClearBreak, 		_T("ClearBreak") },
	{ ToggleBreak, 		_T("ToggleBreak") },
	{ RunToCursor, 		_T("RunToCursor") },
	{ ReadVar, 			_T("ReadVar") },
	{ WriteVar, 		_T("WriteVar") },
	{ SetWatchVar, 		_T("SetWatchVar") },
	{ ClearWatchVar, 	_T("ClearWatchVar") },

	/*
	 * Program
	 */
	{ ShellFinished, /* this is really for internal, use */ _T("ShellFinished") },
	{ ExecShell, 		_T("ExecShell") },
	{ ExitBed, 			_T("ExitBed") },

	/*
	 * Inquire
	 */
	{ LineNum, 			_T("LineNum") },
	{ ColumnNum, 		_T("ColumnNum") },
	{ Found, 			_T("Found") },
	{ BufferName, 		_T("BufferName") },
	{ BufferType, 		_T("BufferType") },
	{ BufferTypeAsString,_T("BufferTypeAsString") },
	{ IsModified, 		_T("IsModified") },
	{ IsUntitled, 		_T("IsUntitled") },
	{ IsNew, 			_T("IsNew") },
	{ IsEmpty, 			_T("IsEmpty") },
	{ LineLen,			_T("LineLen") },
	{ LineContent, 		_T("LineContent") },
	{ CurrentChar, 		_T("CurrentChar") },
	{ CurrentCharAsString,	_T("CurrentCharAsString") },
	{ SelectionStartLine,	_T("SelectionStartLine") },
	{ SelectionStartColumn,	_T("SelectionStartColumn") },
	{ SelectionEndLine, 	_T("SelectionEndLine") },
	{ SelectionEndColumn, 	_T("SelectionEndColumn") },

	/*
	 * Dialog
	 */
	{ InquireNumber, 	_T("InquireNumber") },
	{ InquireString, 	_T("InquireString") },
	{ InquireFilename, 	_T("InquireFilename") },
	{ InquireYesNo, 	_T("InquireYesNo") },
	{ InquireYesNoCancel, 	_T("InquireYesNoCancel") },
	{ InquireYesNoSkipAll, 	_T("InquireYesNoSkipAll") },
	{ Message, 			_T("Message") },
	{ SetStatus, 		_T("SetStatus") },

	/*
	 * Misc
	 */
	{ SetFrgColor, 		_T("SetFrgColor") },
	{ SetBkgColor, 		_T("SetBkgColor") },
	{ SetFont, 			_T("SetFont") },

	{ PageSetup, 		_T("PageSetup") },
	{ PageRange, 		_T("PageRange") },
	{ SetPrinterName,	_T("SetPrinterName") },
	{ PrintScreen, 		_T("PrintScreen") },
	{ PrintSelection, 	_T("PrintSelection") },
	{ PrintDocument, 	_T("PrintDocument") },
	{ AquireScreenImage,_T("AquireScreenImage") },

	{ MaxEditCommand, 	_T("Sentinal") }
};

//**************************************************************************
//**************************************************************************
// map of virtual key codes to our own
// key codes (that move the VK codes out
// of the same 8 bit space uses for ASCII)
//
WORD BkeyTranslate[256] =
{
	keyIllegal,			//					 0x00
	keyIllegal,			// VK_LBUTTON        0x01
	keyIllegal,			// VK_RBUTTON        0x02
	keyCtrlC,			// VK_CANCEL         0x03
	keyIllegal,			// VK_MBUTTON        0x04    /* NOT contiguous with L & RBUTTON */

	keyCtrlE,			// 					 0x05
	keyCtrlF,			// 					 0x06
	keyCtrlG,			// 					 0x07

	keyCtrlH,			// VK_BACK           0x08
	keyCtrlI,			// VK_TAB            0x09

	keyCtrlJ,			// 					 0x0A
	keyCtrlK,			// 					 0x0B

	keyCtrlL,			// VK_CLEAR          0x0C
	keyCtrlM,			// VK_RETURN         0x0D

	keyCtrlN,			// 					 0x0E
	keyCtrlO,			// 					 0x0F

	keyShift,			// VK_SHIFT          0x10
	keyCtrl,			// VK_CONTROL        0x11
	keyAlt,				// VK_MENU           0x12
	keyPause,			// VK_PAUSE          0x13
	keyCapsLock,		// VK_CAPITAL        0x14

	keyIllegal,			// VK_KANA           0x15
	keyIllegal,			//			         0x16
	keyIllegal,			// VK_JUNJA          0x17
	keyIllegal,			// VK_FINAL          0x18
	keyIllegal,			// VK_KANJI          0x19

	keyIllegal,			// VK_KANJI          0x1A

	keyEscape,			// VK_ESCAPE         0x1B

	keyIllegal,			// VK_CONVERT        0x1C
	keyIllegal,			// VK_NONCONVERT     0x1D
	keyIllegal,			// VK_ACCEPT         0x1E
	keyIllegal,			// VK_MODECHANGE     0x1F

	keySpace,			// VK_SPACE          0x20
	keyCursorPageUp,	// VK_PRIOR          0x21
	keyCursorPageDown,	// VK_NEXT           0x22
	keyCursorEnd,		// VK_END            0x23
	keyCursorHome,		// VK_HOME           0x24
	keyCursorLeft,		// VK_LEFT           0x25
	keyCursorUp,		// VK_UP             0x26
	keyCursorRight,		// VK_RIGHT          0x27
	keyCursorDown,		// VK_DOWN           0x28
	keyIllegal,			// VK_SELECT         0x29
	keyIllegal,			// VK_PRINT          0x2A
	keyIllegal,			// VK_EXECUTE        0x2B
	keyIllegal,			// VK_SNAPSHOT       0x2C
	keyInsert,			// VK_INSERT         0x2D
	keyDelete,			// VK_DELETE         0x2E
	keyIllegal,			// VK_HELP           0x2F

	key0,				// 					 0x30
	key1,				// 					 0x31
	key2,				// 					 0x32
	key3,				// 					 0x33
	key4,				// 					 0x34
	key5,				// 					 0x35
	key6,				// 					 0x36
	key7,				// 					 0x37
	key8,				// 					 0x38
	key9,				// 					 0x39

	keyIllegal,			//					 0x3A
	keyIllegal,			//					 0x3B
	keyIllegal,			//					 0x3C
	keyIllegal,			//					 0x3D
	keyIllegal,			//					 0x3E
	keyIllegal,			//					 0x3F
	keyIllegal,			//					 0x40

	keyA,				//					 0x41
	keyB,				//					 0x42
	keyC,				//					 0x43
	keyD,				//					 0x44
	keyR,				//					 0x45
	keyF,				//					 0x46
	keyG,				//					 0x47
	keyH,				//					 0x48
	keyI,				//					 0x49
	keyJ,				//					 0x4A
	keyK,				//					 0x4B
	keyL,				//					 0x4C
	keyM,				//					 0x4D
	keyN,				//					 0x4E
	keyO,				//					 0x4F
	keyP,				//					 0x50
	keyQ,				//					 0x51
	keyR,				//					 0x52
	keyS,				//					 0x53
	keyT,				//					 0x54
	keyU,				//					 0x55
	keyV,				//					 0x56
	keyW,				//					 0x57
	keyX,				//					 0x58
	keyY,				//					 0x59
	keyZ,				//					 0x5A

	keyIllegal,			// VK_LWIN           0x5B
	keyIllegal,			// VK_RWIN           0x5C
	keyIllegal,			// VK_APPS           0x5D

	keyIllegal,			//					 0x5E
	keyIllegal,			//					 0x5F

	keyIllegal,			// VK_NUMPAD0        0x60
	keyIllegal,			// VK_NUMPAD1        0x61
	keyIllegal,			// VK_NUMPAD2        0x62
	keyIllegal,			// VK_NUMPAD3        0x63
	keyIllegal,			// VK_NUMPAD4        0x64
	keyIllegal,			// VK_NUMPAD5        0x65
	keyIllegal,			// VK_NUMPAD6        0x66
	keyIllegal,			// VK_NUMPAD7        0x67
	keyIllegal,			// VK_NUMPAD8        0x68
	keyIllegal,			// VK_NUMPAD9        0x69
	keyIllegal,			// VK_MULTIPLY       0x6A
	keyIllegal,			// VK_ADD            0x6B
	keyIllegal,			// VK_SEPARATOR      0x6C
	keyIllegal,			// VK_SUBTRACT       0x6D
	keyIllegal,			// VK_DECIMAL        0x6E
	keyIllegal,			// VK_DIVIDE         0x6F

	keyF1,				// VK_F1             0x70
	keyF2,				// VK_F2             0x71
	keyF3,				// VK_F3             0x72
	keyF4,				// VK_F4             0x73
	keyF5,				// VK_F5             0x74
	keyF6,				// VK_F6             0x75
	keyF7,				// VK_F7             0x76
	keyF8,				// VK_F8             0x77
	keyF9,				// VK_F9             0x78
	keyF10,				// VK_F10            0x79
	keyF11,				// VK_F11            0x7A
	keyF12,				// VK_F12            0x7B
	keyF13,				// VK_F13            0x7C
	keyF14,				// VK_F14            0x7D
	keyF15,				// VK_F15            0x7E
	keyF16,				// VK_F16            0x7F
	keyF17,				// VK_F17            0x80
	keyF18,				// VK_F18            0x81
	keyF19,				// VK_F19            0x82
	keyF20,				// VK_F20            0x83
	keyF21,				// VK_F21            0x84
	keyF22,				// VK_F22            0x85
	keyF23,				// VK_F23            0x86
	keyF24,				// VK_F24            0x87

	keyIllegal,			//					 0x88
	keyIllegal,			//					 0x89
	keyIllegal,			//					 0x8A
	keyIllegal,			//					 0x8B
	keyIllegal,			//					 0x8C
	keyIllegal,			//					 0x8D
	keyIllegal,			//					 0x8E
	keyIllegal,			//					 0x8F

	keyIllegal,			// VK_NUMLOCK        0x90
	keyIllegal,			// VK_SCROLL         0x91

	keyIllegal,			//					 0x92
	keyIllegal,			//					 0x93
	keyIllegal,			//					 0x94
	keyIllegal,			//					 0x95
	keyIllegal,			//					 0x96
	keyIllegal,			//					 0x97
	keyIllegal,			//					 0x98
	keyIllegal,			//					 0x99
	keyIllegal,			//					 0x9A
	keyIllegal,			//					 0x9B
	keyIllegal,			//					 0x9C
	keyIllegal,			//					 0x9D
	keyIllegal,			//					 0x9E
	keyIllegal,			//					 0x9F

/*
 * VK_L* & VK_R* - left and right Alt, Ctrl and Shift virtual keys.
 * Used only as parameters to GetAsyncKeyState() and GetKeyState().
 * No other API or message will distinguish left and right keys in this way.
 */
	keyIllegal,			// VK_LSHIFT         0xA0
	keyIllegal,			// VK_RSHIFT         0xA1
	keyIllegal,			// VK_LCONTROL       0xA2
	keyIllegal,			// VK_RCONTROL       0xA3
	keyIllegal,			// VK_LMENU          0xA4
	keyIllegal,			// VK_RMENU          0xA5

	keyIllegal,			//					 0xA6
	keyIllegal,			//					 0xA7
	keyIllegal,			//					 0xA8
	keyIllegal,			//					 0xA9
	keyIllegal,			//					 0xAA
	keyIllegal,			//					 0xAB
	keyIllegal,			//					 0xAC
	keyIllegal,			//					 0xAD
	keyIllegal,			//					 0xAE
	keyIllegal,			//					 0xAF

	keyIllegal,			//					 0xB0
	keyIllegal,			//					 0xB1
	keyIllegal,			//					 0xB2
	keyIllegal,			//					 0xB3
	keyIllegal,			//					 0xB4
	keyIllegal,			//					 0xB5
	keyIllegal,			//					 0xB6
	keyIllegal,			//					 0xB7
	keyIllegal,			//					 0xB8
	keyIllegal,			//					 0xB9
	keyIllegal,			//					 0xBA
	keyIllegal,			//					 0xBB
	keyIllegal,			//					 0xBC
	keyIllegal,			//					 0xBD
	keyIllegal,			//					 0xBE
	keyIllegal,			//					 0xBF

	keyIllegal,			//					 0xC0
	keyIllegal,			//					 0xC1
	keyIllegal,			//					 0xC2
	keyIllegal,			//					 0xC3
	keyIllegal,			//					 0xC4
	keyIllegal,			//					 0xC5
	keyIllegal,			//					 0xC6
	keyIllegal,			//					 0xC7
	keyIllegal,			//					 0xC8
	keyIllegal,			//					 0xC9
	keyIllegal,			//					 0xCA
	keyIllegal,			//					 0xCB
	keyIllegal,			//					 0xCC
	keyIllegal,			//					 0xCD
	keyIllegal,			//					 0xCE
	keyIllegal,			//					 0xCF

	keyIllegal,			//					 0xD0
	keyIllegal,			//					 0xD1
	keyIllegal,			//					 0xD2
	keyIllegal,			//					 0xD3
	keyIllegal,			//					 0xD4
	keyIllegal,			//					 0xD5
	keyIllegal,			//					 0xD6
	keyIllegal,			//					 0xD7
	keyIllegal,			//					 0xD8
	keyIllegal,			//					 0xD9
	keyIllegal,			//					 0xDA
	keyIllegal,			//					 0xDB
	keyIllegal,			//					 0xDC
	keyIllegal,			//					 0xDD
	keyIllegal,			//					 0xDE
	keyIllegal,			//					 0xDF

	keyIllegal,			//					 0xE0
	keyIllegal,			//					 0xE1
	keyIllegal,			//					 0xE2
	keyIllegal,			//					 0xE3
	keyIllegal,			//					 0xE4

	keyIllegal,			// VK_PROCESSKEY     0xE5

	keyIllegal,			//					 0xE6
	keyIllegal,			//					 0xE7
	keyIllegal,			//					 0xE8
	keyIllegal,			//					 0xE9
	keyIllegal,			//					 0xEA
	keyIllegal,			//					 0xEB
	keyIllegal,			//					 0xEC
	keyIllegal,			//					 0xED
	keyIllegal,			//					 0xEE
	keyIllegal,			//					 0xEF

	keyIllegal,			//					 0xF0
	keyIllegal,			//					 0xF1
	keyIllegal,			//					 0xF2
	keyIllegal,			//					 0xF3
	keyIllegal,			//					 0xF4
	keyIllegal,			//					 0xF5

	keyIllegal,			// VK_ATTN           0xF6
	keyIllegal,			// VK_CRSEL          0xF7
	keyIllegal,			// VK_EXSEL          0xF8
	keyIllegal,			// VK_EREOF          0xF9
	keyIllegal,			// VK_PLAY           0xFA
	keyIllegal,			// VK_ZOOM           0xFB
	keyIllegal,			// VK_NONAME         0xFC
	keyIllegal,			// VK_PA1            0xFD
	keyIllegal,			// VK_OEM_CLEAR      0xFE
	keyIllegal,			// 					 0xFF
};


//**************************************************************************
BkeyBindItem nativeBindings[] =
{
{{ keyCtrlA, 0 },				MoveToBOL },
{{ keyCtrlB, 0 },				MoveToEOB },
{{ keyCtrlC, 0 },				Copy },
{{ keyCtrlD, 0 },				ToggleMark },
{{ keyCtrlE, 0 },				MoveToEOL },
{{ keyCtrlF, 0 },				FindForward },
{{ keyCtrlG, 0 },				MoveToLine },
{{ keyCtrlH, 0 },				DeleteBack },
{{ keyCtrlI, 0 },				SelfInsert }, // tab
{{ keyCtrlJ, 0 },				SelfInsert }, // CR
{{ keyCtrlK, 0 },				DeleteEolOrLineOrBlock },
{{ keyCtrlL, 0 },				RecenterWindow },
{{ keyCtrlM, 0 },				SelfInsert }, // LF
{{ keyCtrlN, 0 },				NextBuffer },
{{ keyCtrlO, 0 },				OpenBuffer },
{{ keyCtrlP, 0 },				ToggleOverstrike },
{{ keyCtrlQ, 0 },				ExitBed },
{{ keyCtrlR, 0 },				SearchAndReplace },
{{ keyCtrlS, 0 },				SaveBuffer },
{{ keyCtrlT, 0 },				MoveToTOB },
{{ keyCtrlU, 0 },				Undo },
{{ keyCtrlV, 0 },				Paste },
{{ keyCtrlW, 0 },				SaveBuffer },
{{ keyCtrlX, 0 },				DeleteEolOrLineOrBlock },
{{ keyCtrlY, 0 },				Redo },
{{ keyCtrlZ, 0 },				Undo },

{{ keyEscape, 0 },				SelfInsert },		/* 1B */

{{ keyFS, 0 },					SelfInsert },
{{ keyGS, 0 },					SelfInsert },
{{ keyRS, 0 },					SelfInsert },
{{ keyUS, 0 },					SelfInsert },

{{ keySpace, 0 },				SelfInsert },		/* 20 */
{{ keyBang, 0 },				SelfInsert },
{{ keyDoubleQuote, 0 },			SelfInsert },
{{ keyPound, 0 },				SelfInsert },
{{ keyDollar, 0 },				SelfInsert },
{{ keyPercent, 0 },				SelfInsert },
{{ keyAmpersand, 0 },			SelfInsert },
{{ keyRightQuote, 0 },			SelfInsert },
{{ keyOpenParen, 0 },			SelfInsert },
{{ keyCloseParen, 0 },			SelfInsert },
{{ keyStar, 0 },				SelfInsert },
{{ keyPlus, 0 },				SelfInsert },
{{ keyComma, 0 },				SelfInsert },
{{ keyMinus, 0 },				SelfInsert },
{{ keyDot, 0 },					SelfInsert },
{{ keySlash, 0 },				SelfInsert },

{{ key0, 0 },					SelfInsert },		/* 30 */
{{ key1, 0 },					SelfInsert },
{{ key2, 0 },					SelfInsert },
{{ key3, 0 },					SelfInsert },
{{ key4, 0 },					SelfInsert },
{{ key5, 0 },					SelfInsert },
{{ key6, 0 },					SelfInsert },
{{ key7, 0 },					SelfInsert },
{{ key8, 0 },					SelfInsert },
{{ key9, 0 },					SelfInsert },

{{ keyColon, 0 },				SelfInsert },		/* 3A */
{{ keySemiColon, 0 },			SelfInsert },
{{ keyLessThan, 0 },			SelfInsert },
{{ keyEquals, 0 },				SelfInsert },
{{ keyMoreThan, 0 },			SelfInsert },
{{ keyHook, 0 },				SelfInsert },
{{ keyAt, 0 },					SelfInsert },		/* 40 */

{{ keyA, 0 },					SelfInsert },		/* 41 */
{{ keyB, 0 },					SelfInsert },
{{ keyC, 0 },					SelfInsert },
{{ keyD, 0 },					SelfInsert },
{{ keyE, 0 },					SelfInsert },
{{ keyF, 0 },					SelfInsert },
{{ keyG, 0 },					SelfInsert },
{{ keyH, 0 },					SelfInsert },
{{ keyI, 0 },					SelfInsert },
{{ keyJ, 0 },					SelfInsert },
{{ keyK, 0 },					SelfInsert },
{{ keyL, 0 },					SelfInsert },
{{ keyM, 0 },					SelfInsert },
{{ keyN, 0 },					SelfInsert },
{{ keyO, 0 },					SelfInsert },
{{ keyP, 0 },					SelfInsert },		/* 50 */
{{ keyQ, 0 },					SelfInsert },
{{ keyR, 0 },					SelfInsert },
{{ keyS, 0 },					SelfInsert },
{{ keyT, 0 },					SelfInsert },
{{ keyU, 0 },					SelfInsert },
{{ keyV, 0 },					SelfInsert },
{{ keyW, 0 },					SelfInsert },
{{ keyX, 0 },					SelfInsert },
{{ keyY, 0 },					SelfInsert },
{{ keyZ, 0 },					SelfInsert },

{{ keyOpenBracket, 0 },			SelfInsert },		/* 5B */
{{ keyBackSlash, 0 },			SelfInsert },
{{ keyCloseBracket, 0 },		SelfInsert },
{{ keyHat, 0 },		SelfInsert },
{{ keyUnderscore, 0 },			SelfInsert },
{{ keyLeftQuote, 0 },			SelfInsert },		/* 60 */

{{ keya, 0 },					SelfInsert },		/* 61 */
{{ keyb, 0 },					SelfInsert },
{{ keyc, 0 },					SelfInsert },
{{ keyd, 0 },					SelfInsert },
{{ keye, 0 },					SelfInsert },
{{ keyf, 0 },					SelfInsert },
{{ keyg, 0 },					SelfInsert },
{{ keyh, 0 },					SelfInsert },
{{ keyi, 0 },					SelfInsert },
{{ keyj, 0 },					SelfInsert },
{{ keyk, 0 },					SelfInsert },
{{ keyl, 0 },					SelfInsert },
{{ keym, 0 },					SelfInsert },
{{ keyn, 0 },					SelfInsert },
{{ keyo, 0 },					SelfInsert },
{{ keyp, 0 },					SelfInsert },		/* 70 */
{{ keyq, 0 },					SelfInsert },
{{ keyr, 0 },					SelfInsert },
{{ keys, 0 },					SelfInsert },
{{ keyt, 0 },					SelfInsert },
{{ keyu, 0 },					SelfInsert },
{{ keyv, 0 },					SelfInsert },
{{ keyw, 0 },					SelfInsert },
{{ keyx, 0 },					SelfInsert },
{{ keyy, 0 },					SelfInsert },
{{ keyz, 0 },					SelfInsert },		/* 7A */

{{ keyOpenBrace, 0 },			SelfInsert },		/* 7B */
{{ keyVertBar, 0 },				SelfInsert },
{{ keyCloseBrace, 0 },			SelfInsert },
{{ keyTilde, 0 },				SelfInsert },
{{ keyDelete, 0 },				DeleteBlock },		/* 7F */

{{ keyCursorLeft, 0 },			MoveLeft },			/* 80 */
{{ keyCursorRight, 0 },			MoveRight },
{{ keyCursorUp, 0 },			MoveUp },
{{ keyCursorDown, 0 },			MoveDown },
{{ keyCursorHome, 0 },			MoveToTOB },
{{ keyCursorPrev, 0 },			MovePageUp },
{{ keyCursorNext, 0 },			MovePageDown },
{{ keyCursorEnd, 0 },			MoveToEOB },
{{ keyCursorPageUp, 0 },		MovePageUp },
{{ keyCursorUpLeft, 0 },		UnknownCommand },
{{ keyCursorDownLeft, 0 },		UnknownCommand },
{{ keyCursorPageDown, 0 },  	MovePageDown },

{{ keySelect, 0 },				UnknownCommand },	/* 8D */
{{ keyPrint, 0 },				UnknownCommand },
{{ keyExec, 0 },				UnknownCommand },
{{ keyInsert, 0 },				ToggleOverstrike },	/* 90 */
{{ keyUndo, 0 },				UnknownCommand },
{{ keyRedo, 0 },				UnknownCommand },
{{ keyMenu, 0 },				UnknownCommand },
{{ keySearch, 0 },  			UnknownCommand },
{{ keyStop, 0 },				UnknownCommand },
{{ keyHelp, 0 },				UnknownCommand },
{{ keyBreak, 0 },	  			UnknownCommand },
{{ keyModeSwitch, 0 },			UnknownCommand },
{{ keyNumLock, 0 },				UnknownCommand },
#if 0
{{ keyShift, 0 },				UnknownCommand },	/* 9A */
{{ keyCtrl, 0 },				UnknownCommand },
#endif
{{ keyAlt, 0 },					UnknownCommand },
{{ keyCapsLock, 0 },			UnknownCommand },
{{ keyPause, 0 },				UnknownCommand },

{{ keyF1, 0 },					Help },				/* 9F */
{{ keyF2, 0 },					DoCommand },
{{ keyF3, 0 },					FindAgain },
{{ keyF4, 0 },					NextWindow },		/* A2 */
{{ keyF5, 0 },					SplitWindow },
{{ keyF6, 0 },					KillWindow },
{{ keyF7, 0 },					FindTag },			/* A5 */
{{ keyF8, 0 },					DefineMacro },
{{ keyF9, 0 },					PlayMacro },
{{ keyF10, 0 },					Build },
{{ keyF11, 0 },					UnknownCommand },
{{ keyF12, 0 },					IndentLine },
{{ keyF13, 0 },					UnknownCommand },
{{ keyF14, 0 },					Undo },				/* AC - Undo */
{{ keyF15, 0 },					UnknownCommand },
{{ keyF16, 0 },					Copy	 },			/* AE - Copy */
{{ keyF17, 0 },					UnknownCommand },
{{ keyF18, 0 },					Paste },			/* B0 - Paste */
{{ keyF19, 0 },					UnknownCommand },
{{ keyF20, 0 },					Cut }, 				/* B2 - Cut */
{{ keyF21, 0 },					UnknownCommand },
{{ keyF22, 0 },					PrintScreen },
{{ keyF23, 0 },					UnknownCommand },
{{ keyF24, 0 },					UnknownCommand },
{{ keyShift, keyInsert, 0 },	Paste },			/* E1 */
{{ keyCtrl,  keyInsert, 0 },	Copy},				/* E2 */
{{ keyShift, keyDelete, 0 },	Cut },
{{ keyAlt,   keyCtrlH , 0 },	Undo },				/* E7 */
{{ keyNULL, 0 },				MaxEditCommand	},	/* sentinel */
};


/*******************************************************************/
/*
 * EMACS sequences to install
 */
BkeyBindItem emacsBindings[] =
{
{{ keyCtrlA, 0 },				MoveToBOL },
{{ keyCtrlB, 0 },				MoveLeft },
{{ keyCtrlC, 0 },				DoCommand },
{{ keyCtrlD, 0 },				Delete },
{{ keyCtrlE, 0 },				MoveToEOL },
{{ keyCtrlF, 0 },				MoveRight },
{{ keyCtrlG, 0 },				AbortKeySequence },
{{ keyCtrlI, 0 },				SelfInsert },
{{ keyCtrlJ, 0 },				SelfInsert },
{{ keyCtrlK, 0 },				DeleteEolOrLineOrBlock },
{{ keyCtrlL, 0 },				RecenterWindow },
{{ keyCtrlM, 0 },				SelfInsert },
{{ keyCtrlN, 0 },				MoveDown },
{{ keyCtrlO, 0 },				ToggleOverstrike },
{{ keyCtrlP, 0 },				MoveUp },
{{ keyCtrlQ, 0 },				ToggleMark },
{{ keyCtrlR, 0 },				FindReverse},
{{ keyCtrlS, 0 },				FindForward },
{{ keyCtrlT, 0 },				MoveToTOB },
{{ keyCtrlU, 0 },				Undo },
{{ keyCtrlV, 0 },				MovePageDown },
{{ keyCtrlW, 0 },				DeleteBlock },
{{ keyCtrlY, 0 },				Paste },
{{ keyCtrlZ, 0 },				ExecShell },

{{ keyFS, 0 },					SelfInsert },
{{ keyGS, 0 },					SelfInsert },
{{ keyRS, 0 },					SelfInsert },
{{ keyUS, 0 },					SelfInsert },

{{ keySpace, 0 },				SelfInsert },		/* 20 */
{{ keyBang, 0 },				SelfInsert },
{{ keyDoubleQuote, 0 },			SelfInsert },
{{ keyPound, 0 },				SelfInsert },
{{ keyDollar, 0 },				SelfInsert },
{{ keyPercent, 0 },				SelfInsert },
{{ keyAmpersand, 0 },			SelfInsert },
{{ keyRightQuote, 0 },			SelfInsert },
{{ keyOpenParen, 0 },			SelfInsert },
{{ keyCloseParen, 0 },			SelfInsert },
{{ keyStar, 0 },				SelfInsert },
{{ keyPlus, 0 },				SelfInsert },
{{ keyComma, 0 },				SelfInsert },
{{ keyMinus, 0 },				SelfInsert },
{{ keyDot, 0 },					SelfInsert },
{{ keySlash, 0 },				SelfInsert },

{{ key0, 0 },					SelfInsert },		/* 30 */
{{ key1, 0 },					SelfInsert },
{{ key2, 0 },					SelfInsert },
{{ key3, 0 },					SelfInsert },
{{ key4, 0 },					SelfInsert },
{{ key5, 0 },					SelfInsert },
{{ key6, 0 },					SelfInsert },
{{ key7, 0 },					SelfInsert },
{{ key8, 0 },					SelfInsert },
{{ key9, 0 },					SelfInsert },

{{ keyColon, 0 },				SelfInsert },		/* 3A */
{{ keySemiColon, 0 },			SelfInsert },
{{ keyLessThan, 0 },			SelfInsert },
{{ keyEquals, 0 },				SelfInsert },
{{ keyMoreThan, 0 },			SelfInsert },
{{ keyHook, 0 },				SelfInsert },
{{ keyAt, 0 },					SelfInsert },		/* 40 */

{{ keyA, 0 },					SelfInsert },		/* 41 */
{{ keyB, 0 },					SelfInsert },
{{ keyC, 0 },					SelfInsert },
{{ keyD, 0 },					SelfInsert },
{{ keyE, 0 },					SelfInsert },
{{ keyF, 0 },					SelfInsert },
{{ keyG, 0 },					SelfInsert },
{{ keyH, 0 },					SelfInsert },
{{ keyI, 0 },					SelfInsert },
{{ keyJ, 0 },					SelfInsert },
{{ keyK, 0 },					SelfInsert },
{{ keyL, 0 },					SelfInsert },
{{ keyM, 0 },					SelfInsert },
{{ keyN, 0 },					SelfInsert },
{{ keyO, 0 },					SelfInsert },
{{ keyP, 0 },					SelfInsert },		/* 50 */
{{ keyQ, 0 },					SelfInsert },
{{ keyR, 0 },					SelfInsert },
{{ keyS, 0 },					SelfInsert },
{{ keyT, 0 },					SelfInsert },
{{ keyU, 0 },					SelfInsert },
{{ keyV, 0 },					SelfInsert },
{{ keyW, 0 },					SelfInsert },
{{ keyX, 0 },					SelfInsert },
{{ keyY, 0 },					SelfInsert },
{{ keyZ, 0 },					SelfInsert },

{{ keyOpenBracket, 0 },			SelfInsert },		/* 5B */
{{ keyBackSlash, 0 },			SelfInsert },
{{ keyCloseBracket, 0 },		SelfInsert },
{{ keyHat, 0 },		SelfInsert },
{{ keyUnderscore, 0 },			SelfInsert },
{{ keyLeftQuote, 0 },			SelfInsert },		/* 60 */

{{ keya, 0 },					SelfInsert },		/* 61 */
{{ keyb, 0 },					SelfInsert },
{{ keyc, 0 },					SelfInsert },
{{ keyd, 0 },					SelfInsert },
{{ keye, 0 },					SelfInsert },
{{ keyf, 0 },					SelfInsert },
{{ keyg, 0 },					SelfInsert },
{{ keyh, 0 },					SelfInsert },
{{ keyi, 0 },					SelfInsert },
{{ keyj, 0 },					SelfInsert },
{{ keyk, 0 },					SelfInsert },
{{ keyl, 0 },					SelfInsert },
{{ keym, 0 },					SelfInsert },
{{ keyn, 0 },					SelfInsert },
{{ keyo, 0 },					SelfInsert },
{{ keyp, 0 },					SelfInsert },		/* 70 */
{{ keyq, 0 },					SelfInsert },
{{ keyr, 0 },					SelfInsert },
{{ keys, 0 },					SelfInsert },
{{ keyt, 0 },					SelfInsert },
{{ keyu, 0 },					SelfInsert },
{{ keyv, 0 },					SelfInsert },
{{ keyw, 0 },					SelfInsert },
{{ keyx, 0 },					SelfInsert },
{{ keyy, 0 },					SelfInsert },
{{ keyz, 0 },					SelfInsert },		/* 7A */

{{ keyOpenBrace, 0 },			SelfInsert },		/* 7B */
{{ keyVertBar, 0 },				SelfInsert },
{{ keyCloseBrace, 0 },			SelfInsert },
{{ keyTilde, 0 },				SelfInsert },
{{ keyDelete, 0 },				DeleteBlock },		/* 7F */

{{ keyCursorLeft, 0 },			MoveLeft },			/* 80 */
{{ keyCursorRight, 0 },			MoveRight },
{{ keyCursorUp, 0 },			MoveUp },
{{ keyCursorDown, 0 },			MoveDown },
{{ keyCursorHome, 0 },			MoveToTOB },
{{ keyCursorPrev, 0 },			MovePageUp },
{{ keyCursorNext, 0 },			MovePageDown },
{{ keyCursorEnd, 0 },			MoveToEOB },
{{ keyCursorPageUp, 0 },		MovePageUp },
{{ keyCursorUpLeft, 0 },		UnknownCommand },
{{ keyCursorDownLeft, 0 },		UnknownCommand },
{{ keyCursorPageDown, 0 },  	MovePageDown },

{{ keySelect, 0 },				UnknownCommand },	/* 8D */
{{ keyPrint, 0 },				UnknownCommand },
{{ keyExec, 0 },				UnknownCommand },
{{ keyInsert, 0 },				ToggleOverstrike },	/* 90 */
{{ keyUndo, 0 },				UnknownCommand },
{{ keyRedo, 0 },				UnknownCommand },
{{ keyMenu, 0 },				UnknownCommand },
{{ keySearch, 0 },  			UnknownCommand },
{{ keyStop, 0 },				UnknownCommand },
{{ keyHelp, 0 },				UnknownCommand },
{{ keyBreak, 0 },	  			UnknownCommand },
{{ keyModeSwitch, 0 },			UnknownCommand },
{{ keyNumLock, 0 },				UnknownCommand },
#if 0
{{ keyShift, 0 },				UnknownCommand },	/* 9A */
{{ keyCtrl, 0 },				UnknownCommand },
#endif
{{ keyAlt, 0 },					UnknownCommand },
{{ keyCapsLock, 0 },			UnknownCommand },
{{ keyPause, 0 },				UnknownCommand },

{{ keyF1, 0 },					Help },				/* 9F */
{{ keyF2, 0 },					DoCommand },
{{ keyF3, 0 },					FindAgain },
{{ keyF4, 0 },					NextWindow },		/* A2 */
{{ keyF5, 0 },					SplitWindow },
{{ keyF6, 0 },					KillWindow },
{{ keyF7, 0 },					FindTag },			/* A5 */
{{ keyF8, 0 },					DefineMacro },
{{ keyF9, 0 },					PlayMacro },
{{ keyF10, 0 },					Build },
{{ keyF11, 0 },					UnknownCommand },
{{ keyF12, 0 },					IndentLine },
{{ keyF13, 0 },					UnknownCommand },
{{ keyF14, 0 },					Undo },				/* AC - Undo */
{{ keyF15, 0 },					UnknownCommand },
{{ keyF16, 0 },					Copy	 },			/* AE - Copy */
{{ keyF17, 0 },					UnknownCommand },
{{ keyF18, 0 },					Paste },			/* B0 - Paste */
{{ keyF19, 0 },					UnknownCommand },
{{ keyF20, 0 },					Cut }, 				/* B2 - Cut */
{{ keyF21, 0 },					UnknownCommand },
{{ keyF22, 0 },					PrintScreen },
{{ keyF23, 0 },					UnknownCommand },
{{ keyF24, 0 },					UnknownCommand },

{{ keyEscape , keyCtrlX },		PlayMacro	}, /*	eval-defun */
{{ keyEscape, keyCtrlQ },		Indent 		}, /*	indent-sexp */

{{ keyCtrl, keyAt },			SetMark		}, /*	set-mark-command */
{{ keyCtrl, keyUnderscore },	Undo		}, /*	undo		*/

{{ keyCtrl, keySpace },			SetMark		}, /* set-mark-command */
{{ keyCtrl, keySlash },			Undo		}, /* undo */

#ifdef fullEmacsSeqs
{{ ALT_KEY|keyHome },,			?			}, /*	beginning-of-buffer-other-window */ */ */
{{ ALT_KEY|keyEnd },			?			}, /*	end-of-buffer-other-window */

{{ ALT_KEY|keyCursorPrev },		?			}, /*scroll-other-window-down */
{{ Atl|keyCursorNext },			?			}, /*scroll-other-window */

{{ CTRL_KEY|keyCursorPrev },	?			}, /* scroll-right */
{{ CTRL_KEY|keyCursorNext },	?			}, /* scroll-left */

{{ CTRL_KEY|keyCursorDown },	?			}, /* forward-paragraph */
{{ CTRL_KEY|keyCursorUp },		?			}, /* backward-paragraph */

{{ keyCtrlH, q },				?			}, /* help-quit */
{{ keyCtrlH, v },				?			}, /* describe-variable */
{{ keyCtrlH, w },				?			}, /* where-is */
{{ keyCtrlH, t },				?			}, /* help-with-tutorial */
{{ keyCtrlH, s },				?			}, /* describe-syntax */
{{ keyCtrlH, p },				?			}, /* finder-by-keyword */
{{ keyCtrlH, n },				?			}, /* view-emacs-news */
{{ keyCtrlH, keyCtrlN },		?			}, /* view-emacs-news */
{{ keyCtrlH, m },				?			}, /* describe-mode */
{{ keyCtrlH, l },				?			}, /* view-lossage */
{{ keyCtrlH, keyCtrlk },		?			}, /* Info-goto-emacs-key-command-node */
{{ keyCtrlH, keyCtrlf },		?			}, /* Info-goto-emacs-command-node */
{{ keyCtrlH, i },				?			}, /* info */
{{ keyCtrlH, F },				?			}, /* view-emacs-FAQ */
{{ keyCtrlH, f },				?			}, /* describe-function */
{{ keyCtrlH, d },				?			}, /* describe-function */
{{ keyCtrlH, k },				?			}, /* describe-key */
{{ keyCtrlH, c },				?			}, /* describe-key-briefly */
{{ keyCtrlH, a },				?			}, /* command-apropos */

{{ keyCtrlX, CTRL_KEY|keyAt },	?			}, /* pop-global-mark */

{{ keyCtrlX, keyCtrlE },		?			}, /* eval-last-sexp */
{{ keyCtrlX, keyCtrlL },		?			}, /* downcase-region */
{{ keyCtrlX, keyCtrlN },		?			}, /* set-goal-column */
{{ keyCtrlX, keyCtrlO },		?			}, /* delete-blank-lines */
{{ keyCtrlX, keyCtrlP },		?			}, /* mark-page */
{{ keyCtrlX, keyCtrlQ },		?			}, /* vc-toggle-read-only */
{{ keyCtrlX, keyCtrlR },		?			}, /* find-file-read-only */
{{ keyCtrlX, keyCtrlT },		?			}, /* transpose-lines */
{{ keyCtrlX, keyCtrlU },		?			}, /* upcase-region */
{{ keyCtrlX, keyCtrlV },		?			}, /* find-alternate-file */
{{ keyCtrlX, keyCtrlY },		?			}, /* iconify-or-deiconify-frame */

{{ keyCtrlX, keyPlus },			?			}, /* balance-windows */
{{ keyCtrlX, keyMinus },		?			}, /* shrink-window-if-larger-than-buffer */
{{ keyCtrlX, keyPeriod },		?			}, /* set-fill-prefix */
{{ keyCtrlX, keySlash},			?			}, /* point-to-register-compatibility-binding */

{{ keyCtrlX, keyDollar },		?			}, /* set-selective-display */
{{ keyCtrlX, keyRightQuote },	?			}, /* expand-abbrev */

{{ keyCtrlX, 6 },				?			}, /* 2c-command */
{{ keyCtrlX, ; },				?			}, /* set-comment-column */
{{ keyCtrlX, < },				?			}, /* scroll-left */
{{ keyCtrlX, = },				?			}, /* what-cursor-position */
{{ keyCtrlX, > },				?			}, /* scroll-right */

{{ keyCtrlX, ^ },				?			}, /* enlarge-window */

{{ keyCtrlX, d },				?			}, /* dired */

{{ keyCtrlX, f },				?			}, /* set-fill-column */
{{ keyCtrlX, g },				?			}, /* insert-register-compatibility-binding */
{{ keyCtrlX, h },				?			}, /* mark-whole-buffer */

{{ keyCtrlX, j },				?			}, /* jump-to-register-compatibility-binding */

{{ keyCtrlX, l },				?			}, /* count-lines-page */
{{ keyCtrlX, m },				?			}, /* mail */
{{ keyCtrlX, n },				?			}, /* Prefix Command */

{{ keyCtrlX, q },				?			}, /* kbd-macro-query */
{{ keyCtrlX, r },				?			}, /* Prefix Command */
{{ keyCtrlX, s },				?			}, /* save-some-buffers */

{{ keyCtrlX, v },				?			}, /* Prefix Command */
{{ keyCtrlX, x },				?			}, /* copy-to-register-compatibility-binding */
{{ keyCtrlX, { },				?			}, /* shrink-window-horizontally */
{{ keyCtrlX, } },				?			}, /* enlarge-window-horizontally */
{{ keyCtrlX, DEL },				?			}, /* backward-kill-sentence */

{{ keyCtrlX, CTRL_KEY|keySpace },?			}, /* pop-global-mark */

{{ keyEscape, CTRL_KEY|keyAt },	?			}, /* mark-sexp */
{{ keyEscape, keyCtrlA },		?			}, /* beginning-of-defun */
{{ keyEscape, keyCtrlB },		?			}, /* backward-sexp */
{{ keyEscape, keyCtrlC },		?			}, /* exit-recursive-edit */
{{ keyEscape, keyCtrlD },		?			}, /* down-list */
{{ keyEscape, keyCtrlE },		?			}, /* end-of-defun */
{{ keyEscape, keyCtrlF },		?			}, /* forward-sexp */
{{ keyEscape, keyCtrlH },		?			}, /* mark-defun */
{{ keyEscape, keyCtrlJ },		?			}, /* indent-new-comment-line */
{{ keyEscape, keyCtrlK },		?			}, /* kill-sexp */
{{ keyEscape, keyCtrlL },		?			}, /* reposition-window */
{{ keyEscape, keyCtrlN },		?			}, /* forward-list */
{{ keyEscape, keyCtrlO },		?			}, /* split-line */
{{ keyEscape, keyCtrlP },		?			}, /* backward-list */
{{ keyEscape, keyCtrlT },		?			}, /* transpose-sexps */
{{ keyEscape, keyCtrlU },		?			}, /* backward-up-list */
{{ keyEscape, keyCtrlB },		?			}, /* scroll-other-window */
{{ keyEscape, keyCtrlE },		?			}, /* append-next-kill */
{{ keyEscape, CTRL_KEY|keyBackSlash },?		}, /* indent-region */
{{ keyEscape, keyBang },		?			}, /* shell-command */
{{ keyEscape, keyDollar },		?			}, /* ispell-word */
{{ keyEscape, keyRightQuote },	?			}, /* abbrev-prefix-mark */
{{ keyEscape, keyOpenParen }, 	?			}, /* insert-parentheses */
{{ keyEscape, keyCloseParen },	?			}, /* move-past-close-and-reindent */
{{ keyEscape, keySlash },		?			}, /* dabbrev-expand */
{{ keyEscape, keyColon },		?			}, /* eval-expression */
{{ keyEscape, keySemiColon },	?			}, /* indent-for-comment */
{{ keyEscape, keyLessThan },	?			}, /* beginning-of-buffer */
{{ keyEscape, keyEqual },		?			}, /* count-lines-region */
{{ keyEscape, keyAt },			?			}, /* mark-word */
{{ keyEscape, keyBackSlash },	?			}, /* delete-horizontal-space */
{{ keyEscape, keyTilde },		?			}, /* delete-indentation */
{{ keyEscape, a },				?			}, /* backward-sentence */
{{ keyEscape, c },				?			}, /* capitalize-word */
{{ keyEscape, d },				?			}, /* kill-word */
{{ keyEscape, e },				?			}, /* forward-sentence */
{{ keyEscape, g },				?			}, /* facemenu-keymap */
{{ keyEscape, h },				?			}, /* mark-paragraph */
{{ keyEscape, j },				?			}, /* indent-new-comment-line */
{{ keyEscape, k },				?			}, /* kill-sentence */
{{ keyEscape, l },				?			}, /* downcase-word */
{{ keyEscape, m },				?			}, /* back-to-indentation */
{{ keyEscape, q },				?			}, /* fill-paragraph */
{{ keyEscape, r },				?			}, /* move-to-window-line */
{{ keyEscape, t },				?			}, /* transpose-words */
{{ keyEscape, u },				?			}, /* upcase-word */
{{ keyEscape, z },				?			}, /* zap-to-char */
{{ keyEscape, keyOpenBrace },	?			}, /* backward-paragraph */
{{ keyEscape, keyOR },			?			}, /* shell-command-on-region */
{{ keyEscape, keyCloseBrace },	?			}, /* forward-paragraph */
{{ keyEscape, keyTilde },		?			}, /* not-modified */
{{ keyEscape, DEL },			?			}, /* backward-kill-word */
{{ keyEscape, Shit|keyCtrlV },	?			}, /* scroll-other-window-down */
{{ keyEscape, CTRL_KEY|keySlash },?			}, /* dabbrev-completion */
{{ keyEscape, CTRL_KEY|keySpace },?			}, /* mark-sexp */
#endif

{{ keyCtrlH, keyB },			ShowKeyBindings	}, /* describe-bindings */
{{ keyCtrlH, keyHook },			Help		}, /* help-for-help */
{{ keyCtrlH, keyF1 },			Help		}, /* help-for-help */
{{ keyCtrlH, keyHelp },			Help		}, /* help-for-help */
{{ keyCtrlH, keyCtrlH, },		Help		}, /* help-for-help  */

{{ keyCtrlX, keyCtrlB },		ListBuffers }, /*  list-buffers */
{{ keyCtrlX, keyCtrlC },		ExitBed		}, /* save-buffers-kill-emacs */
{{ keyCtrlX, keyCtrlD },		OpenBuffer	}, /* list-directory */
{{ keyCtrlX, keyCtrlF },		FindFile	}, /* find-file */
{{ keyCtrlX, keyCtrlI },		Indent		}, /* indent-rigidly */
{{ keyCtrlX, keyCtrlK },		DefineMacro }, /* edit-kbd-macro */
{{ keyCtrlX, keyCtrlS },		SaveBuffer	}, /* save-buffer */
{{ keyCtrlX, keyCtrlW },		SaveBuffer	}, /* write-file */
{{ keyCtrlX, keyCtrlX },		SwapMark	}, /* exchange-point-and-mark */
{{ keyCtrlX, keyOpenParen },	DefineMacro	}, /* start-kbd-macro */
{{ keyCtrlX, keyCloseParen },	PlayMacro	}, /* end-kbd-macro */

{{ keyCtrlX, key0 },			KillWindow	}, /* delete-window */
{{ keyCtrlX, key1 },			KillWindow	}, /* delete-other-windows */
{{ keyCtrlX, key2 },			SplitWindow	}, /* split-window-vertically */
{{ keyCtrlX, key3 },			SplitWindow	}, /* split-window-horizontally */
{{ keyCtrlX, keyOpenBracket },	MovePageUp	}, /* backward-page */
{{ keyCtrlX, keyCloseBracket },	MovePageDown}, /* forward-page */
{{ keyCtrlX, keyLeftQuote },	NextError	}, /* next-error */
{{ keyCtrlX, keyb },			OpenBuffer	}, /* switch-to-buffer */
{{ keyCtrlX, keye },			PlayMacro	}, /* call-last-kbd-macro */
{{ keyCtrlX, keyi },			ReadBuffer	}, /* insert-file */
{{ keyCtrlX, keyk },			KillBuffer	}, /* kill-buffer */
{{ keyCtrlX, keyo },			NextWindow	}, /* other-window */
{{ keyCtrlX, keyu },			Undo		}, /* advertised-undo */

{{ keyEscape, keyCtrlR },		FindReverse	}, /* isearch-backward-regexp */
{{ keyEscape, keyCtrlS },		FindForward	}, /* isearch-forward-regexp */

{{ keyEscape, keySpace },		SelfInsert		}, /* just-one-space */
{{ keyEscape, keyPercent },		Replace		}, /* query-replace */
{{ keyEscape, keyComma },		FindTag		}, /* tags-loop-continue */
{{ keyEscape, keyDot },			FindTag		}, /* find-tag */
{{ keyEscape, keyMoreThan },	MoveToEOB	}, /* end-of-buffer */
{{ keyEscape, keyLeftQuote },	DoMenu		}, /* tmm-menubar */
{{ keyEscape, keyb },			MoveWordLeft	}, /* backward-word */
{{ keyEscape, keyf },			MoveWordRight	}, /* forward-word */
{{ keyEscape, keyi },			Tabify		}, /* tab-to-tab-stop */
{{ keyEscape, keyv },			MovePageUp	}, /* scroll-down */
{{ keyEscape, keyw },			DeleteBlock	}, /* kill-ring-save */
{{ keyEscape, keyx },			DoCommand	}, /* execute-extended-command */
{{ keyEscape, keyy },			Paste		}, /* yank-pop */
{{ keyEscape, keyCtrl, keyDot },FindTag		}, /* find-tag-regexp  */


{{ keyAlt, keyEscape, keyEscape },	AbortKeySequence	}, /* keyboard-escape-quit */

#ifdef fullExtendedEmacsSeqs
{{ ALT_KEY|keyEscape, keyColon },?			}, /* eval-expression */
{{ keyCtrlX, keyEscape :  },	?			}, /*repeat-complex-command */
{{ keyCtrlX, keyEscape, keyEscape  },
		?			}, /* repeat-complex-command */
{{ keyCtrlX, 4, keyCtrlO },		?			}, /* display-buffer */
{{ keyCtrlX, 4, b },			?			}, /* switch-to-buffer-other-window */
{{ keyCtrlX, 4, keyCtrlf },		?			}, /* find-file-other-window */
{{ keyCtrlX, 4, r },			?			}, /* find-file-read-only-other-window */
{{ keyCtrlX, 4, f },			?			}, /* find-file-other-window */
{{ keyCtrlX, 4, m },			?			}, /* mail-other-window */
{{ keyCtrlX, 4, . },			?			}, /* find-tag-other-window */
{{ keyCtrlX, 4, d },			?			}, /* dired-other-window */
{{ keyCtrlX, 4, a },			?			}, /* add-change-log-entry-other-window */

{{ keyCtrlX, 5, o },			?			}, /* other-frame */
{{ keyCtrlX, 5, 0 },			?			}, /* delete-frame */
{{ keyCtrlX, 5, 2 },			?			}, /* make-frame-command */
{{ keyCtrlX, 5, r },			?			}, /* find-file-read-only-other-frame */
{{ keyCtrlX, 5, keyCtrlF },		?			}, /* find-file-other-frame */
{{ keyCtrlX, 5, f },			?			}, /* find-file-other-frame */
{{ keyCtrlX, 5, b },			?			}, /* switch-to-buffer-other-frame */
{{ keyCtrlX, 5, m },			?			}, /* mail-other-frame */
{{ keyCtrlX, 5, keyDot },		?			}, /* find-tag-other-frame */
{{ keyCtrlX, 5, d },			?			}, /* dired-other-frame */

{{ keyCtrlX, a, keyRightQuote, },?			}, /* expand-abbrev */
{{ keyCtrlX, a, e },			?			}, /* expand-abbrev */
{{ keyCtrlX, a, keyMinus },		?			}, /* inverse-add-global-abbrev */
{{ keyCtrlX, a, i },			?			}, /* Prefix Command */
{{ keyCtrlX, a, keyPlus },		?			}, /* add-mode-abbrev */
{{ keyCtrlX, a, g },			?			}, /* add-global-abbrev */
{{ keyCtrlX, a, keyCtrla },		?			}, /* add-mode-abbrev */
{{ keyCtrlX, a, l },			?			}, /* add-mode-abbrev */

{{ keyCtrlX, n, p },			?			}, /* narrow-to-page */
{{ keyCtrlX, n, w },			?			}, /* widen */
{{ keyCtrlX, n, n },			?			}, /* narrow-to-region */

{{ keyCtrlX, r, l },			?			}, /* bookmark-bmenu-list */
{{ keyCtrlX, r, m },			?			}, /* bookmark-set */
{{ keyCtrlX, r, b },			?			}, /* bookmark-jump */
{{ keyCtrlX, r, f },			?			}, /* frame-configuration-to-register */
{{ keyCtrlX, r, w },			?			}, /* window-configuration-to-register */
{{ keyCtrlX, r, t },			?			}, /* string-rectangle */
{{ keyCtrlX, r, o },			?			}, /* open-rectangle */
{{ keyCtrlX, r, y },			?			}, /* yank-rectangle */
{{ keyCtrlX, r, d },			?			}, /* delete-rectangle */
{{ keyCtrlX, r, k },			?			}, /* kill-rectangle */
{{ keyCtrlX, r, c },			?			}, /* clear-rectangle */
{{ keyCtrlX, r, r },			?			}, /* copy-rectangle-to-register */
{{ keyCtrlX, r, g },			?			}, /* insert-register */
{{ keyCtrlX, r, i },			?			}, /* insert-register */
{{ keyCtrlX, r, x },			?			}, /* copy-to-register */
{{ keyCtrlX, r, s },			?			}, /* copy-to-register */
{{ keyCtrlX, r, j },			?			}, /* jump-to-register */
{{ keyCtrlX, r, SPC },			?			}, /* point-to-register */
{{ keyCtrlX, r, keyCtrlSPC },	?			}, /* point-to-register */
{{ keyCtrlX, r, keyCtrl@ },		?			}, /* point-to-register */
#endif
{{ keyCtrlX, keyPound },	MaxEditCommand	}, /* sentinel */
};


//**************************************************************************
BkeyBindItem msvcBindings[] =
{
{{ keyCtrlA, 0 },				MoveToBOL },
{{ keyCtrlB, 0 },				MoveToEOB },
{{ keyCtrlC, 0 },				Copy },
{{ keyCtrlD, 0 },				MovePageUp },
{{ keyCtrlE, 0 },				MoveToEOL },
{{ keyCtrlF, 0 },				OpenBuffer },
{{ keyCtrlG, 0 },				MoveToLine },
{{ keyCtrlH, 0 },				DeleteBack },
{{ keyCtrlI, 0 },				SelfInsert },
{{ keyCtrlJ, 0 },				SelfInsert },
{{ keyCtrlK, 0 },				DeleteEolOrLineOrBlock },
{{ keyCtrlL, 0 },				RecenterWindow },
{{ keyCtrlM, 0 },				SelfInsert },
{{ keyCtrlN, 0 },				NextBuffer },
{{ keyCtrlO, 0 },				ToggleOverstrike },
{{ keyCtrlP, 0 },				PreviousBuffer },
{{ keyCtrlQ, 0 },				ToggleMark },
{{ keyCtrlR, 0 },				SearchAndReplace },
{{ keyCtrlS, 0 },				FindForward },
{{ keyCtrlT, 0 },				MoveToTOB },
{{ keyCtrlU, 0 },				Undo },
{{ keyCtrlV, 0 },				Paste },
{{ keyCtrlW, 0 },				SaveBuffer },
{{ keyCtrlX, 0 },				ExitBed },
{{ keyCtrlY, 0 },				Paste },
{{ keyCtrlZ, 0 },				Undo },

{{ keyEscape, 0 },				SelfInsert },		/* 1B */

{{ keyFS, 0 },					SelfInsert },
{{ keyGS, 0 },					SelfInsert },
{{ keyRS, 0 },					SelfInsert },
{{ keyUS, 0 },					SelfInsert },

{{ keySpace, 0 },				SelfInsert },		/* 20 */
{{ keyBang, 0 },				SelfInsert },
{{ keyDoubleQuote, 0 },			SelfInsert },
{{ keyPound, 0 },				SelfInsert },
{{ keyDollar, 0 },				SelfInsert },
{{ keyPercent, 0 },				SelfInsert },
{{ keyAmpersand, 0 },			SelfInsert },
{{ keyRightQuote, 0 },			SelfInsert },
{{ keyOpenParen, 0 },			SelfInsert },
{{ keyCloseParen, 0 },			SelfInsert },
{{ keyStar, 0 },				SelfInsert },
{{ keyPlus, 0 },				SelfInsert },
{{ keyComma, 0 },				SelfInsert },
{{ keyMinus, 0 },				SelfInsert },
{{ keyDot, 0 },					SelfInsert },
{{ keySlash, 0 },				SelfInsert },

{{ key0, 0 },					SelfInsert },		/* 30 */
{{ key1, 0 },					SelfInsert },
{{ key2, 0 },					SelfInsert },
{{ key3, 0 },					SelfInsert },
{{ key4, 0 },					SelfInsert },
{{ key5, 0 },					SelfInsert },
{{ key6, 0 },					SelfInsert },
{{ key7, 0 },					SelfInsert },
{{ key8, 0 },					SelfInsert },
{{ key9, 0 },					SelfInsert },

{{ keyColon, 0 },				SelfInsert },		/* 3A */
{{ keySemiColon, 0 },			SelfInsert },
{{ keyLessThan, 0 },			SelfInsert },
{{ keyEquals, 0 },				SelfInsert },
{{ keyMoreThan, 0 },			SelfInsert },
{{ keyHook, 0 },				SelfInsert },
{{ keyAt, 0 },					SelfInsert },		/* 40 */

{{ keyA, 0 },					SelfInsert },		/* 41 */
{{ keyB, 0 },					SelfInsert },
{{ keyC, 0 },					SelfInsert },
{{ keyD, 0 },					SelfInsert },
{{ keyE, 0 },					SelfInsert },
{{ keyF, 0 },					SelfInsert },
{{ keyG, 0 },					SelfInsert },
{{ keyH, 0 },					SelfInsert },
{{ keyI, 0 },					SelfInsert },
{{ keyJ, 0 },					SelfInsert },
{{ keyK, 0 },					SelfInsert },
{{ keyL, 0 },					SelfInsert },
{{ keyM, 0 },					SelfInsert },
{{ keyN, 0 },					SelfInsert },
{{ keyO, 0 },					SelfInsert },
{{ keyP, 0 },					SelfInsert },		/* 50 */
{{ keyQ, 0 },					SelfInsert },
{{ keyR, 0 },					SelfInsert },
{{ keyS, 0 },					SelfInsert },
{{ keyT, 0 },					SelfInsert },
{{ keyU, 0 },					SelfInsert },
{{ keyV, 0 },					SelfInsert },
{{ keyW, 0 },					SelfInsert },
{{ keyX, 0 },					SelfInsert },
{{ keyY, 0 },					SelfInsert },
{{ keyZ, 0 },					SelfInsert },

{{ keyOpenBracket, 0 },			SelfInsert },		/* 5B */
{{ keyBackSlash, 0 },			SelfInsert },
{{ keyCloseBracket, 0 },		SelfInsert },
{{ keyHat, 0 },		SelfInsert },
{{ keyUnderscore, 0 },			SelfInsert },
{{ keyLeftQuote, 0 },			SelfInsert },		/* 60 */

{{ keya, 0 },					SelfInsert },		/* 61 */
{{ keyb, 0 },					SelfInsert },
{{ keyc, 0 },					SelfInsert },
{{ keyd, 0 },					SelfInsert },
{{ keye, 0 },					SelfInsert },
{{ keyf, 0 },					SelfInsert },
{{ keyg, 0 },					SelfInsert },
{{ keyh, 0 },					SelfInsert },
{{ keyi, 0 },					SelfInsert },
{{ keyj, 0 },					SelfInsert },
{{ keyk, 0 },					SelfInsert },
{{ keyl, 0 },					SelfInsert },
{{ keym, 0 },					SelfInsert },
{{ keyn, 0 },					SelfInsert },
{{ keyo, 0 },					SelfInsert },
{{ keyp, 0 },					SelfInsert },		/* 70 */
{{ keyq, 0 },					SelfInsert },
{{ keyr, 0 },					SelfInsert },
{{ keys, 0 },					SelfInsert },
{{ keyt, 0 },					SelfInsert },
{{ keyu, 0 },					SelfInsert },
{{ keyv, 0 },					SelfInsert },
{{ keyw, 0 },					SelfInsert },
{{ keyx, 0 },					SelfInsert },
{{ keyy, 0 },					SelfInsert },
{{ keyz, 0 },					SelfInsert },		/* 7A */

{{ keyOpenBrace, 0 },			SelfInsert },		/* 7B */
{{ keyVertBar, 0 },				SelfInsert },
{{ keyCloseBrace, 0 },			SelfInsert },
{{ keyTilde, 0 },				SelfInsert },
{{ keyDelete, 0 },				DeleteBlock },		/* 7F */

{{ keyCursorLeft, 0 },			MoveLeft },			/* 80 */
{{ keyCursorRight, 0 },			MoveRight },
{{ keyCursorUp, 0 },			MoveUp },
{{ keyCursorDown, 0 },			MoveDown },
{{ keyCursorHome, 0 },			MoveToTOB },
{{ keyCursorPrev, 0 },			MovePageUp },
{{ keyCursorNext, 0 },			MovePageDown },
{{ keyCursorEnd, 0 },			MoveToEOB },
{{ keyCursorPageUp, 0 },		MovePageUp },
{{ keyCursorUpLeft, 0 },		UnknownCommand },
{{ keyCursorDownLeft, 0 },		UnknownCommand },
{{ keyCursorPageDown, 0 },  	MovePageDown },

{{ keySelect, 0 },				UnknownCommand },	/* 8D */
{{ keyPrint, 0 },				UnknownCommand },
{{ keyExec, 0 },				UnknownCommand },
{{ keyInsert, 0 },				ToggleOverstrike },	/* 90 */
{{ keyUndo, 0 },				UnknownCommand },
{{ keyRedo, 0 },				UnknownCommand },
{{ keyMenu, 0 },				UnknownCommand },
{{ keySearch, 0 },  			UnknownCommand },
{{ keyStop, 0 },				UnknownCommand },
{{ keyHelp, 0 },				UnknownCommand },
{{ keyBreak, 0 },	  			UnknownCommand },
{{ keyModeSwitch, 0 },			UnknownCommand },
{{ keyNumLock, 0 },				UnknownCommand },
#if 0
{{ keyShift, 0 },				UnknownCommand },	/* 9A */
{{ keyCtrl, 0 },				UnknownCommand },
#endif
{{ keyAlt, 0 },					UnknownCommand },
{{ keyCapsLock, 0 },			UnknownCommand },
{{ keyPause, 0 },				UnknownCommand },

{{ keyF1, 0 },					Help },				/* 9F */
{{ keyF2, 0 },					DoCommand },
{{ keyF3, 0 },					FindAgain },
{{ keyF4, 0 },					NextWindow },		/* A2 */
{{ keyF5, 0 },					SplitWindow },
{{ keyF6, 0 },					KillWindow },
{{ keyF7, 0 },					FindTag },			/* A5 */
{{ keyF8, 0 },					DefineMacro },
{{ keyF9, 0 },					PlayMacro },
{{ keyF10, 0 },					Build },
{{ keyF11, 0 },					UnknownCommand },
{{ keyF12, 0 },					IndentLine },
{{ keyF13, 0 },					UnknownCommand },
{{ keyF14, 0 },					Undo },				/* AC - Undo */
{{ keyF15, 0 },					UnknownCommand },
{{ keyF16, 0 },					Copy	 },			/* AE - Copy */
{{ keyF17, 0 },					UnknownCommand },
{{ keyF18, 0 },					Paste },			/* B0 - Paste */
{{ keyF19, 0 },					UnknownCommand },
{{ keyF20, 0 },					Cut }, 				/* B2 - Cut */
{{ keyF21, 0 },					UnknownCommand },
{{ keyF22, 0 },					PrintScreen },
{{ keyF23, 0 },					UnknownCommand },
{{ keyF24, 0 },					UnknownCommand },
{{ keyShift, keyInsert, 0 },	Paste },			/* E1 */
{{ keyCtrl,  keyInsert, 0 },	Copy},				/* E2 */
{{ keyShift, keyDelete, 0 },	Cut },
{{ keyAlt,   keyCtrlH , 0 },	Undo },				/* E7 */
{{ keyNULL, 0 },				MaxEditCommand	},	/* sentinel */
};

//**************************************************************************
BkeyBindItem codewarriorBindings[] =
{
{{ keyCtrlA, 0 },				MoveToBOL },
{{ keyCtrlB, 0 },				MoveToEOB },
{{ keyCtrlC, 0 },				Copy },
{{ keyCtrlD, 0 },				MovePageUp },
{{ keyCtrlE, 0 },				MoveToEOL },
{{ keyCtrlF, 0 },				OpenBuffer },
{{ keyCtrlG, 0 },				MoveToLine },
{{ keyCtrlH, 0 },				DeleteBack },
{{ keyCtrlI, 0 },				SelfInsert },
{{ keyCtrlJ, 0 },				SelfInsert },
{{ keyCtrlK, 0 },				DeleteEolOrLineOrBlock },
{{ keyCtrlL, 0 },				RecenterWindow },
{{ keyCtrlM, 0 },				SelfInsert },
{{ keyCtrlN, 0 },				NextBuffer },
{{ keyCtrlO, 0 },				ToggleOverstrike },
{{ keyCtrlP, 0 },				PreviousBuffer },
{{ keyCtrlQ, 0 },				ToggleMark },
{{ keyCtrlR, 0 },				SearchAndReplace },
{{ keyCtrlS, 0 },				FindForward },
{{ keyCtrlT, 0 },				MoveToTOB },
{{ keyCtrlU, 0 },				Undo },
{{ keyCtrlV, 0 },				Paste },
{{ keyCtrlW, 0 },				SaveBuffer },
{{ keyCtrlX, 0 },				ExitBed },
{{ keyCtrlY, 0 },				Paste },
{{ keyCtrlZ, 0 },				ExecShell },

{{ keyEscape, 0 },				SelfInsert },		/* 1B */

{{ keyFS, 0 },					SelfInsert },
{{ keyGS, 0 },					SelfInsert },
{{ keyRS, 0 },					SelfInsert },
{{ keyUS, 0 },					SelfInsert },

{{ keySpace, 0 },				SelfInsert },		/* 20 */
{{ keyBang, 0 },				SelfInsert },
{{ keyDoubleQuote, 0 },			SelfInsert },
{{ keyPound, 0 },				SelfInsert },
{{ keyDollar, 0 },				SelfInsert },
{{ keyPercent, 0 },				SelfInsert },
{{ keyAmpersand, 0 },			SelfInsert },
{{ keyRightQuote, 0 },			SelfInsert },
{{ keyOpenParen, 0 },			SelfInsert },
{{ keyCloseParen, 0 },			SelfInsert },
{{ keyStar, 0 },				SelfInsert },
{{ keyPlus, 0 },				SelfInsert },
{{ keyComma, 0 },				SelfInsert },
{{ keyMinus, 0 },				SelfInsert },
{{ keyDot, 0 },					SelfInsert },
{{ keySlash, 0 },				SelfInsert },

{{ key0, 0 },					SelfInsert },		/* 30 */
{{ key1, 0 },					SelfInsert },
{{ key2, 0 },					SelfInsert },
{{ key3, 0 },					SelfInsert },
{{ key4, 0 },					SelfInsert },
{{ key5, 0 },					SelfInsert },
{{ key6, 0 },					SelfInsert },
{{ key7, 0 },					SelfInsert },
{{ key8, 0 },					SelfInsert },
{{ key9, 0 },					SelfInsert },

{{ keyColon, 0 },				SelfInsert },		/* 3A */
{{ keySemiColon, 0 },			SelfInsert },
{{ keyLessThan, 0 },			SelfInsert },
{{ keyEquals, 0 },				SelfInsert },
{{ keyMoreThan, 0 },			SelfInsert },
{{ keyHook, 0 },				SelfInsert },
{{ keyAt, 0 },					SelfInsert },		/* 40 */

{{ keyA, 0 },					SelfInsert },		/* 41 */
{{ keyB, 0 },					SelfInsert },
{{ keyC, 0 },					SelfInsert },
{{ keyD, 0 },					SelfInsert },
{{ keyE, 0 },					SelfInsert },
{{ keyF, 0 },					SelfInsert },
{{ keyG, 0 },					SelfInsert },
{{ keyH, 0 },					SelfInsert },
{{ keyI, 0 },					SelfInsert },
{{ keyJ, 0 },					SelfInsert },
{{ keyK, 0 },					SelfInsert },
{{ keyL, 0 },					SelfInsert },
{{ keyM, 0 },					SelfInsert },
{{ keyN, 0 },					SelfInsert },
{{ keyO, 0 },					SelfInsert },
{{ keyP, 0 },					SelfInsert },		/* 50 */
{{ keyQ, 0 },					SelfInsert },
{{ keyR, 0 },					SelfInsert },
{{ keyS, 0 },					SelfInsert },
{{ keyT, 0 },					SelfInsert },
{{ keyU, 0 },					SelfInsert },
{{ keyV, 0 },					SelfInsert },
{{ keyW, 0 },					SelfInsert },
{{ keyX, 0 },					SelfInsert },
{{ keyY, 0 },					SelfInsert },
{{ keyZ, 0 },					SelfInsert },

{{ keyOpenBracket, 0 },			SelfInsert },		/* 5B */
{{ keyBackSlash, 0 },			SelfInsert },
{{ keyCloseBracket, 0 },		SelfInsert },
{{ keyHat, 0 },		SelfInsert },
{{ keyUnderscore, 0 },			SelfInsert },
{{ keyLeftQuote, 0 },			SelfInsert },		/* 60 */

{{ keya, 0 },					SelfInsert },		/* 61 */
{{ keyb, 0 },					SelfInsert },
{{ keyc, 0 },					SelfInsert },
{{ keyd, 0 },					SelfInsert },
{{ keye, 0 },					SelfInsert },
{{ keyf, 0 },					SelfInsert },
{{ keyg, 0 },					SelfInsert },
{{ keyh, 0 },					SelfInsert },
{{ keyi, 0 },					SelfInsert },
{{ keyj, 0 },					SelfInsert },
{{ keyk, 0 },					SelfInsert },
{{ keyl, 0 },					SelfInsert },
{{ keym, 0 },					SelfInsert },
{{ keyn, 0 },					SelfInsert },
{{ keyo, 0 },					SelfInsert },
{{ keyp, 0 },					SelfInsert },		/* 70 */
{{ keyq, 0 },					SelfInsert },
{{ keyr, 0 },					SelfInsert },
{{ keys, 0 },					SelfInsert },
{{ keyt, 0 },					SelfInsert },
{{ keyu, 0 },					SelfInsert },
{{ keyv, 0 },					SelfInsert },
{{ keyw, 0 },					SelfInsert },
{{ keyx, 0 },					SelfInsert },
{{ keyy, 0 },					SelfInsert },
{{ keyz, 0 },					SelfInsert },		/* 7A */

{{ keyOpenBrace, 0 },			SelfInsert },		/* 7B */
{{ keyVertBar, 0 },				SelfInsert },
{{ keyCloseBrace, 0 },			SelfInsert },
{{ keyTilde, 0 },				SelfInsert },
{{ keyDelete, 0 },				DeleteBlock },		/* 7F */

{{ keyCursorLeft, 0 },			MoveLeft },			/* 80 */
{{ keyCursorRight, 0 },			MoveRight },
{{ keyCursorUp, 0 },			MoveUp },
{{ keyCursorDown, 0 },			MoveDown },
{{ keyCursorHome, 0 },			MoveToTOB },
{{ keyCursorPrev, 0 },			MovePageUp },
{{ keyCursorNext, 0 },			MovePageDown },
{{ keyCursorEnd, 0 },			MoveToEOB },
{{ keyCursorPageUp, 0 },		MovePageUp },
{{ keyCursorUpLeft, 0 },		UnknownCommand },
{{ keyCursorDownLeft, 0 },		UnknownCommand },
{{ keyCursorPageDown, 0 },  	MovePageDown },

{{ keySelect, 0 },				UnknownCommand },	/* 8D */
{{ keyPrint, 0 },				UnknownCommand },
{{ keyExec, 0 },				UnknownCommand },
{{ keyInsert, 0 },				ToggleOverstrike },	/* 90 */
{{ keyUndo, 0 },				UnknownCommand },
{{ keyRedo, 0 },				UnknownCommand },
{{ keyMenu, 0 },				UnknownCommand },
{{ keySearch, 0 },  			UnknownCommand },
{{ keyStop, 0 },				UnknownCommand },
{{ keyHelp, 0 },				UnknownCommand },
{{ keyBreak, 0 },	  			UnknownCommand },
{{ keyModeSwitch, 0 },			UnknownCommand },
{{ keyNumLock, 0 },				UnknownCommand },
#if 0
{{ keyShift, 0 },				UnknownCommand },	/* 9A */
{{ keyCtrl, 0 },				UnknownCommand },
#endif
{{ keyAlt, 0 },					UnknownCommand },
{{ keyCapsLock, 0 },			UnknownCommand },
{{ keyPause, 0 },				UnknownCommand },

{{ keyF1, 0 },					Help },				/* 9F */
{{ keyF2, 0 },					DoCommand },
{{ keyF3, 0 },					FindAgain },
{{ keyF4, 0 },					NextWindow },		/* A2 */
{{ keyF5, 0 },					SplitWindow },
{{ keyF6, 0 },					KillWindow },
{{ keyF7, 0 },					FindTag },			/* A5 */
{{ keyF8, 0 },					DefineMacro },
{{ keyF9, 0 },					PlayMacro },
{{ keyF10, 0 },					Build },
{{ keyF11, 0 },					UnknownCommand },
{{ keyF12, 0 },					IndentLine },
{{ keyF13, 0 },					UnknownCommand },
{{ keyF14, 0 },					Undo },				/* AC - Undo */
{{ keyF15, 0 },					UnknownCommand },
{{ keyF16, 0 },					Copy	 },			/* AE - Copy */
{{ keyF17, 0 },					UnknownCommand },
{{ keyF18, 0 },					Paste },			/* B0 - Paste */
{{ keyF19, 0 },					UnknownCommand },
{{ keyF20, 0 },					Cut }, 				/* B2 - Cut */
{{ keyF21, 0 },					UnknownCommand },
{{ keyF22, 0 },					PrintScreen },
{{ keyF23, 0 },					UnknownCommand },
{{ keyF24, 0 },					UnknownCommand },
{{ keyShift, keyInsert, 0 },	Paste },			/* E1 */
{{ keyCtrl,  keyInsert, 0 },	Copy},				/* E2 */
{{ keyShift, keyDelete, 0 },	Cut },
{{ keyAlt,   keyCtrlH , 0 },	Undo },				/* E7 */
{{ keyNULL, 0 },				MaxEditCommand	},	/* sentinel */
};

