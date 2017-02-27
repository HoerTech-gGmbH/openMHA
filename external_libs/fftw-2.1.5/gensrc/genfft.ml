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

(* $Id: genfft.ml,v 1.79 2003/03/16 23:43:46 stevenj Exp $ *)

(* This file contains the entry point for the genfft program: it
   parses the command-line parameters and calls the rest of the
   program as needed to generate the requested code. *)

open Util
open To_c
open Expr

let optimize expr =
  let _ = info "simplifiying..." in
  let simple =  Exprdag.simplify_to_alist expr in
  let _ = info "scheduling..." in
  let scheduled = Schedule.schedule simple in
  let _ = info "annotating..." in
  let annotated = Asched.annotate scheduled in
  let _ = info "unparsing..." in
  annotated

let make_expr name = Var (Variable.make_named name)

let iarray = "input"
let oarray = "output"
let istride = "istride"
let ostride = "ostride"
let twiddle_order = "twiddle_order"

type codelet_type = TWIDDLE | NO_TWIDDLE | REAL2HC | HC2HC | HC2REAL
                    | REALEVEN | REALODD | REALEVEN2 | REALODD2
                    | REALEVEN_TWIDDLE | REALODD_TWIDDLE
		    | MP3MDCT  (* mdct used in mp3 encoding *)

let rec list_to_c = function
    [] -> ""
  | [a] -> (string_of_int a)
  | a :: b -> (string_of_int a) ^ ", " ^ (list_to_c b)

let codelet_description n dir ty f =
  let Fcn (_, name, _, _) = f 
  and (ctype, itype) = match ty with
    TWIDDLE -> "FFTW_TWIDDLE", 0
  | NO_TWIDDLE -> "FFTW_NOTW", 1
  | REAL2HC -> "FFTW_REAL2HC", 2
  | HC2HC -> "FFTW_HC2HC", 3
  | HC2REAL -> "FFTW_HC2REAL", 4
  | REALEVEN -> "FFTW_REALEVEN", 5
  | REALODD -> "FFTW_REALODD", 6
  | REALEVEN2 -> "FFTW_REALEVEN2", 7
  | REALODD2 -> "FFTW_REALODD2", 8
  | REALEVEN_TWIDDLE -> "FFTW_REALEVEN_TWIDDLE", 9
  | REALODD_TWIDDLE -> "FFTW_REALODD_TWIDDLE", 10
  | MP3MDCT -> "FFTW_MP3MDCT", 11
  and (cdir, idir) = match dir with
    Fft.FORWARD -> "FFTW_FORWARD", 0
  | Fft.BACKWARD -> "FFTW_BACKWARD", 1

  and (_, num_twiddle, tw_o) = Twiddle.twiddle_policy ()
  in let (declare_order, order, nt) =
    match ty with
      TWIDDLE -> ("static const int " ^ twiddle_order ^ "[] = {" ^
		  (list_to_c (tw_o n)) ^ "};\n"),
	twiddle_order,
	num_twiddle n
    | NO_TWIDDLE -> "", "(const int *) 0", 0
    | REAL2HC -> "", "(const int *) 0", 0
    | HC2HC -> 
	("static const int " ^ twiddle_order ^ "[] = {" ^
		  (list_to_c (tw_o n)) ^ "};\n"),
	twiddle_order,
	num_twiddle n
    | HC2REAL -> "", "(const int *) 0", 0
    | REALEVEN -> "", "(const int *) 0", 0
    | REALODD -> "", "(const int *) 0", 0
    | REALEVEN2 -> "", "(const int *) 0", 0
    | REALODD2 -> "", "(const int *) 0", 0
    | REALEVEN_TWIDDLE -> 
        ("static const int " ^ twiddle_order ^ "[] = {" ^
                  (list_to_c (tw_o n)) ^ "};\n"),
        twiddle_order,
        num_twiddle n
    | REALODD_TWIDDLE -> 
        ("static const int " ^ twiddle_order ^ "[] = {" ^
                  (list_to_c (tw_o n)) ^ "};\n"),
        twiddle_order,
        num_twiddle n
    | MP3MDCT -> "", "(const int *) 0", 0

  (* this should be replaced by CRC/MD5 of the codelet *)
  and signature = 11 * (2 * n + idir) + itype 

  in "\n\n" ^ declare_order ^
  "fftw_codelet_desc " ^ name ^ "_desc = {\n" ^
  "\"" ^ name ^ "\",\n" ^
  "(void (*)()) " ^ name ^ ",\n" ^
  (string_of_int n) ^ ",\n" ^
  cdir ^ ",\n" ^
  ctype ^ ",\n" ^
  (string_of_int signature) ^ ",\n" ^
  (string_of_int nt) ^ ",\n" ^
  order ^ ",\n" ^
  "};\n"

