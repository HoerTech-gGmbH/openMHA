openMHA is an open source product and comes with a selection of
non-commercial hearing aid dynamic compressor prescription rules.

It is also possible to fit a dynamic compressor in openMHA with the
commercial hearing aid prescription rule *NAL NL2* by the National
Acoustic Laboratories (NAL).

This works on Windows and Linux systems and needs the installation
of a NAL-NL2 wrapper and of the NAL-NL2 DLL.

## Installation of the NAL-NL2 wrapper

In order to fit dynamic compressors in closed-source MHA or openMHA with the
commercial prescription rule "NAL NL2", the NAL-NL2.DLL is required and a
command line wrapper that connects the DLL to our Matlab/Octave based fitting
GUIs.

This wrapper is available at
[mha/tools/fitting/NAL-NL2/nalnl2wrapper.exe](mha/tools/fitting/NAL-NL2/nalnl2wrapper.exe)
The wrapper is not part of the openMHA project. The NAL NL2 DLL is required for the
wrapper to work and must be purchased from the National Acoustic Laboratories.

### Linux: 
  1) Create a directory /usr/share/nalnl2wrapper
  2) Store file nalnl2wrapper.exe in directory /usr/share/nalnl2wrapper
  3) Store a licensed copy of NAL-NL2.dll in directory 
     /usr/share/nalnl2wrapper
  4) `sudo dpkg --add-architecture i386`
  5) `sudo apt update`
  6) `sudo apt install wine32:i386`

### Windows:
  1) Create a directory C:\Program Files\nalnl2wrapper
  2) Store file nalnl2wrapper.exe in directory C:\Program Files\nalnl2wrapper
  3) Store a licensed copy of NAL-NL2.dll in directory 
     C:\Program Files\nalnl2wrapper\bin

MHA fitting GUIs, mhagui_fitting and mhagui_fitting_offline now offer "NAL NL2"
as a fitting rule option for dynamic compressors.

Please check the NAL NL2 fitting that you apply before using it for your
research. You can find how we compute the NAL NL2 insertion gains for openMHA
dynamic compressors in file `gainrule_NAL_NL2.m`.  Please
check if these computations match your expectation, and if you find any errors
or inaccuracies, please report them by filing an issue with the openMHA project
on github: https://github.com/HoerTech-gGmbH/openMHA/issues
