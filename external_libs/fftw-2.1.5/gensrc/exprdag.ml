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

(* $Id: exprdag.ml,v 1.43 2003/03/16 23:43:46 stevenj Exp $ *)
let cvsid = "$Id: exprdag.ml,v 1.43 2003/03/16 23:43:46 stevenj Exp $"

open Util

type node =
  | Num of Number.number
  | Load of Variable.variable
  | Store of Variable.variable * node
  | Plus of node list
  | Times of node * node
  | Uminus of node

(* a dag is represented by the list of its roots *)
type dag = Dag of (node list)

module Hash = struct
  (* various hash functions *)
  let hash_float x = 
    let (mantissa, exponent) = frexp x
    in truncate (mantissa *. 10000.0)

  let hash_variable = Variable.hash 

  let rec hash_node = function
      Num x -> hash_float (Number.to_float x)
    | Load v -> 1 + 1237 * hash_variable v
    | Store (v, x) -> 2 * hash_variable v - 2345 * hash_node x
    | Plus l -> 5 + 23451 * sum_list (List.map Hashtbl.hash l)
    | Times (a, b) -> 31415 * Hashtbl.hash a + 2718 * Hashtbl.hash b
    | Uminus x -> 42 + 12345 * (hash_node x)
end

open Hash

module LittleSimplifier = struct 
  (* 
   * The LittleSimplifier module implements a subset of the simplifications
   * of the AlgSimp module.  These simplifications can be executed
   * quickly here, while they would take a long time using the heavy
   * machinery of AlgSimp.  
   * 
   * For example, 0 * x is simplified to 0 tout court by the LittleSimplifier.
   * On the other hand, AlgSimp would first simplify x, generating lots
   * of common subexpressions, storing them in a table etc, just to
   * discard all the work later.  Similarly, the LittleSimplifier
   * reduces the constant FFT in Rader's algorithm to a constant sequence.
   *)
  let rec makeNum = function
    | n -> Num n

  and makeUminus = function
    | Uminus a -> a 
    | Num a -> makeNum (Number.negate a)
    | a -> Uminus a

  and makeTimes = function
    | (Num a, Num b) -> makeNum (Number.mul a b)
    | (Num a, Times (Num b, c)) -> makeTimes (makeNum (Number.mul a b), c)
    | (Num a, b) when Number.is_zero a -> makeNum (Number.zero)
    | (Num a, b) when Number.is_one a -> b
    | (Num a, b) when Number.is_mone a -> makeUminus b
    | (Num a, Uminus b) -> Times (makeUminus (Num a), b)
    | (a, (Num b as b')) -> makeTimes (b', a)
    | (a, b) -> Times (a, b)

  and makePlus l = 
    let rec reduceSum x = match x with
	[] -> []
      | [Num a] -> if Number.is_zero a then [] else x
      | (Num a) :: (Num b) :: c -> 
	  reduceSum ((makeNum (Number.add a b)) :: c)
      | ((Num _) as a') :: b :: c -> b :: reduceSum (a' :: c)
      | a :: s -> a :: reduceSum s
    in match reduceSum l with
      [] -> makeNum (Number.zero)
    | [a] -> a 
    | [a; b] when a == b -> makeTimes (Num Number.two, a)
    | [Times (Num a, b); Times (Num c, d)] when b == d ->
 	makeTimes (makePlus [Num a; Num c], b)
    | a -> Plus a
end

(*************************************************************
 *    Functional associative table
 *************************************************************)
(* 
 * this module implements a functional associative table.  
 * The table is parametrized by an equality predicate and
 * a hash function, with the restriction that (equal a b) ==>
 * hash a == hash b.
 * The table is purely functional and implemented using a binary
 * search tree (not balanced for now)
 *)
module AssocTable : sig
  type ('a, 'b) elem =
    | Leaf
    | Node of int * ('a, 'b) elem * ('a, 'b) elem * ('a * 'b) list
  val empty : ('a, 'b) elem
  val lookup :
      ('a -> int) -> ('a -> 'b -> bool) -> 'a -> ('b, 'c) elem -> 'c option
  val insert :
      ('a -> int) -> 'a -> 'c -> ('a, 'c) elem -> ('a, 'c) elem
end = struct
  type ('a, 'b) elem = 
      Leaf 
    | Node of int * ('a, 'b) elem * ('a, 'b) elem * ('a * 'b) list

  let empty = Leaf

  let lookup hash equal key table =
    let h = hash key in
    let rec look = function
	Leaf -> None
      |	Node (hash_key, left, right, this_list) ->
	  if (hash_key < h) then look left
	  else if (hash_key > h) then look right
	  else let rec loop = function
	      [] -> None
	    | (a, b) :: rest -> if (equal key a) then Some b else loop rest
	  in loop this_list
    in look table

  let insert hash key value table =
    let h = hash key in
    let rec ins = function
	Leaf -> Node (h, Leaf, Leaf, [(key, value)])
      |	Node (hash_key, left, right, this_list) ->
	  if (hash_key < h) then 
	    Node (hash_key, ins left, right, this_list)
	  else if (hash_key > h) then 
	    Node (hash_key, left, ins right, this_list)
	  else 
	    Node (hash_key, left, right, (key, value) :: this_list)
    in ins table
end

let node_insert =  AssocTable.insert hash_node
let node_lookup =  AssocTable.lookup hash_node (==)

(*************************************************************
 *   Monads
 *************************************************************)
(*
 * Phil Wadler has many well written papers about monads.  See
 * http://cm.bell-labs.com/cm/cs/who/wadler/ 
 *)
(* vanilla state monad *)
module StateMonad = struct
  let returnM x = fun s -> (x, s)

  let (>>=) = fun m k -> 
    fun s ->
      let (a', s') = m s
      in let (a'', s'') = k a' s'
      in (a'', s'')

  let (>>) = fun m k ->
    m >>= fun _ -> k

  let rec mapM f = function
      [] -> returnM []
    | a :: b ->
	f a >>= fun a' ->
	  mapM f b >>= fun b' ->
	    returnM (a' :: b')

  let runM m x initial_state =
    let (a, _) = m x initial_state
    in a

  let fetchState =
    fun s -> s, s

  let storeState newState =
    fun _ -> (), newState
end

(* monad with built-in memoizing capabilities *)
module MemoMonad =
  struct
    open StateMonad

    let memoizing lookupM insertM f k =
      lookupM k >>= fun vMaybe ->
	match vMaybe with
	  Some value -> returnM value
	| None ->
	    f k >>= fun value ->
	      insertM k value >> returnM value

    let runM initial_state m x  = StateMonad.runM m x initial_state
end

module Oracle : sig
  val should_flip_sign : node -> bool
end  = struct
  open AssocTable
  let make_memoizer hash equal =
    let table = ref empty in fun f k ->
      match lookup hash equal k !table with
	Some value -> value
      | None ->
	  let value = f k in
	  begin	
	    table := insert hash k value !table;
	    value
	  end

  let almost_equal x y = 
    let epsilon = 1.0E-8 in
    (abs_float (x -. y) < epsilon) or
    (abs_float (x -. y) < epsilon *. (abs_float x +. abs_float y)) 
  let memoizing_numbers = make_memoizer 
      (fun x -> hash_float (abs_float x))
      (fun a b -> almost_equal a b or almost_equal (-. a) b)
  let absid = memoizing_numbers (fun x -> x)

  let memoizing_variables = make_memoizer hash_variable Variable.same

  let memoizing_nodes = make_memoizer hash_node (==)

  let random_oracle =
    memoizing_variables
      (fun _ -> (float (Random.bits())) /. 1073741824.0)


  let sum_list l = List.fold_right (+.) l 0.0

  let rec eval x =
    memoizing_nodes (function
      	Num x -> Number.to_float x
      | Load v -> random_oracle v
      | Store (v, x) -> random_oracle v
      | Plus l -> sum_list (List.map eval l)
      | Times (a, b) -> (eval a) *. (eval b)
      | Uminus x -> -. (eval x) )
      x

  let should_flip_sign node = 
    let v = eval node in
    let v' = absid v in
    not (almost_equal v v')
end

module Reverse = struct
  open StateMonad
  open MemoMonad
  open AssocTable
  open LittleSimplifier

  let fetchDuals = fetchState
  let storeDuals = storeState

  let lookupDualsM key =
    fetchDuals >>= fun table ->
      returnM (node_lookup key table)

  let insertDualsM key value =
    fetchDuals >>= fun table ->
      storeDuals (node_insert key value table)

  let rec visit visited vtable parent_table = function
      [] -> (visited, parent_table)
    | node :: rest ->
	match AssocTable.lookup hash_node (==) node vtable with
	  Some _ -> visit visited vtable parent_table rest
	| None ->
	    let children = match node with
	      Store (v, n) -> [n]
	    | Plus l -> l
	    | Times (a, b) -> [a; b]
	    | Uminus x -> [x]
	    | _ -> []
	    in let rec loop t = function
		[] -> t
	      |	a :: rest ->
		  (match AssocTable.lookup hash_node (==) a t with
		    None -> 
		      loop 
			(AssocTable.insert hash_node a [node] t)
			rest
		  | Some c ->
		      loop 
			(AssocTable.insert hash_node a (node :: c) t)
			rest)
	    in visit 
	      (node :: visited)
	      (AssocTable.insert hash_node node () vtable)
	      (loop parent_table children)
	      (children @ rest)

  let make_reverser parent_table =
    let rec termM node candidate_parent = 
      match candidate_parent with
	Store (_, n) when n == node -> 
	  dualM candidate_parent >>= fun x' -> returnM [x']
      | Plus (l) when List.memq node l -> 
	  dualM candidate_parent >>= fun x' -> returnM [x']
      | Times (a, b) when b == node -> 
	  dualM candidate_parent >>= fun x' -> 
	    returnM [makeTimes (a, x')]
      | Uminus n when n == node -> 
	  dualM candidate_parent >>= fun x' -> 
	    returnM [makeUminus x']
      | _ -> returnM []
    
    and dualExpressionM this_node = 
      mapM (termM this_node) 
	(match AssocTable.lookup hash_node (==) this_node parent_table with
	  Some a -> a
	| None -> failwith "bug in dualExpressionM"
	) >>= fun l ->
	returnM (makePlus (List.flatten l))

    and dualM this_node =
      memoizing lookupDualsM insertDualsM
	(function
	    Load v as x -> 
	      if (Variable.is_twiddle v) then
		returnM (Load v)
	      else
		(dualExpressionM x >>= fun d ->
		  returnM (Store (v, d)))
	  | Store (v, x) -> returnM (Load v)
	  | x -> dualExpressionM x)
	this_node

    in dualM

  let is_store = function 
      Store _ -> true
    | _ -> false

  let reverse (Dag dag) = 
    let (all_nodes, parent_table) = visit [] empty empty dag in
    let reverserM = make_reverser parent_table in
    let mapReverserM = mapM reverserM in
    let duals = runM empty mapReverserM all_nodes in
    let roots = filter is_store duals
    in Dag roots
end

(*************************************************************
 * Various dag statistics
 *************************************************************)
module Stats : sig
  type complexity
  val complexity : dag -> complexity
  val same_complexity : complexity -> complexity -> bool
end = struct
  type complexity = int * int * int * int * int * int
  let rec visit visited vtable = function
      [] -> visited
    | node :: rest ->
	match AssocTable.lookup hash_node (==) node vtable with
	  Some _ -> visit visited vtable rest
	| None ->
	    let children = match node with
	      Store (v, n) -> [n]
	    | Plus l -> l
	    | Times (a, b) -> [a; b]
	    | Uminus x -> [x]
	    | _ -> []
	    in visit (node :: visited)
	      (AssocTable.insert hash_node node () vtable)
	      (children @ rest)

  let complexity (Dag dag) = 
    let rec loop (load, store, plus, times, uminus, num) = function 
      	[] -> (load, store, plus, times, uminus, num)
      | node :: rest ->
	  loop
	    (match node with
	      Load _ -> (load + 1, store, plus, times, uminus, num)
	    | Store _ -> (load, store + 1, plus, times, uminus, num)
	    | Plus _ -> (load, store, plus + 1, times, uminus, num)
	    | Times _ -> (load, store, plus, times + 1, uminus, num)
	    | Uminus _ -> (load, store, plus, times, uminus + 1, num)
	    | Num _ -> (load, store, plus, times, uminus, num + 1))
	    rest
    in let (l, s, p, t, u, n) = 
      loop (0, 0, 0, 0, 0, 0) (visit [] AssocTable.empty dag)
    in (l, s, p, t, u, n)

  let same_complexity a b = (a = b)
end    
  

(*************************************************************
 * Algebraic simplifier/elimination of common subexpressions
 *************************************************************)
module AlgSimp : sig 
  val algsimp : dag -> dag
end = struct

  open StateMonad
  open MemoMonad
  open AssocTable

  let fetchSimp = 
    fetchState >>= fun (s, _) -> returnM s
  let storeSimp s =
    fetchState >>= (fun (_, c) -> storeState (s, c))
  let lookupSimpM key =
    fetchSimp >>= fun table ->
      returnM (node_lookup key table)
  let insertSimpM key value =
    fetchSimp >>= fun table ->
      storeSimp (node_insert key value table)


  let subset a b =
    List.for_all (fun x -> List.exists (fun y -> x == y) b) a

  let equalCSE a b = 
    match (a, b) with
      (Num a, Num b) -> Number.equal a b
    | (Load a, Load b) -> 
	Variable.same a b &&
	(!Magic.collect_common_twiddle or not (Variable.is_twiddle a)) &&
	(!Magic.collect_common_inputs or not (Variable.is_input a))
    | (Times (a, a'), Times (b, b')) ->
	((a == b) && (a' == b')) or
	((a == b') && (a' == b))
    | (Plus a, Plus b) -> subset a b && subset b a
    | (Uminus a, Uminus b) -> (a == b)
    | _ -> false

  let fetchCSE = 
    fetchState >>= fun (_, c) -> returnM c
  let storeCSE c =
    fetchState >>= (fun (s, _) -> storeState (s, c))
  let lookupCSEM key =
    fetchCSE >>= fun table ->
      returnM (AssocTable.lookup hash_node equalCSE key table)
  let insertCSEM key value =
    fetchCSE >>= fun table ->
      storeCSE (AssocTable.insert hash_node key value table)

  (* memoize both x and Uminus x (unless x is already negated) *) 
  let identityM x =
    let memo x = memoizing lookupCSEM insertCSEM returnM x in
    match x with
	Uminus _ -> memo x 
      |	_ -> memo x >>= fun x' -> memo (Uminus x') >> returnM x'

  let makeNode = identityM

  (* simplifiers for various kinds of nodes *)
  let rec snumM = function
      n when Number.is_zero n -> 
	makeNode (Num (Number.zero))
    | n when Number.negative n -> 
	makeNode (Num (Number.negate n)) >>= suminusM
    | n -> makeNode (Num n)

  and suminusM = function
      Uminus x -> makeNode x
    | Num a when (Number.is_zero a) -> snumM Number.zero
    | a -> makeNode (Uminus a)

  and stimesM = function 
    | (Uminus a, b) -> stimesM (a, b) >>= suminusM
    | (a, Uminus b) -> stimesM (a, b) >>= suminusM
    | (Num a, Num b) -> snumM (Number.mul a b)
    | (Num a, Times (Num b, c)) -> 
	snumM (Number.mul a b) >>= fun x -> stimesM (x, c)
    | (Num a, b) when Number.is_zero a -> snumM Number.zero
    | (Num a, b) when Number.is_one a -> makeNode b
    | (Num a, b) when Number.is_mone a -> suminusM b
    | (a, (Num _ as b')) -> stimesM (b', a)
    | (a, b) -> makeNode (Times (a, b))

  and reduce_sumM x = match x with
    [] -> returnM []
  | [Num a] -> 
      if (Number.is_zero a) then 
	returnM [] 
      else returnM x
  | [Uminus (Num a)] -> 
      if (Number.is_zero a) then 
	returnM [] 
      else returnM x
  | (Num a) :: (Num b) :: s -> 
      snumM (Number.add a b) >>= fun x ->
	reduce_sumM (x :: s)
  | (Num a) :: (Uminus (Num b)) :: s -> 
      snumM (Number.sub a b) >>= fun x ->
	reduce_sumM (x :: s)
  | (Uminus (Num a)) :: (Num b) :: s -> 
      snumM (Number.sub b a) >>= fun x ->
	reduce_sumM (x :: s)
  | (Uminus (Num a)) :: (Uminus (Num b)) :: s -> 
      snumM (Number.add a b) >>= 
      suminusM >>= fun x ->
	reduce_sumM (x :: s)
  | ((Num _) as a) :: b :: s -> reduce_sumM (b :: a :: s)
  | ((Uminus (Num _)) as a) :: b :: s -> reduce_sumM (b :: a :: s)
  | a :: s -> 
      reduce_sumM s >>= fun s' -> returnM (a :: s')

  (* collectCoeffM transforms
   *       n x + n y   =>  n (x + y)
   * where n is a number *)
  and collectCoeffM x = 
    let rec filterM coeff = function
	Times (Num a, b) as y :: rest ->
	  filterM coeff rest >>= fun (w, wo) ->
	    if (Number.equal a coeff) then
	      returnM (b :: w, wo)
	    else
	      returnM (w, y :: wo)
      | Uminus (Times (Num a, b)) as y :: rest ->
	  filterM coeff rest >>= fun (w, wo) ->
	    if (Number.equal a coeff) then
	      suminusM b >>= fun b' ->
		returnM (b' :: w, wo)
	    else
	      returnM (w, y :: wo)
      | y :: rest -> 
	  filterM coeff rest >>= fun (w, wo) ->
	    returnM (w, y :: wo)
      |	[] -> returnM ([], [])

    and foundCoeffM a x =
      filterM a x >>= fun (w, wo) ->
	collectCoeffM wo >>= fun wo' ->
	  (match w with 
	    [d] -> makeNode d 
	  | _ -> splusM w) >>= fun p ->
	      snumM a >>= fun a' ->
		stimesM (a', p) >>= fun ap ->
		  returnM (ap :: wo')

    in match x with
      [] -> returnM []
    | Times (Num a, _) :: _ -> foundCoeffM a x
    | (Uminus (Times (Num a, b))) :: _  -> foundCoeffM a x
    | (a :: c) ->  
	collectCoeffM c >>= fun c' ->
	  returnM (a :: c')

  (* transform   n1 * x + n2 * x ==> (n1 + n2) * x *)
  and collectExprM x = 
    let rec findCoeffM = function
	Times (Num a as a', b) -> returnM (a', b)
      | Uminus (Times (Num a as a', b)) -> 
	  suminusM a' >>= fun a'' ->
	    returnM (a'', b)
      | Uminus x -> 
	  snumM Number.one >>= suminusM >>= fun mone ->
	    returnM (mone, x)
      | x -> 
	  snumM Number.one >>= fun one ->
	    returnM (one, x)
    and filterM xpr = function
	[] -> returnM ([], [])
      |	a :: b ->
	  filterM xpr b >>= fun (w, wo) ->
	    findCoeffM a >>= fun (c, x) ->
	      if (xpr == x) then
		returnM (c :: w, wo)
	      else
		returnM (w, a :: wo)
    in match x with
      [] -> returnM x
    | [a] -> returnM x
    | a :: b ->
	findCoeffM a >>= fun (_, xpr) ->
	  filterM xpr x >>= fun (w, wo) ->
	    collectExprM wo >>= fun wo' ->
	      splusM w >>= fun w' ->
		stimesM (w', xpr) >>= fun t' ->
		  returnM (t':: wo')

  and mangleSumM x = returnM x
      >>= reduce_sumM 
      >>= collectExprM 
      >>= collectCoeffM 
      >>= reduce_sumM 
      >>= eliminateButterflyishPatternsM
      >>= reduce_sumM

  and reorder_uminus = function  (* push all Uminuses to the end *)
      [] -> []
    | ((Uminus a) as a' :: b) -> (reorder_uminus b) @ [a']
    | (a :: b) -> a :: (reorder_uminus b)                      

  and canonicalizeM = function 
      [] -> snumM Number.zero
    | [a] -> makeNode a                    (* one term *)
    |	a -> makeNode (Plus (reorder_uminus a)) >>= generateFusedMultAddM

  and negative = function
      Uminus _ -> true
    | _ -> false

  (*
   * simplify patterns of the form
   *
   *  (c_1 * a + ...) +  (c_2 * a + ...)
   *
   * The pattern includes arbitrary coefficients and minus signs.
   * A common case of this pattern is the butterfly
   *   (a + b) + (a - b)
   *   (a + b) - (a - b)
   *)
  and eliminateButterflyishPatternsM l =
    let rec findTerms depth x = match x with
      | Uminus x -> findTerms depth x
      |	Times (Num a, b) -> findTerms (depth - 1) b
      |	Plus l when depth > 0 ->
	  x :: List.flatten (List.map (findTerms (depth - 1)) l)
      |	x -> [x]
    and duplicates = function
	[] -> []
      |	a :: b -> if List.memq a b then a :: duplicates b
      else duplicates b
    in let rec flattenPlusM d coef x =
      if (List.memq x d) then
 	snumM coef >>= fun coef' ->
	  stimesM (coef', x) >>= fun x' -> returnM [x']
      else match x with
      |	Times (Num a, b) ->
	  flattenPlusM d (Number.mul a coef) b
      | Uminus x -> 
	  flattenPlusM d (Number.negate coef) x
      |	Plus l -> 
	  snumM coef >>= fun coef' ->
	    mapM (fun x -> stimesM (coef', x)) l 
      |	x -> snumM coef >>= fun coef' ->
	  stimesM (coef', x) >>= fun x' -> returnM [x']
    in let l' = List.flatten (List.map (findTerms 1) l)
    in let d = duplicates l'
    in if (List.length d) > 0 then
      mapM (flattenPlusM d Number.one) l >>= fun a ->
	collectExprM (List.flatten a) >>=
	mangleSumM
    else
      returnM l

  and splusM l = mangleSumM l >>=  fun l' ->
  (* no terms are negative.  Don't do anything *)
  if not (List.exists negative l') then
    canonicalizeM l'
  (* all terms are negative.  Negate all of them and collect the minus sign *)
  else if List.for_all negative l' then
    mapM suminusM l' >>= splusM >>= suminusM
  (* some terms are positive and some are negative.  We are in trouble.
     Ask the Oracle *)
  else if Oracle.should_flip_sign (Plus l') then
    mapM suminusM l' >>= splusM >>= suminusM
  else
    canonicalizeM l'

  and generateFusedMultAddM = 
    let rec is_multiplication = function
      | Times (Num a, b) -> true
      | Uminus (Times (Num a, b)) -> true
      | _ -> false
    and separate = function
	[] -> ([], [], Number.zero)
      | (Times (Num a, b)) as this :: c -> 
	  let (x, y, max) = separate c in
	  let newmax = if (Number.greater a max) then a else max in
	  (this :: x, y, newmax)
      | (Uminus (Times (Num a, b))) as this :: c -> 
	  let (x, y, max) = separate c in
	  let newmax = if (Number.greater a max) then a else max in
	  (this :: x, y, newmax)
      | this :: c ->
	  let (x, y, max) = separate c in
	  (x, this :: y, max)
    in function
	Plus l when (count is_multiplication l >= 2) && !Magic.enable_fma ->
	  let (w, wo, max) = separate l in
	  snumM (Number.div Number.one max) >>= fun invmax' ->
	    snumM max >>= fun max' ->
	      mapM (fun x -> stimesM (invmax', x)) w >>= splusM >>= fun pw' ->
		stimesM (max', pw') >>= fun mw' ->
		  splusM (wo @ [mw'])
      | x -> returnM x
  (* monadic style algebraic simplifier for the dag *)
  let rec algsimpM x =
    memoizing lookupSimpM insertSimpM 
      (function 
 	  Num a -> snumM a
 	| Plus a -> 
 	    mapM algsimpM a >>= splusM
 	| Times (a, b) -> 
 	    algsimpM a >>= fun a' ->
 	      algsimpM b >>= fun b' ->
 		stimesM (a', b')
 	| Uminus a -> 
 	    algsimpM a >>= suminusM 
 	| Store (v, a) ->
 	    algsimpM a >>= fun a' ->
 	      makeNode (Store (v, a'))
 	| x -> makeNode x)
      x

   let initialTable = (empty, empty)
   let simp_roots = mapM algsimpM
   let algsimp (Dag dag) = Dag (runM initialTable simp_roots dag)
end

(* simplify the dag *)
let rec simplification_loop v =
  let complexity = Stats.complexity v in
  let v = Reverse.reverse v in
  let v = AlgSimp.algsimp v in
  let v = Reverse.reverse v in
  let v = AlgSimp.algsimp v in
  let complexity' = Stats.complexity v in
  if (Stats.same_complexity complexity complexity') then
    v
  else
    simplification_loop v

let algsimp v = 
  let v = AlgSimp.algsimp v in
  simplification_loop v

let make nodes = Dag nodes

(*************************************************************
 * Conversion of the dag to an assignment list
 *************************************************************)
(*
 * This function is messy.  The main problem is that we want to
 * inline dag nodes conditionally, depending on how many times they
 * are used.  The Right Thing to do would be to modify the
 * state monad to propagate some of the state backwards, so that
 * we know whether a given node will be used again in the future.
 * This modification is trivial in a lazy language, but it is
 * messy in a strict language like ML.  
 *
 * In this implementation, we just do the obvious thing, i.e., visit
 * the dag twice, the first to count the node usages, and the second to
 * produce the output.
 *)
module Destructor :  sig
  val to_assignments : dag -> (Variable.variable * Expr.expr) list
end = struct

  open StateMonad
  open MemoMonad
  open AssocTable

  let fresh = Variable.make_temporary

  let fetchAl = 
    fetchState >>= (fun (al, _, _) -> returnM al)

  let storeAl al =
    fetchState >>= (fun (_, visited, visited') ->
      storeState (al, visited, visited'))

  let fetchVisited = fetchState >>= (fun (_, v, _) -> returnM v)

  let storeVisited visited =
    fetchState >>= (fun (al, _, visited') ->
      storeState (al, visited, visited'))

  let fetchVisited' = fetchState >>= (fun (_, _, v') -> returnM v')
  let storeVisited' visited' =
    fetchState >>= (fun (al, visited, _) ->
      storeState (al, visited, visited'))
  let lookupVisitedM' key =
    fetchVisited' >>= fun table ->
      returnM (AssocTable.lookup hash_node (==) key table)
  let insertVisitedM' key value =
    fetchVisited' >>= fun table ->
      storeVisited' (AssocTable.insert hash_node key value table)

  let counting f x =
    fetchVisited >>= (fun v ->
      match AssocTable.lookup hash_node (==) x v with
	Some count -> 
	  fetchVisited >>= (fun v' ->
	    storeVisited (AssocTable.insert hash_node 
			    x (count + 1) v'))
      |	None ->
	  f x >>= fun () ->
	    fetchVisited >>= (fun v' ->
	      storeVisited (AssocTable.insert hash_node 
			      x 1 v')))

  let with_varM v x = 
    fetchAl >>= (fun al -> storeAl ((v, x) :: al)) >> returnM (Expr.Var v)

  let inlineM = returnM

  let with_tempM x = with_varM (fresh ()) x

  (* declare a temporary only if node is used more than once *)
  let with_temp_maybeM node x =
    fetchVisited >>= (fun v ->
      match AssocTable.lookup hash_node (==) node v with
	Some count -> 
	  if (count = 1 && !Magic.inline_single) then
	    inlineM x
	  else
	    with_tempM x
      |	None ->
	  failwith "with_temp_maybeM")

  type fma = 
      NO_FMA
    | FMA of node * node * node   (* FMA (a, b, c) => a + b * c *)
    | FMS of node * node * node   (* FMS (a, b, c) => -a + b * c *)
    | FNMS of node * node * node  (* FNMS (a, b, c) => a - b * c *)

  let build_fma l = 
    if (not !Magic.enable_fma_expansion) then NO_FMA
    else match l with
    | [Uminus a; Times (b, c)] -> FMS (a, b, c)
    | [Times (b, c); Uminus a] -> FMS (a, b, c)
    | [a; Uminus (Times (b, c))] -> FNMS (a, b, c)
    | [Uminus (Times (b, c)); a] -> FNMS (a, b, c)
    | [a; Times (b, c)] -> FMA (a, b, c)
    | [Times (b, c); a] -> FMA (a, b, c)
    | _ -> NO_FMA

  let children_fma l = match build_fma l with
    FMA (a, b, c) -> Some (a, b, c)
  | FMS (a, b, c) -> Some (a, b, c)
  | FNMS (a, b, c) -> Some (a, b, c)
  | NO_FMA -> None

  let rec visitM x =
    counting (function
	Load v -> returnM ()
      |	Num a -> returnM ()
      |	Store (v, x) -> visitM x
      |	Plus a -> (match children_fma a with
	  None -> mapM visitM a >> returnM ()
	| Some (a, b, c) -> 
          (* visit fma's arguments twice to make sure they get a variable *)
	    visitM a >> visitM a >>
	    visitM b >> visitM b >>
	    visitM c >> visitM c)
      |	Times (a, b) ->
	  visitM a >> visitM b
      |	Uminus a ->  visitM a)
      x

  let visit_rootsM = mapM visitM

  let rec expr_of_nodeM x =
    memoizing lookupVisitedM' insertVisitedM'
      (function x -> match x with
	Load v -> 
	  if (!Magic.inline_loads) then
	    inlineM (Expr.Var v)
	  else
	    with_tempM (Expr.Var v)
      | Num a ->
	  inlineM (Expr.Num a)
      | Store (v, x) -> 
	  expr_of_nodeM x >>= 
	  with_varM v 
      | Plus a -> (match build_fma a with
	  FMA (a, b, c) ->	  
	    expr_of_nodeM a >>= fun a' ->
	      expr_of_nodeM b >>= fun b' ->
		expr_of_nodeM c >>= fun c' ->
		  with_temp_maybeM x (Expr.Plus [a'; Expr.Times (b', c')])
	| FMS (a, b, c) ->	  
	    expr_of_nodeM a >>= fun a' ->
	      expr_of_nodeM b >>= fun b' ->
		expr_of_nodeM c >>= fun c' ->
		  with_temp_maybeM x 
		    (Expr.Plus [Expr.Times (b', c'); Expr.Uminus a'])
	| FNMS (a, b, c) ->	  
	    expr_of_nodeM a >>= fun a' ->
	      expr_of_nodeM b >>= fun b' ->
		expr_of_nodeM c >>= fun c' ->
		  with_temp_maybeM x 
		    (Expr.Plus [a'; Expr.Uminus (Expr.Times (b', c'))])
	| NO_FMA ->
	    mapM expr_of_nodeM a >>= fun a' ->
	      with_temp_maybeM x (Expr.Plus a'))
      | Times (a, b) ->
	  expr_of_nodeM a >>= fun a' ->
	    expr_of_nodeM b >>= fun b' ->
	      with_temp_maybeM x (Expr.Times (a', b'))
      | Uminus a ->
	  expr_of_nodeM a >>= fun a' ->
	    inlineM (Expr.Uminus a'))
      x

  let expr_of_rootsM = mapM expr_of_nodeM

  let peek_alistM roots =
    visit_rootsM roots >> expr_of_rootsM roots >> fetchAl

  let to_assignments (Dag dag) =
    List.rev (runM ([], empty, empty) peek_alistM dag)

end


let to_assignments = Destructor.to_assignments

let wrap_assign (a, b) = Expr.Assign (a, b)
let simplify_to_alist dag = 
  let d1 = algsimp dag
  in List.map wrap_assign (to_assignments d1)



