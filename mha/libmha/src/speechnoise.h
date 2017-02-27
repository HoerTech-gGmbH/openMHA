// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2007 2010 2011 2013 2016 HörTech gGmbH
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

#ifndef SPEECHNOISE_H
#define SPEECHNOISE_H

#include "mha_signal.hh"

class speechnoise_t : public MHASignal::waveform_t
{
public:
    typedef enum { mha, olnoise, LTASS_combined, LTASS_female, LTASS_male, white, pink, brown, TEN_SPL, TEN_SPL_250_8k, TEN_SPL_50_16k, sin125, sin250, sin500, sin1k, sin2k, sin4k, sin8k } noise_type_t;
    speechnoise_t(float duration, float srate, unsigned int channels, speechnoise_t::noise_type_t noise_type = speechnoise_t::mha);
    speechnoise_t(unsigned int length_samples, float srate, unsigned int channels, speechnoise_t::noise_type_t noise_type = speechnoise_t::mha);
private:
    void creator(speechnoise_t::noise_type_t noise_type, float srate);
};

#endif

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
