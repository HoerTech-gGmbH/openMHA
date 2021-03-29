This example demonstrates how to write code for the matlab_wrapper plugin with
wave to wave processing. The processing code, contained in process.m, implements a 
simple delay-and-sum algorithm. 

To run this example, the Matlab Coder is needed. Generate the library by running 'make.m'
in Matlab. This generates the C code and, depending on the coder configuration, a shared library.

Copy this shared library into the openMHA plugin directory and then run example_25.cfg with openMHA. 
For details see openMHA_matlab_coder_integration.pdf
