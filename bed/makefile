
BUILD_TYPE  =   Executable

SOURCES		=					\
					bedx.cpp					\
					bed.cpp						\
					edit.cpp					\
					buffer.cpp					\
					buffile.cpp					\
					bufftp.cpp					\
					bufraw.cpp					\
					project.cpp					\
					pdb.cpp						\
					bdbg.cpp					\
					protwind.cpp				\
					dismember.cpp				\
					bsccs.cpp					\
					infopane.cpp				\
					infobufs.cpp				\
					infofunc.cpp				\
					infofile.cpp				\
					infodbg.cpp					\
					infogrep.cpp				\
					infoshell.cpp				\
					infosccs.cpp				\
					infobuild.cpp				\
					inforesults.cpp				\
					parm.cpp					\
					openfiledial.cpp			\
					print.cpp					\
					view.cpp					\
					viewhex.cpp					\
					viewc.cpp					\
					parsec.cpp					\
					viewswift.cpp				\
					viewjava.cpp				\
					viewpython.cpp				\
					viewtcl.cpp					\
					viewasm.cpp					\
					viewhtml.cpp				\
					viewterm.cpp				\
					viewshell.cpp				\
					viewstream.cpp				\
					viewrs232.cpp				\
					viewtelnet.cpp				\
					dispatch.cpp				\
					tclscript.cpp				\
					ftpauth.cpp					\
					sandr.cpp					\
					token.cpp					\
					line.cpp					\
					fontdial.cpp				\
					colordial.cpp				\
					help.cpp					\
					keybind.cpp					\
					macro.cpp					\
					bedrc.cpp

ifeq ($(HOST_OS),Windows_NT)
SOURCES+=					\
					bedres.rc
endif

SOURCES+=						\
					bdbg.h						\
					bed.h						\
					bedrc.h						\
					bedx.h						\
					bsccs.h						\
					buffer.h					\
					infodbg.h					\
					infopane.h					\
					keybind.h					\
					line.h						\
					parm.h						\
					pdb.h						\
					print.h						\
					project.h					\
					resource.h					\
					tclscript.h					\
					view.h						\
					viewshell.h					\
					bed.dsp

LIBRARIES	=						\
					bsapersist					\
					bsaframe					\
					bsaftp						\
					bsabs						\
					bsasercom					\
					bsatelcom					\
					bsautils					\
					bsaopenfile					\
					bcalclib					\
					bsaxml						\
					bsashell					\
					bsawiz


EXTRA_CFLAGS = -I../../../bsawinapi

ifneq ($(TARGET_OS),Windows_NT)
	LIBRARIES +=		bsawinapi

ifndef STATIC

ifeq ($(TARGET_OS),Darwin)
	EXTRA_LDFLAGS=-L/usr/X11R6/lib
	SYSTEM_SHARED_LIBRARIES	= fontconfig Xft X11 pthread util
endif
ifeq ($(TARGET_OS),Linux)
	SYSTEM_SHARED_LIBRARIES	= fontconfig Xft X11 pthread util
endif

#ifeq ($(TARGET_OS),OSX)
#	EXTRA_LDFLAGS=-L/usr/X11R6/lib
#endif


else # static link!

EXTRA_LDFLAGS += -static

SYSTEM_SHARED_LIBRARIES	= Xft X11 pthread util xcb Xau Xdmcp
SYSTEM_SHARED_LIBRARIES	+= freetype z Xrender png pthread fontconfig expat freetype

endif
else # Windows

	SYSTEM_LIBRARIES	= 			\
					C:\MingW\Lib\libgdi32 		\
					C:\MingW\Lib\libcomctl32 		\
					C:\MingW\Lib\libcomdlg32		\
					C:\MingW\Lib\libwsock32
endif

include $(ROOT)/build/template.mak

ifeq ($(TARGET_OS),Windows_NT)
LD=$(CPP)
endif

ifeq ($(HOST_OS),Windows_NT)
$(SRCDIR)/bedrc.cpp: $(SRCDIR)/bedres.res
	echo resource-format $< to $@
	bsarc -o $@ $<
endif
ifeq ($(HOST_OS),Linux)
$(SRCDIR)/bedrc.cpp: $(SRCDIR)/bedres.res
	echo resource-format $< to $@
	bsarc -o $@ $<
endif


