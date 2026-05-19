# Compilation instructions for developers

Please see file INSTALLATION.md for installing pre-compiled binary packages on
Windows, macOS, and Linux.  It is not necessary to compile openMHA yourself unless
you want to make changes to openMHA itself.

This guide describes how to compile openMHA from original source code
for developers.


## I. Compiling from source on Linux

### Linux prerequisites
64-bit version of Ubuntu 22.04 or later.

... with the following software packages installed:
- g++
- make
- libsndfile1-dev
- libjack-jackd2-dev
- jackd2
- portaudio19-dev
- liblo-dev
- liblsl
- libeigen3-dev
- libtorch-dev

### Compilation on Linux

Clone openMHA from github, compile openMHA by typing in a terminal
```
git clone https://github.com/HoerTech-gGmbH/openMHA
cd openMHA
./configure --prefix=/usr/local && make
```

### Installation of self-compiled openMHA on Linux:

To install openMHA into the prefix directory given to ./configure above, execute
```
sudo make install
```

Perform a quick test with
```
mha ? cmd=quit
```
Which should print the default configuration of the openMHA without any plugins
loaded.

### Testing self-compiled openMHA on Linux:
Execute the following tests within the openMHA git clone directory.

#### Testing self-compiled openMHA with unit tests on Linux:
```
sudo apt install libboost-dev cmake
make unit-tests
```

#### Executing system tests with self-compiled openMHA on Linux:
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
- Homebrew with the following packages:
  - `brew install jack`
  - `brew install libsndfile pkgconfig portaudio liblo eigen pytorch`
  - `brew install labstreaminglayer/tap/lsl`
- XCode command line tools

### Compilation on macOS

Clone openMHA from github, compile openMHA by typing in a terminal
```
git clone https://github.com/HoerTech-gGmbH/openMHA
cd openMHA
./configure --prefix=/usr/local && make
```
You can change prefix directory if desired.

### Installation of self-compiled openMHA on macOS:

To install openMHA into the prefix directory given to ./configure above, execute
```
make install
```

You should then add the openMHA library installation directory to the openMHA search
path for libraries:
```
export MHA_LIBRARY_PATH=<YOUR-PREFIX>/lib
```
as well as to the search path for executables:
```
export PATH=<YOUR-PREFIX>/bin:$PATH
```
After this, you can invoke the openMHA command line application.
Perform a quick test with
```
mha ? cmd=quit
```
Which should print the default configuration of the openMHA without any plugins
loaded.

### Testing self-compiled openMHA on macOS:
Execute the following tests within the openMHA git clone directory.

#### Testing self-compiled openMHA with unit tests on macOS:
Install the following additional Homebrew packages:
- brew install boost
- brew install cmake
Then:
```
make unit-tests
```

#### Executing system tests with self-compiled openMHA on macOS:
Install the following additional Homebrew packages:
- brew install openjdk
- brew install octave
Inside Octave, install the Octave packages "control" and "signal", e.g. with
```
pkg install -forge control signal
```
(The above will take a long time without much output. Let it finish.)

Matlab with signal processing toolbox can be used as an alternative.

Then in the shell, in the openMHA git directory:
```
make test
```

These tests include some reproducibility checks that will fail if openMHA
source code was changed and not checked into git, or if different components
were compiled from different git commits.

## III. Compilation on 64-bit Windows (advanced)

### Windows prerequisites

- Get **MSYS2 installer** directly from the MSYS2 homepage https://www.msys2.org/ 
 - Installer required for 64-bit Windows would be named msys2-x86_64-*releasedate*.exe. *Release date is in the format of yyyymmdd*
- Execute the installer
- If you have older versions of these tools installed and an upgrade fails,
  then uninstall the old versions via Windows Add/Remove Software and install
  the latest version.

### Windows preparation
The following instructions are for Windows on x64 processors.
All shell commands have to be executed in the msys2 ucrt64 shell.
For compiling on Windows for ARM processors, replace ucrt64 with clangarm64.
- Run **MSYS2 UCRT64** from start menu (if it didn't open automatically after finishing installation). In the terminal, update base package using:
  ```
  pacman -Syu
  ```
- Agree to the terminal being closed when prompted
- Restart **MSYS2 UCRT64** terminal from start menu (again) and type:
  ```
  pacman -Su
  ```
- Install openMHA build dependencies:
  ```
  pacman -S dos2unix git make openbsd-netcat tar unzip wget zip mingw-w64-ucrt-x86_64-boost mingw-w64-ucrt-x86_64-gcc  mingw-w64-ucrt-x86_64-libsndfile mingw-w64-ucrt-x86_64-jack2 mingw-w64-ucrt-x86_64-nsis mingw-w64-ucrt-x86_64-eigen3 mingw-w64-ucrt-x86_64-curl mingw-w64-ucrt-x86_64-liblo mingw-w64-ucrt-x86_64-portaudio mingw-w64-ucrt-x86_64-7zip mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja
  cp /ucrt64/lib/libjack64.dll.a  /ucrt64/lib/libjack.dll.a
  ```
- openMHA needs liblsl, compile and install a MinGW version:
  ```
  git clone -b main https://github.com/sccn/liblsl
  mkdir -p liblsl/build
  prefix=/ucrt64
  cmake -S liblsl -B liblsl/build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$prefix" -DLSL_UNITTESTS=ON -DLSL_OPTIMIZATIONS=OFF -G Ninja
  cmake --build liblsl/build --target install --config Release -j --verbose
  ```
### Windows compilation

Start an MSYS2-UCRT64 bash shell from the Windows start menu.
Clone openMHA from github and compile openMHA by typing in the terminal:
```
git clone https://github.com/HoerTech-gGmbH/openMHA
cd openMHA
./configure --prefix=/ucrt64 && make install
```

The compilation may take a while.

To start a self-compiled openMHA on Windows, you need to 
start the MSYS2-UCRT64 bash shell in the MSYS2-UCRT64 terminal and
execute "mha" from the MSYS2-UCRT64 command line.
Perform a quick test with
```
mha ? mhalib=identity cmd=quit
```
 
### Testing self-compiled openMHA on Windows:
Execute the following tests from an MSYS2-UCRT64 shell
within the openMHA git clone directory.

#### Known issues on Windows:
* Many of our automated tests (e.g. the unit tests testing plugin lsl2ac) are
  using network communication during test execution.  This can result in
  problems like failed or hanging tests on Windows machines with restrictive
  firewall or network settings.
* openMHA plugins using libtorch are not compiled on Windows.  openMHA
  is compiled using the MinGW UCRT toolchain, which is not compatible with the
  libtorch library.

#### Testing self-compiled openMHA with unit tests on Windows:
```
make unit-tests
```

#### Executing system tests with self-compiled openMHA on Windows:
- Install a 64-bit version of openJDK Java for Windows from
  https://adoptium.net (called "Temurin" there).
  Select Windows, JDK, MSI-Installer
- Let the installer create the JAVA_HOME environment variable.
- Install Octave for 64-bit Windows from http://octave.org.
- Start a new MSYS2 UCRT-64 terminal and there
- change directory to the **openMHA/mha/mhatest** directory, then
- start Octave by typing (insert the correct version of Octave)
```
/<Path-to-Octave>/mingw64/bin/octave-cli
```
- inside Octave, execute the openMHA system tests with
```
set_environement; run_mha_tests
```

## IV. Regeneration of the documentation on Linux:

User manuals for different target audiences are provided with this
release in PDF format.  These files can also be re-generated by
typing './configure && make doc' in the terminal (./configure is only
needed if file config.mk has not yet been created).  The new manuals
will be created in the ./mha/doc/ directory.  In addition, HTML
Doxygen documentation is generated in ./mha/doc/mhadoc/html/.

Please install all dependencies for openMHA compilation on Linux first. The
following additional prerequisites are needed for recreating the documents:

- Ubuntu 26.04
- doxygen
- fig2dev
- graphviz
- texlive
- texlive-latex-extra
- texlive-font-utils
