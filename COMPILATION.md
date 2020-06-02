# Compilation instructions for developers

Please see file INSTALLATION.md for installing pre-compiled binary packages on
Windows, macOS, and Linux.  It is not necessary to compile openMHA yourself unless
you want to make changes to openMHA itself.

This guide describes how to compile openMHA from sources for developers.

## I. Compiling from source on Linux

### Prerequisites
64-bit version of Ubuntu 18.04 or later,
or a Beaglebone Black running Debian Buster.

... with the following software packages installed:
- g++
- make
- libsndfile1-dev
- libjack-jackd2-dev
- jackd2
- portaudio19-dev
- optional:
  - GNU Octave with the signal package and default-jre
  - liblo-dev
  - liblsl
  - libeigen3-dev

The optional libraries are needed to compile the openMHA plugins
ac2lsl, ac2osc, osc2ac, and rohBeam.  When these libraries are not available,
then openMHA will be compiled without these plugins.

### Compilation

Clone openMHA from github, compile openMHA by typing in a terminal
```
git clone https://github.com/HoerTech-gGmbH/openMHA
cd openMHA
./configure && make
```

### Installation of self-compiled openMHA:

A very simple installation routine is provided together with the
source code.  To collect the relevant binaries and libraries execute
```
make install
```

You can set the make variable PREFIX to point to the desired installation
location. The default installation location is ".", the current directory.

You should then add the openMHA installation directory to the system search path
for libraries:
```
export LD_LIBRARY_PATH=<YOUR-MHA-DIRECTORY>/lib:$LD_LIBRARY_PATH
```
as well as to the search path for executables:
```
export PATH=<YOUR-MHA-DIRECTORY>/bin:$PATH
```
Alternatively to the two settings above, the thismha.sh script found in
the openMHA bin directory may be sourced to set these variables correctly for the
current shell:
```
source <YOUR-MHA-DIRECTORY>/bin/thismha.sh
```
After this, you can invoke the openMHA command line application.
Perform a quick test with
```
mha ? cmd=quit
```
Which should print the default configuration of the openMHA without any plugins
loaded.

## II. Compiling from source on macOS

### Prerequisites
- macOS 10.10 or later.
- XCode 7.2 or later (available from App Store)
- Jack2 for OS X http://jackaudio.org (also available from MacPorts)
- MacPorts https://www.macports.org

The following packages should be installed via MacPorts:
- libsndfile
- pkgconfig
- portaudio
- optional:
  - octave +java
  - octave-signal
  - liblo
  - liblsl
  - eigen3

The optional GUI (cf. openMHA_gui_manual.pdf) requires Java-enabled
Octave in version >= 4.2.1.

The optional libraries are needed to compile the openMHA plugins
ac2lsl, ac2osc, osc2ac, and rohBeam.  When these libraries are not available,
then openMHA will be compiled without these plugins.

### Compilation

Clone openMHA from github, compile openMHA by typing in a terminal
```
git clone https://github.com/HoerTech-gGmbH/openMHA
cd openMHA
./configure && make
```

### Installation of self-compiled openMHA:

A very simple installation routine is provided together with the
source code.  To collect the relevant binaries and libraries execute
```
make install
```

You can set the make variable PREFIX to point to the desired installation
location.  The default installation location is ".", the current directory.

You should then add the openMHA library installation directory to the openMHA search
path for libraries:
```
export MHA_LIBRARY_PATH=<YOUR-MHA-DIRECTORY>/lib
```
as well as to the search path for executables:
```
export PATH=<YOUR-MHA-DIRECTORY>/bin:$PATH
```
Alternatively to the two settings above, the thismha.sh script found in
the openMHA bin directory may be sourced to set these variables correctly for the
current shell:
```
source <YOUR-MHA-DIRECTORY>/bin/thismha.sh
```
After this, you can invoke the openMHA command line application.
Perform a quick test with
```
mha ? cmd=quit
```
Which should print the default configuration of the openMHA without any plugins
loaded.

## III. Compilation on 64-bit Windows (advanced)

### Prerequisites

- Get **msys2 installer** directly from the msys2 homepage https://www.msys2.org/ 
 - Installer required for 64-bit Windows would be named as msys2-x86_64-*releasedate*.exe. *Release date is in the format of yyyymmdd*
- Get Jack Audio Connection Kit from http://jackaudio.org (Use the 64-bit installer for windows)
- Execute both installers

### Preparation
- Run **MSYS2 MinGW 64-bit** from start menu (if it didn't open automatically after finishing installation). In the terminal, update base package using: `pacman -Syu`
- Close terminal when prompted
- Restart **MSYS2 MinGW 64-bit** terminal from start menu (again) and type:`pacman -Su`
- Install openMHA build dependencies:
`pacman -S msys/git mingw64/mingw-w64-x86_64-gcc msys/make tar mingw64/mingw-w64-x86_64-boost openbsd-netcat mingw-w64-x86_64-libsndfile mingw-w64-x86_64-portaudio mingw64/mingw-w64-x86_64-nsis mingw-w64-x86_64-eigen3`
- Copy the contents of the includes folder in the JACK directory **(C:\Program Files (x86)\Jack\includes)** into your mingw include directory (default is **C:\msys64\mingw64\include**). There should now be a directory **(C:\msys64\mingw64\include\jack)** containing some files.
- Copy **libjack64.lib** from the JACK installation **(C:\Program Files (x86)\Jack\lib)** to the lib sub-directory of your mingw64
directory and rename it to **libjack.a** afterwards. Windows may warn that the file may become unusable -- ignore this warning.

### Compilation

Start a MinGW-64 bash shell from the Windows start menu.
Clone openMHA from github and compile openMHA by typing in the terminal:
```
git clone https://github.com/HoerTech-gGmbH/openMHA
cd openMHA
./configure && make install
```

The compilation may take a while.

To start openMHA, you need to start a MinGW-64 bash shell and navigate to the
openMHA/bin directory, then type ./mha.exe.

## IV. Regeneration of the documentation on Linux:

User manuals for different levels of usability in PDF format are
provided with this release.  These files can also be re-generated by
typing './configure && make doc' in the terminal (./configure is only
needed if file config.mk has not yet been created).  The new manuals
will be created in the ./mha/doc/ directory.  In addition, HTML
documentation is generated in ./mha/doc/mhadoc/html/ and
./mha/doc/mhaplugins/html/

Linux recommended for document regeneration. 

Please install all dependencies for openMHA compilation first, along 
with the following prerequisites for recreating the documents (optional!):

- doxygen
- xfig
- graphviz
- texlive
- texlive-latex-extra
- texlive-font-utils
