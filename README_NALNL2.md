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

Installers for this wrapper are provided for Windows and Linux.  These install
the wrapper so that NAL NL2 can be used by the MHA fitting GUIs, mhagui_fitting
and mhagui_fitting_offline.  The NAL NL2 DLL is not part of the wrapper and
needs to be purchased from the National Acoustic Laboratories independently.

### Linux: 
  1) Install closed-source MHA or openMHA as documented
  2) sudo apt install nl2-wrapper
  3) Store a licensed copy of NAL-NL2.dll in directory 
     /usr/share/nalnl2wrapper

### Windows:
  1) Install closed-source MHA or openMHA as documented
  2) Download and install
     http://mha.hoertech.de/fitting/nalnl2wrapper-2021.06-installer.exe
  3) Store a licensed copy of NAL-NL2.dll in directory 
     C:\Program Files\nalnl2wrapper\bin
