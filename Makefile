# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2013 2014 2015 2016 2017 2018 2019 2020 HörTech gGmbH
# Copyright © 2022 2024 2026 Hörzentrum Oldenburg gGmbH
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

include config.mk

MODULES = \
	mha/libmha \
	mha/frameworks \
	mha/plugins \
	external_libs

DOCMODULES = \
	mha/doc \

all: $(MODULES)

config.mk:
	./configure

test: all
	$(MAKE) -C mha/mhatest

.PHONY : $(MODULES) $(DOCMODULES) coverage

$(MODULES:external_libs=) $(DOCMODULES):
	$(MAKE) -C $@

external_libs:
	$(MAKE) -C $@

doc: mha/doc
	/bin/cp -lv --remove-destination mha/doc/*.pdf .
	zip -r pdf-$$(cat version).zip *.pdf

clean:
	for m in $(MODULES) $(DOCMODULES); do $(MAKE) -C $$m clean; done

ifeq "$(PLATFORM)" "MinGW"
LIB=bin
else
LIB=lib
endif
ifeq "$(PLATFORM)" "Darwin"
INSTALL_NAME_TOOL=install_name_tool
else
INSTALL_NAME_TOOL=true
endif

install: install_libopenmha install_openmha install_libopenmha-dev install_openmha-examples
install_openmha: install_plugins install_executables install_manuals install_mfiles

install_binaries: install_libopenmha install_plugins install_executables

install_libopenmha: mha/libmha install_COPYING_libopenmha
	mkdir -p "$(abspath $(DESTDIR)$(PREFIX))/$(LIB)"
	rm -f "$(abspath $(DESTDIR)$(PREFIX))/$(LIB)/libopenmha$(DYNAMIC_LIB_EXT)"
	cp "mha/libmha/$(BUILD_DIR)/libopenmha$(DYNAMIC_LIB_EXT)" "$(abspath $(DESTDIR)$(PREFIX))/$(LIB)/"
	$(INSTALL_NAME_TOOL) -id $(PREFIX)/$(LIB)/libopenmha$(DYNAMIC_LIB_EXT) \
	                       $(abspath $(DESTDIR)$(PREFIX))/$(LIB)/libopenmha$(DYNAMIC_LIB_EXT)
	mkdir -p "$(abspath $(DESTDIR)$(PREFIX))/share/openmha"
	cp config.mk "$(abspath $(DESTDIR)$(PREFIX))/share/openmha/"
install_plugins: mha/plugins install_COPYING_openmha
	mkdir -p "$(abspath $(DESTDIR)$(PREFIX))/$(LIB)"
	find mha/plugins/ -type f -name "*$(DYNAMIC_LIB_EXT)" \
	    -execdir rm -f $(abspath $(DESTDIR)$(PREFIX))/$(LIB)/{} \; \
	    -exec cp -v {} $(abspath $(DESTDIR)$(PREFIX))/$(LIB)/ \; \
	    -execdir $(INSTALL_NAME_TOOL) -change "$(shell pwd)/mha/libmha/$(BUILD_DIR)/libopenmha$(DYNAMIC_LIB_EXT)" \
	                                   "$(PREFIX)/$(LIB)/libopenmha$(DYNAMIC_LIB_EXT)" \
	                                   "$(abspath $(DESTDIR)$(PREFIX))/$(LIB)/{}" \; \
	    -execdir $(INSTALL_NAME_TOOL) -id "$(PREFIX)/$(LIB)/{}" \
	                                   "$(abspath $(DESTDIR)$(PREFIX))/$(LIB)/{}" \;
install_executables: install_executable_mha install_executable_analysemhaplugin
install_executable_%: mha/frameworks install_COPYING_openmha
	mkdir -p "$(abspath $(DESTDIR)$(PREFIX))/bin"
	rm -f "$(abspath $(DESTDIR)$(PREFIX))/bin/$*"
	cp "mha/frameworks/$(BUILD_DIR)/$*" "$(abspath $(DESTDIR)$(PREFIX))/bin/"
	$(INSTALL_NAME_TOOL) -change "$(shell pwd)/mha/libmha/$(BUILD_DIR)/libopenmha$(DYNAMIC_LIB_EXT)" \
	                       "$(PREFIX)/$(LIB)/libopenmha$(DYNAMIC_LIB_EXT)" \
	                       "$(abspath $(DESTDIR)$(PREFIX))/bin/$*"
ifeq "$(BUILD_DOCS)" "yes"
install_manuals: doc install_COPYING_openmha
	mkdir -p "$(abspath $(DESTDIR)$(PREFIX))/share/doc/openmha"
	for pdf in *.pdf; do \
	    rm -f "$(abspath $(DESTDIR)$(PREFIX))/share/doc/openmha/$$pdf"; \
	    cp -v "$$pdf" "$(abspath $(DESTDIR)$(PREFIX))/share/doc/openmha/"; \
	done
else
install_manuals:
	@echo "Documentation not installed. Please download from https://www.openmha.org/."
endif
install_libopenmha-dev: install_COPYING_libopenmha-dev
	mkdir -p "$(abspath $(DESTDIR)$(PREFIX))/include/openmha"
	for h in mha/libmha/src/*.h mha/libmha/src/*.hh; do \
	    filename_without_directory=$$(basename "$$h"); \
	    rm -f "$(abspath $(DESTDIR)$(PREFIX))/include/openmha/$$filename_without_directory"; \
	    cp -v "$$h" "$(abspath $(DESTDIR)$(PREFIX))/include/openmha/"; \
	done
install_openmha-examples: install_COPYING_openmha-examples
	mkdir -p "$(abspath $(DESTDIR)$(PREFIX))/share/openmha/examples"
	cp -r "examples/"* "$(abspath $(DESTDIR)$(PREFIX))/share/openmha/examples/"
	mkdir -p "$(abspath $(DESTDIR)$(PREFIX))/share/openmha/reference_algorithms"
	cp -r "reference_algorithms/"* "$(abspath $(DESTDIR)$(PREFIX))/share/openmha/reference_algorithms/"
install_mfiles: install_COPYING_openmha
	mkdir -p "$(abspath $(DESTDIR)$(PREFIX))/lib/openmha/mfiles/mhagui_PHL_generic_hearing_aid_callbacks" # literal "lib" and not $(LIB) on purpose
	cp "mha/tools/mfiles/"*.m "$(abspath $(DESTDIR)$(PREFIX))/lib/openmha/mfiles/"
	cp "mha/tools/mfiles/mhagui_PHL_generic_hearing_aid_callbacks/"*.m "$(abspath $(DESTDIR)$(PREFIX))/lib/openmha/mfiles/mhagui_PHL_generic_hearing_aid_callbacks/"
	cp "mha/tools/mfiles/mhactl_java.jar" "$(abspath $(DESTDIR)$(PREFIX))/lib/openmha/mfiles/"
	mkdir -p "$(abspath $(DESTDIR)$(PREFIX))/share/openmha"
	cp "mha/tools/mfiles/"*.mat "$(abspath $(DESTDIR)$(PREFIX))/share/openmha/"
	cp "mha/tools/mfiles/"*.cfg "$(abspath $(DESTDIR)$(PREFIX))/share/openmha/"
install_COPYING_%:
	mkdir -p "$(abspath $(DESTDIR)$(PREFIX))/share/doc/$*"
	rm -f "$(abspath $(DESTDIR)$(PREFIX))/share/doc/$*/COPYING"
	cp "COPYING" "$(abspath $(DESTDIR)$(PREFIX))/share/doc/$*/"

