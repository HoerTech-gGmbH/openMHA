// This file is part of the open HörTech Master Hearing Aid (openMHA)
// Copyright © 2021 HörTech gGmbH
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
#include "dc_simple.hh"
#include "mha_algo_comm.hh"

class dc_simple_testing : public ::testing::Test {
  public:
  // AC variable space
  MHA_AC::algo_comm_class_t acspace{};

  // C handle to AC variable space
  MHA_AC::algo_comm_t & ac {acspace};

  // Example input to prepare method
  mhaconfig_t signal_properties =  {
    .channels = 2,  .domain = MHA_WAVEFORM, .fragsize = 10,
    .wndlen   = 0, .fftlen = 0, .srate    = 16000
  };
  // Plugin instance
  dc_simple::dc_if_t dc_simple{ac,"algo"};

  /// prepares the plugin for tests for either waveform or spectrum domain
  void prepare(){
        dc_simple.prepare_(signal_properties);
        acspace.set_prepared(true);
  }

  /// releases the plugin after tests
  void release(){
        acspace.set_prepared(false);
        dc_simple.release_();
  }
};


/** Section that tests the helper functions test_fail, force_resize and not_zero*/

TEST_F(dc_simple_testing, test_helper_functions)
{
  // Test test_fail function
  // Expect no error from test_fail when channel and g50_data have same dimensions
  unsigned buscfg_channels = 1;
  std::vector<float> g50_data{10};
  EXPECT_NO_THROW(dc_simple::test_fail(g50_data, buscfg_channels, 
    "Correct channels"));

  // Expect no error because g50_data dimension is 1 and is treated by force resize function, 
  // if channel dimensions don't match with data
  buscfg_channels = 2;
  EXPECT_NO_THROW(dc_simple::test_fail(g50_data, buscfg_channels, 
    "Incorrect channels but equal to data has only 1 entry"));

  // Plugin should throw error from test_fail when channels are set incorrectly compared to g50_data
  g50_data = {10, 25, 40};
  EXPECT_THROW(dc_simple::test_fail(g50_data, buscfg_channels, 
    "Incorrect channels but g50_data is greater than 1"), MHA_Error);

  // Test force_resize function
  // Plugin should throw error from force_resize when channels are set incorrectly compared to the g50_data
  EXPECT_THROW(dc_simple::force_resize(g50_data, buscfg_channels, 
    "Incorrect channels but g50_data is greater than 1"), MHA_Error);
  
  g50_data = {10};
  std::vector<float> g50_resized{10, 10};
  // Expect g50_data to be resized since there are two channels but only 1 entry for it
  EXPECT_EQ(g50_resized, dc_simple::force_resize(g50_data, buscfg_channels, 
  "Incorrect channels but equal to data has only 1 entry"));

  // Test not_zero function
  // Function should do nothing if value is not 0
  mha_real_t x = 2;
  EXPECT_EQ(2, dc_simple::not_zero(x, ""));

  // MHA should throw error from not_zero when input is 0
  x = 0;
  EXPECT_THROW(dc_simple::not_zero(0, ""), MHA_Error);
}


TEST_F(dc_simple_testing, can_be_prepared_for_spectral_and_waveform_processing)
{
  // Plugin should accept signal in waveform or spectrum domain
  signal_properties.domain = MHA_SPECTRUM;
  EXPECT_NO_THROW(prepare());
  release();

  signal_properties.domain = MHA_WAVEFORM;
  EXPECT_NO_THROW(prepare());
  release();
}

/** Section of tests that check configuration parameter existence, 
 * default values and value range.*/

TEST_F(dc_simple_testing, test_parameter_g50)
{
  // default value is [0]
  EXPECT_EQ("[0]", dc_simple.parse("g50?val"));

  // limits are [-80,80]
  EXPECT_EQ("[-80,80]", dc_simple.parse("g50?range"));
  EXPECT_THROW(dc_simple.parse("g50 = 81"), MHA_Error);
  EXPECT_THROW(dc_simple.parse("g50 = -81"), MHA_Error);
}

TEST_F(dc_simple_testing, test_parameter_g80)
{
  // default value is [0]
  EXPECT_EQ("[0]", dc_simple.parse("g80?val"));

  // limits are [-80,80]
  EXPECT_EQ("[-80,80]", dc_simple.parse("g80?range"));
  EXPECT_THROW(dc_simple.parse("g80 = 81"), MHA_Error);
  EXPECT_THROW(dc_simple.parse("g80 = -81"), MHA_Error);
}

TEST_F(dc_simple_testing, test_parameter_maxgain)
{
  // default value is [80]
  EXPECT_EQ("[80]", dc_simple.parse("maxgain?val"));
}

TEST_F(dc_simple_testing, test_parameter_expansion_threshold)
{
  // default value is [0]
  EXPECT_EQ("[0]", dc_simple.parse("expansion_threshold?val"));
}

TEST_F(dc_simple_testing, test_parameter_expansion_slope)
{
  // default value is [1]
  EXPECT_EQ("[1]", dc_simple.parse("expansion_slope?val"));

  // limits are ]0,10]
  EXPECT_EQ("]0,10]", dc_simple.parse("expansion_slope?range"));
  EXPECT_THROW(dc_simple.parse("expansion_slope = -0.1"), MHA_Error);
  EXPECT_THROW(dc_simple.parse("expansion_slope = 10.1"), MHA_Error);
}

TEST_F(dc_simple_testing, test_parameter_limiter_threshold)
{
  // default value is [100]
  EXPECT_EQ("[100]", dc_simple.parse("limiter_threshold?val"));
}

TEST_F(dc_simple_testing, test_parameter_tau_attack)
{
  // default value is 0.005f
  char expected [32];
  int str_sz=snprintf(expected,32, "[%.9g]", 0.005f);
  if (str_sz < 0)
    throw MHA_Error(__FILE__, __LINE__,
                    "Implementation bug: Encoding error in snprintf");
  if (str_sz > 31)
    throw MHA_Error(
        __FILE__, __LINE__,
        "Implementation bug: String of size %i does not fit in buffer.",
        str_sz);
  EXPECT_EQ(expected, dc_simple.parse("tau_attack?val"));

  // limits are [0,]
  EXPECT_EQ("[0,]", dc_simple.parse("tau_attack?range"));
  EXPECT_THROW(dc_simple.parse("tau_attack = -0.1"), MHA_Error);
}

TEST_F(dc_simple_testing, test_parameter_tau_decay)
{
  // default value is 0.05f
  char expected [32];
  int str_sz=snprintf(expected,32, "[%.9g]", 0.05f);
  if (str_sz < 0)
    throw MHA_Error(__FILE__, __LINE__,
                    "Implementation bug: Encoding error in snprintf");
  if (str_sz > 31)
    throw MHA_Error(
        __FILE__, __LINE__,
        "Implementation bug: String of size %i does not fit in buffer.",
        str_sz);
  EXPECT_EQ(expected, dc_simple.parse("tau_decay?val"));

  // limits are [0,]
  EXPECT_EQ("[0,]", dc_simple.parse("tau_decay?range"));
  EXPECT_THROW(dc_simple.parse("tau_decay = -0.1"), MHA_Error);
}

TEST_F(dc_simple_testing, test_parameter_bypass)
{
  // default value is false
  EXPECT_EQ("no", dc_simple.parse("bypass?val"));
}


// Test dc_vars_validator_t method, which uses test_fail helper function
TEST_F(dc_simple_testing, test_variable_validator)
{
  // Plugin method throws an error if channels are set to 0
  signal_properties.channels = 0;
  EXPECT_THROW(prepare(), MHA_Error);

  // If the plugin is prepared with a channel count different than the
  // dimension of parsed variable, expect plugin to throw an error
  signal_properties.channels = 4;
  prepare();
  EXPECT_THROW(dc_simple.parse("g50 = [50 50]"), MHA_Error);
  release();

  // Expect no throw if channel count and parsed variable dimension are equal
  prepare();
  EXPECT_NO_THROW(dc_simple.parse("g50 = [50 50 50 50]"));
  release(); 

  // Expect no throw if channel count and parsed variable dimension
  // are different, but parsed variable has only 1 dimension
  prepare();
  EXPECT_NO_THROW(dc_simple.parse("g50 = [50]"));
  release();
}

/** Test Fixture for the level_smoother_t class*/
class level_smoother_t_testing : public ::testing::Test {
  public:
  // Example input to create MHASignal
  mhaconfig_t signal_properties_wav =  {
    .channels = 2,  .domain = MHA_WAVEFORM, .fragsize = 32,
    .wndlen   = 0,  .fftlen = 0,            .srate    = 16000
  };

  // Using smaller fftlen as buffer beyond fragsize are not available
  mhaconfig_t signal_properties_spec =  {
    .channels = 2,  .domain = MHA_SPECTRUM, .fragsize = 32,
    .wndlen  = 8,  .fftlen = 16,          .srate    = 16000
  };

  MHASignal::waveform_t wave{signal_properties_wav.fragsize, signal_properties_wav.channels};
  MHASignal::spectrum_t spec{signal_properties_spec.fragsize, signal_properties_spec.channels};

  // Instantiate default dc_vars_t for level_smoother_t constructor
  MHAParser::parser_t p;
  dc_simple::dc_vars_t vars{p};
  mha_real_t filter_rate = signal_properties_wav.srate;

  float db0spl = 20.0f * log10f(5e+4f * 1e-10f);
};


/** Test class level_smoother_t waveform process method with attack and 
 * release filters to 0 and waveform to 0 */
TEST_F(level_smoother_t_testing, test_level_smoother_t_wave0_attack0_decay0)
{
  p.parse("tau_attack = [0 0]");
  p.parse("tau_decay = [0 0]");

  // Set up instance of level_smoother with default waveform configuration
  dc_simple::level_smoother_t level_smoother_wav{vars, filter_rate,
                              signal_properties_wav};
  
  // Test if tauattack was registered by parser to level_smoother correctly
  const std::vector<float> tauattack = {0, 0};
  EXPECT_EQ(tauattack, vars.tauattack.data);
  
  // Process a 0 Pa waveform signal
  wave.assign(0.0f);
  mha_wave_t *level_wav =  level_smoother_wav.process(&wave);

  // Compare the output to expectation
  // Expect it to return -106.021 since pa2dbspl eps is set to 1e-10f
  for (unsigned ch = 0; ch < level_wav->num_channels; ++ch) {
    for (unsigned frame = 0; frame < level_wav->num_frames; ++frame) {
      EXPECT_FLOAT_EQ(db0spl, value(level_wav, frame, ch));
    }
  }
}

/** Test class level_smoother_t waveform process method with attack and 
 * release filters to 0 and waveform to 1 */
TEST_F(level_smoother_t_testing, test_level_smoother_t_wave1_attack0_decay0)
{ 
  p.parse("tau_attack = 0");
  p.parse("tau_decay = 0");

  // Set up instance of level_smoother with default waveform configuration
  // and above defined attack and release
  dc_simple::level_smoother_t level_smoother_wav{vars, filter_rate,
                              signal_properties_wav};
  
  // Process a 1 Pa waveform signal
  wave.assign(1.0f);
  mha_wave_t *level_wav =  level_smoother_wav.process(&wave);

  // Expect it to return 93.9794 dB (1 Pa)
  for (unsigned ch = 0; ch < level_wav->num_channels; ++ch) {
    for (unsigned frame = 0; frame < level_wav->num_frames; ++frame) {
      EXPECT_FLOAT_EQ(93.979400086720374929, value(level_wav, frame, ch));
    }
  }
}

/** Test class level_smoother_t spectrum process method with attack and 
 * release filters to 0 and spectrum 0+0i */
TEST_F(level_smoother_t_testing, test_level_smoother_t_spec0_attack0_decay0)
{
  p.parse("tau_attack = [0 0]");
  p.parse("tau_decay = [0 0]");

  // Set up instance of level_smoother with default spectrum configuration
  dc_simple::level_smoother_t level_smoother_spec{vars, filter_rate,
                              signal_properties_spec};
  
  // Assign spectrum to be 0+0i
  mha_complex_t complex{0.0f, 0.0f};
  for (unsigned ch = 0; ch < spec.num_channels; ++ch) {
    for( unsigned int k = 0; k < spec.num_frames; ++k ){
        spec.buf[spec.num_channels * k + ch] = complex;
  }}
  // Process the spectrum signal
  mha_wave_t *level_spec =  level_smoother_spec.process(&spec);
  
  // Expect it to return -106.021 since pa2dbspl eps is set to 1e-10f, where rmslevel should return 0
  for (unsigned k = 0; k < level_spec->num_channels; ++k) {
    EXPECT_FLOAT_EQ(db0spl, value(level_spec, k));
  }
}

/** Inehrit from the level_smoother_t_testing and google parameter testing class (for int) */
class level_smoother_t_filter_parametric_testing : public level_smoother_t_testing,
                public testing::WithParamInterface<int> {};

/** Test for the expected behavior of the maximum tracker decay filter */
TEST_P(level_smoother_t_filter_parametric_testing, test_level_smoother_t_max_tracker)
{ 
  // Parse data with correct channel count as force_resize is not used
  p.parse("tau_attack = [0 0]");
  // large 1 hour decay time constant set so the filter can be tested
  p.parse("tau_decay = [3600 3600]"); 

  // Set up instance of level_smoother with default waveform configuration
  dc_simple::level_smoother_t level_smoother_wav{vars, filter_rate,
                              signal_properties_wav};
 
  // Set the wave to given dB value
  wave.assign(MHASignal::dbspl2pa(GetParam()));
  // Process level_smoother
  mha_wave_t *level_wav =  level_smoother_wav.process(&wave);

  // decay filter is a maximum tracker, so it should return the maximum from 
  // the set filter_level and the signal level 
  for (unsigned ch = 0; ch < level_wav->num_channels; ++ch) {
    for (unsigned frame = 0; frame < level_wav->num_frames; ++frame) {
      EXPECT_NEAR(std::max(GetParam(), 65), value(level_wav, frame, ch), 1e-3);
    }
  }
}

/** Test for the expected behavior of the decay filter time constant */
TEST_P(level_smoother_t_filter_parametric_testing, test_level_smoother_t_decay_timeconstant)
{ 
  // Parse data with correct channel count as force_resize is not used
  p.parse("tau_attack = [0 0]");
  // 1 s decay time constant set
  p.parse("tau_decay = [1 1]"); 

  // Set up instance of level_smoother with default waveform configuration
  dc_simple::level_smoother_t level_smoother_wav{vars, filter_rate,
                              signal_properties_wav};

  // Set the wave to parameteric dB value
  wave.assign(MHASignal::dbspl2pa(GetParam()));

  // Process level_smoother for 1 second
  unsigned repeat = (signal_properties_wav.srate/signal_properties_wav.fragsize) - 1 ;
  mha_wave_t *level_wav = level_smoother_wav.process(&wave);

  for (unsigned i = 0; i<repeat; ++i){
    level_wav = level_smoother_wav.process(&wave);
  }
  
  // computed the smoothed signal level, which is the initial difference 
  // divided by e and added to the input signal level
  float smoothed_level = GetParam()+((65-GetParam())/expf(1));

  // Below 65, the value should be equal to smoothed level. 
  // Above 65, it is tested in the previous test.
  if (GetParam() < 65){
    for (unsigned ch = 0; ch < level_wav->num_channels; ++ch) {
      for (unsigned frame = 0; frame < level_wav->num_frames; ++frame) {
          EXPECT_NEAR(smoothed_level, value(level_wav, frame, ch), 5e-2);
      }
    }
  }
}

