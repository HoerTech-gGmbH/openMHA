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

(* $Id: schedule.ml,v 1.17 2003/03/16 23:43:46 stevenj Exp $ *)

(* This file contains the instruction scheduler, which finds an
   efficient ordering for a given list of instructions.

   The scheduler analyzes the DAG (Directed, Acyclic Graph) formed by
   the instruction dependencies, and recursively partitions it.  The
   resulting schedule data structure expresses a "good" ordering
   and structure for the computation.

   The scheduler makes use of utilties in Dag and other packages to
   manipulate the Dag and the instruction list. *)

open Dag
(*************************************************
 *               Dag scheduler
 *************************************************)
let to_assignment node = (Expr.Assign (node.assigned, node.expression))
let makedag l = Dag.makedag 
    (List.map (function Expr.Assign (v, x) -> (v, x)) l)

let return x = x
let has_color c n = (n.color = c)
let set_color c n = (n.color <- c)
let has_either_color c1 c2 n = (n.color = c1 || n.color = c2)

let infinity = 100000 

let cc dag inputs =
  begin
    Dag.for_all dag (fun node -> 
      node.label <- infinity);
    
    (match inputs with 
      a :: _ -> bfs dag a 0
    | _ -> failwith "connected");

    return
      ((List.map to_assignment (Util.filter (fun n -> n.label < infinity)
				  (Dag.to_list dag))),
       (List.map to_assignment (Util.filter (fun n -> n.label == infinity) 
				  (Dag.to_list dag))))
  end

let rec connected_components alist =
  let dag = makedag alist in
  let inputs = 
    Util.filter (fun node -> Util.null node.predecessors) 
      (Dag.to_list dag) in
  match cc dag inputs with
    (a, []) -> [a]
  | (a, b) -> a :: connected_components b

let loads_twiddle node =
  match (node.input_variables, node.predecessors) with
    ([x], []) -> Variable.is_twiddle x
  | _ -> false

let partition alist =
  let dag = makedag alist in
  let dag' = Dag.to_list dag in
  let inputs = 
    Util.filter (fun node -> Util.null node.predecessors) dag'
  and outputs = 
    Util.filter (fun node -> Util.null node.successors) dag'
  and special_inputs =  Util.filter loads_twiddle dag' in
  begin
    Dag.for_all dag (fun node -> 
      begin
      	node.color <- BLACK;
      end);

    Util.for_list inputs (set_color RED);

    (* The special inputs are input that read a twiddle factor.  They
       can end up either in the blue or in the red part.  If a red
       node needs a special input, the special input becomes red.  If
       all successors of a special input are blue, it becomes blue.
       Outputs are always blue.

       As a consequence, however, the final partition might be
       composed only of blue nodes (which is incorrect).  In this case
       we manually reset all inputs (whether special or not) to be red. *)

    Util.for_list special_inputs (set_color YELLOW);

    Util.for_list outputs (set_color BLUE);

    let rec loopi donep = 
      match (Util.filter
	       (fun node -> (has_color BLACK node) &&
		 List.for_all (has_either_color RED YELLOW) node.predecessors)
	       dag') with
	[] -> if (donep) then () else loopo true
      |	i -> 
	  begin
	    Util.for_list i (fun node -> 
	      begin
      		set_color RED node;
		Util.for_list node.predecessors (set_color RED);
	      end);
	    loopo false; 
	  end

    and loopo donep =
      match (Util.filter
	       (fun node -> (has_either_color BLACK YELLOW node) &&
		 List.for_all (has_color BLUE) node.successors)
	       dag') with
	[] -> if (donep) then () else loopi true
      |	o ->
	  begin
	    Util.for_list o (set_color BLUE);
	    loopi false; 
	  end

    (* among the magic parameters, this is the most obscure *)
    in if !Magic.loopo then 
      loopo false
    else
      loopi false;

    (* fix the partition if it is incorrect *)
    if not (List.exists (has_color RED) dag') then 
	Util.for_list inputs (set_color RED);
    
    return
      ((List.map to_assignment (Util.filter (has_color RED) dag')),
       (List.map to_assignment (Util.filter (has_color BLUE) dag')))
  end

type schedule = 
    Done
  | Instr of Expr.assignment
  | Seq of (schedule * schedule)
  | Par of schedule list


let schedule =
  let rec schedule_alist = function
      [] -> Done
    | [a] -> Instr a
    | alist -> 
	match connected_components alist with
	  ([a]) -> schedule_connected a
	| l -> Par (List.map schedule_alist l)

  and schedule_connected alist = 
    match partition alist with
    | (a, b) -> Seq (schedule_alist a, schedule_alist b)

  in schedule_alist
