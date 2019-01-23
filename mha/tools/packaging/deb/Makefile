# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2018 HörTech gGmbH
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

ALL_CSV=$(wildcard *.csv)

CODENAME=$(shell lsb_release -sc)

REPO=../../../..

all: build pack

include $(REPO)/config.mk

GITMODIFIED=$(shell test -z "`cd $(REPO) && git status --porcelain -uno`" || echo "-modified")
COMMITHASH=$(shell cd $(REPO) && git log -1 --abbrev=7 --pretty='format:%h')
PACKAGEVER=$(shell git log --abbrev=7 --pretty='format:%h' | wc -l)
FULLVERSION=$(shell \
    (echo "\#include \"$(REPO)/mha/libmha/src/mha.hh\"";\
     echo 'mhaversion=MHA_VERSION_MAJOR  MHA_VERSION_MINOR  MHA_VERSION_RELEASE') | \
     cpp -x c++ | grep -e 'mhaversion='|sed -e 's/mhaversion=//1' -e 's/ /./g' \
    )-$(PACKAGEVER)-$(COMMITHASH)$(GITMODIFIED)-gcc$(GCC_VER)
CURRENTPATH=$(shell pwd)

build:
	(cd $(REPO) && make all)

pack:
	rm -Rf hoertech
	mkdir -p hoertech/$(CODENAME)
	for c in $(ALL_CSV); do (cd $(REPO) && BUILD_DIR=$(BUILD_DIR) mhamakedeb $(CURRENTPATH)/$$c $(FULLVERSION) ) || exit 1; done
	mv $(REPO)/*.deb hoertech/$(CODENAME)
