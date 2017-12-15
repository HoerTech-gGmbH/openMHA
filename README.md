# openMHA

HörTech Open Master Hearing Aid (openMHA)

1. Content of the openMHA release 4.5.3 (2017-12-15)

The software contains the source code of the openMHA Toolbox library, of the
openMHA framework and command line application, and of a selection of algorithm
plugins forming a basic hearing aid processing chain featuring
- calibration
- bilateral adaptive differential microphones for noise suppression [1]
- binaural coherence filter for feedback reduction and dereverberation [2]
- multi-band dynamic range compressor for hearing loss compensation [3]
- spatial filtering algorithms:
 - a delay-and-sum beamformer
 - a MVDR beamformer [4]
- single-channel noise reduction [5]
- simple upsampling and downsampling plugins
- STFT cyclic aliasing prevention
- adaptive feedback cancellation [6]

2. Citation in publications

In publications using openMHA, please cite

Herzke, T., Kayser, H., Loshaj, F., Grimm, G., Hohmann, V., Open signal
processing software platform for hearing aid research (openMHA).
Proceedings of the Linux Audio Conference. Université Jean Monnet,
Saint-Étienne, pp. 35-42, 2017.

As we are working on an updated paper, please check back this section
of the README for updates.

For individual algorithms, please also refer to the list of
publications at the end of this README.

3. Installation from binary packages on Ubuntu.

First, add the package source with the openMHA Debian packages to your system:

under Ubuntu 16.04

    sudo apt-add-repository 'deb http://mha.hoertech.de/hoertech/xenial /'

under Ubuntu 14.04

    sudo apt-add-repository 'deb http://mha.hoertech.de/hoertech/trusty /'

Update the list of available packages:

    sudo apt-get update

This will give you a warning:

    W: The repository 'http://mha.hoertech.de/hoertech/xenial Release' does not have a Release file.
    N: Data from such a repository can't be authenticated and is therefore potentially dangerous to use.
    N: See apt-secure(8) manpage for repository creation and user configuration details.

Install openMHA:

    sudo apt-get install openmha

This will give you again an authentication warning:
WARNING: The following packages cannot be authenticated! openmha libopenmha Install these packages without verification? [y/N]

To install openMHA you have to type "y".

The authentication issue will be resolved in the future.

After installation, openMHA documentation is found in
/usr/share/doc/openmha
and tools for GNU Octave/Matlab here:
/usr/lib/openmha/mfiles

To update openMHA when a new release is available run

    sudo apt-get install --only-upgrade openmha



4. Source Code Overview

Together with the MHA source code, we deliver the source code for the FFTW
library, version 2.1.5, below the directory external_libs.
All openMHA source code lives under the directory mha: The MHA Toolbox
Library can be found in the sub-directory mha/libmha. It contains the
implementation of the MHA configuration language, of signal processing
primitives and also many signal processing algorithms to by used in MHA
plugins. The MHA command line application and its support framework
can be found in the sub-directory mha/frameworks. The plugins contained in this
release can be found in the sub-directory mha/plugins. The IO libraries,
that connect the MHA e.g. to the sound card for live processing, or to sound
files for offline processing, are also found here.

5. Build Instructions

The MHA source code has to be compiled before the MHA can be used. While MHA in
general can be compiled for many operating systems and hardware platforms, in
this release we concentrate on compilation on Ubuntu 16.04 for 64-bit PC
processors (x86_64) and on Debian 8 (jessie) for the Beaglebone Black
single-board ARM computer.

## Linux
Prerequisites:
64-bit version of Ubuntu 16.04 or later,
or a Beaglebone Black with Debian jessie installed.

with the following software packages installed:
- g++-5 for Ubuntu, g++-4.9 for Debian
- make
- libsndfile1-dev
- libjack-jackd2-dev
- jackd2
- portaudio19-dev
- optional:
  - octave with the signal package and default-jre (e.g. openjdk-8-jre
    and openjdk-8-jdk for Debian 9)

octave and default-jre are not essential for building or running the
openMHA.  The build process uses Octave + Java to run some tests after
building openMHA.  If Octave is not available, this test will fail,
but the produced openMHA will still work.

## macOS
Prerequisites:

-macOS 10.10 or later.
-XCode 7.2 or later
-Jack2 for OSX http://jackaudio.org
-MacPorts

The following packages should be installed via macports:
- libsndfile
- pkgconfig
- portaudio
-optional:
 -octave +java 
 -octave-signal
 
The optional GUI (cf. openMHA_gui_manual.pdf) requires Java-enabled
Octave in version >= 4.2.1.

### Known Issues
* There are some known issues with Octave under macOS. The mha gui may not work correctly with octave. As an alternative Matlab can be used.
* The qjackctl version provided by the Jack distribution is rather old. The user must replace the default Server Path setting with the absolute path to jackdmp (default: /usr/local/bin/jackdmp)

6. Compilation instructions:

After downloading and unpacking the openMHA package, or cloning from github,
compile the MHA with ./configure && make

6.1 Installation instructions:

A very simple installation routine is provided together with the
source code. To collect the relevant binaries and libraries execute

make install

Then, you should set the environment variable LD_LIBRARY_PATH to point to your
bin directory, like this:

export LD_LIBRARY_PATH=$PWD/bin

You can also add the bin directory to the PATH environment variable:

export PATH=$PATH:$PWD/bin

Alternatively, the mha.sh script found in bin/mha.sh may be sourced.

After this, you can invoke the MHA command line. Perform a quick test with

mha ? cmd=quit

Which should print the default configuration of the MHA without any plugins
loaded.


7. Usage instructions:

We provide with this release several examples of configuration files
and sound examples. These are contained in the directory
./mha/configurations

For example, we can start an example featuring multiple algorithms
together with the following command:

mha ?read:mha/configurations/prerelease_combination.cfg cmd=start cmd=quit

8. Documentation:

User manuals are provided in PDF format.  Recreating them from source is
normally not necessary.

User manuals for different levels of usability in PDF format are
provided with this release.  These files can also be generated by
typing 'make doc' in the terminal.  The new manuals will be created in
the ./mha/doc/ directory.  In addition, html documentation is
generated in ./mha/doc/mhadoc/html/ and ./mha/doc/mhaplugins/html/

Prerequisites for recreating the documents (Optional!):
Extra packages are needed for generating documentation:

- doxygen
- xfig
- graphviz
- texlive
- texlive-latex-extra

9. References for individual algorithms.

[1] Elko GW, Pong ATN. A Simple Adaptive First-order Differential
Microphone. In: Proceedings of 1995 Workshop on Applications of Signal
Processing to Audio and Accoustics; 1995. p. 169–172.

[2] Grimm G, Hohmann V, Kollmeier B. Increase and Subjective
Evaluation of Feedback Stability in Hearing Aids by a Binaural
Coherence-based Noise Reduction Scheme. IEEE Transactions on Audio,
Speech, and Language Processing. 2009;17(7):1408–1419.

[3] Grimm G, Herzke T, Ewert S, Hohmann V. Implementation and
Evaluation of an Experimental Hearing Aid Dynamic Range Compressor
Gain Prescription. In: DAGA 2015; 2015. p. 996–999.

[4] Adiloğlu K, Kayser H, Baumgärtel RM, Rennebeck S, Dietz M, Hohmann
V. A Binaural Steering Beamformer System for Enhancing a Moving Speech
Source. Trends in Hearing. 2015;19:2331216515618903

[5] Gerkmann T, Hendriks RC. Unbiased MMSE-Based Noise Power
Estimation With Low Complexity and Low Tracking Delay. IEEE
Transactions on Audio, Speech, and Language
Processing. 2012;20(4):1383–1393.

[6] Schepker H, Doclo S, A semidefinite programming approach to
min-max estimation of the common part of acoustic feedback paths in
hearing aids. IEEE Transactions on Audio, Speech, and Language
Processing. 2016;24(2):366-377.
