# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2013 2014 2015 2016 2017 2018 2020 HörTech gGmbH
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

# This file defines how we invoke the build tools (gcc and clang) to
# build objects, shared libraries, plugins, and executables throughout
# the MHA project.  It is included by every Makefile in the
# subdirectories that actually builds something.

# To compile MHA for iOS, all "plugins" have to actually be statically linked
# into the App.
ifeq "$(MHA_STATIC_PLUGINS)" "yes"
PLUGIN_EXT = .a
else
PLUGIN_EXT = $(DYNAMIC_LIB_EXT)
endif

PLUGIN_ARTIFACTS = $(patsubst %,$(BUILD_DIR)/%$(PLUGIN_EXT),$(PLUGINS))
PLUGIN_AND_TEST_ARTIFACTS = $(PLUGIN_ARTIFACTS) $(BUILD_DIR)/unit-test-runner

# This is usually the first Makefile rule encountered in any
# subdirectory by inclusion of this Makefile.
all: $(BUILD_DIR)/.directory $(patsubst %,$(BUILD_DIR)/%,$(TARGETS)) $(PLUGIN_ARTIFACTS) $(SUBDIRS)

# BUILD_DIR is a compiler- and platform dependent subdirectory name
# for placing build output files

# Pattern for building object files from C++ sources - with headers
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp $(SOURCE_DIR)/%.hh $(BUILD_DIR)/.directory
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Pattern for building object files from C++ sources - w/o headers
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp $(BUILD_DIR)/.directory
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Pattern for building object files from C sources - with headers
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c $(SOURCE_DIR)/%.h $(BUILD_DIR)/.directory
	$(CC) $(CFLAGS) -c -o $@ $<

# Pattern for building object files from C sources - w/o headers
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c $(BUILD_DIR)/.directory
	$(CC) $(CFLAGS) -c -o $@ $<


# Pattern for building object containing the git commit hash for reproducibility
$(BUILD_DIR)/%_mha_git_commit_hash.o: $(GIT_DIR)/mha/libmha/src/mha_git_commit_hash.cpp $(BUILD_DIR)/.directory
	$(CXX) $(CXXFLAGS) $(GITCOMMITHASHCFLAGS) -c -o $@ $<

# Pattern for linking shared libraries and dynamic plugins
$(BUILD_DIR)/%$(DYNAMIC_LIB_EXT):
	$(CXX) -shared -o $$PWD/$@ $^ ${LDFLAGS} ${LDLIBS}
# Prepending outname with PWD sets install_name on Mac to absolute path
# (points inside sourcetree) for correct runtime linking of tools during tests

# Pattern for building static libraries and static "plugins"
$(BUILD_DIR)/%.a: $(BUILD_DIR)/%.o
	echo --  ${LDFLAGS} ${LDLIBS} >$@.libs
	mkdir -p $(BUILD_DIR)/objects
	cp $^ $(BUILD_DIR)/objects
	ar -rucs $@ $^

# Pattern for linking executables.  The STATIC_DLOPEN needs to be set for iOS.
$(BUILD_DIR)/%: $(BUILD_DIR)/%.o
	$(CXX) $(STATIC_DLOPEN) $(RPATH_FLAGS) -o $@ $^ ${LDFLAGS} ${LDLIBS}

# Pattern for subdirectories for build artifacts
%/.directory:
	mkdir -p $*
	touch $@

clean:
	rm -Rf $(BUILD_DIR)
	for m in $(SUBDIRS); do $(MAKE) -C $$m clean; done

unit-tests: execute-unit-tests $(patsubst %,%-subdir-unit-tests,$(SUBDIRS))

$(patsubst %,%-subdir-unit-tests,$(SUBDIRS)):
	$(MAKE) -C $(@:-subdir-unit-tests=) unit-tests

execute-unit-tests: $(BUILD_DIR)/unit-test-runner
	if [ -x $< ]; then $(call EXTEND_DLLPATH_$(PLATFORM),$(GIT_DIR)/mha/libmha/$(BUILD_DIR)) $<; fi

unit_tests_test_files = $(wildcard $(SOURCE_DIR)/*_unit_tests.cpp)

$(BUILD_DIR)/unit-test-runner: $(BUILD_DIR)/.directory $(unit_tests_test_files) $(patsubst %_unit_tests.cpp, %.cpp , $(unit_tests_test_files))
	if test -n "$(unit_tests_test_files)"; then $(CXX) $(CXXFLAGS) --coverage -o $@ $(wordlist 2, $(words $^), $^) $(LDFLAGS) $(LDLIBS) -lgmock_main -lpthread; fi

# Static Pattern Rule defines standard prerequisites for plugins
$(PLUGINS:%=$(BUILD_DIR)/%$(PLUGIN_EXT)): %$(PLUGIN_EXT): %.o %_mha_git_commit_hash.o

# Always dive into subdirectories
.PHONY : $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@

# Local Variables:
# coding: utf-8-unix
# End:
