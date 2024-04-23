# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2013 2014 2015 2016 2017 2018 2019 2020 2021 HörTech gGmbH
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

include ../../config.mk
include ../../magic.mk

# Compile git commit hash into executables.
CXXFLAGS += $(GITCOMMITHASHCFLAGS)

ifeq "x${BRANCH_NAME}" "xmaster" # If we are in a Jenkins build of branch master
ifeq "x" "x$(findstring modified,$(GITCOMMITHASH))" # and if git is unmodified,
# then this is is a relase build, do not output a non-release warning ...
CXXFLAGS += -DNORELEASE_WARNING='""'
# and also add git commit hash to version output for a relase build.
CXXFLAGS += -DVERSION_EXTENSION='" (" GITCOMMITHASH ")"'
endif
endif

ifneq "x$(HOMEBREW_OPENMHA_TAG)" "x" # If we are building for Homebrew
ifeq "x" "x$(findstring modified,$(GITCOMMITHASH))" # and if git is unmodified,
# then this is is a relase build, do not output a non-release warning ...
CXXFLAGS += -DNORELEASE_WARNING='""'
# and also add git commit hash to version output for a relase build ...
CXXFLAGS += -DVERSION_EXTENSION='" (" GITCOMMITHASH ")"'
# and add build info for homebrew builds (build host, tag = homebrew version).
CXXFLAGS += -DHOMEBREW_OPENMHA_TAG='"$(HOMEBREW_OPENMHA_TAG)"'
CXXFLAGS += -DHOMEBREW_BUILD_HOST='"$(shell hostname)"'
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

$(BUILD_DIR)/analysemhaplugin: $(BUILD_DIR)/mhafw_lib.o $(BUILD_DIR)/frameworks_mha_git_commit_hash.o

$(BUILD_DIR)/generatemhaplugindoc: $(BUILD_DIR)/mhafw_lib.o $(BUILD_DIR)/frameworks_mha_git_commit_hash.o

$(BUILD_DIR)/mha: $(OBJECTS:%.o=$(BUILD_DIR)/%.o) $(BUILD_DIR)/frameworks_mha_git_commit_hash.o

$(BUILD_DIR)/testalsadevice: LDLIBS += -lasound
$(BUILD_DIR)/testalsadevice: $(BUILD_DIR)/testalsadevice.o  $(BUILD_DIR)/frameworks_mha_git_commit_hash.o
$(BUILD_DIR)/browsemhaplugins: $(BUILD_DIR)/browsemhaplugins.o  $(BUILD_DIR)/frameworks_mha_git_commit_hash.o

$(BUILD_DIR)/MHAjava.jar: fw_t.class MHA.class MHAJNI.class
	jar cfv $@ $^

%.class: src/%.java
	javac -cp src -d . $^

src/fw_t.java src/MHA.java src/MHAJNI.java src/fw_t_wrap.cxx: src/fw_t.ii
	swig3.0 -c++ -java src/fw_t.ii

$(BUILD_DIR)/libMHAfw.so: src/mhafw_lib.cpp $(BUILD_DIR)/frameworks_mha_git_commit_hash.o
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LDLIBS) -shared -o $@ -I/usr/lib/jvm/default-java/include/ -I/usr/lib/jvm/default-java/include/linux -fno-strict-aliasing

$(BUILD_DIR)/libMHAjava.so: src/fw_t_wrap.cxx $(BUILD_DIR)/frameworks_mha_git_commit_hash.o $(BUILD_DIR)/libMHAfw.so
	$(CXX) $(CXXFLAGS) $< $(BUILD_DIR)/frameworks_mha_git_commit_hash.o -L$(BUILD_DIR) $(LDFLAGS) -lMHAfw $(LDLIBS) -shared -o $@ -I/usr/lib/jvm/default-java/include/ -I/usr/lib/jvm/default-java/include/linux -fno-strict-aliasing 

$(BUILD_DIR)/mha_ruby.so: src/mha_ruby.cpp $(BUILD_DIR)/frameworks_mha_git_commit_hash.o $(BUILD_DIR)/libMHAfw.so
	$(CXX) $(CXXFLAGS) $< $(BUILD_DIR)/frameworks_mha_git_commit_hash.o -L$(BUILD_DIR) $(LDFLAGS) $$(ruby -rrbconfig -e 'C=RbConfig::CONFIG;printf("-I%s -I%s -L%s %s",C["rubyhdrdir"],C["rubyarchhdrdir"],C["libdir"],C["LIBRUBYARG"])') -lMHAfw $(LDLIBS) -shared -o $@  -fno-strict-aliasing 

# Local Variables:
# compile-command: "make"
# coding: utf-8-unix
# End:
