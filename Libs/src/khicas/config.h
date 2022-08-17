/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.in by autoheader.  */

/* Set if debugging is enabled */
#define DEBUG_SUPPORT 

/* Name of package */
#define PACKAGE "giac"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "giac"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "giac 1.4.9"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "giac"

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.4.9"

/* The size of `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG 4

/* The size of `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG 8

/* Version number of package */
#define VERSION "1.4.9-57"
#define FXCG
#define GIAC_NO_OPTIMIZATIONS
#define HAVE_NO_HOME_DIRECTORY
#define HAVE_NO_SIGNAL_H
#define HAVE_NO_SYS_RESOURCE_WAIT_H
#define HAVE_NO_PWD_H
#define HAVE_NO_CWD

#define HAVE_SYSCONF
#ifndef HAVE_UNISTD_H
#define HAVE_UNISTD_H
#endif
#define HAVE_NO_SYS_TIMES_H 
#define HAVE_LIBTOMMATH
#define USE_GMP_REPLACEMENTS
#ifndef HAVE_CONFIG_H
#define HAVE_CONFIG_H
#endif
#define NO_UNARY_FUNCTION_COMPOSE
#define STATIC_BUILTIN_LEXER_FUNCTIONS
#define IN_GIAC
#define NO_PHYSICAL_CONSTANTS
#define NO_STDEXCEPT
#define NO_TEMPLATE_MULTGCD
//#define GIAC_VECTOR
#define HAVE_SSTREAM
//#define USTL
//#define std ustl
#define ustl std
#define NO_RTTI
#define TIMEOUT
//#include "softmath.h"
#define RELEASE
    extern volatile bool ctrl_c;
    extern volatile bool interrupted;
    #include <cassert>
#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif