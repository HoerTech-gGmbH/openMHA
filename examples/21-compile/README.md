# Compiling self-developed plugins for openMHA

openMHA's Plugin Development Guide linked from 
http://www.openmha.org/documentation/
contains a tutorial how to write openMHA plugins in C++. The C++ code needs to
be compiled before it can be used in the openMHA. How to compile plugins from
source code is described in this README file.

This directory contains the source code of an example plugin in
example21.cpp, and a Makefile which can be used to compile the plugin.

## Compiling the example plugin on Linux and macOS

1) Install openMHA (including the libopenmha-dev package for Linux) as described in file
   ../../INSTALLATION.md.

2) In a directory containing the files example21.cpp and Makefile, execute
   ```
   make
   ```

3) Copy the generated library file to your global library directory:
   __Linux__ 
   ```
   sudo cp example21.so /usr/lib
   ```
   __macOS__
   ```
   sudo cp example21.dylib /usr/local/lib
   ```

### Compiling an example plugin on the PHL (Portable Hearing Laboratory)

Please refer to the README.md file in the directory
examples/33-compile-plugin-on-PHL.

## Compiling the example plugin on Windows

1) Install openMHA as described in file ..\..\INSTALLATION.md.

2) Install a build environment for openMHA on Windows as described in file
   ..\..\COMPILATION.md. Ensure the version of the installed GCC compiler
   matches the version listed in file C:\Program Files\openMHA\config.mk.

3) Git clone the openMHA source code from git in order to get the openMHA
   header files.

4) Edit the example Makefile from this directory:
   - Change the include line to
     ```
     include /c/Progra~1/openMHA/config.mk
     ```
   - Extend the LIBS setting to
     ```
     LIBS = -L/c/Progra~1/openMHA/bin -lopenmha
     ```
   - Change the INCLUDES setting so that include files are found in directory
     mha/libmha/src inside the openMHA directory that you cloned in step 3.

5) In a MinGW64 bash shell, execute
   ```
   make
   ```

6) Copy the generated *.dll file to directory C:\Program Files\openMHA\bin.

# Using the self-compiled plugin in openMHA

In the openMHA configuration, load the self-compiled plugin like any other
plugin. The example plugin compiled here is referenced as "example21" in the
configuration, i.e. the file name without the extension.
