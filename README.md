# openMHA

HörTech Open Master Hearing Aid (openMHA)

## Content of the openMHA release 4.5.8 (2018-05-19)

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
- resampling and filter plugins
- STFT cyclic aliasing prevention
- adaptive feedback cancellation [6]
- probabilistic sound source localization [7]

## Citation in publications

In publications using openMHA, please cite

Herzke, T., Kayser, H., Loshaj, F., Grimm, G., Hohmann, V., Open signal
processing software platform for hearing aid research (openMHA).
Proceedings of the Linux Audio Conference. Université Jean Monnet,
Saint-Étienne, pp. 35-42, 2017.

As we are working on an updated paper, please check back this section
of the README for updates.

For individual algorithms, please also refer to the list of
publications at the end of this README.

## Installation from binary packages on Ubuntu.

First, add the package source with the openMHA installation packages to your system:

In Ubuntu 18.04:

    sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 104A07F9
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

## Source Code Overview

Together with the openMHA source code, we deliver the source code for the FFTW
library, version 2.1.5, below the directory external_libs.
All openMHA source code lives under the directory mha: The openMHA Toolbox
Library can be found in the sub-directory mha/libmha. It contains the
implementation of the openMHA configuration language, of signal processing
primitives and also many signal processing algorithms to by used in openMHA
plugins. The openMHA command line application and its support framework
can be found in the sub-directory mha/frameworks. The plugins contained in this
release can be found in the sub-directory mha/plugins. The IO libraries,
that connect openMHA, e.g., to the sound card for live processing, or to sound
files for offline processing, are also found here.

## Compiling from source

The openMHA source code has to be compiled before openMHA can be used. While openMHA in
general can be compiled for many operating systems and hardware platforms, in
this release we concentrate on compilation on Ubuntu 18.04 for 64-bit PC
processors (x86_64) and on Debian 8 (jessie) for the Beaglebone Black
single-board ARM computer.
### Prerequisites
#### Linux
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

Octave and default-jre are not essential for building or running openMHA.  
The build process uses Octave + Java to run some tests after
building openMHA.  If Octave is not available, this test will fail,
but the produced openMHA will work.

#### macOS
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

The optional GUI (cf. openMHA_gui_manual.pdf) requires Java-enabled
Octave in version >= 4.2.1.



### Compilation instructions for Linux and macOS:

After downloading and unpacking the openMHA tarball, or cloning from github,
compile openMHA with by typing in a terminal (while in the openMHA directory)
```
./configure && make
```

### Installation of self-compiled openMHA:

A very simple installation routine is provided together with the
source code. To collect the relevant binaries and libraries execute

    make install

You can set the make variable PREFIX to point to the desired installation
location. The default installation location is ".", the current directory.

You should then add the openMHA installation directory to the system search path for libraries

under __Linux__:
```
    export LD_LIBRARY_PATH=<YOUR-MHA-DIRECTORY>/lib:$LD_LIBRARY_PATH
```
under __macOS__:
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

    mha ? cmd=quit

Which should print the default configuration of the openMHA without any plugins
loaded.


## Usage instructions:

We provide with this release several examples of configuration files
and sound examples. These are contained in the directory
./mha/configurations

For example, we can start an example featuring multiple algorithms
together with the following command:

    mha ?read:mha/configurations/prerelease_combination.cfg cmd=start cmd=quit

## Documentation:

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

## Known issues
* There are some known issues with Octave under macOS. The openMHA gui may not work correctly with octave. As an alternative Matlab can be used.
* The qjackctl version provided by the JackOSX distribution is rather old. The user must replace the default Server Path setting with the absolute path to jackdmp (default: /usr/local/bin/jackdmp) (May not be necessary any more, check for yourself).

* On some Apple machines jack needs to be run with root privileges to get real-time priority.

## References for individual algorithms.

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

[7] Kayser H, Anemüller J, A discriminative learning approach to
probabilistic acoustic source localization. In: International Workshop
on Acoustic Echo and Noise Control (IWAENC 2014); 2014. p. 100–104.
