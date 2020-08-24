#******************************************************************************
# Author: Brian Dodge
#
# This is the "master" template makefile which includes
# other appropriate makefiles as needed based on settings
#
# This makefile knows how to make dependencies and objects from
#
# .c, .cpp, .s, and .rc files
#
# This makefile also has the following convenience targets
#
# flags:      output the variables and flags used in the build process
# clean:      clean up intermediate files
# doc:        build documentation from build tree
# devtar:     build tar file of all sources needed to dupicate development build tree
# reltar:     build tar file of all sources/tools to ship as a release (e.g. to a customer)
#
# The following variables effect the make process
#
# HOST_ARCH   = x86_32, 4500, Windows, etc. - this is the build system host machine cpu
# HOST_OS     = Linux, Window_NT, etc. - operating system on the build machine
#
# TARGET_ARCH = x86_32, x86_64, 4310, 4500, etc. - this is the platform system cpu
# TARGET_OS   = Linux, Windows_NT, THREADX, Nucleus, etc. - target operating system
# TARGET_REV  = target revision, defaults as blank
# BOARD       = board (development board or product) platform to run code on
#
# BUILD		  = Name of build target as in the thing to build, defaults
#               to the name of the build directory
# BUILD_TYPE  = Executable, Library, SharedLibrary, HostExecutable - this is the thing that
#               should be built, defaults to Executable or HostExecutable (if HOST==TARGET)
#               Tool builds automatically overide to HostExecutable
# BUILD_DEBUG = Debug, Release - general build flavor wrt. optimization etc.
#               defaults to Debug
#
# SOURCES     = list of source files, no prepended path> OR "All" which means list each source
# LIBRARIES          = list of static lib files to link with (and dependencies for recursive make)
# SHARED_LIBRARIES   = list of shared lib files to link with (and dependencies for recursive make)
#
# SYSTEM_LIBRARIES          = list of static lib files to link in, never build
# SYSTEM_SHARED_LIBRARIES   = list of dynamic lib files to link in, but never build
# PREBUILT_SHARED_LIBRARIES = list of dynamic lib files with full path (not system/installed libs), but never build
#
#             System libraries are things supplied by the compiler, or system
#             that need to be explicitly linked, like "X11" for Linux X windows
#             programs or pthreads for a threading library
#
# OTHER_DIRS  = list of directories that should be made under this make, but
#               with no implicit/default build type or result.  directories
#               should contain a makefile that does NOT include this template
#               Example: a makefile that copies web pages to the output directory
#
# TOOLS		  = list of tools that need to be built to build this subsystem, specified as the dir to build them from
# PREBUILT_TOOLS = list of tools that are needed, and expected to be present (not usually useful except in tarring)
# RELEASE_SOURCES= list of source files which belong in release tars OR
# RELEASE_EXCLUDE= list of source files to *remove* from SOURCES to avoid putting them in release tar
#
# SUPERSOURCE = define this to build objects by concatenating ALL of the sources of each type (C,C++)
#               into one source file and giving that to the compiler.  Works only for libraries, and
#               is really a simple hack of replacing the OBJECTS variable handed to the archiver, and
#               working backwards from there by dependency to maintain the super-source file
#               super-sourcing can increase build performance by *multiple* orders of magnitude,
#               (e.g. go from over 100 seconds to 1 second) especially if the compiler reaches out
#               to a license manager each compile.
#
# V           = this is a built-in make var, if set to 1 will show commands being executed even when hid with "@"
# VERBOSE     = set to 1 to give more feedback on building
#
# You can also add to the default C/CPP/ASM command line defines and include path using
# EXTRA_DEFINES and EXTRA_INCLUDES, or override the default ones by defining DEFINES and/or INCLUDES
#
# The general idea is to create a working directory (anywhere in your file system) and in that
# directory you make a makefile which exports the "root" of your source tree and (optionally)
# any defines, include paths, etc. and then recursively makes a subdirectory in that source
# tree using make -C <subsystem>, or, specifies an application or tool using a line like
# export APPLICATION=<subsystem> and including this template file, which will cause the make -C
#
# Each sub system in your source tree that is buildable needs to contain a makefile which
# creates the required target for that sub system.  Typically, the subsystem is an archive
# and the make, in building the subsytem, expects the result to be "subsystem.a" or
# "subsystem.lib", etc.  depending on the appropriate extension for archives (see flags.mak)
# For most subsystems, a simple makefile which specifies the BUILD_TYPE, the SOURCES, and includes
# this template.mak file is sufficient, but any makefile will work if it produces the proper
# target file.
#
# The working directory is where the end result of the build gets placed.
# Generated dependenancies and object files get placed in somewhat hidden subdirectories
# ".dep" and ".obj" under the work directory, where there is a .dep<targ> and .obj<targ>
# for each cpu architecture involved in the build (at least one, and two if cross compiling)
#
# All actual building steps are performed IN the working directory.  This means that
# compiles, links, or any other command performed in any makefile is done with the
# working directory as the "current" working directory.  The WORKDIR or OUTDIR variable
# can be used to reference this working dir in subsystem make files, and SRCDIR  will
# always be set to the directory of the subsystem being built
#
# Since building happens recursivly (typically), you will probably want to export things
# like EXTRA_DEFINES that you would want on the compile/assemble lines of all subsystem makes.
#
# To post-process the result of a make, you can create a new default target in your working
# directory makefile that depends on "All" and performs the post processing, assuming you
# know the file result of the sub-make
#

# The $(BSA_TOP_LEVEL) variable can be tested against $(MAKELEVEL)
# to determine whether or not this is a nested call to make.  Like MAKELEVEL,
# this variable should never be set explicitly in any subordinate makefiles,
# nor should it be set in the environment from which the top-level make
# is invoked.
#
export BSA_TOP_LEVEL ?= ${MAKELEVEL}

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
    ISDOING=Making
    CLEANGOAL=
else
    ISDOING=Cleaning
    CLEANGOAL=clean
endif

#--------------------------------------------------------------------
# Defaults -> HostxHost, Debug, HostExecutable build
#
ifndef HOST_ARCH
	ifeq ($(OS),Windows_NT)
		ifeq (64,$(findstring 64,$(PROCESSOR_ARCHITECTURE)))
			HOST_ARCH = x86_64
		else
			HOST_ARCH = x86_32
		endif
	else
		export OSTYPE=$(shell set | grep OSTYPE)
		export HOSTTYPE=$(shell set | grep HOSTTYPE)
		ifeq (darwin,$(findstring darwin,$(OSTYPE)))
			ifeq (x86,$(findstring x86,$(HOSTTYPE)))
				ifneq (,$(findstring x86_64,$(HOSTTYPE)))
					HOST_ARCH = x86_64
				else
					HOST_ARCH = x86_32
				endif
			else
				HOST_ARCH = PPC
			endif
		else
			ifneq (,$(findstring x86_64,$(shell uname -a)))
				HOST_ARCH = x86_64
			else
				HOST_ARCH = x86_32
			endif
		endif
	endif
	export HOST_ARCH
endif

ifndef ROOT
#   - this makes ROOT the directory ABOVE the current directory
#   - which is why it is needed to have the working directory one level down
#   - from the root if ROOT is not explicitly listed in the top makefile
#
	export ROOT	:=	$(dir $(CURDIR))
#
else
#	once a root is specified, it is used for rest of makes
#
	export ROOT
endif

export WORKDIR ?= .

ifndef HOST_OS
	ifeq ($(OS),Windows_NT)
		HOST_OS	= Windows_NT
	else
		ifeq (darwin,$(findstring darwin,$(OSTYPE)))
			ifeq (x86,$(findstring x86,$(HOSTTYPE)))
				HOST_OS	= OSX
			else
				HOST_OS	= Darwin
			endif
		else
			HOST_OS = Linux
		endif
	endif
	export HOST_OS
endif
ifndef TARGET_ARCH
	TARGET_ARCH = $(HOST_ARCH)
	export TARGET_ARCH
endif
ifndef TARGET_REV
	TARGET_REV		=
	export TARGET_REV
endif
ifndef TARGET_OS
	TARGET_OS		=	$(HOST_OS)
	export TARGET_OS
endif
ifndef BUILD_TYPE
ifeq ($(MAKELEVEL),$(BSA_TOP_LEVEL))
	ifeq ($(TARGET_OS)$(TARGET_ARCH),$(HOST_OS)$(HOST_ARCH))
		BUILD_TYPE		=	HostExecutable
	else
		BUILD_TYPE		=	Executable
	endif
else
	BUILD_TYPE		=	Library
endif
endif

BUILD_DIR		?=	.
BUILD_DEBUG		?=	Release
DEFINES			?=
UI				?=	uistub

V ?= 0

ifndef NODIRMSG
#  - this shuts up alot of noise from gnumake about
#  - directory recursion
ifeq ($V,1)
	NODIRMSG		=	--no-builtin-rules --no-print-directory
else
	NODIRMSG		=	-s --no-builtin-rules --no-print-directory
endif
endif

export BUILD_DEBUG

# define HOST_EXEEXT to be the extension of an program
# that runs on the host (build) machine
#
ifeq ($(HOST_OS),Windows_NT)
HOST_EXEEXT=.exe
else
HOST_EXEEXT=
endif

# The following things SHOULD be defined in the
# hostXtarget specific template
#
# TARGET_OS	- operating system on target
#
# CC		- C compiler
# CPP		- C++ compiler
# ASM		- Assembler
#
# CFLAGS	- flags to c compiler
# CPPFLAGS	- flags to c++ compiler
# ASMFLAGS	- flags to assembler
#
# AR		- archiver
# ARFLAGS	- flags to archiver
#
# LD		- linker
# LDFLAGS	- flags to linker
#
# CINCLS	- formatted includes for c/c++ (e.g. -I../incl)
# CDEFS		- formatted defines for c/c++ (e.g. -DQ4230 -DTHREADX)
#
# AINCLS	- same, for asm
# ADEFS
#
#
# OBJPREF	- flag to indicate output object file (e.g. -o (and make sure to include embedded space))
# ARPREF	- flag to indicate archiver output
# LDPREF	- flag to indicate linker output (e.g. -o )
# LDPATHPREF- flag to indicate library search path (e.g -L)
# LDOBJPREF - prefix directive to indicate object follows (usually blank)
# LDSHLIBPREF-flag to indicated shared library in link (e.g. -l)
# LDSHLIBSUFX-shared library suffix on link line (usually blank)
# LDLIBPREF	- prefix for static libraries in link line ($(OUTDIR)/$(LIBPREF))
# LDLIBSUFX	- suffix for static lib on link line ($(LIBEXT))
#
# OBJEXT	- object file extension (e.g. .o)
# LIBEXT	- static library extension (e.g. .a)
# SHLIBEXT	- shared library extension (e.g. .so)
# EXEEXT	- executable extension (e.g. .exe)
# LIBPREF	- assumbed prefix for library file (e.g. lib)
#

s? = $(subst $(empty) ,?,$1)
?s = $(subst ?, ,$1)
notdirx = $(call ?s,$(notdir $(call s?,$1)))

#---------------------------------------------------------------------
# Directories
#
ifeq ($(BUILD_TYPE), SharedLibrary)
	TARGETINT = $(TARGET_ARCH)
else
ifeq ($(BUILD_TYPE), Library)
	TARGETINT   = $(TARGET_ARCH)
else
ifeq ($(BUILD_TYPE), HostExecutable)
	TARGETINT   = $(HOST_ARCH)
else
	TARGETINT   = $(TARGET_ARCH)
endif
endif
endif

# if SRCDIR isn't defined, make it the current directory
# but use a relative path so things like drive letters aren't a problem
#
ifndef SRCDIR
	SRCDIR		=	.
endif

# if BUILD isnt defined (it usually isnt) then set it to the
# name of the directory suppling the source, sans path, with
# special case when srcdir is .
#
ifndef BUILD
ifeq (x$(SRCDIR),x.)
	BUILD			=	$(call notdirx,$(CURDIR))
else
	BUILD			=	$(call notdirx,$(SRCDIR))
endif
endif

# Results Directory (this directory)
#
OUTBASEDIR	=	$(WORKDIR)
OUTDIR		=	$(OUTBASEDIR)
export	OUTDIR

# Intermediate (Objects/Libraries) files dir
#
INTBASEDIR	=	$(WORKDIR)
INTDIR		=	$(INTBASEDIR)/.obj$(TARGETINT)
export	INTDIR

# Intermediate (Dependency) files dir
#
DEPBASEDIR	=	$(WORKDIR)
DEPDIR		=	$(DEPBASEDIR)/.dep$(TARGETINT)
export	DEPDIR

# Intermediate (Dependency) files dir for tools, which are always host exes
#
TOOLBASEDIR	=	$(WORKDIR)
TOOLDEPDIR	=	$(TOOLBASEDIR)/.dep$(HOST_ARCH)
export	DEPDIR

# Documentation (generated html) files dir
#
DOCBASEDIR	=	$(WORKDIR)
DOCDIR		=	$(INTBASEDIR)/docHTML
export	DOCDIR

ifeq ($(HOST_OS),Windows_NT)
	XINTDIR	= $(subst /,\,$(INTDIR))
	XOUTDIR	= $(subst /,\,$(OUTDIR))
	XDEPDIR	= $(subst /,\,$(DEPDIR))
else
	XINTDIR	= $(INTDIR)
	XOUTDIR	= $(OUTDIR)
	XDEPDIR	= $(DEPDIR)
endif

# Include proper file for defining compiler/etc.  This was delayed till
# here so that proper directory manifests are setup (like OUTDIR) etc.
#
#---------------------------------------------------------------------
# Form the host dependent make include path
#
ifneq ($(BUILD_TYPE), HostExecutable)
#    -- regular make, include hostxtarget flags
	ifeq ($(HOST_OS)$(HOST_ARCH),$(TARGET_OS)$(TARGET_ARCH))
		BUILD_INCL	= $(ROOT)/build/$(HOST_OS)$(HOST_ARCH)native
	else
		BUILD_INCL	= $(ROOT)/build/$(HOST_OS)$(HOST_ARCH)cross$(TARGET_OS)$(TARGET_ARCH)
	endif
else
#	 -- tool or doc build, include hostxhost flags
	BUILD_INCL	= $(ROOT)/build/$(HOST_OS)$(HOST_ARCH)native
endif
BUILD_FLAGS_FILE = $(BUILD_INCL)/flags.mak
include	$(BUILD_FLAGS_FILE)

# Define HOST_BASED make manifest if it isn't set
#
ifndef HOST_BASED
	HOST_BASED = 0
	export HOST_BASED
endif

ifndef BOARD
	export BOARD=$(TARGET_ARCH)
endif

ifeq (,$(findstring $(TARGET_ARCH),$(DEFINES)))
	DEFINES		+=  $(TARGET_ARCH) Q$(BOARD) OS_$(TARGET_OS)
	ifneq ($(TARGET_ARCH),$(TARGET_OS))
		DEFINES		+=$(TARGET_OS)
	endif
endif

ifndef TARGET_OS
	ifneq ($(HOST_OS)$(HOST_ARCH),$(TARGET_OS)$(TARGET_ARCH))
#		-- cross building and no target os specified
#       -- is a build error, cause there is no way to guess
		fail
	else
		TARGET_OS		=	$(HOST_OS)
	endif
endif
ifneq (,$(TARGET_REV))
	ifneq (,$(filter 4% 5%,$(TARGET_ARCH)))
    	DEFINES += $(TARGET_ARCH)$(TARGET_REV)
	endif
endif

# add extra defines to defines, but be careful about not
# adding anything already in.  These recursive builds
# can make command lines long enough to kill some tools (like gnu and windows)
#
CURDEFS:=$(DEFINES)
UNIQDEFS:=$(foreach def,$(EXTRA_DEFINES),$(if $(findstring $(CURDEFS),$(def)),,$(def)))
DEFINES+=$(UNIQDEFS)

#ifeq (,$(findstring $(EXTRA_DEFINES),$(DEFINES)))
#	export DEFINES	+=	$(EXTRA_DEFINES)
#endif

export STD_INCLS ?=	. $(ROOT)/incl $(ROOT)/include $(ROOT) $(OUTDIR)

# Note that INCLUDES do not accumulate on multiple nested inclusions, but
# EXTRA_INCLUDES do, so use that to build nested addative things.  Also,
# for proper building of full trees which include host-based tool builds,
# make sure to put EXTRA_INCLUDES defines in the (target)/flags.mak file
# instead of a top-level makefile
#
INCLUDES	:= $(SRCDIR) $(STD_INCLS) $(EXTRA_INCLUDES)
export INCLUDES

#---------------------------------------------------------------------
# GNU tool chain - needed for dependency generation maybe even
#                  if not being used to compile
GNUCPP		=	g++
GNUCC		=	gcc
GNUAR		=	ar
GNULD		=	g++

ifndef CDEPFLAGS
	CDEPFLAGS	= -MM -MG -MP
endif
ifndef CDEPPP
	ifndef DEPGEN
		export DEPGEN	= $(CURDIR)/depgen
	endif
	CDEPPP		= $(DEPGEN)
endif

ifndef ADEPFLAGS
	ADEPFLAGS	= -MM -MG -dINCLUDE
endif
ifndef ADEPPP
	ifndef DEPGEN
		export DEPGEN	= $(CURDIR)/depgen
	endif
	ADEPPP		= $(DEPGEN)
endif

#---------------------------------------------------------------------
# generic cmd-line stuff which can be provided by unixutils/cygwin for Windows
# note that printf is used for echo in Windows since the window's builtin echo
# doesn't understand "-n"
#
TAR			=	tar
GZIP		=	gzip

ifeq ($(HOST_OS),Windows_NT)
	MKDIR		=	mkdir
	MKDIRFLAGS	=
	ECHON		=	printf
	ECHOE		=	echo -e
	RM			=	rm
	RMFLAGS		=	-f
else
	MKDIR		=	mkdir
	MKDIRFLAGS	=	-p
ifeq ($(HOST_OS),OSX)
	ECHON		=	printf
	ECHOE		=	printf
else
	ECHON		=	/bin/echo -n
	ECHOE		=	/bin/echo -e
endif
	RM			=	rm
	RMFLAGS		=	-f
endif

ifeq ($(HOST_OS),Windows_NT)
	HOSTEXEEXT=.exe
else
	HOSTEXEEXT=
endif

ifndef VERBOSE
ifeq ($(V),1)
    VERBOSE=1
else
    VERBOSE=0
endif
endif

#---------------------------------------------------------------------
# Include File Path
#
CDEPIPATH		:=	$(INCLUDES:%=-I % )
ADEPIPATH		:=	$(INCLUDES:%=-I % )

#---------------------------------------------------------------------
# DEFINES
#
CDEPDEFS		:=	$(CDEFS)
ADEPDEFS		:=	$(ADEFS)

#---------------------------------------------------------------------
# Link editor
#
LDPATHS	=	$(LDPATHEXTRA)
LDPATHS +=	$(CURDIR)
LDPATH :=	$(LDPATHS:%=$(LDPATHPREF)%)

#---------------------------------------------------------------------
# Actual list of sources from sources
#
ifeq ($(strip $(SOURCES)), All)
	AASOURCES= $(wildcard $(SRCDIR)/*.cpp) $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/*.m) $(wildcard $(SRCDIR)/*.s) $(wildcard $(SRCDIR)/*.rc)
	SOURCES= $(subst $(SRCDIR)/,,$(AASOURCES))
endif

# list of sources with absolute source path prepended
#
APALLSOURCES:= $(SOURCES:%=$(SRCDIR)/%)
ALLSOURCES	:= $(subst $(ROOT)/,,$(APALLSOURCES))

# list of sources for release tars
#
ifndef RELEASE_SOURCES
	RELEASE_SOURCES	:= $(ALLSOURCES)
endif
ifdef RELEASE_EXCLUDE
	RELEASE_SOURCES := $(filter-out $(RELEASE_EXCLUDE), $(RELEASE_SOURCES))
endif

# These are the sources that need to be compiled
#
CPPSOURCES	= $(filter %.cpp, $(SOURCES))
MSOURCES	= $(filter %.m,   $(SOURCES))
CSOURCES	= $(filter %.c,   $(SOURCES))
ASSOURCES	= $(filter %.s,   $(SOURCES))
RSOURCES	= $(filter %.rc,  $(SOURCES))

# these are the same source lists, prepended by the actual path
#
CPPSRCS		:= $(CPPSOURCES:%.cpp=$(SRCDIR)/%.cpp)
MSRCS		:= $(CPPSOURCES:%.m=$(SRCDIR)/%.m)
CSRCS		:= $(CSOURCES:%.c=$(SRCDIR)/%.c)
ASSRCS		:= $(ASSOURCES:%.s=$(SRCDIR)/%.s)
RSRCS		:= $(RSOURCES:%.rc=$(SRCDIR)/%.rc)

# this is the complete list of makeable source files, with paths
#
SRCS		= $(CPPSRCS) $(MSRCS) $(CSRCS) $(ASSRCS) $(RSRCS)

# this is the list of sources used to actually compile, which is
# the same as the source list SRCS unless super-sourcing
#
ifdef SUPERSOURCE
ifneq (,$(CPPSRCS))
	ACTCPPSRCS	=	$(INTDIR)/$(BUILD)_sspp.cpp
else
	ACTCPPSRCS	=
endif
ifneq (,$(MSRCS))
	ACTCMRCS	=	$(INTDIR)/$(BUILD)_ssm.m
else
	ACTMSRCS	=
endif
ifneq (,$(CSRCS))
	ACTCSRCS	=	$(INTDIR)/$(BUILD)_ss.c
else
	ACTCSRCS	=
endif
else
	ACTCPPSRCS	=	$(CPPSRCS)
	ACTMSRCS	=	$(MSRCS)
	ACTCSRCS	=	$(CSRCS)
endif

ACTSRCS	= $(ACTCPPSRCS) $(ACTMSRCS) $(ACTCSRCS) $(ASSRCS) $(RSRCS)

#---------------------------------------------------------------------
# Applications
#
# These are applications specified in the makefile, basically as an indirection
#
APPNAMES	= $(notdir $(APPLICATIONS))
APPEXECS	= $(APPNAMES:%=$(OUTDIR)/%$(EXEEXT))
APPDEPNAMES = $(subst /,-,$(APPLICATIONS))
APPDEPENDS	= $(APPDEPNAMES:%=$(DEPDIR)/%.appmake)

#---------------------------------------------------------------------
# Tools
#
# These are the tools that need to be built in order to build the subsystem
# (and as dependency of).  To be completely general, just make a dependency
# file to make each tool recursively
#
TOOLNAMES	= $(notdir $(TOOLS))
TOOLEXECS	= $(TOOLNAMES:%=$(OUTDIR)/%$(HOST_EXEEXT))
TOOLDEPNAMES= $(subst /,-,$(TOOLS))
TOOLDEPENDS	= $(TOOLDEPNAMES:%=$(TOOLDEPDIR)/%.toolmake)

#---------------------------------------------------------------------
# Libraries
#

# From libraries enumerated, create list of shareds (.so./dll) and libs (.a/.lib)
#
# first, if forcing static link, move shared libs list into static libs list
# and build them as static libraries
#
LIBRARIES	+= $(STATIC_LIBRARIES)
ifdef STATIC_LINK
	LIBRARIES		+=	SHARED_LIBRARIES
	override SHARED_LIBRARIES		=
endif

BASESTATIC_LIBS	:= $(notdir $(LIBRARIES))
FIXEDLIBS   	:= $(BASESTATIC_LIBS:%=%$(LIBEXT))

BASESHARED_LIBS:= $(notdir $(SHARED_LIBRARIES))
SHAREDLIBS		:= $(BASESHARED_LIBS:%=%$(SHLIBEXT))

SUBMAKE_DEPS	:= $(OTHER_DIRS)

# this is the same lists, with each lib prepended by proper location
# and prepeneded by any system library prefix (like "lib" for unix)
#
SHLIBS			:= $(SHAREDLIBS:%$(SHLIBEXT)=$(INTDIR)/$(LIBPREF)%$(SHLIBEXT))
LIBS			:= $(FIXEDLIBS:%$(LIBEXT)=$(INTDIR)/$(LIBPREF)%$(LIBEXT))

SHLIBDEPNAMES := $(subst /,-,$(SHARED_LIBRARIES))
SHLIBDEPENDS  := $(SHLIBDEPNAMES:%=$(DEPDIR)/%.shlibmake)

LIBDEPNAMES := $(subst /,-,$(LIBRARIES))
LIBDEPENDS  := $(LIBDEPNAMES:%=$(DEPDIR)/%.libmake)

# this is the list of libraries, no "lib" prefix for use in the linker
# command itself, each item has the "Linker Library Directive" prepended
# (-l, no space, for unix. library, with space, for QNX)
# it is assumed that shared libraries are linked with no pref or ext, and
# fixed libraries are completely spelled out (-lpthread for libpthread.so)
# and (libpthread.a for that)
#

# first, add the abs. path of actually built libraries with no prefix
#
LINKEDLIBS		:= $(FIXEDLIBS:%$(LIBEXT)=$(INTDIR)/%$(LDLIBSUFX))

# dont add prebuilt and system libraries if building tools, or non
# target executables
#
ifeq ($(BUILD_TYPE),Executable)
LINKEDSHLIBS	+= $(PREBUILT_SHARED_LIBRARIES)
LINKEDSHLIBS	+= $(SYSTEM_SHARED_LIBRARIES:%=$(LDSHLIBPREF)%$(LDSHLIBSUFX))
LINKEDLIBS 		+= $(SYSTEM_LIBRARIES:%=%$(LDLIBSUFX))
LINKEDLIBS		+= $(SYSTEM_FRAMEWORKS:%=-framework %)
endif

ifeq ($(BUILD_TYPE),SharedLibrary)
LINKEDSHLIBS	+= $(SYSTEM_SHARED_LIBRARIES:%=$(LDSHLIBPREF)%$(LDSHLIBSUFX))
LINKEDLIBS 		+= $(SYSTEM_LIBRARIES:%=%$(LDLIBSUFX))
endif

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
ifdef NORECURSIVE_ARCHIVES
# this makes a list of objects inside a library used for recursive library
# building for archivers (like GNU) that cant handle recursiveness
#
define listobjectsinlib
$(shell $(AR) -t $(lib))

endef

# this extracts objects from a prebuilt library used for recursive library
# building for archivers (like GNU) that cant handle recursiveness
# NOTE - the ../ means all prebuilt libs need to have a realtive path!! that
# is probably a mistake.  TODO - fix this somehow?
#
define extractobjectsinlib
$(shell cd $(INTDIR) && $(AR) -x ../$(lib))

endef

endif
endif

#---------------------------------------------------------------------
# Objects (from supplied sources)
#

# real objects in proper directory
#
ifdef SUPERSOURCE
	FCPPSOURCES	= $(notdir $(ACTCPPSRCS))
	FMSOURCES	= $(notdir $(ACTMSRCS))
	FCSOURCES	= $(notdir $(ACTCSRCS))
else
	FCPPSOURCES	= $(notdir $(CPPSOURCES))
	FMSOURCES	= $(notdir $(MSOURCES))
	FCSOURCES	= $(notdir $(CSOURCES))
endif

FASSOURCES	= $(notdir $(ASSOURCES))
FRSOURCES	= $(notdir $(RSOURCES))

CPPOBJECTS	:= $(FCPPSOURCES:%.cpp=$(INTDIR)/$(BUILD)_%$(OBJEXT))
MOBJECTS	:= $(FMSOURCES:%.m=$(INTDIR)/$(BUILD)_%$(OBJEXT))
COBJECTS	:= $(FCSOURCES:%.c=$(INTDIR)/$(BUILD)_%$(OBJEXT))
ASOBJECTS	:= $(FASSOURCES:%.s=$(INTDIR)/$(BUILD)_%$(OBJEXT))
#RESOURCE	:= $(FRSOURCES:%.rc=$(INTDIR)/$(BUILD)_%.o)
RESOURCE	:= $(FRSOURCES:%.rc=$(INTDIR)/$(BUILD)_%.res)

OBJECTS		+= $(CPPOBJECTS) $(MOBJECTS) $(COBJECTS) $(ASOBJECTS) $(RESOURCE)

ifeq ($(HOST_OS),Windows_NT)
	LINKEDRESOURCE	=
else
	LINKEDRESOURCE	=
endif

#---------------------------------------------------------------------
# Dependency
#
# these are the files that are created by dependency generation
#
DEPENDS		:= $(FCPPSOURCES:%.cpp=$(DEPDIR)/$(BUILD)_%.d)
DEPENDS		+= $(FMSOURCES:%.m=$(DEPDIR)/$(BUILD)_%.d)
DEPENDS		+= $(FCSOURCES:%.c=$(DEPDIR)/$(BUILD)_%.d)
DEPENDS		+= $(FASSOURCES:%.s=$(DEPDIR)/$(BUILD)_%.d)
DEPENDS		+= $(FRSOURCES:%.rc=$(DEPDIR)/$(BUILD)_%.d)

#---------------------------------------------------------------------
#
# Main Target name generation
#
#
ifeq ($(BUILD_TYPE), SharedLibrary)
	#
	# making a library (shared)
	# like "libNAME.so"
	#
	BUILD_EXT	=	$(SHLIBEXT)
	BUILD_PREF	=	$(LIBPREF)
	ACTUAL_TARGET = $(INTDIR)/$(BUILD_PREF)$(BUILD)$(BUILD_EXT)
else
ifeq ($(BUILD_TYPE), Library)
	#
	# making a library to link
	# like "libNAME.a"
	#
	BUILD_EXT	=	$(LIBEXT)
	BUILD_PREF	=	$(LIBPREF)
	ACTUAL_TARGET = $(INTDIR)/$(BUILD_PREF)$(BUILD)$(BUILD_EXT)
else
ifeq ($(BUILD_TYPE), None)
	#
	# a dummy make, just to make something else
	#
	ACTUAL_TARGET =
else
	#
	# making an executable or host executable
	#
	# if there are no sources, assume this is a blank/stub
	# makefile used to sub-make libs or apps and just start
	# with tools as the top level target
	#
	BUILD_EXT	=	$(EXEEXT)
	BUILD_PREF	=
	ifeq (,$(SOURCES))
	    ACTUAL_TARGET =
    else
        ACTUAL_TARGET = $(OUTDIR)/$(BUILD_PREF)$(BUILD)$(BUILD_EXT)
    endif
endif
endif
endif

# The flags target is used to make sure things get rebuilt if the build flags change
#
BUILDFLAGS = 	$(DEPDIR)/$(BUILD).cppflags 	\
				$(DEPDIR)/$(BUILD).cflags 		\
				$(DEPDIR)/$(BUILD).asmflags

# ##########################################################################3
#
# -------- ALL -----------
#
# This is the actual target for make, which depends on all the other stuff
#
# The order of dependencies is first to last
#  - the directories under the working directory where things go
#  - the tools needed to build sub parts (specify directories of tools that should be made)
#  - anything that is generated (usually by tools) to use as source (usually a header file)
#  - the applications that are to be built (specify a directory that should be made)
#  - for builds with sources, the file which includes all the dependencies, then the dependencies
#  - lists of static and shared libs that need to be built
#  - the actual thing to build (exe or library or shared library)
#  - anything that needs to be built after the final exe (i.e. a stripped version, or hex formatted, etc.)
#
# Since gnu make can and will parallelize sub-makes for dependencies, the order has to be enforced
# by cascading the dependencies.  This isnt currently handled, since it depends on make >= 3.79
# and this should be re-coded by having each thing depend on the previous thing
#
ifneq (,$(PROCESSED_TARGET))
FINAL_TARGET = $(PROCESSED_TARGET)
else
FINAL_TARGET = $(ACTUAL_TARGET)
endif

# This builds whatever the final target of the build is and any applications
# specified in APPLICATIONS.  Note that this is conceptually two completely
# independant operations: the applications don't depend on the target and the
# final target doesn't depend on application, and FINAL_TARGET could be blank
# and there could be no applications to build.
#
All:  $(FINAL_TARGET) $(APPEXECS) $(SUBMAKE_DEPS)
ifeq (1,$(VERBOSE))
	@ echo Built $(FINAL_TARGET) $(APPEXECS)
endif

ifneq (,$(PROCESSED_TARGET))
# there is a processed target, so make that depend on actual
#
PROCESSED_TARGET: $(ACTUAL_TARGET)
endif

# if there's anything to generate, make that thing(s) depend on tools
# so that tools get built *before* anything needs generation, assuming
# the tool is needed to do the generation.
# if nothing to generate, make generate the tools (if any) so they get made
#
# but all this is not done if cleaning, since that is too hard to do
# in a chained fashion, the clean target below cleans the tools by hand
#
ifeq (,$(findstring clean,$(MAKECMDGOALS)))
    ifneq (,$(GENERATE))
        $(GENERATE): $(TOOLEXECS)
    else
        GENERATE = $(TOOLEXECS)
    endif
endif

# This is the actual target dependency list.  It shouldn't matter which
# order these dependencies are evaluated, or even simultaneously as
# they don't depend on each other (note that assumes you don't set
# submake_deps to something that depends on the other things)
#
$(ACTUAL_TARGET):  $(RESOURCE) $(OBJECTS) $(LIBS) $(SHLIBS)

$(SUBMAKE_DEPS):
ifeq ($(V),1)
	@ echo $(ISDOING) Build Dependency $@
endif
	@+ $(MAKE) $(NODIRMSG) -f $(ROOT)/$@/makefile SRCDIR=$(ROOT)/$@

ifeq ($(BUILD_TYPE), SharedLibrary)

# --- rule how to make THE shared library TARGET from objects
#
$(ACTUAL_TARGET):
	$(SHLD) $(SHLDFLAGS) $(LDPREF)$@ $(OBJECTS) $(LINKEDSHLIBS) $(LINKEDLIBS)

endif

ifeq ($(BUILD_TYPE), Library)

MAX_OBJS_PER_LINE = 40

ifneq ($(HOST_OS),Windows_NT)
	DUMPSPAM=	> /dev/null 2>&1
else
	DUMPSPAM=
endif

# Windows cant handle lines longer than 8K characters.
# Unfortunately, the lists of object files for some directories can stretch
# beyond that limit.
#
# To stay out of trouble, write all objects and ar commands to a file and
# use the file as command source for AR
#
ifeq ($(HOST_OS),Windows_NT)
define rearchive
echo $(patsubst %,$(INTDIR)/%,$(shell $(AR) -t $(lib))) >> $@.obl

endef
else
define rearchive
$(AR) $(ARFLAGS) $(ARPREF)$@ $(patsubst %,$(INTDIR)/%,$(filter %.o, $(shell $(AR) -t $(lib))))

endef
endif

# --- rule how to make THE library TARGET from objects.  Note the "@" after (AR)
#     is a flag to AR to indicate "read commands from file" and not a special "make" symbol
#
$(ACTUAL_TARGET):
ifneq (x$(strip $(OBJECTS)), x)
	@ echo creating archive $(call notdirx,$@)
ifeq ($(HOST_OS),Windows_NT)
	@ rm -f $@ $@.obl
	@ echo $(ARFLAGS) $(ARPREF)$@ $(OBJECTS) > $@.obl
	$(AR) @$@.obl
else
	$(AR) $(ARFLAGS) $(ARPREF)$@ $(OBJECTS)
endif

ifdef NORECURSIVE_ARCHIVES
# some (gnu) archivers can NOT deal with archives as input so extract all the objects
# out of each linked library and archive those objects, again in groups to avoid breaking line len
#
# for any pre-built libs on the link line, need to extract the objects from them first since
# they were never put in INTDIR
#
ifneq (x$(strip $(LINKEDPREBUILT_LIBS)), x)
	$(foreach lib,$(LINKEDPREBUILT_LIBS),$(shell cd $(INTDIR) && $(AR) -x ../$(lib)))
endif
ifneq (x$(strip $(LINKEDLIBS)), x)
ifeq ($(HOST_OS),Windows_NT)
	@ rm -f $@.obl
	@ echo $(ARFLAGS) $(ARPREF)$@ > $@.obl
	$(foreach lib,$(LINKEDLIBS),$(call rearchive))
	$(AR) @$@.obl
else
	$(foreach lib,$(LINKEDLIBS),$(call rearchive))
endif
endif
else
ifneq (x$(strip $(LINKEDLIBS)), x)
	$(AR) $(ARFLAGS) $(ARPREF)$@ $(LINKEDLIBS)
endif
endif
endif
endif

ifeq ($(BUILD_TYPE), Executable)
ifneq (x$(strip $(OBJECTS)), x)

#
# rule how to make THE executable from objects and libs
#
$(ACTUAL_TARGET):  $(SCATTERFILE) $(LINKDEP)
	@ echo link  $(call notdirx,$@)
	$(LD) $(LDFLAGS) $(EXTRA_LDFLAGS) $(LDPREF)$@ $(OBJECTS) $(LINKEDLIBS) $(LINKEDSHLIBS) $(LDSTDLIBS) $(LDFLAGSF) $(LINKEDRESOURCE)
ifdef STRIP
#   optional (see flags.mak) step to strip symbol info from exe to make it smaller
#	or step to convert executable into hex file, etc.
	$(STRIP) $(ACTUAL_TARGET)
endif
endif
endif

ifeq ($(BUILD_TYPE), HostExecutable)
ifneq (x$(strip $(OBJECTS)), x)

#
# rule how to make THE tool program from objects and libs# which is pretty much same as target executable,
# but hopefully a different flags.mak was included (HostxHost not HostxTarget) and I use the compiler
# not the link editor to link it all so it includes the C run-time, etc.
#
ifeq (x$(CPPSOURCES), x)
    CLINK	=	"$(CC)"
else
    CLINK	=	"$(CPP)"
endif

$(ACTUAL_TARGET):
	@ echo link-for-host $(call notdirx,$@)
	$(CLINK) $(LDFLAGS) $(EXTRA_LDFLAGS) $(LDPREF)$@ $(OBJECTS) $(LINKEDLIBS) $(LINKEDSHLIBS) $(LDSTDLIBS) $(LDFLAGSF) $(LINKEDRESOURCE)
ifdef STRIP
#   optional (see flags.mak) step to strip symbol info from exe to make is smaller
	$(STRIP) $(ACTUAL_TARGET)
endif
endif
endif

#---------------------------------------------------------------------
# Dependencies
#
# make dependencies if file doesn't exist
#

# how to make a super source source file
#
$(INTDIR)/$(BUILD)_sspp.cpp: $(CPPSRCS)
	cat $+ > $@

$(INTDIR)/$(BUILD)_ss.c: $(CSRCS)
	cat $+ > $@

ifndef NO_GCC
.DELETE_ON_ERROR:
endif

# if gcc is the depends file generator, it makes one line with all the deps
# and gcc doesnt handle "-o" well at all so I build up the dep file

ifeq (gcc,$(findstring gcc,"$(CDEPPP)"))

# special rule for depgen_depgen.d, since that would be chicken-egg
$(DEPDIR)/$(BUILD)_depgen.d: $(SRCDIR)/depgen.c $(DEPDIR)
	@ echo $(INTDIR)/$(BUILD)_$(basename $(call notdirx,$<))$(OBJEXT): $< > $@

$(DEPDIR)/$(BUILD)_%.d: $(SRCDIR)/%.cpp
	@ $(ECHON) $(INTDIR)/$(BUILD)_ > $@
	"$(CDEPPP)" $(CDEPFLAGS) $(CDEPIPATH) $(CDEPDEFS) $< >> $@

$(DEPDIR)/$(BUILD)_%.d: $(INTDIR)/%.cpp
	@ $(ECHON) $(INTDIR)/$(BUILD)_ > $@
	"$(CDEPPP)" $(CDEPFLAGS) $(CDEPIPATH) $(CDEPDEFS) $< >> $@

$(DEPDIR)/$(BUILD)_$(BUILD)_sspp.d: $(INTDIR)/$(BUILD)_sspp.cpp
	@ $(ECHON) $(INTDIR)/ > $@
	"$(CDEPPP)" $(CDEPFLAGS) $(CDEPIPATH) $(CDEPDEFS) $< >> $@


$(DEPDIR)/$(BUILD)_%.d: $(SRCDIR)/%.m
	@ $(ECHON) $(INTDIR)/$(BUILD)_ > $@
	"$(CDEPPP)" $(CDEPFLAGS) $(CDEPIPATH) $(CDEPDEFS) $< >> $@

$(DEPDIR)/$(BUILD)_%.d: $(INTDIR)/%.m
	@ $(ECHON) $(INTDIR)/$(BUILD)_ > $@
	"$(CDEPPP)" $(CDEPFLAGS) $(CDEPIPATH) $(CDEPDEFS) $< >> $@

$(DEPDIR)/$(BUILD)_$(BUILD)_ssm.d: $(INTDIR)/$(BUILD)_ssm.m
	@ $(ECHON) $(INTDIR)/ > $@
	"$(CDEPPP)" $(CDEPFLAGS) $(CDEPIPATH) $(CDEPDEFS) $< >> $@


$(DEPDIR)/$(BUILD)_%.d: $(SRCDIR)/%.c
	@ $(ECHON) $(INTDIR)/$(BUILD)_ > $@
	"$(CDEPPP)" $(CDEPFLAGS) $(CDEPIPATH) $(CDEPDEFS) $< >> $@

$(DEPDIR)/$(BUILD)_%.d: $(INTDIR)/%.c
	@ $(ECHON) $(INTDIR)/$(BUILD)_ > $@
	"$(CDEPPP)" $(CDEPFLAGS) $(CDEPIPATH) $(CDEPDEFS) $< >> $@

$(DEPDIR)/$(BUILD)_$(BUILD)_ss.d: $(INTDIR)/$(BUILD)_ss.c
	@ $(ECHON) $(INTDIR)/ > $@
	"$(CDEPPP)" $(CDEPFLAGS) $(CDEPIPATH) $(CDEPDEFS) $< >> $@

else

# nongcc cc probably puts all the deps on separate lines and can handle -o
#

# special rule for depgen_depgen.d, since that would be chicken-egg
$(DEPDIR)/$(BUILD)_depgen.d: $(SRCDIR)/depgen.c $(DEPDIR)
	@ echo $(INTDIR)/$(BUILD)_$(basename $(call notdirx,$<))$(OBJEXT): $< > $@

$(DEPDIR)/$(BUILD)_%.d: $(SRCDIR)/%.cpp
	"$(CDEPPP)" $(CDEPFLAGS) -o $(INTDIR)/$(BUILD)_$(basename $(call notdirx,$<))$(OBJEXT) $(CDEPIPATH) $(CDEPDEFS) $< > $@

$(DEPDIR)/$(BUILD)_%.d: $(INTDIR)/%.cpp
	"$(CDEPPP)" $(CDEPFLAGS) -o $(INTDIR)/$(BUILD)_$(basename $(call notdirx,$<))$(OBJEXT) $(CDEPIPATH) $(CDEPDEFS) $< > $@

$(DEPDIR)/$(BUILD)_$(BUILD)_sspp.d: $(INTDIR)/$(BUILD)_sspp.cpp
	"$(CDEPPP)" $(CDEPFLAGS) -o $(INTDIR)/$(basename $(call notdirx,$<))$(OBJEXT) $(CDEPIPATH) $(CDEPDEFS) $< > $@


$(DEPDIR)/$(BUILD)_%.d: $(SRCDIR)/%.m
	"$(CDEPPP)" $(CDEPFLAGS) -o $(INTDIR)/$(BUILD)_$(basename $(call notdirx,$<))$(OBJEXT) $(CDEPIPATH) $(CDEPDEFS) $< > $@

$(DEPDIR)/$(BUILD)_%.d: $(INTDIR)/%.m
	"$(CDEPPP)" $(CDEPFLAGS) -o $(INTDIR)/$(BUILD)_$(basename $(call notdirx,$<))$(OBJEXT) $(CDEPIPATH) $(CDEPDEFS) $< > $@

$(DEPDIR)/$(BUILD)_$(BUILD)_ssm.d: $(INTDIR)/$(BUILD)_ssm.m
	"$(CDEPPP)" $(CDEPFLAGS) -o $(INTDIR)/$(basename $(call notdirx,$<))$(OBJEXT) $(CDEPIPATH) $(CDEPDEFS) $< > $@


$(DEPDIR)/$(BUILD)_%.d: $(SRCDIR)/%.c
	"$(CDEPPP)" $(CDEPFLAGS) -o $(INTDIR)/$(BUILD)_$(basename $(call notdirx,$<))$(OBJEXT) $(CDEPIPATH) $(CDEPDEFS) $< > $@

$(DEPDIR)/$(BUILD)_%.d: $(INTDIR)/%.c
	"$(CDEPPP)" $(CDEPFLAGS) -o $(INTDIR)/$(BUILD)_$(basename $(call notdirx,$<))$(OBJEXT) $(CDEPIPATH) $(CDEPDEFS) $< > $@

$(DEPDIR)/$(BUILD)_$(BUILD)_ss.d: $(INTDIR)/$(BUILD)_ss.c
	"$(CDEPPP)" $(CDEPFLAGS) -o $(INTDIR)/$(basename $(call notdirx,$<))$(OBJEXT) $(CDEPIPATH) $(CDEPDEFS) $< > $@


endif

$(DEPDIR)/$(BUILD)_%.d: $(SRCDIR)/%.s
	"$(ADEPPP)" $(ADEPFLAGS) -o $(INTDIR)/$(BUILD)_$(basename $(call notdirx,$<))$(OBJEXT) $(ADEPIPATH) $(ADEPDEFS) $< > $@

$(DEPDIR)/$(BUILD)_%.d: $(INTDIR)/%.s
	"$(ADEPPP)" $(ADEPFLAGS) -o $(INTDIR)/$(BUILD)_$(basename $(call notdirx,$<))$(OBJEXT) $(ADEPIPATH) $(ADEPDEFS) $< > $@

$(DEPDIR)/$(BUILD)_%.d: $(SRCDIR)/%.rc
	@ echo $(INTDIR)/$(BUILD)_$(basename $(call notdirx,$<))$(OBJEXT): $< > $@

#---------------------------------------------------------------------
# compiling
#

# ---------- .cpp, _sspp.cpp object from source
#
$(INTDIR)/$(BUILD)_%$(OBJEXT): $(SRCDIR)/%.cpp $(DEPDIR)/$(BUILD).cppflags $(DEPDIR)/$(BUILD)_%.d $(GENERATE)
	@ echo c++compile $<
	$(CPP) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(OBJPREF)$@ $(CDEFS) $(CINCLS) $<

$(INTDIR)/$(BUILD)_$(BUILD)_sspp$(OBJEXT): $(INTDIR)/$(BUILD)_sspp.cpp $(DEPDIR)/$(BUILD).cppflags  $(DEPDIR)/$(BUILD)_%.d $(GENERATE)
	@ echo c++compile $<
	$(CPP) $(CPPFLAGS) $(EXTRA_CPPFLAGS) $(OBJPREF)$@ $(CDEFS) $(CINCLS) $<

# ---------- .m, _sspp.m object from source
#
$(INTDIR)/$(BUILD)_%$(OBJEXT): $(SRCDIR)/%.m $(DEPDIR)/$(BUILD).cppflags $(DEPDIR)/$(BUILD)_%.d $(GENERATE)
	@ echo objective-c compile $<
	$(CPP) $(CPPFLAGS) -x objective-c $(EXTRA_CPPFLAGS) $(OBJPREF)$@ $(CDEFS) $(CINCLS) $<

$(INTDIR)/$(BUILD)_$(BUILD)_ssm$(OBJEXT): $(INTDIR)/$(BUILD)_ssm.m $(DEPDIR)/$(BUILD).cppflags  $(DEPDIR)/$(BUILD)_%.d $(GENERATE)
	@ echo objective-c compile $<
	$(CPP) $(CPPFLAGS) -x objective-c $(EXTRA_CPPFLAGS) $(OBJPREF)$@ $(CDEFS) $(CINCLS) $<

# ---------- .c, _ss.c object from source
#
$(INTDIR)/$(BUILD)_%$(OBJEXT): $(SRCDIR)/%.c $(DEPDIR)/$(BUILD).cflags $(DEPDIR)/$(BUILD)_%.d $(GENERATE)
	@ echo c-compile $<
	"$(CC)" $(CFLAGS) $(EXTRA_CFLAGS) $(OBJPREF)$@ $(CDEFS) $(CINCLS) $<

$(INTDIR)/$(BUILD)_%$(OBJEXT): $(INTDIR)/%.c $(DEPDIR)/$(BUILD).cflags $(DEPDIR)/$(BUILD)_%.d $(GENERATE)
	@ echo c-compile $<
	"$(CC)" $(CFLAGS) $(EXTRA_CFLAGS) $(OBJPREF)$@ $(CDEFS) $(CINCLS) $<

$(INTDIR)/$(BUILD)_$(BUILD)_ss$(OBJEXT): $(INTDIR)/$(BUILD)_ss.c $(DEPDIR)/$(BUILD).cflags $(DEPDIR)/$(BUILD)_%.d $(GENERATE)
	@ echo c-compile $<
	"$(CC)" $(CFLAGS) $(EXTRA_CFLAGS) $(OBJPREF)$@ $(CDEFS) $(CINCLS) $<

# ---------- .s, object from source
#
$(INTDIR)/$(BUILD)_%$(OBJEXT): $(SRCDIR)/%.s $(DEPDIR)/$(BUILD).asmflags $(DEPDIR)/$(BUILD)_%.d $(GENERATE)
	@ echo assemble $< to $@
	"$(ASM)" $(ASMFLAGS) $(EXTRA_ASMFLAGS) $(OBJPREF)$@ $(ADEFS) $(AINCLS) $<

$(INTDIR)/$(BUILD)_%$(OBJEXT): $(INTDIR)/%.s $(DEPDIR)/$(BUILD).asmflags $(DEPDIR)/$(BUILD)_%.d $(GENERATE)
	@ echo assemble $<
	"$(ASM)" $(ASMFLAGS) $(EXTRA_ASMFLAGS) $(OBJPREF)$@ $(ADEFS) $(AINCLS) $<


# ---------- .rc object from resource
#
$(INTDIR)/$(BUILD)_%.res: $(SRCDIR)/%.rc $(DEPDIR)/$(BUILD).cflags $(DEPDIR)/$(BUILD)_%.d $(GENERATE)
	@ echo resource-compile $<
	windres $(CDEFS) $(CINCLS) $(EXTRA_CFLAGS) -O coff -o $@ -i $<

$(INTDIR)/$(BUILD)_%.o: $(SRCDIR)/%.rc $(DEPDIR)/$(BUILD).cflags $(DEPDIR)/$(BUILD)_%.d $(GENERATE)
	@ echo resource-compile $<
	windres $(CDEFS) $(CINCLS) $(EXTRA_CFLAGS) -o $@ -i $<

ifeq (,$(findstring clean,$(MAKECMDGOALS)))

# -------------------------------------------------
# combine all CPP flags into one entity
# to compare against cached combo
#
NEWCPPFLAGS := $(strip $(CPPFLAGS) $(CDEFS) $(CINCLS))
ifneq (,$(wildcard $(DEPDIR)/$(BUILD).cppflags))
OLDCPPFLAGS := $(strip $(shell cat $(DEPDIR)/$(BUILD).cppflags))
else
NOOLDCPPFLAGS = 1
endif

ifneq ($(NEWCPPFLAGS),$(OLDCPPFLAGS))

$(DEPDIR)/%.cppflags: $(INTDIR) $(OUTDIR) $(DEPDIR)
ifndef NOOLDCPPFLAGS
	@ echo $(BUILD): C++FLAGS Changed
endif
ifneq ($(HOST_OS),Windows_NT)
	@ echo $(subst ",\",$(NEWCPPFLAGS)) > $@
else
	@ echo $(NEWCPPFLAGS) > $@
endif

else

$(DEPDIR)/%.cppflags: $(INTDIR) $(OUTDIR) $(DEPDIR)

endif

$(DEPDIR)/$(BUILD).cppflags: FORCE

# -------------------------------------------------
# combine all C flags into one entity
# to compare against cached combo
#
NEWCFLAGS = $(strip $(CFLAGS) $(CDEFS) $(CINCLS))
ifneq (,$(wildcard $(DEPDIR)/$(BUILD).cflags))
OLDCFLAGS := $(strip $(shell cat $(DEPDIR)/$(BUILD).cflags))
else
NOOLDCFLAGS := 1
endif

ifneq ($(NEWCFLAGS),$(OLDCFLAGS))

$(DEPDIR)/%.cflags: $(INTDIR) $(OUTDIR) $(DEPDIR)
ifndef NOOLDCFLAGS
	@ echo $(BUILD): CFLAGS Changed
#	@ echo old=
#	@ echo $(OLDCFLAGS)
#	@ echo new=
#	@ echo $(NEWCFLAGS)
endif
ifneq ($(HOST_OS),Windows_NT)
	@ echo $(subst ",\",$(NEWCFLAGS)) > $@
else
	@ echo $(NEWCFLAGS) > $@
endif

else

$(DEPDIR)/%.cflags: $(INTDIR) $(OUTDIR) $(DEPDIR)

endif

$(DEPDIR)/$(BUILD).cflags: FORCE

# -------------------------------------------------
# combine all ASM flags into one entity
# to compare against cached combo
#
NEWAFLAGS := $(strip $(ASMFLAGS) $(ADEFS) $(AINCLS))
ifneq (,$(wildcard $(DEPDIR)/$(BUILD).asmflags))
OLDAFLAGS := $(strip $(shell cat $(DEPDIR)/$(BUILD).asmflags))
else
NOOLDAFLAGS := 1
endif

ifneq ($(NEWAFLAGS),$(OLDAFLAGS))

$(DEPDIR)/%.asmflags: $(INTDIR) $(OUTDIR) $(DEPDIR)
ifndef NOOLDAFLAGS
	@ echo $(BUILD): ASMFLAGS Changed
endif
ifneq ($(HOST_OS),Windows_NT)
	@ echo $(subst ",\",$(NEWAFLAGS)) > $@
else
	@ echo $(NEWAFLAGS) > $@
endif
else

$(DEPDIR)/%.asmflags: $(INTDIR) $(OUTDIR) $(DEPDIR)

endif

$(DEPDIR)/$(BUILD).asmflags: FORCE

endif

FORCE:

# -------------------------------------------------

$(INTDIR):
	$(MKDIR) $(MKDIRFLAGS) $(XINTDIR)

ifneq (.,$(OUTDIR))
$(OUTDIR):
	$(MKDIR) $(MKDIRFLAGS) $(XOUTDIR)
else
$(OUTDIR):
endif

$(DEPDIR):
	$(MKDIR) $(MKDIRFLAGS) $(XDEPDIR)

$(DOCDIR):
	$(MKDIR) $(MKDIRFLAGS) $(DOCDIR)

# -- hack rule to quick-build a single subsystem
%.a:
	@+ $(MAKE) $(NODIRMSG) -f $(ROOT)/$(basename $@)/makefile SRCDIR=$(ROOT)/$(basename $@)

#---------------------------------------------------------------------
# Static libary makefile generation
#
ifneq (x$(strip $(LIBS)), x)

ifeq (,$(wildcard $(DEPDIR)))
    LIBRULEDIR = $(DEPDIR) $(INTDIR)
else
    LIBRULEDIR =
endif

$(DEPDIR)/%.libmake: $(LIBRULEDIR)
ifeq ($(V),1)
	@ echo build make $@ for lib $(INTDIR)/$(LIBPREF)$(notdir $(subst -,/,$(basename $@)))$(LIBEXT)
endif
	@ echo $(INTDIR)/$(LIBPREF)$(notdir $(subst -,/,$(basename $@)))$(LIBEXT): FORCE > $@
	@ echo 'ifeq ($$(VERBOSE),1)' >> $@
	@ $(ECHOE) '\t@ echo $$(ISDOING) Library $(subst -,/,$(notdir $(basename $@)))' >> $@
ifeq ($(ECHOE),printf)
	@ echo "" >> $@
endif
	@ echo 'endif' >> $@
	@ $(ECHOE) '\t@+ $(MAKE) $$(NODIRMSG) -f $(ROOT)/$(subst -,/,$(notdir $(basename $@)))/makefile ROOT=$(ROOT) SRCDIR=$(ROOT)/$(subst -,/,$(notdir $(basename $@))) TARGET_OS=$(TARGET_OS) TARGET_ARCH=$(TARGET_ARCH) $$(CLEANGOALS)' >> $@
ifeq ($(ECHOE),printf)
	@ echo "" >> $@
endif
	@ echo >> $@

-include $(LIBDEPENDS)

endif

#---------------------------------------------------------------------
# Shared libary makefile generation
#
ifneq (x$(strip $(SHLIBS)), x)

ifeq (,$(wildcard $(DEPDIR)))
    SHLIBRULEDIR = $(DEPDIR) $(INTDIR)
else
    SHLIBRULEDIR =
endif

$(DEPDIR)/%.shlibmake: $(SHLIBRULEDIR)
ifeq ($(V),1)
	@ echo build make $@ for lib $(INTDIR)/$(LIBPREF)$(notdir $(subst -,/,$(basename $@)))$(SHLIBEXT)
endif
	@ echo $(INTDIR)/$(LIBPREF)$(notdir $(subst -,/,$(basename $@)))$(SHLIBEXT): FORCE > $@
	@ echo 'ifeq ($$(VERBOSE),1)' >> $@
	@ $(ECHOE) '\t@ echo $$(ISDOING) Shared Library $(subst -,/,$(notdir $(basename $@)))' >> $@
ifeq ($(ECHOE),printf)
	@ echo "" >> $@
endif
	@ echo 'endif' >> $@
	@ $(ECHOE) '\t@+ $(MAKE) $$(NODIRMSG) -f $(ROOT)/$(subst -,/,$(notdir $(basename $@)))/makefile ROOT=$(ROOT) SRCDIR=$(ROOT)/$(subst -,/,$(notdir $(basename $@))) TARGET_OS=$(TARGET_OS) TARGET_ARCH=$(TARGET_ARCH) $$(CLEANGOALS)' >> $@
ifeq ($(ECHOE),printf)
	@ echo "" >> $@
endif
	@ echo >> $@

-include $(SHLIBDEPENDS)

endif

#---------------------------------------------------------------------
# Tools makefile generation
#
ifneq (x$(strip $(TOOLS)), x)

ifeq (,$(wildcard $(TOOLDEPDIR)))
    TOOLRULEDIR = $(TOOLDEPDIR) $(INTDIR)
else
    TOOLRULEDIR =
endif

$(TOOLDEPDIR)/%.toolmake: $(TOOLRULEDIR)
ifeq ($(V),1)
	@ echo build make $@ for tool $(OUTDIR)/$(notdir $(subst -,/,$(basename $@)))$(HOST_EXEEXT)
endif
	@ echo $(OUTDIR)/$(notdir $(subst -,/,$(basename $@)))$(HOST_EXEEXT): FORCE > $@
	@ echo 'ifeq ($$(VERBOSE),1)' >> $@
	@ $(ECHOE) '\t@ echo $$(ISDOING) tool $(subst -,/,$(notdir $(basename $@)))' >> $@
ifeq ($(ECHOE),printf)
	@ echo "" >> $@
endif
	@ echo 'endif' >> $@
	@ $(ECHOE) '\t@+ $(MAKE) $$(NODIRMSG) -f $(ROOT)/$(subst -,/,$(notdir $(basename $@)))/makefile ROOT=$(ROOT) SRCDIR=$(ROOT)/$(subst -,/,$(notdir $(basename $@))) TARGET_OS=$(HOST_OS) TARGET_ARCH=$(HOST_ARCH) $$(CLEANGOALS)' >> $@
ifeq ($(ECHOE),printf)
	@ echo "" >> $@
endif
	@ echo >> $@

-include $(TOOLDEPENDS)

endif

#---------------------------------------------------------------------
# Application makefile generation
#
ifneq (x$(strip $(APPLICATIONS)), x)

ifeq (,$(wildcard $(DEPDIR)))
    APPRULEDIR = $(DEPDIR) $(INTDIR)
else
    APPRULEDIR =
endif

$(DEPDIR)/%.appmake: $(APPRULEDIR)
ifeq ($(V),1)
	@ echo build make $@ for app $(OUTDIR)/$(notdir $(subst -,/,$(basename $@)))$(EXEEXT)
endif
	@ echo .PHONY: $(OUTDIR)/$(notdir $(subst -,/,$(basename $@)))$(EXEEXT) > $@
	@ echo $(OUTDIR)/$(notdir $(subst -,/,$(basename $@)))$(EXEEXT): FORCE > $@
	@ echo 'ifeq ($$(VERBOSE),1)' >> $@
	@ $(ECHOE) '\t@ echo $$(ISDOING) Application $(subst -,/,$(notdir $(basename $@)))' >> $@
ifeq ($(ECHOE),printf)
	@ echo "" >> $@
endif
	@ echo 'endif' >> $@
	@ $(ECHOE) '\t@+ $(MAKE) $$(NODIRMSG) -f $(ROOT)/$(subst -,/,$(notdir $(basename $@)))/makefile ROOT=$(ROOT) SRCDIR=$(ROOT)/$(subst -,/,$(notdir $(basename $@))) TARGET_OS=$(TARGET_OS) TARGET_ARCH=$(TARGET_ARCH) $$(CLEANGOALS)' >> $@
ifeq ($(ECHOE),printf)
	@ echo "" >> $@
endif
	@ echo >> $@

-include $(APPDEPENDS)

endif

ifeq (,$(findstring clean,$(MAKECMDGOALS)))

-include $(DEPENDS)

ifeq (,$(wildcard $(DEPDIR)))
    XXXDEPDIR = $(DEPDIR) $(INTDIR)
else
    XXXDEPDIR =
endif

ifeq (,$(findstring depgen,$(BUILD)))
$(DEPENDS): $(DEPGEN)
endif

$(DEPGEN): $(XXXDEPDIR)
ifneq (,$(DEPGEN))
ifeq (,$(wildcard $(call notdirx,$(DEPGEN))*))
ifeq (,$(findstring depgen,$(BUILD)))
	@ echo Building Dependency generator
	@+ $(MAKE) $(NODIRMSG) -f $(ROOT)/build/tools/depgen/makefile SRCDIR=$(ROOT)/build/tools/depgen TARGET_OS=$(HOST_OS) TARGET_ARCH=$(HOST_ARCH)
else
#	@ echo Avoiding recursive make of depgen
endif
else
#	@ echo already built depgen
endif
endif

endif

#---------------------------------------------------------------------
# FLAGS

flags:
	@echo ROOT     = $(ROOT)
	@echo TARGET_ARCH   = $(TARGET_ARCH)  OS = $(TARGET_OS)
	@echo HOST     = $(HOST_ARCH)  OS = $(HOST_OS)
	@echo SRCDIR   = $(SRCDIR)
	@echo BUILDFL  = $(BUILD_INCL)
	@echo CFLAGS   = $(CFLAGS) $(EXTRA_CFLAGS)
	@echo C++FLAGS = $(CPPFLAGS) $(EXTRA_CPPFLAGS)
	@echo ASMFLAGS = $(ASMFLAGS) $(EXTRA_ASMFLAGS)
	@echo LDFLAGS  = $(LDFLAGS) $(EXTRA_LDFLAGS)
	@echo CDEFINES = $(CDEFS)
	@echo ADEFINES = $(ADEFS)
	@echo QADEFINES= $(QADEFS)
	@echo CINCLUDE = $(CINCLS)
	@echo AINCLUDE = $(AINCLS)

#---------------------------------------------------------------------
# DOC

.PHONY: documentation
documentation: All $(DOCDIR) $(GENERATE)
	echo g=$(TOOLEXECS)
ifeq ($(MAKELEVEL),$(BSA_TOP_LEVEL))
	@ $(RM) -rf $(DOCDIR)/*
endif
	@ echo Creating configuration file
	$(CURDIR)/docgen$(HOSTEXEEXT) -d -o $(DOCDIR) $(CDEFS) $(CINCLS) -t "Application Code Documentation" $(ROOT)
	@ echo Creating Documentation
	cd $(DOCDIR) && doxygen doxygenconfig.txt

.PHONY: fulldocumentation
fulldocumentation: All $(DOCDIR) $(GENERATE)
ifeq ($(MAKELEVEL),$(BSA_TOP_LEVEL))
	@ $(RM) -rf $(DOCDIR)/*
endif
	@ echo Creating configuration file
	$(CURDIR)/docgen$(HOSTEXEEXT) -d -g -o $(DOCDIR) $(CDEFS) $(CINCLS) -t "Application Code Documentation" $(ROOT)
	@ echo Creating Documentation with Call-Graphs
	cd $(DOCDIR) && doxygen doxygenconfig.txt

#---------------------------------------------------------------------
# TAR (devel)

ifneq (x$(findstring devtar,$(MAKECMDGOALS)), x)

ifeq ($(MAKELEVEL),$(BSA_TOP_LEVEL))
TARFILE	=	$(CURDIR)/develsource.tar
export TARFILE
endif

# list of file in the source directory which want to be part of a tarball
# but not really wanting to be put in a list of source files
#
MSOURCES:= $(wildcard $(SRCDIR)/*.h) $(wildcard $(SRCDIR)/*.inc) $(wildcard $(SRCDIR)/*.qinc)	\
			$(wildcard $(SRCDIR)/*.mak) $(wildcard $(SRCDIR)/*.dsp)	$(wildcard $(SRCDIR)/*.dsw)	\
			$(wildcard $(SRCDIR)/*.sct) $(wildcard $(SRCDIR)/readme.txt) $(wildcard $(SRCDIR)/README) $(wildcard $(SRCDIR)/makefile)

ifeq ($(HOST_OS),Windows_NT)
	ABSROOT	:=	$(subst \,/,$(shell cd $(subst /,\,$(ROOT)) && pwd))
	SRCROOT	:=	$(subst \,/,$(shell cd $(subst /,\,$(SRCDIR)) && pwd))
else
	ABSROOT	:=	$(shell cd $(ROOT) ; pwd)
	SRCROOT	:=	$(shell cd $(SRCDIR) ; pwd)
endif
RELMSRCS:=$(subst $(ABSROOT)/,,$(subst $(SRCDIR),$(SRCROOT),$(MSOURCES)))
MISCSOURCES:=$(RELMSRCS)

TARSOURCES=$(ALLSOURCES) $(MISCSOURCES)

ifneq (x$(strip $(TARSOURCES)), x)
TARFLAGS	=	-C $(ROOT) -rf
else
TARFLAGS	=
endif

define devtarmake
@ make  $(NODIRMSG) -f $(ROOT)/$(dir)/makefile SRCDIR=$(ROOT)/$(dir) devtar

endef
endif

#---------------------------------------------------------------------
# TAR (release)

ifneq (x$(findstring reltar,$(MAKECMDGOALS)), x)

ifeq ($(MAKELEVEL),$(BSA_TOP_LEVEL))
TARFILE	=	$(CURDIR)/releasesource.tar
export TARFILE
endif

# list of file in the current directory which want to be part of a tarball
# but not really wanting to be put in a list of source files
#
MSOURCES:= $(wildcard $(SRCDIR)/*.h) $(wildcard $(SRCDIR)/*.inc) $(wildcard $(SRCDIR)/*.qinc) \
			$(wildcard $(SRCDIR)/*.mak) $(wildcard $(SRCDIR)/*.dsp) $(wildcard $(SRCDIR)/*.dsw) \
			$(wildcard $(SRCDIR)/*.sct) $(wildcard $(SRCDIR)/makefile)

ifeq ($(HOST_OS),Windows_NT)
	ABSROOT	:=	$(subst \,/,$(shell cd $(subst /,\,$(ROOT)) && pwd))
	SRCROOT	:=	$(subst \,/,$(shell cd $(subst /,\,$(SRCDIR)) && pwd))
else
	ABSROOT	:=	$(shell cd $(ROOT) ; pwd)
	SRCROOT	:=	$(shell cd $(SRCDIR) ; pwd)
endif
RELMSRCS:=$(subst $(ABSROOT)/,,$(subst $(SRCDIR),$(SRCROOT),$(MSOURCES)))
MISCSOURCES:=$(RELMSRCS)

TARSOURCES=$(RELEASE_SOURCES) $(MISCSOURCES)

ifneq (x$(strip $(TARSOURCES)), x)
TARFLAGS	=	-C $(ROOT) -rf
else
TARFLAGS	=
endif

define reltarmake
@ make  $(NODIRMSG) -f $(ROOT)/$(dir)/makefile SRCDIR=$(ROOT)/$(dir) reltar

endef

endif

.PHONY: devtar
devtar:
ifeq ($(MAKELEVEL),$(BSA_TOP_LEVEL))
	$(RM) $(RMFLAGS) $(TARFILE)
	@ echo Creating archive $(TARFILE)
	@ tar -C $(ROOT) -cf $(TARFILE) build documentation tools incl readme.txt
endif
#	echo ar=$(ABSROOT) ts=$(MISCSOURCES)
ifneq (x$(strip $(TARFLAGS)), x)
	@tar $(TARFLAGS) $(TARFILE) $(TARSOURCES)
endif
	$(foreach dir,$(TOOLS), $(devtarmake))
	$(foreach dir,$(SLPPA), $(devtarmake))
	$(foreach dir,$(LIBRARIES),  $(devtarmake))
	$(foreach dir,$(SHARED_LIBRARIES),  $(devtarmake))

.PHONY: reltar
reltar:
ifeq ($(MAKELEVEL),$(BSA_TOP_LEVEL))
	$(RM) $(RMFLAGS) $(TARFILE)
	@ echo Creating archive $(TARFILE)
	@ tar -C $(ROOT) -cf $(TARFILE) build documentation tools incl readme.txt
endif
ifneq (x$(strip $(TARFLAGS)), x)
	@ tar $(TARFLAGS) $(TARFILE) $(TARSOURCES)
endif
	$(foreach dir,$(TOOLS), $(reltarmake))
	$(foreach dir,$(APPLICATIONS), $(reltarmake))
	$(foreach dir,$(LIBRARIES),  $(reltarmake))
	$(foreach dir,$(SHARED_LIBRARIES),  $(reltarmake))

#---------------------------------------------------------------------
# CLEAN

define cleanmake
@+ $(MAKE) $(NODIRMSG) -f $(ROOT)/$(dir)/makefile SRCDIR=$(ROOT)/$(dir) TARGET_OS=$(TARGET_OS) TARGET_ARCH=$(TARGET_ARCH) clean

endef

define cleanobjmake
@+ $(MAKE) $(NODIRMSG) -f $(ROOT)/$(dir)/makefile SRCDIR=$(ROOT)/$(dir) TARGET_OS=$(TARGET_OS) TARGET_ARCH=$(TARGET_ARCH) clean

endef

# these things are in the directories deleted by clean so no
# need to delete directly, but put these under clean if that changes
#	@ rm -f $(OBJECTS)
#	@ rm -f $(DEPENDS)
#	@ rm -f $(BUILDFLAGS)

.PHONY: clean
clean::	$(GENERATE)
	@ -rm -f $(ACTUAL_TARGET)
	@ rm -f $(DEPGEN)*
	@ rm -f $(XCLEAN)
	@ $(foreach dir,$(SUBMAKE_DEPS), $(cleanmake))
	@ $(foreach dir,$(TOOLS), $(cleanmake))
	@ $(foreach dir,$(APPLICATIONS), $(cleanmake))
	@ $(foreach dir,$(LIBRARIES),  $(cleanmake))
	@ $(foreach dir,$(SHARED_LIBRARIES),  $(cleanmake))
ifeq ($(MAKELEVEL),$(BSA_TOP_LEVEL))
	@ -rm -rf $(INTDIR) $(DEPDIR) $(DOCDIR)
ifneq ($(HOST_ARCH),$(TARGET_ARCH))
	@ -rm -rf .dep$(HOST_ARCH) .obj$(HOST_ARCH)
endif
endif

.PHONY: cleanobj
cleanobj:
	@ rm -f $(OBJECTS)
	@ $(foreach dir,$(TOOLS), $(cleanobjmake))
	@ $(foreach dir,$(APPLICATIONS), $(cleanobjmake))
	@ $(foreach dir,$(LIBRARIES),  $(cleanobjmake))
	@ $(foreach dir,$(SHARED_LIBRARIES),  $(cleanobjmake))

#-include $(ROOT)/build/printvars.mak

