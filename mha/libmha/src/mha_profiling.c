// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2012 2013 2016 HörTech gGmbH
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

#include "mha_profiling.h"
#include <string.h>

void mha_tic(mha_tictoc_t* t)
{
    memset(t,0,sizeof(mha_tictoc_t));
#ifndef _WIN32
    gettimeofday(&(t->tv1),&(t->tz));
#else
      
#endif
}

void mha_platform_tic(mha_platform_tictoc_t * t) {
#ifndef _WIN32
  mha_tic(t);
#else
  memset(t,0,sizeof(mha_platform_tictoc_t));
  QueryPerformanceCounter(&t->tv1);
#endif
}

float mha_toc(mha_tictoc_t* t)
{
#ifndef _WIN32
    gettimeofday(&(t->tv2),&(t->tz));
    t->tv2.tv_sec -= t->tv1.tv_sec;
    if( t->tv2.tv_usec >= t->tv1.tv_usec )
        t->tv2.tv_usec -= t->tv1.tv_usec;
    else{
        t->tv2.tv_sec --;
        t->tv2.tv_usec += 1000000;
        t->tv2.tv_usec -= t->tv1.tv_usec;
    }
    t->t = (float)(t->tv2.tv_sec) + 0.000001 * (float)(t->tv2.tv_usec);
#else
    t->t = 1.0;
#endif
    return t->t;
}

float mha_platform_toc(mha_platform_tictoc_t * t) {
#ifndef _WIN32
  return mha_toc(t);
#else
  LARGE_INTEGER frequency; 
  QueryPerformanceCounter(&t->tv2);
  QueryPerformanceFrequency(&frequency);
  t->t = (float)
    (((double)(t->tv2.QuadPart - t->tv1.QuadPart)) / frequency.QuadPart);
  return t->t;
#endif
}

/*
 * Local Variables:
 * compile-command: "make -C .."
 * coding: utf-8-unix
 * End:
 */
