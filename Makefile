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

ifeq "$(WITH_LSL)" "yes"
all: examples_with_Makefiles
endif
examples_with_Makefiles:
	for e in 12 19 30; do $(MAKE) -C examples/"$$e"-* || exit 1; done

.PHONY : $(MODULES) $(DOCMODULES) coverage examples

$(MODULES:external_libs=) $(DOCMODULES):
	$(MAKE) -C $@

external_libs:
	$(MAKE) -C $@

doc: mha/doc
	/bin/cp -lv --remove-destination mha/doc/*.pdf .
	zip -r pdf-$$(cat version).zip *.pdf

clean:
	for m in $(MODULES) $(DOCMODULES); do $(MAKE) -C $$m clean; done

ifeq "$(PLATFORM)" "Darwin"
INSTALL_NAME_TOOL=install_name_tool
else
INSTALL_NAME_TOOL=true
endif
INSTALL_PARTS = yes
install: INSTALL_PARTS = no
install: install_libopenmha install_openmha install_libopenmha-dev install_openmha-examples
install_openmha: install_plugins install_executables install_manuals install_mfiles

install_binaries: install_libopenmha install_plugins install_executables

install_libopenmha: mha/libmha install_COPYING_libopenmha
	mkdir -p "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_LIB)"
	rm -f "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_LIB)/libopenmha$(DYNAMIC_LIB_EXT)"
	cp "mha/libmha/$(BUILD_DIR)/libopenmha$(DYNAMIC_LIB_EXT)" "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_LIB)/"
	$(INSTALL_NAME_TOOL) -id $(PREFIX)$(INSTDIR_LIB)/libopenmha$(DYNAMIC_LIB_EXT) \
	                       $(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_LIB)/libopenmha$(DYNAMIC_LIB_EXT)
	mkdir -p "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_MISC)"
	cp config.mk "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_MISC)/"
install_plugins: mha/plugins install_COPYING_openmha
	mkdir -p "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_LIB)"
	# Find all compiled plugins except for example1 .. example7
	find mha/plugins/ -path "*example[1-7]*" -prune -o -type f -name "*$(DYNAMIC_LIB_EXT)" \
	    -execdir rm -f $(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_LIB)/{} \; \
	    -exec cp -v {} $(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_LIB)/ \; \
	    -execdir $(INSTALL_NAME_TOOL) -change "$(shell pwd)/mha/libmha/$(BUILD_DIR)/libopenmha$(DYNAMIC_LIB_EXT)" \
	                                   "$(PREFIX)$(INSTDIR_LIB)/libopenmha$(DYNAMIC_LIB_EXT)" \
	                                   "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_LIB)/{}" \; \
	    -execdir $(INSTALL_NAME_TOOL) -id "$(PREFIX)$(INSTDIR_LIB)/{}" \
	                                   "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_LIB)/{}" \;
install_executables: install_executable_mha install_executable_analysemhaplugin
install_executable_%: mha/frameworks install_COPYING_openmha
	mkdir -p "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_BIN)"
	rm -f "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_BIN)/$*"
	cp "mha/frameworks/$(BUILD_DIR)/$*" "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_BIN)/"
	$(INSTALL_NAME_TOOL) -change "$(shell pwd)/mha/libmha/$(BUILD_DIR)/libopenmha$(DYNAMIC_LIB_EXT)" \
	                       "$(PREFIX)$(INSTDIR_LIB)/libopenmha$(DYNAMIC_LIB_EXT)" \
	                       "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_BIN)/$*"
ifeq "$(BUILD_DOCS)" "yes"
install_manuals: doc install_COPYING_openmha
	mkdir -p "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_DOC)"
	for pdf in *.pdf; do \
	    rm -f "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_DOC)/$$pdf"; \
	    cp -v "$$pdf" "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_DOC)/"; \
	done
else
install_manuals:
	@echo "Warning: Documentation not installed. Please download from https://www.openmha.org/."
endif
install_libopenmha-dev: install_COPYING_libopenmha-dev
	mkdir -p "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_INCLUDE)"
	for h in mha/libmha/src/*.h mha/libmha/src/*.hh; do \
	    filename_without_directory=$$(basename "$$h"); \
	    rm -f "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_INCLUDE)/$$filename_without_directory"; \
	    cp -v "$$h" "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_INCLUDE)/"; \
	done
install_openmha-examples: install_COPYING_openmha-examples
	mkdir -p "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_MISC)/examples"
	cp -r "examples/"* "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_MISC)/examples/"
	mkdir -p "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_MISC)/reference_algorithms"
	cp -r "reference_algorithms/"* "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_MISC)/reference_algorithms/"
	basedir=$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_MISC)/plugin_development && \
	cd mha/plugins && \
	for source in example*/*.* ; \
	do \
		dir="$$(dirname "$$source")" && \
		mkdir -p "$$basedir/$$dir" && \
	    rm -f "$$basedir/$$source" && \
	    cp -v "$$source" "$$basedir/$$dir/" ; \
	done
install_mfiles: install_COPYING_openmha
	mkdir -p "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_MFILES)/mhagui_PHL_generic_hearing_aid_callbacks" # literal "lib" and not $(LIB) on purpose
	cp "mha/tools/mfiles/"*.* "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_MFILES)/"
	cp "mha/tools/mfiles/mhagui_PHL_generic_hearing_aid_callbacks/"*.m "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_MFILES)/mhagui_PHL_generic_hearing_aid_callbacks/"
install_COPYING_%:
	if [ "x$(INSTALL_PARTS)" = "xyes" ]; then \
		mkdir -p "$(abspath $(DESTDIR)$(PREFIX))/share/doc/$*"; \
		rm -f "$(abspath $(DESTDIR)$(PREFIX))/share/doc/$*/COPYING"; \
		cp "COPYING" "$(abspath $(DESTDIR)$(PREFIX))/share/doc/$*/"; \
	else \
	  if [ "x$*" = "xopenmha" ]; then \
		mkdir -p "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_MISC)"; \
		rm -f "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_MISC)/COPYING"; \
		cp "COPYING" "$(abspath $(DESTDIR)$(PREFIX))$(INSTDIR_MISC)/"; \
	  fi \
	fi

# The following target performs a make install for Windows.
# DESTDIR *must* be set to an empty directory. After calling make install,
# this rule searches DLLs linked by executables and DLLs in
# $(DESTDIR)$(PREFIX)/bin and copies them also to $(DESTDIR)$(PREFIX)/bin.
install_deps_windows: install
	depencencies_script="$(abspath windows_$(MSYSTEM)_find_and_copy_dependencies.sh)" && \
	cd "$(abspath $(DESTDIR)$(PREFIX))/bin" && \
	$$depencencies_script

googletest:
	$(MAKE) -C external_libs googlemock

unit-tests: $(patsubst %,%-subdir-unit-tests,$(MODULES))
$(patsubst %,%-subdir-unit-tests,$(MODULES)): all googletest
	$(MAKE) -C $(@:-subdir-unit-tests=) unit-tests

coverage: unit-tests
	lcov --capture --directory mha --output-file coverage.info
	genhtml coverage.info --prefix $$PWD/mha --output-directory $@
	x-www-browser ./coverage/index.html

# We create 4 different debian packages: openmha, libopenmha, libopenmha-dev
# and openmha-examples To create the debian package openmha, e.g., call
# make deb_openmha or make deb_openmha_debversion.
# The version of the debian package is $(cat version)-$(lsb_release -rs).
# If debversion was given, then this is appended to the version.
# Some fields in the stored control files will be extended or added like this;
# ALL OUR CONTROL FILES HAVE Depends: AS THEIR LAST LINE, WITH NO NEWLINE !!!
# This Makefile rule first copies the control file, then appends individual
# dependency entries, as they may differ from one Ubuntu release to the next,
# or by target architecture, and then appends Architecture:, Package: and
# Version: fields.
deb_%:
	echo "Called target $@. dollar-star is $*."
	packageversion="$$(cat version)-$$(lsb_release -rs)$$(echo "$*"_ | cut -d_ -f2)" && \
	echo "Package version is $$packageversion." && \
	packagename="$$(echo "$*"_ | cut -d_ -f1)" && \
	echo "Package name is $$packagename." && \
	echo "$$packagename" | grep -q openmh.*[^l]$ || (echo "Error: wrong debian package name" && exit 1) && \
	rm -rf "debian_$$packagename" "$$packagename"_*.deb && \
	"$(MAKE)" DESTDIR="debian_$$packagename/" "install_$$packagename" && \
	mkdir -p "debian_$$packagename/DEBIAN" && \
	cp "$$packagename.control" "debian_$$packagename/DEBIAN/control" && \
	for d in $$(cat openmha-packages/additional_$${packagename}_dependencies.txt); \
	do echo -n , $$d >> "debian_$$packagename/DEBIAN/control"; done && \
	echo >> "debian_$$packagename/DEBIAN/control" && \
	ARCH=$(shell dpkg-architecture --query DEB_BUILD_ARCH) && \
	echo "Architecture: $$ARCH" >> "debian_$$packagename/DEBIAN/control" && \
	echo "Package: $$packagename" >> "debian_$$packagename/DEBIAN/control" && \
	echo "Version: $$packageversion" >> "debian_$$packagename/DEBIAN/control" && \
	/usr/bin/fakeroot dpkg-deb -Zxz --build "debian_$$packagename" "$${packagename}_$${packageversion}_$${ARCH}.deb"
	rm -rf "debian_$$packagename"

homebrew: unit-tests install

release: test unit-tests install
	@./mha/tools/release.sh openMHA # 'openMHA' is passed to prevent user from calling script accidentally

# Inter-module dependencies. Required for parallel building (e.g. make -j 4)
mha/libmha: external_libs
mha/frameworks: mha/libmha
mha/plugins: mha/libmha mha/frameworks
mha/mhatest: mha/plugins mha/frameworks
mha/doc: mha/plugins

# Local Variables:
# coding: utf-8-unix
# End:
