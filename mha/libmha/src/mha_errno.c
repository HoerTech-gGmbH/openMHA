// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2013 2016 HörTech gGmbH
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

#include "mha_errno.h"
#include <string.h>

#define STRLEN 0x1000

char next_except_str[STRLEN] = "";

const char* cstr_strerror[MHA_ERR_USER] = {
    "success",
    "invalid handle",
    "invalid range",
    "invalid format"
};

const char* mha_strerror(int mhaerrno)
{
    if( mhaerrno >= MHA_ERR_USER )
        return next_except_str;
    switch( mhaerrno ){
        case MHA_ERR_SUCCESS :
            return "sucess";
        case MHA_ERR_INVALID_HANDLE :
            return "invalid handle";
        case MHA_ERR_NULL :
            return "NULL pointer";
        case MHA_ERR_VARRANGE :
            return "invalid variable range";
        case MHA_ERR_VARFMT :
            return "invalid variable format";
        default :
            return "unknown error";
    }
}

void mha_set_user_error(const char* str)
{
    strncpy( next_except_str, str, STRLEN );
    next_except_str[STRLEN-1] = 0;
}

/*
 * Local Variables:
 * compile-command: "make -C .."
 * coding: utf-8-unix
 * End:
 */
