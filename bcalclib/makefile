
EXTRA_CPPFLAGS += -D_LIB

SOURCES		=					\
					bcalc.cpp				\
					bnumber.cpp				\
					bdblnum.cpp				\
					bintnum.cpp				\
					barbnum.cpp				\
					digit.cpp				\
					bprog.cpp				\
					bplot.cpp

include $(ROOT)/build/template.mak

$(CURDIR)/bcalcrc.cpp: $(INTDIR)/bcalc.res
	echo resource-format $< to $@
	bsarc -o $@ $<

