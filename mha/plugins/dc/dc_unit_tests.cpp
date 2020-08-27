// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2020 HörTech gGmbH
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

#include "dc.hh"
#include "mha_signal.hh"
#include <iostream>
#include <gtest/gtest.h>


class dc_if_t_testing : public ::testing::Test {
protected:
  mhaconfig_t signal_properties;
  MHAKernel::algo_comm_class_t acspace;
  algo_comm_t ac;
  dc::dc_if_t dc_if_handle;
  dc_if_t_testing():
    signal_properties {
    .channels = 1U,
      .domain = MHA_WAVEFORM,
      .fragsize = 100U,
      .wndlen = 0U,
      .fftlen = 0U,
      .srate = 100.0f
      },
    acspace{},
    ac{acspace.get_c_handle()},
    dc_if_handle{ac, "thread", "algo"}
  {
    dc_if_handle.parse("gtdata=[[20 40 40]]");
    dc_if_handle.parse("gtmin=[50]");
    dc_if_handle.parse("gtstep=[20]");
    dc_if_handle.parse("tau_rmslev=[0]");
    dc_if_handle.parse("tau_attack=[0]");
    dc_if_handle.parse("tau_decay=[0]");
  }

  MHASignal::waveform_t genWaveInput(mha_real_t level_dbspl) {
    MHASignal::waveform_t wave_obj{signal_properties};
    wave_obj.assign(MHASignal::dbspl2pa(level_dbspl));
    return wave_obj;
  }
};


TEST_F(dc_if_t_testing,lin_interp){
  dc_if_handle.parse("log_interp=no");
  dc_if_handle.prepare(signal_properties);

  // 0dBSPL - extrapolated linearly (to -InfdB gain)
  MHASignal::waveform_t wave0db{genWaveInput(0.0f)};
  dc_if_handle.process(&wave0db);
  //take a sample after the compressor adjustment instead of rms-level computation
  //because the input signal is a direct current
  EXPECT_FLOAT_EQ(0.0f,value(wave0db,99,0));

  // 40dBSPL - extrapolated linearly (to -InfdB gain)
  MHASignal::waveform_t wave40db{genWaveInput(40.0f)};
  dc_if_handle.process(&wave40db);
  EXPECT_FLOAT_EQ(0.0f,value(wave40db,99,0));

  // 47dBSPL - extrapolated linearly (to -InfdB gain)
  MHASignal::waveform_t wave47db{genWaveInput(47.0f)};
  dc_if_handle.process(&wave47db);
  EXPECT_FLOAT_EQ(0.0f,value(wave47db,99,0));

  // 48dBSPL - extrapolated linearly (to 0dB gain)
  MHASignal::waveform_t wave48db{genWaveInput(48.0f)};
  mha_real_t sample48db{value(wave48db,99,0)};
  dc_if_handle.process(&wave48db);
  EXPECT_NEAR(sample48db,value(wave48db,99,0),sample48db*1e-4f);

  // 50dBSPL - direct gt-entry
  MHASignal::waveform_t wave50db{genWaveInput(50.0f)};
  mha_real_t sample50db{value(wave50db,99,0)};
  dc_if_handle.process(&wave50db);
  EXPECT_FLOAT_EQ(sample50db*10.0f,value(wave50db,99,0));

  // 60dBSPL - interpolated linearly
  MHASignal::waveform_t wave60db{genWaveInput(60.0f)};
  mha_real_t sample60db{value(wave60db,99,0)};
  dc_if_handle.process(&wave60db);
  EXPECT_FLOAT_EQ(sample60db*55.0f,value(wave60db,99,0));

  // 70dBSPL - direct gt-entry
  MHASignal::waveform_t wave70db{genWaveInput(70.0f)};
  mha_real_t sample70db{value(wave70db,99,0)};
  dc_if_handle.process(&wave70db);
  EXPECT_FLOAT_EQ(sample70db*100.0f,value(wave70db,99,0));

  // 80dBSPL - interpolated linearly (to stay at 40dB gain)
  MHASignal::waveform_t wave80db{genWaveInput(80.0f)};
  mha_real_t sample80db{value(wave80db,99,0)};
  dc_if_handle.process(&wave80db);
  EXPECT_FLOAT_EQ(sample80db*100.0f,value(wave80db,99,0));

  // 100dBSPL - extrapolated linearly (to stay at 40dB gain)
  MHASignal::waveform_t wave100db{genWaveInput(100.0f)};
  mha_real_t sample100db{value(wave100db,99,0)};
  dc_if_handle.process(&wave100db);
  EXPECT_FLOAT_EQ(sample100db*100.0f,value(wave100db,99,0));
}

TEST_F(dc_if_t_testing,log_interp){
  dc_if_handle.parse("log_interp=yes");
  dc_if_handle.prepare(signal_properties);

  // -Inf dBSPL (extrapolated logarithmically to -InfdB gain)
  MHASignal::waveform_t wave_infdb{genWaveInput(-std::numeric_limits<float>::infinity())};
  dc_if_handle.process(&wave_infdb);
  EXPECT_FLOAT_EQ(0.0f,value(wave_infdb,99,0));

  // 10dBSPL (extrapolated logarithmically to -20dB gain)
  MHASignal::waveform_t wave10db{genWaveInput(10.0f)};
  mha_real_t sample10db{value(wave10db,99,0)};
  dc_if_handle.process(&wave10db);
  EXPECT_FLOAT_EQ(sample10db/MHASignal::db2lin(20.0f),value(wave10db,99,0));

  // 30dBSPL (extrapolated logarithmically to 0dB gain)
  MHASignal::waveform_t wave30db{genWaveInput(30.0f)};
  mha_real_t sample30db{value(wave30db,99,0)};
  dc_if_handle.process(&wave30db);
  EXPECT_FLOAT_EQ(sample30db,value(wave30db,99,0));

  // 50dBSPL (direct gt-entry)
  MHASignal::waveform_t wave50db{genWaveInput(50.0f)};
  mha_real_t sample50db{value(wave50db,99,0)};
  dc_if_handle.process(&wave50db);
  EXPECT_FLOAT_EQ(sample50db*MHASignal::db2lin(20.0f),value(wave50db,99,0));

  // 60dBSPL (interpolated logarithmically to 30dB gain)
  MHASignal::waveform_t wave60db{genWaveInput(60.0f)};
  mha_real_t sample60db{value(wave60db,99,0)};
  dc_if_handle.process(&wave60db);
  EXPECT_FLOAT_EQ(sample60db*MHASignal::db2lin(30.0f),value(wave60db,99,0));

  // 70dBSPL (direct gt-entry)
  MHASignal::waveform_t wave70db{genWaveInput(70.0f)};
  mha_real_t sample70db{value(wave70db,99,0)};
  dc_if_handle.process(&wave70db);
  EXPECT_FLOAT_EQ(sample70db*MHASignal::db2lin(40.0f),value(wave70db,99,0));

  // 80dBSPL (interpolated logarithmically to stay at 40dB gain)
  MHASignal::waveform_t wave80db{genWaveInput(80.0f)};
  mha_real_t sample80db{value(wave80db,99,0)};
  dc_if_handle.process(&wave80db);
  EXPECT_FLOAT_EQ(sample80db*MHASignal::db2lin(40.0f),value(wave80db,99,0));

  // 100dBSPL (extrapolated logarithmically to stay at 40dB gain)
  MHASignal::waveform_t wave100db{genWaveInput(100.0f)};
  mha_real_t sample100db{value(wave100db,99,0)};
  dc_if_handle.process(&wave100db);
  EXPECT_FLOAT_EQ(sample100db*MHASignal::db2lin(40.0f),value(wave100db,99,0));
}



// Local Variables:
// compile-command: "make unit-tests"
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
