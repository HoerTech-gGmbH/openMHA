// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2014 2018 HörTech gGmbH
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

#include <stdlib.h>
#include <math.h>
#include "hann.h"

#define PI 3.14159265358979323846

float* hannf(const unsigned int N)
{
	float* win = (float*)malloc(N*sizeof(float));

    if( win == NULL ) return NULL;

	for( unsigned int i = 0; i < N; ++i )
		win[i] = (1 - cosf(2*PI*i/N))/2;

	return win;
}

double* hann(const unsigned int N)
{
	double* win = (double*)malloc(N*sizeof(double));

    if( win == NULL ) return NULL;

	for( unsigned int i = 0; i < N; ++i )
		win[i] = (1 - cos(2*PI*i/N))/2;

	return win;
}

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
