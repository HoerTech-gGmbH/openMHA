openMHA is an open source product and comes with a selection of
non-commercial hearing aid dynamic compressor prescription rules.

It is also possible to fit a dynamic compressor in openMHA with the
commercial hearing aid prescription rule *DSLmio 5* by the University of
Western Ontario (UWO).

Currently, this works on Linux systems, but we are working to extend it to
Windows systems.  If you prefer to use a Windows computer to apply the fitting,
please check if the latest version of this document, which is available at
https://github.com/HoerTech-gGmbH/openMHA/blob/development/README_DSLmio5.md,
contains instructions for Windows users.

In order to fit openMHA with *DSLmio 5*, you need to

1) Have a 64-bit PC with Ubuntu 18.04 or Ubuntu 20.04 installed.
2) Purchase the DLL version of the fitting rule *DSLmio 5* from UWO for 64 bit
   Linux.  Copy all files and symbolic links named 
   - libDSLmio-core.so
   - libDSLmio-core.so.5
   - libDSLmio-core.so.<Version>+build.<BuildNo>
   - dslmio.dat
   into the directory `/usr/lib` of the Linux computer.
3) Install openMHA on this computer.  See file INSTALLATION.md for instructions.
4) Install the dsl wrapper with `apt install dsl-wrapper`. This will also
   install Octave if not already installed.
5) Start Octave in order to fit an openMHA instance with DSLmio 5.  You can use
   our tools `mhacontrol` or `mhagui_fitting` in order to fit a separately
   started openMHA instance, or you can use our offline fitting tool
   `mhagui_fitting_offline` to preprocess audio files with hearing aid dynamic
   compression from within Octave.  All of these tools will now find and offer
   an option to fit with `DSL`.

Please check the DSLmio 5 fitting that you apply before using it for your
research. You can find how we compute the DSLmio 5 insertion gains for openMHA
dynamic compressors in file `/usr/lib/openmha/mfiles/gainrule_DSL.m`.  Please
check if these computations match your expectation, and if you find any errors
or inaccuracies, please report them by filing an issue with the openMHA project
on github: https://github.com/HoerTech-gGmbH/openMHA/issues
