// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2008 2013 2016 2017 2018 2021 HörTech gGmbH
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

#ifndef __MHA_SIGNAL_FFT_H__
#define __MHA_SIGNAL_FFT_H__

#include "mha.hh"

#include "sfftw.h"
#include "srfftw.h"

namespace MHASignal {

    class fft_t {
    public:
        fft_t( const unsigned int & );
        ~fft_t(  );
        /// fast fourier transform. if swap is set, the buffer halfes
        /// of the wave signal are exchanged before computing the fft.
        void wave2spec( const mha_wave_t *, mha_spec_t *, bool swap );
        void spec2wave( const mha_spec_t *, mha_wave_t * );
        void spec2wave( const mha_spec_t *, mha_wave_t *,unsigned int offset); 
        void forward( mha_spec_t* sIn, mha_spec_t* sOut );
        void backward( mha_spec_t* sIn, mha_spec_t* sOut );

        //gkc: scale-accurate transforms
        void wave2spec_scale( const mha_wave_t *, mha_spec_t *, bool swap );
        void spec2wave_scale( const mha_spec_t *, mha_wave_t * );
        void forward_scale( mha_spec_t* sIn, mha_spec_t* sOut );
        void backward_scale( mha_spec_t* sIn, mha_spec_t* sOut );
    private:
        unsigned int nfft;
        unsigned int n_re;
        unsigned int n_im;
        mha_real_t scale;
        void sort_fftw2spec( fftw_real * s_fftw, mha_spec_t * s_spec, unsigned int ch );
        void sort_spec2fftw( fftw_real * s_fftw, const mha_spec_t * s_spec, unsigned int ch );
        mha_real_t *buf_in;
        mha_real_t *buf_out;
        rfftw_plan fftw_plan_wave2spec;
        rfftw_plan fftw_plan_spec2wave;
        fftw_plan fftw_plan_fft;
        fftw_plan fftw_plan_ifft;
    };

}

#endif

// Local Variables:
// mode: c++
// coding: utf-8-unix
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
