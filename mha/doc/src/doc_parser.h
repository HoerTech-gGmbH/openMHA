// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2017 HörTech gGmbH
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

/** \defgroup cvariables Concept of Variables and Data Exchange in the \mha

\addindex configuration variable
\addindex variable 
\addindex configuration
\addindex variables
Accessibility of configuration variables and data exchange between plugins 
(processing blocks) are an important issue in the \mha. In general, variable 
types in the \mha are distinguished by their different access methods. The 
variable types in the \mha are:
\addindex configuration variable
	- <B> Configuration variables </B>: Read and write accesses are 
possible through the \mha configuration language interface. Configuration 
variables are implemented as C++ classes with a public data member of the 
underlying C type. Configuration variables can be read and modified from 
``outside'' using the configuration language. The plugin which provides 
the configuration variable can use the exposed data member directly. All 
accesses through the \mha configuration language are checked for data type, 
valid range, and access restrictions. 
\addindex monitor variable
	- <B> Monitor variables </B>: Read access is possible through the 
\mha configuration language. Write access is only possible from the C++ code. 
Internally, monitor variables have a similar C++ class interface as 
configuration variables.
\addindex AC variable
	- <B> AC variables </B> (\ref algocomm "algorithm communication variables"): 
Any C or C++ data structure can be shared within an \mha chain. Access management and 
name space is realised in \mha chain plugin ('mhachain').  AC 
variables are not available to the \mha configuration language interface, 
although a read-only converter plugin \c acmon is available.
\addindex runtime configuration 
	- <B> Runtime configuration </B>: Algorithms usually derive more 
parameters (runtime configuration) from the \mha configuration language 
variables. When a configuration variable changes through configuration 
language write access, then the runtime configuration has to be recomputed. 
Plugin developers are encouraged to encapsulate the runtime configuration in 
a C++ class, which recomputes the runtime configuration from configuration 
variables in the constructor. The \mha supports lock-free and thread-safe 
replacement of the runtime configuration instance (see \ref ex5 and references 
therein).
	.

\image html variables.png 
\image latex variables.pdf "Variable types in the \mha" width=0.7\textwidth

The C++ data types are shown in the figure below. These variables can be 
accessed via the \mhad using the \mha configuration language. For more 
details see 'Application engineers' manual'.

\image html parserelements.png
\image latex parserelements.pdf "MHAParser elements" width=0.7\textwidth

*/ 

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
