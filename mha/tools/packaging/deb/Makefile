# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2018 2019 HörTech gGmbH
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

DEBARCH = $(shell dpkg-architecture --query DEB_BUILD_ARCH)
# FIXME: allow to specify architecture per package to enable architecture "all"

ALL_DEBS=$(patsubst %.csv,hoertech/$(CODENAME)/%_$(FULLVERSION)_$(DEBARCH).deb,$(ALL_CSV))

build:
	(cd $(REPO) && make all)

# Main openMHA Makefile invokes pack after openMHA has been compiled.
pack: $(ALL_DEBS)

# Target clean cannot be invoked in parallel with targe pack, needs to be called
# separately and be allowed to finish BEFORE pack is called if clean is desired.
clean:
	rm -Rf hoertech

hoertech/$(CODENAME)/.directory:
	mkdir -p hoertech/$(CODENAME)
	touch $@

hoertech/$(CODENAME)/%_$(FULLVERSION)_$(DEBARCH).deb: hoertech/$(CODENAME)/.directory %.csv %.control %.dep
	cd $(REPO) && BUILD_DIR=$(BUILD_DIR) mhamakedeb $(CURRENTPATH)/$*.csv $(FULLVERSION)
	mv $(REPO)/$(notdir $@) hoertech/$(CODENAME)/
