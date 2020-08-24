#******************************************************************************
#   Copyright (c) 2009, 2010 Zoran Corporation.
#   $Header: //depot/imgeng/sw/inferno/appsrc/build/Linuxx86_64crossLinuxx86_32/flags.mak#1 $
#   $Change: 163747 $ $Date: 2010/11/03 $
#******************************************************************************
# Author: Brian Dodge

# Setup for building on Linux host for Linux executable

export TARGET_OS	= Linux
export HOST_BASED	=	1

# this forces sub-archives to be built from object, not libraries
# for archivers that can't handle recursion (like GNU)
#
NORECURSIVE_ARCHIVES	= 1

EXTRA_INCLUDES=/usr/include/freetype2
EXTRA_LDFLAGS=-L/usr/lib/i386-linux-gnu

BINPREFIX=

#CDEPPP	= $(BINPREFIX)gcc
#ADEPPP	= $(BINPREFIX)gcc
CC		= $(BINPREFIX)gcc
CPP		= $(BINPREFIX)g++
ASM		= $(BINPREFIX)gcc

CFLAGS	= -c -g -Wall -m32 -std=c++98
CPPFLAGS= -c -g -m32
ASMFLAGS= -c -m32

AR		= $(BINPREFIX)ar
ARFLAGS	= -r

LD		= $(CPP)
LDFLAGS	= -m32

CINCLS	= $(INCLUDES:%=-I%)
CDEFS	= $(DEFINES:%=-D %)

AINCLS	= $(CINCLS)
ADEFS	= $(XDEFS)

QUATROTOOLS	= $(ROOT)/compilers/$(HOSTSYS)/quatro-elf/quatro-elf/bin
FLEXRISCTOOLS=$(ROOT)/compilers/$(HOSTSYS)/flexrisc-elf/flexrisc-elf/bin

QUATROTOOLS	= $(ROOT)/compilers/$(HOSTSYS)/quatro-elf/quatro-elf/bin
FLEXRISCTOOLS=$(ROOT)/compilers/$(HOSTSYS)/flexrisc-elf/flexrisc-elf/bin

QASM	=$(QUATROTOOLS)/as
QLD		=$(QUATROTOOLS)/ld
QOBJCOPY=$(QUATROTOOLS)/objcopy

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

# add host based and X11 paths for linux based
#
ifeq (,$(findstring HOST_BASED,$(EXTRA_DEFINES)))
EXTRA_DEFINES += HOST_BASED PTHREADS X11
endif
SYSTEM_SHARED_LIBRARIES+= X11 pthread freetype rt dl

