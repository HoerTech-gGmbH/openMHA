# This file is part of the HörTech Master Hearing Aid (MHA)
# Copyright © 2016 HörTech gGmbH
# All rights reserved.

include ../config.mk
include ../magic.mk

EXTERNAL_LIBS=$(realpath .)

TARGETS=fftw.is_installed

include ../rules.mk

ifdef CFLAGS
CFLAGS:=$(filter-out -Werror, $(CFLAGS))
export CFLAGS
endif
ifdef CXXFLAGS
CXXFLAGS:=$(filter-out -Werror, $(CXXFLAGS))
export CXXFLAGS
endif
ifdef LDFLAGS
export LDFLAGS
endif
ifdef LDLIBS
export LDLIBS
endif

$(BUILD_DIR)/fftw.is_installed: $(BUILD_DIR)/fftw.is_configured
	cd fftw-2.1.5/. && $(MAKE) install
	cd "$(EXTERNAL_LIBS)" && touch $@

$(BUILD_DIR)/fftw.is_configured: $(BUILD_DIR)/.directory
	cd fftw-2.1.5/. && ./configure --enable-static --disable-shared --prefix="$(EXTERNAL_LIBS)/$(BUILD_DIR)" --bindir="$(EXTERNAL_LIBS)/$(BUILD_DIR)"/bin --libdir="$(EXTERNAL_LIBS)/$(BUILD_DIR)"/lib --includedir="$(EXTERNAL_LIBS)/$(BUILD_DIR)"/include  --enable-float --enable-type-prefix --disable-fast-install --disable-fortran
	cd "$(EXTERNAL_LIBS)" && touch $@
