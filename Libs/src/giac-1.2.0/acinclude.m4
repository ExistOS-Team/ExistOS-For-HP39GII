
dnl Usage: GINAC_STD_CXX_HEADERS
dnl Check for standard C++ headers, bail out if something is missing.
AC_DEFUN([GINAC_STD_CXX_HEADERS], [
AC_CACHE_CHECK([for standard C++ header files], [ginac_cv_std_cxx_headers], [
	ginac_cv_std_cxx_headers="no"
	AC_LANG_PUSH([C++])
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
		#include <iosfwd>
		#include <iostream>
		#include <vector>
		#include <list>
		#include <map>
		#include <set>
		#include <string>
		#include <sstream>
		#include <typeinfo>
		#include <stdexcept>
		#include <algorithm>
		#include <limits>
		#include <ctime>
		]])], [ginac_cv_std_cxx_headers="yes"])
	AC_LANG_POP([C++])])
if test "${ginac_cv_std_cxx_headers}" != "yes"; then
	AC_MSG_ERROR([Standard ISO C++ 98 headers are missing])
fi
])

dnl Usage: GINAC_LIBREADLINE
dnl
dnl Check if GNU readline library and headers are avialable.
dnl Defines GINSH_LIBS variable, and HAVE_LIBREADLINE,
dnl HAVE_READLINE_READLINE_H, HAVE_READLINE_HISTORY_H preprocessor macros.
dnl
dnl Note: this macro rejects readline versions <= 4.2 and non-GNU
dnl implementations.
dnl
AC_DEFUN([GINAC_READLINE],[
AC_REQUIRE([GINAC_TERMCAP])
GINSH_LIBS=""
AC_CHECK_HEADERS([readline/readline.h readline/history.h])
if test "x${ac_cv_header_readline_readline_h}" != "xyes" -o "x${ac_cv_header_readline_history_h}" != "xyes"; then
	AC_MSG_WARN([readline headers could not be found.])
else
	AC_CACHE_CHECK([for version of libreadline], [ginac_cv_rl_supported], [
		ginac_cv_rl_supported="no"
		AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
			#include <stdio.h>
			#include <readline/readline.h>
			#if !defined(RL_VERSION_MAJOR) || !defined(RL_VERSION_MINOR)
			#error "Ancient/unsupported version of readline"
			#endif]])],
			[ginac_cv_rl_supported="yes"])])
	if test "x${ginac_cv_rl_supported}" != "xyes"; then
		AC_MSG_WARN([Unsupported version of readline (<= 4.2 or non-GNU).])
	else
		save_LIBS="$LIBS"
		LIBS="$LIBTERMCAP $LIBS"
		AC_CHECK_LIB(readline, readline)
		if test "x${ac_cv_lib_readline_readline}" != "xyes"; then
			AC_MSG_WARN([libreadline could not be found.])
		fi
		GINSH_LIBS="$LIBS"
		LIBS="$save_LIBS"
	fi
fi
AC_SUBST(GINSH_LIBS)])

dnl Usage: GINAC_TERMCAP
dnl libreadline is based on the termcap functions.
dnl Some systems have tgetent(), tgetnum(), tgetstr(), tgetflag(), tputs(),
dnl tgoto() in libc, some have it in libtermcap, some have it in libncurses.
dnl When both libtermcap and libncurses exist, we prefer the latter, because
dnl libtermcap is being phased out.
AC_DEFUN([GINAC_TERMCAP], [
AC_REQUIRE([AC_CANONICAL_HOST])
LIBTERMCAP=""
case $host_os in
*mingw32*)
 ;; dnl no termcap libraries are necessary (need hacked libreadline)
*)
AC_CHECK_FUNCS(tgetent)
if test "x$ac_cv_func_tgetent" = "xyes"; then
    :
else
    AC_CHECK_LIB(ncurses, tgetent, LIBTERMCAP="-lncurses")
    if test -z "$LIBTERMCAP"; then
        AC_CHECK_LIB(termcap, tgetent, LIBTERMCAP="-ltermcap")
    fi
fi
;;
esac
AC_SUBST(LIBTERMCAP)
])

dnl Is the gmp header file new enough? (should be implemented with an argument)
AC_DEFUN([CL_GMP_H_VERSION], [
AC_CACHE_CHECK([for recent enough gmp.h], cl_cv_new_gmp_h, [
	AC_TRY_CPP([#include <gmp.h>
#if !defined(__GNU_MP_VERSION) || (__GNU_MP_VERSION < 3)
 #error "ancient gmp.h"
#endif],
cl_cv_new_gmp_h="yes", cl_cv_new_gmp_h="no")
])
if test "$cl_cv_new_gmp_h" = "yes"; then
	ifelse([$1], ,:,[$1])
else
	ifelse([$2], ,[AC_MSG_ERROR([GMP version is way too old])],[$3])
fi
])

dnl Does libgmp provide some functionality introduced in version 3.0?
AC_DEFUN([CL_GMP_CHECK], [
AC_REQUIRE([CL_GMP_H_VERSION])
AC_CACHE_CHECK([for working libgmp], cl_cv_new_libgmp, [
	SAVELIBS="$LIBS"
	LIBS="$LIBS -lgmp"
	AC_TRY_LINK([#include <gmp.h>],[mpn_divexact_by3(0,0,0)],
		cl_cv_new_libgmp="yes", cl_cv_new_libgmp="no")
	LIBS="$SAVELIBS"])
	if test "$cl_cv_new_libgmp" = yes; then
		ifelse([$1], ,[LIBS="$LIBS -lgmp"], [$1])
	else
		ifelse([$2], ,[AC_MSG_ERROR([GMP version is way too old])], [$2])
	fi
])

