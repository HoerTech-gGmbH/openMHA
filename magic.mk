# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2013 2014 2015 2016 2017 2018 HörTech gGmbH
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

# This file sets some Makefile variables based on the settings of the
# build system specific found in config.mk

# MHA can be compiled with either GCC or Clang.  We recommend that gcc
# and clang compiler drivers have version suffixes.  The
# COMPILERPREFIX setting is for cross compilation (we do this for ARM)
CC := $(COMPILERPREFIX)gcc$(GCC_VER)
CXX := $(COMPILERPREFIX)g++$(GCC_VER)
PLATFORM_CC = $(ARCH)-$(PLATFORM)-gcc$(GCC_VER)

ifeq "$(TOOLSET)" "clang"
CC := $(COMPILERPREFIX)clang$(CLANG_VER)
CXX := $(COMPILERPREFIX)clang++$(CLANG_VER)
PLATFORM_CC = $(ARCH)-$(PLATFORM)-clang$(CLANG_VER)
ifeq "$(PLATFORM)" "Darwin"
RPATH_FLAGS += -rpath @executable_path/../lib
endif
endif

# iOS does not support dynamic plugins
ifeq "$(MHA_STATIC_PLUGINS)" "yes"
CFLAGS += -DMHA_STATIC_PLUGINS=1
CXXFLAGS += -DMHA_STATIC_PLUGINS=1
endif

# Default linker option for Jack if not set otherwise in config.mk
ifeq "x$(JACK_LINKER_COMMAND)" "x"
JACK_LINKER_COMMAND = -ljack
endif

# Standard source code subdirectory.  May be overwritten by most
# nested Makefile.  Used in rules.mk.
SOURCE_DIR = src

# The git commit SHA is compiled into the binaries for reproducible research.
# Detect current git commit hash:
GITCOMMITHASH = $(shell git log -1 --abbrev=12 --pretty="format:%h")$(shell test -z "`git status --porcelain -uno`" || echo "-modified")
CFLAGS += -DGITCOMMITHASH="\"$(GITCOMMITHASH)\""
CXXFLAGS += -DGITCOMMITHASH="\"$(GITCOMMITHASH)\""

# The name of the toolbox library.
MHATOOLBOX_NAME = openmha

# Setup relative paths. This breaks if the path contains spaces.
GIT_DIR := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
EXTERNAL_LIBS := $(GIT_DIR)/external_libs
EXTERNAL_LIBS_INCLUDE = -I$(EXTERNAL_LIBS)/$(PLATFORM_CC)/include
EXTERNAL_LIBS_LDFLAGS = -L$(EXTERNAL_LIBS)/$(PLATFORM_CC)/lib
CFLAGS += $(EXTERNAL_LIBS_INCLUDE)
CXXFLAGS += $(EXTERNAL_LIBS_INCLUDE)
LDFLAGS += $(EXTERNAL_LIBS_LDFLAGS)

# How to extend the search path for dynamic libraries on windows and linux
EXTEND_DLLPATH_linux = LD_LIBRARY_PATH="$(1):$$LD_LIBRARY_PATH"
EXTEND_DLLPATH_MinGW = PATH="$(1):$$PATH"
# usage: Prepend shell command with
#        $(call EXTEND_DLLPATH_$(PLATFORM),/some/directory) shell command ...

# modifications of DYLD_LIBRARY_PATH have no effect on recent mac os versions
# we use installname for in-sourcetree tests and rpath for installed executables

# Some private magic may override some settings in here. Do not use.
-include $(dir $(lastword $(MAKEFILE_LIST)))/private_magic.mk

# Local Variables:
# coding: utf-8-unix
# End:
