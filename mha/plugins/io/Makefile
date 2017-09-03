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

include ../../../config.mk
include ../../../magic.mk

#SUBDIRS = jack_client

PLUGINS = MHAIOFile MHAIOParser MHAIOPortAudio MHAIOTCP

ifeq "$(WITH_JACK)" "yes"
PLUGINS += MHAIOJack
endif

include ../../../rules.mk

CXXFLAGS += -I../../../external_libs/$(PLATFORM_CC)/include
CXXFLAGS += -I../../libmha/src
CFLAGS += -I../../../external_libs/$(PLATFORM_CC)/include
CFLAGS += -I../../libmha/src
LDFLAGS += -L../../libmha/$(BUILD_DIR) -L../../../external_libs/$(PLATFORM_CC)/lib
LDLIBS += -l$(MHATOOLBOX_NAME)

$(BUILD_DIR)/MHAIOJack$(PLUGIN_EXT): $(BUILD_DIR)/mhajack.o
$(BUILD_DIR)/MHAIOJack$(PLUGIN_EXT): LDLIBS += $(JACK_LINKER_COMMAND)
ifneq "$(TOOLSET)" "clang"
$(BUILD_DIR)/mhajack.o $(BUILD_DIR)/MHAIOJack.o $(BUILD_DIR)/MHAIOJackdb.o $(BUILD_DIR)/test_jack_server.o: CXXFLAGS += -std=gnu++11
endif

$(BUILD_DIR)/MHAIOJackdb$(PLUGIN_EXT): $(BUILD_DIR)/mhajack.o
$(BUILD_DIR)/MHAIOJackdb$(PLUGIN_EXT): LDLIBS += $(JACK_LINKER_COMMAND)

$(BUILD_DIR)/MHAIOFile$(PLUGIN_EXT): LDFLAGS += -L../../../external_libs/$(PLATFORM_CC)/lib
$(BUILD_DIR)/MHAIOFile$(PLUGIN_EXT): LDLIBS += -lsndfile

$(BUILD_DIR)/MHAIOPortAudio$(PLUGIN_EXT): LDLIBS += -lportaudio

ifeq ($(PLATFORM),MinGW)
$(BUILD_DIR)/MHAIOTCP$(PLUGIN_EXT): LDLIBS += -lws2_32
endif

# Local Variables:
# compile-command: "make"
# coding: utf-8-unix
# End:
