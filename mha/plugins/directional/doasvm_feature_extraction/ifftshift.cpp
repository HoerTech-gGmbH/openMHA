#include <mha_toolbox.h>
#include "ifftshift.h"

void ifftshift(mha_wave_t *spec)
{
	const unsigned int center = spec->num_frames/2;

	for( unsigned int i = 0; i < center; ++i ) {
		const mha_real_t tmp = spec->buf[i];
		spec->buf[i] = spec->buf[center+i];
		spec->buf[center+i] = tmp;
	}

	if( spec->num_frames & 1 ) {
		for( unsigned int i = spec->num_frames-1; i > center; --i ) {
			const mha_real_t tmp = spec->buf[i];
			spec->buf[i] = spec->buf[i-1];
			spec->buf[i-1] = tmp;
		}
	}
}
