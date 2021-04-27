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
#include <iostream>

class dc_simple_testing : public ::testing::Test {
  public:
  // AC variable space
  MHAKernel::algo_comm_class_t acspace{};

  // C handle to AC variable space
  algo_comm_t ac {acspace.get_c_handle()};

  // Example input to prepare method
  mhaconfig_t signal_properties =  {
    .channels = 2,  .domain = MHA_WAVEFORM, .fragsize = 10,
    .wndlen   = 0, .fftlen = 0, .srate    = 16000
  };
  // Plugin instance
  dc_simple::dc_if_t dc_simple{ac,"algo"};

  /// prepares the plugin for tests
  void prepare() {
    dc_simple.prepare_(signal_properties);
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
  dc_simple.release_();
  signal_properties.domain = MHA_WAVEFORM;
  EXPECT_NO_THROW(prepare());
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
  sprintf(expected, "[%.9g]", 0.005f);
  EXPECT_EQ(expected, dc_simple.parse("tau_attack?val"));

  // limits are [0,]
  EXPECT_EQ("[0,]", dc_simple.parse("tau_attack?range"));
  EXPECT_THROW(dc_simple.parse("tau_attack = -0.1"), MHA_Error);
}

TEST_F(dc_simple_testing, test_parameter_tau_decay)
{
  // default value is 0.05f
  char expected [32];
  sprintf(expected, "[%.9g]", 0.05f);
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
  dc_simple.release_();

  // Expect no throw if channel count and parsed variable dimension are equal
  prepare();
  EXPECT_NO_THROW(dc_simple.parse("g50 = [50 50 50 50]"));
  dc_simple.release_(); 

  // Expect no throw if channel count and parsed variable dimension
  // are different, but parsed variable has only 1 dimension
  prepare();
  EXPECT_NO_THROW(dc_simple.parse("g50 = [50]"));

}
