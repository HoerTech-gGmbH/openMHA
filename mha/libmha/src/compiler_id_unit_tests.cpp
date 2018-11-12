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
#include "compiler_id.hh"
#include <cstdio>
// we test our own compiler detection against the one from boost
#include <boost/predef.h>

/* Older boost versions do not have these macros, so define them if needed. */
#ifndef BOOST_VERSION_NUMBER_MAJOR
#define BOOST_VERSION_NUMBER_MAJOR(N) \
    ( ((N)/10000000)%100 )
#endif //BOOST_VERSION_NUMBER_MAJOR

#ifndef BOOST_VERSION_NUMBER_MINOR
#define BOOST_VERSION_NUMBER_MINOR(N) \
    ( ((N)/100000)%100 )
#endif //BOOST_VERSION_NUMBER_MINOR

#ifndef BOOST_VERSION_NUMBER_PATCH
#define BOOST_VERSION_NUMBER_PATCH(N) \
( (N)%100000 )
#endif //BOOST_VERSION_NUMBER_PATCH

#ifndef __clang_version__
#define __clang_version__ "0"
#endif

#ifndef __VERSION__
#define  __VERSION__ "0"
#endif

TEST(compiler_id, compiler_vendor_and_version)
{
  if (BOOST_COMP_GNUC) {
    EXPECT_STREQ("gcc", COMPILER_ID_VENDOR);
    EXPECT_EQ(BOOST_VERSION_NUMBER_MAJOR(BOOST_COMP_GNUC), COMPILER_ID_MAJOR);
    EXPECT_EQ(BOOST_VERSION_NUMBER_MINOR(BOOST_COMP_GNUC), COMPILER_ID_MINOR);
    EXPECT_EQ(BOOST_VERSION_NUMBER_PATCH(BOOST_COMP_GNUC), COMPILER_ID_PATCH);
    EXPECT_EQ(0, strncmp(__VERSION__, COMPILER_ID_VERSION,
                         strlen(COMPILER_ID_VERSION)));
  }
  else if (BOOST_COMP_CLANG) {
    EXPECT_STREQ("clang", COMPILER_ID_VENDOR);
    EXPECT_EQ(BOOST_VERSION_NUMBER_MAJOR(BOOST_COMP_CLANG), COMPILER_ID_MAJOR);
    EXPECT_EQ(BOOST_VERSION_NUMBER_MINOR(BOOST_COMP_CLANG), COMPILER_ID_MINOR);
    EXPECT_EQ(BOOST_VERSION_NUMBER_PATCH(BOOST_COMP_CLANG), COMPILER_ID_PATCH);
    EXPECT_EQ(0, strncmp(__clang_version__, COMPILER_ID_VERSION,
                         strlen(COMPILER_ID_VERSION)));
  }
  else {
    EXPECT_STRNE("gcc", COMPILER_ID_VENDOR);
    EXPECT_STRNE("clang", COMPILER_ID_VENDOR);
    FAIL(/* unexpected compiler */);
  }
  printf("COMPILER_ID=%s\n", COMPILER_ID);
}

// Local Variables:
// compile-command: "make -C .. unit-tests"
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
