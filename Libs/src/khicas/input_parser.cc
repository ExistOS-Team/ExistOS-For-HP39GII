/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

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
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         giac_yyparse
#define yylex           giac_yylex
#define yyerror         giac_yyerror
#define yydebug         giac_yydebug
#define yynerrs         giac_yynerrs


/* Copy the first part of user declarations.  */
#line 27 "input_parser.yy" /* yacc.c:339  */

         #define YYPARSE_PARAM scanner
         #define YYLEX_PARAM   scanner
	 
#line 36 "input_parser.yy" /* yacc.c:339  */

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

#line 152 "y.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "y.tab.h".  */
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

/* Copy the second part of user declarations.  */

#line 386 "y.tab.c" /* yacc.c:358  */

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
#else
typedef signed char yytype_int8;
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
# elif ! defined YYSIZE_T
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
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */
#define YYSTACK_USE_ALLOCA 1
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
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
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  109
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   7275

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  93
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  25
/* YYNRULES -- Number of rules.  */
#define YYNRULES  200
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  437

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   347

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
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
      85,    86,    87,    88,    89,    90,    91,    92
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   184,   184,   192,   193,   194,   197,   198,   199,   200,
     201,   202,   204,   205,   208,   209,   210,   211,   215,   219,
     220,   221,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   243,   245,
     246,   247,   248,   249,   250,   251,   253,   254,   255,   264,
     265,   270,   271,   272,   273,   274,   275,   276,   285,   290,
     294,   301,   304,   305,   306,   307,   308,   311,   312,   313,
     317,   324,   325,   326,   327,   328,   330,   332,   333,   335,
     336,   337,   355,   360,   373,   374,   379,   385,   389,   393,
     397,   398,   399,   400,   401,   402,   403,   405,   406,   407,
     408,   409,   410,   413,   414,   419,   423,   427,   428,   450,
     456,   462,   463,   464,   465,   474,   477,   482,   487,   492,
     497,   502,   507,   512,   517,   518,   519,   520,   521,   522,
     523,   524,   528,   531,   535,   536,   537,   538,   539,   540,
     543,   544,   545,   548,   549,   554,   555,   556,   557,   558,
     559,   567,   572,   575,   576,   579,   580,   584,   587,   588,
     595,   596,   597,   598,   599,   600,   601,   602,   610,   620,
     621,   624,   625,   628,   630,   635,   639,   640,   643,   644,
     645,   646,   649,   650,   653,   654,   655,   658,   661,   668,
     669,   673,   678,   681,   684,   685,   686,   689,   690,   691,
     694
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_NUMBER", "T_SYMBOL", "T_LITERAL",
  "T_DIGITS", "T_STRING", "T_END_INPUT", "T_EXPRESSION", "T_UNARY_OP",
  "T_OF", "T_NOT", "T_TYPE_ID", "T_VIRGULE", "T_AFFECT", "T_MAPSTO",
  "T_BEGIN_PAR", "T_END_PAR", "T_PLUS", "T_MOINS", "T_DIV", "T_MOD",
  "T_POW", "T_QUOTED_BINARY", "T_QUOTE", "T_PRIME", "T_TEST_EQUAL",
  "T_EQUAL", "T_INTERVAL", "T_UNION", "T_INTERSECT", "T_MINUS", "T_AND_OP",
  "T_COMPOSE", "T_DOLLAR", "T_DOLLAR_MAPLE", "T_INDEX_BEGIN",
  "T_VECT_BEGIN", "T_VECT_DISPATCH", "T_VECT_END", "T_SEMI",
  "T_DEUXPOINTS", "T_DOUBLE_DEUX_POINTS", "T_IF", "T_ELIF", "T_THEN",
  "T_ELSE", "T_IFTE", "T_SWITCH", "T_CASE", "T_DEFAULT", "T_ENDCASE",
  "T_FOR", "T_FROM", "T_TO", "T_DO", "T_BY", "T_WHILE", "T_MUPMAP_WHILE",
  "T_REPEAT", "T_UNTIL", "T_IN", "T_START", "T_BREAK", "T_CONTINUE",
  "T_TRY", "T_CATCH", "T_TRY_CATCH", "T_PROC", "T_BLOC", "T_BLOC_BEGIN",
  "T_BLOC_END", "T_RETURN", "T_LOCAL", "T_ARGS", "T_FACTORIAL",
  "T_ROOTOF_BEGIN", "T_ROOTOF_END", "T_SPOLY1_BEGIN", "T_SPOLY1_END",
  "T_HELP", "TI_STO", "T_PIPE", "TI_HASH", "T_INTERROGATION", "T_SQ",
  "T_IFERR", "T_UNARY_OP_38", "T_FUNCTION", "T_LOGO", "T_BIDON",
  "T_IMPMULT", "$accept", "input", "correct_input", "exp", "symbol_for",
  "symbol", "symbol_or_literal", "entete", "local", "suite_symbol",
  "affectable_symbol", "exp_or_empty", "suite", "prg_suite", "step",
  "from", "loop38_do", "else", "bloc", "elif", "ti_bloc_end", "ti_else",
  "switch", "case", "semi", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
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
     345,   346,   347
};
# endif

#define YYPACT_NINF -289

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-289)))

