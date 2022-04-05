// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2018 HörTech gGmbH
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


#include "fshift.hh"
#include <gtest/gtest.h>
#include <limits>

TEST(fft_find_bin,dc_and_nyguist){
  EXPECT_EQ(0,fshift::fft_find_bin(0,512,16000));
  EXPECT_EQ(256,fshift::fft_find_bin(8000,512,16000));
}

TEST(fft_find_bin,linearity){
  EXPECT_EQ(110,fshift::fft_find_bin(3437.5,512,16000));
  EXPECT_EQ(220,fshift::fft_find_bin(6875,512,16000));
}

TEST(fft_find_bin,symmetry){
  auto freq=100;
  auto res=fshift::fft_find_bin(freq,512,16000);
  EXPECT_EQ(-res,fshift::fft_find_bin(-freq,512,16000));
}

TEST(fft_find_bin,rounding){
  mha_real_t bin_width=16000.0f/512.0f;
  auto eps = bin_width - nextafterf(bin_width,0);

  EXPECT_EQ(1,fshift::fft_find_bin(bin_width/2,512,16000));
  EXPECT_EQ(0,fshift::fft_find_bin(bin_width/2.0f-eps,512,16000));

  EXPECT_EQ(-1,fshift::fft_find_bin(-bin_width/2.0f,512,16000));
  EXPECT_EQ(0,fshift::fft_find_bin(-bin_width/2.0f+eps,512,16000));
}

TEST(fft_find_bin,out_of_range){
  EXPECT_NO_THROW(fshift::fft_find_bin(7999,512,16000));
  EXPECT_NO_THROW(fshift::fft_find_bin(-7999,512,16000));
  EXPECT_THROW(fshift::fft_find_bin(8001,512,16000),MHA_Error);
  EXPECT_THROW(fshift::fft_find_bin(-8001,512,16000),MHA_Error);
}
