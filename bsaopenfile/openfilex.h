#include "genos.h"
#ifndef Windows
#include "bsawinapi.h"
#include <dirent.h>
#else
#ifdef __GNUC__
    #define _WIN32_IE 0x600
#endif

#undef Windows

#include <commctrl.h>
//#include <afxres.h>
#include <shlobj.h>
#include <direct.h>

#define Windows 1
#endif

#include "bsautils.h"
#include "bsaopenfile.h"
#include "ofpriv.h"
