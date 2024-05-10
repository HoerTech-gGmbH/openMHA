# Compiling self-developed plugins for openMHA

openMHA's Plugin Development Guide linked from 
http://www.openmha.org/documentation/
contains a tutorial how to write openMHA plugins in C++. The C++ code needs to
be compiled before it can be used in openMHA. How to compile plugins from
source code is described in this README file.

This directory contains the source code of an example plugin in
example33.cpp, and a Makefile which can be used to compile the plugin on the 
Portable Hearing Laboratory (PHL) running Mahalia 4.18.0-r0 or later.

### Transfer the plugin source code to the PHL

Since the code must be compiled on the PHL you need to transfer it to the
device first. To do so connect your computer with the PHL via Wifi and copy the
code from this example to the PHL (PW:mahalia):
```
scp example33.cpp Makefile mha@10.0.0.1:
```
copies the required files in the home directory of the user mha on the PHL.
Answer "yes" if your asked if you are sure you want to continue connecting
in case you connect to your device the first time.

If you have trouble to connect and you have connected to a different PHL device
before the connection will be refused for security reasons. This can be solved 
by executing 
```
ssh-keygen -R 10.0.0.1
```

### Compiling the example plugin on the PHL

Connect to the PHL with ssh (PW:mahalia): 
```
ssh mha@10.0.0.1
```

In the directory containing the source code and the Makefile (in this case the
current directory after login, i.e., the mha home directory) execute
```
make
```

This takes around 1 minute. To make the plugin available to openMHA on the PHL
copy the generated *.so file to directory /usr/lib:
```
sudo cp example33.so /usr/lib
```

### Using the example plugin on the PHL 

Now the self-compiled plugin can be used in your openMHA configuration like any
other plugin. The example plugin compiled here is referenced as "example33", 
i.e., the file name without the extension.
