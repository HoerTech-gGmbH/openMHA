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

(* $Id: expr.ml,v 1.14 2003/03/16 23:43:46 stevenj Exp $ *)

(* Here, we define the data type encapsulating a symbolic arithmetic
   expression, and provide some routines for manipulating it.  (See
   also simplify.ml for functions to do symbolic simplifications.) *)

type expr =
    Num of Number.number
  | Var of Variable.variable
  | Plus of expr list
  | Times of expr * expr
  | Uminus of expr
  | Integer of int

type assignment = Assign of Variable.variable * expr

let rec find_vars x =
  match x with
  | Var y -> [y]
  | Plus l -> List.flatten (List.map find_vars l)
  | Times (a, b) -> (find_vars a) @ (find_vars b)
  | Uminus a -> find_vars a
  | _ -> []
