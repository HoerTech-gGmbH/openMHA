# Installation instructions

This installation guide describes the installation process of openMHA on
Linux (__I.__),  macOS(__II.__), and Windows (__III.__) operating systems.

## I. Installation on Ubuntu

Find the latest openMHA release in our
[Github releases page](https://github.com/HoerTech-gGmbH/openMHA/releases).
We provide .zip files for several Ubuntu versions and processor architectures.
Download the .zip file for your system and extract it.  Open the contained
openmha-packages directory in a terminal, and install the .deb files with the
following command: 
```
sudo apt install ./*.deb
```

After installation, openMHA documentation is found in
`/usr/share/doc/openmha`
and tools for GNU Octave/Matlab in `/usr/lib/openmha/mfiles`.

We provide some examples for openMHA, after installing the openmha-examples
package the examples can be found in `/usr/share/openmha/examples`.

The reference algorithms can be found in `/usr/share/openmha/reference_algorithms`.

NOTE: If you want to use the example files we recommend to make a copy in your
home directory as they are located in a system-wide read-only directory. Some of
the examples may require changes to work with the current audio hardware setup
and need write access to store output.

## II. Mac installation with Homebrew

Install and update Homebrew. Instructions can be found at https://brew.sh.

Install openMHA with the following command:

```
brew install openmha/tap/openmha
```

openMHA executes some tests during installation. You may be asked to grant
permissions like network access during the installation process.

Homebrew installs openMHA example setups and some tools. We recommend to
copy the examples folder to a writable location inside your user directory.

The default installation locations depend on the processor type of your Mac:

On Macs with Intel CPU, you can find the examples folder in
`/usr/local/share/openmha/examples/` and the reference algorithms can be found
in `/usr/local/share/openmha/reference_algorithms`. m-files for interacting
with the openMHA from Matlab or Octave are installed in
`/usr/local/lib/openmha/mfiles/`, and documentation can be found in
`/usr/local/share/doc/openmha/`.

On Macs with ARM processor ("Apple Silicon"), you can find the examples
folder in `/opt/homebrew/share/openmha/examples/` and the reference algorithms
can be found in `/opt/homebrew/share/openmha/reference_algorithms`. m-files
for interacting with the openMHA from Matlab or Octave are installed in
`/opt/homebrew/lib/openmha/mfiles/`, and documentation can be found in
`/opt/homebrew/share/doc/openmha/`.

## III. Installation on Windows

Find the latest openMHA release in our
[Github releases page](https://github.com/HoerTech-gGmbH/openMHA/releases).
We provide .zip files for Windows on x64 processors as well as for ARM64
processors. Download the .zip file for your system and extract it into
`C:\Program Files` or another location of your choice. The zip file contains a
folder named `openMHA` containing all files of the installation. Add the folder
`C:\Program Files\openMHA\bin` to your system PATH variable.

After installation, you can find the examples in folder
`C:\Program Files\openMHA\examples`. The reference algorithms are located in
`C:\Program Files\openMHA\reference_algorithms`.
The folder`C:\Program Files\openMHA\mfiles` contains Matlab / Octave files for
interacting with openMHA.  Folder `C:\Program Files\openMHA\doc` contains
documentation.
