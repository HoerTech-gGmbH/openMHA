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

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
