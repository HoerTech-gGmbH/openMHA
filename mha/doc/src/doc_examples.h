// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2007 2009 2013 2017 2018 2019 2020 HörTech gGmbH
//
// openMHA is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// openMHA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License, version 3 for more details.
//
// You should have received a copy of the GNU Affero General Public License, 
// version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.

/**
\defgroup example_tut Writing \mha Plugins. A step-by-step tutorial 

\brief A step-by-step tutorial on writing \mha plugins.

Plugins are C++ code that is compiled and linked against the \mha library.
The compiler needs be instructed on how to find the \mha headers and library
and to link against the \mha library. There are two possible options: One
can compile openMHA and then create a copy of an example plugin directory
and customize from there. See COMPILATION.md for more information on how to
compile \mha.

On Ubuntu is is also possible to install the openmha-dev package and include
config.mk into the user's Makefile. Example 21 provides an example plugin and
Makefile for this scenario.

\mha contains a small number of example plugins as C++ source code.
They are meant to help developers in understanding the concepts 
of \mha plugin programming starting from the simplest example 
and increasing in complexity. This tutorial explains the basic 
parts of the example files.

\section ex1 example1.cpp
\dontinclude example1.cpp

The example plugin file \c example1.cpp demonstrates the easiest way to
implement an \mha Plugin. 
It attenuates the sound signal in the first channel by multiplying the sound
samples with a factor.
The plugin class MHAPlugin::plugin_t exports several methods, 
but only two of them need a non-empty implementation: \c prepare() method 
is a pure virtual function and \c process() is called when signal processing 
starts.

\skip mha_plugin.hh
\until Do nothing in release

Every plugin implementation should include the 'mha_plugin.hh' header
file.  C++ helper classes for plugin development are declared in this
header file, and most header files needed for plugin development are
included by mha_plugin.hh. 

The class plugin1_t inherits from the class MHAPlugin::plugin_t, which
then inherits from MHAParser::parser_t -- the configuration language
interface in the method "parse".  Our plugin class therefore exports
the working "parse" method inherited from MHAParser::parser_t, and the
plugin is visible in the \mha configuration tree.

The constructor has to accept 3 parameters of correct types.
In this simple example, we do not make use of them.

The \c release() method is used to free resources after signal processing.
In this simple example, we do not allocate resources, so there is no need to
free them.

\subsection ex1_prepare The prepare method
\skip prepare(
\until }
\param signal_info Contains information about the input signal's parameters,
                  see \ref mhaconfig_t.

The \c prepare() method of the plugin is called before the signal
processing starts, when the input signal parameters like domain,
number of channels, frames per block, and sampling rate are known.
The \c prepare() method can check these values and raise an exception if the 
plugin cannot cope with them, as is done here.
The plugin can also change these values if the signal processing performed 
in the plugin results in an output signal with different parameters.
This plugin does not change the signal's parameters, 
therefore they are not modified here.

\subsection ex1_sigproc The signal processing method

\skip process(
\until };

\param signal Pointer to the input signal structure mha_wave_t.
\return Pointer to the output signal structure. 
       The input signal structure may be reused 
       if the signal has the same domain and dimensions.

The plugin works with time domain input signal (indicated by the data type 
mha_wave_t of the process method's parameter).
It scales the first channel by a factor of 0.1. The output
signal reuses the structure that previously contained the input signal
(in-place processing).

\subsection ex1_interface Connecting the C++ class with the C plugin interface

Plugins have to export C functions as their interface (to avoid C++
name-mangling issues and other incompatibilities when mixing plugins
compiled with different C++ compilers).

\skip MHAPLUGIN
\until CALLBACKS(

This macro takes care of accessing the C++ class from the C functions
required as the plugin's interface.  It implements the C funtions and
calls the corresponding C++ instance methods. Plugin classes should be
derived from the template class MHAPlugin::plugin_t to be compatible
with the C interface wrapper.

This macro also catches C++ exceptions of type \ref MHA_Error,
when raised in the methods of the plugin class,
and reports the error using an error flag as the return value
of the underlying C function. 
It is therefore important to note that only C++ exceptions of
type \ref MHA_Error may be raised by your plugin.
If your code uses different Exception classes, you will have to catch them
yourself before control leaves your plugin class, and maybe report the error
by throwing an instance of MHA_Error.
This is important, because: (1) C++ exceptions cannot cross the plugin interface, 
which is in C, and (2) there is no error handling code for your exception classes 
in the \mha framework anyways.

\section ex2 example2.cpp

\dontinclude example2.cpp

This is another simple example of \mha plugin written in C++. 
This plugin also scales one channel of the input signal, working in the time
domain.
The scale factor and which channel to scale (index number) 
are made accessible to the configuration language.

The algorithm is again implemented as a C++ class.


\skip example2_t
\until }

\param scale_ch -- the channel number to be scaled \param factor -- the scale factor of
the scaling.

This class again inherits from the template class MHAPlugin::plugin_t
for intergration with the \mha configuration language.  The two data
members serve as externally visible configuration variables.  All
methods of this class have a non-empty implementation.

\subsection ex2_constructor Constructor

\skip ::example2_t
\until }

The constructor invokes the superclass constructor with a string
parameter.  This string parameter serves as the help text that
describes the functionality of the plugin.
The constructor registers configuration variables with the \mha
configuration tree and sets their default values and permitted ranges.
The minimum permitted value for both variables is zero, 
and there is no maximum limit 
(apart from the limitations of the underlying C data type).
The configuration variables have to be registered with the parser node
instance using the MHAParser::parser_t::insert_item method.

\subsection ex2_prepare The prepare method

\until }
\param signal_info -- contains information about the input signal's parameters,
                  see \ref mhaconfig_t.

The user may have changed the configuration variables before preparing
the \mha plugin.
A consequence of this is that it is not sufficient any more to check if the
input signal has at least 1 audio channel.

Instead, this prepare method checks that the input signal has enough
channels so that the current value of \c scale_ch.data is a valid channel index,
i.e. 0 \f$\le\f$ \c scale_ch.data < \c signal_info.channels.
The prepare method does not have to check that 0 \f$\le\f$ \c scale_ch.data,
since this is guaranteed by the valid range setting of the configuration
variable.

The prepare method then modifies the valid range of the \c scale_ch variable,
it modifies the upper bound so that the user cannot set the variable to a
channel index higher than the available channels.
Setting the range is done using a string parameter. 
The prepare method contatenates a string of the form "[0,n[".
n is the number of channels in the input signal, and is used here as
an exclusive upper boundary. 
To convert the number of channels into a string, a helper function for
string conversion from the \mha Toolbox is used. 
This function is overloaded and works for several data types.

It is safe to assume that the value of configuration variables does not 
change while the prepare method executes,
since \mha preparation is triggered from a configuration language command,
and the \mha configuration language parser is busy and cannot accept
other commands until all \mha plugins are prepared 
(or one of them stops the process by raising an exception).
As we will see later in this tutorial, 
the same assumption cannot be made for the process method.


\subsection ex2_release The release method

\until }

The release method should undo the state changes that were performed
by the prepare method. 
In this example, the prepare method has reduced the valid range of the 
\c scale_ch, so that only valid channels could be selected during signal
processing.

The release method reverts this change by setting the valid range back
to its original value, "[0,[".

\subsection ex2_process The signal processing method

\until }

The processing function uses the current values of the configuration
variables to scale every frame in the selected audio channel.

Note that the value of each configuration variable can change while 
the processing method executes, since the process method usually executes in 
a different thread than the configuration interface.

For this simple plugin, this is not a problem,
but for more advanced plugins, it has to be taken into consideration.
The next section takes a closer look at the problem.

\subsubsection ex2_consistency Consistency

Assume that one thread reads the value stored in a variable while another thread writes a new value to that variable concurrently.
In this case, you may have a consistency problem.
You would perhaps expect that the value retrieved from the
variable either 
(a) the old value, or 
(b) the new value, but not 
(c) something else.
Yet generally case (c) is a possibility.

Fortunately, for some data types on PC systems,
case (c) cannot happen. 
These are 32bit wide data types with a 4-byte alignment.
Therefore, the values in MHAParser::int_t and MHAParser::float_t are
always consistent, but this is not the case for vectors, strings, or
complex values.  With these, you can get a mixture of the bit patterns
of old and new values, or you can even cause a memory access violation in
case a vector or string grows and has to be reallocated to a
different memory address.

There is also a consistency problem if you take the combination of two 
"safe" datatypes. 
The \mha provides a mechanism that can cope with these types of problems.
This thread-safe runtime configuration update mechanism is introduced in
example 5.

\section ex3 example3.cpp
\dontinclude example3.cpp

This example introduces the \mha Event mechanism.
Plugins that provide configuration variable can receive a callback from the 
parser base class when a configuration variable is accessed
through the configuration language interface.

The third example performes the same processing as before, but now
only even channel indices are permitted when selecting the audio
channel to scale.
This restriction cannot be ensured by setting the range of the channel
index configuration variable.
Instead, the event mechanism of \mha configuration variables is used.
Configuration variables emit 4 different events, and your plugin can
connect callback methods that are called when the events are triggered.
These events are:

<b>writeaccess</b>

\li triggered on write access to a configuration variable.

<b>valuechanged</b>

\li triggered when write access to a configuration variable actually
changes the value of this variable.

<b>readaccess</b>

\li triggered after the value of the configuration variable has been read.

<b>prereadaccess</b>

\li triggered before the value of a configuration variable is read,
i.e. the value of the requested variable can be changed by the callback to implement
computation on demand.

All of these callbacks are executed in the configuration thread. 
Therefore, the callback implementation does not have to be realtime-safe.
No other updates of configuration language variables through the
configuration language can happen in parallel,
but your processing method can execute in parallel and may change values.

\subsection ex3_data Data member declarations

\skip example3_t
\until patchbay

This plugin exposes another configuration variable, \c "prepared", that keeps 
track of the prepared state of the plugin.
This is a read-only (monitor) integer variable, 
i.e. its value can only be changed by your plugin's C++ code. 
When using the configuration language interface,
the value of this variable can only be read, but not changed.

The patchbay member is an instance of a connector class that connects 
event sources with callbacks.

\subsection ex3_methods Method declarations

\until };

This plugin exposes 4 callback methods that are triggered by events.
Multiple events (from the same or different configuration variables)
can be connected to the same callback method, if desired.

This example plugin uses the \c valuechanged event to check that the \c
scale_ch configuration variable is only set to valid values.

The other callbacks only cause log messages to stdout, but the comments
in the logging callbacks give a hint when listening on the events would be
useful.

\subsection ex3_constructor Example 3 constructor

\skip ::example3_t
\until }

The constructor of monitor variables does not take a parameter for setting
the initial value. The single parameter here is the help text describing the
contents of the read-only variable. 
If the initial value should differ from 0, then the .\c data member of the
configuration variable has to be set to the initial value in the plugin
constructor's body explicitly, as is done here for demonstration although
the initial value of this monitor variable is 0.

Events and callback methods are then connected using the patchbay member 
variable.

\subsection ex3_prepare The prepare method

\skip ::prepare
\until }

The prepare method checks wether the current setting of the scale_ch 
variable is possible with the input signal dimension.
It does not adjust the range of the variable, 
since the range alone is not sufficient to ensure all future settings are 
also valid: The scale channel index has to be even.

\subsection ex3_release The release method

\skip ::release
\until }

The release method is needed for tracking the prepared state only in this 
example.

\subsection ex3_process The signal processing method

\skip ::process
\until }

The signal processing member function is the same as in example 2.

\subsection ex3_callbaks The callback methods

\skip writeaccess
\until CALLBACKS

When the \c writeaccess or \c valuechanged callbacks throw an MHAError exception,
then the change made to the value of the configuration variable is reverted.

If multiple event sources are connected to a single callback method, 
then it is not possible to determine which event has caused the callback to
execute.
Often, this information is not crucial, i.e. when the answer to a change of 
any variable in a set of variables is the same, 
e.g. the recomputation of a new runtime configuration that takes all 
variables of this set as input.

\section ex4 example4.cpp
\dontinclude example4.cpp

This plugin is the same as example 3 except that it works on the
spectral domain (STFT).

\subsection ex4_prepare The Prepare method
\skip ::prepare
\until }

The prepare method now checks that the signal domain is MHA_SPECTRUM.

\subsection ex4_process The signal processing method

\skip ::process
\until }

The signal processing member function works on the spectral signal 
instead of the wave signal as before.

The mha_spec_t instance stores the complex (mha_complex_t) 
spectral signal for positive frequences only (since the waveform signal
is always real).
The num_frames member of mha_spec_t actually denotes the number of STFT 
bins.

Please note that different from mha_wave_t, a multichannel signal in mha_spec_t 
is stored non-interleaved in the signal buffer.

Some arithmetic operations are defined on struct mha_complex_t to 
facilitate efficient complex computations. 
The \c *= operator used here (defined for real and for complex arguments)
is one of them.

\subsection ex4_interface Connecting the C++ class with the C plugin interface
\skip CALLBACK
\until CALLBACK


When connecting a class that performs spectral processing with the C interface,
use \c spec instead of \c wave as the domain indicator.

\section ex5 example5.cpp
\dontinclude example5.cpp

Many algorithms use complex operations to transform the user space
variables into run time configurations.  If this takes a noticeable
time (e.g. more than 100-500 \f$\mu\f$ sec), 
the update of the runtime configuration
can not take place in the real time processing thread. Furthermore, the
parallel access to complex structures may cause unpredictable results
if variables are read while only parts of them are written to
memory (cf. section \ref ex2_consistency). 
To handle these situations, a special C++ template class \ref
MHAPlugin::plugin_t was designed.
This class helps keeping all access to the configuration language variables
in the \b configuration thread rather than in the \b processing thread.

The runtime configuration class \c example5_t is the parameter of the
template class MHAPlugin::plugin_t. Its constructor converts the user
variables into a runtime configuration.
Because the constructor executes in the configuration thread,
there is no harm if the constructor takes a long time. All
other member functions and data members of the runtime configurations are
accessed only from the signal processing thread (real-time thread).

\skip example5_t
\until }

The plugin interface class inherits from the plugin template class
MHAPlugin::plugin_t, parameterised by the runtime
configuration. Configuration changes (write access to the variables)
will emit a write access event of the changed variables. These events
can be connected to member functions of the interface class by the
help of a MHAEvents::patchbay_t instance.

\skip plugin_interface_t
\until }

The constructor of the runtime configuration analyses and validates
the user variables. 
If the configuration is invalid, an exception of type \ref MHA_Error is thrown.
This will cause the \mha configuration language command which caused the change
to fail: 
The modified configuration language variable is then reset to its original
value, 
and the error message will contain the message string of the 
\ref MHA_Error exception.

\skip example5_t::example5_t
\until }

In this example, the run time configuration class \c example5_t has a
signal processing member function. In this function, the selected
channel is scaled by the given scaling factor.

\skip example5_t::process
\until }
\until }

The constructor of the example plugin class is similar to the previous
examples.
A callback triggered on write access to the variables
is registered using the MHAEvents::patchbay_t instance.

\skip plugin_interface_t::plugin_interface_t
\until }

The processing function can gather the latest valid runtime
configuration by a call of \c poll_config. 
On success, the class member \c cfg points to this configuration. 
On error, if there is no usable runtime configuration instance, an exception is
thrown.
In this example, the prepare method ensures that there is a valid runtime 
configuration, so that in this example, no error can be raised at this point.
The prepare method is always executed before the process method is called.
The runtime configuration class in this example provides a signal processing
method. The process method of the plugin interface calls the process method
of this instance to perform the actual signal processing.

\skip plugin_interface_t::process
\until }

The prepare method ensures that a valid runtime configuration exists by
creating a new runtime configuration from the current configuration language
variables.
If the configuraion is invalid, then an exception of type \ref MHA_Error
is raised and the preparation of the \mha fails with an error message.

\skip ::prepare
\until }

The update_cfg member function is called when the value of a 
configuration language variable changes, or from the prepare method.
It allocates a new runtime
configuration and registers it for later access from the real time
processing thread. The function \ref MHAPlugin::plugin_t::push_config "push_config" stores the
configuration in a FiFo queue of runtime configurations.
Once they are inserted in the FiFo, 
the MHAPlugin::plugin_t template is responsible for deleting runtime
configuration instances stored in the FiFo.
You don't need to keep track of the created instances, and you must not delete
them yourself.

\skip ::update_cfg
\until }

In the end of the example code file, the macro \ref
MHAPLUGIN_CALLBACKS defines all ANSI-C interface functions and passes
them to the corresponding C++ class member functions (partly defined
by the MHAPlugin::plugin_t template class). All exceptions of type
\ref MHA_Error are caught and transformed into an appropriate error
code and error message.

\skipline MHAPLUGIN_CALLBACKS

\section ex6 example6.cpp
\dontinclude example6.cpp

This last example is the same as the previous one, but it additionally creates an
'Algorithm Communication Variable' (AC variable). It calculates the
RMS level of a given channel and stores it into this variable. The
variable can be accessed by any other algorithm in the same chain. To
store the data onto disk, the 'acsave' plugin can be used. 'acmon' is
a plugin which converts AC variables into parsable monitor variables.

In the constructor of the plugin class the variable \c rmsdb is
registered under the name \c example6_rmslev as a one-dimensional AC
variable of type float. For registration of other types, read access
and other detailed informations please see \ref algocomm.

\skip ::example6_t
\until }

\latexonly \par~\par\vfill\par~\par\endlatexonly

\section DebuggingMHAplugins Debugging \mha plugins

Suppose you would want to step through the code of your \mha plugin with a 
debugger.  This example details how to use the linux gdb debugger to
inspect the \c example6_t::prepare() and \c example6_t::process() routines of
\ref ex6
example 6.

First, make sure that your plugin is compiled with the compiler option to
include debugging symbols: Apply the -ggdb switch to all gcc, g++ invocations.

Once the plugin is compiled, with debugging symbols, create a test
configuration. For example 6, assuming there is an audio file named 
input.wav in your working directory, you could create a configuration 
file named `debugexample6.cfg', with the following content:

\verbatim
# debugexample6.cfg
fragsize = 64
srate = 44100
nchannels_in = 2
iolib = MHAIOFile

io.in = input.wav
io.out = output.wav
mhalib = example6
mha.channel = 1
cmd=start
\endverbatim

Assuming all your binaries and shared-object libraries 
are in your `bin' directory (see README.md), you could 
start gdb using
\verbatim
$ export MHA_LIBRARY_PATH=$PWD/bin 
$ gdb $MHA_LIBRARY_PATH/mha
\endverbatim

Set breakpoints in prepare and process methods, and start execution.
Note that specifying the breakpoint by symbol (\c example6_t::prepare) does not yet
work, as the symbol lives in the \mha plugin that has not yet been loaded.
Specifying by line number works, however.
Specifying the breakpoint by symbol also works once the plugin is loaded
(i.e. when the debugger stops in the first break point). You can set the 
breakpoints like this (example shown here is run in gdb version 7.11.1):

\verbatim
(gdb) run ?read:debugexample6.cfg
Starting program: {openMHA_directory}/bin/mha ?read:debugexample6.cfg
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".
The Open Master Hearing Aid (openMHA) server
Copyright (c) 2005-2020 HoerTech gGmbH, D-26129 Oldenburg, Germany

This program comes with ABSOLUTELY NO WARRANTY; for details see file COPYING.
This is free software, and you are welcome to redistribute it 
under the terms of the GNU AFFERO GENERAL PUBLIC LICENSE, Version 3; 
for details see file COPYING.


Breakpoint 1, example6_t::prepare (this=0x6478b0, tfcfg=...)
    at example6.cpp:192
192        if( tfcfg.domain != MHA_WAVEFORM )
(gdb) b example6.cpp:162
Breakpoint 2 at 0x7ffff589744a: file example6.cpp, line 162.
(gdb) c
Continuing.
\endverbatim

Where `{openMHA_directory}' is the directory where openMHA is 
located (which should also be your working directory in this case). 
Next stop is the \c process() method. You can now examine and change the
variables, step through the program as needed (using, for example `n' to 
step in the next line):

\verbatim
Breakpoint 2, example6_t::process (this=0x7ffff6a06c0d, wave=0x10a8b550)
    at example6.cpp:162
162     {
(gdb) n
163        poll_config();
(gdb)
\endverbatim

*/

/* LocalWords:  \mha plugin Matlab Configurator
 */

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