let fftw_no_twiddle_gen n dir =
  let _ = info "generating..." in
  let asch = optimize (Fft.no_twiddle_gen_expr n Symmetry.no_sym dir)
  and ns = string_of_int n
  and (name, sign) = match dir with
    Fft.FORWARD -> "fftw_no_twiddle_", (-1)
  | Fft.BACKWARD -> "fftwi_no_twiddle_", 1
  and unparse_var = 
    Variable.make_unparser (iarray, Some istride) 
      (oarray, Some ostride) ("BUG", None)
  in let tree = 
	Fcn ("void", name ^ ns,
	     [Decl ("const fftw_complex *", iarray);
	      Decl ("fftw_complex *", oarray);
	      Decl ("int", istride);
	      Decl ("int", ostride)],
	     Asch asch)
  in let desc = codelet_description n dir NO_TWIDDLE tree
  in ((make_c_unparser unparse_var) tree) ^ desc

let athena_no_twiddle_gen n dir =
  let _ = info "generating..." in
  let asch = optimize (Fft.no_twiddle_gen_expr n Symmetry.no_sym dir)
  and ns = string_of_int n
  and (name, sign) = match dir with
    Fft.FORWARD -> "athfft_", (-1)
  | Fft.BACKWARD -> "athffti_", 1
  and unparse_var = 
    Variable.make_unparser (iarray, None) 
      (iarray, None) ("BUG", None)
  in let tree = 
	Fcn ("void", name ^ ns,
	     [Decl ("fftw_complex *", iarray)],
	     Asch asch)
  and prologue = 
    "#include \"athfft.h\"\n\n" ^
    (if dir == Fft.FORWARD then
      "int ath_permutation_" ^ ns ^ "(int i) { return i; }\n\n\n"
    else "")

  in prologue ^ ((make_c_unparser unparse_var) tree)

let no_twiddle_gen n dir =
  if (!Magic.athenafft) then
    athena_no_twiddle_gen n dir
  else
    fftw_no_twiddle_gen n dir

let real_oarray = "real_output"
let imag_oarray = "imag_output"
let real_ostride = "real_ostride"
let imag_ostride = "imag_ostride"

let real2hc_gen n =
  let _ = info "generating..." in
  let dir = Fft.FORWARD in
  let asch = optimize (Fft.no_twiddle_gen_expr n Symmetry.real_sym dir)
  and ns = string_of_int n
  and (name, sign) = "fftw_real2hc_", (-1)
  and unparse_var = 
    Variable.make_real2hc_unparser (iarray, Some istride) 
      (real_oarray, Some real_ostride) 
      (imag_oarray, Some imag_ostride) 
  in let tree = 
	Fcn ("void", name ^ ns,
	     [Decl ("const fftw_real *", iarray);
	      Decl ("fftw_real *", real_oarray);
	      Decl ("fftw_real *", imag_oarray);
	      Decl ("int", istride);
	      Decl ("int", real_ostride);
	       Decl ("int", imag_ostride)],
	     Asch asch)
  in let desc = codelet_description n dir REAL2HC tree
  in ((make_c_unparser unparse_var) tree) ^ desc


let real_iarray = "real_input"
let imag_iarray = "imag_input"
let real_istride = "real_istride"
let imag_istride = "imag_istride"

let hc2real_gen n =
  let _ = info "generating..." in
  let dir = Fft.BACKWARD in
  let asch = optimize (Fft.no_twiddle_gen_expr n Symmetry.hermitian_sym dir)
  and ns = string_of_int n
  and (name, sign) = "fftw_hc2real_", 1
  and unparse_var = 
    Variable.make_hc2real_unparser (real_iarray, Some real_istride) 
      (imag_iarray, Some imag_istride) 
      (oarray, Some ostride) 
  in let tree = 
	Fcn ("void", name ^ ns,
	     [Decl ("const fftw_real *", real_iarray);
	      Decl ("const fftw_real *", imag_iarray);
	      Decl ("fftw_real *", oarray);
	      Decl ("int", real_istride);
	      Decl ("int", imag_istride);
	       Decl ("int", ostride)],
	     Asch asch)
  in let desc = codelet_description n dir HC2REAL tree
  in ((make_c_unparser unparse_var) tree) ^ desc

let realeven_gen n =
  let _ = info "generating..." in
  let dir = Fft.FORWARD in
  let asch = optimize (Fft.no_twiddle_gen_expr n Symmetry.realeven_sym dir)
  and ns = string_of_int n
  and (name, sign) = "fftw_realeven_", (-1)
  and unparse_var = 
    Variable.make_realeven_unparser (iarray, Some istride) 
                                    (oarray, Some ostride) 
  in let tree = 
	Fcn ("void", name ^ ns,
	     [Decl ("const fftw_real *", iarray);
	      Decl ("fftw_real *", oarray);
	      Decl ("int", istride);
	      Decl ("int", ostride)],
	     Asch asch)
  in let desc = codelet_description n dir REALEVEN tree
  in ((make_c_unparser unparse_var) tree) ^ desc

let realodd_gen n =
  let _ = info "generating..." in
  let dir = Fft.FORWARD in
  let asch = optimize (Fft.no_twiddle_gen_expr n Symmetry.realodd_sym dir)
  and ns = string_of_int n
  and (name, sign) = "fftw_realodd_", (-1)
  and unparse_var = 
    Variable.make_realodd_unparser (iarray, Some istride) 
                                   (oarray, Some ostride) 
  in let tree = 
	Fcn ("void", name ^ ns,
	     [Decl ("const fftw_real *", iarray);
	      Decl ("fftw_real *", oarray);
	      Decl ("int", istride);
	      Decl ("int", ostride)],
	     Asch asch)
  in let desc = codelet_description n dir REALODD tree
  in ((make_c_unparser unparse_var) tree) ^ desc

let realeven2_gen n =
  let _ = info "generating..." in
  let dir = Fft.FORWARD in
  let asch = optimize (Fft.no_twiddle_gen_expr (2 * n)
                         Symmetry.realeven2_input_sym dir) 
  and ns = string_of_int n
  and (name, sign) = "fftw_realeven2_", (-1)
  and unparse_var = 
    Variable.make_realeven_unparser (iarray, Some istride) 
                                    (oarray, Some ostride) 
  in let tree = 
	Fcn ("void", name ^ ns,
	     [Decl ("const fftw_real *", iarray);
	      Decl ("fftw_real *", oarray);
	      Decl ("int", istride);
	      Decl ("int", ostride)],
	     Asch asch)
  in let desc = codelet_description n dir REALEVEN2 tree
  in ((make_c_unparser unparse_var) tree) ^ desc

let realodd2_gen n =
  let _ = info "generating..." in
  let dir = Fft.FORWARD in
  let asch = optimize (Fft.no_twiddle_gen_expr (2 * n)
			 Symmetry.realodd2_input_sym dir)
  and ns = string_of_int n
  and (name, sign) = "fftw_realodd2_", (-1)
  and unparse_var = 
    Variable.make_realodd2_unparser (iarray, Some istride) 
                                    (oarray, Some ostride) 
  in let tree = 
	Fcn ("void", name ^ ns,
	     [Decl ("const fftw_real *", iarray);
	      Decl ("fftw_real *", oarray);
	      Decl ("int", istride);
	      Decl ("int", ostride)],
	     Asch asch)
  in let desc = codelet_description n dir REALODD2 tree
  in ((make_c_unparser unparse_var) tree) ^ desc

let mp3mdct_gen n =
  let _ = info "generating..." in
  let dir = Fft.FORWARD in
  let asch = optimize (Fft.no_twiddle_gen_expr (4 * n)
                         Symmetry.mp3mdct_input_sym dir) 
  and ns = string_of_int n
  and (name, sign) = "fftw_mp3mdct_", (-1)
  and unparse_var = 
    Variable.make_realeven_unparser (iarray, None) (oarray, None) 
  in let tree = 
	Fcn ("void", name ^ ns,
	     [Decl ("const fftw_real *", iarray);
	      Decl ("fftw_real *", oarray)],
	     Asch asch)
  in let desc = codelet_description n dir MP3MDCT tree
  in ((make_c_unparser unparse_var) tree) ^ desc

