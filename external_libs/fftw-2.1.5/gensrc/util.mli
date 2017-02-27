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

(* $Id: util.mli,v 1.12 2003/03/16 23:43:46 stevenj Exp $ *)
val invmod : int -> int -> int
val gcd : int -> int -> int
val lowest_terms : int -> int -> int * int
val find_generator : int -> int
val pow_mod : int -> int -> int -> int
val forall :
  ('a -> 'b list -> 'b list) -> int -> int -> (int -> 'a) -> 'b list
val sum_list : int list -> int
val max_list : int list -> int
val min_list : int list -> int
val count : ('a -> bool) -> 'a list -> int
val filter : ('a -> bool) -> 'a list -> 'a list
val remove : 'a -> 'a list -> 'a list
val cons : 'a -> 'a list -> 'a list
val null : 'a list -> bool
val (@@) : ('a -> 'b) -> ('c -> 'a) -> 'c -> 'b
val forall_flat : int -> int -> (int -> 'a list) -> 'a list
val identity : 'a -> 'a
val for_list : 'a list -> ('a -> 'b) -> unit
val minimize : ('a -> 'b) -> 'a list -> 'a option
val find_elem : ('a -> bool) -> 'a list -> 'a option
val suchthat : int -> (int -> bool) -> int
val info : string -> unit
