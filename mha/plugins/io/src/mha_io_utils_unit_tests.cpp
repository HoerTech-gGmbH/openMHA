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
#include <gtest/gtest.h>
#include "mha_io_utils.hh"
#include <cmath>
#include <limits>

const float notanumber = std::numeric_limits<float>::quiet_NaN();
const float one = 1;
const float almost_one=std::nextafterf(1,0);
const float almost_almost_one=std::nextafterf(almost_one,0);
const float one_plus_epsilon=std::nextafterf(1,2);
const float minusone= -1;
const float almost_minusone=std::nextafterf(-1,0);
const float almost_almost_minusone=std::nextafterf(almost_minusone,0);
const float minusone_minus_epsilon=std::nextafterf(-1,-2);

using namespace mhaioutils;

TEST(to_int_clamped,test_data){
  ASSERT_GT(one_plus_epsilon,one);
  ASSERT_GT(minusone,minusone_minus_epsilon);

  ASSERT_GT(one,almost_one);
  ASSERT_GT(almost_one,almost_almost_one);

  ASSERT_LT(minusone,almost_minusone);
  ASSERT_LT(almost_minusone,almost_almost_minusone);
}

TEST(to_int_clamped_int16_t,zero_nan){
  EXPECT_EQ(0,to_int_clamped<int16_t>(notanumber));
}

TEST(to_int_clamped_int16_t,clamps_low){
  EXPECT_EQ(std::numeric_limits<int16_t>::min(),to_int_clamped<int16_t>(minusone));
  EXPECT_EQ(std::numeric_limits<int16_t>::min(),to_int_clamped<int16_t>(minusone_minus_epsilon));
}

TEST(to_int_clamped_int16_t,clamps_high){
  EXPECT_EQ(std::numeric_limits<int16_t>::max(),to_int_clamped<int16_t>(one));
  EXPECT_EQ(std::numeric_limits<int16_t>::max(),to_int_clamped<int16_t>(one_plus_epsilon));
}

TEST(to_int_clamped_int32_t,zero_nan){
  EXPECT_EQ(0,to_int_clamped<int32_t>(notanumber));
}

TEST(to_int_clamped_int32_t,clamps_low){
  EXPECT_EQ(std::numeric_limits<int32_t>::min(),to_int_clamped<int32_t>(minusone));
  EXPECT_EQ(std::numeric_limits<int32_t>::min(),to_int_clamped<int32_t>(minusone_minus_epsilon));
}

TEST(to_int_clamped_int32_t,clamps_high){
  EXPECT_EQ(std::numeric_limits<int32_t>::max(),to_int_clamped<int32_t>(one));
  EXPECT_EQ(std::numeric_limits<int32_t>::max(),to_int_clamped<int32_t>(one_plus_epsilon));
}

TEST(to_int_clamped_int16_t,monotony){
  EXPECT_LE(to_int_clamped<int16_t>(almost_one),to_int_clamped<int16_t>(one));
  EXPECT_LE(to_int_clamped<int16_t>(almost_almost_one),to_int_clamped<int16_t>(almost_one));
  EXPECT_GE(to_int_clamped<int16_t>(almost_almost_minusone),to_int_clamped<int16_t>(almost_minusone));
  EXPECT_GE(to_int_clamped<int16_t>(almost_minusone),to_int_clamped<int16_t>(minusone));
}

TEST(to_int_clamped_int32_t,monotony){
  EXPECT_LE(to_int_clamped<int32_t>(almost_one),to_int_clamped<int32_t>(one));
  EXPECT_LE(to_int_clamped<int32_t>(almost_almost_one),to_int_clamped<int32_t>(almost_one));
  EXPECT_GE(to_int_clamped<int32_t>(almost_almost_minusone),to_int_clamped<int32_t>(almost_minusone));
  EXPECT_GE(to_int_clamped<int32_t>(almost_minusone),to_int_clamped<int32_t>(minusone));
}