let ioarray = "inout"
let iostride = "iostride"
let twarray = "W"

let fftw_twiddle_gen n dir =
  let _ = info "generating..." in
  let asch = optimize (Fft.twiddle_dit_gen_expr n Symmetry.no_sym 
			 Symmetry.no_sym dir)
  and ns = string_of_int n
  and m = "m"
  and dist = "dist"
  and a = "A"
  and i = "i"

  in let me = make_expr m
  and diste = make_expr dist
  and ae = make_expr a
  and ie = make_expr i
  and ioarraye = make_expr ioarray
  and twarraye = make_expr twarray

  and (name, sign) = match dir with
    Fft.FORWARD -> "fftw_twiddle_", (-1)
  | Fft.BACKWARD -> "fftwi_twiddle_", 1

  and unparse_var =
    Variable.make_unparser (ioarray, Some iostride) 
      (ioarray, Some iostride) (twarray, None)

  and (_, num_twiddles, _) = Twiddle.twiddle_policy ()

  in let body = Block (
    [Decl ("int", i); Decl ("fftw_complex *", ioarray)],
    [Stmt_assign (ioarraye, ae);
      For (Expr_assign (ie, me),
	   Binop (" > ", ie, Integer 0),
	   Comma (Expr_assign (ie, Plus [ie; Uminus (Integer 1)]),
		  (Comma (Expr_assign (ioarraye, Plus [ioarraye; diste]),
                          (Expr_assign (twarraye, 
					Plus [twarraye;
					      Integer (num_twiddles n)]))))),
	   Asch asch)])
  in let fcnname = name ^ ns
  in let tree = 
    Fcn ("void", fcnname,
	 [Decl ("fftw_complex *", a);
	   Decl ("const fftw_complex *", twarray);
	   Decl ("int", iostride);
	   Decl ("int", m);
	   Decl ("int", dist)],
         body)

  in let desc = codelet_description n dir TWIDDLE tree
  in ((make_c_unparser unparse_var) tree) ^ desc

let athena_twiddle_gen n dir =
  let _ = info "generating..." 
  and (name, sign, generator) = match dir with
    Fft.FORWARD -> "athfft_twiddle_", (-1), Fft.twiddle_dif_gen_expr
  | Fft.BACKWARD -> "athffti_twiddle_", 1, Fft.twiddle_dit_gen_expr in
  let asch = optimize (generator n Symmetry.no_sym Symmetry.no_sym dir)
  and ns = string_of_int n
  and a = "A"
  and i = "i"

  in let ae = make_expr a
  and ie = make_expr i
  and ioarraye = make_expr ioarray
  and twarraye = make_expr twarray

  and unparse_var =
    Variable.make_unparser (ioarray, Some iostride) 
      (ioarray, Some iostride) (twarray, None)

  and (_, num_twiddles, _) = Twiddle.twiddle_policy ()

  in let body = Block (
    [Decl ("int", i); Decl ("fftw_complex *", ioarray)],
    [Stmt_assign (ioarraye, ae);
      For (Expr_assign (ie, make_expr iostride),
	   Binop (" > ", ie, Integer 0),
	   Comma (Expr_assign (ie, Plus [ie; Uminus (Integer 1)]),
		  (Comma (Expr_assign (ioarraye, Plus [ioarraye; Integer 1]),
                          (Expr_assign (twarraye, 
					Plus [twarraye;
					      Integer (num_twiddles n)]))))),
	   Asch asch)])
  in let fcnname = name ^ ns
  in let tree = 
    Fcn ("void", fcnname,
	 [Decl ("fftw_complex *", a);
	   Decl ("const fftw_complex *", twarray);
	   Decl ("int", iostride)],
         body)
  and prologue = 
    "#include \"athfft.h\"\n\n"
  in prologue ^ ((make_c_unparser unparse_var) tree)

let twiddle_gen n dir =
  if (!Magic.athenafft) then
    athena_twiddle_gen n dir
  else
    fftw_twiddle_gen n dir

let arrayX = "X"
let arrayY = "Y"

let hc2hc_gen n dir =
  let _ = info "generating..." in
  let (zeroth_elements, middle_elements, final_elements, name, sign, 
       make_variable_unparser) =
    match dir with
      Fft.FORWARD -> (
	optimize (Fft.no_twiddle_gen_expr n Symmetry.real_sym dir),
	optimize (Fft.twiddle_dit_gen_expr n Symmetry.no_sym 
		    Symmetry.middle_hc2hc_forward_sym dir),
	optimize (Fft.no_twiddle_gen_expr (2 * n) 
		    Symmetry.final_hc2hc_forward_sym dir),
	"fftw_hc2hc_forward_", 
	(-1),
	Variable.make_hc2hc_forward_unparser)
    | Fft.BACKWARD -> (
	optimize (Fft.no_twiddle_gen_expr n Symmetry.hermitian_sym dir),
	optimize (Fft.twiddle_dif_gen_expr n Symmetry.middle_hc2hc_backward_sym
		    Symmetry.no_sym dir),
	optimize (Fft.no_twiddle_gen_expr (2 * n) 
		    Symmetry.final_hc2hc_backward_sym dir),
	"fftw_hc2hc_backward_", 
	1,
	Variable.make_hc2hc_backward_unparser)
  and ns = string_of_int n
  and m = "m"
  and dist = "dist"
  and a = "A"
  and i = "i"

  in let me = make_expr m
  and diste = make_expr dist
  and iostridee = make_expr iostride
  and ae = make_expr a
  and ie = make_expr i
  and arrayXe = make_expr arrayX
  and arrayYe = make_expr arrayY
  and twarraye = make_expr twarray
  and unparse_var = make_variable_unparser n (arrayX, Some iostride) 
      (arrayY, Some iostride) (twarray, None)

  and (_, num_twiddles, _) = Twiddle.twiddle_policy ()

  in let body = Block (
    [
    Decl ("int", i); 
    Decl ("fftw_real *", arrayX);
    Decl ("fftw_real *", arrayY)],
    [
    Stmt_assign (arrayXe, ae);
    Stmt_assign (arrayYe, Plus [ae; Times (Integer n, iostridee)]);
    Asch zeroth_elements;
    Stmt_assign (arrayXe, Plus [arrayXe; diste]);
    Stmt_assign (arrayYe, Plus [arrayYe; Uminus diste]);
    For (Expr_assign (ie, Integer 2),
	 Binop (" < ", ie, me),
	 Comma (Expr_assign (ie, Plus [ie; Integer 2]),
		(Comma 
		   (Comma 
		      (Expr_assign (arrayXe, Plus [arrayXe; diste]),
		       Expr_assign (arrayYe, Plus [arrayYe; Uminus diste])),
                    (Expr_assign (twarraye, 
				  Plus [twarraye;
					 Integer (num_twiddles n)]))))),
	 Asch middle_elements);
    If (Binop (" == ", ie, me),
	Asch final_elements)])
  in let fcnname = name ^ ns
  in let tree = 
    Fcn ("void", fcnname,
	 [Decl ("fftw_real *", a);
	   Decl ("const fftw_complex *", twarray);
	   Decl ("int", iostride);
	   Decl ("int", m);
	   Decl ("int", dist)],
         body)

  in let desc = codelet_description n dir HC2HC tree
  in ((make_c_unparser unparse_var) tree) ^ desc

let usage = "Usage: genfft [-notwiddle | -notwiddleinv | -twiddle | -twiddleinv| -real2hc | -hc2hc-forward |  -hc2hc-backward ] <number>"

type mode = 
  | GEN_NOTHING
  | GEN_TWIDDLE of int
  | GEN_NOTWID of int
  | GEN_TWIDDLEI of int
  | GEN_NOTWIDI of int
  | GEN_REAL2HC of int
  | GEN_HC2REAL of int
  | GEN_HC2HC_FORWARD of int
  | GEN_HC2HC_BACKWARD of int
  | GEN_REAL_EVEN of int
  | GEN_REAL_ODD of int
  | GEN_REAL_EVEN2 of int
  | GEN_REAL_ODD2 of int
  | GEN_REAL_EVEN_TWIDDLE of int
  | GEN_REAL_ODD_TWIDDLE of int
  | GEN_MP3MDCT of int

