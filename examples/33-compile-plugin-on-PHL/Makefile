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
# on the on the Portable Hearing Laboratory running  mahalia 4.18.0-r0 or later.
# For more information on Makefiles, see GNU make's manual.


include /usr/share/openmha/config.mk

# Include openmha headers
INCLUDES=-I/usr/include/openmha

# Link against libopenmha
LIBS=-lopenmha

# Explicitely use g++-7
CXX=g++-7

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