#define YYTABLE_NINF -199

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-199)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    4253,  -289,    27,    50,  -289,    65,  -289,  -289,    34,  -289,
    4253,    39,  4253,  4253,  -289,  4341,  4253,  3109,  -289,    79,
    4253,  3197,    82,  4429,    52,    81,  4517,   188,  4605,  4253,
    -289,  -289,    33,    92,     5,    94,   733,    55,  4253,  4253,
    4253,  4253,  4253,   103,  4957,   142,  -289,  5038,   128,  -289,
    -289,   129,   147,   -20,    20,  4253,  3285,  3373,  4253,   315,
    -289,  5300,   315,    10,   997,  5338,  6947,  4253,  6244,  -289,
    7050,  5375,   114,  -289,  4253,  5450,  4253,  4253,  4693,  5075,
     116,   -37,  -289,    39,  3461,  -289,   104,    14,  4253,  5487,
    6650,  1437,  3549,    98,  4253,  3637,   150,  4253,  6650,  4253,
    4253,  5525,  6687,  6650,    73,  1525,  4253,  3725,  6947,  -289,
    -289,   106,  4253,  4253,  3813,  3637,  4253,  4253,  4253,  4253,
    -289,  4253,  4253,   469,  4253,  4253,  4253,  4253,  4253,  4253,
    4253,  4781,  3901,  4253,  4253,  -289,   303,  4253,  4253,  -289,
    3637,  4253,  -289,   166,  -289,  -289,  -289,  -289,  4253,  -289,
    6800,  -289,  5562,  -289,  5600,  5637,   153,  -289,   557,  -289,
    6872,   193,  -289,  5675,  3813,  5712,  5750,    36,   168,  4253,
     120,  5787,   136,  4253,  4253,  4253,  4253,   123,  5825,  4253,
    -289,  4253,  6650,  -289,  3989,  1613,   162,  5862,  6650,   164,
    3637,  5900,  5937,  5975,  -289,  4253,  4253,  6012,  -289,  4253,
    6872,  6800,  6984,  -289,   165,  7154,  7170,  7189,   205,  7071,
      58,  7050,  6244,  7098,  5419,  7127,  5150,    73,   126,  7050,
     -12,  3197,  6050,  -289,  -289,  6947,  7013,  -289,  -289,  -289,
    -289,  -289,  -289,   127,  6725,  6909,   171,  6087,  -289,  6125,
    -289,  -289,  -289,  3637,   116,    19,   141,    39,   193,  -289,
     -13,  -289,   821,  1085,   139,  -289,   122,  -289,   138,  1701,
    -289,  -289,  3461,  6162,  6650,  6650,  6650,  4253,   909,  1789,
    6200,   193,  -289,  1877,  -289,  4253,  -289,  -289,   182,  -289,
    -289,  -289,  5263,  1173,  -289,  7013,  -289,  4253,   645,  4253,
    5113,  -289,  3637,  4869,    -4,   185,  -289,   191,  4253,  4253,
    4253,  4253,   195,   193,  4253,  3637,  6275,   167,  4253,  -289,
    -289,  -289,  -289,  4253,    33,    85,  4253,  6650,   170,  4253,
    6312,  6387,  -289,  -289,  -289,    32,  -289,  6424,  1965,  2053,
    -289,  4253,  -289,  7071,  7013,   175,   198,  3197,  6462,  4077,
    -289,   215,  -289,  6650,  6837,  6762,  6837,  -289,  -289,  5188,
    5300,   167,  -289,  3813,  6500,  2141,  -289,   216,   178,   151,
    1261,  4165,  1349,  4253,    40,  -289,  -289,    33,  4253,  2229,
    -289,  3989,  2317,  2405,  -289,  -289,  5225,  -289,     5,  6800,
    -289,  3989,  -289,  -289,  4253,  -289,  6537,  -289,  4253,  -289,
     184,    33,  -289,   168,  -289,   206,  4253,  -289,  6650,  -289,
    -289,  4253,  -289,  2493,  -289,  3989,  2581,  -289,  -289,   187,
    3989,  2669,  6575,  -289,  1085,    33,  -289,  3813,  2757,  2845,
    -289,  2933,  -289,  -289,  3021,  -289,  -289,  -289,    85,  6612,
    -289,  -289,  -289,  -289,  -289,  -289,  -289
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,   114,     6,   143,    28,    29,    12,    13,    61,    51,
       0,    89,     0,     0,   103,     0,     0,     0,    97,     0,
       0,     0,     0,     0,    68,     0,     0,    84,     0,     0,
      79,    80,     0,   136,     0,    72,     0,    56,     0,     0,
       0,     0,     0,     0,     0,     0,     2,     0,    27,   153,
     154,     0,     0,     7,     0,     0,     0,     0,     0,    53,
     151,     0,    48,    89,     0,     0,    37,     0,    44,    95,
      91,   172,     0,   149,     0,     0,     0,     0,     0,   197,
       0,   143,   141,     0,     0,   142,     0,   178,     0,     0,
     173,     0,     0,     0,     0,     0,     0,     0,    76,     0,
       0,     0,     0,   111,   152,     0,     0,     0,    73,     1,
       3,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      62,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    63,     0,     0,     0,   113,
       0,     0,   150,     0,     9,   146,   145,   144,     0,   147,
      30,    32,     0,    60,     0,     0,   108,    90,     0,   104,
      45,     0,   109,     0,     0,     0,     0,   143,     0,     0,
       0,   170,     0,     0,     0,     0,     0,     0,     0,     0,
     200,     0,   174,   175,     0,     0,     0,     0,   172,     0,
       0,     0,     0,     0,    50,     0,     0,     0,    75,     0,
     110,    52,    70,    69,     0,    38,    39,    41,    40,    34,
      27,    36,    42,    98,   100,   101,    46,    96,    94,    93,
      27,     0,     0,     4,     5,    47,   127,    33,    20,    23,
      21,    22,    24,    19,   102,   112,     0,     0,     8,     0,
      31,    57,    59,     0,   168,   143,   165,   167,     0,   160,
       0,   158,     0,     0,    65,    67,     0,   138,     0,     0,
     139,   126,     0,     0,   179,   180,   181,     0,     0,     0,
      85,     0,   156,     0,   187,     0,   135,   155,     0,    71,
      54,    55,   110,     0,    58,   128,    26,     0,     0,     0,
       0,   105,     0,     0,    25,    10,   148,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   184,     0,   193,
     192,    66,   189,     0,     0,   194,     0,   170,     0,     0,
     176,     0,   131,   133,    86,     0,   188,     0,     0,     0,
      49,     0,    88,    35,    92,     0,     0,     0,     0,     0,
     155,     0,   107,   161,   162,   163,   166,   164,   159,     0,
       0,   184,   115,     0,     0,     0,    64,     0,     0,     0,
       0,     0,     0,     0,     0,   132,   157,     0,     0,     0,
     155,     0,     0,     0,   106,    16,     0,    17,   155,    15,
      14,     0,    11,   129,     0,   116,     0,   186,     0,   190,
       0,     0,   137,    51,   199,     0,     0,    81,   177,   182,
     183,     0,   134,     0,   117,     0,     0,   119,    87,     0,
       0,     0,     0,   185,     0,     0,   195,     0,     0,     0,
     121,     0,   118,    18,     0,   122,   130,   191,   194,     0,
     124,    82,    83,   120,   123,   196,   125
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -289,  -289,    97,     0,  -289,    26,  -289,  -190,  -289,   -39,
    -244,  -260,   -88,    61,  -289,  -289,  -289,  -118,    43,  -180,
    -120,  -288,  -191,  -122,  -289
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    45,    46,    90,    86,    48,    53,   184,   272,   250,
     251,   172,    72,    91,   364,   177,   401,   352,    93,   311,
     312,   313,   359,   170,   183
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      47,   303,   318,   143,   302,   140,    54,   189,    80,     3,
      59,   339,    61,    62,    60,    65,    66,    68,    83,   353,
      70,    71,    95,    75,   145,  -140,    79,   204,    89,   174,
     146,    49,    50,   147,   298,   157,    98,    51,   101,   102,
     103,   104,   175,    60,   108,   148,   303,   299,    22,   304,
     289,    57,   236,    87,   257,   150,   152,   154,   155,   348,
      96,   300,    54,   353,    98,   340,   144,   160,   176,    76,
      52,    58,    99,   366,   163,   140,   165,   166,    61,    54,
      55,   399,    56,    69,   171,   287,    73,   328,   178,    41,
     115,   182,   100,    54,   187,   188,   400,   191,    77,   192,
     193,   395,   278,   105,    92,   182,   197,    61,   149,    94,
     131,    97,   200,   201,   202,   188,   205,   206,   207,   208,
     106,   209,   211,   212,   213,   214,   215,   216,   217,   218,
     219,   222,    47,   225,   226,   357,   358,   234,   235,   371,
     188,   237,   109,   115,   292,   140,   141,   210,   239,   119,
     381,   142,   120,   185,   162,   297,   220,   203,    65,    52,
     128,  -199,   233,   131,   293,   186,   173,   190,   199,   238,
     243,   258,   260,   263,   264,   265,   266,   262,   267,   275,
     405,   270,   277,   286,   301,   182,   314,   249,   410,   294,
     188,    80,    81,   315,   316,   282,   244,   245,    82,   285,
     329,    83,   135,   246,   336,    84,   247,   254,   341,   342,
     248,   361,   139,   347,   309,   374,   375,   297,   382,   390,
     391,   290,   115,   392,   417,   253,   415,   423,   119,   224,
     259,    22,   325,   385,   427,   389,    22,   435,   394,   128,
     269,     0,   131,   188,     0,   273,     0,     0,     0,     0,
       0,     0,   306,   182,     0,     0,     0,   283,     0,   182,
       0,     0,   317,     0,     0,     0,     0,   320,   321,   182,
       0,     0,    41,   182,   249,   327,    85,    41,     0,     0,
       0,   135,     0,   182,     0,     0,     0,   333,   207,   334,
       0,   139,   188,   338,     0,   307,     0,   249,   343,   344,
     345,   346,     0,     0,   349,   350,    80,     3,   354,   227,
       0,   322,     0,   228,     0,     0,    83,   229,     0,     0,
       0,     0,   230,     0,   231,     0,     0,  -199,     0,   249,
       0,     0,   115,     0,     0,     0,     0,   376,   119,   379,
       0,   120,     0,     0,     0,     0,    22,     0,     0,   128,
     129,     0,   131,   386,     0,   182,     0,   356,     0,     0,
     182,   317,   182,   398,     0,     0,     0,     0,     0,   182,
       0,     0,   182,   182,   355,     0,     0,   360,     0,     0,
     362,     0,   380,     0,   412,   232,     0,    41,     0,   369,
     372,   135,   373,     0,     0,     0,   387,     0,     0,     0,
       0,   139,     0,   182,    96,     0,   182,     0,     0,     0,
     402,   182,     0,     0,   182,     0,     0,   429,   182,   182,
       0,   182,     0,     0,   182,     0,     0,     0,     0,   403,
       0,     0,   406,     0,   416,     0,     0,     0,     0,     0,
       0,     0,   411,     0,     0,     0,     0,     0,     0,   414,
       0,     0,     0,     0,     0,     0,     0,   418,   428,     0,
     430,     0,   419,     0,     0,     0,   421,     0,     0,     0,
       1,   424,     2,     3,     4,     5,     6,   -43,     7,     8,
       9,    10,    11,   -43,   -43,   -43,    12,   -43,    13,     0,
     -43,   -43,   -43,    14,    15,   -43,   -43,   -43,   -43,   -43,
     -43,   -43,   -43,    18,    19,   -43,   -43,     0,    21,   -43,
     -43,   -43,    22,    23,   -43,   -43,   -43,    24,    25,    26,
     -43,   -43,   -43,   -43,   -43,   -43,   -43,    28,     0,    29,
     -43,   -43,     0,    30,    31,    32,     0,    33,    34,    35,
       0,   -43,   -43,     0,    37,   -43,    38,   -43,    39,   -43,
      40,   -43,   -43,    41,   -43,   -43,    42,    43,     1,   -43,
       2,     3,     4,     5,     6,   -78,     7,     8,     9,    10,
      63,   -78,   -78,   -78,    12,   -78,    13,     0,   -78,   -78,
     -78,    14,    15,   -78,   -78,    16,    17,   -78,   -78,   -78,
     -78,    18,    19,    20,   -78,     0,    21,   -78,   -78,   -78,
      22,    23,   -78,   -78,   -78,    24,    25,    26,   -78,   -78,
      27,   -78,   -78,   -78,   -78,    28,     0,    29,   -78,   -78,
       0,    30,    31,    32,     0,    33,    34,    35,     0,   -78,
      64,     0,    37,   -78,    38,   -78,    39,   -78,    40,   -78,
     -78,    41,   -78,   -78,    42,    43,     1,    44,   -99,     3,
       4,     5,     6,   -99,     7,     8,     9,    10,    11,   -99,
     -99,   -99,    12,   -99,   -99,     0,   -99,   -99,   -99,    14,
      15,   -99,   -99,   -99,   -99,   -99,   -99,   -99,   -99,    18,
      19,   -99,   -99,     0,    21,   -99,   -99,   -99,    22,    23,
     -99,   -99,   -99,    24,    25,    26,   -99,   -99,   -99,   -99,
     -99,   -99,   -99,    28,     0,    29,   -99,   -99,     0,    30,
      31,    32,     0,    33,    34,    35,     0,   -99,   -99,     0,
      37,   -99,    38,   -99,    39,   -99,    40,   -99,   -99,    41,
     -99,   -99,    42,    43,     1,   -99,     2,     3,     4,     5,
       6,   -77,     7,     8,     9,    10,    11,   -77,   -77,   -77,
      12,   -77,    13,     0,   -77,   -77,   -77,    14,    15,   -77,
     -77,    16,    17,   -77,   -77,   -77,   -77,    18,    19,    20,
     -77,     0,    21,   -77,   -77,   -77,    22,    23,   -77,   -77,
     -77,    24,    25,    26,   -77,   -77,    27,   -77,   -77,   -77,
     -77,    28,     0,    29,   -77,   -77,     0,    30,    31,    32,
       0,    33,    34,    35,     0,   -77,     0,     0,    37,   -77,
      38,   -77,    39,   -77,    40,   -77,   -77,    41,   -77,   -77,
      42,    43,     1,    44,     2,     3,     4,     5,     6,     0,
       7,     8,     9,    10,    11,  -108,  -108,  -108,   305,     0,
      13,     0,  -108,  -108,  -108,    14,    15,  -108,  -108,    16,
      17,  -108,  -108,  -108,  -108,    18,    19,    20,  -108,     0,
      21,     0,     0,  -108,    22,    23,     0,  -108,     0,    24,
      25,    26,     0,     0,    27,     0,     0,     0,     0,    28,
       0,    29,     0,  -108,     0,    30,    31,    32,     0,    33,
      34,    35,    92,     0,    36,     0,    37,  -108,    38,     0,
      39,     0,    40,  -108,  -108,    41,  -108,  -108,    42,    43,
       1,    44,     2,     3,     4,     5,     6,     0,     7,     8,
       9,    10,    11,  -108,  -108,  -108,   305,     0,    13,     0,
    -108,  -108,  -108,    14,    15,  -108,  -108,    16,    17,  -108,
    -108,  -108,  -108,    18,    19,    20,  -108,     0,    21,     0,
       0,  -108,    22,    23,     0,     0,     0,    24,    25,    26,
       0,     0,    27,     0,     0,  -108,     0,    28,     0,    29,
       0,  -108,     0,    30,    31,    32,     0,    33,    34,    35,
      92,     0,    36,     0,    37,  -108,    38,     0,    39,     0,
      40,  -108,  -108,    41,  -108,  -108,    42,    43,     1,    44,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,   -77,   -77,   -77,    12,     0,    13,     0,   -77,   -77,
     -77,    14,   158,   -77,   -77,    16,    17,   -77,   -77,   -77,
     -77,    18,    19,    20,   -77,     0,    21,     0,     0,   -77,
      22,    23,     0,     0,     0,    24,    25,    26,     0,     0,
      27,     0,     0,     0,     0,    28,     0,    29,     0,   -77,
       0,    30,    31,    32,     0,    33,    34,    35,     0,     0,
      36,     0,    37,   -77,    38,     0,    39,     0,    40,   -77,
     -77,    41,   -77,   -77,    42,    43,     1,    44,     2,     3,
       4,     5,     6,     0,     7,     8,     9,    10,    11,     0,
       0,     0,    12,     0,    13,     0,     0,     0,     0,    14,
      15,     0,     0,    16,    17,     0,     0,     0,     0,    18,
      19,    20,     0,     0,    21,     0,   180,     0,    22,    23,
     308,     0,   309,    24,    25,    26,     0,     0,    27,     0,
       0,     0,     0,    28,     0,    29,     0,     0,     0,    30,
      31,    32,     0,    33,    34,    35,     0,   310,    36,     0,
      37,     0,    38,     0,    39,     0,    40,     0,     0,    41,
       0,     0,    42,    43,     1,    44,     2,     3,     4,     5,
       6,     0,     7,     8,     9,    10,    11,     0,     0,     0,
      12,     0,    13,     0,     0,     0,     0,    14,    15,     0,
       0,    16,    17,     0,     0,     0,     0,    18,    19,    20,
       0,     0,    21,     0,   180,     0,    22,    23,     0,     0,
     331,    24,    25,    26,     0,     0,    27,     0,     0,     0,
       0,    28,     0,    29,     0,     0,     0,    30,    31,    32,
       0,    33,    34,    35,     0,   332,    36,     0,    37,     0,
      38,     0,    39,     0,    40,     0,     0,    41,     0,     0,
      42,    43,     1,    44,     2,     3,     4,     5,     6,     0,
       7,     8,   393,    10,    11,     0,     0,     0,    12,     0,
      13,     0,     0,     0,     0,    14,    15,     0,     0,    16,
      17,     0,     0,     0,     0,    18,    19,    20,     0,     0,
      21,     0,   180,     0,    22,    23,     0,     0,     0,    24,
      25,    26,   169,  -197,    27,     0,     0,     0,     0,    28,
       0,    29,     0,     0,     0,    30,    31,    32,     0,    33,
      34,    35,     0,     0,    36,     0,    37,     0,    38,     0,
      39,     0,    40,     0,     0,    41,     0,     0,    42,    43,
       1,    44,     2,     3,     4,     5,     6,     0,     7,     8,
       9,    10,    11,     0,     0,     0,    12,     0,    13,     0,
       0,     0,     0,    14,    15,     0,     0,    16,    17,     0,
       0,     0,     0,    18,    19,    20,     0,     0,    21,     0,
     180,     0,    22,    23,     0,     0,   396,    24,    25,    26,
       0,     0,    27,     0,     0,     0,     0,    28,     0,    29,
       0,     0,     0,    30,    31,    32,     0,    33,    34,    35,
       0,   397,    36,     0,    37,     0,    38,     0,    39,     0,
      40,     0,     0,    41,     0,     0,    42,    43,     1,    44,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,     0,     0,     0,    12,     0,    13,     0,     0,     0,
       0,    14,    15,     0,     0,    16,    17,     0,     0,     0,
       0,    18,    19,    20,     0,     0,    21,     0,   180,     0,
      22,    23,     0,     0,     0,    24,    25,    26,     0,     0,
      27,     0,     0,     0,     0,    28,     0,    29,   181,     0,
       0,    30,    31,    32,     0,    33,    34,    35,     0,     0,
      36,     0,    37,     0,    38,     0,    39,     0,    40,     0,
       0,    41,     0,     0,    42,    43,     1,    44,     2,     3,
       4,     5,     6,     0,     7,     8,     9,    10,    11,     0,
       0,     0,    12,     0,    13,     0,     0,     0,     0,    14,
      15,     0,     0,    16,    17,     0,     0,     0,     0,    18,
      19,    20,     0,     0,    21,     0,   180,     0,    22,    23,
       0,   196,     0,    24,    25,    26,     0,     0,    27,     0,
       0,     0,     0,    28,     0,    29,     0,     0,     0,    30,
      31,    32,     0,    33,    34,    35,     0,     0,    36,     0,
      37,     0,    38,     0,    39,     0,    40,     0,     0,    41,
       0,     0,    42,    43,     1,    44,     2,     3,     4,     5,
       6,     0,     7,     8,     9,    10,    11,     0,     0,     0,
      12,     0,    13,     0,     0,     0,     0,    14,    15,     0,
       0,    16,    17,     0,     0,     0,     0,    18,    19,    20,
       0,     0,    21,     0,   180,     0,    22,    23,     0,     0,
       0,    24,    25,    26,     0,     0,    27,     0,     0,     0,
       0,    28,     0,    29,     0,     0,     0,    30,    31,    32,
       0,    33,    34,    35,     0,   274,    36,     0,    37,     0,
      38,     0,    39,     0,    40,     0,     0,    41,     0,     0,
      42,    43,     1,    44,     2,     3,     4,     5,     6,     0,
       7,     8,     9,    10,    11,     0,     0,     0,    12,     0,
      13,     0,     0,     0,     0,    14,    15,     0,     0,    16,
      17,     0,     0,     0,     0,    18,    19,    20,     0,     0,
      21,     0,   180,     0,    22,    23,     0,     0,     0,    24,
      25,    26,     0,  -198,    27,     0,     0,     0,     0,    28,
       0,    29,     0,     0,     0,    30,    31,    32,     0,    33,
      34,    35,     0,     0,    36,     0,    37,     0,    38,     0,
      39,     0,    40,     0,     0,    41,     0,     0,    42,    43,
       1,    44,     2,     3,     4,     5,     6,     0,     7,     8,
       9,    10,    11,     0,     0,     0,    12,     0,    13,     0,
       0,     0,     0,    14,    15,     0,     0,    16,    17,     0,
       0,     0,     0,    18,    19,    20,     0,     0,    21,     0,
     180,     0,    22,    23,     0,     0,     0,    24,    25,    26,
       0,     0,    27,     0,     0,     0,     0,    28,     0,    29,
       0,     0,     0,    30,    31,    32,     0,    33,    34,    35,
       0,   323,    36,     0,    37,     0,    38,     0,    39,     0,
      40,     0,     0,    41,     0,     0,    42,    43,     1,    44,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,     0,     0,     0,    12,     0,    13,     0,     0,     0,
       0,    14,    15,     0,     0,    16,    17,     0,     0,     0,
       0,    18,    19,    20,     0,     0,    21,     0,   180,     0,
      22,    23,     0,     0,     0,    24,    25,    26,     0,     0,
      27,     0,     0,     0,     0,    28,     0,    29,     0,     0,
       0,    30,    31,    32,     0,    33,    34,    35,     0,   326,
      36,     0,    37,     0,    38,     0,    39,     0,    40,     0,
       0,    41,     0,     0,    42,    43,     1,    44,     2,     3,
       4,     5,     6,     0,     7,     8,     9,    10,    11,     0,
       0,     0,    12,     0,    13,     0,     0,     0,     0,    14,
      15,     0,     0,    16,    17,     0,     0,     0,     0,    18,
      19,    20,     0,     0,    21,     0,     0,     0,    22,    23,
       0,     0,     0,    24,    25,    26,     0,     0,    27,     0,
       0,     0,     0,    28,     0,    29,     0,     0,     0,    30,
      31,    32,     0,    33,    34,    35,   368,     0,    36,   271,
      37,     0,    38,     0,    39,     0,    40,     0,     0,    41,
       0,     0,    42,    43,     1,    44,     2,     3,     4,     5,
       6,     0,     7,     8,     9,    10,    11,     0,     0,     0,
      12,     0,    13,     0,     0,     0,     0,    14,    15,     0,
       0,    16,    17,     0,     0,     0,     0,    18,    19,    20,
       0,     0,    21,     0,     0,     0,    22,    23,     0,     0,
       0,    24,    25,    26,     0,     0,    27,     0,     0,     0,
       0,    28,     0,    29,     0,     0,     0,    30,    31,    32,
       0,    33,    34,    35,   370,     0,    36,  -155,    37,     0,
      38,     0,    39,     0,    40,     0,     0,    41,     0,     0,
      42,    43,     1,    44,     2,     3,     4,     5,     6,     0,
       7,     8,     9,    10,    11,     0,     0,     0,    12,     0,
      13,     0,     0,     0,     0,    14,    15,     0,     0,    16,
      17,     0,     0,     0,     0,    18,    19,    20,     0,     0,
      21,     0,   180,     0,    22,    23,     0,     0,     0,    24,
      25,    26,     0,     0,    27,     0,     0,     0,     0,    28,
       0,    29,     0,     0,     0,    30,    31,    32,     0,    33,
      34,    35,     0,   310,    36,     0,    37,     0,    38,     0,
      39,     0,    40,     0,     0,    41,     0,     0,    42,    43,
       1,    44,     2,     3,     4,     5,     6,     0,     7,     8,
       9,    10,    11,     0,     0,     0,    12,     0,    13,     0,
       0,     0,     0,    14,    15,     0,     0,    16,    17,     0,
       0,     0,     0,    18,    19,    20,     0,     0,    21,     0,
     180,     0,    22,    23,     0,     0,     0,    24,    25,    26,
       0,     0,    27,     0,     0,     0,     0,    28,     0,    29,
       0,     0,     0,    30,    31,    32,     0,    33,    34,    35,
       0,   404,    36,     0,    37,     0,    38,     0,    39,     0,
      40,     0,     0,    41,     0,     0,    42,    43,     1,    44,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,     0,     0,     0,    12,     0,    13,     0,     0,     0,
       0,    14,    15,     0,     0,    16,    17,     0,     0,     0,
       0,    18,    19,    20,     0,     0,    21,     0,   180,     0,
      22,    23,     0,     0,     0,    24,    25,    26,     0,     0,
      27,     0,     0,     0,     0,    28,     0,    29,     0,     0,
       0,    30,    31,    32,     0,    33,    34,    35,     0,   407,
      36,     0,    37,     0,    38,     0,    39,     0,    40,     0,
       0,    41,     0,     0,    42,    43,     1,    44,     2,     3,
       4,     5,     6,     0,     7,     8,     9,    10,    11,     0,
       0,     0,    12,     0,    13,     0,     0,     0,     0,    14,
      15,     0,     0,    16,    17,     0,     0,     0,     0,    18,
      19,    20,     0,     0,    21,     0,   180,     0,    22,    23,
       0,     0,     0,    24,    25,    26,     0,     0,    27,     0,
       0,     0,     0,    28,     0,    29,     0,     0,     0,    30,
      31,    32,     0,    33,    34,    35,     0,   408,    36,     0,
      37,     0,    38,     0,    39,     0,    40,     0,     0,    41,
       0,     0,    42,    43,     1,    44,     2,     3,     4,     5,
       6,     0,     7,     8,     9,    10,    11,     0,     0,     0,
      12,     0,    13,     0,     0,     0,     0,    14,    15,     0,
       0,    16,    17,     0,     0,     0,     0,    18,    19,    20,
       0,     0,    21,     0,   180,     0,    22,    23,     0,     0,
       0,    24,    25,    26,     0,     0,    27,     0,     0,     0,
       0,    28,     0,    29,     0,     0,     0,    30,    31,    32,
       0,    33,    34,    35,     0,   420,    36,     0,    37,     0,
      38,     0,    39,     0,    40,     0,     0,    41,     0,     0,
      42,    43,     1,    44,     2,     3,     4,     5,     6,     0,
       7,     8,     9,    10,    11,     0,     0,     0,    12,     0,
      13,     0,     0,     0,     0,    14,    15,     0,     0,    16,
      17,     0,     0,     0,     0,    18,    19,    20,     0,     0,
      21,     0,   180,     0,    22,    23,     0,     0,     0,    24,
      25,    26,     0,     0,    27,     0,     0,     0,     0,    28,
       0,    29,     0,     0,     0,    30,    31,    32,     0,    33,
      34,    35,     0,   422,    36,     0,    37,     0,    38,     0,
      39,     0,    40,     0,     0,    41,     0,     0,    42,    43,
       1,    44,     2,     3,     4,     5,     6,     0,     7,     8,
       9,    10,    11,     0,     0,     0,    12,     0,    13,     0,
       0,     0,     0,    14,    15,     0,     0,    16,    17,     0,
       0,     0,     0,    18,    19,    20,     0,     0,    21,     0,
     180,     0,    22,    23,     0,     0,     0,    24,    25,    26,
       0,     0,    27,     0,     0,     0,     0,    28,     0,    29,
       0,     0,     0,    30,    31,    32,     0,    33,    34,    35,
       0,   425,    36,     0,    37,     0,    38,     0,    39,     0,
      40,     0,     0,    41,     0,     0,    42,    43,     1,    44,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,     0,     0,     0,    12,     0,    13,     0,     0,     0,
       0,    14,    15,     0,     0,    16,    17,     0,     0,     0,
       0,    18,    19,    20,     0,     0,    21,     0,   180,     0,
      22,    23,     0,     0,     0,    24,    25,    26,     0,     0,
      27,     0,     0,     0,     0,    28,     0,    29,     0,     0,
       0,    30,    31,    32,     0,    33,    34,    35,     0,   431,
      36,     0,    37,     0,    38,     0,    39,     0,    40,     0,
       0,    41,     0,     0,    42,    43,     1,    44,     2,     3,
       4,     5,     6,     0,     7,     8,     9,    10,    11,     0,
       0,     0,    12,     0,    13,     0,     0,     0,     0,    14,
      15,     0,     0,    16,    17,     0,     0,     0,     0,    18,
      19,    20,     0,     0,    21,     0,   180,     0,    22,    23,
       0,     0,     0,    24,    25,    26,     0,     0,    27,     0,
       0,     0,     0,    28,     0,    29,     0,     0,     0,    30,
      31,    32,     0,    33,    34,    35,     0,   432,    36,     0,
      37,     0,    38,     0,    39,     0,    40,     0,     0,    41,
       0,     0,    42,    43,     1,    44,     2,     3,     4,     5,
       6,     0,     7,     8,     9,    10,    11,     0,     0,     0,
      12,     0,    13,     0,     0,     0,     0,    14,    15,     0,
       0,    16,    17,     0,     0,     0,     0,    18,    19,    20,
       0,     0,    21,     0,   180,     0,    22,    23,     0,     0,
       0,    24,    25,    26,     0,     0,    27,     0,     0,     0,
       0,    28,     0,    29,     0,     0,     0,    30,    31,    32,
       0,    33,    34,    35,     0,   433,    36,     0,    37,     0,
      38,     0,    39,     0,    40,     0,     0,    41,     0,     0,
      42,    43,     1,    44,     2,     3,     4,     5,     6,     0,
       7,     8,     9,    10,    11,     0,     0,     0,    12,     0,
      13,     0,     0,     0,     0,    14,    15,     0,     0,    16,
      17,     0,     0,     0,     0,    18,    19,    20,     0,     0,
      21,     0,   180,     0,    22,    23,     0,     0,     0,    24,
      25,    26,     0,     0,    27,     0,     0,     0,     0,    28,
       0,    29,     0,     0,     0,    30,    31,    32,     0,    33,
      34,    35,     0,   434,    36,     0,    37,     0,    38,     0,
      39,     0,    40,     0,     0,    41,     0,     0,    42,    43,
       1,    44,     2,     3,     4,     5,     6,     0,     7,     8,
       9,    10,    11,    67,     0,     0,    12,     0,    13,     0,
       0,     0,     0,    14,    15,     0,     0,    16,    17,     0,
       0,     0,     0,    18,    19,    20,     0,     0,    21,     0,
       0,     0,    22,    23,     0,     0,     0,    24,    25,    26,
       0,     0,    27,     0,     0,     0,     0,    28,     0,    29,
       0,     0,     0,    30,    31,    32,     0,    33,    34,    35,
       0,     0,    36,     0,    37,     0,    38,     0,    39,     0,
      40,     0,     0,    41,     0,     0,    42,    43,     1,    44,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,     0,     0,     0,    12,     0,    13,     0,     0,     0,
       0,    14,    15,     0,     0,    16,    17,     0,     0,     0,
       0,    18,    19,    20,     0,     0,    21,  -171,     0,     0,
      22,    23,     0,     0,     0,    24,    25,    26,     0,     0,
      27,     0,     0,     0,     0,    28,     0,    29,     0,     0,
       0,    30,    31,    32,     0,    33,    34,    35,     0,     0,
      36,     0,    37,     0,    38,     0,    39,     0,    40,     0,
       0,    41,     0,     0,    42,    43,     1,    44,     2,     3,
       4,     5,     6,     0,     7,     8,     9,    10,    11,     0,
       0,     0,    12,   151,    13,     0,     0,     0,     0,    14,
      15,     0,     0,    16,    17,     0,     0,     0,     0,    18,
      19,    20,     0,     0,    21,     0,     0,     0,    22,    23,
       0,     0,     0,    24,    25,    26,     0,     0,    27,     0,
       0,     0,     0,    28,     0,    29,     0,     0,     0,    30,
      31,    32,     0,    33,    34,    35,     0,     0,    36,     0,
      37,     0,    38,     0,    39,     0,    40,     0,     0,    41,
       0,     0,    42,    43,     1,    44,     2,     3,     4,     5,
       6,     0,     7,     8,     9,    10,    11,     0,     0,     0,
      12,   153,    13,     0,     0,     0,     0,    14,    15,     0,
       0,    16,    17,     0,     0,     0,     0,    18,    19,    20,
       0,     0,    21,     0,     0,     0,    22,    23,     0,     0,
       0,    24,    25,    26,     0,     0,    27,     0,     0,     0,
       0,    28,     0,    29,     0,     0,     0,    30,    31,    32,
       0,    33,    34,    35,     0,     0,    36,     0,    37,     0,
      38,     0,    39,     0,    40,     0,     0,    41,     0,     0,
      42,    43,     1,    44,     2,     3,     4,     5,     6,     0,
       7,     8,     9,    10,    11,     0,     0,     0,    12,     0,
      13,     0,     0,     0,     0,    14,    15,     0,     0,    16,
      17,     0,     0,     0,     0,    18,    19,    20,     0,     0,
      21,     0,  -169,     0,    22,    23,     0,     0,     0,    24,
      25,    26,     0,     0,    27,     0,     0,     0,     0,    28,
       0,    29,     0,     0,     0,    30,    31,    32,     0,    33,
      34,    35,     0,     0,    36,     0,    37,     0,    38,     0,
      39,     0,    40,     0,     0,    41,     0,     0,    42,    43,
       1,    44,     2,     3,     4,     5,     6,     0,     7,     8,
       9,    10,    11,     0,     0,     0,    12,     0,    13,     0,
       0,     0,     0,    14,    15,     0,     0,    16,    17,     0,
       0,     0,     0,    18,    19,    20,     0,     0,    21,     0,
       0,     0,    22,    23,     0,     0,     0,    24,    25,    26,
       0,     0,    27,     0,     0,     0,     0,    28,     0,    29,
       0,     0,     0,    30,    31,    32,     0,    33,    34,    35,
       0,     0,    36,  -155,    37,     0,    38,     0,    39,     0,
      40,     0,     0,    41,     0,     0,    42,    43,     1,    44,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,     0,     0,     0,    12,  -171,    13,     0,     0,     0,
       0,    14,    15,     0,     0,    16,    17,     0,     0,     0,
       0,    18,    19,    20,     0,     0,    21,     0,     0,     0,
      22,    23,     0,     0,     0,    24,    25,    26,     0,     0,
      27,     0,     0,     0,     0,    28,     0,    29,     0,     0,
       0,    30,    31,    32,     0,    33,    34,    35,     0,     0,
      36,     0,    37,     0,    38,     0,    39,     0,    40,     0,
       0,    41,     0,     0,    42,    43,     1,    44,     2,     3,
       4,     5,     6,     0,     7,     8,     9,    10,    11,     0,
       0,     0,    12,   198,    13,     0,     0,     0,     0,    14,
      15,     0,     0,    16,    17,     0,     0,     0,     0,    18,
      19,    20,     0,     0,    21,     0,     0,     0,    22,    23,
       0,     0,     0,    24,    25,    26,     0,     0,    27,     0,
       0,     0,     0,    28,     0,    29,     0,     0,     0,    30,
      31,    32,     0,    33,    34,    35,     0,     0,    36,     0,
      37,     0,    38,     0,    39,     0,    40,     0,     0,    41,
       0,     0,    42,    43,     1,    44,     2,     3,     4,     5,
       6,     0,     7,     8,     9,    10,    11,     0,     0,     0,
      12,     0,    13,     0,     0,     0,     0,    14,    15,     0,
       0,    16,    17,     0,     0,     0,     0,    18,    19,    20,
       0,     0,    21,     0,     0,     0,    22,    23,     0,     0,
       0,    24,    25,    26,     0,     0,    27,     0,     0,     0,
       0,    28,     0,    29,     0,     0,     0,    30,    31,    32,
       0,    33,    34,    35,    92,     0,    36,     0,    37,     0,
      38,     0,    39,     0,    40,     0,     0,    41,     0,     0,
      42,    43,     1,    44,     2,     3,     4,     5,     6,   223,
       7,     8,     9,    10,    11,     0,     0,     0,    12,     0,
      13,     0,     0,     0,     0,    14,    15,     0,     0,    16,
      17,     0,     0,     0,     0,    18,    19,    20,     0,     0,
      21,     0,     0,     0,    22,    23,     0,     0,     0,    24,
      25,    26,     0,     0,    27,     0,     0,     0,     0,    28,
       0,    29,     0,     0,     0,    30,    31,    32,     0,    33,
      34,    35,     0,     0,    36,     0,    37,     0,    38,     0,
      39,     0,    40,     0,     0,    41,     0,     0,    42,    43,
       1,    44,     2,     3,     4,     5,     6,     0,     7,     8,
       9,    10,    11,     0,     0,     0,    12,     0,    13,     0,
       0,     0,     0,    14,    15,     0,     0,    16,    17,     0,
       0,     0,     0,    18,    19,    20,     0,     0,    21,     0,
       0,     0,    22,    23,     0,     0,     0,    24,    25,    26,
       0,     0,    27,     0,     0,     0,     0,    28,     0,    29,
       0,     0,     0,    30,    31,    32,     0,    33,    34,    35,
       0,     0,    36,   271,    37,     0,    38,     0,    39,     0,
      40,     0,     0,    41,     0,     0,    42,    43,     1,    44,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,     0,     0,     0,    12,     0,    13,     0,     0,     0,
       0,    14,    15,     0,     0,    16,    17,     0,     0,     0,
       0,    18,    19,    20,     0,     0,    21,     0,     0,     0,
      22,    23,     0,     0,     0,    24,    25,    26,     0,     0,
      27,     0,     0,     0,     0,    28,     0,    29,     0,     0,
       0,    30,    31,    32,     0,    33,   378,    35,    92,     0,
      36,     0,    37,     0,    38,     0,    39,     0,    40,     0,
       0,    41,     0,     0,    42,    43,     1,    44,     2,     3,
       4,     5,     6,     0,     7,     8,     9,    10,    11,     0,
       0,     0,    12,  -169,    13,     0,     0,     0,     0,    14,
      15,     0,     0,    16,    17,     0,     0,     0,     0,    18,
      19,    20,     0,     0,    21,     0,     0,     0,    22,    23,
       0,     0,     0,    24,    25,    26,     0,     0,    27,     0,
       0,     0,     0,    28,     0,    29,     0,     0,     0,    30,
      31,    32,     0,    33,    34,    35,     0,     0,    36,     0,
      37,     0,    38,     0,    39,     0,    40,     0,     0,    41,
       0,     0,    42,    43,     1,    44,     2,     3,     4,     5,
       6,     0,     7,     8,     9,    10,    11,     0,     0,     0,
      12,     0,    13,     0,     0,     0,     0,    14,    15,     0,
       0,    16,    17,     0,     0,     0,     0,    18,    19,    20,
       0,     0,    21,     0,     0,     0,    22,    23,     0,     0,
       0,    24,    25,    26,     0,     0,    27,     0,     0,     0,
       0,    28,     0,    29,     0,     0,     0,    30,    31,    32,
       0,    33,    34,    35,     0,     0,    36,     0,    37,     0,
      38,     0,    39,     0,    40,     0,     0,    41,     0,     0,
      42,    43,     1,    44,     2,     3,     4,     5,     6,     0,
       7,     8,     9,    10,    63,     0,     0,     0,    12,     0,
      13,     0,     0,     0,     0,    14,    15,     0,     0,    16,
      17,     0,     0,     0,     0,    18,    19,    20,     0,     0,
      21,     0,     0,     0,    22,    23,     0,     0,     0,    24,
      25,    26,     0,     0,    27,     0,     0,     0,     0,    28,
       0,    29,     0,     0,     0,    30,    31,    32,     0,    33,
      34,    35,     0,     0,    64,     0,    37,     0,    38,     0,
      39,     0,    40,     0,     0,    41,     0,     0,    42,    43,
       1,    44,     2,     3,     4,     5,     6,     0,     7,     8,
       9,    10,    11,     0,     0,     0,    74,     0,    13,     0,
       0,     0,     0,    14,    15,     0,     0,    16,    17,     0,
       0,     0,     0,    18,    19,    20,     0,     0,    21,     0,
       0,     0,    22,    23,     0,     0,     0,    24,    25,    26,
       0,     0,    27,     0,     0,     0,     0,    28,     0,    29,
       0,     0,     0,    30,    31,    32,     0,    33,    34,    35,
       0,     0,    36,     0,    37,     0,    38,     0,    39,     0,
      40,     0,     0,    41,     0,     0,    42,    43,     1,    44,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,     0,     0,     0,    78,     0,    13,     0,     0,     0,
       0,    14,    15,     0,     0,    16,    17,     0,     0,     0,
       0,    18,    19,    20,     0,     0,    21,     0,     0,     0,
      22,    23,     0,     0,     0,    24,    25,    26,     0,     0,
      27,     0,     0,     0,     0,    28,     0,    29,     0,     0,
       0,    30,    31,    32,     0,    33,    34,    35,     0,     0,
      36,     0,    37,     0,    38,     0,    39,     0,    40,     0,
       0,    41,     0,     0,    42,    43,     1,    44,     2,     3,
       4,     5,     6,     0,     7,     8,     9,    10,    11,     0,
       0,     0,    88,     0,    13,     0,     0,     0,     0,    14,
      15,     0,     0,    16,    17,     0,     0,     0,     0,    18,
      19,    20,     0,     0,    21,     0,     0,     0,    22,    23,
       0,     0,     0,    24,    25,    26,     0,     0,    27,     0,
       0,     0,     0,    28,     0,    29,     0,     0,     0,    30,
      31,    32,     0,    33,    34,    35,     0,     0,    36,     0,
      37,     0,    38,     0,    39,     0,    40,     0,     0,    41,
       0,     0,    42,    43,     1,    44,     2,   167,     4,     5,
       6,     0,     7,     8,     9,    10,    11,     0,     0,     0,
      12,     0,    13,     0,     0,     0,     0,    14,    15,     0,
       0,    16,    17,     0,     0,     0,     0,    18,    19,    20,
       0,     0,    21,     0,     0,     0,    22,    23,     0,     0,
       0,    24,    25,    26,     0,     0,    27,     0,     0,     0,
       0,    28,     0,    29,     0,     0,     0,    30,    31,    32,
       0,    33,    34,    35,     0,     0,    36,     0,    37,     0,
      38,     0,    39,     0,    40,     0,     0,    41,     0,     0,
      42,    43,     1,    44,     2,     3,     4,     5,     6,     0,
       7,     8,     9,    10,    11,     0,     0,     0,    12,     0,
      13,     0,     0,     0,     0,    14,    15,     0,     0,    16,
      17,     0,     0,     0,     0,    18,    19,    20,     0,     0,
     221,     0,     0,     0,    22,    23,     0,     0,     0,    24,
      25,    26,     0,     0,    27,     0,     0,     0,     0,    28,
       0,    29,     0,     0,     0,    30,    31,    32,     0,    33,
      34,    35,     0,     0,    36,     0,    37,     0,    38,     0,
      39,     0,    40,     0,     0,    41,     0,     0,    42,    43,
       1,    44,     2,     3,     4,     5,     6,     0,     7,     8,
       9,    10,    11,     0,     0,     0,    12,     0,    13,     0,
       0,     0,     0,    14,    15,     0,     0,    16,    17,     0,
       0,     0,     0,    18,    19,    20,     0,     0,   337,     0,
       0,     0,    22,    23,     0,     0,     0,    24,    25,    26,
       0,     0,    27,     0,     0,     0,     0,    28,     0,    29,
       0,     0,     0,    30,    31,    32,     0,    33,    34,    35,
       0,     0,    36,     0,    37,     0,    38,     0,    39,     0,
      40,     0,     0,    41,     0,     0,    42,    43,     1,    44,
       2,     3,     4,     5,     6,   -74,     7,     8,     9,    10,
      11,   -74,   -74,   -74,   107,   -74,    13,     0,   -74,   -74,
     -74,    14,    15,   -74,   -74,    16,    17,   -74,   -74,   -74,
     -74,    18,    19,    20,   -74,     0,    21,   -74,   -74,   -74,
      22,    23,   -74,   -74,   -74,    24,    25,    26,   -74,   -74,
     -74,   -74,   -74,   -74,   -74,    28,     0,    29,   -74,   -74,
       0,    30,    31,    32,     0,    33,    34,    35,     0,   -74,
     -74,     0,    37,   -74,    38,   -74,    39,   -74,    40,   -74,
     -74,    41,   -74,   -74,    42,    43,   110,     0,     0,     0,
     111,     0,   112,   113,   114,   115,     0,   116,     0,   117,
     118,   119,     0,     0,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,     0,     0,     0,   132,
     133,     0,     0,     0,     0,     0,   168,   111,     0,   112,
     113,   114,   115,     0,   116,     0,   117,   118,   119,     0,
     134,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,     0,   135,     0,     0,   133,     0,     0,
     136,   137,     0,   138,   139,   111,   169,   112,   113,   114,
     115,     0,   116,     0,   117,   118,   119,   134,     0,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   135,     0,   335,     0,   133,     0,   136,   137,     0,
     138,   139,   111,     0,     0,     0,   161,   115,     0,   116,
       0,   117,   118,   119,     0,   134,   120,   121,   122,   123,
     124,   125,   126,     0,   128,   129,   130,   131,     0,   135,
       0,     0,     0,     0,     0,   136,   137,     0,   138,   139,
     111,     0,   112,   113,   114,   115,     0,   116,     0,   117,
     118,   119,   134,     0,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   135,     0,   383,     0,
     133,     0,   384,     0,     0,     0,   139,   111,     0,   112,
     113,   114,   115,     0,   116,     0,   117,   118,   119,     0,
     134,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,     0,   135,   409,     0,   133,     0,     0,
     136,   137,     0,   138,   139,   111,     0,     0,   161,   114,
     115,     0,   116,     0,   117,   118,   119,   134,     0,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   135,     0,     0,     0,   133,     0,   136,   137,     0,
     138,   139,   111,     0,   112,   113,   114,   115,   156,   116,
       0,   117,   118,   119,     0,   134,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,     0,   135,
       0,     0,   133,   330,     0,     0,     0,     0,   138,   139,
     111,     0,   112,   113,   114,   115,     0,   116,     0,   117,
     118,   119,   134,   159,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   135,     0,     0,     0,
     133,     0,   136,   137,     0,   138,   139,   111,     0,   112,
     113,   114,   115,     0,   116,     0,   117,   118,   119,     0,
     134,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,     0,   135,     0,     0,   133,     0,     0,
     136,   137,     0,   138,   139,     0,     0,     0,   161,     0,
       0,   111,     0,     0,     0,     0,   115,   134,   116,     0,
     117,   118,   119,     0,     0,   120,     0,     0,   123,     0,
       0,   135,     0,   128,   129,     0,   131,   136,   137,     0,
     138,   139,   111,     0,   112,   113,   114,   115,     0,   116,
       0,   117,   118,   119,     0,     0,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,     0,     0,
       0,     0,   133,     0,     0,   135,   164,     0,     0,   111,
       0,   112,   113,   114,   115,   139,   116,     0,   117,   118,
     119,     0,   134,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,     0,   135,     0,     0,   133,
       0,     0,   136,   137,     0,   138,   139,   111,     0,   112,
     113,   114,   115,   179,   116,     0,   117,   118,   119,   134,
       0,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   135,     0,     0,     0,   133,     0,   136,
     137,     0,   138,   139,   111,     0,   112,   113,   114,   115,
     240,   116,     0,   117,   118,   119,     0,   134,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
       0,   135,     0,   194,   133,     0,     0,   136,   137,     0,
     138,   139,   111,     0,   112,   113,   114,   115,   241,   116,
       0,   117,   118,   119,   134,     0,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   135,     0,
       0,     0,   133,     0,   136,   137,     0,   138,   139,   111,
       0,   112,   113,   114,   115,     0,   116,     0,   117,   118,
     119,     0,   134,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,     0,   135,   242,     0,   133,
       0,     0,   136,   137,     0,   138,   139,   111,     0,   112,
     113,   114,   115,   252,   116,     0,   117,   118,   119,   134,
       0,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   135,     0,     0,     0,   133,     0,   136,
     137,     0,   138,   139,   111,     0,   112,   113,   114,   115,
     255,   116,     0,   117,   118,   119,     0,   134,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
       0,   135,     0,     0,   133,     0,     0,   136,   137,     0,
     138,   139,   111,     0,   112,   113,   114,   115,   256,   116,
       0,   117,   118,   119,   134,     0,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   135,     0,
       0,     0,   133,     0,   136,   137,     0,   138,   139,   111,
       0,   112,   113,   114,   115,   261,   116,     0,   117,   118,
     119,     0,   134,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,     0,   135,     0,     0,   133,
       0,     0,   136,   137,     0,   138,   139,   111,     0,   112,
     113,   114,   115,   268,   116,     0,   117,   118,   119,   134,
       0,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   135,     0,     0,     0,   133,     0,   136,
     137,     0,   138,   139,   111,     0,   112,   113,   114,   115,
     276,   116,     0,   117,   118,   119,     0,   134,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
       0,   135,     0,     0,   133,     0,     0,   136,   137,     0,
     138,   139,   111,     0,   112,   113,   114,   115,   279,   116,
       0,   117,   118,   119,   134,     0,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   135,     0,
       0,     0,   133,     0,   136,   137,     0,   138,   139,   111,
       0,   112,   113,   114,   115,   280,   116,     0,   117,   118,
     119,     0,   134,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,     0,   135,     0,     0,   133,
       0,     0,   136,   137,     0,   138,   139,   111,     0,   112,
     113,   114,   115,     0,   116,     0,   117,   118,   119,   134,
       0,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   135,     0,   281,     0,   133,     0,   136,
     137,     0,   138,   139,   111,     0,   112,   113,   114,   115,
     284,   116,     0,   117,   118,   119,     0,   134,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
       0,   135,     0,     0,   133,     0,     0,   136,   137,     0,
     138,   139,   111,     0,   112,   113,   114,   115,     0,   116,
       0,   117,   118,   119,   134,     0,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   135,     0,
     291,     0,   133,     0,   136,   137,     0,   138,   139,   111,
       0,   112,   113,   114,   115,   295,   116,     0,   117,   118,
     119,     0,   134,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,     0,   135,     0,     0,   133,
       0,     0,   136,   137,     0,   138,   139,   111,     0,   112,
     113,   114,   115,     0,   116,     0,   117,   118,   119,   134,
     296,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   135,     0,     0,     0,   133,     0,   136,
     137,     0,   138,   139,   111,     0,   112,   113,   114,   115,
       0,   116,     0,   117,   118,   119,     0,   134,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
       0,   135,     0,     0,   133,     0,     0,   136,   137,     0,
     138,   139,   111,     0,   112,   113,   114,   115,   319,   116,
       0,   117,   118,   119,   134,     0,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   135,     0,
       0,     0,   133,     0,   136,   137,     0,   138,   139,     0,
       0,     0,     0,     0,     0,     0,   111,     0,     0,     0,
       0,   115,   134,   116,     0,   117,   118,   119,     0,     0,
     120,     0,   324,     0,     0,     0,   135,     0,   128,   129,
       0,   131,   136,   137,     0,   138,   139,   111,     0,   112,
     113,   114,   115,     0,   116,     0,   117,   118,   119,     0,
       0,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,     0,     0,     0,   351,   133,     0,     0,
     135,     0,     0,     0,   111,     0,   112,   113,   114,   115,
     139,   116,     0,   117,   118,   119,     0,   134,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
       0,   135,     0,     0,   133,     0,     0,   136,   137,     0,
     138,   139,     0,     0,     0,     0,     0,     0,     0,   363,
       0,     0,     0,     0,   134,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   135,     0,
       0,     0,     0,     0,   136,   137,     0,   138,   139,   111,
       0,   112,   113,   114,   115,     0,   116,     0,   117,   118,
     119,     0,     0,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,     0,     0,     0,   365,   133,
       0,     0,     0,     0,     0,     0,   111,     0,   112,   113,
     114,   115,   367,   116,     0,   117,   118,   119,     0,   134,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,     0,   135,     0,     0,   133,     0,     0,   136,
     137,     0,   138,   139,   111,     0,   112,   113,   114,   115,
       0,   116,     0,   117,   118,   119,   134,     0,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     135,     0,   377,     0,   133,     0,   136,   137,     0,   138,
     139,     0,   111,     0,   112,   113,   114,   115,     0,   116,
       0,   117,   118,   119,   134,     0,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   135,     0,
       0,     0,   133,     0,   136,   137,   388,   138,   139,   111,
       0,   112,   113,   114,   115,     0,   116,     0,   117,   118,
     119,     0,   134,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,     0,   135,     0,   413,   133,
       0,     0,   136,   137,     0,   138,   139,   111,     0,   112,
     113,   114,   115,     0,   116,     0,   117,   118,   119,   134,
       0,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   135,     0,   426,     0,   133,     0,   136,
     137,     0,   138,   139,   111,     0,   112,   113,   114,   115,
       0,   116,     0,   117,   118,   119,     0,   134,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
       0,   135,     0,   436,   133,     0,     0,   136,   137,     0,
     138,   139,   111,     0,   112,   113,   114,   115,     0,   116,
       0,   117,   118,   119,   134,     0,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   135,     0,
       0,     0,   133,     0,   136,   137,     0,   138,   139,   111,
       0,   195,   113,   114,   115,     0,   116,     0,   117,   118,
     119,     0,   134,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,     0,   135,     0,     0,   133,
       0,     0,   136,   137,     0,   138,   139,   111,     0,   112,
     113,   114,   115,     0,   116,     0,   117,   118,   119,   134,
       0,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   135,     0,     0,     0,   133,     0,   136,
     137,     0,   138,   139,   111,     0,     0,   113,   114,   115,
       0,   116,     0,   117,   118,   119,     0,   134,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
       0,   135,     0,     0,   133,     0,     0,     0,  -199,     0,
     138,   139,   111,     0,   112,   113,   114,   115,     0,   116,
       0,   117,   118,   119,   134,     0,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   135,     0,
       0,     0,   133,     0,   136,   137,     0,   138,   139,   111,
       0,     0,   113,   114,   115,     0,   116,     0,   117,   118,
     119,     0,   134,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,     0,   135,     0,     0,   133,
       0,     0,     0,     0,   111,   138,   139,     0,   114,   115,
       0,   116,     0,   117,   118,   119,     0,     0,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
       0,     0,     0,   135,   133,     0,     0,     0,     0,   136,
     137,   111,   138,   139,     0,   114,   115,     0,   116,     0,
     117,   118,   119,     0,   134,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,     0,   135,     0,
       0,   133,     0,     0,     0,     0,     0,   138,   139,   111,
       0,     0,     0,   114,   115,     0,   116,     0,   117,   118,
     119,   134,     0,   120,   121,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   135,     0,     0,     0,   133,
       0,     0,     0,     0,  -199,   139,   111,     0,     0,     0,
    -199,   115,     0,   116,     0,   117,   118,   119,     0,   134,
     120,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,     0,   135,     0,   111,     0,     0,     0,     0,
     115,     0,   116,   139,   117,   118,   119,     0,     0,   120,
     121,   122,   123,   124,   125,   126,   134,   128,   129,   130,
     131,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     135,     0,   111,     0,     0,     0,     0,   115,     0,   116,
     139,   117,   118,   119,     0,  -199,   120,   121,   122,   123,
     124,   125,   126,   111,   128,   129,     0,   131,   115,   135,
     116,     0,   117,   118,   119,     0,     0,   120,     0,   139,
     123,   124,   125,   126,     0,   128,   129,     0,   131,     0,
     111,     0,     0,     0,     0,   115,     0,   116,     0,   117,
     288,   119,     0,     0,   120,     0,   135,   123,     0,   125,
     126,     0,   128,   129,     0,   131,   139,     0,     0,   111,
       0,     0,     0,     0,   115,     0,   116,   135,   117,   118,
     119,     0,     0,   120,     0,     0,   123,   139,   125,  -199,
       0,   128,   129,     0,   131,     0,   111,     0,     0,     0,
       0,   115,     0,     0,   135,   117,   118,   119,     0,     0,
     120,     0,   111,     0,   139,     0,     0,   115,   128,   129,
       0,   131,   118,   119,     0,     0,   120,     0,     0,     0,
       0,   111,     0,   135,   128,   129,   115,   131,     0,     0,
       0,  -199,   119,   139,     0,   120,     0,     0,     0,     0,
       0,     0,     0,   128,   129,     0,   131,     0,     0,     0,
     135,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     139,     0,     0,     0,     0,     0,   135,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   139,     0,     0,     0,
       0,     0,     0,     0,     0,   135,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   139
};

static const yytype_int16 yycheck[] =
{
       0,    14,   262,    23,   248,    17,    43,    95,     3,     4,
      10,    15,    12,    13,     4,    15,    16,    17,    13,   307,
      20,    21,    17,    23,     4,    62,    26,   115,    28,    15,
      10,     4,     5,    13,    15,    25,    36,    10,    38,    39,
      40,    41,    28,     4,    44,    25,    14,    28,    43,    62,
      62,    17,   140,    27,    18,    55,    56,    57,    58,   303,
      34,    42,    43,   351,    64,    69,    86,    67,    54,    17,
      43,    37,    17,    41,    74,    17,    76,    77,    78,    43,
      15,    41,    17,     4,    84,    27,     4,   277,    88,    84,
      17,    91,    37,    43,    94,    95,    56,    97,    17,    99,
     100,   361,   190,    42,    71,   105,   106,   107,    88,    17,
      37,    17,   112,   113,   114,   115,   116,   117,   118,   119,
      17,   121,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,    50,    51,   137,   138,   329,
     140,   141,     0,    17,    17,    17,    17,   121,   148,    23,
     340,     4,    26,    92,    40,   243,   130,   114,   158,    43,
      34,    35,   136,    37,    37,    67,    62,    17,    62,     3,
      17,     3,    52,   173,   174,   175,   176,    41,    55,    17,
     370,   181,    18,    18,    43,   185,    47,   161,   378,    18,
     190,     3,     4,    71,    56,   195,     3,     4,    10,   199,
      18,    13,    76,    10,   292,    17,    13,   164,    23,    18,
      17,    41,    86,    18,    47,    40,    18,   305,     3,     3,
      42,   221,    17,    72,    18,   164,    42,    40,    23,   132,
     169,    43,   271,   351,   414,   355,    43,   428,   360,    34,
     179,    -1,    37,   243,    -1,   184,    -1,    -1,    -1,    -1,
      -1,    -1,   252,   253,    -1,    -1,    -1,   196,    -1,   259,
      -1,    -1,   262,    -1,    -1,    -1,    -1,   267,   268,   269,
      -1,    -1,    84,   273,   248,   275,    88,    84,    -1,    -1,
      -1,    76,    -1,   283,    -1,    -1,    -1,   287,   288,   289,
      -1,    86,   292,   293,    -1,   252,    -1,   271,   298,   299,
     300,   301,    -1,    -1,   304,   305,     3,     4,   308,     6,
      -1,   268,    -1,    10,    -1,    -1,    13,    14,    -1,    -1,
      -1,    -1,    19,    -1,    21,    -1,    -1,    12,    -1,   303,
      -1,    -1,    17,    -1,    -1,    -1,    -1,   337,    23,   339,
      -1,    26,    -1,    -1,    -1,    -1,    43,    -1,    -1,    34,
      35,    -1,    37,   353,    -1,   355,    -1,   314,    -1,    -1,
     360,   361,   362,   363,    -1,    -1,    -1,    -1,    -1,   369,
      -1,    -1,   372,   373,   313,    -1,    -1,   316,    -1,    -1,
     319,    -1,   339,    -1,   384,    82,    -1,    84,    -1,   328,
     329,    76,   331,    -1,    -1,    -1,   353,    -1,    -1,    -1,
      -1,    86,    -1,   403,   378,    -1,   406,    -1,    -1,    -1,
     367,   411,    -1,    -1,   414,    -1,    -1,   417,   418,   419,
      -1,   421,    -1,    -1,   424,    -1,    -1,    -1,    -1,   368,
      -1,    -1,   371,    -1,   391,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   381,    -1,    -1,    -1,    -1,    -1,    -1,   388,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   396,   415,    -1,
     417,    -1,   401,    -1,    -1,    -1,   405,    -1,    -1,    -1,
       1,   410,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    -1,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    -1,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    -1,    60,
      61,    62,    -1,    64,    65,    66,    -1,    68,    69,    70,
      -1,    72,    73,    -1,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,     1,    90,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    -1,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    -1,    60,    61,    62,
      -1,    64,    65,    66,    -1,    68,    69,    70,    -1,    72,
      73,    -1,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,     1,    90,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    -1,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    -1,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    -1,    60,    61,    62,    -1,    64,
      65,    66,    -1,    68,    69,    70,    -1,    72,    73,    -1,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,     1,    90,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    -1,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    -1,    60,    61,    62,    -1,    64,    65,    66,
      -1,    68,    69,    70,    -1,    72,    -1,    -1,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,    88,     1,    90,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    -1,
      19,    -1,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    -1,
      39,    -1,    -1,    42,    43,    44,    -1,    46,    -1,    48,
      49,    50,    -1,    -1,    53,    -1,    -1,    -1,    -1,    58,
      -1,    60,    -1,    62,    -1,    64,    65,    66,    -1,    68,
      69,    70,    71,    -1,    73,    -1,    75,    76,    77,    -1,
      79,    -1,    81,    82,    83,    84,    85,    86,    87,    88,
       1,    90,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    -1,    19,    -1,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    -1,    39,    -1,
      -1,    42,    43,    44,    -1,    -1,    -1,    48,    49,    50,
      -1,    -1,    53,    -1,    -1,    56,    -1,    58,    -1,    60,
      -1,    62,    -1,    64,    65,    66,    -1,    68,    69,    70,
      71,    -1,    73,    -1,    75,    76,    77,    -1,    79,    -1,
      81,    82,    83,    84,    85,    86,    87,    88,     1,    90,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    -1,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    -1,    39,    -1,    -1,    42,
      43,    44,    -1,    -1,    -1,    48,    49,    50,    -1,    -1,
      53,    -1,    -1,    -1,    -1,    58,    -1,    60,    -1,    62,
      -1,    64,    65,    66,    -1,    68,    69,    70,    -1,    -1,
      73,    -1,    75,    76,    77,    -1,    79,    -1,    81,    82,
      83,    84,    85,    86,    87,    88,     1,    90,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    17,    -1,    19,    -1,    -1,    -1,    -1,    24,
      25,    -1,    -1,    28,    29,    -1,    -1,    -1,    -1,    34,
      35,    36,    -1,    -1,    39,    -1,    41,    -1,    43,    44,
      45,    -1,    47,    48,    49,    50,    -1,    -1,    53,    -1,
      -1,    -1,    -1,    58,    -1,    60,    -1,    -1,    -1,    64,
      65,    66,    -1,    68,    69,    70,    -1,    72,    73,    -1,
      75,    -1,    77,    -1,    79,    -1,    81,    -1,    -1,    84,
      -1,    -1,    87,    88,     1,    90,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    -1,    19,    -1,    -1,    -1,    -1,    24,    25,    -1,
      -1,    28,    29,    -1,    -1,    -1,    -1,    34,    35,    36,
      -1,    -1,    39,    -1,    41,    -1,    43,    44,    -1,    -1,
      47,    48,    49,    50,    -1,    -1,    53,    -1,    -1,    -1,
      -1,    58,    -1,    60,    -1,    -1,    -1,    64,    65,    66,
      -1,    68,    69,    70,    -1,    72,    73,    -1,    75,    -1,
      77,    -1,    79,    -1,    81,    -1,    -1,    84,    -1,    -1,
      87,    88,     1,    90,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,
      19,    -1,    -1,    -1,    -1,    24,    25,    -1,    -1,    28,
      29,    -1,    -1,    -1,    -1,    34,    35,    36,    -1,    -1,
      39,    -1,    41,    -1,    43,    44,    -1,    -1,    -1,    48,
      49,    50,    51,    52,    53,    -1,    -1,    -1,    -1,    58,
      -1,    60,    -1,    -1,    -1,    64,    65,    66,    -1,    68,
      69,    70,    -1,    -1,    73,    -1,    75,    -1,    77,    -1,
      79,    -1,    81,    -1,    -1,    84,    -1,    -1,    87,    88,
       1,    90,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,    -1,
      -1,    -1,    -1,    24,    25,    -1,    -1,    28,    29,    -1,
      -1,    -1,    -1,    34,    35,    36,    -1,    -1,    39,    -1,
      41,    -1,    43,    44,    -1,    -1,    47,    48,    49,    50,
      -1,    -1,    53,    -1,    -1,    -1,    -1,    58,    -1,    60,
      -1,    -1,    -1,    64,    65,    66,    -1,    68,    69,    70,
      -1,    72,    73,    -1,    75,    -1,    77,    -1,    79,    -1,
      81,    -1,    -1,    84,    -1,    -1,    87,    88,     1,    90,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    17,    -1,    19,    -1,    -1,    -1,
      -1,    24,    25,    -1,    -1,    28,    29,    -1,    -1,    -1,
      -1,    34,    35,    36,    -1,    -1,    39,    -1,    41,    -1,
      43,    44,    -1,    -1,    -1,    48,    49,    50,    -1,    -1,
      53,    -1,    -1,    -1,    -1,    58,    -1,    60,    61,    -1,
      -1,    64,    65,    66,    -1,    68,    69,    70,    -1,    -1,
      73,    -1,    75,    -1,    77,    -1,    79,    -1,    81,    -1,
      -1,    84,    -1,    -1,    87,    88,     1,    90,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    17,    -1,    19,    -1,    -1,    -1,    -1,    24,
      25,    -1,    -1,    28,    29,    -1,    -1,    -1,    -1,    34,
      35,    36,    -1,    -1,    39,    -1,    41,    -1,    43,    44,
      -1,    46,    -1,    48,    49,    50,    -1,    -1,    53,    -1,
      -1,    -1,    -1,    58,    -1,    60,    -1,    -1,    -1,    64,
      65,    66,    -1,    68,    69,    70,    -1,    -1,    73,    -1,
      75,    -1,    77,    -1,    79,    -1,    81,    -1,    -1,    84,
      -1,    -1,    87,    88,     1,    90,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    -1,    19,    -1,    -1,    -1,    -1,    24,    25,    -1,
      -1,    28,    29,    -1,    -1,    -1,    -1,    34,    35,    36,
      -1,    -1,    39,    -1,    41,    -1,    43,    44,    -1,    -1,
      -1,    48,    49,    50,    -1,    -1,    53,    -1,    -1,    -1,
      -1,    58,    -1,    60,    -1,    -1,    -1,    64,    65,    66,
      -1,    68,    69,    70,    -1,    72,    73,    -1,    75,    -1,
      77,    -1,    79,    -1,    81,    -1,    -1,    84,    -1,    -1,
      87,    88,     1,    90,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,
      19,    -1,    -1,    -1,    -1,    24,    25,    -1,    -1,    28,
      29,    -1,    -1,    -1,    -1,    34,    35,    36,    -1,    -1,
      39,    -1,    41,    -1,    43,    44,    -1,    -1,    -1,    48,
      49,    50,    -1,    52,    53,    -1,    -1,    -1,    -1,    58,
      -1,    60,    -1,    -1,    -1,    64,    65,    66,    -1,    68,
      69,    70,    -1,    -1,    73,    -1,    75,    -1,    77,    -1,
      79,    -1,    81,    -1,    -1,    84,    -1,    -1,    87,    88,
       1,    90,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,    -1,
      -1,    -1,    -1,    24,    25,    -1,    -1,    28,    29,    -1,
      -1,    -1,    -1,    34,    35,    36,    -1,    -1,    39,    -1,
      41,    -1,    43,    44,    -1,    -1,    -1,    48,    49,    50,
      -1,    -1,    53,    -1,    -1,    -1,    -1,    58,    -1,    60,
      -1,    -1,    -1,    64,    65,    66,    -1,    68,    69,    70,
      -1,    72,    73,    -1,    75,    -1,    77,    -1,    79,    -1,
      81,    -1,    -1,    84,    -1,    -1,    87,    88,     1,    90,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    17,    -1,    19,    -1,    -1,    -1,
      -1,    24,    25,    -1,    -1,    28,    29,    -1,    -1,    -1,
      -1,    34,    35,    36,    -1,    -1,    39,    -1,    41,    -1,
      43,    44,    -1,    -1,    -1,    48,    49,    50,    -1,    -1,
      53,    -1,    -1,    -1,    -1,    58,    -1,    60,    -1,    -1,
      -1,    64,    65,    66,    -1,    68,    69,    70,    -1,    72,
      73,    -1,    75,    -1,    77,    -1,    79,    -1,    81,    -1,
      -1,    84,    -1,    -1,    87,    88,     1,    90,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    17,    -1,    19,    -1,    -1,    -1,    -1,    24,
      25,    -1,    -1,    28,    29,    -1,    -1,    -1,    -1,    34,
      35,    36,    -1,    -1,    39,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    48,    49,    50,    -1,    -1,    53,    -1,
      -1,    -1,    -1,    58,    -1,    60,    -1,    -1,    -1,    64,
      65,    66,    -1,    68,    69,    70,    71,    -1,    73,    74,
      75,    -1,    77,    -1,    79,    -1,    81,    -1,    -1,    84,
      -1,    -1,    87,    88,     1,    90,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    -1,    19,    -1,    -1,    -1,    -1,    24,    25,    -1,
      -1,    28,    29,    -1,    -1,    -1,    -1,    34,    35,    36,
      -1,    -1,    39,    -1,    -1,    -1,    43,    44,    -1,    -1,
      -1,    48,    49,    50,    -1,    -1,    53,    -1,    -1,    -1,
      -1,    58,    -1,    60,    -1,    -1,    -1,    64,    65,    66,
      -1,    68,    69,    70,    71,    -1,    73,    74,    75,    -1,
      77,    -1,    79,    -1,    81,    -1,    -1,    84,    -1,    -1,
      87,    88,     1,    90,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,
      19,    -1,    -1,    -1,    -1,    24,    25,    -1,    -1,    28,
      29,    -1,    -1,    -1,    -1,    34,    35,    36,    -1,    -1,
      39,    -1,    41,    -1,    43,    44,    -1,    -1,    -1,    48,
      49,    50,    -1,    -1,    53,    -1,    -1,    -1,    -1,    58,
      -1,    60,    -1,    -1,    -1,    64,    65,    66,    -1,    68,
      69,    70,    -1,    72,    73,    -1,    75,    -1,    77,    -1,
      79,    -1,    81,    -1,    -1,    84,    -1,    -1,    87,    88,
       1,    90,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,    -1,
      -1,    -1,    -1,    24,    25,    -1,    -1,    28,    29,    -1,
      -1,    -1,    -1,    34,    35,    36,    -1,    -1,    39,    -1,
      41,    -1,    43,    44,    -1,    -1,    -1,    48,    49,    50,
      -1,    -1,    53,    -1,    -1,    -1,    -1,    58,    -1,    60,
      -1,    -1,    -1,    64,    65,    66,    -1,    68,    69,    70,
      -1,    72,    73,    -1,    75,    -1,    77,    -1,    79,    -1,
      81,    -1,    -1,    84,    -1,    -1,    87,    88,     1,    90,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    17,    -1,    19,    -1,    -1,    -1,
      -1,    24,    25,    -1,    -1,    28,    29,    -1,    -1,    -1,
      -1,    34,    35,    36,    -1,    -1,    39,    -1,    41,    -1,
      43,    44,    -1,    -1,    -1,    48,    49,    50,    -1,    -1,
      53,    -1,    -1,    -1,    -1,    58,    -1,    60,    -1,    -1,
      -1,    64,    65,    66,    -1,    68,    69,    70,    -1,    72,
      73,    -1,    75,    -1,    77,    -1,    79,    -1,    81,    -1,
      -1,    84,    -1,    -1,    87,    88,     1,    90,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    17,    -1,    19,    -1,    -1,    -1,    -1,    24,
      25,    -1,    -1,    28,    29,    -1,    -1,    -1,    -1,    34,
      35,    36,    -1,    -1,    39,    -1,    41,    -1,    43,    44,
      -1,    -1,    -1,    48,    49,    50,    -1,    -1,    53,    -1,
      -1,    -1,    -1,    58,    -1,    60,    -1,    -1,    -1,    64,
      65,    66,    -1,    68,    69,    70,    -1,    72,    73,    -1,
      75,    -1,    77,    -1,    79,    -1,    81,    -1,    -1,    84,
      -1,    -1,    87,    88,     1,    90,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    -1,    19,    -1,    -1,    -1,    -1,    24,    25,    -1,
      -1,    28,    29,    -1,    -1,    -1,    -1,    34,    35,    36,
      -1,    -1,    39,    -1,    41,    -1,    43,    44,    -1,    -1,
      -1,    48,    49,    50,    -1,    -1,    53,    -1,    -1,    -1,
      -1,    58,    -1,    60,    -1,    -1,    -1,    64,    65,    66,
      -1,    68,    69,    70,    -1,    72,    73,    -1,    75,    -1,
      77,    -1,    79,    -1,    81,    -1,    -1,    84,    -1,    -1,
      87,    88,     1,    90,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,
      19,    -1,    -1,    -1,    -1,    24,    25,    -1,    -1,    28,
      29,    -1,    -1,    -1,    -1,    34,    35,    36,    -1,    -1,
      39,    -1,    41,    -1,    43,    44,    -1,    -1,    -1,    48,
      49,    50,    -1,    -1,    53,    -1,    -1,    -1,    -1,    58,
      -1,    60,    -1,    -1,    -1,    64,    65,    66,    -1,    68,
      69,    70,    -1,    72,    73,    -1,    75,    -1,    77,    -1,
      79,    -1,    81,    -1,    -1,    84,    -1,    -1,    87,    88,
       1,    90,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,    -1,
      -1,    -1,    -1,    24,    25,    -1,    -1,    28,    29,    -1,
      -1,    -1,    -1,    34,    35,    36,    -1,    -1,    39,    -1,
      41,    -1,    43,    44,    -1,    -1,    -1,    48,    49,    50,
      -1,    -1,    53,    -1,    -1,    -1,    -1,    58,    -1,    60,
      -1,    -1,    -1,    64,    65,    66,    -1,    68,    69,    70,
      -1,    72,    73,    -1,    75,    -1,    77,    -1,    79,    -1,
      81,    -1,    -1,    84,    -1,    -1,    87,    88,     1,    90,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    17,    -1,    19,    -1,    -1,    -1,
      -1,    24,    25,    -1,    -1,    28,    29,    -1,    -1,    -1,
      -1,    34,    35,    36,    -1,    -1,    39,    -1,    41,    -1,
      43,    44,    -1,    -1,    -1,    48,    49,    50,    -1,    -1,
      53,    -1,    -1,    -1,    -1,    58,    -1,    60,    -1,    -1,
      -1,    64,    65,    66,    -1,    68,    69,    70,    -1,    72,
      73,    -1,    75,    -1,    77,    -1,    79,    -1,    81,    -1,
      -1,    84,    -1,    -1,    87,    88,     1,    90,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    17,    -1,    19,    -1,    -1,    -1,    -1,    24,
      25,    -1,    -1,    28,    29,    -1,    -1,    -1,    -1,    34,
      35,    36,    -1,    -1,    39,    -1,    41,    -1,    43,    44,
      -1,    -1,    -1,    48,    49,    50,    -1,    -1,    53,    -1,
      -1,    -1,    -1,    58,    -1,    60,    -1,    -1,    -1,    64,
      65,    66,    -1,    68,    69,    70,    -1,    72,    73,    -1,
      75,    -1,    77,    -1,    79,    -1,    81,    -1,    -1,    84,
      -1,    -1,    87,    88,     1,    90,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    -1,    19,    -1,    -1,    -1,    -1,    24,    25,    -1,
      -1,    28,    29,    -1,    -1,    -1,    -1,    34,    35,    36,
      -1,    -1,    39,    -1,    41,    -1,    43,    44,    -1,    -1,
      -1,    48,    49,    50,    -1,    -1,    53,    -1,    -1,    -1,
      -1,    58,    -1,    60,    -1,    -1,    -1,    64,    65,    66,
      -1,    68,    69,    70,    -1,    72,    73,    -1,    75,    -1,
      77,    -1,    79,    -1,    81,    -1,    -1,    84,    -1,    -1,
      87,    88,     1,    90,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,
      19,    -1,    -1,    -1,    -1,    24,    25,    -1,    -1,    28,
      29,    -1,    -1,    -1,    -1,    34,    35,    36,    -1,    -1,
      39,    -1,    41,    -1,    43,    44,    -1,    -1,    -1,    48,
      49,    50,    -1,    -1,    53,    -1,    -1,    -1,    -1,    58,
      -1,    60,    -1,    -1,    -1,    64,    65,    66,    -1,    68,
      69,    70,    -1,    72,    73,    -1,    75,    -1,    77,    -1,
      79,    -1,    81,    -1,    -1,    84,    -1,    -1,    87,    88,
       1,    90,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    14,    -1,    -1,    17,    -1,    19,    -1,
      -1,    -1,    -1,    24,    25,    -1,    -1,    28,    29,    -1,
      -1,    -1,    -1,    34,    35,    36,    -1,    -1,    39,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    48,    49,    50,
      -1,    -1,    53,    -1,    -1,    -1,    -1,    58,    -1,    60,
      -1,    -1,    -1,    64,    65,    66,    -1,    68,    69,    70,
      -1,    -1,    73,    -1,    75,    -1,    77,    -1,    79,    -1,
      81,    -1,    -1,    84,    -1,    -1,    87,    88,     1,    90,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    17,    -1,    19,    -1,    -1,    -1,
      -1,    24,    25,    -1,    -1,    28,    29,    -1,    -1,    -1,
      -1,    34,    35,    36,    -1,    -1,    39,    40,    -1,    -1,
      43,    44,    -1,    -1,    -1,    48,    49,    50,    -1,    -1,
      53,    -1,    -1,    -1,    -1,    58,    -1,    60,    -1,    -1,
      -1,    64,    65,    66,    -1,    68,    69,    70,    -1,    -1,
      73,    -1,    75,    -1,    77,    -1,    79,    -1,    81,    -1,
      -1,    84,    -1,    -1,    87,    88,     1,    90,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    17,    18,    19,    -1,    -1,    -1,    -1,    24,
      25,    -1,    -1,    28,    29,    -1,    -1,    -1,    -1,    34,
      35,    36,    -1,    -1,    39,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    48,    49,    50,    -1,    -1,    53,    -1,
      -1,    -1,    -1,    58,    -1,    60,    -1,    -1,    -1,    64,
      65,    66,    -1,    68,    69,    70,    -1,    -1,    73,    -1,
      75,    -1,    77,    -1,    79,    -1,    81,    -1,    -1,    84,
      -1,    -1,    87,    88,     1,    90,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    18,    19,    -1,    -1,    -1,    -1,    24,    25,    -1,
      -1,    28,    29,    -1,    -1,    -1,    -1,    34,    35,    36,
      -1,    -1,    39,    -1,    -1,    -1,    43,    44,    -1,    -1,
      -1,    48,    49,    50,    -1,    -1,    53,    -1,    -1,    -1,
      -1,    58,    -1,    60,    -1,    -1,    -1,    64,    65,    66,
      -1,    68,    69,    70,    -1,    -1,    73,    -1,    75,    -1,
      77,    -1,    79,    -1,    81,    -1,    -1,    84,    -1,    -1,
      87,    88,     1,    90,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,
      19,    -1,    -1,    -1,    -1,    24,    25,    -1,    -1,    28,
      29,    -1,    -1,    -1,    -1,    34,    35,    36,    -1,    -1,
      39,    -1,    41,    -1,    43,    44,    -1,    -1,    -1,    48,
      49,    50,    -1,    -1,    53,    -1,    -1,    -1,    -1,    58,
      -1,    60,    -1,    -1,    -1,    64,    65,    66,    -1,    68,
      69,    70,    -1,    -1,    73,    -1,    75,    -1,    77,    -1,
      79,    -1,    81,    -1,    -1,    84,    -1,    -1,    87,    88,
       1,    90,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,    -1,
      -1,    -1,    -1,    24,    25,    -1,    -1,    28,    29,    -1,
      -1,    -1,    -1,    34,    35,    36,    -1,    -1,    39,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    48,    49,    50,
      -1,    -1,    53,    -1,    -1,    -1,    -1,    58,    -1,    60,
      -1,    -1,    -1,    64,    65,    66,    -1,    68,    69,    70,
      -1,    -1,    73,    74,    75,    -1,    77,    -1,    79,    -1,
      81,    -1,    -1,    84,    -1,    -1,    87,    88,     1,    90,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    17,    18,    19,    -1,    -1,    -1,
      -1,    24,    25,    -1,    -1,    28,    29,    -1,    -1,    -1,
      -1,    34,    35,    36,    -1,    -1,    39,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    48,    49,    50,    -1,    -1,
      53,    -1,    -1,    -1,    -1,    58,    -1,    60,    -1,    -1,
      -1,    64,    65,    66,    -1,    68,    69,    70,    -1,    -1,
      73,    -1,    75,    -1,    77,    -1,    79,    -1,    81,    -1,
      -1,    84,    -1,    -1,    87,    88,     1,    90,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    17,    18,    19,    -1,    -1,    -1,    -1,    24,
      25,    -1,    -1,    28,    29,    -1,    -1,    -1,    -1,    34,
      35,    36,    -1,    -1,    39,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    48,    49,    50,    -1,    -1,    53,    -1,
      -1,    -1,    -1,    58,    -1,    60,    -1,    -1,    -1,    64,
      65,    66,    -1,    68,    69,    70,    -1,    -1,    73,    -1,
      75,    -1,    77,    -1,    79,    -1,    81,    -1,    -1,    84,
      -1,    -1,    87,    88,     1,    90,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    -1,    19,    -1,    -1,    -1,    -1,    24,    25,    -1,
      -1,    28,    29,    -1,    -1,    -1,    -1,    34,    35,    36,
      -1,    -1,    39,    -1,    -1,    -1,    43,    44,    -1,    -1,
      -1,    48,    49,    50,    -1,    -1,    53,    -1,    -1,    -1,
      -1,    58,    -1,    60,    -1,    -1,    -1,    64,    65,    66,
      -1,    68,    69,    70,    71,    -1,    73,    -1,    75,    -1,
      77,    -1,    79,    -1,    81,    -1,    -1,    84,    -1,    -1,
      87,    88,     1,    90,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,
      19,    -1,    -1,    -1,    -1,    24,    25,    -1,    -1,    28,
      29,    -1,    -1,    -1,    -1,    34,    35,    36,    -1,    -1,
      39,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    48,
      49,    50,    -1,    -1,    53,    -1,    -1,    -1,    -1,    58,
      -1,    60,    -1,    -1,    -1,    64,    65,    66,    -1,    68,
      69,    70,    -1,    -1,    73,    -1,    75,    -1,    77,    -1,
      79,    -1,    81,    -1,    -1,    84,    -1,    -1,    87,    88,
       1,    90,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,    -1,
      -1,    -1,    -1,    24,    25,    -1,    -1,    28,    29,    -1,
      -1,    -1,    -1,    34,    35,    36,    -1,    -1,    39,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    48,    49,    50,
      -1,    -1,    53,    -1,    -1,    -1,    -1,    58,    -1,    60,
      -1,    -1,    -1,    64,    65,    66,    -1,    68,    69,    70,
      -1,    -1,    73,    74,    75,    -1,    77,    -1,    79,    -1,
      81,    -1,    -1,    84,    -1,    -1,    87,    88,     1,    90,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    17,    -1,    19,    -1,    -1,    -1,
      -1,    24,    25,    -1,    -1,    28,    29,    -1,    -1,    -1,
      -1,    34,    35,    36,    -1,    -1,    39,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    48,    49,    50,    -1,    -1,
      53,    -1,    -1,    -1,    -1,    58,    -1,    60,    -1,    -1,
      -1,    64,    65,    66,    -1,    68,    69,    70,    71,    -1,
      73,    -1,    75,    -1,    77,    -1,    79,    -1,    81,    -1,
      -1,    84,    -1,    -1,    87,    88,     1,    90,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    17,    18,    19,    -1,    -1,    -1,    -1,    24,
      25,    -1,    -1,    28,    29,    -1,    -1,    -1,    -1,    34,
      35,    36,    -1,    -1,    39,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    48,    49,    50,    -1,    -1,    53,    -1,
      -1,    -1,    -1,    58,    -1,    60,    -1,    -1,    -1,    64,
      65,    66,    -1,    68,    69,    70,    -1,    -1,    73,    -1,
      75,    -1,    77,    -1,    79,    -1,    81,    -1,    -1,    84,
      -1,    -1,    87,    88,     1,    90,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    -1,    19,    -1,    -1,    -1,    -1,    24,    25,    -1,
      -1,    28,    29,    -1,    -1,    -1,    -1,    34,    35,    36,
      -1,    -1,    39,    -1,    -1,    -1,    43,    44,    -1,    -1,
      -1,    48,    49,    50,    -1,    -1,    53,    -1,    -1,    -1,
      -1,    58,    -1,    60,    -1,    -1,    -1,    64,    65,    66,
      -1,    68,    69,    70,    -1,    -1,    73,    -1,    75,    -1,
      77,    -1,    79,    -1,    81,    -1,    -1,    84,    -1,    -1,
      87,    88,     1,    90,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,
      19,    -1,    -1,    -1,    -1,    24,    25,    -1,    -1,    28,
      29,    -1,    -1,    -1,    -1,    34,    35,    36,    -1,    -1,
      39,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    48,
      49,    50,    -1,    -1,    53,    -1,    -1,    -1,    -1,    58,
      -1,    60,    -1,    -1,    -1,    64,    65,    66,    -1,    68,
      69,    70,    -1,    -1,    73,    -1,    75,    -1,    77,    -1,
      79,    -1,    81,    -1,    -1,    84,    -1,    -1,    87,    88,
       1,    90,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,    -1,
      -1,    -1,    -1,    24,    25,    -1,    -1,    28,    29,    -1,
      -1,    -1,    -1,    34,    35,    36,    -1,    -1,    39,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    48,    49,    50,
      -1,    -1,    53,    -1,    -1,    -1,    -1,    58,    -1,    60,
      -1,    -1,    -1,    64,    65,    66,    -1,    68,    69,    70,
      -1,    -1,    73,    -1,    75,    -1,    77,    -1,    79,    -1,
      81,    -1,    -1,    84,    -1,    -1,    87,    88,     1,    90,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    -1,    -1,    -1,    17,    -1,    19,    -1,    -1,    -1,
      -1,    24,    25,    -1,    -1,    28,    29,    -1,    -1,    -1,
      -1,    34,    35,    36,    -1,    -1,    39,    -1,    -1,    -1,
      43,    44,    -1,    -1,    -1,    48,    49,    50,    -1,    -1,
      53,    -1,    -1,    -1,    -1,    58,    -1,    60,    -1,    -1,
      -1,    64,    65,    66,    -1,    68,    69,    70,    -1,    -1,
      73,    -1,    75,    -1,    77,    -1,    79,    -1,    81,    -1,
      -1,    84,    -1,    -1,    87,    88,     1,    90,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    -1,
      -1,    -1,    17,    -1,    19,    -1,    -1,    -1,    -1,    24,
      25,    -1,    -1,    28,    29,    -1,    -1,    -1,    -1,    34,
      35,    36,    -1,    -1,    39,    -1,    -1,    -1,    43,    44,
      -1,    -1,    -1,    48,    49,    50,    -1,    -1,    53,    -1,
      -1,    -1,    -1,    58,    -1,    60,    -1,    -1,    -1,    64,
      65,    66,    -1,    68,    69,    70,    -1,    -1,    73,    -1,
      75,    -1,    77,    -1,    79,    -1,    81,    -1,    -1,    84,
      -1,    -1,    87,    88,     1,    90,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    -1,    -1,    -1,
      17,    -1,    19,    -1,    -1,    -1,    -1,    24,    25,    -1,
      -1,    28,    29,    -1,    -1,    -1,    -1,    34,    35,    36,
      -1,    -1,    39,    -1,    -1,    -1,    43,    44,    -1,    -1,
      -1,    48,    49,    50,    -1,    -1,    53,    -1,    -1,    -1,
      -1,    58,    -1,    60,    -1,    -1,    -1,    64,    65,    66,
      -1,    68,    69,    70,    -1,    -1,    73,    -1,    75,    -1,
      77,    -1,    79,    -1,    81,    -1,    -1,    84,    -1,    -1,
      87,    88,     1,    90,     3,     4,     5,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    -1,    -1,    17,    -1,
      19,    -1,    -1,    -1,    -1,    24,    25,    -1,    -1,    28,
      29,    -1,    -1,    -1,    -1,    34,    35,    36,    -1,    -1,
      39,    -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    48,
      49,    50,    -1,    -1,    53,    -1,    -1,    -1,    -1,    58,
      -1,    60,    -1,    -1,    -1,    64,    65,    66,    -1,    68,
      69,    70,    -1,    -1,    73,    -1,    75,    -1,    77,    -1,
      79,    -1,    81,    -1,    -1,    84,    -1,    -1,    87,    88,
       1,    90,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    -1,    -1,    -1,    17,    -1,    19,    -1,
      -1,    -1,    -1,    24,    25,    -1,    -1,    28,    29,    -1,
      -1,    -1,    -1,    34,    35,    36,    -1,    -1,    39,    -1,
      -1,    -1,    43,    44,    -1,    -1,    -1,    48,    49,    50,
      -1,    -1,    53,    -1,    -1,    -1,    -1,    58,    -1,    60,
      -1,    -1,    -1,    64,    65,    66,    -1,    68,    69,    70,
      -1,    -1,    73,    -1,    75,    -1,    77,    -1,    79,    -1,
      81,    -1,    -1,    84,    -1,    -1,    87,    88,     1,    90,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    -1,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    -1,    60,    61,    62,
      -1,    64,    65,    66,    -1,    68,    69,    70,    -1,    72,
      73,    -1,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,     8,    -1,    -1,    -1,
      12,    -1,    14,    15,    16,    17,    -1,    19,    -1,    21,
      22,    23,    -1,    -1,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    -1,    -1,    -1,    41,
      42,    -1,    -1,    -1,    -1,    -1,    11,    12,    -1,    14,
      15,    16,    17,    -1,    19,    -1,    21,    22,    23,    -1,
      62,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    -1,    76,    -1,    -1,    42,    -1,    -1,
      82,    83,    -1,    85,    86,    12,    51,    14,    15,    16,
      17,    -1,    19,    -1,    21,    22,    23,    62,    -1,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    76,    -1,    40,    -1,    42,    -1,    82,    83,    -1,
      85,    86,    12,    -1,    -1,    -1,    53,    17,    -1,    19,
      -1,    21,    22,    23,    -1,    62,    26,    27,    28,    29,
      30,    31,    32,    -1,    34,    35,    36,    37,    -1,    76,
      -1,    -1,    -1,    -1,    -1,    82,    83,    -1,    85,    86,
      12,    -1,    14,    15,    16,    17,    -1,    19,    -1,    21,
      22,    23,    62,    -1,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    76,    -1,    40,    -1,
      42,    -1,    44,    -1,    -1,    -1,    86,    12,    -1,    14,
      15,    16,    17,    -1,    19,    -1,    21,    22,    23,    -1,
      62,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    -1,    76,    40,    -1,    42,    -1,    -1,
      82,    83,    -1,    85,    86,    12,    -1,    -1,    53,    16,
      17,    -1,    19,    -1,    21,    22,    23,    62,    -1,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    76,    -1,    -1,    -1,    42,    -1,    82,    83,    -1,
      85,    86,    12,    -1,    14,    15,    16,    17,    18,    19,
      -1,    21,    22,    23,    -1,    62,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    -1,    76,
      -1,    -1,    42,    80,    -1,    -1,    -1,    -1,    85,    86,
      12,    -1,    14,    15,    16,    17,    -1,    19,    -1,    21,
      22,    23,    62,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    76,    -1,    -1,    -1,
      42,    -1,    82,    83,    -1,    85,    86,    12,    -1,    14,
      15,    16,    17,    -1,    19,    -1,    21,    22,    23,    -1,
      62,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    -1,    76,    -1,    -1,    42,    -1,    -1,
      82,    83,    -1,    85,    86,    -1,    -1,    -1,    53,    -1,
      -1,    12,    -1,    -1,    -1,    -1,    17,    62,    19,    -1,
      21,    22,    23,    -1,    -1,    26,    -1,    -1,    29,    -1,
      -1,    76,    -1,    34,    35,    -1,    37,    82,    83,    -1,
      85,    86,    12,    -1,    14,    15,    16,    17,    -1,    19,
      -1,    21,    22,    23,    -1,    -1,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    -1,    -1,
      -1,    -1,    42,    -1,    -1,    76,    46,    -1,    -1,    12,
      -1,    14,    15,    16,    17,    86,    19,    -1,    21,    22,
      23,    -1,    62,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    -1,    76,    -1,    -1,    42,
      -1,    -1,    82,    83,    -1,    85,    86,    12,    -1,    14,
      15,    16,    17,    56,    19,    -1,    21,    22,    23,    62,
      -1,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    76,    -1,    -1,    -1,    42,    -1,    82,
      83,    -1,    85,    86,    12,    -1,    14,    15,    16,    17,
      18,    19,    -1,    21,    22,    23,    -1,    62,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      -1,    76,    -1,    78,    42,    -1,    -1,    82,    83,    -1,
      85,    86,    12,    -1,    14,    15,    16,    17,    18,    19,
      -1,    21,    22,    23,    62,    -1,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    76,    -1,
      -1,    -1,    42,    -1,    82,    83,    -1,    85,    86,    12,
      -1,    14,    15,    16,    17,    -1,    19,    -1,    21,    22,
      23,    -1,    62,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    -1,    76,    40,    -1,    42,
      -1,    -1,    82,    83,    -1,    85,    86,    12,    -1,    14,
      15,    16,    17,    18,    19,    -1,    21,    22,    23,    62,
      -1,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    76,    -1,    -1,    -1,    42,    -1,    82,
      83,    -1,    85,    86,    12,    -1,    14,    15,    16,    17,
      18,    19,    -1,    21,    22,    23,    -1,    62,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      -1,    76,    -1,    -1,    42,    -1,    -1,    82,    83,    -1,
      85,    86,    12,    -1,    14,    15,    16,    17,    18,    19,
      -1,    21,    22,    23,    62,    -1,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    76,    -1,
      -1,    -1,    42,    -1,    82,    83,    -1,    85,    86,    12,
      -1,    14,    15,    16,    17,    18,    19,    -1,    21,    22,
      23,    -1,    62,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    -1,    76,    -1,    -1,    42,
      -1,    -1,    82,    83,    -1,    85,    86,    12,    -1,    14,
      15,    16,    17,    18,    19,    -1,    21,    22,    23,    62,
      -1,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    76,    -1,    -1,    -1,    42,    -1,    82,
      83,    -1,    85,    86,    12,    -1,    14,    15,    16,    17,
      18,    19,    -1,    21,    22,    23,    -1,    62,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      -1,    76,    -1,    -1,    42,    -1,    -1,    82,    83,    -1,
      85,    86,    12,    -1,    14,    15,    16,    17,    18,    19,
      -1,    21,    22,    23,    62,    -1,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    76,    -1,
      -1,    -1,    42,    -1,    82,    83,    -1,    85,    86,    12,
      -1,    14,    15,    16,    17,    18,    19,    -1,    21,    22,
      23,    -1,    62,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    -1,    76,    -1,    -1,    42,
      -1,    -1,    82,    83,    -1,    85,    86,    12,    -1,    14,
      15,    16,    17,    -1,    19,    -1,    21,    22,    23,    62,
      -1,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    76,    -1,    40,    -1,    42,    -1,    82,
      83,    -1,    85,    86,    12,    -1,    14,    15,    16,    17,
      18,    19,    -1,    21,    22,    23,    -1,    62,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      -1,    76,    -1,    -1,    42,    -1,    -1,    82,    83,    -1,
      85,    86,    12,    -1,    14,    15,    16,    17,    -1,    19,
      -1,    21,    22,    23,    62,    -1,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    76,    -1,
      40,    -1,    42,    -1,    82,    83,    -1,    85,    86,    12,
      -1,    14,    15,    16,    17,    18,    19,    -1,    21,    22,
      23,    -1,    62,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    -1,    76,    -1,    -1,    42,
      -1,    -1,    82,    83,    -1,    85,    86,    12,    -1,    14,
      15,    16,    17,    -1,    19,    -1,    21,    22,    23,    62,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    76,    -1,    -1,    -1,    42,    -1,    82,
      83,    -1,    85,    86,    12,    -1,    14,    15,    16,    17,
      -1,    19,    -1,    21,    22,    23,    -1,    62,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      -1,    76,    -1,    -1,    42,    -1,    -1,    82,    83,    -1,
      85,    86,    12,    -1,    14,    15,    16,    17,    56,    19,
      -1,    21,    22,    23,    62,    -1,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    76,    -1,
      -1,    -1,    42,    -1,    82,    83,    -1,    85,    86,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    12,    -1,    -1,    -1,
      -1,    17,    62,    19,    -1,    21,    22,    23,    -1,    -1,
      26,    -1,    72,    -1,    -1,    -1,    76,    -1,    34,    35,
      -1,    37,    82,    83,    -1,    85,    86,    12,    -1,    14,
      15,    16,    17,    -1,    19,    -1,    21,    22,    23,    -1,
      -1,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    -1,    -1,    -1,    41,    42,    -1,    -1,
      76,    -1,    -1,    -1,    12,    -1,    14,    15,    16,    17,
      86,    19,    -1,    21,    22,    23,    -1,    62,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      -1,    76,    -1,    -1,    42,    -1,    -1,    82,    83,    -1,
      85,    86,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    57,
      -1,    -1,    -1,    -1,    62,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    76,    -1,
      -1,    -1,    -1,    -1,    82,    83,    -1,    85,    86,    12,
      -1,    14,    15,    16,    17,    -1,    19,    -1,    21,    22,
      23,    -1,    -1,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    -1,    -1,    -1,    41,    42,
      -1,    -1,    -1,    -1,    -1,    -1,    12,    -1,    14,    15,
      16,    17,    18,    19,    -1,    21,    22,    23,    -1,    62,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    -1,    76,    -1,    -1,    42,    -1,    -1,    82,
      83,    -1,    85,    86,    12,    -1,    14,    15,    16,    17,
      -1,    19,    -1,    21,    22,    23,    62,    -1,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      76,    -1,    40,    -1,    42,    -1,    82,    83,    -1,    85,
      86,    -1,    12,    -1,    14,    15,    16,    17,    -1,    19,
      -1,    21,    22,    23,    62,    -1,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    76,    -1,
      -1,    -1,    42,    -1,    82,    83,    46,    85,    86,    12,
      -1,    14,    15,    16,    17,    -1,    19,    -1,    21,    22,
      23,    -1,    62,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    -1,    76,    -1,    41,    42,
      -1,    -1,    82,    83,    -1,    85,    86,    12,    -1,    14,
      15,    16,    17,    -1,    19,    -1,    21,    22,    23,    62,
      -1,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    76,    -1,    40,    -1,    42,    -1,    82,
      83,    -1,    85,    86,    12,    -1,    14,    15,    16,    17,
      -1,    19,    -1,    21,    22,    23,    -1,    62,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      -1,    76,    -1,    41,    42,    -1,    -1,    82,    83,    -1,
      85,    86,    12,    -1,    14,    15,    16,    17,    -1,    19,
      -1,    21,    22,    23,    62,    -1,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    76,    -1,
      -1,    -1,    42,    -1,    82,    83,    -1,    85,    86,    12,
      -1,    14,    15,    16,    17,    -1,    19,    -1,    21,    22,
      23,    -1,    62,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    -1,    76,    -1,    -1,    42,
      -1,    -1,    82,    83,    -1,    85,    86,    12,    -1,    14,
      15,    16,    17,    -1,    19,    -1,    21,    22,    23,    62,
      -1,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    76,    -1,    -1,    -1,    42,    -1,    82,
      83,    -1,    85,    86,    12,    -1,    -1,    15,    16,    17,
      -1,    19,    -1,    21,    22,    23,    -1,    62,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      -1,    76,    -1,    -1,    42,    -1,    -1,    -1,    83,    -1,
      85,    86,    12,    -1,    14,    15,    16,    17,    -1,    19,
      -1,    21,    22,    23,    62,    -1,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    76,    -1,
      -1,    -1,    42,    -1,    82,    83,    -1,    85,    86,    12,
      -1,    -1,    15,    16,    17,    -1,    19,    -1,    21,    22,
      23,    -1,    62,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    -1,    76,    -1,    -1,    42,
      -1,    -1,    -1,    -1,    12,    85,    86,    -1,    16,    17,
      -1,    19,    -1,    21,    22,    23,    -1,    -1,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      -1,    -1,    -1,    76,    42,    -1,    -1,    -1,    -1,    82,
      83,    12,    85,    86,    -1,    16,    17,    -1,    19,    -1,
      21,    22,    23,    -1,    62,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    -1,    76,    -1,
      -1,    42,    -1,    -1,    -1,    -1,    -1,    85,    86,    12,
      -1,    -1,    -1,    16,    17,    -1,    19,    -1,    21,    22,
      23,    62,    -1,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    76,    -1,    -1,    -1,    42,
      -1,    -1,    -1,    -1,    85,    86,    12,    -1,    -1,    -1,
      16,    17,    -1,    19,    -1,    21,    22,    23,    -1,    62,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    -1,    76,    -1,    12,    -1,    -1,    -1,    -1,
      17,    -1,    19,    86,    21,    22,    23,    -1,    -1,    26,
      27,    28,    29,    30,    31,    32,    62,    34,    35,    36,
      37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      76,    -1,    12,    -1,    -1,    -1,    -1,    17,    -1,    19,
      86,    21,    22,    23,    -1,    62,    26,    27,    28,    29,
      30,    31,    32,    12,    34,    35,    -1,    37,    17,    76,
      19,    -1,    21,    22,    23,    -1,    -1,    26,    -1,    86,
      29,    30,    31,    32,    -1,    34,    35,    -1,    37,    -1,
      12,    -1,    -1,    -1,    -1,    17,    -1,    19,    -1,    21,
      22,    23,    -1,    -1,    26,    -1,    76,    29,    -1,    31,
      32,    -1,    34,    35,    -1,    37,    86,    -1,    -1,    12,
      -1,    -1,    -1,    -1,    17,    -1,    19,    76,    21,    22,
      23,    -1,    -1,    26,    -1,    -1,    29,    86,    31,    32,
      -1,    34,    35,    -1,    37,    -1,    12,    -1,    -1,    -1,
      -1,    17,    -1,    -1,    76,    21,    22,    23,    -1,    -1,
      26,    -1,    12,    -1,    86,    -1,    -1,    17,    34,    35,
      -1,    37,    22,    23,    -1,    -1,    26,    -1,    -1,    -1,
      -1,    12,    -1,    76,    34,    35,    17,    37,    -1,    -1,
      -1,    22,    23,    86,    -1,    26,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    34,    35,    -1,    37,    -1,    -1,    -1,
      76,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    -1,    -1,    -1,    76,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    76,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    86
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     3,     4,     5,     6,     7,     9,    10,    11,
      12,    13,    17,    19,    24,    25,    28,    29,    34,    35,
      36,    39,    43,    44,    48,    49,    50,    53,    58,    60,
      64,    65,    66,    68,    69,    70,    73,    75,    77,    79,
      81,    84,    87,    88,    90,    94,    95,    96,    98,     4,
       5,    10,    43,    99,    43,    15,    17,    17,    37,    96,
       4,    96,    96,    13,    73,    96,    96,    14,    96,     4,
      96,    96,   105,     4,    17,    96,    17,    17,    17,    96,
       3,     4,    10,    13,    17,    88,    97,    98,    17,    96,
      96,   106,    71,   111,    17,    17,    98,    17,    96,    17,
      37,    96,    96,    96,    96,   106,    17,    17,    96,     0,
       8,    12,    14,    15,    16,    17,    19,    21,    22,    23,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    41,    42,    62,    76,    82,    83,    85,    86,
      17,    17,     4,    23,    86,     4,    10,    13,    25,    88,
      96,    18,    96,    18,    96,    96,    18,    25,    25,    25,
      96,    53,    40,    96,    46,    96,    96,     4,    11,    51,
     116,    96,   104,    62,    15,    28,    54,   108,    96,    56,
      41,    61,    96,   117,   100,   106,    67,    96,    96,   105,
      17,    96,    96,    96,    78,    14,    46,    96,    18,    62,
      96,    96,    96,   111,   105,    96,    96,    96,    96,    96,
      98,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      98,    39,    96,     8,    95,    96,    96,     6,    10,    14,
      19,    21,    82,    98,    96,    96,   105,    96,     3,    96,
      18,    18,    40,    17,     3,     4,    10,    13,    17,    98,
     102,   103,    18,   106,   111,    18,    18,    18,     3,   106,
      52,    18,    41,    96,    96,    96,    96,    55,    18,   106,
      96,    74,   101,   106,    72,    17,    18,    18,   105,    18,
      18,    40,    96,   106,    18,    96,    18,    27,    22,    62,
      96,    40,    17,    37,    18,    18,    25,   105,    15,    28,
      42,    43,   103,    14,    62,    17,    96,   111,    45,    47,
      72,   112,   113,   114,    47,    71,    56,    96,   104,    56,
      96,    96,   111,    72,    72,   102,    72,    96,   100,    18,
      80,    47,    72,    96,    96,    40,   105,    39,    96,    15,
      69,    23,    18,    96,    96,    96,    96,    18,   103,    96,
      96,    41,   110,   114,    96,   106,   111,    50,    51,   115,
     106,    41,   106,    57,   107,    41,    41,    18,    71,   106,
      71,   100,   106,   106,    40,    18,    96,    40,    69,    96,
     111,   100,     3,    40,    44,   110,    96,   111,    46,   113,
       3,    42,    72,    11,   116,   104,    47,    72,    96,    41,
      56,   109,   111,   106,    72,   100,   106,    72,    72,    40,
     100,   106,    96,    41,   106,    42,   111,    18,   106,   106,
      72,   106,    72,    40,   106,    72,    40,   112,   111,    96,
     111,    72,    72,    72,    72,   115,    41
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    93,    94,    95,    95,    95,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      96,    96,    96,    96,    96,    96,    96,    96,    96,    96,
      97,    97,    97,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    99,    99,   100,   100,   101,   102,   102,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   104,
     104,   105,   105,   106,   106,   106,   107,   107,   108,   108,
     108,   108,   109,   109,   110,   110,   110,   111,   111,   112,
     112,   112,   113,   114,   115,   115,   115,   116,   116,   116,
     117
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     3,     3,     1,     2,     4,     3,
       5,     7,     1,     1,     6,     6,     6,     6,     8,     3,
       3,     3,     3,     3,     3,     4,     4,     1,     1,     1,
       3,     4,     3,     3,     3,     5,     3,     2,     3,     3,
       3,     3,     3,     2,     2,     3,     3,     3,     2,     5,
       3,     1,     3,     2,     4,     4,     1,     4,     4,     4,
       3,     1,     2,     2,     6,     4,     5,     4,     1,     3,
       3,     4,     1,     2,     1,     3,     2,     1,     3,     1,
       1,     7,     9,     9,     1,     4,     5,     7,     5,     1,
       3,     2,     5,     3,     3,     2,     3,     1,     3,     4,
       3,     3,     3,     1,     3,     4,     6,     6,     3,     3,
       3,     2,     3,     2,     1,     6,     7,     7,     8,     7,
       9,     8,     8,     9,     9,    10,     4,     3,     4,     7,
       9,     5,     6,     5,     7,     4,     1,     7,     4,     4,
       1,     1,     1,     1,     3,     3,     3,     3,     5,     2,
       3,     2,     2,     1,     1,     0,     2,     3,     1,     3,
       1,     3,     3,     3,     3,     1,     3,     1,     1,     0,
       1,     0,     1,     1,     2,     2,     0,     2,     0,     2,
       2,     2,     1,     1,     0,     3,     2,     3,     4,     1,
       3,     5,     1,     1,     0,     3,     5,     0,     2,     5,
       1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (scanner, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, scanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void * scanner)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (scanner);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void * scanner)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, scanner);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule, void * scanner)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              , scanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, scanner); \
} while (0)

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
#ifndef YYINITDEPTH
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
static YYSIZE_T
yystrlen (const char *yystr)
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
static char *
yystpcpy (char *yydest, const char *yysrc)
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
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
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
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
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

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

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

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, void * scanner)
{
  YYUSE (yyvaluep);
  YYUSE (scanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void * scanner)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
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
  int yytoken = 0;
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

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
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
      yychar = yylex (&yylval,YYLEX_PARAM);
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
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
     '$$ = $1'.

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
#line 184 "input_parser.yy" /* yacc.c:1646  */
    {   const giac::context * contextptr = giac_yyget_extra(scanner);
			    if ((yyvsp[0])._VECTptr->size()==1)
			     parsed_gen((yyvsp[0])._VECTptr->front(),contextptr);
                          else
			     parsed_gen(gen(*(yyvsp[0])._VECTptr,_SEQ__VECT),contextptr);
			 }
#line 3147 "y.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 192 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=vecteur(1,(yyvsp[-1])); }
#line 3153 "y.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 193 "input_parser.yy" /* yacc.c:1646  */
    { if ((yyvsp[-1]).val==1) (yyval)=vecteur(1,symbolic(at_nodisp,(yyvsp[-2]))); else (yyval)=vecteur(1,(yyvsp[-2])); }
#line 3159 "y.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 194 "input_parser.yy" /* yacc.c:1646  */
    { if ((yyvsp[-1]).val==1) (yyval)=mergevecteur(makevecteur(symbolic(at_nodisp,(yyvsp[-2]))),*(yyvsp[0])._VECTptr); else (yyval)=mergevecteur(makevecteur((yyvsp[-2])),*(yyvsp[0])._VECTptr); }
#line 3165 "y.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 197 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = (yyvsp[0]);}
#line 3171 "y.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 198 "input_parser.yy" /* yacc.c:1646  */
    {if (is_one((yyvsp[-1]))) (yyval)=(yyvsp[0]); else (yyval)=symbolic(at_prod,gen(makevecteur((yyvsp[-1]),(yyvsp[0])),_SEQ__VECT));}
#line 3177 "y.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 199 "input_parser.yy" /* yacc.c:1646  */
    {if (is_one((yyvsp[-3]))) (yyval)=symb_pow((yyvsp[-2]),(yyvsp[0])); else (yyval)=symbolic(at_prod,gen(makevecteur((yyvsp[-3]),symb_pow((yyvsp[-2]),(yyvsp[0]))),_SEQ__VECT));}
#line 3183 "y.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 200 "input_parser.yy" /* yacc.c:1646  */
    {(yyval)=symbolic(at_prod,gen(makevecteur((yyvsp[-2]),symb_pow((yyvsp[-1]),(yyvsp[0]))) ,_SEQ__VECT));}
#line 3189 "y.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 201 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) =(yyvsp[-4])*symbolic(*(yyvsp[-3])._FUNCptr,python_compat(giac_yyget_extra(scanner))?denest_sto((yyvsp[-1])):(yyvsp[-1])); }
#line 3195 "y.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 202 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) =(yyvsp[-6])*symb_pow(symbolic(*(yyvsp[-5])._FUNCptr,python_compat(giac_yyget_extra(scanner))?denest_sto((yyvsp[-3])):(yyvsp[-3])),(yyvsp[0])); }
#line 3201 "y.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 204 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 3207 "y.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 205 "input_parser.yy" /* yacc.c:1646  */
    { if ((yyvsp[0]).type==_FUNC) (yyval)=symbolic(*(yyvsp[0])._FUNCptr,gen(vecteur(0),_SEQ__VECT)); else (yyval)=(yyvsp[0]); }
#line 3213 "y.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 208 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symb_program_sto((yyvsp[-3]),(yyvsp[-3])*zero,(yyvsp[0]),(yyvsp[-5]),false,giac_yyget_extra(scanner));}
#line 3219 "y.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 209 "input_parser.yy" /* yacc.c:1646  */
    {if (is_array_index((yyvsp[-5]),(yyvsp[-3]),giac_yyget_extra(scanner)) || (abs_calc_mode(giac_yyget_extra(scanner))==38 && (yyvsp[-5]).type==_IDNT && strlen((yyvsp[-5])._IDNTptr->id_name)==2 && check_vect_38((yyvsp[-5])._IDNTptr->id_name))) (yyval)=symbolic(at_sto,makesequence((yyvsp[0]),symbolic(at_of,makesequence((yyvsp[-5]),(yyvsp[-3]))))); else { (yyval) = symb_program_sto((yyvsp[-3]),(yyvsp[-3])*zero,(yyvsp[0]),(yyvsp[-5]),true,giac_yyget_extra(scanner)); (yyval)._SYMBptr->feuille.subtype=_SORTED__VECT;  } }
#line 3225 "y.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 210 "input_parser.yy" /* yacc.c:1646  */
    {if (is_array_index((yyvsp[-3]),(yyvsp[-1]),giac_yyget_extra(scanner)) || (abs_calc_mode(giac_yyget_extra(scanner))==38 && (yyvsp[-3]).type==_IDNT && check_vect_38((yyvsp[-3])._IDNTptr->id_name))) (yyval)=symbolic(at_sto,makesequence((yyvsp[-5]),symbolic(at_of,makesequence((yyvsp[-3]),(yyvsp[-1]))))); else (yyval) = symb_program_sto((yyvsp[-1]),(yyvsp[-1])*zero,(yyvsp[-5]),(yyvsp[-3]),false,giac_yyget_extra(scanner));}
#line 3231 "y.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 211 "input_parser.yy" /* yacc.c:1646  */
    { 
         const giac::context * contextptr = giac_yyget_extra(scanner);
         gen g=symb_at((yyvsp[-3]),(yyvsp[-1]),contextptr); (yyval)=symb_sto((yyvsp[-5]),g); 
        }
#line 3240 "y.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 215 "input_parser.yy" /* yacc.c:1646  */
    { 
         const giac::context * contextptr = giac_yyget_extra(scanner);
         gen g=symbolic(at_of,makesequence((yyvsp[-5]),(yyvsp[-2]))); (yyval)=symb_sto((yyvsp[-7]),g); 
        }
#line 3249 "y.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 219 "input_parser.yy" /* yacc.c:1646  */
    {  (yyval)=symb_sto((yyvsp[-2]),(yyvsp[0])); }
#line 3255 "y.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 220 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(at_convert,makesequence((yyvsp[-2]),(yyvsp[0]))); }
#line 3261 "y.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 221 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(at_convert,makesequence((yyvsp[-2]),(yyvsp[0]))); }
#line 3267 "y.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 222 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(at_convert,makesequence((yyvsp[-2]),(yyvsp[0]))); }
#line 3273 "y.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 223 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(at_time,(yyvsp[-2]));}
#line 3279 "y.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 224 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(at_solve,symb_equal((yyvsp[-2]),0));}
#line 3285 "y.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 225 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = check_symb_of((yyvsp[-3]),python_compat(giac_yyget_extra(scanner))?denest_sto((yyvsp[-1])):(yyvsp[-1]),giac_yyget_extra(scanner));}
#line 3291 "y.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 226 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = check_symb_of((yyvsp[-3]),python_compat(giac_yyget_extra(scanner))?denest_sto((yyvsp[-1])):(yyvsp[-1]),giac_yyget_extra(scanner));}
#line 3297 "y.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 227 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = (yyvsp[0]);}
#line 3303 "y.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 228 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = (yyvsp[0]);}
#line 3309 "y.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 229 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = (yyvsp[0]);}
#line 3315 "y.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 230 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-2])._FUNCptr,(yyvsp[0]));}
#line 3321 "y.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 231 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-3])._FUNCptr,(yyvsp[-1]));}
#line 3327 "y.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 232 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-2])._FUNCptr,gen(vecteur(0),_SEQ__VECT));}
#line 3333 "y.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 233 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[0])._FUNCptr,(yyvsp[-2]));}
#line 3339 "y.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 234 "input_parser.yy" /* yacc.c:1646  */
    {if (is_inequation((yyvsp[-2])) || ((yyvsp[-2]).is_symb_of_sommet(at_same) && ((yyvsp[0]).type!=_INT_ || (yyvsp[0]).subtype!=_INT_BOOLEAN))){ (yyval) = symb_and((yyvsp[-2]),symbolic(*(yyvsp[-1])._FUNCptr,makesequence((yyvsp[-2])._SYMBptr->feuille[1],(yyvsp[0]))));} else (yyval) = symbolic(*(yyvsp[-1])._FUNCptr,makesequence((yyvsp[-2]),(yyvsp[0])));}
#line 3345 "y.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 235 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symb_and(symbolic(*(yyvsp[-3])._FUNCptr,makesequence((yyvsp[-4]),(yyvsp[-2]))),symbolic(*(yyvsp[-1])._FUNCptr,makesequence((yyvsp[-2]),(yyvsp[0]))));}
#line 3351 "y.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 236 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-1])._FUNCptr,makesequence((yyvsp[-2]),(yyvsp[0]))); }
#line 3357 "y.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 237 "input_parser.yy" /* yacc.c:1646  */
    { 
	if ((yyvsp[0]).type==_SYMB) (yyval)=(yyvsp[0]); else (yyval)=symbolic(at_nop,(yyvsp[0])); 
	(yyval).change_subtype(_SPREAD__SYMB); 
        const giac::context * contextptr = giac_yyget_extra(scanner);
       spread_formula(false,contextptr); 
	}
#line 3368 "y.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 243 "input_parser.yy" /* yacc.c:1646  */
    { if ((yyvsp[-2]).is_symb_of_sommet(at_plus) && (yyvsp[-2])._SYMBptr->feuille.type==_VECT && (yyvsp[-1])==at_plus){ (yyvsp[-2])._SYMBptr->feuille._VECTptr->push_back((yyvsp[0])); (yyval)=(yyvsp[-2]); } else
  (yyval) =symbolic(*(yyvsp[-1])._FUNCptr,makesequence((yyvsp[-2]),(yyvsp[0])));}
#line 3375 "y.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 245 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) =symbolic(*(yyvsp[-1])._FUNCptr,makesequence((yyvsp[-2]),(yyvsp[0])));}
#line 3381 "y.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 246 "input_parser.yy" /* yacc.c:1646  */
    {if ((yyvsp[-2])==symbolic(at_exp,1) && (yyvsp[-1])==at_pow) (yyval)=symbolic(at_exp,(yyvsp[0])); else (yyval) =symbolic(*(yyvsp[-1])._FUNCptr,makesequence((yyvsp[-2]),(yyvsp[0])));}
#line 3387 "y.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 247 "input_parser.yy" /* yacc.c:1646  */
    {if ((yyvsp[-1]).type==_FUNC) (yyval)=symbolic(*(yyvsp[-1])._FUNCptr,makesequence((yyvsp[-2]),(yyvsp[0]))); else (yyval) = symbolic(at_normalmod,makesequence((yyvsp[-2]),(yyvsp[0])));}
#line 3393 "y.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 248 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-1])._FUNCptr,makesequence((yyvsp[-2]),(yyvsp[0]))); }
#line 3399 "y.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 249 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[0])._FUNCptr,makesequence((yyvsp[-1]),RAND_MAX)); }
#line 3405 "y.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 250 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-1])._FUNCptr,makesequence(0,(yyvsp[0]))); }
#line 3411 "y.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 251 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = makesequence(symbolic(*(yyvsp[-2])._FUNCptr,makesequence(0,RAND_MAX)),(yyvsp[0])); }
#line 3417 "y.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 253 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-1])._FUNCptr,makesequence((yyvsp[-2]),(yyvsp[0])));}
#line 3423 "y.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 254 "input_parser.yy" /* yacc.c:1646  */
    {(yyval)= symbolic(at_deuxpoints,makesequence((yyvsp[-2]),(yyvsp[0])) );}
#line 3429 "y.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 255 "input_parser.yy" /* yacc.c:1646  */
    { 
					if ((yyvsp[0])==unsigned_inf){
						(yyval) = (yyvsp[-1])==at_binary_minus?minus_inf:plus_inf;
					}
					else {
					 if ((yyvsp[-1])==at_binary_minus)
					   if ((yyvsp[0]).type==_INT_) (yyval)=(-(yyvsp[0]).val); else { if ((yyvsp[0]).type==_DOUBLE_) (yyval)=(-(yyvsp[0])._DOUBLE_val); else (yyval)=symbolic(at_neg,(yyvsp[0])); }
					   }
				}
#line 3443 "y.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 264 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = polynome_or_sparse_poly1(eval((yyvsp[-3]),1, giac_yyget_extra(scanner)),(yyvsp[-1]));}
#line 3449 "y.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 265 "input_parser.yy" /* yacc.c:1646  */
    { 
           if ( ((yyvsp[-1]).type==_SYMB) && ((yyvsp[-1])._SYMBptr->sommet==at_deuxpoints) )
             (yyval) = algebraic_EXTension((yyvsp[-1])._SYMBptr->feuille._VECTptr->front(),(yyvsp[-1])._SYMBptr->feuille._VECTptr->back());
           else (yyval)=(yyvsp[-1]);
        }
#line 3459 "y.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 270 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=gen(at_of,2); }
#line 3465 "y.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 271 "input_parser.yy" /* yacc.c:1646  */
    {if ((yyvsp[-2]).type==_FUNC) *logptr(giac_yyget_extra(scanner))<< ("Warning: "+(yyvsp[-2]).print(context0)+" is a reserved word")<<endl; if ((yyvsp[-2]).type==_INT_) (yyval)=symb_equal((yyvsp[-2]),(yyvsp[0])); else {(yyval) = symb_sto((yyvsp[0]),(yyvsp[-2]),(yyvsp[-1])==at_array_sto); if ((yyvsp[0]).is_symb_of_sommet(at_program)) *logptr(giac_yyget_extra(scanner))<<"// End defining "<<(yyvsp[-2])<<endl;}}
#line 3471 "y.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 272 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = symbolic(*(yyvsp[-1])._FUNCptr,(yyvsp[0]));}
#line 3477 "y.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 273 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symb_args((yyvsp[-1]));}
#line 3483 "y.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 274 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symb_args((yyvsp[-1]));}
#line 3489 "y.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 275 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_args(vecteur(0)); }
#line 3495 "y.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 276 "input_parser.yy" /* yacc.c:1646  */
    {
	gen tmp=python_compat(giac_yyget_extra(scanner))?denest_sto((yyvsp[-1])):(yyvsp[-1]);
	// CERR << python_compat(giac_yyget_extra(scanner)) << tmp << endl;
	(yyval) = symbolic(*(yyvsp[-3])._FUNCptr,tmp);
        const giac::context * contextptr = giac_yyget_extra(scanner);
	if (*(yyvsp[-3])._FUNCptr==at_maple_mode ||*(yyvsp[-3])._FUNCptr==at_xcas_mode ){
          xcas_mode(contextptr)=(yyvsp[-1]).val;
        }
	}
#line 3509 "y.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 285 "input_parser.yy" /* yacc.c:1646  */
    {
	if ((yyvsp[-1]).type==_VECT && (yyvsp[-1])._VECTptr->empty())
          giac_yyerror(scanner,"void argument");
	(yyval) = symbolic(*(yyvsp[-3])._FUNCptr,python_compat(giac_yyget_extra(scanner))?denest_sto((yyvsp[-1])):(yyvsp[-1]));	
	}
#line 3519 "y.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 290 "input_parser.yy" /* yacc.c:1646  */
    { 
          const giac::context * contextptr = giac_yyget_extra(scanner);
          (yyval)=symb_at((yyvsp[-3]),(yyvsp[-1]),contextptr);
        }
#line 3528 "y.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 294 "input_parser.yy" /* yacc.c:1646  */
    {
	(yyval) = symbolic(*(yyvsp[-2])._FUNCptr,gen(vecteur(0),_SEQ__VECT));
	if (*(yyvsp[-2])._FUNCptr==at_rpn)
          rpn_mode(giac_yyget_extra(scanner))=1;
	if (*(yyvsp[-2])._FUNCptr==at_alg)
          rpn_mode(giac_yyget_extra(scanner))=0;
	}
#line 3540 "y.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 301 "input_parser.yy" /* yacc.c:1646  */
    {
	(yyval) = (yyvsp[0]);
	}
#line 3548 "y.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 304 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(at_derive,(yyvsp[-1]));}
#line 3554 "y.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 305 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(*(yyvsp[0])._FUNCptr,(yyvsp[-1])); }
#line 3560 "y.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 306 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic((yyvsp[-5]).type==_FUNC?*(yyvsp[-5])._FUNCptr:*at_ifte,makevecteur(equaltosame((yyvsp[-4])),symb_bloc((yyvsp[-2])),symb_bloc((yyvsp[0]))));}
#line 3566 "y.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 307 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic((yyvsp[-3]).type==_FUNC?*(yyvsp[-3])._FUNCptr:*at_ifte,makevecteur(equaltosame((yyvsp[-2])),(yyvsp[0]),0));}
#line 3572 "y.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 308 "input_parser.yy" /* yacc.c:1646  */
    {
	(yyval) = symbolic((yyvsp[-4]).type==_FUNC?*(yyvsp[-4])._FUNCptr:*at_ifte,makevecteur(equaltosame((yyvsp[-3])),symb_bloc((yyvsp[-1])),(yyvsp[0])));
	}
#line 3580 "y.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 311 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic((yyvsp[-3]).type==_FUNC?*(yyvsp[-3])._FUNCptr:*at_ifte,(yyvsp[-1]));}
#line 3586 "y.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 312 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = (yyvsp[0]);}
#line 3592 "y.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 313 "input_parser.yy" /* yacc.c:1646  */
    {
          const giac::context * contextptr = giac_yyget_extra(scanner);
         (yyval) = symb_program((yyvsp[-2]),zero*(yyvsp[-2]),(yyvsp[0]),contextptr);
        }
#line 3601 "y.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 317 "input_parser.yy" /* yacc.c:1646  */
    {
          const giac::context * contextptr = giac_yyget_extra(scanner);
             if ((yyvsp[0]).type==_VECT) 
                (yyval) = symb_program((yyvsp[-2]),zero*(yyvsp[-2]),symb_bloc(makevecteur(at_nop,(yyvsp[0]))),contextptr); 
             else 
                (yyval) = symb_program((yyvsp[-2]),zero*(yyvsp[-2]),(yyvsp[0]),contextptr);
		}
#line 3613 "y.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 324 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symb_bloc((yyvsp[-1]));}
#line 3619 "y.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 325 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = at_bloc;}
#line 3625 "y.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 326 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(*(yyvsp[-1])._FUNCptr,(yyvsp[0])); }
#line 3631 "y.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 327 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[0])._FUNCptr,gen(vecteur(0),_SEQ__VECT));}
#line 3637 "y.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 328 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(*(yyvsp[-2])._FUNCptr,gen(vecteur(0),_SEQ__VECT));}
#line 3643 "y.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 330 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(*(yyvsp[-1])._FUNCptr,(yyvsp[0])); }
#line 3649 "y.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 332 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = gen(*(yyvsp[0])._FUNCptr,0);}
#line 3655 "y.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 333 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[-1]);}
#line 3661 "y.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 335 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(at_break,zero);}
#line 3667 "y.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 336 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic(at_continue,zero);}
#line 3673 "y.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 337 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=2 && (yyvsp[0]).val!=9)
	    giac_yyerror(scanner,"missing loop end delimiter");
 	  bool rg=(yyvsp[-3]).is_symb_of_sommet(at_range);
          gen f=(yyvsp[-3]).type==_SYMB?(yyvsp[-3])._SYMBptr->feuille:0,inc=1;
          if (rg){
            if (f.type!=_VECT) f=makesequence(0,f);
            vecteur v=*f._VECTptr;
            if (v.size()>=2) f=makesequence(v.front(),v[1]-1);
            if (v.size()==3) inc=v[2];
          }
          if (inc.type==_INT_  && inc.val!=0 && f.type==_VECT && f._VECTptr->size()==2 && (rg || ((yyvsp[-3]).is_symb_of_sommet(at_interval) 
	  // && f._VECTptr->front().type==_INT_ && f._VECTptr->back().type==_INT_ 
	  )))
            (yyval)=symbolic((yyvsp[-6]).type==_FUNC?*(yyvsp[-6])._FUNCptr:*at_for,makevecteur(symb_sto(f._VECTptr->front(),(yyvsp[-5])),inc.val>0?symb_inferieur_egal((yyvsp[-5]),f._VECTptr->back()):symb_superieur_egal((yyvsp[-5]),f._VECTptr->back()),symb_sto(symb_plus((yyvsp[-5]),inc),(yyvsp[-5])),symb_bloc((yyvsp[-1]))));
          else 
            (yyval)=symbolic((yyvsp[-6]).type==_FUNC?*(yyvsp[-6])._FUNCptr:*at_for,makevecteur(1,symbolic(*(yyvsp[-6])._FUNCptr,makevecteur((yyvsp[-5]),(yyvsp[-3]))),1,symb_bloc((yyvsp[-1]))));
	  }
#line 3696 "y.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 355 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=2 && (yyvsp[0]).val!=9)
	    giac_yyerror(scanner,"missing loop end delimiter");
	  (yyval)=symbolic((yyvsp[-8]).type==_FUNC?*(yyvsp[-8])._FUNCptr:*at_for,makevecteur(1,symbolic(*(yyvsp[-8])._FUNCptr,makevecteur((yyvsp[-7]),(yyvsp[-5]),symb_bloc((yyvsp[-1])))),1,symb_bloc((yyvsp[-3]))));
	  }
#line 3706 "y.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 360 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=2 && (yyvsp[0]).val!=9) giac_yyerror(scanner,"missing loop end delimiter");
          gen tmp,st=(yyvsp[-3]);  
       if (st==1 && (yyvsp[-5])!=1) st=(yyvsp[-5]);
          const giac::context * contextptr = giac_yyget_extra(scanner);
	  if (!lidnt(st).empty())
            *logptr(contextptr) << "Warning, step is not numeric " << st << std::endl;
          bool b=has_evalf(st,tmp,1,context0);
          if (!b || is_positive(tmp,context0)) 
             (yyval)=symbolic((yyvsp[-8]).type==_FUNC?*(yyvsp[-8])._FUNCptr:*at_for,makevecteur(symb_sto((yyvsp[-6]),(yyvsp[-7])),symb_inferieur_egal((yyvsp[-7]),(yyvsp[-4])),symb_sto(symb_plus((yyvsp[-7]),b?abs(st,context0):symb_abs(st)),(yyvsp[-7])),symb_bloc((yyvsp[-1])))); 
          else 
            (yyval)=symbolic((yyvsp[-8]).type==_FUNC?*(yyvsp[-8])._FUNCptr:*at_for,makevecteur(symb_sto((yyvsp[-6]),(yyvsp[-7])),symb_superieur_egal((yyvsp[-7]),(yyvsp[-4])),symb_sto(symb_plus((yyvsp[-7]),st),(yyvsp[-7])),symb_bloc((yyvsp[-1])))); 
        }
#line 3724 "y.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 373 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = gen((yyvsp[0]).type==_FUNC?*(yyvsp[0])._FUNCptr:*at_for,4);}
#line 3730 "y.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 374 "input_parser.yy" /* yacc.c:1646  */
    { 
        vecteur v=gen2vecteur((yyvsp[-2]));
        v.push_back(symb_ifte(equaltosame((yyvsp[0])),symbolic(at_break,zero),0));
	(yyval)=symbolic(*(yyvsp[-3])._FUNCptr,makevecteur(zero,1,zero,symb_bloc(v))); 
	}
#line 3740 "y.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 379 "input_parser.yy" /* yacc.c:1646  */
    { 
        if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=2 && (yyvsp[0]).val!=9) giac_yyerror(scanner,"missing loop end delimiter");
        vecteur v=gen2vecteur((yyvsp[-3]));
        v.push_back(symb_ifte(equaltosame((yyvsp[-1])),symbolic(at_break,zero),0));
	(yyval)=symbolic(*(yyvsp[-4])._FUNCptr,makevecteur(zero,1,zero,symb_bloc(v))); 
	}
#line 3751 "y.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 385 "input_parser.yy" /* yacc.c:1646  */
    {
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=4) giac_yyerror(scanner,"missing iferr end delimiter");
           (yyval)=symbolic(at_try_catch,makevecteur(symb_bloc((yyvsp[-5])),0,symb_bloc((yyvsp[-3])),symb_bloc((yyvsp[-1]))));
        }
