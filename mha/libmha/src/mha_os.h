// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2009 2013 2014 2016 2017 2018 HörTech gGmbH
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

#ifndef MHA_OS_H
#define MHA_OS_H

#include <list>

#if defined(__BORLANDC__) && (__BORLANDC__ < 0x560) || defined(_MSC_VER)
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
#else
#include <stdint.h>
#endif

#ifdef _WIN32
/*
 *
 * Windows specific definitions and includes:
 *
 */

#include <windows.h>
#ifdef __BORLANDC__
#include <winsock2.h>
#endif
#define mha_loadlib(x) LoadLibrary(x)
#define mha_freelib(x) FreeLibrary(x)
#define mha_freelib_success(x) (x != 0)
#define mha_getlibfun(h,x) x ## _cb = (x ## _t)GetProcAddress(h,"_" #x)
#define mha_getlibfun_checked(h,x) x ## _cb = (x ## _t)GetProcAddress(h,"_" #x);if(! x ## _cb) throw MHA_Error(__FILE__,__LINE__,"Function " #x " is undefined.")
#define mha_loadlib_error(x) "(MHA: replace this macro for Windows platforms.)"
#define mha_lib_extension ".dll"
#define mha_msleep(milliseconds) Sleep(milliseconds)
typedef HINSTANCE mha_libhandle_t;
/// printf modifier to print integers of type size_t
#define FMTsz "%u"
#else
/*
 *
 * Definitions and includes for non-windows platforms:
 *
 */

#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <netinet/in.h>

#define mha_loadlib(x) dlopen(x,RTLD_NOW)
#define mha_freelib(x) dlclose(x)
#define mha_freelib_success(x) (x == 0)
#define mha_getlibfun(h,x) x ## _cb = (x ## _t)dlsym(h,#x)
#define mha_getlibfun_checked(h,x) x ## _cb = (x ## _t)dlsym(h,#x);if(! x ## _cb) throw MHA_Error(__FILE__,__LINE__,"Function " #x " is undefined.")
#define mha_loadlib_error(x) dlerror()
#ifdef __APPLE__
#define mha_lib_extension ".dylib"
#else
#define mha_lib_extension ".so"
#endif
#define mha_msleep(milliseconds) usleep((milliseconds)*1000)
typedef void* mha_libhandle_t;
/// printf modifier to print integers of type size_t
#define FMTsz "%zu"
#endif

#define MHA_RESOLVE(h,t) t ## _cb = (t ## _t)(h->resolve(#t))
#define MHA_RESOLVE_CHECKED(h,t) t ## _cb = (t ## _t)(h->resolve_checked(#t))

#ifdef __cplusplus
#include <string>


std::string mha_getenv(std::string envvar); 

class dynamiclib_t {
public:
    dynamiclib_t(const std::string&);
    void* resolve(const std::string&);
    void* resolve_checked(const std::string&);
    ~dynamiclib_t();
    const std::string& getmodulename() const {return modulename;};
    const std::string& getname() const {return fullname;};
private:
    std::string fullname;
    std::string modulename;
    mha_libhandle_t h;
};
#endif

std::list<std::string> mha_library_paths();

std::list<std::string> list_dir(const std::string& path,const std::string& pattern);


inline 
void mha_hton(float* data,unsigned int len)
{
    for(unsigned int k=0;k<len;k++)
        *((uint32_t*)(&(data[k]))) = htonl(*((uint32_t*)(&(data[k]))));
}

inline 
void mha_ntoh(float* data,unsigned int len)
{
    for(unsigned int k=0;k<len;k++)
        *((uint32_t*)(&(data[k]))) = ntohl(*((uint32_t*)(&(data[k]))));
}

inline 
void mha_hton(uint32_t* data,unsigned int len)
{
    for(unsigned int k=0;k<len;k++)
        *((uint32_t*)(&(data[k]))) = htonl(*((uint32_t*)(&(data[k]))));
}

inline 
void mha_ntoh(uint32_t* data,unsigned int len)
{
    for(unsigned int k=0;k<len;k++)
        *((uint32_t*)(&(data[k]))) = ntohl(*((uint32_t*)(&(data[k]))));
}

inline 
void mha_hton(int32_t* data,unsigned int len)
{
    for(unsigned int k=0;k<len;k++)
        *((uint32_t*)(&(data[k]))) = htonl(*((uint32_t*)(&(data[k]))));
}

inline 
void mha_ntoh(int32_t* data,unsigned int len)
{
    for(unsigned int k=0;k<len;k++)
        *((uint32_t*)(&(data[k]))) = ntohl(*((uint32_t*)(&(data[k]))));
}

#endif

/*
  Local Variables:
  mode: c++
  compile-command: "make -C .."
  coding: utf-8-unix
  End:
 */
