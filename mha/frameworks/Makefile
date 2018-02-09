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

include ../../config.mk
include ../../magic.mk

TARGETS = mha analysemhaplugin generatemhaplugindoc

ifeq "linux" "$(PLATFORM)"
TARGETS += browsemhaplugins
ifeq "$(WITH_JAVA)" "yes"
ifeq "$(WITH_SWIG3)" "yes"
TARGETS += libMHAjava.so MHAjava.jar
endif
endif
endif

ifeq "$(WITH_ALSA)" "yes"
TARGETS += testalsadevice
endif

include ../../rules.mk

CXXFLAGS += -I../../external_libs/$(PLATFORM_CC)/include
CXXFLAGS += -I../libmha/src
LDLIBS += -l$(MHATOOLBOX_NAME)
LDFLAGS += -L../libmha/$(BUILD_DIR)

ifeq "linux" "$(PLATFORM)"
LDLIBS += -ldl -lpthread
else
ifeq "Darwin" "$(PLATFORM)"
LDLIBS += -ldl -lpthread
else
LDLIBS += -lws2_32
endif
endif

LDFLAGS += -L../../external_libs/$(PLATFORM_CC)/lib

OBJECTS = mhamain.o mha_tcp.o mhafw_lib.o

ifneq "$(TOOLSET)" "clang"
$(BUILD_DIR)/mha: LDFLAGS+=-Wl,--dynamic-list=export_fw_t.list
endif

$(BUILD_DIR)/mha: $(OBJECTS:%.o=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/testalsadevice: LDLIBS += -lasound

$(BUILD_DIR)/MHAjava.jar: fw_t.class MHA.class MHAJNI.class
	jar cfv $@ $^

%.class: src/%.java
	javac -cp src -d . $^

src/fw_t.java src/MHA.java src/MHAJNI.java src/fw_t_wrap.cxx: src/fw_t.ii
	swig3.0 -c++ -java src/fw_t.ii

$(BUILD_DIR)/libMHAfw.so: src/mhafw_lib.cpp
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LDLIBS) -shared -o $@ -I/usr/lib/jvm/default-java/include/ -I/usr/lib/jvm/default-java/include/linux -fno-strict-aliasing

$(BUILD_DIR)/libMHAjava.so: src/fw_t_wrap.cxx $(BUILD_DIR)/libMHAfw.so
	$(CXX) $(CXXFLAGS) $< -L$(BUILD_DIR) $(LDFLAGS) -lMHAfw $(LDLIBS) -shared -o $@ -I/usr/lib/jvm/default-java/include/ -I/usr/lib/jvm/default-java/include/linux -fno-strict-aliasing 


# Local Variables:
# compile-command: "make"
# End:
