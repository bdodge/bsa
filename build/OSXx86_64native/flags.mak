
# Setup for building on Win32 host for Win32 executable

TARGET_OS= OSX

# this forces sub-archives to be built from object, not libraries
# for archivers that can't handle recursion (like GNU)
#
NORECURSIVE_ARCHIVES	= 1


BINPREFIX=

#CDEPPP	= $(BINPREFIX)gcc
#ADEPPP	= $(BINPREFIX)gcc
CC		= $(BINPREFIX)gcc
CPP		= $(BINPREFIX)g++
ASM		= $(BINPREFIX)gcc
RESC	= $(BINPREFIX)windres

RESCFLAGS= -o

ifeq ($(BUILD_DEBUG), Debug)

CFLAGS	= -c -g -O0
CPPFLAGS= -c -g -O0
ASMFLAGS=
else

CFLAGS	= -c -O
CPPFLAGS= -c -O
ASMFLAGS=

endif

AR		= $(BINPREFIX)ar
ARFLAGS	= -r

LD		= $(CPP)
LDFLAGS	= -g

RM		= rm
RMFLAGS	= -f

CINCLS	= $(INCLUDES:%=-I% )
CDEFS	= $(DEFINES:%=-D%)

AINCLS	= $(CINCLS)
ADEFS	= $(DEFINES:%=-D%)

OBJPREF	= -o
ARPREF	=
LDPREF	= -o
LDPATHPREF	=
LDOBJPREF	=
LDSHLIBPREF	= -l
LDSHLIBSUFX	=
LDLIBPREF   = $(OUTDIR)/
LDLIBSUFX	= .a

OBJEXT	= .o
LIBEXT	= .a
SHLIBEXT= .so
EXEEXT	=
LIBPREF	=

ifndef MACWIN
#	MACWIN=X11
	MACWIN=COCOA
endif


ifeq ($(MACWIN),X11)
	EXTRA_INCLUDES=/usr/X11R6/include /usr/X11R6/include/freetype2
endif

ifeq (,$(findstring HOST_BASED,$(EXTRA_DEFINES)))
	EXTRA_DEFINES += HOST_BASED UNIX OSX $(MACWIN) PTHREADS
endif
