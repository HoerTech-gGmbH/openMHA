# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2014 2015 2016 2017 2018 2019 2020 HörTech gGmbH
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

# to be included by plugin specific Makefiles.

include ../../../config.mk
include ../../../magic.mk

SOURCE_DIR=.
PLUGINS = $(notdir $(abspath .))

ifeq "$(NEEDS_JACK)" "yes"
ifneq "$(WITH_JACK)" "yes"
# this plugin needs jack.
# Do not compile if jack not available.
# instead, execute this dummy rule. as default target
dummy01:
	@echo "not compiling" $(PLUGINS) "since jack is not available"
endif
endif

ifeq "$(EXCLUDE_FROM_ARM_COMPILATION)" "yes"
ifeq "$(findstring arm, $(ARCH))" "arm"
# this plugin cannot be compiled for ARM processors.
dummy02:
	@echo "not compiling" $(PLUGINS) "since it cannot be compiled for ARM"
endif
endif

ifeq "$(NEEDS_BOOST)" "yes"
ifneq "$(WITH_BOOST)" "yes"
# this plugin needs boost.
# Do not compile if boost not available.
# instead, execute this dummy rule. as default target
dummy03:
	@echo "not compiling" $(PLUGINS) "since boost is not available"
endif
endif

ifeq "$(NEEDS_ALSA)" "yes"
ifneq "$(WITH_ALSA)" "yes"
# this plugin needs alsa.
# Do not compile if alsa not available.
# instead, execute this dummy rule. as default target
dummy04:
	@echo "not compiling" $(PLUGINS) "since alsa is not available"
endif
endif

ifeq "$(NEEDS_GTKMM)" "yes"
ifneq "$(WITH_GTKMM)" "yes"
# this plugin needs the gtkmm library.
# Do not compile if gtkmm not available or not current enough.
# instead, execute this dummy rule. as default target
dummy_gtkmm:
	@echo "not compiling" $(PLUGINS) "since gtkmm is not available"
endif
endif

ifeq "$(NEEDS_FREENECT)" "yes"
ifneq "$(WITH_FREENECT)" "yes"
# this plugin needs the libfreenect library.
# Do not compile if freenect not available or not current enough.
# instead, execute this dummy rule. as default target
dummy_freenect:
	@echo "not compiling" $(PLUGINS) "since libfreenect is not available"
endif
endif

ifeq "$(NEEDS_LSL)" "yes"
ifneq "$(WITH_LSL)" "yes"
# this plugin needs lsl.
# Do not compile if lsl not available.
# instead, execute this dummy rule. as default target
dummy_lsl:
	@echo "not compiling" $(PLUGINS) "since lsl is not available"
endif
endif

ifeq "$(NEEDS_EIGEN)" "yes"
ifneq "$(WITH_EIGEN)" "yes"
# this plugin needs eigen3.
# Do not compile if eigen3 not available.
# instead, execute this dummy rule. as default target
dummy_eigen:
	@echo "not compiling" $(PLUGINS) "since eigen3 is not available"
endif
endif

ifeq "$(NEEDS_OSC)" "yes"
ifneq "$(WITH_OSC)" "yes"
# this plugin needs osc.
# Do not compile if osc not available.
# instead, execute this dummy rule. as default target
dummy_osc:
	@echo "not compiling" $(PLUGINS) "since osc is not available"
endif
endif

ifeq "$(NEEDS_LIBSERIAL)" "yes"
ifneq "$(WITH_LIBSERIAL)" "yes"
# this plugin needs libserial.
# Do not compile if libserial not available.
# instead, execute this dummy rule. as default target
dummy_libserial:
	@echo "not compiling" $(PLUGINS) "since libserial is not available"
endif
endif

ifeq "$(NEEDS_CXX17)" "yes"
ifeq ($(CXXSTANDARD),$(findstring $(CXXSTANDARD),"gnu++98c++98gnu++03c++03gnu++0xc++0xgnu++11c++11gnu++1yc++1ygnu++14c++14"))
# this plugin needs at least c++17.
# Do not compile if c++17 not available.
# instead, execute this dummy rule. as default target
dummy_cxx17:
	@echo "not compiling" $(PLUGINS) "since c++17 standard not available"
endif
endif

ifeq "$(EXCLUDE_FROM_WINDOWS_COMPILATION)" "yes"
ifeq "$(PLATFORM)" "MinGW"
# this plugin cannot be compiled on windows.
dummy05:
	@echo "not compiling" $(PLUGINS) "since it cannot be compiled on windows"
endif
endif

ifeq "$(EXCLUDE_FROM_MAC_OS_X_COMPILATION)" "yes"
ifeq "$(PLATFORM)" "Darwin"
# this plugin cannot be compiled on Max OS X.
dummy06:
	@echo "not compiling" $(PLUGINS) "since it cannot be compiled on mac os x"
endif
endif

ifeq "$(EXCLUDE_FROM_CLANG_COMPILATION)" "yes"
ifeq "$(TOOLSET)" "clang"
# this plugin cannot be compiled with the clang compiler.
dummy07:
	@echo "not compiling" $(PLUGINS) "since it cannot be compiled with clang"
endif
ifeq "$(PLATFORM)" "Darwin"
# this plugin cannot be compiled on Max OS X.
dummy07a:
	@echo "not compiling" $(PLUGINS) "since mac os x uses the clang compiler and the plugins cannot be compiled with clang"
endif
endif

ifeq "$(EXCLUDE_FROM_i686_COMPILATION)" "yes"
ifeq "$(findstring i686, $(ARCH))" "i686"
# this plugin cannot be compiled for 32 bit intel compatible processors.
dummy08:
	@echo "not compiling" $(PLUGINS) "since it cannot be compiled for i686"
endif
endif

include ../../../rules.mk

CXXFLAGS += -I../../../external_libs/$(PLATFORM_CC)/include
CXXFLAGS += -I../../libmha/src
CFLAGS += -I../../libmha/src
LDFLAGS += -L../../libmha/$(BUILD_DIR)
LDLIBS += -l$(MHATOOLBOX_NAME)

# Library-dependent plugin artifact rule specific modifications of compiler
# and linker flags need to have their own if statements because they must
# come after the include ../../../rules.mk block and the dummy targets must come
# before it.

ifeq "$(NEEDS_LSL)" "yes"
$(PLUGIN_AND_TEST_ARTIFACTS): LDLIBS += -llsl
endif


# Local Variables:
# coding: utf-8-unix
# End:
