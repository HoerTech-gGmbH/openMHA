// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2007 2009 2010 2012 2013 2014 2015 2017 2018
//             2020 HörTech gGmbH
// Copyright © 2022 Hörzentrum Oldenburg gGmbH
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

//! [first docu snippet]
#include "mha_algo_comm.hh"
#include "mha_signal.hh"
#include "example7.hh"
#include <gtest/gtest.h>

class example7_testing : public ::testing::Test {
public:
  /// AC variable space.
  MHA_AC::algo_comm_class_t acspace;
  /// Client interface to AC variable space.
  MHA_AC::algo_comm_t & ac = {acspace};
  /// Example input to prepare method.
  mhaconfig_t signal_properties {
      .channels = 2U,
      .domain = MHA_WAVEFORM,
      .fragsize = 10U,
      .wndlen = 0U,
      .fftlen = 0U,
      .srate = 44100.0f
  };
  /// Plugin instance.
  example7_t ex7 = {ac,"algo"};
  MHASignal::waveform_t wave_input{signal_properties.fragsize,signal_properties.channels};
};
//! [first docu snippet]

//! [second docu snippet]
TEST_F(example7_testing,test_state_methods){
  EXPECT_FALSE(ex7.is_prepared());
  ex7.prepare_(signal_properties);
  acspace.set_prepared(true);
  EXPECT_TRUE(ex7.is_prepared());
  acspace.set_prepared(false);
  ex7.release_();
  EXPECT_FALSE(ex7.is_prepared());
}
//! [second docu snippet]

//! [third docu snippet]
TEST_F(example7_testing,test_functionality){
  ex7.prepare_(signal_properties);
  acspace.set_prepared(true);
  wave_input.assign(1.0f);
  EXPECT_FLOAT_EQ(1.0f,value(wave_input,4,0));
  EXPECT_FLOAT_EQ(1.0f,value(wave_input,5,0));
  EXPECT_FLOAT_EQ(1.0f,value(wave_input,4,1));
  EXPECT_FLOAT_EQ(1.0f,value(wave_input,5,1));
  ex7.process(&wave_input);
  EXPECT_FLOAT_EQ(0.1f,value(wave_input,4,0));
  EXPECT_FLOAT_EQ(0.1f,value(wave_input,5,0));
  EXPECT_FLOAT_EQ(1.0f,value(wave_input,4,1));
  EXPECT_FLOAT_EQ(1.0f,value(wave_input,5,1));
  acspace.set_prepared(false);
  ex7.release_();
}
//! [third docu snippet]

// Local Variables:
// compile-command: "make unit-tests"
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
