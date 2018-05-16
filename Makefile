# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2013 2014 2015 2016 2017 2018 HörTech gGmbH
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
	mha/doc/flowcharts \
  mha/doc/images \
	mha/doc \

all: $(MODULES)

test: all
	$(MAKE) -C mha/mhatest

.PHONY : $(MODULES) $(DOCMODULES) coverage

$(MODULES:external_libs=) $(DOCMODULES):
	$(MAKE) -C $@

external_libs:
	$(MAKE) -C $@

doc: mha/doc
	cp -i mha/doc/*.pdf .

clean:
	for m in $(MODULES) $(DOCMODULES); do $(MAKE) -C $$m clean; done

install: all
	@mkdir -p  $(DESTDIR)$(PREFIX)/bin
	@mkdir -p  $(DESTDIR)$(PREFIX)/lib
	@find ./external_libs/ ./mha/ -type f -name *$(DYNAMIC_LIB_EXT) -exec cp {} $(DESTDIR)$(PREFIX)/lib/ \;
	@find ./mha/frameworks/${BUILD_DIR} -type f ! -name "*.o" -exec cp {} $(DESTDIR)$(PREFIX)/bin/ \;
	@cp mha/tools/thismha.sh $(DESTDIR)$(PREFIX)/bin/.

uninstall:

	@rm -f $(shell find ./external_libs/ ./mha/ -type f -name *$(DYNAMIC_LIB_EXT) -execdir echo $(DESTDIR)$(PREFIX)/lib/{} \;)
	@rm -f $(shell find ./mha/frameworks/${BUILD_DIR} -type f ! -name "*.o" -execdir echo $(DESTDIR)$(PREFIX)/bin/{} \;)
	@rm -f $(DESTDIR)$(PREFIX)/bin/mha.sh


googletest: test
	$(MAKE) -C external_libs googlemock

unit-tests: $(patsubst %,%-subdir-unit-tests,$(MODULES))
$(patsubst %,%-subdir-unit-tests,$(MODULES)): all googletest
	$(MAKE) -C $(@:-subdir-unit-tests=) unit-tests

coverage: unit-tests
	lcov --capture --directory mha --output-file coverage.info
	genhtml coverage.info --prefix $$PWD/mha --output-directory $@
	x-www-browser ./coverage/index.html

deb: unit-tests
	$(MAKE) -C mha/tools/packaging/deb pack


# Inter-module dependencies. Required for parallel building (e.g. make -j 4)
mha/libmha: external_libs
mha/frameworks: mha/libmha
mha/plugins: mha/libmha mha/frameworks
mha/mhatest: mha/plugins mha/frameworks
mha/doc: mha/doc/images all
mha/doc/images: mha/doc/flowcharts

# Local Variables:
# coding: utf-8-unix
# End:
