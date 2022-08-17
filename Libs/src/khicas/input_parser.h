/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_GIAC_YY_Y_TAB_H_INCLUDED
# define YY_GIAC_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int giac_yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    T_NUMBER = 258,
    T_SYMBOL = 259,
    T_LITERAL = 260,
    T_DIGITS = 261,
    T_STRING = 262,
    T_END_INPUT = 263,
    T_EXPRESSION = 264,
    T_UNARY_OP = 265,
    T_OF = 266,
    T_NOT = 267,
    T_TYPE_ID = 268,
    T_VIRGULE = 269,
    T_AFFECT = 270,
    T_MAPSTO = 271,
    T_BEGIN_PAR = 272,
    T_END_PAR = 273,
    T_PLUS = 274,
    T_MOINS = 275,
    T_DIV = 276,
    T_MOD = 277,
    T_POW = 278,
    T_QUOTED_BINARY = 279,
    T_QUOTE = 280,
    T_PRIME = 281,
    T_TEST_EQUAL = 282,
    T_EQUAL = 283,
    T_INTERVAL = 284,
    T_UNION = 285,
    T_INTERSECT = 286,
    T_MINUS = 287,
    T_AND_OP = 288,
    T_COMPOSE = 289,
    T_DOLLAR = 290,
    T_DOLLAR_MAPLE = 291,
    T_INDEX_BEGIN = 292,
    T_VECT_BEGIN = 293,
    T_VECT_DISPATCH = 294,
    T_VECT_END = 295,
    T_SEMI = 296,
    T_DEUXPOINTS = 297,
    T_DOUBLE_DEUX_POINTS = 298,
    T_IF = 299,
    T_ELIF = 300,
    T_THEN = 301,
    T_ELSE = 302,
    T_IFTE = 303,
    T_SWITCH = 304,
    T_CASE = 305,
    T_DEFAULT = 306,
    T_ENDCASE = 307,
    T_FOR = 308,
    T_FROM = 309,
    T_TO = 310,
    T_DO = 311,
    T_BY = 312,
    T_WHILE = 313,
    T_MUPMAP_WHILE = 314,
    T_REPEAT = 315,
    T_UNTIL = 316,
    T_IN = 317,
    T_START = 318,
    T_BREAK = 319,
    T_CONTINUE = 320,
    T_TRY = 321,
    T_CATCH = 322,
    T_TRY_CATCH = 323,
    T_PROC = 324,
    T_BLOC = 325,
    T_BLOC_BEGIN = 326,
    T_BLOC_END = 327,
    T_RETURN = 328,
    T_LOCAL = 329,
    T_ARGS = 330,
    T_FACTORIAL = 331,
    T_ROOTOF_BEGIN = 332,
    T_ROOTOF_END = 333,
    T_SPOLY1_BEGIN = 334,
    T_SPOLY1_END = 335,
    T_HELP = 336,
    TI_STO = 337,
    T_PIPE = 338,
    TI_HASH = 339,
    T_INTERROGATION = 340,
    T_SQ = 341,
    T_IFERR = 342,
    T_UNARY_OP_38 = 343,
    T_FUNCTION = 344,
    T_LOGO = 345,
    T_BIDON = 346,
    T_IMPMULT = 347
  };
#endif
/* Tokens.  */
#define T_NUMBER 258
#define T_SYMBOL 259
#define T_LITERAL 260
#define T_DIGITS 261
#define T_STRING 262
#define T_END_INPUT 263
#define T_EXPRESSION 264
#define T_UNARY_OP 265
#define T_OF 266
#define T_NOT 267
#define T_TYPE_ID 268
#define T_VIRGULE 269
#define T_AFFECT 270
#define T_MAPSTO 271
#define T_BEGIN_PAR 272
#define T_END_PAR 273
#define T_PLUS 274
#define T_MOINS 275
#define T_DIV 276
#define T_MOD 277
#define T_POW 278
#define T_QUOTED_BINARY 279
#define T_QUOTE 280
#define T_PRIME 281
#define T_TEST_EQUAL 282
#define T_EQUAL 283
#define T_INTERVAL 284
#define T_UNION 285
#define T_INTERSECT 286
#define T_MINUS 287
#define T_AND_OP 288
#define T_COMPOSE 289
#define T_DOLLAR 290
#define T_DOLLAR_MAPLE 291
#define T_INDEX_BEGIN 292
#define T_VECT_BEGIN 293
#define T_VECT_DISPATCH 294
#define T_VECT_END 295
#define T_SEMI 296
#define T_DEUXPOINTS 297
#define T_DOUBLE_DEUX_POINTS 298
#define T_IF 299
#define T_ELIF 300
#define T_THEN 301
#define T_ELSE 302
#define T_IFTE 303
#define T_SWITCH 304
#define T_CASE 305
#define T_DEFAULT 306
#define T_ENDCASE 307
#define T_FOR 308
#define T_FROM 309
#define T_TO 310
#define T_DO 311
#define T_BY 312
#define T_WHILE 313
#define T_MUPMAP_WHILE 314
#define T_REPEAT 315
#define T_UNTIL 316
#define T_IN 317
#define T_START 318
#define T_BREAK 319
#define T_CONTINUE 320
#define T_TRY 321
#define T_CATCH 322
#define T_TRY_CATCH 323
#define T_PROC 324
#define T_BLOC 325
#define T_BLOC_BEGIN 326
#define T_BLOC_END 327
#define T_RETURN 328
#define T_LOCAL 329
#define T_ARGS 330
#define T_FACTORIAL 331
#define T_ROOTOF_BEGIN 332
#define T_ROOTOF_END 333
#define T_SPOLY1_BEGIN 334
#define T_SPOLY1_END 335
#define T_HELP 336
#define TI_STO 337
#define T_PIPE 338
#define TI_HASH 339
#define T_INTERROGATION 340
#define T_SQ 341
#define T_IFERR 342
#define T_UNARY_OP_38 343
#define T_FUNCTION 344
#define T_LOGO 345
#define T_BIDON 346
#define T_IMPMULT 347

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int giac_yyparse (void * scanner);

#endif /* !YY_GIAC_YY_Y_TAB_H_INCLUDED  */
