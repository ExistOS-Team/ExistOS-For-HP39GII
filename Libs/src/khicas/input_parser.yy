/* -*- mode:Mail; compile-command: "bison -p giac_yy -y -d input_parser.yy ; mv -f y.tab.c input_parser.cc ; mv -f y.tab.h input_parser.h ; make input_parser.o" -*- 
 *
 *  Input grammar definition for reading expressions.
 *  This file must be processed with yacc/bison.
 * replace YYSTYPE by giac::YYSTYPE in lexer.h after bison
 * add YYLEX_PARAM to yylex ( call
 */

/*
 *  Copyright (C) 2001,14 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
 *  The very first version was inspired by GiNaC parser
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

         %{
         #define YYPARSE_PARAM scanner
         #define YYLEX_PARAM   scanner
	 %}
/*
 *  Definitions
 */
%pure-parser
%parse-param {void * scanner}
%{
#include "giacPCH.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "first.h"
#include <stdexcept>
#include <cstdlib>
#include "giacPCH.h"
#include "index.h"
#include "gen.h"
#define YYSTYPE gen
#define YY_EXTRA_TYPE  context const *
#include "lexer.h"
#include "input_lexer.h"
#include "usual.h"
#include "derive.h"
#include "sym2poly.h"
#include "vecteur.h"
#include "modpoly.h"
#include "alg_ext.h"
#include "prog.h"
#include "rpn.h"
#include "intg.h"
#include "plot.h"
#include "maple.h"
using namespace std;

#ifndef NO_NAMESPACE_GIAC
namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

// It seems there is a bison bug when it reallocates space for the stack
// therefore I redefine YYINITDEPTH to 4000 (max size is YYMAXDEPTH)
// instead of 200
// Feel free to change if you need but then readjust YYMAXDEPTH
#if 1//defined RTOS_THREADX || defined NSPIRE || defined FXCG
#define YYINITDEPTH 100
#define YYMAXDEPTH 101
#else // RTOS_THREADX
// Note that the compilation by bison with -v option generates a file y.output
// to debug the grammar, compile input_parser.yy with bison
// then add yydebug=1 in input_parser.cc at the beginning of yyparse (
#define YYDEBUG 1
#ifdef GNUWINCE
#define YYINITDEPTH 1000
#else 
#define YYINITDEPTH 4000
#define YYMAXDEPTH 20000
#define YYERROR_VERBOSE 1
#endif // GNUWINCE
#endif // RTOS_THREADX
#define YYSTACK_USE_ALLOCA 1


gen polynome_or_sparse_poly1(const gen & coeff, const gen & index){
  if (index.type==_VECT){
    index_t i;
    const_iterateur it=index._VECTptr->begin(),itend=index._VECTptr->end();
    i.reserve(itend-it);
    for (;it!=itend;++it){
      if (it->type!=_INT_)
         return gentypeerr();
      i.push_back(it->val);
    }
    monomial<gen> m(coeff,i);
    return polynome(m);
  }
  else {
    sparse_poly1 res;
    res.push_back(monome(coeff,index));
    return res;
  }
}
%}

/* Tokens  */
%token 	T_NUMBER T_SYMBOL T_LITERAL T_DIGITS T_STRING T_END_INPUT
	T_EXPRESSION T_UNARY_OP T_OF T_NOT T_TYPE_ID T_VIRGULE
	T_AFFECT T_MAPSTO T_BEGIN_PAR T_END_PAR 
        T_PLUS T_MOINS T_DIV T_MOD T_POW T_QUOTED_BINARY T_QUOTE T_PRIME
	T_TEST_EQUAL T_EQUAL 
	T_INTERVAL T_UNION T_INTERSECT T_MINUS 
	T_AND_OP T_COMPOSE T_DOLLAR T_DOLLAR_MAPLE 
	T_INDEX_BEGIN T_VECT_BEGIN T_VECT_DISPATCH T_VECT_END 
	T_SEMI T_DEUXPOINTS T_DOUBLE_DEUX_POINTS
	T_IF T_ELIF T_THEN T_ELSE T_IFTE 
	T_SWITCH T_CASE T_DEFAULT T_ENDCASE 
	T_FOR T_FROM T_TO T_DO T_BY T_WHILE T_MUPMAP_WHILE 
	T_REPEAT T_UNTIL T_IN
	T_START T_BREAK T_CONTINUE 
	T_TRY T_CATCH T_TRY_CATCH
	T_PROC T_BLOC T_BLOC_BEGIN T_BLOC_END T_RETURN 
	T_LOCAL T_ARGS T_FACTORIAL
	T_ROOTOF_BEGIN T_ROOTOF_END 
	T_SPOLY1_BEGIN T_SPOLY1_END 
	T_HELP
	TI_STO T_PIPE TI_HASH
	T_INTERROGATION T_SQ T_IFERR
	T_UNARY_OP_38

/* Operator precedence and associativity */
/* %nonassoc T_ELSE 
%nonassoc T_IF */
%nonassoc T_RETURN
%nonassoc T_FUNCTION
%nonassoc TI_STO
%nonassoc T_PIPE
%right T_AFFECT
%nonassoc T_FOR 
%left T_VIRGULE
%nonassoc T_INTERROGATION
%nonassoc T_LOGO // for repete, must be there w.r.t. T_VIRGULE
%nonassoc T_BIDON
%right T_DEUXPOINTS
%nonassoc T_MAPSTO
%left T_AND_OP 
%nonassoc T_IN
%left T_DOLLAR_MAPLE // not the same precedence than for spreadsheet
%right T_EQUAL 
%left T_TEST_EQUAL
%left T_UNION
%nonassoc T_MINUS
%left T_INTERSECT
%left T_INTERVAL
%left T_PLUS 
%nonassoc T_NUMBER
%left T_IMPMULT
%left T_DIV 
%nonassoc T_MOD
%nonassoc T_NOT
%nonassoc T_DOLLAR // this priority for spreadsheet
%nonassoc T_PRIME
%right T_POW
%nonassoc T_FACTORIAL
%left T_SQ
%nonassoc T_UNARY_OP T_UNARY_OP_38
%left T_COMPOSE
%nonassoc T_DOUBLE_DEUX_POINTS
%nonassoc TI_HASH

%start input


/*
 *  Grammar rules
 */

%%
input	: correct_input {   const giac::context * contextptr = giac_yyget_extra(scanner);
			    if ($1._VECTptr->size()==1)
			     parsed_gen($1._VECTptr->front(),contextptr);
                          else
			     parsed_gen(gen(*$1._VECTptr,_SEQ__VECT),contextptr);
			 }
	;

correct_input : exp T_END_INPUT { $$=vecteur(1,$1); }
	      | exp T_SEMI T_END_INPUT { if ($2.val==1) $$=vecteur(1,symbolic(at_nodisp,$1)); else $$=vecteur(1,$1); }
	      | exp T_SEMI correct_input { if ($2.val==1) $$=mergevecteur(makevecteur(symbolic(at_nodisp,$1)),*$3._VECTptr); else $$=mergevecteur(makevecteur($1),*$3._VECTptr); }
	      ;

exp	: T_NUMBER		{$$ = $1;}
	| T_NUMBER symbol_or_literal %prec T_IMPMULT	{if (is_one($1)) $$=$2; else $$=symbolic(at_prod,gen(makevecteur($1,$2),_SEQ__VECT));}
	| T_NUMBER symbol_or_literal T_POW T_NUMBER %prec T_IMPMULT	{if (is_one($1)) $$=symb_pow($2,$4); else $$=symbolic(at_prod,gen(makevecteur($1,symb_pow($2,$4)),_SEQ__VECT));}
	| T_NUMBER symbol_or_literal T_SQ %prec T_IMPMULT	{$$=symbolic(at_prod,gen(makevecteur($1,symb_pow($2,$3)) ,_SEQ__VECT));}
	| T_NUMBER T_UNARY_OP T_BEGIN_PAR exp T_END_PAR	{ $$ =$1*symbolic(*$2._FUNCptr,python_compat(giac_yyget_extra(scanner))?denest_sto($4):$4); }
	| T_NUMBER T_UNARY_OP T_BEGIN_PAR exp T_END_PAR T_POW T_NUMBER	{ $$ =$1*symb_pow(symbolic(*$2._FUNCptr,python_compat(giac_yyget_extra(scanner))?denest_sto($4):$4),$7); }
	/* | T_LITERAL T_NUMBER	{$$=symbolic(at_prod,makevecteur($1,$2));} */
	| T_STRING		{ $$=$1; }
	| T_EXPRESSION		{ if ($1.type==_FUNC) $$=symbolic(*$1._FUNCptr,gen(vecteur(0),_SEQ__VECT)); else $$=$1; }
	/* | T_COMMENT		{ $$=symb_comment($1); }
	| T_COMMENT exp		{ $$=$2; } */
	| symbol T_BEGIN_PAR suite T_END_PAR T_AFFECT bloc {$$ = symb_program_sto($3,$3*zero,$6,$1,false,giac_yyget_extra(scanner));}
	| symbol T_BEGIN_PAR suite T_END_PAR T_AFFECT exp {if (is_array_index($1,$3,giac_yyget_extra(scanner)) || (abs_calc_mode(giac_yyget_extra(scanner))==38 && $1.type==_IDNT && strlen($1._IDNTptr->id_name)==2 && check_vect_38($1._IDNTptr->id_name))) $$=symbolic(at_sto,makesequence($6,symbolic(at_of,makesequence($1,$3)))); else { $$ = symb_program_sto($3,$3*zero,$6,$1,true,giac_yyget_extra(scanner)); $$._SYMBptr->feuille.subtype=_SORTED__VECT;  } }
	| exp TI_STO symbol T_BEGIN_PAR suite T_END_PAR {if (is_array_index($3,$5,giac_yyget_extra(scanner)) || (abs_calc_mode(giac_yyget_extra(scanner))==38 && $3.type==_IDNT && check_vect_38($3._IDNTptr->id_name))) $$=symbolic(at_sto,makesequence($1,symbolic(at_of,makesequence($3,$5)))); else $$ = symb_program_sto($5,$5*zero,$1,$3,false,giac_yyget_extra(scanner));}
	| exp TI_STO symbol T_INDEX_BEGIN exp T_VECT_END { 
         const giac::context * contextptr = giac_yyget_extra(scanner);
         gen g=symb_at($3,$5,contextptr); $$=symb_sto($1,g); 
        }
	| exp TI_STO symbol T_INDEX_BEGIN T_VECT_DISPATCH exp T_VECT_END T_VECT_END { 
         const giac::context * contextptr = giac_yyget_extra(scanner);
         gen g=symbolic(at_of,makesequence($3,$6)); $$=symb_sto($1,g); 
        }
	| exp TI_STO symbol {  $$=symb_sto($1,$3); }
	| exp TI_STO T_UNARY_OP { $$=symbolic(at_convert,makesequence($1,$3)); }
	| exp TI_STO T_PLUS { $$=symbolic(at_convert,makesequence($1,$3)); }
	| exp TI_STO T_DIV { $$=symbolic(at_convert,makesequence($1,$3)); }
	| exp TI_STO T_VIRGULE { $$=symbolic(at_time,$1);}
	| exp TI_STO TI_STO { $$=symbolic(at_solve,symb_equal($1,0));}
	| symbol T_BEGIN_PAR suite T_END_PAR {$$ = check_symb_of($1,python_compat(giac_yyget_extra(scanner))?denest_sto($3):$3,giac_yyget_extra(scanner));}
	| exp T_BEGIN_PAR suite T_END_PAR {$$ = check_symb_of($1,python_compat(giac_yyget_extra(scanner))?denest_sto($3):$3,giac_yyget_extra(scanner));}
	| symbol 		{$$ = $1;}  
	| T_LITERAL		{$$ = $1;}
	| T_DIGITS		{$$ = $1;}
	| T_DIGITS T_AFFECT exp	{$$ = symbolic(*$1._FUNCptr,$3);}
	| T_DIGITS T_BEGIN_PAR exp T_END_PAR	{$$ = symbolic(*$1._FUNCptr,$3);}
	| T_DIGITS T_BEGIN_PAR T_END_PAR	{$$ = symbolic(*$1._FUNCptr,gen(vecteur(0),_SEQ__VECT));}
	| exp TI_STO T_DIGITS	{$$ = symbolic(*$3._FUNCptr,$1);}
	| exp T_TEST_EQUAL exp	{if (is_inequation($1) || ($1.is_symb_of_sommet(at_same) && ($3.type!=_INT_ || $3.subtype!=_INT_BOOLEAN))){ $$ = symb_and($1,symbolic(*$2._FUNCptr,makesequence($1._SYMBptr->feuille[1],$3)));} else $$ = symbolic(*$2._FUNCptr,makesequence($1,$3));}
	| exp T_TEST_EQUAL symbol T_TEST_EQUAL exp	{$$ = symb_and(symbolic(*$2._FUNCptr,makesequence($1,$3)),symbolic(*$4._FUNCptr,makesequence($3,$5)));}
	| exp T_EQUAL exp	        {$$ = symbolic(*$2._FUNCptr,makesequence($1,$3)); }
	| T_EQUAL exp %prec T_BIDON { 
	if ($2.type==_SYMB) $$=$2; else $$=symbolic(at_nop,$2); 
	$$.change_subtype(_SPREAD__SYMB); 
        const giac::context * contextptr = giac_yyget_extra(scanner);
       spread_formula(false,contextptr); 
	}
	| exp T_PLUS exp    { if ($1.is_symb_of_sommet(at_plus) && $1._SYMBptr->feuille.type==_VECT && $2==at_plus){ $1._SYMBptr->feuille._VECTptr->push_back($3); $$=$1; } else
  $$ =symbolic(*$2._FUNCptr,makesequence($1,$3));}
	| exp T_DIV exp		{$$ =symbolic(*$2._FUNCptr,makesequence($1,$3));}
	| exp T_POW exp		{if ($1==symbolic(at_exp,1) && $2==at_pow) $$=symbolic(at_exp,$3); else $$ =symbolic(*$2._FUNCptr,makesequence($1,$3));}
	| exp T_MOD exp		{if ($2.type==_FUNC) $$=symbolic(*$2._FUNCptr,makesequence($1,$3)); else $$ = symbolic(at_normalmod,makesequence($1,$3));}
	| exp T_INTERVAL exp	{$$ = symbolic(*$2._FUNCptr,makesequence($1,$3)); }
	| exp T_INTERVAL	{$$ = symbolic(*$2._FUNCptr,makesequence($1,RAND_MAX)); }
	| T_INTERVAL exp	{$$ = symbolic(*$1._FUNCptr,makesequence(0,$2)); }
	| T_INTERVAL T_VIRGULE exp	{$$ = makesequence(symbolic(*$1._FUNCptr,makesequence(0,RAND_MAX)),$3); }
	/* | exp T_PLUS T_PLUS		{$$ = symb_sto($1+1,$1);} */
	| exp T_AND_OP exp	{$$ = symbolic(*$2._FUNCptr,makesequence($1,$3));}
	| exp T_DEUXPOINTS exp  {$$= symbolic(at_deuxpoints,makesequence($1,$3) );}
	| T_PLUS exp %prec T_NOT	{ 
					if ($2==unsigned_inf){
						$$ = $1==at_binary_minus?minus_inf:plus_inf;
					}
					else {
					 if ($1==at_binary_minus)
					   if ($2.type==_INT_) $$=(-$2.val); else { if ($2.type==_DOUBLE_) $$=(-$2._DOUBLE_val); else $$=symbolic(at_neg,$2); }
					   }
				}
	| T_SPOLY1_BEGIN exp T_VIRGULE exp T_SPOLY1_END {$$ = polynome_or_sparse_poly1(eval($2,1, giac_yyget_extra(scanner)),$4);}
	| T_ROOTOF_BEGIN exp T_ROOTOF_END { 
           if ( ($2.type==_SYMB) && ($2._SYMBptr->sommet==at_deuxpoints) )
             $$ = algebraic_EXTension($2._SYMBptr->feuille._VECTptr->front(),$2._SYMBptr->feuille._VECTptr->back());
           else $$=$2;
        }
	| T_OF { $$=gen(at_of,2); }
	| exp T_AFFECT exp 		{if ($1.type==_FUNC) *logptr(giac_yyget_extra(scanner))<< ("Warning: "+$1.print(context0)+" is a reserved word")<<endl; if ($1.type==_INT_) $$=symb_equal($1,$3); else {$$ = symb_sto($3,$1,$2==at_array_sto); if ($3.is_symb_of_sommet(at_program)) *logptr(giac_yyget_extra(scanner))<<"// End defining "<<$1<<endl;}}
	| T_NOT exp	{ $$ = symbolic(*$1._FUNCptr,$2);}
	| T_ARGS T_BEGIN_PAR exp T_END_PAR	{$$ = symb_args($3);}
	| T_ARGS T_INDEX_BEGIN exp T_VECT_END	{$$ = symb_args($3);}
	| T_ARGS { $$=symb_args(vecteur(0)); }
	| T_UNARY_OP T_BEGIN_PAR exp T_END_PAR	{
	gen tmp=python_compat(giac_yyget_extra(scanner))?denest_sto($3):$3;
	// CERR << python_compat(giac_yyget_extra(scanner)) << tmp << endl;
	$$ = symbolic(*$1._FUNCptr,tmp);
        const giac::context * contextptr = giac_yyget_extra(scanner);
	if (*$1._FUNCptr==at_maple_mode ||*$1._FUNCptr==at_xcas_mode ){
          xcas_mode(contextptr)=$3.val;
        }
	}
	| T_UNARY_OP_38 T_BEGIN_PAR exp T_END_PAR	{
	if ($3.type==_VECT && $3._VECTptr->empty())
          giac_yyerror(scanner,"void argument");
	$$ = symbolic(*$1._FUNCptr,python_compat(giac_yyget_extra(scanner))?denest_sto($3):$3);	
	}
	| T_UNARY_OP T_INDEX_BEGIN exp T_VECT_END { 
          const giac::context * contextptr = giac_yyget_extra(scanner);
          $$=symb_at($1,$3,contextptr);
        }
	| T_UNARY_OP T_BEGIN_PAR T_END_PAR	{
	$$ = symbolic(*$1._FUNCptr,gen(vecteur(0),_SEQ__VECT));
	if (*$1._FUNCptr==at_rpn)
          rpn_mode(giac_yyget_extra(scanner))=1;
	if (*$1._FUNCptr==at_alg)
          rpn_mode(giac_yyget_extra(scanner))=0;
	}
	| T_UNARY_OP {
	$$ = $1;
	}
	| exp T_PRIME	{$$ = symbolic(at_derive,$1);}
	| exp T_FACTORIAL { $$=symbolic(*$2._FUNCptr,$1); }
	| T_IF exp T_THEN bloc T_ELSE bloc {$$ = symbolic($1.type==_FUNC?*$1._FUNCptr:*at_ifte,makevecteur(equaltosame($2),symb_bloc($4),symb_bloc($6)));} 
 	| T_IF exp T_THEN bloc {$$ = symbolic($1.type==_FUNC?*$1._FUNCptr:*at_ifte,makevecteur(equaltosame($2),$4,0));} 
	| T_IF exp T_THEN prg_suite elif {
	$$ = symbolic($1.type==_FUNC?*$1._FUNCptr:*at_ifte,makevecteur(equaltosame($2),symb_bloc($4),$5));
	} 
	| T_IFTE T_BEGIN_PAR exp T_END_PAR	{$$ = symbolic($1.type==_FUNC?*$1._FUNCptr:*at_ifte,$3);}
	| T_IFTE {$$ = $1;}
	| exp T_MAPSTO bloc	{
          const giac::context * contextptr = giac_yyget_extra(scanner);
         $$ = symb_program($1,zero*$1,$3,contextptr);
        }
	| exp T_MAPSTO exp	{
          const giac::context * contextptr = giac_yyget_extra(scanner);
             if ($3.type==_VECT) 
                $$ = symb_program($1,zero*$1,symb_bloc(makevecteur(at_nop,$3)),contextptr); 
             else 
                $$ = symb_program($1,zero*$1,$3,contextptr);
		}
	| T_BLOC T_BEGIN_PAR exp T_END_PAR	{$$ = symb_bloc($3);}
	| T_BLOC {$$ = at_bloc;}
	| T_LOGO exp  { $$=symbolic(*$1._FUNCptr,$2); } 
	| T_LOGO {$$ = symbolic(*$1._FUNCptr,gen(vecteur(0),_SEQ__VECT));} 
	| T_LOGO T_BEGIN_PAR T_END_PAR  {$$ = symbolic(*$1._FUNCptr,gen(vecteur(0),_SEQ__VECT));}
	/* | T_RETURN T_BEGIN_PAR exp T_END_PAR	{$$ = symb_return($3);} */
	| T_RETURN exp  { $$=symbolic(*$1._FUNCptr,$2); } 
	/* | T_RETURN exp T_IF exp T_ELSE exp { $$=symbolic(*$1._FUNCptr,symb_ifte(equaltosame($4),$2,$6)); }  */
	| T_RETURN {$$ = gen(*$1._FUNCptr,0);} 
	| T_QUOTE T_RETURN T_QUOTE { $$=$2;}
	/* | T_RETURN T_SEMI {$$ = gen(*$1._FUNCptr,0);}  */
	| T_BREAK	{$$ = symbolic(at_break,zero);}
	| T_CONTINUE	{$$ = symbolic(at_continue,zero);}
	| T_FOR symbol_for T_IN exp T_DO prg_suite T_BLOC_END { 
          if ($7.type==_INT_ && $7.val && $7.val!=2 && $7.val!=9)
	    giac_yyerror(scanner,"missing loop end delimiter");
 	  bool rg=$4.is_symb_of_sommet(at_range);
          gen f=$4.type==_SYMB?$4._SYMBptr->feuille:0,inc=1;
          if (rg){
            if (f.type!=_VECT) f=makesequence(0,f);
            vecteur v=*f._VECTptr;
            if (v.size()>=2) f=makesequence(v.front(),v[1]-1);
            if (v.size()==3) inc=v[2];
          }
          if (inc.type==_INT_  && inc.val!=0 && f.type==_VECT && f._VECTptr->size()==2 && (rg || ($4.is_symb_of_sommet(at_interval) 
	  // && f._VECTptr->front().type==_INT_ && f._VECTptr->back().type==_INT_ 
	  )))
            $$=symbolic($1.type==_FUNC?*$1._FUNCptr:*at_for,makevecteur(symb_sto(f._VECTptr->front(),$2),inc.val>0?symb_inferieur_egal($2,f._VECTptr->back()):symb_superieur_egal($2,f._VECTptr->back()),symb_sto(symb_plus($2,inc),$2),symb_bloc($6)));
          else 
            $$=symbolic($1.type==_FUNC?*$1._FUNCptr:*at_for,makevecteur(1,symbolic(*$1._FUNCptr,makevecteur($2,$4)),1,symb_bloc($6)));
	  }
	| T_FOR symbol_for T_IN exp T_DO prg_suite T_ELSE prg_suite T_BLOC_END { 
          if ($9.type==_INT_ && $9.val && $9.val!=2 && $9.val!=9)
	    giac_yyerror(scanner,"missing loop end delimiter");
	  $$=symbolic($1.type==_FUNC?*$1._FUNCptr:*at_for,makevecteur(1,symbolic(*$1._FUNCptr,makevecteur($2,$4,symb_bloc($8))),1,symb_bloc($6)));
	  }
	| T_FOR symbol from T_TO exp step loop38_do prg_suite T_BLOC_END { 
          if ($9.type==_INT_ && $9.val && $9.val!=2 && $9.val!=9) giac_yyerror(scanner,"missing loop end delimiter");
          gen tmp,st=$6;  
       if (st==1 && $4!=1) st=$4;
          const giac::context * contextptr = giac_yyget_extra(scanner);
	  if (!lidnt(st).empty())
            *logptr(contextptr) << "Warning, step is not numeric " << st << std::endl;
          bool b=has_evalf(st,tmp,1,context0);
          if (!b || is_positive(tmp,context0)) 
             $$=symbolic($1.type==_FUNC?*$1._FUNCptr:*at_for,makevecteur(symb_sto($3,$2),symb_inferieur_egal($2,$5),symb_sto(symb_plus($2,b?abs(st,context0):symb_abs(st)),$2),symb_bloc($8))); 
          else 
            $$=symbolic($1.type==_FUNC?*$1._FUNCptr:*at_for,makevecteur(symb_sto($3,$2),symb_superieur_egal($2,$5),symb_sto(symb_plus($2,st),$2),symb_bloc($8))); 
        }
	| T_FOR {$$ = gen($1.type==_FUNC?*$1._FUNCptr:*at_for,4);}
	| T_REPEAT prg_suite T_UNTIL exp { 
        vecteur v=gen2vecteur($2);
        v.push_back(symb_ifte(equaltosame($4),symbolic(at_break,zero),0));
	$$=symbolic(*$1._FUNCptr,makevecteur(zero,1,zero,symb_bloc(v))); 
	}
	| T_REPEAT prg_suite T_UNTIL exp T_BLOC_END { 
        if ($5.type==_INT_ && $5.val && $5.val!=2 && $5.val!=9) giac_yyerror(scanner,"missing loop end delimiter");
        vecteur v=gen2vecteur($2);
        v.push_back(symb_ifte(equaltosame($4),symbolic(at_break,zero),0));
	$$=symbolic(*$1._FUNCptr,makevecteur(zero,1,zero,symb_bloc(v))); 
	}
	| T_IFERR prg_suite T_THEN prg_suite T_ELSE prg_suite T_BLOC_END {
          if ($7.type==_INT_ && $7.val && $7.val!=4) giac_yyerror(scanner,"missing iferr end delimiter");
           $$=symbolic(at_try_catch,makevecteur(symb_bloc($2),0,symb_bloc($4),symb_bloc($6)));
        }
	| T_IFERR prg_suite T_THEN prg_suite T_BLOC_END {
          if ($5.type==_INT_ && $5.val && $5.val!=4) giac_yyerror(scanner,"missing iferr end delimiter");
           $$=symbolic(at_try_catch,makevecteur(symb_bloc($2),0,symb_bloc($4),symb_bloc(0)));
        }
	| T_TYPE_ID { 
	$$=$1; 
	// $$.subtype=1; 
	}
	| T_QUOTE T_TYPE_ID T_QUOTE { $$=$2; /* $$.subtype=1; */ } 
	| T_DOLLAR_MAPLE exp { $$ = symb_dollar($2); } 
	| exp T_DOLLAR_MAPLE symbol T_IN exp {$$=symb_dollar(makesequence($1,$3,$5));}
	| exp T_DOLLAR_MAPLE exp { $$ = symb_dollar(makesequence($1,$3)); } 
	| exp T_DOLLAR exp { $$ = symb_dollar(makesequence($1,$3)); } 
	| T_DOLLAR T_SYMBOL { $$=symb_dollar($2); }
	| exp T_COMPOSE exp {  //CERR << $1 << " compose " << $2 << $3 << endl;
$$ = symbolic(*$2._FUNCptr,makesequence($1,python_compat(giac_yyget_extra(scanner))?denest_sto($3):$3) ); }
	| T_COMPOSE {$$=symbolic(at_ans,-1);}
	| exp T_UNION exp { $$ = symbolic(($2.type==_FUNC?*$2._FUNCptr:*at_union),makesequence($1,$3)); }
	| exp T_UNION exp T_MOD { $$ = symbolic(($2.type==_FUNC?*$2._FUNCptr:*at_union),gen(makevecteur($1,$1*$3/100) ,_SEQ__VECT)); }
	| exp T_INTERSECT exp { $$ = symb_intersect(makesequence($1,$3)); }
	| exp T_MINUS exp { $$ = symb_minus(makesequence($1,$3)); }
	| exp T_PIPE exp { 
	$$=symbolic(*$2._FUNCptr,makesequence($1,$3) ); 
	}
	| T_QUOTED_BINARY { $$ = $1; }
	| T_QUOTE exp T_QUOTE		{if ($2.type==_FUNC) $$=$2; else { 
          // const giac::context * contextptr = giac_yyget_extra(scanner);
          $$=symb_quote($2);
          } 
        }
	| exp T_INDEX_BEGIN exp T_VECT_END	{
          const giac::context * contextptr = giac_yyget_extra(scanner);
	  $$ = symb_at($1,$3,contextptr);
        }
	| exp T_INDEX_BEGIN T_VECT_DISPATCH exp T_VECT_END T_VECT_END	{
          const giac::context * contextptr = giac_yyget_extra(scanner);
	  $$ = symbolic(at_of,makesequence($1,$4));
        }
	| T_BEGIN_PAR exp T_END_PAR T_BEGIN_PAR suite T_END_PAR {$$ = check_symb_of($2,$5,giac_yyget_extra(scanner));}
	| T_BEGIN_PAR exp T_END_PAR		{
	if ($1==_LIST__VECT && python_compat(giac_yyget_extra(scanner))){
           $$=symbolic(at_python_list,$2);
        }
        else {
 	 if (abs_calc_mode(giac_yyget_extra(scanner))==38 && $2.type==_VECT && $2.subtype==_SEQ__VECT && $2._VECTptr->size()==2 && ($2._VECTptr->front().type<=_DOUBLE_ || $2._VECTptr->front().type==_FLOAT_) && ($2._VECTptr->back().type<=_DOUBLE_ || $2._VECTptr->back().type==_FLOAT_)){ 
           const giac::context * contextptr = giac_yyget_extra(scanner);
	   gen a=evalf($2._VECTptr->front(),1,contextptr),
	       b=evalf($2._VECTptr->back(),1,contextptr);
	   if ( (a.type==_DOUBLE_ || a.type==_FLOAT_) &&
                (b.type==_DOUBLE_ || b.type==_FLOAT_))
             $$= a+b*cst_i; 
           else $$=$2;
  	 } else {
              if (calc_mode(giac_yyget_extra(scanner))==1 && $2.type==_VECT && $1!=_LIST__VECT &&
	      $2.subtype==_SEQ__VECT && ($2._VECTptr->size()==2 || $2._VECTptr->size()==3) )
                $$ = gen(*$2._VECTptr,_GGB__VECT);
              else
                $$=$2;
          }
	 }
        } 
	| T_VECT_DISPATCH suite T_VECT_END { 
        //cerr << $1 << " " << $2 << endl;
        $$ = gen(*($2._VECTptr),$1.val);
        // cerr << $$ << endl;

        }
	| exp T_VIRGULE exp           { 
         if ($1.type==_VECT && $1.subtype==_SEQ__VECT && !($3.type==_VECT && $2.subtype==_SEQ__VECT)){ $$=$1; $$._VECTptr->push_back($3); }
	 else
           $$ = makesuite($1,$3); 

        }
	| T_HELP exp {$$=symb_findhelp($2);} 
	| exp T_INTERROGATION exp { $$=symb_interrogation($1,$3); }
	| exp T_SQ { $$=symb_pow($1,$2); }
	| error	{ 
        const giac::context * contextptr = giac_yyget_extra(scanner);
#ifdef HAVE_SIGNAL_H_OLD
	messages_to_print += parser_filename(contextptr) + parser_error(contextptr); 
	/* *logptr(giac_yyget_extra(scanner)) << messages_to_print; */
#endif
	$$=undef;
        spread_formula(false,contextptr); 
	}
	| T_IF T_BEGIN_PAR exp T_END_PAR bloc else {
	$$ = symbolic(*$1._FUNCptr,makevecteur(equaltosame($3),symb_bloc($5),$6));
	}
	| T_IF T_BEGIN_PAR exp T_END_PAR exp T_SEMI else {
        vecteur v=makevecteur(equaltosame($3),$5,$7);
	// *logptr(giac_yyget_extra(scanner)) << v << endl;
	$$ = symbolic(*$1._FUNCptr,v);
	}
	| T_PROC T_BEGIN_PAR suite T_END_PAR entete prg_suite T_BLOC_END { 
          if ($7.type==_INT_ && $7.val && $7.val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           $$=symb_program($3,zero*$3,symb_local($5,$6,contextptr),contextptr); 
        }
	| T_PROC symbol T_BEGIN_PAR suite T_END_PAR entete prg_suite T_BLOC_END { 
          if ($8.type==_INT_ && $8.val && $8.val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           $$=symb_program_sto($4,zero*$4,symb_local($6,$7,contextptr),$2,false,contextptr); 
        }
	| T_PROC symbol T_BEGIN_PAR suite T_END_PAR prg_suite T_BLOC_END { 
          if ($7.type==_INT_ && $7.val && $7.val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           $$=symb_program_sto($4,zero*$4,symb_bloc($6),$2,false,contextptr); 
        }
	| T_PROC symbol T_BEGIN_PAR suite T_END_PAR T_BLOC_BEGIN entete prg_suite T_BLOC_END { 
          if ($9.type==_INT_ && $9.val && $9.val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           $$=symb_program_sto($4,zero*$4,symb_local($7,$8,contextptr),$2,false,contextptr); 
        }
	| T_PROC T_BEGIN_PAR suite T_END_PAR entete T_BLOC_BEGIN prg_suite T_BLOC_END { 
          if ($8.type==_INT_ && $8.val && $8.val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
         $$=symb_program($3,zero*$3,symb_local($5,$7,contextptr),contextptr); 
        } 
	| symbol T_BEGIN_PAR suite T_END_PAR T_PROC entete prg_suite T_BLOC_END { 
          if ($8.type==_INT_ && $8.val && $8.val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           $$=symb_program_sto($3,zero*$3,symb_local($6,$7,contextptr),$1,false,contextptr); 
        }
	| symbol T_BEGIN_PAR suite T_END_PAR T_AFFECT T_PROC entete prg_suite T_BLOC_END { 
          if ($9.type==_INT_ && $9.val && $9.val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           $$=symb_program_sto($3,zero*$3,symb_local($7,$8,contextptr),$1,false,contextptr); 
        }
	| T_FOR T_BEGIN_PAR exp_or_empty T_SEMI exp_or_empty T_SEMI exp_or_empty T_END_PAR bloc  {$$ = symbolic($1.type==_FUNC?*$1._FUNCptr:*at_for,makevecteur($3,equaltosame($5),$7,symb_bloc($9)));}
	| T_FOR T_BEGIN_PAR exp_or_empty T_SEMI exp_or_empty T_SEMI exp_or_empty T_END_PAR exp T_SEMI  {$$ = symbolic($1.type==_FUNC?*$1._FUNCptr:*at_for,makevecteur($3,equaltosame($5),$7,$9));}
	| T_FOR T_BEGIN_PAR exp T_END_PAR	{$$ = symbolic($1.type==_FUNC?*$1._FUNCptr:*at_for,gen2vecteur($3));}
	| exp T_IN exp {$$=symbolic(at_member,makesequence($1,$3)); if ($2==at_not) $$=symbolic(at_not,$$);}
	| exp T_NOT T_IN exp {$$=symbolic(at_not,symbolic(at_member,makesequence($1,$4)));}
	| T_VECT_DISPATCH exp T_FOR suite_symbol T_IN exp T_VECT_END { $$=symbolic(at_apply,makesequence(symbolic(at_program,makesequence($4,0*$4,vecteur(1,$2))),$6)); if ($1==_TABLE__VECT) $$=symbolic(at_table,$$);}
	| T_VECT_DISPATCH exp T_FOR suite_symbol T_IN exp T_IF exp T_VECT_END { $$=symbolic(at_apply,symbolic(at_program,makesequence($4,0*$4,vecteur(1,$2))),symbolic(at_select,makesequence(symbolic(at_program,makesequence($4,0*$4,$8)),$6))); if ($1==_TABLE__VECT) $$=symbolic(at_table,$$);}
	| T_WHILE T_BEGIN_PAR exp T_END_PAR bloc { 
	vecteur v=makevecteur(zero,equaltosame($3),zero,symb_bloc($5));
	$$=symbolic($1.type==_FUNC?*$1._FUNCptr:*at_for,v); 
	}
	| T_WHILE T_BEGIN_PAR exp T_END_PAR exp T_SEMI { 
	$$=symbolic($1.type==_FUNC?*$1._FUNCptr:*at_for,makevecteur(zero,equaltosame($3),zero,$5)); 
	}
	| T_WHILE exp T_DO prg_suite T_BLOC_END { 
          if ($5.type==_INT_ && $5.val && $5.val!=9 && $5.val!=8) giac_yyerror(scanner,"missing loop end delimiter");
	  $$=symbolic($1.type==_FUNC?*$1._FUNCptr:*at_for,makevecteur(zero,equaltosame($2),zero,symb_bloc($4))); 
        }
	| T_TRY bloc T_CATCH T_BEGIN_PAR exp T_END_PAR bloc { $$=symb_try_catch(makevecteur(symb_bloc($2),$5,symb_bloc($7)));}
	| T_TRY_CATCH T_BEGIN_PAR exp T_END_PAR {$$=symb_try_catch(gen2vecteur($3));}
	| T_TRY_CATCH {$$=gen(at_try_catch,3);}
	| T_SWITCH T_BEGIN_PAR exp T_END_PAR T_BLOC_BEGIN switch T_BLOC_END { $$=symb_case($3,$6); }
	| T_CASE T_BEGIN_PAR T_SYMBOL T_END_PAR { $$ = symb_case($3); }
	| T_CASE exp case T_ENDCASE { $$=symb_case($2,$3); }
	;

symbol_for : T_SYMBOL { $$=$1; }
	| T_UNARY_OP { $$=$1; } 
	| T_UNARY_OP_38 { $$=$1; } 
	;

symbol	: T_SYMBOL { $$=$1; }
	| T_SYMBOL T_DOUBLE_DEUX_POINTS T_TYPE_ID { 
	       gen tmp($3); 
	       // tmp.subtype=1; 
	       $$=symb_check_type(makevecteur(tmp,$1),context0); 
          } 
	| T_SYMBOL T_DOUBLE_DEUX_POINTS T_UNARY_OP { $$=symb_double_deux_points(makevecteur($1,$3)); } 
	| T_SYMBOL T_DOUBLE_DEUX_POINTS T_SYMBOL { $$=symb_double_deux_points(makevecteur($1,$3)); } 
	| T_SYMBOL T_DOUBLE_DEUX_POINTS T_UNARY_OP_38 { $$=symb_double_deux_points(makevecteur($1,$3)); } 
	| T_SYMBOL T_DOUBLE_DEUX_POINTS T_QUOTE exp T_QUOTE %prec TI_HASH { $$=symb_double_deux_points(makevecteur($1,$4)); } 
	| T_DOUBLE_DEUX_POINTS T_SYMBOL { $$=symb_double_deux_points(makevecteur(0,$2)); } 
	| T_NUMBER T_DOUBLE_DEUX_POINTS T_SYMBOL { $$=symb_double_deux_points(makevecteur($1,$3)); } 
	/* | T_SYMBOL T_DOUBLE_DEUX_POINTS exp { 
        if ($3.type==_INT_ && $3.subtype==_INT_TYPE){
	   $$=symb_check_type(makevecteur($3,$1),context0); 
        }
        else
	  $$=symb_double_deux_points(makevecteur($1,$3)); 
	} */
	| T_TYPE_ID T_SYMBOL { 
	  gen tmp($1); 
	  // tmp.subtype=1; 
	  $$=symb_check_type(makevecteur(tmp,$2),context0); 
	  }
	| TI_HASH exp {$$=symbolic(*$1._FUNCptr,$2); }
	;

symbol_or_literal: T_SYMBOL { $$=$1; }
	| T_LITERAL { $$=$1; }
	;

entete	: /* empty */ { $$=makevecteur(vecteur(0),vecteur(0)); }
	| entete local	{ vecteur v1 =gen2vecteur($1); vecteur v2=gen2vecteur($2); $$=makevecteur(mergevecteur(gen2vecteur(v1[0]),gen2vecteur(v2[0])),mergevecteur(gen2vecteur(v1[1]),gen2vecteur(v2[1]))); }
	;


local	: T_LOCAL suite_symbol T_SEMI  { if (!$1.val) $$=makevecteur($2,vecteur(0)); else $$=makevecteur(vecteur(0),$2);}
	;

suite_symbol : affectable_symbol { $$=gen(vecteur(1,$1),_SEQ__VECT); }
	     | suite_symbol T_VIRGULE affectable_symbol { 
	       vecteur v=*$1._VECTptr;
	       v.push_back($3);
	       $$=gen(v,_SEQ__VECT);
	     }
	     ;

affectable_symbol : symbol { $$=$1; }
	     | T_SYMBOL T_AFFECT exp { $$=symb_sto($3,$1,$2==at_array_sto); }
	     | T_SYMBOL T_EQUAL exp { $$=symb_equal($1,$3); }
	     | T_SYMBOL T_DEUXPOINTS exp { $$=symbolic(at_deuxpoints,makesequence($1,$3));  }
	     | T_BEGIN_PAR affectable_symbol T_END_PAR { $$=$2; }
	     | T_UNARY_OP { $$=$1; *logptr(giac_yyget_extra(scanner)) << "Error: reserved word "<< $1 <<endl;}
	     | T_UNARY_OP T_DOUBLE_DEUX_POINTS exp { $$=symb_double_deux_points(makevecteur($1,$3)); *logptr(giac_yyget_extra(scanner)) << "Error: reserved word "<< $1 <<endl; }
	     | T_TYPE_ID { 
  const giac::context * contextptr = giac_yyget_extra(scanner);
  $$=string2gen("_"+$1.print(contextptr),false); 
  if (!giac::first_error_line(contextptr)){
    giac::first_error_line(giac::lexer_line_number(contextptr),contextptr);
    giac:: error_token_name($1.print(contextptr)+ " (reserved word)",contextptr);
  }
}
	     | T_NUMBER { 
  const giac::context * contextptr = giac_yyget_extra(scanner);
  $$=string2gen("_"+$1.print(contextptr),false);
  if (!giac::first_error_line(contextptr)){
    giac::first_error_line(giac::lexer_line_number(contextptr),contextptr);
    giac:: error_token_name($1.print(contextptr)+ " reserved word",contextptr);
  }
}
	     ;

exp_or_empty: /* empty */ { $$=plus_one;}
	| exp	{ $$=$1; }
	;

suite: /* empty */ { $$=gen(vecteur(0),_SEQ__VECT); }
       | exp { $$=makesuite($1); }
       ;

prg_suite: exp 	{ $$ = makevecteur($1); }
	/* | bloc { $$=makevecteur(symb_bloc($1)); } */
	| prg_suite exp	{ vecteur v(1,$1); 
			  if ($1.type==_VECT) v=*($1._VECTptr); 
			  v.push_back($2); 
			  $$ = v;
			}
	| prg_suite semi		{ $$ = $1;}
	;


step:	/* empty */ { $$=plus_one; }
	| T_BY exp { $$=$2; }
	;

from:	/* empty */ { $$=plus_one; }
	| T_AFFECT exp { $$=$2; }
	| T_EQUAL exp { $$=$2; }
	| T_FROM exp { $$=$2; }
	;

loop38_do: T_SEMI { $$=plus_one; }
	| T_DO { $$=$1; }
	;

else:	/* empty */ { $$=0; }
	| ti_else exp T_SEMI { $$=$2; }
	| ti_else bloc { $$=symb_bloc($2); }
	;

bloc	: T_BLOC_BEGIN prg_suite T_BLOC_END { 
	$$ = $2;
	}
	| T_BLOC_BEGIN entete prg_suite T_BLOC_END	{
          const giac::context * contextptr = giac_yyget_extra(scanner);
          $$ = symb_local($2,$3,contextptr);
         }
	/* | T_BLOC_BEGIN T_COMMENT local prg_suite T_BLOC_END	{ $$ = symb_local($3,$4,contextptr);} */
	;

elif:	 ti_bloc_end	{ if ($1.type==_INT_ && $1.val && $1.val!=4) giac_yyerror(scanner,"missing test end delimiter"); $$=0; }
	| ti_else prg_suite ti_bloc_end {
          if ($3.type==_INT_ && $3.val && $3.val!=4) giac_yyerror(scanner,"missing test end delimiter");
	$$=symb_bloc($2); 
	}
	| T_ELIF exp T_THEN prg_suite elif { 
	  $$=symb_ifte(equaltosame($2),symb_bloc($4),$5);
	  }
	;

ti_bloc_end:	T_BLOC_END { $$=$1; }
	     ;

ti_else:     T_ELSE { $$=0; }
	     ;

switch:	  /* empty */ { $$=vecteur(0); }
	| T_DEFAULT T_DEUXPOINTS bloc { $$=makevecteur(symb_bloc($3));}
	| T_CASE T_NUMBER T_DEUXPOINTS bloc switch { $$=mergevecteur(makevecteur($2,symb_bloc($4)),*($5._VECTptr));}
	;

case:	/* empty */ { $$=vecteur(0); }
	| T_DEFAULT prg_suite { $$=vecteur(1,symb_bloc($2)); }
	| T_OF T_NUMBER T_DO prg_suite case { $$=mergevecteur(makevecteur($2,symb_bloc($4)),*($5._VECTptr));}
	;

semi:	T_SEMI { $$=$1; }
	;

/*
 *  Routines
 */

%%

#ifndef NO_NAMESPACE_GIAC
} // namespace giac


#endif // ndef NO_NAMESPACE_GIAC
int giac_yyget_column  (yyscan_t yyscanner);

// Error print routine (store error string in parser_error)
#if 1
int giac_yyerror(yyscan_t scanner,const char *s) {
 const giac::context * contextptr = giac_yyget_extra(scanner);
 int col = giac_yyget_column(scanner);
 int line = giac::lexer_line_number(contextptr);
 const char * scanb=giac::currently_scanned(contextptr);
 std::string curline;
 if (scanb){
  for (int i=1;i<line;++i){
   for (;*scanb;++scanb){
     if (*scanb=='\n'){
       ++scanb;
       break;
     }
   }
  }
  const char * scane=scanb;
  for (;*scane;++scane){
    if (*scane=='\n') break;
  }
  curline=std::string (scanb,scane);
 }
 std::string token_name=string(giac_yyget_text(scanner));
 bool is_at_end = (token_name.size()==2 && (token_name[0]==char(0xC3)) && (token_name[1]==char(0xBF)));
 std::string suffix = " (reserved word)";
 if (token_name.size()>suffix.size() && token_name.compare(token_name.size()-suffix.size(),suffix.size(),suffix)) {
  if (col>=token_name.size()-suffix.size()) {
   col -= token_name.size()-suffix.size();
  }
 } else if (col>=token_name.size()) {
   col -= token_name.size();
 }
 giac::lexer_column_number(contextptr)=col;
 string sy("syntax error ");
 if (0 && strlen(s)){
   sy += ": ";
   sy += s;
   sy +=", ";
 }
 if (is_at_end) {
  parser_error(":" + giac::print_INT_(line) + ": " +sy + " at end of input\n",contextptr); // string(s) replaced with syntax error
  giac::parsed_gen(giac::undef,contextptr);
 } else {
 parser_error( ":" + giac::print_INT_(line) + ": " + sy + " line " + giac::print_INT_(line) + " col " + giac::print_INT_(col) + " at " + token_name +" in "+curline+" \n",contextptr); // string(s) replaced with syntax error
 giac::parsed_gen(giac::string2gen(token_name,false),contextptr);
 }
 if (!giac::first_error_line(contextptr)) {
  giac::first_error_line(line,contextptr);
  if (is_at_end) {
   token_name="end of input";
  }
  giac:: error_token_name(token_name,contextptr);
 }
 return line;
}

#else

int giac_yyerror(yyscan_t scanner,const char *s)
{
  const giac::context * contextptr = giac_yyget_extra(scanner);
  int col= giac_yyget_column(scanner);
  giac::lexer_column_number(contextptr)=col;
  if ( (*giac_yyget_text( scanner )) && (giac_yyget_text( scanner )[0]!=-61) && (giac_yyget_text( scanner )[1]!=-65)){
    std::string txt=giac_yyget_text( scanner );
    parser_error( ":" + giac::print_INT_(giac::lexer_line_number(contextptr)) + ": " + string(s) + " line " + giac::print_INT_(giac::lexer_line_number(contextptr)) + " col " + giac::print_INT_(col) + " at " + txt +"\n",contextptr);
     giac::parsed_gen(giac::string2gen(txt,false),contextptr);
  }
  else {
    parser_error(":" + giac::print_INT_(giac::lexer_line_number(contextptr)) + ": " +string(s) + " at end of input\n",contextptr);
    giac::parsed_gen(giac::undef,contextptr);
  }
  if (!giac::first_error_line(contextptr)){
    giac::first_error_line(giac::lexer_line_number(contextptr),contextptr);
    std::string s=string(giac_yyget_text( scanner ));
    if (s.size()==2 && s[0]==-61 && s[1]==-65)
      s="end of input";
    giac:: error_token_name(s,contextptr);
  }
  return giac::lexer_line_number(contextptr);
}
#endif
