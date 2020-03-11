// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2019 2020 HörTech gGmbH
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

#include "mha_utils.hh"
#include "mha_tablelookup.hh"
#include <array>
#include <iostream>

namespace {
  static constexpr std::array<float,29> hl_freqs={125, 160, 200, 250, 315, 400, 500, 630, 750, 800, 1000, 1250,
                                                  1500, 1600, 2000, 2500, 3000, 3150, 4000, 5000, 6000, 6300,
                                                  8000, 9000, 10000, 11200, 12500, 14000, 16000};

  static constexpr std::array<float,29> hl_vals={22.1, 17.9, 14.4, 11.4, 8.6, 6.2, 4.4, 3.0, 2.4, 2.2, 2.4, 3.5,
                                                 2.4, 1.7, -1.3, -4.2, -5.8, -6.0, -5.4, -1.5, 4.3, 6.0, 12.6,
                                                 13.9, 13.9, 13.0, 12.3, 18.4, 40.2};

  static_assert(hl_freqs.size()==hl_vals.size(),"__FILE__:__LINE__: dB(SPL) to dB(HL) x and y"
                " vectors must have same size!");
}

mha_real_t MHAUtils::spl2hl(mha_real_t f){
  //Thread safe for C++11 and later
  static MHATableLookup::xy_table_t spl_to_hl_table=[](){
                                                      MHATableLookup::xy_table_t res;
                                                      for(unsigned idx=0U; idx<hl_freqs.size();++idx){
                                                        res.add_entry(hl_freqs[idx],-hl_vals[idx]);
                                                      }
                                                      return res;
                                                    }();
  if(f<0)
    throw MHA_Error(__FILE__,__LINE__,"SPL to HL conversion not defined for frequencies < 0 Hz!");
  return spl_to_hl_table.interp(f);
}

// Local Variables:
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