#line 3760 "y.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 389 "input_parser.yy" /* yacc.c:1646  */
    {
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=4) giac_yyerror(scanner,"missing iferr end delimiter");
           (yyval)=symbolic(at_try_catch,makevecteur(symb_bloc((yyvsp[-3])),0,symb_bloc((yyvsp[-1])),symb_bloc(0)));
        }
#line 3769 "y.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 393 "input_parser.yy" /* yacc.c:1646  */
    { 
	(yyval)=(yyvsp[0]); 
	// $$.subtype=1; 
	}
#line 3778 "y.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 397 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[-1]); /* $$.subtype=1; */ }
#line 3784 "y.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 398 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = symb_dollar((yyvsp[0])); }
#line 3790 "y.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 399 "input_parser.yy" /* yacc.c:1646  */
    {(yyval)=symb_dollar(makesequence((yyvsp[-4]),(yyvsp[-2]),(yyvsp[0])));}
#line 3796 "y.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 400 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = symb_dollar(makesequence((yyvsp[-2]),(yyvsp[0]))); }
#line 3802 "y.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 401 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = symb_dollar(makesequence((yyvsp[-2]),(yyvsp[0]))); }
#line 3808 "y.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 402 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_dollar((yyvsp[0])); }
#line 3814 "y.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 403 "input_parser.yy" /* yacc.c:1646  */
    {  //CERR << $1 << " compose " << $2 << $3 << endl;
(yyval) = symbolic(*(yyvsp[-1])._FUNCptr,makesequence((yyvsp[-2]),python_compat(giac_yyget_extra(scanner))?denest_sto((yyvsp[0])):(yyvsp[0])) ); }
#line 3821 "y.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 405 "input_parser.yy" /* yacc.c:1646  */
    {(yyval)=symbolic(at_ans,-1);}
