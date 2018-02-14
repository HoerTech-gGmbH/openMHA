// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2014 2018 HörTech gGmbH
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

#ifndef __HANN_H__
#define __HANN_H__

#ifdef __cplusplus
extern "C" {
#endif

float* hannf(const unsigned int N);
double* hann(const unsigned int N);

#ifdef __cplusplus
}
#endif

#endif /* __HANN_H__ */

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
