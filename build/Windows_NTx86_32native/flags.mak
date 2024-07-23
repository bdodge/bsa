
# Setup for building on Windows host for Windows executable

TARGET_OS	= $(HOST_OS)

#USEMSVC	=	1

ifdef USEMSVC

MSVCPATH = C:\\PROGRA~1\\MICROS~3\\VC98\\Lib\\
MSVCBIN  = $(MSVCPATH)\\Bin

BINPREFIX=c:\MinGW\bin\\

#CDEPPP	= $(BINPREFIX)gcc
#ADEPPP	= $(BINPREFIX)gcc
CC		= /Program Files/Microsoft Visual Studio/VC98/Bin/cl
CPP		= /Program Files/Microsoft Visual Studio/VC98/Bin/cl
ASM		=

OBJEXT	= .obj
LIBEXT	= .lib
SHLIBEXT= .dll
EXEEXT	= .exe

else

BINPREFIX=c:\\MinGW\\bin\\

#CDEPPP	= $(BINPREFIX)gcc
#ADEPPP	= $(BINPREFIX)gcc
CC		= $(BINPREFIX)gcc
CPP		= $(BINPREFIX)g++
ASM		= $(BINPREFIX)gcc
RESC    = $(BINPREFIX)windres
RESCFLAGS = -o

OBJEXT	= .o
LIBEXT	= .a
SHLIBEXT= .a
EXEEXT	= .exe

# this forces sub-archives to be built from object, not libraries
# for archivers that can't handle recursion (like GNU)
#
NORECURSIVE_ARCHIVES	= 1

endif

ifeq ($(BUILD_DEBUG), Debug)

CFLAGS	= -c -g -m32 -O0
CPPFLAGS= -c -g -m32 -O0
ASMFLAGS=

else

CFLAGS	= -c -m32 -O
CPPFLAGS= -c -m32 -O
ASMFLAGS=

endif

AR		= $(BINPREFIX)ar
ARFLAGS	= -r

LD		= $(CC)
LDFLAGS	=

RM		= rm
RMFLAGS	= -f

CINCLS	= $(INCLUDES:%=-I%)
CDEFS	= $(DEFINES:%=-D %)

AINCLS	= $(CINCLS)
ADEFS	= $(DEFINES:%=--PreDefine "% SETL {TRUE}")

OBJPREF	= -o
ARPREF	=
LDPREF	= -o
LDPATHPREF	= 
LDOBJPREF	=
LDSHLIBPREF	=
LDSHLIBSUFX	=
LDLIBPREF   = $(OUTDIR)\\
LDLIBSUFX	= $(LIBEXT)

LIBPREF	= 

# add host based and defines
#
EXTRA_DEFINES+= HOST_BASED Windows

#LDPATHEXTRA	+=	"C:\PROGRA~1\MICROS~1\VC98\LIB"

# this is if using native compiler
#LDFLAGS		=	$(LDPATH) /subsystem:windows /machine:I386 
LDFLAGS = -mwindows

MSVCPATH = C:\\PROGRA~1\\MICROS~3\\VC98\\Lib\\
MSVCLIBS =	kernel32 user32 gdi32 winspool	\
			comdlg32 advapi32 shell32		\
			ole32 oleaut32 uuid odbc32		\
			odbccp32 wsock32 winmm

EXTRA_LIBRARIES	+=
