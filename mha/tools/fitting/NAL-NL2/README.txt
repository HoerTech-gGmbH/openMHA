This directory contains source code for a command line wrapper of the NAL NL2
DLL.

INSTALLATION

We provide installers for Windows and Linux. These installers install the
wrapper so that NAL NL2 can be used by the MHA fitting GUIs, mhagui_fitting
and mhagui_fitting_offline.

Linux: ...
Windows: ...

COMPILATION

Requirements:
* Windows operating system
* msys2:  https://www.msys2.org/
* In msys2: packages "make" and "mingw32/mingw-w64-i686-gcc"
* From NAL: files "NAL-NL2.lib" and "NAL-NL2.dll"

To compile, add NAL-NL2.dll and NAL-NL2.lib files to this directory, start an msys2 MinGW 32 bit
terminal, and in this terminal change to this directory and invoke "make".
