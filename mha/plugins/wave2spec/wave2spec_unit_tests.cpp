// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2019 HörTech gGmbH
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

#include <gtest/gtest.h>
#include "wave2spec.hh"
#include "mha_algo_comm.hh"

class wave2spec_testing : public ::testing::Test {
public:
  MHAKernel::algo_comm_class_t ac;
};

TEST_F(wave2spec_testing, scaling)
{
  const unsigned fftlen = 800, wndlen = 600, fragsize = 300;
  wave2spec_if_t w2s(ac.get_c_handle(), "thread_name", "algo_name");
  w2s.parse("wndlen=" + std::to_string(wndlen));
  w2s.parse("fftlen=" + std::to_string(fftlen));
  w2s.parse("wndtype=hanning");
  mhaconfig_t signal_dimensions = {
    .channels = 2U,
    .domain = MHA_WAVEFORM,
    .fragsize = fragsize,
    .wndlen = 0U,
    .fftlen = 0U,
    .srate = 44100.0f
  };
  w2s.prepare_(signal_dimensions);
  EXPECT_EQ(2U,                     signal_dimensions.channels);
  EXPECT_EQ(unsigned(MHA_SPECTRUM), signal_dimensions.domain);
  EXPECT_EQ(fragsize,               signal_dimensions.fragsize);
  EXPECT_EQ(wndlen,                 signal_dimensions.wndlen);
  EXPECT_EQ(fftlen,                 signal_dimensions.fftlen);
  EXPECT_EQ(44100.0f,               signal_dimensions.srate);

  MHASignal::waveform_t waveform(fragsize,signal_dimensions.channels);
  waveform.assign(1.0f);
  mha_spec_t * spectrum = nullptr;
  w2s.process(&waveform, &spectrum); // Call twice to fill the window
  w2s.process(&waveform, &spectrum);
  mha_real_t wave_rms = 1.0; // 300 * (1.0^2) / 300

  mha_real_t spec_rms = 0;    // accumulate squared bins below
  for (unsigned bin = 0; bin < spectrum->num_frames; ++bin) {
    float weight = 2;         // negative frequencies
    if (bin == 0)
      weight = 1;             // dc has no negative frequency counterpart
    if ((bin == fftlen / 2U + 1U) && ((fftlen & 1U) == 0U))
      weight = 1;             // nyqvist has no negative frequency counterpart
    spec_rms += abs2(value(spectrum,bin,0)) * weight;
  }
  EXPECT_NEAR(wave_rms, spec_rms, 1e-6);
  w2s.release();
}

// Local Variables:
// compile-command: "make unit-tests"
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
