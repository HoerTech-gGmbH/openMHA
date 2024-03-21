# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2005 2009 2011 2013 2015 2017 2018 2019 2020 2021 HörTech gGmbH
# Copyright © 2022 2024 Hörzentrum Oldenburg gGmbH
#
# openMHA is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, version 3 of the License.
#
# openMHA is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License, version 3 for more details.
#
# You should have received a copy of the GNU Affero General Public License, 
# version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.

include ../../config.mk
include ../../magic.mk
ifeq "$(findstring arm, $(ARCH))" "arm"
all: paralleloctavetests reproducibilitytest
else ifeq "$(findstring aarch64, $(ARCH))" "aarch64"
all: paralleloctavetests reproducibilitytest
else
all: octavetest reproducibilitytest
endif

unit-tests:


paralleloctavetests: paralleloctavetest_0 paralleloctavetest_1

paralleloctavetest_%:
	rm -rf ../$@
	cp -rl . ../$@
	$(MAKE) -C ../$@ paralleloctavetest
	rm -rf ../$@

paralleloctavetest:
	pwd | grep -q $@
	ls test_*.m | awk 'NR%2!='$(shell pwd | sed -e 's/.*$@_//') | xargs rm
	$(MAKE) octavetest

ifeq "$(findstring arm, $(COMPILERPREFIX))" "arm"
octavetest:
	@echo cannot execute automatic tests for cross-compiled MHA
else
ifneq "$(WITH_TESTS)" "no"
octavetest:
	$(MHA_TEST_COMMAND)
else
octavetest:
endif
endif

find_executable_arg = -executable
ifeq "$(PLATFORM)" "Darwin"
	find_executable_arg = -perm +111
endif


reproducibilitytest:
	# Fail if there are any modifications not checked into git
	@if [ -n "`git status --porcelain -uno`" ]; then (echo; echo "*******************************************************************"; echo "***" The following files have been modified since git check-out: "***";  echo "*******************************************************************"; git status --porcelain -uno; exit 1); fi
	# Fail if during compilation there were any modifications not checked into git
	@if [ -n "`find ../plugins ../frameworks ../libmha -type f -exec grep -- -modified '{}' ';'`" ] ; then (echo; echo "***********************************************************************"; echo "***" The following files carry a modified-since-git-check-out stamp: "***";  echo "***********************************************************************"; find ../plugins ../frameworks ../libmha -type f -exec grep -- -modified '{}' ';'; exit 1); fi
	# Fail if any compiled artifact is missing the git commit hash or has a wrong git commit hash
	find ../plugins -type f -name "*"$(DYNAMIC_LIB_EXT) | xargs ./check_git_commit_hash $(GITCOMMITHASH)
	find ../frameworks -type f $(find_executable_arg) | grep -v unit-test-runner | xargs ./check_git_commit_hash $(GITCOMMITHASH)
	./check_git_commit_hash $(GITCOMMITHASH) ../libmha/*/*$(DYNAMIC_LIB_EXT)

install: all

clean:

# Local Variables:
# coding: utf-8-unix
# End:
