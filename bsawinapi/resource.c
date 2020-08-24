//--------------------------------------------------------------------
//
// File: dialog.c
// Desc: dialogs for winapi
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------


#ifndef Windows
#include "winapix.h"

#include <fcntl.h>
#define O_RAW 0


//-----------------------------------------------------------------------------
long	_zg_cbresource = 0;
LPDWORD _zg_resource   = NULL;

//**************************************************************************
BOOL __LoadResource(LPCTSTR resFile)
{
	int  rfh;
	
	// load a resource file into memory
	// (permanently like)
	//
	if(! resFile)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	if(_zg_resource)
	{
		free(_zg_resource);
		_zg_resource = NULL;
		_zg_cbresource = 0;
	}
#ifdef UNICODE
	{
		char resFileA[MAX_PATH];
	
		TCharToChar(resFileA, resFile);
		rfh = open(resFileA, O_RDONLY | O_RAW);
	}
#else
	rfh = open(resFile, O_RDONLY | O_RAW);
#endif
	if(rfh < 0)
	{
		SetLastError(ERROR_FILE_NOT_FOUND);
		return FALSE;
	}
	// get file size
	_zg_cbresource = lseek(rfh, 0L, SEEK_END);
	lseek(rfh, 0L, SEEK_SET);
	
	_zg_resource = (LPDWORD)malloc(((_zg_cbresource + 3) / 4) * 4);
	if(! _zg_resource)
	{
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		close(rfh);
		return FALSE;
	}
	read(rfh, _zg_resource, _zg_cbresource);
	close(rfh);
	return TRUE;
}

//**************************************************************************
LPDWORD __FindResource(WORD rtype, LPCTSTR rname, LPBYTE ressrc, DWORD ressize)
{
	LPBYTE  resstrt, resbase, res;
	DWORD	cbData;
	DWORD	cbHdr;
	WORD	type;
	WORD	name;
	WORD	tt, nt;
	
	if(! (resbase = ressrc))
		if(! (resbase = (LPBYTE)_zg_resource))
			return NULL;
		
	if(rname > (LPCTSTR)(intptr_t)0xffff) return NULL; // only numbers allowed now
	
	// skip resource header
	//
	resstrt = resbase;
	
	while((resbase - resstrt) < ressize)
	{
		res = resbase;
		
		// read a resource header in
		//
		cbData 	= RESDWORD(res);
		cbHdr  	= RESDWORD(res);
	
		tt		= RESWORD(res);
		type	= RESWORD(res);
		if(tt != 0xffff)
		{
			SetLastError(ERROR_BAD_ENVIRONMENT);
			return NULL;
		}
		nt		= RESWORD(res);
		name	= RESWORD(res);
		if(nt != 0xffff)
		{
			SetLastError(ERROR_BAD_ENVIRONMENT);
			return NULL;
		}
#if 0
		printf("type=%d name=%08X   type=%d name=%08X\n",
				type, name, rtype, rname);
#endif
		if((type & RT_TypeMask) == rtype && (LPCTSTR)(intptr_t)name == rname)
			return (LPDWORD)resbase;
		// resources are dword aligned
		resbase += ((cbData + cbHdr + 3) & ~ 3);
	}
	return NULL;
}

//**************************************************************************
HMENU WINAPI LoadMenuIndirect(CONST LPMENUTEMPLATE menu)
{
	LPZMENU	zMenu;
	LPZMENU	zItem;
	LPZMENU	zLastItem;
	LPZMENU	zLastParent;
	
	LPBYTE  res, resbase;
	int		menuoffset;
	
	WORD	option;
	WORD	id;
	TCHAR	szLabel[128];
	int		tlmenuID = 306;
	int		i;
	int		resbytes;
	
	if(! (res = (LPBYTE)menu)) return NULL;
	resbase  = res;

	// data bytes
	resbytes = RESDWORD(res);
	
	// header bytes
	menuoffset = RESDWORD(res);
	res = resbase + menuoffset;
	
	resbytes += menuoffset;
	
	// menuheader
	//
	id  = RESWORD(res);	// version
	if(id != 0) return NULL;
	id = RESWORD(res);	// cbHeader
	if(id != 0) return NULL;
	
	zMenu = _w_newZMENU(NULL, 0x8000, 0xffff);
	zLastItem   = zMenu;
	zLastParent = zMenu;

	do
	{
		option = RESWORD(res);
		if(! (option & MF_POPUP))
		{
			id = RESWORD(res);
		}
		else
		{
			id = tlmenuID++;
		}
		for(i = 0; i < 127; i++)
		{
			szLabel[i] = RESWORD(res);
			if(szLabel[i] == 0)
				break;
		}
		szLabel[i] = 0;
		
		zItem = _w_newZMENU(szLabel[0] ? szLabel : NULL, option, id);

		zItem->parent = zLastParent;
		if(option & MF_POPUP)
			zLastParent = zItem;
		if(zLastItem)
		{
			if(zLastItem == zMenu || 
					(zLastItem->opts & MF_POPUP && ! (zLastItem->opts & 0x800)))
				zLastItem->child = zItem;
			else
				zLastItem->sibling = zItem;
			zLastItem->opts &= ~ 0x800;
		}
		/*
		{
			LPZMENU pp = zItem;
			
			while(pp->parent)
			{
				_tprintf(_T("  "));
				pp = pp->parent;
			}
			_tprintf(_T("label=%ls opt=%04X is=%d   "), szLabel, option, id);
			if(zLastItem)
				_tprintf(_T("prev=%ls %ls\n"),
						zLastItem->text ? zLastItem->text :_T("--"),
						zLastItem->sibling == zItem ? _T("sibling") : _T("child"));
		}
		*/
		zLastItem = zItem;

		if(option & 0x80 && ! (option & MF_POPUP))
		{
			do
			{
				zLastItem  = zLastItem->parent;
			}
			while(zLastItem->parent != zMenu && (zLastItem->opts & 0x80));
			zLastParent = zLastItem->parent;
			zLastItem->opts |= 0x800;
		}
	}
	while((res - resbase) < resbytes);
	return (HMENU)zMenu;
}

//**************************************************************************
HMENU WINAPI LoadMenu(HINSTANCE hInst, LPCTSTR lpszMenu)
{
	LPDWORD menu;
	
	if(! (menu = __FindResource(RT_Menu, lpszMenu, _zg_resources, _cb_zg_resources)))
	{
		return NULL;
	}
	return LoadMenuIndirect((LPMENUTEMPLATE)menu);
}

#endif
