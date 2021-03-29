This example demonstrates how to write code for the matlab_wrapper plugin with
spectrum to wave processing. The processing code, contained in process.m, implements a 
naive vocoder. 

For every frequency f of the user configurable frequency vector, the frequency is first 
changed to the nearest frequency f' that generates a sinus wave that exactly fits in the
audio block.

The code then injects a sinusoid for every frequency f' with the magnitude proportional to
the magnitude of the frequency bin in the spectrum that corresponds to the frequency f'.

To run this example, the Matlab Coder is needed. Generate the library by running 'make.m'
in Matlab. This generates the C code and, depending on the coder configuration, a shared library.

Copy this shared library into the openMHA plugin directory and then run example_28.cfg with openMHA. 
For details see openMHA_matlab_coder_integration.pdf
