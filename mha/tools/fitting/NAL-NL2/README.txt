This directory contains source code for a command line wrapper of the NAL NL2
DLL.

INSTALLATION

We provide installers for Windows and Linux. These installers install the
wrapper so that NAL NL2 can be used by the MHA fitting GUIs, mhagui_fitting
and mhagui_fitting_offline.

Linux: 
  1) Install closed-source MHA or openMHA as documented
  2) sudo apt install nl2-wrapper
  3) Store a licensed copy of NAL-NL2.dll in directory 
     /usr/share/nalnl2wrapper

Windows:
  1) Install closed-source MHA or openMHA as documented
  2) Download and install
     http://mha.hoertech.de/fitting/nalnl2wrapper-2021.06-installer.exe
  3) Store a licensed copy of NAL-NL2.dll in directory 
     C:\Program Files\nalnl2wrapper\bin


COMPILATION

Requirements:
* Windows operating system
* msys2:  https://www.msys2.org/
* In msys2: packages "make" and "mingw32/mingw-w64-i686-gcc"
* From NAL: files "NAL-NL2.lib" and "NAL-NL2.dll"

To compile, add NAL-NL2.dll and NAL-NL2.lib files to this directory, start an msys2 MinGW 32 bit
terminal, and in this terminal change to this directory and invoke "make".
