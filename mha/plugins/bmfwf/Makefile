# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2023 2024 Hörzentrum Oldenburg gGmbH
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

# This plugin can only be compiled if libtorch is available in version recent
# enough to contain class c10::InferenceMode. Only the C++11-ABI version of 
# libtorch can be used.
NEEDS_TORCH=yes

include ../plugin.mk

# Silence warnings about unused parameters for this plugin.
CXXFLAGS += -Wno-unused-parameter

LDLIBS += $(TORCH_LIBS)

# If we use libtorch from openMHA docker build environment, then store the used
# libtorch shared libraries in a subdirectory of (only) this plugin directory
# for inclusion in the generated openMHA .deb package.
ifeq (openMHA,$(findstring openMHA, $(TORCH_LIBS)))
# Add targets to copy openMHA libtorch libraries to a subdirectory of the
# build directory.  There, they will be picked up by the packager, but will
# be ignored by the plugin documentation builder.
$(BUILD_DIR)/.directory: \
  $(BUILD_DIR)/openMHA_torch_libs/libtorch_cpu_openMHA.so \
  $(BUILD_DIR)/openMHA_torch_libs/libc10_openMHA.so \
  $(BUILD_DIR)/openMHA_torch_libs/libgomp_openMHA.so
$(BUILD_DIR)/openMHA_torch_libs/%.so: /usr/local/lib/%.so
	mkdir -p $(BUILD_DIR)/openMHA_torch_libs
	cp $< $@
endif
