# Early access to features and bug fixes between openMHA releases

openMHA typically has three to four public releases per year.
We recommend that hearing aid algorithm researchers use these
releases to conduct their own studies and refer to these in
publications.

Between these releases, we continuously develop new features and bug
fixes.  After internal review we make these features and bug fixes
available in installer packages for Ubuntu and in a development
version of the source code.

## 1. Ubuntu packages for development versions

You can install packages created from the development branch from a
separate apt repository.  Please be aware that

* Development versions of openMHA will replace existing openMHA releases on
  your computer.
* Development versions will be updated often.
* We will provide each development version only for a limited time (typically
  2 weeks).
* We recommend strongly against using development versions in scientific
  studies.  For the sake of reproducible research, please use released versions.

### 1.1 Installation of development versions on Ubuntu

In Ubuntu 20.04:

    wget -qO- https://aptdev.hoertech.de/openmha-packaging.pub | sudo apt-key add -
    sudo apt-add-repository 'deb [arch=amd64] http://aptdev.hoertech.de focal universe'
    sudo apt-get install openmha openmha-examples

In Ubuntu 18.04:

    wget -qO- https://aptdev.hoertech.de/openmha-packaging.pub | sudo apt-key add -
    sudo apt-add-repository 'deb [arch=amd64] http://aptdev.hoertech.de bionic universe'
    sudo apt-get install openmha openmha-examples

### 1.2 Removal of Ubuntu development packages

In order to remove openMHA development packages from Ubuntu e.g. to use
a regular openMHA release again on a computer that has been configured
to install development versions,

In Ubuntu 20.04:

    sudo apt-add-repository --remove 'deb [arch=amd64] http://aptdev.hoertech.de focal universe'
    sudo apt-get purge libopenmha

In Ubuntu 18.04:

    sudo apt-add-repository --remove 'deb [arch=amd64] http://aptdev.hoertech.de bionic universe'
    sudo apt-get purge libopenmha

Then check that openMHA is really uninstalled: executing

    mha
    
on the command line must now fail with

    Command 'mha' not found

or a similar error message.  After that, follow the instructions from
file INSTALLATION.md to install the latest released version.

## 2. Source code for development versions

Interested developers can access the development branch on
GitHub: https://github.com/HoerTech-gGmbH/openMHA/tree/development

See file COMPILATION.md for how to set up your build environment to
work with openMHA source code.
