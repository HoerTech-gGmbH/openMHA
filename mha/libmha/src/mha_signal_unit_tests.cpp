// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2017 2018 HörTech gGmbH
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
#include "mha_signal.hh"
#include "mha_filter.hh"

TEST(ringbuffer_t, initial_filling)
{
  unsigned frames = 1024;
  unsigned channels = 2;
  unsigned prefilled_frames = 512;

  // constructor creates a ringbuffer with capacity frames, prefilled_frames
  // of which are pre-filled with 0.0f, all this for channels audio channels
  MHASignal::ringbuffer_t ringbuffer(frames, channels, prefilled_frames);

  // Check that it is prefilled with the correct ammount of zeros
  EXPECT_EQ(prefilled_frames, ringbuffer.contained_frames());

  // Check that all these zeros are really zeroes in all channels
  for (unsigned channel = 0; channel < channels; ++channel)
    for (unsigned frame = 0; frame < prefilled_frames; ++frame)
      EXPECT_EQ(0.0f, ringbuffer.value(frame, channel));

  // Accessing index 512 == prefilled_frames raises an error. We want to
  // check that the error is raised and that the error message is correct.
  try {
    ringbuffer.value(prefilled_frames, 0);
    FAIL() << "index 512 is out of bounds and should have raised an exception";
  } catch (MHA_Error & e) {
    EXPECT_STREQ("(mha_signal) ringbuffer_t::value: frame 512 out of bounds",\
                 e.get_msg());
  }
}

TEST(ringbuffer_t, impossible_prefill)
{
  unsigned frames = 500;
  unsigned channels = 2;
  unsigned prefilled_frames = 600;

  try {
    MHASignal::ringbuffer_t ringbuffer(frames, channels, prefilled_frames);
    FAIL() << "prefilling more frames than exist should have raised exception";
  } catch (MHA_Error & e) {
    EXPECT_STREQ("(mha_signal) ringbuffer_t: 600 prefilled_frames > 500"\
                 " frames capacity", e.get_msg());
  }
}

TEST(ringbuffer_t, write)
{
  unsigned capacity_frames = 1024;
  unsigned channels = 2;
  unsigned prefilled_frames = 512;
  MHASignal::ringbuffer_t ringbuffer(capacity_frames,
                                     channels,
                                     prefilled_frames);
  unsigned write_frames = 400;
  MHASignal::waveform_t input(write_frames, channels);
  input.assign(1.0f);
  ringbuffer.write(input);
  ringbuffer.discard(write_frames);
  input.assign(2.0f);
  ringbuffer.write(input);
  unsigned frame0, frame1, frame2;
  for (frame0 = 0; frame0 < (prefilled_frames-write_frames); ++frame0)
    for (unsigned channel = 0; channel < channels; ++channel)
      ASSERT_EQ(0.0f, ringbuffer.value(frame0, channel));
  for (frame1 = frame0; frame1 < (write_frames + frame0); ++frame1)
    for (unsigned channel = 0; channel < channels; ++channel)
      ASSERT_EQ(1.0f, ringbuffer.value(frame1, channel));
  for (frame2 = frame1; frame2 < (write_frames + frame1); ++frame2)
    for (unsigned channel = 0; channel < channels; ++channel)
      ASSERT_EQ(2.0f, ringbuffer.value(frame2, channel));

  try {
    ringbuffer.value(frame2, 0);
    FAIL() << "Access to out-of-bounds index should have raised exception";
  } catch (MHA_Error & e) {
    ASSERT_STREQ("(mha_signal) ringbuffer_t::value: frame 912 out of bounds",\
                 e.get_msg());
  }

  try {
    ringbuffer.discard(913);
    FAIL() << "discard more frames than present should have raised exception";
  } catch (MHA_Error & e) {
    ASSERT_STREQ("(mha_signal) ringbuffer_t::discard: parameter 913"\
                 " exceeds value 912 of contained frames",\
                 e.get_msg());
  }
}

TEST(ringbuffer_t, discard)
{
  unsigned capacity_frames = 1024;
  unsigned channels = 2;
  unsigned prefilled_frames = 512;
  MHASignal::ringbuffer_t ringbuffer(capacity_frames,
                                     channels,
                                     prefilled_frames);
  ASSERT_EQ(512U, ringbuffer.contained_frames());

  unsigned write_frames = 400;
  MHASignal::waveform_t input(write_frames, channels);
  input.assign(1.0f);
  ringbuffer.write(input);
  ASSERT_EQ(912U, ringbuffer.contained_frames());
  ringbuffer.discard(511);
  ASSERT_EQ(401U, ringbuffer.contained_frames());
  ASSERT_EQ(0.0f, ringbuffer.value(0, 0));
  ASSERT_EQ(1.0f, ringbuffer.value(1, 0));
  ringbuffer.discard(1);
  ASSERT_EQ(400U, ringbuffer.contained_frames());
  ASSERT_EQ(1.0f, ringbuffer.value(0, 0));
  ASSERT_EQ(1.0f, ringbuffer.value(1, 0));
  ringbuffer.discard(400);
  ASSERT_EQ(0U, ringbuffer.contained_frames());
  
  try {
    ringbuffer.discard(913);
    FAIL() << "discard nonzero from empty ringbuffer raises exception";
  } catch (MHA_Error & e) {
    ASSERT_STREQ("(mha_signal) ringbuffer_t::discard: parameter 913"\
                 " exceeds value 0 of contained frames",\
                 e.get_msg());
  }
}

TEST(ringbuffer_t, wrap_bug)
{
  unsigned capacity_frames = 1024;
  unsigned channels = 2;
  unsigned prefilled_frames = 82;
  MHASignal::ringbuffer_t ringbuffer(capacity_frames,
                                     channels,
                                     prefilled_frames);
  // ringbuffer(0) to ringbuffer(81) are now 0;
  MHASignal::waveform_t in163(163, channels);
  MHASignal::waveform_t in512a(512, channels);
  MHASignal::waveform_t in512b(512, channels);
  for (unsigned k = 0; k < 163U; ++k)
    in163.value(k,0) = k;
  for (unsigned k = 0; k < 512U; ++k) {
    in512a.value(k,0) = k + 163;
    in512b.value(k,0) = k + 163 + 512;
  }

#define RB(val,index) ASSERT_EQ(float(val),ringbuffer.value(index,0))

  ringbuffer.write(in163);
  // ringbuffer(82) is zero, ringbuffer(83) is 1, rising from there
  RB(0.0,82);
  RB(1.0,83);
  // i.e. ringbuffer(161) is 99
  RB(79, 161);

  ringbuffer.discard(161); // ringbuffer(0) is now 79
  RB(79, 0);
  ringbuffer.write(in512a);
  ringbuffer.discard(162); // ringbuffer(0) is now 241
  RB(241, 0);
  ringbuffer.discard(162); // ringbuffer(0) is now 403
  RB(403, 0);
  ringbuffer.discard(163); // ringbuffer(0) is now 566
  RB(566, 0);
  ringbuffer.write(in512b);

  // Check if the error below is caused by the discards, or if it is already
  // present here
  RB(890+52,52+162+162); // is ok below (expect = avtual = 942)
  RB(890+53,53+162+162); // fails below (expect = 943, actual = 675)

  ringbuffer.discard(162); // ringbuffer(0) is now 728
  RB(728, 0);
  ringbuffer.discard(162); // ringbuffer(0) is now 890
  RB(890, 0);
  // we have now seen a discontinuity around approx. ringbuffer(50);

  for (unsigned i = 40; i <= 60; ++i)
    RB(890+i, i); // fails at i=53, expects 943, actual 675
}

TEST(polyphase_resampling_t, compute_interpolation_decimation_factors) {
  ASSERT_EQ(58U,  MHAFilter::resampling_factors(44100.0f,17400.0f).first);
  ASSERT_EQ(147U, MHAFilter::resampling_factors(44100.0f,17400.0f).second);
  ASSERT_EQ(58U,  MHAFilter::resampling_factors(44.1f,17.4f, 10).first);
  ASSERT_THROW(MHAFilter::resampling_factors(44.1f,17.4f), MHA_Error);
  ASSERT_THROW(MHAFilter::resampling_factors(441,0),       MHA_Error);
}

TEST(polyphase_resampling_t,write) {
  unsigned n_up = 58U;
  unsigned n_down = 147U;
  mha_real_t nyquist_ratio = 0.8f;
  unsigned input_capacity = 1024;
  unsigned n_channels = 2;
  unsigned n_prefill = 400;
  unsigned irslen = std::max(n_up, n_down) * 32;
  unsigned fragsize_in = 512;
  MHAFilter::polyphase_resampling_t polyphase_resampling(n_up, n_down,
                                                         nyquist_ratio, irslen,
                                                         input_capacity,
                                                         n_channels, n_prefill);
  MHASignal::waveform_t input(fragsize_in, n_channels);
  
  polyphase_resampling.write(input);
  input.assign(1.0f);

  try {
    polyphase_resampling.write(input);
    FAIL() << "write more frames than available space raises exception";
  } catch (MHA_Error & e) {
    ASSERT_STREQ("(mha_signal) ringbuffer_t::write: number 512 of"\
                 " frames contained in input signal exceeds"\
                 " available space of 112 frames in this ringbuffer",\
                 e.get_msg());
  }
}

TEST(polyphase_resampling_t, level) {
  unsigned n_up = 58U;
  unsigned n_down = 147U;
  mha_real_t nyquist_ratio = 0.9f;
  unsigned input_capacity = 1024;
  unsigned n_channels = 2;
  unsigned n_prefill = 400;
  unsigned irslen = n_up * 32;
  unsigned fragsize_in = 512;
  unsigned fragsize_out = 200;
  MHAFilter::polyphase_resampling_t polyphase_resampling(n_up, n_down,
                                                         nyquist_ratio, irslen,
                                                         input_capacity,
                                                         n_channels, n_prefill);
  MHASignal::waveform_t input(fragsize_in, n_channels);
  MHASignal::waveform_t output(fragsize_out, n_channels);
  
  input.assign(1.0f);
  polyphase_resampling.write(input);
  polyphase_resampling.read(output);
  ASSERT_NEAR(1.0f, output.value(fragsize_out - 1, 0), 1e-5);
}

TEST(polyphase_resampling_t,filter) {
  unsigned n_up = 58U;
  unsigned n_down = 147U;
  mha_real_t nyquist_ratio = 0.8f;
  unsigned input_capacity = 1024;
  unsigned n_channels = 2;
  unsigned irslen = std::max(n_up, n_down) * 32;
  unsigned fragsize_in = 512;
  unsigned fragsize_out = 64;

 // just long enough to apply the filter once.
  unsigned n_prefill = (irslen - 1U) / n_up + 1U;

  MHAFilter::polyphase_resampling_t polyphase_resampling(n_up, n_down,
                                                         nyquist_ratio, irslen,
                                                         input_capacity,
                                                         n_channels, n_prefill);
  unsigned n_required = (fragsize_out * n_down) / n_up + 1U;

  MHASignal::waveform_t input_zero(n_required, n_channels);
  MHASignal::waveform_t input(fragsize_in, n_channels);
  MHASignal::waveform_t output(fragsize_out, n_channels);

  output.assign(1.0f);
  polyphase_resampling.write(input_zero);
  polyphase_resampling.read(output);
  for (unsigned frame = 0U; frame < fragsize_out; ++frame)
    for (unsigned channel = 0U; channel < n_channels; ++channel)
      ASSERT_EQ(0.0f, output.value(frame,channel));

  double f = 12.0/512.0*44100.0;
  for (unsigned frame = 0U; frame < fragsize_in; ++frame)
    for (unsigned channel = 0U; channel < n_channels; ++channel)
      input.value(frame,channel) =
        sin(frame/44100.0*2.0*M_PI*f) * (channel*2.0-1.0);

  polyphase_resampling.write(input);
  polyphase_resampling.read(output);
  polyphase_resampling.read(output);
  polyphase_resampling.read(output);
  polyphase_resampling.write(input);
  polyphase_resampling.read(output);
  polyphase_resampling.read(output);
  polyphase_resampling.read(output);
}

TEST(polyphase_resampling_t, filter_underflow) {
  unsigned n_up = 58U;
  unsigned n_down = 147U;
  mha_real_t nyquist_ratio = 0.8f;
  unsigned input_capacity = 1024;
  unsigned n_channels = 2;
  unsigned irslen = std::max(n_up, n_down) * 32;
  unsigned fragsize_in = 512;
  unsigned fragsize_out = 64;

 // too short to filter anything without further data:
  unsigned n_prefill = irslen / n_up - 1;

  MHAFilter::polyphase_resampling_t polyphase_resampling(n_up, n_down,
                                                         nyquist_ratio, irslen,
                                                         input_capacity,
                                                         n_channels, n_prefill);
  MHASignal::waveform_t input(fragsize_in, n_channels);
  MHASignal::waveform_t output(fragsize_out, n_channels);
  
  try {
    polyphase_resampling.read(output);
    FAIL() << "Underflow raises exception";
  } catch (MHA_Error & e) {
    ASSERT_STREQ("(mha_filter) MHAFilter::polyphase_resampling_t::read: Not"\
                 " enough input data: underflow detected when producing output"\
                 " for frame=0 convolution_index=4697", e.get_msg());
  }
}

/* parameters for polyphase resampling tests */
struct polyphase_parameters_t {
  unsigned n_up;
  unsigned n_down;
  mha_real_t nyquist_ratio;
  unsigned input_capacity;
  unsigned n_channels;
  unsigned irslen;
  unsigned n_prefill;
};

/* a parameterizable fixture for testing readable_frames / underflow
 * conditions */
class test_underflow_t :
  public ::testing::TestWithParam<polyphase_parameters_t> {
};

TEST_P(test_underflow_t, can_read_available_samples_but_not_more) {
  polyphase_parameters_t p = GetParam();
  MHAFilter::polyphase_resampling_t resampler(p.n_up, p.n_down,
                                              p.nyquist_ratio, p.irslen,
                                              p.input_capacity,
                                              p.n_channels, p.n_prefill);
  unsigned available_frames = resampler.readable_frames();
  EXPECT_GT(available_frames, 0U);

  MHASignal::waveform_t all_readable(available_frames, p.n_channels);
  MHASignal::waveform_t one_sample_too_many(1U, p.n_channels);

  resampler.read(all_readable);
  EXPECT_EQ(0U, resampler.readable_frames());
  EXPECT_THROW(resampler.read(one_sample_too_many), MHA_Error);
}

struct polyphase_parameters_t params[] = {
  //n_up, n_down, nyq_ratio, in_cap, ch, irslen,   n_prefill
  {  58U,   147U,      0.8f,  1024U, 2U, 147U*32U, (147U*32U-1U)/58U+1U},
  {   1U,     3U,     0.85f,   219U, 1U,      34U,                  94U},
};

INSTANTIATE_TEST_SUITE_P(InstantiationName, test_underflow_t, ::testing::ValuesIn(params));
                        

TEST(polyphase_resampling_t,readable_frames) {
#define MAKE_NEW_POLYPHASE \
  unsigned n_up = 58U; \
  unsigned n_down = 147U; \
  mha_real_t nyquist_ratio = 0.8f; \
  unsigned input_capacity = 1024; \
  unsigned n_channels = 2; \
  unsigned irslen = std::max(n_up, n_down) * 32; \
  unsigned n_prefill = (irslen-1U) / n_up + 1U; \
  MHAFilter::polyphase_resampling_t polyphase_resampling(n_up, n_down, \
                                                         nyquist_ratio, irslen,\
                                                         input_capacity, \
                                                         n_channels, n_prefill);
  MHASignal::waveform_t s1(1,2);
  MHASignal::waveform_t s203(203,2);
  MHASignal::waveform_t s512(512,2);
  {
    MAKE_NEW_POLYPHASE;
    ASSERT_EQ(1U, polyphase_resampling.readable_frames());
    polyphase_resampling.read(s1);
    ASSERT_EQ(0U, polyphase_resampling.readable_frames());
    ASSERT_THROW(polyphase_resampling.read(s1), MHA_Error);
  }
  {
    MAKE_NEW_POLYPHASE;
    polyphase_resampling.write(s512);
    ASSERT_EQ(203U, polyphase_resampling.readable_frames());
    polyphase_resampling.read(s203);
    ASSERT_EQ(0U, polyphase_resampling.readable_frames());
    ASSERT_THROW(polyphase_resampling.read(s1), MHA_Error);
  }
}

TEST(blockprocessing_polyphase_resampling_t,test_44100_17400_512_64) {
  unsigned nchannels = 2U;
  float outer_srate = 44100.0f;
  unsigned outer_fragsize = 512;
  float inner_srate = 17400.0f;
  unsigned inner_fragsize = 64; 
  float nyquist_ratio = 0.9;
  float irslen = 0.72e-3f;
  MHAFilter::blockprocessing_polyphase_resampling_t
    resampling(outer_srate, outer_fragsize,
               inner_srate, inner_fragsize,
               nyquist_ratio, irslen, nchannels, true);
  
  MHASignal::waveform_t s77_2(77U, 2U);
  MHASignal::waveform_t s512_1(512U, 1U);
  MHASignal::waveform_t s64_1(64U, 1U);

  try {
    resampling.write(s77_2);
    FAIL() << "wrong number of frames should have raised exception";
  } catch (MHA_Error & e) {
    ASSERT_STREQ("(mha_filter) MHAFilter::"\
                 "blockprocessing_polyphase_resampling_t::write:"\
                 " Number 77 of signal frames does not match"\
                 " configured input block size 512",\
                 e.get_msg());
  }

  try {
    resampling.write(s512_1);
    FAIL() << "wrong number of channels should have raised exception";
  } catch (MHA_Error & e) {
    ASSERT_STREQ("(mha_signal) ringbuffer_t::write:"\
                 " number 1 of channels in signal does not match"\
                 " the number 2 of channels of this ringbuffer",\
                 e.get_msg());
  }

  try {
    resampling.read(s77_2);
    FAIL() << "wrong number of frames should have raised exception";
  } catch (MHA_Error & e) {
    ASSERT_STREQ("(mha_filter) MHAFilter::blockprocessing_polyphase_"\
                 "resampling_t::read:"\
                 " Number 77 of frames in signal structure does not"\
                 " match configured output block size 64",\
                 e.get_msg());
  }

  try {
    resampling.read(s64_1);
    FAIL() << "wrong number of channels should have raised exception";
  } catch (MHA_Error & e) {
    ASSERT_STREQ("(mha_filter) MHAFilter::blockprocessing_polyphase_"\
                 "resampling_t::read:"\
                 " Number 1 of channels in signal structure does not"\
                 " match configured number 2 of channels",           \
                 e.get_msg());
  }
  
  MHASignal::waveform_t s512(512U, 2U);
  MHASignal::waveform_t s64(64U, 2U);

  double f = 12.0/512.0*44100.0;
  for (unsigned frame = 0U; frame < 512U; ++frame)
    for (unsigned channel = 0U; channel < nchannels; ++channel)
      s512.value(frame,channel) =
        sin(frame/44100.0*2.0*M_PI*f) * (channel*2.0-1.0);

  unsigned j[44100/512+1];
  for (unsigned i = 0U; i <= 44100U/512U; ++i) {
    resampling.write(s512);
    for(j[i] = 0U; resampling.can_read(); j[i]++)
      resampling.read(s64);
    EXPECT_TRUE(j[i] == 3U || j[i] == 4U);
  }
}

/*
  void pfile(FILE * f, mha_wave_t & s) {
    for (unsigned frame = 0; frame < s.num_frames; ++frame) {
      for (unsigned ch = 0; ch < s.num_channels; ++ch)
        fprintf(f, "%.7f\t", value(s,frame,ch));
      fprintf(f,"\n");
    }
    fprintf(f,"\n");
  }
*/

TEST(blockprocessing_polyphase_resampling_t, test_44100_17400_512_64_roundtrip)
{
  unsigned nchannels = 2U;
  float outer_srate = 44100.0f;
  unsigned outer_fragsize = 512;
  float inner_srate = 17400.0f;
  unsigned inner_fragsize = 64; 
  float nyquist_ratio = 0.9;
  float irslen = 0.72e-3f;
  MHAFilter::blockprocessing_polyphase_resampling_t
    to(outer_srate, outer_fragsize,
       inner_srate, inner_fragsize,
       nyquist_ratio, irslen, nchannels, true);
  MHAFilter::blockprocessing_polyphase_resampling_t
    fro(inner_srate, inner_fragsize,
        outer_srate, outer_fragsize,
        nyquist_ratio, irslen, nchannels, false);

  MHASignal::waveform_t s512(512U, 2U);
  MHASignal::waveform_t o512(512U, 2U);
  MHASignal::waveform_t p512(512U, 2U);
  MHASignal::waveform_t s64(64U, 2U);
  
  double f = 12.0/512.0*44100.0;
  for (unsigned frame = 0U; frame < 512U; ++frame)
    for (unsigned channel = 0U; channel < nchannels; ++channel)
      s512.value(frame,channel) =
        sin(frame/44100.0*2.0*M_PI*f) * (channel*2.0-1.0);

  //FILE * fo =  fopen("output","w");
  for (unsigned i = 0U; i <= 44100U/512U; ++i) {
    to.write(s512);
    for(unsigned j = 0U; to.can_read(); j++) {
      to.read(s64);
      fro.write(s64);
    }
    ::assign(p512,o512);
    fro.read(o512);
    //pfile(fo,o512);
  }
  //fclose(fo);
  mha_real_t min = 1.0f, max = -1.0f, zero = -1.0f;
  for (unsigned frame = 0U; frame < 512U; ++frame) {
    min = std::min(min, o512.value(frame, 0));
    max = std::max(max, o512.value(frame, 0));
    if (fabsf(zero) > fabsf(o512.value(frame, 0)))
      zero = o512.value(frame, 0);
    for (unsigned channel = 0U; channel < 2U; ++channel) {
      ASSERT_NEAR(o512.value(frame, channel), p512.value(frame, channel), 1e-4);
    }
  }
  ASSERT_NEAR(-1.0f, min, 1e-3);
  ASSERT_NEAR(1.0f, max, 1e-3);
  ASSERT_NEAR(0.0f, zero, 0.017);
}

TEST(blockprocessing_polyphase_resampling_t, test_48k_16k_64_20_roundtrip)
{
  unsigned nchannels = 1U;
  float outer_srate = 48000.0f;
  unsigned outer_fragsize = 64U;
  float inner_srate = 16000.0f;
  unsigned inner_fragsize = 20U;
  float nyquist_ratio = 0.850000024f;
  float irslen = 0.000699999975f;
  MHAFilter::blockprocessing_polyphase_resampling_t
    to(outer_srate, outer_fragsize,
       inner_srate, inner_fragsize,
       nyquist_ratio, irslen, nchannels, true);
  MHAFilter::blockprocessing_polyphase_resampling_t
    fro(inner_srate, inner_fragsize,
        outer_srate, outer_fragsize,
        nyquist_ratio, irslen, nchannels, false);

  MHASignal::waveform_t s64(64U, 1U);
  MHASignal::waveform_t o64(64U, 1U);
  MHASignal::waveform_t p64(64U, 1U);
  MHASignal::waveform_t s20(20U, 1U);

  // A frequency with multiple waveforms in the outer buffer
  double f = 4.0/outer_fragsize*outer_srate;
  for (unsigned frame = 0U; frame < outer_fragsize; ++frame)
    for (unsigned channel = 0U; channel < nchannels; ++channel)
      s64.value(frame,channel) =
        sin(frame/outer_srate*2.0*M_PI*f) * (channel*2.0-1.0);

  //FILE * fo =  fopen("output","w");
  for (unsigned i = 0U; i <= unsigned(outer_srate)/outer_fragsize; ++i) {
    to.write(s64);
    for(unsigned j = 0U; to.can_read(); j++) {
      to.read(s20);
      fro.write(s20);
    }
    ::assign(p64,o64);
    fro.read(o64);
    //pfile(fo,o64);
  }
  //fclose(fo);
  mha_real_t min = 1.0f, max = -1.0f, zero = -1.0f;
  for (unsigned frame = 0U; frame < outer_fragsize; ++frame) {
    min = std::min(min, o64.value(frame, 0));
    max = std::max(max, o64.value(frame, 0));
    if (fabsf(zero) > fabsf(o64.value(frame, 0)))
      zero = o64.value(frame, 0);
    for (unsigned channel = 0U; channel < nchannels; ++channel) {
      ASSERT_NEAR(o64.value(frame, channel), p64.value(frame, channel), 1e-3);
    }
  }
  ASSERT_NEAR(-1.0f, min, 1e-3);
  ASSERT_NEAR(1.0f, max, 1e-3);
  ASSERT_NEAR(0.0f, zero, 0.017);
}

// Local Variables:
// compile-command: "make -C .. unit-tests"
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
