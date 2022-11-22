 /* -*- mode: C++; compile-command: "flex input_lexer.ll && make input_lexer.o " -*- */
/* Note: for the nspire port, after flex, move from #ifdef HAVE_CONFIG_H 
   to #include "first.h" before #include<stdio.h> 
   and map "log" to log10 instead of ln
   // casio: copy lexer.h.bak to lexer.h
  // replace isatty( fileno(...) by isatty(0 in input_lexer.cc, replace getc( yyin ) by fgetc(yyin)
*/
/** @file input_lexer.ll
 *
 *  Lexical analyzer definition for reading expressions.
 *  Note Maple input should be processed replacing # with // and { } for set
 *  This file must be processed with flex. */

/*
 *  Copyright (C) 2001,14 B. Parisse, Institut Fourier, 38402 St Martin d'Heres
 *  The very first version was inspired by GiNaC lexer
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


/*
 * The lexer will first check for static patterns and strings (defined below)
 * If a match is not found, it calls find_or_make_symbol
 * This function looks first if the string should be translated
 * (e.g. add a prefix from the export table)
 * then look in lexer_functions for a match, then look in sym_tab
 * if not found in sym_tab, a new identificateur is created & added in sym_tab
 * Functions in lexer_functions are added during the construction
 * of the corresponding unary_functions using lexer_functions_register
 */


/*
 *  Definitions
 */

%{
#include "giacPCH.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <iostream>
#include <stdexcept>

#include "gen.h"
#include "input_lexer.h"
#include "help.h"
#include "identificateur.h"
#include "usual.h"
#include "derive.h"
#include "series.h"
#include "intg.h"
#include "sym2poly.h"
#include "moyal.h"
#include "subst.h"
#include "vecteur.h"
#include "modpoly.h"
#include "lin.h"
#include "solve.h"
#include "ifactor.h"
#include "alg_ext.h"
#include "gauss.h"
#include "isom.h"
#include "plot.h"
#include "ti89.h"

#include "prog.h"
#include "rpn.h"
#include "ezgcd.h"
#include "tex.h"
#include "risch.h"
#include "permu.h"
#include "input_parser.h"    

#if defined(RTOS_THREADX) || defined(__MINGW_H) || defined NSPIRE || defined FXCG || defined MS_SMART || defined(FREERTOS)
  int isatty (int ){ return 0; }
#endif


  using namespace std;
  using namespace giac;
  void giac_yyset_column (int  column_no , yyscan_t yyscanner);
  int giac_yyget_column (yyscan_t yyscanner);
#define YY_USER_ACTION giac_yyset_column(giac_yyget_column(yyscanner)+yyleng,yyscanner);
#define YY_USER_INIT giac_yyset_column(1,yyscanner);

#ifndef NO_NAMESPACE_GIAC
  namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

    void increment_lexer_line_number_setcol(yyscan_t yyscanner,GIAC_CONTEXT){
      giac_yyset_column(1,yyscanner);
      increment_lexer_line_number(contextptr);
    }
    bool doing_insmod = false;

    int lock_syms_mutex(){ return 0; }
    void unlock_syms_mutex(){}

    sym_string_tab & syms(){
      static sym_string_tab * ans=0;
      if (!ans) ans=new sym_string_tab;
      return * ans;
    }


    bool tri1(const lexer_tab_int_type & a,const lexer_tab_int_type & b){
      int res= strcmp(a.keyword,b.keyword);
      return res<0;
    }

    bool tri2(const char * a,const char * b){
      return strcmp(a,b)<0;
    }

    const lexer_tab_int_type lexer_tab_int_values []={
#include "lexer_tab_int.h"
    };

    const lexer_tab_int_type * const lexer_tab_int_values_begin = lexer_tab_int_values;
    const unsigned lexer_tab_int_values_n=sizeof(lexer_tab_int_values)/sizeof(lexer_tab_int_type);
    const lexer_tab_int_type * const lexer_tab_int_values_end = lexer_tab_int_values+lexer_tab_int_values_n;
#ifndef NO_NAMESPACE_GIAC
  } // namespace giac
#endif // ndef NO_NAMESPACE_GIAC

%}

%option reentrant bison-bridge
%option outfile="input_lexer.cc"
%option header-file="lexer.h"
%option noyywrap
%option prefix="giac_yy"

	/* Abbreviations */
D	[0-9]
E	[eE][eE]?[-+]?{D}+
A	[a-zA-Z\200-\355\357-\376] 
AN	[0-9a-zA-Z_~ ?\200-\355\357-\376] 
        /* If changed, modify isalphan in help.cc FIXME is . allowed inside alphanumeric ? answer NO */
%x comment
%x comment_hash
%x str
%x backquote
/*
 *  Lexical rules
 */

%%

[ \t\\]+			/* skip whitespace */
\n                increment_lexer_line_number_setcol(yyscanner,yyextra); //CERR << "Scanning line " << lexer_line_number(yyextra) << endl;
  /* Strings */
  /* \"[^\"]*\"        yylval = string2gen( giac_yytext); return T_STRING; */
\"                BEGIN(str); comment_s("",yyextra);
<str>\"\"         increment_comment_s('"',yyextra);
<str>\"        {  index_status(yyextra)=1; BEGIN(INITIAL); 
                  (*yylval)=string2gen(comment_s(yyextra),false); 
                  return T_STRING; }
<str>\n        increment_comment_s('\n',yyextra); increment_lexer_line_number_setcol(yyscanner,yyextra);
<str>\\[0-7]{1,3} {
                   /* octal escape sequence */
                   int result=0;
                   //(void) sscanf( yytext + 1, "%o", &result );
                   increment_comment_s(char(result & 0xff),yyextra);
                   }
<str>\\[0-9]+      {
                   /* generate error - bad escape sequence; something
                    * like '\48' or '\0777777'
                    */
                   }
<str>\\n  increment_comment_s('\n',yyextra);
<str>\\t  increment_comment_s('\t',yyextra);
<str>\\r  increment_comment_s('\r',yyextra);
<str>\\b  increment_comment_s('\b',yyextra);
<str>\\f  increment_comment_s('\f',yyextra);
<str>\\(.|\n)  increment_comment_s(yytext[1],yyextra);
<str>[^\\\n\"]+ increment_comment_s(yytext,yyextra);
`                   BEGIN(backquote); comment_s("",yyextra); 
<backquote>\n      increment_comment_s('\n',yyextra); increment_lexer_line_number_setcol(yyscanner,yyextra);
<backquote>[^\n`]+       increment_comment_s(yytext,yyextra);
<backquote>`       {  index_status(yyextra)=1; BEGIN(INITIAL); 
  return find_or_make_symbol(comment_s(yyextra),(*yylval),yyscanner,true,yyextra); }

"://"[^\n]*\n      index_status(yyextra)=0; increment_lexer_line_number_setcol(yyscanner,yyextra);
"//"[^\n]*\n      index_status(yyextra)=0; increment_lexer_line_number_setcol(yyscanner,yyextra);/* (*yylval) = string2gen('"'+string(giac_yytext).substr(2,string(giac_yytext).size()-3)+'"');   return T_COMMENT; */
"/*"              BEGIN(comment); comment_s(yyextra)="";

<comment>[^*\n]*        comment_s(yyextra)+=yytext; /* eat anything that's not a '*' */
<comment>"*"+[^*/\n]*   comment_s(yyextra)+=yytext; /* eat up '*'s not followed by '/'s */
<comment>\n             comment_s(yyextra) += '\n'; increment_lexer_line_number_setcol(yyscanner,yyextra); CERR << "(Comment) scanning line " << lexer_line_number(yyextra) << endl;
<comment>"*"+"/"        BEGIN(INITIAL); index_status(yyextra)=0; /* (*yylval) = string2gen(comment_s(yyextra),false); return T_COMMENT; */
"#++"[^*]*"++#"         index_status(yyextra)=0; /* (*yylval) = string2gen('"'+string(yytext).substr(3,string(yytext).size()-6)+'"'); return T_COMMENT; */
"#--"[^*]*"--#"         index_status(yyextra)=0; /* (*yylval) = string2gen('"'+string(yytext).substr(3,string(yytext).size()-6)+'"'); return T_COMMENT; */

"?"                     if (index_status(yyextra)) return T_INTERROGATION; if (calc_mode(yyextra)==1){ *yylval=undef; return T_SYMBOL;}  return T_HELP;
"'"                     if (opened_quote(yyextra) & 1) { opened_quote(yyextra) &= 0x7ffffffe; return T_QUOTE; } if (index_status(yyextra) && !in_rpn(yyextra) && xcas_mode(yyextra)!= 1) return T_PRIME; opened_quote(yyextra) |= 1; return T_QUOTE;
";"			index_status(yyextra)=0; (*yylval)=0; return T_SEMI;
  /* commented otherwise for(;;) will not work ";;"			index_status(yyextra)=0; if (xcas_mode(yyextra)==3) return TI_SEMI; (*yylval)=0; return T_SEMI; */
"§"                  index_status(yyextra)=0; return T_SEMI; 
":"			if (spread_formula(yyextra)) return T_DEUXPOINTS;  index_status(yyextra)=0; if (xcas_mode(yyextra)>0) { (*yylval)=1; return T_SEMI; } else return T_DEUXPOINTS;
":;"                    index_status(yyextra)=0; (*yylval)=1; return T_SEMI;
"::"                    index_status(yyextra)=0;return T_DOUBLE_DEUX_POINTS;

			/* special values */

"θ"	index_status(yyextra)=1; (*yylval)=theta__IDNT_e; return T_SYMBOL;
"i"			index_status(yyextra)=1; if (xcas_mode(yyextra) > 0 || !i_sqrt_minus1(yyextra)) { (*yylval)=i__IDNT_e; return T_SYMBOL; } else { (*yylval) = cst_i; return T_LITERAL;};
"ί"                      index_status(yyextra)=1; (*yylval) = cst_i; return T_LITERAL;
""                      index_status(yyextra)=1; (*yylval) = cst_i; return T_LITERAL;
\xa1                    index_status(yyextra)=1; (*yylval) = cst_i; return T_LITERAL;
  /* \xef\xbd\x89            index_status(yyextra)=1; (*yylval) = cst_i; return T_LITERAL; */
"I"                     index_status(yyextra)=1; if (xcas_mode(yyextra)==0 || xcas_mode(yyextra)==3 || rpn_mode(yyextra)) { return find_or_make_symbol(yytext,(*yylval),yyscanner,true,yyextra); } else { (*yylval) = cst_i; return T_LITERAL; };
"pi"			index_status(yyextra)=1; (*yylval) = cst_pi; return T_LITERAL;
"π"			index_status(yyextra)=1; (*yylval) = cst_pi; return T_LITERAL;
"Pi"			index_status(yyextra)=1; (*yylval) = cst_pi; return T_LITERAL;
"PI"			index_status(yyextra)=1; (*yylval) = cst_pi; return T_LITERAL;
"euler_gamma"		index_status(yyextra)=1; (*yylval) = cst_euler_gamma; return T_LITERAL;
"infinity"		index_status(yyextra)=1; (*yylval) = unsigned_inf; return T_LITERAL;
"oo"		index_status(yyextra)=1; (*yylval) = plus_inf; return T_LITERAL;
"inf"		index_status(yyextra)=1; (*yylval) = plus_inf; return T_LITERAL;
"unsigned_inf"		index_status(yyextra)=1; (*yylval) = unsigned_inf; return T_LITERAL;
"plus_inf"		index_status(yyextra)=1; (*yylval) = plus_inf; return T_LITERAL;
"minus_inf"		index_status(yyextra)=1; (*yylval) = minus_inf; return T_LITERAL;
"undef"		        index_status(yyextra)=1; (*yylval) = undef; return T_LITERAL;
"ÿ"                     return T_END_INPUT;

               /* integer values */
"list"         if (python_compat(yyextra)){ *yylval=at_python_list; return T_UNARY_OP; } if (xcas_mode(yyextra)==3) { index_status(yyextra)=1; return find_or_make_symbol(yytext,(*yylval),yyscanner,true,yyextra); } index_status(yyextra)=0; (*yylval) = _MAPLE_LIST ; (*yylval).subtype=_INT_MAPLECONVERSION ;return T_TYPE_ID;


    /* vector/polynom/matrice delimiters */
"seq["              (*yylval) = _SEQ__VECT; return T_VECT_DISPATCH;
"set["              (*yylval) = _SET__VECT; return T_VECT_DISPATCH;
"i["              (*yylval) = _INTERVAL__VECT; return T_VECT_DISPATCH;
"list[" index_status(yyextra)=0; (*yylval) = _LIST__VECT; return T_VECT_DISPATCH;
"list(" index_status(yyextra)=0; (*yylval) = _LIST__VECT; return T_BEGIN_PAR;
"rpn_func["         (*yylval) = _RPN_FUNC__VECT; return T_VECT_DISPATCH;
"group["            (*yylval) = _GROUP__VECT; return T_VECT_DISPATCH;
"line["             (*yylval) = _LINE__VECT; return T_VECT_DISPATCH;
"vector["           (*yylval) = _VECTOR__VECT; return T_VECT_DISPATCH;
"matrix["           (*yylval) = _MATRIX__VECT; return T_VECT_DISPATCH;
"pnt["              (*yylval) = _PNT__VECT; return T_VECT_DISPATCH;
"point["            (*yylval) = _POINT__VECT; return T_VECT_DISPATCH;
"tuple["            (*yylval) = _TUPLE__VECT; return T_VECT_DISPATCH;
"curve["            (*yylval) = _CURVE__VECT; return T_VECT_DISPATCH;
"halfline["         (*yylval) = _HALFLINE__VECT; return T_VECT_DISPATCH;
"poly1["            (*yylval) = _POLY1__VECT; return T_VECT_DISPATCH;
"assume["           (*yylval) = _ASSUME__VECT; return T_VECT_DISPATCH;
"logo["      (*yylval) = _LOGO__VECT; return T_VECT_DISPATCH;
"folder["      (*yylval) = _FOLDER__VECT; return T_VECT_DISPATCH;
"polyedre["      (*yylval) = _POLYEDRE__VECT; return T_VECT_DISPATCH;
"rgba["      (*yylval) = _RGBA__VECT; return T_VECT_DISPATCH;
"â¦" index_status(yyextra)=0; (*yylval) = _LIST__VECT; return T_VECT_DISPATCH;
"â¦" index_status(yyextra)=1; return T_VECT_END;
"<"                     index_status(yyextra)=0; (*yylval)=gen(at_inferieur_strict,2);  return T_TEST_EQUAL;
">"                     index_status(yyextra)=0; (*yylval)=gen(at_superieur_strict,2); return T_TEST_EQUAL;
"<<"                    index_status(yyextra)=0; (*yylval)=gen(at_rotate,2); return T_UNION; 
">>"                    index_status(yyextra)=0; (*yylval)=gen(at_shift,2); return T_UNION; 
"<<="                    index_status(yyextra)=0; (*yylval)=gen("rotatesto",0); return T_UNION; 
">>="                    index_status(yyextra)=0; (*yylval)=gen("shiftsto",0); return T_UNION; 
","                     index_status(yyextra)=0; return T_VIRGULE;
",,"                     index_status(yyextra)=0; return T_VIRGULE;
"("                     index_status(yyextra)=0; *yylval = 0; return T_BEGIN_PAR;
")"                     index_status(yyextra)=1; return T_END_PAR;
\[			if (index_status(yyextra)) { index_status(yyextra)=0; return T_INDEX_BEGIN; } else { (*yylval) = 0; return T_VECT_DISPATCH; } ;
\]			index_status(yyextra)=1; return T_VECT_END;
",]"                    index_status(yyextra)=1; return T_VECT_END;
"%["                    index_status(yyextra)=0; (*yylval) = _POLY1__VECT; return T_VECT_DISPATCH; 
"%]"                    index_status(yyextra)=1; return T_VECT_END;
"%%["                   index_status(yyextra)=0; (*yylval) = _MATRIX__VECT; return T_VECT_DISPATCH; 
"%%]"                   index_status(yyextra)=1; return T_VECT_END;
"%%%["                  index_status(yyextra)=0; (*yylval) = _ASSUME__VECT; return T_VECT_DISPATCH; 
"%%%]"                  index_status(yyextra)=1; return T_VECT_END;
    /* geometric delimiters */
"%_("                   index_status(yyextra)=0; (*yylval) = _GROUP__VECT; return T_VECT_DISPATCH;
"%_)"                    index_status(yyextra)=1; return T_VECT_END;
"%%("                   index_status(yyextra)=0; (*yylval) = _LINE__VECT; return T_VECT_DISPATCH; 
"%%)"                   index_status(yyextra)=1; return T_VECT_END;
"%%%("                  index_status(yyextra)=0; (*yylval) = _VECTOR__VECT; return T_VECT_DISPATCH; 
"%%%)"                  index_status(yyextra)=1; return T_VECT_END;
"%%%%("                 index_status(yyextra)=0; (*yylval) = _CURVE__VECT; return T_VECT_DISPATCH; 
"%%%%)"                 index_status(yyextra)=1; return T_VECT_END;
    /* gen delimiters */
"{/"                     index_status(yyextra)=0; (*yylval)=_TABLE__VECT;return T_VECT_DISPATCH; 
"/}"                     index_status(yyextra)=1;  return T_VECT_END;
"{"                     index_status(yyextra)=0;  if (rpn_mode(yyextra)||calc_mode(yyextra)==1) { (*yylval)=0; return T_VECT_DISPATCH; } if (xcas_mode(yyextra)==3 || abs_calc_mode(yyextra)==38){ (*yylval) = _LIST__VECT;  return T_VECT_DISPATCH; } if (xcas_mode(yyextra) > 0 ){ (*yylval)=_SET__VECT; return T_VECT_DISPATCH; } else return T_BLOC_BEGIN;
"}"                     index_status(yyextra)=1; if (rpn_mode(yyextra) || calc_mode(yyextra)==1 || python_compat(yyextra)) return T_VECT_END; if (xcas_mode(yyextra)==3 || abs_calc_mode(yyextra)==38) return T_VECT_END; if (xcas_mode(yyextra) > 0) return T_VECT_END; else return T_BLOC_END;
"%{"                    index_status(yyextra)=0;  (*yylval)=_SET__VECT; return T_VECT_DISPATCH;
"%}"                    index_status(yyextra)=1; return T_VECT_END;
"%%{"                   index_status(yyextra)=0; return T_ROOTOF_BEGIN;
"%%}"                   index_status(yyextra)=1; return T_ROOTOF_END;
"%%%{"                  index_status(yyextra)=0; return T_SPOLY1_BEGIN;
"%%%}"                  index_status(yyextra)=1; return T_SPOLY1_END;

    /* binary operators */
"->"                    index_status(yyextra)=0; return T_MAPSTO;
"=="			index_status(yyextra)=0; (*yylval)=gen(at_same,2); return T_TEST_EQUAL;
"==="			index_status(yyextra)=0; (*yylval)=gen(at_equal,2); return T_EQUAL;
"-/-"			index_status(yyextra)=0; (*yylval)=gen(at_deuxpoints,2); return T_DEUXPOINTS;
"!="			index_status(yyextra)=0; (*yylval)=gen(at_different,2); return T_TEST_EQUAL;
"<>"			index_status(yyextra)=0; (*yylval)=gen(at_different,2); return T_TEST_EQUAL;
"<="			index_status(yyextra)=0; (*yylval)=gen(at_inferieur_egal,2); return T_TEST_EQUAL;
">="			index_status(yyextra)=0; (*yylval)=gen(at_superieur_egal,2); return T_TEST_EQUAL;
"="                     spread_formula(yyextra)=!index_status(yyextra); index_status(yyextra)=0; (*yylval)=gen(at_equal,2); return T_EQUAL;
"%="                     spread_formula(yyextra)=!index_status(yyextra); index_status(yyextra)=0; (*yylval)=gen(at_equal2,2); return T_EQUAL;
"$"                     index_status(yyextra)=0; (*yylval)=gen(at_dollar,2); if (xcas_mode(yyextra)>0) return T_DOLLAR_MAPLE; else return T_DOLLAR;
"%$"                   index_status(yyextra)=0; (*yylval)=gen(at_dollar,2); return T_DOLLAR_MAPLE;
":="			index_status(yyextra)=0; (*yylval)=gen(at_sto,2); return T_AFFECT;
"►"                    index_status(yyextra)=0; (*yylval)=gen(at_sto,2); return TI_STO;
"▶"                index_status(yyextra)=0; (*yylval)=gen(at_sto,2); return TI_STO;
"→"                    index_status(yyextra)=0; if (xcas_mode(yyextra)==3){ (*yylval)=gen(at_sto,2); return TI_STO; } else return T_MAPSTO;
"=>"                    index_status(yyextra)=0; (*yylval)=gen(at_sto,2); return TI_STO;
"=%"                    index_status(yyextra)=0; (*yylval)=gen(at_sto,2); return TI_STO;
"=<"                    index_status(yyextra)=0; (*yylval)=gen(at_array_sto,2); return T_AFFECT;
"@"{D}+                   index_status(yyextra)=1; yytext[0]='0'; (*yylval) = symb_double_deux_points(makevecteur(_IDNT_id_at,chartab2gen(yytext,yyextra))); return T_SYMBOL;
"@"                     if (xcas_mode(yyextra)!=3) {index_status(yyextra)=0; (*yylval)=gen(at_compose,2); return T_COMPOSE; } BEGIN(comment_hash);
"@@"                     index_status(yyextra)=0; (*yylval)=gen(at_composepow,2); return T_POW;
"&&"                    index_status(yyextra)=0; (*yylval)=gen(at_and,2); return T_AND_OP;
"|"                     index_status(yyextra)=0; (*yylval)=gen(at_tilocal,2); return T_PIPE;
"||"                    index_status(yyextra)=0; (*yylval)=gen(at_ou,2); return T_AND_OP;
"or"                    index_status(yyextra)=0; (*yylval)=gen(at_ou,2); return T_AND_OP;
"and"                    index_status(yyextra)=0; (*yylval)=gen(at_and,2); return T_AND_OP;
"xor"                    index_status(yyextra)=0; (*yylval)=gen(at_xor,2); return T_AND_OP;
".."                    index_status(yyextra)=0; (*yylval)=gen(at_interval,2); return T_INTERVAL;
"..."                    index_status(yyextra)=0; (*yylval)=gen(at_interval,2); return T_INTERVAL;
"!"                     if (xcas_mode(yyextra) || index_status(yyextra)) { (*yylval)=gen(at_factorial); return T_FACTORIAL; } else { index_status(yyextra)=0; (*yylval)=gen(at_not,1); return T_NOT; }

    /* standard functions */
"Ans"                   index_status(yyextra)=1; (*yylval)=symbolic(at_ans,0); return T_LITERAL;
"+"                     index_status(yyextra)=0; (*yylval)=gen(at_plus,2); return T_PLUS;
"++"                    index_status(yyextra)=0; (*yylval)=gen(at_increment,1); return T_FACTORIAL;
"+="                    index_status(yyextra)=0; (*yylval)=gen(at_increment,1); return T_UNION;
"augmente_de"                    index_status(yyextra)=0; (*yylval)=gen(at_increment,1); return T_UNION;
"**="                    index_status(yyextra)=0; (*yylval)=gen("powsto",0); return T_UNION;
"est_eleve_puissance"                    index_status(yyextra)=0; (*yylval)=gen("powsto",0); return T_UNION;
"--"                    index_status(yyextra)=0; (*yylval)=gen(at_decrement,1); return T_FACTORIAL;
"-="                    index_status(yyextra)=0; (*yylval)=gen(at_decrement,1); return T_UNION;
"diminue_de"                    index_status(yyextra)=0; (*yylval)=gen(at_decrement,1); return T_UNION;
".+"                    index_status(yyextra)=0; (*yylval)=gen(at_pointplus,2); return T_PLUS;
"&"                     index_status(yyextra)=0; if (python_compat(yyextra)) { (*yylval)=gen(at_bitand,2); return T_AND_OP; } else { *yylval=gen(at_plus,2); return T_PLUS; }
"~"                     index_status(yyextra)=0; (*yylval)=gen(at_bitnot,1); return T_NOT;
"√"                     index_status(yyextra)=0; (*yylval)=gen(at_sqrt,2); return T_NOT;
"\xE6\xBC"                     index_status(yyextra)=0; (*yylval)=gen(at_polar_complex,2); return T_MOD;
"²"                     index_status(yyextra)=1; (*yylval)=2; return T_SQ;
  /* "','"                   index_status(yyextra)=0; (*yylval)=gen(at_makevector,2); return T_QUOTED_BINARY; commented because of f('a','b') */
"'+'"                   index_status(yyextra)=0; (*yylval)=gen(at_plus,2); return T_QUOTED_BINARY;
"-"                     index_status(yyextra)=0; (*yylval)=gen(at_binary_minus,2); return T_PLUS; // return (calc_mode(yyextra)==38)?T_MOINS38:T_MOINS;
"−"                     index_status(yyextra)=0; (*yylval)=gen(at_binary_minus,2); return T_PLUS;
".-"                     index_status(yyextra)=0; (*yylval)=gen(at_pointminus,2); return T_PLUS;
"'-'"                   index_status(yyextra)=0; (*yylval)=gen(at_binary_minus,2); return T_QUOTED_BINARY;
"*"                     index_status(yyextra)=0; (*yylval)=gen(at_prod,2); return T_DIV;
"*="                    index_status(yyextra)=0; (*yylval)=gen(at_multcrement,1); return T_UNION;
"est_multiplie_par"                    index_status(yyextra)=0; (*yylval)=gen(at_multcrement,1); return T_UNION;
"."                     index_status(yyextra)=0; if (abs_calc_mode(yyextra)==38){return T_DOUBLE_DEUX_POINTS; } else {(*yylval)=gen(at_struct_dot,2); return T_COMPOSE;}
"&*"                     index_status(yyextra)=0; (*yylval)=gen(at_ampersand_times,2); return T_DIV;
"&^"                     index_status(yyextra)=0; (*yylval)=gen(at_quote_pow,2); return T_POW;
".*"                     index_status(yyextra)=0; (*yylval)=gen(at_pointprod,2); return T_DIV;
"'*'"                   index_status(yyextra)=0; (*yylval)=gen(at_prod,2); return T_QUOTED_BINARY;
"/"                     index_status(yyextra)=0; (*yylval)=gen(at_division,2); return T_DIV;
"/%"                     index_status(yyextra)=0; (*yylval)=gen(at_iquo,2); return T_DIV;
"%/"                     index_status(yyextra)=0; (*yylval)=gen(at_irem,2); return T_DIV;
"/%="                     index_status(yyextra)=0; (*yylval)=gen("iquosto",0); return T_UNION;
"%/="                     index_status(yyextra)=0; (*yylval)=gen("iremsto",0); return T_UNION;
"&="                     index_status(yyextra)=0; (*yylval)=gen("andsto",0); return T_UNION;
"|="                     index_status(yyextra)=0; (*yylval)=gen("orsto",0); return T_UNION;
"^="                     index_status(yyextra)=0; (*yylval)=gen("xorsto",0); return T_UNION;
"/="                    index_status(yyextra)=0; (*yylval)=gen(at_divcrement,1); return T_DIV;
"est_divise_par"                    index_status(yyextra)=0; (*yylval)=gen(at_divcrement,1); return T_UNION;
"./"                     index_status(yyextra)=0; (*yylval)=gen(at_pointdivision,2); return T_DIV;
"%"                     index_status(yyextra)=0; if (xcas_mode(yyextra)==3 || calc_mode(yyextra)==1) { (*yylval)=gen(at_pourcent); return T_FACTORIAL; } if (xcas_mode(yyextra)==1) { (*yylval)=symbolic(at_ans,vecteur(0)); return T_NUMBER; }  if (xcas_mode(yyextra) || python_compat(yyextra)) (*yylval)=gen(at_irem,2); else (*yylval)=0; return T_MOD;
"%%"                     index_status(yyextra)=0; if (xcas_mode(yyextra)==0){ (*yylval)=gen(at_iquorem,2); return T_MOD;} (*yylval)=symbolic(at_ans,-2); return T_NUMBER; 
  /* \xe2\x88\xa1             index_status(yyextra)=0; (*yylval)=gen(at_polar_complex,2); return T_MOD; */
"%%%"                     if (xcas_mode(yyextra)==0){ (*yylval)=gen(at_quorem,2); return T_MOD;} index_status(yyextra)=0; (*yylval)=symbolic(at_ans,-3); return T_NUMBER; 
"mod"                   index_status(yyextra)=0; if (xcas_mode(yyextra)==3) { (*yylval)=gen(at_irem,2); return T_UNARY_OP; } else { if (xcas_mode(yyextra)) (*yylval)=gen(at_irem,2); else (*yylval)=0; return T_MOD; }
  /* "MOD"                   index_status(yyextra)=0; return T_MOD; */
"^"                     index_status(yyextra)=0; (*yylval)=gen(python_compat(yyextra)==2?at_bitxor:at_pow,2); return T_POW;
"^*"                     index_status(yyextra)=0; (*yylval)=gen(at_trn,1); return T_FACTORIAL;
"**"                     index_status(yyextra)=0; (*yylval)=gen(at_pow,2); return T_POW;
".^"                     index_status(yyextra)=0; (*yylval)=gen(at_pointpow,2); return T_POW;
"Digits"                  (*yylval) = gen(at_Digits,0); index_status(yyextra)=0; return T_DIGITS;
"scientific_format"		        (*yylval) = gen(at_scientific_format,0); index_status(yyextra)=0; return T_DIGITS;
"angle_radian"		(*yylval) = gen(at_angle_radian,0); index_status(yyextra)=0; return T_DIGITS;
"approx_mode"		(*yylval) = gen(at_approx_mode,0); index_status(yyextra)=0; return T_DIGITS;
"all_trig_solutions"		(*yylval) = gen(at_all_trig_solutions,1); index_status(yyextra)=0; return T_DIGITS;
"complex_mode"		(*yylval) = gen(at_complex_mode,1); index_status(yyextra)=0; return T_DIGITS;
"keep_algext"		(*yylval) = gen(at_keep_algext,1); index_status(yyextra)=0; return T_DIGITS;
"complex_variables"	(*yylval) = gen(at_complex_variables,0); index_status(yyextra)=0; return T_DIGITS;
"epsilon"               (*yylval) = gen(at_epsilon,0); index_status(yyextra)=0; return T_DIGITS;
"proba_epsilon"               (*yylval) = gen(at_proba_epsilon,0); index_status(yyextra)=0; return T_DIGITS;

"randnorm"		(*yylval) = gen(at_randNorm,1); index_status(yyextra)=0; return T_UNARY_OP;
"'args'"                index_status(yyextra)=0; (*yylval)=gen(at_args,0); return T_QUOTED_BINARY;
"at"			(*yylval) = gen(at_at,2); index_status(yyextra)=0; return T_UNARY_OP;
"bloc"  		(*yylval) = gen(at_bloc,1); index_status(yyextra)=0; return T_UNARY_OP;
"D"  		if (xcas_mode(yyextra)==1 || xcas_mode(yyextra)==2) { (*yylval) = gen(at_function_diff,1); index_status(yyextra)=1; return T_UNARY_OP;} else { index_status(yyextra)=1; return find_or_make_symbol(yytext,(*yylval),yyscanner,true,yyextra); }
"e"                     if (xcas_mode(yyextra)==1 || xcas_mode(yyextra)==2) { (*yylval)=e__IDNT_e; }else (*yylval)=symbolic(at_exp,1); index_status(yyextra)=1; return T_NUMBER;
"ℯ"                     (*yylval)=symbolic(at_exp,1); index_status(yyextra)=1; return T_NUMBER;
"equal"			(*yylval) = gen(at_equal,2); index_status(yyextra)=0; return T_UNARY_OP;
"error"		        index_status(yyextra)=0; (*yylval)=gen(at_throw,1); return T_RETURN;
"for"			index_status(yyextra)=0; (*yylval)=gen(at_for,4); return T_FOR;
"if"      		index_status(yyextra)=0; (*yylval)=gen(at_ifte,3); return T_IF;
"ifte"      		index_status(yyextra)=0; (*yylval)=gen(at_ifte,3); return T_IFTE;
"log"			(*yylval) = gen(at_ln,1); index_status(yyextra)=1; return T_UNARY_OP; /* index_status(yyextra)=1 to accept log[] for a basis log */
"not"                 (*yylval) = gen(at_not,1); if (xcas_mode(yyextra) || python_compat(yyextra)) return T_NOT;  index_status(yyextra)=0; return T_UNARY_OP;
"not in"                 (*yylval) = gen(at_not,1); return T_IN;  
"neg"		(*yylval) = gen(at_neg,1); index_status(yyextra)=0; return T_UNARY_OP;
"op"                     (*yylval) = gen(at_feuille,1); index_status(yyextra)=0; return T_UNARY_OP;
"feuille"               (*yylval) = gen(at_feuille,1); index_status(yyextra)=0; return T_UNARY_OP;
"option"                (*yylval)=2; index_status(yyextra)=0; return T_LOCAL;
"purge"                  {(*yylval) = gen(at_purge,1); index_status(yyextra)=0; return T_UNARY_OP;};
"repeat"                (*yylval) = gen(at_for,1) ; index_status(yyextra)=0; return T_REPEAT;
"retourne"		(*yylval) = gen(at_return,1) ; index_status(yyextra)=0; return T_RETURN;
"return"		(*yylval) = gen(at_return,1) ; index_status(yyextra)=0; return T_RETURN;
"restart"		(*yylval) = gen(at_restart,1) ; index_status(yyextra)=0; return T_RETURN;
"same"                  (*yylval) = gen(at_same,1); index_status(yyextra)=0; return T_UNARY_OP;
"subs"			if (xcas_mode(yyextra)==1) (*yylval) = gen(at_maple_subs,2); else (*yylval) = gen(at_subs,2); index_status(yyextra)=0; return T_UNARY_OP;
"VARS"                  (*yylval) = gen(at_VARS,0); index_status(yyextra)=0; return T_UNARY_OP;
"while"                 index_status(yyextra)=0; (*yylval)=gen(at_for,4);  if (xcas_mode(yyextra)!=0) return T_MUPMAP_WHILE; return T_WHILE;
"do"                 index_status(yyextra)=0; (*yylval)=gen(at_for,4); return T_DO; /* must be here for DO ... END loop */
"faire"                 index_status(yyextra)=0; (*yylval)=gen(at_for,4); return T_DO; /* must be here for DO ... END loop */

"\xE6\xB9" index_status(yyextra)=0;(*yylval)=gen(at_diff); return T_UNARY_OP; 
"\xE6\xBB" index_status(yyextra)=0;(*yylval)=gen(at_integrate); return T_UNARY_OP;
"\xE5\x51" index_status(yyextra)=0;(*yylval)=gen(at_sum); return T_UNARY_OP;
"≤" index_status(yyextra)=0; (*yylval)=gen(at_inferieur_egal,2); return T_TEST_EQUAL;
"≠" index_status(yyextra)=0; (*yylval)=gen(at_different,2); return T_TEST_EQUAL;
"≥" 		index_status(yyextra)=0; (*yylval)=gen(at_superieur_egal,2); return T_TEST_EQUAL;
"∏" index_status(yyextra)=0;(*yylval)=gen(at_product); return T_UNARY_OP; 
			/* numbers, also accept DMS e.g 1°15′27″13 */
{D}+			|
{D}+"?"			|
{D}+"°" | 
{D}+\302\260{D}+ | 
{D}+"°"{D}+\342\200\262 | 
{D}+"°"{D}+\342\200\262{D}+ | 
{D}+"°"{D}+\342\200\262{D}+\342\200\263 | 
{D}+"°"{D}+\342\200\262{D}+\342\200\263{D}+ | 
{D}+"°"{D}+\342\200\262{D}+"."{D}+ | 
{D}+"°"{D}+\342\200\262{D}+"."{D}+\342\200\263 | 
"#"[0-7]+"o"		|
"#"[0-1]+"b"		|
"#"[0-9a-fA-F]+"h"	|
"#o"[0-7]+		|
"#b"[0-1]+		|
"#x"[0-9a-fA-F]+	|
"0o"[0-7]+		|
"0b"[0-1]+		|
"0x"[0-9a-fA-F]+	|
{D}+"."{D}*({E})?	|
{D}+"."{D}*"?"({E})?	|
{D}*"."{D}+({E})?	|
{D}*"."{D}+"?"({E})?	|
{D}+{E}			| 
{D}+"?"{E}			{ 
  index_status(yyextra)=1;
  int l=strlen(yytext);
  int interv=0; // set to non-zero if ? in the number
  int dot=-1;
  for (int i=0;i<l;++i){
    if (yytext[i]=='?'){
      interv=i; // mark ? position and remove it from the string
      for (;i<l;++i){
	yytext[i]=yytext[i+1];
      }
      --l;
      break;
    }
    if (yytext[i]=='.')
      dot=i;
  }
  // CERR << yytext << " " << interv << endl;
  if (dot>=0 && interv>1){
    --interv; // interv is the relative precision of the interval
    if (interv && dot>=1 && yytext[dot-1]=='0')
      --interv;
    ++dot;
    while (interv && dot<l && yytext[dot]=='0'){
      --interv; ++dot;
    }
  }
  char ch,ch2;
  if (l>2 && yytext[1]!='x' && (yytext[l-1]=='o' || yytext[l-1]=='b' || yytext[l-1]=='h') ){
    char base=yytext[l-1];
    for (int i=l-1;i>1;--i){
      yytext[i]=yytext[i-1];
    }
    if (base=='h')
      base='x';
    yytext[1]=base;
  }
  else {
    for (l=0;(ch=*(yytext+l));++l){
      if (ch=='x')
	break;
      if (ch=='e' || ch=='E'){
	if ( (ch2=*(yytext+l+1)) && (ch2=='e' || ch2=='E')){
	  ++l;
	  for (;(ch=*(yytext+l));++l)
	    *(yytext+l-1)=ch;
	  *(yytext+l-1)=0;
	  --l;
	}
      }
#ifndef BCD
      if ( (ch==-30 && *(yytext+l+1)==-128) || (ch==-62 && *(yytext+l+1)==-80) ){
	*yylval=0; return T_NUMBER;
      }
#endif
      if (ch==-30 && *(yytext+l+1)==-120 &&  *(yytext+l+2)==-110){
	l += 3;
	for (;(ch=*(yytext+l));++l)
	  *(yytext+l-2)=ch;
	*(yytext+l-2)=0;
	l -= 3;
	*(yytext+l)='-';
      }
    }
  }
  (*yylval) = chartab2gen(yytext,yyextra); 
  if (interv){
    double d=evalf_double(*yylval,1,context0)._DOUBLE_val;
    if (d<0 && interv>1)
      --interv;
    double tmp=std::floor(std::log(absdouble(d))/std::log(10.0));
    tmp=(std::pow(10.,1+tmp-interv));
    *yylval=eval(gen(makevecteur(d-tmp,d+tmp),_INTERVAL__VECT),1,context0);
  }
  return T_NUMBER; 
}

			/* symbols */
{A}{AN}*        |
"%"{A}{AN}*     {
 index_status(yyextra)=1;
 int res=find_or_make_symbol(yytext,(*yylval),yyscanner,true,yyextra);
 if (res==T_NUMBER)
   *yylval=(*yylval)(string2gen(unlocalize(yytext),false),yyextra);
 return res;
} 
"#"                     if (!xcas_mode(yyextra) || xcas_mode(yyextra)==3) { 
  // CERR << "hash" << endl;
  (*yylval)=gen(at_hash,1); return TI_HASH; 
} else BEGIN(comment_hash);
<comment_hash>[^*\n]*\n BEGIN(INITIAL); index_status(yyextra)=0; increment_lexer_line_number_setcol(yyscanner,yyextra);  /* comment_s(yyextra)=string(yytext); (*yylval)=string2gen(comment_s(yyextra).substr(0,comment_s(yyextra).size()-1),false); return T_COMMENT; */
			/* everything else */
.			(*yylval)=string2gen(string(yytext),false); return T_STRING;

%%

/*
 *  Routines
 */
#ifndef NO_NAMESPACE_GIAC
  namespace giac {
#endif // ndef NO_NAMESPACE_GIAC

    // Set the input string
    // export GIAC_DEBUG=-2 to renew static_lexer.h/static_extern.h

    YY_BUFFER_STATE set_lexer_string(const std::string &s_orig,yyscan_t & scanner,GIAC_CONTEXT,int maxsize){
      char lexer_string[4096]="1";
      giac_yylex_init(&scanner);
      giac_yyset_extra(contextptr,scanner);
      currently_scanned(contextptr)=lexer_string;
      index_status(contextptr)=0;
      opened_quote(contextptr)=0;
      in_rpn(contextptr)=0;
      lexer_line_number(contextptr)=1;
      first_error_line(contextptr)=0;
      spread_formula(contextptr)=0;
      if (s_orig.size()>=(maxsize?maxsize:sizeof(lexer_string)*.75)){
	//giac_yyerror(scanner,s.c_str());
	confirm("Parse_string_too_long","",true);
	YY_BUFFER_STATE state=giac_yy_scan_string("1",scanner);
	return state;
      }
      {
	string s; s.reserve(s_orig.size()+128);
	s=s_orig;
#if defined NSPIRE // || defined FXCG
      for (unsigned i=0;i<s.size()-1;++i){
	if (s[i]==']' && s[i+1]=='['){
	  string tmp=s.substr(0,i+1)+string(",");
	  s=tmp+s.substr(i+1,s.size()-i-1);
	}
      }
#endif
      bool instring=false;
      // stupid match of bracket then parenthesis
      int l=s.size(),nb=0,np=0;
      int i=0;
      if (lexer_close_parenthesis(contextptr)){
	for (;i<l;++i){
	  if (!instring && i && s[i]=='/' && s[i-1]=='/'){
	    // skip comment until end of line
	    for (;i<l;++i){
	      if (s[i]==13)
		break;
	    }
	    continue;
	  }
	  if (!instring && i>=2 && ( (s[i-2]=='-' && s[i-1]=='-') || (s[i-2]=='+' && s[i-1]=='+') ) && (s[i]=='.'|| (s[i]>='0' && s[i]<='9')) ){
	    s[i-2]='+';
	    s[i-1]=' ';
	  }
	  if (!instring && i>=5 && s[i-5]=='(' && s[i-4]=='N' && s[i-3]=='U' && s[i-2]=='L' && s[i-1]=='L' ){
	    s[i-4]=s[i-3]=s[i-2]=s[i-1]=' ';
	  }
	  if (!instring && i && s[i]=='*' && s[i-1]=='/'){
	    // skip comment 
	    for (;i<l;++i){
	      if (s[i]=='/' && s[i-1]=='*')
		break;
	    }
	    if (i==l){
	      s = s.substr(0,l-1)+"*/"+s[l-1];
	      CERR << "unfinished comment, adding */" << endl << s << endl;
	    }
	    continue;
	  }
	  if (!instring && s[i]==92){
	    i += 2;
	    if (i>=l)
	      break;
	  }
	  if (instring){
	    if (s[i]=='"'&& (i==0 || s[i-1]!='\\'))
	      instring=false;
	  }
	  else {
	    switch (s[i]){
	    case '"':
	      instring=i==0 || s[i-1]!='\\';
	      break;
	    case '(':
	      ++np;
	      break;
	    case ')':
	      --np;
	      break;
	    case '[':
	      ++nb;
	      break;
	    case ']':
	      --nb;
	      break;
	    }
	  }
	}
	if (nb<0)
	  *logptr(contextptr) << "Too many ]" << endl;
	if (np<0)
	  *logptr(contextptr) << "Too many )" << endl;
	while (np<0 && i>=0 && s[i-1]==')'){
	  --i;
	  ++np;
	}
	while (nb<0 && i>=0 && s[i-1]==']'){
	  --i;
	  ++nb;
	}
	s=s.substr(0,i);
	if (nb>0){
	  *logptr(contextptr) << "Warning adding " << nb << " ] at end of input" << endl;
	  s += string(nb,']');
	}
	if (np>0){
	  *logptr(contextptr) << "Warning adding " << np << " ) at end of input" << endl;
	  s += string(np,')');
	}
      }
      l=s.size();
      for (;l;l--){
	if (s[l-1]!=' ')
	  break;
      }
      // strings ending with :;
      while (l>=4 && s[l-1]==';' && s[l-2]==':'){
	// skip spaces before :;
	int m;
	for (m=l-3;m>0;--m){
	  if (s[m]!=' ')
	    break;
	}
	if (m<=1 || s[m]!=';')
	  break;
	if (s[m-1]==':')
	  l = m+1;
	else {
	  s[m]=':';
	  s[m+1]=';';
	  l=m+2;
	}
      }
      s=s.substr(0,l);
      /* if (l && ( (s[l-1]==';') || (s[l-1]==':')))
	 l--; */
      string ss;
      ss.reserve(s.size()*1.1);
      for (int i=0;i<l;++i){
	if (s[i]=='\\' && s[i+1]=='\n'){
	  ++i;
	  continue;
	}
	if (i && (unsigned char)s[i]==0xc2 && (unsigned char)s[i+1]!=0xb0)
	  ss += ' ';
	if ( (unsigned char)s[i]==0xef && i<l-3 ){
          if ((unsigned char)s[i+1]==0x80 && (unsigned char)s[i+2]==0x80 ){  
	    ss+='e';
	    i+=2;
	    continue;
	  }
	}
	if ( (unsigned char)s[i]==0xe2 && i<l-3 ){
	  if ((unsigned char)s[i+1]==134 && (unsigned char)s[i+2]==146){
	    // 0xe2 0x86 0x92
	    ss += ' ';
	    ss += s[i];
	    ++i;
	    ss += s[i];
	    ++i;
	    ss += s[i];
	    ss += ' ';
	    continue;
	  }
          if ((unsigned char)s[i+1]==0x89){ 
	    ss += ' ';
	    ss += s[i];
	    ++i;
	    ss += s[i];
	    ++i;
	    ss += s[i];
	    ss += ' ';
	    continue;
	  } // 0xe2 0x89	  
          if ((unsigned char)s[i+1]==0x88){ 
	    // mathop, add blank before and after except following an e/E 
	    if ((unsigned char) s[i+2]==0x91){ // sigma
	      ss += " SIGMA";
	      i +=2;
	      continue;
	    }
	    if ((unsigned char) s[i+2]==0x86){ // delta
	      ss += " DELTA";
	      i +=2;
	      continue;
	    }
	    if ((unsigned char) s[i+2]==0x8f){ // pi
	      ss += " PI";
	      i +=2;
	      continue;
	    }
	    if ( i>1 && (s[i-1]=='e' || s[i-1]=='E')){
	      ss +='-';
	      i +=2;
	      continue;
	    }
	    if (i>2  && (s[i-1]==' ' && (s[i-2]=='e' || s[i-2]=='E')) ){
	      ss[ss.size()-1] = '-';
	      i += 3;
	      continue;
	    }
	    ss += ' ';
	    ss += s[i];
	    ++i;
	    ss += s[i];
	    ++i;
	    ss += s[i];
	    ss += ' ';
	    continue;
	  } // 0xe2 0x88
          if ((unsigned char)s[i+1]==0x96 && ((unsigned char)s[i+2]==0xba || (unsigned char)s[i+2]==182 )){  
	    // sto 
	    ss += s[i];
	    ++i;
	    ss += s[i];
	    ++i;
	    ss += s[i];
	    ss += ' ';
	    continue;
	  } // 0xe2 0x96
          if ((unsigned char)s[i+1]==0x86 && (unsigned char)s[i+2]==0x92){  
	    // sto 
	    ss += s[i];
	    ++i;
	    ss += s[i];
	    ++i;
	    ss += s[i];
	    ss += ' ';
	    continue;
	  } // 0xe2 0x96
	} //end if s[i]=0xe2
	if (s[i]=='.'){
	  if ( i && (i<l-1) && (s[i-1]!=' ') && (s[i+1]=='.') ){
	    ss+= " ..";
	    ++i;
	  }
	  else
	    ss+='.';
	}
	else {
	  if (xcas_mode(contextptr) > 0 && xcas_mode(contextptr) !=3){
	    if (s[i]=='#')
	      ss += "//";
	    else
	      ss += s[i];
	  }
	  else
	    ss+=s[i];
	}
      }
      // ofstream of("log"); of << s << endl << ss << endl; of.close();
      if (debug_infolevel>2)
	CERR << "lexer " << ss << endl;
      s.clear();
      ss += " \n ÿ";
      if (ss.size()>=sizeof(lexer_string))
	ss ="Parse_string_too_long";
      strcpy(lexer_string,ss.c_str());
      }
      YY_BUFFER_STATE state=giac_yy_scan_string(lexer_string,scanner);
      return state;
    }

    int delete_lexer_string(YY_BUFFER_STATE & state,yyscan_t & scanner){
      yy_delete_buffer(state,scanner);
      yylex_destroy(scanner);
      return 1;
    }

#if 0 //def STATIC_BUILTIN_LEXER_FUNCTIONS
    bool CasIsBuildInFunction(char const *s, gen &g){ 
      // binary search in builtin_lexer_functions
      int i=0, j=builtin_lexer_functions_number-1;
      int cmp;
      cmp= strcmp(s,builtin_lexer_functions[i].s);
      if (cmp==0) goto found; if (cmp<0) return false;
      cmp= strcmp(s,builtin_lexer_functions[j].s);
      if (cmp==0) { i=j; goto found; } if (cmp>0) return false;
      while (1){
        if (i+1>=j) return false;
        int mid= (i+j)/2;
        cmp= strcmp(s,builtin_lexer_functions[mid].s);
        if (cmp==0) { i=mid; goto found; } 
        if (cmp>0) i= mid; else j=mid;
      }
    found:
#if defined NSPIRE 
      g= gen(int((*builtin_lexer_functions_())[i]+builtin_lexer_functions[i]._FUNC_));
#else
      g= gen(int(builtin_lexer_functions_[i]+builtin_lexer_functions[i]._FUNC_));
#endif
      g= gen(*g._FUNCptr);
      return true;
    }
#endif

#ifndef NO_NAMESPACE_GIAC
  } // namespace giac
#endif // ndef NO_NAMESPACE_GIAC
  
