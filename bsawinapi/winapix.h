//--------------------------------------------------------------------
//
// File: winapix.h
// Desc:
// Auth: Brian Dodge
//
// (C)opyright 2003  - BSA and Brian Dodge
// ALL RIGHTS RESERVED
//
//--------------------------------------------------------------------


#include "genos.h"
#include "bsawinapi.h"
#ifndef Windows
	#if X11
	#ifndef XFT_FONT
		#define XFT_FONT 1
	#endif
	// pad font heights by these many lines. later ubuntu (16.04) has crappy font metrics
	#define X_TRA_PADDING 0
	#include <signal.h>
	#include <sys/time.h>
	#if XFT_FONT
		#include <ft2build.h>
		#include <X11/Xft/Xft.h>
	#endif
	#include <X11/Xatom.h>
	#else
	#define X_TRA_PADDING 0
	#endif
#endif
#include "winapi.h"
#include "resource.h"
#include "winresrc.h"
