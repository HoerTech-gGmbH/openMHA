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
#include "windnoise.hh"
#include "mha_algo_comm.hh"

/// A googletest test fixture to help instantiate and prepare the plugin
class windnoise_testing : public ::testing::Test {
public:
  /// algorithm communication variable space needed for instantiating plugin
  MHAKernel::algo_comm_class_t ac;

  /// plugin instance under test
  windnoise::if_t windnoise;

  /// signal dimensions which can be used to prepare the plugin in tests
  mhaconfig_t signal_info =  {
    .channels = 2,  .domain = MHA_SPECTRUM, .fragsize = 32,
    .wndlen   = 64, .fftlen = 128,          .srate    = 16000
  };

  /// constructor creates plugin instance but does not prepare it
  windnoise_testing()
    : windnoise(ac.get_c_handle(), "thread_name", "algo_name")
  {
  }

  /// prepares the plugin for tests
  void prepare() {
    windnoise.prepare_(signal_info);
  }
};



// ------------------------------------------------------------------
// Section of tests that check configuration parameter existance,
// default values and value range.  Compared against values extracted
// from matlab reference code.
// ------------------------------------------------------------------

TEST_F(windnoise_testing, has_parameter_UseChannel_LF_attenuation)
{
  // default value is false
  EXPECT_EQ("no", windnoise.parse("UseChannel_LF_attenuation?val"));

  // can be set to true
  windnoise.parse("UseChannel_LF_attenuation = yes");
  EXPECT_EQ("yes", windnoise.parse("UseChannel_LF_attenuation?val"));
}

TEST_F(windnoise_testing, has_parameter_tau_Lowpass)
{
  // default value is 1 second
  EXPECT_EQ("1", windnoise.parse("tau_Lowpass?val"));
  windnoise.parse("tau_Lowpass = 0");
  EXPECT_EQ("0", windnoise.parse("tau_Lowpass?val"));

  // limits are [0,1]
  EXPECT_EQ("[0,1]", windnoise.parse("tau_Lowpass?range"));
  EXPECT_THROW(windnoise.parse("tau_Lowpass = 1.01"), MHA_Error);
  EXPECT_THROW(windnoise.parse("tau_Lowpass = -0.01"), MHA_Error);
}

TEST_F(windnoise_testing, has_parameter_LowPassCutOffFrequency)
{
  // default value is 1 second
  EXPECT_EQ("500", windnoise.parse("LowPassCutOffFrequency?val"));
  windnoise.parse("LowPassCutOffFrequency = 0");
  EXPECT_EQ("0", windnoise.parse("LowPassCutOffFrequency?val"));
  windnoise.parse("LowPassCutOffFrequency = 25000");
  EXPECT_EQ("25000", windnoise.parse("LowPassCutOffFrequency?val"));

  // limits are [0,]
  EXPECT_EQ("[0,]", windnoise.parse("LowPassCutOffFrequency?range"));
  EXPECT_THROW(windnoise.parse("LowPassCutOffFrequency = -0.01"), MHA_Error);
}

TEST_F(windnoise_testing, has_parameter_LowPassFraction)
{
  // default value is -1 dB difference
  EXPECT_EQ("-1", windnoise.parse("LowPassFraction?val"));
  windnoise.parse("LowPassFraction = -10");
  EXPECT_EQ("-10", windnoise.parse("LowPassFraction?val"));
  windnoise.parse("LowPassFraction = 10");
  EXPECT_EQ("10", windnoise.parse("LowPassFraction?val"));

  // limits are [-10,10] dB difference
  EXPECT_EQ("[-10,10]", windnoise.parse("LowPassFraction?range"));
  EXPECT_THROW(windnoise.parse("LowPassFraction = 10.01"), MHA_Error);
  EXPECT_THROW(windnoise.parse("LowPassFraction = -10.01"), MHA_Error);
}

TEST_F(windnoise_testing, has_parameter_LowPassWindGain)
{
  // default value is -10 dB (= 10dB attenuation)
  EXPECT_EQ("-10", windnoise.parse("LowPassWindGain?val"));
  windnoise.parse("LowPassWindGain = 0");
  EXPECT_EQ("0", windnoise.parse("LowPassWindGain?val"));
  windnoise.parse("LowPassWindGain = -100");
  EXPECT_EQ("-100", windnoise.parse("LowPassWindGain?val"));

  // limits are [-100,0]
  EXPECT_EQ("[-100,0]", windnoise.parse("LowPassWindGain?range"));
  EXPECT_THROW(windnoise.parse("LowPassWindGain = 0.01"), MHA_Error);
  EXPECT_THROW(windnoise.parse("LowPassWindGain = -100.01"), MHA_Error);
}

TEST_F(windnoise_testing, has_parameter_WindNoiseDetector)
{
  // default value is psd, but can be set to any other value in range
  // [psd diffsum msc none]
  EXPECT_EQ("diffsum", windnoise.parse("WindNoiseDetector?val"));
  windnoise.parse("WindNoiseDetector = psd");
  EXPECT_EQ("psd", windnoise.parse("WindNoiseDetector?val"));
  windnoise.parse("WindNoiseDetector = diffsum");
  EXPECT_EQ("diffsum", windnoise.parse("WindNoiseDetector?val"));
  windnoise.parse("WindNoiseDetector = msc");
  EXPECT_EQ("msc", windnoise.parse("WindNoiseDetector?val"));
  windnoise.parse("WindNoiseDetector = none");
  EXPECT_EQ("none", windnoise.parse("WindNoiseDetector?val"));
  EXPECT_EQ("[psd diffsum msc none]",
            windnoise.parse("WindNoiseDetector?range"));
}


// ------------------------------------------------------------------
// Section of tests that check plugin behaviour on preparation and
// configuration changes.
// ------------------------------------------------------------------

TEST_F(windnoise_testing, cannot_be_prepared_for_waveform_processing)
{
  signal_info.domain = MHA_WAVEFORM;
  EXPECT_THROW(prepare(), MHA_Error);
}

TEST_F(windnoise_testing, can_be_prepared_for_spectral_processing)
{
  EXPECT_NO_THROW(prepare());
}

TEST_F(windnoise_testing, all_configuration_variables_are_registered) {
  // Names of configuration variables from reference windnoise_detector_Init.m
#define CONFIGURATION_NAMES \
  "UseChannel_LF_attenuation", "tau_Lowpass", "LowPassCutOffFrequency", \
    "LowPassFraction", "LowPassWindGain", "WindNoiseDetector"
  const char * const configuration_names[] = {CONFIGURATION_NAMES};

  // Names of extra monitor variables introduced for openMHA
#define MONITOR_NAMES "detected", "lowpass_quotient"

  // All names of writable and read-only known parser variables:
  const char * const parser_names[] = {CONFIGURATION_NAMES, MONITOR_NAMES};

  // Check if the plugin contains exactly the parser variables the tests know of
  std::string parser_entries = windnoise.parse("?entries");
  for (const std::string name : parser_names) {
    auto string_index = parser_entries.find(name);
    ASSERT_LT(string_index, parser_entries.size()) << "Missing: " << name;
    parser_entries.erase(string_index, name.length()+1);
  }
  // After erasing all configuration variables known to the tests, only the
  // opening bracket and default entries should remain:
  EXPECT_EQ("[mhaconfig_in mhaconfig_out ", parser_entries);

  // Check if all writable parser variables are registered to update the
  // runtime config on write access
  prepare();
  for (const std::string name : configuration_names) {
    const auto old_cfg = windnoise.poll_config();
    const std::string query_val = "?val";

    // Reassign current value for simplicity.  Works with writeaccess.
    windnoise.parse(name + "=" + windnoise.parse(name + query_val));
    EXPECT_NE(old_cfg, windnoise.poll_config()) << "should have been updated";
  }
}

// Test for the runtime configuration

TEST_F(windnoise_testing, runtime_parameters_from_default_values) {
  prepare(); // creates a runtime config from default parameters and signal_info
  auto cfg = windnoise.poll_config();
  
  // This bool is copied from configuration variable to the runtime config
  EXPECT_EQ(windnoise.UseChannel_LF_attenuation.data,
            cfg->UseChannel_LF_attenuation);
               

  // Matlab code used milliseconds, fragsize (OLA hopsize) name is RingbufferLen
  EXPECT_EQ(expf(-float(signal_info.fragsize) /
                 (windnoise.tau_Lowpass.data * signal_info.srate)),
            cfg->alpha_Lowpass);
  // same as previous line but precomputed in Octave with default values
  EXPECT_FLOAT_EQ(0.9980019987f, cfg->alpha_Lowpass);

  // The low pass cut-off frequency is translated to frequency bin index
  EXPECT_EQ(roundf(MHASignal::freq2bin(windnoise.LowPassCutOffFrequency.data,
                                       signal_info.fftlen,
                                       signal_info.srate)),
            cfg->FrequencyBinLowPass);
  // same as previous line but precomputed in Octave with default values
  EXPECT_EQ(4U, cfg->FrequencyBinLowPass);

  // The low pass fraction threshold is converted from dB to linear factor
  EXPECT_EQ(powf(10, windnoise.LowPassFraction.data/20), cfg->LowPassFraction);
  // same as previous line but precomputed in Octave with default values
  EXPECT_FLOAT_EQ(0.89125093813f, cfg->LowPassFraction);

  // The low pass wind gain is converted from dB to linear factor
  EXPECT_EQ(powf(10, windnoise.LowPassWindGain.data/20), cfg->LowPassWindGain);
  // same as previous line but precomputed in Octave with default values
  EXPECT_FLOAT_EQ(0.316227766f, cfg->LowPassWindGain);
}

// Local Variables:
// compile-command: "make unit-tests"
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
