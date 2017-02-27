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

(* $Id: variable.ml,v 1.21 2003/03/16 23:43:46 stevenj Exp $ *)

(* Data types and functions for dealing with variables in symbolic
 * expressions and the abstract syntax tree. *)

(* Variables fall into one of four categories: temporary variables
 * (which the generator can add or delete at will), named (fixed)
 * variables, and the real/imaginary parts of complex arrays.  Arrays
 * can be either input arrays, output arrays, or arrays of precomputed
 * twiddle factors (roots of unity). *)

type array = 
    Input
  | Output
  | Twiddle

type variable =
    Temporary of int
  | Named of string
  | RealArrayElem of (array * int)
  | ImagArrayElem of (array * int)

let make_temporary =
  let tmp_count = ref 0
  in fun () -> begin
    tmp_count := !tmp_count + 1;
    Temporary !tmp_count
  end

let is_temporary = function
    Temporary _ -> true
  | _ -> false

let is_output = function
    RealArrayElem (Output, _) -> true
  | ImagArrayElem (Output, _) -> true
  | _ -> false

let is_input = function
    RealArrayElem (Input, _) -> true
  | ImagArrayElem (Input, _) -> true
  | _ -> false

let is_twiddle = function
    RealArrayElem (Twiddle, _) -> true
  | ImagArrayElem (Twiddle, _) -> true
  | _ -> false

let same = (=)

let hash = function
  | RealArrayElem (Input, i) ->  i * 8
  | ImagArrayElem (Input, i) ->  -i * 8 + 1
  | RealArrayElem (Output, i) ->  i * 8 + 2
  | ImagArrayElem (Output, i) ->  -i * 8 + 3
  | RealArrayElem (Twiddle, i) ->  i * 8 + 4
  | ImagArrayElem (Twiddle, i) ->  -i * 8 + 5
  | _ -> 0
  
let similar a b = 
  same a b or
  (match (a, b) with
    (RealArrayElem (a1, k1), ImagArrayElem (a2, k2)) -> 
      a1 = a2 && k1 = k2
  | (ImagArrayElem (a1, k1), RealArrayElem (a2, k2)) -> 
      a1 = a2 && k1 = k2
  | _ -> false)
    
(* true if assignment of a clobbers variable b *)
let clobbers a b =
  match (a, b) with
    (RealArrayElem (Output, k1), RealArrayElem (Input, k2)) -> k1 = k2
  | (ImagArrayElem (Output, k1), ImagArrayElem (Input, k2)) -> k1 = k2
  | _ -> false

(* true if a is the real part and b the imaginary of the same array *)
let real_imag a b =
  match (a, b) with
    (RealArrayElem (a1, k1), ImagArrayElem (a2, k2)) -> 
      a1 = a2 && k1 = k2 
  | _ -> false

(* true if a and b are elements of the same array, and a has smaller index *)
let increasing_indices a b =
  match (a, b) with
    (RealArrayElem (a1, k1), RealArrayElem (a2, k2)) -> 
      a1 = a2 && k1 < k2 
  | (RealArrayElem (a1, k1), ImagArrayElem (a2, k2)) -> 
      a1 = a2 && k1 < k2 
  | (ImagArrayElem (a1, k1), RealArrayElem (a2, k2)) -> 
      a1 = a2 && k1 < k2 
  | (ImagArrayElem (a1, k1), ImagArrayElem (a2, k2)) -> 
      a1 = a2 && k1 < k2 
  | _ -> false

let access array k =
  (RealArrayElem (array, k),  ImagArrayElem (array, k))

let access_input = access Input
let access_output = access Output
let access_twiddle = access Twiddle

let make_named name = Named name

let unparse_index name stride k = 
  let index = 
    match (stride, k) with
      (_, 0) -> "0"
    | (Some s, 1) -> s
    | (Some s, (-1)) -> "-" ^ s
    | (None, k) -> (string_of_int k)
    | (Some s, k) -> (string_of_int k) ^ " * " ^ s 
  in name ^ "[" ^ index ^ "]"

let default_unparser = function
      Temporary k -> "tmp" ^ (string_of_int k)
    | Named s -> s
    | _ -> failwith "attempt to unparse unknown variable"

let make_unparser (input_name, input_stride)
    (output_name, output_stride)
    (twiddle_name, twiddle_stride) =
  let rec unparse_var = function
    | RealArrayElem (array, k) ->
        "c_re(" ^ (unparse_array array k) ^ ")"
    | ImagArrayElem (array, k) ->
        "c_im(" ^ (unparse_array array k) ^ ")"
    | x -> default_unparser x

  and unparse_array = function
      Input -> unparse_index input_name input_stride
    | Output -> unparse_index output_name output_stride
    | Twiddle -> unparse_index twiddle_name twiddle_stride

  in unparse_var

