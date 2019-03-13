// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2019 HörTech gGmbH
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

#ifndef __MHA_UTILS_HH__
#define __MHA_UTILS_HH__

#include "mha_error.hh"
#include <string>
#include <algorithm>
namespace MHAUtils {
  inline bool is_multiple_of(const unsigned big, const unsigned small) {
    if(small==0)
      return false;
    else
      return (big % small) == 0U;
    }

    inline bool is_power_of_two(const unsigned n) {
        return n != 0U && (n & (n-1U)) == 0U;
    }

    inline bool is_multiple_of_by_power_of_two(const unsigned big, const unsigned small) {
        return is_multiple_of(big, small) && is_power_of_two(big / small);
    }

  inline std::string strip(const std::string& line){
    std::string res(line);
    while (res.size() && (res.back() == '\r' || res.back() == '\n' || res.back()==' ')) {
      res.resize(res.size()-1);
    }
    return res;
  }

  inline std::string remove(const std::string& str_, char c){
    auto str(str_);
    std::string::iterator end_pos = std::remove(str.begin(), str.end(), c);
    str.erase(end_pos, str.end());
    return str;
  }
} // MHAUtils

#endif // __MHA_UTILS_HH__

// Local Variables:
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
