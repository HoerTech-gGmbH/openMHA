# Compilation instructions for developers

Please see file INSTALLATION.md for installing pre-compiled binary packages on
Windows, macOS, and Linux.  It is not necessary to compile openMHA yourself unless
you want to make changes to openMHA itself.

This guide describes how to compile openMHA from sources for developers.

## I. Compiling from source on Linux

### Linux prerequisites
64-bit version of Ubuntu 20.04 or later,
or a Beaglebone Black running Debian Buster.

... with the following software packages installed:
- g++ (minimum version: g++ 7)
- make
- libsndfile1-dev
- libjack-jackd2-dev
- jackd2
- portaudio19-dev
- liblo-dev
- liblsl
- libeigen3-dev

### Compilation on Linux

Clone openMHA from github, compile openMHA by typing in a terminal
```
git clone https://github.com/HoerTech-gGmbH/openMHA
cd openMHA
./configure && make
```

### Installation of self-compiled openMHA on Linux:

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

### Testing self-compiled openMHA on Linux:

#### Testing self-compiled openMHA with unit tests on Linux:
```
sudo apt install libboost-dev cmake
make unit-tests
```

#### Executing system tests with self-compiled openMHA on Linux:
If using Ubuntu 20.04, then edit or create a file
`/usr/share/octave/5.2.0/m/java/java.opts` and make sure that it contains a line
```
-Djdk.lang.processReaperUseDefaultStackSize=true
```
This works around an error in the Octave package of Ubuntu 20.04, for details
refer to https://savannah.gnu.org/bugs/?59310.  Then:
```
sudo make install octave-signal default-jre-headless
./configure
make test
```
Matlab with signal processing toolbox can be used as an alternative to
installing Octave.
These tests include some reproducibility checks that will fail if openMHA
source code was changed and not checked into git, or if different components
were compiled from different git commits.

## II. Compiling from source on macOS

### macOS prerequisites
- macOS 10.10 or later.
- XCode 7.2 or later (available from App Store)
- Jack2 for OS X http://jackaudio.org (also available from MacPorts)
- MacPorts https://www.macports.org

Homebrew can be used as an alternative to MacPorts.

The following MacPorts packages should be installed:
- libsndfile
- pkgconfig
- portaudio
- liblo
- liblsl
- eigen3

### Compilation on macOS

Clone openMHA from github, compile openMHA by typing in a terminal
```
git clone https://github.com/HoerTech-gGmbH/openMHA
cd openMHA
./configure && make
```

### Installation of self-compiled openMHA on macOS:

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

### Testing self-compiled openMHA on macOS:

#### Testing self-compiled openMHA with unit tests on macOS:
Install the boost library headers and cmake, then
```
make unit-tests
```

#### Executing system tests with self-compiled openMHA on macOS:
Install Octave and Java so that Octave can be found on the PATH.
Install the Octave packages "control" and "signal".
Matlab with signal processing toolbox can be used as an alternative.
```
./configure
make test
```

These tests include some reproducibility checks that will fail if openMHA
source code was changed and not checked into git, or if different components
were compiled from different git commits.

## III. Compilation on 64-bit Windows (advanced)

### Windows prerequisites

- Get **MSYS2 installer** directly from the MSYS2 homepage https://www.msys2.org/ 
 - Installer required for 64-bit Windows would be named as msys2-x86_64-*releasedate*.exe. *Release date is in the format of yyyymmdd*
- Get Jack Audio Connection Kit from http://jackaudio.org (Use the 64-bit installer for windows)
- Execute both installers
- If you have older versions of these tools installed and an upgrade fails,
  then uninstall the old versions via Windows Add/Remove Software and install
  the latest version.

### Windows preparation
- Run **MSYS2 MinGW 64-bit** from start menu (if it didn't open automatically after finishing installation). In the terminal, update base package using:
  ```
  pacman -Syu
  ```
- Close terminal when prompted
- Restart **MSYS2 MinGW 64-bit** terminal from start menu (again) and type:
  ```
  pacman -Su
  ```
- Install openMHA build dependencies:
  ```
  pacman -S msys/git mingw64/mingw-w64-x86_64-gcc msys/make tar
  pacman -S mingw64/mingw-w64-x86_64-boost openbsd-netcat
  pacman -S mingw-w64-x86_64-libsndfile mingw-w64-x86_64-portaudio
  pacman -S mingw64/mingw-w64-x86_64-nsis mingw-w64-x86_64-eigen3 msys/wget
  pacman -S msys/unzip msys/zip dos2unix mingw64/mingw-w64-x86_64-curl
  pacman -S mingw-w64-x86_64-liblo
```
- Copy the Jack for Windows developer resources to directories where the
  MSYS2 MinGW64 toolchain can find them (renaming import lib in the process):
  ```
  cp -rv /c/Program*Files/Jack2/include/* /mingw64/include/
  cp /c/Program*Files/Jack2/lib/libjack64.dll.a /mingw64/lib/libjack.dll.a
  ```
- openMHA needs liblsl, install a MinGW version:
  ```
  wget https://github.com/HoerTech-gGmbH/liblsl/releases/download/v1.14.0-htch/liblsl-1.14.0-MinGW64.zip
  unzip -d /mingw64 liblsl-1.14.0-MinGW64.zip
  rm liblsl-1.14.0-MinGW64.zip
  ```
### Windows compilation

Start a MinGW-64 bash shell from the Windows start menu.
Clone openMHA from github and compile openMHA by typing in the terminal:
```
git clone https://github.com/HoerTech-gGmbH/openMHA
cd openMHA
./configure && make install
```

The compilation may take a while.

To start a self-compiled openMHA on Windows, you need to 
1) start the MinGW-64 bash shell in the MSYS2 terminal and there
2) change directory to the openMHA/bin directory, then
3) test mha execution by typing 
   ```
   ./mha.exe ? cmd=quit
   ```
Not following this procedure can result in MHA not being able to find
all necessary DLLs.

### Testing self-compiled openMHA on Windows:
#### Known issues on Windows:
Many of our automated tests (e.g. the unit tests testing plugin lsl2ac) are
using network communication during test execution.  This can result in
problems like failed or hanging tests on Windows machines with restrictive
firewall or network settings.

#### Testing self-compiled openMHA with unit tests on Windows:
```
pacman -S  mingw-w64-x86_64-cmake
make unit-tests
```

#### Executing system tests with self-compiled openMHA on Windows:
- Install a 64-bit version of openJDK Java for Windows from
  https://jdk.java.net/.
- Add the `bin` directory of the openJDK installation to the system PATH and
  create the JAVA_HOME environment variable to point to the parent directory
  of that bin directory.
- Install Octave for 64-bit Windows from http://octave.org.
- Start the MinGW-64 bash shell in an MSYS2 terminal and there
- change directory to the **openMHA/mha/mhatest** directory, then
- start Octave by typing (insert the correct version of Octave)
```
/c/Octave/Octave->>version<</mingw64/bin/octave-gui.exe --gui
```
- inside Octave, execute the openMHA system tests with
```
set_environement; run_all_tests
```

## IV. Regeneration of the documentation on Linux:

User manuals for different levels of usability in PDF format are
provided with this release.  These files can also be re-generated by
typing './configure && make doc' in the terminal (./configure is only
needed if file config.mk has not yet been created).  The new manuals
will be created in the ./mha/doc/ directory.  In addition, HTML
Doxygen documentation is generated in ./mha/doc/mhadoc/html/.

Please install all dependencies for openMHA compilation on Linux first. The
following additional prerequisites are needed for recreating the documents:

- Ubuntu 20.04 or Ubuntu 22.04
- doxygen
- xfig
- graphviz
- texlive
- texlive-latex-extra
- texlive-font-utils
