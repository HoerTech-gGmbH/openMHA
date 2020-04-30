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
#include "mha_parser.hh"
#include "mha_error.hh"

TEST(mha_parser, insert_2_subparsers_with_same_name_fails)
{
  // create a parser
  MHAParser::parser_t parser;

  // create two parser variables
  MHAParser::int_t i1("i1 help","0","[,]");
  MHAParser::int_t i2("i2 help","0","[,]");

  // try to register both parser variables under the same name
  parser.insert_item("i", &i1);
  EXPECT_THROW(parser.insert_item("i", &i2), MHA_Error);

  // try again to check the error message content
  try {
    parser.insert_item("i", &i2);
    FAIL() << "parser.insert_item should have thrown an exception";
  } catch(MHA_Error & e) {
    ASSERT_STREQ("(mha_parser) The entry \"i\" already exists.", e.get_msg());
  }
}

TEST(mha_parser,num_brackets)
{
  EXPECT_EQ(-1,MHAParser::StrCnv::num_brackets(""));
  EXPECT_EQ(-2,MHAParser::StrCnv::num_brackets("["));
  EXPECT_EQ(-2,MHAParser::StrCnv::num_brackets("]"));
  EXPECT_EQ(2,MHAParser::StrCnv::num_brackets("[]"));
  EXPECT_EQ(2,MHAParser::StrCnv::num_brackets("[foo]"));
  EXPECT_EQ(-2,MHAParser::StrCnv::num_brackets("]bar["));
  EXPECT_EQ(4,MHAParser::StrCnv::num_brackets("[foo[bar]]"));
  EXPECT_EQ(-2,MHAParser::StrCnv::num_brackets("[[[foo]]"));
  EXPECT_EQ(-2,MHAParser::StrCnv::num_brackets("[[bar]]]"));
  EXPECT_EQ(4,MHAParser::StrCnv::num_brackets("[[foo][bar]]"));
}

// Local Variables:
// compile-command: "make -C .. unit-tests"
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
