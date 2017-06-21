# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2005 2009 2011 2013 2015 2017 HörTech gGmbH
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

# This is the main MHA Makefile.
#
# The MHA is organized in modules. Each module is a
# subdirectory. Inter-module dependencies are defined in in this file.
#
# For cross-compilers please set the variable COMPILERPREFIX, e.g.:
#
# make COMPILERPREFIX=arm-linux-gnueabihf-
#
# or add the COMPILERPREFIX variable to config.mk

include ../../config.mk

all: matlabtest reproducibilitytest

matlabtest:
	octave --no-gui --no-window-system --eval "set_environment;exit(~runtests())"

reproducibilitytest:
	@if [ -n "`git status --porcelain -uno`" ]; then (echo; echo "*******************************************************************"; echo "***" The following files have been modified since git check-out: "***";  echo "*******************************************************************"; git status --porcelain -uno; exit 1); fi
	@if [ -n "`find ../plugins -type f -exec grep -- -modified '{}' ';'`" ] ; then (echo; echo "***********************************************************************"; echo "***" The following files carry a modified-since-git-check-out stamp: "***";  echo "***********************************************************************"; find ../plugins -type f -exec grep -- -modified '{}' ';'; exit 1); fi

install: all

clean:
