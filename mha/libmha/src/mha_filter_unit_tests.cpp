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
#include <limits>
#include "mha.hh"
#include "mha_signal.hh"
#include "mha_filter.hh"


template<typename T>
void test_friendly_number(){
  T x = std::numeric_limits<T>::max()*2;
  MHAFilter::make_friendly_number(x);
  EXPECT_EQ(x,0);

  x = -std::numeric_limits<T>::max()*2;
  MHAFilter::make_friendly_number(x);
  EXPECT_EQ(x,0);


  T a = std::numeric_limits<T>::min();
  T b=a;
  MHAFilter::make_friendly_number(a);
  EXPECT_EQ(a,b);
  a=std::numeric_limits<T>::denorm_min();
  MHAFilter::make_friendly_number(a);
  EXPECT_EQ(0,a);

  a = -std::numeric_limits<T>::min();
  b=a;
  MHAFilter::make_friendly_number(a);
  EXPECT_EQ(a,b);
  a=-std::numeric_limits<T>::denorm_min();
  MHAFilter::make_friendly_number(a);
  EXPECT_EQ(0,a);

  x = std::numeric_limits<T>::min() / 2;
  MHAFilter::make_friendly_number(x);
  EXPECT_EQ(0,x);

  x = -std::numeric_limits<T>::min() / 2;
  MHAFilter::make_friendly_number(x);
  EXPECT_EQ(0,x);
}

TEST(make_friendly_number, real_types)
{
  test_friendly_number<mha_real_t>();
  test_friendly_number<float>();
  test_friendly_number<double>();
  test_friendly_number<long double>();
}

TEST(make_friendly_number, complex_types){
  mha_complex_t x;
  x.re = x.im = std::numeric_limits<mha_real_t>::max()*2;
  MHAFilter::make_friendly_number(x);
  EXPECT_EQ(x.re,0);
  EXPECT_EQ(x.im,0);

  x.re = x.im = -std::numeric_limits<mha_real_t>::max()*2;
  MHAFilter::make_friendly_number(x);
  EXPECT_EQ(x.re,0);
  EXPECT_EQ(x.im,0);


  mha_complex_t a;
  a.re = a.im = std::numeric_limits<mha_real_t>::min();
  auto b=a;
  MHAFilter::make_friendly_number(a);
  EXPECT_EQ(a,b);

  x.re = x.im = std::numeric_limits<mha_real_t>::denorm_min();
  MHAFilter::make_friendly_number(x);
  EXPECT_EQ(0,x.re);
  EXPECT_EQ(0,x.im);

  a.re = a.im = -std::numeric_limits<mha_real_t>::min();
  b=a;
  MHAFilter::make_friendly_number(a);
  EXPECT_EQ(a,b);

  x.re = x.im = -std::numeric_limits<mha_real_t>::denorm_min();
  MHAFilter::make_friendly_number(x);
  EXPECT_EQ(0,x.re);
  EXPECT_EQ(0,x.im);

  x.re = x.im = std::numeric_limits<mha_real_t>::min() / 2;
  MHAFilter::make_friendly_number(x);
  EXPECT_EQ(0,x.re);
  EXPECT_EQ(0,x.im);

  x.re = x.im = -std::numeric_limits<mha_real_t>::min() / 2;
  MHAFilter::make_friendly_number(x);
  EXPECT_EQ(0,x.re);
  EXPECT_EQ(0,x.im);
  
}

TEST(fir_lp,ctor){
  std::vector<float> expected={0.000708189, 0.00206211, 0.0034785, 0.00263068, -0.00392844, -0.0163139, -0.0270606, -0.0220697, 0.0115061, 0.0746584, 0.151125, 0.21276, 0.233843, 0.205538, 0.140899, 0.0670368, 0.00991681, -0.0181695, -0.0211416, -0.0120001, -0.00270404, 0.00171053, 0.00229722, 0};
  auto actual=MHAFilter::fir_lp(4000, 6000 , 48000 , 24);
  EXPECT_EQ(expected.size(),actual.size());
  for(unsigned i=0U; i<actual.size();i++){
    EXPECT_NEAR(expected[i],actual[i],1e-2);
  }
}

TEST(polyphase_resampling_t, buffer_space) {
  unsigned frames=std::numeric_limits<unsigned>::max();
  EXPECT_THROW(MHAFilter::blockprocessing_polyphase_resampling_t b(48000,frames,32000,1U,0.85f,7e-4,1,false),MHA_Error);
}
TEST(o1flt_maxtrack_t, pass_max) {
  // rising values are passed, regardless of time constants
  MHAFilter::o1flt_maxtrack_t maxtrack({1.0f}, 48000, 0);
  ASSERT_FLOAT_EQ(0.0f, maxtrack(0,0.0f));
  ASSERT_FLOAT_EQ(1.0f, maxtrack(0,1.0f));
  ASSERT_FLOAT_EQ(1.0f, maxtrack(0,1.0f));
  ASSERT_FLOAT_EQ(2.0f, maxtrack(0,2.0f));
}

TEST(o1flt_maxtrack_t, decay_one_sample) {
  // Check time constant with single sample
  MHAFilter::o1flt_maxtrack_t maxtrack({1.0f}, 1, 1);
  ASSERT_FLOAT_EQ(1.0f, maxtrack(0,1.0f));
  ASSERT_FLOAT_EQ(1/expf(1), maxtrack(0,0.0f));
}

TEST(o1flt_maxtrack_t, decay_some_samples) {
  // Check time constant with three samples
  MHAFilter::o1flt_maxtrack_t maxtrack({1.0f}, 3, 1);
  ASSERT_FLOAT_EQ(1.0f, maxtrack(0,1.0f));
  ASSERT_FLOAT_EQ(1/expf(1), (maxtrack(0,0),maxtrack(0,0),maxtrack(0,0)));
}

TEST(o1flt_maxtrack_t, decay_many_samples) {
  // Check time constant with 48000 samples
  MHAFilter::o1flt_maxtrack_t maxtrack({1.0f}, 48000, 1);
  ASSERT_FLOAT_EQ(1.0f, maxtrack(0,1.0f));
  for (int i = 1; i < 48000; ++i)
    maxtrack(0,0);
  ASSERT_NEAR(1/expf(1), maxtrack(0,0), 0.001);
}
