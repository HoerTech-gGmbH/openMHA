# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2005 2006 2013 HörTech gGmbH
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

DOC_SRC = $(wildcard *.c) $(wildcard *.cpp)
DOC_OBJ = $(patsubst %,%.o,$(basename $(DOC_SRC)))
DOC_DEP = $(patsubst %,%.dep,$(basename $(DOC_SRC)))
CPPFLAGS = -Wall -I../../libmha/src
CFLAGS = -Wall -I../../libmha/src
CXXFLAGS = -Wall -I../../libmha/src

all: $(DOC_OBJ)

include $(DOC_DEP)

%.dep: %.c
	$(CPP) $(CPPFLAGS) -MM -MF temp_dep.mk $<
	cat temp_dep.mk | sed 's/\([^:]*\)\.o/\1.dep/g' > $@
	echo "" >> $@
	cat temp_dep.mk >> $@
	rm -f temp_dep.mk

%.dep: %.cpp
	$(CPP) $(CPPFLAGS) -MM -MF temp_dep.mk $<
	cat temp_dep.mk | sed 's/\([^:]*\)\.o/\1.dep/g' > $@
	echo "" >> $@
	cat temp_dep.mk >> $@
	rm -f temp_dep.mk

# Local Variables:
# coding: utf-8-unix
# End:
