include ../../config.mk

all: matlabtest reproducibilitytest

ifeq "$(findstring arm, $(ARCH))" "arm"
matlabtest:
	@echo cannot execute automatic tests for cross-compiled MHA
else
matlabtest:
	octave --no-gui --no-window-system --eval "set_environment;exit(~runtests())"
endif

reproducibilitytest:
	@if [ -n "`git status --porcelain -uno`" ]; then (echo; echo "*******************************************************************"; echo "***" The following files have been modified since git check-out: "***";  echo "*******************************************************************"; git status --porcelain -uno; exit 1); fi
	@if [ -n "`find ../plugins -type f -exec grep -- -modified '{}' ';'`" ] ; then (echo; echo "***********************************************************************"; echo "***" The following files carry a modified-since-git-check-out stamp: "***";  echo "***********************************************************************"; find ../plugins -type f -exec grep -- -modified '{}' ';'; exit 1); fi

install: all

clean:
