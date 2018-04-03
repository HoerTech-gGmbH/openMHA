// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2017 HörTech gGmbH
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

#include "mha_tablelookup.hh"
#include <gtest/gtest.h>
#include <math.h>
// Test linear interpolation between 3 mesh points
class MHATableLookup_linear_interpolation : public ::testing::Test {
public:
  MHATableLookup::xy_table_t t;
  void SetUp() {
    t.add_entry(  0,   0);
    t.add_entry(100, 100);
    t.add_entry(300,-100);
  }
};

TEST_F(MHATableLookup_linear_interpolation, values_at_mesh_points) {
  EXPECT_EQ(   0, t.interp(  0));
  EXPECT_EQ( 100, t.interp(100));
  EXPECT_EQ(-100, t.interp(300));
}
TEST_F(MHATableLookup_linear_interpolation, interpolate_between_mesh_points) {
  EXPECT_EQ( 1.0f,  t.interp(  1));
  EXPECT_EQ(99.0f,  t.interp( 99));
  EXPECT_EQ(99.0f,  t.interp(101));
  EXPECT_EQ( 0.0f,  t.interp(200));
}
TEST_F(MHATableLookup_linear_interpolation, table_extrapolates) {
  EXPECT_EQ( -10.0f,  t.interp(-10));
  EXPECT_EQ(-200.0f,  t.interp(400));
}

// Test linear interpolation between 3 mesh points
class MHATableLookup_log_interpolation : public ::testing::Test {
public:
  MHATableLookup::xy_table_t t;
  void SetUp() {
    t.set_xfun(logf); // set log domain
    float frequencies[] = {125,500,1000};
    float gains[] = {20,0,30};
    t.add_entry(frequencies, gains, 3);
  }
};

TEST_F(MHATableLookup_log_interpolation, values_at_mesh_points) {
  EXPECT_FLOAT_EQ(20, t.interp( 125));
  EXPECT_FLOAT_EQ( 0, t.interp( 500));
  EXPECT_FLOAT_EQ(30, t.interp(1000));
}
TEST_F(MHATableLookup_log_interpolation, interpolate_between_mesh_points) {
  EXPECT_FLOAT_EQ(10.0f,  t.interp(250));
  EXPECT_NEAR(15.0f, t.interp(sqrtf(500*1000)),     15e-6f);
  EXPECT_NEAR(1,     t.interp(500 * pow(2,1.0/30)), 15e-6f);
}
TEST_F(MHATableLookup_log_interpolation, table_extrapolates) {
  EXPECT_FLOAT_EQ(  30.0f,  t.interp(125.0f/2));
  EXPECT_FLOAT_EQ(  60.0f,  t.interp(2000));
}

// Local Variables:
// compile-command: "make -C .. unit-tests"
// coding: utf-8-unix
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
