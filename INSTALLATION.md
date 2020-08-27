# Installation instructions

This installation guide describes the installation process of openMHA on
Linux (__I.__), Windows (__II.__), and macOS (__III.__) operating systems.

## I. Installation from binary packages on Ubuntu.

First, add the openMHA package repository to your system:

In Ubuntu 20.04:

    wget -qO- https://apt.hoertech.de/openmha-packaging.pub | sudo apt-key add -
    sudo apt-add-repository 'deb [arch=amd64] http://apt.hoertech.de focal universe'

In Ubuntu 18.04:

    wget -qO- https://apt.hoertech.de/openmha-packaging.pub | sudo apt-key add -
    sudo apt-add-repository 'deb [arch=amd64] http://apt.hoertech.de bionic universe'

In Ubuntu 16.04:

    wget -qO- https://apt.hoertech.de/openmha-packaging.pub | sudo apt-key add -
    sudo apt-add-repository 'deb [arch=amd64] http://apt.hoertech.de xenial universe'
    sudo apt update

Install openMHA and some openMHA usage examples:
```
sudo apt-get install openmha openmha-examples
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
sudo apt-get update
sudo apt-get install openmha
```

This will upgrade all installed openmha packages to their latest version.

## II. macOS installer

An openMHA installer for macOS can be downloaded from our
Github releases page, https://github.com/HoerTech-gGmbH/openMHA/releases.

To use the Jack audio plugin, the [JackOSX distribution](http://www.jackaudio.org)
needs to be installed.

On some machines macOS refuses to open the installer because it is from an
unverified developer. Opening the installer while holding Ctrl makes macOS
offer an override option.

The installer installs openMHA example setups and some tools. We recommend to
copy the examples folder to a writable location inside your user directory.

On macOS, you can find the examples folder in
`/usr/local/share/openmha/examples/` and the reference algorithms can be found in
`/usr/local/share/openmha/reference_algorithms`. m-files for interacting with the openMHA
from Matlab or Octave are installed in `/usr/local/lib/openmha/mfiles/`, and
documentation can be found in `/usr/local/doc/openmha/`.



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
