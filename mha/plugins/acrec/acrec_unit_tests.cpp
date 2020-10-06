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


#include "acrec.hh"
#include <gtest/gtest.h>
#include <ctime>
using namespace plugins::hoertech::acrec;
TEST(to_iso8601,epoch){
  // Semi-arbitrary date
  std::tm tm = { /*.tm_sec  =*/ 0,
                 /*.tm_min  =*/ 0,
                 /*.tm_hour =*/ 0,
                 /*.tm_mday =*/ 1,
                 /*.tm_mon  =*/ 0,
                 /*.tm_year =*/ 100,
                 /*.tm_isdst=*/ 0
  };
  auto expected=std::string("2000-01-01T000000");
  mktime(&tm);
  auto actual=to_iso8601(mktime(&tm)) ;
  EXPECT_EQ(expected,actual);
}
