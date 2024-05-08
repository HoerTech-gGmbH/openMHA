# Installation instructions

This installation guide describes the installation process of openMHA on
Linux (__I.__),  macOS(__II.__), and Windows (__III.__) operating systems.

## I. Installation from binary packages on Ubuntu, and on ARM-based Linux systems.

First, add the openMHA package repository to your system:

In Ubuntu 24.04:

    wget -qO- http://apt.hoertech.de/openmha-packaging.pub | sudo tee /etc/apt/trusted.gpg.d/openmha-packaging.asc
    sudo apt-add-repository 'deb [arch=amd64] http://apt.hoertech.de noble universe'

In Ubuntu 22.04:

    wget -qO- http://apt.hoertech.de/openmha-packaging.pub | sudo tee /etc/apt/trusted.gpg.d/openmha-packaging.asc
    sudo apt-add-repository 'deb [arch=amd64] http://apt.hoertech.de jammy universe'

In Ubuntu 20.04:

    wget -qO- http://apt.hoertech.de/openmha-packaging.pub | sudo apt-key add -
    sudo apt-add-repository 'deb [arch=amd64] http://apt.hoertech.de focal universe'

On computers with an ARM CPU running a variant of Debian, Ubuntu,
Raspberry Pi OS, Armbian, or similar: The following instructions work for
both, 32 and 64 bit ARM systems.  A requirement for 32 bit ARM systems is that
the CPU needs to be at least ARMv7.

For ARM systems based on Debian 10 or Ubuntu 20.04:

    wget -qO- http://apt.hoertech.de/openmha-packaging.pub | sudo apt-key add -
    echo 'deb http://apt.hoertech.de bionic universe' | sudo tee /etc/apt/sources.list.d/openmha.list
    sudo apt update

For ARM systems based on Debian 11 or Ubuntu 22.04 or later:

    wget -qO- http://apt.hoertech.de/openmha-packaging.pub | sudo tee /etc/apt/trusted.gpg.d/openmha-packaging.asc
    echo 'deb http://apt.hoertech.de bullseye universe' | sudo tee /etc/apt/sources.list.d/openmha.list
    sudo apt update


Second, install openMHA and some openMHA usage examples:
```
sudo apt install openmha openmha-examples
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

Algorithm developers interested in implementing their own plugins should also
install the development package __libopenmha-dev__.

For updating openMHA when a new release is available, execute:

```
sudo apt update
sudo apt install openmha
```

This will upgrade all installed openmha packages to their latest version.

## II. Mac installation with Homebrew

(Note: If you upgrade an older version of openMHA (up to 4.17.0), which used a
pkg installer instead of Homebrew, then you need to remove the old version
first. See the next section on "Uninstalling openMHA on Mac".)

Install and update Homebrew. Instructions can be found at https://brew.sh.

Install openMHA with the following command:

```
brew install openmha/tap/openmha
```

openMHA executes some tests during installation. You may be asked to grant
permissions like network access during the installation process.

(Note: If the installation fails, because some homebrew packages could not be
linked, then you have older, non-Homebrew versions of openMHA or its
dependencies installed. Uninstall them and retry. See also the next section on
"Uninstalling openMHA on Mac".)

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

### II.a Uninstalling openMHA on Mac

If you have openMHA v4.17.0 or older installed on your Mac, then you need
to uninstall it and its dependencies before you can install the new version
with Homebrew. You can use our script
[Mac_Uninstall_openMHA_Jack_pkg](Mac_Uninstall_openMHA_Jack_pkg)
to uninstall openMHA and Jack installed with the old pkg installer.
Download the file and execute it with bash:
```
bash Mac_Uninstall_openMHA_Jack_pkg
```

If you have installed openMHA with Homebrew, you can uninstall it with the
following command:
```
brew uninstall openmha
```
There is no need to uninstall a Homebrew-installed openMHA before upgrading,
in this case you can upgrade to the new version with:
```
brew update && brew upgrade
```

## III. Windows installer

An openMHA installer for 64 bit Windows 10 can be downloaded from our
Github releases page, https://github.com/HoerTech-gGmbH/openMHA/releases.

The installer installs openMHA example setups and some tools. We recommend to
copy the examples folder to a writable location inside your user directory.

After installation, you can find the examples in folder
`C:\Program Files\openMHA\examples`. The reference algorithms are located in
`C:\Program Files\openMHA\reference_algorithms`.
The folder`C:\Program Files\openMHA\mfiles` contains Matlab / Octave files for
interacting with openMHA.  Folder `C:\Program Files\openMHA\doc` contains
documentation.

Due to compatibility issues the Windows build requires Jack version 1.9.21, which can
be downloaded here: https://jackaudio.org/downloads/
