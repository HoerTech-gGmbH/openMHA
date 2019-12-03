# Compilation instructions for developers

Please see file INSTALLATION.md for installing pre-compiled binary packages on
Windows, macOS, and Linux.  It is not necessary to compile openMHA yourself unless
you want to make changes to openMHA itself.

This guide describes how to compile openMHA from sources for developers.

## I. Compiling from source on Linux

### Prerequisites
64-bit version of Ubuntu 18.04 or later,
or a Beaglebone Black running Debian Stretch.

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

Octave and default-jre are not essential for building or running openMHA.
The build process uses Octave + Java to run some tests after
building openMHA.  If Octave is not available, this test will fail,
but the produced openMHA will still work.

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

The optional GUI (cf. openMHA_gui_manual.pdf) requires Java-enabled
Octave in version >= 4.2.1.

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

You should then add the openMHA installation directory to the system search path
for libraries:
```
export DYLD_LIBRARY_PATH=<YOUR-MHA-DIRECTORY>/lib:$DYLD_LIBRARY_PATH
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

- msys2 installation with MinGW64 C++ compiler
- Jack Audio Connection Kit (Use the 64-bit installer for windows) (http://jackaudio.org)

### Preparation

- With the msys2 package manager pacman, install the following packages:
mingw-w64-x86_64-libsndfile, mingw-w64-x86_64-portaudio, and git.
- Copy the contents of the includes folder in the JACK directory into your mingw
include directory (default is c:\msys64\mingw64\include).  There should now be a
directory c:\msys64\mingw64\include\jack containing some files.
- Copy libjack64.lib from the JACK installation to the lib directory of your mingw64
directory and rename it to libjack.a afterwards.  Windows may warn that the
file may become unusable -- ignore this warning.

### Compilation

Start a mingw64 bash shell from the Windows start menu.
Clone openMHA from github, compile openMHA by typing in a terminal
```
git clone https://github.com/HoerTech-gGmbH/openMHA
cd openMHA
./configure && make install
```

The compilation may take a while.
You then need to copy the openMHA libraries into the openMHA bin directory:
```
mv lib/* bin/
```

To start openMHA, you need to start a MinGW64 bash shell and navigate to the
openMHA/bin directory, then type ./mha.exe.

## IV. Regeneration of the documentation:

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
