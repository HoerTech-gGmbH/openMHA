# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2016 2017 HörTech gGmbH
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

include ../config.mk
include ../magic.mk

EXTERNAL_LIBS=$(realpath .)

TARGETS = fftw.is_installed

include ../rules.mk

# Pass compiler flags to third-party library build process.
# If third-party libraries produce compiler warnings, then we do not treat
# these as errors.
ifdef CFLAGS
CFLAGS:=$(filter-out -Werror, $(CFLAGS))
export CFLAGS
endif
ifdef CXXFLAGS
CXXFLAGS:=$(filter-out -Werror, $(CXXFLAGS))
export CXXFLAGS
endif

# Pass linker flags to third-party library build process
ifdef LDFLAGS
export LDFLAGS
endif
ifdef LDLIBS
export LDLIBS
endif

# Build fftw-2
$(BUILD_DIR)/fftw.is_installed: $(BUILD_DIR)/fftw.is_configured
	cd fftw-2.1.5/. && $(MAKE) -j 1 install
	cd "$(EXTERNAL_LIBS)" && touch $@

# Configure fftw-2
$(BUILD_DIR)/fftw.is_configured: $(BUILD_DIR)/.directory
	sleep 0.01 && touch fftw-2.1.5/depcomp       # Recreate the original
	sleep 0.01 && touch fftw-2.1.5/install-sh    # ordering of the files'
	sleep 0.01 && touch fftw-2.1.5/missing       # mtimes.
	sleep 0.01 && touch fftw-2.1.5/mkinstalldirs # Otherwise, autotools
	sleep 0.01 && touch fftw-2.1.5/ltmain.sh     # (if installed) would
	sleep 0.01 && touch fftw-2.1.5/config.guess  # attempt to recreate
	sleep 0.01 && touch fftw-2.1.5/config.sub    # FFTW's configure script.
	sleep 0.01 && touch fftw-2.1.5/acinclude.m4  # 
	sleep 0.01 && touch fftw-2.1.5/AUTHORS       # We have to avoid
	sleep 0.01 && touch fftw-2.1.5/bootstrap.sh  # recreation of the
	sleep 0.01 && touch fftw-2.1.5/COPYING       # configure script,
	sleep 0.01 && touch fftw-2.1.5/COPYRIGHT     # because a final
	sleep 0.01 && touch fftw-2.1.5/fftw.spec.in  # reproducibility check,
	sleep 0.01 && touch fftw-2.1.5/INSTALL       # that is performed after
	sleep 0.01 && touch fftw-2.1.5/Makefile.am   # openMHA is built, will
	sleep 0.01 && touch fftw-2.1.5/README        # fail if any files differ
	sleep 0.01 && touch fftw-2.1.5/README.hacks  # from their version
	sleep 0.01 && touch fftw-2.1.5/TODO          # checked into git.
	sleep 0.01 && touch fftw-2.1.5/acx_mpi.m4
	sleep 0.01 && touch fftw-2.1.5/acx_pthread.m4
	sleep 0.01 && touch fftw-2.1.5/configure.in
	sleep 0.01 && touch fftw-2.1.5/ChangeLog
	sleep 0.01 && touch fftw-2.1.5/aclocal.m4
	sleep 0.01 && touch fftw-2.1.5/configure
	sleep 0.01 && touch fftw-2.1.5/Makefile.in
	cd fftw-2.1.5/. && ./configure --enable-static --disable-shared --prefix="$(EXTERNAL_LIBS)/$(BUILD_DIR)" --bindir="$(EXTERNAL_LIBS)/$(BUILD_DIR)"/bin --libdir="$(EXTERNAL_LIBS)/$(BUILD_DIR)"/lib --includedir="$(EXTERNAL_LIBS)/$(BUILD_DIR)"/include  --enable-float --enable-type-prefix --disable-fast-install --disable-fortran
	cd "$(EXTERNAL_LIBS)" && touch $@

# Local Variables:
# compile-command: "make"
# coding: utf-8-unix
# End:
