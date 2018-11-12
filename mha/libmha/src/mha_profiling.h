// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2012 2013 2016 2017 2018 HörTech gGmbH
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

#ifndef MHA_PROFILING_H
#define MHA_PROFILING_H

#ifndef _WIN32
#include <sys/time.h>

typedef struct {
    struct timeval tv1;
    struct timeval tv2;
    struct timezone tz;
    float t;
} mha_tictoc_t;

typedef mha_tictoc_t mha_platform_tictoc_t;

#else

typedef struct {
    float t;
} mha_tictoc_t;

#include <windows.h>
typedef struct {
    LARGE_INTEGER tv1;
    LARGE_INTEGER tv2;
    float t;
} mha_platform_tictoc_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

void mha_platform_tic(mha_platform_tictoc_t* t);
float mha_platform_toc(mha_platform_tictoc_t* t);

#ifdef __cplusplus
}
#endif

#endif

/* Local Variables:        */
/* coding: utf-8-unix      */
/* c-basic-offset: 4       */
/* indent-tabs-mode: nil   */
/* End:                    */
