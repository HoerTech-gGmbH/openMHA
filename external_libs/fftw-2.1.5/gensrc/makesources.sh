#! /bin/sh
# This file generates all the FFTW sources

. ./config

# Compute the list of file names
PARALLEL="-j 4"
notw_codelets=""
notwi_codelets=""

for i in $NOTW
do
    notw_codelets="$notw_codelets ${NOTW_PREFIX}${i}.c"
    notwi_codelets="$notwi_codelets ${NOTWI_PREFIX}${i}.c"
done

for i in $NOTW_REAL
do
    notw_codelets="$notw_codelets ${REAL2HC_PREFIX}${i}.c"
    notwi_codelets="$notwi_codelets ${HC2REAL_PREFIX}${i}.c"
done

twiddle_codelets=""
twiddlei_codelets=""

for i in $TWIDDLE
do
    twiddle_codelets="$twiddle_codelets ${TWID_PREFIX}${i}.c"
    twiddlei_codelets="$twiddlei_codelets ${TWIDI_PREFIX}${i}.c"
done

for i in $TWIDDLE_REAL
do
    twiddle_codelets="$twiddle_codelets ${HC2HC_FORWARD_PREFIX}${i}.c"
    twiddlei_codelets="$twiddlei_codelets ${HC2HC_BACKWARD_PREFIX}${i}.c"
done

make $PARALLEL -f Makefile.sources $notw_codelets $notwi_codelets $twiddle_codelets $twiddlei_codelets
