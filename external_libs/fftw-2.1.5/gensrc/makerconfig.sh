#! /bin/sh

# This shell script generates the file rconfig.c containing a table
# of the rfftw codelets

. ./config

cat $COPYRIGHT
echo
cat rconfig_prelude
echo

# declare the external variables
for i in $NOTW_REAL
do
    echo "extern fftw_codelet_desc fftw_real2hc_${i}_desc;"
    echo "extern fftw_codelet_desc fftw_hc2real_${i}_desc;"
done
for i in $TWIDDLE_REAL
do
    echo "extern fftw_codelet_desc fftw_hc2hc_forward_${i}_desc;"
    echo "extern fftw_codelet_desc fftw_hc2hc_backward_${i}_desc;"
done

echo
echo

echo "fftw_codelet_desc *rfftw_config[] = {"

for i in $NOTW_REAL
do
    echo "NOTW_CODELET(${i}),"
    echo "NOTWI_CODELET(${i}),"
done

for i in $TWIDDLE_REAL
do
    echo "TWIDDLE_CODELET(${i}),"
    echo "TWIDDLEI_CODELET(${i}),"
done

echo "(fftw_codelet_desc *) 0 "
echo "};"
