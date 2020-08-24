#ifndef _BS_X_H_
#define _BS_X_H_ 1

#if defined(Darwin)||defined(OSX)
#define BSA_IPX 0
#else
#define BSA_IPX 1
#endif

#include "genos.h"
#ifdef Windows
#include <winsock.h>
#include <wsipx.h>
#else
#if BSA_IPX
#include <netipx/ipx.h>
#endif
#endif
#include "bsautils.h"
#include "bsabs.h"


#endif

