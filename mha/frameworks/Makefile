# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2013 2014 2015 2016 2017 2018 2019 2020 HörTech gGmbH
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

ifeq "x${BRANCH_NAME}" "xmaster" # If we are in a Jenkins build of branch master
ifeq "x" "x$(findstring modified,$(GITCOMMITHASH))" # and if git is unmodified,
# then this is is a relase build, do not output a non-release warning.
CXXFLAGS += -DNORELEASE_WARNING='""'
# and also add git commit hash to version output for a relase build
CXXFLAGS += $(GITCOMMITHASHCFLAGS) -DVERSION_EXTENSION='" (" GITCOMMITHASH ")"'
endif
endif

TARGETS = mha analysemhaplugin generatemhaplugindoc

ifeq "linux" "$(PLATFORM)"
TARGETS += browsemhaplugins
ifeq "$(WITH_RUBY)" "yes"
TARGETS += mha_ruby.so
endif
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
CXXFLAGS += -I../libmha/src -DASIO_STANDALONE
LDLIBS += -l$(MHATOOLBOX_NAME)
LDFLAGS += -L../libmha/$(BUILD_DIR)

ifeq "linux" "$(PLATFORM)"
LDLIBS += -ldl -lpthread
else
ifeq "Darwin" "$(PLATFORM)"
LDLIBS += -ldl -lpthread
else
LDLIBS += -lmswsock -lws2_32
endif
endif

LDFLAGS += -L../../external_libs/$(PLATFORM_CC)/lib

OBJECTS = mhamain.o mha_tcp_server.o mhafw_lib.o

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

$(BUILD_DIR)/mha_ruby.so: src/mha_ruby.cpp $(BUILD_DIR)/libMHAfw.so
	$(CXX) $(CXXFLAGS) $< -L$(BUILD_DIR) $(LDFLAGS) $$(ruby -rrbconfig -e 'C=RbConfig::CONFIG;printf("-I%s -I%s -L%s %s",C["rubyhdrdir"],C["rubyarchhdrdir"],C["libdir"],C["LIBRUBYARG"])') -lMHAfw $(LDLIBS) -shared -o $@  -fno-strict-aliasing 

# The asio library headers trigger this warning for timer object template
# instantiations used in the mha_tcp_server.  Disable the
# warning-as-error for duplicated branches.
$(BUILD_DIR)/unit-test-runner $(BUILD_DIR)/mha_tcp_server.o: CXXFLAGS := \
     $(patsubst \
       -Wduplicated-branches,\
       -Wduplicated-branches -Wno-error=duplicated-branches,\
       $(CXXFLAGS))

# Local Variables:
# compile-command: "make"
# coding: utf-8-unix
# End:
