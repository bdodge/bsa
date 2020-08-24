
BUILD_TYPE	=	Executable


SOURCES		=					\
					bcalcmain.cpp				\
					bcalcrc.cpp

ifeq ($(TARGET_OS),Windows_NT)
SOURCES +=						\
					bcalcres.rc
endif
LIBRARIES	=					\
					bcalclib					\
					bsapersist					\
					bsabs						\
					bsaframe					\
					bsautils					\
					bsaopenfile					\
					bsaxml						\
					bsawiz

ifeq ($(TARGET_OS),OSX)
	EXTRA_LDFLAGS=-L/usr/X11R6/lib
endif

ifneq ($(TARGET_OS),Windows_NT)
	LIBRARIES +=		bsawinapi
	SYSTEM_SHARED_LIBRARIES	= fontconfig Xft
	SYSTEM_SHARED_LIBRARIES += pthread
ifneq ($(TARGET_OS),Darwin)
	SYSTEM_SHARED_LIBRARIES += X11 util
else
	SYSTEM_SHARED_LIBRARIES += X11
endif
else
	SYSTEM_LIBRARIES	= 			\
					\MingW\Lib\libgdi32 		\
					\MingW\Lib\libcomctl32 		\
					\MingW\Lib\libcomdlg32		\
					\MingW\Lib\libwsock32
endif

include $(ROOT)/build/template.mak

ifeq ($(TARGET_OS),Windows_NT)
LD=$(CPP)
endif

$(CURDIR)/bcalcrc.cpp: $(INTDIR)/$(BUILD)_bcalcres.res
	echo resource-format $< to $@
	bsarc -o $@ $<


