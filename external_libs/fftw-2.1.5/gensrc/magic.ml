(*
 * Copyright (c) 1997-1999, 2003 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *)

(* $Id: magic.ml,v 1.30 2003/03/16 23:43:46 stevenj Exp $ *)

(* magic parameters *)

let window = ref 5

let number_of_variables = ref 4

let use_wsquare = ref false

let inline_single = ref true

type twiddle_policy =
    TWIDDLE_LOAD_ALL
  | TWIDDLE_ITER
  | TWIDDLE_LOAD_ODD
  | TWIDDLE_SQUARE1
  | TWIDDLE_SQUARE2
  | TWIDDLE_SQUARE3

let twiddle_policy = ref TWIDDLE_LOAD_ALL

let inline_konstants = ref false
let inline_loads = ref false
let loopo = ref false

let rader_min = ref 13
let rader_list = ref [5]

let alternate_convolution = ref 17

let alignment_check = ref false
let times_3_3 = ref false

let enable_fma = ref false
let enable_fma_expansion = ref false

let collect_common_twiddle = ref true
let collect_common_inputs = ref true

let verbose = ref false

let athenafft = ref false