#line 3827 "y.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 406 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = symbolic(((yyvsp[-1]).type==_FUNC?*(yyvsp[-1])._FUNCptr:*at_union),makesequence((yyvsp[-2]),(yyvsp[0]))); }
#line 3833 "y.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 407 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = symbolic(((yyvsp[-2]).type==_FUNC?*(yyvsp[-2])._FUNCptr:*at_union),gen(makevecteur((yyvsp[-3]),(yyvsp[-3])*(yyvsp[-1])/100) ,_SEQ__VECT)); }
#line 3839 "y.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 408 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = symb_intersect(makesequence((yyvsp[-2]),(yyvsp[0]))); }
#line 3845 "y.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 409 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = symb_minus(makesequence((yyvsp[-2]),(yyvsp[0]))); }
#line 3851 "y.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 410 "input_parser.yy" /* yacc.c:1646  */
    { 
	(yyval)=symbolic(*(yyvsp[-1])._FUNCptr,makesequence((yyvsp[-2]),(yyvsp[0])) ); 
	}
#line 3859 "y.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 413 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = (yyvsp[0]); }
#line 3865 "y.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 414 "input_parser.yy" /* yacc.c:1646  */
    {if ((yyvsp[-1]).type==_FUNC) (yyval)=(yyvsp[-1]); else { 
          // const giac::context * contextptr = giac_yyget_extra(scanner);
          (yyval)=symb_quote((yyvsp[-1]));
          } 
        }
