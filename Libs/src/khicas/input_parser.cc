/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse         giac_yyparse
#define yylex           giac_yylex
#define yyerror         giac_yyerror
#define yylval          giac_yylval
#define yychar          giac_yychar
#define yydebug         giac_yydebug
#define yynerrs         giac_yynerrs


/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 24 "input_parser.yy"

         #define YYPARSE_PARAM scanner
         #define YYLEX_PARAM   scanner
	 
/* Line 268 of yacc.c  */
#line 33 "input_parser.yy"

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
#define YYSTYPE giac::gen
#define YY_EXTRA_TYPE  const giac::context *
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
#if defined RTOS_THREADX || defined NSPIRE || defined NSPIRE_NEWLIB || defined NUMWORKS
#ifdef RTOS_THREADX
#define YYINITDEPTH 100
#define YYMAXDEPTH 101
#else
#define YYINITDEPTH 200
#define YYMAXDEPTH 201
#endif
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

#if 0
#define YYSTACK_USE_ALLOCA 1
#endif


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


/* Line 268 of yacc.c  */
#line 170 "y.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
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
     T_FOIS = 276,
     T_DIV = 277,
     T_MOD = 278,
     T_POW = 279,
     T_QUOTED_BINARY = 280,
     T_QUOTE = 281,
     T_PRIME = 282,
     T_TEST_EQUAL = 283,
     T_EQUAL = 284,
     T_INTERVAL = 285,
     T_UNION = 286,
     T_INTERSECT = 287,
     T_MINUS = 288,
     T_AND_OP = 289,
     T_COMPOSE = 290,
     T_DOLLAR = 291,
     T_DOLLAR_MAPLE = 292,
     T_INDEX_BEGIN = 293,
     T_VECT_BEGIN = 294,
     T_VECT_DISPATCH = 295,
     T_VECT_END = 296,
     T_SET_BEGIN = 297,
     T_SET_END = 298,
     T_SEMI = 299,
     T_DEUXPOINTS = 300,
     T_DOUBLE_DEUX_POINTS = 301,
     T_IF = 302,
     T_RPN_IF = 303,
     T_ELIF = 304,
     T_THEN = 305,
     T_ELSE = 306,
     T_IFTE = 307,
     T_SWITCH = 308,
     T_CASE = 309,
     T_DEFAULT = 310,
     T_ENDCASE = 311,
     T_FOR = 312,
     T_FROM = 313,
     T_TO = 314,
     T_DO = 315,
     T_BY = 316,
     T_WHILE = 317,
     T_MUPMAP_WHILE = 318,
     T_RPN_WHILE = 319,
     T_REPEAT = 320,
     T_UNTIL = 321,
     T_IN = 322,
     T_START = 323,
     T_BREAK = 324,
     T_CONTINUE = 325,
     T_TRY = 326,
     T_CATCH = 327,
     T_TRY_CATCH = 328,
     T_PROC = 329,
     T_BLOC = 330,
     T_BLOC_BEGIN = 331,
     T_BLOC_END = 332,
     T_RETURN = 333,
     T_LOCAL = 334,
     T_LOCALBLOC = 335,
     T_NAME = 336,
     T_PROGRAM = 337,
     T_NULL = 338,
     T_ARGS = 339,
     T_FACTORIAL = 340,
     T_RPN_OP = 341,
     T_RPN_BEGIN = 342,
     T_RPN_END = 343,
     T_STACK = 344,
     T_GROUPE_BEGIN = 345,
     T_GROUPE_END = 346,
     T_LINE_BEGIN = 347,
     T_LINE_END = 348,
     T_VECTOR_BEGIN = 349,
     T_VECTOR_END = 350,
     T_CURVE_BEGIN = 351,
     T_CURVE_END = 352,
     T_ROOTOF_BEGIN = 353,
     T_ROOTOF_END = 354,
     T_SPOLY1_BEGIN = 355,
     T_SPOLY1_END = 356,
     T_POLY1_BEGIN = 357,
     T_POLY1_END = 358,
     T_MATRICE_BEGIN = 359,
     T_MATRICE_END = 360,
     T_ASSUME_BEGIN = 361,
     T_ASSUME_END = 362,
     T_HELP = 363,
     TI_DEUXPOINTS = 364,
     TI_LOCAL = 365,
     TI_LOOP = 366,
     TI_FOR = 367,
     TI_WHILE = 368,
     TI_STO = 369,
     TI_TRY = 370,
     TI_DIALOG = 371,
     T_PIPE = 372,
     TI_DEFINE = 373,
     TI_PRGM = 374,
     TI_SEMI = 375,
     TI_HASH = 376,
     T_ACCENTGRAVE = 377,
     T_MAPLELIB = 378,
     T_INTERROGATION = 379,
     T_UNIT = 380,
     T_BIDON = 381,
     T_LOGO = 382,
     T_SQ = 383,
     T_CASE38 = 384,
     T_IFERR = 385,
     T_MOINS38 = 386,
     T_NEG38 = 387,
     T_UNARY_OP_38 = 388,
     T_FUNCTION = 389,
     T_IMPMULT = 390
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
#define T_FOIS 276
#define T_DIV 277
#define T_MOD 278
#define T_POW 279
#define T_QUOTED_BINARY 280
#define T_QUOTE 281
#define T_PRIME 282
#define T_TEST_EQUAL 283
#define T_EQUAL 284
#define T_INTERVAL 285
#define T_UNION 286
#define T_INTERSECT 287
#define T_MINUS 288
#define T_AND_OP 289
#define T_COMPOSE 290
#define T_DOLLAR 291
#define T_DOLLAR_MAPLE 292
#define T_INDEX_BEGIN 293
#define T_VECT_BEGIN 294
#define T_VECT_DISPATCH 295
#define T_VECT_END 296
#define T_SET_BEGIN 297
#define T_SET_END 298
#define T_SEMI 299
#define T_DEUXPOINTS 300
#define T_DOUBLE_DEUX_POINTS 301
#define T_IF 302
#define T_RPN_IF 303
#define T_ELIF 304
#define T_THEN 305
#define T_ELSE 306
#define T_IFTE 307
#define T_SWITCH 308
#define T_CASE 309
#define T_DEFAULT 310
#define T_ENDCASE 311
#define T_FOR 312
#define T_FROM 313
#define T_TO 314
#define T_DO 315
#define T_BY 316
#define T_WHILE 317
#define T_MUPMAP_WHILE 318
#define T_RPN_WHILE 319
#define T_REPEAT 320
#define T_UNTIL 321
#define T_IN 322
#define T_START 323
#define T_BREAK 324
#define T_CONTINUE 325
#define T_TRY 326
#define T_CATCH 327
#define T_TRY_CATCH 328
#define T_PROC 329
#define T_BLOC 330
#define T_BLOC_BEGIN 331
#define T_BLOC_END 332
#define T_RETURN 333
#define T_LOCAL 334
#define T_LOCALBLOC 335
#define T_NAME 336
#define T_PROGRAM 337
#define T_NULL 338
#define T_ARGS 339
#define T_FACTORIAL 340
#define T_RPN_OP 341
#define T_RPN_BEGIN 342
#define T_RPN_END 343
#define T_STACK 344
#define T_GROUPE_BEGIN 345
#define T_GROUPE_END 346
#define T_LINE_BEGIN 347
#define T_LINE_END 348
#define T_VECTOR_BEGIN 349
#define T_VECTOR_END 350
#define T_CURVE_BEGIN 351
#define T_CURVE_END 352
#define T_ROOTOF_BEGIN 353
#define T_ROOTOF_END 354
#define T_SPOLY1_BEGIN 355
#define T_SPOLY1_END 356
#define T_POLY1_BEGIN 357
#define T_POLY1_END 358
#define T_MATRICE_BEGIN 359
#define T_MATRICE_END 360
#define T_ASSUME_BEGIN 361
#define T_ASSUME_END 362
#define T_HELP 363
#define TI_DEUXPOINTS 364
#define TI_LOCAL 365
#define TI_LOOP 366
#define TI_FOR 367
#define TI_WHILE 368
#define TI_STO 369
#define TI_TRY 370
#define TI_DIALOG 371
#define T_PIPE 372
#define TI_DEFINE 373
#define TI_PRGM 374
#define TI_SEMI 375
#define TI_HASH 376
#define T_ACCENTGRAVE 377
#define T_MAPLELIB 378
#define T_INTERROGATION 379
#define T_UNIT 380
#define T_BIDON 381
#define T_LOGO 382
#define T_SQ 383
#define T_CASE38 384
#define T_IFERR 385
#define T_MOINS38 386
#define T_NEG38 387
#define T_UNARY_OP_38 388
#define T_FUNCTION 389
#define T_IMPMULT 390




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 482 "y.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  157
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   14221

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  136
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  30
/* YYNRULES -- Number of rules.  */
#define YYNRULES  260
/* YYNRULES -- Number of states.  */
#define YYNSTATES  607

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   390

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,    12,    16,    18,    21,    26,
      33,    37,    43,    51,    53,    55,    62,    69,    76,    83,
      92,    96,   100,   104,   108,   112,   116,   120,   125,   130,
     135,   137,   139,   141,   145,   150,   154,   158,   162,   166,
     169,   173,   177,   181,   185,   189,   193,   197,   201,   204,
     207,   211,   215,   219,   222,   225,   228,   234,   238,   240,
     244,   247,   252,   257,   259,   264,   269,   274,   278,   280,
     283,   286,   293,   298,   304,   309,   311,   316,   318,   322,
     326,   331,   333,   336,   338,   342,   344,   346,   354,   364,
     374,   384,   392,   402,   404,   409,   415,   423,   429,   433,
     435,   439,   442,   448,   452,   456,   459,   463,   465,   469,
     474,   478,   482,   486,   488,   492,   497,   504,   511,   515,
     519,   523,   525,   528,   532,   535,   539,   542,   544,   546,
     549,   551,   555,   560,   562,   569,   577,   581,   583,   588,
     596,   605,   613,   623,   632,   641,   651,   661,   672,   677,
     681,   686,   694,   704,   710,   717,   723,   729,   737,   742,
     744,   752,   757,   762,   766,   768,   771,   775,   780,   786,
     791,   798,   804,   808,   822,   835,   848,   857,   861,   864,
     867,   875,   889,   899,   905,   911,   913,   915,   917,   919,
     923,   927,   931,   935,   941,   944,   948,   951,   954,   956,
     958,   959,   962,   965,   970,   973,   977,   981,   983,   987,
     989,   993,   997,  1001,  1005,  1007,  1011,  1013,  1015,  1016,
    1018,  1019,  1021,  1023,  1026,  1029,  1030,  1033,  1037,  1039,
    1040,  1043,  1044,  1047,  1050,  1053,  1055,  1057,  1058,  1062,
    1065,  1069,  1074,  1076,  1080,  1086,  1093,  1095,  1098,  1100,
    1103,  1104,  1108,  1114,  1115,  1118,  1124,  1125,  1128,  1135,
    1143
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     137,     0,    -1,   138,    -1,   139,     8,    -1,   139,    44,
       8,    -1,   139,    44,   138,    -1,     3,    -1,     3,   142,
      -1,     3,   142,    24,     3,    -1,     3,   142,    24,    17,
       3,    18,    -1,     3,   142,   128,    -1,     3,    10,    17,
     139,    18,    -1,     3,    10,    17,   139,    18,    24,     3,
      -1,     7,    -1,     9,    -1,   141,    17,   150,    18,    15,
     158,    -1,   141,    17,   150,    18,    15,   139,    -1,   139,
     114,   141,    17,   150,    18,    -1,   139,   114,   141,    38,
     139,    41,    -1,   139,   114,   141,    38,    40,   139,    41,
      41,    -1,   139,   114,   141,    -1,   139,   114,    10,    -1,
     139,   114,    19,    -1,   139,   114,    21,    -1,   139,   114,
      22,    -1,   139,   114,    14,    -1,   139,   114,   114,    -1,
     139,   114,   125,   139,    -1,   141,    17,   150,    18,    -1,
     139,    17,   150,    18,    -1,   141,    -1,     5,    -1,     6,
      -1,     6,    15,   139,    -1,     6,    17,   139,    18,    -1,
       6,    17,    18,    -1,   139,   114,     6,    -1,   139,    28,
     139,    -1,   139,    29,   139,    -1,    29,   139,    -1,   139,
      19,   139,    -1,   139,    20,   139,    -1,   139,   131,   139,
      -1,   139,    21,   139,    -1,   139,    22,   139,    -1,   139,
      24,   139,    -1,   139,    23,   139,    -1,   139,    30,   139,
      -1,   139,    30,    -1,    30,   139,    -1,    30,    14,   139,
      -1,   139,    34,   139,    -1,   139,    45,   139,    -1,    20,
     139,    -1,   132,   139,    -1,    19,   139,    -1,   100,   139,
      14,   139,   101,    -1,    98,   139,    99,    -1,    11,    -1,
     139,    15,   139,    -1,    12,   139,    -1,    84,    17,   139,
      18,    -1,    84,    38,   139,    41,    -1,    84,    -1,    10,
      17,   139,    18,    -1,   133,    17,   139,    18,    -1,    10,
      38,   139,    41,    -1,    10,    17,    18,    -1,    10,    -1,
     139,    27,    -1,   139,    85,    -1,    47,   139,    50,   158,
      51,   158,    -1,    47,   139,    50,   158,    -1,    47,   139,
      50,   151,   159,    -1,    52,    17,   139,    18,    -1,    52,
      -1,    82,    17,   139,    18,    -1,    82,    -1,   139,    16,
     158,    -1,   139,    16,   139,    -1,    75,    17,   139,    18,
      -1,    75,    -1,    78,   139,    -1,    78,    -1,    26,    78,
      26,    -1,    69,    -1,    70,    -1,    57,   140,    67,   139,
      60,   151,    77,    -1,    57,   140,    67,   139,    60,   151,
      51,   151,    77,    -1,    57,   141,   155,    59,   139,   154,
     156,   151,    77,    -1,    57,   141,   155,   154,    59,   139,
      60,   151,    77,    -1,    57,   141,   155,   154,    60,   151,
      77,    -1,    57,   141,   155,   154,    63,   139,    60,   151,
      77,    -1,    57,    -1,    65,   151,    66,   139,    -1,    65,
     151,    66,   139,    77,    -1,   130,   151,    50,   151,    51,
     151,    77,    -1,   130,   151,    50,   151,    77,    -1,   129,
     164,    77,    -1,    13,    -1,    26,    13,    26,    -1,    37,
     139,    -1,   139,    37,   141,    67,   139,    -1,   139,    37,
     139,    -1,   139,    36,   139,    -1,    36,     4,    -1,   139,
      35,   139,    -1,    35,    -1,   139,    31,   139,    -1,   139,
      31,   139,    23,    -1,   139,    32,   139,    -1,   139,    33,
     139,    -1,   139,   117,   139,    -1,    25,    -1,    26,   139,
      26,    -1,   139,    38,   139,    41,    -1,   139,    38,    40,
     139,    41,    41,    -1,    17,   139,    18,    17,   150,    18,
      -1,    17,   139,    18,    -1,    40,   150,    41,    -1,   139,
      14,   139,    -1,    83,    -1,   108,   139,    -1,   139,   124,
     139,    -1,   125,   139,    -1,   139,   125,   139,    -1,   139,
     128,    -1,     1,    -1,   144,    -1,   127,   139,    -1,   127,
      -1,   127,    17,    18,    -1,    80,    17,   139,    18,    -1,
      80,    -1,    47,    17,   139,    18,   158,   157,    -1,    47,
      17,   139,    18,   139,    44,   157,    -1,    87,   152,    88,
      -1,   123,    -1,   123,    38,   139,    41,    -1,    74,    17,
     150,    18,   143,   151,    77,    -1,    74,   141,    17,   150,
      18,   143,   151,    77,    -1,    74,   141,    17,   150,    18,
     151,    77,    -1,    74,   141,    17,   150,    18,    76,   143,
     151,    77,    -1,    74,    17,   150,    18,   143,    76,   151,
      77,    -1,   141,    17,   150,    18,    74,   143,   151,    77,
      -1,   141,    17,   150,    18,    15,    74,   143,   151,    77,
      -1,    57,    17,   149,    44,   149,    44,   149,    18,   158,
      -1,    57,    17,   149,    44,   149,    44,   149,    18,   139,
      44,    -1,    57,    17,   139,    18,    -1,   139,    67,   139,
      -1,   139,    12,    67,   139,    -1,    40,   139,    57,   147,
      67,   139,    41,    -1,    40,   139,    57,   147,    67,   139,
      47,   139,    41,    -1,    62,    17,   139,    18,   158,    -1,
      62,    17,   139,    18,   139,    44,    -1,    62,   139,    60,
     151,    77,    -1,    63,   139,    60,   151,    77,    -1,    71,
     158,    72,    17,   139,    18,   158,    -1,    73,    17,   139,
      18,    -1,    73,    -1,    53,    17,   139,    18,    76,   162,
      77,    -1,    54,    17,     4,    18,    -1,    54,   139,   163,
      56,    -1,   122,   153,   122,    -1,    86,    -1,    78,   109,
      -1,   111,   151,   160,    -1,    47,   139,   109,   139,    -1,
     115,   151,    51,   151,   160,    -1,   115,   151,    51,   160,
      -1,   115,   151,   109,    51,   151,   160,    -1,   115,   151,
     109,    51,   160,    -1,   139,   120,   139,    -1,   109,   141,
      17,   150,    18,   119,   151,   109,   110,   150,   109,   151,
     160,    -1,   109,   141,    17,   150,    18,   119,   151,   110,
     150,   109,   151,   160,    -1,   109,   141,    17,   150,    18,
     119,   109,   110,   150,   109,   151,   160,    -1,   109,   141,
      17,   150,    18,   119,   151,   160,    -1,   116,   151,   160,
      -1,   116,   158,    -1,   109,   139,    -1,   118,   141,    17,
     150,    18,    29,   139,    -1,   118,   141,    17,   150,    18,
      29,   119,   109,   110,   150,   109,   151,   160,    -1,   118,
     141,    17,   150,    18,    29,   119,   151,   160,    -1,   112,
     150,   109,   151,   160,    -1,   113,   139,   109,   151,   160,
      -1,     4,    -1,    10,    -1,   133,    -1,     4,    -1,     4,
      46,    13,    -1,     4,    46,    10,    -1,     4,    46,     4,
      -1,     4,    46,   133,    -1,     4,    46,    26,   139,    26,
      -1,    46,     4,    -1,     3,    46,     4,    -1,    13,     4,
      -1,   121,   139,    -1,     4,    -1,     5,    -1,    -1,   143,
     145,    -1,   146,   143,    -1,    89,    17,   139,    18,    -1,
      89,    83,    -1,    79,   147,    44,    -1,    81,   139,    44,
      -1,   148,    -1,   147,    14,   148,    -1,   141,    -1,     4,
      15,   139,    -1,     4,    29,   139,    -1,     4,    45,   139,
      -1,    17,   148,    18,    -1,    10,    -1,    10,    46,   139,
      -1,    13,    -1,     3,    -1,    -1,   139,    -1,    -1,   139,
      -1,   139,    -1,   151,   139,    -1,   151,   165,    -1,    -1,
     153,   152,    -1,   153,    14,   152,    -1,    10,    -1,    -1,
      61,   139,    -1,    -1,    15,   139,    -1,    29,   139,    -1,
      58,   139,    -1,    44,    -1,    60,    -1,    -1,   161,   139,
      44,    -1,   161,   158,    -1,    76,   151,    77,    -1,    76,
     143,   151,    77,    -1,   160,    -1,   161,   151,   160,    -1,
      49,   139,    50,   151,   159,    -1,   109,    49,   139,    50,
     151,   159,    -1,    77,    -1,   109,    77,    -1,    51,    -1,
     109,    51,    -1,    -1,    55,    45,   158,    -1,    54,     3,
      45,   158,   162,    -1,    -1,    55,   151,    -1,    11,     3,
      60,   151,   163,    -1,    -1,    55,   151,    -1,    47,   139,
      50,   151,    77,   164,    -1,    47,   139,    50,   151,    77,
      44,   164,    -1,    44,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   199,   199,   207,   208,   209,   212,   213,   214,   215,
     216,   217,   218,   220,   221,   224,   225,   226,   227,   231,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   254,   255,
     261,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   275,   276,   277,   282,   287,   293,   294,   300,   301,
     302,   303,   304,   305,   306,   320,   325,   329,   336,   339,
     340,   342,   343,   344,   347,   348,   349,   350,   351,   355,
     362,   363,   365,   367,   368,   370,   371,   372,   397,   402,
     415,   428,   432,   436,   441,   446,   452,   456,   460,   461,
     465,   466,   467,   468,   469,   470,   471,   473,   474,   475,
     476,   477,   478,   481,   482,   487,   491,   495,   496,   522,
     531,   537,   538,   539,   540,   545,   549,   550,   559,   560,
     561,   562,   563,   567,   568,   571,   576,   577,   578,   579,
     584,   589,   594,   599,   604,   609,   614,   615,   616,   617,
     618,   619,   620,   621,   625,   628,   632,   636,   637,   638,
     639,   640,   641,   642,   643,   644,   645,   646,   647,   648,
     649,   650,   651,   652,   656,   660,   664,   667,   668,   669,
     670,   671,   675,   676,   695,   707,   708,   709,   712,   713,
     719,   720,   721,   722,   723,   724,   732,   738,   741,   742,
     745,   746,   747,   751,   752,   755,   758,   761,   762,   769,
     770,   771,   772,   773,   774,   775,   776,   784,   794,   795,
     798,   799,   802,   804,   809,   812,   813,   814,   817,   887,
     888,   891,   892,   893,   894,   897,   898,   901,   902,   903,
     907,   910,   917,   918,   922,   925,   930,   931,   934,   935,
     938,   939,   940,   943,   944,   945,   948,   949,   950,   951,
     954
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_NUMBER", "T_SYMBOL", "T_LITERAL",
  "T_DIGITS", "T_STRING", "T_END_INPUT", "T_EXPRESSION", "T_UNARY_OP",
  "T_OF", "T_NOT", "T_TYPE_ID", "T_VIRGULE", "T_AFFECT", "T_MAPSTO",
  "T_BEGIN_PAR", "T_END_PAR", "T_PLUS", "T_MOINS", "T_FOIS", "T_DIV",
  "T_MOD", "T_POW", "T_QUOTED_BINARY", "T_QUOTE", "T_PRIME",
  "T_TEST_EQUAL", "T_EQUAL", "T_INTERVAL", "T_UNION", "T_INTERSECT",
  "T_MINUS", "T_AND_OP", "T_COMPOSE", "T_DOLLAR", "T_DOLLAR_MAPLE",
  "T_INDEX_BEGIN", "T_VECT_BEGIN", "T_VECT_DISPATCH", "T_VECT_END",
  "T_SET_BEGIN", "T_SET_END", "T_SEMI", "T_DEUXPOINTS",
  "T_DOUBLE_DEUX_POINTS", "T_IF", "T_RPN_IF", "T_ELIF", "T_THEN", "T_ELSE",
  "T_IFTE", "T_SWITCH", "T_CASE", "T_DEFAULT", "T_ENDCASE", "T_FOR",
  "T_FROM", "T_TO", "T_DO", "T_BY", "T_WHILE", "T_MUPMAP_WHILE",
  "T_RPN_WHILE", "T_REPEAT", "T_UNTIL", "T_IN", "T_START", "T_BREAK",
  "T_CONTINUE", "T_TRY", "T_CATCH", "T_TRY_CATCH", "T_PROC", "T_BLOC",
  "T_BLOC_BEGIN", "T_BLOC_END", "T_RETURN", "T_LOCAL", "T_LOCALBLOC",
  "T_NAME", "T_PROGRAM", "T_NULL", "T_ARGS", "T_FACTORIAL", "T_RPN_OP",
  "T_RPN_BEGIN", "T_RPN_END", "T_STACK", "T_GROUPE_BEGIN", "T_GROUPE_END",
  "T_LINE_BEGIN", "T_LINE_END", "T_VECTOR_BEGIN", "T_VECTOR_END",
  "T_CURVE_BEGIN", "T_CURVE_END", "T_ROOTOF_BEGIN", "T_ROOTOF_END",
  "T_SPOLY1_BEGIN", "T_SPOLY1_END", "T_POLY1_BEGIN", "T_POLY1_END",
  "T_MATRICE_BEGIN", "T_MATRICE_END", "T_ASSUME_BEGIN", "T_ASSUME_END",
  "T_HELP", "TI_DEUXPOINTS", "TI_LOCAL", "TI_LOOP", "TI_FOR", "TI_WHILE",
  "TI_STO", "TI_TRY", "TI_DIALOG", "T_PIPE", "TI_DEFINE", "TI_PRGM",
  "TI_SEMI", "TI_HASH", "T_ACCENTGRAVE", "T_MAPLELIB", "T_INTERROGATION",
  "T_UNIT", "T_BIDON", "T_LOGO", "T_SQ", "T_CASE38", "T_IFERR",
  "T_MOINS38", "T_NEG38", "T_UNARY_OP_38", "T_FUNCTION", "T_IMPMULT",
  "$accept", "input", "correct_input", "exp", "symbol_for", "symbol",
  "symbol_or_literal", "entete", "stack", "local", "nom", "suite_symbol",
  "affectable_symbol", "exp_or_empty", "suite", "prg_suite", "rpn_suite",
  "rpn_token", "step", "from", "loop38_do", "else", "bloc", "elif",
  "ti_bloc_end", "ti_else", "switch", "case", "case38", "semi", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   136,   137,   138,   138,   138,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   140,   140,   140,   141,   141,
     141,   141,   141,   141,   141,   141,   141,   141,   142,   142,
     143,   143,   143,   144,   144,   145,   146,   147,   147,   148,
     148,   148,   148,   148,   148,   148,   148,   148,   149,   149,
     150,   150,   151,   151,   151,   152,   152,   152,   153,   154,
     154,   155,   155,   155,   155,   156,   156,   157,   157,   157,
     158,   158,   159,   159,   159,   159,   160,   160,   161,   161,
     162,   162,   162,   163,   163,   163,   164,   164,   164,   164,
     165
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     3,     3,     1,     2,     4,     6,
       3,     5,     7,     1,     1,     6,     6,     6,     6,     8,
       3,     3,     3,     3,     3,     3,     3,     4,     4,     4,
       1,     1,     1,     3,     4,     3,     3,     3,     3,     2,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       3,     3,     3,     2,     2,     2,     5,     3,     1,     3,
       2,     4,     4,     1,     4,     4,     4,     3,     1,     2,
       2,     6,     4,     5,     4,     1,     4,     1,     3,     3,
       4,     1,     2,     1,     3,     1,     1,     7,     9,     9,
       9,     7,     9,     1,     4,     5,     7,     5,     3,     1,
       3,     2,     5,     3,     3,     2,     3,     1,     3,     4,
       3,     3,     3,     1,     3,     4,     6,     6,     3,     3,
       3,     1,     2,     3,     2,     3,     2,     1,     1,     2,
       1,     3,     4,     1,     6,     7,     3,     1,     4,     7,
       8,     7,     9,     8,     8,     9,     9,    10,     4,     3,
       4,     7,     9,     5,     6,     5,     5,     7,     4,     1,
       7,     4,     4,     3,     1,     2,     3,     4,     5,     4,
       6,     5,     3,    13,    12,    12,     8,     3,     2,     2,
       7,    13,     9,     5,     5,     1,     1,     1,     1,     3,
       3,     3,     3,     5,     2,     3,     2,     2,     1,     1,
       0,     2,     2,     4,     2,     3,     3,     1,     3,     1,
       3,     3,     3,     3,     1,     3,     1,     1,     0,     1,
       0,     1,     1,     2,     2,     0,     2,     3,     1,     0,
       2,     0,     2,     2,     2,     1,     1,     0,     3,     2,
       3,     4,     1,     3,     5,     6,     1,     2,     1,     2,
       0,     3,     5,     0,     2,     5,     0,     2,     6,     7,
       1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,   127,     6,   188,    31,    32,    13,    14,    68,    58,
       0,    99,     0,     0,     0,   113,     0,     0,     0,   107,
       0,     0,     0,     0,     0,    75,     0,     0,    93,     0,
       0,     0,    85,    86,     0,   159,     0,    81,     0,   133,
      77,   121,    63,   164,   225,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   137,     0,
       0,   256,     0,     0,     0,     0,     2,     0,    30,   128,
     198,   199,     0,     0,     7,     0,     0,     0,     0,     0,
      60,   196,     0,    55,    53,    99,     0,     0,    39,     0,
      49,   105,   101,   221,     0,   194,     0,     0,     0,     0,
       0,   253,     0,   188,   186,     0,     0,   187,     0,   231,
       0,     0,     0,   222,     0,     0,     0,     0,     0,     0,
       0,     0,    82,     0,     0,     0,     0,   228,     0,   225,
       0,   204,     0,     0,   122,   179,    30,     0,   221,     0,
       0,     0,     0,   178,     0,   197,     0,     0,   124,     0,
     129,     0,     0,     0,     0,    54,     0,     1,     3,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      69,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    70,     0,     0,     0,     0,
       0,   126,     0,     0,     0,   195,     0,    10,   191,   190,
     189,     0,   192,    33,    35,     0,    67,     0,     0,   118,
     100,     0,   114,    50,     0,   119,     0,     0,     0,     0,
       0,   188,     0,     0,     0,   219,     0,     0,     0,     0,
       0,   229,     0,     0,     0,   260,     0,   223,   224,     0,
       0,   200,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   136,   225,   226,     0,    57,     0,     0,   246,
       0,   166,     0,     0,     0,     0,   177,     0,   163,     0,
     131,     0,     0,    98,     0,     0,     0,   120,    59,    79,
      78,     0,    40,    41,    43,    44,    46,    45,    37,    38,
      47,   108,   110,   111,    51,   106,   104,   103,    30,     0,
       0,     4,     5,    52,   149,    36,    21,    25,    22,    23,
      24,    26,     0,    20,   112,   172,   123,   125,    42,     0,
       0,     8,     0,     0,    34,    64,    66,     0,   217,   188,
     214,   216,     0,   209,     0,   207,     0,     0,    72,   167,
      74,     0,   161,     0,     0,   162,   148,     0,     0,   232,
     233,   234,     0,     0,     0,     0,     0,     0,    94,     0,
       0,   201,     0,   202,   240,     0,   158,   200,     0,    80,
     132,    76,    61,    62,   227,   203,   120,     0,   247,     0,
       0,     0,   169,     0,     0,   138,     0,     0,    65,   150,
      29,     0,     0,     0,   115,    27,     0,     0,    28,    11,
       0,   193,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   237,     0,   248,     0,    73,   242,     0,     0,
     250,     0,   219,     0,     0,   229,   230,     0,     0,     0,
       0,   153,   155,   156,    95,   206,     0,   241,     0,     0,
       0,    56,    28,   183,   184,   168,     0,   171,     0,     0,
       0,    97,   102,     0,     0,     0,     0,     0,   200,     0,
       9,   117,   210,   211,   212,   215,   213,   208,     0,     0,
     237,     0,   134,     0,     0,     0,   249,     0,    71,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   154,
     205,     0,     0,     0,   200,     0,     0,     0,   170,     0,
     256,     0,   116,    17,     0,    18,   200,    16,    15,     0,
      12,   151,     0,   135,     0,   239,     0,     0,   243,     0,
       0,   160,    58,   255,     0,     0,    87,   235,   236,     0,
       0,    91,     0,   157,     0,   139,     0,     0,   141,     0,
       0,     0,   180,   256,   258,    96,     0,     0,     0,     0,
     238,     0,     0,     0,   251,     0,     0,     0,     0,     0,
     143,     0,   140,     0,     0,     0,   176,     0,     0,   259,
      19,     0,   144,   152,   244,     0,   250,     0,   146,    88,
      89,    90,    92,   142,     0,     0,     0,     0,   182,   145,
     245,   252,   147,     0,     0,     0,     0,     0,     0,     0,
       0,   175,     0,   174,     0,   173,   181
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    65,    66,   113,   108,    68,    74,   240,    69,   361,
     241,   334,   335,   226,    94,   114,   128,   129,   354,   231,
     529,   472,   116,   416,   417,   418,   481,   224,   153,   238
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -521
static const yytype_int16 yypact[] =
{
    8966,  -521,    98,    -4,  -521,   142,  -521,  -521,    23,  -521,
    8966,    81,  8966,  8966,  8966,  -521,  9099,  8966,  6572,  -521,
      91,  8966,  6705,   100,  9232,   102,   122,  9365,    22,  9498,
    8966,  8966,  -521,  -521,    64,   131,   218,   135,  1252,   169,
     192,  -521,    73,  -521,   143,   -13,  8966,  8966,  8966,  8966,
    8966,  8966,  8966,  8966,  6838,   194,  8966,   143,   132,  8966,
    1385,   -11,  8966,  8966,   196,   185,  -521, 10286,   199,  -521,
    -521,  -521,   203,   220,   -23,    62,  8966,  6971,  7104,  8966,
     407,  -521, 10763,   407,   407,    19,  1917, 10790, 13881,  8966,
   14055,  -521, 10401, 10908,   184,  -521,  8966, 10445,  8966,  8966,
    9631, 10327,   186,    46,  -521,    81,  7237,  -521,   170,     4,
    8966, 10967, 11026, 13350,  2981,  3114,   166,  8966,  7370,   226,
    8966,  1518, 13350,  8966,  8966,  8966,  8966,  -521,   157,   136,
    8966,  -521, 11085, 13409, 13350, 13350,   230,  3247, 13350,   140,
   11144,  3380,  3247,  -521,   233,    90,   129,  8966,   724,  7503,
   13881,  8966,  8966,   177,  3513,   407,  8966,  -521,  -521,   201,
    8966,  8966,  6838,  7370,  8966,  8966,  8966,  8966,  8966,  8966,
    -521,  8966,  8966,   853,  8966,  8966,  8966,  8966,  8966,  8966,
    8966,  9764,  7636,  8966,  8966,  -521,   274,  8966,  8966,  8966,
    8966,  -521,  8966,  7370,  8966,  -521,   119,  -521,  -521,  -521,
    -521,  8966,  -521, 13586,  -521, 11203,  -521, 11262, 11321,   242,
    -521,   986,  -521, 13763,    70,  -521, 11380,  6838,  8966, 11439,
   11498,    25,   258,  8966,   207, 11557,   222,  8966,  8966,  8966,
    8966,   153, 11616,  8966,  8966,  -521,  8966, 13350,  -521,  8966,
    7769,   188,  3646,   253, 11675,   257,  7370, 11734, 11793, 11852,
   11911, 11970,  -521,   143,  -521, 12029,  -521,  8966,  7370,  -521,
    7902,  -521,  8966,  8966,  8035,  8168,  -521,  7370,  -521, 12088,
    -521, 12147,  3779,  -521,  8966, 12206,  8966, 13763, 13586, 13940,
    -521,   261, 10363, 10363, 10522, 14077, 14093,   326, 10563, 10401,
   14055, 13967, 14018, 13996,   689,    90,   217, 10401,    17,  6705,
   12265,  -521,  -521, 13881, 10832,  -521,  -521,  -521,  -521,  -521,
    -521,  -521,  8966,    95, 13468,   630, 13822,   724, 10363,   263,
   12324,  -521,   268, 12391,  -521,  -521,  -521,  7370,   186,    86,
     227,    81,    70,  -521,    14,  -521,  1651,  2050,   231, 13350,
    -521,   209,  -521,   232,  3912,  -521,  -521,  7237, 12450, 13350,
   13350, 13350,  8966,  8966,    34,  1784,  4045,  4178, 12509, 12568,
      70,  -521,  4311,   211,  -521,  8966,  -521,   188,   273,  -521,
    -521,  -521,  -521,  -521,  -521,  -521, 13704,   276,  -521,  3247,
    3247,  3247,  -521,  8035,   279,  -521,  8966,  2183,  -521, 10832,
    -521,  1119,  8966, 10486,  -521,   724,  7370,  9897,    -7,   277,
     282,  -521,   285,  8966,  8966,  8966,  8966,   286,    70,  8966,
    7370, 12627,   -44,  8966,  -521,  2316,  -521,  -521,  8966,    64,
      -1,  8966, 13350,   262,  8966, 12701, 13350,  8966,  8966,  8966,
   12760,  -521,  -521,  -521,  -521,  -521,     6,  -521, 12819,  4444,
    2449,  -521,   -10,  -521,  -521,  -521,  3247,  -521,   281,  4577,
    8966,  -521, 10832,   270,   295,  6705, 12878,  8301,   188,   311,
    -521,  -521, 13350, 13645, 13527, 13645,  -521,  -521, 10604, 10763,
     -44,   265,  -521,  6838, 12937,  8966,  -521,  3247,  -521,   315,
     280,   244,  2582,  8434,  2715,    -3, 12996,  4710, 13055,  -521,
    -521,    64,  8966,  4843,   188,  7769,  4976, 10030,  -521,  8567,
     155,  5109,  -521,  -521, 10645,  -521,   202, 13586,  -521,  7769,
    -521,  -521,  8966,  -521, 13114,  -521,  8966, 13173,  -521,   283,
      64,  -521,   258,  -521,   304,  8966,  -521,  -521,  -521,  8966,
    8966,  -521,  8966,  -521,  5242,  -521,  7769,  5375,  -521,  8700,
    2848, 10163, 10401,   -11,  -521,  -521,   288,  7769,  5508, 13232,
    -521,  2050,  8966,    64,  -521,  6838,  5641,  5774,  5907,  6040,
    -521,  6173,  -521,  8966,  6306,  8966,  -521,  8833,  3247,  -521,
    -521,  6439,  -521,  -521,  -521,  2050,    -1, 13291,  -521,  -521,
    -521,  -521,  -521,  -521,   215,  8966,   221,  8966,  -521,  -521,
    -521,  -521,  -521,  8966,   223,  8966,   224,  3247,  8966,  3247,
    8966,  -521,  3247,  -521,  3247,  -521,  -521
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -521,  -521,   144,     0,  -521,   168,  -521,  -232,  -521,  -521,
    -521,   -29,  -326,  -345,    40,   255,  -126,   278,   -91,  -521,
    -521,  -130,   -17,  -520,    -8,  -401,  -235,  -140,  -485,  -521
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -258
static const yytype_int16 yytable[] =
{
      67,   196,   423,   254,   130,   457,   407,   414,   457,   363,
      80,   473,    82,    83,    84,   544,    87,    88,    90,   228,
     408,    92,    93,    81,    97,   102,   103,   101,   408,   111,
     112,   574,   104,   229,   193,   105,   151,   143,   122,   106,
      78,   527,    75,   342,   152,   210,   132,   133,   134,   135,
     490,   138,   140,   479,   480,   590,   145,   528,   569,   148,
     150,    79,   230,   155,   458,   471,   198,   458,    23,   473,
     131,    75,   199,   328,   329,   200,   203,   205,   207,   208,
     330,   409,   467,   331,   392,    81,   122,   332,   201,   213,
     125,   139,    75,   427,   428,    91,   216,   429,   219,   220,
      82,   403,    70,    71,    95,   197,   225,   163,    72,   497,
     232,   126,   396,  -185,   237,   404,    23,   244,   138,    98,
     247,   135,   321,   248,   249,   250,   251,   374,   181,   261,
     255,   405,    75,   397,   266,   439,   322,   237,   524,    99,
     115,   237,   237,    56,    73,   280,   127,   269,   117,    82,
     253,   271,   120,   127,   237,   107,   275,    76,   245,    77,
     277,   278,   279,   138,   282,   283,   284,   285,   286,   287,
     147,   288,   289,   290,   291,   292,   293,   294,   295,   296,
     297,   300,    67,   303,   304,   157,   123,   314,   315,   316,
     317,    56,   318,   138,   320,   202,   109,   102,     3,   543,
     338,   323,   151,   281,   119,   102,     3,   105,   495,   124,
     152,    87,   352,   156,   353,   105,   193,   136,   339,   118,
     194,   102,     3,   144,   195,   215,   509,   348,   349,   350,
     351,   105,    73,   319,   163,   118,   358,   227,   243,   359,
      23,   169,   237,   246,   170,   252,   138,   258,    23,   262,
     267,   268,   178,  -258,   273,   181,   382,   376,   138,   327,
     135,   343,   536,   345,    23,   135,   347,   138,   276,   239,
     365,   400,   237,   406,   547,   367,   389,   102,     3,   390,
     305,   398,   419,   239,   306,   420,   368,   105,   307,   136,
     360,   440,   421,   308,   442,   309,   310,   448,   377,   393,
     460,   459,   185,   461,   466,   137,   483,   384,   141,   142,
     499,   502,   395,   503,   510,    56,   476,   154,   519,   412,
      23,   521,   555,    56,   593,   520,   302,   138,   553,   570,
     595,   436,   598,   600,   485,   146,   411,   237,   431,    56,
     513,   591,   523,   163,   237,   191,     0,   422,   298,     0,
     169,     0,   425,   426,   313,   430,   237,   237,     0,     0,
       0,   178,   237,     0,   181,   438,     0,   402,     0,     0,
     242,   443,   444,   445,     0,   447,     0,     0,     0,   237,
     237,   237,   333,     0,     0,     0,     0,   237,   311,     0,
       0,   286,   452,     0,     0,    56,   138,   456,     0,   312,
       0,     0,   478,   462,   463,   464,   465,   272,     0,   468,
     469,   185,     0,   474,     0,   135,     0,     0,     0,  -258,
       0,     0,     0,     0,   163,     0,     0,   486,   136,   488,
       0,   169,     0,   136,   170,     0,   454,     0,   498,     0,
     508,     0,   178,   179,     0,   181,   237,     0,     0,   237,
     402,     0,     0,     0,   191,   504,   515,   507,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   518,
       0,     0,   337,   514,   533,   517,     0,   237,   344,     0,
       0,     0,   237,   422,   237,     0,     0,   237,   356,   357,
       0,     0,   185,   237,     0,   362,   237,     0,     0,   542,
     333,   237,     0,   554,     0,     0,     0,     0,     0,     0,
       0,     0,   549,     0,     0,     0,     0,   379,   380,   381,
       0,     0,     0,     0,     0,     0,     0,     0,   333,   387,
       0,     0,   566,     0,   237,   191,   576,   237,   578,   135,
     237,     0,     0,     0,     0,     0,     0,     0,   237,     0,
       0,   237,     0,     0,     0,   577,   237,   237,   237,   237,
     588,   237,     0,   138,   135,   138,     0,   135,   237,     0,
       0,   237,     0,     0,     0,   237,   333,     0,     0,     0,
       0,     0,     0,   136,     0,   138,     0,   138,     0,   601,
       0,   603,     0,     0,   605,     0,   606,   237,     0,   237,
       0,     0,   237,   584,   237,   586,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   594,     0,   596,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   446,     0,
       0,   449,   159,     0,   160,     0,   162,   163,     0,   164,
     165,   166,   167,   168,   169,     0,     0,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,     0,
       0,     0,     0,   477,   119,   183,   482,     0,     0,   484,
       0,     0,     0,   487,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   493,   496,     0,   184,     0,     0,
       0,   159,     0,     0,     0,   501,   163,   136,   164,   165,
     166,   167,   168,   169,     0,   185,   170,   171,   172,   173,
     174,   175,   176,     0,   178,   179,   180,   181,     0,     0,
       0,     0,   136,     0,     0,   136,   159,     0,     0,     0,
       0,   163,     0,     0,     0,     0,     0,   534,   169,     0,
     537,   170,   540,     0,   189,   190,   184,     0,   191,   178,
     179,   192,   181,     0,   548,     0,     0,     0,     0,     0,
       0,   551,     0,     0,   185,     0,     0,     0,     0,     0,
     556,     0,     0,     0,   557,   558,     0,   559,     0,     0,
       0,   561,     0,     0,     0,     0,   568,     0,     0,     0,
       0,     0,   571,     0,     0,     0,     0,   575,     0,   185,
       0,     0,     0,     0,   190,     0,     0,   191,     0,     0,
     192,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   597,  -258,
     599,     0,   191,   602,     1,   604,     2,     3,     4,     5,
       6,   -48,     7,     8,     9,    10,    11,   -48,   -48,   -48,
      12,   -48,    13,    14,   -48,   -48,   -48,   -48,    15,    16,
     -48,   -48,   -48,   -48,   -48,   -48,   -48,   -48,    19,    20,
     -48,   -48,     0,    22,   -48,     0,     0,   -48,   -48,    23,
      24,     0,   -48,   -48,   -48,    25,    26,    27,   -48,   -48,
     -48,   -48,   -48,   -48,   -48,    29,    30,     0,    31,   -48,
     -48,     0,    32,    33,    34,     0,    35,    36,    37,     0,
     -48,   -48,     0,    39,     0,    40,    41,    42,   -48,    43,
      44,     0,    45,     0,     0,     0,     0,     0,     0,     0,
       0,    46,   -48,    47,   -48,     0,     0,     0,     0,     0,
       0,    48,   -48,   -48,    50,    51,    52,   -48,    53,    54,
     -48,    55,     0,   -48,    56,    57,    58,   -48,    59,     0,
     -48,   -48,    61,    62,   -48,    63,    64,     1,     0,     2,
       3,     4,     5,     6,   -84,     7,     8,     9,    10,    85,
     -84,   -84,   -84,    12,   -84,    13,    14,   -84,   -84,   -84,
     -84,    15,    16,   -84,   -84,    17,    18,   -84,   -84,   -84,
     -84,    19,    20,    21,   -84,     0,    22,   -84,     0,     0,
     -84,   -84,    23,    24,     0,   -84,   -84,   -84,    25,    26,
      27,   -84,   -84,    28,   -84,   -84,   -84,   -84,    29,    30,
       0,    31,   -84,   -84,     0,    32,    33,    34,     0,    35,
      36,    37,     0,   -84,    86,     0,    39,     0,    40,    41,
      42,   -84,    43,    44,     0,    45,     0,     0,     0,     0,
       0,     0,     0,     0,    46,   -84,    47,   -84,     0,     0,
       0,     0,     0,     0,    48,    49,   -84,    50,    51,    52,
     -84,    53,    54,   -84,    55,     0,   -84,    56,    57,    58,
     -84,    59,     0,    60,   -84,    61,    62,   -84,    63,    64,
       1,     0,  -109,     3,     4,     5,     6,  -109,     7,     8,
       9,    10,    11,  -109,  -109,  -109,    12,  -109,  -109,  -109,
    -109,  -109,  -109,  -109,    15,    16,  -109,  -109,  -109,  -109,
    -109,  -109,  -109,  -109,    19,    20,  -109,  -109,     0,    22,
    -109,     0,     0,  -109,  -109,    23,    24,     0,  -109,  -109,
    -109,    25,    26,    27,  -109,  -109,  -109,  -109,  -109,  -109,
    -109,    29,    30,     0,    31,  -109,  -109,     0,    32,    33,
      34,     0,    35,    36,    37,     0,  -109,  -109,     0,    39,
       0,    40,    41,    42,  -109,    43,    44,     0,    45,     0,
       0,     0,     0,     0,     0,     0,     0,    46,  -109,    47,
    -109,     0,     0,     0,     0,     0,     0,    48,  -109,  -109,
      50,    51,    52,  -109,    53,    54,  -109,    55,     0,  -109,
      56,    57,    58,  -109,    59,     0,  -109,  -109,    61,    62,
    -109,    63,    64,     1,     0,     2,     3,     4,     5,     6,
     -83,     7,     8,     9,    10,    11,   -83,   -83,   -83,    12,
     -83,    13,    14,   -83,   -83,   -83,   -83,    15,    16,   -83,
     -83,    17,    18,   -83,   -83,   -83,   -83,    19,    20,    21,
     -83,     0,    22,   -83,     0,     0,   -83,   -83,    23,    24,
       0,   -83,   -83,   -83,    25,    26,    27,   -83,   -83,    28,
     -83,   -83,   -83,   -83,    29,    30,     0,    31,   -83,   -83,
       0,    32,    33,    34,     0,    35,    36,    37,     0,   -83,
       0,     0,    39,     0,    40,    41,    42,   -83,    43,    44,
       0,    45,     0,     0,     0,     0,     0,     0,     0,     0,
      46,   -83,    47,   -83,     0,     0,     0,     0,     0,     0,
      48,   -83,   -83,    50,    51,    52,   -83,    53,    54,   -83,
      55,     0,   -83,    56,    57,    58,   -83,    59,     0,    60,
     -83,    61,    62,   -83,    63,    64,     1,     0,     2,     3,
       4,     5,     6,  -130,     7,     8,     9,    10,    11,  -130,
    -130,  -130,   149,  -130,    13,    14,  -130,  -130,  -130,  -130,
      15,    16,  -130,  -130,    17,    18,  -130,  -130,  -130,  -130,
      19,    20,    21,  -130,     0,    22,  -130,     0,     0,  -130,
    -130,    23,    24,     0,  -130,  -130,  -130,    25,    26,    27,
    -130,  -130,  -130,  -130,  -130,  -130,  -130,    29,    30,     0,
      31,  -130,  -130,     0,    32,    33,    34,     0,    35,    36,
      37,     0,  -130,  -130,     0,    39,     0,    40,    41,    42,
    -130,    43,    44,     0,    45,     0,     0,     0,     0,     0,
       0,     0,     0,    46,  -130,    47,  -130,     0,     0,     0,
       0,     0,     0,    48,  -130,  -130,    50,    51,    52,  -130,
      53,    54,  -130,    55,     0,  -130,    56,    57,    58,  -130,
      59,     0,     0,  -130,    61,    62,  -130,    63,    64,     1,
       0,     2,     3,     4,     5,     6,  -165,     7,     8,     9,
      10,    11,  -165,  -165,  -165,    12,  -165,    13,    14,  -165,
    -165,  -165,  -165,    15,    16,  -165,  -165,    17,    18,  -165,
    -165,  -165,  -165,    19,    20,    21,  -165,     0,    22,  -165,
       0,     0,  -165,  -165,    23,    24,     0,  -165,  -165,  -165,
      25,    26,    27,  -165,  -165,    28,  -165,  -165,  -165,  -165,
      29,    30,     0,    31,  -165,  -165,     0,    32,    33,    34,
       0,    35,    36,    37,     0,  -165,    38,     0,    39,     0,
      40,    41,    42,  -165,    43,    44,     0,    45,     0,     0,
       0,     0,     0,     0,     0,     0,    46,  -165,    47,  -165,
       0,     0,     0,     0,     0,     0,    48,     0,  -165,    50,
      51,    52,  -165,    53,    54,  -165,    55,     0,  -165,    56,
      57,    58,  -165,    59,     0,    60,  -165,    61,    62,  -165,
      63,    64,     1,     0,     2,     3,     4,     5,     6,     0,
       7,     8,     9,    10,    11,  -118,  -118,  -118,   410,     0,
      13,    14,  -118,  -118,  -118,  -118,    15,    16,  -118,  -118,
      17,    18,  -118,  -118,  -118,  -118,    19,    20,    21,  -118,
       0,    22,     0,     0,     0,     0,  -118,    23,    24,     0,
       0,  -118,     0,    25,    26,    27,     0,     0,    28,     0,
       0,     0,     0,    29,    30,     0,    31,     0,  -118,     0,
      32,    33,    34,     0,    35,    36,    37,   115,     0,    38,
       0,    39,     0,    40,    41,    42,  -118,    43,    44,     0,
      45,     0,     0,     0,     0,     0,     0,     0,     0,    46,
       0,    47,     0,     0,     0,     0,     0,     0,     0,    48,
      49,     0,    50,    51,    52,  -118,    53,    54,  -118,    55,
       0,  -118,    56,    57,    58,  -118,    59,     0,    60,  -118,
      61,    62,  -118,    63,    64,     1,     0,     2,     3,     4,
       5,     6,     0,     7,     8,     9,    10,    11,  -118,  -118,
    -118,   410,     0,    13,    14,  -118,  -118,  -118,  -118,    15,
      16,  -118,  -118,    17,    18,  -118,  -118,  -118,  -118,    19,
      20,    21,  -118,     0,    22,     0,     0,     0,     0,  -118,
      23,    24,     0,     0,     0,     0,    25,    26,    27,     0,
       0,    28,     0,     0,  -118,     0,    29,    30,     0,    31,
       0,  -118,     0,    32,    33,    34,     0,    35,    36,    37,
     115,     0,    38,     0,    39,     0,    40,    41,    42,  -118,
      43,    44,     0,    45,     0,     0,     0,     0,     0,     0,
       0,     0,    46,     0,    47,     0,     0,     0,     0,     0,
       0,     0,    48,    49,     0,    50,    51,    52,  -118,    53,
      54,  -118,    55,     0,  -118,    56,    57,    58,  -118,    59,
       0,    60,  -118,    61,    62,  -118,    63,    64,     1,     0,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,   -83,   -83,   -83,    12,     0,    13,    14,   -83,   -83,
     -83,   -83,    15,   211,   -83,   -83,    17,    18,   -83,   -83,
     -83,   -83,    19,    20,    21,   -83,     0,    22,     0,     0,
       0,     0,   -83,    23,    24,     0,     0,     0,     0,    25,
      26,    27,     0,     0,    28,     0,     0,     0,     0,    29,
      30,     0,    31,     0,   -83,     0,    32,    33,    34,     0,
      35,    36,    37,     0,     0,    38,     0,    39,     0,    40,
      41,    42,   -83,    43,    44,     0,    45,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,    47,     0,     0,
       0,     0,     0,     0,     0,    48,   121,     0,    50,    51,
      52,   -83,    53,    54,   -83,    55,     0,   -83,    56,    57,
      58,   -83,    59,     0,    60,   -83,    61,    62,   -83,    63,
      64,     1,     0,     2,     3,     4,     5,     6,     0,     7,
       8,     9,    10,    11,     0,     0,     0,    12,     0,    13,
      14,     0,     0,     0,     0,    15,    16,     0,     0,    17,
      18,     0,     0,     0,     0,    19,    20,    21,     0,     0,
      22,     0,     0,     0,   235,     0,    23,    24,     0,   413,
       0,   414,    25,    26,    27,     0,     0,    28,     0,     0,
       0,     0,    29,    30,     0,    31,     0,     0,     0,    32,
      33,    34,     0,    35,    36,    37,     0,   259,    38,     0,
      39,     0,    40,    41,    42,     0,    43,    44,     0,    45,
       0,     0,     0,     0,     0,     0,     0,     0,    46,     0,
      47,     0,     0,     0,     0,     0,     0,     0,    48,   415,
       0,    50,    51,    52,     0,    53,    54,     0,    55,     0,
       0,    56,    57,    58,     0,    59,     0,    60,     0,    61,
      62,     0,    63,    64,     1,     0,     2,     3,     4,     5,
       6,     0,     7,     8,     9,    10,    11,     0,     0,     0,
      12,     0,    13,    14,     0,     0,     0,     0,    15,    16,
       0,     0,    17,    18,     0,     0,     0,     0,    19,    20,
      21,     0,     0,    22,     0,     0,     0,   235,     0,    23,
      24,     0,     0,     0,   450,    25,    26,    27,     0,     0,
      28,     0,     0,     0,     0,    29,    30,     0,    31,     0,
       0,     0,    32,    33,    34,     0,    35,    36,    37,     0,
     451,    38,     0,    39,     0,    40,    41,    42,     0,    43,
      44,     0,    45,     0,     0,     0,     0,     0,     0,     0,
       0,    46,     0,    47,     0,     0,     0,     0,     0,     0,
       0,    48,    49,     0,    50,    51,    52,     0,    53,    54,
       0,    55,     0,     0,    56,    57,    58,     0,    59,     0,
      60,     0,    61,    62,     0,    63,    64,     1,     0,     2,
       3,     4,     5,     6,     0,     7,     8,     9,    10,    11,
       0,     0,     0,    12,     0,    13,    14,     0,     0,     0,
       0,    15,    16,     0,     0,    17,    18,     0,     0,     0,
       0,    19,    20,    21,     0,     0,    22,     0,     0,     0,
       0,     0,    23,    24,     0,   475,     0,   476,    25,    26,
      27,     0,     0,    28,     0,     0,     0,     0,    29,    30,
       0,    31,     0,     0,     0,    32,    33,    34,     0,    35,
      36,    37,     0,   378,    38,     0,    39,     0,    40,    41,
      42,     0,    43,    44,     0,    45,     0,     0,     0,     0,
       0,     0,     0,     0,    46,     0,    47,     0,     0,     0,
       0,     0,     0,     0,    48,    49,     0,    50,    51,    52,
       0,    53,    54,     0,    55,     0,     0,    56,    57,    58,
       0,    59,     0,    60,     0,    61,    62,     0,    63,    64,
       1,     0,     2,     3,     4,     5,     6,     0,     7,     8,
       9,    10,    11,     0,     0,     0,    12,     0,    13,    14,
       0,     0,     0,     0,    15,    16,     0,     0,    17,    18,
       0,     0,     0,     0,    19,    20,    21,     0,     0,    22,
       0,     0,     0,     0,     0,    23,    24,     0,     0,     0,
       0,    25,    26,    27,     0,     0,    28,     0,     0,     0,
       0,    29,    30,     0,    31,     0,     0,     0,    32,    33,
      34,     0,    35,    36,    37,   494,     0,    38,  -200,    39,
     239,    40,    41,    42,     0,    43,    44,     0,    45,     0,
       0,     0,     0,     0,     0,     0,     0,    46,     0,    47,
       0,     0,     0,     0,     0,     0,     0,    48,    49,     0,
      50,    51,    52,     0,    53,    54,     0,    55,     0,     0,
      56,    57,    58,     0,    59,     0,    60,     0,    61,    62,
       0,    63,    64,     1,     0,     2,     3,     4,     5,     6,
       0,     7,     8,   522,    10,    11,     0,     0,     0,    12,
       0,    13,    14,     0,     0,     0,     0,    15,    16,     0,
       0,    17,    18,     0,     0,     0,     0,    19,    20,    21,
       0,     0,    22,     0,     0,     0,   235,     0,    23,    24,
       0,     0,     0,     0,    25,    26,    27,   223,  -253,    28,
       0,     0,     0,     0,    29,    30,     0,    31,     0,     0,
       0,    32,    33,    34,     0,    35,    36,    37,     0,     0,
      38,     0,    39,     0,    40,    41,    42,     0,    43,    44,
       0,    45,     0,     0,     0,     0,     0,     0,     0,     0,
      46,     0,    47,     0,     0,     0,     0,     0,     0,     0,
      48,    49,     0,    50,    51,    52,     0,    53,    54,     0,
      55,     0,     0,    56,    57,    58,     0,    59,     0,    60,
       0,    61,    62,     0,    63,    64,     1,     0,     2,     3,
       4,     5,     6,     0,     7,     8,     9,    10,    11,     0,
       0,     0,    12,     0,    13,    14,     0,     0,     0,     0,
      15,    16,     0,     0,    17,    18,     0,     0,     0,     0,
      19,    20,    21,     0,     0,    22,     0,     0,     0,   235,
       0,    23,    24,     0,     0,     0,   525,    25,    26,    27,
       0,     0,    28,     0,     0,     0,     0,    29,    30,     0,
      31,     0,     0,     0,    32,    33,    34,     0,    35,    36,
      37,     0,   526,    38,     0,    39,     0,    40,    41,    42,
       0,    43,    44,     0,    45,     0,     0,     0,     0,     0,
       0,     0,     0,    46,     0,    47,     0,     0,     0,     0,
       0,     0,     0,    48,    49,     0,    50,    51,    52,     0,
      53,    54,     0,    55,     0,     0,    56,    57,    58,     0,
      59,     0,    60,     0,    61,    62,     0,    63,    64,     1,
       0,     2,     3,     4,     5,     6,     0,     7,     8,     9,
      10,    11,     0,     0,     0,    12,     0,    13,    14,     0,
       0,     0,     0,    15,    16,     0,     0,    17,    18,     0,
       0,     0,     0,    19,    20,    21,     0,     0,    22,     0,
       0,     0,   235,     0,    23,    24,     0,     0,     0,     0,
      25,    26,    27,     0,     0,    28,     0,     0,     0,     0,
      29,    30,     0,    31,     0,     0,     0,    32,    33,    34,
       0,    35,    36,    37,     0,   259,    38,     0,    39,     0,
      40,    41,    42,     0,    43,    44,     0,    45,     0,     0,
       0,     0,     0,     0,     0,     0,    46,     0,    47,     0,
       0,     0,     0,     0,     0,     0,    48,   564,   565,    50,
      51,    52,     0,    53,    54,     0,    55,     0,     0,    56,
      57,    58,     0,    59,     0,    60,     0,    61,    62,     0,
      63,    64,     1,     0,     2,     3,     4,     5,     6,     0,
       7,     8,     9,    10,    11,     0,     0,     0,    12,     0,
      13,    14,     0,     0,     0,     0,    15,    16,     0,     0,
      17,    18,     0,     0,     0,     0,    19,    20,    21,     0,
       0,    22,     0,     0,     0,   235,     0,    23,    24,     0,
       0,     0,     0,    25,    26,    27,     0,     0,    28,     0,
       0,     0,     0,    29,    30,     0,    31,   236,     0,     0,
      32,    33,    34,     0,    35,    36,    37,     0,     0,    38,
       0,    39,     0,    40,    41,    42,     0,    43,    44,     0,
      45,     0,     0,     0,     0,     0,     0,     0,     0,    46,
       0,    47,     0,     0,     0,     0,     0,     0,     0,    48,
      49,     0,    50,    51,    52,     0,    53,    54,     0,    55,
       0,     0,    56,    57,    58,     0,    59,     0,    60,     0,
      61,    62,     0,    63,    64,     1,     0,     2,     3,     4,
       5,     6,     0,     7,     8,     9,    10,    11,     0,     0,
       0,    12,     0,    13,    14,     0,     0,     0,     0,    15,
      16,     0,     0,    17,    18,     0,     0,     0,     0,    19,
      20,    21,     0,     0,    22,     0,     0,     0,     0,     0,
      23,    24,     0,     0,     0,     0,    25,    26,    27,     0,
       0,    28,     0,     0,     0,     0,    29,    30,     0,    31,
       0,     0,     0,    32,    33,    34,     0,    35,    36,    37,
       0,     0,    38,  -200,    39,   239,    40,    41,    42,     0,
      43,    44,     0,    45,     0,     0,     0,     0,     0,     0,
       0,     0,    46,     0,    47,     0,     0,     0,     0,     0,
       0,     0,    48,    49,     0,    50,    51,    52,     0,    53,
      54,     0,    55,     0,     0,    56,    57,    58,     0,    59,
       0,    60,     0,    61,    62,     0,    63,    64,     1,     0,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,     0,     0,     0,    12,     0,    13,    14,     0,     0,
       0,     0,    15,    16,     0,     0,    17,    18,     0,     0,
       0,     0,    19,    20,    21,     0,     0,    22,     0,     0,
       0,   235,     0,    23,    24,     0,     0,     0,     0,    25,
      26,    27,     0,     0,    28,     0,     0,     0,     0,    29,
      30,     0,    31,     0,     0,     0,    32,    33,    34,     0,
      35,    36,    37,     0,   259,    38,     0,    39,     0,    40,
      41,    42,     0,    43,    44,     0,    45,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,    47,     0,     0,
       0,     0,     0,     0,     0,    48,   260,     0,    50,    51,
      52,     0,    53,    54,     0,    55,     0,     0,    56,    57,
      58,     0,    59,     0,    60,     0,    61,    62,     0,    63,
      64,     1,     0,     2,     3,     4,     5,     6,     0,     7,
       8,     9,    10,    11,     0,     0,     0,    12,     0,    13,
      14,     0,     0,     0,     0,    15,    16,     0,     0,    17,
      18,     0,     0,     0,     0,    19,    20,    21,     0,     0,
      22,     0,     0,     0,   235,     0,    23,    24,     0,     0,
       0,   264,    25,    26,    27,     0,     0,    28,     0,     0,
       0,     0,    29,    30,     0,    31,     0,     0,     0,    32,
      33,    34,     0,    35,    36,    37,     0,     0,    38,     0,
      39,     0,    40,    41,    42,     0,    43,    44,     0,    45,
       0,     0,     0,     0,     0,     0,     0,     0,    46,     0,
      47,     0,     0,     0,     0,     0,     0,     0,    48,   265,
       0,    50,    51,    52,     0,    53,    54,     0,    55,     0,
       0,    56,    57,    58,     0,    59,     0,    60,     0,    61,
      62,     0,    63,    64,     1,     0,     2,     3,     4,     5,
       6,     0,     7,     8,     9,    10,    11,     0,     0,     0,
      12,     0,    13,    14,     0,     0,     0,     0,    15,    16,
       0,     0,    17,    18,     0,     0,     0,     0,    19,    20,
      21,     0,     0,    22,     0,     0,     0,   235,     0,    23,
      24,     0,     0,   274,     0,    25,    26,    27,     0,     0,
      28,     0,     0,     0,     0,    29,    30,     0,    31,     0,
       0,     0,    32,    33,    34,     0,    35,    36,    37,     0,
       0,    38,     0,    39,     0,    40,    41,    42,     0,    43,
      44,     0,    45,     0,     0,     0,     0,     0,     0,     0,
       0,    46,     0,    47,     0,     0,     0,     0,     0,     0,
       0,    48,    49,     0,    50,    51,    52,     0,    53,    54,
       0,    55,     0,     0,    56,    57,    58,     0,    59,     0,
      60,     0,    61,    62,     0,    63,    64,     1,     0,     2,
       3,     4,     5,     6,     0,     7,     8,     9,    10,    11,
       0,     0,     0,    12,     0,    13,    14,     0,     0,     0,
       0,    15,    16,     0,     0,    17,    18,     0,     0,     0,
       0,    19,    20,    21,     0,     0,    22,     0,     0,     0,
     235,     0,    23,    24,     0,     0,     0,     0,    25,    26,
      27,     0,     0,    28,     0,     0,     0,     0,    29,    30,
       0,    31,     0,     0,     0,    32,    33,    34,     0,    35,
      36,    37,     0,   364,    38,     0,    39,     0,    40,    41,
      42,     0,    43,    44,     0,    45,     0,     0,     0,     0,
       0,     0,     0,     0,    46,     0,    47,     0,     0,     0,
       0,     0,     0,     0,    48,    49,     0,    50,    51,    52,
       0,    53,    54,     0,    55,     0,     0,    56,    57,    58,
       0,    59,     0,    60,     0,    61,    62,     0,    63,    64,
       1,     0,     2,     3,     4,     5,     6,     0,     7,     8,
       9,    10,    11,     0,     0,     0,    12,     0,    13,    14,
       0,     0,     0,     0,    15,    16,     0,     0,    17,    18,
       0,     0,     0,     0,    19,    20,    21,     0,     0,    22,
       0,     0,     0,   235,     0,    23,    24,     0,     0,     0,
       0,    25,    26,    27,     0,     0,    28,     0,     0,     0,
       0,    29,    30,     0,    31,     0,     0,     0,    32,    33,
      34,     0,    35,    36,    37,     0,  -257,    38,     0,    39,
       0,    40,    41,    42,     0,    43,    44,     0,    45,     0,
       0,     0,     0,     0,     0,     0,     0,    46,     0,    47,
       0,     0,     0,     0,     0,     0,     0,    48,    49,     0,
      50,    51,    52,     0,    53,    54,     0,    55,     0,     0,
      56,    57,    58,     0,    59,     0,    60,     0,    61,    62,
       0,    63,    64,     1,     0,     2,     3,     4,     5,     6,
       0,     7,     8,     9,    10,    11,     0,     0,     0,    12,
       0,    13,    14,     0,     0,     0,     0,    15,    16,     0,
       0,    17,    18,     0,     0,     0,     0,    19,    20,    21,
       0,     0,    22,     0,     0,     0,   235,     0,    23,    24,
       0,     0,     0,     0,    25,    26,    27,     0,  -254,    28,
       0,     0,     0,     0,    29,    30,     0,    31,     0,     0,
       0,    32,    33,    34,     0,    35,    36,    37,     0,     0,
      38,     0,    39,     0,    40,    41,    42,     0,    43,    44,
       0,    45,     0,     0,     0,     0,     0,     0,     0,     0,
      46,     0,    47,     0,     0,     0,     0,     0,     0,     0,
      48,    49,     0,    50,    51,    52,     0,    53,    54,     0,
      55,     0,     0,    56,    57,    58,     0,    59,     0,    60,
       0,    61,    62,     0,    63,    64,     1,     0,     2,     3,
       4,     5,     6,     0,     7,     8,     9,    10,    11,     0,
       0,     0,    12,     0,    13,    14,     0,     0,     0,     0,
      15,    16,     0,     0,    17,    18,     0,     0,     0,     0,
      19,    20,    21,     0,     0,    22,     0,     0,     0,   235,
       0,    23,    24,     0,     0,     0,     0,    25,    26,    27,
       0,     0,    28,     0,     0,     0,     0,    29,    30,     0,
      31,     0,     0,     0,    32,    33,    34,     0,    35,    36,
      37,     0,   432,    38,     0,    39,     0,    40,    41,    42,
       0,    43,    44,     0,    45,     0,     0,     0,     0,     0,
       0,     0,     0,    46,     0,    47,     0,     0,     0,     0,
       0,     0,     0,    48,    49,     0,    50,    51,    52,     0,
      53,    54,     0,    55,     0,     0,    56,    57,    58,     0,
      59,     0,    60,     0,    61,    62,     0,    63,    64,     1,
       0,     2,     3,     4,     5,     6,     0,     7,     8,     9,
      10,    11,     0,     0,     0,    12,     0,    13,    14,     0,
       0,     0,     0,    15,    16,     0,     0,    17,    18,     0,
       0,     0,     0,    19,    20,    21,     0,     0,    22,     0,
       0,     0,   235,     0,    23,    24,     0,     0,     0,     0,
      25,    26,    27,     0,     0,    28,     0,     0,     0,     0,
      29,    30,     0,    31,     0,     0,     0,    32,    33,    34,
       0,    35,    36,    37,     0,   433,    38,     0,    39,     0,
      40,    41,    42,     0,    43,    44,     0,    45,     0,     0,
       0,     0,     0,     0,     0,     0,    46,     0,    47,     0,
       0,     0,     0,     0,     0,     0,    48,    49,     0,    50,
      51,    52,     0,    53,    54,     0,    55,     0,     0,    56,
      57,    58,     0,    59,     0,    60,     0,    61,    62,     0,
      63,    64,     1,     0,     2,     3,     4,     5,     6,     0,
       7,     8,     9,    10,    11,     0,     0,     0,    12,     0,
      13,    14,     0,     0,     0,     0,    15,    16,     0,     0,
      17,    18,     0,     0,     0,     0,    19,    20,    21,     0,
       0,    22,     0,     0,     0,   235,     0,    23,    24,     0,
       0,     0,     0,    25,    26,    27,     0,     0,    28,     0,
       0,     0,     0,    29,    30,     0,    31,     0,     0,     0,
      32,    33,    34,     0,    35,    36,    37,     0,   437,    38,
       0,    39,     0,    40,    41,    42,     0,    43,    44,     0,
      45,     0,     0,     0,     0,     0,     0,     0,     0,    46,
       0,    47,     0,     0,     0,     0,     0,     0,     0,    48,
      49,     0,    50,    51,    52,     0,    53,    54,     0,    55,
       0,     0,    56,    57,    58,     0,    59,     0,    60,     0,
      61,    62,     0,    63,    64,     1,     0,     2,     3,     4,
       5,     6,     0,     7,     8,     9,    10,    11,     0,     0,
       0,    12,     0,    13,    14,     0,     0,     0,     0,    15,
      16,     0,     0,    17,    18,     0,     0,     0,     0,    19,
      20,    21,     0,     0,    22,     0,     0,     0,     0,     0,
      23,    24,     0,     0,     0,     0,    25,    26,    27,     0,
       0,    28,     0,     0,     0,     0,    29,    30,     0,    31,
       0,     0,     0,    32,    33,    34,     0,    35,    36,    37,
     492,     0,    38,   360,    39,     0,    40,    41,    42,     0,
      43,    44,     0,    45,     0,     0,     0,     0,     0,     0,
       0,     0,    46,     0,    47,     0,     0,     0,     0,     0,
       0,     0,    48,    49,     0,    50,    51,    52,     0,    53,
      54,     0,    55,     0,     0,    56,    57,    58,     0,    59,
       0,    60,     0,    61,    62,     0,    63,    64,     1,     0,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,     0,     0,     0,    12,     0,    13,    14,     0,     0,
       0,     0,    15,    16,     0,     0,    17,    18,     0,     0,
       0,     0,    19,    20,    21,     0,     0,    22,     0,     0,
       0,   235,     0,    23,    24,     0,     0,     0,     0,    25,
      26,    27,     0,     0,    28,     0,     0,     0,     0,    29,
      30,     0,    31,     0,     0,     0,    32,    33,    34,     0,
      35,    36,    37,     0,   500,    38,     0,    39,     0,    40,
      41,    42,     0,    43,    44,     0,    45,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,    47,     0,     0,
       0,     0,     0,     0,     0,    48,    49,     0,    50,    51,
      52,     0,    53,    54,     0,    55,     0,     0,    56,    57,
      58,     0,    59,     0,    60,     0,    61,    62,     0,    63,
      64,     1,     0,     2,     3,     4,     5,     6,     0,     7,
       8,     9,    10,    11,     0,     0,     0,    12,     0,    13,
      14,     0,     0,     0,     0,    15,    16,     0,     0,    17,
      18,     0,     0,     0,     0,    19,    20,    21,     0,     0,
      22,     0,     0,     0,   235,     0,    23,    24,     0,     0,
       0,     0,    25,    26,    27,     0,     0,    28,     0,     0,
       0,     0,    29,    30,     0,    31,     0,     0,     0,    32,
      33,    34,     0,    35,    36,    37,     0,   531,    38,     0,
      39,     0,    40,    41,    42,     0,    43,    44,     0,    45,
       0,     0,     0,     0,     0,     0,     0,     0,    46,     0,
      47,     0,     0,     0,     0,     0,     0,     0,    48,    49,
       0,    50,    51,    52,     0,    53,    54,     0,    55,     0,
       0,    56,    57,    58,     0,    59,     0,    60,     0,    61,
      62,     0,    63,    64,     1,     0,     2,     3,     4,     5,
       6,     0,     7,     8,     9,    10,    11,     0,     0,     0,
      12,     0,    13,    14,     0,     0,     0,     0,    15,    16,
       0,     0,    17,    18,     0,     0,     0,     0,    19,    20,
      21,     0,     0,    22,     0,     0,     0,   235,     0,    23,
      24,     0,     0,     0,     0,    25,    26,    27,     0,     0,
      28,     0,     0,     0,     0,    29,    30,     0,    31,     0,
       0,     0,    32,    33,    34,     0,    35,    36,    37,     0,
     535,    38,     0,    39,     0,    40,    41,    42,     0,    43,
      44,     0,    45,     0,     0,     0,     0,     0,     0,     0,
       0,    46,     0,    47,     0,     0,     0,     0,     0,     0,
       0,    48,    49,     0,    50,    51,    52,     0,    53,    54,
       0,    55,     0,     0,    56,    57,    58,     0,    59,     0,
      60,     0,    61,    62,     0,    63,    64,     1,     0,     2,
       3,     4,     5,     6,     0,     7,     8,     9,    10,    11,
       0,     0,     0,    12,     0,    13,    14,     0,     0,     0,
       0,    15,    16,     0,     0,    17,    18,     0,     0,     0,
       0,    19,    20,    21,     0,     0,    22,     0,     0,     0,
     235,     0,    23,    24,     0,     0,     0,     0,    25,    26,
      27,     0,     0,    28,     0,     0,     0,     0,    29,    30,
       0,    31,     0,     0,     0,    32,    33,    34,     0,    35,
      36,    37,     0,   538,    38,     0,    39,     0,    40,    41,
      42,     0,    43,    44,     0,    45,     0,     0,     0,     0,
       0,     0,     0,     0,    46,     0,    47,     0,     0,     0,
       0,     0,     0,     0,    48,    49,     0,    50,    51,    52,
       0,    53,    54,     0,    55,     0,     0,    56,    57,    58,
       0,    59,     0,    60,     0,    61,    62,     0,    63,    64,
       1,     0,     2,     3,     4,     5,     6,     0,     7,     8,
       9,    10,    11,     0,     0,     0,    12,     0,    13,    14,
       0,     0,     0,     0,    15,    16,     0,     0,    17,    18,
       0,     0,     0,     0,    19,    20,    21,     0,     0,    22,
       0,     0,     0,   235,     0,    23,    24,     0,     0,     0,
       0,    25,    26,    27,     0,     0,    28,     0,     0,     0,
       0,    29,    30,     0,    31,     0,     0,     0,    32,    33,
      34,     0,    35,    36,    37,     0,   545,    38,     0,    39,
       0,    40,    41,    42,     0,    43,    44,     0,    45,     0,
       0,     0,     0,     0,     0,     0,     0,    46,     0,    47,
       0,     0,     0,     0,     0,     0,     0,    48,    49,     0,
      50,    51,    52,     0,    53,    54,     0,    55,     0,     0,
      56,    57,    58,     0,    59,     0,    60,     0,    61,    62,
       0,    63,    64,     1,     0,     2,     3,     4,     5,     6,
       0,     7,     8,     9,    10,    11,     0,     0,     0,    12,
       0,    13,    14,     0,     0,     0,     0,    15,    16,     0,
       0,    17,    18,     0,     0,     0,     0,    19,    20,    21,
       0,     0,    22,     0,     0,     0,   235,     0,    23,    24,
       0,     0,     0,     0,    25,    26,    27,     0,     0,    28,
       0,     0,     0,     0,    29,    30,     0,    31,     0,     0,
       0,    32,    33,    34,     0,    35,    36,    37,     0,   560,
      38,     0,    39,     0,    40,    41,    42,     0,    43,    44,
       0,    45,     0,     0,     0,     0,     0,     0,     0,     0,
      46,     0,    47,     0,     0,     0,     0,     0,     0,     0,
      48,    49,     0,    50,    51,    52,     0,    53,    54,     0,
      55,     0,     0,    56,    57,    58,     0,    59,     0,    60,
       0,    61,    62,     0,    63,    64,     1,     0,     2,     3,
       4,     5,     6,     0,     7,     8,     9,    10,    11,     0,
       0,     0,    12,     0,    13,    14,     0,     0,     0,     0,
      15,    16,     0,     0,    17,    18,     0,     0,     0,     0,
      19,    20,    21,     0,     0,    22,     0,     0,     0,   235,
       0,    23,    24,     0,     0,     0,     0,    25,    26,    27,
       0,     0,    28,     0,     0,     0,     0,    29,    30,     0,
      31,     0,     0,     0,    32,    33,    34,     0,    35,    36,
      37,     0,   562,    38,     0,    39,     0,    40,    41,    42,
       0,    43,    44,     0,    45,     0,     0,     0,     0,     0,
       0,     0,     0,    46,     0,    47,     0,     0,     0,     0,
       0,     0,     0,    48,    49,     0,    50,    51,    52,     0,
      53,    54,     0,    55,     0,     0,    56,    57,    58,     0,
      59,     0,    60,     0,    61,    62,     0,    63,    64,     1,
       0,     2,     3,     4,     5,     6,     0,     7,     8,     9,
      10,    11,     0,     0,     0,    12,     0,    13,    14,     0,
       0,     0,     0,    15,    16,     0,     0,    17,    18,     0,
       0,     0,     0,    19,    20,    21,     0,     0,    22,     0,
       0,     0,   235,     0,    23,    24,     0,     0,     0,     0,
      25,    26,    27,     0,     0,    28,     0,     0,     0,     0,
      29,    30,     0,    31,     0,     0,     0,    32,    33,    34,
       0,    35,    36,    37,     0,   572,    38,     0,    39,     0,
      40,    41,    42,     0,    43,    44,     0,    45,     0,     0,
       0,     0,     0,     0,     0,     0,    46,     0,    47,     0,
       0,     0,     0,     0,     0,     0,    48,    49,     0,    50,
      51,    52,     0,    53,    54,     0,    55,     0,     0,    56,
      57,    58,     0,    59,     0,    60,     0,    61,    62,     0,
      63,    64,     1,     0,     2,     3,     4,     5,     6,     0,
       7,     8,     9,    10,    11,     0,     0,     0,    12,     0,
      13,    14,     0,     0,     0,     0,    15,    16,     0,     0,
      17,    18,     0,     0,     0,     0,    19,    20,    21,     0,
       0,    22,     0,     0,     0,   235,     0,    23,    24,     0,
       0,     0,     0,    25,    26,    27,     0,     0,    28,     0,
       0,     0,     0,    29,    30,     0,    31,     0,     0,     0,
      32,    33,    34,     0,    35,    36,    37,     0,   579,    38,
       0,    39,     0,    40,    41,    42,     0,    43,    44,     0,
      45,     0,     0,     0,     0,     0,     0,     0,     0,    46,
       0,    47,     0,     0,     0,     0,     0,     0,     0,    48,
      49,     0,    50,    51,    52,     0,    53,    54,     0,    55,
       0,     0,    56,    57,    58,     0,    59,     0,    60,     0,
      61,    62,     0,    63,    64,     1,     0,     2,     3,     4,
       5,     6,     0,     7,     8,     9,    10,    11,     0,     0,
       0,    12,     0,    13,    14,     0,     0,     0,     0,    15,
      16,     0,     0,    17,    18,     0,     0,     0,     0,    19,
      20,    21,     0,     0,    22,     0,     0,     0,   235,     0,
      23,    24,     0,     0,     0,     0,    25,    26,    27,     0,
       0,    28,     0,     0,     0,     0,    29,    30,     0,    31,
       0,     0,     0,    32,    33,    34,     0,    35,    36,    37,
       0,   580,    38,     0,    39,     0,    40,    41,    42,     0,
      43,    44,     0,    45,     0,     0,     0,     0,     0,     0,
       0,     0,    46,     0,    47,     0,     0,     0,     0,     0,
       0,     0,    48,    49,     0,    50,    51,    52,     0,    53,
      54,     0,    55,     0,     0,    56,    57,    58,     0,    59,
       0,    60,     0,    61,    62,     0,    63,    64,     1,     0,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,     0,     0,     0,    12,     0,    13,    14,     0,     0,
       0,     0,    15,    16,     0,     0,    17,    18,     0,     0,
       0,     0,    19,    20,    21,     0,     0,    22,     0,     0,
       0,   235,     0,    23,    24,     0,     0,     0,     0,    25,
      26,    27,     0,     0,    28,     0,     0,     0,     0,    29,
      30,     0,    31,     0,     0,     0,    32,    33,    34,     0,
      35,    36,    37,     0,   581,    38,     0,    39,     0,    40,
      41,    42,     0,    43,    44,     0,    45,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,    47,     0,     0,
       0,     0,     0,     0,     0,    48,    49,     0,    50,    51,
      52,     0,    53,    54,     0,    55,     0,     0,    56,    57,
      58,     0,    59,     0,    60,     0,    61,    62,     0,    63,
      64,     1,     0,     2,     3,     4,     5,     6,     0,     7,
       8,     9,    10,    11,     0,     0,     0,    12,     0,    13,
      14,     0,     0,     0,     0,    15,    16,     0,     0,    17,
      18,     0,     0,     0,     0,    19,    20,    21,     0,     0,
      22,     0,     0,     0,   235,     0,    23,    24,     0,     0,
       0,     0,    25,    26,    27,     0,     0,    28,     0,     0,
       0,     0,    29,    30,     0,    31,     0,     0,     0,    32,
      33,    34,     0,    35,    36,    37,     0,   582,    38,     0,
      39,     0,    40,    41,    42,     0,    43,    44,     0,    45,
       0,     0,     0,     0,     0,     0,     0,     0,    46,     0,
      47,     0,     0,     0,     0,     0,     0,     0,    48,    49,
       0,    50,    51,    52,     0,    53,    54,     0,    55,     0,
       0,    56,    57,    58,     0,    59,     0,    60,     0,    61,
      62,     0,    63,    64,     1,     0,     2,     3,     4,     5,
       6,     0,     7,     8,     9,    10,    11,     0,     0,     0,
      12,     0,    13,    14,     0,     0,     0,     0,    15,    16,
       0,     0,    17,    18,     0,     0,     0,     0,    19,    20,
      21,     0,     0,    22,     0,     0,     0,   235,     0,    23,
      24,     0,     0,     0,     0,    25,    26,    27,     0,     0,
      28,     0,     0,     0,     0,    29,    30,     0,    31,     0,
       0,     0,    32,    33,    34,     0,    35,    36,    37,     0,
     583,    38,     0,    39,     0,    40,    41,    42,     0,    43,
      44,     0,    45,     0,     0,     0,     0,     0,     0,     0,
       0,    46,     0,    47,     0,     0,     0,     0,     0,     0,
       0,    48,    49,     0,    50,    51,    52,     0,    53,    54,
       0,    55,     0,     0,    56,    57,    58,     0,    59,     0,
      60,     0,    61,    62,     0,    63,    64,     1,     0,     2,
       3,     4,     5,     6,     0,     7,     8,     9,    10,    11,
       0,     0,     0,    12,     0,    13,    14,     0,     0,     0,
       0,    15,    16,     0,     0,    17,    18,     0,     0,     0,
       0,    19,    20,    21,     0,     0,    22,     0,     0,     0,
       0,     0,    23,    24,     0,     0,     0,     0,    25,    26,
      27,     0,     0,    28,     0,     0,     0,     0,    29,    30,
       0,    31,     0,     0,     0,    32,    33,    34,     0,    35,
      36,    37,     0,   378,    38,     0,    39,     0,    40,    41,
      42,     0,    43,    44,     0,    45,     0,     0,     0,     0,
       0,     0,     0,     0,    46,     0,    47,     0,     0,     0,
       0,     0,     0,     0,    48,    49,   585,    50,    51,    52,
       0,    53,    54,     0,    55,     0,     0,    56,    57,    58,
       0,    59,     0,    60,     0,    61,    62,     0,    63,    64,
       1,     0,     2,     3,     4,     5,     6,     0,     7,     8,
       9,    10,    11,     0,     0,     0,    12,     0,    13,    14,
       0,     0,     0,     0,    15,    16,     0,     0,    17,    18,
       0,     0,     0,     0,    19,    20,    21,     0,     0,    22,
       0,     0,     0,   235,     0,    23,    24,     0,     0,     0,
       0,    25,    26,    27,     0,     0,    28,     0,     0,     0,
       0,    29,    30,     0,    31,     0,     0,     0,    32,    33,
      34,     0,    35,    36,    37,     0,   589,    38,     0,    39,
       0,    40,    41,    42,     0,    43,    44,     0,    45,     0,
       0,     0,     0,     0,     0,     0,     0,    46,     0,    47,
       0,     0,     0,     0,     0,     0,     0,    48,    49,     0,
      50,    51,    52,     0,    53,    54,     0,    55,     0,     0,
      56,    57,    58,     0,    59,     0,    60,     0,    61,    62,
       0,    63,    64,     1,     0,     2,     3,     4,     5,     6,
       0,     7,     8,     9,    10,    11,    89,     0,     0,    12,
       0,    13,    14,     0,     0,     0,     0,    15,    16,     0,
       0,    17,    18,     0,     0,     0,     0,    19,    20,    21,
       0,     0,    22,     0,     0,     0,     0,     0,    23,    24,
       0,     0,     0,     0,    25,    26,    27,     0,     0,    28,
       0,     0,     0,     0,    29,    30,     0,    31,     0,     0,
       0,    32,    33,    34,     0,    35,    36,    37,     0,     0,
      38,     0,    39,     0,    40,    41,    42,     0,    43,    44,
       0,    45,     0,     0,     0,     0,     0,     0,     0,     0,
      46,     0,    47,     0,     0,     0,     0,     0,     0,     0,
      48,    49,     0,    50,    51,    52,     0,    53,    54,     0,
      55,     0,     0,    56,    57,    58,     0,    59,     0,    60,
       0,    61,    62,     0,    63,    64,     1,     0,     2,     3,
       4,     5,     6,     0,     7,     8,     9,    10,    11,     0,
       0,     0,    12,     0,    13,    14,     0,     0,     0,     0,
      15,    16,     0,     0,    17,    18,     0,     0,     0,     0,
      19,    20,    21,     0,     0,    22,  -220,     0,     0,     0,
       0,    23,    24,     0,     0,     0,     0,    25,    26,    27,
       0,     0,    28,     0,     0,     0,     0,    29,    30,     0,
      31,     0,     0,     0,    32,    33,    34,     0,    35,    36,
      37,     0,     0,    38,     0,    39,     0,    40,    41,    42,
       0,    43,    44,     0,    45,     0,     0,     0,     0,     0,
       0,     0,     0,    46,     0,    47,     0,     0,     0,     0,
       0,     0,     0,    48,    49,     0,    50,    51,    52,     0,
      53,    54,     0,    55,     0,     0,    56,    57,    58,     0,
      59,     0,    60,     0,    61,    62,     0,    63,    64,     1,
       0,     2,     3,     4,     5,     6,     0,     7,     8,     9,
      10,    11,     0,     0,     0,    12,     0,    13,    14,     0,
       0,     0,     0,    15,    16,     0,     0,    17,    18,     0,
       0,     0,     0,    19,    20,    21,     0,     0,    22,     0,
       0,     0,     0,     0,    23,    24,     0,     0,     0,     0,
      25,    26,    27,     0,     0,    28,     0,     0,     0,     0,
      29,    30,     0,    31,     0,     0,     0,    32,    33,    34,
       0,    35,    36,    37,   115,     0,    38,     0,    39,     0,
      40,    41,    42,     0,    43,    44,     0,    45,     0,     0,
       0,     0,     0,     0,     0,     0,    46,     0,    47,     0,
       0,     0,     0,     0,     0,     0,    48,    49,     0,    50,
      51,    52,     0,    53,    54,     0,    55,     0,     0,    56,
      57,    58,     0,    59,     0,    60,     0,    61,    62,     0,
      63,    64,     1,     0,     2,     3,     4,     5,     6,     0,
       7,     8,     9,    10,    11,     0,     0,     0,    12,   204,
      13,    14,     0,     0,     0,     0,    15,    16,     0,     0,
      17,    18,     0,     0,     0,     0,    19,    20,    21,     0,
       0,    22,     0,     0,     0,     0,     0,    23,    24,     0,
       0,     0,     0,    25,    26,    27,     0,     0,    28,     0,
       0,     0,     0,    29,    30,     0,    31,     0,     0,     0,
      32,    33,    34,     0,    35,    36,    37,     0,     0,    38,
       0,    39,     0,    40,    41,    42,     0,    43,    44,     0,
      45,     0,     0,     0,     0,     0,     0,     0,     0,    46,
       0,    47,     0,     0,     0,     0,     0,     0,     0,    48,
      49,     0,    50,    51,    52,     0,    53,    54,     0,    55,
       0,     0,    56,    57,    58,     0,    59,     0,    60,     0,
      61,    62,     0,    63,    64,     1,     0,     2,     3,     4,
       5,     6,     0,     7,     8,     9,    10,    11,     0,     0,
       0,    12,   206,    13,    14,     0,     0,     0,     0,    15,
      16,     0,     0,    17,    18,     0,     0,     0,     0,    19,
      20,    21,     0,     0,    22,     0,     0,     0,     0,     0,
      23,    24,     0,     0,     0,     0,    25,    26,    27,     0,
       0,    28,     0,     0,     0,     0,    29,    30,     0,    31,
       0,     0,     0,    32,    33,    34,     0,    35,    36,    37,
       0,     0,    38,     0,    39,     0,    40,    41,    42,     0,
      43,    44,     0,    45,     0,     0,     0,     0,     0,     0,
       0,     0,    46,     0,    47,     0,     0,     0,     0,     0,
       0,     0,    48,    49,     0,    50,    51,    52,     0,    53,
      54,     0,    55,     0,     0,    56,    57,    58,     0,    59,
       0,    60,     0,    61,    62,     0,    63,    64,     1,     0,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,     0,     0,     0,    12,     0,    13,    14,     0,     0,
       0,     0,    15,    16,     0,     0,    17,    18,     0,     0,
       0,     0,    19,    20,    21,     0,     0,    22,     0,     0,
       0,  -218,     0,    23,    24,     0,     0,     0,     0,    25,
      26,    27,     0,     0,    28,     0,     0,     0,     0,    29,
      30,     0,    31,     0,     0,     0,    32,    33,    34,     0,
      35,    36,    37,     0,     0,    38,     0,    39,     0,    40,
      41,    42,     0,    43,    44,     0,    45,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,    47,     0,     0,
       0,     0,     0,     0,     0,    48,    49,     0,    50,    51,
      52,     0,    53,    54,     0,    55,     0,     0,    56,    57,
      58,     0,    59,     0,    60,     0,    61,    62,     0,    63,
      64,     1,     0,     2,     3,     4,     5,     6,     0,     7,
       8,     9,    10,    11,     0,     0,     0,    12,  -220,    13,
      14,     0,     0,     0,     0,    15,    16,     0,     0,    17,
      18,     0,     0,     0,     0,    19,    20,    21,     0,     0,
      22,     0,     0,     0,     0,     0,    23,    24,     0,     0,
       0,     0,    25,    26,    27,     0,     0,    28,     0,     0,
       0,     0,    29,    30,     0,    31,     0,     0,     0,    32,
      33,    34,     0,    35,    36,    37,     0,     0,    38,     0,
      39,     0,    40,    41,    42,     0,    43,    44,     0,    45,
       0,     0,     0,     0,     0,     0,     0,     0,    46,     0,
      47,     0,     0,     0,     0,     0,     0,     0,    48,    49,
       0,    50,    51,    52,     0,    53,    54,     0,    55,     0,
       0,    56,    57,    58,     0,    59,     0,    60,     0,    61,
      62,     0,    63,    64,     1,     0,     2,     3,     4,     5,
       6,     0,     7,     8,     9,    10,    11,     0,     0,     0,
      12,   270,    13,    14,     0,     0,     0,     0,    15,    16,
       0,     0,    17,    18,     0,     0,     0,     0,    19,    20,
      21,     0,     0,    22,     0,     0,     0,     0,     0,    23,
      24,     0,     0,     0,     0,    25,    26,    27,     0,     0,
      28,     0,     0,     0,     0,    29,    30,     0,    31,     0,
       0,     0,    32,    33,    34,     0,    35,    36,    37,     0,
       0,    38,     0,    39,     0,    40,    41,    42,     0,    43,
      44,     0,    45,     0,     0,     0,     0,     0,     0,     0,
       0,    46,     0,    47,     0,     0,     0,     0,     0,     0,
       0,    48,    49,     0,    50,    51,    52,     0,    53,    54,
       0,    55,     0,     0,    56,    57,    58,     0,    59,     0,
      60,     0,    61,    62,     0,    63,    64,     1,     0,     2,
       3,     4,     5,     6,   301,     7,     8,     9,    10,    11,
       0,     0,     0,    12,     0,    13,    14,     0,     0,     0,
       0,    15,    16,     0,     0,    17,    18,     0,     0,     0,
       0,    19,    20,    21,     0,     0,    22,     0,     0,     0,
       0,     0,    23,    24,     0,     0,     0,     0,    25,    26,
      27,     0,     0,    28,     0,     0,     0,     0,    29,    30,
       0,    31,     0,     0,     0,    32,    33,    34,     0,    35,
      36,    37,     0,     0,    38,     0,    39,     0,    40,    41,
      42,     0,    43,    44,     0,    45,     0,     0,     0,     0,
       0,     0,     0,     0,    46,     0,    47,     0,     0,     0,
       0,     0,     0,     0,    48,    49,     0,    50,    51,    52,
       0,    53,    54,     0,    55,     0,     0,    56,    57,    58,
       0,    59,     0,    60,     0,    61,    62,     0,    63,    64,
       1,     0,     2,     3,     4,     5,     6,     0,     7,     8,
       9,    10,    11,     0,     0,     0,    12,     0,    13,    14,
       0,     0,     0,     0,    15,    16,     0,     0,    17,    18,
       0,     0,     0,     0,    19,    20,    21,     0,     0,    22,
       0,     0,     0,     0,     0,    23,    24,     0,     0,     0,
       0,    25,    26,    27,     0,     0,    28,     0,     0,     0,
       0,    29,    30,     0,    31,     0,     0,     0,    32,    33,
      34,     0,    35,    36,    37,     0,     0,    38,   360,    39,
       0,    40,    41,    42,     0,    43,    44,     0,    45,     0,
       0,     0,     0,     0,     0,     0,     0,    46,     0,    47,
       0,     0,     0,     0,     0,     0,     0,    48,    49,     0,
      50,    51,    52,     0,    53,    54,     0,    55,     0,     0,
      56,    57,    58,     0,    59,     0,    60,     0,    61,    62,
       0,    63,    64,     1,     0,     2,     3,     4,     5,     6,
       0,     7,     8,     9,    10,    11,     0,     0,     0,    12,
       0,    13,    14,     0,     0,     0,     0,    15,    16,     0,
       0,    17,    18,     0,     0,     0,     0,    19,    20,    21,
       0,     0,    22,     0,     0,     0,     0,     0,    23,    24,
       0,     0,     0,     0,    25,    26,    27,     0,     0,    28,
       0,     0,     0,     0,    29,    30,     0,    31,     0,     0,
       0,    32,    33,    34,     0,    35,    36,    37,     0,   378,
      38,     0,    39,     0,    40,    41,    42,     0,    43,    44,
       0,    45,     0,     0,     0,     0,     0,     0,     0,     0,
      46,     0,    47,     0,     0,     0,     0,     0,     0,     0,
      48,    49,     0,    50,    51,    52,     0,    53,    54,     0,
      55,     0,     0,    56,    57,    58,     0,    59,     0,    60,
       0,    61,    62,     0,    63,    64,     1,     0,     2,     3,
       4,     5,     6,     0,     7,     8,     9,    10,    11,     0,
       0,     0,    12,     0,    13,    14,     0,     0,     0,     0,
      15,    16,     0,     0,    17,    18,     0,     0,     0,     0,
      19,    20,    21,     0,     0,    22,     0,     0,     0,     0,
       0,    23,    24,     0,     0,     0,     0,    25,    26,    27,
       0,     0,    28,     0,     0,     0,     0,    29,    30,     0,
      31,     0,     0,     0,    32,    33,    34,     0,    35,    36,
      37,     0,   259,    38,     0,    39,     0,    40,    41,    42,
       0,    43,    44,     0,    45,     0,     0,     0,     0,     0,
       0,     0,     0,    46,     0,    47,     0,     0,     0,     0,
       0,     0,     0,    48,   260,     0,    50,    51,    52,     0,
      53,    54,     0,    55,     0,     0,    56,    57,    58,     0,
      59,     0,    60,     0,    61,    62,     0,    63,    64,     1,
       0,     2,     3,     4,     5,     6,     0,     7,     8,     9,
      10,    11,     0,     0,     0,    12,     0,    13,    14,     0,
       0,     0,     0,    15,    16,     0,     0,    17,    18,     0,
       0,     0,     0,    19,    20,    21,     0,     0,    22,     0,
       0,     0,     0,     0,    23,    24,     0,     0,     0,   383,
      25,    26,    27,     0,     0,    28,     0,     0,     0,     0,
      29,    30,     0,    31,     0,     0,     0,    32,    33,    34,
       0,    35,    36,    37,     0,     0,    38,     0,    39,     0,
      40,    41,    42,     0,    43,    44,     0,    45,     0,     0,
       0,     0,     0,     0,     0,     0,    46,     0,    47,     0,
       0,     0,     0,     0,     0,     0,    48,    49,     0,    50,
      51,    52,     0,    53,    54,     0,    55,     0,     0,    56,
      57,    58,     0,    59,     0,    60,     0,    61,    62,     0,
      63,    64,     1,     0,     2,     3,     4,     5,     6,     0,
       7,     8,     9,    10,    11,     0,     0,     0,    12,     0,
      13,    14,     0,     0,     0,     0,    15,    16,     0,     0,
      17,    18,     0,     0,     0,     0,    19,    20,    21,     0,
       0,    22,     0,     0,     0,     0,     0,    23,    24,     0,
       0,     0,     0,    25,    26,    27,     0,     0,    28,     0,
       0,     0,     0,    29,    30,     0,    31,     0,     0,     0,
      32,    33,    34,     0,    35,   506,    37,   115,     0,    38,
       0,    39,     0,    40,    41,    42,     0,    43,    44,     0,
      45,     0,     0,     0,     0,     0,     0,     0,     0,    46,
       0,    47,     0,     0,     0,     0,     0,     0,     0,    48,
      49,     0,    50,    51,    52,     0,    53,    54,     0,    55,
       0,     0,    56,    57,    58,     0,    59,     0,    60,     0,
      61,    62,     0,    63,    64,     1,     0,     2,     3,     4,
       5,     6,     0,     7,     8,     9,    10,    11,     0,     0,
       0,    12,  -218,    13,    14,     0,     0,     0,     0,    15,
      16,     0,     0,    17,    18,     0,     0,     0,     0,    19,
      20,    21,     0,     0,    22,     0,     0,     0,     0,     0,
      23,    24,     0,     0,     0,     0,    25,    26,    27,     0,
       0,    28,     0,     0,     0,     0,    29,    30,     0,    31,
       0,     0,     0,    32,    33,    34,     0,    35,    36,    37,
       0,     0,    38,     0,    39,     0,    40,    41,    42,     0,
      43,    44,     0,    45,     0,     0,     0,     0,     0,     0,
       0,     0,    46,     0,    47,     0,     0,     0,     0,     0,
       0,     0,    48,    49,     0,    50,    51,    52,     0,    53,
      54,     0,    55,     0,     0,    56,    57,    58,     0,    59,
       0,    60,     0,    61,    62,     0,    63,    64,     1,     0,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,     0,     0,     0,    12,     0,    13,    14,     0,     0,
       0,     0,    15,    16,     0,     0,    17,    18,     0,     0,
       0,     0,    19,    20,    21,     0,     0,    22,     0,     0,
       0,     0,     0,    23,    24,     0,     0,     0,     0,    25,
      26,    27,     0,     0,    28,     0,     0,     0,     0,    29,
      30,     0,    31,     0,     0,     0,    32,    33,    34,     0,
      35,    36,    37,     0,     0,    38,     0,    39,     0,    40,
      41,    42,     0,    43,    44,     0,    45,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,    47,     0,     0,
       0,     0,     0,     0,     0,    48,    49,     0,    50,    51,
      52,     0,    53,    54,     0,    55,   541,     0,    56,    57,
      58,     0,    59,     0,    60,     0,    61,    62,     0,    63,
      64,     1,     0,     2,     3,     4,     5,     6,     0,     7,
       8,     9,    10,    11,     0,     0,     0,    12,     0,    13,
      14,     0,     0,     0,     0,    15,    16,     0,     0,    17,
      18,     0,     0,     0,     0,    19,    20,    21,     0,     0,
      22,     0,     0,     0,     0,     0,    23,    24,     0,     0,
       0,     0,    25,    26,    27,     0,     0,    28,     0,     0,
       0,     0,    29,    30,     0,    31,     0,     0,     0,    32,
      33,    34,     0,    35,    36,    37,     0,     0,    38,     0,
      39,     0,    40,    41,    42,     0,    43,    44,     0,    45,
       0,     0,     0,     0,     0,     0,     0,     0,    46,     0,
      47,     0,     0,     0,     0,     0,     0,     0,    48,    49,
     563,    50,    51,    52,     0,    53,    54,     0,    55,     0,
       0,    56,    57,    58,     0,    59,     0,    60,     0,    61,
      62,     0,    63,    64,     1,     0,     2,     3,     4,     5,
       6,     0,     7,     8,     9,    10,    11,     0,     0,     0,
      12,     0,    13,    14,     0,     0,     0,     0,    15,    16,
       0,     0,    17,    18,     0,     0,     0,     0,    19,    20,
      21,     0,     0,    22,     0,     0,     0,     0,     0,    23,
      24,     0,     0,     0,     0,    25,    26,    27,     0,     0,
      28,     0,     0,     0,     0,    29,    30,     0,    31,     0,
       0,     0,    32,    33,    34,     0,    35,    36,    37,     0,
       0,    38,     0,    39,     0,    40,    41,    42,     0,    43,
      44,     0,    45,     0,     0,     0,     0,     0,     0,     0,
       0,    46,     0,    47,     0,     0,     0,     0,     0,     0,
       0,    48,    49,   587,    50,    51,    52,     0,    53,    54,
       0,    55,     0,     0,    56,    57,    58,     0,    59,     0,
      60,     0,    61,    62,     0,    63,    64,     1,     0,     2,
       3,     4,     5,     6,     0,     7,     8,     9,    10,    11,
       0,     0,     0,    12,     0,    13,    14,     0,     0,     0,
       0,    15,    16,     0,     0,    17,    18,     0,     0,     0,
       0,    19,    20,    21,     0,     0,    22,     0,     0,     0,
       0,     0,    23,    24,     0,     0,     0,     0,    25,    26,
      27,     0,     0,    28,     0,     0,     0,     0,    29,    30,
       0,    31,     0,     0,     0,    32,    33,    34,     0,    35,
      36,    37,     0,     0,    38,     0,    39,     0,    40,    41,
      42,     0,    43,    44,     0,    45,     0,     0,     0,     0,
       0,     0,     0,     0,    46,     0,    47,     0,     0,     0,
       0,     0,     0,     0,    48,    49,     0,    50,    51,    52,
       0,    53,    54,     0,    55,     0,     0,    56,    57,    58,
       0,    59,     0,    60,     0,    61,    62,     0,    63,    64,
       1,     0,     2,     3,     4,     5,     6,     0,     7,     8,
       9,    10,    85,     0,     0,     0,    12,     0,    13,    14,
       0,     0,     0,     0,    15,    16,     0,     0,    17,    18,
       0,     0,     0,     0,    19,    20,    21,     0,     0,    22,
       0,     0,     0,     0,     0,    23,    24,     0,     0,     0,
       0,    25,    26,    27,     0,     0,    28,     0,     0,     0,
       0,    29,    30,     0,    31,     0,     0,     0,    32,    33,
      34,     0,    35,    36,    37,     0,     0,    86,     0,    39,
       0,    40,    41,    42,     0,    43,    44,     0,    45,     0,
       0,     0,     0,     0,     0,     0,     0,    46,     0,    47,
       0,     0,     0,     0,     0,     0,     0,    48,    49,     0,
      50,    51,    52,     0,    53,    54,     0,    55,     0,     0,
      56,    57,    58,     0,    59,     0,    60,     0,    61,    62,
       0,    63,    64,     1,     0,     2,     3,     4,     5,     6,
       0,     7,     8,     9,    10,    11,     0,     0,     0,    96,
       0,    13,    14,     0,     0,     0,     0,    15,    16,     0,
       0,    17,    18,     0,     0,     0,     0,    19,    20,    21,
       0,     0,    22,     0,     0,     0,     0,     0,    23,    24,
       0,     0,     0,     0,    25,    26,    27,     0,     0,    28,
       0,     0,     0,     0,    29,    30,     0,    31,     0,     0,
       0,    32,    33,    34,     0,    35,    36,    37,     0,     0,
      38,     0,    39,     0,    40,    41,    42,     0,    43,    44,
       0,    45,     0,     0,     0,     0,     0,     0,     0,     0,
      46,     0,    47,     0,     0,     0,     0,     0,     0,     0,
      48,    49,     0,    50,    51,    52,     0,    53,    54,     0,
      55,     0,     0,    56,    57,    58,     0,    59,     0,    60,
       0,    61,    62,     0,    63,    64,     1,     0,     2,     3,
       4,     5,     6,     0,     7,     8,     9,    10,    11,     0,
       0,     0,   100,     0,    13,    14,     0,     0,     0,     0,
      15,    16,     0,     0,    17,    18,     0,     0,     0,     0,
      19,    20,    21,     0,     0,    22,     0,     0,     0,     0,
       0,    23,    24,     0,     0,     0,     0,    25,    26,    27,
       0,     0,    28,     0,     0,     0,     0,    29,    30,     0,
      31,     0,     0,     0,    32,    33,    34,     0,    35,    36,
      37,     0,     0,    38,     0,    39,     0,    40,    41,    42,
       0,    43,    44,     0,    45,     0,     0,     0,     0,     0,
       0,     0,     0,    46,     0,    47,     0,     0,     0,     0,
       0,     0,     0,    48,    49,     0,    50,    51,    52,     0,
      53,    54,     0,    55,     0,     0,    56,    57,    58,     0,
      59,     0,    60,     0,    61,    62,     0,    63,    64,     1,
       0,     2,     3,     4,     5,     6,     0,     7,     8,     9,
      10,    11,     0,     0,     0,   110,     0,    13,    14,     0,
       0,     0,     0,    15,    16,     0,     0,    17,    18,     0,
       0,     0,     0,    19,    20,    21,     0,     0,    22,     0,
       0,     0,     0,     0,    23,    24,     0,     0,     0,     0,
      25,    26,    27,     0,     0,    28,     0,     0,     0,     0,
      29,    30,     0,    31,     0,     0,     0,    32,    33,    34,
       0,    35,    36,    37,     0,     0,    38,     0,    39,     0,
      40,    41,    42,     0,    43,    44,     0,    45,     0,     0,
       0,     0,     0,     0,     0,     0,    46,     0,    47,     0,
       0,     0,     0,     0,     0,     0,    48,    49,     0,    50,
      51,    52,     0,    53,    54,     0,    55,     0,     0,    56,
      57,    58,     0,    59,     0,    60,     0,    61,    62,     0,
      63,    64,     1,     0,     2,   221,     4,     5,     6,     0,
       7,     8,     9,    10,    11,     0,     0,     0,    12,     0,
      13,    14,     0,     0,     0,     0,    15,    16,     0,     0,
      17,    18,     0,     0,     0,     0,    19,    20,    21,     0,
       0,    22,     0,     0,     0,     0,     0,    23,    24,     0,
       0,     0,     0,    25,    26,    27,     0,     0,    28,     0,
       0,     0,     0,    29,    30,     0,    31,     0,     0,     0,
      32,    33,    34,     0,    35,    36,    37,     0,     0,    38,
       0,    39,     0,    40,    41,    42,     0,    43,    44,     0,
      45,     0,     0,     0,     0,     0,     0,     0,     0,    46,
       0,    47,     0,     0,     0,     0,     0,     0,     0,    48,
      49,     0,    50,    51,    52,     0,    53,    54,     0,    55,
       0,     0,    56,    57,    58,     0,    59,     0,    60,     0,
      61,    62,     0,    63,    64,     1,     0,     2,     3,     4,
       5,     6,     0,     7,     8,     9,    10,    11,     0,     0,
       0,    12,     0,    13,    14,     0,     0,     0,     0,    15,
      16,     0,     0,    17,    18,     0,     0,     0,     0,    19,
      20,    21,     0,     0,   299,     0,     0,     0,     0,     0,
      23,    24,     0,     0,     0,     0,    25,    26,    27,     0,
       0,    28,     0,     0,     0,     0,    29,    30,     0,    31,
       0,     0,     0,    32,    33,    34,     0,    35,    36,    37,
       0,     0,    38,     0,    39,     0,    40,    41,    42,     0,
      43,    44,     0,    45,     0,     0,     0,     0,     0,     0,
       0,     0,    46,     0,    47,     0,     0,     0,     0,     0,
       0,     0,    48,    49,     0,    50,    51,    52,     0,    53,
      54,     0,    55,     0,     0,    56,    57,    58,     0,    59,
       0,    60,     0,    61,    62,     0,    63,    64,     1,     0,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,     0,     0,     0,    12,     0,    13,    14,     0,     0,
       0,     0,    15,    16,     0,     0,    17,    18,     0,     0,
       0,     0,    19,    20,    21,     0,     0,   455,     0,     0,
       0,     0,     0,    23,    24,     0,     0,     0,     0,    25,
      26,    27,     0,     0,    28,     0,     0,     0,     0,    29,
      30,     0,    31,     0,     0,     0,    32,    33,    34,     0,
      35,    36,    37,     0,     0,    38,     0,    39,     0,    40,
      41,    42,     0,    43,    44,     0,    45,     0,     0,     0,
       0,     0,     0,     0,     0,    46,     0,    47,     0,     0,
       0,     0,     0,     0,     0,    48,    49,     0,    50,    51,
      52,     0,    53,    54,     0,    55,     0,     0,    56,    57,
      58,     0,    59,     0,    60,     0,    61,    62,     0,    63,
      64,     1,     0,     2,     3,     4,     5,     6,     0,     7,
       8,     9,    10,    11,     0,     0,     0,    12,     0,    13,
      14,     0,     0,     0,     0,    15,    16,     0,     0,    17,
      18,     0,     0,     0,     0,    19,    20,    21,     0,     0,
      22,     0,     0,     0,     0,     0,    23,    24,     0,     0,
       0,     0,    25,    26,    27,     0,     0,    28,     0,     0,
       0,     0,    29,    30,     0,    31,     0,     0,     0,    32,
      33,    34,     0,    35,    36,    37,     0,     0,    38,     0,
      39,     0,    40,    41,    42,     0,    43,    44,     0,    45,
       0,     0,     0,     0,     0,     0,     0,     0,    46,     0,
      47,     0,     0,     0,     0,     0,     0,     0,    48,   539,
       0,    50,    51,    52,     0,    53,    54,     0,    55,     0,
       0,    56,    57,    58,     0,    59,     0,    60,     0,    61,
      62,     0,    63,    64,     1,     0,     2,     3,     4,     5,
       6,     0,     7,     8,     9,    10,    11,     0,     0,     0,
      12,     0,    13,    14,     0,     0,     0,     0,    15,    16,
       0,     0,    17,    18,     0,     0,     0,     0,    19,    20,
      21,     0,     0,    22,     0,     0,     0,     0,     0,    23,
      24,     0,     0,     0,     0,    25,    26,    27,     0,     0,
      28,     0,     0,     0,     0,    29,    30,     0,    31,     0,
       0,     0,    32,    33,    34,     0,    35,    36,    37,     0,
       0,    38,     0,    39,     0,    40,    41,    42,     0,    43,
      44,     0,    45,     0,     0,     0,     0,     0,     0,     0,
       0,    46,     0,    47,     0,     0,     0,     0,     0,     0,
       0,    48,   567,     0,    50,    51,    52,     0,    53,    54,
       0,    55,     0,     0,    56,    57,    58,     0,    59,     0,
      60,     0,    61,    62,   158,    63,    64,     0,   159,     0,
     160,   161,   162,   163,     0,   164,   165,   166,   167,   168,
     169,     0,     0,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,     0,     0,     0,     0,     0,
     182,   183,     0,     0,     0,     0,     0,     0,   222,   159,
       0,   160,   161,   162,   163,     0,   164,   165,   166,   167,
     168,   169,     0,   184,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,     0,     0,     0,     0,
       0,   185,   183,     0,     0,   159,     0,     0,     0,     0,
     163,     0,   223,     0,   166,   167,   168,   169,     0,     0,
     170,     0,     0,     0,   184,     0,     0,     0,   178,   179,
     186,   181,     0,   187,     0,     0,   188,     0,     0,     0,
     189,   190,   185,   159,   191,     0,     0,   192,   163,     0,
     164,   165,   166,   167,   168,   169,     0,     0,   170,   171,
     172,   173,   174,   175,   176,     0,   178,   179,     0,   181,
       0,   186,     0,     0,   187,     0,     0,   188,   185,     0,
       0,   189,   190,     0,     0,   191,     0,   159,   192,   160,
     161,   162,   163,     0,   164,   165,   166,   167,   168,   169,
       0,     0,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,     0,     0,   185,     0,   190,     0,
     183,   191,     0,     0,     0,   217,     0,     0,   159,     0,
     160,   161,   162,   163,     0,   164,   165,   166,   167,   168,
     169,     0,   184,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,     0,   190,   453,     0,   191,
     185,   183,   192,     0,   159,     0,     0,     0,     0,   163,
       0,     0,     0,   214,   167,   168,   169,     0,     0,   170,
       0,     0,     0,   184,   218,     0,     0,   178,   179,   186,
     181,     0,   187,     0,     0,   188,     0,     0,     0,   189,
     190,   185,     0,   191,     0,   159,   192,     0,     0,     0,
     163,     0,   164,   165,   166,   167,   168,   169,     0,     0,
     170,     0,     0,   173,   174,   175,   176,     0,   178,   179,
     186,   181,     0,   187,     0,     0,   188,   185,     0,     0,
     189,   190,     0,     0,   191,     0,   159,   192,   160,   161,
     162,   163,     0,   164,   165,   166,   167,   168,   169,     0,
       0,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,     0,     0,   511,     0,   190,   185,   183,
     191,   512,     0,     0,     0,     0,     0,   159,     0,   160,
     161,   162,   163,     0,   164,   165,   166,   167,   168,   169,
       0,   184,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,     0,     0,   546,     0,   190,   185,
     183,   191,     0,     0,   192,     0,     0,     0,     0,     0,
       0,     0,   214,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   184,     0,     0,     0,     0,     0,   186,     0,
       0,   187,     0,     0,   188,     0,     0,     0,   189,   190,
     185,     0,   191,     0,     0,   192,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   186,
       0,     0,   187,     0,     0,   188,     0,     0,     0,   189,
     190,     0,     0,   191,     0,   159,   192,   160,   161,   162,
     163,   209,   164,   165,   166,   167,   168,   169,     0,     0,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   159,     0,   160,   161,   162,   163,   183,   164,
     165,   166,   167,   168,   169,     0,   212,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,     0,
     184,     0,     0,     0,     0,   183,     0,     0,     0,     0,
       0,     0,     0,     0,   159,     0,     0,     0,   185,   163,
       0,   164,   165,   166,   167,   168,   169,   184,     0,   170,
     171,   172,   173,   174,   175,   176,     0,   178,   179,   180,
     181,     0,     0,     0,     0,   185,     0,   186,     0,     0,
     187,     0,     0,   188,     0,     0,     0,   189,   190,     0,
       0,   191,     0,     0,   192,     0,     0,     0,     0,  -258,
       0,     0,     0,     0,   186,     0,     0,   187,     0,     0,
     188,     0,     0,     0,   189,   190,     0,   185,   191,     0,
     159,   192,   160,   161,   162,   163,     0,   164,   165,   166,
     167,   168,   169,     0,     0,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,     0,     0,     0,
       0,     0,     0,   183,     0,     0,     0,   190,     0,     0,
     191,     0,     0,   192,     0,   214,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   184,     0,     0,     0,   159,
       0,   160,   161,   162,   163,     0,   164,   165,   166,   167,
     168,   169,     0,   185,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,     0,     0,     0,     0,
       0,     0,   183,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   186,     0,     0,   187,     0,   233,   188,     0,
       0,     0,   189,   190,   184,     0,   191,     0,   159,   192,
     160,   161,   162,   163,     0,   164,   165,   166,   167,   168,
     169,     0,   185,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,     0,     0,     0,     0,     0,
       0,   183,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   186,     0,     0,   187,     0,   234,   188,     0,     0,
       0,   189,   190,   184,     0,   191,     0,   159,   192,   160,
     161,   162,   163,     0,   164,   165,   166,   167,   168,   169,
       0,   185,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,     0,     0,     0,     0,     0,     0,
     183,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     186,     0,     0,   187,     0,     0,   188,     0,     0,     0,
     189,   190,   184,     0,   191,     0,   159,   192,   160,   161,
     162,   163,     0,   164,   165,   166,   167,   168,   169,     0,
     185,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,     0,   256,     0,     0,     0,     0,   183,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   186,
       0,     0,   187,     0,     0,   188,     0,     0,     0,   189,
     190,   184,     0,   191,     0,   159,   192,   160,   161,   162,
     163,   324,   164,   165,   166,   167,   168,   169,     0,   185,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,     0,     0,     0,     0,     0,     0,   183,     0,
       0,     0,     0,   263,     0,     0,     0,     0,   186,     0,
       0,   187,     0,     0,   188,     0,     0,     0,   189,   190,
     184,     0,   191,     0,   159,   192,   160,   161,   162,   163,
     325,   164,   165,   166,   167,   168,   169,     0,   185,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,     0,     0,     0,     0,     0,     0,   183,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   186,     0,     0,
     187,     0,     0,   188,     0,     0,     0,   189,   190,   184,
       0,   191,     0,   159,   192,   160,   161,   162,   163,     0,
     164,   165,   166,   167,   168,   169,     0,   185,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
       0,     0,   326,     0,     0,     0,   183,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   186,     0,     0,   187,
       0,     0,   188,     0,     0,     0,   189,   190,   184,     0,
     191,     0,   159,   192,   160,   161,   162,   163,   336,   164,
     165,   166,   167,   168,   169,     0,   185,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,     0,
       0,     0,     0,     0,     0,   183,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   186,     0,     0,   187,     0,
       0,   188,     0,     0,     0,   189,   190,   184,     0,   191,
       0,   159,   192,   160,   161,   162,   163,   340,   164,   165,
     166,   167,   168,   169,     0,   185,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,     0,     0,
       0,     0,     0,     0,   183,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   186,     0,     0,   187,     0,     0,
     188,     0,     0,     0,   189,   190,   184,     0,   191,     0,
     159,   192,   160,   161,   162,   163,   341,   164,   165,   166,
     167,   168,   169,     0,   185,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,     0,     0,     0,
       0,     0,     0,   183,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   186,     0,     0,   187,     0,     0,   188,
       0,     0,     0,   189,   190,   184,     0,   191,     0,   159,
     192,   160,   161,   162,   163,   346,   164,   165,   166,   167,
     168,   169,     0,   185,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,     0,     0,     0,     0,
       0,     0,   183,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   186,     0,     0,   187,     0,     0,   188,     0,
       0,     0,   189,   190,   184,     0,   191,     0,   159,   192,
     160,   161,   162,   163,   355,   164,   165,   166,   167,   168,
     169,     0,   185,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,     0,     0,     0,     0,     0,
       0,   183,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   186,     0,     0,   187,     0,     0,   188,     0,     0,
       0,   189,   190,   184,     0,   191,     0,   159,   192,   160,
     161,   162,   163,   366,   164,   165,   166,   167,   168,   169,
       0,   185,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,     0,     0,     0,     0,     0,     0,
     183,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     186,     0,     0,   187,     0,     0,   188,     0,     0,     0,
     189,   190,   184,     0,   191,     0,   159,   192,   160,   161,
     162,   163,   369,   164,   165,   166,   167,   168,   169,     0,
     185,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,     0,     0,     0,     0,     0,     0,   183,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   186,
       0,     0,   187,     0,     0,   188,     0,     0,     0,   189,
     190,   184,     0,   191,     0,   159,   192,   160,   161,   162,
     163,   370,   164,   165,   166,   167,   168,   169,     0,   185,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,     0,     0,     0,     0,     0,     0,   183,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   186,     0,
       0,   187,     0,     0,   188,     0,     0,     0,   189,   190,
     184,     0,   191,     0,   159,   192,   160,   161,   162,   163,
     371,   164,   165,   166,   167,   168,   169,     0,   185,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,     0,     0,     0,     0,     0,     0,   183,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   186,     0,     0,
     187,     0,     0,   188,     0,     0,     0,   189,   190,   184,
       0,   191,     0,   159,   192,   160,   161,   162,   163,   372,
     164,   165,   166,   167,   168,   169,     0,   185,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
       0,     0,     0,     0,     0,     0,   183,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   186,     0,     0,   187,
       0,     0,   188,     0,     0,     0,   189,   190,   184,     0,
     191,     0,   159,   192,   160,   161,   162,   163,     0,   164,
     165,   166,   167,   168,   169,     0,   185,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,     0,
       0,   373,     0,     0,     0,   183,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   186,     0,     0,   187,     0,
       0,   188,     0,     0,     0,   189,   190,   184,     0,   191,
       0,   159,   192,   160,   161,   162,   163,   375,   164,   165,
     166,   167,   168,   169,     0,   185,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,     0,     0,
       0,     0,     0,     0,   183,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   186,     0,     0,   187,     0,     0,
     188,     0,     0,     0,   189,   190,   184,     0,   191,     0,
     159,   192,   160,   161,   162,   163,     0,   164,   165,   166,
     167,   168,   169,     0,   185,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,     0,     0,   385,
       0,     0,     0,   183,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   186,     0,     0,   187,     0,     0,   188,
       0,     0,     0,   189,   190,   184,     0,   191,     0,   159,
     192,   160,   161,   162,   163,     0,   164,   165,   166,   167,
     168,   169,     0,   185,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,     0,     0,     0,     0,
       0,     0,   183,     0,     0,     0,     0,   386,     0,     0,
       0,     0,   186,     0,     0,   187,     0,     0,   188,     0,
       0,     0,   189,   190,   184,     0,   191,     0,   159,   192,
     160,   161,   162,   163,   388,   164,   165,   166,   167,   168,
     169,     0,   185,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,     0,     0,     0,     0,     0,
       0,   183,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   186,     0,     0,   187,     0,     0,   188,     0,     0,
       0,   189,   190,   184,     0,   191,     0,   159,   192,   160,
     161,   162,   163,     0,   164,   165,   166,   167,   168,   169,
       0,   185,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,     0,     0,   394,     0,     0,     0,
     183,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     186,     0,     0,   187,     0,     0,   188,     0,     0,     0,
     189,   190,   184,     0,   191,     0,   159,   192,   160,   161,
     162,   163,   399,   164,   165,   166,   167,   168,   169,     0,
     185,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,     0,     0,     0,     0,     0,     0,   183,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   186,
       0,     0,   187,     0,     0,   188,     0,     0,     0,   189,
     190,   184,     0,   191,     0,     0,   192,     0,     0,     0,
       0,     0,     0,   159,     0,   160,   161,   162,   163,   185,
     164,   165,   166,   167,   168,   169,     0,   401,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
       0,     0,     0,     0,     0,     0,   183,     0,   186,     0,
       0,   187,     0,     0,   188,     0,     0,     0,   189,   190,
       0,     0,   191,     0,     0,   192,     0,     0,   184,     0,
       0,     0,   159,     0,   160,   161,   162,   163,     0,   164,
     165,   166,   167,   168,   169,     0,   185,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,     0,
       0,     0,     0,     0,     0,   183,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   186,     0,     0,   187,     0,
     424,   188,     0,     0,     0,   189,   190,   184,     0,   191,
       0,   159,   192,   160,   161,   162,   163,     0,   164,   165,
     166,   167,   168,   169,     0,   185,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,     0,     0,
       0,     0,     0,     0,   183,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   186,     0,     0,   187,     0,     0,
     188,     0,     0,     0,   189,   190,   184,     0,   191,     0,
     159,   192,   160,   161,   162,   163,   434,   164,   165,   166,
     167,   168,   169,     0,   185,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,     0,     0,     0,
       0,     0,   435,   183,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   186,     0,     0,   187,     0,     0,   188,
       0,     0,     0,   189,   190,   184,     0,   191,     0,   159,
     192,   160,   161,   162,   163,     0,   164,   165,   166,   167,
     168,   169,     0,   185,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,     0,     0,     0,     0,
       0,   470,   183,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   186,     0,     0,   187,     0,     0,   188,     0,
       0,     0,   189,   190,   184,     0,   191,     0,     0,   192,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   185,   159,     0,   160,   161,   162,   163,     0,
     164,   165,   166,   167,   168,   169,     0,     0,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
       0,   186,     0,     0,   187,     0,   183,   188,     0,     0,
       0,   189,   190,     0,     0,   191,     0,     0,   192,     0,
       0,     0,   353,     0,     0,     0,     0,     0,   184,     0,
       0,     0,   159,     0,   160,   161,   162,   163,     0,   164,
     165,   166,   167,   168,   169,     0,   185,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,     0,
       0,     0,     0,     0,   489,   183,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   186,     0,     0,   187,     0,
       0,   188,     0,     0,     0,   189,   190,   184,     0,   191,
       0,   159,   192,   160,   161,   162,   163,   491,   164,   165,
     166,   167,   168,   169,     0,   185,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,     0,     0,
       0,     0,     0,     0,   183,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   186,     0,     0,   187,     0,     0,
     188,     0,     0,     0,   189,   190,   184,     0,   191,     0,
     159,   192,   160,   161,   162,   163,     0,   164,   165,   166,
     167,   168,   169,     0,   185,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,     0,     0,   505,
       0,     0,     0,   183,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   186,     0,     0,   187,     0,     0,   188,
       0,     0,     0,   189,   190,   184,     0,   191,     0,   159,
     192,   160,   161,   162,   163,     0,   164,   165,   166,   167,
     168,   169,     0,   185,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,     0,     0,     0,     0,
       0,     0,   183,     0,     0,     0,     0,   516,     0,     0,
       0,     0,   186,     0,     0,   187,     0,     0,   188,     0,
       0,     0,   189,   190,   184,     0,   191,     0,   159,   192,
     160,   161,   162,   163,     0,   164,   165,   166,   167,   168,
     169,     0,   185,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,     0,     0,     0,     0,     0,
       0,   183,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   186,     0,     0,   187,     0,   530,   188,     0,     0,
       0,   189,   190,   184,     0,   191,     0,   159,   192,   160,
     161,   162,   163,     0,   164,   165,   166,   167,   168,   169,
       0,   185,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,     0,     0,     0,     0,     0,     0,
     183,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     186,     0,     0,   187,     0,   532,   188,     0,     0,     0,
     189,   190,   184,     0,   191,     0,   159,   192,   160,   161,
     162,   163,     0,   164,   165,   166,   167,   168,   169,     0,
     185,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,     0,     0,     0,     0,     0,   550,   183,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   186,
       0,     0,   187,     0,     0,   188,     0,     0,     0,   189,
     190,   184,     0,   191,     0,   159,   192,   160,   161,   162,
     163,     0,   164,   165,   166,   167,   168,   169,     0,   185,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,     0,     0,     0,     0,     0,     0,   183,     0,
       0,     0,     0,   552,     0,     0,     0,     0,   186,     0,
       0,   187,     0,     0,   188,     0,     0,     0,   189,   190,
     184,     0,   191,     0,   159,   192,   160,   161,   162,   163,
       0,   164,   165,   166,   167,   168,   169,     0,   185,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,     0,     0,   573,     0,     0,     0,   183,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   186,     0,     0,
     187,     0,     0,   188,     0,     0,     0,   189,   190,   184,
       0,   191,     0,   159,   192,   160,   161,   162,   163,     0,
     164,   165,   166,   167,   168,   169,     0,   185,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
       0,     0,     0,     0,     0,   592,   183,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   186,     0,     0,   187,
       0,     0,   188,     0,     0,     0,   189,   190,   184,     0,
     191,     0,   159,   192,   160,   161,   162,   163,     0,   164,
     165,   166,   167,   168,   169,     0,   185,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,     0,
       0,     0,     0,     0,     0,   183,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   186,     0,     0,   187,     0,
       0,   188,     0,     0,     0,   189,   190,   184,     0,   191,
       0,   159,   192,   257,   161,   162,   163,     0,   164,   165,
     166,   167,   168,   169,     0,   185,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,     0,     0,
       0,     0,     0,     0,   183,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   186,     0,     0,   187,     0,     0,
     188,     0,     0,     0,   189,   190,   184,     0,   191,     0,
     159,   192,   160,   161,   162,   163,     0,   164,   165,   166,
     167,   168,   169,     0,   185,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,     0,     0,     0,
       0,     0,     0,   183,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   186,     0,     0,   187,     0,     0,   188,
       0,     0,     0,   189,   190,   184,     0,   191,     0,   159,
     192,     0,   161,   162,   163,     0,   164,   165,   166,   167,
     168,   169,     0,   185,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,     0,     0,     0,     0,
       0,     0,   183,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,  -258,     0,     0,   188,     0,
       0,     0,   189,   190,   184,     0,   191,     0,   159,   192,
     160,   161,   162,   163,     0,   164,   165,   166,   167,   168,
     169,     0,   185,   170,   171,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,     0,     0,     0,     0,     0,
       0,   183,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   186,     0,     0,   187,     0,     0,   188,     0,     0,
       0,   189,   190,   184,     0,   191,     0,   159,   192,     0,
     161,   162,   163,     0,   164,   165,   166,   167,   168,   169,
       0,   185,   170,   171,   172,   173,   174,   175,   176,   177,
     178,   179,   180,   181,     0,     0,     0,     0,     0,     0,
     183,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   188,     0,     0,     0,
     189,   190,     0,     0,   191,     0,   159,   192,     0,     0,
     162,   163,     0,   164,   165,   166,   167,   168,   169,     0,
     185,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,     0,     0,     0,     0,     0,     0,   183,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   186,
       0,     0,   187,     0,     0,   188,     0,     0,     0,   189,
     190,   184,     0,   191,     0,   159,   192,     0,     0,   162,
     163,     0,   164,   165,   166,   167,   168,   169,     0,   185,
     170,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,     0,     0,     0,   441,     0,     0,   183,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   189,   190,
     184,     0,   191,     0,   159,   192,     0,     0,   162,   163,
       0,   164,   165,   166,   167,   168,   169,     0,   185,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,     0,     0,     0,     0,     0,     0,   183,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   189,   190,   184,
       0,   191,     0,   159,   192,     0,     0,   162,   163,     0,
     164,   165,   166,   167,   168,   169,     0,   185,   170,   171,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
       0,     0,     0,     0,     0,     0,   183,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  -258,   190,   184,     0,
     191,     0,   159,   192,     0,     0,  -258,   163,     0,   164,
     165,   166,   167,   168,   169,     0,   185,   170,   171,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   159,
       0,     0,     0,     0,   163,     0,   164,   165,   166,   167,
     391,   169,     0,     0,   170,     0,     0,   173,     0,   175,
     176,     0,   178,   179,     0,   181,   190,   184,   159,   191,
       0,     0,   192,   163,     0,   164,   165,   166,   167,   168,
     169,     0,     0,   170,     0,   185,   173,     0,   175,  -258,
     159,   178,   179,     0,   181,   163,     0,   164,   165,   166,
     167,   168,   169,     0,     0,   170,     0,     0,   173,     0,
       0,     0,   185,   178,   179,     0,   181,     0,     0,     0,
       0,     0,     0,     0,     0,   190,     0,   159,   191,     0,
       0,   192,   163,     0,   164,   165,   166,   167,   168,   169,
       0,   185,   170,     0,     0,     0,     0,     0,     0,   159,
     178,   179,   190,   181,   163,   191,     0,     0,   192,     0,
     168,   169,     0,   185,   170,   159,     0,     0,     0,     0,
     163,     0,   178,   179,     0,   181,  -258,   169,     0,     0,
     170,   190,     0,     0,   191,     0,     0,   192,   178,   179,
       0,   181,     0,     0,     0,     0,     0,     0,     0,     0,
     185,     0,     0,   190,     0,     0,   191,     0,     0,   192,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   185,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   185,     0,
     190,     0,     0,   191,     0,     0,   192,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   190,     0,     0,   191,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   190,     0,
       0,   191
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-521))

