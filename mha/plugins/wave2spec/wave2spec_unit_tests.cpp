// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2019 2020 HörTech gGmbH
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
  /// AC variable space
  MHAKernel::algo_comm_class_t acspace = {};
  /// C handle to AC variable space
  algo_comm_t ac = {acspace.get_c_handle()};
  /// Example STFT settings used in tests
  const unsigned fftlen = 800, wndlen = 600, fragsize = 300;
  /// example input to prepare method
  mhaconfig_t signal_dimensions = {
    .channels = 2U,
    .domain = MHA_WAVEFORM,
    .fragsize = fragsize,
    .wndlen = 0U,
    .fftlen = 0U,
    .srate = 44100.0f
  };
  /// Plugin wave2spec instance used during test
  wave2spec_if_t w2s = {ac, "thread_name", "algo_name"};
  /// Waveform input example used during process callbacks in tests
  MHASignal::waveform_t waveform = {fragsize,signal_dimensions.channels};
  /// Output parameter to process used during tests
  mha_spec_t * spectrum = nullptr;
  /// Test setup prepares plugin and sets wave input to known value
  void SetUp() override {
    w2s.parse("wndlen=" + std::to_string(wndlen));
    w2s.parse("fftlen=" + std::to_string(fftlen));
    w2s.parse("wndtype=hanning");
    w2s.prepare_(signal_dimensions);
    waveform.assign(1.0f);
  }
  /// Test teardown releases plugin
  void TearDown() override {
    w2s.release();
  }
};

TEST_F(wave2spec_testing, scaling)
{
  EXPECT_EQ(2U,                     signal_dimensions.channels);
  EXPECT_EQ(unsigned(MHA_SPECTRUM), signal_dimensions.domain);
  EXPECT_EQ(fragsize,               signal_dimensions.fragsize);
  EXPECT_EQ(wndlen,                 signal_dimensions.wndlen);
  EXPECT_EQ(fftlen,                 signal_dimensions.fftlen);
  EXPECT_EQ(44100.0f,               signal_dimensions.srate);

  // We use the default zeropadding placement, 0.5, i.e. zeropadding
  // before and after analysis window should be the same (for even
  // FFT length and window length like here), and the sum of all
  // zeropadding and analysis window length should be equal to FFT length
  unsigned expected_zeropadding = (fftlen - wndlen) / 2U;
  std::string expected_zeropadding_str = std::to_string(expected_zeropadding);
  EXPECT_EQ("[" + expected_zeropadding_str +" "+ expected_zeropadding_str + "]",
            w2s.parse("zeropadding?val"));
  EXPECT_EQ(fftlen, expected_zeropadding + wndlen + expected_zeropadding);

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
}

TEST_F(wave2spec_testing, ac_variables)
{
  // Checks use ASSERT_... macros when continuing makes no sense because the
  // following checks depend on these assertions being successful.
  // Other tests use EXPECT_...
  
  // AC space accessor
  comm_var_t cv = {0,0,0,nullptr};
  // AC variables exist already after prepare:
  // "algo_name" is a spectrum with 2 channels and 401 (fftlen/2+1) bins
  ASSERT_EQ(0, ac.get_var(ac.handle, "algo_name", &cv)); // exists
  EXPECT_EQ(unsigned(MHA_AC_MHACOMPLEX), cv.data_type);
  EXPECT_EQ(signal_dimensions.channels * (fftlen / 2 + 1), cv.num_entries);
  EXPECT_EQ(fftlen/2+1, cv.stride);
  ASSERT_NE(nullptr, cv.data); // points to something
  void const * const spec_ptr = cv.data;

  // "algo_name_wnd" is a real array with wndlen entries
  ASSERT_EQ(0, ac.get_var(ac.handle, "algo_name_wnd", &cv)); // exists
  EXPECT_EQ(unsigned(MHA_AC_MHAREAL), cv.data_type);
  EXPECT_EQ(wndlen, cv.num_entries);
  EXPECT_EQ(1U, cv.stride);
  ASSERT_NE(nullptr, cv.data); // points to something
  mha_real_t const * const wnd_ptr = static_cast<mha_real_t*>(cv.data);

  // to test that wave2spec re-inserts both AC variables during processing,
  // we shadow the variables temporarily
  int shadow1 = 0, shadow2 = 0;
  ac.insert_var_int(ac.handle, "algo_name", &shadow1);
  ac.insert_var_int(ac.handle, "algo_name_wnd", &shadow2);
  ASSERT_EQ(0, ac.get_var(ac.handle, "algo_name", &cv)); // exists
  EXPECT_NE(spec_ptr, cv.data);
  ASSERT_EQ(0, ac.get_var(ac.handle, "algo_name_wnd", &cv)); // exists
  ASSERT_NE(wnd_ptr, cv.data); // was shadowed

  // Calling process should restore the AC variables
  w2s.process(&waveform, &spectrum);
  ASSERT_EQ(0, ac.get_var(ac.handle, "algo_name", &cv)); // exists
  EXPECT_EQ(spec_ptr, cv.data);
  ASSERT_EQ(0, ac.get_var(ac.handle, "algo_name_wnd", &cv)); // exists
  EXPECT_EQ(wnd_ptr, cv.data);
}

TEST_F(wave2spec_testing, zeropadding_distribution)
{
  // Test parameters for testing zeropadding
  struct {
    std::string id;   // test case identifier
    unsigned fftlen;  // FFT length in samples
    unsigned wndlen;  // window length in samples
    unsigned hopsize; // fragsize = window shift = hop size in samples
    float wndpos;     // Placement of analysis window in FFT buffer
    unsigned npad1;   // Expected zeropadding in samples before analysis window
    unsigned npad2;   // Expected zeropadding in samples after analysis window
  } zeropadding_test_parameters[] =
      {
       // symmetric zero-padding:
       {"test_case_0", 512, 256, 128, 0.5f, 128, 128},
       // analysis window first, trailing zeropadding:
       {"test_case_1", 512, 256, 128, 0.0f,   0, 256},
       // analysis window at end, zeropadding in front:
       {"test_case_2", 512, 256, 128, 1.0f, 256,   0},
       // odd window length: symmetric zeropadding gets asymmetric from rounding
       {"test_case_3", 512,   1,   1, 0.5f, 255, 256},
       // analysis window first, trailing zeropadding:
       {"test_case_4", 512,   1,   1, 0.0f,   0, 511},
       // analysis window at end, zeropadding in front:
       {"test_case_5", 512,   1,   1, 1.0f, 511,   0}
      };
  unsigned channels = 2U;
  for (auto params : zeropadding_test_parameters) {
    auto runtime =
      std::make_unique<wave2spec_t>(params.fftlen, params.wndlen,
                                    params.hopsize, channels, params.wndpos,
                                    MHAWindow::hanning_t(params.wndlen),
                                    ac, params.id);
    EXPECT_EQ(params.npad1, runtime->get_zeropadding(false)) << params.id;
    EXPECT_EQ(params.npad2, runtime->get_zeropadding(true)) << params.id;
  }
}

// Local Variables:
// compile-command: "make unit-tests"
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
