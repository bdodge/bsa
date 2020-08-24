//--------------------------------------------------------------------
//
// File: bsaopefile.h
// Desc: generic file-open
// Auth: Brian Dodge
//
// (C)opyright 2003-2005  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------
#ifndef _BSAOPENFILE_H_
#define _BSAOPENFILE_H_

int OpenFileDialog(
				   HINSTANCE	hInstance,
				   LPCTSTR		lpTitle,
				   bool			openExisting,
				   bool			openorsave,
				   bool			dirsonly,
				   bool			qpenabled,
				   LPTSTR		lpName,
				   int			nName,
				   HWND			hwndParent,
				   DLGPROC		msgHook
				  );

#endif
