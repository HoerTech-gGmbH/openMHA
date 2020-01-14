// This file is part of the HörTech Open Master Hearing Aid (openMHA)
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

#ifdef __clang__
# define COMPILER_ID_VENDOR "clang"
# define COMPILER_ID_MAJOR __clang_major__
# define COMPILER_ID_MINOR __clang_minor__
# define COMPILER_ID_PATCH __clang_patchlevel__
#else
# define COMPILER_ID_VENDOR "gcc"
# define COMPILER_ID_MAJOR __GNUC__
# define COMPILER_ID_MINOR __GNUC_MINOR__
# define COMPILER_ID_PATCH __GNUC_PATCHLEVEL__
#endif
#define COMPILER_ID_VERSION_HELPER2(x,y,z) #x "." #y "." #z
#define COMPILER_ID_VERSION_HELPER1(x,y,z) COMPILER_ID_VERSION_HELPER2(x,y,z)
#define COMPILER_ID_VERSION COMPILER_ID_VERSION_HELPER1(COMPILER_ID_MAJOR, \
                                                        COMPILER_ID_MINOR, \
                                                        COMPILER_ID_PATCH)
#if __cplusplus == 201402L
# define COMPILER_ID_STANDARD "c++14"
#endif
#if __cplusplus == 201703L
# define COMPILER_ID_STANDARD "c++17"
#endif
#ifndef COMPILER_ID_STANDARD
# error "unsupported value of predefined macro __cplusplus"
#endif

// The compiler id will be sth like "gcc-8.2.0-c++14" or "clang-6.0.0-c++17"
#define COMPILER_ID \
  COMPILER_ID_VENDOR "-" COMPILER_ID_VERSION "-" COMPILER_ID_STANDARD

// Local Variables:
// compile-command: "make -C .."
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
