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

#include "overlapadd.hh"
#include "mha_algo_comm.hh"
#include <gtest/gtest.h>
#include <random>

class overlapadd_testing : public ::testing::Test {
public:
  MHA_AC::algo_comm_class_t ac;
  const unsigned fftlen = 800, wndlen = 600, fragsize = 300;
  MHAParser::window_t window = {"window type"};
  MHAParser::window_t zerowindow = {"zero padding post window type"};
  mhaconfig_t signal_dimensions_in = {
    .channels = 2U,
    .domain = MHA_WAVEFORM,
    .fragsize = fragsize,
    .wndlen = wndlen,
    .fftlen = fftlen,
    .srate = 44100.0f
  }, signal_dimensions_out = signal_dimensions_in;
  const float exponent = 1, position = 0.5;
  float prescale = 0, postscale = 0;
  overlapadd::overlapadd_t ola;
  MHASignal::waveform_t waveform;
  overlapadd_testing()
    : ola(signal_dimensions_in, signal_dimensions_out,
          exponent, position, window, zerowindow,
          prescale, postscale),
      waveform(fragsize,signal_dimensions_in.channels)
  {
    waveform.assign(1.0f);
  }
  void fill_wave_with_noise_of_rms_1_with_seed(size_t seed)
  {
    std::mt19937 rng;
    std::uniform_real_distribution<float> white(-1.0f,1.0f);
    rng.seed(seed);
    for (unsigned ch = 0; ch < waveform.num_channels; ++ch) {
      float sum_of_squares = 0;
      for (unsigned k = 0; k < waveform.num_frames; ++k) {
        const float & s = waveform(k,ch) = white(rng);
        sum_of_squares += s*s;
      }
      const float norm = 1/sqrtf(sum_of_squares / waveform.num_frames);
      for (unsigned k = 0; k < waveform.num_frames; ++k) {
        waveform(k,ch) *= norm;
      }
    }
  }

  void wave2spec_hop_forward(mha_wave_t * s)
  {ola.wave2spec_hop_forward(s);}
  void wave2spec_apply_window(void)
  {ola.wave2spec_apply_window();}
  mha_spec_t * wave2spec_compute_fft()
  {return ola.wave2spec_compute_fft();}
  MHASignal::waveform_t & get_wave_in1()
  {return ola.wave_in1;}
  MHASignal::waveform_t & get_wave_out1()
  {return ola.wave_out1;}
};

TEST_F(overlapadd_testing, scaling_rectangular_signal)
{
  ola.wave2spec(&waveform); // Call twice to fill the window:
  mha_spec_t * spectrum = ola.wave2spec(&waveform);

  // comparing mean squares:
  mha_real_t wave_ms = 1.0; // 300 * (1.0^2) / 300

  mha_real_t spec_ms = 0;    // accumulate squared bins below
  for (unsigned bin = 0; bin < spectrum->num_frames; ++bin) {
    float weight = 2;         // negative frequencies
    if (bin == 0)
      weight = 1;             // dc has no negative frequency counterpart
    if ((bin == fftlen / 2U + 1U) && ((fftlen & 1U) == 0U))
      weight = 1;             // nyqvist has no negative frequency counterpart
    spec_ms += abs2(value(spectrum,bin,0)) * weight;
  }
  EXPECT_NEAR(wave_ms, spec_ms, 1e-6);
}

TEST_F(overlapadd_testing, scaling_noise_signal)
{
  // The level should be correct on average but may differ in each block
  fill_wave_with_noise_of_rms_1_with_seed(-1);
  ola.wave2spec(&waveform);
  double sum = 0;
  for (unsigned iterations = 0; iterations < 100U; ++ iterations) {
    fill_wave_with_noise_of_rms_1_with_seed(iterations);
    mha_spec_t * spectrum = ola.wave2spec(&waveform);

    mha_real_t wave_ms = 1.0; // Because it was designed that way:
    EXPECT_NEAR(waveform.get_size(), waveform.sumsqr(), 1e-3) << iterations;

    mha_real_t spec_ms = 0;    // accumulate squared bins below
    for (unsigned bin = 0; bin < spectrum->num_frames; ++bin) {
      float weight = 2;         // negative frequencies
      if (bin == 0)
        weight = 1;             // dc has no negative frequency counterpart
      if ((bin == fftlen / 2U + 1U) && ((fftlen & 1U) == 0U))
        weight = 1;             // nyqvist has no negative frequency counterpart
      spec_ms += abs2(value(spectrum,bin,0)) * weight;
    }
    EXPECT_NEAR(sqrtf(wave_ms), sqrtf(spec_ms), 0.05) << iterations;
    sum += sqrtf(spec_ms);
  }
  EXPECT_NEAR(1, sum / 100, 1e-4);
}

