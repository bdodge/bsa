
#include "bedx.h"

//**************************************************************************
BOOL CALLBACK PickBinFormatWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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

		switch(lpParm->nString & 0xF)
		{
		default:
		case 1:
			CheckDlgButton(hDlg, IDC_VIEW_BYTES, BST_CHECKED);
			break;
		case 2:
			CheckDlgButton(hDlg, IDC_VIEW_WORDS, BST_CHECKED);
			break;
		case 4:
			CheckDlgButton(hDlg, IDC_VIEW_DWORDS, BST_CHECKED);
			break;
		}
		if(lpParm->nString & 0x10)
		{
			CheckDlgButton(hDlg, IDC_SUPPRESSADDR, BST_CHECKED);
		}
		if(lpParm->nString & 0x20)
		{
			CheckDlgButton(hDlg, IDC_VIEWBYTESWAPPED, BST_CHECKED);
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

		case IDC_VIEW_BYTES:
			lpParm->nString &= ~0xF;
			lpParm->nString |= 1;
			break;

		case IDC_VIEW_WORDS:
			lpParm->nString &= ~0xF;
			lpParm->nString |= 2;
			break;

		case IDC_VIEW_DWORDS:
			lpParm->nString &= ~0xF;
			lpParm->nString |= 4;
			break;

		case IDC_SUPPRESSADDR:
			lpParm->nString ^= 0x10;
			break;

		case IDC_VIEWBYTESWAPPED:
			lpParm->nString ^= 0x20;
			break;

		default:
			break;
		}
	}
    return FALSE;
}

//**************************************************************************
int PickBinFormatDialog(LPPARMPASS pParm, HWND hwndParent)
{
	int rc = DialogBoxParam(
							g_hInstance,
							MAKEINTRESOURCE(IDD_HEXFORMAT),
							hwndParent,
							PickBinFormatWndProc,
							(LPARAM)pParm
						);
	return rc;
}

// STATIC
//**************************************************************************
int Bview::UnescapeString(LPTSTR pRes, LPCTSTR pSrc, bool isPath)
{
	LPTSTR pBase, pEon;
	TCHAR  c;
	bool   done;
	int    iv;
	int	   radix;

	if(! pSrc || ! pRes)
		return 0;

	done  = false;
	pBase = pRes;

	if(*pSrc == _T('~'))
	{
		if(BUtil::GetHomePath(pRes, MAX_PATH) == errOK)
		{
			iv = _tcslen(pRes);

			pRes += iv;
			pSrc++;

			if(iv > 0 && pRes[-1] == _PTC_)
				if(*pSrc == _PTC_)
					pSrc++;
		}
	}

	do
	{
		c = *pSrc++;

		switch(c)
		{
		case _T('\0'):

			*pRes++ = c;
			done = true;
			break;

		case _T('\\'):

			if(isPath && _PTC_ == c)
			{
				*pRes++ = c;
				break;
			}
			c = *pSrc++;
			switch(c)
			{
			case _T('0'):
				if(*pSrc == _T('x') || *pSrc == _T('X'))
				{
					pSrc++;
					radix = 16;
				}
				else if(*pSrc >= _T('0') && *pSrc <= _T('7'))
				{
					radix = 8;
				}
				else
				{
					*pRes++ = 0;
					break;
				}
				iv = _tcstol(pSrc, &pEon, radix);
				pSrc = (LPCTSTR)pEon;
				*pRes++ = iv;
				break;

			case _T('1'): case _T('2'): case _T('3'): case _T('4'):
			case _T('5'): case _T('6'): case _T('7'): case _T('8'): case _T('9'):
				iv = _tcstol(pSrc-1, &pEon, 0);
				pSrc = (LPCTSTR)pEon;
				*pRes++ = iv;
				break;

			case _T('x'):
				iv = _tcstol(pSrc, &pEon, 16);
				pSrc = (LPCTSTR)pEon;
				*pRes++ = iv;
				break;

			case _T('e'):
				*pRes++ = 27;
				break;

			case _T('r'):
				*pRes++ = 13;
				break;

			case _T('f'):
				*pRes++ = 12;
				break;

			case _T('v'):
				*pRes++ = 11;
				break;

			case _T('n'):
				*pRes++ = 10;
				break;

			case _T('t'):
				*pRes++ = 9;
				break;

			case _T('b'):
				*pRes++ = 8;
				break;

			case '\\':
				*pRes++ = c;
				break;

			default:
				*pRes++ = _T('\\');
				*pRes++ = c;
				break;
			}
			break;

		default:

			*pRes++ = c;
			break;
		}
	}
	while(! done);

	return pRes - pBase - 1;
}

// STATIC
//**************************************************************************
int Bview::EscapeString(LPTSTR pRes, LPCTSTR pSrc)
{
	LPTSTR pBase, pEon;
	TCHAR  c;
	bool   done;
	int    iv;
	int	   radix;

	if(! pSrc || ! pRes)
		return 0;

	done  = false;
	pBase = pRes;

	do
	{
		c = *pSrc++;

		switch(c)
		{
		case _T('\0'):
			*pRes++ = c;
			done = true;
			break;

		case 8:
			*pRes++ = _T('\\');
			*pRes++ = _T('b');
			break;

		case 9:
			*pRes++ = _T('\\');
			*pRes++ = _T('t');
			break;

		case 10:
			*pRes++ = _T('\\');
			*pRes++ = _T('n');
			break;

		case 11:
			*pRes++ = _T('\\');
			*pRes++ = _T('v');
			break;

		case 12:
			*pRes++ = _T('\\');
			*pRes++ = _T('f');
			break;

		case 13:
			*pRes++ = _T('\\');
			*pRes++ = _T('r');
			break;

		case 27:
			*pRes++ = _T('\\');
			*pRes++ = _T('e');
			break;

		case _T('\\'):
			*pRes++ = _T('\\');
			*pRes++ = _T('\\');
			break;

		default:
			if(c < 0x20 || c > 0x7E)
			{
				*pRes++ = '\\';
				*pRes++ = '0';
				*pRes++ = 'x';
				if(((c & 0xF0) >> 4) >= 10)
				{
					*pRes++ = ((c & 0xF0) >> 4) - 10 + 'A';
				}
				else
				{
					*pRes++ = ((c & 0xF0) >> 4) + '0';
				}
				if((c & 0xF) >= 10)
				{
					*pRes++ = (c & 0xF) - 10 + 'A';
				}
				else
				{
					*pRes++ = (c & 0xF) + '0';
				}
			}
			else
			{
				*pRes++ = c;
			}
			break;
		}
	}
	while(! done);

	return pRes - pBase - 1;
}

// STATIC
//**************************************************************************
BOOL CALLBACK StringProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static LPPARMPASS lpParm = NULL;

	RECT	rc;
	RECT	rcme;
	HWND	hwndString;
	HWND	hwndParent;

	switch (message)
	{
	case WM_INITDIALOG:
		lpParm = (LPPARMPASS)lParam;
		if(lpParm)
		{
			SetWindowText(hWnd, (LPTSTR)lpParm->lpTitle);
			SetDlgItemText(hWnd, IDC_PROMPT, (LPTSTR)lpParm->lpPrompt);

			hwndString = GetDlgItem(hWnd, IDC_STRING);
			if(hwndString)
			{
				TCHAR tmp2[1024];

				Bview::EscapeString(tmp2, lpParm->lpInitVal);
				SetDlgItemText(hWnd, IDC_STRING, tmp2);
				if(lpParm->type == ptNumber)
					SetWindowLong(hwndString, GWL_STYLE, GetWindowLong(hwndString, GWL_STYLE) | ES_NUMBER);
			}
			SetFocus(GetDlgItem(hWnd, IDC_STRING));
		}
		hwndParent = GetParent(hWnd);
		if(! hwndParent)
			hwndParent = GetDesktopWindow();
		GetClientRect(hwndParent, &rc);
		GetWindowRect(hWnd, &rcme);
		{
			int x = ((rc.right - rc.left) - (rcme.right - rcme.left)) / 2;
			int y = ((rc.bottom - rc.top) - (rcme.bottom - rcme.top)) / 2;
			int w = rcme.right - rcme.left;
			int h = rcme.bottom - rcme.top;

			if(x < 0) x = 0;
			if(y < 0) y = 0;

			MoveWindow(hWnd, x, y, w, h, FALSE);
		}
		return FALSE;
		break;

	case WM_COMMAND:

		switch(wParam)
		{
		case IDOK:
			if(lpParm)
			{
				LPTSTR tmp = new TCHAR [ lpParm->nString + MAX_PATH + 32 ];

				GetDlgItemText(hWnd, IDC_STRING, tmp, lpParm->nString);
				lpParm->nString = Bview::UnescapeString(lpParm->lpString, tmp, false);
				delete [] tmp;
			}
			EndDialog(hWnd, wParam);
			break;
		case IDCANCEL:
			EndDialog(hWnd, wParam);
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}
	return 0;
}

//**************************************************************************
tagParm::tagParm(LPCTSTR pParm, int nParm, ParmType parmType)
{
	m_type  = parmType;
	m_pParm = new TCHAR [ nParm + 2 ];
	memcpy(m_pParm, pParm, nParm * sizeof(TCHAR));
	m_pParm[nParm] = _T('\0');
	m_nParm = nParm;
}

//**************************************************************************
tagParm::tagParm(int parm, ParmType parmType)
{
	m_type  = parmType;
	m_pParm = NULL;
	m_nParm = parm;
}

//**************************************************************************
tagParm::~tagParm()
{
	if(m_pParm) delete [] m_pParm;
}

//**************************************************************************
ERRCODE Bview::PushParm(LPCTSTR pParmval, int nParm, ParmType type)
{
	LPPARM pParm;

	if(m_recording && ! m_suppressmacro)
		AddToMacro(new MACROREC(PushString, pParmval, nParm, type));
	pParm = new PARM(pParmval, nParm, type);
	pParm->m_next = m_parmstack;
	m_parmstack = pParm;
	return errOK;
}

//**************************************************************************
ERRCODE Bview::PushParm(int parm, ParmType type)
{
	LPPARM pParm;

	if(m_recording && ! m_suppressmacro)
		AddToMacro(new MACROREC(PushNumber, parm, type));
	pParm = new PARM(parm, type);
	pParm->m_next = m_parmstack;
	m_parmstack = pParm;
	return errOK;
}

//**************************************************************************
ERRCODE Bview::PopParm(LPCTSTR pPrompt, ParmType type, LPCTSTR& pParm, int& nParm, LPCTSTR pInitParm)
{
	LPPARM	 pPop;
	PARMPASS parm;
	TCHAR	 stringBuffer[MAX_PATH];
	TCHAR	 titleBuffer[128];
	BufType	 buftype;
	TEXTENCODING encoding;
	LINETERMINATION lineterm;

	if(! m_parmstack)
	{
		TCHAR versbuffer[64];
		int rc;

		if(! pPrompt)
			return errUNDERFLOW;

		_tcsncpy(versbuffer, m_buffer->GetEditor()->GetVersion(), sizeof(versbuffer)/sizeof(TCHAR));
		_sntprintf(titleBuffer, 128, _T("" _Pfs_ " - " _Pfs_ ""), versbuffer, pPrompt);

		parm.lpTitle	= titleBuffer;
		parm.lpPrompt	= pPrompt;
		parm.lpInitVal	= pInitParm ? pInitParm : _T("");
		parm.lpString	= stringBuffer;
		parm.nString	= MAX_PATH;
		parm.type		= type;
		parm.pView		= this;

		switch(type)
		{
		case ptOpenFilename:
		case ptOpenExistingFilename:
		case ptSaveFilename:
		case ptDirectory:

			encoding = (TEXTENCODING)-1;
			buftype  = btAny;
#ifdef Windows
			lineterm = ltCRLF;
#else
			lineterm = ltLF;
#endif
			stringBuffer[0] = _T('\0');
			if(type == ptSaveFilename)
			{
				if(m_buffer)
				{
					encoding = m_buffer->GetEncoding();
					lineterm = m_buffer->GetHasCR() ? ltCRLF : ltLF;
					_tcsncpy(stringBuffer, m_buffer->GetName(), MAX_PATH-1);
					stringBuffer[MAX_PATH-1] = _T('\0');
				}
			}
			rc = PickFileDialog(titleBuffer, type == ptOpenExistingFilename,
				type == ptOpenFilename || type == ptOpenExistingFilename,
				stringBuffer, MAX_PATH, encoding, lineterm, buftype, m_hwnd);

			if(rc == IDOK)
			{
				LPTSTR tmp;
				int    nString;

				PushParm(buftype,  ptBufferType);
				PushParm(lineterm, ptTextLineTerm);
				PushParm(encoding, ptTextEncoding);
				nString = _tcslen(stringBuffer);
				tmp = new TCHAR [ nString + MAX_PATH + 32 ];
				nString = Bview::UnescapeString(tmp, stringBuffer, true);
				PushParm(tmp, nString, type);
				delete [] tmp;
			}
			break;

		case ptString:
		case ptNumber:
		case ptTextEncoding:
		case ptBufferType:

			rc = DialogBoxParam(
								g_hInstance,
								MAKEINTRESOURCE(IDD_GENSTRING),
								m_hwnd,
								StringProc,
								(LPARAM)&parm
								);
			if(rc == IDOK)
			{
				if(type == ptNumber)
					PushParm(_tcstol(parm.lpString, NULL, 0), parm.type);
				else
					PushParm(parm.lpString, parm.nString, parm.type);
			}
			break;

		case ptKeyBindings:

			parm.nString = nParm;
			rc = PickKeyBindingDialog(&parm, m_hwnd);

			if(rc == IDOK)
				PushParm(parm.nString, ptKeyBindings);
			break;

		case ptBinFormat:

			parm.nString = nParm;
			rc = PickBinFormatDialog(&parm, m_hwnd);

			if(rc == IDOK)
				PushParm(parm.nString, ptBinFormat);
			break;

		case ptFontSpec:

			parm.lpInitVal = pParm;
			rc = PickFontDialog(&parm, m_hwnd);
			if(rc == IDOK)
			{
				PushParm(parm.lpString, parm.nString, ptFontSpec);
			}
			break;

		case ptColor:

			parm.lpInitVal = pParm;
			rc = PickColorDialog(&parm, m_hwnd);
			if(rc == IDOK)
			{
				PushParm(parm.lpString, parm.nString, ptColor);
			}
			break;

		default:
			break;
		}
	}
	if(! m_parmstack)
		return errUNDERFLOW;

	switch(type)
	{
	default:
	case ptString:
	case ptOpenFilename:
	case ptOpenExistingFilename:
	case ptSaveFilename:
	case ptDirectory:
		if(
				m_parmstack->m_type != ptString 				&&
				m_parmstack->m_type != ptOpenFilename 			&&
				m_parmstack->m_type != ptSaveFilename 			&&
				m_parmstack->m_type != ptOpenExistingFilename 	&&
				m_parmstack->m_type != ptDirectory
		)
			return errFAILURE;
		break;

	case ptNumber:
		if(
				m_parmstack->m_type != ptNumber 		&&
				m_parmstack->m_type != ptColor			&&
				m_parmstack->m_type != ptTextEncoding	&&
				m_parmstack->m_type != ptBufferType		&&
				m_parmstack->m_type != ptKeyBindings
		)
			return errFAILURE;
		break;

	case ptFontSpec:
		if(m_parmstack->m_type != ptFontSpec)
			return errFAILURE;
		break;

	case ptColor:
		if(m_parmstack->m_type != ptColor)
			return errFAILURE;
		break;

	case ptTextEncoding:
		if(m_parmstack->m_type != ptTextEncoding)
			return errFAILURE;
		break;

	case ptTextLineTerm:
		if(m_parmstack->m_type != ptTextLineTerm)
			return errFAILURE;
		break;

	case ptBufferType:
		if(m_parmstack->m_type != ptBufferType)
			return errFAILURE;
		break;

	case ptKeyBindings:
		if(m_parmstack->m_type != ptKeyBindings)
			return errFAILURE;
		break;

	case ptBinFormat:
		if(m_parmstack->m_type != ptBinFormat)
			return errFAILURE;
		break;
	}
	pParm		 = m_parmstack->m_pParm;
	nParm		 = m_parmstack->m_nParm;
	pPop		 = m_parmstack;
	m_parmstack  = m_parmstack->m_next;
	pPop->m_next = m_oldparms;
	m_oldparms   = pPop;
	return errOK;
}

//**************************************************************************
ERRCODE Bview::PopParm(LPCTSTR pPrompt, ParmType type, int& parm)
{
	ERRCODE ec;
	LPCTSTR pParm;
	TCHAR	szInit[32];
	int		nParm;

	nParm = parm;
	if(parm != 0)
		_sntprintf(szInit, 32, _T("%d"), parm);
	else
		szInit[0] = '\0';
	ec	= PopParm(pPrompt, type, pParm, nParm, szInit);
	if(ec != errOK) return ec;
	parm = nParm;
	return errOK;
}

//**************************************************************************
ERRCODE Bview::FreeOldParms()
{
	LPPARM pFree;

	while(m_oldparms)
	{
		pFree = m_oldparms;
		m_oldparms = pFree->m_next;
		delete pFree;
	}
	return errOK;
}

