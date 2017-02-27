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

(* $Id: util.ml,v 1.14 2003/03/16 23:43:46 stevenj Exp $ *)

(* various utility functions *)
open List
open Unix 

(*****************************************
 * Integer operations
 *****************************************)
(* fint the inverse of n modulo m *)
let invmod n m =
    let rec loop i =
	if ((i * n) mod m == 1) then i
	else loop (i + 1)
    in
	loop 1

(* Yooklid's algorithm *)
let rec gcd n m =
    if (n > m)
      then gcd m n
    else
      let r = m mod n
      in
	  if (r == 0) then n
	  else gcd r n

(* reduce the fraction m/n to lowest terms, modulo factors of n/n *)
let lowest_terms n m =
    if (m mod n == 0) then
      (1,0)
    else
      let nn = (abs n) in let mm = m * (n / nn)
      in let mpos = 
	  if (mm > 0) then (mm mod nn)
	  else (mm + (1 + (abs mm) / nn) * nn) mod nn
      and d = gcd nn (abs mm)
      in (nn / d, mpos / d)

(* find a generator for the multiplicative group mod p
   (where p must be prime for a generator to exist!!) *)

exception No_Generator

let find_generator p =
    let rec period x prod =
 	if (prod == 1) then 1
	else 1 + (period x (prod * x mod p))
    in let rec findgen x =
	if (x == 0) then raise No_Generator
	else if ((period x x) == (p - 1)) then x
	else findgen ((x + 1) mod p)
    in findgen 1

(* raise x to a power n modulo p (requires n > 0) (in principle,
   negative powers would be fine, provided that x and p are relatively
   prime...we don't need this functionality, though) *)

exception Negative_Power

let rec pow_mod x n p =
    if (n == 0) then 1
    else if (n < 0) then raise Negative_Power
    else if (n mod 2 == 0) then pow_mod (x * x mod p) (n / 2) p
    else x * (pow_mod x (n - 1) p) mod p

(******************************************
 * auxiliary functions 
 ******************************************)
let rec forall combiner a b f =
    if (a >= b) then []
    else combiner (f a) (forall combiner (a + 1) b f)

let sum_list l = fold_right (+) l 0
let max_list l = fold_right (max) l (-999999)
let min_list l = fold_right (min) l 999999
let count pred = fold_left 
    (fun a elem -> if (pred elem) then 1 + a else a) 0
let filter pred l = fold_right 
    (fun elem a -> if (pred elem) then elem :: a else a) l []
let remove elem = filter (fun e -> (e != elem))
let cons a b = a :: b
let null = function 
    [] -> true
  | _ -> false

(* functional composition *)
let (@@) f g x = f (g x)

(* Hmm... CAML won't allow second-order polymorphism.  Oh well.. *)
(* let forall_flat = forall (@);; *)
let rec forall_flat a b f = 
    if (a >= b) then []
    else (f a) @ (forall_flat (a + 1) b f)

let identity x = x

let for_list l f = 
  let rec loop = function
      [] -> ()
    | s::r -> begin f s; loop r; end
  in loop l

let rec minimize f = function
    [] -> None
  | elem :: rest ->
      match minimize f rest with
	None -> Some elem
      |	Some x -> if (f x) >= (f elem) then Some elem else Some x


let rec find_elem condition = function
    [] -> None
  | elem :: rest ->
      if condition elem then
	Some elem
      else
	find_elem condition rest


(* find x, x >= a, such that (p x) is true *)
let rec suchthat a pred =
  if (pred a) then a else suchthat (a + 1) pred

(* print an information message *)
let info string =
  if !Magic.verbose then begin
    let now = Unix.times () 
    and pid = Unix.getpid () in
    prerr_string ((string_of_int pid) ^ ": " ^
		  "at t = " ^  (string_of_float now.tms_utime) ^ " : ");
    prerr_string (string ^ "\n");
    flush Pervasives.stderr;
  end
