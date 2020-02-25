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

#include "altconfig.hh"
#include <gtest/gtest.h>
class altconfig_t_testing : public ::testing::Test {
protected:
  altconfig_t plug={algo_comm_t(),"",""};
};

TEST_F(altconfig_t_testing,set_algos_success){
  plug.parse("algos=[A B C D E]");
  EXPECT_EQ("[A B C D E]",plug.parse("algos?val"));

  // Verify check for existing sub parsers
  plug.parse("algos=[A B AB]");
  EXPECT_NO_THROW(plug.parse("A=Foo"));
  EXPECT_NO_THROW(plug.parse("B=Bar"));
  EXPECT_EQ("[A B AB]",plug.parse("algos?val"));

  EXPECT_NO_THROW(plug.parse("algos=[AB A B]"));
  EXPECT_NO_THROW(plug.parse("A=Foo"));
  EXPECT_NO_THROW(plug.parse("B=Bar"));
  EXPECT_EQ("[AB A B]",plug.parse("algos?val"));
}

TEST_F(altconfig_t_testing,set_algos_fail){
  EXPECT_NO_THROW(plug.parse("algos=[A B C D E]"));
  EXPECT_NO_THROW(plug.parse("A=Foo"));
  EXPECT_NO_THROW(plug.parse("B=Bar"));
  // Trying to use a reserved word a algo name must fail
  EXPECT_THROW(plug.parse("algos=[mhaconfig_in]"),MHA_Error);
  // Test the restore
  EXPECT_EQ("[A B C D E]",plug.parse("algos?val"));
  EXPECT_EQ("Foo",plug.parse("A?val"));
  EXPECT_EQ("Bar",plug.parse("B?val"));
  EXPECT_NO_THROW(plug.parse("A=Lorem"));
  EXPECT_EQ("Lorem",plug.parse("A?val"));
}

TEST_F(altconfig_t_testing,set_algo_success){
  EXPECT_NO_THROW(plug.parse("algos=[A]"));
  EXPECT_NO_THROW(plug.parse("A=Foo"));
  EXPECT_EQ("Foo",plug.parse("A?val"));
}

TEST_F(altconfig_t_testing,set_algo_fail){
  EXPECT_NO_THROW(plug.parse("algos=[A]"));
  EXPECT_THROW(plug.parse("B=Foo"),MHA_Error);
}

TEST_F(altconfig_t_testing,set_algos_fail_recovery_removes){
  // set alternatives to just "A"
  plug.parse("algos=[A]");
  // Check that there is a parser "A" but no parser "B"
  EXPECT_NO_THROW(plug.parse("A?"));
  EXPECT_THROW(plug.parse("B?"), MHA_Error);

  // Perform a failing update of alternatives to "B" and "plugin_name"
  // "plugin_name" is an existing parser entry, this will fail.
  EXPECT_THROW(plug.parse("algos=[B plugin_name]"),MHA_Error);

  // When altconfig detected that it could not use the name "plugin_name",
  // it performed a rollback and restored "A".  Check that this rollback
  // did not leave a parser "B" behind.  Accessing B should throw, as before.
  EXPECT_THROW(plug.parse("B?"), MHA_Error);
}

TEST_F(altconfig_t_testing,set_algos_fail_recovery_preserves){
  // set alternatives to just "A"
  plug.parse("algos=[A]");
  // Check that there is a parser "A" and assign a marker value
  EXPECT_NO_THROW(plug.parse("A=marker"));

  // Perform a failing update of alternatives to "A" and "plugin_name"
  // "plugin_name" is an existing parser entry, this will fail.
  EXPECT_THROW(plug.parse("algos=[A plugin_name]"),MHA_Error);

  // When altconfig detected that it could not use the name "plugin_name",
  // it performed a rollback.  Check that this rollback did not compromise
  // parser "A". Accessing A return the same value as before.
  EXPECT_EQ("marker",plug.parse("A?val"));
}

// This test only works if sine is in the MHA_LIBRARY_PATH/LD_LIBRARY_PATH, which is
// not the case for a default "make unit-tests"
// TEST_F(altconfig_t_testing,set_select){
//   plug.parse("algos=[A B C D E]");
//   plug.parse("plugin_name=sine");
//   plug.parse("A=f=440");
//   plug.parse("B=f=880");
//   plug.parse("select=A");
//   EXPECT_EQ("440",plug.parse("sine.f?val"));
//   plug.parse("B=f=880");
//   plug.parse("select=B");
//   EXPECT_EQ("880",plug.parse("sine.f?val"));
// }

// Local Variables:
// compile-command: "make unit-tests"
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