#define yytable_value_is_error(yytable_value) \
  ((yytable_value) == (-258))

static const yytype_int16 yycheck[] =
{
       0,    24,   347,   129,    17,    15,   332,    51,    15,   241,
      10,   412,    12,    13,    14,   500,    16,    17,    18,    15,
      14,    21,    22,     4,    24,     3,     4,    27,    14,    29,
      30,   551,    10,    29,    17,    13,    47,    54,    38,    17,
      17,    44,    46,    18,    55,    26,    46,    47,    48,    49,
      44,    51,    52,    54,    55,   575,    56,    60,   543,    59,
      60,    38,    58,    63,    74,   109,     4,    74,    46,   470,
      83,    46,    10,     3,     4,    13,    76,    77,    78,    79,
      10,    67,   408,    13,    67,     4,    86,    17,    26,    89,
      17,    51,    46,    59,    60,     4,    96,    63,    98,    99,
     100,    15,     4,     5,     4,   128,   106,    17,    10,   119,
     110,    38,    17,    67,   114,    29,    46,   117,   118,    17,
     120,   121,     3,   123,   124,   125,   126,   253,    38,   137,
     130,    45,    46,    38,   142,   367,    17,   137,   483,    17,
      76,   141,   142,   121,    46,   162,    10,   147,    17,   149,
      14,   151,    17,    10,   154,   133,   156,    15,   118,    17,
     160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
      38,   171,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,     0,    17,   187,   188,   189,
     190,   121,   192,   193,   194,   133,    28,     3,     4,    44,
     217,   201,    47,   163,    36,     3,     4,    13,   440,    17,
      55,   211,    59,    17,    61,    13,    17,    49,   218,    17,
      17,     3,     4,    55,     4,    41,   458,   227,   228,   229,
     230,    13,    46,   193,    17,    17,   236,    67,    72,   239,
      46,    24,   242,    17,    27,    88,   246,    17,    46,   109,
      17,   122,    35,    36,    77,    38,   264,   257,   258,    17,
     260,     3,   494,    56,    46,   265,    44,   267,    67,    81,
      17,     3,   272,    46,   506,    18,   276,     3,     4,    18,
       6,    18,    51,    81,    10,    76,   246,    13,    14,   121,
      79,    18,    60,    19,    18,    21,    22,    18,   258,   299,
      18,    24,    85,    18,    18,    50,    44,   267,    53,    54,
      29,    41,   312,    18,     3,   121,    51,    62,     3,   336,
      46,    77,    18,   121,   109,    45,   182,   327,    45,    41,
     109,   360,   109,   109,   425,    57,   336,   337,   355,   121,
     470,   576,   482,    17,   344,   128,    -1,   347,   180,    -1,
      24,    -1,   352,   353,   186,   355,   356,   357,    -1,    -1,
      -1,    35,   362,    -1,    38,   365,    -1,   327,    -1,    -1,
     115,   379,   380,   381,    -1,   383,    -1,    -1,    -1,   379,
     380,   381,   214,    -1,    -1,    -1,    -1,   387,   114,    -1,
      -1,   391,   392,    -1,    -1,   121,   396,   397,    -1,   125,
      -1,    -1,   419,   403,   404,   405,   406,   152,    -1,   409,
     410,    85,    -1,   413,    -1,   415,    -1,    -1,    -1,    12,
      -1,    -1,    -1,    -1,    17,    -1,    -1,   427,   260,   429,
      -1,    24,    -1,   265,    27,    -1,   396,    -1,   446,    -1,
     457,    -1,    35,    36,    -1,    38,   446,    -1,    -1,   449,
     410,    -1,    -1,    -1,   128,   455,   473,   457,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   477,
      -1,    -1,   217,   473,   491,   475,    -1,   477,   223,    -1,
      -1,    -1,   482,   483,   484,    -1,    -1,   487,   233,   234,
      -1,    -1,    85,   493,    -1,   240,   496,    -1,    -1,   499,
     332,   501,    -1,   520,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   512,    -1,    -1,    -1,    -1,   262,   263,   264,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   360,   274,
      -1,    -1,   540,    -1,   534,   128,   553,   537,   555,   539,
     540,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   548,    -1,
      -1,   551,    -1,    -1,    -1,   555,   556,   557,   558,   559,
     568,   561,    -1,   563,   564,   565,    -1,   567,   568,    -1,
      -1,   571,    -1,    -1,    -1,   575,   408,    -1,    -1,    -1,
      -1,    -1,    -1,   415,    -1,   585,    -1,   587,    -1,   597,
      -1,   599,    -1,    -1,   602,    -1,   604,   597,    -1,   599,
      -1,    -1,   602,   563,   604,   565,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   585,    -1,   587,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   383,    -1,
      -1,   386,    12,    -1,    14,    -1,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    -1,    -1,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      -1,    -1,    -1,   418,   506,    45,   421,    -1,    -1,   424,
      -1,    -1,    -1,   428,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   439,   440,    -1,    67,    -1,    -1,
      -1,    12,    -1,    -1,    -1,   450,    17,   539,    19,    20,
      21,    22,    23,    24,    -1,    85,    27,    28,    29,    30,
      31,    32,    33,    -1,    35,    36,    37,    38,    -1,    -1,
      -1,    -1,   564,    -1,    -1,   567,    12,    -1,    -1,    -1,
      -1,    17,    -1,    -1,    -1,    -1,    -1,   492,    24,    -1,
     495,    27,   497,    -1,   124,   125,    67,    -1,   128,    35,
      36,   131,    38,    -1,   509,    -1,    -1,    -1,    -1,    -1,
      -1,   516,    -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,
     525,    -1,    -1,    -1,   529,   530,    -1,   532,    -1,    -1,
      -1,   536,    -1,    -1,    -1,    -1,   541,    -1,    -1,    -1,
      -1,    -1,   547,    -1,    -1,    -1,    -1,   552,    -1,    85,
      -1,    -1,    -1,    -1,   125,    -1,    -1,   128,    -1,    -1,
     131,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   593,   125,
     595,    -1,   128,   598,     1,   600,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    -1,    40,    41,    -1,    -1,    44,    45,    46,
      47,    -1,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    -1,    65,    66,
      67,    -1,    69,    70,    71,    -1,    73,    74,    75,    -1,
      77,    78,    -1,    80,    -1,    82,    83,    84,    85,    86,
      87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    99,   100,   101,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,    -1,   120,   121,   122,   123,   124,   125,    -1,
     127,   128,   129,   130,   131,   132,   133,     1,    -1,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    40,    41,    -1,    -1,
      44,    45,    46,    47,    -1,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      -1,    65,    66,    67,    -1,    69,    70,    71,    -1,    73,
      74,    75,    -1,    77,    78,    -1,    80,    -1,    82,    83,
      84,    85,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    99,   100,   101,    -1,    -1,
      -1,    -1,    -1,    -1,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,    -1,   120,   121,   122,   123,
     124,   125,    -1,   127,   128,   129,   130,   131,   132,   133,
       1,    -1,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    -1,    40,
      41,    -1,    -1,    44,    45,    46,    47,    -1,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    -1,    65,    66,    67,    -1,    69,    70,
      71,    -1,    73,    74,    75,    -1,    77,    78,    -1,    80,
      -1,    82,    83,    84,    85,    86,    87,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    99,   100,
     101,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,    -1,   120,
     121,   122,   123,   124,   125,    -1,   127,   128,   129,   130,
     131,   132,   133,     1,    -1,     3,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    -1,    40,    41,    -1,    -1,    44,    45,    46,    47,
      -1,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    -1,    65,    66,    67,
      -1,    69,    70,    71,    -1,    73,    74,    75,    -1,    77,
      -1,    -1,    80,    -1,    82,    83,    84,    85,    86,    87,
      -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    99,   100,   101,    -1,    -1,    -1,    -1,    -1,    -1,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,    -1,   120,   121,   122,   123,   124,   125,    -1,   127,
     128,   129,   130,   131,   132,   133,     1,    -1,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    40,    41,    -1,    -1,    44,
      45,    46,    47,    -1,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    -1,
      65,    66,    67,    -1,    69,    70,    71,    -1,    73,    74,
      75,    -1,    77,    78,    -1,    80,    -1,    82,    83,    84,
      85,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    99,   100,   101,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,    -1,   120,   121,   122,   123,   124,
     125,    -1,    -1,   128,   129,   130,   131,   132,   133,     1,
      -1,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    40,    41,
      -1,    -1,    44,    45,    46,    47,    -1,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    -1,    65,    66,    67,    -1,    69,    70,    71,
      -1,    73,    74,    75,    -1,    77,    78,    -1,    80,    -1,
      82,    83,    84,    85,    86,    87,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    99,   100,   101,
      -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,   110,   111,
     112,   113,   114,   115,   116,   117,   118,    -1,   120,   121,
     122,   123,   124,   125,    -1,   127,   128,   129,   130,   131,
     132,   133,     1,    -1,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    -1,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      -1,    40,    -1,    -1,    -1,    -1,    45,    46,    47,    -1,
      -1,    50,    -1,    52,    53,    54,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    62,    63,    -1,    65,    -1,    67,    -1,
      69,    70,    71,    -1,    73,    74,    75,    76,    -1,    78,
      -1,    80,    -1,    82,    83,    84,    85,    86,    87,    -1,
      89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
     109,    -1,   111,   112,   113,   114,   115,   116,   117,   118,
      -1,   120,   121,   122,   123,   124,   125,    -1,   127,   128,
     129,   130,   131,   132,   133,     1,    -1,     3,     4,     5,
       6,     7,    -1,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    -1,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    -1,    40,    -1,    -1,    -1,    -1,    45,
      46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,
      -1,    57,    -1,    -1,    60,    -1,    62,    63,    -1,    65,
      -1,    67,    -1,    69,    70,    71,    -1,    73,    74,    75,
      76,    -1,    78,    -1,    80,    -1,    82,    83,    84,    85,
      86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,   109,    -1,   111,   112,   113,   114,   115,
     116,   117,   118,    -1,   120,   121,   122,   123,   124,   125,
      -1,   127,   128,   129,   130,   131,   132,   133,     1,    -1,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    -1,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    -1,    40,    -1,    -1,
      -1,    -1,    45,    46,    47,    -1,    -1,    -1,    -1,    52,
      53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,
      63,    -1,    65,    -1,    67,    -1,    69,    70,    71,    -1,
      73,    74,    75,    -1,    -1,    78,    -1,    80,    -1,    82,
      83,    84,    85,    86,    87,    -1,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,
     113,   114,   115,   116,   117,   118,    -1,   120,   121,   122,
     123,   124,   125,    -1,   127,   128,   129,   130,   131,   132,
     133,     1,    -1,     3,     4,     5,     6,     7,    -1,     9,
      10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,
      20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,
      30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,
      40,    -1,    -1,    -1,    44,    -1,    46,    47,    -1,    49,
      -1,    51,    52,    53,    54,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,
      70,    71,    -1,    73,    74,    75,    -1,    77,    78,    -1,
      80,    -1,    82,    83,    84,    -1,    86,    87,    -1,    89,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,
      -1,   111,   112,   113,    -1,   115,   116,    -1,   118,    -1,
      -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,   129,
     130,    -1,   132,   133,     1,    -1,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,
      -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,
      37,    -1,    -1,    40,    -1,    -1,    -1,    44,    -1,    46,
      47,    -1,    -1,    -1,    51,    52,    53,    54,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,
      -1,    -1,    69,    70,    71,    -1,    73,    74,    75,    -1,
      77,    78,    -1,    80,    -1,    82,    83,    84,    -1,    86,
      87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,    -1,   111,   112,   113,    -1,   115,   116,
      -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,
     127,    -1,   129,   130,    -1,   132,   133,     1,    -1,     3,
       4,     5,     6,     7,    -1,     9,    10,    11,    12,    13,
      -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,
      -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,
      -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    49,    -1,    51,    52,    53,
      54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,
      -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,
      74,    75,    -1,    77,    78,    -1,    80,    -1,    82,    83,
      84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,   113,
      -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,   123,
      -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,   133,
       1,    -1,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,    20,
      -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,
      -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,
      71,    -1,    73,    74,    75,    76,    -1,    78,    79,    80,
      81,    82,    83,    84,    -1,    86,    87,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,
     111,   112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,
     121,   122,   123,    -1,   125,    -1,   127,    -1,   129,   130,
      -1,   132,   133,     1,    -1,     3,     4,     5,     6,     7,
      -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,    17,
      -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,
      -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,
      -1,    -1,    40,    -1,    -1,    -1,    44,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    53,    54,    55,    56,    57,
      -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,
      -1,    69,    70,    71,    -1,    73,    74,    75,    -1,    -1,
      78,    -1,    80,    -1,    82,    83,    84,    -1,    86,    87,
      -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,   109,    -1,   111,   112,   113,    -1,   115,   116,    -1,
     118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,
      -1,   129,   130,    -1,   132,   133,     1,    -1,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,
      25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,
      35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,    44,
      -1,    46,    47,    -1,    -1,    -1,    51,    52,    53,    54,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,
      65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,
      75,    -1,    77,    78,    -1,    80,    -1,    82,    83,    84,
      -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,
     115,   116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,
     125,    -1,   127,    -1,   129,   130,    -1,   132,   133,     1,
      -1,     3,     4,     5,     6,     7,    -1,     9,    10,    11,
      12,    13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,
      -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,
      -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,
      -1,    -1,    44,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,
      -1,    73,    74,    75,    -1,    77,    78,    -1,    80,    -1,
      82,    83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,   110,   111,
     112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,
     122,   123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,
     132,   133,     1,    -1,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,
      19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,
      29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,
      -1,    40,    -1,    -1,    -1,    44,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    62,    63,    -1,    65,    66,    -1,    -1,
      69,    70,    71,    -1,    73,    74,    75,    -1,    -1,    78,
      -1,    80,    -1,    82,    83,    84,    -1,    86,    87,    -1,
      89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
     109,    -1,   111,   112,   113,    -1,   115,   116,    -1,   118,
      -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,
     129,   130,    -1,   132,   133,     1,    -1,     3,     4,     5,
       6,     7,    -1,     9,    10,    11,    12,    13,    -1,    -1,
      -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,
      26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,
      36,    37,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,
      -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,    75,
      -1,    -1,    78,    79,    80,    81,    82,    83,    84,    -1,
      86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,   115,
     116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,
      -1,   127,    -1,   129,   130,    -1,   132,   133,     1,    -1,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,
      -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,
      -1,    44,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,
      63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,
      73,    74,    75,    -1,    77,    78,    -1,    80,    -1,    82,
      83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,
     113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,
     123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,
     133,     1,    -1,     3,     4,     5,     6,     7,    -1,     9,
      10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,
      20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,
      30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,
      40,    -1,    -1,    -1,    44,    -1,    46,    47,    -1,    -1,
      -1,    51,    52,    53,    54,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,
      70,    71,    -1,    73,    74,    75,    -1,    -1,    78,    -1,
      80,    -1,    82,    83,    84,    -1,    86,    87,    -1,    89,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,
      -1,   111,   112,   113,    -1,   115,   116,    -1,   118,    -1,
      -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,   129,
     130,    -1,   132,   133,     1,    -1,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,
      -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,
      37,    -1,    -1,    40,    -1,    -1,    -1,    44,    -1,    46,
      47,    -1,    -1,    50,    -1,    52,    53,    54,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,
      -1,    -1,    69,    70,    71,    -1,    73,    74,    75,    -1,
      -1,    78,    -1,    80,    -1,    82,    83,    84,    -1,    86,
      87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,    -1,   111,   112,   113,    -1,   115,   116,
      -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,
     127,    -1,   129,   130,    -1,   132,   133,     1,    -1,     3,
       4,     5,     6,     7,    -1,     9,    10,    11,    12,    13,
      -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,
      -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,
      -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,
      44,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    53,
      54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,
      -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,
      74,    75,    -1,    77,    78,    -1,    80,    -1,    82,    83,
      84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,   113,
      -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,   123,
      -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,   133,
       1,    -1,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,    20,
      -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,
      -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,
      -1,    -1,    -1,    44,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,
      71,    -1,    73,    74,    75,    -1,    77,    78,    -1,    80,
      -1,    82,    83,    84,    -1,    86,    87,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,
     111,   112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,
     121,   122,   123,    -1,   125,    -1,   127,    -1,   129,   130,
      -1,   132,   133,     1,    -1,     3,     4,     5,     6,     7,
      -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,    17,
      -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,
      -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,
      -1,    -1,    40,    -1,    -1,    -1,    44,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    53,    54,    -1,    56,    57,
      -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,
      -1,    69,    70,    71,    -1,    73,    74,    75,    -1,    -1,
      78,    -1,    80,    -1,    82,    83,    84,    -1,    86,    87,
      -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,   109,    -1,   111,   112,   113,    -1,   115,   116,    -1,
     118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,
      -1,   129,   130,    -1,   132,   133,     1,    -1,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,
      25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,
      35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,    44,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,
      65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,
      75,    -1,    77,    78,    -1,    80,    -1,    82,    83,    84,
      -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,
     115,   116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,
     125,    -1,   127,    -1,   129,   130,    -1,   132,   133,     1,
      -1,     3,     4,     5,     6,     7,    -1,     9,    10,    11,
      12,    13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,
      -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,
      -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,
      -1,    -1,    44,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,
      -1,    73,    74,    75,    -1,    77,    78,    -1,    80,    -1,
      82,    83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,
     112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,
     122,   123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,
     132,   133,     1,    -1,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,
      19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,
      29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,
      -1,    40,    -1,    -1,    -1,    44,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,
      69,    70,    71,    -1,    73,    74,    75,    -1,    77,    78,
      -1,    80,    -1,    82,    83,    84,    -1,    86,    87,    -1,
      89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
     109,    -1,   111,   112,   113,    -1,   115,   116,    -1,   118,
      -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,
     129,   130,    -1,   132,   133,     1,    -1,     3,     4,     5,
       6,     7,    -1,     9,    10,    11,    12,    13,    -1,    -1,
      -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,
      26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,
      36,    37,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,
      -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,    75,
      76,    -1,    78,    79,    80,    -1,    82,    83,    84,    -1,
      86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,   115,
     116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,
      -1,   127,    -1,   129,   130,    -1,   132,   133,     1,    -1,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,
      -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,
      -1,    44,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,
      63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,
      73,    74,    75,    -1,    77,    78,    -1,    80,    -1,    82,
      83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,
     113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,
     123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,
     133,     1,    -1,     3,     4,     5,     6,     7,    -1,     9,
      10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,
      20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,
      30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,
      40,    -1,    -1,    -1,    44,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,
      70,    71,    -1,    73,    74,    75,    -1,    77,    78,    -1,
      80,    -1,    82,    83,    84,    -1,    86,    87,    -1,    89,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,
      -1,   111,   112,   113,    -1,   115,   116,    -1,   118,    -1,
      -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,   129,
     130,    -1,   132,   133,     1,    -1,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,
      -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,
      37,    -1,    -1,    40,    -1,    -1,    -1,    44,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,
      -1,    -1,    69,    70,    71,    -1,    73,    74,    75,    -1,
      77,    78,    -1,    80,    -1,    82,    83,    84,    -1,    86,
      87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,    -1,   111,   112,   113,    -1,   115,   116,
      -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,
     127,    -1,   129,   130,    -1,   132,   133,     1,    -1,     3,
       4,     5,     6,     7,    -1,     9,    10,    11,    12,    13,
      -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,
      -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,
      -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,
      44,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    53,
      54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,
      -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,
      74,    75,    -1,    77,    78,    -1,    80,    -1,    82,    83,
      84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,   113,
      -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,   123,
      -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,   133,
       1,    -1,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,    20,
      -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,
      -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,
      -1,    -1,    -1,    44,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,
      71,    -1,    73,    74,    75,    -1,    77,    78,    -1,    80,
      -1,    82,    83,    84,    -1,    86,    87,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,
     111,   112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,
     121,   122,   123,    -1,   125,    -1,   127,    -1,   129,   130,
      -1,   132,   133,     1,    -1,     3,     4,     5,     6,     7,
      -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,    17,
      -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,
      -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,
      -1,    -1,    40,    -1,    -1,    -1,    44,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,
      -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,
      -1,    69,    70,    71,    -1,    73,    74,    75,    -1,    77,
      78,    -1,    80,    -1,    82,    83,    84,    -1,    86,    87,
      -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,   109,    -1,   111,   112,   113,    -1,   115,   116,    -1,
     118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,
      -1,   129,   130,    -1,   132,   133,     1,    -1,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,
      25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,
      35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,    44,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,
      65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,
      75,    -1,    77,    78,    -1,    80,    -1,    82,    83,    84,
      -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,
     115,   116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,
     125,    -1,   127,    -1,   129,   130,    -1,   132,   133,     1,
      -1,     3,     4,     5,     6,     7,    -1,     9,    10,    11,
      12,    13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,
      -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,
      -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,
      -1,    -1,    44,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,
      -1,    73,    74,    75,    -1,    77,    78,    -1,    80,    -1,
      82,    83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,
     112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,
     122,   123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,
     132,   133,     1,    -1,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,
      19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,
      29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,
      -1,    40,    -1,    -1,    -1,    44,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,
      69,    70,    71,    -1,    73,    74,    75,    -1,    77,    78,
      -1,    80,    -1,    82,    83,    84,    -1,    86,    87,    -1,
      89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
     109,    -1,   111,   112,   113,    -1,   115,   116,    -1,   118,
      -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,
     129,   130,    -1,   132,   133,     1,    -1,     3,     4,     5,
       6,     7,    -1,     9,    10,    11,    12,    13,    -1,    -1,
      -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,
      26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,
      36,    37,    -1,    -1,    40,    -1,    -1,    -1,    44,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,
      -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,    75,
      -1,    77,    78,    -1,    80,    -1,    82,    83,    84,    -1,
      86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,   115,
     116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,
      -1,   127,    -1,   129,   130,    -1,   132,   133,     1,    -1,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,
      -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,
      -1,    44,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,
      63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,
      73,    74,    75,    -1,    77,    78,    -1,    80,    -1,    82,
      83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,
     113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,
     123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,
     133,     1,    -1,     3,     4,     5,     6,     7,    -1,     9,
      10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,
      20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,
      30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,
      40,    -1,    -1,    -1,    44,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,
      70,    71,    -1,    73,    74,    75,    -1,    77,    78,    -1,
      80,    -1,    82,    83,    84,    -1,    86,    87,    -1,    89,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,
      -1,   111,   112,   113,    -1,   115,   116,    -1,   118,    -1,
      -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,   129,
     130,    -1,   132,   133,     1,    -1,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,
      -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,
      37,    -1,    -1,    40,    -1,    -1,    -1,    44,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,
      -1,    -1,    69,    70,    71,    -1,    73,    74,    75,    -1,
      77,    78,    -1,    80,    -1,    82,    83,    84,    -1,    86,
      87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,    -1,   111,   112,   113,    -1,   115,   116,
      -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,
     127,    -1,   129,   130,    -1,   132,   133,     1,    -1,     3,
       4,     5,     6,     7,    -1,     9,    10,    11,    12,    13,
      -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,
      -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,
      -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    53,
      54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,
      -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,
      74,    75,    -1,    77,    78,    -1,    80,    -1,    82,    83,
      84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,   109,   110,   111,   112,   113,
      -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,   123,
      -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,   133,
       1,    -1,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,    20,
      -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,
      -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,
      -1,    -1,    -1,    44,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,
      71,    -1,    73,    74,    75,    -1,    77,    78,    -1,    80,
      -1,    82,    83,    84,    -1,    86,    87,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,
     111,   112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,
     121,   122,   123,    -1,   125,    -1,   127,    -1,   129,   130,
      -1,   132,   133,     1,    -1,     3,     4,     5,     6,     7,
      -1,     9,    10,    11,    12,    13,    14,    -1,    -1,    17,
      -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,
      -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,
      -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,
      -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,
      -1,    69,    70,    71,    -1,    73,    74,    75,    -1,    -1,
      78,    -1,    80,    -1,    82,    83,    84,    -1,    86,    87,
      -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,   109,    -1,   111,   112,   113,    -1,   115,   116,    -1,
     118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,
      -1,   129,   130,    -1,   132,   133,     1,    -1,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,
      25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,
      35,    36,    37,    -1,    -1,    40,    41,    -1,    -1,    -1,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,
      65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,
      75,    -1,    -1,    78,    -1,    80,    -1,    82,    83,    84,
      -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,
     115,   116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,
     125,    -1,   127,    -1,   129,   130,    -1,   132,   133,     1,
      -1,     3,     4,     5,     6,     7,    -1,     9,    10,    11,
      12,    13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,
      -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,
      -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,
      -1,    73,    74,    75,    76,    -1,    78,    -1,    80,    -1,
      82,    83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,
     112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,
     122,   123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,
     132,   133,     1,    -1,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    17,    18,
      19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,
      29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,
      69,    70,    71,    -1,    73,    74,    75,    -1,    -1,    78,
      -1,    80,    -1,    82,    83,    84,    -1,    86,    87,    -1,
      89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
     109,    -1,   111,   112,   113,    -1,   115,   116,    -1,   118,
      -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,
     129,   130,    -1,   132,   133,     1,    -1,     3,     4,     5,
       6,     7,    -1,     9,    10,    11,    12,    13,    -1,    -1,
      -1,    17,    18,    19,    20,    -1,    -1,    -1,    -1,    25,
      26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,
      36,    37,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,
      -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,    75,
      -1,    -1,    78,    -1,    80,    -1,    82,    83,    84,    -1,
      86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,   115,
     116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,
      -1,   127,    -1,   129,   130,    -1,   132,   133,     1,    -1,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,
      -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,
      -1,    44,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,
      63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,
      73,    74,    75,    -1,    -1,    78,    -1,    80,    -1,    82,
      83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,
     113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,
     123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,
     133,     1,    -1,     3,     4,     5,     6,     7,    -1,     9,
      10,    11,    12,    13,    -1,    -1,    -1,    17,    18,    19,
      20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,
      30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,
      40,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,
      70,    71,    -1,    73,    74,    75,    -1,    -1,    78,    -1,
      80,    -1,    82,    83,    84,    -1,    86,    87,    -1,    89,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,
      -1,   111,   112,   113,    -1,   115,   116,    -1,   118,    -1,
      -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,   129,
     130,    -1,   132,   133,     1,    -1,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    18,    19,    20,    -1,    -1,    -1,    -1,    25,    26,
      -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,
      37,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,
      -1,    -1,    69,    70,    71,    -1,    73,    74,    75,    -1,
      -1,    78,    -1,    80,    -1,    82,    83,    84,    -1,    86,
      87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,    -1,   111,   112,   113,    -1,   115,   116,
      -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,
     127,    -1,   129,   130,    -1,   132,   133,     1,    -1,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,
      -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,
      -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    53,
      54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,
      -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,
      74,    75,    -1,    -1,    78,    -1,    80,    -1,    82,    83,
      84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,   113,
      -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,   123,
      -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,   133,
       1,    -1,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,    20,
      -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,
      -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,
      71,    -1,    73,    74,    75,    -1,    -1,    78,    79,    80,
      -1,    82,    83,    84,    -1,    86,    87,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,
     111,   112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,
     121,   122,   123,    -1,   125,    -1,   127,    -1,   129,   130,
      -1,   132,   133,     1,    -1,     3,     4,     5,     6,     7,
      -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,    17,
      -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,
      -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,
      -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,
      -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,
      -1,    69,    70,    71,    -1,    73,    74,    75,    -1,    77,
      78,    -1,    80,    -1,    82,    83,    84,    -1,    86,    87,
      -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,   109,    -1,   111,   112,   113,    -1,   115,   116,    -1,
     118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,
      -1,   129,   130,    -1,   132,   133,     1,    -1,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,
      25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,
      35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,    -1,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,
      65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,
      75,    -1,    77,    78,    -1,    80,    -1,    82,    83,    84,
      -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,
     115,   116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,
     125,    -1,   127,    -1,   129,   130,    -1,   132,   133,     1,
      -1,     3,     4,     5,     6,     7,    -1,     9,    10,    11,
      12,    13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,
      -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,
      -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    51,
      52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,
      -1,    73,    74,    75,    -1,    -1,    78,    -1,    80,    -1,
      82,    83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,
     112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,
     122,   123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,
     132,   133,     1,    -1,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,
      19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,
      29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,
      69,    70,    71,    -1,    73,    74,    75,    76,    -1,    78,
      -1,    80,    -1,    82,    83,    84,    -1,    86,    87,    -1,
      89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
     109,    -1,   111,   112,   113,    -1,   115,   116,    -1,   118,
      -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,
     129,   130,    -1,   132,   133,     1,    -1,     3,     4,     5,
       6,     7,    -1,     9,    10,    11,    12,    13,    -1,    -1,
      -1,    17,    18,    19,    20,    -1,    -1,    -1,    -1,    25,
      26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,
      36,    37,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,
      -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,    75,
      -1,    -1,    78,    -1,    80,    -1,    82,    83,    84,    -1,
      86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,   115,
     116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,
      -1,   127,    -1,   129,   130,    -1,   132,   133,     1,    -1,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,
      -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,
      63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,
      73,    74,    75,    -1,    -1,    78,    -1,    80,    -1,    82,
      83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,
     113,    -1,   115,   116,    -1,   118,   119,    -1,   121,   122,
     123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,
     133,     1,    -1,     3,     4,     5,     6,     7,    -1,     9,
      10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,
      20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,
      30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,
      40,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,
      70,    71,    -1,    73,    74,    75,    -1,    -1,    78,    -1,
      80,    -1,    82,    83,    84,    -1,    86,    87,    -1,    89,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,
     110,   111,   112,   113,    -1,   115,   116,    -1,   118,    -1,
      -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,   129,
     130,    -1,   132,   133,     1,    -1,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,
      -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,
      37,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,
      -1,    -1,    69,    70,    71,    -1,    73,    74,    75,    -1,
      -1,    78,    -1,    80,    -1,    82,    83,    84,    -1,    86,
      87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,   110,   111,   112,   113,    -1,   115,   116,
      -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,
     127,    -1,   129,   130,    -1,   132,   133,     1,    -1,     3,
       4,     5,     6,     7,    -1,     9,    10,    11,    12,    13,
      -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,
      -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,
      -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    53,
      54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,
      -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,
      74,    75,    -1,    -1,    78,    -1,    80,    -1,    82,    83,
      84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,   113,
      -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,   123,
      -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,   133,
       1,    -1,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,    20,
      -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,
      -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,
      -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,
      71,    -1,    73,    74,    75,    -1,    -1,    78,    -1,    80,
      -1,    82,    83,    84,    -1,    86,    87,    -1,    89,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,
     111,   112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,
     121,   122,   123,    -1,   125,    -1,   127,    -1,   129,   130,
      -1,   132,   133,     1,    -1,     3,     4,     5,     6,     7,
      -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,    17,
      -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,
      -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,
      -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,
      -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,
      -1,    69,    70,    71,    -1,    73,    74,    75,    -1,    -1,
      78,    -1,    80,    -1,    82,    83,    84,    -1,    86,    87,
      -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     108,   109,    -1,   111,   112,   113,    -1,   115,   116,    -1,
     118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,
      -1,   129,   130,    -1,   132,   133,     1,    -1,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,
      25,    26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,
      35,    36,    37,    -1,    -1,    40,    -1,    -1,    -1,    -1,
      -1,    46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,
      65,    -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,
      75,    -1,    -1,    78,    -1,    80,    -1,    82,    83,    84,
      -1,    86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,
     115,   116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,
     125,    -1,   127,    -1,   129,   130,    -1,   132,   133,     1,
      -1,     3,     4,     5,     6,     7,    -1,     9,    10,    11,
      12,    13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,
      -1,    -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,
      -1,    -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
      52,    53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,
      62,    63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,
      -1,    73,    74,    75,    -1,    -1,    78,    -1,    80,    -1,
      82,    83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,
     112,   113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,
     122,   123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,
     132,   133,     1,    -1,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,
      19,    20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,
      29,    30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,
      69,    70,    71,    -1,    73,    74,    75,    -1,    -1,    78,
      -1,    80,    -1,    82,    83,    84,    -1,    86,    87,    -1,
      89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,
      -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,
     109,    -1,   111,   112,   113,    -1,   115,   116,    -1,   118,
      -1,    -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,
     129,   130,    -1,   132,   133,     1,    -1,     3,     4,     5,
       6,     7,    -1,     9,    10,    11,    12,    13,    -1,    -1,
      -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,
      26,    -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,
      36,    37,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,
      -1,    -1,    -1,    69,    70,    71,    -1,    73,    74,    75,
      -1,    -1,    78,    -1,    80,    -1,    82,    83,    84,    -1,
      86,    87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,   109,    -1,   111,   112,   113,    -1,   115,
     116,    -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,
      -1,   127,    -1,   129,   130,    -1,   132,   133,     1,    -1,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,
      -1,    -1,    25,    26,    -1,    -1,    29,    30,    -1,    -1,
      -1,    -1,    35,    36,    37,    -1,    -1,    40,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,
      53,    54,    -1,    -1,    57,    -1,    -1,    -1,    -1,    62,
      63,    -1,    65,    -1,    -1,    -1,    69,    70,    71,    -1,
      73,    74,    75,    -1,    -1,    78,    -1,    80,    -1,    82,
      83,    84,    -1,    86,    87,    -1,    89,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    98,    -1,   100,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   108,   109,    -1,   111,   112,
     113,    -1,   115,   116,    -1,   118,    -1,    -1,   121,   122,
     123,    -1,   125,    -1,   127,    -1,   129,   130,    -1,   132,
     133,     1,    -1,     3,     4,     5,     6,     7,    -1,     9,
      10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,
      20,    -1,    -1,    -1,    -1,    25,    26,    -1,    -1,    29,
      30,    -1,    -1,    -1,    -1,    35,    36,    37,    -1,    -1,
      40,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,
      -1,    -1,    52,    53,    54,    -1,    -1,    57,    -1,    -1,
      -1,    -1,    62,    63,    -1,    65,    -1,    -1,    -1,    69,
      70,    71,    -1,    73,    74,    75,    -1,    -1,    78,    -1,
      80,    -1,    82,    83,    84,    -1,    86,    87,    -1,    89,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    98,    -1,
     100,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   108,   109,
      -1,   111,   112,   113,    -1,   115,   116,    -1,   118,    -1,
      -1,   121,   122,   123,    -1,   125,    -1,   127,    -1,   129,
     130,    -1,   132,   133,     1,    -1,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    -1,    19,    20,    -1,    -1,    -1,    -1,    25,    26,
      -1,    -1,    29,    30,    -1,    -1,    -1,    -1,    35,    36,
      37,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    53,    54,    -1,    -1,
      57,    -1,    -1,    -1,    -1,    62,    63,    -1,    65,    -1,
      -1,    -1,    69,    70,    71,    -1,    73,    74,    75,    -1,
      -1,    78,    -1,    80,    -1,    82,    83,    84,    -1,    86,
      87,    -1,    89,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    98,    -1,   100,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   108,   109,    -1,   111,   112,   113,    -1,   115,   116,
      -1,   118,    -1,    -1,   121,   122,   123,    -1,   125,    -1,
     127,    -1,   129,   130,     8,   132,   133,    -1,    12,    -1,
      14,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      24,    -1,    -1,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,
      44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    11,    12,
      -1,    14,    15,    16,    17,    -1,    19,    20,    21,    22,
      23,    24,    -1,    67,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,
      -1,    85,    45,    -1,    -1,    12,    -1,    -1,    -1,    -1,
      17,    -1,    55,    -1,    21,    22,    23,    24,    -1,    -1,
      27,    -1,    -1,    -1,    67,    -1,    -1,    -1,    35,    36,
     114,    38,    -1,   117,    -1,    -1,   120,    -1,    -1,    -1,
     124,   125,    85,    12,   128,    -1,    -1,   131,    17,    -1,
      19,    20,    21,    22,    23,    24,    -1,    -1,    27,    28,
      29,    30,    31,    32,    33,    -1,    35,    36,    -1,    38,
      -1,   114,    -1,    -1,   117,    -1,    -1,   120,    85,    -1,
      -1,   124,   125,    -1,    -1,   128,    -1,    12,   131,    14,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      -1,    -1,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    -1,    85,    -1,   125,    -1,
      45,   128,    -1,    -1,    -1,    50,    -1,    -1,    12,    -1,
      14,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      24,    -1,    67,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,   125,    41,    -1,   128,
      85,    45,   131,    -1,    12,    -1,    -1,    -1,    -1,    17,
      -1,    -1,    -1,    57,    22,    23,    24,    -1,    -1,    27,
      -1,    -1,    -1,    67,   109,    -1,    -1,    35,    36,   114,
      38,    -1,   117,    -1,    -1,   120,    -1,    -1,    -1,   124,
     125,    85,    -1,   128,    -1,    12,   131,    -1,    -1,    -1,
      17,    -1,    19,    20,    21,    22,    23,    24,    -1,    -1,
      27,    -1,    -1,    30,    31,    32,    33,    -1,    35,    36,
     114,    38,    -1,   117,    -1,    -1,   120,    85,    -1,    -1,
     124,   125,    -1,    -1,   128,    -1,    12,   131,    14,    15,
      16,    17,    -1,    19,    20,    21,    22,    23,    24,    -1,
      -1,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    -1,    -1,    41,    -1,   125,    85,    45,
     128,    47,    -1,    -1,    -1,    -1,    -1,    12,    -1,    14,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      -1,    67,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    -1,    41,    -1,   125,    85,
      45,   128,    -1,    -1,   131,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    -1,    -1,    -1,   114,    -1,
      -1,   117,    -1,    -1,   120,    -1,    -1,    -1,   124,   125,
      85,    -1,   128,    -1,    -1,   131,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,
      -1,    -1,   117,    -1,    -1,   120,    -1,    -1,    -1,   124,
     125,    -1,    -1,   128,    -1,    12,   131,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    -1,    -1,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    12,    -1,    14,    15,    16,    17,    45,    19,
      20,    21,    22,    23,    24,    -1,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      67,    -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,    85,    17,
      -1,    19,    20,    21,    22,    23,    24,    67,    -1,    27,
      28,    29,    30,    31,    32,    33,    -1,    35,    36,    37,
      38,    -1,    -1,    -1,    -1,    85,    -1,   114,    -1,    -1,
     117,    -1,    -1,   120,    -1,    -1,    -1,   124,   125,    -1,
      -1,   128,    -1,    -1,   131,    -1,    -1,    -1,    -1,    67,
      -1,    -1,    -1,    -1,   114,    -1,    -1,   117,    -1,    -1,
     120,    -1,    -1,    -1,   124,   125,    -1,    85,   128,    -1,
      12,   131,    14,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    -1,    -1,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    -1,    -1,   125,    -1,    -1,
     128,    -1,    -1,   131,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    -1,    -1,    -1,    12,
      -1,    14,    15,    16,    17,    -1,    19,    20,    21,    22,
      23,    24,    -1,    85,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,   117,    -1,    60,   120,    -1,
      -1,    -1,   124,   125,    67,    -1,   128,    -1,    12,   131,
      14,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      24,    -1,    85,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   114,    -1,    -1,   117,    -1,    60,   120,    -1,    -1,
      -1,   124,   125,    67,    -1,   128,    -1,    12,   131,    14,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      -1,    85,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     114,    -1,    -1,   117,    -1,    -1,   120,    -1,    -1,    -1,
     124,   125,    67,    -1,   128,    -1,    12,   131,    14,    15,
      16,    17,    -1,    19,    20,    21,    22,    23,    24,    -1,
      85,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    -1,    99,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,
      -1,    -1,   117,    -1,    -1,   120,    -1,    -1,    -1,   124,
     125,    67,    -1,   128,    -1,    12,   131,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    -1,    85,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,
      -1,    -1,    -1,   109,    -1,    -1,    -1,    -1,   114,    -1,
      -1,   117,    -1,    -1,   120,    -1,    -1,    -1,   124,   125,
      67,    -1,   128,    -1,    12,   131,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    -1,    85,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,
     117,    -1,    -1,   120,    -1,    -1,    -1,   124,   125,    67,
      -1,   128,    -1,    12,   131,    14,    15,    16,    17,    -1,
      19,    20,    21,    22,    23,    24,    -1,    85,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      -1,    -1,    41,    -1,    -1,    -1,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,   117,
      -1,    -1,   120,    -1,    -1,    -1,   124,   125,    67,    -1,
     128,    -1,    12,   131,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    -1,    85,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,   117,    -1,
      -1,   120,    -1,    -1,    -1,   124,   125,    67,    -1,   128,
      -1,    12,   131,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    -1,    85,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   114,    -1,    -1,   117,    -1,    -1,
     120,    -1,    -1,    -1,   124,   125,    67,    -1,   128,    -1,
      12,   131,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    -1,    85,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   114,    -1,    -1,   117,    -1,    -1,   120,
      -1,    -1,    -1,   124,   125,    67,    -1,   128,    -1,    12,
     131,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    -1,    85,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,   117,    -1,    -1,   120,    -1,
      -1,    -1,   124,   125,    67,    -1,   128,    -1,    12,   131,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    -1,    85,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   114,    -1,    -1,   117,    -1,    -1,   120,    -1,    -1,
      -1,   124,   125,    67,    -1,   128,    -1,    12,   131,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      -1,    85,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     114,    -1,    -1,   117,    -1,    -1,   120,    -1,    -1,    -1,
     124,   125,    67,    -1,   128,    -1,    12,   131,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    -1,
      85,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,
      -1,    -1,   117,    -1,    -1,   120,    -1,    -1,    -1,   124,
     125,    67,    -1,   128,    -1,    12,   131,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    -1,    85,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,
      -1,   117,    -1,    -1,   120,    -1,    -1,    -1,   124,   125,
      67,    -1,   128,    -1,    12,   131,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    -1,    85,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,
     117,    -1,    -1,   120,    -1,    -1,    -1,   124,   125,    67,
      -1,   128,    -1,    12,   131,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    -1,    85,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,   117,
      -1,    -1,   120,    -1,    -1,    -1,   124,   125,    67,    -1,
     128,    -1,    12,   131,    14,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    -1,    85,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      -1,    41,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,   117,    -1,
      -1,   120,    -1,    -1,    -1,   124,   125,    67,    -1,   128,
      -1,    12,   131,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    -1,    85,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   114,    -1,    -1,   117,    -1,    -1,
     120,    -1,    -1,    -1,   124,   125,    67,    -1,   128,    -1,
      12,   131,    14,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    -1,    85,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    41,
      -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   114,    -1,    -1,   117,    -1,    -1,   120,
      -1,    -1,    -1,   124,   125,    67,    -1,   128,    -1,    12,
     131,    14,    15,    16,    17,    -1,    19,    20,    21,    22,
      23,    24,    -1,    85,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    -1,    -1,    50,    -1,    -1,
      -1,    -1,   114,    -1,    -1,   117,    -1,    -1,   120,    -1,
      -1,    -1,   124,   125,    67,    -1,   128,    -1,    12,   131,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    -1,    85,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   114,    -1,    -1,   117,    -1,    -1,   120,    -1,    -1,
      -1,   124,   125,    67,    -1,   128,    -1,    12,   131,    14,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      -1,    85,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    -1,    41,    -1,    -1,    -1,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     114,    -1,    -1,   117,    -1,    -1,   120,    -1,    -1,    -1,
     124,   125,    67,    -1,   128,    -1,    12,   131,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    -1,
      85,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,
      -1,    -1,   117,    -1,    -1,   120,    -1,    -1,    -1,   124,
     125,    67,    -1,   128,    -1,    -1,   131,    -1,    -1,    -1,
      -1,    -1,    -1,    12,    -1,    14,    15,    16,    17,    85,
      19,    20,    21,    22,    23,    24,    -1,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,   114,    -1,
      -1,   117,    -1,    -1,   120,    -1,    -1,    -1,   124,   125,
      -1,    -1,   128,    -1,    -1,   131,    -1,    -1,    67,    -1,
      -1,    -1,    12,    -1,    14,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    -1,    85,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,   117,    -1,
      60,   120,    -1,    -1,    -1,   124,   125,    67,    -1,   128,
      -1,    12,   131,    14,    15,    16,    17,    -1,    19,    20,
      21,    22,    23,    24,    -1,    85,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   114,    -1,    -1,   117,    -1,    -1,
     120,    -1,    -1,    -1,   124,   125,    67,    -1,   128,    -1,
      12,   131,    14,    15,    16,    17,    77,    19,    20,    21,
      22,    23,    24,    -1,    85,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    -1,
      -1,    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   114,    -1,    -1,   117,    -1,    -1,   120,
      -1,    -1,    -1,   124,   125,    67,    -1,   128,    -1,    12,
     131,    14,    15,    16,    17,    -1,    19,    20,    21,    22,
      23,    24,    -1,    85,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,
      -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   114,    -1,    -1,   117,    -1,    -1,   120,    -1,
      -1,    -1,   124,   125,    67,    -1,   128,    -1,    -1,   131,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    85,    12,    -1,    14,    15,    16,    17,    -1,
      19,    20,    21,    22,    23,    24,    -1,    -1,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      -1,   114,    -1,    -1,   117,    -1,    45,   120,    -1,    -1,
      -1,   124,   125,    -1,    -1,   128,    -1,    -1,   131,    -1,
      -1,    -1,    61,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    12,    -1,    14,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    -1,    85,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,   117,    -1,
      -1,   120,    -1,    -1,    -1,   124,   125,    67,    -1,   128,
      -1,    12,   131,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    -1,    85,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   114,    -1,    -1,   117,    -1,    -1,
     120,    -1,    -1,    -1,   124,   125,    67,    -1,   128,    -1,
      12,   131,    14,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    -1,    85,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    41,
      -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   114,    -1,    -1,   117,    -1,    -1,   120,
      -1,    -1,    -1,   124,   125,    67,    -1,   128,    -1,    12,
     131,    14,    15,    16,    17,    -1,    19,    20,    21,    22,
      23,    24,    -1,    85,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    -1,    -1,    50,    -1,    -1,
      -1,    -1,   114,    -1,    -1,   117,    -1,    -1,   120,    -1,
      -1,    -1,   124,   125,    67,    -1,   128,    -1,    12,   131,
      14,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      24,    -1,    85,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   114,    -1,    -1,   117,    -1,    60,   120,    -1,    -1,
      -1,   124,   125,    67,    -1,   128,    -1,    12,   131,    14,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      -1,    85,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     114,    -1,    -1,   117,    -1,    60,   120,    -1,    -1,    -1,
     124,   125,    67,    -1,   128,    -1,    12,   131,    14,    15,
      16,    17,    -1,    19,    20,    21,    22,    23,    24,    -1,
      85,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    -1,    -1,    -1,    -1,    -1,    44,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,
      -1,    -1,   117,    -1,    -1,   120,    -1,    -1,    -1,   124,
     125,    67,    -1,   128,    -1,    12,   131,    14,    15,    16,
      17,    -1,    19,    20,    21,    22,    23,    24,    -1,    85,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,
      -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,   114,    -1,
      -1,   117,    -1,    -1,   120,    -1,    -1,    -1,   124,   125,
      67,    -1,   128,    -1,    12,   131,    14,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    24,    -1,    85,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    -1,    -1,    41,    -1,    -1,    -1,    45,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,
     117,    -1,    -1,   120,    -1,    -1,    -1,   124,   125,    67,
      -1,   128,    -1,    12,   131,    14,    15,    16,    17,    -1,
      19,    20,    21,    22,    23,    24,    -1,    85,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      -1,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,   117,
      -1,    -1,   120,    -1,    -1,    -1,   124,   125,    67,    -1,
     128,    -1,    12,   131,    14,    15,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    -1,    85,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   114,    -1,    -1,   117,    -1,
      -1,   120,    -1,    -1,    -1,   124,   125,    67,    -1,   128,
      -1,    12,   131,    14,    15,    16,    17,    -1,    19,    20,
      21,    22,    23,    24,    -1,    85,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    -1,    -1,
      -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   114,    -1,    -1,   117,    -1,    -1,
     120,    -1,    -1,    -1,   124,   125,    67,    -1,   128,    -1,
      12,   131,    14,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    -1,    85,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   114,    -1,    -1,   117,    -1,    -1,   120,
      -1,    -1,    -1,   124,   125,    67,    -1,   128,    -1,    12,
     131,    -1,    15,    16,    17,    -1,    19,    20,    21,    22,
      23,    24,    -1,    85,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,
      -1,    -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   117,    -1,    -1,   120,    -1,
      -1,    -1,   124,   125,    67,    -1,   128,    -1,    12,   131,
      14,    15,    16,    17,    -1,    19,    20,    21,    22,    23,
      24,    -1,    85,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,
      -1,    45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   114,    -1,    -1,   117,    -1,    -1,   120,    -1,    -1,
      -1,   124,   125,    67,    -1,   128,    -1,    12,   131,    -1,
      15,    16,    17,    -1,    19,    20,    21,    22,    23,    24,
      -1,    85,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   120,    -1,    -1,    -1,
     124,   125,    -1,    -1,   128,    -1,    12,   131,    -1,    -1,
      16,    17,    -1,    19,    20,    21,    22,    23,    24,    -1,
      85,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   114,
      -1,    -1,   117,    -1,    -1,   120,    -1,    -1,    -1,   124,
     125,    67,    -1,   128,    -1,    12,   131,    -1,    -1,    16,
      17,    -1,    19,    20,    21,    22,    23,    24,    -1,    85,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    -1,    -1,    -1,   101,    -1,    -1,    45,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,   125,
      67,    -1,   128,    -1,    12,   131,    -1,    -1,    16,    17,
      -1,    19,    20,    21,    22,    23,    24,    -1,    85,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   124,   125,    67,
      -1,   128,    -1,    12,   131,    -1,    -1,    16,    17,    -1,
      19,    20,    21,    22,    23,    24,    -1,    85,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      -1,    -1,    -1,    -1,    -1,    -1,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   124,   125,    67,    -1,
     128,    -1,    12,   131,    -1,    -1,    16,    17,    -1,    19,
      20,    21,    22,    23,    24,    -1,    85,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    12,
      -1,    -1,    -1,    -1,    17,    -1,    19,    20,    21,    22,
      23,    24,    -1,    -1,    27,    -1,    -1,    30,    -1,    32,
      33,    -1,    35,    36,    -1,    38,   125,    67,    12,   128,
      -1,    -1,   131,    17,    -1,    19,    20,    21,    22,    23,
      24,    -1,    -1,    27,    -1,    85,    30,    -1,    32,    33,
      12,    35,    36,    -1,    38,    17,    -1,    19,    20,    21,
      22,    23,    24,    -1,    -1,    27,    -1,    -1,    30,    -1,
      -1,    -1,    85,    35,    36,    -1,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   125,    -1,    12,   128,    -1,
      -1,   131,    17,    -1,    19,    20,    21,    22,    23,    24,
      -1,    85,    27,    -1,    -1,    -1,    -1,    -1,    -1,    12,
      35,    36,   125,    38,    17,   128,    -1,    -1,   131,    -1,
      23,    24,    -1,    85,    27,    12,    -1,    -1,    -1,    -1,
      17,    -1,    35,    36,    -1,    38,    23,    24,    -1,    -1,
      27,   125,    -1,    -1,   128,    -1,    -1,   131,    35,    36,
      -1,    38,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      85,    -1,    -1,   125,    -1,    -1,   128,    -1,    -1,   131,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    85,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    85,    -1,
     125,    -1,    -1,   128,    -1,    -1,   131,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   125,    -1,    -1,   128,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   125,    -1,
      -1,   128
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     3,     4,     5,     6,     7,     9,    10,    11,
      12,    13,    17,    19,    20,    25,    26,    29,    30,    35,
      36,    37,    40,    46,    47,    52,    53,    54,    57,    62,
      63,    65,    69,    70,    71,    73,    74,    75,    78,    80,
      82,    83,    84,    86,    87,    89,    98,   100,   108,   109,
     111,   112,   113,   115,   116,   118,   121,   122,   123,   125,
     127,   129,   130,   132,   133,   137,   138,   139,   141,   144,
       4,     5,    10,    46,   142,    46,    15,    17,    17,    38,
     139,     4,   139,   139,   139,    13,    78,   139,   139,    14,
     139,     4,   139,   139,   150,     4,    17,   139,    17,    17,
      17,   139,     3,     4,    10,    13,    17,   133,   140,   141,
      17,   139,   139,   139,   151,    76,   158,    17,    17,   141,
      17,   109,   139,    17,    17,    17,    38,    10,   152,   153,
      17,    83,   139,   139,   139,   139,   141,   151,   139,   150,
     139,   151,   151,   158,   141,   139,   153,    38,   139,    17,
     139,    47,    55,   164,   151,   139,    17,     0,     8,    12,
      14,    15,    16,    17,    19,    20,    21,    22,    23,    24,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    44,    45,    67,    85,   114,   117,   120,   124,
     125,   128,   131,    17,    17,     4,    24,   128,     4,    10,
      13,    26,   133,   139,    18,   139,    18,   139,   139,    18,
      26,    26,    26,   139,    57,    41,   139,    50,   109,   139,
     139,     4,    11,    55,   163,   139,   149,    67,    15,    29,
      58,   155,   139,    60,    60,    44,    66,   139,   165,    81,
     143,   146,   151,    72,   139,   150,    17,   139,   139,   139,
     139,   139,    88,    14,   152,   139,    99,    14,    17,    77,
     109,   160,   109,   109,    51,   109,   160,    17,   122,   139,
      18,   139,   151,    77,    50,   139,    67,   139,   139,   139,
     158,   150,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   141,    40,
     139,     8,   138,   139,   139,     6,    10,    14,    19,    21,
      22,   114,   125,   141,   139,   139,   139,   139,   139,   150,
     139,     3,    17,   139,    18,    18,    41,    17,     3,     4,
      10,    13,    17,   141,   147,   148,    18,   151,   158,   139,
      18,    18,    18,     3,   151,    56,    18,    44,   139,   139,
     139,   139,    59,    61,   154,    18,   151,   151,   139,   139,
      79,   145,   151,   143,    77,    17,    18,    18,   150,    18,
      18,    18,    18,    41,   152,    18,   139,   150,    77,   151,
     151,   151,   160,    51,   150,    41,    50,   151,    18,   139,
      18,    23,    67,   139,    41,   139,    17,    38,    18,    18,
       3,    26,   150,    15,    29,    45,    46,   148,    14,    67,
      17,   139,   158,    49,    51,   109,   159,   160,   161,    51,
      76,    60,   139,   149,    60,   139,   139,    59,    60,    63,
     139,   158,    77,    77,    77,    44,   147,    77,   139,   143,
      18,   101,    18,   160,   160,   160,   151,   160,    18,   151,
      51,    77,   139,    41,   150,    40,   139,    15,    74,    24,
      18,    18,   139,   139,   139,   139,    18,   148,   139,   139,
      44,   109,   157,   161,   139,    49,    51,   151,   158,    54,
      55,   162,   151,    44,   151,   154,   139,   151,   139,    44,
      44,    18,    76,   151,    76,   143,   151,   119,   160,    29,
      77,   151,    41,    18,   139,    41,    74,   139,   158,   143,
       3,    41,    47,   157,   139,   158,    50,   139,   160,     3,
      45,    77,    11,   163,   149,    51,    77,    44,    60,   156,
      60,    77,    60,   158,   151,    77,   143,   151,    77,   109,
     151,   119,   139,    44,   164,    77,    41,   143,   151,   139,
      44,   151,    50,    45,   158,    18,   151,   151,   151,   151,
      77,   151,    77,   110,   109,   110,   160,   109,   151,   164,
      41,   151,    77,    41,   159,   151,   158,   139,   158,    77,
      77,    77,    77,    77,   150,   110,   150,   110,   160,    77,
     159,   162,    44,   109,   150,   109,   150,   151,   109,   151,
     109,   160,   151,   160,   151,   160,   160
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (scanner, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, scanner); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void * scanner)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, scanner)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    void * scanner;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (scanner);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void * scanner)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, scanner)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    void * scanner;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, scanner);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule, void * scanner)
#else
static void
yy_reduce_print (yyvsp, yyrule, scanner)
    YYSTYPE *yyvsp;
    int yyrule;
    void * scanner;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       , scanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule, scanner); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, void * scanner)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, scanner)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    void * scanner;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (scanner);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void * scanner);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void * scanner)
