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

#include "mha_tablelookup.hh"
#include <gtest/gtest.h>
#include <math.h>
// Test linear interpolation between 3 mesh points
class MHATableLookup_xy_linear_interpolation : public ::testing::Test {
public:
  MHATableLookup::xy_table_t t;
  void SetUp() {
    t.add_entry(  0,   0);
    t.add_entry(100, 100);
    t.add_entry(300,-100);
  }
};

TEST_F(MHATableLookup_xy_linear_interpolation, values_at_mesh_points) {
  EXPECT_EQ(   0, t.interp(  0));
  EXPECT_EQ( 100, t.interp(100));
  EXPECT_EQ(-100, t.interp(300));
}
TEST_F(MHATableLookup_xy_linear_interpolation, interpolate_between_mesh_points)
{
  EXPECT_EQ( 1.0f,  t.interp(  1));
  EXPECT_EQ(99.0f,  t.interp( 99));
  EXPECT_EQ(99.0f,  t.interp(101));
  EXPECT_EQ( 0.0f,  t.interp(200));
}
TEST_F(MHATableLookup_xy_linear_interpolation, table_extrapolates) {
  EXPECT_EQ( -10.0f,  t.interp(-10));
  EXPECT_EQ(-200.0f,  t.interp(400));
}

// Test interpolation on logarithmic x axis between 3 mesh points
class MHATableLookup_xy_log_interpolation : public ::testing::Test {
public:
  MHATableLookup::xy_table_t t;
  void SetUp() {
    t.set_xfun(logf); // set log domain
    float frequencies[] = {125,500,1000};
    float gains[] = {20,0,30};
    t.add_entry(frequencies, gains, 3);
  }
};

TEST_F(MHATableLookup_xy_log_interpolation, values_at_mesh_points) {
  EXPECT_FLOAT_EQ(20, t.interp( 125));
  EXPECT_FLOAT_EQ( 0, t.interp( 500));
  EXPECT_FLOAT_EQ(30, t.interp(1000));
}
TEST_F(MHATableLookup_xy_log_interpolation, interpolate_between_mesh_points) {
  EXPECT_FLOAT_EQ(10.0f,  t.interp(250));
  EXPECT_NEAR(15.0f, t.interp(sqrtf(500*1000)),     15e-6f);
  EXPECT_NEAR(1,     t.interp(500 * pow(2,1.0/30)), 15e-6f);
}
TEST_F(MHATableLookup_xy_log_interpolation, table_extrapolates) {
  EXPECT_FLOAT_EQ(  30.0f,  t.interp(125.0f/2));
  EXPECT_FLOAT_EQ(  60.0f,  t.interp(2000));
}


// The following tests are for class linear_table_t, which is limited to
// equidistant x mesh points

// Test linear_table interpolation between 3 mesh points
class MHATableLookup_linear_table : public ::testing::Test {
public:
  MHATableLookup::linear_table_t t;
  struct point {float x, y;}
    points[3] = {{100,   0},
                 {107, 100},
                 {114,  50},
                 // next point would be at 121
    };
  void SetUp() {
    t.set_xmin(points[0].x);  // xmin is the x of the first entry
    // now add the y of all points in correct order from low x to high x
    t.add_entry(points[0].y); //   0.0f {x = 100, y = 0}
    t.add_entry(points[1].y); // 100.0f {x = 107, y = 100}
    t.add_entry(points[2].y); //  50.0f {x = 114, y = 50}

    // set xmax to the x of the entry that would follow next, but does not
    t.set_xmax(points[2].x + 7);

    // let the object compute the distance between the x values
    t.prepare();
  }
};
TEST_F(MHATableLookup_linear_table, values_at_mesh_points) {
  for (point p : points) {
    EXPECT_FLOAT_EQ(p.y, t.lookup(p.x));
    EXPECT_FLOAT_EQ(p.y, t.interp(p.x));
  }
}

TEST_F(MHATableLookup_linear_table, interpolate_between_mesh_points) {
  EXPECT_FLOAT_EQ(50.0f, t.interp(103.5f));
  EXPECT_FLOAT_EQ( 0.0f, t.lookup(103.5f));
  
  EXPECT_FLOAT_EQ(75.0f, t.interp(110.5f));
  EXPECT_FLOAT_EQ(100.f, t.lookup(110.5f));

  // lookup will return the y for the x meshpoint with lesser x than looked up 
  // even if the meshpoint with higher x is nearer
  EXPECT_FLOAT_EQ( 0.0f, t.lookup(106.0f));
  EXPECT_FLOAT_EQ(100.f, t.lookup(113.0f));
}

TEST_F(MHATableLookup_linear_table, table_extrapolates) {

  // the interp method extrapolates
  EXPECT_FLOAT_EQ(-100.0f,  t.interp(93));
  EXPECT_NEAR(       0.0f,  t.interp(121), 2e-5f);
  EXPECT_FLOAT_EQ( -50.0f,  t.interp(128));

  // but the lookup method does not extrapolate
  EXPECT_FLOAT_EQ(   0.0f,  t.lookup(93));
  EXPECT_FLOAT_EQ(  50.0f,  t.lookup(121));
  EXPECT_FLOAT_EQ(  50.0f,  t.lookup(128));
}

// =================================================

// Older tests translated from cppunit to googletest
using MHATableLookup::xy_table_t;

TEST(xy_table_t, lookup_InRange)
{
    xy_table_t xy;
    xy.add_entry(1.0f,2.0f);
    xy.add_entry(2.0f,4.0f);
    xy.add_entry(3.0f,6.0f);
    EXPECT_EQ(2.0f,xy.lookup(1.0f));
    EXPECT_EQ(4.0f,xy.lookup(2.0f));
    EXPECT_EQ(4.0f,xy.lookup(2.4f));
    EXPECT_EQ(4.0f,xy.lookup(1.6f));
    EXPECT_EQ(6.0f,xy.lookup(2.6f));
    EXPECT_EQ(6.0f,xy.lookup(3.0f));
}

TEST(xy_table_t, test_interp_InRange)
{
    xy_table_t xy;
    xy.add_entry(1.0f,2.0f);
    xy.add_entry(2.0f,4.0f);
    xy.add_entry(3.0f,6.0f);
    EXPECT_EQ(2.0f,xy.interp(1.0f));
    EXPECT_EQ(4.0f,xy.interp(2.0f));
    EXPECT_EQ(4.8f,xy.interp(2.4f));
    EXPECT_EQ(3.2f,xy.interp(1.6f));
    EXPECT_EQ(5.2f,xy.interp(2.6f));
    EXPECT_EQ(6.0f,xy.interp(3.0f));
}

TEST(xy_table_t, test_lookup_OutOfRange)
{
    xy_table_t xy;
    xy.add_entry(1.0f,2.0f);
    xy.add_entry(2.0f,4.0f);
    xy.add_entry(3.0f,6.0f);
    EXPECT_EQ(2.0f,xy.lookup(-2.2f));
    EXPECT_EQ(2.0f,xy.lookup(-1.0f));
    EXPECT_EQ(6.0f,xy.lookup(3.2f));
    EXPECT_EQ(6.0f,xy.lookup(13.2f));
}

TEST(xy_table_t, test_interp_OutOfRange)
{
    xy_table_t xy;
    xy.add_entry(1.0f,2.0f);
    xy.add_entry(2.0f,4.0f);
    xy.add_entry(3.0f,6.0f);
    EXPECT_EQ(-4.4f,xy.interp(-2.2f));
    EXPECT_EQ(-2.0f,xy.interp(-1.0f));
    EXPECT_EQ(6.4f,xy.interp(3.2f));
    EXPECT_EQ(26.4f,xy.interp(13.2f));
}

TEST(xy_table_t, test_interp_SingleEntry)
{
    xy_table_t xy;
    xy.add_entry(1.0f,2.0f);
    EXPECT_EQ(2.0f,xy.interp(-2.2f));
    EXPECT_EQ(2.0f,xy.interp(1.0f));
    EXPECT_EQ(2.0f,xy.interp(2.3f));
}

TEST(xy_table_t, test_interp_NonMonotonicInsertion)
{
    xy_table_t xy;
    xy.add_entry(1.0f,2.0f);
    xy.add_entry(2.0f,4.0f);
    xy.add_entry(3.0f,6.0f);
    EXPECT_EQ(4.4f,xy.interp(2.2f));
    xy.add_entry(2.4f,-1.0f);
    EXPECT_EQ(1.5f,xy.interp(2.2f));
    EXPECT_EQ(-1.0f,xy.interp(2.4f));
}

TEST(xy_table_t, test_interp_XFun)
{
    xy_table_t xy;
    xy.set_xfun(logf);
    xy.add_entry(1.0f,2.0f);
    xy.add_entry(2.0f,4.0f);
    xy.add_entry(8.0f,6.0f);
    EXPECT_EQ(2.0f,xy.interp(1.0f));
    EXPECT_EQ(4.0f,xy.interp(2.0f));
    EXPECT_EQ(6.0f,xy.interp(8.0f));
    EXPECT_EQ(5.0f,xy.interp(4.0f));
    EXPECT_EQ(3.0f,xy.interp(1.414213562373095145f));
}


// Local Variables:
// compile-command: "make -C .. unit-tests"
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
