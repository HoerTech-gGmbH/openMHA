// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2013 2016 2017 2018 HörTech gGmbH
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

#ifndef _MHA_ERRNO_H_
#define _MHA_ERRNO_H_

#define MHA_ERR_SUCCESS 0
#define MHA_ERR_UNKNOWN 1
#define MHA_ERR_INVALID_HANDLE 2
#define MHA_ERR_NULL 3
#define MHA_ERR_VARRANGE 4
#define MHA_ERR_VARFMT 5
#define MHA_ERR_USER 10000

#ifdef __cplusplus
extern "C" {
#endif

const char* mha_strerror(int mhaerrno);

void mha_set_user_error(const char* str);

#ifdef __cplusplus
}
#endif

#endif

/*
 * Local Variables:
 * compile-command: "make -C .."
 * coding: utf-8-unix
 * indent-tabs-mode: nil
 * End:
 */
