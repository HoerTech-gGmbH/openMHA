// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2021 HörTech gGmbH
// Copyright © 2021 2022 Hörzentrum Oldenburg gGmbH
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

#include "mha_algo_comm.hh"
#include <gtest/gtest.h>

// This using directive allows to specify non-const std::string objects through
// literals directly in source code by using string literals with a suffix.
// Example: "key"s
using namespace std::string_literals;
TEST(comm_var_map_t, string_literals_are_available)
{
  auto string_literal = "test"s;
  std::string & reference = string_literal;
  const std::string & constref = string_literal;
  // All point to the same address (identity comparison)
  EXPECT_EQ(&string_literal, &reference);
  EXPECT_EQ(&string_literal, &constref);
}

TEST(comm_var_map_t, is_prepared)
{
  MHAKernel::comm_var_map_t s;
  EXPECT_FALSE(s.is_prepared);
}

TEST(comm_var_map_t, has_key_insert_erase_while_unprepared)
{
  MHAKernel::comm_var_map_t s;
  EXPECT_FALSE(s.has_key("key"s));
  comm_var_t v = {};
  s.insert("key"s,v);
  EXPECT_TRUE(s.has_key("key"s));
  s.erase_by_name("key"s);
  EXPECT_FALSE(s.has_key("key"s));
  s.insert("key"s,v);
  EXPECT_TRUE(s.has_key("key"s));
  s.erase_by_pointer(v.data);
  EXPECT_FALSE(s.has_key("key"s));

  // Trying to erase what is not there while unprepared does nothing
  EXPECT_NO_THROW(s.erase_by_name("key"s));
  EXPECT_NO_THROW(s.erase_by_pointer(v.data));
}

TEST(comm_var_map_t, insert_erase_disallowed_while_prepared)
{
  MHAKernel::comm_var_map_t s;
  comm_var_t v = {};
  s.is_prepared = true;
  EXPECT_THROW(s.insert("key"s,v), MHA_Error);
  EXPECT_FALSE(s.has_key("key"s));

  s.is_prepared = false;
  EXPECT_NO_THROW(s.insert("key"s,v));
  EXPECT_TRUE(s.has_key("key"s));

  s.is_prepared = true;
  EXPECT_THROW(s.erase_by_name("key"s), MHA_Error);
  EXPECT_THROW(s.erase_by_pointer(v.data), MHA_Error);
  EXPECT_TRUE(s.has_key("key"s));
}

TEST(comm_var_map_t, retrieve_get_entries_size)
{
  MHAKernel::comm_var_map_t s;
  int i1 = 1, i2 = 2;
  comm_var_t v1 = {}, v2a = {}, v2b={};
  v1.data = &i1;
  v2a.data = v2b.data = &i2; // two AC references to same memory
  EXPECT_THROW(s.retrieve("key1"s), MHA_Error);
  EXPECT_EQ("", s.get_entries());
  EXPECT_EQ(0U, s.size());

  s.insert("key1"s,v1);
  EXPECT_EQ(&i1, s.retrieve("key1"s).data);
  EXPECT_EQ("key1"s, s.get_entries());
  EXPECT_EQ(1U, s.size());

  s.insert("key2a"s,v2a);
  EXPECT_EQ(&i2, s.retrieve("key2a"s).data);
  EXPECT_EQ(&i1, s.retrieve("key1"s).data);
  EXPECT_EQ("key1 key2a"s, s.get_entries());
  EXPECT_EQ(2U, s.size());

  s.insert("key2b"s,v2b);
  EXPECT_EQ(s.retrieve("key2a"s).data, s.retrieve("key2b"s).data);
  EXPECT_EQ(&i1, s.retrieve("key1"s).data);
  EXPECT_EQ("key1 key2a key2b"s, s.get_entries());
  EXPECT_EQ(3U, s.size());

  s.erase_by_name("key1"s);
  EXPECT_EQ(&i2, s.retrieve("key2a"s).data);
  EXPECT_EQ("key2a key2b"s, s.get_entries());
  EXPECT_EQ(2U, s.size());

  s.erase_by_pointer(&i2); // erases both: key2a and key2b
  EXPECT_THROW(s.retrieve("key2a"s), MHA_Error);
  EXPECT_THROW(s.retrieve("key2b"s), MHA_Error);
  EXPECT_EQ("", s.get_entries());
  EXPECT_EQ(0U, s.size());

  EXPECT_EQ(1, i1);
  EXPECT_EQ(2, i2);
}

/*
 * Local Variables:
 * compile-command: "make -C .. unit-tests"
 * coding: utf-8-unix
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
