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
