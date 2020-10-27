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
#ifndef MHAIOUTILS
#define MHAIOUTILS

#include <limits>
#include <cmath>

namespace mhaioutils {
  template<typename T>
  T to_int_clamped(float val){
    constexpr float gain = -1.0f / std::numeric_limits<T>::min();
    constexpr float invgain = 1.0f / gain;
    if (std::isnan(val))
      return 0;
    else if(val>=1.0f)
      return std::numeric_limits<T>::max();
    else if(val<=-1.0f)
      return std::numeric_limits<T>::min();
    else
      return static_cast<T>(invgain * val);
  }
}
#endif