uninstall:
	find ./mha/plugins -type f -name "*$(DYNAMIC_LIB_EXT)" -execdir rm -vf $(abspath $(DESTDIR)$(PREFIX))/$(LIB)/{} \;
	rm -vf "$(abspath $(DESTDIR)$(PREFIX))/bin/mha" \
	      "$(abspath $(DESTDIR)$(PREFIX))/bin/analysemhaplugin" \
	      "$(abspath $(DESTDIR)$(PREFIX))/$(LIB)/libopenmha$(DYNAMIC_LIB_EXT)"
	rm -rvf "$(abspath $(DESTDIR)$(PREFIX))/share/doc/openmha"
	rm -rvf "$(abspath $(DESTDIR)$(PREFIX))/share/doc/libopenmha"
	rm -rvf "$(abspath $(DESTDIR)$(PREFIX))/share/doc/libopenmha-dev"
	rm -rvf "$(abspath $(DESTDIR)$(PREFIX))/share/doc/openmha-examples"
	rm -rvf "$(abspath $(DESTDIR)$(PREFIX))/include/openmha"
	rm -rvf "$(abspath $(DESTDIR)$(PREFIX))/share/openmha"
	rm -rvf "$(abspath $(DESTDIR)$(PREFIX))/lib/openmha" # literal "lib" and not $(LIB) on purpose

googletest:
	$(MAKE) -C external_libs googlemock

unit-tests: $(patsubst %,%-subdir-unit-tests,$(MODULES))
$(patsubst %,%-subdir-unit-tests,$(MODULES)): all googletest
	$(MAKE) -C $(@:-subdir-unit-tests=) unit-tests

