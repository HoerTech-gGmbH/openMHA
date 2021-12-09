// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2003 2004 2005 2006 2008 2013 2016 2017 HörTech gGmbH
// Copyright © 2021 Hörzentrum Oldenburg gGmbH
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

#ifndef __MHA_DEFS_H__
#define __MHA_DEFS_H__

/**
   \file mha_defs.h
   \brief Preprocessor definitions common to all MHA components

   This file contains all preprocessor and type definitions which are
   common to all Master Hearing Aid components.
*/

#define CHECK_EXPR(x) {if(!(x)){throw MHA_Error(__FILE__,__LINE__,"The expression \"" #x "\" is invalid.");}}
#define CHECK_VAR(x) {if(!(x)){throw MHA_Error(__FILE__,__LINE__,"The variable \"" #x "\" is not defined.");}}

/**
 Define pi if it is not defined yet.
 */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#endif

/*
 Local Variables:
 compile-command: "make -C .."
 coding: utf-8-unix
 indent-tabs-mode: nil
 End:
*/
