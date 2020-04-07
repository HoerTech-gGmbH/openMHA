// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2007 2009 2010 2012 2013 2014 2015 2017 2018
//             2020 HörTech gGmbH
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
#include "example7.hh"
#include "mha_algo_comm.hh"
#include "mha_signal.hh"


class example7_testing : public ::testing::Test {
public:
  // AC variable space
  MHAKernel::algo_comm_class_t acspace{};
  // C handle to AC variable space
  algo_comm_t ac {acspace.get_c_handle()};
  // example input to prepare method
  mhaconfig_t signal_properties {
      .channels = 1U,
      .domain = MHA_WAVEFORM,
      .fragsize = 10U,
      .wndlen = 0U,
      .fftlen = 0U,
      .srate = 44100.0f
      };
  //Plugin instance
  example7_t ex7{ac,"thread","algo"};
  MHASignal::waveform_t wave_input{signal_properties.fragsize,signal_properties.channels};
  void setWaveValues(mha_real_t amp){
    wave_input.assign(amp);
  }
};

TEST_F(example7_testing,test_state_methods){
  ex7.prepare_(signal_properties);
  setWaveValues(1.0f);
  EXPECT_NEAR(1.0f,wave_input.buf[4],1e-6);
  EXPECT_NEAR(1.0f,wave_input.buf[5],1e-6);
  ex7.process(&wave_input);
  EXPECT_NEAR(0.1f,wave_input.buf[4],1e-6);
  EXPECT_NEAR(0.1f,wave_input.buf[5],1e-6);
  ex7.release();
}

// Local Variables:
// compile-command: "make unit-tests"
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
