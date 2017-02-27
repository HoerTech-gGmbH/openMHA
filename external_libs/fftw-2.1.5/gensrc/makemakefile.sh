#! /bin/sh

# This file generates the FFTW makefile, starting from Makefile.fftw.am.
# It also generates the rfftw makefile.

. ./config

###########################################################################

# Compute the list of file names
notw_codelets=""
notwi_codelets=""

for i in $NOTW
do
    notw_codelets="$notw_codelets ${NOTW_PREFIX}${i}.c"
    notwi_codelets="$notwi_codelets ${NOTWI_PREFIX}${i}.c"
done

twiddle_codelets=""
twiddlei_codelets=""

for i in $TWIDDLE
do
    twiddle_codelets="$twiddle_codelets ${TWID_PREFIX}${i}.c"
    twiddlei_codelets="$twiddlei_codelets ${TWIDI_PREFIX}${i}.c"
done

# now substitute list in Makefile.fftw.am, to get Makefile.fftw
# (the two cats are redundant, but the script is clearer this way)
cat Makefile.fftw.am |
    sed -e "s/@NOTW_CODELETS@/$notw_codelets/g" |
    sed -e "s/@NOTWI_CODELETS@/$notwi_codelets/g" |
    sed -e "s/@TWID_CODELETS@/$twiddle_codelets/g" |
    sed -e "s/@TWIDI_CODELETS@/$twiddlei_codelets/g" |
cat >Makefile.fftw

###########################################################################

# Compute the list of file names
notw_codelets=""
notwi_codelets=""

for i in $NOTW_REAL
do
    notw_codelets="$notw_codelets ${REAL2HC_PREFIX}${i}.c"
    notwi_codelets="$notwi_codelets ${HC2REAL_PREFIX}${i}.c"
done

twiddle_codelets=""
twiddlei_codelets=""

for i in $TWIDDLE_REAL
do
    twiddle_codelets="$twiddle_codelets ${HC2HC_FORWARD_PREFIX}${i}.c"
    twiddlei_codelets="$twiddlei_codelets ${HC2HC_BACKWARD_PREFIX}${i}.c"
done

# now substitute list in Makefile.rfftw.am, to get Makefile.rfftw
# (the two cats are redundant, but the script is clearer this way)
cat Makefile.rfftw.am |
    sed -e "s/@NOTW_CODELETS@/$notw_codelets/g" |
    sed -e "s/@NOTWI_CODELETS@/$notwi_codelets/g" |
    sed -e "s/@TWID_CODELETS@/$twiddle_codelets/g" |
    sed -e "s/@TWIDI_CODELETS@/$twiddlei_codelets/g" |
cat >Makefile.rfftw