#else
int
yyparse (scanner)
    void * scanner;
#endif
#endif
{
/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1806 of yacc.c  */
#line 199 "input_parser.yy"
    {   const giac::context * contextptr = giac_yyget_extra(scanner);
			    if ((yyvsp[(1) - (1)])._VECTptr->size()==1)
			     parsed_gen((yyvsp[(1) - (1)])._VECTptr->front(),contextptr);
                          else
			     parsed_gen(gen(*(yyvsp[(1) - (1)])._VECTptr,_SEQ__VECT),contextptr);
			 }
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 207 "input_parser.yy"
    { (yyval)=vecteur(1,(yyvsp[(1) - (2)])); }
    break;

  case 4:

/* Line 1806 of yacc.c  */
#line 208 "input_parser.yy"
    { if ((yyvsp[(2) - (3)]).val==1) (yyval)=vecteur(1,symbolic(at_nodisp,(yyvsp[(1) - (3)]))); else (yyval)=vecteur(1,(yyvsp[(1) - (3)])); }
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 209 "input_parser.yy"
    { if ((yyvsp[(2) - (3)]).val==1) (yyval)=mergevecteur(makevecteur(symbolic(at_nodisp,(yyvsp[(1) - (3)]))),*(yyvsp[(3) - (3)])._VECTptr); else (yyval)=mergevecteur(makevecteur((yyvsp[(1) - (3)])),*(yyvsp[(3) - (3)])._VECTptr); }
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 212 "input_parser.yy"
    {(yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 213 "input_parser.yy"
    {if (is_one((yyvsp[(1) - (2)]))) (yyval)=(yyvsp[(2) - (2)]); else (yyval)=symbolic(at_prod,gen(makevecteur((yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])),_SEQ__VECT));}
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 214 "input_parser.yy"
    {if (is_one((yyvsp[(1) - (4)]))) (yyval)=symb_pow((yyvsp[(2) - (4)]),(yyvsp[(4) - (4)])); else (yyval)=symbolic(at_prod,gen(makevecteur((yyvsp[(1) - (4)]),symb_pow((yyvsp[(2) - (4)]),(yyvsp[(4) - (4)]))),_SEQ__VECT));}
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 215 "input_parser.yy"
    {if (is_one((yyvsp[(1) - (6)]))) (yyval)=symb_pow((yyvsp[(2) - (6)]),(yyvsp[(5) - (6)])); else (yyval)=symbolic(at_prod,gen(makevecteur((yyvsp[(1) - (6)]),symb_pow((yyvsp[(2) - (6)]),(yyvsp[(5) - (6)]))),_SEQ__VECT));}
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 216 "input_parser.yy"
    {(yyval)=symbolic(at_prod,gen(makevecteur((yyvsp[(1) - (3)]),symb_pow((yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]))) ,_SEQ__VECT));}
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 217 "input_parser.yy"
    { (yyval) =(yyvsp[(1) - (5)])*symbolic(*(yyvsp[(2) - (5)])._FUNCptr,python_compat(giac_yyget_extra(scanner))?denest_sto(os_nary_workaround((yyvsp[(4) - (5)]))):os_nary_workaround((yyvsp[(4) - (5)]))); }
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 218 "input_parser.yy"
    { (yyval) =(yyvsp[(1) - (7)])*symb_pow(symbolic(*(yyvsp[(2) - (7)])._FUNCptr,python_compat(giac_yyget_extra(scanner))?denest_sto(os_nary_workaround((yyvsp[(4) - (7)]))):os_nary_workaround((yyvsp[(4) - (7)]))),(yyvsp[(7) - (7)])); }
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 220 "input_parser.yy"
    { (yyval)=(yyvsp[(1) - (1)]); }
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 221 "input_parser.yy"
    { if ((yyvsp[(1) - (1)]).type==_FUNC) (yyval)=symbolic(*(yyvsp[(1) - (1)])._FUNCptr,gen(vecteur(0),_SEQ__VECT)); else (yyval)=(yyvsp[(1) - (1)]); }
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 224 "input_parser.yy"
    {(yyval) = symb_program_sto((yyvsp[(3) - (6)]),(yyvsp[(3) - (6)])*zero,(yyvsp[(6) - (6)]),(yyvsp[(1) - (6)]),false,giac_yyget_extra(scanner));}
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 225 "input_parser.yy"
    {if (is_array_index((yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),giac_yyget_extra(scanner)) || (abs_calc_mode(giac_yyget_extra(scanner))==38 && (yyvsp[(1) - (6)]).type==_IDNT && strlen((yyvsp[(1) - (6)])._IDNTptr->id_name)==2 && check_vect_38((yyvsp[(1) - (6)])._IDNTptr->id_name))) (yyval)=symbolic(at_sto,gen(makevecteur((yyvsp[(6) - (6)]),symbolic(at_of,gen(makevecteur((yyvsp[(1) - (6)]),(yyvsp[(3) - (6)])) ,_SEQ__VECT))) ,_SEQ__VECT)); else { (yyval) = symb_program_sto((yyvsp[(3) - (6)]),(yyvsp[(3) - (6)])*zero,(yyvsp[(6) - (6)]),(yyvsp[(1) - (6)]),true,giac_yyget_extra(scanner)); (yyval)._SYMBptr->feuille.subtype=_SORTED__VECT;  } }
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 226 "input_parser.yy"
    {if (is_array_index((yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),giac_yyget_extra(scanner)) || (abs_calc_mode(giac_yyget_extra(scanner))==38 && (yyvsp[(3) - (6)]).type==_IDNT && check_vect_38((yyvsp[(3) - (6)])._IDNTptr->id_name))) (yyval)=symbolic(at_sto,gen(makevecteur((yyvsp[(1) - (6)]),symbolic(at_of,gen(makevecteur((yyvsp[(3) - (6)]),(yyvsp[(5) - (6)])) ,_SEQ__VECT))) ,_SEQ__VECT)); else (yyval) = symb_program_sto((yyvsp[(5) - (6)]),(yyvsp[(5) - (6)])*zero,(yyvsp[(1) - (6)]),(yyvsp[(3) - (6)]),false,giac_yyget_extra(scanner));}
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 227 "input_parser.yy"
    { 
         const giac::context * contextptr = giac_yyget_extra(scanner);
         gen g=symb_at((yyvsp[(3) - (6)]),(yyvsp[(5) - (6)]),contextptr); (yyval)=parser_symb_sto((yyvsp[(1) - (6)]),g); 
        }
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 231 "input_parser.yy"
    { 
         const giac::context * contextptr = giac_yyget_extra(scanner);
         gen g=symbolic(at_of,gen(makevecteur((yyvsp[(3) - (8)]),(yyvsp[(6) - (8)])) ,_SEQ__VECT)); (yyval)=parser_symb_sto((yyvsp[(1) - (8)]),g); 
        }
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 235 "input_parser.yy"
    { if ((yyvsp[(3) - (3)]).type==_IDNT) { const char * ch=(yyvsp[(3) - (3)]).print(context0).c_str(); if (ch[0]=='_' && unit_conversion_map().find(ch+1) != unit_conversion_map().end()) (yyval)=symbolic(at_convert,gen(makevecteur((yyvsp[(1) - (3)]),symbolic(at_unit,makevecteur(1,(yyvsp[(3) - (3)])))) ,_SEQ__VECT)); else (yyval)=parser_symb_sto((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])); } else (yyval)=parser_symb_sto((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])); }
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 236 "input_parser.yy"
    { (yyval)=symbolic(at_convert,gen(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])) ,_SEQ__VECT)); }
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 237 "input_parser.yy"
    { (yyval)=symbolic(at_convert,gen(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])) ,_SEQ__VECT)); }
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 238 "input_parser.yy"
    { (yyval)=symbolic(at_convert,gen(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])) ,_SEQ__VECT)); }
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 239 "input_parser.yy"
    { (yyval)=symbolic(at_convert,gen(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])) ,_SEQ__VECT)); }
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 240 "input_parser.yy"
    { (yyval)=symbolic(at_time,(yyvsp[(1) - (3)]));}
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 241 "input_parser.yy"
    { if ((yyvsp[(1) - (3)])==16 || (yyvsp[(1) - (3)])==10 || (yyvsp[(1) - (3)])==8 || (yyvsp[(1) - (3)])==2) (yyval)=symbolic(at_integer_format,(yyvsp[(1) - (3)])); else (yyval)=symbolic(at_solve,symb_equal((yyvsp[(1) - (3)]),0));}
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 242 "input_parser.yy"
    { (yyval)=symbolic(at_convert,gen(makevecteur((yyvsp[(1) - (4)]),symb_unit(plus_one,(yyvsp[(4) - (4)]),giac_yyget_extra(scanner))),_SEQ__VECT)); opened_quote(giac_yyget_extra(scanner)) &= 0x7ffffffd;}
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 243 "input_parser.yy"
    {(yyval) = check_symb_of((yyvsp[(1) - (4)]),python_compat(giac_yyget_extra(scanner))?denest_sto(os_nary_workaround((yyvsp[(3) - (4)]))):os_nary_workaround((yyvsp[(3) - (4)])),giac_yyget_extra(scanner));}
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 244 "input_parser.yy"
    {(yyval) = check_symb_of((yyvsp[(1) - (4)]),python_compat(giac_yyget_extra(scanner))?denest_sto(os_nary_workaround((yyvsp[(3) - (4)]))):os_nary_workaround((yyvsp[(3) - (4)])),giac_yyget_extra(scanner));}
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 245 "input_parser.yy"
    {(yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 246 "input_parser.yy"
    {(yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 247 "input_parser.yy"
    {(yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 248 "input_parser.yy"
    {(yyval) = symbolic(*(yyvsp[(1) - (3)])._FUNCptr,(yyvsp[(3) - (3)]));}
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 249 "input_parser.yy"
    {(yyval) = symbolic(*(yyvsp[(1) - (4)])._FUNCptr,(yyvsp[(3) - (4)]));}
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 250 "input_parser.yy"
    {(yyval) = symbolic(*(yyvsp[(1) - (3)])._FUNCptr,gen(vecteur(0),_SEQ__VECT));}
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 251 "input_parser.yy"
    {(yyval) = symbolic(*(yyvsp[(3) - (3)])._FUNCptr,(yyvsp[(1) - (3)]));}
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 252 "input_parser.yy"
    {(yyval)=symb_test_equal((yyvsp[(1) - (3)]),(yyvsp[(2) - (3)]),(yyvsp[(3) - (3)]));}
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 254 "input_parser.yy"
    {(yyval) = symbolic(*(yyvsp[(2) - (3)])._FUNCptr,makesequence((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]))); }
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 255 "input_parser.yy"
    { 
	if ((yyvsp[(2) - (2)]).type==_SYMB) (yyval)=(yyvsp[(2) - (2)]); else (yyval)=symbolic(at_nop,(yyvsp[(2) - (2)])); 
	(yyval).change_subtype(_SPREAD__SYMB); 
        const giac::context * contextptr = giac_yyget_extra(scanner);
       spread_formula(false,contextptr); 
	}
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 261 "input_parser.yy"
    { if ((yyvsp[(1) - (3)]).is_symb_of_sommet(at_plus) && (yyvsp[(1) - (3)])._SYMBptr->feuille.type==_VECT){ (yyvsp[(1) - (3)])._SYMBptr->feuille._VECTptr->push_back((yyvsp[(3) - (3)])); (yyval)=(yyvsp[(1) - (3)]); } else
  (yyval) =symbolic(*(yyvsp[(2) - (3)])._FUNCptr,gen(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])),_SEQ__VECT));}
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 263 "input_parser.yy"
    {(yyval) = symb_plus((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]).type<_IDNT?-(yyvsp[(3) - (3)]):symbolic(at_neg,(yyvsp[(3) - (3)])));}
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 264 "input_parser.yy"
    {(yyval) = symb_plus((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]).type<_IDNT?-(yyvsp[(3) - (3)]):symbolic(at_neg,(yyvsp[(3) - (3)])));}
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 265 "input_parser.yy"
    {(yyval) =symbolic(*(yyvsp[(2) - (3)])._FUNCptr,gen(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])),_SEQ__VECT));}
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 266 "input_parser.yy"
    {(yyval) =symbolic(*(yyvsp[(2) - (3)])._FUNCptr,gen(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])),_SEQ__VECT));}
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 267 "input_parser.yy"
    {if ((yyvsp[(1) - (3)])==symbolic(at_exp,1) && (yyvsp[(2) - (3)])==at_pow) (yyval)=symbolic(at_exp,(yyvsp[(3) - (3)])); else (yyval) =symbolic(*(yyvsp[(2) - (3)])._FUNCptr,gen(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])),_SEQ__VECT));}
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 268 "input_parser.yy"
    {if ((yyvsp[(2) - (3)]).type==_FUNC) (yyval)=symbolic(*(yyvsp[(2) - (3)])._FUNCptr,gen(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])),_SEQ__VECT)); else (yyval) = symbolic(at_normalmod,gen(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])),_SEQ__VECT));}
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 269 "input_parser.yy"
    {(yyval) = symbolic(*(yyvsp[(2) - (3)])._FUNCptr,gen(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])) ,_SEQ__VECT)); }
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 270 "input_parser.yy"
    {(yyval) = symbolic(*(yyvsp[(2) - (2)])._FUNCptr,gen(makevecteur((yyvsp[(1) - (2)]),RAND_MAX) ,_SEQ__VECT)); }
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 271 "input_parser.yy"
    {(yyval) = symbolic(*(yyvsp[(1) - (2)])._FUNCptr,gen(makevecteur(0,(yyvsp[(2) - (2)])) ,_SEQ__VECT)); }
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 272 "input_parser.yy"
    {(yyval) = makesequence(symbolic(*(yyvsp[(1) - (3)])._FUNCptr,gen(makevecteur(0,RAND_MAX) ,_SEQ__VECT)),(yyvsp[(3) - (3)])); }
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 275 "input_parser.yy"
    {(yyval) = symbolic(*(yyvsp[(2) - (3)])._FUNCptr,gen(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])),_SEQ__VECT));}
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 276 "input_parser.yy"
    {(yyval)= symbolic(at_deuxpoints,gen(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])) ,_SEQ__VECT));}
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 277 "input_parser.yy"
    { 
					if ((yyvsp[(2) - (2)])==unsigned_inf)
						(yyval) = minus_inf;
					else { if ((yyvsp[(2) - (2)]).type==_INT_) (yyval)=(-(yyvsp[(2) - (2)]).val); else { if ((yyvsp[(2) - (2)]).type==_DOUBLE_) (yyval)=(-(yyvsp[(2) - (2)])._DOUBLE_val); else (yyval)=symbolic(at_neg,(yyvsp[(2) - (2)])); } }
				}
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 282 "input_parser.yy"
    { 
					if ((yyvsp[(2) - (2)])==unsigned_inf)
						(yyval) = minus_inf;
					else { if ((yyvsp[(2) - (2)]).type==_INT_ || (yyvsp[(2) - (2)]).type==_DOUBLE_ || (yyvsp[(2) - (2)]).type==_FLOAT_) (yyval)=-(yyvsp[(2) - (2)]); else (yyval)=symbolic(at_neg,(yyvsp[(2) - (2)])); }
				}
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 287 "input_parser.yy"
    {
					if ((yyvsp[(2) - (2)])==unsigned_inf)
						(yyval) = plus_inf;
					else
						(yyval) = (yyvsp[(2) - (2)]);
				}
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 293 "input_parser.yy"
    {(yyval) = polynome_or_sparse_poly1(eval((yyvsp[(2) - (5)]),1, giac_yyget_extra(scanner)),(yyvsp[(4) - (5)]));}
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 294 "input_parser.yy"
    { 
           if ( ((yyvsp[(2) - (3)]).type==_SYMB) && ((yyvsp[(2) - (3)])._SYMBptr->sommet==at_deuxpoints) )
             (yyval) = algebraic_EXTension((yyvsp[(2) - (3)])._SYMBptr->feuille._VECTptr->front(),(yyvsp[(2) - (3)])._SYMBptr->feuille._VECTptr->back());
           else (yyval)=(yyvsp[(2) - (3)]);
        }
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 300 "input_parser.yy"
    { (yyval)=gen(at_of,2); }
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 301 "input_parser.yy"
    {if ((yyvsp[(1) - (3)]).type==_FUNC) *logptr(giac_yyget_extra(scanner))<< ("Warning: "+(yyvsp[(1) - (3)]).print(context0)+" is a reserved word")<<'\n'; if ((yyvsp[(1) - (3)]).type==_INT_) (yyval)=symb_equal((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])); else {(yyval) = parser_symb_sto((yyvsp[(3) - (3)]),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)])==at_array_sto); if ((yyvsp[(3) - (3)]).is_symb_of_sommet(at_program)) *logptr(giac_yyget_extra(scanner))<<"// End defining "<<(yyvsp[(1) - (3)])<<'\n';}}
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 302 "input_parser.yy"
    { (yyval) = symbolic(*(yyvsp[(1) - (2)])._FUNCptr,(yyvsp[(2) - (2)]));}
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 303 "input_parser.yy"
    {(yyval) = symb_args((yyvsp[(3) - (4)]));}
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 304 "input_parser.yy"
    {(yyval) = symb_args((yyvsp[(3) - (4)]));}
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 305 "input_parser.yy"
    { (yyval)=symb_args(vecteur(0)); }
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 306 "input_parser.yy"
    {
	gen tmp=python_compat(giac_yyget_extra(scanner))?denest_sto(os_nary_workaround((yyvsp[(3) - (4)]))):os_nary_workaround((yyvsp[(3) - (4)]));
	// CERR << python_compat(giac_yyget_extra(scanner)) << tmp << '\n';
	(yyval) = symbolic(*(yyvsp[(1) - (4)])._FUNCptr,tmp);
        const giac::context * contextptr = giac_yyget_extra(scanner);
	if (*(yyvsp[(1) - (4)])._FUNCptr==at_maple_mode ||*(yyvsp[(1) - (4)])._FUNCptr==at_xcas_mode ){
          xcas_mode(contextptr)=(yyvsp[(3) - (4)]).val;
        }
        if (*(yyvsp[(1) - (4)])._FUNCptr==at_python_compat)
          python_compat(contextptr)=(yyvsp[(3) - (4)]).val;
	if (*(yyvsp[(1) - (4)])._FUNCptr==at_user_operator){
          user_operator((yyvsp[(3) - (4)]),contextptr);
        }
	}
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 320 "input_parser.yy"
    {
	if ((yyvsp[(3) - (4)]).type==_VECT && (yyvsp[(3) - (4)])._VECTptr->empty())
          giac_yyerror(scanner,"void argument");
	(yyval) = symbolic(*(yyvsp[(1) - (4)])._FUNCptr,python_compat(giac_yyget_extra(scanner))?denest_sto(os_nary_workaround((yyvsp[(3) - (4)]))):os_nary_workaround((yyvsp[(3) - (4)])));	
	}
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 325 "input_parser.yy"
    { 
          const giac::context * contextptr = giac_yyget_extra(scanner);
          (yyval)=symb_at((yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),contextptr);
        }
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 329 "input_parser.yy"
    {
	(yyval) = symbolic(*(yyvsp[(1) - (3)])._FUNCptr,gen(vecteur(0),_SEQ__VECT));
	if (*(yyvsp[(1) - (3)])._FUNCptr==at_rpn)
          rpn_mode(giac_yyget_extra(scanner))=1;
	if (*(yyvsp[(1) - (3)])._FUNCptr==at_alg)
          rpn_mode(giac_yyget_extra(scanner))=0;
	}
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 336 "input_parser.yy"
    {
	(yyval) = (yyvsp[(1) - (1)]);
	}
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 339 "input_parser.yy"
    {(yyval) = symbolic(at_derive,(yyvsp[(1) - (2)]));}
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 340 "input_parser.yy"
    { (yyval)=symbolic(*(yyvsp[(2) - (2)])._FUNCptr,(yyvsp[(1) - (2)])); }
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 342 "input_parser.yy"
    {(yyval) = symbolic(*(yyvsp[(1) - (6)])._FUNCptr,makevecteur(equaltosame((yyvsp[(2) - (6)])),symb_bloc((yyvsp[(4) - (6)])),symb_bloc((yyvsp[(6) - (6)]))));}
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 343 "input_parser.yy"
    {(yyval) = symbolic(*(yyvsp[(1) - (4)])._FUNCptr,makevecteur(equaltosame((yyvsp[(2) - (4)])),(yyvsp[(4) - (4)]),0));}
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 344 "input_parser.yy"
    {
	(yyval) = symbolic(*(yyvsp[(1) - (5)])._FUNCptr,makevecteur(equaltosame((yyvsp[(2) - (5)])),symb_bloc((yyvsp[(4) - (5)])),(yyvsp[(5) - (5)])));
	}
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 347 "input_parser.yy"
    {(yyval) = symbolic(*(yyvsp[(1) - (4)])._FUNCptr,(yyvsp[(3) - (4)]));}
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 348 "input_parser.yy"
    {(yyval) = (yyvsp[(1) - (1)]);}
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 349 "input_parser.yy"
    {(yyval) = symb_program((yyvsp[(3) - (4)]));}
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 350 "input_parser.yy"
    {(yyval) = gen(at_program,3);}
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 351 "input_parser.yy"
    {
          const giac::context * contextptr = giac_yyget_extra(scanner);
         (yyval) = symb_program((yyvsp[(1) - (3)]),zero*(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),contextptr);
        }
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 355 "input_parser.yy"
    {
          const giac::context * contextptr = giac_yyget_extra(scanner);
             if ((yyvsp[(3) - (3)]).type==_VECT) 
                (yyval) = symb_program((yyvsp[(1) - (3)]),zero*(yyvsp[(1) - (3)]),symb_bloc(makevecteur(at_nop,(yyvsp[(3) - (3)]))),contextptr); 
             else 
                (yyval) = symb_program((yyvsp[(1) - (3)]),zero*(yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),contextptr);
		}
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 362 "input_parser.yy"
    {(yyval) = symb_bloc((yyvsp[(3) - (4)]));}
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 363 "input_parser.yy"
    {(yyval) = at_bloc;}
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 365 "input_parser.yy"
    { (yyval)=symbolic(*(yyvsp[(1) - (2)])._FUNCptr,(yyvsp[(2) - (2)])); }
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 367 "input_parser.yy"
    {(yyval) = gen(*(yyvsp[(1) - (1)])._FUNCptr,0);}
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 368 "input_parser.yy"
    { (yyval)=(yyvsp[(2) - (3)]);}
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 370 "input_parser.yy"
    {(yyval) = symbolic(at_break,zero);}
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 371 "input_parser.yy"
    {(yyval) = symbolic(at_continue,zero);}
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 372 "input_parser.yy"
    { 
	/*
	  gen kk(identificateur("index"));
	  vecteur v(*$6._VECTptr);
          const giac::context * contextptr = giac_yyget_extra(scanner);
	  v.insert(v.begin(),symb_sto(symb_at($4,kk,contextptr),$2));
	  $$=symbolic(*$1._FUNCptr,makevecteur(symb_sto(xcas_mode(contextptr)!=0,kk),symb_inferieur_strict(kk,symb_size($4)+(xcas_mode(contextptr)!=0)),symb_sto(symb_plus(kk,plus_one),kk),symb_bloc(v))); 
          */
          if ((yyvsp[(7) - (7)]).type==_INT_ && (yyvsp[(7) - (7)]).val && (yyvsp[(7) - (7)]).val!=2 && (yyvsp[(7) - (7)]).val!=9)
	    giac_yyerror(scanner,"missing loop end delimiter");
 	  bool rg=(yyvsp[(4) - (7)]).is_symb_of_sommet(at_range);
          gen f=(yyvsp[(4) - (7)]).type==_SYMB?(yyvsp[(4) - (7)])._SYMBptr->feuille:0,inc=1;
          if (rg){
            if (f.type!=_VECT) f=makesequence(0,f);
            vecteur v=*f._VECTptr;
            if (v.size()>=2) f=makesequence(v.front(),v[1]-1);
            if (v.size()==3) inc=v[2];
          }
          if (inc.type==_INT_  && inc.val!=0 && f.type==_VECT && f._VECTptr->size()==2 && (rg || ((yyvsp[(4) - (7)]).is_symb_of_sommet(at_interval) 
	  // && f._VECTptr->front().type==_INT_ && f._VECTptr->back().type==_INT_ 
	  )))
            (yyval)=symbolic(*(yyvsp[(1) - (7)])._FUNCptr,makevecteur(symb_sto(f._VECTptr->front(),(yyvsp[(2) - (7)])),inc.val>0?symb_inferieur_egal((yyvsp[(2) - (7)]),f._VECTptr->back()):symb_superieur_egal((yyvsp[(2) - (7)]),f._VECTptr->back()),symb_sto(symb_plus((yyvsp[(2) - (7)]),inc),(yyvsp[(2) - (7)])),symb_bloc((yyvsp[(6) - (7)]))));
          else 
            (yyval)=symbolic(*(yyvsp[(1) - (7)])._FUNCptr,makevecteur(1,symbolic(*(yyvsp[(1) - (7)])._FUNCptr,makevecteur((yyvsp[(2) - (7)]),(yyvsp[(4) - (7)]))),1,symb_bloc((yyvsp[(6) - (7)]))));
	  }
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 397 "input_parser.yy"
    { 
          if ((yyvsp[(9) - (9)]).type==_INT_ && (yyvsp[(9) - (9)]).val && (yyvsp[(9) - (9)]).val!=2 && (yyvsp[(9) - (9)]).val!=9)
	    giac_yyerror(scanner,"missing loop end delimiter");
	  (yyval)=symbolic(*(yyvsp[(1) - (9)])._FUNCptr,makevecteur(1,symbolic(*(yyvsp[(1) - (9)])._FUNCptr,makevecteur((yyvsp[(2) - (9)]),(yyvsp[(4) - (9)]),symb_bloc((yyvsp[(8) - (9)])))),1,symb_bloc((yyvsp[(6) - (9)]))));
	  }
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 402 "input_parser.yy"
    { 
          if ((yyvsp[(9) - (9)]).type==_INT_ && (yyvsp[(9) - (9)]).val && (yyvsp[(9) - (9)]).val!=2 && (yyvsp[(9) - (9)]).val!=9) giac_yyerror(scanner,"missing loop end delimiter");
          gen tmp,st=(yyvsp[(6) - (9)]);  
       if (st==1 && (yyvsp[(4) - (9)])!=1) st=(yyvsp[(4) - (9)]);
          const giac::context * contextptr = giac_yyget_extra(scanner);
	  if (!lidnt(st).empty())
            *logptr(contextptr) << "Warning, step is not numeric " << st << '\n';
          bool b=has_evalf(st,tmp,1,context0);
          if (!b || is_positive(tmp,context0)) 
             (yyval)=symbolic(*(yyvsp[(1) - (9)])._FUNCptr,makevecteur(symb_sto((yyvsp[(3) - (9)]),(yyvsp[(2) - (9)])),symb_inferieur_egal((yyvsp[(2) - (9)]),(yyvsp[(5) - (9)])),symb_sto(symb_plus((yyvsp[(2) - (9)]),b?abs(st,context0):symb_abs(st)),(yyvsp[(2) - (9)])),symb_bloc((yyvsp[(8) - (9)])))); 
          else 
            (yyval)=symbolic(*(yyvsp[(1) - (9)])._FUNCptr,makevecteur(symb_sto((yyvsp[(3) - (9)]),(yyvsp[(2) - (9)])),symb_superieur_egal((yyvsp[(2) - (9)]),(yyvsp[(5) - (9)])),symb_sto(symb_plus((yyvsp[(2) - (9)]),st),(yyvsp[(2) - (9)])),symb_bloc((yyvsp[(8) - (9)])))); 
        }
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 415 "input_parser.yy"
    { 
          if ((yyvsp[(9) - (9)]).type==_INT_ && (yyvsp[(9) - (9)]).val && (yyvsp[(9) - (9)]).val!=2 && (yyvsp[(9) - (9)]).val!=9) giac_yyerror(scanner,"missing loop end delimiter");
         gen tmp,st=(yyvsp[(4) - (9)]); 
        if (st==1 && (yyvsp[(5) - (9)])!=1) st=(yyvsp[(5) - (9)]);
         const giac::context * contextptr = giac_yyget_extra(scanner);
	 if (!lidnt(st).empty())
            *logptr(contextptr) << "Warning, step is not numeric " << st << '\n';
         bool b=has_evalf(st,tmp,1,context0);
         if (!b || is_positive(tmp,context0)) 
           (yyval)=symbolic(*(yyvsp[(1) - (9)])._FUNCptr,makevecteur(symb_sto((yyvsp[(3) - (9)]),(yyvsp[(2) - (9)])),symb_inferieur_egal((yyvsp[(2) - (9)]),(yyvsp[(6) - (9)])),symb_sto(symb_plus((yyvsp[(2) - (9)]),b?abs(st,context0):symb_abs(st)),(yyvsp[(2) - (9)])),symb_bloc((yyvsp[(8) - (9)])))); 
         else 
           (yyval)=symbolic(*(yyvsp[(1) - (9)])._FUNCptr,makevecteur(symb_sto((yyvsp[(3) - (9)]),(yyvsp[(2) - (9)])),symb_superieur_egal((yyvsp[(2) - (9)]),(yyvsp[(6) - (9)])),symb_sto(symb_plus((yyvsp[(2) - (9)]),st),(yyvsp[(2) - (9)])),symb_bloc((yyvsp[(8) - (9)])))); 
        }
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 428 "input_parser.yy"
    { 
          if ((yyvsp[(7) - (7)]).type==_INT_ && (yyvsp[(7) - (7)]).val && (yyvsp[(7) - (7)]).val!=2 && (yyvsp[(7) - (7)]).val!=9) giac_yyerror(scanner,"missing loop end delimiter");
          (yyval)=symbolic(*(yyvsp[(1) - (7)])._FUNCptr,makevecteur(symb_sto((yyvsp[(3) - (7)]),(yyvsp[(2) - (7)])),plus_one,symb_sto(symb_plus((yyvsp[(2) - (7)]),(yyvsp[(4) - (7)])),(yyvsp[(2) - (7)])),symb_bloc((yyvsp[(6) - (7)])))); 
        }
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 432 "input_parser.yy"
    { 
          if ((yyvsp[(9) - (9)]).type==_INT_ && (yyvsp[(9) - (9)]).val && (yyvsp[(9) - (9)]).val!=2 && (yyvsp[(9) - (9)]).val!=9 && (yyvsp[(9) - (9)]).val!=8) giac_yyerror(scanner,"missing loop end delimiter");
          (yyval)=symbolic(*(yyvsp[(1) - (9)])._FUNCptr,makevecteur(symb_sto((yyvsp[(3) - (9)]),(yyvsp[(2) - (9)])),(yyvsp[(6) - (9)]),symb_sto(symb_plus((yyvsp[(2) - (9)]),(yyvsp[(4) - (9)])),(yyvsp[(2) - (9)])),symb_bloc((yyvsp[(8) - (9)])))); 
        }
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 436 "input_parser.yy"
    {(yyval) = gen(*(yyvsp[(1) - (1)])._FUNCptr,4);}
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 441 "input_parser.yy"
    { 
        vecteur v=gen2vecteur((yyvsp[(2) - (4)]));
        v.push_back(symb_ifte(equaltosame((yyvsp[(4) - (4)])),symbolic(at_break,zero),0));
	(yyval)=symbolic(*(yyvsp[(1) - (4)])._FUNCptr,makevecteur(zero,1,zero,symb_bloc(v))); 
	}
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 446 "input_parser.yy"
    { 
        if ((yyvsp[(5) - (5)]).type==_INT_ && (yyvsp[(5) - (5)]).val && (yyvsp[(5) - (5)]).val!=2 && (yyvsp[(5) - (5)]).val!=9) giac_yyerror(scanner,"missing loop end delimiter");
        vecteur v=gen2vecteur((yyvsp[(2) - (5)]));
        v.push_back(symb_ifte(equaltosame((yyvsp[(4) - (5)])),symbolic(at_break,zero),0));
	(yyval)=symbolic(*(yyvsp[(1) - (5)])._FUNCptr,makevecteur(zero,1,zero,symb_bloc(v))); 
	}
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 452 "input_parser.yy"
    {
          if ((yyvsp[(7) - (7)]).type==_INT_ && (yyvsp[(7) - (7)]).val && (yyvsp[(7) - (7)]).val!=4) giac_yyerror(scanner,"missing iferr end delimiter");
           (yyval)=symbolic(at_try_catch,makevecteur(symb_bloc((yyvsp[(2) - (7)])),0,symb_bloc((yyvsp[(4) - (7)])),symb_bloc((yyvsp[(6) - (7)]))));
        }
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 456 "input_parser.yy"
    {
          if ((yyvsp[(5) - (5)]).type==_INT_ && (yyvsp[(5) - (5)]).val && (yyvsp[(5) - (5)]).val!=4) giac_yyerror(scanner,"missing iferr end delimiter");
           (yyval)=symbolic(at_try_catch,makevecteur(symb_bloc((yyvsp[(2) - (5)])),0,symb_bloc((yyvsp[(4) - (5)])),symb_bloc(0)));
        }
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 460 "input_parser.yy"
    {(yyval)=symbolic(at_piecewise,(yyvsp[(2) - (3)])); }
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 461 "input_parser.yy"
    { 
	(yyval)=(yyvsp[(1) - (1)]); 
	// $$.subtype=1; 
	}
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 465 "input_parser.yy"
    { (yyval)=(yyvsp[(2) - (3)]); /* $$.subtype=1; */ }
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 466 "input_parser.yy"
    { (yyval) = symb_dollar((yyvsp[(2) - (2)])); }
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 467 "input_parser.yy"
    {(yyval)=symb_dollar(gen(makevecteur((yyvsp[(1) - (5)]),(yyvsp[(3) - (5)]),(yyvsp[(5) - (5)])) ,_SEQ__VECT));}
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 468 "input_parser.yy"
    { (yyval) = symb_dollar(gen(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])) ,_SEQ__VECT)); }
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 469 "input_parser.yy"
    { (yyval) = symb_dollar(gen(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])) ,_SEQ__VECT)); }
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 470 "input_parser.yy"
    { (yyval)=symb_dollar((yyvsp[(2) - (2)])); }
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 471 "input_parser.yy"
    {  //CERR << $1 << " compose " << $2 << $3 << '\n';
(yyval) = symbolic(*(yyvsp[(2) - (3)])._FUNCptr,gen(makevecteur((yyvsp[(1) - (3)]),python_compat(giac_yyget_extra(scanner))?denest_sto((yyvsp[(3) - (3)])):(yyvsp[(3) - (3)])) ,_SEQ__VECT)); }
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 473 "input_parser.yy"
    {(yyval)=symbolic(at_ans,-1);}
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 474 "input_parser.yy"
    { (yyval) = symbolic(((yyvsp[(2) - (3)]).type==_FUNC?*(yyvsp[(2) - (3)])._FUNCptr:*at_union),gen(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])) ,_SEQ__VECT)); }
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 475 "input_parser.yy"
    { (yyval) = symbolic(((yyvsp[(2) - (4)]).type==_FUNC?*(yyvsp[(2) - (4)])._FUNCptr:*at_union),gen(makevecteur((yyvsp[(1) - (4)]),(yyvsp[(1) - (4)])*(yyvsp[(3) - (4)])/100) ,_SEQ__VECT)); }
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 476 "input_parser.yy"
    { (yyval) = symb_intersect(gen(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])) ,_SEQ__VECT)); }
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 477 "input_parser.yy"
    { (yyval) = symb_minus(gen(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])) ,_SEQ__VECT)); }
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 478 "input_parser.yy"
    { 
	(yyval)=symbolic(*(yyvsp[(2) - (3)])._FUNCptr,gen(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])) ,_SEQ__VECT)); 
	}
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 481 "input_parser.yy"
    { (yyval) = (yyvsp[(1) - (1)]); }
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 482 "input_parser.yy"
    {if ((yyvsp[(2) - (3)]).type==_FUNC) (yyval)=(yyvsp[(2) - (3)]); else { 
          // const giac::context * contextptr = giac_yyget_extra(scanner);
          (yyval)=symb_quote((yyvsp[(2) - (3)]));
          } 
        }
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 487 "input_parser.yy"
    {
          const giac::context * contextptr = giac_yyget_extra(scanner);
	  (yyval) = symb_at((yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]),contextptr);
        }
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 491 "input_parser.yy"
    {
          const giac::context * contextptr = giac_yyget_extra(scanner);
	  (yyval) = symbolic(at_of,gen(makevecteur((yyvsp[(1) - (6)]),(yyvsp[(4) - (6)])) ,_SEQ__VECT));
        }
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 495 "input_parser.yy"
    {(yyval) = check_symb_of((yyvsp[(2) - (6)]),(yyvsp[(5) - (6)]),giac_yyget_extra(scanner));}
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 496 "input_parser.yy"
    {
	if ( ((yyvsp[(1) - (3)])==_LIST__VECT && python_compat(giac_yyget_extra(scanner))) ||
              python_compat(giac_yyget_extra(scanner))==2){
           if (python_compat(giac_yyget_extra(scanner))==2)
             (yyval)=change_subtype((yyvsp[(2) - (3)]),_TUPLE__VECT);
           else
             (yyval)=symbolic(at_python_list,(yyvsp[(2) - (3)]));
        }
        else {
 	 if (abs_calc_mode(giac_yyget_extra(scanner))==38 && (yyvsp[(2) - (3)]).type==_VECT && (yyvsp[(2) - (3)]).subtype==_SEQ__VECT && (yyvsp[(2) - (3)])._VECTptr->size()==2 && ((yyvsp[(2) - (3)])._VECTptr->front().type<=_DOUBLE_ || (yyvsp[(2) - (3)])._VECTptr->front().type==_FLOAT_) && ((yyvsp[(2) - (3)])._VECTptr->back().type<=_DOUBLE_ || (yyvsp[(2) - (3)])._VECTptr->back().type==_FLOAT_)){ 
           const giac::context * contextptr = giac_yyget_extra(scanner);
	   gen a=evalf((yyvsp[(2) - (3)])._VECTptr->front(),1,contextptr),
	       b=evalf((yyvsp[(2) - (3)])._VECTptr->back(),1,contextptr);
	   if ( (a.type==_DOUBLE_ || a.type==_FLOAT_) &&
                (b.type==_DOUBLE_ || b.type==_FLOAT_))
             (yyval)= a+b*cst_i; 
           else (yyval)=(yyvsp[(2) - (3)]);
  	 } else {
              if (calc_mode(giac_yyget_extra(scanner))==1 && (yyvsp[(2) - (3)]).type==_VECT && (yyvsp[(1) - (3)])!=_LIST__VECT &&
	      (yyvsp[(2) - (3)]).subtype==_SEQ__VECT && ((yyvsp[(2) - (3)])._VECTptr->size()==2 || (yyvsp[(2) - (3)])._VECTptr->size()==3) )
                (yyval) = gen(*(yyvsp[(2) - (3)])._VECTptr,_GGB__VECT);
              else
                (yyval)=(yyvsp[(2) - (3)]);
          }
	 }
        }
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 522 "input_parser.yy"
    { 
        //cerr << $1 << " " << $2 << '\n';
        (yyval) = gen(*((yyvsp[(2) - (3)])._VECTptr),(yyvsp[(1) - (3)]).val);
	if ((yyvsp[(2) - (3)])._VECTptr->size()==1 && (yyvsp[(2) - (3)])._VECTptr->front().is_symb_of_sommet(at_ti_semi) ) {
	  (yyval)=(yyvsp[(2) - (3)])._VECTptr->front();
        }
        // cerr << $$ << '\n';

        }
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 531 "input_parser.yy"
    { 
         if ((yyvsp[(1) - (3)]).type==_VECT && (yyvsp[(1) - (3)]).subtype==_SEQ__VECT && !((yyvsp[(3) - (3)]).type==_VECT && (yyvsp[(2) - (3)]).subtype==_SEQ__VECT)){ (yyval)=(yyvsp[(1) - (3)]); (yyval)._VECTptr->push_back((yyvsp[(3) - (3)])); }
	 else
           (yyval) = makesuite((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])); 

        }
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 537 "input_parser.yy"
    { (yyval)=gen(vecteur(0),_SEQ__VECT); }
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 538 "input_parser.yy"
    {(yyval)=symb_findhelp((yyvsp[(2) - (2)]));}
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 539 "input_parser.yy"
    { (yyval)=symb_interrogation((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])); }
    break;

  case 124:

/* Line 1806 of yacc.c  */
#line 540 "input_parser.yy"
    {
          const giac::context * contextptr = giac_yyget_extra(scanner);
          (yyval)=symb_unit(plus_one,(yyvsp[(2) - (2)]),contextptr); 
          opened_quote(giac_yyget_extra(scanner)) &= 0x7ffffffd;	
        }
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 545 "input_parser.yy"
    {
          const giac::context * contextptr = giac_yyget_extra(scanner);
          (yyval)=symb_unit((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]),contextptr); 
          opened_quote(giac_yyget_extra(scanner)) &= 0x7ffffffd;        }
    break;

  case 126:

/* Line 1806 of yacc.c  */
#line 549 "input_parser.yy"
    { (yyval)=symb_pow((yyvsp[(1) - (2)]),(yyvsp[(2) - (2)])); }
    break;

  case 127:

/* Line 1806 of yacc.c  */
#line 550 "input_parser.yy"
    { 
        const giac::context * contextptr = giac_yyget_extra(scanner);
#ifdef HAVE_SIGNAL_H_OLD
	messages_to_print += parser_filename(contextptr) + parser_error(contextptr); 
	/* *logptr(giac_yyget_extra(scanner)) << messages_to_print; */
#endif
	(yyval)=undef;
        spread_formula(false,contextptr); 
	}
    break;

  case 128:

/* Line 1806 of yacc.c  */
#line 559 "input_parser.yy"
    { (yyval)=(yyvsp[(1) - (1)]); }
    break;

  case 129:

/* Line 1806 of yacc.c  */
#line 560 "input_parser.yy"
    { (yyval)=symbolic(*(yyvsp[(1) - (2)])._FUNCptr,(yyvsp[(2) - (2)])); }
    break;

  case 130:

/* Line 1806 of yacc.c  */
#line 561 "input_parser.yy"
    {(yyval) = symbolic(*(yyvsp[(1) - (1)])._FUNCptr,gen(vecteur(0),_SEQ__VECT));}
    break;

  case 131:

/* Line 1806 of yacc.c  */
#line 562 "input_parser.yy"
    {(yyval) = symbolic(*(yyvsp[(1) - (3)])._FUNCptr,gen(vecteur(0),_SEQ__VECT));}
    break;

  case 132:

/* Line 1806 of yacc.c  */
#line 563 "input_parser.yy"
    {
          const giac::context * contextptr = giac_yyget_extra(scanner);
          (yyval) = symb_local((yyvsp[(3) - (4)]),contextptr);
        }
    break;

  case 133:

/* Line 1806 of yacc.c  */
#line 567 "input_parser.yy"
    {(yyval) = gen(at_local,2);}
    break;

  case 134:

/* Line 1806 of yacc.c  */
#line 568 "input_parser.yy"
    {
	(yyval) = symbolic(*(yyvsp[(1) - (6)])._FUNCptr,makevecteur(equaltosame((yyvsp[(3) - (6)])),symb_bloc((yyvsp[(5) - (6)])),(yyvsp[(6) - (6)])));
	}
    break;

  case 135:

/* Line 1806 of yacc.c  */
#line 571 "input_parser.yy"
    {
        vecteur v=makevecteur(equaltosame((yyvsp[(3) - (7)])),(yyvsp[(5) - (7)]),(yyvsp[(7) - (7)]));
	// *logptr(giac_yyget_extra(scanner)) << v << '\n';
	(yyval) = symbolic(*(yyvsp[(1) - (7)])._FUNCptr,v);
	}
    break;

  case 136:

/* Line 1806 of yacc.c  */
#line 576 "input_parser.yy"
    { (yyval)=symb_rpn_prog((yyvsp[(2) - (3)])); }
    break;

  case 137:

/* Line 1806 of yacc.c  */
#line 577 "input_parser.yy"
    { (yyval)=(yyvsp[(1) - (1)]); }
    break;

  case 138:

/* Line 1806 of yacc.c  */
#line 578 "input_parser.yy"
    { (yyval)=symbolic(at_maple_lib,makevecteur((yyvsp[(1) - (4)]),(yyvsp[(3) - (4)]))); }
    break;

  case 139:

/* Line 1806 of yacc.c  */
#line 579 "input_parser.yy"
    { 
          if ((yyvsp[(7) - (7)]).type==_INT_ && (yyvsp[(7) - (7)]).val && (yyvsp[(7) - (7)]).val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           (yyval)=symb_program((yyvsp[(3) - (7)]),zero*(yyvsp[(3) - (7)]),symb_local((yyvsp[(5) - (7)]),(yyvsp[(6) - (7)]),contextptr),contextptr); 
        }
    break;

  case 140:

/* Line 1806 of yacc.c  */
#line 584 "input_parser.yy"
    { 
          if ((yyvsp[(8) - (8)]).type==_INT_ && (yyvsp[(8) - (8)]).val && (yyvsp[(8) - (8)]).val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           (yyval)=symb_program_sto((yyvsp[(4) - (8)]),zero*(yyvsp[(4) - (8)]),symb_local((yyvsp[(6) - (8)]),(yyvsp[(7) - (8)]),contextptr),(yyvsp[(2) - (8)]),false,contextptr); 
        }
    break;

  case 141:

/* Line 1806 of yacc.c  */
#line 589 "input_parser.yy"
    { 
          if ((yyvsp[(7) - (7)]).type==_INT_ && (yyvsp[(7) - (7)]).val && (yyvsp[(7) - (7)]).val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           (yyval)=symb_program_sto((yyvsp[(4) - (7)]),zero*(yyvsp[(4) - (7)]),symb_bloc((yyvsp[(6) - (7)])),(yyvsp[(2) - (7)]),false,contextptr); 
        }
    break;

  case 142:

/* Line 1806 of yacc.c  */
#line 594 "input_parser.yy"
    { 
          if ((yyvsp[(9) - (9)]).type==_INT_ && (yyvsp[(9) - (9)]).val && (yyvsp[(9) - (9)]).val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           (yyval)=symb_program_sto((yyvsp[(4) - (9)]),zero*(yyvsp[(4) - (9)]),symb_local((yyvsp[(7) - (9)]),(yyvsp[(8) - (9)]),contextptr),(yyvsp[(2) - (9)]),false,contextptr); 
        }
    break;

  case 143:

/* Line 1806 of yacc.c  */
#line 599 "input_parser.yy"
    { 
          if ((yyvsp[(8) - (8)]).type==_INT_ && (yyvsp[(8) - (8)]).val && (yyvsp[(8) - (8)]).val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
         (yyval)=symb_program((yyvsp[(3) - (8)]),zero*(yyvsp[(3) - (8)]),symb_local((yyvsp[(5) - (8)]),(yyvsp[(7) - (8)]),contextptr),contextptr); 
        }
    break;

  case 144:

/* Line 1806 of yacc.c  */
#line 604 "input_parser.yy"
    { 
          if ((yyvsp[(8) - (8)]).type==_INT_ && (yyvsp[(8) - (8)]).val && (yyvsp[(8) - (8)]).val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           (yyval)=symb_program_sto((yyvsp[(3) - (8)]),zero*(yyvsp[(3) - (8)]),symb_local((yyvsp[(6) - (8)]),(yyvsp[(7) - (8)]),contextptr),(yyvsp[(1) - (8)]),false,contextptr); 
        }
    break;

  case 145:

/* Line 1806 of yacc.c  */
#line 609 "input_parser.yy"
    { 
          if ((yyvsp[(9) - (9)]).type==_INT_ && (yyvsp[(9) - (9)]).val && (yyvsp[(9) - (9)]).val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           (yyval)=symb_program_sto((yyvsp[(3) - (9)]),zero*(yyvsp[(3) - (9)]),symb_local((yyvsp[(7) - (9)]),(yyvsp[(8) - (9)]),contextptr),(yyvsp[(1) - (9)]),false,contextptr); 
        }
    break;

  case 146:

/* Line 1806 of yacc.c  */
#line 614 "input_parser.yy"
    {(yyval) = symbolic(*(yyvsp[(1) - (9)])._FUNCptr,makevecteur((yyvsp[(3) - (9)]),equaltosame((yyvsp[(5) - (9)])),(yyvsp[(7) - (9)]),symb_bloc((yyvsp[(9) - (9)]))));}
    break;

  case 147:

/* Line 1806 of yacc.c  */
#line 615 "input_parser.yy"
    {(yyval) = symbolic(*(yyvsp[(1) - (10)])._FUNCptr,makevecteur((yyvsp[(3) - (10)]),equaltosame((yyvsp[(5) - (10)])),(yyvsp[(7) - (10)]),(yyvsp[(9) - (10)])));}
    break;

  case 148:

/* Line 1806 of yacc.c  */
#line 616 "input_parser.yy"
    {(yyval) = symbolic(*(yyvsp[(1) - (4)])._FUNCptr,gen2vecteur((yyvsp[(3) - (4)])));}
    break;

  case 149:

/* Line 1806 of yacc.c  */
#line 617 "input_parser.yy"
    {(yyval)=symbolic(at_member,makesequence((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]))); if ((yyvsp[(2) - (3)])==at_not) (yyval)=symbolic(at_not,(yyval));}
    break;

  case 150:

/* Line 1806 of yacc.c  */
#line 618 "input_parser.yy"
    {(yyval)=symbolic(at_not,symbolic(at_member,makesequence((yyvsp[(1) - (4)]),(yyvsp[(4) - (4)]))));}
    break;

  case 151:

/* Line 1806 of yacc.c  */
#line 619 "input_parser.yy"
    { (yyval)=symbolic(at_apply,makesequence(symbolic(at_program,makesequence((yyvsp[(4) - (7)]),0*(yyvsp[(4) - (7)]),vecteur(1,(yyvsp[(2) - (7)])))),(yyvsp[(6) - (7)]))); if ((yyvsp[(1) - (7)])==_TABLE__VECT) (yyval)=symbolic(at_table,(yyval));}
    break;

  case 152:

/* Line 1806 of yacc.c  */
#line 620 "input_parser.yy"
    { (yyval)=symbolic(at_apply,symbolic(at_program,makesequence((yyvsp[(4) - (9)]),0*(yyvsp[(4) - (9)]),vecteur(1,(yyvsp[(2) - (9)])))),symbolic(at_select,makesequence(symbolic(at_program,makesequence((yyvsp[(4) - (9)]),0*(yyvsp[(4) - (9)]),(yyvsp[(8) - (9)]))),(yyvsp[(6) - (9)])))); if ((yyvsp[(1) - (9)])==_TABLE__VECT) (yyval)=symbolic(at_table,(yyval));}
    break;

  case 153:

/* Line 1806 of yacc.c  */
#line 621 "input_parser.yy"
    { 
	vecteur v=makevecteur(zero,equaltosame((yyvsp[(3) - (5)])),zero,symb_bloc((yyvsp[(5) - (5)])));
	(yyval)=symbolic(*(yyvsp[(1) - (5)])._FUNCptr,v); 
	}
    break;

  case 154:

/* Line 1806 of yacc.c  */
#line 625 "input_parser.yy"
    { 
	(yyval)=symbolic(*(yyvsp[(1) - (6)])._FUNCptr,makevecteur(zero,equaltosame((yyvsp[(3) - (6)])),zero,(yyvsp[(5) - (6)]))); 
	}
    break;

  case 155:

/* Line 1806 of yacc.c  */
#line 628 "input_parser.yy"
    { 
          if ((yyvsp[(5) - (5)]).type==_INT_ && (yyvsp[(5) - (5)]).val && (yyvsp[(5) - (5)]).val!=9 && (yyvsp[(5) - (5)]).val!=8) giac_yyerror(scanner,"missing loop end delimiter");
	  (yyval)=symbolic(*(yyvsp[(1) - (5)])._FUNCptr,makevecteur(zero,equaltosame((yyvsp[(2) - (5)])),zero,symb_bloc((yyvsp[(4) - (5)])))); 
        }
    break;

  case 156:

/* Line 1806 of yacc.c  */
#line 632 "input_parser.yy"
    { 
          if ((yyvsp[(5) - (5)]).type==_INT_ && (yyvsp[(5) - (5)]).val && (yyvsp[(5) - (5)]).val!=9 && (yyvsp[(5) - (5)]).val!=8) giac_yyerror(scanner,"missing loop end delimiter");
          (yyval)=symbolic(*(yyvsp[(1) - (5)])._FUNCptr,makevecteur(zero,equaltosame((yyvsp[(2) - (5)])),zero,symb_bloc((yyvsp[(4) - (5)])))); 
        }
    break;

  case 157:

/* Line 1806 of yacc.c  */
#line 636 "input_parser.yy"
    { (yyval)=symb_try_catch(makevecteur(symb_bloc((yyvsp[(2) - (7)])),(yyvsp[(5) - (7)]),symb_bloc((yyvsp[(7) - (7)]))));}
    break;

  case 158:

/* Line 1806 of yacc.c  */
#line 637 "input_parser.yy"
    {(yyval)=symb_try_catch(gen2vecteur((yyvsp[(3) - (4)])));}
    break;

  case 159:

/* Line 1806 of yacc.c  */
#line 638 "input_parser.yy"
    {(yyval)=gen(at_try_catch,3);}
    break;

  case 160:

/* Line 1806 of yacc.c  */
#line 639 "input_parser.yy"
    { (yyval)=symb_case((yyvsp[(3) - (7)]),(yyvsp[(6) - (7)])); }
    break;

  case 161:

/* Line 1806 of yacc.c  */
#line 640 "input_parser.yy"
    { (yyval) = symb_case((yyvsp[(3) - (4)])); }
    break;

  case 162:

/* Line 1806 of yacc.c  */
#line 641 "input_parser.yy"
    { (yyval)=symb_case((yyvsp[(2) - (4)]),(yyvsp[(3) - (4)])); }
    break;

  case 163:

/* Line 1806 of yacc.c  */
#line 642 "input_parser.yy"
    { (yyval)=(yyvsp[(2) - (3)]); }
    break;

  case 164:

/* Line 1806 of yacc.c  */
#line 643 "input_parser.yy"
    { (yyval)=(yyvsp[(1) - (1)]); }
    break;

  case 165:

/* Line 1806 of yacc.c  */
#line 644 "input_parser.yy"
    {(yyval) = gen(*(yyvsp[(1) - (2)])._FUNCptr,0);}
    break;

  case 166:

/* Line 1806 of yacc.c  */
#line 645 "input_parser.yy"
    { (yyval)=symbolic(*(yyvsp[(1) - (3)])._FUNCptr,makevecteur(zero,plus_one,zero,symb_bloc((yyvsp[(2) - (3)])))); }
    break;

  case 167:

/* Line 1806 of yacc.c  */
#line 646 "input_parser.yy"
    {(yyval) = symbolic(*(yyvsp[(1) - (4)])._FUNCptr,makevecteur(equaltosame((yyvsp[(2) - (4)])),(yyvsp[(4) - (4)]),0));}
    break;

  case 168:

/* Line 1806 of yacc.c  */
#line 647 "input_parser.yy"
    { (yyval)=symb_try_catch(makevecteur(symb_bloc((yyvsp[(2) - (5)])),at_break,symb_bloc((yyvsp[(4) - (5)])))); }
    break;

  case 169:

/* Line 1806 of yacc.c  */
#line 648 "input_parser.yy"
    { (yyval)=symb_try_catch(makevecteur(symb_bloc((yyvsp[(2) - (4)])),at_break,0)); }
    break;

  case 170:

/* Line 1806 of yacc.c  */
#line 649 "input_parser.yy"
    { (yyval)=symb_try_catch(makevecteur(symb_bloc((yyvsp[(2) - (6)])),at_break,symb_bloc((yyvsp[(5) - (6)])))); }
    break;

  case 171:

/* Line 1806 of yacc.c  */
#line 650 "input_parser.yy"
    { (yyval)=symb_try_catch(makevecteur(symb_bloc((yyvsp[(2) - (5)])),at_break,0)); }
    break;

  case 172:

/* Line 1806 of yacc.c  */
#line 651 "input_parser.yy"
    { vecteur v1(gen2vecteur((yyvsp[(1) - (3)]))),v3(gen2vecteur((yyvsp[(3) - (3)]))); (yyval)=symbolic(at_ti_semi,makevecteur(v1,v3)); }
    break;

  case 173:

/* Line 1806 of yacc.c  */
#line 652 "input_parser.yy"
    { 
          const giac::context * contextptr = giac_yyget_extra(scanner);
          (yyval)=symb_program_sto((yyvsp[(4) - (13)]),(yyvsp[(4) - (13)])*zero,symb_local((yyvsp[(10) - (13)]),mergevecteur(*(yyvsp[(7) - (13)])._VECTptr,*(yyvsp[(12) - (13)])._VECTptr),contextptr),(yyvsp[(2) - (13)]),false,contextptr); 
	}
    break;

  case 174:

/* Line 1806 of yacc.c  */
#line 656 "input_parser.yy"
    { 
          const giac::context * contextptr = giac_yyget_extra(scanner);
	(yyval)=symb_program_sto((yyvsp[(4) - (12)]),(yyvsp[(4) - (12)])*zero,symb_local((yyvsp[(9) - (12)]),mergevecteur(*(yyvsp[(7) - (12)])._VECTptr,*(yyvsp[(11) - (12)])._VECTptr),contextptr),(yyvsp[(2) - (12)]),false,contextptr); 
	}
    break;

  case 175:

/* Line 1806 of yacc.c  */
#line 660 "input_parser.yy"
    { 
          const giac::context * contextptr = giac_yyget_extra(scanner);
	(yyval)=symb_program_sto((yyvsp[(4) - (12)]),(yyvsp[(4) - (12)])*zero,symb_local((yyvsp[(9) - (12)]),(yyvsp[(11) - (12)]),contextptr),(yyvsp[(2) - (12)]),false,contextptr); 
	}
    break;

  case 176:

/* Line 1806 of yacc.c  */
#line 664 "input_parser.yy"
    { 
	(yyval)=symb_program_sto((yyvsp[(4) - (8)]),(yyvsp[(4) - (8)])*zero,symb_bloc((yyvsp[(7) - (8)])),(yyvsp[(2) - (8)]),false,giac_yyget_extra(scanner)); 
	}
    break;

  case 177:

/* Line 1806 of yacc.c  */
#line 667 "input_parser.yy"
    { (yyval)=symbolic(*(yyvsp[(1) - (3)])._FUNCptr,(yyvsp[(2) - (3)])); }
    break;

  case 178:

/* Line 1806 of yacc.c  */
#line 668 "input_parser.yy"
    { (yyval)=symbolic(*(yyvsp[(1) - (2)])._FUNCptr,(yyvsp[(2) - (2)])); }
    break;

  case 179:

/* Line 1806 of yacc.c  */
#line 669 "input_parser.yy"
    { (yyval)=(yyvsp[(2) - (2)]); }
    break;

  case 180:

/* Line 1806 of yacc.c  */
#line 670 "input_parser.yy"
    { (yyval)=symb_program_sto((yyvsp[(4) - (7)]),(yyvsp[(4) - (7)])*zero,(yyvsp[(7) - (7)]),(yyvsp[(2) - (7)]),false,giac_yyget_extra(scanner));}
    break;

  case 181:

/* Line 1806 of yacc.c  */
#line 671 "input_parser.yy"
    { 
          const giac::context * contextptr = giac_yyget_extra(scanner);
          (yyval)=symb_program_sto((yyvsp[(4) - (13)]),(yyvsp[(4) - (13)])*zero,symb_local((yyvsp[(10) - (13)]),(yyvsp[(12) - (13)]),contextptr),(yyvsp[(2) - (13)]),false,contextptr);
        }
    break;

  case 182:

/* Line 1806 of yacc.c  */
#line 675 "input_parser.yy"
    { (yyval)=symb_program_sto((yyvsp[(4) - (9)]),(yyvsp[(4) - (9)])*zero,symb_bloc((yyvsp[(8) - (9)])),(yyvsp[(2) - (9)]),false,giac_yyget_extra(scanner)); }
    break;

  case 183:

/* Line 1806 of yacc.c  */
#line 676 "input_parser.yy"
    {
           vecteur & v=*(yyvsp[(2) - (5)])._VECTptr;
           if ( (v.size()<3) || v[0].type!=_IDNT){
             *logptr(giac_yyget_extra(scanner)) << "Syntax For name,begin,end[,step]" << '\n';
             (yyval)=undef;
           }
           else {
             gen pas(plus_one);
             if (v.size()==4)
               pas=v[3];
             gen condition;
             if (is_positive(-pas,0))
               condition=symb_superieur_egal(v[0],v[2]);
            else
               condition=symb_inferieur_egal(v[0],v[2]);
            vecteur w=makevecteur(symb_sto(v[1],v[0]),condition,symb_sto(symb_plus(v[0],pas),v[0]),symb_bloc((yyvsp[(4) - (5)])));
             (yyval)=symbolic(*(yyvsp[(1) - (5)])._FUNCptr,w);
           }
	}
    break;

  case 184:

/* Line 1806 of yacc.c  */
#line 695 "input_parser.yy"
    { 
	vecteur v=makevecteur(zero,equaltosame((yyvsp[(2) - (5)])),zero,symb_bloc((yyvsp[(4) - (5)])));
	(yyval)=symbolic(*(yyvsp[(1) - (5)])._FUNCptr,v); 
	}
    break;

  case 185:

/* Line 1806 of yacc.c  */
#line 707 "input_parser.yy"
    { (yyval)=(yyvsp[(1) - (1)]); }
    break;

  case 186:

/* Line 1806 of yacc.c  */
#line 708 "input_parser.yy"
    { (yyval)=(yyvsp[(1) - (1)]); }
    break;

  case 187:

/* Line 1806 of yacc.c  */
#line 709 "input_parser.yy"
    { (yyval)=(yyvsp[(1) - (1)]); }
    break;

  case 188:

/* Line 1806 of yacc.c  */
#line 712 "input_parser.yy"
    { (yyval)=(yyvsp[(1) - (1)]); }
    break;

  case 189:

/* Line 1806 of yacc.c  */
#line 713 "input_parser.yy"
    { 
	       gen tmp((yyvsp[(3) - (3)])); 
	       // tmp.subtype=1; 
	       //$$=symb_check_type(makevecteur(tmp,$1),context0); 
               (yyval)=symbolic(at_deuxpoints,makesequence((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])));
          }
    break;

  case 190:

/* Line 1806 of yacc.c  */
#line 719 "input_parser.yy"
    { (yyval)=symb_double_deux_points(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]))); }
    break;

  case 191:

/* Line 1806 of yacc.c  */
#line 720 "input_parser.yy"
    { (yyval)=symb_double_deux_points(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]))); }
    break;

  case 192:

/* Line 1806 of yacc.c  */
#line 721 "input_parser.yy"
    { (yyval)=symb_double_deux_points(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]))); }
    break;

  case 193:

/* Line 1806 of yacc.c  */
#line 722 "input_parser.yy"
    { (yyval)=symb_double_deux_points(makevecteur((yyvsp[(1) - (5)]),(yyvsp[(4) - (5)]))); }
    break;

  case 194:

/* Line 1806 of yacc.c  */
#line 723 "input_parser.yy"
    { (yyval)=symb_double_deux_points(makevecteur(0,(yyvsp[(2) - (2)]))); }
    break;

  case 195:

/* Line 1806 of yacc.c  */
#line 724 "input_parser.yy"
    { (yyval)=symb_double_deux_points(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]))); }
    break;

  case 196:

/* Line 1806 of yacc.c  */
#line 732 "input_parser.yy"
    { 
	  gen tmp((yyvsp[(1) - (2)])); 
	  // tmp.subtype=1; 
	  // $$=symb_check_type(makevecteur(tmp,$2),context0); 
          (yyval)=symbolic(at_deuxpoints,makesequence((yyvsp[(2) - (2)]),(yyvsp[(1) - (2)])));
	  }
    break;

  case 197:

/* Line 1806 of yacc.c  */
#line 738 "input_parser.yy"
    {(yyval)=symbolic(*(yyvsp[(1) - (2)])._FUNCptr,(yyvsp[(2) - (2)])); }
    break;

  case 198:

/* Line 1806 of yacc.c  */
#line 741 "input_parser.yy"
    { (yyval)=(yyvsp[(1) - (1)]); }
    break;

  case 199:

/* Line 1806 of yacc.c  */
#line 742 "input_parser.yy"
    { (yyval)=(yyvsp[(1) - (1)]); }
    break;

  case 200:

/* Line 1806 of yacc.c  */
#line 745 "input_parser.yy"
    { (yyval)=makevecteur(vecteur(0),vecteur(0)); }
    break;

  case 201:

/* Line 1806 of yacc.c  */
#line 746 "input_parser.yy"
    { vecteur v1 =gen2vecteur((yyvsp[(1) - (2)])); vecteur v2=gen2vecteur((yyvsp[(2) - (2)])); (yyval)=makevecteur(mergevecteur(gen2vecteur(v1[0]),gen2vecteur(v2[0])),mergevecteur(gen2vecteur(v1[1]),gen2vecteur(v2[1]))); }
    break;

  case 202:

/* Line 1806 of yacc.c  */
#line 747 "input_parser.yy"
    { (yyval)=(yyvsp[(2) - (2)]); }
    break;

  case 203:

/* Line 1806 of yacc.c  */
#line 751 "input_parser.yy"
    { if ((yyvsp[(3) - (4)]).type==_VECT) (yyval)=gen(*(yyvsp[(3) - (4)])._VECTptr,_RPN_STACK__VECT); else (yyval)=gen(vecteur(1,(yyvsp[(3) - (4)])),_RPN_STACK__VECT); }
    break;

  case 204:

/* Line 1806 of yacc.c  */
#line 752 "input_parser.yy"
    { (yyval)=gen(vecteur(0),_RPN_STACK__VECT); }
    break;

  case 205:

/* Line 1806 of yacc.c  */
#line 755 "input_parser.yy"
    { if (!(yyvsp[(1) - (3)]).val) (yyval)=makevecteur((yyvsp[(2) - (3)]),vecteur(0)); else (yyval)=makevecteur(vecteur(0),(yyvsp[(2) - (3)]));}
    break;

  case 206:

/* Line 1806 of yacc.c  */
#line 758 "input_parser.yy"
    { (yyval)=(yyvsp[(2) - (3)]); }
    break;

  case 207:

/* Line 1806 of yacc.c  */
#line 761 "input_parser.yy"
    { (yyval)=gen(vecteur(1,(yyvsp[(1) - (1)])),_SEQ__VECT); }
    break;

  case 208:

/* Line 1806 of yacc.c  */
#line 762 "input_parser.yy"
    { 
	       vecteur v=*(yyvsp[(1) - (3)])._VECTptr;
	       v.push_back((yyvsp[(3) - (3)]));
	       (yyval)=gen(v,_SEQ__VECT);
	     }
    break;

  case 209:

/* Line 1806 of yacc.c  */
#line 769 "input_parser.yy"
    { (yyval)=(yyvsp[(1) - (1)]); }
    break;

  case 210:

/* Line 1806 of yacc.c  */
#line 770 "input_parser.yy"
    { (yyval)=parser_symb_sto((yyvsp[(3) - (3)]),(yyvsp[(1) - (3)]),(yyvsp[(2) - (3)])==at_array_sto); }
    break;

  case 211:

/* Line 1806 of yacc.c  */
#line 771 "input_parser.yy"
    { (yyval)=symb_equal((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])); }
    break;

  case 212:

/* Line 1806 of yacc.c  */
#line 772 "input_parser.yy"
    { (yyval)=symbolic(at_deuxpoints,makesequence((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)])));  }
    break;

  case 213:

/* Line 1806 of yacc.c  */
#line 773 "input_parser.yy"
    { (yyval)=(yyvsp[(2) - (3)]); }
    break;

  case 214:

/* Line 1806 of yacc.c  */
#line 774 "input_parser.yy"
    { (yyval)=(yyvsp[(1) - (1)]); *logptr(giac_yyget_extra(scanner)) << "Error: reserved word "<< (yyvsp[(1) - (1)]) <<'\n';}
    break;

  case 215:

/* Line 1806 of yacc.c  */
#line 775 "input_parser.yy"
    { (yyval)=symb_double_deux_points(makevecteur((yyvsp[(1) - (3)]),(yyvsp[(3) - (3)]))); *logptr(giac_yyget_extra(scanner)) << "Error: reserved word "<< (yyvsp[(1) - (3)]) <<'\n'; }
    break;

  case 216:

/* Line 1806 of yacc.c  */
#line 776 "input_parser.yy"
    { 
  const giac::context * contextptr = giac_yyget_extra(scanner);
  (yyval)=string2gen("_"+(yyvsp[(1) - (1)]).print(contextptr),false); 
  if (!giac::first_error_line(contextptr)){
    giac::first_error_line(giac::lexer_line_number(contextptr),contextptr);
    giac:: error_token_name((yyvsp[(1) - (1)]).print(contextptr)+ " (reserved word)",contextptr);
  }
}
    break;

  case 217:

/* Line 1806 of yacc.c  */
#line 784 "input_parser.yy"
    { 
  const giac::context * contextptr = giac_yyget_extra(scanner);
  (yyval)=string2gen("_"+(yyvsp[(1) - (1)]).print(contextptr),false);
  if (!giac::first_error_line(contextptr)){
    giac::first_error_line(giac::lexer_line_number(contextptr),contextptr);
    giac:: error_token_name((yyvsp[(1) - (1)]).print(contextptr)+ " reserved word",contextptr);
  }
}
    break;

  case 218:

/* Line 1806 of yacc.c  */
#line 794 "input_parser.yy"
    { (yyval)=plus_one;}
    break;

  case 219:

/* Line 1806 of yacc.c  */
#line 795 "input_parser.yy"
    { (yyval)=(yyvsp[(1) - (1)]); }
    break;

  case 220:

/* Line 1806 of yacc.c  */
#line 798 "input_parser.yy"
    { (yyval)=gen(vecteur(0),_SEQ__VECT); }
    break;

  case 221:

/* Line 1806 of yacc.c  */
#line 799 "input_parser.yy"
    { (yyval)=makesuite((yyvsp[(1) - (1)])); }
    break;

  case 222:

/* Line 1806 of yacc.c  */
#line 802 "input_parser.yy"
    { (yyval) = gen(makevecteur((yyvsp[(1) - (1)])),_PRG__VECT); }
    break;

  case 223:

/* Line 1806 of yacc.c  */
#line 804 "input_parser.yy"
    { vecteur v(1,(yyvsp[(1) - (2)])); 
			  if ((yyvsp[(1) - (2)]).type==_VECT) v=*((yyvsp[(1) - (2)])._VECTptr); 
			  v.push_back((yyvsp[(2) - (2)])); 
			  (yyval) = gen(v,_PRG__VECT);
			}
    break;

  case 224:

/* Line 1806 of yacc.c  */
#line 809 "input_parser.yy"
    { (yyval) = (yyvsp[(1) - (2)]);}
    break;

  case 225:

/* Line 1806 of yacc.c  */
#line 812 "input_parser.yy"
    { (yyval)=vecteur(0); }
    break;

  case 226:

/* Line 1806 of yacc.c  */
#line 813 "input_parser.yy"
    { (yyval)=mergevecteur(vecteur(1,(yyvsp[(1) - (2)])),*((yyvsp[(2) - (2)])._VECTptr));}
    break;

  case 227:

/* Line 1806 of yacc.c  */
#line 814 "input_parser.yy"
    { (yyval)=mergevecteur(vecteur(1,(yyvsp[(1) - (3)])),*((yyvsp[(3) - (3)])._VECTptr));}
    break;

  case 228:

/* Line 1806 of yacc.c  */
#line 817 "input_parser.yy"
    { (yyval)=(yyvsp[(1) - (1)]); }
    break;

  case 229:

/* Line 1806 of yacc.c  */
#line 887 "input_parser.yy"
    { (yyval)=plus_one; }
    break;

  case 230:

/* Line 1806 of yacc.c  */
#line 888 "input_parser.yy"
    { (yyval)=(yyvsp[(2) - (2)]); }
    break;

  case 231:

/* Line 1806 of yacc.c  */
#line 891 "input_parser.yy"
    { (yyval)=plus_one; }
    break;

  case 232:

/* Line 1806 of yacc.c  */
#line 892 "input_parser.yy"
    { (yyval)=(yyvsp[(2) - (2)]); }
    break;

  case 233:

/* Line 1806 of yacc.c  */
#line 893 "input_parser.yy"
    { (yyval)=(yyvsp[(2) - (2)]); }
    break;

  case 234:

/* Line 1806 of yacc.c  */
#line 894 "input_parser.yy"
    { (yyval)=(yyvsp[(2) - (2)]); }
    break;

  case 235:

/* Line 1806 of yacc.c  */
#line 897 "input_parser.yy"
    { (yyval)=plus_one; }
    break;

  case 236:

/* Line 1806 of yacc.c  */
#line 898 "input_parser.yy"
    { (yyval)=(yyvsp[(1) - (1)]); }
    break;

  case 237:

/* Line 1806 of yacc.c  */
#line 901 "input_parser.yy"
    { (yyval)=0; }
    break;

  case 238:

/* Line 1806 of yacc.c  */
#line 902 "input_parser.yy"
    { (yyval)=(yyvsp[(2) - (3)]); }
    break;

  case 239:

/* Line 1806 of yacc.c  */
#line 903 "input_parser.yy"
    { (yyval)=symb_bloc((yyvsp[(2) - (2)])); }
    break;

  case 240:

/* Line 1806 of yacc.c  */
#line 907 "input_parser.yy"
    { 
	(yyval) = (yyvsp[(2) - (3)]);
	}
    break;

  case 241:

/* Line 1806 of yacc.c  */
#line 910 "input_parser.yy"
    {
          const giac::context * contextptr = giac_yyget_extra(scanner);
          (yyval) = symb_local((yyvsp[(2) - (4)]),(yyvsp[(3) - (4)]),contextptr);
         }
    break;

  case 242:

/* Line 1806 of yacc.c  */
#line 917 "input_parser.yy"
    { if ((yyvsp[(1) - (1)]).type==_INT_ && (yyvsp[(1) - (1)]).val && (yyvsp[(1) - (1)]).val!=4) giac_yyerror(scanner,"missing test end delimiter"); (yyval)=0; }
    break;

  case 243:

/* Line 1806 of yacc.c  */
#line 918 "input_parser.yy"
    {
          if ((yyvsp[(3) - (3)]).type==_INT_ && (yyvsp[(3) - (3)]).val && (yyvsp[(3) - (3)]).val!=4) giac_yyerror(scanner,"missing test end delimiter");
	(yyval)=symb_bloc((yyvsp[(2) - (3)])); 
	}
    break;

  case 244:

/* Line 1806 of yacc.c  */
#line 922 "input_parser.yy"
    { 
	  (yyval)=symb_ifte(equaltosame((yyvsp[(2) - (5)])),symb_bloc((yyvsp[(4) - (5)])),(yyvsp[(5) - (5)]));
	  }
    break;

  case 245:

/* Line 1806 of yacc.c  */
#line 925 "input_parser.yy"
    { 
	  (yyval)=symb_ifte(equaltosame((yyvsp[(3) - (6)])),symb_bloc((yyvsp[(5) - (6)])),(yyvsp[(6) - (6)]));
	  }
    break;

  case 246:

/* Line 1806 of yacc.c  */
#line 930 "input_parser.yy"
    { (yyval)=(yyvsp[(1) - (1)]); }
    break;

  case 247:

/* Line 1806 of yacc.c  */
#line 931 "input_parser.yy"
    { (yyval)=(yyvsp[(2) - (2)]); }
    break;

  case 248:

/* Line 1806 of yacc.c  */
#line 934 "input_parser.yy"
    { (yyval)=0; }
    break;

  case 249:

/* Line 1806 of yacc.c  */
#line 935 "input_parser.yy"
    { (yyval)=0; }
    break;

  case 250:

/* Line 1806 of yacc.c  */
#line 938 "input_parser.yy"
    { (yyval)=vecteur(0); }
    break;

  case 251:

/* Line 1806 of yacc.c  */
#line 939 "input_parser.yy"
    { (yyval)=makevecteur(symb_bloc((yyvsp[(3) - (3)])));}
    break;

  case 252:

/* Line 1806 of yacc.c  */
#line 940 "input_parser.yy"
    { (yyval)=mergevecteur(makevecteur((yyvsp[(2) - (5)]),symb_bloc((yyvsp[(4) - (5)]))),*((yyvsp[(5) - (5)])._VECTptr));}
    break;

  case 253:

/* Line 1806 of yacc.c  */
#line 943 "input_parser.yy"
    { (yyval)=vecteur(0); }
    break;

  case 254:

/* Line 1806 of yacc.c  */
#line 944 "input_parser.yy"
    { (yyval)=vecteur(1,symb_bloc((yyvsp[(2) - (2)]))); }
    break;

  case 255:

/* Line 1806 of yacc.c  */
#line 945 "input_parser.yy"
    { (yyval)=mergevecteur(makevecteur((yyvsp[(2) - (5)]),symb_bloc((yyvsp[(4) - (5)]))),*((yyvsp[(5) - (5)])._VECTptr));}
    break;

  case 256:

/* Line 1806 of yacc.c  */
#line 948 "input_parser.yy"
    { (yyval)=vecteur(0); }
    break;

  case 257:

/* Line 1806 of yacc.c  */
#line 949 "input_parser.yy"
    { (yyval)=vecteur(1,symb_bloc((yyvsp[(2) - (2)]))); }
    break;

  case 258:

/* Line 1806 of yacc.c  */
#line 950 "input_parser.yy"
    { (yyval)=mergevecteur(makevecteur((yyvsp[(2) - (6)]),symb_bloc((yyvsp[(4) - (6)]))),gen2vecteur((yyvsp[(6) - (6)])));}
    break;

  case 259:

/* Line 1806 of yacc.c  */
#line 951 "input_parser.yy"
    { (yyval)=mergevecteur(makevecteur((yyvsp[(2) - (7)]),symb_bloc((yyvsp[(4) - (7)]))),gen2vecteur((yyvsp[(7) - (7)])));}
    break;

  case 260:

/* Line 1806 of yacc.c  */
#line 954 "input_parser.yy"
    { (yyval)=(yyvsp[(1) - (1)]); }
    break;



/* Line 1806 of yacc.c  */
#line 7153 "y.tab.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (scanner, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (scanner, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, scanner);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, scanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (scanner, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, scanner);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, scanner);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 2067 of yacc.c  */
#line 961 "input_parser.yy"


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

