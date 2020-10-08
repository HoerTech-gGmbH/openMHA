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


#include "lsl2ac.hh"
#include <gtest/gtest.h>
#include "mha_algo_comm.hh"

#include <chrono>
#include <thread>
#include <atomic>
using namespace std::chrono_literals;

constexpr int NCHUNKS=12;
constexpr int NCHANNELS=12;

class Test_save_var_t : public ::testing::Test {
protected:
  Test_save_var_t():
    info("SimpleStream","Audio",NCHANNELS),
    acspace(),
    ac(acspace.get_c_handle())
  {
    expected.resize(NCHUNKS);
    int i=0;
    for(auto& vec : expected){
      vec.resize(NCHANNELS);
      std::generate(vec.begin(), vec.end(), [&i](){ return i++;});
    }
    outlet_thread=std::thread([this](){
                                {
                                  lsl::stream_outlet outlet(info);
                                  outlet_ready=true;
                                  outlet.wait_for_consumers(5/*s*/);
                                  for(const auto& vec : expected)
                                    outlet.push_sample(vec);
                                  // Short sleep needed for the lsl buffers to sort themselves out
                                  std::this_thread::sleep_for(0.01s);
                                  outlet_done=true;
                                  while(!stop_request){
                                    std::this_thread::sleep_for(0.01s);
                                  }
                                }
                              });
    while(!outlet_ready)
      std::this_thread::sleep_for(0.01s);
  }
  ~Test_save_var_t(){
    stop_request=true;
    outlet_thread.join();
  }
  void SetUp() override {
    while(!outlet_done)
      std::this_thread::sleep_for(0.01s);
  }
  // Creates an outlet stream. Sends the samples as fast as possible once a
  // consumer connects. Synchronization through the atomic bools.
  void open_stream();
  void TearDown() override {}
  std::atomic<bool> outlet_ready{false};
  std::atomic<bool> outlet_done{false};
  std::atomic<bool> stop_request{false};
  std::thread outlet_thread;
  lsl::stream_info info;
  MHAKernel::algo_comm_class_t acspace;
  algo_comm_t ac;
  std::vector<std::vector<float>> expected;
};


class Test_save_var_t_abort : public Test_save_var_t {
protected:
  lsl2ac::save_var_t var;
  Test_save_var_t_abort():
    Test_save_var_t(),
    var(info,ac,lsl2ac::underrun_behavior::Abort,
        lsl2ac::overrun_behavior::Discard,
        1,0){}
};

class Test_save_var_t_copy : public Test_save_var_t {
protected:
  lsl2ac::save_var_t var;
  Test_save_var_t_copy():
    Test_save_var_t(),
    var(info,ac,lsl2ac::underrun_behavior::Copy,
        lsl2ac::overrun_behavior::Discard,
        1,0){}
};

class Test_save_var_t_zero : public Test_save_var_t {
protected:
  lsl2ac::save_var_t var;
  Test_save_var_t_zero():
    Test_save_var_t(),
    var(info,ac,lsl2ac::underrun_behavior::Zero,
        lsl2ac::overrun_behavior::Discard,
        1,0){}
};


TEST_F(Test_save_var_t_abort,receive_frame){
  var.receive_frame();
  auto buf=MHA_AC::get_var_waveform(ac,"SimpleStream").buf;
  std::vector<float> actual(buf,buf+NCHANNELS);
  EXPECT_EQ(expected.back(),actual);
  EXPECT_THROW(var.receive_frame(),MHA_Error);
  }


TEST_F(Test_save_var_t_zero,receive_frame){
  var.receive_frame();
  auto buf=MHA_AC::get_var_waveform(ac,"SimpleStream").buf;
  std::vector<float> actual(buf,buf+NCHANNELS);
  EXPECT_EQ(expected.back(),actual);

  var.receive_frame();
  expected.back().assign(NCHANNELS,0.0);
  buf=MHA_AC::get_var_waveform(ac,"SimpleStream").buf;
  actual.assign(buf,buf+NCHANNELS);
  EXPECT_EQ(expected.back(),actual);
}


TEST_F(Test_save_var_t_copy,receive_frame){
  var.receive_frame();
  auto buf=MHA_AC::get_var_waveform(ac,"SimpleStream").buf;
  std::vector<float> actual(buf,buf+NCHANNELS);
  EXPECT_EQ(expected.back(),actual);

  var.receive_frame();
  buf=MHA_AC::get_var_waveform(ac,"SimpleStream").buf;
  actual.assign(buf,buf+NCHANNELS);
  EXPECT_EQ(expected.back(),actual);
}
