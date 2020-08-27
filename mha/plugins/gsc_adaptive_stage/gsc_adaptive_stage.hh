// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2013 2014 2016 2018 2020 HörTech gGmbH
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

#ifndef SHYNKADAPTIVE_H
#define SHYNKADAPTIVE_H

namespace gsc_adaptive_stage {

/** Small constant to ensure no division by zero occurs */
constexpr mha_real_t DELT=1e-12;

class gsc_adaptive_stage {

public:
  gsc_adaptive_stage(algo_comm_t & ac, const mhaconfig_t,
                int lenOldSamps, bool doCircularComp,
                float mu, float alp, bool useVAD,
                const std::string& vadName_);

  ~gsc_adaptive_stage()=default;
  mha_wave_t* process(mha_wave_t* wavin);


private:

  void insert();

  /** Handle to AC space */
  algo_comm_t ac;
  /** Number of old samples to buffer */
  unsigned int lenOldSamps;
  /** Number of new samples*/
  unsigned int lenNewSamps;
  /** Total buffer size. Must be lenOldSamps+lenNewSamps */
  unsigned int bufSize;
  /** Fraction of new samples to total buffer size */
  float frac_old;
  /** FFT handle */
  mha_fft_t mha_fft;
  /** Number of frequency bins */
  unsigned int nfreq;
  /** Number of channels in input signal */
  unsigned int nchan;
  /** Channel index containing the desired response. Always last channel by convention */
  unsigned int desired_chan;

  /** Whether to compensate for circular convolution */
  bool doCircularComp;

  /** Linear coefficient for gradient */
  float mu;
  /** Autoregressive coefficient for estimating PSD */
  float alp;
  /** Wether to use VAD */
  bool useVAD;
  /** Name of VAD AC variable */
  std::string vadName;

  /** Buffered input signal */
  MHA_AC::waveform_t x;
  /** FFT of the buffered input signal */
  MHA_AC::spectrum_t X;
  /** Time-varying filter */
  MHA_AC::spectrum_t W;
  /** Filter output, frequency domain */
  MHA_AC::spectrum_t Y;
  /** Filter output, time domain */
  MHA_AC::waveform_t y;
  /** Desired response */
  MHA_AC::waveform_t d;
  /** Error signal, time domain */
  MHA_AC::waveform_t e;
  /** Error signal, frequency domain */
  MHA_AC::spectrum_t E;
  /** Error spectrum multiplied by input spectrum: E2=X*E */
  MHA_AC::spectrum_t E2;
  /** Gradient */
  MHA_AC::waveform_t grad;
  /** FT of the gradient */
  MHA_AC::spectrum_t Grad;
  /** Error signal */
  MHA_AC::waveform_t e_out;
  /** Signal power estimate*/
  MHA_AC::waveform_t P;
  /** Signal power estimate, summed over all channels */
  MHA_AC::waveform_t Psum;
};
}
#endif // SHYNKADAPTIVE_H

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
