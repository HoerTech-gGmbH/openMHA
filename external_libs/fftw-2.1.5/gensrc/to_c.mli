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

(* $Id: to_c.mli,v 1.9 2003/03/16 23:43:46 stevenj Exp $ *)

type c_decl = | Decl of string * string
type c_ast =
  | Asch of Asched.annotated_schedule
  | For of c_ast * c_ast * c_ast * c_ast
  | If of c_ast * c_ast
  | Block of c_decl list * c_ast list
  | Binop of string * Expr.expr * Expr.expr
  | Expr_assign of Expr.expr * Expr.expr
  | Stmt_assign of Expr.expr * Expr.expr
  | Comma of c_ast * c_ast
type c_fcn = | Fcn of string * string * c_decl list * c_ast

val make_c_unparser : (Variable.variable -> string) -> c_fcn -> string
