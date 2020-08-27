// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2003 2004 2005 2006 2008 2009 2013 2014 2016 2017 HörTech gGmbH
// Copyright © 2018 2019 2020 HörTech gGmbH
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


#ifndef MHA_ERROR_HH
#define MHA_ERROR_HH

#ifdef __cplusplus

#include <exception>

#ifdef MHA_DEBUG
#define Getmsg(e) ((e).get_longmsg())
#else
#define Getmsg(e) ((e).get_msg())
#endif

class MHA_Error : public std::exception {
public:
    MHA_Error(const char* file,int line,const char* fmt,...)
        __attribute__        // Let compiler check format strings
        ((__format__(
#ifdef _WIN32 // MinGW needs format gnu_printf to accept %zu for size_t
                     gnu_printf,
#else         // All other platforms accept %zu with format printf
                     printf,
#endif 
                             // invisible "this" is 1st parameter
                     4,      // format string is in 4th parameter
                     5       // varargs "..." starts at 5th parameter
                     )
          ));
    MHA_Error(const MHA_Error&);
    MHA_Error& operator=(const MHA_Error&);
    ~MHA_Error() throw ();
    /** Return the error message without source position*/
    const char* get_msg() const {return msg;};
    /** Return the error message with source position*/
    const char* get_longmsg() const {return longmsg;};
    /** overwrite std::execption::what()*/
    const char* what() const throw () {return Getmsg(*this);} ;
private:
    char* msg;
    char* longmsg;
};

/**
   \ingroup mhaerror
   \brief Throw an \mha error with a text message.
   \param x Text message.
 */
#define MHA_ErrorMsg(x) MHA_Error(__FILE__,__LINE__,"%s",x)

/**
   \ingroup mhaerror
   \brief Assertion macro, which throws an MHA_Error.
   \param x Boolean expression which should be true.
 */
#define MHA_assert(x) if(!(x)) throw MHA_Error(__FILE__,__LINE__,"\"%s\" is false.",#x)

/**
   \ingroup mhaerror
   \brief Equality assertion macro, which throws an MHA_Error with the values.
   \param a Numeric expression which can be converted to double (for printing).
   \param b Numeric expression which should be equal to a
 */
#define MHA_assert_equal(a,b) if( a != b ) throw MHA_Error(__FILE__,__LINE__,"\"%s == %s\" is false (%s = %g, %s = %g).",#a,#b,#a,(double)(a),#b,(double)(b))

#endif /* __cplusplus */

/**
   \ingroup mhaerror
   \brief Print an info message (stderr on Linux, OutputDebugString in Windows).
 */
void mha_debug(const char *fmt,...)
    __attribute__        // Let compiler check format strings
    ((__format__(
#ifdef _WIN32 // MinGW needs format gnu_printf to accept %zu for size_t
                     gnu_printf,
#else         // All other platforms accept %zu with format printf
                     printf,
#endif 
                     1,      // format string is in 1st parameter
                     2       // varargs "..." starts at 2nd parameter
                     )
          ));
#endif

namespace mha_error_helpers {
/**
 * Compute number of decimal digits required to represent an
 * unsigned integer.
 * @param n The unsigned integer that we want to know the number of
 *          required decimal digits for.
 * return The number of decimal digits in @c n.
 */
unsigned digits(unsigned n);

/** snprintf_required_length
 * Compute the number of bytes (excluding the terminating nul) required to
 * store the result of an snprintf.
 * @param formatstring The format string with standard printf formatstring
 * @return the number of bytes required by printf without the terminating nul
 */
unsigned snprintf_required_length(const char * formatstring, ...);

} // namespace mha_error_helpers



/*
 * Local Variables:
 * compile-command: "make -C .."
 * coding: utf-8-unix
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
