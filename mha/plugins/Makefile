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

# mha plugins makefile
#
# Plugins are grouped by category. Each category / plugin group is a
# subdirectory.

include ../../config.mk
include ../../magic.mk

SUBDIRS = $(patsubst %/Makefile,%,$(wildcard */Makefile))

# gtfb_simd uses x86-only SSE intrinsics (xmmintrin.h / __builtin_ia32_*) and
# can only be compiled for Intel-compatible processors. Skip it on every other
# architecture.
ifeq "$(filter x86_64 i%86,$(ARCH))" ""
SUBDIRS := $(filter-out gtfb_simd,$(SUBDIRS))
endif

include ../../rules.mk

# Local Variables:
# compile-command: "make"
# coding: utf-8-unix
# End:
