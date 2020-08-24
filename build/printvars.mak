#******************************************************************************
#   Copyright (c) 2009 Zoran Corporation.
#   $Header: //depot/imgeng/sw/inferno/appsrc/build/printvars.mak#2 $
#   $Change: 149021 $ $Date: 2009/11/22 $
#******************************************************************************
# Author: Eric Miller
#
# DESCRIPTION for printvars.mak
# 	Makefile debugging targets adapted from:
#	  "Ask Mr. Make: Dumping Every Makefile Variable"
#	  <http://www.cmcrossroads.com/index2.php?do_pdf=1&id=6521>
# 
ifndef PRINTVARS_MAKE_INCLUDED
PRINTVARS_MAKE_INCLUDED:=1

# .FEATURES is undefined before Gnu Make 3.81
# If we're running 3.81 (or some later release), use the "info" function to
# print debug info; otherwise, use the older "warning" function, which is a
# little noisier but still gets the job done.
ifdef .FEATURES
  .print=info
else
  .print=warning
endif


# Print out all the variables defined in the top level makefile.
# Note that expanding some variables that use $(call ...) or $(shell ...)
# may not work in all cases, so the simpler printvars is preferred.
printvars_expanded:
	@$(foreach V,$(sort ${.VARIABLES}),                                  \
	   $(if $(filter-out environment% default automatic,$(origin $V)),   \
	        $(call ${.print},$V=${$V})))


# Print out all the top level makefile's variables without expansion.
printvars:
	@$(foreach V,$(sort ${.VARIABLES}),                                  \
	   $(if $(filter-out environment% default automatic,$(origin $V)),   \
	        $(call ${.print},$V=$(value $V))))


# Print out a subsystem's variables.
%.printvars:
	@${MAKE} -C ../$* printvars

%.printvars_unexpanded:
	@${MAKE} -C ../$* printvars_unexpanded


# Print out a single variable, including extra information about how it was
# defined and where it came from.
ifdef .FEATURES
printvar\:%:
	@$(if $(subst ${$*},,$(value $*)),                                   \
	      $(call ${.print},$* $(if $(filter $(flavor $*),simple),:)=     \
		     ${$*} := $(value $*) <$(origin $*)>),                   \
	      $(call ${.print},$* $(if $(filter $(flavor $*),simple),:)=     \
		     ${$*} <$(origin $*)>))
else
printvar\:%:
	@$(if $(subst ${$*},,$(value $*)),                                   \
	      $(call ${.print},$* =                                          \
		     ${$*} := $(value $*) <$(origin $*)>),                   \
	      $(call ${.print},$* =                                          \
		     ${$*} <$(origin $*)>))
endif

printvar:
	$(call ${.print},)
	$(call ${.print},Usage: make printvar:VARIABLE_NAME)
	$(call ${.print},     colon required ^)
	$(call ${.print}, spaces not allowed^ ^)
	$(call ${.print},)

endif # PRINTVARS_MAKE_INCLUDED


# Format: tabs=8
# See "Local Variables in Files", GNU Emacs Manual
# Local Variables:
# mode: makefile
# tab-width: 8
# End:
# Used by vim and some versions of vi: set tabstop=8:
