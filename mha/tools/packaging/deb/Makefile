# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2018 2019 2020 HörTech gGmbH
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

# debian package content is defined by comma-separated value files with name
# <packagename>.csv. ALL_CSV is set to a list of all .csv files in this
# directory
ALL_CSV=$(wildcard *.csv)

# CODENAME is the name of the debian or ubuntu distribution on which this
# Makefile executes and for which the debian packages are created.
# CODENAME is e.g. "bionic" for ubuntu 18.04 or "focal" for ubuntu 20.04
CODENAME=$(shell lsb_release -sc)

# REPO is the location of the base of this git workspace relative to the
# directory containing this Makefile.  This Makefile is in
# $(REPO)/mha/tools/packaging/deb, therefore we need to go up four parent
# directory levels to get to REPO.
REPO=../../../..

# Default target of the Makefile for manual invocation. Not safe for parallel
# make execution.  Main openMHA Makefile directly invokes target pack which is
# safe for parallel execution.
all: build pack

# Include Makefile variables from config.mk
include $(REPO)/config.mk

# If the git repository contains any non-committed modifications, then
# GITMODIFIED is set to "-modified", else it is empty.  GITMODIFIED is used
# as part of the debian package version to easily spot non-reproducible debs
GITMODIFIED=$(shell test -z "`cd $(REPO) && git status --porcelain -uno`" || echo "-modified")

# Part of the git commit SHA1 is stored in COMMITHASH and becomes part of the
# debian package version
COMMITHASH=$(shell cd $(REPO) && git log -1 --abbrev=7 --pretty='format:%h')

# PACKAGEVER stores the git age, i.e. how many revisions are in the git history
# from the start to the version that is packaged here.
PACKAGEVER=$(shell cd $(REPO) && git log --abbrev=7 --pretty='format:%h' |wc -l)

# MHAVERSION is the <major>.<minor>.<release> version of MHA extracted from
# header file mha.hh
MHAVERSION = $(shell \
    (echo "\#include \"$(REPO)/mha/libmha/src/mha.hh\"";\
     echo 'mhaversion=MHA_VERSION_MAJOR  MHA_VERSION_MINOR  MHA_VERSION_RELEASE') | \
     cpp -x c++ | grep -e 'mhaversion='|sed -e 's/mhaversion=//1' -e 's/ /./g')

# FULLVERSIONALL is the MHA version plus git age plus git hash plus any
# modification indicator but without the compiler version.
FULLVERSIONALL=$(MHAVERSION)-$(PACKAGEVER)-$(COMMITHASH)$(GITMODIFIED)
export FULLVERSIONALL

# FULLVERSIONGCC is FULLVERSIONALL plus the compiler version
FULLVERSIONGCC=$(FULLVERSIONALL)-gcc$(GCC_VER)

CURRENTPATH=$(shell pwd)

# DEBARCH is set to the native debian architecture name of the platform where
# this Makefile executes, i.e. amd64, armhf, or i386
DEBARCH = $(shell dpkg-architecture --query DEB_BUILD_ARCH)
# FIXME: allow to specify architecture per package to enable architecture "all"

# List of all debian control files
ALL_CONTROL_FILES=$(patsubst %.csv,%.control,$(ALL_CSV))

# List of only those debian control files that contain
# Architecture: all
# i.e. the same deb file works on all architectures
NOARCH_CONTROL_FILES=$(shell grep 'Architecture: all' *.control | cut -d: -f1)

# List of only those debian control files that do *NOT* contain
# Architecture: all
# i.e. the deb file works only on one architecture
NATIVE_CONTROL_FILES=$(filter-out $(NOARCH_CONTROL_FILES), $(ALL_CONTROL_FILES))

# List of all architecture-specific debian packages in their final location
# to be packaged by this Makefile
NATIVE_DEBS=$(patsubst %.control,hoertech/$(CODENAME)/%_$(FULLVERSIONGCC)_$(DEBARCH).deb,$(NATIVE_CONTROL_FILES))

# List of all architecture independent debian packages in their final location
# to be packaged by this Makefile
NOARCH_DEBS=$(patsubst %.control,hoertech/$(CODENAME)/%_$(FULLVERSIONALL)_all.deb,$(NOARCH_CONTROL_FILES))

# Pre-dependency to ensure before packaging starts.  Here that openMHA is
# compiled.  Main openMHA Makefile ensures this itself and invokes target pack.
build:
	(cd $(REPO) && make all)

# Main openMHA Makefile invokes target pack after openMHA has been compiled.
pack: pack-all pack-native

pack-all: $(NOARCH_DEBS)
pack-native: $(NATIVE_DEBS)

# Target clean cannot be invoked in parallel with target pack, needs to be
# called separately and be allowed to finish BEFORE pack is called if clean
# before pack is desired.
# The output of all Makefile variables is not necessary for the cleaning, only
# here for informational purposes and maybe to help debugging package generation
clean:
	rm -Rf hoertech
	@echo List of Makefile variables defined in $$PWD/Makefile:
	@echo ALL_CSV='$(ALL_CSV)'
	@echo CODENAME='$(CODENAME)'
	@echo REPO='$(REPO)'
	@echo GITMODIFIED='$(GITMODIFIED)'
	@echo COMMITHASH='$(COMMITHASH)'
	@echo PACKAGEVER='$(PACKAGEVER)'
	@echo MHAVERSION='$(MHAVERSION)'
	@echo FULLVERSIONALL='$(FULLVERSIONALL)'
	@echo FULLVERSIONGCC='$(FULLVERSIONGCC)'
	@echo CURRENTPATH='$(CURRENTPATH)'
	@echo DEBARCH='$(DEBARCH)'
	@echo ALL_CONTROL_FILES='$(ALL_CONTROL_FILES)'
	@echo NOARCH_CONTROL_FILES='$(NOARCH_CONTROL_FILES)'
	@echo NATIVE_CONTROL_FILES='$(NATIVE_CONTROL_FILES)'
	@echo NOARCH_DEBS='$(NOARCH_DEBS)'
	@echo NATIVE_DEBS='$(NATIVE_DEBS)'

# Rule for creating output directories
hoertech/$(CODENAME)/.directory:
	mkdir -p hoertech/$(CODENAME)
	touch $@

# Rules for creating the debian packages.  First they are created in the base
# directory $(REPO) with mhamakedeb and then moved to the target directory.
hoertech/$(CODENAME)/%_$(FULLVERSIONGCC)_$(DEBARCH).deb: hoertech/$(CODENAME)/.directory %.csv %.control %.dep
	cd $(REPO) && BUILD_DIR=$(BUILD_DIR) mhamakedeb $(CURRENTPATH)/$*.csv $(FULLVERSIONGCC)
	mv $(REPO)/$(notdir $@) hoertech/$(CODENAME)/

# A copy of the cross-platform packages will be left in $(REPO) to enable
# sharing of these packages through Jenkins
hoertech/$(CODENAME)/%_$(FULLVERSIONALL)_all.deb: hoertech/$(CODENAME)/.directory $(REPO)/%_$(FULLVERSIONALL)_all.deb
	ln $(REPO)/$(notdir $@) hoertech/$(CODENAME)/
$(REPO)/%_$(FULLVERSIONALL)_all.deb: %.csv %.control %.dep
	cd $(REPO) && mhamakedeb $(CURRENTPATH)/$*.csv $(FULLVERSIONALL) all

# Do not delete the intermediate deb files in the base directory
.PRECIOUS:
.SECONDARY:
