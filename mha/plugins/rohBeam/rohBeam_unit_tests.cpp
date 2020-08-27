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
#include "rohBeam.hh"


TEST(j0,compare_to_reference){
  //j0(x), x=0,0.1,...,10, bionic default implementation
  constexpr double j0_expected[]={1, 0.997502, 0.990025, 0.977626, 0.960398, 0.93847, 0.912005, 0.881201, 0.846287, 0.807524,
                                  0.765198, 0.719622, 0.671133, 0.620086, 0.566855, 0.511828, 0.455402, 0.397985, 0.339986,
                                  0.281819, 0.223891, 0.166607, 0.110362, 0.0555398, 0.00250768, -0.0483838, -0.096805,
                                  -0.142449, -0.185036, -0.224312, -0.260052, -0.292064, -0.320188, -0.344296, -0.364296,
                                  -0.380128, -0.391769, -0.39923, -0.402556, -0.401826, -0.39715, -0.38867, -0.376557,
                                  -0.361011, -0.342257, -0.320543, -0.296138, -0.269331, -0.240425, -0.209738, -0.177597,
                                  -0.144335, -0.11029, -0.0758031, -0.0412101, -0.00684387, 0.0269709, 0.05992, 0.0917026,
                                  0.122033, 0.150645, 0.177291, 0.201747, 0.223812, 0.243311, 0.260095, 0.274043, 0.285065,
                                  0.293096, 0.298102, 0.300079, 0.299051, 0.295071, 0.288217, 0.278596, 0.26634, 0.251602,
                                  0.234559, 0.215408, 0.194362, 0.171651, 0.147517, 0.122215, 0.0960061, 0.0691573, 0.0419393,
                                  0.014623, -0.0125227, -0.0392338, -0.0652532, -0.0903336, -0.114239, -0.136748, -0.157655,
                                  -0.176772, -0.193929, -0.208979, -0.221795, -0.232276, -0.240341, -0.245936};
  for(int i=0; i<101; i++){
    EXPECT_NEAR(rohBeam::j0(0.1*i),j0_expected[i],1e-5);
  }
}

// Local Variables:
// compile-command: "make unit-tests"
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
