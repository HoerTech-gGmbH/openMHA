// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2018 HörTech gGmbH
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
#include <gmock/gmock.h>
#include "mha_os.h"

TEST(mha_os, mha_hasenv)
{
  EXPECT_TRUE(mha_hasenv("PATH"));
  EXPECT_FALSE(mha_hasenv("NO_SUCH_ENVIRONMENT_VARIABLE_IN_2018"));
}

TEST(mha_os, mha_setenv_delenv)
{
  EXPECT_FALSE(mha_hasenv("NO_SUCH_ENVIRONMENT_VARIABLE_IN_2018"));
  mha_setenv("NO_SUCH_ENVIRONMENT_VARIABLE_IN_2018", "hello");
  EXPECT_TRUE(mha_hasenv("NO_SUCH_ENVIRONMENT_VARIABLE_IN_2018"));
  mha_delenv("NO_SUCH_ENVIRONMENT_VARIABLE_IN_2018");
  EXPECT_FALSE(mha_hasenv("NO_SUCH_ENVIRONMENT_VARIABLE_IN_2018"));
}

TEST(mha_os, mha_getenv)
{
  using ::testing::StartsWith;
#ifdef _WIN32
  EXPECT_THAT(mha_getenv("PATH"), StartsWith("C:\\"));
#else
  EXPECT_THAT(mha_getenv("PATH"), StartsWith("/"));
#endif

  EXPECT_NE("hello", mha_getenv("MHA_OS_UNIT_TEST_SETS_THIS_TO_HELLO"));
  mha_setenv("MHA_OS_UNIT_TEST_SETS_THIS_TO_HELLO", "hello");
  EXPECT_EQ("hello", mha_getenv("MHA_OS_UNIT_TEST_SETS_THIS_TO_HELLO"));
}

TEST(mha_os, mha_stash_environment_variable_t_hide_existing)
{
  EXPECT_TRUE(mha_hasenv("PATH"));
  std::string original_value = mha_getenv("PATH");

  EXPECT_EQ(original_value, mha_getenv("PATH"));

  {
    // allocation will hide the original PATH environment variable
    mha_stash_environment_variable_t hidepath("PATH", "/useless/path");
    EXPECT_NE(original_value, mha_getenv("PATH"));
    EXPECT_EQ("/useless/path",mha_getenv("PATH"));
    // deallocation will restore original value
  }

  EXPECT_EQ(original_value, mha_getenv("PATH"));
}

TEST(mha_os, mha_stash_environment_variable_t_hide_nonexisting)
{
  EXPECT_FALSE(mha_hasenv("NONEXISTING_ENVIRONMENT_VARIABLE"));

  {
    // allocation will hide the original PATH environment variable
    mha_stash_environment_variable_t hidepath("NONEXISTING_ENVIRONMENT_VARIABLE",
                                              "hello");
    EXPECT_TRUE(mha_hasenv("NONEXISTING_ENVIRONMENT_VARIABLE"));
    EXPECT_NE("", mha_getenv("NONEXISTING_ENVIRONMENT_VARIABLE"));
    EXPECT_EQ("hello",mha_getenv("NONEXISTING_ENVIRONMENT_VARIABLE"));
    // deallocation will restore the original undefinedness
  }

  EXPECT_FALSE(mha_hasenv("NONEXISTING_ENVIRONMENT_VARIABLE"));
}

// Local Variables:
// compile-command: "make -C .. unit-tests"
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