coverage: unit-tests
	lcov --capture --directory mha --output-file coverage.info
	genhtml coverage.info --prefix $$PWD/mha --output-directory $@
	x-www-browser ./coverage/index.html

# Unit-test can not be run when cross-compiling
ifeq "$(ARCH)" "armhf"
deb: install
	$(MAKE) -C mha/tools/packaging/deb clean
	$(MAKE) -C mha/tools/packaging/deb pack
else
deb: unit-tests
	$(MAKE) -C mha/tools/packaging/deb clean
	$(MAKE) -C mha/tools/packaging/deb pack
endif

exe: installer-exe unit-tests
installer-exe: install
	$(MAKE) -C mha/tools/packaging/exe exe

homebrew: installer-homebrew
installer-homebrew: unit-tests
	$(MAKE) -C mha/tools/packaging/homebrew install

release: test unit-tests install
	@./mha/tools/release.sh openMHA # 'openMHA' is passed to prevent user from calling script accidentally

# Inter-module dependencies. Required for parallel building (e.g. make -j 4)
mha/libmha: external_libs
mha/frameworks: mha/libmha
mha/plugins: mha/libmha mha/frameworks
mha/mhatest: mha/plugins mha/frameworks
mha/doc: mha/plugins

# Debian package management by Jenkins:
# New Debian Packages are stored in our storage for debian repositories.
# The storage is cleaned of old packages depending on the current branch.
#
# Glossary:
#
# supply:
# The packages stored here have just been built. Location is
# project-specific. For openMHA, it is ./mha/tools/packaging/deb/hoertech/$SYSTEM/
# regardless of $BRANCH_NAME.
# The packages here may be new or they may be rebuilds of
# existing versions (e.g. if someone clicks build-now while
# there is no new revision). Rebuilds of existing versions
# will not be used, but will cause an update of the timestamp of the respective
# files in storage.  New packages are copied to storage
#
# storage:
# The packages here are kept persistently across builds.
# Old packets (timestamp older than some threshold) will be deleted for
# branch development.
# Storage location:
# /var/lib/jenkins/packages-for-aptly/STORAGE/$PROJECT/$BRANCH_NAME/$SYSTEM/
# on the host, which is mounted to /STORAGE/$PROJECT/$BRANCH_NAME/$SYSTEM/
# in the container.
#
#
# $PROJECT:
# openMHA, liblsl, liblsl-matlab, tascarpro, more may be added
#
# $BRANCH_NAME:
# master, uploaded to apt.hoertech.de, and development, uploaded to
# aptdev.hoertech.de. BRANCH_NAME is set by Jenkins
#
# SYSTEMs:
# focal, jessie, jammy etc. Available SYSTEMs that
# contain packages are detected automatically with $(wildcard)

# There will by $SYSTEM subdirectories below this directory.
# These subdirectories then contain the package files.
SUPPLY_DIR = ./mha/tools/packaging/deb/hoertech/

PROJECT = openMHA

# There will be $SYSTEM subdirectories below this directory.
STORAGE_DIR = /STORAGE/$(PROJECT)/$(BRANCH_NAME)/

# How many days to keep debian packages in storage that are superceded by a
# newer version
RETENTION = 14

storage: pruned-storage-$(BRANCH_NAME)

# Delete debian packages in storage older than RETENTION days
pruned-storage-%: updated-storage-%
	@echo uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu
	@echo Begin pruning storage...
	@echo nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn
	find $(STORAGE_DIR) -name "*.deb" -type f -mtime +$(RETENTION) -delete -print
	-rmdir $(STORAGE_DIR)/*   #  delete empty subdirs if there are any
	@echo uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu
	@echo Storage pruning finished.
	@echo nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn

# Never delete old packages in the master database
pruned-storage-master: updated-storage-master
	@echo uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu
	@echo "Keep all existing packages on branch master"
	@echo nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn

# copy the $SYSTEM/packages.deb tree without overwriting existing package files.
# then, update the timestamps for all files that are now in the supply to
# prevent deletion of latest files because they might be too old.
updated-storage-$(BRANCH_NAME):
	@echo uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu
	@echo Begin updating storage...
	@echo nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn
	mkdir -p $(STORAGE_DIR)
	cp -rnv $(SUPPLY_DIR)* $(STORAGE_DIR)
	cd $(SUPPLY_DIR) && find . -name \*.deb -exec touch $(STORAGE_DIR){} \;
	@echo uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu
	@echo Storage update finished.
	@echo nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn


# Local Variables:
# coding: utf-8-unix
# End:
