This example demonstrates how to write code for the matlab_wrapper plugin with
spectrum to spectrum processing. 
The processing code, contained in process.m implements a
generic filter in the spectral domain. The filter coefficients are multiplied
element-wise with the fft bins. For a valid configuration there need to be as many filter 
coefficients as there are fft bins. 

To run this example, the Matlab Coder is needed. Generate the library by running 'make.m'
in Matlab. This generates the C code and, depending on the coder configuration, a shared library.

Copy this shared library into the openMHA plugin directory and then run example_27.cfg with openMHA. 
For details see openMHA_matlab_coder_integration.pdf
