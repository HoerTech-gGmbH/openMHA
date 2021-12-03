// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2021 HörTech gGmbH
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

TEST(comm_var_map_t, is_prepared)
{
  MHAKernel::comm_var_map_t s;
  EXPECT_FALSE(s.is_prepared);
}

TEST(comm_var_map_t, has_key_insert_erase_while_unprepared)
{
  MHAKernel::comm_var_map_t s;
  EXPECT_FALSE(s.has_key("key"));
  comm_var_t v = {};
  s.insert("key",v);
  EXPECT_TRUE(s.has_key("key"));
  s.erase_by_name("key");
  EXPECT_FALSE(s.has_key("key"));
  s.insert("key",v);
  EXPECT_TRUE(s.has_key("key"));
  s.erase_by_pointer(v.data);
  EXPECT_FALSE(s.has_key("key"));

  // Trying to erase what is not there while unprepared does nothing
  EXPECT_NO_THROW(s.erase_by_name("key"));
  EXPECT_NO_THROW(s.erase_by_pointer(v.data));
}

TEST(comm_var_map_t, insert_erase_disallowed_while_prepared)
{
  MHAKernel::comm_var_map_t s;
  comm_var_t v = {};
  s.is_prepared = true;
  EXPECT_THROW(s.insert("key",v), MHA_Error);
  EXPECT_FALSE(s.has_key("key"));

  s.is_prepared = false;
  EXPECT_NO_THROW(s.insert("key",v));
  EXPECT_TRUE(s.has_key("key"));

  s.is_prepared = true;
  EXPECT_THROW(s.erase_by_name("key"), MHA_Error);
  EXPECT_THROW(s.erase_by_pointer(v.data), MHA_Error);
  EXPECT_TRUE(s.has_key("key"));
}

TEST(comm_var_map_t, retrieve_get_entries_size)
{
  MHAKernel::comm_var_map_t s;
  int i1 = 1, i2 = 2;
  comm_var_t v1 = {}, v2a = {}, v2b={};
  v1.data = &i1;
  v2a.data = v2b.data = &i2; // two AC references to same memory
  EXPECT_THROW(s.retrieve("key1"), MHA_Error);
  EXPECT_EQ("", s.get_entries());
  EXPECT_EQ(0U, s.size());

  s.insert("key1",v1);
  EXPECT_EQ(&i1, s.retrieve("key1").data);
  EXPECT_EQ("key1", s.get_entries());
  EXPECT_EQ(1U, s.size());

  s.insert("key2a",v2a);
  EXPECT_EQ(&i2, s.retrieve("key2a").data);
  EXPECT_EQ(&i1, s.retrieve("key1").data);
  EXPECT_EQ("key1 key2a", s.get_entries());
  EXPECT_EQ(2U, s.size());

  s.insert("key2b",v2b);
  EXPECT_EQ(s.retrieve("key2a").data, s.retrieve("key2b").data);
  EXPECT_EQ(&i1, s.retrieve("key1").data);
  EXPECT_EQ("key1 key2a key2b", s.get_entries());
  EXPECT_EQ(3U, s.size());

  s.erase_by_name("key1");
  EXPECT_EQ(&i2, s.retrieve("key2a").data);
  EXPECT_EQ("key2a key2b", s.get_entries());
  EXPECT_EQ(2U, s.size());

  s.erase_by_pointer(&i2); // erases both: key2a and key2b
  EXPECT_THROW(s.retrieve("key2a"), MHA_Error);
  EXPECT_THROW(s.retrieve("key2b"), MHA_Error);
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
