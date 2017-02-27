#! /bin/sh

# This shell script generates the file config.c containing a table
# of the codelets

. ./config

cat $COPYRIGHT
echo
cat config_prelude
echo

# declare the external variables
for i in $NOTW
do
    echo "extern fftw_codelet_desc fftw_no_twiddle_${i}_desc;"
    echo "extern fftw_codelet_desc fftwi_no_twiddle_${i}_desc;"
done
for i in $TWIDDLE
do
    echo "extern fftw_codelet_desc fftw_twiddle_${i}_desc;"
    echo "extern fftw_codelet_desc fftwi_twiddle_${i}_desc;"
done

echo
echo

echo "fftw_codelet_desc *fftw_config[] = {"

for i in $NOTW
do
    echo "NOTW_CODELET(${i}),"
    echo "NOTWI_CODELET(${i}),"
done

for i in $TWIDDLE
do
    echo "TWIDDLE_CODELET(${i}),"
    echo "TWIDDLEI_CODELET(${i}),"
done

echo "(fftw_codelet_desc *) 0 "
echo "};"
