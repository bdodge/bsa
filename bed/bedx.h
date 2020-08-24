#ifndef _BEDX_H_
#define _BEDX_H_ 1

#include "genos.h"
#include "bsautils.h"
#include "bsabs.h"
#ifndef Windows
#include "bsawinapi.h"
#include <dirent.h>
#else
#ifdef __GNUC__
    #define _WIN32_IE 0x600
#endif
#include <commctrl.h>
#include <direct.h>
#include <zmouse.h>
#endif
#include "bsashell.h"
#include "bsaxml.h"
#include "bsapersist.h"
#include "bsaframe.h"
#include "bsaopenfile.h"
#include "bsawiz.h"
#include "bsaftp.h"
#include "bsasercom.h"
#include "bsatelcom.h"
#include "bcalc.h"

#include "line.h"
#include "parm.h"
#include "keybind.h"
#include "buffer.h"
#include "pdb.h"
#include "project.h"
#include "infopane.h"
#include "bdbg.h"
#include "infodbg.h"
#include "print.h"
#include "view.h"
#include "viewshell.h"
#include "bsccs.h"
#include "tclscript.h"
#include "bed.h"

#ifdef Windows
#include "shellapi.h"
#endif
#if 1
#ifdef Windows
#include <io.h>
#else
#if !defined(Darwin) && !defined(OSX)
#include <sys/io.h>
#endif
#endif
#include <fcntl.h>
#endif

#include "resource.h"

#include "stdids.h"

#endif

