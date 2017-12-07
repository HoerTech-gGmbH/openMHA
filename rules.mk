# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2013 2014 2015 2016 2017 HörTech gGmbH
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

# This is usually the first Makefile rule encountered in any
# subdirectory by inclusion of this Makefile.
all: $(BUILD_DIR)/.directory $(patsubst %,$(BUILD_DIR)/%,$(TARGETS)) $(patsubst %,$(BUILD_DIR)/%$(PLUGIN_EXT),$(PLUGINS))  $(SUBDIRS)

# BUILD_DIR is a compiler- and platform dependent subdirectory name
# for placing build output files

# Pattern for building object files from C++ sources
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp $(BUILD_DIR)/.directory
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Pattern for building object files from C sources
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c $(BUILD_DIR)/.directory
	$(CC) $(CFLAGS) -c -o $@ $<

# Pattern for linking shared libraries and dynamic plugins
$(BUILD_DIR)/%$(DYNAMIC_LIB_EXT):
	$(CXX) -shared -o $@ $^ ${LDFLAGS} ${LDLIBS}

# Pattern for building static libraries and static "plugins"
$(BUILD_DIR)/%.a: $(BUILD_DIR)/%.o
	echo --  ${LDFLAGS} ${LDLIBS} >$@.libs
	mkdir -p $(BUILD_DIR)/objects
	cp $^ $(BUILD_DIR)/objects
	ar -rucs $@ $^

# Pattern for linking executables.  The STATIC_DLOPEN needs to be set for iOS.
$(BUILD_DIR)/%: $(BUILD_DIR)/%.o
	$(CXX) $(STATIC_DLOPEN) -o $@ $^ ${LDFLAGS} ${LDLIBS}

# Pattern for subdirectories for build artifacts
%/.directory:
	mkdir -p $*
	touch $@

clean:
	rm -Rf $(BUILD_DIR)
	for m in $(SUBDIRS); do $(MAKE) -C $$m clean; done

# Static Pattern Rule defines standard prerequisites for plugins
$(PLUGINS:%=$(BUILD_DIR)/%$(PLUGIN_EXT)): %$(PLUGIN_EXT): %.o

# Always dive into subdirectories
.PHONY : $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@

# Local Variables:
# coding: utf-8-unix
# End:
