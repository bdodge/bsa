
SOURCES		=	\
			winapix.c			\
			winapi.c			\
			icccm.c				\
			winresrc.c			\
			font.c				\
			resource.c			\
			dialog.c			\
			commctrl.c			\
			button.c			\
			editbox.c			\
			listview.c			\
			scroller.c			\
			treeview.c			\
			trackbar.c			\
			menu.c				\
			image.c				\
			msgbox.c			\
			prtdlg.c			\
			x11drvr.c			\
			psdrvr.c			\
			pcldrvr.c

ifeq ($(TARGET_OS),OSX)
SOURCES += cocoadrvr.m	
endif

include $(ROOT)/build/template.mak

winresrc.c: Debug/I386/winres.res
	bsarc -c -n_zg_winapires Debug/I386/winres.res

