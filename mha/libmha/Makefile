# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2013 2014 2016 2017 2018 2019 2020 HörTech gGmbH
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

include ../../config.mk
include ../../magic.mk

libmha = lib$(MHATOOLBOX_NAME)

TARGETS=$(libmha)$(DYNAMIC_LIB_EXT)

include ../../rules.mk
-include ./private.mk

OBJECTS = \
	mha_parser.o mha_error.o mha_errno.o \
	mha_profiling.o mha_signal.o mha_algo_comm.o \
	mha_filter.o complex_filter.o mha_tablelookup.o mha_fftfb.o \
	mha_events.o mha_os.o \
	mhasndfile.o \
	mha_multisrc.o \
	mhapluginloader.o \
	mha_windowparser.o \
	speechnoise.o \
	auditory_profile.o \
	gaintable.o \
	dc_afterburn.o \
	windowselector.o \
	mha_fifo.o \
	pluginbrowser.o \
	mha_utils.o \

LDFLAGS += -L../../external_libs/$(PLATFORM_CC)/lib

ifeq "$(PLATFORM)" "linux"
LDLIBS += -ldl -lpthread -lsndfile
else
ifeq "$(PLATFORM)" "Darwin"
LDLIBS += -lsndfile
else
LDLIBS += -lsndfile -lws2_32
endif
endif

FFTW_LIB = ../../external_libs/$(PLATFORM_CC)/lib/libsfftw.a
RFFTW_LIB = ../../external_libs/$(PLATFORM_CC)/lib/libsrfftw.a
MHA_FFTW_OBJECTS = $(shell ar t $(FFTW_LIB) | grep -v _.SYMDEF)
MHA_RFFTW_OBJECTS = $(shell ar t $(RFFTW_LIB) | grep -v _.SYMDEF)

OBJECTS += $(MHA_FFTW_OBJECTS) $(MHA_RFFTW_OBJECTS)

CXXFLAGS += -I../../external_libs/$(PLATFORM_CC)/include

$(BUILD_DIR)/$(libmha)$(DYNAMIC_LIB_EXT): $(OBJECTS:%.o=$(BUILD_DIR)/%.o)

ifeq "MinGW" "$(PLATFORM)"
LDLIBS += -Wl,--out-implib,$(basename $@).a
endif

$(MHA_FFTW_OBJECTS:%.o=$(BUILD_DIR)/%.o): $(FFTW_LIB) $(BUILD_DIR)/.directory
	ar x $< $(notdir $@) && mv $(notdir $@) $@

$(MHA_RFFTW_OBJECTS:%.o=$(BUILD_DIR)/%.o): $(RFFTW_LIB) $(BUILD_DIR)/.directory
	ar x $< $(notdir $@) && mv $(notdir $@) $@

ifeq "$(PLATFORM)" "linux"
FIND_LIBOPENMHA_RPATH = -Wl,-rpath=./$(BUILD_DIR)
endif
# Different rule than normal to build unit-test-runner in this
# directory: we do link against libopenmha, and we do not list the
# files under test on the compiler command line, because they are
# already in the lib and would duplicate all the classes. Make the lib
# an explicit dependency.
$(BUILD_DIR)/unit-test-runner: $(unit_tests_test_files) $(BUILD_DIR)/$(libmha)$(DYNAMIC_LIB_EXT)
	@echo dependencies = $^
	$(CXX) $(CXXFLAGS) --coverage -o $@ $(unit_tests_test_files) $(LDFLAGS) $(LDLIBS) $(BUILD_DIR)/$(libmha)$(DYNAMIC_LIB_EXT) -lgmock_main -lpthread

# Local Variables:
# coding: utf-8-unix
# End:
