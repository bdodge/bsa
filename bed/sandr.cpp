
#include "bedx.h"

// STATIC
//**************************************************************************
void RepositionSandR(HWND hWnd, Bview* pView)
{
	RECT	rc;
	RECT	rcme;
	HWND	hwndParent;
	int		x, y, w, h, l;

	hwndParent = GetParent(hWnd);
	if(! hwndParent)
		hwndParent = GetDesktopWindow();
	GetClientRect(hwndParent, &rc);

	if(pView)
	{
		l = pView->GetCurLine() - pView->GetTopLine();
		l *= pView->GetFontHeight();
	}
	else
	{
		l = 0;
	}

	GetWindowRect(hWnd, &rcme);

	x = ((rc.right - rc.left) - (rcme.right - rcme.left)) / 2;
	if(l >= ((rc.bottom - rc.top) / 2))
	{
		y = (l - (rcme.bottom - rcme.top)) / 2;
	}
	else
	{
		y = l + ((rc.bottom - rc.top) - (rcme.bottom - rcme.top)) / 2;
	}
	if(y < 0) y = 0;
	w = rcme.right - rcme.left;
	h = rcme.bottom - rcme.top;
	
	if(x < 0) x = 0;
	if(y < 0) y = 0;
		
	MoveWindow(hWnd, x, y, w, h, FALSE);
}

// STATIC
//**************************************************************************
BOOL CALLBACK SRProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static Bview* pView = NULL;
	static int    sl = 1, sc = 1;

	ERRCODE ec;
	bool    visitedRT = false;
	
	TCHAR tmp[512];
	TCHAR tmp2[1024];
	int   len;

	switch (message) 
	{
	case WM_INITDIALOG:

		pView = (Bview*)lParam;
		if(pView)
		{
			Bview::EscapeString(tmp2, (LPTSTR)pView->GetSearch(len));
			SetDlgItemText(hWnd, IDC_FT, tmp2);
			Bview::EscapeString(tmp2, (LPTSTR)pView->GetReplacement(len));
			SetDlgItemText(hWnd, IDC_RT, tmp2);
			RepositionSandR(hWnd, pView);
			sl = pView->GetCurLine();
			sc = pView->GetCurCol();
		}
		SetFocus(GetDlgItem(hWnd, IDC_FT));
		return FALSE;

	case WM_COMMAND:

		switch(wParam)
		{
		case IDC_REPLACE:

			if(! visitedRT && (GetFocus() == GetDlgItem(hWnd, IDC_FT)))
			{
				GetDlgItemText(hWnd, IDC_RT, tmp, 512);
				if(! tmp[0])
				{
					visitedRT = true;
					SetFocus(GetDlgItem(hWnd, IDC_RT));
					break;
				}
			}
			if(pView)
			{
				GetDlgItemText(hWnd, IDC_FT, tmp, 512);
				len = Bview::UnescapeString(tmp2, tmp, false);
				pView->PushParm(tmp2, len, ptString);
				ec = pView->Dispatch(SetSearch);
				
				GetDlgItemText(hWnd, IDC_RT, tmp, 512);
				len = Bview::UnescapeString(tmp2, tmp, false);
				pView->PushParm(tmp2, len, ptString);
				ec = pView->Dispatch(SetReplace);
				
				if(pView->OnFoundText())
				{
					ec = pView->Dispatch(ReplaceFoundText);
				}
				ec = pView->Dispatch(FindAgain);

				if(ec != errOK)
				{
					if(pView->RegionSet())
					{
						pView->MoveAbs(sl, sc);
						pView->ResetFoundLocation();
					}
					EndDialog(hWnd, IDOK);
				}
				//else
				//	RepositionSandR(hWnd, pView);
			}
			break;

		case IDNO:

			if(pView)
			{
				GetDlgItemText(hWnd, IDC_FT, tmp, 512);
				len = Bview::UnescapeString(tmp2, tmp, false);
				pView->PushParm(tmp2, len, ptString);
				ec = pView->Dispatch(SetSearch);
				
				GetDlgItemText(hWnd, IDC_RT, tmp, 512);
				len = Bview::UnescapeString(tmp2, tmp, false);
				pView->PushParm(tmp2, len, ptString);
				ec = pView->Dispatch(SetReplace);

				ec = pView->Dispatch(FindAgain);

				if(ec != errOK)
				{
					if(pView->RegionSet())
					{
						pView->MoveAbs(sl, sc);
						pView->ResetFoundLocation();
					}
					EndDialog(hWnd, IDOK);
				}
			}
			break;

		case IDC_ALL:

			if(pView)
			{
				int sl, el;
			
				GetDlgItemText(hWnd, IDC_FT, tmp, 512);
				len = Bview::UnescapeString(tmp2, tmp, false);
				pView->PushParm(tmp2, len, ptString);
				ec = pView->Dispatch(SetSearch);
				
				GetDlgItemText(hWnd, IDC_RT, tmp, 512);
				len = Bview::UnescapeString(tmp2, tmp, false);
				pView->PushParm(tmp2, len, ptString);
				ec = pView->Dispatch(SetReplace);

				ec = pView->Dispatch(DisableRefresh);

				sl = pView->GetCurLine();

				if(! pView->OnFoundText())
				{
					ec = pView->Dispatch(FindAgain);
				}
				do
				{
					if(pView->OnFoundText())
					{
						ec = pView->Dispatch(ReplaceFoundText);
					}
					else
					{
						break;
					}
					ec = pView->Dispatch(FindAgain);
				}
				while(ec == errOK && pView->InRegion(pView->GetCurLine(), pView->GetCurCol()));

				el = pView->GetCurLine();
				pView->Dispatch(EnableRefresh);
				
				if(pView->RegionSet())
				{
					pView->MoveAbs(sl, sc);
					pView->ResetFoundLocation();
				}
				pView->UpdateAllViews(sl, 1, el, 1, pView->GetBuffer());
				EndDialog(hWnd, IDOK);
			}
			break;

		case IDCANCEL:

			EndDialog(hWnd, wParam);
			break;

		default:

			break;
		}
		if(! pView)
			EndDialog(hWnd, IDCANCEL);
		if(! pView->InRegion(pView->GetCurLine(), pView->GetCurCol()))
			EndDialog(hWnd, IDOK);
		break;

	default:
		break;
	}
	return FALSE;
}

//**************************************************************************
ERRCODE Bview::SearchAndReplace()
{
	int rc;

	m_searchlen = 0;

	SetOpRegionFromSelection();

	if(m_regional)
	{
		SetHandMarked(false);

		// if there is a region selected, and only one word in the region
		// assume that user wants to use the selection as the search text
		//
		if(m_ola == m_olb)
		{
			LPCTSTR lpText;
			int     nText;
			ERRCODE ec;

			ec = m_buffer->GetLineText(m_ola, lpText, nText);
			if(ec == errOK)
			{
				bool oneword;
				int  scol, ecol, col;

				if(m_oca <= m_ocb)
				{
					scol = m_oca;
					ecol = m_ocb;
				}
				else
				{
					scol = m_ocb;
					ecol = m_oca;
				}
				for(col = scol - 1, oneword = true; oneword && (col < (ecol - 1)); col++)
					if(IsDelim(lpText[col]))
						oneword = false;
				if(oneword)
				{
					col = ecol - scol;

					if(col < sizeof(m_searchstr))
					{
						_tcsncpy(m_searchstr, lpText + scol - 1, col);
						m_searchstr[col] = _T('\0');
						m_searchlen = col;
					}
					m_regional = false;
				}
			}
		}
	}
	rc = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_SANDR), m_hwnd, SRProc, (LPARAM)this);

	ClearOpRegion();
	return errOK;
}