#line 3875 "y.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 419 "input_parser.yy" /* yacc.c:1646  */
    {
          const giac::context * contextptr = giac_yyget_extra(scanner);
	  (yyval) = symb_at((yyvsp[-3]),(yyvsp[-1]),contextptr);
        }
#line 3884 "y.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 423 "input_parser.yy" /* yacc.c:1646  */
    {
          const giac::context * contextptr = giac_yyget_extra(scanner);
	  (yyval) = symbolic(at_of,makesequence((yyvsp[-5]),(yyvsp[-2])));
        }
#line 3893 "y.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 427 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = check_symb_of((yyvsp[-4]),(yyvsp[-1]),giac_yyget_extra(scanner));}
#line 3899 "y.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 428 "input_parser.yy" /* yacc.c:1646  */
    {
	if ((yyvsp[-2])==_LIST__VECT && python_compat(giac_yyget_extra(scanner))){
           (yyval)=symbolic(at_python_list,(yyvsp[-1]));
        }
        else {
 	 if (abs_calc_mode(giac_yyget_extra(scanner))==38 && (yyvsp[-1]).type==_VECT && (yyvsp[-1]).subtype==_SEQ__VECT && (yyvsp[-1])._VECTptr->size()==2 && ((yyvsp[-1])._VECTptr->front().type<=_DOUBLE_ || (yyvsp[-1])._VECTptr->front().type==_FLOAT_) && ((yyvsp[-1])._VECTptr->back().type<=_DOUBLE_ || (yyvsp[-1])._VECTptr->back().type==_FLOAT_)){ 
           const giac::context * contextptr = giac_yyget_extra(scanner);
	   gen a=evalf((yyvsp[-1])._VECTptr->front(),1,contextptr),
	       b=evalf((yyvsp[-1])._VECTptr->back(),1,contextptr);
	   if ( (a.type==_DOUBLE_ || a.type==_FLOAT_) &&
                (b.type==_DOUBLE_ || b.type==_FLOAT_))
             (yyval)= a+b*cst_i; 
           else (yyval)=(yyvsp[-1]);
  	 } else {
              if (calc_mode(giac_yyget_extra(scanner))==1 && (yyvsp[-1]).type==_VECT && (yyvsp[-2])!=_LIST__VECT &&
	      (yyvsp[-1]).subtype==_SEQ__VECT && ((yyvsp[-1])._VECTptr->size()==2 || (yyvsp[-1])._VECTptr->size()==3) )
                (yyval) = gen(*(yyvsp[-1])._VECTptr,_GGB__VECT);
              else
                (yyval)=(yyvsp[-1]);
          }
	 }
        }
#line 3926 "y.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 450 "input_parser.yy" /* yacc.c:1646  */
    { 
        //cerr << $1 << " " << $2 << endl;
        (yyval) = gen(*((yyvsp[-1])._VECTptr),(yyvsp[-2]).val);
        // cerr << $$ << endl;

        }
#line 3937 "y.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 456 "input_parser.yy" /* yacc.c:1646  */
    { 
         if ((yyvsp[-2]).type==_VECT && (yyvsp[-2]).subtype==_SEQ__VECT && !((yyvsp[0]).type==_VECT && (yyvsp[-1]).subtype==_SEQ__VECT)){ (yyval)=(yyvsp[-2]); (yyval)._VECTptr->push_back((yyvsp[0])); }
	 else
           (yyval) = makesuite((yyvsp[-2]),(yyvsp[0])); 

        }
#line 3948 "y.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 462 "input_parser.yy" /* yacc.c:1646  */
    {(yyval)=symb_findhelp((yyvsp[0]));}
#line 3954 "y.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 463 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_interrogation((yyvsp[-2]),(yyvsp[0])); }
#line 3960 "y.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 464 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_pow((yyvsp[-1]),(yyvsp[0])); }
#line 3966 "y.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 465 "input_parser.yy" /* yacc.c:1646  */
    { 
        const giac::context * contextptr = giac_yyget_extra(scanner);
#ifdef HAVE_SIGNAL_H_OLD
	messages_to_print += parser_filename(contextptr) + parser_error(contextptr); 
	/* *logptr(giac_yyget_extra(scanner)) << messages_to_print; */
#endif
	(yyval)=undef;
        spread_formula(false,contextptr); 
	}
#line 3980 "y.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 474 "input_parser.yy" /* yacc.c:1646  */
    {
	(yyval) = symbolic(*(yyvsp[-5])._FUNCptr,makevecteur(equaltosame((yyvsp[-3])),symb_bloc((yyvsp[-1])),(yyvsp[0])));
	}
#line 3988 "y.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 477 "input_parser.yy" /* yacc.c:1646  */
    {
        vecteur v=makevecteur(equaltosame((yyvsp[-4])),(yyvsp[-2]),(yyvsp[0]));
	// *logptr(giac_yyget_extra(scanner)) << v << endl;
	(yyval) = symbolic(*(yyvsp[-6])._FUNCptr,v);
	}
#line 3998 "y.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 482 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           (yyval)=symb_program((yyvsp[-4]),zero*(yyvsp[-4]),symb_local((yyvsp[-2]),(yyvsp[-1]),contextptr),contextptr); 
        }
#line 4008 "y.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 487 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           (yyval)=symb_program_sto((yyvsp[-4]),zero*(yyvsp[-4]),symb_local((yyvsp[-2]),(yyvsp[-1]),contextptr),(yyvsp[-6]),false,contextptr); 
        }
#line 4018 "y.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 492 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           (yyval)=symb_program_sto((yyvsp[-3]),zero*(yyvsp[-3]),symb_bloc((yyvsp[-1])),(yyvsp[-5]),false,contextptr); 
        }
#line 4028 "y.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 497 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           (yyval)=symb_program_sto((yyvsp[-5]),zero*(yyvsp[-5]),symb_local((yyvsp[-2]),(yyvsp[-1]),contextptr),(yyvsp[-7]),false,contextptr); 
        }
#line 4038 "y.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 502 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
         (yyval)=symb_program((yyvsp[-5]),zero*(yyvsp[-5]),symb_local((yyvsp[-3]),(yyvsp[-1]),contextptr),contextptr); 
        }
#line 4048 "y.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 507 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           (yyval)=symb_program_sto((yyvsp[-5]),zero*(yyvsp[-5]),symb_local((yyvsp[-2]),(yyvsp[-1]),contextptr),(yyvsp[-7]),false,contextptr); 
        }
#line 4058 "y.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 512 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=3) giac_yyerror(scanner,"missing func/prog/proc end delimiter");
          const giac::context * contextptr = giac_yyget_extra(scanner);
           (yyval)=symb_program_sto((yyvsp[-6]),zero*(yyvsp[-6]),symb_local((yyvsp[-2]),(yyvsp[-1]),contextptr),(yyvsp[-8]),false,contextptr); 
        }
#line 4068 "y.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 517 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic((yyvsp[-8]).type==_FUNC?*(yyvsp[-8])._FUNCptr:*at_for,makevecteur((yyvsp[-6]),equaltosame((yyvsp[-4])),(yyvsp[-2]),symb_bloc((yyvsp[0]))));}
#line 4074 "y.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 518 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic((yyvsp[-9]).type==_FUNC?*(yyvsp[-9])._FUNCptr:*at_for,makevecteur((yyvsp[-7]),equaltosame((yyvsp[-5])),(yyvsp[-3]),(yyvsp[-1])));}
#line 4080 "y.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 519 "input_parser.yy" /* yacc.c:1646  */
    {(yyval) = symbolic((yyvsp[-3]).type==_FUNC?*(yyvsp[-3])._FUNCptr:*at_for,gen2vecteur((yyvsp[-1])));}
#line 4086 "y.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 520 "input_parser.yy" /* yacc.c:1646  */
    {(yyval)=symbolic(at_member,makesequence((yyvsp[-2]),(yyvsp[0]))); if ((yyvsp[-1])==at_not) (yyval)=symbolic(at_not,(yyval));}
#line 4092 "y.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 521 "input_parser.yy" /* yacc.c:1646  */
    {(yyval)=symbolic(at_not,symbolic(at_member,makesequence((yyvsp[-3]),(yyvsp[0]))));}
#line 4098 "y.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 522 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(at_apply,makesequence(symbolic(at_program,makesequence((yyvsp[-3]),0*(yyvsp[-3]),vecteur(1,(yyvsp[-5])))),(yyvsp[-1]))); if ((yyvsp[-6])==_TABLE__VECT) (yyval)=symbolic(at_table,(yyval));}
#line 4104 "y.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 523 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(at_apply,symbolic(at_program,makesequence((yyvsp[-5]),0*(yyvsp[-5]),vecteur(1,(yyvsp[-7])))),symbolic(at_select,makesequence(symbolic(at_program,makesequence((yyvsp[-5]),0*(yyvsp[-5]),(yyvsp[-1]))),(yyvsp[-3])))); if ((yyvsp[-8])==_TABLE__VECT) (yyval)=symbolic(at_table,(yyval));}
#line 4110 "y.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 524 "input_parser.yy" /* yacc.c:1646  */
    { 
	vecteur v=makevecteur(zero,equaltosame((yyvsp[-2])),zero,symb_bloc((yyvsp[0])));
	(yyval)=symbolic((yyvsp[-4]).type==_FUNC?*(yyvsp[-4])._FUNCptr:*at_for,v); 
	}
#line 4119 "y.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 528 "input_parser.yy" /* yacc.c:1646  */
    { 
	(yyval)=symbolic((yyvsp[-5]).type==_FUNC?*(yyvsp[-5])._FUNCptr:*at_for,makevecteur(zero,equaltosame((yyvsp[-3])),zero,(yyvsp[-1]))); 
	}
#line 4127 "y.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 531 "input_parser.yy" /* yacc.c:1646  */
    { 
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=9 && (yyvsp[0]).val!=8) giac_yyerror(scanner,"missing loop end delimiter");
	  (yyval)=symbolic((yyvsp[-4]).type==_FUNC?*(yyvsp[-4])._FUNCptr:*at_for,makevecteur(zero,equaltosame((yyvsp[-3])),zero,symb_bloc((yyvsp[-1])))); 
        }
#line 4136 "y.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 535 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_try_catch(makevecteur(symb_bloc((yyvsp[-5])),(yyvsp[-2]),symb_bloc((yyvsp[0]))));}
#line 4142 "y.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 536 "input_parser.yy" /* yacc.c:1646  */
    {(yyval)=symb_try_catch(gen2vecteur((yyvsp[-1])));}
#line 4148 "y.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 537 "input_parser.yy" /* yacc.c:1646  */
    {(yyval)=gen(at_try_catch,3);}
#line 4154 "y.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 538 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_case((yyvsp[-4]),(yyvsp[-1])); }
#line 4160 "y.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 539 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = symb_case((yyvsp[-1])); }
#line 4166 "y.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 540 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_case((yyvsp[-2]),(yyvsp[-1])); }
#line 4172 "y.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 543 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 4178 "y.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 544 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 4184 "y.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 545 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 4190 "y.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 548 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 4196 "y.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 549 "input_parser.yy" /* yacc.c:1646  */
    { 
	       gen tmp((yyvsp[0])); 
	       // tmp.subtype=1; 
	       (yyval)=symb_check_type(makevecteur(tmp,(yyvsp[-2])),context0); 
          }
#line 4206 "y.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 554 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_double_deux_points(makevecteur((yyvsp[-2]),(yyvsp[0]))); }
#line 4212 "y.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 555 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_double_deux_points(makevecteur((yyvsp[-2]),(yyvsp[0]))); }
#line 4218 "y.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 556 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_double_deux_points(makevecteur((yyvsp[-2]),(yyvsp[0]))); }
#line 4224 "y.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 557 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_double_deux_points(makevecteur((yyvsp[-4]),(yyvsp[-1]))); }
#line 4230 "y.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 558 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_double_deux_points(makevecteur(0,(yyvsp[0]))); }
#line 4236 "y.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 559 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_double_deux_points(makevecteur((yyvsp[-2]),(yyvsp[0]))); }
#line 4242 "y.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 567 "input_parser.yy" /* yacc.c:1646  */
    { 
	  gen tmp((yyvsp[-1])); 
	  // tmp.subtype=1; 
	  (yyval)=symb_check_type(makevecteur(tmp,(yyvsp[0])),context0); 
	  }
#line 4252 "y.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 572 "input_parser.yy" /* yacc.c:1646  */
    {(yyval)=symbolic(*(yyvsp[-1])._FUNCptr,(yyvsp[0])); }
#line 4258 "y.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 575 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 4264 "y.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 576 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 4270 "y.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 579 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=makevecteur(vecteur(0),vecteur(0)); }
#line 4276 "y.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 580 "input_parser.yy" /* yacc.c:1646  */
    { vecteur v1 =gen2vecteur((yyvsp[-1])); vecteur v2=gen2vecteur((yyvsp[0])); (yyval)=makevecteur(mergevecteur(gen2vecteur(v1[0]),gen2vecteur(v2[0])),mergevecteur(gen2vecteur(v1[1]),gen2vecteur(v2[1]))); }
#line 4282 "y.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 584 "input_parser.yy" /* yacc.c:1646  */
    { if (!(yyvsp[-2]).val) (yyval)=makevecteur((yyvsp[-1]),vecteur(0)); else (yyval)=makevecteur(vecteur(0),(yyvsp[-1]));}
#line 4288 "y.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 587 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=gen(vecteur(1,(yyvsp[0])),_SEQ__VECT); }
#line 4294 "y.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 588 "input_parser.yy" /* yacc.c:1646  */
    { 
	       vecteur v=*(yyvsp[-2])._VECTptr;
	       v.push_back((yyvsp[0]));
	       (yyval)=gen(v,_SEQ__VECT);
	     }
#line 4304 "y.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 595 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 4310 "y.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 596 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_sto((yyvsp[0]),(yyvsp[-2]),(yyvsp[-1])==at_array_sto); }
#line 4316 "y.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 597 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_equal((yyvsp[-2]),(yyvsp[0])); }
#line 4322 "y.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 598 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symbolic(at_deuxpoints,makesequence((yyvsp[-2]),(yyvsp[0])));  }
#line 4328 "y.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 599 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[-1]); }
#line 4334 "y.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 600 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); *logptr(giac_yyget_extra(scanner)) << "Error: reserved word "<< (yyvsp[0]) <<endl;}
#line 4340 "y.tab.c" /* yacc.c:1646  */
    break;

  case 166:
#line 601 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_double_deux_points(makevecteur((yyvsp[-2]),(yyvsp[0]))); *logptr(giac_yyget_extra(scanner)) << "Error: reserved word "<< (yyvsp[-2]) <<endl; }
#line 4346 "y.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 602 "input_parser.yy" /* yacc.c:1646  */
    { 
  const giac::context * contextptr = giac_yyget_extra(scanner);
  (yyval)=string2gen("_"+(yyvsp[0]).print(contextptr),false); 
  if (!giac::first_error_line(contextptr)){
    giac::first_error_line(giac::lexer_line_number(contextptr),contextptr);
    giac:: error_token_name((yyvsp[0]).print(contextptr)+ " (reserved word)",contextptr);
  }
}
#line 4359 "y.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 610 "input_parser.yy" /* yacc.c:1646  */
    { 
  const giac::context * contextptr = giac_yyget_extra(scanner);
  (yyval)=string2gen("_"+(yyvsp[0]).print(contextptr),false);
  if (!giac::first_error_line(contextptr)){
    giac::first_error_line(giac::lexer_line_number(contextptr),contextptr);
    giac:: error_token_name((yyvsp[0]).print(contextptr)+ " reserved word",contextptr);
  }
}
#line 4372 "y.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 620 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=plus_one;}
#line 4378 "y.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 621 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 4384 "y.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 624 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=gen(vecteur(0),_SEQ__VECT); }
#line 4390 "y.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 625 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=makesuite((yyvsp[0])); }
#line 4396 "y.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 628 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = makevecteur((yyvsp[0])); }
#line 4402 "y.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 630 "input_parser.yy" /* yacc.c:1646  */
    { vecteur v(1,(yyvsp[-1])); 
			  if ((yyvsp[-1]).type==_VECT) v=*((yyvsp[-1])._VECTptr); 
			  v.push_back((yyvsp[0])); 
			  (yyval) = v;
			}
#line 4412 "y.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 635 "input_parser.yy" /* yacc.c:1646  */
    { (yyval) = (yyvsp[-1]);}
#line 4418 "y.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 639 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=plus_one; }
#line 4424 "y.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 640 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 4430 "y.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 643 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=plus_one; }
#line 4436 "y.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 644 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 4442 "y.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 645 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 4448 "y.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 646 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 4454 "y.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 649 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=plus_one; }
#line 4460 "y.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 650 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 4466 "y.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 653 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=0; }
#line 4472 "y.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 654 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[-1]); }
#line 4478 "y.tab.c" /* yacc.c:1646  */
    break;

  case 186:
#line 655 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=symb_bloc((yyvsp[0])); }
#line 4484 "y.tab.c" /* yacc.c:1646  */
    break;

  case 187:
#line 658 "input_parser.yy" /* yacc.c:1646  */
    { 
	(yyval) = (yyvsp[-1]);
	}
#line 4492 "y.tab.c" /* yacc.c:1646  */
    break;

  case 188:
#line 661 "input_parser.yy" /* yacc.c:1646  */
    {
          const giac::context * contextptr = giac_yyget_extra(scanner);
          (yyval) = symb_local((yyvsp[-2]),(yyvsp[-1]),contextptr);
         }
#line 4501 "y.tab.c" /* yacc.c:1646  */
    break;

  case 189:
#line 668 "input_parser.yy" /* yacc.c:1646  */
    { if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=4) giac_yyerror(scanner,"missing test end delimiter"); (yyval)=0; }
#line 4507 "y.tab.c" /* yacc.c:1646  */
    break;

  case 190:
#line 669 "input_parser.yy" /* yacc.c:1646  */
    {
          if ((yyvsp[0]).type==_INT_ && (yyvsp[0]).val && (yyvsp[0]).val!=4) giac_yyerror(scanner,"missing test end delimiter");
	(yyval)=symb_bloc((yyvsp[-1])); 
	}
#line 4516 "y.tab.c" /* yacc.c:1646  */
    break;

  case 191:
#line 673 "input_parser.yy" /* yacc.c:1646  */
    { 
	  (yyval)=symb_ifte(equaltosame((yyvsp[-3])),symb_bloc((yyvsp[-1])),(yyvsp[0]));
	  }
#line 4524 "y.tab.c" /* yacc.c:1646  */
    break;

  case 192:
#line 678 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 4530 "y.tab.c" /* yacc.c:1646  */
    break;

  case 193:
#line 681 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=0; }
#line 4536 "y.tab.c" /* yacc.c:1646  */
    break;

  case 194:
#line 684 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=vecteur(0); }
#line 4542 "y.tab.c" /* yacc.c:1646  */
    break;

  case 195:
#line 685 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=makevecteur(symb_bloc((yyvsp[0])));}
#line 4548 "y.tab.c" /* yacc.c:1646  */
    break;

  case 196:
#line 686 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=mergevecteur(makevecteur((yyvsp[-3]),symb_bloc((yyvsp[-1]))),*((yyvsp[0])._VECTptr));}
#line 4554 "y.tab.c" /* yacc.c:1646  */
    break;

  case 197:
#line 689 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=vecteur(0); }
#line 4560 "y.tab.c" /* yacc.c:1646  */
    break;

  case 198:
#line 690 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=vecteur(1,symb_bloc((yyvsp[0]))); }
#line 4566 "y.tab.c" /* yacc.c:1646  */
    break;

  case 199:
#line 691 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=mergevecteur(makevecteur((yyvsp[-3]),symb_bloc((yyvsp[-1]))),*((yyvsp[0])._VECTptr));}
#line 4572 "y.tab.c" /* yacc.c:1646  */
    break;

  case 200:
#line 694 "input_parser.yy" /* yacc.c:1646  */
    { (yyval)=(yyvsp[0]); }
#line 4578 "y.tab.c" /* yacc.c:1646  */
    break;


#line 4582 "y.tab.c" /* yacc.c:1646  */
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

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
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

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

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

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


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

#if !defined yyoverflow || YYERROR_VERBOSE
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
  /* Do not reclaim the symbols of the rule whose action triggered
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
  return yyresult;
}
#line 701 "input_parser.yy" /* yacc.c:1906  */


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