TEST_F(overlapadd_testing, individual_steps)
{
  wave2spec_hop_forward(&waveform);
  for (unsigned ch = 0; ch < signal_dimensions_in.channels; ++ch)
    for (unsigned k = 0; k < get_wave_in1().num_frames; ++k)
      ASSERT_EQ(k >= get_wave_in1().num_frames - waveform.num_frames,
                get_wave_in1().value(k,ch)) << "ch="<<ch << ", k="<<k;

  wave2spec_apply_window();

  float sum_of_hanning_squares = 0;
  for (unsigned k = 0; k < wndlen; ++k) {
    float hanning = (1-cos(2*M_PI*(k)/wndlen))/2;
    sum_of_hanning_squares += hanning * hanning;
  }
  float hanning_rms = sqrt(sum_of_hanning_squares / wndlen);

  EXPECT_NEAR(1.88562,1/hanning_rms*sqrt(float(fftlen)/wndlen), 1e-5);
  EXPECT_NEAR(1.88562,sqrt(fftlen / sum_of_hanning_squares), 1e-5);

  for (unsigned ch = 0; ch < signal_dimensions_in.channels; ++ch)
    for (unsigned k = 0; k < get_wave_out1().num_frames; ++k)
      if (k < 400U || k >= 700U) // only 2nd half of window is filled
        ASSERT_EQ(0, get_wave_out1().value(k,ch)) << "ch="<<ch << ", k="<<k;
      else
        // TODO: Where does this number come from?
        ASSERT_NEAR((1-cos(2*M_PI*(k-100)/wndlen))/2 * 1.88562,
                    get_wave_out1().value(k,ch),
                    1e-5) << "ch="<<ch << ", k="<<k;

}

TEST_F(overlapadd_testing, manual_scaling)
{
  // compute the RMS of the analysis window, a hanning window of length 600
  double sum_squares = 0;
  for (unsigned k = 0; k < wndlen; ++k) {
    float window_sample = (1 - cos(2*M_PI*k/wndlen))/2;
    sum_squares += window_sample * window_sample;
  }
  float hanning_rms = sqrtf(sum_squares / wndlen);
  // compare hanning_rms for wndlen=600 with result from matlab
  EXPECT_NEAR(0.6123724357, hanning_rms, 1e-7);

  // Is the RMS of the normalized window equal to 1?
  sum_squares = 0;
  for (unsigned k = 0; k < wndlen; ++k) {
    float window_sample = (1 - cos(2*M_PI*k/wndlen))/2/hanning_rms;
    sum_squares += window_sample * window_sample;
  }
  float normalized_window_rms = sqrtf(sum_squares / wndlen);
  EXPECT_NEAR(1, normalized_window_rms, 1e-7);

  // Is the RMS of the fft buffer equal to 1 when we adjust for the zeropadding?
  sum_squares = 0;
  get_wave_out1().assign(0.0f);
  for (unsigned k = 0; k < wndlen; ++k) {
    float window_sample = (1 - cos(2*M_PI*k/wndlen))/2/hanning_rms * sqrtf(float(fftlen) / wndlen);
    get_wave_out1().value(k+100,0) = window_sample;
    sum_squares += window_sample * window_sample;
  }
  float fft_normalized_window_rms = sqrtf(sum_squares / fftlen);
  EXPECT_NEAR(1, fft_normalized_window_rms, 3e-7);

  // perform the FFT. Has the result the expected scaling?
  mha_spec_t * spectrum = wave2spec_compute_fft();
  sum_squares = 0;
  for (unsigned bin = 0; bin < spectrum->num_frames; ++bin) {
    float weight = 2;         // negative frequencies
    if (bin == 0)
      weight = 1;             // dc has no negative frequency counterpart
    if ((bin == fftlen / 2U + 1U) && ((fftlen & 1U) == 0U))
      weight = 1;             // nyqvist has no negative frequency counterpart
    sum_squares += abs2(value(spectrum,bin,0)) * weight;
  }
  EXPECT_NEAR(1, sum_squares, 1e-6);
}

// Local Variables:
// compile-command: "make unit-tests"
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