let mode = ref GEN_NOTHING

let undocumented = " Undocumented voodoo parameter"

let main () =
  Arg.parse [
  "-notwiddle", Arg.Int(fun i -> mode := GEN_NOTWID i), "<n> : Generate a no twiddle codelet of size <n>";
  "-notwiddleinv", Arg.Int(fun i -> mode := GEN_NOTWIDI i), "<n> : Generate a no twiddle inverse codelet of size <n>";
  "-twiddle", Arg.Int(fun i -> mode := GEN_TWIDDLE i), "<n> : Generate a twiddle codelet of size <n>";
  "-twiddleinv", Arg.Int(fun i -> mode := GEN_TWIDDLEI i), "<n> : Generate a twiddle inverse codelet of size <n>";
  "-real2hc", Arg.Int(fun i -> mode := GEN_REAL2HC i), "<n> : Generate a real to halfcomplex codelet of size <n>";
  "-hc2hc-forward", Arg.Int(fun i -> mode := GEN_HC2HC_FORWARD i), "<n> : Generate a forward halfcomplex to halfcomplex codelet of size <n>";
  "-hc2hc-backward", Arg.Int(fun i -> mode := GEN_HC2HC_BACKWARD i), "<n> : Generate a backward halfcomplex to halfcomplex codelet of size <n>";
  "-hc2real", Arg.Int(fun i -> mode := GEN_HC2REAL i), "<n> : Generate a halfcomplex to real codelet of size <n>";
  "-realeven", Arg.Int(fun i -> mode := GEN_REAL_EVEN i), "<n> : Generate a real even codelet of size <n>";
  "-realodd", Arg.Int(fun i -> mode := GEN_REAL_ODD i), "<n> : Generate a real odd codelet of size <n>";
  "-realeven2", Arg.Int(fun i -> mode := GEN_REAL_EVEN2 i), "<n> : Generate a real even-2 codelet of size <n>";
  "-realodd2", Arg.Int(fun i -> mode := GEN_REAL_ODD2 i), "<n> : Generate a real odd-2 codelet of size <n>";
  "-realeventwiddle", Arg.Int(fun i -> mode := GEN_REAL_EVEN_TWIDDLE i), "<n> : Generate a real even twiddle codelet of size <n>";
  "-realoddtwiddle", Arg.Int(fun i -> mode := GEN_REAL_ODD_TWIDDLE i), "<n> : Generate a real odd twiddle codelet of size <n>";
  "-mp3mdct", Arg.Int(fun i -> mode := GEN_MP3MDCT i), "<n> : Generate a MPEG3 MDCT codelet of size <n>";

  "-verbose", 
  Arg.Unit(fun () -> Magic.verbose := true),
  " Enable verbose logging messages to stderr";


  "-inline-konstants", 
  Arg.Unit(fun () -> Magic.inline_konstants := true),
  " Inline floating point constants";
  "-no-inline-konstants", 
  Arg.Unit(fun () -> Magic.inline_konstants := false),
  " Do not inline floating point constants";

  "-rader-min", Arg.Int(fun i -> Magic.rader_min := i),
  "<n> : Use Rader's algorithm for prime sizes >= <n>";

  "-magic-alternate-convolution", 
  Arg.Int(fun i -> Magic.alternate_convolution := i),
  undocumented;

  "-magic-athenafft", 
  Arg.Unit(fun () -> Magic.athenafft := true),
  undocumented;

  "-magic-window", Arg.Int(fun i -> Magic.window := i), undocumented;
  "-magic-variables", Arg.Int(fun i -> Magic.number_of_variables := i),
  undocumented;

  "-magic-loopo", 
  Arg.Unit(fun () -> Magic.loopo := true),
  undocumented;
  "-magic-loopi", 
  Arg.Unit(fun () -> Magic.loopo := false),
  undocumented;

  "-magic-times-3-3", 
  Arg.Unit(fun () -> Magic.times_3_3 := true),
  undocumented;
  "-magic-times-4-2", 
  Arg.Unit(fun () -> Magic.times_3_3 := false),
  undocumented;

  "-magic-inline-single", 
  Arg.Unit(fun () -> Magic.inline_single := true),
  undocumented;
  "-magic-no-inline-single", 
  Arg.Unit(fun () -> Magic.inline_single := false),
  undocumented;

  "-magic-inline-loads", 
  Arg.Unit(fun () -> Magic.inline_loads := true),
  undocumented;
  "-magic-no-inline-loads", 
  Arg.Unit(fun () -> Magic.inline_loads := false),
  undocumented;

  "-magic-enable-fma", 
  Arg.Unit(fun () -> Magic.enable_fma := true),
  undocumented;
  "-magic-disable-fma", 
  Arg.Unit(fun () -> Magic.enable_fma := false),
  undocumented;

  "-magic-enable-fma-expansion", 
  Arg.Unit(fun () -> Magic.enable_fma_expansion := true),
  undocumented;
  "-magic-disable-fma-expansion", 
  Arg.Unit(fun () -> Magic.enable_fma_expansion := false),
  undocumented;

  "-magic-collect-common-twiddle", 
  Arg.Unit(fun () -> Magic.collect_common_twiddle := true),
  undocumented;
  "-magic-no-collect-common-twiddle", 
  Arg.Unit(fun () -> Magic.collect_common_twiddle := false),
  undocumented;

  "-magic-collect-common-inputs", 
  Arg.Unit(fun () -> Magic.collect_common_inputs := true),
  undocumented;
  "-magic-no-collect-common-inputs", 
  Arg.Unit(fun () -> Magic.collect_common_inputs := false),
  undocumented;

  "-magic-alignment-check", 
  Arg.Unit(fun () -> Magic.alignment_check := true),
  undocumented;

  "-magic-use-wsquare", 
  Arg.Unit(fun () -> Magic.use_wsquare := true),
  undocumented;
  "-magic-no-wsquare", 
  Arg.Unit(fun () -> Magic.use_wsquare := false),
  undocumented;

  "-magic-twiddle-load-all", 
  Arg.Unit(fun () -> Magic.twiddle_policy := Magic.TWIDDLE_LOAD_ALL),
  undocumented;
  "-magic-twiddle-iter", 
  Arg.Unit(fun () -> Magic.twiddle_policy := Magic.TWIDDLE_ITER),
  undocumented;
  "-magic-twiddle-load-odd", 
  Arg.Unit(fun () -> Magic.twiddle_policy := Magic.TWIDDLE_LOAD_ODD),
  undocumented;
  "-magic-twiddle-square1", 
  Arg.Unit(fun () -> Magic.twiddle_policy := Magic.TWIDDLE_SQUARE1),
  undocumented;
  "-magic-twiddle-square2", 
  Arg.Unit(fun () -> Magic.twiddle_policy := Magic.TWIDDLE_SQUARE2),
  undocumented;
  "-magic-twiddle-square3", 
  Arg.Unit(fun () -> Magic.twiddle_policy := Magic.TWIDDLE_SQUARE3),
  undocumented;
] (fun _ -> failwith "too many arguments") usage;

  let out = 
    match !mode with
    | GEN_TWIDDLE i -> (twiddle_gen i Fft.FORWARD)
    | GEN_TWIDDLEI i -> (twiddle_gen i Fft.BACKWARD)
    | GEN_NOTWID i -> (no_twiddle_gen i Fft.FORWARD)
    | GEN_NOTWIDI i -> (no_twiddle_gen i Fft.BACKWARD)
    | GEN_REAL2HC i -> (real2hc_gen i)
    | GEN_MP3MDCT i -> (mp3mdct_gen i)
    | GEN_HC2HC_FORWARD i -> (hc2hc_gen i Fft.FORWARD)
    | GEN_HC2REAL i -> (hc2real_gen i)
    | GEN_HC2HC_BACKWARD i -> (hc2hc_gen i Fft.BACKWARD)
    | GEN_REAL_EVEN i -> (realeven_gen i)
    | GEN_REAL_ODD i -> (realodd_gen i)
    | GEN_REAL_EVEN2 i -> (realeven2_gen i)
    | GEN_REAL_ODD2 i -> (realodd2_gen i)
    | _ -> failwith "one of -notwiddle, -notwiddleinv, -twiddle, -twiddleinv, -real2hc, -hc2real, -hc2hc-forward, or -hc2hc-forward must be specified"

  in begin
    print_string out;
    exit 0;
  end

let _ = main()

