// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2020 2021 HörTech gGmbH
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
#include "delay.hh"
#include "mha_algo_comm.hh"

class delay_testing : public ::testing::Test {
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
  delay::interface_t delay{ac,"thread","algo"};
  // Three MHASignal type waveform declared
  MHASignal::waveform_t wave_original{signal_properties.fragsize, signal_properties.channels};
  MHASignal::waveform_t wave_processed{signal_properties.fragsize, signal_properties.channels};
  MHASignal::waveform_t wave_processed_twice{signal_properties.fragsize, signal_properties.channels};

  // Prepares the plugin for tests
  void prepare() {
    delay.prepare_(signal_properties);
    // Assigned reproducible signal
    for (unsigned ch = 0; ch < wave_original.num_channels; ++ch) {
      for (unsigned k = 0; k < wave_original.num_frames; ++k) {
          wave_original.assign(k, ch, k + ch * 0.5);
      }
    }
  }
};

TEST_F(delay_testing, has_parameter_delay)
{
  // Limits are [0,]
  EXPECT_EQ("[0,[", delay.parse("delay?range"));
  // MHA should throw error when delay is specified negative
  EXPECT_THROW(delay.parse("delay = -1"), MHA_Error);
  // No error if multiple delay are specified 
  EXPECT_NO_THROW(delay.parse("delay = [50 100]"));
  // Expect delay values to be set
  delay.parse("delay = [50 100]");
  EXPECT_EQ("[50 100]", delay.parse("delay?val"));
}

TEST_F(delay_testing, cannot_be_prepared_for_spectral_processing)
{
  signal_properties.domain = MHA_SPECTRUM;
  EXPECT_THROW(delay.prepare_(signal_properties), MHA_Error);
}

// This fixture tests if the signal is correctly delayed. For that purpose, three variables
// are prepared in prepare(): one of which retains the original signal, and the other two 
// would be processed and compared with the original
TEST_F(delay_testing, test_delay)
{
  prepare();
  unsigned ndelay = 2;
  unsigned ch1 = 0;
  unsigned ch2 = 1;
  unsigned buffer_size = signal_properties.fragsize;
  unsigned nchannels = signal_properties.channels;
  delay.parse("delay = [2 0]");
  
  // Process copy of original wave
  wave_processed.copy(wave_original);
  delay.process(&wave_processed);

  // Expect channel 2 to have not been delayed
  for (unsigned frame = 0; frame < buffer_size; frame++) {
    EXPECT_EQ(wave_original.buf[nchannels * frame + ch2], 
                                wave_processed.buf[nchannels * frame + ch2]);
  }
  
  // Expect first ndelay values of channel 1 to be 0
  for (unsigned frame = 0; frame < ndelay; frame++) {

    EXPECT_EQ(0, wave_processed.buf[nchannels * frame + ch1]);
  }

  // Expect channel 1 to be delayed by ndelay samples
  for (unsigned frame = 0; frame < buffer_size - ndelay; frame++) {
    EXPECT_EQ(wave_original.buf[nchannels * frame + ch1], 
                          wave_processed.buf[nchannels * (frame + ndelay) + ch1]);
  }

  // Delayed signal copied and processed again
  wave_processed_twice.copy(wave_processed);
  delay.process(&wave_processed_twice);
  
  // Expect channel 1 to be delayed further by ndelay samples
  for (unsigned frame = 0; frame < buffer_size - ndelay; frame++) {
  EXPECT_EQ(wave_processed.buf[nchannels * frame + ch1], 
                      wave_processed_twice.buf[nchannels * (frame + ndelay) + ch1]);
  }

  // Expect that the first ndelay samples in the new processed signal are equal 
  // to the last ndelay samples of the previous block (which are wave_original 
  // last ndelay samples)
  for (unsigned frame = 0; frame < ndelay; frame++) {
  EXPECT_EQ(wave_original.buf[nchannels * (buffer_size - ndelay + frame) + ch1], 
                      wave_processed_twice.buf[nchannels * frame + ch1]);
  }

  // Change delay value for channel 2 and change ndelay variable
  delay.parse("delay = [2 4]");
  ndelay = 4;

  // Process copy of original wave
  wave_processed.copy(wave_original);
  delay.process(&wave_processed);

  // Expect channel 2 to be delayed by ndelay samples
  for (unsigned frame = 0; frame < buffer_size - ndelay; frame++) {
    EXPECT_EQ(wave_original.buf[nchannels * frame + ch2], 
                          wave_processed.buf[nchannels * (frame + ndelay) + ch2]);
  }

  // Release 
  delay.release_();
}
