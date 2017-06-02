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
valid range, and access restrictions. See section \ref scriptlng for details.
\addindex monitor variable
	- <B> Monitor variables </B>: Read access is possible through the 
\mha configuration language. Write access is only possible from the C++ code. 
Internally, monitor variables have a similar C++ class interface as 
configuration variables. See section \ref scriptlng for details.
\addindex AC variable
	- <B> AC variables </B> (algorithm communication variables): Any C or 
C++ data structure can be shared within an \mha chain. Access management and 
name space is realised in \mha chain plugins ('mhachain' and 'altplugs').  AC 
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
replacement of the runtime configuration instance. See the programmers reference handbook for details.
	.

\image html variables.png 
\image latex variables.pdf "Variable types in the \mha" width=0.7\textwidth

\addindex script language
\addindex \mha script language
\addindex language 
\addindex configuration 
\addindex hierarchical configuration
\addindex configuration!hierarchical 
\addindex configuration language 
\addindex \mha configuration language
\section scriptlng The \mha configuration language

The \mhad and most of the \mha plugins are controlled
through the \mha configuration language. This language is implemented in the
MHAToolbox library. It allows hierarchical configuration similar to the
concept of \Matlab structures. Each configuration level (parser) \addindex parser
can contain items like variables or sub-parsers. Properties of any item can be
queried.  Write access to items can be connected with C++ callback functions
which makes it possible to change the configuration and state of the \mha 
and all plugins while the audio signal is being processed.

The \mha configuration language consists of line-based human-readable
text commands. The \mha configuration language interpreter receives commands by
reading text files or through a TCP network stream. The \mha also provides 
access to the configuration language parser via a C++ object, 
which also uses the text interface \addindex(text interface),
for embedding the \mha into other applications (e.g. Windows \mha
configurator GUI, \Matlab access).

\subsection strcriptlng Structure of the \mha configuration language

A \mha configuration language command has a very simple structure: Each
command consists of a left value, an operator \addindex(operator) and a
right value. Three operators are defined:
\addindex(access operator)
\addindex(operator!access-)
\addindex(query operator)
\addindex(operator!query-)
\addindex(descending operator)
\addindex(operator!descending-)
	- An <b> access operator "=" </b> is used to set a value of a variable.
	- A <b> query operator "?" </b> is used to query a value, type or other 
information of a variable or other nodes (with some exceptions).
	- A <b> descending operator "." </b> descends into the next level of the 
hierarchical \mha configuration.
	.

Each left value is the name of a parser entry. Not all operators are
available for all parser entries: A subparser supports only "?" and
".", a monitor only "?". The structure of parser entries is shown in
\figref{parserelements}. In the configuration files, \mha script
language commands can be split up into multiple lines similar as in
\Matlab: If a lines ends with \c "...", the next line will be
appended. This holds not for the command prompt (e.g.\ Windows
framework, Configurator, TCP interface).

\image html parserelements.png
\image latex parserelements.pdf "MHAParser elements" width=0.7\textwidth


The \mha configuration language features strong static typing, the data type
of a variable is defined by the plugin that implements this variable.
Many configuration language commands like write access ("=") to variables can
be connected to C++ callbacks by the plugin developer.

\subsection querycmds Query commands

\addindex(query command)\addindex(command!query)
The query operator without any right value shows the contents of a parser item
in a human readable way. By passing a right value to the query operator, the
type of query can be influenced. A query operator together with its right
value forms a <em>query command</em>. Valid query commands
are:
\addindex(\c .?.)
\addindex(\c .?cmds.)
\addindex(\c .?val.)
\addindex(\c .?type.)
\addindex(\c .?perm.)
\addindex(\c .?range.)
\addindex(\c .?subst.)
\addindex(\c .?entries.)
	- <b>?</b>: Show contents of a parser element.
	- <b> ?cmds</b>: Show a list of all query commands for this element.
	- <b> ?help</b>: Show the detailed description of an element.
	- <b> ?val</b>: Return the value of an element.
	- <b> ?type</b>: Return the data type of an element.
	- <b> ?perm</b>: Return the access rights for an element.
	- <b> ?range</b>: Return the range of valid values for this variable.
	- <b> ?subst</b>: Show all variable substitutions applied to this node.
	- <b> ?entries</b>: Show a list of all entries in this node.
	.

Special query commands are:
\addindex(\c .?save .)
\addindex(\c .?saveshort.)
\addindex(\c .?savemons.)
\addindex(\c .?read.)
	- <b> ?save:filename</b>: Save the contents of this node into the
