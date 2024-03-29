# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2013 2014 2015 2016 2017 2018 2019 2020 2021 HörTech gGmbH
# Copyright © 2022 Hörzentrum Oldenburg gGmbH
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

PLUGINS = MHAIODummy MHAIOFile MHAIOParser MHAIOPortAudio MHAIOTCP MHAIOAsterisk

ifeq "$(WITH_JACK)" "yes"
PLUGINS += MHAIOJack MHAIOJackdb
endif

ifeq "$(WITH_ALSA)" "yes"
PLUGINS +=  MHAIOalsa
endif

include ../../../rules.mk

CXXFLAGS +=-I../../libmha/src
CFLAGS +=-I../../libmha/src

LDFLAGS += -L../../libmha/$(BUILD_DIR) -L../../../external_libs/$(PLATFORM_CC)/lib
LDLIBS += -l$(MHATOOLBOX_NAME)

$(BUILD_DIR)/MHAIOJack$(PLUGIN_EXT): $(BUILD_DIR)/mhajack.o
$(BUILD_DIR)/MHAIOJack$(PLUGIN_EXT): LDLIBS += $(JACK_LINKER_COMMAND)

$(BUILD_DIR)/MHAIOJackdb$(PLUGIN_EXT): $(BUILD_DIR)/mhajack.o
$(BUILD_DIR)/MHAIOJackdb$(PLUGIN_EXT): LDLIBS += $(JACK_LINKER_COMMAND)

$(BUILD_DIR)/MHAIOFile$(PLUGIN_EXT): LDFLAGS += -L../../../external_libs/$(PLATFORM_CC)/lib
$(BUILD_DIR)/MHAIOFile$(PLUGIN_EXT): LDLIBS += -lsndfile

$(BUILD_DIR)/MHAIOPortAudio$(PLUGIN_EXT): LDLIBS += -lportaudio

$(BUILD_DIR)/MHAIOalsa$(PLUGIN_EXT): $(BUILD_DIR)/mha_io_utils.o
$(BUILD_DIR)/MHAIOalsa$(PLUGIN_EXT): LDLIBS += -lasound

$(BUILD_DIR)/MHAIOTCP$(PLUGIN_EXT) $(BUILD_DIR)/MHAIOAsterisk$(PLUGIN_EXT): $(BUILD_DIR)/mha_tcp.o
ifeq ($(PLATFORM),MinGW)
$(BUILD_DIR)/MHAIOTCP$(PLUGIN_EXT) $(BUILD_DIR)/MHAIOAsterisk$(PLUGIN_EXT): LDLIBS += -lws2_32
endif

# Local Variables:
# compile-command: "make"
# coding: utf-8-unix
# End:
