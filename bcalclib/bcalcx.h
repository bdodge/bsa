#ifndef _BCALCX_H_
#define _BCALCX_H_ 1

#include "genos.h"
#include <math.h>
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
//#include <afxres.h>
#ifndef IDC_STATIC
#define IDC_STATIC -1
#endif
#include <direct.h>
#include <zmouse.h>
#endif
#include "bsashell.h"
#include "bsaxml.h"
#include "bsapersist.h"
#include "bsaframe.h"
#include "bsaopenfile.h"
#include "bsawiz.h"


#include "bcalc.h"
#include "bnumber.h"
#include "bprog.h"

#ifdef Windows
#include "shellapi.h"
#endif

#include "../bcalc/resource.h"

#endif