text file "filename", complete with element description comments.
	- <b> ?saveshort:filename</b>: Save the contents of this node into the
text file "filename", without additional comments or blank lines.
	- <b> ?savemons:filename</b>: Save the contents of all monitor variables to
  the file 'filename'.
	- <b> ?read:filename</b>: Read the file "filename" into the current
parser node.

\subsection mltdimvars Multidimensional variables
\addindex(multidimensional variable)
\addindex(variable!multidimensional)

The \mha configuration language supports vectors and matrices in a way similar
to the \Matlab notation: Vectors are put into squared brackets, with
the items separated by withespace (\Matlab row vectors). Matrices are
noted as vectors of vectors, with each vector separated by a semicolon
from the other vectors:

\verbatim
vector = [1.0 2.7 4]
matrix = [[1 2 3];[4 5 6]]
\endverbatim

Vectors with real values (vector<float>, library class
\c vfloat_t support also the special notation
\c min:increment:max. A mixture of explicit and incremential
notation is allowed. The vector is internally expanded and will return
the explicit notation on read:

\verbatim
vector = [1.0 1.7 2.1:1.1:5]
\endverbatim

This will be expanded as:

\verbatim
vector = [1.0 1.7 2.1 3.2 4.3]
\endverbatim

\subsection complexvars Complex variables
\addindex(complex variable)
\addindex(variable!complex)

Variables with complex values are notated in parenthesis as a sum of
real and imaginary part. Pure real values can be noted without
parenthesis:

\verbatim
complex = (1.3 + 2.7i)
vcomplex = [(1.3 + 2.7i) (2.0 - 1.1i) 6.3]
\endverbatim

\subsection txtvars Text variables \addindex(text variable) \addindex(variable!text)

Strings in the \mha configuration language can contain any characters. Special
characters do not have to be quoted; quote characters are treated
literally. Leading and trailing whitespace of strings is automatically
removed. Vector elements in string vectors are separated by a single
space character. This means that vector elements
(\c vstring_t, \c kw_t) can not contain spaces.

\verbatim
string =  This is a valid text string.
samestr=This is a valid text string.
strvec = [pears bananas green_apples]
\endverbatim

\subsection varranges Variable ranges \addindex(variable range)\addindex(range)

Numeric variables can have a restricted range, the value of keyword list 
variables is always restricted to one of the keywords. New values are checked 
against this range when the variable is changed through the \mha configuration 
language interface. For numeric variables, the range can be 
\f$[x_{min},x_{max}]\f$ (boundaries included), \f$]x_{min},x_{max}[\f$
(boundaries excluded) or a mixed version of both. If \f$x_{min}\f$ or
\f$x_{max}\f$ are omitted then the variable will not have a lower or
upper boundary.

For keyword list variables, the range is simply a space separated list of valid
entries.

\subsection subsvars Variable Substitution and Environment Variables
\addindex(substitution)\addindex(environment variable)\addindex(variable!environment)

Each node in the \mha configuration tree can define a set of text
substitutions.  The pattern to be replaced has the form "\$[VARNAME]",
where VARNAME can be any text. Any occurrence of this pattern is
replaced. The set of substitutions can be queried with the "?subst"
query command. Replacements can be activated with the "?addsubst"
query command in the style
\c ?addsubst:<VARNAME> <REPLACEMENT>. Each parser node has its own
set of text substitutions, which is not inherited by children parser
nodes.

Environment variables can be used in the \mha configuration language in the form
"\$\{VARNAME\}", where VARNAME is the name of an environment variable. Each
occurrence of \$\{VARNAME\} is replaced by its contents before interpreting the
\mha configuration language, i.e.\ the left hand side or even operators can be part of
the substitution.

\section commplugs Communication between \mha Plugins \addindex(AC variable)\addindex(variable!AC)\addindex(communication)

Interaction of algorithms is a major issue in hearing aid
development. In order to systematically analyse and control
interaction problems, the \mha chain plugins 'mhachain' and
'concurrentchains' provide a mechanism for sharing parameters and
states between algorithms. Any algorithm plugin can register selected
algorithm communication (AC) variables (any data segment) to be public
within one signal processing
chain. Other algorithms within the same processing chain can read and
modify these AC variables. AC variables are accessed by name. Type and
dimension are checked on each access. This concept does not only
provide analysis of interaction aspects but also modular combination
of signal processing strategies, e.g.\ separation of noise estimators
and noise reduction strategies in different logical processing
stages. A detailed description of the programming interface can be
found in the Programmers Reference Handbook.

*/ 
