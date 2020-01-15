// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2019 2020 HörTech gGmbH
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
#include "mha.hh"
#include "mha_utils.hh"
#include <gtest/gtest.h>
#include <cmath>
using MHAUtils::is_multiple_of;

TEST(is_multiple_of, int_zero_and_int_max){
  EXPECT_FALSE(is_multiple_of(2U,0U));
  EXPECT_FALSE(is_multiple_of(std::numeric_limits<unsigned>::max(),std::numeric_limits<unsigned>::max()-1));
}

TEST(is_multiple_of, true){
  EXPECT_TRUE(is_multiple_of(86723976, 7907));
  EXPECT_TRUE(is_multiple_of(457342968, 14698));
  EXPECT_TRUE(is_multiple_of(1778368757, 38891));
  EXPECT_TRUE(is_multiple_of(1725508688, 27568));
  EXPECT_TRUE(is_multiple_of(1213596748, 35581));
}

TEST(is_multiple_of, false){
  EXPECT_FALSE(is_multiple_of(1474187490, 35906));
  EXPECT_FALSE(is_multiple_of(266032375, 20126));
  EXPECT_FALSE(is_multiple_of(98750875, 46472));
  EXPECT_FALSE(is_multiple_of(1373869842, 45643));
  EXPECT_FALSE(is_multiple_of(2287643146, 49343));
}

using MHAUtils::is_power_of_two;
TEST(is_power_of_two, int_zero_and_int_max){
  EXPECT_FALSE(is_power_of_two(0U));
  EXPECT_FALSE(is_power_of_two(std::numeric_limits<unsigned>::max()));
}

TEST(is_power_of_two, true){
  EXPECT_TRUE(is_power_of_two(16777216U));
  EXPECT_TRUE(is_power_of_two(16U));
  EXPECT_TRUE(is_power_of_two(32U));
  EXPECT_TRUE(is_power_of_two(65536U));
  EXPECT_TRUE(is_power_of_two(524288U));
}

TEST(is_power_of_two, false){
  EXPECT_FALSE(is_power_of_two(2147483647U));
  EXPECT_FALSE(is_power_of_two(24U));
  EXPECT_FALSE(is_power_of_two(8388607U));
  EXPECT_FALSE(is_power_of_two(67108863U));
  EXPECT_FALSE(is_power_of_two(15U));

}

using MHAUtils::strip;
TEST(strip, strip){
  EXPECT_EQ(strip(""),"");
  EXPECT_EQ(strip("\r"),"");
  EXPECT_EQ(strip("\n"),"");
  EXPECT_EQ(strip("\n\r"),"");
  EXPECT_EQ(strip("\r\n"),"");
  EXPECT_EQ(strip("\r\nfoo"),"\r\nfoo");
  EXPECT_EQ(strip("nolineendinghere"),"nolineendinghere");
  EXPECT_EQ(strip("foo baz bar \n"),"foo baz bar");
}

using MHAUtils::spl2hl;
TEST(spl2hl,spl2hl){
  EXPECT_NEAR(spl2hl(125),-22.1,1e-4);
  EXPECT_NEAR(spl2hl(1250),-3.5,1e-4);
  EXPECT_NEAR(spl2hl(16000),-40.2,1e-4);
  EXPECT_NEAR(spl2hl(18000),-62.0,1e-4);
  EXPECT_THROW(spl2hl(-1), MHA_Error);
}

using MHAUtils::is_denormal;
TEST(is_denormal,real){
  EXPECT_FALSE(is_denormal(1));
  EXPECT_FALSE(is_denormal(0));

  EXPECT_FALSE(is_denormal(std::numeric_limits<mha_real_t>::min()));
  EXPECT_FALSE(-is_denormal(std::numeric_limits<mha_real_t>::min()));

  EXPECT_FALSE(-is_denormal(std::numeric_limits<mha_real_t>::infinity()));
  EXPECT_FALSE(-is_denormal(std::numeric_limits<mha_real_t>::infinity()));

  EXPECT_FALSE(is_denormal(std::numeric_limits<mha_real_t>::quiet_NaN()));
  EXPECT_FALSE(is_denormal(-std::numeric_limits<mha_real_t>::quiet_NaN()));

  EXPECT_TRUE(is_denormal(std::numeric_limits<mha_real_t>::denorm_min()));
  EXPECT_TRUE(-is_denormal(std::numeric_limits<mha_real_t>::denorm_min()));
}

TEST(is_denormal,complex){
  mha_real_t not_a_denormal_number=1;
  mha_real_t a_denormal_number=std::numeric_limits<mha_real_t>::denorm_min();
  mha_complex_t x;
  x.im=not_a_denormal_number;
  x.re=not_a_denormal_number;
  EXPECT_FALSE(is_denormal(x));

  x.im=a_denormal_number;
  EXPECT_TRUE(is_denormal(x));

  x.im=not_a_denormal_number;
  x.re=a_denormal_number;
  EXPECT_TRUE(is_denormal(x));

  x.im=a_denormal_number;
  EXPECT_TRUE(is_denormal(x));

}

// Local Variables:
// compile-command: "make -C .. unit-tests"
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
