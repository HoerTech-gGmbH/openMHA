# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2020 HörTech gGmbH
# Copyright © 2024 Hörzentrum Oldenburg gGmbH
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

# This Makefile illustrates how to compile and link a plugin against openMHA
# on an Ubuntu system with the libopenmha-dev package installed
# as well as under macOS systems on Intel and ARM Mac computers with openMHA
# installed via homebrew
# For more information on how to
# set up a build environment for openMHA see COMPILATION.md and your compiler
# and/or OS vendor's documentation. For more information on Makefiles, see
# GNU make's manual


# Ubuntu
-include /usr/share/openmha/config.mk
# Intel Macs 
-include /usr/local/share/openmha/config.mk
# ARM Macs 
-include /opt/homebrew/share/openmha/config.mk

# Include openmha headers
INCLUDES=-I/usr/include/openmha
# Intel Macs
INCLUDES=-I/usr/local/include/openmha
# ARM Macs:
INCLUDES=-I/opt/homebrew/include/openmha


# Link against libopenmha
LIBS=-lopenmha

# Default target is example21
all: example21

# Convenience target: Remove compiled products
clean:
	rm -f example21$(DYNAMIC_LIB_EXT)
# DYNAMIC_LIB_EXT is defined in config.mk. Its value is .so for Linux, .dll
# for Windows, and .dylib for Mac (always including the dot).

# Compile the plugin
example21: example21.cpp
	$(CXX) -shared -o example21$(DYNAMIC_LIB_EXT) $(CXXFLAGS) $(INCLUDES) $(LIBS) $<
