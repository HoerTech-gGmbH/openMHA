// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2007 2013 2017 2018 HörTech gGmbH
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

/** \defgroup plugif The \mha Plugins (programming interface)

* An \mha plugin is the signal processing unit, usually an algorithm. \mha
* plugins can be combined into processing chains. One of the configured
* chains can be selected for output which allows direct comparison of
* single algorithms or complex signal processing
* configurations. Algorithms within one chain can communicate with each
* other by sharing some of their variables, see section \ref algocomm.
*
* The \mha plugins can use the \mha configuration language for their
* configuration. If they do so, the configuration can be changed through
* the framework even at run time. A description of this language can be
* found in section \ref mhascript. If the algorithms should make use of
* the \mha configuration language, they need to be written in C++ rather than
* pure C.
*
* In the \mha package a set of example plugins is included. These examples 
* are the base of a step by step tutorial on how to write an \mha plugin. 
* See section \ref example_tut for detailes.
*
* \mha plugins communicate with the \mha using a simple ANSI-C interface.
* This way it is easy to mix plugins compiled with different C++ compilers.
* For convenience, we provide C++ classes which can be connected
* to the C++ interface. We strongly recommend the usage of these C++ wrappers.
* They include out-of-the box support exporting variables to the
* configuration interface and for thread safe configuration update.
*
* The \mha C++ plugin interface consists of a few number of
* method prototypes:
*
* The output domain (spectrum or waveform) of an \mha plugin
* will typically be the same as the input domain:
*
* - mha_wave_t * process(\ref mha_wave_t *): pure waveform processing
* - mha_spec_t * process(\ref mha_spec_t *): pure spectral processing
* .
* But it is also possible to implement domain transformations (from
* the time domain into spectrum or vice versa). 
* The corresponding method signatures are:
*
* - mha_spec_t * process(\ref mha_wave_t *):
*   Domain transformation from waveform to spectrum
* - mha_wave_t * process(\ref mha_spec_t *):
*   Domain transformation from spectrum to waveform
* .
* For preparation and release of a plugin, the methods
*
* - void prepare(\ref mhaconfig_t &) and
* - void release(void)
* .
* have to be implemented.
* The \mha will call the \c process() method only ater the prepare method has 
* returned and before \c release() is invoked.
* It is guarantteed by the \mha framework that signal
* processing is performed only between calls of <tt>prepare()</tt> and
* <tt>release()</tt>. Each call of <tt>prepare()</tt> is followed by a call
* of <tt>release()</tt> (after some optional signal processing).
*
* For configuration purposes, the plugin class has to export a method called 
* \c parse() which implements the \mha configuration language.
* We strongly recommend that you do not implement this method yourself,
* but by inheriting from the class MHAParser::parser_t from the \mha toolbox,
* directly or indirectly (inheriting from a class that itself inherits from
* MHAParser::parser_t).
*
* \section sec_plugif_cxx Connecting the C++ class with the C Interface
*
* A C++ class which provides the appropriate methods can be used as
* an \mha Plugin by connecting it to the C interface using the
* \ref MHAPLUGIN_CALLBACKS macro. 
*
* The \mha Toolbox library provides a base class MHAPlugin::plugin_t\<T\>
* (a template class) which can be used as the base class for a plugin class.
* This base class implements some necessary features for \mha plugin developers
* like integration into the \mha configuration language environment 
* (it inherits from MHAParser::parser_t) and 
* thread-safe runtime configuration update.
*
* \section sec_plugif_error Error reporting
*
* When your plugin detects a situation that it cannot handle, 
* like input signal of the wrong signal domain at preparation time,
* unsupported number of input channels at preparation time,
* unsupported combinations of values in the plugin's variables during
* configuration, it should throw a C++ exception.
* The exception should be of type MHAError. Exceptions of this type are caught by the 
* \ref MHAPLUGIN_CALLBACKS macro for further error
* Reporting.
*
* Throwing exceptions in response to unsupported configuration changes does not
* stop the signal processing. 
* The \mha configuration language parser will restore the previous value of that
* variable and report an error to the configurator, while the signal processing continues.
* Throwing exceptions from the signal processing thread will terminate the
* signal processing. 
* Therefore, you should generally avoid throwing exceptions from the
* process method.
* Only do this if you detected a defect in your plugin, 
* and then you should include enough information in the error message to be able
* to fix the defect.
*
* \section sec_plugif_contents Contents of the \mha Plugin programming interface
*
*/

#include "mha_plugin.hh"

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