let make_real2hc_unparser (input_name, input_stride)
    (real_output_name, real_output_stride)
    (imag_output_name, imag_output_stride) = function
    | RealArrayElem (Input, k) ->
	unparse_index input_name input_stride k
    | ImagArrayElem (Input, _) ->
	failwith "trying to access imaginary part of real input"
    | RealArrayElem (Output, k) ->
	unparse_index real_output_name real_output_stride k
    | ImagArrayElem (Output, k) ->
	unparse_index imag_output_name imag_output_stride k
    | x -> default_unparser x

let make_hc2real_unparser 
    (real_input_name, real_input_stride)
    (imag_input_name, imag_input_stride)
    (output_name, output_stride) = function
    | RealArrayElem (Input, k) ->
	unparse_index real_input_name real_input_stride k
    | ImagArrayElem (Input, k) ->
	unparse_index imag_input_name imag_input_stride k
    | RealArrayElem (Output, k) ->
	unparse_index output_name output_stride k
    | ImagArrayElem (Output, _) ->
	failwith "trying to access imaginary part of real output"
    | x -> default_unparser x

let make_hc2hc_forward_unparser n
    (first_name, first_stride)
    (second_name, second_stride)
    (twiddle_name, twiddle_stride) = function
    | RealArrayElem (Input, k) ->
	unparse_index first_name first_stride k
    | ImagArrayElem (Input, k) ->
	unparse_index second_name second_stride (k - n + 1)
    | RealArrayElem (Output, k) ->
	unparse_index first_name first_stride k
    | ImagArrayElem (Output, k) ->
	unparse_index second_name second_stride (- k)
    | RealArrayElem (Twiddle, k) ->
	"c_re(" ^ (unparse_index twiddle_name twiddle_stride k) ^ ")"
    | ImagArrayElem (Twiddle, k) ->
	"c_im(" ^ (unparse_index twiddle_name twiddle_stride k) ^ ")"
    | x -> default_unparser x

let make_hc2hc_backward_unparser n
    (first_name, first_stride)
    (second_name, second_stride)
    (twiddle_name, twiddle_stride) = function
    | RealArrayElem (Input, k) ->
	unparse_index first_name first_stride k
    | ImagArrayElem (Input, k) ->
	unparse_index second_name second_stride (- k)
    | RealArrayElem (Output, k) ->
	unparse_index first_name first_stride k
    | ImagArrayElem (Output, k) ->
	unparse_index second_name second_stride (k - n + 1)
    | RealArrayElem (Twiddle, k) ->
	"c_re(" ^ (unparse_index twiddle_name twiddle_stride k) ^ ")"
    | ImagArrayElem (Twiddle, k) ->
	"c_im(" ^ (unparse_index twiddle_name twiddle_stride k) ^ ")"
    | x -> default_unparser x

let make_realeven_unparser (input_name, input_stride)
    (real_output_name, real_output_stride) = function
    | RealArrayElem (Input, k) ->
	unparse_index input_name input_stride k
    | ImagArrayElem (Input, _) ->
	failwith "trying to access imaginary part of real input"
    | RealArrayElem (Output, k) ->
	unparse_index real_output_name real_output_stride k
    | ImagArrayElem (Output, k) ->
	failwith "trying to access imaginary part of real output"
    | x -> default_unparser x

let make_realodd_unparser (input_name, input_stride)
    (imag_output_name, imag_output_stride) = function
    | RealArrayElem (Input, k) ->
	unparse_index input_name input_stride (k - 1)
    | ImagArrayElem (Input, _) ->
	failwith "trying to access imaginary part of real input"
    | RealArrayElem (Output, k) ->
	failwith "trying to access real part of imaginary output"
    | ImagArrayElem (Output, k) ->
	unparse_index imag_output_name imag_output_stride (k - 1)
    | x -> default_unparser x

let make_realodd2_unparser (input_name, input_stride)
    (imag_output_name, imag_output_stride) = function
    | RealArrayElem (Input, k) ->
	unparse_index input_name input_stride k
    | ImagArrayElem (Input, _) ->
	failwith "trying to access imaginary part of real input"
    | RealArrayElem (Output, k) ->
	failwith "trying to access real part of imaginary output"
    | ImagArrayElem (Output, k) ->
	unparse_index imag_output_name imag_output_stride (k - 1)
    | x -> default_unparser x

let make_mp3dct_unparser (input_name, input_stride)
    (real_output_name, real_output_stride) = function
    | RealArrayElem (Input, k) ->
	unparse_index input_name input_stride k
    | ImagArrayElem (Input, _) ->
	failwith "trying to access imaginary part of real input"
    | RealArrayElem (Output, k) ->
	unparse_index real_output_name real_output_stride k
    | ImagArrayElem (Output, k) ->
	failwith "trying to access imaginary part of real output"
    | x -> default_unparser x
