# Installation instructions

This installation guide describes the installation process of openMHA on Linux (__I.__, __II.__), macOS (__III.__) and Windows (__IV.__) operating systems as well as how to (re-)build the documentation files delivered with openMHA. 

## I. Installation from binary packages on Ubuntu.

First, add the package source with the openMHA installation packages to your system:

In Ubuntu 18.04:

    sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys B7D6CDF547DA4ABD
    sudo apt-add-repository 'deb http://apt.openmha.org/ubuntu bionic universe'

In Ubuntu 16.04:

    sudo apt-add-repository 'deb http://mha.hoertech.de/hoertech/xenial /'
    sudo apt-get update

For Ubuntu 16.04, this will give you a warning:
```
W: The repository 'http://mha.hoertech.de/hoertech/xenial Release' does not have a Release file.
N: Data from such a repository can't be authenticated and is therefore potentially dangerous to use.
N: See apt-secure(8) manpage for repository creation and user configuration details.`
```

Install openMHA:
```
sudo apt-get install openmha
```

For Ubuntu 16.04, this will give you again an authentication warning:
```
WARNING: The following packages cannot be authenticated! openmha libopenmha
Install these packages without verification? [y/N]
```

To install openMHA you have to type "y".

These authentication issues have been solved for Ubuntu 18.04 starting with openMHA release 4.5.7.

After installation, openMHA documentation is found in
`/usr/share/doc/openmha`
and tools for GNU Octave/Matlab in `/usr/lib/openmha/mfiles`

We provide some examples together with the openMHA.
When using debian packages, you can find the examples in a separate package,
*openmha-examples*. After installing the openmha-examples package:
```
sudo apt-get install openmha-examples
```
the examples can be found in `/usr/share/openmha/examples`.

NOTE: If you want to use the example files we recommend to make a copy in your home directory as they are located in a system-wide read-only directory. Some of the examples may require changes to work with the current audio hardware setup and need write access to store output.

Algorithm developers interested in implementing their own plugins should also install the development package libopenmha-dev.

For updating openMHA when a new release is available run
```
sudo apt-get install --only-upgrade openmha
```

## II. Compiling from source (Linux)

The openMHA source code has to be compiled before openMHA can be used. While openMHA in
general can be compiled for many operating systems and hardware platforms, in
this release we concentrate on compilation on Ubuntu 18.04 for 64-bit PC
processors (x86_64) and on Debian 8 (jessie) for the Beaglebone Black
single-board ARM computer.

### Prerequisites
64-bit version of Ubuntu 18.04 or later,
or a Beaglebone Black with Debian jessie installed.

... with the following software packages installed:
- g++-7 for Ubuntu, g++-4.9 for Debian
- make
- libsndfile1-dev
- libjack-jackd2-dev
- jackd2
- portaudio19-dev
- optional:
  - GNU Octave with the signal package and default-jre (e.g. openjdk-8-jre for Debian 9)
  - liblo
  - liblsl 

Octave and default-jre are not essential for building or running openMHA.
The build process uses Octave + Java to run some tests after
building openMHA.  If Octave is not available, this test will fail,
but the produced openMHA will work.

### Compilation

After downloading and unpacking the openMHA tarball, or cloning from github,
compile openMHA with by typing in a terminal (while in the openMHA directory)
```
./configure && make
```

### Installation of self-compiled openMHA:

A very simple installation routine is provided together with the
source code. To collect the relevant binaries and libraries execute
```
make install
```

You can set the make variable PREFIX to point to the desired installation
location. The default installation location is ".", the current directory.

You should then add the openMHA installation directory to the system search path for libraries

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

## III. Compiling from source (macOS)

### Prerequisites
- macOS 10.10 or later.
- XCode 7.2 or later (available from App Store)
- Jack2 for OSX http://jackaudio.org (also available from MacPorts)
- MacPorts https://www.macports.org

The following packages should be installed via macports:
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

After downloading and unpacking the openMHA tarball, or cloning from github,
compile openMHA with by typing in a terminal (while in the openMHA directory)
```
./configure && make
```

### Installation of self-compiled openMHA:

A very simple installation routine is provided together with the
source code. To collect the relevant binaries and libraries execute
```
make install
```

You can set the make variable PREFIX to point to the desired installation
location. The default installation location is ".", the current directory.

You should then add the openMHA installation directory to the system search path for libraries
```
export DYLD_LIBRARY_PATH=<YOUR-MHA-DIRECTORY>/lib:$LD_LIBRARY_PATH
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

## IV. Compilation on 64-bit Windows (advanced)

### Prerequisites

- Java JRE 64-bit (Choose 64-bit windows version)  (https://java.com)
- GNU Octave 64-bit (Choose latest 64-bit windows version) (https://www.gnu.org/software/octave/)
- Jack Audio Connection Kit (Use the 64-bit installer for windows) (http://jackaudio.org)

### Preparation

- Copy the contents of the includes folder in the JACK directory into the include directory in your Octave directory (default is c:\octave\octave-x.y.z) there should now be a directory c:\octave\octave-x.y.z\include\jack containing some files
- Copy libjack64.lib from the JACK installation to the lib directory in your Octave directory and rename it to libjack.a . Windows may warn you that the file may become unusable -- ignore this warning.
- Download the openMHA source from http://www.openmha.org/download/ and extract the zip archive in your Octave directory. There should now be a directory named openMHA-master in your Octave directory.

### Compilation
Start a bash shell by double clicking on bash.exe in the bin subdirectory of your
Octave installation. Enter the following commands in the bash shell prompt:
```
cd /openMHA-master
PATH=/bin
ln -s /bin/true /bin/git
ln -s /bin/g++ /bin/g++-7
ln -s /bin/gcc /bin/gcc-7
ln -s /bin/cpp /bin/cpp-7
./configure
make
make install
```
The compilation may take a while.
You then need to copy the openMHA libraries into the openMHA bin directory:
```
mv lib/* bin/.
```

To start openMHA, you need to start a bash shell and navigate to the openMHA-master/bin directory, then type ./mha.exe.

## V. Regeneration of the documentation:

User manuals are provided in PDF format.  Recreating them from source is
normally not necessary.

User manuals for different levels of usability in PDF format are
provided with this release.  These files can also be re-generated by
typing './configure && make doc' in the terminal (./configure is only
needed if file config.mk has not yet been created).  The new manuals
will be created in the ./mha/doc/ directory.  In addition, html
documentation is generated in ./mha/doc/mhadoc/html/ and
./mha/doc/mhaplugins/html/

Prerequisites for recreating the documents (Optional!):
Extra packages are needed for generating documentation:

- doxygen
- xfig
- graphviz
- texlive
- texlive-latex-extra