/** Test for the low pass attack filter is neither a maximum or minimum tracker */
TEST_P(level_smoother_t_filter_parametric_testing, test_level_smoother_t_attack_not_max_min_tracker)
{ 
  // Set attack value but decay to 0
  p.parse("tau_attack = [1 1]");
  p.parse("tau_decay = [0 0]"); 

  // Set up instance of level_smoother with default waveform configuration
  dc_simple::level_smoother_t level_smoother_wav{vars, filter_rate,
                              signal_properties_wav};
 
  // Set the wave to given dB value
  wave.assign(MHASignal::dbspl2pa(GetParam()));
  // Process level_smoother
  mha_wave_t *level_wav =  level_smoother_wav.process(&wave);

  // Expect the attack filter to be neither a maximum or minimum filter
  // on values other than 65 
  if (GetParam() != 65){
    for (unsigned ch = 0; ch < level_wav->num_channels; ++ch) {
      for (unsigned frame = 0; frame < level_wav->num_frames; ++frame) {
        EXPECT_NE(std::max(GetParam(), 65), value(level_wav, frame, ch));
        EXPECT_NE(std::min(GetParam(), 65), value(level_wav, frame, ch));
      }
    }
  }
}

/** Test the low pass attack filter with long time constant */
TEST_P(level_smoother_t_filter_parametric_testing, test_level_smoother_t_attack_low_pass)
{ 
  // Set attack value but decay to 0
  p.parse("tau_attack = [3600 3600]");
  p.parse("tau_decay = [0 0]"); 

  // Set up instance of level_smoother with default waveform configuration
  dc_simple::level_smoother_t level_smoother_wav{vars, filter_rate,
                              signal_properties_wav};
 
  // Set the wave to given dB value
  wave.assign(MHASignal::dbspl2pa(GetParam()));
  // Process level_smoother
  mha_wave_t *level_wav =  level_smoother_wav.process(&wave);

  // Expect the predefined 65 dB value to be there after processing once, with long time constant
  for (unsigned ch = 0; ch < level_wav->num_channels; ++ch) {
    for (unsigned frame = 0; frame < level_wav->num_frames; ++frame) {
      EXPECT_EQ(65, value(level_wav, frame, ch));
    }
  }
}


/** Test for the expected behavior of the attack filter time constant */
TEST_P(level_smoother_t_filter_parametric_testing, test_level_smoother_t_attack_timeconstant)
{ 
  // Parse data with correct channel count as force_resize is not used
  p.parse("tau_attack = [1 1]");
  // 1 s decay time constant set
  p.parse("tau_decay = [0 0]"); 

  // Set up instance of level_smoother with default waveform configuration
  dc_simple::level_smoother_t level_smoother_wav{vars, filter_rate,
                              signal_properties_wav};

  // Set the wave to parameteric dB value
  wave.assign(MHASignal::dbspl2pa(GetParam()));

  // Process level_smoother for 1 second
  unsigned repeat = (signal_properties_wav.srate/signal_properties_wav.fragsize) - 1 ;
  mha_wave_t *level_wav = level_smoother_wav.process(&wave);

  for (unsigned i = 0; i<repeat; ++i){
    level_wav = level_smoother_wav.process(&wave);
  }
  
  // computed the smoothed signal level, which is the initial difference 
  // divided by e and added to the input signal level
  float smoothed_level = GetParam()+((65-GetParam())/expf(1));

  // Expect an exponential decay similar to the max tracker filter, but
  // not a max or min tracker
  for (unsigned ch = 0; ch < level_wav->num_channels; ++ch) {
    for (unsigned frame = 0; frame < level_wav->num_frames; ++frame) {
        EXPECT_NEAR(smoothed_level, value(level_wav, frame, ch), 5e-2);
    }
  }
}

/** Instantiate the parametric tests for the delay filter being fed signals with different levels */
INSTANTIATE_TEST_SUITE_P(30dB_40dB_65dB_85dB, 
  level_smoother_t_filter_parametric_testing,::testing::Values(30, 40, 65, 85));
